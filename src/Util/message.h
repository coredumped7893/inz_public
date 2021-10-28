//
// Created by Maciek Malik
//

#ifndef MM_Error_H_
#define MM_Error_H_

#include "stdint.h"
#include "../config.h"

#ifdef __cplusplus
extern "C" {
#endif


//Static map of errors
extern const char ERR_LIST[11][50];

extern const char MSG_LIST[6][50];



void throw_err(uint8_t id, uint8_t exitOnErr);
void print_message_local(uint8_t id);

#ifdef __cplusplus
}
#endif

#endif //MM_Error_H_
