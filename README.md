# 🌱 Plant Monitor - Complete IoT System

A comprehensive, industry-standard plant monitoring system featuring ESP32-C6 sensors and Raspberry Pi Zero V2 server with modular architecture, professional testing, and enterprise-grade documentation.

## 🎯 **System Architecture**

### **ESP32-C6 Plant Monitor**
- **Modular Sensor Interface** - Support for AHT10, DS18B20, GY-302, and analog sensors
- **Modular Display Interface** - Support for OLED, E-paper, and console displays
- **Professional Documentation** - Numpy-style documentation throughout
- **Comprehensive Testing** - Unit, integration, and system verification tests
- **Industry Standards** - Clean, extensible architecture with error handling

### **Raspberry Pi Zero V2 Server**
- **RESTful API** - Comprehensive data reception and querying endpoints
- **Real-time WebSocket** - Live updates and monitoring
- **SQLite Database** - Robust data storage with SQLAlchemy ORM
- **Security Features** - Rate limiting, CORS, input validation
- **Monitoring & Metrics** - Prometheus metrics and health checks

## 🚀 **Quick Start**

### **ESP32 Development**
```bash
# Install PlatformIO
pip3 install platformio

# Build and upload
pio run --target upload

# Run tests
./run_tests.sh
```

### **Raspberry Pi Server**
```bash
cd raspberry_pi
pip3 install -r requirements.txt
python3 server.py

# Run tests
./run_tests.sh
```

## 📁 **Project Structure**

```
plant_monitor/
├── src/                          # ESP32 Source Code
│   ├── sensors/                  # Modular Sensor Interface
│   │   ├── sensor_interface.h/c  # Unified sensor interface
│   │   ├── aht10.h/c            # AHT10 temperature/humidity
│   │   ├── ds18b20.h/c          # DS18B20 waterproof temp
│   │   └── gy302.h/c            # GY-302 light intensity
│   ├── display/                  # Modular Display Interface
│   │   ├── display_interface.h/c # Unified display interface
│   │   └── (future displays)    # OLED, E-paper, etc.
│   └── main_example.cpp         # Example application
├── test/                         # ESP32 Testing
│   ├── unit/                     # Unit tests
│   └── integration/              # Integration tests
├── raspberry_pi/                 # Raspberry Pi Server
│   ├── server.py                 # Main server implementation
│   ├── test_server.py            # Comprehensive test suite
│   ├── run_tests.sh             # Test runner
│   ├── requirements.txt          # Python dependencies
│   └── README.md                # Server documentation
├── platformio.ini               # PlatformIO configuration
├── config.h                     # ESP32 configuration
├── run_tests.sh                 # ESP32 test runner
└── README.md                    # This file
```

## 🔧 **Hardware Support**

### **ESP32 Sensors**
- ✅ **AHT10** - Temperature and humidity sensor (I2C)
- ✅ **DS18B20** - Waterproof temperature sensor (One-Wire)
- ✅ **GY-302** - Digital light intensity sensor (I2C)
- ✅ **Soil Moisture** - Analog soil moisture sensor (ADC)
- ✅ **Light Sensor** - Analog light sensor (ADC)

### **ESP32 Displays**
- ✅ **Built-in SSD1306** - ESP32 DevKit V1 OLED display (I2C)
- ✅ **E-paper Display** - Low-power display (SPI)
- ✅ **Console Display** - Serial output for debugging

### **Raspberry Pi Features**
- ✅ **RESTful API** - Data reception and querying
- ✅ **WebSocket Support** - Real-time updates
- ✅ **Database Storage** - SQLite with SQLAlchemy ORM
- ✅ **Alert System** - Intelligent alert generation
- ✅ **Monitoring** - Prometheus metrics and health checks

## 🧪 **Testing Framework**

### **ESP32 Testing**
```bash
# Run all tests
./run_tests.sh

# Specific test categories
./run_tests.sh unit          # Unit tests
./run_tests.sh integration   # Integration tests
./run_tests.sh system        # System verification
./run_tests.sh quality       # Code quality checks
```

### **Raspberry Pi Testing**
```bash
cd raspberry_pi
# Run all tests
./run_tests.sh

# Specific test categories
./run_tests.sh unit          # Unit tests
./run_tests.sh performance   # Performance tests
./run_tests.sh security      # Security tests
./run_tests.sh coverage      # Coverage analysis
```

