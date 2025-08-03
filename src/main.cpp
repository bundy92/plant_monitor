/**
 * @file main_example.cpp
 * @brief Plant Monitor System - Modular Example Application
 * 
 * This file demonstrates the modular plant monitoring system using
 * separate sensor and display interfaces. It provides a clean,
 * extensible implementation that can easily support different sensor
 * and display types including AHT10, DS18B20, GY-302, and various displays.
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
#include "sensor_interface.h"
#include "display_interface.h"

static const char *TAG = "PLANT_MONITOR_MODULAR";

// Global variables for sensor data and health
static sensor_reading_t sensor_readings[8];
static plant_health_t plant_health;

/**
 * @brief Calculate plant health based on sensor readings
 * 
 * @param readings Array of sensor readings
 * @param reading_count Number of readings
 * @param health Pointer to store health status
 * @return ESP_OK on success, error code on failure
 */
esp_err_t calculate_plant_health(const sensor_reading_t *readings, int reading_count, plant_health_t *health)
{
    if (!readings || !health || reading_count <= 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Calculate average temperature and humidity
    float avg_temp = 0.0f;
    float avg_humidity = 0.0f;
    float avg_lux = 0.0f;
    int valid_readings = 0;
    int temp_readings = 0;
    int humidity_readings = 0;
    int lux_readings = 0;
    
    for (int i = 0; i < reading_count; i++) {
        if (readings[i].valid) {
            if (readings[i].temperature > -50.0f && readings[i].temperature < 150.0f) {
                avg_temp += readings[i].temperature;
                temp_readings++;
            }
            if (readings[i].humidity >= 0.0f && readings[i].humidity <= 100.0f) {
                avg_humidity += readings[i].humidity;
                humidity_readings++;
            }
            if (readings[i].lux >= 0.0f) {
                avg_lux += readings[i].lux;
                lux_readings++;
            }
            valid_readings++;
        }
    }
    
    if (valid_readings == 0) {
        health->health_score = 0.0f;
        health->health_text = "Unknown";
        health->emoji = "❓";
        health->recommendation = "No sensor data available";
        return ESP_OK;
    }
    
    if (temp_readings > 0) avg_temp /= temp_readings;
    if (humidity_readings > 0) avg_humidity /= humidity_readings;
    if (lux_readings > 0) avg_lux /= lux_readings;
    
    // Calculate health score based on optimal ranges
    float temp_score = 100.0f;
    float humidity_score = 100.0f;
    float light_score = 100.0f;
    
    // Temperature scoring (optimal: 18-28°C, acceptable: 10-35°C)
    if (avg_temp < 10.0f || avg_temp > 35.0f) {
        temp_score = 0.0f;
    } else if (avg_temp < 18.0f || avg_temp > 28.0f) {
        temp_score = 50.0f;
    }
    
    // Humidity scoring (optimal: 40-70%, acceptable: 30-80%)
    if (avg_humidity < 30.0f || avg_humidity > 80.0f) {
        humidity_score = 0.0f;
    } else if (avg_humidity < 40.0f || avg_humidity > 70.0f) {
        humidity_score = 50.0f;
    }
    
    // Light scoring (optimal: 1000-10000 lux, acceptable: 100-50000 lux)
    if (avg_lux < 100.0f || avg_lux > 50000.0f) {
        light_score = 0.0f;
    } else if (avg_lux < 1000.0f || avg_lux > 10000.0f) {
        light_score = 50.0f;
    }
    
    // Calculate overall health score
    int score_count = 0;
    float total_score = 0.0f;
    
    if (temp_readings > 0) {
        total_score += temp_score;
        score_count++;
    }
    if (humidity_readings > 0) {
        total_score += humidity_score;
        score_count++;
    }
    if (lux_readings > 0) {
        total_score += light_score;
        score_count++;
    }
    
    health->health_score = score_count > 0 ? total_score / score_count : 0.0f;
    
    // Set health status and emoji
    if (health->health_score >= 90.0f) {
        health->health_text = "Excellent";
        health->emoji = "😊";
        health->recommendation = "Perfect conditions! Keep it up.";
    } else if (health->health_score >= 70.0f) {
        health->health_text = "Good";
        health->emoji = "🙂";
        health->recommendation = "Good conditions, monitor regularly.";
    } else if (health->health_score >= 50.0f) {
        health->health_text = "Fair";
        health->emoji = "😐";
        health->recommendation = "Acceptable conditions, consider adjustments.";
    } else if (health->health_score >= 30.0f) {
        health->health_text = "Poor";
        health->emoji = "😟";
        health->recommendation = "Needs attention, check environment.";
    } else {
        health->health_text = "Critical";
        health->emoji = "😱";
        health->recommendation = "Immediate attention required!";
    }
    
    return ESP_OK;
}

/**
 * @brief Main monitoring task
 * 
 * This task continuously reads sensors, calculates plant health,
 * updates displays, and transmits data at regular intervals.
 * 
 * @param pvParameters Task parameters (unused)
 */
void monitoring_task(void *pvParameters) 
{
    ESP_LOGI(TAG, "Plant monitoring task started");
    
    while (1) {
        // Read all sensors
        int reading_count = sensor_interface_read_all(sensor_readings, 8);
        if (reading_count < 0) {
            ESP_LOGE(TAG, "Failed to read sensors");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }
        
        ESP_LOGI(TAG, "Read %d sensor readings", reading_count);
        
        // Calculate plant health
        esp_err_t ret = calculate_plant_health(sensor_readings, reading_count, &plant_health);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to calculate health: %s", esp_err_to_name(ret));
        }
        
        // Update displays
        sensor_data_t display_data = {
            .temperature = 0.0f,
            .humidity = 0.0f,
            .soil_moisture = 0,
            .light_level = 0,
            .lux = 0.0f,
            .uptime_seconds = 0
        };
        
        // Aggregate sensor data for display
        for (int i = 0; i < reading_count; i++) {
            if (sensor_readings[i].valid) {
                display_data.temperature = sensor_readings[i].temperature;
                display_data.humidity = sensor_readings[i].humidity;
                display_data.soil_moisture = sensor_readings[i].soil_moisture;
                display_data.light_level = sensor_readings[i].light_level;
                display_data.lux = sensor_readings[i].lux;
                break; // Use first valid reading for display
            }
        }
        
        ret = display_interface_update(&display_data, &plant_health);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to update display: %s", esp_err_to_name(ret));
        }
        
        // Log summary
        ESP_LOGI(TAG, "=== Plant Monitor Summary ===");
        ESP_LOGI(TAG, "Valid sensors: %d/%d", reading_count, 8);
        ESP_LOGI(TAG, "Temperature: %.2f°C", display_data.temperature);
        ESP_LOGI(TAG, "Humidity: %.2f%%", display_data.humidity);
        ESP_LOGI(TAG, "Soil Moisture: %d", display_data.soil_moisture);
        ESP_LOGI(TAG, "Light Level: %d", display_data.light_level);
        ESP_LOGI(TAG, "Light Intensity: %.1f lux", display_data.lux);
        ESP_LOGI(TAG, "Plant Health: %s %s (Score: %.1f)", 
                 plant_health.health_text, plant_health.emoji, plant_health.health_score);
        ESP_LOGI(TAG, "Recommendation: %s", plant_health.recommendation);
        ESP_LOGI(TAG, "================================");
        
        // Wait for next reading
        vTaskDelay(pdMS_TO_TICKS(30000)); // 30 seconds
    }
}

