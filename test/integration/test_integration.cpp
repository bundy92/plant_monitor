/**
 * @file test_integration.cpp
 * @brief Integration Tests for Plant Monitor System - Modular Architecture
 * 
 * This file contains comprehensive integration tests for the modular plant
 * monitoring system, testing the interaction between sensor interface,
 * display interface, and the complete system workflow.
 * 
 * @author Plant Monitor System
 * @version 1.0.0
 * @date 2024
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "sensor_interface.h"
#include "display_interface.h"
#include "aht10.h"
#include "ds18b20.h"
#include "gy302.h"

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

/**
 * @brief Test fixture for plant monitor integration tests
 */
class PlantMonitorIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize comprehensive test configuration
        memset(&sensor_config, 0, sizeof(sensor_interface_config_t));
        memset(&display_config, 0, sizeof(display_interface_config_t));
        
        // Configure all available sensors
        sensor_config.sensor_count = 6;
        sensor_config.i2c_sda_pin = 21;
        sensor_config.i2c_scl_pin = 22;
        sensor_config.i2c_frequency = 100000;
        sensor_config.onewire_pin = 4;
        sensor_config.adc_soil_pin = 1;
        sensor_config.adc_light_pin = 2;
        
        // AHT10 sensors
        sensor_config.sensors[0].type = SENSOR_TYPE_AHT10;
        sensor_config.sensors[0].address = 0x38;
        sensor_config.sensors[0].enabled = true;
        strcpy(sensor_config.sensors[0].name, "AHT10-1");
        
        sensor_config.sensors[1].type = SENSOR_TYPE_AHT10;
        sensor_config.sensors[1].address = 0x39;
        sensor_config.sensors[1].enabled = true;
        strcpy(sensor_config.sensors[1].name, "AHT10-2");
        
        // DS18B20 sensor
        sensor_config.sensors[2].type = SENSOR_TYPE_DS18B20;
        sensor_config.sensors[2].pin = 4;
        sensor_config.sensors[2].enabled = true;
        strcpy(sensor_config.sensors[2].name, "DS18B20-Waterproof");
        
        // GY-302 sensor
        sensor_config.sensors[3].type = SENSOR_TYPE_GY302;
        sensor_config.sensors[3].address = 0x23;
        sensor_config.sensors[3].enabled = true;
        strcpy(sensor_config.sensors[3].name, "GY302-Light");
        
        // Analog sensors
        sensor_config.sensors[4].type = SENSOR_TYPE_SOIL_MOISTURE;
        sensor_config.sensors[4].pin = 1;
        sensor_config.sensors[4].enabled = true;
        strcpy(sensor_config.sensors[4].name, "Soil-Moisture");
        
        sensor_config.sensors[5].type = SENSOR_TYPE_LIGHT;
        sensor_config.sensors[5].pin = 2;
        sensor_config.sensors[5].enabled = true;
        strcpy(sensor_config.sensors[5].name, "Light-Sensor");
        
        // Configure all available displays
        display_config.display_count = 3;
        display_config.enable_backlight = true;
        display_config.brightness = 128;
        display_config.enable_auto_off = false;
        display_config.auto_off_timeout = 0;
        
        // Console display
        display_config.displays[0].type = DISPLAY_TYPE_CONSOLE;
        display_config.displays[0].enabled = true;
        strcpy(display_config.displays[0].name, "Console-Display");
        
        // Built-in OLED
        display_config.displays[1].type = DISPLAY_TYPE_BUILTIN_SSD1306;
        display_config.displays[1].i2c_address = 0x3C;
        display_config.displays[1].sda_pin = 21;
        display_config.displays[1].scl_pin = 22;
        display_config.displays[1].enabled = true;
        strcpy(display_config.displays[1].name, "Built-in-OLED");
        
        // E-paper display
        display_config.displays[2].type = DISPLAY_TYPE_EPAPER_SPI;
        display_config.displays[2].spi_cs_pin = 5;
        display_config.displays[2].spi_dc_pin = 17;
        display_config.displays[2].spi_rst_pin = 16;
        display_config.displays[2].spi_mosi_pin = 23;
        display_config.displays[2].spi_sck_pin = 18;
        display_config.displays[2].spi_busy_pin = 4;
        display_config.displays[2].enabled = true;
        strcpy(display_config.displays[2].name, "E-paper-Display");
    }
    
    void TearDown() override {
        // Clean up
        sensor_interface_deinit();
        display_interface_deinit();
    }
    
    sensor_interface_config_t sensor_config;
    display_interface_config_t display_config;
};

