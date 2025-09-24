package handlers

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"net/http/httptest"

	"daysync/api/config"
	"daysync/api/models"
	"daysync/api/renderers"
)

func GetKindleMotoGP(w http.ResponseWriter, r *http.Request) {
	cacheKey := "kindle:motogp"
	if cached, exists := apiCache.Get(cacheKey); exists {
		log.Printf("[CACHE HIT] Returning cached Kindle MotoGP image")
		w.Header().Set("Content-Type", "image/png")
		w.Write(cached.([]byte))
		return
	}

	// Create a new request
	req, err := http.NewRequest("GET", "/api/motogpnextrace", nil)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	// Create a response recorder
	rr := httptest.NewRecorder()

	// Call the handler
	GetNextMotoGPRace(rr, req)

	// Check the status code
	if status := rr.Code; status != http.StatusOK {
		http.Error(w, rr.Body.String(), status)
		return
	}

	var race models.Race
	if err := json.Unmarshal(rr.Body.Bytes(), &race); err != nil {
		http.Error(w, "Error parsing race data", http.StatusInternalServerError)
		return
	}

	svg, err := renderers.RenderMotoGPSVG(race)
	if err != nil {
		http.Error(w, "Error rendering SVG", http.StatusInternalServerError)
		return
	}

	width, height := config.GetKindleImageConfig()
	png, err := renderers.ConvertSVGToPNG(svg, width, height)
	if err != nil {
		log.Printf("Error converting SVG to PNG: %v", err)
		http.Error(w, "Error converting to PNG", http.StatusInternalServerError)
		return
	}

	apiCache.Set(cacheKey, png)
	w.Header().Set("Content-Type", "image/png")
	w.Write(png)
}

func GetKindleFormula1(w http.ResponseWriter, r *http.Request) {
	cacheKey := "kindle:formula1"
	if cached, exists := apiCache.Get(cacheKey); exists {
		log.Printf("[CACHE HIT] Returning cached Kindle Formula 1 image")
		w.Header().Set("Content-Type", "image/png")
		w.Write(cached.([]byte))
		return
	}

	req, err := http.NewRequest("GET", "/api/formula1nextrace", nil)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	rr := httptest.NewRecorder()
	GetNextFormula1Race(rr, req)

	if status := rr.Code; status != http.StatusOK {
		http.Error(w, rr.Body.String(), status)
		return
	}

	var race models.Race
	if err := json.Unmarshal(rr.Body.Bytes(), &race); err != nil {
		http.Error(w, "Error parsing race data", http.StatusInternalServerError)
		return
	}

	svg, err := renderers.RenderFormula1SVG(race)
	if err != nil {
		http.Error(w, "Error rendering SVG", http.StatusInternalServerError)
		return
	}

	width, height := config.GetKindleImageConfig()
	png, err := renderers.ConvertSVGToPNG(svg, width, height)
	if err != nil {
		http.Error(w, "Error converting to PNG", http.StatusInternalServerError)
		return
	}

	apiCache.Set(cacheKey, png)
	w.Header().Set("Content-Type", "image/png")
	w.Write(png)
}

func GetKindleWeather(w http.ResponseWriter, r *http.Request) {
	cacheKey := "kindle:weather"
	if cached, exists := apiCache.Get(cacheKey); exists {
		log.Printf("[CACHE HIT] Returning cached Kindle weather image")
		w.Header().Set("Content-Type", "image/png")
		w.Write(cached.([]byte))
		return
	}

	req, err := http.NewRequest("GET", "/api/weather?location=auto:ip", nil)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	rr := httptest.NewRecorder()
	GetWeather(rr, req)

	if status := rr.Code; status != http.StatusOK {
		http.Error(w, rr.Body.String(), status)
		return
	}

	var weather models.Weather
	if err := json.Unmarshal(rr.Body.Bytes(), &weather); err != nil {
		http.Error(w, "Error parsing weather data", http.StatusInternalServerError)
		return
	}

	svg, err := renderers.RenderWeatherSVG(weather)
	if err != nil {
		http.Error(w, "Error rendering SVG", http.StatusInternalServerError)
		return
	}

	width, height := config.GetKindleImageConfig()
	png, err := renderers.ConvertSVGToPNG(svg, width, height)
	if err != nil {
		http.Error(w, "Error converting to PNG", http.StatusInternalServerError)
		return
	}

	apiCache.Set(cacheKey, png)
	w.Header().Set("Content-Type", "image/png")
	w.Write(png)
}

