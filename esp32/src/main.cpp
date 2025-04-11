#include <lvgl.h>
#include <TFT_eSPI.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Weather API configuration
const char* weather_api_url = "https://daysync.karan.myds.me/api/weather?location=adelaide";

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

// Function declarations
void get_weather_data();
void get_weather_description(int code);
void lv_create_main_gui(void);
void get_motogp_data();
void create_motogp_screen();
void switch_screen();

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
      Serial.println("Fetching weather data from: " + String(weather_api_url));
      http.begin(weather_api_url);
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
  lv_obj_set_size(title_bar, SCREEN_WIDTH - 10, 25); // Reduced height
  lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 0); // Aligned to top
  lv_obj_set_style_bg_color(title_bar, lv_color_hex(0x87CEEB), 0); // Sky blue color
  lv_obj_set_style_border_width(title_bar, 0, 0);
  lv_obj_set_style_radius(title_bar, 0, 0); // Square corners
  
  lv_obj_t* title_label = lv_label_create(title_bar);
  lv_label_set_text(title_label, title);
  lv_obj_set_style_text_font(title_label, &lv_font_montserrat_16, 0); // Smaller font
  lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
  lv_obj_align(title_label, LV_ALIGN_LEFT_MID, 5, 0);
  
  return title_bar;
}

void create_about_screen() {
  // Create a new screen for about information
  lv_obj_t * about_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(about_screen, lv_color_white(), 0);
  
  // Create a container for better layout
  lv_obj_t * cont = lv_obj_create(about_screen);
  lv_obj_set_size(cont, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10);
  lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(cont, lv_color_white(), 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 5, 0);
  
  // Add title bar
  create_title_bar(cont, "About");
  
  // Version text
  lv_obj_t * version_label = lv_label_create(cont);
  lv_label_set_text(version_label, "Daysync v0.1");
  lv_obj_set_style_text_font(version_label, &lv_font_montserrat_26, 0);
  lv_obj_set_style_text_color(version_label, lv_color_hex(0xE31837), 0);
  lv_obj_align(version_label, LV_ALIGN_CENTER, 0, 0);
  
  // GitHub link
  lv_obj_t * github_label = lv_label_create(cont);
  lv_label_set_text(github_label, "github.com/intothevoid/daysync");
  lv_obj_set_style_text_font(github_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(github_label, lv_color_hex(0x808080), 0);
  lv_obj_align(github_label, LV_ALIGN_CENTER, 0, 40);
  
  // Load the screen
  lv_screen_load(about_screen);
}

void lv_create_main_gui(void) {
  // Create a new screen for weather data
  lv_obj_t * weather_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(weather_screen, lv_color_white(), 0);
  
  // Create a container for better layout
  lv_obj_t * cont = lv_obj_create(weather_screen);
  lv_obj_set_size(cont, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10);
  lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(cont, lv_color_white(), 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 5, 0);

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
      String url = "https://daysync.karan.myds.me/api/motogpnextrace";
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
  lv_obj_set_size(cont, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10);
  lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(cont, lv_color_white(), 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 5, 0);
  
  // Add title bar
  create_title_bar(cont, "MotoGP - Upcoming");
  
  // Parse JSON data
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, motogp_data);
  
  if (!error) {
    // Race name - largest text
    lv_obj_t * name_label = lv_label_create(cont);
    lv_label_set_text(name_label, doc["name"].as<const char*>());
    lv_obj_set_style_text_font(name_label, &lv_font_montserrat_20, 0); // Reduced from 22
    lv_obj_set_style_text_color(name_label, lv_color_hex(0xE31837), 0);
    lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 35);
    
    // Location and Circuit - medium text
    lv_obj_t * location_label = lv_label_create(cont);
    String location_text = String(doc["location"].as<const char*>()) + ", " + String(doc["country"].as<const char*>());
    lv_label_set_text(location_label, location_text.c_str());
    lv_obj_set_style_text_font(location_label, &lv_font_montserrat_16, 0); // Reduced from 18
    lv_obj_align(location_label, LV_ALIGN_TOP_MID, 0, 60);
    
    lv_obj_t * circuit_label = lv_label_create(cont);
    lv_label_set_text(circuit_label, doc["circuit"].as<const char*>());
    lv_obj_set_style_text_font(circuit_label, &lv_font_montserrat_14, 0); // Reduced from 16
    lv_obj_align(circuit_label, LV_ALIGN_TOP_MID, 0, 80);
    
    // Date - medium text
    lv_obj_t * date_label = lv_label_create(cont);
    lv_label_set_text(date_label, doc["date"].as<const char*>());
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_16, 0); // Reduced from 18
    lv_obj_set_style_text_color(date_label, lv_color_hex(0xE31837), 0);
    lv_obj_align(date_label, LV_ALIGN_TOP_MID, 0, 105);
    
    // Sessions - smaller text in a single column
    JsonObject sessions = doc["sessions"];
    int y_offset = 130; // Starting y position
    int row_spacing = 35; // Spacing between rows
    
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
        create_about_screen();
        break;
      case 2:
        lv_create_main_gui();
        break;
    }
    
    // Delete the old screen after the new one is created
    if (current) {
      lv_obj_del(current);
    }
    
    current_screen = (current_screen + 1) % 3;
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
  lv_obj_set_size(cont, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10);
  lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(cont, lv_color_white(), 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_pad_all(cont, 5, 0);

  // Add title bar
  create_title_bar(cont, "Daysync");

  // Create status label
  lv_obj_t * status_label = lv_label_create(cont);
  lv_label_set_text(status_label, "Connecting to WiFi...");
  lv_obj_set_style_text_font(status_label, &lv_font_montserrat_20, 0);
  lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 0);

  // Load the startup screen
  lv_screen_load(startup_screen);
  
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
  }
  
  if (connected) {
    Serial.print("\nConnected to Wi-Fi network with IP Address: ");
    Serial.println(WiFi.localIP());
    
    // Update status to show success
    lv_label_set_text(status_label, "Connected!");
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x00AA00), 0);
    delay(1000); // Show success message briefly
    
    // Get initial data
    get_weather_data();
    get_motogp_data();
    
    // Start with weather screen
    lv_create_main_gui();
  } else {
    Serial.println("\nFailed to connect to WiFi");
    
    // Update status to show error
    lv_label_set_text(status_label, "Could not connect to WiFi");
    lv_obj_set_style_text_color(status_label, lv_color_hex(0xAA0000), 0);
    
    // Create error screen
    lv_obj_t * error_label = lv_label_create(cont);
    lv_label_set_text(error_label, "Please check your WiFi settings\nand restart the device");
    lv_obj_set_style_text_font(error_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(error_label, lv_color_hex(0x808080), 0);
    lv_obj_align(error_label, LV_ALIGN_CENTER, 0, 40);
  }
}

void loop() {
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  
  // Check if it's time to switch screens
  switch_screen();
  
  delay(5);           // let this time pass
}
