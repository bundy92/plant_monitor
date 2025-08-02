# Quick Start Guide

## 🚀 Get Started in 5 Minutes

### 1. Update Configuration
Edit `config.h` with your WiFi credentials:
```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"
```

**Optional**: Update server URL if different:
```cpp
#define SERVER_URL "http://YOUR_RASPBERRY_PI_IP:8080/data"
```

### 2. Connect Sensors
- **AHT10 Sensors**: 
  - **VCC**: 3.3V
  - **GND**: Ground
  - **SDA**: GPIO 21
  - **SCL**: GPIO 22
- **Soil Moisture**: GPIO 5 (Analog), 3.3V, GND  
- **Light Sensor**: GPIO 6 (Analog), 3.3V, GND
- **LED**: GPIO 2 (Status indicator)

### 3. Test Build (Optional)
```bash
./test_build.sh
```

### 4. Deploy
```bash
./deploy.sh
```

### 5. Monitor
```bash
pio device monitor
```

## 📁 Project Structure

```
plant_monitor/
├── src/
│   ├── main.cpp              # Main ESP-IDF application
│   └── CMakeLists.txt        # Component build configuration
├── config.h                  # Configuration settings
├── deploy.sh                 # Deployment script
├── test_build.sh            # Test build script
├── raspberry_pi_server.py    # Raspberry Pi server
├── web_dashboard.html        # Web dashboard
├── requirements.txt          # Python dependencies
├── platformio.ini           # PlatformIO configuration
├── CMakeLists.txt           # Project build configuration
├── partitions.csv           # Flash partition table
├── sdkconfig.defaults       # ESP-IDF configuration
├── .gitignore              # Git ignore rules
├── setup_guide.md          # Detailed setup instructions
├── README.md               # Main documentation
└── QUICK_START.md          # This file
```

## 🔧 Configuration

All settings are centralized in `config.h`:
- WiFi credentials
- Server URL
- Pin assignments
- Data transmission interval
- Device ID

## 📊 Status Indicators

- **3 blinks**: Startup successful
- **1 blink**: Data sent successfully
- **2 blinks**: Data transmission failed

## 🐛 Troubleshooting

1. **WiFi issues**: Check credentials in `config.h`
2. **Build errors**: Run `pio run --target clean`
3. **Upload fails**: Check USB connection
4. **No sensor data**: Verify wiring connections

## 📈 Next Steps

1. Connect your sensors
2. Update WiFi credentials
3. Deploy with `./deploy.sh`
4. Monitor output
5. Set up Raspberry Pi server when available 