#pragma once
#include "Math.h"
#include <vector>
#include <memory>
#include <array>

class Car;
class Terrain;

// Physics configuration for performance tuning
struct PhysicsConfig {
    int substeps = 4;              // Physics substeps per frame for stability
    float maxDeltaTime = 0.1f;     // Maximum delta time (10 FPS minimum)
    bool useAdaptiveSubsteps = true; // Adjust substeps based on performance
    bool enableCollisionCache = true; // Cache terrain collision data
    int collisionCacheSize = 256;   // Size of collision cache
};

// Collision cache for terrain queries
struct CollisionCache {
    struct Entry {
        float x, z;
        float height;
        Vector3 normal;
        float friction;
        int frameStamp;
    };
    
    std::vector<Entry> entries;
    int currentFrame = 0;
    
    CollisionCache(int size) : entries(size) {}
    
    bool query(float x, float z, float& height, Vector3& normal, float& friction);
    void insert(float x, float z, float height, const Vector3& normal, float friction);
};

class PhysicsEngine {
public:
    PhysicsEngine();
    ~PhysicsEngine();
    
    void update(float deltaTime);
    void reset();
    
    // Object management
    Car* createCar(const Vector3& position);
    void removeCar(Car* car);
    void setTerrain(std::unique_ptr<Terrain> terrain);
    Terrain* getTerrain() { return terrain.get(); }
    void regenerateTerrain(); // Regenerate terrain with new seed
    
    // Physics parameters
    void setGravity(const Vector3& g) { gravity = g; }
    Vector3 getGravity() const { return gravity; }
    void setTimeScale(float scale) { timeScale = scale; }
    float getTimeScale() const { return timeScale; }
    
    // Configuration
    void setConfig(const PhysicsConfig& cfg) { config = cfg; }
    PhysicsConfig getConfig() const { return config; }
    
    // Advanced features
    void setAirDensity(float density) { airDensity = density; }
    float getAirDensity() const { return airDensity; }
    void setWindVelocity(const Vector3& wind) { windVelocity = wind; }
    Vector3 getWindVelocity() const { return windVelocity; }
    
    const std::vector<std::unique_ptr<Car>>& getCars() const { return cars; }
    
    // Statistics
    int getCarCount() const { return static_cast<int>(cars.size()); }
    float getAverageUpdateTime() const { return avgUpdateTime; }
    int getCurrentSubsteps() const { return currentSubsteps; }
    
    // Optimized terrain queries with caching
    float getTerrainHeight(float x, float z);
    Vector3 getTerrainNormal(float x, float z);
    float getTerrainFriction(float x, float z);
    
private:
    void updateCar(Car* car, float dt);
    void updatePhysicsSubstep(float dt);
    
    std::vector<std::unique_ptr<Car>> cars;
    std::unique_ptr<Terrain> terrain;
    std::unique_ptr<CollisionCache> collisionCache;
    
    Vector3 gravity;
    Vector3 windVelocity;
    float airDensity;
    float fixedTimeStep;
    float timeScale;
    float accumulator;
    
    PhysicsConfig config;
    int currentSubsteps;
    float avgUpdateTime;
    
    // Performance tracking
    std::array<float, 60> updateTimes;
    int updateTimeIndex;
};
