//
// Created by Maciek Malik
//

#include "route.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "../Util/route_utils.h"

static queue_list_t _route;
double bezierCurve(float t, const float *ws, const float *points);
uint8_t route_segment = 0;
queue_node first_point = {0};


/**
 * Route: EPBC -> most g-roweckiego -> most gdanski -> arkadia  -> ratusz ars
 */
__attribute__ ((unused)) static route_node_t SAMPLE_ROUTE_1[]={//Normal route, no obstacles
    {.lat = 52.269f,.lng = 20.908f,.alt = 800},
    {.lat = 52.284f,.lng = 20.989f,.alt = 700},
    {.lat = 52.26f,.lng = 21.007f,.alt = 900},
    {.lat = 52.255f, .lng = 20.982f, .alt = 900},
    {.lat = 52.244f, .lng = 21.001f,.alt = 700}
};

/**
 * Building collision expected
 */
__attribute__ ((unused)) static route_node_t SAMPLE_ROUTE_2[]={
        {.lat = 52.24389913818221f,.lng = 20.99970078468324f,.alt = 580},
        {.lat = 52.255269f,.lng = 20.999232f,.alt = 580},
        {.lat = 52.224f,.lng = 21.029f,.alt = 580},
};

/**
 * Building collision expected
 */
__attribute__ ((unused)) static route_node_t SAMPLE_ROUTE_3[3]={
        {.lat = 52.2501346252929f,.lng = 20.9926440925598f,.alt = 500},
        {.lat = 52.2549671617953f,.lng = 21.000026822090152f,.alt = 530},
        {.lat = 52.25546146251476f,.lng = 21.012136894226078f,.alt = 510},
};

/**
 * @brief Init route - load points and generate bezier curve across them
 * @return
 */
uint8_t route_init(){

    _route = q_init();

    route_load(SAMPLE_ROUTE_1, sizeof(SAMPLE_ROUTE_1)/sizeof(route_node_t));

    if(q_num_elements(_route) > 0){
        first_point = *q_first_node(_route);
    }

    printf("\nRoute set with %i legs\n",(int)q_num_elements(_route));
    return 1;
}

/**
 * @brief Return difference in degrees from the north (0 deg.)
 * @param deg
 * @return
 */
int deg_from_north(int deg){
    if(deg <= 180){
        return deg;
    }else{
        return (360-deg);
    }
}

/**
 * @brief Load route from the predefined points, and generate bezier curve basen on them
 * @param route
 * @param size
 */
void route_load(route_node_t *route, int size) {

    route_node_t *routeDyn = calloc(size,sizeof(route_node_t));
    memcpy(routeDyn,route,sizeof(route_node_t)*size);//Make sure that we are using dynamic memory, queue lib require dyn. mem.

    printf("Generating Bezier for route...\n");

    uint8_t firstPoint = 1;
    float oldLat=0,oldLng=0;
    int i;
    for (i = 0; ((i + 2) < size) && (i%2 == 0) ; i+=2) {

        /*
         * calculate dynamic bezier for the next 3 WP (if number is correct)
         * push it after main WP
         * flag as generated
         */
            int step = 2;//Calculate step dynamically based on route length
            for(int t = 0; t <= 100; t+=step){
                route_node_t* newTmpNode = (route_node_t*) calloc(1,sizeof(route_node_t));
                const float w[3] = {1,9.1f,1};
                const float pLat[3] = {route[i + 0].lat, route[i + 1].lat, route[i + 2].lat};
                const float pLng[3] = {route[i + 0].lng, route[i + 1].lng, route[i + 2].lng};
                newTmpNode->lat = (float) bezierCurve(((float)t)/100.0f, (const float *) &w, (const float *) &pLat);
                newTmpNode->lng =(float) bezierCurve(((float)t)/100.0f, (const float *) &w, (const float *) &pLng);

                if(firstPoint){
                    newTmpNode->hdg  = -1;//Disabled, calculate dynamic heading
                    firstPoint = 0;
                    newTmpNode->generated = 0;
                }else{
                    //heading to the next point
                    newTmpNode->hdg = get_target_heading(oldLat,oldLng,newTmpNode->lat,newTmpNode->lng);
                    newTmpNode->generated = 1;
                }

                newTmpNode->time = (i*1000) + t;

                //curve between WP1 and WP2
                if(t <= 25){
                    newTmpNode->alt = route[i + 1].alt;
                }else{
                    newTmpNode->alt = route[i + 2].alt;
                }

                oldLat = newTmpNode->lat;//Save previous values
                oldLng = newTmpNode->lng;


                q_push(_route,newTmpNode);
            }

    }

#if ROUTE_DUMP
    printf("-------- Route dump --------\n");
    for (queue_node* node = q_first_node(_route); node != NULL; node = node->next){
        route_node_t* tmp = (route_node_t*) node->data;
        printf("%f %f %f \n",tmp->lat,tmp->lng,tmp->hdg);
    }
    printf("-------- Route dump --------\n");
#endif


}


/**
 * @brief Get current target node
 */
queue_node * route_get_target() {
    return (queue_node*) q_first_node(_route);
}

/**
 * @brief Gets next route points
 * @deprecated
 */
void route_next_point(){
    q_pop(_route);
}

/**
 * @brief Calculates geographic course based on two points
 * @see https://www.movable-type.co.uk/scripts/latlong.html
 * @param lat
 * @param lng
 * @param targetLAT
 * @param targetLNG
 * @return
 */
