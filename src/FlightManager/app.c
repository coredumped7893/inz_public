//
// Created by Maciek Malik
//

#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include "app.h"
#include "../DataInput/xPlane.h"
#include "../FlightManager/route.h"
#include "../DataInput/dataInput.h"
#include "main.h"
#include <stdlib.h>
#include <unistd.h>
#include "PID.h"
#include <math.h>
#include <errno.h>
#include "../config.h"
#include "string.h"
#include "../Util/route_utils.h"


pthread_t flight_manager;
pthread_t route_manager;
pthread_t img_proc_send;
pthread_t img_proc_rec;
pthread_mutex_t fd_lock;
pid_initial_targets initial_pid_targets;

void *th_image_data_send(void *a);

void *th_image_data_reccmd(void *a);

void *th_flight_manager(void *a);

void *th_route_manager(void *a);

int control_loop_run = 1;
int img_proc_soc = 0;
int run_rec = 1;
int run_send = 1;

/**
 * @brief Aircraft config set inside startAll, required for drone control in flight
 */
aircraftConfig_t aircraft_config;
control_strategy_t control_strategy;

/**
 * @brief Initialization of the required structures
 * aircraft config, control strategy, data inputs, route
 * @return
 */
int startAll() {

    printConfig();

    //Set Aircraft config struct
    aircraft_config = prk2;

    //Create aircraft control context
    control_strategy = create_control_simulated_context();


    //Setup initial collision response strategy
    //preferred_collision_strategy =

    printf("\n\nLoaded config for: %s\n\n", aircraft_config.name);
    //--------------------------

    if (aircraft_config.engine_count == 0 || control_strategy.set_throttle == NULL) return 0;
    if (!init_sources()) return 0;
    if (!route_init()) return 0;


    strategy_selection tmp_buildings = {collision_b_squeeze_strategy, collision_return_strategy, collision_b_reset_squeeze,collision_reset_return_strategy};
    strategy_selection tmp_terrain = {collision_t_over_strategy, collision_return_strategy,collision_t_reset_over,collision_reset_return_strategy};
    if (!create_collision_context(tmp_buildings, tmp_terrain)) return 0;
    return 1;

}

/*
 * @brief Allocates memory for structure and return its pointer if succeeded
 *
 */
status_frame *alloc_frame() {

    status_frame *tmpF = calloc(1, sizeof(status_frame));
    if (tmpF == NULL) {
        throw_err(3, 1);
    }
    return tmpF;

}

void clean_up() {

    int ret = destroy_inet_socket(x_plane_socket);

    if (ret < 0) {
        perror(0);
        exit(1);
    }

}

/**
 * @brief Returns currently loaded aircraft config
 */
aircraftConfig_t* get_aircraft_config(){
    return &aircraft_config;
}


/**
 * @brief Method executed by main function after startAll()
 * @see startAll()
 */
void run() {

    //Run PID thread
    printf("Running...\n");

    if (pthread_mutex_init(&fd_lock, NULL) != 0) {
        throw_err(4, 1);
        return;
    }

#if ENABLE_PROFILER
    start_profiler(FLIGHTMANAGER);
#endif

    pthread_create(&flight_manager, NULL, &th_flight_manager, NULL);
    pthread_setname_np(flight_manager,"flight_manager");

    pthread_create(&route_manager, NULL, &th_route_manager, NULL);
    pthread_setname_np(route_manager, "route_manager");

    //img_proc_soc = create_inet_stream_socket(IMG_PROC_MODULE_IP,IMG_PROC_MODULE_PORT,LIBSOCKET_IPv4,0);//Connect to ImgProc
    img_proc_soc = create_inet_dgram_socket(LIBSOCKET_IPv4, 0);
    if (img_proc_soc < 0) {
        throw_err(9, 1);
    }

    pthread_create(&img_proc_send, NULL, &th_image_data_send, NULL);
    pthread_setname_np(img_proc_send, "img_proc_send");

    pthread_create(&img_proc_rec, NULL, &th_image_data_reccmd, NULL);
    pthread_setname_np(img_proc_rec,"img_proc_rec");

    //Wait for all to finish
    pthread_join(route_manager, NULL);
    pthread_join(data_input, NULL);
    pthread_join(flight_manager, NULL);

}

/**
 * @brief Returns pointer to the status frame
 * @see status_frame
 * @param old
 * @param delayed
 * @return
 */
