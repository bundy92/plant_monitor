/**
 * @file test_plant_monitor.cpp
 * @brief Unit Tests for Plant Monitor System - Modular Architecture
 * 
 * This file contains comprehensive unit tests for the modular plant
 * monitoring system including sensor interface, display interface,
 * and individual sensor drivers (AHT10, DS18B20, GY-302).
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "sensor_interface.h"
#include "display_interface.h"
#include "aht10.h"
#include "ds18b20.h"
#include "gy302.h"

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

/**
 * @brief Test fixture for plant monitor system tests
 */
class PlantMonitorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test configuration
        memset(&sensor_config, 0, sizeof(sensor_interface_config_t));
        memset(&display_config, 0, sizeof(display_interface_config_t));
        
        // Configure test sensors
        sensor_config.sensor_count = 4;
        sensor_config.i2c_sda_pin = 21;
        sensor_config.i2c_scl_pin = 22;
        sensor_config.i2c_frequency = 100000;
        sensor_config.onewire_pin = 4;
        sensor_config.adc_soil_pin = 1;
        sensor_config.adc_light_pin = 2;
        
        // AHT10 sensor
        sensor_config.sensors[0].type = SENSOR_TYPE_AHT10;
        sensor_config.sensors[0].address = 0x38;
        sensor_config.sensors[0].enabled = true;
        strcpy(sensor_config.sensors[0].name, "AHT10-Test");
        
        // DS18B20 sensor
        sensor_config.sensors[1].type = SENSOR_TYPE_DS18B20;
        sensor_config.sensors[1].pin = 4;
        sensor_config.sensors[1].enabled = true;
        strcpy(sensor_config.sensors[1].name, "DS18B20-Test");
        
        // GY-302 sensor
        sensor_config.sensors[2].type = SENSOR_TYPE_GY302;
        sensor_config.sensors[2].address = 0x23;
        sensor_config.sensors[2].enabled = true;
        strcpy(sensor_config.sensors[2].name, "GY302-Test");
        
        // Soil moisture sensor
        sensor_config.sensors[3].type = SENSOR_TYPE_SOIL_MOISTURE;
        sensor_config.sensors[3].pin = 1;
        sensor_config.sensors[3].enabled = true;
        strcpy(sensor_config.sensors[3].name, "Soil-Test");
        
        // Configure test displays
        display_config.display_count = 3;
        display_config.enable_backlight = true;
        display_config.brightness = 128;
        
        // Console display
        display_config.displays[0].type = DISPLAY_TYPE_CONSOLE;
        display_config.displays[0].enabled = true;
        strcpy(display_config.displays[0].name, "Console-Test");
        
        // Built-in OLED
        display_config.displays[1].type = DISPLAY_TYPE_BUILTIN_SSD1306;
        display_config.displays[1].i2c_address = 0x3C;
        display_config.displays[1].enabled = true;
        strcpy(display_config.displays[1].name, "OLED-Test");
        
        // E-paper display
        display_config.displays[2].type = DISPLAY_TYPE_EPAPER_SPI;
        display_config.displays[2].spi_cs_pin = 5;
        display_config.displays[2].enabled = true;
        strcpy(display_config.displays[2].name, "Epaper-Test");
    }
    
    void TearDown() override {
        // Clean up
        sensor_interface_deinit();
        display_interface_deinit();
    }
    
    sensor_interface_config_t sensor_config;
    display_interface_config_t display_config;
};

/**
 * @brief Test sensor interface initialization
 */
TEST_F(PlantMonitorTest, SensorInterfaceInit) {
    esp_err_t ret = sensor_interface_init(&sensor_config);
    EXPECT_EQ(ret, ESP_OK);
    
    // Test with invalid config
    ret = sensor_interface_init(nullptr);
    EXPECT_EQ(ret, ESP_ERR_INVALID_ARG);
}

/**
 * @brief Test display interface initialization
 */
TEST_F(PlantMonitorTest, DisplayInterfaceInit) {
    esp_err_t ret = display_interface_init(&display_config);
    EXPECT_EQ(ret, ESP_OK);
    
    // Test with invalid config
    ret = display_interface_init(nullptr);
    EXPECT_EQ(ret, ESP_ERR_INVALID_ARG);
}

/**
 * @brief Test sensor reading functionality
 */
