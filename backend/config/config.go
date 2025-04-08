package config

import (
	"os"

	"gopkg.in/yaml.v3"
)

type Config struct {
	WeatherAPIKey string `yaml:"weather_api_key"`
	APINinjasKey  string `yaml:"api_ninjas_key"`
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
