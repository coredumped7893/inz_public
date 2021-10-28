//
// Created by Maciek Malik
//

#ifndef MM_DATAINPUT_H
#define MM_DATAINPUT_H

#include <stdint.h>
#include <pthread.h>
#include "../FlightManager/app.h"

uint8_t init_sources();
//extern uint8_t DEFAULT_DATA_FREQ;
void send_data_request(uint8_t freq, const uint8_t datarefID);//Sends RREF request
float get_data(uint8_t dataIndex, int delayed);//Return saved value from buffer
void send_status_frame_request();
void set_data_delay(int packets);
extern pthread_t data_input;
status_frame* get_status_frame(void*, int delayed);

#endif //MM_DATAINPUT_H