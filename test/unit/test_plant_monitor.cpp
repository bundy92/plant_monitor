/**
 * @file test_plant_monitor.cpp
 * @brief Unit tests for Plant Monitor System
 * 
 * This file contains comprehensive unit tests for the plant monitoring system
 * using Google Test framework. It tests all major functionality including
 * sensor reading, health calculation, and configuration management.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "plant_monitor.h"
#include <string.h>

// ============================================================================
// TEST FIXTURES
// ============================================================================

/**
 * @brief Test fixture for plant monitor tests
 * 
 * This fixture provides common setup and teardown for plant monitor tests,
 * including configuration initialization and cleanup.
 */
class PlantMonitorTest : public ::testing::Test 
{
protected:
    plant_monitor_config_t config;
    plant_monitor_data_t sensor_data;
    plant_health_t health_data;
    
    void SetUp() override 
    {
        // Get default configuration
        esp_err_t ret = plant_monitor_get_default_config(&config);
        ASSERT_EQ(ret, ESP_OK);
        
        // Disable WiFi and display for unit tests
        config.enable_wifi = false;
        config.enable_display = false;
        
        // Initialize system
        ret = plant_monitor_init(&config);
        ASSERT_EQ(ret, ESP_OK);
    }
    
    void TearDown() override 
    {
        // Clean up
        plant_monitor_deinit();
    }
};

// ============================================================================
// CONFIGURATION TESTS
// ============================================================================

/**
 * @brief Test default configuration
 */
TEST_F(PlantMonitorTest, DefaultConfiguration) 
{
    plant_monitor_config_t test_config;
    esp_err_t ret = plant_monitor_get_default_config(&test_config);
    
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(test_config.sda_pin, PLANT_MONITOR_DEFAULT_SDA_PIN);
    EXPECT_EQ(test_config.scl_pin, PLANT_MONITOR_DEFAULT_SCL_PIN);
    EXPECT_EQ(test_config.i2c_freq_hz, PLANT_MONITOR_DEFAULT_I2C_FREQ_HZ);
    EXPECT_EQ(test_config.aht10_addr_1, PLANT_MONITOR_AHT10_ADDR_1);
    EXPECT_EQ(test_config.aht10_addr_2, PLANT_MONITOR_AHT10_ADDR_2);
    EXPECT_FALSE(test_config.enable_dht_sensors);
    EXPECT_FALSE(test_config.enable_display);
    EXPECT_FALSE(test_config.enable_wifi);
}

/**
 * @brief Test configuration validation
 */
TEST_F(PlantMonitorTest, ConfigurationValidation) 
{
    plant_monitor_config_t invalid_config;
    memset(&invalid_config, 0, sizeof(invalid_config));
    
    // Test with NULL config
    esp_err_t ret = plant_monitor_init(NULL);
    EXPECT_NE(ret, ESP_OK);
}

// ============================================================================
// SENSOR READING TESTS
// ============================================================================

/**
 * @brief Test sensor reading with valid data
 */
TEST_F(PlantMonitorTest, ReadSensorsValid) 
{
    esp_err_t ret = plant_monitor_read_sensors(&sensor_data);
    
    // Should succeed even if sensors are not connected
    EXPECT_EQ(ret, ESP_OK);
    
    // Check that data structure is properly initialized
    EXPECT_GE(sensor_data.uptime_seconds, 0);
    EXPECT_FALSE(sensor_data.wifi_connected); // WiFi disabled in test
    EXPECT_FALSE(sensor_data.data_sent);
    EXPECT_GT(sensor_data.timestamp, 0);
}

/**
 * @brief Test sensor reading with NULL data
 */
TEST_F(PlantMonitorTest, ReadSensorsNullData) 
{
    esp_err_t ret = plant_monitor_read_sensors(NULL);
    EXPECT_NE(ret, ESP_OK);
}

// ============================================================================
// HEALTH CALCULATION TESTS
// ============================================================================

/**
 * @brief Test health calculation with excellent conditions
 */
