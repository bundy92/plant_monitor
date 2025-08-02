# ESP32-C6 Plant Monitor Setup Guide

This guide will walk you through setting up the complete plant monitoring system with ESP32-C6 and Raspberry Pi Zero V2.

## Prerequisites

### Hardware Requirements
- ESP32-C6 DevKit
- AHT10 temperature and humidity sensors (I2C)
- Capacitive soil moisture sensor
- Photoresistor or light sensor
- Jumper wires
- Breadboard (optional)
- Raspberry Pi Zero V2 (when available)

### Software Requirements
- PlatformIO IDE or Arduino IDE
- Python 3.7+ (for Raspberry Pi server)
- Git

## Step 1: ESP32-C6 Setup

### 1.1 Hardware Connections

Connect your sensors to the ESP32-C6 as follows:

```
ESP32-C6 Pin Connections:
├── GPIO 21 (SDA) → AHT10 sensors (I2C Data)
├── GPIO 22 (SCL) → AHT10 sensors (I2C Clock)
├── GPIO 5  → Soil Moisture Sensor (Analog)
├── GPIO 6  → Light Sensor (Analog)
├── GPIO 2  → Built-in LED (Status)
├── 3.3V   → VCC for all sensors
└── GND    → Ground for all sensors
```

**Detailed Wiring:**
- **AHT10 Sensors**: 
  - VCC → 3.3V
  - GND → GND
  - SDA → GPIO 21
  - SCL → GPIO 22
- **Soil Moisture Sensor**:
  - VCC → 3.3V
  - Signal → GPIO 5 (Analog)
  - GND → GND
- **Light Sensor**:
  - VCC → 3.3V
  - Signal → GPIO 6 (Analog)
  - GND → GND

### 1.2 Software Setup

1. **Install PlatformIO IDE** (recommended) or Arduino IDE
2. **Clone this repository** to your development machine
3. **Open the project** in PlatformIO IDE
4. **Connect ESP32-C6** to your computer via USB
5. **Build and upload** the code:
   ```bash
   pio run --target upload
   ```

### 1.3 WiFi Configuration

On first boot, the ESP32 will create a WiFi access point:
- **SSID**: `PlantMonitor_AP`
- **Password**: `plant123`

1. **Connect to the WiFi network** `PlantMonitor_AP`
2. **Navigate to** `192.168.1.1` in your browser
3. **Enter your WiFi credentials**
4. **Save the configuration**

The ESP32 will now connect to your WiFi network and start sending data.

## Step 2: Raspberry Pi Zero V2 Setup (When Available)

### 2.1 Initial Setup

1. **Flash Raspberry Pi OS** to your SD card
2. **Enable SSH** and **WiFi** in the boot partition
3. **Boot the Raspberry Pi**
4. **Connect via SSH** or use the desktop interface

### 2.2 Install Dependencies

```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install Python and pip
sudo apt install python3 python3-pip python3-venv -y

# Install additional dependencies
sudo apt install sqlite3 -y
```

### 2.3 Setup Python Environment

```bash
# Create project directory
mkdir ~/plant_monitor
cd ~/plant_monitor

# Create virtual environment
python3 -m venv venv
source venv/bin/activate

# Install Python dependencies
pip install -r requirements.txt
```

### 2.4 Configure the Server

1. **Copy the server files** to your Raspberry Pi:
   ```bash
   # Copy these files to ~/plant_monitor/
   raspberry_pi_server.py
   requirements.txt
   web_dashboard.html
   ```

2. **Update the IP address** in `src/main.cpp` on the ESP32:
   ```cpp
   const char* server_url = "http://YOUR_RASPBERRY_PI_IP:8080/data";
   ```

3. **Start the server**:
   ```bash
   python3 raspberry_pi_server.py
   ```

### 2.5 Setup as Service (Optional)

Create a systemd service for automatic startup:

```bash
sudo nano /etc/systemd/system/plant-monitor.service
```

Add the following content:
```ini
[Unit]
Description=Plant Monitor Server
After=network.target

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/plant_monitor
Environment=PATH=/home/pi/plant_monitor/venv/bin
ExecStart=/home/pi/plant_monitor/venv/bin/python raspberry_pi_server.py
Restart=always

[Install]
WantedBy=multi-user.target
```

Enable and start the service:
```bash
sudo systemctl enable plant-monitor.service
sudo systemctl start plant-monitor.service
```

## Step 3: Testing and Verification

### 3.1 ESP32 Testing

1. **Open Serial Monitor** at 115200 baud rate
2. **Check for sensor readings**:
   ```
   === Sensor Readings ===
   Temperature: 23.50°C
   Humidity: 65.20%
   Soil Moisture: 2048
   Light Level: 1500
   =====================
   ```

3. **Verify WiFi connection** and data transmission

### 3.2 Raspberry Pi Testing

1. **Check server status**:
   ```bash
   curl http://localhost:8080/health
   ```

2. **View recent data**:
   ```bash
   curl http://localhost:8080/data?limit=5
   ```

3. **Open web dashboard**:
   - Copy `web_dashboard.html` to a web server or open directly
   - Update the IP address in the JavaScript code

## Step 4: Troubleshooting

### Common ESP32 Issues

1. **WiFi Connection Failed**
   - Check WiFi credentials
   - Ensure ESP32 is within range
   - Restart device if needed

2. **Sensor Readings are Zero**
   - Check wiring connections
   - Verify sensor power supply
   - Test sensors individually

3. **Data Transmission Fails**
   - Check Raspberry Pi IP address
   - Ensure server is running
   - Verify network connectivity

### Common Raspberry Pi Issues

1. **Server Won't Start**
   - Check Python dependencies
   - Verify port 8080 is available
   - Check firewall settings

2. **Database Errors**
   - Ensure write permissions
   - Check disk space
   - Verify SQLite installation

3. **Network Connectivity**
   - Check IP address configuration
   - Verify firewall rules
   - Test network connectivity

## Step 5: Advanced Configuration

### Customizing Sensor Readings

Edit `src/main.cpp` to adjust:
- **Data transmission interval** (default: 30 seconds)
- **Pin assignments** for different sensors
- **Server URL** for different Raspberry Pi IPs

### Adding New Sensors

1. **Define new pin** in the pin definitions section
2. **Add sensor reading** in the `readSensors()` function
3. **Update data structure** to include new sensor
4. **Modify JSON payload** in `sendDataToServer()`

### Web Dashboard Customization

The `web_dashboard.html` file can be customized to:
- Add new sensor displays
- Include charts and graphs
- Modify the visual design
- Add alert systems

## Monitoring and Maintenance

### Regular Maintenance

1. **Check sensor readings** weekly
2. **Clean sensors** monthly
3. **Update software** as needed
4. **Backup database** regularly

### Data Management

The system automatically:
- **Logs all sensor data** to SQLite database
- **Cleans old data** (keeps last 30 days)
- **Provides API endpoints** for data access

### Scaling the System

To monitor multiple plants:
1. **Deploy multiple ESP32s** with different device IDs
2. **Update server** to handle multiple devices
3. **Modify dashboard** to show multiple plants

## Support and Resources

- **GitHub Repository**: [Link to your repo]
- **Documentation**: This setup guide
- **Community**: Arduino and Raspberry Pi forums
- **Troubleshooting**: Check the troubleshooting section above

## License

This project is open source and available under the MIT License. 