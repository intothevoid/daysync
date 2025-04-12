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

	"daysync/api/cache"
	"daysync/api/config"
	"daysync/api/helpers"
	"daysync/api/models"
	"daysync/api/services"
)

var (
	apiCache *cache.Cache
	testMode bool
)

func init() {
	// Initialize cache with timeout from config
	apiCache = cache.NewCache(config.GetCacheTimeout())
}

// SetTestMode enables or disables test mode
func SetTestMode(enabled bool) {
	testMode = enabled
}

func GetMotoGPSeason(w http.ResponseWriter, r *http.Request) {
	log.Printf("Incoming request: %s %s from %s", r.Method, r.URL.Path, r.RemoteAddr)

	// Get timezone from query parameter
	timezone := r.URL.Query().Get("timezone")
	if timezone == "" {
		timezone = "UTC" // Default to UTC if not specified
	}

	// Check cache first
	cacheKey := fmt.Sprintf("motogp:season:%s", timezone)
	if cached, exists := apiCache.Get(cacheKey); exists {
		log.Printf("[CACHE HIT] Returning cached MotoGP season data for timezone %s", timezone)
		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(cached)
		return
	}

	log.Printf("[API CALL] No cache found for MotoGP season data, reading from file for timezone %s", timezone)

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
		http.Error(w, "invalid timezone", http.StatusBadRequest)
		return
	}

	// Create a copy of the calendar to avoid modifying the cached version
	calendarCopy := calendar
	for i := range calendarCopy.Races {
		// Parse each time string and convert to the specified timezone
		if q1, err := time.Parse(time.RFC3339, calendarCopy.Races[i].Sessions.Q1); err == nil {
			calendarCopy.Races[i].Sessions.Q1 = q1.In(loc).Format(time.RFC3339)
		}
		if q2, err := time.Parse(time.RFC3339, calendarCopy.Races[i].Sessions.Q2); err == nil {
			calendarCopy.Races[i].Sessions.Q2 = q2.In(loc).Format(time.RFC3339)
		}
		if sprint, err := time.Parse(time.RFC3339, calendarCopy.Races[i].Sessions.Sprint); err == nil {
			calendarCopy.Races[i].Sessions.Sprint = sprint.In(loc).Format(time.RFC3339)
		}
		if race, err := time.Parse(time.RFC3339, calendarCopy.Races[i].Sessions.Race); err == nil {
			calendarCopy.Races[i].Sessions.Race = race.In(loc).Format(time.RFC3339)
		}
	}

	// Cache the response
	apiCache.Set(cacheKey, calendarCopy)
	log.Printf("[CACHE SET] Cached MotoGP season data for timezone %s", timezone)

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(calendarCopy)
}

