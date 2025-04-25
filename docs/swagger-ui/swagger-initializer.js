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
