#pragma once
#include "Math.h"

class Car {
public:
    Car(const Vector3& position);
    
    void update(float deltaTime, const Vector3& terrainNormal, float terrainHeight);
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
    
private:
    void updateKinematics(float dt);
    void applyForces(float dt, const Vector3& terrainNormal);
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
};
