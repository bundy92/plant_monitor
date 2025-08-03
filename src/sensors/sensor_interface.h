/**
 * @file sensor_interface.h
 * @brief Modular Sensor Interface for Plant Monitoring System
 * 
 * This module provides a unified interface for different sensor types
 * including AHT10, DHT, and future sensor implementations. It supports
 * temperature, humidity, soil moisture, and light sensors with
 * configurable sensor selection and error handling.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#ifndef SENSOR_INTERFACE_H
#define SENSOR_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sensor types supported by the system
 */
typedef enum {
    SENSOR_TYPE_AHT10 = 0,    /**< AHT10 temperature/humidity sensor */
    SENSOR_TYPE_DHT11,        /**< DHT11 temperature/humidity sensor */
    SENSOR_TYPE_DHT22,        /**< DHT22 temperature/humidity sensor */
    SENSOR_TYPE_SOIL_MOISTURE,/**< Analog soil moisture sensor */
    SENSOR_TYPE_LIGHT,        /**< Analog light sensor */
    SENSOR_TYPE_MAX           /**< Maximum sensor type value */
} sensor_type_t;

/**
 * @brief Sensor configuration structure
 */
typedef struct {
    sensor_type_t type;       /**< Type of sensor */
    uint8_t address;          /**< I2C address (for I2C sensors) */
    uint8_t pin;              /**< GPIO pin (for one-wire/analog sensors) */
    bool enabled;             /**< Whether sensor is enabled */
    char name[32];            /**< Human-readable sensor name */
} sensor_config_t;

/**
 * @brief Sensor reading data structure
 */
typedef struct {
    float temperature;        /**< Temperature in Celsius */
    float humidity;          /**< Humidity percentage */
    uint16_t soil_moisture;  /**< Soil moisture value (0-4095) */
    uint16_t light_level;    /**< Light level value (0-4095) */
    bool valid;              /**< Whether reading is valid */
    esp_err_t error;         /**< Error code if reading failed */
} sensor_reading_t;

/**
 * @brief Sensor interface configuration
 */
typedef struct {
    sensor_config_t sensors[8];  /**< Array of sensor configurations */
    uint8_t sensor_count;        /**< Number of configured sensors */
    uint8_t i2c_sda_pin;        /**< I2C SDA pin */
    uint8_t i2c_scl_pin;        /**< I2C SCL pin */
    uint32_t i2c_frequency;     /**< I2C frequency in Hz */
    uint8_t adc_soil_pin;       /**< ADC pin for soil moisture */
    uint8_t adc_light_pin;      /**< ADC pin for light sensor */
} sensor_interface_config_t;

/**
 * @brief Initialize the sensor interface
 * 
 * @param config Sensor interface configuration
 * @return ESP_OK on success, error code on failure
 */
esp_err_t sensor_interface_init(const sensor_interface_config_t *config);

/**
 * @brief Read all configured sensors
 * 
 * @param readings Array to store sensor readings
 * @param max_readings Maximum number of readings to store
 * @return Number of valid readings, negative on error
 */
int sensor_interface_read_all(sensor_reading_t *readings, int max_readings);

/**
 * @brief Read a specific sensor by type
 * 
 * @param type Sensor type to read
 * @param reading Pointer to store the reading
 * @return ESP_OK on success, error code on failure
 */
esp_err_t sensor_interface_read_sensor(sensor_type_t type, sensor_reading_t *reading);

/**
 * @brief Scan for I2C devices
 * 
 * @return Number of devices found
 */
int sensor_interface_scan_i2c(void);

/**
 * @brief Get sensor status
 * 
 * @param working_sensors Number of working sensors
 * @param total_sensors Total number of configured sensors
 * @return ESP_OK on success, error code on failure
 */
esp_err_t sensor_interface_get_status(int *working_sensors, int *total_sensors);

/**
 * @brief Deinitialize the sensor interface
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t sensor_interface_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_INTERFACE_H 