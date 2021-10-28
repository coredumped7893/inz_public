//
// Created by Maciek Malik
//

#ifndef MM_controlStrategy_H_
#define MM_controlStrategy_H_

//typedef void (*controlStrategy)();
typedef void (*fn_set_axis)(AxisSet* axis);
typedef void (*fn_send_data)(float value,const uint8_t datarefID);
typedef void (*fn_set_throttle)(float value);
typedef void (*fn_send_command)(const uint8_t commandID);


/**
 * @brief Control strategy structure containing pointer to all the necessary functions required for controlling an aircraft
 */
typedef struct {
    fn_set_axis set_axis;
    fn_send_data send_data;
    fn_set_throttle set_throttle;
    fn_send_command send_command;
}control_strategy_t;

//Include here new strategy headers
#include "simulatedOut.h"
#include "hardwareOut.h"

control_strategy_t create_control_simulated_context();
control_strategy_t create_control_hardware_context();


#endif //MM_controlStrategy_H_
