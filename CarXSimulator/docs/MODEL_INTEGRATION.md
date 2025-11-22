# 3D Model Integration Guide

## Using the Low-Poly Car Model

The simulator now supports loading the `lowpoly_game_model.glb` file for realistic car visualization.

### Preview (Web)

The preview automatically loads the GLB model using Three.js:

```javascript
// Automatically loaded in preview/demo.html
loader.load('../lowpoly_game_model.glb', (gltf) => {
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
- Format: glTF 2.0 (.glb)
- Include: Meshes, Materials
- Transform: +Y Up
- Compression: Draco (optional)

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

## Resources

- [glTF Format Specification](https://www.khronos.org/gltf/)
- [tinygltf Library](https://github.com/syoyo/tinygltf)
- [Blender glTF Exporter](https://docs.blender.org/manual/en/latest/addons/import_export/scene_gltf2.html)
- [Three.js GLTFLoader](https://threejs.org/docs/#examples/en/loaders/GLTFLoader)
