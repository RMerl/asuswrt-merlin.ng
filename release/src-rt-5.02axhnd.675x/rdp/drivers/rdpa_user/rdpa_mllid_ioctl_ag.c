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
 * mllid object ioctl functions implementation file.
 * This ioctl file is generated automatically. Do not edit!
 */
#include "rdpa_api.h"
#include "rdpa_user.h"
#include "rdpa_user_int.h"
#include "rdpa_mllid_user_ioctl_ag.h"

static int rdpa_user_mllid_drv(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_mllid_drv\n");

	if (!(pa->drv = rdpa_mllid_drv()))
	{
		BDMF_TRACE_DBG("rdpa_mllid_drv failed\n");
	}

	return 0;
}

static int rdpa_user_mllid_get(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_mllid_drv\n");

	if ((pa->ret = rdpa_mllid_get((bdmf_number)(long)pa->parm, &pa->mo)))
	{
		BDMF_TRACE_DBG("rdpa_mllid_get failed ret: %d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_mllid_index_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside mllid_user_index_get\n");

	if ((pa->ret = rdpa_mllid_index_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_mllid_index_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_mllid_index_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside mllid_user_index_set\n");

	if ((pa->ret = rdpa_mllid_index_set(pa->mo, (bdmf_number)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_mllid_index_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_mllid_flow_id_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside mllid_user_flow_id_get\n");

	if ((pa->ret = rdpa_mllid_flow_id_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_mllid_flow_id_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_mllid_flow_id_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside mllid_user_flow_id_set\n");

	if ((pa->ret = rdpa_mllid_flow_id_set(pa->mo, (bdmf_number)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_mllid_flow_id_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_mllid_enable_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside mllid_user_enable_get\n");

	if ((pa->ret = rdpa_mllid_enable_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_mllid_enable_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_mllid_enable_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside mllid_user_enable_set\n");

	if ((pa->ret = rdpa_mllid_enable_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_mllid_enable_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_mllid_stat_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_mllid_stat_t  parm;

	BDMF_TRACE_DBG("inside mllid_user_stat_get\n");

	if ((pa->ret = rdpa_mllid_stat_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_mllid_stat_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_mllid_stat_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_mllid_stat_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_mllid_stat_t  parm;

	BDMF_TRACE_DBG("inside mllid_user_stat_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_mllid_stat_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_mllid_stat_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_mllid_stat_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

long rdpa_mllid_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa)
{
	int ret;

	switch (op){
		case RDPA_MLLID_DRV:
			ret = rdpa_user_mllid_drv(pa);
			break;

		case RDPA_MLLID_GET:
			ret = rdpa_user_mllid_get(pa);
			break;

		case RDPA_MLLID_INDEX_GET:
			ret = rdpa_user_mllid_index_get(pa);
			break;

		case RDPA_MLLID_INDEX_SET:
			ret = rdpa_user_mllid_index_set(pa);
			break;

		case RDPA_MLLID_FLOW_ID_GET:
			ret = rdpa_user_mllid_flow_id_get(pa);
			break;

		case RDPA_MLLID_FLOW_ID_SET:
			ret = rdpa_user_mllid_flow_id_set(pa);
			break;

		case RDPA_MLLID_ENABLE_GET:
			ret = rdpa_user_mllid_enable_get(pa);
			break;

		case RDPA_MLLID_ENABLE_SET:
			ret = rdpa_user_mllid_enable_set(pa);
			break;

		case RDPA_MLLID_STAT_GET:
			ret = rdpa_user_mllid_stat_get(pa);
			break;

		case RDPA_MLLID_STAT_SET:
			ret = rdpa_user_mllid_stat_set(pa);
			break;

		default:
			BDMF_TRACE_ERR("no such ioctl cmd: %u\n", op);
			ret = EINVAL;
		}

	return ret;
}
