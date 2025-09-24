package config

import (
	"os"
	"time"

	"gopkg.in/yaml.v3"
)

type Config struct {
	WeatherAPIKey       string `yaml:"weather_api_key"`
	APINinjasKey        string `yaml:"api_ninjas_key"`
	GNewsAPIKey         string `yaml:"gnews_api_key"`
	CacheTimeout        int    `yaml:"cache_timeout_minutes"`    // Cache timeout in minutes
	KindleImageWidth    int    `yaml:"kindle_image_width"`     // Width of the Kindle image
	KindleImageHeight   int    `yaml:"kindle_image_height"`    // Height of the Kindle image
}

var cfg Config

func LoadConfig() error {
	// Read config file
	data, err := os.ReadFile("config.yaml")
	if err != nil {
		return err
	}

	// Parse YAML
	if err := yaml.Unmarshal(data, &cfg); err != nil {
		return err
	}

	// If weather API key is not in config, try environment variable
	if cfg.WeatherAPIKey == "" {
		cfg.WeatherAPIKey = os.Getenv("WEATHER_API_KEY")
	}

	return nil
}

func GetWeatherAPIKey() string {
	return cfg.WeatherAPIKey
}

func GetAPINinjasKey() string {
	return cfg.APINinjasKey
}

func GetGNewsAPIKey() string {
	return cfg.GNewsAPIKey
}

// GetCacheTimeout returns the cache timeout duration
func GetCacheTimeout() time.Duration {
	if cfg.CacheTimeout == 0 {
		return 30 * time.Minute // Default to 30 minutes if not configured
	}
	return time.Duration(cfg.CacheTimeout) * time.Minute
}

// GetKindleImageConfig returns the Kindle image dimensions
func GetKindleImageConfig() (int, int) {
	width := cfg.KindleImageWidth
	height := cfg.KindleImageHeight
	if width == 0 {
		width = 800 // Default width
	}
	if height == 0 {
		height = 600 // Default height
	}
	return width, height
}
