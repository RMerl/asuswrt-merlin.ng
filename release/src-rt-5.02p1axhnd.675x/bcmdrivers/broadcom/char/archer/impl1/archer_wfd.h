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
* File Name  : archer_wfd.h
*
* Description: Archer WLAN Forwarding Driver Header
*
*******************************************************************************
*/

#ifndef __ARCHER_WFD_H__
#define __ARCHER_WFD_H__

#include "archer_cpu_queues.h"

extern archer_wfd_hooks_t archer_wfd_hooks;

extern uint32_t archer_wfd_tx_context_g;

static inline int archer_wlan_tx_packet(int egress_port, int egress_queue, FkBuff_t *fkb_p)
{
    archer_wfd_tx_context_g |= (1 << egress_queue);

    return archer_wfd_hooks.queue_write(egress_queue, fkb_p);
}

static inline void archer_wlan_tx_notify(int egress_queue)
{
    archer_wfd_hooks.queue_notify(egress_queue);
}

static inline char *archer_wlan_tx_dev_name(int egress_port, int egress_queue)
{
    return archer_wfd_hooks.queue_dev_name(egress_queue);
}

static inline void archer_wlan_tx_flush(void)
{
    while(archer_wfd_tx_context_g)
    {
        int egress_queue = ffs(archer_wfd_tx_context_g) - 1;

        archer_wfd_tx_context_g &= ~(1 << egress_queue);

        archer_wlan_tx_notify(egress_queue);
    }
}

void archer_wfd_stats(void);
#define archer_wlan_stats archer_wfd_stats

int __init archer_wfd_construct(void);
#define archer_wlan_construct archer_wfd_construct

#endif  /* __ARCHER_WFD_H__ */
