#pragma once
#include "Math.h"

/**
 * Advanced Tire Model based on Pacejka Magic Formula
 * Reference: svenlr/car-physics-pacejka
 * 
 * Implements the Magic Formula tire model for realistic tire forces
 */

struct PacejkaCoefficients {
    // Longitudinal force coefficients
    float B_long;  // Stiffness factor
    float C_long;  // Shape factor
    float D_long;  // Peak factor
    float E_long;  // Curvature factor
    
    // Lateral force coefficients
    float B_lat;
    float C_lat;
    float D_lat;
    float E_lat;
    
    // Aligning moment coefficients
    float B_align;
    float C_align;
    float D_align;
    float E_align;
    
    // Load sensitivity
    float loadSensitivity;
    float camberSensitivity;
    
    PacejkaCoefficients() {
        // Default values for passenger car tire
        B_long = 10.0f;
        C_long = 1.9f;
        D_long = 1.0f;
        E_long = 0.97f;
        
        B_lat = 8.0f;
        C_lat = 1.3f;
        D_lat = 1.0f;
        E_lat = -2.0f;
        
        B_align = 9.0f;
        C_align = 2.4f;
        D_align = 1.0f;
        E_align = -1.0f;
        
        loadSensitivity = 1.0f;
        camberSensitivity = 0.1f;
    }
};

struct TireState {
    float slipRatio;        // Longitudinal slip
    float slipAngle;        // Lateral slip (rad)
    float normalLoad;       // Vertical load (N)
    float camberAngle;      // Camber angle (rad)
    float pressure;         // Tire pressure (bar)
    float temperature;      // Tire temperature (°C)
    
    TireState() : slipRatio(0), slipAngle(0), normalLoad(0), 
                  camberAngle(0), pressure(2.5f), temperature(25.0f) {}
};

struct TireForces {
    float longitudinal;     // Fx (N)
    float lateral;          // Fy (N)
    float vertical;         // Fz (N)
    float aligningMoment;   // Mz (Nm)
    
    TireForces() : longitudinal(0), lateral(0), vertical(0), aligningMoment(0) {}
};

class TireModel {
public:
    TireModel();
    
    // Calculate tire forces using Pacejka Magic Formula
    TireForces calculateForces(const TireState& state);
    
    // Individual force components
    float calculateLongitudinalForce(float slipRatio, float normalLoad);
    float calculateLateralForce(float slipAngle, float normalLoad, float camber);
    float calculateAligningMoment(float slipAngle, float normalLoad);
    
    // Tire dynamics
    void updateTemperature(float dt, float slipEnergy);
    void updateWear(float dt, float slipEnergy);
    
    // Configuration
    void setCoefficients(const PacejkaCoefficients& coeff) { coefficients = coeff; }
    void setTireType(const std::string& type);
    
    // Getters
    float getTemperature() const { return temperature; }
    float getWear() const { return wear; }
    float getGripMultiplier() const;
    
private:
    PacejkaCoefficients coefficients;
    float temperature;      // Current temperature (°C)
    float wear;            // Wear level (0-1, 0=new, 1=worn)
    float optimalTemp;     // Optimal operating temperature
    float maxTemp;         // Maximum safe temperature
    
    // Magic Formula implementation
    float magicFormula(float x, float B, float C, float D, float E);
    float calculatePeakValue(float normalLoad);
};

