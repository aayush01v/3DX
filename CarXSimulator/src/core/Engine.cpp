#include "Engine.h"
#include "PhysicsEngine.h"
#include "Renderer.h"
#include "CodeExecutor.h"
#include <chrono>

Engine::Engine() : running(false), timeAccumulator(0.0f) {}

Engine::~Engine() {
    shutdown();
}

bool Engine::initialize(int width, int height) {
    physics = std::make_unique<PhysicsEngine>();
    renderer = std::make_unique<Renderer>();
    
    if (!renderer->initialize(width, height)) {
        return false;
    }
    
    codeExecutor = std::make_unique<CodeExecutor>(physics.get());
    codeExecutor->registerAPI();
    
    running = true;
    return true;
}

void Engine::run() {
    const float fixedDeltaTime = 1.0f / 60.0f;
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        timeAccumulator += deltaTime;
        
        while (timeAccumulator >= fixedDeltaTime) {
            update(fixedDeltaTime);
            timeAccumulator -= fixedDeltaTime;
        }
        
        render();
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
}

bool Engine::executeUserCode(const std::string& code) {
    return codeExecutor->execute(code);
}

std::string Engine::getLastError() const {
    return codeExecutor->getLastError();
}
