#!/usr/bin/env python3

import os
import sys
import time
import requests
import subprocess

# Configuration
CONFIG = {
    "base_url": "http://localhost:8080/api/kindle",  # Update with your DaySync server IP
    "endpoints": ["weather", "motogp", "formula1", "crypto", "news", "stock"],
    "cache_dir": "/mnt/us/extensions/daysync/cache/",
    "png_file": "/mnt/us/extensions/daysync/cache/dashboard.png",
    "endpoint_index_file": "/mnt/us/extensions/daysync/cache/endpoint_index.txt"
}

def display_image():
    """Display the generated image on Kindle screen"""
    print("Displaying dashboard on Kindle...")
    
    if os.path.exists(CONFIG["png_file"]):
        # Use eips to display on Kindle screen
        cmd = f"eips -g {CONFIG['png_file']}"
        try:
            subprocess.run(cmd.split(), check=True)
            print("Dashboard displayed successfully")
            return True
        except Exception as e:
            print(f"Error displaying image: {e}")
    
    return False

def get_next_endpoint_index():
    """Get the index of the next endpoint to fetch."""
    if not os.path.exists(CONFIG["endpoint_index_file"]):
        return 0
    
    with open(CONFIG["endpoint_index_file"], 'r') as f:
        try:
            index = int(f.read())
            return (index + 1) % len(CONFIG["endpoints"])
        except (ValueError, IndexError):
            return 0

def save_endpoint_index(index):
    """Save the index of the last fetched endpoint."""
    with open(CONFIG["endpoint_index_file"], 'w') as f:
        f.write(str(index))

def fetch_and_display_once():
    """Fetch image from an endpoint and display it once."""
    endpoint_index = get_next_endpoint_index()
    endpoint = CONFIG["endpoints"][endpoint_index]
    url = f"{CONFIG['base_url']}/{endpoint}"
    
    print(f"Fetching image from {url}...")
    
    try:
        response = requests.get(url, timeout=30)
        response.raise_for_status()
        
        # Ensure cache directory exists
        os.makedirs(CONFIG["cache_dir"], exist_ok=True)
        
        # Save image to cache
        with open(CONFIG["png_file"], 'wb') as f:
            f.write(response.content)
        
        print("Image downloaded successfully")
        
        # Display the image
        display_image()
        
        # Save the endpoint index
        save_endpoint_index(endpoint_index)
        
    except requests.exceptions.RequestException as e:
        print(f"Error fetching image: {e}")

def fetch_and_display_loop():
    """Fetch image from an endpoint and display it in a loop."""
    while True:
        fetch_and_display_once()
        # Wait for 5 minutes
        print("Waiting for 5 minutes...")
        time.sleep(300)

def main():
    """Main entry point"""
    if len(sys.argv) < 2:
        print("Usage: python dashboard.py <start|refresh>")
        sys.exit(1)
    
    command = sys.argv[1]
    
    if command == "start":
        fetch_and_display_loop()
    elif command == "refresh":
        fetch_and_display_once()
    else:
        print("Invalid command. Use 'start' or 'refresh'")
        sys.exit(1)

if __name__ == "__main__":
    main()
