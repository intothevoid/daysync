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
TFT_eSprite sprite = TFT_eSprite(&tft); // Sprite for smooth drawing

// Colors
#define BACKGROUND TFT_BLACK
#define TEXT_COLOR TFT_WHITE
#define HEADER_COLOR TFT_SKYBLUE
#define VALUE_COLOR TFT_GREEN

// Tab state
int currentTab = 0; // 0 for Weather, 1 for MotoGP, 2 for Crypto, 3 for News
unsigned long lastTabSwitch = 0;
const unsigned long tabSwitchInterval = 5000; // Switch tabs every 5 seconds

// Touch handling
uint16_t t_x = 0, t_y = 0; // Touch coordinates
bool touch_pressed = false;
const uint8_t TOUCH_THRESHOLD = 40;

void setup() {
    Serial.begin(115200);
    
    // Initialize display
    tft.init();
    tft.setRotation(1); // Landscape
    tft.fillScreen(BACKGROUND);
    
    // Initialize backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    
    // Initialize sprite
    sprite.createSprite(tft.width(), tft.height());
    sprite.setTextDatum(TL_DATUM);
    
    // Initialize touch
    tft.setTouch(nullptr);
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    
    // Show connecting message
    tft.setTextColor(TEXT_COLOR, BACKGROUND);
    tft.setTextSize(2);
    tft.setCursor(10, 120);
    tft.println("Connecting to WiFi...");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    
    // Clear screen after connection
    tft.fillScreen(BACKGROUND);
}

void drawHeader(const char* title) {
    sprite.fillRect(0, 0, tft.width(), 30, HEADER_COLOR);
    sprite.setTextColor(TEXT_COLOR);
    sprite.setTextSize(2);
    sprite.setCursor(10, 5);
    sprite.println(title);
}

void displayWeather() {
    HTTPClient http;
    http.begin(String(apiBaseUrl) + "/api/weather?location=Adelaide");
    
    sprite.fillSprite(BACKGROUND);
    drawHeader("Weather - Adelaide");
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        DynamicJsonDocument doc(512);
        deserializeJson(doc, payload);
        
        JsonObject weather = doc.as<JsonObject>();
        
        sprite.setTextColor(TEXT_COLOR);
        sprite.setTextSize(2);
        
        // Temperature
        sprite.setCursor(10, 40);
        sprite.print("Temperature: ");
        sprite.setTextColor(VALUE_COLOR);
        sprite.print(weather["temperature"].as<float>());
        sprite.println("°C");
        
        // Humidity
        sprite.setTextColor(TEXT_COLOR);
        sprite.setCursor(10, 70);
        sprite.print("Humidity: ");
        sprite.setTextColor(VALUE_COLOR);
        sprite.print(weather["humidity"].as<float>());
        sprite.println("%");
        
        // Conditions
        sprite.setTextColor(TEXT_COLOR);
        sprite.setCursor(10, 100);
        sprite.print("Conditions: ");
        sprite.setTextColor(VALUE_COLOR);
        sprite.println(weather["conditions"].as<String>());
    } else {
        sprite.setTextColor(TFT_RED);
        sprite.setCursor(10, 40);
        sprite.println("Failed to load weather data");
    }
    
    sprite.pushSprite(0, 0);
    http.end();
}

void displayMotoGPNextRace() {
    HTTPClient http;
    http.begin(String(apiBaseUrl) + "/api/motogpnextrace");
    
    sprite.fillSprite(BACKGROUND);
    drawHeader("Next MotoGP Race");
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        
        JsonObject race = doc.as<JsonObject>();
        
        sprite.setTextColor(TEXT_COLOR);
        sprite.setTextSize(2);
        
        // Circuit
        sprite.setCursor(10, 40);
        sprite.println(race["circuit"].as<String>());
        
        // Date and Time
        sprite.setCursor(10, 70);
        sprite.print(race["date"].as<String>());
        sprite.print(" ");
        sprite.println(race["time"].as<String>());
        
        // Round
        sprite.setCursor(10, 100);
        sprite.print("Round: ");
        sprite.setTextColor(VALUE_COLOR);
        sprite.println(race["round"].as<String>());
    } else {
        sprite.setTextColor(TFT_RED);
        sprite.setCursor(10, 40);
        sprite.println("Failed to load race data");
    }
    
    sprite.pushSprite(0, 0);
    http.end();
}

void displayCrypto() {
    HTTPClient http;
    http.begin(String(apiBaseUrl) + "/api/crypto?symbol=BTCUSD");
    
    sprite.fillSprite(BACKGROUND);
    drawHeader("Bitcoin Price");
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        DynamicJsonDocument doc(512);
        deserializeJson(doc, payload);
        
        JsonObject crypto = doc.as<JsonObject>();
        
        sprite.setTextColor(TEXT_COLOR);
        sprite.setTextSize(2);
        
        // Price
        sprite.setCursor(10, 40);
        sprite.print("Price: $");
        sprite.setTextColor(VALUE_COLOR);
        sprite.println(crypto["price"].as<float>(), 2);
        
        // 24h Change
        sprite.setTextColor(TEXT_COLOR);
        sprite.setCursor(10, 70);
        sprite.print("24h Change: ");
        float change = crypto["change_24h"].as<float>();
        sprite.setTextColor(change >= 0 ? TFT_GREEN : TFT_RED);
        sprite.print(change, 2);
        sprite.println("%");
        
        // Volume
        sprite.setTextColor(TEXT_COLOR);
        sprite.setCursor(10, 100);
        sprite.print("Volume: $");
        sprite.setTextColor(VALUE_COLOR);
        sprite.println(crypto["volume_24h"].as<float>(), 0);
    } else {
        sprite.setTextColor(TFT_RED);
        sprite.setCursor(10, 40);
        sprite.println("Failed to load crypto data");
    }
    
    sprite.pushSprite(0, 0);
    http.end();
}

void displayNews() {
    HTTPClient http;
    http.begin(String(apiBaseUrl) + "/api/news?category=general&lang=us&country=au&max=5");
    
    sprite.fillSprite(BACKGROUND);
    drawHeader("Latest News");
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, payload);
        
        JsonArray items = doc.as<JsonArray>();
        
        sprite.setTextColor(TEXT_COLOR);
        sprite.setTextSize(1);
        
        int y = 40;
        for (JsonObject item : items) {
            // Truncate title if too long
            String title = item["title"].as<String>();
            if (title.length() > 40) {
                title = title.substring(0, 37) + "...";
            }
            
            sprite.setCursor(10, y);
            sprite.println(title);
            
            sprite.setTextColor(HEADER_COLOR);
            sprite.setCursor(10, y + 12);
            sprite.println(item["source"].as<String>());
            sprite.setTextColor(TEXT_COLOR);
            
            y += 35;
        }
    } else {
        sprite.setTextColor(TFT_RED);
        sprite.setCursor(10, 40);
        sprite.println("Failed to load news data");
    }
    
    sprite.pushSprite(0, 0);
    http.end();
}

void checkTouch() {
    uint16_t x, y;
    if (tft.getTouch(&x, &y)) {
        if (!touch_pressed) {  // Only trigger on initial press
            touch_pressed = true;
            if (y < 30) {  // Touch in header area
                currentTab = (currentTab + 1) % 4;
                lastTabSwitch = millis();
            }
        }
    } else {
        touch_pressed = false;
    }
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Check for touch input
    checkTouch();
    
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
    
    delay(1000); // Small delay to prevent too frequent updates
} 