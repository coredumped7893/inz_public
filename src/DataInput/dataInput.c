//
// Created by Maciek Malik
//

#include "dataInput.h"
#include "../../inc/libinetsocket.h"
#include <stdio.h>
#include <stdlib.h>
#include "xPlane.h"
#include "string.h"

#include "../Util/message.h"
#include "../config.h"

pthread_t data_input;

/**
 * @brief Data history up to X seconds (defined in config.h)
 */
float* data_buffer_packets;

/**
 * @brief Current data received from data source
 */
float* current_data_buffer;
//static float* _prev_data_buffer;
static int data_thread_run = 1;
static int delay_by_x_packets = 0;
static uint16_t packet_number = 0;
static uint16_t  max_packet_number = DATA_DELAY_SECONDS*DEFAULT_DATA_FREQ;
static int max_data_idx = (NUMBER_OF_DATA_INPUTS*DEFAULT_DATA_FREQ*DATA_DELAY_SECONDS);
void* th_receive_data(void * a);
static inline uint16_t get_delayed_index(int delayed);

/**
 * @brief Initializes buffers for data, create socket for comms with x-plane
 * @see send_data_request()
 * @see x_plane_socket
 * @see data_buffer_packets
 * @return
 */
uint8_t init_sources(){
    if (pthread_mutex_init(&x_lock, NULL) != 0) {
        throw_err(4, 1);
        return 0;
    }

    //@TODO move code to simulationInput

    //In case of DATA_DELAY_SECONDS is zero
    if(max_data_idx <= 0){
        //Doesn't require sync -> thread start below
        max_data_idx = NUMBER_OF_DATA_INPUTS*DEFAULT_DATA_FREQ;
        max_packet_number = DEFAULT_DATA_FREQ;
    }

    data_buffer_packets = calloc(max_data_idx, sizeof(float));
    current_data_buffer = calloc(NUMBER_OF_DATA_INPUTS, sizeof(float));

    x_plane_socket = create_inet_dgram_socket(LIBSOCKET_IPv4, 0);

    //------------------------------------------------
    send_data_request(DEFAULT_DATA_FREQ, ENG_1_THR);
    send_data_request(DEFAULT_DATA_FREQ, ENG_1_EGT);
    send_data_request(DEFAULT_DATA_FREQ,MAG_VAR);
    send_status_frame_request();
    //------------------------------------------------

    //Start new thread and let him collect input in the background
    pthread_create(&data_input,NULL,&th_receive_data,NULL);

    return 1;
}

/**
 * @brief Sends data request packet to x-plane
 * @see send_RREF()
 * @see x_dataref_request_t
 * @param freq
 * @param datarefID
 */
void send_data_request(uint8_t freq, const uint8_t datarefID) {
    x_dataref_request_t* req = calloc(1,sizeof(struct x_dataref_request));
    req->freq = freq;
    req->index = datarefID;
    send_RREF(req);
}

/**
 * @brief Request data required for status_frame
 * @see status_frame
 * @see send_data_request
 * @see DEFAULT_DATA_FREQ
 */
void send_status_frame_request(){
    send_data_request(DEFAULT_DATA_FREQ, 2);
    send_data_request(DEFAULT_DATA_FREQ, 3);
    send_data_request(DEFAULT_DATA_FREQ, 4);
    send_data_request(DEFAULT_DATA_FREQ, 5);
    send_data_request(DEFAULT_DATA_FREQ, 6);
    send_data_request(DEFAULT_DATA_FREQ, 7);
    send_data_request(DEFAULT_DATA_FREQ, 8);
    send_data_request(DEFAULT_DATA_FREQ, 9);
    send_data_request(DEFAULT_DATA_FREQ, 10);
    send_data_request(DEFAULT_DATA_FREQ, 11);
    send_data_request(DEFAULT_DATA_FREQ, 12);
    send_data_request(DEFAULT_DATA_FREQ, 13);
    send_data_request(DEFAULT_DATA_FREQ, 14);
    send_data_request(DEFAULT_DATA_FREQ, 15);

}

/**
 * @brief Get data from local buffer, data can be delayed by DATA_DELAY_SECONDS seconds
 * @param dataIndex
 * @param delayed
 * @return
 */
float get_data(uint8_t dataIndex, int delayed) {
    float  tmpOut = -1;
    pthread_mutex_lock(&x_lock);
    int tmpIdx = dataIndex + get_delayed_index(delayed);
        if(tmpIdx < max_data_idx){
            tmpOut = data_buffer_packets[tmpIdx];
        }
    pthread_mutex_unlock(&x_lock);
    return tmpOut;
}

/**
 * @brief Returns pointer to the status_frame with current data
 * @param old_frame
 * @param delayed
 * @return
 */
