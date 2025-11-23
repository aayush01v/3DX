#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) 
    : programID(0) {
    
    std::string vertexCode, fragmentCode;
    std::ifstream vShaderFile, fShaderFile;
    
    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        
        vShaderFile.close();
        fShaderFile.close();
        
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        
        compile(vertexCode, fragmentCode);
    }
    catch (std::exception& e) {
        std::cerr << "ERROR: Shader file read failed: " << e.what() << std::endl;
    }
}

Shader::~Shader() {
    // glDeleteProgram(programID);
}

void Shader::use() {
    // glUseProgram(programID);
}

void Shader::setMat4(const char* name, const float* value) {
    // GLint location = glGetUniformLocation(programID, name);
    // glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void Shader::setVec3(const char* name, float x, float y, float z) {
    // GLint location = glGetUniformLocation(programID, name);
    // glUniform3f(location, x, y, z);
}

void Shader::setFloat(const char* name, float value) {
    // GLint location = glGetUniformLocation(programID, name);
    // glUniform1f(location, value);
}

void Shader::setInt(const char* name, int value) {
    // GLint location = glGetUniformLocation(programID, name);
    // glUniform1i(location, value);
}

bool Shader::compile(const std::string& vertexCode, const std::string& fragmentCode) {
    // Compile shaders (OpenGL-specific implementation)
    return true;
}
