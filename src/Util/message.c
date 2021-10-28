//
// Created by Maciek Malik
//

#include "message.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Throw error from ERR_LIST array, program can exit or continue according to the exitOnErr param
 * @param id
 * @param exitOnErr m
 */
TESTING_REQ_WEAK
void throw_err(uint8_t id, uint8_t exitOnErr) {
    printf("\n ->[%s]<- \n", ERR_LIST[id]);
    if(exitOnErr) exit(10 + (int)id);
}

/**
 * Just a printf wrapper that prints message from MSG_LIST array.
 * @param id
 */
TESTING_REQ_WEAK
void print_message_local(uint8_t id) {
    printf(" [%s] \n", MSG_LIST[id]);
}

//-------------------------------------

const char ERR_LIST[11][50] = {
        "Failed to initialize modules",
        "No connection to data input",
        "Couldn't create server",
        "Couldn't allocate memory",
        "Mutex init error",
        "Incorrect PID constants",
        "Failed to initialize maps",
        "Failed to open directory",
        "Failed to open file",
        "Couldn`t connect socket",
        "Failed to open the video stream"
};

const char MSG_LIST[6][50] = {
        "Init OK",
        "Socket up and running",
        "--- img prc started ---",
        "Connected to RTMP Server on: ",
        "-- Maps Loaded --",
        "waiting for video..."

};