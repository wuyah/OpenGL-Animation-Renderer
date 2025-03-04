#include "Window.h"
#include "core.h"
#include <iostream>

#define ENABLE_SKELETON_SYSTEM false

void error_callback(int error, const char* description) {
    // Print error.
    std::cerr << description << std::endl;
}

void setup_callbacks(GLFWwindow* window) {
    // Set the error callback.
    glfwSetErrorCallback(error_callback);
    // Set the window resize callback.
    glfwSetWindowSizeCallback(window, Window::resizeCallback);

    // Set the key callback.
    glfwSetKeyCallback(window, Window::keyCallback);

    // Set the mouse and cursor callbacks
    glfwSetMouseButtonCallback(window, Window::mouse_callback);
    glfwSetCursorPosCallback(window, Window::cursor_callback);
}

void setup_opengl_settings() {
    // Enable depth buffering.
    glEnable(GL_DEPTH_TEST);
    // Related to shaders and z value comparisons for the depth buffer.
    glDepthFunc(GL_LEQUAL);
    // Set polygon drawing mode to fill front and back of each polygon.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // Set clear color to black.
    glClearColor(0.0, 0.0, 0.0, 0.0);
}

void print_versions() {
    // Get info of GPU and supported OpenGL version.
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version supported: " << glGetString(GL_VERSION)
              << std::endl;

    // If the shading language symbol is defined.
#ifdef GL_SHADING_LANGUAGE_VERSION
    std::cout << "Supported GLSL version is: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
#endif
}

int main(int argc, char** argv) {
    // Create the GLFW window.
    GLFWwindow* window = Window::createWindow(1200, 1000);
    if (!window) exit(EXIT_FAILURE);

    if (ENABLE_SKELETON_SYSTEM) {
        std::string skel_filename, skin_filename;
        bool skel_provided = false;
        bool skin_provided = false;

        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == "-skel" && i + 1 < argc) {
                skel_filename = argv[i + 1];
                skel_provided = true;
                ++i;
            }
            if (std::string(argv[i]) == "-skin" && i + 1 < argc) {
                skin_filename = argv[i + 1];
                skin_provided = true;
                ++i;
            }

        }

        if (!skel_provided) {
            std::cout << "Skeleton file not provided, using test.skel." << std::endl;
            if (!Window::initializeSkeletonSystem("test.skel")) exit(EXIT_FAILURE);
        }
        else if (skin_provided) {
            std::cout << "Loading file: " + skel_filename << std::endl;
            if (!Window::initializeSkeletonSystem(skel_filename, skin_filename)) exit(EXIT_FAILURE);
        }
        else {
            std::cout << "Skin file: " << skin_filename << std::endl;
            if (!Window::initializeSkeletonSystem(skel_filename, skin_filename)) exit(EXIT_FAILURE);

        }

        std::cout << "Loaded file: " << skel_filename << std::endl;

    }


    if (!Window::initializeClothSystem()) {
        exit(EXIT_FAILURE);
    }

    std::cout << " Initialize Cloth System Success! " << std::endl;


    // Print OpenGL and GLSL versions.
    print_versions();
    // Setup callbacks.
    setup_callbacks(window);
    // Setup OpenGL settings.
    setup_opengl_settings();

    // Initialize the shader program; exit if initialization fails.
    if (!Window::initializeProgram()) exit(EXIT_FAILURE);
    // Initialize objects/pointers for rendering; exit if initialization fails.
    if (!Window::initializeObjects()) exit(EXIT_FAILURE);

    // Add fixed delta time for fixed update loop
    float fixedDeltaTime = 0.004f; // Fixed update timestep (~240 updates per second)
    float accumulator = 0.0f;
    float lastTime = glfwGetTime();

    // Loop while GLFW window should stay open.
    while (!glfwWindowShouldClose(window)) {

        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        accumulator += deltaTime;

        while (accumulator >= fixedDeltaTime) {
            Window::fixedCallback(fixedDeltaTime);
            accumulator -= fixedDeltaTime;
        }

        // Main render display callback. Rendering of objects is done here.
        Window::displayCallback(window);

        // Idle callback. Updating objects, etc. can be done here.
        Window::idleCallback();
    }

    Window::cleanUp();
    // Destroy the window.
    glfwDestroyWindow(window);
    // Terminate GLFW.
    glfwTerminate();

    exit(EXIT_SUCCESS);
}