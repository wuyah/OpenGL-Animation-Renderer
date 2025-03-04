#pragma once

#include <core.h>
#include <memory>
#include <mutex>
#include <imgui.h>
#include <backend/imgui_impl_glfw.h>
#include <backend/imgui_impl_opengl3.h>

class ClothManager;
class SkeletonManager; 
class Joint;

class ImGuiController {
public:
    static ImGuiController& getInstance();

    ImGuiController(const ImGuiController&) = delete;
    ImGuiController& operator=(const ImGuiController&) = delete;

    bool initialize(GLFWwindow* window, const char* glsl_version);
    void beginFrame();
    void render();
    void shutdown();

    bool bindSkeletonManager(SkeletonManager* skeletonManager);
    bool bindClothManager(ClothManager* clothManager);

    // Rendering functions
    void renderSkeletonRendererUI();
    void renderClothRendererUI();
    void renderClothSimulationUI();

    ImGuiController() = default;
    ~ImGuiController();
    static std::unique_ptr<ImGuiController> instance;
    static std::once_flag initFlag;
    void renderSkeletonUI();
    void renderFPS(); 

private:

    float lastFrameTime = 0.0f;  
    float deltaTime = 0.0f;    
    float fps = 0.0f;       

    ClothManager* clothManager = nullptr;
    GLFWwindow* window = nullptr;
    SkeletonManager* skeletonManager = nullptr; 
    bool initialized = false;
    void renderJointRecursive(const std::shared_ptr<Joint>& joint, int level);

};

