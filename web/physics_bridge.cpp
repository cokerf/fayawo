// Thin browser ABI over the ORIGINAL VDrift physics code in src/vdrift/.
// This is a porting milestone, NOT a replacement for CARDYNAMICS.
#include "carbrake.h"
#include "caraero.h"
#include "mathvector.h"
#include "carclutch.h"
#include "cartransmission.h"
#include "carengine.h"
#include "carwheel.h"
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

int fayawo_physics_abi_version() { return 3; }
}
