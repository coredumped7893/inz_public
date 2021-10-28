//
// Created by Maciek Malik
//

#include "cvParseFrameStrategy.hpp"

#include "../Util/message.h"
#include "stdlib.h"
#include <time.h>
#include <unistd.h>
#include <map>
#include <unordered_map>
#include <random>
#include <opencv2/imgproc.hpp>
#include "../Util/other.h"
#include "cvParseFrameStrategy.h"
#include "../Cameras/cameras.h"

using namespace std;
using namespace cv;
void draw_terrain_crosssection(cv::Mat &frame);


/**
 * @brief Opens Video stream and starts parsing it
 *
 * @return
 * @see parse_frame
 */
uint8_t getStream(imageProcessingStrategy imageStrategy) {

    //Execute passed strategy
    imageStrategy();

    return 1;
}

/**
 * @brief Main strategy, opens video stream then tries to parse it.
 */
void cvParseStrategy() {
//OpenCV strategy -------------------------------------
    try {
        VideoCapture stream;
        //Create camera context to open data stream
        camera_controls_t camera_ctx = create_camera_context(VIRTUAL_UDP);
        camera_ctx.init_source(stream);
        if (!stream.isOpened()) {
            throw_err(10, 0);
        }
        parse_frame(stream);
    } catch (cv::Exception &exception) {
        std::cout << "Could not open video stream" << std::endl;
        std::cout << "Caught exception: " << exception.what() << std::endl;
    }
//-----------------------------------------------------
}

/**
 * @brief Strategy used for testing.
 * Can record output movie, measure average execution time for single frame and test segmentation results
 */
void cvParseTestStrategy() {
//Benchmark test
    cv::Mat img = imread("../tmp_images/Selection_241.png");
    if (img.empty()) {
        exit(5);
    }

#if DEBUG && SAVE_FRAMES_TO_VIDEO
    cv::VideoWriter outMovie;
#endif

    status_frame sF = get_status_frame();
    sF.roll = 11.372;
    sF.pitch = -0.698;
    std::chrono::duration<double> elapsed_seconds{};
    auto start = std::chrono::system_clock::now();
    for (int i = 0; i < 1; ++i) {
        //usleep(50000);//delay for testing
    }
    segment_frame(img, sF);//Should yield collision advisory
    auto end = std::chrono::system_clock::now();
    elapsed_seconds = end - start;
    std::cout << "Time: " << elapsed_seconds.count() << endl;
    std::cout << "TimeAVG: " << elapsed_seconds.count() / 100.0 << endl;
    cv::imwrite("outTest.png", img);
//
//    exit(0);
}

/**
 * @brief Prints openCV build information
 */
void getOpenCVBuildInformation() {
    std::cout << cv::getBuildInformation() << std::endl;
}

/**
 * @brief Gets single frame and apply color segmentation alg.
 * @param stream
 */
void parse_frame(VideoCapture &stream) {
    //@TODO parameter type to &Mat, increases flexibility for other implementations
    float coeff = 1;

    /**
     * Extracted frame from the video stream
     */
    Mat frame;

#if DEBUG && SAVE_FRAMES_TO_VIDEO
    status_frame sF = get_status_frame();

//    string ffmpegCommand = "ffmpeg -y -f rawvideo -vcodec rawvideo -framerate 10 -pix_fmt bgr24 -s 320x180 -i - -c:v h264_nvenc -crf 14 -maxrate:v 10M -r 10 myVideoFile.mkv";
//
//    FILE *pipeout = popen(ffmpegCommand.data(), "w");


    string fName = "outmovie1.avi";
    int cc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
//    int cc = cv::VideoWriter::fourcc('X', 'V', 'I', 'D');
//    int cc = cv::VideoWriter::fourcc('r', 'g', 'b', 'a');
    stream >> frame;//Get first frame
    cv::VideoWriter outMovie(fName, cc, 10, cv::Size(frame.cols, frame.rows), true);
#endif

    //auto start = std::chrono::system_clock::now();

    while (stream.isOpened()) {
        status_frame sF = get_status_frame();
        if (fabs(sF.roll) < 2) {
            coeff = 0.1;
        }else{
            coeff = 1;
        }
        sF.roll *= coeff;
        stream >> frame;//Get next frame

        if(!frame.isContinuous()){
            cout << "Not cont!!";
            exit(25);
        }

        if(frame.empty()){
            cout << "Empty!!";
            exit(25);
        }

        Mat cpyFrame = frame.clone();

        #if !DISABLE_IMAGE_SEGMENTATION
                segment_frame(cpyFrame, sF);
        #endif

#if DEBUG && SAVE_FRAMES_TO_VIDEO

    #if !SAVE_ORIGINAL_FRAMES
            //fwrite(frame.data, frame.cols*frame.rows*3, 1, pipeout);
//        draw_terrain_crosssection(cpyFrame);
            outMovie.write(cpyFrame);//Save segmented frame
    #else
    outMovie.write(frame);//Save original frame
    #endif

#endif
    }


#if DEBUG && SAVE_FRAMES_TO_VIDEO
    outMovie.release();
#endif

}

