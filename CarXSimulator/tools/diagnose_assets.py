#!/usr/bin/env python3
"""
CarX Asset Diagnostic Script
=============================
Run this in Blender to diagnose why GLB/EXR files aren't loading.

Usage:
  blender --background --python diagnose_assets.py
  blender --python diagnose_assets.py
"""

import sys
from pathlib import Path

print("=" * 70)
print("CARX ASSET DIAGNOSTIC")
print("=" * 70)

# Check if running in Blender
try:
    import bpy
    IN_BLENDER = True
    print(f"✓ Running in Blender {bpy.app.version[0]}.{bpy.app.version[1]}.{bpy.app.version[2]}")
except ImportError:
    IN_BLENDER = False
    print("✗ NOT running in Blender - run this script with:")
    print("  blender --background --python diagnose_assets.py")
    sys.exit(1)

print(f"\nCurrent Working Directory: {Path.cwd()}")
print(f"Script arguments: {sys.argv}")

# ============================================================================
# STEP 1: Find preview directory
# ============================================================================
print("\n" + "-" * 70)
print("STEP 1: Finding preview directory")
print("-" * 70)

cwd = Path.cwd()

# Build list of candidate paths
candidates = [
    cwd / "CarXSimulator" / "preview",
    cwd / "preview",
    cwd.parent / "CarXSimulator" / "preview",
    cwd.parent / "preview",
]

# Add paths from __file__ if available
try:
    script_path = Path(__file__).resolve()
    script_dir = script_path.parent
    candidates.insert(0, script_dir.parent / "preview")  # tools/../preview
    candidates.insert(1, script_dir / "preview")
    print(f"Script location: {script_path}")
except NameError:
    print("__file__ not available (running from Blender text editor?)")

# Add paths from sys.argv
for arg in sys.argv:
    if "CarXSimulator" in arg or "blender_scene_tools" in arg:
        try:
            arg_path = Path(arg).resolve()
            if arg_path.is_file():
                arg_path = arg_path.parent
            candidates.insert(0, arg_path.parent / "preview")
            candidates.insert(0, arg_path / "preview")
        except:
            pass

# Remove duplicates while preserving order
seen = set()
unique_candidates = []
for c in candidates:
    try:
        resolved = c.resolve()
        if resolved not in seen:
            seen.add(resolved)
            unique_candidates.append(resolved)
    except:
        pass

print(f"\nChecking {len(unique_candidates)} candidate paths:")
preview_dir = None
for i, path in enumerate(unique_candidates):
    exists = path.exists()
    has_glb = (path / "lowpoly_game_model.glb").exists() if exists else False
    has_exr = (path / "farmland_overcast_1k.exr").exists() if exists else False
    
    status = "✗"
    if exists and has_glb and has_exr:
        status = "✓✓"
        if preview_dir is None:
            preview_dir = path
    elif exists and (has_glb or has_exr):
        status = "✓"
        if preview_dir is None:
            preview_dir = path
    elif exists:
        status = "~"
    
    print(f"  {status} {path}")
    print(f"      exists={exists}, glb={has_glb}, exr={has_exr}")

if preview_dir:
    print(f"\n✓ Using preview directory: {preview_dir}")
else:
    print("\n✗ NO PREVIEW DIRECTORY FOUND!")
    print("  Make sure you run Blender from the project root directory")
    print("  or the CarXSimulator directory")
    sys.exit(1)

# ============================================================================
# STEP 2: Check files
# ============================================================================
print("\n" + "-" * 70)
print("STEP 2: Checking asset files")
print("-" * 70)

# List all files in preview directory
print(f"\nContents of {preview_dir}:")
try:
    for item in sorted(preview_dir.iterdir()):
        if item.is_file():
            size = item.stat().st_size
            print(f"  📄 {item.name:40} {size:>12,} bytes")
        else:
            print(f"  📁 {item.name}/")
except Exception as e:
    print(f"  Error listing directory: {e}")

