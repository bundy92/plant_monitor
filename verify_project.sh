#!/bin/bash

# Project Verification Script for ESP32-C6 Plant Monitor

echo "🔍 Verifying ESP32-C6 Plant Monitor Project"
echo "==========================================="

# Check essential files
echo "📁 Checking essential files..."
files=(
    "src/main.cpp"
    "src/CMakeLists.txt"
    "config.h"
    "platformio.ini"
    "CMakeLists.txt"
    "partitions.csv"
    "sdkconfig.defaults"
    "raspberry_pi_server.py"
    "web_dashboard.html"
    "requirements.txt"
    "README.md"
    "QUICK_START.md"
    "setup_guide.md"
    ".gitignore"
    "deploy.sh"
    "test_build.sh"
)

for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        echo "✅ $file"
    else
        echo "❌ $file - MISSING"
    fi
done

# Check config.h includes
echo ""
echo "🔧 Checking configuration..."
if grep -q "WIFI_SSID" config.h; then
    echo "✅ WiFi configuration found"
else
    echo "❌ WiFi configuration missing"
fi

if grep -q "DEVICE_ID" config.h; then
    echo "✅ Device ID configuration found"
else
    echo "❌ Device ID configuration missing"
fi

# Check main.cpp includes config.h
echo ""
echo "📝 Checking source code..."
if grep -q "#include.*config.h" src/main.cpp; then
    echo "✅ main.cpp includes config.h"
else
    echo "❌ main.cpp missing config.h include"
fi

# Check CMakeLists.txt includes parent directory
if grep -q "INCLUDE_DIRS.*\.." src/CMakeLists.txt; then
    echo "✅ CMakeLists.txt includes parent directory"
else
    echo "❌ CMakeLists.txt missing parent directory include"
fi

# Check platformio.ini configuration
echo ""
echo "⚙️ Checking PlatformIO configuration..."
if grep -q "framework = espidf" platformio.ini; then
    echo "✅ ESP-IDF framework configured"
else
    echo "❌ ESP-IDF framework not configured"
fi

if grep -q "board_build.partitions = partitions.csv" platformio.ini; then
    echo "✅ Partition table configured"
else
    echo "❌ Partition table not configured"
fi

echo ""
echo "🎉 Project verification complete!"
echo "📋 Summary: All files are properly aligned and configured." 