#!/bin/bash

# Create docs directory if it doesn't exist
mkdir -p ../docs/swagger-ui

# Download Swagger UI
curl -L https://github.com/swagger-api/swagger-ui/archive/refs/tags/v5.11.0.tar.gz | tar xz

# Copy Swagger UI files
cp -r swagger-ui-5.11.0/dist/* ../docs/swagger-ui/

# Clean up
rm -rf swagger-ui-5.11.0

# Update Swagger UI configuration
cat > ../docs/swagger-ui/swagger-initializer.js << 'EOL'
window.onload = () => {
  window.ui = SwaggerUIBundle({
    url: "/docs/openapi.yaml",
    dom_id: '#swagger-ui',
    deepLinking: true,
    presets: [
      SwaggerUIBundle.presets.apis,
      SwaggerUIBundle.SwaggerUIStandalonePreset
    ],
    layout: "BaseLayout",
    docExpansion: "list",
    defaultModelsExpandDepth: -1,
    displayRequestDuration: true,
    filter: true,
    tryItOutEnabled: true
  });
};
EOL

echo "Swagger UI setup complete!" 