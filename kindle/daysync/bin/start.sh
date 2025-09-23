#!/bin/sh

# DaySync Dashboard KUAL Extension
# Usage: start.sh <update|show>

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

# Execute dashboard script
if [ "$1" = "update" ]; then
    echo "Updating DaySync dashboard..."
    $PYTHON_CMD bin/dashboard.py update
elif [ "$1" = "show" ]; then
    echo "Showing cached dashboard..."
    $PYTHON_CMD bin/dashboard.py show
else
    echo "Usage: $0 <update|show>"
    exit 1
fi
