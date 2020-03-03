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
 * pbit_to_queue object ioctl functions implementation file.
 * This ioctl file is generated automatically. Do not edit!
 */
#include "rdpa_api.h"
#include "rdpa_user.h"
#include "rdpa_user_int.h"
#include "rdpa_pbit_to_queue_user_ioctl_ag.h"

static int rdpa_user_pbit_to_queue_drv(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_pbit_to_queue_drv\n");

	if (!(pa->drv = rdpa_pbit_to_queue_drv()))
	{
		BDMF_TRACE_DBG("rdpa_pbit_to_queue_drv failed\n");
	}

	return 0;
}

static int rdpa_user_pbit_to_queue_get(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_pbit_to_queue_drv\n");

	if ((pa->ret = rdpa_pbit_to_queue_get((bdmf_number)(long)pa->parm, &pa->mo)))
	{
		BDMF_TRACE_DBG("rdpa_pbit_to_queue_get failed ret: %d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_pbit_to_queue_table_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside pbit_to_queue_user_table_get\n");

	if ((pa->ret = rdpa_pbit_to_queue_table_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_pbit_to_queue_table_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_pbit_to_queue_table_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside pbit_to_queue_user_table_set\n");

	if ((pa->ret = rdpa_pbit_to_queue_table_set(pa->mo, (bdmf_number)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_pbit_to_queue_table_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_pbit_to_queue_pbit_map_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside pbit_to_queue_user_pbit_map_get\n");

	if ((pa->ret = rdpa_pbit_to_queue_pbit_map_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_pbit_to_queue_pbit_map_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_pbit_to_queue_pbit_map_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside pbit_to_queue_user_pbit_map_set\n");

	if ((pa->ret = rdpa_pbit_to_queue_pbit_map_set(pa->mo, pa->ai, (bdmf_number)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_pbit_to_queue_pbit_map_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

long rdpa_pbit_to_queue_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa)
{
	int ret;

	switch (op){
		case RDPA_PBIT_TO_QUEUE_DRV:
			ret = rdpa_user_pbit_to_queue_drv(pa);
			break;

		case RDPA_PBIT_TO_QUEUE_GET:
			ret = rdpa_user_pbit_to_queue_get(pa);
			break;

		case RDPA_PBIT_TO_QUEUE_TABLE_GET:
			ret = rdpa_user_pbit_to_queue_table_get(pa);
			break;

		case RDPA_PBIT_TO_QUEUE_TABLE_SET:
			ret = rdpa_user_pbit_to_queue_table_set(pa);
			break;

		case RDPA_PBIT_TO_QUEUE_PBIT_MAP_GET:
			ret = rdpa_user_pbit_to_queue_pbit_map_get(pa);
			break;

		case RDPA_PBIT_TO_QUEUE_PBIT_MAP_SET:
			ret = rdpa_user_pbit_to_queue_pbit_map_set(pa);
			break;

		default:
			BDMF_TRACE_ERR("no such ioctl cmd: %u\n", op);
			ret = EINVAL;
		}

	return ret;
}
