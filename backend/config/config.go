package config

import (
	"os"
	"time"

	"gopkg.in/yaml.v3"
)

type Config struct {
	Timezone      string `yaml:"timezone"`
	WeatherAPIKey string `yaml:"weather_api_key"`
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

func GetTimezone() *time.Location {
	loc, err := time.LoadLocation(cfg.Timezone)
	if err != nil {
		// Default to UTC if timezone is invalid
		return time.UTC
	}
	return loc
}

func GetWeatherAPIKey() string {
	return cfg.WeatherAPIKey
}
