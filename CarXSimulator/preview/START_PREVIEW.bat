@echo off
echo ========================================
echo   CarX Physics Simulator - Preview
echo    CREDIT TO - AAYUSH KUSHWAHA
echo ========================================
echo.
set "PORT=%PORT%"
if "%PORT%"=="" set "PORT=8080"

echo Starting local server on port %PORT%...
echo Open your browser to: http://localhost:%PORT%/final-demo.html
echo.
echo Press Ctrl+C to stop the server
echo.

cd /d "%~dp0"
python serve.py

pause
