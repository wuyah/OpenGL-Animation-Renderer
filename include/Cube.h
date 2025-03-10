#pragma once

#include <vector>

#include "core.h"

class Cube {
private:
    GLuint VAO;
    GLuint VBO_positions, VBO_normals, EBO;

    glm::mat4 model;
    glm::vec3 color;

    // Cube Information
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

public:
    Cube(glm::vec3 cubeMin = glm::vec3(-1, -1, -1), glm::vec3 cubeMax = glm::vec3(1, 1, 1));
    ~Cube();

    void draw(const glm::mat4& viewProjMtx, GLuint shader);
    void update();
    void setModelMatrix(const glm::mat4& matrix);
    void spin(float deg);
};

namespace CubeHelper {
    std::vector<GLfloat> generateCubeVertices(glm::vec3 min, glm::vec3 max);
    std::vector<GLuint> generateCubeIndices(GLuint indexOffset);
    std::vector<GLfloat> generateCubeNormals();
}
