#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_err.h>

#define I2C_MASTER_SCL_IO            GPIO_NUM_22
#define I2C_MASTER_SDA_IO            GPIO_NUM_21
#define I2C_MASTER_NUM               I2C_NUM_0
#define I2C_MASTER_FREQ_HZ           100000
#define I2C_MASTER_TIMEOUT_MS        1000

static const char *TAG = "I2C_SCANNER";

esp_err_t i2c_master_init(void)
{
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
    
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        return err;
    }
    
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void scan_i2c_devices(void)
{
    ESP_LOGI(TAG, "Starting I2C scan...");
    ESP_LOGI(TAG, "SDA Pin: %d, SCL Pin: %d", I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
    ESP_LOGI(TAG, "Scanning addresses 0x03 to 0x77...");
    ESP_LOGI(TAG, "=======================================");
    
    int found_devices = 0;
    
    for (int i = 3; i < 127; i++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Found I2C device at address: 0x%02X (%d)", i, i);
            found_devices++;
            
            // Check if it's an AHT10 sensor
            if (i == 0x38 || i == 0x39) {
                ESP_LOGI(TAG, "  -> This looks like an AHT10 sensor!");
            }
        }
    }
    
    ESP_LOGI(TAG, "=======================================");
    ESP_LOGI(TAG, "I2C scan complete! Found %d devices", found_devices);
    
    if (found_devices == 0) {
        ESP_LOGE(TAG, "No I2C devices found!");
        ESP_LOGI(TAG, "Please check:");
        ESP_LOGI(TAG, "1. Sensor wiring (SDA→GPIO21, SCL→GPIO22)");
        ESP_LOGI(TAG, "2. Power connections (VCC→3.3V, GND→GND)");
        ESP_LOGI(TAG, "3. Sensor orientation");
        ESP_LOGI(TAG, "4. Try external pull-up resistors (4.7kΩ)");
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Simple I2C Scanner for ESP32-C6");
    ESP_LOGI(TAG, "=================================");
    
    // Initialize I2C
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "I2C initialized successfully");
    
    // Scan for devices
    scan_i2c_devices();
    
    // Keep running to show the scan results
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "Scanner still running... Press reset to scan again");
    }
} 