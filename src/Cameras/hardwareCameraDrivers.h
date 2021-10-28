//
// Created by Maciek Malik
//

#ifndef MM_hardwareCameraDrivers_H_
#define MM_hardwareCameraDrivers_H_

#include "../config.h"
#include "../Util/message.h"
#include <opencv2/core/mat.hpp>
#include <opencv2/videoio.hpp>

void hardware_camera_imx(cv::VideoCapture &stream);

void hardware_camera_ov(cv::VideoCapture &stream);


#endif //MM_hardwareCameraDrivers_H_
