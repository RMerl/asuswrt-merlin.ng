/***********************************************************************
 *
 *  Copyright (c) 2016  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2016:proprietary:standard

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
 *
 ************************************************************************/


#ifndef __BEEP_NETWORKING_H__
#define __BEEP_NETWORKING_H__


/*!\file beep_networking.h
 * \brief System level interface functions for iptables functionality for BEEP.
 *
 */

#define BEEP_NETWORKING_GROUP_PRIMARY	"BEEP_NETWORKING_GROUP_PRIMARY"
#define BEEP_NETWORKING_GROUP_SECONDARY	"BEEP_NETWORKING_GROUP_SECONDARY"
#define BEEP_NETWORKING_GROUP_WANONLY	"BEEP_NETWORKING_GROUP_WANONLY"
#define BEEP_NETWORKING_GROUP_LANONLY	"BEEP_NETWORKING_GROUP_LANONLY"

typedef enum {
   INTFGRP_BR_HOST_MODE   = 0,
   INTFGRP_BR_BEEP_PRIMARY_MODE,
   INTFGRP_BR_BEEP_SECONDARY_MODE,
   INTFGRP_BR_BEEP_WANONLY_MODE,
   INTFGRP_BR_BEEP_LANONLY_MODE
} IntfGrpBridgeMode;


/** Insert the networking firewall rules
 *
 * @param (IN) ifname
 * @param (IN) mode
 * @return void
 */
void rutIpt_beepNetworkingSecurity(const char *ifname, int mode);


/** Insert the NAT rules
 *
 * @param (IN) brifname
 * @param (IN) wanifname
 * @return void
 */
void rutIpt_beepNetworkingMasqueurade(const char *brifname, const char *wanifname);


/** setup port mapping rules
 *
 * @param (IN) ipaddr
 * @param (IN) ports with format "port/proto,..." "3333/udp,4444/tcp"
 * @param (IN) isAdd
 * @return void
 */
void rutIpt_beepNetworkingPortMapping(const char *ipaddr, const char *ports, int isAdd);

#endif /* __BEEP_NETWORKING_H__ */

