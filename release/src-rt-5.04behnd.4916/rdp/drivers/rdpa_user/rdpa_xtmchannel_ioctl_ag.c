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
