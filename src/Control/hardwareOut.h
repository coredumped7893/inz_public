//
// Created by Maciek Malik
//

#ifndef MM_hardwareOut_H_
#define MM_hardwareOut_H_
#include "stdint.h"
#include "../Control/control.h"

void h_set_throttle(float value);
void h_send_command(uint8_t cmdid);
void h_set_axis(AxisSet * axis);
void h_send_data(float val, uint8_t datarefID);
#endif //MM_hardwareOut_H_
