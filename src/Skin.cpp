#include "Skin.h"


bool Skin::loadFromFile(const std::string& filename) {
    Tokenizer tokenizer;

    if (!tokenizer.Open(filename.c_str())) {
        std::cerr << "Error opening skin file: " << filename << std::endl;
        return false;
    }

    char token[256];
    int vertexCount = 0, triangleCount = 0, weightCount = 0, normalCount = 0;
    int bindCount = 0;

    while (true) {
        if (!tokenizer.GetToken(token)) break;

        if (strcmp(token, "positions") == 0) {
            vertexCount = tokenizer.GetInt();
            vertices.resize(vertexCount);
            tokenizer.GetToken(token);
            if (!(strcmp(token, "{") == 0)) {
                std::cerr << "Error in skin file: " << filename << "position dont have '{'" << std::endl;
            }
            for (int i = 0; i < vertexCount; ++i) {
                vertices[i].position.x = tokenizer.GetFloat();
                vertices[i].position.y = tokenizer.GetFloat();
                vertices[i].position.z = tokenizer.GetFloat();
            }
            tokenizer.GetToken(token);

            if (!(strcmp(token, "}") == 0)) {
                std::cerr << "Error in skin file: " << filename << "position dont have '}'" << std::endl;
            }

        }
        else if (strcmp(token, "normals") == 0) {
            normalCount = tokenizer.GetInt();
            tokenizer.GetToken(token);

            assert(normalCount == vertexCount);
            if (!(strcmp(token, "{") == 0)) {
                std::cerr << "Error in skin file: " << filename << "normal dont have '{'" << std::endl;
            }

            for (int i = 0; i < normalCount; ++i) {
                vertices[i].normal.x = tokenizer.GetFloat();
                vertices[i].normal.y = tokenizer.GetFloat();
                vertices[i].normal.z = tokenizer.GetFloat();
            }
            tokenizer.GetToken(token);

            if (!(strcmp(token, "}") == 0)) {
                std::cerr << "Error in skin file: " << filename << "normal dont have '}'" << std::endl;
            }

        }
        else if (strcmp(token, "skinweights") == 0) {
            bindCount = tokenizer.GetInt();
            //triangles.reserve(triangleCount);
            tokenizer.GetToken(token);

            if (!(strcmp(token, "{") == 0)) {
                std::cerr << "Error in skin file: " << filename << "bindings dont have '{'" << std::endl;
            }

            for (int i = 0; i < bindCount; ++i) {
                int bindingCount = tokenizer.GetInt();

                for (int j = 0; j < bindingCount; j++) {
                    int index = tokenizer.GetInt();
                    float weight = tokenizer.GetFloat();
                    vertices[i].weights.push_back(VertexWeight(index, weight));
                }

            }
            tokenizer.GetToken(token);

            if (!(strcmp(token, "}") == 0)) {
                std::cerr << "Error in skin file: " << filename << "bindings dont have '}'" << std::endl;
            }

        }

        else if (strcmp(token, "triangles") == 0) {
            triangleCount = tokenizer.GetInt();
            triangles.reserve(triangleCount);
            tokenizer.GetToken(token);

            if (!(strcmp(token, "{") == 0)) {
                std::cerr << "Error in skin file: " << filename << "triangles dont have '{'" << std::endl;
            }

            for (int i = 0; i < triangleCount; ++i) {
                int v0 = tokenizer.GetInt();
                int v1 = tokenizer.GetInt();
                int v2 = tokenizer.GetInt();
                triangles.emplace_back(v0, v1, v2);
            }
            tokenizer.GetToken(token);

            if (!(strcmp(token, "}") == 0)) {
                std::cerr << "Error in skin file: " << filename << "triangles dont have '}'" << std::endl;
            }

        }
        else if (strcmp(token, "bindings") == 0) {
            bindCount = tokenizer.GetInt();
            tokenizer.GetToken(token);

            if (!(strcmp(token, "{") == 0)) {
                std::cerr << "Error in skin file: " << filename << "bindings dont have '{'" << std::endl;
            }
            for (int i = 0; i < bindCount; i++) {
                tokenizer.GetToken(token);

                if (!(strcmp(token, "matrix") == 0)) {
                    std::cerr << "Error in skin file: " << filename << "bindings dont have 'matrix'" << std::endl;
                }
                tokenizer.GetToken(token);

                if (!(strcmp(token, "{") == 0)) {
                    std::cerr << "Error in skin file: " << filename << "bindings's matrix dont have '{'" << std::endl;
                }
                glm::mat4 matrix = glm::identity<glm::mat4>();
                for (int row = 0; row < 3; row++) {
                    matrix[row][0] = tokenizer.GetFloat(); // x
                    matrix[row][1] = tokenizer.GetFloat(); // y
                    matrix[row][2] = tokenizer.GetFloat(); // z
                    matrix[row][3] = 0.0f;                 // 行末
                }

                // 2) 第四行 (平移)
                matrix[3][0] = tokenizer.GetFloat(); // tx
                matrix[3][1] = tokenizer.GetFloat(); // ty
                matrix[3][2] = tokenizer.GetFloat(); // tz
                matrix[3][3] = 1.0f;

                tokenizer.GetToken(token);
                if (!(strcmp(token, "}") == 0)) {
                    std::cerr << "Error in skin file: " << filename << "bindings's matrix dont have '{'" << std::endl;
                }
                bindingMats.push_back(matrix);
            }
            tokenizer.GetToken(token);

            if (!(strcmp(token, "}") == 0)) {
                std::cerr << "Error in skin file: " << filename << "bindings dont have '{'" << std::endl;
            }
        }
        else {
            tokenizer.SkipLine();
        }
    }

    tokenizer.Close();
    return true;
}

void Skin::computeNormals() {
    // Reset all normals
    for (auto& vertex : vertices) {
        vertex.normal = glm::vec3(0.0f);
    }

    // Accumulate face normals
    for (const auto& triangle : triangles) {
        glm::vec3 normal = triangle.computeNormal(vertices);
        vertices[triangle.v0].normal += normal;
        vertices[triangle.v1].normal += normal;
        vertices[triangle.v2].normal += normal;
    }

    // Normalize the accumulated normals
    for (auto& vertex : vertices) {
        vertex.normal = glm::normalize(vertex.normal);
    }
}
