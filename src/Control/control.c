//
// Created by Maciek Malik
//

#include "control.h"
#include "stdlib.h"
#include "../Util/message.h"
//#include "../DataInput/dataInput.h"

//#if SIM
//    #include "simulatedOut.h"
//#else
//    #include "hardwareOut.h"
//#endif

/**
 * @brief Create control context for use with simulator (x-plane)
 * @return control_strategy_t
 */
control_strategy_t create_control_simulated_context(){
    control_strategy_t s;
    s.set_axis = sim_set_axis;
    s.send_data = sim_send_data;
    s.send_command = sim_send_command;
    s.set_throttle = sim_set_throttle;
    return s;
}

/**
 * @brief Create control context for use with real hardware (not implemented yet)
 * @return control_strategy_t
 */
control_strategy_t create_control_hardware_context(){
    control_strategy_t s;
    s.set_axis = h_set_axis;
    s.send_data = h_send_data;
    s.send_command = h_send_command;
    s.set_throttle = h_set_throttle;
    return s;
}

/**
 * @brief Allocate AxisSet structure on the heap
 * @return
 */
AxisSet *alloc_axis_set() {
    AxisSet* tmpOut  = calloc(1,sizeof(AxisSet));
    if(tmpOut == NULL){
        throw_err(3, 1);
    }
    return tmpOut;
}



