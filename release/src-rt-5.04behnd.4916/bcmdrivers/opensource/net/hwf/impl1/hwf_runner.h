/*
   <:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom
   All Rights Reserved

   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.

   :>
 */

#include <rdpa_api.h>

static bdmf_object_handle ct_class;

static inline void ct_tuple_to_hw_key(const struct nf_conntrack_tuple *key_tuple, rdpa_ip_flow_key_t *hw_key)
{

	/* CT stores keys in Network order, rdpa API is epexting host order */

	if (key_tuple->src.l3num == AF_INET) {
		hw_key->src_ip.addr.ipv4 = ntohl(key_tuple->src.u3.ip);
		hw_key->dst_ip.addr.ipv4 = ntohl(key_tuple->dst.u3.ip);
		hw_key->src_ip.family =	bdmf_ip_family_ipv4;
		hw_key->dst_ip.family =	bdmf_ip_family_ipv4;
	} else if (key_tuple->src.l3num == AF_INET6) {
		memcpy(&hw_key->src_ip.addr.ipv6, key_tuple->src.u3.ip6, sizeof(hw_key->src_ip.addr.ipv6));
		memcpy(&hw_key->dst_ip.addr.ipv6, key_tuple->dst.u3.ip6, sizeof(hw_key->dst_ip.addr.ipv6));
		hw_key->src_ip.family =	bdmf_ip_family_ipv6;
		hw_key->dst_ip.family =	bdmf_ip_family_ipv6;
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
		hw_key->dst_ip.family =	bdmf_ip_family_ipv4;
	} else if (key_tuple->src.l3num == AF_INET6) {
		memcpy(&hw_key->dst_ip.addr.ipv6, key_tuple->dst.u3.ip6, sizeof(hw_key->dst_ip.addr.ipv6));
		hw_key->dst_ip.family =	bdmf_ip_family_ipv6;
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

static inline long  hwf_hw_node_add(bcm_hwf_obj_t *hwf_obj)
{
	rdpa_ct_entry_info_t ct_node = {};
	bdmf_index index = HWF_INVALID_HW_IDX;
	int ret;

	if (unlikely(hwf_obj->is_exp))
		exp_tuple_to_hw_key(&hwf_obj->tuple, &ct_node.key);
	else
		ct_tuple_to_hw_key(&hwf_obj->tuple, &ct_node.key);

	ret = rdpa_ct_class_flow_add(ct_class, &index, &ct_node);
	if (ret) {
		pr_warn("%s: Failed to add entry\n", __func__);
		return HWF_INVALID_HW_IDX;
	}
	return index;
}
static inline int hwf_hw_node_delete(bcm_hwf_obj_t *hwf_obj)
{
	int ret;
	bdmf_index index = hwf_obj->hwid;

	ret = rdpa_ct_class_flow_delete(ct_class, index);
	if (ret)
		pr_warn("%s: Failed to delete entry\n", __func__);

	return ret;
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
	rdpa_ct_class_enable_set(ct_class, 0);
	rdpa_ct_class_flush_set(ct_class, 1);
	bdmf_destroy(ct_class);
	return 0;
}
