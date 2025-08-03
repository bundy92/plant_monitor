#!/bin/bash
# Plant Monitor - Build Test Script
# =================================
# This script tests the build for errors and warnings

set -e  # Exit on any error

echo "🔧 Plant Monitor - Build Test"
echo "=============================="

# Check if we're in the right directory
if [ ! -f "platformio.ini" ]; then
    echo "❌ Error: platformio.ini not found. Please run this script from the project root."
    exit 1
fi

# Check for required files
echo "📋 Checking required files..."
required_files=(
    "src/main_example.cpp"
    "src/sensors/sensor_interface.h"
    "src/sensors/sensor_interface.c"
    "src/sensors/aht10.h"
    "src/sensors/aht10.c"
    "src/sensors/ds18b20.h"
    "src/sensors/ds18b20.c"
    "src/sensors/gy302.h"
    "src/sensors/gy302.c"
    "src/display/display_interface.h"
    "src/display/display_interface.c"
    "src/CMakeLists.txt"
    "platformio.ini"
    "config.h"
)

for file in "${required_files[@]}"; do
    if [ -f "$file" ]; then
        echo "✅ $file"
    else
        echo "❌ $file - MISSING"
        exit 1
    fi
done

echo ""
echo "🔍 Checking for common issues..."

# Check for undefined error codes
echo "Checking for undefined ESP_ERR codes..."
if grep -r "ESP_ERR_INVALID_CRC" src/; then
    echo "❌ Found ESP_ERR_INVALID_CRC - this is not a standard ESP-IDF error code"
    exit 1
fi

# Check for missing includes
echo "Checking for missing includes..."
if ! grep -q "#include.*esp_log.h" src/sensors/sensor_interface.c; then
    echo "❌ Missing esp_log.h include in sensor_interface.c"
    exit 1
fi

if ! grep -q "#include.*driver/i2c.h" src/sensors/sensor_interface.c; then
    echo "❌ Missing driver/i2c.h include in sensor_interface.c"
    exit 1
fi

# Check for function name mismatches
echo "Checking for function name mismatches..."
if grep -q "sensor_interface_get_status.*working_displays" src/display/display_interface.h; then
    echo "❌ Found incorrect function name in display_interface.h"
    exit 1
fi

echo "✅ All basic checks passed"
echo ""
echo "🚀 Ready for build test!"
echo ""
echo "To run the actual build test, use:"
echo "  pio run --target build"
echo ""
echo "To run with verbose output:"
echo "  pio run --target build -v"
echo ""
echo "To clean and rebuild:"
echo "  pio run --target clean && pio run --target build" 