#!/usr/bin/env python3
"""
CarX Simulator - Blender Scene Tools v2.0
==========================================
Robust import/export pipeline between CarX Simulator (Three.js) and Blender.

Compatibility: Blender 2.80 - 5.x (auto-detects API changes)

Usage:
------
  blender --background --python blender_scene_tools.py -- --help
  blender --python blender_scene_tools.py -- --generate --export scene.glb
  blender --python blender_scene_tools.py -- --import model.glb --update-scene

Author: CarX Simulator Team
License: MIT
Version: 2.0.0
"""

import sys
import os
import json
import math
import traceback
from pathlib import Path
from typing import Optional, Dict, List, Tuple, Any, Callable

# ============================================================================
# BLENDER VERSION DETECTION & COMPATIBILITY
# ============================================================================

IN_BLENDER = False
BLENDER_VERSION = (0, 0, 0)
API_COMPAT = {}

try:
    import bpy
    from mathutils import Vector, Matrix, Euler
    IN_BLENDER = True
    BLENDER_VERSION = bpy.app.version
    
    # API compatibility flags
    API_COMPAT = {
        "use_new_principled": BLENDER_VERSION >= (4, 0, 0),  # Blender 4.0 changed Principled BSDF
        "use_new_gltf": BLENDER_VERSION >= (3, 6, 0),        # glTF exporter changes
        "has_asset_browser": BLENDER_VERSION >= (3, 0, 0),
        "emission_input_name": "Emission Color" if BLENDER_VERSION >= (4, 0, 0) else "Emission",
        "has_calc_normals": BLENDER_VERSION < (4, 1, 0),     # calc_normals removed in 4.1+
        "is_blender_5": BLENDER_VERSION >= (5, 0, 0),        # Blender 5.0+ specific changes
    }
    
    # Auto-enable glTF addon if not already enabled
    if not hasattr(bpy.ops.import_scene, 'gltf'):
        try:
            bpy.ops.preferences.addon_enable(module='io_scene_gltf2')
            print("✓ Auto-enabled glTF 2.0 addon")
        except Exception as e:
            print(f"⚠ Could not auto-enable glTF addon: {e}")
            print("  Enable it manually: Edit > Preferences > Add-ons > search 'gltf'")
            
except ImportError:
    pass


# ============================================================================
# CONFIGURATION
# ============================================================================

# Try to determine absolute path to this script file for reliable path resolution
_THIS_FILE_DIR = None
try:
    _THIS_FILE_DIR = Path(__file__).resolve().parent
except (NameError, OSError):
    pass

CONFIG = {
    "version": "2.0.0",
    "preview_dir": "../preview",
    "assets_dir": "../preview/assets",
    "export_dir": "../preview/assets/models",
    
    # Absolute path hint - set this if auto-detection fails
    # This is resolved at module load time from __file__
    "absolute_preview_dir": str(_THIS_FILE_DIR.parent / "preview") if _THIS_FILE_DIR else None,
    
    # Three.js coordinate system
    "coordinate_system": {"up": "Y", "forward": "-Z"},
    
    # Car dimensions (matches final-demo.html)
    "car": {
        "wheelbase": 2.6,
        "track_width": 1.8,
        "wheel_radius": 0.4,
        "wheel_width": 0.3,
        "body_length": 4.0,
        "body_width": 2.0,
        "body_height": 0.8,
    },
    
    # Available car models with metadata
    "car_models": {
        "sports_car": {
            "filename": "lowpoly_game_model.glb",
            "name": "Sports Car",
            "scale": 2.5,
            "rotation_offset": -1.5708,  # -90 degrees in radians
        },
        "bmw_e30": {
            "filename": "bmw_e30_1985.glb",
            "name": "BMW E30 (1985)",
            "scale": 1.0,
            "rotation_offset": 0,
        },
        "chevy_c10": {
            "filename": "chevrolet_c10_pickup_1963.glb",
            "name": "Chevrolet C10 (1963)",
            "scale": 1.0,
            "rotation_offset": 0,
        },
    },
    
    # Default car model to load
    "default_car": "sports_car",
    
    # Supported car model filenames (checked in order) - legacy fallback
    "car_model_names": [
        "lowpoly_game_model.glb",
        "bmw_e30_1985.glb",
        "chevrolet_c10_pickup_1963.glb",
        "car_model.glb",
        "car.glb",
        "vehicle.glb",
    ],
    
    # Supported HDR environment filenames
    "hdr_names": [
        "farmland_overcast_1k.exr",
        "environment.exr",
        "hdri.exr",
    ],
}

# ============================================================================
# UNIFIED SCENE DEFINITION LOADER
# ============================================================================

# Try to load from scene_definition.json for cross-platform consistency
SCENE_DEFINITION = None
_scene_def_instance = None

def _load_scene_definition():
    """Load unified scene definition from JSON."""
    global SCENE_DEFINITION, _scene_def_instance
    
    if SCENE_DEFINITION is not None:
        return SCENE_DEFINITION
    
    # Initialize data variable
    data = None
    
    # Try to import scene_loader module
    try:
        from scene_loader import load_scene_definition, SceneDefinition
        _scene_def_instance = load_scene_definition()
        log.info(f"Loaded unified scene definition v{_scene_def_instance.version}")
    except (ImportError, FileNotFoundError):
        _scene_def_instance = None
        # Try direct JSON load
        try:
            scene_json_paths = [
                get_script_dir().parent / "scene_definition.json",
                get_script_dir() / "scene_definition.json",
                Path.cwd() / "CarXSimulator" / "scene_definition.json",
                Path.cwd() / "scene_definition.json",
            ]
            
            for path in scene_json_paths:
                if path.exists():
                    with open(path, 'r') as f:
                        data = json.load(f)
                    log.info(f"Loaded scene definition from: {path}")
                    break
        except Exception as e:
            log.warn(f"Could not load scene_definition.json: {e}")
            data = None
    
    # Fallback to hardcoded definition
    if _scene_def_instance is not None:
        # Convert SceneDefinition to dict format for backward compatibility
        SCENE_DEFINITION = _convert_scene_def_to_dict(_scene_def_instance)
    elif data is not None:
        SCENE_DEFINITION = _convert_json_to_legacy(data)
    else:
        log.warn("Using fallback scene definition")
        SCENE_DEFINITION = _get_fallback_scene_definition()
    
    return SCENE_DEFINITION


def _get_fallback_scene_definition() -> Dict:
    """Fallback scene definition if JSON not found."""
    return {
        "car": {
            "body": {"dimensions": [2.0, 0.8, 4.0], "position": [0, 0.6, 0], 
                     "color": [1.0, 0.42, 0.21], "metalness": 0.7, "roughness": 0.3},
            "cabin": {"dimensions": [1.6, 0.7, 2.0], "position": [0, 1.15, -0.3],
                      "color": [0.17, 0.24, 0.31], "metalness": 0.8, "roughness": 0.2},
            "hood": {"dimensions": [1.8, 0.3, 1.5], "position": [0, 0.85, 1.25],
                     "color": [1.0, 0.42, 0.21], "metalness": 0.7, "roughness": 0.3},
            "spoiler": {"dimensions": [1.8, 0.1, 0.6], "position": [0, 1.3, -1.8],
                        "rotation": [-0.2, 0, 0], "color": [1.0, 0.42, 0.21], "metalness": 0.7, "roughness": 0.3},
            "wheels": [
                {"name": "wheel_FL", "position": [-0.9, 0.35, 1.3]},
                {"name": "wheel_FR", "position": [0.9, 0.35, 1.3]},
                {"name": "wheel_RL", "position": [-0.9, 0.35, -1.3]},
                {"name": "wheel_RR", "position": [0.9, 0.35, -1.3]},
            ],
            "headlights": [{"position": [-0.7, 0.7, 2.05]}, {"position": [0.7, 0.7, 2.05]}],
            "taillights": [{"position": [-0.7, 0.7, -2.05]}, {"position": [0.7, 0.7, -2.05]}],
        },
        "terrain": {"size": 400, "segments": 120, "height_scale": 8},
        "physics": {
            "wheelbase": 2.6,
            "track_width": 1.8,
            "wheel_radius": 0.35,
            "ground_clearance": 0.5,
            "mass": 1200,
        },
        "environment": {
            "sun": {"position": [50, 100, 50], "rotation": [45, 30, 0], "intensity": 1.5},
        },
        "camera": {
            "position": [15, 10, 15],
            "target": [0, 2, 0],
        }
    }


def _hex_to_rgb(hex_color: str) -> Tuple[float, float, float]:
    """Convert hex color to RGB (0-1 range)."""
    hex_color = hex_color.lstrip('#')
    return tuple(int(hex_color[i:i+2], 16) / 255.0 for i in (0, 2, 4))


def _convert_scene_def_to_dict(scene_def) -> Dict:
    """Convert SceneDefinition object to legacy dict format."""
    try:
        car = scene_def.car
        terrain = scene_def.terrain
        env = scene_def.environment
        cam = scene_def.camera
        
        # Convert body parts
        body_parts = {}
        for part in car.body_parts:
            body_parts[part.name] = {
                "dimensions": part.dimensions.to_list(),
                "position": part.position.to_list(),
                "rotation": part.rotation.to_list() if part.rotation else None,
                "color": list(_hex_to_rgb(part.material.color)),
                "metalness": part.material.metalness,
                "roughness": part.material.roughness,
            }
        
        return {
            "car": {
                **body_parts,
                "wheels": [{"name": w.name, "position": w.position.to_list()} for w in car.wheels],
                "headlights": [{"position": l.position.to_list()} for l in car.headlights],
                "taillights": [{"position": l.position.to_list()} for l in car.taillights],
            },
            "terrain": {
                "size": terrain.size,
                "segments": terrain.segments,
                "height_scale": terrain.height_scale,
                "noise": {
                    "frequency": terrain.noise.frequency,
                    "amplitude": terrain.noise.amplitude,
                }
            },
            "physics": {
                "wheelbase": car.physics.wheelbase,
                "track_width": car.physics.track_width,
                "wheel_radius": car.physics.wheel_radius,
                "ground_clearance": car.physics.ground_clearance,
                "mass": car.physics.mass,
            },
            "environment": {
                "sun": {
                    "position": env.sun.position.to_list(),
                    "rotation": env.sun.rotation.to_list(),
                    "intensity": env.sun.intensity,
                    "color": _hex_to_rgb(env.sun.color),
                }
            },
            "camera": {
                "position": cam.position.to_list(),
                "target": cam.target.to_list(),
            },
            "vegetation": {
                "tree_count": scene_def.vegetation.tree_count,
                "spawn_min": scene_def.vegetation.spawn_min,
                "spawn_max": scene_def.vegetation.spawn_max,
            }
        }
    except Exception as e:
        log.warn(f"Error converting scene definition: {e}")
        return _get_fallback_scene_definition()


def _convert_json_to_legacy(data: Dict) -> Dict:
    """Convert JSON scene definition to legacy format."""
    car_data = data.get('car', {})
    
    # Convert body_parts array to dict
    body_parts = {}
    for part in car_data.get('body_parts', []):
        name = part.get('name', 'part')
        mat = part.get('material', {})
        body_parts[name] = {
            "dimensions": part.get('dimensions', [1, 1, 1]),
            "position": part.get('position', [0, 0, 0]),
            "rotation": part.get('rotation'),
            "color": list(_hex_to_rgb(mat.get('color', '#FFFFFF'))),
            "metalness": mat.get('metalness', 0.5),
            "roughness": mat.get('roughness', 0.5),
        }
    
    return {
        "car": {
            **body_parts,
            "wheels": car_data.get('wheels', []),
            "headlights": [{"position": l.get('position', [0, 0, 0])} 
                          for l in car_data.get('lights', {}).get('headlights', [])],
            "taillights": [{"position": l.get('position', [0, 0, 0])} 
                          for l in car_data.get('lights', {}).get('taillights', [])],
        },
        "terrain": data.get('terrain', {}),
        "physics": car_data.get('physics', {}),
        "environment": data.get('environment', {}),
        "camera": data.get('camera', {}).get('default', {}),
        "vegetation": data.get('vegetation', {}),
    }


def get_scene_definition() -> Dict:
    """Get the current scene definition (loads if needed)."""
    global SCENE_DEFINITION
    if SCENE_DEFINITION is None:
        _load_scene_definition()
    return SCENE_DEFINITION

# ============================================================================
# PATH UTILITIES
# ============================================================================

def get_script_dir() -> Path:
    """
    Get the script directory, handling various execution contexts.
    Works when run from command line, console, or text editor.
    
    Returns the directory containing this script (blender_scene_tools.py).
    """
    # Method 1: Try __file__ (works when run as external script)
    try:
        script_path = Path(__file__).resolve()
        if script_path.exists() and script_path.name == "blender_scene_tools.py":
            log.debug(f"Script dir from __file__: {script_path.parent}")
            return script_path.parent
    except (NameError, OSError) as e:
        log.debug(f"__file__ not available: {e}")
    
    # Method 2: Check Blender's text editor (if script is loaded there)
    if IN_BLENDER:
        try:
            for text in bpy.data.texts:
                if 'blender_scene_tools' in text.name.lower():
                    # Script is in text editor, try to find external path
                    if text.filepath:
                        log.debug(f"Script dir from text editor: {Path(text.filepath).parent}")
                        return Path(text.filepath).parent
        except:
            pass
        
        # Method 3: If a .blend file is open, look for tools/ relative to it
        if bpy.data.filepath:
            blend_dir = Path(bpy.data.filepath).parent
            candidates = [
                blend_dir / "tools",
                blend_dir.parent / "tools",
                blend_dir / "CarXSimulator" / "tools",
            ]
            for candidate in candidates:
                if candidate.exists() and (candidate / "blender_scene_tools.py").exists():
                    log.debug(f"Script dir from blend file: {candidate}")
                    return candidate
    
    # Method 4: Search from current working directory
    cwd = Path.cwd()
    search_paths = [
        cwd / "CarXSimulator" / "tools",
        cwd / "tools",
        cwd.parent / "CarXSimulator" / "tools",
        cwd.parent / "tools",
        cwd,
    ]
    
    for path in search_paths:
        try:
            if path.exists() and path.is_dir():
                # Verify this is the right tools directory
                if (path / "blender_scene_tools.py").exists():
                    log.debug(f"Script dir from CWD search: {path}")
                    return path
                # Or if it has scene_loader.py (another tools file)
                if (path / "scene_loader.py").exists():
                    log.debug(f"Script dir from CWD search (scene_loader): {path}")
                    return path
        except (OSError, PermissionError):
            continue
    
    # Last resort: return cwd
    log.warn(f"Could not determine script directory, using CWD: {cwd}")
    return cwd

