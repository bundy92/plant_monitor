#include "aht10.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "AHT10";

// Default I2C pins
#define AHT10_DEFAULT_SDA_PIN    GPIO_NUM_21
#define AHT10_DEFAULT_SCL_PIN    GPIO_NUM_22

esp_err_t aht10_get_default_config(aht10_config_t* config, uint8_t sensor_addr) {
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    config->i2c_port = I2C_NUM_0;
    config->sensor_addr = sensor_addr;
    config->i2c_freq_hz = AHT10_DEFAULT_FREQ_HZ;
    config->timeout_ms = AHT10_DEFAULT_TIMEOUT_MS;
    config->initialized = false;
    
    return ESP_OK;
}

esp_err_t aht10_init(aht10_config_t* config) {
    if (config == NULL) {
        ESP_LOGE(TAG, "Invalid config pointer");
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Initializing AHT10 sensor at address 0x%02X", config->sensor_addr);
    
    // Configure I2C
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = AHT10_DEFAULT_SDA_PIN,
        .scl_io_num = AHT10_DEFAULT_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = config->i2c_freq_hz,
        },
    };
    
    esp_err_t ret = i2c_param_config(config->i2c_port, &i2c_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C parameter config failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Try to install I2C driver, ignore if already installed
    ret = i2c_driver_install(config->i2c_port, i2c_conf.mode, 0, 0, 0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // If driver was already installed, that's fine
    if (ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGI(TAG, "I2C driver already installed");
    }
    
    // Send soft reset command
    ret = aht10_reset_sensor(config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 reset failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(20)); // Wait for reset
    
    // Send initialization command
    uint8_t init_cmd[] = {AHT10_CMD_INITIALIZE, 0x08, 0x00};
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (config->sensor_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, init_cmd, 3, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(config->i2c_port, cmd, config->timeout_ms / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT10 initialization failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(10)); // Wait for initialization
    
    config->initialized = true;
    ESP_LOGI(TAG, "AHT10 sensor initialized successfully");
    
    return ESP_OK;
}

esp_err_t aht10_deinit(aht10_config_t* config) {
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = i2c_driver_delete(config->i2c_port);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver delete failed: %s", esp_err_to_name(ret));
    }
    
    config->initialized = false;
    return ret;
}

esp_err_t aht10_reset_sensor(aht10_config_t* config) {
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t reset_cmd[] = {AHT10_CMD_SOFT_RESET};
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (config->sensor_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, reset_cmd, 1, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(config->i2c_port, cmd, config->timeout_ms / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    return ret;
}

esp_err_t aht10_read_sensor(aht10_config_t* config, aht10_data_t* data) {
    if (config == NULL || data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!config->initialized) {
        ESP_LOGE(TAG, "Sensor not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Send measurement command
    uint8_t measure_cmd[] = {AHT10_CMD_MEASURE, 0x33, 0x00};
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (config->sensor_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, measure_cmd, 3, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(config->i2c_port, cmd, config->timeout_ms / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Measurement command failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(80)); // Wait for measurement
    
    // Read 6 bytes of data
    uint8_t sensor_data[6];
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (config->sensor_addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, sensor_data, 6, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(config->i2c_port, cmd, config->timeout_ms / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Read data failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Check if measurement is ready
    if (sensor_data[0] & 0x80) {
        ESP_LOGE(TAG, "Measurement not ready");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Calculate humidity (20-bit value)
    uint32_t humidity_raw = ((uint32_t)(sensor_data[1] & 0x0F) << 16) | 
                           ((uint32_t)sensor_data[2] << 8) | 
                           sensor_data[3];
    data->humidity = (float)humidity_raw * 100.0 / 1048576.0;
    
    // Calculate temperature (20-bit value)
    uint32_t temp_raw = ((uint32_t)(sensor_data[3] & 0x0F) << 16) | 
                        ((uint32_t)sensor_data[4] << 8) | 
                        sensor_data[5];
    data->temperature = (float)temp_raw * 200.0 / 1048576.0 - 50.0;
    
    data->timestamp = esp_timer_get_time() / 1000; // Convert to milliseconds
    data->valid = true;
    
    ESP_LOGD(TAG, "Temperature: %.2f°C, Humidity: %.2f%%", data->temperature, data->humidity);
    
    return ESP_OK;
}

bool aht10_is_initialized(const aht10_config_t* config) {
    return (config != NULL) && config->initialized;
}

esp_err_t aht10_validate_data(const aht10_data_t* data) {
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!data->valid) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Check temperature range (-40 to 85°C)
    if (data->temperature < -40.0f || data->temperature > 85.0f) {
        ESP_LOGW(TAG, "Temperature out of range: %.2f°C", data->temperature);
        return ESP_ERR_INVALID_STATE;
    }
    
    // Check humidity range (0 to 100%)
    if (data->humidity < 0.0f || data->humidity > 100.0f) {
        ESP_LOGW(TAG, "Humidity out of range: %.2f%%", data->humidity);
        return ESP_ERR_INVALID_STATE;
    }
    
    return ESP_OK;
}

esp_err_t aht10_scan_devices(i2c_port_t i2c_port) {
    ESP_LOGI(TAG, "Scanning I2C bus for AHT10 devices...");
    
    int found_devices = 0;
    
    for (int i = 0; i < 128; i++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Found I2C device at address: 0x%02X", i);
            found_devices++;
            
            // Check if it's an AHT10 sensor
            if (i == AHT10_ADDR_1 || i == AHT10_ADDR_2) {
                ESP_LOGI(TAG, "  -> This looks like an AHT10 sensor!");
            }
        }
    }
    
    ESP_LOGI(TAG, "I2C scan complete! Found %d devices", found_devices);
    
    if (found_devices == 0) {
        ESP_LOGW(TAG, "No I2C devices found. Check wiring and power connections.");
        return ESP_ERR_NOT_FOUND;
    }
    
    return ESP_OK;
} 