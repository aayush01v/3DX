#pragma once
#include "Math.h"
#include "TireModel.h"

/**
 * Advanced Vehicle Dynamics
 * Based on: TUMFTM/Open-Car-Dynamics and Project Chrono
 * 
 * Implements professional vehicle dynamics with:
 * - Multi-body dynamics
 * - Load transfer
 * - Anti-roll bars
 * - Aerodynamics with downforce
 * - Advanced drivetrain
 */

struct SuspensionParameters {
    float springRate;           // N/m
    float damperRate;          // Ns/m
    float antiRollStiffness;   // Nm/rad
    float rideHeight;          // m
    float maxTravel;           // m
    float bumpStopRate;        // N/m (progressive spring)
    
    SuspensionParameters() 
        : springRate(50000.0f), damperRate(3000.0f), 
          antiRollStiffness(5000.0f), rideHeight(0.15f),
          maxTravel(0.15f), bumpStopRate(100000.0f) {}
};

struct AerodynamicsParameters {
    float dragCoefficient;     // Cd
    float frontalArea;         // m^2
    float liftCoefficient;     // Cl (negative = downforce)
    float downforceBalance;    // 0-1 (0=all rear, 1=all front)
    
    AerodynamicsParameters()
        : dragCoefficient(0.3f), frontalArea(2.2f),
          liftCoefficient(-0.5f), downforceBalance(0.45f) {}
};

struct DrivetrainParameters {
    enum DriveType { FWD, RWD, AWD };
    
    DriveType driveType;
    float frontTorqueSplit;    // For AWD (0-1)
    float clutchTorque;        // Nm
    float differentialRatio;   // Final drive
    float efficiency;          // 0-1
    
    DrivetrainParameters()
        : driveType(RWD), frontTorqueSplit(0.4f),
          clutchTorque(500.0f), differentialRatio(3.5f),
          efficiency(0.85f) {}
};

struct WheelState {
    Vector3 position;
    Vector3 velocity;
    float angularVelocity;     // rad/s
    float normalLoad;          // N
    float suspensionTravel;    // m
    TireState tireState;
    TireForces tireForces;
    
    WheelState() : angularVelocity(0), normalLoad(0), suspensionTravel(0) {}
};

class VehicleDynamics {
public:
    VehicleDynamics();
    
    // Main update
    void update(float dt, const Vector3& terrainNormal, float terrainHeight);
    
    // Load transfer calculation (critical for realistic handling)
    void calculateLoadTransfer(float dt);
    
    // Aerodynamic forces
    Vector3 calculateAerodynamicForces(const Vector3& velocity);
    Vector3 calculateDownforce(float speed);
    
    // Drivetrain
    void updateDrivetrain(float dt, float throttle);
    float calculateEngineTorque(float rpm, float throttle);
    void distributeTorque(float engineTorque, float wheelSpeeds[4]);
    
    // Anti-roll bar effect
    float calculateAntiRollTorque(float leftTravel, float rightTravel);
    
    // Weight distribution
    void calculateWeightDistribution();
    
    // Configuration
    void setSuspension(const SuspensionParameters& params) { suspension = params; }
    void setAerodynamics(const AerodynamicsParameters& params) { aero = params; }
    void setDrivetrain(const DrivetrainParameters& params) { drivetrain = params; }
    
    // Getters
    const WheelState& getWheel(int index) const { return wheels[index]; }
    float getDownforce() const { return totalDownforce; }
    float getRollAngle() const { return rollAngle; }
    float getPitchAngle() const { return pitchAngle; }
    
    // Tire models
    TireModel tireModels[4];
    
private:
    WheelState wheels[4];
    SuspensionParameters suspension;
    AerodynamicsParameters aero;
    DrivetrainParameters drivetrain;
    
    // Vehicle state
    float totalDownforce;
    float rollAngle;
    float pitchAngle;
    float yawRate;
    
    // Mass properties
    float mass;
    float wheelbase;
    float trackWidth;
    float centerOfGravityHeight;
    float frontWeightDistribution;
    
    // Inertia
    float rollInertia;
    float pitchInertia;
    float yawInertia;
    
    // Helper functions
    void updateWheelLoads(float dt);
    void updateSuspension(float dt);
    Vector3 calculateRollCenter();
};

