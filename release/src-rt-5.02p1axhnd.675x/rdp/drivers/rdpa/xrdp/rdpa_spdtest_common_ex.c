/*
* <:copyright-BRCM:2018:proprietary:standard
*
*    Copyright (c) 2018 Broadcom
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
 :>
*/

#include <rdpa_api.h>
#include "rdpa_spdtest_common.h"
#include "rdpa_spdtest_common_ex.h"
#include "rdpa_int.h"
#include "rdd.h"
#include "rdd_ag_pktgen_tx.h"
#include "rdp_drv_sbpm.h"
#include "rdp_drv_qm.h"

static void spdt_tx_free_ref_pkt_sbpms(spdt_ref_pkt_container_t *container, int pkt_id)
{
    int i;
    uint16_t sbpm_end_ptr, total_num_of_bns;

    for (i = 0; i < MAX_NUM_OF_HDRS_PER_STREAM; i++)
    {
        if (container->sbpm_pkt_head_bn[pkt_id][i] != SBPM_INVALID_BUFFER_NUMBER)
        {
            /* If SBPM list is already allocated for index, free it first */
            drv_sbpm_free_list(container->sbpm_pkt_head_bn[pkt_id][i]);

            rdd_ag_pktgen_tx_pktgen_sbpm_hdr_bns_set(pkt_id * MAX_NUM_OF_HDRS_PER_STREAM + i,
                SBPM_INVALID_BUFFER_NUMBER, 0);
            container->sbpm_pkt_head_bn[pkt_id][i] = SBPM_INVALID_BUFFER_NUMBER;
        }
    }
    rdd_ag_pktgen_tx_pktgen_sbpm_exts_set(pkt_id, 0, SBPM_INVALID_BUFFER_NUMBER);

    rdd_ag_pktgen_tx_pktgen_num_of_avail_sbpm_hdrs_get(&total_num_of_bns);
    total_num_of_bns -= MAX_NUM_OF_HDRS_PER_STREAM;
    rdd_ag_pktgen_tx_pktgen_num_of_avail_sbpm_hdrs_set(total_num_of_bns);

    sbpm_end_ptr = RDD_PKTGEN_SBPM_HDR_BNS_ADDRESS_ARR[get_runner_idx(tcpspdtest_engine_runner_image)] + total_num_of_bns * 2;
    rdd_ag_pktgen_tx_pktgen_sbpm_end_ptr_set(sbpm_end_ptr);

    memset(&(container->pkt[pkt_id]), 0, sizeof(rdpa_spdtest_ref_pkt_t));
    container->total_num_of_bns = total_num_of_bns;
}

int spdt_tx_ref_pkt_set(spdt_ref_pkt_container_t *container, bdmf_index index, rdpa_spdtest_ref_pkt_t *ref_pkt)
{
    int rc, i;
    uint16_t next_bn, last_bn, total_num_of_bns;
    uint16_t sbpm_end_ptr;
    uint8_t num_of_bns;

    for (i = 0; i < index; i++)
    {
        if (container->sbpm_pkt_head_bn[i][0] == SBPM_INVALID_BUFFER_NUMBER && (ref_pkt && ref_pkt->size))
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Please set ref pkt %d first\n", i);
    }
    /* If SBPM list is already allocated for index, free it first. First index is a good indication for alloc */
    if (container->sbpm_pkt_head_bn[index][0] != SBPM_INVALID_BUFFER_NUMBER)
        spdt_tx_free_ref_pkt_sbpms(container, index);

    if (!ref_pkt || !ref_pkt->size)
        return 0; /* Only release allocated list */
    
    /* Allocate new SBPM list and copy the header to it */
    rc = drv_sbpm_alloc_list(ref_pkt->size, RDD_PACKET_HEADROOM_OFFSET, (uint8_t *)ref_pkt->data,
        &(container->sbpm_pkt_head_bn[index][0]), &last_bn, &num_of_bns);
    if (rc)
        goto exit;

    /* Store first BN to SRAM */
    rc = rdd_ag_pktgen_tx_pktgen_sbpm_hdr_bns_set(index * MAX_NUM_OF_HDRS_PER_STREAM,
        container->sbpm_pkt_head_bn[index][0], index);
    if (rc)
        goto exit;

    rc = rdd_ag_pktgen_tx_pktgen_sbpm_exts_set(index, num_of_bns, last_bn);
    if (rc)
        goto exit;

    /* Allocate additional headers for stream */
    next_bn = ref_pkt->size > SBPM_BUF_SIZE ?
        drv_sbpm_get_next_bn(container->sbpm_pkt_head_bn[index][0]) : SBPM_INVALID_BUFFER_NUMBER;
    for (i = 1; i < MAX_NUM_OF_HDRS_PER_STREAM; i++)
    {
        rc = drv_sbpm_alloc_and_connect_next_bn(ref_pkt->size, RDD_PACKET_HEADROOM_OFFSET, (uint8_t *)ref_pkt->data,
            next_bn, 1, &(container->sbpm_pkt_head_bn[index][i]));
        rc = rc ? rc : rdd_ag_pktgen_tx_pktgen_sbpm_hdr_bns_set(index * MAX_NUM_OF_HDRS_PER_STREAM + i,
            container->sbpm_pkt_head_bn[index][i], index);
        if (rc)
            goto exit;
    }

    /* stream added, update streams counter */
    rdd_ag_pktgen_tx_pktgen_num_of_avail_sbpm_hdrs_get(&total_num_of_bns);
    total_num_of_bns += MAX_NUM_OF_HDRS_PER_STREAM;
    rdd_ag_pktgen_tx_pktgen_num_of_avail_sbpm_hdrs_set(total_num_of_bns);

    sbpm_end_ptr = RDD_PKTGEN_SBPM_HDR_BNS_ADDRESS_ARR[get_runner_idx(tcpspdtest_engine_runner_image)] + total_num_of_bns * 2;
    rdd_ag_pktgen_tx_pktgen_sbpm_end_ptr_set(sbpm_end_ptr);

    memcpy(&(container->pkt[index]), ref_pkt, sizeof(rdpa_spdtest_ref_pkt_t));
    container->total_num_of_bns = total_num_of_bns;

exit:
    if (rc)
        spdt_tx_free_ref_pkt_sbpms(container, index);

    return rc;
}

