#include "User_Setup.h"
#include "wifi_config.h"
#include "DataTypes.h" // Include the new data types header

#include <lvgl.h>
#include <TFT_eSPI.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Base URL for all API calls
const char* BASE_URL = "https://daysync.karan.myds.me";

// Base URL for local development - replace 192.168.50.180 with your computer's IP address
// const char* BASE_URL = "http://192.168.50.180:5173";

// Helper function to fetch and parse JSON data
static bool fetch_json_data(const String& endpoint, JsonDocument& doc) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Not connected to Wi-Fi");
    return false;
  }

  HTTPClient http;
  String url = String(BASE_URL) + endpoint;
  Serial.println("Fetching data from: " + url);
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("API Response:");
      // Serial.println(payload); // Optional: Reduce serial output

      doc.clear();
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return false;
      }
    } else {
      Serial.println("API request failed with HTTP code: " + String(httpCode));
      return false;
    }
  } else {
    Serial.printf("API GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    return false;
  }
  http.end();
  return true;
}

// Enter your location
String location = "Adelaide";
// Type the timezone you want to get the time for
String timezone = "Adelaide/Australia";

// HTTP data caching
const unsigned long HTTP_CACHE_INTERVAL = 3600000; // 60 minutes in milliseconds
// const unsigned long HTTP_CACHE_INTERVAL = 60000; // 1 minute in milliseconds for testing
unsigned long last_weather_timestamp = 0;
unsigned long last_motogp_timestamp = 0;
unsigned long last_f1_timestamp = 0;
unsigned long last_crypto_timestamp = 0;
unsigned long last_news_timestamp = 0;
unsigned long last_finance_timestamp = 0; // Keep this for now, will be updated in get_all_finance_data
const unsigned long SCREEN_SWITCH_INTERVAL = 10000;   // 10 seconds in milliseconds
unsigned long last_screen_switch = 0;
int current_screen = 0; // 0 = weather, 1 = motogp, 2 = f1, 3 = finance, 4 = crypto, 5 = news1, 6 = news2, 7 = about

// Global instances for storing fetched data
WeatherData weather_data;
RaceData motogp_race_data;
RaceData f1_race_data;
FinanceData sp500_finance_data;
FinanceData ndq_finance_data;
FinanceData vas_finance_data;
FinanceData vgs_finance_data;
CryptoData btc_crypto_data;
CryptoData eth_crypto_data;
CryptoData doge_crypto_data;
CryptoData xrp_crypto_data;
CryptoData bnb_crypto_data;
NewsData news_data_all;


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

// MotoGP screen objects
// static lv_obj_t * motogp_label; // Will be handled within the screen creation function

// Function declarations
void get_weather_data(); // Keep declaration, implementation changes
void get_weather_description(int code);
void lv_create_main_gui(void);
void get_motogp_data();
void create_motogp_screen();
void get_f1_data();
void create_f1_screen();
void get_finance_data();
void create_finance_screen();
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

// TODO: Refactor timer_cb later to use the new data structures and update UI elements
// static void timer_cb(lv_timer_t * timer){
//   LV_UNUSED(timer);
//   get_weather_data(); // This will now populate weather_data struct
//   // Update UI elements using weather_data fields
//   if (weather_data.valid) {
//      lv_label_set_text(text_label_date, weather_data.current_date.c_str());
//      lv_label_set_text(text_label_temperature, String(weather_data.temperature + "°C").c_str());
//      lv_label_set_text(text_label_humidity, String(weather_data.humidity + "%").c_str());
//      lv_label_set_text(text_label_weather_description, weather_data.description.c_str());
//      lv_label_set_text(text_label_time_location, String("Last Update: " + weather_data.last_update_time).c_str());
//   }
// }

// Function to check if cache needs refresh
bool should_refresh_cache(unsigned long last_timestamp) {
  unsigned long current_time = millis();
  // Handle millis() overflow and initial (0) case
  return (last_timestamp == 0) || (current_time < last_timestamp) || (current_time - last_timestamp >= HTTP_CACHE_INTERVAL);
}

