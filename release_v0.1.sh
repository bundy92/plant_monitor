#!/bin/bash
# ESP32 Plant Monitor - v0.1 Release Script
# ==========================================
# This script prepares the v0.1 release of the ESP32 Plant Monitor system

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print status messages
print_status() {
    local level=$1
    local message=$2
    case $level in
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

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check if we're in a git repository
if [ ! -d ".git" ]; then
    print_status "ERROR" "Not in a git repository. Please initialize git first."
    exit 1
fi

print_status "INFO" "Preparing ESP32 Plant Monitor v0.1 Release"
echo "=================================================="

# Check for uncommitted changes
if [ -n "$(git status --porcelain)" ]; then
    print_status "WARNING" "There are uncommitted changes. Please commit or stash them first."
    git status --short
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Create release directory
RELEASE_DIR="release_v0.1"
print_status "INFO" "Creating release directory: $RELEASE_DIR"
rm -rf "$RELEASE_DIR"
mkdir -p "$RELEASE_DIR"

# Copy project files (excluding build artifacts and git)
print_status "INFO" "Copying project files..."
rsync -av --exclude='.git' --exclude='.pio' --exclude='build' --exclude='*.log' --exclude='*.bin' --exclude='*.elf' --exclude='*.map' --exclude='*.hex' --exclude='*.dis' --exclude='*.a' --exclude='*.o' --exclude='*.d' --exclude='*.pyc' --exclude='__pycache__' --exclude='.DS_Store' --exclude='Thumbs.db' . "$RELEASE_DIR/"

# Create release documentation
print_status "INFO" "Creating release documentation..."

cat > "$RELEASE_DIR/RELEASE_NOTES.md" << 'EOF'
# ESP32 Plant Monitor - v0.1 Release Notes

## 🎉 Initial Release

This is the first public release of the ESP32 Plant Monitor system, featuring a comprehensive plant monitoring solution with modular sensor support and web dashboard integration.

## ✨ What's New

### Core Features
- **Modular Sensor Architecture**: Support for multiple sensor types with unified interface
- **ESP32-C6 Support**: Optimized for ESP32-C6-DevKitC-1 development board
- **Real-time Monitoring**: Continuous sensor data collection and health assessment
- **Plant Health Algorithm**: Intelligent plant health scoring based on environmental conditions

### Supported Sensors
- **AHT10**: Temperature and humidity sensors (I2C)
- **DS18B20**: Waterproof temperature sensors (One-Wire)
- **GY-302**: Digital light intensity sensors (I2C)
- **Analog Sensors**: Soil moisture and light level sensors (ADC)

### Development Features
- **Terminal Safety**: Enhanced terminal configuration to prevent hanging in Cursor IDE
- **Comprehensive Testing**: Unit tests, integration tests, and system verification
- **Documentation**: Complete setup guides and hardware documentation
- **Raspberry Pi Server**: Backend server for data collection and storage

## 🚀 Quick Start

1. **Hardware Setup**: Follow `HARDWARE_SETUP.md`
2. **Software Setup**: Run `./setup.sh`
3. **Configuration**: Edit `config.h` for your network
4. **Build & Upload**: `pio run --target upload`
5. **Monitor**: `pio device monitor`

## 📋 Requirements

### Hardware
- ESP32-C6-DevKitC-1 development board
- AHT10 temperature/humidity sensors
- DS18B20 waterproof temperature sensors
- GY-302 light intensity sensors
- Analog soil moisture and light sensors
- Optional: SSD1306 OLED display

### Software
- PlatformIO IDE or CLI
- ESP-IDF 5.5.0
- Python 3.8+ (for Raspberry Pi server)

## 🐛 Known Issues

1. **Flash Size Warning**: Expected 8MB, found 2MB (non-critical)
2. **ADC Calibration**: Raw values only (calibration planned for v0.2)
3. **WiFi Credentials**: Hardcoded in config.h (environment variables planned)

## 📈 Future Plans

- MQTT support for IoT integration
- Mobile app companion
- Advanced plant health algorithms
- Sensor calibration tools
- Machine learning for plant health prediction

## 🤝 Contributing

This project welcomes contributions! Please see the documentation for guidelines.

---

**Release Date**: August 3, 2024  
**Version**: v0.1.0  
**Maintainer**: Plant Monitor System Team
EOF

# Create installation script
cat > "$RELEASE_DIR/install.sh" << 'EOF'
#!/bin/bash
# ESP32 Plant Monitor - Installation Script
# ========================================
# This script installs the ESP32 Plant Monitor system

set -e

echo "Installing ESP32 Plant Monitor v0.1..."
echo "======================================"

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo "PlatformIO not found. Installing..."
    pip install platformio
fi

# Check if we're in the right directory
if [ ! -f "platformio.ini" ]; then
    echo "Error: platformio.ini not found. Please run this script from the project root."
    exit 1
fi

# Install dependencies
echo "Installing dependencies..."
pio pkg install

# Build the project
echo "Building project..."
pio run --target build

echo "Installation complete!"
echo "Next steps:"
echo "1. Edit config.h with your WiFi credentials"
echo "2. Connect your ESP32-C6-DevKitC-1 board"
echo "3. Run: pio run --target upload"
echo "4. Monitor output: pio device monitor"
EOF

chmod +x "$RELEASE_DIR/install.sh"

# Create a simple test script
cat > "$RELEASE_DIR/test_release.sh" << 'EOF'
#!/bin/bash
# ESP32 Plant Monitor - Release Test Script
# ========================================
# This script tests the basic functionality of the release

set -e

echo "Testing ESP32 Plant Monitor v0.1 Release"
echo "========================================"

# Test if PlatformIO is available
if command -v pio &> /dev/null; then
    echo "✅ PlatformIO found"
else
    echo "❌ PlatformIO not found"
    exit 1
fi

# Test if we can build the project
echo "Testing build..."
if pio run --target build --silent; then
    echo "✅ Build successful"
else
    echo "❌ Build failed"
    exit 1
fi

# Test if configuration files exist
echo "Testing configuration files..."
if [ -f "config.h" ]; then
    echo "✅ config.h found"
else
    echo "❌ config.h missing"
fi

if [ -f "platformio.ini" ]; then
    echo "✅ platformio.ini found"
else
    echo "❌ platformio.ini missing"
fi

if [ -f "src/main.cpp" ]; then
    echo "✅ main.cpp found"
else
    echo "❌ main.cpp missing"
fi

echo "Release test completed successfully!"
EOF

chmod +x "$RELEASE_DIR/test_release.sh"

# Create a summary file
cat > "$RELEASE_DIR/SUMMARY.txt" << 'EOF'
ESP32 Plant Monitor v0.1 Release Summary
========================================

Release Date: August 3, 2024
Version: v0.1.0
Target Board: ESP32-C6-DevKitC-1
Framework: ESP-IDF 5.5.0

Files Included:
- Complete source code (src/)
- Configuration files (config.h, platformio.ini)
- Documentation (README.md, HARDWARE_SETUP.md, etc.)
- Test scripts (run_tests.sh, test_aht10.sh)
- Raspberry Pi server (raspberry_pi/)
- Web dashboard (web_dashboard.html)
- Terminal safety configuration
- Release documentation

Key Features:
- Modular sensor architecture
- Plant health algorithm
- Real-time monitoring
- Web dashboard
- Terminal safety features
- Comprehensive testing

Supported Sensors:
- AHT10 (Temperature/Humidity)
- DS18B20 (Waterproof Temperature)
- GY-302 (Light Intensity)
- Analog sensors (Soil Moisture, Light)

Installation:
1. Run ./install.sh
2. Edit config.h with WiFi credentials
3. Connect ESP32-C6-DevKitC-1
4. Run: pio run --target upload
5. Monitor: pio device monitor

Testing:
- Run ./test_release.sh for basic functionality test
- Run ./run_tests.sh for comprehensive testing

Documentation:
- README.md: Main documentation
- HARDWARE_SETUP.md: Hardware setup guide
- PROJECT_SUMMARY.md: Technical overview
- VERSION.md: Version history

Known Issues:
- Flash size warning (non-critical)
- ADC calibration pending
- WiFi credentials hardcoded

Future Plans:
- MQTT support
- Mobile app
- Advanced algorithms
- Cloud integration
EOF

# Create a compressed archive
print_status "INFO" "Creating release archive..."
tar -czf "esp32_plant_monitor_v0.1.tar.gz" "$RELEASE_DIR"

# Create a zip file as well
print_status "INFO" "Creating ZIP archive..."
zip -r "esp32_plant_monitor_v0.1.zip" "$RELEASE_DIR"

# Show release summary
print_status "SUCCESS" "Release v0.1 prepared successfully!"
echo ""
echo "Release files created:"
echo "  📁 $RELEASE_DIR/ (release directory)"
echo "  📦 esp32_plant_monitor_v0.1.tar.gz"
echo "  📦 esp32_plant_monitor_v0.1.zip"
echo ""
echo "Release contents:"
echo "  📄 RELEASE_NOTES.md - Release notes and features"
echo "  🔧 install.sh - Installation script"
echo "  🧪 test_release.sh - Release test script"
echo "  📋 SUMMARY.txt - Release summary"
echo "  📚 Complete project documentation"
echo "  💻 Full source code"
echo ""
echo "Next steps:"
echo "  1. Review the release files"
echo "  2. Test the installation: cd $RELEASE_DIR && ./test_release.sh"
echo "  3. Create a git tag: git tag -a v0.1.0 -m 'Release v0.1.0'"
echo "  4. Push the tag: git push origin v0.1.0"
echo "  5. Upload the archives to GitHub releases"
echo ""
print_status "INFO" "Release v0.1 preparation complete! 🎉" 