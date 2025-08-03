/**
 * @file aht10.c
 * @brief AHT10 Temperature and Humidity Sensor Implementation
 * 
 * This module implements the AHT10 sensor driver with proper I2C
 * communication, calibration, and error handling. It provides
 * temperature and humidity readings with high accuracy.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#include "aht10.h"
#include <string.h>
#include <esp_log.h>
#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "AHT10";

// Global variables
static aht10_config_t g_config;
static bool g_initialized = false;

/**
 * @brief Write command to AHT10 sensor
 * 
 * @param cmd Command to write
 * @param data Additional data (if any)
 * @param data_len Length of additional data
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t aht10_write_cmd(uint8_t cmd, const uint8_t *data, size_t data_len)
{
    if (!g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (g_config.address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, cmd, true);
    
    if (data && data_len > 0) {
        i2c_master_write(cmd_handle, (uint8_t*)data, data_len, true);
    }
    
    i2c_master_stop(cmd_handle);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd_handle);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 write command failed: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

/**
 * @brief Read data from AHT10 sensor
 * 
 * @param data Buffer to store read data
 * @param data_len Length of data to read
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t aht10_read_data(uint8_t *data, size_t data_len)
{
    if (!g_initialized || !data || data_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (g_config.address << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd_handle, data, data_len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd_handle);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd_handle);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 read data failed: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

esp_err_t aht10_init(const aht10_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Initializing AHT10 sensor at 0x%02x", config->address);
    
    // Copy configuration
    memcpy(&g_config, config, sizeof(aht10_config_t));
    
    // Check if sensor is enabled
    if (!g_config.enabled) {
        ESP_LOGW(TAG, "AHT10 sensor is disabled");
        return ESP_OK;
    }
    
    // Wait for sensor to power up
    vTaskDelay(pdMS_TO_TICKS(40));
    
    // Send soft reset command
    esp_err_t ret = aht10_write_cmd(AHT10_CMD_SOFT_RESET, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 soft reset failed");
        return ret;
    }
    
    // Wait after reset
    vTaskDelay(pdMS_TO_TICKS(20));
    
    // Check if sensor is calibrated
    bool calibrated;
    ret = aht10_is_calibrated(&calibrated);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to check AHT10 calibration status");
        return ret;
    }
    
    if (!calibrated) {
        ESP_LOGI(TAG, "AHT10 sensor not calibrated, starting calibration");
        ret = aht10_calibrate();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "AHT10 calibration failed");
            return ret;
        }
    } else {
        ESP_LOGI(TAG, "AHT10 sensor is already calibrated");
    }
    
    g_initialized = true;
    ESP_LOGI(TAG, "AHT10 sensor initialized successfully");
    return ESP_OK;
}

esp_err_t aht10_read(aht10_reading_t *reading)
{
    if (!reading || !g_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memset(reading, 0, sizeof(aht10_reading_t));
    
    // Send measurement command
    uint8_t cmd_data[] = {0x33, 0x00};
    esp_err_t ret = aht10_write_cmd(AHT10_CMD_MEASURE, cmd_data, sizeof(cmd_data));
    if (ret != ESP_OK) {
        reading->error = ret;
        reading->valid = false;
        return ret;
    }
    
    // Wait for measurement to complete
    vTaskDelay(pdMS_TO_TICKS(80));
    
    // Read measurement data (6 bytes)
    uint8_t data[6];
    ret = aht10_read_data(data, sizeof(data));
    if (ret != ESP_OK) {
        reading->error = ret;
        reading->valid = false;
        return ret;
    }
    
    // Check if sensor is busy
    if (data[0] & AHT10_STATUS_BUSY) {
        ESP_LOGW(TAG, "AHT10 sensor is busy");
        reading->error = ESP_ERR_TIMEOUT;
        reading->valid = false;
        return ESP_ERR_TIMEOUT;
    }
    
    // Check if sensor is calibrated
    if (!(data[0] & AHT10_STATUS_CAL)) {
        ESP_LOGW(TAG, "AHT10 sensor is not calibrated");
        reading->error = ESP_ERR_INVALID_STATE;
        reading->valid = false;
        return ESP_ERR_INVALID_STATE;
    }
    
    // Convert humidity data (20 bits)
    uint32_t humidity_raw = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
    reading->humidity = (float)humidity_raw * 100.0f / 1048576.0f; // 2^20
    
    // Convert temperature data (20 bits)
    uint32_t temp_raw = ((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];
    reading->temperature = (float)temp_raw * 200.0f / 1048576.0f - 50.0f; // 2^20, -50°C offset
    
    // Validate readings
    if (reading->humidity >= 0.0f && reading->humidity <= 100.0f &&
        reading->temperature >= -50.0f && reading->temperature <= 150.0f) {
        reading->valid = true;
        reading->error = ESP_OK;
    } else {
        ESP_LOGW(TAG, "AHT10 readings out of range: T=%.2f°C, H=%.2f%%", 
                 reading->temperature, reading->humidity);
        reading->valid = false;
        reading->error = ESP_ERR_INVALID_RESPONSE;
    }
    
    return reading->error;
}

esp_err_t aht10_read_temperature(float *temperature)
{
    if (!temperature) {
        return ESP_ERR_INVALID_ARG;
    }
    
    aht10_reading_t reading;
    esp_err_t ret = aht10_read(&reading);
    if (ret == ESP_OK && reading.valid) {
        *temperature = reading.temperature;
    }
    
    return ret;
}

esp_err_t aht10_read_humidity(float *humidity)
{
    if (!humidity) {
        return ESP_ERR_INVALID_ARG;
    }
    
    aht10_reading_t reading;
    esp_err_t ret = aht10_read(&reading);
    if (ret == ESP_OK && reading.valid) {
        *humidity = reading.humidity;
    }
    
    return ret;
}

esp_err_t aht10_soft_reset(void)
{
    if (!g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Sending soft reset to AHT10");
    
    esp_err_t ret = aht10_write_cmd(AHT10_CMD_SOFT_RESET, NULL, 0);
    if (ret == ESP_OK) {
        vTaskDelay(pdMS_TO_TICKS(20));
        ESP_LOGI(TAG, "AHT10 soft reset completed");
    }
    
    return ret;
}

esp_err_t aht10_is_calibrated(bool *calibrated)
{
    if (!calibrated || !g_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t data[1];
    esp_err_t ret = aht10_read_data(data, sizeof(data));
    if (ret != ESP_OK) {
        *calibrated = false;
        return ret;
    }
    
    *calibrated = (data[0] & AHT10_STATUS_CAL) != 0;
    return ESP_OK;
}

esp_err_t aht10_calibrate(void)
{
    if (!g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Calibrating AHT10 sensor");
    
    // Send calibration command
    uint8_t cmd_data[] = {0x08, 0x00};
    esp_err_t ret = aht10_write_cmd(AHT10_CMD_INIT, cmd_data, sizeof(cmd_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 calibration command failed");
        return ret;
    }
    
    // Wait for calibration to complete
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Check if calibration was successful
    bool calibrated;
    ret = aht10_is_calibrated(&calibrated);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to check calibration status");
        return ret;
    }
    
    if (calibrated) {
        ESP_LOGI(TAG, "AHT10 calibration successful");
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "AHT10 calibration failed");
        return ESP_ERR_INVALID_STATE;
    }
}

esp_err_t aht10_get_status(bool *busy, bool *calibrated)
{
    if (!busy || !calibrated || !g_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t data[1];
    esp_err_t ret = aht10_read_data(data, sizeof(data));
    if (ret != ESP_OK) {
        *busy = false;
        *calibrated = false;
        return ret;
    }
    
    *busy = (data[0] & AHT10_STATUS_BUSY) != 0;
    *calibrated = (data[0] & AHT10_STATUS_CAL) != 0;
    
    return ESP_OK;
}

esp_err_t aht10_deinit(void)
{
    if (!g_initialized) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Deinitializing AHT10 sensor");
    
    g_initialized = false;
    ESP_LOGI(TAG, "AHT10 sensor deinitialized");
    
    return ESP_OK;
} 