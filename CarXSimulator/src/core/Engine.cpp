#include "Engine.h"
#include "PhysicsEngine.h"
#include "Renderer.h"
#include "CodeExecutor.h"
#include <chrono>
#include <algorithm>
#include <iostream>

Engine::Engine() : running(false), timeAccumulator(0.0f), frameCount(0), fps(0.0f) {}

Engine::~Engine() {
    shutdown();
}

bool Engine::initialize(int width, int height) {
    physics = std::make_unique<PhysicsEngine>();
    renderer = std::make_unique<Renderer>();
    
    if (!renderer->initialize(width, height)) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }
    
    codeExecutor = std::make_unique<CodeExecutor>(physics.get());
    codeExecutor->registerAPI();
    
    running = true;
    std::cout << "✓ Engine initialized successfully" << std::endl;
    return true;
}

void Engine::run() {
    const float fixedDeltaTime = 1.0f / 60.0f;
    const float maxDeltaTime = 0.25f; // Cap at 4 FPS minimum to prevent spiral of death
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    auto fpsTimer = lastTime;
    
    while (running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Clamp delta time to prevent large jumps
        deltaTime = std::min(deltaTime, maxDeltaTime);
        
        timeAccumulator += deltaTime;
        
        // Fixed timestep physics updates
        int physicsSteps = 0;
        while (timeAccumulator >= fixedDeltaTime && physicsSteps < 5) {
            update(fixedDeltaTime);
            timeAccumulator -= fixedDeltaTime;
            physicsSteps++;
        }
        
        // If we're too far behind, reset accumulator
        if (timeAccumulator > fixedDeltaTime * 5) {
            timeAccumulator = 0.0f;
        }
        
        render();
        
        // FPS calculation
        frameCount++;
        auto fpsElapsed = std::chrono::duration<float>(currentTime - fpsTimer).count();
        if (fpsElapsed >= 1.0f) {
            fps = frameCount / fpsElapsed;
            frameCount = 0;
            fpsTimer = currentTime;
        }
    }
}

void Engine::update(float deltaTime) {
    physics->update(deltaTime);
}

void Engine::render() {
    renderer->beginFrame();
    renderer->render(physics.get());
    renderer->endFrame();
}

void Engine::shutdown() {
    running = false;
    renderer.reset();
    physics.reset();
    codeExecutor.reset();
    std::cout << "✓ Engine shutdown complete" << std::endl;
}

bool Engine::executeUserCode(const std::string& code) {
    if (!codeExecutor) {
        std::cerr << "Code executor not initialized" << std::endl;
        return false;
    }
    return codeExecutor->execute(code);
}

std::string Engine::getLastError() const {
    return codeExecutor ? codeExecutor->getLastError() : "Engine not initialized";
}

float Engine::getFPS() const {
    return fps;
}
