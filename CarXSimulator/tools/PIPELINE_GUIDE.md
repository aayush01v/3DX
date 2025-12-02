# CarX Simulator - Blender Pipeline Guide

## Overview

This guide explains the complete pipeline between Blender and the CarX Simulator (Three.js) for consistent 3D model synchronization.

## Coordinate System & Transforms

**IMPORTANT**: Understanding the coordinate system conversion is critical for correct model orientation.

### Coordinate Systems

| System | Right | Up | Forward |
|--------|-------|-----|---------|
| **Three.js / glTF** | +X | +Y | -Z |
| **Blender (native)** | +X | +Z | -Y |
| **Blender (after glTF import)** | +X | +Z | -Y |

When Blender imports a glTF/GLB file, it converts from Y-up to Z-up:
- glTF Y-axis → Blender Z-axis (up)
- glTF Z-axis → Blender -Y-axis (forward)

### Required Transforms (from final-demo.html)

The `final-demo.html` applies these transforms to the loaded GLB model:

```javascript
// From final-demo.html lines ~2100-2110:
carMesh.scale.set(2.5, 2.5, 2.5);           // Scale up for visibility
carMesh.rotation.y = -Math.PI / 2;          // -90° yaw to align with physics
```

### Blender Equivalent

When loading the GLB in Blender, apply the equivalent transforms:

```python
# Scale: 2.5x (matches Three.js)
car.scale = (2.5, 2.5, 2.5)

# Rotation: -90° on Z-axis (Blender's yaw axis after glTF import)
# Three.js rotation.y (yaw around Y-up) = Blender rotation_euler.z (yaw around Z-up)
car.rotation_euler = (0, 0, -math.pi / 2)

# Position: Z is up in Blender
car.location = (0, 0, 1.5)  # 1.5 units above ground
```

### Axis Mapping Summary

| Three.js | Blender (after glTF import) | Description |
|----------|----------------------------|-------------|
| X | X | Right |
| Y | Z | Up |
| Z | -Y | Forward (towards viewer) |
| rotation.y | rotation_euler.z | Yaw (turn left/right) |
| rotation.x | rotation_euler.x | Pitch (nose up/down) |
| rotation.z | rotation_euler.y | Roll (tilt left/right) |

### Why the Rotation?

The GLB model's forward direction doesn't match the physics engine's forward direction. The -90° rotation aligns the model so:
- Model's visual front → Physics forward direction
- Steering and movement work correctly

### Common Mistakes to Avoid

❌ **DON'T** forget the scale (2.5x)  
❌ **DON'T** forget the -90° rotation  
❌ **DON'T** apply rotation to Y-axis in Blender (use Z-axis!)  
❌ **DON'T** confuse Y-up (Three.js) with Z-up (Blender)  
✅ **DO** use `generate_scene()` which applies correct transforms automatically  
✅ **DO** verify orientation in both Blender and browser  
✅ **DO** remember: Three.js Y = Blender Z (up axis)

## File Structure

```
CarXSimulator/
├── preview/
│   ├── final-demo.html          # Main Three.js application
│   ├── lowpoly_game_model.glb   # Car model (loaded by Three.js)
│   ├── tree.glb                 # Tree model (optional)
│   └── scene_data.json          # Scene metadata
└── tools/
    ├── blender_scene_tools.py   # This script
    └── PIPELINE_GUIDE.md        # This file
```

## Pipeline Workflow

### 1. Import Existing Model (Recommended)

```bash
# Load existing car model from preview/
blender --python blender_scene_tools.py -- --clear --use-existing
```

This will:
- Clear the scene
- Load `preview/lowpoly_game_model.glb`
- Set up the car with correct orientation (no rotation)
- Name the root object "Car"

### 2. Edit in Blender

- Modify the car model as needed
- Add/remove parts
- Adjust materials
- Create animations

**Important**: Keep the root object named "Car" for proper export.

### 3. Export Back to CarX

```bash
# Export to preview/lowpoly_game_model.glb
blender --python blender_scene_tools.py -- --sync-to-carx
```

This will:
- Export the "Car" object and all children
- Save to `preview/lowpoly_game_model.glb`
- Generate `preview/scene_data.json`
- Use Y-up coordinate system (no rotation)

### 4. Verify Consistency

```bash
# Check if everything is correct
blender --python blender_scene_tools.py -- --verify
```

This will check:
- Car object exists
- Rotation is identity (or close to it)
- Position is reasonable
- Wheels are present
- Export path is correct

### 5. Test in Browser

Open `preview/final-demo.html` in a browser to see your changes.

## Common Workflows

### A. Quick Edit Workflow

```bash
# 1. Load existing model
blender --python blender_scene_tools.py

# 2. Edit in Blender UI (use CarX panel in sidebar)

# 3. Click "Sync to CarX" button in CarX panel

# 4. Refresh browser to see changes
```

### B. Command-Line Workflow

```bash
# Load, edit, and export in one command
blender --python blender_scene_tools.py -- \
  --clear \
  --use-existing \
  --sync-to-carx \
  --verify
```

### C. Create New Car from Scratch

```bash
# Generate procedural car
blender --python blender_scene_tools.py -- \
  --clear \
  --generate-car \
  --sync-to-carx
```

### D. Import External Model

```bash
# Import FBX/OBJ and convert to CarX format
blender --python blender_scene_tools.py -- \
  --clear \
  --import my_car.fbx \
  --update-scene \
  --sync-to-carx
```

