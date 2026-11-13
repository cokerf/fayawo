#include <cassert>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

extern "C" double fayawo_brake_torque(double,double,double);
extern "C" double fayawo_aero_drag(double,double,double);
extern "C" double fayawo_vector_length(double,double,double);
extern "C" int fayawo_physics_abi_version();
extern "C" double fayawo_clutch_torque(double,double,double);
extern "C" double fayawo_gearbox_torque(int,double);
extern "C" double fayawo_clutch_speed(int,double);
extern "C" double fayawo_engine_torque(double,double);
extern "C" double fayawo_wheel_rolling_resistance(double,double);
extern "C" double fayawo_wheel_inertia();
extern "C" double fayawo_suspension_force(double,double);
extern "C" double fayawo_suspension_step(double,double);
extern "C" int fayawo_car_config_load(const char*);
extern "C" double fayawo_car_config_number(const char*,double);
extern "C" int fayawo_car_config_exists(const char*);

static bool near(double a, double b, double tol=1e-6) {
  return std::abs(a-b)<tol;
}
int main() {
    assert(fayawo_physics_abi_version()==5);
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
    assert(near(fayawo_wheel_rolling_resistance(10,1),0.01365));
    assert(near(fayawo_wheel_rolling_resistance(-10,1),-0.01365));
    assert(near(fayawo_wheel_inertia(),10));
    assert(near(fayawo_suspension_force(0.1,0),-5000));
    assert(near(fayawo_suspension_force(0.1,1),-7588));
    assert(near(fayawo_suspension_force(0.1,-1),-2400));
    assert(near(fayawo_suspension_step(0.01,0),0));
    assert(fayawo_engine_torque(1,4000)>0);
    assert(std::isfinite(fayawo_engine_torque(0.5,2500)));
    std::ifstream original("data/carsim/normal/cars/ES.car");
    assert(original.good());
    std::ostringstream buffer;
    buffer << original.rdbuf();
    assert(fayawo_car_config_load(buffer.str().c_str()) == 1);
    assert(fayawo_car_config_exists("engine.rpm-limit") == 1);
    assert(near(fayawo_car_config_number("engine.rpm-limit",0),6600));
    assert(near(fayawo_car_config_number("transmission.gear-ratio-1",0),2.909,1e-4));
    assert(near(fayawo_car_config_number("suspension-front.travel",0),0.32,1e-4));
    assert(near(fayawo_car_config_number("missing.parameter",27),27));
    std::cout << "Original VDrift configs, suspension, engine, wheel and driveline smoke tests passed\n";
}
