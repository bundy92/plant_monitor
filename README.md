# 🌱 Plant Monitor - Clean & Simple

A clean, industry-standard plant monitoring system for ESP32-C6 with minimal complexity and maximum reliability.

## 🎯 **Clean Architecture**

### **Single Library Approach**
- **`plant_monitor.h`** - Clean, unified interface
- **`plant_monitor.c`** - Consolidated implementation
- **`main_example.cpp`** - Simple application example

### **Key Benefits**
- ✅ **Minimal Complexity** - Only 3 core files
- ✅ **Industry Standard** - Professional documentation
- ✅ **Robust & Stable** - Clean error handling
- ✅ **Easy to Maintain** - Simple structure
- ✅ **Extensible** - Easy to add features

## 🚀 **Quick Start**

### **1. Deploy the System**
```bash
# Use the deployment script
./deploy.sh

# Or specify port manually
./deploy.sh /dev/ttyACM0
```

### **2. Monitor Output**
```bash
pio device monitor
```

### **3. Expected Output**
```
I (1234) PLANT_MONITOR_MAIN: Plant Monitor System Starting...
I (1234) PLANT_MONITOR: Initializing Plant Monitor System
I (1234) PLANT_MONITOR: I2C initialized successfully
I (1234) PLANT_MONITOR: ADC initialized successfully
I (1234) PLANT_MONITOR: AHT10 sensor at 0x38 initialized successfully
I (1234) PLANT_MONITOR: Plant Monitor System initialized successfully
I (1234) PLANT_MONITOR: Features:
I (1234) PLANT_MONITOR: - AHT10 temperature/humidity sensors
I (1234) PLANT_MONITOR: - Analog soil moisture and light sensors
I (1234) PLANT_MONITOR: - Plant health analysis with emoji indicators
I (1234) PLANT_MONITOR: - WiFi connectivity and data transmission
I (1234) PLANT_MONITOR: - Professional numpy-style documentation
I (1234) PLANT_MONITOR: - Clean, industry-standard architecture
```

## 📁 **Clean Project Structure**

```
plant_monitor/
├── src/
│   ├── main.cpp              # Main application
│   ├── main_example.cpp      # Example application
│   ├── plant_monitor.h       # Clean interface
│   ├── plant_monitor.c       # Consolidated implementation
│   └── CMakeLists.txt        # Build configuration
├── test/
│   ├── unit/                 # Unit tests
│   └── integration/          # Integration tests
├── platformio.ini            # PlatformIO config
├── config.h                  # System configuration
├── deploy.sh                 # Deployment script
├── run_tests.sh              # Test runner
└── README.md                 # This file
```

## 🔧 **Configuration**

### **Default Settings**
```cpp
// I2C Configuration
#define PLANT_MONITOR_DEFAULT_SDA_PIN        21
#define PLANT_MONITOR_DEFAULT_SCL_PIN        22
#define PLANT_MONITOR_DEFAULT_I2C_FREQ_HZ    100000

// AHT10 Sensor Addresses
#define PLANT_MONITOR_AHT10_ADDR_1           0x38
#define PLANT_MONITOR_AHT10_ADDR_2           0x39

// Plant Health Ranges
temp_min: 10.0°C, temp_max: 35.0°C
temp_optimal_min: 18.0°C, temp_optimal_max: 28.0°C
humidity_min: 30.0%, humidity_max: 80.0%
humidity_optimal_min: 40.0%, humidity_optimal_max: 70.0%
```

## 📊 **Features**

### **Core Functionality**
- **AHT10 Sensors** - Temperature and humidity monitoring
- **Analog Sensors** - Soil moisture and light level
- **Plant Health Analysis** - Emoji-based health indicators
- **Display Output** - ASCII art display in console
- **WiFi Connectivity** - Data transmission capability
- **Professional Documentation** - Numpy-style docstrings

### **Health Indicators**
- 😊 **Excellent** (90-100%) - Perfect conditions
- 🙂 **Good** (70-89%) - Good conditions
- 😐 **Fair** (50-69%) - Acceptable conditions
- 😟 **Poor** (30-49%) - Needs improvement
- 😱 **Critical** (0-29%) - Immediate attention required

## 🛠️ **Hardware Setup**

### **AHT10 Sensors**
```
Sensor 1 (0x38):
- VCC → 3.3V
- GND → GND
- SDA → GPIO 21
- SCL → GPIO 22

Sensor 2 (0x39):
- VCC → 3.3V
- GND → GND
- SDA → GPIO 21
- SCL → GPIO 22
```

### **Analog Sensors**
```
Soil Moisture: GPIO 1 (ADC1_CH0)
Light Sensor: GPIO 2 (ADC1_CH1)
```

## 📈 **Sample Output**

### **Console Display**
```
┌─────────────────────────┐
│     Plant Monitor       │
│      😊 Excellent       │
│                         │
│  T: 23.5°C  H: 45.7%   │
│  Soil: 512  Light: 2048 │
│  WiFi: ✗  Data: ✗       │
│  Uptime: 01:00:00       │
└─────────────────────────┘
```

### **Data Logging**
```
I (1234) PLANT_MONITOR: Sensor readings: T1=23.45°C, H1=45.67%, T2=23.52°C, H2=45.89%, Avg T=23.49°C, Avg H=45.78%, Soil=512, Light=2048
I (1234) PLANT_MONITOR: Plant health: Excellent 😊 (Score: 95.0) - Perfect conditions! Keep it up.
```

## 🧪 **Testing**

### **Run All Tests**
```bash
./run_tests.sh
```

### **Run Specific Tests**
```bash
./run_tests.sh unit          # Unit tests
./run_tests.sh integration   # Integration tests
./run_tests.sh system        # System tests
./run_tests.sh quality       # Code quality checks
./run_tests.sh performance   # Performance tests
./run_tests.sh memory        # Memory usage tests
```

## 🔍 **Troubleshooting**

### **Common Issues**

1. **AHT10 Not Found**
   ```bash
   # Check I2C devices
   pio device monitor
   # Look for "Found I2C device at address: 0x38"
   ```

2. **Build Errors**
   ```bash
   # Clean and rebuild
   pio run --target clean
   pio run
   ```

3. **Upload Issues**
   ```bash
   # Specify port manually
   ./deploy.sh /dev/ttyACM0
   ```

## 🎯 **Why This Approach?**

### **Before (Complex)**
- 15+ files with complex interdependencies
- Multiple interfaces and abstractions
- Difficult to maintain and debug
- High risk of breaking changes

### **After (Clean)**
- 3 core files with clear responsibilities
- Single, unified interface
- Easy to understand and maintain
- Stable and reliable

## 🚀 **Next Steps**

1. **Deploy the system**: `./deploy.sh`
2. **Monitor output**: `pio device monitor`
3. **Customize configuration** in `config.h`
4. **Add features** as needed

---

**Happy Clean Plant Monitoring! 🌱📊😊** 