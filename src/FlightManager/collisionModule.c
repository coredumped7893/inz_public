//
// Created by Maciek Malik
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../DataInput/dataInput.h"
#include "../Util/message.h"
#include "../Util/other.h"
#include "collisionModule.h"
#include "route.h"
#include "../Util/route_utils.h"

#define SECTION_BUFFER_SIZE 10

/**
 * @brief Default collision strategy that is executed after receiving collision packet
 * @see send_collision_advisory()
 */
//collisionStrategy preferred_collision_strategy;

/**
 * @brief Fallback strategy that is not modifying heading when no collision is detected
 * or when some requirements were not met
 * @see collision_return_strategy()
 */
//collisionStrategy fallback_collision_strategy;

pthread_mutex_t collision_data_lock;
pthread_mutex_t collision_data_status_lock;


typedef struct {

    /**
     * @brief Buffer containing sections from the newest frame
     * @see obstacle_distance_ang_t
     */
    obstacle_distance_ang_t *building_section_buffer;
    int building_section_buffer_size;

    /**
     * @brief Array of obstacle_terrain structs with terrain collision data
     * @see obstacle_terrain
     */
    obstacle_terrain *terrain_section_buffer;
    int terrain_section_buffer_size;

    /**
    * @brief Holds recent frame size, used for calculating new heading after detecting new collision
    */
    frame_size f_size;

    uint8_t collision_avoidance_in_progress;
    int collision_type_in_progress;

    /**
    * @brief number of the sections received
    * @remark If section count is zero then data is considered invalid
    */
    int building_section_count;

    /**
     * @remark If section count is zero then data is considered invalid
     */
    int terrain_section_count;

    /**
     * @brief Determines if current section data is valid for calculations/interpretation
     * @see building_section_buffer
     * @see terrain_section_buffer
     */
    int section_data_invalid;

    time_t last_collision_started;
    time_t last_collision_ended;

    /**
     * @brief secondary timer value to use with resetting building collision strategy
     */
    time_t collision_secondary_timer_start;

    strategy_selection building;

    strategy_selection terrain;

    //Used only when SELECT_COLLISION_STRATEGY_DYNAMICALLY == 1
    uint8_t is_calculated;

}collision_ctx;

collision_ctx collision_context;
static void select_best_collision_strategy(collision_ctx* collision_context, const position_t *current, const route_node_t *next, float magnetic_variation);


/**
 * Initializes initial strategy and mutex for accessing section data
 * @param selected_strategy
 * @return
 */
uint8_t create_collision_context(strategy_selection building_strat, strategy_selection terrain_strat) {

    //Init mutexes
    if (pthread_mutex_init(&collision_data_lock, NULL) != 0) {
        throw_err(4, 0);
        return 0;
    }
    if (pthread_mutex_init(&collision_data_status_lock, NULL) != 0) {
        throw_err(4, 0);
        return 0;
    }

    collision_context.f_size.x = 0;
    collision_context.f_size.y = 0;

    collision_context.collision_avoidance_in_progress = 0;

    collision_context.building_section_count = 0;

    collision_context.terrain_section_count = 0;

    collision_context.section_data_invalid = 0;

    collision_context.is_calculated = 0;

    collision_context.collision_secondary_timer_start = 0;

    set_context(building_strat,C_BUILDING);
    set_context(terrain_strat,C_TERRAIN);

    collision_context.building_section_buffer_size = SECTION_BUFFER_SIZE;
    collision_context.terrain_section_buffer_size = SECTION_BUFFER_SIZE;

    obstacle_distance_ang_t *tmp_b_sec_b = calloc(1,sizeof(obstacle_distance_ang_t) * SECTION_BUFFER_SIZE);
    if(tmp_b_sec_b != NULL){
        collision_context.building_section_buffer = tmp_b_sec_b;
    }

    obstacle_terrain *tmp_terr_sec_b = calloc(1,sizeof(obstacle_terrain) * SECTION_BUFFER_SIZE);
    if(tmp_terr_sec_b != NULL){
        collision_context.terrain_section_buffer = tmp_terr_sec_b;
    }


    return 1;
}

collision_safe_params empty_collision_strategy(queue_list_t* route,const position_t *current, const route_node_t *next, float magnetic_variation){
    collision_safe_params ret = {-1,-1,-1};
    //Just to avoid crash in case pointer in strategy happens to be null
    return ret;
}



