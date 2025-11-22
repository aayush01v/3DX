#pragma once
#include "Math.h"
#include <memory>
#include <vector>

class Shader;
class Mesh;
class Camera;
class PhysicsEngine;

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    bool initialize(int width, int height);
    void shutdown();
    
    void beginFrame();
    void render(PhysicsEngine* physics);
    void endFrame();
    
    void resize(int width, int height);
    Camera* getCamera() { return camera.get(); }
    
private:
    void renderCar(const class Car* car);
    void renderTerrain(const class Terrain* terrain);
    void renderForceVectors(const Car* car);
    
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Shader> carShader;
    std::unique_ptr<Shader> terrainShader;
    std::unique_ptr<Shader> lineShader;
    
    std::unique_ptr<Mesh> carMesh;
    std::unique_ptr<Mesh> terrainMesh;
    std::unique_ptr<class Model> carModel;
    
    int width, height;
    bool initialized;
};

class Camera {
public:
    Camera();
    
    void setPosition(const Vector3& pos) { position = pos; }
    void setTarget(const Vector3& target) { this->target = target; }
    void setFOV(float fov) { this->fov = fov; }
    
    Matrix4 getViewMatrix() const;
    Matrix4 getProjectionMatrix(float aspect) const;
    
    void orbit(float deltaX, float deltaY);
    void zoom(float delta);
    
private:
    Vector3 position;
    Vector3 target;
    Vector3 up;
    float fov;
    float distance;
    float pitch, yaw;
};
