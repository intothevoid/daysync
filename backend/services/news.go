package services

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"

	"daysync/api/config"
	"daysync/api/models"
)

// GetNews fetches news from the GNews API
func GetNews(category, lang, country, max string) (*models.NewsResponse, error) {
	apiKey := config.GetGNewsAPIKey()
	if apiKey == "" {
		return nil, fmt.Errorf("API key not configured")
	}

	url := fmt.Sprintf("https://gnews.io/api/v4/top-headlines?category=%s&lang=%s&country=%s&max=%s&apikey=%s",
		category, lang, country, max, apiKey)

	resp, err := http.Get(url)
	if err != nil {
		return nil, fmt.Errorf("error making API request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("error reading response body: %w", err)
	}

	var gnewsResponse struct {
		Articles []struct {
			Title       string `json:"title"`
			Description string `json:"description"`
			URL         string `json:"url"`
			Source      struct {
				Name string `json:"name"`
			} `json:"source"`
			PublishedAt string `json:"publishedAt"`
		} `json:"articles"`
	}

	if err := json.Unmarshal(body, &gnewsResponse); err != nil {
		return nil, fmt.Errorf("error parsing news response: %w", err)
	}

	newsResponse := &models.NewsResponse{
		Items: make([]models.NewsItem, len(gnewsResponse.Articles)),
	}

	for i, article := range gnewsResponse.Articles {
		newsResponse.Items[i] = models.NewsItem{
			Title:       article.Title,
			Description: article.Description,
			Source:      article.Source.Name,
			URL:         article.URL,
			// PublishedAt needs to be parsed from string to time.Time if needed
		}
	}

	return newsResponse, nil
}

