//
// Created by Maciek Malik
//

#ifndef MM_collisionAvoidanceStrategy_H_
#define MM_collisionAvoidanceStrategy_H_

#include "../../inc/queue.h"
#include "route.h"
#include "PID.h"

typedef struct {
    float heading;
    float ias;

    /**
     * @brief Suggested altitude given in meters
     */
    float alt;
}collision_safe_params;


typedef collision_safe_params (*collisionStrategy)(queue_list_t*, const position_t*, const route_node_t*, float);

typedef void (*collisionUpdateStatusStrategy)(const pid_input_t roll, const pid_input_t pitch);

#endif //MM_collisionAvoidanceStrategy_H_
