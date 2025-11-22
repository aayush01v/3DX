# Building CarX Simulator

## Prerequisites

### All Platforms
- CMake 3.20 or higher
- C++20 compatible compiler:
  - GCC 11+
  - Clang 14+
  - MSVC 2022+

### Desktop Build
- OpenGL 4.5+ or Vulkan 1.2+
- GLFW 3.3+

### WebAssembly Build
- Emscripten SDK 3.1+

### Mobile Build
- Android NDK (for Android)
- Xcode (for iOS)

## Building for Desktop

### Windows (MSVC)
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

### Linux
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### macOS
```bash
mkdir build
cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

## Building for WebAssembly

### Install Emscripten
```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

### Build
```bash
mkdir build-wasm
cd build-wasm
emcmake cmake -DBUILD_WASM=ON ..
emmake make
```

### Run
```bash
cd ../web
python -m http.server 8080
# Open http://localhost:8080
```

## Building Shared Library for Mobile

### Android
```bash
mkdir build-android
cd build-android
cmake -DBUILD_SHARED=ON \
      -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-24 \
      ..
make
```

### iOS
```bash
mkdir build-ios
cd build-ios
cmake -DBUILD_SHARED=ON \
      -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake \
      -DPLATFORM=OS64 \
      ..
make
```

## Using Vulkan Instead of OpenGL

```bash
cmake -DUSE_VULKAN=ON ..
```

## Build Options

- `BUILD_WASM` - Build for WebAssembly (default: OFF)
- `USE_VULKAN` - Use Vulkan instead of OpenGL (default: OFF)
- `BUILD_SHARED` - Build shared library for mobile (default: OFF)

## Running Examples

```bash
# After building
./build/CarXSimulator

# Or run specific example
./build/examples/basic_car
./build/examples/curved_terrain
```

## Troubleshooting

### Missing GLFW
```bash
# Ubuntu/Debian
sudo apt-get install libglfw3-dev

# macOS
brew install glfw

# Windows
vcpkg install glfw3
```

### Missing OpenGL
```bash
# Ubuntu/Debian
sudo apt-get install libgl1-mesa-dev

# Fedora
sudo dnf install mesa-libGL-devel
```

### Emscripten Issues
Make sure to source the environment:
```bash
source /path/to/emsdk/emsdk_env.sh
```
