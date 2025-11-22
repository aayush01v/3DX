#pragma once
#include <memory>
#include <string>

class PhysicsEngine;
class Renderer;
class CodeExecutor;

class Engine {
public:
    Engine();
    ~Engine();
    
    bool initialize(int width, int height);
    void run();
    void shutdown();
    
    // Code execution interface
    bool executeUserCode(const std::string& code);
    std::string getLastError() const;
    
    PhysicsEngine* getPhysics() { return physics.get(); }
    Renderer* getRenderer() { return renderer.get(); }
    
private:
    void update(float deltaTime);
    void render();
    
    std::unique_ptr<PhysicsEngine> physics;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<CodeExecutor> codeExecutor;
    
    bool running;
    float timeAccumulator;
};
