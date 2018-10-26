/*
 * ingress_class object ioctl functions implementation file.
 * This ioctl file is generated automatically. Do not edit!
 */
#include "rdpa_api.h"
#include "rdpa_user.h"
#include "rdpa_user_int.h"
#include "rdpa_ingress_class_user_ioctl_ag.h"

static int rdpa_user_ingress_class_drv(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_ingress_class_drv\n");

	if (!(pa->drv = rdpa_ingress_class_drv()))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_drv failed\n");
	}

	return 0;
}

static int rdpa_user_ingress_class_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_ingress_class_key_t  parm;

	BDMF_TRACE_DBG("inside rdpa_user_ingress_class_drv\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_ingress_class_key_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}
	if ((pa->ret = rdpa_ingress_class_get(&parm, &pa->mo)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_get failed ret: %d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_ingress_class_dir_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_traffic_dir parm;

	BDMF_TRACE_DBG("inside ingress_class_user_dir_get\n");

	if ((pa->ret = rdpa_ingress_class_dir_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_dir_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_traffic_dir)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ingress_class_dir_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside ingress_class_user_dir_set\n");

	if ((pa->ret = rdpa_ingress_class_dir_set(pa->mo, (rdpa_traffic_dir)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_dir_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_ingress_class_index_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside ingress_class_user_index_get\n");

	if ((pa->ret = rdpa_ingress_class_index_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_index_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ingress_class_index_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside ingress_class_user_index_set\n");

	if ((pa->ret = rdpa_ingress_class_index_set(pa->mo, (bdmf_number)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_index_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_ingress_class_cfg_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_ic_cfg_t   parm;

	BDMF_TRACE_DBG("inside ingress_class_user_cfg_get\n");

	if ((pa->ret = rdpa_ingress_class_cfg_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_cfg_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_ic_cfg_t  )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ingress_class_cfg_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_ic_cfg_t   parm;

	BDMF_TRACE_DBG("inside ingress_class_user_cfg_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_ic_cfg_t  )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_ingress_class_cfg_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_cfg_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_ingress_class_nflow_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside ingress_class_user_nflow_get\n");

	if ((pa->ret = rdpa_ingress_class_nflow_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_nflow_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ingress_class_flow_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_ic_info_t   parm;

	BDMF_TRACE_DBG("inside ingress_class_user_flow_get\n");

	if ((pa->ret = rdpa_ingress_class_flow_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_flow_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_ic_info_t  )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ingress_class_flow_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_ic_info_t   parm;

	BDMF_TRACE_DBG("inside ingress_class_user_flow_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_ic_info_t  )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_ingress_class_flow_set(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_flow_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_ingress_class_flow_add(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;
	rdpa_ic_info_t   parm;

	BDMF_TRACE_DBG("inside ingress_class_user_flow_add\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_ic_info_t  )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_ingress_class_flow_add(pa->mo, &ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_flow_add failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ingress_class_flow_delete(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside ingress_class_flow_delete\n");

	if ((pa->ret = rdpa_ingress_class_flow_delete(pa->mo, (bdmf_index)pa->ai)))
	{
		BDMF_TRACE_ERR("rdpa_ingress_class_flow_delete failed\n");
	}

	return 0;
}

static int rdpa_user_ingress_class_flow_get_next(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;

	BDMF_TRACE_DBG("inside ingress_class_user_flow_get_next\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_ingress_class_flow_get_next(pa->mo, &ai)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_flow_get_next failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ingress_class_flow_find(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;
	rdpa_ic_info_t   parm;

	BDMF_TRACE_DBG("inside ingress_class_user_flow_find\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_ic_info_t  )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_ingress_class_flow_find(pa->mo, &ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_flow_find failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_ic_info_t  )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ingress_class_flow_stat_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_stat_t  parm;

	BDMF_TRACE_DBG("inside ingress_class_user_flow_stat_get\n");

	if ((pa->ret = rdpa_ingress_class_flow_stat_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_flow_stat_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_stat_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ingress_class_flow_stat_get_next(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;

	BDMF_TRACE_DBG("inside ingress_class_user_flow_stat_get_next\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_ingress_class_flow_stat_get_next(pa->mo, &ai)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_flow_stat_get_next failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ingress_class_port_action_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_action_key_t  ai;
	rdpa_port_action_t  parm;

	BDMF_TRACE_DBG("inside ingress_class_user_port_action_get\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(rdpa_port_action_key_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_ingress_class_port_action_get(pa->mo, &ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_port_action_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_port_action_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ingress_class_port_action_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_action_key_t  ai;
	rdpa_port_action_t  parm;

	BDMF_TRACE_DBG("inside ingress_class_user_port_action_set\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(rdpa_port_action_key_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_port_action_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_ingress_class_port_action_set(pa->mo, &ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_port_action_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_ingress_class_port_action_get_next(rdpa_ioctl_cmd_t *pa)
{
	rdpa_port_action_key_t  ai;

	BDMF_TRACE_DBG("inside ingress_class_user_port_action_get_next\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(rdpa_port_action_key_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_ingress_class_port_action_get_next(pa->mo, &ai)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_port_action_get_next failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(rdpa_port_action_key_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_ingress_class_flush_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside ingress_class_user_flush_set\n");

	if ((pa->ret = rdpa_ingress_class_flush_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_ingress_class_flush_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

long rdpa_ingress_class_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa)
{
	int ret;

	switch (op){
		case RDPA_INGRESS_CLASS_DRV:
			ret = rdpa_user_ingress_class_drv(pa);
			break;

		case RDPA_INGRESS_CLASS_GET:
			ret = rdpa_user_ingress_class_get(pa);
			break;

		case RDPA_INGRESS_CLASS_DIR_GET:
			ret = rdpa_user_ingress_class_dir_get(pa);
			break;

		case RDPA_INGRESS_CLASS_DIR_SET:
			ret = rdpa_user_ingress_class_dir_set(pa);
			break;

		case RDPA_INGRESS_CLASS_INDEX_GET:
			ret = rdpa_user_ingress_class_index_get(pa);
			break;

		case RDPA_INGRESS_CLASS_INDEX_SET:
			ret = rdpa_user_ingress_class_index_set(pa);
			break;

		case RDPA_INGRESS_CLASS_CFG_GET:
			ret = rdpa_user_ingress_class_cfg_get(pa);
			break;

		case RDPA_INGRESS_CLASS_CFG_SET:
			ret = rdpa_user_ingress_class_cfg_set(pa);
			break;

		case RDPA_INGRESS_CLASS_NFLOW_GET:
			ret = rdpa_user_ingress_class_nflow_get(pa);
			break;

		case RDPA_INGRESS_CLASS_FLOW_GET:
			ret = rdpa_user_ingress_class_flow_get(pa);
			break;

		case RDPA_INGRESS_CLASS_FLOW_SET:
			ret = rdpa_user_ingress_class_flow_set(pa);
			break;

		case RDPA_INGRESS_CLASS_FLOW_ADD:
			ret = rdpa_user_ingress_class_flow_add(pa);
			break;

		case RDPA_INGRESS_CLASS_FLOW_DELETE:
			ret = rdpa_user_ingress_class_flow_delete(pa);
			break;

		case RDPA_INGRESS_CLASS_FLOW_GET_NEXT:
			ret = rdpa_user_ingress_class_flow_get_next(pa);
			break;

		case RDPA_INGRESS_CLASS_FLOW_FIND:
			ret = rdpa_user_ingress_class_flow_find(pa);
			break;

		case RDPA_INGRESS_CLASS_FLOW_STAT_GET:
			ret = rdpa_user_ingress_class_flow_stat_get(pa);
			break;

		case RDPA_INGRESS_CLASS_FLOW_STAT_GET_NEXT:
			ret = rdpa_user_ingress_class_flow_stat_get_next(pa);
			break;

		case RDPA_INGRESS_CLASS_PORT_ACTION_GET:
			ret = rdpa_user_ingress_class_port_action_get(pa);
			break;

		case RDPA_INGRESS_CLASS_PORT_ACTION_SET:
			ret = rdpa_user_ingress_class_port_action_set(pa);
			break;

		case RDPA_INGRESS_CLASS_PORT_ACTION_GET_NEXT:
			ret = rdpa_user_ingress_class_port_action_get_next(pa);
			break;

		case RDPA_INGRESS_CLASS_FLUSH_SET:
			ret = rdpa_user_ingress_class_flush_set(pa);
			break;

		default:
			BDMF_TRACE_ERR("no such ioctl cmd: %u\n", op);
			ret = EINVAL;
		}

	return ret;
}
