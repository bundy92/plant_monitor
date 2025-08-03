/**
 * @file gy302.c
 * @brief GY-302 Digital Light Intensity Sensor Implementation
 * 
 * This module implements the GY-302 (BH1750FVI) digital light intensity
 * sensor driver using I2C communication. It includes proper initialization,
 * reading, and error handling for reliable light intensity measurements.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#include "gy302.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "GY302";

// Global variables for I2C communication
static uint8_t g_i2c_address = 0;
static uint8_t g_current_mode = 0;
static bool g_initialized = false;
static i2c_port_t g_i2c_port = I2C_NUM_0;

/**
 * @brief Initialize I2C master for GY-302
 * 
 * @param sda_pin SDA pin number
 * @param scl_pin SCL pin number
 * @param freq I2C frequency in Hz
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t gy302_i2c_master_init(uint8_t sda_pin, uint8_t scl_pin, uint32_t freq)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = freq,
    };
    
    esp_err_t ret = i2c_param_config(g_i2c_port, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure I2C: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = i2c_driver_install(g_i2c_port, conf.mode, 0, 0, 0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to install I2C driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

/**
 * @brief Write command to GY-302
 * 
 * @param cmd Command to write
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t gy302_write_cmd(uint8_t cmd)
{
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (g_i2c_address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, cmd, true);
    i2c_master_stop(cmd_handle);
    
    esp_err_t ret = i2c_master_cmd_begin(g_i2c_port, cmd_handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd_handle);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write command 0x%02X: %s", cmd, esp_err_to_name(ret));
    }
    
    return ret;
}

/**
 * @brief Read data from GY-302
 * 
 * @param data Buffer to store read data
 * @param len Number of bytes to read
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t gy302_read_data(uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (g_i2c_address << 1) | I2C_MASTER_READ, true);
    
    if (len > 1) {
        i2c_master_read(cmd_handle, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd_handle, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd_handle);
    
    esp_err_t ret = i2c_master_cmd_begin(g_i2c_port, cmd_handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd_handle);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read data: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

/**
 * @brief Initialize GY-302 sensor
 * 
 * @param config GY-302 configuration
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_init(const gy302_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "Invalid configuration");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (g_initialized) {
        ESP_LOGW(TAG, "GY-302 already initialized");
        return ESP_OK;
    }
    
    g_i2c_address = config->address;
    g_current_mode = config->mode;
    
    // Initialize I2C
    esp_err_t ret = gy302_i2c_master_init(config->sda_pin, config->scl_pin, config->i2c_freq);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Power on the sensor
    ret = gy302_write_cmd(GY302_CMD_POWER_ON);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to power on GY-302");
        return ret;
    }
    
    // Reset the sensor
    ret = gy302_write_cmd(GY302_CMD_RESET);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset GY-302");
        return ret;
    }
    
    // Set measurement mode
    ret = gy302_write_cmd(g_current_mode);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set measurement mode");
        return ret;
    }
    
    ESP_LOGI(TAG, "GY-302 initialized on I2C address 0x%02X", g_i2c_address);
    g_initialized = true;
    
    return ESP_OK;
}

/**
 * @brief Read light intensity from GY-302
 * 
 * @param reading Pointer to store the reading
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_read(gy302_reading_t *reading)
{
    if (!reading) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_initialized) {
        ESP_LOGE(TAG, "GY-302 not initialized");
        reading->valid = false;
        reading->error = ESP_ERR_INVALID_STATE;
        return ESP_ERR_INVALID_STATE;
    }
    
    // For one-time measurement modes, send the command
    if (g_current_mode >= GY302_MODE_ONE_H) {
        esp_err_t ret = gy302_write_cmd(g_current_mode);
        if (ret != ESP_OK) {
            reading->valid = false;
            reading->error = ret;
            return ret;
        }
        
        // Wait for measurement (180ms for high resolution)
        if (g_current_mode == GY302_MODE_ONE_H || g_current_mode == GY302_MODE_ONE_H2) {
            vTaskDelay(pdMS_TO_TICKS(180));
        } else {
            vTaskDelay(pdMS_TO_TICKS(24)); // Low resolution
        }
    }
    
    // Read 2 bytes of data
    uint8_t data[2];
    esp_err_t ret = gy302_read_data(data, 2);
    if (ret != ESP_OK) {
        reading->valid = false;
        reading->error = ret;
        return ret;
    }
    
    // Convert to lux value
    uint16_t raw_value = (data[0] << 8) | data[1];
    
    // Convert based on measurement mode
    float lux = 0.0f;
    switch (g_current_mode) {
        case GY302_MODE_CONT_H:
        case GY302_MODE_ONE_H:
            lux = (float)raw_value / 1.2f;
            break;
        case GY302_MODE_CONT_H2:
        case GY302_MODE_ONE_H2:
            lux = (float)raw_value / 1.2f;
            break;
        case GY302_MODE_CONT_L:
        case GY302_MODE_ONE_L:
            lux = (float)raw_value / 1.2f;
            break;
        default:
            lux = (float)raw_value / 1.2f;
            break;
    }
    
    reading->lux = lux;
    reading->valid = true;
    reading->error = ESP_OK;
    
    ESP_LOGD(TAG, "GY-302 light intensity: %.1f lux", reading->lux);
    
    return ESP_OK;
}

/**
 * @brief Read only light intensity from GY-302
 * 
 * @param lux Pointer to store lux value
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_read_lux(float *lux)
{
    if (!lux) {
        return ESP_ERR_INVALID_ARG;
    }
    
    gy302_reading_t reading;
    esp_err_t ret = gy302_read(&reading);
    
    if (ret == ESP_OK && reading.valid) {
        *lux = reading.lux;
    }
    
    return ret;
}

/**
 * @brief Set measurement mode
 * 
 * @param mode Measurement mode
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_set_mode(uint8_t mode)
{
    if (!g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = gy302_write_cmd(mode);
    if (ret == ESP_OK) {
        g_current_mode = mode;
        ESP_LOGI(TAG, "GY-302 measurement mode set to 0x%02X", mode);
    }
    
    return ret;
}

/**
 * @brief Get current measurement mode
 * 
 * @param mode Pointer to store mode
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_get_mode(uint8_t *mode)
{
    if (!mode) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    *mode = g_current_mode;
    return ESP_OK;
}

/**
 * @brief Power down GY-302 sensor
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_power_down(void)
{
    if (!g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = gy302_write_cmd(GY302_CMD_POWER_DOWN);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "GY-302 powered down");
    }
    
    return ret;
}

/**
 * @brief Power on GY-302 sensor
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_power_on(void)
{
    if (!g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = gy302_write_cmd(GY302_CMD_POWER_ON);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "GY-302 powered on");
    }
    
    return ret;
}

/**
 * @brief Reset GY-302 sensor
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_reset(void)
{
    if (!g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = gy302_write_cmd(GY302_CMD_RESET);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "GY-302 reset");
    }
    
    return ret;
}

/**
 * @brief Get GY-302 sensor status
 * 
 * @param powered Whether sensor is powered
 * @param connected Whether sensor is connected
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_get_status(bool *powered, bool *connected)
{
    if (!powered || !connected) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_initialized) {
        *powered = false;
        *connected = false;
        return ESP_ERR_INVALID_STATE;
    }
    
    // Try to read from sensor to check if connected
    gy302_reading_t reading;
    esp_err_t ret = gy302_read(&reading);
    
    *connected = (ret == ESP_OK);
    *powered = *connected; // If connected, assume powered
    
    return ESP_OK;
}

/**
 * @brief Deinitialize GY-302 sensor
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t gy302_deinit(void)
{
    if (!g_initialized) {
        return ESP_OK;
    }
    
    // Power down the sensor
    gy302_power_down();
    
    // Uninstall I2C driver
    i2c_driver_delete(g_i2c_port);
    
    g_initialized = false;
    g_i2c_address = 0;
    g_current_mode = 0;
    
    ESP_LOGI(TAG, "GY-302 deinitialized");
    
    return ESP_OK;
} 