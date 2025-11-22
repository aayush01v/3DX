// Example: Car on curved terrain with gravity effects
#include "Engine.h"

int main() {
    Engine engine;
    engine.initialize(1280, 720);
    
    // Create highly curved terrain
    engine.executeUserCode("generateTerrain 10.0 4.0");
    
    PhysicsEngine* physics = engine.getPhysics();
    Terrain* terrain = physics->getTerrain();
    terrain->setCurvature(0.8f);  // High curvature affects gravity
    
    // Create car on slope
    Car* car = physics->createCar(Vector3(-20, 15, 0));
    car->setMass(1500.0f);
    
    // Simulate rolling down curved surface
    for (int i = 0; i < 200; ++i) {
        physics->update(1.0f / 60.0f);
        
        Vector3 pos = car->getPosition();
        Vector3 normal = terrain->getNormal(pos.x, pos.z);
        Vector3 gravityDir = terrain->getGravityDirection(pos.x, pos.z);
        
        if (i % 20 == 0) {
            printf("Time: %.2fs\n", i / 60.0f);
            printf("  Terrain Normal: (%.2f, %.2f, %.2f)\n",
                   normal.x, normal.y, normal.z);
            printf("  Gravity Direction: (%.2f, %.2f, %.2f)\n",
                   gravityDir.x, gravityDir.y, gravityDir.z);
            printf("  Car Speed: %.2f m/s\n\n", car->getSpeed());
        }
    }
    
    return 0;
}