def get_preview_dir() -> Path:
    """
    Get the preview directory path.
    Searches multiple locations to find the preview folder with assets.
    
    Priority order:
    0. Absolute path from CONFIG (set at module load from __file__)
    1. Explicit CONFIG['preview_dir'] resolved from script directory
    2. Relative to .blend file if open
    3. Relative to script location (tools/../preview)
    4. Relative to current working directory
    5. Absolute paths for common project structures
    """
    script_dir = get_script_dir()
    cwd = Path.cwd()
    
    log.debug(f"get_preview_dir: script_dir={script_dir}, cwd={cwd}")
    
    # List of possible preview directory locations
    candidates = []
    
    # PRIORITY 0: Absolute path captured at module load time (most reliable)
    if CONFIG.get("absolute_preview_dir"):
        abs_preview = Path(CONFIG["absolute_preview_dir"])
        candidates.append(abs_preview)
        log.debug(f"  Absolute preview dir from CONFIG: {abs_preview}")
    
    # PRIORITY 1: Explicit CONFIG path (resolved from script dir)
    config_preview = (script_dir / CONFIG["preview_dir"]).resolve()
    candidates.append(config_preview)
    
    # PRIORITY 2: If running in Blender with an open file
    if IN_BLENDER:
        try:
            if bpy.data.filepath:
                blend_dir = Path(bpy.data.filepath).parent
                candidates.extend([
                    blend_dir / "preview",
                    blend_dir.parent / "preview",
                    blend_dir / ".." / "preview",
                    blend_dir / "CarXSimulator" / "preview",
                ])
        except:
            pass
    
    # PRIORITY 3: Relative to script location
    candidates.extend([
        script_dir.parent / "preview",  # tools/../preview (most common)
        script_dir / "preview",          # ./preview
        script_dir / ".." / "preview",   # Alternative relative path
        script_dir / "CarXSimulator" / "preview",
    ])
    
    # PRIORITY 4: Relative to current working directory
    candidates.extend([
        cwd / "CarXSimulator" / "preview",
        cwd / "preview",
        cwd.parent / "CarXSimulator" / "preview",
        cwd.parent / "preview",
    ])
    
    # PRIORITY 5: Check sys.argv for script path hints
    for arg in sys.argv:
        if 'blender_scene_tools' in arg or 'CarXSimulator' in arg:
            try:
                arg_path = Path(arg).resolve()
                if arg_path.is_file():
                    arg_path = arg_path.parent
                for parent in [arg_path, arg_path.parent, arg_path.parent.parent]:
                    candidates.append(parent / "preview")
                    candidates.append(parent / "CarXSimulator" / "preview")
            except:
                pass
    
    # Resolve all paths to absolute and remove duplicates (preserving order)
    resolved_candidates = []
    seen = set()
    for candidate in candidates:
        try:
            resolved = candidate.resolve()
            if resolved not in seen:
                seen.add(resolved)
                resolved_candidates.append(resolved)
        except (OSError, RuntimeError):
            continue
    
    log.debug(f"Preview dir candidates: {len(resolved_candidates)}")
    
    # Get list of car model names to search for
    car_model_names = CONFIG.get("car_model_names", ["lowpoly_game_model.glb"])
    hdr_names = CONFIG.get("hdr_names", ["farmland_overcast_1k.exr"])
    
    # First pass: find directory with any supported car GLB
    for candidate in resolved_candidates:
        try:
            if candidate.exists():
                for glb_name in car_model_names:
                    if (candidate / glb_name).exists():
                        log.info(f"✓ Found preview dir with GLB: {candidate}")
                        log.info(f"  Car model: {glb_name}")
                        return candidate
        except (OSError, PermissionError):
            continue
    
    # Second pass: find directory with any supported HDR
    for candidate in resolved_candidates:
        try:
            if candidate.exists():
                for hdr_name in hdr_names:
                    if (candidate / hdr_name).exists():
                        log.info(f"✓ Found preview dir with HDR: {candidate}")
                        log.info(f"  HDR file: {hdr_name}")
                        return candidate
        except (OSError, PermissionError):
            continue
    
    # Third pass: find any existing preview directory
    for candidate in resolved_candidates:
        try:
            if candidate.exists() and candidate.is_dir():
                log.warn(f"Preview dir found but may be missing assets: {candidate}")
                try:
                    files = list(candidate.glob("*"))
                    if files:
                        log.info(f"  Files found: {[f.name for f in files[:10]]}")
                except:
                    pass
                return candidate
        except (OSError, PermissionError):
            continue
    
    # Last resort - print ALL candidates with their exists() state
    default = script_dir.parent / "preview"
    log.error(f"Preview dir not found!")
    log.error(f"  Script dir: {script_dir}")
    log.error(f"  CWD: {cwd}")
    log.error(f"  CONFIG preview_dir: {CONFIG['preview_dir']} -> {config_preview}")
    log.error(f"  Searched for car models: {car_model_names}")
    log.error(f"  Searched for HDR files: {hdr_names}")
    log.error(f"  Checked {len(resolved_candidates)} candidate paths:")
    for i, c in enumerate(resolved_candidates):
        try:
            exists = c.exists()
            is_dir = c.is_dir() if exists else False
            log.error(f"    {i+1}. {c}")
            log.error(f"       exists={exists}, is_dir={is_dir}")
        except Exception as e:
            log.error(f"    {i+1}. {c} (error: {e})")
    log.warn(f"Using default: {default}")
    return default

# ============================================================================
# LOGGING & ERROR HANDLING
# ============================================================================

class Logger:
    """Simple logger with levels."""
    LEVELS = {"DEBUG": 0, "INFO": 1, "WARN": 2, "ERROR": 3}
    level = 1
    
    @classmethod
    def set_level(cls, level: str):
        cls.level = cls.LEVELS.get(level.upper(), 1)
    
    @classmethod
    def _log(cls, level: str, msg: str):
        if cls.LEVELS.get(level, 0) >= cls.level:
            prefix = {"DEBUG": "🔍", "INFO": "✓", "WARN": "⚠️", "ERROR": "❌"}.get(level, "")
            print(f"{prefix} [{level}] {msg}")
    
    @classmethod
    def debug(cls, msg): cls._log("DEBUG", msg)
    @classmethod
    def info(cls, msg): cls._log("INFO", msg)
    @classmethod
    def warn(cls, msg): cls._log("WARN", msg)
    @classmethod
    def error(cls, msg): cls._log("ERROR", msg)

log = Logger()

def safe_execute(func: Callable, *args, **kwargs) -> Tuple[bool, Any]:
    """Execute function with error handling."""
    try:
        result = func(*args, **kwargs)
        return True, result
    except Exception as e:
        log.error(f"{func.__name__}: {e}")
        if log.level == 0:
            traceback.print_exc()
        return False, None


# ============================================================================
# BLENDER UTILITIES (Version-Safe)
# ============================================================================

def ensure_blender():
    """Check if running in Blender."""
    if not IN_BLENDER:
        print("ERROR: This script must be run inside Blender.")
        print("Usage: blender --python blender_scene_tools.py -- --help")
        sys.exit(1)

def get_blender_info() -> Dict:
    """Get Blender version and capabilities."""
    ensure_blender()
    return {
        "version": f"{BLENDER_VERSION[0]}.{BLENDER_VERSION[1]}.{BLENDER_VERSION[2]}",
        "version_tuple": BLENDER_VERSION,
        "api_compat": API_COMPAT,
        "gltf_available": hasattr(bpy.ops.export_scene, 'gltf'),
    }

def clear_scene(keep_camera=False, keep_lights=False):
    """Remove objects from scene safely."""
    ensure_blender()
    
    for obj in list(bpy.data.objects):
        if keep_camera and obj.type == 'CAMERA':
            continue
        if keep_lights and obj.type == 'LIGHT':
            continue
        bpy.data.objects.remove(obj, do_unlink=True)
    
    # Clean orphan data
    for block in list(bpy.data.meshes):
        if block.users == 0:
            bpy.data.meshes.remove(block)
    for block in list(bpy.data.materials):
        if block.users == 0:
            bpy.data.materials.remove(block)
    
    log.info("Scene cleared")

def apply_transforms(obj):
    """Apply all transforms to object (safe for all versions)."""
    ensure_blender()
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    obj.select_set(False)

# ============================================================================
# MATERIAL CREATION (Version-Compatible)
# ============================================================================

def create_material(name: str, color: Tuple[float, float, float], 
                   metalness: float = 0.0, roughness: float = 0.5,
                   emission: Optional[Tuple[float, float, float]] = None) -> 'bpy.types.Material':
    """Create PBR material compatible with all Blender versions."""
    ensure_blender()
    
    # Check if material exists
    if name in bpy.data.materials:
        return bpy.data.materials[name]
    
    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()
    
    # Principled BSDF (works 2.80+)
    bsdf = nodes.new('ShaderNodeBsdfPrincipled')
    bsdf.location = (0, 0)
    bsdf.inputs['Base Color'].default_value = (*color, 1.0)
    bsdf.inputs['Metallic'].default_value = metalness
    bsdf.inputs['Roughness'].default_value = roughness
    
    # Emission (handle API changes)
    if emission:
        emission_input = API_COMPAT.get("emission_input_name", "Emission")
        if emission_input in bsdf.inputs:
            bsdf.inputs[emission_input].default_value = (*emission, 1.0)
        # Blender 4.0+ has separate Emission Color and Emission Strength
        if "Emission Strength" in bsdf.inputs:
            bsdf.inputs["Emission Strength"].default_value = 0.5
    
    # Output
    output = nodes.new('ShaderNodeOutputMaterial')
    output.location = (300, 0)
    links.new(bsdf.outputs['BSDF'], output.inputs['Surface'])
    
    return mat


# ============================================================================
# MESH CREATION (Safe Primitives)
# ============================================================================

def create_box(name: str, dimensions: List[float], position: List[float],
               rotation: Optional[List[float]] = None, 
               material: Optional['bpy.types.Material'] = None) -> 'bpy.types.Object':
    """Create box mesh matching THREE.BoxGeometry."""
    ensure_blender()
    
    bpy.ops.mesh.primitive_cube_add(size=1, location=position)
    obj = bpy.context.active_object
    obj.name = name
    obj.scale = [d / 2 for d in dimensions]
    
    if rotation:
        obj.rotation_euler = rotation
    if material:
        obj.data.materials.append(material)
    
    return obj

def create_cylinder(name: str, radius: float, height: float, segments: int = 16,
                   position: Tuple = (0, 0, 0), rotation: Optional[Tuple] = None,
                   material: Optional['bpy.types.Material'] = None) -> 'bpy.types.Object':
    """Create cylinder mesh matching THREE.CylinderGeometry."""
    ensure_blender()
    
    bpy.ops.mesh.primitive_cylinder_add(
        radius=radius, depth=height, vertices=segments, location=position
    )
    obj = bpy.context.active_object
    obj.name = name
    
    if rotation:
        obj.rotation_euler = rotation
    if material:
        obj.data.materials.append(material)
    
    return obj

def create_cone(name: str, radius: float, height: float, segments: int = 8,
               position: Tuple = (0, 0, 0), 
               material: Optional['bpy.types.Material'] = None) -> 'bpy.types.Object':
    """Create cone mesh matching THREE.ConeGeometry."""
    ensure_blender()
    
    bpy.ops.mesh.primitive_cone_add(
        radius1=radius, radius2=0, depth=height, vertices=segments, location=position
    )
    obj = bpy.context.active_object
    obj.name = name
    
    if material:
        obj.data.materials.append(material)
    
    return obj

def create_plane(name: str, size: float, position: Tuple = (0, 0, 0),
                subdivisions: int = 0,
                material: Optional['bpy.types.Material'] = None) -> 'bpy.types.Object':
    """Create plane mesh."""
    ensure_blender()
    
    bpy.ops.mesh.primitive_plane_add(size=size, location=position)
    obj = bpy.context.active_object
    obj.name = name
    
    if subdivisions > 0:
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.subdivide(number_cuts=subdivisions)
        bpy.ops.object.mode_set(mode='OBJECT')
    
    if material:
        obj.data.materials.append(material)
    
    return obj

# ============================================================================
# CAR MODEL CREATION
# ============================================================================

