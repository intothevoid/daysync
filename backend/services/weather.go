package services

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"time"

	"daysync/api/config"
	"daysync/api/models"
)

type WeatherAPIResponse struct {
	Location struct {
		Name      string `json:"name"`
		Region    string `json:"region"`
		Country   string `json:"country"`
		LocalTime string `json:"localtime"`
	} `json:"location"`
	Current struct {
		TempC      float64 `json:"temp_c"`
		WindKph    float64 `json:"wind_kph"`
		PrecipMm   float64 `json:"precip_mm"`
		Humidity   float64 `json:"humidity"`
		FeelsLikeC float64 `json:"feelslike_c"`
		UV         float64 `json:"uv"`
	} `json:"current"`
}

func GetWeather(location string) (*models.Weather, error) {
	apiKey := config.GetWeatherAPIKey()
	if apiKey == "" {
		return nil, fmt.Errorf("no weather api key specified")
	}

	// Build URL
	baseURL := "http://api.weatherapi.com/v1/current.json"
	params := url.Values{}
	params.Add("key", apiKey)
	params.Add("q", location)
	params.Add("aqi", "no")

	url := fmt.Sprintf("%s?%s", baseURL, params.Encode())

	// Make request
	resp, err := http.Get(url)
	if err != nil {
		return nil, fmt.Errorf("error making weather API request: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(resp.Body)
		return nil, fmt.Errorf("weather API returned status %d: %s", resp.StatusCode, string(body))
	}

	// Parse response
	var apiResp WeatherAPIResponse
	if err := json.NewDecoder(resp.Body).Decode(&apiResp); err != nil {
		return nil, fmt.Errorf("error parsing weather API response: %v", err)
	}

	// Convert to our model
	weather := &models.Weather{
		Location:      apiResp.Location.Name,
		Region:        apiResp.Location.Region,
		LocalTime:     apiResp.Location.LocalTime,
		Temperature:   apiResp.Current.TempC,
		WindSpeed:     apiResp.Current.WindKph,
		Precipitation: apiResp.Current.PrecipMm,
		Humidity:      apiResp.Current.Humidity,
		FeelsLike:     apiResp.Current.FeelsLikeC,
		UVIndex:       apiResp.Current.UV,
		UpdatedAt:     time.Now(),
	}

	return weather, nil
}
