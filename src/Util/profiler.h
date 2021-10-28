//
// Created by Maciek Malik
//

#ifndef MM_profiler_H_
#define MM_profiler_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "proc.h"
#include "stdint.h"

#define FOREACH_TYPE(TYPE) \
        TYPE(IMAGEPROCESSING)   \
        TYPE(FLIGHTMANAGER)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum TYPE_ENUM {
    FOREACH_TYPE(GENERATE_ENUM)
};

__attribute__((unused)) static const char *TYPE_STRING[] = {
        FOREACH_TYPE(GENERATE_STRING)
};

void start_profiler(enum TYPE_ENUM type);

#endif //MM_profiler_H_
