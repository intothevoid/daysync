#!/usr/bin/env python3

import os
import sys
import json
import subprocess
from fetch_data import collect_all_data
from svg_generator import generate_dashboard_svg

# Configuration
CONFIG = {
    "api_url": "http://localhost:5173/api",  # Update with your DaySync server IP
    "cache_dir": "/mnt/us/extensions/daysync/cache/",
    "data_file": "/mnt/us/extensions/daysync/cache/data.json",
    "svg_file": "/mnt/us/extensions/daysync/cache/dashboard.svg",
    "png_file": "/mnt/us/extensions/daysync/cache/dashboard.png"
}

def fetch_and_cache_data():
    """Fetch data from DaySync API and cache it"""
    print("Fetching data from DaySync API...")
    try:
        data = collect_all_data(CONFIG["api_url"])
        
        # Ensure cache directory exists
        os.makedirs(CONFIG["cache_dir"], exist_ok=True)
        
        # Save data to cache
        with open(CONFIG["data_file"], 'w') as f:
            json.dump(data, f, indent=2)
            
        print("Data cached successfully")
        return data
        
    except Exception as e:
        print(f"Error fetching data: {e}")
        # Try to load cached data
        if os.path.exists(CONFIG["data_file"]):
            print("Loading cached data...")
            with open(CONFIG["data_file"], 'r') as f:
                return json.load(f)
        else:
            return None

def generate_svg():
    """Generate SVG from cached data"""
    print("Generating dashboard SVG...")
    
    if not os.path.exists(CONFIG["data_file"]):
        print("No data file found, fetching new data...")
        data = fetch_and_cache_data()
    else:
        with open(CONFIG["data_file"], 'r') as f:
            data = json.load(f)
    
    if data:
        generate_dashboard_svg(data, CONFIG["svg_file"])
        return True
    else:
        print("No data available for SVG generation")
        return False

def convert_to_png():
    """Convert SVG to PNG for Kindle display"""
    print("Converting SVG to PNG...")
    
    # Try different conversion methods
    conversion_commands = [
        f"rsvg-convert {CONFIG['svg_file']} -o {CONFIG['png_file']}",
        f"convert {CONFIG['svg_file']} {CONFIG['png_file']}",
        f"inkscape {CONFIG['svg_file']} --export-png={CONFIG['png_file']}"
    ]
    
    for cmd in conversion_commands:
        try:
            result = subprocess.run(cmd.split(), capture_output=True, text=True)
            if result.returncode == 0:
                print("PNG conversion successful")
                return True
        except Exception as e:
            print(f"Conversion attempt failed: {e}")
            continue
    
    print("All PNG conversion attempts failed")
    return False

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

def update_dashboard():
    """Complete dashboard update cycle"""
    print("=== DaySync Dashboard Update ===")
    
    # Fetch new data
    data = fetch_and_cache_data()
    if not data:
        print("Failed to fetch data, aborting update")
        return False
    
    # Generate SVG
    if not generate_svg():
        print("Failed to generate SVG, aborting update")
        return False
    
    # Convert to PNG
    if not convert_to_png():
        print("Failed to convert to PNG, aborting update")
        return False
    
    # Display on screen
    if not display_image():
        print("Failed to display image")
        return False
    
    print("Dashboard update completed successfully!")
    return True

def show_dashboard():
    """Display cached dashboard without updating"""
    print("=== Showing Cached Dashboard ===")
    return display_image()

def main():
    """Main entry point"""
    if len(sys.argv) < 2:
        print("Usage: python dashboard.py <update|show>")
        sys.exit(1)
    
    command = sys.argv[1]
    
    if command == "update":
        success = update_dashboard()
    elif command == "show":
        success = show_dashboard()
    else:
        print("Invalid command. Use 'update' or 'show'")
        sys.exit(1)
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