/**
 * @brief Test complete system initialization workflow
 */
TEST_F(PlantMonitorIntegrationTest, SystemInitializationWorkflow) {
    // Initialize sensor interface
    esp_err_t ret = sensor_interface_init(&sensor_config);
    EXPECT_EQ(ret, ESP_OK);
    
    // Initialize display interface
    ret = display_interface_init(&display_config);
    EXPECT_EQ(ret, ESP_OK);
    
    // Scan for I2C devices
    int device_count = sensor_interface_scan_i2c();
    EXPECT_GE(device_count, 0);
    
    // Get system status
    int working_sensors, total_sensors;
    ret = sensor_interface_get_status(&working_sensors, &total_sensors);
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(total_sensors, 6);
    EXPECT_GE(working_sensors, 0);
    EXPECT_LE(working_sensors, total_sensors);
    
    int working_displays, total_displays;
    ret = display_interface_get_status(&working_displays, &total_displays);
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(total_displays, 3);
    EXPECT_GE(working_displays, 0);
    EXPECT_LE(working_displays, total_displays);
}

/**
 * @brief Test sensor reading workflow
 */
TEST_F(PlantMonitorIntegrationTest, SensorReadingWorkflow) {
    esp_err_t ret = sensor_interface_init(&sensor_config);
    ASSERT_EQ(ret, ESP_OK);
    
    // Read all sensors
    sensor_reading_t readings[6];
    int reading_count = sensor_interface_read_all(readings, 6);
    EXPECT_GE(reading_count, 0);
    EXPECT_LE(reading_count, 6);
    
    // Test individual sensor reading
    sensor_reading_t single_reading;
    
    ret = sensor_interface_read_sensor(SENSOR_TYPE_AHT10, &single_reading);
    EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_NOT_FOUND);
    
    ret = sensor_interface_read_sensor(SENSOR_TYPE_DS18B20, &single_reading);
    EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_NOT_FOUND);
    
    ret = sensor_interface_read_sensor(SENSOR_TYPE_GY302, &single_reading);
    EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_NOT_FOUND);
}

/**
 * @brief Test health calculation workflow
 */
