#pragma once
#include <string>
#include <functional>
#include <map>

class PhysicsEngine;

// Enhanced code execution with better error handling and autocomplete
struct APIFunction {
    std::string name;
    std::string description;
    std::vector<std::string> parameters;
    std::function<bool(const std::vector<std::string>&)> func;
};

class CodeExecutor {
public:
    CodeExecutor(PhysicsEngine* physics);
    
    bool execute(const std::string& code);
    std::string getLastError() const { return lastError; }
    std::string getLastOutput() const { return lastOutput; }
    
    // Register API functions
    void registerAPI();
    
    // Autocomplete and help
    std::vector<std::string> getAvailableCommands() const;
    std::vector<std::string> getSuggestions(const std::string& partial) const;
    std::string getCommandHelp(const std::string& command) const;
    std::string getAPIDocumentation() const;
    
    // Batch execution
    bool executeScript(const std::string& script); // Multi-line script
    
    // Variable storage for scripting
    void setVariable(const std::string& name, float value);
    float getVariable(const std::string& name) const;
    
private:
    bool parseAndExecute(const std::string& code);
    void setupSandbox();
    bool evaluateExpression(const std::string& expr, float& result);
    
    PhysicsEngine* physics;
    std::string lastError;
    std::string lastOutput;
    
    // Enhanced command system
    std::map<std::string, APIFunction> apiFunctions;
    std::map<std::string, float> variables;
};
