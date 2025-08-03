/**
 * @file aht10.h
 * @brief AHT10 Temperature and Humidity Sensor Driver
 * 
 * This module provides a complete driver for the AHT10 temperature and
 * humidity sensor. It includes initialization, reading, calibration,
 * and error handling with proper I2C communication.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#ifndef AHT10_H
#define AHT10_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief AHT10 sensor address
 */
#define AHT10_I2C_ADDR        0x38    /**< Default I2C address for AHT10 */

/**
 * @brief AHT10 command codes
 */
#define AHT10_CMD_INIT        0xE1    /**< Initialization command */
#define AHT10_CMD_MEASURE     0xAC    /**< Measurement command */
#define AHT10_CMD_NORMAL      0xA8    /**< Normal mode command */
#define AHT10_CMD_SOFT_RESET  0xBA    /**< Soft reset command */

/**
 * @brief AHT10 status bits
 */
#define AHT10_STATUS_BUSY     0x80    /**< Busy status bit */
#define AHT10_STATUS_CAL      0x08    /**< Calibration status bit */

/**
 * @brief AHT10 configuration structure
 */
typedef struct {
    uint8_t address;          /**< I2C address of the sensor */
    uint8_t sda_pin;         /**< SDA pin number */
    uint8_t scl_pin;         /**< SCL pin number */
    uint32_t i2c_freq;       /**< I2C frequency in Hz */
    bool enabled;             /**< Whether sensor is enabled */
} aht10_config_t;

/**
 * @brief AHT10 reading data structure
 */
typedef struct {
    float temperature;        /**< Temperature in Celsius */
    float humidity;          /**< Humidity percentage */
    bool valid;              /**< Whether reading is valid */
    esp_err_t error;         /**< Error code if reading failed */
} aht10_reading_t;

/**
 * @brief Initialize AHT10 sensor
 * 
 * @param config AHT10 configuration
 * @return ESP_OK on success, error code on failure
 */
esp_err_t aht10_init(const aht10_config_t *config);

/**
 * @brief Read temperature and humidity from AHT10
 * 
 * @param reading Pointer to store the reading
 * @return ESP_OK on success, error code on failure
 */
esp_err_t aht10_read(aht10_reading_t *reading);

/**
 * @brief Read only temperature from AHT10
 * 
 * @param temperature Pointer to store temperature value
 * @return ESP_OK on success, error code on failure
 */
esp_err_t aht10_read_temperature(float *temperature);

/**
 * @brief Read only humidity from AHT10
 * 
 * @param humidity Pointer to store humidity value
 * @return ESP_OK on success, error code on failure
 */
esp_err_t aht10_read_humidity(float *humidity);

/**
 * @brief Soft reset AHT10 sensor
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t aht10_soft_reset(void);

/**
 * @brief Check if AHT10 is calibrated
 * 
 * @param calibrated Pointer to store calibration status
 * @return ESP_OK on success, error code on failure
 */
esp_err_t aht10_is_calibrated(bool *calibrated);

/**
 * @brief Calibrate AHT10 sensor
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t aht10_calibrate(void);

/**
 * @brief Get AHT10 sensor status
 * 
 * @param busy Pointer to store busy status
 * @param calibrated Pointer to store calibration status
 * @return ESP_OK on success, error code on failure
 */
esp_err_t aht10_get_status(bool *busy, bool *calibrated);

/**
 * @brief Deinitialize AHT10 sensor
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t aht10_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // AHT10_H 