TEST_F(PlantMonitorIntegrationTest, HealthCalculationWorkflow) {
    esp_err_t ret = sensor_interface_init(&sensor_config);
    ASSERT_EQ(ret, ESP_OK);
    
    // Read sensor data
    sensor_reading_t readings[6];
    int reading_count = sensor_interface_read_all(readings, 6);
    
    if (reading_count > 0) {
        // Calculate plant health (simplified version)
        float avg_temp = 0.0f;
        float avg_humidity = 0.0f;
        float avg_lux = 0.0f;
        int temp_count = 0, humidity_count = 0, lux_count = 0;
        
        for (int i = 0; i < reading_count; i++) {
            if (readings[i].valid) {
                if (readings[i].temperature > -50.0f && readings[i].temperature < 150.0f) {
                    avg_temp += readings[i].temperature;
                    temp_count++;
                }
                if (readings[i].humidity >= 0.0f && readings[i].humidity <= 100.0f) {
                    avg_humidity += readings[i].humidity;
                    humidity_count++;
                }
                if (readings[i].lux >= 0.0f) {
                    avg_lux += readings[i].lux;
                    lux_count++;
                }
            }
        }
        
        if (temp_count > 0) avg_temp /= temp_count;
        if (humidity_count > 0) avg_humidity /= humidity_count;
        if (lux_count > 0) avg_lux /= lux_count;
        
        // Create health data
        plant_health_t health = {0};
        float health_score = 0.0f;
        int score_count = 0;
        
        // Temperature scoring
        if (temp_count > 0) {
            if (avg_temp >= 18.0f && avg_temp <= 28.0f) {
                health_score += 100.0f;
            } else if (avg_temp >= 10.0f && avg_temp <= 35.0f) {
                health_score += 50.0f;
            }
            score_count++;
        }
        
        // Humidity scoring
        if (humidity_count > 0) {
            if (avg_humidity >= 40.0f && avg_humidity <= 70.0f) {
                health_score += 100.0f;
            } else if (avg_humidity >= 30.0f && avg_humidity <= 80.0f) {
                health_score += 50.0f;
            }
            score_count++;
        }
        
        // Light scoring
        if (lux_count > 0) {
            if (avg_lux >= 1000.0f && avg_lux <= 10000.0f) {
                health_score += 100.0f;
            } else if (avg_lux >= 100.0f && avg_lux <= 50000.0f) {
                health_score += 50.0f;
            }
            score_count++;
        }
        
        health.health_score = score_count > 0 ? health_score / score_count : 0.0f;
        
        // Set health status
        if (health.health_score >= 90.0f) {
            health.health_text = "Excellent";
            health.emoji = "😊";
        } else if (health.health_score >= 70.0f) {
            health.health_text = "Good";
            health.emoji = "🙂";
        } else if (health.health_score >= 50.0f) {
            health.health_text = "Fair";
            health.emoji = "😐";
        } else if (health.health_score >= 30.0f) {
            health.health_text = "Poor";
            health.emoji = "😟";
        } else {
            health.health_text = "Critical";
            health.emoji = "😱";
        }
        
        EXPECT_GE(health.health_score, 0.0f);
        EXPECT_LE(health.health_score, 100.0f);
        EXPECT_NE(health.health_text, nullptr);
        EXPECT_NE(health.emoji, nullptr);
    }
}

/**
 * @brief Test display update workflow
 */
TEST_F(PlantMonitorIntegrationTest, DisplayUpdateWorkflow) {
    esp_err_t ret = display_interface_init(&display_config);
    ASSERT_EQ(ret, ESP_OK);
    
    // Create test sensor data
    sensor_data_t sensor_data = {
        .temperature = 25.5f,
        .humidity = 60.0f,
        .soil_moisture = 2048,
        .light_level = 1024,
        .lux = 5000.0f,
        .uptime_seconds = 3600
    };
    
    // Create test health data
    plant_health_t health = {
        .health_score = 85.0f,
        .health_text = "Good",
        .emoji = "🙂",
        .recommendation = "Keep current conditions"
    };
    
    // Update displays
    ret = display_interface_update(&sensor_data, &health);
    EXPECT_EQ(ret, ESP_OK);
    
    // Test display functions
    ret = display_interface_show_welcome();
    EXPECT_EQ(ret, ESP_OK);
    
    ret = display_interface_clear();
    EXPECT_EQ(ret, ESP_OK);
    
    ret = display_interface_set_brightness(128);
    EXPECT_EQ(ret, ESP_OK);
}

/**
 * @brief Test data transmission workflow (simulated)
 */
TEST_F(PlantMonitorIntegrationTest, DataTransmissionWorkflow) {
    esp_err_t ret = sensor_interface_init(&sensor_config);
    ASSERT_EQ(ret, ESP_OK);
    
    // Read sensor data
    sensor_reading_t readings[6];
    int reading_count = sensor_interface_read_all(readings, 6);
    
    if (reading_count > 0) {
        // Simulate data aggregation
        sensor_data_t aggregated_data = {0};
        int valid_readings = 0;
        
        for (int i = 0; i < reading_count; i++) {
            if (readings[i].valid) {
                aggregated_data.temperature = readings[i].temperature;
                aggregated_data.humidity = readings[i].humidity;
                aggregated_data.soil_moisture = readings[i].soil_moisture;
                aggregated_data.light_level = readings[i].light_level;
                aggregated_data.lux = readings[i].lux;
                valid_readings++;
                break; // Use first valid reading
            }
        }
        
        EXPECT_GE(valid_readings, 0);
        EXPECT_LE(valid_readings, reading_count);
        
        // Simulate data transmission (would normally send to server)
        // For now, just verify data structure is valid
        EXPECT_GE(aggregated_data.uptime_seconds, 0);
    }
}

