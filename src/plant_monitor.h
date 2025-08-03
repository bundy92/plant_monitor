/**
 * @file plant_monitor.h
 * @brief Professional Plant Monitoring System - Unified Interface
 * 
 * This header provides a clean, industry-standard interface for the ESP32-C6
 * plant monitoring system. It consolidates sensor management, display control,
 * and plant health analysis into a single, maintainable API.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#ifndef PLANT_MONITOR_H
#define PLANT_MONITOR_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONFIGURATION STRUCTURES
// ============================================================================

/**
 * @brief Plant monitoring system configuration
 * 
 * This structure contains all configuration parameters for the plant monitoring
 * system including sensors, display, and health analysis settings.
 */
typedef struct {
    // I2C Configuration
    int sda_pin;                    /**< I2C SDA pin (default: 21) */
    int scl_pin;                    /**< I2C SCL pin (default: 22) */
    uint32_t i2c_freq_hz;          /**< I2C frequency in Hz (default: 100000) */
    
    // Sensor Configuration
    uint8_t aht10_addr_1;          /**< First AHT10 address (default: 0x38) */
    uint8_t aht10_addr_2;          /**< Second AHT10 address (default: 0x39) */
    bool enable_dht_sensors;        /**< Enable DHT22/DHT11 sensors */
    int dht_pin;                    /**< DHT sensor pin */
    
    // Display Configuration
    bool enable_display;            /**< Enable OLED/LCD display */
    uint8_t display_addr;           /**< Display I2C address (default: 0x3C) */
    int display_width;              /**< Display width in pixels */
    int display_height;             /**< Display height in pixels */
    
    // Plant Health Configuration
    float temp_min;                 /**< Minimum temperature (°C) */
    float temp_max;                 /**< Maximum temperature (°C) */
    float temp_optimal_min;         /**< Optimal temperature minimum (°C) */
    float temp_optimal_max;         /**< Optimal temperature maximum (°C) */
    float humidity_min;             /**< Minimum humidity (%) */
    float humidity_max;             /**< Maximum humidity (%) */
    float humidity_optimal_min;     /**< Optimal humidity minimum (%) */
    float humidity_optimal_max;     /**< Optimal humidity maximum (%) */
    
    // System Configuration
    int data_interval_ms;           /**< Data transmission interval (ms) */
    bool enable_wifi;               /**< Enable WiFi connectivity */
    const char* wifi_ssid;          /**< WiFi SSID */
    const char* wifi_password;      /**< WiFi password */
    const char* server_url;         /**< Server URL for data transmission */
} plant_monitor_config_t;

/**
 * @brief Sensor data structure
 * 
 * This structure contains all sensor readings from the plant monitoring system.
 */
typedef struct {
    // Temperature and Humidity
    float temperature_1;            /**< Temperature from sensor 1 (°C) */
    float humidity_1;               /**< Humidity from sensor 1 (%) */
    float temperature_2;            /**< Temperature from sensor 2 (°C) */
    float humidity_2;               /**< Humidity from sensor 2 (%) */
    float temperature_avg;          /**< Average temperature (°C) */
    float humidity_avg;             /**< Average humidity (%) */
    
    // Analog Sensors
    uint16_t soil_moisture;        /**< Soil moisture reading (0-4095) */
    uint16_t light_level;          /**< Light level reading (0-4095) */
    
    // System Status
    uint32_t uptime_seconds;       /**< System uptime in seconds */
    bool wifi_connected;            /**< WiFi connection status */
    bool data_sent;                /**< Last data transmission status */
    uint32_t timestamp;            /**< Timestamp of readings */
} plant_monitor_data_t;

/**
 * @brief Plant health status
 * 
 * This structure contains the calculated plant health status and recommendations.
 */
typedef struct {
    enum {
        PLANT_HEALTH_EXCELLENT,     /**< Plant health is excellent */
        PLANT_HEALTH_GOOD,          /**< Plant health is good */
        PLANT_HEALTH_FAIR,          /**< Plant health is fair */
        PLANT_HEALTH_POOR,          /**< Plant health is poor */
        PLANT_HEALTH_CRITICAL       /**< Plant health is critical */
    } health_level;
    
    const char* health_text;        /**< Human-readable health status */
    const char* emoji;              /**< Emoji representation */
    const char* recommendation;     /**< Care recommendation */
    float health_score;             /**< Health score (0.0-100.0) */
} plant_health_t;

