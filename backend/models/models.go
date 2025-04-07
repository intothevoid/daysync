package models

import "time"

type Race struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	Circuit     string    `json:"circuit"`
	Country     string    `json:"country"`
	Date        time.Time `json:"date"`
	IsCompleted bool      `json:"is_completed"`
}

type Season struct {
	Year  int    `json:"year"`
	Races []Race `json:"races"`
}

type Weather struct {
	Location    string    `json:"location"`
	Temperature float64   `json:"temperature"`
	Humidity    float64   `json:"humidity"`
	Conditions  string    `json:"conditions"`
	UpdatedAt   time.Time `json:"updated_at"`
}

type CryptoPrice struct {
	Symbol      string    `json:"symbol"`
	PriceUSD    float64   `json:"price_usd"`
	Change24h   float64   `json:"change_24h"`
	MarketCap   float64   `json:"market_cap"`
	LastUpdated time.Time `json:"last_updated"`
}

type NewsItem struct {
	Title       string    `json:"title"`
	Description string    `json:"description"`
	Source      string    `json:"source"`
	URL         string    `json:"url"`
	PublishedAt time.Time `json:"published_at"`
}

type NewsResponse struct {
	Items []NewsItem `json:"items"`
}
