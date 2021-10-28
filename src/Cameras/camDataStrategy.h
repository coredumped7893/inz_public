//
// Created by Maciek Malik
//

#ifndef MM_camDataStrategy_H_
#define MM_camDataStrategy_H_

#include <opencv2/core/mat.hpp>
#include <opencv2/videoio.hpp>

//Include here any strategies header files ----------
#include "virtualSim.h"
#include "hardwareCameraDrivers.h"
//---------------------------------------------------


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief C style strategy
 */
typedef void (*camera_data_strategy)(cv::VideoCapture &stream);





#ifdef __cplusplus
}



#endif







#endif //MM_camDataStrategy_H_