/**
 * @brief Main application entry point
 * 
 * This function initializes the modular plant monitoring system
 * with sensor and display interfaces and starts the monitoring task.
 * 
 * @return void
 */
extern "C" void app_main(void) 
{
    ESP_LOGI(TAG, "Plant Monitor System Starting...");
    ESP_LOGI(TAG, "==================================");
    
    // Configure sensor interface with all available sensors
    sensor_interface_config_t sensor_config = {
        .sensors = {
            // AHT10 Sensors
            {
                .type = SENSOR_TYPE_AHT10,
                .address = 0x38,
                .pin = 0,
                .enabled = true,
                .name = "AHT10-1"
            },
            {
                .type = SENSOR_TYPE_AHT10,
                .address = 0x39,
                .pin = 0,
                .enabled = true,
                .name = "AHT10-2"
            },
            // DS18B20 Waterproof Temperature Sensor
            {
                .type = SENSOR_TYPE_DS18B20,
                .address = 0,
                .pin = 4,  // One-Wire pin
                .enabled = true,
                .name = "DS18B20-Waterproof"
            },
            // GY-302 Digital Light Sensor
            {
                .type = SENSOR_TYPE_GY302,
                .address = 0x23,
                .pin = 0,
                .enabled = true,
                .name = "GY-302-Light"
            },
            // Analog Sensors
            {
                .type = SENSOR_TYPE_SOIL_MOISTURE,
                .address = 0,
                .pin = 1,
                .enabled = true,
                .name = "Soil-Moisture"
            },
            {
                .type = SENSOR_TYPE_LIGHT,
                .address = 0,
                .pin = 2,
                .enabled = true,
                .name = "Light-Sensor"
            }
        },
        .sensor_count = 6,
        .i2c_sda_pin = 21,
        .i2c_scl_pin = 22,
        .i2c_frequency = 100000,
        .onewire_pin = 4,
        .adc_soil_pin = 1,
        .adc_light_pin = 2
    };
    
    // Configure display interface with multiple displays
    display_interface_config_t display_config = {
        .displays = {
            // Console Display (for debugging)
            {
                .type = DISPLAY_TYPE_CONSOLE,
                .i2c_address = 0,
                .sda_pin = 0,
                .scl_pin = 0,
                .spi_cs_pin = 0,
                .spi_dc_pin = 0,
                .spi_rst_pin = 0,
                .spi_mosi_pin = 0,
                .spi_sck_pin = 0,
                .spi_busy_pin = 0,
                .enabled = true,
                .name = "Console Display"
            },
            // Built-in SSD1306 (ESP32 DevKit)
            {
                .type = DISPLAY_TYPE_BUILTIN_SSD1306,
                .i2c_address = 0x3C,
                .sda_pin = 21,
                .scl_pin = 22,
                .spi_cs_pin = 0,
                .spi_dc_pin = 0,
                .spi_rst_pin = 0,
                .spi_mosi_pin = 0,
                .spi_sck_pin = 0,
                .spi_busy_pin = 0,
                .enabled = true,
                .name = "Built-in OLED"
            },
            // E-paper Display (SPI)
            {
                .type = DISPLAY_TYPE_EPAPER_SPI,
                .i2c_address = 0,
                .sda_pin = 0,
                .scl_pin = 0,
                .spi_cs_pin = 5,
                .spi_dc_pin = 17,
                .spi_rst_pin = 16,
                .spi_mosi_pin = 23,
                .spi_sck_pin = 18,
                .spi_busy_pin = 4,
                .enabled = true,
                .name = "E-paper Display"
            }
        },
        .display_count = 3,
        .enable_backlight = true,
        .brightness = 128,
        .enable_auto_off = false,
        .auto_off_timeout = 0
    };
    
    // Initialize sensor interface
    esp_err_t ret = sensor_interface_init(&sensor_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize sensor interface: %s", esp_err_to_name(ret));
        return;
    }
    
    // Initialize display interface
    ret = display_interface_init(&display_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize display interface: %s", esp_err_to_name(ret));
        return;
    }
    
    // Scan for I2C devices
    ESP_LOGI(TAG, "Scanning for I2C devices...");
    int device_count = sensor_interface_scan_i2c();
    ESP_LOGI(TAG, "Found %d I2C devices", device_count);
    
    // Get system status
    int working_sensors, total_sensors;
    sensor_interface_get_status(&working_sensors, &total_sensors);
    
    int working_displays, total_displays;
    display_interface_get_status(&working_displays, &total_displays);
    
    ESP_LOGI(TAG, "System initialized successfully!");
    ESP_LOGI(TAG, "Features:");
    ESP_LOGI(TAG, "- Modular sensor interface (AHT10, DS18B20, GY-302, analog)");
    ESP_LOGI(TAG, "- Modular display interface (OLED, E-paper, console)");
    ESP_LOGI(TAG, "- Professional numpy-style documentation");
    ESP_LOGI(TAG, "- Robust error handling and recovery");
    ESP_LOGI(TAG, "- Plant health analysis with emoji indicators");
    ESP_LOGI(TAG, "- Extensible architecture for future sensors");
    ESP_LOGI(TAG, "- Clean, industry-standard design");
    ESP_LOGI(TAG, "- Support for multiple hardware platforms");
    
    // Show welcome message
    display_interface_show_welcome();
    
    // Create monitoring task
    xTaskCreate(&monitoring_task, "monitoring_task", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Plant monitoring task created");
    ESP_LOGI(TAG, "System is now running...");
} 