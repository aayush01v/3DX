#include "Engine.h"
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <string>

static Engine* g_engine = nullptr;

extern "C" {

EMSCRIPTEN_KEEPALIVE
void initEngine(int width, int height) {
    if (!g_engine) {
        g_engine = new Engine();
        g_engine->initialize(width, height);
    }
}

EMSCRIPTEN_KEEPALIVE
int executeCode(const char* code) {
    if (!g_engine) return 0;
    return g_engine->executeUserCode(std::string(code)) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
const char* getLastError() {
    if (!g_engine) return "";
    static std::string error;
    error = g_engine->getLastError();
    return error.c_str();
}

EMSCRIPTEN_KEEPALIVE
void updatePhysics(float deltaTime) {
    if (g_engine) {
        g_engine->getPhysics()->update(deltaTime);
    }
}

}

void mainLoop() {
    if (g_engine) {
        g_engine->getPhysics()->update(1.0f / 60.0f);
        g_engine->getRenderer()->beginFrame();
        g_engine->getRenderer()->render(g_engine->getPhysics());
        g_engine->getRenderer()->endFrame();
    }
}

int main() {
    emscripten_set_main_loop(mainLoop, 0, 1);
    return 0;
}
