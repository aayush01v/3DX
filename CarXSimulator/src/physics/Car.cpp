#include "Car.h"
#include <algorithm>

Car::Car(const Vector3& position) 
    : position(position), velocity(0, 0, 0), acceleration(0, 0, 0),
      rotation(1, 0, 0, 0), angularVelocity(0, 0, 0), totalForce(0, 0, 0),
      mass(1200.0f), inertia(1800.0f), wheelbase(2.5f), trackWidth(1.5f),
      dragCoeff(0.3f), rollingResistance(0.015f), tireGrip(1.0f),
      centerOfMassHeight(0.5f),
      throttle(0), steering(0), brake(0), handbrake(false),
      enginePower(150000.0f), engineRPM(1000.0f), maxRPM(7000.0f), idleRPM(1000.0f),
      currentGear(1), finalDrive(3.5f),
      airborne(false), airborneTime(0.0f),
      maxSpeed(60.0f), maxSteeringAngle(0.6f),
      suspensionStiffness(50000.0f), suspensionDamping(3000.0f),
      suspensionTravel(0.3f), wheelRadius(0.35f),
      differentialType(1), tractionControlEnabled(false), absEnabled(false),
      useAdvancedPhysics(true) {
    
    // Gear ratios (6-speed)
    gearRatios[0] = 3.5f;  // 1st
    gearRatios[1] = 2.5f;  // 2nd
    gearRatios[2] = 1.8f;  // 3rd
    gearRatios[3] = 1.3f;  // 4th
    gearRatios[4] = 1.0f;  // 5th
    gearRatios[5] = 0.8f;  // 6th
    
    for (int i = 0; i < 4; ++i) {
        suspensionCompression[i] = 0.0f;
        wheelSlip[i] = 0.0f;
    }
    
    updateInertia();
}

void Car::update(float deltaTime, const Vector3& terrainNormal, float terrainHeight, float terrainFriction) {
    // Multi-point ground contact (4 wheels)
    float wheelHeights[4];
    for (int i = 0; i < 4; ++i) {
        wheelHeights[i] = terrainHeight; // Use same height for all wheels (simplified)
    }
    
    updateWithWheelHeights(deltaTime, terrainNormal, wheelHeights, terrainFriction);
}

void Car::updateWithWheelHeights(float deltaTime, const Vector3& terrainNormal, const float wheelHeights[4], float terrainFriction) {
    // Update suspension
    updateSuspension(deltaTime, wheelHeights);
    
    // Calculate average wheel height
    float avgWheelHeight = 0.0f;
    for (int i = 0; i < 4; ++i) {
        avgWheelHeight += wheelHeights[i];
    }
    avgWheelHeight /= 4.0f;
    
    // Calculate the target car height (center of mass should be above the wheels)
    // The car's center is at centerOfMassHeight above the wheel contact point
    float targetCarHeight = avgWheelHeight + wheelRadius + centerOfMassHeight;
    
    // Distance from current position to target ground position
    float distanceToGround = position.y - targetCarHeight;
    
    // Airborne detection with proper threshold
    if (distanceToGround > 0.15f) {
        airborne = true;
        airborneTime += deltaTime;
    } else {
        // Advanced landing impact physics
        if (airborne && airborneTime > 0.05f) {
            float verticalImpactSpeed = std::abs(velocity.y);
            float horizontalSpeed = std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
            
            // Calculate total impact energy
            float totalImpactSpeed = std::sqrt(verticalImpactSpeed * verticalImpactSpeed + 
                                              horizontalSpeed * horizontalSpeed);
            
            if (totalImpactSpeed > 20.0f) {
                // Catastrophic landing - major energy loss
                velocity.y *= 0.15f;
                velocity.x *= 0.3f; // Lose most horizontal speed
                velocity.z *= 0.3f;
            } else if (totalImpactSpeed > 10.0f) {
                // Hard landing - significant bounce and energy loss
                velocity.y *= 0.25f;
                velocity.x *= 0.5f;
                velocity.z *= 0.5f;
            } else if (totalImpactSpeed > 5.0f) {
                // Medium landing - small bounce
                velocity.y *= 0.2f;
                velocity.x *= 0.7f;
                velocity.z *= 0.7f;
            } else if (verticalImpactSpeed > 2.0f) {
                // Soft landing with minor bounce
                velocity.y *= 0.3f;
                velocity.x *= 0.9f;
                velocity.z *= 0.9f;
            } else {
                // Very soft landing - no bounce
                velocity.y *= 0.5f;
                velocity.x *= 0.95f;
                velocity.z *= 0.95f;
            }
        }
        
        airborne = false;
        airborneTime = 0.0f;
        
        // Snap to ground - position car at correct height above terrain
        position.y = targetCarHeight;
        
        // Remove any downward velocity when on ground
        if (velocity.y < 0) {
            velocity.y = 0;
        }
    }
    
    updateEngine(deltaTime);
    updateTransmission();
    applyForces(deltaTime, terrainNormal, terrainFriction);
    updateKinematics(deltaTime);
}

