// <:copyright-BRCM:2013:DUAL/GPL:standard
// 
//    Copyright (c) 2013 Broadcom 
//    All Rights Reserved
// 
// Unless you and Broadcom execute a separate written software license
// agreement governing use of this software, this software is licensed
// to you under the terms of the GNU General Public License version 2
// (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
// with the following added to such license:
// 
//    As a special exception, the copyright holders of this software give
//    you permission to link this software with independent modules, and
//    to copy and distribute the resulting executable under terms of your
//    choice, provided that you also meet, for each linked independent
//    module, the terms and conditions of the license of that module.
//    An independent module is a module which is not derived from this
//    software.  The special exception does not apply to any modifications
//    of the software.
// 
// Not withstanding the above, under no circumstances may you combine
// this software in any way with any other Broadcom software provided
// under a license other than the GPL, without Broadcom's express prior
// written consent.
// 
// :>
/*
 * rdpa user ioctl define header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_PORT_USER_IOCTL_H_
#define _RDPA_PORT_USER_IOCTL_H_

enum
{
	RDPA_PORT_DRV,
	RDPA_PORT_GET,
	RDPA_PORT_INDEX_GET,
	RDPA_PORT_INDEX_SET,
	RDPA_PORT_WAN_TYPE_GET,
	RDPA_PORT_WAN_TYPE_SET,
	RDPA_PORT_SPEED_GET,
	RDPA_PORT_SPEED_SET,
	RDPA_PORT_CFG_GET,
	RDPA_PORT_CFG_SET,
	RDPA_PORT_TM_CFG_GET,
	RDPA_PORT_TM_CFG_SET,
	RDPA_PORT_SA_LIMIT_GET,
	RDPA_PORT_SA_LIMIT_SET,
	RDPA_PORT_DEF_FLOW_GET,
	RDPA_PORT_DEF_FLOW_SET,
	RDPA_PORT_STAT_GET,
	RDPA_PORT_STAT_SET,
	RDPA_PORT_FLOW_CONTROL_GET,
	RDPA_PORT_FLOW_CONTROL_SET,
	RDPA_PORT_INGRESS_RATE_LIMIT_GET,
	RDPA_PORT_INGRESS_RATE_LIMIT_SET,
	RDPA_PORT_MIRROR_CFG_GET,
	RDPA_PORT_MIRROR_CFG_SET,
	RDPA_PORT_VLAN_ISOLATION_GET,
	RDPA_PORT_VLAN_ISOLATION_SET,
	RDPA_PORT_TRANSPARENT_GET,
	RDPA_PORT_TRANSPARENT_SET,
	RDPA_PORT_LOOPBACK_GET,
	RDPA_PORT_LOOPBACK_SET,
	RDPA_PORT_MTU_SIZE_GET,
	RDPA_PORT_MTU_SIZE_SET,
	RDPA_PORT_CPU_OBJ_GET,
	RDPA_PORT_CPU_OBJ_SET,
	RDPA_PORT_CPU_METER_GET,
	RDPA_PORT_CPU_METER_SET,
	RDPA_PORT_INGRESS_FILTER_GET,
	RDPA_PORT_INGRESS_FILTER_SET,
	RDPA_PORT_INGRESS_FILTER_GET_NEXT,
	RDPA_PORT_PROTOCOL_FILTERS_GET,
	RDPA_PORT_PROTOCOL_FILTERS_SET,
	RDPA_PORT_ENABLE_GET,
	RDPA_PORT_ENABLE_SET,
	RDPA_PORT_IS_EMPTY_GET,
	RDPA_PORT_UNINIT_SET,
	RDPA_PORT_MAC_GET,
	RDPA_PORT_MAC_SET,
	RDPA_PORT_PKT_SIZE_STAT_EN_GET,
	RDPA_PORT_PKT_SIZE_STAT_EN_SET,
	RDPA_PORT_PBIT_TO_DP_MAP_GET,
	RDPA_PORT_PBIT_TO_DP_MAP_SET,
	RDPA_PORT_OPTIONS_GET,
	RDPA_PORT_OPTIONS_SET,
	RDPA_PORT_PFC_TX_ENABLE_GET,
	RDPA_PORT_PFC_TX_ENABLE_SET,
	RDPA_PORT_PFC_TX_TIMER_GET,
	RDPA_PORT_PFC_TX_TIMER_SET,
};

#endif /* _RDPA_PORT_USER_IOCTL_H_ */
