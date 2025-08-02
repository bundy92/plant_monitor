# Testing Guide - ESP32 Plant Monitor

## 🧪 Overview

This project implements a comprehensive testing framework with multiple layers of testing to ensure code quality, reliability, and maintainability.

## 📋 Testing Layers

### 1. **Unit Tests** (`test/unit/`)
- **Purpose**: Test individual functions and classes in isolation
- **Framework**: Google Test (gtest/gmock)
- **Coverage**: AHT10 sensor library, data validation, error handling
- **Location**: `test/unit/test_aht10.cpp`

### 2. **Integration Tests** (`test/integration/`)
- **Purpose**: Test component interactions and system behavior
- **Framework**: Custom test framework
- **Coverage**: I2C communication, sensor initialization, data flow
- **Location**: `test/integration/`

### 3. **Hardware Tests** (`test/hardware/`)
- **Purpose**: Test with actual hardware components
- **Framework**: ESP-IDF test framework
- **Coverage**: Real sensor communication, timing, accuracy
- **Location**: `test/hardware/`

### 4. **System Tests** (`test/system/`)
- **Purpose**: End-to-end system validation
- **Framework**: Custom test scripts
- **Coverage**: Complete data pipeline, WiFi, server communication
- **Location**: `test/system/`

## 🚀 Running Tests

### Quick Test Commands

```bash
# Run all tests
./run_tests.sh all

# Run specific test types
./run_tests.sh unit
./run_tests.sh integration
./run_tests.sh sensor
./run_tests.sh verify

# Run hardware tests
pio run --target upload --environment test_hardware

# Run unit tests
pio test --environment test_unit

# Run integration tests
pio test --environment test_integration
```

### Test Scripts

| Script | Purpose | Usage |
|--------|---------|-------|
| `run_tests.sh` | Main test runner | `./run_tests.sh {type}` |
| `test_aht10.sh` | AHT10 sensor test | `./test_aht10.sh` |
| `test_build.sh` | Build verification | `./test_build.sh` |
| `verify_project.sh` | Project structure check | `./verify_project.sh` |

## 📊 Test Coverage

### Unit Tests Coverage

```cpp
// Example unit test structure
TEST_F(AHT10Test, InitializationTest) {
    esp_err_t ret = sensor1->init();
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_TRUE(sensor1->is_initialized());
}

TEST_F(AHT10Test, TemperatureHumidityReadTest) {
    esp_err_t ret = sensor1->init();
    ASSERT_EQ(ret, ESP_OK);
    
    float temperature, humidity;
    ret = sensor1->read_temperature_humidity(&temperature, &humidity);
    
    if (ret == ESP_OK) {
        EXPECT_GE(temperature, -40.0f);
        EXPECT_LE(temperature, 85.0f);
        EXPECT_GE(humidity, 0.0f);
        EXPECT_LE(humidity, 100.0f);
    }
}
```

### Integration Tests Coverage

- **I2C Communication**: Test I2C bus initialization and device detection
- **Sensor Initialization**: Test sensor configuration and setup
- **Data Validation**: Test sensor data parsing and validation
- **Error Handling**: Test error conditions and recovery

### Hardware Tests Coverage

- **Sensor Accuracy**: Compare readings with reference sensors
- **Timing Tests**: Verify measurement timing and intervals
- **Power Tests**: Test sensor behavior under different power conditions
- **Temperature Tests**: Test sensor response to temperature changes

## 🔧 Test Configuration

### PlatformIO Test Environments

```ini
[env:test_unit]
platform = espressif32
board = esp32-c6-devkitc-1
framework = espidf
build_flags = -DTESTING=1 -DUNIT_TESTS=1
lib_deps = 
    google/googletest@^1.14.0

[env:test_integration]
platform = espressif32
board = esp32-c6-devkitc-1
framework = espidf
build_flags = -DTESTING=1 -DINTEGRATION_TESTS=1

[env:test_hardware]
platform = espressif32
board = esp32-c6-devkitc-1
framework = espidf
build_flags = -DTESTING=1 -DHARDWARE_TESTS=1
```

### Test Data Files

