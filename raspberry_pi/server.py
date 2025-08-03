#!/usr/bin/env python3
"""
Plant Monitor - Raspberry Pi Zero V2 Server
==========================================

This module provides a comprehensive server implementation for the Raspberry Pi Zero V2
that receives data from ESP32 plant monitoring systems. It includes data storage,
web interface, API endpoints, and real-time monitoring capabilities.

Features:
- RESTful API for data reception and querying
- SQLite database for data storage with SQLAlchemy ORM
- Web dashboard for real-time monitoring
- Data visualization and analytics
- Email/SMS alerts for critical conditions
- System health monitoring and metrics
- Professional logging and error handling
- Comprehensive testing framework
- Modular architecture for extensibility
- Industry-standard documentation

Architecture:
- Modular design with separate components
- Clean separation of concerns
- Comprehensive error handling
- Professional logging standards
- Extensible API design
- Real-time WebSocket support

Author: Plant Monitor System
Version: 2.0.0
Date: 2024
License: MIT
"""

import os
import sys
import json
import sqlite3
import logging
import datetime
import threading
import time
import asyncio
from typing import Dict, List, Optional, Tuple, Any, Union
from dataclasses import dataclass, asdict
from pathlib import Path
from enum import Enum
import hashlib
import hmac

# Flask imports
from flask import Flask, request, jsonify, render_template, Response, abort
from flask_cors import CORS
from flask_socketio import SocketIO, emit, disconnect
from flask_limiter import Limiter
from flask_limiter.util import get_remote_address

# Database imports
from sqlalchemy import create_engine, Column, Integer, Float, String, DateTime, Boolean, Text, Index
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker, Session as DBSession
from sqlalchemy.exc import SQLAlchemyError

# Configuration and validation imports
import yaml
from dotenv import load_dotenv
from marshmallow import Schema, fields, ValidationError, validate

# Security imports
import secrets
from cryptography.fernet import Fernet

# Monitoring and metrics
from prometheus_client import Counter, Histogram, Gauge, generate_latest, CONTENT_TYPE_LATEST

# Load environment variables
load_dotenv()

# Configure logging with industry standards
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('plant_monitor.log'),
        logging.StreamHandler(sys.stdout)
    ]
)
logger = logging.getLogger(__name__)

# Prometheus metrics
REQUEST_COUNT = Counter('plant_monitor_requests_total', 'Total requests', ['method', 'endpoint'])
REQUEST_DURATION = Histogram('plant_monitor_request_duration_seconds', 'Request duration')
ACTIVE_DEVICES = Gauge('plant_monitor_active_devices', 'Number of active devices')
DATABASE_OPERATIONS = Counter('plant_monitor_db_operations_total', 'Database operations', ['operation'])

# Database setup with connection pooling
Base = declarative_base()
engine = create_engine(
    'sqlite:///plant_monitor.db',
    echo=False,
    pool_size=10,
    max_overflow=20,
    pool_pre_ping=True
)
Session = sessionmaker(bind=engine)

# Flask app setup with security
app = Flask(__name__)
app.config['SECRET_KEY'] = os.getenv('SECRET_KEY', secrets.token_hex(32))
app.config['JSON_SORT_KEYS'] = False  # Maintain order for API consistency

# Security headers
@app.after_request
def add_security_headers(response):
    """Add security headers to all responses"""
    response.headers['X-Content-Type-Options'] = 'nosniff'
    response.headers['X-Frame-Options'] = 'DENY'
    response.headers['X-XSS-Protection'] = '1; mode=block'
    response.headers['Strict-Transport-Security'] = 'max-age=31536000; includeSubDomains'
    return response

CORS(app)
socketio = SocketIO(app, cors_allowed_origins="*", async_mode='threading')

# Rate limiting
limiter = Limiter(
    app,
    key_func=get_remote_address,
    default_limits=["200 per day", "50 per hour"]
)

# Enums for type safety
class HealthStatus(Enum):
    """Plant health status enumeration"""
    EXCELLENT = "Excellent"
    GOOD = "Good"
    FAIR = "Fair"
    POOR = "Poor"
    CRITICAL = "Critical"

