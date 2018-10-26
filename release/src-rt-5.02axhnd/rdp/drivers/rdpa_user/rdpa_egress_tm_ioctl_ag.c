/*
 * egress_tm object ioctl functions implementation file.
 * This ioctl file is generated automatically. Do not edit!
 */
#include "rdpa_api.h"
#include "rdpa_user.h"
#include "rdpa_user_int.h"
#include "rdpa_egress_tm_user_ioctl_ag.h"

static int rdpa_user_egress_tm_drv(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_egress_tm_drv\n");

	if (!(pa->drv = rdpa_egress_tm_drv()))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_drv failed\n");
	}

	return 0;
}

static int rdpa_user_egress_tm_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_egress_tm_key_t  parm;

	BDMF_TRACE_DBG("inside rdpa_user_egress_tm_drv\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_egress_tm_key_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}
	if ((pa->ret = rdpa_egress_tm_get(&parm, &pa->mo)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_get failed ret: %d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_egress_tm_dir_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_traffic_dir parm;

	BDMF_TRACE_DBG("inside egress_tm_user_dir_get\n");

	if ((pa->ret = rdpa_egress_tm_dir_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_dir_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_traffic_dir)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_dir_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside egress_tm_user_dir_set\n");

	if ((pa->ret = rdpa_egress_tm_dir_set(pa->mo, (rdpa_traffic_dir)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_dir_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_egress_tm_index_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside egress_tm_user_index_get\n");

	if ((pa->ret = rdpa_egress_tm_index_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_index_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_index_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside egress_tm_user_index_set\n");

	if ((pa->ret = rdpa_egress_tm_index_set(pa->mo, (bdmf_number)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_index_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_egress_tm_level_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_level_type parm;

	BDMF_TRACE_DBG("inside egress_tm_user_level_get\n");

	if ((pa->ret = rdpa_egress_tm_level_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_level_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_tm_level_type)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_level_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside egress_tm_user_level_set\n");

	if ((pa->ret = rdpa_egress_tm_level_set(pa->mo, (rdpa_tm_level_type)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_level_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_egress_tm_mode_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_sched_mode parm;

	BDMF_TRACE_DBG("inside egress_tm_user_mode_get\n");

	if ((pa->ret = rdpa_egress_tm_mode_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_mode_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_tm_sched_mode)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_mode_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside egress_tm_user_mode_set\n");

	if ((pa->ret = rdpa_egress_tm_mode_set(pa->mo, (rdpa_tm_sched_mode)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_mode_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_egress_tm_overall_rl_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside egress_tm_user_overall_rl_get\n");

	if ((pa->ret = rdpa_egress_tm_overall_rl_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_overall_rl_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_overall_rl_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside egress_tm_user_overall_rl_set\n");

	if ((pa->ret = rdpa_egress_tm_overall_rl_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_overall_rl_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_egress_tm_service_queue_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_service_queue_t  parm;

	BDMF_TRACE_DBG("inside egress_tm_user_service_queue_get\n");

	if ((pa->ret = rdpa_egress_tm_service_queue_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_service_queue_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_tm_service_queue_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_service_queue_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_service_queue_t  parm;

	BDMF_TRACE_DBG("inside egress_tm_user_service_queue_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_tm_service_queue_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_egress_tm_service_queue_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_service_queue_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_egress_tm_rl_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_rl_cfg_t  parm;

	BDMF_TRACE_DBG("inside egress_tm_user_rl_get\n");

	if ((pa->ret = rdpa_egress_tm_rl_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_rl_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_tm_rl_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_rl_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_rl_cfg_t  parm;

	BDMF_TRACE_DBG("inside egress_tm_user_rl_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_tm_rl_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_egress_tm_rl_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_rl_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_egress_tm_rl_rate_mode_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_rl_rate_mode parm;

	BDMF_TRACE_DBG("inside egress_tm_user_rl_rate_mode_get\n");

	if ((pa->ret = rdpa_egress_tm_rl_rate_mode_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_rl_rate_mode_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_tm_rl_rate_mode)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_rl_rate_mode_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside egress_tm_user_rl_rate_mode_set\n");

	if ((pa->ret = rdpa_egress_tm_rl_rate_mode_set(pa->mo, (rdpa_tm_rl_rate_mode)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_rl_rate_mode_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_egress_tm_num_queues_get(rdpa_ioctl_cmd_t *pa)
{
	uint8_t parm;

	BDMF_TRACE_DBG("inside egress_tm_user_num_queues_get\n");

	if ((pa->ret = rdpa_egress_tm_num_queues_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_num_queues_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(uint8_t)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_num_queues_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside egress_tm_user_num_queues_set\n");

	if ((pa->ret = rdpa_egress_tm_num_queues_set(pa->mo, (uint8_t)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_num_queues_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_egress_tm_num_sp_elements_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_num_sp_elem parm;

	BDMF_TRACE_DBG("inside egress_tm_user_num_sp_elements_get\n");

	if ((pa->ret = rdpa_egress_tm_num_sp_elements_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_num_sp_elements_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_tm_num_sp_elem)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_num_sp_elements_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside egress_tm_user_num_sp_elements_set\n");

	if ((pa->ret = rdpa_egress_tm_num_sp_elements_set(pa->mo, (rdpa_tm_num_sp_elem)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_num_sp_elements_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_egress_tm_queue_cfg_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_queue_cfg_t  parm;

	BDMF_TRACE_DBG("inside egress_tm_user_queue_cfg_get\n");

	if ((pa->ret = rdpa_egress_tm_queue_cfg_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_queue_cfg_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_tm_queue_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_queue_cfg_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_queue_cfg_t  parm;

	BDMF_TRACE_DBG("inside egress_tm_user_queue_cfg_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_tm_queue_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_egress_tm_queue_cfg_set(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_queue_cfg_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_egress_tm_queue_cfg_delete(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside egress_tm_queue_cfg_delete\n");

	if ((pa->ret = rdpa_egress_tm_queue_cfg_delete(pa->mo, (bdmf_index)pa->ai)))
	{
		BDMF_TRACE_ERR("rdpa_egress_tm_queue_cfg_delete failed\n");
	}

	return 0;
}

static int rdpa_user_egress_tm_queue_statistics_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_queue_index_t  ai;
	rdpa_stat_1way_t  parm;

	BDMF_TRACE_DBG("inside egress_tm_user_queue_statistics_get\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(rdpa_tm_queue_index_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_egress_tm_queue_statistics_get(pa->mo, &ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_queue_statistics_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_stat_1way_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_queue_statistics_get_next(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_queue_index_t  ai;

	BDMF_TRACE_DBG("inside egress_tm_user_queue_statistics_get_next\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(rdpa_tm_queue_index_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_egress_tm_queue_statistics_get_next(pa->mo, &ai)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_queue_statistics_get_next failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(rdpa_tm_queue_index_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_queue_stat_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_queue_index_t  ai;
	rdpa_stat_1way_t  parm;

	BDMF_TRACE_DBG("inside egress_tm_user_queue_stat_get\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(rdpa_tm_queue_index_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_egress_tm_queue_stat_get(pa->mo, &ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_queue_stat_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_stat_1way_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_queue_stat_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_queue_index_t  ai;
	rdpa_stat_1way_t  parm;

	BDMF_TRACE_DBG("inside egress_tm_user_queue_stat_set\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(rdpa_tm_queue_index_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_stat_1way_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_egress_tm_queue_stat_set(pa->mo, &ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_queue_stat_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_egress_tm_queue_stat_get_next(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_queue_index_t  ai;

	BDMF_TRACE_DBG("inside egress_tm_user_queue_stat_get_next\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(rdpa_tm_queue_index_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_egress_tm_queue_stat_get_next(pa->mo, &ai)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_queue_stat_get_next failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(rdpa_tm_queue_index_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_queue_occupancy_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_queue_index_t  ai;
	rdpa_stat_t  parm;

	BDMF_TRACE_DBG("inside egress_tm_user_queue_occupancy_get\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(rdpa_tm_queue_index_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_egress_tm_queue_occupancy_get(pa->mo, &ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_queue_occupancy_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_stat_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_queue_occupancy_get_next(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_queue_index_t  ai;

	BDMF_TRACE_DBG("inside egress_tm_user_queue_occupancy_get_next\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(rdpa_tm_queue_index_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_egress_tm_queue_occupancy_get_next(pa->mo, &ai)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_queue_occupancy_get_next failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(rdpa_tm_queue_index_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_subsidiary_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_object_handle parm;

	BDMF_TRACE_DBG("inside egress_tm_user_subsidiary_get\n");

	if ((pa->ret = rdpa_egress_tm_subsidiary_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_subsidiary_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_object_handle)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_subsidiary_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside egress_tm_user_subsidiary_set\n");

	if ((pa->ret = rdpa_egress_tm_subsidiary_set(pa->mo, pa->ai, pa->object)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_subsidiary_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_egress_tm_subsidiary_find(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;
	bdmf_object_handle parm;

	BDMF_TRACE_DBG("inside egress_tm_user_subsidiary_find\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(bdmf_object_handle)))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_egress_tm_subsidiary_find(pa->mo, &ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_subsidiary_find failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_object_handle)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_weight_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_number parm;

	BDMF_TRACE_DBG("inside egress_tm_user_weight_get\n");

	if ((pa->ret = rdpa_egress_tm_weight_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_weight_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_number)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_egress_tm_weight_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside egress_tm_user_weight_set\n");

	if ((pa->ret = rdpa_egress_tm_weight_set(pa->mo, (bdmf_number)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_weight_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_egress_tm_queue_location_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_tm_queue_location_t  parm;

	BDMF_TRACE_DBG("inside egress_tm_user_queue_location_get\n");

	if ((pa->ret = rdpa_egress_tm_queue_location_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_egress_tm_queue_location_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_tm_queue_location_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

long rdpa_egress_tm_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa)
{
	int ret;

	switch (op){
		case RDPA_EGRESS_TM_DRV:
			ret = rdpa_user_egress_tm_drv(pa);
			break;

		case RDPA_EGRESS_TM_GET:
			ret = rdpa_user_egress_tm_get(pa);
			break;

		case RDPA_EGRESS_TM_DIR_GET:
			ret = rdpa_user_egress_tm_dir_get(pa);
			break;

		case RDPA_EGRESS_TM_DIR_SET:
			ret = rdpa_user_egress_tm_dir_set(pa);
			break;

		case RDPA_EGRESS_TM_INDEX_GET:
			ret = rdpa_user_egress_tm_index_get(pa);
			break;

		case RDPA_EGRESS_TM_INDEX_SET:
			ret = rdpa_user_egress_tm_index_set(pa);
			break;

		case RDPA_EGRESS_TM_LEVEL_GET:
			ret = rdpa_user_egress_tm_level_get(pa);
			break;

		case RDPA_EGRESS_TM_LEVEL_SET:
			ret = rdpa_user_egress_tm_level_set(pa);
			break;

		case RDPA_EGRESS_TM_MODE_GET:
			ret = rdpa_user_egress_tm_mode_get(pa);
			break;

		case RDPA_EGRESS_TM_MODE_SET:
			ret = rdpa_user_egress_tm_mode_set(pa);
			break;

		case RDPA_EGRESS_TM_OVERALL_RL_GET:
			ret = rdpa_user_egress_tm_overall_rl_get(pa);
			break;

		case RDPA_EGRESS_TM_OVERALL_RL_SET:
			ret = rdpa_user_egress_tm_overall_rl_set(pa);
			break;

		case RDPA_EGRESS_TM_SERVICE_QUEUE_GET:
			ret = rdpa_user_egress_tm_service_queue_get(pa);
			break;

		case RDPA_EGRESS_TM_SERVICE_QUEUE_SET:
			ret = rdpa_user_egress_tm_service_queue_set(pa);
			break;

		case RDPA_EGRESS_TM_RL_GET:
			ret = rdpa_user_egress_tm_rl_get(pa);
			break;

		case RDPA_EGRESS_TM_RL_SET:
			ret = rdpa_user_egress_tm_rl_set(pa);
			break;

		case RDPA_EGRESS_TM_RL_RATE_MODE_GET:
			ret = rdpa_user_egress_tm_rl_rate_mode_get(pa);
			break;

		case RDPA_EGRESS_TM_RL_RATE_MODE_SET:
			ret = rdpa_user_egress_tm_rl_rate_mode_set(pa);
			break;

		case RDPA_EGRESS_TM_NUM_QUEUES_GET:
			ret = rdpa_user_egress_tm_num_queues_get(pa);
			break;

		case RDPA_EGRESS_TM_NUM_QUEUES_SET:
			ret = rdpa_user_egress_tm_num_queues_set(pa);
			break;

		case RDPA_EGRESS_TM_NUM_SP_ELEMENTS_GET:
			ret = rdpa_user_egress_tm_num_sp_elements_get(pa);
			break;

		case RDPA_EGRESS_TM_NUM_SP_ELEMENTS_SET:
			ret = rdpa_user_egress_tm_num_sp_elements_set(pa);
			break;

		case RDPA_EGRESS_TM_QUEUE_CFG_GET:
			ret = rdpa_user_egress_tm_queue_cfg_get(pa);
			break;

		case RDPA_EGRESS_TM_QUEUE_CFG_SET:
			ret = rdpa_user_egress_tm_queue_cfg_set(pa);
			break;

		case RDPA_EGRESS_TM_QUEUE_CFG_DELETE:
			ret = rdpa_user_egress_tm_queue_cfg_delete(pa);
			break;

		case RDPA_EGRESS_TM_QUEUE_STATISTICS_GET:
			ret = rdpa_user_egress_tm_queue_statistics_get(pa);
			break;

		case RDPA_EGRESS_TM_QUEUE_STATISTICS_GET_NEXT:
			ret = rdpa_user_egress_tm_queue_statistics_get_next(pa);
			break;

		case RDPA_EGRESS_TM_QUEUE_STAT_GET:
			ret = rdpa_user_egress_tm_queue_stat_get(pa);
			break;

		case RDPA_EGRESS_TM_QUEUE_STAT_SET:
			ret = rdpa_user_egress_tm_queue_stat_set(pa);
			break;

		case RDPA_EGRESS_TM_QUEUE_STAT_GET_NEXT:
			ret = rdpa_user_egress_tm_queue_stat_get_next(pa);
			break;

		case RDPA_EGRESS_TM_QUEUE_OCCUPANCY_GET:
			ret = rdpa_user_egress_tm_queue_occupancy_get(pa);
			break;

		case RDPA_EGRESS_TM_QUEUE_OCCUPANCY_GET_NEXT:
			ret = rdpa_user_egress_tm_queue_occupancy_get_next(pa);
			break;

		case RDPA_EGRESS_TM_SUBSIDIARY_GET:
			ret = rdpa_user_egress_tm_subsidiary_get(pa);
			break;

		case RDPA_EGRESS_TM_SUBSIDIARY_SET:
			ret = rdpa_user_egress_tm_subsidiary_set(pa);
			break;

		case RDPA_EGRESS_TM_SUBSIDIARY_FIND:
			ret = rdpa_user_egress_tm_subsidiary_find(pa);
			break;

		case RDPA_EGRESS_TM_WEIGHT_GET:
			ret = rdpa_user_egress_tm_weight_get(pa);
			break;

		case RDPA_EGRESS_TM_WEIGHT_SET:
			ret = rdpa_user_egress_tm_weight_set(pa);
			break;

		case RDPA_EGRESS_TM_QUEUE_LOCATION_GET:
			ret = rdpa_user_egress_tm_queue_location_get(pa);
			break;

		default:
			BDMF_TRACE_ERR("no such ioctl cmd: %u\n", op);
			ret = EINVAL;
		}

	return ret;
}
