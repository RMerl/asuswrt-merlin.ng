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
#ifndef _RDPA_CPU_USER_IOCTL_H_
#define _RDPA_CPU_USER_IOCTL_H_

enum
{
	RDPA_CPU_DRV,
	RDPA_CPU_GET,
	RDPA_CPU_INDEX_GET,
	RDPA_CPU_INDEX_SET,
	RDPA_CPU_NUM_QUEUES_GET,
	RDPA_CPU_NUM_QUEUES_SET,
	RDPA_CPU_RXQ_FLUSH_SET,
	RDPA_CPU_RXQ_STAT_GET,
	RDPA_CPU_RXQ_STAT_SET,
	RDPA_CPU_METER_CFG_GET,
	RDPA_CPU_METER_CFG_SET,
	RDPA_CPU_METER_STAT_GET,
	RDPA_CPU_METER_STAT_SET,
	RDPA_CPU_REASON_CFG_GET,
	RDPA_CPU_REASON_CFG_SET,
	RDPA_CPU_REASON_CFG_GET_NEXT,
	RDPA_CPU_REASON_STAT_GET,
	RDPA_CPU_REASON_STAT_SET,
	RDPA_CPU_REASON_STAT_GET_NEXT,
	RDPA_CPU_INT_CONNECT_SET,
	RDPA_CPU_TC_TO_RXQ_GET,
	RDPA_CPU_TC_TO_RXQ_SET,
};

#endif /* _RDPA_CPU_USER_IOCTL_H_ */
