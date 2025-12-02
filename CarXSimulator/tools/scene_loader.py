#!/usr/bin/env python3
"""
CarX Simulator - Universal Scene Loader
========================================
Cross-platform scene definition loader for Blender, Unreal, Unity, Android, etc.

This module reads scene_definition.json and provides a unified API
for creating scenes in any 3D engine.

Usage:
------
  from scene_loader import SceneDefinition, load_scene_definition
  
  scene = load_scene_definition("scene_definition.json")
  car_data = scene.get_car()
  terrain_data = scene.get_terrain()

Author: CarX Simulator Team
License: MIT
Version: 1.0.0
"""

import json
import os
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Any, Callable
from dataclasses import dataclass, field
from abc import ABC, abstractmethod


# ============================================================================
# DATA CLASSES
# ============================================================================

@dataclass
class Vector3:
    """Universal 3D vector."""
    x: float = 0.0
    y: float = 0.0
    z: float = 0.0
    
    @classmethod
    def from_list(cls, data: List[float]) -> 'Vector3':
        return cls(data[0] if len(data) > 0 else 0,
                   data[1] if len(data) > 1 else 0,
                   data[2] if len(data) > 2 else 0)
    
    def to_list(self) -> List[float]:
        return [self.x, self.y, self.z]
    
    def to_tuple(self) -> Tuple[float, float, float]:
        return (self.x, self.y, self.z)


@dataclass
class Transform:
    """Universal transform (position, rotation, scale)."""
    position: Vector3 = field(default_factory=Vector3)
    rotation: Vector3 = field(default_factory=Vector3)
    scale: Vector3 = field(default_factory=lambda: Vector3(1, 1, 1))
    
    @classmethod
    def from_dict(cls, data: Dict) -> 'Transform':
        return cls(
            position=Vector3.from_list(data.get('position', [0, 0, 0])),
            rotation=Vector3.from_list(data.get('rotation', [0, 0, 0])),
            scale=Vector3.from_list(data.get('scale', [1, 1, 1]))
        )


@dataclass
class Material:
    """Universal PBR material."""
    color: str = "#FFFFFF"
    metalness: float = 0.0
    roughness: float = 0.5
    emission: Optional[str] = None
    emission_intensity: float = 1.0
    
    @classmethod
    def from_dict(cls, data: Dict) -> 'Material':
        return cls(
            color=data.get('color', '#FFFFFF'),
            metalness=data.get('metalness', 0.0),
            roughness=data.get('roughness', 0.5),
            emission=data.get('emission'),
            emission_intensity=data.get('emission_intensity', 1.0)
        )
    
    def to_rgb(self) -> Tuple[float, float, float]:
        """Convert hex color to RGB (0-1 range)."""
        hex_color = self.color.lstrip('#')
        return tuple(int(hex_color[i:i+2], 16) / 255.0 for i in (0, 2, 4))


@dataclass
class Wheel:
    """Wheel definition."""
    name: str
    position: Vector3
    steering: bool = False
    drive: bool = False
    
    @classmethod
    def from_dict(cls, data: Dict) -> 'Wheel':
        return cls(
            name=data.get('name', 'wheel'),
            position=Vector3.from_list(data.get('position', [0, 0, 0])),
            steering=data.get('steering', False),
            drive=data.get('drive', False)
        )


@dataclass
class BodyPart:
    """Car body part definition."""
    name: str
    type: str  # 'box', 'cylinder', 'mesh'
    dimensions: Vector3
    position: Vector3
    rotation: Vector3 = field(default_factory=Vector3)
    material: Material = field(default_factory=Material)
    
    @classmethod
    def from_dict(cls, data: Dict) -> 'BodyPart':
        return cls(
            name=data.get('name', 'part'),
            type=data.get('type', 'box'),
            dimensions=Vector3.from_list(data.get('dimensions', [1, 1, 1])),
            position=Vector3.from_list(data.get('position', [0, 0, 0])),
            rotation=Vector3.from_list(data.get('rotation', [0, 0, 0])),
            material=Material.from_dict(data.get('material', {}))
        )


@dataclass
class Light:
    """Light definition (headlight/taillight)."""
    position: Vector3
    color: str = "#FFFFFF"
    intensity: float = 1.0
    
    @classmethod
    def from_dict(cls, data: Dict) -> 'Light':
        return cls(
            position=Vector3.from_list(data.get('position', [0, 0, 0])),
            color=data.get('color', '#FFFFFF'),
            intensity=data.get('intensity', 1.0)
        )