void spdt_tx_ref_pkts_free(spdt_ref_pkt_container_t *container)
{
    int i;
    uint16_t total_num_of_bns;

    /* Prior to releasing the SBPMs, verify that TX stopped and all SBPMs are back to the ref_pkt pool. */
    do {
        rdd_ag_pktgen_tx_pktgen_num_of_avail_sbpm_hdrs_get(&total_num_of_bns);
        BDMF_TRACE_DBG("Waiting to complete the packets, container BNs %d, completed %d\n",
            container->total_num_of_bns, total_num_of_bns);
    } while (container->total_num_of_bns != total_num_of_bns);

    for (i = 0; i < PKTGEN_TX_NUM_OF_DATA_PKTS; i++)
    {
        if (container->sbpm_pkt_head_bn[i][0] != SBPM_INVALID_BUFFER_NUMBER)
            spdt_tx_free_ref_pkt_sbpms(container, i);
    }

    rdd_ag_pktgen_tx_pktgen_num_of_avail_sbpm_hdrs_set(0);
    rdd_ag_pktgen_tx_pktgen_curr_sbpm_hdr_ptr_set(RDD_PKTGEN_SBPM_HDR_BNS_ADDRESS_ARR[get_runner_idx(tcpspdtest_engine_runner_image)]);
}

void spdt_tx_defs_init(int thread_num)
{
    uint32_t bb_msg_0 = 0, bb_msg_1 = 0;
    uint32_t ref_pd_0 = 0, ref_pd_1 = 0, ref_pd_2 = 0, ref_pd_3 = 0;
    int core_id = get_runner_idx(tcpspdtest_engine_runner_image);

    /* Prepare BBMSG template for mcast increment */
    /* Reply address in scratch; first 16 bytes used for dispatcher buffer allocation, third 8 bytes for SBPM mcast inc
     * reply */
    RDD_SBPM_MULTICAST_REQUEST_TARGET_ADDRESS_L_WRITE(bb_msg_0,
        RDD_PKTGEN_BBMSG_REPLY_SCRATCH_ADDRESS_ARR[core_id] >> 3);
    RDD_SBPM_MULTICAST_REQUEST_TASK_NUM_L_WRITE(bb_msg_0, thread_num);
    RDD_SBPM_MULTICAST_REQUEST_WAKEUP_L_WRITE(bb_msg_0, 1);
    RDD_SBPM_MULTICAST_REQUEST_SYNCH_L_WRITE(bb_msg_0, 0); /* Sync */

    RDD_SBPM_MULTICAST_REQUEST_VALUE_L_WRITE(bb_msg_1, 1); /* Mcast increment by 1 */
    RDD_SBPM_MULTICAST_REQUEST_ACK_REQUEST_L_WRITE(bb_msg_1, 1); /* Wait for ACK */

    /* Prepare reference PD template */
    /* PD-0 */
    RDD_PROCESSING_TX_DESCRIPTOR_VALID_L_WRITE(ref_pd_0, 1);
    RDD_PROCESSING_TX_DESCRIPTOR_REPROCESS_L_WRITE(ref_pd_0, 1); /* Reprocessing bit to avoid ingress-to-egress by QM */
    RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_L_WRITE(ref_pd_0, QM_QUEUE_PKTGEN_REPROCESSING);
    RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_L_WRITE(ref_pd_0, 1); /* Copy to FPM and ingore second level queue */
    RDD_PROCESSING_TX_DESCRIPTOR_COLOR_L_WRITE(ref_pd_0, 1); /* Green indication for WRED */

    /* PD-2 */
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_1_L_WRITE(ref_pd_2, 1); /* Target_mem_1 = 1 to indicate QM to ingore BN1
                                                                       when updating FPM user-group counters */

    /* PD-3 */
    RDD_PROCESSING_TX_DESCRIPTOR_SOP_L_WRITE(ref_pd_3, RDD_PACKET_HEADROOM_OFFSET); 
    RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_0_L_WRITE(ref_pd_3, 1); /* SBPM */

    rdd_ag_pktgen_tx_pktgen_session_data_set(bb_msg_0, bb_msg_1, ref_pd_0, ref_pd_1, ref_pd_2, ref_pd_3);

    /* Init sbpms header pointer */
    rdd_ag_pktgen_tx_pktgen_curr_sbpm_hdr_ptr_set(RDD_PKTGEN_SBPM_HDR_BNS_ADDRESS_ARR[core_id]);
}

