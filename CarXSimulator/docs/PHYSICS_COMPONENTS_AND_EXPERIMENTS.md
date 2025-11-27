# Physics Components and Experiments Guide

## Force and Integration Pipeline
- External forces (wind drag, scripted pushes, etc.) are accumulated through `Car::applyForce` before each physics step. They are carried into the solver via `applyForces` before being cleared, ensuring the physics engine and vehicle share the same force budget.
- Ground interaction recalculates grip from tire and terrain friction, then applies longitudinal, lateral, rolling-resistance, aerodynamic, and gravity forces before updating velocity and orientation.

## Driveline and Acceleration
- Engine output uses a mid-range torque peak: a torque multiplier scales with RPM, multiplies the base `enginePower`, and routes through the current gear and final drive with a driveline efficiency factor. Wheel force is `driveTorque / wheelRadius` and respects forward/reverse bias.
- Traction control reads predicted slip (`requestedForce` vs. available tire force) and scales throttle output before clamping the applied drive force to the terrain-limited grip envelope. Wheel slip telemetry is written per wheel for HUD/debug features and ABS logic.

## Lateral Grip and Stability
- The slip angle from lateral vs. forward velocity feeds a Pacejka-inspired curve. Lateral force is limited by the same grip envelope, softened at high slip angles to encourage progressive breakaway instead of instant oversteer.
- Additional lateral damping in `updateKinematics` nudges velocity back toward the heading vector to bleed unrealistic sideways drift while preserving inertia at speed.

## Braking, Handbrake, and ABS
- Braking distributes force front/rear (60/40 bias, shifted rearward when the handbrake is active) and caps per-wheel effort at the available friction. Extra rear weighting for the handbrake increases yaw authority without locking the fronts.
- Predicted per-wheel slip is recorded before ABS; ABS halves brake torque when slip exceeds 0.2 to prevent lockup. The final summed brake force is applied opposite the current velocity vector so stopping remains stable regardless of heading.

## Aerodynamics and Rolling Losses
- Rolling resistance scales with speed and vehicle weight. Aerodynamic drag uses the standard `0.5 * rho * Cd * A * v^2` formulation inside the car to complement the wind-aware drag applied by `PhysicsEngine`, providing stable high-speed damping without double-counting wind effects.

---

## Code Editor API Reference

The simulator includes an interactive code editor that allows you to run physics experiments in real-time. The editor supports C++, Python, and JavaScript syntax.

### Supported Languages
- **C++**: Variable declarations (`float`, `int`, `double`), math functions, physics commands
- **Python**: `print()`, `for` loops, `range()`, `if/elif/else`, functions
- **JavaScript**: Full ES6+, `console.log()`, arrays, objects, async/await

### Car API Commands

#### Setters (modify car state)
| Command | Description | Example |
|---------|-------------|---------|
| `car.setMass(value)` | Set vehicle mass in kg | `car.setMass(1500)` |
| `car.setEnginePower(value)` | Set engine power in Watts | `car.setEnginePower(150000)` |
| `car.setVelocity(Vector3(x, y, z))` | Set velocity in m/s | `car.setVelocity(Vector3(0, 0, 20))` |
| `car.setPosition(Vector3(x, y, z))` | Set position in meters | `car.setPosition(Vector3(0, 10, 0))` |
| `car.setGravity(value)` | Set gravity (negative for downward) | `car.setGravity(-9.81)` |
| `car.applyForce(Vector3(x, y, z))` | Apply force in Newtons | `car.applyForce(Vector3(0, 0, 10000))` |
| `car.drive(throttle, steering, brake)` | Control car directly (-1 to 1) | `car.drive(1, 0.5, 0)` |
| `reset()` | Reset car to starting position | `reset()` |

