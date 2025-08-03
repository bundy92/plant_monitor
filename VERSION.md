# ESP32 Plant Monitor - Version History

## v0.1.0 (2024-08-03)

### 🎉 Initial Release

This is the first public release of the ESP32 Plant Monitor system, featuring a comprehensive plant monitoring solution with modular sensor support and web dashboard integration.

### ✨ Features

#### Core System
- **Modular Sensor Architecture**: Support for multiple sensor types with unified interface
- **ESP32-C6 Support**: Optimized for ESP32-C6-DevKitC-1 development board
- **Real-time Monitoring**: Continuous sensor data collection and health assessment
- **Plant Health Algorithm**: Intelligent plant health scoring based on environmental conditions

#### Supported Sensors
- **AHT10**: Temperature and humidity sensors (I2C)
- **DS18B20**: Waterproof temperature sensors (One-Wire)
- **GY-302**: Digital light intensity sensors (I2C)
- **Analog Sensors**: Soil moisture and light level sensors (ADC)

#### Display Support
- **Console Output**: Real-time sensor data display via serial console
- **SSD1306 OLED**: Built-in display support for ESP32 DevKit boards
- **Web Dashboard**: HTML-based dashboard for remote monitoring

#### Development Features
- **Terminal Safety**: Enhanced terminal configuration to prevent hanging in Cursor IDE
- **Comprehensive Testing**: Unit tests, integration tests, and system verification
- **Documentation**: Complete setup guides and hardware documentation
- **Raspberry Pi Server**: Backend server for data collection and storage

### 🔧 Technical Specifications

#### Hardware Requirements
- ESP32-C6-DevKitC-1 development board
- AHT10 temperature/humidity sensors
- DS18B20 waterproof temperature sensors
- GY-302 light intensity sensors
- Analog soil moisture and light sensors
- Optional: SSD1306 OLED display

#### Software Stack
- **Framework**: ESP-IDF 5.5.0
- **Platform**: PlatformIO
- **Language**: C/C++
- **Backend**: Python Flask server (Raspberry Pi)
- **Frontend**: HTML/CSS/JavaScript dashboard

#### Configuration
- **WiFi**: Configurable network credentials
- **I2C**: SDA=21, SCL=22, 100kHz frequency
- **ADC**: 12-bit resolution, 12dB attenuation
- **Serial**: 115200 baud rate for monitoring

### 🛠️ Build System

#### PlatformIO Configuration
- **Board**: esp32-c6-devkitc-1
- **Framework**: espidf
- **Monitor Speed**: 115200 baud
- **Upload Speed**: 921600 baud
- **Flash Size**: 2MB (configured)

#### Terminal Safety Features
- **Timeout Protection**: All commands wrapped with explicit timeouts
- **Cursor Integration**: Auto-detection and configuration for Cursor IDE
- **Error Handling**: Proper timeout vs. failure detection
- **Safe Aliases**: PlatformIO commands with built-in safety

### 📁 Project Structure

```
plant_monitor/
├── src/                    # ESP32 source code
│   ├── main.cpp           # Main application entry point
│   ├── sensors/           # Sensor drivers and interface
│   └── display/           # Display interface
├── raspberry_pi/          # Backend server
├── test/                  # Test suite
├── docs/                  # Documentation
├── scripts/               # Build and deployment scripts
└── config/               # Configuration files
```

### 🚀 Getting Started

1. **Hardware Setup**: Follow `HARDWARE_SETUP.md`
2. **Software Setup**: Run `./setup.sh`
3. **Configuration**: Edit `config.h` for your network
4. **Build & Upload**: `pio run --target upload`
5. **Monitor**: `pio device monitor`

### 🧪 Testing

- **Unit Tests**: `./run_tests.sh`
- **Integration Tests**: `pio test`
- **Hardware Tests**: `./test_aht10.sh`

### 📊 Performance

- **Sensor Reading**: ~100ms per sensor
- **Health Calculation**: Real-time scoring
- **Memory Usage**: ~50KB RAM
- **Flash Usage**: ~800KB (with all features)

### 🔒 Security

- **WiFi Credentials**: Configurable via `config.h`
- **Data Transmission**: HTTP POST to configurable server
- **Error Handling**: Comprehensive error checking and reporting

### 📈 Future Roadmap

#### v0.2.0 (Planned)
- MQTT support for IoT integration
- Mobile app companion
- Advanced plant health algorithms
- Sensor calibration tools

#### v0.3.0 (Planned)
- Machine learning for plant health prediction
- Cloud dashboard integration
- Multi-plant support
- Automated watering system integration

### 🐛 Known Issues

1. **Flash Size Warning**: Expected 8MB, found 2MB (non-critical)
2. **ADC Calibration**: Raw values only (calibration planned for v0.2)
3. **WiFi Credentials**: Hardcoded in config.h (environment variables planned)

### 📝 Changelog

#### v0.1.0 (2024-08-03)
- Initial release
- Modular sensor architecture
- Plant health algorithm
- Web dashboard
- Terminal safety features
- Comprehensive documentation
- Test suite implementation

### 🤝 Contributing

This project welcomes contributions! Please see `CONTRIBUTING.md` for guidelines.

### 📄 License

This project is licensed under the MIT License - see `LICENSE` file for details.

---

**Release Date**: August 3, 2024  
**Maintainer**: Plant Monitor System Team  
**Repository**: [GitHub Repository URL]  
**Documentation**: See `README.md` and project documentation 