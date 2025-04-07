package handlers

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"time"

	"daysync/api/config"
	"daysync/api/models"
	"daysync/api/services"
)

func GetMotoGPSeason(w http.ResponseWriter, r *http.Request) {
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

	// Convert times to configured timezone and format
	loc := config.GetTimezone()
	for i := range calendar.Races {
		calendar.Races[i].Sessions.Q1 = formatTime(calendar.Races[i].Sessions.Q1, loc)
		calendar.Races[i].Sessions.Q2 = formatTime(calendar.Races[i].Sessions.Q2, loc)
		calendar.Races[i].Sessions.Sprint = formatTime(calendar.Races[i].Sessions.Sprint, loc)
		calendar.Races[i].Sessions.Race = formatTime(calendar.Races[i].Sessions.Race, loc)
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

	// Convert times to configured timezone and format
	loc := config.GetTimezone()
	nextRace.Sessions.Q1 = formatTime(nextRace.Sessions.Q1, loc)
	nextRace.Sessions.Q2 = formatTime(nextRace.Sessions.Q2, loc)
	nextRace.Sessions.Sprint = formatTime(nextRace.Sessions.Sprint, loc)
	nextRace.Sessions.Race = formatTime(nextRace.Sessions.Race, loc)

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(nextRace)
}

func formatTime(timeStr string, loc *time.Location) string {
	t, err := time.Parse(time.RFC3339, timeStr)
	if err != nil {
		log.Printf("Error parsing time string: %v", err)
		return timeStr
	}

	t = t.In(loc)
	day := t.Day()
	ordinal := getOrdinal(day)

	return fmt.Sprintf("%d%s %s %d at %02d:%02d",
		day, ordinal, t.Month().String(), t.Year(), t.Hour(), t.Minute())
}

func getOrdinal(n int) string {
	switch {
	case n >= 11 && n <= 13:
		return "th"
	case n%10 == 1:
		return "st"
	case n%10 == 2:
		return "nd"
	case n%10 == 3:
		return "rd"
	default:
		return "th"
	}
}

func GetWeather(w http.ResponseWriter, r *http.Request) {
	location := r.URL.Query().Get("location")
	if location == "" {
		http.Error(w, "location parameter is required", http.StatusBadRequest)
		return
	}

	weather, err := services.GetWeather(location)
	if err != nil {
		log.Printf("Error getting weather: %v", err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
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
