package services

import (
	"encoding/json"
	"fmt"
	"net/http"
	"strconv"
	"time"

	"daysync/api/config"
	"daysync/api/models"
)

// GetCryptoPrice fetches cryptocurrency prices from the API Ninjas API
func GetCryptoPrice(symbol string) (*models.CryptoPrice, error) {
	apiKey := config.GetAPINinjasKey()
	if apiKey == "" {
		return nil, fmt.Errorf("API key not configured")
	}

	client := &http.Client{}
	req, err := http.NewRequest("GET", "https://api.api-ninjas.com/v1/cryptoprice?symbol="+symbol, nil)
	if err != nil {
		return nil, fmt.Errorf("error creating request: %w", err)
	}

	req.Header.Set("X-Api-Key", apiKey)

	resp, err := client.Do(req)
	if err != nil {
		return nil, fmt.Errorf("error making API request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var result struct {
		Symbol string `json:"symbol"`
		Price  string `json:"price"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("error parsing API response: %w", err)
	}

	price, err := strconv.ParseFloat(result.Price, 64)
	if err != nil {
		return nil, fmt.Errorf("error parsing price: %w", err)
	}

	return &models.CryptoPrice{
		Symbol:      result.Symbol,
		PriceUSD:    price,
		LastUpdated: time.Now(),
	}, nil
}


