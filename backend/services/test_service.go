package services

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"strings"
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
func GetTestCryptoPrice(symbol string) (interface{}, error) {
	cryptoData, ok := testData["crypto"].(map[string]interface{})
	if !ok {
		return nil, nil
	}

	// Convert symbol to uppercase for case-insensitive matching
	symbol = strings.ToUpper(symbol)

	if data, ok := cryptoData[symbol].(map[string]interface{}); ok {
		return data, nil
	}

	// If no exact match, return first available data
	for _, data := range cryptoData {
		return data, nil
	}

	return nil, nil
}

// GetTestNews returns test news data
func GetTestNews(category, lang, country, max string) (interface{}, error) {
	newsData, ok := testData["news"].(map[string]interface{})
	if !ok {
		return nil, nil
	}

	key := fmt.Sprintf("%s_%s_%s_%s", category, lang, country, max)
	if data, ok := newsData[key].(map[string]interface{}); ok {
		return data, nil
	}

	// If no exact match, return first available data
	for _, data := range newsData {
		return data, nil
	}

	return nil, nil
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
