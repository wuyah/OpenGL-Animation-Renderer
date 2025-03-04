#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <string>

class DirectionalLight {
public:
    glm::vec3 direction;
    glm::vec3 color;  // 统一用一个颜色控制

    float intensity;

    DirectionalLight(glm::vec3 dir = glm::vec3(-0.2f, -1.0f, -0.3f),
        glm::vec3 col = glm::vec3(1.0f), float intense = 1.f)  
        : direction(glm::normalize(dir)), color(col), intensity(intense) {}

    void SetUniforms(GLuint shaderProgram, const std::string& uniformName) const {
        glUseProgram(shaderProgram);
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformName + ".direction").c_str()), 1, &direction[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformName + ".color").c_str()), 1, &color[0]);
    }
};

class PointLight {
public:
    glm::vec3 position;
    glm::vec3 color;
    float intensity;

    PointLight(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 col = glm::vec3(1.0f), float inten = 1.0f)
        : position(pos), color(col), intensity(inten) {}

    void SetUniforms(GLuint shaderProgram, const std::string& uniformName) const {
        glUseProgram(shaderProgram);
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformName + ".position").c_str()), 1, &position[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformName + ".color").c_str()), 1, &color[0]);
        glUniform1f(glGetUniformLocation(shaderProgram, (uniformName + ".intensity").c_str()), intensity);
    }
};
