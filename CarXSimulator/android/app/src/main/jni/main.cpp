/**
 * CarX Simulator - Native C++ Entry Point for AIDE+ Pro
 * 
 * This file provides JNI bindings for the physics engine
 * to be called from Java/Kotlin Android code.
 */

#include <jni.h>
#include <android/log.h>
#include <string>
#include <cmath>
#include <vector>

#define LOG_TAG "CarXSimulator"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ============================================================================
// Math Classes
// ============================================================================

struct Vec3 {
    float x, y, z;
    
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3 normalized() const {
        float len = length();
        return len > 0 ? Vec3(x/len, y/len, z/len) : Vec3();
    }
};

// ============================================================================
// Terrain Class
// ============================================================================

class Terrain {
public:
    enum Type { FLAT, HILLS, MOUNTAINS, TRACK };
    
private:
    Type type = HILLS;
    unsigned int seed = 42;
    std::vector<int> perm;
    
    void generatePermutation() {
        perm.resize(512);
        std::vector<int> base(256);
        for (int i = 0; i < 256; i++) base[i] = i;
        
        // Deterministic shuffle
        unsigned int s = seed;
        for (int i = 255; i > 0; i--) {
            s = (s * 1103515245 + 12345) & 0x7fffffff;
            int j = s % (i + 1);
            std::swap(base[i], base[j]);
        }
        
        for (int i = 0; i < 512; i++) {
            perm[i] = base[i % 256];
        }
    }
    
    float fade(float t) const {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }
    
    float lerp(float a, float b, float t) const {
        return a + t * (b - a);
    }
    
    float grad(int hash, float x, float z) const {
        int h = hash & 7;
        float u = h < 4 ? x : z;
        float v = h < 4 ? z : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
    }
    
    float noise(float x, float z) const {
        int floorX = (int)std::floor(x);
        int floorZ = (int)std::floor(z);
        float xf = x - floorX;
        float zf = z - floorZ;
        
        int X = ((floorX % 256) + 256) & 255;
        int Z = ((floorZ % 256) + 256) & 255;
        
        float u = fade(xf);
        float v = fade(zf);
        
        int X1 = (X + 1) & 255;
        int Z1 = (Z + 1) & 255;
        
        int aa = perm[perm[X] + Z];
        int ab = perm[perm[X] + Z1];
        int ba = perm[perm[X1] + Z];
        int bb = perm[perm[X1] + Z1];
        
        float x1 = lerp(grad(aa, xf, zf), grad(ba, xf - 1, zf), u);
        float x2 = lerp(grad(ab, xf, zf - 1), grad(bb, xf - 1, zf - 1), u);
        
        return (lerp(x1, x2, v) + 1) * 0.5f;
    }
    
    float fbm(float x, float z, int octaves, float persistence, float scale) const {
        float total = 0;
        float amplitude = 1;
        float maxValue = 0;
        float freq = 1;
        
        for (int i = 0; i < octaves; i++) {
            total += noise(x * freq / scale, z * freq / scale) * amplitude;
            maxValue += amplitude;
            amplitude *= persistence;
            freq *= 2;
        }
        
        return total / maxValue;
    }
    
public:
    Terrain() {
        generatePermutation();
    }
    
    void setType(Type t) { type = t; }
    Type getType() const { return type; }
    
    float getHeight(float worldX, float worldZ) const {
        float height = 0;
        
        switch (type) {
            case FLAT:
                height = 0;
                break;
            case HILLS:
                height = fbm(worldX, worldZ, 4, 0.5f, 50) * 8 +
                         fbm(worldX, worldZ, 2, 0.3f, 20) * 2;
                break;
            case MOUNTAINS:
                height = std::pow(fbm(worldX, worldZ, 5, 0.55f, 60), 1.5f) * 30 +
                         fbm(worldX, worldZ, 3, 0.4f, 25) * 5;
                break;
            case TRACK: {
                float dist = std::sqrt(worldX * worldX + worldZ * worldZ);
                float trackRadius = 50;
                float trackWidth = 20;
                float distFromTrack = std::abs(dist - trackRadius);
                
                if (distFromTrack < trackWidth / 2) {
                    float angle = std::atan2(worldZ, worldX);
                    height = std::sin(angle * 4) * 1.5f - 0.5f;
                } else {
                    height = fbm(worldX, worldZ, 3, 0.4f, 30) * 3;
                }
                break;
            }
        }
        
        // Clamp to reasonable range
        return std::max(-100.0f, std::min(100.0f, height));
    }
    
