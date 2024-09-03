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


#ifndef _XRDP_AG_H_
#define _XRDP_AG_H_

#include "ru_types.h"

#include "XRDP_BAC_IF_AG.h"
#include "XRDP_BBH_RX_AG.h"
#include "XRDP_BBH_TX_AG.h"
#include "XRDP_BUFMNG_AG.h"
#include "XRDP_CNPL_AG.h"
#include "XRDP_DMA_AG.h"
#include "XRDP_DQM_AG.h"
#include "XRDP_DQM_FPMINI_AG.h"
#include "XRDP_DSPTCHR_AG.h"
#include "XRDP_FPM_AG.h"
#include "XRDP_FPM_FPMINI_AG.h"
#include "XRDP_HASH_AG.h"
#include "XRDP_NATC_AG.h"
#include "XRDP_NATC_CFG_AG.h"
#include "XRDP_NATC_CTRS_AG.h"
#include "XRDP_NATC_DDR_CFG_AG.h"
#include "XRDP_NATC_ENG_AG.h"
#include "XRDP_NATC_INDIR_AG.h"
#include "XRDP_PSRAM_AG.h"
#include "XRDP_PSRAM_MEM_AG.h"
#include "XRDP_QM_AG.h"
#include "XRDP_RNR_CNTXT_AG.h"
#include "XRDP_RNR_INST_AG.h"
#include "XRDP_RNR_MEM_AG.h"
#include "XRDP_RNR_PRED_AG.h"
#include "XRDP_RNR_QUAD_AG.h"
#include "XRDP_RNR_REGS_AG.h"
#include "XRDP_SBPM_AG.h"
#include "XRDP_TCAM_AG.h"
#include "XRDP_UBUS_REQU_AG.h"
#include "XRDP_UBUS_RESP_AG.h"
#include "XRDP_UNIMAC_MISC_AG.h"
#include "XRDP_XLIF_XRDP0_AG.h"
#include "XRDP_XLIF_XRDP1_AG.h"
#include "XRDP_XLIF_XRDP2_AG.h"
#include "XRDP_XUMAC_RDP_AG.h"

extern const ru_block_rec *RU_ALL_BLOCKS[];
#define RU_BLOCK_COUNT 36
#define RU_REG_COUNT 1350
#define RU_CHIP_ID_LOWER "bcm68837_a0"
#define RU_CHIP_ID_UPPER "BCM68837_A0"

#endif
