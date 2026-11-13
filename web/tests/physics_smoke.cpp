#include <cassert>
#include <cmath>
#include <iostream>

extern "C" double fayawo_brake_torque(double,double,double);
extern "C" double fayawo_aero_drag(double,double,double);
extern "C" double fayawo_vector_length(double,double,double);
extern "C" int fayawo_physics_abi_version();

static bool near(double a, double b, double tol=1e-6) {
  return std::abs(a-b)<tol;
}
int main() {
    assert(fayawo_physics_abi_version()==1);
    assert(near(fayawo_vector_length(3,4,0),5));
    assert(near(fayawo_brake_torque(0,0,0),0));
    assert(near(fayawo_brake_torque(1,0,0),3066));
    assert(near(fayawo_brake_torque(0,1,1),3066));
    assert(near(fayawo_aero_drag(0,2.2,0.3),0));
    assert(near(fayawo_aero_drag(10,2.2,0.3),39.6));
    std::cout << "Original VDrift brake, aero and math smoke tests passed\n";
}
