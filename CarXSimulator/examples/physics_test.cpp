/**
 * Physics Test - Verify all fixes are working
 * 
 * Tests:
 * 1. Ground contact and free fall
 * 2. Acceleration and gear shifting
 * 3. Braking and ABS
 * 4. Steering response
 * 5. Terrain friction
 * 6. Jump and landing
 */

#include "PhysicsEngine.h"
#include "Car.h"
#include "Terrain.h"
#include <iostream>
#include <iomanip>

void printStatus(const std::string& test, bool passed) {
    std::cout << "[" << (passed ? "✓" : "✗") << "] " << test << std::endl;
}

void printCarState(Car* car, const std::string& label) {
    std::cout << "\n--- " << label << " ---" << std::endl;
    std::cout << "Position: (" << std::fixed << std::setprecision(2) 
              << car->getPosition().x << ", " 
              << car->getPosition().y << ", " 
              << car->getPosition().z << ")" << std::endl;
    std::cout << "Speed: " << car->getSpeed() << " m/s" << std::endl;
    std::cout << "RPM: " << car->getRPM() << std::endl;
    std::cout << "Gear: " << car->getGear() << std::endl;
    std::cout << "Airborne: " << (car->isAirborne() ? "Yes" : "No") << std::endl;
}

bool test1_GroundContact() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test 1: Ground Contact & Free Fall" << std::endl;
    std::cout << "========================================" << std::endl;
    
    PhysicsEngine physics;
    auto terrain = std::make_unique<Terrain>(100, 100);
    terrain->generateByType(TerrainType::FLAT);
    physics.setTerrain(std::move(terrain));
    
    // Create car above ground
    Car* car = physics.createCar(Vector3(0, 10, 0));
    
    std::cout << "Car created at height 10m" << std::endl;
    printCarState(car, "Initial State");
    
    // Let it fall
    for (int i = 0; i < 120; ++i) {
        physics.update(1.0f / 60.0f);
    }
    
    printCarState(car, "After 2 seconds");
    
    // Check if car is on ground
    float finalHeight = car->getPosition().y;
    bool onGround = !car->isAirborne();
    bool correctHeight = finalHeight < 1.0f && finalHeight > 0.0f;
    
    printStatus("Car landed on ground", onGround);
    printStatus("Car at correct height", correctHeight);
    
    return onGround && correctHeight;
}

bool test2_Acceleration() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test 2: Acceleration & Gear Shifting" << std::endl;
    std::cout << "========================================" << std::endl;
    
    PhysicsEngine physics;
    auto terrain = std::make_unique<Terrain>(100, 100);
    terrain->generateByType(TerrainType::FLAT);
    physics.setTerrain(std::move(terrain));
    
    Car* car = physics.createCar(Vector3(0, 5, 0));
    car->applyThrottle(1.0f);
    
    std::cout << "Full throttle applied" << std::endl;
    printCarState(car, "Initial State");
    
    int gearChanges = 0;
    int lastGear = car->getGear();
    
    // Accelerate for 10 seconds
    for (int i = 0; i < 600; ++i) {
        physics.update(1.0f / 60.0f);
        
        if (car->getGear() != lastGear) {
            gearChanges++;
            std::cout << "Gear change: " << lastGear << " -> " << car->getGear() 
                     << " at " << car->getSpeed() << " m/s" << std::endl;
            lastGear = car->getGear();
        }
        
        if (i % 120 == 0) {
            std::cout << "t=" << i/60 << "s: Speed=" << car->getSpeed() 
                     << " m/s, RPM=" << car->getRPM() 
                     << ", Gear=" << car->getGear() << std::endl;
        }
    }
    
    printCarState(car, "After 10 seconds");
    
    bool accelerated = car->getSpeed() > 20.0f;
    bool gearShifted = gearChanges >= 2;
    
    printStatus("Car accelerated (>20 m/s)", accelerated);
    printStatus("Gears shifted (>=2 times)", gearShifted);
    
    return accelerated && gearShifted;
}

bool test3_Braking() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test 3: Braking & ABS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    PhysicsEngine physics;
    auto terrain = std::make_unique<Terrain>(100, 100);
    terrain->generateByType(TerrainType::FLAT);
    physics.setTerrain(std::move(terrain));
    
    Car* car = physics.createCar(Vector3(0, 5, 0));
    car->setAntiLockBrakes(true);
    
    // Accelerate first
    car->applyThrottle(1.0f);
    for (int i = 0; i < 300; ++i) {
        physics.update(1.0f / 60.0f);
    }
    
    float initialSpeed = car->getSpeed();
    std::cout << "Initial speed: " << initialSpeed << " m/s" << std::endl;
    
    // Now brake
    car->applyThrottle(0.0f);
    car->applyBrake(1.0f);
    
    std::cout << "Full brake applied with ABS" << std::endl;
    
    int frames = 0;
    while (car->getSpeed() > 0.5f && frames < 300) {
        physics.update(1.0f / 60.0f);
        frames++;
        
        if (frames % 30 == 0) {
            std::cout << "t=" << frames/60.0f << "s: Speed=" << car->getSpeed() << " m/s" << std::endl;
        }
    }
    
    float finalSpeed = car->getSpeed();
    float brakingTime = frames / 60.0f;
    
    printCarState(car, "After Braking");
    std::cout << "Braking time: " << brakingTime << " seconds" << std::endl;
    
    bool stopped = finalSpeed < 1.0f;
    bool reasonableTime = brakingTime < 5.0f;
    
    printStatus("Car stopped", stopped);
    printStatus("Reasonable braking time (<5s)", reasonableTime);
    
    return stopped && reasonableTime;
}

