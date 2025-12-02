# CarX Simulator - Unified Scene Definition System

## Overview

The Unified Scene Definition System ensures that scenes created in Blender match exactly what appears in Three.js (and future platforms like Unreal Engine, Unity, Android, etc.).

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                  scene_definition.json                       │
│              (Single Source of Truth)                        │
└─────────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
        ▼                   ▼                   ▼
┌───────────────┐   ┌───────────────┐   ┌───────────────┐
│    Blender    │   │   Three.js    │   │ Unreal/Unity  │
│ scene_loader  │   │ scene_loader  │   │ scene_loader  │
│     .py       │   │     .js       │   │   (future)    │
└───────────────┘   └───────────────┘   └───────────────┘
        │                   │                   │
        ▼                   ▼                   ▼
┌───────────────┐   ┌───────────────┐   ┌───────────────┐
│   Blender     │   │  Browser      │   │    Game       │
│   Scene       │   │  Preview      │   │   Engine      │
└───────────────┘   └───────────────┘   └───────────────┘
```

## File Structure

```
CarXSimulator/
├── scene_definition.json      # ← SINGLE SOURCE OF TRUTH
├── tools/
│   ├── scene_loader.py        # Python loader (Blender, scripts)
│   └── blender_scene_tools.py # Blender integration
├── preview/
│   ├── scene_loader.js        # JavaScript loader (Three.js)
│   ├── final-demo.html        # Web preview
│   └── lowpoly_game_model.glb # Car model asset
└── docs/
    └── UNIFIED_SCENE_SYSTEM.md # This file
```

## scene_definition.json

The central configuration file that defines:

### 1. Coordinate System
```json
{
  "coordinate_system": {
    "up": "Y",
    "forward": "-Z",
    "right": "X",
    "units": "meters"
  }
}
```

### 2. Assets
```json
{
  "assets": {
    "car": {
      "model": "preview/lowpoly_game_model.glb",
      "fallback": "procedural",
      "scale": 1.0
    },
    "tree": {
      "model": "preview/tree.glb",
      "fallback": "procedural"
    }
  }
}
```

### 3. Car Definition
```json
{
  "car": {
    "name": "Car",
    "transform": {
      "position": [0, 0.85, 0],
      "rotation": [0, 0, 0],
      "scale": [1, 1, 1]
    },
    "physics": {
      "mass": 1200,
      "wheelbase": 2.6,
      "track_width": 1.8,
      "wheel_radius": 0.35,
      "ground_clearance": 0.5
    },
    "wheels": [
      {"name": "wheel_FL", "position": [-0.9, 0.35, 1.3], "steering": true},
      {"name": "wheel_FR", "position": [0.9, 0.35, 1.3], "steering": true},
      {"name": "wheel_RL", "position": [-0.9, 0.35, -1.3], "drive": true},
      {"name": "wheel_RR", "position": [0.9, 0.35, -1.3], "drive": true}
    ],
    "body_parts": [
      {
        "name": "body",
        "type": "box",
        "dimensions": [2.0, 0.8, 4.0],
        "position": [0, 0.6, 0],
        "material": {"color": "#FF6B35", "metalness": 0.7, "roughness": 0.3}
      }
    ]
  }
}
```

### 4. Terrain Definition
```json
{
  "terrain": {
    "type": "procedural",
    "size": 400,
    "segments": 120,
    "noise": {
      "type": "perlin",
      "frequency": [0.02, 0.015, 0.01],
      "amplitude": [2, 1.5, 3]
    }
  }
}
```

### 5. Environment
```json
{
  "environment": {
    "sun": {
      "position": [50, 100, 50],
      "color": "#FFF5E6",
      "intensity": 1.5,
      "shadows": true
    },
    "ambient": {
      "color": "#FFFFFF",
      "intensity": 0.4
    },
    "fog": {
      "enabled": true,
      "near": 100,
      "far": 500
    }
  }
}
```

## Usage

### Python (Blender)

```python
from scene_loader import load_scene_definition

# Load scene definition
scene = load_scene_definition("scene_definition.json")

# Access car data
car = scene.get_car()
print(f"Car mass: {car.physics.mass} kg")
print(f"Wheels: {len(car.wheels)}")

# Access terrain data
terrain = scene.get_terrain()
height = terrain.get_height(10, 20)  # Get height at position
```

### JavaScript (Three.js)

```javascript
import { loadSceneDefinition, ThreeJSSceneBuilder } from './scene_loader.js';

// Load scene definition
const sceneDef = await loadSceneDefinition('../scene_definition.json');

// Create scene builder
const builder = new ThreeJSSceneBuilder(sceneDef);

// Create procedural car (if GLB not available)
const car = builder.createProceduralCar();
scene.add(car);

// Create terrain
const terrain = builder.createTerrain();
scene.add(terrain);

