#include "Cube.h"

Cube::Cube(glm::vec3 cubeMin, glm::vec3 cubeMax) {
    // Model matrix.
    model = glm::mat4(1.0f);

    // The color of the cube. Try setting it to something else!
    color = glm::vec3(1.0f, 0.95f, 0.1f);

    // Specify vertex positions
    positions = {
        // Front
        glm::vec3(cubeMin.x, cubeMin.y, cubeMax.z),
        glm::vec3(cubeMax.x, cubeMin.y, cubeMax.z),
        glm::vec3(cubeMax.x, cubeMax.y, cubeMax.z),
        glm::vec3(cubeMin.x, cubeMax.y, cubeMax.z),

        // Back
        glm::vec3(cubeMax.x, cubeMin.y, cubeMin.z),
        glm::vec3(cubeMin.x, cubeMin.y, cubeMin.z),
        glm::vec3(cubeMin.x, cubeMax.y, cubeMin.z),
        glm::vec3(cubeMax.x, cubeMax.y, cubeMin.z),

        // Top
        glm::vec3(cubeMin.x, cubeMax.y, cubeMax.z),
        glm::vec3(cubeMax.x, cubeMax.y, cubeMax.z),
        glm::vec3(cubeMax.x, cubeMax.y, cubeMin.z),
        glm::vec3(cubeMin.x, cubeMax.y, cubeMin.z),

        // Bottom
        glm::vec3(cubeMin.x, cubeMin.y, cubeMin.z),
        glm::vec3(cubeMax.x, cubeMin.y, cubeMin.z),
        glm::vec3(cubeMax.x, cubeMin.y, cubeMax.z),
        glm::vec3(cubeMin.x, cubeMin.y, cubeMax.z),

        // Left
        glm::vec3(cubeMin.x, cubeMin.y, cubeMin.z),
        glm::vec3(cubeMin.x, cubeMin.y, cubeMax.z),
        glm::vec3(cubeMin.x, cubeMax.y, cubeMax.z),
        glm::vec3(cubeMin.x, cubeMax.y, cubeMin.z),

        // Right
        glm::vec3(cubeMax.x, cubeMin.y, cubeMax.z),
        glm::vec3(cubeMax.x, cubeMin.y, cubeMin.z),
        glm::vec3(cubeMax.x, cubeMax.y, cubeMin.z),
        glm::vec3(cubeMax.x, cubeMax.y, cubeMax.z)};

    // Specify normals
    normals = {
        // Front
        glm::vec3(0, 0, 1),
        glm::vec3(0, 0, 1),
        glm::vec3(0, 0, 1),
        glm::vec3(0, 0, 1),

        // Back
        glm::vec3(0, 0, -1),
        glm::vec3(0, 0, -1),
        glm::vec3(0, 0, -1),
        glm::vec3(0, 0, -1),

        // Top
        glm::vec3(0, 1, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(0, 1, 0),

        // Bottom
        glm::vec3(0, -1, 0),
        glm::vec3(0, -1, 0),
        glm::vec3(0, -1, 0),
        glm::vec3(0, -1, 0),

        // Left
        glm::vec3(-1, 0, 0),
        glm::vec3(-1, 0, 0),
        glm::vec3(-1, 0, 0),
        glm::vec3(-1, 0, 0),

        // Right
        glm::vec3(1, 0, 0),
        glm::vec3(1, 0, 0),
        glm::vec3(1, 0, 0),
        glm::vec3(1, 0, 0)};

    // Specify indices
    indices = {
        0, 1, 2, 0, 2, 3,        // Front
        4, 5, 6, 4, 6, 7,        // Back
        8, 9, 10, 8, 10, 11,     // Top
        12, 13, 14, 12, 14, 15,  // Bottom
        16, 17, 18, 16, 18, 19,  // Left
        20, 21, 22, 20, 22, 23,  // Right
    };

    // Generate a vertex array (VAO) and two vertex buffer objects (VBO).
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO_positions);
    glGenBuffers(1, &VBO_normals);

    // Bind to the VAO.
    glBindVertexArray(VAO);

    // Bind to the first VBO - We will use it to store the vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBO_positions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * positions.size(), positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    // Bind to the second VBO - We will use it to store the normals
    glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    // Generate EBO, bind the EBO to the bound VAO and send the data
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    // Unbind the VBOs.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Cube::~Cube() {
    // Delete the VBOs and the VAO.
    glDeleteBuffers(1, &VBO_positions);
    glDeleteBuffers(1, &VBO_normals);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void Cube::draw(const glm::mat4& viewProjMtx, GLuint shader) {
    // actiavte the shader program
    glUseProgram(shader);

    // get the locations and send the uniforms to the shader
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewProj"), 1, false, (float*)&viewProjMtx);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, (float*)&model);
    glUniform3fv(glGetUniformLocation(shader, "DiffuseColor"), 1, &color[0]);

    // Bind the VAO
    glBindVertexArray(VAO);

    // draw the points using triangles, indexed with the EBO
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // Unbind the VAO and shader program
    glBindVertexArray(0);
    glUseProgram(0);
}

void Cube::update() {
    // Spin the cube
    spin(0.05f);
}

void Cube::spin(float deg) {
    // Update the model matrix by multiplying a rotation matrix
    model = model * glm::rotate(glm::radians(deg), glm::vec3(0.0f, 1.0f, 0.0f));
}

void Cube::setModelMatrix(const glm::mat4& matrix) {
    model = matrix;
}


namespace CubeHelper {
    std::vector<GLfloat> generateCubeVertices(glm::vec3 min, glm::vec3 max) {
        // ÿ����4�����㣬��6���棬24������
        return {
            // Front face
            min.x, min.y, max.z,  // 0
            max.x, min.y, max.z,  // 1
            max.x, max.y, max.z,  // 2
            min.x, max.y, max.z,  // 3

            // Back face
            max.x, min.y, min.z,  // 4
            min.x, min.y, min.z,  // 5
            min.x, max.y, min.z,  // 6
            max.x, max.y, min.z,  // 7

            // Top face
            min.x, max.y, max.z,  // 8
            max.x, max.y, max.z,  // 9
            max.x, max.y, min.z,  // 10
            min.x, max.y, min.z,  // 11

            // Bottom face
            min.x, min.y, min.z,  // 12
            max.x, min.y, min.z,  // 13
            max.x, min.y, max.z,  // 14
            min.x, min.y, max.z,  // 15

            // Left face
            min.x, min.y, min.z,  // 16
            min.x, min.y, max.z,  // 17
            min.x, max.y, max.z,  // 18
            min.x, max.y, min.z,  // 19

            // Right face
            max.x, min.y, max.z,  // 20
            max.x, min.y, min.z,  // 21
            max.x, max.y, min.z,  // 22
            max.x, max.y, max.z   // 23
        };
    }

    std::vector<GLuint> generateCubeIndices(GLuint indexOffset = 0) {
        // ÿ����2�������Σ�6���棬��12�������Σ�36������
        return {
            // Front face
            0 + indexOffset, 1 + indexOffset, 2 + indexOffset,
            2 + indexOffset, 3 + indexOffset, 0 + indexOffset,

            // Back face
            4 + indexOffset, 5 + indexOffset, 6 + indexOffset,
            6 + indexOffset, 7 + indexOffset, 4 + indexOffset,

            // Top face
            8 + indexOffset, 9 + indexOffset, 10 + indexOffset,
            10 + indexOffset, 11 + indexOffset, 8 + indexOffset,

            // Bottom face
            12 + indexOffset, 13 + indexOffset, 14 + indexOffset,
            14 + indexOffset, 15 + indexOffset, 12 + indexOffset,

            // Left face
            16 + indexOffset, 17 + indexOffset, 18 + indexOffset,
            18 + indexOffset, 19 + indexOffset, 16 + indexOffset,

            // Right face
            20 + indexOffset, 21 + indexOffset, 22 + indexOffset,
            22 + indexOffset, 23 + indexOffset, 20 + indexOffset
        };
    }

    std::vector<GLfloat> generateCubeNormals() {
        // ÿ����4�����㣬6���棬24������
        return {
            // Front face normals
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,

            // Back face normals
            0.0f,  0.0f, -1.0f,
            0.0f,  0.0f, -1.0f,
            0.0f,  0.0f, -1.0f,
            0.0f,  0.0f, -1.0f,

            // Top face normals
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,

            // Bottom face normals
            0.0f, -1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,

            // Left face normals
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,

           // Right face normals
           1.0f,  0.0f,  0.0f,
           1.0f,  0.0f,  0.0f,
           1.0f,  0.0f,  0.0f,
           1.0f,  0.0f,  0.0f
        };
    }
}
