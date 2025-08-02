# Simple AHT10 Test Guide

## 🎯 Test with Just ESP32 + AHT10 Sensors

You can test your AHT10 sensors using only the ESP32's built-in 3.3V power supply!

## 🔌 Simple Wiring (No External Power Needed)

### ESP32-C6 Pin Connections:
```
ESP32-C6 DevKit:
├── 3.3V  → AHT10 VCC (Power)
├── GND   → AHT10 GND (Ground)
├── GPIO21 → AHT10 SDA (Data)
└── GPIO22 → AHT10 SCL (Clock)
```

### AHT10 Sensor Pinout:
```
AHT10 Sensor:
┌─────────────┐
│   AHT10     │
│             │
│ VCC ── 3.3V │
│ GND ── GND  │
│ SDA ── GPIO21│
│ SCL ── GPIO22│
└─────────────┘
```

## 🧪 Testing Steps

### 1. Wire Your Sensors
Connect your AHT10 sensors to the ESP32:
- **VCC** → **3.3V** (ESP32's built-in 3.3V)
- **GND** → **GND** (ESP32's ground)
- **SDA** → **GPIO 21**
- **SCL** → **GPIO 22**

### 2. Upload Test Code
Replace the main code with the simple test:

```bash
# Backup current main.cpp
cp src/main.cpp src/main_backup.cpp

# Replace with test code
cp simple_test.cpp src/main.cpp

# Build and upload
pio run --target upload
```

### 3. Monitor Output
```bash
pio device monitor
```

## 📊 Expected Output

### Successful Connection:
```
I (1234) AHT10_TEST: AHT10 Simple Test for ESP32-C6
I (1234) AHT10_TEST: =================================
I (1234) AHT10_TEST: Using ESP32's built-in 3.3V power
I (1234) AHT10_TEST: SDA Pin: GPIO 21
I (1234) AHT10_TEST: SCL Pin: GPIO 22
I (1234) AHT10_TEST: =================================
I (1234) AHT10_TEST: I2C initialized successfully
I (1234) AHT10_TEST: Starting AHT10 sensor test...
I (1234) AHT10_TEST: Scanning I2C bus for devices...
I (1234) AHT10_TEST: Found I2C device at address: 0x38
I (1234) AHT10_TEST:   -> This looks like an AHT10 sensor!
I (1234) AHT10_TEST: Found I2C device at address: 0x39
I (1234) AHT10_TEST:   -> This looks like an AHT10 sensor!
I (1234) AHT10_TEST: I2C scan complete! Found 2 devices
I (1234) AHT10_TEST: Expected AHT10 addresses: 0x38, 0x39
I (1234) AHT10_TEST: Initializing AHT10 sensor at address 0x38
I (1234) AHT10_TEST: AHT10 sensor at address 0x38 initialized successfully
I (1234) AHT10_TEST: Initializing AHT10 sensor at address 0x39
I (1234) AHT10_TEST: AHT10 sensor at address 0x39 initialized successfully
I (1234) AHT10_TEST: AHT10 sensors initialized successfully!
I (1234) AHT10_TEST: Starting continuous readings...
I (1234) AHT10_TEST: =======================================
I (1234) AHT10_TEST: Reading #1:
I (1234) AHT10_TEST:   Sensor 1 (0x38): Temperature: 23.45°C, Humidity: 65.20%
I (1234) AHT10_TEST:   Sensor 2 (0x39): Temperature: 23.52°C, Humidity: 65.18%
I (1234) AHT10_TEST:   Average: Temperature: 23.49°C, Humidity: 65.19%
I (1234) AHT10_TEST: =======================================
```

### No Sensors Found:
```
I (1234) AHT10_TEST: I2C scan complete! Found 0 devices
I (1234) AHT10_TEST: No AHT10 sensors found or initialized!
I (1234) AHT10_TEST: Please check:
I (1234) AHT10_TEST: 1. Sensor wiring (SDA→GPIO21, SCL→GPIO22)
I (1234) AHT10_TEST: 2. Power connections (VCC→3.3V, GND→GND)
I (1234) AHT10_TEST: 3. Sensor orientation
```

## 🔧 Troubleshooting

### No I2C Devices Found
1. **Check wiring**: Ensure SDA→GPIO21, SCL→GPIO22
2. **Verify power**: VCC→3.3V, GND→GND
3. **Check sensor orientation**: Make sure pins align correctly
4. **Test with multimeter**: Verify 3.3V at sensor VCC pin

### AHT10 Not Responding
1. **Check sensor address**: Use I2C scanner to find correct address
2. **Verify sensor orientation**: Check pin alignment
3. **Test individual sensors**: Connect one at a time
4. **Check for shorts**: Ensure no pins are touching

### Inaccurate Readings
1. **Allow warm-up time**: Sensors need 2-3 minutes to stabilize
2. **Check sensor placement**: Avoid heat sources
3. **Verify calibration**: AHT10 sensors are factory calibrated

## 🚀 Power Considerations

### ESP32-C6 Built-in 3.3V:
- **Current**: Up to 500mA (sufficient for AHT10 sensors)
- **Voltage**: Stable 3.3V output
- **Advantages**: No external power supply needed

### AHT10 Power Requirements:
- **Voltage**: 2.2V to 5.5V (3.3V is perfect)
- **Current**: ~1.5mA during measurement
- **Power**: ~5mW (very low power)

## 📈 Test Results

Once your sensors are working, you should see:
- **Temperature readings**: 20-30°C (room temperature)
- **Humidity readings**: 40-70% (typical indoor humidity)
- **Stable readings**: Consistent values over time
- **Dual sensor averaging**: More accurate combined readings

## 🔄 Return to Full Project

After successful testing:

```bash
# Restore the full plant monitor code
cp src/main_backup.cpp src/main.cpp

# Build and upload the full project
pio run --target upload
```

## 🎉 Success!

If you see temperature and humidity readings, your AHT10 sensors are working perfectly with the ESP32's built-in power supply!

Your sensors are ready for the full plant monitoring system when you get your external power supply next week. 