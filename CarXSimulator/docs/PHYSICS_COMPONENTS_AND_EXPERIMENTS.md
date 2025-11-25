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

## Using and Extending “Select Physics Experiment”
- The preview UI (`preview/final-demo.html`) exposes a **Select Physics Experiment** dropdown that auto-loads ready-to-run C++ snippets into the code editor and displays a description. Selecting an experiment updates the editor and narrative panel without restarting the simulation.
- To add or modify experiments, edit the `experiments` object near the bottom of `preview/final-demo.html`. Each entry defines `cpp` (the code injected into the editor) and `desc` (the UI label). The handler `loadExperiment()` pulls the chosen snippet into the editor and re-renders the description field, so new entries instantly become selectable.
- Experiments interact with the live car instance (via helpers like `car.setMass`, `car.applyForce`, etc.), letting users observe the same physics stack described above while exploring textbook scenarios.
