// Example: Advanced physics demonstration with performance benchmarks
#include "Engine.h"
#include "PhysicsEngine.h"
#include "Car.h"
#include "Terrain.h"
#include <iostream>
#include <iomanip>
#include <chrono>

void printCarState(const Car* car, int frame) {
    std::cout << "\n=== Frame " << frame << " ===\n";
    std::cout << std::fixed << std::setprecision(2);
    
    Vector3 pos = car->getPosition();
    std::cout << "Position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")\n";
    std::cout << "Speed: " << car->getSpeed() << " m/s (" 
              << car->getSpeed() * 3.6f << " km/h)\n";
    std::cout << "RPM: " << car->getRPM() << "\n";
    std::cout << "Gear: " << car->getGear() + 1 << "/6\n";
    
    Vector3 centripetal = car->getCentripetalForce();
    float cfMagnitude = centripetal.length();
    std::cout << "Centripetal Force: " << cfMagnitude << " N\n";
    
    float slipAngle = car->getSlipAngle() * 180.0f / 3.14159f;
    std::cout << "Slip Angle: " << slipAngle << "°\n";
    
    float lateralG = car->getLateralAcceleration() / 9.81f;
    std::cout << "Lateral G-Force: " << lateralG << "g\n";
    
    float longitudinalG = car->getLongitudinalAcceleration() / 9.81f;
    std::cout << "Longitudinal G-Force: " << longitudinalG << "g\n";
    
    if (car->isAirborne()) {
        std::cout << "Status: ⚠️ AIRBORNE\n";
    } else {
        std::cout << "Status: ✓ On Ground\n";
    }
}

void printPerformanceStats(const PhysicsEngine* physics) {
    std::cout << "\n--- Performance Statistics ---\n";
    std::cout << "Average Update Time: " << physics->getAverageUpdateTime() << " ms\n";
    std::cout << "Current Substeps: " << physics->getCurrentSubsteps() << "\n";
    std::cout << "Active Cars: " << physics->getCarCount() << "\n";
}

