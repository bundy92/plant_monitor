/**
 * @file main.cpp
 * @brief Single resistor I2C diagnostic test for ESP32-C6
 * 
 * This module provides comprehensive I2C bus testing with limited hardware
 * (single external resistor). It tests different I2C configurations, frequencies,
 * and pull-up strengths to diagnose communication issues with AHT10 sensors.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_err.h>

static const char *TAG = "SINGLE_RESISTOR_TEST";

// I2C Configuration constants
#define I2C_MASTER_SCL_IO            GPIO_NUM_22    /**< SCL pin for I2C communication */
#define I2C_MASTER_SDA_IO            GPIO_NUM_21    /**< SDA pin for I2C communication */
#define I2C_MASTER_NUM               I2C_NUM_0      /**< I2C port number */
#define I2C_MASTER_TIMEOUT_MS        1000           /**< I2C timeout in milliseconds */

// Test frequency configurations
#define FREQ_10KHZ    10000   /**< Very slow frequency for testing */
#define FREQ_50KHZ    50000   /**< Slow frequency for testing */
#define FREQ_100KHZ   100000  /**< Standard I2C frequency */
#define FREQ_400KHZ   400000  /**< Fast I2C frequency */

// AHT10 sensor addresses
#define AHT10_ADDR_1  0x38    /**< First AHT10 sensor address */
#define AHT10_ADDR_2  0x39    /**< Second AHT10 sensor address */

/**
 * @brief Initialize I2C with specific frequency and pull-up strength
 * 
 * This function initializes the I2C master with the specified frequency and
 * pull-up configuration. It can apply additional GPIO pull-up strengthening
 * if requested.
 * 
 * @param freq I2C frequency in Hz
 * @param strong_pullup Whether to apply additional GPIO pull-up strengthening
 * @return ESP_OK on success, error code on failure
 * 
 * @note This function will delete any existing I2C driver before initializing
 * @see i2c_config_t
 */
