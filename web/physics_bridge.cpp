// Thin browser ABI over the ORIGINAL VDrift physics code in src/vdrift/.
// This is a porting milestone, NOT a replacement for CARDYNAMICS.
#include "carbrake.h"
#include "caraero.h"
#include "mathvector.h"
#include "carclutch.h"
#include "cartransmission.h"
#include "carengine.h"
#include "carwheel.h"
#include "carsuspension.h"
#include "configfile.h"
#include "cartire.h"
#include <sstream>
#include <cstring>

static CONFIGFILE fayawo_original_car;
#include <cmath>
#include <algorithm>

extern "C" {
// Actual original CARBRAKE::GetTorque() in src/vdrift/carbrake.cpp.
double fayawo_brake_torque(double pedal, double handbrake, double handbrake_bias) {
    CARBRAKE brake;
    brake.SetBrakeFactor(std::clamp(pedal, 0.0, 1.0));
    brake.SetHandbrakeFactor(std::clamp(handbrake, 0.0, 1.0));
    brake.SetHandbrake(std::clamp(handbrake_bias, 0.0, 1.0));
    return brake.GetTorque();
}

// Actual original CARAERO::GetForce() in src/vdrift/caraero.cpp.
double fayawo_aero_drag(double speed_metres_per_second,
                        double frontal_area, double drag_coefficient) {
    CARAERO aero;
    MATHVECTOR<Dbl,3> position(0.0, 0.0, 0.0);
    aero.Set(position, std::max(0.0, frontal_area),
             std::max(0.0, drag_coefficient), 0.0, 0.0, 0.0);
    MATHVECTOR<Dbl,3> headwind(-std::max(0.0, speed_metres_per_second), 0.0, 0.0);
    return -aero.GetForce(headwind)[0]; // positive resisting force, N
}

double fayawo_vector_length(double x, double y, double z) {
    return MATHVECTOR<Dbl,3>(x,y,z).Magnitude();
}

// Actual original CARCLUTCH::GetTorque() in src/vdrift/carclutch.cpp.
double fayawo_clutch_torque(double engagement,double engine_rads,double shaft_rads) {
    CARCLUTCH clutch;
    clutch.SetClutch(std::clamp(engagement, 0.0, 1.0));
    return clutch.GetTorque(engine_rads, shaft_rads);
}

// Actual original CARTRANSMISSION gearbox and shaft calculations.
double fayawo_gearbox_torque(int gear,double engine_torque) {
    CARTRANSMISSION transmission;
    transmission.SetGearRatio(-1,-3.1);
    transmission.SetGearRatio(1,3.2);
    transmission.SetGearRatio(2,2.1);
    transmission.SetGearRatio(3,1.4);
    transmission.Shift(gear);
    return transmission.GetTorque(engine_torque);
}

double fayawo_clutch_speed(int gear,double shaft_rads) {
    CARTRANSMISSION transmission;
    transmission.SetGearRatio(1,3.2);
    transmission.SetGearRatio(2,2.1);
    transmission.SetGearRatio(3,1.4);
    transmission.Shift(gear);
    return transmission.GetClutchSpeed(shaft_rads);
}

// Original CARENGINE torque curve, crankshaft and friction implementation.
double fayawo_engine_torque(double throttle, double rpm) {
    CARENGINE engine;
    std::vector<std::pair<Dbl,Dbl>> curve = {
        {1000.0, 200.0}, {4000.0, 250.0}, {7800.0, 100.0}
    };
    engine.SetTorqueCurve(4000.0, curve);
    engine.StartEngine();
    engine.SetAngularVelocity(std::max(0.0, rpm) * PI_d / 30.0);
    engine.SetThrottle(std::clamp(throttle, 0.0, 1.0));
    engine.ComputeForces();
    return engine.GetTorque();
}

// Original CARWHEEL rolling resistance and rotational inertia model.
double fayawo_wheel_rolling_resistance(double speed, double surface_multiplier) {
    CARWHEEL wheel;
    return wheel.GetRollingResistance(speed, std::max(0.0, surface_multiplier));
}

double fayawo_wheel_inertia() {
    CARWHEEL wheel;
    return wheel.GetInertia();
}

// Original CARSUSPENSION spring, compression and rebound damping.
double fayawo_suspension_force(double displacement, double velocity) {
    CARSUSPENSION suspension;
    return suspension.GetForce(std::clamp(displacement, 0.0, suspension.GetTravel()), velocity);
}

double fayawo_suspension_step(double dt, double displacement) {
    if (dt <= 0.0) return 0.0;
    CARSUSPENSION suspension;
    return suspension.Update(dt, displacement);
}

// Parse the ORIGINAL .car format with the ORIGINAL VDrift CONFIGFILE parser.
// JavaScript fetches an actual .car asset and passes its text into the C++ parser.
int fayawo_car_config_load(const char* text) {
    if (!text || !*text) return 0;
    std::istringstream in(text);
    fayawo_original_car.Clear();
    return fayawo_original_car.Load(in) ? 1 : 0;
}
double fayawo_car_config_number(const char* key, double fallback) {
    if (!key || !*key) return fallback;
    float result = 0.0f;
    return fayawo_original_car.GetParam(std::string(key), result)
        ? static_cast<double>(result) : fallback;
}
int fayawo_car_config_exists(const char* key) {
    if (!key || !*key) return 0;
    std::string value;
    return fayawo_original_car.GetParam(std::string(key), value) ? 1 : 0;
}
// Bind several original C++ systems to the selected *real* .car configuration.
// These are subsystem calculations, not the full CARDYNAMICS vehicle simulation.
double fayawo_selected_gearbox_torque(int gear, double clutch_torque) {
    if (gear < 1 || gear > 8) return 0.0;
    CARTRANSMISSION transmission;
    for (int i=1; i<=gear; ++i) {
        float ratio = 0;
        if (!fayawo_original_car.GetParam("transmission.gear-ratio-" + std::to_string(i), ratio))
            return 0.0;
        transmission.SetGearRatio(i, ratio);
    }
    transmission.Shift(gear);
    return transmission.GetTorque(clutch_torque);
}
double fayawo_selected_brake_torque(double pedal) {
    CARBRAKE brake;
    float value;
    if (fayawo_original_car.GetParam("brakes-front.friction", value)) brake.SetFriction(value);
    if (fayawo_original_car.GetParam("brakes-front.max-pressure", value)) brake.SetMaxPressure(value);
    if (fayawo_original_car.GetParam("brakes-front.bias", value)) brake.SetBias(value);
    if (fayawo_original_car.GetParam("brakes-front.radius", value)) brake.SetRadius(value);
    if (fayawo_original_car.GetParam("brakes-front.area", value)) brake.SetArea(value);
    brake.SetBrakeFactor(std::clamp(pedal,0.0,1.0));
    return brake.GetTorque();
}
double fayawo_selected_suspension_force(double compression, double vertical_speed) {
    CARSUSPENSION suspension;
    float value;
    if (fayawo_original_car.GetParam("suspension-front.spring-constant",value)) suspension.SetSpringConstant(value);
    if (fayawo_original_car.GetParam("suspension-front.travel",value)) suspension.SetTravel(value);
    if (fayawo_original_car.GetParam("suspension-front.bounce",value)) suspension.SetBounce(value);
    if (fayawo_original_car.GetParam("suspension-front.rebound",value)) suspension.SetRebound(value);
    return suspension.GetForce(std::clamp(compression,0.0,suspension.GetTravel()),vertical_speed);
}
double fayawo_selected_engine_torque(double throttle,double rpm) {
    std::vector<std::pair<double,double>> curve;
    fayawo_original_car.GetPoints("engine","torque-curve-",curve);
    if (curve.size()<2) return 0.0;
    CARENGINE engine;
    float value;
    if (fayawo_original_car.GetParam("engine.rpm-limit",value)) engine.SetRpmMax(value);
    if (fayawo_original_car.GetParam("engine.start-rpm",value)) engine.SetStartRPM(value);
    if (fayawo_original_car.GetParam("engine.stall-rpm",value)) engine.SetStallRPM(value);
    double max_power_rpm = curve.front().first;
    double max_power = 0.0;
    for(const auto& p:curve) {
       if(p.first*p.second>max_power){max_power=p.first*p.second;max_power_rpm=p.first;}
    }
    engine.SetTorqueCurve(max_power_rpm,curve);
    engine.StartEngine();
    engine.SetAngularVelocity(std::max(0.0,rpm)*PI_d/30.0);
    engine.SetThrottle(std::clamp(throttle,0.0,1.0));
    engine.ComputeForces();
    return engine.GetTorque();
}
int fayawo_physics_abi_version() { return 7; }
}