void Car::reset(const Vector3& pos) {
    position = pos;
    velocity = Vector3(0, 0, 0);
    acceleration = Vector3(0, 0, 0);
    rotation = Quaternion(1, 0, 0, 0);
    angularVelocity = Vector3(0, 0, 0);
    throttle = 0;
    steering = 0;
    brake = 0;
    engineRPM = idleRPM;
    currentGear = 1;
    airborne = false;
}

void Car::applyForces(float dt, const Vector3& terrainNormal, float terrainFriction) {
    Vector3 forward = getForwardVector();
    Vector3 right = getRightVector();
    Vector3 up = terrainNormal;
    
    totalForce = Vector3(0, 0, 0);
    
    if (!airborne) {
        // Engine force with improved torque curve
        float gearRatio = gearRatios[currentGear] * finalDrive;
        
        // Better torque curve - more power at mid-range RPM
        float rpmRatio = engineRPM / maxRPM;
        float torqueMultiplier = 1.0f;
        if (rpmRatio < 0.3f) {
            torqueMultiplier = 0.6f + (rpmRatio / 0.3f) * 0.4f; // Low RPM: 60-100%
        } else if (rpmRatio > 0.85f) {
            torqueMultiplier = 1.0f - ((rpmRatio - 0.85f) / 0.15f) * 0.3f; // High RPM: 100-70%
        }
        
        float engineTorque = enginePower * torqueMultiplier * std::abs(throttle);
        float wheelForce = (engineTorque * gearRatio) / (wheelbase * 2.0f);
        
        // Apply throttle direction (forward/reverse)
        Vector3 engineForce = forward * (wheelForce * (throttle >= 0 ? 1.0f : -0.5f));
        
        // Tire grip limit with terrain friction
        float effectiveGrip = tireGrip * terrainFriction;
        float maxTireForce = effectiveGrip * mass * 9.81f * 0.8f;
        
        if (engineForce.length() > maxTireForce) {
            engineForce = engineForce.normalized() * maxTireForce;
            // Calculate wheel slip for traction control
            for (int i = 0; i < 4; ++i) {
                wheelSlip[i] = (engineForce.length() - maxTireForce) / maxTireForce;
            }
        } else {
            for (int i = 0; i < 4; ++i) {
                wheelSlip[i] = 0.0f;
            }
        }
        
        // Apply traction control
        float throttleOutput = std::abs(throttle);
        applyTractionControl(throttleOutput);
        engineForce = engineForce * (throttleOutput / std::max(0.01f, std::abs(throttle)));
        
        totalForce = totalForce + engineForce;
        
        // Improved lateral tire forces (cornering)
        float lateralVel = velocity.dot(right);
        float forwardVel = velocity.dot(forward);
        
        if (std::abs(forwardVel) > 0.5f) {
            float slipAngle = std::atan2(lateralVel, std::abs(forwardVel));
            
            // Pacejka-like tire model for lateral force
            float normalizedSlip = std::clamp(slipAngle * 10.0f, -1.0f, 1.0f);
            float lateralForce = -normalizedSlip * effectiveGrip * mass * 9.81f * 1.5f;
            
            // Reduce lateral grip at high slip angles (drifting)
            if (std::abs(slipAngle) > 0.2f) {
                lateralForce *= 0.7f;
            }
            
            lateralForce = std::clamp(lateralForce, -maxTireForce, maxTireForce);
            totalForce = totalForce + right * lateralForce;
        }
        
        // Improved rolling resistance
        float speed = velocity.length();
        if (speed > 0.1f) {
            float rollingCoeff = rollingResistance * (1.0f + speed * 0.01f); // Increases with speed
            Vector3 rollingForce = velocity.normalized() * (-rollingCoeff * mass * 9.81f);
            totalForce = totalForce + rollingForce;
        }
        
        // Enhanced brake force with ABS
        if (brake > 0.01f || handbrake) {
            float brakeMultiplier = handbrake ? 2.0f : 1.0f;
            float brakePower = brake * brakeMultiplier;
            
            // Apply ABS per wheel
            for (int i = 0; i < 4; ++i) {
                applyABS(brakePower, i);
            }
            
            // Brake force proportional to speed (more effective at high speed)
            float brakeForce = brakePower * mass * 30.0f * std::min(1.0f, speed / 10.0f);
            
            if (speed > 0.1f) {
                Vector3 brakeVector = velocity.normalized() * (-brakeForce);
                totalForce = totalForce + brakeVector;
            }
        }
    } else {
        // In air - enhanced airborne physics
        float speed = velocity.length();
        
        // Realistic air resistance (higher drag when tumbling)
        const float airDensity = 1.225f; // kg/m³ at sea level
        const float airborneDragCoeff = 0.8f; // Higher drag when not streamlined
        const float frontalArea = 2.5f; // m² (larger when tumbling)
        
        if (speed > 0.1f) {
            // Quadratic air drag: F = 0.5 * ρ * Cd * A * v²
            float dragMagnitude = 0.5f * airDensity * airborneDragCoeff * frontalArea * speed * speed;
            Vector3 airResistance = velocity.normalized() * (-dragMagnitude);
            totalForce = totalForce + airResistance;
        }
        
        // Tumbling rotation during flight (angular momentum)
        if (std::abs(velocity.y) > 5.0f) {
            // Car rotates slightly when falling fast
            float tumbleRate = velocity.y * 0.02f * dt;
            angularVelocity.y += tumbleRate * steering * 0.5f; // Steering affects tumble
        }
        
        // Dampen angular velocity in air (air resistance on rotation)
        angularVelocity = angularVelocity * 0.98f;
    }
    
    // Aerodynamic drag (always applied)
    float speed = velocity.length();
    if (speed > 1.0f) {
        Vector3 dragForce = velocity.normalized() * (-dragCoeff * speed * speed * 1.5f);
        totalForce = totalForce + dragForce;
    }
    
    // Gravity - always applied
    if (airborne) {
        // Full gravity when airborne
        totalForce = totalForce + Vector3(0, -9.81f * mass, 0);
    } else {
        // Gravity adjusted for terrain slope
        Vector3 gravityForce = getGravityForce(terrainNormal);
        totalForce = totalForce + gravityForce;
    }
    
    // Calculate acceleration
    acceleration = totalForce / mass;
}

