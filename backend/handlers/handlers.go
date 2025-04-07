package handlers

import (
	"encoding/json"
	"net/http"
	"time"

	"daysync/api/models"
)

func GetMotoGPSeason(w http.ResponseWriter, r *http.Request) {
	// TODO: Implement actual data retrieval from a database or external API
	// This is a mock implementation
	season := models.Season{
		Year: 2024,
		Races: []models.Race{
			{
				ID:          "1",
				Name:        "Qatar Grand Prix",
				Circuit:     "Losail International Circuit",
				Country:     "Qatar",
				Date:        time.Date(2024, 3, 10, 0, 0, 0, 0, time.UTC),
				IsCompleted: false,
			},
			// Add more races as needed
		},
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(season)
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
