// Example: Basic car physics simulation
#include "Engine.h"
#include "PhysicsEngine.h"
#include "Car.h"
#include "Terrain.h"

int main() {
    Engine engine;
    engine.initialize(1280, 720);
    
    // Create terrain
    auto terrain = std::make_unique<Terrain>(100, 100);
    terrain->generateProcedural(5.0f, 2.0f);
    terrain->setCurvature(0.3f);
    
    PhysicsEngine* physics = engine.getPhysics();
    physics->setTerrain(std::move(terrain));
    
    // Create car
    Car* car = physics->createCar(Vector3(0, 5, 0));
    car->setMass(1200.0f);
    car->setWheelbase(2.5f);
    
    // Simulate
    for (int i = 0; i < 100; ++i) {
        car->applyThrottle(0.8f);
        car->applySteering(0.1f);
        
        physics->update(1.0f / 60.0f);
        
        // Print forces
        Vector3 centripetal = car->getCentripetalForce();
        Vector3 centrifugal = car->getCentrifugalForce();
        
        printf("Frame %d:\n", i);
        printf("  Position: (%.2f, %.2f, %.2f)\n", 
               car->getPosition().x, car->getPosition().y, car->getPosition().z);
        printf("  Speed: %.2f m/s\n", car->getSpeed());
        printf("  Centripetal Force: (%.2f, %.2f, %.2f)\n",
               centripetal.x, centripetal.y, centripetal.z);
    }
    
    return 0;
}
