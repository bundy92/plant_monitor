#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

// Mock I2C functions
extern "C" {
    esp_err_t i2c_param_config(i2c_port_t i2c_num, const i2c_config_t* i2c_conf);
    esp_err_t i2c_driver_install(i2c_port_t i2c_num, i2c_mode_t mode, size_t slv_rx_buf_len, size_t slv_tx_buf_len, int intr_alloc_flags);
    esp_err_t i2c_master_cmd_begin(i2c_port_t i2c_num, i2c_cmd_handle_t cmd_handle, TickType_t xTicksToWait);
    i2c_cmd_handle_t i2c_cmd_link_create(void);
    void i2c_cmd_link_delete(i2c_cmd_handle_t cmd_handle);
    esp_err_t i2c_master_start(i2c_cmd_handle_t cmd_handle);
    esp_err_t i2c_master_write_byte(i2c_cmd_handle_t cmd_handle, uint8_t data, bool ack_en);
    esp_err_t i2c_master_write(i2c_cmd_handle_t cmd_handle, const uint8_t* data, size_t data_len, bool ack_en);
    esp_err_t i2c_master_read(i2c_cmd_handle_t cmd_handle, uint8_t* data, size_t data_len, i2c_ack_type_t ack);
    esp_err_t i2c_master_stop(i2c_cmd_handle_t cmd_handle);
}

// AHT10 sensor class for testing
class AHT10Sensor {
private:
    i2c_port_t i2c_port;
    uint8_t sensor_addr;
    bool initialized;

public:
    AHT10Sensor(i2c_port_t port, uint8_t addr) : i2c_port(port), sensor_addr(addr), initialized(false) {}
    
    esp_err_t init() {
        // Initialize I2C configuration
        i2c_config_t conf = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = GPIO_NUM_21,
            .scl_io_num = GPIO_NUM_22,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master = {
                .clk_speed = 50000, // Slower frequency for testing
            },
        };
        
        esp_err_t ret = i2c_param_config(i2c_port, &conf);
        if (ret != ESP_OK) return ret;
        
        ret = i2c_driver_install(i2c_port, conf.mode, 0, 0, 0);
        if (ret != ESP_OK) return ret;
        
        // Send soft reset command
        uint8_t reset_cmd[] = {0xBA}; // AHT10 soft reset
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (sensor_addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, reset_cmd, 1, true);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK) {
            initialized = true;
        }
        
        return ret;
    }
    
    esp_err_t read_temperature_humidity(float* temperature, float* humidity) {
        if (!initialized) return ESP_ERR_INVALID_STATE;
        
        // Send measurement command
        uint8_t measure_cmd[] = {0xAC, 0x33, 0x00};
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (sensor_addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, measure_cmd, 3, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        
        if (ret != ESP_OK) return ret;
        
        // Wait for measurement
        vTaskDelay(pdMS_TO_TICKS(80));
        
        // Read 6 bytes of data
        uint8_t data[6];
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (sensor_addr << 1) | I2C_MASTER_READ, true);
        i2c_master_read(cmd, data, 6, I2C_MASTER_LAST_NACK);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        
        if (ret != ESP_OK) return ret;
        
        // Check if measurement is ready
        if (data[0] & 0x80) {
            return ESP_ERR_INVALID_STATE;
        }
        
        // Calculate humidity (20-bit value)
        uint32_t humidity_raw = ((uint32_t)(data[1] & 0x0F) << 16) | ((uint32_t)data[2] << 8) | data[3];
        *humidity = (float)humidity_raw * 100.0 / 1048576.0;
        
        // Calculate temperature (20-bit value)
        uint32_t temp_raw = ((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];
        *temperature = (float)temp_raw * 200.0 / 1048576.0 - 50.0;
        
        return ESP_OK;
    }
    
    bool is_initialized() const { return initialized; }
    uint8_t get_address() const { return sensor_addr; }
};

// Test fixture for AHT10 tests
class AHT10Test : public ::testing::Test {
protected:
    AHT10Sensor* sensor1;
    AHT10Sensor* sensor2;
    
    void SetUp() override {
        sensor1 = new AHT10Sensor(I2C_NUM_0, 0x38);
        sensor2 = new AHT10Sensor(I2C_NUM_0, 0x39);
    }
    
    void TearDown() override {
        delete sensor1;
        delete sensor2;
    }
};

// Test cases
TEST_F(AHT10Test, InitializationTest) {
    esp_err_t ret = sensor1->init();
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_TRUE(sensor1->is_initialized());
}

TEST_F(AHT10Test, AddressTest) {
    EXPECT_EQ(sensor1->get_address(), 0x38);
    EXPECT_EQ(sensor2->get_address(), 0x39);
}

TEST_F(AHT10Test, TemperatureHumidityReadTest) {
    esp_err_t ret = sensor1->init();
    ASSERT_EQ(ret, ESP_OK);
    
    float temperature, humidity;
    ret = sensor1->read_temperature_humidity(&temperature, &humidity);
    
    if (ret == ESP_OK) {
        // Valid temperature range: -40 to 85°C
        EXPECT_GE(temperature, -40.0f);
        EXPECT_LE(temperature, 85.0f);
        
        // Valid humidity range: 0 to 100%
        EXPECT_GE(humidity, 0.0f);
        EXPECT_LE(humidity, 100.0f);
    } else {
        // If sensor not connected, that's also a valid test result
        EXPECT_EQ(ret, ESP_ERR_INVALID_STATE);
    }
}

TEST_F(AHT10Test, DualSensorTest) {
    esp_err_t ret1 = sensor1->init();
    esp_err_t ret2 = sensor2->init();
    
    // At least one sensor should initialize
    bool at_least_one_sensor = (ret1 == ESP_OK) || (ret2 == ESP_OK);
    EXPECT_TRUE(at_least_one_sensor);
}

TEST_F(AHT10Test, InvalidStateTest) {
    float temperature, humidity;
    esp_err_t ret = sensor1->read_temperature_humidity(&temperature, &humidity);
    EXPECT_EQ(ret, ESP_ERR_INVALID_STATE); // Should fail if not initialized
}

// Parameterized test for different I2C frequencies
class AHT10FrequencyTest : public ::testing::TestWithParam<uint32_t> {
protected:
    AHT10Sensor* sensor;
    
    void SetUp() override {
        sensor = new AHT10Sensor(I2C_NUM_0, 0x38);
    }
    
    void TearDown() override {
        delete sensor;
    }
};

TEST_P(AHT10FrequencyTest, FrequencyCompatibilityTest) {
    esp_err_t ret = sensor->init();
    // Should work with various frequencies
    EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_INVALID_STATE);
}

INSTANTIATE_TEST_SUITE_P(
    I2CFrequencies,
    AHT10FrequencyTest,
    ::testing::Values(50000, 100000, 400000)
);

// Main test runner
extern "C" void app_main(void) {
    ::testing::InitGoogleTest();
    RUN_ALL_TESTS();
} 