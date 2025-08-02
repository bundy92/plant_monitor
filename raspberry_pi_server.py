#!/usr/bin/env python3
"""
Raspberry Pi Zero V2 Server for ESP32 Plant Monitor
Receives sensor data from ESP32-C6 and logs it to a database
"""

import json
import sqlite3
import logging
from datetime import datetime
from flask import Flask, request, jsonify
from flask_cors import CORS
import threading
import time

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('plant_monitor.log'),
        logging.StreamHandler()
    ]
)

app = Flask(__name__)
CORS(app)  # Enable CORS for web interface

# Database setup
def init_database():
    """Initialize SQLite database with plant monitoring table"""
    conn = sqlite3.connect('plant_data.db')
    cursor = conn.cursor()
    
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS sensor_data (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device_id TEXT NOT NULL,
            temperature REAL,
            humidity REAL,
            soil_moisture INTEGER,
            light_level INTEGER,
            timestamp INTEGER,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    ''')
    
    conn.commit()
    conn.close()
    logging.info("Database initialized successfully")

def save_sensor_data(data):
    """Save sensor data to SQLite database"""
    try:
        conn = sqlite3.connect('plant_data.db')
        cursor = conn.cursor()
        
        cursor.execute('''
            INSERT INTO sensor_data 
            (device_id, temperature, humidity, soil_moisture, light_level, timestamp)
            VALUES (?, ?, ?, ?, ?, ?)
        ''', (
            data.get('device_id', 'unknown'),
            data.get('temperature'),
            data.get('humidity'),
            data.get('soil_moisture'),
            data.get('light_level'),
            data.get('timestamp')
        ))
        
        conn.commit()
        conn.close()
        logging.info(f"Data saved: {data}")
        return True
    except Exception as e:
        logging.error(f"Error saving data: {e}")
        return False

@app.route('/data', methods=['POST'])
def receive_data():
    """Receive sensor data from ESP32"""
    try:
        data = request.get_json()
        
        if not data:
            return jsonify({'error': 'No JSON data received'}), 400
        
        # Validate required fields
        required_fields = ['temperature', 'humidity', 'soil_moisture', 'light_level']
        for field in required_fields:
            if field not in data:
                return jsonify({'error': f'Missing required field: {field}'}), 400
        
        # Save data to database
        if save_sensor_data(data):
            return jsonify({'status': 'success', 'message': 'Data received and saved'}), 200
        else:
            return jsonify({'status': 'error', 'message': 'Failed to save data'}), 500
            
    except Exception as e:
        logging.error(f"Error processing request: {e}")
        return jsonify({'error': 'Internal server error'}), 500

@app.route('/data', methods=['GET'])
def get_data():
    """Get recent sensor data"""
    try:
        limit = request.args.get('limit', 100, type=int)
        
        conn = sqlite3.connect('plant_data.db')
        cursor = conn.cursor()
        
        cursor.execute('''
            SELECT * FROM sensor_data 
            ORDER BY created_at DESC 
            LIMIT ?
        ''', (limit,))
        
        rows = cursor.fetchall()
        conn.close()
        
        # Convert to list of dictionaries
        data = []
        for row in rows:
            data.append({
                'id': row[0],
                'device_id': row[1],
                'temperature': row[2],
                'humidity': row[3],
                'soil_moisture': row[4],
                'light_level': row[5],
                'timestamp': row[6],
                'created_at': row[7]
            })
        
        return jsonify({'data': data, 'count': len(data)})
        
    except Exception as e:
        logging.error(f"Error retrieving data: {e}")
        return jsonify({'error': 'Internal server error'}), 500

@app.route('/status', methods=['GET'])
def get_status():
    """Get system status and latest readings"""
    try:
        conn = sqlite3.connect('plant_data.db')
        cursor = conn.cursor()
        
        # Get latest reading
        cursor.execute('''
            SELECT * FROM sensor_data 
            ORDER BY created_at DESC 
            LIMIT 1
        ''')
        
        latest = cursor.fetchone()
        
        # Get count of records
        cursor.execute('SELECT COUNT(*) FROM sensor_data')
        total_records = cursor.fetchone()[0]
        
        conn.close()
        
        if latest:
            status = {
                'status': 'online',
                'last_update': latest[7],
                'latest_reading': {
                    'temperature': latest[2],
                    'humidity': latest[3],
                    'soil_moisture': latest[4],
                    'light_level': latest[5]
                },
                'total_records': total_records
            }
        else:
            status = {
                'status': 'no_data',
                'total_records': 0
            }
        
        return jsonify(status)
        
    except Exception as e:
        logging.error(f"Error getting status: {e}")
        return jsonify({'error': 'Internal server error'}), 500

@app.route('/health', methods=['GET'])
def health_check():
    """Simple health check endpoint"""
    return jsonify({'status': 'healthy', 'timestamp': datetime.now().isoformat()})

def cleanup_old_data():
    """Clean up old data (keep last 30 days)"""
    while True:
        try:
            conn = sqlite3.connect('plant_data.db')
            cursor = conn.cursor()
            
            # Delete data older than 30 days
            cursor.execute('''
                DELETE FROM sensor_data 
                WHERE created_at < datetime('now', '-30 days')
            ''')
            
            deleted_count = cursor.rowcount
            conn.commit()
            conn.close()
            
            if deleted_count > 0:
                logging.info(f"Cleaned up {deleted_count} old records")
                
        except Exception as e:
            logging.error(f"Error during cleanup: {e}")
        
        # Run cleanup every 24 hours
        time.sleep(86400)

if __name__ == '__main__':
    # Initialize database
    init_database()
    
    # Start cleanup thread
    cleanup_thread = threading.Thread(target=cleanup_old_data, daemon=True)
    cleanup_thread.start()
    
    logging.info("Plant Monitor Server starting...")
    logging.info("Server will be available at http://0.0.0.0:8080")
    
    # Run Flask app
    app.run(host='0.0.0.0', port=8080, debug=False) 