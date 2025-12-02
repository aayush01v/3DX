# Blender ↔ CarX Simulator Sync Guide

## Overview
The updated `blender_scene_tools.py` now maintains **bidirectional consistency** between Blender and the Three.js scene in `final-demo.html`.

## Key Improvements

### 1. **Existing Model Loading**
- Script automatically loads existing GLB models from `preview/` directory
- Car model: `lowpoly_game_model.glb`
- Tree model: `tree.glb`
- Falls back to procedural generation if models not found

### 2. **Required Transforms (from final-demo.html)**
The GLB model requires specific transforms to match `final-demo.html`:

| Property | Three.js | Blender |
|----------|----------|---------|
| Scale | `carMesh.scale.set(2.5, 2.5, 2.5)` | `car.scale = (2.5, 2.5, 2.5)` |
| Rotation | `carMesh.rotation.y = -Math.PI / 2` | `car.rotation_euler = (0, 0, -math.pi / 2)` |

The `generate_scene()` function applies these automatically.

### 3. **Coordinate System Consistency**
- Both Blender and Three.js use **Y-up** coordinate system
- Car orientation: Forward is **-Z direction** in physics
- The -90° rotation aligns the model's visual front with physics forward

### 3. **Scene Specifications Match final-demo.html**
- Terrain: 400x400 units, 120 segments (was 200x200)
- Car dimensions and colors exactly match Three.js scene
- Wheel positions corrected for proper alignment
- Lighting matches (Sun at 50,100,50 with warm color)

### 4. **Export Consistency**
- `sync_to_carx()` exports to `preview/lowpoly_game_model.glb` (matches HTML loader)
- Proper scale and rotation preserved
- Optional full scene export as `scene.glb`

## Usage

### Generate Scene with Existing Models
```bash
# Load existing GLB models and generate scene
blender -b --python blender_scene_tools.py -- --generate --use-existing

# Export back to CarX
blender -b --python blender_scene_tools.py -- --generate --use-existing --sync-to-carx
```

### Create New Procedural Scene
```bash
# Generate from scratch (procedural car + terrain)
blender -b --python blender_scene_tools.py -- --generate --export scene.glb
```

### Import External Model and Update
```bash
# Import FBX/OBJ and replace car
blender -b --python blender_scene_tools.py -- --import custom_car.fbx --update-scene --sync-to-carx
```

### Interactive Mode
```bash
# Open Blender with CarX panel
blender --python blender_scene_tools.py

# Then use the CarX sidebar panel (press N in 3D view)
```

## Workflow Examples

### 1. Edit Existing Car in Blender
```bash
# 1. Load existing scene
blender --python blender_scene_tools.py -- --use-existing --generate

# 2. Edit in Blender GUI (modify materials, add details, etc.)

# 3. Export back to CarX
# In Blender: CarX Panel → "Sync to CarX"
# Or CLI: blender -b scene.blend --python blender_scene_tools.py -- --sync-to-carx
```

### 2. Import Custom 3D Model
```bash
# Import and convert to CarX format
blender -b --python blender_scene_tools.py -- --import my_car.fbx --update-scene --sync-to-carx

# This will:
# - Import the FBX
# - Organize hierarchy (rename to "Car", identify wheels)
# - Export as lowpoly_game_model.glb
# - Ready to use in final-demo.html
```

### 3. Create Animated Car
```bash
# Generate car with wheel spin animation
blender -b --python blender_scene_tools.py -- --generate-car --wheel-anim --export-anim animated_car.glb
```

## File Locations

```
CarXSimulator/
├── preview/
│   ├── lowpoly_game_model.glb  ← Car model (loaded by final-demo.html)
│   ├── tree.glb                ← Tree model
│   ├── scene_data.json         ← Metadata
│   └── final-demo.html         ← Three.js viewer
└── tools/
    ├── blender_scene_tools.py  ← This script
    └── BLENDER_SYNC_GUIDE.md   ← This guide
```

## Coordinate System Reference

### Blender → Three.js Mapping
- **Position**: Direct 1:1 mapping (both Y-up)
- **Rotation**: -90° on Z-axis (Blender) = -90° on Y-axis (Three.js)
- **Scale**: 2.5x applied in both systems

### Why the -90° Rotation?
The GLB model's forward direction doesn't match the physics engine's forward direction (-Z). The rotation aligns:
- Model's visual front → Physics forward direction
- Steering and movement work correctly

### Wheel Positions (Both Systems)
```
Front Left:  (-0.9, 0.35,  1.3)
Front Right: ( 0.9, 0.35,  1.3)
Rear Left:   (-0.9, 0.35, -1.3)
Rear Right:  ( 0.9, 0.35, -1.3)
```

## Troubleshooting

### Car appears rotated wrong in Three.js
- Ensure car's forward direction is along -Z in Blender
- Check rotation_euler[2] = -π/2 is applied during export

### Models not loading
- Check file paths: `preview/lowpoly_game_model.glb` must exist
- Verify GLB export settings (use `--verbose` flag)

### Scale mismatch
- Car should be scaled 2.5x in Three.js
- Blender model should be at 1.0 scale
- Export handles this automatically

## Advanced Features

### Custom Terrain Functions
Edit `terrain_height_function()` in the script to match your Three.js terrain:
```python
def terrain_height_function(x: float, z: float) -> float:
    h = math.sin(x * 0.02) * 2
    h += math.cos(z * 0.015) * 1.5
    h += math.sin((x + z) * 0.01) * 3
    return h
```

### Material Consistency
Materials are automatically converted to PBR (Principled BSDF):
- Metalness and roughness preserved
- Colors match exactly
- Emission for lights (headlights/taillights)

## Version Compatibility
- Blender 2.80 - 5.0+ (auto-detects API changes)
- Three.js r128+ (GLTFLoader required)
- Tested on Linux, macOS, Windows

## Support
For issues or questions, check:
- `blender_scene_tools.py --help`
- CarX Simulator documentation
- Blender API docs: https://docs.blender.org/api/
