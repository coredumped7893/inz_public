//
// Created by Maciek Malik
//

#include "virtualSim.h"

/**
 * @brief Init sources for UDP video stream
 * @param stream
 */
void virtual_sim_udp(cv::VideoCapture &stream) {
    stream.setExceptionMode(true);
    stream.open(VIDEO_SOURCE_PROTOCOL "://@" VIDEO_SOURCE_IP ":" VIDEO_SOURCE_PORT, cv::CAP_FFMPEG);
    print_message_local(3);
}

/**
 * benchmark data source, not needed in this case as it`s loaded from file
 * @param stream
 */
void virtual_sim_benchmark_frame(cv::VideoCapture &stream) {
//Data source not needed
}

/**
 * Open rtmp video stream, not used because of too much delay
 * @deprecated
 * @see virtual_sim_udp
 * @param stream
 */
void virtual_sim_rtmp(cv::VideoCapture &stream) {
}