def create_car_model() -> 'bpy.types.Object':
    """Create procedural car model from unified scene definition."""
    ensure_blender()
    log.info("Creating car model from scene definition...")
    
    # Load scene definition
    scene_def = get_scene_definition()
    car_def = scene_def.get("car", {})
    physics = scene_def.get("physics", {})
    
    # Parent empty
    bpy.ops.object.empty_add(type='PLAIN_AXES', location=(0, 0, 0))
    car_parent = bpy.context.active_object
    car_parent.name = "Car"
    
    # Create materials from scene definition
    mats = {}
    
    # Body material
    body_part = car_def.get("body", {})
    body_color = tuple(body_part.get("color", [1.0, 0.42, 0.21]))
    mats["body"] = create_material("CarBody", body_color, 
                                   body_part.get("metalness", 0.7), 
                                   body_part.get("roughness", 0.3))
    
    # Cabin material
    cabin_part = car_def.get("cabin", {})
    cabin_color = tuple(cabin_part.get("color", [0.17, 0.24, 0.31]))
    mats["cabin"] = create_material("CarCabin", cabin_color,
                                    cabin_part.get("metalness", 0.8),
                                    cabin_part.get("roughness", 0.2))
    
    # Standard materials
    mats["wheel"] = create_material("Wheel", (0.1, 0.1, 0.1), 0.5, 0.8)
    mats["rim"] = create_material("Rim", (0.8, 0.8, 0.8), 0.9, 0.1)
    mats["headlight"] = create_material("Headlight", (1.0, 1.0, 0.67), emission=(1.0, 1.0, 0.67))
    mats["taillight"] = create_material("Taillight", (1.0, 0, 0), emission=(1.0, 0, 0))
    
    # Body parts from scene definition
    for part_name in ["body", "cabin", "hood", "spoiler"]:
        part = car_def.get(part_name)
        if not part:
            continue
        
        # Determine material
        if part_name in ["body", "hood", "spoiler"]:
            mat = mats["body"]
        else:
            mat = mats["cabin"]
        
        # Create part
        obj = create_box(
            part_name.capitalize(), 
            part.get("dimensions", [1, 1, 1]), 
            part.get("position", [0, 0, 0]),
            part.get("rotation"), 
            mat
        )
        obj.parent = car_parent
    
    # Wheels from scene definition
    wheel_r = physics.get("wheel_radius", CONFIG["car"]["wheel_radius"])
    wheel_w = CONFIG["car"]["wheel_width"]
    
    for wheel_def in car_def.get("wheels", []):
        # Assembly empty
        pos = wheel_def.get("position", [0, 0, 0])
        bpy.ops.object.empty_add(type='PLAIN_AXES', location=pos)
        assembly = bpy.context.active_object
        assembly.name = wheel_def.get("name", "wheel")
        assembly.parent = car_parent
        
        # Tire (rotated to align with X axis for proper spinning)
        tire = create_cylinder(f"{wheel_def['name']}_tire", wheel_r, wheel_w, 16,
                              (0, 0, 0), (math.pi/2, 0, 0), mats["wheel"])
        tire.parent = assembly
        tire.location = (0, 0, 0)
        
        # Rim
        rim = create_cylinder(f"{wheel_def['name']}_rim", 0.25, wheel_w + 0.05, 16,
                             (0, 0, 0), (math.pi/2, 0, 0), mats["rim"])
        rim.parent = assembly
        rim.location = (0, 0, 0)
    
    # Lights
    light_dims = [0.3, 0.2, 0.1]
    for i, hl in enumerate(car_def.get("headlights", [])):
        obj = create_box(f"Headlight_{i}", light_dims, hl.get("position", [0, 0, 0]), 
                        material=mats["headlight"])
        obj.parent = car_parent
    
    for i, tl in enumerate(car_def.get("taillights", [])):
        obj = create_box(f"Taillight_{i}", light_dims, tl.get("position", [0, 0, 0]), 
                        material=mats["taillight"])
        obj.parent = car_parent
    
    log.info("Car model created from scene definition")
    return car_parent


# ============================================================================
# TERRAIN & ENVIRONMENT
# ============================================================================

def terrain_height_function(x: float, z: float) -> float:
    """
    Calculate terrain height from unified scene definition.
    Uses noise parameters from scene_definition.json for consistency.
    """
    scene_def = get_scene_definition()
    terrain = scene_def.get("terrain", {})
    noise = terrain.get("noise", {})
    
    # Get noise parameters (with fallbacks matching final-demo.html)
    frequencies = noise.get("frequency", [0.02, 0.015, 0.01])
    amplitudes = noise.get("amplitude", [2, 1.5, 3])
    
    h = 0
    for freq, amp in zip(frequencies, amplitudes):
        h += math.sin(x * freq) * amp
        h += math.cos(z * freq * 0.75) * amp * 0.75
    
    return h

def create_terrain(size: int = 200, segments: int = 50, 
                  height_func: Optional[Callable] = None) -> 'bpy.types.Object':
    """Create terrain mesh."""
    ensure_blender()
    log.info(f"Creating terrain ({size}x{size}, {segments} segments)...")
    
    terrain_mat = create_material("Terrain", (0.29, 0.49, 0.23), roughness=0.85)
    terrain = create_plane("Terrain", size, (0, 0, 0), segments, terrain_mat)
    
    if height_func:
        mesh = terrain.data
        for vert in mesh.vertices:
            x, y, _ = vert.co
            vert.co.z = height_func(x, y)
        mesh.update()
    
    log.info("Terrain created")
    return terrain

def create_tree(position: Tuple[float, float, float], scale: float = 1.0) -> 'bpy.types.Object':
    """Create pine tree (Blender Z-up coordinate system)."""
    ensure_blender()
    
    bpy.ops.object.empty_add(type='PLAIN_AXES', location=position)
    tree = bpy.context.active_object
    tree.name = "Tree"
    tree.scale = (scale, scale, scale)
    
    trunk_mat = create_material("TreeTrunk", (0.29, 0.22, 0.16), roughness=0.9)
    foliage_mat = create_material("TreeFoliage", (0.18, 0.31, 0.09), roughness=0.9)
    
    # Trunk - Z is up in Blender, cylinder is created along Z by default
    trunk = create_cylinder("Trunk", 0.35, 3, 8, (0, 0, 1.5), material=trunk_mat)
    trunk.parent = tree
    trunk.location = (0, 0, 1.5)
    
    # Foliage layers - Z is up, so heights are on Z axis
    for i, (z, r) in enumerate([(2.5, 2.0), (3.5, 1.6), (4.3, 1.2), (5.0, 0.7)]):
        foliage = create_cone(f"Foliage_{i}", r, 1.5, 8, (0, 0, z), foliage_mat)
        foliage.parent = tree
        foliage.location = (0, 0, z)
    
    return tree

# ============================================================================
# EXPORT FUNCTIONS (Robust, Version-Safe)
# ============================================================================

def get_gltf_export_settings(filepath: str, selected_only: bool = False) -> Dict:
    """Get version-appropriate glTF export settings."""
    is_glb = filepath.lower().endswith('.glb')
    
    # Base settings (work on all versions)
    settings = {
        'filepath': filepath,
        'use_selection': selected_only,
        'export_format': 'GLB' if is_glb else 'GLTF_SEPARATE',
        'export_apply_modifiers': True,
        'export_cameras': False,
        'export_lights': False,
    }
    
    # Version-specific settings
    if BLENDER_VERSION >= (3, 6, 0):
        settings.update({
            'export_materials': 'EXPORT',
            'export_texcoords': True,
            'export_normals': True,
            'export_colors': True,
        })
    elif BLENDER_VERSION >= (2, 80, 0):
        settings.update({
            'export_materials': 'EXPORT',
            'export_texcoords': True,
            'export_normals': True,
        })
    
    return settings

def export_gltf(filepath: str, selected_only: bool = False) -> bool:
    """Export scene to GLB/glTF with maximum compatibility."""
    ensure_blender()
    
    # Ensure directory exists
    Path(filepath).parent.mkdir(parents=True, exist_ok=True)
    
    # Check if glTF exporter is available
    if not hasattr(bpy.ops.export_scene, 'gltf'):
        log.error("glTF exporter not available. Enable it in Preferences > Add-ons")
        return False
    
    settings = get_gltf_export_settings(filepath, selected_only)
    
    try:
        bpy.ops.export_scene.gltf(**settings)
        log.info(f"Exported: {filepath}")
        return True
    except Exception as e:
        log.error(f"Export failed: {e}")
        # Try minimal export
        try:
            minimal = {'filepath': filepath, 'export_format': 'GLB' if filepath.endswith('.glb') else 'GLTF_SEPARATE'}
            bpy.ops.export_scene.gltf(**minimal)
            log.warn(f"Exported with minimal settings: {filepath}")
            return True
        except:
            return False

def export_car(filepath: str = "car_model.glb", apply_rotation: bool = False) -> bool:
    """
    Export only the car model.
    
    Args:
        filepath: Output file path
        apply_rotation: NOT USED - kept for compatibility
                       Three.js and Blender both use Y-up, no rotation needed
    """
    ensure_blender()
    
    bpy.ops.object.select_all(action='DESELECT')
    car = bpy.data.objects.get("Car")
    
    if not car:
        log.error("No 'Car' object found")
        return False
    
    # Select car and all children
    car.select_set(True)
    for child in car.children_recursive:
        child.select_set(True)
    
    # Export without rotation - both systems use Y-up
    result = export_gltf(filepath, selected_only=True)
    
    return result

def export_scene_json(filepath: str = "scene_data.json") -> bool:
    """Export scene metadata as JSON for Three.js."""
    ensure_blender()
    
    scene_data = {
        "metadata": {
            "generator": "CarX Blender Tools",
            "version": CONFIG["version"],
            "blender_version": f"{BLENDER_VERSION[0]}.{BLENDER_VERSION[1]}.{BLENDER_VERSION[2]}",
        },
        "objects": [],
        "materials": [],
    }
    
    for obj in bpy.data.objects:
        if obj.type == 'MESH':
            obj_data = {
                "name": obj.name,
                "type": "mesh",
                "position": list(obj.location),
                "rotation": list(obj.rotation_euler),
                "scale": list(obj.scale),
                "parent": obj.parent.name if obj.parent else None,
            }
            scene_data["objects"].append(obj_data)
    
    for mat in bpy.data.materials:
        if mat.use_nodes:
            scene_data["materials"].append({"name": mat.name})
    
    try:
        with open(filepath, 'w') as f:
            json.dump(scene_data, f, indent=2)
        log.info(f"Exported JSON: {filepath}")
        return True
    except Exception as e:
        log.error(f"JSON export failed: {e}")
        return False


# ============================================================================
# MULTI-FORMAT IMPORT (FBX, OBJ, DAE, glTF, etc.)
# ============================================================================

SUPPORTED_FORMATS = {
    '.glb': 'gltf',
    '.gltf': 'gltf',
    '.fbx': 'fbx',
    '.obj': 'obj',
    '.dae': 'collada',
    '.stl': 'stl',
    '.ply': 'ply',
    '.abc': 'alembic',
    '.blend': 'blend',
}

def get_import_operator(ext: str) -> Optional[Callable]:
    """Get the appropriate import operator for file extension."""
    ensure_blender()
    
    operators = {
        'gltf': getattr(bpy.ops.import_scene, 'gltf', None),
        'fbx': getattr(bpy.ops.import_scene, 'fbx', None),
        'obj': getattr(bpy.ops.import_scene, 'obj', None) or getattr(bpy.ops.wm, 'obj_import', None),
        'collada': getattr(bpy.ops.wm, 'collada_import', None),
        'stl': getattr(bpy.ops.import_mesh, 'stl', None),
        'ply': getattr(bpy.ops.import_mesh, 'ply', None),
        'alembic': getattr(bpy.ops.wm, 'alembic_import', None),
    }
    
    format_type = SUPPORTED_FORMATS.get(ext.lower())
    return operators.get(format_type)

def import_any_format(filepath: str, apply_transform: bool = True, scale: float = 1.0) -> Optional[List['bpy.types.Object']]:
    """
    Import 3D file in any supported format (FBX, OBJ, glTF, DAE, STL, etc.)
    Automatically detects format and uses appropriate importer.
    
    Args:
        filepath: Path to the 3D file
        apply_transform: Whether to apply transforms after import
        scale: Scale factor to apply to imported objects
    """
    ensure_blender()
    
    if not os.path.exists(filepath):
        log.error(f"File not found: {filepath}")
        return None
    
    ext = Path(filepath).suffix.lower()
    
    if ext not in SUPPORTED_FORMATS:
        log.error(f"Unsupported format: {ext}")
        log.info(f"Supported formats: {', '.join(SUPPORTED_FORMATS.keys())}")
        return None
    
    # Store current selection
    prev_selected = set(bpy.context.selected_objects)
    bpy.ops.object.select_all(action='DESELECT')
    
    format_type = SUPPORTED_FORMATS[ext]
    log.info(f"Importing {format_type.upper()}: {filepath}")
    
    try:
        if format_type == 'gltf':
            bpy.ops.import_scene.gltf(filepath=filepath)
        
        elif format_type == 'fbx':
            if not hasattr(bpy.ops.import_scene, 'fbx'):
                log.error("FBX importer not available. Enable 'Import-Export: FBX format' addon")
                return None
            bpy.ops.import_scene.fbx(
                filepath=filepath,
                use_anim=True,
                use_custom_normals=True,
                ignore_leaf_bones=True,
                automatic_bone_orientation=True,
            )
        
        elif format_type == 'obj':
            # Blender 4.0+ uses wm.obj_import, older uses import_scene.obj
            if hasattr(bpy.ops.wm, 'obj_import'):
                bpy.ops.wm.obj_import(filepath=filepath)
            elif hasattr(bpy.ops.import_scene, 'obj'):
                bpy.ops.import_scene.obj(filepath=filepath)
            else:
                log.error("OBJ importer not available")
                return None
        
        elif format_type == 'collada':
            bpy.ops.wm.collada_import(filepath=filepath)
        
        elif format_type == 'stl':
            bpy.ops.import_mesh.stl(filepath=filepath)
        
        elif format_type == 'ply':
            bpy.ops.import_mesh.ply(filepath=filepath)
        
        elif format_type == 'alembic':
            bpy.ops.wm.alembic_import(filepath=filepath)
        
        # Get newly imported objects
        imported = [obj for obj in bpy.context.selected_objects if obj not in prev_selected]
        
        if not imported:
            # Some importers don't select, find new objects
            all_objects = set(bpy.data.objects)
            imported = list(all_objects - prev_selected)
        
        log.info(f"Imported {len(imported)} objects")
        
        # Apply scale if specified
        if scale != 1.0 and imported:
            for obj in imported:
                obj.scale = (scale, scale, scale)
        
        # Apply transforms if requested
        if apply_transform and imported:
            for obj in imported:
                if obj.type == 'MESH':
                    obj.select_set(True)
                    bpy.context.view_layer.objects.active = obj
            if bpy.context.selected_objects:
                bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
            bpy.ops.object.select_all(action='DESELECT')
        
        return imported
        
    except Exception as e:
        log.error(f"Import failed: {e}")
        traceback.print_exc()
        return None

def import_gltf(filepath: str) -> Optional[List['bpy.types.Object']]:
    """Import GLB/glTF file (wrapper for compatibility)."""
    return import_any_format(filepath)

# ============================================================================
# ANIMATION SUPPORT
# ============================================================================

