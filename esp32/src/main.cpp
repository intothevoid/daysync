#include "User_Setup.h"

#include <lvgl.h>
#include <TFT_eSPI.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Base URL for all API calls
// const char* BASE_URL = "https://daysync.karan.myds.me";

// Base URL for local development - replace 192.168.50.180 with your computer's IP address
const char* BASE_URL = "http://192.168.50.180:5173";

// Replace with your network credentials
const char* ssid = "SSID";
const char* password = "PASSWORD";

// Enter your location
String location = "Adelaide";
// Type the timezone you want to get the time for
String timezone = "Adelaide/Australia";

// HTTP data caching
const unsigned long HTTP_CACHE_INTERVAL = 3600000; // 60 minutes in milliseconds
unsigned long last_weather_timestamp = -HTTP_CACHE_INTERVAL; // Force first call
unsigned long last_motogp_timestamp = -HTTP_CACHE_INTERVAL; // Force first call
const unsigned long SCREEN_SWITCH_INTERVAL = 10000;   // 10 seconds in milliseconds
unsigned long last_screen_switch = 0;
int current_screen = 0; // 0 = weather, 1 = motogp, 2 = about

// Store weather data
String current_date;
String last_weather_update;
String temperature;
String humidity;
String wind_speed;
String feels_like;
String uv_index;
String precipitation;
String weather_description;

// SET VARIABLE TO 0 FOR TEMPERATURE IN FAHRENHEIT DEGREES
#define TEMP_CELSIUS 1

#if TEMP_CELSIUS
  String temperature_unit = "";
  const char degree_symbol[] = "\u00B0C";
#else
  String temperature_unit = "&temperature_unit=fahrenheit";
  const char degree_symbol[] = "\u00B0F";
#endif

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// MotoGP data variables
String motogp_data;

// MotoGP screen objects
static lv_obj_t * motogp_label;

// Crypto data variables
String btc_data;
String eth_data;
String doge_data;
String xrp_data;
String bnb_data;
unsigned long last_crypto_timestamp = -HTTP_CACHE_INTERVAL;

// News data variables
String news_data;
unsigned long last_news_timestamp = -HTTP_CACHE_INTERVAL; // Force first call

// Function declarations
void get_weather_data();
void get_weather_description(int code);
void lv_create_main_gui(void);
void get_motogp_data();
void create_motogp_screen();
void switch_screen();
void get_all_crypto_data();
void create_bitcoin_screen();
void get_news_data();
void create_news_screen(int page);

// If logging is enabled, it will inform the user about what is happening in the library
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

static lv_obj_t * weather_image;
static lv_obj_t * text_label_date;
static lv_obj_t * text_label_temperature;
static lv_obj_t * text_label_humidity;
static lv_obj_t * text_label_weather_description;
static lv_obj_t * text_label_time_location;

static void timer_cb(lv_timer_t * timer){
  LV_UNUSED(timer);
  get_weather_data();
  lv_label_set_text(text_label_date, current_date.c_str());
  lv_label_set_text(text_label_temperature, String(temperature + "°C").c_str());
  lv_label_set_text(text_label_humidity, String(humidity + "%").c_str());
  lv_label_set_text(text_label_weather_description, weather_description.c_str());
  lv_label_set_text(text_label_time_location, String("Last Update: " + last_weather_update).c_str());
}

