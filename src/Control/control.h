//
// Created by Maciek Malik
//

#ifndef MM_control_H_
#define MM_control_H_

#include "../config.h"
#include "stdint.h"
//#include "../DataInput/xPlane.h"
#include "../FlightManager/app.h"

typedef struct {
    float pitch;
    float roll;
    float yaw;
}AxisSet;
AxisSet* alloc_axis_set();

#include "controlStrategy.h"


//void set_axis(AxisSet*);//Sets AxisSet
//void set_throttle(float t);//value range: [-1;1]
//void send_data(float,  uint8_t);//Send any value
//void send_command( uint8_t);//CMND




#endif //MM_control_H_