/**
 * @brief Get single section struct by value
 * @param id
 * @return
 */
obstacle_distance_ang_t get_building_section(int id) {
    pthread_mutex_lock(&collision_data_lock);
    obstacle_distance_ang_t result = {0,0,0,0,0};
    if(id < collision_context.building_section_count){
        result = collision_context.building_section_buffer[id];
    }
    pthread_mutex_unlock(&collision_data_lock);
    return result;
}

/**
 * @brief Returns sections size
 * @see set_building_sections
 * @return
 */
int get_section_count() {
    int tmp;
    pthread_mutex_lock(&collision_data_lock);
    tmp = collision_context.building_section_count;
    pthread_mutex_unlock(&collision_data_lock);
    return tmp;
}

/**
 * @brief Returns pointer to the terrain sections data
 * @return
 */
obstacle_terrain *get_terrain_sections() {
    pthread_mutex_lock(&collision_data_lock);
    obstacle_terrain* result = collision_context.terrain_section_buffer;
    pthread_mutex_unlock(&collision_data_lock);
    return result;
}


/**
 * @brief Returns pointer to the sections array (thread safe)
 * @return
 */
obstacle_distance_ang_t* get_building_sections() {
    pthread_mutex_lock(&collision_data_lock);
    obstacle_distance_ang_t* result = collision_context.building_section_buffer;
    pthread_mutex_unlock(&collision_data_lock);
    return result;
}

/**
 * @brief Sets building sections data from th_image_data_reccmd thread
 * data that is received from image processing module
 * thread safe
 * @see th_image_data_reccmd()
 * @param sections
 * @param sec_count
 */
void set_building_sections(obstacle_distance_ang_t *sections, int sec_count, frame_size size) {
    pthread_mutex_lock(&collision_data_lock);
    if(sec_count > collision_context.building_section_buffer_size){
        //Buffer is too small
        obstacle_distance_ang_t *tmpsec = realloc(collision_context.building_section_buffer,sec_count * sizeof(obstacle_distance_ang_t));
        if(tmpsec != NULL){
            collision_context.building_section_buffer = tmpsec;
            collision_context.building_section_buffer_size = sec_count;
        }
    }

    memcpy(collision_context.building_section_buffer, sections, sec_count * sizeof(obstacle_distance_ang_t));
    collision_context.building_section_count = sec_count;
    collision_context.f_size = size;
    pthread_mutex_unlock(&collision_data_lock);
}

/**
 * @brief Sets terrain sections data from th_image_data_reccmd thread
 * @param sections
 * @param sec_count
 * @param size
 */
void set_terrain_sections(obstacle_terrain *sections, int sec_count, frame_size size) {
    pthread_mutex_lock(&collision_data_lock);
    if(sec_count > collision_context.terrain_section_buffer_size){
        //Buffer is too small
        obstacle_terrain *tmpsec = realloc(collision_context.terrain_section_buffer,sec_count * sizeof(obstacle_terrain));
        if(tmpsec != NULL){
            collision_context.terrain_section_buffer = tmpsec;
            collision_context.terrain_section_buffer_size = sec_count;
        }
    }

    memcpy(collision_context.terrain_section_buffer, sections, sec_count * sizeof(obstacle_terrain));
    collision_context.terrain_section_count = sec_count;
    collision_context.f_size.x = 0;
    collision_context.f_size.y = 0;
    pthread_mutex_unlock(&collision_data_lock);
}

/**
 * @brief Invalidates sections data to force strategy to wait for a new data frame
 * works for building and terrain collisions
 */
void invalidate_sections_data() {
    pthread_mutex_lock(&collision_data_lock);
    collision_context.building_section_count = 0;
    collision_context.terrain_section_count = 0;
    collision_context.f_size.y = 0;
    collision_context.f_size.x = 0;
    collision_context.section_data_invalid = 1;
    pthread_mutex_unlock(&collision_data_lock);
}


/**
 * @brief Middleware for route controller, returns unchanged heading if no collision is detected
 * or new safe heading - executes strategy that performs all the calculations
 * @param current
 * @param next
 * @param magnetic_variation
 * @return
 */
