# 🚀 Cross-Platform Deployment Guide

## Overview
CarX Physics Simulator can be deployed across multiple platforms:
- **Web** (Browser) - Instant preview, no installation
- **Desktop** (Windows, macOS, Linux) - Native C++ application
- **Mobile** (iOS, Android) - React Native or Flutter integration
- **Embedded** - WebAssembly integration in existing apps

---

## 📱 Platform-Specific Deployment

### 1. Web Deployment (Easiest - Recommended)

#### Option A: Static Hosting (GitHub Pages, Netlify, Vercel)

**Files to Deploy:**
```
preview/
├── final-demo.html
├── lowpoly_game_model.glb
└── (optional) serve.py
```

**Steps:**

1. **GitHub Pages:**
```bash
# Push to GitHub
git add .
git commit -m "Deploy CarX Simulator"
git push origin main

# Enable GitHub Pages
# Go to: Settings → Pages → Source: main branch → /preview folder
# Your site: https://username.github.io/CarXSimulator/preview/final-demo.html
```

2. **Netlify:**
```bash
# Install Netlify CLI
npm install -g netlify-cli

# Deploy
cd preview
netlify deploy --prod

# Or drag & drop the preview folder to netlify.com
```

3. **Vercel:**
```bash
# Install Vercel CLI
npm install -g vercel

# Deploy from repository root using the included vercel.json
npm run deploy:vercel

# Or use the CLI manually
vercel --prebuilt --prod --confirm
```

> **Note:** `vercel.json` is preconfigured to serve the static `preview/` folder
> (rewriting `/` to `/final-demo.html`) and skips unnecessary installs/builds so
> you can deploy the prebuilt preview assets directly.

> **Monorepo tip:** If your Vercel project points at the repository root (and
> not the `CarXSimulator/` subdirectory), the root-level `vercel.json` mirrors
> this configuration and targets `CarXSimulator/preview` so you can deploy
> without changing the project settings.

> The repo is preconfigured for static hosting on Vercel: `vercel.json` points to the
> `preview/` output, rewrites `/` to `final-demo.html`, and sets cache headers for
> large assets like `.glb` and `.exr` models.

#### Option B: Self-Hosted Server

**Requirements:**
- Web server (Apache, Nginx, or Node.js)
- HTTPS (for WebGL features)

**Apache (.htaccess):**
```apache
# Enable CORS for GLB model
<FilesMatch "\.(glb|gltf)$">
    Header set Access-Control-Allow-Origin "*"
</FilesMatch>

# Enable compression
AddOutputFilterByType DEFLATE text/html text/css application/javascript
```

**Nginx (nginx.conf):**
```nginx
server {
    listen 80;
    server_name your-domain.com;
    
    root /var/www/carx-simulator/preview;
    index final-demo.html;
    
    location ~* \.(glb|gltf)$ {
        add_header Access-Control-Allow-Origin *;
    }
    
    gzip on;
    gzip_types text/html text/css application/javascript;
}
```

**Node.js Server:**
```javascript
// server.js
const express = require('express');
const app = express();

app.use(express.static('preview'));
app.get('/', (req, res) => {
    res.sendFile(__dirname + '/preview/final-demo.html');
});

app.listen(3000, () => {
    console.log('CarX Simulator running on http://localhost:3000');
});
```

---

### 2. Desktop Application (Native C++)

#### Windows

**Requirements:**
- Visual Studio 2022 or MinGW
- CMake 3.20+
- OpenGL 4.5+

**Build Steps:**
```bash
# Clone repository
git clone https://github.com/yourusername/CarXSimulator.git
cd CarXSimulator

# Create build directory
mkdir build
cd build

# Generate Visual Studio project
cmake .. -G "Visual Studio 17 2022"

# Build
cmake --build . --config Release

# Run
./Release/CarXSimulator.exe
```

**Distribution:**
```bash
# Create installer with NSIS
makensis installer.nsi

# Or create portable ZIP
7z a CarXSimulator-Windows.zip Release/
```

#### macOS

**Requirements:**
- Xcode Command Line Tools
- CMake 3.20+
- OpenGL 4.1+

**Build Steps:**
```bash
# Install dependencies
brew install cmake glew glfw

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4

# Create app bundle
cmake --build . --target package
```

