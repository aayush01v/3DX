#include "Terrain.h"
#include <cmath>
#include <algorithm>

Terrain::Terrain(int width, int depth) 
    : width(width), depth(depth), curvature(0.0f), scale(1.0f), type(TerrainType::FLAT) {
    heights.resize(width * depth, 0.0f);
    friction.resize(width * depth, 1.0f);
}

void Terrain::generateProcedural(float amplitude, float frequency) {
    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            float fx = x * frequency / width;
            float fz = z * frequency / depth;
            
            // Multi-octave noise
            float height = 0.0f;
            float amp = amplitude;
            float freq = frequency;
            
            for (int octave = 0; octave < 4; ++octave) {
                height += amp * (
                    std::sin(fx * freq) * std::cos(fz * freq) +
                    0.5f * std::sin(fx * freq * 2.0f) * std::cos(fz * freq * 2.0f)
                );
                amp *= 0.5f;
                freq *= 2.0f;
            }
            
            heights[z * width + x] = height;
            
            // Vary friction based on height (higher = less grip)
            friction[z * width + x] = 1.0f - std::abs(height) / (amplitude * 2.0f) * 0.3f;
        }
    }
    type = TerrainType::CUSTOM;
}

void Terrain::generateByType(TerrainType terrainType) {
    type = terrainType;
    
    switch (type) {
        case TerrainType::FLAT:
            std::fill(heights.begin(), heights.end(), 0.0f);
            std::fill(friction.begin(), friction.end(), 1.0f);
            break;
            
        case TerrainType::HILLS:
            generateProcedural(3.0f, 2.0f);
            break;
            
        case TerrainType::MOUNTAINS:
            generateProcedural(15.0f, 1.5f);
            break;
            
        case TerrainType::VALLEY:
            for (int z = 0; z < depth; ++z) {
                for (int x = 0; x < width; ++x) {
                    float dx = (x - width / 2.0f) / (width / 2.0f);
                    float dz = (z - depth / 2.0f) / (depth / 2.0f);
                    float dist = std::sqrt(dx * dx + dz * dz);
                    heights[z * width + x] = -5.0f + dist * dist * 10.0f;
                    friction[z * width + x] = 1.0f;
                }
            }
            break;
            
        case TerrainType::RACETRACK:
            generateRaceTrack(30.0f, 10.0f, 0.2f);
            break;
            
        default:
            break;
    }
}

void Terrain::generateRaceTrack(float radius, float trackWidth, float banking) {
    Vector3 center(width / 2.0f, 0, depth / 2.0f);
    
    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            float dx = x - center.x;
            float dz = z - center.z;
            float distFromCenter = std::sqrt(dx * dx + dz * dz);
            
            // Distance from track centerline
            float distFromTrack = std::abs(distFromCenter - radius);
            
            if (distFromTrack < trackWidth / 2.0f) {
                // On track - apply banking
                float bankAmount = (distFromCenter - radius) / (trackWidth / 2.0f);
                heights[z * width + x] = bankAmount * banking * 5.0f;
                friction[z * width + x] = 1.2f;  // High grip on track
            } else {
                // Off track - rough terrain
                heights[z * width + x] = perlinNoise(x * 0.1f, z * 0.1f) * 2.0f;
                friction[z * width + x] = 0.6f;  // Low grip off track
            }
        }
    }
    type = TerrainType::RACETRACK;
}

float Terrain::getHeight(float x, float z) const {
    int ix = static_cast<int>((x / scale) + width / 2);
    int iz = static_cast<int>((z / scale) + depth / 2);
    
    if (ix < 0 || ix >= width - 1 || iz < 0 || iz >= depth - 1) {
        return 0.0f;
    }
    
    float fx = (x / scale) + width / 2 - ix;
    float fz = (z / scale) + depth / 2 - iz;
    
    float h00 = heights[iz * width + ix];
    float h10 = heights[iz * width + ix + 1];
    float h01 = heights[(iz + 1) * width + ix];
    float h11 = heights[(iz + 1) * width + ix + 1];
    
    float h0 = h00 * (1 - fx) + h10 * fx;
    float h1 = h01 * (1 - fx) + h11 * fx;
    
    return h0 * (1 - fz) + h1 * fz;
}

