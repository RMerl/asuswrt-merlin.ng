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
 * ip_class object ioctl functions implementation file.
 * This ioctl file is generated automatically. Do not edit!
 */
#include "rdpa_api.h"
#include "rdpa_user.h"
#include "rdpa_user_int.h"
#include "rdpa_ip_class_user_ioctl_ag.h"

static int rdpa_user_ip_class_drv(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_ip_class_drv\n");

	if (!(pa->drv = rdpa_ip_class_drv()))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_drv failed\n");
	}

	return 0;
}

static int rdpa_user_ip_class_get(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_ip_class_drv\n");

	if ((pa->ret = rdpa_ip_class_get(&pa->mo)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_get failed ret: %d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_ip_class_nflows_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside ip_class_user_nflows_get\n");

	if ((pa->ret = rdpa_ip_class_nflows_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_nflows_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ip_class_flow_stat_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_stat_t  parm;

	BDMF_TRACE_DBG("inside ip_class_user_flow_stat_get\n");

	if ((pa->ret = rdpa_ip_class_flow_stat_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_flow_stat_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_stat_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ip_class_flow_stat_get_next(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;

	BDMF_TRACE_DBG("inside ip_class_user_flow_stat_get_next\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_ip_class_flow_stat_get_next(pa->mo, &ai)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_flow_stat_get_next failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ip_class_flush_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside ip_class_user_flush_set\n");

	if ((pa->ret = rdpa_ip_class_flush_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_flush_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_ip_class_l4_filter_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_l4_filter_cfg_t  parm;

	BDMF_TRACE_DBG("inside ip_class_user_l4_filter_get\n");

	if ((pa->ret = rdpa_ip_class_l4_filter_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_l4_filter_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_l4_filter_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ip_class_l4_filter_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_l4_filter_cfg_t  parm;

	BDMF_TRACE_DBG("inside ip_class_user_l4_filter_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_l4_filter_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_ip_class_l4_filter_set(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_l4_filter_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_ip_class_l4_filter_stat_get(rdpa_ioctl_cmd_t *pa)
{
	uint32_t parm;

	BDMF_TRACE_DBG("inside ip_class_user_l4_filter_stat_get\n");

	if ((pa->ret = rdpa_ip_class_l4_filter_stat_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_l4_filter_stat_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(uint32_t)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ip_class_key_type_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_key_type parm;

	BDMF_TRACE_DBG("inside ip_class_user_key_type_get\n");

	if ((pa->ret = rdpa_ip_class_key_type_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_key_type_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_key_type)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ip_class_key_type_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside ip_class_user_key_type_set\n");

	if ((pa->ret = rdpa_ip_class_key_type_set(pa->mo, (rdpa_key_type)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_key_type_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_ip_class_pathstat_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_stat_t  parm;

	BDMF_TRACE_DBG("inside ip_class_user_pathstat_get\n");

	if ((pa->ret = rdpa_ip_class_pathstat_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_pathstat_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_stat_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ip_class_tcp_ack_prio_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside ip_class_user_tcp_ack_prio_get\n");

	if ((pa->ret = rdpa_ip_class_tcp_ack_prio_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_tcp_ack_prio_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ip_class_tcp_ack_prio_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside ip_class_user_tcp_ack_prio_set\n");

	if ((pa->ret = rdpa_ip_class_tcp_ack_prio_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_tcp_ack_prio_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_ip_class_tos_mflows_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside ip_class_user_tos_mflows_get\n");

	if ((pa->ret = rdpa_ip_class_tos_mflows_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_tos_mflows_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ip_class_tos_mflows_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside ip_class_user_tos_mflows_set\n");

	if ((pa->ret = rdpa_ip_class_tos_mflows_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_tos_mflows_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_ip_class_pbit_key_mode_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside ip_class_user_pbit_key_mode_get\n");

	if ((pa->ret = rdpa_ip_class_pbit_key_mode_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_pbit_key_mode_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ip_class_pbit_key_mode_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside ip_class_user_pbit_key_mode_set\n");

	if ((pa->ret = rdpa_ip_class_pbit_key_mode_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_ip_class_pbit_key_mode_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

long rdpa_ip_class_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa)
{
	int ret;

	switch (op){
		case RDPA_IP_CLASS_DRV:
			ret = rdpa_user_ip_class_drv(pa);
			break;

		case RDPA_IP_CLASS_GET:
			ret = rdpa_user_ip_class_get(pa);
			break;

		case RDPA_IP_CLASS_NFLOWS_GET:
			ret = rdpa_user_ip_class_nflows_get(pa);
			break;

		case RDPA_IP_CLASS_FLOW_STAT_GET:
			ret = rdpa_user_ip_class_flow_stat_get(pa);
			break;

		case RDPA_IP_CLASS_FLOW_STAT_GET_NEXT:
			ret = rdpa_user_ip_class_flow_stat_get_next(pa);
			break;

		case RDPA_IP_CLASS_FLUSH_SET:
			ret = rdpa_user_ip_class_flush_set(pa);
			break;

		case RDPA_IP_CLASS_L4_FILTER_GET:
			ret = rdpa_user_ip_class_l4_filter_get(pa);
			break;

		case RDPA_IP_CLASS_L4_FILTER_SET:
			ret = rdpa_user_ip_class_l4_filter_set(pa);
			break;

		case RDPA_IP_CLASS_L4_FILTER_STAT_GET:
			ret = rdpa_user_ip_class_l4_filter_stat_get(pa);
			break;

		case RDPA_IP_CLASS_KEY_TYPE_GET:
			ret = rdpa_user_ip_class_key_type_get(pa);
			break;

		case RDPA_IP_CLASS_KEY_TYPE_SET:
			ret = rdpa_user_ip_class_key_type_set(pa);
			break;

		case RDPA_IP_CLASS_PATHSTAT_GET:
			ret = rdpa_user_ip_class_pathstat_get(pa);
			break;

		case RDPA_IP_CLASS_TCP_ACK_PRIO_GET:
			ret = rdpa_user_ip_class_tcp_ack_prio_get(pa);
			break;

		case RDPA_IP_CLASS_TCP_ACK_PRIO_SET:
			ret = rdpa_user_ip_class_tcp_ack_prio_set(pa);
			break;

		case RDPA_IP_CLASS_TOS_MFLOWS_GET:
			ret = rdpa_user_ip_class_tos_mflows_get(pa);
			break;

		case RDPA_IP_CLASS_TOS_MFLOWS_SET:
			ret = rdpa_user_ip_class_tos_mflows_set(pa);
			break;

		case RDPA_IP_CLASS_PBIT_KEY_MODE_GET:
			ret = rdpa_user_ip_class_pbit_key_mode_get(pa);
			break;

		case RDPA_IP_CLASS_PBIT_KEY_MODE_SET:
			ret = rdpa_user_ip_class_pbit_key_mode_set(pa);
			break;

		default:
			BDMF_TRACE_ERR("no such ioctl cmd: %u\n", op);
			ret = EINVAL;
		}

	return ret;
}
