#!/bin/bash

# Plant Monitor - Comprehensive Test Runner
# =======================================
# This script runs all types of tests for the plant monitoring system
# including unit tests, integration tests, and system verification.

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test results
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    
    case $status in
        "PASS")
            echo -e "${GREEN}✅ PASS${NC}: $message"
            ;;
        "FAIL")
            echo -e "${RED}❌ FAIL${NC}: $message"
            ;;
        "INFO")
            echo -e "${BLUE}ℹ️  INFO${NC}: $message"
            ;;
        "WARN")
            echo -e "${YELLOW}⚠️  WARN${NC}: $message"
            ;;
    esac
}

# Function to run a test and capture results
run_test() {
    local test_name=$1
    local test_command=$2
    
    print_status "INFO" "Running $test_name..."
    
    if eval "$test_command" > /tmp/test_output.log 2>&1; then
        print_status "PASS" "$test_name completed successfully"
        ((PASSED_TESTS++))
    else
        print_status "FAIL" "$test_name failed"
        echo -e "${RED}Test output:${NC}"
        cat /tmp/test_output.log
        ((FAILED_TESTS++))
    fi
    
    ((TOTAL_TESTS++))
    echo ""
}

# Function to check if we're in the right directory
check_environment() {
    if [ ! -f "platformio.ini" ]; then
        print_status "FAIL" "platformio.ini not found. Please run from project root."
        exit 1
    fi
    
    if [ ! -f "src/plant_monitor.h" ]; then
        print_status "FAIL" "plant_monitor.h not found."
        exit 1
    fi
    
    if [ ! -f "src/plant_monitor.c" ]; then
        print_status "FAIL" "plant_monitor.c not found."
        exit 1
    fi
    
    print_status "INFO" "Environment check passed"
}

# Function to run unit tests
run_unit_tests() {
    print_status "INFO" "Starting Unit Tests"
    echo "=================================="
    
    # Check if unit test file exists
    if [ ! -f "test/unit/test_plant_monitor.cpp" ]; then
        print_status "WARN" "Unit test file not found, skipping unit tests"
        return
    fi
    
    # Copy unit test to main.cpp
    cp test/unit/test_plant_monitor.cpp src/main.cpp
    
    # Build and run unit tests
    run_test "Unit Tests" "pio run --target upload && pio device monitor --timeout 30"
    
    # Restore original main.cpp
    cp src/main_example.cpp src/main.cpp
}

# Function to run integration tests
run_integration_tests() {
    print_status "INFO" "Starting Integration Tests"
    echo "========================================"
    
    # Check if integration test file exists
    if [ ! -f "test/integration/test_integration.cpp" ]; then
        print_status "WARN" "Integration test file not found, skipping integration tests"
        return
    fi
    
    # Copy integration test to main.cpp
    cp test/integration/test_integration.cpp src/main.cpp
    
    # Build and run integration tests
    run_test "Integration Tests" "pio run --target upload && pio device monitor --timeout 30"
    
    # Restore original main.cpp
    cp src/main_example.cpp src/main.cpp
}

# Function to run system verification tests
run_system_tests() {
    print_status "INFO" "Starting System Verification Tests"
    echo "================================================"
    
    # Copy example main to main.cpp
    cp src/main_example.cpp src/main.cpp
    
    # Test build
    run_test "Build Test" "pio run"
    
    # Test upload (if device connected)
    if pio device list | grep -q "tty"; then
        run_test "Upload Test" "pio run --target upload"
    else
        print_status "WARN" "No device connected, skipping upload test"
    fi
    
    # Test I2C scanning
    run_test "I2C Scan Test" "pio run --target upload && pio device monitor --timeout 10"
}

