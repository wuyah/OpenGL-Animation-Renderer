#include "ImGuiController.h"
#include "SkeletonManager.h"
#include "ClothManager.h"
#include <iostream>

// 静态成员初始化
std::unique_ptr<ImGuiController> ImGuiController::instance;
std::once_flag ImGuiController::initFlag;

ImGuiController& ImGuiController::getInstance() {
    std::call_once(initFlag, []() {
        instance.reset(new ImGuiController());
        });
    return *instance;
}

bool ImGuiController::initialize(GLFWwindow* window, const char* glsl_version) {
    this->window = window;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();



    // Setup ImGui bindings
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cerr << "Failed to initialize ImGui GLFW backend!" << std::endl;
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        std::cerr << "Failed to initialize ImGui OpenGL backend!" << std::endl;
        return false;
    }

    // Setup ImGui style
    ImGui::StyleColorsDark();
    initialized = true;
    return true;
}

bool ImGuiController::bindSkeletonManager(SkeletonManager* skeletonManager) {
    this->skeletonManager = skeletonManager;
    return skeletonManager == nullptr ? false : true;
}

bool ImGuiController::bindClothManager(ClothManager* skeletonManager) {
    this->clothManager = skeletonManager;
    return clothManager == nullptr ? false : true;
}


void ImGuiController::beginFrame() {
    if (!initialized) {
        std::cerr << "ImGuiController is not initialized! Call initialize() first." << std::endl;
        return;
    }
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrameTime;
    lastFrameTime = currentFrame;
    fps = 1.0f / deltaTime; 

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}


