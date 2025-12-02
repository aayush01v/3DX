"""
Quick reload script for Blender console.
Copy and paste this into Blender's Python Console to reload the CarX tools.
"""

import sys
import importlib
from pathlib import Path

# Set the correct path
script_path = '/home/codespace/Documents/3DX/CarXSimulator/tools/blender_scene_tools.py'

# Remove old module if it exists
module_names = [name for name in sys.modules.keys() if 'blender_scene_tools' in name]
for name in module_names:
    del sys.modules[name]

# Execute the script
print("🔄 Reloading CarX Tools...")
exec(open(script_path).read())
print("✓ CarX Tools reloaded successfully!")
print("📌 Press N in 3D viewport to see the CarX panel")
