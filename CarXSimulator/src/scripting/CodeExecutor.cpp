#include "CodeExecutor.h"
#include "PhysicsEngine.h"
#include "Car.h"
#include "Terrain.h"
#include <sstream>
#include <vector>

CodeExecutor::CodeExecutor(PhysicsEngine* physics) : physics(physics) {
    registerAPI();
}

void CodeExecutor::registerAPI() {
    // Car creation
    apiFunctions["createCar"] = {
        "createCar",
        "Create a new car at specified position",
        {"x", "y", "z", "[mass]", "[power]"},
        [this](const std::vector<std::string>& args) {
        if (args.size() < 3) {
            lastError = "createCar requires x, y, z coordinates";
            return false;
        }
        float x = std::stof(args[0]);
        float y = std::stof(args[1]);
        float z = std::stof(args[2]);
        Car* car = physics->createCar(Vector3(x, y, z));
        
        // Optional parameters
        if (args.size() >= 4) car->setMass(std::stof(args[3]));
        if (args.size() >= 5) car->setEnginePower(std::stof(args[4]));
        
        lastOutput = "Car created successfully";
        return true;
    }};
    
    // Gravity control
    apiFunctions["setGravity"] = {
        "setGravity",
        "Set gravity vector",
        {"x", "y", "z"},
        [this](const std::vector<std::string>& args) {
        if (args.size() < 3) {
            lastError = "setGravity requires x, y, z components";
            return false;
        }
        float x = std::stof(args[0]);
        float y = std::stof(args[1]);
        float z = std::stof(args[2]);
        physics->setGravity(Vector3(x, y, z));
        lastOutput = "Gravity set to (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
        return true;
    }};
    
    // Terrain generation
    apiFunctions["generateTerrain"] = {
        "generateTerrain",
        "Generate procedural terrain",
        {"amplitude", "frequency", "[curvature]"},
        [this](const std::vector<std::string>& args) {
        if (args.size() < 2) {
            lastError = "generateTerrain requires amplitude and frequency";
            return false;
        }
        float amplitude = std::stof(args[0]);
        float frequency = std::stof(args[1]);
        
        auto terrain = std::make_unique<Terrain>(100, 100);
        terrain->generateProcedural(amplitude, frequency);
        
        if (args.size() >= 3) {
            terrain->setCurvature(std::stof(args[2]));
        }
        
        physics->setTerrain(std::move(terrain));
        lastOutput = "Terrain generated with amplitude=" + std::to_string(amplitude);
        return true;
    }};
    
    // Terrain presets
    apiFunctions["loadTerrainPreset"] = {
        "loadTerrainPreset",
        "Load terrain preset: flat, hills, mountains, valley, racetrack",
        {"type"},
        [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            lastError = "loadTerrainPreset requires type: flat, hills, mountains, valley, racetrack";
            return false;
        }
        
        auto terrain = std::make_unique<Terrain>(100, 100);
        std::string type = args[0];
        
        if (type == "flat") terrain->generateByType(TerrainType::FLAT);
        else if (type == "hills") terrain->generateByType(TerrainType::HILLS);
        else if (type == "mountains") terrain->generateByType(TerrainType::MOUNTAINS);
        else if (type == "valley") terrain->generateByType(TerrainType::VALLEY);
        else if (type == "racetrack") terrain->generateByType(TerrainType::RACETRACK);
        else {
            lastError = "Unknown terrain type: " + type;
            return false;
        }
        
        physics->setTerrain(std::move(terrain));
        lastOutput = "Loaded terrain preset: " + type;
        return true;
    }};
    
    // Car control
    apiFunctions["setThrottle"] = {
        "setThrottle",
        "Set car throttle (-1 to 1)",
        {"amount"},
        [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            lastError = "setThrottle requires amount (-1 to 1)";
            return false;
        }
        auto& cars = physics->getCars();
        if (!cars.empty()) {
            cars[0]->applyThrottle(std::stof(args[0]));
        }
        return true;
    }};
    
    apiFunctions["setSteering"] = {
        "setSteering",
        "Set steering angle (-1 to 1)",
        {"angle"},
        [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            lastError = "setSteering requires angle (-1 to 1)";
            return false;
        }
        auto& cars = physics->getCars();
        if (!cars.empty()) {
            cars[0]->applySteering(std::stof(args[0]));
        }
        return true;
    }};
    
    apiFunctions["setBrake"] = {
        "setBrake",
        "Set brake amount (0 to 1)",
        {"amount"},
        [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            lastError = "setBrake requires amount (0 to 1)";
            return false;
        }
        auto& cars = physics->getCars();
        if (!cars.empty()) {
            cars[0]->applyBrake(std::stof(args[0]));
        }
        return true;
    }};
    
    // Car properties
    apiFunctions["setCarMass"] = {
        "setCarMass",
        "Set car mass in kg",
        {"mass"},
        [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            lastError = "setCarMass requires mass in kg";
            return false;
        }
        auto& cars = physics->getCars();
        if (!cars.empty()) {
            cars[0]->setMass(std::stof(args[0]));
        }
        return true;
    }};
    
    apiFunctions["setTireGrip"] = {
        "setTireGrip",
        "Set tire grip coefficient (0.5 to 2.0)",
        {"grip"},
        [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            lastError = "setTireGrip requires grip coefficient (0.5 to 2.0)";
            return false;
        }
        auto& cars = physics->getCars();
        if (!cars.empty()) {
            cars[0]->setTireGrip(std::stof(args[0]));
        }
        return true;
    }};
    
    // Simulation control
    apiFunctions["reset"] = {
        "reset",
        "Reset simulation",
        {},
        [this](const std::vector<std::string>& args) {
        physics->reset();
        lastOutput = "Simulation reset";
        return true;
    }};
    
    // Advanced terrain features
    apiFunctions["addObstacle"] = {
        "addObstacle",
        "Add obstacle to terrain",
        {"x", "z", "radius", "height"},
        [this](const std::vector<std::string>& args) {
        if (args.size() < 4) {
            lastError = "addObstacle requires x, z, radius, height";
            return false;
        }
        auto* terrain = physics->getTerrain();
        if (terrain) {
            terrain->addObstacle(std::stof(args[0]), std::stof(args[1]), 
                               std::stof(args[2]), std::stof(args[3]));
            lastOutput = "Obstacle added";
        }
        return true;
    }};
    
    apiFunctions["addRamp"] = {
        "addRamp",
        "Add ramp to terrain",
        {"x", "z", "width", "length", "height"},
        [this](const std::vector<std::string>& args) {
        if (args.size() < 5) {
            lastError = "addRamp requires x, z, width, length, height";
            return false;
        }
        auto* terrain = physics->getTerrain();
        if (terrain) {
            terrain->addRamp(std::stof(args[0]), std::stof(args[1]), 
                           std::stof(args[2]), std::stof(args[3]), std::stof(args[4]));
            lastOutput = "Ramp added";
        }
        return true;
    }};
    
    apiFunctions["smoothTerrain"] = {
        "smoothTerrain",
        "Smooth terrain surface",
        {"[iterations]"},
        [this](const std::vector<std::string>& args) {
        auto* terrain = physics->getTerrain();
        if (terrain) {
            int iterations = args.empty() ? 1 : std::stoi(args[0]);
            terrain->smoothTerrain(iterations);
            lastOutput = "Terrain smoothed";
        }
        return true;
    }};
    
    apiFunctions["setTractionControl"] = {
        "setTractionControl",
        "Enable/disable traction control",
        {"enabled (0 or 1)"},
        [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            lastError = "setTractionControl requires 0 or 1";
            return false;
        }
        auto& cars = physics->getCars();
        if (!cars.empty()) {
            cars[0]->setTractionControl(std::stoi(args[0]) != 0);
            lastOutput = "Traction control " + std::string(std::stoi(args[0]) ? "enabled" : "disabled");
        }
        return true;
    }};
    
    apiFunctions["setABS"] = {
        "setABS",
        "Enable/disable anti-lock brakes",
        {"enabled (0 or 1)"},
        [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            lastError = "setABS requires 0 or 1";
            return false;
        }
        auto& cars = physics->getCars();
        if (!cars.empty()) {
            cars[0]->setAntiLockBrakes(std::stoi(args[0]) != 0);
            lastOutput = "ABS " + std::string(std::stoi(args[0]) ? "enabled" : "disabled");
        }
        return true;
    }};
}

