#include "VehicleDynamics.h"
#include <cmath>
#include <algorithm>

VehicleDynamics::VehicleDynamics()
    : totalDownforce(0), rollAngle(0), pitchAngle(0), yawRate(0),
      mass(1200.0f), wheelbase(2.6f), trackWidth(1.5f),
      centerOfGravityHeight(0.5f), frontWeightDistribution(0.5f) {
    
    // Calculate moments of inertia (approximation for box)
    rollInertia = mass * (wheelbase * wheelbase + centerOfGravityHeight * centerOfGravityHeight) / 12.0f;
    pitchInertia = mass * (trackWidth * trackWidth + centerOfGravityHeight * centerOfGravityHeight) / 12.0f;
    yawInertia = mass * (wheelbase * wheelbase + trackWidth * trackWidth) / 12.0f;
    
    // Initialize tire models
    for (int i = 0; i < 4; ++i) {
        tireModels[i].setTireType("sport");
    }
}

void VehicleDynamics::calculateLoadTransfer(float dt) {
    // Longitudinal load transfer (acceleration/braking)
    float longitudinalAccel = 0.0f; // Get from vehicle state
    float longitudinalTransfer = mass * longitudinalAccel * centerOfGravityHeight / wheelbase;
    
    // Lateral load transfer (cornering)
    float lateralAccel = 0.0f; // Get from vehicle state
    float lateralTransfer = mass * lateralAccel * centerOfGravityHeight / trackWidth;
    
    // Roll angle from lateral acceleration
    float rollStiffness = suspension.antiRollStiffness + 
                         suspension.springRate * trackWidth * trackWidth / 2.0f;
    float rollMoment = mass * lateralAccel * centerOfGravityHeight;
    rollAngle = rollMoment / rollStiffness;
    
    // Pitch angle from longitudinal acceleration
    float pitchStiffness = suspension.springRate * wheelbase * wheelbase / 2.0f;
    float pitchMoment = mass * longitudinalAccel * centerOfGravityHeight;
    pitchAngle = pitchMoment / pitchStiffness;
    
    // Static weight distribution
    float frontStaticLoad = mass * 9.81f * frontWeightDistribution;
    float rearStaticLoad = mass * 9.81f * (1.0f - frontWeightDistribution);
    
    // Apply load transfer to wheels
    // Front left
    wheels[0].normalLoad = frontStaticLoad / 2.0f - longitudinalTransfer / 2.0f - lateralTransfer / 2.0f;
    // Front right
    wheels[1].normalLoad = frontStaticLoad / 2.0f - longitudinalTransfer / 2.0f + lateralTransfer / 2.0f;
    // Rear left
    wheels[2].normalLoad = rearStaticLoad / 2.0f + longitudinalTransfer / 2.0f - lateralTransfer / 2.0f;
    // Rear right
    wheels[3].normalLoad = rearStaticLoad / 2.0f + longitudinalTransfer / 2.0f + lateralTransfer / 2.0f;
    
    // Ensure positive loads
    for (int i = 0; i < 4; ++i) {
        wheels[i].normalLoad = std::max(0.0f, wheels[i].normalLoad);
    }
}

Vector3 VehicleDynamics::calculateAerodynamicForces(const Vector3& velocity) {
    float speed = velocity.length();
    if (speed < 0.1f) return Vector3(0, 0, 0);
    
    float airDensity = 1.225f; // kg/m^3 at sea level
    float dynamicPressure = 0.5f * airDensity * speed * speed;
    
    // Drag force (opposes motion)
    Vector3 dragDirection = velocity.normalized();
    float dragForce = dynamicPressure * aero.frontalArea * aero.dragCoefficient;
    Vector3 drag = dragDirection * -dragForce;
    
    // Downforce (negative lift)
    float downforceMagnitude = dynamicPressure * aero.frontalArea * std::abs(aero.liftCoefficient);
    totalDownforce = downforceMagnitude;
    
    Vector3 downforce(0, -downforceMagnitude, 0);
    
    return drag + downforce;
}

Vector3 VehicleDynamics::calculateDownforce(float speed) {
    if (speed < 0.1f) return Vector3(0, 0, 0);
    
    float airDensity = 1.225f;
    float dynamicPressure = 0.5f * airDensity * speed * speed;
    float downforceMagnitude = dynamicPressure * aero.frontalArea * std::abs(aero.liftCoefficient);
    
    // Distribute downforce front/rear
    float frontDownforce = downforceMagnitude * aero.downforceBalance;
    float rearDownforce = downforceMagnitude * (1.0f - aero.downforceBalance);
    
    // Add to wheel loads
    wheels[0].normalLoad += frontDownforce / 2.0f;
    wheels[1].normalLoad += frontDownforce / 2.0f;
    wheels[2].normalLoad += rearDownforce / 2.0f;
    wheels[3].normalLoad += rearDownforce / 2.0f;
    
    return Vector3(0, -downforceMagnitude, 0);
}

float VehicleDynamics::calculateEngineTorque(float rpm, float throttle) {
    // Realistic engine torque curve
    float maxTorque = 300.0f; // Nm
    float maxPowerRPM = 6000.0f;
    float idleRPM = 1000.0f;
    
    // Normalized RPM
    float rpmNorm = (rpm - idleRPM) / (maxPowerRPM - idleRPM);
    rpmNorm = std::clamp(rpmNorm, 0.0f, 1.0f);
    
    // Torque curve (peaks at mid-range)
    float torqueMultiplier;
    if (rpmNorm < 0.3f) {
        torqueMultiplier = 0.6f + (rpmNorm / 0.3f) * 0.4f;
    } else if (rpmNorm < 0.7f) {
        torqueMultiplier = 1.0f;
    } else {
        torqueMultiplier = 1.0f - ((rpmNorm - 0.7f) / 0.3f) * 0.3f;
    }
    
    return maxTorque * torqueMultiplier * throttle * drivetrain.efficiency;
}

