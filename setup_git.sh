#!/bin/bash

echo "Setting up Git repository for ESP32 Plant Monitor..."

# Initialize Git repository
git init

# Add all files
git add .

# Make initial commit
git commit -m "Initial commit: ESP32-C6 Plant Monitor with AHT10 sensors

- ESP-IDF framework with PlatformIO
- Dual AHT10 temperature/humidity sensors
- Soil moisture and light sensors
- WiFi connectivity and data transmission
- Comprehensive documentation and testing tools
- Web dashboard and Raspberry Pi server"

echo "Git repository initialized successfully!"
echo "You can now push to a remote repository with:"
echo "git remote add origin <your-repo-url>"
echo "git push -u origin main" 