void get_weather_data() {
  if (WiFi.status() == WL_CONNECTED) {
    // Only update if more than 60 minutes have passed
    if (millis() - last_weather_timestamp > HTTP_CACHE_INTERVAL) {
      HTTPClient http;
      String url = String(BASE_URL) + "/api/weather?location=adelaide";
      Serial.println("Fetching weather data from: " + url);
      http.begin(url);
      int httpCode = http.GET();

      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println("Weather API Response:");
          Serial.println(payload);
          
          JsonDocument doc;
          DeserializationError error = deserializeJson(doc, payload);
          
          if (!error) {
            // Extract data from the new API response format
            temperature = String(doc["temperature"].as<float>(), 1);
            humidity = String(doc["humidity"].as<int>());
            wind_speed = String(doc["wind_speed"].as<float>(), 1);
            feels_like = String(doc["feels_like"].as<float>(), 1);
            uv_index = String(doc["uv_index"].as<int>());
            precipitation = String(doc["precipitation"].as<float>(), 1);
            
            // Format the local time
            String local_time = doc["local_time"].as<String>();
            current_date = local_time.substring(0, 10); // Extract date
            last_weather_update = local_time.substring(11, 16); // Extract time
            
            // Create weather description
            weather_description = String("Wind: ") + wind_speed + "km/h | Feels: " + feels_like + "°C";
            
            last_weather_timestamp = millis();
            
            Serial.println("Parsed weather data:");
            Serial.println("Temperature: " + temperature + "°C");
            Serial.println("Humidity: " + humidity + "%");
            Serial.println("Wind Speed: " + wind_speed + "km/h");
            Serial.println("Feels Like: " + feels_like + "°C");
            Serial.println("UV Index: " + uv_index);
            Serial.println("Precipitation: " + precipitation + "mm");
          } else {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
          }
        } else {
          Serial.println("Weather API request failed with HTTP code: " + String(httpCode));
        }
      } else {
        Serial.printf("Weather API GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.println("Using cached weather data");
    }
  } else {
    Serial.println("Not connected to Wi-Fi for weather data");
  }
}

// Function to create a title bar
lv_obj_t* create_title_bar(lv_obj_t* parent, const char* title) {
  lv_obj_t* title_bar = lv_obj_create(parent);
  lv_obj_set_size(title_bar, 320, 40); // Set width to 320 (screen height when rotated) and height to 40
  lv_obj_set_pos(title_bar, 0, 0); // Position at absolute 0,0
  lv_obj_set_style_bg_color(title_bar, lv_color_hex(0x2196F3), 0); // Material Design Blue
  lv_obj_set_style_bg_opa(title_bar, LV_OPA_COVER, 0); // Full opacity
  lv_obj_set_style_border_width(title_bar, 0, 0);
  lv_obj_set_style_pad_all(title_bar, 10, 0); // Add some padding
  lv_obj_set_style_radius(title_bar, 0, 0); // No rounded corners
  
  lv_obj_t* title_label = lv_label_create(title_bar);
  lv_label_set_text(title_label, title);
  lv_obj_set_style_text_font(title_label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
  lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0); // Center the text
  
  return title_bar;
}

