#!/bin/sh

# DaySync Dashboard KUAL Extension

EXTENSION_DIR="/mnt/us/extensions/daysync"

# Change to extension directory
cd "$EXTENSION_DIR" || exit 1

# Source the configuration
. bin/config.sh

# File to store the index of the next endpoint
ENDPOINT_INDEX_FILE="$EXTENSION_DIR/cache/endpoint_index.txt"

if [ ! -f "$ENDPOINT_INDEX_FILE" ]; then
    echo 0 > "$ENDPOINT_INDEX_FILE"
fi

while true; do
    # Read the index and increment it
    ENDPOINT_INDEX=$(cat "$ENDPOINT_INDEX_FILE")
    ENDPOINT=${ENDPOINTS[$ENDPOINT_INDEX]}
    
    # Fetch the image
    echo "Fetching image for $ENDPOINT..."
    curl -s -o "$EXTENSION_DIR/cache/dashboard.png" "$BASE_URL/$ENDPOINT"
    
    # Display the image
    echo "Displaying image..."
    eips -g "$EXTENSION_DIR/cache/dashboard.png"
    
    # Update the index
    NEXT_ENDPOINT_INDEX=$(( (ENDPOINT_INDEX + 1) % ${#ENDPOINTS[@]} ))
    echo $NEXT_ENDPOINT_INDEX > "$ENDPOINT_INDEX_FILE"
    
    # Wait for 5 minutes
    echo "Waiting for 5 minutes..."
    sleep 300
done
