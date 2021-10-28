//
// Created by Maciek Malik
//
/*
 * Simplified implementation of the KD tree, limited to only 2d planes - position coordinates
 */

#include "kd_tree.h"
#include <stdlib.h>
#include <stdio.h>
#include "../Util/other.h"

kd_index_t alloc_node(KD_Tree_t *tree, pl92_point_alt_t point);

//Source
//https://www.cs.cmu.edu/~ckingsf/bioinfo-lectures/kdtrees.pdf

/**
 * @brief Counter for max level achieved during recursive point insertion
 */
long max_level = 0;

long exec_number = 0;

/**
 * @brief Number of dimensions that this tree will operate on
 * (X and Y)
 */
const uint8_t dims = 2;

/**
 *  @brief Returns index in external_data array to KD_Node_t structure
 *  external data needs to be big enough to fit every point (size is not checked)
 * @param tree
 * @param point
 * @return
 */
kd_index_t alloc_node(KD_Tree_t *tree, pl92_point_alt_t point) {
    if (tree->external_data != NULL) {
        //Data is pre allocated elsewhere
        tree->external_data[tree->idx_reserved].map_point = point;
        tree->external_data[tree->idx_reserved].left_up = -1;
        tree->external_data[tree->idx_reserved].right_down = -1;
        ++tree->idx_reserved;
        return (int) tree->idx_reserved-1;
        //return tree->external_data + (tree->idx_reserved - 1);
    } else {
        printf("\n\nOnly pre allocated memory supported\n\n");
        KD_Node_t *tmp = calloc(1, sizeof(KD_Node_t));
        tmp->map_point = point;
        tmp->left_up = 0;
        tmp->right_down = 0;
//        return tmp;
        return 0;
    }
}

/**
 * @brief Allocate node by values in dynamic memory
 * @param tree
 * @param x
 * @param y
 * @param alt
 * @deprecated
 * @return
 */
KD_Node_t *alloc_node_by_val(KD_Tree_t *tree, uint32_t x, uint32_t y, float alt) {
    KD_Node_t *tmp_node = malloc(sizeof(KD_Node_t));
    pl92_point_alt_t *p = malloc(sizeof(pl92_point_alt_t));
    p->x = x;
    p->y = y;
    p->alt = alt;
    tmp_node->map_point = *p;
    tmp_node->right_down = -1;
    tmp_node->left_up = -1;
    return tmp_node;
}

/**
 * @brief Recursive implementation for 2d tree
 * @param tree
 * @param point
 * @param level
 * @return
 */
kd_index_t insert_point(KD_Tree_t *tree, kd_index_t root, pl92_point_alt_t *point, uint32_t level) {

    ++exec_number;

    if (root == -1) {
        //Create new node if tree is empty or we are at empty leaf level
        kd_index_t tmp_node = alloc_node(tree, *point);
        tree->root = tmp_node;//Set new root

        //Update min and max values
        if(tree->max.alt < point->alt){
            tree->max = *point;
        }
        if(tree->min.alt > point->alt){
            tree->min = *point;
        }

        return tmp_node;
    }

    if (level > max_level) {
        max_level = level;
    }

    uint32_t compare_point[2];// (X,Y)
    compare_point[0] = point->x;
    compare_point[1] = point->y;

    uint32_t root_compare_point[2];//(X,Y)
    root_compare_point[0] = (tree->external_data + root)->map_point.x;
    root_compare_point[1] = (tree->external_data + root)->map_point.y;

    uint8_t current_dimension = level % dims;//Calculate at which dimension we are currently

    if (compare_point[current_dimension] < root_compare_point[current_dimension]) {
        (tree->external_data + root)->left_up = insert_point(tree, (tree->external_data + root)->left_up, point, level + 1);
    } else {
        (tree->external_data + root)->right_down = insert_point(tree, (tree->external_data + root)->right_down, point, level + 1);
    }

    tree->root = root;
    return tree->root;
}

/**
 * @brief Initializes tree, if there is no pre allocated buffer then set parameters to NULL,0
 * @param data_ptr
 * @param data_size
 * @return pointer to the KD_Tree_t structure
 */
KD_Tree_t *init_tree(KD_Node_t *data_ptr, long data_size) {
    KD_Tree_t *out;
    KD_Tree_t *tmp = calloc(1, sizeof(KD_Tree_t));
    if (tmp == NULL) {
        throw_err(3, 1);
    }
    out = tmp;

    out->root = -1;
    out->external_data = data_ptr;
    out->idx_reserved = 0;
    out->data_size = data_size;

    return out;
}


/**
 * @brief Search for a exact point in the 2d tree using one of the comparators
 * @see tree_points_equal
 * @see tree_points_close_enough
 * @see tree_points_equal_altitudes
 * @param root
 * @param point
 * @param level
 * @param comparator_fn
 * @return index of the found point or -1
 */
