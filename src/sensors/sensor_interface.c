/**
 * @file sensor_interface.c
 * @brief Modular Sensor Interface Implementation
 * 
 * This module implements the unified sensor interface that supports
 * multiple sensor types including AHT10, DS18B20, GY-302, and analog
 * sensors. It provides a clean abstraction layer for sensor management
 * and data collection.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#include "sensor_interface.h"
#include "aht10.h"
#include "ds18b20.h"
#include "gy302.h"
#include "driver/i2c.h"
#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "SENSOR_INTERFACE";

// Global variables
static sensor_interface_config_t g_config;
static bool g_initialized = false;
static adc_oneshot_unit_handle_t g_adc_handle = NULL;

/**
 * @brief Initialize I2C master for sensors
 * 
 * @param sda_pin SDA pin number
 * @param scl_pin SCL pin number
 * @param freq I2C frequency in Hz
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t i2c_master_init(uint8_t sda_pin, uint8_t scl_pin, uint32_t freq)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = freq,
    };
    
    esp_err_t ret = i2c_param_config(I2C_NUM_0, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure I2C: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to install I2C driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

/**
 * @brief Initialize ADC for analog sensors
 * 
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t adc_init(void)
{
    // Initialize ADC oneshot driver
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &g_adc_handle));
    
    // Configure ADC channel for soil moisture sensor
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(g_adc_handle, g_config.adc_soil_pin, &config));
    
    // Configure light sensor ADC channel
    ESP_ERROR_CHECK(adc_oneshot_config_channel(g_adc_handle, g_config.adc_light_pin, &config));
    
    return ESP_OK;
}

/**
 * @brief Read AHT10 sensor
 * 
 * @param config Sensor configuration
 * @param reading Pointer to store reading
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t read_aht10_sensor(const sensor_config_t *config, sensor_reading_t *reading)
{
    aht10_config_t aht10_config = {
        .address = config->address,
        .sda_pin = g_config.i2c_sda_pin,
        .scl_pin = g_config.i2c_scl_pin,
        .i2c_freq = g_config.i2c_frequency,
        .enabled = config->enabled
    };
    
    esp_err_t ret = aht10_init(&aht10_config);
    if (ret != ESP_OK) {
        reading->valid = false;
        reading->error = ret;
        return ret;
    }
    
    aht10_reading_t aht10_reading;
    ret = aht10_read(&aht10_reading);
    if (ret == ESP_OK && aht10_reading.valid) {
        reading->temperature = aht10_reading.temperature;
        reading->humidity = aht10_reading.humidity;
        reading->valid = true;
        reading->error = ESP_OK;
    } else {
        reading->valid = false;
        reading->error = ret;
    }
    
    aht10_deinit();
    return ret;
}

/**
 * @brief Read DS18B20 sensor
 * 
 * @param config Sensor configuration
 * @param reading Pointer to store reading
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t read_ds18b20_sensor(const sensor_config_t *config, sensor_reading_t *reading)
{
    ds18b20_config_t ds18b20_config = {
        .pin = config->pin,
        .resolution = 12,
        .enabled = config->enabled,
        .rom_code = 0
    };
    
    esp_err_t ret = ds18b20_init(&ds18b20_config);
    if (ret != ESP_OK) {
        reading->valid = false;
        reading->error = ret;
        return ret;
    }
    
    ds18b20_reading_t ds18b20_reading;
    ret = ds18b20_read(&ds18b20_reading);
    if (ret == ESP_OK && ds18b20_reading.valid) {
        reading->temperature = ds18b20_reading.temperature;
        reading->humidity = 0.0f; // DS18B20 doesn't measure humidity
        reading->valid = true;
        reading->error = ESP_OK;
    } else {
        reading->valid = false;
        reading->error = ret;
    }
    
    ds18b20_deinit();
    return ret;
}

/**
 * @brief Read GY-302 sensor
 * 
 * @param config Sensor configuration
 * @param reading Pointer to store reading
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t read_gy302_sensor(const sensor_config_t *config, sensor_reading_t *reading)
{
    gy302_config_t gy302_config = {
        .address = config->address,
        .sda_pin = g_config.i2c_sda_pin,
        .scl_pin = g_config.i2c_scl_pin,
        .i2c_freq = g_config.i2c_frequency,
        .mode = GY302_MODE_ONE_H,
        .enabled = config->enabled
    };
    
    esp_err_t ret = gy302_init(&gy302_config);
    if (ret != ESP_OK) {
        reading->valid = false;
        reading->error = ret;
        return ret;
    }
    
    gy302_reading_t gy302_reading;
    ret = gy302_read(&gy302_reading);
    if (ret == ESP_OK && gy302_reading.valid) {
        reading->lux = gy302_reading.lux;
        reading->light_level = (uint16_t)(gy302_reading.lux / 10.0f); // Convert to ADC-like scale
        reading->valid = true;
        reading->error = ESP_OK;
    } else {
        reading->valid = false;
        reading->error = ret;
    }
    
    gy302_deinit();
    return ret;
}

/**
 * @brief Read analog soil moisture sensor
 * 
 * @param config Sensor configuration
 * @param reading Pointer to store reading
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t read_soil_moisture_sensor(const sensor_config_t *config, sensor_reading_t *reading)
{
    int adc_raw = 0;
    
    esp_err_t ret = adc_oneshot_read(g_adc_handle, g_config.adc_soil_pin, &adc_raw);
    if (ret != ESP_OK) {
        reading->valid = false;
        reading->error = ret;
        return ret;
    }
    
    // Store raw ADC value (calibration can be added later if needed)
    reading->soil_moisture = (uint16_t)adc_raw;
    reading->valid = true;
    reading->error = ESP_OK;
    
    return ESP_OK;
}

/**
 * @brief Read analog light sensor
 * 
 * @param config Sensor configuration
 * @param reading Pointer to store reading
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t read_light_sensor(const sensor_config_t *config, sensor_reading_t *reading)
{
    int adc_raw = 0;
    
    esp_err_t ret = adc_oneshot_read(g_adc_handle, g_config.adc_light_pin, &adc_raw);
    if (ret != ESP_OK) {
        reading->valid = false;
        reading->error = ret;
        return ret;
    }
    
    // Store raw ADC value (calibration can be added later if needed)
    reading->light_level = (uint16_t)adc_raw;
    reading->valid = true;
    reading->error = ESP_OK;
    
    return ESP_OK;
}

/**
 * @brief Initialize the sensor interface
 * 
 * @param config Sensor interface configuration
 * @return ESP_OK on success, error code on failure
 */