def get_scene_animations() -> List[Dict]:
    """Get all animations in the scene."""
    ensure_blender()
    
    animations = []
    
    for action in bpy.data.actions:
        anim_data = {
            "name": action.name,
            "frame_start": int(action.frame_range[0]),
            "frame_end": int(action.frame_range[1]),
            "fps": bpy.context.scene.render.fps,
            "channels": len(action.fcurves),
            "users": action.users,
        }
        animations.append(anim_data)
    
    return animations

def export_with_animations(filepath: str, selected_only: bool = False) -> bool:
    """Export scene with animations to GLB."""
    ensure_blender()
    
    Path(filepath).parent.mkdir(parents=True, exist_ok=True)
    
    settings = get_gltf_export_settings(filepath, selected_only)
    
    # Add animation settings
    settings.update({
        'export_animations': True,
        'export_frame_range': True,
        'export_frame_step': 1,
        'export_force_sampling': True,
        'export_nla_strips': True,
        'export_current_frame': False,
    })
    
    # Version-specific animation settings
    if BLENDER_VERSION >= (3, 3, 0):
        settings['export_animation_mode'] = 'ACTIONS'
    
    try:
        bpy.ops.export_scene.gltf(**settings)
        log.info(f"Exported with animations: {filepath}")
        return True
    except Exception as e:
        log.error(f"Animation export failed: {e}")
        # Fallback to non-animated export
        return export_gltf(filepath, selected_only)

def create_wheel_spin_animation(car_obj: 'bpy.types.Object', duration: float = 2.0, 
                                 rotations: float = 4.0) -> bool:
    """Create wheel spinning animation for the car."""
    ensure_blender()
    
    if not car_obj:
        log.error("No car object provided")
        return False
    
    fps = bpy.context.scene.render.fps
    frame_end = int(duration * fps)
    
    # Find wheel objects
    wheels = [obj for obj in car_obj.children_recursive 
              if 'wheel' in obj.name.lower() and obj.type == 'EMPTY']
    
    if not wheels:
        # Try finding wheel meshes directly
        wheels = [obj for obj in car_obj.children_recursive 
                  if 'wheel' in obj.name.lower() and 'tire' in obj.name.lower()]
    
    if not wheels:
        log.warn("No wheel objects found")
        return False
    
    log.info(f"Creating spin animation for {len(wheels)} wheels")
    
    for wheel in wheels:
        # Create action
        action_name = f"{wheel.name}_spin"
        action = bpy.data.actions.new(name=action_name)
        
        # Assign to wheel
        if not wheel.animation_data:
            wheel.animation_data_create()
        wheel.animation_data.action = action
        
        # Create rotation keyframes (rotate around local X for spinning)
        fcurve = action.fcurves.new(data_path="rotation_euler", index=0)  # X rotation
        
        # Start keyframe
        fcurve.keyframe_points.insert(1, 0)
        # End keyframe
        fcurve.keyframe_points.insert(frame_end, rotations * 2 * math.pi)
        
        # Set interpolation to linear
        for kf in fcurve.keyframe_points:
            kf.interpolation = 'LINEAR'
    
    # Set scene frame range
    bpy.context.scene.frame_start = 1
    bpy.context.scene.frame_end = frame_end
    
    log.info(f"Created wheel spin animation: {duration}s, {rotations} rotations")
    return True

def clear_animations(obj: Optional['bpy.types.Object'] = None):
    """Clear animations from object or entire scene."""
    ensure_blender()
    
    if obj:
        if obj.animation_data:
            obj.animation_data_clear()
        for child in obj.children_recursive:
            if child.animation_data:
                child.animation_data_clear()
        log.info(f"Cleared animations from {obj.name}")
    else:
        for action in list(bpy.data.actions):
            bpy.data.actions.remove(action)
        log.info("Cleared all animations")

# ============================================================================
# ENHANCED PIPELINE (Round-Trip Workflow)
# ============================================================================

def convert_to_carx_format(input_path: str, output_path: Optional[str] = None,
                           include_animations: bool = True) -> bool:
    """
    Convert any 3D format to CarX-compatible GLB.
    Handles FBX, OBJ, DAE, etc. and outputs optimized GLB.
    """
    ensure_blender()
    
    if output_path is None:
        output_path = str(Path(input_path).with_suffix('.glb'))
    
    log.info(f"Converting: {input_path} -> {output_path}")
    
    # Clear scene
    clear_scene()
    
    # Import source file
    imported = import_any_format(input_path)
    if not imported:
        return False
    
    # Organize hierarchy
    _organize_imported_hierarchy(imported)
    
    # Export to GLB
    if include_animations and bpy.data.actions:
        return export_with_animations(output_path)
    else:
        return export_gltf(output_path)

def _organize_imported_hierarchy(objects: List['bpy.types.Object']):
    """Organize imported objects into CarX-compatible hierarchy."""
    ensure_blender()
    
    # Find root objects (no parent)
    roots = [obj for obj in objects if obj.parent is None]
    
    if len(roots) == 1:
        # Single root - rename to "Car" if it looks like a vehicle
        root = roots[0]
        if _looks_like_car(root):
            root.name = "Car"
            log.info("Renamed root to 'Car'")
    elif len(roots) > 1:
        # Multiple roots - create parent empty
        bpy.ops.object.empty_add(type='PLAIN_AXES', location=(0, 0, 0))
        parent = bpy.context.active_object
        parent.name = "Car"
        
        for root in roots:
            root.parent = parent
        
        log.info(f"Created parent 'Car' for {len(roots)} root objects")
    
    # Identify and rename wheels
    _identify_wheels(objects)

def _looks_like_car(obj: 'bpy.types.Object') -> bool:
    """Heuristic to determine if object is a car."""
    name_lower = obj.name.lower()
    car_keywords = ['car', 'vehicle', 'auto', 'body', 'chassis']
    return any(kw in name_lower for kw in car_keywords)

def _identify_wheels(objects: List['bpy.types.Object']):
    """Identify and standardize wheel naming."""
    wheel_keywords = ['wheel', 'tire', 'tyre', 'rim']
    position_keywords = {
        'fl': 'wheel_FL', 'front_left': 'wheel_FL', 'frontleft': 'wheel_FL',
        'fr': 'wheel_FR', 'front_right': 'wheel_FR', 'frontright': 'wheel_FR',
        'rl': 'wheel_RL', 'rear_left': 'wheel_RL', 'rearleft': 'wheel_RL', 'bl': 'wheel_RL',
        'rr': 'wheel_RR', 'rear_right': 'wheel_RR', 'rearright': 'wheel_RR', 'br': 'wheel_RR',
    }
    
    for obj in objects:
        name_lower = obj.name.lower().replace(' ', '_').replace('-', '_')
        
        # Check if it's a wheel
        if not any(kw in name_lower for kw in wheel_keywords):
            continue
        
        # Try to identify position
        for key, standard_name in position_keywords.items():
            if key in name_lower:
                if obj.type == 'EMPTY' or (obj.parent and obj.parent.type == 'EMPTY'):
                    # This is a wheel assembly
                    target = obj if obj.type == 'EMPTY' else obj.parent
                    if target.name != standard_name:
                        log.debug(f"Renamed wheel: {target.name} -> {standard_name}")
                        target.name = standard_name
                break

def update_scene_from_external(filepath: str, replace_car: bool = True,
                               preserve_materials: bool = False) -> bool:
    """
    Import external file and integrate into CarX scene.
    Supports FBX, OBJ, glTF, DAE, etc.
    """
    ensure_blender()
    
    # Import the file
    imported = import_any_format(filepath)
    if not imported:
        return False
    
    if replace_car:
        # Remove existing car
        old_car = bpy.data.objects.get("Car")
        if old_car:
            # Optionally preserve materials
            old_materials = []
            if preserve_materials:
                for child in old_car.children_recursive:
                    if child.type == 'MESH' and child.data.materials:
                        old_materials.extend(child.data.materials)
            
            # Remove old car
            for child in list(old_car.children_recursive):
                bpy.data.objects.remove(child, do_unlink=True)
            bpy.data.objects.remove(old_car, do_unlink=True)
            log.info("Removed existing car")
    
    # Organize imported objects
    _organize_imported_hierarchy(imported)
    
    log.info(f"Updated scene from: {filepath}")
    return True

def update_scene_from_glb(filepath: str, replace_car: bool = True) -> bool:
    """Import GLB and integrate into existing scene (wrapper for compatibility)."""
    return update_scene_from_external(filepath, replace_car)

# ============================================================================
# SCENE GENERATION
# ============================================================================

def load_existing_models() -> Dict[str, 'bpy.types.Object']:
    """
    Load existing GLB models from preview directory if available.
    Returns dict with 'car' and 'tree' keys if found.
    
    This function searches for car models in order from CONFIG['car_model_names']:
    - lowpoly_game_model.glb, car_model.glb, car.glb, vehicle.glb
    Also searches for:
    - tree.glb (tree model)
    - farmland_overcast_1k.exr (HDR environment - logged but not loaded here)
    """
    ensure_blender()
    preview_dir = get_preview_dir()
    
    models = {}
    
    log.info(f"Searching for assets in: {preview_dir}")
    
    # List available files for debugging
    if preview_dir.exists():
        try:
            available_files = [f.name for f in preview_dir.iterdir() if f.is_file()]
            log.debug(f"  Available files: {available_files}")
        except Exception as e:
            log.debug(f"  Could not list files: {e}")
    else:
        log.error(f"Preview directory does not exist: {preview_dir}")
        log.error(f"  Please ensure the preview/ folder exists with GLB assets")
        return models
    
    # Try to load car model - check multiple filenames in order
    car_model_names = CONFIG.get("car_model_names", ["lowpoly_game_model.glb"])
    car_path = None
    
    for glb_name in car_model_names:
        candidate = preview_dir / glb_name
        if candidate.exists():
            car_path = candidate
            log.info(f"Found car model: {glb_name}")
            break
    
    if car_path:
        log.info(f"Loading car model: {car_path}")
        try:
            imported = import_any_format(str(car_path), apply_transform=False, scale=1.0)
            if imported:
                # Find root object
                root = None
                for obj in imported:
                    if obj.parent is None:
                        root = obj
                        break
                if not root and imported:
                    root = imported[0]
                
                if root:
                    root.name = "Car"
                    # NO ROTATION NEEDED HERE - rotation is applied in generate_scene()
                    # The model is in glTF Y-up space
                    models['car'] = root
                    log.info("✓ Loaded car model from GLB")
            else:
                log.warn(f"GLB import returned no objects: {car_path}")
        except Exception as e:
            log.error(f"Failed to load car model: {e}")
    else:
        log.warn(f"Car model not found in: {preview_dir}")
        log.warn(f"  Searched for: {car_model_names}")
    
    # Try to load tree model
    tree_path = preview_dir / "tree.glb"
    if tree_path.exists():
        log.info(f"Loading tree model: {tree_path}")
        try:
            imported = import_any_format(str(tree_path), apply_transform=False)
            if imported:
                root = None
                for obj in imported:
                    if obj.parent is None:
                        root = obj
                        break
                if not root and imported:
                    root = imported[0]
                
                if root:
                    root.name = "TreeTemplate"
                    models['tree'] = root
                    log.info("✓ Loaded tree model from GLB")
            else:
                log.warn(f"GLB import returned no objects: {tree_path}")
        except Exception as e:
            log.error(f"Failed to load tree model: {e}")
    else:
        log.debug(f"Tree model not found (optional): {tree_path}")
    
    # Check for HDR environment (just log, don't load here)
    hdr_path = preview_dir / "farmland_overcast_1k.exr"
    if hdr_path.exists():
        log.info(f"✓ HDR environment available: {hdr_path.name}")
    else:
        log.debug(f"HDR environment not found (optional): {hdr_path}")
    
    return models

