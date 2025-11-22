cmake_minimum_required(VERSION 3.20)
project(CarXSimulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_WASM "Build for WebAssembly" OFF)
option(USE_VULKAN "Use Vulkan instead of OpenGL" OFF)
option(BUILD_SHARED "Build shared library for mobile" OFF)

# Core engine sources
set(CORE_SOURCES
    src/core/Engine.cpp
    src/physics/PhysicsEngine.cpp
    src/physics/Car.cpp
    src/physics/Terrain.cpp
    src/physics/Forces.cpp
    src/renderer/Renderer.cpp
    src/renderer/Shader.cpp
    src/renderer/Mesh.cpp
    src/renderer/Camera.cpp
    src/scripting/CodeExecutor.cpp
    src/utils/Math.cpp
)

# Graphics backend
if(USE_VULKAN)
    find_package(Vulkan REQUIRED)
    list(APPEND CORE_SOURCES src/renderer/VulkanBackend.cpp)
    add_compile_definitions(USE_VULKAN)
else()
    find_package(OpenGL REQUIRED)
    list(APPEND CORE_SOURCES src/renderer/OpenGLBackend.cpp)
endif()

# WebAssembly build
if(BUILD_WASM)
    add_executable(CarXSimulator ${CORE_SOURCES} src/platforms/wasm/main.cpp)
    set_target_properties(CarXSimulator PROPERTIES
        LINK_FLAGS "-s USE_WEBGL2=1 -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_FUNCTIONS='[\"_main\",\"_executeCode\",\"_updatePhysics\"]' -s EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]'"
    )
# Shared library for mobile
elseif(BUILD_SHARED)
    add_library(CarXSimulator SHARED ${CORE_SOURCES} src/platforms/mobile/exports.cpp)
# Native executable
else()
    find_package(glfw3 REQUIRED)
    add_executable(CarXSimulator ${CORE_SOURCES} src/platforms/desktop/main.cpp)
    target_link_libraries(CarXSimulator PRIVATE glfw)
endif()

# Link graphics
if(USE_VULKAN)
    target_link_libraries(CarXSimulator PRIVATE Vulkan::Vulkan)
else()
    target_link_libraries(CarXSimulator PRIVATE OpenGL::GL)
endif()

target_include_directories(CarXSimulator PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)

# Install
install(TARGETS CarXSimulator
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
