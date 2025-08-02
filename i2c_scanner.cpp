#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2c.h>
#include <esp_log.h>
#include <esp_err.h>

// I2C Configuration
#define I2C_MASTER_SCL_IO            GPIO_NUM_22      // I2C SCL pin
#define I2C_MASTER_SDA_IO            GPIO_NUM_21      // I2C SDA pin
#define I2C_MASTER_NUM               I2C_NUM_0        // I2C port number
#define I2C_MASTER_FREQ_HZ           100000           // I2C master clock frequency
#define I2C_MASTER_TIMEOUT_MS        1000             // I2C master timeout

static const char *TAG = "I2C_SCANNER";

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

// Scan I2C devices
void i2c_scanner_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Starting I2C Scanner...");
    ESP_LOGI(TAG, "Scanning I2C bus for devices...");
    
    for (int i = 0; i < 128; i++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Found I2C device at address: 0x%02X", i);
            
            // Check if it's an AHT10 sensor
            if (i == 0x38 || i == 0x39) {
                ESP_LOGI(TAG, "  -> This looks like an AHT10 sensor!");
            }
        }
    }
    
    ESP_LOGI(TAG, "I2C scan complete!");
    ESP_LOGI(TAG, "Expected AHT10 addresses: 0x38, 0x39");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "I2C Scanner for ESP32-C6 Plant Monitor");
    ESP_LOGI(TAG, "=======================================");
    
    // Initialize I2C
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "I2C initialized successfully");
    ESP_LOGI(TAG, "SDA Pin: GPIO %d", I2C_MASTER_SDA_IO);
    ESP_LOGI(TAG, "SCL Pin: GPIO %d", I2C_MASTER_SCL_IO);
    
    // Create scanner task
    xTaskCreate(&i2c_scanner_task, "i2c_scanner", 4096, NULL, 5, NULL);
} 