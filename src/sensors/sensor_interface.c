/**
 * @file sensor_interface.c
 * @brief Modular Sensor Interface Implementation
 * 
 * This module implements the unified sensor interface for different
 * sensor types including AHT10, DHT, and analog sensors. It provides
 * a clean abstraction layer for sensor management and reading.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#include "sensor_interface.h"
#include "aht10.h"
#include <string.h>
#include <esp_log.h>
#include <driver/i2c.h>
#include <driver/adc.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>

static const char *TAG = "SENSOR_INTERFACE";

// Global variables
static sensor_interface_config_t g_config;
static bool g_initialized = false;
static adc_oneshot_unit_handle_t g_adc_handle = NULL;
static adc_cali_handle_t g_adc_cali_handle = NULL;

/**
 * @brief Initialize I2C master
 * 
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t i2c_master_init(void)
{
    ESP_LOGI(TAG, "Initializing I2C master");
    
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = g_config.i2c_sda_pin,
        .scl_io_num = g_config.i2c_scl_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = g_config.i2c_frequency,
        },
    };
    
    esp_err_t ret = i2c_param_config(I2C_NUM_0, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C parameter config failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "I2C master initialized successfully");
    return ESP_OK;
}

/**
 * @brief Initialize ADC for analog sensors
 * 
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t adc_init(void)
{
    ESP_LOGI(TAG, "Initializing ADC");
    
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    
    esp_err_t ret = adc_oneshot_new_unit(&init_config, &g_adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC unit init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Configure ADC channels
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };
    
    ret = adc_oneshot_config_channel(g_adc_handle, ADC_CHANNEL_0, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC channel 0 config failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = adc_oneshot_config_channel(g_adc_handle, ADC_CHANNEL_1, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC channel 1 config failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "ADC initialized successfully");
    return ESP_OK;
}

esp_err_t sensor_interface_init(const sensor_interface_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Initializing Sensor Interface");
    
    // Copy configuration
    memcpy(&g_config, config, sizeof(sensor_interface_config_t));
    
    // Initialize I2C
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C initialization failed");
        return ret;
    }
    
    // Initialize ADC
    ret = adc_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC initialization failed");
        return ret;
    }
    
    // Initialize individual sensors
    for (int i = 0; i < g_config.sensor_count; i++) {
        if (g_config.sensors[i].enabled) {
            ESP_LOGI(TAG, "Initializing sensor: %s", g_config.sensors[i].name);
            
            switch (g_config.sensors[i].type) {
                case SENSOR_TYPE_AHT10: {
                    aht10_config_t aht10_config = {
                        .address = g_config.sensors[i].address,
                        .sda_pin = g_config.i2c_sda_pin,
                        .scl_pin = g_config.i2c_scl_pin,
                        .i2c_freq = g_config.i2c_frequency,
                        .enabled = true
                    };
                    ret = aht10_init(&aht10_config);
                    if (ret != ESP_OK) {
                        ESP_LOGE(TAG, "AHT10 sensor initialization failed: %s", esp_err_to_name(ret));
                    } else {
                        ESP_LOGI(TAG, "AHT10 sensor initialized successfully");
                    }
                    break;
                }
                case SENSOR_TYPE_SOIL_MOISTURE:
                case SENSOR_TYPE_LIGHT:
                    // Analog sensors are handled by ADC
                    ESP_LOGI(TAG, "Analog sensor configured: %s", g_config.sensors[i].name);
                    break;
                default:
                    ESP_LOGW(TAG, "Unknown sensor type: %d", g_config.sensors[i].type);
                    break;
            }
        }
    }
    
    g_initialized = true;
    ESP_LOGI(TAG, "Sensor Interface initialized successfully");
    return ESP_OK;
}

int sensor_interface_read_all(sensor_reading_t *readings, int max_readings)
{
    if (!readings || max_readings <= 0 || !g_initialized) {
        return -1;
    }
    
    int valid_readings = 0;
    
    for (int i = 0; i < g_config.sensor_count && valid_readings < max_readings; i++) {
        if (!g_config.sensors[i].enabled) {
            continue;
        }
        
        sensor_reading_t *reading = &readings[valid_readings];
        memset(reading, 0, sizeof(sensor_reading_t));
        
        esp_err_t ret = ESP_OK;
        
        switch (g_config.sensors[i].type) {
            case SENSOR_TYPE_AHT10: {
                aht10_reading_t aht10_reading;
                ret = aht10_read(&aht10_reading);
                if (ret == ESP_OK && aht10_reading.valid) {
                    reading->temperature = aht10_reading.temperature;
                    reading->humidity = aht10_reading.humidity;
                    reading->valid = true;
                    valid_readings++;
                } else {
                    reading->error = ret;
                    reading->valid = false;
                }
                break;
            }
            case SENSOR_TYPE_SOIL_MOISTURE: {
                int adc_reading;
                ret = adc_oneshot_read(g_adc_handle, ADC_CHANNEL_0, &adc_reading);
                if (ret == ESP_OK) {
                    reading->soil_moisture = (uint16_t)adc_reading;
                    reading->valid = true;
                    valid_readings++;
                } else {
                    reading->error = ret;
                    reading->valid = false;
                }
                break;
            }
            case SENSOR_TYPE_LIGHT: {
                int adc_reading;
                ret = adc_oneshot_read(g_adc_handle, ADC_CHANNEL_1, &adc_reading);
                if (ret == ESP_OK) {
                    reading->light_level = (uint16_t)adc_reading;
                    reading->valid = true;
                    valid_readings++;
                } else {
                    reading->error = ret;
                    reading->valid = false;
                }
                break;
            }
            default:
                ESP_LOGW(TAG, "Unknown sensor type: %d", g_config.sensors[i].type);
                break;
        }
    }
    
    return valid_readings;
}

esp_err_t sensor_interface_read_sensor(sensor_type_t type, sensor_reading_t *reading)
{
    if (!reading || !g_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memset(reading, 0, sizeof(sensor_reading_t));
    
    // Find the first sensor of the requested type
    for (int i = 0; i < g_config.sensor_count; i++) {
        if (g_config.sensors[i].enabled && g_config.sensors[i].type == type) {
            switch (type) {
                case SENSOR_TYPE_AHT10: {
                    aht10_reading_t aht10_reading;
                    esp_err_t ret = aht10_read(&aht10_reading);
                    if (ret == ESP_OK && aht10_reading.valid) {
                        reading->temperature = aht10_reading.temperature;
                        reading->humidity = aht10_reading.humidity;
                        reading->valid = true;
                        return ESP_OK;
                    } else {
                        reading->error = ret;
                        reading->valid = false;
                        return ret;
                    }
                }
                case SENSOR_TYPE_SOIL_MOISTURE: {
                    int adc_reading;
                    esp_err_t ret = adc_oneshot_read(g_adc_handle, ADC_CHANNEL_0, &adc_reading);
                    if (ret == ESP_OK) {
                        reading->soil_moisture = (uint16_t)adc_reading;
                        reading->valid = true;
                        return ESP_OK;
                    } else {
                        reading->error = ret;
                        reading->valid = false;
                        return ret;
                    }
                }
                case SENSOR_TYPE_LIGHT: {
                    int adc_reading;
                    esp_err_t ret = adc_oneshot_read(g_adc_handle, ADC_CHANNEL_1, &adc_reading);
                    if (ret == ESP_OK) {
                        reading->light_level = (uint16_t)adc_reading;
                        reading->valid = true;
                        return ESP_OK;
                    } else {
                        reading->error = ret;
                        reading->valid = false;
                        return ret;
                    }
                }
                default:
                    return ESP_ERR_NOT_SUPPORTED;
            }
        }
    }
    
    return ESP_ERR_NOT_FOUND;
}

int sensor_interface_scan_i2c(void)
{
    if (!g_initialized) {
        return -1;
    }
    
    ESP_LOGI(TAG, "Scanning I2C devices...");
    
    int device_count = 0;
    for (uint8_t address = 1; address < 127; address++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Found I2C device at address: 0x%02x", address);
            device_count++;
        }
    }
    
    ESP_LOGI(TAG, "I2C scan complete, found %d devices", device_count);
    return device_count;
}

esp_err_t sensor_interface_get_status(int *working_sensors, int *total_sensors)
{
    if (!working_sensors || !total_sensors || !g_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *total_sensors = g_config.sensor_count;
    *working_sensors = 0;
    
    // Count enabled sensors
    for (int i = 0; i < g_config.sensor_count; i++) {
        if (g_config.sensors[i].enabled) {
            (*working_sensors)++;
        }
    }
    
    return ESP_OK;
}

esp_err_t sensor_interface_deinit(void)
{
    if (!g_initialized) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Deinitializing Sensor Interface");
    
    // Deinitialize AHT10 sensors
    aht10_deinit();
    
    // Deinitialize ADC
    if (g_adc_handle) {
        adc_oneshot_del_unit(g_adc_handle);
        g_adc_handle = NULL;
    }
    
    // Deinitialize I2C
    i2c_driver_delete(I2C_NUM_0);
    
    g_initialized = false;
    ESP_LOGI(TAG, "Sensor Interface deinitialized");
    
    return ESP_OK;
} 