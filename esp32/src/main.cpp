#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <SPI.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// API endpoint
const char* apiBaseUrl = "http://YOUR_API_IP:8080";

// Display setup
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

// Tab state
int currentTab = 0; // 0 for Weather, 1 for MotoGP, 2 for Crypto, 3 for News
unsigned long lastTabSwitch = 0;
const unsigned long tabSwitchInterval = 5000; // Switch tabs every 5 seconds

void setup() {
    Serial.begin(115200);
    
    // Initialize display
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");
}

void displayWeather() {
    HTTPClient http;
    http.begin(String(apiBaseUrl) + "/api/weather?location=Adelaide");
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        DynamicJsonDocument doc(512);
        deserializeJson(doc, payload);
        
        JsonObject weather = doc.as<JsonObject>();
        
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(2);
        tft.setCursor(10, 10);
        tft.println("Weather");
        
        tft.setTextSize(1);
        tft.setCursor(10, 40);
        tft.print("Location: ");
        tft.println(weather["location"].as<String>());
        
        tft.setCursor(10, 60);
        tft.print("Temp: ");
        tft.print(weather["temperature"].as<float>());
        tft.println("°C");
        
        tft.setCursor(10, 80);
        tft.print("Humidity: ");
        tft.print(weather["humidity"].as<float>());
        tft.println("%");
        
        tft.setCursor(10, 100);
        tft.print("Conditions: ");
        tft.println(weather["conditions"].as<String>());
    }
    http.end();
}

void displayMotoGPNextRace() {
    HTTPClient http;
    http.begin(String(apiBaseUrl) + "/api/motogpnextrace");
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        
        JsonObject race = doc.as<JsonObject>();
        
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(2);
        tft.setCursor(10, 10);
        tft.println("Next MotoGP Race");
        
        tft.setTextSize(1);
        tft.setCursor(10, 40);
        tft.print("Circuit: ");
        tft.println(race["circuit"].as<String>());
        
        tft.setCursor(10, 60);
        tft.print("Date: ");
        tft.println(race["date"].as<String>());
        
        tft.setCursor(10, 80);
        tft.print("Time: ");
        tft.println(race["time"].as<String>());
        
        tft.setCursor(10, 100);
        tft.print("Round: ");
        tft.println(race["round"].as<String>());
    }
    http.end();
}

void displayCrypto() {
    HTTPClient http;
    http.begin(String(apiBaseUrl) + "/api/crypto?symbol=BTCUSD");
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        DynamicJsonDocument doc(512);
        deserializeJson(doc, payload);
        
        JsonObject crypto = doc.as<JsonObject>();
        
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(2);
        tft.setCursor(10, 10);
        tft.println("Crypto");
        
        tft.setTextSize(1);
        tft.setCursor(10, 40);
        tft.print("Symbol: ");
        tft.println(crypto["symbol"].as<String>());
        
        tft.setCursor(10, 60);
        tft.print("Price: $");
        tft.println(crypto["price"].as<float>());
        
        tft.setCursor(10, 80);
        tft.print("24h Change: ");
        tft.print(crypto["change_24h"].as<float>());
        tft.println("%");
        
        tft.setCursor(10, 100);
        tft.print("Volume: $");
        tft.println(crypto["volume_24h"].as<float>());
    }
    http.end();
}

void displayNews() {
    HTTPClient http;
    http.begin(String(apiBaseUrl) + "/api/news?category=general&lang=us&country=au&max=5");
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, payload);
        
        JsonArray items = doc.as<JsonArray>();
        
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(2);
        tft.setCursor(10, 10);
        tft.println("News");
        
        tft.setTextSize(1);
        int y = 40;
        for (JsonObject item : items) {
            tft.setCursor(10, y);
            tft.println(item["title"].as<String>());
            tft.setCursor(10, y + 15);
            tft.println(item["source"].as<String>());
            y += 40;
        }
    }
    http.end();
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Switch tabs periodically
    if (currentMillis - lastTabSwitch >= tabSwitchInterval) {
        currentTab = (currentTab + 1) % 4;
        lastTabSwitch = currentMillis;
    }
    
    // Update display based on current tab
    switch (currentTab) {
        case 0:
            displayWeather();
            break;
        case 1:
            displayMotoGPNextRace();
            break;
        case 2:
            displayCrypto();
            break;
        case 3:
            displayNews();
            break;
    }
    
    delay(1000); // Update every second
} 