void Car::updateKinematics(float dt) {
    // Update velocity with proper integration
    velocity = velocity + acceleration * dt;
    
    // Speed limiter
    float speed = velocity.length();
    if (speed > maxSpeed) {
        velocity = velocity.normalized() * maxSpeed;
    }
    
    // Update position
    position = position + velocity * dt;
    
    // Improved steering with speed-dependent response
    if (!airborne && std::abs(steering) > 0.01f) {
        Vector3 forward = getForwardVector();
        float forwardSpeed = velocity.dot(forward);
        
        // Only steer if moving with minimum speed
        if (std::abs(forwardSpeed) > 0.3f) {
            // Speed-dependent steering sensitivity
            float speedFactor = std::min(1.0f, std::abs(forwardSpeed) / 10.0f);
            float effectiveSteering = steering * speedFactor;
            
            // Calculate turn radius using Ackermann geometry
            float steerAngle = effectiveSteering * maxSteeringAngle;
            
            // Prevent division by zero
            if (std::abs(steerAngle) > 0.001f) {
                float turnRadius = wheelbase / std::tan(steerAngle);
                
                // Limit minimum turn radius for realism
                turnRadius = std::max(std::abs(turnRadius), wheelbase * 2.0f) * (turnRadius >= 0 ? 1.0f : -1.0f);
                
                // Angular velocity based on speed and turn radius
                float angularSpeed = forwardSpeed / turnRadius;
                
                // Apply rotation smoothly
                Quaternion deltaRotation = Quaternion::fromAxisAngle(Vector3(0, 1, 0), angularSpeed * dt);
                rotation = deltaRotation * rotation;
                rotation = rotation.normalized();
                
                // Update angular velocity for physics
                angularVelocity.y = angularSpeed;
            }
        } else {
            // Very slow speed - minimal steering
            angularVelocity.y *= 0.9f;
        }
    } else {
        // No steering input - dampen rotation
        angularVelocity.y *= 0.92f;
    }
    
    // Apply residual angular velocity (for drifting/sliding)
    if (std::abs(angularVelocity.y) > 0.01f) {
        Quaternion angularQuat = Quaternion::fromAxisAngle(
            Vector3(0, 1, 0), 
            angularVelocity.y * dt
        );
        rotation = angularQuat * rotation;
        rotation = rotation.normalized();
    }
}

