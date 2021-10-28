//
// Created by Maciek Malik
//

#include "terrain.h"
#include "maps.h"
#include "../ImageProcessing/imageProcessing.h"
#include "../ImageProcessing/wgs84_do_puwg92.h"
#include <math.h>
#include "../Util/time_diff.h"
#include "../Util/route_utils.h"
#include "kd_tree.h"

/**
 * @brief Index found in the last search
 * Allows Method 2 to speed up searching
 */
__attribute__((unused)) static long last_search_idx = 0;

/**
 * @brief How many data samples were taken. Used for calculation average execution time.
 * @see start_timer()
 */
static int data_samples = 0;
/**
 * @brief Average time to search point in the dataset
 * @see data_samples
 */
static double time_avg = 0.0;
/**
 * @see data_samples
 */
static long total_time = 0;



#if TERRAIN_MAPS_SEARCH_METHOD == 4
/**
 * File handler for regular mesh binary file with map data
 */
FILE *map_binary_regular = NULL;
#endif

uint8_t init_done = 0;

FILE *outLog;


void init_terrain() {

    //Logging to file for the first 300 number of cycles
    outLog = fopen("../collision_terrain_log.csv", "w");

    if (outLog == NULL) {
        printf("File error");
        exit(-1);
    }

    fprintf(outLog,"Altitude,IAS,HDG,LAT,LNG,Terrain Alt.\n");

#if TERRAIN_MAPS_SEARCH_METHOD == 4
    if (map_binary_regular == NULL) {
        map_binary_regular = fopen(BINARY_MAP_FILENAME_REGULAR,"rb");
        if (map_binary_regular == NULL) {
            //@TODO change to message with quit status
            printf("\nCould not open/create map file\n");
            exit(-1);
        }
    }
#endif
}




/**
 * Analyzes terrain on the current heading
 * 3 methods are available with measuring average execution time
 * @see TERRAIN_MAPS_SEARCH_METHOD
 */
