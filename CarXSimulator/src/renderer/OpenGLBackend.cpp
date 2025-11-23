#include "Renderer.h"
#include "PhysicsEngine.h"
#include "Car.h"
#include "Terrain.h"
#include <GL/glew.h>
#include <algorithm>
#include <cmath>

#ifndef USE_VULKAN

// OpenGL implementation with advanced optimizations
Renderer::Renderer() : width(0), height(0), initialized(false),
                       msaaFBO(0), msaaColorBuffer(0), msaaDepthBuffer(0),
                       msaaSamples(4), shadowMapSize(2048), shadowMapFBO(0),
                       shadowMapTexture(0), useInstancing(true),
                       useFrustumCulling(true), instanceVBO(0),
                       instanceBufferCapacity(0) {}

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::initialize(int w, int h) {
    width = w;
    height = h;
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        return false;
    }
    
    // Enable modern OpenGL features
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Enable seamless cubemap sampling
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    // Enable MSAA if supported
    if (msaaSamples > 0) {
        glEnable(GL_MULTISAMPLE);
    }
    
    // Initialize camera
    camera = std::make_unique<Camera>();
    camera->setPosition(Vector3(0, 10, 20));
    camera->setTarget(Vector3(0, 0, 0));
    
    // Load optimized shaders with modern GLSL
    carShader = std::make_unique<Shader>("shaders/car.vert", "shaders/car.frag");
    terrainShader = std::make_unique<Shader>("shaders/terrain.vert", "shaders/terrain.frag");
    shadowShader = std::make_unique<Shader>("shaders/shadow.vert", "shaders/shadow.frag");
    lineShader = std::make_unique<Shader>("shaders/line.vert", "shaders/line.frag");
    
    // Initialize shadow mapping
    initShadowMap();
    
    // Initialize MSAA framebuffer
    initMSAAFramebuffer();
    
    // Setup vertex array objects for batching
    setupRenderBatches();
    
    initialized = true;
    return true;
}

void Renderer::shutdown() {
    // Cleanup shadow map
    if (shadowMapFBO) {
        glDeleteFramebuffers(1, &shadowMapFBO);
        glDeleteTextures(1, &shadowMapTexture);
    }

    if (instanceVBO) {
        glDeleteBuffers(1, &instanceVBO);
        instanceVBO = 0;
        instanceBufferCapacity = 0;
    }
    
    // Cleanup MSAA framebuffer
    if (msaaFBO) {
        glDeleteFramebuffers(1, &msaaFBO);
        glDeleteTextures(1, &msaaColorBuffer);
        glDeleteRenderbuffers(1, &msaaDepthBuffer);
    }
    
    // Cleanup render batches
    for (auto& batch : renderBatches) {
        glDeleteVertexArrays(1, &batch.vao);
        glDeleteBuffers(1, &batch.vbo);
        glDeleteBuffers(1, &batch.ebo);
    }
    
    initialized = false;
}

