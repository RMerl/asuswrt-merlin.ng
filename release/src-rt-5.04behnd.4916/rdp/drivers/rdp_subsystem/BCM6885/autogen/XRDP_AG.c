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


#include "XRDP_AG.h"

/******************************************************************************
 * Chip: CRDP_BCM68850_A0
 ******************************************************************************/

const ru_block_rec *RU_ALL_BLOCKS[] =
{
    &ALL_RNR_CMN_BLOCK,
    &ALL_RNR_CNTXT_BLOCK,
    &ALL_RNR_INST_BLOCK,
    &ALL_RNR_MEM_BLOCK,
    &ALL_RNR_PRED_BLOCK,
    &ALL_RNR_REGS_BLOCK,
    &BAC_IF_BLOCK,
    &BBH_RX_BLOCK,
    &BUFMNG_BLOCK,
    &CNPL_BLOCK,
    &CRDP_ACC_UBUS_RESPONDER_BRDG_BLOCK,
    &CRDP_DS_CMN_BBH_TX_BLOCK,
    &CRDP_DS_CMN_PFB_BLOCK,
    &CRDP_DS_CMN_PSRAM_BLOCK,
    &CRDP_DS_CMN_SBPM_BLOCK,
    &CRDP_DS_CMN_UBUS_RESPONDER_BRDG_BLOCK,
    &CRDP_EGR_BBH_TX_BLOCK,
    &CRDP_EGR_CNPL_BLOCK,
    &CRDP_EGR_PFB_BLOCK,
    &CRDP_EGR_RNR_CMN_BLOCK,
    &CRDP_EGR_RNR_CNTXT_BLOCK,
    &CRDP_EGR_RNR_INST_BLOCK,
    &CRDP_EGR_RNR_MEM_BLOCK,
    &CRDP_EGR_RNR_PRED_BLOCK,
    &CRDP_EGR_RNR_REGS_BLOCK,
    &CRDP_EGR_UBUS_RESPONDER_BRDG_BLOCK,
    &CRDP_ING_PFB_BLOCK,
    &CRDP_ING_PSRAM_BLOCK,
    &CRDP_ING_SBPM_BLOCK,
    &CRDP_ING_UBUS_RESPONDER_BRDG_BLOCK,
    &CRDP_RCG_RNR_CMN_BLOCK,
    &CRDP_RCG_RNR_CNTXT_BLOCK,
    &CRDP_RCG_RNR_INST_BLOCK,
    &CRDP_RCG_RNR_MEM_BLOCK,
    &CRDP_RCG_RNR_PRED_BLOCK,
    &CRDP_RCG_RNR_REGS_BLOCK,
    &CRDP_RCG_UBUS_RESPONDER_BRDG_BLOCK,
    &DSPTCHR_BLOCK,
    &DSPTCHR_CMN_BLOCK,
    &FPM_BLOCK,
    &FPM_FPMINI_BLOCK,
    &GSU_MEM_BLOCK,
    &GSU_REG_BLOCK,
    &HASH_BLOCK,
    &NAT_CACHE_BLOCK,
    &PSRAM_MEM_BLOCK,
    &QM_BLOCK,
    &TCAM_BLOCK,
    NULL,
};