void analyze_terrain() {
    //Load for demo B-d-1, B-d-2, B-b-3, B-b-4
    /*
    * MAX X: 785000
    * MIN X: 130000
    * MAX Y: 870000
    * MIN Y: 160000
     */
    ++data_samples;
    __attribute__((unused)) int i = 0;
    __attribute__((unused)) pl92_point_alt_t *map = get_map_ptr();
    status_frame status = get_status_frame();
    __attribute__((unused)) long map_size = get_map_lines();
    pl92_min_max_point minmax = get_min_max_point();
    if(!init_done){
        init_terrain();
        init_done = 1;
    }

    //Check map data boundaries
    if(status.lat < minmax.min_x || status.lng < minmax.min_y || status.lat > minmax.max_x || status.lng > minmax.max_y){
        return;//No data loaded for current position
    }


    position_puwg92_t intermediate_test_points[60];


    /*
     * create test points every 1 seconds of flight (up to 60)
     * @ASSERT drone speed is at least 1 m/s (1.95knots)
     */
    for(int pt = 0 ; pt < 60 ; ++pt){
        position_t tmp = point_from_dist_brg(status.lat, status.lng, ((status.ias) * 0.514) *(pt+1), status.hdg);
        wgs84_do_puwg92(tmp.lat,tmp.lng, &intermediate_test_points[pt].X, &intermediate_test_points[pt].Y);
    }


    start_timer();

//Uncomment for tests
//    status.lat = 49.25;
//    status.lng = 19.9375;
//
//    position_t test_point = point_from_dist_brg(status.lat, status.lng, rand()%4300, rand()%360);

    /**
     * Get point coordinates about one minute ahead
     */
    position_t test_point = point_from_dist_brg(status.lat, status.lng, ((status.ias) * 0.514) * 60, status.hdg);

    double x, y,currentX,currentY;

    printf("SPD: %f ALT: %f",status.ias, status.alt);

    wgs84_do_puwg92(status.lat, status.lng, &currentX, &currentY);

    wgs84_do_puwg92(test_point.lat, test_point.lng, &x, &y);
    //Convert it to puwg92 format to compare with map data
    wgs84_do_puwg92(test_point.lat, test_point.lng, &x, &y);
    printf("<%f,%f>(%.10lf,%.10lf)", y, x, test_point.lat, test_point.lng);

    __attribute__((unused)) long best_idx = -1; // using __attribute__((unused)) to suppress possible unused variable warnings
    __attribute__((unused)) double min_dist = 0;
    __attribute__((unused)) pl92_point_alt_t best_point_found;

#if TERRAIN_MAPS_SEARCH_METHOD < 3
    //    double lat,lng;
    //
    //    puwg92_do_wgs84(568098.0, 162768.0, &lat, &lng);
    //    printf("%f %f \n",lat,lng);

    //    int sth = 0;
    //    int sth2 = 0;
    //    unsigned long jst = 1;
    //
    //
    //
    //    for(int i = 0; i < 43000000; ++i){
    //
    //        sth = (pow(sth-sth2,2) + pow(i-sth2,2));
    //        jst *= (jst-1) * 2;
    //
    //    }

    /*
     * Iterations: 73700765 AVG time: 2532891.433333 Samples: 30 // Method no2
     * Iterations: 73700765 AVG time: 2748420.166667 Samples: 30 // Method no1
     *
     *
     *
     * //2 maps B-d-1 i B-b-3, cached, loaded every 8th point, full precision, (x86)
     * Iterations: 4606256 AVG time: 174068.640000 Samples: 50 //Method 1
     * Iterations: 4606256 AVG time: 157535.200000 Samples: 50 //Method 2
     * Iterations: 0 AVG time: 88.680000 Samples: 50 //Method 3
     *
     *
     *
     */



    /*
     *     status.lat = 49.267925636521994;
        status.lng = 19.92790592320758;
     */


#if TERRAIN_MAPS_SEARCH_METHOD == 1
        //Plain linear method [DONE] -----------------------------------------------------------------------------
        //Search for closest point
        //Best map line found so far

        //Get distance from the first point
         min_dist = dist_2d((float)map[0].x,(float)map[0].y,(float)x,(float)y);
        for(; i < map_size; ++i){
            double tmp_dist = dist_2d((float)map[i].x,(float)map[i].y,(float)x,(float)y);
            if(tmp_dist < min_dist){
                min_dist = tmp_dist;
                best_idx = i;
            }
        }
        best_idx = 0;
        //--------------------------------------------------------------------------------------------------------
#endif

#if TERRAIN_MAPS_SEARCH_METHOD == 2
        //Improved linear method [DONE] -----------------------------------------------------------------------
         min_dist = dist_2d((float)map[0].x,(float)map[0].y,(float)x,(float)y);
        for ( ;i < map_size; ++i) {
            long idxPOS = last_search_idx + i;
            long idxNEG = last_search_idx - i;
            if(idxNEG >= 0){
                double tmp_dist = dist_2d((float)map[idxNEG].x,(float)map[idxNEG].y,(float)x,(float)y);
                if(tmp_dist < min_dist){
                    min_dist = tmp_dist;
                    best_idx = idxNEG;
                    last_search_idx = idxNEG;
                    if(tmp_dist < TERRAIN_POINTS_EPSILON){
                        break; //Good enough
                    }
                }
            }
            if(idxPOS <= map_size){
                double tmp_dist = dist_2d((float)map[idxPOS].x,(float)map[idxPOS].y,(float)x,(float)y);
                if(tmp_dist < min_dist){
                    min_dist = tmp_dist;
                    best_idx = idxPOS;
                    last_search_idx = idxPOS;
                    if(tmp_dist < TERRAIN_POINTS_EPSILON){
                        break; //Good enough
                    }
                }
            }
        }
        best_idx = 0;
        //----------------------------------------------------------------------------------------------
#endif


        best_point_found = map[best_idx];

#elif TERRAIN_MAPS_SEARCH_METHOD == 3 //KD tree searching
    pl92_point_alt_t search1 = {.x = (uint32_t) x, .y = (uint32_t) y, .alt = 0};
    KD_Tree_t *t = get_tree_data();

    //If current alt is higher than maximum terrain level, skip searching - no collision possible
    if ((t->max.alt - MIN_AGL_HEIGHT) > status.alt) {
        best_idx = search_point(t, t->root, &search1, 0, tree_points_close_enough);
        best_point_found = (t->external_data + best_idx)->map_point;

        if (best_idx > 0 && ((status.alt - MIN_AGL_HEIGHT) <= best_point_found.alt)) {
            //Drone is too low
#if DEBUG > 0
            printf("\n---------------- [CDS - Terrain] ----------------\n");
            printf("Terrain col detected! acftH: %f terrH: %f dH: %f lat: %f lng: %f \n", status.alt,
                   best_point_found.alt, (best_point_found.alt - status.alt), test_point.lat, test_point.lng);
            printf(  "---------------- [CDS - Terrain] ----------------\n");
#endif
        }
    }

#elif TERRAIN_MAPS_SEARCH_METHOD == 4



    float map_alt;


    //If current alt is higher than maximum terrain level, skip searching - no collision possible
    if ((minmax.max_alt - MIN_AGL_HEIGHT) > status.alt) {
        int collision_detected = 0;
        double max_terr_alt = -1;

        obstacle_terrain *obs_terrain = calloc(1, sizeof(obstacle_terrain));

        for(int pt = 0 ; pt < 60 ; ++pt){
            pl92_point_alt_t search1 = {.x = (uint32_t) intermediate_test_points[pt].X, .y = (uint32_t) intermediate_test_points[pt].Y, .alt = 0};
            fseek(map_binary_regular, point_file_index(search1), SEEK_SET);
            if(fread(&map_alt, sizeof(float ), 1, map_binary_regular) <= 0){
                printf("Couldn't`t read a value from file");
                exit(-1);
            }

            if ( ((status.alt - MIN_AGL_HEIGHT) <= map_alt)) {
                //Drone is too low

                collision_detected = 1;

                if(map_alt > max_terr_alt){
                    //Select the highest point
                    obs_terrain->colliding_point = test_point;
                    obs_terrain->dHeight = (map_alt - status.alt);
                    obs_terrain->distanceToTerrain = (float) (((status.ias) * 0.514) *(pt+1));
                    obs_terrain->time_to_terrain = (pt+1);
                    max_terr_alt = map_alt;
                }

            }

        }


        #if RECORD_COLLISION

        float current_terrain_alt;
        pl92_point_alt_t search1 = {.x = (uint32_t) currentX, .y = (uint32_t) currentY, .alt = 0};
        fseek(map_binary_regular, point_file_index(search1), SEEK_SET);
        if(fread(&current_terrain_alt, sizeof(float ), 1, map_binary_regular) <= 0){
            printf("Couldn't`t read a value from file");
            exit(-1);
        }

        fprintf(outLog,"%f,%f,%f,%f,%f,%f\n",status.alt,status.ias,status.hdg,status.lat,status.lng,current_terrain_alt);
        fflush(outLog);

        #endif


        if(collision_detected){

            #if DEBUG > 0

            printf("My alt: %f\n", status.alt);
            printf("Value read: %f\n", map_alt);
            printf("MAX ALT: %f\n\n",minmax.max_alt);


            printf("\n---------------- [CDS - Terrain] ----------------\n");
            printf("Terrain col detected! acftH: %f terrH: %f dH: %f time to collsision: %d lat: %f lng: %f \n", status.alt,
                   map_alt, (map_alt - status.alt), obs_terrain->time_to_terrain ,test_point.lat, test_point.lng);
            printf(  "---------------- [CDS - Terrain] ----------------\n");
            #endif

            obs_terrain->bearing = status.hdg;
            collision_packet col_p;
            col_p.type = C_TERRAIN;
            col_p.sections_size = 1;
            col_p.terrain = obs_terrain;
            send_collision_advisory(&col_p);
        }else{
            free(obs_terrain);
        }

    }else{
    #if DEBUG > 0
        printf("Skipping alt checking, drone above terrain\n");
    #endif
    }



#endif


    long elapsed = end_timer();
    total_time += elapsed;
    time_avg = (double) total_time / (double) data_samples;
    printf("Exec time: %ld micro seconds\n", elapsed);
    #if TERRAIN_MAPS_SEARCH_METHOD == 3
        printf("Point found on line: %ld, values are X: %d Y: %d ALT: %f\n", best_idx, best_point_found.x,best_point_found.y, best_point_found.alt);
    #endif
    printf("Iterations: %d AVG time: %lf Samples: %d\n\n", i, time_avg, data_samples);


}

