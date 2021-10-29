/*
    Copyright 2000-2010 Broadcom Corporation
    
    <:label-BRCM:2011:DUAL/GPL:standard
    
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
