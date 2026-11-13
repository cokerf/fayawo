// Persistent original VDrift RIGIDBODY integrator used by CARDYNAMICS.
// This probe is a free body in gravity: it does NOT include Bullet collisions,
// tires or car suspension, which must be coupled in a later stage.
#include "rigidbody.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <cstdio>
#include <string>

extern "C" double fayawo_car_config_number(const char*,double);

namespace {
std::unique_ptr<RIGIDBODY> body;
double mass_kg = 0.0;

double original_vehicle_mass_from_config() {
    double sum = 0.0;
    sum += fayawo_car_config_number("engine.mass",0);
    sum += fayawo_car_config_number("driver.mass",0);
    sum += fayawo_car_config_number("fuel-tank.volume",0)
        * fayawo_car_config_number("fuel-tank.fuel-density",0);
    for(int i=0;i<32;i++){
        char name[48];
        std::snprintf(name,sizeof(name),"particle-%02d.mass",i);
        sum += fayawo_car_config_number(name,0);
    }
    const char* positions[]={"FL","FR","RL","RR","RL2","RR2","RL3","RR3"};
    for(const auto* pos:positions){
        const std::string key=std::string("wheel-")+pos+".mass";
        sum += fayawo_car_config_number(key.c_str(),0);
    }
    return sum;
}
}

extern "C" {
double fayawo_original_vehicle_mass() {
    return original_vehicle_mass_from_config();
}

// Initialise the ORIGINAL VDrift RIGIDBODY with the selected .car mass.
// Inertia is a simplified diagonal shape estimate for this integration
// probe; the complete CARDYNAMICS mass-particle inertia tensor is pending.
int fayawo_original_body_reset(double mass,double height) {
    if(!std::isfinite(mass)||mass<=0||mass>50000||
       !std::isfinite(height)||height<0||height>1000)return 0;
    body = std::make_unique<RIGIDBODY>();
    mass_kg=mass;
    body->SetMass(mass);
    body->SetPosition(MATHVECTOR<Dbl,3>(0,0,height));
    body->SetVelocity(MATHVECTOR<Dbl,3>(0,0,0));
    QUATERNION<Dbl> orientation;
    orientation.LoadIdentity();
    body->SetOrientation(orientation);
    MATRIX3<Dbl> inertia;
    inertia.Scale(mass * 0.75);
    body->SetInertia(inertia);
    body->SetAngularVelocity(MATHVECTOR<Dbl,3>(0,0,0));
    body->SetInitialForce(MATHVECTOR<Dbl,3>(0,0,-mass*9.81));
    body->SetInitialTorque(MATHVECTOR<Dbl,3>(0,0,0));
    return 1;
}

int fayawo_original_body_step(double dt,double forward_force,double yaw_torque){
    if(!body||!std::isfinite(dt)||dt<=0||dt>1.0/30.0||
       !std::isfinite(forward_force)||!std::isfinite(yaw_torque))return 0;
    body->Integrate1(dt);
    body->ApplyForce(MATHVECTOR<Dbl,3>(forward_force,0,-mass_kg*9.81));
    body->ApplyTorque(MATHVECTOR<Dbl,3>(0,0,yaw_torque));
    body->Integrate2(dt);
    return 1;
}

double fayawo_original_body_x() { return body?body->GetPosition()[0]:0; }
double fayawo_original_body_z() { return body?body->GetPosition()[2]:0; }
double fayawo_original_body_vx() { return body?body->GetVelocity()[0]:0; }
double fayawo_original_body_vz() { return body?body->GetVelocity()[2]:0; }
double fayawo_original_body_yaw_rate() {
    return body?body->GetAngularVelocity()[2]:0;
}
}
