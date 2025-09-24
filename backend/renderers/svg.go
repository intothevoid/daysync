package renderers

import (
	"bytes"
	"daysync/api/models"
	"html/template"
	"io/ioutil"
	"path/filepath"
)

// RenderMotoGPSVG renders the next MotoGP race data to an SVG template
func RenderMotoGPSVG(data models.Race) (string, error) {
	return renderSVGTemplate("motogp", data)
}

// RenderFormula1SVG renders the next Formula 1 race data to an SVG template
func RenderFormula1SVG(data models.Race) (string, error) {
	return renderSVGTemplate("formula1", data)
}

// RenderWeatherSVG renders weather data to an SVG template
func RenderWeatherSVG(data models.Weather) (string, error) {
	return renderSVGTemplate("weather", data)
}

// RenderCryptoSVG renders crypto data to an SVG template
func RenderCryptoSVG(data []models.CryptoPrice) (string, error) {
	return renderSVGTemplate("crypto", data)
}

// RenderFinanceSVG renders finance data to an SVG template
func RenderFinanceSVG(data []models.Stock) (string, error) {
	return renderSVGTemplate("finance", data)
}

// RenderNewsSVG renders news data to an SVG template
func RenderNewsSVG(data models.NewsResponse) (string, error) {
	return renderSVGTemplate("news", data)
}

// renderSVGTemplate is a helper function to render a specific SVG template
func renderSVGTemplate(templateName string, data interface{}) (string, error) {
	// Read the SVG template file
	templatePath := filepath.Join("templates", templateName+".svg")
	templateBytes, err := ioutil.ReadFile(templatePath)
	if err != nil {
		return "", err
	}

	funcMap := template.FuncMap{
		"add": func(a, b int) int {
			return a + b
		},
		"mul": func(a, b int) int {
			return a * b
		},
	}

	// Create a new template and parse the SVG content
	tmpl, err := template.New(templateName).Funcs(funcMap).Parse(string(templateBytes))
	if err != nil {
		return "", err
	}

	// Execute the template with the provided data
	var renderedSVG bytes.Buffer
	if err := tmpl.Execute(&renderedSVG, data); err != nil {
		return "", err
	}

	return renderedSVG.String(), nil
}