#### Getters (read car state)
| Command | Description | Returns |
|---------|-------------|---------|
| `car.getSpeed()` | Get current speed | Speed in m/s and km/h |
| `car.getPosition()` | Get current position | (x, y, z) coordinates |
| `car.getVelocity()` | Get velocity vector | (x, y, z) in m/s |
| `car.getGravity()` | Get current gravity | Gravity in m/s² |
| `car.getMass()` | Get vehicle mass | Mass in kg |
| `car.getEnergy()` | Get kinetic & potential energy | Energy in Joules |
| `car.getMomentum()` | Get momentum vector | Momentum in kg⋅m/s |

### Math Functions Available
`sin`, `cos`, `tan`, `sqrt`, `pow`, `abs`, `atan`, `PI`

---

## Physics Experiments (Select from Dropdown)

The preview UI exposes a **Select Physics Experiment** dropdown that auto-loads ready-to-run code snippets into the editor. Click **Execute Code** to run the experiment.


### Kinematics
| Experiment | Key | Description |
|------------|-----|-------------|
| Equations of Motion | `kinematics1` | Demonstrates v = u + at, s = ut + ½at² |
| Relative Velocity | `kinematics2` | Reference frames and relative motion |
| Projectile Motion | `projectile` | Range and maximum height calculations |

### Laws of Motion
| Experiment | Key | Description |
|------------|-----|-------------|
| Newton's Laws | `newton1` | F = ma force analysis |
| Friction | `friction` | Static and kinetic friction (μs, μk) |
| Circular Motion | `circular` | Centripetal acceleration and force |
| Banking of Roads | `banking` | Optimal banking angle for curves |

### Work, Energy & Power
| Experiment | Key | Description |
|------------|-----|-------------|
| Work-Energy Theorem | `work_energy` | W = ΔKE relationship |
| Power | `power` | P = Fv power calculations |
| Spring Force | `spring` | Hooke's Law F = -kx |

### Momentum & Collisions
| Experiment | Key | Description |
|------------|-----|-------------|
| Linear Momentum | `momentum` | p = mv conservation |
| Impulse | `impulse` | J = Δp = FΔt |
| Elastic Collisions | `elasticity` | Perfectly elastic collision formulas |

### Advanced Topics
| Experiment | Key | Description |
|------------|-----|-------------|
| Universal Gravitation | `gravitation` | F = GMm/r², escape velocity |
| Rotational Dynamics | `rotation` | τ = Iα, angular momentum |
| Simple Harmonic Motion | `sho` | x = A·sin(ωt + φ) oscillations |
| Fluid Mechanics | `fluid` | Pressure, buoyancy, Torricelli's theorem |
| Doppler Effect | `doppler` | Frequency shift with motion |

---

## Extending Physics Experiments

To add or modify experiments, edit the `experiments` object in `preview/final-demo.html`:

```javascript
const experiments = {
    myExperiment: {
        cpp: `// Your C++ style code here
float mass = 1200;
car.setMass(mass);
car.getSpeed();`,
        desc: "Description shown in the UI"
    }
};
```

Each entry requires:
- `cpp`: The code snippet loaded into the editor
- `desc`: A brief description displayed below the dropdown

The `loadExperiment()` function handles loading the selected experiment into the editor.

---

## Vehicle Controls

### Keyboard Controls
| Key | Action |
|-----|--------|
| `↑` / `W` | Throttle (accelerate forward) |
| `↓` / `S` | Reverse gear (drive backward) |
| `←` / `A` | Steer left |
| `→` / `D` | Steer right |
| `Space` | Brake (handbrake) |
| `C` | Toggle camera follow |
| `R` | Reset camera |
| `G` | Toggle debug visualization |

### Mouse Controls
| Action | Effect |
|--------|--------|
| Drag | Rotate camera around car |
| Scroll wheel | Zoom in/out |

### Mobile Touch Controls
On-screen buttons appear automatically on mobile devices for throttle, brake, steering, and handbrake.

**Note**: Keyboard controls are disabled while typing in the code editor. Click on the 3D viewport to re-enable car controls.
