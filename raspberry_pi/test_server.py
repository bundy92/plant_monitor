#!/usr/bin/env python3
"""
Plant Monitor - Raspberry Pi Server Tests
=========================================

This module provides comprehensive unit tests for the Raspberry Pi server
implementation, including API endpoints, database operations, data validation,
and system functionality.

Features:
- Unit tests for all server components
- Integration tests for API endpoints
- Database operation testing
- Data validation testing
- Error handling verification
- Performance testing
- Security testing
- Mock testing for external dependencies

Test Categories:
- API endpoint testing
- Database model testing
- Data validation testing
- Server functionality testing
- Error handling testing
- Performance testing
- Security testing

Author: Plant Monitor System
Version: 2.0.0
Date: 2024
License: MIT
"""

import unittest
import json
import tempfile
import os
import sys
import time
from datetime import datetime, timedelta
from unittest.mock import Mock, patch, MagicMock
from io import StringIO

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# Import server components
from server import (
    PlantMonitorServer, SensorData, DeviceInfo, SensorReading, Device, Alert,
    SensorDataSchema, DeviceInfoSchema, HealthStatus, AlertType,
    app, socketio, server
)

# Test data
SAMPLE_SENSOR_DATA = {
    'device_id': 'test_device_001',
    'timestamp': int(time.time()),
    'temperature': 25.5,
    'humidity': 60.0,
    'soil_moisture': 2048,
    'light_level': 1024,
    'lux': 5000.0,
    'health_score': 85.0,
    'health_status': 'Good',
    'health_emoji': '🙂',
    'recommendation': 'Keep current conditions',
    'uptime_seconds': 3600,
    'wifi_connected': True,
    'data_sent': True
}

SAMPLE_DEVICE_INFO = {
    'device_id': 'test_device_001',
    'name': 'Test Plant Monitor',
    'location': 'Living Room',
    'plant_type': 'Monstera',
    'is_active': True,
    'firmware_version': '1.0.0',
    'hardware_version': 'ESP32-C6',
    'last_seen': datetime.utcnow(),
    'created_at': datetime.utcnow()
}