status_frame *get_status(void *old, int delayed) {
    return get_status_frame(old, delayed);
}


double get_target_alt() {
    return aircraft_config.input_fd_pitch.target;
}

double get_target_hdg() {
    return aircraft_config.input_fd_roll.target;
}

double get_target_ias() {
    return aircraft_config.input_throttle.target;
}

/**
 * @brief Thread function to send actual plane position coords to Image processing module
 * @param a
 * @return
 */
void *th_image_data_send(void *a) {

    while (run_send) {
        //If we got disconnected
#if DEBUG > 1
        printf("[%i,%i]",img_proc_soc,errno);
#endif
        module_request_t req;
        status_frame *sf = get_status(sf, 1);
        memcpy(req.cmd, "STAT", 4);//Position report
        memcpy(&req.status, sf, sizeof(status_frame));
        int ret = -1;

        ret = sendto_inet_dgram_socket(img_proc_soc, &req, sizeof(req), IMG_PROC_MODULE_IP, IMG_PROC_MODULE_PORT, 0);

        if (ret <= 0) {
            printf("No data sent (Image Processing)\n");
        }

        usleep(250000);//~4Hz
    }

    return NULL;
}

/**
 * @brief Thread function to receive data from Image processing module
 * @param a not used
 * @return
 */
void *th_image_data_reccmd(void *a) {

    uint8_t buff[1500] = {0};//Max MTU

    while (run_rec) {
        if (img_proc_soc <= 0) continue;
        printf("<>");
        //int r = recv(img_proc_soc,buff,255,0);
        int r = (int) recvfrom_inet_dgram_socket(img_proc_soc, buff, 1500, 0, 0, 0, 0, 0, LIBSOCKET_NUMERIC);
        collision_packet *p = (collision_packet *) buff;

        //int sec_number =  r / sizeof(obstacle_distance_ang_t);
        if (r > 0) {

            if(p->type == C_BUILDING){

                obstacle_distance_ang_t *sections = (obstacle_distance_ang_t *) (((uint8_t *) &buff) + sizeof(collision_packet));
                p->sections = sections;

                if(p->sections_size <= 0){
                    printf("FROM IMGPROC[BLDG]: | sections number: %d start: ??? end: ??? deg: ??? (%i)|\n", p->sections_size,r);
                }else{
                    printf("FROM IMGPROC[BLDG]: | sections number: %d start: %i end: %i deg: %f (%i)|\n", p->sections_size,
                           sections[0].startX, sections[0].endX, sections[0].distanceDeg, r);
                    set_building_sections(sections, p->sections_size, p->size);//Set received sections data
                }

            }else if(p->type == C_TERRAIN){
                //set terrain map

                obstacle_terrain *terr_sections = (obstacle_terrain *) (((uint8_t *) &buff) + sizeof(collision_packet));
                p->terrain = terr_sections;


                if(p->sections_size <=0){
                    printf("FROM IMGPROC[TERR]: | sections number: %d deltaALT: ???\n", p->sections_size);
                }else{
                    printf("FROM IMGPROC[TERR]: | sections number: %d deltaALT: %f\n", p->sections_size,
                           terr_sections[0].dHeight);

                    set_terrain_sections(terr_sections, p->sections_size, p->size);
                }

            }

            //start collision avoidance
            set_collision_status(1, p->type);//This should start preferred_collision_strategy
            //-------------------------

        }
    }

    return NULL;
}


/**
 * @brief Control values for all axis
 * @param a not used
 * @return nothing
 */