@dataclass
class CarPhysics:
    """Car physics parameters."""
    mass: float = 1200
    wheelbase: float = 2.6
    track_width: float = 1.8
    wheel_radius: float = 0.35
    ground_clearance: float = 0.5
    center_of_mass: Vector3 = field(default_factory=Vector3)
    
    @classmethod
    def from_dict(cls, data: Dict) -> 'CarPhysics':
        return cls(
            mass=data.get('mass', 1200),
            wheelbase=data.get('wheelbase', 2.6),
            track_width=data.get('track_width', 1.8),
            wheel_radius=data.get('wheel_radius', 0.35),
            ground_clearance=data.get('ground_clearance', 0.5),
            center_of_mass=Vector3.from_list(data.get('center_of_mass', [0, 0.4, 0]))
        )


@dataclass
class CarDefinition:
    """Complete car definition."""
    name: str
    transform: Transform
    physics: CarPhysics
    wheels: List[Wheel]
    body_parts: List[BodyPart]
    headlights: List[Light]
    taillights: List[Light]
    model_path: Optional[str] = None
    fallback: str = "procedural"
    
    @classmethod
    def from_dict(cls, data: Dict, assets: Dict = None) -> 'CarDefinition':
        lights = data.get('lights', {})
        asset_info = assets.get('car', {}) if assets else {}
        
        return cls(
            name=data.get('name', 'Car'),
            transform=Transform.from_dict(data.get('transform', {})),
            physics=CarPhysics.from_dict(data.get('physics', {})),
            wheels=[Wheel.from_dict(w) for w in data.get('wheels', [])],
            body_parts=[BodyPart.from_dict(p) for p in data.get('body_parts', [])],
            headlights=[Light.from_dict(l) for l in lights.get('headlights', [])],
            taillights=[Light.from_dict(l) for l in lights.get('taillights', [])],
            model_path=asset_info.get('model'),
            fallback=asset_info.get('fallback', 'procedural')
        )


@dataclass
class TerrainNoise:
    """Terrain noise parameters."""
    type: str = "perlin"
    octaves: int = 3
    frequency: List[float] = field(default_factory=lambda: [0.02, 0.015, 0.01])
    amplitude: List[float] = field(default_factory=lambda: [2, 1.5, 3])
    
    @classmethod
    def from_dict(cls, data: Dict) -> 'TerrainNoise':
        return cls(
            type=data.get('type', 'perlin'),
            octaves=data.get('octaves', 3),
            frequency=data.get('frequency', [0.02, 0.015, 0.01]),
            amplitude=data.get('amplitude', [2, 1.5, 3])
        )


@dataclass
class TerrainDefinition:
    """Terrain definition."""
    type: str = "procedural"
    size: int = 400
    segments: int = 120
    height_scale: float = 8
    noise: TerrainNoise = field(default_factory=TerrainNoise)
    material: Material = field(default_factory=Material)
    
    @classmethod
    def from_dict(cls, data: Dict) -> 'TerrainDefinition':
        return cls(
            type=data.get('type', 'procedural'),
            size=data.get('size', 400),
            segments=data.get('segments', 120),
            height_scale=data.get('height_scale', 8),
            noise=TerrainNoise.from_dict(data.get('noise', {})),
            material=Material.from_dict(data.get('material', {}))
        )
    
    def get_height(self, x: float, z: float) -> float:
        """Calculate terrain height at position using noise parameters."""
        import math
        h = 0
        for freq, amp in zip(self.noise.frequency, self.noise.amplitude):
            h += math.sin(x * freq) * amp
            h += math.cos(z * freq * 0.75) * amp * 0.75
        return h


@dataclass
class SunLight:
    """Sun/directional light definition."""
    position: Vector3
    rotation: Vector3
    color: str = "#FFF5E6"
    intensity: float = 1.5
    shadows: bool = True
    
    @classmethod
    def from_dict(cls, data: Dict) -> 'SunLight':
        return cls(
            position=Vector3.from_list(data.get('position', [50, 100, 50])),
            rotation=Vector3.from_list(data.get('rotation', [45, 30, 0])),
            color=data.get('color', '#FFF5E6'),
            intensity=data.get('intensity', 1.5),
            shadows=data.get('shadows', True)
        )


@dataclass
class EnvironmentDefinition:
    """Environment settings."""
    sky_top_color: str = "#87CEEB"
    sky_bottom_color: str = "#E0F7FA"
    sun: SunLight = field(default_factory=SunLight)
    ambient_color: str = "#FFFFFF"
    ambient_intensity: float = 0.4
    fog_enabled: bool = True
    fog_color: str = "#CCCCCC"
    fog_near: float = 100
    fog_far: float = 500
    
    @classmethod
    def from_dict(cls, data: Dict) -> 'EnvironmentDefinition':
        sky = data.get('sky', {})
        ambient = data.get('ambient', {})
        fog = data.get('fog', {})
        
        return cls(
            sky_top_color=sky.get('top_color', '#87CEEB'),
            sky_bottom_color=sky.get('bottom_color', '#E0F7FA'),
            sun=SunLight.from_dict(data.get('sun', {})),
            ambient_color=ambient.get('color', '#FFFFFF'),
            ambient_intensity=ambient.get('intensity', 0.4),
            fog_enabled=fog.get('enabled', True),
            fog_color=fog.get('color', '#CCCCCC'),
            fog_near=fog.get('near', 100),
            fog_far=fog.get('far', 500)
        )


