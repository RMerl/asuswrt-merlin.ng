/*
   <:copyright-BRCM:2023:DUAL/GPL:standard
   
      Copyright (c) 2023 Broadcom 
      All Rights Reserved
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2, as published by
   the Free Software Foundation (the "GPL").
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   
   A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
   writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
   
   :>
 */

#include <rdpa_api.h>

static bdmf_object_handle ct_class;
static bdmf_object_handle rdpa_cpu_obj = NULL;

typedef struct
{
	uint32_t cnt_success;
	uint32_t cnt_removed;
	uint32_t cnt_overflow;
	uint32_t cnt_error;
} hwf_procfs_stat_t;

extern hwf_procfs_stat_t hwf_stat_g;

#define CPU_RX_HW_FIREWALL_QUEUE_ID  0   /* lowest priority queue */
#define RDPA_INVALID_KEY     (-1)
#define RDPA_INVALID_KEY_U64 (~0ULL)
/* 1000 multiplier is  used to match CT_LKP_POLICER_LEN in firmware */
#define PPS_TO_BPS   (8*1000)

static inline int hwf_hw_policer_add(bcm_hwf_ratelimiter_t *hwf_ratelimiter)
{
#if defined(POLICER_SUPPORT)
	bdmf_mattr_handle mattr = NULL;
	bdmf_number index = BDMF_INDEX_UNASSIGNED;
	bdmf_object_handle policer = NULL;
	rdpa_tm_policer_cfg_t policer_cfg = {0};
	int rc = 0;

	policer_cfg.type = rdpa_tm_policer_single_token_bucket;
	policer_cfg.commited_rate = hwf_ratelimiter->pps_rate * PPS_TO_BPS; 
	policer_cfg.committed_burst_size = hwf_ratelimiter->pps_burst_size * PPS_TO_BPS;
	policer_cfg.peak_rate = RDPA_INVALID_KEY_U64;
	policer_cfg.peak_burst_size = RDPA_INVALID_KEY;

	policer_cfg.dei_mode = RDPA_INVALID_KEY;
	policer_cfg.color_aware_enabled = RDPA_INVALID_KEY;
	policer_cfg.rl_overhead = RDPA_INVALID_KEY;

	/* allocate a new policer object from runner */
	mattr = bdmf_mattr_alloc(rdpa_policer_drv());
	if (!mattr)
	{
		rc = -ENOMEM;
		goto error;
	}

	if ((rc = rdpa_policer_cfg_set(mattr, &policer_cfg)))
	{
		pr_err("Failed to set RDPA policer config, rc=%d\n", rc);
		goto error;
	}

	if (bdmf_new_and_set(rdpa_policer_drv(), NULL, mattr, &policer))
	{
		rc = -EINVAL;
		goto error;
	}

	if ((rc = rdpa_policer_index_get(policer, &index)))
	{
		pr_err("Failed to get RDPA policer index, rc=%d\n", rc);
		goto error;
	}

	hwf_ratelimiter->index = index;
	bdmf_mattr_free(mattr);

	return rc;
error:
	if (mattr)
		bdmf_mattr_free(mattr);
	return rc;

#else
	pr_err("Runner Policer not supported\n")
	return -1;
#endif
}