void *th_flight_manager(void *a) {

    controller_init(&aircraft_config.roll);
    controller_init(&aircraft_config.pitch);
    controller_init(&aircraft_config.FD_roll);
    controller_init(&aircraft_config.FD_pitch);
    controller_init(&aircraft_config.throttle);

    //Save initial values of the PID controller
    initial_pid_targets.hdg = aircraft_config.input_fd_roll.target;
    initial_pid_targets.ias = aircraft_config.input_throttle.target;
    initial_pid_targets.alt = aircraft_config.input_fd_pitch.target;


    aircraft_config.input_fd_roll.isHeading = 1;

    FILE *outLog;

    //Logging to file for the first 300 number of cycles
    outLog = fopen("../outLog.csv", "w");

    if (outLog == NULL) {
        printf("File error");
        return NULL;
    }

    uint32_t counter = 0;
    uint32_t counter2 = 0;
    uint8_t disable_delay = 0;

    control_strategy.send_data(1.0f, 17);//Override flight director
    usleep(100000);//Wait ~0.1 sec.

    uint8_t reduced_FD = 0;
    uint8_t reduced_ailrn = 0;
    uint8_t extended_pitch = 0;
    uint8_t extended_roll = 0;

    while (control_loop_run && counter < 300) {

        aircraft_config.input_roll.measurement = get_data(ROLL, 0);
        aircraft_config.input_pitch.measurement = get_data(PITCH, 0);
        aircraft_config.input_fd_roll.measurement = get_data(HEADING, 0);//Magnetic heading
        aircraft_config.input_fd_pitch.measurement = get_data(ALT, 0);//Altitude MSL
        aircraft_config.input_throttle.measurement = get_data(IAS, 0);


        /*@TODO Move to tests
        printf("%i\n",min_heading_error(1,20));//-65
        printf("%i\n",min_heading_error(1,315));//+65
        printf("%i\n",min_heading_error(1,355));//+100
        printf("%i\n",min_heading_error(1,330));//+100
       printf("%i\n",min_heading_error(1,2));//-7
       exit(1);*/

        pthread_mutex_lock(&fd_lock);
        int heading_error_tmp = min_heading_error((int) aircraft_config.input_fd_roll.target,
                                                  (int) aircraft_config.input_fd_roll.measurement);
//        printf("<heading_error_tmp: %d>", heading_error_tmp);
        if (aircraft_config.adjust_limit_to_error) {

            uint8_t collistion_status = get_collision_status();
            //Only when there is no risk in colliding
            if ((abs(heading_error_tmp) < 20 && !reduced_FD) && !collistion_status) {
                aircraft_config.FD_roll.limMax *= 0.25f;
                aircraft_config.FD_roll.limMin *= 0.25f;
                //            aircraft_config.FD_roll.Kp *= 0.25;
                //            aircraft_config.FD_roll.Ki *= 0.25;
                //            aircraft_config.roll.Kp *= 0.25;
                reduced_FD = 1;
            } else if ((abs(heading_error_tmp) >= 20 && reduced_FD) && !collistion_status) {
                aircraft_config.FD_roll.limMax *= 4;
                aircraft_config.FD_roll.limMin *= 4;
                //           aircraft_config.FD_roll.Kp *= 4;
                //           aircraft_config.FD_roll.Ki *= 4;
                //           aircraft_config.roll.Kp *= 4;
                reduced_FD = 0;
            }



            //Limit ailrn movement to decrease yaw spikes
            if ((abs(heading_error_tmp) < 5 && !reduced_ailrn) && !collistion_status) {
                aircraft_config.roll.limMin *= 0.25f;
                aircraft_config.roll.limMax *= 0.25f;
                reduced_ailrn = 1;
            } else if ((abs(heading_error_tmp) >= 5 && reduced_ailrn) && !collistion_status) {
                aircraft_config.roll.limMin *= 4;
                aircraft_config.roll.limMax *= 4;
                reduced_ailrn = 0;
            }


        }

        //In case of terrain collision, increase the maximum pitch angle.
        if(get_collision_status() && (get_collision_type() & (1 << C_TERRAIN)) ){
            if(!extended_pitch){
                aircraft_config.FD_pitch.limMax *= 2;
                aircraft_config.input_throttle.target *=1.4f;
                extended_pitch = 1;
            }
        }else{
            if(extended_pitch){
                aircraft_config.FD_pitch.limMax *= 0.5f;
                aircraft_config.input_throttle.target *=0.714286f;
                extended_pitch = 0;
            }
        }

        //In case of building collision, increase the maximum roll angle.
        if(get_collision_status() && (get_collision_type() & (1 << C_BUILDING)) ){
            if(!extended_roll){
                aircraft_config.FD_roll.limMax *= 2;
                aircraft_config.FD_roll.Kp += 0.2;
                aircraft_config.roll.Kp *= 1.1;
                aircraft_config.roll.Ki += 0.2;
                extended_roll = 1;
            }
        }else{
            if(extended_roll){
                aircraft_config.FD_roll.limMax *= 0.5f;
                aircraft_config.FD_roll.Kp -= 0.2;
                aircraft_config.roll.Kp *= 0.9090909;
                aircraft_config.roll.Ki -= 0.2;
                extended_roll = 0;
            }
        }


        float FD_rollOut = controller_calc_frame(&aircraft_config.FD_roll, aircraft_config.input_fd_roll);
        float FD_pitchOut = controller_calc_frame(&aircraft_config.FD_pitch, aircraft_config.input_fd_pitch);

        //Fix for B-64 -------------------
        int d_heading = (int) fabsf(aircraft_config.input_fd_roll.target - aircraft_config.input_fd_roll.measurement);
        int d_short_heading = deg_from_north((int) aircraft_config.input_fd_roll.measurement) +
                              (int) deg_from_north((int) aircraft_config.input_fd_roll.target);
        //printf(" %d % d\n",d_heading,d_short_heading);
        if (d_short_heading < d_heading) {
            //Change turn direction - for shortest turn
            FD_rollOut *= -1;
        }

        pthread_mutex_unlock(&fd_lock);

        aircraft_config.input_roll.target = FD_rollOut;
        aircraft_config.input_pitch.target = FD_pitchOut;

        float rollOut = controller_calc_frame(&aircraft_config.roll, aircraft_config.input_roll);
        float pitchOut = controller_calc_frame(&aircraft_config.pitch, aircraft_config.input_pitch);
        float throttleOut = controller_calc_frame(&aircraft_config.throttle, aircraft_config.input_throttle);
//        printf("SPD: %f\n",  aircraft_config.input_throttle.target);
        //fprintf(outLog,"%i|%f|%f\n",counter,rollOut,inputRoll.measurement);
//        printf("%f(MES: %f TRG: %f) | %f \n",throttleOut,aircraft_config.input_throttle.measurement,aircraft_config.input_throttle.target,pitchOut);

        if (counter2 > 20 || disable_delay) {
            disable_delay = 1;
            //@TODO change to send_axis when finished
            control_strategy.send_data(rollOut, ROLL_INPUT);
            control_strategy.send_data(pitchOut, PITCH_INPUT);
            control_strategy.send_data(throttleOut, ENG_1_THR);
            if (aircraft_config.engine_count > 1) {
                control_strategy.send_data(throttleOut, ENG_2_THR);
            }
        }
        control_strategy.send_data(FD_rollOut, FD_ROLL);
        control_strategy.send_data(FD_pitchOut, FD_PITCH);

        //++counter;
        ++counter2;
        usleep(33000);//~30Hz
    }

    printf("Control finished\n");
    fclose(outLog);

    return NULL;
}


