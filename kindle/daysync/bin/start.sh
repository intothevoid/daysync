#!/bin/sh
set -x # Keep set -x for now to debug this change

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
    # Read the index
    ENDPOINT_INDEX=$(cat "$ENDPOINT_INDEX_FILE")

    # Get the Nth word from the ENDPOINTS string (1-indexed for awk)
    # Add 1 to ENDPOINT_INDEX because awk is 1-indexed
    ENDPOINT=$(echo "$ENDPOINTS" | awk "{print \$((ENDPOINT_INDEX + 1))}")
    
    # Fetch the image
    echo "Fetching image for $ENDPOINT..."
    curl -s -o "$EXTENSION_DIR/cache/dashboard.png" "$BASE_URL/$ENDPOINT"
    
    # Display the image
    echo "Displaying image..."
    /usr/sbin/eips -g "$EXTENSION_DIR/cache/dashboard.png"
    
    # Update the index
    # Calculate the total number of endpoints by counting words in the string
    NUM_ENDPOINTS=$(echo "$ENDPOINTS" | wc -w)
    NEXT_ENDPOINT_INDEX=$(( (ENDPOINT_INDEX + 1) % NUM_ENDPOINTS ))
    echo "$NEXT_ENDPOINT_INDEX" > "$ENDPOINT_INDEX_FILE"
    
    # Wait for 5 minutes
    echo "Waiting for 5 minutes..."
    sleep 300
done