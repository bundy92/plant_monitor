#ifndef AHT10_H
#define AHT10_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

// AHT10 sensor configuration
typedef struct {
    i2c_port_t i2c_port;
    uint8_t sensor_addr;
    uint32_t i2c_freq_hz;
    int timeout_ms;
    bool initialized;
} aht10_config_t;

// AHT10 sensor data
typedef struct {
    float temperature;
    float humidity;
    uint32_t timestamp;
    bool valid;
} aht10_data_t;

// AHT10 sensor commands
#define AHT10_CMD_INITIALIZE    0xE1
#define AHT10_CMD_MEASURE       0xAC
#define AHT10_CMD_NORMAL        0xA8
#define AHT10_CMD_SOFT_RESET    0xBA

// AHT10 sensor addresses
#define AHT10_ADDR_1            0x38
#define AHT10_ADDR_2            0x39

// Default configuration
#define AHT10_DEFAULT_FREQ_HZ   100000
#define AHT10_DEFAULT_TIMEOUT_MS 1000

// Function prototypes
esp_err_t aht10_init(aht10_config_t* config);
esp_err_t aht10_deinit(aht10_config_t* config);
esp_err_t aht10_read_sensor(aht10_config_t* config, aht10_data_t* data);
esp_err_t aht10_reset_sensor(aht10_config_t* config);
bool aht10_is_initialized(const aht10_config_t* config);
esp_err_t aht10_validate_data(const aht10_data_t* data);

// Utility functions
esp_err_t aht10_scan_devices(i2c_port_t i2c_port);
esp_err_t aht10_get_default_config(aht10_config_t* config, uint8_t sensor_addr);

#ifdef __cplusplus
}
#endif

#endif // AHT10_H 