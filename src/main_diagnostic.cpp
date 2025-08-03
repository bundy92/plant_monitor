#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_err.h>

static const char *TAG = "I2C_DIAG";

// I2C Configuration
#define I2C_MASTER_SCL_IO            GPIO_NUM_22
#define I2C_MASTER_SDA_IO            GPIO_NUM_21
#define I2C_MASTER_NUM               I2C_NUM_0
#define I2C_MASTER_FREQ_HZ           50000
#define I2C_MASTER_TIMEOUT_MS        1000

// Test different frequencies
#define FREQ_50KHZ    50000
#define FREQ_100KHZ   100000
#define FREQ_400KHZ   400000

// Initialize I2C with specific frequency
esp_err_t i2c_master_init_with_freq(uint32_t freq) {
    ESP_LOGI(TAG, "Initializing I2C master at %d Hz...", freq);
    
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = freq,
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
    
    ESP_LOGI(TAG, "I2C initialized successfully at %d Hz", freq);
    return ESP_OK;
}

// Test I2C bus health
esp_err_t test_i2c_bus_health(void) {
    ESP_LOGI(TAG, "Testing I2C bus health...");
    
    // Test SDA and SCL lines
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << I2C_MASTER_SDA_IO) | (1ULL << I2C_MASTER_SCL_IO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Read SDA and SCL levels
    int sda_level = gpio_get_level(I2C_MASTER_SDA_IO);
    int scl_level = gpio_get_level(I2C_MASTER_SCL_IO);
    
    ESP_LOGI(TAG, "SDA (GPIO %d) level: %s", I2C_MASTER_SDA_IO, sda_level ? "HIGH" : "LOW");
    ESP_LOGI(TAG, "SCL (GPIO %d) level: %s", I2C_MASTER_SCL_IO, scl_level ? "HIGH" : "LOW");
    
    if (sda_level == 0 || scl_level == 0) {
        ESP_LOGW(TAG, "⚠️  WARNING: SDA or SCL is LOW - this indicates a problem!");
        ESP_LOGI(TAG, "Possible causes:");
        ESP_LOGI(TAG, "1. Sensor is pulling the line LOW (short circuit)");
        ESP_LOGI(TAG, "2. Missing pull-up resistors");
        ESP_LOGI(TAG, "3. Wrong voltage level (sensor needs 5V instead of 3.3V)");
        ESP_LOGI(TAG, "4. Incorrect wiring");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "✅ SDA and SCL are HIGH - bus looks healthy");
    return ESP_OK;
}

// Scan I2C bus with detailed error reporting
void scan_i2c_bus_detailed(void) {
    ESP_LOGI(TAG, "Scanning I2C bus with detailed error reporting...");
    int found_devices = 0;
    int total_errors = 0;
    
    for (int i = 0; i < 128; i++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "✅ Found I2C device at address: 0x%02X", i);
            found_devices++;
            
            // Check if it's an AHT10 sensor
            if (i == 0x38 || i == 0x39) {
                ESP_LOGI(TAG, "   -> This looks like an AHT10 sensor!");
            }
        } else {
            total_errors++;
            if (total_errors <= 5) { // Only show first 5 errors to avoid spam
                ESP_LOGW(TAG, "❌ Address 0x%02X: %s", i, esp_err_to_name(ret));
            }
        }
    }
    
    ESP_LOGI(TAG, "I2C scan complete! Found %d devices, %d errors", found_devices, total_errors);
    
    if (found_devices == 0) {
        ESP_LOGE(TAG, "❌ No I2C devices found!");
        ESP_LOGI(TAG, "Troubleshooting steps:");
        ESP_LOGI(TAG, "1. Check sensor power (VCC→3.3V or 5V, GND→GND)");
        ESP_LOGI(TAG, "2. Verify wiring (SDA→GPIO21, SCL→GPIO22)");
        ESP_LOGI(TAG, "3. Try external pull-up resistors (4.7kΩ to 3.3V)");
        ESP_LOGI(TAG, "4. Check sensor orientation and pinout");
        ESP_LOGI(TAG, "5. Try different I2C frequency");
    }
}

// Test different I2C frequencies
void test_i2c_frequencies(void) {
    uint32_t frequencies[] = {FREQ_50KHZ, FREQ_100KHZ, FREQ_400KHZ};
    const char* freq_names[] = {"50kHz", "100kHz", "400kHz"};
    
    ESP_LOGI(TAG, "Testing different I2C frequencies...");
    
    for (int i = 0; i < 3; i++) {
        ESP_LOGI(TAG, "--- Testing %s ---", freq_names[i]);
        
        // Uninstall previous driver
        i2c_driver_delete(I2C_MASTER_NUM);
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // Initialize with new frequency
        esp_err_t ret = i2c_master_init_with_freq(frequencies[i]);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize I2C at %s", freq_names[i]);
            continue;
        }
        
        // Test bus health
        test_i2c_bus_health();
        
        // Scan for devices
        scan_i2c_bus_detailed();
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// Main diagnostic task
void diagnostic_task(void *pvParameters) {
    ESP_LOGI(TAG, "Starting I2C diagnostic...");
    
    // Test bus health first
    test_i2c_bus_health();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Initialize I2C
    esp_err_t ret = i2c_master_init_with_freq(I2C_MASTER_FREQ_HZ);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C initialization failed!");
        return;
    }
    
    // Scan with current frequency
    scan_i2c_bus_detailed();
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Test different frequencies
    test_i2c_frequencies();
    
    ESP_LOGI(TAG, "Diagnostic complete!");
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "I2C Diagnostic Tool for ESP32-C6");
    ESP_LOGI(TAG, "==================================");
    ESP_LOGI(TAG, "SDA Pin: GPIO %d", I2C_MASTER_SDA_IO);
    ESP_LOGI(TAG, "SCL Pin: GPIO %d", I2C_MASTER_SCL_IO);
    ESP_LOGI(TAG, "==================================");
    
    // Create diagnostic task
    xTaskCreate(&diagnostic_task, "diagnostic", 4096, NULL, 5, NULL);
} 