void create_about_screen() {
  // Create a new screen for about information
  lv_obj_t * about_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(about_screen, lv_color_white(), 0);
  
  // Create a container for better layout
  lv_obj_t * cont = lv_obj_create(about_screen);
  lv_obj_set_size(cont, 320, 240); // Set to full screen size (rotated)
  lv_obj_set_pos(cont, 0, 0); // Position at absolute 0,0
  lv_obj_set_style_bg_color(cont, lv_color_white(), 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 0, 0);
  
  // Add title bar
  create_title_bar(cont, "About");
  
  // Version text
  lv_obj_t * version_label = lv_label_create(cont);
  lv_label_set_text(version_label, "Daysync v0.1");
  lv_obj_set_style_text_font(version_label, &lv_font_montserrat_26, 0);
  lv_obj_set_style_text_color(version_label, lv_color_hex(0xE31837), 0);
  lv_obj_align(version_label, LV_ALIGN_TOP_MID, 0, 70);

  // Author label
  lv_obj_t * author_label = lv_label_create(cont);
  lv_label_set_text(author_label, "by bindok");
  lv_obj_set_style_text_font(author_label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(author_label, lv_color_hex(0xE31837), 0);
  lv_obj_align(author_label, LV_ALIGN_TOP_MID, 0, 110);
  
  // GitHub link
  lv_obj_t * github_label = lv_label_create(cont);
  lv_label_set_text(github_label, "github.com/intothevoid/daysync");
  lv_obj_set_style_text_font(github_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(github_label, lv_color_hex(0x808080), 0);
  lv_obj_align(github_label, LV_ALIGN_TOP_MID, 0, 140);
  
  // Load the screen
  lv_screen_load(about_screen);
}

void lv_create_main_gui(void) {
  // Create a new screen for weather data
  lv_obj_t * weather_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(weather_screen, lv_color_white(), 0);
  
  // Create a container for better layout
  lv_obj_t * cont = lv_obj_create(weather_screen);
  lv_obj_set_size(cont, 320, 240); // Set to full screen size (rotated)
  lv_obj_set_pos(cont, 0, 0); // Position at absolute 0,0
  lv_obj_set_style_bg_color(cont, lv_color_white(), 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 0, 0);

  // Add title bar
  create_title_bar(cont, "Weather (Adelaide)");

  // Date label below title bar
  text_label_date = lv_label_create(cont);
  lv_label_set_text(text_label_date, current_date.c_str());
  lv_obj_set_style_text_font(text_label_date, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(text_label_date, lv_color_hex(0xE31837), 0);
  lv_obj_align(text_label_date, LV_ALIGN_TOP_MID, 0, 50);

  // Temperature section
  lv_obj_t * temp_label = lv_label_create(cont);
  lv_label_set_text(temp_label, "Temperature");
  lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_16, 0);
  lv_obj_align(temp_label, LV_ALIGN_CENTER, 0, -40);

  text_label_temperature = lv_label_create(cont);
  lv_label_set_text(text_label_temperature, String(temperature + "°C").c_str());
  lv_obj_set_style_text_font(text_label_temperature, &lv_font_montserrat_26, 0);
  lv_obj_align(text_label_temperature, LV_ALIGN_CENTER, 0, -10);

  // Humidity section
  lv_obj_t * hum_label = lv_label_create(cont);
  lv_label_set_text(hum_label, "Humidity");
  lv_obj_set_style_text_font(hum_label, &lv_font_montserrat_16, 0);
  lv_obj_align(hum_label, LV_ALIGN_CENTER, 0, 30);

  text_label_humidity = lv_label_create(cont);
  lv_label_set_text(text_label_humidity, String(humidity + "%").c_str());
  lv_obj_set_style_text_font(text_label_humidity, &lv_font_montserrat_20, 0);
  lv_obj_align(text_label_humidity, LV_ALIGN_CENTER, 0, 60);

  // Weather description at the bottom
  text_label_weather_description = lv_label_create(cont);
  lv_label_set_text(text_label_weather_description, weather_description.c_str());
  lv_obj_set_style_text_font(text_label_weather_description, &lv_font_montserrat_16, 0);
  lv_obj_align(text_label_weather_description, LV_ALIGN_BOTTOM_MID, 0, -30);

  // Last update time at the very bottom
  text_label_time_location = lv_label_create(cont);
  lv_label_set_text(text_label_time_location, String("Last Update: " + last_weather_update).c_str());
  lv_obj_set_style_text_font(text_label_time_location, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(text_label_time_location, lv_color_hex(0x808080), 0);
  lv_obj_align(text_label_time_location, LV_ALIGN_BOTTOM_MID, 0, -5);

  // Load the screen
  lv_screen_load(weather_screen);
}

void get_motogp_data() {
  if (WiFi.status() == WL_CONNECTED) {
    // Only update if more than 60 minutes have passed
    if (millis() - last_motogp_timestamp > HTTP_CACHE_INTERVAL) {
      HTTPClient http;
      String url = String(BASE_URL) + "/api/motogpnextrace?timezone=ACDT";
      Serial.println("Fetching MotoGP data from: " + url);
      http.begin(url);
      int httpCode = http.GET();

      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
          motogp_data = http.getString();
          Serial.println("MotoGP API Response:");
          Serial.println(motogp_data);
          last_motogp_timestamp = millis();
        } else {
          Serial.println("MotoGP API request failed with HTTP code: " + String(httpCode));
        }
      } else {
        Serial.printf("MotoGP API GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.println("Using cached MotoGP data");
    }
  } else {
    Serial.println("Not connected to Wi-Fi for MotoGP data");
  }
}

void create_motogp_screen() {
  // Create a new screen for MotoGP data
  lv_obj_t * motogp_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(motogp_screen, lv_color_white(), 0);
  
  // Create a container for better layout
  lv_obj_t * cont = lv_obj_create(motogp_screen);
  lv_obj_set_size(cont, 320, 240); // Set to full screen size (rotated)
  lv_obj_set_pos(cont, 0, 0); // Position at absolute 0,0
  lv_obj_set_style_bg_color(cont, lv_color_white(), 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 0, 0);
  
  // Add title bar
  create_title_bar(cont, "MotoGP - Upcoming");
  
  // Parse JSON data
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, motogp_data);
  
  if (!error) {
    // Race name - largest text
    lv_obj_t * name_label = lv_label_create(cont);
    lv_label_set_text(name_label, doc["name"].as<const char*>());
    lv_obj_set_style_text_font(name_label, &lv_font_montserrat_22, 0); 
    lv_obj_set_style_text_color(name_label, lv_color_hex(0xE31837), 0);
    lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 50);
    
    // Location and Circuit - medium text
    lv_obj_t * location_label = lv_label_create(cont);
    String location_text = String(doc["location"].as<const char*>()) + ", " + String(doc["country"].as<const char*>());
    lv_label_set_text(location_label, location_text.c_str());
    lv_obj_set_style_text_font(location_label, &lv_font_montserrat_16, 0); 
    lv_obj_align(location_label, LV_ALIGN_TOP_MID, 0, 80);
    
    lv_obj_t * circuit_label = lv_label_create(cont);
    lv_label_set_text(circuit_label, doc["circuit"].as<const char*>());
    lv_obj_set_style_text_font(circuit_label, &lv_font_montserrat_14, 0); 
    lv_obj_align(circuit_label, LV_ALIGN_TOP_MID, 0, 100);
    
    // Date - medium text
    lv_obj_t * date_label = lv_label_create(cont);
    lv_label_set_text(date_label, doc["date"].as<const char*>());
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, 0); 
    lv_obj_set_style_text_color(date_label, lv_color_hex(0xE31837), 0);
    lv_obj_align(date_label, LV_ALIGN_TOP_MID, 0, 120);
    
    // Sessions - smaller text in a single column
    JsonObject sessions = doc["sessions"];
    int y_offset = 150; // Starting y position
    int row_spacing = 20; // Spacing between rows
    
    // Create session labels in a single column
    lv_obj_t * q1_label = lv_label_create(cont);
    lv_label_set_text(q1_label, ("Q1: " + String(sessions["q1"].as<const char*>())).c_str());
    lv_obj_set_style_text_font(q1_label, &lv_font_montserrat_14, 0);
    lv_obj_align(q1_label, LV_ALIGN_TOP_MID, 0, y_offset);
    
    lv_obj_t * q2_label = lv_label_create(cont);
    lv_label_set_text(q2_label, ("Q2: " + String(sessions["q2"].as<const char*>())).c_str());
    lv_obj_set_style_text_font(q2_label, &lv_font_montserrat_14, 0);
    lv_obj_align(q2_label, LV_ALIGN_TOP_MID, 0, y_offset + row_spacing);
    
    lv_obj_t * sprint_label = lv_label_create(cont);
    lv_label_set_text(sprint_label, ("Sprint: " + String(sessions["sprint"].as<const char*>())).c_str());
    lv_obj_set_style_text_font(sprint_label, &lv_font_montserrat_14, 0);
    lv_obj_align(sprint_label, LV_ALIGN_TOP_MID, 0, y_offset + (row_spacing * 2));
    
    lv_obj_t * race_label = lv_label_create(cont);
    lv_label_set_text(race_label, ("Race: " + String(sessions["race"].as<const char*>())).c_str());
    lv_obj_set_style_text_font(race_label, &lv_font_montserrat_14, 0);
    lv_obj_align(race_label, LV_ALIGN_TOP_MID, 0, y_offset + (row_spacing * 3));
  } else {
    // Error message if JSON parsing fails
    lv_obj_t * error_label = lv_label_create(cont);
    lv_label_set_text(error_label, "Error loading MotoGP data");
    lv_obj_set_style_text_font(error_label, &lv_font_montserrat_20, 0);
    lv_obj_align(error_label, LV_ALIGN_CENTER, 0, 0);
  }
  
  // Load the screen
  lv_screen_load(motogp_screen);
}

