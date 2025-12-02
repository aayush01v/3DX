# CarX Simulator Documentation

## Documentation Structure

### Core Documentation

| Document | Description | Audience |
|----------|-------------|----------|
| [API.md](API.md) | API reference & quick start | Developers |
| [BUILDING.md](BUILDING.md) | Build instructions | Developers |
| [TECHNOLOGY.md](TECHNOLOGY.md) | Tech stack & modification guide | All |
| [PROJECT_ARCHITECTURE.md](PROJECT_ARCHITECTURE.md) | System architecture | Developers |

### 3D Pipeline & Scene System

| Document | Description | Audience |
|----------|-------------|----------|
| [MODEL_INTEGRATION.md](MODEL_INTEGRATION.md) | 3D models & Blender workflow | Artists/Devs |
| [UNIFIED_SCENE_SYSTEM.md](UNIFIED_SCENE_SYSTEM.md) | Cross-platform scene definition | Developers |
| [../tools/PIPELINE_GUIDE.md](../tools/PIPELINE_GUIDE.md) | Blender ↔ Three.js pipeline | Artists/Devs |
| [../tools/QUICK_REFERENCE.md](../tools/QUICK_REFERENCE.md) | Quick reference for Blender tools | Artists |

### Physics & Simulation

| Document | Description | Audience |
|----------|-------------|----------|
| [ADVANCED_VEHICLE_DYNAMICS.md](ADVANCED_VEHICLE_DYNAMICS.md) | Professional physics (Pacejka, load transfer) | Advanced |
| [PHYSICS_COMPONENTS_AND_EXPERIMENTS.md](PHYSICS_COMPONENTS_AND_EXPERIMENTS.md) | Interactive code experiments | Education |
| [DEBUGGING_ANALYSIS.md](DEBUGGING_ANALYSIS.md) | Performance analysis | Developers |

## Quick Links

### Getting Started
1. **Run the demo**: See main [README.md](../README.md)
2. **Build from source**: [BUILDING.md](BUILDING.md)
3. **API reference**: [API.md](API.md)

### For Artists
- **3D Models**: [MODEL_INTEGRATION.md](MODEL_INTEGRATION.md)
- **Blender Pipeline**: [../tools/PIPELINE_GUIDE.md](../tools/PIPELINE_GUIDE.md)
- **Quick Reference**: [../tools/QUICK_REFERENCE.md](../tools/QUICK_REFERENCE.md)

### For Developers
- **Architecture**: [PROJECT_ARCHITECTURE.md](PROJECT_ARCHITECTURE.md)
- **Technology Stack**: [TECHNOLOGY.md](TECHNOLOGY.md)
- **Scene System**: [UNIFIED_SCENE_SYSTEM.md](UNIFIED_SCENE_SYSTEM.md)
- **Performance**: [DEBUGGING_ANALYSIS.md](DEBUGGING_ANALYSIS.md)

### Physics Deep Dive
- **Vehicle Dynamics**: [ADVANCED_VEHICLE_DYNAMICS.md](ADVANCED_VEHICLE_DYNAMICS.md)
- **Experiments**: [PHYSICS_COMPONENTS_AND_EXPERIMENTS.md](PHYSICS_COMPONENTS_AND_EXPERIMENTS.md)

## Other Documentation

| Location | Content |
|----------|---------|
| [../README.md](../README.md) | Project overview |
| [../CHANGELOG.md](../CHANGELOG.md) | Version history |
| [../DEPLOYMENT.md](../DEPLOYMENT.md) | Deployment guide |
| [../BUILD_APK.md](../BUILD_APK.md) | Android build |
| [../scene_definition.json](../scene_definition.json) | Unified scene definition |

## Key Concepts

### Coordinate System
Both Blender and Three.js use **Y-up** coordinate system:
- **X**: Right
- **Y**: Up  
- **Z**: Forward (negative in Three.js physics)

### Model Transforms
The GLB car model requires specific transforms to match `final-demo.html`:
- **Scale**: 2.5x
- **Rotation**: -90° on Y-axis (Three.js) / Z-axis (Blender)

See [PIPELINE_GUIDE.md](../tools/PIPELINE_GUIDE.md) for details.