esp_err_t i2c_master_init_with_config(uint32_t freq, bool strong_pullup) 
{
    ESP_LOGI(TAG, "Initializing I2C at %d Hz, strong pullup: %s", 
             freq, strong_pullup ? "YES" : "NO");
    
    // Configure I2C parameters
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
    
    // Configure I2C parameters
    esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C parameter config failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Install I2C driver
    ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Apply additional GPIO pull-up strengthening if requested
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

/**
 * @brief Test I2C bus with manual GPIO control
 * 
 * This function tests the I2C bus by manually controlling the SDA and SCL lines
 * to check if they can be set HIGH and remain HIGH. This helps diagnose if
 * sensors are pulling the lines LOW, which would indicate a communication issue.
 * 
 * @return ESP_OK if lines can be set HIGH, ESP_FAIL if lines are pulled LOW
 * 
 * @note This test helps identify if the issue is with pull-up resistors or
 *       sensor voltage levels
 */
esp_err_t test_i2c_manual_control(void) 
{
    ESP_LOGI(TAG, "Testing I2C with manual control...");
    
    // Configure pins as outputs for manual control
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
    
    // Set both lines HIGH manually
    gpio_set_level(I2C_MASTER_SDA_IO, 1);
    gpio_set_level(I2C_MASTER_SCL_IO, 1);
    ESP_LOGI(TAG, "Set SDA and SCL HIGH manually");
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Check if lines are still HIGH (not pulled LOW by sensor)
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

/**
 * @brief Scan I2C bus with retry mechanism
 * 
 * This function scans the entire I2C address space (0-127) with a retry mechanism
 * to detect devices. It provides detailed logging for AHT10 sensor addresses
 * and gives specific recommendations for single resistor setups.
 * 
 * @note This function will attempt each address up to 3 times before giving up
 */
void scan_i2c_with_retry(void) 
{
    ESP_LOGI(TAG, "Scanning I2C bus with retry mechanism...");
    int found_devices = 0;
    
    // Scan all possible I2C addresses
    for (int i = 0; i < 128; i++) {
        bool found = false;
        
        // Try each address up to 3 times
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
                
                // Check if this looks like an AHT10 sensor
                if (i == AHT10_ADDR_1 || i == AHT10_ADDR_2) {
                    ESP_LOGI(TAG, "   -> This looks like an AHT10 sensor!");
                }
                break;
            }
            
            // Small delay between retries
            if (retry < 2) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
        
        // Log if AHT10 sensors were not found
        if (!found && (i == AHT10_ADDR_1 || i == AHT10_ADDR_2)) {
            ESP_LOGW(TAG, "❌ AHT10 sensor at 0x%02X not found", i);
        }
    }
    
    ESP_LOGI(TAG, "I2C scan complete! Found %d devices", found_devices);
    
    // Provide recommendations if no devices found
    if (found_devices == 0) {
        ESP_LOGE(TAG, "❌ No I2C devices found!");
        ESP_LOGI(TAG, "With your single resistor, try:");
        ESP_LOGI(TAG, "1. Connect resistor between SDA and 3.3V");
        ESP_LOGI(TAG, "2. Try 5V power for the sensor");
        ESP_LOGI(TAG, "3. Check sensor pinout and orientation");
    }
}

/**
 * @brief Test different I2C configurations
 * 
 * This function systematically tests different I2C frequencies and pull-up
 * configurations to find the optimal setup for the available hardware.
 * It tests combinations of frequencies (10kHz, 50kHz, 100kHz, 400kHz) and
 * pull-up strengths (normal, strong).
 * 
 * @note Each configuration is tested with manual control and device scanning
 */
void test_different_configs(void) 
{
    // Test frequencies and their names
    uint32_t frequencies[] = {FREQ_10KHZ, FREQ_50KHZ, FREQ_100KHZ, FREQ_400KHZ};
    const char* freq_names[] = {"10kHz", "50kHz", "100kHz", "400kHz"};
    
    // Test pull-up configurations and their names
    bool pullup_configs[] = {false, true};
    const char* pullup_names[] = {"Normal", "Strong"};
    
    ESP_LOGI(TAG, "Testing different I2C configurations...");
    
    // Test all combinations of frequency and pull-up strength
    for (int freq_idx = 0; freq_idx < 4; freq_idx++) {
        for (int pullup_idx = 0; pullup_idx < 2; pullup_idx++) {
            ESP_LOGI(TAG, "--- Testing %s at %s pullup ---", 
                     freq_names[freq_idx], pullup_names[pullup_idx]);
            
            // Uninstall previous driver to start fresh
            i2c_driver_delete(I2C_MASTER_NUM);
            vTaskDelay(pdMS_TO_TICKS(100));
            
            // Initialize with new configuration
            esp_err_t ret = i2c_master_init_with_config(frequencies[freq_idx], pullup_configs[pullup_idx]);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to initialize I2C");
                continue;
            }
            
            // Test manual control first
            test_i2c_manual_control();
            
            // Scan for devices with this configuration
            scan_i2c_with_retry();
            
            // Delay between configurations
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
    }
}

/**
 * @brief Main test task for single resistor I2C testing
 * 
 * This task orchestrates the complete single resistor I2C diagnostic test.
 * It performs manual control testing followed by systematic configuration
 * testing to help diagnose I2C communication issues with limited hardware.
 * 
 * @param pvParameters Task parameters (unused)
 * 
 * @note This task provides comprehensive diagnostics and recommendations
 *       for single resistor setups
 */
void single_resistor_test_task(void *pvParameters) 
{
    ESP_LOGI(TAG, "Starting single resistor test...");
    
    // Test manual control first to check basic line levels
    test_i2c_manual_control();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Test different I2C configurations systematically
    test_different_configs();
    
    ESP_LOGI(TAG, "Single resistor test complete!");
    ESP_LOGI(TAG, "If nothing works, try:");
    ESP_LOGI(TAG, "1. Connect your resistor: SDA → 3.3V");
    ESP_LOGI(TAG, "2. Try 5V power for sensor");
    ESP_LOGI(TAG, "3. Check sensor pinout and orientation");
}

/**
 * @brief Main application entry point
 * 
 * This function initializes the single resistor I2C diagnostic test.
 * It creates the test task and provides information about the test configuration.
 * 
 * @return void
 */
extern "C" void app_main(void) 
{
    ESP_LOGI(TAG, "Single Resistor I2C Test for ESP32-C6");
    ESP_LOGI(TAG, "=====================================");
    ESP_LOGI(TAG, "SDA Pin: GPIO %d", I2C_MASTER_SDA_IO);
    ESP_LOGI(TAG, "SCL Pin: GPIO %d", I2C_MASTER_SCL_IO);
    ESP_LOGI(TAG, "=====================================");
    ESP_LOGI(TAG, "This test will try different configurations");
    ESP_LOGI(TAG, "to work with limited hardware.");
    
    // Create the main test task
    xTaskCreate(&single_resistor_test_task, "single_test", 4096, NULL, 5, NULL);
} 