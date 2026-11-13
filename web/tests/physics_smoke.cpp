#include <cassert>
#include <cmath>
#include <iostream>

extern "C" double fayawo_brake_torque(double,double,double);
extern "C" double fayawo_aero_drag(double,double,double);
extern "C" double fayawo_vector_length(double,double,double);
extern "C" int fayawo_physics_abi_version();
extern "C" double fayawo_clutch_torque(double,double,double);
extern "C" double fayawo_gearbox_torque(int,double);
extern "C" double fayawo_clutch_speed(int,double);

static bool near(double a, double b, double tol=1e-6) {
  return std::abs(a-b)<tol;
}
int main() {
    assert(fayawo_physics_abi_version()==2);
    assert(near(fayawo_vector_length(3,4,0),5));
    assert(near(fayawo_brake_torque(0,0,0),0));
    assert(near(fayawo_brake_torque(1,0,0),3066));
    assert(near(fayawo_brake_torque(0,1,1),3066));
    assert(near(fayawo_aero_drag(0,2.2,0.3),0));
    assert(near(fayawo_aero_drag(10,2.2,0.3),39.6));
    assert(near(fayawo_clutch_torque(0,100,99),0));
    assert(near(fayawo_clutch_torque(1,100,99),336.53));
    assert(near(fayawo_gearbox_torque(1,100),320));
    assert(near(fayawo_gearbox_torque(2,100),210));
    assert(near(fayawo_gearbox_torque(-1,100),-310));
    assert(near(fayawo_clutch_speed(2,20),42));
    std::cout << "Original VDrift brakes, aero, clutch, gearbox and math smoke tests passed\n";
}
