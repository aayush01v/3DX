# Building CarXSimulator with AIDE+ Pro

## Overview

This guide explains how to build CarXSimulator APK using AIDE+ Pro on your Android device.

## Prerequisites

1. **AIDE+ Pro** installed from Play Store
2. **NDK Support** addon for AIDE (for C++ compilation)
3. At least 2GB free storage
4. Android 7.0+ device

## Project Structure

```
android/
├── app/
│   ├── src/main/
│   │   ├── java/com/carx/simulator/
│   │   │   ├── MainActivity.java      # Main activity (WebView)
│   │   │   └── NativeLib.java         # JNI wrapper
│   │   ├── jni/
│   │   │   ├── main.cpp               # C++ physics engine
│   │   │   ├── Android.mk             # NDK build config
│   │   │   └── Application.mk         # NDK app config
│   │   ├── res/                        # Resources
│   │   └── AndroidManifest.xml
│   └── build.gradle
├── build.gradle
└── settings.gradle
```

## Build Steps

### Step 1: Open Project in AIDE

1. Open AIDE+ Pro
2. Tap "Open Project"
3. Navigate to `CarXSimulator/android`
4. Select the `android` folder

### Step 2: Configure NDK (First Time Only)

1. Go to AIDE Settings → Build & Run
2. Enable "NDK Support"
3. Set NDK path if prompted

### Step 3: Build APK

1. Tap the Play button (▶) or Menu → Build → Build APK
2. Wait for compilation (may take 2-5 minutes)
3. APK will be in `app/build/outputs/apk/`

### Step 4: Install

1. AIDE will prompt to install after build
2. Or manually install the APK

## Native C++ Code

The physics engine is in `jni/main.cpp` and includes:

### Classes
- `Vec3` - 3D vector math
- `Terrain` - Procedural terrain with Perlin noise
- `Car` - Vehicle physics simulation

### JNI Functions
```cpp
// Initialize engine
Java_com_carx_simulator_NativeLib_init()

// Update physics (call every frame)
Java_com_carx_simulator_NativeLib_update(float deltaTime)

// Controls
Java_com_carx_simulator_NativeLib_setThrottle(float value)
Java_com_carx_simulator_NativeLib_setSteering(float value)
Java_com_carx_simulator_NativeLib_setBrake(float value)
Java_com_carx_simulator_NativeLib_shiftUp()
Java_com_carx_simulator_NativeLib_shiftDown()

// Get car state (position, speed, rpm, gear, etc.)
Java_com_carx_simulator_NativeLib_getCarState() -> float[10]

// Terrain
Java_com_carx_simulator_NativeLib_setTerrain(int type)
Java_com_carx_simulator_NativeLib_getTerrainHeight(float x, float z)

// Cleanup
Java_com_carx_simulator_NativeLib_cleanup()
```

## Using Native Physics in Java

```java
// Initialize
NativeLib physics = NativeLib.getInstance();
physics.init();

// Game loop
void onFrame(float dt) {
    // Set inputs
    physics.setThrottle(throttleValue);
    physics.setSteering(steeringValue);
    physics.setBrake(brakeValue);
    
    // Update physics
    physics.update(dt);
    
    // Get state
    NativeLib.CarState state = physics.getState();
    
    // Use state
    float speed = state.getSpeedKmh();
    String gear = state.getGearName();
    float rpm = state.rpm;
}

// Cleanup
physics.cleanup();
```

## Troubleshooting

### "NDK not found"
- Install NDK addon from AIDE settings
- Restart AIDE after installation

### "Cannot find -llog"
- This is an Android system library, should work automatically
- Try rebuilding the project

### Build takes too long
- First build compiles all ABIs (arm, arm64, x86, x86_64)
- Subsequent builds are faster
- You can edit Application.mk to build only your device's ABI

### App crashes on start
- Check logcat for errors
- Ensure native library loaded: `System.loadLibrary("carx-native")`
- Verify JNI function signatures match

## Modifying the Code

### Adding New Physics Features

Edit `jni/main.cpp`:

```cpp
// Add to Car class
float turboBoost = 1.0f;

void activateTurbo() {
    turboBoost = 1.5f;
}

// Add JNI function
extern "C" JNIEXPORT void JNICALL
Java_com_carx_simulator_NativeLib_activateTurbo(JNIEnv* env, jobject thiz) {
    if (g_car) g_car->activateTurbo();
}
```

Add to `NativeLib.java`:
```java
public native void activateTurbo();
```

### Changing Terrain

Modify `Terrain::getHeight()` in main.cpp to create custom terrain.

## Performance Tips

1. Use `arm64-v8a` only for modern devices (faster build)
2. Enable ProGuard for release builds
3. Use fixed timestep for physics (1/60 second)
4. Batch JNI calls when possible

## Version Info

- CarX Simulator: 2.3.1
- Min Android: 7.0 (API 24)
- NDK: r21+ recommended
- C++ Standard: C++17