# Supported filenames (matches CONFIG in blender_scene_tools.py)
CAR_MODEL_NAMES = [
    "lowpoly_game_model.glb",
    "bmw_e30_1985.glb",
    "chevrolet_c10_pickup_1963.glb",
    "car_model.glb",
    "car.glb",
    "vehicle.glb",
]
HDR_NAMES = ["farmland_overcast_1k.exr", "environment.exr", "hdri.exr"]

# Check GLB file - try multiple filenames
car_glb = None
print(f"\n--- Car GLB (checking: {CAR_MODEL_NAMES}) ---")
for glb_name in CAR_MODEL_NAMES:
    candidate = preview_dir / glb_name
    if candidate.exists():
        car_glb = candidate
        print(f"  Found: {glb_name}")
        break
    else:
        print(f"  Not found: {glb_name}")

if car_glb:
    size = car_glb.stat().st_size
    print(f"  Size: {size:,} bytes ({size/1024/1024:.2f} MB)")
    
    if size < 1000:
        print(f"  ⚠ File is very small ({size} bytes) - may be corrupted!")
    
    # Check GLB magic bytes
    try:
        with open(car_glb, 'rb') as f:
            magic = f.read(4)
            version = int.from_bytes(f.read(4), 'little')
            length = int.from_bytes(f.read(4), 'little')
        
        if magic == b'glTF':
            print(f"  ✓ Valid GLB header (magic=glTF, version={version}, length={length})")
            if length != size:
                print(f"  ⚠ Length mismatch: header says {length}, file is {size}")
        else:
            print(f"  ✗ INVALID GLB! Magic bytes: {magic} (expected b'glTF')")
    except Exception as e:
        print(f"  ✗ Error reading GLB header: {e}")
else:
    print(f"  ✗ NO CAR GLB FOUND!")

# Check EXR file - try multiple filenames
hdr_exr = None
print(f"\n--- HDR EXR (checking: {HDR_NAMES}) ---")
for hdr_name in HDR_NAMES:
    candidate = preview_dir / hdr_name
    if candidate.exists():
        hdr_exr = candidate
        print(f"  Found: {hdr_name}")
        break
    else:
        print(f"  Not found: {hdr_name}")

if hdr_exr:
    size = hdr_exr.stat().st_size
    print(f"  Size: {size:,} bytes ({size/1024/1024:.2f} MB)")
    
    # Check EXR magic bytes (0x76, 0x2f, 0x31, 0x01)
    try:
        with open(hdr_exr, 'rb') as f:
            magic = f.read(4)
            # Read more bytes for debugging if needed
            f.seek(0)
            header_bytes = f.read(32)
        
        if magic == b'\x76\x2f\x31\x01':
            print(f"  ✓ Valid EXR header (OpenEXR magic bytes: {magic.hex()})")
        else:
            print(f"  ⚠ Unexpected magic bytes: {magic.hex()} (expected 762f3101)")
            print(f"  First 32 bytes: {header_bytes.hex()}")
    except Exception as e:
        print(f"  ✗ Error reading EXR header: {e}")
else:
    print(f"  ✗ NO HDR EXR FOUND!")

# Check tree GLB
tree_glb = preview_dir / "tree.glb"
print(f"\n--- Tree GLB: {tree_glb.name} ---")
if tree_glb.exists():
    size = tree_glb.stat().st_size
    print(f"  Size: {size:,} bytes ({size/1024/1024:.2f} MB)")
else:
    print(f"  (optional) File not found")

# ============================================================================
# STEP 3: Check Blender addons
# ============================================================================
print("\n" + "-" * 70)
print("STEP 3: Checking Blender addons")
print("-" * 70)

# Check glTF importer
has_gltf_import = hasattr(bpy.ops.import_scene, 'gltf')
print(f"glTF Importer (import_scene.gltf): {'✓ Available' if has_gltf_import else '✗ NOT AVAILABLE'}")

if not has_gltf_import:
    print("\n  To enable glTF addon:")
    print("  1. Open Blender Preferences (Edit > Preferences)")
    print("  2. Go to Add-ons tab")
    print("  3. Search for 'gltf'")
    print("  4. Enable 'Import-Export: glTF 2.0 format'")
    print("\n  Or run this in Blender's Python console:")
    print("    bpy.ops.preferences.addon_enable(module='io_scene_gltf2')")

