//
// Created by Maciek Malik
//

#ifndef MM_buff_tools_H_
#define MM_buff_tools_H_

#include "stdint.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t * buff_append(uint8_t *a, uint8_t *b, uint16_t sizea, uint16_t sizeb);
void float2Bytes(unsigned char bytes_temp[4],float float_variable);
void int2bytes(unsigned char bytes_temp[4],uint32_t int_val);

#endif //MM_buff_tools_H_
