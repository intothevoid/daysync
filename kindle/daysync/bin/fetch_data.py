#!/usr/bin/env python3

import requests
import json
import time
from datetime import datetime
import os

class DaySyncAPI:
    def __init__(self, base_url=None):
        # Read base_url from environment variable if not provided
        if base_url is None:
            base_url = os.environ.get("DAYSYNC_API_URL", "https://localhost:5173/api")
        self.base_url = base_url
        self.session = requests.Session()
        self.session.timeout = 10
        
    def get_weather(self, location="Adelaide"):
        """Fetch weather data for specified location"""
        try:
            response = self.session.get(f"{self.base_url}/weather", 
                                      params={"location": location})
            response.raise_for_status()
            return response.json()
        except Exception as e:
            print(f"Error fetching weather: {e}")
            return None
            
    def get_next_motogp_race(self, timezone="Australia/Adelaide"):
        """Fetch next MotoGP race information"""
        try:
            response = self.session.get(f"{self.base_url}/motogpnextrace",
                                      params={"timezone": timezone})
            response.raise_for_status()
            return response.json()
        except Exception as e:
            print(f"Error fetching MotoGP data: {e}")
            return None
            
    def get_next_formula1_race(self, timezone="Australia/Adelaide"):
        """Fetch next Formula 1 race information"""
        try:
            response = self.session.get(f"{self.base_url}/formula1nextrace",
                                      params={"timezone": timezone})
            response.raise_for_status()
            return response.json()
        except Exception as e:
            print(f"Error fetching F1 data: {e}")
            return None
            
    def get_crypto_price(self, symbol="BTC"):
        """Fetch cryptocurrency price data"""
        try:
            response = self.session.get(f"{self.base_url}/crypto",
                                      params={"symbol": symbol})
            response.raise_for_status()
            return response.json()
        except Exception as e:
            print(f"Error fetching crypto data: {e}")
            return None
            
    def get_stock_price(self, symbol="AAPL"):
        """Fetch stock market data"""
        try:
            response = self.session.get(f"{self.base_url}/finance",
                                      params={"symbol": symbol})
            response.raise_for_status()
            return response.json()
        except Exception as e:
            print(f"Error fetching stock data: {e}")
            return None
            
    def get_news(self, category="general", country="au", max_articles="5"):
        """Fetch news headlines"""
        try:
            response = self.session.get(f"{self.base_url}/news",
                                      params={
                                          "category": category,
                                          "country": country,
                                          "max": max_articles
                                      })
            response.raise_for_status()
            return response.json()
        except Exception as e:
            print(f"Error fetching news: {e}")
            return None

def collect_all_data(api_url=None):
    """Collect all dashboard data from DaySync API"""
    api = DaySyncAPI(api_url)
    
    data = {
        "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "weather": api.get_weather("Adelaide"),
        "motogp_next": api.get_next_motogp_race(),
        "formula1_next": api.get_next_formula1_race(),
        "crypto": {
            "bitcoin": api.get_crypto_price("BTC"),
            "ethereum": api.get_crypto_price("ETH")
        },
        "stocks": {
            "apple": api.get_stock_price("AAPL"),
            "google": api.get_stock_price("GOOGL")
        },
        "news": api.get_news()
    }
    
    return data

if __name__ == "__main__":
    import sys
    # Use env var if no CLI arg
    api_url = sys.argv[1] if len(sys.argv) > 1 else None
    data = collect_all_data(api_url)
    print(json.dumps(data, indent=2))
