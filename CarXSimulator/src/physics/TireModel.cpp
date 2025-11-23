#include "TireModel.h"
#include <cmath>
#include <algorithm>

TireModel::TireModel() 
    : temperature(25.0f), wear(0.0f), optimalTemp(80.0f), maxTemp(120.0f) {
    coefficients = PacejkaCoefficients();
}

float TireModel::magicFormula(float x, float B, float C, float D, float E) {
    // Pacejka Magic Formula: y = D * sin(C * atan(B*x - E*(B*x - atan(B*x))))
    float BX = B * x;
    float atanBX = std::atan(BX);
    float innerTerm = BX - E * (BX - atanBX);
    return D * std::sin(C * std::atan(innerTerm));
}

float TireModel::calculatePeakValue(float normalLoad) {
    // Peak force increases with load but with diminishing returns
    float nominalLoad = 4000.0f; // N (typical passenger car)
    float loadRatio = normalLoad / nominalLoad;
    
    // Non-linear load sensitivity
    return coefficients.loadSensitivity * std::pow(loadRatio, 0.9f);
}

float TireModel::calculateLongitudinalForce(float slipRatio, float normalLoad) {
    if (normalLoad < 10.0f) return 0.0f;
    
    // Temperature effect on grip
    float tempFactor = getGripMultiplier();
    
    // Wear effect
    float wearFactor = 1.0f - wear * 0.3f; // 30% loss at full wear
    
    // Calculate peak force
    float D = calculatePeakValue(normalLoad) * normalLoad * tempFactor * wearFactor;
    
    // Apply magic formula
    float Fx = magicFormula(slipRatio, 
                           coefficients.B_long, 
                           coefficients.C_long, 
                           D, 
                           coefficients.E_long);
    
    return Fx;
}

float TireModel::calculateLateralForce(float slipAngle, float normalLoad, float camber) {
    if (normalLoad < 10.0f) return 0.0f;
    
    // Temperature and wear effects
    float tempFactor = getGripMultiplier();
    float wearFactor = 1.0f - wear * 0.25f;
    
    // Camber effect on lateral force
    float camberEffect = 1.0f + coefficients.camberSensitivity * std::abs(camber);
    
    // Calculate peak force
    float D = calculatePeakValue(normalLoad) * normalLoad * tempFactor * wearFactor * camberEffect;
    
    // Apply magic formula
    float Fy = magicFormula(slipAngle, 
                           coefficients.B_lat, 
                           coefficients.C_lat, 
                           D, 
                           coefficients.E_lat);
    
    return Fy;
}

float TireModel::calculateAligningMoment(float slipAngle, float normalLoad) {
    if (normalLoad < 10.0f) return 0.0f;
    
    // Aligning moment (self-aligning torque)
    float D = 0.1f * normalLoad; // Typical pneumatic trail
    
    float Mz = magicFormula(slipAngle,
                           coefficients.B_align,
                           coefficients.C_align,
                           D,
                           coefficients.E_align);
    
    return Mz;
}

TireForces TireModel::calculateForces(const TireState& state) {
    TireForces forces;
    
    forces.longitudinal = calculateLongitudinalForce(state.slipRatio, state.normalLoad);
    forces.lateral = calculateLateralForce(state.slipAngle, state.normalLoad, state.camberAngle);
    forces.vertical = state.normalLoad;
    forces.aligningMoment = calculateAligningMoment(state.slipAngle, state.normalLoad);
    
    // Combined slip (friction ellipse)
    float totalSlip = std::sqrt(state.slipRatio * state.slipRatio + 
                                std::tan(state.slipAngle) * std::tan(state.slipAngle));
    
    if (totalSlip > 0.3f) {
        // Reduce forces when combined slip is high
        float reduction = 0.3f / totalSlip;
        forces.longitudinal *= reduction;
        forces.lateral *= reduction;
    }
    
    return forces;
}

void TireModel::updateTemperature(float dt, float slipEnergy) {
    // Heat generation from slip
    float heatGeneration = slipEnergy * 0.01f; // Convert slip energy to temperature rise
    
    // Cooling (convection to air)
    float cooling = (temperature - 25.0f) * 0.1f * dt;
    
    // Update temperature
    temperature += heatGeneration * dt - cooling;
    temperature = std::clamp(temperature, 0.0f, maxTemp);
}

void TireModel::updateWear(float dt, float slipEnergy) {
    // Wear increases with slip energy and temperature
    float tempFactor = 1.0f + (temperature - optimalTemp) / optimalTemp * 0.5f;
    float wearRate = slipEnergy * 0.00001f * tempFactor;
    
    wear += wearRate * dt;
    wear = std::clamp(wear, 0.0f, 1.0f);
}

float TireModel::getGripMultiplier() const {
    // Grip is optimal at operating temperature
    float tempDiff = std::abs(temperature - optimalTemp);
    float tempFactor = 1.0f - (tempDiff / optimalTemp) * 0.3f;
    return std::clamp(tempFactor, 0.5f, 1.1f);
}

void TireModel::setTireType(const std::string& type) {
    if (type == "street") {
        // Street tire - balanced
        coefficients.B_long = 10.0f;
        coefficients.C_long = 1.9f;
        coefficients.D_long = 1.0f;
        coefficients.loadSensitivity = 1.0f;
        optimalTemp = 60.0f;
    }
    else if (type == "sport") {
        // Sport tire - more grip
        coefficients.B_long = 12.0f;
        coefficients.C_long = 1.8f;
        coefficients.D_long = 1.2f;
        coefficients.loadSensitivity = 1.1f;
        optimalTemp = 80.0f;
    }
    else if (type == "racing") {
        // Racing slick - maximum grip
        coefficients.B_long = 15.0f;
        coefficients.C_long = 1.7f;
        coefficients.D_long = 1.5f;
        coefficients.loadSensitivity = 1.3f;
        optimalTemp = 100.0f;
    }
    else if (type == "offroad") {
        // Off-road tire - softer
        coefficients.B_long = 8.0f;
        coefficients.C_long = 2.0f;
        coefficients.D_long = 0.9f;
        coefficients.loadSensitivity = 0.9f;
        optimalTemp = 50.0f;
    }
}

