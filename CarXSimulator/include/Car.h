#pragma once
#include "Math.h"
#include "TireModel.h"
#include "VehicleDynamics.h"

class Car {
public:
    Car(const Vector3& position);
    
    void update(float deltaTime, const Vector3& terrainNormal, float terrainHeight, float terrainFriction = 1.0f);
    void updateWithWheelHeights(float deltaTime, const Vector3& terrainNormal, const float wheelHeights[4], float terrainFriction = 1.0f);
    void reset(const Vector3& position);
    
    // Control inputs
    void applyThrottle(float amount);  // -1 to 1
    void applySteering(float angle);   // radians
    void applyBrake(float amount);     // 0 to 1
    void applyHandbrake(bool active);
    
    // Physics properties
    void setMass(float m) { mass = m; updateInertia(); }
    void setWheelbase(float wb) { wheelbase = wb; }
    void setDragCoefficient(float cd) { dragCoeff = cd; }
    void setTireGrip(float grip) { tireGrip = grip; }
    void setEnginePower(float power) { enginePower = power; }
    
    // Getters
    Vector3 getPosition() const { return position; }
    Vector3 getVelocity() const { return velocity; }
    Quaternion getRotation() const { return rotation; }
    float getSpeed() const { return velocity.length(); }
    float getRPM() const { return engineRPM; }
    int getGear() const { return currentGear; }
    Vector3 getForwardVector() const { return rotation.rotate(Vector3(0, 0, 1)); }
    Vector3 getRightVector() const { return rotation.rotate(Vector3(1, 0, 0)); }
    
    // Forces
    Vector3 getCentripetalForce() const;
    Vector3 getCentrifugalForce() const;
    Vector3 getGravityForce(const Vector3& terrainNormal) const;
    Vector3 getTotalForce() const { return totalForce; }
    
    // Advanced getters
    float getSlipAngle() const;
    float getLateralAcceleration() const;
    bool isAirborne() const { return airborne; }
    float getDragCoefficient() const { return dragCoeff; }
    float getFrontalArea() const { return 2.2f; } // m^2
    
    // Advanced physics
    void applyForce(const Vector3& force) { totalForce = totalForce + force; }
    float getRotation() const; // Get yaw angle in radians
    Vector3 getWheelPosition(int wheelIndex) const; // 0=FR, 1=FL, 2=RR, 3=RL
    
    // Suspension system
    void setSuspensionStiffness(float stiffness) { suspensionStiffness = stiffness; }
    void setSuspensionDamping(float damping) { suspensionDamping = damping; }
    float getSuspensionCompression(int wheelIndex) const;
    
    // Differential and traction control
    void setDifferentialType(int type) { differentialType = type; } // 0=open, 1=limited-slip, 2=locked
    void setTractionControl(bool enabled) { tractionControlEnabled = enabled; }
    void setAntiLockBrakes(bool enabled) { absEnabled = enabled; }
    
    // Advanced vehicle dynamics
    void setTireType(const std::string& type);
    void setDriveType(int type); // 0=FWD, 1=RWD, 2=AWD
    void setDownforce(float coefficient);
    void setAntiRollStiffness(float stiffness);
    
    // Advanced getters
    float getTireTemperature(int wheelIndex) const;
    float getTireWear(int wheelIndex) const;
    float getDownforceAmount() const;
    float getRollAngle() const;
    float getPitchAngle() const;
    TireForces getTireForces(int wheelIndex) const;
    
    // Vehicle dynamics system
    VehicleDynamics* getVehicleDynamics() { return &vehicleDynamics; }
    
private:
    void updateKinematics(float dt);
    void applyForces(float dt, const Vector3& terrainNormal, float terrainFriction = 1.0f);
    void updateEngine(float dt);
    void updateTransmission();
    void updateInertia();
    float calculateTireForce(float slipRatio) const;
    
    // State
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;
    Quaternion rotation;
    Vector3 angularVelocity;
    Vector3 totalForce;
    
    // Properties
    float mass;
    float inertia;
    float wheelbase;
    float trackWidth;
    float dragCoeff;
    float rollingResistance;
    float tireGrip;
    float centerOfMassHeight;
    
    // Control
    float throttle;
    float steering;
    float brake;
    bool handbrake;
    
    // Engine & Transmission
    float enginePower;
    float engineRPM;
    float maxRPM;
    float idleRPM;
    int currentGear;
    float gearRatios[6];
    float finalDrive;
    
    // State flags
    bool airborne;
    float airborneTime;
    
    // Limits
    float maxSpeed;
    float maxSteeringAngle;
    
    // Suspension
    float suspensionStiffness;
    float suspensionDamping;
    float suspensionTravel;
    float wheelRadius;
    float suspensionCompression[4]; // Per wheel
    
    // Advanced systems
    int differentialType;
    bool tractionControlEnabled;
    bool absEnabled;
    float wheelSlip[4]; // Per wheel slip ratio
    
    // Helpers
    void updateSuspension(float dt, const float wheelHeights[4]);
    void applyTractionControl(float& throttleOutput);
    void applyABS(float& brakeOutput, int wheelIndex);
    
    // Advanced vehicle dynamics
    VehicleDynamics vehicleDynamics;
    bool useAdvancedPhysics;
};