void get_weather_data() {
  weather_data.valid = false; // Invalidate data before fetching
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(BASE_URL) + "/api/weather?location=adelaide"; // TODO: Use config value for location
    Serial.println("Fetching weather data from: " + url);
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Weather API Response:");
        // Serial.println(payload); // Optional: Reduce serial output

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          // Populate the WeatherData struct
          weather_data.temperature = String(doc["temperature"].as<float>(), 1);
          weather_data.humidity = String(doc["humidity"].as<int>());
          weather_data.wind_speed = String(doc["wind_speed"].as<float>(), 1);
          weather_data.feels_like = String(doc["feels_like"].as<float>(), 1);
          weather_data.uv_index = String(doc["uv_index"].as<int>());
          weather_data.precipitation = String(doc["precipitation"].as<float>(), 1);

          String local_time = doc["local_time"].as<String>();
          if (local_time.length() >= 16) { // Basic check for expected format
             weather_data.current_date = local_time.substring(0, 10);
             weather_data.last_update_time = local_time.substring(11, 16);
          } else {
             weather_data.current_date = "N/A";
             weather_data.last_update_time = "N/A";
          }


          weather_data.description = String("Wind: ") + weather_data.wind_speed + "km/h | Feels: " + weather_data.feels_like + "°C";
          weather_data.valid = true; // Mark data as valid
          last_weather_timestamp = millis(); // Update timestamp
        } else {
          Serial.print("Weather JSON parsing failed: ");
          Serial.println(error.c_str());
        }
      } else {
        Serial.println("Weather API request failed, HTTP code: " + String(httpCode));
      }
    } else {
      Serial.printf("Weather API GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected, cannot fetch weather data.");
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
  lv_label_set_text(text_label_date, weather_data.valid ? weather_data.current_date.c_str() : "Loading...");
  lv_obj_set_style_text_font(text_label_date, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(text_label_date, lv_color_hex(0xE31837), 0);
  lv_obj_align(text_label_date, LV_ALIGN_TOP_MID, 0, 50);

  // Temperature section
  lv_obj_t * temp_label = lv_label_create(cont);
  lv_label_set_text(temp_label, "Temperature");
  lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_16, 0);
  lv_obj_align(temp_label, LV_ALIGN_CENTER, 0, -40);

  text_label_temperature = lv_label_create(cont);
  lv_label_set_text(text_label_temperature, weather_data.valid ? String(weather_data.temperature + "°C").c_str() : "--°C");
  lv_obj_set_style_text_font(text_label_temperature, &lv_font_montserrat_26, 0);
  lv_obj_align(text_label_temperature, LV_ALIGN_CENTER, 0, -10);

  // Humidity section
  lv_obj_t * hum_label = lv_label_create(cont);
  lv_label_set_text(hum_label, "Humidity");
  lv_obj_set_style_text_font(hum_label, &lv_font_montserrat_16, 0);
  lv_obj_align(hum_label, LV_ALIGN_CENTER, 0, 30);

  text_label_humidity = lv_label_create(cont);
  lv_label_set_text(text_label_humidity, weather_data.valid ? String(weather_data.humidity + "%").c_str() : "--%");
  lv_obj_set_style_text_font(text_label_humidity, &lv_font_montserrat_20, 0);
  lv_obj_align(text_label_humidity, LV_ALIGN_CENTER, 0, 60);

  // Weather description at the bottom
  text_label_weather_description = lv_label_create(cont);
  lv_label_set_text(text_label_weather_description, weather_data.valid ? weather_data.description.c_str() : " ");
  lv_obj_set_style_text_font(text_label_weather_description, &lv_font_montserrat_16, 0);
  lv_obj_align(text_label_weather_description, LV_ALIGN_BOTTOM_MID, 0, -30);

  // Last update time at the very bottom
  text_label_time_location = lv_label_create(cont);
  lv_label_set_text(text_label_time_location, weather_data.valid ? String("Last Update: " + weather_data.last_update_time).c_str() : " ");
  lv_obj_set_style_text_font(text_label_time_location, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(text_label_time_location, lv_color_hex(0x808080), 0);
  lv_obj_align(text_label_time_location, LV_ALIGN_BOTTOM_MID, 0, -5);

  // Load the screen
  lv_screen_load(weather_screen);
}

void get_motogp_data() {
  motogp_race_data.valid = false; // Invalidate data before fetching
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(BASE_URL) + "/api/motogpnextrace?timezone=ACDT"; // TODO: Use config value for timezone
    Serial.println("Fetching MotoGP data from: " + url);
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("MotoGP API Response:");
        // Serial.println(payload); // Optional: Reduce serial output

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          // Populate the RaceData struct for MotoGP
          motogp_race_data.name = doc["name"].as<String>();
          motogp_race_data.location = doc["location"].as<String>();
          motogp_race_data.country = doc["country"].as<String>();
          motogp_race_data.circuit = doc["circuit"].as<String>();
          motogp_race_data.date = doc["date"].as<String>();

          JsonObject sessions = doc["sessions"];
          motogp_race_data.sessions.q1 = sessions["q1"].as<String>();
          motogp_race_data.sessions.q2 = sessions["q2"].as<String>();
          motogp_race_data.sessions.sprint = sessions["sprint"].as<String>();
          motogp_race_data.sessions.race = sessions["race"].as<String>();

          motogp_race_data.valid = true; // Mark data as valid
          last_motogp_timestamp = millis(); // Update timestamp
        } else {
          Serial.print("MotoGP JSON parsing failed: ");
          Serial.println(error.c_str());
        }
      } else {
        Serial.println("MotoGP API request failed, HTTP code: " + String(httpCode));
      }
    } else {
      Serial.printf("MotoGP API GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected, cannot fetch MotoGP data.");
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

  // Use data from the motogp_race_data struct
  if (motogp_race_data.valid) {
    // Race name - largest text
    lv_obj_t * name_label = lv_label_create(cont);
    lv_label_set_text(name_label, motogp_race_data.name.c_str());
    lv_obj_set_style_text_font(name_label, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(name_label, lv_color_hex(0xE31837), 0);
    lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 50);

    // Location and Circuit - medium text
    lv_obj_t * location_label = lv_label_create(cont);
    String location_text = motogp_race_data.location + ", " + motogp_race_data.country;
    lv_label_set_text(location_label, location_text.c_str());
    lv_obj_set_style_text_font(location_label, &lv_font_montserrat_16, 0);
    lv_obj_align(location_label, LV_ALIGN_TOP_MID, 0, 80);

    lv_obj_t * circuit_label = lv_label_create(cont);
    lv_label_set_text(circuit_label, motogp_race_data.circuit.c_str());
    lv_obj_set_style_text_font(circuit_label, &lv_font_montserrat_14, 0);
    lv_obj_align(circuit_label, LV_ALIGN_TOP_MID, 0, 100);

    // Date - medium text
    lv_obj_t * date_label = lv_label_create(cont);
    lv_label_set_text(date_label, motogp_race_data.date.c_str());
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(date_label, lv_color_hex(0xE31837), 0);
    lv_obj_align(date_label, LV_ALIGN_TOP_MID, 0, 120);

    // Sessions - smaller text in a single column
    int y_offset = 150; // Starting y position
    int row_spacing = 20; // Spacing between rows

    // Create session labels in a single column
    lv_obj_t * q1_label = lv_label_create(cont);
    lv_label_set_text(q1_label, ("Q1: " + motogp_race_data.sessions.q1).c_str());
    lv_obj_set_style_text_font(q1_label, &lv_font_montserrat_14, 0);
    lv_obj_align(q1_label, LV_ALIGN_TOP_MID, 0, y_offset);

    lv_obj_t * q2_label = lv_label_create(cont);
    lv_label_set_text(q2_label, ("Q2: " + motogp_race_data.sessions.q2).c_str());
    lv_obj_set_style_text_font(q2_label, &lv_font_montserrat_14, 0);
    lv_obj_align(q2_label, LV_ALIGN_TOP_MID, 0, y_offset + row_spacing);

    lv_obj_t * sprint_label = lv_label_create(cont);
    lv_label_set_text(sprint_label, ("Sprint: " + motogp_race_data.sessions.sprint).c_str());
    lv_obj_set_style_text_font(sprint_label, &lv_font_montserrat_14, 0);
    lv_obj_align(sprint_label, LV_ALIGN_TOP_MID, 0, y_offset + (row_spacing * 2));

    lv_obj_t * race_label = lv_label_create(cont);
    lv_label_set_text(race_label, ("Race: " + motogp_race_data.sessions.race).c_str());
    lv_obj_set_style_text_font(race_label, &lv_font_montserrat_14, 0);
    lv_obj_align(race_label, LV_ALIGN_TOP_MID, 0, y_offset + (row_spacing * 3));
  } else {
    // Error message if data is not valid
    lv_obj_t * error_label = lv_label_create(cont);
    lv_label_set_text(error_label, "Error loading MotoGP data"); // Or "Loading..."
    lv_obj_set_style_text_font(error_label, &lv_font_montserrat_20, 0);
    lv_obj_align(error_label, LV_ALIGN_CENTER, 0, 0);
  }
  
  // Load the screen
  lv_screen_load(motogp_screen);
}