collision_safe_params ca_avoid_obstacles(const position_t *current, const route_node_t *next, float magnetic_variation) {
    collision_safe_params ret = {-2,-2,-2};
    if(get_collision_status()){
        //Exec only when collision event occurred

    #if SELECT_COLLISION_STRATEGY_DYNAMICALLY
        if(!collision_context.is_calculated){
            collision_context.is_calculated = 1;
            select_best_collision_strategy(&collision_context, current, next, magnetic_variation);
        }
    #endif

        uint8_t collision_type = get_collision_type();
        if(collision_type & (1 << C_BUILDING) ){
            return collision_context.building.primary(get_route_ptr(),current,next,magnetic_variation);
        }else if(collision_type & (1 << C_TERRAIN) ){
            return collision_context.terrain.primary(get_route_ptr(),current,next,magnetic_variation);
        }


    }
    return ret;
//    return preferred_collision_strategy(get_route_ptr(),current,next,magnetic_variation);
}


/**
 * @brief Tries to choose the best strategy for current collision situation
 * @param col_context
 * @param current
 * @param next
 * @param magnetic_variation
 */
static void select_best_collision_strategy(collision_ctx* col_context,const position_t *current, const route_node_t *next, float magnetic_variation){
    aircraftConfig_t* cfg = get_aircraft_config();
    uint8_t collision_type = get_collision_type();
    if(collision_type & (1 << C_BUILDING) ){
        //Basic checks are already done inside strategy
    }else if(collision_type & (1 << C_TERRAIN) ){
        obstacle_terrain* o_terrain = get_terrain_sections();
        status_frame *status = get_status(status,0);

        if( ((status->alt + (o_terrain->dHeight*TERRAIN_SAFETY_MARGIN)) < cfg->service_ceiling/3.281) && (((float)o_terrain->time_to_terrain/60.0)*cfg->roc_10k  >= o_terrain->dHeight*3.281)  ){
            //Set fly over strat.
            printf("Terrain Col. Selected fly over strategy\n");
            col_context->terrain.primary = collision_t_over_strategy;
            col_context->terrain.update_status = collision_t_reset_over;
        }else{
            //Change to return strategy
            printf("Terrain Col. Selected return to base strategy\n");
            col_context->terrain.primary = collision_return_strategy;
            col_context->terrain.update_status = collision_t_reset_over;
        }

    }


}

/**
 * @brief Gets collision type
 * thread safe
 * @see COLLISION_TYPES
 * @return
 */
uint8_t get_collision_type() {
    int tmp = 0;
    pthread_mutex_lock(&collision_data_status_lock);
    tmp = collision_context.collision_type_in_progress;
    pthread_mutex_unlock(&collision_data_status_lock);
    return tmp;
}

frame_size get_frame_size() {
    frame_size result;
    pthread_mutex_lock(&collision_data_lock);
    result = collision_context.f_size;
    pthread_mutex_unlock(&collision_data_lock);
    return result;
}

/**
 * @brief Set collision strategy
 * @param strategy
 */
void set_context(strategy_selection strategy, enum COLLISION_TYPES type) {
    if(strategy.primary == NULL){
        printf("Primary strategy is null\n");
        strategy.primary = empty_collision_strategy;
    }
    if(strategy.fallback_default == NULL){
        printf("Fallback strategy is null");
        strategy.fallback_default = empty_collision_strategy;
    }
    if(strategy.update_status == NULL){
        printf("Update strategy is null");
        strategy.update_status = collision_reset_empty_strategy;
    }

    if(type == C_BUILDING){
        collision_context.building = strategy;
    }else if(type == C_TERRAIN){
        collision_context.terrain = strategy;
    }
}
/**
 * @brief Can be applied to buildings and terrain
 * Default strategy - must be defined
 * @param route
 * @param current
 * @param next
 * @param magnetic_variation
 * @return
 */
collision_safe_params
collision_return_strategy(queue_list_t* route,const position_t *current, const route_node_t *next, float magnetic_variation) {
    collision_safe_params ret = {-2,-2,-2};

    //position_t current_local = *current;
    position_t next_local = {next->lat, next->lng};


    if(get_collision_status()){
        //Collision was detected, modify heading to new safe value

        queue_node tmpN = get_first_point();
        if(tmpN.data != 0){
            route_node_t* node = (route_node_t*) tmpN.data;
            next_local.lat = node->lat;
            next_local.lng = node->lng;
        }

        ret.heading = degWrap360(get_target_heading(current->lat, current->lng, next_local.lat, next_local.lng) + magnetic_variation);
        //Just return normal heading

    }

    return ret;

}