void Car::applyThrottle(float amount) {
    throttle = std::clamp(amount, -1.0f, 1.0f);
}

void Car::applySteering(float angle) {
    steering = std::clamp(angle, -1.0f, 1.0f);
}

void Car::applyBrake(float amount) {
    brake = std::clamp(amount, 0.0f, 1.0f);
}

Vector3 Car::getCentripetalForce() const {
    float speed = velocity.length();
    if (speed < 0.1f || std::abs(steering) < 0.01f) return Vector3(0, 0, 0);
    
    float turnRadius = wheelbase / std::tan(steering);
    float centripetalAccel = (speed * speed) / turnRadius;
    
    Vector3 right = rotation.rotate(Vector3(1, 0, 0));
    return right * (mass * centripetalAccel * (steering > 0 ? 1.0f : -1.0f));
}

Vector3 Car::getCentrifugalForce() const {
    return getCentripetalForce() * -1.0f;
}

Vector3 Car::getGravityForce(const Vector3& terrainNormal) const {
    Vector3 gravity(0, -9.81f, 0);
    Vector3 projectedGravity = gravity - terrainNormal * gravity.dot(terrainNormal);
    return projectedGravity * mass;
}

void Car::updateEngine(float dt) {
    float speed = velocity.length();
    
    // Calculate wheel speed from actual velocity
    Vector3 forward = getForwardVector();
    float forwardSpeed = velocity.dot(forward);
    float wheelSpeed = std::abs(forwardSpeed) / (wheelRadius * 2.0f * 3.14159f); // rad/s
    
    // Calculate target RPM based on wheel speed and gear
    float gearRatio = gearRatios[currentGear] * finalDrive;
    float targetRPM = wheelSpeed * gearRatio * 60.0f; // Convert to RPM
    targetRPM = std::max(idleRPM, targetRPM);
    
    // Engine response based on throttle
    if (std::abs(throttle) > 0.01f) {
        // Throttle increases RPM
        float targetThrottleRPM = idleRPM + (maxRPM - idleRPM) * std::abs(throttle);
        targetRPM = std::max(targetRPM, targetThrottleRPM * 0.8f);
    } else {
        // No throttle - engine braking
        targetRPM = std::max(targetRPM * 0.95f, idleRPM);
    }
    
    // Smooth RPM transition with realistic engine inertia
    float rpmDiff = targetRPM - engineRPM;
    float rpmChangeRate = 8.0f; // Faster response
    
    if (rpmDiff > 0) {
        // Accelerating - faster response
        engineRPM += rpmDiff * dt * rpmChangeRate;
    } else {
        // Decelerating - slower response (engine inertia)
        engineRPM += rpmDiff * dt * (rpmChangeRate * 0.6f);
    }
    
    engineRPM = std::clamp(engineRPM, idleRPM, maxRPM);
}

