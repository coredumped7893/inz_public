//
// Created by Maciek Malik
//

#ifndef MM_config_H_
#define MM_config_H_

/**
 * Defines environment; 1 = simulated; 0 = real hardware (not implemented yet)
 */
#define SIM 1

/**
 * Enables printing out to the console debug messages
 * Accepted values: 0,1,2
 */
#define DEBUG 1

#if SIM == 1
    #define IMG_PROC_MODULE_IP "127.0.0.1"
#else
    #define IMG_PROC_MODULE_IP "192.168.1.90"
#endif
#define IMG_PROC_MODULE_PORT "2137"

//DataInput ---------------------------
/**
 * IP address on which x-plane instance is listening
 */
#define X_PLANE_IP "127.0.0.1"
/**
 * @see X_PLANE_IP
 */
#define X_PLANE_PORT "49000"
#define NUMBER_OF_DATA_INPUTS 25
/**
 * Default incoming data frequency from the sim
 */
#define DEFAULT_DATA_FREQ 20
/**
 * How much data remember from the past, must be integer
 */
#define DATA_DELAY_SECONDS 1
//-------------------------------------

//App Route ---------------------------
#define FIRST_POINT_EPSILON 0.0015
/**
 * Print all the points after route init
 */
#define ROUTE_DUMP 0
/**
 * Prints message in the terminal what point is currently targeted and on what heading
 */
#define PRINT_TARGETING_MSG 1

#define DUMP_CSV_TARGETING_MSG 1

/**
 * Disables horizontal route following, drone keeps steady heading defines in aircraft config
 */
#define DISABLE_HEADING_FOLLOWING 0

/**
 * Disables following altitude along the route
 */
#define DISABLE_ALTITUDE_FOLLOWING 1

/**
 * @brief enables Wind Correction Angle calculation
 * requires valid wind data
 */
#define ENABLE_WCA_CALCULATION 0

//-------------------------------------

//Image Processing --------------------------
#define VIDEO_SOURCE_PROTOCOL "udp"
#define VIDEO_SOURCE_IP ""
#define VIDEO_SOURCE_PORT "60000"

/**
 * If == 1 then full frame will be segmented otherwise only small section
 * around horizon would be processed
 */
#define SEGMENT_FULL_FRAME 0

/**
 * Parse every X pixels
 */
#define SEGMENT_PIXEL_STEP 1

/**
 * How much segment from the middle
 */
#define SEGMENT_VERTICAL_POSITIVE_OFFSET 25

/**
 * @see SEGMENT_VERTICAL_POSITIVE_OFFSET
 */
#define SEGMENT_VERTICAL_NEGATIVE_OFFSET 25

/**
 * Offset from the middle of the frame
 */
#define SEGMENT_DETECTION_POSITIVE_OFFSET 3

/**
 * @see SEGMENT_DETECTION_POSITIVE_OFFSET
 */
#define SEGMENT_DETECTION_NEGATIVE_OFFSET 3

/**
 * Offset from middle to increase separation from horizon
 * decreases number of false positives
 */
#define SEGMENT_DETECTION_OFFSET (-11)

/**
 * Get background color from random points or from hardcoded ones
 */
#define SKY_COLOR_GET_RANDOM 1

/**
 * Number of random points used to determine background color
 * @see SKY_COLOR_GET_RANDOM
 */
#define SKY_COLOR_GET_RANDOM_NUMBERS_COUNT 6

/**
 * How many random rows can be accessed
 * @see SKY_COLOR_GET_RANDOM
 */
#define SKY_COLOR_RANDOM_ROWS 2

/**
 * How many detected points in one column is considered as a collision
 * By default collision pixels are checked across 3 horizontal lines.
 * This value should be more than 50% of the total number of horizontal lines
 */
#define COLLISIONS_PER_POINT 2

/**
 * When 1, absolute pixel values will be used
 */
#define BUILDING_DETECTION_USE_ABSOLUTE_PIXEL_VALUES 0

/**
 * count collision points X amount of pixels from the middle
 */
#define BUILDING_DETECTION_BOUNDING_RECTANGLE_X1 300

