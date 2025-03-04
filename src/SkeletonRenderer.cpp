#include "SkeletonRenderer.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

SkeletonRenderer::SkeletonRenderer()
    : VAO(0), VBO(0), EBO(0), VBO_normals(0), skeleton(nullptr), 
    totalBones(0), 
    renderMode(SkeletonRenderMode::Wireframe) {}

SkeletonRenderer::~SkeletonRenderer() {
    cleanup();
}

void checkOpenGLError(const std::string& context) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "[OpenGL Error] (" << err << ") in " << context << std::endl;
    }
}


void SkeletonRenderer::cleanup() {
    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO_normals) {
        glDeleteBuffers(1, &VBO_normals);
        VBO_normals = 0;
    }
    if (VBO) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (EBO) {
        glDeleteBuffers(1, &EBO);
        EBO = 0;
    }
}
void SkeletonRenderer::initialize(Skeleton& skel) {
    cleanup();

    skeleton = &skel;
    renderMode = SkeletonRenderMode::Wireframe;

    setupCubeBuffers(); 

}

// Skin initialize
void SkeletonRenderer::initialize(Skeleton& skel, Skin* skin) {
    cleanup(); 

    skeleton = &skel;
    renderMode = SkeletonRenderMode::Wireframe;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    if (render_skin && skin) { 
        this->skin = skin;  
        if (!renderInGPU)
            setupSkinBuffersCPU();
        else
            setupSkinBuffersGPU();
    }
    else {
        this->skin = nullptr; 
        setupCubeBuffers(); 
    }
}

void SkeletonRenderer::setupCubeBuffers() {
    if (!skeleton) return;

    vertexData.clear();
    indexData.clear();
    normalData.clear();

    size_t currentIndex = 0;
    const auto& joints = skeleton->getJointData();

    for (const auto& joint : joints) {
        auto jointVertices = CubeHelper::generateCubeVertices(joint->boxMin, joint->boxMax);
        vertexData.insert(vertexData.end(), jointVertices.begin(), jointVertices.end());

        auto jointIndices = CubeHelper::generateCubeIndices(static_cast<GLuint>(currentIndex));
        indexData.insert(indexData.end(), jointIndices.begin(), jointIndices.end());
        auto jointNormals = CubeHelper::generateCubeNormals();
        normalData.insert(normalData.end(), jointNormals.begin(), jointNormals.end());

        currentIndex += jointVertices.size() / 3;
    }
    assert(normalData.size() == vertexData.size());
    totalBones = joints.size();


    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        vertexData.size() * sizeof(GLfloat),
        vertexData.data(),
        GL_STATIC_DRAW);
    checkOpenGLError("VBOBind");
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    checkOpenGLError("VBO Enable");

    // Set vbo normals
    glGenBuffers(1, &VBO_normals);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
    glBufferData(GL_ARRAY_BUFFER,
        normalData.size() * sizeof(GLfloat),
        normalData.data(),
        GL_STATIC_DRAW);
    checkOpenGLError("Normal Bind");
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    checkOpenGLError("Normal Enable");


    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indexData.size() * sizeof(GLuint),
        indexData.data(),
        GL_STATIC_DRAW);
    checkOpenGLError("EBO Bind");

    glBindVertexArray(0);
}

void SkeletonRenderer::setupSkinBuffersCPU() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, skin->vertices.size() * sizeof(SkinVertex), skin->vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, skin->triangles.size() * sizeof(Triangle), skin->triangles.data(), GL_STATIC_DRAW);

    // Position attribute (layout = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), (void*)offsetof(SkinVertex, position));
    glEnableVertexAttribArray(0);

    // Normal attribute (layout = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), (void*)offsetof(SkinVertex, normal));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void SkeletonRenderer::setupSkinBuffersGPU() {
    std::vector<GPUSkinVertex> gpuVertices;
    gpuVertices.reserve(skin->vertices.size());

    for (const auto& srcVertex : skin->vertices) {
        gpuVertices.emplace_back(static_cast<const SkinVertex&>(srcVertex));
    }

    // 创建VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // 顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        gpuVertices.size() * sizeof(GPUSkinVertex),
        gpuVertices.data(),
        GL_STATIC_DRAW);

    // 属性设置
    // Position (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(GPUSkinVertex),
        (void*)offsetof(GPUSkinVertex, position));
    glEnableVertexAttribArray(0);

    // Normal (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(GPUSkinVertex),
        (void*)offsetof(GPUSkinVertex, normal));
    glEnableVertexAttribArray(1);

    // Joint indices (location 2) - 使用整数属性
    glVertexAttribIPointer(2, 4, GL_UNSIGNED_BYTE,
        sizeof(GPUSkinVertex),
        (void*)offsetof(GPUSkinVertex, jointIndices));
    glEnableVertexAttribArray(2);

    // Weights (location 3) - 标准化到[0,1]
    glVertexAttribPointer(3, 3, GL_UNSIGNED_BYTE, GL_TRUE,
        sizeof(GPUSkinVertex),
        (void*)offsetof(GPUSkinVertex, weights));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        skin->triangles.size() * sizeof(Triangle),
        skin->triangles.data(),
        GL_STATIC_DRAW);

    glBindVertexArray(0);
}


