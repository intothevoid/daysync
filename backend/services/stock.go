package services

// GetTestStockInfo returns dummy stock data for test mode
func GetTestStockInfo(symbol string) (interface{}, error) {
	// Return dummy data for test mode
	dummyData := struct {
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
	}{
		Symbol:               symbol,
		LongName:             "Test Stock " + symbol,
		Timezone:             "AEST",
		ExchangeName:         "ASX",
		Gmtoffset:            36000,
		FiftyTwoWeekHigh:     100.0,
		FiftyTwoWeekLow:      80.0,
		RegularMarketDayHigh: 95.0,
		RegularMarketDayLow:  90.0,
		PreviousClose:        92.5,
		Scale:                3,
		PriceHint:            2,
	}

	return dummyData, nil
}
