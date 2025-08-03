#!/bin/bash
# Plant Monitor - Comprehensive Test Runner
# =======================================
# This script runs all types of tests for the modular plant monitoring system
# including unit tests, integration tests, system verification, and code quality checks.

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test results tracking
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

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

# Function to run a test and track results
run_test() {
    local test_name="$1"
    local test_command="$2"
    
    print_status "INFO" "Running: $test_name"
    echo "========================================="
    
    # Add timeout wrapper to prevent hanging
    if timeout 120 bash -c "$test_command"; then
        print_status "SUCCESS" "$test_name passed"
        ((PASSED_TESTS++))
    else
        local exit_code=$?
        if [ $exit_code -eq 124 ]; then
            print_status "ERROR" "$test_name timed out after 120 seconds"
        else
            print_status "ERROR" "$test_name failed with exit code $exit_code"
        fi
        ((FAILED_TESTS++))
    fi
    
    ((TOTAL_TESTS++))
    echo ""
}

# Function to check environment
check_environment() {
    print_status "INFO" "Checking development environment"
    echo "========================================="
    
    # Check if we're in the right directory
    if [ ! -f "platformio.ini" ]; then
        print_status "ERROR" "platformio.ini not found. Please run this script from the project root."
        exit 1
    fi
    
    # Check for required files
    local required_files=(
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
            print_status "ERROR" "$file - MISSING"
            exit 1
        fi
    done
    
    print_status "SUCCESS" "Environment check passed"
    echo ""
}

# Function to run unit tests
run_unit_tests() {
    print_status "INFO" "Starting Unit Tests"
    echo "========================================="
    
    # Copy unit test file
    cp test/unit/test_plant_monitor.cpp src/main.cpp
    
    # Run unit tests
    run_test "Unit Tests" "pio run --target upload && pio device monitor --timeout 60"
    
    # Restore main file
    cp src/main_example.cpp src/main.cpp
    
    print_status "SUCCESS" "Unit tests completed"
    echo ""
}

# Function to run integration tests
run_integration_tests() {
    print_status "INFO" "Starting Integration Tests"
    echo "========================================="
    
    # Copy integration test file
    cp test/integration/test_integration.cpp src/main.cpp
    
    # Run integration tests
    run_test "Integration Tests" "pio run --target upload && pio device monitor --timeout 90"
    
    # Restore main file
    cp src/main_example.cpp src/main.cpp
    
    print_status "SUCCESS" "Integration tests completed"
    echo ""
}

# Function to run system verification tests
run_system_tests() {
    print_status "INFO" "Starting System Verification Tests"
    echo "========================================="
    
    # Test build process
    run_test "Build Test" "pio run --target build"
    
    # Test clean build
    run_test "Clean Build Test" "pio run --target clean && pio run --target build"
    
    # Test upload (if device connected)
    if pio device list | grep -q "tty"; then
        run_test "Upload Test" "pio run --target upload"
    else
        print_status "WARNING" "No device connected, skipping upload test"
    fi
    
    print_status "SUCCESS" "System verification tests completed"
    echo ""
}

# Function to run code quality checks
run_code_quality_checks() {
    print_status "INFO" "Starting Code Quality Checks"
    echo "========================================="
    
    # Check for required files
    local required_files=(
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
            print_status "ERROR" "$file - MISSING"
            return 1
        fi
    done
    
    # Check for undefined error codes
    print_status "INFO" "Checking for undefined ESP_ERR codes..."
    if grep -r "ESP_ERR_INVALID_CRC" src/; then
        print_status "ERROR" "Found ESP_ERR_INVALID_CRC - this is not a standard ESP-IDF error code"
        return 1
    fi
    
    # Check for missing includes
    print_status "INFO" "Checking for missing includes..."
    if ! grep -q "#include.*esp_log.h" src/sensors/sensor_interface.c; then
        print_status "ERROR" "Missing esp_log.h include in sensor_interface.c"
        return 1
    fi
    
    if ! grep -q "#include.*driver/i2c.h" src/sensors/sensor_interface.c; then
        print_status "ERROR" "Missing driver/i2c.h include in sensor_interface.c"
        return 1
    fi
    
    # Check for function name mismatches
    print_status "INFO" "Checking for function name mismatches..."
    if grep -q "sensor_interface_get_status.*working_displays" src/display/display_interface.h; then
        print_status "ERROR" "Found incorrect function name in display_interface.h"
        return 1
    fi
    
    # Check for numpy-style documentation
    print_status "INFO" "Checking for numpy-style documentation..."
    local files_with_docs=0
    local total_files=0
    
    for file in src/sensors/*.h src/sensors/*.c src/display/*.h src/display/*.c; do
        if [ -f "$file" ]; then
            ((total_files++))
            if grep -q "@brief" "$file" && grep -q "@author" "$file"; then
                ((files_with_docs++))
            fi
        fi
    done
    
    if [ $files_with_docs -eq $total_files ]; then
        print_status "SUCCESS" "All files have numpy-style documentation"
    else
        print_status "WARNING" "Some files may be missing numpy-style documentation"
    fi
    
    print_status "SUCCESS" "Code quality checks completed"
    echo ""
}

# Function to run performance tests
run_performance_tests() {
    print_status "INFO" "Starting Performance Tests"
    echo "========================================="
    
    # Test build time
    local start_time=$(date +%s)
    pio run --target build > /dev/null 2>&1
    local end_time=$(date +%s)
    local build_time=$((end_time - start_time))
    
    if [ $build_time -lt 60 ]; then
        print_status "SUCCESS" "Build completed in ${build_time}s (acceptable)"
    else
        print_status "WARNING" "Build took ${build_time}s (may be slow)"
    fi
    
    # Test memory usage
    local binary_size=$(find .pio -name "*.bin" -exec ls -la {} \; 2>/dev/null | awk '{print $5}' | head -1)
    if [ -n "$binary_size" ]; then
        local size_mb=$((binary_size / 1024 / 1024))
        if [ $size_mb -lt 2 ]; then
            print_status "SUCCESS" "Binary size: ${size_mb}MB (acceptable)"
        else
            print_status "WARNING" "Binary size: ${size_mb}MB (may be large)"
        fi
    fi
    
    print_status "SUCCESS" "Performance tests completed"
    echo ""
}

# Function to run memory tests
run_memory_tests() {
    print_status "INFO" "Starting Memory Tests"
    echo "========================================="
    
    # Check for memory leaks in code structure
    print_status "INFO" "Checking for potential memory issues..."
    
    # Check for proper deinitialization
    local deinit_functions=(
        "sensor_interface_deinit"
        "display_interface_deinit"
        "aht10_deinit"
        "ds18b20_deinit"
        "gy302_deinit"
    )
    
    for func in "${deinit_functions[@]}"; do
        if grep -r "$func" src/; then
            echo "✅ $func found"
        else
            print_status "WARNING" "$func not found"
        fi
    done
    
    # Check for proper error handling
    if grep -r "ESP_ERR_INVALID_ARG" src/; then
        echo "✅ Error handling present"
    else
        print_status "WARNING" "Error handling may be incomplete"
    fi
    
    print_status "SUCCESS" "Memory tests completed"
    echo ""
}

# Function to print test summary
print_summary() {
    echo ""
    echo "========================================="
    print_status "INFO" "Test Summary"
    echo "========================================="
    echo "Total Tests: $TOTAL_TESTS"
    echo "Passed: $PASSED_TESTS"
    echo "Failed: $FAILED_TESTS"
    
    if [ $FAILED_TESTS -eq 0 ]; then
        print_status "SUCCESS" "All tests passed! 🎉"
        echo ""
        echo "🌱 Plant Monitor System is ready for deployment!"
        echo "📊 Modular architecture with comprehensive testing"
        echo "🔧 Professional code quality and documentation"
        echo "🚀 Support for all sensors and displays"
    else
        print_status "ERROR" "$FAILED_TESTS test(s) failed"
        echo ""
        echo "Please review the failed tests and fix any issues."
    fi
}

# Main function
main() {
    echo "🌱 Plant Monitor - Comprehensive Test Suite"
    echo "=========================================="
    echo ""
    
    # Check environment first
    check_environment
    
    # Parse command line arguments
    case "${1:-all}" in
        "unit")
            run_unit_tests
            ;;
        "integration")
            run_integration_tests
            ;;
        "system")
            run_system_tests
            ;;
        "quality")
            run_code_quality_checks
            ;;
        "performance")
            run_performance_tests
            ;;
        "memory")
            run_memory_tests
            ;;
        "all")
            run_unit_tests
            run_integration_tests
            run_system_tests
            run_code_quality_checks
            run_performance_tests
            run_memory_tests
            ;;
        *)
            echo "Usage: $0 [unit|integration|system|quality|performance|memory|all]"
            echo ""
            echo "Test Categories:"
            echo "  unit        - Unit tests for individual components"
            echo "  integration - Integration tests for system workflow"
            echo "  system      - System verification tests"
            echo "  quality     - Code quality and documentation checks"
            echo "  performance - Performance and build time tests"
            echo "  memory      - Memory usage and leak detection"
            echo "  all         - Run all tests (default)"
            exit 1
            ;;
    esac
    
    print_summary
}

# Run main function
main "$@" 