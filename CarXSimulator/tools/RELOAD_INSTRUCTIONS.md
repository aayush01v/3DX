# How to Reload Script in Blender

## Quick One-Liner (Copy & Paste into Python Console)

```python
exec(open('/home/codespace/Documents/3DX/CarXSimulator/tools/blender_scene_tools.py').read())
```

## Better Reload (Clears old modules first)

```python
import sys; [sys.modules.pop(k) for k in list(sys.modules.keys()) if 'blender_scene_tools' in k]; exec(open('/home/codespace/Documents/3DX/CarXSimulator/tools/blender_scene_tools.py').read())
```

## Create a Reload Function (Paste once, then reuse)

```python
def reload_carx():
    import sys
    [sys.modules.pop(k) for k in list(sys.modules.keys()) if 'blender_scene_tools' in k]
    exec(open('/home/codespace/Documents/3DX/CarXSimulator/tools/blender_scene_tools.py').read())
    print("✓ CarX Tools reloaded!")

# Now just type: reload_carx()
```

## After Reloading

1. Press **N** in the 3D viewport to toggle the sidebar
2. Look for the **CarX** tab
3. If you don't see it, try:
   - Closing and reopening the sidebar (press N twice)
   - Restarting Blender

## Using the CarX Panel

Once loaded, you'll see these buttons in the sidebar:

### Generate:
- **Generate CarX Scene** - Full scene with car, terrain, trees
- **Generate Car Only** - Just the car model

### Export:
- **Export GLB** - Export entire scene
- **Export Car GLB** - Export only the car
- **Export with Animations** - Include animations

### Import:
- **Import Any Format** - Load FBX, OBJ, DAE, etc.
- **Update Car** - Replace existing car with imported model
- **Convert to GLB** - Convert any format to GLB

### Pipeline:
- **Sync to CarX** - Export to preview directory
- **Sync from CarX** - Import from preview directory

## Troubleshooting

### Error: "name '__file__' is not defined"
**Fixed!** The script now handles this automatically by detecting the execution context.

### Panel doesn't appear
Try this in the console:
```python
import bpy
bpy.utils.register_class(CARX_PT_MainPanel)
```

### Need to set working directory manually
```python
import os
os.chdir('/home/codespace/Documents/3DX')
```

Then reload the script.
