/*****************************************************************************
*   Include Files
*****************************************************************************/
#include "uds_request.h"
#include "uds_server.h"
#include "uds_diag.h"
#include "uds_node_miss.h"


#define ALLGROUPOFDTC                     0xFFFFFF


/*****************************************************************************
*   Function   :    UDS_SRV_ClearDTC
*   Description:
*   Inputs     :    None
*   Outputs    :    NULL
*   Notes      :
*****************************************************************************/
void UDS_SRV_ClearDTC(UDS_T *tUDS, uint8_t *p_u8PDU_Data, uint16_t u16PDU_DLC)
{
    uint8_t  Ar_u8RePDU_DATA[1];
    uint32_t u32GroupOfDTC ;
    int node_num = 0;

    if (u16PDU_DLC == 4)
    {
        u32GroupOfDTC = (uint32_t)p_u8PDU_Data[1] | (((uint32_t)p_u8PDU_Data[2]) << 8) | (((
                            uint32_t)p_u8PDU_Data[3]) << 16);

        if (ALLGROUPOFDTC == u32GroupOfDTC)
        {
            uds_negative_response(tUDS, p_u8PDU_Data[0], NRC_RequestCorrectlyReceivedResponsePending);
            uds_diag_dtc_clear();
            
            for (node_num = CAN_NODE_MISS_ITEM_ACU; node_num < CAN_NODE_MISS_ITEM_NUM; node_num++)
            {
                set_can_node_miss_state(node_num, CAN_NODE_MISS_FLAG_NORMAL);
                set_node_sig_time(node_num, tm_get_time());
            }

            
            Ar_u8RePDU_DATA[0] = p_u8PDU_Data[0] + POS_RESPOND_SID_MASK;
            uds_positive_response(tUDS, tUDS->can_id_res, 1, Ar_u8RePDU_DATA);
        }
        else
        {
            uds_negative_response(tUDS, p_u8PDU_Data[0], NRC_RequestOutOfRange);
        }
    }
    else
    {
        uds_negative_response(tUDS, p_u8PDU_Data[0], NRC_IncorrectMessageLengthOrInvailFormat);
    }
}

