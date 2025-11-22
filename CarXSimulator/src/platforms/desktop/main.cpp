#include "Engine.h"
#include <iostream>

// Platform-specific includes
#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
    std::cout << "CarX Physics Simulator - Desktop Edition\n";
    std::cout << "=========================================\n\n";
    
    Engine engine;
    
    if (!engine.initialize(1280, 720)) {
        std::cerr << "Failed to initialize engine\n";
        return 1;
    }
    
    std::cout << "Engine initialized successfully\n";
    std::cout << "Commands:\n";
    std::cout << "  createCar <x> <y> <z>\n";
    std::cout << "  setGravity <x> <y> <z>\n";
    std::cout << "  generateTerrain <amplitude> <frequency>\n\n";
    
    // Example: Setup initial scene
    engine.executeUserCode("generateTerrain 5.0 2.0");
    engine.executeUserCode("createCar 0 5 0");
    engine.executeUserCode("setGravity 0 -9.81 0");
    
    // Run main loop
    engine.run();
    
    return 0;
}
