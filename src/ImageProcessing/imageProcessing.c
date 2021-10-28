//
// Created by Maciek Malik
// takes images from cameras and has access to parsed map buffers
// listens for net packed with request

#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include "imageProcessing.h"
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "../inc/libinetsocket.h"
#include "../Maps/maps.h"
#include "../Maps/terrain.h"
#include "../Util/message.h"
#include "../Util/time_diff.h"
//#include "cvParseFrameStrategy.h"
#include "imageProcessingStrategy.h"

//Local threads declarations ---------
void *th_request_manager(void *a);

void *th_main_worker(void *a);

void *th_terrain_worker(void *a);

pthread_t request_manager;
pthread_t main_worker;
pthread_t terrain_worker;
//------------------------------------

/**
 * Condition for th_main_worker to run
 * @see th_main_worker()
 */
uint8_t worker_run = 1;

/**
 * Condition for th_request_manager to run
 * @see th_request_manager()
 */
uint8_t request_manager_run = 1;

/**
 * Condition for th_terrain_worker to run
 * @see th_terrain_worker()
 */
uint8_t terrain_worker_run = 1;

//static status_frame position_data[2];
status_frame *position_data;

/**
 * @brief array holding ip address of the sender. Used mainly for communicating with FlightManager module.
 * @see serv_soc
 */
char fm_host[128] = {0};

/**
 * @brief port of the sender (FlightManager)
 * @see serv_soc
 * @see fm_host
 */
char fm_service[7] = {0};

/**
 * @brief UDP server socket (id/number) bound to port defined in IMG_PROC_MODULE_PORT
 * Used for receiving status data from FM with ability to respond.
 * @see th_request_manager()
 */
int serv_soc = 0;

/**
 * @brief Determines if collision advisory should be sent,
 * by default it forces send_collision_advisory to skip reports.
 * It is needed at the beginning to let the new data from the x-plane get read and send here, to imageProcessing.
 * After that proper collision detection is possible
 * @see send_collision_advisory()
 * @see th_request_manager()
 * @see th_image_data_send()
 */
uint8_t skip_collision_adv = 1;


//Mutexes declarations --------------------
/**
 * @brief Mutex protecting fm_host and fm_service
 * @see fm_host
 * @see fm_service
 */
pthread_mutex_t fd_host_service_lock;

/**
 * @brief Mutex protecting position_data array
 * @see position_data
 */
pthread_mutex_t fd_position_data_lock;


/**
 * @brief Mutex synchronizing receiving data with detecting collisons
 */
pthread_mutex_t collision_adv_lock;

//-----------------------------------------

#include "../Util/route_utils.h"


TESTING_REQ_WEAK
int main() {

//@TODO move to tests
//    deg2Rad(232);
//    position_t  tmpPOS = point_from_dist_brg(53.1112,20.665,28000,57);
//    printf("POSlat: %f POSlng: %f", rad2Deg(tmpPOS.lat), rad2Deg(tmpPOS.lng));//POSlat: 53.247824 POSlng: 21.017945

    init_timer();

#if ENABLE_MAPS
    if (!maps_init()) {
        throw_err(6, 1);
    }
#endif

    status_frame *tmp = calloc(2, sizeof(status_frame));//Make space for current and old position
    if (tmp != NULL) {
        position_data = tmp;
    }

    if (pthread_mutex_init(&fd_host_service_lock, NULL) != 0) {
        throw_err(4, 1);
        return -1;
    }
    if (pthread_mutex_init(&fd_position_data_lock, NULL) != 0) {
        throw_err(4, 1);
        return -1;
    }

    if (pthread_mutex_init(&collision_adv_lock, NULL) != 0) {
        throw_err(4, 1);
        return -1;
    }


    pthread_create(&request_manager, NULL, &th_request_manager, NULL);
    pthread_setname_np(request_manager, "request_mg");

    pthread_create(&main_worker, NULL, &th_main_worker, NULL);
    pthread_setname_np(main_worker, "img_main_worker");

    pthread_create(&terrain_worker, NULL, &th_terrain_worker, NULL);
    pthread_setname_np(terrain_worker,"terrain_worker");

    //Wait for threads before exiting
    pthread_join(request_manager, NULL);
    pthread_join(main_worker, NULL);

    print_message_local(2);
    return 0;
}

/**
 * @brief Waits for data request and then sends it back or receives current flight data and saves it
 * @see request_manager
 * @param a
 */
