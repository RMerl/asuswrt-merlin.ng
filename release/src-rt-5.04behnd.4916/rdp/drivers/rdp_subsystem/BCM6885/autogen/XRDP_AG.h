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

#include "XRDP_ALL_RNR_CMN_AG.h"
#include "XRDP_ALL_RNR_CNTXT_AG.h"
#include "XRDP_ALL_RNR_INST_AG.h"
#include "XRDP_ALL_RNR_MEM_AG.h"
#include "XRDP_ALL_RNR_PRED_AG.h"
#include "XRDP_ALL_RNR_REGS_AG.h"
#include "XRDP_BAC_IF_AG.h"
#include "XRDP_BBH_RX_AG.h"
#include "XRDP_BUFMNG_AG.h"
#include "XRDP_CNPL_AG.h"
#include "XRDP_CRDP_ACC_UBUS_RESPONDER_BRDG_AG.h"
#include "XRDP_CRDP_DS_CMN_BBH_TX_AG.h"
#include "XRDP_CRDP_DS_CMN_PFB_AG.h"
#include "XRDP_CRDP_DS_CMN_PSRAM_AG.h"
#include "XRDP_CRDP_DS_CMN_SBPM_AG.h"
#include "XRDP_CRDP_DS_CMN_UBUS_RESPONDER_BRDG_AG.h"
#include "XRDP_CRDP_EGR_BBH_TX_AG.h"
#include "XRDP_CRDP_EGR_CNPL_AG.h"
#include "XRDP_CRDP_EGR_PFB_AG.h"
#include "XRDP_CRDP_EGR_RNR_CMN_AG.h"
#include "XRDP_CRDP_EGR_RNR_CNTXT_AG.h"
#include "XRDP_CRDP_EGR_RNR_INST_AG.h"
#include "XRDP_CRDP_EGR_RNR_MEM_AG.h"
#include "XRDP_CRDP_EGR_RNR_PRED_AG.h"
#include "XRDP_CRDP_EGR_RNR_REGS_AG.h"
#include "XRDP_CRDP_EGR_UBUS_RESPONDER_BRDG_AG.h"
#include "XRDP_CRDP_ING_PFB_AG.h"
#include "XRDP_CRDP_ING_PSRAM_AG.h"
#include "XRDP_CRDP_ING_SBPM_AG.h"
#include "XRDP_CRDP_ING_UBUS_RESPONDER_BRDG_AG.h"
#include "XRDP_CRDP_RCG_RNR_CMN_AG.h"
#include "XRDP_CRDP_RCG_RNR_CNTXT_AG.h"
#include "XRDP_CRDP_RCG_RNR_INST_AG.h"
#include "XRDP_CRDP_RCG_RNR_MEM_AG.h"
#include "XRDP_CRDP_RCG_RNR_PRED_AG.h"
#include "XRDP_CRDP_RCG_RNR_REGS_AG.h"
#include "XRDP_CRDP_RCG_UBUS_RESPONDER_BRDG_AG.h"
#include "XRDP_DSPTCHR_AG.h"
#include "XRDP_DSPTCHR_CMN_AG.h"
#include "XRDP_FPM_AG.h"
#include "XRDP_FPM_FPMINI_AG.h"
#include "XRDP_GSU_MEM_AG.h"
#include "XRDP_GSU_REG_AG.h"
#include "XRDP_HASH_AG.h"
#include "XRDP_NAT_CACHE_AG.h"
#include "XRDP_PSRAM_MEM_AG.h"
#include "XRDP_QM_AG.h"
#include "XRDP_RUNNER_IO_AG.h"
#include "XRDP_TCAM_AG.h"

extern const ru_block_rec *RU_ALL_BLOCKS[];
#define RU_BLOCK_COUNT 48
#define RU_REG_COUNT 1984
#define RU_CHIP_ID_LOWER "crdp_bcm68850_a0"
#define RU_CHIP_ID_UPPER "CRDP_BCM68850_A0"

#endif