def generate_scene(include_terrain: bool = True, include_trees: bool = True,
                  tree_count: int = None, setup_camera: bool = True,
                  setup_lighting: bool = True, use_existing_models: bool = True) -> 'bpy.types.Object':
    """
    Generate complete CarX scene matching final-demo.html EXACTLY.
    
    This loads the actual GLB models used in Three.js and creates
    matching terrain, trees, lighting, and camera setup.
    
    CRITICAL: Apply the SAME transforms as final-demo.html:
    - Scale: 2.5 (carMesh.scale.set(2.5, 2.5, 2.5))
    - Rotation: -90° on Y-axis (carMesh.rotation.y = -Math.PI / 2)
    Both Blender and Three.js use Y-up coordinate system.
    """
    ensure_blender()
    log.info("=" * 50)
    log.info("Generating CarX scene to match final-demo.html...")
    log.info("=" * 50)
    
    # Load scene definition
    scene_def = get_scene_definition()
    terrain_def = scene_def.get("terrain", {})
    env_def = scene_def.get("environment", {})
    cam_def = scene_def.get("camera", {})
    veg_def = scene_def.get("vegetation", {})
    
    # Default tree count matching final-demo.html (high quality mode spawns ~50 trees)
    if tree_count is None:
        tree_count = veg_def.get("tree_count", 50)
    
    clear_scene()
    
    car = None
    tree_template = None
    
    # =========================================================================
    # STEP 1: Load the ACTUAL GLB models from preview/ directory
    # =========================================================================
    preview_dir = get_preview_dir()
    
    # Debug: Show path resolution info
    log.info(f"Path resolution debug:")
    log.info(f"  CONFIG absolute_preview_dir: {CONFIG.get('absolute_preview_dir')}")
    log.info(f"  Resolved preview_dir: {preview_dir}")
    log.info(f"  preview_dir exists: {preview_dir.exists()}")
    if preview_dir.exists():
        try:
            files = [f.name for f in preview_dir.iterdir() if f.suffix in ['.glb', '.exr']]
            log.info(f"  GLB/EXR files found: {files}")
        except Exception as e:
            log.warn(f"  Could not list files: {e}")
    
    # Load car model - check multiple filenames in order from CONFIG
    car_model_names = CONFIG.get("car_model_names", ["lowpoly_game_model.glb"])
    car_glb_path = None
    
    log.info(f"Searching for car model in: {preview_dir}")
    log.info(f"  Checking filenames: {car_model_names}")
    
    for glb_name in car_model_names:
        candidate = preview_dir / glb_name
        if candidate.exists():
            car_glb_path = candidate
            log.info(f"  Found: {glb_name}")
            break
        else:
            log.debug(f"  Not found: {glb_name}")
    
    # Debug: Show file info
    if car_glb_path:
        log.info(f"Car GLB path: {car_glb_path}")
        log.info(f"  Absolute: {car_glb_path.resolve()}")
    
    if car_glb_path and car_glb_path.exists():
        # Check file size
        file_size = car_glb_path.stat().st_size
        log.info(f"  File size: {file_size:,} bytes ({file_size/1024/1024:.2f} MB)")
        
        if file_size < 1000:
            log.error(f"  GLB file is too small ({file_size} bytes) - may be corrupted!")
        
        # Check if glTF importer is available
        if not hasattr(bpy.ops.import_scene, 'gltf'):
            log.error("glTF importer not available!")
            log.error("Enable 'Import-Export: glTF 2.0 format' addon in Blender Preferences > Add-ons")
            log.warn("Falling back to procedural car model")
            car = create_car_model()
        else:
            log.info(f"Loading car GLB: {car_glb_path}")
            
            try:
                # Store objects before import to find new ones
                objects_before = set(bpy.data.objects)
                
                # Deselect all first
                bpy.ops.object.select_all(action='DESELECT')
                
                # Import the GLB - this is the EXACT same model used in Three.js
                log.info("  Calling bpy.ops.import_scene.gltf()...")
                result = bpy.ops.import_scene.gltf(filepath=str(car_glb_path))
                log.info(f"  Import result: {result}")
                
                # Find ALL imported objects by comparing before/after
                objects_after = set(bpy.data.objects)
                imported_objects = list(objects_after - objects_before)
                
                # Also check selected objects as fallback
                if not imported_objects:
                    imported_objects = list(bpy.context.selected_objects)
                
                log.info(f"  Imported {len(imported_objects)} objects")
                
                # List all imported objects for debugging
                for obj in imported_objects:
                    log.info(f"    - {obj.name} (type: {obj.type})")
                
                if imported_objects:
                    # Find root object (the one with no parent among imported objects)
                    car = None
                    for obj in imported_objects:
                        if obj.parent is None or obj.parent not in imported_objects:
                            car = obj
                            log.info(f"  Found root: {obj.name} (type: {obj.type})")
                            break
                    
                    if not car:
                        car = imported_objects[0]
                        log.info(f"  Using first object as root: {car.name}")
                    
                    # If the root is an armature or empty, find the mesh
                    if car.type not in ['MESH', 'EMPTY']:
                        for obj in imported_objects:
                            if obj.type == 'MESH':
                                # Create a parent empty for the mesh
                                bpy.ops.object.empty_add(type='PLAIN_AXES', location=(0, 0, 0))
                                car_parent = bpy.context.active_object
                                car_parent.name = "Car"
                                obj.parent = car_parent
                                car = car_parent
                                log.info(f"  Created parent empty for mesh")
                                break
                    
                    # Rename root to "Car" for consistency
                    car.name = "Car"
                    
                    # =====================================================================
                    # CRITICAL: Apply EXACT same transforms as final-demo.html
                    # From final-demo.html:
                    #   carMesh.scale.set(2.5, 2.5, 2.5);
                    #   carMesh.rotation.y = -Math.PI / 2; // -90 degrees
                    #
                    # COORDINATE SYSTEM MAPPING:
                    #   Three.js: X=right, Y=up, Z=towards viewer (right-handed, Y-up)
                    #   Blender:  X=right, Y=forward, Z=up (right-handed, Z-up in viewport)
                    #   BUT: glTF uses Y-up, so Blender's glTF importer converts to Y-up
                    #
                    # When importing GLB into Blender:
                    #   - The model is already in Y-up space (glTF standard)
                    #   - Blender displays it correctly with Y as up
                    #   - Three.js rotation.y = Blender rotation around Z axis (after glTF import)
                    #
                    # IMPORTANT: Blender's glTF importer converts Y-up to Z-up!
                    # So the model comes in with Z as up-axis in Blender's coordinate system.
                    # Three.js rotation.y (yaw) = Blender rotation.z (yaw around Z-up)
                    # =====================================================================
                    
                    # Scale: 2.5x (matches Three.js)
                    car.scale = (2.5, 2.5, 2.5)
                    
                    # Rotation: -90° around Z-axis in Blender = -90° around Y-axis in Three.js
                    # This is because glTF Y-up becomes Blender Z-up after import
                    # Three.js: rotation.y = -Math.PI / 2 (yaw -90°)
                    # Blender:  rotation_euler.z = -math.pi / 2 (yaw -90° around Z)
                    car.rotation_euler = (0, 0, -math.pi / 2)
                    
                    # Position: At origin, slightly above ground
                    # In Blender with Z-up: Z is the height axis
                    car.location = (0, 0, 1.5)
                    
                    # Force update the scene to apply transforms
                    bpy.context.view_layer.update()
                    
                    log.info(f"✓ Car model loaded and transformed:")
                    log.info(f"  Root object: {car.name}")
                    log.info(f"  Scale: {tuple(car.scale)} (matches Three.js 2.5x)")
                    log.info(f"  Rotation: Z={math.degrees(car.rotation_euler.z):.1f}° (= Three.js Y=-90°)")
                    log.info(f"  Position: {tuple(car.location)} (Z-up in Blender)")
                else:
                    log.error("GLB imported but no objects found!")
                    log.warn("Creating procedural car as fallback")
                    car = create_car_model()
                    
            except Exception as e:
                log.error(f"Failed to import GLB: {e}")
                log.error(f"Exception type: {type(e).__name__}")
                import traceback
                traceback.print_exc()
                log.warn("Creating procedural car as fallback")
                car = create_car_model()
    else:
        log.error(f"Car GLB not found in: {preview_dir}")
        log.error(f"  Searched for filenames: {car_model_names}")
        log.warn("Creating procedural car as fallback")
        car = create_car_model()
    
    # NOTE: Three.js uses PROCEDURAL trees (createSimpleTree function), not GLB
    # So we skip loading tree.glb and use procedural trees to match exactly
    tree_template = None  # Force procedural tree creation
    log.info("Using procedural trees (matches final-demo.html createSimpleTree)")
    
    # =========================================================================
    # STEP 2: Create terrain matching final-demo.html
    # =========================================================================
    # Terrain height function matching final-demo.html 'hills' terrain
    # Defined outside conditional so it can be used for tree placement too
    # =========================================================================
    def fbm_approx(x, z, octaves=4, persistence=0.5, scale=50):
        """
        Approximate FBM noise using sin/cos waves.
        Matches final-demo.html: this.fbm(x, z, 4, 0.5, 50) * 8
        """
        total = 0
        amplitude = 1
        max_value = 0
        freq = 1
        
        for i in range(octaves):
            # Use sin/cos combination for pseudo-noise
            sx = x * freq / scale
            sz = z * freq / scale
            # Combine multiple sin/cos for more natural look
            noise_val = (math.sin(sx * 1.7 + sz * 0.3) * 0.5 + 
                        math.cos(sz * 1.3 - sx * 0.7) * 0.5 +
                        math.sin(sx * 2.1 + sz * 1.9) * 0.25 +
                        math.cos(sx * 0.9 - sz * 2.3) * 0.25) * 0.5 + 0.5
            total += noise_val * amplitude
            max_value += amplitude
            amplitude *= persistence
            freq *= 2
        
        return total / max_value if max_value > 0 else 0
    
    def get_terrain_height(x, z):
        """
        Calculate terrain height matching final-demo.html 'hills' terrain.
        From final-demo.html:
            height = this.fbm(x, z, 4, 0.5, 50) * 8 +
                     this.fbm(x, z, 2, 0.3, 20) * 2;
        """
        height = fbm_approx(x, z, 4, 0.5, 50) * 8
        height += fbm_approx(x, z, 2, 0.3, 20) * 2
        return height
    
    # =========================================================================
    terrain = None
    if include_terrain:
        size = terrain_def.get("size", 400)
        # Use fewer segments for Blender performance (Three.js uses 120)
        segments = min(terrain_def.get("segments", 120), 60)
        
        log.info(f"Creating terrain: {size}x{size}, {segments} segments")
        
        # Terrain color: #4A7C3A = (0.29, 0.49, 0.23) - matches Three.js
        terrain_mat = create_material("Terrain", (0.29, 0.49, 0.23), roughness=0.85)
        
        # Create a grid mesh directly in XZ plane (Y up) to match Three.js
        import bmesh
        
        mesh_data = bpy.data.meshes.new("TerrainMesh")
        terrain = bpy.data.objects.new("Terrain", mesh_data)
        bpy.context.collection.objects.link(terrain)
        
        bm = bmesh.new()
        
        # Create grid vertices in XY plane with Z as height
        # Blender native: X=right, Y=forward, Z=up
        # glTF export converts: Blender Z -> glTF Y (up), Blender Y -> glTF -Z (forward)
        half_size = size / 2
        step = size / segments
        
        verts = []
        for iy in range(segments + 1):
            row = []
            for ix in range(segments + 1):
                x = -half_size + ix * step
                y = -half_size + iy * step
                
                # Calculate height using FBM approximation matching final-demo.html
                # Pass (x, y) as the horizontal coordinates for noise
                z = get_terrain_height(x, y)
                
                # Create vertex at (X, Y, Z=height) - Z is up in Blender
                vert = bm.verts.new((x, y, z))
                row.append(vert)
            verts.append(row)
        
        bm.verts.ensure_lookup_table()
        
        # Create faces (winding order for correct normals pointing up)
        for iy in range(segments):
            for ix in range(segments):
                v1 = verts[iy][ix]
                v2 = verts[iy][ix + 1]
                v3 = verts[iy + 1][ix + 1]
                v4 = verts[iy + 1][ix]
                # Counter-clockwise winding for upward-facing normals
                bm.faces.new([v1, v4, v3, v2])
        
        bm.to_mesh(mesh_data)
        bm.free()
        
        # Update mesh - this handles normals automatically in all Blender versions
        mesh_data.update()
        
        # Apply material
        terrain.data.materials.append(terrain_mat)
        
        # Enable shadow receiving but not casting for terrain
        # Handle API differences across Blender versions safely
        try:
            if hasattr(terrain, 'cycles') and terrain.cycles is not None:
                if hasattr(terrain.cycles, 'is_shadow_catcher'):
                    terrain.cycles.is_shadow_catcher = False
        except (AttributeError, TypeError):
            pass  # Cycles not available or different API
        
        try:
            if hasattr(terrain, 'visible_shadow'):
                terrain.visible_shadow = False
        except (AttributeError, TypeError):
            pass
        
        log.info("✓ Terrain created with hills (XY plane, Z up - Blender native)")
        
        # Reposition car to sit on terrain at origin
        # In Blender: Z is up, so car.location.z is the height
        if car:
            terrain_height_at_origin = get_terrain_height(0, 0)
            car.location = (0, 0, terrain_height_at_origin + 1.0)
            log.info(f"  Car repositioned to terrain height: Z={car.location.z:.2f}")
    
    # =========================================================================
    # STEP 3: Create trees matching final-demo.html vegetation
    # =========================================================================
    if include_trees and tree_count > 0:
        import random
        random.seed(42)  # Deterministic placement for consistency
        
        log.info(f"Creating {tree_count} trees...")
        
        # Get vegetation settings from scene definition
        spawn_min = veg_def.get("spawn_min", (-80, -80))
        spawn_max = veg_def.get("spawn_max", (80, 80))
        exclusion_x = 15  # Exclusion zone around car path
        exclusion_z = 35
        
        trees_created = 0
        attempts = 0
        max_attempts = tree_count * 5
        
        # Create a collection for trees
        tree_collection = bpy.data.collections.new("Trees")
        bpy.context.scene.collection.children.link(tree_collection)
        
        while trees_created < tree_count and attempts < max_attempts:
            attempts += 1
            
            # Random position matching final-demo.html spawn area
            # In Blender: X=right, Y=forward (horizontal plane), Z=up
            x = random.uniform(spawn_min[0] if isinstance(spawn_min, (list, tuple)) else -80, 
                              spawn_max[0] if isinstance(spawn_max, (list, tuple)) else 80)
            y = random.uniform(spawn_min[1] if isinstance(spawn_min, (list, tuple)) else -80, 
                              spawn_max[1] if isinstance(spawn_max, (list, tuple)) else 80)
            
            # Exclusion zone (where car drives) - matches final-demo.html
            exclusion_y = 35  # Y is forward in Blender
            if abs(x) < exclusion_x and abs(y) < exclusion_y:
                continue
            
            # Calculate terrain height at this position using same FBM function
            # In Blender: X=right, Y=forward, Z=up (height)
            z_height = 0
            if include_terrain:
                # Use same FBM approximation as terrain (defined in STEP 2)
                z_height = get_terrain_height(x, y)
            
            scale_factor = random.uniform(0.8, 1.2)
            
            if tree_template:
                # Duplicate the GLB tree (linked duplicate for performance)
                tree = _duplicate_hierarchy(tree_template, f"Tree_{trees_created}")
                # Position: (X, Y, Z=height) - Z is up in Blender
                tree.location = (x, y, z_height)
                tree.scale = (scale_factor, scale_factor, scale_factor)
                tree.hide_set(False)
                tree.hide_render = False
                
                # Unhide all children too
                for child in tree.children_recursive:
                    child.hide_set(False)
                    child.hide_render = False
                
                # Move to trees collection
                for coll in tree.users_collection:
                    coll.objects.unlink(tree)
                tree_collection.objects.link(tree)
            else:
                # Create procedural tree as fallback
                # Position: (X, Y, Z=height) - Z is up in Blender
                tree = create_tree((x, y, z_height), scale_factor)
                tree.name = f"Tree_{trees_created}"
                
                # Move to trees collection
                for coll in tree.users_collection:
                    coll.objects.unlink(tree)
                tree_collection.objects.link(tree)
            
            trees_created += 1
        
        log.info(f"✓ Created {trees_created} trees in 'Trees' collection")
    
    # =========================================================================
    # STEP 4: Setup camera matching final-demo.html default view
    # =========================================================================
    if setup_camera:
        # Camera position from final-demo.html: camera.position.set(0, 12, 25)
        # Three.js: X=right, Y=up, Z=towards viewer
        # Blender:  X=right, Y=forward, Z=up
        # Conversion: Three.js (x, y, z) -> Blender (x, -z, y)
        # Three.js (0, 12, 25) -> Blender (0, -25, 12)
        cam_pos = (0, -25, 12)  # Behind and above the car (Blender Z-up)
        cam_target = (0, 0, 2)  # Look at car center (slightly above ground)
        
        bpy.ops.object.camera_add(location=cam_pos)
        cam = bpy.context.active_object
        cam.name = "MainCamera"
        
        # FOV 60 degrees = focal length ~30mm on 36mm sensor
        cam.data.lens = 30
        cam.data.sensor_width = 36
        
        # Point camera at target using track-to constraint for accuracy
        from mathutils import Vector
        direction = Vector(cam_target) - Vector(cam_pos)
        rot_quat = direction.to_track_quat('-Z', 'Y')
        cam.rotation_euler = rot_quat.to_euler()
        
        # Set as active camera
        bpy.context.scene.camera = cam
        
        # Set render resolution to match typical browser viewport
        bpy.context.scene.render.resolution_x = 1920
        bpy.context.scene.render.resolution_y = 1080
        
        log.info(f"✓ Camera setup: pos={cam_pos}, target={cam_target} (Blender Z-up)")
    
    # =========================================================================
    # STEP 5: Setup lighting matching final-demo.html
    # =========================================================================
    if setup_lighting:
        # Sun light from final-demo.html:
        # sunLight.position.set(50, 100, 50)
        # Three.js (x, y, z) -> Blender (x, -z, y) for Z-up conversion
        # Three.js (50, 100, 50) -> Blender (50, -50, 100)
        sun_pos = (50, -50, 100)
        
        bpy.ops.object.light_add(type='SUN', location=sun_pos)
        sun = bpy.context.active_object
        sun.name = "Sun"
        sun.data.energy = 3.0
        sun.data.color = (1.0, 0.96, 0.9)  # Warm sunlight #FFF5E6
        
        # Calculate rotation to point at origin from position
        from mathutils import Vector
        sun_dir = Vector((0, 0, 0)) - Vector(sun_pos)
        sun_rot = sun_dir.to_track_quat('-Z', 'Y').to_euler()
        sun.rotation_euler = sun_rot
        
        # Enable shadows
        sun.data.use_shadow = True
        if hasattr(sun.data, 'shadow_cascade_count'):
            sun.data.shadow_cascade_count = 4
        
        # Hemisphere/ambient light from final-demo.html:
        # hemisphereLight = new THREE.HemisphereLight(0x87ceeb, 0x4a7c3a, 0.6)
        # Sky color: #87CEEB, Ground color: #4A7C3A
        # Position above scene pointing down (Z is up in Blender)
        bpy.ops.object.light_add(type='SUN', location=(0, 0, 50))
        ambient = bpy.context.active_object
        ambient.name = "AmbientFill"
        ambient.data.energy = 0.8
        ambient.data.color = (0.53, 0.81, 0.92)  # Sky blue #87CEEB
        ambient.rotation_euler = (math.radians(180), 0, 0)  # Point down (-Z direction)
        ambient.data.use_shadow = False
        
        log.info("✓ Lighting setup: Sun + Ambient fill (Blender Z-up)")
    
    # =========================================================================
    # STEP 6: Setup world/background with HDR environment
    # =========================================================================
    world = bpy.context.scene.world
    if world is None:
        world = bpy.data.worlds.new("World")
        bpy.context.scene.world = world
    
    world.use_nodes = True
    nodes = world.node_tree.nodes
    links = world.node_tree.links
    
    # Try to load HDR environment - check multiple filenames from CONFIG
    hdr_names = CONFIG.get("hdr_names", ["farmland_overcast_1k.exr"])
    hdr_path = None
    hdr_loaded = False
    
    log.info(f"Searching for HDR environment in: {preview_dir}")
    log.info(f"  Checking filenames: {hdr_names}")
    
    for hdr_name in hdr_names:
        candidate = preview_dir / hdr_name
        if candidate.exists():
            hdr_path = candidate
            log.info(f"  Found: {hdr_name}")
            break
        else:
            log.debug(f"  Not found: {hdr_name}")
    
    if hdr_path and hdr_path.exists():
        file_size = hdr_path.stat().st_size
        log.info(f"Loading HDR environment: {hdr_path}")
        log.info(f"  File size: {file_size / 1024:.1f} KB ({file_size:,} bytes)")
        
        # Read and log first bytes for format verification
        try:
            with open(hdr_path, 'rb') as f:
                magic_bytes = f.read(4)
            log.info(f"  Magic bytes: {magic_bytes.hex()} ({magic_bytes!r})")
            # OpenEXR magic: 0x762f3101
            if magic_bytes != b'\x76\x2f\x31\x01':
                log.warn(f"  Unexpected magic bytes (expected 762f3101 for OpenEXR)")
        except Exception as e:
            log.warn(f"  Could not read magic bytes: {e}")
        
        try:
            # Load EXR as environment texture
            log.info("  Loading EXR image...")
            env_tex = bpy.data.images.load(str(hdr_path))
            log.info(f"  Image loaded: {env_tex.name} ({env_tex.size[0]}x{env_tex.size[1]})")
            
            # Clear existing nodes
            nodes.clear()
            
            # Create environment texture node setup
            tex_coord = nodes.new('ShaderNodeTexCoord')
            tex_coord.location = (-800, 300)
            
            mapping = nodes.new('ShaderNodeMapping')
            mapping.location = (-600, 300)
            
            env_texture = nodes.new('ShaderNodeTexEnvironment')
            env_texture.location = (-300, 300)
            env_texture.image = env_tex
            
            background = nodes.new('ShaderNodeBackground')
            background.location = (0, 300)
            background.inputs['Strength'].default_value = 0.5  # Match Three.js exposure
            
            output = nodes.new('ShaderNodeOutputWorld')
            output.location = (200, 300)
            
            # Link nodes
            links.new(tex_coord.outputs['Generated'], mapping.inputs['Vector'])
            links.new(mapping.outputs['Vector'], env_texture.inputs['Vector'])
            links.new(env_texture.outputs['Color'], background.inputs['Color'])
            links.new(background.outputs['Background'], output.inputs['Surface'])
            
            hdr_loaded = True
            log.info("✓ HDR environment loaded and applied to world")
        except Exception as e:
            log.error(f"Failed to load HDR: {e}")
            log.error(f"  Exception type: {type(e).__name__}")
            log.error(f"  File path: {hdr_path}")
            log.error(f"  File size: {file_size:,} bytes")
            try:
                with open(hdr_path, 'rb') as f:
                    header_bytes = f.read(32)
                log.error(f"  First 32 bytes: {header_bytes.hex()}")
            except:
                pass
            import traceback
            traceback.print_exc()
    else:
        log.warn(f"HDR file not found in: {preview_dir}")
        log.warn(f"  Searched for filenames: {hdr_names}")
    
    # Fallback to gradient sky if HDR not available
    if not hdr_loaded:
        log.info("Using gradient sky fallback")
        nodes.clear()
        
        # Create sky gradient matching Three.js: scene.background = new THREE.Color(0x7ab3ff)
        background = nodes.new('ShaderNodeBackground')
        background.location = (0, 300)
        background.inputs['Color'].default_value = (0.48, 0.70, 1.0, 1.0)  # #7AB3FF
        background.inputs['Strength'].default_value = 1.0
        
        output = nodes.new('ShaderNodeOutputWorld')
        output.location = (200, 300)
        
        links.new(background.outputs['Background'], output.inputs['Surface'])
    
    # =========================================================================
    # STEP 7: Final adjustments
    # =========================================================================
    
    # Set viewport shading to Material Preview for better visualization
    try:
        for area in bpy.context.screen.areas:
            if area.type == 'VIEW_3D':
                for space in area.spaces:
                    if space.type == 'VIEW_3D':
                        space.shading.type = 'MATERIAL'
                        if hasattr(space.shading, 'use_scene_lights'):
                            space.shading.use_scene_lights = True
                        if hasattr(space.shading, 'use_scene_world'):
                            space.shading.use_scene_world = True
                        break
    except Exception as e:
        log.debug(f"Could not set viewport shading: {e}")
    
    # Enable Cycles for better rendering (optional)
    try:
        bpy.context.scene.render.engine = 'CYCLES'
        # Try to use GPU if available
        if hasattr(bpy.context.scene, 'cycles'):
            bpy.context.scene.cycles.device = 'GPU'
    except Exception as e:
        log.debug(f"Could not set Cycles renderer: {e}")
        # Fall back to EEVEE
        try:
            bpy.context.scene.render.engine = 'BLENDER_EEVEE'
        except:
            pass
    
    # =========================================================================
    # STEP 8: Zoom viewport to fit scene
    # =========================================================================
    try:
        # Select the car to focus on it
        if car:
            bpy.ops.object.select_all(action='DESELECT')
            car.select_set(True)
            bpy.context.view_layer.objects.active = car
        
        # Zoom to fit selected/all objects
        for area in bpy.context.screen.areas:
            if area.type == 'VIEW_3D':
                for region in area.regions:
                    if region.type == 'WINDOW':
                        override = {'area': area, 'region': region}
                        with bpy.context.temp_override(**override):
                            if car:
                                bpy.ops.view3d.view_selected()
                            else:
                                bpy.ops.view3d.view_all()
                        break
                break
        log.info("✓ Viewport zoomed to fit scene")
    except Exception as e:
        log.debug(f"Could not zoom viewport: {e}")
    
    log.info("=" * 50)
    log.info("✓ Scene generation complete!")
    log.info("  The scene now matches final-demo.html")
    log.info("=" * 50)
    log.info("")
    log.info("Scene contents:")
    log.info(f"  - Car: {'Loaded from GLB' if car else 'Procedural'}")
    log.info(f"  - Terrain: {'Yes' if terrain else 'No'}")
    log.info(f"  - Trees: {tree_count if include_trees else 0}")
    log.info(f"  - HDR Environment: {'Yes' if hdr_loaded else 'Gradient sky'}")
    log.info("")
    
    return car


