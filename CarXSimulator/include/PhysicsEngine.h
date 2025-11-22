#pragma once
#include "Math.h"
#include <vector>
#include <memory>

class Car;
class Terrain;

class PhysicsEngine {
public:
    PhysicsEngine();
    ~PhysicsEngine();
    
    void update(float deltaTime);
    void reset();
    
    // Object management
    Car* createCar(const Vector3& position);
    void setTerrain(std::unique_ptr<Terrain> terrain);
    Terrain* getTerrain() { return terrain.get(); }
    
    // Physics parameters
    void setGravity(const Vector3& g) { gravity = g; }
    Vector3 getGravity() const { return gravity; }
    
    const std::vector<std::unique_ptr<Car>>& getCars() const { return cars; }
    
private:
    std::vector<std::unique_ptr<Car>> cars;
    std::unique_ptr<Terrain> terrain;
    Vector3 gravity;
    float fixedTimeStep;
};
