package renderers

import (
	"bytes"
	"fmt"
	"os/exec"
)

// ConvertSVGToPNG converts an SVG string to a PNG image using rsvg-convert
func ConvertSVGToPNG(svgData string, width, height int) ([]byte, error) {
	// Command to execute
	cmd := exec.Command("rsvg-convert",
		"--width", fmt.Sprintf("%d", width),
		"--height", fmt.Sprintf("%d", height),
		"--format", "png",
		"/dev/stdin")

	// Set the command's standard input to the SVG data
	cmd.Stdin = bytes.NewReader([]byte(svgData))

	// Create buffers to capture stdout and stderr
	var out bytes.Buffer
	var stderr bytes.Buffer
	cmd.Stdout = &out
	cmd.Stderr = &stderr

	// Run the command
	err := cmd.Run()
	if err != nil {
		return nil, fmt.Errorf("rsvg-convert failed: %v\nStderr: %s", err, stderr.String())
	}

	return out.Bytes(), nil
}