Vector3 Terrain::getNormal(float x, float z) const {
    int ix = static_cast<int>((x / scale) + width / 2);
    int iz = static_cast<int>((z / scale) + depth / 2);
    
    if (ix < 1 || ix >= width - 1 || iz < 1 || iz >= depth - 1) {
        return Vector3(0, 1, 0);
    }
    
    return calculateNormal(ix, iz);
}

Vector3 Terrain::calculateNormal(int x, int z) const {
    float hL = heights[z * width + (x - 1)];
    float hR = heights[z * width + (x + 1)];
    float hD = heights[(z - 1) * width + x];
    float hU = heights[(z + 1) * width + x];
    
    Vector3 normal(hL - hR, 2.0f * scale, hD - hU);
    return normal.normalized();
}

Vector3 Terrain::getGravityDirection(float x, float z) const {
    Vector3 normal = getNormal(x, z);
    Vector3 down(0, -1, 0);
    
    Vector3 gravityDir = down * (1.0f - curvature) + normal * -curvature;
    return gravityDir.normalized();
}

float Terrain::getFriction(float x, float z) const {
    int ix = static_cast<int>((x / scale) + width / 2);
    int iz = static_cast<int>((z / scale) + depth / 2);
    
    if (ix < 0 || ix >= width || iz < 0 || iz >= depth) {
        return 0.5f;  // Low friction out of bounds
    }
    
    return friction[iz * width + ix];
}

bool Terrain::isInBounds(float x, float z) const {
    int ix = static_cast<int>((x / scale) + width / 2);
    int iz = static_cast<int>((z / scale) + depth / 2);
    return ix >= 0 && ix < width && iz >= 0 && iz < depth;
}

float Terrain::perlinNoise(float x, float z) const {
    // Simple Perlin-like noise
    int xi = static_cast<int>(std::floor(x));
    int zi = static_cast<int>(std::floor(z));
    
    float xf = x - xi;
    float zf = z - zi;
    
    // Smooth interpolation
    float u = xf * xf * (3.0f - 2.0f * xf);
    float v = zf * zf * (3.0f - 2.0f * zf);
    
    float n00 = smoothNoise(xi, zi);
    float n10 = smoothNoise(xi + 1, zi);
    float n01 = smoothNoise(xi, zi + 1);
    float n11 = smoothNoise(xi + 1, zi + 1);
    
    float nx0 = n00 * (1 - u) + n10 * u;
    float nx1 = n01 * (1 - u) + n11 * u;
    
    return nx0 * (1 - v) + nx1 * v;
}

float Terrain::smoothNoise(int x, int z) const {
    // Pseudo-random noise based on coordinates
    int n = x + z * 57;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}


float Terrain::getFriction(float x, float z) const {
    int ix = static_cast<int>((x / scale) + width / 2);
    int iz = static_cast<int>((z / scale) + depth / 2);
    
    if (ix < 0 || ix >= width || iz < 0 || iz >= depth) {
        return 1.0f;
    }
    
    return friction[iz * width + ix];
}

bool Terrain::isInBounds(float x, float z) const {
    int ix = static_cast<int>((x / scale) + width / 2);
    int iz = static_cast<int>((z / scale) + depth / 2);
    
    return ix >= 0 && ix < width && iz >= 0 && iz < depth;
}

float Terrain::perlinNoise(float x, float z) const {
    // Simple noise implementation
    int xi = static_cast<int>(x);
    int zi = static_cast<int>(z);
    
    float fx = x - xi;
    float fz = z - zi;
    
    float n00 = smoothNoise(xi, zi);
    float n10 = smoothNoise(xi + 1, zi);
    float n01 = smoothNoise(xi, zi + 1);
    float n11 = smoothNoise(xi + 1, zi + 1);
    
    float nx0 = n00 * (1 - fx) + n10 * fx;
    float nx1 = n01 * (1 - fx) + n11 * fx;
    
    return nx0 * (1 - fz) + nx1 * fz;
}

float Terrain::smoothNoise(int x, int z) const {
    // Simple pseudo-random noise
    int n = x + z * 57;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}
