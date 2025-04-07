package helpers

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	ical "github.com/arran4/golang-ical"
)

type Race struct {
	Round    int      `json:"round"`
	Name     string   `json:"name"`
	Location string   `json:"location"`
	Country  string   `json:"country"`
	Circuit  string   `json:"circuit"`
	Date     string   `json:"date"`
	Sessions Sessions `json:"sessions"`
}

type Sessions struct {
	Q1     string `json:"q1"`
	Q2     string `json:"q2"`
	Sprint string `json:"sprint"`
	Race   string `json:"race"`
}

type Calendar struct {
	Year  int    `json:"year"`
	Races []Race `json:"races"`
}

// IcsToJson converts an ICS file to JSON format
func IcsToJson(icsFileName string) error {
	// Read the ICS file
	file, err := os.Open(icsFileName)
	if err != nil {
		return fmt.Errorf("error opening ICS file: %v", err)
	}
	defer file.Close()

	// Parse the ICS file
	cal, err := ical.ParseCalendar(file)
	if err != nil {
		return fmt.Errorf("error parsing ICS file: %v", err)
	}

	// Extract year from the first event
	var year int
	if len(cal.Events()) > 0 {
		startTime, err := cal.Events()[0].GetStartAt()
		if err == nil {
			year = startTime.Year()
		}
	}

	// Create calendar structure
	calendar := Calendar{
		Year:  year,
		Races: make([]Race, 0),
	}

	// Process each event
	for i, event := range cal.Events() {
		startTime, err := event.GetStartAt()
		if err != nil {
			continue
		}

		endTime, err := event.GetEndAt()
		if err != nil {
			continue
		}

		// Extract race information from summary
		summary := event.GetProperty(ical.ComponentPropertySummary).Value
		raceInfo := extractRaceInfo(summary)

		// Create race sessions
		sessions := Sessions{
			Q1:     startTime.Format(time.RFC3339),
			Q2:     startTime.Add(35 * time.Minute).Format(time.RFC3339),
			Sprint: startTime.Add(24 * time.Hour).Format(time.RFC3339),
			Race:   endTime.Format(time.RFC3339),
		}

		// Create race entry
		race := Race{
			Round:    i + 1,
			Name:     raceInfo.name,
			Location: raceInfo.location,
			Country:  raceInfo.country,
			Circuit:  raceInfo.circuit,
			Date:     startTime.Format("2006-01-02"),
			Sessions: sessions,
		}

		calendar.Races = append(calendar.Races, race)
	}

	// Create JSON file name
	jsonFileName := strings.TrimSuffix(icsFileName, filepath.Ext(icsFileName)) + ".json"

	// Write JSON file
	jsonData, err := json.MarshalIndent(calendar, "", "  ")
	if err != nil {
		return fmt.Errorf("error marshaling JSON: %v", err)
	}

	err = os.WriteFile(jsonFileName, jsonData, 0644)
	if err != nil {
		return fmt.Errorf("error writing JSON file: %v", err)
	}

	return nil
}

type raceInfo struct {
	name     string
	location string
	country  string
	circuit  string
}

// extractRaceInfo extracts race information from the summary
func extractRaceInfo(summary string) raceInfo {
	// This is a simplified version - you might want to enhance this
	// with more sophisticated parsing or a lookup table
	parts := strings.Split(summary, " ")
	if len(parts) < 2 {
		return raceInfo{name: summary}
	}

	// Basic mapping - you should expand this with more accurate data
	info := raceInfo{
		name: summary,
	}

	// Add more specific mappings based on the race name
	switch {
	case strings.Contains(summary, "Thailand"):
		info.location = "Buriram"
		info.country = "Thailand"
		info.circuit = "Chang International Circuit"
	case strings.Contains(summary, "Argentina"):
		info.location = "Termas de Río Hondo"
		info.country = "Argentina"
		info.circuit = "Autódromo Termas de Río Hondo"
	case strings.Contains(summary, "Americas"):
		info.location = "Austin"
		info.country = "United States"
		info.circuit = "Circuit of The Americas"
	case strings.Contains(summary, "Qatar"):
		info.location = "Lusail"
		info.country = "Qatar"
		info.circuit = "Lusail International Circuit"
	case strings.Contains(summary, "España"):
		info.location = "Jerez"
		info.country = "Spain"
		info.circuit = "Circuito de Jerez"
	case strings.Contains(summary, "France"):
		info.location = "Le Mans"
		info.country = "France"
		info.circuit = "Bugatti Circuit"
	case strings.Contains(summary, "British"):
		info.location = "Silverstone"
		info.country = "United Kingdom"
		info.circuit = "Silverstone Circuit"
	case strings.Contains(summary, "Aragón"):
		info.location = "Alcañiz"
		info.country = "Spain"
		info.circuit = "MotorLand Aragón"
	case strings.Contains(summary, "Italia"):
		info.location = "Mugello"
		info.country = "Italy"
		info.circuit = "Mugello Circuit"
	case strings.Contains(summary, "Assen"):
		info.location = "Assen"
		info.country = "Netherlands"
		info.circuit = "TT Circuit Assen"
	case strings.Contains(summary, "Deutschland"):
		info.location = "Sachsenring"
		info.country = "Germany"
		info.circuit = "Sachsenring"
	case strings.Contains(summary, "České Republiky"):
		info.location = "Brno"
		info.country = "Czech Republic"
		info.circuit = "Brno Circuit"
	case strings.Contains(summary, "Österreich"):
		info.location = "Spielberg"
		info.country = "Austria"
		info.circuit = "Red Bull Ring"
	case strings.Contains(summary, "Hungary"):
		info.location = "Mogyoród"
		info.country = "Hungary"
		info.circuit = "Hungaroring"
	case strings.Contains(summary, "Catalunya"):
		info.location = "Montmeló"
		info.country = "Spain"
		info.circuit = "Circuit de Barcelona-Catalunya"
	case strings.Contains(summary, "San Marino"):
		info.location = "Misano"
		info.country = "San Marino"
		info.circuit = "Misano World Circuit Marco Simoncelli"
	case strings.Contains(summary, "Japan"):
		info.location = "Motegi"
		info.country = "Japan"
		info.circuit = "Twin Ring Motegi"
	case strings.Contains(summary, "Indonesia"):
		info.location = "Lombok"
		info.country = "Indonesia"
		info.circuit = "Mandalika International Street Circuit"
	case strings.Contains(summary, "Australia"):
		info.location = "Phillip Island"
		info.country = "Australia"
		info.circuit = "Phillip Island Circuit"
	case strings.Contains(summary, "Malaysia"):
		info.location = "Sepang"
		info.country = "Malaysia"
		info.circuit = "Sepang International Circuit"
	case strings.Contains(summary, "Portugal"):
		info.location = "Portimão"
		info.country = "Portugal"
		info.circuit = "Autódromo Internacional do Algarve"
	case strings.Contains(summary, "Valenciana"):
		info.location = "Valencia"
		info.country = "Spain"
		info.circuit = "Circuit Ricardo Tormo"
	default:
		// Default values if no specific mapping is found
		info.location = "Unknown"
		info.country = "Unknown"
		info.circuit = "Unknown"
	}

	return info
}
