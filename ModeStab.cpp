/*
	This file is part of my quadcopter project.
	https://github.com/mkschreder/bettercopter

	This software is firmware project is free software: you can 
	redistribute it and/or modify it under the terms of the GNU General 
	Public License as published by the Free Software Foundation, either 
	version 3 of the License, or (at your option) any later version.

	This software is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder
*/

#include "ModeStab.hpp"
#include <kernel.h>
#include "mavlink.h"

// defualts

ModeStab::ModeStab(){
		
}

void ModeStab::SetPIDValues(
	const pid_values_t &stab_yaw, 
	const pid_values_t &stab_pitch, 
	const pid_values_t &stab_roll, 
	const pid_values_t &rate_yaw, 
	const pid_values_t &rate_pitch, 
	const pid_values_t &rate_roll){
	mPID[PID_STAB_YAW].kP(stab_yaw.p); 
	mPID[PID_STAB_YAW].kI(stab_yaw.i); 
	mPID[PID_STAB_YAW].kD(stab_yaw.d);
	mPID[PID_STAB_YAW].imax(stab_yaw.max_i); 
	mPID[PID_STAB_PITCH].kP(stab_pitch.p); 
	mPID[PID_STAB_PITCH].kI(stab_pitch.i); 
	mPID[PID_STAB_PITCH].kD(stab_pitch.d); 
	mPID[PID_STAB_PITCH].imax(stab_pitch.max_i); 
	mPID[PID_STAB_ROLL].kP(stab_roll.p); 
	mPID[PID_STAB_ROLL].kI(stab_roll.i); 
	mPID[PID_STAB_ROLL].kD(stab_roll.d); 
	mPID[PID_STAB_ROLL].imax(stab_roll.max_i); 
	mPID[PID_RATE_YAW].kP(rate_yaw.p); 
	mPID[PID_RATE_YAW].kI(rate_yaw.i); 
	mPID[PID_RATE_YAW].kD(rate_yaw.d); 
	mPID[PID_RATE_YAW].imax(rate_yaw.max_i); 
	mPID[PID_RATE_PITCH].kP(rate_pitch.p); 
	mPID[PID_RATE_PITCH].kI(rate_pitch.i); 
	mPID[PID_RATE_PITCH].kD(rate_pitch.d); 
	mPID[PID_RATE_PITCH].imax(rate_pitch.max_i); 
	mPID[PID_RATE_ROLL].kP(rate_roll.p); 
	mPID[PID_RATE_ROLL].kI(rate_roll.i); 
	mPID[PID_RATE_ROLL].kD(rate_roll.d); 
	mPID[PID_RATE_ROLL].imax(rate_roll.max_i); 
}

void ModeStab::Reset(){
	for(int c = 0; c < STAB_PID_COUNT; c++){
		mPID[c].reset_I(); 
	}
}

ThrottleValues ModeStab::ComputeThrottle(float dt, const RCValues &rc, 
	float yaw, float pitch, float roll, 
	float omega_yaw, float omega_pitch, float omega_roll){
	
	float rcp = -map(rc.pitch, 1000, 2000, -45, 45); //(pitch - 1500.0); 
	float rcr =  map(rc.roll, 1000, 2000, -45, 45); //(roll - 1500.0); 
	float rcy = -map(rc.yaw, 1000, 2000, -50, 50); //(yaw - 1500.0); 
	
	// calculate desired rotation rate in degrees / sec
	float sp = mPID[PID_STAB_PITCH].get_pid(rcp - pitch, dt); 
	float sr = mPID[PID_STAB_ROLL].get_pid(rcr - roll, dt); 
	float sy = mPID[PID_STAB_YAW].get_pid(wrap_180(rcy - mTargetYaw), dt); 
	
	if(abs(rc.yaw - 1500) > 10){
		sy = rcy;  
		mTargetYaw = yaw; 
	} 
	
	// calculate the actual rate based on current gyro rate in degrees
	float rp = mPID[PID_RATE_PITCH	].get_pid(sp - omega_pitch, dt); 
	float rr = mPID[PID_RATE_ROLL		].get_pid(sr - omega_roll, dt); 
	float ry = mPID[PID_RATE_YAW		].get_pid(sy - omega_yaw, dt); 
	
	// H-copter control test
	/*
	return glm::i16vec4(
		// front left
				- rr - rp + ry,
		// back right
				+ rr + rp - ry,
		// back left 
				- rr + rp - ry,
		// front right
				+ rr - rp + ry);
	*/
	return glm::i16vec4(
		// front
				+ rp + ry,
		// back  
				- rp + ry,
		// left 
				+ rr - ry,
		// right
				- rr - ry
	); 
}
