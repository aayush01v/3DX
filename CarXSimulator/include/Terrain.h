#pragma once
#include "Math.h"
#include <vector>
#include <string>

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
    
    float getHeight(float x, float z) const;
    Vector3 getNormal(float x, float z) const;
    Vector3 getGravityDirection(float x, float z) const;
    float getFriction(float x, float z) const;
    
    void setCurvature(float c) { curvature = c; }
    float getCurvature() const { return curvature; }
    void setScale(float s) { scale = s; }
    
    const std::vector<float>& getHeightData() const { return heights; }
    int getWidth() const { return width; }
    int getDepth() const { return depth; }
    
    // Collision detection
    bool isInBounds(float x, float z) const;
    
private:
    Vector3 calculateNormal(int x, int z) const;
    float perlinNoise(float x, float z) const;
    float smoothNoise(int x, int z) const;
    
    std::vector<float> heights;
    std::vector<float> friction;  // Per-vertex friction
    int width, depth;
    float curvature;  // Affects gravity direction
    float scale;
    TerrainType type;
};
