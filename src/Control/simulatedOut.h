//
// Created by Maciek Malik
//

#ifndef MM_simulatedOut_H_
#define MM_simulatedOut_H_
#include "../DataInput/xPlane.h"
#include "../Control/control.h"
#include "stdint.h"

void sim_set_throttle(float value);
void sim_send_command( uint8_t);
void sim_set_axis(AxisSet * axis);
void sim_send_data(float val, uint8_t datarefID);
#endif //MM_simulatedOut_H_
