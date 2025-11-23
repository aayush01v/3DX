#pragma once
#include "Math.h"
#include <GL/glew.h>
#include <array>
#include <memory>
#include <vector>

class Shader;
class Mesh;
class Camera;
class PhysicsEngine;

struct RenderBatch {
    GLuint vao{0}, vbo{0}, ebo{0};
    size_t indexCount{0};
    GLenum primitiveType{GL_TRIANGLES};
};

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
    void renderCarOptimized(const class Car* car);
    void renderTerrainOptimized(const class Terrain* terrain);
    void renderShadowPass(PhysicsEngine* physics);
    void renderCarsInstanced(const std::vector<std::unique_ptr<class Car>>& cars);
    void renderTerrainWithLOD(const class Terrain* terrain);
    void renderSkybox();
    void drawLine(const Vector3& start, const Vector3& end, const Vector3& color);
    void updateFrustum();
    bool isInFrustum(const Vector3& position, float radius) const;
    Plane extractPlane(const Matrix4& vp, int planeIndex);
    void initShadowMap();
    void initMSAAFramebuffer();
    void setupRenderBatches();

    std::unique_ptr<Camera> camera;
    std::unique_ptr<Shader> carShader;
    std::unique_ptr<Shader> terrainShader;
    std::unique_ptr<Shader> shadowShader;
    std::unique_ptr<Shader> lineShader;

    std::unique_ptr<Mesh> carMesh;
    std::unique_ptr<Mesh> terrainMesh;
    std::unique_ptr<class Model> carModel;

    int width, height;
    bool initialized;
    GLuint msaaFBO;
    GLuint msaaColorBuffer;
    GLuint msaaDepthBuffer;
    int msaaSamples;

    GLuint shadowMapFBO;
    GLuint shadowMapTexture;
    int shadowMapSize;

    std::vector<RenderBatch> renderBatches;
    std::array<Plane, 6> frustumPlanes;
    Matrix4 lightSpaceMatrix;
    bool useInstancing;
    bool useFrustumCulling;
    GLuint instanceVBO;
    size_t instanceBufferCapacity;
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
