#!/bin/bash
# ESP32 Plant Monitor v0.1 Release Script
# ======================================
# This script creates a release package for the ESP32 Plant Monitor v0.1
# without requiring git status checks that can hang.

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    local status=$1
    local message=$2
    case $status in
        "INFO")
            echo -e "${BLUE}ℹ️  $message${NC}"
            ;;
        "SUCCESS")
            echo -e "${GREEN}✅ $message${NC}"
            ;;
        "WARNING")
            echo -e "${YELLOW}⚠️  $message${NC}"
            ;;
        "ERROR")
            echo -e "${RED}❌ $message${NC}"
            ;;
    esac
}

# Check if required commands exist
check_command() {
    if ! command -v $1 &> /dev/null; then
        print_status "ERROR" "$1 is not installed"
        exit 1
    fi
}

echo "ℹ️  Preparing ESP32 Plant Monitor v0.1 Release"
echo "=================================================="

# Check required commands
check_command "tar"
check_command "zip"

# Create release directory
RELEASE_DIR="release_v0.1"
print_status "INFO" "Creating release directory: $RELEASE_DIR"

if [ -d "$RELEASE_DIR" ]; then
    rm -rf "$RELEASE_DIR"
fi
mkdir -p "$RELEASE_DIR"

# Copy project files (excluding build artifacts and git files)
print_status "INFO" "Copying project files..."

# Create release directory structure
mkdir -p "$RELEASE_DIR/src/sensors"
mkdir -p "$RELEASE_DIR/src/display"

# Copy source files individually to avoid recursion
print_status "INFO" "Copying source files..."

# Copy main source file
if [ -f "src/main.cpp" ]; then
    cp "src/main.cpp" "$RELEASE_DIR/src/"
fi

# Copy sensor files
if [ -f "src/sensors/sensor_interface.h" ]; then
    cp "src/sensors/sensor_interface.h" "$RELEASE_DIR/src/sensors/"
fi
if [ -f "src/sensors/sensor_interface.c" ]; then
    cp "src/sensors/sensor_interface.c" "$RELEASE_DIR/src/sensors/"
fi
if [ -f "src/sensors/aht10.h" ]; then
    cp "src/sensors/aht10.h" "$RELEASE_DIR/src/sensors/"
fi
if [ -f "src/sensors/aht10.c" ]; then
    cp "src/sensors/aht10.c" "$RELEASE_DIR/src/sensors/"
fi
if [ -f "src/sensors/ds18b20.h" ]; then
    cp "src/sensors/ds18b20.h" "$RELEASE_DIR/src/sensors/"
fi
if [ -f "src/sensors/ds18b20.c" ]; then
    cp "src/sensors/ds18b20.c" "$RELEASE_DIR/src/sensors/"
fi
if [ -f "src/sensors/gy302.h" ]; then
    cp "src/sensors/gy302.h" "$RELEASE_DIR/src/sensors/"
fi
if [ -f "src/sensors/gy302.c" ]; then
    cp "src/sensors/gy302.c" "$RELEASE_DIR/src/sensors/"
fi

# Copy display files
if [ -f "src/display/display_interface.h" ]; then
    cp "src/display/display_interface.h" "$RELEASE_DIR/src/display/"
fi
if [ -f "src/display/display_interface.c" ]; then
    cp "src/display/display_interface.c" "$RELEASE_DIR/src/display/"
fi

# Copy CMakeLists.txt
if [ -f "src/CMakeLists.txt" ]; then
    cp "src/CMakeLists.txt" "$RELEASE_DIR/src/"
fi

# Copy configuration files
print_status "INFO" "Copying configuration files..."
config_files=(
    "platformio.ini"
    "config.h"
    "sdkconfig.defaults"
    "partitions.csv"
)

for file in "${config_files[@]}"; do
    if [ -f "$file" ]; then
        cp "$file" "$RELEASE_DIR/"
    fi
done

# Copy scripts
print_status "INFO" "Copying scripts..."
script_files=(
    "setup.sh"
    "run_tests.sh"
    "test_build.sh"
    "terminal_config.sh"
)

for file in "${script_files[@]}"; do
    if [ -f "$file" ]; then
        cp "$file" "$RELEASE_DIR/"
    fi
done

# Copy documentation
print_status "INFO" "Copying documentation..."
doc_files=(
    "VERSION.md"
    "README.md"
    "LICENSE"
    "requirements.txt"
)

for file in "${doc_files[@]}"; do
    if [ -f "$file" ]; then
        cp "$file" "$RELEASE_DIR/"
    fi
done

# Create release notes
print_status "INFO" "Creating RELEASE_NOTES.md..."
cat > "$RELEASE_DIR/RELEASE_NOTES.md" << 'EOF'
# ESP32 Plant Monitor v0.1.0 Release Notes

## 🎉 Initial Release

This is the first official release of the ESP32 Plant Monitor system.

### ✨ Features

- **Modular Sensor Architecture**: Support for AHT10, DS18B20, GY-302, and analog sensors
- **Plant Health Algorithm**: Real-time plant health scoring with emoji indicators
- **Web Dashboard**: Remote monitoring interface
- **Terminal Safety**: Cursor IDE integration with timeout protection
- **Comprehensive Testing**: Automated build and test scripts
- **Complete Documentation**: Setup guides and API documentation
- **Raspberry Pi Backend**: Server for data logging and analysis

### 🔧 Technical Specifications