@dataclass
class CameraDefinition:
    """Camera settings."""
    position: Vector3
    target: Vector3
    fov: float = 60
    near: float = 0.1
    far: float = 2000
    follow_distance: float = 25
    follow_height: float = 10
    follow_angle: float = 0
    follow_pitch: float = 0.4
    follow_smooth: float = 0.1
    
    @classmethod
    def from_dict(cls, data: Dict) -> 'CameraDefinition':
        default = data.get('default', {})
        follow = data.get('follow', {})
        
        return cls(
            position=Vector3.from_list(default.get('position', [15, 10, 15])),
            target=Vector3.from_list(default.get('target', [0, 2, 0])),
            fov=default.get('fov', 60),
            near=default.get('near', 0.1),
            far=default.get('far', 2000),
            follow_distance=follow.get('distance', 25),
            follow_height=follow.get('height', 10),
            follow_angle=follow.get('angle', 0),
            follow_pitch=follow.get('pitch', 0.4),
            follow_smooth=follow.get('smooth', 0.1)
        )


@dataclass
class VegetationDefinition:
    """Vegetation/trees settings."""
    enabled: bool = True
    tree_count: int = 50
    spawn_min: Tuple[float, float] = (-80, -80)
    spawn_max: Tuple[float, float] = (80, 80)
    exclusion_center: Tuple[float, float] = (0, 0)
    exclusion_radius_x: float = 10
    exclusion_radius_z: float = 30
    scale_range: Tuple[float, float] = (0.8, 1.2)
    model_path: Optional[str] = None
    
    @classmethod
    def from_dict(cls, data: Dict, assets: Dict = None) -> 'VegetationDefinition':
        trees = data.get('trees', {})
        spawn = trees.get('spawn_area', {})
        exclusion = trees.get('exclusion_zone', {})
        scale = trees.get('scale_range', [0.8, 1.2])
        asset_info = assets.get('tree', {}) if assets else {}
        
        return cls(
            enabled=data.get('enabled', True),
            tree_count=trees.get('count', 50),
            spawn_min=tuple(spawn.get('min', [-80, -80])),
            spawn_max=tuple(spawn.get('max', [80, 80])),
            exclusion_center=tuple(exclusion.get('center', [0, 0])),
            exclusion_radius_x=exclusion.get('radius_x', 10),
            exclusion_radius_z=exclusion.get('radius_z', 30),
            scale_range=tuple(scale),
            model_path=asset_info.get('model')
        )


# ============================================================================
# SCENE DEFINITION
# ============================================================================

class SceneDefinition:
    """
    Universal scene definition that can be used by any 3D engine.
    
    Provides a consistent API for accessing scene data regardless
    of the target platform (Blender, Three.js, Unreal, Unity, etc.)
    """
    
    def __init__(self, data: Dict, base_path: str = ""):
        self.raw_data = data
        self.base_path = base_path
        self.version = data.get('version', '1.0.0')
        self.metadata = data.get('metadata', {})
        
        # Coordinate system
        coord = data.get('coordinate_system', {})
        self.up_axis = coord.get('up', 'Y')
        self.forward_axis = coord.get('forward', '-Z')
        self.units = coord.get('units', 'meters')
        
        # Assets
        self.assets = data.get('assets', {})
        
        # Parse components
        self.car = CarDefinition.from_dict(data.get('car', {}), self.assets)
        self.terrain = TerrainDefinition.from_dict(data.get('terrain', {}))
        self.environment = EnvironmentDefinition.from_dict(data.get('environment', {}))
        self.camera = CameraDefinition.from_dict(data.get('camera', {}))
        self.vegetation = VegetationDefinition.from_dict(data.get('vegetation', {}), self.assets)
    
    def get_asset_path(self, asset_key: str) -> Optional[str]:
        """Get full path to an asset file."""
        asset = self.assets.get(asset_key, {})
        model_path = asset.get('model')
        if model_path and self.base_path:
            return os.path.join(self.base_path, model_path)
        return model_path
    
    def get_car(self) -> CarDefinition:
        """Get car definition."""
        return self.car
    
    def get_terrain(self) -> TerrainDefinition:
        """Get terrain definition."""
        return self.terrain
    
    def get_environment(self) -> EnvironmentDefinition:
        """Get environment definition."""
        return self.environment
    
    def get_camera(self) -> CameraDefinition:
        """Get camera definition."""
        return self.camera
    
    def get_vegetation(self) -> VegetationDefinition:
        """Get vegetation definition."""
        return self.vegetation
    
    def to_dict(self) -> Dict:
        """Export scene definition back to dictionary."""
        return self.raw_data
    
    def save(self, filepath: str):
        """Save scene definition to JSON file."""
        with open(filepath, 'w') as f:
            json.dump(self.raw_data, f, indent=2)
    
    @classmethod
    def load(cls, filepath: str) -> 'SceneDefinition':
        """Load scene definition from JSON file."""
        base_path = str(Path(filepath).parent)
        with open(filepath, 'r') as f:
            data = json.load(f)
        return cls(data, base_path)