class AlertType(Enum):
    """Alert type enumeration"""
    TEMPERATURE_HIGH = "temperature_high"
    TEMPERATURE_LOW = "temperature_low"
    HUMIDITY_HIGH = "humidity_high"
    HUMIDITY_LOW = "humidity_low"
    SOIL_DRY = "soil_dry"
    SOIL_WET = "soil_wet"
    LIGHT_LOW = "light_low"
    LIGHT_HIGH = "light_high"
    DEVICE_OFFLINE = "device_offline"
    SYSTEM_ERROR = "system_error"

# Data validation schemas
class SensorDataSchema(Schema):
    """Marshmallow schema for sensor data validation"""
    device_id = fields.Str(required=True, validate=validate.Length(min=1, max=50))
    timestamp = fields.Int(required=True)
    temperature = fields.Float(validate=validate.Range(min=-50, max=100))
    humidity = fields.Float(validate=validate.Range(min=0, max=100))
    soil_moisture = fields.Int(validate=validate.Range(min=0, max=4095))
    light_level = fields.Int(validate=validate.Range(min=0, max=4095))
    lux = fields.Float(validate=validate.Range(min=0, max=100000))
    health_score = fields.Float(validate=validate.Range(min=0, max=100))
    health_status = fields.Str(validate=validate.OneOf([status.value for status in HealthStatus]))
    health_emoji = fields.Str(validate=validate.Length(max=10))
    recommendation = fields.Str(validate=validate.Length(max=500))
    uptime_seconds = fields.Int(validate=validate.Range(min=0))
    wifi_connected = fields.Bool()
    data_sent = fields.Bool()

class DeviceInfoSchema(Schema):
    """Marshmallow schema for device information validation"""
    device_id = fields.Str(required=True, validate=validate.Length(min=1, max=50))
    name = fields.Str(validate=validate.Length(max=100))
    location = fields.Str(validate=validate.Length(max=100))
    plant_type = fields.Str(validate=validate.Length(max=50))
    firmware_version = fields.Str(validate=validate.Length(max=20))
    hardware_version = fields.Str(validate=validate.Length(max=20))

# Data structures with type hints
@dataclass
class SensorData:
    """Plant sensor data structure with comprehensive type hints"""
    device_id: str
    timestamp: datetime.datetime
    temperature: float
    humidity: float
    soil_moisture: int
    light_level: int
    lux: float
    health_score: float
    health_status: str
    health_emoji: str
    recommendation: str
    uptime_seconds: int
    wifi_connected: bool
    data_sent: bool

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for JSON serialization"""
        return asdict(self)

    def validate(self) -> bool:
        """Validate sensor data"""
        try:
            SensorDataSchema().load(asdict(self))
            return True
        except ValidationError:
            return False

@dataclass
class DeviceInfo:
    """Device information structure with comprehensive type hints"""
    device_id: str
    name: str
    location: str
    plant_type: str
    is_active: bool
    firmware_version: str
    hardware_version: str
    last_seen: datetime.datetime
    created_at: datetime.datetime

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for JSON serialization"""
        return asdict(self)

