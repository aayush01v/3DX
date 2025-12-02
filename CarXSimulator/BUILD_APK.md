# Building CarXSimulator APK

## Prerequisites

1. **Java JDK 17+**
   ```bash
   # Ubuntu/Debian
   sudo apt install openjdk-17-jdk
   
   # macOS
   brew install openjdk@17
   
   # Windows - Download from https://adoptium.net/
   ```

2. **Android SDK** (or Android Studio)
   ```bash
   # Set environment variables
   export ANDROID_HOME=$HOME/Android/Sdk
   export PATH=$PATH:$ANDROID_HOME/cmdline-tools/latest/bin
   export PATH=$PATH:$ANDROID_HOME/platform-tools
   ```

## Build Steps

### Option 1: Command Line (Recommended)

```bash
# Navigate to android directory
cd android

# Make gradlew executable
chmod +x gradlew

# Build debug APK
./gradlew assembleDebug

# Build release APK (requires signing)
./gradlew assembleRelease

# APK location
# Debug: android/app/build/outputs/apk/debug/app-debug.apk
# Release: android/app/build/outputs/apk/release/app-release.apk
```

### Option 2: Android Studio

1. Open Android Studio
2. File → Open → Select `android` folder
3. Wait for Gradle sync
4. Build → Build Bundle(s) / APK(s) → Build APK(s)

## Signing Release APK

### Create Keystore (one-time)
```bash
keytool -genkey -v -keystore carx-release.keystore \
  -alias carx -keyalg RSA -keysize 2048 -validity 10000
```

### Configure Signing
Add to `android/app/build.gradle`:
```gradle
android {
    signingConfigs {
        release {
            storeFile file('carx-release.keystore')
            storePassword 'your-store-password'
            keyAlias 'carx'
            keyPassword 'your-key-password'
        }
    }
    buildTypes {
        release {
            signingConfig signingConfigs.release
        }
    }
}
```

## Install on Device

```bash
# Via ADB
adb install -r app/build/outputs/apk/debug/app-debug.apk

# Or drag APK to device
```

## Troubleshooting

### WebGL Not Working
- Ensure device supports OpenGL ES 3.0+
- Check WebView is updated (Google Play → Android System WebView)

### Black Screen
- Enable USB debugging
- Check logcat: `adb logcat | grep -i webview`

### Performance Issues
- Use release build (not debug)
- Close other apps
- Test on device with Snapdragon 660+ or equivalent

## APK Size Optimization

Current size: ~15-20 MB (includes 3D models and textures)

To reduce:
1. Compress GLB models with Draco
2. Convert EXR to compressed HDR
3. Enable ProGuard minification (already configured)

## Minimum Requirements

- Android 7.0 (API 24) or higher
- OpenGL ES 3.0 support
- 2GB RAM recommended
- ~50MB storage