    Vec3 getNormal(float x, float z) const {
        float h = 0.5f;
        float hL = getHeight(x - h, z);
        float hR = getHeight(x + h, z);
        float hD = getHeight(x, z - h);
        float hU = getHeight(x, z + h);
        
        Vec3 normal(hL - hR, 2 * h, hD - hU);
        return normal.normalized();
    }
};

// ============================================================================
// Car Class
// ============================================================================

class Car {
public:
    Vec3 position;
    Vec3 velocity;
    float rotation = 0;
    float verticalVelocity = 0;
    
    float mass = 1200;
    float enginePower = 150000;
    float wheelRadius = 0.35f;
    float groundClearance = 0.5f;
    float gravity = -9.81f;
    
    float throttle = 0;
    float steering = 0;
    float brake = 0;
    
    int gear = 0;
    float engineRPM = 800;
    bool onGround = true;
    
    Car() : position(0, 5, 0) {}
    
    void update(float dt, const Terrain& terrain) {
        // Get terrain height at car position
        float terrainHeight = terrain.getHeight(position.x, position.z);
        float targetY = terrainHeight + wheelRadius + groundClearance;
        
        // Forward direction
        Vec3 forward(-std::sin(rotation), 0, -std::cos(rotation));
        float speed = velocity.length();
        
        if (onGround) {
            // Engine force
            if (gear != 0 && throttle > 0) {
                float gearRatio = std::abs(getGearRatio());
                float direction = gear > 0 ? 1.0f : -1.0f;
                float force = direction * throttle * enginePower * gearRatio / 3.5f / mass;
                velocity = velocity + forward * (force * dt);
            }
            
            // Braking
            if (brake > 0 && speed > 0.1f) {
                Vec3 brakeForce = velocity.normalized() * (-brake * 25);
                velocity = velocity + brakeForce * dt;
            }
            
            // Steering
            if (std::abs(steering) > 0.01f && speed > 0.5f) {
                float turnRate = steering * 0.8f * std::min(1.0f, 10.0f / speed);
                rotation += turnRate * dt;
            }
            
            // Drag
            velocity = velocity * (1.0f - 0.01f * dt);
            
            // Follow terrain
            float heightDiff = targetY - position.y;
            verticalVelocity += heightDiff * 50 * dt;
            verticalVelocity *= 0.8f;
            position.y += verticalVelocity * dt;
            
            if (position.y < targetY - 0.05f) {
                position.y = targetY;
                verticalVelocity = 0;
            }
        } else {
            // Airborne - apply gravity
            verticalVelocity += gravity * dt;
            position.y += verticalVelocity * dt;
            
            // Check landing
            if (position.y <= targetY) {
                position.y = targetY;
                verticalVelocity = 0;
                onGround = true;
            }
        }
        
        // Update position
        position.x += velocity.x * dt;
        position.z += velocity.z * dt;
        
        // Check if airborne
        if (position.y > targetY + 0.2f) {
            onGround = false;
        }
        
        // Update RPM
        float wheelRPM = speed / (2 * 3.14159f * wheelRadius) * 60;
        float gearRatio = std::abs(getGearRatio());
        if (gear == 0 || gearRatio == 0) {
            engineRPM = 800 + throttle * 3000;
        } else {
            engineRPM = std::max(800.0f, std::min(7500.0f, wheelRPM * gearRatio * 3.5f));
        }
    }
    
    float getGearRatio() const {
        switch (gear) {
            case -2: return -4.0f;
            case -1: return -3.2f;
            case 0: return 0;
            case 1: return 3.5f;
            case 2: return 2.5f;
            case 3: return 1.8f;
            case 4: return 1.4f;
            case 5: return 1.1f;
            case 6: return 0.9f;
            case 7: return 0.75f;
            default: return 0;
        }
    }
    
    void shiftUp() {
        if (gear < 7) gear++;
    }
    
    void shiftDown() {
        if (gear > -2) gear--;
    }
    
    float getSpeed() const { return velocity.length(); }
};

// ============================================================================
// Global State
// ============================================================================

static Car* g_car = nullptr;
static Terrain* g_terrain = nullptr;

// ============================================================================
// JNI Functions
// ============================================================================