// Browser tyre milestone: same CARTIRE Pacejka coefficients and equations as the game.
// The original .tire format is loaded by the original CONFIGFILE parser.
static CONFIGFILE fayawo_original_tire;
static CARTIRE fayawo_tire;

static bool load_original_tire_values(CARTIRE& tire, const CONFIGFILE& source) {
    float value;
    for (int i=0;i<=10;++i) {
        if (!source.GetParam("params.b"+std::to_string(i),value)) return false;
        tire.longitudinal[i]=value;
    }
    for (int i=0;i<=10;++i) {
        if (!source.GetParam("params.a"+std::to_string(i),value)) return false;
        tire.lateral[i]=value;
    }
    const char* extra[]={"a111","a112","a12","a13"};
    for (int i=0;i<4;++i) {
        if (!source.GetParam(std::string("params.")+extra[i],value)) return false;
        tire.lateral[11+i]=value;
    }
    for (int i=0;i<18;++i) {
        if (!source.GetParam("params.c"+std::to_string(i),value)) return false;
        tire.aligning[i]=value;
    }
    return true;
}

extern "C" {
int fayawo_tire_config_load(const char* text) {
    if (!text || !*text) return 0;
    std::istringstream input(text);
    fayawo_original_tire.Clear();
    if (!fayawo_original_tire.Load(input)) return 0;
    CARTIRE tire;
    if (!load_original_tire_values(tire,fayawo_original_tire)) return 0;
    fayawo_tire=tire;
    return 1;
}
double fayawo_tire_longitudinal_force(double slip_ratio,double normal_force_newtons) {
    if (normal_force_newtons<=0) return 0;
    Dbl peak=0;
    return fayawo_tire.Pacejka_Fx(slip_ratio,normal_force_newtons*0.001,1.0,peak);
}
double fayawo_tire_lateral_force(double slip_angle_degrees,double normal_force_newtons) {
    if (normal_force_newtons<=0) return 0;
    Dbl peak=0;
    return fayawo_tire.Pacejka_Fy(slip_angle_degrees,normal_force_newtons*0.001,0.0,1.0,peak);
}
}
