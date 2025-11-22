#include "PhysicsEngine.h"
#include "Car.h"
#include "Terrain.h"

PhysicsEngine::PhysicsEngine() 
    : gravity(0, -9.81f, 0), fixedTimeStep(1.0f / 60.0f) {
}

PhysicsEngine::~PhysicsEngine() = default;

void PhysicsEngine::update(float deltaTime) {
    if (!terrain) return;
    
    for (auto& car : cars) {
        Vector3 pos = car->getPosition();
        float terrainHeight = terrain->getHeight(pos.x, pos.z);
        Vector3 terrainNormal = terrain->getNormal(pos.x, pos.z);
        
        car->update(deltaTime, terrainNormal, terrainHeight);
    }
}

void PhysicsEngine::reset() {
    cars.clear();
}

Car* PhysicsEngine::createCar(const Vector3& position) {
    auto car = std::make_unique<Car>(position);
    Car* ptr = car.get();
    cars.push_back(std::move(car));
    return ptr;
}

void PhysicsEngine::setTerrain(std::unique_ptr<Terrain> newTerrain) {
    terrain = std::move(newTerrain);
}
