#include "radar01_tlv.h"
#include <json-c/json_object.h>
#include "linux_common.h"
#include "radar01_utils.h"
#include "vender/mmw_output.h"

int process_pointcloud_msg(uint8_t *rx_buff, int pkt_length, void *out)
{
    /* Start to process TLV IEs */
    uint8_t *pData = rx_buff;
    uint32_t tlv = 1;
    struct radar01_pointcloud_data_t *out_data =
        (struct radar01_pointcloud_data_t *) out;
    MmwDemo_output_message_header msgh = {0};
    memset((uint8_t *) out_data, 0, sizeof(struct radar01_pointcloud_data_t));
    memcpy(&msgh, rx_buff, sizeof(MmwDemo_output_message_header));
    printf("Frame %u: Stamp %u: Detected %u objs: TLVs=%u\n", msgh.frameNumber,
           msgh.timeCpuCycles, msgh.numDetectedObj, msgh.numTLVs);
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

void pointcloud_Cartesian_info_dump(void *datain)
{
    struct radar01_pointcloud_data_t *data =
        (struct radar01_pointcloud_data_t *) datain;
    if (RADAR01_CSV_DEBUG_ENABLE == 0)
        return;
    DPIF_PointCloudSideInfo *side_info = &data->points_side_info[0];
    DPIF_PointCloudCartesian *points = &data->points[0];

    for (uint32_t i = 0; i < data->numDetectedObj; i++) {
        printf("%u, obj_%u, %f, %f, %f, %f, %d, %d\n", data->frameNumber, i,
               points[i].x, points[i].y, points[i].z, points[i].velocity,
               side_info[i].snr, side_info[i].noise);
    }
}

void pointcloud_create_json_msg(void *datain,
                                struct radar01_json_entry_t *share,
                                size_t sz_limit)
{
    struct radar01_pointcloud_data_t *pdata =
        (struct radar01_pointcloud_data_t *) datain;
    DPIF_PointCloudSideInfo *side_info = &pdata->points_side_info[0];
    DPIF_PointCloudCartesian *points = &pdata->points[0];

    json_object *jarr;
    json_object *jobj[32] = {NULL};
    jarr = json_object_new_array();

    for (uint32_t i = 0; i < pdata->numDetectedObj; i++) {
        char val_str[128] = {0};
        jobj[i] = json_object_new_object();
        json_object_object_add(jobj[i], "frm_seq",
                               json_object_new_int(pdata->frameNumber));
        json_object_object_add(jobj[i], "obj_idx", json_object_new_int(i));

        snprintf(val_str, 128, "%8.6f", points[i].x);
        json_object_object_add(jobj[i], "x", json_object_new_string(val_str));
        snprintf(val_str, 128, "%8.6f", points[i].y);
        json_object_object_add(jobj[i], "y", json_object_new_string(val_str));
        snprintf(val_str, 128, "%8.6f", points[i].z);
        json_object_object_add(jobj[i], "z", json_object_new_string(val_str));
        // /* velocity */
        snprintf(val_str, 128, "%8.6f", points[i].velocity);
        json_object_object_add(jobj[i], "velocity",
                               json_object_new_string(val_str));

        json_object_object_add(jobj[i], "snr",
                               json_object_new_int(side_info[i].snr));
        json_object_object_add(jobj[i], "noise",
                               json_object_new_int(side_info[i].noise));
        json_object_array_add(jarr, jobj[i]);
    }

    snprintf(share->payload, sz_limit, "data=%s",
             json_object_to_json_string_ext(jarr, JSON_C_TO_STRING_NOZERO));
    share->length = strlen(share->payload);
    if (share->length > (int) sz_limit)
        printf("[WARNING] JSON Size overflow %d, expect: %d\n", share->length,
               (int) sz_limit);
    if (RADAR01_JSON_MSG_DEBUG_ENABLE == 1)
        printf("JSON Raw data Length:%d, data=%s\n", share->length,
               json_object_to_json_string_ext(jarr, JSON_C_TO_STRING_NOZERO));
    for (uint32_t i = 0; i < pdata->numDetectedObj; i++)
        json_object_put(jobj[i]);
    json_object_put(jarr);
}

int dss_ring_enqueue(struct ringbuffer_t *rbuf, void *payload, uint32_t size)
{
    uint8_t *txcell = (uint8_t *) radar01_alloc_mem(size);
    if (txcell == NULL) {
        fprintf(stderr, "[%s:%d] txcell allocate fail!!\n", __func__, __LINE__);
        goto empty;
    }
    memcpy(txcell, payload, size);
    rb_push(rbuf, txcell);
empty:
    return 0;
}