/**
 * Try to 'squeeze' between buildings
 * @param route
 * @param current
 * @param next
 * @param magnetic_variation
 * @return
 */
collision_safe_params
collision_b_squeeze_strategy(queue_list_t* route, const position_t *current, const route_node_t *next, float magnetic_variation) {
    collision_safe_params ret = {-1,-1,-2};
    /*
     * 0. if(collision_avoidance_in_progress)
     * 1. get sections
     * 2. iterate over sections and select widest.
     * 3. choose one closest to the original route
     */

    if(get_collision_status()){

        if(collision_context.section_data_invalid){
            return ret;//Ignore that value in app.c
        }

    double new_hdg;
    __attribute__((unused)) int type = get_collision_type();


    //Building type --------------------------------------------------------------------------
    obstacle_distance_ang_t* sections = get_building_sections();
    int sections_count = get_section_count();
    int biggest_valid_idx = 0;
    int middle_pixel_x = (get_frame_size().x >> 1);
    int curr_hdg = (int) get_data(HEADING, 0);
    uint8_t sec_err = 1;//Checks if at least one section is valid

    //printf("\n\n----------------\n");
    for (int sec = 0; sec < sections_count;++sec) {
        if(sections[sec].distanceDeg >= MIN_SECTION_ANGLE){
            sec_err = 0;
            if(sections[sec].distanceDeg > sections[biggest_valid_idx].distanceDeg){
                biggest_valid_idx = sec;//Select biggest and valid section
            }
        }
        //printf("%d %d \n", sections[sec].startX,sections[sec].startY);
    }

    //Invalidate data, force wait for new sections calculations
    invalidate_sections_data();

    if(sec_err){
        printf("\n\nSec ERR %d\n\n",sections_count);
        //No valid sections found, execute default strategy
        return collision_context.building.fallback_default(route,current,next,magnetic_variation);
//        return fallback_collision_strategy(route,current,next,magnetic_variation);
    }

    int sign;

    if( (sections[biggest_valid_idx].startX + sections[biggest_valid_idx].endX)/2 >= middle_pixel_x ){
        sign = 1;
    }else{
        sign = -1;
    }
    new_hdg = degWrap360(curr_hdg + (sign*(sections[biggest_valid_idx].distanceDeg/2)));

    printf("\n\nSelected section idx: %d ang: %f\n",biggest_valid_idx,sections[biggest_valid_idx].distanceDeg);
    printf("New hdg: %f\n\n",  new_hdg);

    //----------------------------------------------------------------------------------------
    ret.heading = (float) new_hdg;
    return ret;

    }else{
        return collision_context.building.fallback_default(route,current,next,magnetic_variation);
//        return fallback_collision_strategy(route,current,next,magnetic_variation);
    }



}

/**
 * Return collision status (1 or 0)
 * @return
 */
uint8_t get_collision_status() {
    int tmp;
    pthread_mutex_lock(&collision_data_status_lock);
    tmp = collision_context.collision_avoidance_in_progress;
    pthread_mutex_unlock(&collision_data_status_lock);
    return tmp;
}

/**
 * Sets the collision status
 * @param status (1 or 0)
 * @param type (type from COLLISION_TYPES)
 * @return
 */
uint8_t set_collision_status(int status, int type) {
    pthread_mutex_lock(&collision_data_status_lock);
    time_t  current_time = time(NULL);
    if(status && !collision_context.collision_avoidance_in_progress){
        collision_context.last_collision_started = current_time;
    }
    collision_context.collision_avoidance_in_progress = status;
    if(status){
        //Set 1
        collision_context.collision_type_in_progress = collision_context.collision_type_in_progress | (1 << type);
    }else{
        // Set 0
        collision_context.last_collision_ended = current_time;
        collision_context.is_calculated = 0;
        collision_context.collision_type_in_progress = collision_context.collision_type_in_progress & ~(1 << type);
    }
    pthread_mutex_unlock(&collision_data_status_lock);
    return 1;
}

/**
 * Checks if current collision avoidance can be disabled and normal flight continued
 * called on every route tick
 */
void update_status(const pid_input_t roll, const pid_input_t pitch) {
    if(get_collision_status()){
        int type = get_collision_type();
        if(type & (1 << C_BUILDING)  ){

            collision_context.building.update_status(roll,pitch);

        }else if(type & (1 << C_TERRAIN) ) {

            collision_context.terrain.update_status(roll,pitch);

        }
    }


}

