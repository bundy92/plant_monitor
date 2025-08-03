/**
 * @file display_interface.h
 * @brief Modular Display Interface for Plant Monitoring System
 * 
 * This module provides a unified interface for different display types
 * including OLED, LCD, and TFT screens. It supports text, graphics,
 * and emoji display with configurable layouts and themes.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#ifndef DISPLAY_INTERFACE_H
#define DISPLAY_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Display types supported by the system
 */
typedef enum {
    DISPLAY_TYPE_OLED_SSD1306 = 0,  /**< SSD1306 OLED display */
    DISPLAY_TYPE_OLED_SH1106,       /**< SH1106 OLED display */
    DISPLAY_TYPE_LCD_16X2,          /**< 16x2 LCD display */
    DISPLAY_TYPE_LCD_20X4,          /**< 20x4 LCD display */
    DISPLAY_TYPE_TFT_SPI,           /**< TFT SPI display */
    DISPLAY_TYPE_CONSOLE,           /**< Console output (for debugging) */
    DISPLAY_TYPE_MAX                /**< Maximum display type value */
} display_type_t;

/**
 * @brief Display configuration structure
 */
typedef struct {
    display_type_t type;     /**< Type of display */
    uint8_t i2c_address;     /**< I2C address (for I2C displays) */
    uint8_t sda_pin;         /**< SDA pin (for I2C displays) */
    uint8_t scl_pin;         /**< SCL pin (for I2C displays) */
    uint8_t spi_cs_pin;      /**< SPI CS pin (for SPI displays) */
    uint8_t spi_dc_pin;      /**< SPI DC pin (for SPI displays) */
    uint8_t spi_rst_pin;     /**< SPI RST pin (for SPI displays) */
    uint8_t spi_mosi_pin;    /**< SPI MOSI pin (for SPI displays) */
    uint8_t spi_sck_pin;     /**< SPI SCK pin (for SPI displays) */
    bool enabled;             /**< Whether display is enabled */
    char name[32];            /**< Human-readable display name */
} display_config_t;

/**
 * @brief Plant health status for display
 */
typedef struct {
    float health_score;      /**< Health score (0-100) */
    const char *health_text; /**< Health status text */
    const char *emoji;       /**< Health emoji */
    const char *recommendation; /**< Care recommendation */
} plant_health_t;

/**
 * @brief Sensor data for display
 */
typedef struct {
    float temperature;       /**< Temperature in Celsius */
    float humidity;         /**< Humidity percentage */
    uint16_t soil_moisture; /**< Soil moisture value */
    uint16_t light_level;   /**< Light level value */
    uint32_t uptime_seconds; /**< System uptime in seconds */
} sensor_data_t;

/**
 * @brief Display interface configuration
 */
typedef struct {
    display_config_t displays[4];  /**< Array of display configurations */
    uint8_t display_count;         /**< Number of configured displays */
    bool enable_backlight;         /**< Enable display backlight */
    uint8_t brightness;            /**< Display brightness (0-255) */
    bool enable_auto_off;          /**< Enable auto power off */
    uint32_t auto_off_timeout;     /**< Auto off timeout in seconds */
} display_interface_config_t;

/**
 * @brief Initialize the display interface
 * 
 * @param config Display interface configuration
 * @return ESP_OK on success, error code on failure
 */
esp_err_t display_interface_init(const display_interface_config_t *config);

/**
 * @brief Update display with sensor data and health status
 * 
 * @param sensor_data Sensor data to display
 * @param health Plant health status
 * @return ESP_OK on success, error code on failure
 */
esp_err_t display_interface_update(const sensor_data_t *sensor_data, const plant_health_t *health);

/**
 * @brief Clear all displays
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t display_interface_clear(void);

/**
 * @brief Show welcome message on displays
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t display_interface_show_welcome(void);

/**
 * @brief Show error message on displays
 * 
 * @param error_message Error message to display
 * @return ESP_OK on success, error code on failure
 */
esp_err_t display_interface_show_error(const char *error_message);

/**
 * @brief Set display brightness
 * 
 * @param brightness Brightness value (0-255)
 * @return ESP_OK on success, error code on failure
 */
esp_err_t display_interface_set_brightness(uint8_t brightness);

/**
 * @brief Get display status
 * 
 * @param working_displays Number of working displays
 * @param total_displays Total number of configured displays
 * @return ESP_OK on success, error code on failure
 */
esp_err_t display_interface_get_status(int *working_displays, int *total_displays);

/**
 * @brief Deinitialize the display interface
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t display_interface_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_INTERFACE_H 