// ============================================================================
// CORE FUNCTIONS
// ============================================================================

/**
 * @brief Initialize the plant monitoring system
 * 
 * This function initializes all components of the plant monitoring system
 * including sensors, display, WiFi, and health analysis.
 * 
 * @param config Pointer to the system configuration
 * @return ESP_OK on success, error code on failure
 * 
 * @note This function must be called before any other plant monitor functions
 */
esp_err_t plant_monitor_init(const plant_monitor_config_t* config);

/**
 * @brief Deinitialize the plant monitoring system
 * 
 * This function cleans up all resources used by the plant monitoring system.
 * 
 * @return ESP_OK on success, error code on failure
 * 
 * @note This function should be called when the system is no longer needed
 */
esp_err_t plant_monitor_deinit(void);

/**
 * @brief Read all sensor data
 * 
 * This function reads data from all sensors and calculates averages.
 * 
 * @param data Pointer to store the sensor readings
 * @return ESP_OK on success, error code on failure
 * 
 * @note The system must be initialized before calling this function
 */
esp_err_t plant_monitor_read_sensors(plant_monitor_data_t* data);

/**
 * @brief Calculate plant health status
 * 
 * This function analyzes sensor data and calculates the plant health status
 * including health level, recommendations, and visual indicators.
 * 
 * @param data Pointer to the sensor data
 * @param health Pointer to store the health analysis
 * @return ESP_OK on success, error code on failure
 */
esp_err_t plant_monitor_calculate_health(const plant_monitor_data_t* data, plant_health_t* health);

/**
 * @brief Update display with current data
 * 
 * This function updates the display with current sensor data and health status.
 * 
 * @param data Pointer to the sensor data
 * @param health Pointer to the health analysis
 * @return ESP_OK on success, error code on failure
 * 
 * @note Display must be enabled in configuration
 */
esp_err_t plant_monitor_update_display(const plant_monitor_data_t* data, const plant_health_t* health);

/**
 * @brief Transmit data to server
 * 
 * This function transmits sensor data and health status to the configured server.
 * 
 * @param data Pointer to the sensor data
 * @param health Pointer to the health analysis
 * @return ESP_OK on success, error code on failure
 * 
 * @note WiFi must be enabled and connected
 */
esp_err_t plant_monitor_transmit_data(const plant_monitor_data_t* data, const plant_health_t* health);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Get default configuration
 * 
 * This function provides a default configuration for the plant monitoring system
 * with sensible defaults for most use cases.
 * 
 * @param config Pointer to store the default configuration
 * @return ESP_OK on success, error code on failure
 */
esp_err_t plant_monitor_get_default_config(plant_monitor_config_t* config);

/**
 * @brief Scan for I2C devices
 * 
 * This function scans the I2C bus for connected devices and reports findings.
 * 
 * @return ESP_OK on success, error code on failure
 * 
 * @note Useful for troubleshooting sensor connections
 */
esp_err_t plant_monitor_scan_i2c_devices(void);

/**
 * @brief Get system status
 * 
 * This function returns the current status of all system components.
 * 
 * @param sensors_working Number of working sensors
 * @param display_working Whether display is working
 * @param wifi_connected Whether WiFi is connected
 * @return ESP_OK on success, error code on failure
 */
esp_err_t plant_monitor_get_status(int* sensors_working, bool* display_working, bool* wifi_connected);

// ============================================================================
// CONSTANTS
// ============================================================================

/** Default I2C SDA pin */
#define PLANT_MONITOR_DEFAULT_SDA_PIN        21

/** Default I2C SCL pin */
#define PLANT_MONITOR_DEFAULT_SCL_PIN        22

/** Default I2C frequency in Hz */
#define PLANT_MONITOR_DEFAULT_I2C_FREQ_HZ    100000

/** Default AHT10 sensor addresses */
#define PLANT_MONITOR_AHT10_ADDR_1           0x38
#define PLANT_MONITOR_AHT10_ADDR_2           0x39

/** Default display I2C address */
#define PLANT_MONITOR_DEFAULT_DISPLAY_ADDR   0x3C

/** Default data transmission interval in milliseconds */
#define PLANT_MONITOR_DEFAULT_DATA_INTERVAL_MS 30000

#ifdef __cplusplus
}
#endif

#endif // PLANT_MONITOR_H 