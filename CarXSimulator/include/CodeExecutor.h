#pragma once
#include <string>
#include <functional>
#include <map>

class PhysicsEngine;

class CodeExecutor {
public:
    CodeExecutor(PhysicsEngine* physics);
    
    bool execute(const std::string& code);
    std::string getLastError() const { return lastError; }
    
    // Register API functions
    void registerAPI();
    
private:
    bool parseAndExecute(const std::string& code);
    void setupSandbox();
    
    PhysicsEngine* physics;
    std::string lastError;
    
    // Simple command parser for demo
    using CommandFunc = std::function<bool(const std::vector<std::string>&)>;
    std::map<std::string, CommandFunc> commands;
};
