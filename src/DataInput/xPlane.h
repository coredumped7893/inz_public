//
// Created by Maciek Malik
//

#ifndef MM_xPlane_H_
#define MM_xPlane_H_

#include "stdint.h"
#include "stdio.h"
#include <sys/types.h>
#include <pthread.h>
#include "../config.h"

#include "../../inc/libinetsocket.h"
#include "../../inc/libunixsocket.h"

extern const char*  X_PORT_SOURCE;
extern const char* X_PORT_SEND;
extern const char* X_HOST;



extern int x_plane_socket;//Global socket for comms with simulator
extern pthread_mutex_t x_lock;

/**
 * @brief RREF header used for communication with x-plane
 */
typedef struct  x_dataref_request{
    uint32_t freq;
    uint32_t index;
    uint8_t name[400];
}x_dataref_request_t;

/**
 * @brief Data packet used for communication with x-plane
 * @see send_DREF()
 * @see send_RREF()
 */
typedef struct{
    const void* message;
    uint16_t size;
    const char* host;
    const char* port;
}socket_packet_t;


/**
 * @brief RREFs return structure
 * @see x_dataref_request_t
 */
typedef struct __attribute((packed)) {
    //uint8_t label[5];//4 chars and null terminated
    uint32_t index;
    float value;
} x_rref_received_t;


extern const char X_DATAREF[25][50];
extern const uint8_t  X_DATAREF_SIZE[25];
extern const uint8_t X_COMMANDS[2][50];


/**
 * @brief Datarefs indexes mapped to more readable names
 */
enum DATAREFS{
    PITCH = 12,
    ROLL = 13,
    LAT = 2,
    LNG = 3,
    HEADING = 4,
    IAS = 5,
    GS = 6,
    VS = 11,
    ROLL_INPUT = 14,
    PITCH_INPUT = 15,
    YAW_INPUT = 16,
    CMD_FLAPS_UP = 0,
    CMD_FLAPS_DOWN = 1,
    FD_PITCH = 18,
    FD_ROLL = 19,
    ALT = 7,
    ENG_1_THR = 0,
    MAG_VAR = 20,
    ENG_2_THR = 21,
    ENG_1_EGT = 1,
    WIND_DIR = 22,
    WIND_SPEED = 23,
    TAS = 24
};

#ifdef TEST
 void socket_send_ip(socket_packet_t *packet);
#endif

void printConfig();
void send_DREF(const uint8_t datarefID, float value);
void send_RREF(x_dataref_request_t* request);
socket_packet_t * alloc_socket_packet();
void x_send_command(uint8_t cID);

#endif //MM_xPlane_H_
