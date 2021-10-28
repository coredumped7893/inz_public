//
// Created by Maciek Malik
//

#ifndef MM_maps_H_
#define MM_maps_H_


#include <stdint.h>
#include "../Util/message.h"
#include "string.h"
#include "../Util/buff_tools.h"
#include "../ImageProcessing/wgs84_do_puwg92.h"


uint8_t maps_init();

/**
 * @brief Representation of a single point in PUWG92 coordinates; 12 bytes
 */
typedef struct {
    /**
     * X axis in PUWG92 format LAT, 4 bytes
     */
    uint32_t x;
    /**
     * Y axis in PUWG92 format LNG, 4 bytes
     */
    uint32_t y;
    /**
     * Altitude above sea level in meters.
     */
    float alt;
}pl92_point_alt_t;

typedef struct{
    /**
     * X axis minimal value
     */
    uint32_t min_x;

    /**
     * X axis biggest value
     */
    uint32_t max_x;

    /**
     * Y axis minimal value
     */
    uint32_t min_y;

    /**
     * Y axis biggest value
     */
    uint32_t max_y;

    /**
     * Highest altitude value
     */
    float max_alt;

}pl92_min_max_point;

pl92_point_alt_t *get_map_ptr();
long get_map_lines();
void convert_map_binary();
extern long point_file_index(pl92_point_alt_t tmpPoint);
pl92_min_max_point get_min_max_point();

extern int are_maps_loaded;

#endif //MM_maps_H_
