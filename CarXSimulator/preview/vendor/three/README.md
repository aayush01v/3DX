# Three.js Vendor Files

This directory contains the Three.js library and its loaders for the CarX Simulator preview.

## Files

- **three.min.js** (590KB) - Three.js r128 core library (UMD build)
- **GLTFLoader.js** (95KB) - GLTF/GLB model loader for loading `lowpoly_game_model.glb`
- **EXRLoader.js** (55KB) - EXR HDR environment loader for loading `farmland_overcast_1k.exr`

## Version

Three.js r128 (v0.128.0)

## Source

Downloaded from: https://cdn.jsdelivr.net/npm/three@0.128.0/

## Usage

These files are loaded by `final-demo.html` with automatic fallback to CDN if local files fail.

The loaders enable:
- Loading the car model (`lowpoly_game_model.glb`)
- Loading HDR environment maps (`farmland_overcast_1k.exr`) in high quality mode

## Format

All files are in UMD (Universal Module Definition) format, which means they:
- Define the global `THREE` object
- Are compatible with script tags
- Work without a module bundler
