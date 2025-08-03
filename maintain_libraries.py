#!/usr/bin/env python3
"""
Plant Monitor Project - Library Consistency Maintenance Script
=============================================================

This script maintains library consistency by:
1. Monitoring current versions
2. Checking for updates
3. Validating compatibility
4. Performing safe updates with rollback capability
5. Running tests after updates

Usage:
    python maintain_libraries.py check    - Check current status
    python maintain_libraries.py update   - Update all dependencies safely
    python maintain_libraries.py monitor  - Run continuous monitoring
    python maintain_libraries.py rollback - Rollback to last known good state
"""

import os
import sys
import json
import subprocess
import time
import logging
from datetime import datetime
from pathlib import Path
import configparser
import shutil
from typing import Dict, List, Tuple, Optional

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('library_maintenance.log'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

class LibraryMaintainer:
    def __init__(self):
        self.project_root = Path.cwd()
        self.venv_path = self.project_root / "venv"
        self.lock_file = self.project_root / "library_versions.lock"
        self.backup_dir = self.project_root / ".library_backups"
        self.backup_dir.mkdir(exist_ok=True)
        
        # Ensure virtual environment is activated
        self.ensure_venv()
    
    def ensure_venv(self):
        """Ensure virtual environment is available and activated"""
        if not self.venv_path.exists():
            logger.error("Virtual environment not found. Please run setup first.")
            sys.exit(1)
    
    def run_command(self, cmd: str, cwd: Optional[Path] = None, use_venv: bool = True) -> Tuple[int, str, str]:
        """Run a command and return exit code, stdout, stderr"""
        if use_venv and self.venv_path.exists():
            # Activate virtual environment
            activate_script = self.venv_path / "bin" / "activate"
            cmd = f"source {activate_script} && {cmd}"
        
        try:
            result = subprocess.run(
                cmd, 
                shell=True, 
                cwd=cwd or self.project_root,
                capture_output=True, 
                text=True,
                timeout=300  # 5 minute timeout
            )
            return result.returncode, result.stdout, result.stderr
        except subprocess.TimeoutExpired:
            logger.error(f"Command timed out: {cmd}")
            return -1, "", "Command timed out"
        except Exception as e:
            logger.error(f"Error running command '{cmd}': {e}")
            return -1, "", str(e)
    
    def get_current_versions(self) -> Dict:
        """Get current versions of all dependencies"""
        versions = {
            'timestamp': datetime.now().isoformat(),
            'platformio': {},
            'python_packages': {},
            'esp_tools': {},
            'system': {}
        }
        
        # Get PlatformIO version
        exit_code, stdout, stderr = self.run_command("pio --version")
        if exit_code == 0:
            versions['platformio']['core'] = stdout.strip().split()[-1]
        
        # Get Python package versions
        exit_code, stdout, stderr = self.run_command("pip freeze")
        if exit_code == 0:
            for line in stdout.strip().split('\n'):
                if '==' in line:
                    package, version = line.split('==')
                    versions['python_packages'][package] = version
        
        # Get ESP32 platform version
        exit_code, stdout, stderr = self.run_command("pio pkg list --global")
        if exit_code == 0:
            # Parse platform versions from output
            for line in stdout.split('\n'):
                if 'espressif32' in line and '@' in line:
                    version = line.split('@')[-1].strip()
                    versions['platformio']['espressif32'] = version
        
        # Get system versions
        exit_code, stdout, stderr = self.run_command("python3 --version", use_venv=False)
        if exit_code == 0:
            versions['system']['python'] = stdout.strip().split()[-1]
        
        return versions
    
    def check_for_updates(self) -> Dict:
        """Check for available updates"""
        updates = {
            'python_packages': [],
            'platformio_core': None,
            'esp32_platform': None
        }
        
        # Check Python package updates
        exit_code, stdout, stderr = self.run_command("pip list --outdated --format=json")
        if exit_code == 0:
            try:
                outdated = json.loads(stdout)
                updates['python_packages'] = outdated
            except json.JSONDecodeError:
                logger.warning("Could not parse pip outdated output")
        
        # Check PlatformIO core updates
        exit_code, stdout, stderr = self.run_command("pio upgrade --dry-run")
        if exit_code == 0 and "new version" in stdout.lower():
            updates['platformio_core'] = "Available"
        
        return updates
    
    def validate_compatibility(self, versions: Dict) -> List[str]:
        """Validate compatibility between different components"""
        issues = []
        
        # Check Python version compatibility
        python_version = versions.get('system', {}).get('python', '')
        if python_version:
            major, minor = python_version.split('.')[:2]
            if int(major) < 3 or (int(major) == 3 and int(minor) < 8):
                issues.append(f"Python {python_version} may not be compatible with latest PlatformIO")
        
        # Check Flask version compatibility
        flask_version = versions.get('python_packages', {}).get('Flask', '')
        werkzeug_version = versions.get('python_packages', {}).get('Werkzeug', '')
        
        if flask_version and werkzeug_version:
            flask_major = int(flask_version.split('.')[0])
            werkzeug_major = int(werkzeug_version.split('.')[0])
            
            if flask_major == 2 and werkzeug_major > 2:
                issues.append(f"Flask {flask_version} may not be compatible with Werkzeug {werkzeug_version}")
        
        return issues
    
    def create_backup(self) -> str:
        """Create backup of current state"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        backup_name = f"backup_{timestamp}"
        backup_path = self.backup_dir / backup_name
        backup_path.mkdir(exist_ok=True)
        
        # Backup requirements.txt
        if (self.project_root / "requirements.txt").exists():
            shutil.copy2(self.project_root / "requirements.txt", backup_path / "requirements.txt")
        
        # Backup platformio.ini
        if (self.project_root / "platformio.ini").exists():
            shutil.copy2(self.project_root / "platformio.ini", backup_path / "platformio.ini")
        
        # Backup current versions
        current_versions = self.get_current_versions()
        with open(backup_path / "versions.json", 'w') as f:
            json.dump(current_versions, f, indent=2)
        
        logger.info(f"Backup created: {backup_name}")
        return backup_name
    
    def update_python_packages(self, packages: List[str] = None) -> bool:
        """Update Python packages safely"""
        if packages is None:
            # Update all packages
            exit_code, stdout, stderr = self.run_command("pip install --upgrade -r requirements.txt")
        else:
            # Update specific packages
            for package in packages:
                exit_code, stdout, stderr = self.run_command(f"pip install --upgrade {package}")
                if exit_code != 0:
                    logger.error(f"Failed to update {package}: {stderr}")
                    return False
        
        return exit_code == 0
    
    def update_platformio(self) -> bool:
        """Update PlatformIO core and platform"""
        # Update PlatformIO core
        exit_code, stdout, stderr = self.run_command("pio upgrade")
        if exit_code != 0:
            logger.error(f"Failed to update PlatformIO core: {stderr}")
            return False
        
        # Update ESP32 platform
        exit_code, stdout, stderr = self.run_command("pio pkg update --global")
        if exit_code != 0:
            logger.error(f"Failed to update ESP32 platform: {stderr}")
            return False
        
        return True
    
    def run_tests(self) -> bool:
        """Run project tests to validate updates"""
        test_commands = [
            "python raspberry_pi_server.py --test",  # Test server
            "pio check",  # Check PlatformIO project
        ]
        
        for cmd in test_commands:
            if cmd.startswith("python") and not (self.project_root / cmd.split()[1]).exists():
                continue  # Skip if file doesn't exist
            
            exit_code, stdout, stderr = self.run_command(cmd)
            if exit_code != 0:
                logger.error(f"Test failed: {cmd}")
                logger.error(f"Error: {stderr}")
                return False
        
        return True
    
    def rollback(self, backup_name: str) -> bool:
        """Rollback to a previous backup"""
        backup_path = self.backup_dir / backup_name
        if not backup_path.exists():
            logger.error(f"Backup {backup_name} not found")
            return False
        
        try:
            # Restore requirements.txt
            if (backup_path / "requirements.txt").exists():
                shutil.copy2(backup_path / "requirements.txt", self.project_root / "requirements.txt")
                # Reinstall packages
                self.run_command("pip install -r requirements.txt")
            
            # Restore platformio.ini
            if (backup_path / "platformio.ini").exists():
                shutil.copy2(backup_path / "platformio.ini", self.project_root / "platformio.ini")
            
            logger.info(f"Rollback to {backup_name} completed")
            return True
        except Exception as e:
            logger.error(f"Rollback failed: {e}")
            return False
    
    def monitor_continuously(self, interval_hours: int = 24):
        """Run continuous monitoring"""
        logger.info(f"Starting continuous monitoring (checking every {interval_hours} hours)")
        
        while True:
            try:
                logger.info("Running dependency check...")
                self.check_status()
                
                # Sleep for specified interval
                time.sleep(interval_hours * 3600)
            except KeyboardInterrupt:
                logger.info("Monitoring stopped by user")
                break
            except Exception as e:
                logger.error(f"Error in monitoring loop: {e}")
                time.sleep(300)  # Wait 5 minutes before retrying
    
    def check_status(self):
        """Check current status and report"""
        logger.info("=== Library Status Check ===")
        
        # Get current versions
        current_versions = self.get_current_versions()
        
        # Check for updates
        updates = self.check_for_updates()
        
        # Validate compatibility
        issues = self.validate_compatibility(current_versions)
        
        # Report status
        logger.info(f"PlatformIO Core: {current_versions['platformio'].get('core', 'Unknown')}")
        logger.info(f"ESP32 Platform: {current_versions['platformio'].get('espressif32', 'Unknown')}")
        logger.info(f"Python Packages: {len(current_versions['python_packages'])} installed")
        
        if updates['python_packages']:
            logger.info(f"Python package updates available: {len(updates['python_packages'])}")
            for pkg in updates['python_packages'][:5]:  # Show first 5
                logger.info(f"  - {pkg['name']}: {pkg['version']} -> {pkg['latest_version']}")
        
        if updates['platformio_core']:
            logger.info("PlatformIO core update available")
        
        if issues:
            logger.warning("Compatibility issues found:")
            for issue in issues:
                logger.warning(f"  - {issue}")
        else:
            logger.info("No compatibility issues found")
    
    def safe_update(self):
        """Perform safe update with backup and testing"""
        logger.info("=== Starting Safe Update Process ===")
        
        # Create backup
        backup_name = self.create_backup()
        
        try:
            # Check current status
            self.check_status()
            
            # Update Python packages
            logger.info("Updating Python packages...")
            if not self.update_python_packages():
                raise Exception("Python package update failed")
            
            # Update PlatformIO
            logger.info("Updating PlatformIO...")
            if not self.update_platformio():
                raise Exception("PlatformIO update failed")
            
            # Run tests
            logger.info("Running tests...")
            if not self.run_tests():
                raise Exception("Tests failed after update")
            
            # Update lock file
            self.update_lock_file()
            
            logger.info("=== Update completed successfully ===")
            
        except Exception as e:
            logger.error(f"Update failed: {e}")
            logger.info("Rolling back changes...")
            if self.rollback(backup_name):
                logger.info("Rollback completed successfully")
            else:
                logger.error("Rollback failed - manual intervention required")
    
    def update_lock_file(self):
        """Update the version lock file with current versions"""
        current_versions = self.get_current_versions()
        
        # Update lock file timestamp
        with open(self.lock_file, 'r') as f:
            content = f.read()
        
        # Replace timestamp
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        content = content.replace("$(date)", timestamp)
        
        with open(self.lock_file, 'w') as f:
            f.write(content)
        
        logger.info("Lock file updated")

def main():
    maintainer = LibraryMaintainer()
    
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    
    command = sys.argv[1].lower()
    
    if command == "check":
        maintainer.check_status()
    elif command == "update":
        maintainer.safe_update()
    elif command == "monitor":
        interval = int(sys.argv[2]) if len(sys.argv) > 2 else 24
        maintainer.monitor_continuously(interval)
    elif command == "rollback":
        if len(sys.argv) < 3:
            logger.error("Please specify backup name")
            sys.exit(1)
        maintainer.rollback(sys.argv[2])
    else:
        print("Unknown command. Use: check, update, monitor, or rollback")
        sys.exit(1)

if __name__ == "__main__":
    main()