void get_crypto_data(String symbol, String &data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(BASE_URL) + "/api/crypto?symbol=" + symbol;
    Serial.println("Fetching " + symbol + " data from: " + url);
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        data = http.getString();
        Serial.println(symbol + " API Response:");
        Serial.println(data);
      } else {
        Serial.println(symbol + " API request failed with HTTP code: " + String(httpCode));
      }
    } else {
      Serial.printf("%s API GET request failed, error: %s\n", symbol.c_str(), http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}

void get_all_crypto_data() {
  if (millis() - last_crypto_timestamp > HTTP_CACHE_INTERVAL) {
    get_crypto_data("BTCUSD", btc_data);
    get_crypto_data("ETHUSD", eth_data);
    get_crypto_data("DOGEUSD", doge_data);
    get_crypto_data("XRPUSD", xrp_data);
    get_crypto_data("BNBUSD", bnb_data);
    last_crypto_timestamp = millis();
  } else {
    Serial.println("Using cached crypto data");
  }
}

void create_small_crypto_display(lv_obj_t * parent, String data, String expected_symbol, int y_offset) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, data);
  
  if (!error) {
    // Debug logging
    Serial.println("Creating display for " + expected_symbol + " with data: " + data);
    
    // Use the expected symbol instead of parsing from response
    String symbol = expected_symbol;
    String price_str = "$" + String(doc["price"].as<const char*>());
    String symbol_with_price = symbol + " " + price_str;
    
    // Symbol
    lv_obj_t * symbol_label = lv_label_create(parent);
    lv_label_set_text(symbol_label, symbol_with_price.c_str());
    lv_obj_set_style_text_font(symbol_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(symbol_label, lv_color_black(), 0); // Change to black
    lv_obj_align(symbol_label, LV_ALIGN_TOP_MID, 0, y_offset); // Offset to left for alignment with price
    
    Serial.println("Created display for " + symbol + " with price " + price_str);
  } else {
    Serial.println("Failed to parse JSON for " + expected_symbol + ": " + error.c_str());
  }
}

