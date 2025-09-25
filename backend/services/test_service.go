package services

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"daysync/api/models"
)

var testData map[string]interface{}

func init() {
	// Load test data
	data, err := os.ReadFile(filepath.Join("testdata", "test_responses.json"))
	if err != nil {
		panic("Failed to load test data: " + err.Error())
	}

	if err := json.Unmarshal(data, &testData); err != nil {
		panic("Failed to parse test data: " + err.Error())
	}
}

// GetTestWeather returns test weather data
func GetTestWeather(location string) (interface{}, error) {
	weatherData, ok := testData["weather"].(map[string]interface{})
	if !ok {
		return nil, nil
	}

	// Convert location to lowercase for case-insensitive matching
	location = strings.ToLower(location)

	// Try to find exact match
	if data, ok := weatherData[location].(map[string]interface{}); ok {
		return data, nil
	}

	// If no exact match, return first available data
	for _, data := range weatherData {
		return data, nil
	}

	return nil, nil
}

// GetTestCryptoPrice returns test crypto data
func GetTestCryptoPrice(symbol string) (*models.CryptoPrice, error) {
	cryptoData, ok := testData["crypto"].(map[string]interface{})
	if !ok {
		return nil, fmt.Errorf("crypto test data not found")
	}

	// Convert symbol to uppercase for case-insensitive matching
	symbol = strings.ToUpper(symbol)

	var crypto models.CryptoPrice
	if data, ok := cryptoData[symbol].(map[string]interface{}); ok {
		jsonData, err := json.Marshal(data)
		if err != nil {
			return nil, fmt.Errorf("error marshalling crypto test data: %w", err)
		}
		if err := json.Unmarshal(jsonData, &crypto); err != nil {
			return nil, fmt.Errorf("error unmarshalling crypto test data: %w", err)
		}
		return &crypto, nil
	}

	// If no exact match, return first available data
	for _, data := range cryptoData {
		jsonData, err := json.Marshal(data)
		if err != nil {
			return nil, fmt.Errorf("error marshalling crypto test data: %w", err)
		}
		if err := json.Unmarshal(jsonData, &crypto); err != nil {
			return nil, fmt.Errorf("error unmarshalling crypto test data: %w", err)
		}
		return &crypto, nil
	}

	return nil, fmt.Errorf("no crypto test data found for symbol %s", symbol)
}

// GetTestNews returns test news data
func GetTestNews(category, lang, country, max string) (*models.NewsResponse, error) {
	newsData, ok := testData["news"].(map[string]interface{})
	if !ok {
		return nil, fmt.Errorf("news test data not found")
	}

	key := fmt.Sprintf("%s_%s_%s_%s", category, lang, country, max)

	var news models.NewsResponse
	if data, ok := newsData[key].(map[string]interface{}); ok {
		jsonData, err := json.Marshal(data)
		if err != nil {
			return nil, fmt.Errorf("error marshalling news test data: %w", err)
		}
		if err := json.Unmarshal(jsonData, &news); err != nil {
			return nil, fmt.Errorf("error unmarshalling news test data: %w", err)
		}
		return &news, nil
	}

	// If no exact match, return first available data
	for _, data := range newsData {
		jsonData, err := json.Marshal(data)
		if err != nil {
			return nil, fmt.Errorf("error marshalling news test data: %w", err)
		}
		if err := json.Unmarshal(jsonData, &news); err != nil {
			return nil, fmt.Errorf("error unmarshalling news test data: %w", err)
		}
		return &news, nil
	}

	return nil, fmt.Errorf("no news test data found for category %s, lang %s, country %s, max %s", category, lang, country, max)
}

// GetTestMotoGPSeason returns test MotoGP season data
func GetTestMotoGPSeason(timezone string) (interface{}, error) {
	motogpData, ok := testData["motogp"].(map[string]interface{})
	if !ok {
		return nil, nil
	}

	key := fmt.Sprintf("season_%s", timezone)
	if data, ok := motogpData[key].(map[string]interface{}); ok {
		return data, nil
	}

	// If no exact match, return first available data
	for _, data := range motogpData {
		return data, nil
	}

	return nil, nil
}
