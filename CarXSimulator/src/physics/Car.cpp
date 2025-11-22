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
      maxSpeed(60.0f), maxSteeringAngle(0.6f) {
    
    // Gear ratios (6-speed)
    gearRatios[0] = 3.5f;  // 1st
    gearRatios[1] = 2.5f;  // 2nd
    gearRatios[2] = 1.8f;  // 3rd
    gearRatios[3] = 1.3f;  // 4th
    gearRatios[4] = 1.0f;  // 5th
    gearRatios[5] = 0.8f;  // 6th
    
    updateInertia();
}

void Car::update(float deltaTime, const Vector3& terrainNormal, float terrainHeight) {
    // Check if airborne
    float groundClearance = 0.5f;
    float distanceToGround = position.y - terrainHeight - groundClearance;
    
    if (distanceToGround > 0.1f) {
        airborne = true;
        airborneTime += deltaTime;
    } else {
        if (airborne && airborneTime > 0.1f) {
            // Landing impact
            velocity.y *= 0.3f;  // Dampen vertical velocity
        }
        airborne = false;
        airborneTime = 0.0f;
        position.y = terrainHeight + groundClearance;
    }
    
    updateEngine(deltaTime);
    updateTransmission();
    applyForces(deltaTime, terrainNormal);
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

void Car::applyForces(float dt, const Vector3& terrainNormal) {
    Vector3 forward = getForwardVector();
    Vector3 right = getRightVector();
    Vector3 up = terrainNormal;
    
    totalForce = Vector3(0, 0, 0);
    
    if (!airborne) {
        // Engine force with gear ratio
        float gearRatio = gearRatios[currentGear] * finalDrive;
        float engineTorque = (engineRPM / maxRPM) * enginePower;
        float wheelForce = (engineTorque * gearRatio * throttle) / wheelbase;
        Vector3 engineForce = forward * wheelForce;
        
        // Tire grip limit
        float maxTireForce = tireGrip * mass * 9.81f;
        if (engineForce.length() > maxTireForce) {
            engineForce = engineForce.normalized() * maxTireForce;
        }
        
        totalForce = totalForce + engineForce;
        
        // Lateral tire forces (slip angle)
        float lateralVel = velocity.dot(right);
        float forwardVel = velocity.dot(forward);
        
        if (std::abs(forwardVel) > 0.1f) {
            float slipAngle = std::atan2(lateralVel, std::abs(forwardVel));
            float lateralForce = -slipAngle * tireGrip * mass * 9.81f * 2.0f;
            lateralForce = std::clamp(lateralForce, -maxTireForce, maxTireForce);
            totalForce = totalForce + right * lateralForce;
        }
        
        // Rolling resistance
        if (velocity.length() > 0.1f) {
            Vector3 rollingForce = velocity.normalized() * (-rollingResistance * mass * 9.81f);
            totalForce = totalForce + rollingForce;
        }
        
        // Brake force
        if (brake > 0.01f || handbrake) {
            float brakeMultiplier = handbrake ? 1.5f : 1.0f;
            Vector3 brakeForce = velocity.normalized() * (-brake * brakeMultiplier * mass * 25.0f);
            totalForce = totalForce + brakeForce;
        }
    }
    
    // Drag force (always applied)
    float speed = velocity.length();
    if (speed > 0.1f) {
        Vector3 dragForce = velocity.normalized() * (-dragCoeff * speed * speed * 0.5f);
        totalForce = totalForce + dragForce;
    }
    
    // Gravity adjusted for terrain
    Vector3 gravityForce = getGravityForce(terrainNormal);
    totalForce = totalForce + gravityForce;
    
    // Calculate acceleration
    acceleration = totalForce / mass;
}

void Car::updateKinematics(float dt) {
    // Update velocity
    velocity = velocity + acceleration * dt;
    
    // Speed limiter
    float speed = velocity.length();
    if (speed > maxSpeed) {
        velocity = velocity.normalized() * maxSpeed;
    }
    
    // Update position
    position = position + velocity * dt;
    
    // Steering (Ackermann steering geometry)
    if (!airborne && speed > 0.5f && std::abs(steering) > 0.01f) {
        Vector3 forward = getForwardVector();
        float forwardSpeed = velocity.dot(forward);
        
        if (std::abs(forwardSpeed) > 0.5f) {
            // Calculate turn radius using Ackermann geometry
            float steerAngle = steering * maxSteeringAngle;
            float turnRadius = wheelbase / std::tan(steerAngle);
            
            // Angular velocity based on speed and turn radius
            float angularSpeed = forwardSpeed / turnRadius;
            
            // Apply rotation
            Quaternion deltaRotation = Quaternion::fromAxisAngle(Vector3(0, 1, 0), angularSpeed * dt);
            rotation = deltaRotation * rotation;
            rotation = rotation.normalized();
            
            // Update angular velocity for physics
            angularVelocity.y = angularSpeed;
        }
    } else {
        angularVelocity.y *= 0.95f;  // Damping
    }
    
    // Apply angular velocity
    if (angularVelocity.length() > 0.01f) {
        Quaternion angularQuat = Quaternion::fromAxisAngle(
            angularVelocity.normalized(), 
            angularVelocity.length() * dt
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
    float wheelSpeed = speed / (wheelbase * 0.5f);  // Approximate wheel angular velocity
    
    // Calculate target RPM based on wheel speed and gear
    float gearRatio = gearRatios[currentGear] * finalDrive;
    float targetRPM = wheelSpeed * gearRatio * 60.0f / (2.0f * 3.14159f);
    targetRPM = std::max(idleRPM, targetRPM);
    
    // Engine response
    if (throttle > 0.01f) {
        float targetThrottleRPM = idleRPM + (maxRPM - idleRPM) * throttle;
        targetRPM = std::max(targetRPM, targetThrottleRPM);
    }
    
    // Smooth RPM transition
    float rpmDiff = targetRPM - engineRPM;
    engineRPM += rpmDiff * dt * 5.0f;  // Engine inertia
    engineRPM = std::clamp(engineRPM, idleRPM, maxRPM);
}

void Car::updateTransmission() {
    // Automatic transmission logic
    float speed = velocity.length();
    
    // Upshift
    if (engineRPM > maxRPM * 0.85f && currentGear < 5) {
        currentGear++;
    }
    // Downshift
    else if (engineRPM < maxRPM * 0.3f && currentGear > 0 && speed > 2.0f) {
        currentGear--;
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
