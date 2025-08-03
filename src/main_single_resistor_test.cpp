#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_err.h>

static const char *TAG = "SINGLE_RESISTOR_TEST";

// I2C Configuration
#define I2C_MASTER_SCL_IO            GPIO_NUM_22
#define I2C_MASTER_SDA_IO            GPIO_NUM_21
#define I2C_MASTER_NUM               I2C_NUM_0
#define I2C_MASTER_TIMEOUT_MS        1000

// Test different configurations
#define FREQ_10KHZ    10000   // Very slow for testing
#define FREQ_50KHZ    50000   // Slow
#define FREQ_100KHZ   100000  // Standard
#define FREQ_400KHZ   400000  // Fast

// Initialize I2C with specific frequency and pull-up strength
esp_err_t i2c_master_init_with_config(uint32_t freq, bool strong_pullup) {
    ESP_LOGI(TAG, "Initializing I2C at %d Hz, strong pullup: %s", freq, strong_pullup ? "YES" : "NO");
    
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
    
    // Try to strengthen pull-ups if needed
    if (strong_pullup) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << I2C_MASTER_SDA_IO) | (1ULL << I2C_MASTER_SCL_IO),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&io_conf);
        ESP_LOGI(TAG, "Applied additional GPIO pull-up configuration");
    }
    
    ESP_LOGI(TAG, "I2C initialized successfully");
    return ESP_OK;
}

// Test I2C bus with manual control
esp_err_t test_i2c_manual_control(void) {
    ESP_LOGI(TAG, "Testing I2C with manual control...");
    
    // Configure pins as outputs first
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << I2C_MASTER_SDA_IO) | (1ULL << I2C_MASTER_SCL_IO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Set both lines HIGH
    gpio_set_level(I2C_MASTER_SDA_IO, 1);
    gpio_set_level(I2C_MASTER_SCL_IO, 1);
    ESP_LOGI(TAG, "Set SDA and SCL HIGH manually");
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Check if lines are still HIGH
    int sda_level = gpio_get_level(I2C_MASTER_SDA_IO);
    int scl_level = gpio_get_level(I2C_MASTER_SCL_IO);
    
    ESP_LOGI(TAG, "SDA level: %s, SCL level: %s", 
             sda_level ? "HIGH" : "LOW", 
             scl_level ? "HIGH" : "LOW");
    
    if (sda_level == 0 || scl_level == 0) {
        ESP_LOGW(TAG, "⚠️  Lines are being pulled LOW by sensor!");
        ESP_LOGI(TAG, "This suggests the sensor is working but needs:");
        ESP_LOGI(TAG, "1. Stronger pull-up resistors");
        ESP_LOGI(TAG, "2. Different voltage level (5V instead of 3.3V)");
        ESP_LOGI(TAG, "3. Different I2C frequency");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "✅ Lines are HIGH - sensor is not pulling them LOW");
    return ESP_OK;
}

// Scan I2C bus with retry mechanism
void scan_i2c_with_retry(void) {
    ESP_LOGI(TAG, "Scanning I2C bus with retry mechanism...");
    int found_devices = 0;
    
    for (int i = 0; i < 128; i++) {
        // Try multiple times for each address
        bool found = false;
        for (int retry = 0; retry < 3; retry++) {
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, true);
            i2c_master_stop(cmd);
            
            esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 2000 / portTICK_PERIOD_MS);
            i2c_cmd_link_delete(cmd);
            
            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "✅ Found I2C device at address: 0x%02X (retry %d)", i, retry + 1);
                found_devices++;
                found = true;
                
                if (i == 0x38 || i == 0x39) {
                    ESP_LOGI(TAG, "   -> This looks like an AHT10 sensor!");
                }
                break;
            }
            
            if (retry < 2) {
                vTaskDelay(pdMS_TO_TICKS(10)); // Small delay between retries
            }
        }
        
        if (!found && (i == 0x38 || i == 0x39)) {
            ESP_LOGW(TAG, "❌ AHT10 sensor at 0x%02X not found", i);
        }
    }
    
    ESP_LOGI(TAG, "I2C scan complete! Found %d devices", found_devices);
    
    if (found_devices == 0) {
        ESP_LOGE(TAG, "❌ No I2C devices found!");
        ESP_LOGI(TAG, "With your single resistor, try:");
        ESP_LOGI(TAG, "1. Connect resistor between SDA and 3.3V");
        ESP_LOGI(TAG, "2. Try 5V power for the sensor");
        ESP_LOGI(TAG, "3. Check sensor pinout");
    }
}

// Test different configurations
void test_different_configs(void) {
    uint32_t frequencies[] = {FREQ_10KHZ, FREQ_50KHZ, FREQ_100KHZ, FREQ_400KHZ};
    const char* freq_names[] = {"10kHz", "50kHz", "100kHz", "400kHz"};
    bool pullup_configs[] = {false, true};
    const char* pullup_names[] = {"Normal", "Strong"};
    
    ESP_LOGI(TAG, "Testing different I2C configurations...");
    
    for (int freq_idx = 0; freq_idx < 4; freq_idx++) {
        for (int pullup_idx = 0; pullup_idx < 2; pullup_idx++) {
            ESP_LOGI(TAG, "--- Testing %s at %s pullup ---", 
                     freq_names[freq_idx], pullup_names[pullup_idx]);
            
            // Uninstall previous driver
            i2c_driver_delete(I2C_MASTER_NUM);
            vTaskDelay(pdMS_TO_TICKS(100));
            
            // Initialize with new config
            esp_err_t ret = i2c_master_init_with_config(frequencies[freq_idx], pullup_configs[pullup_idx]);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to initialize I2C");
                continue;
            }
            
            // Test manual control
            test_i2c_manual_control();
            
            // Scan for devices
            scan_i2c_with_retry();
            
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
    }
}

// Main test task
void single_resistor_test_task(void *pvParameters) {
    ESP_LOGI(TAG, "Starting single resistor test...");
    
    // Test manual control first
    test_i2c_manual_control();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Test different configurations
    test_different_configs();
    
    ESP_LOGI(TAG, "Single resistor test complete!");
    ESP_LOGI(TAG, "If nothing works, try:");
    ESP_LOGI(TAG, "1. Connect your resistor: SDA → 3.3V");
    ESP_LOGI(TAG, "2. Try 5V power for sensor");
    ESP_LOGI(TAG, "3. Check sensor pinout and orientation");
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Single Resistor I2C Test for ESP32-C6");
    ESP_LOGI(TAG, "=====================================");
    ESP_LOGI(TAG, "SDA Pin: GPIO %d", I2C_MASTER_SDA_IO);
    ESP_LOGI(TAG, "SCL Pin: GPIO %d", I2C_MASTER_SCL_IO);
    ESP_LOGI(TAG, "=====================================");
    ESP_LOGI(TAG, "This test will try different configurations");
    ESP_LOGI(TAG, "to work with limited hardware.");
    
    // Create test task
    xTaskCreate(&single_resistor_test_task, "single_test", 4096, NULL, 5, NULL);
} 