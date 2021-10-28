//
// Created by Maciek Malik
//

#include <stddef.h>
#include "time_diff.h"
#include "pthread.h"
#include "message.h"

struct timeval t_start, t_end;
static pthread_mutex_t timer_lock;
static int init_done = 0;


/**
 * @brief Initializes mutex used in timer
 */
void init_timer(){
    if(!init_done){
        if (pthread_mutex_init(&timer_lock, NULL) != 0) {
            throw_err(4, 0);
            return;
        }
        init_done = 1;
    }
}


/**
 * @brief Saves the time as a starting point
 * @remark Only one timer can be start at a time, otherwise lock can happen - no nested timers
 * @param t
 * @param tz
 */
void start_timer() {
    pthread_mutex_lock(&timer_lock);
    gettimeofday(&t_start,NULL);
    //pthread_mutex_unlock(&timer_lock);
}

/**
 * @brief Gets second timestamp and calculates difference in microseconds
 * @param t
 * @param tz
 * @return elapsed time in microseconds
 */
long end_timer() {
    //pthread_mutex_lock(&timer_lock);
    gettimeofday(&t_end,NULL);
    long elapsed_time = ((t_end.tv_sec - t_start.tv_sec) * 1000000) + (t_end.tv_usec - t_start.tv_usec);
    pthread_mutex_unlock(&timer_lock);
    return elapsed_time;
}
