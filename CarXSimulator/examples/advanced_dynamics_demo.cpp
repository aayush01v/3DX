/**
 * Advanced Vehicle Dynamics Demo
 * 
 * Demonstrates professional-grade vehicle dynamics:
 * - Pacejka tire model
 * - Load transfer
 * - Aerodynamic downforce
 * - Tire temperature and wear
 * - Advanced suspension
 */

#include "PhysicsEngine.h"
#include "Car.h"
#include "Terrain.h"
#include "VehicleDynamics.h"
#include <iostream>
#include <iomanip>

void printTelemetry(Car* car) {
    std::cout << "\n=== Vehicle Telemetry ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    
    // Basic data
    std::cout << "Speed: " << car->getSpeed() * 3.6f << " km/h" << std::endl;
    std::cout << "RPM: " << car->getRPM() << std::endl;
    std::cout << "Gear: " << car->getGear() << std::endl;
    
    // Advanced data
    std::cout << "Downforce: " << car->getDownforceAmount() << " N" << std::endl;
    std::cout << "Roll angle: " << car->getRollAngle() * 57.3f << "°" << std::endl;
    std::cout << "Pitch angle: " << car->getPitchAngle() * 57.3f << "°" << std::endl;
    
    // Tire data
    std::cout << "\n--- Tire Data ---" << std::endl;
    const char* wheelNames[] = {"FL", "FR", "RL", "RR"};
    
    for (int i = 0; i < 4; ++i) {
        TireForces forces = car->getTireForces(i);
        float temp = car->getTireTemperature(i);
        float wear = car->getTireWear(i);
        
        std::cout << wheelNames[i] << ": "
                  << "Fx=" << forces.longitudinal << "N, "
                  << "Fy=" << forces.lateral << "N, "
                  << "T=" << temp << "°C, "
                  << "Wear=" << (wear * 100) << "%" << std::endl;
    }
}

void demo1_PacejkaTireModel() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Demo 1: Pacejka Tire Model" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // Test different tire types
    std::string tireTypes[] = {"street", "sport", "racing", "offroad"};
    
    for (const auto& type : tireTypes) {
        TireModel tire;
        tire.setTireType(type);
        
        TireState state;
        state.slipRatio = 0.1f;      // 10% slip
        state.slipAngle = 0.05f;     // ~3 degrees
        state.normalLoad = 4000.0f;  // 4000 N
        
        TireForces forces = tire.calculateForces(state);
        
        std::cout << "Tire type: " << type << std::endl;
        std::cout << "  Longitudinal force: " << forces.longitudinal << " N" << std::endl;
        std::cout << "  Lateral force: " << forces.lateral << " N" << std::endl;
        std::cout << "  Aligning moment: " << forces.aligningMoment << " Nm" << std::endl;
        std::cout << std::endl;
    }
}

void demo2_LoadTransfer() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Demo 2: Load Transfer" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    PhysicsEngine physics;
    auto terrain = std::make_unique<Terrain>(100, 100);
    terrain->generateByType(TerrainType::FLAT);
    physics.setTerrain(std::move(terrain));
    
    Car* car = physics.createCar(Vector3(0, 5, 0));
    car->setTireType("sport");
    
    std::cout << "Testing load transfer during acceleration..." << std::endl;
    
    // Accelerate hard
    car->applyThrottle(1.0f);
    
    for (int i = 0; i < 120; ++i) {
        physics.update(1.0f / 60.0f);
        
        if (i % 30 == 0) {
            std::cout << "\nTime: " << i/60.0f << "s" << std::endl;
            
            VehicleDynamics* dynamics = car->getVehicleDynamics();
            for (int w = 0; w < 4; ++w) {
                float load = dynamics->getWheel(w).normalLoad;
                std::cout << "  Wheel " << w << " load: " << load << " N" << std::endl;
            }
        }
    }
    
    std::cout << "\nNote: Rear wheels have more load during acceleration!" << std::endl;
}

void demo3_Aerodynamics() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Demo 3: Aerodynamic Downforce" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    PhysicsEngine physics;
    auto terrain = std::make_unique<Terrain>(100, 100);
    terrain->generateByType(TerrainType::FLAT);
    physics.setTerrain(std::move(terrain));
    
    // Test different downforce levels
    float downforceLevels[] = {0.0f, 0.5f, 1.0f, 2.0f};
    
    for (float df : downforceLevels) {
        Car* car = physics.createCar(Vector3(0, 5, 0));
        car->setDownforce(df);
        car->applyThrottle(1.0f);
        
        // Accelerate to high speed
        for (int i = 0; i < 600; ++i) {
            physics.update(1.0f / 60.0f);
        }
        
        float speed = car->getSpeed();
        float downforce = car->getDownforceAmount();
        
        std::cout << "Downforce coefficient: " << df << std::endl;
        std::cout << "  Speed: " << speed * 3.6f << " km/h" << std::endl;
        std::cout << "  Downforce: " << downforce << " N" << std::endl;
        std::cout << "  Effect: " << (downforce / (car->getSpeed() * car->getSpeed())) << " N/(m/s)²" << std::endl;
        std::cout << std::endl;
        
        physics.reset();
    }
}

