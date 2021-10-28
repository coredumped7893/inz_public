//
// Created by Maciek Malik
// Parses maps and caches data in buffers; maps it as WGS84->height

#define _POSIX_SOURCE
#define  _XOPEN_SOURCE 500
#include <dirent.h>
#include "stdio.h"
#include "maps.h"
#include "../Util/message.h"
#include "../Util/time_diff.h"
#include "kd_tree.h"
#include "unistd.h"
#include "../ImageProcessing/wgs84_do_puwg92.h"

void parse_height_map(FILE *current_fd);

void maps_load_all();

long build_map_tree(long map_size);

void test_wgs_pl1992_convert();

void convert_map_binary_regular();

void maps_load_regular();

/**
 * @brief Pointer to the maps data array
 */
pl92_point_alt_t *map_H = NULL;
KD_Tree_t *map_tree = NULL;
KD_Node_t *map_tree_data = NULL;
pl92_min_max_point min_point;

/**
 * @brief Current state of map_H array
 */
int are_maps_loaded = 0;

/**
 * @brief How many points has been loaded from map files
 */
uint32_t lines = 0;


/**
 * @brief Initializes maps and data structures
 * @return 1 if succeeded or 0 otherwise
 */
uint8_t maps_init() {

    printf("---- Initialising maps ----\n\n");


    map_H = (pl92_point_alt_t *) calloc(1, sizeof(pl92_point_alt_t));
    min_point.min_x = 999999;
    min_point.min_y = 999999;
    min_point.max_y = 0;
    min_point.max_alt = 0;
    min_point.max_x = 0;

    printf("int: %lu long:%lud", sizeof(int), sizeof(long));
    printf("int*: %lu", sizeof(int*));
    printf("KD_Tree_t: %lu", sizeof(KD_Tree_t));


    //Test 2d tree------------------------------------------------------------------
    //@TODO move to tests folder

    KD_Node_t *test_data = (KD_Node_t *) calloc(5, sizeof(KD_Node_t));
    KD_Tree_t *testTree = init_tree(test_data, 5);

    pl92_point_alt_t in1 = {245, 32, 13.56};
    pl92_point_alt_t in2 = {683, 131, 56.987};
    pl92_point_alt_t in3 = {457123, 1945, 1187.118};
    pl92_point_alt_t in4 = {4712, 9971, 563.287};
    pl92_point_alt_t in5 = {42, 91, 53.87};

    pl92_point_alt_t search1 = {144, 65, 89.9};//Not in the set
    pl92_point_alt_t search2 = {683, 131, 89.9};//In the set but different value
    pl92_point_alt_t search3 = {683, 131, 187.1};//Value is almost the same that is in set
    pl92_point_alt_t search4 = {457125, 1941, 89.9};//Close enough value in the set
    pl92_point_alt_t search5 = {4712, 9971, 19.9};

    insert_point(testTree, testTree->root, &in1, 0);
    insert_point(testTree, testTree->root, &in2, 0);
    insert_point(testTree, testTree->root, &in3, 0);
    insert_point(testTree, testTree->root, &in4, 0);
    insert_point(testTree, testTree->root, &in5, 0);

//    //Test if tree can be reallocated after inserting points
//    test_data = (KD_Node_t *) realloc(test_data, sizeof(KD_Node_t) * 10);
//    testTree->external_data = test_data;
//    testTree->data_size = 10;

    //------------------------------------------------------------------------------  @TODO Move to the tests
    printf("\nSearching point[1]: %d\n", search_point(testTree, testTree->root, &search1, 0, tree_points_equal));//0
    printf("Searching point[2]: %d\n", search_point(testTree, testTree->root, &search2, 0, tree_points_equal));//1
    printf("Searching point[5]: %d\n", search_point(testTree, testTree->root, &search5, 0, tree_points_equal));//1
    printf("Searching point values: %d\n",
           search_point(testTree, testTree->root, &search3, 0, tree_points_equal_altitudes));//0
    printf("Searching point close: %d\n",
           search_point(testTree, testTree->root, &search4, 0, tree_points_close_enough));//1
    printf("Current root, X: %u Y: %u\n", (testTree->external_data + testTree->root)->map_point.x,
           (testTree->external_data + testTree->root)->map_point.y);

    //------------------------------------------------------------------------------

    double testLat, testLng;

    puwg92_do_wgs84(154062.0,570013.0,&testLat,&testLng);
    printf("Test coords: %f %f\n", testLat, testLng);
    //Test coords: 49.250004 19.968381 for: 154062.0,570453.0
    //Test coords: 49.250055 19.962334L



#if LOAD_ALL_MAPS
        printf("Loading map data...\n");
    #if TERRAIN_MAPS_SEARCH_METHOD == 3
        if (access(TREE_BINARY_FILENAME, F_OK) != 0) {
            maps_load_all();//Force map loading, tree doest not exist
            long map_size = get_map_lines();
            map_tree_data = (KD_Node_t *) calloc(map_size,
                                                 sizeof(KD_Node_t));//Allocate the same amount of nodes that are points in the maps
            map_tree = init_tree(map_tree_data, map_size);//Initial tree
            long time_t = build_map_tree(map_size);
            printf("Tree finished, max level: %ld (Executed %ld times) time: %ld\n\n", max_level, exec_number, time_t);
        #if SAVE_TREE_TO_BINARY
            tree_save_binary(get_tree_data());
        #endif
        } else {
            //Otherwise load it from file
            start_timer();
            map_tree = tree_load_binary();
            lines = map_tree->data_size;
            printf("\nMap data tree loaded from file time: %ld\n",end_timer());
        }

        printf("Map min: %f max: %f\n\n", map_tree->min.alt, map_tree->max.alt);

    #elif TERRAIN_MAPS_SEARCH_METHOD == 4
        if(access(BINARY_MAP_FILENAME_REGULAR, F_OK) != 0){
            //Generate file
            maps_load_all();//Force map loading, data is not loaded
            //Map will be saved below as a default action when CONVERT_MAPS_TO_BINARY is enabled
        }else{
            //Load only min max data
            maps_load_regular();
        }
    #else
        //Load maps normally (not tree)
        maps_load_all();
    #endif

    #if CONVERT_MAPS_TO_BINARY
        convert_map_binary();//We still want original data in binary format to speed up init process
        convert_map_binary_regular();
//        #if TERRAIN_MAPS_SEARCH_METHOD == 4
//            convert_map_binary_regular();
//        #else
//            convert_map_binary();
//        #endif
    #endif

    #else
        //...
#endif// end LOAD_ALL_MAPS
    are_maps_loaded = 1;//Enable th_terrain_worker thread
    print_message_local(4);//maps loaded

    return 1;
}

