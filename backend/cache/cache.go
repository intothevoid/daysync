package cache

import (
	"sync"
	"time"
)

// CacheEntry represents a cached API response with its timestamp
type CacheEntry struct {
	Data      interface{}
	Timestamp time.Time
}

// Cache is a thread-safe cache store for API responses
type Cache struct {
	mu      sync.RWMutex
	store   map[string]CacheEntry
	timeout time.Duration
}

// NewCache creates a new cache with the specified timeout duration
func NewCache(timeout time.Duration) *Cache {
	return &Cache{
		store:   make(map[string]CacheEntry),
		timeout: timeout,
	}
}

// Get retrieves a value from the cache if it exists and is not stale
func (c *Cache) Get(key string) (interface{}, bool) {
	c.mu.RLock()
	defer c.mu.RUnlock()

	entry, exists := c.store[key]
	if !exists {
		return nil, false
	}

	// Check if the entry is stale
	if time.Since(entry.Timestamp) > c.timeout {
		return nil, false
	}

	return entry.Data, true
}

// Set stores a value in the cache with the current timestamp
func (c *Cache) Set(key string, value interface{}) {
	c.mu.Lock()
	defer c.mu.Unlock()

	c.store[key] = CacheEntry{
		Data:      value,
		Timestamp: time.Now(),
	}
}

// Clear removes all entries from the cache
func (c *Cache) Clear() {
	c.mu.Lock()
	defer c.mu.Unlock()

	c.store = make(map[string]CacheEntry)
}
