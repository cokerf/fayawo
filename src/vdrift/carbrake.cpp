#ifndef FAYAWO_WEB
#include "pch.h"
#endif
#include "carbrake.h"
#ifndef FAYAWO_WEB
#include "Def_Str.h"
#endif


#ifndef FAYAWO_WEB
void CARBRAKE::DebugPrint(std::ostream & out)
{
	//out << "---Brake---" << std::endl;
	//out << "Brake    : " << brake_factor << std::endl;
	//out << "Handbrake: " << handbrake_factor << std::endl;
	out << "Torque " << fToStr(lasttorque, 0,5) << std::endl;
	out << "Locked  " << locked << std::endl;
}
#endif

///brake torque magnitude
Dbl CARBRAKE::GetTorque()
{
	Dbl brake = brake_factor > handbrake*handbrake_factor ? brake_factor : handbrake*handbrake_factor;
	Dbl pressure = brake * bias * max_pressure;
	Dbl normal = pressure * area;
	Dbl torque = friction * normal * radius;
	lasttorque = torque;
	return torque;
}