// Main UI logic
void ImGuiController::render() {
    if (!initialized) {
        std::cerr << "ImGuiController is not initialized! Call initialize() first." << std::endl;
        return;
    }
    ImGui::Begin("Editor");
    if (ImGui::CollapsingHeader("Performance")) {
        renderFPS();
    }
    if (ImGui::CollapsingHeader("Skeleton Renderer Settings")) {
        renderSkeletonRendererUI();
    }
    if (ImGui::CollapsingHeader("Cloth Manager Settings")) {
        renderClothRendererUI();
    }
    if (ImGui::CollapsingHeader("Cloth Settings")) {
        renderClothSimulationUI();
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiController::shutdown() {
    if (!initialized) return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    initialized = false;
}

ImGuiController::~ImGuiController() {
    shutdown();
}

void ImGuiController::renderJointRecursive(const std::shared_ptr<Joint>& joint, int level) {
    if (!joint) return;

    // Push a unique ID
    ImGui::PushID(joint.get());

    // Create tree node
    if (ImGui::TreeNodeEx("##node", ImGuiTreeNodeFlags_None,
        "%s", joint->name.empty() ? "Unnamed Joint" : joint->name.c_str()))
    {
        // Edit joint properties
        ImGui::Text("Pose (Euler Angles):");
        if (skeletonManager->enableSkeletonEdit) {
            ImGui::DragFloat("Pose X", &joint->pose.x, 0.05f, joint->rotXLimit.x, joint->rotXLimit.y);
            ImGui::DragFloat("Pose Y", &joint->pose.y, 0.05f, joint->rotYLimit.x, joint->rotYLimit.y);
            ImGui::DragFloat("Pose Z", &joint->pose.z, 0.05f, joint->rotZLimit.x, joint->rotZLimit.y);

        }
        else {
            ImGui::Text("Pose: X: %.3f", joint->pose.x);
            ImGui::Text("Y: %.3f", joint->pose.y);
            ImGui::Text("Z: %.3f", joint->pose.z);
        }

        ImGui::Text("Orig Pose: X: %.3f", joint->orginalPos.x);
        ImGui::SameLine();
        ImGui::Text("Y: %.3f", joint->orginalPos.y);
        ImGui::SameLine();
        ImGui::Text("Z: %.3f", joint->orginalPos.z);

        // Recursively render children
        for (const auto& child : joint->children) {
            renderJointRecursive(child, level + 1);
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
}


void ImGuiController::renderSkeletonUI() {
    if (!skeletonManager || !skeletonManager->getSkeleton()) {
        ImGui::Text("No skeleton loaded!");
        return;
    }

    ImGui::Text("Skeleton Editor");

    Skeleton* skeleton = skeletonManager->getSkeleton();

    // Skeleton-level properties
    glm::vec3 position = skeleton->getPosition();
    glm::quat rotation = skeleton->getRotation();

    if (ImGui::DragFloat3("Skeleton Position", &position.x, 0.1f)) {
        skeleton->setPosition(position);
    }

    if (ImGui::DragFloat4("Skeleton Rotation (Quaternion)", &rotation.x, 0.1f)) {
        skeleton->setRotation(rotation);
    }

    bool& playAnim = skeletonManager->playAnim; // Access the playAnim variable

    if (ImGui::Button(playAnim ? "Pause Animation" : "Play Animation" ) && 
        skeletonManager->haveclip()) {
        playAnim = !playAnim; // Toggle play state
    }

    ImGui::SameLine();
    if ( skeletonManager->haveclip() ) {
        ImGui::Text("Status: %s", playAnim ? "Playing" : "Paused");
    }
    else {
        ImGui::Text("No animation clip loaded");
    }

    static char filename[128] = "skeleton_output.skel"; // Default file name
    ImGui::InputText("Skeleton Filename", filename, IM_ARRAYSIZE(filename));

    if (ImGui::Button("Save Skeleton")) {
        if (skeletonManager) {
            SkeletonParser parser;
            if (parser.writeSkeletonFile(*skeletonManager->getSkeleton() , filename)) {
                std::cout << "Skeleton saved successfully to " << filename << "\n";
            }
            else {
                std::cerr << "Failed to save skeleton to " << filename << "\n";
            }
        }
    }


    ImGui::Separator();
    ImGui::Text("Joint Hierarchy:");

    renderJointRecursive(skeleton->getRoot(), 0);
}

void ImGuiController::renderFPS() {
    //ImGui::Text("Performance");
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Frame Time: %.3f ms", deltaTime * 1000.0f);
}
void ImGuiController::renderSkeletonRendererUI() {
    if (!skeletonManager) {
        //std::cerr << "SkeletonManager not bound to ImGuiController!" << std::endl;
        return;
    }

    // Render Material
    SkeletonRenderer* renderer = skeletonManager->getRenderer();
    if (renderer) {
        Material* material = renderer->getMaterial();

        // Ambient color
        ImGui::ColorEdit3("Ambient", &material->ambient[0]);

        // Diffuse color
        ImGui::ColorEdit3("Diffuse", &material->diffuse[0]);

        // Specular color
        ImGui::ColorEdit3("Specular", &material->specular[0]);

        ImGui::DragFloat("Shininess", &material->shininess, 0.1f, 1.0f, 100.0f);

        // Directional Light
        ImGui::Separator();
        ImGui::Text("Directional Light");
        ImGui::ColorEdit3("Dir Light Color", &(renderer->getDirectLight()->color[0]));
        ImGui::DragFloat3("Dir Light Direction", &(renderer->getDirectLight()->direction[0]), 0.02f, -1.0f, 1.0f);
        ImGui::DragFloat("Dir Light Intensity", &(renderer->getDirectLight()->intensity), 0.02f);

        // Point Light
        ImGui::Separator();
        ImGui::Text("Point Light");
        ImGui::ColorEdit3("Point Light Color", &(renderer->getPointLight()->color[0]));
        ImGui::DragFloat3("Point Light Position", &(renderer->getPointLight()->position[0]), 0.02f);
        ImGui::DragFloat("Point Light Intensity", &(renderer->getPointLight()->intensity), 0.02f);

    }
    else {
        ImGui::Text("No Skeleton Renderer found!");
    }

    // Render skeleton editor
    renderSkeletonUI();
    renderIKUI();
    // renderClothRendererUI();
}

void ImGuiController::renderIKUI() {
    if (!skeletonManager) return;
    IKController* ik = skeletonManager->getIK();
    ImGui::Text("IK Control Points!");

    for (auto& p : ik->controlPoints) {
        ImGui::Text(p.boundJoint->name.c_str());    // name
        ImGui::DragFloat3("Position", & p.position[0], 0.01);
    }

}


void ImGuiController::renderClothRendererUI() {
    // Check that a ClothManager (and thus a ClothRenderer) is bound.
    if (!clothManager) {
        ImGui::Text("No Cloth Manager bound!");
        return;
    }

    // Assume ClothManager provides access to its ClothRenderer via getRenderer().
    ClothRenderer* renderer = clothManager->getRenderer();
    if (renderer) {
        Material* material = renderer->getMaterial();

        ImGui::Text("Cloth Renderer Settings");

        if (ImGui::CollapsingHeader("Material")) {
            ImGui::ColorEdit3("Ambient", &material->ambient[0]);
            ImGui::ColorEdit3("Diffuse", &material->diffuse[0]);
            ImGui::ColorEdit3("Specular", &material->specular[0]);
            ImGui::DragFloat("Shininess", &material->shininess, 0.1f, 1.0f, 100.0f);
        }

        if (ImGui::CollapsingHeader("Directional Light")) {
            DirectionalLight* dirLight = renderer->getDirectLight();
            ImGui::ColorEdit3("Dir Light Color", &(dirLight->color[0]));
            ImGui::DragFloat3("Dir Light Direction", &(dirLight->direction[0]), 0.02f, -1.0f, 1.0f);
            ImGui::DragFloat("Dir Light Intensity", &(dirLight->intensity), 0.02f);
        }

        if (ImGui::CollapsingHeader("Point Light")) {
            PointLight* pointLight = renderer->getPointLight();
            ImGui::ColorEdit3("Point Light Color", &(pointLight->color[0]));
            ImGui::DragFloat3("Point Light Position", &(pointLight->position[0]), 0.02f);
            ImGui::DragFloat("Point Light Intensity", &(pointLight->intensity), 0.02f);
        }
    }
    else {
        ImGui::Text("No Cloth Renderer found!");
    }
}

void ImGuiController::renderClothSimulationUI() {
    // Check if the clothManager is bound.
    if (!clothManager) {
        ImGui::Text("No Cloth Manager bound!");
        return;
    }

    // Get a reference to the cloth simulation.
    Cloth* cloth = clothManager->getCloth();

    ImGui::Text("Cloth Simulation Settings");

    if (ImGui::CollapsingHeader("Simulation Parameters")) {
        ImGui::DragFloat("Damper", &clothManager->damper, 1.f, 0.0f, 0.f, "%.3f");

        ImGui::DragFloat("Stifness", &clothManager->stiffness, 1.f, 0.0f, 0.f, "%.3f");

        // Gravity vector.
        ImGui::DragFloat3("Gravity", &cloth->gravity[0], 0.1f, -50.0f, 50.0f, "%.2f");
        // Wind vector.
        ImGui::DragFloat3("Wind", &cloth->wind[0], 0.1f, -50.0f, 50.0f, "%.2f");
        // Ground level.
        ImGui::DragFloat("Ground Level", &clothManager->groundLevel, 0.1f, -50.0f, 50.0f, "%.2f");
        // Air density (rho).
        ImGui::DragFloat("Air Density (rho)", &cloth->rho, 0.01f, 0.0f, 10.0f, "%.3f");
        // Drag coefficient (Cd).
        ImGui::DragFloat("Drag Coefficient (Cd)", &cloth->Cd, 0.01f, 0.0f, 10.0f, "%.3f");

        // Fixed Particle
        for (int i = 0; i < cloth->fixedParticles.size(); i++) {
            ImGui::DragFloat3( ("Fixed" + std::to_string(i)).c_str(), &cloth->fixedParticles[i]->position[0], 0.1f, 0.f, 0.f, "%.2f");
        }

    }
}
