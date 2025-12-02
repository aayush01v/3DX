# 🏎️ CarX Physics Simulator

A high-performance C++ physics engine for realistic car simulation with advanced kinematics, terrain interaction, and real-time 3D visualization.

## ✨ Features

### 🚗 Professional Car Physics (v2.2 - Industry Standard! 🏎️)
- **Pacejka Tire Model** - Industry-standard Magic Formula (from svenlr/car-physics-pacejka)
- **Load Transfer** - Realistic weight distribution during acceleration/braking/cornering
- **Aerodynamic Downforce** - Speed-dependent grip enhancement
- **Tire Temperature** - Heat generation and cooling simulation
- **Tire Wear** - Long-term degradation modeling
- **Advanced Suspension** - Multi-body dynamics with anti-roll bars (from Project Chrono)
- **Perfect Ground Contact** - No more falling through terrain or floating
- **Smooth Acceleration** - Realistic torque curve with proper gear ratios
- **Effective Braking** - Speed-dependent brake force with ABS
- **Natural Steering** - Speed-dependent response, no over-sensitivity
- **Drivetrain Options** - FWD, RWD, AWD with torque distribution
- **Driver Assistance** - Traction control and anti-lock braking systems (ABS)
- **Differential Systems** - Open, limited-slip, and locked differential options
- **Professional Grade** - Based on TUMFTM/Open-Car-Dynamics principles

### 🏔️ Enhanced Terrain System
- **Advanced Generation** - Multi-octave Perlin noise for natural landscapes
- **Multiple Presets** - Flat, hills, mountains, valleys, and racetracks
- **Dynamic Features** - Add obstacles, ramps, and deformations at runtime
- **Material Zones** - Define areas with different friction (ice, mud, asphalt)
- **Terrain Tools** - Smoothing, erosion simulation, and normal caching
- **Performance Optimized** - Cached normals and collision detection

### 💻 Enhanced Coding Support
- **Structured API** - 20+ commands with autocomplete and help system
- **Variable System** - Define and use variables in scripts
- **Batch Execution** - Run multi-line scripts with comments
- **Command Help** - Built-in documentation for all API functions
- **Output Feedback** - Clear success/error messages for all operations

### Visualization
- **3D Graphics** - Real-time rendering with Three.js and WebGL
- **Interactive Camera** - Touch/mouse controls with orbit and free modes
- **Force Vectors** - Visual representation of physics forces
- **Debug Mode** - Ground contact visualization (press G)

### Controls
- **Keyboard** - Arrow keys or WASD for driving
- **Mouse** - Drag to rotate camera, wheel to zoom
- **Touch** - Single finger drag, pinch zoom, double-tap reset
- **Code Editor** - Live physics experiments with C++/Python/JavaScript

## 🚀 Quick Start & Deployment

### Instant Web Preview (No Build Required!)

**Windows:**
```bash
cd preview
START_PREVIEW.bat
```

**Linux/macOS:**
```bash
cd preview
./START_PREVIEW.sh
```

**Or manually:**
```bash
cd preview
python -m http.server 8080
# Open: http://localhost:8080/final-demo.html
```

### C++ Build

**Prerequisites:**
- CMake 3.20+
- C++20 compiler (GCC 11+, Clang 14+, MSVC 2022+)
- OpenGL 4.5+ or Vulkan 1.2+

**Build:**
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## 🎮 Controls Reference

### Driving
- `↑` or `W` - Throttle
- `↓` or `S` - Brake
- `←` `→` or `A` `D` - Steer
- `Space` - Handbrake

### Camera (Touch/Mouse Interactive)
- `Drag` - Rotate camera around car (orbit mode)
- `Mouse Wheel` / `Pinch` - Zoom in/out (10-80m range)
- `C` - Toggle follow mode (auto-track car)
- `R` - Reset camera to default view
- `G` - Toggle ground contact debug sphere
- `Double-tap` (mobile) - Reset camera view

### UI Buttons
- **Reset Car** - Return to start position
- **Pause Physics** - Freeze simulation
- **Terrain Types** - Flat, Hills, Mountains, Track
- **Quality Settings** - Low, Medium, High

## 📖 Documentation

### Getting Started
- **[API Reference](docs/API.md)** - Quick start & API documentation
- **[Building Guide](docs/BUILDING.md)** - Compilation instructions
- **[Technology Stack](docs/TECHNOLOGY.md)** - Tech overview & modification guide

