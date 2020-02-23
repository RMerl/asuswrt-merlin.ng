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
#ifndef _RDPA_SYSTEM_USER_IOCTL_H_
#define _RDPA_SYSTEM_USER_IOCTL_H_

enum
{
	RDPA_SYSTEM_DRV,
	RDPA_SYSTEM_GET,
	RDPA_SYSTEM_INIT_CFG_GET,
	RDPA_SYSTEM_INIT_CFG_SET,
	RDPA_SYSTEM_CFG_GET,
	RDPA_SYSTEM_CFG_SET,
	RDPA_SYSTEM_SW_VERSION_GET,
	RDPA_SYSTEM_CLOCK_GATE_GET,
	RDPA_SYSTEM_CLOCK_GATE_SET,
	RDPA_SYSTEM_DROP_PRECEDENCE_GET,
	RDPA_SYSTEM_DROP_PRECEDENCE_SET,
	RDPA_SYSTEM_DROP_PRECEDENCE_GET_NEXT,
	RDPA_SYSTEM_TPID_DETECT_GET,
	RDPA_SYSTEM_TPID_DETECT_SET,
	RDPA_SYSTEM_TOD_GET,
	RDPA_SYSTEM_CPU_REASON_TO_TC_GET,
	RDPA_SYSTEM_CPU_REASON_TO_TC_SET,
	RDPA_SYSTEM_IPV4_HOST_ADDRESS_TABLE_GET,
	RDPA_SYSTEM_IPV4_HOST_ADDRESS_TABLE_ADD,
	RDPA_SYSTEM_IPV4_HOST_ADDRESS_TABLE_DELETE,
	RDPA_SYSTEM_IPV4_HOST_ADDRESS_TABLE_FIND,
	RDPA_SYSTEM_IPV6_HOST_ADDRESS_TABLE_GET,
	RDPA_SYSTEM_IPV6_HOST_ADDRESS_TABLE_ADD,
	RDPA_SYSTEM_IPV6_HOST_ADDRESS_TABLE_DELETE,
	RDPA_SYSTEM_IPV6_HOST_ADDRESS_TABLE_FIND,
	RDPA_SYSTEM_QM_CFG_GET,
	RDPA_SYSTEM_QM_CFG_SET,
	RDPA_SYSTEM_PACKET_BUFFER_CFG_GET,
	RDPA_SYSTEM_PACKET_BUFFER_CFG_SET,
	RDPA_SYSTEM_HIGH_PRIO_TC_THRESHOLD_GET,
	RDPA_SYSTEM_HIGH_PRIO_TC_THRESHOLD_SET,
	RDPA_SYSTEM_COUNTER_CFG_GET,
	RDPA_SYSTEM_COUNTER_CFG_SET,
	RDPA_SYSTEM_FPM_ISR_DELAY_TIMER_PERIOD_GET,
	RDPA_SYSTEM_FPM_ISR_DELAY_TIMER_PERIOD_SET,
	RDPA_SYSTEM_NATC_COUNTER_GET,
	RDPA_SYSTEM_NATC_COUNTER_SET,
	RDPA_SYSTEM_IH_CONG_THRESHOLD_GET,
	RDPA_SYSTEM_IH_CONG_THRESHOLD_SET,
	RDPA_SYSTEM_INGRESS_CONGESTION_CTRL_GET,
	RDPA_SYSTEM_INGRESS_CONGESTION_CTRL_SET,
	RDPA_SYSTEM_SYSTEM_RESOURCES_GET,
};

#endif /* _RDPA_SYSTEM_USER_IOCTL_H_ */
