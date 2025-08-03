/**
 * @file gy302.h
 * @brief GY-302 Digital Light Intensity Sensor Driver
 * 
 * This module provides a complete driver for the GY-302 (BH1750FVI)
 * digital light intensity sensor using I2C communication. It includes
 * initialization, reading, and error handling with proper I2C
 * communication for light intensity measurements in lux.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#ifndef GY302_H
#define GY302_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief GY-302 I2C address
 */
#define GY302_I2C_ADDR        0x23    /**< Default I2C address for GY-302 */

/**
 * @brief GY-302 command codes
 */
#define GY302_CMD_POWER_DOWN  0x00    /**< Power down command */
#define GY302_CMD_POWER_ON    0x01    /**< Power on command */
#define GY302_CMD_RESET       0x07    /**< Reset command */

/**
 * @brief GY-302 measurement modes
 */
#define GY302_MODE_CONT_H     0x10    /**< Continuous high resolution mode */
#define GY302_MODE_CONT_H2    0x11    /**< Continuous high resolution mode 2 */
#define GY302_MODE_CONT_L     0x13    /**< Continuous low resolution mode */
#define GY302_MODE_ONE_H      0x20    /**< One-time high resolution mode */
#define GY302_MODE_ONE_H2     0x21    /**< One-time high resolution mode 2 */
#define GY302_MODE_ONE_L      0x23    /**< One-time low resolution mode */

/**
 * @brief GY-302 configuration structure
 */
typedef struct {
    uint8_t address;          /**< I2C address of the sensor */
    uint8_t sda_pin;         /**< SDA pin number */
    uint8_t scl_pin;         /**< SCL pin number */
    uint32_t i2c_freq;       /**< I2C frequency in Hz */
    uint8_t mode;            /**< Measurement mode */
    bool enabled;             /**< Whether sensor is enabled */
} gy302_config_t;

/**
 * @brief GY-302 reading data structure
 */
typedef struct {
    float lux;               /**< Light intensity in lux */
    bool valid;              /**< Whether reading is valid */
    esp_err_t error;         /**< Error code if reading failed */
} gy302_reading_t;

/**
 * @brief Initialize GY-302 sensor
 * 
 * @param config GY-302 configuration
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_init(const gy302_config_t *config);

/**
 * @brief Read light intensity from GY-302
 * 
 * @param reading Pointer to store the reading
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_read(gy302_reading_t *reading);

/**
 * @brief Read only light intensity from GY-302
 * 
 * @param lux Pointer to store lux value
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_read_lux(float *lux);

/**
 * @brief Set measurement mode
 * 
 * @param mode Measurement mode
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_set_mode(uint8_t mode);

/**
 * @brief Get current measurement mode
 * 
 * @param mode Pointer to store mode
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_get_mode(uint8_t *mode);

/**
 * @brief Power down GY-302 sensor
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_power_down(void);

/**
 * @brief Power on GY-302 sensor
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_power_on(void);

/**
 * @brief Reset GY-302 sensor
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_reset(void);

/**
 * @brief Get GY-302 sensor status
 * 
 * @param powered Whether sensor is powered
 * @param connected Whether sensor is connected
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_get_status(bool *powered, bool *connected);

/**
 * @brief Deinitialize GY-302 sensor
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // GY302_H 