int main() {
    std::cout << "=== CarX Advanced Physics Simulator ===\n";
    std::cout << "High-Performance Physics Engine Demo\n\n";
    
    Engine engine;
    engine.initialize(1920, 1080);
    
    PhysicsEngine* physics = engine.getPhysics();
    
    // Configure physics for optimal performance
    PhysicsConfig config;
    config.substeps = 4;
    config.useAdaptiveSubsteps = true;
    config.enableCollisionCache = true;
    config.collisionCacheSize = 512;
    physics->setConfig(config);
    
    // Set environmental conditions
    physics->setAirDensity(1.225f); // Sea level
    physics->setWindVelocity(Vector3(2.0f, 0, 0)); // 2 m/s crosswind
    
    // Create procedurally generated race track
    auto terrain = std::make_unique<Terrain>(200, 200);
    terrain->generateRaceTrack(50.0f, 15.0f, 0.2f);
    terrain->setCurvature(0.25f);
    physics->setTerrain(std::move(terrain));
    
    std::cout << "✓ Terrain generated: Race Track (50m radius, 15m width)\n";
    std::cout << "✓ Physics configured: 4 substeps, adaptive, cached\n";
    std::cout << "✓ Environment: Air density 1.225 kg/m³, 2 m/s crosswind\n\n";
    
    // Create high-performance racing car
    Car* car = physics->createCar(Vector3(0, 5, 0));
    car->setMass(1100.0f);           // Lightweight racing car
    car->setEnginePower(300000.0f);  // 300 kW (400 HP)
    car->setTireGrip(1.8f);          // Racing slicks
    car->setDragCoefficient(0.28f);  // Aerodynamic body
    car->setFrontalArea(2.2f);       // m²
    
    std::cout << "Car Configuration:\n";
    std::cout << "  Mass: 1100 kg\n";
    std::cout << "  Power: 300 kW (400 HP)\n";
    std::cout << "  Tire Grip: 1.8x (Racing Slicks)\n";
    std::cout << "  Drag Coefficient: 0.28\n";
    std::cout << "  Frontal Area: 2.2 m²\n\n";
    
    // ===== Scenario 1: 0-100 km/h Acceleration Test =====
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Scenario 1: 0-100 km/h Acceleration Test\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    auto startTime = std::chrono::high_resolution_clock::now();
    int frames = 0;
    
    for (int i = 0; i < 300; ++i) {
        car->applyThrottle(1.0f);
        physics->update(1.0f / 60.0f);
        frames++;
        
        if (car->getSpeed() * 3.6f >= 100.0f) {
            auto endTime = std::chrono::high_resolution_clock::now();
            float duration = std::chrono::duration<float>(endTime - startTime).count();
            std::cout << "\n✓ 0-100 km/h achieved in " << duration << " seconds!\n";
            std::cout << "  Frames: " << frames << " (" << frames / 60.0f << "s @ 60 FPS)\n";
            printCarState(car, frames);
            break;
        }
        
        if (i % 60 == 0) {
            printCarState(car, i);
        }
    }
    
    printPerformanceStats(physics);
    
    // ===== Scenario 2: High-Speed Cornering Test =====
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Scenario 2: High-Speed Cornering Test\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    car->reset(Vector3(0, 5, 0));
    
    float maxLateralG = 0.0f;
    float maxSpeed = 0.0f;
    
    for (int i = 0; i < 240; ++i) {
        car->applyThrottle(0.85f);
        car->applySteering(0.6f);  // Sustained turn
        physics->update(1.0f / 60.0f);
        
        float lateralG = std::abs(car->getLateralAcceleration() / 9.81f);
        maxLateralG = std::max(maxLateralG, lateralG);
        maxSpeed = std::max(maxSpeed, car->getSpeed());
        
        if (i % 60 == 0) {
            printCarState(car, i);
        }
    }
    
    std::cout << "\n📊 Cornering Statistics:\n";
    std::cout << "  Max Lateral G-Force: " << maxLateralG << "g\n";
    std::cout << "  Max Cornering Speed: " << maxSpeed * 3.6f << " km/h\n";
    
    printPerformanceStats(physics);
    
    // ===== Scenario 3: Emergency Braking Test =====
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Scenario 3: 100-0 km/h Emergency Braking\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    car->reset(Vector3(0, 5, 0));
    
    // Accelerate to 100 km/h
    while (car->getSpeed() * 3.6f < 100.0f) {
        car->applyThrottle(1.0f);
        physics->update(1.0f / 60.0f);
    }
    
    std::cout << "Initial speed: " << car->getSpeed() * 3.6f << " km/h\n";
    
    startTime = std::chrono::high_resolution_clock::now();
    frames = 0;
    float maxBrakeG = 0.0f;
    
    while (car->getSpeed() > 0.1f && frames < 300) {
        car->applyThrottle(0.0f);
        car->applyBrake(1.0f);
        physics->update(1.0f / 60.0f);
        frames++;
        
        float brakeG = std::abs(car->getLongitudinalAcceleration() / 9.81f);
        maxBrakeG = std::max(maxBrakeG, brakeG);
        
        if (frames % 30 == 0) {
            std::cout << "Frame " << frames << " - Speed: " 
                     << car->getSpeed() * 3.6f << " km/h\n";
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    float brakeDuration = std::chrono::duration<float>(endTime - startTime).count();
    
    std::cout << "\n✓ Stopped in " << brakeDuration << " seconds\n";
    std::cout << "  Frames: " << frames << " (" << frames / 60.0f << "s @ 60 FPS)\n";
    std::cout << "  Max Braking G-Force: " << maxBrakeG << "g\n";
    std::cout << "  Braking Distance: ~" << (100.0f / 3.6f) * brakeDuration / 2.0f << " meters\n";
    
    printPerformanceStats(physics);
    
    // ===== Scenario 4: Multi-Car Stress Test =====
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Scenario 4: Multi-Car Performance Test\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    // Create multiple cars
    std::vector<Car*> testCars;
    for (int i = 0; i < 10; ++i) {
        float angle = (i / 10.0f) * 2.0f * 3.14159f;
        float radius = 30.0f;
        Vector3 pos(radius * std::cos(angle), 5, radius * std::sin(angle));
        Car* testCar = physics->createCar(pos);
        testCar->setMass(1200.0f);
        testCar->setEnginePower(200000.0f);
        testCars.push_back(testCar);
    }
    
    std::cout << "Created " << testCars.size() << " cars for stress test\n";
    
    startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 300; ++i) {
        for (auto* testCar : testCars) {
            testCar->applyThrottle(0.7f);
            testCar->applySteering(0.3f * std::sin(i * 0.05f));
        }
        physics->update(1.0f / 60.0f);
    }
    
    endTime = std::chrono::high_resolution_clock::now();
    float totalTime = std::chrono::duration<float>(endTime - startTime).count();
    
    std::cout << "\n📊 Stress Test Results:\n";
    std::cout << "  Total Cars: " << physics->getCarCount() << "\n";
    std::cout << "  Simulation Time: " << totalTime << " seconds\n";
    std::cout << "  Average FPS: " << 300.0f / totalTime << "\n";
    std::cout << "  Physics Update Time: " << physics->getAverageUpdateTime() << " ms\n";
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "✓ All Scenarios Complete!\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    return 0;
}