/**
 * @see BUILDING_DETECTION_BOUNDING_RECTANGLE_X1
 */
#define BUILDING_DETECTION_BOUNDING_RECTANGLE_X2 300

/**
 * How much percent to the left and right will be analyzed (percent of the frame width)
 */
#define BUILDING_DETECTION_BOUNDING_RECTANGLE_PERCENT 28

/**
 * How much points inside bounding rectangle needs to be positive to consider it as collision
 */
#define BUILDING_DETECTION_MIN_POINTS_PERCENT 60

/**
 * Save newest frame to the file as image
 */
#define SAVE_PROCESSED_FRAME 1

/**
 * Create video from the input frames (10 fps)
 * @see cv::outMovie()
 */
#define SAVE_FRAMES_TO_VIDEO 1

/**
 * Save original frames - before segmentation
 */
#define SAVE_ORIGINAL_FRAMES 0

/**
 * Print sections after detecting collisions (only for building type)
 */
#define PRINT_SECTIONS 1


#define DISABLE_IMAGE_SEGMENTATION 0

/**
 * Draw white or black dots on the segmented frame, should be used with SAVE_PROCESSED_FRAME=1
 */
#define WRITE_ON_FRAME 1

/**
 * Draw yellow rectangle that marks the region where building are detected
 */
#define DRAW_DETECTION_ZONE 1

/**
 * Enables debug info about pixels detected their position
 */
#define WRITE_DETECTING_FROM 0

//-------------------------------------------

//Collision module---------------------------
/**
 * Min angle in section to be able to squeeze between buildings
 */
#define MIN_SECTION_ANGLE 10

#define SELECT_COLLISION_STRATEGY_DYNAMICALLY 1

/**
 * Terrain will be avoided with additional safety margin
 */
#define TERRAIN_SAFETY_MARGIN 1.11

/**
 * @brief Additional time (in seconds) after which collision strategy will be turned off
 */
#define BUILDING_RESET_TIME 2

//-------------------------------------------

//SIM Config/Data----------------------------
/**
 * Field Of View of the simulator
 */
#define SIM_FOV 85
//-------------------------------------------

//MAPS --------------------------------------
/**
 * 0 - Disables loading maps into memory and any terrain checking
 */
#define ENABLE_MAPS 1

/**
 * 1 - plain linear(slowest);
 * 2 - improved linear;
 * 3 - 2d search tree(fast);
 * 4 - get data directly form binary file based on its coordinates (fast and recommended)
 * @remark On rPI only method 3 or 4 should be used otherwise performance would be very degraded
 */
#define TERRAIN_MAPS_SEARCH_METHOD 4

/**
 * Threshold by which searched point can be considered as found
 */
#define TERRAIN_POINTS_EPSILON 200

/**
 * @deprecated
 */
#define REDUCE_MEMORY_USAGE 0 //if 1 then file will be iterated and excat number of lines will be counted otherwise overshooting estimate would be made

/**
 * Loads maps and then saves it as packed binary file
 * If set to 1 then only binary map file would be loaded
 * @remark Should be disabled for rPI
 */
#define CONVERT_MAPS_TO_BINARY 1

/**
 * Filename + path to the binary file compressed with the maps data
 */
#define BINARY_MAP_FILENAME "maps/map.bmap"

/**
 * path to the binary file saved as regular mesh of points
 */
#define BINARY_MAP_FILENAME_REGULAR "maps/map_regular.bmap"

/**
 * Default width of the map in meters. Used only if SEARCH_METHOD == 4
 */
#define BINARY_MAP_REGULAR_DEFAULT_WIDTH 2000

/**
 * if SEARCH_METHOD == 3 then save created tree as binary data using relative offsets
 * if set to 1 and SEARCH_METHOD == 3 then binary file will be loaded by default - normal ascii map would no load
 * @remark Should be disabled for rPI
 */
#define SAVE_TREE_TO_BINARY 1

/**
 * Mesh resolution of the original maps (input data) in meters, used to calculate correct position
 * in the binary map file. Used only if SEARCH_METHOD == 4
 */