void Car::updateTransmission() {
    // Improved automatic transmission logic
    float speed = velocity.length();
    
    // Upshift at high RPM (6-speed transmission: gears 0-5)
    if (engineRPM > maxRPM * 0.88f && currentGear < 5) {
        currentGear++;
        engineRPM *= 0.7f; // RPM drops after shift
    }
    // Downshift at low RPM (but not too low to avoid gear hunting)
    else if (engineRPM < maxRPM * 0.25f && currentGear > 0 && speed > 1.5f) {
        currentGear--;
        engineRPM *= 1.4f; // RPM increases after downshift
    }
    
    // Force first gear at very low speeds
    if (speed < 1.0f && currentGear > 0) {
        currentGear = 0;
    }
}

void Car::updateInertia() {
    // Approximate moment of inertia for a box
    inertia = mass * (wheelbase * wheelbase + trackWidth * trackWidth) / 12.0f;
}

float Car::calculateTireForce(float slipRatio) const {
    // Pacejka tire model (simplified)
    float B = 10.0f;  // Stiffness factor
    float C = 1.9f;   // Shape factor
    float D = tireGrip * mass * 9.81f;  // Peak value
    float E = 0.97f;  // Curvature factor
    
    return D * std::sin(C * std::atan(B * slipRatio - E * (B * slipRatio - std::atan(B * slipRatio))));
}

float Car::getSlipAngle() const {
    Vector3 forward = getForwardVector();
    Vector3 right = getRightVector();
    
    float lateralVel = velocity.dot(right);
    float forwardVel = velocity.dot(forward);
    
    if (std::abs(forwardVel) < 0.1f) return 0.0f;
    
    return std::atan2(lateralVel, std::abs(forwardVel));
}

float Car::getLateralAcceleration() const {
    Vector3 right = getRightVector();
    return acceleration.dot(right);
}

void Car::applyHandbrake(bool active) {
    handbrake = active;
}

float Car::getRotation() const {
    // Extract yaw angle from quaternion
    float siny_cosp = 2.0f * (rotation.w * rotation.z + rotation.x * rotation.y);
    float cosy_cosp = 1.0f - 2.0f * (rotation.y * rotation.y + rotation.z * rotation.z);
    return std::atan2(siny_cosp, cosy_cosp);
}

Vector3 Car::getWheelPosition(int wheelIndex) const {
    float rotation = getRotation();
    float cosR = std::cos(rotation);
    float sinR = std::sin(rotation);
    
    Vector3 offset;
    switch (wheelIndex) {
        case 0: // Front Right
            offset = Vector3(trackWidth/2 * cosR - wheelbase/2 * sinR, 0, 
                           -trackWidth/2 * sinR - wheelbase/2 * cosR);
            break;
        case 1: // Front Left
            offset = Vector3(-trackWidth/2 * cosR - wheelbase/2 * sinR, 0, 
                           trackWidth/2 * sinR - wheelbase/2 * cosR);
            break;
        case 2: // Rear Right
            offset = Vector3(trackWidth/2 * cosR + wheelbase/2 * sinR, 0, 
                           -trackWidth/2 * sinR + wheelbase/2 * cosR);
            break;
        case 3: // Rear Left
            offset = Vector3(-trackWidth/2 * cosR + wheelbase/2 * sinR, 0, 
                           trackWidth/2 * sinR + wheelbase/2 * cosR);
            break;
    }
    
    return position + offset;
}

float Car::getSuspensionCompression(int wheelIndex) const {
    if (wheelIndex >= 0 && wheelIndex < 4) {
        return suspensionCompression[wheelIndex];
    }
    return 0.0f;
}

