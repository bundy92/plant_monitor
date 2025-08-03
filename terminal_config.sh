#!/bin/bash
# Terminal Configuration for ESP32 Plant Monitor
# ============================================
# This file contains terminal-specific configurations to prevent
# hanging issues when running commands in Cursor's agent mode.
#
# Based on: https://forum.cursor.com/t/cursor-agent-mode-when-running-terminal-commands-often-hangs-up-the-terminal-requiring-a-click-to-pop-it-out-in-order-to-continue-commands/59969/31

# Terminal timeout settings
export TERMINAL_TIMEOUT=120
export PIO_MONITOR_TIMEOUT=60
export UPLOAD_TIMEOUT=90

# PlatformIO monitor settings
export PIO_MONITOR_FLAGS="--timeout $PIO_MONITOR_TIMEOUT --echo --eol LF"

# Serial communication settings
export SERIAL_TIMEOUT=1000
export SERIAL_BUFFER_SIZE=1024

# Function to run commands with proper timeout handling
run_with_timeout() {
    local timeout_seconds=${1:-$TERMINAL_TIMEOUT}
    local command="$2"
    
    echo "Running command with ${timeout_seconds}s timeout: $command"
    
    if timeout "$timeout_seconds" bash -c "$command"; then
        echo "Command completed successfully"
        return 0
    else
        local exit_code=$?
        if [ $exit_code -eq 124 ]; then
            echo "Command timed out after ${timeout_seconds} seconds"
        else
            echo "Command failed with exit code $exit_code"
        fi
        return $exit_code
    fi
}

# Function to run PlatformIO commands safely
run_pio_safe() {
    local command="$1"
    local timeout_seconds=${2:-$TERMINAL_TIMEOUT}
    
    echo "Running PlatformIO command: $command"
    
    # Add PlatformIO-specific flags
    local full_command="$command"
    if [[ "$command" == *"monitor"* ]]; then
        full_command="$command $PIO_MONITOR_FLAGS"
    fi
    
    run_with_timeout "$timeout_seconds" "$full_command"
}

# Export functions for use in other scripts
export -f run_with_timeout
export -f run_pio_safe

echo "Terminal configuration loaded successfully"
echo "Timeout settings:"
echo "  - General timeout: ${TERMINAL_TIMEOUT}s"
echo "  - Monitor timeout: ${PIO_MONITOR_TIMEOUT}s"
echo "  - Upload timeout: ${UPLOAD_TIMEOUT}s" 