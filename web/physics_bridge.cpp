// Thin browser ABI over the ORIGINAL VDrift physics code in src/vdrift/.
// This is a porting milestone, NOT a replacement for CARDYNAMICS.
#include "carbrake.h"
#include "caraero.h"
#include "mathvector.h"
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

int fayawo_physics_abi_version() { return 1; }
}
