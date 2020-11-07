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
* File Name  : archer_xtmrt.h
*
* Description: This file contains XTMRT details needed by Archer
*
*******************************************************************************
*/

#ifndef __ARCHER_XTMRT_H__
#define __ARCHER_XTMRT_H__

#define ARCHER_XTM_MAX_DEV_CTXS   16    /* up to 256 */
#define ARCHER_XTM_MAX_MATCH_IDS  128

#define ARCHER_XTM_TRAFFIC_TYPE_ATM 0x1
#define ARCHER_XTM_TRAFFIC_TYPE_PTM 0x2

#define ARCHER_XTM_FSTAT_CT_MASK 0x000000f0
#define ARCHER_XTM_FSTAT_CT_AAL5 0x00000070

void bindXtmHandlers(void);

void iudma_update_device_details (uint32_t dev_id, uint32_t encap, uint32_t trafficType, uint32_t bufStatus, uint32_t headerLen, uint32_t trailerLen);
void iudma_update_linkup(uint32_t devId, uint32_t matchId, uint8_t txVcid);
int iudma_txenable(uint32_t dmaIndex, uint32_t txVcid);
int iudma_txdisable(uint32_t dmaIndex);
void setup_iudma(void);
int iudma_tx_dropAlg_set (int queue_id, archer_drop_config_t *cfg);
int iudma_tx_dropAlg_get (int queue_id, archer_drop_config_t *cfg);
uint32_t iudma_tx_qsize_get (void);

void iudma_driver_stats(void);
int iudma_txq_stats_get(int queue_id, archer_txq_stats_t *stats_p);

int __init iudma_driver_construct(void);

#endif /* __ARCHER_XTMRT_H__ */