void maps_load_regular(){
    FILE *current_fd;
    //Assert BINARY_MAP_FILENAME_REGULAR exists
    current_fd = fopen(BINARY_MAP_FILENAME_REGULAR, "rb");
    if (current_fd == NULL) {
        printf("\nCould not open/create map file\n");
        return;
    }

    uint32_t loaded_map_size = 0;
    fread(&loaded_map_size, sizeof(uint32_t), 1, current_fd);//Get number of points in the file
    fread(&min_point, sizeof(pl92_min_max_point), 1, current_fd);
    printf("Loaded map size[regular]: %d", loaded_map_size);
    if (loaded_map_size <= 0) {
        printf("\nMap file is corrupted or empty\n");
    }
    lines = loaded_map_size;//Needed for correctly building a 2d tree

}


long inline point_file_index(pl92_point_alt_t tmpPoint){
    long map_width = min_point.max_y - min_point.min_y;
    return (sizeof(uint32_t) + sizeof(pl92_min_max_point) + ((map_width * (tmpPoint.x - min_point.min_x) + (tmpPoint.y - min_point.min_y))*sizeof(float)));
}



/**
 * @brief Convert map to binary format with points saved as regular mesh with resolution of MAP_SOURCE_MESH_RESOLUTION
 */
void convert_map_binary_regular() {
    if(access(BINARY_MAP_FILENAME_REGULAR, F_OK) != 0){
        //File does not exist, create one.
        pl92_point_alt_t *map = get_map_ptr();
        long map_size = get_map_lines();//Number of points loaded
        FILE *map_bin = fopen(BINARY_MAP_FILENAME_REGULAR, "wb");
        if(map_bin == NULL){
            printf("\nCould not open/create map file\n");
            return;
        }

        unsigned long file_idx = 0;
        long map_width = min_point.max_y - min_point.min_y;
        long map_height = min_point.max_x - min_point.min_x;

        long map_elements_number = ((map_width*map_height) + map_width);

        ftruncate(fileno(map_bin), (sizeof(uint32_t) + sizeof(pl92_min_max_point) + (map_elements_number * sizeof(float))));//Set file size and fill it with zeros
        uint32_t size_f = map_size;
        fwrite(&size_f, sizeof(uint32_t), 1, map_bin);//Write header containing data size
        fwrite(&min_point, sizeof(pl92_min_max_point), 1, map_bin);

        printf("\n\nMap width: %ld | Height: %ld\n\n", map_width, map_height);

        /**
         * use temporary buffer to speed up file generation
         */
        float* map_tmp_values = calloc(map_elements_number, sizeof(float));
        //Y=( RightY*(X-LeftX) - LeftY*(X-RightX) ) / (RightX-LeftX)

        for(int point_idx=0; point_idx < map_size; ++point_idx){
            pl92_point_alt_t tmpPoint = map[point_idx];
            file_idx = map_width * (tmpPoint.x - min_point.min_x) + (tmpPoint.y - min_point.min_y);
            map_tmp_values[file_idx] = tmpPoint.alt;
        }

        //Write whole temp buffer in one action
        fwrite(map_tmp_values, (map_elements_number * sizeof(float)),1, map_bin);

        fclose(map_bin);
        free(map_tmp_values);
        printf("Map converted to binary regular format and saved.\n");

    }

}

