#ifndef DATATYPES_H
#define DATATYPES_H

#include <Arduino.h> // For String type

// Structure to hold Weather Data
struct WeatherData {
    String current_date;
    String last_update_time;
    String temperature;
    String humidity;
    String wind_speed;
    String feels_like;
    String uv_index;
    String precipitation;
    String description; // Combined description like "Wind: X km/h | Feels: Y°C"
    bool valid = false; // Flag to indicate if data is valid
};

// Structure to hold session times for Races
struct RaceSessionData {
    String q1;
    String q2;
    String sprint;
    String race;
};

// Structure to hold Race Data (MotoGP/F1)
struct RaceData {
    String name;
    String location;
    String country;
    String circuit;
    String date;
    RaceSessionData sessions;
    bool valid = false; // Flag to indicate if data is valid
};

// Structure to hold Crypto Data
struct CryptoData {
    String symbol; // e.g., BTC, ETH
    String price;
    bool valid = false; // Flag to indicate if data is valid
};

// Structure to hold Finance Data (Stocks/ETFs)
struct FinanceData {
    String symbol; // e.g., ^GSPC, NDQ.AX
    String display_symbol; // e.g., S&P 500, NDQ
    float previous_close = 0.0;
    float current_price = 0.0;
    float day_low = 0.0;
    float day_high = 0.0;
    bool valid = false; // Flag to indicate if data is valid
};

// Structure to hold News Article Data
struct NewsArticle {
    String title;
    // Add other fields if needed later, e.g., source, url
};

// Structure to hold all News Data
struct NewsData {
    static const int MAX_ARTICLES = 10;
    NewsArticle articles[MAX_ARTICLES];
    int article_count = 0;
    bool valid = false; // Flag to indicate if data is valid
};

#endif // DATATYPES_H