void create_bitcoin_screen() {
  Serial.println("Creating Bitcoin screen");
  
  // Create a new screen for crypto data
  lv_obj_t * crypto_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(crypto_screen, lv_color_white(), 0);
  
  // Create a container for better layout
  lv_obj_t * cont = lv_obj_create(crypto_screen);
  lv_obj_set_size(cont, 320, 240);
  lv_obj_set_pos(cont, 0, 0);
  lv_obj_set_style_bg_color(cont, lv_color_white(), 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 0, 0);
  
  // Add title bar
  create_title_bar(cont, "Crypto Prices");
  
  // Parse BTC data
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, btc_data);
  
  if (!error) {
    Serial.println("Successfully parsed BTC data: " + btc_data);
    
    // BTC Symbol in large text
    lv_obj_t * symbol_label = lv_label_create(cont);
    lv_label_set_text(symbol_label, "BTC");
    lv_obj_set_style_text_font(symbol_label, &lv_font_montserrat_26, 0);
    lv_obj_set_style_text_color(symbol_label, lv_color_hex(0xE31837), 0);
    lv_obj_align(symbol_label, LV_ALIGN_TOP_MID, 0, 60);
    
    // BTC Price in large text
    lv_obj_t * price_label = lv_label_create(cont);
    String price_str = "$" + String(doc["price"].as<const char*>());
    price_str = price_str.substring(0, price_str.length() - 9);
    lv_label_set_text(price_label, price_str.c_str());
    lv_obj_set_style_text_font(price_label, &lv_font_montserrat_26, 0);
    lv_obj_align(price_label, LV_ALIGN_TOP_MID, 0, 100);
    
    Serial.println("Created BTC display with price " + price_str);
    
    // Add other cryptos vertically
    int start_y = 140; // Start position for additional coins
    int spacing = 20; // Space between each coin row
    
    create_small_crypto_display(cont, eth_data, "ETH", start_y);
    create_small_crypto_display(cont, xrp_data, "XRP", start_y + spacing);
    create_small_crypto_display(cont, doge_data, "DOGE", start_y + (spacing * 2));
    create_small_crypto_display(cont, bnb_data, "BNB", start_y + (spacing * 3));
  } else {
    Serial.println("Failed to parse BTC data: " + String(error.c_str()));
    // Error message if JSON parsing fails
    lv_obj_t * error_label = lv_label_create(cont);
    lv_label_set_text(error_label, "Error loading crypto data");
    lv_obj_set_style_text_font(error_label, &lv_font_montserrat_20, 0);
    lv_obj_align(error_label, LV_ALIGN_CENTER, 0, 0);
  }
  
  // Load the screen
  lv_screen_load(crypto_screen);
  Serial.println("Bitcoin screen created and loaded");
}

