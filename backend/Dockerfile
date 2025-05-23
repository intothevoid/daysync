# Build stage
FROM golang:1.24.1-alpine AS builder

WORKDIR /app

# Copy go mod and sum files
COPY go.mod go.sum ./

# Download dependencies
RUN go mod download

# Copy source code
COPY . .

# Build the application
RUN CGO_ENABLED=0 GOOS=linux go build -o main .

# Final stage
FROM alpine:latest

WORKDIR /app

# Install timezone data
RUN apk add --no-cache tzdata

# Copy the binary from builder
COPY --from=builder /app/main .
# Copy config file
COPY --from=builder /app/config.yaml .
# Copy data directory
COPY --from=builder /app/data ./data
# Copy testdata directory
COPY --from=builder /app/testdata ./testdata

# Expose port 5173
EXPOSE 5173

# Command to run the executable
CMD ["./main"] 