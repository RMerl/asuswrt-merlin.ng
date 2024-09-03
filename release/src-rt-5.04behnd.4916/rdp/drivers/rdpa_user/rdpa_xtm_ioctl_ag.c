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
 * xtm object ioctl functions implementation file.
 * This ioctl file is generated automatically. Do not edit!
 */
#include "rdpa_api.h"
#include "rdpa_user.h"
#include "rdpa_user_int.h"
#include "rdpa_xtm_user_ioctl_ag.h"

static int rdpa_user_xtm_drv(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_xtm_drv\n");

	if (!(pa->drv = rdpa_xtm_drv()))
	{
		BDMF_TRACE_DBG("rdpa_xtm_drv failed\n");
	}

	return 0;
}

static int rdpa_user_xtm_get(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_xtm_drv\n");

	if ((pa->ret = rdpa_xtm_get(&pa->mo)))
	{
		BDMF_TRACE_DBG("rdpa_xtm_get failed ret: %d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_xtm_index_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside xtm_user_index_get\n");

	if ((pa->ret = rdpa_xtm_index_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_xtm_index_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

long rdpa_xtm_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa)
{
	int ret;

	switch (op){
		case RDPA_XTM_DRV:
			ret = rdpa_user_xtm_drv(pa);
			break;

		case RDPA_XTM_GET:
			ret = rdpa_user_xtm_get(pa);
			break;

		case RDPA_XTM_INDEX_GET:
			ret = rdpa_user_xtm_index_get(pa);
			break;

		default:
			BDMF_TRACE_ERR("no such ioctl cmd: %u\n", op);
			ret = EINVAL;
		}

	return ret;
}
