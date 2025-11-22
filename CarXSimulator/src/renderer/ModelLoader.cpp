#include "ModelLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

// For GLB loading, we'll use tinygltf library
// #define TINYGLTF_IMPLEMENTATION
// #include "tiny_gltf.h"

std::unique_ptr<Model> ModelLoader::loadGLB(const std::string& filepath) {
    auto model = std::make_unique<Model>();
    
    // TODO: Implement GLB loading with tinygltf
    // For now, create a simple car mesh as fallback
    
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // Simple car body (box)
    float carVertices[] = {
        // Front face
        -1.5f, 0.0f,  2.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         1.5f, 0.0f,  2.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
         1.5f, 1.0f,  2.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -1.5f, 1.0f,  2.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
        // Back face
        -1.5f, 0.0f, -2.0f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
         1.5f, 0.0f, -2.0f,  0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
         1.5f, 1.0f, -2.0f,  0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -1.5f, 1.0f, -2.0f,  0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
    };
    
    for (int i = 0; i < 8; ++i) {
        Vertex v;
        v.position = Vector3(carVertices[i * 8], carVertices[i * 8 + 1], carVertices[i * 8 + 2]);
        v.normal = Vector3(carVertices[i * 8 + 3], carVertices[i * 8 + 4], carVertices[i * 8 + 5]);
        v.u = carVertices[i * 8 + 6];
        v.v = carVertices[i * 8 + 7];
        vertices.push_back(v);
    }
    
    unsigned int carIndices[] = {
        0, 1, 2, 2, 3, 0,  // Front
        4, 5, 6, 6, 7, 4,  // Back
        0, 4, 7, 7, 3, 0,  // Left
        1, 5, 6, 6, 2, 1,  // Right
        3, 2, 6, 6, 7, 3,  // Top
        0, 1, 5, 5, 4, 0   // Bottom
    };
    
    for (unsigned int idx : carIndices) {
        indices.push_back(idx);
    }
    
    model->meshes.push_back(std::make_unique<Mesh>(vertices, indices));
    calculateBoundingBox(model.get());
    
    std::cout << "Loaded car model (fallback): " << filepath << std::endl;
    
    return model;
}

std::unique_ptr<Model> ModelLoader::loadOBJ(const std::string& filepath) {
    auto model = std::make_unique<Model>();
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << filepath << std::endl;
        return model;
    }
    
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "v") {
            Vector3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        }
        else if (prefix == "vn") {
            Vector3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (prefix == "f") {
            // Simple face parsing (assumes triangles)
            for (int i = 0; i < 3; ++i) {
                std::string vertexStr;
                iss >> vertexStr;
                
                std::istringstream vss(vertexStr);
                std::string indexStr;
                int posIdx = 0, normalIdx = 0;
                
                std::getline(vss, indexStr, '/');
                posIdx = std::stoi(indexStr) - 1;
                
                std::getline(vss, indexStr, '/'); // Skip texture
                
                if (std::getline(vss, indexStr, '/')) {
                    normalIdx = std::stoi(indexStr) - 1;
                }
                
                Vertex v;
                v.position = positions[posIdx];
                v.normal = normalIdx < normals.size() ? normals[normalIdx] : Vector3(0, 1, 0);
                v.u = 0;
                v.v = 0;
                
                vertices.push_back(v);
                indices.push_back(static_cast<unsigned int>(vertices.size() - 1));
            }
        }
    }
    
    if (!vertices.empty()) {
        model->meshes.push_back(std::make_unique<Mesh>(vertices, indices));
        calculateBoundingBox(model.get());
        std::cout << "Loaded OBJ model: " << filepath << std::endl;
    }
    
    return model;
}

void ModelLoader::calculateBoundingBox(Model* model) {
    if (model->meshes.empty()) return;
    
    model->boundingBoxMin = Vector3(1e10f, 1e10f, 1e10f);
    model->boundingBoxMax = Vector3(-1e10f, -1e10f, -1e10f);
    
    // TODO: Iterate through all mesh vertices to find min/max
}
