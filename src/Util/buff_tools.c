//
// Created by Maciek Malik
//

#include "buff_tools.h"
#include <stdint.h>
#include <string.h>

/**
 * @brief Appends two buffers and returns new bigger one
 * @param a
 * @param b
 * @param sizea
 * @param sizeb
 * @return
 */
uint8_t * buff_append(uint8_t *a, uint8_t *b, uint16_t sizea, uint16_t sizeb) {
    // out  = a + b;
    uint8_t * tmpOut = (uint8_t*) calloc(sizea+sizeb,sizeof(char));
    memcpy(tmpOut,a,sizea);//add a
    memcpy(tmpOut+sizea,b,sizeb);//Add b at offset (sizea)
    return tmpOut;
}

/**
 * @brief Convert float to byte array
 * @param bytes_temp
 * @param float_variable
 * @see https://stackoverflow.com/questions/24420246/c-function-to-convert-float-to-byte-array
 */
void float2Bytes(unsigned char bytes_temp[4],float float_variable){
    if(bytes_temp == NULL) return;
    memcpy(bytes_temp, (unsigned char*) (&float_variable), 4);
}

/**
 * Convert integer type to byte array
 * @param bytes_temp
 * @param int_val
 */
void int2bytes(unsigned char bytes_temp[4],uint32_t int_val){
    if(bytes_temp == NULL) return;
    memcpy(bytes_temp,(unsigned char*) (&int_val),4);
}