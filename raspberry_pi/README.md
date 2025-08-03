# 🌱 Plant Monitor - Raspberry Pi Zero V2 Server

A comprehensive, industry-standard server implementation for the Raspberry Pi Zero V2 that receives and processes data from ESP32 plant monitoring systems. Built with Flask, SQLAlchemy, and modern Python practices.

## 🎯 **Features**

### **Core Functionality**
- ✅ **RESTful API** - Comprehensive REST API for data reception and querying
- ✅ **Real-time WebSocket Support** - Live updates via WebSocket connections
- ✅ **SQLite Database** - Robust data storage with SQLAlchemy ORM
- ✅ **Data Validation** - Comprehensive input validation with Marshmallow
- ✅ **Alert System** - Intelligent alert generation for critical conditions
- ✅ **Rate Limiting** - Built-in rate limiting for API protection
- ✅ **Prometheus Metrics** - Comprehensive monitoring and metrics

### **Security & Reliability**
- ✅ **Input Sanitization** - Protection against SQL injection and XSS
- ✅ **CORS Support** - Cross-origin resource sharing configuration
- ✅ **Error Handling** - Comprehensive error handling and logging
- ✅ **Data Validation** - Strict input validation and sanitization
- ✅ **Security Headers** - HTTP security headers for protection

### **Monitoring & Analytics**
- ✅ **Health Monitoring** - System health checks and status endpoints
- ✅ **Performance Metrics** - Request timing and performance tracking
- ✅ **Device Management** - Active device tracking and statistics
- ✅ **Data Analytics** - Statistical analysis and reporting
- ✅ **Background Tasks** - Automated data cleanup and maintenance

## 🚀 **Quick Start**

### **Prerequisites**
- Raspberry Pi Zero V2 (or any Raspberry Pi)
- Python 3.8+
- Internet connection for dependency installation

### **Installation**

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd plant_monitor/raspberry_pi
   ```

2. **Install dependencies**
   ```bash
   pip3 install -r requirements.txt
   ```

3. **Configure the server**
   ```bash
   cp config/server_config.yaml.example config/server_config.yaml
   # Edit config/server_config.yaml with your settings
   ```

4. **Run the server**
   ```bash
   python3 server.py
   ```

### **Configuration**

The server uses YAML configuration files. Create `config/server_config.yaml`:

```yaml
server:
  host: '0.0.0.0'
  port: 5000
  debug: false
  threaded: true

database:
  url: 'sqlite:///plant_monitor.db'
  pool_size: 10
  max_overflow: 20

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
```

## 📊 **API Endpoints**

### **Core Endpoints**

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/health` | GET | System health check |
| `/api/data` | POST | Receive sensor data |
| `/api/readings` | GET | Get sensor readings |
| `/api/devices` | GET | Get active devices |
| `/api/statistics/<device_id>` | GET | Get device statistics |
| `/api/alerts` | GET | Get active alerts |
| `/api/cleanup` | POST | Trigger data cleanup |
| `/metrics` | GET | Prometheus metrics |

### **Example Usage**

**Send sensor data:**
```bash
curl -X POST http://localhost:5000/api/data \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": "esp32_001",
    "timestamp": 1640995200,
    "temperature": 25.5,
    "humidity": 60.0,
    "soil_moisture": 2048,
    "light_level": 1024,
    "lux": 5000.0,
    "health_score": 85.0,
    "health_status": "Good",
    "health_emoji": "🙂",
    "recommendation": "Keep current conditions",
    "uptime_seconds": 3600,
    "wifi_connected": true,
    "data_sent": true
  }'
```

**Get latest readings:**
```bash
curl http://localhost:5000/api/readings?limit=10
```

**Get device statistics:**
```bash
curl http://localhost:5000/api/statistics/esp32_001
```

## 🧪 **Testing**

### **Run All Tests**
```bash
chmod +x run_tests.sh
./run_tests.sh
```

### **Run Specific Test Categories**
```bash
# Unit tests only
./run_tests.sh unit

# Performance tests
./run_tests.sh performance

# Security tests
./run_tests.sh security

# Code quality checks
./run_tests.sh quality

# Coverage analysis
./run_tests.sh coverage
```

### **Test Categories**

- **Unit Tests** - Individual component testing
- **Integration Tests** - System workflow testing
- **Performance Tests** - Response time and throughput
- **Security Tests** - Input validation and protection
- **Code Quality** - Linting, formatting, and documentation
- **Coverage Analysis** - Code coverage reporting
- **Load Testing** - Concurrent request handling
- **Database Tests** - Database operation testing
- **API Tests** - Endpoint functionality testing

## 📈 **Monitoring & Metrics**

### **Prometheus Metrics**

The server exposes Prometheus metrics at `/metrics`:

- `plant_monitor_requests_total` - Total API requests
- `plant_monitor_request_duration_seconds` - Request duration
- `plant_monitor_active_devices` - Number of active devices
- `plant_monitor_db_operations_total` - Database operations

### **Health Checks**

