//
// Created by Maciek Malik
//

#ifndef MM_app_H_
#define MM_app_H_
#include "main.h"
#include "../Util/message.h"
#include "../config.h"
#include "../Control/control.h"
#include <pthread.h>
#include "collisionModule.h"
#include "../Util/profiler.h"
//Aircraft configs -------------
#include "../Aircraft/aircraft.h"
//------------------------------

int startAll();

void clean_up();
void run();
status_frame* get_status(void*, int delayed);
double get_target_alt();
double get_target_hdg();
double get_target_ias();
aircraftConfig_t* get_aircraft_config();

#endif //MM_app_H_
