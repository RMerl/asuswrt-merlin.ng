/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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
