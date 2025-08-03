# 🌱 Plant Monitor - Modular & Extensible

A clean, industry-standard plant monitoring system for ESP32-C6 with modular sensor and display interfaces for maximum flexibility and future expansion.

## 🎯 **Modular Architecture**

### **Sensor Interface**
- **`sensors/sensor_interface.h`** - Unified sensor interface
- **`sensors/aht10.h`** - AHT10 temperature/humidity sensor
- **Future sensors** - DHT11/22, BME280, SHT30, etc.

### **Display Interface**
- **`display/display_interface.h`** - Unified display interface
- **Future displays** - OLED, LCD, TFT, e-ink, etc.

### **Key Benefits**
- ✅ **Modular Design** - Easy to add new sensors and displays
- ✅ **Industry Standard** - Professional documentation
- ✅ **Robust & Stable** - Clean error handling
- ✅ **Future-Proof** - Extensible architecture
- ✅ **Clean Structure** - Organized by functionality

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
I (1234) PLANT_MONITOR_MODULAR: Plant Monitor System Starting...
I (1234) SENSOR_INTERFACE: Initializing Sensor Interface
I (1234) AHT10: Initializing AHT10 sensor at 0x38
I (1234) AHT10: AHT10 sensor initialized successfully
I (1234) DISPLAY_INTERFACE: Initializing Display Interface
I (1234) DISPLAY_INTERFACE: Console display initialized
I (1234) PLANT_MONITOR_MODULAR: Found 2 I2C devices
I (1234) PLANT_MONITOR_MODULAR: System initialized successfully!
I (1234) PLANT_MONITOR_MODULAR: Features:
I (1234) PLANT_MONITOR_MODULAR: - Modular sensor interface (AHT10, DHT, analog)
I (1234) PLANT_MONITOR_MODULAR: - Modular display interface (OLED, LCD, TFT)
I (1234) PLANT_MONITOR_MODULAR: - Professional numpy-style documentation
I (1234) PLANT_MONITOR_MODULAR: - Robust error handling and recovery
I (1234) PLANT_MONITOR_MODULAR: - Plant health analysis with emoji indicators
I (1234) PLANT_MONITOR_MODULAR: - Extensible architecture for future sensors
I (1234) PLANT_MONITOR_MODULAR: - Clean, industry-standard design
```

## 📁 **Modular Project Structure**

```
plant_monitor/
├── src/
│   ├── main_example.cpp      # Modular example application
│   ├── sensors/
│   │   ├── sensor_interface.h # Unified sensor interface
│   │   ├── sensor_interface.c # Sensor interface implementation
│   │   ├── aht10.h          # AHT10 sensor driver
│   │   └── aht10.c          # AHT10 implementation
│   ├── display/
│   │   ├── display_interface.h # Unified display interface
│   │   └── display_interface.c # Display implementation
│   └── CMakeLists.txt       # Build configuration
├── test/
│   ├── unit/                # Unit tests
│   └── integration/         # Integration tests
├── platformio.ini           # PlatformIO config
├── config.h                 # System configuration
├── deploy.sh                # Deployment script
├── run_tests.sh             # Test runner
└── README.md                # This file
```

## 🔧 **Configuration**

### **Sensor Configuration**
```cpp
// AHT10 Sensors
sensor_config_t aht10_1 = {
    .type = SENSOR_TYPE_AHT10,
    .address = 0x38,
    .enabled = true,
    .name = "AHT10-1"
};

sensor_config_t aht10_2 = {
    .type = SENSOR_TYPE_AHT10,
    .address = 0x39,
    .enabled = true,
    .name = "AHT10-2"
};

// Analog Sensors
sensor_config_t soil_moisture = {
    .type = SENSOR_TYPE_SOIL_MOISTURE,
    .pin = 1,
    .enabled = true,
    .name = "Soil Moisture"
};

sensor_config_t light_sensor = {
    .type = SENSOR_TYPE_LIGHT,
    .pin = 2,
    .enabled = true,
    .name = "Light Sensor"
};
```

### **Display Configuration**
```cpp
// Console Display (for debugging)
display_config_t console_display = {
    .type = DISPLAY_TYPE_CONSOLE,
    .enabled = true,
    .name = "Console Display"
};

