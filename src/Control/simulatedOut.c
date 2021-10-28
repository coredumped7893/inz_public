//
// Created by Maciek Malik
//

#include "simulatedOut.h"

/**
 * @brief Setting throttle inside SIM
 * @see send_DREF()
 * @param value
 */
void sim_set_throttle(float value) {
    send_DREF(ENG_1_THR, value);
    send_DREF(ENG_2_THR, value);
}

/**
 * Sending command to the SIM
 * @param cmndID - index in  the X_COMMANDS array
 * @see x_send_command()
 */
void sim_send_command(const uint8_t cmndID) {
    x_send_command(cmndID);
}

/**
 * @brief Set axis in SIM
 * @see AxisSet
 * @see send_DREF
 * @param axis
 */
void sim_set_axis(AxisSet * axis){
    send_DREF(ROLL_INPUT,axis->roll);//Roll
    send_DREF(PITCH_INPUT,axis->pitch);//Pitch
    send_DREF(YAW_INPUT,axis->yaw);//Yaw
}

/**
 * @brief Send data to SIM
 * @see send_DREF()
 * @see X_DATAREF
 * @param val
 * @param datarefID
 */
void sim_send_data(float val, const uint8_t datarefID) {
    send_DREF(datarefID,val);
}