pl92_min_max_point get_min_max_point(){
    return min_point;
}

/**
 * @return Returns map pointer
 */
pl92_point_alt_t *get_map_ptr() {
    return map_H;
}

/**
 * @return Returns number of lines(measurements) across every loaded file
 */
long get_map_lines() {
    return lines;
}

/**
 * @brief If CONVERT_MAPS_TO_BINARY is set then currently loaded map data to memory gets saved to binary file
 * saves points ine the format of pl92_point_alt_t in the same order as they appear in the original map files
 */
void convert_map_binary() {
    //Check if binary map file is already created
    if (access(BINARY_MAP_FILENAME, F_OK) != 0) {
        //File does not exist, create one.
        pl92_point_alt_t *map = get_map_ptr();
        long map_size = get_map_lines();
        FILE *map_bin = fopen(BINARY_MAP_FILENAME, "wb");
        if (map_bin == NULL) {
            printf("\nCould not open/create map file\n");
            return;
        }
        uint32_t size_f = map_size;
        fwrite(&size_f, sizeof(uint32_t), 1, map_bin);//Write header containing data size
        fwrite(&min_point, sizeof(pl92_min_max_point), 1, map_bin);//Write min,max values of points
        fwrite(map, sizeof(pl92_point_alt_t), map_size, map_bin);//Copy the rest of the buffer

        fclose(map_bin);
        printf("Map converted to binary format and saved.\n");
    }
}

/**
 * @brief Returns pointer to the tree dynamic data
 * @return KD_Tree_t
 */
KD_Tree_t *get_tree_data() {
    return map_tree;
}

/**
 * @deprecated
 * @param file
 */
