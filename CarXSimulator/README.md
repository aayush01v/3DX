# 🏎️ CarX Physics Simulator

A high-performance C++ physics engine for realistic car simulation with advanced kinematics, terrain interaction, and real-time 3D visualization.

## ✨ Features

### Physics Engine
- **Realistic Car Dynamics** - Advanced force calculations, friction, and momentum
- **Terrain Following** - Smooth ground contact with proper collision detection
- **Gravity Simulation** - Adjustable gravity (Earth, Moon, Mars, Jupiter)
- **Free Fall Physics** - Realistic airborne behavior and landing

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

### Essential Guides
- **[Preview README](preview/README.md)** - How to run the web demo

### C++ Development
- **[API Documentation](docs/API.md)** - C++ API reference
- **[Building Guide](docs/BUILDING.md)** - Compilation instructions
- **[Model Integration](docs/MODEL_INTEGRATION.md)** - 3D model loading

### Examples
- **[Basic Car](examples/basic_car.cpp)** - Simple car simulation
- **[Advanced Physics](examples/advanced_physics.cpp)** - Complex scenarios
- **[Curved Terrain](examples/curved_terrain.cpp)** - Terrain interaction

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

### Available Commands
- `setMass(kg)`, `setEnginePower(W)`, `setGravity(m/s²)`
- `applyForce(x,y,z)`, `setVelocity(x,y,z)`, `setPosition(x,y,z)`
- `getSpeed()`, `getPosition()`, `getVelocity()`, `getMass()`
- `getEnergy()`, `getMomentum()`, `reset()`

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
```

**Docker:**
```bash
docker build -t carx-simulator .
docker run -p 8080:80 carx-simulator
```

**Mobile (React Native):**
```bash
npx react-native init MyApp
# Add WebView with simulator URL
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

- ✅ Web preview fully functional
- ✅ Physics engine complete
- ✅ Interactive camera system
- ✅ Code editor with 18 experiments
- ✅ Cross-platform ready
- ✅ Docker support
- ✅ CI/CD pipeline
- 🚧 Native desktop builds (in progress)
- 🚧 Mobile apps (in progress)

---

**Ready to deploy across all platforms!** 🚀
