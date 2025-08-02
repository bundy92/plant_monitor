#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_err.h>

// I2C Configuration
#define I2C_MASTER_SCL_IO            GPIO_NUM_22      // I2C SCL pin
#define I2C_MASTER_SDA_IO            GPIO_NUM_21      // I2C SDA pin
#define I2C_MASTER_NUM               I2C_NUM_0        // I2C port number
#define I2C_MASTER_FREQ_HZ           100000           // I2C master clock frequency
#define I2C_MASTER_TIMEOUT_MS        1000             // I2C master timeout

// AHT10 sensor addresses
#define AHT10_SENSOR_1_ADDR          0x38             // First AHT10 sensor address
#define AHT10_SENSOR_2_ADDR          0x39             // Second AHT10 sensor address

// AHT10 sensor commands
#define AHT10_CMD_INITIALIZE    0xE1
#define AHT10_CMD_MEASURE       0xAC
#define AHT10_CMD_NORMAL        0xA8
#define AHT10_CMD_SOFT_RESET    0xBA

static const char *TAG = "AHT10_TEST";

// Initialize I2C
esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        return err;
    }
    
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

// Initialize AHT10 sensor
esp_err_t aht10_init(uint8_t sensor_addr)
{
    ESP_LOGI(TAG, "Initializing AHT10 sensor at address 0x%02X", sensor_addr);
    
    // Send soft reset command
    uint8_t reset_cmd[] = {AHT10_CMD_SOFT_RESET};
    esp_err_t ret = i2c_master_write_to_device(I2C_MASTER_NUM, sensor_addr, reset_cmd, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 reset failed for address 0x%02X", sensor_addr);
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(20)); // Wait for reset
    
    // Send initialization command
    uint8_t init_cmd[] = {AHT10_CMD_INITIALIZE, 0x08, 0x00};
    ret = i2c_master_write_to_device(I2C_MASTER_NUM, sensor_addr, init_cmd, 3, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 initialization failed for address 0x%02X", sensor_addr);
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(10)); // Wait for initialization
    
    ESP_LOGI(TAG, "AHT10 sensor at address 0x%02X initialized successfully", sensor_addr);
    return ESP_OK;
}

// Read temperature and humidity from AHT10
esp_err_t aht10_read_sensor(uint8_t sensor_addr, float *temperature, float *humidity)
{
    // Send measurement command
    uint8_t measure_cmd[] = {AHT10_CMD_MEASURE, 0x33, 0x00};
    esp_err_t ret = i2c_master_write_to_device(I2C_MASTER_NUM, sensor_addr, measure_cmd, 3, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 measurement command failed for address 0x%02X", sensor_addr);
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(80)); // Wait for measurement
    
    // Read 6 bytes of data
    uint8_t data[6];
    ret = i2c_master_read_from_device(I2C_MASTER_NUM, sensor_addr, data, 6, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 read data failed for address 0x%02X", sensor_addr);
        return ret;
    }
    
    // Check if measurement is ready
    if (data[0] & 0x80) {
        ESP_LOGE(TAG, "AHT10 measurement not ready for address 0x%02X", sensor_addr);
        return ESP_ERR_INVALID_STATE;
    }
    
    // Calculate humidity (20-bit value)
    uint32_t humidity_raw = ((uint32_t)(data[1] & 0x0F) << 16) | ((uint32_t)data[2] << 8) | data[3];
    *humidity = (float)humidity_raw * 100.0 / 1048576.0; // Convert to percentage
    
    // Calculate temperature (20-bit value)
    uint32_t temp_raw = ((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];
    *temperature = (float)temp_raw * 200.0 / 1048576.0 - 50.0; // Convert to Celsius
    
    return ESP_OK;
}

// Scan for I2C devices
void scan_i2c_devices(void)
{
    ESP_LOGI(TAG, "Scanning I2C bus for devices...");
    int found_devices = 0;
    
    for (int i = 0; i < 128; i++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Found I2C device at address: 0x%02X", i);
            found_devices++;
            
            // Check if it's an AHT10 sensor
            if (i == 0x38 || i == 0x39) {
                ESP_LOGI(TAG, "  -> This looks like an AHT10 sensor!");
            }
        }
    }
    
    ESP_LOGI(TAG, "I2C scan complete! Found %d devices", found_devices);
    ESP_LOGI(TAG, "Expected AHT10 addresses: 0x38, 0x39");
}

