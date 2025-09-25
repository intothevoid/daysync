#!/bin/sh

# DaySync Dashboard KUAL Extension - Stop Script

echo "Stopping DaySync dashboard..."

# Find and kill the dashboard process
PID=$(pgrep -f "python.*bin/dashboard.py start")
if [ -n "$PID" ]; then
    kill $PID
    echo "DaySync dashboard stopped."
else
    echo "DaySync dashboard not running."
fi