bool CodeExecutor::execute(const std::string& code) {
    lastError.clear();
    return parseAndExecute(code);
}

bool CodeExecutor::parseAndExecute(const std::string& code) {
    std::istringstream iss(code);
    std::string command;
    iss >> command;
    
    auto it = commands.find(command);
    if (it == commands.end()) {
        lastError = "Unknown command: " + command;
        return false;
    }
    
    std::vector<std::string> args;
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }
    
    return it->second(args);
}

std::vector<std::string> CodeExecutor::getAvailableCommands() const {
    std::vector<std::string> commands;
    for (const auto& pair : apiFunctions) {
        commands.push_back(pair.first);
    }
    std::sort(commands.begin(), commands.end());
    return commands;
}

std::vector<std::string> CodeExecutor::getSuggestions(const std::string& partial) const {
    std::vector<std::string> suggestions;
    std::string lowerPartial = partial;
    std::transform(lowerPartial.begin(), lowerPartial.end(), lowerPartial.begin(), ::tolower);
    
    for (const auto& pair : apiFunctions) {
        std::string lowerCmd = pair.first;
        std::transform(lowerCmd.begin(), lowerCmd.end(), lowerCmd.begin(), ::tolower);
        
        if (lowerCmd.find(lowerPartial) == 0) {
            suggestions.push_back(pair.first);
        }
    }
    
    std::sort(suggestions.begin(), suggestions.end());
    return suggestions;
}