void *th_request_manager(void *a) {

    char fm_host_tmp[127] = {0};
    char fm_service_tmp[6] = {0};

    serv_soc = create_inet_server_socket("0.0.0.0", IMG_PROC_MODULE_PORT, LIBSOCKET_UDP, LIBSOCKET_IPv4, 0);

    if (serv_soc < 0) {
        throw_err(2, 1);
    }

    printf("\nWaiting for FM data\n");

    char rec_data[60];
    module_request_t req = {0};

    while (request_manager_run) {
        memset(rec_data, 0, sizeof(rec_data));
        int rec = (int) recvfrom_inet_dgram_socket(serv_soc, rec_data, 60, fm_host_tmp, 127, fm_service_tmp, 6, 0,
                                             LIBSOCKET_NUMERIC);

        //Separate temp variables required - recvfrom is blocking therefore mutexes would block send_collision_advisory
        pthread_mutex_lock(&fd_host_service_lock);
        memcpy(fm_host, fm_host_tmp, 127);
        memcpy(fm_service, fm_service_tmp, 6);
        pthread_mutex_unlock(&fd_host_service_lock);
        if (rec < 0) {
            pthread_mutex_lock(&collision_adv_lock);
            skip_collision_adv = 1;
            pthread_mutex_unlock(&collision_adv_lock);
            continue;
        }
        memcpy(&req, rec_data, sizeof(module_request_t));//Save it to the structure

        pthread_mutex_lock(&fd_position_data_lock);
        memmove(&position_data[1], position_data, sizeof(status_frame));
        //memcpy(&position_data[1], &position_data[0], sizeof(status_frame));//Save prev position , memmove_avx_unaligned_erms = SEGFAULT
        pthread_mutex_unlock(&fd_position_data_lock);
        if (strcmp(req.cmd, "POS") == 0) {

            if(req.position.lat != 0 && req.position.lng != 0){
                pthread_mutex_lock(&collision_adv_lock);
                skip_collision_adv = 0;
                pthread_mutex_unlock(&collision_adv_lock);
            }

#if DEBUG
            //pthread_mutex_lock(&fd_host_service_lock);
            pthread_mutex_lock(&fd_position_data_lock);
            printf("[POS]Received data from: %s:%s DATA: %f %f\n", fm_host, fm_service, req.position.lat,
                   req.position.lng);
            pthread_mutex_unlock(&fd_position_data_lock);
            //pthread_mutex_unlock(&fd_host_service_lock);
#endif
            pthread_mutex_lock(&fd_position_data_lock);
            position_data[0].lng = req.position.lng;
            position_data[0].lat = req.position.lat;
            //Position update from FM
            pthread_mutex_unlock(&fd_position_data_lock);

            //printf("%f %f",position_data[0].y,position_data[1].y);
        } else if (strcmp(req.cmd, "STAT") == 0) {

            if(req.status.lat != 0 && req.status.ias != 0 && req.status.pitch != 0){
                pthread_mutex_lock(&collision_adv_lock);
                skip_collision_adv = 0;
                pthread_mutex_unlock(&collision_adv_lock);
            }

            //Status frame
            pthread_mutex_lock(&fd_position_data_lock);
            memcpy(&position_data[0], &req.status, sizeof(status_frame));
            pthread_mutex_unlock(&fd_position_data_lock);
#if DEBUG
            //pthread_mutex_lock(&fd_host_service_lock);
            pthread_mutex_lock(&fd_position_data_lock);
            printf("[STAT]Received data from: %s:%s DATA: %f %f\n", fm_host, fm_service, position_data[0].roll,
                   position_data[0].pitch);
            pthread_mutex_unlock(&fd_position_data_lock);
            //pthread_mutex_unlock(&fd_host_service_lock);
#endif

        }

//        //pthread_mutex_lock(&fd_host_service_lock);
//            sendto_inet_dgram_socket(serv_soc,"TY",5,fm_host,fm_service,0);
//        //pthread_mutex_unlock(&fd_host_service_lock);

    }


    return NULL;
}


/**
 * @brief Analyze terrain ahead and send collision advisory if necessary
 * @param a
 */
void *th_terrain_worker(void *a) {

    //while (!are_maps_loaded || skip_collision_adv) {
    while (!are_maps_loaded || skip_collision_adv) {
        usleep(10000);//Wait for maps to be loaded and first status data to arrive (get_status_frame needs to return any useful values)
    }

    while (terrain_worker_run) {
        #if !DISABLE_TERRAIN_ANALYSIS
            analyze_terrain();
        #endif
        usleep(250000);//~4Hz @TODO change usleep to custom time delay
    }

    return NULL;
}


