//
// Created by Maciek Malik
//

#ifndef MM_sf50_H_
#define MM_sf50_H_


__attribute__((used)) static aircraftConfig_t sf50 = {
    "Cirrus SF-50",

    {0.07f,0.1f,0.0f,0.01f,-1.0f,1.0f,0.009f,0.009f},
    {0.0433f,0.533f,0.00f,0.01f,-0.7f,0.8f,0.001f,0.001f},
    {0.06f,0.38f,0.0f,0.01f,0.0f,1.0f,0.001f,0.001f},

    {0.61f,0.095f,2.0f,0.01f,-30.0f,30.0f,0.9f,0.9f},
    {0.33f,0.195f,1.0f,0.01f,-2.0f,12.0f,0.9f,0.9f},

    {15.0f, 0},
    {5.0f, 0},
    {240.0f, 0},
    {6500.0f/3.281f, 0},
    {85, 0},

    1,
    0

};


#endif //MM_sf50_H_