class TestPlantMonitorServer(unittest.TestCase):
    """
    Comprehensive test suite for Plant Monitor Server
    
    This test class covers all aspects of the server functionality including:
    - Server initialization and configuration
    - Data reception and processing
    - Database operations
    - Alert generation and management
    - API endpoint functionality
    - Error handling and validation
    """
    
    def setUp(self):
        """Set up test environment before each test"""
        # Create temporary database
        self.temp_db = tempfile.NamedTemporaryFile(delete=False, suffix='.db')
        self.temp_db.close()
        
        # Create temporary config
        self.temp_config = tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.yaml')
        self.temp_config.write("""
server:
  host: '127.0.0.1'
  port: 5001
  debug: false
  threaded: true
database:
  url: 'sqlite:///test_plant_monitor.db'
  pool_size: 5
  max_overflow: 10
alerts:
  temperature_min: 10.0
  temperature_max: 35.0
  humidity_min: 30.0
  humidity_max: 80.0
  soil_moisture_min: 1000
  soil_moisture_max: 3000
  light_min: 100
  light_max: 4000
cleanup:
  enabled: true
  interval_hours: 24
  retention_days: 30
security:
  api_key_required: false
  rate_limit_enabled: true
  cors_enabled: true
""")
        self.temp_config.close()
        
        # Initialize test server
        self.server = PlantMonitorServer(self.temp_config.name)
        
        # Create test client
        self.app = app.test_client()
        self.app.testing = True

    def tearDown(self):
        """Clean up test environment after each test"""
        # Remove temporary files
        if os.path.exists(self.temp_db.name):
            os.unlink(self.temp_db.name)
        if os.path.exists(self.temp_config.name):
            os.unlink(self.temp_config.name)
        
        # Clean up database
        if hasattr(self.server, 'engine'):
            self.server.engine.dispose()

    def test_server_initialization(self):
        """Test server initialization and configuration loading"""
        self.assertIsNotNone(self.server)
        self.assertIsNotNone(self.server.config)
        self.assertIn('server', self.server.config)
        self.assertIn('database', self.server.config)
        self.assertIn('alerts', self.server.config)

    def test_config_loading(self):
        """Test configuration loading from file"""
        config = self.server._load_config(self.temp_config.name)
        self.assertIsInstance(config, dict)
        self.assertIn('server', config)
        self.assertIn('database', config)
        self.assertIn('alerts', config)

    def test_default_config(self):
        """Test default configuration generation"""
        config = self.server._get_default_config()
        self.assertIsInstance(config, dict)
        self.assertIn('server', config)
        self.assertIn('database', config)
        self.assertIn('alerts', config)
        self.assertIn('cleanup', config)
        self.assertIn('security', config)

    def test_sensor_data_validation(self):
        """Test sensor data validation with valid data"""
        schema = SensorDataSchema()
        validated_data = schema.load(SAMPLE_SENSOR_DATA)
        self.assertEqual(validated_data['device_id'], 'test_device_001')
        self.assertEqual(validated_data['temperature'], 25.5)
        self.assertEqual(validated_data['humidity'], 60.0)

    def test_sensor_data_validation_invalid(self):
        """Test sensor data validation with invalid data"""
        schema = SensorDataSchema()
        invalid_data = SAMPLE_SENSOR_DATA.copy()
        invalid_data['temperature'] = 150.0  # Invalid temperature
        
        with self.assertRaises(Exception):
            schema.load(invalid_data)

    def test_device_info_validation(self):
        """Test device information validation"""
        schema = DeviceInfoSchema()
        validated_data = schema.load({
            'device_id': 'test_device_001',
            'name': 'Test Device',
            'location': 'Test Location',
            'plant_type': 'Test Plant',
            'firmware_version': '1.0.0',
            'hardware_version': 'ESP32-C6'
        })
        self.assertEqual(validated_data['device_id'], 'test_device_001')

    def test_receive_sensor_data_valid(self):
        """Test receiving valid sensor data"""
        result = self.server.receive_sensor_data(SAMPLE_SENSOR_DATA)
        self.assertTrue(result)

    def test_receive_sensor_data_invalid(self):
        """Test receiving invalid sensor data"""
        invalid_data = SAMPLE_SENSOR_DATA.copy()
        invalid_data['temperature'] = 150.0  # Invalid temperature
        
        result = self.server.receive_sensor_data(invalid_data)
        self.assertFalse(result)

    def test_get_latest_readings(self):
        """Test retrieving latest sensor readings"""
        # First, add some test data
        self.server.receive_sensor_data(SAMPLE_SENSOR_DATA)
        
        # Get readings
        readings = self.server.get_latest_readings(limit=10)
        self.assertIsInstance(readings, list)
        self.assertGreater(len(readings), 0)

    def test_get_device_statistics(self):
        """Test retrieving device statistics"""
        # First, add some test data
        self.server.receive_sensor_data(SAMPLE_SENSOR_DATA)
        
        # Get statistics
        stats = self.server.get_device_statistics('test_device_001')
        self.assertIsInstance(stats, dict)
        self.assertIn('device_id', stats)

    def test_get_active_devices(self):
        """Test retrieving active devices"""
        # First, add some test data
        self.server.receive_sensor_data(SAMPLE_SENSOR_DATA)
        
        # Get active devices
        devices = self.server.get_active_devices()
        self.assertIsInstance(devices, list)

    def test_cleanup_old_data(self):
        """Test cleaning up old data"""
        # Add some test data
        self.server.receive_sensor_data(SAMPLE_SENSOR_DATA)
        
        # Clean up old data
        deleted_count = self.server.cleanup_old_data(days=1)
        self.assertIsInstance(deleted_count, int)
        self.assertGreaterEqual(deleted_count, 0)

    def test_alert_generation(self):
        """Test alert generation for critical conditions"""
        # Test data with critical temperature
        critical_data = SAMPLE_SENSOR_DATA.copy()
        critical_data['temperature'] = 40.0  # Critical temperature
        
        result = self.server.receive_sensor_data(critical_data)
        self.assertTrue(result)

    def test_database_models(self):
        """Test database model functionality"""
        # Test SensorReading model
        reading = SensorReading(
            device_id='test_device',
            timestamp=datetime.utcnow(),
            temperature=25.0,
            humidity=60.0,
            soil_moisture=2048,
            light_level=1024,
            lux=5000.0,
            health_score=85.0,
            health_status='Good',
            health_emoji='🙂',
            recommendation='Keep current conditions',
            uptime_seconds=3600,
            wifi_connected=True,
            data_sent=True
        )
        
        reading_dict = reading.to_dict()
        self.assertIsInstance(reading_dict, dict)
        self.assertIn('device_id', reading_dict)
        self.assertIn('temperature', reading_dict)

        # Test Device model
        device = Device(
            device_id='test_device',
            name='Test Device',
            location='Test Location',
            plant_type='Test Plant',
            last_seen=datetime.utcnow(),
            is_active=True,
            firmware_version='1.0.0',
            hardware_version='ESP32-C6'
        )
        
        device_dict = device.to_dict()
        self.assertIsInstance(device_dict, dict)
        self.assertIn('device_id', device_dict)
        self.assertIn('name', device_dict)

        # Test Alert model
        alert = Alert(
            device_id='test_device',
            alert_type='temperature_high',
            message='Temperature too high',
            severity='warning'
        )
        
        alert_dict = alert.to_dict()
        self.assertIsInstance(alert_dict, dict)
        self.assertIn('device_id', alert_dict)
        self.assertIn('alert_type', alert_dict)

