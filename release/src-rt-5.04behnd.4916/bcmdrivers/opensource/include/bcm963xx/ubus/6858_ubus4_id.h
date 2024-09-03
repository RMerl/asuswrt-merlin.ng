/*
 * <:copyright-BRCM:2021:DUAL/GPL:standard
 * 
 *    Copyright (c) 2021 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 */

#ifndef __6858_UBUS4_ID_H__
#define __6858_UBUS4_ID_H__

#define UCB_NODE_ID_SLV_SYS     0
#define UCB_NODE_ID_MST_PCIE0   3
#define UCB_NODE_ID_SLV_PCIE0   4
#define UCB_NODE_ID_MST_PCIE2   5
#define UCB_NODE_ID_SLV_PCIE2   6
#define UCB_NODE_ID_MST_SATA    UCB_NODE_ID_MST_PCIE2
#define UCB_NODE_ID_SLV_SATA    UCB_NODE_ID_SLV_PCIE2
#define UCB_NODE_ID_MST_USB     14
#define UCB_NODE_ID_SLV_USB     15
#define UCB_NODE_ID_SLV_LPORT   19
#define UCB_NODE_ID_SLV_WAN     21

#define UBUS_MAX_PORT_NUM        35
#define UBUS_NUM_OF_MST_PORTS    19
#define UBUS_PORT_ID_LAST_SYSTOP 15

#define UBUS_PORT_ID_MEMC        1
#define UBUS_PORT_ID_BIU         2
#define UBUS_PORT_ID_PER         3
#define UBUS_PORT_ID_USB         4
#define UBUS_PORT_ID_PERDMA      5
#define UBUS_PORT_ID_SPU         6
#define UBUS_PORT_ID_PCIE0       8
#define UBUS_PORT_ID_PCIE2       9
#define UBUS_PORT_ID_PMC         15
#define UBUS_PORT_ID_XRDP_VPB    20
#define UBUS_PORT_ID_QM          22
#define UBUS_PORT_ID_DQM         23
#define UBUS_PORT_ID_DMA0        24
#define UBUS_PORT_ID_DMA1        25
#define UBUS_PORT_ID_NATC        26
#define UBUS_PORT_ID_TOP_BUFF    28
#define UBUS_PORT_ID_XRDP_BUFF   29
#define UBUS_PORT_ID_RQ0         32
#define UBUS_PORT_ID_RQ1         33
#define UBUS_PORT_ID_RQ2         34
#define UBUS_PORT_ID_RQ3         35

#endif
