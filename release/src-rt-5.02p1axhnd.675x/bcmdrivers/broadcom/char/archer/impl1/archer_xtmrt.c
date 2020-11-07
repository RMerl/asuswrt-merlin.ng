/*
<:copyright-BRCM:2018:proprietary:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:> 
*/

/*
*******************************************************************************
*
* File Name  : archer_xtmrt.c
*
* Description: hooks for Archer API specific for XTM
*
*******************************************************************************
*/

#include <linux/module.h>
#include <linux/bcm_log.h>

#include <archer_cpu_queues.h>
#include "archer_xtmrt.h"

static int archer_xtm_deviceDetails (uint32_t devId, uint32_t encap, uint32_t trafficType, uint32_t bufStatus, uint32_t headerLen, uint32_t trailerLen)
{
    iudma_update_device_details (devId, encap, trafficType, bufStatus, headerLen, trailerLen);
    return 0;
}

static int archer_xtm_linkUp (uint32_t devId, uint32_t matchId, uint8_t txVcid)
{
    if(devId >= ARCHER_XTM_MAX_DEV_CTXS)
    {
        bcm_print("Invalid Device Context Index: %u", devId);

        return -1;
    }

    if(matchId >= ARCHER_XTM_MAX_MATCH_IDS)
    {
        bcm_print("Invalid Device Context Index: %u", matchId);

        return -1;
    }

    iudma_update_linkup(devId,matchId,txVcid);
    return 0;
}

static int archer_txdma_enable (uint32_t dmaIndex, uint32_t txVcid)
{
    /* this function will be called when TX queue shutdown */
    /* TDMA has to be disabled, and then flushed */
    iudma_txenable (dmaIndex, txVcid);
    return 0;
}

static int archer_txdma_disable (uint32_t dmaIndex)
{
    /* this function will be called when TX queue shutdown */
    /* TDMA has to be disabled, and then flushed */
    iudma_txdisable (dmaIndex);
    return 0;
}

static int archer_reinit_iudma (void)
{
    setup_iudma();
    return 0;
}

static int archer_txdma_setDropAlg (int queue_id, archer_drop_config_t *cfg)
{
    iudma_tx_dropAlg_set (queue_id, cfg);
    return 0;
}

static uint32_t archer_txdma_getQSize(void)
{
    return iudma_tx_qsize_get();
}


void bindXtmHandlers(void)
{
    bcmFun_t *archer_xtm_bind = bcmFun_get(BCM_FUN_ID_ARCHER_XTMRT_BIND);
    archer_xtm_hooks_t xtmHooks;

    BCM_ASSERT(archer_xtm_bind);

    xtmHooks.deviceDetails = archer_xtm_deviceDetails;
    xtmHooks.xtmLinkUp = archer_xtm_linkUp;
    xtmHooks.reInitDma = archer_reinit_iudma;
    xtmHooks.txdmaEnable = archer_txdma_enable;
    xtmHooks.txdmaDisable = archer_txdma_disable;
    xtmHooks.setTxChanDropAlg = archer_txdma_setDropAlg;
    xtmHooks.txdmaGetQSize = archer_txdma_getQSize;

    /* details for INGQOS and DropAlg can go here */
#if defined (CONFIG_BCM963268) || defined(CONFIG_BCM963178) 
    archer_xtm_bind(&xtmHooks);
#endif
}