### Core Documentation
- **[Project Architecture](docs/PROJECT_ARCHITECTURE.md)** - System design & integration
- **[Advanced Vehicle Dynamics](docs/ADVANCED_VEHICLE_DYNAMICS.md)** - Professional physics (Pacejka, load transfer)
- **[Unified Scene System](docs/UNIFIED_SCENE_SYSTEM.md)** - Cross-platform scene definition

### 3D Pipeline & Blender Tools
- **[Model Integration](docs/MODEL_INTEGRATION.md)** - 3D models & Blender workflow
- **[Blender Tools](tools/README.md)** - Import/export pipeline for Blender
- **[Pipeline Guide](tools/PIPELINE_GUIDE.md)** - Blender ↔ Three.js transforms
- **[Quick Reference](tools/QUICK_REFERENCE.md)** - Blender tools cheat sheet

### Physics & Experiments
- **[Physics Experiments](docs/PHYSICS_COMPONENTS_AND_EXPERIMENTS.md)** - Interactive code experiments

### Examples
- **[Basic Car](examples/basic_car.cpp)** - Simple car simulation
- **[Advanced Physics](examples/advanced_physics.cpp)** - Complex scenarios
- **[Curved Terrain](examples/curved_terrain.cpp)** - Terrain interaction

📚 **[Full Documentation Index](docs/README.md)**

## 🔧 Troubleshooting

### Car Floating or Sinking?
- Press `G` to see ground contact debug sphere
- Check `groundClearance` value in code (default: 0.5)
- See [Ground Fix Guide](preview/GROUND_FIX.md)

### Camera Controls Inverted?
- Fixed in latest version
- Drag left = rotate left, drag right = rotate right

### Low FPS?
- Reduce quality to "Low" in settings
- Close other browser tabs
- Check FPS counter in UI

### Model Not Loading?
- Ensure `lowpoly_game_model.glb` is in preview folder
- Check browser console for errors
- Fallback procedural car will load automatically

## 🎯 Physics Experiments & Code Editor

### Full Programming Support
The code editor supports variables, functions, and complex calculations:

```cpp
// Define variables
float mass = 1500;
float velocity = 20;

// Calculate kinetic energy
float KE = 0.5 * mass * velocity * velocity;

// Apply to simulation
car.setMass(mass);
car.setVelocity(Vector3(0, 0, velocity));
car.getEnergy();
```

### Available Commands (20+ Total)

**Car Control:**
- `createCar x y z [mass] [power]` - Create new car
- `setThrottle amount` - Set throttle (-1 to 1)
- `setSteering angle` - Set steering (-1 to 1)
- `setBrake amount` - Set brake (0 to 1)
- `setTractionControl 0/1` - Enable/disable TC
- `setABS 0/1` - Enable/disable ABS

**Terrain:**
- `loadTerrainPreset type` - Load preset (flat, hills, mountains, valley, racetrack)
- `generateTerrain amplitude frequency` - Custom procedural terrain
- `addObstacle x z radius height` - Add obstacle
- `addRamp x z width length height` - Add jump ramp
- `smoothTerrain [iterations]` - Smooth terrain

**Car Properties:**
- `setCarMass kg` - Set car mass
- `setTireGrip coefficient` - Set tire grip (0.5-2.0)

**Physics:**
- `setGravity x y z` - Set gravity vector
- `reset` - Reset simulation

See [IMPROVEMENTS.md](docs/IMPROVEMENTS.md) for complete API reference.

### Built-in Experiments (18 total)
- **Kinematics**: Equations of motion, projectile motion, relative velocity
- **Forces**: Newton's laws, friction, circular motion, banking
- **Energy**: Work-energy theorem, power, springs
- **Momentum**: Collisions, impulse, conservation
- **Gravitation**: Universal gravitation, escape velocity
- **Rotation**: Torque, angular momentum
- **Oscillations**: Simple harmonic motion
- **Fluids & Waves**: Pressure, buoyancy, Doppler effect

## 🌟 Key Features Explained

### Ground Contact System
- Physics position = center of mass
- Mesh position = wheels on ground
- Smooth terrain following with interpolation
- Bounce damping on landing

### Camera System
- Spherical coordinates for orbit
- Smooth lerp for movement (10% per frame)
- Follow mode: tracks car automatically
- Free mode: manual camera positioning

