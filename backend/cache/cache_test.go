package cache

import (
	"sync"
	"testing"
	"time"
)

func TestCache(t *testing.T) {
	// Use a very short timeout for testing
	c := NewCache(100 * time.Millisecond)

	// Test basic set and get
	c.Set("key1", "value1")
	if val, exists := c.Get("key1"); !exists || val != "value1" {
		t.Errorf("Expected value1, got %v", val)
	}

	// Test non-existent key
	if val, exists := c.Get("nonexistent"); exists || val != nil {
		t.Errorf("Expected nil, got %v", val)
	}

	// Test cache expiration
	c.Set("key2", "value2")
	time.Sleep(200 * time.Millisecond) // Wait for cache to expire
	if val, exists := c.Get("key2"); exists || val != nil {
		t.Errorf("Expected nil after expiration, got %v", val)
	}
}

func TestCacheConcurrent(t *testing.T) {
	c := NewCache(100 * time.Millisecond)
	var wg sync.WaitGroup
	iterations := 100

	// Test concurrent writes
	wg.Add(iterations)
	for i := 0; i < iterations; i++ {
		i := i // Create a new variable for the goroutine
		go func() {
			defer wg.Done()
			key := string(rune('a' + i%26))
			c.Set(key, i)
		}()
	}
	wg.Wait()

	// Test concurrent reads
	wg.Add(iterations)
	for i := 0; i < iterations; i++ {
		i := i // Create a new variable for the goroutine
		go func() {
			defer wg.Done()
			key := string(rune('a' + i%26))
			c.Get(key)
		}()
	}
	wg.Wait()

	// Verify some values
	for i := 0; i < 26; i++ {
		key := string(rune('a' + i))
		if val, exists := c.Get(key); !exists {
			t.Errorf("Expected value for key %s to exist", key)
		} else if val == nil {
			t.Errorf("Expected non-nil value for key %s", key)
		}
	}
}

func TestCacheClear(t *testing.T) {
	c := NewCache(100 * time.Millisecond)

	// Set some values
	c.Set("key1", "value1")
	c.Set("key2", "value2")

	// Clear the cache
	c.Clear()

	// Verify cache is empty
	if val, exists := c.Get("key1"); exists || val != nil {
		t.Errorf("Expected nil after clear, got %v", val)
	}
	if val, exists := c.Get("key2"); exists || val != nil {
		t.Errorf("Expected nil after clear, got %v", val)
	}
}

func TestCacheStockData(t *testing.T) {
	c := NewCache(100 * time.Millisecond)

	// Define the stock data type
	type StockData struct {
		Symbol               string  `json:"symbol"`
		LongName             string  `json:"longName"`
		Timezone             string  `json:"timezone"`
		ExchangeName         string  `json:"exchangeName"`
		Gmtoffset            int     `json:"gmtoffset"`
		FiftyTwoWeekHigh     float64 `json:"fiftyTwoWeekHigh"`
		FiftyTwoWeekLow      float64 `json:"fiftyTwoWeekLow"`
		RegularMarketDayHigh float64 `json:"regularMarketDayHigh"`
		RegularMarketDayLow  float64 `json:"regularMarketDayLow"`
		PreviousClose        float64 `json:"previousClose"`
		Scale                int     `json:"scale"`
		PriceHint            int     `json:"priceHint"`
	}

	// Test stock data caching
	stockData := StockData{
		Symbol:               "VAS.AX",
		LongName:             "Vanguard Australian Shares Index ETF",
		Timezone:             "AEST",
		ExchangeName:         "ASX",
		Gmtoffset:            36000,
		FiftyTwoWeekHigh:     106.39,
		FiftyTwoWeekLow:      88.64,
		RegularMarketDayHigh: 94.68,
		RegularMarketDayLow:  93.08,
		PreviousClose:        95.4,
		Scale:                3,
		PriceHint:            2,
	}

	// Set stock data in cache
	c.Set("stock:VAS.AX", stockData)

	// Get stock data from cache
	if val, exists := c.Get("stock:VAS.AX"); !exists {
		t.Errorf("Expected stock data to exist in cache")
	} else {
		// Verify the data matches
		if cachedData, ok := val.(StockData); !ok {
			t.Errorf("Expected StockData type, got %T", val)
		} else if cachedData != stockData {
			t.Errorf("Expected stock data to match, got %v", cachedData)
		}
	}

	// Test cache expiration
	time.Sleep(200 * time.Millisecond)
	if val, exists := c.Get("stock:VAS.AX"); exists || val != nil {
		t.Errorf("Expected nil after expiration, got %v", val)
	}
}
