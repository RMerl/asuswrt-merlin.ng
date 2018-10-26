/*
 * vlan object ioctl functions implementation file.
 * This ioctl file is generated automatically. Do not edit!
 */
#include "rdpa_api.h"
#include "rdpa_user.h"
#include "rdpa_user_int.h"
#include "rdpa_vlan_user_ioctl_ag.h"

static int rdpa_user_vlan_drv(rdpa_ioctl_cmd_t *pa)
{
	BDMF_TRACE_DBG("inside rdpa_user_vlan_drv\n");

	if (!(pa->drv = rdpa_vlan_drv()))
	{
		BDMF_TRACE_DBG("rdpa_vlan_drv failed\n");
	}

	return 0;
}

static int rdpa_user_vlan_get(rdpa_ioctl_cmd_t *pa)
{
	char parm[32] = {0};

	BDMF_TRACE_DBG("inside rdpa_user_vlan_drv\n");

	if (copy_from_user((void *)parm, (void *)(long)pa->ptr, 31))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}
	if ((pa->ret = rdpa_vlan_get(parm, &pa->mo)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_get failed ret: %d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_vlan_name_get(rdpa_ioctl_cmd_t *pa)
{
	char parm[32] = {0};

	BDMF_TRACE_DBG("inside vlan_user_name_get\n");

	if (copy_from_user((void *)parm, (void *)(long)pa->ptr, 31))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_vlan_name_get(pa->mo, parm, pa->size)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_name_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)parm, 32))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_vlan_name_set(rdpa_ioctl_cmd_t *pa)
{
	char parm[32] = {0};

	BDMF_TRACE_DBG("inside vlan_user_name_set\n");

	if (copy_from_user((void *)parm, (void *)(long)pa->ptr, 31))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_vlan_name_set(pa->mo, parm)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_name_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_vlan_vid_enable_get(rdpa_ioctl_cmd_t *pa)
{
	bdmf_boolean parm;

	BDMF_TRACE_DBG("inside vlan_user_vid_enable_get\n");

	if ((pa->ret = rdpa_vlan_vid_enable_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_vid_enable_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(bdmf_boolean)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_vlan_vid_enable_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside vlan_user_vid_enable_set\n");

	if ((pa->ret = rdpa_vlan_vid_enable_set(pa->mo, pa->ai, (bdmf_boolean)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_vid_enable_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_vlan_ingress_filter_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_filter_ctrl_t  parm;

	BDMF_TRACE_DBG("inside vlan_user_ingress_filter_get\n");

	if ((pa->ret = rdpa_vlan_ingress_filter_get(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_ingress_filter_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_filter_ctrl_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_vlan_ingress_filter_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_filter_ctrl_t  parm;

	BDMF_TRACE_DBG("inside vlan_user_ingress_filter_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_filter_ctrl_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_vlan_ingress_filter_set(pa->mo, pa->ai, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_ingress_filter_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_vlan_ingress_filter_get_next(rdpa_ioctl_cmd_t *pa)
{
	rdpa_filter  ai;

	BDMF_TRACE_DBG("inside vlan_user_ingress_filter_get_next\n");

	if (copy_from_user((void *)&ai, (void *)(long)pa->ai_ptr, sizeof(rdpa_filter )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_vlan_ingress_filter_get_next(pa->mo, &ai)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_ingress_filter_get_next failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ai_ptr, (void *)&ai, sizeof(rdpa_filter )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_vlan_mac_lookup_cfg_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_mac_lookup_cfg_t  parm;

	BDMF_TRACE_DBG("inside vlan_user_mac_lookup_cfg_get\n");

	if ((pa->ret = rdpa_vlan_mac_lookup_cfg_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_mac_lookup_cfg_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_mac_lookup_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_vlan_mac_lookup_cfg_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_mac_lookup_cfg_t  parm;

	BDMF_TRACE_DBG("inside vlan_user_mac_lookup_cfg_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_mac_lookup_cfg_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_vlan_mac_lookup_cfg_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_mac_lookup_cfg_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_vlan_protocol_filters_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_proto_filters_mask_t parm;

	BDMF_TRACE_DBG("inside vlan_user_protocol_filters_get\n");

	if ((pa->ret = rdpa_vlan_protocol_filters_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_protocol_filters_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_proto_filters_mask_t)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_vlan_protocol_filters_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside vlan_user_protocol_filters_set\n");

	if ((pa->ret = rdpa_vlan_protocol_filters_set(pa->mo, (rdpa_proto_filters_mask_t)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_protocol_filters_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_vlan_discard_prty_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_discard_prty parm;

	BDMF_TRACE_DBG("inside vlan_user_discard_prty_get\n");

	if ((pa->ret = rdpa_vlan_discard_prty_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_discard_prty_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_discard_prty)))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_vlan_discard_prty_set(rdpa_ioctl_cmd_t *pa)
{

	BDMF_TRACE_DBG("inside vlan_user_discard_prty_set\n");

	if ((pa->ret = rdpa_vlan_discard_prty_set(pa->mo, (rdpa_discard_prty)(long)pa->parm)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_discard_prty_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

static int rdpa_user_vlan_stat_get(rdpa_ioctl_cmd_t *pa)
{
	rdpa_stat_tx_rx_valid_t  parm;

	BDMF_TRACE_DBG("inside vlan_user_stat_get\n");

	if ((pa->ret = rdpa_vlan_stat_get(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_stat_get failed, ret:%d\n", pa->ret);
	}

	if (copy_to_user((void *)(long)pa->ptr, (void *)&parm, sizeof(rdpa_stat_tx_rx_valid_t )))
	{
		BDMF_TRACE_ERR("failed to copy to user\n");
		return -1;
	}

	return 0;
}

static int rdpa_user_vlan_stat_set(rdpa_ioctl_cmd_t *pa)
{
	rdpa_stat_tx_rx_valid_t  parm;

	BDMF_TRACE_DBG("inside vlan_user_stat_set\n");

	if (copy_from_user((void *)&parm, (void *)(long)pa->ptr, sizeof(rdpa_stat_tx_rx_valid_t )))
	{
		BDMF_TRACE_ERR("failed to copy from user\n");
		return -1;
	}

	if ((pa->ret = rdpa_vlan_stat_set(pa->mo, &parm)))
	{
		BDMF_TRACE_DBG("rdpa_vlan_stat_set failed, ret:%d\n", pa->ret);
	}

	return 0;
}

long rdpa_vlan_ag_ioctl(unsigned int op, rdpa_ioctl_cmd_t *pa)
{
	int ret;

	switch (op){
		case RDPA_VLAN_DRV:
			ret = rdpa_user_vlan_drv(pa);
			break;

		case RDPA_VLAN_GET:
			ret = rdpa_user_vlan_get(pa);
			break;

		case RDPA_VLAN_NAME_GET:
			ret = rdpa_user_vlan_name_get(pa);
			break;

		case RDPA_VLAN_NAME_SET:
			ret = rdpa_user_vlan_name_set(pa);
			break;

		case RDPA_VLAN_VID_ENABLE_GET:
			ret = rdpa_user_vlan_vid_enable_get(pa);
			break;

		case RDPA_VLAN_VID_ENABLE_SET:
			ret = rdpa_user_vlan_vid_enable_set(pa);
			break;

		case RDPA_VLAN_INGRESS_FILTER_GET:
			ret = rdpa_user_vlan_ingress_filter_get(pa);
			break;

		case RDPA_VLAN_INGRESS_FILTER_SET:
			ret = rdpa_user_vlan_ingress_filter_set(pa);
			break;

		case RDPA_VLAN_INGRESS_FILTER_GET_NEXT:
			ret = rdpa_user_vlan_ingress_filter_get_next(pa);
			break;

		case RDPA_VLAN_MAC_LOOKUP_CFG_GET:
			ret = rdpa_user_vlan_mac_lookup_cfg_get(pa);
			break;

		case RDPA_VLAN_MAC_LOOKUP_CFG_SET:
			ret = rdpa_user_vlan_mac_lookup_cfg_set(pa);
			break;

		case RDPA_VLAN_PROTOCOL_FILTERS_GET:
			ret = rdpa_user_vlan_protocol_filters_get(pa);
			break;

		case RDPA_VLAN_PROTOCOL_FILTERS_SET:
			ret = rdpa_user_vlan_protocol_filters_set(pa);
			break;

		case RDPA_VLAN_DISCARD_PRTY_GET:
			ret = rdpa_user_vlan_discard_prty_get(pa);
			break;

		case RDPA_VLAN_DISCARD_PRTY_SET:
			ret = rdpa_user_vlan_discard_prty_set(pa);
			break;

		case RDPA_VLAN_STAT_GET:
			ret = rdpa_user_vlan_stat_get(pa);
			break;

		case RDPA_VLAN_STAT_SET:
			ret = rdpa_user_vlan_stat_set(pa);
			break;

		default:
			BDMF_TRACE_ERR("no such ioctl cmd: %u\n", op);
			ret = EINVAL;
		}

	return ret;
}
