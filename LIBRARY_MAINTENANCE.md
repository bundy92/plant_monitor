# Library Consistency Maintenance System

This document describes the comprehensive library consistency maintenance system for the ESP32 Plant Monitor project.

## Overview

The library maintenance system automatically:
- ✅ Monitors dependency versions
- ✅ Checks for compatibility issues
- ✅ Performs safe updates with rollback capability
- ✅ Validates hardware configurations
- ✅ Runs continuous background monitoring
- ✅ Maintains version lock files for reproducible builds

## Components

### 1. Core Scripts

#### `maintain_libraries.py`
Main maintenance script with the following commands:
- `check` - Check current library status and compatibility
- `update` - Perform safe updates with automatic rollback on failure
- `monitor` - Run continuous monitoring (24-hour intervals by default)
- `rollback <backup_name>` - Rollback to a previous backup

#### `compatibility_checker.py`
Specialized compatibility checking:
- `check` - Check current system compatibility
- `matrix` - Display compatibility matrix
- `recommend` - Get optimization recommendations

#### `setup_library_maintenance.sh`
Setup script that:
- Creates Python virtual environment
- Installs all dependencies
- Sets up monitoring (systemd service or cron job)
- Creates configuration files

### 2. Configuration Files

#### `library_versions.lock`
Version lock file containing:
- PlatformIO core and platform versions
- Python package versions
- ESP-IDF component versions
- Hardware configuration
- Build tool versions

#### `library_maintenance.conf`
Configuration file with settings for:
- Monitoring intervals
- Auto-update preferences
- Backup retention
- Notification settings
- Protected packages

### 3. Monitoring Options

#### Systemd Service (Recommended for servers)
```bash
sudo systemctl start plant-monitor-library-maintenance
sudo systemctl status plant-monitor-library-maintenance
sudo journalctl -u plant-monitor-library-maintenance -f
```

#### Cron Job (User-level)
Runs daily checks at 2 AM and logs to `library_maintenance.log`

## Installation & Setup

### Quick Setup
```bash
# Run the setup script
./setup_library_maintenance.sh

# Follow the prompts to configure monitoring
```

### Manual Setup
```bash
# Create virtual environment
python3 -m venv venv
source venv/bin/activate

# Install dependencies
pip install --upgrade pip
pip install -r requirements.txt
pip install platformio

# Install ESP32 platform
pio pkg install --global --platform espressif32

# Make scripts executable
chmod +x maintain_libraries.py compatibility_checker.py
```

## Usage Examples

### Daily Operations

#### Check System Status
```bash
python maintain_libraries.py check
```
Output:
```
=== Library Status Check ===
PlatformIO Core: 6.1.18
ESP32 Platform: 6.12.0
Python Packages: 22 installed
Python package updates available: 3
  - requests: 2.32.4 -> 2.32.5
  - urllib3: 2.5.0 -> 2.5.1
No compatibility issues found
```

#### Perform Safe Updates
```bash
python maintain_libraries.py update
```
This will:
1. Create automatic backup
2. Update dependencies
3. Run compatibility checks
4. Execute tests
5. Rollback if any issues occur

#### Check Compatibility
```bash
python compatibility_checker.py check
```

#### Get Recommendations
```bash
python compatibility_checker.py recommend
```

### Monitoring Operations

#### Start Continuous Monitoring
```bash
# Using systemd (requires root)
sudo systemctl start plant-monitor-library-maintenance

# Or run manually
python maintain_libraries.py monitor 24  # Check every 24 hours
```

#### View Monitoring Logs
```bash
# Systemd logs
sudo journalctl -u plant-monitor-library-maintenance -f

# File logs
tail -f library_maintenance.log
```

### Recovery Operations

#### List Available Backups
```bash
ls .library_backups/
```

#### Rollback to Previous State
```bash
python maintain_libraries.py rollback backup_20250803_090000
```

## Compatibility Matrix

### PlatformIO Core 6.1.18
- **Python:** 3.8 - 3.13
- **ESP32 Platform:** 6.12.0, 6.11.0, 6.10.0
- **Status:** Stable

### ESP32 Platform 6.12.0
- **ESP-IDF:** 5.1.4
- **Toolchain:** 8.4.0+2021r2-patch5
- **Boards:** esp32-c6-devkitc-1, esp32-s3-devkitc-1
- **Status:** Stable

