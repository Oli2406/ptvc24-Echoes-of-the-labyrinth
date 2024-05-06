#include "PhysXInitializer.h"
#include <iostream>

static PxDefaultErrorCallback errorCallback;
static PxDefaultAllocator allocatorCallback;

PxPhysics* PhysXInitializer::initializePhysX() {
    PxFoundation* foundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocatorCallback,
        errorCallback);
    PxPhysics* physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), true);
    if (foundation && physics) {
        std::cout << "PhysX setup was successful!" << std::endl;
    }
    else {
        std::cout << "PhysX setup failed!" << std::endl;
    }
    return physics;
}

PxScene* PhysXInitializer::createPhysXScene(PxPhysics* physics) {
    PxSceneDesc sceneDesc(physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    PxScene* scene = physics->createScene(sceneDesc);
    if (scene) {
        std::cout << "scene setup successful!" << std:: endl;
    }
    else {
        std::cout << "scene setup failed!" << std::endl;
    }
    return scene;
}