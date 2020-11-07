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

#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_bac_if_ag.h"

#if defined(BCM6858)
#define BACIF_ID_TCAM_0 0
#define BACIF_ID_TCAM_1 1
#define BACIF_ID_HASH 2
#define BACIF_ID_CNPL 3
#define BACIF_ID_NATC 4
#elif defined(BCM6856)
#define BACIF_ID_TCAM_0 0
#define BACIF_ID_TCAM_1 4
#define BACIF_ID_HASH 1
#define BACIF_ID_CNPL 2
#define BACIF_ID_NATC 3
#else
#define BACIF_ID_TCAM 0
#define BACIF_ID_HASH 1
#define BACIF_ID_CNPL 2
#define BACIF_ID_NATC 3
#endif

int drv_bac_if_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val, bdmf_boolean keep_alive)
{
    bac_if_bacif_block_bacif_configurations_clk_gate_cntrl bacif_ctrl;
    uint8_t block_id = 0;

    for (block_id = 0; block_id < RU_BLK(BAC_IF).addr_count; block_id++){
        /* DO NOT auto clock gate BACIF #3 which is connected to the NATC (Nat Cache).
           Currently NATC is not supporting auto clock gating mode.
        */
        if (block_id == BACIF_ID_NATC)
            continue;
#if defined(BCM6858)
        if (block_id == BACIF_ID_CNPL)
            continue;
#endif
        ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_get(block_id, &bacif_ctrl);
        bacif_ctrl.bypass_clk_gate = auto_gate ? 0 : 1;
        bacif_ctrl.timer_val = timer_val;
        bacif_ctrl.keep_alive_en = keep_alive;
        ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_set(block_id, &bacif_ctrl);
    }
    return 0;
}