std::string CodeExecutor::getCommandHelp(const std::string& command) const {
    auto it = apiFunctions.find(command);
    if (it == apiFunctions.end()) {
        return "Command not found: " + command;
    }
    
    const APIFunction& func = it->second;
    std::string help = func.name + " - " + func.description + "\n";
    help += "Parameters: ";
    
    if (func.parameters.empty()) {
        help += "none";
    } else {
        for (size_t i = 0; i < func.parameters.size(); ++i) {
            if (i > 0) help += ", ";
            help += func.parameters[i];
        }
    }
    
    return help;
}

std::string CodeExecutor::getAPIDocumentation() const {
    std::string doc = "=== CarX Simulator API Documentation ===\n\n";
    
    std::vector<std::string> categories = {
        "Car Control", "Car Properties", "Terrain", "Physics", "Simulation"
    };
    
    for (const auto& pair : apiFunctions) {
        doc += pair.first + "\n";
        doc += "  " + pair.second.description + "\n";
        doc += "  Parameters: ";
        
        if (pair.second.parameters.empty()) {
            doc += "none";
        } else {
            for (size_t i = 0; i < pair.second.parameters.size(); ++i) {
                if (i > 0) doc += ", ";
                doc += pair.second.parameters[i];
            }
        }
        doc += "\n\n";
    }
    
    return doc;
}

bool CodeExecutor::executeScript(const std::string& script) {
    std::istringstream iss(script);
    std::string line;
    bool allSuccess = true;
    
    while (std::getline(iss, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == '/') continue;
        
        if (!execute(line)) {
            allSuccess = false;
            // Continue executing remaining lines
        }
    }
    
    return allSuccess;
}

void CodeExecutor::setVariable(const std::string& name, float value) {
    variables[name] = value;
}

float CodeExecutor::getVariable(const std::string& name) const {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second;
    }
    return 0.0f;
}

bool CodeExecutor::evaluateExpression(const std::string& expr, float& result) {
    // Simple expression evaluator for basic math
    // Supports: numbers, variables, +, -, *, /
    
    // Check if it's a variable
    auto it = variables.find(expr);
    if (it != variables.end()) {
        result = it->second;
        return true;
    }
    
    // Try to parse as number
    try {
        result = std::stof(expr);
        return true;
    } catch (...) {
        lastError = "Invalid expression: " + expr;
        return false;
    }
}

bool CodeExecutor::parseAndExecute(const std::string& code) {
    std::istringstream iss(code);
    std::string command;
    iss >> command;
    
    // Check for variable assignment
    if (code.find('=') != std::string::npos) {
        size_t eqPos = code.find('=');
        std::string varName = code.substr(0, eqPos);
        std::string valueStr = code.substr(eqPos + 1);
        
        // Trim whitespace
        varName.erase(0, varName.find_first_not_of(" \t"));
        varName.erase(varName.find_last_not_of(" \t") + 1);
        valueStr.erase(0, valueStr.find_first_not_of(" \t"));
        valueStr.erase(valueStr.find_last_not_of(" \t") + 1);
        
        float value;
        if (evaluateExpression(valueStr, value)) {
            setVariable(varName, value);
            lastOutput = varName + " = " + std::to_string(value);
            return true;
        }
        return false;
    }
    
    auto it = apiFunctions.find(command);
    if (it == apiFunctions.end()) {
        lastError = "Unknown command: " + command;
        return false;
    }
    
    std::vector<std::string> args;
    std::string arg;
    while (iss >> arg) {
        // Check if arg is a variable
        if (variables.find(arg) != variables.end()) {
            args.push_back(std::to_string(variables[arg]));
        } else {
            args.push_back(arg);
        }
    }
    
    return it->second.func(args);
}
