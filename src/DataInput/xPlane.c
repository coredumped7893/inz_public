//
// Created by Maciek Malik
//

#include "xPlane.h"
#include <stdlib.h>
#include <string.h>
#include "../Util/message.h"
#include "../../inc/stringlib.h"
#include "../Util/buff_tools.h"


pthread_mutex_t x_lock;

/**
 * Socket for communication with X-plane
 * receiving requested data with send_data_request
 * and sending RREF/DREF packets
 * @see X_PORT_SEND
 * @see X_HOST
 * @see send_data_request()
 */
int x_plane_socket;

#ifndef TEST
static_testable void socket_send_ip(socket_packet_t *packet);
#endif

/**
 * @brief Just prints network config of the simulator
 */
void printConfig(){
    printf("----- X-PLane CFG -----\n");
    printf("PORT: %s\n", X_PORT_SEND);
    printf("HOST: %s\n",X_HOST);
    printf("----------");
}

/**
 * @brief Set a dataref to a value
 * @see X_DATAREF
 * @see alloc_socket_packet()
 * @param datarefID
 * @param value
 */
void send_DREF(const uint8_t datarefID, float value) {

    //@TODO put this in other thread to not block pid controllers

    socket_packet_t* p = alloc_socket_packet();
    p->size = 509;

    uint8_t * padding = calloc(p->size-(X_DATAREF_SIZE[datarefID]+9),sizeof(uint8_t));
    memset(padding,' ',p->size-(X_DATAREF_SIZE[datarefID]+9));//Fill empty space with spaces

    uint8_t float_bytes[4];
    float2Bytes(float_bytes,value);

    //Memory leak fix ------
    uint8_t* bApp2 = buff_append((uint8_t*)"DREF\0",float_bytes,5,4);
    uint8_t* bApp1 = buff_append(bApp2,(uint8_t*)X_DATAREF[datarefID],9, X_DATAREF_SIZE[datarefID] );
    //----------------------

    p->message = (char*) buff_append(
            bApp1 ,padding, X_DATAREF_SIZE[datarefID]+9 ,  p->size-(X_DATAREF_SIZE[datarefID]+9) );
    socket_send_ip(p);
    free((void*)p->message);
    free(padding);
    free(bApp1);
    free(bApp2);
    free(p);
}

/**
 * @brief Request dataref subscription from simulator
 * data will be sent back on the same socket with index and frequency specified earlier in the parameter
 * @see alloc_socket_packet()
 * @see X_DATAREF
 * @param request
 */
void send_RREF(x_dataref_request_t* request) {

    socket_packet_t* p = alloc_socket_packet();
    p->size = 413;
    uint8_t freq_bytes[4];
    uint8_t index_bytes[4];
    int2bytes(index_bytes,request->index);
    int2bytes(freq_bytes,request->freq);

    uint8_t * padding = calloc(p->size-(13+X_DATAREF_SIZE[request->index]),sizeof(uint8_t));
    memset(padding,' ',p->size-(13+X_DATAREF_SIZE[request->index]));//Fill empty space with spaces

    //Memory leak fix ------
    uint8_t* bApp3 = buff_append((uint8_t*) freq_bytes,(uint8_t*) index_bytes,4,4);
    uint8_t* bApp2 = buff_append((uint8_t *)"RREF\0",bApp3,5, 8);
    uint8_t* bApp1 = buff_append(bApp2,(uint8_t*)X_DATAREF[request->index],13,X_DATAREF_SIZE[request->index]);
    //----------------------

    p->message = (char*)
        buff_append(bApp1,padding,(13+X_DATAREF_SIZE[request->index]),p->size-(13+X_DATAREF_SIZE[request->index]));

    socket_send_ip(p);
    free((void*)p->message);
    free(request);
    free(padding);
    free(bApp1);
    free(bApp2);
    free(bApp3);
    free(p);

}

/**
 * Sends packet to predefined host and port
 * @param packet
 */