static inline int hwf_hw_policer_modify(bcm_hwf_ratelimiter_t *hwf_ratelimiter)
{
#if defined(POLICER_SUPPORT)
	bdmf_number index = hwf_ratelimiter->index;
	bdmf_object_handle policer = NULL;
	rdpa_tm_policer_cfg_t policer_cfg = {0};
	int rc = 0;

	rc = rdpa_policer_get(index, &policer);
	if (rc)
	{
		pr_err("Failed to get RDPA policer object, index=%lld, rc=%d\n", index, rc);
		return rc;
	}

	rc = rdpa_policer_cfg_get(policer, &policer_cfg);
	if (rc) 
	{
		pr_err("Failed to get RDPA policer config, index=%lld, rc=%d\n", index, rc);
		bdmf_put(policer);
		return rc;
	}

	policer_cfg.type = rdpa_tm_policer_single_token_bucket;
	policer_cfg.commited_rate = hwf_ratelimiter->pps_rate * PPS_TO_BPS; 
	policer_cfg.committed_burst_size = hwf_ratelimiter->pps_burst_size * PPS_TO_BPS;
	policer_cfg.peak_rate = RDPA_INVALID_KEY_U64;
	policer_cfg.peak_burst_size = RDPA_INVALID_KEY;

	policer_cfg.dei_mode = RDPA_INVALID_KEY;
	policer_cfg.color_aware_enabled = RDPA_INVALID_KEY;
	policer_cfg.rl_overhead = RDPA_INVALID_KEY;

	rc = rdpa_policer_cfg_set(policer, &policer_cfg);
	if (rc)
	{
		pr_err("Failed to set RDPA policer config, rc=%d\n", rc);
		bdmf_put(policer);
		return rc;
	}
	return rc;
#else
	pr_err("Runner Policer not supported\n")
	return -1;
#endif
}

static inline int hwf_hw_policer_delete(bcm_hwf_ratelimiter_t *hwf_ratelimiter)
{
#if defined(POLICER_SUPPORT)
	bdmf_object_handle policer = NULL;

	if (!rdpa_policer_get(hwf_ratelimiter->index, &policer))
	{
		bdmf_destroy(policer);
	} else
	{
		pr_err("RDPA policer=%lld doesn't exist\n", hwf_ratelimiter->index);
		return -EEXIST;
	}

	return 0;
#else
	pr_err("Runner Policer not supported\n")
	return -1;
#endif
}

static inline int hwf_hw_policer_stat_get(bcm_hwf_ratelimiter_t *hwf_ratelimiter)
{
#if defined(POLICER_SUPPORT)
	bdmf_number index = hwf_ratelimiter->index;
	bdmf_object_handle policer = NULL;
	rdpa_tm_policer_stat_t stat = {};
	int rc = 0;

	rc = rdpa_policer_get(index, &policer);
	if (rc)
	{
		pr_err("Failed to get RDPA policer object, index=%lld, rc=%d\n", index, rc);
		return rc;
	}

	rc = rdpa_policer_stat_get(policer, &stat);
	if (rc)
	{
		pr_err("Failed to get RDPA policer stat, index=%lld, rc=%d\n", index, rc);
		return rc;
	}

	/* no need to accumulate in ratelimter as accumulation is alrerady done in rdpa */
	hwf_ratelimiter->drop_cnt = stat.red.packets;
	hwf_ratelimiter->hit_cnt = stat.green.packets + stat.yellow.packets;
	return 0;
#else
	pr_err("Runner Policer not supported\n")
	return -1;
#endif
}

static inline bdmf_object_handle _get_rdpa_cpu_obj(void)
{
	bdmf_object_handle _rdpa_cpu_obj = NULL;

	if (rdpa_cpu_get(rdpa_cpu_host, &_rdpa_cpu_obj)) 
	{
		pr_err("can't find rdpa cpu port\n");
		return NULL;
	}

	return _rdpa_cpu_obj;
}