void get_crypto_data(const String& symbol, CryptoData& crypto) {
  crypto.valid = false;
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(BASE_URL) + "/api/crypto?symbol=" + symbol;
    Serial.println("Fetching " + symbol + " data from: " + url);
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        // Serial.println(symbol + " API Response:");
        // Serial.println(payload);

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          crypto.symbol = symbol;
          crypto.price = String(doc["price"].as<const char*>());
          crypto.valid = true;
        } else {
          Serial.println("Failed to parse JSON for " + symbol + ": " + error.c_str());
        }
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
  get_crypto_data("BTCUSD", btc_crypto_data);
  get_crypto_data("ETHUSD", eth_crypto_data);
  get_crypto_data("DOGEUSD", doge_crypto_data);
  get_crypto_data("XRPUSD", xrp_crypto_data);
  get_crypto_data("BNBUSD", bnb_crypto_data);
  last_crypto_timestamp = millis();
}

void create_small_crypto_display(lv_obj_t * parent, const CryptoData& crypto, int y_offset) {
  String symbol_with_price = crypto.valid ? (crypto.symbol + " $" + crypto.price) : (crypto.symbol + " --");
  lv_obj_t * symbol_label = lv_label_create(parent);
  lv_label_set_text(symbol_label, symbol_with_price.c_str());
  lv_obj_set_style_text_font(symbol_label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(symbol_label, lv_color_black(), 0);
  lv_obj_align(symbol_label, LV_ALIGN_TOP_MID, 0, y_offset);
}

void create_bitcoin_screen() {
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

  // BTC Symbol in large text
  lv_obj_t * symbol_label = lv_label_create(cont);
  lv_label_set_text(symbol_label, "BTC");
  lv_obj_set_style_text_font(symbol_label, &lv_font_montserrat_26, 0);
  lv_obj_set_style_text_color(symbol_label, lv_color_hex(0xE31837), 0);
  lv_obj_align(symbol_label, LV_ALIGN_TOP_MID, 0, 60);

  // BTC Price in large text
  lv_obj_t * price_label = lv_label_create(cont);
  String price_str = btc_crypto_data.valid ? ("$" + btc_crypto_data.price) : "--";
  lv_label_set_text(price_label, price_str.c_str());
  lv_obj_set_style_text_font(price_label, &lv_font_montserrat_26, 0);
  lv_obj_align(price_label, LV_ALIGN_TOP_MID, 0, 100);

  // Add other cryptos vertically
  int start_y = 140; // Start position for additional coins
  int spacing = 20; // Space between each coin row

  create_small_crypto_display(cont, eth_crypto_data, start_y);
  create_small_crypto_display(cont, xrp_crypto_data, start_y + spacing);
  create_small_crypto_display(cont, doge_crypto_data, start_y + (spacing * 2));
  create_small_crypto_display(cont, bnb_crypto_data, start_y + (spacing * 3));

  // Load the screen
  lv_screen_load(crypto_screen);
}

void get_news_data() {
  news_data_all.valid = false;
  news_data_all.article_count = 0;
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(BASE_URL) + "/api/news?location=au&max=10";
    Serial.println("Fetching News data from: " + url);
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        // Serial.println("News API Response:");
        // Serial.println(payload);

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          JsonArray articles = doc["articles"];
          int count = 0;
          for (JsonObject article : articles) {
            if (count >= NewsData::MAX_ARTICLES) break;
            news_data_all.articles[count].title = article["title"].as<String>();
            count++;
          }
          news_data_all.article_count = count;
          news_data_all.valid = (count > 0);
          last_news_timestamp = millis();
        } else {
          Serial.print("News JSON parsing failed: ");
          Serial.println(error.c_str());
        }
      } else {
        Serial.println("News API request failed with HTTP code: " + String(httpCode));
      }
    } else {
      Serial.printf("News API GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
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

  // Create a container for the news titles
  lv_obj_t * list_cont = lv_obj_create(cont);
  lv_obj_set_size(list_cont, 300, 180); // Leave space for title bar
  lv_obj_align(list_cont, LV_ALIGN_TOP_MID, 0, 50); // Position below title bar
  lv_obj_set_style_bg_color(list_cont, lv_color_white(), 0);
  lv_obj_set_style_border_width(list_cont, 0, 0);
  lv_obj_set_style_pad_all(list_cont, 0, 0);

  int y_offset = 0;
  int row_spacing = 35; // Space between articles
  const int MAX_CHARS = 80; // Maximum characters for 2 lines

  // Calculate start and end indices for this page
  int start_index = (page == 1) ? 0 : 5;
  int end_index = (page == 1) ? 5 : 10;

  if (news_data_all.valid) {
    // Display up to 5 titles for this page
    for (int i = start_index; i < end_index && i < news_data_all.article_count; i++) {
      String article_title = news_data_all.articles[i].title;
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
    // Error message if data is not valid
    lv_obj_t * error_label = lv_label_create(list_cont);
    lv_label_set_text(error_label, "Error loading News data");
    lv_obj_set_style_text_font(error_label, &lv_font_montserrat_20, 0);
    lv_obj_align(error_label, LV_ALIGN_CENTER, 0, 0);
  }

  // Load the screen
  lv_screen_load(news_screen);
}

void get_f1_data() {
  f1_race_data.valid = false; // Invalidate data before fetching
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(BASE_URL) + "/api/formula1nextrace?timezone=ACDT"; // TODO: Use config value for timezone
    Serial.println("Fetching Formula 1 data from: " + url);
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Formula 1 API Response:");
        // Serial.println(payload); // Optional: Reduce serial output

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          // Populate the RaceData struct for F1
          f1_race_data.name = doc["name"].as<String>();
          f1_race_data.location = doc["location"].as<String>();
          f1_race_data.country = doc["country"].as<String>();
          f1_race_data.circuit = doc["circuit"].as<String>();
          f1_race_data.date = doc["date"].as<String>();

          JsonObject sessions = doc["sessions"];
          f1_race_data.sessions.q1 = sessions["q1"].as<String>();
          f1_race_data.sessions.q2 = sessions["q2"].as<String>(); // Note: F1 might have different session names (e.g., fp1, fp2, q, race) - API response dictates this
          f1_race_data.sessions.sprint = sessions["sprint"].as<String>();
          f1_race_data.sessions.race = sessions["race"].as<String>();

          f1_race_data.valid = true; // Mark data as valid
          last_f1_timestamp = millis(); // Update timestamp
        } else {
          Serial.print("Formula 1 JSON parsing failed: ");
          Serial.println(error.c_str());
        }
      } else {
        Serial.println("Formula 1 API request failed, HTTP code: " + String(httpCode));
      }
    } else {
      Serial.printf("Formula 1 API GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected, cannot fetch Formula 1 data.");
  }
}

