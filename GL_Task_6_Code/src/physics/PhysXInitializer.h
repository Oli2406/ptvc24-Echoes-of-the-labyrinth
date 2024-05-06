#pragma once

#include <PxPhysicsAPI.h>

using namespace physx;

class PhysXInitializer
{
public:
    static PxPhysics* initializePhysX();
    static PxScene* createPhysXScene(PxPhysics* physics);
};