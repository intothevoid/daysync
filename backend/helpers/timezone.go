package helpers

import (
	"fmt"
	"time"
)

// timezoneMap maps common timezone abbreviations to their IANA timezone names
var timezoneMap = map[string]string{
	// Australian timezones
	"ACDT": "Australia/Adelaide", // Australian Central Daylight Time
	"ACST": "Australia/Darwin",   // Australian Central Standard Time
	"AEDT": "Australia/Sydney",   // Australian Eastern Daylight Time
	"AEST": "Australia/Brisbane", // Australian Eastern Standard Time
	"AWDT": "Australia/Perth",    // Australian Western Daylight Time
	"AWST": "Australia/Perth",    // Australian Western Standard Time

	// US timezones
	"EST": "America/New_York",    // Eastern Standard Time
	"EDT": "America/New_York",    // Eastern Daylight Time
	"CST": "America/Chicago",     // Central Standard Time
	"CDT": "America/Chicago",     // Central Daylight Time
	"MST": "America/Denver",      // Mountain Standard Time
	"MDT": "America/Denver",      // Mountain Daylight Time
	"PST": "America/Los_Angeles", // Pacific Standard Time
	"PDT": "America/Los_Angeles", // Pacific Daylight Time

	// European timezones
	"GMT":  "Europe/London",    // Greenwich Mean Time
	"BST":  "Europe/London",    // British Summer Time
	"CET":  "Europe/Paris",     // Central European Time
	"CEST": "Europe/Paris",     // Central European Summer Time
	"EET":  "Europe/Bucharest", // Eastern European Time
	"EEST": "Europe/Bucharest", // Eastern European Summer Time

	// Asian timezones
	"JST":  "Asia/Tokyo",    // Japan Standard Time
	"KST":  "Asia/Seoul",    // Korea Standard Time
	"CNST": "Asia/Shanghai", // China Standard Time
	"IST":  "Asia/Kolkata",  // India Standard Time

	// UTC
	"UTC": "UTC", // Coordinated Universal Time
}

// GetLocationFromAbbreviation converts a timezone abbreviation to a time.Location
func GetLocationFromAbbreviation(abbr string) (*time.Location, error) {
	ianaName, ok := timezoneMap[abbr]
	if !ok {
		return nil, fmt.Errorf("unsupported timezone abbreviation: %s", abbr)
	}

	loc, err := time.LoadLocation(ianaName)
	if err != nil {
		return nil, fmt.Errorf("error loading timezone: %v", err)
	}

	return loc, nil
}
