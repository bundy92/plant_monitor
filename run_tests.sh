#!/bin/bash

echo "🧪 ESP32 Plant Monitor - Test Runner"
echo "====================================="

# Function to run unit tests
run_unit_tests() {
    echo "📋 Running unit tests..."
    if [ -f "test/unit/test_aht10.cpp" ]; then
        echo "✅ Unit test file found"
        # TODO: Add actual unit test execution
        echo "⚠️  Unit tests not yet implemented (requires Google Test framework)"
    else
        echo "❌ Unit test file not found"
    fi
}

# Function to run integration tests
run_integration_tests() {
    echo "🔗 Running integration tests..."
    
    # Test 1: Build verification
    echo "📦 Testing build process..."
    if pio run --target build > /dev/null 2>&1; then
        echo "✅ Build test passed"
    else
        echo "❌ Build test failed"
        return 1
    fi
    
    # Test 2: Code style check
    echo "🎨 Testing code style..."
    if command -v clang-format > /dev/null 2>&1; then
        echo "✅ Code style check available"
    else
        echo "⚠️  clang-format not found, skipping style check"
    fi
    
    # Test 3: Memory check
    echo "🧠 Testing memory usage..."
    if pio run --target size > /dev/null 2>&1; then
        echo "✅ Memory usage check passed"
    else
        echo "❌ Memory usage check failed"
    fi
}

# Function to run sensor tests
run_sensor_tests() {
    echo "🌡️  Running sensor tests..."
    
    echo "📡 Testing AHT10 sensor library..."
    if [ -f "src/sensors/aht10.h" ] && [ -f "src/sensors/aht10.c" ]; then
        echo "✅ AHT10 sensor library found"
    else
        echo "❌ AHT10 sensor library missing"
        return 1
    fi
    
    echo "🔧 Testing sensor initialization..."
    # This would require actual hardware testing
    echo "⚠️  Hardware sensor tests require connected ESP32"
}

# Function to run project verification
run_project_verification() {
    echo "🔍 Running project verification..."
    
    # Check essential files
    essential_files=(
        "src/main.cpp"
        "src/sensors/aht10.h"
        "src/sensors/aht10.c"
        "platformio.ini"
        "config.h"
        "README.md"
    )
    
    for file in "${essential_files[@]}"; do
        if [ -f "$file" ]; then
            echo "✅ $file"
        else
            echo "❌ $file (missing)"
        fi
    done
    
    # Check project structure
    echo "📁 Checking project structure..."
    if [ -d "src" ] && [ -d "test" ]; then
        echo "✅ Project structure looks good"
    else
        echo "⚠️  Some directories missing"
    fi
}

# Main test execution
main() {
    case "$1" in
        "unit")
            run_unit_tests
            ;;
        "integration")
            run_integration_tests
            ;;
        "sensor")
            run_sensor_tests
            ;;
        "verify")
            run_project_verification
            ;;
        "all")
            echo "🚀 Running all tests..."
            run_project_verification
            run_unit_tests
            run_integration_tests
            run_sensor_tests
            ;;
        *)
            echo "Usage: $0 {unit|integration|sensor|verify|all}"
            echo ""
            echo "Test types:"
            echo "  unit        - Run unit tests"
            echo "  integration - Run integration tests"
            echo "  sensor      - Run sensor-specific tests"
            echo "  verify      - Verify project structure"
            echo "  all         - Run all tests"
            exit 1
            ;;
    esac
    
    echo ""
    echo "✨ Test execution completed!"
}

# Run main function with arguments
main "$@" 