**Distribution:**
```bash
# Create DMG
hdiutil create -volname "CarX Simulator" -srcfolder CarXSimulator.app -ov -format UDZO CarXSimulator.dmg
```

#### Linux

**Requirements:**
- GCC 11+ or Clang 14+
- CMake 3.20+
- OpenGL 4.5+

**Build Steps:**
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential cmake libgl1-mesa-dev libglew-dev libglfw3-dev

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Install
sudo make install
```

**Distribution:**
```bash
# Create AppImage
linuxdeploy --appdir AppDir --executable CarXSimulator --create-desktop-file --output appimage

# Or create DEB package
cpack -G DEB
```

---

### 3. Mobile Deployment

#### React Native Integration

**Setup:**
```bash
# Create React Native project
npx react-native init CarXSimulatorApp
cd CarXSimulatorApp

# Install WebView
npm install react-native-webview
```

**App.js:**
```javascript
import React from 'react';
import { WebView } from 'react-native-webview';
import { SafeAreaView, StyleSheet } from 'react-native';

export default function App() {
  return (
    <SafeAreaView style={styles.container}>
      <WebView
        source={{ uri: 'https://your-domain.com/preview/final-demo.html' }}
        // Or use local file:
        // source={require('./assets/final-demo.html')}
        javaScriptEnabled={true}
        domStorageEnabled={true}
        startInLoadingState={true}
        scalesPageToFit={true}
      />
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
  },
});
```

**Build:**
```bash
# Android
npx react-native run-android

# iOS
npx react-native run-ios

# Release builds
cd android && ./gradlew assembleRelease
cd ios && xcodebuild -scheme CarXSimulatorApp archive
```

#### Flutter Integration

**Setup:**
```bash
# Create Flutter project
flutter create carx_simulator_app
cd carx_simulator_app

# Add WebView dependency to pubspec.yaml
dependencies:
  webview_flutter: ^4.0.0
```

**main.dart:**
```dart
import 'package:flutter/material.dart';
import 'package:webview_flutter/webview_flutter.dart';

void main() {
  runApp(CarXSimulatorApp());
}

class CarXSimulatorApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'CarX Simulator',
      home: Scaffold(
        body: SafeArea(
          child: WebView(
            initialUrl: 'https://your-domain.com/preview/final-demo.html',
            javascriptMode: JavascriptMode.unrestricted,
          ),
        ),
      ),
    );
  }
}
```

**Build:**
```bash
# Android
flutter build apk --release

# iOS
flutter build ios --release

# Both
flutter build appbundle
```

---

### 4. WebAssembly Integration

#### Embed in Existing Website

**HTML:**
```html
<!DOCTYPE html>
<html>
<head>
    <title>My App with CarX Simulator</title>
</head>
<body>
    <h1>My Application</h1>
    
    <!-- Embed CarX Simulator -->
    <iframe 
        src="https://your-domain.com/preview/final-demo.html"
        width="100%" 
        height="600px"
        frameborder="0"
        allow="accelerometer; gyroscope"
    ></iframe>
</body>
</html>
```

#### React Integration

```jsx
import React from 'react';

function CarXSimulator() {
  return (
    <div className="simulator-container">
      <iframe
        src="/preview/final-demo.html"
        width="100%"
        height="600px"
        frameBorder="0"
        title="CarX Physics Simulator"
      />
    </div>
  );
}

export default CarXSimulator;
```

#### Vue Integration

```vue
<template>
  <div class="simulator-container">
    <iframe
      src="/preview/final-demo.html"
      width="100%"
      height="600px"
      frameborder="0"
      title="CarX Physics Simulator"
    />
  </div>
</template>

<script>
export default {
  name: 'CarXSimulator'
}
</script>
```

---

## 🔧 Configuration & Customization

### Environment Variables

Create `.env` file:
```bash
# API Endpoints
VITE_API_URL=https://api.your-domain.com

# Feature Flags
VITE_ENABLE_SHADOWS=true
VITE_DEFAULT_QUALITY=medium
VITE_AUTO_QUALITY=false

