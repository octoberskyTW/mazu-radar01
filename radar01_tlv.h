#ifndef __RADAR01_TLV_H__
#define __RADAR01_TLV_H__


#include "radar01_share_msg.h"
#include "ringbuffer.h"
#include "vender/dpif_pointcloud.h"
#include "vender/mmw_mss.h"
#include "vender/mmw_output.h"
#include "vender/vital_signs.h"
#define MAXIMUM_OBJS 32
#define MAXIMUM_RANGEBINS 128
struct radar01_pointcloud_data_t {
    uint32_t frameNumber;
    uint32_t numDetectedObj;
    /* MMWDEMO_OUTPUT_MSG_DETECTED_POINTS = 1 */
    // sizeof(DPIF_PointCloudCartesian) * result->numObjOut
    DPIF_PointCloudCartesian points[32];

    /* MMWDEMO_OUTPUT_MSG_RANGE_PROFILE = 2 */
    // sizeof(uint16_t) * subFrameCfg->numRangeBins;
    uint16_t *range_prof;

    /* MMWDEMO_OUTPUT_MSG_NOISE_PROFILE = 3*/
    // sizeof(uint16_t) * subFrameCfg->numRangeBins;
    uint16_t *noise_prof;

    /* MMWDEMO_OUTPUT_MSG_AZIMUT_STATIC_HEAT_MAP = 4*/
    // result->azimuthStaticHeatMapSize * sizeof(cmplx16ImRe_t)
    uint16_t *azimut_static_heat_map;

    /* MMWDEMO_OUTPUT_MSG_RANGE_DOPPLER_HEAT_MAP = 5*/
    // subFrameCfg->numRangeBins * subFrameCfg->numDopplerBins *
    // sizeof(uint16_t)
    uint16_t *range_doppler_heat_map;

    /* MMWDEMO_OUTPUT_MSG_STATS = 6*/
    MmwDemo_output_message_stats msg_stats;

    /* MMWDEMO_OUTPUT_MSG_DETECTED_POINTS_SIDE_INFO = 7 */
    // sizeof(DPIF_PointCloudSideInfo) * result->numObjOut
    DPIF_PointCloudSideInfo points_side_info[32];

    /* MMWDEMO_OUTPUT_MSG_AZIMUT_ELEVATION_STATIC_HEAT_MAP = 8 */
    // Not support yet
    /* MMWDEMO_OUTPUT_MSG_TEMPERATURE_STATS = 9*/
    MmwDemo_temperatureStats temp_stats;
};

struct radar01_vitalsign_data_t {
    uint32_t frameNumber;
    uint32_t numDetectedObj;
    /* VITALSIGN_OUTPUT_MSG_STATS = 0 */
    VitalSignsDemo_OutputStats stats;
    /* VITALSIGN_OUTPUT_MSG_RANGE_PROFILE = 1 */
    // Number of range bins that can be processed
    // obj->numRangeBinProcessed = obj->rangeBinEndIndex -
    // obj->rangeBinStartIndex + 1;
    uint16_t numRangeBinProcessed;
    uint16_t ptrMatrix[MAXIMUM_RANGEBINS * 2];
};

int process_pointcloud_msg(uint8_t *rx_buff, int pkt_length, void *out);
int process_vitalsign_msg(uint8_t *rx_buff, int pkt_length, void *out);
void pointcloud_Cartesian_info_dump(void *datain);
void vitalsign_stats_dump(void *datain);
void pointcloud_create_json_msg(void *datain,
                                struct radar01_json_entry_t *share,
                                size_t sz_limit);
void vitalsign_create_json_msg(void *datain,
                               struct radar01_json_entry_t *share,
                               size_t sz_limit);
int dss_ring_enqueue(struct ringbuffer_t *rbuf, void *payload, uint32_t size);
#endif  //  __RADAR01_TLV_H__