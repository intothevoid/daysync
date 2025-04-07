package handlers

import (
	"encoding/json"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"time"

	"daysync/api/config"
	"daysync/api/models"

	"github.com/gorilla/mux"
)

func GetMotoGPSeason(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	year := vars["year"]

	if year != "2025" {
		http.Error(w, "data not found", http.StatusNotFound)
		return
	}

	// Read the JSON file
	data, err := os.ReadFile(filepath.Join("data", "motogp-2025.json"))
	if err != nil {
		log.Printf("Error reading data file: %v", err)
		http.Error(w, "error reading data", http.StatusInternalServerError)
		return
	}

	var calendar models.Calendar
	if err := json.Unmarshal(data, &calendar); err != nil {
		log.Printf("Error parsing JSON data: %v", err)
		http.Error(w, "error parsing data", http.StatusInternalServerError)
		return
	}

	// Convert times to configured timezone
	loc := config.GetTimezone()
	for i := range calendar.Races {
		calendar.Races[i].Sessions.Q1 = convertToTimezone(calendar.Races[i].Sessions.Q1, loc)
		calendar.Races[i].Sessions.Q2 = convertToTimezone(calendar.Races[i].Sessions.Q2, loc)
		calendar.Races[i].Sessions.Sprint = convertToTimezone(calendar.Races[i].Sessions.Sprint, loc)
		calendar.Races[i].Sessions.Race = convertToTimezone(calendar.Races[i].Sessions.Race, loc)
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(calendar)
}

func GetNextMotoGPRace(w http.ResponseWriter, r *http.Request) {
	// Read the JSON file
	data, err := os.ReadFile(filepath.Join("data", "motogp-2025.json"))
	if err != nil {
		log.Printf("Error reading data file: %v", err)
		http.Error(w, "error reading data", http.StatusInternalServerError)
		return
	}

	var calendar models.Calendar
	if err := json.Unmarshal(data, &calendar); err != nil {
		log.Printf("Error parsing JSON data: %v", err)
		http.Error(w, "error parsing data", http.StatusInternalServerError)
		return
	}

	// Get current time in UTC
	now := time.Now().UTC()

	// Find the next race
	var nextRace *models.Race
	for i := range calendar.Races {
		raceTime, err := time.Parse(time.RFC3339, calendar.Races[i].Sessions.Race)
		if err != nil {
			log.Printf("Error parsing race time: %v", err)
			continue
		}

		if raceTime.After(now) {
			nextRace = &calendar.Races[i]
			break
		}
	}

	if nextRace == nil {
		http.Error(w, "no upcoming races found", http.StatusNotFound)
		return
	}

	// Convert times to configured timezone
	loc := config.GetTimezone()
	nextRace.Sessions.Q1 = convertToTimezone(nextRace.Sessions.Q1, loc)
	nextRace.Sessions.Q2 = convertToTimezone(nextRace.Sessions.Q2, loc)
	nextRace.Sessions.Sprint = convertToTimezone(nextRace.Sessions.Sprint, loc)
	nextRace.Sessions.Race = convertToTimezone(nextRace.Sessions.Race, loc)

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(nextRace)
}

func convertToTimezone(timeStr string, loc *time.Location) string {
	t, err := time.Parse(time.RFC3339, timeStr)
	if err != nil {
		log.Printf("Error parsing time string: %v", err)
		return timeStr
	}
	return t.In(loc).Format(time.RFC3339)
}

func GetWeather(w http.ResponseWriter, r *http.Request) {
	// TODO: Implement actual weather data retrieval from a weather API
	// This is a mock implementation
	weather := models.Weather{
		Location:    "Qatar",
		Temperature: 25.5,
		Humidity:    65.0,
		Conditions:  "Sunny",
		UpdatedAt:   time.Now(),
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(weather)
}

func GetCryptoPrice(w http.ResponseWriter, r *http.Request) {
	// TODO: Implement actual crypto price retrieval from a crypto API
	// This is a mock implementation
	crypto := models.CryptoPrice{
		Symbol:      "BTC",
		PriceUSD:    50000.0,
		Change24h:   2.5,
		MarketCap:   950000000000.0,
		LastUpdated: time.Now(),
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(crypto)
}

func GetNews(w http.ResponseWriter, r *http.Request) {
	// TODO: Implement actual news retrieval from a news API
	// This is a mock implementation
	news := models.NewsResponse{
		Items: []models.NewsItem{
			{
				Title:       "MotoGP Season Preview",
				Description: "A comprehensive look at the upcoming MotoGP season",
				Source:      "MotoGP.com",
				URL:         "https://motogp.com/news",
				PublishedAt: time.Now(),
			},
			{
				Title:       "New Weather System",
				Description: "Advanced weather tracking for race weekends",
				Source:      "WeatherTech",
				URL:         "https://weathertech.com/news",
				PublishedAt: time.Now().Add(-1 * time.Hour),
			},
			// Add more news items as needed
		},
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(news)
}
