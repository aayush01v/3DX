# 3D Model Integration Guide

## Overview

CarX Simulator supports both procedural 3D models and imported GLB assets. This guide covers:
- Loading GLB models in the web preview
- Blender workflow for creating/editing models
- C++ engine integration
- Model specifications and best practices

> 📖 **See also:** [TECHNOLOGY.md](TECHNOLOGY.md) for complete technology stack documentation.

## Blender Integration (v2.0)

We provide a robust Python script for seamless Blender ↔ CarX workflow with:
- ✅ Blender 2.80 - 4.x compatibility (auto-detects API changes)
- ✅ Crash-safe export with fallback modes
- ✅ Interactive UI panel
- ✅ Bidirectional sync pipeline

### Quick Start

```bash
# Generate the full CarX scene in Blender
blender --python tools/blender_scene_tools.py -- --generate

# Generate and export to GLB
blender -b --python tools/blender_scene_tools.py -- --generate --export scene.glb

# Export just the car model
blender -b --python tools/blender_scene_tools.py -- --generate-car --export-car car.glb

# Import an existing GLB into Blender
blender --python tools/blender_scene_tools.py -- --import my_model.glb

# Sync workflow (export to CarX preview/assets)
blender -b --python tools/blender_scene_tools.py -- --generate --sync-to-carx
```

### Interactive Mode

Open Blender with the CarX panel:
```bash
blender --python tools/blender_scene_tools.py
```

This adds a "CarX" tab in the 3D View sidebar (press N) with:
- **Generate CarX Scene** - Creates car, terrain, trees with options
- **Generate Car Only** - Just the car model
- **Export GLB** - Export entire scene
- **Export Car GLB** - Export car only
- **Import GLB** - Import external models
- **Sync to CarX** - Push to preview/assets
- **Sync from CarX** - Pull from preview/assets

### Script Location

```
tools/
├── blender_scene_tools.py   # Main script (v2.0)
└── README.md                # Detailed documentation
```

### Supported Blender Versions

| Version | Status |
|---------|--------|
| 2.80 - 2.93 | ✅ Full support |
| 3.0 - 3.6 | ✅ Full support |
| 4.0 - 4.x | ✅ Full support (handles API changes) |

## Using the Low-Poly Car Model

The simulator supports loading GLB models for realistic car visualization.

### Preview (Web)

The preview automatically loads GLB models using Three.js:

```javascript
// Automatically loaded in preview/final-demo.html
loader.load('assets/lowpoly_game_model.glb', (gltf) => {
    this.carModel = gltf.scene;
    this.carModel.scale.set(1.5, 1.5, 1.5);
    this.scene.add(this.carModel);
});
```

### C++ Engine

For the full C++ engine, you can integrate GLB loading using tinygltf:

#### 1. Add tinygltf to your project

```cmake
# In CMakeLists.txt
include(FetchContent)
FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG v2.8.0
)
FetchContent_MakeAvailable(tinygltf)

target_link_libraries(CarXSimulator PRIVATE tinygltf)
```

#### 2. Load the model

```cpp
#include "ModelLoader.h"

// In Renderer initialization
carModel = ModelLoader::loadGLB("lowpoly_game_model.glb");

// Render the model
void Renderer::renderCar(const Car* car) {
    if (!carModel) return;
    
    Matrix4 modelMatrix = Matrix4::translate(car->getPosition());
    modelMatrix = modelMatrix * Matrix4::rotate(car->getRotation());
    
    carShader->use();
    carShader->setMat4("model", modelMatrix.m);
    
    for (const auto& mesh : carModel->meshes) {
        mesh->draw();
    }
}
```

## Model Specifications

### Low-Poly Car Model
- **Format**: GLB (GLTF Binary)
- **Triangles**: ~500-1000 (optimized for real-time)
- **Scale**: Adjust to match physics (typically 1.5x)
- **Pivot**: Center bottom (for proper ground placement)

### Coordinate System
- **X**: Right
- **Y**: Up
- **Z**: Forward

### Physics Alignment

The car model should align with the physics simulation:

```cpp
// Car dimensions from model
float modelLength = 4.0f;  // meters
float modelWidth = 1.8f;   // meters
float modelHeight = 1.5f;  // meters

// Match physics wheelbase
car->setWheelbase(2.5f);  // Distance between front and rear axles
```

