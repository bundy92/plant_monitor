#!/bin/bash

# Plant Monitor - Deployment Script
# =================================
# This script deploys the clean, industry-standard plant monitoring system
# with minimal complexity and maximum reliability.

set -e  # Exit on any error

echo "🌱 Plant Monitor - Deployment"
echo "============================="

# Check if we're in the right directory
if [ ! -f "platformio.ini" ]; then
    echo "❌ Error: platformio.ini not found. Please run this script from the project root."
    exit 1
fi

# Check if main_example.cpp exists
if [ ! -f "src/main_example.cpp" ]; then
    echo "❌ Error: src/main_example.cpp not found."
    exit 1
fi

# Check if plant_monitor.h exists
if [ ! -f "src/plant_monitor.h" ]; then
    echo "❌ Error: src/plant_monitor.h not found."
    exit 1
fi

# Check if plant_monitor.c exists
if [ ! -f "src/plant_monitor.c" ]; then
    echo "❌ Error: src/plant_monitor.c not found."
    exit 1
fi

echo "✅ Project structure verified"

# Clean previous build
echo "🧹 Cleaning previous build..."
pio run --target clean

# Build the project
echo "🔨 Building project..."
pio run

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
else
    echo "❌ Build failed!"
    exit 1
fi

# Check if upload port is specified
if [ -n "$1" ]; then
    UPLOAD_PORT="$1"
    echo "📤 Uploading to port: $UPLOAD_PORT"
    pio run --target upload --upload-port "$UPLOAD_PORT"
else
    echo "📤 Uploading to auto-detected port..."
    pio run --target upload
fi

if [ $? -eq 0 ]; then
    echo "✅ Upload successful!"
    echo ""
    echo "🎉 Plant Monitor deployed successfully!"
    echo ""
    echo "📊 Features:"
    echo "- Clean, industry-standard architecture"
    echo "- Professional numpy-style documentation"
    echo "- Robust error handling and recovery"
    echo "- Plant health analysis with emoji indicators"
    echo "- Analog sensor support (soil moisture, light)"
    echo "- WiFi connectivity and data transmission"
    echo "- Display output with health status"
    echo ""
    echo "🔍 Monitor output with: pio device monitor"
    echo "🛑 Stop monitoring with: Ctrl+C"
else
    echo "❌ Upload failed!"
    echo ""
    echo "💡 Troubleshooting:"
    echo "1. Check USB connection"
    echo "2. Verify correct port: ./deploy.sh /dev/ttyACM0"
    echo "3. Try different USB cable"
    echo "4. Check device permissions"
    exit 1
fi 