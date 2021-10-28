//
// Created by Maciek Malik
//

#ifndef MM_imageProcessing_H_
#define MM_imageProcessing_H_

#include "../config.h"
#include "../FlightManager/app.h"

#ifdef __cplusplus
extern "C" {
#endif

position_t get_position_data();
status_frame get_status_frame();




void send_collision_advisory(collision_packet* p);

#ifdef __cplusplus
}
#endif


#endif //MM_imageProcessing_H_
