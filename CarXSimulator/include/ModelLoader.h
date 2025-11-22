#pragma once
#include "Mesh.h"
#include <string>
#include <vector>
#include <memory>

struct Model {
    std::vector<std::unique_ptr<Mesh>> meshes;
    Vector3 boundingBoxMin;
    Vector3 boundingBoxMax;
};

class ModelLoader {
public:
    static std::unique_ptr<Model> loadGLB(const std::string& filepath);
    static std::unique_ptr<Model> loadOBJ(const std::string& filepath);
    
private:
    static void calculateBoundingBox(Model* model);
};