static inline int hwf_hw_meter_add(bcm_hwf_ratelimiter_t *hwf_ratelimiter)
{
	rdpa_cpu_meter_cfg_t dummy;
	rdpa_cpu_meter_cfg_t meter_cfg = {};
	bdmf_index index = 0;
	int rc = BDMF_ERR_OK;
	/* Use dedicated hw_firewall_reason to meter firewall queue */
	rdpa_cpu_reason_index_t reason_index = {.reason = rdpa_cpu_rx_reason_hw_firewall_miss};
	rdpa_cpu_reason_cfg_t reason_cfg = {};

	if (rdpa_cpu_obj == NULL) 
	{
		return -EINVAL;
	}

	for (index = 0; index < RDPA_CPU_MAX_METERS; index++)
	{
		rc = rdpa_cpu_meter_cfg_get(rdpa_cpu_obj, index, &dummy);
		if (rc == BDMF_ERR_NOENT)
			break; /* Found available slot */
	}
	if (index == RDPA_CPU_MAX_METERS)
	{
		pr_err("can't find free meter object\n");
		return -ENOMEM;
	}
	pr_debug("found free meter id %ld\n", index);

	meter_cfg.sir = hwf_ratelimiter->pps_rate; 
	meter_cfg.burst_size = hwf_ratelimiter->pps_burst_size;

	rc = rdpa_cpu_meter_cfg_set(rdpa_cpu_obj, index, &meter_cfg);
	if (rc)
	{
		pr_err("can't configure meter object\n");
		return -EINVAL;
	}

	/* Associate with FIREWALL queue */
	rc = rdpa_cpu_reason_cfg_get(rdpa_cpu_obj, &reason_index, &reason_cfg);
	if (rc)
	{
		pr_err("failed to get CPU RX reason cfg, rc %d\n", rc);
		goto assoc_error;
	}

	reason_cfg.meter = index;
	rc = rdpa_cpu_reason_cfg_set(rdpa_cpu_obj, &reason_index, &reason_cfg);
	if (rc)
	{
		pr_err("failed to set/associate CPU RX reason cfg, rc %d\n", rc);
		goto assoc_error;
	}

	hwf_ratelimiter->index = index;
	return 0;

assoc_error:
	/* rollback meter selection */ 
	meter_cfg.sir = 0;
	meter_cfg.burst_size = 0;

	rc = rdpa_cpu_meter_cfg_set(rdpa_cpu_obj, index, &meter_cfg);
	if (rc) {
		pr_err("failed to rollback hwf_meter config, rc %d\n", rc);
		return rc;
	}

	hwf_ratelimiter->index = BDMF_INDEX_UNASSIGNED;
	return -ENOMEM;    
}

static inline int hwf_hw_meter_modify(bcm_hwf_ratelimiter_t *hwf_ratelimiter)
{
	rdpa_cpu_meter_cfg_t meter_cfg = {};
	int rc = 0;

	if (rdpa_cpu_obj == NULL) 
	{
		return -EINVAL;
	}

	meter_cfg.sir = hwf_ratelimiter->pps_rate;
	meter_cfg.burst_size = hwf_ratelimiter->pps_burst_size;

	rc = rdpa_cpu_meter_cfg_set(rdpa_cpu_obj, hwf_ratelimiter->index, &meter_cfg);
	if (rc) {
		pr_err("failed to modify hwf_meter config, rc %d\n", rc);
	}
	return rc;
}

static inline int hwf_hw_meter_delete(bcm_hwf_ratelimiter_t *hwf_ratelimiter)
{
	rdpa_cpu_meter_cfg_t meter_cfg = {};
	rdpa_cpu_reason_cfg_t reason_cfg = {};
	rdpa_cpu_reason_index_t reason_index = {.reason = rdpa_cpu_rx_reason_hw_firewall_miss};
	int rc = 0;

	if (rdpa_cpu_obj == NULL)
	{
		return -EINVAL;
	}

	/* detach meter from reason */ 
	reason_cfg.meter = BDMF_INDEX_UNASSIGNED;
	rc = rdpa_cpu_reason_cfg_set(rdpa_cpu_obj, &reason_index, &reason_cfg);
	if (rc)
	{
		pr_err("failed to unset CPU RX reason cfg, rc %d\n", rc);
		return rc;
	}

	/* remove meter by zero out config */
	meter_cfg.sir = 0;
	meter_cfg.burst_size = 0;

	rc = rdpa_cpu_meter_cfg_set(rdpa_cpu_obj, hwf_ratelimiter->index, &meter_cfg);
	if (rc) {
		pr_err("failed to zero out hwf_meter config, rc %d\n", rc);
		return rc;
	}

	hwf_ratelimiter->index = BDMF_INDEX_UNASSIGNED;
	return rc;
}

