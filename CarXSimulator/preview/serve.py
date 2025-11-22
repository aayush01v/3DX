#!/usr/bin/env python3
"""
Simple HTTP server with CORS enabled for loading GLB files
"""
import http.server
import socketserver

PORT = 8080

class CORSRequestHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', '*')
        self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate')
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(200)
        self.end_headers()

with socketserver.TCPServer(("", PORT), CORSRequestHandler) as httpd:
    print(f"✓ Server running at http://localhost:{PORT}/")
    print(f"✓ Open: http://localhost:{PORT}/final-demo.html")
    print("✓ CORS enabled for GLB loading")
    print("\nPress Ctrl+C to stop")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n\n✓ Server stopped")
