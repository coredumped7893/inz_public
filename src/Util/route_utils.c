//
// Created by Maciek Malik
//

#include "route_utils.h"



/**
 * @brief Get point(position_t) from bearing and distance
 * @param latPos
 * @param lngPos
 * @param distance in meters
 * @param bearing
 * @return position_t - new point in degrees
 */
position_t point_from_dist_brg(double latPos, double lngPos, double distance, double bearing){
    position_t out;

    latPos = deg2Rad(latPos);
    lngPos = deg2Rad(lngPos);
    bearing = deg2Rad(bearing);

    double ang_dist = (double)distance/(double)E_RADIUS;//angular distance, Earth radius in meters

    double tmpLat = (asin(
            (sin(latPos) * cos(ang_dist)) +
            (cos(latPos) * sin(ang_dist) * cos(bearing))
    ));
    out.lat = (float) rad2Deg(tmpLat);

    double tmpLNG = (lngPos + atan2(
            sin(bearing) * sin(ang_dist) * cos(latPos),
            cos(ang_dist) - sin(latPos) * sin(tmpLat)));
    out.lng = (float) rad2Deg(tmpLNG);

    return out;
}


/**
 * @brief Converts degrees to radians
 * @param deg
 * @return radians
 */
double deg2Rad(double deg) {
    return deg * (M_PI / 180);
}

/**
 * @brief Converts radians to degrees
 * @param reg
 * @return degress
 */
double rad2Deg(double reg) {
    return reg * (180 / M_PI);
}

/**
 * @brief Wraps input parameter to [0,359] range
 * @see https://www.movable-type.co.uk/scripts/latlong.html
 * @param deg
 * @return value in range of 0 to 359 deg
 */
float degWrap360(double deg) {
    //source https://www.movable-type.co.uk/scripts/latlong.html
    if (0<=deg && deg<360) return (float) deg; // avoid rounding due to arithmetic ops if within range
    //return (deg % 360+360) % 360; // sawtooth wave p:360, a:360
    return (float) fmod(fmod(deg,360)+360,360);
}