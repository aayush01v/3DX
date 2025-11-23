# Quick Start Guide - CarX Simulator

## Getting Started in 5 Minutes

### 1. Basic Setup
```cpp
#include "PhysicsEngine.h"
#include "CodeExecutor.h"

// Create physics engine
PhysicsEngine physics;

// Create code executor for scripting
CodeExecutor executor(&physics);
executor.registerAPI();
```

### 2. Create Terrain
```cpp
// Option A: Use preset
executor.execute("loadTerrainPreset hills");

// Option B: Generate custom
auto terrain = std::make_unique<Terrain>(100, 100);
terrain->generateByType(TerrainType::MOUNTAINS);
physics.setTerrain(std::move(terrain));
```

### 3. Create a Car
```cpp
// Using API
executor.execute("createCar 0 5 0");

// Or directly
Car* car = physics.createCar(Vector3(0, 5, 0));
car->setMass(1200.0f);
car->setEnginePower(150000.0f);
```

### 4. Control the Car
```cpp
// Throttle (forward/reverse)
car->applyThrottle(0.8f);   // 80% throttle

// Steering
car->applySteering(0.3f);   // Turn right

// Braking
car->applyBrake(0.5f);      // 50% brake
```

### 5. Update Physics
```cpp
// In your game loop
float deltaTime = 1.0f / 60.0f;  // 60 FPS
physics.update(deltaTime);
```

---

## Common Scenarios

### Scenario 1: Racing Game
```cpp
// Create racetrack
executor.execute("loadTerrainPreset racetrack");

// Create performance car
executor.execute("createCar 0 5 0 1100 200000");
executor.execute("setTireGrip 1.5");
executor.execute("setTractionControl 1");
executor.execute("setABS 1");
```

### Scenario 2: Off-Road Rally
```cpp
// Create hilly terrain
auto terrain = std::make_unique<Terrain>(100, 100);
terrain->generateByType(TerrainType::HILLS);
terrain->addRamp(20, 30, 10, 20, 8);  // Jump
terrain->setMaterialZone(-15, -15, 10, 0.6f);  // Mud patch
physics.setTerrain(std::move(terrain));

// Rally car setup
Car* car = physics.createCar(Vector3(0, 5, 0));
car->setMass(1100.0f);
car->setSuspensionStiffness(40000.0f);  // Softer
car->setSuspensionDamping(2500.0f);
car->setTireGrip(1.2f);
car->setTractionControl(true);
```

### Scenario 3: Stunt Course
```cpp
// Create custom terrain
executor.execute("loadTerrainPreset flat");
executor.execute("addRamp 0 20 12 25 10");      // Big jump
executor.execute("addObstacle 30 30 8 5");      // Hill
executor.execute("addRamp -20 15 8 15 6");      // Side ramp

// Stunt car
executor.execute("createCar 0 5 0 1000 180000");
executor.execute("setTireGrip 1.3");
```

---

## Scripting Examples

### Example 1: Complete Setup Script
```cpp
std::string script = R"(
    # Load terrain
    loadTerrainPreset mountains
    
    # Add features
    addRamp 15 20 10 20 8
    addObstacle -10 -10 5 4
    
    # Create car
    createCar 0 10 0 1200 150000
    
    # Configure car
    setTireGrip 1.2
    setTractionControl 1
    setABS 1
)";

executor.executeScript(script);
```

### Example 2: Using Variables
```cpp
executor.execute("mass = 1500");
executor.execute("power = 200000");
executor.execute("createCar 0 5 0 mass power");
```

### Example 3: Get Help
```cpp
// List all commands
auto commands = executor.getAvailableCommands();

// Get suggestions
auto suggestions = executor.getSuggestions("set");

// Get command help
std::string help = executor.getCommandHelp("createCar");

// Full documentation
std::string docs = executor.getAPIDocumentation();
```

---

## Physics Configuration

### Basic Configuration
```cpp
PhysicsConfig config;
config.substeps = 4;                    // Physics substeps per frame
config.useAdaptiveSubsteps = true;      // Adjust based on performance
config.enableCollisionCache = true;     // Cache terrain queries
config.collisionCacheSize = 256;        // Cache size

physics.setConfig(config);
```

### Gravity and Environment
```cpp
// Standard gravity
physics.setGravity(Vector3(0, -9.81f, 0));

// Low gravity (moon)
physics.setGravity(Vector3(0, -1.62f, 0));

// Wind effects
physics.setWindVelocity(Vector3(5, 0, 0));  // 5 m/s wind

// Air density (affects drag)
physics.setAirDensity(1.225f);  // Sea level
```