void draw_terrain_crosssection(cv::Mat &frame){

    Point start_point(0,0);
    Point end_point(105,59);//keep 16:9 ratio

    cv::rectangle(frame, start_point, end_point, cv::Scalar(255,255,255), -1 );
}

/**
 * @brief Get index for colors array from single pixel
 * @param rowPtr - pointer to the beginning of the row
 * @param col - current column of the frame (X axis value)
 * @see colors
 * @return
 */
static inline int color_idx_from_pixel(cv::Vec3b *rowPtr, int col) {
    int maxIdx = max_pixel_val_idx(rowPtr[col]);//Get index of the brightest pixel channel
    if (rowPtr[col][maxIdx] < 127) {
        return (maxIdx + 3);
    } else {
        return maxIdx;
    }
}

/**
 * @brief Apply new color for given pixel - color segmentation
 * @param rowPtr
 * @param col - current column on the row pointer
 * @see color_idx_from_pixel()
 * @return
 */
static inline int segment_by_color(cv::Vec3b *rowPtr, int col) {
    //Get max brightness channel in each pixel
    int tmp = color_idx_from_pixel(rowPtr, col);
    rowPtr[col] = colors[tmp];
    return tmp;
}


/**
 * @brief Segments the frame to color clusters, detects buildings and calculates the angles between them
 * @param frame
 * @param sF
 */
