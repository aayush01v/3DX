#include "Engine.h"
#include <memory>

static std::unique_ptr<Engine> g_engine;

extern "C" {

// Mobile library exports for React Native / Flutter

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

EXPORT void CarX_Initialize(int width, int height) {
    if (!g_engine) {
        g_engine = std::make_unique<Engine>();
        g_engine->initialize(width, height);
    }
}

EXPORT void CarX_Shutdown() {
    if (g_engine) {
        g_engine->shutdown();
        g_engine.reset();
    }
}

EXPORT int CarX_ExecuteCode(const char* code) {
    if (!g_engine) return 0;
    return g_engine->executeUserCode(code) ? 1 : 0;
}

EXPORT const char* CarX_GetLastError() {
    if (!g_engine) return "";
    static std::string error;
    error = g_engine->getLastError();
    return error.c_str();
}

EXPORT void CarX_Update(float deltaTime) {
    if (g_engine) {
        g_engine->getPhysics()->update(deltaTime);
    }
}

EXPORT void CarX_Render() {
    if (g_engine) {
        g_engine->getRenderer()->beginFrame();
        g_engine->getRenderer()->render(g_engine->getPhysics());
        g_engine->getRenderer()->endFrame();
    }
}

// Get car data for rendering in native UI
EXPORT int CarX_GetCarCount() {
    if (!g_engine) return 0;
    return static_cast<int>(g_engine->getPhysics()->getCars().size());
}

EXPORT void CarX_GetCarPosition(int index, float* x, float* y, float* z) {
    if (!g_engine) return;
    const auto& cars = g_engine->getPhysics()->getCars();
    if (index >= 0 && index < cars.size()) {
        Vector3 pos = cars[index]->getPosition();
        *x = pos.x;
        *y = pos.y;
        *z = pos.z;
    }
}

}