func GetNextMotoGPRace(w http.ResponseWriter, r *http.Request) {
	log.Printf("Incoming request: %s %s from %s", r.Method, r.URL.Path, r.RemoteAddr)

	// Get timezone from query parameter
	timezone := r.URL.Query().Get("timezone")
	if timezone == "" {
		timezone = "UTC" // Default to UTC if not specified
	}

	// Check cache first
	cacheKey := fmt.Sprintf("motogp:nextrace:%s", timezone)
	if cached, exists := apiCache.Get(cacheKey); exists {
		log.Printf("[CACHE HIT] Returning cached next MotoGP race data for timezone %s", timezone)
		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(cached)
		return
	}

	log.Printf("[API CALL] No cache found for next MotoGP race, reading from file for timezone %s", timezone)

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
		log.Printf("No upcoming races found")
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

	// Cache the response
	apiCache.Set(cacheKey, nextRace)
	log.Printf("[CACHE SET] Cached next MotoGP race data for timezone %s", timezone)

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
	log.Printf("Incoming request: %s %s from %s", r.Method, r.URL.Path, r.RemoteAddr)

	location := r.URL.Query().Get("location")
	if location == "" {
		log.Printf("Missing location parameter")
		http.Error(w, "location parameter is required", http.StatusBadRequest)
		return
	}

	// Check cache first
	cacheKey := fmt.Sprintf("weather:%s", location)
	if cached, exists := apiCache.Get(cacheKey); exists {
		log.Printf("[CACHE HIT] Returning cached weather data for %s", location)
		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(cached)
		return
	}

	log.Printf("[API CALL] No cache found for weather data, calling weather API for %s", location)

	var weather interface{}
	var err error

	if testMode {
		weather, err = services.GetTestWeather(location)
	} else {
		weather, err = services.GetWeather(location)
	}

	if err != nil {
		log.Printf("Error getting weather: %v", err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	// Cache the response
	apiCache.Set(cacheKey, weather)
	log.Printf("[CACHE SET] Cached weather data for %s", location)

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(weather)
}

func GetCryptoPrice(w http.ResponseWriter, r *http.Request) {
	log.Printf("Incoming request: %s %s from %s", r.Method, r.URL.Path, r.RemoteAddr)

	symbol := r.URL.Query().Get("symbol")
	if symbol == "" {
		log.Printf("Missing symbol parameter")
		http.Error(w, "symbol parameter is required", http.StatusBadRequest)
		return
	}

	// Check cache first
	cacheKey := fmt.Sprintf("crypto:%s", symbol)
	if cached, exists := apiCache.Get(cacheKey); exists {
		log.Printf("[CACHE HIT] Returning cached crypto data for %s", symbol)
		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(cached)
		return
	}

	log.Printf("[API CALL] No cache found for crypto data, calling API Ninjas for %s", symbol)

	var response interface{}
	var err error

	if testMode {
		response, err = services.GetTestCryptoPrice(symbol)
	} else {
		// Get API key from config or environment
		apiKey := config.GetAPINinjasKey()
		if apiKey == "" {
			apiKey = os.Getenv("API_NINJAS_KEY")
			if apiKey == "" {
				log.Printf("API key not configured")
				http.Error(w, "API key not configured", http.StatusInternalServerError)
				return
			}
		}

		// Create request to API Ninja
		client := &http.Client{}
		req, err := http.NewRequest("GET", "https://api.api-ninjas.com/v1/cryptoprice?symbol="+symbol, nil)
		if err != nil {
			log.Printf("Error creating request: %v", err)
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		req.Header.Set("X-Api-Key", apiKey)

		// Make the request
		resp, err := client.Do(req)
		if err != nil {
			log.Printf("Error making API request: %v", err)
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		defer resp.Body.Close()

		// Check response status
		if resp.StatusCode != http.StatusOK {
			log.Printf("API request failed with status: %d", resp.StatusCode)
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
			log.Printf("Error parsing API response: %v", err)
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		// Convert timestamp to time.Time
		t := time.Unix(result.Timestamp, 0)
		// Format the time as "dd/mm/yy hh:mm:ss"
		formattedTime := t.Format("02/01/06 15:04:05")

		// Create response with formatted time
		response = struct {
			Symbol    string `json:"symbol"`
			Price     string `json:"price"`
			Timestamp string `json:"timestamp"`
		}{
			Symbol:    result.Symbol,
			Price:     result.Price,
			Timestamp: formattedTime,
		}
	}

	if err != nil {
		log.Printf("Error getting crypto price: %v", err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	// Cache the response
	apiCache.Set(cacheKey, response)
	log.Printf("[CACHE SET] Cached crypto data for %s", symbol)

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(response)
}

func GetNews(w http.ResponseWriter, r *http.Request) {
	log.Printf("Incoming request: %s %s from %s", r.Method, r.URL.Path, r.RemoteAddr)

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

	// Check cache first
	cacheKey := fmt.Sprintf("news:%s:%s:%s:%s", category, lang, country, max)
	if cached, exists := apiCache.Get(cacheKey); exists {
		log.Printf("[CACHE HIT] Returning cached news data for category %s, lang %s, country %s", category, lang, country)
		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(cached)
		return
	}

	log.Printf("[API CALL] No cache found for news data, calling GNews API for category %s, lang %s, country %s", category, lang, country)

	var newsResponse interface{}
	var err error

	if testMode {
		newsResponse, err = services.GetTestNews(category, lang, country, max)
	} else {
		// Get API key from config or environment
		apiKey := config.GetGNewsAPIKey()
		if apiKey == "" {
			apiKey = os.Getenv("GNEWS_API_KEY")
			if apiKey == "" {
				log.Printf("API key not configured")
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
			log.Printf("Error creating request: %v", err)
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		// Make the request
		resp, err := client.Do(req)
		if err != nil {
			log.Printf("Error making API request: %v", err)
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		defer resp.Body.Close()

		// Check response status
		if resp.StatusCode != http.StatusOK {
			log.Printf("API request failed with status: %d", resp.StatusCode)
			http.Error(w, fmt.Sprintf("API request failed with status: %d", resp.StatusCode), http.StatusInternalServerError)
			return
		}

		// Read the response body
		body, err := io.ReadAll(resp.Body)
		if err != nil {
			log.Printf("Error reading response body: %v", err)
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		// Parse the response
		if err := json.Unmarshal(body, &newsResponse); err != nil {
			log.Printf("Error parsing news response: %v", err)
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
	}

	if err != nil {
		log.Printf("Error getting news: %v", err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	// Cache the response
	apiCache.Set(cacheKey, newsResponse)
	log.Printf("[CACHE SET] Cached news data for category %s, lang %s, country %s", category, lang, country)

	// Send the response
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(newsResponse)
}