void segment_frame(Mat &frame, const status_frame &sF) {

    int height = frame.rows, width = frame.cols;
    double rollRad = -sF.roll * PI / 180.0;
    int pitchOffset = (int) (((double) height / (double) SIM_FOV) * sF.pitch) * (1 + sin(rollRad));
    int x1 = width >> 1;
    int y1 = (height >> 1) + pitchOffset;
    int row = 0;
    int middleX = (width >> 1);
#if SEGMENT_FULL_FRAME == 0 //Segment only part of the frame so save CPU time

    int startY = get_next_y2(y1, 2 * middleX, rollRad);

    if (startY > height) {
        startY = height - 1;
    }
    if (startY < 0) {
        startY = 0;
    }
    int endY = get_next_y2(y1, -middleX, rollRad);
    if (endY > height) {
        endY = height - 1;
    }
    if (endY < 0) {
        endY = 0;
    }

    row = max(min(startY, endY) - SEGMENT_VERTICAL_NEGATIVE_OFFSET, 0);
    height = min(max(startY, endY) + SEGMENT_VERTICAL_POSITIVE_OFFSET, height);

#else
    int startY = 0;
    int endY = height - 1;
#endif

    for (; row < height; ++row) {
        auto *rowPtr = frame.ptr(row);//Get a pointer to current row
        for (int col = 0; col < width; ++col) {
            segment_by_color((Vec3b*) rowPtr, col);
        }
    }


    //Building detection and calculating angular distances between them
    bool first = true;
    std::vector<obstacle_distance_ang_t> sections;
    cv::Vec3b firstColor;
    bool section_start = true;
    int collisionsDetected = 0;
    int potentialCollisionPoints = 0;
    double loopCond = (((float) (width >> 1)) * (1 + fabs(sin(rollRad))));//Loop run condition

#if BUILDING_DETECTION_USE_ABSOLUTE_PIXEL_VALUES
    int posMax = (width>>1) + BUILDING_DETECTION_BOUNDING_RECTANGLE_X2;
    int posMin = (width>>1) - BUILDING_DETECTION_BOUNDING_RECTANGLE_X1;
#else
    int posMax = (float) (width >> 1) * (float) ((100 + BUILDING_DETECTION_BOUNDING_RECTANGLE_PERCENT) / 100.0);
    int posMin = (float) (width >> 1) * (float) ((100 - BUILDING_DETECTION_BOUNDING_RECTANGLE_PERCENT) / 100.0);
#endif

    for (int xPos = (-(float) (width >> 1)) * (1 + fabs(sin(rollRad))); xPos < loopCond; xPos += SEGMENT_PIXEL_STEP) {

        int x = get_next_x2(x1, xPos, rollRad);
        int y = get_next_y2(y1, xPos, rollRad) + SEGMENT_DETECTION_OFFSET;

        int yUp = y + SEGMENT_DETECTION_POSITIVE_OFFSET;
        int yDown = y - SEGMENT_DETECTION_NEGATIVE_OFFSET;

        if (x >= width || y >= height || yUp >= height || yDown >= height) {
            continue;
        }

        if (x < 0 || y < 0 || yDown < 0 || yUp < 0) {
            continue;
        }

        if (first) {
            first = false;
//            firstColor = frame.at<cv::Vec3b>(min(rowInitial, endY) + 1, 10);
            firstColor = get_first_color(frame);
        } else {


            short collisions = 0;//If  >= COLLISIONS_PER_POINT then consider that point as collision
            //Upper scan
            auto &tmpPixelUp = frame.at<cv::Vec3b>(yUp, x);
            if (is_pixel_equal(tmpPixelUp, firstColor)) {
#if DEBUG > 0 && WRITE_ON_FRAME
                tmpPixelUp = white;
#endif
            } else {
                ++collisions;
#if DEBUG > 0 && WRITE_ON_FRAME
                tmpPixelUp = black;
#endif
            }

            auto &tmpPixel = frame.at<cv::Vec3b>(y, x);
            //Middle scan
            if (is_pixel_equal(tmpPixel, firstColor)) {
#if DEBUG > 0 && WRITE_ON_FRAME
                tmpPixel = white;
#endif
            } else {
                ++collisions;
#if DEBUG > 0 && WRITE_ON_FRAME
                tmpPixel = black;
#endif
            }

            //Lower scan
            auto &tmpPixelDown = frame.at<cv::Vec3b>(yDown, x);
            if (is_pixel_equal(tmpPixelDown, firstColor)) {
#if DEBUG > 0 && WRITE_ON_FRAME
                tmpPixelDown = white;
#endif
            } else {
                ++collisions;
#if DEBUG > 0 && WRITE_ON_FRAME
                tmpPixelDown = black;
#endif
            }

            //Calculate free space between buildings/obstacles ------------------------------
            if (collisions >= COLLISIONS_PER_POINT) {
                if (sections.empty()) {
                    continue;//Skip if empty
                }
                if (!section_start) {
                    append_section_end(sections, x, y, width);
                    section_start ^= 1;//Set to true
                }
            } else {
                if (section_start) {
                    obstacle_distance_ang_t tmp{x, y, 0, 0, 0};
                    //Append section start
                    sections.push_back(tmp);
                    section_start ^= 1;//Set to false (XOR bit flip)
                } else if (x >= (width - 1)) {
                    //Fix for one sections being incorrectly set to zero (when only one colliding object has been found)
                    append_section_end(sections, x, y, width);
                    section_start ^= 1;//Set to true (XOR bit flip)
                }
            }
            //-------------------------------------------------------------------------------


#if SAVE_FRAMES_TO_VIDEO && DRAW_DETECTION_ZONE
            //Draws yellow rectangle around section of the frame when collision is detected
            cv::Rect r(posMin,(height>>1)-40,(posMax-posMin),80);
            cv::rectangle(frame,r, cv::Scalar(0,255,255));
#endif


            //Count how many points has obstacle in front of the drone ----------------------
            if (((middleX + xPos) > posMin && (middleX + xPos) < posMax)) {
                ++potentialCollisionPoints;
                if (collisions >= COLLISIONS_PER_POINT) {
                    ++collisionsDetected;
                }
            }
            //-------------------------------------------------------------------------------


        }

    }

    double detection_threshold = (BUILDING_DETECTION_MIN_POINTS_PERCENT / 100.0) * potentialCollisionPoints;

    //Final check if building was detected
    if (detection_threshold > 0 && collisionsDetected >= detection_threshold) {
        collision_packet p;
        p.type = C_BUILDING;
        p.size.x = width;
        p.size.y = height;
        p.sections_size = static_cast<int>(sections.size());
        p.sections = sections_to_ptr(sections);

#if DEBUG > 0
        std::cout << "---------------- [CDS - Building] ----------------" << endl;
        std::cout << "Collision Detected!" << endl;
        std::cout << "---------------- [CDS - Building] ----------------" << endl;
#endif


#if DEBUG && PRINT_SECTIONS > 0
        for (auto i:sections) {
            std::cout << "<" << i.startX << ":" << i.startY << "," << i.endX << ":" << i.endY << "[" << i.distanceDeg
                      << "]"
                      << " | ";
        }
        std::cout << ">" << endl;
#endif
        //Send info about collision to the FlightManager module
        send_collision_advisory(&p);
    }

#if DEBUG > 0 && WRITE_DETECTING_FROM
    std::cout << "points detected: " << collisionsDetected << " out of: " << potentialCollisionPoints << " threshold: "
              << detection_threshold << endl;
    std::cout << "detecting From: " << posMin << " To: " << posMax << endl;
#endif

    //Test frame
#if DEBUG && SAVE_PROCESSED_FRAME > 0
    cv::imwrite("out.png", frame);
#endif

}