## Physics Parameters

The car model should match these physics parameters from `final-demo.html`:

```javascript
wheelbase: 2.6,        // Distance between front and rear wheels (m)
track_width: 1.8,      // Distance between left and right wheels (m)
wheel_radius: 0.35,    // Wheel size (m)
ground_clearance: 0.5, // Distance from wheel contact to car center (m)
mass: 1200,            // Car mass (kg)
```

### Wheel Naming Convention

Wheels should be named:
- `wheel_FL` - Front Left
- `wheel_FR` - Front Right
- `wheel_RL` - Rear Left
- `wheel_RR` - Rear Right

The script will automatically detect and rename wheels if they contain keywords like "wheel", "tire", "front", "rear", "left", "right".

## Troubleshooting

### Car appears rotated/misaligned in browser

**Cause**: Missing or incorrect rotation transform  
**Fix**: The car needs -90° rotation on Z-axis in Blender (equivalent to Y-axis in Three.js)

```python
# In Blender Python console:
import bpy
import math
car = bpy.data.objects.get("Car")
# IMPORTANT: Use Z-axis in Blender (not Y!) because glTF import converts Y-up to Z-up
car.rotation_euler = (0, 0, -math.pi / 2)  # -90° on Z (yaw)
car.scale = (2.5, 2.5, 2.5)  # Also apply scale
car.location = (0, 0, 1.5)  # Z is up in Blender
```

Or use the `generate_scene()` function which applies correct transforms automatically.

### Car is too small/large

**Cause**: Scale mismatch  
**Fix**: Adjust scale in Blender before export, or use scale parameter:
```bash
blender --python blender_scene_tools.py -- \
  --import model.glb \
  --scale 2.0 \
  --sync-to-carx
```

### Car is underground/floating

**Cause**: Z position mismatch (Z is up in Blender after glTF import)  
**Fix**: The car's Z position in Blender should be around 0.6-1.5 (ground clearance + wheel radius)

```python
car.location = (0, 0, 1.0)  # Z is up in Blender
```

### Wheels not spinning

**Cause**: Wheel objects not properly named or parented  
**Fix**: Ensure wheels are:
1. Named with "wheel" in the name
2. Parented to the Car object
3. Have correct pivot point (center of wheel)

### Materials look different

**Cause**: Material settings not compatible  
**Fix**: Use PBR materials with:
- Base Color
- Metallic (0-1)
- Roughness (0-1)
- Emission (optional)

## Advanced Features

### Animation Export

```bash
# Create wheel spin animation and export
blender --python blender_scene_tools.py -- \
  --use-existing \
  --wheel-anim \
  --anim-duration 2.0 \
  --anim-rotations 4.0 \
  --export-anim animated_car.glb
```

### Batch Processing

```bash
# Process multiple models
for model in models/*.fbx; do
  blender --python blender_scene_tools.py -- \
    --clear \
    --import "$model" \
    --convert
done
```

### Custom Export Path

```python
# In Blender Python console:
import sys
sys.path.append('/path/to/CarXSimulator/tools')
from blender_scene_tools import sync_to_carx

sync_to_carx(output_dir="/custom/path")
```

## API Reference

### Python Functions

```python
# Import the module
import sys
sys.path.append('/path/to/CarXSimulator/tools')
from blender_scene_tools import *

# Load existing model
models = load_existing_models()
car = models.get('car')

# Generate new car
car = create_car_model()

# Export
export_car("output.glb")
sync_to_carx()

# Verify
results = verify_pipeline_consistency()
print(results)
```

### Blender UI Operators

Available in the CarX panel (press N in 3D View):

- **Generate CarX Scene** - Create full scene with terrain
- **Generate Car Only** - Create just the car model
- **Export GLB** - Export entire scene
- **Export Car GLB** - Export only the car
- **Import 3D File** - Import any supported format
- **Update Car from File** - Replace existing car
- **Convert to GLB** - Convert external file to GLB
- **Export with Animations** - Export with animations
- **Create Wheel Animation** - Generate wheel spin
- **Sync to CarX** - Export to preview/
- **Sync from CarX** - Import from preview/
- **Verify** - Check pipeline consistency

## Best Practices

1. **Always verify** after export: `--verify`
2. **Use existing models** when possible: `--use-existing`
3. **Keep backups** of working models
4. **Test in browser** after each export
5. **Use version control** (git) for models
6. **Document changes** in commit messages
7. **Check console output** for warnings
8. **Maintain naming conventions** for wheels and parts

## Performance Tips

1. **Optimize polygon count** - Keep under 50k triangles for web
2. **Use texture atlases** - Combine multiple textures
3. **Limit material count** - Fewer materials = better performance
4. **Compress textures** - Use appropriate resolution
5. **Remove hidden geometry** - Delete faces inside the model
6. **Use LOD models** - Create multiple detail levels

## Support

For issues or questions:
1. Check console output for errors
2. Run `--verify` to diagnose problems
3. Review this guide
4. Check the code comments in `blender_scene_tools.py`

## Version History

- **v2.0.0** - Fixed coordinate system (Y-up ↔ Y-up, no rotation)
- **v1.x** - Initial release with rotation (deprecated)

---

**Last Updated**: 2024-11-29  
**Blender Compatibility**: 2.80 - 5.x  
**Three.js Version**: r150+
