/**
 * @file ds18b20.h
 * @brief DS18B20 Waterproof Temperature Sensor Driver
 * 
 * This module provides a complete driver for the DS18B20 waterproof
 * temperature sensor using One-Wire communication protocol. It includes
 * initialization, reading, and error handling with proper One-Wire
 * communication.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief DS18B20 ROM commands
 */
#define DS18B20_CMD_CONVERT_TEMP    0x44    /**< Convert temperature command */
#define DS18B20_CMD_READ_SCRATCHPAD 0xBE    /**< Read scratchpad command */
#define DS18B20_CMD_WRITE_SCRATCHPAD 0x4E   /**< Write scratchpad command */
#define DS18B20_CMD_COPY_SCRATCHPAD 0x48    /**< Copy scratchpad command */
#define DS18B20_CMD_RECALL_E2      0xB8    /**< Recall E2 command */
#define DS18B20_CMD_READ_POWER     0xB4    /**< Read power supply command */

/**
 * @brief DS18B20 One-Wire commands
 */
#define DS18B20_CMD_SKIP_ROM       0xCC    /**< Skip ROM command */
#define DS18B20_CMD_READ_ROM       0x33    /**< Read ROM command */
#define DS18B20_CMD_MATCH_ROM      0x55    /**< Match ROM command */
#define DS18B20_CMD_SEARCH_ROM     0xF0    /**< Search ROM command */

/**
 * @brief DS18B20 configuration structure
 */
typedef struct {
    uint8_t pin;              /**< One-Wire pin number */
    uint8_t resolution;       /**< Temperature resolution (9-12 bits) */
    bool enabled;             /**< Whether sensor is enabled */
    uint64_t rom_code;        /**< ROM code for this sensor */
} ds18b20_config_t;

/**
 * @brief DS18B20 reading data structure
 */
typedef struct {
    float temperature;        /**< Temperature in Celsius */
    bool valid;              /**< Whether reading is valid */
    esp_err_t error;         /**< Error code if reading failed */
} ds18b20_reading_t;

/**
 * @brief Initialize DS18B20 sensor
 * 
 * @param config DS18B20 configuration
 * @return ESP_OK on success, error code on failure
 */
esp_err_t ds18b20_init(const ds18b20_config_t *config);

/**
 * @brief Read temperature from DS18B20
 * 
 * @param reading Pointer to store the reading
 * @return ESP_OK on success, error code on failure
 */
esp_err_t ds18b20_read(ds18b20_reading_t *reading);

/**
 * @brief Read only temperature from DS18B20
 * 
 * @param temperature Pointer to store temperature value
 * @return ESP_OK on success, error code on failure
 */
esp_err_t ds18b20_read_temperature(float *temperature);

/**
 * @brief Set temperature resolution
 * 
 * @param resolution Resolution in bits (9-12)
 * @return ESP_OK on success, error code on failure
 */
esp_err_t ds18b20_set_resolution(uint8_t resolution);

/**
 * @brief Get temperature resolution
 * 
 * @param resolution Pointer to store resolution
 * @return ESP_OK on success, error code on failure
 */
esp_err_t ds18b20_get_resolution(uint8_t *resolution);

/**
 * @brief Search for DS18B20 devices on One-Wire bus
 * 
 * @param rom_codes Array to store found ROM codes
 * @param max_devices Maximum number of devices to find
 * @return Number of devices found
 */
int ds18b20_search_devices(uint64_t *rom_codes, int max_devices);

/**
 * @brief Get DS18B20 sensor status
 * 
 * @param connected Whether sensor is connected
 * @param powered Whether sensor is powered
 * @return ESP_OK on success, error code on failure
 */
esp_err_t ds18b20_get_status(bool *connected, bool *powered);

/**
 * @brief Deinitialize DS18B20 sensor
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t ds18b20_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // DS18B20_H 