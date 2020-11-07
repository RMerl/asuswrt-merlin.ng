/*
<:copyright-BRCM:2019:proprietary:standard

   Copyright (c) 2019 Broadcom 
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
* File Name  : archer_wlan.h
*
* Description: Archer WLAN Interface Header File
*
*******************************************************************************
*/

#ifndef __ARCHER_WLAN_H__
#define __ARCHER_WLAN_H__

#if defined(CONFIG_BCM_ARCHER_WLAN)

int archer_wlan_tx_packet_int(int radio_index, FkBuff_t *fkb_p);

static inline int archer_wlan_tx_packet(int egress_port, int egress_queue, FkBuff_t *fkb_p)
{
    int radio_index = egress_port - SYSPORT_FLOW_WLAN_PORT_0;

    return archer_wlan_tx_packet_int(radio_index, fkb_p);
}

char *archer_wlan_tx_dev_name_int(int radio_index);

static inline char *archer_wlan_tx_dev_name(int egress_port, int egress_queue)
{
    int radio_index = egress_port - SYSPORT_FLOW_WLAN_PORT_0;

    return archer_wlan_tx_dev_name_int(radio_index);
}

#define archer_wlan_tx_flush()

void archer_wlan_stats(void);

int __init archer_wlan_construct(void);

#if defined(CC_AWL_FLCTL)
int archer_wlan_flctl_config_set (archer_wlflctl_config_t *cfg_p);
int archer_wlan_flctl_config_get (archer_wlflctl_config_t *cfg_p);
#endif /* CC_AWL_FLCTL */

#else /* CONFIG_BCM_ARCHER_WLAN */

#include "archer_wfd.h"

#endif /* CONFIG_BCM_ARCHER_WLAN */

#endif  /* __ARCHER_WLAN_H__ */
