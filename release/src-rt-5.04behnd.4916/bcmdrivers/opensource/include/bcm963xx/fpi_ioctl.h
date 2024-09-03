#ifndef __FPI_IOCTL_H_INCLUDED__
#define __FPI_IOCTL_H_INCLUDED__
/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom 
   All Rights Reserved

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

/*
 *******************************************************************************
 * File Name : fpi_ioctl.h
 *
 *******************************************************************************
 */

#include "fpi_defs.h"

#define FPI_DEV_NAME		"fpi"
#define FPI_NUM_DEVICES		1
#define FPI_DRV_DEV_NAME	"/dev/" FPI_DEV_NAME

/*
 * Ioctl definitions.
 */
typedef enum {
/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
 * processor. Hence start all IOCTL values from 100 to prevent conflicts
 */
	fpictl_ioctl_sys	= 100,
	fpictl_ioctl_max
} fpictl_ioctl_t;

typedef enum {
	fpictl_subsys_mode,
	fpictl_subsys_def_prio,
	fpictl_subsys_flow,
	fpictl_subsys_stat,
	fpictl_subsys_apmac,
	fpictl_subsys_gre_mode,
	fpictl_subsys_l2lkp_on_etype,
	fpictl_subsys_lkp_enable,
	fpictl_subsys_max
} fpictl_subsys_t;

typedef enum {
	fpictl_op_set,
	fpictl_op_get,
	fpictl_op_add,
	fpictl_op_del_by_handle,
	fpictl_op_del_by_key,
	fpictl_op_getnext,
	fpictl_op_max
} fpictl_op_t;

typedef struct {
	fpictl_subsys_t subsys;
	fpictl_op_t op;
	int rc;
	fpi_mode_t mode;
	uint8_t def_prio;
	fpi_flow_t flow;
	fpi_stat_t stat;
	fpi_wlan_egress_info_t wl_info;
	fpi_gre_mode_t gre_mode;
	uint8_t enable;
	uint32_t handle;
	int next_idx;
} fpictl_data_t;

#endif /* __FPI_IOCTL_H_INCLUDED__ */