void SkeletonRenderer::renderSkinCPU(const glm::mat4& viewProjMatrix, GLuint shaderProgram, const glm::vec3 cameraPos) {
    glUseProgram(shaderProgram);

    GLuint vpLoc = glGetUniformLocation(shaderProgram, "viewProj");
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint camPos = glGetUniformLocation(shaderProgram, "CameraPos");

    glm::mat4 modelMatrix = glm::mat4(1.0f);  // Modify if needed
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(vpLoc, 1, GL_FALSE, &viewProjMatrix[0][0]);
    glUniform3fv(camPos, 1, &cameraPos[0]);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, skin->triangles.size()*3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void SkeletonRenderer::renderSkinGPU(const glm::mat4& viewProjMatrix,
    GLuint shaderProgram,
    const glm::vec3 cameraPos) {
    glUseProgram(shaderProgram);

    GLuint vpLoc = glGetUniformLocation(shaderProgram, "viewProj");
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint jointsLoc = glGetUniformLocation(shaderProgram, "jointMatrices");
    GLuint camPos = glGetUniformLocation(shaderProgram, "CameraPos");

    if (jointsLoc == -1) {
        std::cerr << "ERROR: jointMatrices uniform not found!" << std::endl;
        return;
    }

    glm::mat4 modelMatrix(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(vpLoc, 1, GL_FALSE, &viewProjMatrix[0][0]);
    glUniform3fv(camPos, 1, &cameraPos[0]);

    const auto& joints = skeleton->getJointData();
    const auto& bindingMats = skin->bindingMats;
    const int MAX_JOINTS = 150; 

    std::vector<glm::mat4> jointMatrices(MAX_JOINTS, glm::mat4(1.0f));

    for (int i = 0; i < std::min((int)joints.size(), MAX_JOINTS); ++i) {
        if (i >= bindingMats.size()) {
            std::cerr << "WARNING: Binding matrix missing for joint " << i
                << ", using identity" << std::endl;
            jointMatrices[i] = glm::mat4(1.0f);
            continue;
        }

        glm::mat4 inverseBind = glm::inverse(bindingMats[i]); 
        glm::mat4 worldMat = skeleton->getJointWorldMatrix(i);
        jointMatrices[i] = worldMat * inverseBind;
    }

    glUniformMatrix4fv(jointsLoc,
        MAX_JOINTS, 
        GL_FALSE,
        glm::value_ptr(jointMatrices[0]));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES,
        skin->triangles.size() * 3,
        GL_UNSIGNED_INT,
        0);
    glBindVertexArray(0);
}


void SkeletonRenderer::render(const glm::mat4& viewProjMatrix, GLuint shaderProgram, const glm::vec3 cameraPos) {
    if (!skeleton || !VAO) return;

    const auto& joints = skeleton->getJointData();

    // set uniforms for materials and lights
    material.SetUniforms(shaderProgram, "material");
    directLight.SetUniforms(shaderProgram, "dirLight");
    pointLight.SetUniforms(shaderProgram, "pointLight");

    // set the bool
    GLuint skinModeLoc = glGetUniformLocation(shaderProgram, "useGPUSkinning");
    glUniform1i(skinModeLoc, renderInGPU ? 1 : 0); 

    if (render_skin) {
        if(!renderInGPU)
            renderSkinCPU(viewProjMatrix, shaderProgram, cameraPos);
        else
            renderSkinGPU(viewProjMatrix, shaderProgram, cameraPos);
    }
    else {
        glUseProgram(shaderProgram);

        GLuint vpLoc = glGetUniformLocation(shaderProgram, "viewProj");
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint camPos = glGetUniformLocation(shaderProgram, "CameraPos");
        glUniformMatrix4fv(vpLoc, 1, GL_FALSE, &viewProjMatrix[0][0]);
        glUniform3fv(camPos, 1, &cameraPos[0]);

        switch (renderMode) {
        case SkeletonRenderMode::Fill:
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case SkeletonRenderMode::Wireframe:
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(2.0f);
            break;
        }

        glBindVertexArray(VAO);

        for (size_t i = 0; i < joints.size(); ++i) {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(joints[i]->worldMatrix));

            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT,
                (void*)(i * 36 * sizeof(GLuint)));
        }

        glBindVertexArray(0);
        glUseProgram(0);
    }


    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Reset to default mode
}


void SkeletonRenderer::Update() {
    if(!renderInGPU)
        updateSkinVerticesCPU();
}

void SkeletonRenderer::updateSkinVerticesCPU() {

    if (!render_skin) return;

    const auto& jointData = skeleton->getJointData();
    const auto& bindingMats = skin->bindingMats;

    std::vector<SkinVertex> deformedVertices = skin->vertices;

    for (size_t i = 0; i < skin->vertices.size(); ++i) {
        glm::vec3 skinnedPos(0.0f);
        glm::vec3 skinnedNormal(0.0f);

        const auto& originalVertex = skin->vertices[i];
        for (const auto& weight : originalVertex.weights) {
            if (weight.jointIndex >= jointData.size()) {
                std::cerr << "Invalid joint index: " << weight.jointIndex << std::endl;
                exit(-3);
            }

            glm::mat4 jointWorldMatrix = skeleton->getJointWorldMatrix(weight.jointIndex);
            glm::mat4 bindingMatrix = bindingMats[weight.jointIndex];
            glm::mat4 skinMatrix = jointWorldMatrix * glm::inverse(bindingMatrix);

            glm::vec4 transformedPos = skinMatrix * glm::vec4(originalVertex.position, 1.0f);
            skinnedPos += glm::vec3(transformedPos) * weight.weight;

            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(skinMatrix)));
            glm::vec3 transformedNormal = normalMatrix * originalVertex.normal;
            skinnedNormal += transformedNormal * weight.weight;
        }

        deformedVertices[i].position = skinnedPos;
        deformedVertices[i].normal = glm::normalize(skinnedNormal);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, deformedVertices.size() * sizeof(SkinVertex), deformedVertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