void count_total_lines(FILE *file) {}

/**
 * @brief Iterates over loaded map points and creates tree from them for more efficient searching
 * start from the middle and go in both directions at the same time
 * @return returns time elapsed
 */
long build_map_tree(long map_size) {
    printf("Building search tree \n");
    start_timer();
    long middle_idx = map_size >> 1;
    for (long data_point = 0; data_point < middle_idx + 1; ++data_point) {
        long idxPOS = middle_idx + data_point;
        long idxNEG = middle_idx - data_point;
        if (idxNEG >= 0) {
            insert_point(map_tree, map_tree->root, map_H + idxNEG, 0);
        }
        if (idxPOS < map_size) {
            insert_point(map_tree, map_tree->root, map_H + idxPOS, 0);
        }
    }
    return end_timer();
}

/**
 * @brief Function that loads map/s file/s located in the "map/" directory.
 * If binary map file is found then only it is loaded, otherwise every file ending with .xyz
 * @see parse_height_map()
 */
void maps_load_all() {

    DIR *dir_fd;
    FILE *current_fd;
    struct dirent *current_dir;

    dir_fd = opendir("maps");

    if (dir_fd == NULL) {
        closedir(dir_fd);
        throw_err(7, 1);
    }

    start_timer();


    if (access(BINARY_MAP_FILENAME, F_OK) == 0) {
        //File exists, load it instead
        printf("\n--> Loading map data from binary file.\n");
        current_fd = fopen(BINARY_MAP_FILENAME, "rb");
        if (current_fd == NULL) {
            printf("\nCould not open/create map file\n");
            return;
        }
        uint32_t loaded_map_size = 0;
        fread(&loaded_map_size, sizeof(uint32_t), 1, current_fd);//Get number of points in the file
        fread(&min_point, sizeof(pl92_min_max_point), 1, current_fd);
        printf("Loaded map size: %d", loaded_map_size);
        if (loaded_map_size <= 0) {
            printf("\nMap file is corrupted or empty\n");
        }
        lines = loaded_map_size;//Needed for correctly building a 2d tree
        //Allocate enough memory for every point
        pl92_point_alt_t *tmp = (pl92_point_alt_t *) realloc(map_H,
                                                             sizeof(pl92_point_alt_t) * (loaded_map_size + 1));
        if (tmp != NULL) {
            map_H = tmp;
        } else {
            //@TODO change it to message
            printf("\nCould not allocate memory for map data\n");
            exit(5);
        }
        if (fread(map_H, sizeof(pl92_point_alt_t), loaded_map_size, current_fd) == 0) {
            printf("\nCould not load binary map file\n");
            return;
        }
        fclose(current_fd);
    } else {
        //Binary file not found, load raw map data
        //Iterate over whole structure
        while ((current_dir = readdir(dir_fd)) != NULL) {
            //Load only files ending with .xyz extension
            if (strstr(current_dir->d_name, ".xyz") != NULL) {
                printf("Map found, parsing: %s,\n", current_dir->d_name);
                current_fd = fopen(
                        (const char *) buff_append((uint8_t *) "maps/", (uint8_t *) current_dir->d_name, 5, 256), "r");
                if (current_fd == NULL) {
                    printf("%s: %s\n", ERR_LIST[8], current_dir->d_name);
                    continue;
                }
                //fgets(tmpBuff,30,current_fd);
                parse_height_map(current_fd);
            }

//        if (strcmp(".", current_dir->d_name) != 0 && strcmp("..", current_dir->d_name) != 0) {
//            //We dont want to read this and parent directory
//        }
        }
    }

    printf("\nFinished loading maps (time: %ld)\n", end_timer());

}

/**
 * @brief Loads height map to memory
 * @param current_fd
 */
