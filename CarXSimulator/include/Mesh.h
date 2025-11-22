#pragma once
#include "Math.h"
#include <vector>

struct Vertex {
    Vector3 position;
    Vector3 normal;
    float u, v;
};

class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();
    
    void draw();
    
    static Mesh* createCube();
    static Mesh* createSphere(int segments);
    static Mesh* createPlane(int width, int depth);
    
private:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;
    
    void setupMesh();
};
