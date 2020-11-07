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


#ifndef _RDD_AG_PKTGEN_TX_H_
#define _RDD_AG_PKTGEN_TX_H_

#include "rdd.h"

int rdd_ag_pktgen_tx_pktgen_fpm_ug_mgmt_set(uint32_t budget, uint32_t fpm_ug_cnt_dummy, uint32_t fpm_ug_cnt, uint32_t fpm_ug_cnt_reg_addr, uint32_t fpm_ug_threshold, uint8_t fpm_tokens_quantum);
int rdd_ag_pktgen_tx_pktgen_fpm_ug_mgmt_get(uint32_t *budget, uint32_t *fpm_ug_cnt_dummy, uint32_t *fpm_ug_cnt, uint32_t *fpm_ug_cnt_reg_addr, uint32_t *fpm_ug_threshold, uint8_t *fpm_tokens_quantum);
int rdd_ag_pktgen_tx_pktgen_no_sbpm_hdrs_cntr_set(uint32_t bits);
int rdd_ag_pktgen_tx_pktgen_no_sbpm_hdrs_cntr_get(uint32_t *bits);
int rdd_ag_pktgen_tx_pktgen_sbpm_hdr_bns_set(uint32_t _entry, uint16_t first_bn, uint8_t ext_idx);
int rdd_ag_pktgen_tx_pktgen_sbpm_hdr_bns_get(uint32_t _entry, uint16_t *first_bn, uint8_t *ext_idx);
int rdd_ag_pktgen_tx_pktgen_curr_sbpm_hdr_ptr_set(uint16_t bits);
int rdd_ag_pktgen_tx_pktgen_curr_sbpm_hdr_ptr_get(uint16_t *bits);
int rdd_ag_pktgen_tx_pktgen_num_of_avail_sbpm_hdrs_set(uint16_t bits);
int rdd_ag_pktgen_tx_pktgen_num_of_avail_sbpm_hdrs_get(uint16_t *bits);
int rdd_ag_pktgen_tx_pktgen_sbpm_end_ptr_set(uint16_t bits);
int rdd_ag_pktgen_tx_pktgen_sbpm_end_ptr_get(uint16_t *bits);
int rdd_ag_pktgen_tx_pktgen_max_ut_pkts_set(uint32_t bits);
int rdd_ag_pktgen_tx_pktgen_max_ut_pkts_get(uint32_t *bits);
int rdd_ag_pktgen_tx_pktgen_ut_trigger_set(uint32_t bits);
int rdd_ag_pktgen_tx_pktgen_ut_trigger_get(uint32_t *bits);
int rdd_ag_pktgen_tx_pktgen_sbpm_exts_set(uint32_t _entry, uint8_t num_of_bns, uint16_t bn1);
int rdd_ag_pktgen_tx_pktgen_sbpm_exts_get(uint32_t _entry, uint8_t *num_of_bns, uint16_t *bn1);
int rdd_ag_pktgen_tx_pktgen_session_data_set(uint32_t bbmsg_sbpm_mcast_inc_req_0, uint32_t bbmsg_sbpm_mcast_inc_req_1, uint32_t ref_pd_0, uint32_t ref_pd_1, uint32_t ref_pd_2, uint32_t ref_pd_3);
int rdd_ag_pktgen_tx_pktgen_session_data_get(uint32_t *bbmsg_sbpm_mcast_inc_req_0, uint32_t *bbmsg_sbpm_mcast_inc_req_1, uint32_t *ref_pd_0, uint32_t *ref_pd_1, uint32_t *ref_pd_2, uint32_t *ref_pd_3);

#endif /* _RDD_AG_PKTGEN_TX_H_ */