#define MAP_SOURCE_MESH_RESOLUTION 1

/**
 * Filename + path to the binary file with tree build from maps data
 */
#define TREE_BINARY_FILENAME "maps/map_tree.btree"

/**
 * @deprecated
 */
#define X_RANGE_CONSTANT 30000
/**
 * @deprecated
 */
#define Y_RANGE_CONSTANT 50000

/**
 * @brief Minimum height above terrain that is considered safe.
 * By default: 5 meters
 */
#define MIN_AGL_HEIGHT 5

/**
 * Loads all maps in the folder or calculates what files to load
 * Currently only loading all maps is supported
 *
 */
#define LOAD_ALL_MAPS 1

/**
 * Enables recording to a .csv file
 */
#define RECORD_COLLISION 1

#define DISABLE_TERRAIN_ANALYSIS 0

//-------------------------------------------

//Other -------------------------------------
#define PI 3.141592
/**
 * Enables profiler that logs process information to the csv file (data is read from /proc/self filesystem)
 */
#define ENABLE_PROFILER 1
#define PROFILER_DATA_DEBUG 0
//-------------------------------------------


/**
 * @brief Status frame containing most of the flight parameters
 * @see alloc_frame()
 */
typedef struct  __attribute((packed)) {
    /**
     * Latitude [deg]
     * @remark dataref: sim/flightmodel/position/latitude
     */
    float lat;

    /**
     * Longitude [deg]
     * @remark dataref: sim/flightmodel/position/longitude
     */
    float lng;

    /**
     * Heading [deg]
     * @remark dataref: sim/flightmodel/position/mag_psi
     */
    float hdg;

    /**
     * Indicated airspeed [kias]
     * @remark dataref: sim/flightmodel/position/indicated_airspeed
     */
    float ias;

    /**
     * Groudspeed [m/s]
     * @remark dataref: sim/flightmodel/position/groundspeed
     */
    float gs;

    /**
     * Altitude above MSL [m]
     * @remark dataref: sim/flightmodel/position/elevation
     */
    float alt;

    /**
     * Vertical speed [m/s]
     * @remark dataref: sim/flightmodel/position/vh_ind
     */
    float vsi;

    /**
     * Pitch relative to the earth [deg]
     * @remark dataref: sim/flightmodel/position/true_theta
     */
    float pitch;

    /**
     * Roll relative to the earth [deg]
     * @remark dataref: sim/flightmodel/position/true_phi
     */
    float roll;

    /**
     * Rotation on pitch axis [deg/s]
     * dataref: sim/flightmodel/position/Q
     */
    float pitch_ratio;

    /**
     * Rotation on roll axis [deg/s]
     * @remark dataref: sim/flightmodel/position/P
     */
    float roll_ratio;

    /**
     * Rotation on yaw axis [deg/s]
     * @remark dataref: sim/flightmodel/position/R
     */
    float yaw_ratio;
}status_frame;
status_frame* alloc_frame();

/**
 * @brief Point on map (coordinates in degrees)
 */
typedef struct{
    /**
     * X in PUWG92
     */
    float lat;

    /**
     * Y in PUWG92
     */
    float lng;
}position_t;

typedef struct {
    /**
     * @brief PUWG92 X corrdinate
     */
    double X;

    /**
     * @brief PUWG92 Y coordinate
     */
    double Y;
}position_puwg92_t;


/**
 * @brief Request data structure to send data to other modules
 * @see th_image_data_send()
 */
typedef struct __attribute((packed)){
    /**
     * Short id representing the type of the data
     * ex. "STAT" or "POS"
     */
    char cmd[5];
    union{
        position_t position;
        status_frame status;
    };
}module_request_t;

typedef struct __attribute((packed)){
    char cmd[4];//Response code/command
    float value;
}module_response_t;


//For tests ----------------------------
#ifdef TEST
    #define TESTING_REQ_WEAK __attribute__((weak))
#else
    #define TESTING_REQ_WEAK
#endif

#ifndef TEST
    #define static_testable static
#else
    #define static_testable
#endif

#endif //MM_config_H_