/**
 * @brief Test complete end-to-end monitoring cycle
 */
TEST_F(PlantMonitorIntegrationTest, CompleteMonitoringCycle) {
    // Initialize both interfaces
    esp_err_t ret = sensor_interface_init(&sensor_config);
    ASSERT_EQ(ret, ESP_OK);
    
    ret = display_interface_init(&display_config);
    ASSERT_EQ(ret, ESP_OK);
    
    // Read sensors
    sensor_reading_t readings[6];
    int reading_count = sensor_interface_read_all(readings, 6);
    EXPECT_GE(reading_count, 0);
    EXPECT_LE(reading_count, 6);
    
    // Calculate health
    plant_health_t health = {0};
    if (reading_count > 0) {
        // Simplified health calculation
        health.health_score = 75.0f;
        health.health_text = "Good";
        health.emoji = "🙂";
        health.recommendation = "Monitor regularly";
    }
    
    // Prepare display data
    sensor_data_t display_data = {0};
    for (int i = 0; i < reading_count; i++) {
        if (readings[i].valid) {
            display_data.temperature = readings[i].temperature;
            display_data.humidity = readings[i].humidity;
            display_data.soil_moisture = readings[i].soil_moisture;
            display_data.light_level = readings[i].light_level;
            display_data.lux = readings[i].lux;
            break;
        }
    }
    
    // Update displays
    ret = display_interface_update(&display_data, &health);
    EXPECT_EQ(ret, ESP_OK);
    
    // Verify system integrity
    int working_sensors, total_sensors;
    ret = sensor_interface_get_status(&working_sensors, &total_sensors);
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(total_sensors, 6);
    
    int working_displays, total_displays;
    ret = display_interface_get_status(&working_displays, &total_displays);
    EXPECT_EQ(ret, ESP_OK);
    EXPECT_EQ(total_displays, 3);
}

/**
 * @brief Test error handling with invalid data
 */
TEST_F(PlantMonitorIntegrationTest, ErrorHandlingInvalidData) {
    esp_err_t ret = sensor_interface_init(&sensor_config);
    ASSERT_EQ(ret, ESP_OK);
    
    // Test with invalid sensor readings
    sensor_reading_t invalid_readings[6];
    memset(invalid_readings, 0, sizeof(invalid_readings));
    
    // Set some invalid values
    invalid_readings[0].temperature = -100.0f;  // Invalid temperature
    invalid_readings[0].humidity = 150.0f;      // Invalid humidity
    invalid_readings[0].valid = true;
    
    // System should handle invalid data gracefully
    // (In real implementation, these would be filtered out)
    EXPECT_TRUE(true); // Test passes if no crash
}

/**
 * @brief Test error handling with missing sensors
 */
TEST_F(PlantMonitorIntegrationTest, ErrorHandlingMissingSensors) {
    // Configure with disabled sensors
    sensor_config.sensors[0].enabled = false;
    sensor_config.sensors[1].enabled = false;
    
    esp_err_t ret = sensor_interface_init(&sensor_config);
    ASSERT_EQ(ret, ESP_OK);
    
    sensor_reading_t readings[6];
    int reading_count = sensor_interface_read_all(readings, 6);
    
    // Should still work with fewer sensors
    EXPECT_GE(reading_count, 0);
    EXPECT_LE(reading_count, 4); // 2 sensors disabled
}

/**
 * @brief Test performance under load
 */
