
//
// Maciek Malik
//

#include <stdio.h>
#include <stdlib.h>
#include "../Util/other.h"
#include "PID.h"
#include "../Util/message.h"
#include "route.h"

/**
 * @brief Initializes PIDController_t structure for calculations
 * @param pid
 */
void controller_init(PIDController_t* pid) {

    if(pid->T == 0 || pid->limMax == 0){
        //parameters not initialized
        throw_err(5,1);
    }

    pid->integrator = 0.0f;
    pid->prevError  = 0.0f;

    pid->differentiator  = 0.0f;
    pid->prevMeasurement = 0.0f;

    pid->limMinInt = pid->limMin*0.4f;
    pid->limMaxInt  = pid->limMax*0.4f;

    pid->out = 0.0f;

}




//Source: https://github.com/pms67/PID/blob/master/PID.c
/*
MIT License

Copyright (c) 2020 Philip Salmony

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

/**
 * @brief Calculates next PID state, based on PIDController_t configuration
 * and pid_input_t input structure
 * @see Source of the function(most of it): https://github.com/pms67/PID/blob/master/PID.c
 * @see pid_input_t
 * @see PIDController_t
 * @param pid
 * @param in
 * @return
 */
float controller_calc_frame(PIDController_t *pid, pid_input_t in) {
    float p,err;

    if(in.isHeading){
        err = (float)(min_heading_error((int)in.target, (int)in.measurement));
        //printf("ERR2: %f\n",err);
    }else{
        err = (in.target - in.measurement);
    }
    p = pid->Kp * err;

    pid->integrator = pid->integrator + 0.5f * pid->Ki * pid->T * (err + pid->prevError);

    pid->differentiator = -(2.0f * pid->Kd * (in.measurement - pid->prevMeasurement)	/* Note: derivative on measurement, therefore minus sign in front of equation! */
                                + (2.0f * pid->tau - pid->T) * pid->differentiator)
                              / (2.0f * pid->tau + pid->T);

    //Limits the value for integrator to prevent big overshooting
    if(pid->integrator > pid->limMaxInt){
        pid->integrator = pid->limMaxInt;
    }else if(pid->integrator < pid->limMinInt){
        pid->integrator = pid->limMinInt;
    }

    pid->out = p + pid->integrator + pid->differentiator;

    pid->prevMeasurement = in.measurement;
    pid->prevError       = err;

    //Restricting output to allowed domain, in our case, for controls it would be [-1;1]
    if(pid->out > pid->limMax){
        pid->out = pid->limMax;
    } else if(pid->out < pid->limMin){
        pid->out = pid->limMin;
    }

    return pid->out;
}

/**
 * @brief Finds minimal heading difference between current aircraft heading and target.
 * Takes into account passing 0 deg
 * @param target
 * @param hdg
 * @return
 */
int min_heading_error(int target, int hdg) {

    int d_heading = (target - hdg);
    int d_short_heading = deg_from_north((int)hdg) + (int)deg_from_north((int)target);
    if(d_short_heading < abs(d_heading)){
        if(hdg > 0 && hdg <= 180){
            return d_short_heading;
        }else{
            return -1*d_short_heading;
        }
    }else{
        return d_heading;
    }

}