void parse_height_map(FILE *current_fd) {
    //Get file size
    fseek(current_fd, 0, SEEK_END);//Put cursor at the end
    __attribute__((unused)) long size = ftell(current_fd);//Get it`s (file cursor) position ( == size in bytes)
    rewind(current_fd);//Move back to the beginning

    //char tmpBuff[30];//Line buffer
    float x = 0, y = 0, z = 0;

    //Get some more memory for the whole map
    pl92_point_alt_t *tmp = (pl92_point_alt_t *) realloc(map_H, (sizeof(pl92_point_alt_t) * (lines + 1)) +
                                                                (sizeof(pl92_point_alt_t) *
                                                                 (size / 24)));//Estimate buffer size
    if (tmp != NULL) {
        map_H = tmp;
    }
//    long file_line = 0;
    //Iterate line by line
    while (!feof(current_fd)) {
        //fgets(tmpBuff, sizeof(tmpBuff),current_fd)
        int status = fscanf(current_fd, "%f   %f   %f", &y, &x, &z);
        if (status != 3 && status != EOF) {
            printf("\nWarning: height map point omitted [%f %f %f]\n", y, x, z);
            continue;
        }

        //pl92_point_alt_t part;
        //part.x = (uint32_t) (x);
        //part.y = (uint32_t) (y);
        //part.alt = z;
        map_H[lines].x = (uint32_t) x;
        map_H[lines].y = (uint32_t) y;
        map_H[lines].alt = z;

        if(map_H[lines].alt > min_point.max_alt) min_point.max_alt = map_H[lines].alt;
        #if TERRAIN_MAPS_SEARCH_METHOD == 4
        //Get min point values
        if(map_H[lines].x < min_point.min_x) min_point.min_x = map_H[lines].x;
        if(map_H[lines].y < min_point.min_y) min_point.min_y = map_H[lines].y;
        if(map_H[lines].y > min_point.max_y) min_point.max_y = map_H[lines].y;
        if(map_H[lines].x > min_point.max_x) min_point.max_x = map_H[lines].x;
        #endif
        ++lines;


//        //Load every 8th point
//        if ((file_line & 7) == 0) {
//
//        }
//
//        ++file_line;
    }
    printf("Map loaded, size: %i bytes ", (int) (lines * sizeof(pl92_point_alt_t)));
    //printf("X:%i Y:%i Z:%f lines: %d\n", map_tree->root->map_point->x, map_tree->root->map_point->y,map_tree->root->map_point->alt, lines);
    tmp = (pl92_point_alt_t *) realloc(map_H, sizeof(pl92_point_alt_t) *
                                              (lines + 1));//Shrink down previously reserved space to save up memory
    if (tmp != NULL) {
        map_H = tmp;
    }
    printf("X:%i Y:%i Z:%f lines: %d\n", map_H[0].x, map_H[0].y, map_H[0].alt, lines);


}

/**
 * @deprecated
 */
void test_wgs_pl1992_convert() {
    //@TODO move it to tests files
    double B_stopnie = 48.00;
    double L_stopnie = 13.00;
    double Xpuwg;
    double Ypuwg;
    double B1_stopnie;
    double L1_stopnie;

    if (wgs84_do_puwg92(B_stopnie, L_stopnie, &Xpuwg, &Ypuwg) != 0) {
        printf("\nErr: invalid input params.\n");
    } else {
        printf("WGS 84 -> PUWG 1992\n");
        printf("B:%11.4f  L:%11.4f\n", B_stopnie, L_stopnie);
        printf("X:%11.4f  Y:%11.4f\n\n", Xpuwg, Ypuwg);
        //UWAGA - w ukladzie PUWG1992 wspolrzedna pozioma to Y, a pionowa to X

        puwg92_do_wgs84(Xpuwg, Ypuwg, &B1_stopnie, &L1_stopnie);
        printf("PUWG 1992 -> WGS 84\n");
        printf("X:%11.4f  Y:%11.4f\n", Xpuwg, Ypuwg);
        printf("B:%11.4f  L:%11.4f\n", B1_stopnie, L1_stopnie);
    }
}



