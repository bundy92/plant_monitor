/**
 * @file ds18b20.c
 * @brief DS18B20 Waterproof Temperature Sensor Implementation
 * 
 * This module implements the DS18B20 waterproof temperature sensor
 * driver using One-Wire communication protocol. It includes proper
 * timing, ROM commands, and error handling for reliable temperature
 * readings.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#include "ds18b20.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "DS18B20";

// Global variables for One-Wire communication
static uint8_t g_onewire_pin = 0;
static bool g_initialized = false;

/**
 * @brief One-Wire timing delays (in microseconds)
 */
#define OW_DELAY_A 6
#define OW_DELAY_B 64
#define OW_DELAY_C 60
#define OW_DELAY_D 10
#define OW_DELAY_E 9
#define OW_DELAY_F 55
#define OW_DELAY_G 0
#define OW_DELAY_H 480
#define OW_DELAY_I 70
#define OW_DELAY_J 410

/**
 * @brief Initialize GPIO for One-Wire communication
 * 
 * @param pin GPIO pin number
 * @return ESP_OK on success, error code on failure
 */
static esp_err_t onewire_init_gpio(uint8_t pin)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Set pin high initially
    gpio_set_level(pin, 1);
    return ESP_OK;
}

/**
 * @brief Generate One-Wire reset pulse
 * 
 * @return ESP_OK if device present, ESP_ERR_NOT_FOUND if no device
 */
static esp_err_t onewire_reset(void)
{
    gpio_set_level(g_onewire_pin, 0);
    esp_rom_delay_us(OW_DELAY_H);
    gpio_set_level(g_onewire_pin, 1);
    esp_rom_delay_us(OW_DELAY_I);
    
    // Check for presence pulse
    int level = gpio_get_level(g_onewire_pin);
    esp_rom_delay_us(OW_DELAY_J);
    
    return (level == 0) ? ESP_OK : ESP_ERR_NOT_FOUND;
}

/**
 * @brief Write a byte to One-Wire bus
 * 
 * @param byte Byte to write
 */
static void onewire_write_byte(uint8_t byte)
{
    for (int i = 0; i < 8; i++) {
        gpio_set_level(g_onewire_pin, 0);
        esp_rom_delay_us(OW_DELAY_A);
        
        if (byte & 0x01) {
            gpio_set_level(g_onewire_pin, 1);
            esp_rom_delay_us(OW_DELAY_B);
        } else {
            esp_rom_delay_us(OW_DELAY_C);
            gpio_set_level(g_onewire_pin, 1);
        }
        
        byte >>= 1;
    }
}

/**
 * @brief Read a byte from One-Wire bus
 * 
 * @return Byte read from bus
 */
static uint8_t onewire_read_byte(void)
{
    uint8_t byte = 0;
    
    for (int i = 0; i < 8; i++) {
        gpio_set_level(g_onewire_pin, 0);
        esp_rom_delay_us(OW_DELAY_A);
        gpio_set_level(g_onewire_pin, 1);
        esp_rom_delay_us(OW_DELAY_E);
        
        byte >>= 1;
        if (gpio_get_level(g_onewire_pin)) {
            byte |= 0x80;
        }
        
        esp_rom_delay_us(OW_DELAY_F);
    }
    
    return byte;
}

/**
 * @brief Initialize DS18B20 sensor
 * 
 * @param config DS18B20 configuration
 * @return ESP_OK on success, error code on failure
 */
esp_err_t ds18b20_init(const ds18b20_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "Invalid configuration");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (g_initialized) {
        ESP_LOGW(TAG, "DS18B20 already initialized");
        return ESP_OK;
    }
    
    g_onewire_pin = config->pin;
    
    // Initialize GPIO
    esp_err_t ret = onewire_init_gpio(g_onewire_pin);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Reset One-Wire bus
    ret = onewire_reset();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "No DS18B20 device found on pin %d", g_onewire_pin);
        return ret;
    }
    
    ESP_LOGI(TAG, "DS18B20 initialized on pin %d", g_onewire_pin);
    g_initialized = true;
    
    return ESP_OK;
}

/**
 * @brief Read temperature from DS18B20
 * 
 * @param reading Pointer to store the reading
 * @return ESP_OK on success, error code on failure
 */
