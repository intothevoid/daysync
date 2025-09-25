#!/bin/sh

# DaySync Dashboard KUAL Extension

EXTENSION_DIR="/mnt/us/extensions/daysync"
PYTHON_CMD="python3"

# Check if Python is available
if ! command -v $PYTHON_CMD >/dev/null 2>&1; then
    echo "Python3 not found, trying python..."
    PYTHON_CMD="python"
    if ! command -v $PYTHON_CMD >/dev/null 2>&1; then
        echo "ERROR: Python not found on system"
        exit 1
    fi
fi

# Change to extension directory
cd "$EXTENSION_DIR" || exit 1

# Make sure scripts are executable
chmod +x bin/stop.sh
chmod +x bin/dashboard.py

# Execute dashboard script in the background
echo "Starting DaySync dashboard..."
$PYTHON_CMD bin/dashboard.py start &