static inline int hwf_hw_cpu_rxq_stat_get(uint64_t *drop_counter, uint64_t *rx_counter)
{
	rdpa_cpu_rx_stat_t rxstat;

	if (rdpa_cpu_obj == NULL)
	{
		return -EINVAL;
	}

	/* Get DDOS stats from cpu_rx_host HW_FIREWALL_queue */
	*drop_counter = 0;
	*rx_counter = 0;
	if (rdpa_cpu_rxq_stat_get(rdpa_cpu_obj, CPU_RX_HW_FIREWALL_QUEUE_ID, &rxstat) == 0)
	{
		*drop_counter = rxstat.dropped;
		*rx_counter = rxstat.received;
		return 0;
	}
	else
		return -1;
}

/* 
 * retrieve ct_lkp miss counter 
 */
static inline int hwf_hw_ct_miss_counter_get(bdmf_number *miss_counter)
{
	int ret;

	ret = rdpa_ct_class_ct_miss_counter_get(ct_class, miss_counter);
	if (ret) {
		pr_err("%s: hw firewall get miss_counter failed ret = %d.\n", __func__, ret);
		return ret;
	}
	return 0;
}

/* 
 * retrieve DDOS cpu meter drop counter 
 */
static inline int hwf_hw_meter_stat_get(bcm_hwf_ratelimiter_t *hwf_ratelimiter)
{
	int ret;
	bdmf_number counter = 0; /*TODO fix bdmf_bumber in int64 instead of uint64*/

	/*TODO better to get stats basd on meter id instead of from CT class */
	ret = rdpa_ct_class_ratelimit_drop_counter_get(ct_class, &counter);
	if (ret) {
		pr_err("%s: hw firewall get ratelimit_drop_counter failed ret = %d.\n", __func__, ret);
		return ret;
	}
	hwf_ratelimiter->drop_cnt += counter; /*not accumualted in rdpa */

#if 0
	{/* there is no hit cnt for meter, use ct_miss_counter*/
		counter = 0;
		ret = hwf_hw_ct_miss_counter_get(&counter);
		if (ret == 0)
			hwf_ratelimiter->hit_cnt = counter; /*already accumulated */
	}
#endif
	return 0;
}

static inline int hwf_hw_ratelimiter_add(bcm_hwf_ratelimiter_t *hwf_ratelimiter)
{
	if (hwf_ratelimiter->type == hwfctl_ratelimit_wan_miss_all)
		return hwf_hw_meter_add(hwf_ratelimiter);
	else
		return hwf_hw_policer_add(hwf_ratelimiter);
}

static inline int hwf_hw_ratelimiter_modify(bcm_hwf_ratelimiter_t *hwf_ratelimiter)
{
	if (hwf_ratelimiter->type == hwfctl_ratelimit_wan_miss_all)
		return hwf_hw_meter_modify(hwf_ratelimiter);
	else
		return hwf_hw_policer_modify(hwf_ratelimiter);

}

static inline int hwf_hw_ratelimiter_delete(bcm_hwf_ratelimiter_t *hwf_ratelimiter)
{
	if (hwf_ratelimiter->type == hwfctl_ratelimit_wan_miss_all)
		return hwf_hw_meter_delete(hwf_ratelimiter);
	else
		return hwf_hw_policer_delete(hwf_ratelimiter);
}

static inline int hwf_hw_ratelimiter_get_stats(bcm_hwf_ratelimiter_t *hwf_ratelimiter)
{

	if (hwf_ratelimiter->type == hwfctl_ratelimit_wan_miss_all)
		return hwf_hw_meter_stat_get(hwf_ratelimiter);
	else
	    return hwf_hw_policer_stat_get(hwf_ratelimiter);
}

