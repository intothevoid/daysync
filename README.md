# DaySync

A project that displays MotoGP race calendar, weather information, crypto prices, and news on an ESP32 with a TFT display.

## Project Structure

- `backend/`: Go API server
- `esp32/`: ESP32 display code

## Backend Setup

1. Navigate to the backend directory:
```bash
cd backend
```

2. Install dependencies:
```bash
go mod tidy
```

3. Run the server:
```bash
go run main.go
```

The API will be available at `http://localhost:8080/api` with the following endpoints:
- `GET /api/motogp/{year}` - Get MotoGP season data
- `GET /api/weather/{location}` - Get weather data for a location
- `GET /api/crypto/{coin}` - Get cryptocurrency price data
- `GET /api/news` - Get top news headlines

## ESP32 Setup

1. Install required libraries in Arduino IDE:
   - TFT_eSPI
   - ArduinoJson
   - HTTPClient

2. Configure TFT_eSPI:
   - Edit the `User_Setup.h` file in the TFT_eSPI library
   - Uncomment the correct display configuration for your ESP32 board

3. Update the following in `esp32/src/main.cpp`:
   - WiFi credentials (`ssid` and `password`)
   - API endpoint URL (`apiBaseUrl`)

4. Upload the code to your ESP32

## Hardware Requirements

- ESP32 development board
- TFT display compatible with TFT_eSPI library
- USB cable for programming
- Power supply

## Display Features

- Tabbed interface switching between:
  1. MotoGP Calendar
     - Race name
     - Circuit
     - Date
  2. Weather Information
     - Location
     - Temperature
     - Humidity
     - Conditions
  3. Cryptocurrency
     - Symbol
     - Current price
     - 24h change
     - Market cap
  4. News Headlines
     - News title
     - Source
     - Multiple headlines

The display automatically switches between tabs every 5 seconds. 