status_frame* get_status_frame(void* old_frame, int delayed) {
    status_frame* frame = alloc_frame();
    if(old_frame != NULL){
        free(old_frame);//Release old frame
    }
    pthread_mutex_lock(&x_lock);
        int idxOffset = get_delayed_index(delayed);
//        printf("<< %d | %d >>\n", idxOffset, packet_number);
        frame->alt   = data_buffer_packets[7 + idxOffset];
        frame->gs    = data_buffer_packets[6 + idxOffset];
        frame->hdg   = data_buffer_packets[4 + idxOffset];
        frame->ias   = data_buffer_packets[5 + idxOffset];
        frame->lat   = data_buffer_packets[2 + idxOffset];
        frame->lng   = data_buffer_packets[3 + idxOffset];
        frame->vsi   = data_buffer_packets[11 + idxOffset];
        frame->pitch = data_buffer_packets[12 + idxOffset];
        frame->roll  = data_buffer_packets[13 + idxOffset];
        frame->pitch_ratio = data_buffer_packets[9 + idxOffset];
        frame->roll_ratio = data_buffer_packets[8 + idxOffset];
        frame->yaw_ratio = data_buffer_packets[10 + idxOffset];
    pthread_mutex_unlock(&x_lock);

    return frame;
}

/**
 * @brief Gets index for data buffer delayed X number of seconds
 * not thread safe
 * @return
 */
static inline uint16_t get_delayed_index(int delayed) {
    int tmpIdx;
    if(delayed){
       tmpIdx = (((packet_number+1)%max_packet_number)*NUMBER_OF_DATA_INPUTS);
    }else{
        if((packet_number-1) < 0){
            tmpIdx = (max_packet_number-1)*NUMBER_OF_DATA_INPUTS;
        }else{
            tmpIdx = (((packet_number-1)%max_packet_number)*NUMBER_OF_DATA_INPUTS);
        }

    }
    return tmpIdx;
}



/**
 * @brief Sets data delay by X number of packets
 * @param packets
 * @deprecated
 */
void set_data_delay(int packets) {
    delay_by_x_packets = packets;
}


//Data thread runnable
/**
 * @brief Receive in separate thread and update global buffer
 * @param a
 * @return
 */
void* th_receive_data(void * a) {
    int sfd ;
    char buf[1500]={0};
    int ret;

    printf("[Collecting data started...]\n");

    ret = sfd = x_plane_socket;

    if (sfd < 0) {
        printf("Thr err");
        exit(1);
    }

    x_rref_received_t* rec_data = calloc(1,sizeof(x_rref_received_t));

    while(data_thread_run){
        //1500 - receiving max MTU

        #if DEBUG >=2
            printf("{%i}",sfd);
        #endif
        ret = recvfrom_inet_dgram_socket(sfd, buf, 1500, 0, 0, 0, 0, 0,
                                         LIBSOCKET_NUMERIC);

        if (ret < 0) {
            printf("Thr err");
            exit(1);
        }else if(ret == 0){
            break;//EOF received
        }

        int p_offset = 5;//First integer value
        while(p_offset < ret){

            memcpy(rec_data,(buf+p_offset),sizeof(x_rref_received_t));

            pthread_mutex_lock(&x_lock);
                if(rec_data->index < max_data_idx){
                    data_buffer_packets[rec_data->index + ((packet_number) * NUMBER_OF_DATA_INPUTS)] = rec_data->value;
//                    current_data_buffer[rec_data->index] = rec_data->value;
                }
            pthread_mutex_unlock(&x_lock);

            p_offset += 8;
        }
        pthread_mutex_lock(&x_lock);
        packet_number = (packet_number+1)%max_packet_number;
        pthread_mutex_unlock(&x_lock);

        #if DEBUG >=2
            printf("[THRO: %.3f| ",data_buffer_packets[0+ ((packet_number)*NUMBER_OF_DATA_INPUTS)]);
            printf("EGT: %.3f| ",data_buffer_packets[1+ ((packet_number)*NUMBER_OF_DATA_INPUTS)]);
            printf("LAT: %.3f| ",data_buffer_packets[2+ ((packet_number)*NUMBER_OF_DATA_INPUTS)]);
            printf("LNG: %.3f| ",data_buffer_packets[3+ ((packet_number)*NUMBER_OF_DATA_INPUTS)]);
            printf("HDG: %.3f| ",data_buffer_packets[4+ ((packet_number)*NUMBER_OF_DATA_INPUTS)]);
            printf("IAS: %.3f| ",data_buffer_packets[5+ ((packet_number)*NUMBER_OF_DATA_INPUTS)]);
            printf("GS: %.3f| ",data_buffer_packets[6+ ((packet_number)*NUMBER_OF_DATA_INPUTS)]);
            printf("ALT: %.3f| ",data_buffer_packets[7+ ((packet_number)*NUMBER_OF_DATA_INPUTS)]);
            printf("ROLL RATIO: %.3f| ",data_buffer_packets[8+ ((packet_number)*NUMBER_OF_DATA_INPUTS)]);
            printf("PITCH RATIO: %.3f| ",data_buffer_packets[9+ ((packet_number)*NUMBER_OF_DATA_INPUTS)]);
            printf("HDG RATIO: %.3f| ",data_buffer_packets[10+ ((packet_number)*NUMBER_OF_DATA_INPUTS)]);
            printf("VSI: %.3f| ",data_buffer_packets[11+ ((packet_number)*NUMBER_OF_DATA_INPUTS)]);
            printf("PITCH: %.3f| ",data_buffer_packets[12+ ((packet_number)*NUMBER_OF_DATA_INPUTS)]);
            printf("ROLL: %.3f| ",data_buffer_packets[13+ ((packet_number)*NUMBER_OF_DATA_INPUTS)]);
            printf("]\n");
        #endif
    }

    free(rec_data);

    return NULL;
}
