//
// Created by Maciek Malik
//
#ifndef MM_route_H_
#define MM_route_H_

#include <stdint.h>
#include "../config.h"
#define _USE_MATH_DEFINES
#ifndef __USE_MISC
#define __USE_MISC
#endif
#include <math.h>



#include "../../inc/queue.h"

/**
 * Earth radius in meters
 */
#define E_RADIUS 6371000



uint8_t route_init();

/**
 * @brief Single node of the whole route
 * @see route_init()
 * @see route_load();
 * @see status_frame
 */
typedef struct {
    float lat;
    float lng;
    float ias;
    float alt;
    float hdg;
    float maxXTE;
    uint32_t time;
    uint8_t generated;
    /**
     * @deprecated
     */
    uint8_t disabled;
}route_node_t;

typedef struct {
    double direction;// [deg 0-359] from where wind is coming from
    double speed;// [m/s]
} wind_layer_data_t;

//extern stack_list_t _route;

float get_target_heading(float lat, float lng, float targetLAT, float targetLNG);
void route_next_point();//Selects next node on the route
void route_load(route_node_t *route, int size);
queue_node * route_get_target();
float crossTrackErr(float latPos, float lngPos,float latStart, float lngStart, float latEnd, float lngEnd);
float pointDistance(float latStart, float lngStart,float latEnd, float lngEnd);
queue_node* get_closest_point(float lat, float lng);
int deg_from_north(int deg);
queue_node get_first_point();
queue_list_t* get_route_ptr();
uint8_t is_at_route_start();
float wca_calculate(double tas_speed, wind_layer_data_t wind);









#endif //MM_route_H_

