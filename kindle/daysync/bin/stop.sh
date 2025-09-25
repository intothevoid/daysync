#!/bin/sh

# DaySync Dashboard KUAL Extension - Stop Script

echo "Stopping DaySync dashboard..."

# Find and kill the dashboard process and its children
PID=$(pgrep -f "/bin/sh bin/start.sh")
if [ -n "$PID" ]; then
    pkill -P $PID
    kill $PID
    echo "DaySync dashboard stopped."
else
    echo "DaySync dashboard not running."
fi