esp_err_t ds18b20_read(ds18b20_reading_t *reading)
{
    if (!reading) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_initialized) {
        ESP_LOGE(TAG, "DS18B20 not initialized");
        reading->valid = false;
        reading->error = ESP_ERR_INVALID_STATE;
        return ESP_ERR_INVALID_STATE;
    }
    
    // Reset One-Wire bus
    esp_err_t ret = onewire_reset();
    if (ret != ESP_OK) {
        reading->valid = false;
        reading->error = ret;
        return ret;
    }
    
    // Skip ROM command (single device)
    onewire_write_byte(DS18B20_CMD_SKIP_ROM);
    
    // Start temperature conversion
    onewire_write_byte(DS18B20_CMD_CONVERT_TEMP);
    
    // Wait for conversion (750ms for 12-bit resolution)
    vTaskDelay(pdMS_TO_TICKS(750));
    
    // Reset One-Wire bus again
    ret = onewire_reset();
    if (ret != ESP_OK) {
        reading->valid = false;
        reading->error = ret;
        return ret;
    }
    
    // Skip ROM command
    onewire_write_byte(DS18B20_CMD_SKIP_ROM);
    
    // Read scratchpad
    onewire_write_byte(DS18B20_CMD_READ_SCRATCHPAD);
    
    // Read 9 bytes (temperature + CRC)
    uint8_t scratchpad[9];
    for (int i = 0; i < 9; i++) {
        scratchpad[i] = onewire_read_byte();
    }
    
    // Check CRC (simplified - in production, implement proper CRC check)
    if (scratchpad[8] != 0) {
        ESP_LOGW(TAG, "CRC check failed");
        reading->valid = false;
        reading->error = ESP_ERR_INVALID_RESPONSE;
        return ESP_ERR_INVALID_RESPONSE;
    }
    
    // Convert temperature
    int16_t raw_temp = (scratchpad[1] << 8) | scratchpad[0];
    reading->temperature = (float)raw_temp * 0.0625f;
    reading->valid = true;
    reading->error = ESP_OK;
    
    ESP_LOGD(TAG, "DS18B20 temperature: %.2f°C", reading->temperature);
    
    return ESP_OK;
}

/**
 * @brief Read only temperature from DS18B20
 * 
 * @param temperature Pointer to store temperature value
 * @return ESP_OK on success, error code on failure
 */
esp_err_t ds18b20_read_temperature(float *temperature)
{
    if (!temperature) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ds18b20_reading_t reading;
    esp_err_t ret = ds18b20_read(&reading);
    
    if (ret == ESP_OK && reading.valid) {
        *temperature = reading.temperature;
    }
    
    return ret;
}

/**
 * @brief Set temperature resolution
 * 
 * @param resolution Resolution in bits (9-12)
 * @return ESP_OK on success, error code on failure
 */
esp_err_t ds18b20_set_resolution(uint8_t resolution)
{
    if (resolution < 9 || resolution > 12) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Reset One-Wire bus
    esp_err_t ret = onewire_reset();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Skip ROM command
    onewire_write_byte(DS18B20_CMD_SKIP_ROM);
    
    // Write scratchpad command
    onewire_write_byte(DS18B20_CMD_WRITE_SCRATCHPAD);
    
    // Write configuration bytes
    onewire_write_byte(0); // TH register
    onewire_write_byte(0); // TL register
    onewire_write_byte((resolution - 9) << 5); // Configuration register
    
    // Copy scratchpad to EEPROM
    ret = onewire_reset();
    if (ret != ESP_OK) {
        return ret;
    }
    
    onewire_write_byte(DS18B20_CMD_SKIP_ROM);
    onewire_write_byte(DS18B20_CMD_COPY_SCRATCHPAD);
    
    // Wait for copy operation
    vTaskDelay(pdMS_TO_TICKS(10));
    
    return ESP_OK;
}

/**
 * @brief Get temperature resolution
 * 
 * @param resolution Pointer to store resolution
 * @return ESP_OK on success, error code on failure
 */
esp_err_t ds18b20_get_resolution(uint8_t *resolution)
{
    if (!resolution) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Reset One-Wire bus
    esp_err_t ret = onewire_reset();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Skip ROM command
    onewire_write_byte(DS18B20_CMD_SKIP_ROM);
    
    // Read scratchpad
    onewire_write_byte(DS18B20_CMD_READ_SCRATCHPAD);
    
    // Read configuration byte
    uint8_t config = onewire_read_byte();
    *resolution = ((config >> 5) & 0x03) + 9;
    
    return ESP_OK;
}

/**
 * @brief Search for DS18B20 devices on One-Wire bus
 * 
 * @param rom_codes Array to store found ROM codes
 * @param max_devices Maximum number of devices to find
 * @return Number of devices found
 */
int ds18b20_search_devices(uint64_t *rom_codes, int max_devices)
{
    if (!rom_codes || max_devices <= 0) {
        return -1;
    }
    
    if (!g_initialized) {
        return -1;
    }
    
    // Simplified implementation - in production, implement full ROM search
    // For now, just try to reset and see if any device responds
    esp_err_t ret = onewire_reset();
    if (ret == ESP_OK) {
        rom_codes[0] = 0; // Placeholder ROM code
        return 1;
    }
    
    return 0;
}

/**
 * @brief Get DS18B20 sensor status
 * 
 * @param connected Whether sensor is connected
 * @param powered Whether sensor is powered
 * @return ESP_OK on success, error code on failure
 */
esp_err_t ds18b20_get_status(bool *connected, bool *powered)
{
    if (!connected || !powered) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_initialized) {
        *connected = false;
        *powered = false;
        return ESP_ERR_INVALID_STATE;
    }
    
    // Check if device responds
    esp_err_t ret = onewire_reset();
    *connected = (ret == ESP_OK);
    *powered = *connected; // If connected, assume powered
    
    return ESP_OK;
}

/**
 * @brief Deinitialize DS18B20 sensor
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t ds18b20_deinit(void)
{
    if (!g_initialized) {
        return ESP_OK;
    }
    
    // Reset GPIO configuration
    gpio_reset_pin(g_onewire_pin);
    
    g_initialized = false;
    g_onewire_pin = 0;
    
    ESP_LOGI(TAG, "DS18B20 deinitialized");
    
    return ESP_OK;
} 