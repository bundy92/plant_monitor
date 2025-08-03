#!/bin/bash

# Plant Monitor Project - Library Maintenance Setup Script
# ========================================================

set -e  # Exit on any error

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENV_PATH="$PROJECT_ROOT/venv"
SERVICE_NAME="plant-monitor-library-maintenance"

echo "=== Plant Monitor Library Maintenance Setup ==="
echo "Project root: $PROJECT_ROOT"

# Function to print colored output
print_info() {
    echo -e "\033[1;34m[INFO]\033[0m $1"
}

print_success() {
    echo -e "\033[1;32m[SUCCESS]\033[0m $1"
}

print_warning() {
    echo -e "\033[1;33m[WARNING]\033[0m $1"
}

print_error() {
    echo -e "\033[1;31m[ERROR]\033[0m $1"
}

# Check if running as root for system service setup
check_root() {
    if [[ $EUID -eq 0 ]]; then
        return 0
    else
        return 1
    fi
}

# Setup virtual environment if it doesn't exist
setup_venv() {
    print_info "Setting up Python virtual environment..."
    
    if [ ! -d "$VENV_PATH" ]; then
        python3 -m venv "$VENV_PATH"
        print_success "Virtual environment created"
    else
        print_info "Virtual environment already exists"
    fi
    
    # Activate and install dependencies
    source "$VENV_PATH/bin/activate"
    pip install --upgrade pip
    
    # Install project dependencies
    if [ -f "$PROJECT_ROOT/requirements.txt" ]; then
        pip install -r "$PROJECT_ROOT/requirements.txt"
    fi
    
    # Install PlatformIO
    pip install platformio
    
    # Install ESP32 platform
    pio pkg install --global --platform espressif32
    
    print_success "Dependencies installed"
}

# Make scripts executable
setup_permissions() {
    print_info "Setting up file permissions..."
    
    chmod +x "$PROJECT_ROOT/maintain_libraries.py"
    chmod +x "$PROJECT_ROOT/setup_library_maintenance.sh"
    
    # Make other shell scripts executable
    find "$PROJECT_ROOT" -name "*.sh" -exec chmod +x {} \;
    
    print_success "Permissions set"
}

# Create systemd service for continuous monitoring
create_systemd_service() {
    if ! check_root; then
        print_warning "Skipping systemd service creation (requires root)"
        return 0
    fi
    
    print_info "Creating systemd service for continuous monitoring..."
    
    cat > "/etc/systemd/system/${SERVICE_NAME}.service" << EOF
[Unit]
Description=Plant Monitor Library Maintenance Service
After=network.target
StartLimitIntervalSec=0

[Service]
Type=simple
Restart=always
RestartSec=1
User=$USER
WorkingDirectory=$PROJECT_ROOT
Environment=PATH=$VENV_PATH/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
ExecStart=$VENV_PATH/bin/python $PROJECT_ROOT/maintain_libraries.py monitor 24
StandardOutput=journal
StandardError=journal
SyslogIdentifier=plant-monitor-maintenance

[Install]
WantedBy=multi-user.target
EOF
    
    # Reload systemd and enable service
    systemctl daemon-reload
    systemctl enable "$SERVICE_NAME"
    
    print_success "Systemd service created and enabled"
    print_info "To start monitoring: sudo systemctl start $SERVICE_NAME"
    print_info "To check status: sudo systemctl status $SERVICE_NAME"
    print_info "To view logs: sudo journalctl -u $SERVICE_NAME -f"
}

# Create cron job for regular checks (alternative to systemd)
create_cron_job() {
    print_info "Setting up cron job for regular library checks..."
    
    # Create cron job that runs daily at 2 AM
    CRON_JOB="0 2 * * * cd $PROJECT_ROOT && $VENV_PATH/bin/python maintain_libraries.py check >> $PROJECT_ROOT/library_maintenance.log 2>&1"
    
    # Add to user's crontab if not already present
    (crontab -l 2>/dev/null | grep -v "maintain_libraries.py"; echo "$CRON_JOB") | crontab -
    
    print_success "Cron job created (daily check at 2 AM)"
}

# Create configuration file
create_config() {
    print_info "Creating configuration file..."
    
    cat > "$PROJECT_ROOT/library_maintenance.conf" << EOF
# Plant Monitor Library Maintenance Configuration
# ==============================================

[monitoring]
# Check interval in hours (for continuous monitoring)
check_interval = 24

# Enable automatic updates (true/false)
auto_update = false

# Maximum number of backups to keep
max_backups = 10

[notifications]
# Enable email notifications (requires mail setup)
enable_email = false
email_address = 

# Enable log file notifications
enable_logging = true
log_level = INFO

[compatibility]
# Minimum Python version
min_python_version = 3.8

# Critical packages that should not be auto-updated
protected_packages = Flask,Werkzeug,platformio

[testing]
# Run tests after updates
run_tests = true

# Test timeout in seconds
test_timeout = 300
EOF
    
    print_success "Configuration file created"
}

# Test the maintenance script
test_maintenance_script() {
    print_info "Testing library maintenance script..."
    
    source "$VENV_PATH/bin/activate"
    
    if python "$PROJECT_ROOT/maintain_libraries.py" check; then
        print_success "Maintenance script test passed"
    else
        print_error "Maintenance script test failed"
        return 1
    fi
}

# Main setup function
main() {
    echo "Starting setup process..."
    
    # Check system requirements
    if ! command -v python3 &> /dev/null; then
        print_error "Python 3 is required but not installed"
        exit 1
    fi
    
    if ! command -v git &> /dev/null; then
        print_error "Git is required but not installed"
        exit 1
    fi
    
    # Run setup steps
    setup_venv
    setup_permissions
    create_config
    test_maintenance_script
    
    # Ask user about monitoring setup
    echo ""
    read -p "Do you want to set up continuous monitoring? (y/n): " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        if check_root; then
            print_info "Setting up systemd service (recommended for servers)..."
            create_systemd_service
        else
            print_info "Setting up cron job (user-level monitoring)..."
            create_cron_job
        fi
    else
        print_info "Skipping continuous monitoring setup"
        print_info "You can run manual checks with: python maintain_libraries.py check"
    fi
    
    echo ""
    print_success "=== Setup Complete ==="
    echo ""
    echo "Available commands:"
    echo "  python maintain_libraries.py check    - Check library status"
    echo "  python maintain_libraries.py update   - Update libraries safely"
    echo "  python maintain_libraries.py monitor  - Run continuous monitoring"
    echo "  python maintain_libraries.py rollback <backup_name> - Rollback changes"
    echo ""
    echo "Configuration file: library_maintenance.conf"
    echo "Log file: library_maintenance.log"
    echo "Version lock file: library_versions.lock"
    echo ""
    
    if check_root && systemctl is-enabled "$SERVICE_NAME" &>/dev/null; then
        echo "Systemd service commands:"
        echo "  sudo systemctl start $SERVICE_NAME    - Start monitoring"
        echo "  sudo systemctl stop $SERVICE_NAME     - Stop monitoring"
        echo "  sudo systemctl status $SERVICE_NAME   - Check status"
        echo "  sudo journalctl -u $SERVICE_NAME -f   - View logs"
    fi
}

# Run main function
main "$@"