void get_news_data() {
  if (WiFi.status() == WL_CONNECTED) {
    // Only update if more than 60 minutes have passed
    if (millis() - last_news_timestamp > HTTP_CACHE_INTERVAL) {
      HTTPClient http;
      String url = String(BASE_URL) + "/api/news?location=au&max=10";
      Serial.println("Fetching News data from: " + url);
      http.begin(url);
      int httpCode = http.GET();

      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
          news_data = http.getString();
          Serial.println("News API Response:");
          Serial.println(news_data);
          last_news_timestamp = millis();
        } else {
          Serial.println("News API request failed with HTTP code: " + String(httpCode));
        }
      } else {
        Serial.printf("News API GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.println("Using cached News data");
    }
  } else {
    Serial.println("Not connected to Wi-Fi for News data");
  }
}

void create_news_screen(int page) {
  // Create a new screen for News data
  lv_obj_t * news_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(news_screen, lv_color_white(), 0);
  
  // Create a container for better layout
  lv_obj_t * cont = lv_obj_create(news_screen);
  lv_obj_set_size(cont, 320, 240); // Set to full screen size (rotated)
  lv_obj_set_pos(cont, 0, 0); // Position at absolute 0,0
  lv_obj_set_style_bg_color(cont, lv_color_white(), 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 0, 0);
  
  // Add title bar
  create_title_bar(cont, page == 1 ? "News (1/2)" : "News (2/2)");
  
  // Parse JSON data
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, news_data);
  
  if (!error) {
    // Create a container for the news titles
    lv_obj_t * list_cont = lv_obj_create(cont);
    lv_obj_set_size(list_cont, 300, 180); // Leave space for title bar
    lv_obj_align(list_cont, LV_ALIGN_TOP_MID, 0, 50); // Position below title bar
    lv_obj_set_style_bg_color(list_cont, lv_color_white(), 0);
    lv_obj_set_style_border_width(list_cont, 0, 0);
    lv_obj_set_style_pad_all(list_cont, 0, 0);
    
    // Get the articles array
    JsonArray articles = doc["articles"];
    int y_offset = 0;
    int row_spacing = 35; // Space between articles
    const int MAX_CHARS = 80; // Maximum characters for 2 lines
    
    // Calculate start and end indices for this page
    int start_index = (page == 1) ? 0 : 5;
    int end_index = (page == 1) ? 5 : 10;
    
    // Display 5 titles for this page
    for (int i = start_index; i < end_index && i < articles.size(); i++) {
      String article_title = articles[i]["title"].as<const char*>();
      String prefix = String(i + 1) + ". ";
      String title = prefix + article_title;
      
      // Truncate title if longer than MAX_CHARS
      if (title.length() > MAX_CHARS) {
        title = title.substring(0, MAX_CHARS - 3) + "...";
      }
      
      lv_obj_t * title_label = lv_label_create(list_cont);
      lv_obj_set_style_text_font(title_label, &lv_font_montserrat_12, 0); // Reduced font size
      lv_obj_set_width(title_label, 280); // Width for wrapping
      lv_label_set_long_mode(title_label, LV_LABEL_LONG_WRAP);
      lv_label_set_text(title_label, title.c_str());
      lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 10, y_offset);
      
      y_offset += row_spacing;
    }
  } else {
    // Error message if JSON parsing fails
    lv_obj_t * error_label = lv_label_create(cont);
    lv_label_set_text(error_label, "Error loading News data");
    lv_obj_set_style_text_font(error_label, &lv_font_montserrat_20, 0);
    lv_obj_align(error_label, LV_ALIGN_CENTER, 0, 0);
  }
  
  // Load the screen
  lv_screen_load(news_screen);
}