void demo4_TireTemperature() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Demo 4: Tire Temperature & Wear" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    PhysicsEngine physics;
    auto terrain = std::make_unique<Terrain>(100, 100);
    terrain->generateByType(TerrainType::RACETRACK);
    physics.setTerrain(std::move(terrain));
    
    Car* car = physics.createCar(Vector3(0, 5, 0));
    car->setTireType("racing");
    
    std::cout << "Simulating 10 laps of aggressive driving..." << std::endl;
    
    // Aggressive driving for 10 minutes
    for (int lap = 0; lap < 10; ++lap) {
        std::cout << "\n--- Lap " << (lap + 1) << " ---" << std::endl;
        
        // One lap (60 seconds)
        for (int i = 0; i < 3600; ++i) {
            // Vary throttle and steering
            float t = i / 3600.0f;
            car->applyThrottle(0.7f + 0.3f * std::sin(t * 10.0f));
            car->applySteering(0.5f * std::sin(t * 5.0f));
            
            physics.update(1.0f / 60.0f);
        }
        
        // Check tire condition
        float avgTemp = 0.0f;
        float avgWear = 0.0f;
        
        for (int i = 0; i < 4; ++i) {
            avgTemp += car->getTireTemperature(i);
            avgWear += car->getTireWear(i);
        }
        
        avgTemp /= 4.0f;
        avgWear /= 4.0f;
        
        std::cout << "Average tire temp: " << avgTemp << "°C" << std::endl;
        std::cout << "Average tire wear: " << (avgWear * 100) << "%" << std::endl;
        
        if (avgTemp > 100.0f) {
            std::cout << "WARNING: Tires overheating!" << std::endl;
        }
        
        if (avgWear > 0.8f) {
            std::cout << "WARNING: Tires heavily worn, pit stop recommended!" << std::endl;
        }
    }
}

void demo5_SuspensionTuning() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Demo 5: Suspension Tuning" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    PhysicsEngine physics;
    auto terrain = std::make_unique<Terrain>(100, 100);
    terrain->generateByType(TerrainType::HILLS);
    physics.setTerrain(std::move(terrain));
    
    // Test different suspension setups
    struct Setup {
        std::string name;
        float springRate;
        float damperRate;
        float antiRoll;
    };
    
    Setup setups[] = {
        {"Comfort", 35000.0f, 2000.0f, 3000.0f},
        {"Sport", 50000.0f, 3000.0f, 5000.0f},
        {"Race", 65000.0f, 4500.0f, 10000.0f}
    };
    
    for (const auto& setup : setups) {
        Car* car = physics.createCar(Vector3(0, 5, 0));
        
        SuspensionParameters suspension;
        suspension.springRate = setup.springRate;
        suspension.damperRate = setup.damperRate;
        suspension.antiRollStiffness = setup.antiRoll;
        
        car->getVehicleDynamics()->setSuspension(suspension);
        car->applyThrottle(0.8f);
        car->applySteering(0.3f);
        
        // Drive for 5 seconds
        float maxRoll = 0.0f;
        float maxPitch = 0.0f;
        
        for (int i = 0; i < 300; ++i) {
            physics.update(1.0f / 60.0f);
            
            maxRoll = std::max(maxRoll, std::abs(car->getRollAngle()));
            maxPitch = std::max(maxPitch, std::abs(car->getPitchAngle()));
        }
        
        std::cout << "Setup: " << setup.name << std::endl;
        std::cout << "  Max roll: " << maxRoll * 57.3f << "°" << std::endl;
        std::cout << "  Max pitch: " << maxPitch * 57.3f << "°" << std::endl;
        std::cout << "  Feel: " << (maxRoll < 0.1f ? "Stiff" : "Soft") << std::endl;
        std::cout << std::endl;
        
        physics.reset();
    }
}

void demo6_DrivetrainComparison() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Demo 6: Drivetrain Comparison" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    PhysicsEngine physics;
    auto terrain = std::make_unique<Terrain>(100, 100);
    terrain->generateByType(TerrainType::FLAT);
    physics.setTerrain(std::move(terrain));
    
    const char* driveTypes[] = {"FWD", "RWD", "AWD"};
    
    for (int type = 0; type < 3; ++type) {
        Car* car = physics.createCar(Vector3(0, 5, 0));
        car->setDriveType(type);
        car->applyThrottle(1.0f);
        
        // 0-100 km/h test
        float time = 0.0f;
        while (car->getSpeed() * 3.6f < 100.0f && time < 20.0f) {
            physics.update(1.0f / 60.0f);
            time += 1.0f / 60.0f;
        }
        
        std::cout << driveTypes[type] << " 0-100 km/h: " << time << " seconds" << std::endl;
        
        physics.reset();
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Advanced Vehicle Dynamics Demo" << std::endl;
    std::cout << "Version 2.2" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        demo1_PacejkaTireModel();
        demo2_LoadTransfer();
        demo3_Aerodynamics();
        demo4_TireTemperature();
        demo5_SuspensionTuning();
        demo6_DrivetrainComparison();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "All demos completed successfully!" << std::endl;
        std::cout << "========================================" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