/**
 * @brief Appends end of the section.
 * Sets end X and Y coordinates and calculated angle between buildings
 * @see calc_buildings_angle()
 * @param sections
 * @param x
 * @param y
 * @param width
 */
static void append_section_end(std::vector<obstacle_distance_ang_t> &sections, int x, int y, int width) {
    obstacle_distance_ang_t &tmp = sections.at(sections.size() - 1);
    tmp.endX = x;
    tmp.endY = y;
    tmp.distanceDeg = calc_buildings_angle(tmp.startX, tmp.startY, x, y, width);
}

/**
 * @brief Calculate X coordinate with given offset from the middle of the frame to follow horizon line
 * @param offset - static offset
 * @param X - dynamic offset from the middle
 * @param th - roll angle in radians
 * @return
 */
inline int get_next_x2(int offset, int X, double th) {
    return (int) (offset + X * cos(th));
}

/**
 * Calculate Y coordinate with given offset from the middle of the frame to follow horizon line
 * @param offset - static offset
 * @param X - dynamic offset from the middle
 * @param th - roll angle in radians
 * @return
 */
inline int get_next_y2(int offset, int Y, double th) {
    return (int) (offset + Y * sin(th));
}

/**
 * @brief Calculates angle between buildings
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @param width
 * @see append_section_end()
 * @see SIM_FOV
 * @return
 */
double calc_buildings_angle(int x1, int y1, int x2, int y2, int width) {
    double fx1 = (abs(x1 - x2));
    double fy1 = (abs(y1 - y2));
    return ((float) SIM_FOV / (float) width) * sqrt(pow(fx1, 2) + pow(fy1, 2));
}