```
test/
├── data/
│   ├── aht10_sample_data.bin    # Sample sensor data
│   ├── i2c_scan_results.json    # I2C scan results
│   └── test_config.json         # Test configuration
├── mocks/
│   ├── i2c_mock.h              # I2C mock functions
│   └── sensor_mock.h            # Sensor mock functions
└── fixtures/
    ├── test_sensors.h           # Test sensor fixtures
    └── test_data.h              # Test data fixtures
```

## 📈 Test Metrics

### Coverage Metrics

- **Code Coverage**: Target >90% for critical functions
- **Branch Coverage**: Target >85% for error handling
- **Function Coverage**: Target 100% for sensor library

### Performance Metrics

- **Test Execution Time**: <30 seconds for unit tests
- **Memory Usage**: <50KB for test framework
- **Build Time**: <2 minutes for full test suite

### Quality Metrics

- **Test Reliability**: >95% pass rate
- **False Positives**: <5% of test failures
- **Test Maintenance**: <10% of development time

## 🛠️ Writing Tests

### Unit Test Template

```cpp
#include <gtest/gtest.h>
#include "sensors/aht10.h"

class AHT10Test : public ::testing::Test {
protected:
    aht10_config_t config;
    aht10_data_t data;
    
    void SetUp() override {
        // Initialize test fixtures
        aht10_get_default_config(&config, AHT10_ADDR_1);
    }
    
    void TearDown() override {
        // Clean up test fixtures
        aht10_deinit(&config);
    }
};

TEST_F(AHT10Test, TestName) {
    // Arrange
    // Act
    // Assert
}
```

### Integration Test Template

```cpp
#include "unity.h"
#include "sensors/aht10.h"

void setUp(void) {
    // Setup test environment
}

void tearDown(void) {
    // Clean up test environment
}

void test_sensor_initialization(void) {
    // Test sensor initialization
    TEST_ASSERT_EQUAL(ESP_OK, aht10_init(&config));
}
```

## 🔍 Debugging Tests

### Common Test Issues

1. **Build Failures**
   ```bash
   # Check build configuration
   pio run --target clean
   pio run --target build
   ```

2. **Test Failures**
   ```bash
   # Run tests with verbose output
   pio test --verbose
   
   # Run specific test
   pio test --filter test_name
   ```

3. **Hardware Issues**
   ```bash
   # Check device connection
   pio device list
   
   # Monitor serial output
   pio device monitor
   ```

### Test Logging

```cpp
// Enable test logging
#define TEST_LOG_LEVEL ESP_LOG_VERBOSE

// In test functions
ESP_LOGI(TAG, "Test step: %s", "description");
ESP_LOGD(TAG, "Test data: %f", test_value);
```

## 📋 Test Checklist

### Before Running Tests

- [ ] Hardware connected properly
- [ ] Sensors powered and functional
- [ ] WiFi network available (for integration tests)
- [ ] Test environment configured
- [ ] Dependencies installed

### Test Execution Checklist

- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Hardware tests pass
- [ ] System tests pass
- [ ] Code coverage meets targets
- [ ] Performance metrics met

### Post-Test Validation

- [ ] All tests pass
- [ ] No memory leaks
- [ ] Code coverage report generated
- [ ] Test results documented
- [ ] Issues logged and tracked

## 🎯 Best Practices

### Test Design

1. **Arrange-Act-Assert**: Structure tests clearly
2. **Single Responsibility**: Each test tests one thing
3. **Descriptive Names**: Use clear test names
4. **Independent Tests**: Tests should not depend on each other
5. **Fast Execution**: Keep tests fast and efficient

### Test Maintenance

1. **Regular Updates**: Update tests when code changes
2. **Version Control**: Track test changes in git
3. **Documentation**: Document test purpose and setup
4. **Review Process**: Review test code with application code

### Continuous Integration

1. **Automated Testing**: Run tests on every commit
2. **Coverage Reports**: Generate coverage reports
3. **Quality Gates**: Block merges on test failures
4. **Performance Monitoring**: Track test performance over time

---

**Happy Testing! 🧪✨** 