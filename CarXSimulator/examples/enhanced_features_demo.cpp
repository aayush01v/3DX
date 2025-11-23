/**
 * Enhanced Features Demo
 * 
 * Demonstrates all the new features added in v2.0:
 * - Advanced terrain generation with obstacles and ramps
 * - Enhanced car physics with suspension and driver aids
 * - Improved coding support with scripting
 */

#include "PhysicsEngine.h"
#include "CodeExecutor.h"
#include "Car.h"
#include "Terrain.h"
#include <iostream>
#include <thread>
#include <chrono>

void printCarTelemetry(Car* car) {
    std::cout << "\n=== Car Telemetry ===" << std::endl;
    std::cout << "Position: " << car->getPosition().toString() << std::endl;
    std::cout << "Speed: " << car->getSpeed() << " m/s" << std::endl;
    std::cout << "RPM: " << car->getRPM() << std::endl;
    std::cout << "Gear: " << car->getGear() << std::endl;
    std::cout << "Slip Angle: " << car->getSlipAngle() << " rad" << std::endl;
    std::cout << "Lateral G: " << car->getLateralAcceleration() << " m/s²" << std::endl;
    std::cout << "Airborne: " << (car->isAirborne() ? "Yes" : "No") << std::endl;
    
    std::cout << "\nSuspension Compression:" << std::endl;
    for (int i = 0; i < 4; ++i) {
        std::cout << "  Wheel " << i << ": " << car->getSuspensionCompression(i) << " m" << std::endl;
    }
}

void demo1_AdvancedTerrain() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Demo 1: Advanced Terrain Generation" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    PhysicsEngine physics;
    
    // Create terrain with mountains
    auto terrain = std::make_unique<Terrain>(100, 100);
    terrain->generateByType(TerrainType::MOUNTAINS);
    
    std::cout << "Generated mountain terrain" << std::endl;
    
    // Add a jump ramp
    terrain->addRamp(20, 30, 10, 20, 8);
    std::cout << "Added jump ramp at (20, 30)" << std::endl;
    
    // Add obstacles
    terrain->addObstacle(-15, -15, 8, 5);
    terrain->addObstacle(25, -20, 6, 4);
    std::cout << "Added 2 obstacles" << std::endl;
    
    // Smooth rough areas
    terrain->smoothTerrain(2);
    std::cout << "Applied smoothing (2 iterations)" << std::endl;
    
    // Add ice patch
    terrain->setMaterialZone(0, 0, 15, 0.4f);
    std::cout << "Added ice patch at center (friction: 0.4)" << std::endl;
    
    // Apply erosion for realism
    terrain->applyErosion(0.1f);
    std::cout << "Applied erosion simulation" << std::endl;
    
    physics.setTerrain(std::move(terrain));
    
    std::cout << "\nTerrain setup complete!" << std::endl;
}

void demo2_EnhancedCarPhysics() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Demo 2: Enhanced Car Physics" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    PhysicsEngine physics;
    
    // Create flat terrain for testing
    auto terrain = std::make_unique<Terrain>(100, 100);
    terrain->generateByType(TerrainType::FLAT);
    physics.setTerrain(std::move(terrain));
    
    // Create a performance car
    Car* car = physics.createCar(Vector3(0, 5, 0));
    
    // Configure mass and power
    car->setMass(1100.0f);  // Lightweight
    car->setEnginePower(200000.0f);  // High power
    std::cout << "Created performance car (1100 kg, 200 kW)" << std::endl;
    
    // Configure suspension
    car->setSuspensionStiffness(55000.0f);  // Stiff for racing
    car->setSuspensionDamping(3500.0f);
    std::cout << "Configured racing suspension" << std::endl;
    
    // Configure tires
    car->setTireGrip(1.5f);  // Racing slicks
    std::cout << "Set racing slicks (grip: 1.5)" << std::endl;
    
    // Enable driver aids
    car->setTractionControl(true);
    car->setAntiLockBrakes(true);
    std::cout << "Enabled traction control and ABS" << std::endl;
    
    // Set differential
    car->setDifferentialType(1);  // Limited-slip
    std::cout << "Set limited-slip differential" << std::endl;
    
    // Configure aerodynamics
    car->setDragCoefficient(0.28f);  // Low drag
    std::cout << "Set drag coefficient: 0.28" << std::endl;
    
    // Simulate driving
    std::cout << "\nSimulating acceleration..." << std::endl;
    car->applyThrottle(1.0f);
    
    for (int i = 0; i < 300; ++i) {
        physics.update(1.0f / 60.0f);
        
        if (i % 60 == 0) {
            std::cout << "Time: " << i/60 << "s, Speed: " << car->getSpeed() 
                     << " m/s, RPM: " << car->getRPM() 
                     << ", Gear: " << car->getGear() << std::endl;
        }
    }
    
    printCarTelemetry(car);
}

