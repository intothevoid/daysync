package renderers

import (
	"bytes"
	"image"
	"image/png"

	"github.com/srwiley/oksvg"
	"github.com/srwiley/rasterx"
)

// ConvertSVGToPNG converts an SVG string to a PNG image
func ConvertSVGToPNG(svgData string, width, height int) ([]byte, error) {
	// Create an oksvg icon
	icon, err := oksvg.ReadIconStream(bytes.NewReader([]byte(svgData)))
	if err != nil {
		return nil, err
	}

	// Set the icon's dimensions
	icon.SetTarget(0, 0, float64(width), float64(height))

	// Create a new RGBA image
	rgba := image.NewRGBA(image.Rect(0, 0, width, height))

	// Create a rasterizer
	rasterizer := rasterx.NewDasher(width, height, rasterx.NewScannerGV(width, height, rgba, rgba.Bounds()))

	// Draw the icon
	icon.Draw(rasterizer, 1.0)

	// Encode the image to PNG
	var buf bytes.Buffer
	if err := png.Encode(&buf, rgba); err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}