/**
 * @brief Analyzes current photo frame and calculates future altitude over ground to avoid collision
 * in case of too low altitude, advisory message is sent to Flight manager to change height
 * @see main_worker
 * @param a
 */
void *th_main_worker(void *a) {

    test_open_cv();//Should return oCV version number
//    imageProcessingStrategy strategy = cvParseTestStrategy;
    imageProcessingStrategy strategy = cvParseStrategy;

    while (worker_run) {
        print_message_local(5);//waiting for video...
        getStream(strategy);//If collision is detected, call send_collision_advisory()
        usleep(250000);//~4Hz
    }

    return NULL;
}

/**
 * @brief Returns structure with current position
 * @return position_t
 */
position_t get_position_data() {
    position_t out;
    pthread_mutex_lock(&fd_position_data_lock);
    out.lng = position_data[0].lng;
    out.lat = position_data[0].lat;
    pthread_mutex_unlock(&fd_position_data_lock);
    return out;
}

/**
 * @brief Returns status_frame
 * @see status_frame
 * @return
 */
status_frame get_status_frame() {
    pthread_mutex_lock(&fd_position_data_lock);
    //B-81 points here (GDB SegFault)
    status_frame f = position_data[0];
    pthread_mutex_unlock(&fd_position_data_lock);
    return f;
}

/**
 * @brief Sends packet to FM when collision is detected
 */
void send_collision_advisory(collision_packet *p) {
    pthread_mutex_lock(&collision_adv_lock);
    if (skip_collision_adv) return;//Wait for status data to arrive first
    pthread_mutex_unlock(&collision_adv_lock);

#if DEBUG > 0
    printf("Sending info about collision\n");
#endif
    //Assuming not more than 60 sections, using static buffer to avoid calling malloc every time. Trading memory for cpu time
    static uint8_t collision_packet_tmp_buffer[1500];

    //Clear out previous buffer contents
    memset(collision_packet_tmp_buffer, 0, 1500);
    size_t memsize;

    if(p->type == C_BUILDING){


        if (p->sections_size > 60) {
            perror("Sections size too big :/");
            //exit(-66);
        }

        //Copy on the first place collision_packet
        memcpy(collision_packet_tmp_buffer, p, sizeof(collision_packet));

        printf(" ptr: %p", (uint8_t *) &collision_packet_tmp_buffer);
        printf(" ptr: %p", ((uint8_t *) collision_packet_tmp_buffer) + sizeof(collision_packet));
        printf(" ptr: %p\n", (collision_packet_tmp_buffer) + sizeof(collision_packet));


        size_t max_section_size = ( (1500 - sizeof(collision_packet))/sizeof(obstacle_distance_ang_t));
        if(p->sections_size > max_section_size ){//Max number of sections (~61)
            memsize = sizeof(obstacle_distance_ang_t) * max_section_size;
        }else{
            memsize = sizeof(obstacle_distance_ang_t) * (p->sections_size);
        }

        if(p->sections != NULL){
            //and after that obstacle_distance_ang_t array
            memcpy(((uint8_t *) collision_packet_tmp_buffer) + sizeof(collision_packet), p->sections,
                   memsize);
        }

    }else if(p->type == C_TERRAIN){
        memcpy(collision_packet_tmp_buffer, p, sizeof(collision_packet));
        size_t max_section_size = ( (1500 - sizeof(collision_packet))/sizeof(obstacle_terrain));
        if(p->sections_size > max_section_size ){//Max number of sections (~123)
            memsize = sizeof(obstacle_terrain) * max_section_size;
        }else{
            memsize = sizeof(obstacle_terrain) * (p->sections_size);
        }

        if(p->terrain != NULL){
            memcpy(((uint8_t *) collision_packet_tmp_buffer) + sizeof(collision_packet), p->terrain,
                   memsize );
        }

    }

    pthread_mutex_lock(&fd_host_service_lock);
    int send_ret = (int) sendto_inet_dgram_socket(serv_soc, collision_packet_tmp_buffer, sizeof(collision_packet_tmp_buffer),
                                            fm_host, fm_service, 0);
    pthread_mutex_unlock(&fd_host_service_lock);
    if (send_ret < 0) {
        perror(0);
        exit(1);
    }

    if(p->type == C_BUILDING){
        free(p->sections);
    }else if(p->type == C_TERRAIN){
        free(p->terrain);
    }

}