func GetKindleCrypto(w http.ResponseWriter, r *http.Request) {
	cacheKey := "kindle:crypto"
	if cached, exists := apiCache.Get(cacheKey); exists {
		log.Printf("[CACHE HIT] Returning cached Kindle crypto image")
		w.Header().Set("Content-Type", "image/png")
		w.Write(cached.([]byte))
		return
	}

	symbols := []string{"BTCUSD", "ETHUSD", "SOLUSD"}
	var cryptos []models.CryptoPrice

	for _, symbol := range symbols {
		req, err := http.NewRequest("GET", fmt.Sprintf("/api/crypto?symbol=%s", symbol), nil)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		rr := httptest.NewRecorder()
		GetCryptoPrice(rr, req)

		if status := rr.Code; status != http.StatusOK {
			log.Printf("Error getting crypto price for %s: %s", symbol, rr.Body.String())
			continue
		}

		var crypto models.CryptoPrice
		if err := json.Unmarshal(rr.Body.Bytes(), &crypto); err != nil {
			log.Printf("Error parsing crypto data for %s: %v", symbol, err)
			continue
		}
		cryptos = append(cryptos, crypto)
	}

	if len(cryptos) == 0 {
		http.Error(w, "Could not fetch any crypto data", http.StatusInternalServerError)
		return
	}

	svg, err := renderers.RenderCryptoSVG(cryptos)
	if err != nil {
		http.Error(w, "Error rendering SVG", http.StatusInternalServerError)
		return
	}

	width, height := config.GetKindleImageConfig()
	png, err := renderers.ConvertSVGToPNG(svg, width, height)
	if err != nil {
		http.Error(w, "Error converting to PNG", http.StatusInternalServerError)
		return
	}

	apiCache.Set(cacheKey, png)
	w.Header().Set("Content-Type", "image/png")
	w.Write(png)
}
func GetKindleFinance(w http.ResponseWriter, r *http.Request) {
	cacheKey := "kindle:finance"
	if cached, exists := apiCache.Get(cacheKey); exists {
		log.Printf("[CACHE HIT] Returning cached Kindle finance image")
		w.Header().Set("Content-Type", "image/png")
		w.Write(cached.([]byte))
		return
	}

	symbols := []string{"NDQ.AX", "VAS.AX", "VGS.AX"}
	var stocks []models.Stock

	for _, symbol := range symbols {
		req, err := http.NewRequest("GET", fmt.Sprintf("/api/finance?symbol=%s", symbol), nil)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		rr := httptest.NewRecorder()
		GetStockInfo(rr, req)

		if status := rr.Code; status != http.StatusOK {
			log.Printf("Error getting stock info for %s: %s", symbol, rr.Body.String())
			continue
		}

		var stock models.Stock
		if err := json.Unmarshal(rr.Body.Bytes(), &stock); err != nil {
			log.Printf("Error parsing stock data for %s: %v", symbol, err)
			continue
		}
		stocks = append(stocks, stock)
	}

	if len(stocks) == 0 {
		http.Error(w, "Could not fetch any stock data", http.StatusInternalServerError)
		return
	}

	svg, err := renderers.RenderFinanceSVG(stocks)
	if err != nil {
		http.Error(w, "Error rendering SVG", http.StatusInternalServerError)
		return
	}

	width, height := config.GetKindleImageConfig()
	png, err := renderers.ConvertSVGToPNG(svg, width, height)
	if err != nil {
		http.Error(w, "Error converting to PNG", http.StatusInternalServerError)
		return
	}

	apiCache.Set(cacheKey, png)
	w.Header().Set("Content-Type", "image/png")
	w.Write(png)
}
func GetKindleNews(w http.ResponseWriter, r *http.Request) {
	cacheKey := "kindle:news"
	if cached, exists := apiCache.Get(cacheKey); exists {
		log.Printf("[CACHE HIT] Returning cached Kindle news image")
		w.Header().Set("Content-Type", "image/png")
		w.Write(cached.([]byte))
		return
	}

	req, err := http.NewRequest("GET", "/api/news", nil)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	rr := httptest.NewRecorder()
	GetNews(rr, req)

	if status := rr.Code; status != http.StatusOK {
		http.Error(w, rr.Body.String(), status)
		return
	}

	var news models.NewsResponse
	if err := json.Unmarshal(rr.Body.Bytes(), &news); err != nil {
		http.Error(w, "Error parsing news data", http.StatusInternalServerError)
		return
	}

	svg, err := renderers.RenderNewsSVG(news)
	if err != nil {
		http.Error(w, "Error rendering SVG", http.StatusInternalServerError)
		return
	}

	width, height := config.GetKindleImageConfig()
	png, err := renderers.ConvertSVGToPNG(svg, width, height)
	if err != nil {
		http.Error(w, "Error converting to PNG", http.StatusInternalServerError)
		return
	}

	apiCache.Set(cacheKey, png)
	w.Header().Set("Content-Type", "image/png")
	w.Write(png)
}