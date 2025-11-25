#!/usr/bin/env python3
"""Simple HTTP server with CORS enabled for loading GLB files.

The server prefers the port provided by the ``PORT`` (or ``PREVIEW_PORT``)
environment variable, otherwise it falls back to ``8080``. If that port is
already in use, it automatically selects the next available free port to avoid
crashing with ``OSError: [Errno 98] Address already in use``.
"""

import errno
import http.server
import os
import socket
import socketserver
from typing import Tuple, Type

DEFAULT_PORT = 8080


def parse_port() -> int:
    env_port = os.getenv("PORT") or os.getenv("PREVIEW_PORT")
    if not env_port:
        return DEFAULT_PORT

    try:
        value = int(env_port)
        if value < 1 or value > 65535:
            raise ValueError
        return value
    except ValueError:
        print(f"⚠️  Invalid port '{env_port}', falling back to {DEFAULT_PORT}")
        return DEFAULT_PORT


class CORSRequestHandler(http.server.SimpleHTTPRequestHandler):
    """Simple request handler that serves files from the preview folder.

    The preview app sometimes requests assets with a leading "/preview/"
    prefix (for example when deployed behind a rewrite rule). When we run
    the local helper from the ``preview`` directory, those absolute paths
    would normally resolve to ``<cwd>/preview/...`` which does not exist
    and results in confusing 404s for core assets like ``three.min.js``.

    To make the local experience resilient, we transparently strip a
    leading "/preview" segment before resolving the path on disk. This
    keeps the URLs that work in hosted environments while ensuring they
    load locally without duplicated directory prefixes.
    """

    def translate_path(self, path):
        # Allow requests such as /preview/vendor/... to resolve when the
        # server is started inside the preview directory.
        if path.startswith("/preview/"):
            path = path[len("/preview") :]

        return super().translate_path(path)

    def end_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "*")
        self.send_header("Cache-Control", "no-store, no-cache, must-revalidate")
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(200)
        self.end_headers()


class ReusableTCPServer(socketserver.TCPServer):
    # Allows re-binding to the same port immediately after a previous server exits
    allow_reuse_address = True


def get_server(
    handler_class: Type[http.server.SimpleHTTPRequestHandler],
    preferred_port: int,
) -> Tuple[int, socketserver.TCPServer]:
    try:
        return preferred_port, ReusableTCPServer(("", preferred_port), handler_class)
    except OSError as exc:
        if exc.errno != errno.EADDRINUSE:
            raise

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.bind(("", 0))
            fallback_port = sock.getsockname()[1]

        print(
            f"⚠️  Port {preferred_port} is busy, switching to available port {fallback_port}."
        )
        return fallback_port, ReusableTCPServer(("", fallback_port), handler_class)


if __name__ == "__main__":
    port = parse_port()
    port, httpd = get_server(CORSRequestHandler, port)

    print(f"✓ Server running at http://localhost:{port}/")
    print(f"✓ Open: http://localhost:{port}/final-demo.html")
    print("✓ CORS enabled for GLB loading")
    print("\nPress Ctrl+C to stop")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n\n✓ Server stopped")