TEST_F(PlantMonitorTest, SensorReading) {
    esp_err_t ret = sensor_interface_init(&sensor_config);
    ASSERT_EQ(ret, ESP_OK);
    
    sensor_reading_t readings[4];
    int count = sensor_interface_read_all(readings, 4);
    
    // Should return number of enabled sensors (may be 0 if no hardware)
    EXPECT_GE(count, 0);
    EXPECT_LE(count, 4);
}

/**
 * @brief Test individual sensor reading
 */
TEST_F(PlantMonitorTest, IndividualSensorReading) {
    esp_err_t ret = sensor_interface_init(&sensor_config);
    ASSERT_EQ(ret, ESP_OK);
    
    sensor_reading_t reading;
    
    // Test AHT10 reading
    ret = sensor_interface_read_sensor(SENSOR_TYPE_AHT10, &reading);
    // May fail if no hardware, but should not crash
    EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_NOT_FOUND);
    
    // Test DS18B20 reading
    ret = sensor_interface_read_sensor(SENSOR_TYPE_DS18B20, &reading);
    EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_NOT_FOUND);
    
    // Test GY-302 reading
    ret = sensor_interface_read_sensor(SENSOR_TYPE_GY302, &reading);
    EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_NOT_FOUND);
}

/**
 * @brief Test display update functionality
 */
TEST_F(PlantMonitorTest, DisplayUpdate) {
    esp_err_t ret = display_interface_init(&display_config);
    ASSERT_EQ(ret, ESP_OK);
    
    sensor_data_t sensor_data = {
        .temperature = 25.5f,
        .humidity = 60.0f,
        .soil_moisture = 2048,
        .light_level = 1024,
        .lux = 5000.0f,
        .uptime_seconds = 3600
    };
    
    plant_health_t health = {
        .health_score = 85.0f,
        .health_text = "Good",
        .emoji = "🙂",
        .recommendation = "Keep current conditions"
    };
    
    ret = display_interface_update(&sensor_data, &health);
    EXPECT_EQ(ret, ESP_OK);
}

/**
 * @brief Test I2C device scanning
 */
TEST_F(PlantMonitorTest, I2CScan) {
    esp_err_t ret = sensor_interface_init(&sensor_config);
    ASSERT_EQ(ret, ESP_OK);
    
    int device_count = sensor_interface_scan_i2c();
    // Should return >= 0 (may be 0 if no I2C devices)
    EXPECT_GE(device_count, 0);
}

/**
 * @brief Test sensor status
 */
TEST_F(PlantMonitorTest, SensorStatus) {
    esp_err_t ret = sensor_interface_init(&sensor_config);
    ASSERT_EQ(ret, ESP_OK);
    
    int working_sensors, total_sensors;
    ret = sensor_interface_get_status(&working_sensors, &total_sensors);
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(total_sensors, 4);
    EXPECT_GE(working_sensors, 0);
    EXPECT_LE(working_sensors, total_sensors);
}

/**
 * @brief Test display status
 */
TEST_F(PlantMonitorTest, DisplayStatus) {
    esp_err_t ret = display_interface_init(&display_config);
    ASSERT_EQ(ret, ESP_OK);
    
    int working_displays, total_displays;
    ret = display_interface_get_status(&working_displays, &total_displays);
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(total_displays, 3);
    EXPECT_GE(working_displays, 0);
    EXPECT_LE(working_displays, total_displays);
}

/**
 * @brief Test AHT10 sensor driver
 */
TEST_F(PlantMonitorTest, AHT10Driver) {
    aht10_config_t config = {
        .address = 0x38,
        .sda_pin = 21,
        .scl_pin = 22,
        .i2c_freq = 100000,
        .enabled = true
    };
    
    esp_err_t ret = aht10_init(&config);
    // May fail if no hardware, but should not crash
    EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_NOT_FOUND);
    
    if (ret == ESP_OK) {
        aht10_reading_t reading;
        ret = aht10_read(&reading);
        // May fail if no hardware, but should not crash
        EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_NOT_FOUND);
        
        aht10_deinit();
    }
}

/**
 * @brief Test DS18B20 sensor driver
 */
TEST_F(PlantMonitorTest, DS18B20Driver) {
    ds18b20_config_t config = {
        .pin = 4,
        .resolution = 12,
        .enabled = true,
        .rom_code = 0
    };
    
    esp_err_t ret = ds18b20_init(&config);
    // May fail if no hardware, but should not crash
    EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_NOT_FOUND);
    
    if (ret == ESP_OK) {
        ds18b20_reading_t reading;
        ret = ds18b20_read(&reading);
        // May fail if no hardware, but should not crash
        EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_NOT_FOUND);
        
        ds18b20_deinit();
    }
}

