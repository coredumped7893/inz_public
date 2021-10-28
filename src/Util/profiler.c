//
// Created by Maciek Malik
//

#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include "profiler.h"
#include <pthread.h>
#include <string.h>

void *th_log_process_data(void *a);
pthread_t profiler;
FILE* profiler_dump_fd = NULL;
uint8_t run_thread = 1;

void start_profiler(enum TYPE_ENUM type) {

    char f_name[1024] = "profiler_dump";
    strcat(f_name, TYPE_STRING[type]);

    profiler_dump_fd = fopen(strcat(f_name, ".csv"), "w");
    if(profiler_dump_fd == NULL){
        printf("Could not open file for data dump");
        exit(-1);
    }

    //Start thread
    pthread_create(&profiler, NULL, &th_log_process_data, NULL);
    pthread_setname_np(profiler,"profiler");
}



void *th_log_process_data(void *a){
    init_proc_sources();
    while(run_thread){
        int mem_data[10];
        get_memory_data(mem_data);
        fprintf(
                profiler_dump_fd,
        "%d,%d,%d,%d,%d,%d,%d\n",
                mem_data[SIZE],
                mem_data[RESIDENT],
                mem_data[SHARED],
                mem_data[TEXT],
                mem_data[LIB],
                mem_data[DATRA],
                mem_data[DT]
        );
        fflush(profiler_dump_fd);
        usleep(250000);//~4Hz
    }
    return NULL;
}