void create_f1_screen() {
  // Create a new screen for Formula 1 data
  lv_obj_t * f1_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(f1_screen, lv_color_white(), 0);

  // Create a container for better layout
  lv_obj_t * cont = lv_obj_create(f1_screen);
  lv_obj_set_size(cont, 320, 240); // Set to full screen size (rotated)
  lv_obj_set_pos(cont, 0, 0); // Position at absolute 0,0
  lv_obj_set_style_bg_color(cont, lv_color_white(), 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 0, 0);

  // Add title bar
  create_title_bar(cont, "Formula 1 - Upcoming");

  // Use data from the f1_race_data struct
  if (f1_race_data.valid) {
    // Race name - largest text
    lv_obj_t * name_label = lv_label_create(cont);
    lv_label_set_text(name_label, f1_race_data.name.c_str());
    lv_obj_set_style_text_font(name_label, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(name_label, lv_color_hex(0xE31837), 0);
    lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 50);

    // Location and Circuit - medium text
    lv_obj_t * location_label = lv_label_create(cont);
    String location_text = f1_race_data.location + ", " + f1_race_data.country;
    lv_label_set_text(location_label, location_text.c_str());
    lv_obj_set_style_text_font(location_label, &lv_font_montserrat_16, 0);
    lv_obj_align(location_label, LV_ALIGN_TOP_MID, 0, 80);

    lv_obj_t * circuit_label = lv_label_create(cont);
    lv_label_set_text(circuit_label, f1_race_data.circuit.c_str());
    lv_obj_set_style_text_font(circuit_label, &lv_font_montserrat_14, 0);
    lv_obj_align(circuit_label, LV_ALIGN_TOP_MID, 0, 100);

    // Date - medium text
    lv_obj_t * date_label = lv_label_create(cont);
    lv_label_set_text(date_label, f1_race_data.date.c_str());
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(date_label, lv_color_hex(0xE31837), 0);
    lv_obj_align(date_label, LV_ALIGN_TOP_MID, 0, 120);

    // Sessions - smaller text in a single column
    int y_offset = 150; // Starting y position
    int row_spacing = 20; // Spacing between rows

    // Create session labels in a single column (Adjust keys based on actual API response for F1)
    lv_obj_t * q1_label = lv_label_create(cont);
    lv_label_set_text(q1_label, ("Q1: " + f1_race_data.sessions.q1).c_str()); // Or FP1, Q, etc.
    lv_obj_set_style_text_font(q1_label, &lv_font_montserrat_14, 0);
    lv_obj_align(q1_label, LV_ALIGN_TOP_MID, 0, y_offset);

    lv_obj_t * q2_label = lv_label_create(cont);
    lv_label_set_text(q2_label, ("Q2: " + f1_race_data.sessions.q2).c_str()); // Or FP2, etc.
    lv_obj_set_style_text_font(q2_label, &lv_font_montserrat_14, 0);
    lv_obj_align(q2_label, LV_ALIGN_TOP_MID, 0, y_offset + row_spacing);

    lv_obj_t * sprint_label = lv_label_create(cont);
    lv_label_set_text(sprint_label, ("Sprint: " + f1_race_data.sessions.sprint).c_str()); // If applicable
    lv_obj_set_style_text_font(sprint_label, &lv_font_montserrat_14, 0);
    lv_obj_align(sprint_label, LV_ALIGN_TOP_MID, 0, y_offset + (row_spacing * 2));

    lv_obj_t * race_label = lv_label_create(cont);
    lv_label_set_text(race_label, ("Race: " + f1_race_data.sessions.race).c_str());
    lv_obj_set_style_text_font(race_label, &lv_font_montserrat_14, 0);
    lv_obj_align(race_label, LV_ALIGN_TOP_MID, 0, y_offset + (row_spacing * 3));
  } else {
    // Error message if data is not valid
    lv_obj_t * error_label = lv_label_create(cont);
    lv_label_set_text(error_label, "Error loading Formula 1 data"); // Or "Loading..."
    lv_obj_set_style_text_font(error_label, &lv_font_montserrat_20, 0);
    lv_obj_align(error_label, LV_ALIGN_CENTER, 0, 0);
  }

  // Load the screen
  lv_screen_load(f1_screen);
}

