#include "Terrain.h"
#include <cmath>
#include <algorithm>
#include <chrono>

Terrain::Terrain(int width, int depth) 
    : width(width), depth(depth), curvature(0.0f), scale(1.0f), type(TerrainType::FLAT) {
    heights.resize(width * depth, 0.0f);
    friction.resize(width * depth, 1.0f);
    
    // Initialize with random seed
    auto now = std::chrono::high_resolution_clock::now();
    seed = static_cast<unsigned int>(now.time_since_epoch().count());
    rng.seed(seed);
}

// Advanced noise function for natural terrain
float Terrain::noise(float x, float z) const {
    float n = std::sin(x * 12.9898f + z * 78.233f + seed) * 43758.5453f;
    return n - std::floor(n);
}

float Terrain::smoothNoise(float x, float z) const {
    int intX = static_cast<int>(std::floor(x));
    int intZ = static_cast<int>(std::floor(z));
    float fracX = x - intX;
    float fracZ = z - intZ;
    
    // Get noise at corners
    float v1 = noise(static_cast<float>(intX), static_cast<float>(intZ));
    float v2 = noise(static_cast<float>(intX + 1), static_cast<float>(intZ));
    float v3 = noise(static_cast<float>(intX), static_cast<float>(intZ + 1));
    float v4 = noise(static_cast<float>(intX + 1), static_cast<float>(intZ + 1));
    
    // Smooth interpolation
    float i1 = interpolate(v1, v2, fracX);
    float i2 = interpolate(v3, v4, fracX);
    return interpolate(i1, i2, fracZ);
}

float Terrain::interpolate(float a, float b, float t) const {
    float ft = t * 3.14159265f;
    float f = (1.0f - std::cos(ft)) * 0.5f;
    return a * (1.0f - f) + b * f;
}

float Terrain::perlinNoise(float x, float z, int octaves, float persistence, float noiseScale) const {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;
    
    for (int i = 0; i < octaves; ++i) {
        total += smoothNoise(x * frequency / noiseScale, z * frequency / noiseScale) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    
    return total / maxValue;
}

void Terrain::generateProcedural(float amplitude, float frequency) {
    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            float worldX = (x - width / 2.0f) * scale;
            float worldZ = (z - depth / 2.0f) * scale;
            
            // Use advanced Perlin noise
            float height = perlinNoise(worldX, worldZ, 4, 0.5f, 30.0f) * amplitude;
            
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
            for (int z = 0; z < depth; ++z) {
                for (int x = 0; x < width; ++x) {
                    float worldX = (x - width / 2.0f) * scale;
                    float worldZ = (z - depth / 2.0f) * scale;
                    
                    // Gentle rolling hills with multiple frequencies
                    float baseHeight = perlinNoise(worldX, worldZ, 4, 0.5f, 30.0f) * 5.0f;
                    float detail = std::sin(worldX / 15.0f) * std::cos(worldZ / 15.0f) * 2.0f;
                    float microDetail = perlinNoise(worldX, worldZ, 2, 0.3f, 10.0f) * 1.0f;
                    
                    heights[z * width + x] = baseHeight + detail + microDetail;
                    friction[z * width + x] = 0.95f;
                }
            }
            break;
            
        case TerrainType::MOUNTAINS:
            for (int z = 0; z < depth; ++z) {
                for (int x = 0; x < width; ++x) {
                    float worldX = (x - width / 2.0f) * scale;
                    float worldZ = (z - depth / 2.0f) * scale;
                    
                    // Dramatic mountain peaks with valleys
                    float baseNoise = perlinNoise(worldX, worldZ, 6, 0.6f, 40.0f);
                    float mountains = std::pow(baseNoise, 2.0f) * 20.0f;
                    
                    // Add ridges
                    float ridgeX = std::abs(std::sin(worldX / 25.0f)) * 8.0f;
                    float ridgeZ = std::abs(std::cos(worldZ / 25.0f)) * 8.0f;
                    float ridges = std::min(ridgeX, ridgeZ);
                    
                    // Add valleys
                    float valley = std::sin(worldX / 30.0f) * std::cos(worldZ / 30.0f) * -5.0f;
                    
                    // Combine with detail
                    float detail = perlinNoise(worldX, worldZ, 3, 0.4f, 15.0f) * 3.0f;
                    
                    heights[z * width + x] = mountains + ridges + valley + detail;
                    friction[z * width + x] = 0.85f; // Less grip on steep mountains
                }
            }
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
            generateRaceTrack(40.0f, 15.0f, 0.2f);
            break;
            
        default:
            break;
    }
}

void Terrain::generateRaceTrack(float radius, float trackWidth, float banking) {
    float centerX = 0.0f;
    float centerZ = 0.0f;
    
    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            float worldX = (x - width / 2.0f) * scale;
            float worldZ = (z - depth / 2.0f) * scale;
            
            float dx = worldX - centerX;
            float dz = worldZ - centerZ;
            float dist = std::sqrt(dx * dx + dz * dz);
            float distFromTrack = std::abs(dist - radius);
            
            if (distFromTrack < trackWidth / 2.0f) {
                // On the track - add banking for turns
                float angle = std::atan2(dz - centerZ, dx - centerX);
                float bankingEffect = std::sin(angle * 4.0f) * 2.0f;
                
                // Smooth track surface with slight elevation changes
                float elevation = std::sin(angle * 2.0f) * 1.5f;
                
                // Track surface (slightly lower than surroundings)
                float trackDepth = -1.0f + (distFromTrack / (trackWidth / 2.0f)) * 1.0f;
                
                heights[z * width + x] = trackDepth + bankingEffect + elevation;
                friction[z * width + x] = 1.3f;  // High grip on track
            } else if (distFromTrack < trackWidth / 2.0f + 5.0f) {
                // Track edge/shoulder - smooth transition
                float edgeFactor = (distFromTrack - trackWidth / 2.0f) / 5.0f;
                float trackHeight = -1.0f;
                float surroundHeight = perlinNoise(worldX, worldZ, 3, 0.4f, 20.0f) * 2.0f;
                heights[z * width + x] = trackHeight * (1.0f - edgeFactor) + surroundHeight * edgeFactor;
                friction[z * width + x] = 1.0f - edgeFactor * 0.4f;
            } else {
                // Outside track - natural terrain
                heights[z * width + x] = perlinNoise(worldX, worldZ, 4, 0.5f, 25.0f) * 3.0f;
                friction[z * width + x] = 0.7f;  // Lower grip off track
            }
        }
    }
    type = TerrainType::RACETRACK;
}

void Terrain::regenerate() {
    // Generate new random seed
    auto now = std::chrono::high_resolution_clock::now();
    seed = static_cast<unsigned int>(now.time_since_epoch().count());
    rng.seed(seed);
    
    // Regenerate current terrain type
    generateByType(type);
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


// Remove duplicate definitions - already defined above
