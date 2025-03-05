#pragma once

#include <memory>
#include "Camera.h"
#include "Cube.h"
#include "Shader.h"
#include "core.h"
#include "../src/SkeletonManager.h"
#include <string>
#include "../src/ImGuiController.h"
#include "../src/ClothManager.h"

class Window {
public:
    // Window Properties
    static int width;
    static int height;
    static const char* windowTitle;

    // Objects to render
    static std::unique_ptr<Cube> cube; // Use smart pointer
    static std::unique_ptr<SkeletonManager> skeletonManager;
    static std::unique_ptr<ClothManager> clothManager;

    // ImGui
    //static std::unique_ptr<ImGuiController> ImGuiController::instance;
    //static std::once_flag ImGuiController::initFlag;


    // Shader Program
    static GLuint shaderProgram;

    // Act as Constructors and desctructors
    static bool initializeProgram();
    static bool initializeObjects();
    static bool initializeSkeletonSystem(std::string skel_file);
    static bool initializeSkeletonSystem(std::string skel_file, std::string skin_file, std::string anim_file);
    static bool initializeImGui(GLFWwindow*);
    static bool initializeClothSystem();

    static void cleanUp();

    // for the Window
    static GLFWwindow* createWindow(int width, int height);
    static void resizeCallback(GLFWwindow* window, int width, int height);

    // update and draw functions
    static void idleCallback();
    static void displayCallback(GLFWwindow*);

    // fixed update for physical engine
    static void fixedCallback(float dt);

    // helper to reset the camera
    static void resetCamera();

    // callbacks - for interaction
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_callback(GLFWwindow* window, double currX, double currY);
};