## Alternative Models

You can use other 3D models by:

1. **Converting to GLB**: Use Blender to export as GLB
2. **Adjusting scale**: Modify the scale factor in code
3. **Updating physics**: Match wheelbase and dimensions

### Blender Export Settings

Using the CarX Blender tools (recommended):
```bash
blender my_scene.blend --python tools/blender_scene_tools.py -- --export my_model.glb
```

Manual export settings:
- Format: glTF 2.0 (.glb)
- Include: Meshes, Materials
- Transform: +Y Up
- Apply Modifiers: Yes
- Compression: Draco (optional, reduces file size)

### Required Transforms

**Important**: The GLB model requires specific transforms to match `final-demo.html`:

| Property | Three.js | Blender |
|----------|----------|---------|
| Scale | `2.5, 2.5, 2.5` | `(2.5, 2.5, 2.5)` |
| Rotation | `rotation.y = -π/2` | `rotation_euler.z = -π/2` |

The `generate_scene()` function in `blender_scene_tools.py` applies these automatically.

See [PIPELINE_GUIDE.md](../tools/PIPELINE_GUIDE.md) for detailed transform documentation.

### Blender Import Settings

To edit an existing GLB model:
```bash
blender --python tools/blender_scene_tools.py -- --import existing_model.glb
```

Or manually: File → Import → glTF 2.0 (.glb/.gltf)

## Fallback Rendering

If the model fails to load, the engine uses a simple box representation:

```cpp
void Renderer::createFallbackCar() {
    // Simple box geometry
    carMesh = Mesh::createCube();
}
```

## Performance Tips

1. **LOD (Level of Detail)**: Use lower poly models for distant cars
2. **Instancing**: Render multiple cars efficiently
3. **Culling**: Don't render off-screen cars
4. **Texture Atlas**: Combine textures to reduce draw calls

## Troubleshooting

### Model not loading?
- Check file path is correct
- Verify GLB file is valid (test in Blender)
- Check console for error messages

### Model too big/small?
- Adjust scale in code: `model->scale.set(1.5, 1.5, 1.5)`
- Or re-export from Blender with correct scale

### Model not aligned?
- Check pivot point in Blender
- Adjust rotation offset in code
- Verify coordinate system matches

### Performance issues?
- Reduce polygon count in Blender
- Enable backface culling
- Use simpler materials
- Consider LOD system

## Example: Custom Car Model

```cpp
// Load custom model
auto customCar = ModelLoader::loadGLB("my_custom_car.glb");

// Scale to match physics
float scale = 2.0f;
customCar->scale = Vector3(scale, scale, scale);

// Adjust physics to match model
car->setWheelbase(3.0f);  // Longer wheelbase
car->setMass(1800.0f);    // Heavier car
```

## Blender Workflow Examples

### Creating a Custom Car

1. Generate the base car in Blender:
   ```bash
   blender --python tools/blender_scene_tools.py -- --generate-car
   ```

2. Modify the car in Blender (change colors, add details, etc.)

3. Export the modified car:
   ```bash
   # From Blender's Python console or script
   import bpy
   bpy.ops.export_scene.gltf(filepath="my_custom_car.glb", use_selection=True)
   ```

4. Update `final-demo.html` to load your model:
   ```javascript
   loader.load('assets/my_custom_car.glb', (gltf) => { ... });
   ```

### Batch Processing

Export multiple variations:
```bash
# Generate and export with different settings
for style in sport racing offroad; do
    blender --background --python tools/blender_scene_tools.py -- \
        --generate-car --export-car "car_${style}.glb"
done
```

### Scene Composition

Create a complete scene for screenshots/renders:
```bash
blender --python tools/blender_scene_tools.py -- --generate
# Then render in Blender with Cycles/Eevee
```

## Resources

- [glTF Format Specification](https://www.khronos.org/gltf/)
- [tinygltf Library](https://github.com/syoyo/tinygltf)
- [Blender glTF Exporter](https://docs.blender.org/manual/en/latest/addons/import_export/scene_gltf2.html)
- [Three.js GLTFLoader](https://threejs.org/docs/#examples/en/loaders/GLTFLoader)
- CarX Blender Tools: `tools/blender_scene_tools.py`
