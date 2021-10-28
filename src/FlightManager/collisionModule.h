//
// Created by Maciek Malik
//

#ifndef MM_collisionModule_H_
#define MM_collisionModule_H_

#include "stdint.h"
#include <pthread.h>
#include "collisionAvoidanceStrategy.h"
#include "PID.h"

/**
 * @brief Parameters of sections between buildings (safe space)
 */
typedef struct {
    /**
     * @brief Section start - position of the pixel on the X axis
     */
    int startX;

    /**
     * @brief Section start - position of the pixel on the Y axis
     */
    int startY;

    /**
     * @brief Section end - position of the pixel on the X axis
     */
    int endX;

    /**
     * @brief Section end - position of the pixel on the Y axis
     */
    int endY;

    /**
     * Angular width estimated based on SIM_FOV.
     * Using this value FlightManger calculates new heading to avoid crashing
     * @see SIM_FOV
     */
    double distanceDeg;
}obstacle_distance_ang_t;

/**
 * @brief Info about terrain collision.
 * It is sent to the FlightManager to take actions to avoid crash
 * @see set_terrain_sections()
 */
typedef struct{
    /**
     * @brief Delta height - difference between current altitude and terrain in front of the aircraft.
     */
    float dHeight;

    /**
     * @brief Distance to the nearest collision in meters
     */
    float distanceToTerrain;

    /**
     * @brief coordinates of a point where collision was found
     */
    position_t colliding_point;

    /**
     * @brief estimated time to collision
     */
    uint16_t time_to_terrain;

    /**
     * @brief Bearing on which measurement was taken in degrees
     */
    float bearing;
}obstacle_terrain;

/**
 * @brief Size of the frame that is received from the video stream for interpretation
 */
typedef struct {
    int x;//Width
    int y;//Height
}frame_size;

/**
 * @brief Structure containing data about upcoming collision.
 * obstacle_distance_ang_t or obstacle_terrain are sent with it to FM
 * @see obstacle_terrain
 * @see obstacle_distance_ang_t
 */
typedef struct{
    int type;
    int sections_size;//Number of sections in case of collision
    frame_size size;
    union {//Members needs to be dynamically allocated
        obstacle_distance_ang_t* sections;
        obstacle_terrain* terrain;
    };
}collision_packet;


enum COLLISION_TYPES{
    C_TERRAIN = 0,
    C_BUILDING = 1
};


/**
 * @brief Structure that holds strategy pointers for given event.
 * Holds pointer to the primary function and secondary default
 */
typedef struct {
    collisionStrategy primary;
    collisionStrategy fallback_default;
    collisionUpdateStatusStrategy update_status;
    collisionUpdateStatusStrategy fallback_update_status;
}strategy_selection;


//
//extern collisionStrategy preferred_collision_strategy;
//extern collisionStrategy fallback_collision_strategy;
extern pthread_mutex_t collision_data_lock;
extern pthread_mutex_t collision_data_status_lock;

//Collision avoiding strategies
collision_safe_params collision_return_strategy(queue_list_t* route, const position_t *current, const route_node_t *next, float magnetic_variation);
collision_safe_params collision_b_squeeze_strategy(queue_list_t* route, const position_t *current, const route_node_t *next, float magnetic_variation);
collision_safe_params collision_t_around_strategy(queue_list_t* route, const position_t *current, const route_node_t *next, float magnetic_variation);
collision_safe_params collision_t_over_strategy(queue_list_t* route, const position_t *current, const route_node_t *next, float magnetic_variation);
//------------------------------

//Collision update status strategies
void collision_reset_empty_strategy(pid_input_t roll, pid_input_t pitch);
void collision_b_reset_squeeze(pid_input_t roll, pid_input_t pitch);
void collision_t_reset_over(pid_input_t roll, pid_input_t pitch);
void collision_reset_return_strategy(pid_input_t roll, pid_input_t pitch);
//----------------------------------



//if collision is detected, provide route manager thread new headings to follow, else return value unchanged
collision_safe_params ca_avoid_obstacles(const position_t *current, const route_node_t *next, float magnetic_variation);


void set_context(strategy_selection strategy, enum COLLISION_TYPES type);
void set_building_sections(obstacle_distance_ang_t *sections, int sec_count, frame_size size);
void set_terrain_sections(obstacle_terrain *sections, int sec_count, frame_size size);
int get_section_count();
void invalidate_sections_data();
frame_size get_frame_size();

obstacle_distance_ang_t get_building_section(int id);
obstacle_distance_ang_t* get_building_sections();
obstacle_terrain* get_terrain_sections();
//Initializes initial strategy, mutexes
uint8_t create_collision_context(strategy_selection building_strat, strategy_selection terrain_strat);

uint8_t get_collision_status();//returns if collision was detected
uint8_t set_collision_status(int status, int type);//Used when received packet with collision
uint8_t get_collision_type();

void update_status( pid_input_t roll,  pid_input_t pitch);//Checks if current collision avoidance can be disabled and normal flight continued




#endif //MM_collisionModule_H_
