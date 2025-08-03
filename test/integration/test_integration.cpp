/**
 * @file test_integration.cpp
 * @brief Integration tests for Plant Monitor System
 * 
 * This file contains integration tests that verify the complete system
 * workflow, including sensor initialization, data flow, and system
 * behavior under various conditions.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#include <gtest/gtest.h>
#include "plant_monitor.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>

// ============================================================================
// INTEGRATION TEST FIXTURES
// ============================================================================

/**
 * @brief Integration test fixture
 * 
 * This fixture provides setup for integration tests that simulate
 * real-world usage scenarios.
 */
class PlantMonitorIntegrationTest : public ::testing::Test 
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
        
        // Configure for integration testing
        config.enable_display = true;
        config.enable_wifi = false; // Disable WiFi for integration tests
        
        // Initialize system
        ret = plant_monitor_init(&config);
        ASSERT_EQ(ret, ESP_OK);
    }
    
    void TearDown() override 
    {
        plant_monitor_deinit();
    }
};

// ============================================================================
// SYSTEM WORKFLOW TESTS
// ============================================================================

/**
 * @brief Test complete system initialization workflow
 */
TEST_F(PlantMonitorIntegrationTest, SystemInitializationWorkflow) 
{
    // Test system status after initialization
    int sensors_working;
    bool display_working, wifi_connected;
    
    esp_err_t ret = plant_monitor_get_status(&sensors_working, &display_working, &wifi_connected);
    EXPECT_EQ(ret, ESP_OK);
    
    // Verify expected state
    EXPECT_GE(sensors_working, 0);
    EXPECT_LE(sensors_working, 2);
    EXPECT_TRUE(display_working);  // Display enabled in integration test
    EXPECT_FALSE(wifi_connected);  // WiFi disabled in integration test
}

/**
 * @brief Test sensor reading workflow
 */
