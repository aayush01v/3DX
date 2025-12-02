# CarX Blender Pipeline - Quick Reference

## 🚀 Quick Start

```bash
# Generate scene from unified definition
blender --python blender_scene_tools.py -- --generate --sync-to-carx

# Load existing car model
blender --python blender_scene_tools.py -- --clear --use-existing

# Edit in Blender, then export
blender --python blender_scene_tools.py -- --sync-to-carx --verify
```

## 🎯 Unified Scene System

**Single Source of Truth**: `CarXSimulator/scene_definition.json`

Both Blender and Three.js read from this file to ensure consistency!

## 📐 Coordinate System & Transforms

```
Three.js / glTF: Y-up, -Z forward, X-right
Blender (native): Z-up, -Y forward, X-right

After glTF import into Blender:
  Three.js Y → Blender Z (up)
  Three.js Z → Blender -Y (forward)

Required transforms (from final-demo.html):
  Scale:    2.5x (carMesh.scale.set(2.5, 2.5, 2.5))
  Rotation: -90° on Y (Three.js) = -90° on Z (Blender)
  Position: Y=1.5 (Three.js) = Z=1.5 (Blender)
```

The `generate_scene()` function applies these automatically!

## 📁 File Locations

```
preview/lowpoly_game_model.glb  ← Car model (Three.js loads this)
preview/tree.glb                ← Tree model (optional)
preview/scene_data.json         ← Metadata
```

## 🎯 Common Commands

### Load & Edit
```bash
# Interactive mode with UI
blender --python blender_scene_tools.py

# Load existing model (headless)
blender -b --python blender_scene_tools.py -- --clear --use-existing
```

### Export
```bash
# Export car to preview/
blender -b --python blender_scene_tools.py -- --sync-to-carx

# Export with verification
blender -b --python blender_scene_tools.py -- --sync-to-carx --verify
```

### Import External
```bash
# Import FBX/OBJ and convert
blender -b --python blender_scene_tools.py -- \
  --clear --import model.fbx --update-scene --sync-to-carx
```

### Generate New
```bash
# Create procedural car
blender -b --python blender_scene_tools.py -- \
  --clear --generate-car --sync-to-carx
```

### Verify
```bash
# Check consistency
blender -b --python blender_scene_tools.py -- --verify
```

## 🎨 Blender UI (Interactive Mode)

1. Open Blender with script: `blender --python blender_scene_tools.py`
2. Press `N` in 3D View to open sidebar
3. Find **CarX** tab
4. Use buttons:
   - **Generate Car Only** - Create new car
   - **Import 3D File** - Load external model
   - **Sync to CarX** - Export to preview/
   - **Verify** - Check consistency

## 🔧 Wheel Naming

```
wheel_FL  ← Front Left
wheel_FR  ← Front Right
wheel_RL  ← Rear Left
wheel_RR  ← Rear Right
```

Auto-detected keywords: `wheel`, `tire`, `front`, `rear`, `left`, `right`, `fl`, `fr`, `rl`, `rr`

## 📊 Physics Parameters

```javascript
wheelbase: 2.6 m        // Front-rear distance
track_width: 1.8 m      // Left-right distance
wheel_radius: 0.35 m    // Wheel size
ground_clearance: 0.5 m // Bottom clearance
mass: 1200 kg           // Car mass
```

## ⚠️ Common Issues

| Problem | Solution |
|---------|----------|
| Car rotated/misaligned | Apply -90° Z rotation + 2.5x scale (use `generate_scene()`) |
| Car too small/large | Scale should be 2.5x to match Three.js |
| Car underground | Set Z position to ~1.5 (Z is up in Blender!) |
| Wheels not spinning | Check naming and parenting |
| Materials wrong | Use PBR materials (Base Color, Metallic, Roughness) |
| Wrong rotation axis | Use Z-axis in Blender (not Y!) for yaw rotation |

## 🐛 Debugging

```bash
# Verbose output
blender --python blender_scene_tools.py -- --verbose --verify

# Check what's in the scene
blender --python blender_scene_tools.py -- --generate --export-json scene.json
```

## 📝 Checklist

Before exporting:
- [ ] Car object named "Car"
- [ ] Scale is (2.5, 2.5, 2.5)
- [ ] Rotation Z is -90° (-1.5708 rad) - NOT Y!
- [ ] Position Z is ~1.5 (Z is up in Blender)
- [ ] 4 wheels present and named
- [ ] Materials are PBR
- [ ] No hidden geometry
- [ ] Polygon count < 50k

After exporting:
- [ ] Run `--verify`
- [ ] Check file size (< 5 MB recommended)
- [ ] Test in browser (final-demo.html)
- [ ] Check console for errors

## 🔗 Workflow

```
┌─────────────────────────────┐
│   Blender (generate_scene)  │
│   Scale: 2.5x               │
│   Rotation Z: -90°          │
└──────────────┬──────────────┘
               │ Export GLB
               ↓
┌─────────────────────────────┐
│   lowpoly_game_model.glb    │
└──────────────┬──────────────┘
               │ Load
               ↓
┌─────────────────────────────┐
│   Three.js (final-demo.html)│
│   Scale: 2.5x               │
│   Rotation Y: -90°          │
└─────────────────────────────┘
```

## 💡 Pro Tips

1. Use `--use-existing` to load current model
2. Always run `--verify` after export
3. Keep backups of working models
4. Test in browser after each change
5. Use version control (git)
6. Check console for warnings

## 📚 Full Documentation

See `PIPELINE_GUIDE.md` for complete documentation.

---

**Quick Help**: `blender --python blender_scene_tools.py -- --help`
