#!/usr/bin/env python3

import xml.etree.ElementTree as ET
from datetime import datetime
import json

class KindleDashboardSVG:
    def __init__(self, width=600, height=800):
        self.width = width
        self.height = height
        self.svg_root = None
        self.setup_svg()
        
    def setup_svg(self):
        """Initialize SVG document structure"""
        self.svg_root = ET.Element("svg", {
            "width": str(self.width),
            "height": str(self.height),
            "xmlns": "http://www.w3.org/2000/svg"
        })
        
        # Add styles for Kindle e-ink display
        style = ET.SubElement(self.svg_root, "style")
        style.text = """
        .title { font-family: Arial; font-size: 24px; font-weight: bold; }
        .subtitle { font-family: Arial; font-size: 18px; font-weight: bold; }
        .content { font-family: Arial; font-size: 14px; }
        .small { font-family: Arial; font-size: 12px; }
        .section-bg { fill: #f5f5f5; stroke: #000; stroke-width: 1; }
        """
        
    def add_text(self, x, y, text, class_name="content"):
        """Add text element to SVG"""
        text_elem = ET.SubElement(self.svg_root, "text", {
            "x": str(x),
            "y": str(y),
            "class": class_name
        })
        text_elem.text = str(text)
        return text_elem
        
    def add_rect(self, x, y, width, height, class_name="section-bg"):
        """Add rectangle element to SVG"""
        return ET.SubElement(self.svg_root, "rect", {
            "x": str(x),
            "y": str(y), 
            "width": str(width),
            "height": str(height),
            "class": class_name
        })
        
    def add_line(self, x1, y1, x2, y2):
        """Add line element to SVG"""
        return ET.SubElement(self.svg_root, "line", {
            "x1": str(x1),
            "y1": str(y1),
            "x2": str(x2), 
            "y2": str(y2),
            "stroke": "#000",
            "stroke-width": "1"
        })

    def generate_dashboard(self, data):
        """Generate complete dashboard SVG from data"""
        current_y = 30
        
        # Title
        self.add_text(300, current_y, "DaySync Dashboard", "title")
        current_y += 40
        
        # Timestamp
        self.add_text(10, current_y, f"Updated: {data.get('timestamp', 'Unknown')}", "small")
        current_y += 30
        
        # Weather section
        if data.get('weather'):
            current_y = self.add_weather_section(data['weather'], current_y)
            
        # MotoGP section
        if data.get('motogp_next'):
            current_y = self.add_motogp_section(data['motogp_next'], current_y)
            
        # Formula 1 section
        if data.get('formula1_next'):
            current_y = self.add_formula1_section(data['formula1_next'], current_y)
            
        # Crypto section
        if data.get('crypto'):
            current_y = self.add_crypto_section(data['crypto'], current_y)
            
        # Stocks section
        if data.get('stocks'):
            current_y = self.add_stocks_section(data['stocks'], current_y)
            
        # News section
        if data.get('news'):
            current_y = self.add_news_section(data['news'], current_y)
            
    def add_weather_section(self, weather_data, start_y):
        """Add weather information section"""
        self.add_rect(10, start_y - 5, 580, 80)
        self.add_text(20, start_y + 20, "WEATHER", "subtitle")
        
        if weather_data:
            location = weather_data.get('location', 'Unknown')
            temp = weather_data.get('temperature', 'N/A')
            humidity = weather_data.get('humidity', 'N/A')
            conditions = weather_data.get('conditions', 'N/A')
            
            self.add_text(20, start_y + 45, f"Location: {location}")
            self.add_text(20, start_y + 65, f"Temperature: {temp}°C | Humidity: {humidity}%")
            self.add_text(300, start_y + 45, f"Conditions: {conditions}")
        else:
            self.add_text(20, start_y + 45, "Weather data unavailable")
            
        return start_y + 100
        
    def add_motogp_section(self, motogp_data, start_y):
        """Add MotoGP race information section"""
        self.add_rect(10, start_y - 5, 580, 100)
        self.add_text(20, start_y + 20, "NEXT MOTOGP RACE", "subtitle")
        
        if motogp_data:
            name = motogp_data.get('name', 'Unknown')
            circuit = motogp_data.get('circuit', 'Unknown')
            sessions = motogp_data.get('sessions', {})
            race_time = sessions.get('race', 'TBA')
            
            self.add_text(20, start_y + 45, f"Race: {name}")
            self.add_text(20, start_y + 65, f"Circuit: {circuit}")
            self.add_text(20, start_y + 85, f"Race Time: {race_time}")
        else:
            self.add_text(20, start_y + 45, "MotoGP data unavailable")
            
        return start_y + 120
        
    def add_formula1_section(self, f1_data, start_y):
        """Add Formula 1 race information section"""
        self.add_rect(10, start_y - 5, 580, 100)
        self.add_text(20, start_y + 20, "NEXT FORMULA 1 RACE", "subtitle")
        
        if f1_data:
            name = f1_data.get('name', 'Unknown')
            circuit = f1_data.get('circuit', 'Unknown')
            sessions = f1_data.get('sessions', {})
            race_time = sessions.get('race', 'TBA')
            
            self.add_text(20, start_y + 45, f"Race: {name}")
            self.add_text(20, start_y + 65, f"Circuit: {circuit}")
            self.add_text(20, start_y + 85, f"Race Time: {race_time}")
        else:
            self.add_text(20, start_y + 45, "Formula 1 data unavailable")
            
        return start_y + 120
        
    def add_crypto_section(self, crypto_data, start_y):
        """Add cryptocurrency prices section"""
        self.add_rect(10, start_y - 5, 580, 80)
        self.add_text(20, start_y + 20, "CRYPTOCURRENCY", "subtitle")
        
        x_pos = 20
        if crypto_data.get('bitcoin'):
            btc = crypto_data['bitcoin']
            self.add_text(x_pos, start_y + 45, f"BTC: ${btc.get('price', 'N/A')}")
            self.add_text(x_pos, start_y + 65, f"Updated: {btc.get('timestamp', 'Unknown')}")
            x_pos += 200
            
        if crypto_data.get('ethereum'):
            eth = crypto_data['ethereum']
            self.add_text(x_pos, start_y + 45, f"ETH: ${eth.get('price', 'N/A')}")
            self.add_text(x_pos, start_y + 65, f"Updated: {eth.get('timestamp', 'Unknown')}")
            
        return start_y + 100
        
    def add_stocks_section(self, stocks_data, start_y):
        """Add stock prices section"""
        self.add_rect(10, start_y - 5, 580, 80)
        self.add_text(20, start_y + 20, "STOCK MARKET", "subtitle")
        
        x_pos = 20
        # NASDAQ
        if stocks_data.get('nasdaq'):
            ndq = stocks_data['nasdaq']
            price = ndq.get('regularMarketPrice', ndq.get('price', ndq.get('fiftyTwoWeekHigh', 'N/A')))
            self.add_text(x_pos, start_y + 45, f"NDQ: ${price}")
            x_pos += 200
            
        # Australia Top 200
        if stocks_data.get('australiatop200'):
            vas = stocks_data['australiatop200']
            price = vas.get('regularMarketPrice', vas.get('price', vas.get('fiftyTwoWeekHigh', 'N/A')))
            self.add_text(x_pos, start_y + 45, f"VAS: ${price}")
            x_pos += 200

        # US Top 200
        if stocks_data.get('ustop200'):
            vgs = stocks_data['ustop200']
            price = vgs.get('regularMarketPrice', vgs.get('price', vgs.get('fiftyTwoWeekHigh', 'N/A')))
            self.add_text(x_pos, start_y + 45, f"VGS: ${price}")

        return start_y + 100
        
    def add_news_section(self, news_data, start_y):
        """Add news headlines section"""
        self.add_rect(10, start_y - 5, 580, 120)
        self.add_text(20, start_y + 20, "NEWS HEADLINES", "subtitle")
        
        if news_data and news_data.get('articles'):
            articles = news_data['articles'][:3]  # Show first 3 articles
            y_pos = start_y + 45
            
            for article in articles:
                title = article.get('title', 'No title')
                # Truncate long titles
                if len(title) > 70:
                    title = title[:67] + "..."
                self.add_text(20, y_pos, title)
                y_pos += 20
        else:
            self.add_text(20, start_y + 45, "News data unavailable")
            
        return start_y + 140
        
    def save_svg(self, filename):
        """Save SVG to file"""
        tree = ET.ElementTree(self.svg_root)
        tree.write(filename, encoding='utf-8', xml_declaration=True)

def generate_dashboard_svg(data, output_file):
    """Generate dashboard SVG from data dictionary"""
    dashboard = KindleDashboardSVG()
    dashboard.generate_dashboard(data)
    dashboard.save_svg(output_file)
    print(f"Dashboard SVG generated: {output_file}")

if __name__ == "__main__":
    import sys
    if len(sys.argv) < 3:
        print("Usage: python svg_generator.py <data.json> <output.svg>")
        sys.exit(1)
        
    with open(sys.argv[1], 'r') as f:
        data = json.load(f)
        
    generate_dashboard_svg(data, sys.argv[2])
