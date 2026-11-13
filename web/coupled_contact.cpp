// First integrated Fayawo vehicle-contact proof, not complete CARDYNAMICS.
// Original CONFIGFILE, CARSUSPENSION and CARTIRE algorithms operate inside
// the SAME Bullet rigid-body world. Real .car/.tire data drive the forces.
// Fayawo uses Z-up vehicle coordinates; Bullet probe uses Y-up.
#include <btBulletDynamicsCommon.h>
#include "configfile.h"
#include "carsuspension.h"
#include "cartire.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <sstream>
#include <cstdio>
#include <string>

namespace {
CONFIGFILE config, tireConfig;
CARTIRE tire;
bool carLoaded=false, tireLoaded=false;
double mass=1200, radius=.31, travel=.32;
std::array<btVector3,4> anchors;
std::array<CARSUSPENSION,4> springs;
std::unique_ptr<btDefaultCollisionConfiguration> collision;
std::unique_ptr<btCollisionDispatcher> dispatcher;
std::unique_ptr<btDbvtBroadphase> broadphase;
std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
std::unique_ptr<btDiscreteDynamicsWorld> world;
std::unique_ptr<btBoxShape> groundShape, chassisShape;
std::unique_ptr<btDefaultMotionState> groundMotion, chassisMotion;
std::unique_ptr<btRigidBody> ground, chassis;
double lastNormal=0, lastTraction=0;
int lastContacts=0;

double param(const std::string& name,double fallback){
    float value;
    return config.GetParam(name,value)?value:fallback;
}
void destroyWorld(){
    if(world){
        if(chassis)world->removeRigidBody(chassis.get());
        if(ground)world->removeRigidBody(ground.get());
    }
    chassis.reset();ground.reset();chassisMotion.reset();groundMotion.reset();
    chassisShape.reset();groundShape.reset();world.reset();solver.reset();
    broadphase.reset();dispatcher.reset();collision.reset();
}
bool loadTire(){
    float value;
    CARTIRE candidate;
    for(int i=0;i<=10;i++){
        if(!tireConfig.GetParam("params.b"+std::to_string(i),value))return false;
        candidate.longitudinal[i]=value;
        if(!tireConfig.GetParam("params.a"+std::to_string(i),value))return false;
        candidate.lateral[i]=value;
    }
    const char* lateralExtras[]={"a111","a112","a12","a13"};
    for(int i=0;i<4;i++){
        if(!tireConfig.GetParam(std::string("params.")+lateralExtras[i],value))return false;
        candidate.lateral[i+11]=value;
    }
    for(int i=0;i<18;i++){
        if(!tireConfig.GetParam("params.c"+std::to_string(i),value))return false;
        candidate.aligning[i]=value;
    }
    tire=candidate;
    return true;
}
double vehicleMass(){
    double m=param("engine.mass",0)+param("driver.mass",0);
    m+=param("fuel-tank.volume",0)*param("fuel-tank.fuel-density",0);
    for(int i=0;i<32;i++){
        char key[48];
        std::snprintf(key,sizeof(key),"particle-%02d.mass",i);
        m+=param(key,0);
    }
    const char* keys[]={"FL","FR","RL","RR"};
    for(const auto* key:keys)m+=param(std::string("wheel-")+key+".mass",0);
    return m;
}
}
extern "C" {
int fayawo_contact_load_car(const char* data){
    if(!data||!*data)return 0;
    std::istringstream stream(data);
    config.Clear(); carLoaded=config.Load(stream);
    if(!carLoaded)return 0;
    mass=vehicleMass();
    radius=param("tire-both.radius",.31);
    travel=param("suspension-front.travel",.32);
    const char* wheels[]={"FL","FR","RL","RR"};
    for(int i=0;i<4;i++){
        // src coordinates: (lateral x, forward y, vertical z).
        std::string name=std::string("wheel-")+wheels[i]+".position";
        float xyz[3]={0,0,0};
        if(!config.GetParam(name,xyz))return 0;
        anchors[i]=btVector3(xyz[0],xyz[2],xyz[1]);
        const bool front=i<2;
        const std::string section=front?"suspension-front.":"suspension-rear.";
        springs[i]=CARSUSPENSION();
        springs[i].SetTravel(param(section+"travel",.32));
        springs[i].SetSpringConstant(param(section+"spring-constant",50000));
        springs[i].SetBounce(param(section+"bounce",2588));
        springs[i].SetRebound(param(section+"rebound",2600));
    }
    return (std::isfinite(mass)&&mass>200&&mass<10000&&radius>0&&travel>0)?1:0;
}
int fayawo_contact_load_tire(const char* data){
    if(!data||!*data)return 0;
    std::istringstream stream(data);
    tireConfig.Clear();
    tireLoaded=tireConfig.Load(stream)&&loadTire();
    return tireLoaded?1:0;
}
int fayawo_contact_reset(){
    if(!carLoaded||!tireLoaded)return 0;
    destroyWorld();
    collision=std::make_unique<btDefaultCollisionConfiguration>();
    dispatcher=std::make_unique<btCollisionDispatcher>(collision.get());
    broadphase=std::make_unique<btDbvtBroadphase>();
    solver=std::make_unique<btSequentialImpulseConstraintSolver>();
    world=std::make_unique<btDiscreteDynamicsWorld>(dispatcher.get(),broadphase.get(),solver.get(),collision.get());
    world->setGravity(btVector3(0,-9.81f,0));
    groundShape=std::make_unique<btBoxShape>(btVector3(100,.5,100));
    groundMotion=std::make_unique<btDefaultMotionState>(btTransform(btQuaternion(0,0,0,1),btVector3(0,-.5,0)));
    btRigidBody::btRigidBodyConstructionInfo groundInfo(0,groundMotion.get(),groundShape.get());
    ground=std::make_unique<btRigidBody>(groundInfo);
    world->addRigidBody(ground.get());
    chassisShape=std::make_unique<btBoxShape>(btVector3(.55,.23,1.25));
    btTransform start;start.setIdentity();start.setOrigin(btVector3(0,1.0,0));
    chassisMotion=std::make_unique<btDefaultMotionState>(start);
    btVector3 inertia;chassisShape->calculateLocalInertia(mass,inertia);
    btRigidBody::btRigidBodyConstructionInfo info(mass,chassisMotion.get(),chassisShape.get(),inertia);
    chassis=std::make_unique<btRigidBody>(info);
    chassis->setActivationState(DISABLE_DEACTIVATION);
    world->addRigidBody(chassis.get());
    lastNormal=lastTraction=0;lastContacts=0;
    return 1;
}
int fayawo_contact_step(double dt,double throttle,double steering){
    if(!world||!chassis||!std::isfinite(dt)||dt<=0||dt>1.0/30.0)return 0;
    throttle=std::clamp(throttle,0.0,1.0);
    steering=std::clamp(steering,-1.0,1.0);
    lastContacts=0;lastNormal=0;lastTraction=0;
    const btTransform transform=chassis->getWorldTransform();
    for(int i=0;i<4;i++){
        const btVector3 mount=transform*anchors[i];
        const double wheelTravel=springs[i].GetTravel();
        const btVector3 down=transform.getBasis()*btVector3(0,-1,0);
        const btVector3 end=mount+down*btScalar(radius+wheelTravel+.05);
        btCollisionWorld::ClosestRayResultCallback hit(mount,end);
        hit.m_collisionFilterMask=btBroadphaseProxy::StaticFilter;
        world->rayTest(mount,end,hit);
        if(!hit.hasHit())continue;
        const double distance=(hit.m_hitPointWorld-mount).length();
        const double compression=std::clamp(wheelTravel-(distance-radius),0.0,wheelTravel);
        if(compression<=0)continue;
        const btVector3 relative=mount-chassis->getCenterOfMassPosition();
        const btVector3 pointVelocity=chassis->getVelocityInLocalPoint(relative);
        const double towardGround=pointVelocity.dot(down);
        // The original CARSUSPENSION GetForce returns a negative force
        // for positive spring compression. Negate to obtain upward reaction.
        const double force=std::clamp(-springs[i].GetForce(compression,-towardGround),0.0,60000.0);
        if(force<=0)continue;
        const btVector3 up=-down;
        chassis->applyForce(up*btScalar(force),relative);
        lastNormal+=force;lastContacts++;
        if(throttle>0){
            Dbl maxforce=0;
            // Original CARTIRE Pacejka longitudinal response at wheel load (kN).
            const double tireForce=tire.Pacejka_Fx(throttle*.09,force*.001,1.0,maxforce);
            const btVector3 forward=transform.getBasis()*btVector3(0,0,1);
            const double drive=std::clamp(std::abs(tireForce),0.0,force*2.0);
            chassis->applyForce(forward*btScalar(drive),relative);
            lastTraction+=drive;
        }
    }
    if(std::abs(steering)>0.0001&&lastContacts>0){
        chassis->applyTorque(btVector3(0,btScalar(steering*lastNormal*.01),0));
    }
    world->stepSimulation(btScalar(dt),1,btScalar(dt));
    return 1;
}
double fayawo_contact_y(){return chassis?chassis->getWorldTransform().getOrigin().y():0;}
double fayawo_contact_z(){return chassis?chassis->getWorldTransform().getOrigin().z():0;}
double fayawo_contact_vz(){return chassis?chassis->getLinearVelocity().z():0;}
double fayawo_contact_mass(){return mass;}
double fayawo_contact_normal(){return lastNormal;}
double fayawo_contact_traction(){return lastTraction;}
int fayawo_contact_count(){return lastContacts;}
}