/**
 * @brief Test GY-302 sensor driver
 */
TEST_F(PlantMonitorTest, GY302Driver) {
    gy302_config_t config = {
        .address = 0x23,
        .sda_pin = 21,
        .scl_pin = 22,
        .i2c_freq = 100000,
        .mode = GY302_MODE_ONE_H,
        .enabled = true
    };
    
    esp_err_t ret = gy302_init(&config);
    // May fail if no hardware, but should not crash
    EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_NOT_FOUND);
    
    if (ret == ESP_OK) {
        gy302_reading_t reading;
        ret = gy302_read(&reading);
        // May fail if no hardware, but should not crash
        EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_NOT_FOUND);
        
        gy302_deinit();
    }
}

/**
 * @brief Test error handling with invalid parameters
 */
TEST_F(PlantMonitorTest, ErrorHandling) {
    // Test sensor interface with invalid parameters
    esp_err_t ret = sensor_interface_read_all(nullptr, 0);
    EXPECT_EQ(ret, -1);
    
    ret = sensor_interface_read_sensor(SENSOR_TYPE_AHT10, nullptr);
    EXPECT_EQ(ret, ESP_ERR_INVALID_ARG);
    
    // Test display interface with invalid parameters
    ret = display_interface_update(nullptr, nullptr);
    EXPECT_EQ(ret, ESP_ERR_INVALID_ARG);
    
    ret = display_interface_get_status(nullptr, nullptr);
    EXPECT_EQ(ret, ESP_ERR_INVALID_ARG);
}

/**
 * @brief Test sensor type validation
 */
TEST_F(PlantMonitorTest, SensorTypeValidation) {
    sensor_reading_t reading;
    
    // Test unknown sensor type
    esp_err_t ret = sensor_interface_read_sensor(SENSOR_TYPE_MAX, &reading);
    EXPECT_EQ(ret, ESP_ERR_NOT_FOUND);
}

/**
 * @brief Test display type validation
 */
TEST_F(PlantMonitorTest, DisplayTypeValidation) {
    display_interface_config_t config = display_config;
    config.displays[0].type = DISPLAY_TYPE_MAX;
    
    esp_err_t ret = display_interface_init(&config);
    // Should handle unknown display type gracefully
    EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_INVALID_ARG);
}

/**
 * @brief Test configuration validation
 */
TEST_F(PlantMonitorTest, ConfigurationValidation) {
    // Test with invalid sensor count
    sensor_interface_config_t invalid_config = sensor_config;
    invalid_config.sensor_count = 10; // More than array size
    
    esp_err_t ret = sensor_interface_init(&invalid_config);
    // Should handle gracefully
    EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_INVALID_ARG);
}

/**
 * @brief Test deinitialization
 */
TEST_F(PlantMonitorTest, Deinitialization) {
    esp_err_t ret = sensor_interface_init(&sensor_config);
    ASSERT_EQ(ret, ESP_OK);
    
    ret = sensor_interface_deinit();
    EXPECT_EQ(ret, ESP_OK);
    
    ret = display_interface_init(&display_config);
    ASSERT_EQ(ret, ESP_OK);
    
    ret = display_interface_deinit();
    EXPECT_EQ(ret, ESP_OK);
}

/**
 * @brief Test multiple initialization calls
 */
TEST_F(PlantMonitorTest, MultipleInitialization) {
    esp_err_t ret = sensor_interface_init(&sensor_config);
    ASSERT_EQ(ret, ESP_OK);
    
    // Second initialization should succeed (already initialized)
    ret = sensor_interface_init(&sensor_config);
    EXPECT_EQ(ret, ESP_OK);
    
    sensor_interface_deinit();
}

/**
 * @brief Test sensor reading without initialization
 */
TEST_F(PlantMonitorTest, ReadingWithoutInit) {
    sensor_reading_t reading;
    esp_err_t ret = sensor_interface_read_sensor(SENSOR_TYPE_AHT10, &reading);
    EXPECT_EQ(ret, ESP_ERR_INVALID_STATE);
}

/**
 * @brief Test display update without initialization
 */
TEST_F(PlantMonitorTest, DisplayWithoutInit) {
    sensor_data_t sensor_data = {0};
    plant_health_t health = {0};
    
    esp_err_t ret = display_interface_update(&sensor_data, &health);
    EXPECT_EQ(ret, ESP_ERR_INVALID_STATE);
}

/**
 * @brief Main function for running tests
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 