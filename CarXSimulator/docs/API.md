# CarX Simulator - API Reference

Complete API documentation for the CarX Physics Simulator.

## Quick Start

### Basic Setup (C++)
```cpp
#include "PhysicsEngine.h"
#include "CodeExecutor.h"

PhysicsEngine physics;
CodeExecutor executor(&physics);
executor.registerAPI();

// Create terrain and car
executor.execute("loadTerrainPreset hills");
executor.execute("createCar 0 5 0 1200 150000");

// Game loop
while (running) {
    physics.update(1.0f / 60.0f);
    render();
}
```

### Web Preview
```bash
cd preview
python -m http.server 8080
# Open http://localhost:8080/final-demo.html
```

---

## Core Classes

### Car

```cpp
Car(const Vector3& position);
```

#### Control Methods
| Method | Parameters | Description |
|--------|------------|-------------|
| `applyThrottle(float)` | -1 to 1 | Throttle/reverse |
| `applySteering(float)` | -1 to 1 | Steering angle |
| `applyBrake(float)` | 0 to 1 | Brake force |

#### Properties
| Method | Parameters | Description |
|--------|------------|-------------|
| `setMass(float)` | kg | Vehicle mass |
| `setEnginePower(float)` | Watts | Engine power |
| `setWheelbase(float)` | meters | Axle distance |
| `setTireGrip(float)` | 0.5-2.0 | Grip coefficient |
| `setDragCoefficient(float)` | 0.2-0.5 | Aero drag |
| `setTractionControl(bool)` | true/false | TC system |
| `setAntiLockBrakes(bool)` | true/false | ABS system |
| `setDifferentialType(int)` | 0/1/2 | Open/LSD/Locked |

#### Getters
| Method | Returns | Description |
|--------|---------|-------------|
| `getPosition()` | Vector3 | World position |
| `getVelocity()` | Vector3 | Velocity vector |
| `getSpeed()` | float | Speed (m/s) |
| `getRotation()` | float | Yaw angle (rad) |
| `getRPM()` | float | Engine RPM |
| `getGear()` | int | Current gear |


### Terrain

```cpp
Terrain(int width = 100, int depth = 100);
```

| Method | Parameters | Description |
|--------|------------|-------------|
| `generateProcedural(float, float)` | amplitude, frequency | Generate terrain |
| `generateByType(TerrainType)` | enum | Use preset type |
| `getHeight(float, float)` | x, z | Get height at point |
| `getNormal(float, float)` | x, z | Get surface normal |
| `addObstacle(x, z, r, h)` | position, size | Add obstacle |
| `addRamp(x, z, w, l, h)` | position, dimensions | Add ramp |
| `setMaterialZone(x, z, r, f)` | position, friction | Set friction zone |
| `smoothTerrain(int)` | iterations | Smooth surface |

### PhysicsEngine

```cpp
PhysicsEngine();
```

| Method | Parameters | Description |
|--------|------------|-------------|
| `update(float)` | deltaTime | Step simulation |
| `createCar(Vector3)` | position | Create car |
| `setTerrain(unique_ptr)` | terrain | Set terrain |
| `setGravity(Vector3)` | gravity | Set gravity |

---

## Script Commands

### Terrain Commands
```
loadTerrainPreset <type>     # flat, hills, mountains, valley, racetrack
generateTerrain <amp> <freq> # Custom procedural
addObstacle <x> <z> <r> <h>  # Add obstacle
addRamp <x> <z> <w> <l> <h>  # Add ramp
smoothTerrain [iterations]   # Smooth terrain
```

### Car Commands
```
createCar <x> <y> <z> [mass] [power]  # Create car
setThrottle <-1 to 1>                 # Throttle
setSteering <-1 to 1>                 # Steering
setBrake <0 to 1>                     # Brake
setCarMass <kg>                       # Set mass
setTireGrip <0.5 to 2.0>              # Set grip
setTractionControl <0/1>              # TC on/off
setABS <0/1>                          # ABS on/off
```

### Physics Commands
```
setGravity <x> <y> <z>   # Set gravity vector
reset                     # Reset simulation
```

---

## Usage Examples

### Racing Setup
```cpp
executor.execute("loadTerrainPreset racetrack");
executor.execute("createCar 0 5 0 1100 200000");
executor.execute("setTireGrip 1.5");
executor.execute("setTractionControl 1");
executor.execute("setABS 1");
```

### Rally Setup
```cpp
auto terrain = std::make_unique<Terrain>(100, 100);
terrain->generateByType(TerrainType::HILLS);
terrain->addRamp(20, 30, 10, 20, 8);
terrain->setMaterialZone(-15, -15, 10, 0.6f);  // Mud
physics.setTerrain(std::move(terrain));

Car* car = physics.createCar(Vector3(0, 5, 0));
car->setMass(1100.0f);
car->setSuspensionStiffness(40000.0f);
car->setTireGrip(1.2f);
car->setTractionControl(true);
```

### Stunt Course
```cpp
executor.execute("loadTerrainPreset flat");
executor.execute("addRamp 0 20 12 25 10");
executor.execute("addObstacle 30 30 8 5");
executor.execute("createCar 0 5 0 1000 180000");
```

---

## WebAssembly API

```javascript
// Initialize
Module.ccall('initEngine', null, ['number', 'number'], [width, height]);

// Execute code
Module.ccall('executeCode', 'number', ['string'], [code]);

// Update physics
Module.ccall('updatePhysics', null, ['number'], [deltaTime]);

// Get error
Module.ccall('getLastError', 'string', [], []);
```

## Mobile Library API

```cpp
// Initialize
CarX_Initialize(width, height);

// Execute
int success = CarX_ExecuteCode("createCar 0 5 0");

// Update
CarX_Update(deltaTime);
CarX_Render();

// Get data
CarX_GetCarPosition(0, &x, &y, &z);
```

---

## Physics Reference

### Centripetal Force
```
F_c = m * v² / r
```

### Aerodynamic Drag
```
F_d = 0.5 * ρ * Cd * A * v²
```

### Tire Grip
```
F_max = μ * N
```
Where μ = grip coefficient, N = normal force

---

## See Also

- [ADVANCED_VEHICLE_DYNAMICS.md](ADVANCED_VEHICLE_DYNAMICS.md) - Professional physics
- [PHYSICS_COMPONENTS_AND_EXPERIMENTS.md](PHYSICS_COMPONENTS_AND_EXPERIMENTS.md) - Code experiments
- [BUILDING.md](BUILDING.md) - Build instructions
- [TECHNOLOGY.md](TECHNOLOGY.md) - Tech stack overview
