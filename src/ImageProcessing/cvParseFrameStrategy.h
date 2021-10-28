//
// Created by Maciek Malik
//

#ifndef MM_cvParseFrame_H_
#define MM_cvParseFrame_H_

#include <stdint.h>
#include "../config.h"
#include "imageProcessingStrategy.h"
#include "imageProcessing.h"


    #ifdef __cplusplus
    extern "C" {
    #endif

        void test_open_cv();
        void getOpenCVBuildInformation();
        uint8_t getStream(imageProcessingStrategy imageStrategy);

        //Make strategies visible to other main module(imageProcessing.c)
        void cvParseStrategy();
        void cvParseTestStrategy();


    #ifdef __cplusplus
    }
    #endif

#endif //MM_cvParseFrame_H_
