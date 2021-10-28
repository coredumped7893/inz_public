//
// Maciek Malik
//

#ifndef MM_PID_H_
#define MM_PID_H_

//Source: https://github.com/pms67/PID/blob/master/PID.h
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
 * @brief PID instance structure, hold all the data needed to calculate current and next states,
 * also contains limiting values to protect output stay inside acceptable bounds
 */
typedef struct {

    /* Controller gains */
    float Kp;
    float Ki;
    float Kd;

    /* Derivative low-pass filter time constant */
    float tau;

    /* Output limits */
    float limMin;
    float limMax;

    /* Sample time (in seconds) */
    float T;


    //Added by Maciek Malik -------------
    float Td;//"prediction" - how far to look into the future
    float Ti;//how long to apply tuning
    float preD1; //Pre calculated value for differentiator
    float preI1;//Pre calculated value for integrator
    //------------------------------------



    /* Integrator limits */
    float limMinInt;
    float limMaxInt;

    /* Controller "memory" */
    float integrator;
    float prevError;			/* Required for integrator */
    float differentiator;
    float prevMeasurement;		/* Required for differentiator */

    /* Controller output */
    float out;

} PIDController_t;

/**
 * @brief Structure used for passing data required to calculate PID output
 */
typedef struct {
    float target;
    float measurement;
    short isHeading;//if target is given in degrees, change error calculation method.
}pid_input_t;

/**
 * @brief Struct containing initial PID targets values
 */
typedef struct{
    /**
     * Altitude in meters
     */
    double alt;

    /**
     * Heading in degrees
     */
    double hdg;

    /**
     * Indicated air speed in knots
     */
    double ias;
}pid_initial_targets;



void controller_init(PIDController_t* pid);
float controller_calc_frame(PIDController_t* pid,pid_input_t in);
int min_heading_error(int target, int hdg);
#endif //MM_PID_H_












