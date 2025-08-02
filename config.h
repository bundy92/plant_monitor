#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "TP-Link_0E28"
#define WIFI_PASS "44191207"

// Server Configuration
#define SERVER_URL "http://192.168.1.100:8080/data"
#define DATA_INTERVAL_MS 30000 // Send data every 30 seconds

// I2C Configuration for AHT10 sensors
#define I2C_MASTER_SCL_IO            GPIO_NUM_22      // I2C SCL pin
#define I2C_MASTER_SDA_IO            GPIO_NUM_21      // I2C SDA pin
#define I2C_MASTER_NUM               I2C_NUM_0        // I2C port number
#define I2C_MASTER_FREQ_HZ           100000           // I2C master clock frequency
#define I2C_MASTER_TX_BUF_DISABLE   0                // I2C master doesn't need buffer
#define I2C_MASTER_RX_BUF_DISABLE   0                // I2C master doesn't need buffer
#define I2C_MASTER_TIMEOUT_MS        1000             // I2C master timeout

// AHT10 Sensor Addresses (you can have multiple sensors)
#define AHT10_SENSOR_1_ADDR          0x38             // First AHT10 sensor address
#define AHT10_SENSOR_2_ADDR          0x39             // Second AHT10 sensor address (if different)

// Pin Assignments
#define SOIL_MOISTURE_PIN ADC_CHANNEL_0  // Soil moisture sensor (analog)
#define LIGHT_SENSOR_PIN ADC_CHANNEL_1   // Light sensor (analog)
#define LED_PIN GPIO_NUM_2               // Built-in LED for status indication

// Device Configuration
#define DEVICE_ID "esp32_plant_monitor"

#endif // CONFIG_H 