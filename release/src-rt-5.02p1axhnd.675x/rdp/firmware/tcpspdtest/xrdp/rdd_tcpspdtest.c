/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


/*
 * rdd_tcpspdtest.c
 */

#include "rdd_tcpspdtest.h"
#include "rdp_drv_rnr.h"

bdmf_error_t rdd_tcpspdtest_engine_global_info_set(RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_DTS *global_info)
{
    //don't set .num_streams, .stream_vector global variables. it's managed by rdd and fw.
    //RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_NUM_STREAMS_WRITE_G(global_info->num_streams, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);
    //RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_STREAM_VECTOR_WRITE_G(global_info->stream_vector, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);

    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_UP_BUCKET_TOKENS_WRITE_G(global_info->up_bucket_tokens, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);
    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_UP_BUCKET_FULL_TOKENS_WRITE_G(global_info->up_bucket_full_tokens, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);
    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_UP_BUCKET_TOKENS_FILL_RATE_WRITE_G(global_info->up_bucket_tokens_fill_rate, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);
    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_UP_NEXT_STREAM_ID_WRITE_G(global_info->up_next_stream_id, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_tcpspdtest_engine_global_info_get(RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_DTS *global_info)
{
    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_NUM_STREAMS_READ_G(global_info->num_streams, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);
    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_STREAM_VECTOR_READ_G(global_info->stream_vector, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);
    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_UP_BUCKET_TOKENS_READ_G(global_info->up_bucket_tokens, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);
    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_UP_BUCKET_FULL_TOKENS_READ_G(global_info->up_bucket_full_tokens, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);
    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_UP_BUCKET_TOKENS_FILL_RATE_READ_G(global_info->up_bucket_tokens_fill_rate, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);
    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_UP_NEXT_STREAM_ID_READ_G(global_info->up_next_stream_id, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_tcpspdtest_engine_conn_info_set(RDD_TCPSPDTEST_ENGINE_CONN_INFO_DTS *conn_info,
    RDD_PKTGEN_TX_STREAM_ENTRY_DTS *pktgen_tx_stream, uint8_t stream_idx)
{
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_CONN_INFO_CPU_RX_RDD_QUEUE_WRITE_G(conn_info->cpu_rx_rdd_queue, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_CONN_INFO_UP_TX_MSS_WRITE_G(conn_info->up_tx_mss, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_CONN_INFO_UP_TX_MAX_PD_LEN_WRITE_G(conn_info->up_tx_max_pd_len, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_CONN_INFO_UP_PPPOE_HDR_OFS_WRITE_G(conn_info->up_pppoe_hdr_ofs, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_CONN_INFO_UP_PEER_RX_SCALE_WRITE_G(conn_info->up_peer_rx_scale, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_CONN_INFO_SACK_PERMITTED_WRITE_G(conn_info->sack_permitted, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);

    /* Update pktgen_tx stream info */
    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_WAN_FLOW_WRITE_G(pktgen_tx_stream->entry_parms_wan_flow, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_TX_QM_QUEUE_WRITE_G(pktgen_tx_stream->entry_parms_tx_qm_queue, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_L2_HDR_LEN_WRITE_G(pktgen_tx_stream->entry_parms_l2_hdr_len, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_L3_HDR_LEN_WRITE_G(pktgen_tx_stream->entry_parms_l3_hdr_len, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_L3_PROTOCOL_WRITE_G(pktgen_tx_stream->entry_parms_l3_protocol, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_TX_HDR_LEN_WRITE_G(pktgen_tx_stream->entry_parms_tx_hdr_len, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);

    /* Set SPDTEST_TCP as owner for spdtest
       TODO: Upload set it to SPDTEST_TCP on first opened stream and set it to SPDTEST_NONE on last closed stream.
             Download should do the same but currently no need to recognize first opened stream and last closed stream for download.
             Currently it is set for each opened stream despite that spdsvc type is global for all streams.
             Currently no tx stop for download streams so spdsvc type will remain SPDTEST_TCP after download test ends - 
             but this is not a problem since Download flow is removed and pkt type will not be CPU_RX_TYPE_SPEED_SERVICE.
    */
    RDD_SPDTEST_GEN_CFG_TEST_TYPE_WRITE_G(SPDTEST_TCP, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_tcpspdtest_engine_conn_info_get(RDD_TCPSPDTEST_ENGINE_CONN_INFO_DTS *conn_info,
    RDD_PKTGEN_TX_STREAM_ENTRY_DTS *pktgen_tx_stream, uint8_t stream_idx)
{
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_CONN_INFO_CPU_RX_RDD_QUEUE_READ_G(conn_info->cpu_rx_rdd_queue, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_CONN_INFO_UP_TX_MSS_READ_G(conn_info->up_tx_mss, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_CONN_INFO_UP_TX_MAX_PD_LEN_READ_G(conn_info->up_tx_max_pd_len, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_CONN_INFO_UP_PPPOE_HDR_OFS_READ_G(conn_info->up_pppoe_hdr_ofs, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_CONN_INFO_UP_PEER_RX_SCALE_READ_G(conn_info->up_peer_rx_scale, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_CONN_INFO_SACK_PERMITTED_READ_G(conn_info->sack_permitted, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);

    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_WAN_FLOW_READ_G(pktgen_tx_stream->entry_parms_wan_flow, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_TX_QM_QUEUE_READ_G(pktgen_tx_stream->entry_parms_tx_qm_queue, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_L2_HDR_LEN_READ_G(pktgen_tx_stream->entry_parms_l2_hdr_len, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_L3_HDR_LEN_READ_G(pktgen_tx_stream->entry_parms_l3_hdr_len, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_L3_PROTOCOL_READ_G(pktgen_tx_stream->entry_parms_l3_protocol, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_TX_HDR_LEN_READ_G(pktgen_tx_stream->entry_parms_tx_hdr_len, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_tcpspdtest_engine_tcb_set(RDD_TCPSPDTEST_ENGINE_TCB_DTS *tcb, uint8_t stream_idx)
{
    //#ib#todo#: temporary
    if (tcb->expected_bytes == 0xffffffff && tcb->expected_bytes_1 == 0xffffffff)
    {
        RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_EXPECTED_BYTES_1_WRITE_G(0, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
        RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_EXPECTED_BYTES_WRITE_G(0, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
        return BDMF_ERR_OK;
    }

    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_RX_PKTS_WRITE_G(tcb->rx_pkts, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_TXED_PKTS_WRITE_G(tcb->txed_pkts, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_FREED_PKTS_WRITE_G(tcb->freed_pkts, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_ACK_SEQ_WRITE_G(tcb->ack_seq, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_TX_SEQ_WRITE_G(tcb->tx_seq, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_BAD_PKTS_WRITE_G(tcb->bad_pkts, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_NO_CREDITS_WRITE_G(tcb->no_credits, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);

    /* BUG_ARRAY_NOT_ALLOWED_IN_MIDDLE_OF_TABLE - use array when bug is fixed */
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_BAD_BYTES_1_WRITE_G(tcb->bad_bytes_1, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_BAD_BYTES_WRITE_G(tcb->bad_bytes, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_RX_BYTES_1_WRITE_G(tcb->rx_bytes_1, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_RX_BYTES_WRITE_G(tcb->rx_bytes, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_EXPECTED_BYTES_1_WRITE_G(tcb->expected_bytes_1, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_EXPECTED_BYTES_WRITE_G(tcb->expected_bytes, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);

    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_TO_SEND_BYTES_1_WRITE_G(tcb->up_to_send_bytes_1, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_TO_SEND_BYTES_WRITE_G(tcb->up_to_send_bytes, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_CWND_WRITE_G(tcb->up_cwnd, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_CWND_THR_WRITE_G(tcb->up_cwnd_thr, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_CWND_BUDGET_WRITE_G(tcb->up_cwnd_budget, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_LAST_ACK_SEQ_WRITE_G(tcb->up_last_ack_seq, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_LAST_ACK_TIME_WRITE_G(tcb->up_last_ack_time, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_DUP_ACK_CNT_WRITE_G(tcb->up_dup_ack_cnt, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_DUP_ACK_SEQ_DONE_WRITE_G(tcb->up_dup_ack_seq_done, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_DUP_ACK_TOTAL_PKTS_CNT_WRITE_G(tcb->up_dup_ack_total_pkts_cnt, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_FAST_RETRANS_CNT_WRITE_G(tcb->up_fast_retrans_cnt, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_TIMEOUT_CNT_WRITE_G(tcb->up_timeout_cnt, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_RX_PKTS_AFTER_TIMEOUT_CNT_WRITE_G(tcb->up_rx_pkts_after_timeout_cnt, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_DUP_ACK_STATE_WRITE_G(tcb->up_dup_ack_state, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_IS_RX_WIN_STOP_WRITE_G(tcb->up_is_rx_win_stop, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_IS_TIMEOUT_WRITE_G(tcb->up_is_timeout, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_IS_ACTIVE_WRITE_G(tcb->up_is_active, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_tcpspdtest_engine_tcb_get(RDD_TCPSPDTEST_ENGINE_TCB_DTS *tcb, uint8_t stream_idx)
{
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_RX_PKTS_READ_G(tcb->rx_pkts, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_TXED_PKTS_READ_G(tcb->txed_pkts, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_FREED_PKTS_READ_G(tcb->freed_pkts, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_ACK_SEQ_READ_G(tcb->ack_seq, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_TX_SEQ_READ_G(tcb->tx_seq, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_BAD_PKTS_READ_G(tcb->bad_pkts, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_NO_CREDITS_READ_G(tcb->no_credits, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);

    /* BUG_ARRAY_NOT_ALLOWED_IN_MIDDLE_OF_TABLE - use array when bug is fixed */
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_BAD_BYTES_1_READ_G(tcb->bad_bytes_1, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_BAD_BYTES_READ_G(tcb->bad_bytes, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_RX_BYTES_1_READ_G(tcb->rx_bytes_1, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_RX_BYTES_READ_G(tcb->rx_bytes, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_EXPECTED_BYTES_1_READ_G(tcb->expected_bytes_1, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_EXPECTED_BYTES_READ_G(tcb->expected_bytes, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);

    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_TO_SEND_BYTES_1_READ_G(tcb->up_to_send_bytes_1, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_TO_SEND_BYTES_READ_G(tcb->up_to_send_bytes, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_CWND_READ_G(tcb->up_cwnd, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_CWND_THR_READ_G(tcb->up_cwnd_thr, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_CWND_BUDGET_READ_G(tcb->up_cwnd_budget, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_LAST_ACK_SEQ_READ_G(tcb->up_last_ack_seq, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_LAST_ACK_TIME_READ_G(tcb->up_last_ack_time, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_DUP_ACK_CNT_READ_G(tcb->up_dup_ack_cnt, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_DUP_ACK_SEQ_DONE_READ_G(tcb->up_dup_ack_seq_done, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_DUP_ACK_TOTAL_PKTS_CNT_READ_G(tcb->up_dup_ack_total_pkts_cnt, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_FAST_RETRANS_CNT_READ_G(tcb->up_fast_retrans_cnt, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_TIMEOUT_CNT_READ_G(tcb->up_timeout_cnt, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_RX_PKTS_AFTER_TIMEOUT_CNT_READ_G(tcb->up_rx_pkts_after_timeout_cnt, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_DUP_ACK_STATE_READ_G(tcb->up_dup_ack_state, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_IS_RX_WIN_STOP_READ_G(tcb->up_is_rx_win_stop, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_IS_TIMEOUT_READ_G(tcb->up_is_timeout, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_IS_ACTIVE_READ_G(tcb->up_is_active, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_tcpspdtest_engine_ref_pkt_hdr_set(uint16_t size, uint16_t offset, uint8_t *hdr, uint8_t stream_idx)
{
    uint32_t *entry;
    uint32_t *addr_arr;
    uint32_t addr;
    int mem_id;

    /* 
     * Set ref pkt for 1. Download or 2. Upload maximum length tx pkt
     */

	RDD_PKTGEN_TX_STREAM_ENTRY_REF_PKT_HDR_SIZE_WRITE_G(size, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_PKTGEN_TX_STREAM_ENTRY_REF_PKT_HDR_OFFSET_WRITE_G(offset, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);


    /* BUG_ARRAY_NOT_ALLOWED_IN_MIDDLE_OF_TABLE - use array when bug is fixed */
    entry = NULL;
    addr_arr = RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR;
    addr = stream_idx * sizeof(RDD_PKTGEN_TX_STREAM_ENTRY_DTS) + PKTGEN_TX_STREAM_ENTRY_REF_PKT_HDR_HDR_OFFSET + offset;

    for (mem_id = 0; mem_id < GROUPED_EN_SEGMENTS_NUM; mem_id++)
    {
        if (addr_arr[mem_id] == INVALID_TABLE_ADDRESS)
            continue;

        entry = (uint32_t *)(DEVICE_ADDRESS((rdp_runner_core_addr[mem_id] + addr_arr[mem_id]) + addr));
        break;
    }

    MWRITE_BLK_8(entry, hdr, size);

	return BDMF_ERR_OK;
}

bdmf_error_t rdd_tcpspdtest_engine_ref_pkt_hdr_get(uint16_t *size, uint16_t *offset, uint8_t *hdr, uint8_t stream_idx, uint16_t read_size)
{
    uint32_t *entry;
    uint32_t *addr_arr;
    uint32_t addr;
    int mem_id;

	RDD_PKTGEN_TX_STREAM_ENTRY_REF_PKT_HDR_SIZE_READ_G(*size, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_PKTGEN_TX_STREAM_ENTRY_REF_PKT_HDR_OFFSET_READ_G(*offset, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, stream_idx);

    /* BUG_ARRAY_NOT_ALLOWED_IN_MIDDLE_OF_TABLE - use array when bug is fixed */
    entry = NULL;
    addr_arr = RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR;
    addr = stream_idx * sizeof(RDD_TCPSPDTEST_STREAM_DTS) + PKTGEN_TX_STREAM_ENTRY_REF_PKT_HDR_HDR_OFFSET + *offset;

    for (mem_id = 0; mem_id < GROUPED_EN_SEGMENTS_NUM; mem_id++)
    {
        if (addr_arr[mem_id] == INVALID_TABLE_ADDRESS)
            continue;

        entry = (uint32_t *)(DEVICE_ADDRESS((rdp_runner_core_addr[mem_id] + addr_arr[mem_id]) + addr));
        break;
    }

    MREAD_BLK_8(hdr, entry, read_size);

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_tcpspdtest_engine_pkt_drop_set(RDD_TCPSPDTEST_ENGINE_PKT_DROP_DTS *pkt_drop, uint8_t stream_idx)
{
	int i;

    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_PKT_DROP_IS_DROP_WRITE_G(0, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_PKT_DROP_IS_WIN_FULL_WRITE_G(0, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_PKT_DROP_NUM_ERRS_WRITE_G(0, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_PKT_DROP_WR_OFS_WRITE_G(0, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_PKT_DROP_RD_OFS_WRITE_G(0, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);

    for (i=0; i<RDD_TCPSPDTEST_ENGINE_PKT_DROP_SEQ_NUMBER; i++)
	{
		RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_PKT_DROP_SEQ_WRITE_G(0, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx, i);
	}

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_tcpspdtest_engine_pkt_drop_get(RDD_TCPSPDTEST_ENGINE_PKT_DROP_DTS *pkt_drop, uint8_t stream_idx)
{
	RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_PKT_DROP_IS_DROP_READ_G(pkt_drop->is_drop, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
	RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_PKT_DROP_IS_WIN_FULL_READ_G(pkt_drop->is_win_full, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
	RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_PKT_DROP_NUM_ERRS_READ_G(pkt_drop->num_errs, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
	RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_PKT_DROP_WR_OFS_READ_G(pkt_drop->wr_ofs, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);
	RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_PKT_DROP_RD_OFS_READ_G(pkt_drop->rd_ofs, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_tcpspdtest_engine_start_upload_tx(uint8_t stream_idx, uint8_t num_active_streams)
{
    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_DTS global_info;

    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_NUM_STREAMS_READ_G(global_info.num_streams, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);
    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_STREAM_VECTOR_READ_G(global_info.stream_vector, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);

    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_IS_ACTIVE_WRITE_G(1, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);

    if (num_active_streams != global_info.num_streams || global_info.num_streams >= PROJ_DEFS_NUMBER_OF_TCPSPDTEST_STREAMS)
    {
        BDMF_TRACE_ERR("Upload start_tx stream_idx:%d number of streams state error ! (%d - %d)\n", stream_idx, num_active_streams, global_info.num_streams);
        return BDMF_ERR_STATE;
    }

    global_info.num_streams++;
    global_info.stream_vector |= (1 << stream_idx );

    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_NUM_STREAMS_WRITE_G(global_info.num_streams, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);
    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_STREAM_VECTOR_WRITE_G(global_info.stream_vector, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);

    if (1 == global_info.num_streams)
    {
        /* Set spdsvc task with tcpspdtest as owner */
        RDD_SPDTEST_GEN_CFG_TEST_TYPE_WRITE_G(SPDTEST_TCP, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);

        /* Kick spdsvc/tcpspdtest task */
        WMB();
        ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(spdsvc_gen_runner_image), SPDSVC_GEN_THREAD_NUMBER);
    }
    else
    {
        /* Wakeup the cpu_rx_copy task with tcpspdtest wakeup reason */
        //#ib#todo#: define common wakeup reason instead of #1
        RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY_CPU_RX_COPY_IS_TCPSPDTEST_TIMEOUT_WAKEUP_WRITE_G(1, RDD_SPDSVC_TCPSPDTEST_COMMON_TABLE_ADDRESS_ARR, 0);

        /* Kick cpu_rx_copy task */
        WMB();
        ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(cpu_rx_runner_image), PKTGEN_TX_THREAD_NUMBER);
    }

    return BDMF_ERR_OK;   
}

bdmf_error_t rdd_tcpspdtest_engine_stop_upload_tx (uint8_t stream_idx, uint8_t num_active_streams)
{
    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_DTS global_info;
    bdmf_error_t rc = BDMF_ERR_OK;

    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_NUM_STREAMS_READ_G(global_info.num_streams, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);
    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_STREAM_VECTOR_READ_G(global_info.stream_vector, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);

    RDD_TCPSPDTEST_STREAM_TCPSPDTEST_ENGINE_TCB_UP_IS_ACTIVE_WRITE_G(0, RDD_TCPSPDTEST_STREAM_TABLE_ADDRESS_ARR, stream_idx);

    if (num_active_streams != global_info.num_streams || global_info.num_streams == 0)
    {
        BDMF_TRACE_ERR("Upload stop_tx stream_idx:%d number of streams state error ! (%d - %d)\n", stream_idx, num_active_streams, global_info.num_streams);
        global_info.num_streams = 0;
        global_info.stream_vector = 0;
        /* #ib#todo#: reset l2_hdr for all streams */
        rc = BDMF_ERR_STATE;
    }

    if (BDMF_ERR_OK == rc)
    {
        global_info.num_streams--;
        global_info.stream_vector &= ~((uint32_t)(1 << stream_idx));

        if (0 == global_info.num_streams)
        {
            if (global_info.stream_vector)
            {
                BDMF_TRACE_ERR("Upload stop_tx stream_idx:%d stream vector:0x%08x state error !\n", stream_idx, global_info.stream_vector);
                global_info.stream_vector = 0;
                rc = BDMF_ERR_STATE;
            }

            /* Set spdsvc task with none owner */
            RDD_SPDTEST_GEN_CFG_TEST_TYPE_WRITE_G(SPDTEST_NONE, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);
        }
    }

    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_NUM_STREAMS_WRITE_G(global_info.num_streams, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);
    RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO_STREAM_VECTOR_WRITE_G(global_info.stream_vector, RDD_TCPSPDTEST_ENGINE_GLOBAL_TABLE_ADDRESS_ARR, 0);

    return rc;
}
