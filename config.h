/**
 * @file config.h
 * @brief Configuration header for ESP32-C6 Plant Monitoring System
 * 
 * This header file contains all configurable parameters for the plant monitoring
 * system including WiFi credentials, server settings, I2C configuration,
 * sensor addresses, and pin assignments.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#ifndef CONFIG_H
#define CONFIG_H

/**
 * @brief WiFi Configuration
 * 
 * These constants define the WiFi network credentials for connecting
 * to the local network and transmitting data to the server.
 */
#define WIFI_SSID "TP-Link_0E28"         /**< WiFi network SSID */
#define WIFI_PASS "44191207"              /**< WiFi network password */

/**
 * @brief Server Configuration
 * 
 * These constants define the server endpoint and data transmission
 * interval for sending sensor data to the Raspberry Pi server.
 */
#define SERVER_URL "http://192.168.1.100:8080/data"  /**< Server endpoint URL */
#define DATA_INTERVAL_MS 30000                        /**< Data transmission interval in milliseconds */

/**
 * @brief I2C Configuration for AHT10 Sensors
 * 
 * These constants define the I2C bus configuration for communicating
 * with AHT10 temperature and humidity sensors.
 */
#define I2C_MASTER_SCL_IO            GPIO_NUM_22      /**< I2C SCL pin */
#define I2C_MASTER_SDA_IO            GPIO_NUM_21      /**< I2C SDA pin */
#define I2C_MASTER_NUM               I2C_NUM_0        /**< I2C port number */
#define I2C_MASTER_FREQ_HZ           100000           /**< I2C master clock frequency in Hz */
#define I2C_MASTER_TX_BUF_DISABLE   0                /**< I2C master TX buffer disable flag */
#define I2C_MASTER_RX_BUF_DISABLE   0                /**< I2C master RX buffer disable flag */
#define I2C_MASTER_TIMEOUT_MS        1000             /**< I2C master timeout in milliseconds */

/**
 * @brief AHT10 Sensor Addresses
 * 
 * These constants define the I2C addresses for AHT10 sensors.
 * Multiple sensors can be used with different addresses.
 */
#define AHT10_SENSOR_1_ADDR          0x38             /**< First AHT10 sensor address */
#define AHT10_SENSOR_2_ADDR          0x39             /**< Second AHT10 sensor address */

/**
 * @brief Pin Assignments
 * 
 * These constants define the GPIO pins used for various sensors
 * and status indicators in the plant monitoring system.
 */
#define SOIL_MOISTURE_PIN ADC_CHANNEL_0  /**< Soil moisture sensor (analog input) */
#define LIGHT_SENSOR_PIN ADC_CHANNEL_1   /**< Light sensor (analog input) */
#define LED_PIN GPIO_NUM_2               /**< Built-in LED for status indication */

/**
 * @brief Device Configuration
 * 
 * These constants define device-specific configuration parameters.
 */
#define DEVICE_ID "esp32_plant_monitor"  /**< Unique device identifier */

#endif // CONFIG_H 