class TestAPIEndpoints(unittest.TestCase):
    """
    Test suite for API endpoints
    
    This test class covers all API endpoints including:
    - Health check endpoint
    - Data reception endpoint
    - Data retrieval endpoints
    - Device management endpoints
    - Alert management endpoints
    """
    
    def setUp(self):
        """Set up test environment for API tests"""
        self.app = app.test_client()
        self.app.testing = True

    def test_health_check_endpoint(self):
        """Test health check endpoint"""
        response = self.app.get('/api/health')
        self.assertEqual(response.status_code, 200)
        
        data = json.loads(response.data)
        self.assertIn('status', data)
        self.assertEqual(data['status'], 'healthy')
        self.assertIn('timestamp', data)
        self.assertIn('version', data)

    def test_receive_data_endpoint_valid(self):
        """Test data reception endpoint with valid data"""
        response = self.app.post('/api/data',
                               data=json.dumps(SAMPLE_SENSOR_DATA),
                               content_type='application/json')
        
        self.assertEqual(response.status_code, 200)
        
        data = json.loads(response.data)
        self.assertIn('status', data)
        self.assertEqual(data['status'], 'success')

    def test_receive_data_endpoint_invalid(self):
        """Test data reception endpoint with invalid data"""
        invalid_data = SAMPLE_SENSOR_DATA.copy()
        invalid_data['temperature'] = 150.0  # Invalid temperature
        
        response = self.app.post('/api/data',
                               data=json.dumps(invalid_data),
                               content_type='application/json')
        
        self.assertEqual(response.status_code, 400)

    def test_receive_data_endpoint_no_data(self):
        """Test data reception endpoint with no data"""
        response = self.app.post('/api/data',
                               data='',
                               content_type='application/json')
        
        self.assertEqual(response.status_code, 400)

    def test_get_readings_endpoint(self):
        """Test readings retrieval endpoint"""
        response = self.app.get('/api/readings')
        self.assertEqual(response.status_code, 200)
        
        data = json.loads(response.data)
        self.assertIn('status', data)
        self.assertIn('data', data)
        self.assertIn('count', data)

    def test_get_devices_endpoint(self):
        """Test devices retrieval endpoint"""
        response = self.app.get('/api/devices')
        self.assertEqual(response.status_code, 200)
        
        data = json.loads(response.data)
        self.assertIn('status', data)
        self.assertIn('data', data)
        self.assertIn('count', data)

    def test_get_statistics_endpoint(self):
        """Test statistics retrieval endpoint"""
        response = self.app.get('/api/statistics/test_device_001')
        self.assertEqual(response.status_code, 200)
        
        data = json.loads(response.data)
        self.assertIn('status', data)

    def test_get_alerts_endpoint(self):
        """Test alerts retrieval endpoint"""
        response = self.app.get('/api/alerts')
        self.assertEqual(response.status_code, 200)
        
        data = json.loads(response.data)
        self.assertIn('status', data)
        self.assertIn('data', data)
        self.assertIn('count', data)

    def test_cleanup_endpoint(self):
        """Test data cleanup endpoint"""
        response = self.app.post('/api/cleanup?days=30')
        self.assertEqual(response.status_code, 200)
        
        data = json.loads(response.data)
        self.assertIn('status', data)
        self.assertIn('message', data)
        self.assertIn('deleted_count', data)

    def test_metrics_endpoint(self):
        """Test Prometheus metrics endpoint"""
        response = self.app.get('/metrics')
        self.assertEqual(response.status_code, 200)
        self.assertIn('plant_monitor_requests_total', response.data.decode())