```bash
curl http://localhost:5000/api/health
```

Response:
```json
{
  "status": "healthy",
  "timestamp": "2024-01-01T12:00:00Z",
  "version": "2.0.0",
  "database": "connected"
}
```

## 🔧 **Development**

### **Project Structure**
```
raspberry_pi/
├── server.py              # Main server implementation
├── test_server.py         # Comprehensive test suite
├── run_tests.sh          # Test runner script
├── requirements.txt      # Python dependencies
├── config/              # Configuration files
│   └── server_config.yaml
├── reports/             # Test reports and coverage
└── README.md           # This file
```

### **Code Quality Standards**

- **Documentation** - Comprehensive docstrings and comments
- **Type Hints** - Full type annotation support
- **Error Handling** - Robust error handling and logging
- **Testing** - 80%+ code coverage requirement
- **Security** - Input validation and sanitization
- **Performance** - Optimized database queries and caching

### **Development Workflow**

1. **Install development dependencies**
   ```bash
   pip3 install -r requirements.txt
   ```

2. **Run code quality checks**
   ```bash
   ./run_tests.sh quality
   ```

3. **Run tests**
   ```bash
   ./run_tests.sh all
   ```

4. **Format code**
   ```bash
   black server.py test_server.py
   ```

5. **Type checking**
   ```bash
   mypy server.py
   ```

## 🚀 **Production Deployment**

### **Systemd Service**

Create `/etc/systemd/system/plant-monitor.service`:

```ini
[Unit]
Description=Plant Monitor Server
After=network.target

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/plant_monitor/raspberry_pi
ExecStart=/usr/bin/python3 server.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

### **Enable and Start Service**
```bash
sudo systemctl enable plant-monitor
sudo systemctl start plant-monitor
sudo systemctl status plant-monitor
```

### **Nginx Configuration**

Create `/etc/nginx/sites-available/plant-monitor`:

```nginx
server {
    listen 80;
    server_name your-domain.com;

    location / {
        proxy_pass http://127.0.0.1:5000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

## 🔒 **Security Considerations**

### **Input Validation**
- All API inputs are validated using Marshmallow schemas
- SQL injection protection through SQLAlchemy ORM
- XSS protection through input sanitization

### **Rate Limiting**
- Built-in rate limiting to prevent abuse
- Configurable limits per endpoint
- IP-based rate limiting

### **CORS Configuration**
- Configurable CORS settings
- Secure default configuration
- Domain-specific CORS rules

### **Data Protection**
- Sensitive data encryption
- Secure session management
- Audit logging for security events

## 📊 **Performance Optimization**

### **Database Optimization**
- Connection pooling for database efficiency
- Indexed queries for fast data retrieval
- Automated data cleanup to prevent bloat

### **Caching Strategy**
- In-memory caching for frequently accessed data
- Redis integration for distributed caching
- Cache invalidation strategies

### **Load Balancing**
- Horizontal scaling support
- Load balancer configuration
- Health check endpoints

## 🐛 **Troubleshooting**

### **Common Issues**

**Server won't start:**
```bash
# Check Python version
python3 --version

# Check dependencies
pip3 list | grep -E "(Flask|SQLAlchemy)"

# Check logs
tail -f plant_monitor.log
```

**Database errors:**
```bash
# Check database file permissions
ls -la plant_monitor.db

# Recreate database
rm plant_monitor.db
python3 -c "from server import PlantMonitorServer; server = PlantMonitorServer()"
```

**API connection issues:**
```bash
# Test API locally
curl http://localhost:5000/api/health

# Check firewall settings
sudo ufw status
```

### **Logging**

The server uses structured logging with different levels:

```python
import logging
logging.basicConfig(level=logging.INFO)
```

Log files are created in the current directory:
- `plant_monitor.log` - Application logs
- `test_results.log` - Test execution logs

## 🤝 **Contributing**

### **Development Setup**

1. **Fork the repository**
2. **Create a feature branch**
3. **Make your changes**
4. **Run tests**
   ```bash
   ./run_tests.sh all
   ```
5. **Submit a pull request**

### **Code Standards**

- Follow PEP 8 style guidelines
- Add comprehensive docstrings
- Include type hints
- Write unit tests for new features
- Update documentation

### **Testing Requirements**

- All new code must have tests
- Maintain 80%+ code coverage
- Pass all linting checks
- Include integration tests for new features

## 📄 **License**

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 **Acknowledgments**

- **Flask** - Web framework
- **SQLAlchemy** - Database ORM
- **Marshmallow** - Data validation
- **Prometheus** - Metrics collection
- **Raspberry Pi Foundation** - Hardware platform

## 📞 **Support**

For support and questions:

- **Issues**: Create an issue on GitHub
- **Documentation**: Check the inline documentation
- **Tests**: Run the test suite for debugging
- **Logs**: Check application logs for errors

---

**🌱 Plant Monitor Server v2.0.0** - Professional, scalable, and secure plant monitoring solution for Raspberry Pi Zero V2. 