// Get terrain height (matches Blender exactly!)
const height = sceneDef.getTerrain().getHeight(10, 20);
```

### Blender CLI

```bash
# Generate scene from unified definition
blender -b --python blender_scene_tools.py -- --generate --sync-to-carx

# Verify consistency
blender -b --python blender_scene_tools.py -- --verify
```

## Model Transforms

**Important**: The GLB car model requires specific transforms to match `final-demo.html`:

| Property | Three.js | Blender |
|----------|----------|---------|
| Scale | `carMesh.scale.set(2.5, 2.5, 2.5)` | `car.scale = (2.5, 2.5, 2.5)` |
| Rotation | `carMesh.rotation.y = -Math.PI / 2` | `car.rotation_euler = (0, 0, -math.pi / 2)` |

The `generate_scene()` function in `blender_scene_tools.py` applies these automatically.

## Terrain Height Function

The terrain height function uses FBM (Fractal Brownian Motion) for the 'hills' terrain type:

```javascript
// From final-demo.html
height = this.fbm(x, z, 4, 0.5, 50) * 8 + this.fbm(x, z, 2, 0.3, 20) * 2;
```

The Blender implementation uses a sin/cos approximation of FBM to match this behavior.

This ensures that:
1. Blender terrain matches Three.js terrain visually
2. Car physics work correctly on both platforms
3. Tree placement is consistent

## Workflow

### Making Changes

1. **Edit `scene_definition.json`** - Change car dimensions, terrain parameters, etc.

2. **Regenerate Blender scene**:
   ```bash
   blender -b --python blender_scene_tools.py -- --generate --sync-to-carx
   ```

3. **Refresh browser** - Three.js will load the updated definition

4. **Verify consistency**:
   ```bash
   blender -b --python blender_scene_tools.py -- --verify
   ```

### Adding New Platforms

To add support for a new platform (e.g., Unreal Engine):

1. Create a new scene loader (e.g., `scene_loader.cpp` or Blueprint)
2. Implement the same data structures
3. Implement the terrain height function identically
4. Load from `scene_definition.json`

Example for Unreal Engine (C++):

```cpp
// UnrealSceneLoader.h
class FSceneDefinition {
public:
    static FSceneDefinition LoadFromJSON(const FString& Path);
    
    FCarDefinition GetCar() const;
    FTerrainDefinition GetTerrain() const;
    
    // Terrain height - MUST match Python/JS implementation!
    float GetTerrainHeight(float X, float Z) const {
        float H = 0;
        for (int i = 0; i < Noise.Frequency.Num(); i++) {
            H += FMath::Sin(X * Noise.Frequency[i]) * Noise.Amplitude[i];
            H += FMath::Cos(Z * Noise.Frequency[i] * 0.75f) * Noise.Amplitude[i] * 0.75f;
        }
        return H;
    }
};
```

## Benefits

1. **Single Source of Truth** - One file defines everything
2. **Cross-Platform Consistency** - Same scene in Blender, Three.js, Unreal, etc.
3. **Easy Iteration** - Change JSON, regenerate everywhere
4. **Version Control Friendly** - JSON is easy to diff and merge
5. **Extensible** - Add new properties without breaking existing code
6. **Human Readable** - Easy to understand and edit manually

## Best Practices

1. **Always use scene_definition.json** - Don't hardcode values
2. **Keep terrain function identical** - Critical for physics consistency
3. **Use relative paths** - For asset references
4. **Validate after changes** - Run `--verify` in Blender
5. **Test on all platforms** - After making changes
6. **Document custom properties** - If you extend the schema

## Troubleshooting

### Scene doesn't match between platforms

1. Check that both platforms are loading the same `scene_definition.json`
2. Verify terrain height function implementation
3. Check coordinate system (Y-up)
4. Verify asset paths are correct

### Terrain height mismatch

The terrain height function MUST be implemented identically:

```python
# Python
h = 0
for freq, amp in zip(frequencies, amplitudes):
    h += math.sin(x * freq) * amp
    h += math.cos(z * freq * 0.75) * amp * 0.75
```

```javascript
// JavaScript
let h = 0;
for (let i = 0; i < freq.length; i++) {
    h += Math.sin(x * freq[i]) * amp[i];
    h += Math.cos(z * freq[i] * 0.75) * amp[i] * 0.75;
}
```

### Assets not loading

1. Check asset paths in `scene_definition.json`
2. Verify files exist at specified paths
3. Check for CORS issues in browser
4. Use fallback procedural generation if needed

## Future Enhancements

- [ ] JSON Schema validation
- [ ] Visual scene editor
- [ ] Real-time sync between platforms
- [ ] Asset bundling for deployment
- [ ] Scene versioning and migration
- [ ] Unreal Engine plugin
- [ ] Unity package
- [ ] Android/iOS native loaders
