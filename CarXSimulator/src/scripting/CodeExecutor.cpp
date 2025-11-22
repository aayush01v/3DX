#include "CodeExecutor.h"
#include "PhysicsEngine.h"
#include "Car.h"
#include "Terrain.h"
#include <sstream>
#include <vector>

CodeExecutor::CodeExecutor(PhysicsEngine* physics) : physics(physics) {}

void CodeExecutor::registerAPI() {
    // Car creation
    commands["createCar"] = [this](const std::vector<std::string>& args) {
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
        
        return true;
    };
    
    // Gravity control
    commands["setGravity"] = [this](const std::vector<std::string>& args) {
        if (args.size() < 3) {
            lastError = "setGravity requires x, y, z components";
            return false;
        }
        float x = std::stof(args[0]);
        float y = std::stof(args[1]);
        float z = std::stof(args[2]);
        physics->setGravity(Vector3(x, y, z));
        return true;
    };
    
    // Terrain generation
    commands["generateTerrain"] = [this](const std::vector<std::string>& args) {
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
        return true;
    };
    
    // Terrain presets
    commands["loadTerrainPreset"] = [this](const std::vector<std::string>& args) {
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
        return true;
    };
    
    // Car control
    commands["setThrottle"] = [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            lastError = "setThrottle requires amount (-1 to 1)";
            return false;
        }
        auto& cars = physics->getCars();
        if (!cars.empty()) {
            cars[0]->applyThrottle(std::stof(args[0]));
        }
        return true;
    };
    
    commands["setSteering"] = [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            lastError = "setSteering requires angle (-1 to 1)";
            return false;
        }
        auto& cars = physics->getCars();
        if (!cars.empty()) {
            cars[0]->applySteering(std::stof(args[0]));
        }
        return true;
    };
    
    commands["setBrake"] = [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            lastError = "setBrake requires amount (0 to 1)";
            return false;
        }
        auto& cars = physics->getCars();
        if (!cars.empty()) {
            cars[0]->applyBrake(std::stof(args[0]));
        }
        return true;
    };
    
    // Car properties
    commands["setCarMass"] = [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            lastError = "setCarMass requires mass in kg";
            return false;
        }
        auto& cars = physics->getCars();
        if (!cars.empty()) {
            cars[0]->setMass(std::stof(args[0]));
        }
        return true;
    };
    
    commands["setTireGrip"] = [this](const std::vector<std::string>& args) {
        if (args.empty()) {
            lastError = "setTireGrip requires grip coefficient (0.5 to 2.0)";
            return false;
        }
        auto& cars = physics->getCars();
        if (!cars.empty()) {
            cars[0]->setTireGrip(std::stof(args[0]));
        }
        return true;
    };
    
    // Simulation control
    commands["reset"] = [this](const std::vector<std::string>& args) {
        physics->reset();
        return true;
    };
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
