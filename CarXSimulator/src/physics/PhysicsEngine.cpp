#include "PhysicsEngine.h"
#include "Car.h"
#include "Terrain.h"
#include <algorithm>
#include <chrono>
#include <cmath>

// Collision Cache Implementation
bool CollisionCache::query(float x, float z, float& height, Vector3& normal, float& friction) {
    const float tolerance = 0.5f; // Cache hit tolerance
    
    for (const auto& entry : entries) {
        if (entry.frameStamp == currentFrame) {
            float dx = entry.x - x;
            float dz = entry.z - z;
            if (dx * dx + dz * dz < tolerance * tolerance) {
                height = entry.height;
                normal = entry.normal;
                friction = entry.friction;
                return true;
            }
        }
    }
    return false;
}

void CollisionCache::insert(float x, float z, float height, const Vector3& normal, float friction) {
    static int insertIndex = 0;
    Entry& entry = entries[insertIndex];
    entry.x = x;
    entry.z = z;
    entry.height = height;
    entry.normal = normal;
    entry.friction = friction;
    entry.frameStamp = currentFrame;
    insertIndex = (insertIndex + 1) % entries.size();
}

// PhysicsEngine Implementation
PhysicsEngine::PhysicsEngine() 
    : gravity(0, -9.81f, 0), 
      windVelocity(0, 0, 0),
      airDensity(1.225f), // Sea level air density
      fixedTimeStep(1.0f / 60.0f),
      timeScale(1.0f),
      accumulator(0.0f),
      currentSubsteps(4),
      avgUpdateTime(0.0f),
      updateTimeIndex(0) {
    
    updateTimes.fill(0.0f);
    collisionCache = std::make_unique<CollisionCache>(256);
}

PhysicsEngine::~PhysicsEngine() = default;

void PhysicsEngine::update(float deltaTime) {
    if (!terrain) return;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Apply time scale
    deltaTime *= timeScale;
    
    // Clamp delta time to prevent spiral of death
    deltaTime = std::min(deltaTime, config.maxDeltaTime);
    
    // Adaptive substeps based on frame time
    if (config.useAdaptiveSubsteps) {
        if (deltaTime > 0.025f) { // > 40 FPS
            currentSubsteps = std::min(config.substeps * 2, 8);
        } else if (deltaTime < 0.016f) { // < 60 FPS
            currentSubsteps = std::max(config.substeps / 2, 1);
        } else {
            currentSubsteps = config.substeps;
        }
    } else {
        currentSubsteps = config.substeps;
    }
    
    // Fixed timestep with accumulator for stable physics
    accumulator += deltaTime;
    const float substepDt = fixedTimeStep / currentSubsteps;
    
    int iterations = 0;
    const int maxIterations = 10; // Prevent infinite loop
    
    while (accumulator >= fixedTimeStep && iterations < maxIterations) {
        // Update collision cache frame
        if (config.enableCollisionCache) {
            collisionCache->currentFrame++;
        }
        
        // Run physics substeps
        for (int i = 0; i < currentSubsteps; ++i) {
            updatePhysicsSubstep(substepDt);
        }
        
        accumulator -= fixedTimeStep;
        iterations++;
    }
    
    // Track performance
    auto endTime = std::chrono::high_resolution_clock::now();
    float updateTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    updateTimes[updateTimeIndex] = updateTime;
    updateTimeIndex = (updateTimeIndex + 1) % updateTimes.size();
    
    // Calculate average
    float sum = 0.0f;
    for (float t : updateTimes) sum += t;
    avgUpdateTime = sum / updateTimes.size();
}

void PhysicsEngine::updatePhysicsSubstep(float dt) {
    // Update all cars with optimized terrain queries
    for (auto& car : cars) {
        updateCar(car.get(), dt);
    }
}

