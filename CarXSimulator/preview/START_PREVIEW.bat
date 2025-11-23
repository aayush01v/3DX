@echo off
echo ========================================
echo   CarX Physics Simulator - Preview
echo    CREDIT TO - AAYUSH KUSHWAHA
echo ========================================
echo.
echo Starting local server...
echo Open your browser to: http://localhost:8080/final-demo.html
echo.
echo Press Ctrl+C to stop the server
echo.

cd /d "%~dp0"
python -m http.server 8080

pause
