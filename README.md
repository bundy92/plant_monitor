# ESP32-C6 Plant Monitor

A comprehensive plant monitoring system using ESP32-C6 with AHT10 temperature/humidity sensors, soil moisture sensor, and light sensor. Data is transmitted to a Raspberry Pi Zero V2 for logging and web dashboard display.

## 🌱 Features

- **Temperature & Humidity Monitoring**: Dual AHT10 sensors for accurate readings
- **Soil Moisture Sensing**: Capacitive soil moisture sensor
- **Light Level Detection**: Photoresistor for ambient light monitoring
- **WiFi Connectivity**: Automatic connection and data transmission
- **Web Dashboard**: Real-time data visualization
- **Data Logging**: SQLite database storage on Raspberry Pi
- **RESTful API**: JSON data transmission over HTTP

## 🛠️ Hardware Requirements

### ESP32-C6 Setup
- **Microcontroller**: ESP32-C6 WROOM
- **Temperature/Humidity**: 2x AHT10 sensors
- **Soil Moisture**: Capacitive soil moisture sensor
- **Light Sensor**: Photoresistor with voltage divider
- **Power**: USB-C connection or 3.3V supply

### Raspberry Pi Zero V2
- **Server**: Flask web application
- **Database**: SQLite for data storage
- **Web Interface**: HTML/JavaScript dashboard

## 📋 Pin Configuration

### ESP32-C6 Pinout
```
I2C (AHT10 Sensors):
- SDA: GPIO 21
- SCL: GPIO 22
- VCC: 3.3V
- GND: GND

Analog Sensors:
- Soil Moisture: GPIO 4 (ADC1_CH3)
- Light Sensor: GPIO 5 (ADC1_CH4)

LED Indicator:
- Status LED: GPIO 2
```

### AHT10 Sensor Wiring
```
Sensor 1 (Address 0x38):
- VCC → 3.3V
- GND → GND
- SDA → GPIO 21
- SCL → GPIO 22

Sensor 2 (Address 0x39):
- VCC → 3.3V
- GND → GND
- SDA → GPIO 21
- SCL → GPIO 22
```

## 🚀 Quick Start

### 1. Prerequisites
```bash
# Install PlatformIO
pip install platformio

# Or use pipx (recommended)
pipx install platformio
```

### 2. Clone and Setup
```bash
git clone <repository-url>
cd plant_monitor
```

### 3. Configure WiFi
Edit `config.h`:
```cpp
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
#define SERVER_URL "http://your_raspberry_pi_ip:5000/data"
```

### 4. Build and Upload
```bash
# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

### 5. Test AHT10 Sensors
```bash
# Switch to test mode
./test_aht10.sh

# Or manually
cp simple_test.cpp src/main.cpp
pio run --target upload
pio device monitor
```

## 📁 Project Structure

```
plant_monitor/
├── src/
│   ├── main.cpp              # Main ESP32 firmware
│   └── CMakeLists.txt        # ESP-IDF component config
├── config.h                  # Configuration constants
├── platformio.ini           # PlatformIO configuration
├── partitions.csv           # Flash partition table
├── sdkconfig.defaults      # ESP-IDF defaults
├── simple_test.cpp         # AHT10 sensor test
├── i2c_scanner.cpp         # I2C device scanner
├── raspberry_pi_server.py  # Flask server for RPi
├── web_dashboard.html      # Web dashboard
├── requirements.txt        # Python dependencies
├── deploy.sh              # Deployment script
├── test_aht10.sh         # AHT10 testing script
├── test_build.sh         # Build verification
├── verify_project.sh      # Project verification
├── setup_guide.md        # Detailed setup guide
├── AHT10_SETUP.md        # AHT10 specific guide
├── SIMPLE_TEST_GUIDE.md  # Simple test instructions
├── QUICK_START.md        # Quick start guide
├── PROJECT_SUMMARY.md    # Project overview
└── README.md             # This file
```

## 🔧 Configuration

### WiFi Settings
Edit `config.h`:
```cpp
#define WIFI_SSID "your_network"
#define WIFI_PASSWORD "your_password"
#define SERVER_URL "http://192.168.1.100:5000/data"
```

### Sensor Configuration
```cpp
// AHT10 Sensor Addresses
#define AHT10_SENSOR_1_ADDR 0x38
#define AHT10_SENSOR_2_ADDR 0x39

// I2C Configuration
#define I2C_MASTER_SDA_IO GPIO_NUM_21
#define I2C_MASTER_SCL_IO GPIO_NUM_22
#define I2C_MASTER_FREQ_HZ 100000
```

### Data Transmission
```cpp
#define DATA_INTERVAL_MS 30000  // 30 seconds
#define DEVICE_ID "ESP32_PLANT_MONITOR_01"
```

## 📊 Data Format

### JSON Payload
```json
{
  "temperature": 23.45,
  "humidity": 45.67,
  "soil_moisture": 512,
  "light_level": 2048,
  "timestamp": 1640995200,
  "device_id": "ESP32_PLANT_MONITOR_01"
}
```

### Database Schema
```sql
CREATE TABLE sensor_data (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id TEXT NOT NULL,
    temperature REAL,
    humidity REAL,
    soil_moisture INTEGER,
    light_level INTEGER,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

## 🧪 Testing

### AHT10 Sensor Test
```bash
# Quick test
./test_aht10.sh

# Manual test
cp simple_test.cpp src/main.cpp
pio run --target upload
pio device monitor
```

### I2C Scanner
```bash
# Scan for I2C devices
cp i2c_scanner.cpp src/main.cpp
pio run --target upload
pio device monitor
```

### Build Verification
```bash
# Verify project structure
./verify_project.sh

# Test build without upload
./test_build.sh
```

## 🔍 Troubleshooting

### Common Issues

1. **AHT10 Not Found**
   - Check wiring: SDA→GPIO21, SCL→GPIO22
   - Verify power: VCC→3.3V, GND→GND
   - Run I2C scanner to detect devices

2. **WiFi Connection Failed**
   - Verify SSID and password in `config.h`
   - Check network availability
   - Monitor serial output for error messages

3. **Build Errors**
   - Ensure PlatformIO is installed: `pip install platformio`
   - Check ESP-IDF framework compatibility
   - Verify `extern "C" void app_main(void)` declaration

4. **Upload Issues**
   - Check USB connection
   - Verify correct COM port
   - Try different USB cable

### Debug Commands
```bash
# Check device connection
pio device list

# Monitor serial output
pio device monitor

# Clean build
pio run --target clean

# Rebuild
pio run --target clean && pio run
```

## 📈 Monitoring

### Serial Output
```
I (1234) AHT10_TEST: AHT10 Simple Test for ESP32-C6
I (1234) AHT10_TEST: I2C initialized successfully
I (1234) AHT10_TEST: Found I2C device at address: 0x38
I (1234) AHT10_TEST: Found I2C device at address: 0x39
I (1234) AHT10_TEST: Sensor 1 (0x38): Temperature: 23.45°C, Humidity: 45.67%
I (1234) AHT10_TEST: Sensor 2 (0x39): Temperature: 23.52°C, Humidity: 45.89%
```

### Web Dashboard
Access the web dashboard at `http://your_raspberry_pi_ip:5000` to view real-time sensor data and historical charts.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- ESP-IDF framework for ESP32 development
- PlatformIO for build system
- AHT10 sensor library and documentation
- Flask framework for web server
- SQLite for data storage

## 📞 Support

For issues and questions:
1. Check the troubleshooting section
2. Review the setup guides
3. Open an issue on GitHub
4. Check the documentation files

---

**Happy Plant Monitoring! 🌱📊** 