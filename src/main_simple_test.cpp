#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_err.h>

static const char *TAG = "SIMPLE_TEST";

// I2C Configuration
#define I2C_MASTER_SCL_IO            GPIO_NUM_22
#define I2C_MASTER_SDA_IO            GPIO_NUM_21
#define I2C_MASTER_NUM               I2C_NUM_0
#define I2C_MASTER_FREQ_HZ           50000
#define I2C_MASTER_TIMEOUT_MS        1000

// AHT10 sensor addresses
#define AHT10_SENSOR_1_ADDR          0x38
#define AHT10_SENSOR_2_ADDR          0x39

// Initialize I2C once
esp_err_t i2c_master_init(void) {
    ESP_LOGI(TAG, "Initializing I2C master...");
    
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = I2C_MASTER_FREQ_HZ,
        },
    };
    
    esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C parameter config failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "I2C initialized successfully");
    return ESP_OK;
}

// Scan for I2C devices
void scan_i2c_devices(void) {
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
            if (i == AHT10_SENSOR_1_ADDR || i == AHT10_SENSOR_2_ADDR) {
                ESP_LOGI(TAG, "  -> This looks like an AHT10 sensor!");
            }
        }
    }
    
    ESP_LOGI(TAG, "I2C scan complete! Found %d devices", found_devices);
    
    if (found_devices == 0) {
        ESP_LOGE(TAG, "No I2C devices found!");
        ESP_LOGI(TAG, "Please check:");
        ESP_LOGI(TAG, "1. Sensor wiring (SDA→GPIO21, SCL→GPIO22)");
        ESP_LOGI(TAG, "2. Power connections (VCC→3.3V, GND→GND)");
        ESP_LOGI(TAG, "3. Sensor orientation");
    }
}

// Test AHT10 sensor
esp_err_t test_aht10_sensor(uint8_t sensor_addr) {
    ESP_LOGI(TAG, "Testing AHT10 sensor at address 0x%02X", sensor_addr);
    
    // Send soft reset command
    uint8_t reset_cmd[] = {0xBA}; // AHT10 soft reset
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (sensor_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, reset_cmd, 1, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 reset failed for address 0x%02X", sensor_addr);
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(20)); // Wait for reset
    
    // Send initialization command
    uint8_t init_cmd[] = {0xE1, 0x08, 0x00};
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (sensor_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, init_cmd, 3, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 initialization failed for address 0x%02X", sensor_addr);
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(10)); // Wait for initialization
    
    ESP_LOGI(TAG, "AHT10 sensor at address 0x%02X initialized successfully", sensor_addr);
    return ESP_OK;
}

// Main test task
void simple_test_task(void *pvParameters) {
    ESP_LOGI(TAG, "Starting simple AHT10 test...");
    
    // Initialize I2C first
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C initialization failed!");
        return;
    }
    
    // Scan for devices
    scan_i2c_devices();
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Test sensors
    esp_err_t ret1 = test_aht10_sensor(AHT10_SENSOR_1_ADDR);
    esp_err_t ret2 = test_aht10_sensor(AHT10_SENSOR_2_ADDR);
    
    if (ret1 != ESP_OK && ret2 != ESP_OK) {
        ESP_LOGE(TAG, "No AHT10 sensors could be initialized!");
        return;
    }
    
    ESP_LOGI(TAG, "AHT10 sensor test completed!");
    ESP_LOGI(TAG, "Sensor 1 (0x%02X): %s", AHT10_SENSOR_1_ADDR, 
             (ret1 == ESP_OK) ? "OK" : "FAILED");
    ESP_LOGI(TAG, "Sensor 2 (0x%02X): %s", AHT10_SENSOR_2_ADDR, 
             (ret2 == ESP_OK) ? "OK" : "FAILED");
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Simple AHT10 Test for ESP32-C6");
    ESP_LOGI(TAG, "===============================");
    ESP_LOGI(TAG, "SDA Pin: GPIO %d", I2C_MASTER_SDA_IO);
    ESP_LOGI(TAG, "SCL Pin: GPIO %d", I2C_MASTER_SCL_IO);
    ESP_LOGI(TAG, "I2C Frequency: %d Hz", I2C_MASTER_FREQ_HZ);
    ESP_LOGI(TAG, "===============================");
    
    // Create test task
    xTaskCreate(&simple_test_task, "simple_test", 4096, NULL, 5, NULL);
} 