#include "IKController.h"
#include "Shader.h"

//-------------Renderer Class-------------
IKControlPointRenderer::IKControlPointRenderer()
    : shaderProgram(0), VAO(0), VBO(0) {}

IKControlPointRenderer::~IKControlPointRenderer() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
}

void IKControlPointRenderer::initialize() {
    setupShaders();
    setupBuffers();
}

void IKControlPointRenderer::setupShaders() {
    shaderProgram = LoadShaders("shaders/point.vert", "shaders/point.frag");
    if (!shaderProgram) {
        std::cerr << "Failed to load IKControlPoint shaders!" << std::endl;
        return;
    }
}

void IKControlPointRenderer::setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // We will update the buffer dynamically based on IKController data
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW); // initially set to 16

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void IKControlPointRenderer::render(const std::vector<IKControlPoint> &points, const glm::mat4& viewProjMatrix, const glm::vec3& camPos) {
    //if (ikController.getControlPoints().empty()) return;
    if (points.size() == 0) return;
    glEnable(GL_PROGRAM_POINT_SIZE);

    glUseProgram(shaderProgram);

    GLuint vpLoc = glGetUniformLocation(shaderProgram, "viewProj");
    GLuint camPosLoc = glGetUniformLocation(shaderProgram, "cameraPos");
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");


    glUniformMatrix4fv(vpLoc, 1, GL_FALSE, &(viewProjMatrix[0][0]));
    glUniform3fv(camPosLoc, 1, &camPos[0]);

    std::vector<glm::vec3> positions;
    std::vector<glm::mat4> modelMatrices;

    for (const auto& p : points) {
        positions.push_back(p.position);
        modelMatrices.push_back(p.boundJoint->worldMatrix);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());

    glBindVertexArray(VAO);

    // **逐个绘制点，并绑定对应的 modelMatrix**
    for (size_t i = 0; i < points.size(); ++i) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelMatrices[i][0][0]); // 绑定每个点的 modelMatrix
        glDrawArrays(GL_POINTS, i, 1);  // 逐个绘制点
    }

    glBindVertexArray(0);
}

void IKControlPointRenderer::cleanup() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    VBO = 0;
    VAO = 0;
}

//-------------Controller Class-------------
IKController::IKController() : skeleton(nullptr), renderer(nullptr) {

}

IKController::~IKController() {

}

void IKController::initialize(Skeleton* newSkeleton, const std::set<std::string> jointNames) {
    this->skeleton = newSkeleton;

    controlPoints.clear();

    const auto& joints = skeleton->getJointData();
    for (auto joint : joints) {
        if (jointNames.find(joint->name) != jointNames.end()) {
            glm::vec4 localPos4 = glm::vec4(joint->pose, 1.0f);

            IKControlPoint p( glm::vec3( joint->worldMatrix * localPos4), POINT_COLOR, joint);
            controlPoints.push_back(p);
        }
    }

    renderer = std::make_unique<IKControlPointRenderer>();
    renderer->initialize();
}

void  IKController::draw(const glm::mat4& viewProjMat, const Camera& cam) {
    this->renderer->render(this->controlPoints, viewProjMat, cam.worldPos );
}

void IKController::Update() {

}

void IKController::cleanup() {
    controlPoints.clear();

    if (renderer) {
        renderer->cleanup();  
        renderer.reset();     
    }
    skeleton = nullptr;
}
