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
 * Chip: BCM68888_B0
 ******************************************************************************/

const ru_block_rec *RU_ALL_BLOCKS[] =
{
    &BAC_IF_BLOCK,
    &BBH_RX_BLOCK,
    &BBH_TX_BLOCK,
    &BUFMNG_BLOCK,
    &CNPL_BLOCK,
    &DMA_BLOCK,
    &DQM_TOKEN_FIFO_BLOCK,
    &DQM_BLOCK,
    &DQM_FPMINI_BLOCK,
    &DSPTCHR_BLOCK,
    &FPM_BLOCK,
    &FPM_FPMINI_BLOCK,
    &HASH_BLOCK,
    &NATC_BLOCK,
    &NATC_KEY_BLOCK,
    &NATC_TBL_BLOCK,
    &NATC_CTRS_BLOCK,
    &NATC_DDR_CFG_BLOCK,
    &NATC_ENG_BLOCK,
    &NATC_INDIR_BLOCK,
    &PSRAM_BLOCK,
    &PSRAM_MEM_BLOCK,
    &QM_BLOCK,
    &RNR_CNTXT_BLOCK,
    &RNR_INST_BLOCK,
    &RNR_MEM_BLOCK,
    &RNR_PRED_BLOCK,
    &RNR_QUAD_BLOCK,
    &RNR_REGS_BLOCK,
    &SBPM_BLOCK,
    &TCAM_BLOCK,
    &UBUS_REQU_BLOCK,
    &UBUS_RESP_BLOCK,
    &UNIMAC_MISC_BLOCK,
    &XLIF_XRDP0_BLOCK,
    &XLIF_XRDP1_BLOCK,
    &XLIF_XRDP2_BLOCK,
    &XUMAC_RDP_BLOCK,
    NULL,
};
