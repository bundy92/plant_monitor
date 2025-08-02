# AHT10 Sensor Setup Guide

## 🎯 AHT10 Integration Complete!

Your ESP32-C6 plant monitor now supports **AHT10 sensors** for accurate temperature and humidity measurements.

## 📋 AHT10 Advantages over DHT22

- **Higher Accuracy**: ±0.3°C temperature, ±2% humidity
- **I2C Communication**: More reliable than single-wire
- **Faster Response**: Quicker measurements
- **Better Calibration**: Factory calibrated
- **Dual Sensor Support**: Can use two sensors for redundancy

## 🔌 Wiring Diagram

```
AHT10 Sensor Wiring:
┌─────────────┐
│   AHT10     │
│             │
│ VCC ── 3.3V │
│ GND ── GND  │
│ SDA ── GPIO21│
│ SCL ── GPIO22│
└─────────────┘
```

## 🧪 Testing Your AHT10 Sensors

### 1. I2C Scanner Test
To verify your sensors are properly connected:

1. **Temporarily replace** `src/main.cpp` with `i2c_scanner.cpp`
2. **Build and upload**:
   ```bash
   pio run --target upload
   ```
3. **Monitor output**:
   ```bash
   pio device monitor
   ```

**Expected Output:**
```
I (1234) I2C_SCANNER: Found I2C device at address: 0x38
I (1234) I2C_SCANNER:   -> This looks like an AHT10 sensor!
I (1234) I2C_SCANNER: Found I2C device at address: 0x39
I (1234) I2C_SCANNER:   -> This looks like an AHT10 sensor!
```

### 2. Sensor Address Configuration

If your sensors have different addresses, update `config.h`:
```cpp
#define AHT10_SENSOR_1_ADDR 0x38  // Your first sensor address
#define AHT10_SENSOR_2_ADDR 0x39  // Your second sensor address
```

## 🔧 Configuration

### I2C Pins (config.h)
```cpp
#define I2C_MASTER_SDA_IO GPIO_NUM_21  // SDA pin
#define I2C_MASTER_SCL_IO GPIO_NUM_22  // SCL pin
```

### Sensor Addresses
```cpp
#define AHT10_SENSOR_1_ADDR 0x38  // Default AHT10 address
#define AHT10_SENSOR_2_ADDR 0x39  // Alternative address
```

## 📊 Data Output

The system will output readings like:
```
I (1234) PLANT_MONITOR: AHT10 Sensor 1 - Temperature: 23.45°C, Humidity: 65.20%
I (1234) PLANT_MONITOR: AHT10 Sensor 2 - Temperature: 23.52°C, Humidity: 65.18%
I (1234) PLANT_MONITOR: === Sensor Readings ===
I (1234) PLANT_MONITOR: Temperature: 23.49°C
I (1234) PLANT_MONITOR: Humidity: 65.19%
I (1234) PLANT_MONITOR: Soil Moisture: 2048
I (1234) PLANT_MONITOR: Light Level: 1500
I (1234) PLANT_MONITOR: =====================
```

## 🐛 Troubleshooting

### No I2C Devices Found
1. **Check wiring**: SDA→GPIO21, SCL→GPIO22
2. **Verify power**: 3.3V and GND connections
3. **Check pull-up resistors**: Some boards need external resistors
4. **Test with multimeter**: Verify voltage levels

### AHT10 Not Responding
1. **Check sensor address**: Use I2C scanner to find correct address
2. **Verify sensor orientation**: Check pin alignment
3. **Test individual sensors**: Connect one at a time
4. **Check for shorts**: Ensure no pins are touching

### Inaccurate Readings
1. **Allow warm-up time**: Sensors need 2-3 minutes to stabilize
2. **Check sensor placement**: Avoid heat sources
3. **Verify calibration**: AHT10 sensors are factory calibrated

## 🚀 Ready to Deploy

Once your AHT10 sensors are working:

1. **Restore main.cpp** (if you used the scanner)
2. **Update WiFi credentials** in `config.h`
3. **Deploy**: `./deploy.sh`
4. **Monitor**: `pio device monitor`

## 📈 Benefits

- **Dual sensor averaging** for more accurate readings
- **I2C reliability** over single-wire communication
- **Better accuracy** than DHT22
- **Faster response times**
- **Factory calibration** included

Your plant monitoring system is now ready with professional-grade AHT10 sensors! 🌱 