#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_timer.h>
#include "sensors/aht10.h"

static const char *TAG = "PLANT_MONITOR";

// Sensor configurations
static aht10_config_t sensor1_config;
static aht10_config_t sensor2_config;

// Sensor data
static aht10_data_t sensor1_data;
static aht10_data_t sensor2_data;

// Task for sensor reading
void sensor_task(void *pvParameters) {
    ESP_LOGI(TAG, "Sensor task started");
    
    while (1) {
        // Read from first sensor
        esp_err_t ret1 = aht10_read_sensor(&sensor1_config, &sensor1_data);
        if (ret1 == ESP_OK) {
            esp_err_t validate1 = aht10_validate_data(&sensor1_data);
            if (validate1 == ESP_OK) {
                ESP_LOGI(TAG, "Sensor 1: Temperature: %.2f°C, Humidity: %.2f%%", 
                         sensor1_data.temperature, sensor1_data.humidity);
            } else {
                ESP_LOGW(TAG, "Sensor 1 data validation failed");
            }
        } else {
            ESP_LOGE(TAG, "Sensor 1 read failed: %s", esp_err_to_name(ret1));
        }
        
        // Read from second sensor
        esp_err_t ret2 = aht10_read_sensor(&sensor2_config, &sensor2_data);
        if (ret2 == ESP_OK) {
            esp_err_t validate2 = aht10_validate_data(&sensor2_data);
            if (validate2 == ESP_OK) {
                ESP_LOGI(TAG, "Sensor 2: Temperature: %.2f°C, Humidity: %.2f%%", 
                         sensor2_data.temperature, sensor2_data.humidity);
            } else {
                ESP_LOGW(TAG, "Sensor 2 data validation failed");
            }
        } else {
            ESP_LOGE(TAG, "Sensor 2 read failed: %s", esp_err_to_name(ret2));
        }
        
        // Calculate averages if both sensors work
        if (ret1 == ESP_OK && ret2 == ESP_OK && 
            aht10_validate_data(&sensor1_data) == ESP_OK && 
            aht10_validate_data(&sensor2_data) == ESP_OK) {
            
            float avg_temp = (sensor1_data.temperature + sensor2_data.temperature) / 2.0;
            float avg_hum = (sensor1_data.humidity + sensor2_data.humidity) / 2.0;
            
            ESP_LOGI(TAG, "Average: Temperature: %.2f°C, Humidity: %.2f%%", avg_temp, avg_hum);
        }
        
        ESP_LOGI(TAG, "=======================================");
        vTaskDelay(pdMS_TO_TICKS(5000)); // Read every 5 seconds
    }
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "ESP32-C6 Plant Monitor - Clean Version");
    ESP_LOGI(TAG, "=======================================");
    
    // Initialize sensor configurations
    esp_err_t ret1 = aht10_get_default_config(&sensor1_config, AHT10_ADDR_1);
    esp_err_t ret2 = aht10_get_default_config(&sensor2_config, AHT10_ADDR_2);
    
    if (ret1 != ESP_OK || ret2 != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get default configurations");
        return;
    }
    
    // Initialize first sensor (this will also initialize I2C)
    ESP_LOGI(TAG, "Initializing sensors...");
    esp_err_t init1 = aht10_init(&sensor1_config);
    
    // Scan for I2C devices after I2C is initialized
    ESP_LOGI(TAG, "Scanning for I2C devices...");
    esp_err_t scan_ret = aht10_scan_devices(I2C_NUM_0);
    if (scan_ret != ESP_OK) {
        ESP_LOGW(TAG, "I2C scan failed or no devices found");
    }
    
    // Initialize second sensor
    esp_err_t init2 = aht10_init(&sensor2_config);
    
    if (init1 != ESP_OK && init2 != ESP_OK) {
        ESP_LOGE(TAG, "No AHT10 sensors could be initialized!");
        ESP_LOGI(TAG, "Please check:");
        ESP_LOGI(TAG, "1. Sensor wiring (SDA→GPIO21, SCL→GPIO22)");
        ESP_LOGI(TAG, "2. Power connections (VCC→3.3V, GND→GND)");
        ESP_LOGI(TAG, "3. Sensor orientation");
        return;
    }
    
    if (init1 == ESP_OK) {
        ESP_LOGI(TAG, "Sensor 1 (0x%02X) initialized successfully", sensor1_config.sensor_addr);
    }
    
    if (init2 == ESP_OK) {
        ESP_LOGI(TAG, "Sensor 2 (0x%02X) initialized successfully", sensor2_config.sensor_addr);
    }
    
    ESP_LOGI(TAG, "Starting sensor monitoring...");
    ESP_LOGI(TAG, "=======================================");
    
    // Create sensor task
    xTaskCreate(&sensor_task, "sensor_task", 4096, NULL, 5, NULL);
} 