// Main test task
void aht10_test_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Starting AHT10 sensor test...");
    
    // Scan for devices first
    scan_i2c_devices();
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Try to initialize sensors
    esp_err_t ret1 = aht10_init(AHT10_SENSOR_1_ADDR);
    esp_err_t ret2 = aht10_init(AHT10_SENSOR_2_ADDR);
    
    if (ret1 != ESP_OK && ret2 != ESP_OK) {
        ESP_LOGE(TAG, "No AHT10 sensors found or initialized!");
        ESP_LOGI(TAG, "Please check:");
        ESP_LOGI(TAG, "1. Sensor wiring (SDA→GPIO21, SCL→GPIO22)");
        ESP_LOGI(TAG, "2. Power connections (VCC→3.3V, GND→GND)");
        ESP_LOGI(TAG, "3. Sensor orientation");
        return;
    }
    
    ESP_LOGI(TAG, "AHT10 sensors initialized successfully!");
    ESP_LOGI(TAG, "Starting continuous readings...");
    ESP_LOGI(TAG, "=======================================");
    
    int reading_count = 0;
    
    while (1) {
        reading_count++;
        ESP_LOGI(TAG, "Reading #%d:", reading_count);
        
        // Try to read from first sensor
        float temp1 = 0.0, hum1 = 0.0;
        esp_err_t ret = aht10_read_sensor(AHT10_SENSOR_1_ADDR, &temp1, &hum1);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "  Sensor 1 (0x%02X): Temperature: %.2f°C, Humidity: %.2f%%", 
                     AHT10_SENSOR_1_ADDR, temp1, hum1);
        } else {
            ESP_LOGE(TAG, "  Sensor 1 (0x%02X): Failed to read", AHT10_SENSOR_1_ADDR);
        }
        
        // Try to read from second sensor
        float temp2 = 0.0, hum2 = 0.0;
        ret = aht10_read_sensor(AHT10_SENSOR_2_ADDR, &temp2, &hum2);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "  Sensor 2 (0x%02X): Temperature: %.2f°C, Humidity: %.2f%%", 
                     AHT10_SENSOR_2_ADDR, temp2, hum2);
        } else {
            ESP_LOGE(TAG, "  Sensor 2 (0x%02X): Failed to read", AHT10_SENSOR_2_ADDR);
        }
        
        // Calculate averages if both sensors work
        if (temp1 != 0.0 && temp2 != 0.0) {
            float avg_temp = (temp1 + temp2) / 2.0;
            float avg_hum = (hum1 + hum2) / 2.0;
            ESP_LOGI(TAG, "  Average: Temperature: %.2f°C, Humidity: %.2f%%", avg_temp, avg_hum);
        }
        
        ESP_LOGI(TAG, "=======================================");
        vTaskDelay(pdMS_TO_TICKS(5000)); // Read every 5 seconds
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "AHT10 Simple Test for ESP32-C6");
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "Using ESP32's built-in 3.3V power");
    ESP_LOGI(TAG, "SDA Pin: GPIO %d", I2C_MASTER_SDA_IO);
    ESP_LOGI(TAG, "SCL Pin: GPIO %d", I2C_MASTER_SCL_IO);
    ESP_LOGI(TAG, "=================================");
    
    // Initialize I2C
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "I2C initialized successfully");
    
    // Create test task
    xTaskCreate(&aht10_test_task, "aht10_test", 4096, NULL, 5, NULL);
} 