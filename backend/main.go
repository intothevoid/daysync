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
	api.HandleFunc("/motogp", handlers.GetMotoGPSeason).Methods("GET")
	api.HandleFunc("/motogpnextrace", handlers.GetNextMotoGPRace).Methods("GET")
	api.HandleFunc("/weather", handlers.GetWeather).Methods("GET")
	api.HandleFunc("/crypto", handlers.GetCryptoPrice).Methods("GET")
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
		Addr:         "0.0.0.0:5173",
		WriteTimeout: 15 * time.Second,
		ReadTimeout:  15 * time.Second,
	}

	log.Println("Starting server on :5173")
	log.Fatal(srv.ListenAndServe())
}