void demo3_ScriptingSystem() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Demo 3: Enhanced Scripting System" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    PhysicsEngine physics;
    CodeExecutor executor(&physics);
    
    // Show available commands
    std::cout << "Available commands:" << std::endl;
    auto commands = executor.getAvailableCommands();
    for (size_t i = 0; i < commands.size() && i < 10; ++i) {
        std::cout << "  - " << commands[i] << std::endl;
    }
    std::cout << "  ... and " << (commands.size() - 10) << " more\n" << std::endl;
    
    // Get command help
    std::cout << "Command help for 'createCar':" << std::endl;
    std::cout << executor.getCommandHelp("createCar") << "\n" << std::endl;
    
    // Execute script
    std::string script = R"(
        # Setup terrain
        loadTerrainPreset hills
        
        # Add features
        addRamp 15 20 10 20 8
        addObstacle -10 -10 5 4
        
        # Create car
        createCar 0 10 0 1200 150000
        
        # Configure car
        setTireGrip 1.2
        setTractionControl 1
        setABS 1
        
        # Set controls
        setThrottle 0.8
        setSteering 0.2
    )";
    
    std::cout << "Executing script..." << std::endl;
    bool success = executor.executeScript(script);
    
    if (success) {
        std::cout << "Script executed successfully!" << std::endl;
        std::cout << "Output: " << executor.getLastOutput() << std::endl;
    } else {
        std::cout << "Script error: " << executor.getLastError() << std::endl;
    }
    
    // Use variables
    std::cout << "\nUsing variables:" << std::endl;
    executor.execute("mass = 1500");
    executor.execute("power = 180000");
    std::cout << "Set mass = " << executor.getVariable("mass") << std::endl;
    std::cout << "Set power = " << executor.getVariable("power") << std::endl;
    
    // Get suggestions
    std::cout << "\nCommand suggestions for 'set':" << std::endl;
    auto suggestions = executor.getSuggestions("set");
    for (const auto& suggestion : suggestions) {
        std::cout << "  - " << suggestion << std::endl;
    }
}

void demo4_CompleteScenario() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Demo 4: Complete Rally Scenario" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    PhysicsEngine physics;
    
    // Configure physics
    PhysicsConfig config;
    config.substeps = 6;
    config.useAdaptiveSubsteps = true;
    config.enableCollisionCache = true;
    physics.setConfig(config);
    std::cout << "Configured physics engine" << std::endl;
    
    // Create rally terrain
    auto terrain = std::make_unique<Terrain>(100, 100);
    terrain->generateByType(TerrainType::HILLS);
    terrain->addRamp(25, 30, 12, 25, 10);  // Big jump
    terrain->addObstacle(40, 40, 10, 6);   // Hill
    terrain->setMaterialZone(-20, -20, 12, 0.6f);  // Mud section
    terrain->smoothTerrain(1);
    physics.setTerrain(std::move(terrain));
    std::cout << "Created rally terrain with jump, hill, and mud section" << std::endl;
    
    // Create rally car
    Car* car = physics.createCar(Vector3(0, 10, 0));
    car->setMass(1150.0f);
    car->setEnginePower(180000.0f);
    car->setSuspensionStiffness(40000.0f);  // Softer for off-road
    car->setSuspensionDamping(2800.0f);
    car->setTireGrip(1.1f);  // All-terrain tires
    car->setTractionControl(true);
    car->setAntiLockBrakes(true);
    car->setDifferentialType(1);  // Limited-slip
    std::cout << "Created rally car with off-road setup" << std::endl;
    
    // Set wind
    physics.setWindVelocity(Vector3(3, 0, 0));
    std::cout << "Set wind velocity: 3 m/s" << std::endl;
    
    // Simulate rally stage
    std::cout << "\nSimulating rally stage..." << std::endl;
    car->applyThrottle(0.9f);
    car->applySteering(0.15f);
    
    for (int i = 0; i < 600; ++i) {
        physics.update(1.0f / 60.0f);
        
        if (i % 120 == 0) {
            std::cout << "\n--- Time: " << i/60 << "s ---" << std::endl;
            std::cout << "Speed: " << car->getSpeed() << " m/s" << std::endl;
            std::cout << "Position: " << car->getPosition().toString() << std::endl;
            std::cout << "Airborne: " << (car->isAirborne() ? "Yes" : "No") << std::endl;
            
            // Check if on mud
            Vector3 pos = car->getPosition();
            float friction = physics.getTerrain()->getFriction(pos.x, pos.z);
            if (friction < 0.7f) {
                std::cout << "On mud section! (friction: " << friction << ")" << std::endl;
            }
        }
        
        // Adjust steering for terrain
        if (i == 300) {
            car->applySteering(-0.2f);
            std::cout << "\nAdjusting steering for turn..." << std::endl;
        }
    }
    
    std::cout << "\nFinal telemetry:" << std::endl;
    printCarTelemetry(car);
    
    // Performance stats
    std::cout << "\n=== Performance Stats ===" << std::endl;
    std::cout << "Average update time: " << physics.getAverageUpdateTime() << " ms" << std::endl;
    std::cout << "Current substeps: " << physics.getCurrentSubsteps() << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "CarX Simulator - Enhanced Features Demo" << std::endl;
    std::cout << "Version 2.0" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        // Run all demos
        demo1_AdvancedTerrain();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        demo2_EnhancedCarPhysics();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        demo3_ScriptingSystem();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        demo4_CompleteScenario();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "All demos completed successfully!" << std::endl;
        std::cout << "========================================" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