/**
 * @brief Returns index of the biggest value from vector
 * @param pixel cv::Vec3b
 * @return
 */
inline int max_pixel_val_idx(cv::Vec3b pixel) {
    if (pixel[0] > pixel[1] && pixel[0] > pixel[2]) {
        return 0;
    }
    if (pixel[1] > pixel[0] && pixel[1] > pixel[2]) {
        return 1;
    }
    return 2;
}

/**
 * @brief Checks if two pixels are the same (values on every channel are equal)
 * @param pixelA cv::Vec3b
 * @param pixelB cv::Vec3b
 * @return
 */
bool is_pixel_equal(cv::Vec3b pixelA, cv::Vec3b pixelB) {
    if (pixelA[0] == pixelB[0] && pixelA[1] == pixelB[1] && pixelA[2] == pixelB[2]) {
        return true;
    }
    return false;
}

/**
 * @brief Allocates sections local var to heap
 * @param sections
 * @return
 */
obstacle_distance_ang_t *sections_to_ptr(std::vector<obstacle_distance_ang_t> &sections) {
    //@TODO dont create buffer every time function is called - reuse old one?
    obstacle_distance_ang_t *memPtr = (obstacle_distance_ang_t *) calloc(sections.size(), sizeof(obstacle_distance_ang_t));
    if (memPtr == nullptr) {
        throw_err(3, 1);
    }
    memcpy(memPtr, (obstacle_distance_ang_t *) sections.data(), sections.size() * sizeof(obstacle_distance_ang_t));
    return memPtr;
}

/**
 * @brief Gets the background color that represents the free space - not obstacle.
 * Estimated based on few points from the top of the frame, can be random or predefined
 * @see segment_frame()
 * @param frame
 * @return
 */
cv::Vec3b get_first_color(Mat &frame) {
    // Get few points from the top corners and the top-middle
    int occur[6] = {0};
#if SKY_COLOR_GET_RANDOM
    cv::Vec3b *rowPtr;
    srand(time(nullptr)); // NOLINT(cert-msc51-cpp)
    for (int i = 0; i < SKY_COLOR_RANDOM_ROWS; ++i) {
        rowPtr = frame.ptr<cv::Vec3b>(2 + (rand() % 6)); // NOLINT(cert-msc50-cpp)
        for (int j = 0; j < SKY_COLOR_GET_RANDOM_NUMBERS_COUNT; ++j) {
            ++occur[segment_by_color(rowPtr, (rand() % frame.cols))]; // NOLINT(cert-msc50-cpp)
        }
    }
#else

    auto rowPtr = frame.ptr<cv::Vec3b>(3);
    ++occur[segment_by_color(rowPtr,5)];
    ++occur[segment_by_color(rowPtr,frame.cols>>1)];
    ++occur[segment_by_color(rowPtr,frame.cols-5)];
    rowPtr = frame.ptr<cv::Vec3b>(6);
    ++occur[segment_by_color(rowPtr,6)];
    ++occur[segment_by_color(rowPtr,frame.cols>>1)];
    ++occur[segment_by_color(rowPtr,frame.cols-6)];

#endif
    return colors.at(std::distance(occur, std::max_element(occur, occur + 6)));
}

/**
 * @test
 */
void test_max_pixel_val_idx() {
    cout << max_pixel_val_idx(cv::Vec3b(0, 0, 255)) << endl;
    cout << max_pixel_val_idx(cv::Vec3b(125, 255, 2)) << endl;
    cout << max_pixel_val_idx(cv::Vec3b(211, 78, 155)) << endl;
    cout << max_pixel_val_idx(cv::Vec3b(22, 22, 22)) << endl;
}

/**
 * @test
 */
void test_open_cv() {
    cout << "OpenCV version : " << CV_VERSION << endl;

    cout << "Major version : " << CV_MAJOR_VERSION << endl;

    cout << "Minor version : " << CV_MINOR_VERSION << endl;

    cout << "Subminor version : " << CV_SUBMINOR_VERSION << endl;
}




























