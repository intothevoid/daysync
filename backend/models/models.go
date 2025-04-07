package models

import "time"

type Sessions struct {
	Q1     string `json:"q1"`
	Q2     string `json:"q2"`
	Sprint string `json:"sprint"`
	Race   string `json:"race"`
}

type Race struct {
	Round    int      `json:"round"`
	Name     string   `json:"name"`
	Location string   `json:"location"`
	Country  string   `json:"country"`
	Circuit  string   `json:"circuit"`
	Date     string   `json:"date"`
	Sessions Sessions `json:"sessions"`
}

type Calendar struct {
	Year  int    `json:"year"`
	Races []Race `json:"races"`
}

type Weather struct {
	Location      string    `json:"location"`
	Region        string    `json:"region"`
	LocalTime     string    `json:"local_time"`
	Temperature   float64   `json:"temperature"`
	WindSpeed     float64   `json:"wind_speed"`
	Precipitation float64   `json:"precipitation"`
	Humidity      float64   `json:"humidity"`
	FeelsLike     float64   `json:"feels_like"`
	UVIndex       float64   `json:"uv_index"`
	UpdatedAt     time.Time `json:"updated_at"`
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