void Renderer::beginFrame() {
    // Bind MSAA framebuffer for rendering
    if (msaaFBO) {
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
    
    // Update frustum for culling
    if (useFrustumCulling) {
        updateFrustum();
    }
}

void Renderer::render(PhysicsEngine* physics) {
    if (!initialized || !physics) return;
    
    // Shadow pass - render to shadow map
    renderShadowPass(physics);
    
    // Main render pass
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Render terrain with optimized batching
    if (physics->getTerrain()) {
        renderTerrainOptimized(physics->getTerrain());
    }
    
    // Render cars with instancing if multiple cars
    const auto& cars = physics->getCars();
    if (useInstancing && cars.size() > 1) {
        renderCarsInstanced(cars);
    } else {
        for (const auto& car : cars) {
            renderCarOptimized(car.get());
        }
    }
    
    // Render force vectors (no culling for debug visuals)
    for (const auto& car : cars) {
        renderForceVectors(car.get());
    }
    
    // Render skybox last for optimization
    renderSkybox();
}

void Renderer::endFrame() {
    // Resolve MSAA to default framebuffer
    if (msaaFBO) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                         GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Swap buffers (platform-specific)
}

void Renderer::renderCarOptimized(const Car* car) {
    if (!car || !carShader || !carMesh) return;
    
    // Frustum culling
    if (useFrustumCulling && !isInFrustum(car->getPosition(), 5.0f)) {
        return;
    }
    
    carShader->use();
    
    // Set matrices
    Matrix4 view = camera->getViewMatrix();
    Matrix4 proj = camera->getProjectionMatrix((float)width / height);
    Matrix4 model = car->getModelMatrix();
    
    carShader->setMat4("view", view.data());
    carShader->setMat4("projection", proj.data());
    carShader->setMat4("model", model.data());
    
    // Set lighting uniforms
    Vector3 lightDir = Vector3(-0.3f, -1.0f, -0.5f).normalized();
    carShader->setVec3("lightDir", lightDir.x, lightDir.y, lightDir.z);
    carShader->setVec3("viewPos", camera->getPosition().x, 
                                  camera->getPosition().y, 
                                  camera->getPosition().z);
    
    // Bind shadow map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
    carShader->setInt("shadowMap", 1);
    carShader->setMat4("lightSpaceMatrix", lightSpaceMatrix.data());
    
    // Draw with VAO (optimized)
    carMesh->draw();
}

void Renderer::renderTerrainOptimized(const Terrain* terrain) {
    if (!terrain || !terrainShader) return;

    // Skip terrain if it is completely outside the view frustum
    if (useFrustumCulling) {
        float halfWidth = terrain->getWidth() * terrain->getScale() * 0.5f;
        float halfDepth = terrain->getDepth() * terrain->getScale() * 0.5f;
        float radius = std::sqrt(halfWidth * halfWidth + halfDepth * halfDepth);
        if (!isInFrustum(Vector3(0, 0, 0), radius)) {
            return;
        }
    }

    terrainShader->use();
    
    // Set matrices
    Matrix4 view = camera->getViewMatrix();
    Matrix4 proj = camera->getProjectionMatrix((float)width / height);
    Matrix4 model = Matrix4::identity();
    
    terrainShader->setMat4("view", view.data());
    terrainShader->setMat4("projection", proj.data());
    terrainShader->setMat4("model", model.data());
    
    // Set terrain-specific uniforms
    terrainShader->setFloat("gridSize", terrain->getGridSize());
    terrainShader->setVec3("terrainColor", 0.4f, 0.7f, 0.3f);
    
    // Bind shadow map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
    terrainShader->setInt("shadowMap", 1);
    
    // Use LOD based on distance
    renderTerrainWithLOD(terrain);
}

void Renderer::renderForceVectors(const Car* car) {
    if (!car || !lineShader) return;
    
    lineShader->use();
    
    Matrix4 view = camera->getViewMatrix();
    Matrix4 proj = camera->getProjectionMatrix((float)width / height);
    
    lineShader->setMat4("view", view.data());
    lineShader->setMat4("projection", proj.data());
    
    Vector3 pos = car->getPosition();
    
    // Draw centripetal force (red)
    Vector3 centripetal = car->getCentripetalForce();
    drawLine(pos, pos + centripetal * 0.01f, Vector3(1, 0, 0));
    
    // Draw velocity (blue)
    Vector3 velocity = car->getVelocity();
    drawLine(pos, pos + velocity * 0.5f, Vector3(0, 0, 1));
    
    // Draw normal force (green)
    Vector3 normal = car->getNormalForce();
    drawLine(pos, pos + normal * 0.01f, Vector3(0, 1, 0));
}

void Renderer::resize(int w, int h) {
    width = w;
    height = h;
    // glViewport(0, 0, width, height);
}

// Camera implementation
Camera::Camera() 
    : position(0, 10, 20), target(0, 0, 0), up(0, 1, 0),
      fov(45.0f), distance(20.0f), pitch(0.3f), yaw(0.0f) {}

Matrix4 Camera::getViewMatrix() const {
    return Matrix4::lookAt(position, target, up);
}

Matrix4 Camera::getProjectionMatrix(float aspect) const {
    return Matrix4::perspective(fov * 3.14159f / 180.0f, aspect, 0.1f, 1000.0f);
}

void Camera::orbit(float deltaX, float deltaY) {
    yaw += deltaX;
    pitch += deltaY;
    pitch = std::max(-1.5f, std::min(1.5f, pitch));
    
    position.x = target.x + distance * std::cos(pitch) * std::sin(yaw);
    position.y = target.y + distance * std::sin(pitch);
    position.z = target.z + distance * std::cos(pitch) * std::cos(yaw);
}

void Camera::zoom(float delta) {
    distance = std::max(5.0f, std::min(100.0f, distance + delta));
    orbit(0, 0);
}

#endif

// Advanced OpenGL optimization implementations

void Renderer::initShadowMap() {
    // Create shadow map framebuffer
    glGenFramebuffers(1, &shadowMapFBO);
    
    // Create shadow map texture
    glGenTextures(1, &shadowMapTexture);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 
                 shadowMapSize, shadowMapSize, 0, 
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    // Set texture parameters for PCF
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // Enable comparison mode for shadow mapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    
    // Attach to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                          GL_TEXTURE_2D, shadowMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        // Shadow mapping not supported
        glDeleteFramebuffers(1, &shadowMapFBO);
        shadowMapFBO = 0;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::initMSAAFramebuffer() {
    if (msaaSamples == 0) return;
    
    // Create MSAA framebuffer
    glGenFramebuffers(1, &msaaFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    
    // Create multisampled color buffer
    glGenTextures(1, &msaaColorBuffer);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaColorBuffer);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaaSamples, 
                           GL_RGBA8, width, height, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                          GL_TEXTURE_2D_MULTISAMPLE, msaaColorBuffer, 0);
    
    // Create multisampled depth buffer
    glGenRenderbuffers(1, &msaaDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthBuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaaSamples, 
                                    GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
                             GL_RENDERBUFFER, msaaDepthBuffer);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        // MSAA not supported, cleanup
        glDeleteFramebuffers(1, &msaaFBO);
        glDeleteTextures(1, &msaaColorBuffer);
        glDeleteRenderbuffers(1, &msaaDepthBuffer);
        msaaFBO = 0;
        msaaSamples = 0;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::setupRenderBatches() {
    // Setup line rendering batch (for force vectors)
    RenderBatch lineBatch;
    glGenVertexArrays(1, &lineBatch.vao);
    glGenBuffers(1, &lineBatch.vbo);
    
    glBindVertexArray(lineBatch.vao);
    glBindBuffer(GL_ARRAY_BUFFER, lineBatch.vbo);
    
    // Reserve space for dynamic line data
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 1000, nullptr, GL_DYNAMIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 
                         (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    lineBatch.primitiveType = GL_LINES;
    renderBatches.push_back(lineBatch);
    
    glBindVertexArray(0);
}

void Renderer::renderShadowPass(PhysicsEngine* physics) {
    if (!shadowMapFBO || !shadowShader) return;
    
    // Bind shadow framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glViewport(0, 0, shadowMapSize, shadowMapSize);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Enable front face culling for shadow acne reduction
    glCullFace(GL_FRONT);
    
    shadowShader->use();
    
    // Calculate light space matrix
    Vector3 lightPos = Vector3(-50, 100, -50);
    Vector3 lightTarget = Vector3(0, 0, 0);
    Matrix4 lightView = Matrix4::lookAt(lightPos, lightTarget, Vector3(0, 1, 0));
    Matrix4 lightProj = Matrix4::ortho(-100, 100, -100, 100, 1.0f, 200.0f);
    lightSpaceMatrix = lightProj * lightView;
    
    shadowShader->setMat4("lightSpaceMatrix", lightSpaceMatrix.data());
    
    // Render cars to shadow map
    for (const auto& car : physics->getCars()) {
        Matrix4 model = car->getModelMatrix();
        shadowShader->setMat4("model", model.data());
        if (carMesh) carMesh->draw();
    }
    
    // Restore back face culling
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::renderCarsInstanced(const std::vector<std::unique_ptr<Car>>& cars) {
    if (cars.empty() || !carShader || !carMesh) return;
    
    carShader->use();
    
    // Set common uniforms
    Matrix4 view = camera->getViewMatrix();
    Matrix4 proj = camera->getProjectionMatrix((float)width / height);
    carShader->setMat4("view", view.data());
    carShader->setMat4("projection", proj.data());
    
    // Prepare instance data
    std::vector<Matrix4> modelMatrices;
    modelMatrices.reserve(cars.size());
    
    for (const auto& car : cars) {
        if (!useFrustumCulling || isInFrustum(car->getPosition(), 5.0f)) {
            modelMatrices.push_back(car->getModelMatrix());
        }
    }
    
    if (modelMatrices.empty()) return;
    
    // Upload instance data to GPU
    size_t requiredBytes = modelMatrices.size() * sizeof(Matrix4);
    if (!instanceVBO) {
        glGenBuffers(1, &instanceVBO);
        instanceBufferCapacity = 0;
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

    // Grow buffer only when needed to avoid per-frame allocations
    if (requiredBytes > instanceBufferCapacity) {
        glBufferData(GL_ARRAY_BUFFER, requiredBytes, modelMatrices.data(), GL_DYNAMIC_DRAW);
        instanceBufferCapacity = requiredBytes;
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, requiredBytes, modelMatrices.data());
    }
    
    // Setup instanced attributes
    GLuint vao = carMesh->getVAO();
    glBindVertexArray(vao);
    
    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4),
                             (void*)(sizeof(Vector4) * i));
        glVertexAttribDivisor(3 + i, 1);
    }
    
    // Draw instanced
    carMesh->drawInstanced(modelMatrices.size());
    
    // Cleanup
    glBindVertexArray(0);
}

void Renderer::renderTerrainWithLOD(const Terrain* terrain) {
    if (!terrain) return;

    Vector3 camPos = camera->getPosition();
    float distance = (camPos - Vector3(0, 0, 0)).length();
    
    // Select LOD based on distance
    int lodLevel = 0;
    if (distance > 50.0f) lodLevel = 2;
    else if (distance > 25.0f) lodLevel = 1;
    
    // Render terrain with appropriate detail level
    // This would use pre-generated terrain meshes at different LODs
    if (terrainMesh) {
        terrainShader->setInt("lodLevel", lodLevel);
        terrainShader->setFloat("lodDistance", distance);
        terrainMesh->draw();
    }
}

void Renderer::renderSkybox() {
    // Render skybox last with depth test set to LEQUAL
    // This allows early-z rejection for better performance
    glDepthFunc(GL_LEQUAL);
    
    // Skybox rendering code here
    
    glDepthFunc(GL_LESS);
}

void Renderer::drawLine(const Vector3& start, const Vector3& end, const Vector3& color) {
    if (renderBatches.empty()) return;
    
    float vertices[] = {
        start.x, start.y, start.z, color.x, color.y, color.z,
        end.x, end.y, end.z, color.x, color.y, color.z
    };
    
    RenderBatch& lineBatch = renderBatches[0];
    glBindVertexArray(lineBatch.vao);
    glBindBuffer(GL_ARRAY_BUFFER, lineBatch.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}

void Renderer::updateFrustum() {
    Matrix4 vp = camera->getProjectionMatrix((float)width / height) * 
                 camera->getViewMatrix();
    
    // Extract frustum planes from view-projection matrix
    // Left, Right, Bottom, Top, Near, Far planes
    for (int i = 0; i < 6; i++) {
        frustumPlanes[i] = extractPlane(vp, i);
    }
}

bool Renderer::isInFrustum(const Vector3& position, float radius) const {
    // Check if sphere is inside frustum
    for (int i = 0; i < 6; i++) {
        float distance = frustumPlanes[i].distance(position);
        if (distance < -radius) {
            return false;
        }
    }
    return true;
}

Plane Renderer::extractPlane(const Matrix4& vp, int planeIndex) {
    // Extract plane from view-projection matrix
    Plane plane;
    switch (planeIndex) {
        case 0: // Left
            plane.normal = Vector3(vp.m[3] + vp.m[0], vp.m[7] + vp.m[4], vp.m[11] + vp.m[8]);
            plane.d = vp.m[15] + vp.m[12];
            break;
        case 1: // Right
            plane.normal = Vector3(vp.m[3] - vp.m[0], vp.m[7] - vp.m[4], vp.m[11] - vp.m[8]);
            plane.d = vp.m[15] - vp.m[12];
            break;
        case 2: // Bottom
            plane.normal = Vector3(vp.m[3] + vp.m[1], vp.m[7] + vp.m[5], vp.m[11] + vp.m[9]);
            plane.d = vp.m[15] + vp.m[13];
            break;
        case 3: // Top
            plane.normal = Vector3(vp.m[3] - vp.m[1], vp.m[7] - vp.m[5], vp.m[11] - vp.m[9]);
            plane.d = vp.m[15] - vp.m[13];
            break;
        case 4: // Near
            plane.normal = Vector3(vp.m[3] + vp.m[2], vp.m[7] + vp.m[6], vp.m[11] + vp.m[10]);
            plane.d = vp.m[15] + vp.m[14];
            break;
        case 5: // Far
            plane.normal = Vector3(vp.m[3] - vp.m[2], vp.m[7] - vp.m[6], vp.m[11] - vp.m[10]);
            plane.d = vp.m[15] - vp.m[14];
            break;
        default:
            plane = Plane(Vector3(0, 1, 0), 0);
            break;
    }

    float length = plane.normal.length();
    if (length > 0.0f) {
        plane.normal = plane.normal / length;
        plane.d /= length;
    }

    return plane;
}

#endif
