#!/bin/bash
# Plant Monitor - Complete Setup Script
# ====================================
# This script sets up the entire plant monitoring system including
# ESP32 development environment, Raspberry Pi server, and testing framework.

set -e  # Exit on any error

# Load terminal configuration to prevent hanging issues
if [ -f "terminal_config.sh" ]; then
    source terminal_config.sh
fi

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

# Function to install Python virtual environment
setup_python_env() {
    print_status "INFO" "Setting up Python virtual environment"
    
    if [ ! -d "plant_monitor_env" ]; then
        python3 -m venv plant_monitor_env
        print_status "SUCCESS" "Created Python virtual environment"
    else
        print_status "INFO" "Python virtual environment already exists"
    fi
    
    # Activate virtual environment and install core dependencies
    source plant_monitor_env/bin/activate
    
    print_status "INFO" "Installing core Python dependencies"
    pip install --upgrade pip
    
    # Install core dependencies (skip problematic packages)
    pip install flask flask-cors flask-socketio flask-limiter \
                sqlalchemy marshmallow pyyaml cryptography \
                python-dotenv prometheus-client psutil \
                pytest pytest-cov pytest-mock pytest-asyncio \
                requests urllib3 certifi charset-normalizer idna
    
    print_status "SUCCESS" "Core Python dependencies installed"
    
    # Try to install optional dependencies
    print_status "INFO" "Installing optional dependencies (may fail on some systems)"
    pip install black flake8 mypy isort || print_status "WARNING" "Some optional packages failed to install"
    
    print_status "SUCCESS" "Python environment setup complete"
}

# Function to check ESP32 development environment
check_esp32_env() {
    print_status "INFO" "Checking ESP32 development environment"
    
    if ! command_exists pio; then
        print_status "ERROR" "PlatformIO not found. Please install PlatformIO first:"
        echo "  pip3 install platformio"
        echo "  or visit: https://platformio.org/install"
        return 1
    fi
    
    print_status "SUCCESS" "PlatformIO found"
    
    # Check if we're in the right directory
    if [ ! -f "platformio.ini" ]; then
        print_status "ERROR" "platformio.ini not found. Please run this script from the project root."
        return 1
    fi
    
    print_status "SUCCESS" "ESP32 development environment ready"
}

# Function to test ESP32 build
test_esp32_build() {
    print_status "INFO" "Testing ESP32 build"
    
    if pio run --target build; then
        print_status "SUCCESS" "ESP32 build successful"
    else
        print_status "ERROR" "ESP32 build failed. Check the error messages above."
        return 1
    fi
}

# Function to test Raspberry Pi server
test_raspberry_pi_server() {
    print_status "INFO" "Testing Raspberry Pi server"
    
    # Activate virtual environment
    source plant_monitor_env/bin/activate
    
    # Test basic imports
    if python3 -c "
import flask
import sqlalchemy
import marshmallow
print('✅ All core dependencies imported successfully')
"; then
        print_status "SUCCESS" "Raspberry Pi server dependencies working"
    else
        print_status "ERROR" "Raspberry Pi server dependencies failed"
        return 1
    fi
}

# Function to run basic tests
run_basic_tests() {
    print_status "INFO" "Running basic tests"
    
    # Test ESP32 compilation
    if test_esp32_build; then
        print_status "SUCCESS" "ESP32 compilation test passed"
    else
        print_status "ERROR" "ESP32 compilation test failed"
        return 1
    fi
    
    # Test Python server
    if test_raspberry_pi_server; then
        print_status "SUCCESS" "Raspberry Pi server test passed"
    else
        print_status "ERROR" "Raspberry Pi server test failed"
        return 1
    fi
}

# Function to create configuration files
create_config_files() {
    print_status "INFO" "Creating configuration files"
    
    # Create server config directory
    mkdir -p raspberry_pi/config
    
    # Create server configuration file
    cat > raspberry_pi/config/server_config.yaml << 'EOF'
server:
  host: '0.0.0.0'
  port: 5000
  debug: false

database:
  url: 'sqlite:///plant_monitor.db'

alerts:
  temperature_min: 10.0
  temperature_max: 35.0
  humidity_min: 30.0
  humidity_max: 80.0
  soil_moisture_min: 1000
  soil_moisture_max: 3000
  light_level_min: 500
  light_level_max: 3500

security:
  rate_limit_per_minute: 100
  enable_cors: true
  enable_rate_limiting: true
EOF
    
    print_status "SUCCESS" "Configuration files created"
}

# Function to display setup summary
show_summary() {
    print_status "INFO" "Setup Summary"
    echo "========================================="
    echo "✅ ESP32 Development Environment: Ready"
    echo "✅ Python Virtual Environment: Ready"
    echo "✅ Core Dependencies: Installed"
    echo "✅ Configuration Files: Created"
    echo ""
    echo "Next Steps:"
    echo "1. Configure WiFi credentials in config.h"
    echo "2. Update server URL in config.h"
    echo "3. Connect ESP32 hardware"
    echo "4. Run: ./run_tests.sh"
    echo "5. For server: cd raspberry_pi && python3 server.py"
    echo ""
    echo "Documentation:"
    echo "- README.md: Main documentation"
    echo "- HARDWARE_SETUP.md: Hardware setup guide"
    echo "- raspberry_pi/README.md: Server documentation"
}

# Main setup function
main() {
    print_status "INFO" "Starting Plant Monitor Setup"
    echo "========================================="
    
    # Check prerequisites
    if ! command_exists python3; then
        print_status "ERROR" "Python 3 not found. Please install Python 3.8 or higher."
        exit 1
    fi
    
    if ! command_exists pip; then
        print_status "ERROR" "pip not found. Please install pip."
        exit 1
    fi
    
    # Setup Python environment
    setup_python_env
    
    # Check ESP32 environment
    check_esp32_env
    
    # Create configuration files
    create_config_files
    
    # Run basic tests
    run_basic_tests
    
    # Show summary
    show_summary
    
    print_status "SUCCESS" "Setup completed successfully!"
}

# Run main function
main "$@" 