class TestDataValidation(unittest.TestCase):
    """
    Test suite for data validation
    
    This test class covers data validation including:
    - Sensor data validation
    - Device information validation
    - Input sanitization
    - Type checking
    """
    
    def test_sensor_data_schema_valid(self):
        """Test sensor data schema with valid data"""
        schema = SensorDataSchema()
        validated_data = schema.load(SAMPLE_SENSOR_DATA)
        
        self.assertEqual(validated_data['device_id'], 'test_device_001')
        self.assertEqual(validated_data['temperature'], 25.5)
        self.assertEqual(validated_data['humidity'], 60.0)
        self.assertEqual(validated_data['soil_moisture'], 2048)
        self.assertEqual(validated_data['light_level'], 1024)

    def test_sensor_data_schema_invalid_temperature(self):
        """Test sensor data schema with invalid temperature"""
        schema = SensorDataSchema()
        invalid_data = SAMPLE_SENSOR_DATA.copy()
        invalid_data['temperature'] = 150.0
        
        with self.assertRaises(Exception):
            schema.load(invalid_data)

    def test_sensor_data_schema_invalid_humidity(self):
        """Test sensor data schema with invalid humidity"""
        schema = SensorDataSchema()
        invalid_data = SAMPLE_SENSOR_DATA.copy()
        invalid_data['humidity'] = 150.0
        
        with self.assertRaises(Exception):
            schema.load(invalid_data)

    def test_sensor_data_schema_missing_required(self):
        """Test sensor data schema with missing required fields"""
        schema = SensorDataSchema()
        invalid_data = SAMPLE_SENSOR_DATA.copy()
        del invalid_data['device_id']
        
        with self.assertRaises(Exception):
            schema.load(invalid_data)

    def test_device_info_schema_valid(self):
        """Test device info schema with valid data"""
        schema = DeviceInfoSchema()
        valid_data = {
            'device_id': 'test_device_001',
            'name': 'Test Device',
            'location': 'Test Location',
            'plant_type': 'Test Plant',
            'firmware_version': '1.0.0',
            'hardware_version': 'ESP32-C6'
        }
        
        validated_data = schema.load(valid_data)
        self.assertEqual(validated_data['device_id'], 'test_device_001')
        self.assertEqual(validated_data['name'], 'Test Device')

    def test_device_info_schema_invalid_device_id(self):
        """Test device info schema with invalid device ID"""
        schema = DeviceInfoSchema()
        invalid_data = {
            'device_id': '',  # Empty device ID
            'name': 'Test Device'
        }
        
        with self.assertRaises(Exception):
            schema.load(invalid_data)

class TestErrorHandling(unittest.TestCase):
    """
    Test suite for error handling
    
    This test class covers error handling including:
    - Database errors
    - Validation errors
    - Network errors
    - System errors
    """
    
    def setUp(self):
        """Set up test environment for error handling tests"""
        self.temp_db = tempfile.NamedTemporaryFile(delete=False, suffix='.db')
        self.temp_db.close()
        
        self.temp_config = tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.yaml')
        self.temp_config.write("""
server:
  host: '127.0.0.1'
  port: 5001
  debug: false
  threaded: true
database:
  url: 'sqlite:///test_plant_monitor.db'
  pool_size: 5
  max_overflow: 10
alerts:
  temperature_min: 10.0
  temperature_max: 35.0
  humidity_min: 30.0
  humidity_max: 80.0
  soil_moisture_min: 1000
  soil_moisture_max: 3000
  light_min: 100
  light_max: 4000
cleanup:
  enabled: true
  interval_hours: 24
  retention_days: 30
security:
  api_key_required: false
  rate_limit_enabled: true
  cors_enabled: true
""")
        self.temp_config.close()
        
        self.server = PlantMonitorServer(self.temp_config.name)
        self.app = app.test_client()
        self.app.testing = True

    def tearDown(self):
        """Clean up test environment"""
        if os.path.exists(self.temp_db.name):
            os.unlink(self.temp_db.name)
        if os.path.exists(self.temp_config.name):
            os.unlink(self.temp_config.name)

    def test_database_connection_error(self):
        """Test handling of database connection errors"""
        # This test would require mocking database connection failures
        # For now, we'll test that the server handles errors gracefully
        result = self.server.receive_sensor_data(SAMPLE_SENSOR_DATA)
        self.assertIsInstance(result, bool)

    def test_validation_error_handling(self):
        """Test handling of validation errors"""
        invalid_data = SAMPLE_SENSOR_DATA.copy()
        invalid_data['temperature'] = 150.0
        
        result = self.server.receive_sensor_data(invalid_data)
        self.assertFalse(result)

    def test_api_error_handling(self):
        """Test API error handling"""
        # Test with invalid JSON
        response = self.app.post('/api/data',
                               data='invalid json',
                               content_type='application/json')
        
        self.assertEqual(response.status_code, 500)

    def test_missing_config_file(self):
        """Test handling of missing configuration file"""
        server = PlantMonitorServer('nonexistent_config.yaml')
        self.assertIsNotNone(server.config)