## 📊 **API Endpoints**

### **Raspberry Pi Server API**
- `GET /api/health` - System health check
- `POST /api/data` - Receive sensor data
- `GET /api/readings` - Get sensor readings
- `GET /api/devices` - Get active devices
- `GET /api/statistics/<device_id>` - Device statistics
- `GET /api/alerts` - Get active alerts
- `GET /metrics` - Prometheus metrics

### **Example API Usage**
```bash
# Send sensor data
curl -X POST http://localhost:5000/api/data \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": "esp32_001",
    "temperature": 25.5,
    "humidity": 60.0,
    "soil_moisture": 2048,
    "light_level": 1024,
    "lux": 5000.0,
    "health_score": 85.0,
    "health_status": "Good"
  }'

# Get latest readings
curl http://localhost:5000/api/readings?limit=10
```

## 🔧 **Configuration**

### **ESP32 Configuration**
Edit `config.h` for WiFi, server, and pin configurations:
```c
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASS "your_wifi_password"
#define SERVER_URL "http://your_raspberry_pi_ip:5000"
```

### **Raspberry Pi Configuration**
Create `raspberry_pi/config/server_config.yaml`:
```yaml
server:
  host: '0.0.0.0'
  port: 5000
  debug: false

database:
  url: 'sqlite:///plant_monitor.db'

alerts:
  temperature_min: 10.0
  temperature_max: 35.0
  humidity_min: 30.0
  humidity_max: 80.0
```

## 🚀 **Deployment**

### **ESP32 Deployment**
```bash
# Build and upload
pio run --target upload

# Monitor output
pio device monitor
```

### **Raspberry Pi Deployment**
```bash
cd raspberry_pi

# Install dependencies
pip3 install -r requirements.txt

# Run as service
sudo systemctl enable plant-monitor
sudo systemctl start plant-monitor
```

## 📈 **Monitoring & Analytics**

### **ESP32 Monitoring**
- Real-time sensor readings via serial monitor
- Plant health calculation with emoji indicators
- WiFi connection status and data transmission
- System uptime and performance metrics

### **Raspberry Pi Monitoring**
- Prometheus metrics at `/metrics`
- Health check endpoint at `/api/health`
- Real-time WebSocket updates
- Database statistics and analytics

## 🔒 **Security Features**

### **ESP32 Security**
- Secure WiFi configuration
- Input validation for sensor data
- Error handling and recovery
- Professional logging standards

### **Raspberry Pi Security**
- Rate limiting for API protection
- Input validation with Marshmallow
- CORS configuration
- SQL injection prevention
- Security headers

## 🧪 **Quality Assurance**

### **Code Quality Standards**
- ✅ **Documentation** - Comprehensive docstrings and comments
- ✅ **Type Safety** - Full type annotation support
- ✅ **Error Handling** - Robust error handling and logging
- ✅ **Testing** - 80%+ code coverage requirement
- ✅ **Security** - Input validation and sanitization
- ✅ **Performance** - Optimized database queries and caching

### **Testing Coverage**
- **ESP32**: Unit tests, integration tests, system verification
- **Raspberry Pi**: Unit tests, performance tests, security tests
- **Code Quality**: Linting, formatting, documentation checks
- **Coverage**: 80%+ code coverage requirement

## 🤝 **Contributing**

### **Development Workflow**
1. Fork the repository
2. Create a feature branch
3. Make changes with tests
4. Run comprehensive tests
5. Submit pull request

### **Code Standards**
- Follow PEP 8 (Python) and industry standards (C/C++)
- Add comprehensive documentation
- Include type hints where applicable
- Write tests for new features
- Update documentation

## 📄 **License**

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 **Acknowledgments**

- **Espressif** - ESP32 hardware and ESP-IDF framework
- **Raspberry Pi Foundation** - Raspberry Pi Zero V2 platform
- **Flask** - Web framework for Raspberry Pi server
- **SQLAlchemy** - Database ORM
- **PlatformIO** - ESP32 development platform

---

**🌱 Plant Monitor v2.0.0** - Professional, scalable, and secure IoT plant monitoring solution with modular architecture and comprehensive testing. 