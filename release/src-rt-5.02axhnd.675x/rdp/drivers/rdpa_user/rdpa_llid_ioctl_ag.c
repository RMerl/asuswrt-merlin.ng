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
 * llid object ioctl functions implementation file.
 * This ioctl file is generated automatically. Do not edit!
 */
#include "rdpa_api.h"
#include "rdpa_user.h"
#include "rdpa_user_int.h"
#include "rdpa_llid_user_ioctl_ag.h"

static int rdpa_user_llid_drv(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_llid_drv\n");

	if (!(pa->drv = rdpa_llid_drv()))
	{
		BDMF_TRACE_DBG("rdpa_llid_drv failed\n");
	}

	return 0;
}

static int rdpa_user_llid_get(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_llid_drv\n");

	if ((pa->ret = rdpa_llid_get((bdmf_number)(long)pa->parm, &pa->mo)))
	{
		BDMF_TRACE_DBG("rdpa_llid_get failed ret: %d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_llid_index_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside llid_user_index_get\n");

	if ((pa->ret = rdpa_llid_index_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_index_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_llid_index_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside llid_user_index_set\n");

	if ((pa->ret = rdpa_llid_index_set(pa->mo, (bdmf_number)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_index_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_llid_egress_tm_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_object_handle parm;

	BDMF_TRACE_DBG("inside llid_user_egress_tm_get\n");

	if ((pa->ret = rdpa_llid_egress_tm_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_egress_tm_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_object_handle)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_llid_egress_tm_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside llid_user_egress_tm_set\n");

	if ((pa->ret = rdpa_llid_egress_tm_set(pa->mo, pa->object)))
	{
		BDMF_TRACE_DBG("rdpa_llid_egress_tm_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_llid_control_egress_tm_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_object_handle parm;

	BDMF_TRACE_DBG("inside llid_user_control_egress_tm_get\n");

	if ((pa->ret = rdpa_llid_control_egress_tm_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_control_egress_tm_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_object_handle)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_llid_control_egress_tm_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside llid_user_control_egress_tm_set\n");

	if ((pa->ret = rdpa_llid_control_egress_tm_set(pa->mo, pa->object)))
	{
		BDMF_TRACE_DBG("rdpa_llid_control_egress_tm_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_llid_control_enable_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside llid_user_control_enable_get\n");

	if ((pa->ret = rdpa_llid_control_enable_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_control_enable_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_llid_control_enable_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside llid_user_control_enable_set\n");

	if ((pa->ret = rdpa_llid_control_enable_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_control_enable_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_llid_data_enable_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside llid_user_data_enable_get\n");

	if ((pa->ret = rdpa_llid_data_enable_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_data_enable_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_llid_data_enable_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside llid_user_data_enable_set\n");

	if ((pa->ret = rdpa_llid_data_enable_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_data_enable_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_llid_ds_def_flow_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_ic_result_t   parm;

	BDMF_TRACE_DBG("inside llid_user_ds_def_flow_get\n");

	if ((pa->ret = rdpa_llid_ds_def_flow_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_ds_def_flow_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_ic_result_t  )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_llid_ds_def_flow_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_ic_result_t   parm;

	BDMF_TRACE_DBG("inside llid_user_ds_def_flow_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_ic_result_t  )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_llid_ds_def_flow_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_ds_def_flow_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_llid_port_action_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_llid_port_action_t  parm;

	BDMF_TRACE_DBG("inside llid_user_port_action_get\n");

	if ((pa->ret = rdpa_llid_port_action_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_port_action_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_llid_port_action_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_llid_port_action_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_llid_port_action_t  parm;

	BDMF_TRACE_DBG("inside llid_user_port_action_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_llid_port_action_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_llid_port_action_set(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_port_action_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_llid_fec_overhead_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside llid_user_fec_overhead_get\n");

	if ((pa->ret = rdpa_llid_fec_overhead_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_fec_overhead_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_llid_fec_overhead_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside llid_user_fec_overhead_set\n");

	if ((pa->ret = rdpa_llid_fec_overhead_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_fec_overhead_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_llid_sci_overhead_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside llid_user_sci_overhead_get\n");

	if ((pa->ret = rdpa_llid_sci_overhead_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_sci_overhead_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_llid_sci_overhead_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside llid_user_sci_overhead_set\n");

	if ((pa->ret = rdpa_llid_sci_overhead_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_sci_overhead_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_llid_q_802_1ae_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside llid_user_q_802_1ae_get\n");

	if ((pa->ret = rdpa_llid_q_802_1ae_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_q_802_1ae_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_llid_q_802_1ae_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside llid_user_q_802_1ae_set\n");

	if ((pa->ret = rdpa_llid_q_802_1ae_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_q_802_1ae_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_llid_is_empty_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside llid_user_is_empty_get\n");

	if ((pa->ret = rdpa_llid_is_empty_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_llid_is_empty_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

long rdpa_llid_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa)
{
	int ret;

	switch (op){
		case RDPA_LLID_DRV:
			ret = rdpa_user_llid_drv(pa);
			break;

		case RDPA_LLID_GET:
			ret = rdpa_user_llid_get(pa);
			break;

		case RDPA_LLID_INDEX_GET:
			ret = rdpa_user_llid_index_get(pa);
			break;

		case RDPA_LLID_INDEX_SET:
			ret = rdpa_user_llid_index_set(pa);
			break;

		case RDPA_LLID_EGRESS_TM_GET:
			ret = rdpa_user_llid_egress_tm_get(pa);
			break;

		case RDPA_LLID_EGRESS_TM_SET:
			ret = rdpa_user_llid_egress_tm_set(pa);
			break;

		case RDPA_LLID_CONTROL_EGRESS_TM_GET:
			ret = rdpa_user_llid_control_egress_tm_get(pa);
			break;

		case RDPA_LLID_CONTROL_EGRESS_TM_SET:
			ret = rdpa_user_llid_control_egress_tm_set(pa);
			break;

		case RDPA_LLID_CONTROL_ENABLE_GET:
			ret = rdpa_user_llid_control_enable_get(pa);
			break;

		case RDPA_LLID_CONTROL_ENABLE_SET:
			ret = rdpa_user_llid_control_enable_set(pa);
			break;

		case RDPA_LLID_DATA_ENABLE_GET:
			ret = rdpa_user_llid_data_enable_get(pa);
			break;

		case RDPA_LLID_DATA_ENABLE_SET:
			ret = rdpa_user_llid_data_enable_set(pa);
			break;

		case RDPA_LLID_DS_DEF_FLOW_GET:
			ret = rdpa_user_llid_ds_def_flow_get(pa);
			break;

		case RDPA_LLID_DS_DEF_FLOW_SET:
			ret = rdpa_user_llid_ds_def_flow_set(pa);
			break;

		case RDPA_LLID_PORT_ACTION_GET:
			ret = rdpa_user_llid_port_action_get(pa);
			break;

		case RDPA_LLID_PORT_ACTION_SET:
			ret = rdpa_user_llid_port_action_set(pa);
			break;

		case RDPA_LLID_FEC_OVERHEAD_GET:
			ret = rdpa_user_llid_fec_overhead_get(pa);
			break;

		case RDPA_LLID_FEC_OVERHEAD_SET:
			ret = rdpa_user_llid_fec_overhead_set(pa);
			break;

		case RDPA_LLID_SCI_OVERHEAD_GET:
			ret = rdpa_user_llid_sci_overhead_get(pa);
			break;

		case RDPA_LLID_SCI_OVERHEAD_SET:
			ret = rdpa_user_llid_sci_overhead_set(pa);
			break;

		case RDPA_LLID_Q_802_1AE_GET:
			ret = rdpa_user_llid_q_802_1ae_get(pa);
			break;

		case RDPA_LLID_Q_802_1AE_SET:
			ret = rdpa_user_llid_q_802_1ae_set(pa);
			break;

		case RDPA_LLID_IS_EMPTY_GET:
			ret = rdpa_user_llid_is_empty_get(pa);
			break;

		default:
			BDMF_TRACE_ERR("no such ioctl cmd: %u\n", op);
			ret = EINVAL;
		}

	return ret;
}
