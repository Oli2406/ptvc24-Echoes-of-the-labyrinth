/*#include "PhysXInitializer.h"
#include <iostream>
#include <PxPhysicsAPI.h>

using namespace physx;

PxDefaultErrorCallback errorCallback;
PxDefaultAllocator allocatorCallback;
PxDefaultCpuDispatcher* gDispatcher = nullptr;
PxMaterial* gMaterial = nullptr;

PxPhysics* PhysXInitializer::initializePhysX() {
    PxFoundation* foundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocatorCallback, errorCallback);
    if (!foundation) {
        std::cerr << "Failed to create PhysX foundation!" << std::endl;
        return nullptr;
    }
    
    PxPvd* pvd = PxCreatePvd(*foundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    if (transport == NULL)
        return NULL;
    pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    PxPhysics* physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), true);
    if (!physics) {
        std::cerr << "Failed to create PhysX physics!" << std::endl;
        return nullptr;
    }
    std::cout << "PhysX setup was successful!" << std::endl;
    return physics;
}

PxScene* PhysXInitializer::createPhysXScene(PxPhysics* physics) {
    PxSceneDesc sceneDesc(physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    gDispatcher = PxDefaultCpuDispatcherCreate(0);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    PxScene* scene = physics->createScene(sceneDesc);
    scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
    scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 2.0f);
    if (!scene) {
        std::cerr << "Failed to create PhysX scene!" << std::endl;
        return nullptr;
    }
    PxPvdSceneClient* pvdClient = scene->getScenePvdClient();
    if (pvdClient) {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }
    else {
        std::cout << "nope" << std::endl;
    }
    gMaterial = physics->createMaterial(0.5f, 0.5f, 0.6f);
    PxRigidStatic* groundPlane = PxCreatePlane(*physics, PxPlane(0, 1, 0, 0), *gMaterial);
    scene->addActor(*groundPlane);
    PxShape* shapeBuffer[1];
    groundPlane->getShapes(shapeBuffer, 1);
    shapeBuffer[0]->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
    shapeBuffer[0]->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
    std::cout << "Scene setup was successful!" << std::endl;
    return scene;
}*/