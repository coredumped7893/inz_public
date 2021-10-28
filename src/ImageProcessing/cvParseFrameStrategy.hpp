//
// Created by Maciek Malik
//

#ifndef MM_cvParseFrame_HPP_
#define MM_cvParseFrame_HPP_

#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include "cvParseFrameStrategy.h"
#include <iostream>
#include <vector>
#include <opencv2/core.hpp>


void parse_frame(cv::VideoCapture& stream);
void segment_frame(cv::Mat& frame, const status_frame& sF);

inline int get_next_x2(int offset, int X, double th);
inline int get_next_y2(int offset, int Y, double th);
double calc_buildings_angle(int x1, int y1, int x2, int y2, int width);
inline int max_pixel_val_idx(cv::Vec3b pixel);
bool is_pixel_equal(cv::Vec3b pixelA, cv::Vec3b pixelB);
static cv::Vec3b get_first_color(cv::Mat &frame);// Gets the background color that represents the free space - not obstacle
static inline int segment_by_color(cv::Vec3b* rowPtr, int col);
static inline int color_idx_from_pixel(cv::Vec3b *rowPtr, int col);
static void append_section_end(std::vector<obstacle_distance_ang_t> &sections, int x, int y, int width);

cv::Vec3b blue(0,0,255);
cv::Vec3b blueHalf  = blue/2;

cv::Vec3b green(0,255,0);
cv::Vec3b greenHalf = green/2;

cv::Vec3b red(255,0,0);
cv::Vec3b redHalf = red/2;

//get_first_color uses this array size
std::vector<cv::Vec3b> colors = {red,green,blue,redHalf,greenHalf,blueHalf};

cv::Vec3b black(0,0,0);
cv::Vec3b white(255,255,255);



obstacle_distance_ang_t *sections_to_ptr(std::vector<obstacle_distance_ang_t> &sections);

#endif //MM_cvParseFrame_HPP_