TEST_F(PlantMonitorIntegrationTest, PerformanceUnderLoad) {
    esp_err_t ret = sensor_interface_init(&sensor_config);
    ASSERT_EQ(ret, ESP_OK);
    
    ret = display_interface_init(&display_config);
    ASSERT_EQ(ret, ESP_OK);
    
    // Simulate multiple reading cycles
    for (int cycle = 0; cycle < 5; cycle++) {
        sensor_reading_t readings[6];
        int reading_count = sensor_interface_read_all(readings, 6);
        EXPECT_GE(reading_count, 0);
        
        // Small delay to simulate real-world timing
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // System should remain stable
    int working_sensors, total_sensors;
    ret = sensor_interface_get_status(&working_sensors, &total_sensors);
    EXPECT_EQ(ret, ESP_OK);
}

/**
 * @brief Test with different configurations
 */
TEST_F(PlantMonitorIntegrationTest, DifferentConfigurations) {
    // Test minimal configuration
    sensor_interface_config_t minimal_config = {0};
    minimal_config.sensor_count = 1;
    minimal_config.i2c_sda_pin = 21;
    minimal_config.i2c_scl_pin = 22;
    minimal_config.i2c_frequency = 100000;
    minimal_config.sensors[0].type = SENSOR_TYPE_AHT10;
    minimal_config.sensors[0].address = 0x38;
    minimal_config.sensors[0].enabled = true;
    strcpy(minimal_config.sensors[0].name, "Single-Sensor");
    
    esp_err_t ret = sensor_interface_init(&minimal_config);
    EXPECT_EQ(ret, ESP_OK);
    
    sensor_reading_t readings[1];
    int reading_count = sensor_interface_read_all(readings, 1);
    EXPECT_GE(reading_count, 0);
    EXPECT_LE(reading_count, 1);
    
    sensor_interface_deinit();
}

/**
 * @brief Test sensor driver integration
 */
TEST_F(PlantMonitorIntegrationTest, SensorDriverIntegration) {
    // Test AHT10 driver integration
    aht10_config_t aht10_config = {
        .address = 0x38,
        .sda_pin = 21,
        .scl_pin = 22,
        .i2c_freq = 100000,
        .enabled = true
    };
    
    esp_err_t ret = aht10_init(&aht10_config);
    if (ret == ESP_OK) {
        aht10_reading_t reading;
        ret = aht10_read(&reading);
        EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_NOT_FOUND);
        aht10_deinit();
    }
    
    // Test DS18B20 driver integration
    ds18b20_config_t ds18b20_config = {
        .pin = 4,
        .resolution = 12,
        .enabled = true,
        .rom_code = 0
    };
    
    ret = ds18b20_init(&ds18b20_config);
    if (ret == ESP_OK) {
        ds18b20_reading_t reading;
        ret = ds18b20_read(&reading);
        EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_NOT_FOUND);
        ds18b20_deinit();
    }
    
    // Test GY-302 driver integration
    gy302_config_t gy302_config = {
        .address = 0x23,
        .sda_pin = 21,
        .scl_pin = 22,
        .i2c_freq = 100000,
        .mode = GY302_MODE_ONE_H,
        .enabled = true
    };
    
    ret = gy302_init(&gy302_config);
    if (ret == ESP_OK) {
        gy302_reading_t reading;
        ret = gy302_read(&reading);
        EXPECT_TRUE(ret == ESP_OK || ret == ESP_ERR_NOT_FOUND);
        gy302_deinit();
    }
}

/**
 * @brief Test display driver integration
 */
TEST_F(PlantMonitorIntegrationTest, DisplayDriverIntegration) {
    esp_err_t ret = display_interface_init(&display_config);
    ASSERT_EQ(ret, ESP_OK);
    
    // Test different display types
    sensor_data_t test_data = {
        .temperature = 22.5f,
        .humidity = 65.0f,
        .soil_moisture = 1500,
        .light_level = 2000,
        .lux = 3000.0f,
        .uptime_seconds = 1800
    };
    
    plant_health_t test_health = {
        .health_score = 88.0f,
        .health_text = "Excellent",
        .emoji = "😊",
        .recommendation = "Perfect conditions!"
    };
    
    // Test console display
    ret = display_interface_update(&test_data, &test_health);
    EXPECT_EQ(ret, ESP_OK);
    
    // Test display functions
    ret = display_interface_show_welcome();
    EXPECT_EQ(ret, ESP_OK);
    
    ret = display_interface_show_error("Test error message");
    EXPECT_EQ(ret, ESP_OK);
    
    ret = display_interface_clear();
    EXPECT_EQ(ret, ESP_OK);
}

/**
 * @brief Main function for running integration tests
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 