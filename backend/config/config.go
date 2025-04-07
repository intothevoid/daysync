package config

import (
	"os"
	"time"

	"gopkg.in/yaml.v3"
)

type Config struct {
	Timezone string `yaml:"timezone"`
}

var cfg *Config

func LoadConfig() error {
	data, err := os.ReadFile("config.yaml")
	if err != nil {
		return err
	}

	cfg = &Config{}
	return yaml.Unmarshal(data, cfg)
}

func GetTimezone() *time.Location {
	if cfg == nil {
		LoadConfig()
	}

	loc, err := time.LoadLocation(cfg.Timezone)
	if err != nil {
		// Fallback to UTC if timezone is invalid
		return time.UTC
	}
	return loc
}