def load_scene_definition(filepath: str = None) -> SceneDefinition:
    """
    Load scene definition from JSON file.
    
    If no filepath is provided, looks for scene_definition.json in:
    1. Current directory
    2. Parent directory
    3. CarXSimulator directory
    """
    if filepath and os.path.exists(filepath):
        return SceneDefinition.load(filepath)
    
    # Search for scene_definition.json
    search_paths = [
        "scene_definition.json",
        "../scene_definition.json",
        "CarXSimulator/scene_definition.json",
        os.path.join(os.path.dirname(__file__), "..", "scene_definition.json"),
    ]
    
    for path in search_paths:
        if os.path.exists(path):
            return SceneDefinition.load(path)
    
    raise FileNotFoundError("scene_definition.json not found")


# ============================================================================
# ABSTRACT SCENE BUILDER
# ============================================================================

class SceneBuilder(ABC):
    """
    Abstract base class for building scenes in different engines.
    
    Implement this for each target platform:
    - BlenderSceneBuilder
    - ThreeJSSceneBuilder
    - UnrealSceneBuilder
    - UnitySceneBuilder
    - AndroidSceneBuilder
    """
    
    def __init__(self, scene_def: SceneDefinition):
        self.scene = scene_def
    
    @abstractmethod
    def create_car(self) -> Any:
        """Create car in the target engine."""
        pass
    
    @abstractmethod
    def create_terrain(self) -> Any:
        """Create terrain in the target engine."""
        pass
    
    @abstractmethod
    def create_environment(self) -> Any:
        """Create environment (sky, lights, fog) in the target engine."""
        pass
    
    @abstractmethod
    def create_camera(self) -> Any:
        """Create camera in the target engine."""
        pass
    
    @abstractmethod
    def create_vegetation(self) -> Any:
        """Create vegetation in the target engine."""
        pass
    
    def build_scene(self) -> Any:
        """Build complete scene."""
        self.create_environment()
        self.create_terrain()
        self.create_vegetation()
        self.create_car()
        self.create_camera()
        return self.get_scene_root()
    
    @abstractmethod
    def get_scene_root(self) -> Any:
        """Get the root object/node of the scene."""
        pass
    
    @abstractmethod
    def export(self, filepath: str) -> bool:
        """Export scene to file."""
        pass


# ============================================================================
# UTILITY FUNCTIONS
# ============================================================================

def hex_to_rgb(hex_color: str) -> Tuple[float, float, float]:
    """Convert hex color to RGB (0-1 range)."""
    hex_color = hex_color.lstrip('#')
    return tuple(int(hex_color[i:i+2], 16) / 255.0 for i in (0, 2, 4))


def rgb_to_hex(r: float, g: float, b: float) -> str:
    """Convert RGB (0-1 range) to hex color."""
    return '#{:02x}{:02x}{:02x}'.format(int(r * 255), int(g * 255), int(b * 255))


def degrees_to_radians(degrees: float) -> float:
    """Convert degrees to radians."""
    import math
    return degrees * math.pi / 180


def radians_to_degrees(radians: float) -> float:
    """Convert radians to degrees."""
    import math
    return radians * 180 / math.pi


# ============================================================================
# MAIN
# ============================================================================

if __name__ == "__main__":
    # Test loading
    try:
        scene = load_scene_definition()
        print(f"Loaded scene: {scene.metadata.get('name', 'Unknown')}")
        print(f"Version: {scene.version}")
        print(f"Car: {scene.car.name}")
        print(f"  - Wheels: {len(scene.car.wheels)}")
        print(f"  - Body parts: {len(scene.car.body_parts)}")
        print(f"  - Physics mass: {scene.car.physics.mass} kg")
        print(f"Terrain: {scene.terrain.size}x{scene.terrain.size}")
        print(f"Trees: {scene.vegetation.tree_count}")
    except FileNotFoundError as e:
        print(f"Error: {e}")
