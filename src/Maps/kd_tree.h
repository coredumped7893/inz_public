//
// Created by Maciek Malik
//

#ifndef MM_kd_tree_H_
#define MM_kd_tree_H_

#include "maps.h"
#include <math.h>

extern const uint8_t dims;//2 dimensions are enough for this purpose

/**
 * Index to the tree node inside external_data data array
 * @deprecated
 */
typedef int32_t kd_index_t;

/**
 * @brief Single node of the 2d tree, holds single point (pl92_point_alt_t)
 */
typedef struct KD_Node_t {
    /**
     * Single point with data
     */
    pl92_point_alt_t map_point;
    /**
     * Index of the next KD_Node_t with smaller value on current dimension
     */
    kd_index_t left_up;
    /**
     * Index of the next KD_Node_t with bigger or equal value on current dimension
     */
    kd_index_t right_down;

    //has to be as a pointer otherwise: error: field ‘node_left_up’ has incomplete type
    //this would be infinitely recursive definition
    struct KD_Node_t* node_left_up;

    struct KD_Node_t* node_right_down;


} KD_Node_t;

/**
 * @brief Structure for 2d tree holding pl92_point_alt_t points separated between 2 dimensions
 * data with points has to be dynamically allocated before
 * @see init_tree()
 */
typedef struct {
    kd_index_t root;

    /**
     * If external data is used then it is the buffer length
     */
    uint32_t data_size;

    /**
     * How many points has been reserved in external data pointer
     */
    uint32_t idx_reserved;
    pl92_point_alt_t min;

    /**
     * If drone is flying above max terrain height, omit checking it on the map
     */
    pl92_point_alt_t max;
    /**
     * Ensure that this field is always 8 bytes long (for rPI compatibility)
     */
    union {
        /**
         * Use pre allocated data if not then value is set to NULL
         */
        KD_Node_t *external_data;
        uint64_t padding__;
    };
} KD_Tree_t;

typedef uint8_t (*point_compare_fn) (pl92_point_alt_t, pl92_point_alt_t);//pointer to the comparator function
//Point comparators ---------------------
uint8_t tree_points_equal(pl92_point_alt_t a, pl92_point_alt_t b);
uint8_t tree_points_close_enough(pl92_point_alt_t a, pl92_point_alt_t b);
uint8_t tree_points_equal_altitudes(pl92_point_alt_t a, pl92_point_alt_t b);
//---------------------------------------

kd_index_t insert_point(KD_Tree_t *tree, kd_index_t root, pl92_point_alt_t *point, uint32_t level);

kd_index_t search_point(KD_Tree_t *tree, kd_index_t root, pl92_point_alt_t *point, uint32_t level, point_compare_fn comparator_fn);

KD_Tree_t *init_tree(KD_Node_t *data_ptr, long data_size);

KD_Tree_t *get_tree_data();

double dist_2d(float x1, float y1, float x2, float y2);

KD_Node_t *alloc_node_by_val(KD_Tree_t *tree, uint32_t x, uint32_t y, float alt);

pl92_point_alt_t *alloc_point(uint32_t x, uint32_t y, float alt);

void tree_save_binary(KD_Tree_t *tree);
KD_Tree_t *tree_load_binary();


extern long max_level;
extern long exec_number;

#endif //MM_kd_tree_H_
