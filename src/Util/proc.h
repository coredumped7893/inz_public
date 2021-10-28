//
// Created by Maciek Malik
//

#ifndef MM_proc_H_
#define MM_proc_H_

#include <stdio.h>


#define PAGE_SIZE 4


enum STATM_INDEXES {
    SIZE = 0,
    RESIDENT = 1,
    SHARED = 2,
    TEXT = 3,
    LIB = 4,
    DATRA = 5,
    DT = 6
};
void init_proc_sources();
void get_memory_data(int* dataDest);





#endif //MM_proc_H_