/**
 * @brief Terrain collision s.
 * Tries to fly around colliding terrain
 * @param route
 * @param current
 * @param next
 * @param magnetic_variation
 * @return
 */
collision_safe_params collision_t_around_strategy(queue_list_t *route, const position_t *current, const route_node_t *next, float magnetic_variation) {
    collision_safe_params ret = {-1,-1,-2};
    return ret;
}

/**
 * @brief Terrain collision s.. Collision is avoided from above.
 * No heading change is made
 * @param route
 * @param current
 * @param next
 * @param magnetic_variation
 * @return
 */
collision_safe_params collision_t_over_strategy(queue_list_t *route, const position_t *current, const route_node_t *next, float magnetic_variation) {
    collision_safe_params ret = {-2,-2,-1};
    obstacle_terrain* o_terrain = get_terrain_sections();
    int type = get_collision_type();
    status_frame *status = get_status(status,0);
    printf("OVER STRAT status: %d targeting: %f\n", get_collision_status(), get_target_alt()*3.281f);

    if(get_collision_status()){

        float tmp_alt = (float) (status->alt + (o_terrain->dHeight*TERRAIN_SAFETY_MARGIN));

        if(collision_context.section_data_invalid && tmp_alt <= get_target_alt()*1.01 ){
            return ret;//Ignore that value in app.c
        }

//        ret.alt = (float) (status->alt + (((next->alt/3.281f) + (o_terrain->dHeight))*1.05));
        ret.alt = tmp_alt;

        printf(">>>>> Avoiding terrain from above (%d); changing altitude to: %f  (%f)<<<<<\n",type, ret.alt, next->alt);

        invalidate_sections_data();

        return ret;

    }else{
        return collision_context.terrain.fallback_default(route,current,next,magnetic_variation);
    }

}

/**
 * @brief empty strategy for updating status
 * It just resets status
 * @param roll
 * @param pitch
 */
void collision_reset_empty_strategy(pid_input_t roll, pid_input_t pitch) {
    collision_context.section_data_invalid = 0;
    set_collision_status(0, C_TERRAIN);
    set_collision_status(0, C_BUILDING);
}

/**
 *
 */
void collision_reset_return_strategy(pid_input_t roll, pid_input_t pitch){
    //Just return to the starting point and wait
}


/**
 * @brief resets collision status when flying over obstacles
 * @param roll
 * @param pitch
 */
void collision_t_reset_over(pid_input_t roll, pid_input_t pitch) {
    obstacle_terrain* o_terrain = get_terrain_sections();//Get info about terrain collision
    //Check if drone has met all the requirements for avoiding this collision
    status_frame *status = get_status(status,0);
    time_t  current_time = time(NULL);

    //@TODO define constant with time in seconds

    //Reset collision alert after one minute
    if( ((current_time - collision_context.last_collision_started) >= o_terrain->time_to_terrain) &&  status->alt >= (get_target_alt()-5)){
        printf("\n\n ---- Resetting status [terrain] ---- \n\n");
        collision_context.section_data_invalid = 0;
        set_collision_status(0, C_TERRAIN);
    }


    //Distance is in meters
//    float dist = pointDistance(status->lat, status->lng, o_terrain->colliding_point.lat,o_terrain->colliding_point.lng);

//    if(fp_compare(dist, 0, 10)){
//        printf("\n\n ---- Resetting status [terrain] ---- \n\n");
//        collision_context.section_data_invalid = 0;
//        set_collision_status(0, C_TERRAIN);
//        exit(-2);
//    }

}

void collision_b_reset_squeeze(pid_input_t roll, pid_input_t pitch) {
    float curr_hdg = get_data(HEADING, 0);

    if(fp_compare(curr_hdg, roll.target, CMP_DELTA_HEADING)){
        if(collision_context.collision_secondary_timer_start == 0){
            //Set current time after aircraft reached its target heading
            collision_context.collision_secondary_timer_start = time(NULL);
        }else{
            if( (time(NULL) - collision_context.collision_secondary_timer_start) >= BUILDING_RESET_TIME ){
                printf("\n\n ---- Resetting status [building] ---- \n\n");
                collision_context.section_data_invalid = 0;
                set_collision_status(0, C_BUILDING);
            }
        }
    }

}


