esp_err_t sensor_interface_init(const sensor_interface_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "Invalid configuration");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (g_initialized) {
        ESP_LOGW(TAG, "Sensor interface already initialized");
        return ESP_OK;
    }
    
    memcpy(&g_config, config, sizeof(sensor_interface_config_t));
    
    // Initialize I2C
    esp_err_t ret = i2c_master_init(g_config.i2c_sda_pin, g_config.i2c_scl_pin, g_config.i2c_frequency);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Initialize ADC
    ret = adc_init();
    if (ret != ESP_OK) {
        return ret;
    }
    
    ESP_LOGI(TAG, "Sensor interface initialized with %d sensors", g_config.sensor_count);
    g_initialized = true;
    
    return ESP_OK;
}

/**
 * @brief Read all configured sensors
 * 
 * @param readings Array to store sensor readings
 * @param max_readings Maximum number of readings to store
 * @return Number of valid readings, negative on error
 */
int sensor_interface_read_all(sensor_reading_t *readings, int max_readings)
{
    if (!readings || max_readings <= 0) {
        return -1;
    }
    
    if (!g_initialized) {
        ESP_LOGE(TAG, "Sensor interface not initialized");
        return -1;
    }
    
    int valid_readings = 0;
    
    for (int i = 0; i < g_config.sensor_count && i < max_readings; i++) {
        const sensor_config_t *config = &g_config.sensors[i];
        
        if (!config->enabled) {
            continue;
        }
        
        // Initialize reading structure
        readings[i].temperature = 0.0f;
        readings[i].humidity = 0.0f;
        readings[i].soil_moisture = 0;
        readings[i].light_level = 0;
        readings[i].lux = 0.0f;
        readings[i].valid = false;
        readings[i].error = ESP_OK;
        
        esp_err_t ret = ESP_OK;
        
        switch (config->type) {
            case SENSOR_TYPE_AHT10:
                ret = read_aht10_sensor(config, &readings[i]);
                break;
                
            case SENSOR_TYPE_DS18B20:
                ret = read_ds18b20_sensor(config, &readings[i]);
                break;
                
            case SENSOR_TYPE_GY302:
                ret = read_gy302_sensor(config, &readings[i]);
                break;
                
            case SENSOR_TYPE_SOIL_MOISTURE:
                ret = read_soil_moisture_sensor(config, &readings[i]);
                break;
                
            case SENSOR_TYPE_LIGHT:
                ret = read_light_sensor(config, &readings[i]);
                break;
                
            default:
                ESP_LOGW(TAG, "Unknown sensor type: %d", config->type);
                readings[i].error = ESP_ERR_INVALID_ARG;
                break;
        }
        
        if (ret == ESP_OK && readings[i].valid) {
            valid_readings++;
            ESP_LOGD(TAG, "Sensor %s: T=%.2f°C, H=%.2f%%, SM=%d, L=%d, Lux=%.1f",
                     config->name, readings[i].temperature, readings[i].humidity,
                     readings[i].soil_moisture, readings[i].light_level, readings[i].lux);
        } else {
            ESP_LOGW(TAG, "Failed to read sensor %s: %s", config->name, esp_err_to_name(ret));
        }
    }
    
    return valid_readings;
}