kd_index_t search_point(KD_Tree_t *tree, kd_index_t root, pl92_point_alt_t *point, uint32_t level, point_compare_fn comparator_fn) {

    if (root == -1) {
        return -1;
    }
    if (comparator_fn((tree->external_data + root)->map_point, *point)) {
        return root;//Return index of the found node
    }

    uint32_t compare_point[2];// (X,Y)
    compare_point[0] = point->x;
    compare_point[1] = point->y;

    uint32_t root_compare_point[2];//(X,Y)
    root_compare_point[0] = (tree->external_data + root)->map_point.x;
    root_compare_point[1] = (tree->external_data + root)->map_point.y;

    uint8_t current_dimension = level % dims;

    if (compare_point[current_dimension] < root_compare_point[current_dimension]) {
        return search_point(tree, (tree->external_data + root)->left_up, point, level + 1, comparator_fn);
    }

    return search_point(tree,(tree->external_data + root)->right_down, point, level + 1, comparator_fn);

}

/**
 * @brief Saves tree to file as binary data, speeds up loading times by a lot
 * assuming that binary tree file doest not exist
 * @param tree
 */
void tree_save_binary(KD_Tree_t *tree) {
    printf("\nSaving tree to file.\n");

    FILE *bin_tree = fopen(TREE_BINARY_FILENAME, "wb");
    if (bin_tree == NULL) {
        printf("\nCould not open/create map file\n");
        return;
    }

    //Save first KD_Tree_t struct and then data array
    if(fwrite(tree, sizeof(KD_Tree_t), 1 , bin_tree) <= 0){
        printf("\nCould not write to file (binary tree).\n");
        return;
    }

    //Save rest of the data
    if(fwrite(tree->external_data, sizeof(KD_Node_t), tree->data_size,bin_tree) <= 0){
        printf("\nCould not write to file (binary tree).\n");
        return;
    }
    fclose(bin_tree);
    printf("\nGenerated tree structure for maps has been saved.\n");
}

/**
 * @brief Loads tree from binary file, assumes that file exists
 * @return KD_Tree_t pointer
 */
KD_Tree_t *tree_load_binary() {

    FILE *bin_tree = fopen(TREE_BINARY_FILENAME, "rb");
    if (bin_tree == NULL) {
        printf("\nCould not open/create map file\n");
        return NULL;
    }

    KD_Tree_t *loaded_tree = calloc(1,sizeof(KD_Tree_t));

    //Load tree structure
    if (fread(loaded_tree, sizeof(KD_Tree_t), 1, bin_tree) == 0) {
        printf("\nCould not load binary map file\n");
        return NULL;
    }
    printf("Allocating memory for %u nodes\n", loaded_tree->data_size);
    KD_Node_t *loaded_tree_data = (KD_Node_t *) calloc(loaded_tree->data_size+1,sizeof(KD_Node_t));
    if (fread(loaded_tree_data, sizeof(KD_Tree_t), loaded_tree->data_size, bin_tree) == 0) {
        printf("\nCould not load binary map file\n");
        return NULL;
    }
    loaded_tree->external_data = loaded_tree_data;
    fclose(bin_tree);
    return loaded_tree;
}





/**
 * @brief Comparator for checking if two points has exactly the same coordinates
 * @param a
 * @param b
 * @return
 */
uint8_t tree_points_equal(pl92_point_alt_t a, pl92_point_alt_t b) {
    if (a.x == b.x && a.y == b.y) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Comparator for points to check if they are relatively close to each other
 * @param a
 * @param b
 * @return
 */
uint8_t tree_points_close_enough(pl92_point_alt_t a, pl92_point_alt_t b) {
    if (dist_2d((float) a.x, (float) a.y, (float) b.x, (float) b.y) <= TERRAIN_POINTS_EPSILON) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Comparator for the same altitudes
 * coords has to be close to the point with searched value
 * @to improve
 * @param a
 * @param b
 * @return
 */
uint8_t tree_points_equal_altitudes(pl92_point_alt_t a, pl92_point_alt_t b) {
    if (fp_compare(b.alt, a.alt, 0.5)) {
        return 1;
    } else {
        return 0;
    }
}


/**
 * @brief Calculate pseudo distance from given two points
 * square root is omitted in this case to speed up whole calculation
 * results can be still compared against each other
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @return
 */
double dist_2d(float x1, float y1, float x2, float y2) {
    return pow((x1 - x2), 2) + pow((y1 - y2), 2);
}

/**
 * @brief Allocates new point in dynamic memory
 * @param x
 * @param y
 * @param alt
 * @return
 */
pl92_point_alt_t *alloc_point(uint32_t x, uint32_t y, float alt) {
    pl92_point_alt_t *tmp = calloc(1, sizeof(pl92_point_alt_t));
    if (tmp == NULL) {
        throw_err(3, 1);
    }
    return tmp;
}



