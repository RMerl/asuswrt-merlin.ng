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
 * score object ioctl functions implementation file.
 * This ioctl file is generated automatically. Do not edit!
 */
#include "rdpa_api.h"
#include "rdpa_user.h"
#include "rdpa_user_int.h"
#include "rdpa_score_user_ioctl_ag.h"

static int rdpa_user_score_drv(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_score_drv\n");

	if (!(pa->drv = rdpa_score_drv()))
	{
		BDMF_TRACE_DBG("rdpa_score_drv failed\n");
	}

	return 0;
}

static int rdpa_user_score_get(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_score_drv\n");

	if ((pa->ret = rdpa_score_get(&pa->mo)))
	{
		BDMF_TRACE_DBG("rdpa_score_get failed ret: %d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_score_nstreams_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside score_user_nstreams_get\n");

	if ((pa->ret = rdpa_score_nstreams_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_nstreams_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_score_stream_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_score_stream_info_t  parm;

	BDMF_TRACE_DBG("inside score_user_stream_get\n");

	if ((pa->ret = rdpa_score_stream_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_stream_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ptr, (void *)&parm, sizeof(rdpa_score_stream_info_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_score_stream_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_score_stream_info_t  parm;

	BDMF_TRACE_DBG("inside score_user_stream_set\n");

	if (copy_from_user((void *)&parm, (void *)(bdmf_long_t)pa->ptr, sizeof(rdpa_score_stream_info_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_score_stream_set(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_stream_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_score_stream_add(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;
	rdpa_score_stream_info_t  parm;

	BDMF_TRACE_DBG("inside score_user_stream_add\n");

	if (copy_from_user((void *)&ai, (void *)(bdmf_long_t)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if (copy_from_user((void *)&parm, (void *)(bdmf_long_t)pa->ptr, sizeof(rdpa_score_stream_info_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_score_stream_add(pa->mo, &ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_stream_add failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_score_stream_delete(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside score_stream_delete\n");

	if ((pa->ret = rdpa_score_stream_delete(pa->mo, (bdmf_index)pa->ai)))
	{
		BDMF_TRACE_ERR("rdpa_score_stream_delete failed\n");
	}

	return 0;
}

static int rdpa_user_score_stream_get_next(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;

	BDMF_TRACE_DBG("inside score_user_stream_get_next\n");

	if (copy_from_user((void *)&ai, (void *)(bdmf_long_t)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_score_stream_get_next(pa->mo, &ai)))
	{
		BDMF_TRACE_DBG("rdpa_score_stream_get_next failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_score_stream_find(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;
	rdpa_score_stream_info_t  parm;

	BDMF_TRACE_DBG("inside score_user_stream_find\n");

	if (copy_from_user((void *)&ai, (void *)(bdmf_long_t)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if (copy_from_user((void *)&parm, (void *)(bdmf_long_t)pa->ptr, sizeof(rdpa_score_stream_info_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_score_stream_find(pa->mo, &ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_stream_find failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ptr, (void *)&parm, sizeof(rdpa_score_stream_info_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_score_nprofiles_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside score_user_nprofiles_get\n");

	if ((pa->ret = rdpa_score_nprofiles_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_nprofiles_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_score_profile_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_score_profile_cfg_t  parm;

	BDMF_TRACE_DBG("inside score_user_profile_get\n");

	if ((pa->ret = rdpa_score_profile_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_profile_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ptr, (void *)&parm, sizeof(rdpa_score_profile_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_score_profile_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_score_profile_cfg_t  parm;

	BDMF_TRACE_DBG("inside score_user_profile_set\n");

	if (copy_from_user((void *)&parm, (void *)(bdmf_long_t)pa->ptr, sizeof(rdpa_score_profile_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_score_profile_set(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_profile_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_score_profile_add(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;
	rdpa_score_profile_cfg_t  parm;

	BDMF_TRACE_DBG("inside score_user_profile_add\n");

	if (copy_from_user((void *)&ai, (void *)(bdmf_long_t)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if (copy_from_user((void *)&parm, (void *)(bdmf_long_t)pa->ptr, sizeof(rdpa_score_profile_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_score_profile_add(pa->mo, &ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_profile_add failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_score_profile_delete(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside score_profile_delete\n");

	if ((pa->ret = rdpa_score_profile_delete(pa->mo, (bdmf_index)pa->ai)))
	{
		BDMF_TRACE_ERR("rdpa_score_profile_delete failed\n");
	}

	return 0;
}

static int rdpa_user_score_profile_get_next(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;

	BDMF_TRACE_DBG("inside score_user_profile_get_next\n");

	if (copy_from_user((void *)&ai, (void *)(bdmf_long_t)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_score_profile_get_next(pa->mo, &ai)))
	{
		BDMF_TRACE_DBG("rdpa_score_profile_get_next failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_score_profile_find(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;
	rdpa_score_profile_cfg_t  parm;

	BDMF_TRACE_DBG("inside score_user_profile_find\n");

	if (copy_from_user((void *)&ai, (void *)(bdmf_long_t)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if (copy_from_user((void *)&parm, (void *)(bdmf_long_t)pa->ptr, sizeof(rdpa_score_profile_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_score_profile_find(pa->mo, &ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_profile_find failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ptr, (void *)&parm, sizeof(rdpa_score_profile_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_score_nsdrs_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside score_user_nsdrs_get\n");

	if ((pa->ret = rdpa_score_nsdrs_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_nsdrs_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_score_score_irq_coalescing_num_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside score_user_score_irq_coalescing_num_get\n");

	if ((pa->ret = rdpa_score_score_irq_coalescing_num_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_score_irq_coalescing_num_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_score_score_irq_coalescing_num_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside score_user_score_irq_coalescing_num_set\n");

	if ((pa->ret = rdpa_score_score_irq_coalescing_num_set(pa->mo, (bdmf_number)(bdmf_long_t)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_score_irq_coalescing_num_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_score_score_irq_coalescing_timeout_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside score_user_score_irq_coalescing_timeout_get\n");

	if ((pa->ret = rdpa_score_score_irq_coalescing_timeout_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_score_irq_coalescing_timeout_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_score_score_irq_coalescing_timeout_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside score_user_score_irq_coalescing_timeout_set\n");

	if ((pa->ret = rdpa_score_score_irq_coalescing_timeout_set(pa->mo, (bdmf_number)(bdmf_long_t)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_score_irq_coalescing_timeout_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_score_score_enable_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside score_user_score_enable_get\n");

	if ((pa->ret = rdpa_score_score_enable_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_score_enable_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(bdmf_long_t)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_score_score_enable_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside score_user_score_enable_set\n");

	if ((pa->ret = rdpa_score_score_enable_set(pa->mo, (bdmf_boolean)(bdmf_long_t)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_score_enable_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_score_qstat_cfg_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside score_user_qstat_cfg_set\n");

	if ((pa->ret = rdpa_score_qstat_cfg_set(pa->mo, (bdmf_number)(bdmf_long_t)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_score_qstat_cfg_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

bdmf_long_t rdpa_score_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa)
{
	int ret;

	switch (op){
		case RDPA_SCORE_DRV:
			ret = rdpa_user_score_drv(pa);
			break;

		case RDPA_SCORE_GET:
			ret = rdpa_user_score_get(pa);
			break;

		case RDPA_SCORE_NSTREAMS_GET:
			ret = rdpa_user_score_nstreams_get(pa);
			break;

		case RDPA_SCORE_STREAM_GET:
			ret = rdpa_user_score_stream_get(pa);
			break;

		case RDPA_SCORE_STREAM_SET:
			ret = rdpa_user_score_stream_set(pa);
			break;

		case RDPA_SCORE_STREAM_ADD:
			ret = rdpa_user_score_stream_add(pa);
			break;

		case RDPA_SCORE_STREAM_DELETE:
			ret = rdpa_user_score_stream_delete(pa);
			break;

		case RDPA_SCORE_STREAM_GET_NEXT:
			ret = rdpa_user_score_stream_get_next(pa);
			break;

		case RDPA_SCORE_STREAM_FIND:
			ret = rdpa_user_score_stream_find(pa);
			break;

		case RDPA_SCORE_NPROFILES_GET:
			ret = rdpa_user_score_nprofiles_get(pa);
			break;

		case RDPA_SCORE_PROFILE_GET:
			ret = rdpa_user_score_profile_get(pa);
			break;

		case RDPA_SCORE_PROFILE_SET:
			ret = rdpa_user_score_profile_set(pa);
			break;

		case RDPA_SCORE_PROFILE_ADD:
			ret = rdpa_user_score_profile_add(pa);
			break;

		case RDPA_SCORE_PROFILE_DELETE:
			ret = rdpa_user_score_profile_delete(pa);
			break;

		case RDPA_SCORE_PROFILE_GET_NEXT:
			ret = rdpa_user_score_profile_get_next(pa);
			break;

		case RDPA_SCORE_PROFILE_FIND:
			ret = rdpa_user_score_profile_find(pa);
			break;

		case RDPA_SCORE_NSDRS_GET:
			ret = rdpa_user_score_nsdrs_get(pa);
			break;

		case RDPA_SCORE_SCORE_IRQ_COALESCING_NUM_GET:
			ret = rdpa_user_score_score_irq_coalescing_num_get(pa);
			break;

		case RDPA_SCORE_SCORE_IRQ_COALESCING_NUM_SET:
			ret = rdpa_user_score_score_irq_coalescing_num_set(pa);
			break;

		case RDPA_SCORE_SCORE_IRQ_COALESCING_TIMEOUT_GET:
			ret = rdpa_user_score_score_irq_coalescing_timeout_get(pa);
			break;

		case RDPA_SCORE_SCORE_IRQ_COALESCING_TIMEOUT_SET:
			ret = rdpa_user_score_score_irq_coalescing_timeout_set(pa);
			break;

		case RDPA_SCORE_SCORE_ENABLE_GET:
			ret = rdpa_user_score_score_enable_get(pa);
			break;

		case RDPA_SCORE_SCORE_ENABLE_SET:
			ret = rdpa_user_score_score_enable_set(pa);
			break;

		case RDPA_SCORE_QSTAT_CFG_SET:
			ret = rdpa_user_score_qstat_cfg_set(pa);
			break;

		default:
			BDMF_TRACE_ERR("no such ioctl cmd: %u\n", op);
			ret = EINVAL;
		}

	return ret;
}