void Car::updateSuspension(float dt, const float wheelHeights[4]) {
    for (int i = 0; i < 4; ++i) {
        Vector3 wheelPos = getWheelPosition(i);
        
        // Calculate how much the suspension should compress
        // wheelHeights[i] is the terrain height at this wheel
        // wheelPos.y is the current wheel position in world space
        float groundContactPoint = wheelHeights[i];
        float wheelBottom = wheelPos.y - wheelRadius;
        
        // Compression is positive when wheel is pushed up into the car body
        float compression = groundContactPoint - wheelBottom;
        
        // Clamp compression to suspension travel
        compression = std::clamp(compression, -suspensionTravel, suspensionTravel);
        
        // Spring force (pushes car up when compressed)
        float springForce = compression * suspensionStiffness;
        
        // Damping force (resists compression velocity)
        float compressionVelocity = (compression - suspensionCompression[i]) / std::max(dt, 0.001f);
        float dampingForce = compressionVelocity * suspensionDamping;
        
        // Total suspension force per wheel
        float wheelForce = springForce + dampingForce;
        
        // Apply force to car (distributed across all wheels)
        if (!airborne && compression > -wheelRadius * 0.5f) {
            // Only apply suspension force if wheel is near or touching ground
            Vector3 suspensionForce(0, wheelForce / 4.0f, 0);
            // Note: totalForce is accumulated in applyForces, not here
        }
        
        suspensionCompression[i] = compression;
    }
}

void Car::applyTractionControl(float& throttleOutput) {
    if (!tractionControlEnabled) return;
    
    // Calculate average wheel slip
    float avgSlip = 0.0f;
    for (int i = 0; i < 4; ++i) {
        avgSlip += std::abs(wheelSlip[i]);
    }
    avgSlip /= 4.0f;
    
    // Reduce throttle if excessive slip detected
    if (avgSlip > 0.15f) {
        throttleOutput *= 0.7f;
    }
}

void Car::applyABS(float& brakeOutput, int wheelIndex) {
    if (!absEnabled) return;
    
    // Prevent wheel lockup
    if (std::abs(wheelSlip[wheelIndex]) > 0.2f) {
        brakeOutput *= 0.5f;
    }
}

// Advanced vehicle dynamics methods

void Car::setTireType(const std::string& type) {
    for (int i = 0; i < 4; ++i) {
        vehicleDynamics.tireModels[i].setTireType(type);
    }
}

void Car::setDriveType(int type) {
    DrivetrainParameters params = vehicleDynamics.getVehicleDynamics()->getDrivetrain();
    params.driveType = static_cast<DrivetrainParameters::DriveType>(type);
    vehicleDynamics.setDrivetrain(params);
}

void Car::setDownforce(float coefficient) {
    AerodynamicsParameters params;
    params.liftCoefficient = -std::abs(coefficient);
    vehicleDynamics.setAerodynamics(params);
}

void Car::setAntiRollStiffness(float stiffness) {
    SuspensionParameters params;
    params.antiRollStiffness = stiffness;
    vehicleDynamics.setSuspension(params);
}

float Car::getTireTemperature(int wheelIndex) const {
    if (wheelIndex >= 0 && wheelIndex < 4) {
        return vehicleDynamics.tireModels[wheelIndex].getTemperature();
    }
    return 0.0f;
}

float Car::getTireWear(int wheelIndex) const {
    if (wheelIndex >= 0 && wheelIndex < 4) {
        return vehicleDynamics.tireModels[wheelIndex].getWear();
    }
    return 0.0f;
}

float Car::getDownforceAmount() const {
    return vehicleDynamics.getDownforce();
}

float Car::getRollAngle() const {
    return vehicleDynamics.getRollAngle();
}

float Car::getPitchAngle() const {
    return vehicleDynamics.getPitchAngle();
}

TireForces Car::getTireForces(int wheelIndex) const {
    if (wheelIndex >= 0 && wheelIndex < 4) {
        return vehicleDynamics.getWheel(wheelIndex).tireForces;
    }
    return TireForces();
}
