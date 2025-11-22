# CarX Simulator API Documentation

## Physics Engine API

### Car Class

```cpp
Car(const Vector3& position);
```

#### Control Methods
- `void applyThrottle(float amount)` - Apply throttle (-1 to 1)
- `void applySteering(float angle)` - Apply steering angle in radians
- `void applyBrake(float amount)` - Apply brake force (0 to 1)

#### Physics Properties
- `void setMass(float m)` - Set car mass in kg
- `void setWheelbase(float wb)` - Set wheelbase in meters
- `void setDragCoefficient(float cd)` - Set aerodynamic drag coefficient

#### Getters
- `Vector3 getPosition()` - Get current position
- `Vector3 getVelocity()` - Get current velocity
- `float getSpeed()` - Get speed magnitude
- `Quaternion getRotation()` - Get orientation

#### Force Analysis
- `Vector3 getCentripetalForce()` - Get centripetal force vector
- `Vector3 getCentrifugalForce()` - Get centrifugal force (reaction)
- `Vector3 getGravityForce(const Vector3& terrainNormal)` - Get gravity adjusted for terrain

### Terrain Class

```cpp
Terrain(int width = 100, int depth = 100);
```

#### Methods
- `void generateProcedural(float amplitude, float frequency)` - Generate procedural terrain
- `bool loadHeightmap(const std::string& filename)` - Load from heightmap image
- `float getHeight(float x, float z)` - Get interpolated height at position
- `Vector3 getNormal(float x, float z)` - Get terrain normal
- `Vector3 getGravityDirection(float x, float z)` - Get gravity direction affected by curvature
- `void setCurvature(float c)` - Set terrain curvature (0-1)

### PhysicsEngine Class

```cpp
PhysicsEngine();
```

#### Methods
- `void update(float deltaTime)` - Update physics simulation
- `Car* createCar(const Vector3& position)` - Create new car
- `void setTerrain(std::unique_ptr<Terrain> terrain)` - Set terrain
- `void setGravity(const Vector3& g)` - Set gravity vector

## Code Execution API

### Car Commands

```cpp
// Create a car at position with optional mass and power
createCar <x> <y> <z> [mass] [power]

// Car control
setThrottle <amount>      // -1.0 to 1.0
setSteering <angle>       // -1.0 to 1.0
setBrake <amount>         // 0.0 to 1.0

// Car properties
setCarMass <kg>           // Mass in kilograms
setTireGrip <coeff>       // 0.5 to 2.0 (1.0 = normal)
```

### Terrain Commands

```cpp
// Load terrain preset
loadTerrainPreset <type>  // flat, hills, mountains, valley, racetrack

// Generate custom terrain
generateTerrain <amplitude> <frequency> [curvature]

// Set gravity vector
setGravity <x> <y> <z>
```

### Simulation Commands

```cpp
// Reset simulation
reset
```

### Examples

```cpp
// Race track setup
loadTerrainPreset racetrack
createCar 0 5 0 1200 180000
setTireGrip 1.2

// Mountain driving
loadTerrainPreset mountains
createCar -20 15 0 1500 150000
setGravity 0 -9.81 0

// Low gravity test
loadTerrainPreset hills
createCar 0 5 0
setGravity 0 -3.0 0

// High performance car
createCar 0 5 0 1000 250000
setTireGrip 1.5
setThrottle 0.8
```

## WebAssembly Exports

```javascript
// Initialize engine
Module.ccall('initEngine', null, ['number', 'number'], [width, height]);

// Execute code
const result = Module.ccall('executeCode', 'number', ['string'], [code]);

// Get error message
const error = Module.ccall('getLastError', 'string', [], []);

// Update physics
Module.ccall('updatePhysics', null, ['number'], [deltaTime]);
```

## Mobile Library Exports

### React Native / Flutter

```cpp
// Initialize
CarX_Initialize(width, height);

// Execute code
int success = CarX_ExecuteCode("createCar 0 5 0");

// Get error
const char* error = CarX_GetLastError();

// Update and render
CarX_Update(deltaTime);
CarX_Render();

// Get car data
int count = CarX_GetCarCount();
float x, y, z;
CarX_GetCarPosition(0, &x, &y, &z);
```

## Physics Concepts

### Centripetal Force
Force directed toward the center of circular motion:
```
F_c = m * v² / r
```

### Centrifugal Force
Apparent outward force in rotating reference frame (reaction to centripetal):
```
F_cf = -F_c
```

### Curved Terrain Gravity
Gravity direction adjusted by terrain curvature:
```
g' = g * (1 - curvature) + normal * (-curvature)
```
