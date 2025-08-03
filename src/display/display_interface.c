/**
 * @file display_interface.c
 * @brief Modular Display Interface Implementation
 * 
 * This module implements the unified display interface for different
 * display types including OLED, LCD, and console output. It provides
 * a clean abstraction layer for display management and updates.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#include "display_interface.h"
#include <string.h>
#include <esp_log.h>
#include <stdio.h>

static const char *TAG = "DISPLAY_INTERFACE";

// Global variables
static display_interface_config_t g_config;
static bool g_initialized = false;

/**
 * @brief Update console display with sensor data and health status
 * 
 * @param sensor_data Sensor data to display
 * @param health Plant health status
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t console_display_update(const sensor_data_t *sensor_data, const plant_health_t *health)
{
    if (!sensor_data || !health) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Clear console (simple approach)
    printf("\033[2J\033[H"); // Clear screen and move cursor to top
    
    // Display plant monitor frame
    printf("┌─────────────────────────┐\n");
    printf("│     Plant Monitor       │\n");
    printf("│      %s %s       │\n", health->emoji, health->health_text);
    printf("│                         │\n");
    printf("│  T: %.1f°C  H: %.1f%%   │\n", sensor_data->temperature, sensor_data->humidity);
    printf("│  Soil: %d  Light: %d │\n", sensor_data->soil_moisture, sensor_data->light_level);
    printf("│  Health: %.1f%%         │\n", health->health_score);
    printf("│  Uptime: %02d:%02d:%02d       │\n", 
           (int)(sensor_data->uptime_seconds / 3600),
           (int)((sensor_data->uptime_seconds % 3600) / 60),
           (int)(sensor_data->uptime_seconds % 60));
    printf("└─────────────────────────┘\n");
    printf("\n");
    printf("Recommendation: %s\n", health->recommendation);
    printf("\n");
    
    return ESP_OK;
}

esp_err_t display_interface_init(const display_interface_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Initializing Display Interface");
    
    // Copy configuration
    memcpy(&g_config, config, sizeof(display_interface_config_t));
    
    // Initialize individual displays
    for (int i = 0; i < g_config.display_count; i++) {
        if (g_config.displays[i].enabled) {
            ESP_LOGI(TAG, "Initializing display: %s", g_config.displays[i].name);
            
            switch (g_config.displays[i].type) {
                case DISPLAY_TYPE_CONSOLE:
                    ESP_LOGI(TAG, "Console display initialized");
                    break;
                case DISPLAY_TYPE_OLED_SSD1306:
                case DISPLAY_TYPE_OLED_SH1106:
                case DISPLAY_TYPE_LCD_16X2:
                case DISPLAY_TYPE_LCD_20X4:
                case DISPLAY_TYPE_TFT_SPI:
                    ESP_LOGW(TAG, "Display type %d not yet implemented", g_config.displays[i].type);
                    break;
                default:
                    ESP_LOGW(TAG, "Unknown display type: %d", g_config.displays[i].type);
                    break;
            }
        }
    }
    
    g_initialized = true;
    ESP_LOGI(TAG, "Display Interface initialized successfully");
    return ESP_OK;
}

esp_err_t display_interface_update(const sensor_data_t *sensor_data, const plant_health_t *health)
{
    if (!sensor_data || !health || !g_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ESP_OK;
    
    // Update all enabled displays
    for (int i = 0; i < g_config.display_count; i++) {
        if (!g_config.displays[i].enabled) {
            continue;
        }
        
        esp_err_t display_ret = ESP_OK;
        
        switch (g_config.displays[i].type) {
            case DISPLAY_TYPE_CONSOLE:
                display_ret = console_display_update(sensor_data, health);
                break;
            case DISPLAY_TYPE_OLED_SSD1306:
            case DISPLAY_TYPE_OLED_SH1106:
            case DISPLAY_TYPE_LCD_16X2:
            case DISPLAY_TYPE_LCD_20X4:
            case DISPLAY_TYPE_TFT_SPI:
                // Not yet implemented
                display_ret = ESP_ERR_NOT_SUPPORTED;
                break;
            default:
                ESP_LOGW(TAG, "Unknown display type: %d", g_config.displays[i].type);
                display_ret = ESP_ERR_NOT_SUPPORTED;
                break;
        }
        
        if (display_ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to update display %s: %s", 
                     g_config.displays[i].name, esp_err_to_name(display_ret));
            ret = display_ret;
        }
    }
    
    return ret;
}

esp_err_t display_interface_clear(void)
{
    if (!g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Clearing all displays");
    
    // Clear console display
    printf("\033[2J\033[H"); // Clear screen and move cursor to top
    
    // TODO: Clear other display types when implemented
    
    return ESP_OK;
}

esp_err_t display_interface_show_welcome(void)
{
    if (!g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Showing welcome message");
    
    // Show welcome message on console
    printf("\033[2J\033[H"); // Clear screen
    printf("┌─────────────────────────┐\n");
    printf("│   🌱 Plant Monitor 🌱   │\n");
    printf("│                         │\n");
    printf("│    System Starting...   │\n");
    printf("│                         │\n");
    printf("│  Modular Architecture   │\n");
    printf("│  Professional Design    │\n");
    printf("│  Clean Implementation   │\n");
    printf("└─────────────────────────┘\n");
    printf("\n");
    printf("Initializing sensors and displays...\n");
    printf("\n");
    
    return ESP_OK;
}

esp_err_t display_interface_show_error(const char *error_message)
{
    if (!error_message || !g_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Showing error message: %s", error_message);
    
    // Show error message on console
    printf("\033[2J\033[H"); // Clear screen
    printf("┌─────────────────────────┐\n");
    printf("│      ❌ ERROR ❌        │\n");
    printf("│                         │\n");
    printf("│  %-21s │\n", error_message);
    printf("│                         │\n");
    printf("│  Check connections and  │\n");
    printf("│  try again later...     │\n");
    printf("└─────────────────────────┘\n");
    printf("\n");
    
    return ESP_OK;
}

esp_err_t display_interface_set_brightness(uint8_t brightness)
{
    if (!g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    g_config.brightness = brightness;
    ESP_LOGI(TAG, "Display brightness set to %d", brightness);
    
    // TODO: Apply brightness to actual displays when implemented
    
    return ESP_OK;
}

esp_err_t display_interface_get_status(int *working_displays, int *total_displays)
{
    if (!working_displays || !total_displays || !g_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *total_displays = g_config.display_count;
    *working_displays = 0;
    
    // Count enabled displays
    for (int i = 0; i < g_config.display_count; i++) {
        if (g_config.displays[i].enabled) {
            (*working_displays)++;
        }
    }
    
    return ESP_OK;
}

esp_err_t display_interface_deinit(void)
{
    if (!g_initialized) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Deinitializing Display Interface");
    
    // Clear displays
    display_interface_clear();
    
    g_initialized = false;
    ESP_LOGI(TAG, "Display Interface deinitialized");
    
    return ESP_OK;
} 