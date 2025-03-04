#include "SkeletonManager.h"

bool SkeletonManager::initializeSkeleton(const std::string& fileName) {
    // Parse the .skel file
    std::string filePath = resourcePath + fileName;
    if (!parser.parseSkeletonFile(filePath)) {
        std::cerr << "Failed to parse skeleton file: " << filePath << std::endl;
        return false;
    }
    std::cout << "Skeleton file loaded!" << std::endl;

    skeleton = parser.getSkeleton();
    skeleton.buildJointList();
    //renderer.initialize(skeleton);

    return true;
}

bool SkeletonManager::initializeSkin(const std::string& skinFileName) {
    std::string filePath = resourcePath + skinFileName;
    skin = std::make_unique<Skin>();

    // Allocate a new Skin object
    skin = std::make_unique<Skin>();

    if (!skin->loadFromFile(filePath)) {
        std::cerr << "Failed to load skin file: " << filePath << std::endl;
        skin.reset(); // Reset to nullptr if loading fails
        return false;
    }

    std::cout << "Skin file loaded successfully!" << std::endl;
    skin->computeNormals();
    return true;
}

bool SkeletonManager::initializeAnim(const std::string& animFileName) {
    std::string filePath = resourcePath + animFileName;

    // Allocate a new Skin object
    if (!clip.Load(filePath.c_str())) {
        std::cerr << "Failed to load animation clip" << std::endl;
        return false;
    }

    std::cout << "Anim clip file loaded successfully!" << std::endl;

    lastTime = glfwGetTime();
    return true;
}

bool SkeletonManager::initializeRenderer() {
    renderer.initialize(skeleton, skin.get());
    return true;
}


void SkeletonManager::Update() {
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastTime; // Compute time difference
    lastTime = currentTime;

    currentAnimTime += deltaTime;
    if (currentAnimTime > clip.rangeEnd) {
        float period = clip.rangeEnd - clip.rangeStart;
        if (period > 0.0f) {
            // Wrap the animation time within the clip's range.
            currentAnimTime = clip.rangeStart + std::fmod(currentAnimTime - clip.rangeStart, period);
        }
        else {
            currentAnimTime = clip.rangeStart;
        }
    }

    // Evaluate the animation clip to update the skeleton's joint poses.
    // We use the non-const accessor getJointList() we added to Skeleton.
    clip.Evaluate(currentAnimTime, skeleton.getJointList());

    // Update the skeleton's transformation matrices.
    skeleton.update();

    // Update the renderer if needed.
    renderer.Update();
}

void SkeletonManager::draw(const glm::mat4& viewProjMatrix, GLuint shaderProgram) {
    skeleton.update();
    renderer.render(viewProjMatrix, shaderProgram, camera->GetWorldPos() );
}
