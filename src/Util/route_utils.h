//
// Created by Maciek Malik
//

#ifndef MM_route_utils_H_
#define MM_route_utils_H_

#include "../FlightManager/route.h"

position_t point_from_dist_brg(double latPos, double lngPos, double distance, double bearing);
double deg2Rad(double deg);
double rad2Deg(double reg);
float degWrap360(double deg);

#endif //MM_route_utils_H_