extern uintptr_t xrdp_virt2phys(const ru_block_rec *ru_block, uint8_t addr_idx);
#define MIN_RSV_UG_THRESHOLD 256
extern dpi_params_t dpi_params;
void spdt_tx_fpm_ug_budget_set(uint32_t fpm_ug)
{
    uint32_t budget, fpm_ug_cnt_reg_addr;
    int32_t fpm_ug_threshold = 0;
    uint16_t fpm_ug_cnt; 
    qm_fpm_ug_thr fpm_ug_thr;
    int fpm_tokens_quantum = 3; /* Assuming FPM_BUF_SIZE_256 */

    switch (dpi_params.fpm_buf_size)
    {
    case FPM_BUF_SIZE_2K:
        fpm_tokens_quantum = 0;
        break;
    case FPM_BUF_SIZE_1K:
        fpm_tokens_quantum = 1;
        break;
    case FPM_BUF_SIZE_512:
        fpm_tokens_quantum = 2;
        break;
    default:
        break;
    }

    /* this calculation bring us to HIGHER_THR which is for alignment purpose because of DMA HW issue in 6878
     * we will read 8 bytes instead of 4 to fix it */
    fpm_ug_cnt_reg_addr = xrdp_virt2phys(&RU_BLK(QM), 0) + RU_REG_OFFSET(QM, FPM_USR_GRP_CNT) +
        RU_REG(QM, FPM_USR_GRP_CNT).offset * fpm_ug - 4;
    ag_drv_qm_fpm_ug_thr_get(fpm_ug, &fpm_ug_thr);

    /* Some huristics to apply minimum threshold. TBD: align if necessary to minimum buffer reservation logic */
    fpm_ug_threshold = fpm_ug_thr.lower_thr - (MIN_RSV_UG_THRESHOLD << fpm_tokens_quantum);

    ag_drv_qm_fpm_ug_cnt_get(fpm_ug, &fpm_ug_cnt);
    if (fpm_ug_cnt > fpm_ug_threshold)
    {
        budget = 0; /* Should not happen */
        BDMF_TRACE_ERR("Too many traffic in the system (FPMs in use = %d), cannot start the test immediately!\n",
            fpm_ug_cnt);
    }
    else
        budget = fpm_ug_threshold - fpm_ug_cnt;
    
    budget >>= fpm_tokens_quantum;
    fpm_ug_threshold >>= fpm_tokens_quantum;
    BDMF_TRACE_DBG("===== FPM UG = %d, budget = %d, fpm_ug_cnt_reg_addr = %x, fpm_ug_threshold = %d, "
        "tokens_quantum = %d\n", fpm_ug, budget, fpm_ug_cnt_reg_addr, fpm_ug_threshold, fpm_tokens_quantum);
    rdd_ag_pktgen_tx_pktgen_fpm_ug_mgmt_set(budget, 0, 0, fpm_ug_cnt_reg_addr, fpm_ug_threshold, fpm_tokens_quantum);
}

void spdt_tx_sbpm_bn_reset(spdt_ref_pkt_container_t *container)
{
    int i, j;

    for (i = 0; i < PKTGEN_TX_NUM_OF_DATA_PKTS; i++)
    {
        for (j = 0; j < MAX_NUM_OF_HDRS_PER_STREAM; j++)
        {
            container->sbpm_pkt_head_bn[i][j] = SBPM_INVALID_BUFFER_NUMBER;
            rdd_ag_pktgen_tx_pktgen_sbpm_hdr_bns_set(i * MAX_NUM_OF_HDRS_PER_STREAM + j,
                SBPM_INVALID_BUFFER_NUMBER, 0);
        }
        rdd_ag_pktgen_tx_pktgen_sbpm_exts_set(i, 0, SBPM_INVALID_BUFFER_NUMBER);
    }
}

