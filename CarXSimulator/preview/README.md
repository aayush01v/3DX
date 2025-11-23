# 🎮 CarX Simulator - Web Preview

Interactive 3D physics simulation - no build required!

## 🚀 Quick Launch

### Windows
```bash
:: optional - change port if 8080 is busy
set PORT=9000
START_PREVIEW.bat
```

### Linux/macOS
```bash
# optional - change port if 8080 is busy
PORT=9000 ./START_PREVIEW.sh
```

### Manual
```bash
PORT=9000 python serve.py  # defaults to 8080 if not set
# Open: http://localhost:9000/final-demo.html
```

## 📁 Files

- **final-demo.html** - Main demo (use this!)
- **lowpoly_game_model.glb** - 3D car model
- **serve.py** - Python server script
- **START_PREVIEW.bat** - Windows launcher
- **START_PREVIEW.sh** - Linux/macOS launcher

## 🎮 Controls

### Driving
- `↑` `W` - Throttle
- `↓` `S` - Brake
- `←` `→` `A` `D` - Steer
- `Space` - Handbrake

### Camera
- **Drag** - Rotate around car
- **Wheel** - Zoom in/out
- **C** - Toggle follow mode
- **R** - Reset camera
- **G** - Debug ground contact

### Touch (Mobile)
- **Single finger** - Rotate camera
- **Pinch** - Zoom
- **Double tap** - Reset view

## 🎯 Features

- ✅ Realistic physics simulation
- ✅ Interactive camera controls
- ✅ Multiple terrain types
- ✅ Adjustable gravity
- ✅ Live code editor
- ✅ Force visualization
- ✅ Performance monitoring

## 🔧 Troubleshooting

**Car floating?**
- Press `G` to see ground contact
- See [GROUND_FIX.md](GROUND_FIX.md)

**Camera weird?**
- Press `R` to reset
- See [CAMERA_GUIDE.md](CAMERA_GUIDE.md)

**Low FPS?**
- Change quality to "Low"
- Close other tabs

**Port already in use?**
- Set a custom port: `PORT=9000 ./START_PREVIEW.sh`
- The server will automatically fall back to a free port if the preferred one is busy

**Model not loading?**
- Check browser console
- Fallback car will appear

## 📚 Documentation

- [CAMERA_GUIDE.md](CAMERA_GUIDE.md) - Camera system details
- [GROUND_FIX.md](GROUND_FIX.md) - Physics troubleshooting
- [../README.md](../README.md) - Main documentation

## 🌐 Browser Support

- ✅ Chrome/Edge (recommended)
- ✅ Firefox
- ✅ Safari
- ✅ Mobile browsers

## 🎓 Code Editor

Try these experiments:

```cpp
// Change mass
car.setMass(2000);

// Apply force
car.applyForce(Vector3(0, 0, 5000));

// Moon gravity
car.setGravity(-1.62);

// Get energy
car.getEnergy();
```

---

**Enjoy the simulation!** 🏎️💨
