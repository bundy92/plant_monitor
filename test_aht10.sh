#!/bin/bash

# AHT10 Test Script for ESP32-C6 Plant Monitor

echo "🧪 AHT10 Sensor Test Script"
echo "=========================="

# Check if we're in test mode or full mode
if [ -f "src/main_backup.cpp" ]; then
    echo "📋 Current status: TEST MODE (simple_test.cpp active)"
    echo ""
    echo "Options:"
    echo "1. Run test (upload and monitor)"
    echo "2. Switch to FULL PROJECT mode"
    echo "3. Exit"
    echo ""
    read -p "Choose option (1-3): " choice
    
    case $choice in
        1)
            echo "🚀 Uploading test code and starting monitor..."
            pio run --target upload
            echo "📊 Starting monitor (Ctrl+C to exit)..."
            pio device monitor
            ;;
        2)
            echo "🔄 Switching to FULL PROJECT mode..."
            cp src/main_backup.cpp src/main.cpp
            rm src/main_backup.cpp
            echo "✅ Switched to full project mode"
            echo "📤 Uploading full project..."
            pio run --target upload
            ;;
        3)
            echo "👋 Exiting..."
            exit 0
            ;;
        *)
            echo "❌ Invalid option"
            exit 1
            ;;
    esac
else
    echo "📋 Current status: FULL PROJECT mode"
    echo ""
    echo "Options:"
    echo "1. Switch to TEST MODE (simple AHT10 test)"
    echo "2. Upload and monitor full project"
    echo "3. Exit"
    echo ""
    read -p "Choose option (1-3): " choice
    
    case $choice in
        1)
            echo "🔄 Switching to TEST MODE..."
            cp src/main.cpp src/main_backup.cpp
            cp simple_test.cpp src/main.cpp
            echo "✅ Switched to test mode"
            echo "📤 Uploading test code..."
            pio run --target upload
            echo "📊 Starting monitor (Ctrl+C to exit)..."
            pio device monitor
            ;;
        2)
            echo "📤 Uploading full project..."
            pio run --target upload
            echo "📊 Starting monitor (Ctrl+C to exit)..."
            pio device monitor
            ;;
        3)
            echo "👋 Exiting..."
            exit 0
            ;;
        *)
            echo "❌ Invalid option"
            exit 1
            ;;
    esac
fi 