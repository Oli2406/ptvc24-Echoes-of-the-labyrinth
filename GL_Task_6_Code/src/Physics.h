#pragma once

#include <PxPhysicsAPI.h>

class Physics {
private:
    physx::PxDefaultAllocator mDefaultAllocatorCallback;
    physx::PxDefaultErrorCallback mDefaultErrorCallback;
    physx::PxDefaultCpuDispatcher* mDispatcher = nullptr;
    physx::PxTolerancesScale mToleranceScale;
    physx::PxFoundation* mFoundation = nullptr;
    physx::PxPhysics* mPhysics = nullptr;
    physx::PxScene* mScene = nullptr;
    physx::PxMaterial* mMaterial = nullptr;
    physx::PxPvd* mPvd = nullptr;

public:
    Physics() {
        mFoundation = PxCreateFoundation(0x05030000, mDefaultAllocatorCallback, mDefaultErrorCallback);
        if (!mFoundation)
            throw("PxCreateFoundation failed!");

        mPvd = physx::PxCreatePvd(*mFoundation);
        physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
        mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

        mToleranceScale.length = 100; // typical length of an object
        mToleranceScale.speed = 981;  // typical speed of an object, gravity*1s is a reasonable choice
        mPhysics = PxCreatePhysics(0x05030000, *mFoundation, mToleranceScale, true, mPvd);

        physx::PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
        sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
        mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
        sceneDesc.cpuDispatcher = mDispatcher;
        sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
        mScene = mPhysics->createScene(sceneDesc);

        physx::PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
        if (pvdClient) {
            pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
            pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
            pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
        }

        mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
    }

    ~Physics() {
        if (mScene) {
            mScene->release();
            mScene = nullptr;
        }
        if (mDispatcher) {
            mDispatcher->release();
            mDispatcher = nullptr;
        }
        if (mPhysics) {
            mPhysics->release();
            mPhysics = nullptr;
        }
        if (mFoundation) {
            mFoundation->release();
            mFoundation = nullptr;
        }
        if (mPvd) {
            mPvd->release();
            mPvd = nullptr;
        }
    }

    void addGroundPlane() {
        physx::PxRigidStatic* groundPlane = PxCreatePlane(*mPhysics, physx::PxPlane(0, 1, 0, 50), *mMaterial);
        mScene->addActor(*groundPlane);
    }

    void addDynamicBoxes(float halfExtent, physx::PxU32 size) {
        physx::PxShape* shape = mPhysics->createShape(physx::PxBoxGeometry(halfExtent, halfExtent, halfExtent), *mMaterial);
        physx::PxTransform t(physx::PxVec3(0));
        for (physx::PxU32 i = 0; i < size; i++) {
            for (physx::PxU32 j = 0; j < size - i; j++) {
                physx::PxTransform localTm(physx::PxVec3(physx::PxReal(j * 2) - physx::PxReal(size - i), physx::PxReal(i * 2 + 1), 0) * halfExtent);
                physx::PxRigidDynamic* body = mPhysics->createRigidDynamic(t.transform(localTm));
                body->attachShape(*shape);
                physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
                mScene->addActor(*body);
            }
        }
        shape->release();
    }

    void simulate(float timeStep = 1.0f / 60.0f) {
        mScene->simulate(timeStep);
        mScene->fetchResults(true);
    }
};