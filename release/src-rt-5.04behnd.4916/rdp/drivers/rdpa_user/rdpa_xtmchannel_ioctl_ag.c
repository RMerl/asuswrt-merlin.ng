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
 * xtmchannel object ioctl functions implementation file.
 * This ioctl file is generated automatically. Do not edit!
 */
#include "rdpa_api.h"
#include "rdpa_user.h"
#include "rdpa_user_int.h"
#include "rdpa_xtmchannel_user_ioctl_ag.h"

static int rdpa_user_xtmchannel_drv(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_xtmchannel_drv\n");

	if (!(pa->drv = rdpa_xtmchannel_drv()))
	{
		BDMF_TRACE_DBG("rdpa_xtmchannel_drv failed\n");
	}

	return 0;
}

static int rdpa_user_xtmchannel_get(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_xtmchannel_drv\n");

	if ((pa->ret = rdpa_xtmchannel_get((bdmf_number)(long)pa->parm, &pa->mo)))
	{
		BDMF_TRACE_DBG("rdpa_xtmchannel_get failed ret: %d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_xtmchannel_index_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside xtmchannel_user_index_get\n");

	if ((pa->ret = rdpa_xtmchannel_index_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_xtmchannel_index_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_xtmchannel_index_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside xtmchannel_user_index_set\n");

	if ((pa->ret = rdpa_xtmchannel_index_set(pa->mo, (bdmf_number)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_xtmchannel_index_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_xtmchannel_egress_tm_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_object_handle parm;

	BDMF_TRACE_DBG("inside xtmchannel_user_egress_tm_get\n");

	if ((pa->ret = rdpa_xtmchannel_egress_tm_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_xtmchannel_egress_tm_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_object_handle)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_xtmchannel_egress_tm_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside xtmchannel_user_egress_tm_set\n");

	if ((pa->ret = rdpa_xtmchannel_egress_tm_set(pa->mo, pa->object)))
	{
		BDMF_TRACE_DBG("rdpa_xtmchannel_egress_tm_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_xtmchannel_enable_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside xtmchannel_user_enable_get\n");

	if ((pa->ret = rdpa_xtmchannel_enable_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_xtmchannel_enable_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_xtmchannel_enable_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside xtmchannel_user_enable_set\n");

	if ((pa->ret = rdpa_xtmchannel_enable_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_xtmchannel_enable_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_xtmchannel_orl_prty_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_orl_prty parm;

	BDMF_TRACE_DBG("inside xtmchannel_user_orl_prty_get\n");

	if ((pa->ret = rdpa_xtmchannel_orl_prty_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_xtmchannel_orl_prty_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_tm_orl_prty)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_xtmchannel_orl_prty_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside xtmchannel_user_orl_prty_set\n");

	if ((pa->ret = rdpa_xtmchannel_orl_prty_set(pa->mo, (rdpa_tm_orl_prty)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_xtmchannel_orl_prty_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

long rdpa_xtmchannel_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa)
{
	int ret;

	switch (op){
		case RDPA_XTMCHANNEL_DRV:
			ret = rdpa_user_xtmchannel_drv(pa);
			break;

		case RDPA_XTMCHANNEL_GET:
			ret = rdpa_user_xtmchannel_get(pa);
			break;

		case RDPA_XTMCHANNEL_INDEX_GET:
			ret = rdpa_user_xtmchannel_index_get(pa);
			break;

		case RDPA_XTMCHANNEL_INDEX_SET:
			ret = rdpa_user_xtmchannel_index_set(pa);
			break;

		case RDPA_XTMCHANNEL_EGRESS_TM_GET:
			ret = rdpa_user_xtmchannel_egress_tm_get(pa);
			break;

		case RDPA_XTMCHANNEL_EGRESS_TM_SET:
			ret = rdpa_user_xtmchannel_egress_tm_set(pa);
			break;

		case RDPA_XTMCHANNEL_ENABLE_GET:
			ret = rdpa_user_xtmchannel_enable_get(pa);
			break;

		case RDPA_XTMCHANNEL_ENABLE_SET:
			ret = rdpa_user_xtmchannel_enable_set(pa);
			break;

		case RDPA_XTMCHANNEL_ORL_PRTY_GET:
			ret = rdpa_user_xtmchannel_orl_prty_get(pa);
			break;

		case RDPA_XTMCHANNEL_ORL_PRTY_SET:
			ret = rdpa_user_xtmchannel_orl_prty_set(pa);
			break;

		default:
			BDMF_TRACE_ERR("no such ioctl cmd: %u\n", op);
			ret = EINVAL;
		}

	return ret;
}
