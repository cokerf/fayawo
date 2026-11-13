// Bullet is one of the original native dependencies of Fayawo's CARDYNAMICS.
// This tests the *same Bullet C++ library* compiled via the Emscripten Bullet port.
// It is not yet the game's full CARDYNAMICS chassis/wheel integration.
#include <btBulletDynamicsCommon.h>
#include <algorithm>
#include <cmath>

extern "C" {

double fayawo_bullet_drop_height(double seconds) {
    btDefaultCollisionConfiguration collisionConfig;
    btCollisionDispatcher dispatcher(&collisionConfig);
    btDbvtBroadphase broadphase;
    btSequentialImpulseConstraintSolver solver;
    btDiscreteDynamicsWorld world(&dispatcher, &broadphase, &solver, &collisionConfig);
    world.setGravity(btVector3(0, -9.81f, 0));

    btBoxShape groundShape(btVector3(50.0f, 0.5f, 50.0f));
    btDefaultMotionState groundMotion(btTransform(btQuaternion(0,0,0,1),btVector3(0,-0.5f,0)));
    btRigidBody::btRigidBodyConstructionInfo groundInfo(0.0f,&groundMotion,&groundShape,btVector3(0,0,0));
    btRigidBody ground(groundInfo);
    world.addRigidBody(&ground);

    btBoxShape chassisShape(btVector3(0.6f,0.3f,1.2f));
    btTransform startingTransform;
    startingTransform.setIdentity();
    startingTransform.setOrigin(btVector3(0, 5.0f, 0));
    btDefaultMotionState chassisMotion(startingTransform);
    btVector3 inertia(0,0,0);
    chassisShape.calculateLocalInertia(950.0f,inertia);
    btRigidBody::btRigidBodyConstructionInfo chassisInfo(950.0f,&chassisMotion,&chassisShape,inertia);
    btRigidBody chassis(chassisInfo);
    chassis.setRestitution(0.0f);
    world.addRigidBody(&chassis);

    const int steps=static_cast<int>(std::clamp(seconds,0.0,10.0)*120.0);
    const btScalar delta=btScalar(1.0/120.0);
    for(int i=0;i<steps;i++)world.stepSimulation(delta,1,delta);

    const btScalar height=chassis.getWorldTransform().getOrigin().getY();
    world.removeRigidBody(&chassis);
    world.removeRigidBody(&ground);
    return static_cast<double>(height);
}

int fayawo_bullet_probe_version() { return 1; }

}