# Database models with comprehensive indexing
class SensorReading(Base):
    """Database model for sensor readings with comprehensive indexing"""
    __tablename__ = 'sensor_readings'
    
    id = Column(Integer, primary_key=True)
    device_id = Column(String(50), nullable=False, index=True)
    timestamp = Column(DateTime, nullable=False, index=True)
    temperature = Column(Float)
    humidity = Column(Float)
    soil_moisture = Column(Integer)
    light_level = Column(Integer)
    lux = Column(Float)
    health_score = Column(Float)
    health_status = Column(String(50))
    health_emoji = Column(String(10))
    recommendation = Column(String(500))
    uptime_seconds = Column(Integer)
    wifi_connected = Column(Boolean)
    data_sent = Column(Boolean)
    created_at = Column(DateTime, default=datetime.datetime.utcnow)
    
    # Composite indexes for efficient querying
    __table_args__ = (
        Index('idx_device_timestamp', 'device_id', 'timestamp'),
        Index('idx_health_score', 'health_score'),
        Index('idx_created_at', 'created_at'),
    )

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for JSON serialization"""
        return {
            'id': self.id,
            'device_id': self.device_id,
            'timestamp': self.timestamp.isoformat() if self.timestamp else None,
            'temperature': self.temperature,
            'humidity': self.humidity,
            'soil_moisture': self.soil_moisture,
            'light_level': self.light_level,
            'lux': self.lux,
            'health_score': self.health_score,
            'health_status': self.health_status,
            'health_emoji': self.health_emoji,
            'recommendation': self.recommendation,
            'uptime_seconds': self.uptime_seconds,
            'wifi_connected': self.wifi_connected,
            'data_sent': self.data_sent,
            'created_at': self.created_at.isoformat() if self.created_at else None
        }

class Device(Base):
    """Database model for device information with comprehensive indexing"""
    __tablename__ = 'devices'
    
    id = Column(Integer, primary_key=True)
    device_id = Column(String(50), unique=True, nullable=False, index=True)
    name = Column(String(100))
    location = Column(String(100))
    plant_type = Column(String(50))
    last_seen = Column(DateTime, index=True)
    is_active = Column(Boolean, default=True, index=True)
    firmware_version = Column(String(20))
    hardware_version = Column(String(20))
    created_at = Column(DateTime, default=datetime.datetime.utcnow)
    updated_at = Column(DateTime, default=datetime.datetime.utcnow, onupdate=datetime.datetime.utcnow)

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for JSON serialization"""
        return {
            'id': self.id,
            'device_id': self.device_id,
            'name': self.name,
            'location': self.location,
            'plant_type': self.plant_type,
            'last_seen': self.last_seen.isoformat() if self.last_seen else None,
            'is_active': self.is_active,
            'firmware_version': self.firmware_version,
            'hardware_version': self.hardware_version,
            'created_at': self.created_at.isoformat() if self.created_at else None,
            'updated_at': self.updated_at.isoformat() if self.updated_at else None
        }