static inline void ct_tuple_to_hw_key(const struct nf_conntrack_tuple *key_tuple, rdpa_ip_flow_key_t *hw_key)
{

	/* CT stores keys in Network order, rdpa API is epexting host order */

	if (key_tuple->src.l3num == AF_INET) {
		hw_key->src_ip.addr.ipv4 = ntohl(key_tuple->src.u3.ip);
		hw_key->dst_ip.addr.ipv4 = ntohl(key_tuple->dst.u3.ip);
		hw_key->src_ip.family = bdmf_ip_family_ipv4;
		hw_key->dst_ip.family = bdmf_ip_family_ipv4;
	} else if (key_tuple->src.l3num == AF_INET6) {
		memcpy(&hw_key->src_ip.addr.ipv6, key_tuple->src.u3.ip6, sizeof(hw_key->src_ip.addr.ipv6));
		memcpy(&hw_key->dst_ip.addr.ipv6, key_tuple->dst.u3.ip6, sizeof(hw_key->dst_ip.addr.ipv6));
		hw_key->src_ip.family = bdmf_ip_family_ipv6;
		hw_key->dst_ip.family = bdmf_ip_family_ipv6;
	} else {
		pr_warn("%s: unsupported l3 protoccol %x\n", __func__, ntohs(key_tuple->src.l3num));
	}

	hw_key->src_port = ntohs(key_tuple->src.u.all);
	hw_key->dst_port = ntohs(key_tuple->dst.u.all);
	hw_key->prot = key_tuple->dst.protonum;
}

void exp_tuple_to_hw_key(const struct nf_conntrack_tuple *key_tuple, rdpa_ip_flow_key_t *hw_key)
{
	/* only set DSTIP + DSTPORT + L4PROTO, even when we have other fields */
	if (key_tuple->src.l3num == AF_INET) {
		hw_key->dst_ip.addr.ipv4 = ntohl(key_tuple->dst.u3.ip);
		hw_key->dst_ip.family = bdmf_ip_family_ipv4;
	} else if (key_tuple->src.l3num == AF_INET6) {
		memcpy(&hw_key->dst_ip.addr.ipv6, key_tuple->dst.u3.ip6, sizeof(hw_key->dst_ip.addr.ipv6));
		hw_key->dst_ip.family = bdmf_ip_family_ipv6;
	} else {
		pr_warn("%s: unsupported l3 protoccol %x\n", __func__, ntohs(key_tuple->src.l3num));
	}

	hw_key->dst_port = ntohs(key_tuple->dst.u.all);
	hw_key->prot = key_tuple->dst.protonum;

}

static inline bool hwf_hw_node_lookup(rdpa_ct_entry_info_t *ct_node, bdmf_index *index)
{
	int ret;

	ret = rdpa_ct_class_flow_find(ct_class, index, ct_node);
	if (ret == 0)
		return true;
	pr_notice("%s: ret =%d\n", __func__, ret);
	return false;
}