# Analytics
VITE_ANALYTICS_ID=UA-XXXXXXXXX-X
```

### Custom Branding

**Modify HTML:**
```html
<!-- Change title -->
<title>Your Brand - Physics Simulator</title>

<!-- Change colors in CSS -->
<style>
    h1 { color: #YOUR_COLOR; }
    button { background: linear-gradient(135deg, #COLOR1, #COLOR2); }
</style>
```

**Modify JavaScript:**
```javascript
// Change default settings
let zoomLevel = 25;  // Default camera distance
let cameraFollowCar = true;  // Auto-follow
let autoQualityEnabled = false;  // Manual quality control
```

---

## 📊 Performance Optimization

### Production Build Checklist

- [ ] Minify HTML/CSS/JavaScript
- [ ] Compress GLB model (use gltf-pipeline)
- [ ] Enable gzip/brotli compression
- [ ] Add CDN for Three.js library
- [ ] Enable browser caching
- [ ] Add service worker for offline support

### Optimization Commands

```bash
# Minify HTML
html-minifier --collapse-whitespace --remove-comments final-demo.html -o final-demo.min.html

# Compress GLB model
gltf-pipeline -i lowpoly_game_model.glb -o lowpoly_game_model_compressed.glb -d

# Generate service worker
workbox generateSW workbox-config.js
```

---

## 🔒 Security Considerations

### Content Security Policy

Add to HTML `<head>`:
```html
<meta http-equiv="Content-Security-Policy" content="
    default-src 'self';
    script-src 'self' 'unsafe-inline' https://cdn.jsdelivr.net;
    style-src 'self' 'unsafe-inline';
    img-src 'self' data:;
    connect-src 'self';
">
```

### HTTPS Requirements

- WebGL features require HTTPS
- Use Let's Encrypt for free SSL certificates
- Configure automatic renewal

---

## 📱 Platform-Specific Features

### iOS Considerations

- Add to `<head>` for full-screen:
```html
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<link rel="apple-touch-icon" href="icon.png">
```

### Android Considerations

- Add manifest.json:
```json
{
  "name": "CarX Physics Simulator",
  "short_name": "CarX",
  "start_url": "/preview/final-demo.html",
  "display": "standalone",
  "background_color": "#667eea",
  "theme_color": "#667eea",
  "icons": [{
    "src": "icon-192.png",
    "sizes": "192x192",
    "type": "image/png"
  }]
}
```

---

## 🧪 Testing

### Browser Testing

Test on:
- ✅ Chrome/Edge (Chromium)
- ✅ Firefox
- ✅ Safari (macOS/iOS)
- ✅ Mobile browsers

### Performance Testing

```bash
# Lighthouse audit
lighthouse https://your-domain.com/preview/final-demo.html --view

# WebPageTest
# Visit: https://www.webpagetest.org/
```

---

## 📦 Distribution Packages

### NPM Package (for developers)

**package.json:**
```json
{
  "name": "carx-physics-simulator",
  "version": "1.0.0",
  "description": "Interactive physics simulation engine",
  "main": "preview/final-demo.html",
  "files": [
    "preview/",
    "README.md"
  ],
  "keywords": ["physics", "simulation", "education"],
  "license": "MIT"
}
```

**Publish:**
```bash
npm publish
```

### Docker Container

**Dockerfile:**
```dockerfile
FROM nginx:alpine

COPY preview/ /usr/share/nginx/html/

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]
```

**Build & Run:**
```bash
docker build -t carx-simulator .
docker run -p 8080:80 carx-simulator
```

---

## 🎯 Quick Start Commands

### Web (Instant)
```bash
cd preview
python -m http.server 8080
# Open: http://localhost:8080/final-demo.html
```

### Desktop (C++)
```bash
mkdir build && cd build
cmake .. && cmake --build .
./CarXSimulator
```

### Mobile (React Native)
```bash
npx react-native init MyApp
# Add WebView component
npx react-native run-android
```

### Docker
```bash
docker build -t carx-simulator .
docker run -p 8080:80 carx-simulator
```

---

## 📞 Support & Resources

- **Documentation**: See README.md
- **Issues**: GitHub Issues
- **Examples**: See examples/ folder
- **API Reference**: See docs/API.md

---

**Ready to deploy!** Choose your platform and follow the steps above. 🚀
