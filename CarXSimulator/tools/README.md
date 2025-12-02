# CarX Simulator - Blender Tools v2.0

Robust import/export pipeline between CarX Simulator and Blender with multi-format support.

## Features

- ✅ **Multi-Format Import**: GLB, FBX, OBJ, DAE, STL, PLY, Alembic
- ✅ **Animation Support**: Export/import animations, create wheel spin
- ✅ **Format Conversion**: Convert any format to CarX-compatible GLB
- ✅ **Round-Trip Pipeline**: Edit in Blender → Export → Use in CarX
- ✅ **Blender 2.80 - 4.x**: Auto-detects API changes
- ✅ **Crash-Safe Export**: Fallback modes if primary export fails

## Quick Start

### Interactive Mode
```bash
blender --python blender_scene_tools.py
```
Opens Blender with "CarX" panel in 3D View sidebar (press N).

### Command Line
```bash
# Show help
blender -b --python blender_scene_tools.py -- --help

# Generate and export
blender -b --python blender_scene_tools.py -- --generate --export scene.glb

# Import FBX and convert to GLB
blender -b --python blender_scene_tools.py -- --convert model.fbx

# Import OBJ and update car
blender --python blender_scene_tools.py -- --import car.obj --update-scene
```

## Supported Formats

| Format | Import | Export | Notes |
|--------|--------|--------|-------|
| GLB/glTF | ✅ | ✅ | Primary format for Three.js |
| FBX | ✅ | - | Common 3D interchange |
| OBJ | ✅ | - | Simple mesh format |
| DAE (Collada) | ✅ | - | XML-based format |
| STL | ✅ | - | 3D printing format |
| PLY | ✅ | - | Point cloud/mesh |
| Alembic | ✅ | - | Animation cache |

## CLI Options

```
GENERATION:
  --generate              Generate full scene (car + terrain + trees)
  --generate-car          Generate car model only
  --no-terrain            Skip terrain
  --no-trees              Skip trees
  --trees N               Number of trees (default: 20)

EXPORT:
  --export FILE           Export scene to GLB
  --export-car FILE       Export car only
  --export-anim FILE      Export with animations
  --export-json FILE      Export metadata as JSON

IMPORT:
  --import FILE           Import any 3D file (GLB, FBX, OBJ, etc.)
  --update-scene          Replace existing car with imported
  --convert FILE          Convert any format to GLB

ANIMATION:
  --wheel-anim            Create wheel spin animation
  --anim-duration N       Duration in seconds (default: 2.0)
  --anim-rotations N      Number of rotations (default: 4.0)

PIPELINE:
  --sync-to-carx          Export to CarX preview/assets
  --sync-from-carx        Import from CarX preview/assets
  --clear                 Clear scene before operations
```

## Workflow Examples

### Import FBX Car Model
```bash
# Convert FBX to GLB
blender -b --python blender_scene_tools.py -- --convert my_car.fbx

# Or import and edit interactively
blender --python blender_scene_tools.py -- --import my_car.fbx
```

### Update Car from External File
```bash
# Import OBJ and replace existing car
blender --python blender_scene_tools.py -- --import new_car.obj --update-scene

# Then export
blender -b scene.blend --python blender_scene_tools.py -- --export-car car.glb
```


### Export with Wheel Animation
```bash
# Generate car with spinning wheels animation
blender -b --python blender_scene_tools.py -- \
    --generate-car \
    --wheel-anim \
    --anim-duration 3.0 \
    --anim-rotations 6 \
    --export-anim animated_car.glb
```

### Full Pipeline (Edit → Export → Use)
```bash
# 1. Generate base scene
blender --python blender_scene_tools.py -- --generate

# 2. Edit in Blender (change colors, add details, etc.)
# ... make your edits ...

# 3. Sync back to CarX
# From Blender UI: Click "Sync to CarX" button
# Or via CLI:
blender -b edited_scene.blend --python blender_scene_tools.py -- --sync-to-carx
```