# Check glTF exporter
has_gltf_export = hasattr(bpy.ops.export_scene, 'gltf')
print(f"glTF Exporter (export_scene.gltf): {'✓ Available' if has_gltf_export else '✗ NOT AVAILABLE'}")

# ============================================================================
# STEP 4: Try importing GLB
# ============================================================================
print("\n" + "-" * 70)
print("STEP 4: Testing GLB import")
print("-" * 70)

if car_glb and car_glb.exists() and has_gltf_import:
    print(f"Attempting to import: {car_glb}")
    
    # Store objects before import
    objects_before = set(bpy.data.objects)
    
    try:
        # Clear selection
        bpy.ops.object.select_all(action='DESELECT')
        
        # Import
        result = bpy.ops.import_scene.gltf(filepath=str(car_glb))
        print(f"  Import result: {result}")
        
        # Find imported objects
        objects_after = set(bpy.data.objects)
        imported = list(objects_after - objects_before)
        
        if not imported:
            imported = list(bpy.context.selected_objects)
        
        print(f"  Imported {len(imported)} objects:")
        for obj in imported:
            print(f"    - {obj.name} (type: {obj.type})")
        
        if imported:
            print("\n  ✓ GLB IMPORT SUCCESSFUL!")
        else:
            print("\n  ⚠ Import completed but no objects found")
            
    except Exception as e:
        print(f"\n  ✗ IMPORT FAILED: {e}")
        import traceback
        traceback.print_exc()
else:
    if not car_glb or not car_glb.exists():
        print("  Skipping: GLB file not found")
    if not has_gltf_import:
        print("  Skipping: glTF importer not available")

# ============================================================================
# STEP 5: Try loading EXR
# ============================================================================
print("\n" + "-" * 70)
print("STEP 5: Testing EXR loading")
print("-" * 70)

if hdr_exr and hdr_exr.exists():
    print(f"Attempting to load: {hdr_exr}")
    
    # Get file metadata for error reporting
    exr_size = hdr_exr.stat().st_size
    try:
        with open(hdr_exr, 'rb') as f:
            exr_header = f.read(32)
    except:
        exr_header = b''
    
    try:
        img = bpy.data.images.load(str(hdr_exr))
        print(f"  ✓ EXR LOADED SUCCESSFULLY!")
        print(f"    Name: {img.name}")
        print(f"    Size: {img.size[0]}x{img.size[1]} pixels")
        print(f"    Format: {img.file_format}")
        print(f"    Color space: {img.colorspace_settings.name}")
        
        # Clean up
        bpy.data.images.remove(img)
        
    except Exception as e:
        print(f"\n  ✗ EXR LOAD FAILED: {e}")
        print(f"    Exception type: {type(e).__name__}")
        print(f"    File path: {hdr_exr}")
        print(f"    File size: {exr_size:,} bytes")
        print(f"    First 32 bytes: {exr_header.hex()}")
        import traceback
        traceback.print_exc()
else:
    print("  Skipping: EXR file not found")

# ============================================================================
# SUMMARY
# ============================================================================
print("\n" + "=" * 70)
print("SUMMARY")
print("=" * 70)

issues = []

if not preview_dir:
    issues.append("Preview directory not found")
if not car_glb or not car_glb.exists():
    issues.append(f"Car GLB file missing (searched: {CAR_MODEL_NAMES})")
if not hdr_exr or not hdr_exr.exists():
    issues.append(f"HDR EXR file missing (searched: {HDR_NAMES})")
if not has_gltf_import:
    issues.append("glTF importer addon not enabled")

if issues:
    print("\n⚠ Issues found:")
    for issue in issues:
        print(f"  - {issue}")
else:
    print("\n✓ All checks passed! Assets should load correctly.")
    print("\nTo generate the scene, run:")
    print(f"  blender --background --python {preview_dir.parent}/tools/blender_scene_tools.py -- --generate --verbose")

print("\n" + "=" * 70)