/**
 * @brief Read a specific sensor by type
 * 
 * @param type Sensor type to read
 * @param reading Pointer to store the reading
 * @return ESP_OK on success, error code on failure
 */
esp_err_t sensor_interface_read_sensor(sensor_type_t type, sensor_reading_t *reading)
{
    if (!reading) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Find first sensor of the specified type
    for (int i = 0; i < g_config.sensor_count; i++) {
        if (g_config.sensors[i].type == type && g_config.sensors[i].enabled) {
            return sensor_interface_read_all(reading, 1);
        }
    }
    
    return ESP_ERR_NOT_FOUND;
}

/**
 * @brief Scan for I2C devices
 * 
 * @return Number of devices found
 */
int sensor_interface_scan_i2c(void)
{
    if (!g_initialized) {
        return -1;
    }
    
    int device_count = 0;
    ESP_LOGI(TAG, "Scanning I2C bus...");
    
    for (uint8_t address = 1; address < 127; address++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Found I2C device at address 0x%02X", address);
            device_count++;
        }
    }
    
    ESP_LOGI(TAG, "I2C scan complete, found %d devices", device_count);
    return device_count;
}

/**
 * @brief Get sensor status
 * 
 * @param working_sensors Number of working sensors
 * @param total_sensors Total number of configured sensors
 * @return ESP_OK on success, error code on failure
 */
esp_err_t sensor_interface_get_status(int *working_sensors, int *total_sensors)
{
    if (!working_sensors || !total_sensors) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_initialized) {
        *working_sensors = 0;
        *total_sensors = 0;
        return ESP_ERR_INVALID_STATE;
    }
    
    *total_sensors = g_config.sensor_count;
    *working_sensors = 0;
    
    // Test each sensor
    sensor_reading_t test_reading;
    for (int i = 0; i < g_config.sensor_count; i++) {
        if (g_config.sensors[i].enabled) {
            if (sensor_interface_read_sensor(g_config.sensors[i].type, &test_reading) == ESP_OK) {
                (*working_sensors)++;
            }
        }
    }
    
    return ESP_OK;
}

/**
 * @brief Deinitialize the sensor interface
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t sensor_interface_deinit(void)
{
    if (!g_initialized) {
        return ESP_OK;
    }
    
    // Deinitialize ADC
    if (g_adc_handle) {
        adc_oneshot_del_unit(g_adc_handle);
        g_adc_handle = NULL;
    }
    
    // Deinitialize I2C
    i2c_driver_delete(I2C_NUM_0);
    
    g_initialized = false;
    memset(&g_config, 0, sizeof(sensor_interface_config_t));
    
    ESP_LOGI(TAG, "Sensor interface deinitialized");
    
    return ESP_OK;
} 