### Batch Convert Multiple Files
```bash
for file in models/*.fbx; do
    blender -b --python blender_scene_tools.py -- --convert "$file"
done
```

## Blender UI Panel

The "CarX" tab in 3D View sidebar includes:

**Generate:**
- Generate CarX Scene (with options dialog)
- Generate Car Only

**Export:**
- Export GLB (full scene)
- Export Car GLB
- Export with Animations

**Import:**
- Import Any Format (FBX, OBJ, DAE, etc.)
- Update Car (replace existing)
- Convert to GLB

**Animation:**
- Create Wheel Animation

**Pipeline:**
- Sync to CarX
- Sync from CarX

## Required Transforms

**Important**: The GLB car model requires specific transforms to match `final-demo.html`:

| Property | Three.js | Blender |
|----------|----------|---------|
| Scale | `2.5, 2.5, 2.5` | `(2.5, 2.5, 2.5)` |
| Rotation | `rotation.y = -π/2` (-90°) | `rotation_euler.z = -π/2` (-90°) |

The `generate_scene()` function applies these automatically. See [PIPELINE_GUIDE.md](PIPELINE_GUIDE.md) for details.

## Model Structure

### Expected Car Hierarchy
```
Car (Empty)
├── Body (Mesh)
├── Cabin (Mesh)
├── wheel_FL (Empty)  ← Front Left
│   ├── wheel_FL_tire (Mesh)
│   └── wheel_FL_rim (Mesh)
├── wheel_FR (Empty)  ← Front Right
├── wheel_RL (Empty)  ← Rear Left
├── wheel_RR (Empty)  ← Rear Right
└── ... other parts
```

### Automatic Wheel Detection
The script automatically identifies wheels by name patterns:
- `wheel`, `tire`, `tyre`, `rim`
- Position hints: `fl`, `fr`, `rl`, `rr`, `front_left`, etc.

Imported models are automatically reorganized to match this structure.

## Animation Support

### Wheel Spin Animation
Creates keyframed rotation for all wheels:
- Rotates around local X axis
- Linear interpolation
- Configurable duration and rotation count

### Export with Animations
Exports include:
- All actions/animations
- NLA strips
- Frame range
- Proper sampling for Three.js

### Three.js Animation Loading
```javascript
loader.load('animated_car.glb', (gltf) => {
    const mixer = new THREE.AnimationMixer(gltf.scene);
    gltf.animations.forEach((clip) => {
        mixer.clipAction(clip).play();
    });
    scene.add(gltf.scene);
});
```

## Troubleshooting

### FBX Import Not Available
Enable addon: Edit → Preferences → Add-ons → Search "FBX" → Enable

### OBJ Import Fails (Blender 4.0+)
Blender 4.0 changed OBJ importer location. The script handles this automatically.

### Animations Not Exporting
- Ensure objects have animation data
- Check frame range in timeline
- Use `--export-anim` instead of `--export`

### Wheel Animation Not Working
- Ensure car object is named "Car"
- Wheels should be child empties or meshes with "wheel" in name

### Materials Look Different
- Use Principled BSDF shader
- Check metalness/roughness values
- Pack textures before export

## API Reference

```python
# Import in Blender script
import sys
sys.path.append('/path/to/CarXSimulator/tools')
from blender_scene_tools import *

# Multi-format import
objects = import_any_format("model.fbx")
objects = import_any_format("model.obj")

# Convert format
convert_to_carx_format("input.fbx", "output.glb")

# Update scene from external
update_scene_from_external("new_car.fbx", replace_car=True)

# Animation
create_wheel_spin_animation(car_object, duration=2.0, rotations=4.0)
export_with_animations("animated.glb")

# Get animation info
anims = get_scene_animations()
clear_animations()

# Pipeline
sync_to_carx()
sync_from_carx()
```

---

*Version 2.0 | Blender 2.80 - 4.x | Multi-format support*