static inline long  hwf_hw_node_add(bcm_hwf_obj_t *hwf_obj, bcm_hwf_ratelimiter_t *hwf_ratelimiter)
{
	rdpa_ct_entry_info_t ct_node = {};
	bdmf_index index = HWF_INVALID_HW_IDX;
	int ret = 0;

	if (unlikely(hwf_obj->is_exp))
		exp_tuple_to_hw_key(&hwf_obj->tuple, &ct_node.key);
	else
		ct_tuple_to_hw_key(&hwf_obj->tuple, &ct_node.key);

#if defined(POLICER_SUPPORT)
	if (unlikely(hwf_obj->is_ratelimiter))
	{
		if (hwf_ratelimiter == NULL || hwf_ratelimiter->index < 0) {
			pr_warn("%s: Failed to add entry\n", __func__);
			return HWF_INVALID_HW_IDX;
		}
		ct_node.policer_id = hwf_ratelimiter->index;
		ct_node.policer_en = 1; 
		ret = rdpa_policer_get(hwf_ratelimiter->index, &ct_node.policer_obj);
	}
#endif

	ret = ret ? ret : rdpa_ct_class_flow_add(ct_class, &index, &ct_node);
	if (ret) {
		pr_warn("%s: Failed to add entry\n", __func__);
		hwf_stat_g.cnt_error ++;
		return HWF_INVALID_HW_IDX;
	}
	else if (index < 0)
	{
		pr_notice("%s: hash table collision, failed to add entry\n", __func__);
		hwf_stat_g.cnt_overflow ++;
		return HWF_INVALID_HW_IDX;
	}
	hwf_stat_g.cnt_success ++;
	return index;
}
static inline int hwf_hw_node_delete(bcm_hwf_obj_t *hwf_obj)
{
	int ret;
	bdmf_index index = hwf_obj->hwid;

	ret = rdpa_ct_class_flow_delete(ct_class, index);
	if (ret)
		pr_warn("%s: Failed to delete entry\n", __func__);

	hwf_stat_g.cnt_removed ++;
	return ret;
}


/* 
 * retrieve ct_3_tuple lookup enable bit 
 */
static inline int hwf_hw_config_ct_expect_enable_get(int *enable)
{
	int ret;

	ret = rdpa_ct_class_enable_ct_expect_get(ct_class, (bdmf_boolean *)enable);
	if (ret) {
		pr_err("%s: hw firewall get ct_expect enable 3-tuple status failed.\n", __func__);
		return ret;
	}
	return 0;
}

static inline int hwf_hw_config_ct_expect_enable(int enable)
{
	int ret;

	ret = rdpa_ct_class_enable_ct_expect_set(ct_class, enable);
	if (ret) {
		pr_err("%s: HW firewall enable 3-tuple lookup failed.\n", __func__);
		return ret;
	}
	return 0;
}

/* 
 * retrieve ct_global/ct_5_tuple lookup enable bit 
 */
static inline int hwf_hw_config_enable_get(int *enable)
{
	int ret;

	ret = rdpa_ct_class_enable_get(ct_class, (bdmf_boolean *)enable);
	if (ret) {
		pr_err("%s: hw firewall get enable status failed.\n", __func__);
		return ret;
	}
	return 0;
}

static int hwf_hw_config_enable(int enable)
{
	int ret;

	ret = rdpa_ct_class_enable_set(ct_class, enable);
	if (ret) {
		pr_err("%s: HW firewall enabled failed.\n", __func__);
		return ret;
	}
	return 0;
}

static int __init hwf_hw_init(void)
{
	int ret;

	BDMF_MATTR_ALLOC(ct_class_attrs, rdpa_ct_class_drv());

	rdpa_cpu_obj = _get_rdpa_cpu_obj();
	if (rdpa_cpu_obj == NULL) 
	{
		pr_err("hwf_init: failed to get rdpa cpu host\n");
		return -EINVAL;
	}

	ret = bdmf_new_and_set(rdpa_ct_class_drv(), NULL, ct_class_attrs, &ct_class);
	if (ret < 0) {
		pr_err("hwf_init: bdmf ct_class creation failed\n");
		return ret;
	}

	ret = rdpa_ct_class_enable_set(ct_class, 1);
	if (ret) {
		pr_err("%s: HW firewall enable failed.\n", __func__);
		bdmf_destroy(ct_class);
		return ret;
	}

	return ret;
}

static int __exit hwf_hw_exit(void)
{
	/* disable firewall & flush the table */
	bdmf_put(rdpa_cpu_obj);
	rdpa_ct_class_enable_set(ct_class, 0);
	rdpa_ct_class_flush_set(ct_class, 1);
	bdmf_destroy(ct_class);
	return 0;
}