TEST_F(PlantMonitorIntegrationTest, SensorReadingWorkflow) 
{
    // Read sensors multiple times to test consistency
    for (int i = 0; i < 3; i++) {
        esp_err_t ret = plant_monitor_read_sensors(&sensor_data);
        EXPECT_EQ(ret, ESP_OK);
        
        // Verify data structure integrity
        EXPECT_GE(sensor_data.uptime_seconds, 0);
        EXPECT_GT(sensor_data.timestamp, 0);
        EXPECT_FALSE(sensor_data.wifi_connected);
        EXPECT_FALSE(sensor_data.data_sent);
        
        // Small delay between readings
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief Test health calculation workflow
 */
TEST_F(PlantMonitorIntegrationTest, HealthCalculationWorkflow) 
{
    // Read sensors first
    esp_err_t ret = plant_monitor_read_sensors(&sensor_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // Calculate health
    ret = plant_monitor_calculate_health(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // Verify health data integrity
    EXPECT_NE(health_data.health_text, nullptr);
    EXPECT_NE(health_data.emoji, nullptr);
    EXPECT_NE(health_data.recommendation, nullptr);
    EXPECT_GE(health_data.health_score, 0.0f);
    EXPECT_LE(health_data.health_score, 100.0f);
    
    // Verify health level is valid
    EXPECT_GE(health_data.health_level, PLANT_HEALTH_EXCELLENT);
    EXPECT_LE(health_data.health_level, PLANT_HEALTH_CRITICAL);
}

/**
 * @brief Test display update workflow
 */
TEST_F(PlantMonitorIntegrationTest, DisplayUpdateWorkflow) 
{
    // Read sensors and calculate health
    esp_err_t ret = plant_monitor_read_sensors(&sensor_data);
    EXPECT_EQ(ret, ESP_OK);
    
    ret = plant_monitor_calculate_health(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // Update display
    ret = plant_monitor_update_display(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
}

/**
 * @brief Test data transmission workflow
 */
TEST_F(PlantMonitorIntegrationTest, DataTransmissionWorkflow) 
{
    // Read sensors and calculate health
    esp_err_t ret = plant_monitor_read_sensors(&sensor_data);
    EXPECT_EQ(ret, ESP_OK);
    
    ret = plant_monitor_calculate_health(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // Transmit data (WiFi disabled, so this should succeed gracefully)
    ret = plant_monitor_transmit_data(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
}

// ============================================================================
// END-TO-END TESTS
// ============================================================================

/**
 * @brief Test complete monitoring cycle
 */
TEST_F(PlantMonitorIntegrationTest, CompleteMonitoringCycle) 
{
    // Simulate a complete monitoring cycle
    esp_err_t ret;
    
    // 1. Read sensors
    ret = plant_monitor_read_sensors(&sensor_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // 2. Calculate health
    ret = plant_monitor_calculate_health(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // 3. Update display
    ret = plant_monitor_update_display(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // 4. Transmit data
    ret = plant_monitor_transmit_data(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // 5. Verify data consistency
    EXPECT_GE(sensor_data.uptime_seconds, 0);
    EXPECT_GT(sensor_data.timestamp, 0);
    EXPECT_NE(health_data.health_text, nullptr);
    EXPECT_NE(health_data.emoji, nullptr);
    EXPECT_NE(health_data.recommendation, nullptr);
}

/**
 * @brief Test multiple monitoring cycles
 */
TEST_F(PlantMonitorIntegrationTest, MultipleMonitoringCycles) 
{
    // Run multiple monitoring cycles to test system stability
    for (int cycle = 0; cycle < 5; cycle++) {
        esp_err_t ret;
        
        // Complete cycle
        ret = plant_monitor_read_sensors(&sensor_data);
        EXPECT_EQ(ret, ESP_OK);
        
        ret = plant_monitor_calculate_health(&sensor_data, &health_data);
        EXPECT_EQ(ret, ESP_OK);
        
        ret = plant_monitor_update_display(&sensor_data, &health_data);
        EXPECT_EQ(ret, ESP_OK);
        
        ret = plant_monitor_transmit_data(&sensor_data, &health_data);
        EXPECT_EQ(ret, ESP_OK);
        
        // Verify uptime increases
        EXPECT_GE(sensor_data.uptime_seconds, cycle);
        
        // Small delay between cycles
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// ============================================================================
// ERROR HANDLING TESTS
// ============================================================================

/**
 * @brief Test system behavior with invalid sensor data
 */
TEST_F(PlantMonitorIntegrationTest, InvalidSensorDataHandling) 
{
    // Create invalid sensor data
    plant_monitor_data_t invalid_data = {0};
    invalid_data.temperature_avg = -100.0f;  // Invalid temperature
    invalid_data.humidity_avg = 150.0f;     // Invalid humidity
    
    // System should handle invalid data gracefully
    esp_err_t ret = plant_monitor_calculate_health(&invalid_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // Should result in critical health
    EXPECT_EQ(health_data.health_level, PLANT_HEALTH_CRITICAL);
}

/**
 * @brief Test system behavior with missing sensors
 */
TEST_F(PlantMonitorIntegrationTest, MissingSensorsHandling) 
{
    // Read sensors (may not be connected)
    esp_err_t ret = plant_monitor_read_sensors(&sensor_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // System should handle missing sensors gracefully
    ret = plant_monitor_calculate_health(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // Should still provide valid health assessment
    EXPECT_NE(health_data.health_text, nullptr);
    EXPECT_NE(health_data.emoji, nullptr);
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

/**
 * @brief Test system performance under load
 */
TEST_F(PlantMonitorIntegrationTest, PerformanceUnderLoad) 
{
    const int iterations = 10;
    uint32_t start_time = esp_timer_get_time();
    
    // Run multiple iterations quickly
    for (int i = 0; i < iterations; i++) {
        esp_err_t ret = plant_monitor_read_sensors(&sensor_data);
        EXPECT_EQ(ret, ESP_OK);
        
        ret = plant_monitor_calculate_health(&sensor_data, &health_data);
        EXPECT_EQ(ret, ESP_OK);
        
        ret = plant_monitor_update_display(&sensor_data, &health_data);
        EXPECT_EQ(ret, ESP_OK);
    }
    
    uint32_t end_time = esp_timer_get_time();
    uint32_t total_time = end_time - start_time;
    
    // Verify reasonable performance (should complete quickly)
    EXPECT_LT(total_time, 1000000); // Less than 1 second for 10 iterations
}

// ============================================================================
// CONFIGURATION TESTS
// ============================================================================

/**
 * @brief Test system with different configurations
 */
TEST_F(PlantMonitorIntegrationTest, DifferentConfigurations) 
{
    // Test with display disabled
    plant_monitor_config_t test_config;
    esp_err_t ret = plant_monitor_get_default_config(&test_config);
    EXPECT_EQ(ret, ESP_OK);
    
    test_config.enable_display = false;
    test_config.enable_wifi = false;
    
    ret = plant_monitor_init(&test_config);
    EXPECT_EQ(ret, ESP_OK);
    
    // Test sensor reading
    ret = plant_monitor_read_sensors(&sensor_data);
    EXPECT_EQ(ret, ESP_OK);
    
    // Test display update (should succeed even when disabled)
    ret = plant_monitor_update_display(&sensor_data, &health_data);
    EXPECT_EQ(ret, ESP_OK);
    
    plant_monitor_deinit();
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

/**
 * @brief Main function for running integration tests
 */
extern "C" void app_main(void) 
{
    // Initialize Google Test
    ::testing::InitGoogleTest();
    
    // Run all integration tests
    int result = RUN_ALL_TESTS();
    
    // Exit with test result
    exit(result);
} 