extern "C" {

JNIEXPORT void JNICALL
Java_com_carx_simulator_NativeLib_init(JNIEnv* env, jobject thiz) {
    LOGI("CarX Native Engine initializing...");
    
    if (g_car) delete g_car;
    if (g_terrain) delete g_terrain;
    
    g_terrain = new Terrain();
    g_car = new Car();
    
    // Position car on terrain
    float terrainHeight = g_terrain->getHeight(0, 0);
    g_car->position.y = terrainHeight + g_car->wheelRadius + g_car->groundClearance;
    
    LOGI("CarX initialized. Terrain height at origin: %.2f", terrainHeight);
}

JNIEXPORT void JNICALL
Java_com_carx_simulator_NativeLib_update(JNIEnv* env, jobject thiz, jfloat deltaTime) {
    if (g_car && g_terrain) {
        g_car->update(deltaTime, *g_terrain);
    }
}

JNIEXPORT void JNICALL
Java_com_carx_simulator_NativeLib_setThrottle(JNIEnv* env, jobject thiz, jfloat value) {
    if (g_car) g_car->throttle = std::max(0.0f, std::min(1.0f, value));
}

JNIEXPORT void JNICALL
Java_com_carx_simulator_NativeLib_setSteering(JNIEnv* env, jobject thiz, jfloat value) {
    if (g_car) g_car->steering = std::max(-1.0f, std::min(1.0f, value));
}

JNIEXPORT void JNICALL
Java_com_carx_simulator_NativeLib_setBrake(JNIEnv* env, jobject thiz, jfloat value) {
    if (g_car) g_car->brake = std::max(0.0f, std::min(1.0f, value));
}

JNIEXPORT void JNICALL
Java_com_carx_simulator_NativeLib_shiftUp(JNIEnv* env, jobject thiz) {
    if (g_car) g_car->shiftUp();
}

JNIEXPORT void JNICALL
Java_com_carx_simulator_NativeLib_shiftDown(JNIEnv* env, jobject thiz) {
    if (g_car) g_car->shiftDown();
}

JNIEXPORT void JNICALL
Java_com_carx_simulator_NativeLib_setTerrain(JNIEnv* env, jobject thiz, jint type) {
    if (g_terrain) {
        g_terrain->setType(static_cast<Terrain::Type>(type));
        if (g_car) {
            float h = g_terrain->getHeight(g_car->position.x, g_car->position.z);
            g_car->position.y = h + g_car->wheelRadius + g_car->groundClearance;
        }
    }
}

JNIEXPORT void JNICALL
Java_com_carx_simulator_NativeLib_reset(JNIEnv* env, jobject thiz) {
    if (g_car && g_terrain) {
        g_car->position = Vec3(0, 0, 0);
        g_car->velocity = Vec3(0, 0, 0);
        g_car->rotation = 0;
        g_car->verticalVelocity = 0;
        g_car->gear = 0;
        g_car->throttle = 0;
        g_car->steering = 0;
        g_car->brake = 0;
        
        float h = g_terrain->getHeight(0, 0);
        g_car->position.y = h + g_car->wheelRadius + g_car->groundClearance;
    }
}

JNIEXPORT jfloatArray JNICALL
Java_com_carx_simulator_NativeLib_getCarState(JNIEnv* env, jobject thiz) {
    jfloatArray result = env->NewFloatArray(10);
    
    if (g_car) {
        float state[10] = {
            g_car->position.x,
            g_car->position.y,
            g_car->position.z,
            g_car->rotation,
            g_car->getSpeed(),
            g_car->engineRPM,
            (float)g_car->gear,
            g_car->onGround ? 1.0f : 0.0f,
            g_car->throttle,
            g_car->steering
        };
        env->SetFloatArrayRegion(result, 0, 10, state);
    }
    
    return result;
}

JNIEXPORT jfloat JNICALL
Java_com_carx_simulator_NativeLib_getTerrainHeight(JNIEnv* env, jobject thiz, jfloat x, jfloat z) {
    if (g_terrain) {
        return g_terrain->getHeight(x, z);
    }
    return 0;
}

JNIEXPORT void JNICALL
Java_com_carx_simulator_NativeLib_cleanup(JNIEnv* env, jobject thiz) {
    LOGI("CarX Native Engine cleanup");
    if (g_car) { delete g_car; g_car = nullptr; }
    if (g_terrain) { delete g_terrain; g_terrain = nullptr; }
}

} // extern "C"