void switch_screen() {
  if (millis() - last_screen_switch > SCREEN_SWITCH_INTERVAL) {
    // Get the current screen
    lv_obj_t * current = lv_screen_active();
    
    // Create the new screen first
    switch (current_screen) {
      case 0:
        create_motogp_screen();
        break;
      case 1:
        create_bitcoin_screen();
        break;
      case 2:
        create_news_screen(1); // First news page
        break;
      case 3:
        create_news_screen(2); // Second news page
        break;
      case 4:
        create_about_screen();
        break;
      case 5:
        lv_create_main_gui();
        break;
    }
    
    // Delete the old screen after the new one is created
    if (current) {
      lv_obj_del(current);
    }
    
    current_screen = (current_screen + 1) % 6; // Now we have 6 screens
    last_screen_switch = millis();
  }
}

void setup() {
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);

  // Start LVGL
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(log_print);

  // Create a display object
  lv_display_t * disp;
  // Initialize the TFT display using the TFT_eSPI library
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);

  // Create startup screen
  lv_obj_t * startup_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(startup_screen, lv_color_white(), 0);
  
  // Create a container for better layout
  lv_obj_t * cont = lv_obj_create(startup_screen);
  lv_obj_set_size(cont, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(cont, lv_color_white(), 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 0, 0);

  // Add title bar
  create_title_bar(cont, "Daysync");

  // Create status label
  lv_obj_t * status_label = lv_label_create(cont);
  lv_label_set_text(status_label, "Connecting to WiFi...");
  lv_obj_set_style_text_font(status_label, &lv_font_montserrat_20, 0);
  lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 0);

  // Load the startup screen immediately
  lv_screen_load(startup_screen);
  lv_task_handler();
  delay(100); // Give LVGL time to render
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  
  // Wait for connection with timeout
  unsigned long startAttemptTime = millis();
  bool connected = false;
  
  while (millis() - startAttemptTime < 20000) { // 20 second timeout
    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      break;
    }
    delay(500);
    Serial.print(".");
    lv_task_handler(); // Keep LVGL responsive
  }
  
  if (connected) {
    Serial.print("\nConnected to Wi-Fi network with IP Address: ");
    Serial.println(WiFi.localIP());
    
    // Update status to show success
    lv_label_set_text(status_label, "Connected!");
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x00AA00), 0);
    lv_task_handler();
    delay(1000); // Show success message briefly
    
    // Get initial data
    get_weather_data();
    get_motogp_data();
    get_all_crypto_data();
    get_news_data();
    
    // Start with weather screen
    lv_create_main_gui();
  } else {
    Serial.println("\nFailed to connect to WiFi");
    
    // Update status to show error
    lv_label_set_text(status_label, "Unable to connect to WiFi.");
    lv_obj_set_style_text_color(status_label, lv_color_hex(0xAA0000), 0);
    lv_task_handler();
    
    // Stay on error screen indefinitely
    while (1) {
      lv_task_handler();
      delay(100);
    }
  }
}

void loop() {
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  
  // Check if it's time to switch screens
  switch_screen();
  
  delay(5);           // let this time pass
}
