//
// Created by Maciek Malik
//

#include <stdlib.h>
#include <string.h>
#include "proc.h"

FILE* proc_statm_fd = NULL;

/**
 * @brief Splits contents of the /proc/self/statm file and converts them to number format
 * @param dataDest
 */
void get_memory_data(int *dataDest) {

    char data_line[512] = {0};
    rewind(proc_statm_fd);
    fgets(data_line, sizeof(data_line),proc_statm_fd);

    char * partial_val = strtok(data_line, " ");
    if(partial_val != NULL) dataDest[0] = atoi(partial_val);

    int idx = 1;
    while(partial_val != NULL ) {
        partial_val = strtok(NULL, " ");
        if(idx < 7 && partial_val != NULL) dataDest[idx] = atoi(partial_val);
        ++idx;
    }

#if PROFILER_DATA_DEBUG
    for (int i = 0; i < 7; ++i) {
        printf("%d ", dataDest[i]);
    }
    printf("\n");
#endif

}

void init_proc_sources() {
    proc_statm_fd = fopen("/proc/self/statm", "r");
    if(proc_statm_fd == NULL){
        printf("Could not open file");
        exit(-1);
    }
}