void VehicleDynamics::distributeTorque(float engineTorque, float wheelSpeeds[4]) {
    float totalTorque = engineTorque * drivetrain.differentialRatio;
    
    switch (drivetrain.driveType) {
        case DrivetrainParameters::FWD:
            // Front wheel drive
            wheels[0].tireState.slipRatio = (wheelSpeeds[0] - wheelSpeeds[2]) / std::max(0.1f, wheelSpeeds[2]);
            wheels[1].tireState.slipRatio = (wheelSpeeds[1] - wheelSpeeds[3]) / std::max(0.1f, wheelSpeeds[3]);
            break;
            
        case DrivetrainParameters::RWD:
            // Rear wheel drive
            wheels[2].tireState.slipRatio = (wheelSpeeds[2] - wheelSpeeds[0]) / std::max(0.1f, wheelSpeeds[0]);
            wheels[3].tireState.slipRatio = (wheelSpeeds[3] - wheelSpeeds[1]) / std::max(0.1f, wheelSpeeds[1]);
            break;
            
        case DrivetrainParameters::AWD:
            // All wheel drive with torque split
            float frontTorque = totalTorque * drivetrain.frontTorqueSplit;
            float rearTorque = totalTorque * (1.0f - drivetrain.frontTorqueSplit);
            
            for (int i = 0; i < 4; ++i) {
                float refSpeed = (wheelSpeeds[0] + wheelSpeeds[1] + wheelSpeeds[2] + wheelSpeeds[3]) / 4.0f;
                wheels[i].tireState.slipRatio = (wheelSpeeds[i] - refSpeed) / std::max(0.1f, refSpeed);
            }
            break;
    }
}

float VehicleDynamics::calculateAntiRollTorque(float leftTravel, float rightTravel) {
    // Anti-roll bar resists roll by connecting left and right suspension
    float travelDifference = leftTravel - rightTravel;
    return suspension.antiRollStiffness * travelDifference;
}

void VehicleDynamics::updateSuspension(float dt) {
    for (int i = 0; i < 4; ++i) {
        WheelState& wheel = wheels[i];
        
        // Spring force
        float springForce = suspension.springRate * wheel.suspensionTravel;
        
        // Damper force
        float damperVelocity = wheel.velocity.y;
        float damperForce = suspension.damperRate * damperVelocity;
        
        // Bump stop (progressive spring at travel limits)
        if (wheel.suspensionTravel > suspension.maxTravel * 0.9f) {
            float bumpCompression = wheel.suspensionTravel - suspension.maxTravel * 0.9f;
            springForce += suspension.bumpStopRate * bumpCompression;
        }
        
        // Total suspension force
        float totalForce = springForce + damperForce;
        
        // Apply to wheel load
        wheel.normalLoad += totalForce;
    }
    
    // Anti-roll bars
    float frontAntiRoll = calculateAntiRollTorque(wheels[0].suspensionTravel, wheels[1].suspensionTravel);
    float rearAntiRoll = calculateAntiRollTorque(wheels[2].suspensionTravel, wheels[3].suspensionTravel);
    
    // Distribute anti-roll forces
    wheels[0].normalLoad += frontAntiRoll / trackWidth;
    wheels[1].normalLoad -= frontAntiRoll / trackWidth;
    wheels[2].normalLoad += rearAntiRoll / trackWidth;
    wheels[3].normalLoad -= rearAntiRoll / trackWidth;
}

void VehicleDynamics::calculateWeightDistribution() {
    // Calculate static weight distribution based on CG position
    float frontAxleDistance = wheelbase * (1.0f - frontWeightDistribution);
    float rearAxleDistance = wheelbase * frontWeightDistribution;
    
    // Adjust for CG height (affects load transfer)
    float heightFactor = centerOfGravityHeight / wheelbase;
    
    // Update front weight distribution
    frontWeightDistribution = frontAxleDistance / wheelbase;
}

void VehicleDynamics::update(float dt, const Vector3& terrainNormal, float terrainHeight) {
    // 1. Calculate load transfer
    calculateLoadTransfer(dt);
    
    // 2. Update suspension
    updateSuspension(dt);
    
    // 3. Calculate tire forces for each wheel
    for (int i = 0; i < 4; ++i) {
        wheels[i].tireForces = tireModels[i].calculateForces(wheels[i].tireState);
        
        // Update tire temperature and wear
        float slipEnergy = std::abs(wheels[i].tireState.slipRatio) * 
                          std::abs(wheels[i].tireForces.longitudinal) +
                          std::abs(wheels[i].tireState.slipAngle) * 
                          std::abs(wheels[i].tireForces.lateral);
        
        tireModels[i].updateTemperature(dt, slipEnergy);
        tireModels[i].updateWear(dt, slipEnergy);
    }
}

Vector3 VehicleDynamics::calculateRollCenter() {
    // Simplified roll center calculation
    // In reality, this depends on suspension geometry
    float rollCenterHeight = centerOfGravityHeight * 0.3f; // Typical ratio
    return Vector3(0, rollCenterHeight, 0);
}

