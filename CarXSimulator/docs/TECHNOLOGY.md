# CarX Simulator - Technology Stack & Modification Guide

This document provides a comprehensive overview of all technologies used in CarX Simulator and how to modify each component.

## Table of Contents

1. [Technology Overview](#technology-overview)
2. [Web Preview (Three.js)](#web-preview-threejs)
3. [3D Assets & Blender Pipeline](#3d-assets--blender-pipeline)
4. [C++ Physics Engine](#c-physics-engine)
5. [Build Systems](#build-systems)
6. [Deployment](#deployment)
7. [Modification Guides](#modification-guides)

---

## Technology Overview

| Component | Technology | Version | Purpose |
|-----------|------------|---------|---------|
| Web Rendering | Three.js | r152+ | 3D graphics in browser |
| Web UI | HTML5/CSS3/JS | ES6+ | User interface |
| 3D Modeling | Blender | 2.80-4.x | Asset creation |
| Asset Format | glTF/GLB | 2.0 | 3D model interchange |
| Physics Core | C++20 | GCC 11+ | Simulation engine |
| Math Library | Custom | - | Vector/Matrix operations |
| Desktop Graphics | OpenGL | 4.5+ | Native rendering |
| Web Compilation | Emscripten | 3.0+ | WASM build |
| Build System | CMake | 3.20+ | Cross-platform builds |
| Android | Gradle | 8.4+ | APK packaging |
| Containerization | Docker | 20+ | Deployment |

---

## Web Preview (Three.js)

### Location
```
preview/
├── final-demo.html      # Main simulator (all-in-one)
├── assets/              # 3D models, textures, HDR
└── vendor/three/        # Three.js library
```

### Key Technologies

**Three.js** - 3D rendering library
- Version: r152+ (loaded via CDN or local)
- Features used: WebGL2, PBR materials, shadows, HDR environment


### How to Modify

#### Change Car Appearance
Edit `final-demo.html`, find `createCarModel()` function:

```javascript
// Around line 1840 - Car body color
const bodyMat = new THREE.MeshStandardMaterial({ 
    color: 0xff6b35,      // Change this hex color
    metalness: 0.7,       // 0-1, higher = more metallic
    roughness: 0.3        // 0-1, lower = more shiny
});
```

#### Change Physics Parameters
Edit the `Car` class constructor:

```javascript
// Around line 1050
this.mass = 1500;           // kg
this.enginePower = 150000;  // Watts
this.maxSpeed = 50;         // m/s
this.wheelRadius = 0.35;    // meters
```

#### Add New Terrain Type
Find `loadTerrainPreset()` function:

```javascript
case 'custom':
    terrain.generateTerrain(amplitude, frequency, octaves);
    break;
```

#### Change Camera Behavior
Edit camera settings around line 2400:

```javascript
const cameraDistance = 30;    // Default zoom
const cameraMinDist = 10;     // Min zoom
const cameraMaxDist = 80;     // Max zoom
```

---

## 3D Assets & Blender Pipeline

### Asset Locations
```
preview/assets/
├── lowpoly_game_model.glb   # Car model
├── tree.glb                  # Vegetation
├── farmland_overcast_1k.exr # HDR environment
└── models/                   # Custom exports
```

### Blender Tools
```
tools/
├── blender_scene_tools.py   # Import/export script
└── README.md                # Tool documentation
```

### Blender Workflow

#### Quick Start
```bash
# Generate CarX scene in Blender
blender --python tools/blender_scene_tools.py -- --generate

# Export to GLB
blender -b --python tools/blender_scene_tools.py -- --generate --export car.glb

# Import existing model
blender --python tools/blender_scene_tools.py -- --import model.glb
```

#### Interactive Mode
```bash
blender --python tools/blender_scene_tools.py
# Opens Blender with CarX panel in 3D View sidebar
```

#### Supported Blender Versions
- Blender 2.80 - 2.93 (LTS)
- Blender 3.0 - 3.6 (LTS)
- Blender 4.0 - 4.x (Current)

The script auto-detects version and adjusts API calls accordingly.

### Creating Custom Car Model

1. **Generate base in Blender:**
   ```bash
   blender --python tools/blender_scene_tools.py -- --generate-car
   ```

2. **Edit in Blender:**
   - Modify geometry, add details
   - Adjust materials (PBR: Base Color, Metallic, Roughness)
   - Keep wheel naming: `wheel_FL`, `wheel_FR`, `wheel_RL`, `wheel_RR`

3. **Export:**
   ```bash
   blender -b your_scene.blend --python tools/blender_scene_tools.py -- --export-car my_car.glb
   ```

4. **Use in CarX:**
   - Place GLB in `preview/assets/`
   - Update `final-demo.html` loader path

### Material Mapping (Blender → Three.js)

| Blender (Principled BSDF) | Three.js (MeshStandardMaterial) |
|---------------------------|--------------------------------|
| Base Color | color |
| Metallic | metalness |
| Roughness | roughness |
| Emission Color | emissive |
| Emission Strength | emissiveIntensity |
| Normal Map | normalMap |
| Alpha | opacity + transparent |

### Export Settings (Manual)

If exporting manually from Blender:
- Format: glTF 2.0 (.glb)
- Include: Meshes, Materials
- Transform: +Y Up (matches Three.js)
- Apply Modifiers: Yes
- Compression: Optional (Draco)

---

## C++ Physics Engine

### Source Structure
```
src/
├── core/
│   └── Engine.cpp           # Main loop
├── physics/
│   ├── Car.cpp              # Vehicle physics
│   ├── PhysicsEngine.cpp    # Simulation coordinator
│   ├── Terrain.cpp          # Terrain collision
│   ├── TireModel.cpp        # Pacejka tire model
│   └── VehicleDynamics.cpp  # Advanced dynamics
├── renderer/
│   ├── OpenGLBackend.cpp    # OpenGL rendering
│   ├── Shader.cpp           # GPU shaders
│   └── Mesh.cpp             # Geometry
└── utils/
    └── Math.cpp             # Vector/Matrix math
```

### Headers
```
include/
├── Car.h                # Vehicle entity
├── PhysicsEngine.h      # Physics coordinator
├── VehicleDynamics.h    # Advanced dynamics
├── TireModel.h          # Pacejka formula
├── Terrain.h            # Terrain system
├── Renderer.h           # Graphics abstraction
├── Math.h               # Vector3, Matrix4, Quaternion
└── Engine.h             # Application lifecycle
```

### How to Modify Physics

#### Change Tire Model
Edit `src/physics/TireModel.cpp`:

```cpp
// Pacejka Magic Formula coefficients
float TireModel::calculateLateralForce(float slipAngle, float load) {
    // B, C, D, E coefficients
    float B = 10.0f;   // Stiffness factor
    float C = 1.9f;    // Shape factor
    float D = load;    // Peak factor
    float E = 0.97f;   // Curvature factor
    
    return D * sin(C * atan(B * slipAngle - E * (B * slipAngle - atan(B * slipAngle))));
}
```

#### Add New Vehicle Property
1. Add to `include/Car.h`:
   ```cpp
   float turboBoost = 1.0f;
   ```

2. Use in `src/physics/Car.cpp`:
   ```cpp
   float effectivePower = enginePower * turboBoost;
   ```

---

## Build Systems

### CMake (C++ Engine)

```bash
# Standard build
mkdir build && cd build
cmake ..
cmake --build .

# With options
cmake -DUSE_VULKAN=ON -DBUILD_TESTS=ON ..
```

Key CMake options:
- `USE_VULKAN` - Enable Vulkan backend
- `BUILD_WASM` - Build for WebAssembly
- `BUILD_TESTS` - Include test suite

### Gradle (Android)

```bash
cd android

# Debug build
./gradlew assembleDebug

# Release build (requires signing)
./gradlew assembleRelease
```

APK output: `android/app/build/outputs/apk/`

### npm Scripts

```bash
npm run build:apk      # Build Android APK
npm run deploy:vercel  # Deploy to Vercel
npm run start          # Start local server
```

---

## Deployment

### Web Deployment

**Vercel/Netlify:**
```bash
# From repo root
vercel --prod
# or
netlify deploy --prod --dir=preview
```

**Docker:**
```bash
docker build -t carx-simulator .
docker run -p 8080:80 carx-simulator
```

**GitHub Pages:**
- Push to `gh-pages` branch
- Or use GitHub Actions workflow

### Local Development

```bash
cd preview
python -m http.server 8080
# Open http://localhost:8080/final-demo.html
```

Or use the provided scripts:
```bash
# Windows
preview/START_PREVIEW.bat

# Linux/macOS
preview/START_PREVIEW.sh
```


---

## Modification Guides

### Adding a New Car Part (Web)

1. **Define geometry in `createCarModel()`:**
   ```javascript
   // Add after spoiler creation
   const wingGeom = new THREE.BoxGeometry(2.0, 0.05, 0.3);
   const wing = new THREE.Mesh(wingGeom, bodyMat);
   wing.position.set(0, 1.5, -1.9);
   carGroup.add(wing);
   ```

2. **For animated parts, store reference:**
   ```javascript
   carParts.wing = wing;  // Access later for animation
   ```

### Adding New Terrain Feature

1. **In terrain generation:**
   ```javascript
   // Add ramp
   terrain.addRamp(x, z, width, length, height, angle);
   
   // Add obstacle
   terrain.addObstacle(x, z, radius, height);
   ```

2. **Custom height function:**
   ```javascript
   terrain.setHeightFunction((x, z) => {
       return Math.sin(x * 0.1) * 5 + Math.cos(z * 0.1) * 3;
   });
   ```

### Creating Custom Blender Export

1. **Modify `tools/blender_scene_tools.py`:**
   ```python
   def create_custom_part():
       # Add your geometry creation here
       bpy.ops.mesh.primitive_cube_add(size=1)
       obj = bpy.context.active_object
       obj.name = "CustomPart"
       return obj
   ```

2. **Add to scene generation:**
   ```python
   def generate_scene(...):
       # ... existing code ...
       custom = create_custom_part()
       custom.parent = car_parent
   ```

### Integrating External 3D Models

1. **Prepare model in Blender:**
   - Import your model (FBX, OBJ, etc.)
   - Apply transforms (Ctrl+A → All Transforms)
   - Check scale (1 unit = 1 meter)
   - Ensure Y-up orientation

2. **Export using CarX tools:**
   ```bash
   blender your_model.blend --python tools/blender_scene_tools.py -- --export model.glb
   ```

3. **Load in Three.js:**
   ```javascript
   loader.load('assets/model.glb', (gltf) => {
       scene.add(gltf.scene);
   });
   ```

### Adding New Physics Feature (C++)

1. **Define in header:**
   ```cpp
   // include/Car.h
   class Car {
       // ...
       float nitroBoost = 0.0f;
       void activateNitro(float duration);
   };
   ```

2. **Implement:**
   ```cpp
   // src/physics/Car.cpp
   void Car::activateNitro(float duration) {
       nitroBoost = 2.0f;  // Double power
       nitroTimer = duration;
   }
   
   void Car::update(float dt) {
       if (nitroTimer > 0) {
           nitroTimer -= dt;
           if (nitroTimer <= 0) nitroBoost = 0.0f;
       }
       // Apply boost to engine power
       float power = enginePower * (1.0f + nitroBoost);
   }
   ```

### Customizing Mobile Controls

Edit the mobile control panel in `final-demo.html`:

```html
<!-- Around line 520 -->
<div class="mobile-control-panel">
    <!-- Add new button -->
    <button id="btn-nitro" class="touch-btn action-btn nitro">N</button>
</div>
```

```javascript
// Add handler
document.getElementById('btn-nitro').addEventListener('touchstart', () => {
    car.activateNitro(3.0);  // 3 second boost
});
```

---

## File Reference

### Key Files to Modify

| Goal | File(s) |
|------|---------|
| Car appearance | `preview/final-demo.html` (createCarModel) |
| Physics behavior | `preview/final-demo.html` (Car class) |
| UI/Controls | `preview/final-demo.html` (CSS + HTML) |
| 3D models | `tools/blender_scene_tools.py` |
| C++ physics | `src/physics/*.cpp` |
| Build config | `CMakeLists.txt`, `android/build.gradle` |
| Deployment | `Dockerfile`, `vercel.json`, `nginx.conf` |

### Configuration Files

| File | Purpose |
|------|---------|
| `package.json` | npm scripts, metadata |
| `CMakeLists.txt` | C++ build configuration |
| `android/build.gradle` | Android build settings |
| `Dockerfile` | Container build |
| `vercel.json` | Vercel deployment |
| `nginx.conf` | Web server config |

---

## Troubleshooting

### Blender Export Issues

**"glTF exporter not available"**
- Enable in Blender: Edit → Preferences → Add-ons → Search "glTF"

**Model appears wrong scale**
- Apply transforms in Blender: Ctrl+A → All Transforms
- Check export scale settings

**Materials not showing**
- Use Principled BSDF shader
- Ensure textures are packed or paths are correct

### Three.js Loading Issues

**Model not loading**
- Check browser console for errors
- Verify file path is correct
- Ensure CORS headers if loading from different domain

**Performance issues**
- Reduce polygon count in Blender
- Use lower resolution textures
- Enable LOD system

### Build Issues

**CMake errors**
- Ensure CMake 3.20+ installed
- Check compiler supports C++20

**Android build fails**
- Verify JDK 17+ installed
- Check Android SDK path in local.properties

---

## Resources

- [Three.js Documentation](https://threejs.org/docs/)
- [Blender Manual](https://docs.blender.org/manual/)
- [glTF Specification](https://www.khronos.org/gltf/)
- [CMake Documentation](https://cmake.org/documentation/)
- [Pacejka Tire Model](https://en.wikipedia.org/wiki/Hans_B._Pacejka)

---

*Document Version: 2.0 | Last Updated: November 2024*
