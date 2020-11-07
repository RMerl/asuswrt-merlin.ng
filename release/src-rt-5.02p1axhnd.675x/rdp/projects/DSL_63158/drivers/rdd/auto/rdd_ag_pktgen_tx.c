/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/



/* This is an automated file. Do not edit its contents. */


#include "rdd_ag_pktgen_tx.h"

int rdd_ag_pktgen_tx_pktgen_no_sbpm_hdrs_cntr_set(uint32_t bits)
{
    RDD_BYTES_4_BITS_WRITE_G(bits, RDD_PKTGEN_NO_SBPM_HDRS_CNTR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_no_sbpm_hdrs_cntr_get(uint32_t *bits)
{
    RDD_BYTES_4_BITS_READ_G(*bits, RDD_PKTGEN_NO_SBPM_HDRS_CNTR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_fpm_ug_mgmt_set(uint32_t budget, uint32_t fpm_ug_cnt_dummy, uint32_t fpm_ug_cnt, uint32_t fpm_ug_cnt_reg_addr, uint32_t fpm_ug_threshold, uint8_t fpm_tokens_quantum)
{
    if(fpm_ug_threshold >= 268435456 || fpm_tokens_quantum >= 16)
          return BDMF_ERR_PARM;

    RDD_PKTGEN_FPM_UG_MGMT_ENTRY_BUDGET_WRITE_G(budget, RDD_PKTGEN_FPM_UG_MGMT_ADDRESS_ARR, 0);
    RDD_PKTGEN_FPM_UG_MGMT_ENTRY_FPM_UG_CNT_DUMMY_WRITE_G(fpm_ug_cnt_dummy, RDD_PKTGEN_FPM_UG_MGMT_ADDRESS_ARR, 0);
    RDD_PKTGEN_FPM_UG_MGMT_ENTRY_FPM_UG_CNT_WRITE_G(fpm_ug_cnt, RDD_PKTGEN_FPM_UG_MGMT_ADDRESS_ARR, 0);
    RDD_PKTGEN_FPM_UG_MGMT_ENTRY_FPM_UG_CNT_REG_ADDR_WRITE_G(fpm_ug_cnt_reg_addr, RDD_PKTGEN_FPM_UG_MGMT_ADDRESS_ARR, 0);
    RDD_PKTGEN_FPM_UG_MGMT_ENTRY_FPM_UG_THRESHOLD_WRITE_G(fpm_ug_threshold, RDD_PKTGEN_FPM_UG_MGMT_ADDRESS_ARR, 0);
    RDD_PKTGEN_FPM_UG_MGMT_ENTRY_FPM_TOKENS_QUANTUM_WRITE_G(fpm_tokens_quantum, RDD_PKTGEN_FPM_UG_MGMT_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_fpm_ug_mgmt_get(uint32_t *budget, uint32_t *fpm_ug_cnt_dummy, uint32_t *fpm_ug_cnt, uint32_t *fpm_ug_cnt_reg_addr, uint32_t *fpm_ug_threshold, uint8_t *fpm_tokens_quantum)
{
    RDD_PKTGEN_FPM_UG_MGMT_ENTRY_BUDGET_READ_G(*budget, RDD_PKTGEN_FPM_UG_MGMT_ADDRESS_ARR, 0);
    RDD_PKTGEN_FPM_UG_MGMT_ENTRY_FPM_UG_CNT_DUMMY_READ_G(*fpm_ug_cnt_dummy, RDD_PKTGEN_FPM_UG_MGMT_ADDRESS_ARR, 0);
    RDD_PKTGEN_FPM_UG_MGMT_ENTRY_FPM_UG_CNT_READ_G(*fpm_ug_cnt, RDD_PKTGEN_FPM_UG_MGMT_ADDRESS_ARR, 0);
    RDD_PKTGEN_FPM_UG_MGMT_ENTRY_FPM_UG_CNT_REG_ADDR_READ_G(*fpm_ug_cnt_reg_addr, RDD_PKTGEN_FPM_UG_MGMT_ADDRESS_ARR, 0);
    RDD_PKTGEN_FPM_UG_MGMT_ENTRY_FPM_UG_THRESHOLD_READ_G(*fpm_ug_threshold, RDD_PKTGEN_FPM_UG_MGMT_ADDRESS_ARR, 0);
    RDD_PKTGEN_FPM_UG_MGMT_ENTRY_FPM_TOKENS_QUANTUM_READ_G(*fpm_tokens_quantum, RDD_PKTGEN_FPM_UG_MGMT_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_curr_sbpm_hdr_ptr_set(uint16_t bits)
{
    RDD_BYTES_2_BITS_WRITE_G(bits, RDD_PKTGEN_CURR_SBPM_HDR_PTR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_curr_sbpm_hdr_ptr_get(uint16_t *bits)
{
    RDD_BYTES_2_BITS_READ_G(*bits, RDD_PKTGEN_CURR_SBPM_HDR_PTR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_num_of_avail_sbpm_hdrs_set(uint16_t bits)
{
    RDD_BYTES_2_BITS_WRITE_G(bits, RDD_PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_num_of_avail_sbpm_hdrs_get(uint16_t *bits)
{
    RDD_BYTES_2_BITS_READ_G(*bits, RDD_PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_sbpm_hdr_bns_set(uint32_t _entry, uint16_t first_bn, uint8_t ext_idx)
{
    if(_entry >= RDD_PKTGEN_SBPM_HDR_BNS_SIZE || first_bn >= 16384 || ext_idx >= 4)
          return BDMF_ERR_PARM;

    RDD_PKTGEN_SBPM_HDR_BN_FIRST_BN_WRITE_G(first_bn, RDD_PKTGEN_SBPM_HDR_BNS_ADDRESS_ARR, _entry);
    RDD_PKTGEN_SBPM_HDR_BN_EXT_IDX_WRITE_G(ext_idx, RDD_PKTGEN_SBPM_HDR_BNS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_sbpm_hdr_bns_get(uint32_t _entry, uint16_t *first_bn, uint8_t *ext_idx)
{
    if(_entry >= RDD_PKTGEN_SBPM_HDR_BNS_SIZE)
          return BDMF_ERR_PARM;

    RDD_PKTGEN_SBPM_HDR_BN_FIRST_BN_READ_G(*first_bn, RDD_PKTGEN_SBPM_HDR_BNS_ADDRESS_ARR, _entry);
    RDD_PKTGEN_SBPM_HDR_BN_EXT_IDX_READ_G(*ext_idx, RDD_PKTGEN_SBPM_HDR_BNS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_sbpm_end_ptr_set(uint16_t bits)
{
    RDD_BYTES_2_BITS_WRITE_G(bits, RDD_PKTGEN_SBPM_END_PTR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_sbpm_end_ptr_get(uint16_t *bits)
{
    RDD_BYTES_2_BITS_READ_G(*bits, RDD_PKTGEN_SBPM_END_PTR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_max_ut_pkts_set(uint32_t bits)
{
    RDD_BYTES_4_BITS_WRITE_G(bits, RDD_PKTGEN_MAX_UT_PKTS_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_max_ut_pkts_get(uint32_t *bits)
{
    RDD_BYTES_4_BITS_READ_G(*bits, RDD_PKTGEN_MAX_UT_PKTS_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_ut_trigger_set(uint32_t bits)
{
    RDD_BYTES_4_BITS_WRITE_G(bits, RDD_PKTGEN_UT_TRIGGER_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_ut_trigger_get(uint32_t *bits)
{
    RDD_BYTES_4_BITS_READ_G(*bits, RDD_PKTGEN_UT_TRIGGER_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_session_data_set(uint32_t bbmsg_sbpm_mcast_inc_req_0, uint32_t bbmsg_sbpm_mcast_inc_req_1, uint32_t ref_pd_0, uint32_t ref_pd_1, uint32_t ref_pd_2, uint32_t ref_pd_3)
{
    RDD_PKTGEN_TX_PARAMS_BBMSG_SBPM_MCAST_INC_REQ_0_WRITE_G(bbmsg_sbpm_mcast_inc_req_0, RDD_PKTGEN_SESSION_DATA_ADDRESS_ARR, 0);
    RDD_PKTGEN_TX_PARAMS_BBMSG_SBPM_MCAST_INC_REQ_1_WRITE_G(bbmsg_sbpm_mcast_inc_req_1, RDD_PKTGEN_SESSION_DATA_ADDRESS_ARR, 0);
    RDD_PKTGEN_TX_PARAMS_REF_PD_0_WRITE_G(ref_pd_0, RDD_PKTGEN_SESSION_DATA_ADDRESS_ARR, 0);
    RDD_PKTGEN_TX_PARAMS_REF_PD_1_WRITE_G(ref_pd_1, RDD_PKTGEN_SESSION_DATA_ADDRESS_ARR, 0);
    RDD_PKTGEN_TX_PARAMS_REF_PD_2_WRITE_G(ref_pd_2, RDD_PKTGEN_SESSION_DATA_ADDRESS_ARR, 0);
    RDD_PKTGEN_TX_PARAMS_REF_PD_3_WRITE_G(ref_pd_3, RDD_PKTGEN_SESSION_DATA_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_session_data_get(uint32_t *bbmsg_sbpm_mcast_inc_req_0, uint32_t *bbmsg_sbpm_mcast_inc_req_1, uint32_t *ref_pd_0, uint32_t *ref_pd_1, uint32_t *ref_pd_2, uint32_t *ref_pd_3)
{
    RDD_PKTGEN_TX_PARAMS_BBMSG_SBPM_MCAST_INC_REQ_0_READ_G(*bbmsg_sbpm_mcast_inc_req_0, RDD_PKTGEN_SESSION_DATA_ADDRESS_ARR, 0);
    RDD_PKTGEN_TX_PARAMS_BBMSG_SBPM_MCAST_INC_REQ_1_READ_G(*bbmsg_sbpm_mcast_inc_req_1, RDD_PKTGEN_SESSION_DATA_ADDRESS_ARR, 0);
    RDD_PKTGEN_TX_PARAMS_REF_PD_0_READ_G(*ref_pd_0, RDD_PKTGEN_SESSION_DATA_ADDRESS_ARR, 0);
    RDD_PKTGEN_TX_PARAMS_REF_PD_1_READ_G(*ref_pd_1, RDD_PKTGEN_SESSION_DATA_ADDRESS_ARR, 0);
    RDD_PKTGEN_TX_PARAMS_REF_PD_2_READ_G(*ref_pd_2, RDD_PKTGEN_SESSION_DATA_ADDRESS_ARR, 0);
    RDD_PKTGEN_TX_PARAMS_REF_PD_3_READ_G(*ref_pd_3, RDD_PKTGEN_SESSION_DATA_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_sbpm_exts_set(uint32_t _entry, uint8_t num_of_bns, uint16_t bn1)
{
    if(_entry >= RDD_PKTGEN_SBPM_EXTS_SIZE || num_of_bns >= 16 || bn1 >= 16384)
          return BDMF_ERR_PARM;

    RDD_PKTGEN_SBPM_EXT_NUM_OF_BNS_WRITE_G(num_of_bns, RDD_PKTGEN_SBPM_EXTS_ADDRESS_ARR, _entry);
    RDD_PKTGEN_SBPM_EXT_BN1_WRITE_G(bn1, RDD_PKTGEN_SBPM_EXTS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_pktgen_tx_pktgen_sbpm_exts_get(uint32_t _entry, uint8_t *num_of_bns, uint16_t *bn1)
{
    if(_entry >= RDD_PKTGEN_SBPM_EXTS_SIZE)
          return BDMF_ERR_PARM;

    RDD_PKTGEN_SBPM_EXT_NUM_OF_BNS_READ_G(*num_of_bns, RDD_PKTGEN_SBPM_EXTS_ADDRESS_ARR, _entry);
    RDD_PKTGEN_SBPM_EXT_BN1_READ_G(*bn1, RDD_PKTGEN_SBPM_EXTS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