# Function to run code quality checks
run_code_quality_checks() {
    print_status "INFO" "Starting Code Quality Checks"
    echo "========================================="
    
    # Check for required files
    local required_files=(
        "src/plant_monitor.h"
        "src/plant_monitor.c"
        "src/main_example.cpp"
        "platformio.ini"
        "config.h"
    )
    
    for file in "${required_files[@]}"; do
        if [ -f "$file" ]; then
            print_status "PASS" "Found $file"
            ((PASSED_TESTS++))
        else
            print_status "FAIL" "Missing $file"
            ((FAILED_TESTS++))
        fi
        ((TOTAL_TESTS++))
    done
    
    # Check for documentation
    if grep -q "@brief" src/plant_monitor.h; then
        print_status "PASS" "Header documentation found"
        ((PASSED_TESTS++))
    else
        print_status "FAIL" "Header documentation missing"
        ((FAILED_TESTS++))
    fi
    ((TOTAL_TESTS++))
    
    if grep -q "@brief" src/plant_monitor.c; then
        print_status "PASS" "Implementation documentation found"
        ((PASSED_TESTS++))
    else
        print_status "FAIL" "Implementation documentation missing"
        ((FAILED_TESTS++))
    fi
    ((TOTAL_TESTS++))
}

# Function to run performance tests
run_performance_tests() {
    print_status "INFO" "Starting Performance Tests"
    echo "====================================="
    
    # Test build time
    local start_time=$(date +%s)
    if pio run --target clean > /dev/null 2>&1 && pio run > /dev/null 2>&1; then
        local end_time=$(date +%s)
        local build_time=$((end_time - start_time))
        
        if [ $build_time -lt 60 ]; then
            print_status "PASS" "Build completed in ${build_time}s (under 60s limit)"
            ((PASSED_TESTS++))
        else
            print_status "FAIL" "Build took ${build_time}s (over 60s limit)"
            ((FAILED_TESTS++))
        fi
        ((TOTAL_TESTS++))
    else
        print_status "FAIL" "Build failed during performance test"
        ((FAILED_TESTS++))
        ((TOTAL_TESTS++))
    fi
}

# Function to run memory usage tests
run_memory_tests() {
    print_status "INFO" "Starting Memory Usage Tests"
    echo "======================================="
    
    # Check binary size
    if [ -f ".pio/build/esp32-c6-devkitc-1/firmware.elf" ]; then
        local binary_size=$(stat -c%s .pio/build/esp32-c6-devkitc-1/firmware.elf)
        local max_size=1048576  # 1MB limit
        
        if [ $binary_size -lt $max_size ]; then
            print_status "PASS" "Binary size: ${binary_size} bytes (under 1MB limit)"
            ((PASSED_TESTS++))
        else
            print_status "FAIL" "Binary size: ${binary_size} bytes (over 1MB limit)"
            ((FAILED_TESTS++))
        fi
        ((TOTAL_TESTS++))
    else
        print_status "WARN" "Binary file not found, skipping memory test"
    fi
}

# Function to print test summary
print_summary() {
    echo ""
    echo "========================================"
    echo "           TEST SUMMARY"
    echo "========================================"
    echo "Total Tests: $TOTAL_TESTS"
    echo -e "Passed: ${GREEN}$PASSED_TESTS${NC}"
    echo -e "Failed: ${RED}$FAILED_TESTS${NC}"
    
    if [ $FAILED_TESTS -eq 0 ]; then
        echo -e "${GREEN}🎉 All tests passed!${NC}"
        exit 0
    else
        echo -e "${RED}❌ Some tests failed${NC}"
        exit 1
    fi
}

# Main function
main() {
    echo "🌱 Plant Monitor - Comprehensive Test Suite"
    echo "=========================================="
    echo ""
    
    # Check environment
    check_environment
    
    # Run different types of tests
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
            run_code_quality_checks
            run_performance_tests
            run_memory_tests
            run_system_tests
            run_unit_tests
            run_integration_tests
            ;;
        *)
            echo "Usage: $0 [unit|integration|system|quality|performance|memory|all]"
            echo ""
            echo "Test Types:"
            echo "  unit        - Run unit tests"
            echo "  integration - Run integration tests"
            echo "  system      - Run system verification tests"
            echo "  quality     - Run code quality checks"
            echo "  performance - Run performance tests"
            echo "  memory      - Run memory usage tests"
            echo "  all         - Run all tests (default)"
            exit 1
            ;;
    esac
    
    # Print summary
    print_summary
}

# Run main function
main "$@" 