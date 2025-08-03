#!/usr/bin/env python3
"""
ESP32 Plant Monitor - Compatibility Checker
==========================================

This script checks compatibility between different components:
- ESP-IDF framework versions
- PlatformIO platform versions
- Python package versions
- Hardware-specific requirements

Usage:
    python compatibility_checker.py check        - Check current compatibility
    python compatibility_checker.py matrix       - Show compatibility matrix
    python compatibility_checker.py recommend    - Get recommendations
"""

import json
import sys
import subprocess
from pathlib import Path
from typing import Dict, List, Tuple, Optional
import logging

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')
logger = logging.getLogger(__name__)

class CompatibilityChecker:
    def __init__(self):
        self.project_root = Path.cwd()
        self.venv_path = self.project_root / "venv"
        
        # Known compatibility matrix
        self.compatibility_matrix = {
            "platformio_core": {
                "6.1.18": {
                    "python_min": "3.8",
                    "python_max": "3.13",
                    "esp32_platform": ["6.12.0", "6.11.0", "6.10.0"],
                    "status": "stable"
                }
            },
            "esp32_platform": {
                "6.12.0": {
                    "esp_idf": "5.1.4",
                    "toolchain": "8.4.0+2021r2-patch5",
                    "boards": ["esp32-c6-devkitc-1", "esp32-s3-devkitc-1"],
                    "status": "stable"
                }
            },
            "python_packages": {
                "Flask": {
                    "2.3.3": {
                        "werkzeug": ["2.3.7", "2.3.6"],
                        "python": ">=3.8",
                        "status": "stable"
                    }
                },
                "Werkzeug": {
                    "2.3.7": {
                        "flask": ["2.3.3", "2.3.2"],
                        "python": ">=3.8",
                        "status": "stable"
                    }
                }
            },
            "hardware": {
                "esp32-c6-devkitc-1": {
                    "i2c_pins": {
                        "sda": [21, 19, 18],
                        "scl": [22, 20, 17]
                    },
                    "adc_channels": [0, 1, 2, 3, 4, 5, 6],
                    "gpio_available": list(range(0, 22)),
                    "voltage": "3.3V",
                    "sensors_compatible": ["AHT10", "AHT20", "DHT22"]
                }
            }
        }
    
    def run_command(self, cmd: str, use_venv: bool = True) -> Tuple[int, str, str]:
        """Run a command and return exit code, stdout, stderr"""
        if use_venv and self.venv_path.exists():
            activate_script = self.venv_path / "bin" / "activate"
            cmd = f"source {activate_script} && {cmd}"
        
        try:
            result = subprocess.run(
                cmd, 
                shell=True, 
                cwd=self.project_root,
                capture_output=True, 
                text=True,
                timeout=60
            )
            return result.returncode, result.stdout, result.stderr
        except subprocess.TimeoutExpired:
            return -1, "", "Command timed out"
        except Exception as e:
            return -1, "", str(e)
    
    def get_current_versions(self) -> Dict:
        """Get current installed versions"""
        versions = {}
        
        # Get PlatformIO version
        exit_code, stdout, stderr = self.run_command("pio --version")
        if exit_code == 0:
            versions['platformio_core'] = stdout.strip().split()[-1]
        
        # Get ESP32 platform version
        exit_code, stdout, stderr = self.run_command("pio pkg list --global")
        if exit_code == 0:
            for line in stdout.split('\n'):
                if 'espressif32' in line and '@' in line:
                    versions['esp32_platform'] = line.split('@')[-1].strip()
        
        # Get Python version
        exit_code, stdout, stderr = self.run_command("python --version")
        if exit_code == 0:
            versions['python'] = stdout.strip().split()[-1]
        
        # Get Python packages
        exit_code, stdout, stderr = self.run_command("pip freeze")
        if exit_code == 0:
            packages = {}
            for line in stdout.strip().split('\n'):
                if '==' in line:
                    package, version = line.split('==')
                    packages[package] = version
            versions['python_packages'] = packages
        
        return versions
    
    def check_version_compatibility(self, component: str, version: str, target_component: str, target_version: str) -> bool:
        """Check if two component versions are compatible"""
        if component not in self.compatibility_matrix:
            logger.warning(f"No compatibility data for {component}")
            return True  # Assume compatible if no data
        
        if version not in self.compatibility_matrix[component]:
            logger.warning(f"No compatibility data for {component} {version}")
            return True
        
        compat_data = self.compatibility_matrix[component][version]
        
        if target_component in compat_data:
            if isinstance(compat_data[target_component], list):
                return target_version in compat_data[target_component]
            else:
                return target_version == compat_data[target_component]
        
        return True  # Assume compatible if no specific restriction
    
    def check_python_version_range(self, version: str, min_version: str, max_version: str = None) -> bool:
        """Check if Python version is within range"""
        def version_tuple(v):
            return tuple(map(int, v.split('.')))
        
        current = version_tuple(version)
        minimum = version_tuple(min_version)
        
        if current < minimum:
            return False
        
        if max_version:
            maximum = version_tuple(max_version)
            if current > maximum:
                return False
        
        return True
    
    def check_hardware_compatibility(self, board: str, config: Dict) -> List[str]:
        """Check hardware configuration compatibility"""
        issues = []
        
        if board not in self.compatibility_matrix["hardware"]:
            issues.append(f"Unknown board: {board}")
            return issues
        
        hw_spec = self.compatibility_matrix["hardware"][board]
        
        # Check I2C pin configuration
        if "i2c_pins" in config:
            sda_pin = config["i2c_pins"].get("sda")
            scl_pin = config["i2c_pins"].get("scl")
            
            if sda_pin and sda_pin not in hw_spec["i2c_pins"]["sda"]:
                issues.append(f"SDA pin {sda_pin} not recommended for {board}")
            
            if scl_pin and scl_pin not in hw_spec["i2c_pins"]["scl"]:
                issues.append(f"SCL pin {scl_pin} not recommended for {board}")
        
        # Check ADC channels
        if "adc_channels" in config:
            for channel in config["adc_channels"]:
                if channel not in hw_spec["adc_channels"]:
                    issues.append(f"ADC channel {channel} not available on {board}")
        
        # Check sensor compatibility
        if "sensors" in config:
            for sensor in config["sensors"]:
                if sensor not in hw_spec["sensors_compatible"]:
                    issues.append(f"Sensor {sensor} compatibility not verified for {board}")
        
        return issues
    
    def check_current_compatibility(self) -> Dict:
        """Check compatibility of current installation"""
        logger.info("Checking current system compatibility...")
        
        versions = self.get_current_versions()
        issues = []
        warnings = []
        
        # Check PlatformIO core compatibility
        if 'platformio_core' in versions:
            pio_version = versions['platformio_core']
            if pio_version in self.compatibility_matrix['platformio_core']:
                pio_spec = self.compatibility_matrix['platformio_core'][pio_version]
                
                # Check Python version compatibility
                if 'python' in versions:
                    python_version = versions['python']
                    if not self.check_python_version_range(
                        python_version, 
                        pio_spec['python_min'], 
                        pio_spec.get('python_max')
                    ):
                        issues.append(f"Python {python_version} not compatible with PlatformIO {pio_version}")
                
                # Check ESP32 platform compatibility
                if 'esp32_platform' in versions:
                    esp_version = versions['esp32_platform']
                    if esp_version not in pio_spec['esp32_platform']:
                        warnings.append(f"ESP32 platform {esp_version} not in tested versions for PlatformIO {pio_version}")
        
        # Check Flask/Werkzeug compatibility
        packages = versions.get('python_packages', {})
        flask_version = packages.get('Flask')
        werkzeug_version = packages.get('Werkzeug')
        
        if flask_version and werkzeug_version:
            if not self.check_version_compatibility('python_packages', f'Flask.{flask_version}', 'werkzeug', werkzeug_version):
                issues.append(f"Flask {flask_version} may not be compatible with Werkzeug {werkzeug_version}")
        
        # Check hardware configuration
        hw_issues = self.check_project_hardware_config()
        issues.extend(hw_issues)
        
        return {
            'versions': versions,
            'issues': issues,
            'warnings': warnings,
            'status': 'compatible' if not issues else 'incompatible'
        }
    
    def check_project_hardware_config(self) -> List[str]:
        """Check project's hardware configuration"""
        issues = []
        
        # Read config.h for hardware configuration
        config_file = self.project_root / "config.h"
        if not config_file.exists():
            return ["config.h not found"]
        
        try:
            with open(config_file, 'r') as f:
                content = f.read()
            
            # Extract hardware configuration
            config = {
                "i2c_pins": {},
                "adc_channels": [],
                "sensors": ["AHT10"]  # Known from project
            }
            
            # Parse I2C pins
            for line in content.split('\n'):
                if 'I2C_MASTER_SDA_IO' in line and 'GPIO_NUM_' in line:
                    pin = int(line.split('GPIO_NUM_')[1].split()[0])
                    config["i2c_pins"]["sda"] = pin
                elif 'I2C_MASTER_SCL_IO' in line and 'GPIO_NUM_' in line:
                    pin = int(line.split('GPIO_NUM_')[1].split()[0])
                    config["i2c_pins"]["scl"] = pin
                elif 'ADC_CHANNEL_' in line:
                    channel = int(line.split('ADC_CHANNEL_')[1].split()[0])
                    config["adc_channels"].append(channel)
            
            # Check hardware compatibility
            issues = self.check_hardware_compatibility("esp32-c6-devkitc-1", config)
            
        except Exception as e:
            issues.append(f"Error reading hardware configuration: {e}")
        
        return issues
    
    def get_recommendations(self) -> Dict:
        """Get recommendations for optimal configuration"""
        current = self.check_current_compatibility()
        recommendations = {
            'updates': [],
            'configuration': [],
            'alternatives': []
        }
        
        # Recommend updates for compatibility issues
        for issue in current['issues']:
            if 'Python' in issue and 'PlatformIO' in issue:
                recommendations['updates'].append("Consider updating Python to 3.11 or 3.12 for best PlatformIO compatibility")
            elif 'Flask' in issue and 'Werkzeug' in issue:
                recommendations['updates'].append("Update Flask and Werkzeug together to maintain compatibility")
        
        # Configuration recommendations
        recommendations['configuration'].extend([
            "Use GPIO 21 (SDA) and GPIO 22 (SCL) for I2C on ESP32-C6",
            "Keep I2C frequency at 100kHz for reliable AHT10 communication",
            "Use ADC channels 0-3 for analog sensors on ESP32-C6"
        ])
        
        # Alternative suggestions
        if current['warnings']:
            recommendations['alternatives'].append("Consider using PlatformIO platform version 6.12.0 for best stability")
        
        return recommendations
    
    def show_compatibility_matrix(self):
        """Display the compatibility matrix"""
        print("=== ESP32 Plant Monitor Compatibility Matrix ===\n")
        
        print("PlatformIO Core Compatibility:")
        for version, spec in self.compatibility_matrix['platformio_core'].items():
            print(f"  {version}:")
            print(f"    Python: {spec['python_min']} - {spec.get('python_max', 'latest')}")
            print(f"    ESP32 Platform: {', '.join(spec['esp32_platform'])}")
            print(f"    Status: {spec['status']}")
        
        print("\nESP32 Platform Compatibility:")
        for version, spec in self.compatibility_matrix['esp32_platform'].items():
            print(f"  {version}:")
            print(f"    ESP-IDF: {spec['esp_idf']}")
            print(f"    Toolchain: {spec['toolchain']}")
            print(f"    Boards: {', '.join(spec['boards'])}")
        
        print("\nHardware Specifications:")
        for board, spec in self.compatibility_matrix['hardware'].items():
            print(f"  {board}:")
            print(f"    I2C SDA pins: {spec['i2c_pins']['sda']}")
            print(f"    I2C SCL pins: {spec['i2c_pins']['scl']}")
            print(f"    ADC channels: {spec['adc_channels']}")
            print(f"    Compatible sensors: {', '.join(spec['sensors_compatible'])}")

