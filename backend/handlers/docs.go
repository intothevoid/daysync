package handlers

import (
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"path"
	"path/filepath"
	"strings"
)

// ServeOpenAPISpec serves the OpenAPI specification file
func ServeOpenAPISpec(w http.ResponseWriter, r *http.Request) {
	wd, err := os.Getwd()
	if err != nil {
		log.Printf("Error getting working directory: %v", err)
		http.Error(w, "Internal server error", http.StatusInternalServerError)
		return
	}
	log.Printf("Working directory: %s", wd)

	specPath := filepath.Join(wd, "handlers", "docs", "openapi.yaml")
	log.Printf("Looking for OpenAPI spec at: %s", specPath)

	data, err := ioutil.ReadFile(specPath)
	if err != nil {
		log.Printf("Error reading OpenAPI spec: %v", err)
		http.Error(w, "Error reading OpenAPI spec", http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/yaml")
	w.Write(data)
}

// ServeSwaggerUI serves the Swagger UI interface
func ServeSwaggerUI(w http.ResponseWriter, r *http.Request) {
	wd, err := os.Getwd()
	if err != nil {
		log.Printf("Error getting working directory: %v", err)
		http.Error(w, "Internal server error", http.StatusInternalServerError)
		return
	}
	log.Printf("Working directory: %s", wd)

	// Get the requested file path
	filePath := r.URL.Path
	log.Printf("Requested path: %s", filePath)

	if filePath == "/docs" || filePath == "/docs/" {
		filePath = "index.html"
		log.Printf("Redirecting to index.html")
	} else {
		// Remove the /docs/ prefix if it exists
		if strings.HasPrefix(filePath, "/docs/") {
			filePath = filePath[len("/docs/"):]
		}
	}

	// Construct the full file path
	fullPath := filepath.Join(wd, "..", "docs", "swagger-ui", filePath)
	log.Printf("Looking for file at: %s", fullPath)

	// Check if the file exists
	if _, err := os.Stat(fullPath); os.IsNotExist(err) {
		log.Printf("File not found: %s", fullPath)
		http.Error(w, "File not found: "+filePath, http.StatusNotFound)
		return
	}

	// Try to read the file
	data, err := ioutil.ReadFile(fullPath)
	if err != nil {
		log.Printf("Error reading file %s: %v", fullPath, err)
		http.Error(w, "Error reading file: "+filePath, http.StatusInternalServerError)
		return
	}

	// Set appropriate content type based on file extension
	contentType := "text/plain"
	switch path.Ext(filePath) {
	case ".html":
		contentType = "text/html"
	case ".css":
		contentType = "text/css"
	case ".js":
		contentType = "application/javascript"
	case ".png":
		contentType = "image/png"
	case ".yaml":
		contentType = "application/yaml"
	}
	log.Printf("Setting content type to: %s for file: %s", contentType, filePath)
	w.Header().Set("Content-Type", contentType)

	w.Write(data)
}
