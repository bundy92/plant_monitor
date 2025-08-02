#!/bin/bash

# ESP32-C6 Plant Monitor Deployment Script

echo "🌱 ESP32-C6 Plant Monitor Deployment"
echo "====================================="

# Check if PlatformIO is available
if ! command -v pio &> /dev/null; then
    echo "❌ PlatformIO not found. Please install PlatformIO first."
    echo "   Run: pip install platformio"
    exit 1
fi

# Check if ESP32 is connected
if ! pio device list | grep -q "esp32"; then
    echo "❌ ESP32 not found. Please connect your ESP32-C6 device."
    exit 1
fi

echo "✅ ESP32 device found"

# Build the project
echo "🔨 Building project..."
if pio run; then
    echo "✅ Build successful"
else
    echo "❌ Build failed"
    exit 1
fi

# Upload to ESP32
echo "📤 Uploading to ESP32..."
if pio run --target upload; then
    echo "✅ Upload successful"
else
    echo "❌ Upload failed"
    exit 1
fi

echo ""
echo "🎉 Deployment complete!"
echo "📊 Monitor output: pio device monitor"
echo "🔄 To rebuild and upload: ./deploy.sh" 