- **Hardware**: ESP32-C6 DevKitC-1
- **Framework**: ESP-IDF 5.5.0
- **Build System**: PlatformIO
- **Sensors**: Temperature, Humidity, Soil Moisture, Light Level
- **Displays**: Console, OLED, E-paper support
- **Communication**: WiFi, I2C, One-Wire, ADC

### 📦 Installation

1. Extract the release archive
2. Run `./setup.sh` to install dependencies
3. Configure `config.h` with your WiFi credentials
4. Run `./test_build.sh` to verify the build
5. Upload to ESP32 with `pio run --target upload`

### 🐛 Known Issues

- AHT10 sensor may show initialization errors if not connected
- OLED and E-paper displays show "not supported" warnings (normal for console-only mode)
- Some sensors return 0 values when not connected (expected behavior)

### 🔄 Future Roadmap

- v0.2: Web dashboard improvements
- v0.3: Additional sensor support
- v0.4: Mobile app integration
- v1.0: Production-ready release

### 📄 License

MIT License - see LICENSE file for details.

---

**Release Date**: $(date)
**Version**: v0.1.0
**Build**: $(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
EOF

# Create install script
print_status "INFO" "Creating install.sh..."
cat > "$RELEASE_DIR/install.sh" << 'EOF'
#!/bin/bash
# ESP32 Plant Monitor v0.1.0 Installation Script

set -e

echo "🌱 ESP32 Plant Monitor v0.1.0 Installation"
echo "=========================================="

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo "❌ PlatformIO not found. Installing..."
    curl -fsSL https://raw.githubusercontent.com/platformio/platformio/master/scripts/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
    python3 -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/master/scripts/get-platformio.py)"
    echo "✅ PlatformIO installed"
else
    echo "✅ PlatformIO already installed"
fi

# Install Python dependencies
if [ -f "requirements.txt" ]; then
    echo "📦 Installing Python dependencies..."
    pip3 install -r requirements.txt
fi

# Make scripts executable
chmod +x *.sh

echo "✅ Installation complete!"
echo ""
echo "Next steps:"
echo "1. Configure config.h with your WiFi credentials"
echo "2. Run ./test_build.sh to verify the build"
echo "3. Connect ESP32 and run: pio run --target upload"
echo "4. Monitor with: pio device monitor"
EOF

chmod +x "$RELEASE_DIR/install.sh"

# Create test script
print_status "INFO" "Creating test_release.sh..."
cat > "$RELEASE_DIR/test_release.sh" << 'EOF'
#!/bin/bash
# Test script for ESP32 Plant Monitor v0.1.0

set -e

echo "🧪 Testing ESP32 Plant Monitor v0.1.0"
echo "====================================="

# Test build
echo "🔨 Testing build..."
if ./test_build.sh; then
    echo "✅ Build test passed"
else
    echo "❌ Build test failed"
    exit 1
fi

# Test file structure
echo "📁 Testing file structure..."
required_files=(
    "src/main.cpp"
    "platformio.ini"
    "config.h"
    "setup.sh"
    "test_build.sh"
)

for file in "${required_files[@]}"; do
    if [ -f "$file" ]; then
        echo "✅ $file"
    else
        echo "❌ $file - MISSING"
        exit 1
    fi
done

echo "✅ All tests passed!"
echo ""
echo "🎉 Release package is ready!"
EOF

chmod +x "$RELEASE_DIR/test_release.sh"

# Create summary
print_status "INFO" "Creating SUMMARY.txt..."
cat > "$RELEASE_DIR/SUMMARY.txt" << 'EOF'
ESP32 Plant Monitor v0.1.0 Release Package
==========================================

This package contains the complete ESP32 Plant Monitor system.

Contents:
- src/                    Source code directory
- platformio.ini         PlatformIO configuration
- config.h               Configuration file
- setup.sh              Setup script
- test_build.sh         Build test script
- terminal_config.sh    Terminal safety configuration
- VERSION.md            Version documentation
- README.md             Project documentation
- LICENSE               MIT License
- requirements.txt      Python dependencies
- install.sh            Installation script
- test_release.sh       Release test script

Quick Start:
1. Run: ./install.sh
2. Configure: config.h
3. Test: ./test_build.sh
4. Upload: pio run --target upload
5. Monitor: pio device monitor

Features:
- Modular sensor architecture
- Plant health algorithm
- Web dashboard
- Terminal safety
- Comprehensive testing
- Complete documentation

Hardware: ESP32-C6 DevKitC-1
Framework: ESP-IDF 5.5.0
License: MIT
EOF

# Create archives
print_status "INFO" "Creating release archives..."

# Create tar.gz
tar -czf "esp32-plant-monitor-v0.1.0.tar.gz" -C "$RELEASE_DIR" .
print_status "SUCCESS" "Created esp32-plant-monitor-v0.1.0.tar.gz"

# Create zip
cd "$RELEASE_DIR"
zip -r "../esp32-plant-monitor-v0.1.0.zip" . > /dev/null
cd ..
print_status "SUCCESS" "Created esp32-plant-monitor-v0.1.0.zip"

# Cleanup completed

print_status "SUCCESS" "Release package created successfully!"
echo ""
echo "📦 Release files:"
echo "  - esp32-plant-monitor-v0.1.0.tar.gz"
echo "  - esp32-plant-monitor-v0.1.0.zip"
echo "  - $RELEASE_DIR/ (source directory)"
echo ""
echo "🚀 Ready for GitHub release!"
echo ""
echo "Next steps:"
echo "1. Upload archives to GitHub release"
echo "2. Tag the release as v0.1.0"
echo "3. Update documentation if needed" 