void PhysicsEngine::updateCar(Car* car, float dt) {
    Vector3 pos = car->getPosition();
    
    // Multi-point terrain sampling for better collision (4 wheels)
    const float wheelBase = 2.6f;
    const float wheelTrack = 1.8f;
    float rotation = car->getRotation();
    float cosR = std::cos(rotation);
    float sinR = std::sin(rotation);
    
    // Calculate wheel positions - use actual wheel positions from car
    Vector3 wheelPositions[4];
    for (int i = 0; i < 4; ++i) {
        wheelPositions[i] = car->getWheelPosition(i);
    }
    
    // Sample terrain at each wheel with caching
    float wheelHeights[4];
    float avgHeight = 0.0f;
    
    for (int i = 0; i < 4; ++i) {
        wheelHeights[i] = getTerrainHeight(wheelPositions[i].x, wheelPositions[i].z);
        avgHeight += wheelHeights[i];
    }
    avgHeight /= 4.0f;
    
    // Get terrain properties at car center
    Vector3 terrainNormal = getTerrainNormal(pos.x, pos.z);
    float terrainFriction = getTerrainFriction(pos.x, pos.z);
    
    // Apply wind resistance (aerodynamic drag)
    Vector3 relativeVelocity = car->getVelocity() - windVelocity;
    float speed = relativeVelocity.length();
    if (speed > 0.1f) {
        Vector3 dragDirection = relativeVelocity.normalized();
        float dragMagnitude = 0.5f * airDensity * car->getDragCoefficient() * 
                             car->getFrontalArea() * speed * speed;
        Vector3 dragForce = dragDirection * -dragMagnitude;
        car->applyForce(dragForce);
    }
    
    // Update car physics with individual wheel heights
    car->updateWithWheelHeights(dt, terrainNormal, wheelHeights, terrainFriction);
}

float PhysicsEngine::getTerrainHeight(float x, float z) {
    if (!terrain) return 0.0f;
    
    if (config.enableCollisionCache) {
        float height;
        Vector3 normal;
        float friction;
        if (collisionCache->query(x, z, height, normal, friction)) {
            return height;
        }
        
        height = terrain->getHeight(x, z);
        normal = terrain->getNormal(x, z);
        friction = terrain->getFriction(x, z);
        collisionCache->insert(x, z, height, normal, friction);
        return height;
    }
    
    return terrain->getHeight(x, z);
}

Vector3 PhysicsEngine::getTerrainNormal(float x, float z) {
    if (!terrain) return Vector3(0, 1, 0);
    
    if (config.enableCollisionCache) {
        float height;
        Vector3 normal;
        float friction;
        if (collisionCache->query(x, z, height, normal, friction)) {
            return normal;
        }
    }
    
    return terrain->getNormal(x, z);
}

float PhysicsEngine::getTerrainFriction(float x, float z) {
    if (!terrain) return 1.0f;
    
    if (config.enableCollisionCache) {
        float height;
        Vector3 normal;
        float friction;
        if (collisionCache->query(x, z, height, normal, friction)) {
            return friction;
        }
    }
    
    return terrain->getFriction(x, z);
}

void PhysicsEngine::reset() {
    cars.clear();
    accumulator = 0.0f;
    if (collisionCache) {
        collisionCache->currentFrame = 0;
    }
}

Car* PhysicsEngine::createCar(const Vector3& position) {
    auto car = std::make_unique<Car>(position);
    Car* ptr = car.get();
    cars.push_back(std::move(car));
    return ptr;
}

void PhysicsEngine::removeCar(Car* car) {
    cars.erase(
        std::remove_if(cars.begin(), cars.end(),
            [car](const std::unique_ptr<Car>& c) { return c.get() == car; }),
        cars.end()
    );
}

void PhysicsEngine::setTerrain(std::unique_ptr<Terrain> newTerrain) {
    terrain = std::move(newTerrain);
    // Clear collision cache when terrain changes
    if (collisionCache) {
        collisionCache->currentFrame = 0;
    }
}

void PhysicsEngine::regenerateTerrain() {
    if (terrain) {
        terrain->regenerate();
        // Clear collision cache
        if (collisionCache) {
            collisionCache->currentFrame = 0;
        }
    }
}
