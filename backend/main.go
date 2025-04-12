package main

import (
	"flag"
	"log"
	"net/http"
	"os"
	"time"

	"daysync/api/config"
	"daysync/api/handlers"
	"daysync/api/helpers"

	"github.com/gorilla/mux"
)

func main() {
	// Parse command line flags
	icsToJsonFlag := flag.String("ics-to-json", "", "Convert ICS file to JSON format")
	testModeFlag := flag.Bool("test-mode", false, "Run in test mode (use dummy data)")
	flag.Parse()

	// If --ics-to-json flag is provided, convert the file and exit
	if *icsToJsonFlag != "" {
		if err := helpers.IcsToJson(*icsToJsonFlag); err != nil {
			log.Fatalf("Error converting ICS to JSON: %v", err)
		}
		log.Printf("Successfully converted %s to JSON", *icsToJsonFlag)
		os.Exit(0)
	}

	// Load configuration
	if err := config.LoadConfig(); err != nil {
		log.Fatalf("Error loading config: %v", err)
	}

	// Set test mode if flag is provided
	if *testModeFlag {
		log.Println("Running in test mode - using dummy data")
		handlers.SetTestMode(true)
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