### Performance Optimizations
- **Adaptive Quality System** - Auto-adjusts based on FPS
- **ACES Tone Mapping** - Realistic HDR lighting
- **PCF Soft Shadows** - Smooth shadow edges (1024-2048px)
- **Environment Mapping** - Realistic reflections
- **Fixed Timestep Physics** - Consistent 60 FPS simulation
- **Smart Rendering** - Optimized draw calls and culling
- **Visibility Pausing** - Stops when tab hidden

## 📁 Project Structure

```
CarXSimulator/
├── preview/              # Web demo (no build required)
│   ├── final-demo.html  # Main demo file
│   ├── lowpoly_game_model.glb
│   └── *.md             # Documentation
├── src/                 # C++ source code
│   ├── core/           # Engine core
│   ├── physics/        # Physics simulation
│   ├── renderer/       # Graphics rendering
│   └── scripting/      # Code execution
├── include/            # C++ headers
├── examples/           # Example programs
├── docs/              # API documentation
└── CMakeLists.txt     # Build configuration
```

## 🎓 Learning Objectives

This simulator teaches:
- **Kinematics** - Position, velocity, acceleration
- **Forces** - Engine, drag, gravity, centripetal
- **Circular Motion** - Why cars need force to turn
- **Friction** - How tire grip affects handling
- **Energy** - Mass and momentum relationships
- **Terrain Physics** - Ground contact and slopes

## 🤝 Contributing

Contributions welcome! Areas for improvement:
- Suspension system
- Multi-car support
- Replay system
- More terrain types
- VR support

## 📝 License

MIT License - Free to use and modify

---

**Made with ❤️ for physics education and C++ learning**


## 🌐 Cross-Platform Deployment

### Quick Deploy Options

**Web (Instant):**
```bash
# GitHub Pages, Netlify, Vercel
cd preview && netlify deploy --prod

# Or deploy the prebuilt preview to Vercel from the repo root
npm run deploy:vercel
```

**Docker:**
```bash
docker build -t carx-simulator .
docker run -p 8080:80 carx-simulator
```

**Android APK (Native):**
```bash
# Build debug APK
npm run build:apk

# Or manually
cd android
./gradlew assembleDebug

# APK: android/app/build/outputs/apk/debug/app-debug.apk
```

**Desktop (Native C++):**
```bash
mkdir build && cd build
cmake .. && cmake --build .
```

📖 **Full deployment guide**: See [DEPLOYMENT.md](DEPLOYMENT.md) for detailed instructions on:
- Web hosting (GitHub Pages, Netlify, Vercel, self-hosted)
- Desktop builds (Windows, macOS, Linux)
- Mobile integration (React Native, Flutter)
- WebAssembly embedding
- Docker containers
- CI/CD pipelines

## 📦 Distribution

### NPM Package
```bash
npm install carx-physics-simulator
```

### Docker Hub
```bash
docker pull carx-simulator:latest
```

### GitHub Releases
Download pre-built binaries for Windows, macOS, and Linux from [Releases](https://github.com/yourusername/CarXSimulator/releases)

## 🎯 Project Status

- ✅ **v2.3.1** - Android APK support, terrain fixes
- ✅ **v2.3.0** - Professional Physics with Pacejka tires
- ✅ Web preview fully functional with all features
- ✅ Physics engine complete with industry-standard models
- ✅ Interactive camera system
- ✅ Code editor with 18 experiments
- ✅ Cross-platform ready (Web, Android, Desktop)
- ✅ Docker support
- ✅ CI/CD pipeline
- ✅ Android APK build system
- 🚧 iOS app (in progress)

---

## 📊 Quick Reference

### Terrain Types
- **Flat** - Testing and drag racing
- **Hills** - Rally and scenic driving  
- **Mountains** - Extreme off-road
- **Valley** - Downhill racing
- **Racetrack** - Circuit racing with banking

### Vehicle Configurations

**Racing Car**: 1000kg, 200kW, racing tires (1.5 grip), stiff suspension
**Rally Car**: 1150kg, 180kW, all-terrain tires (1.1 grip), soft suspension  
**Truck**: 2500kg, 180kW, off-road tires (0.9 grip), medium suspension

### Friction Values
- Ice: 0.3-0.4 | Wet: 0.5-0.6 | Dry: 0.8-1.0 | Asphalt: 1.0-1.2 | Racing: 1.3-1.5

---

**Ready to deploy across all platforms!** 🚀
