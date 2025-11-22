#!/bin/bash

echo "========================================"
echo "  CarX Physics Simulator - Preview"
echo "========================================"
echo ""
echo "Starting local server..."
echo "Open your browser to: http://localhost:8080/demo.html"
echo ""
echo "Press Ctrl+C to stop the server"
echo ""

cd "$(dirname "$0")"

# Try Python 3 first, then Python 2
if command -v python3 &> /dev/null; then
    python3 -m http.server 8080
elif command -v python &> /dev/null; then
    python -m http.server 8080
else
    echo "Error: Python not found. Please install Python to run the preview."
    exit 1
fi