---

## Terrain Customization

### Procedural Generation
```cpp
auto terrain = std::make_unique<Terrain>(100, 100);

// Generate with custom parameters
terrain->generateProcedural(
    10.0f,   // amplitude (height variation)
    0.1f     // frequency (detail level)
);

// Add curvature (affects gravity)
terrain->setCurvature(0.2f);

physics.setTerrain(std::move(terrain));
```

### Adding Features
```cpp
// Obstacles
terrain->addObstacle(x, z, radius, height);

// Ramps
terrain->addRamp(x, z, width, length, height);

// Smooth rough areas
terrain->smoothTerrain(3);  // 3 iterations

// Apply erosion
terrain->applyErosion(0.1f);

// Material zones
terrain->setMaterialZone(x, z, radius, friction);
```

---

## Car Tuning Guide

### Mass and Power
```cpp
// Lightweight sports car
car->setMass(1000.0f);
car->setEnginePower(200000.0f);

// Heavy truck
car->setMass(2500.0f);
car->setEnginePower(180000.0f);
```

### Suspension Tuning
```cpp
// Stiff (racing)
car->setSuspensionStiffness(60000.0f);
car->setSuspensionDamping(4000.0f);

// Soft (off-road)
car->setSuspensionStiffness(35000.0f);
car->setSuspensionDamping(2000.0f);
```

### Tire Configuration
```cpp
// Racing slicks
car->setTireGrip(1.5f);

// All-terrain
car->setTireGrip(1.1f);

// Ice/snow
car->setTireGrip(0.6f);
```

### Aerodynamics
```cpp
// Low drag (top speed)
car->setDragCoefficient(0.25f);

// High downforce (cornering)
car->setDragCoefficient(0.35f);
```

### Driver Aids
```cpp
// Enable all aids
car->setTractionControl(true);
car->setAntiLockBrakes(true);

// Differential type
car->setDifferentialType(0);  // Open (drifting)
car->setDifferentialType(1);  // Limited-slip (balanced)
car->setDifferentialType(2);  // Locked (off-road)
```

---

## Reading Car Data

### Basic Info
```cpp
Vector3 pos = car->getPosition();
Vector3 vel = car->getVelocity();
float speed = car->getSpeed();  // m/s
float rpm = car->getRPM();
int gear = car->getGear();
```

### Advanced Telemetry
```cpp
// Forces
Vector3 centripetal = car->getCentripetalForce();
Vector3 gravity = car->getGravityForce(terrainNormal);
Vector3 total = car->getTotalForce();

// Dynamics
float slipAngle = car->getSlipAngle();
float lateralG = car->getLateralAcceleration();
bool airborne = car->isAirborne();

// Suspension
for (int i = 0; i < 4; ++i) {
    float compression = car->getSuspensionCompression(i);
    Vector3 wheelPos = car->getWheelPosition(i);
}
```

---

## Performance Tips

1. **Use Collision Cache**: Enables ~3x faster terrain queries
2. **Adaptive Substeps**: Maintains stability across different framerates
3. **Cache Normals**: Pre-compute terrain normals for better performance
4. **Limit Terrain Size**: 100x100 is good balance between detail and performance
5. **Batch Updates**: Update all cars in single physics step

---

## Troubleshooting

### Car Falls Through Terrain
```cpp
// Ensure terrain is set before creating car
physics.setTerrain(std::move(terrain));
Car* car = physics.createCar(Vector3(0, 10, 0));  // Start above ground
```

### Car Won't Move
```cpp
// Check throttle is applied
car->applyThrottle(0.8f);

// Check car isn't stuck in terrain
Vector3 pos = car->getPosition();
float terrainHeight = physics.getTerrain()->getHeight(pos.x, pos.z);
```

### Unstable Physics
```cpp
// Increase substeps
PhysicsConfig config;
config.substeps = 8;
physics.setConfig(config);

// Or reduce time scale
physics.setTimeScale(0.5f);
```

### Poor Performance
```cpp
// Enable optimizations
PhysicsConfig config;
config.enableCollisionCache = true;
config.useAdaptiveSubsteps = true;
physics.setConfig(config);

// Cache terrain normals
terrain->cacheNormals();
```

---

## Next Steps

- Read [IMPROVEMENTS.md](IMPROVEMENTS.md) for detailed feature documentation
- Check [API.md](API.md) for complete API reference
- See [examples/](../examples/) for more code samples
- Experiment with different terrain types and car configurations!