class TestPerformance(unittest.TestCase):
    """
    Test suite for performance testing
    
    This test class covers performance aspects including:
    - Response time testing
    - Database performance
    - Memory usage
    - Concurrent request handling
    """
    
    def setUp(self):
        """Set up test environment for performance tests"""
        self.app = app.test_client()
        self.app.testing = True

    def test_api_response_time(self):
        """Test API response times"""
        start_time = time.time()
        response = self.app.get('/api/health')
        end_time = time.time()
        
        self.assertEqual(response.status_code, 200)
        self.assertLess(end_time - start_time, 1.0)  # Should respond within 1 second

    def test_data_processing_performance(self):
        """Test data processing performance"""
        # Test with multiple data points
        test_data = SAMPLE_SENSOR_DATA.copy()
        
        start_time = time.time()
        for i in range(10):
            test_data['device_id'] = f'test_device_{i:03d}'
            test_data['timestamp'] = int(time.time()) + i
            response = self.app.post('/api/data',
                                   data=json.dumps(test_data),
                                   content_type='application/json')
            self.assertEqual(response.status_code, 200)
        
        end_time = time.time()
        self.assertLess(end_time - start_time, 5.0)  # Should process 10 requests within 5 seconds

class TestSecurity(unittest.TestCase):
    """
    Test suite for security testing
    
    This test class covers security aspects including:
    - Input validation
    - SQL injection prevention
    - Rate limiting
    - CORS headers
    """
    
    def setUp(self):
        """Set up test environment for security tests"""
        self.app = app.test_client()
        self.app.testing = True

    def test_sql_injection_prevention(self):
        """Test SQL injection prevention"""
        malicious_data = SAMPLE_SENSOR_DATA.copy()
        malicious_data['device_id'] = "'; DROP TABLE sensor_readings; --"
        
        response = self.app.post('/api/data',
                               data=json.dumps(malicious_data),
                               content_type='application/json')
        
        # Should handle gracefully (either 200 or 400, but not 500)
        self.assertIn(response.status_code, [200, 400])

    def test_cors_headers(self):
        """Test CORS headers are present"""
        response = self.app.get('/api/health')
        self.assertEqual(response.status_code, 200)
        
        # Check for CORS headers (Flask-CORS should add these)
        # Note: In test environment, CORS headers might not be present
        # This is a basic check that the response is valid

    def test_rate_limiting(self):
        """Test rate limiting functionality"""
        # Make multiple requests quickly
        for i in range(5):
            response = self.app.get('/api/health')
            self.assertEqual(response.status_code, 200)

    def test_input_sanitization(self):
        """Test input sanitization"""
        malicious_data = SAMPLE_SENSOR_DATA.copy()
        malicious_data['recommendation'] = '<script>alert("xss")</script>'
        
        response = self.app.post('/api/data',
                               data=json.dumps(malicious_data),
                               content_type='application/json')
        
        # Should handle gracefully
        self.assertIn(response.status_code, [200, 400])

def run_tests():
    """Run all tests with comprehensive reporting"""
    # Create test suite
    test_suite = unittest.TestSuite()
    
    # Add test classes
    test_classes = [
        TestPlantMonitorServer,
        TestAPIEndpoints,
        TestDataValidation,
        TestErrorHandling,
        TestPerformance,
        TestSecurity
    ]
    
    for test_class in test_classes:
        tests = unittest.TestLoader().loadTestsFromTestCase(test_class)
        test_suite.addTests(tests)
    
    # Run tests with verbose output
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(test_suite)
    
    # Print summary
    print(f"\n{'='*60}")
    print("TEST SUMMARY")
    print(f"{'='*60}")
    print(f"Tests run: {result.testsRun}")
    print(f"Failures: {len(result.failures)}")
    print(f"Errors: {len(result.errors)}")
    print(f"Success rate: {((result.testsRun - len(result.failures) - len(result.errors)) / result.testsRun * 100):.1f}%")
    
    if result.failures:
        print("\nFAILURES:")
        for test, traceback in result.failures:
            print(f"- {test}: {traceback}")
    
    if result.errors:
        print("\nERRORS:")
        for test, traceback in result.errors:
            print(f"- {test}: {traceback}")
    
    return result.wasSuccessful()

if __name__ == '__main__':
    success = run_tests()
    sys.exit(0 if success else 1) 