#include "Mesh.h"
#include <cmath>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : VAO(0), VBO(0), EBO(0), indexCount(indices.size()) {
    setupMesh();
}

Mesh::~Mesh() {
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);
    // glDeleteBuffers(1, &EBO);
}

void Mesh::setupMesh() {
    // OpenGL setup
    // glGenVertexArrays(1, &VAO);
    // glGenBuffers(1, &VBO);
    // glGenBuffers(1, &EBO);
}

void Mesh::draw() {
    // glBindVertexArray(VAO);
    // glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

Mesh* Mesh::createCube() {
    std::vector<Vertex> vertices = {
        // Front face
        {{-1, -1,  1}, {0, 0, 1}, 0, 0},
        {{ 1, -1,  1}, {0, 0, 1}, 1, 0},
        {{ 1,  1,  1}, {0, 0, 1}, 1, 1},
        {{-1,  1,  1}, {0, 0, 1}, 0, 1},
        // Add other faces...
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2, 2, 3, 0,  // Front
        // Add other faces...
    };
    
    return new Mesh(vertices, indices);
}

Mesh* Mesh::createSphere(int segments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * 3.14159f / segments;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);
        
        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2.0f * 3.14159f / segments;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);
            
            Vector3 pos(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
            vertices.push_back({pos, pos.normalized(), 
                              (float)lon / segments, (float)lat / segments});
        }
    }
    
    // Generate indices
    for (int lat = 0; lat < segments; ++lat) {
        for (int lon = 0; lon < segments; ++lon) {
            int first = lat * (segments + 1) + lon;
            int second = first + segments + 1;
            
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
    
    return new Mesh(vertices, indices);
}
