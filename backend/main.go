package main

import (
	"log"
	"net/http"
	"time"

	"daysync/api/config"
	"daysync/api/handlers"

	"github.com/gorilla/mux"
)

func main() {
	// Load configuration
	if err := config.LoadConfig(); err != nil {
		log.Fatalf("Error loading config: %v", err)
	}

	r := mux.NewRouter()

	// API routes
	api := r.PathPrefix("/api").Subrouter()
	api.HandleFunc("/motogp/{year}", handlers.GetMotoGPSeason).Methods("GET")
	api.HandleFunc("/motogp/next", handlers.GetNextMotoGPRace).Methods("GET")
	api.HandleFunc("/weather/{location}", handlers.GetWeather).Methods("GET")
	api.HandleFunc("/crypto/{coin}", handlers.GetCryptoPrice).Methods("GET")
	api.HandleFunc("/news", handlers.GetNews).Methods("GET")

	// Add CORS middleware
	r.Use(func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			w.Header().Set("Access-Control-Allow-Origin", "*")
			w.Header().Set("Access-Control-Allow-Methods", "GET, OPTIONS")
			w.Header().Set("Access-Control-Allow-Headers", "Content-Type")

			if r.Method == "OPTIONS" {
				w.WriteHeader(http.StatusOK)
				return
			}

			next.ServeHTTP(w, r)
		})
	})

	// Server configuration
	srv := &http.Server{
		Handler:      r,
		Addr:         "0.0.0.0:8080",
		WriteTimeout: 15 * time.Second,
		ReadTimeout:  15 * time.Second,
	}

	log.Println("Starting server on :8080")
	log.Fatal(srv.ListenAndServe())
}

func getMotoGPSeason(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	year := vars["year"]
	// TODO: Implement MotoGP season data retrieval
	w.Write([]byte("MotoGP season " + year))
}

func getWeather(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	location := vars["location"]
	// TODO: Implement weather data retrieval
	w.Write([]byte("Weather for " + location))
}