TEST_F(PlantMonitorTest, HealthCalculationExcellent) 
{
    // Set up excellent conditions
    sensor_data.temperature_avg = 23.0f;  // Optimal range
    sensor_data.humidity_avg = 55.0f;    // Optimal range
    
    esp_err_t ret = plant_monitor_calculate_health(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(health_data.health_level, PLANT_HEALTH_EXCELLENT);
    EXPECT_STREQ(health_data.health_text, "Excellent");
    EXPECT_STREQ(health_data.emoji, "😊");
    EXPECT_GE(health_data.health_score, 90.0f);
}

/**
 * @brief Test health calculation with good conditions
 */
TEST_F(PlantMonitorTest, HealthCalculationGood) 
{
    // Set up good conditions
    sensor_data.temperature_avg = 25.0f;  // Optimal range
    sensor_data.humidity_avg = 35.0f;    // Acceptable range
    
    esp_err_t ret = plant_monitor_calculate_health(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(health_data.health_level, PLANT_HEALTH_GOOD);
    EXPECT_STREQ(health_data.health_text, "Good");
    EXPECT_STREQ(health_data.emoji, "🙂");
    EXPECT_GE(health_data.health_score, 70.0f);
    EXPECT_LT(health_data.health_score, 90.0f);
}

/**
 * @brief Test health calculation with fair conditions
 */
TEST_F(PlantMonitorTest, HealthCalculationFair) 
{
    // Set up fair conditions
    sensor_data.temperature_avg = 30.0f;  // Acceptable range
    sensor_data.humidity_avg = 25.0f;    // Below minimum
    
    esp_err_t ret = plant_monitor_calculate_health(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(health_data.health_level, PLANT_HEALTH_FAIR);
    EXPECT_STREQ(health_data.health_text, "Fair");
    EXPECT_STREQ(health_data.emoji, "😐");
    EXPECT_GE(health_data.health_score, 50.0f);
    EXPECT_LT(health_data.health_score, 70.0f);
}

/**
 * @brief Test health calculation with poor conditions
 */
TEST_F(PlantMonitorTest, HealthCalculationPoor) 
{
    // Set up poor conditions
    sensor_data.temperature_avg = 8.0f;   // Below minimum
    sensor_data.humidity_avg = 25.0f;    // Below minimum
    
    esp_err_t ret = plant_monitor_calculate_health(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(health_data.health_level, PLANT_HEALTH_POOR);
    EXPECT_STREQ(health_data.health_text, "Poor");
    EXPECT_STREQ(health_data.emoji, "😟");
    EXPECT_GE(health_data.health_score, 30.0f);
    EXPECT_LT(health_data.health_score, 50.0f);
}

/**
 * @brief Test health calculation with critical conditions
 */
TEST_F(PlantMonitorTest, HealthCalculationCritical) 
{
    // Set up critical conditions
    sensor_data.temperature_avg = 5.0f;   // Way below minimum
    sensor_data.humidity_avg = 10.0f;    // Way below minimum
    
    esp_err_t ret = plant_monitor_calculate_health(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(health_data.health_level, PLANT_HEALTH_CRITICAL);
    EXPECT_STREQ(health_data.health_text, "Critical");
    EXPECT_STREQ(health_data.emoji, "😱");
    EXPECT_LT(health_data.health_score, 30.0f);
}

/**
 * @brief Test health calculation with NULL data
 */
TEST_F(PlantMonitorTest, HealthCalculationNullData) 
{
    esp_err_t ret = plant_monitor_calculate_health(NULL, &health_data);
    EXPECT_NE(ret, ESP_OK);
    
    ret = plant_monitor_calculate_health(&sensor_data, NULL);
    EXPECT_NE(ret, ESP_OK);
}

// ============================================================================
// DISPLAY TESTS
// ============================================================================

/**
 * @brief Test display update with enabled display
 */
TEST_F(PlantMonitorTest, DisplayUpdateEnabled) 
{
    // Enable display
    config.enable_display = true;
    plant_monitor_init(&config);
    
    // Set up test data
    sensor_data.temperature_avg = 23.0f;
    sensor_data.humidity_avg = 55.0f;
    sensor_data.soil_moisture = 512;
    sensor_data.light_level = 2048;
    sensor_data.uptime_seconds = 3600;
    
    plant_monitor_calculate_health(&sensor_data, &health_data);
    
    esp_err_t ret = plant_monitor_update_display(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
}

/**
 * @brief Test display update with disabled display
 */
TEST_F(PlantMonitorTest, DisplayUpdateDisabled) 
{
    // Disable display
    config.enable_display = false;
    plant_monitor_init(&config);
    
    esp_err_t ret = plant_monitor_update_display(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK); // Should succeed even when disabled
}

// ============================================================================
// DATA TRANSMISSION TESTS
// ============================================================================

/**
 * @brief Test data transmission with disabled WiFi
 */
TEST_F(PlantMonitorTest, DataTransmissionDisabled) 
{
    // WiFi is disabled in test setup
    esp_err_t ret = plant_monitor_transmit_data(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK); // Should succeed even when WiFi is disabled
}

// ============================================================================
// SYSTEM STATUS TESTS
// ============================================================================

/**
 * @brief Test system status retrieval
 */
TEST_F(PlantMonitorTest, SystemStatus) 
{
    int sensors_working;
    bool display_working, wifi_connected;
    
    esp_err_t ret = plant_monitor_get_status(&sensors_working, &display_working, &wifi_connected);
    EXPECT_EQ(ret, ESP_OK);
    
    EXPECT_GE(sensors_working, 0);
    EXPECT_LE(sensors_working, 2);
    EXPECT_FALSE(display_working); // Display disabled in test
    EXPECT_FALSE(wifi_connected);  // WiFi disabled in test
}

/**
 * @brief Test system status with NULL parameters
 */
TEST_F(PlantMonitorTest, SystemStatusNullParams) 
{
    esp_err_t ret = plant_monitor_get_status(NULL, NULL, NULL);
    EXPECT_NE(ret, ESP_OK);
}

// ============================================================================
// I2C SCANNING TESTS
// ============================================================================

/**
 * @brief Test I2C device scanning
 */
TEST_F(PlantMonitorTest, I2CDeviceScanning) 
{
    esp_err_t ret = plant_monitor_scan_i2c_devices();
    // Should succeed even if no devices are found
    EXPECT_EQ(ret, ESP_OK);
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

/**
 * @brief Test complete monitoring cycle
 */
TEST_F(PlantMonitorTest, CompleteMonitoringCycle) 
{
    // Read sensors
    esp_err_t ret = plant_monitor_read_sensors(&sensor_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // Calculate health
    ret = plant_monitor_calculate_health(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // Update display
    ret = plant_monitor_update_display(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // Transmit data
    ret = plant_monitor_transmit_data(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // Verify data integrity
    EXPECT_GE(sensor_data.uptime_seconds, 0);
    EXPECT_GT(sensor_data.timestamp, 0);
    EXPECT_NE(health_data.health_text, nullptr);
    EXPECT_NE(health_data.emoji, nullptr);
    EXPECT_NE(health_data.recommendation, nullptr);
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

/**
 * @brief Test with extreme temperature values
 */
TEST_F(PlantMonitorTest, ExtremeTemperatureValues) 
{
    sensor_data.temperature_avg = -50.0f;  // Extreme low
    esp_err_t ret = plant_monitor_calculate_health(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(health_data.health_level, PLANT_HEALTH_CRITICAL);
    
    sensor_data.temperature_avg = 100.0f;  // Extreme high
    ret = plant_monitor_calculate_health(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(health_data.health_level, PLANT_HEALTH_CRITICAL);
}

/**
 * @brief Test with extreme humidity values
 */
TEST_F(PlantMonitorTest, ExtremeHumidityValues) 
{
    sensor_data.humidity_avg = -10.0f;  // Invalid low
    esp_err_t ret = plant_monitor_calculate_health(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(health_data.health_level, PLANT_HEALTH_CRITICAL);
    
    sensor_data.humidity_avg = 110.0f;  // Invalid high
    ret = plant_monitor_calculate_health(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(health_data.health_level, PLANT_HEALTH_CRITICAL);
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

/**
 * @brief Main function for running tests
 */
extern "C" void app_main(void) 
{
    // Initialize Google Test
    ::testing::InitGoogleTest();
    
    // Run all tests
    int result = RUN_ALL_TESTS();
    
    // Exit with test result
    exit(result);
} 