TESTING_REQ_WEAK
static_testable void socket_send_ip(socket_packet_t * packet) {

    int sfd;
    int ret;

    ret = sfd = x_plane_socket;

    if (sfd < 0) {
        perror(0);
        exit(1);
    }

    ret = sendto_inet_dgram_socket(sfd, packet->message, packet->size, packet->host, packet->port, 0);

    //ret = destroy_inet_socket(sfd);

    if (ret < 0) {
        perror(0);
        exit(1);
    }
}

/**
 * @brief Send single command to the simulator.
 * ex. Change flap position, turn on carb heat, etc...
 * @param commandID
 */
void x_send_command(uint8_t commandID) {
    socket_packet_t* p = alloc_socket_packet();
    p->size = sizeof(X_COMMANDS[commandID])+5;
    p->message = (char*)
            buff_append((uint8_t *)"CMND\0",(uint8_t *)X_COMMANDS[commandID],5,p->size);
    socket_send_ip(p);
    free((void*)p->message);
    free(p);
}


//--------------------------------------------------------------------------
const char*  X_PORT_SOURCE = "49011";
const char* X_PORT_SEND = X_PLANE_PORT;
#if SIM
    const char* X_HOST = X_PLANE_IP;
#else
    const char* X_HOST = "192.168.1.92";
#endif

/**
 * @brief Predefined list of possible datarefs.
 * Defined here as a global array to simplify use in other functions (only index would be required)
 * Indexes are mapped in enum DATAREFS in xPlane.h
 * @see xPlane.h
 */
const char X_DATAREF[25][50] = {
        "sim/flightmodel/engine/ENGN_thro[0]\0",//0
        "sim/flightmodel/engine/ENGN_EGT_c[0]\0",//1
        "sim/flightmodel/position/latitude\0",//2
        "sim/flightmodel/position/longitude\0",//3
        "sim/flightmodel/position/mag_psi\0",//4
        "sim/flightmodel/position/indicated_airspeed\0",//5
        "sim/flightmodel/position/groundspeed\0",//6
        "sim/flightmodel/position/elevation\0",//7
        "sim/flightmodel/position/P\0",//8
        "sim/flightmodel/position/Q\0",//9
        "sim/flightmodel/position/R\0",//10
        "sim/flightmodel/position/vh_ind\0",//11
        "sim/flightmodel/position/true_theta\0",//12 pitch
        "sim/flightmodel/position/true_phi\0",//13 roll
        "sim/joystick/yoke_roll_ratio\0",//14 input
        "sim/joystick/yoke_pitch_ratio\0",//15 input
        "sim/joystick/yoke_heading_ratio\0",//16 input
        "sim/operation/override/override_flightdir\0",//17
        "sim/cockpit/autopilot/flight_director_pitch\0",//18 input
        "sim/cockpit/autopilot/flight_director_roll\0",//19 input
        "sim/flightmodel/position/magnetic_variation\0",//20
        "sim/flightmodel/engine/ENGN_thro[1]\0",//21
        "sim/weather/wind_direction_degt\0",//22
        "sim/weather/wind_speed_kt\0",//23
        "sim/flightmodel/position/true_airspeed\0"//24
};

const uint8_t X_DATAREF_SIZE[25] = {
        36,
        37,
        34,
        35,
        33,
        44,
        37,
        35,
        27,
        27,
        27,
        32,
        36,
        34,
        29,
        30,
        33,
        42,
        44,
        43,
        44,
        36,
        32,
        26,
        39
};

/**
 * @brief Commands list that can be send
 * @see x_send_command()
 */
const uint8_t X_COMMANDS[2][50] = {
        "sim/flight_controls/flaps_up",
        "sim/flight_controls/flaps_down"
};

/**
 * @brief Allocates socket_packet_t on the heap
 * @return ptr to socket_packet_t
 */
socket_packet_t * alloc_socket_packet() {
    socket_packet_t* tmpP = calloc(1,sizeof(socket_packet_t));
    if(tmpP == NULL){
        throw_err(3, 1);
        return NULL;
    }
    tmpP->port = X_PORT_SEND;
    tmpP->host = X_HOST;

    return tmpP;
}