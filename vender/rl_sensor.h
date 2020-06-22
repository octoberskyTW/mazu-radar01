#ifndef __RL_SENSOR_H__
#define __RL_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "rl_datatypes.h"
/*! \brief
* RF characterization Time and Temperature data structure
*/
typedef struct rlRfTempData
{
    /**
     * @brief  radarSS local Time from device powerup. 1 LSB = 1 ms
     */
    rlUInt32_t time;
    /**
     * @brief  RX0 temperature sensor reading (signed value). 1 LSB = 1 deg C
     */
    rlInt16_t tmpRx0Sens;
    /**
     * @brief  RX1 temperature sensor reading (signed value). 1 LSB = 1 deg C
     */
    rlInt16_t tmpRx1Sens;
    /**
     * @brief  RX2 temperature sensor reading (signed value). 1 LSB = 1 deg C
     */
    rlInt16_t tmpRx2Sens;
    /**
     * @brief  RX3 temperature sensor reading (signed value). 1 LSB = 1 deg C
     */
    rlInt16_t tmpRx3Sens;
    /**
     * @brief  TX0 temperature sensor reading (signed value). 1 LSB = 1 deg C
     */
    rlInt16_t tmpTx0Sens;
    /**
     * @brief  TX1 temperature sensor reading (signed value). 1 LSB = 1 deg C
     */
    rlInt16_t tmpTx1Sens;
    /**
     * @brief  TX2 temperature sensor reading (signed value). 1 LSB = 1 deg C
     */
    rlInt16_t tmpTx2Sens;
    /**
     * @brief  PM temperature sensor reading (signed value). 1 LSB = 1 deg C
     */
    rlInt16_t tmpPmSens;
    /**
     * @brief  Digital temp sensor reading (signed value). 1 LSB = 1 deg C
     */
    rlInt16_t tmpDig0Sens;
    /**
     * @brief  Second digital temp sensor reading (signed value).( applicable only in \n
     *         xWR1642/IWR6843/xWR1843.) \n
     *         1 LSB = 1 deg C \n
     */
    rlInt16_t tmpDig1Sens;
}rlRfTempData_t;


#ifdef __cplusplus
}
#endif


#endif //__RL_SENSOR_H__