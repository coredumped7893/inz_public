//
// Created by Maciek Malik
//

#ifndef MM_prk2_H_
#define MM_prk2_H_


/**
 * __attribute__((used)) -  avoid_removal_by_linker_unused_section_removal
 */
__attribute__((used)) static aircraftConfig_t prk2 = {
    "PRK-2 Solar UAV",

    {0.35f,0.14f,0.002f,0.01f,-1.0f,1.0f,0.001f,0.004f},
    {0.2433f,0.533f,0.00f,0.01f,-1.0f,0.8f,0.001f,0.001f},
    {0.11f,0.28f,0.0f,0.01f,0.02f,1.0f,0.001f,0.001f},

    {0.405f,0.22f,0.053f,0.07f,-15.0f,15.0f,0.03f,0.03f},
    {0.3f,0.03f,1.0f,0.01f,-8.0f,10.0f,0.9f,0.9f},

    {00.0f, 0},
    {1.0f, 0},
    {118.0f, 0},
    {660.0f/3.281f, 0},
    {35, 0},

    2,
    1,
    140,
    20,
    90,
    18000,
    2.5,
    720,
    790,
    840
};
//{0.75f,0.5f,0.2f,0.01f,-1.0f,1.0f,0.009f,0.009f},
//{0.0433f,0.533f,0.00f,0.01f,-0.7f,0.7f,0.001f,0.001f},
//{0.01f,0.18f,0.0f,0.01f,0.02f,1.0f,0.001f,0.001f},
//
//{0.61f,0.095f,2.0f,0.01f,-30.0f,30.0f,0.9f,0.9f},
//{0.3f,0.03f,1.0f,0.01f,-2.0f,10.0f,0.9f,0.9f},

#endif //MM_prk2_H_