void get_finance_data(const String& symbol, FinanceData& finance) {
  finance.valid = false;
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(BASE_URL) + "/api/finance?symbol=" + symbol;
    Serial.println("Fetching finance data for " + symbol + " from: " + url);
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        // Serial.println(symbol + " Finance API Response:");
        // Serial.println(payload);

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          finance.symbol = symbol;
          // finance.display_symbol = doc["shortName"].as<String>(); // Or use a mapping if needed
          finance.previous_close = doc["previousClose"].as<float>();
          finance.current_price = doc["regularMarketPrice"].as<float>();
          finance.day_low = doc["regularMarketDayLow"].as<float>();
          finance.day_high = doc["regularMarketDayHigh"].as<float>();
          finance.valid = true;
        } else {
           Serial.println("Failed to parse JSON for " + symbol + ": " + error.c_str());
        }
      } else {
         Serial.println(symbol + " Finance API request failed with HTTP code: " + String(httpCode));
      }
    } else {
       Serial.printf("%s Finance API GET request failed, error: %s\n", symbol.c_str(), http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}

void get_all_finance_data() {
  get_finance_data("^GSPC", sp500_finance_data);
  sp500_finance_data.display_symbol = "S&P 500"; // Set display name manually
  get_finance_data("NDQ.AX", ndq_finance_data);
  ndq_finance_data.display_symbol = "NDQ";
  get_finance_data("VAS.AX", vas_finance_data);
  vas_finance_data.display_symbol = "VAS";
  get_finance_data("VGS.AX", vgs_finance_data);
  vgs_finance_data.display_symbol = "VGS";
  last_finance_timestamp = millis(); // Update timestamp after fetching all
}

void create_small_finance_display(lv_obj_t * parent, const FinanceData& finance, int y_offset) {
  if (finance.valid) {
    // Calculate percentage change using current price instead of high
    float percent_change = 0.0;
    if (finance.previous_close > 0) {  // Prevent division by zero
      percent_change = ((finance.current_price - finance.previous_close) / finance.previous_close) * 100.0;
    }

    // Format the strings
    String price_range = "$" + String(finance.day_low, 2) + " - $" + String(finance.day_high, 2);
    String change_str;
    lv_color_t change_color;

    if (abs(percent_change) < 0.1) { // Consider changes less than 0.1% as 0%
      change_str = "0.0%";
      change_color = lv_color_black();
    } else {
      change_str = (percent_change > 0 ? "/\\ +" : "\\/ ") + String(abs(percent_change), 1) + "%";
      change_color = percent_change > 0 ? lv_color_hex(0x00AA00) : lv_color_hex(0xE31837);
    }

    // Create a container for this row
    lv_obj_t * row_cont = lv_obj_create(parent);
    lv_obj_set_size(row_cont, 280, 20);
    lv_obj_set_style_bg_color(row_cont, lv_color_white(), 0);
    lv_obj_set_style_border_width(row_cont, 0, 0);
    lv_obj_set_style_pad_all(row_cont, 0, 0);
    lv_obj_align(row_cont, LV_ALIGN_TOP_MID, 0, y_offset);

    // Symbol
    lv_obj_t * symbol_label = lv_label_create(row_cont);
    lv_label_set_text(symbol_label, finance.display_symbol.c_str());
    lv_obj_set_style_text_font(symbol_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(symbol_label, lv_color_hex(0xE31837), 0);
    lv_obj_align(symbol_label, LV_ALIGN_LEFT_MID, 0, 0);

    // Price range
    lv_obj_t * price_label = lv_label_create(row_cont);
    lv_label_set_text(price_label, price_range.c_str());
    lv_obj_set_style_text_font(price_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(price_label, lv_color_black(), 0);
    lv_obj_align(price_label, LV_ALIGN_LEFT_MID, 50, 0); // Adjust alignment as needed

    // Change percentage
    lv_obj_t * change_label = lv_label_create(row_cont);
    lv_label_set_text(change_label, change_str.c_str());
    lv_obj_set_style_text_font(change_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(change_label, change_color, 0);
    lv_obj_align(change_label, LV_ALIGN_RIGHT_MID, 0, 0);
  } else {
     // Optionally display a loading/error state for this specific item
     lv_obj_t * error_label = lv_label_create(parent);
     lv_label_set_text(error_label, (finance.display_symbol + ": --").c_str());
     lv_obj_set_style_text_font(error_label, &lv_font_montserrat_14, 0);
     lv_obj_set_style_text_color(error_label, lv_color_hex(0x808080), 0);
     lv_obj_align(error_label, LV_ALIGN_TOP_LEFT, 10, y_offset);
  }
}

void create_finance_screen() {
  lv_obj_t * finance_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(finance_screen, lv_color_white(), 0);

  // Create a container for better layout
  lv_obj_t * cont = lv_obj_create(finance_screen);
  lv_obj_set_size(cont, 320, 240);
  lv_obj_set_pos(cont, 0, 0);
  lv_obj_set_style_bg_color(cont, lv_color_white(), 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 0, 0);

  // Add title bar
  create_title_bar(cont, "Stocks");

  // Use S&P 500 data from the struct
  if (sp500_finance_data.valid) {
    // Calculate percentage change using current price
    float percent_change = 0.0;
    if (sp500_finance_data.previous_close > 0) {  // Prevent division by zero
      percent_change = ((sp500_finance_data.current_price - sp500_finance_data.previous_close) / sp500_finance_data.previous_close) * 100.0;
    }

    // Create main container for S&P 500
    lv_obj_t * sp500_cont = lv_obj_create(cont);
    lv_obj_set_size(sp500_cont, 280, 80);
    lv_obj_set_style_bg_color(sp500_cont, lv_color_white(), 0);
    lv_obj_set_style_border_width(sp500_cont, 0, 0);
    lv_obj_set_style_pad_all(sp500_cont, 0, 0);
    lv_obj_align(sp500_cont, LV_ALIGN_TOP_MID, 0, 50);

    // S&P 500 Symbol
    lv_obj_t * symbol_label = lv_label_create(sp500_cont);
    lv_label_set_text(symbol_label, sp500_finance_data.display_symbol.c_str());
    lv_obj_set_style_text_font(symbol_label, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(symbol_label, lv_color_hex(0xE31837), 0);
    lv_obj_align(symbol_label, LV_ALIGN_TOP_MID, 0, 0);

    // Price range
    String price_range = "$" + String(sp500_finance_data.day_low, 2) + " - $" + String(sp500_finance_data.day_high, 2);
    lv_obj_t * price_label = lv_label_create(sp500_cont);
    lv_label_set_text(price_label, price_range.c_str());
    lv_obj_set_style_text_font(price_label, &lv_font_montserrat_22, 0);
    lv_obj_align(price_label, LV_ALIGN_TOP_MID, 0, 30);

    // Change percentage
    String change_str;
    lv_color_t change_color;

    if (abs(percent_change) < 0.1) { // Consider changes less than 0.1% as 0%
      change_str = "0.0%";
      change_color = lv_color_black();
    } else {
      change_str = (percent_change > 0 ? "/\\ +" : "\\/ ") + String(abs(percent_change), 1) + "%";
      change_color = percent_change > 0 ? lv_color_hex(0x00AA00) : lv_color_hex(0xE31837);
    }

    lv_obj_t * change_label = lv_label_create(sp500_cont);
    lv_label_set_text(change_label, change_str.c_str());
    lv_obj_set_style_text_font(change_label, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(change_label, change_color, 0);
    lv_obj_align(change_label, LV_ALIGN_TOP_MID, 0, 60);

  } else {
     // Display error/loading for S&P 500 section
     lv_obj_t * error_label = lv_label_create(cont);
     lv_label_set_text(error_label, "Loading S&P 500...");
     lv_obj_set_style_text_font(error_label, &lv_font_montserrat_20, 0);
     lv_obj_align(error_label, LV_ALIGN_TOP_MID, 0, 70);
  }

  // Add other stocks vertically using the helper function
  int start_y = 140;
  int spacing = 25;

  create_small_finance_display(cont, ndq_finance_data, start_y);
  create_small_finance_display(cont, vas_finance_data, start_y + spacing);
  create_small_finance_display(cont, vgs_finance_data, start_y + (spacing * 2));

  lv_screen_load(finance_screen);
}

void switch_screen() {
  if (millis() - last_screen_switch > SCREEN_SWITCH_INTERVAL) {
    lv_obj_t * current = lv_screen_active();
    
    switch (current_screen) {
      case 0:
        create_motogp_screen();
        break;
      case 1:
        create_f1_screen();
        break;
      case 2:
        create_finance_screen();
        break;
      case 3:
        create_bitcoin_screen();
        break;
      case 4:
        create_news_screen(1);
        break;
      case 5:
        create_news_screen(2);
        break;
      case 6:
        create_about_screen();
        break;
      case 7:
        lv_create_main_gui();
        break;
    }
    
    if (current) {
      lv_obj_del(current);
    }
    
    current_screen = (current_screen + 1) % 8; // Now we have 8 screens
    last_screen_switch = millis();
  }
}

// Function to refresh all data if needed
void check_and_refresh_data() {
  if (WiFi.status() == WL_CONNECTED) {
    if (should_refresh_cache(last_weather_timestamp)) {
      get_weather_data();
    }
    if (should_refresh_cache(last_motogp_timestamp)) {
      get_motogp_data();
    }
    if (should_refresh_cache(last_f1_timestamp)) {
      get_f1_data();
    }
    if (should_refresh_cache(last_finance_timestamp)) {
      get_all_finance_data();
    }
    if (should_refresh_cache(last_crypto_timestamp)) {
      get_all_crypto_data();
    }
    if (should_refresh_cache(last_news_timestamp)) {
      get_news_data();
    }
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
    get_f1_data();
    get_all_finance_data();
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
  
  // Check if data needs to be refreshed
  check_and_refresh_data();
  
  // Check if it's time to switch screens
  switch_screen();
  
  delay(5);           // let this time pass
}
