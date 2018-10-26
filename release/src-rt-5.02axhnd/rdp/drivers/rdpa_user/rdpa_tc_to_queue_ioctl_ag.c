/*
 * tc_to_queue object ioctl functions implementation file.
 * This ioctl file is generated automatically. Do not edit!
 */
#include "rdpa_api.h"
#include "rdpa_user.h"
#include "rdpa_user_int.h"
#include "rdpa_tc_to_queue_user_ioctl_ag.h"

static int rdpa_user_tc_to_queue_drv(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_tc_to_queue_drv\n");

	if (!(pa->drv = rdpa_tc_to_queue_drv()))
	{
		BDMF_TRACE_DBG("rdpa_tc_to_queue_drv failed\n");
	}

	return 0;
}

static int rdpa_user_tc_to_queue_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tc_to_queue_key_t  parm;

	BDMF_TRACE_DBG("inside rdpa_user_tc_to_queue_drv\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_tc_to_queue_key_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}
	if ((pa->ret = rdpa_tc_to_queue_get(&parm, &pa->mo)))
	{
		BDMF_TRACE_DBG("rdpa_tc_to_queue_get failed ret: %d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_tc_to_queue_table_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside tc_to_queue_user_table_get\n");

	if ((pa->ret = rdpa_tc_to_queue_table_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_tc_to_queue_table_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_tc_to_queue_table_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside tc_to_queue_user_table_set\n");

	if ((pa->ret = rdpa_tc_to_queue_table_set(pa->mo, (bdmf_number)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_tc_to_queue_table_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_tc_to_queue_dir_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_traffic_dir parm;

	BDMF_TRACE_DBG("inside tc_to_queue_user_dir_get\n");

	if ((pa->ret = rdpa_tc_to_queue_dir_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_tc_to_queue_dir_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_traffic_dir)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_tc_to_queue_dir_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside tc_to_queue_user_dir_set\n");

	if ((pa->ret = rdpa_tc_to_queue_dir_set(pa->mo, (rdpa_traffic_dir)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_tc_to_queue_dir_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_tc_to_queue_tc_map_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside tc_to_queue_user_tc_map_get\n");

	if ((pa->ret = rdpa_tc_to_queue_tc_map_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_tc_to_queue_tc_map_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_tc_to_queue_tc_map_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside tc_to_queue_user_tc_map_set\n");

	if ((pa->ret = rdpa_tc_to_queue_tc_map_set(pa->mo, pa->ai, (bdmf_number)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_tc_to_queue_tc_map_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_tc_to_queue_tc_map_get_next(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;

	BDMF_TRACE_DBG("inside tc_to_queue_user_tc_map_get_next\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_tc_to_queue_tc_map_get_next(pa->mo, &ai)))
	{
		BDMF_TRACE_DBG("rdpa_tc_to_queue_tc_map_get_next failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

long rdpa_tc_to_queue_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa)
{
	int ret;

	switch (op){
		case RDPA_TC_TO_QUEUE_DRV:
			ret = rdpa_user_tc_to_queue_drv(pa);
			break;

		case RDPA_TC_TO_QUEUE_GET:
			ret = rdpa_user_tc_to_queue_get(pa);
			break;

		case RDPA_TC_TO_QUEUE_TABLE_GET:
			ret = rdpa_user_tc_to_queue_table_get(pa);
			break;

		case RDPA_TC_TO_QUEUE_TABLE_SET:
			ret = rdpa_user_tc_to_queue_table_set(pa);
			break;

		case RDPA_TC_TO_QUEUE_DIR_GET:
			ret = rdpa_user_tc_to_queue_dir_get(pa);
			break;

		case RDPA_TC_TO_QUEUE_DIR_SET:
			ret = rdpa_user_tc_to_queue_dir_set(pa);
			break;

		case RDPA_TC_TO_QUEUE_TC_MAP_GET:
			ret = rdpa_user_tc_to_queue_tc_map_get(pa);
			break;

		case RDPA_TC_TO_QUEUE_TC_MAP_SET:
			ret = rdpa_user_tc_to_queue_tc_map_set(pa);
			break;

		case RDPA_TC_TO_QUEUE_TC_MAP_GET_NEXT:
			ret = rdpa_user_tc_to_queue_tc_map_get_next(pa);
			break;

		default:
			BDMF_TRACE_ERR("no such ioctl cmd: %u\n", op);
			ret = EINVAL;
		}

	return ret;
}
