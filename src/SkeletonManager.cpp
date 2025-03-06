#include "SkeletonManager.h"
#include <set>


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
    skin = std::make_shared<Skin>();

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
    clip = std::make_unique<AnimationClip>();
    // Allocate a new Skin object
    if (!clip->Load(filePath.c_str())) {
        std::cerr << "Failed to load animation clip" << std::endl;
        return false;
    }

    std::cout << "Anim clip file loaded successfully!" << std::endl;

    lastTime = glfwGetTime();
    return true;
}

bool SkeletonManager::initializeRenderer() {
    renderer.initialize(skeleton, skin);
    return true;
}


void SkeletonManager::initilizeIK() {
    std::set<std::string> initialPoints = { "j_5" };

    ik.initialize(&skeleton, initialPoints);
}

bool SkeletonManager::initialize(const std::string& skeletonFileName, const std::string& skinFileName, const std::string& animFileName) {
    if (!initializeSkeleton(skeletonFileName)) {
        return false;
    }
    if (!skinFileName.empty()) {
        if (!initializeSkin(skinFileName)) {
            return false;
        }
    }
    else {
        std::cout << "Not providing skin file, skip skin file loading." << std::endl;
    }

    if (!animFileName.empty()) {
        initializeAnim(animFileName);
    }

    initilizeIK();
    
    return true;
}


void SkeletonManager::storeCurrentSkeleton(const std::string& skelStoreFileName, const std::string& filename ) {
    parser.writeSkeletonFile(skeleton, filename);
}


void SkeletonManager::Update() {
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastTime; // Compute time difference
    lastTime = currentTime;

    if (clip && playAnim)
        currentAnimTime += deltaTime;
    if (clip &&  currentAnimTime > clip->rangeEnd) {
        float period = clip->rangeEnd - clip->rangeStart;
        if (period > 0.0f) {
            // Wrap the animation time within the clip's range.
            currentAnimTime = clip->rangeStart + std::fmod(currentAnimTime - clip->rangeStart, period);
        }
        else {
            currentAnimTime = clip->rangeStart;
        }
    }

    // Evaluate the animation clip to update the skeleton's joint poses.
    // We use the non-const accessor getJointList() we added to Skeleton.
    if(clip && playAnim)
        clip->Evaluate(currentAnimTime, skeleton.getJointList());

    // Update the skeleton's transformation matrices.
    skeleton.update();

    // Update the renderer if needed.
    renderer.Update();
}

void SkeletonManager::draw(const glm::mat4& viewProjMatrix, GLuint shaderProgram) {
    skeleton.update();
    renderer.render(viewProjMatrix, shaderProgram, camera->GetWorldPos() );
    ik.draw(viewProjMatrix, *camera);
}

void SkeletonManager::cleanUp() {
    renderer.cleanup();
    ik.cleanup();
}