#include "Window.h"


// Window Properties
int Window::width;
int Window::height;
const char* Window::windowTitle = "Model Environment";

// Objects to render
std::unique_ptr<Cube> Window::cube = nullptr;
std::unique_ptr<SkeletonManager> Window::skeletonManager = nullptr;
std::unique_ptr<ClothManager> Window::clothManager = nullptr;


// Camera Properties
Camera* Cam;

// Interaction Variables
bool LeftDown, RightDown;
int MouseX, MouseY;

// The shader program id
GLuint Window::shaderProgram;

// Constructors and desctructors
bool Window::initializeProgram() {
    // Create a shader program with a vertex shader and a fragment shader.
    shaderProgram = LoadShaders("shaders/shader.vert", "shaders/shader.frag");

    // Check the shader program.
    if (!shaderProgram) {
        std::cerr << "Failed to initialize shader program" << std::endl;
        return false;
    }

    return true;
}

bool Window::initializeObjects() {
    // Create a cube

   
    return true;
}

bool Window::initializeImGui(GLFWwindow* window) {
    if (!ImGuiController::getInstance().initialize(window, "#version 330")) {
        std::cerr << "Failed to initialize ImGuiController!" << std::endl;
        return false;
    }
    return true;
}

bool Window::initializeSkeletonSystem(std::string skel_file) {
    skeletonManager = std::make_unique<SkeletonManager>();
    if (!skeletonManager.get()->initializeSkeleton(skel_file)) {
        return false;
    }
    if (skeletonManager) {
        ImGuiController::getInstance().bindSkeletonManager(skeletonManager.get());
        std::cout << "Bind skeletonManager to Imgui success!" << std::endl;
    }
    skeletonManager.get()->bindCamera(Cam);
    skeletonManager->initializeRenderer();

    return true;
}

bool Window::initializeSkeletonSystem(std::string skel_file, std::string skin_file, std::string anim_file) {
    skeletonManager = std::make_unique<SkeletonManager>();
    if (!skeletonManager.get()->initializeSkeleton(skel_file)) {
        return false;
    }
    if (!skin_file.empty()) {
        if (!skeletonManager->initializeSkin(skin_file)) {
            return false;
        }
    }
    else {
        std::cout << "Not providing skin file, skip skin file loading." << std::endl;
    }

    if (!anim_file.empty()) {
        skeletonManager->initializeAnim(anim_file);

    }


    if (skeletonManager) {
        ImGuiController::getInstance().bindSkeletonManager(skeletonManager.get());
        std::cout << "Bind skeletonManager to Imgui success!" << std::endl;
    }
    skeletonManager.get()->bindCamera(Cam);
    skeletonManager->initializeRenderer();
    return true;
}

bool Window::initializeClothSystem() {
    clothManager = std::make_unique<ClothManager>();
    if (!clothManager->initializeCloth()) {
        std::cerr << "Failed to initialize ClothManager!" << std::endl;
        return false;
    }
    if (clothManager) {
        if( ImGuiController::getInstance().bindClothManager(clothManager.get()) ) 
            std::cout << "Bind clothManager to Imgui success!" << std::endl;
        clothManager->bindCamera(Cam);
        std::cout << "ClothManager initialized successfully." << std::endl;
    }
    return true;
}

void Window::cleanUp() {
    // Deallcoate the objects.
    if(skeletonManager)
        skeletonManager->cleanUp();

    // Delete the shader program.
    glDeleteProgram(shaderProgram);

}

// for the Window
GLFWwindow* Window::createWindow(int width, int height) {
    // Initialize GLFW.
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return NULL;
    }

    // 4x antialiasing.
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create the GLFW window.
    GLFWwindow* window = glfwCreateWindow(width, height, windowTitle, NULL, NULL);

    // Check if the window could not be created.
    if (!window) {
        std::cerr << "Failed to open GLFW window." << std::endl;
        glfwTerminate();
        return NULL;
    }

    // Make the context of the window.
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewInit();

    // Set swap interval to 1.
    glfwSwapInterval(0);

    // set up the camera
    Cam = new Camera();
    Cam->SetAspect(float(width) / float(height));

    // initialize the interaction variables
    LeftDown = RightDown = false;
    MouseX = MouseY = 0;


    // Call the resize callback to make sure things get drawn immediately.
    Window::resizeCallback(window, width, height);
    Window::initializeImGui(window);

    return window;
}

void Window::resizeCallback(GLFWwindow* window, int width, int height) {
    Window::width = width;
    Window::height = height;
    // Set the viewport size.
    glViewport(0, 0, width, height);

    Cam->SetAspect(float(width) / float(height));
}

// update and draw functions
void Window::idleCallback() {
    // Perform any updates as necessary.
    Cam->Update();

    if (skeletonManager) {
        skeletonManager->Update();
    }
}

void Window::displayCallback(GLFWwindow* window) {
    // Clear the color and depth buffers.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGuiController::getInstance().beginFrame();
    //ImGuiController::getInstance().renderSkeletonRendererUI();

    // Render the object.
    //cube->draw(Cam->GetViewProjectMtx(), Window::shaderProgram);
    if (skeletonManager) {
        skeletonManager.get()->draw(Cam->GetViewProjectMtx(), Window::shaderProgram);
    }

    if (clothManager) {
        clothManager->render(Cam->GetViewProjectMtx(), Window::shaderProgram);
    }

    ImGuiController::getInstance().render();

    // Swap buffers.
    glfwSwapBuffers(window);
    // Gets events, including input such as keyboard and mouse or window resizing.
    glfwPollEvents();

}

// Fixed update for physical engine;
void Window::fixedCallback(float dt) {
    if (clothManager) {
        clothManager->Update(dt); // Now Update() accepts dt
    }
}


// helper to reset the camera
void Window::resetCamera() {
    Cam->Reset();
    Cam->SetAspect(float(Window::width) / float(Window::height));
}

// callbacks - for Interaction
void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    /*
     * TODO: Modify below to add your key callbacks.
     */
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard) {
        return; // don't process game hotkeys
    }


    // Check for a key press.
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                // Close the window. This causes the program to also terminate.
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;

            case GLFW_KEY_R:
                resetCamera();
                break;

            default:
                break;
        }
    }
}

void Window::mouse_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        LeftDown = (action == GLFW_PRESS);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        RightDown = (action == GLFW_PRESS);
    }
}

void Window::cursor_callback(GLFWwindow* window, double currX, double currY) {
    ImGui_ImplGlfw_CursorPosCallback(window, currX, currY);
    if (ImGui::GetIO().WantCaptureMouse) {
        return; // Early exit, do not process camera logic
    }

    int maxDelta = 100;
    int dx = glm::clamp((int)currX - MouseX, -maxDelta, maxDelta);
    int dy = glm::clamp(-((int)currY - MouseY), -maxDelta, maxDelta);

    MouseX = (int)currX;
    MouseY = (int)currY;

    // Move camera
    // NOTE: this should really be part of Camera::Update()
    if (LeftDown) {
        const float rate = 1.f;
        Cam->SetAzimuth(Cam->GetAzimuth() + dx * rate);
        Cam->SetIncline(glm::clamp(Cam->GetIncline() + dy * rate, -90.0f, 90.0f));
    }
    if (RightDown) {
        const float rate = 0.01f;
        float dist = glm::clamp(Cam->GetDistance() * (1.0f - dx * rate), 0.01f, 1000.0f);
        Cam->SetDistance(dist);
    }
}