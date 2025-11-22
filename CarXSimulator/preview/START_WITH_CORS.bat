@echo off
echo ========================================
echo   CarX Simulator - CORS Enabled Server
echo ========================================
echo.
echo Starting server with CORS support...
echo This allows GLB model loading!
echo.
echo Open: http://localhost:8080/final-demo.html
echo.
echo Press Ctrl+C to stop
echo.

cd /d "%~dp0"
python serve.py

pause
