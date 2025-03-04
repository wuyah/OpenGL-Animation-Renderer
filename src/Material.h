#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <string>

class Material {
public:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

    Material(glm::vec3 amb = glm::vec3(0.2f),
        glm::vec3 diff = glm::vec3(0.8f),
        glm::vec3 spec = glm::vec3(1.0f),
        float shininessVal = 32.0f)
        : ambient(amb), diffuse(diff), specular(spec), shininess(shininessVal) {}

    // 将材质数据传输到 Shader
    void SetUniforms(GLuint shaderProgram, const std::string& uniformName) const {
        glUseProgram(shaderProgram);
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformName + ".ambient").c_str()), 1, &ambient[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformName + ".diffuse").c_str()), 1, &diffuse[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformName + ".specular").c_str()), 1, &specular[0]);
        glUniform1f(glGetUniformLocation(shaderProgram, (uniformName + ".shininess").c_str()), shininess);
    }
};
