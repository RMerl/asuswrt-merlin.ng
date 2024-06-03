#ifndef __FPI_IOCTL_H_INCLUDED__
#define __FPI_IOCTL_H_INCLUDED__
/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom
   All Rights Reserved

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
	fpictl_subsys_flow,
	fpictl_subsys_stat,
	fpictl_subsys_apmac,
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
	fpi_flow_t flow;
	fpi_stat_t stat;
	uint32_t handle;
	int next_idx;
} fpictl_data_t;

#endif /* __FPI_IOCTL_H_INCLUDED__ */