float get_target_heading(float lat, float lng, float targetLAT, float targetLNG) {

    double phi1 =  deg2Rad(lat);
    double phi2 =  deg2Rad(targetLAT);

    double dLam = deg2Rad((targetLNG - lng));

    double tmpX = cos(phi2) * sin(dLam);
    double tmpY = (sin(phi2) * cos(phi1)) - sin(phi1)*cos(phi2)*cos(dLam);
    double a = rad2Deg(atan2(tmpX,tmpY));
    /*
     * atan2 return values from -pi to +pi -> this gives range of -180 to 180 degrees
     * + 360 and then modulo is required to get typical compass range (0 to 360 deg)
     * @see degWrap360()
     */
    return degWrap360(a);
    /*

    }
     *
     *
     */
}

/**
 * @brief Calculate cross tract error from current position and planned route
 * @see https://www.movable-type.co.uk/scripts/latlong.html
 * @param latPos deg.
 * @param lngPos deg.
 * @param latStart deg.
 * @param lngStart deg.
 * @param latEnd deg.
 * @param lngEnd deg.
 * @return
 */
float crossTrackErr(float latPos, float lngPos,float latStart, float lngStart, float latEnd, float lngEnd) {

    //Distance between two equal points
    if(latStart == latEnd && lngStart == lngEnd){
        return pointDistance(latPos,lngPos,latStart,lngStart);
    }

    int R = E_RADIUS;

    double disFromStart = pointDistance(latStart,lngStart,latPos,lngPos) / (double)R;
    double hdgToMe = deg2Rad(get_target_heading(latStart,lngStart,latPos,lngPos));
    double hdgPath = deg2Rad(get_target_heading(latStart,lngStart,latEnd,lngEnd));

    double tmpOut = asin(sin(disFromStart) * sin((hdgToMe-hdgPath)));

    return (float) (tmpOut * R);

}

/**
 * @brief Calculates distance between two points in meters.
 * Approximates Earth to sphere
 * @see https://www.movable-type.co.uk/scripts/latlong.html
 * @param[in] latStart
 * @param[in] lngStart
 * @param[in] latEnd
 * @param[in] lngEnd
 * @return distance between two points in meters
 */
float pointDistance(float latStart, float lngStart, float latEnd, float lngEnd) {

    //Distance between two equal points
    if(latStart == latEnd && lngStart == lngEnd){
        return 0;
    }

    int R = E_RADIUS;

    double phi1 =  deg2Rad(latStart);
    double phi2 =  deg2Rad(latEnd);
    double lam1 =  deg2Rad(lngStart);
    double lam2 =  deg2Rad(lngEnd);

    double dPhi = phi2 - phi1;
    double dLam = lam2 - lam1;

    double a = pow(sin(dPhi/2.0),2) + cos(phi1)*cos(phi2) * pow(sin(dLam/2.0),2);
    double c =  2 * atan2(sqrt(a),sqrt(1-a));

    return (float) (R * c);

}

/**
 * @brief Finds the closest point on the current route
 * @param[in] lat current latitude
 * @param[in] lng current longitude
 * @return pointer to route node
 */
queue_node* get_closest_point(float lat, float lng) {
    queue_node* tmp;
    queue_node* first_node = q_first_node(_route);
    route_node_t* first_node_route_point = (route_node_t*) first_node->data;
    //Calc for first point
    float minDist = FLT_MAX;
    //@TODO check only single route sector - sectors need to be updated separately
    //Iterate over over every element
    q_traverse(_route, tmp){
        route_node_t* tmpNode = (route_node_t*) tmp->data;
        float d = pointDistance(lat,lng,tmpNode->lat,tmpNode->lng);
        if(d < minDist){
            //Save new min value
            minDist = d;
            first_node = tmp;
            first_node_route_point = (route_node_t*) first_node->data;
        }
    }
    first_node_route_point->maxXTE = minDist;//Temp. save min distance to 'maxXTE' field
    return first_node;
}

/**
 * @brief Returns first point from the loaded route
 * @return
 */
queue_node get_first_point() {
    if(first_point.data == 0){
        printf("First point is empty!\n");
    }
    return first_point;
}

/**
 * @brief Gets pointer to the route points structure
 * @return
 */
inline queue_list_t* get_route_ptr() {
    return &_route;
}

/**
 * @deprecated
 * @return
 */
uint8_t is_at_route_start() {
   // return (pow(FIRST_POINT_EPSILON,2) >= (pow((tmpLAT - current_route_node_point->lat), 2) + pow((tmpLNG - current_route_node_point->lng), 2)));
   return 0;
}

/**
 * @brief calculate wind correction angle in degrees
 * @param tas_speed [m/s]
 * @param wind
 * @return
 */
float wca_calculate(double tas_speed, wind_layer_data_t wind) {
    return (float) rad2Deg(asin((wind.speed/tas_speed) * sin(deg2Rad(wind.direction)) ));
}

/**
 * @brief Generates Bezier ("rational Bezier") curve for given 3 points with coefficients
 * @param t - range [0,3]
 * @param ws - points coeffs
 * @param points
 * @return
 */
double bezierCurve(float t, const float *ws, const float *points) {
    double tmp = (ws[0]*points[0]*pow((1.0f-t),2) +
                 2*(1.0f-t)*t*ws[1]*points[1] +
                 pow(t,2)*ws[2]*points[2]) /
                (ws[0]*pow((1.0f-t),2) +
                 2*(1.0f-t)*t*ws[1] +
                 pow(t,2)*ws[2]);
    return tmp;
}



