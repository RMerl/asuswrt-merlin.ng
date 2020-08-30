/*
 * port object ioctl functions implementation file.
 * This ioctl file is generated automatically. Do not edit!
 */
#include "rdpa_api.h"
#include "rdpa_user.h"
#include "rdpa_user_int.h"
#include "rdpa_port_user_ioctl_ag.h"

static int rdpa_user_port_drv(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_port_drv\n");

	if (!(pa->drv = rdpa_port_drv()))
	{
		BDMF_TRACE_DBG("rdpa_port_drv failed\n");
	}

	return 0;
}

static int rdpa_user_port_get(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_port_drv\n");

	if ((pa->ret = rdpa_port_get((rdpa_if)(long)pa->parm, &pa->mo)))
	{
		BDMF_TRACE_DBG("rdpa_port_get failed ret: %d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_index_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_if parm;

	BDMF_TRACE_DBG("inside port_user_index_get\n");

	if ((pa->ret = rdpa_port_index_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_index_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_if)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_index_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside port_user_index_set\n");

	if ((pa->ret = rdpa_port_index_set(pa->mo, (rdpa_if)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_index_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_wan_type_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_wan_type parm;

	BDMF_TRACE_DBG("inside port_user_wan_type_get\n");

	if ((pa->ret = rdpa_port_wan_type_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_wan_type_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_wan_type)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_wan_type_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside port_user_wan_type_set\n");

	if ((pa->ret = rdpa_port_wan_type_set(pa->mo, (rdpa_wan_type)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_wan_type_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_speed_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_speed_type parm;

	BDMF_TRACE_DBG("inside port_user_speed_get\n");

	if ((pa->ret = rdpa_port_speed_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_speed_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_speed_type)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_speed_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside port_user_speed_set\n");

	if ((pa->ret = rdpa_port_speed_set(pa->mo, (rdpa_speed_type)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_speed_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_cfg_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_dp_cfg_t  parm;

	BDMF_TRACE_DBG("inside port_user_cfg_get\n");

	if ((pa->ret = rdpa_port_cfg_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_cfg_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_port_dp_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_cfg_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_dp_cfg_t  parm;

	BDMF_TRACE_DBG("inside port_user_cfg_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_port_dp_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_port_cfg_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_cfg_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_tm_cfg_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_tm_cfg_t  parm;

	BDMF_TRACE_DBG("inside port_user_tm_cfg_get\n");

	if ((pa->ret = rdpa_port_tm_cfg_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_tm_cfg_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_port_tm_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_tm_cfg_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_tm_cfg_t  parm;

	BDMF_TRACE_DBG("inside port_user_tm_cfg_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_port_tm_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_port_tm_cfg_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_tm_cfg_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_sa_limit_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_sa_limit_t  parm;

	BDMF_TRACE_DBG("inside port_user_sa_limit_get\n");

	if ((pa->ret = rdpa_port_sa_limit_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_sa_limit_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_port_sa_limit_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_sa_limit_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_sa_limit_t  parm;

	BDMF_TRACE_DBG("inside port_user_sa_limit_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_port_sa_limit_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_port_sa_limit_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_sa_limit_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_def_flow_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_ic_result_t   parm;

	BDMF_TRACE_DBG("inside port_user_def_flow_get\n");

	if ((pa->ret = rdpa_port_def_flow_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_def_flow_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_ic_result_t  )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_def_flow_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_ic_result_t   parm;

	BDMF_TRACE_DBG("inside port_user_def_flow_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_ic_result_t  )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_port_def_flow_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_def_flow_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_stat_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_stat_t  parm;

	BDMF_TRACE_DBG("inside port_user_stat_get\n");

	if ((pa->ret = rdpa_port_stat_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_stat_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_port_stat_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_stat_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_stat_t  parm;

	BDMF_TRACE_DBG("inside port_user_stat_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_port_stat_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_port_stat_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_stat_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_flow_control_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_flow_ctrl_t  parm;

	BDMF_TRACE_DBG("inside port_user_flow_control_get\n");

	if ((pa->ret = rdpa_port_flow_control_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_flow_control_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_port_flow_ctrl_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_flow_control_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_flow_ctrl_t  parm;

	BDMF_TRACE_DBG("inside port_user_flow_control_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_port_flow_ctrl_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_port_flow_control_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_flow_control_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_ingress_rate_limit_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_ingress_rate_limit_t  parm;

	BDMF_TRACE_DBG("inside port_user_ingress_rate_limit_get\n");

	if ((pa->ret = rdpa_port_ingress_rate_limit_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_ingress_rate_limit_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_port_ingress_rate_limit_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_ingress_rate_limit_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_ingress_rate_limit_t  parm;

	BDMF_TRACE_DBG("inside port_user_ingress_rate_limit_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_port_ingress_rate_limit_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_port_ingress_rate_limit_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_ingress_rate_limit_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_mirror_cfg_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_mirror_cfg_t  parm;

	BDMF_TRACE_DBG("inside port_user_mirror_cfg_get\n");

	if ((pa->ret = rdpa_port_mirror_cfg_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_mirror_cfg_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_port_mirror_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_mirror_cfg_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_mirror_cfg_t  parm;

	BDMF_TRACE_DBG("inside port_user_mirror_cfg_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_port_mirror_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_port_mirror_cfg_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_mirror_cfg_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_vlan_isolation_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_vlan_isolation_t  parm;

	BDMF_TRACE_DBG("inside port_user_vlan_isolation_get\n");

	if ((pa->ret = rdpa_port_vlan_isolation_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_vlan_isolation_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_port_vlan_isolation_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_vlan_isolation_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_vlan_isolation_t  parm;

	BDMF_TRACE_DBG("inside port_user_vlan_isolation_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_port_vlan_isolation_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_port_vlan_isolation_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_vlan_isolation_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_transparent_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside port_user_transparent_get\n");

	if ((pa->ret = rdpa_port_transparent_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_transparent_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_transparent_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside port_user_transparent_set\n");

	if ((pa->ret = rdpa_port_transparent_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_transparent_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_loopback_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_loopback_t  parm;

	BDMF_TRACE_DBG("inside port_user_loopback_get\n");

	if ((pa->ret = rdpa_port_loopback_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_loopback_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_port_loopback_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_loopback_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_loopback_t  parm;

	BDMF_TRACE_DBG("inside port_user_loopback_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_port_loopback_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_port_loopback_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_loopback_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_mtu_size_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside port_user_mtu_size_get\n");

	if ((pa->ret = rdpa_port_mtu_size_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_mtu_size_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_mtu_size_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside port_user_mtu_size_set\n");

	if ((pa->ret = rdpa_port_mtu_size_set(pa->mo, (bdmf_number)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_mtu_size_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_cpu_obj_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_object_handle parm;

	BDMF_TRACE_DBG("inside port_user_cpu_obj_get\n");

	if ((pa->ret = rdpa_port_cpu_obj_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_cpu_obj_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_object_handle)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_cpu_obj_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside port_user_cpu_obj_set\n");

	if ((pa->ret = rdpa_port_cpu_obj_set(pa->mo, pa->object)))
	{
		BDMF_TRACE_DBG("rdpa_port_cpu_obj_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_cpu_meter_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside port_user_cpu_meter_get\n");

	if ((pa->ret = rdpa_port_cpu_meter_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_cpu_meter_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_cpu_meter_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside port_user_cpu_meter_set\n");

	if ((pa->ret = rdpa_port_cpu_meter_set(pa->mo, (bdmf_number)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_cpu_meter_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_ingress_filter_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_filter_ctrl_t  parm;

	BDMF_TRACE_DBG("inside port_user_ingress_filter_get\n");

	if ((pa->ret = rdpa_port_ingress_filter_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_ingress_filter_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_filter_ctrl_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_ingress_filter_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_filter_ctrl_t  parm;

	BDMF_TRACE_DBG("inside port_user_ingress_filter_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_filter_ctrl_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_port_ingress_filter_set(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_ingress_filter_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_ingress_filter_get_next(rdpa_ioctl_cmd_t *pa)
{
	rdpa_filter  ai;

	BDMF_TRACE_DBG("inside port_user_ingress_filter_get_next\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(rdpa_filter )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_port_ingress_filter_get_next(pa->mo, &ai)))
	{
		BDMF_TRACE_DBG("rdpa_port_ingress_filter_get_next failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(rdpa_filter )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_protocol_filters_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_proto_filters_mask_t parm;

	BDMF_TRACE_DBG("inside port_user_protocol_filters_get\n");

	if ((pa->ret = rdpa_port_protocol_filters_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_protocol_filters_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_proto_filters_mask_t)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_protocol_filters_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside port_user_protocol_filters_set\n");

	if ((pa->ret = rdpa_port_protocol_filters_set(pa->mo, (rdpa_proto_filters_mask_t)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_protocol_filters_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_enable_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside port_user_enable_get\n");

	if ((pa->ret = rdpa_port_enable_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_enable_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_enable_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside port_user_enable_set\n");

	if ((pa->ret = rdpa_port_enable_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_enable_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_is_empty_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside port_user_is_empty_get\n");

	if ((pa->ret = rdpa_port_is_empty_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_is_empty_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_mac_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_mac_t  parm;

	BDMF_TRACE_DBG("inside port_user_mac_get\n");

	if ((pa->ret = rdpa_port_mac_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_mac_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_mac_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_mac_set(rdpa_ioctl_cmd_t *pa)
{
	bdmf_mac_t  parm;

	BDMF_TRACE_DBG("inside port_user_mac_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(bdmf_mac_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_port_mac_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_mac_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_port_pkt_size_stat_en_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside port_user_pkt_size_stat_en_get\n");

	if ((pa->ret = rdpa_port_pkt_size_stat_en_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_pkt_size_stat_en_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_port_pkt_size_stat_en_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside port_user_pkt_size_stat_en_set\n");

	if ((pa->ret = rdpa_port_pkt_size_stat_en_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_port_pkt_size_stat_en_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

long rdpa_port_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa)
{
	int ret;

	switch (op){
		case RDPA_PORT_DRV:
			ret = rdpa_user_port_drv(pa);
			break;

		case RDPA_PORT_GET:
			ret = rdpa_user_port_get(pa);
			break;

		case RDPA_PORT_INDEX_GET:
			ret = rdpa_user_port_index_get(pa);
			break;

		case RDPA_PORT_INDEX_SET:
			ret = rdpa_user_port_index_set(pa);
			break;

		case RDPA_PORT_WAN_TYPE_GET:
			ret = rdpa_user_port_wan_type_get(pa);
			break;

		case RDPA_PORT_WAN_TYPE_SET:
			ret = rdpa_user_port_wan_type_set(pa);
			break;

		case RDPA_PORT_SPEED_GET:
			ret = rdpa_user_port_speed_get(pa);
			break;

		case RDPA_PORT_SPEED_SET:
			ret = rdpa_user_port_speed_set(pa);
			break;

		case RDPA_PORT_CFG_GET:
			ret = rdpa_user_port_cfg_get(pa);
			break;

		case RDPA_PORT_CFG_SET:
			ret = rdpa_user_port_cfg_set(pa);
			break;

		case RDPA_PORT_TM_CFG_GET:
			ret = rdpa_user_port_tm_cfg_get(pa);
			break;

		case RDPA_PORT_TM_CFG_SET:
			ret = rdpa_user_port_tm_cfg_set(pa);
			break;

		case RDPA_PORT_SA_LIMIT_GET:
			ret = rdpa_user_port_sa_limit_get(pa);
			break;

		case RDPA_PORT_SA_LIMIT_SET:
			ret = rdpa_user_port_sa_limit_set(pa);
			break;

		case RDPA_PORT_DEF_FLOW_GET:
			ret = rdpa_user_port_def_flow_get(pa);
			break;

		case RDPA_PORT_DEF_FLOW_SET:
			ret = rdpa_user_port_def_flow_set(pa);
			break;

		case RDPA_PORT_STAT_GET:
			ret = rdpa_user_port_stat_get(pa);
			break;

		case RDPA_PORT_STAT_SET:
			ret = rdpa_user_port_stat_set(pa);
			break;

		case RDPA_PORT_FLOW_CONTROL_GET:
			ret = rdpa_user_port_flow_control_get(pa);
			break;

		case RDPA_PORT_FLOW_CONTROL_SET:
			ret = rdpa_user_port_flow_control_set(pa);
			break;

		case RDPA_PORT_INGRESS_RATE_LIMIT_GET:
			ret = rdpa_user_port_ingress_rate_limit_get(pa);
			break;

		case RDPA_PORT_INGRESS_RATE_LIMIT_SET:
			ret = rdpa_user_port_ingress_rate_limit_set(pa);
			break;

		case RDPA_PORT_MIRROR_CFG_GET:
			ret = rdpa_user_port_mirror_cfg_get(pa);
			break;

		case RDPA_PORT_MIRROR_CFG_SET:
			ret = rdpa_user_port_mirror_cfg_set(pa);
			break;

		case RDPA_PORT_VLAN_ISOLATION_GET:
			ret = rdpa_user_port_vlan_isolation_get(pa);
			break;

		case RDPA_PORT_VLAN_ISOLATION_SET:
			ret = rdpa_user_port_vlan_isolation_set(pa);
			break;

		case RDPA_PORT_TRANSPARENT_GET:
			ret = rdpa_user_port_transparent_get(pa);
			break;

		case RDPA_PORT_TRANSPARENT_SET:
			ret = rdpa_user_port_transparent_set(pa);
			break;

		case RDPA_PORT_LOOPBACK_GET:
			ret = rdpa_user_port_loopback_get(pa);
			break;

		case RDPA_PORT_LOOPBACK_SET:
			ret = rdpa_user_port_loopback_set(pa);
			break;

		case RDPA_PORT_MTU_SIZE_GET:
			ret = rdpa_user_port_mtu_size_get(pa);
			break;

		case RDPA_PORT_MTU_SIZE_SET:
			ret = rdpa_user_port_mtu_size_set(pa);
			break;

		case RDPA_PORT_CPU_OBJ_GET:
			ret = rdpa_user_port_cpu_obj_get(pa);
			break;

		case RDPA_PORT_CPU_OBJ_SET:
			ret = rdpa_user_port_cpu_obj_set(pa);
			break;

		case RDPA_PORT_CPU_METER_GET:
			ret = rdpa_user_port_cpu_meter_get(pa);
			break;

		case RDPA_PORT_CPU_METER_SET:
			ret = rdpa_user_port_cpu_meter_set(pa);
			break;

		case RDPA_PORT_INGRESS_FILTER_GET:
			ret = rdpa_user_port_ingress_filter_get(pa);
			break;

		case RDPA_PORT_INGRESS_FILTER_SET:
			ret = rdpa_user_port_ingress_filter_set(pa);
			break;

		case RDPA_PORT_INGRESS_FILTER_GET_NEXT:
			ret = rdpa_user_port_ingress_filter_get_next(pa);
			break;

		case RDPA_PORT_PROTOCOL_FILTERS_GET:
			ret = rdpa_user_port_protocol_filters_get(pa);
			break;

		case RDPA_PORT_PROTOCOL_FILTERS_SET:
			ret = rdpa_user_port_protocol_filters_set(pa);
			break;

		case RDPA_PORT_ENABLE_GET:
			ret = rdpa_user_port_enable_get(pa);
			break;

		case RDPA_PORT_ENABLE_SET:
			ret = rdpa_user_port_enable_set(pa);
			break;

		case RDPA_PORT_IS_EMPTY_GET:
			ret = rdpa_user_port_is_empty_get(pa);
			break;

		case RDPA_PORT_MAC_GET:
			ret = rdpa_user_port_mac_get(pa);
			break;

		case RDPA_PORT_MAC_SET:
			ret = rdpa_user_port_mac_set(pa);
			break;

		case RDPA_PORT_PKT_SIZE_STAT_EN_GET:
			ret = rdpa_user_port_pkt_size_stat_en_get(pa);
			break;

		case RDPA_PORT_PKT_SIZE_STAT_EN_SET:
			ret = rdpa_user_port_pkt_size_stat_en_set(pa);
			break;

		default:
			BDMF_TRACE_ERR("no such ioctl cmd: %u\n", op);
			ret = EINVAL;
		}

	return ret;
}
