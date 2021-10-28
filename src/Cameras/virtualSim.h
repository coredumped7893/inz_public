//
// Created by Maciek Malik
//

#ifndef MM_virtualSim_H_
#define MM_virtualSim_H_

#include "../config.h"
#include "../Util/message.h"
#include <opencv2/core/mat.hpp>
#include <opencv2/videoio.hpp>


void virtual_sim_udp(cv::VideoCapture &stream);

void virtual_sim_benchmark_frame(cv::VideoCapture &stream);

void virtual_sim_rtmp(cv::VideoCapture &stream);

#endif //MM_virtualSim_H_