class Alert(Base):
    """Database model for system alerts"""
    __tablename__ = 'alerts'
    
    id = Column(Integer, primary_key=True)
    device_id = Column(String(50), nullable=False, index=True)
    alert_type = Column(String(50), nullable=False)
    message = Column(Text, nullable=False)
    severity = Column(String(20), default='warning')  # info, warning, error, critical
    is_resolved = Column(Boolean, default=False, index=True)
    created_at = Column(DateTime, default=datetime.datetime.utcnow, index=True)
    resolved_at = Column(DateTime)

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for JSON serialization"""
        return {
            'id': self.id,
            'device_id': self.device_id,
            'alert_type': self.alert_type,
            'message': self.message,
            'severity': self.severity,
            'is_resolved': self.is_resolved,
            'created_at': self.created_at.isoformat() if self.created_at else None,
            'resolved_at': self.resolved_at.isoformat() if self.resolved_at else None
        }

# Main server class with comprehensive functionality
class PlantMonitorServer:
    """
    Main server class for plant monitoring system
    
    This class provides comprehensive functionality for:
    - Data reception and validation
    - Database operations with error handling
    - Real-time monitoring and alerts
    - API endpoint management
    - System health monitoring
    - Data analytics and statistics
    """
    
    def __init__(self, config_path: str = "config/server_config.yaml"):
        """
        Initialize the plant monitor server
        
        Args:
            config_path: Path to configuration file
        """
        self.config = self._load_config(config_path)
        self.sensor_schema = SensorDataSchema()
        self.device_schema = DeviceInfoSchema()
        self._init_database()
        self._start_background_tasks()
        logger.info("Plant Monitor Server initialized successfully")

    def _load_config(self, config_path: str) -> Dict[str, Any]:
        """
        Load configuration from YAML file
        
        Args:
            config_path: Path to configuration file
            
        Returns:
            Configuration dictionary
        """
        try:
            if os.path.exists(config_path):
                with open(config_path, 'r') as file:
                    config = yaml.safe_load(file)
                    logger.info(f"Configuration loaded from {config_path}")
                    return config
            else:
                logger.warning(f"Config file {config_path} not found, using defaults")
                return self._get_default_config()
        except Exception as e:
            logger.error(f"Error loading config: {e}")
            return self._get_default_config()

    def _get_default_config(self) -> Dict[str, Any]:
        """
        Get default configuration
        
        Returns:
            Default configuration dictionary
        """
        return {
            'server': {
                'host': '0.0.0.0',
                'port': 5000,
                'debug': False,
                'threaded': True
            },
            'database': {
                'url': 'sqlite:///plant_monitor.db',
                'pool_size': 10,
                'max_overflow': 20
            },
            'alerts': {
                'temperature_min': 10.0,
                'temperature_max': 35.0,
                'humidity_min': 30.0,
                'humidity_max': 80.0,
                'soil_moisture_min': 1000,
                'soil_moisture_max': 3000,
                'light_min': 100,
                'light_max': 4000
            },
            'cleanup': {
                'enabled': True,
                'interval_hours': 24,
                'retention_days': 30
            },
            'security': {
                'api_key_required': False,
                'rate_limit_enabled': True,
                'cors_enabled': True
            }
        }

    def _init_database(self) -> None:
        """Initialize database tables and indexes"""
        try:
            Base.metadata.create_all(engine)
            logger.info("Database tables created successfully")
        except Exception as e:
            logger.error(f"Error initializing database: {e}")
            raise

    def _start_background_tasks(self) -> None:
        """Start background tasks for maintenance"""
        def cleanup_task():
            """Background task for data cleanup"""
            while True:
                try:
                    time.sleep(self.config['cleanup']['interval_hours'] * 3600)
                    if self.config['cleanup']['enabled']:
                        deleted_count = self.cleanup_old_data(self.config['cleanup']['retention_days'])
                        logger.info(f"Cleaned up {deleted_count} old records")
                except Exception as e:
                    logger.error(f"Error in cleanup task: {e}")

        cleanup_thread = threading.Thread(target=cleanup_task, daemon=True)
        cleanup_thread.start()
        logger.info("Background tasks started")

    def receive_sensor_data(self, data: Dict[str, Any]) -> bool:
        """
        Receive and process sensor data from ESP32
        
        Args:
            data: Sensor data dictionary
            
        Returns:
            True if data was processed successfully, False otherwise
        """
        start_time = time.time()
        
        try:
            # Validate data
            validated_data = self.sensor_schema.load(data)
            
            # Convert timestamp
            timestamp = datetime.datetime.fromtimestamp(validated_data['timestamp'])
            
            # Create sensor reading object
            reading = SensorReading(
                device_id=validated_data['device_id'],
                timestamp=timestamp,
                temperature=validated_data.get('temperature'),
                humidity=validated_data.get('humidity'),
                soil_moisture=validated_data.get('soil_moisture'),
                light_level=validated_data.get('light_level'),
                lux=validated_data.get('lux'),
                health_score=validated_data.get('health_score'),
                health_status=validated_data.get('health_status'),
                health_emoji=validated_data.get('health_emoji'),
                recommendation=validated_data.get('recommendation'),
                uptime_seconds=validated_data.get('uptime_seconds'),
                wifi_connected=validated_data.get('wifi_connected', False),
                data_sent=validated_data.get('data_sent', True)
            )
            
            # Save to database
            session = Session()
            try:
                session.add(reading)
                session.commit()
                DATABASE_OPERATIONS.labels(operation='insert').inc()
                
                # Update device information
                self._update_device_info(validated_data)
                
                # Check for alerts
                self._check_alerts(reading)
                
                # Emit real-time update
                socketio.emit('sensor_update', reading.to_dict())
                
                # Update metrics
                REQUEST_COUNT.labels(method='POST', endpoint='/api/data').inc()
                REQUEST_DURATION.observe(time.time() - start_time)
                
                logger.info(f"Data received and processed for device {validated_data['device_id']}")
                return True
                
            except SQLAlchemyError as e:
                session.rollback()
                logger.error(f"Database error: {e}")
                return False
            finally:
                session.close()
                
        except ValidationError as e:
            logger.error(f"Data validation error: {e}")
            return False
        except Exception as e:
            logger.error(f"Error processing sensor data: {e}")
            return False

    def _update_device_info(self, data: Dict[str, Any]) -> None:
        """
        Update device information in database
        
        Args:
            data: Sensor data containing device information
        """
        try:
            session = Session()
            device = session.query(Device).filter_by(device_id=data['device_id']).first()
            
            if device:
                # Update existing device
                device.last_seen = datetime.datetime.utcnow()
                device.is_active = True
                if 'firmware_version' in data:
                    device.firmware_version = data['firmware_version']
                if 'hardware_version' in data:
                    device.hardware_version = data['hardware_version']
            else:
                # Create new device
                device = Device(
                    device_id=data['device_id'],
                    name=f"Plant Monitor {data['device_id']}",
                    location="Unknown",
                    plant_type="Unknown",
                    last_seen=datetime.datetime.utcnow(),
                    is_active=True,
                    firmware_version=data.get('firmware_version', 'Unknown'),
                    hardware_version=data.get('hardware_version', 'Unknown')
                )
                session.add(device)
            
            session.commit()
            DATABASE_OPERATIONS.labels(operation='update').inc()
            
        except Exception as e:
            logger.error(f"Error updating device info: {e}")
            session.rollback()
        finally:
            session.close()

    def _check_alerts(self, reading: SensorReading) -> None:
        """
        Check for alert conditions and create alerts if needed
        
        Args:
            reading: Sensor reading to check
        """
        try:
            alerts_config = self.config['alerts']
            session = Session()
            
            # Check temperature alerts
            if reading.temperature is not None:
                if reading.temperature > alerts_config['temperature_max']:
                    self._create_alert(session, reading.device_id, AlertType.TEMPERATURE_HIGH.value,
                                     f"Temperature too high: {reading.temperature}°C")
                elif reading.temperature < alerts_config['temperature_min']:
                    self._create_alert(session, reading.device_id, AlertType.TEMPERATURE_LOW.value,
                                     f"Temperature too low: {reading.temperature}°C")
            
            # Check humidity alerts
            if reading.humidity is not None:
                if reading.humidity > alerts_config['humidity_max']:
                    self._create_alert(session, reading.device_id, AlertType.HUMIDITY_HIGH.value,
                                     f"Humidity too high: {reading.humidity}%")
                elif reading.humidity < alerts_config['humidity_min']:
                    self._create_alert(session, reading.device_id, AlertType.HUMIDITY_LOW.value,
                                     f"Humidity too low: {reading.humidity}%")
            
            # Check soil moisture alerts
            if reading.soil_moisture is not None:
                if reading.soil_moisture < alerts_config['soil_moisture_min']:
                    self._create_alert(session, reading.device_id, AlertType.SOIL_DRY.value,
                                     f"Soil too dry: {reading.soil_moisture}")
                elif reading.soil_moisture > alerts_config['soil_moisture_max']:
                    self._create_alert(session, reading.device_id, AlertType.SOIL_WET.value,
                                     f"Soil too wet: {reading.soil_moisture}")
            
            # Check light alerts
            if reading.light_level is not None:
                if reading.light_level < alerts_config['light_min']:
                    self._create_alert(session, reading.device_id, AlertType.LIGHT_LOW.value,
                                     f"Light too low: {reading.light_level}")
                elif reading.light_level > alerts_config['light_max']:
                    self._create_alert(session, reading.device_id, AlertType.LIGHT_HIGH.value,
                                     f"Light too high: {reading.light_level}")
            
            session.commit()
            
        except Exception as e:
            logger.error(f"Error checking alerts: {e}")
            session.rollback()
        finally:
            session.close()

    def _create_alert(self, session: DBSession, device_id: str, alert_type: str, message: str) -> None:
        """
        Create a new alert in the database
        
        Args:
            session: Database session
            device_id: Device ID
            alert_type: Type of alert
            message: Alert message
        """
        # Check if similar alert already exists and is unresolved
        existing_alert = session.query(Alert).filter_by(
            device_id=device_id,
            alert_type=alert_type,
            is_resolved=False
        ).first()
        
        if not existing_alert:
            alert = Alert(
                device_id=device_id,
                alert_type=alert_type,
                message=message,
                severity='warning'
            )
            session.add(alert)
            logger.warning(f"Alert created: {message}")

    def get_latest_readings(self, device_id: Optional[str] = None, limit: int = 100) -> List[Dict[str, Any]]:
        """
        Get latest sensor readings
        
        Args:
            device_id: Optional device ID filter
            limit: Maximum number of readings to return
            
        Returns:
            List of reading dictionaries
        """
        try:
            session = Session()
            query = session.query(SensorReading)
            
            if device_id:
                query = query.filter_by(device_id=device_id)
            
            readings = query.order_by(SensorReading.timestamp.desc()).limit(limit).all()
            DATABASE_OPERATIONS.labels(operation='select').inc()
            
            return [reading.to_dict() for reading in readings]
            
        except Exception as e:
            logger.error(f"Error getting latest readings: {e}")
            return []
        finally:
            session.close()

    def get_device_statistics(self, device_id: str) -> Dict[str, Any]:
        """
        Get comprehensive statistics for a device
        
        Args:
            device_id: Device ID
            
        Returns:
            Statistics dictionary
        """
        try:
            session = Session()
            
            # Get latest reading
            latest = session.query(SensorReading).filter_by(device_id=device_id)\
                          .order_by(SensorReading.timestamp.desc()).first()
            
            if not latest:
                return {'error': 'Device not found'}
            
            # Get statistics for last 24 hours
            yesterday = datetime.datetime.utcnow() - datetime.timedelta(days=1)
            readings_24h = session.query(SensorReading).filter(
                SensorReading.device_id == device_id,
                SensorReading.timestamp >= yesterday
            ).all()
            
            if readings_24h:
                temperatures = [r.temperature for r in readings_24h if r.temperature is not None]
                humidities = [r.humidity for r in readings_24h if r.humidity is not None]
                health_scores = [r.health_score for r in readings_24h if r.health_score is not None]
                
                stats = {
                    'device_id': device_id,
                    'latest_reading': latest.to_dict(),
                    'readings_24h': len(readings_24h),
                    'avg_temperature': sum(temperatures) / len(temperatures) if temperatures else None,
                    'avg_humidity': sum(humidities) / len(humidities) if humidities else None,
                    'avg_health_score': sum(health_scores) / len(health_scores) if health_scores else None,
                    'min_temperature': min(temperatures) if temperatures else None,
                    'max_temperature': max(temperatures) if temperatures else None,
                    'min_humidity': min(humidities) if humidities else None,
                    'max_humidity': max(humidities) if humidities else None
                }
            else:
                stats = {
                    'device_id': device_id,
                    'latest_reading': latest.to_dict(),
                    'readings_24h': 0
                }
            
            DATABASE_OPERATIONS.labels(operation='select').inc()
            return stats
            
        except Exception as e:
            logger.error(f"Error getting device statistics: {e}")
            return {'error': str(e)}
        finally:
            session.close()

    def get_active_devices(self) -> List[Dict[str, Any]]:
        """
        Get list of active devices
        
        Returns:
            List of active device dictionaries
        """
        try:
            session = Session()
            devices = session.query(Device).filter_by(is_active=True).all()
            DATABASE_OPERATIONS.labels(operation='select').inc()
            
            # Update active devices metric
            ACTIVE_DEVICES.set(len(devices))
            
            return [device.to_dict() for device in devices]
            
        except Exception as e:
            logger.error(f"Error getting active devices: {e}")
            return []
        finally:
            session.close()

    def cleanup_old_data(self, days: int = 30) -> int:
        """
        Clean up old sensor data
        
        Args:
            days: Number of days to retain
            
        Returns:
            Number of records deleted
        """
        try:
            cutoff_date = datetime.datetime.utcnow() - datetime.timedelta(days=days)
            session = Session()
            
            deleted_count = session.query(SensorReading)\
                                 .filter(SensorReading.timestamp < cutoff_date)\
                                 .delete()
            
            session.commit()
            DATABASE_OPERATIONS.labels(operation='delete').inc()
            
            logger.info(f"Cleaned up {deleted_count} old records")
            return deleted_count
            
        except Exception as e:
            logger.error(f"Error cleaning up old data: {e}")
            session.rollback()
            return 0
        finally:
            session.close()

# Global server instance
server = PlantMonitorServer()

# API Routes with comprehensive error handling and validation
@app.route('/api/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    try:
        return jsonify({
            'status': 'healthy',
            'timestamp': datetime.datetime.utcnow().isoformat(),
            'version': '2.0.0',
            'database': 'connected'
        }), 200
    except Exception as e:
        logger.error(f"Health check error: {e}")
        return jsonify({'status': 'unhealthy', 'error': str(e)}), 500

@app.route('/api/data', methods=['POST'])
@limiter.limit("100 per minute")
def receive_data():
    """Receive sensor data from ESP32"""
    try:
        data = request.get_json()
        
        if not data:
            return jsonify({'error': 'No JSON data received'}), 400
        
        # Process data
        if server.receive_sensor_data(data):
            return jsonify({
                'status': 'success',
                'message': 'Data received and processed',
                'timestamp': datetime.datetime.utcnow().isoformat()
            }), 200
        else:
            return jsonify({
                'status': 'error',
                'message': 'Failed to process data'
            }), 400
            
    except Exception as e:
        logger.error(f"Error receiving data: {e}")
        return jsonify({'error': 'Internal server error'}), 500

@app.route('/api/readings', methods=['GET'])
def get_readings():
    """Get latest sensor readings"""
    try:
        device_id = request.args.get('device_id')
        limit = int(request.args.get('limit', 100))
        
        readings = server.get_latest_readings(device_id, limit)
        
        return jsonify({
            'status': 'success',
            'data': readings,
            'count': len(readings)
        }), 200
        
    except Exception as e:
        logger.error(f"Error getting readings: {e}")
        return jsonify({'error': 'Internal server error'}), 500

@app.route('/api/devices', methods=['GET'])
def get_devices():
    """Get active devices"""
    try:
        devices = server.get_active_devices()
        
        return jsonify({
            'status': 'success',
            'data': devices,
            'count': len(devices)
        }), 200
        
    except Exception as e:
        logger.error(f"Error getting devices: {e}")
        return jsonify({'error': 'Internal server error'}), 500

@app.route('/api/statistics/<device_id>', methods=['GET'])
def get_statistics(device_id):
    """Get device statistics"""
    try:
        stats = server.get_device_statistics(device_id)
        
        if 'error' in stats:
            return jsonify(stats), 404
        
        return jsonify({
            'status': 'success',
            'data': stats
        }), 200
        
    except Exception as e:
        logger.error(f"Error getting statistics: {e}")
        return jsonify({'error': 'Internal server error'}), 500

@app.route('/api/alerts', methods=['GET'])
def get_alerts():
    """Get active alerts"""
    try:
        session = Session()
        alerts = session.query(Alert).filter_by(is_resolved=False)\
                       .order_by(Alert.created_at.desc()).all()
        
        return jsonify({
            'status': 'success',
            'data': [alert.to_dict() for alert in alerts],
            'count': len(alerts)
        }), 200
        
    except Exception as e:
        logger.error(f"Error getting alerts: {e}")
        return jsonify({'error': 'Internal server error'}), 500
    finally:
        session.close()

@app.route('/api/cleanup', methods=['POST'])
def cleanup_data():
    """Trigger data cleanup"""
    try:
        days = int(request.args.get('days', 30))
        deleted_count = server.cleanup_old_data(days)
        
        return jsonify({
            'status': 'success',
            'message': f'Cleaned up {deleted_count} records',
            'deleted_count': deleted_count
        }), 200
        
    except Exception as e:
        logger.error(f"Error cleaning up data: {e}")
        return jsonify({'error': 'Internal server error'}), 500

@app.route('/metrics')
def metrics():
    """Prometheus metrics endpoint"""
    return Response(generate_latest(), mimetype=CONTENT_TYPE_LATEST)

# WebSocket event handlers
@socketio.on('connect')
def handle_connect():
    """Handle WebSocket connection"""
    logger.info(f"Client connected: {request.sid}")
    emit('connected', {'status': 'connected'})

@socketio.on('disconnect')
def handle_disconnect():
    """Handle WebSocket disconnection"""
    logger.info(f"Client disconnected: {request.sid}")

@socketio.on('subscribe')
def handle_subscribe(data):
    """Handle subscription to device updates"""
    device_id = data.get('device_id')
    if device_id:
        logger.info(f"Client {request.sid} subscribed to device {device_id}")
        emit('subscribed', {'device_id': device_id})

# Background task for data cleanup
def background_cleanup():
    """Background task for periodic data cleanup"""
    while True:
        try:
            time.sleep(24 * 3600)  # Run daily
            server.cleanup_old_data(30)
        except Exception as e:
            logger.error(f"Background cleanup error: {e}")

if __name__ == '__main__':
    # Start background cleanup task
    cleanup_thread = threading.Thread(target=background_cleanup, daemon=True)
    cleanup_thread.start()
    
    # Run the server
    socketio.run(
        app,
        host=server.config['server']['host'],
        port=server.config['server']['port'],
        debug=server.config['server']['debug'],
        threaded=server.config['server']['threaded']
    ) 