#pragma once
#include "Math.h"
#include <vector>
#include <string>
#include <random>

enum class TerrainType {
    FLAT,
    HILLS,
    MOUNTAINS,
    VALLEY,
    RACETRACK,
    CUSTOM
};

class Terrain {
public:
    Terrain(int width = 100, int depth = 100);
    
    bool loadHeightmap(const std::string& filename);
    void generateProcedural(float amplitude, float frequency);
    void generateByType(TerrainType type);
    void generateRaceTrack(float radius, float width, float banking);
    void regenerate(); // Regenerate with new random seed
    void regenerateSmooth(int smoothingIterations = 2, float blendStrength = 0.45f);
    
    float getHeight(float x, float z) const;
    Vector3 getNormal(float x, float z) const;
    Vector3 getGravityDirection(float x, float z) const;
    float getFriction(float x, float z) const;
    
    void setCurvature(float c) { curvature = c; }
    float getCurvature() const { return curvature; }
    void setScale(float s) { scale = s; }
    float getScale() const { return scale; }
    void setSeed(unsigned int s) { seed = s; }
    unsigned int getSeed() const { return seed; }
    
    const std::vector<float>& getHeightData() const { return heights; }
    int getWidth() const { return width; }
    int getDepth() const { return depth; }
    TerrainType getType() const { return type; }
    
    // Collision detection
    bool isInBounds(float x, float z) const;
    
    // Advanced terrain features
    void addObstacle(float x, float z, float radius, float height);
    void addRamp(float x, float z, float width, float length, float height);
    void smoothTerrain(int iterations = 1);
    void applyErosion(float strength = 0.1f);
    
    // Material zones
    void setMaterialZone(float x, float z, float radius, float frictionValue);
    
    // Terrain deformation
    void deform(float x, float z, float radius, float depth);
    
private:
    Vector3 calculateNormal(int x, int z) const;
    
    // Advanced noise functions
    float noise(float x, float z) const;
    float smoothNoise(float x, float z) const;
    float interpolate(float a, float b, float t) const;
    float perlinNoise(float x, float z, int octaves, float persistence, float scale) const;
    void normalizeHeightField(float targetAmplitude = 12.0f);
    void recalculateFriction(float maxGripVariance = 0.3f);

    std::vector<float> heights;
    std::vector<float> friction;  // Per-vertex friction
    std::vector<Vector3> normals; // Cached normals for performance
    int width, depth;
    float curvature;  // Affects gravity direction
    float scale;
    unsigned int seed; // Random seed for terrain generation
    TerrainType type;
    bool normalsCached;
    
    mutable std::mt19937 rng; // Random number generator
    
    void cacheNormals();
    void invalidateNormals() { normalsCached = false; }
};
