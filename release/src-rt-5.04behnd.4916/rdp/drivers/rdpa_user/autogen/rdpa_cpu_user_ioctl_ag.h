// <:copyright-BRCM:2013:DUAL/GPL:standard
// 
//    Copyright (c) 2013 Broadcom 
//    All Rights Reserved
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2, as published by
// the Free Software Foundation (the "GPL").
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// 
// A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
// writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
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
