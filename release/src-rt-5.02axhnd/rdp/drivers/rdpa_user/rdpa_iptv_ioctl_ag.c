/*
 * iptv object ioctl functions implementation file.
 * This ioctl file is generated automatically. Do not edit!
 */
#include "rdpa_api.h"
#include "rdpa_user.h"
#include "rdpa_user_int.h"
#include "rdpa_iptv_user_ioctl_ag.h"

static int rdpa_user_iptv_drv(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_iptv_drv\n");

	if (!(pa->drv = rdpa_iptv_drv()))
	{
		BDMF_TRACE_DBG("rdpa_iptv_drv failed\n");
	}

	return 0;
}

static int rdpa_user_iptv_get(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_iptv_drv\n");

	if ((pa->ret = rdpa_iptv_get(&pa->mo)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_get failed ret: %d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_iptv_lookup_method_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_iptv_lookup_method parm;

	BDMF_TRACE_DBG("inside iptv_user_lookup_method_get\n");

	if ((pa->ret = rdpa_iptv_lookup_method_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_lookup_method_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_iptv_lookup_method)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_iptv_lookup_method_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside iptv_user_lookup_method_set\n");

	if ((pa->ret = rdpa_iptv_lookup_method_set(pa->mo, (rdpa_iptv_lookup_method)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_lookup_method_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_iptv_mcast_prefix_filter_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_mcast_filter_method parm;

	BDMF_TRACE_DBG("inside iptv_user_mcast_prefix_filter_get\n");

	if ((pa->ret = rdpa_iptv_mcast_prefix_filter_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_mcast_prefix_filter_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_mcast_filter_method)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_iptv_mcast_prefix_filter_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside iptv_user_mcast_prefix_filter_set\n");

	if ((pa->ret = rdpa_iptv_mcast_prefix_filter_set(pa->mo, (rdpa_mcast_filter_method)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_mcast_prefix_filter_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_iptv_lookup_miss_action_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_forward_action parm;

	BDMF_TRACE_DBG("inside iptv_user_lookup_miss_action_get\n");

	if ((pa->ret = rdpa_iptv_lookup_miss_action_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_lookup_miss_action_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_forward_action)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_iptv_lookup_miss_action_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside iptv_user_lookup_miss_action_set\n");

	if ((pa->ret = rdpa_iptv_lookup_miss_action_set(pa->mo, (rdpa_forward_action)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_lookup_miss_action_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_iptv_iptv_stat_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_iptv_stat_t  parm;

	BDMF_TRACE_DBG("inside iptv_user_iptv_stat_get\n");

	if ((pa->ret = rdpa_iptv_iptv_stat_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_iptv_stat_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_iptv_stat_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_iptv_iptv_stat_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_iptv_stat_t  parm;

	BDMF_TRACE_DBG("inside iptv_user_iptv_stat_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_iptv_stat_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_iptv_iptv_stat_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_iptv_stat_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_iptv_channel_request_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_iptv_channel_request_t  parm;

	BDMF_TRACE_DBG("inside iptv_user_channel_request_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_iptv_channel_request_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_iptv_channel_request_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_channel_request_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_iptv_channel_request_add(rdpa_ioctl_cmd_t *pa)
{
	rdpa_channel_req_key_t  ai;
	rdpa_iptv_channel_request_t  parm;

	BDMF_TRACE_DBG("inside iptv_user_channel_request_add\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(rdpa_channel_req_key_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_iptv_channel_request_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_iptv_channel_request_add(pa->mo, &ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_channel_request_add failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(rdpa_channel_req_key_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_iptv_channel_request_delete(rdpa_ioctl_cmd_t *pa)
{
	rdpa_channel_req_key_t  ai;
	BDMF_TRACE_DBG("inside iptv_channel_request_delete\n");


	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(rdpa_channel_req_key_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}
	if ((pa->ret = rdpa_iptv_channel_request_delete(pa->mo, &ai)))
	{
		BDMF_TRACE_ERR("rdpa_iptv_channel_request_delete failed\n");
	}

	return 0;
}

static int rdpa_user_iptv_channel_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_iptv_channel_t  parm;

	BDMF_TRACE_DBG("inside iptv_user_channel_get\n");

	if ((pa->ret = rdpa_iptv_channel_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_channel_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_iptv_channel_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_iptv_channel_get_next(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;

	BDMF_TRACE_DBG("inside iptv_user_channel_get_next\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_iptv_channel_get_next(pa->mo, &ai)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_channel_get_next failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_iptv_channel_find(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;
	rdpa_iptv_channel_t  parm;

	BDMF_TRACE_DBG("inside iptv_user_channel_find\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_iptv_channel_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_iptv_channel_find(pa->mo, &ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_channel_find failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_iptv_channel_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_iptv_channel_pm_stats_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_stat_t  parm;

	BDMF_TRACE_DBG("inside iptv_user_channel_pm_stats_get\n");

	if ((pa->ret = rdpa_iptv_channel_pm_stats_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_channel_pm_stats_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_stat_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_iptv_channel_pm_stats_get_next(rdpa_ioctl_cmd_t *pa)
{
	bdmf_index  ai;

	BDMF_TRACE_DBG("inside iptv_user_channel_pm_stats_get_next\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_iptv_channel_pm_stats_get_next(pa->mo, &ai)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_channel_pm_stats_get_next failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(bdmf_index )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_iptv_flush_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside iptv_user_flush_set\n");

	if ((pa->ret = rdpa_iptv_flush_set(pa->mo, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_iptv_flush_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

long rdpa_iptv_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa)
{
	int ret;

	switch (op){
		case RDPA_IPTV_DRV:
			ret = rdpa_user_iptv_drv(pa);
			break;

		case RDPA_IPTV_GET:
			ret = rdpa_user_iptv_get(pa);
			break;

		case RDPA_IPTV_LOOKUP_METHOD_GET:
			ret = rdpa_user_iptv_lookup_method_get(pa);
			break;

		case RDPA_IPTV_LOOKUP_METHOD_SET:
			ret = rdpa_user_iptv_lookup_method_set(pa);
			break;

		case RDPA_IPTV_MCAST_PREFIX_FILTER_GET:
			ret = rdpa_user_iptv_mcast_prefix_filter_get(pa);
			break;

		case RDPA_IPTV_MCAST_PREFIX_FILTER_SET:
			ret = rdpa_user_iptv_mcast_prefix_filter_set(pa);
			break;

		case RDPA_IPTV_LOOKUP_MISS_ACTION_GET:
			ret = rdpa_user_iptv_lookup_miss_action_get(pa);
			break;

		case RDPA_IPTV_LOOKUP_MISS_ACTION_SET:
			ret = rdpa_user_iptv_lookup_miss_action_set(pa);
			break;

		case RDPA_IPTV_IPTV_STAT_GET:
			ret = rdpa_user_iptv_iptv_stat_get(pa);
			break;

		case RDPA_IPTV_IPTV_STAT_SET:
			ret = rdpa_user_iptv_iptv_stat_set(pa);
			break;

		case RDPA_IPTV_CHANNEL_REQUEST_SET:
			ret = rdpa_user_iptv_channel_request_set(pa);
			break;

		case RDPA_IPTV_CHANNEL_REQUEST_ADD:
			ret = rdpa_user_iptv_channel_request_add(pa);
			break;

		case RDPA_IPTV_CHANNEL_REQUEST_DELETE:
			ret = rdpa_user_iptv_channel_request_delete(pa);
			break;

		case RDPA_IPTV_CHANNEL_GET:
			ret = rdpa_user_iptv_channel_get(pa);
			break;

		case RDPA_IPTV_CHANNEL_GET_NEXT:
			ret = rdpa_user_iptv_channel_get_next(pa);
			break;

		case RDPA_IPTV_CHANNEL_FIND:
			ret = rdpa_user_iptv_channel_find(pa);
			break;

		case RDPA_IPTV_CHANNEL_PM_STATS_GET:
			ret = rdpa_user_iptv_channel_pm_stats_get(pa);
			break;

		case RDPA_IPTV_CHANNEL_PM_STATS_GET_NEXT:
			ret = rdpa_user_iptv_channel_pm_stats_get_next(pa);
			break;

		case RDPA_IPTV_FLUSH_SET:
			ret = rdpa_user_iptv_flush_set(pa);
			break;

		default:
			BDMF_TRACE_ERR("no such ioctl cmd: %u\n", op);
			ret = EINVAL;
		}

	return ret;
}