bool test4_Steering() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test 4: Steering Response" << std::endl;
    std::cout << "========================================" << std::endl;
    
    PhysicsEngine physics;
    auto terrain = std::make_unique<Terrain>(100, 100);
    terrain->generateByType(TerrainType::FLAT);
    physics.setTerrain(std::move(terrain));
    
    Car* car = physics.createCar(Vector3(0, 5, 0));
    
    // Accelerate to moderate speed
    car->applyThrottle(0.7f);
    for (int i = 0; i < 180; ++i) {
        physics.update(1.0f / 60.0f);
    }
    
    Vector3 initialPos = car->getPosition();
    std::cout << "Speed: " << car->getSpeed() << " m/s" << std::endl;
    std::cout << "Initial position: (" << initialPos.x << ", " << initialPos.z << ")" << std::endl;
    
    // Apply steering
    car->applySteering(0.5f);
    std::cout << "Steering applied (50% right)" << std::endl;
    
    for (int i = 0; i < 180; ++i) {
        physics.update(1.0f / 60.0f);
    }
    
    Vector3 finalPos = car->getPosition();
    std::cout << "Final position: (" << finalPos.x << ", " << finalPos.z << ")" << std::endl;
    
    float lateralMovement = std::abs(finalPos.x - initialPos.x);
    
    printCarState(car, "After Steering");
    std::cout << "Lateral movement: " << lateralMovement << " m" << std::endl;
    
    bool turned = lateralMovement > 5.0f;
    
    printStatus("Car turned (>5m lateral)", turned);
    
    return turned;
}

bool test5_TerrainFriction() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test 5: Terrain Friction" << std::endl;
    std::cout << "========================================" << std::endl;
    
    PhysicsEngine physics;
    auto terrain = std::make_unique<Terrain>(100, 100);
    terrain->generateByType(TerrainType::FLAT);
    
    // Create ice patch
    terrain->setMaterialZone(0, 0, 20, 0.3f);
    physics.setTerrain(std::move(terrain));
    
    Car* car = physics.createCar(Vector3(0, 5, 0));
    car->setTractionControl(true);
    
    std::cout << "Car on ice (friction 0.3)" << std::endl;
    
    car->applyThrottle(1.0f);
    
    for (int i = 0; i < 180; ++i) {
        physics.update(1.0f / 60.0f);
    }
    
    float iceSpeed = car->getSpeed();
    std::cout << "Speed on ice: " << iceSpeed << " m/s" << std::endl;
    
    // Now test on normal surface
    PhysicsEngine physics2;
    auto terrain2 = std::make_unique<Terrain>(100, 100);
    terrain2->generateByType(TerrainType::FLAT);
    physics2.setTerrain(std::move(terrain2));
    
    Car* car2 = physics2.createCar(Vector3(0, 5, 0));
    car2->setTractionControl(true);
    car2->applyThrottle(1.0f);
    
    for (int i = 0; i < 180; ++i) {
        physics2.update(1.0f / 60.0f);
    }
    
    float normalSpeed = car2->getSpeed();
    std::cout << "Speed on asphalt: " << normalSpeed << " m/s" << std::endl;
    
    bool slowerOnIce = iceSpeed < normalSpeed * 0.8f;
    
    printStatus("Slower on ice (<80% of normal)", slowerOnIce);
    
    return slowerOnIce;
}

bool test6_JumpLanding() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test 6: Jump & Landing" << std::endl;
    std::cout << "========================================" << std::endl;
    
    PhysicsEngine physics;
    auto terrain = std::make_unique<Terrain>(100, 100);
    terrain->generateByType(TerrainType::FLAT);
    terrain->addRamp(15, 20, 10, 20, 8);
    physics.setTerrain(std::move(terrain));
    
    Car* car = physics.createCar(Vector3(0, 5, 0));
    car->applyThrottle(1.0f);
    
    std::cout << "Accelerating towards ramp..." << std::endl;
    
    bool wasAirborne = false;
    bool landed = false;
    float maxHeight = 0;
    
    for (int i = 0; i < 600; ++i) {
        physics.update(1.0f / 60.0f);
        
        if (car->isAirborne()) {
            wasAirborne = true;
            maxHeight = std::max(maxHeight, car->getPosition().y);
            
            if (i % 30 == 0) {
                std::cout << "Airborne! Height: " << car->getPosition().y << " m" << std::endl;
            }
        }
        
        if (wasAirborne && !car->isAirborne()) {
            landed = true;
            std::cout << "Landed! Max height was: " << maxHeight << " m" << std::endl;
            break;
        }
    }
    
    printCarState(car, "After Landing");
    
    bool jumped = wasAirborne && maxHeight > 6.0f;
    bool landedSafely = landed && !car->isAirborne();
    
    printStatus("Car jumped (>6m height)", jumped);
    printStatus("Car landed safely", landedSafely);
    
    return jumped && landedSafely;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "CarX Simulator - Physics Test Suite" << std::endl;
    std::cout << "Version 2.1" << std::endl;
    std::cout << "========================================" << std::endl;
    
    int passed = 0;
    int total = 6;
    
    if (test1_GroundContact()) passed++;
    if (test2_Acceleration()) passed++;
    if (test3_Braking()) passed++;
    if (test4_Steering()) passed++;
    if (test5_TerrainFriction()) passed++;
    if (test6_JumpLanding()) passed++;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Results: " << passed << "/" << total << " passed" << std::endl;
    
    if (passed == total) {
        std::cout << "✓ All tests passed! Physics working correctly." << std::endl;
    } else {
        std::cout << "✗ Some tests failed. Check output above." << std::endl;
    }
    
    std::cout << "========================================" << std::endl;
    
    return (passed == total) ? 0 : 1;
}
