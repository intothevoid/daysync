package handlers

import (
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"time"

	"daysync/api/config"
	"daysync/api/helpers"
	"daysync/api/models"
	"daysync/api/services"
)

func GetMotoGPSeason(w http.ResponseWriter, r *http.Request) {
	// Get timezone from query parameter
	timezone := r.URL.Query().Get("timezone")
	if timezone == "" {
		timezone = "UTC" // Default to UTC if not specified
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

	// Convert times to specified timezone and format
	loc, err := helpers.GetLocationFromAbbreviation(timezone)
	if err != nil {
		log.Printf("Invalid timezone: %v", err)
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

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
	// Get timezone from query parameter
	timezone := r.URL.Query().Get("timezone")
	if timezone == "" {
		timezone = "UTC" // Default to UTC if not specified
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

	// Convert times to specified timezone and format
	loc, err := helpers.GetLocationFromAbbreviation(timezone)
	if err != nil {
		log.Printf("Invalid timezone: %v", err)
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

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
	symbol := r.URL.Query().Get("symbol")
	if symbol == "" {
		http.Error(w, "symbol parameter is required", http.StatusBadRequest)
		return
	}

	// Get API key from config or environment
	apiKey := config.GetAPINinjasKey()
	if apiKey == "" {
		apiKey = os.Getenv("API_NINJAS_KEY")
		if apiKey == "" {
			http.Error(w, "API key not configured", http.StatusInternalServerError)
			return
		}
	}

	// Create request to API Ninja
	client := &http.Client{}
	req, err := http.NewRequest("GET", "https://api.api-ninjas.com/v1/cryptoprice?symbol="+symbol, nil)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	req.Header.Set("X-Api-Key", apiKey)

	// Make the request
	resp, err := client.Do(req)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	defer resp.Body.Close()

	// Check response status
	if resp.StatusCode != http.StatusOK {
		http.Error(w, fmt.Sprintf("API request failed with status: %d", resp.StatusCode), http.StatusInternalServerError)
		return
	}

	// Parse the response
	var result struct {
		Symbol    string `json:"symbol"`
		Price     string `json:"price"`
		Timestamp int64  `json:"timestamp"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	// Convert timestamp to time.Time
	t := time.Unix(result.Timestamp, 0)
	// Format the time as "dd/mm/yy hh:mm:ss"
	formattedTime := t.Format("02/01/06 15:04:05")

	// Create response with formatted time
	response := struct {
		Symbol    string `json:"symbol"`
		Price     string `json:"price"`
		Timestamp string `json:"timestamp"`
	}{
		Symbol:    result.Symbol,
		Price:     result.Price,
		Timestamp: formattedTime,
	}

	// Send the response
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(response)
}

func GetNews(w http.ResponseWriter, r *http.Request) {
	// Get query parameters
	category := r.URL.Query().Get("category")
	lang := r.URL.Query().Get("lang")
	country := r.URL.Query().Get("country")
	max := r.URL.Query().Get("max")

	// Set default values if not provided
	if category == "" {
		category = "general"
	}
	if lang == "" {
		lang = "en"
	}
	if country == "" {
		country = "au"
	}
	if max == "" {
		max = "10"
	}

	// Get API key from config or environment
	apiKey := config.GetGNewsAPIKey()
	if apiKey == "" {
		apiKey = os.Getenv("GNEWS_API_KEY")
		if apiKey == "" {
			http.Error(w, "API key not configured", http.StatusInternalServerError)
			return
		}
	}

	// Create request to GNews
	client := &http.Client{}
	url := fmt.Sprintf("https://gnews.io/api/v4/top-headlines?category=%s&lang=%s&country=%s&max=%s&apikey=%s",
		category, lang, country, max, apiKey)

	req, err := http.NewRequest("GET", url, nil)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	// Make the request
	resp, err := client.Do(req)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	defer resp.Body.Close()

	// Check response status
	if resp.StatusCode != http.StatusOK {
		http.Error(w, fmt.Sprintf("API request failed with status: %d", resp.StatusCode), http.StatusInternalServerError)
		return
	}

	// Forward the response as is
	w.Header().Set("Content-Type", "application/json")
	io.Copy(w, resp.Body)
}