def _duplicate_hierarchy(obj, new_name):
    """
    Duplicate an object and all its children (deep copy).
    Used for instancing trees from the template.
    """
    ensure_blender()
    
    # Create copy of the object
    new_obj = obj.copy()
    if obj.data:
        # Use linked data for performance (instances share mesh data)
        # Change to obj.data.copy() for independent copies
        new_obj.data = obj.data  # Linked copy (faster, less memory)
    new_obj.name = new_name
    
    # Link to scene collection
    bpy.context.collection.objects.link(new_obj)
    
    # Recursively duplicate children
    for child in obj.children:
        child_copy = _duplicate_hierarchy(child, f"{new_name}_{child.name}")
        child_copy.parent = new_obj
        # Preserve relative transform
        child_copy.matrix_parent_inverse = child.matrix_parent_inverse.copy()
    
    return new_obj

# ============================================================================
# PIPELINE FUNCTIONS (Blender <-> CarX)
# ============================================================================

def sync_to_carx(output_dir: Optional[str] = None, export_full_scene: bool = False) -> bool:
    """
    Export current scene to CarX preview directory with proper consistency.
    
    IMPORTANT: This exports to preview/ directory (NOT preview/assets/)
    to match final-demo.html which loads from preview/lowpoly_game_model.glb
    
    Args:
        output_dir: Output directory (defaults to preview/)
        export_full_scene: If True, exports entire scene as scene.glb
    """
    ensure_blender()
    
    if output_dir is None:
        output_dir = str(get_preview_dir())
    
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    success = True
    
    # Export car model to preview/lowpoly_game_model.glb (matches final-demo.html)
    car_path = os.path.join(output_dir, "lowpoly_game_model.glb")
    if not export_car(car_path, apply_rotation=False):
        log.warn("Car export failed or no car found")
        success = False
    else:
        log.info(f"✓ Exported car: {car_path}")
        log.info("  → Coordinate system: Y-up (Blender) = Y-up (Three.js) ✓")
        log.info("  → No rotation applied - direct export")
    
    # Export full scene if requested
    if export_full_scene:
        scene_path = os.path.join(output_dir, "scene.glb")
        if export_gltf(scene_path):
            log.info(f"✓ Exported full scene: {scene_path}")
        else:
            success = False
    
    # Export scene metadata JSON
    json_path = os.path.join(output_dir, "scene_data.json")
    if not export_scene_json(json_path):
        log.warn("Scene JSON export failed")
        success = False
    else:
        log.info(f"✓ Exported metadata: {json_path}")
    
    if success:
        log.info(f"✓ Successfully synced to CarX: {output_dir}")
        log.info("  → Ready to view in final-demo.html")
    else:
        log.warn(f"⚠ Sync completed with warnings: {output_dir}")
    
    return success

def sync_from_carx(input_dir: Optional[str] = None) -> bool:
    """Import CarX assets into Blender from preview/ directory."""
    ensure_blender()
    
    if input_dir is None:
        input_dir = str(get_preview_dir())
    
    # Look for GLB files
    glb_files = list(Path(input_dir).glob("*.glb"))
    
    if not glb_files:
        log.warn(f"No GLB files found in: {input_dir}")
        return False
    
    for glb in glb_files:
        log.info(f"Importing: {glb.name}")
        import_gltf(str(glb))
    
    log.info(f"Synced from CarX: {len(glb_files)} files")
    return True