### Hardware: ESP32-C6-DevKitC-1
- **I2C SDA Pins:** 21 (recommended), 19, 18
- **I2C SCL Pins:** 22 (recommended), 20, 17
- **ADC Channels:** 0, 1, 2, 3, 4, 5, 6
- **Compatible Sensors:** AHT10, AHT20, DHT22
- **Voltage:** 3.3V

### Python Packages
- **Flask 2.3.3** ↔ **Werkzeug 2.3.7** ✅ Compatible
- **Python 3.13.3** ↔ **PlatformIO 6.1.18** ✅ Compatible

## File Structure

```
project/
├── maintain_libraries.py          # Main maintenance script
├── compatibility_checker.py       # Compatibility checker
├── setup_library_maintenance.sh   # Setup script
├── library_versions.lock          # Version lock file
├── library_maintenance.conf       # Configuration
├── library_maintenance.log        # Log file
├── .library_backups/              # Backup directory
│   ├── backup_20250803_090000/
│   └── backup_20250803_120000/
└── venv/                          # Python virtual environment
```

## Configuration Options

### Monitoring Settings
```ini
[monitoring]
check_interval = 24          # Hours between checks
auto_update = false          # Enable automatic updates
max_backups = 10            # Maximum backups to keep
```

### Notifications
```ini
[notifications]
enable_email = false        # Email notifications
enable_logging = true       # Log file notifications
log_level = INFO           # Logging level
```

### Compatibility
```ini
[compatibility]
min_python_version = 3.8   # Minimum Python version
protected_packages = Flask,Werkzeug,platformio  # No auto-update
```

## Safety Features

### Automatic Backups
- Created before any update operation
- Include requirements.txt, platformio.ini, and version snapshots
- Timestamped for easy identification
- Automatic cleanup of old backups

### Rollback Capability
- Complete restoration of previous state
- Reinstalls exact package versions
- Restores configuration files
- Validates rollback success

### Testing Integration
- Runs project tests after updates
- Validates PlatformIO project integrity
- Checks server functionality
- Automatic rollback on test failures

### Compatibility Validation
- Cross-checks all component versions
- Hardware configuration validation
- Known incompatibility detection
- Recommendation engine for optimal configurations

## Troubleshooting

### Common Issues

#### Virtual Environment Not Found
```bash
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

#### Permission Denied for Systemd Service
```bash
sudo ./setup_library_maintenance.sh
```

#### PlatformIO Not Found
```bash
source venv/bin/activate
pip install platformio
```

#### Backup Restoration Failed
```bash
# Check backup integrity
ls -la .library_backups/backup_YYYYMMDD_HHMMSS/

# Manual restoration
cp .library_backups/backup_YYYYMMDD_HHMMSS/requirements.txt .
pip install -r requirements.txt
```

### Log Analysis

#### Check Recent Activity
```bash
tail -n 50 library_maintenance.log
```

#### Monitor Live Updates
```bash
tail -f library_maintenance.log
```

#### Filter by Level
```bash
grep "ERROR" library_maintenance.log
grep "WARNING" library_maintenance.log
```

## Best Practices

### Regular Maintenance
1. Run weekly compatibility checks
2. Review update notifications
3. Test updates in development environment first
4. Keep at least 5 recent backups

### Update Strategy
1. Update Python packages first
2. Update PlatformIO core second
3. Update ESP32 platform last
4. Always run full test suite after updates

### Monitoring
1. Enable continuous monitoring for production systems
2. Set up log rotation for long-running systems
3. Monitor disk space for backup directory
4. Review compatibility reports monthly

## Integration with CI/CD

### GitHub Actions Example
```yaml
name: Library Maintenance
on:
  schedule:
    - cron: '0 2 * * 0'  # Weekly on Sunday at 2 AM

jobs:
  maintain:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Setup Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.11'
      - name: Run Library Check
        run: |
          ./setup_library_maintenance.sh
          python maintain_libraries.py check
```

## Support

For issues or questions about the library maintenance system:

1. Check the troubleshooting section above
2. Review log files for error details
3. Verify virtual environment is properly activated
4. Ensure all dependencies are installed

## Version History

- **v1.0.0** - Initial release with basic monitoring
- **v1.1.0** - Added compatibility checking
- **v1.2.0** - Implemented safe update with rollback
- **v1.3.0** - Added hardware configuration validation
- **v1.4.0** - Integrated continuous monitoring system

---

This library maintenance system ensures your ESP32 Plant Monitor project remains stable and up-to-date while minimizing the risk of compatibility issues and system failures.