/**
 * @brief Thread to recalculate route headings, update point queue
 * @param a
 * @return
 */
void *th_route_manager(void *a) {

    queue_node *tmpTarget = route_get_target();//Gets first point from the route
    route_node_t oldNode = {.lng = 0, .lat = 0};
    uint8_t first = 1;
    //Our current closest route point
    queue_node *current_route_node;
    queue_node *next_route_node;
    route_node_t *current_route_node_point = NULL;
    route_node_t *next_route_node_point = NULL;

    //Wait for first data to arrive
    usleep(1000000);

    //queue_node* q = get_closest_point(52.2758f, 20.9567f);
    queue_node *q = tmpTarget;
    route_node_t *qn = (route_node_t *) q->data;

    printf("\n_____________\n\n");
    printf("CLOSEST: %f %f hdg to next: %f ID: %i\n", qn->lat, qn->lng, qn->hdg, qn->time);
    printf("_____________\n\n");

#if DUMP_CSV_TARGETING_MSG
    FILE *csv_dump = fopen("targeting_data.csv", "w");
#endif

    while (control_loop_run) {
        printf("->");
        fflush(stdout);

        //@TODO change it to return all three at once
        float mag_var = get_data(MAG_VAR, 0);
        float tmpLAT = get_data(LAT, 0);
        float tmpLNG = get_data(LNG, 0);
        #if ENABLE_WCA_CALCULATION
                float wind_dir = get_data(WIND_DIR,0);
                float wind_speed = get_data(WIND_SPEED,0);// m/s
                float tas = get_data(TAS,0);// m/s
        #endif

        if (oldNode.lng == 0) {
            //Set Curr position
            oldNode.lat = tmpLAT;
            oldNode.lng = tmpLNG;
        }

        if (first) {

            if (tmpTarget != NULL) {
                current_route_node = tmpTarget;
                current_route_node_point = (route_node_t *) current_route_node->data;
            } else {
                printf(" - Route finished/empty - \n");
                break;
            }

            next_route_node = current_route_node;
            next_route_node_point = current_route_node_point;

            //0.001 deg  = 111m
            if ((pow(FIRST_POINT_EPSILON, 2) >= (pow((tmpLAT - current_route_node_point->lat), 2) +
                                                 pow((tmpLNG - current_route_node_point->lng), 2)))) {
                //If close enough then switch to the next WP
                first = 0;
                printf("\n\n|| First point reached ||\n");
            }

        } else {

            /**
             * Get closest point on the route, and proceed directly to its next point
             */
            current_route_node = get_closest_point(tmpLAT, tmpLNG);
            current_route_node_point = (route_node_t *) current_route_node->data;


            if (current_route_node != NULL) {
                if (current_route_node->next != NULL) {
                    next_route_node = current_route_node->next;
                    next_route_node_point = (route_node_t *) next_route_node->data;
                }
            }

        }

        oldNode.lat = current_route_node_point->lat;
        oldNode.lng = current_route_node_point->lng;

        pthread_mutex_lock(&fd_lock);
        //Calculate heading to the next point based on current position and next planned point
        //Disable it when avoiding collision

        position_t current_tmp = {tmpLAT, tmpLNG};
//        position_t next_tmp = {next_route_node_point->lat, next_route_node_point->lng};
        collision_safe_params c_s_p = ca_avoid_obstacles(&current_tmp, next_route_node_point, mag_var);
        if (c_s_p.heading != -1 && c_s_p.heading != -2) {
            aircraft_config.input_fd_roll.target = c_s_p.heading;
        }else if(c_s_p.heading == -2){
        aircraft_config.input_fd_roll.target = (float) initial_pid_targets.hdg;
        #if !DISABLE_HEADING_FOLLOWING
            aircraft_config.input_fd_roll.target = degWrap360(get_target_heading(tmpLAT, tmpLNG, next_route_node_point->lat, next_route_node_point->lng) + mag_var);
        #endif
        }

        //Apply wind correction angle
        #if ENABLE_WCA_CALCULATION
            wind_layer_data_t w = {wind_dir,wind_speed};
            aircraft_config.input_fd_roll.target += wca_calculate(tas, w);
        #endif

        if (c_s_p.ias != -1 && c_s_p.ias != -2) {
            aircraft_config.input_throttle.target = c_s_p.ias;
        }else if(c_s_p.ias == -2){
            aircraft_config.input_throttle.target = (float) initial_pid_targets.ias;
        }

        if (c_s_p.alt != -1 && c_s_p.alt != -2) {
            aircraft_config.input_fd_pitch.target = c_s_p.alt;
        }else if(c_s_p.alt == -2){
            //Auto calculate it
        aircraft_config.input_fd_pitch.target = (float) initial_pid_targets.alt;
        #if !DISABLE_ALTITUDE_FOLLOWING
            aircraft_config.input_fd_pitch.target = next_route_node_point->alt / 3.281f;
        #endif
        }


        //Needed for collision avoidance, disables collision mode when needed
        update_status(aircraft_config.input_fd_roll, aircraft_config.input_fd_pitch);

        //aircraft_config.input_fd_roll.target = degWrap360(get_target_heading(tmpLAT, tmpLNG, next_route_node_point->lat, next_route_node_point->lng) + mag_var);

#if DUMP_CSV_TARGETING_MSG
        fprintf(csv_dump,"%f,%f\n",
                current_route_node_point->maxXTE,
                crossTrackErr(tmpLAT, tmpLNG, oldNode.lat, oldNode.lng, current_route_node_point->lat,current_route_node_point->lng)
                );
        fflush(csv_dump);
#endif

#if PRINT_TARGETING_MSG
        printf("Targeting: %f (Troll: %f) Closest point: %f %f ID: %i  ALT: %f MAG_VAR: %f dist: %f XTE: %f nm\n",
               aircraft_config.input_fd_roll.target,
               aircraft_config.input_fd_roll.target, current_route_node_point->lat, current_route_node_point->lng,
               current_route_node_point->time, (next_route_node_point->alt), mag_var, current_route_node_point->maxXTE,
               (crossTrackErr(tmpLAT, tmpLNG, oldNode.lat, oldNode.lng, current_route_node_point->lat,
                              current_route_node_point->lng) / 1000) / 1.852);
#endif
        pthread_mutex_unlock(&fd_lock);

        usleep(250000);//~4Hz
    }

    return NULL;
}