def verify_pipeline_consistency() -> Dict[str, Any]:
    """
    Verify that Blender scene matches final-demo.html expectations.
    Returns dict with verification results.
    """
    ensure_blender()
    
    results = {
        "valid": True,
        "errors": [],
        "warnings": [],
        "info": []
    }
    
    # Check if Car object exists
    car = bpy.data.objects.get("Car")
    if not car:
        results["valid"] = False
        results["errors"].append("No 'Car' object found in scene")
        return results
    
    results["info"].append(f"✓ Car object found: {car.name}")
    
    # Check coordinate system
    results["info"].append("✓ Coordinate system: Y-up (matches Three.js)")
    
    # Check car rotation (should match Three.js: -90° on Y axis)
    if hasattr(car, 'rotation_euler'):
        rot = car.rotation_euler
        expected_y_rot = -math.pi / 2  # -90 degrees
        # Allow for the expected -90° Y rotation from Three.js transform
        y_diff = abs(rot.y - expected_y_rot)
        if abs(rot.x) > 0.1 or (y_diff > 0.1 and abs(rot.y) > 0.1) or abs(rot.z) > 0.1:
            results["warnings"].append(
                f"Car has non-zero rotation: ({rot.x:.2f}, {rot.y:.2f}, {rot.z:.2f})"
            )
            results["warnings"].append("  → This may cause orientation mismatch in Three.js")
        else:
            results["info"].append("✓ Car rotation is identity (correct)")
    
    # Check car position
    if hasattr(car, 'location'):
        pos = car.location
        results["info"].append(f"  Car position: ({pos.x:.2f}, {pos.y:.2f}, {pos.z:.2f})")
        
        # Check if car is at reasonable height
        if pos.y < 0:
            results["warnings"].append(f"Car Y position is negative: {pos.y:.2f}")
        elif pos.y > 10:
            results["warnings"].append(f"Car Y position is very high: {pos.y:.2f}")
    
    # Check for wheels
    wheels = [obj for obj in car.children_recursive if 'wheel' in obj.name.lower()]
    if wheels:
        results["info"].append(f"✓ Found {len(wheels)} wheel objects")
    else:
        results["warnings"].append("No wheel objects found (expected 4)")
    
    # Check export path
    preview_dir = get_preview_dir()
    export_path = preview_dir / "lowpoly_game_model.glb"
    results["info"].append(f"  Export target: {export_path}")
    
    if export_path.exists():
        size_mb = export_path.stat().st_size / (1024 * 1024)
        results["info"].append(f"  Existing GLB size: {size_mb:.2f} MB")
    
    # Summary
    if results["errors"]:
        results["valid"] = False
        log.error("Pipeline verification FAILED:")
        for err in results["errors"]:
            log.error(f"  ❌ {err}")
    
    if results["warnings"]:
        log.warn("Pipeline verification has warnings:")
        for warn in results["warnings"]:
            log.warn(f"  ⚠️  {warn}")
    
    if results["valid"] and not results["warnings"]:
        log.info("✓ Pipeline verification PASSED")
    
    for info in results["info"]:
        log.info(f"  {info}")
    
    return results


# ============================================================================
# BLENDER UI OPERATORS (Version-Safe Registration)
# ============================================================================

if IN_BLENDER:
    
    class CARX_OT_GenerateScene(bpy.types.Operator):
        """Generate CarX Simulator Scene (matches final-demo.html)"""
        bl_idname = "carx.generate_scene"
        bl_label = "Generate CarX Scene"
        bl_options = {'REGISTER', 'UNDO'}
        
        include_terrain: bpy.props.BoolProperty(name="Terrain", default=True, 
            description="Create terrain with hills matching Three.js")
        include_trees: bpy.props.BoolProperty(name="Trees", default=True,
            description="Add trees from tree.glb model")
        tree_count: bpy.props.IntProperty(name="Tree Count", default=50, min=0, max=200,
            description="Number of trees to spawn (50 matches final-demo.html)")
        
        def execute(self, context):
            generate_scene(
                include_terrain=self.include_terrain, 
                include_trees=self.include_trees, 
                tree_count=self.tree_count,
                setup_camera=True,
                setup_lighting=True,
                use_existing_models=True
            )
            return {'FINISHED'}
        
        def invoke(self, context, event):
            return context.window_manager.invoke_props_dialog(self)
    
    class CARX_OT_GenerateCar(bpy.types.Operator):
        """Generate CarX Car Model Only"""
        bl_idname = "carx.generate_car"
        bl_label = "Generate Car Only"
        bl_options = {'REGISTER', 'UNDO'}
        
        def execute(self, context):
            clear_scene()
            create_car_model()
            return {'FINISHED'}
    
    class CARX_OT_ExportGLB(bpy.types.Operator):
        """Export Scene to GLB"""
        bl_idname = "carx.export_glb"
        bl_label = "Export GLB"
        
        filepath: bpy.props.StringProperty(subtype='FILE_PATH', default="carx_scene.glb")
        filter_glob: bpy.props.StringProperty(default="*.glb;*.gltf", options={'HIDDEN'})
        
        def execute(self, context):
            export_gltf(self.filepath)
            return {'FINISHED'}
        
        def invoke(self, context, event):
            self.filepath = "carx_scene.glb"
            context.window_manager.fileselect_add(self)
            return {'RUNNING_MODAL'}
    
    class CARX_OT_ExportCar(bpy.types.Operator):
        """Export Car Model to GLB"""
        bl_idname = "carx.export_car"
        bl_label = "Export Car GLB"
        
        filepath: bpy.props.StringProperty(subtype='FILE_PATH', default="car_model.glb")
        filter_glob: bpy.props.StringProperty(default="*.glb;*.gltf", options={'HIDDEN'})
        
        def execute(self, context):
            export_car(self.filepath)
            return {'FINISHED'}
        
        def invoke(self, context, event):
            self.filepath = "car_model.glb"
            context.window_manager.fileselect_add(self)
            return {'RUNNING_MODAL'}
    
    class CARX_OT_ImportGLB(bpy.types.Operator):
        """Import GLB File"""
        bl_idname = "carx.import_glb"
        bl_label = "Import GLB"
        
        filepath: bpy.props.StringProperty(subtype='FILE_PATH')
        filter_glob: bpy.props.StringProperty(default="*.glb;*.gltf", options={'HIDDEN'})
        
        def execute(self, context):
            import_gltf(self.filepath)
            return {'FINISHED'}
        
        def invoke(self, context, event):
            context.window_manager.fileselect_add(self)
            return {'RUNNING_MODAL'}
    
    class CARX_OT_ImportAny(bpy.types.Operator):
        """Import Any 3D Format (FBX, OBJ, DAE, glTF, etc.)"""
        bl_idname = "carx.import_any"
        bl_label = "Import 3D File"
        
        filepath: bpy.props.StringProperty(subtype='FILE_PATH')
        filter_glob: bpy.props.StringProperty(
            default="*.glb;*.gltf;*.fbx;*.obj;*.dae;*.stl;*.ply;*.abc",
            options={'HIDDEN'}
        )
        apply_transform: bpy.props.BoolProperty(name="Apply Transform", default=True)
        
        def execute(self, context):
            import_any_format(self.filepath, self.apply_transform)
            return {'FINISHED'}
        
        def invoke(self, context, event):
            context.window_manager.fileselect_add(self)
            return {'RUNNING_MODAL'}
    
    class CARX_OT_ImportCarModel(bpy.types.Operator):
        """Import a specific car model from the preview directory"""
        bl_idname = "carx.import_car_model"
        bl_label = "Import Car Model"
        bl_options = {'REGISTER', 'UNDO'}
        
        car_model: bpy.props.EnumProperty(
            name="Car Model",
            description="Select car model to import",
            items=[
                ('sports_car', 'Sports Car', 'Default low-poly sports car'),
                ('bmw_e30', 'BMW E30 (1985)', 'Classic BMW E30 coupe'),
                ('chevy_c10', 'Chevrolet C10 (1963)', 'Classic Chevy pickup truck'),
            ],
            default='sports_car'
        )
        replace_existing: bpy.props.BoolProperty(
            name="Replace Existing Car",
            description="Remove existing car before importing",
            default=True
        )
        
        def execute(self, context):
            preview_dir = get_preview_dir()
            car_models = CONFIG.get("car_models", {})
            
            model_config = car_models.get(self.car_model)
            if not model_config:
                self.report({'ERROR'}, f"Unknown car model: {self.car_model}")
                return {'CANCELLED'}
            
            glb_path = preview_dir / model_config["filename"]
            
            if not glb_path.exists():
                self.report({'ERROR'}, f"Car model not found: {glb_path}")
                return {'CANCELLED'}
            
            # Remove existing car if requested
            if self.replace_existing:
                old_car = bpy.data.objects.get("Car")
                if old_car:
                    for child in list(old_car.children_recursive):
                        bpy.data.objects.remove(child, do_unlink=True)
                    bpy.data.objects.remove(old_car, do_unlink=True)
                    log.info("Removed existing car")
            
            # Import the new car model
            log.info(f"Importing car model: {model_config['name']}")
            imported = import_any_format(str(glb_path), apply_transform=False)
            
            if imported:
                # Find root and rename to "Car"
                root = None
                for obj in imported:
                    if obj.parent is None:
                        root = obj
                        break
                if not root and imported:
                    root = imported[0]
                
                if root:
                    root.name = "Car"
                    # Apply model-specific scale
                    scale = model_config.get("scale", 1.0)
                    root.scale = (scale, scale, scale)
                    # Apply rotation offset
                    rot_offset = model_config.get("rotation_offset", 0)
                    root.rotation_euler.z = rot_offset
                    
                    log.info(f"✓ Imported {model_config['name']}")
                    self.report({'INFO'}, f"Imported {model_config['name']}")
            else:
                self.report({'WARNING'}, "Import completed but no objects found")
            
            return {'FINISHED'}
        
        def invoke(self, context, event):
            return context.window_manager.invoke_props_dialog(self)
    
    class CARX_OT_UpdateFromExternal(bpy.types.Operator):
        """Update Car from External File (replaces existing)"""
        bl_idname = "carx.update_from_external"
        bl_label = "Update Car from File"
        
        filepath: bpy.props.StringProperty(subtype='FILE_PATH')
        filter_glob: bpy.props.StringProperty(
            default="*.glb;*.gltf;*.fbx;*.obj;*.dae",
            options={'HIDDEN'}
        )
        replace_car: bpy.props.BoolProperty(name="Replace Existing Car", default=True)
        
        def execute(self, context):
            update_scene_from_external(self.filepath, self.replace_car)
            return {'FINISHED'}
        
        def invoke(self, context, event):
            context.window_manager.fileselect_add(self)
            return {'RUNNING_MODAL'}
    
    class CARX_OT_ConvertToGLB(bpy.types.Operator):
        """Convert Any 3D File to CarX GLB"""
        bl_idname = "carx.convert_to_glb"
        bl_label = "Convert to GLB"
        
        filepath: bpy.props.StringProperty(subtype='FILE_PATH')
        filter_glob: bpy.props.StringProperty(
            default="*.fbx;*.obj;*.dae;*.stl",
            options={'HIDDEN'}
        )
        
        def execute(self, context):
            output = str(Path(self.filepath).with_suffix('.glb'))
            convert_to_carx_format(self.filepath, output)
            return {'FINISHED'}
        
        def invoke(self, context, event):
            context.window_manager.fileselect_add(self)
            return {'RUNNING_MODAL'}
    
    class CARX_OT_ExportWithAnim(bpy.types.Operator):
        """Export Scene with Animations"""
        bl_idname = "carx.export_with_anim"
        bl_label = "Export with Animations"
        
        filepath: bpy.props.StringProperty(subtype='FILE_PATH', default="animated_scene.glb")
        filter_glob: bpy.props.StringProperty(default="*.glb;*.gltf", options={'HIDDEN'})
        
        def execute(self, context):
            export_with_animations(self.filepath)
            return {'FINISHED'}
        
        def invoke(self, context, event):
            self.filepath = "animated_scene.glb"
            context.window_manager.fileselect_add(self)
            return {'RUNNING_MODAL'}
    
    class CARX_OT_CreateWheelAnim(bpy.types.Operator):
        """Create Wheel Spin Animation"""
        bl_idname = "carx.create_wheel_anim"
        bl_label = "Create Wheel Animation"
        bl_options = {'REGISTER', 'UNDO'}
        
        duration: bpy.props.FloatProperty(name="Duration (sec)", default=2.0, min=0.1, max=10.0)
        rotations: bpy.props.FloatProperty(name="Rotations", default=4.0, min=0.5, max=20.0)
        
        def execute(self, context):
            car = bpy.data.objects.get("Car")
            if car:
                create_wheel_spin_animation(car, self.duration, self.rotations)
            else:
                self.report({'ERROR'}, "No 'Car' object found")
            return {'FINISHED'}
        
        def invoke(self, context, event):
            return context.window_manager.invoke_props_dialog(self)
    
    class CARX_OT_SyncToCarX(bpy.types.Operator):
        """Sync Scene to CarX Preview"""
        bl_idname = "carx.sync_to_carx"
        bl_label = "Sync to CarX"
        
        def execute(self, context):
            sync_to_carx()
            return {'FINISHED'}
    
    class CARX_OT_SyncFromCarX(bpy.types.Operator):
        """Import CarX Assets"""
        bl_idname = "carx.sync_from_carx"
        bl_label = "Sync from CarX"
        
        def execute(self, context):
            sync_from_carx()
            return {'FINISHED'}
    
    class CARX_OT_VerifyPipeline(bpy.types.Operator):
        """Verify Pipeline Consistency"""
        bl_idname = "carx.verify_pipeline"
        bl_label = "Verify Pipeline"
        
        def execute(self, context):
            results = verify_pipeline_consistency()
            if results["valid"]:
                self.report({'INFO'}, "Pipeline verification passed!")
            else:
                self.report({'ERROR'}, "Pipeline verification failed - check console")
            return {'FINISHED'}
    
    class CARX_PT_MainPanel(bpy.types.Panel):
        """CarX Tools Panel"""
        bl_label = "CarX Simulator"
        bl_idname = "CARX_PT_main"
        bl_space_type = 'VIEW_3D'
        bl_region_type = 'UI'
        bl_category = 'CarX'
        
        def draw(self, context):
            layout = self.layout
            
            # Info
            box = layout.box()
            box.label(text=f"Blender {BLENDER_VERSION[0]}.{BLENDER_VERSION[1]}", icon='BLENDER')
            box.label(text=f"Tools v{CONFIG['version']}")
            box.label(text="Matches final-demo.html", icon='CHECKMARK')
            
            # Generate - Main action
            layout.label(text="Generate Scene:", icon='SCENE_DATA')
            col = layout.column(align=True)
            col.scale_y = 1.5
            col.operator("carx.generate_scene", icon='PLAY', text="Generate CarX Scene")
            
            # Info about what it does
            info_box = layout.box()
            info_box.scale_y = 0.8
            info_box.label(text="Loads from preview/:", icon='INFO')
            info_box.label(text="  • lowpoly_game_model.glb")
            info_box.label(text="  • tree.glb")
            info_box.label(text="  • farmland_overcast_1k.exr")
            
            layout.separator()
            
            # Procedural fallback
            layout.label(text="Procedural:", icon='MESH_DATA')
            layout.operator("carx.generate_car", icon='AUTO', text="Procedural Car Only")
            
            layout.separator()
            
            # Export
            layout.label(text="Export:", icon='EXPORT')
            layout.operator("carx.export_glb", icon='FILE_3D')
            layout.operator("carx.export_car", icon='AUTO')
            layout.operator("carx.export_with_anim", icon='ACTION')
            
            layout.separator()
            
            # Import
            layout.label(text="Import:", icon='IMPORT')
            layout.operator("carx.import_car_model", icon='AUTO', text="Select Car Model")
            layout.operator("carx.import_any", icon='FILE_3D', text="Import Any Format")
            layout.operator("carx.update_from_external", icon='FILE_REFRESH', text="Update Car")
            layout.operator("carx.convert_to_glb", icon='EXPORT', text="Convert to GLB")
            
            layout.separator()
            
            # Animation
            layout.label(text="Animation:", icon='ACTION')
            layout.operator("carx.create_wheel_anim", icon='DRIVER_ROTATIONAL_DIFFERENCE')
            
            layout.separator()
            
            # Sync
            layout.label(text="Pipeline:", icon='FILE_REFRESH')
            row = layout.row()
            row.operator("carx.sync_to_carx", icon='EXPORT')
            row.operator("carx.sync_from_carx", icon='IMPORT')
            layout.operator("carx.verify_pipeline", icon='CHECKMARK', text="Verify")
    
    # Registration
    _classes = [
        CARX_OT_GenerateScene,
        CARX_OT_GenerateCar,
        CARX_OT_ExportGLB,
        CARX_OT_ExportCar,
        CARX_OT_ImportGLB,
        CARX_OT_ImportAny,
        CARX_OT_ImportCarModel,
        CARX_OT_UpdateFromExternal,
        CARX_OT_ConvertToGLB,
        CARX_OT_ExportWithAnim,
        CARX_OT_CreateWheelAnim,
        CARX_OT_SyncToCarX,
        CARX_OT_SyncFromCarX,
        CARX_OT_VerifyPipeline,
        CARX_PT_MainPanel,
    ]
    
    def register():
        for cls in _classes:
            try:
                bpy.utils.register_class(cls)
            except ValueError:
                pass  # Already registered
    
    def unregister():
        for cls in reversed(_classes):
            try:
                bpy.utils.unregister_class(cls)
            except RuntimeError:
                pass


