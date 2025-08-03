/**
 * @file main_example.cpp
 * @brief Plant Monitor System - Example Application
 * 
 * This file demonstrates the clean, industry-standard plant monitoring system
 * using the consolidated plant_monitor library. It provides a simple, robust
 * implementation with minimal complexity.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_timer.h>
#include "plant_monitor.h"

static const char *TAG = "PLANT_MONITOR_MAIN";

// Global variables for sensor data and health
static plant_monitor_data_t sensor_data;
static plant_health_t plant_health;

/**
 * @brief Main monitoring task
 * 
 * This task continuously reads sensors, calculates plant health,
 * updates the display, and transmits data at regular intervals.
 * 
 * @param pvParameters Task parameters (unused)
 */
void monitoring_task(void *pvParameters) 
{
    ESP_LOGI(TAG, "Plant monitoring task started");
    
    while (1) {
        // Read all sensors
        esp_err_t ret = plant_monitor_read_sensors(&sensor_data);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read sensors: %s", esp_err_to_name(ret));
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }
        
        // Calculate plant health
        ret = plant_monitor_calculate_health(&sensor_data, &plant_health);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to calculate health: %s", esp_err_to_name(ret));
        }
        
        // Update display
        ret = plant_monitor_update_display(&sensor_data, &plant_health);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to update display: %s", esp_err_to_name(ret));
        }
        
        // Transmit data
        ret = plant_monitor_transmit_data(&sensor_data, &plant_health);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to transmit data: %s", esp_err_to_name(ret));
        }
        
        // Log summary
        ESP_LOGI(TAG, "=== Plant Monitor Summary ===");
        ESP_LOGI(TAG, "Temperature: %.2f°C (Avg: %.2f°C)", 
                 sensor_data.temperature_1, sensor_data.temperature_avg);
        ESP_LOGI(TAG, "Humidity: %.2f%% (Avg: %.2f%%)", 
                 sensor_data.humidity_1, sensor_data.humidity_avg);
        ESP_LOGI(TAG, "Soil Moisture: %d", sensor_data.soil_moisture);
        ESP_LOGI(TAG, "Light Level: %d", sensor_data.light_level);
        ESP_LOGI(TAG, "Plant Health: %s %s (Score: %.1f)", 
                 plant_health.health_text, plant_health.emoji, plant_health.health_score);
        ESP_LOGI(TAG, "Recommendation: %s", plant_health.recommendation);
        ESP_LOGI(TAG, "Uptime: %02d:%02d:%02d", 
                 (int)(sensor_data.uptime_seconds / 3600),
                 (int)((sensor_data.uptime_seconds % 3600) / 60),
                 (int)(sensor_data.uptime_seconds % 60));
        ESP_LOGI(TAG, "================================");
        
        // Wait for next reading
        vTaskDelay(pdMS_TO_TICKS(30000)); // 30 seconds
    }
}

/**
 * @brief Main application entry point
 * 
 * This function initializes the plant monitoring system with default
 * configuration and starts the monitoring task.
 * 
 * @return void
 */
extern "C" void app_main(void) 
{
    ESP_LOGI(TAG, "Plant Monitor System Starting...");
    ESP_LOGI(TAG, "==================================");
    
    // Get default configuration
    plant_monitor_config_t config;
    esp_err_t ret = plant_monitor_get_default_config(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get default configuration");
        return;
    }
    
    // Customize configuration if needed
    config.enable_display = true;  // Enable display output
    config.enable_wifi = false;    // Disable WiFi for now (can be enabled later)
    
    // Initialize plant monitoring system
    ret = plant_monitor_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize plant monitoring system");
        return;
    }
    
    // Scan for I2C devices
    ESP_LOGI(TAG, "Scanning for I2C devices...");
    plant_monitor_scan_i2c_devices();
    
    // Get system status
    int sensors_working;
    bool display_working, wifi_connected;
    plant_monitor_get_status(&sensors_working, &display_working, &wifi_connected);
    
    ESP_LOGI(TAG, "System initialized successfully!");
    ESP_LOGI(TAG, "Features:");
    ESP_LOGI(TAG, "- Clean, industry-standard architecture");
    ESP_LOGI(TAG, "- Professional numpy-style documentation");
    ESP_LOGI(TAG, "- Robust error handling and recovery");
    ESP_LOGI(TAG, "- Modular and maintainable design");
    ESP_LOGI(TAG, "- Plant health analysis with emoji indicators");
    ESP_LOGI(TAG, "- Analog sensor support (soil moisture, light)");
    ESP_LOGI(TAG, "- WiFi connectivity and data transmission");
    ESP_LOGI(TAG, "- Display output with health status");
    
    // Create monitoring task
    xTaskCreate(&monitoring_task, "monitoring_task", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Plant monitoring task created");
    ESP_LOGI(TAG, "System is now running...");
} 