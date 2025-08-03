# 🔧 Hardware Setup Guide

This guide provides detailed wiring instructions for all supported sensors and displays in the modular plant monitoring system.

## 📋 **Hardware Components**

### **ESP32 Boards**
- **ESP32-C6 DevKit** - Current development board
- **ESP32 DevKit V1 WROOM-32** - With built-in 128×64 SSD1306 display

### **Sensors**
- **AHT10** - Temperature and humidity sensor (I2C)
- **DS18B20** - Waterproof temperature sensor (One-Wire)
- **GY-302 (BH1750FVI)** - Digital light intensity sensor (I2C)
- **Soil Moisture Sensor** - Analog soil moisture sensor
- **Light Sensor** - Analog light sensor

### **Displays**
- **Built-in SSD1306** - 128×64 OLED display (ESP32 DevKit)
- **E-paper Module** - 2.9" E-paper display (SPI)
- **Console Output** - Serial monitor display

## 🔌 **Wiring Diagrams**

### **ESP32-C6 DevKit Setup**

#### **I2C Sensors (AHT10, GY-302)**
```
AHT10 Sensor 1 (0x38):
- VCC → 3.3V
- GND → GND
- SDA → GPIO 21 (with 4.7kΩ pull-up)
- SCL → GPIO 22 (with 4.7kΩ pull-up)

AHT10 Sensor 2 (0x39):
- VCC → 3.3V
- GND → GND
- SDA → GPIO 21 (with 4.7kΩ pull-up)
- SCL → GPIO 22 (with 4.7kΩ pull-up)

GY-302 Light Sensor (0x23):
- VCC → 3.3V
- GND → GND
- SDA → GPIO 21 (with 4.7kΩ pull-up)
- SCL → GPIO 22 (with 4.7kΩ pull-up)
```

#### **One-Wire Sensor (DS18B20)**
```
DS18B20 Waterproof Temperature:
- VCC → 3.3V
- GND → GND
- DATA → GPIO 4 (with 4.7kΩ pull-up)
```

#### **Analog Sensors**
```
Soil Moisture Sensor:
- VCC → 3.3V
- GND → GND
- SIG → GPIO 1 (ADC1_CH0)

Light Sensor:
- VCC → 3.3V
- GND → GND
- SIG → GPIO 2 (ADC1_CH1)
```

### **ESP32 DevKit V1 WROOM-32 Setup**

#### **Built-in Display**
```
Built-in SSD1306:
- Already connected internally
- I2C Address: 0x3C
- SDA: Internal connection
- SCL: Internal connection
```

#### **E-paper Display (SPI)**
```
2.9" E-paper Module:
- VCC → 3.3V
- GND → GND
- CS → GPIO 5
- DC → GPIO 17
- RST → GPIO 16
- MOSI → GPIO 23
- SCK → GPIO 18
- BUSY → GPIO 4
```

#### **Sensors (Same as ESP32-C6)**
```
I2C Sensors:
- SDA → GPIO 21
- SCL → GPIO 22
- Pull-up resistors: 4.7kΩ each

One-Wire Sensor:
- DATA → GPIO 4
- Pull-up resistor: 4.7kΩ

Analog Sensors:
- Soil Moisture → GPIO 1
- Light Sensor → GPIO 2
```

## 📊 **Pin Assignment Summary**

| Component | ESP32-C6 | ESP32 DevKit | Notes |
|-----------|----------|--------------|-------|
| I2C SDA | GPIO 21 | GPIO 21 | Shared by all I2C devices |
| I2C SCL | GPIO 22 | GPIO 22 | Shared by all I2C devices |
| One-Wire | GPIO 4 | GPIO 4 | DS18B20 data line |
| ADC Soil | GPIO 1 | GPIO 1 | Soil moisture sensor |
| ADC Light | GPIO 2 | GPIO 2 | Analog light sensor |
| E-paper CS | N/A | GPIO 5 | E-paper chip select |
| E-paper DC | N/A | GPIO 17 | E-paper data/command |
| E-paper RST | N/A | GPIO 16 | E-paper reset |
| E-paper MOSI | N/A | GPIO 23 | E-paper data in |
| E-paper SCK | N/A | GPIO 18 | E-paper clock |
| E-paper BUSY | N/A | GPIO 4 | E-paper busy signal |

## 🔧 **Power Requirements**

### **Voltage Levels**
- **3.3V** - All sensors and displays
- **5V** - Not required (all components are 3.3V compatible)

### **Current Requirements**
- **AHT10**: ~2.4mA
- **DS18B20**: ~1.5mA
- **GY-302**: ~120μA
- **Soil Moisture**: ~35mA
- **Light Sensor**: ~20μA
- **Built-in OLED**: ~40mA
- **E-paper**: ~25mA (during refresh)

### **Total Current**
- **Typical**: ~100mA
- **Peak**: ~150mA (during E-paper refresh)

## 🛠️ **Assembly Instructions**

### **Step 1: Prepare Components**
1. Gather all sensors and displays
2. Check for any damage or missing parts
3. Verify voltage compatibility (all should be 3.3V)

### **Step 2: Install Pull-up Resistors**
1. Add 4.7kΩ resistors to I2C lines (SDA, SCL)
2. Add 4.7kΩ resistor to One-Wire line (DATA)
3. Use breadboard or PCB for clean connections

### **Step 3: Connect Power**
1. Connect all VCC pins to 3.3V
2. Connect all GND pins to ground
3. Verify stable power supply

### **Step 4: Connect Data Lines**
1. Connect I2C sensors (AHT10, GY-302)
2. Connect One-Wire sensor (DS18B20)
3. Connect analog sensors (soil moisture, light)
4. Connect E-paper display (if using ESP32 DevKit)

### **Step 5: Test Connections**
1. Upload test code
2. Check serial monitor for device detection
3. Verify sensor readings
4. Test display output

## 🔍 **Troubleshooting**

### **I2C Issues**
- **No devices found**: Check pull-up resistors
- **Communication errors**: Verify wiring and addresses
- **Address conflicts**: Ensure unique addresses

### **One-Wire Issues**
- **No DS18B20 detected**: Check pull-up resistor
- **Communication errors**: Verify single data line
- **Multiple devices**: Use ROM code matching

### **Analog Sensor Issues**
- **Inconsistent readings**: Check wiring and power
- **No signal**: Verify ADC pin connections
- **Noise**: Add filtering capacitors

### **Display Issues**
- **No display**: Check I2C/SPI connections
- **Garbled output**: Verify pin assignments
- **E-paper not updating**: Check BUSY pin

## 📈 **Expected Readings**

### **Temperature Sensors**
- **AHT10**: -40°C to +85°C, ±0.3°C accuracy
- **DS18B20**: -55°C to +125°C, ±0.5°C accuracy

### **Humidity Sensor**
- **AHT10**: 0-100% RH, ±2% accuracy

### **Light Sensors**
- **GY-302**: 1-65535 lux, ±20% accuracy
- **Analog Light**: 0-4095 ADC value

### **Soil Moisture**
- **Analog Sensor**: 0-4095 ADC value
- **Dry soil**: ~3000-4095
- **Wet soil**: ~0-1500

## 🚀 **Next Steps**

1. **Assemble hardware** according to wiring diagrams
2. **Upload test code** to verify connections
3. **Calibrate sensors** for your specific environment
4. **Deploy the system** and monitor plant health
5. **Add more sensors** as needed using the modular interface

---

**Happy Hardware Assembly! 🔧📊🌱** 