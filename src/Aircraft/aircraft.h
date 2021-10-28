//
// Created by Maciek Malik
//

#ifndef MM_aircraft_H_
#define MM_aircraft_H_

#include "../FlightManager/PID.h"
#define UNUSED(x) (void)(x)
#include <stdint.h>

/**
 * @brief Structure containing all the required configuration for aircraft to fly and navigate properly
 * @see PIDController_t
 * @see pid_input_t
 */
typedef struct{
    char name[50];
    //Main axis configs ---------
    PIDController_t roll;
    PIDController_t pitch;
    PIDController_t throttle;
    //---------------------------

    //FD data -------------------------
    PIDController_t FD_roll;
    PIDController_t FD_pitch;
    //---------------------------------

    //Initial targets-----------
    pid_input_t input_roll;
    pid_input_t input_pitch;
    pid_input_t input_fd_roll;
    pid_input_t input_fd_pitch;
    pid_input_t input_throttle;
    //--------------------------

    //Other---------------------------------
    uint8_t engine_count;
    uint8_t adjust_limit_to_error;//When = 1, max bank angle decreases when close to the target
    //--------------------------------------

    //Technical specs ----------------
    float turn_radius;
    double vs;//stall speed (max gross w., clean config)
    double vne;//never exceed speed
    double service_ceiling;//altitude at which aircraft will still have at least 100fpm
    double wingspan;
    double roc_0;//rate of climb at the sea level
    double roc_5k;// rate of climb at 5000 ft amsl
    double roc_10k;//rate of climb at 10000 ft amsl
    //--------------------------------


}aircraftConfig_t;
//Include aircraft configs below aircraftConfig_t only!


#include "prk2.h"
#include "sf50.h"


#endif //MM_aircraft_H_
