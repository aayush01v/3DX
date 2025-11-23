#pragma once
#include <string>

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();
    
    void use();
    void setMat4(const char* name, const float* value);
    void setVec3(const char* name, float x, float y, float z);
    void setFloat(const char* name, float value);
    void setInt(const char* name, int value);
    
private:
    unsigned int programID;
    bool compile(const std::string& vertexCode, const std::string& fragmentCode);
};
