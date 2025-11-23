#!/bin/bash

echo "========================================"
echo "  CarX Physics Simulator - Preview"
echo "========================================"
echo ""
PORT=${PORT:-8080}
echo "Starting local server on port $PORT..."
echo "Open your browser to: http://localhost:$PORT/final-demo.html"
echo ""
echo "Press Ctrl+C to stop the server"
echo ""

cd "$(dirname "$0")"

# Try Python 3 first, then Python 2
if command -v python3 &> /dev/null; then
    PORT=$PORT python3 serve.py
elif command -v python &> /dev/null; then
    PORT=$PORT python serve.py
else
    echo "Error: Python not found. Please install Python to run the preview."
    exit 1
fi
