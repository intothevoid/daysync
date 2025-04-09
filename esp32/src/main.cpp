#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Display configuration
TFT_eSPI tft = TFT_eSPI();

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// API endpoints
const char* motogpEndpoint = "https://daysync.karan.myds.me/motogpnextrace";
const char* cryptoEndpoint = "https://daysync.karan.myds.me/crypto?symbol=BTCUSD";
const char* weatherEndpoint = "https://daysync.karan.myds.me/weather?location=adelaide";
const char* newsEndpoint = "https://daysync.karan.myds.me/news?location=au&language=en&max=5";

// Data storage
String motogpData = "";
String cryptoData = "";
String weatherData = "";
String newsData = "";

// Screen management
int currentScreen = 0;
const int SCREEN_COUNT = 4;
unsigned long lastScreenChange = 0;
const int SCREEN_CHANGE_INTERVAL = 5000; // 5 seconds
unsigned long lastDataUpdate = 0;
const int DATA_UPDATE_INTERVAL = 3600000; // 60 minutes

// Colors
#define BACKGROUND_COLOR TFT_BLACK
#define TEXT_COLOR TFT_WHITE
#define HEADER_COLOR TFT_CYAN
#define ACCENT_COLOR TFT_YELLOW

void fetchData(const char* endpoint, String* data) {
  HTTPClient http;
  http.begin(endpoint);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      *data = http.getString();
    } else {
      *data = "HTTP Error: " + String(httpCode);
    }
  } else {
    *data = "Connection Error";
  }
  http.end();
}

void fetchAllData() {
  if (WiFi.status() == WL_CONNECTED) {
    fetchData(motogpEndpoint, &motogpData);
    fetchData(cryptoEndpoint, &cryptoData);
    fetchData(weatherEndpoint, &weatherData);
    fetchData(newsEndpoint, &newsData);
    lastDataUpdate = millis();
  }
}

void drawMotogpScreen() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setTextColor(HEADER_COLOR);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("MotoGP Next Race");
  
  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(1);
  tft.setCursor(10, 40);
  
  // Parse and display MotoGP data
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, motogpData);
  
  if (!error && !doc.isNull()) {
    tft.println("Circuit: " + String(doc["circuit"].as<const char*>()));
    tft.println("Date: " + String(doc["date"].as<const char*>()));
    tft.println("Time: " + String(doc["time"].as<const char*>()));
  } else {
    tft.println("No race data available");
  }
}

void drawCryptoScreen() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setTextColor(HEADER_COLOR);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Bitcoin Price");
  
  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(1);
  tft.setCursor(10, 40);
  
  // Parse and display crypto data
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, cryptoData);
  
  if (!error && !doc.isNull()) {
    tft.println("Price: $" + String(doc["price"].as<float>(), 2));
    tft.println("24h Change: " + String(doc["change_24h"].as<float>(), 2) + "%");
  } else {
    tft.println("No crypto data available");
  }
}

void drawWeatherScreen() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setTextColor(HEADER_COLOR);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Adelaide Weather");
  
  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(1);
  tft.setCursor(10, 40);
  
  // Parse and display weather data
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, weatherData);
  
  if (!error && !doc.isNull()) {
    tft.println("Temperature: " + String(doc["temperature"].as<float>(), 1) + "°C");
    tft.println("Condition: " + String(doc["condition"].as<const char*>()));
    tft.println("Humidity: " + String(doc["humidity"].as<int>()) + "%");
  } else {
    tft.println("No weather data available");
  }
}

void drawNewsScreen() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setTextColor(HEADER_COLOR);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Latest News");
  
  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(1);
  
  // Parse and display news data
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, newsData);
  
  if (!error && !doc.isNull() && doc.containsKey("articles")) {
    JsonArray articles = doc["articles"];
    int y = 40;
    for (JsonObject article : articles) {
      if (y > 200) break; // Prevent overflow
      tft.setCursor(10, y);
      tft.println(article["title"].as<const char*>());
      y += 20;
    }
  } else {
    tft.setCursor(10, 40);
    tft.println("No news available");
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  tft.init();
  tft.setRotation(1); // Landscape mode
  tft.fillScreen(BACKGROUND_COLOR);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Connecting to WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tft.print(".");
  }
  
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setCursor(10, 10);
  tft.println("Connected!");
  delay(1000);
  
  // Initial data fetch
  fetchAllData();
}

void loop() {
  // Check if it's time to update data
  if (millis() - lastDataUpdate >= DATA_UPDATE_INTERVAL) {
    fetchAllData();
  }
  
  // Check if it's time to change screens
  if (millis() - lastScreenChange >= SCREEN_CHANGE_INTERVAL) {
    currentScreen = (currentScreen + 1) % SCREEN_COUNT;
    lastScreenChange = millis();
    
    // Draw the appropriate screen
    switch (currentScreen) {
      case 0:
        drawMotogpScreen();
        break;
      case 1:
        drawCryptoScreen();
        break;
      case 2:
        drawWeatherScreen();
        break;
      case 3:
        drawNewsScreen();
        break;
    }
  }
  
  delay(100); // Small delay to prevent overwhelming the ESP32
}
