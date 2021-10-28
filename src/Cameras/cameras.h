//
// Created by Maciek Malik
//

#ifndef MM_cameras_H_
#define MM_cameras_H_

#include "camDataStrategy.h"

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief struct that contains pointers to functions for controlling camera features
     */
    typedef struct {
        camera_data_strategy init_source;
    } camera_controls_t;

    /**
     * @brief mapping context id to proper camera_controls_t structure
     */
    enum CONTEXT_TYPES {
        VIRTUAL_UDP       = 0,
        VIRTUAL_RTMP      = 1,
        RPI_IMX           = 2,
        RPI_OV            = 3,
        TEST_SINGLE_FRAME = 4
    };

    camera_controls_t create_camera_context(CONTEXT_TYPES type);

#ifdef __cplusplus
}
#endif


#endif //MM_cameras_H_
