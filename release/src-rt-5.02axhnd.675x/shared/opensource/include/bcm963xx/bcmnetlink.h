/*
    Copyright 2000-2010 Broadcom Corporation
    
    <:label-BRCM:2011:DUAL/GPL:standard
    
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

/**************************************************************************
 * File Name  : bcmnetlink.h
 *
 * Description: This file defines broadcom specific netlink message types
 ***************************************************************************/
#ifndef _BCMNETLINK_H
#define _BCMNETLINK_H

#include<linux/netlink.h>

#ifndef NETLINK_BRCM_MONITOR
#define NETLINK_BRCM_MONITOR 25
#endif

#ifndef NETLINK_BRCM_EPON
#define NETLINK_BRCM_EPON 26
#endif

/* message types exchanged using NETLINK_BRCM_MONITOR */
#define MSG_NETLINK_BRCM_WAKEUP_MONITOR_TASK 0X1000

#define MSG_NETLINK_BRCM_LINK_STATUS_CHANGED 0X2000

#define MSG_NETLINK_BRCM_LINK_TRAFFIC_TYPE_MISMATCH 0X4000


/* For the 63138 and 63148, implement a workaround to strip bytes and
   allow OAM traffic due to JIRA HW63138-12 */
#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
#define MSG_NETLINK_BRCM_LINK_OAM_STRIP_BYTE 0X5000
#endif
#define MSG_NETLINK_BRCM_XTM_BNDGRP_ID_MISMATCH 0x6000


#define MSG_NETLINK_BRCM_SAVE_DSL_CFG		0X8000	/* Why the previous defined values are like bitmap???, just follow */
#define MSG_ID_BRCM_SAVE_DSL_CFG_ALL		0
#define MSG_ID_BRCM_SAVE_DSL_PREFERRED_LINE	1

#define MSG_NETLINK_BRCM_CALLBACK_DSL_DRV	0X0100

extern void kerSysSendtoMonitorTask(int msgType, char *msgData, int msgDataLen);

#endif /*_BCMNETLINK_H */
