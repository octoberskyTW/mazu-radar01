#include "radar01_tlv.h"
#include "linux_common.h"
#include "radar01_utils.h"
#include "vender/mmw_output.h"

int radar01_process_message(uint8_t *rx_buff,
                            int pkt_length,
                            struct radar01_message_data_t *out_data)
{
    /* Start to process TLV IEs */
    uint8_t *pData = rx_buff;
    uint32_t tlv = 1;
    MmwDemo_output_message_header msgh = {0};
    memset((uint8_t *) out_data, 0, sizeof(struct radar01_message_data_t));
    memcpy(&msgh, rx_buff, sizeof(MmwDemo_output_message_header));
    // if(RADAR01_DEBUG_ENABLE)
    printf("Frame Num %u: DetectedObjs %u, numTLVs %u \n", msgh.frameNumber,
           msgh.numDetectedObj, msgh.numTLVs);
    out_data->frameNumber = msgh.frameNumber;
    out_data->numDetectedObj = msgh.numDetectedObj;
    pData += sizeof(MmwDemo_output_message_header);
    pkt_length -= sizeof(MmwDemo_output_message_header);
    while (tlv < msgh.numTLVs && pkt_length > 0) {
        MmwDemo_output_message_tl tlv_recv;
        memcpy((uint8_t *) &tlv_recv, pData, sizeof(MmwDemo_output_message_tl));
        pData += sizeof(MmwDemo_output_message_tl);
        pkt_length -= sizeof(MmwDemo_output_message_tl);
        switch (tlv_recv.type) {
        case MMWDEMO_OUTPUT_MSG_DETECTED_POINTS:
            for (uint32_t i = 0; i < msgh.numDetectedObj; i++) {
                memcpy((void *) &out_data->points[i],
                       pData + i * sizeof(DPIF_PointCloudCartesian),
                       sizeof(DPIF_PointCloudCartesian));
            }
            break;
        case MMWDEMO_OUTPUT_MSG_DETECTED_POINTS_SIDE_INFO:
            for (uint32_t i = 0; i < msgh.numDetectedObj; i++) {
                memcpy((void *) &out_data->points_side_info[i],
                       pData + i * sizeof(DPIF_PointCloudSideInfo),
                       sizeof(DPIF_PointCloudSideInfo));
            }
            break;
        default:
            break;
        }
        pData += tlv_recv.length;
        pkt_length -= tlv_recv.length;
        tlv++;
    }
    return 0;
}

void radar01_Cartesian_info_dump(struct radar01_message_data_t *data)
{
    DPIF_PointCloudSideInfo *side_info = &data->points_side_info[0];
    DPIF_PointCloudCartesian *points = &data->points[0];

    for (uint32_t i = 0; i < data->numDetectedObj; i++) {
        printf("%u, obj_%u, %f, %f, %f, %f, %d, %d\n", data->frameNumber, i,
               points[i].x, points[i].y, points[i].z, points[i].velocity,
               side_info[i].snr, side_info[i].noise);
    }
}