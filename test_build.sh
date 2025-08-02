#!/bin/bash

# Test Build Script for ESP32-C6 Plant Monitor

echo "🧪 Testing ESP32-C6 Plant Monitor Build"
echo "======================================="

# Check if PlatformIO is available
if ! command -v pio &> /dev/null; then
    echo "❌ PlatformIO not found. Please install PlatformIO first."
    echo "   Run: pip install platformio"
    exit 1
fi

echo "✅ PlatformIO found"

# Check project structure
echo "📁 Checking project structure..."
if [ ! -f "src/main.cpp" ]; then
    echo "❌ src/main.cpp not found"
    exit 1
fi

if [ ! -f "config.h" ]; then
    echo "❌ config.h not found"
    exit 1
fi

if [ ! -f "platformio.ini" ]; then
    echo "❌ platformio.ini not found"
    exit 1
fi

echo "✅ Project structure looks good"

# Test build
echo "🔨 Testing build..."
if pio run --target build; then
    echo "✅ Build test successful"
    echo "🎉 Project is ready for deployment!"
else
    echo "❌ Build test failed"
    exit 1
fi 