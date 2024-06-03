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