def main():
    checker = CompatibilityChecker()
    
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    
    command = sys.argv[1].lower()
    
    if command == "check":
        result = checker.check_current_compatibility()
        
        print("=== Compatibility Check Results ===")
        print(f"Status: {result['status'].upper()}")
        print()
        
        print("Current Versions:")
        for component, version in result['versions'].items():
            if isinstance(version, dict):
                print(f"  {component}: {len(version)} packages")
            else:
                print(f"  {component}: {version}")
        print()
        
        if result['issues']:
            print("Issues Found:")
            for issue in result['issues']:
                print(f"  ❌ {issue}")
            print()
        
        if result['warnings']:
            print("Warnings:")
            for warning in result['warnings']:
                print(f"  ⚠️  {warning}")
            print()
        
        if not result['issues'] and not result['warnings']:
            print("✅ All components are compatible!")
    
    elif command == "matrix":
        checker.show_compatibility_matrix()
    
    elif command == "recommend":
        recommendations = checker.get_recommendations()
        
        print("=== Compatibility Recommendations ===")
        
        if recommendations['updates']:
            print("\nRecommended Updates:")
            for rec in recommendations['updates']:
                print(f"  • {rec}")
        
        if recommendations['configuration']:
            print("\nConfiguration Recommendations:")
            for rec in recommendations['configuration']:
                print(f"  • {rec}")
        
        if recommendations['alternatives']:
            print("\nAlternative Options:")
            for rec in recommendations['alternatives']:
                print(f"  • {rec}")
    
    else:
        print("Unknown command. Use: check, matrix, or recommend")
        sys.exit(1)

if __name__ == "__main__":
    main()