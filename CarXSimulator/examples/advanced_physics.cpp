// Example: Advanced physics demonstration
#include "Engine.h"
#include "PhysicsEngine.h"
#include "Car.h"
#include "Terrain.h"
#include <iostream>
#include <iomanip>

void printCarState(const Car* car, int frame) {
    std::cout << "\n=== Frame " << frame << " ===\n";
    std::cout << std::fixed << std::setprecision(2);
    
    Vector3 pos = car->getPosition();
    std::cout << "Position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")\n";
    std::cout << "Speed: " << car->getSpeed() << " m/s\n";
    std::cout << "RPM: " << car->getRPM() << "\n";
    std::cout << "Gear: " << car->getGear() + 1 << "\n";
    
    Vector3 centripetal = car->getCentripetalForce();
    std::cout << "Centripetal Force: (" 
              << centripetal.x << ", " << centripetal.y << ", " << centripetal.z << ") N\n";
    
    float slipAngle = car->getSlipAngle() * 180.0f / 3.14159f;
    std::cout << "Slip Angle: " << slipAngle << " degrees\n";
    
    float lateralG = car->getLateralAcceleration() / 9.81f;
    std::cout << "Lateral G-Force: " << lateralG << "g\n";
    
    if (car->isAirborne()) {
        std::cout << "Status: AIRBORNE\n";
    }
}

int main() {
    Engine engine;
    engine.initialize(1280, 720);
    
    PhysicsEngine* physics = engine.getPhysics();
    
    // Create race track
    auto terrain = std::make_unique<Terrain>(150, 150);
    terrain->generateRaceTrack(40.0f, 12.0f, 0.15f);
    terrain->setCurvature(0.2f);
    physics->setTerrain(std::move(terrain));
    
    // Create high-performance car
    Car* car = physics->createCar(Vector3(0, 5, 0));
    car->setMass(1100.0f);  // Lightweight
    car->setEnginePower(250000.0f);  // High power
    car->setTireGrip(1.5f);  // Racing tires
    car->setDragCoefficient(0.25f);  // Aerodynamic
    
    std::cout << "=== Advanced Physics Simulation ===\n";
    std::cout << "Car Configuration:\n";
    std::cout << "  Mass: 1100 kg\n";
    std::cout << "  Power: 250 kW\n";
    std::cout << "  Tire Grip: 1.5x\n\n";
    
    // Simulation scenarios
    std::cout << "\n--- Scenario 1: Acceleration Test ---\n";
    for (int i = 0; i < 120; ++i) {
        car->applyThrottle(1.0f);
        physics->update(1.0f / 60.0f);
        
        if (i % 30 == 0) {
            printCarState(car, i);
        }
    }
    
    std::cout << "\n--- Scenario 2: High-Speed Cornering ---\n";
    car->reset(Vector3(0, 5, 0));
    
    for (int i = 0; i < 180; ++i) {
        car->applyThrottle(0.8f);
        car->applySteering(0.7f);  // Sharp turn
        physics->update(1.0f / 60.0f);
        
        if (i % 30 == 0) {
            printCarState(car, i);
        }
    }
    
    std::cout << "\n--- Scenario 3: Emergency Braking ---\n";
    car->reset(Vector3(0, 5, 0));
    
    // Accelerate first
    for (int i = 0; i < 60; ++i) {
        car->applyThrottle(1.0f);
        physics->update(1.0f / 60.0f);
    }
    
    std::cout << "Initial speed: " << car->getSpeed() << " m/s\n";
    
    // Hard braking
    for (int i = 0; i < 90; ++i) {
        car->applyThrottle(0.0f);
        car->applyBrake(1.0f);
        physics->update(1.0f / 60.0f);
        
        if (i % 15 == 0) {
            std::cout << "Frame " << i << " - Speed: " << car->getSpeed() << " m/s\n";
        }
        
        if (car->getSpeed() < 0.1f) {
            std::cout << "Stopped at frame " << i << "\n";
            break;
        }
    }
    
    std::cout << "\n=== Simulation Complete ===\n";
    
    return 0;
}
