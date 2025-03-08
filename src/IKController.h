#pragma once

#include "core.h"
#include "Skeleton.h"
#include <set>
#include <deque>
#include "Camera.h"
#include "IKSolver.h"

class IKSolver;

// Control points of IK
class IKControlPoint {
public:
    glm::vec3 position;  // Control point position
    glm::vec3 color;     // Comtrol Point Color
    std::shared_ptr<Joint> boundJoint;   // Binding joint

    IKControlPoint(glm::vec3 pos, glm::vec3 col = glm::vec3(1.0f, 0.0f, 0.0f), std::shared_ptr<Joint> joint = nullptr)
        : position(pos), color(col), boundJoint(joint) {}

    void setPosition(const glm::vec3& newPos) {
        position = newPos;
    }

    void bindToJoint(std::shared_ptr<Joint> joint) {
        boundJoint = joint;
    }
};

// Render Target points
class IKControlPointRenderer {
private:
    GLuint shaderProgram; // Own shader program
    GLuint VAO, VBO;      // Vertex Array Object & Vertex Buffer Object

    void setupShaders();  // Load and compile shaders
    void setupBuffers();  // Initialize VAO and VBO

public:
    IKControlPointRenderer();
    ~IKControlPointRenderer();

    void initialize();  // Initializes the renderer
    void render(const std::vector<IKControlPoint>& points, const glm::mat4& viewProjMatrix, const glm::vec3& camPos);

    void cleanup();
};

// Main Controller class
class IKController {
public:
    const glm::vec3 POINT_COLOR = { 1.f, 0.f, 0.f };

    std::vector<IKControlPoint> controlPoints;
    Skeleton* skeleton;                             // holds for update world matrix
    std::vector< std::shared_ptr<Joint>> joints;    // All joints

    std::unique_ptr<IKControlPointRenderer>  renderer;

    std::unique_ptr<IKSolver> solver;

	IKController();
	~IKController();

    void initialize(Skeleton* newSkeleton, const std::set<std::string> jointName);
    void initializeAuto(Skeleton* skel);
    void update();
    void draw(const glm::mat4& viewProjMat, const Camera& cam);

    void cleanup();
private:
};