# ============================================================================
# COMMAND LINE INTERFACE
# ============================================================================

HELP_TEXT = """
CarX Simulator - Blender Scene Tools v{version}
================================================
Blender {blender_ver} | Python {python_ver}

USAGE:
  blender --background --python blender_scene_tools.py -- [OPTIONS]
  blender --python blender_scene_tools.py -- [OPTIONS]

OPTIONS:
  --help, -h              Show this help
  --version               Show version info
  --verbose               Enable debug logging
  
  GENERATION:
  --generate              Generate full scene (car + terrain + trees)
  --generate-car          Generate car model only
  --use-existing          Load existing GLB models from preview/ if available
  --no-terrain            Skip terrain when generating
  --no-trees              Skip trees when generating
  --trees N               Number of trees (default: 20)
  
  EXPORT:
  --export FILE           Export scene to GLB/glTF
  --export-car FILE       Export car only to GLB
  --export-anim FILE      Export with animations
  --export-json FILE      Export scene metadata as JSON
  
  IMPORT (supports GLB, FBX, OBJ, DAE, STL, PLY):
  --import FILE           Import any 3D file
  --update-scene          Replace existing car with imported
  --convert FILE          Convert any format to GLB
  
  ANIMATION:
  --wheel-anim            Create wheel spin animation
  --anim-duration N       Animation duration in seconds (default: 2.0)
  --anim-rotations N      Number of wheel rotations (default: 4.0)
  
  PIPELINE:
  --sync-to-carx          Export to CarX preview/ directory
  --sync-from-carx        Import from CarX preview/ directory
  --verify                Verify pipeline consistency
  --test-assets           Test asset file accessibility (debug)
  --clear                 Clear scene before operations

COORDINATE SYSTEM:
  Both Blender and Three.js use Y-up coordinate system.
  No rotation is applied during export/import.
  Models are exported as-is with identity rotation.

SUPPORTED FORMATS:
  Import: .glb .gltf .fbx .obj .dae .stl .ply .abc
  Export: .glb .gltf (with animations)

EXAMPLES:
  # Generate and export full scene
  blender -b --python blender_scene_tools.py -- --generate --export scene.glb
  
  # Generate car only and export
  blender -b --python blender_scene_tools.py -- --generate-car --export-car car.glb
  
  # Import FBX and convert to GLB
  blender -b --python blender_scene_tools.py -- --convert model.fbx
  
  # Import OBJ and update car in scene
  blender --python blender_scene_tools.py -- --import car.obj --update-scene
  
  # Export with wheel animation
  blender -b --python blender_scene_tools.py -- --generate --wheel-anim --export-anim animated.glb
  
  # Sync workflow (recommended)
  blender -b --python blender_scene_tools.py -- --generate --use-existing --sync-to-carx
  
  # Verify pipeline consistency
  blender -b --python blender_scene_tools.py -- --verify
  
  # Load existing model and re-export
  blender -b --python blender_scene_tools.py -- --clear --use-existing --sync-to-carx
  
  # Interactive mode (opens Blender with CarX panel)
  blender --python blender_scene_tools.py
"""

def print_help():
    info = get_blender_info() if IN_BLENDER else {"version": "N/A"}
    print(HELP_TEXT.format(
        version=CONFIG["version"],
        blender_ver=info.get("version", "N/A"),
        python_ver=f"{sys.version_info.major}.{sys.version_info.minor}"
    ))

def print_version():
    print(f"CarX Blender Tools v{CONFIG['version']}")
    if IN_BLENDER:
        info = get_blender_info()
        print(f"Blender: {info['version']}")
        print(f"glTF Export: {'Available' if info['gltf_available'] else 'Not Available'}")

def main():
    """CLI entry point."""
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []
    
    # Help/Version
    if not argv or "--help" in argv or "-h" in argv:
        print_help()
        return
    
    if "--version" in argv:
        print_version()
        return
    
    # Logging
    if "--verbose" in argv:
        Logger.set_level("DEBUG")
    
    # Clear
    if "--clear" in argv:
        clear_scene()
    
    # Generation options
    include_terrain = "--no-terrain" not in argv
    include_trees = "--no-trees" not in argv
    tree_count = 20
    if "--trees" in argv:
        idx = argv.index("--trees")
        if idx + 1 < len(argv):
            tree_count = int(argv[idx + 1])
    
    # Use existing models flag
    use_existing = "--use-existing" in argv or "--load-existing" in argv
    
    # Generate
    if "--generate" in argv:
        generate_scene(include_terrain, include_trees, tree_count, use_existing_models=use_existing)
    
    if "--generate-car" in argv:
        if "--generate" not in argv:
            clear_scene()
        
        # Try to load existing model first if flag is set
        if use_existing:
            existing = load_existing_models()
            if 'car' not in existing:
                create_car_model()
        else:
            create_car_model()
    
    # Convert (FBX/OBJ/etc to GLB)
    if "--convert" in argv:
        idx = argv.index("--convert")
        if idx + 1 < len(argv):
            input_path = argv[idx + 1]
            output_path = str(Path(input_path).with_suffix('.glb'))
            convert_to_carx_format(input_path, output_path)
    
    # Import (any format)
    if "--import" in argv:
        idx = argv.index("--import")
        if idx + 1 < len(argv):
            filepath = argv[idx + 1]
            if "--update-scene" in argv:
                update_scene_from_external(filepath)
            else:
                import_any_format(filepath)
    
    # Animation
    if "--wheel-anim" in argv:
        car = bpy.data.objects.get("Car")
        if car:
            duration = 2.0
            rotations = 4.0
            if "--anim-duration" in argv:
                idx = argv.index("--anim-duration")
                if idx + 1 < len(argv):
                    duration = float(argv[idx + 1])
            if "--anim-rotations" in argv:
                idx = argv.index("--anim-rotations")
                if idx + 1 < len(argv):
                    rotations = float(argv[idx + 1])
            create_wheel_spin_animation(car, duration, rotations)
        else:
            log.error("No 'Car' object found for animation")
    
    # Export
    if "--export" in argv:
        idx = argv.index("--export")
        filepath = argv[idx + 1] if idx + 1 < len(argv) else "carx_scene.glb"
        export_gltf(filepath)
    
    if "--export-car" in argv:
        idx = argv.index("--export-car")
        filepath = argv[idx + 1] if idx + 1 < len(argv) else "car_model.glb"
        export_car(filepath)
    
    if "--export-anim" in argv:
        idx = argv.index("--export-anim")
        filepath = argv[idx + 1] if idx + 1 < len(argv) else "animated_scene.glb"
        export_with_animations(filepath)
    
    if "--export-json" in argv:
        idx = argv.index("--export-json")
        filepath = argv[idx + 1] if idx + 1 < len(argv) else "scene_data.json"
        export_scene_json(filepath)
    
    # Pipeline
    if "--sync-to-carx" in argv:
        sync_to_carx()
    
    if "--sync-from-carx" in argv:
        sync_from_carx()
    
    if "--verify" in argv:
        verify_pipeline_consistency()
    
    # Test assets command - verify all files are accessible
    if "--test-assets" in argv:
        test_assets()

def test_assets():
    """Test function to verify assets are accessible from Blender."""
    print("=" * 60)
    print("CARX ASSET VERIFICATION TEST")
    print("=" * 60)
    
    # Show environment info
    print(f"\nEnvironment:")
    print(f"  IN_BLENDER: {IN_BLENDER}")
    print(f"  CWD: {Path.cwd()}")
    print(f"  sys.argv: {sys.argv[:5]}...")  # First 5 args
    
    # Get script directory
    script_dir = get_script_dir()
    print(f"\nScript directory: {script_dir}")
    print(f"  Absolute: {script_dir.resolve()}")
    print(f"  Exists: {script_dir.exists()}")
    
    # Get preview directory
    preview_dir = get_preview_dir()
    print(f"\nPreview directory: {preview_dir}")
    print(f"  Absolute: {preview_dir.resolve()}")
    print(f"  Exists: {preview_dir.exists()}")
    
    if preview_dir.exists():
        print("\nDirectory contents:")
        try:
            for item in sorted(preview_dir.iterdir()):
                if item.is_file():
                    size = item.stat().st_size
                    print(f"  📄 {item.name} ({size:,} bytes)")
                else:
                    print(f"  📁 {item.name}/")
        except Exception as e:
            print(f"  Error listing: {e}")
    
    # Test GLB file
    car_glb = preview_dir / "lowpoly_game_model.glb"
    print(f"\nCar GLB: {car_glb}")
    print(f"  Exists: {car_glb.exists()}")
    if car_glb.exists():
        size = car_glb.stat().st_size
        print(f"  Size: {size:,} bytes ({size/1024/1024:.2f} MB)")
        # Check GLB header
        try:
            with open(car_glb, 'rb') as f:
                magic = f.read(4)
                print(f"  Magic: {magic} (should be b'glTF')")
                if magic == b'glTF':
                    print(f"  ✓ Valid GLB file")
                else:
                    print(f"  ✗ Invalid GLB file!")
        except Exception as e:
            print(f"  Error reading: {e}")
    
    # Test EXR file
    hdr_exr = preview_dir / "farmland_overcast_1k.exr"
    print(f"\nHDR EXR: {hdr_exr}")
    print(f"  Exists: {hdr_exr.exists()}")
    if hdr_exr.exists():
        size = hdr_exr.stat().st_size
        print(f"  Size: {size:,} bytes ({size/1024/1024:.2f} MB)")
    
    # Test tree GLB
    tree_glb = preview_dir / "tree.glb"
    print(f"\nTree GLB: {tree_glb}")
    print(f"  Exists: {tree_glb.exists()}")
    if tree_glb.exists():
        size = tree_glb.stat().st_size
        print(f"  Size: {size:,} bytes ({size/1024/1024:.2f} MB)")
    
    # Check glTF importer availability
    if IN_BLENDER:
        print(f"\nBlender glTF importer:")
        has_gltf = hasattr(bpy.ops.import_scene, 'gltf')
        print(f"  Available: {has_gltf}")
        if not has_gltf:
            print(f"  ✗ Enable 'Import-Export: glTF 2.0 format' addon!")
    
    print("\n" + "=" * 60)

# ============================================================================
# ENTRY POINT
# ============================================================================

if __name__ == "__main__":
    if IN_BLENDER:
        try:
            register()
            log.info(f"CarX Tools v{CONFIG['version']} loaded for Blender {BLENDER_VERSION[0]}.{BLENDER_VERSION[1]}")
        except Exception as e:
            log.warn(f"UI registration: {e}")
        
        if "--" in sys.argv:
            main()
        else:
            print("\n" + "="*50)
            print("CarX Blender Tools - Interactive Mode")
            print("="*50)
            print("Use the 'CarX' tab in 3D View sidebar (press N)")
            print("Or run with -- --help for CLI options")
            print("="*50 + "\n")
    else:
        print("This script must be run inside Blender.")
        print("Usage: blender --python blender_scene_tools.py -- --help")
        sys.exit(1)
