//
// Created by Maciek Malik
//

#include "cameras.h"


//temporary definitions here to avoid warning 'ISO C++ forbids compound-literals'
camera_controls_t tmp0 = {virtual_sim_udp};
camera_controls_t tmp1 = {virtual_sim_rtmp};
camera_controls_t tmp2 = {hardware_camera_imx};
camera_controls_t tmp3 = {hardware_camera_ov};
camera_controls_t tmp4 = {virtual_sim_benchmark_frame};
//-----------------------------------------------------------------------

/**
 * @brief mapping different strategies to enum types
 */
static camera_controls_t strategy_mapping[5] = {tmp0,tmp1,tmp2,tmp3,tmp4};

/**
 * @brief returns camera_controls_t structure which contains functions pointer to proper functions(strategy)
 * @param type
 * @return
 */
camera_controls_t create_camera_context(CONTEXT_TYPES type) {
    return strategy_mapping[type];
}