// Future: OLED Display
display_config_t oled_display = {
    .type = DISPLAY_TYPE_OLED_SSD1306,
    .i2c_address = 0x3C,
    .sda_pin = 21,
    .scl_pin = 22,
    .enabled = true,
    .name = "OLED Display"
};
```

## 📊 **Features**

### **Modular Sensor Support**
- **AHT10** - Temperature and humidity monitoring
- **DHT11/22** - One-wire temperature/humidity (planned)
- **BME280** - Pressure, temperature, humidity (planned)
- **SHT30** - High-precision temperature/humidity (planned)
- **Analog Sensors** - Soil moisture and light level
- **Custom Sensors** - Easy to add new sensor types

### **Modular Display Support**
- **Console** - ASCII art display in console
- **OLED SSD1306** - 128x64 I2C OLED display (planned)
- **LCD 16x2** - Standard LCD display (planned)
- **TFT SPI** - Color TFT displays (planned)
- **E-Ink** - Low-power e-ink displays (planned)

### **Plant Health Analysis**
- 😊 **Excellent** (90-100%) - Perfect conditions
- 🙂 **Good** (70-89%) - Good conditions
- 😐 **Fair** (50-69%) - Acceptable conditions
- 😟 **Poor** (30-49%) - Needs improvement
- 😱 **Critical** (0-29%) - Immediate attention required

## 🛠️ **Hardware Setup**

### **AHT10 Sensors (with proper resistors)**
```
Sensor 1 (0x38):
- VCC → 3.3V
- GND → GND
- SDA → GPIO 21 (with 4.7kΩ pull-up)
- SCL → GPIO 22 (with 4.7kΩ pull-up)

Sensor 2 (0x39):
- VCC → 3.3V
- GND → GND
- SDA → GPIO 21 (with 4.7kΩ pull-up)
- SCL → GPIO 22 (with 4.7kΩ pull-up)
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
│  Sensors: 4/4 working   │
│  Displays: 1/1 working  │
│  Uptime: 01:00:00       │
└─────────────────────────┘
```

### **Data Logging**
```
I (1234) PLANT_MONITOR_MODULAR: Read 4 sensor readings
I (1234) PLANT_MONITOR_MODULAR: Valid sensors: 4/4
I (1234) PLANT_MONITOR_MODULAR: Temperature: 23.45°C
I (1234) PLANT_MONITOR_MODULAR: Humidity: 45.67%
I (1234) PLANT_MONITOR_MODULAR: Soil Moisture: 512
I (1234) PLANT_MONITOR_MODULAR: Light Level: 2048
I (1234) PLANT_MONITOR_MODULAR: Plant Health: Excellent 😊 (Score: 95.0)
I (1234) PLANT_MONITOR_MODULAR: Recommendation: Perfect conditions! Keep it up.
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

## 🎯 **Why This Modular Approach?**

### **Before (Consolidated)**
- Single large library with all functionality
- Difficult to add new sensor types
- Hard to customize display behavior
- Limited extensibility

### **After (Modular)**
- Separate interfaces for sensors and displays
- Easy to add new sensor types
- Simple to implement new displays
- Highly extensible and maintainable

## 🚀 **Future Expansion**

### **Adding New Sensors**
1. Create new sensor header (e.g., `dht11.h`)
2. Implement sensor functions
3. Add to `sensor_interface.c`
4. Configure in main application

### **Adding New Displays**
1. Create new display header (e.g., `oled_ssd1306.h`)
2. Implement display functions
3. Add to `display_interface.c`
4. Configure in main application

## 🚀 **Next Steps**

1. **Deploy the system**: `./deploy.sh`
2. **Monitor output**: `pio device monitor`
3. **Add resistors** for proper I2C communication
4. **Add new sensors** as needed
5. **Add new displays** as needed

---

**Happy Modular Plant Monitoring! 🌱📊😊** 