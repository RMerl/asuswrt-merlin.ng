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

/*
 *******************************************************************************
 * File Name : fpi.c
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/cdev.h>
#include <linux/if_vlan.h>
#include <linux/blog.h>
#include <linux/bcm_skb_defines.h>
#include <linux/bcm_netdev_path.h>
#include <linux/bcm_log_mod.h>
#include <linux/bcm_log.h>
#include <net/gre.h>

#include "idx_pool_util.h"
#include "pktHdr.h"
#include "fpi.h"
#include "fpi_hw.h"
#include "fpi_wlan.h"
#include "fpi_ioctl.h"
#include "fpi_spu.h"

typedef struct {
	uint32_t hw_handle;
	fpi_flow_t flow;
	fpi_stat_t stat;
	fpi_wlan_egress_info_t wl_info;
} fpi_flow_entry_t;

typedef struct {
	uint8_t mac[ETH_ALEN];
	int ref_cnt;
} fpi_ap_mac_entry_t;

typedef struct {
	struct class *class;
	struct device *device;
	struct cdev cdev;
	int major;
	int max_flow;
	atomic_t num_flow;
	fpi_mode_t mode;
	uint8_t def_prio;
	fpi_gre_mode_t gre_mode;
	bool l2lkp_on_etype;
	bool lkp_enable;
	uint16_t etype_for_l2lkp;
	IdxPool_t idx_pool;
	fpi_flow_entry_t *table_p;
	fpi_ap_mac_entry_t mac_table[FPI_MAX_AP_MAC];
	fpi_get_wl_metadata_func_t wl_get_metadata_func_p;
	fpi_get_wl_metadata_func_t dhd_get_metadata_func_p;
} fpi_db_t;

/* Handle info.  Embed flow mode at [31:28], index = [27:0] */
#define FPI_HANDLE_MODE_SHIFT	28
#define FPI_HANDLE_INDEX_MASK	((1 << 28) - 1)
#define FPI_HANDLE(_mode, _index) (_index + (_mode << FPI_HANDLE_MODE_SHIFT))
#define FPI_HANDLE_GET_INDEX(_handle)	(_handle & FPI_HANDLE_INDEX_MASK)
#define FPI_HANDLE_GET_MODE(_handle)	(_handle >> FPI_HANDLE_MODE_SHIFT)

static fpi_hw_info_t g_hw_info;
static fpi_db_t fpi_db;

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
DEFINE_SPINLOCK(fpi_lock);
#define FPI_LOCK_BH()	spin_lock_bh(&fpi_lock)
#define FPI_UNLOCK_BH()	spin_unlock_bh(&fpi_lock)
#else
#define FPI_LOCK_BH()	do {} while (0)
#define FPI_UNLOCK_BH()	do {} while (0)
#endif

static int local_db_add_flow(fpi_flow_t *flow_p, uint32_t hw_handle,
			     fpi_wlan_egress_info_t *wl_info_t,
			     uint32_t *fpi_handle_p)
{
	int index;
	fpi_flow_entry_t *flow_entry_p;

	index = idx_pool_get_index(&fpi_db.idx_pool);
	if (index < 0)
		return -ENOSPC;

	flow_entry_p = &fpi_db.table_p[index];
	flow_entry_p->hw_handle = hw_handle;
	memcpy(&flow_entry_p->flow, flow_p, sizeof(fpi_flow_t));
	memset(&flow_entry_p->stat, 0, sizeof(fpi_stat_t));
	memcpy(&flow_entry_p->wl_info, wl_info_t,
	       sizeof(fpi_wlan_egress_info_t));
	atomic_inc(&fpi_db.num_flow);

	*fpi_handle_p = FPI_HANDLE(flow_p->key.mode, index);

	return 0;
}

static int local_db_delete_flow(uint32_t fpi_handle)
{
	int index = FPI_HANDLE_GET_INDEX(fpi_handle);

	if (index > fpi_db.max_flow)
		return -EINVAL;

	memset(&fpi_db.table_p[index], 0,
	       sizeof(fpi_flow_entry_t));
	idx_pool_return_index(&fpi_db.idx_pool, index);

	atomic_dec(&fpi_db.num_flow);

	return 0;
}

static fpi_flow_entry_t *local_db_get_flow(uint32_t fpi_handle)
{
	int index = FPI_HANDLE_GET_INDEX(fpi_handle);
	int mode = FPI_HANDLE_GET_MODE(fpi_handle);

	if (index > fpi_db.max_flow)
		return NULL;

	if (mode != fpi_db.table_p[index].flow.key.mode)
		return NULL;

	return &fpi_db.table_p[index];
}

static fpi_flow_entry_t *local_db_get_flow_by_index(int index)
{
	if ((index > fpi_db.max_flow) || (index < 0))
		return NULL;

	return &fpi_db.table_p[index];
}

static int local_db_get_hw_handle(uint32_t fpi_handle, uint32_t *hw_handle_p)
{
	fpi_flow_entry_t *flow_entry_p;

	flow_entry_p = local_db_get_flow(fpi_handle);
	if (flow_entry_p == NULL)
		return -EINVAL;

	*hw_handle_p = flow_entry_p->hw_handle;
	return 0;
}

#define FPI_FIELD_COMPARE(_name, _field)	\
	if (_name##_key_1_p->_field != _name##_key_2_p->_field)	\
		return false

static bool fpi_key_compare(fpi_key_t *key_1_p, fpi_key_t *key_2_p)
{
	fpi_l2_key_t *l2_key_1_p, *l2_key_2_p;
	fpi_l3l4_key_t *l3l4_key_1_p, *l3l4_key_2_p;

	if (key_1_p->mode != key_2_p->mode)
		return false;

	switch (key_1_p->mode) {
	case fpi_mode_l2:
		l2_key_1_p = &key_1_p->l2_key;
		l2_key_2_p = &key_2_p->l2_key;

		FPI_FIELD_COMPARE(l2, ingress_device_ptr);

		if (memcmp(l2_key_1_p->src_mac, l2_key_2_p->src_mac, ETH_ALEN))
			return false;

		if (memcmp(l2_key_1_p->dst_mac, l2_key_2_p->dst_mac, ETH_ALEN))
			return false;

		FPI_FIELD_COMPARE(l2, eth_type);
		FPI_FIELD_COMPARE(l2, vtag_num);
		FPI_FIELD_COMPARE(l2, packet_priority);

		return true;

	case fpi_mode_l3l4:
		l3l4_key_1_p = &key_1_p->l3l4_key;
		l3l4_key_2_p = &key_2_p->l3l4_key;

		FPI_FIELD_COMPARE(l3l4, ingress_device_ptr);
		FPI_FIELD_COMPARE(l3l4, vtag_num);
		FPI_FIELD_COMPARE(l3l4, is_ipv6);

		if (l3l4_key_1_p->is_ipv6) {
			if (memcmp(l3l4_key_1_p->src_ip, l3l4_key_2_p->src_ip,
				   16) != 0)
				return false;
			if (memcmp(l3l4_key_1_p->dst_ip, l3l4_key_2_p->dst_ip,
				   16) != 0)
				return false;

		} else {
			FPI_FIELD_COMPARE(l3l4, src_ip[0]);
			FPI_FIELD_COMPARE(l3l4, dst_ip[0]);
		}

		FPI_FIELD_COMPARE(l3l4, l4_proto);
		FPI_FIELD_COMPARE(l3l4, src_port);
		FPI_FIELD_COMPARE(l3l4, dst_port);
		FPI_FIELD_COMPARE(l3l4, esp_spi_mode);
		FPI_FIELD_COMPARE(l3l4, esp_spi);
		FPI_FIELD_COMPARE(l3l4, packet_priority);

		return true;

	default:
		BCM_LOG_ERROR(BCM_LOG_ID_FPI, "Invalid mode[%u]",
			      key_1_p->mode);
		return false;
	}

	return false;
}

static int local_db_find_flow_by_key(fpi_key_t *key_p, uint32_t *fpi_handle_p)
{
	uint32_t curr_index;
	int rc;
	fpi_flow_entry_t *flow_entry_p;

	rc = idx_pool_first_in_use(&fpi_db.idx_pool, &curr_index);
	if (rc < 0)
		return -ENOENT;

	do {
		flow_entry_p = &fpi_db.table_p[curr_index];
		if (fpi_key_compare(key_p, &flow_entry_p->flow.key)) {
			*fpi_handle_p = FPI_HANDLE(key_p->mode, curr_index);
			return 0;
		}
	} while (idx_pool_next_in_use(&fpi_db.idx_pool, curr_index,
				      &curr_index) == 0);

	return -ENOENT;
}

int fpi_set_mode(fpi_mode_t mode)
{
	int rc;

	if (unlikely(g_hw_info.registered == 0))
		return -EPERM;

	if (mode >= fpi_mode_num)
		return -EINVAL;

	fpi_db.mode = mode;
	rc = g_hw_info.set_mode(mode);

	return rc;
}
EXPORT_SYMBOL(fpi_set_mode);

int fpi_get_mode(fpi_mode_t *mode_p)
{
	fpi_mode_t mode_hw;
	int rc = 0;

	if (g_hw_info.get_mode != NULL) {
		rc = g_hw_info.get_mode(&mode_hw);
		if (rc) {
			BCM_LOG_ERROR(BCM_LOG_ID_FPI,
				      "Fail to obtain mode from HW");
			return rc;
		}

		if (fpi_db.mode != mode_hw) {
			BCM_LOG_ERROR(BCM_LOG_ID_FPI,
				      "Mode obtained from HW (%d) does not "
				      "match with SW-maintained (%d). Reset "
				      "SW-maintained to HW one.", mode_hw,
				      fpi_db.mode);
			fpi_db.mode = mode_hw;
		}
	}
	*mode_p = fpi_db.mode;
	return 0;
}
EXPORT_SYMBOL(fpi_get_mode);

int fpi_set_default_priority(uint8_t prio)
{
	int rc;

	if (unlikely(g_hw_info.registered == 0))
		return -EPERM;

	if (prio > 8)
		return -EINVAL;

	fpi_db.def_prio = prio;
	rc = g_hw_info.set_default_priority(prio);

	return rc;
}
EXPORT_SYMBOL(fpi_set_default_priority);

int fpi_get_default_priority(uint8_t *prio_p)
{
	uint8_t prio_hw;
	int rc = 0;

	if (g_hw_info.get_default_priority != NULL) {
		rc = g_hw_info.get_default_priority(&prio_hw);
		if (rc) {
			BCM_LOG_ERROR(BCM_LOG_ID_FPI,
				      "Fail to get default priority from HW");
			return rc;
		}

		if (fpi_db.def_prio != prio_hw) {
			BCM_LOG_ERROR(BCM_LOG_ID_FPI,
				      "Default Priority from HW (%d) does not "
				      "match with SW-maintained (%d). Reset "
				      "SW-maintained to HW one.", prio_hw,
				      fpi_db.def_prio);
			fpi_db.def_prio = prio_hw;
		}
	}
	*prio_p = fpi_db.def_prio;
	return 0;
}
EXPORT_SYMBOL(fpi_get_default_priority);

static int fpi_key_to_blog(fpi_key_t *key_p, Blog_t *blog_p)
{
	fpi_l2_key_t *l2_key_p;
	fpi_l3l4_key_t *l3l4_key_p;
	int i;

	blog_p->fpi_mode = key_p->mode;

	switch (key_p->mode) {
	case fpi_mode_l2:
		l2_key_p = &key_p->l2_key;

		blog_p->rx_dev_p = (void *)(uintptr_t)
				l2_key_p->ingress_device_ptr;
		blog_p->rx.info.bmap.PLD_L2 = 1;
		blog_p->tx.info.bmap.PLD_L2 = 1;
		memcpy(&blog_p->rx.l2hdr[6], l2_key_p->src_mac, ETH_ALEN);
		memcpy(&blog_p->rx.l2hdr[0], l2_key_p->dst_mac, ETH_ALEN);
		memcpy(&blog_p->tx.l2hdr[6], l2_key_p->src_mac, ETH_ALEN);
		memcpy(&blog_p->tx.l2hdr[0], l2_key_p->dst_mac, ETH_ALEN);
		if (l2_key_p->vtag_num) {
			blog_p->rx.l2hdr[12] = BLOG_ETH_P_8021Q >> 8;
			blog_p->rx.l2hdr[13] = BLOG_ETH_P_8021Q & 0xff;
			blog_p->tx.l2hdr[12] = BLOG_ETH_P_8021Q >> 8;
			blog_p->tx.l2hdr[13] = BLOG_ETH_P_8021Q & 0xff;
		} else {
			blog_p->rx.l2hdr[12] = (l2_key_p->eth_type >> 8) & 0xff;
			blog_p->rx.l2hdr[13] = l2_key_p->eth_type & 0xff;
			blog_p->tx.l2hdr[12] = (l2_key_p->eth_type >> 8) & 0xff;
			blog_p->tx.l2hdr[13] = l2_key_p->eth_type & 0xff;
		}
		blog_p->eth_type = htons(l2_key_p->eth_type);
		blog_p->vtag_num = l2_key_p->vtag_num;
		blog_p->tx.tuple.tos = blog_p->rx.tuple.tos =
				BLOG_IPDSCP2TOS(l2_key_p->packet_priority);

		if (l2_key_p->eth_type == ETH_P_IP)
			blog_p->l2_ipv4 = 1;
		else if (l2_key_p->eth_type == ETH_P_IPV6) {
			blog_p->l2_ipv6 = 1;
			PKT_IPV6_SET_TOS_WORD(blog_p->tupleV6.word0,
				BLOG_IPDSCP2TOS(l2_key_p->packet_priority));
		} else if ((fpi_db.l2lkp_on_etype == true) &&
			   (key_p->l2_key.eth_type == fpi_db.etype_for_l2lkp)) {
			blog_p->rx.info.bmap.DEL_L2 = 1;
			blog_p->tx.info.bmap.PLD_L2 = 0;
			blog_p->tx.tuple.tos = blog_p->rx.tuple.tos = 0;
		}

		break;

	case fpi_mode_l3l4:
		l3l4_key_p = &key_p->l3l4_key;

		blog_p->rx_dev_p = (void *)(uintptr_t)
				l3l4_key_p->ingress_device_ptr;
		blog_p->vtag_num = l3l4_key_p->vtag_num;

		if (l3l4_key_p->is_ipv6) {
			blog_p->rx.info.bmap.PLD_IPv6 = 1;
			blog_p->rx.info.bmap.PLD_IPv4 = 0;
			blog_p->tx.info.bmap.PLD_IPv6 = 1;
			blog_p->tx.info.bmap.PLD_IPv4 = 0;
			for (i = 0; i < 4; i++) {
				blog_p->tupleV6.saddr.p32[i] =
						htonl(l3l4_key_p->src_ip[i]);
				blog_p->tupleV6.daddr.p32[i] =
						htonl(l3l4_key_p->dst_ip[i]);
			}
			/* TODO! the current Blog_t doesn't support ESP
			 * for IPv6 yet
			 */
			blog_p->tupleV6.port.source =
					htons(l3l4_key_p->src_port);
			blog_p->tupleV6.port.dest =
					htons(l3l4_key_p->dst_port);
			blog_p->del_tupleV6.port.source =
					htons(l3l4_key_p->src_port);
			blog_p->del_tupleV6.port.dest =
					htons(l3l4_key_p->dst_port);
			PKT_IPV6_SET_TOS_WORD(blog_p->tupleV6.word0,
				BLOG_IPDSCP2TOS(l3l4_key_p->packet_priority));
		} else {
			blog_p->rx.info.bmap.PLD_IPv6 = 0;
			blog_p->rx.info.bmap.PLD_IPv4 = 1;
			blog_p->tx.info.bmap.PLD_IPv6 = 0;
			blog_p->tx.info.bmap.PLD_IPv4 = 1;
			blog_p->rx.tuple.saddr = htonl(l3l4_key_p->src_ip[0]);
			blog_p->rx.tuple.daddr = htonl(l3l4_key_p->dst_ip[0]);
			blog_p->tx.tuple.saddr = htonl(l3l4_key_p->src_ip[0]);
			blog_p->tx.tuple.daddr = htonl(l3l4_key_p->dst_ip[0]);
			blog_p->rx.tuple.port.source =
					htons(l3l4_key_p->src_port);
			blog_p->rx.tuple.port.dest =
					htons(l3l4_key_p->dst_port);
			blog_p->tx.tuple.port.source =
					htons(l3l4_key_p->src_port);
			blog_p->tx.tuple.port.dest =
					htons(l3l4_key_p->dst_port);
		}
		blog_p->key.protocol = l3l4_key_p->l4_proto;
		blog_p->rx.tuple.tos = l3l4_key_p->packet_priority << 2;
		blog_p->tx.tuple.tos = blog_p->rx.tuple.tos;

		if ((l3l4_key_p->l4_proto == IPPROTO_GRE) &&
		    (fpi_db.gre_mode == fpi_gre_mode_proprietary)) {
			if ((l3l4_key_p->src_port != 0) ||
			    (l3l4_key_p->dst_port != 0))
				return -EINVAL;

			/* purposely set the following to 0, so this
			 * flow won't trigger decapsulation
			 */
			blog_p->rx.info.bmap.PLD_IPv6 = 0;
			blog_p->rx.info.bmap.PLD_IPv4 = 0;

			blog_p->rx.info.bmap.GRE = 1;
			blog_p->grerx.gre_flags.u16 = 0;
			blog_p->grerx.hlen = gre_calc_hlen(htons(
						blog_p->grerx.gre_flags.u16));
			if (l3l4_key_p->is_ipv6) {
				blog_p->rx.info.bmap.DEL_IPv6 = 1;
				memcpy(blog_p->del_tupleV6.saddr.p8,
				       blog_p->tupleV6.saddr.p8, 16);
				memcpy(blog_p->del_tupleV6.daddr.p8,
				       blog_p->tupleV6.daddr.p8, 16);
				blog_p->del_tupleV6.rx_tos =
						blog_p->rx.tuple.tos;
				memset(blog_p->tupleV6.saddr.p8, 0, 16);
				memset(blog_p->tupleV6.daddr.p8, 0, 16);
			} else {
				blog_p->rx.info.bmap.DEL_IPv4 = 1;
				blog_p->delrx_tuple.saddr =
						blog_p->rx.tuple.saddr;
				blog_p->delrx_tuple.daddr =
						blog_p->rx.tuple.daddr;
				blog_p->delrx_tuple.tos = blog_p->rx.tuple.tos;
				blog_p->rx.tuple.saddr = 0;
				blog_p->rx.tuple.daddr = 0;
				blog_p->tx.tuple.saddr = 0;
				blog_p->tx.tuple.daddr = 0;
			}

			/* L2 GRE specific START */
			blog_p->rx.info.bmap.GRE_ETH = 1;
			/* L2 GRE specific END */
		}
		break;

	default:
		BCM_LOG_ERROR(BCM_LOG_ID_FPI, "Invalid mode[%u]", key_p->mode);
		return -EINVAL;
	}
	blog_p->ucast_vtag_num = blog_p->vtag_num;
	blog_p->vtag_tx_num = blog_p->vtag_num;
	blog_p->rx.info.phyHdrType = netdev_path_get_hw_port_type(
			(struct net_device *)blog_p->rx_dev_p);
	blog_p->rx.info.channel = netdev_path_get_hw_port(
			(struct net_device *)blog_p->rx_dev_p);
	blog_p->rx.length = ETH_HLEN + BLOG_VLAN_HDR_LEN * blog_p->vtag_num;
	blog_p->tx.length = blog_p->rx.length;

	/* TODO!! even though we don't support it yet, but how do
	 * we find rx->wan_flow
	 */

	return 0;
}

static int _fpi_flow_to_blog_wlan(fpi_flow_t *flow_p,
				  fpi_wlan_egress_info_t *wl_info_p,
				  Blog_t *blog_p)
{
	fpi_context_t *ctx_p = &flow_p->context;
	struct net_device *net = (struct net_device *)blog_p->tx_dev_p;
	uint8_t prio = ctx_p->wl_user_priority;
	uint8_t *sa;
	uint8_t *da;
	uint32_t metadata;

	if (flow_p->key.mode == fpi_mode_l2) {
		/* get SA and DA from the key */
		sa = flow_p->key.l2_key.src_mac;
		da = flow_p->key.l2_key.dst_mac;
	} else {
		/* get SA and DA from the context */
		sa = ctx_p->src_mac;
		da = ctx_p->dst_mac;
	}

	if (is_netdev_wlan_dhd(net)) {
		if (unlikely(fpi_db.dhd_get_metadata_func_p == NULL)) {
			BCM_LOG_INFO(BCM_LOG_ID_FPI, "NULL WLAN DHD metadata_func_p\n");
			return -EINVAL;
		}
		if (fpi_db.dhd_get_metadata_func_p(net, sa, da, prio, &metadata) != 0) {
			BCM_LOG_ERROR(BCM_LOG_ID_FPI,
				"WLAN DHD metadata_func_p returned error\n");
			return -EINVAL;
		}
	} else if (is_netdev_wlan_nic(net)) {
		if (unlikely(fpi_db.wl_get_metadata_func_p == NULL)) {
			BCM_LOG_INFO(BCM_LOG_ID_FPI, "NULL WLAN NIC metadata_func_p\n");
			return -EINVAL;
		}
		if (fpi_db.wl_get_metadata_func_p(net, sa, da, prio, &metadata) != 0) {
			BCM_LOG_ERROR(BCM_LOG_ID_FPI,
				"WLAN NIC metadata_func_p returned error\n");
			return -EINVAL;
		}
	} else {
		BCM_LOG_ERROR(BCM_LOG_ID_FPI, "Unknown WLAN netdevice\n");
		return -EINVAL;
	}

	blog_p->wl = metadata;
	if (blog_p->wfd.nic_ucast.is_wfd == 1) {
		if (blog_p->wfd.nic_ucast.is_chain == 1) {
			wl_info_p->wl_dst_type = FPI_WL_DST_TYPE_WFD_NIC;
			wl_info_p->wfd_nic_key.half =
					blog_p->wfd.nic_ucast.chain_idx;
			wl_info_p->radio_idx = blog_p->wfd.nic_ucast.wfd_idx;
		} else {
			wl_info_p->wl_dst_type = FPI_WL_DST_TYPE_WFD_DHD;
			wl_info_p->flowring_id =
					blog_p->wfd.dhd_ucast.flowring_idx;
			wl_info_p->radio_idx = blog_p->wfd.dhd_ucast.wfd_idx;
		}
	} else {
		wl_info_p->wl_dst_type = FPI_WL_DST_TYPE_WDO_DIRECT;
		wl_info_p->flowring_id = blog_p->rnr.flowring_idx;
		wl_info_p->radio_idx = blog_p->rnr.radio_idx;
	}

	return 0;
}

static int fpi_ctx_to_blog(fpi_flow_t *flow_p,
			   fpi_wlan_egress_info_t *wl_info_p, Blog_t *blog_p)
{
	fpi_key_t *key_p = &flow_p->key;
	fpi_context_t *ctx_p = &flow_p->context;
	uint16_t inner_ethtype, ipv6_tos, csum16;
	int rc, i, l2hdr_adj;
	bool is_flow_ipv6 = false, is_flow_ipv4 = false, is_l2_updated = false;
	bool do_icsum = false;
	struct iphdr *iph_ptr;

	if (ctx_p->drop) {
		blog_p->pkt_drop = 1;
		return 0;
	}

	blog_p->tx_dev_p = (void *)(uintptr_t)ctx_p->egress_device_ptr;
	blog_p->tx.info.phyHdrType = netdev_path_get_hw_port_type(
			(struct net_device *)blog_p->tx_dev_p);
	blog_p->tx.info.channel = netdev_path_get_hw_port(
			(struct net_device *)blog_p->tx_dev_p);
	wl_info_p->wl_dst_type = FPI_WL_DST_TYPE_INVALID;

	if (BLOG_GET_PHYTYPE(blog_p->tx.info.phyHdrType) == BLOG_ENETPHY) {
		blog_p->mark = SKBMARK_SET_Q_PRIO(blog_p->mark,
						  ctx_p->egress_priority);
	} else if (BLOG_GET_PHYTYPE(blog_p->tx.info.phyHdrType) ==
		   BLOG_WLANPHY) {
		rc = _fpi_flow_to_blog_wlan(flow_p, wl_info_p, blog_p);
		if (rc) {
			BCM_LOG_ERROR(BCM_LOG_ID_FPI,
				      "fail to parse WLAN Egress Info\n");
			return rc;
		}
	} else if (blog_p->tx.info.phyHdrType == BLOG_SPU_DS ||
			blog_p->tx.info.phyHdrType == BLOG_SPU_US) {
		rc = fpi_ctx_to_blog_spu(flow_p, blog_p);
		if (rc) {
			BCM_LOG_ERROR(BCM_LOG_ID_FPI,
				"failed to prepare SPU Info, rc %d\n", rc);
			return rc;
		}
	} else /* Do not support */
		return -EINVAL;

	/* for future, if we are to expand this for other interface types.
	 * for PON: (blog_p->tx.info.phyHdrType == BLOG_GPONPHY || BLOG_EPONPHY)
	 *	    => It requires blog_p->tx.info.channel == GEM index
	 * for DSL: blog_p->tx.info.bmap.BCM_XPHY == 1
	 *	    => it requires blog_p->tx.info.channel == WAN flow
	 */


	if (key_p->mode != fpi_mode_l3l4)
		goto vtag_check;

	if (key_p->l3l4_key.esp_spi_mode == FPI_ESP_IGNORED) {
		is_l2_updated = true;
		memcpy(&blog_p->tx.l2hdr[0], ctx_p->dst_mac, ETH_ALEN);
		memcpy(&blog_p->tx.l2hdr[6], ctx_p->src_mac, ETH_ALEN);
		for (i = 0; i < blog_p->vtag_num; i++) {
			blog_p->tx.l2hdr[12 + (i * 4)] =
					BLOG_ETH_P_8021Q >> 8;
			blog_p->tx.l2hdr[13 + (i * 4)] =
					BLOG_ETH_P_8021Q & 0xff;
		}
		if (key_p->l3l4_key.is_ipv6) {
			blog_p->tx.l2hdr[12 + (blog_p->vtag_num * 4)] =
					(ETH_P_IPV6 >> 8) & 0xff;
			blog_p->tx.l2hdr[13 + (blog_p->vtag_num * 4)] =
					ETH_P_IPV6 & 0xff;
		} else {
			blog_p->tx.l2hdr[12 + (blog_p->vtag_num * 4)] =
					(ETH_P_IP >> 8) & 0xff;
			blog_p->tx.l2hdr[13 + (blog_p->vtag_num * 4)] =
					ETH_P_IP & 0xff;
		}

		if (ctx_p->napt_enable == 1) {
			if (key_p->l3l4_key.is_ipv6) {
				for (i = 0; i < 4; i++) {
					blog_p->tupleV6.saddr.p32[i] =
							htonl(ctx_p->src_ip[i]);
					blog_p->tupleV6.daddr.p32[i] =
							htonl(ctx_p->dst_ip[i]);
				}
				blog_p->del_tupleV6.port.source =
						ctx_p->src_port;
				blog_p->del_tupleV6.port.dest =
						ctx_p->dst_port;
				/* put dummy rx hop limit vs tx hop limit, so
				 * it is considered as routed packet by hardware
				 */
				blog_p->tupleV6.rx_hop_limit = 32;
				blog_p->tupleV6.tx_hop_limit = 31;
			} else {
				blog_p->tx.tuple.saddr =
						htonl(ctx_p->src_ip[0]);
				blog_p->tx.tuple.daddr =
						htonl(ctx_p->dst_ip[0]);
				blog_p->tx.tuple.port.source =
						htons(ctx_p->src_port);
				blog_p->tx.tuple.port.dest =
						htons(ctx_p->dst_port);
				/* put dummy rx ttl vs tx ttl, so it is
				 * considered as routed packet by hardware
				 */
				blog_p->rx.tuple.ttl = 32;
				blog_p->tx.tuple.ttl = 31;
				do_icsum = true;
			}
		} else if (blog_p->rx.info.phyHdrType == BLOG_SPU_DS ||
				blog_p->rx.info.phyHdrType == BLOG_SPU_US) {
			blog_p->rx.length = 0;
			blog_p->tx.length = 14;
			blog_p->tx.l2hdr[12] = (ETH_P_IP >> 8) & 0xff;
			blog_p->tx.l2hdr[13] = ETH_P_IP & 0xff;
			blog_p->rx.tuple.ttl = 32;
			blog_p->tx.tuple.ttl =
				(blog_p->rx.info.phyHdrType == BLOG_SPU_US ?
					32 : 31);
			blog_p->tuple_offset = blog_p->rx.length;
		}
	} else {
		blog_p->rx.length = 14;
		blog_p->tx.length = 0;
		blog_p->rx.tuple.ttl = 32;
		blog_p->tx.tuple.ttl =
				(blog_p->tx.info.phyHdrType == BLOG_SPU_US ?
					31 : 32);
		blog_p->tuple_offset = blog_p->rx.length;
	}

vtag_check:

	if (((key_p->mode == fpi_mode_l3l4) &&
			(key_p->l3l4_key.is_ipv6 == 0)) ||
	    ((key_p->mode == fpi_mode_l2) &&
			(key_p->l2_key.eth_type == ETH_P_IP)))
		is_flow_ipv4 = true;

	if (((key_p->mode == fpi_mode_l3l4) && (key_p->l3l4_key.is_ipv6)) ||
	    ((key_p->mode == fpi_mode_l2) &&
			(key_p->l2_key.eth_type == ETH_P_IPV6)))
		is_flow_ipv6 = true;

	blog_p->rx_max_pktlen = ctx_p->max_ingress_packet_size;

	if (ctx_p->vtag_check) {
		blog_p->outVtagCk = 1;
		blog_p->outer_vtag[0] = ctx_p->vtag_value;
	}

	/* Note: case vlan_8021_replace is basically remove then prepend */
	/* vtag_tx_num should equal to vtag_num (rx) at this moment */
	if ((ctx_p->vlan_8021q_remove) && (blog_p->vtag_tx_num > 0)) {
		is_l2_updated = true;
		blog_p->tx.length -= 4;

		if (blog_p->vtag_num > 1)
			inner_ethtype = BLOG_ETH_P_8021Q;
		else if (key_p->mode == fpi_mode_l3l4) {
			if (key_p->l3l4_key.is_ipv6)
				inner_ethtype = ETH_P_IPV6;
			else
				inner_ethtype = ETH_P_IP;
		} else /* if (key_p->mode == fpi_mode_l2) */
			inner_ethtype = key_p->l2_key.eth_type;

		blog_p->rx.l2hdr[16] = inner_ethtype >> 8;
		blog_p->rx.l2hdr[17] = inner_ethtype & 0xff;
		blog_p->rx.l2hdr[12] = BLOG_ETH_P_8021Q >> 8;
		blog_p->rx.l2hdr[13] = BLOG_ETH_P_8021Q & 0xff;
		blog_p->rx.l2hdr[14] = (ctx_p->vtag_value >> 8) & 0xff;
		blog_p->rx.l2hdr[15] = ctx_p->vtag_value & 0xff;
		blog_p->tx.l2hdr[12] = inner_ethtype >> 8;
		blog_p->tx.l2hdr[13] = inner_ethtype & 0xff;

		blog_p->vtag_tx_num--;
	}

	if (ctx_p->vlan_8021q_prepend) {
		is_l2_updated = true;
		blog_p->tx.length += 4;
		blog_p->tx.l2hdr[16] = blog_p->tx.l2hdr[12];
		blog_p->tx.l2hdr[17] = blog_p->tx.l2hdr[13];
		if ((blog_p->vtag_num) && (blog_p->vtag_tx_num)) {
			blog_p->tx.l2hdr[16] = BLOG_ETH_P_8021Q >> 8;
			blog_p->tx.l2hdr[17] = BLOG_ETH_P_8021Q & 0xff;
			if (blog_p->vtag_tx_num == blog_p->vtag_num) {
				blog_p->tx.l2hdr[18] =
						(ctx_p->vtag_value >> 8) & 0xff;
				blog_p->tx.l2hdr[19] = ctx_p->vtag_value & 0xff;
			}
		}
		blog_p->tx.l2hdr[12] = (ctx_p->vlan_8021q_hdr >> 24) & 0xff;
		blog_p->tx.l2hdr[13] = (ctx_p->vlan_8021q_hdr >> 16) & 0xff;
		blog_p->tx.l2hdr[14] = (ctx_p->vlan_8021q_hdr >> 8) & 0xff;
		blog_p->tx.l2hdr[15] = ctx_p->vlan_8021q_hdr & 0xff;
		blog_p->vtag_tx_num++;
	}

	/* when outer_l2hdr_remove_mode is set but not GRE remove, it is
	 * for Mesh header.
	 */
	if ((ctx_p->outer_l2hdr_remove_mode) && (ctx_p->gre_remove == 0)) {
		l2hdr_adj = (ctx_p->outer_l2hdr_remove_mode -
				fpi_l2hdr_mode_14b) * VLAN_HLEN + ETH_HLEN;
		memmove(&blog_p->rx.l2hdr[l2hdr_adj], blog_p->rx.l2hdr,
			blog_p->rx.length);
		blog_p->rx.length += l2hdr_adj;
		if (!is_l2_updated) {
			blog_p->tunl_l2tx_adj = blog_p->tx.length;
			blog_p->tx.length = 0;
		}
	}

	l2hdr_adj = 0;

	/* when outer_l2hdr_insert_mode is set but not GRE prepend, it is
	 * for Mesh header.
	 */
	if ((ctx_p->outer_l2hdr_insert_mode) && (ctx_p->gre_prepend == 0)) {
		is_l2_updated = true;
		/* blog_p->tx.info.bmap.DEL_L2 = 1; */
		l2hdr_adj = (ctx_p->outer_l2hdr_insert_mode -
				fpi_l2hdr_mode_14b) * VLAN_HLEN + ETH_HLEN;
		memmove(&blog_p->tx.l2hdr[l2hdr_adj], blog_p->tx.l2hdr,
			blog_p->tx.length);
		memcpy(blog_p->tx.l2hdr, ctx_p->outer_l2hdr, l2hdr_adj);

		if (!is_l2_updated)
			blog_p->tunl_l2tx_adj = blog_p->tx.length;
		else if ((blog_p->tx.length > ETH_HLEN) &&
			 ctx_p->vlan_8021q_prepend)
			blog_p->tunl_l2tx_adj = blog_p->tx.length - ETH_HLEN -
						BLOG_VLAN_HDR_LEN;
		else
			blog_p->tunl_l2tx_adj = blog_p->tx.length - ETH_HLEN;
		blog_p->tx.length += l2hdr_adj - blog_p->tunl_l2tx_adj;
	}

	if (ctx_p->dscp_rewrite) {
		if (is_flow_ipv6) {
			ipv6_tos = PKT_IPV6_GET_TOS_WORD(blog_p->tupleV6.word0);
			if (unlikely((ctx_p->dscp & FPI_DSCP_MAX) ==
				     BLOG_IPTOS2DSCP(ipv6_tos)))
				return -EINVAL;

			ipv6_tos &= ~BLOG_IPDSCP2TOS(FPI_DSCP_MAX);
			ipv6_tos |= BLOG_IPDSCP2TOS(ctx_p->dscp);
			PKT_IPV6_SET_TOS_WORD(blog_p->tupleV6.word0, ipv6_tos);
		} else {
			if (unlikely((ctx_p->dscp & FPI_DSCP_MAX) ==
				     BLOG_IPTOS2DSCP(blog_p->rx.tuple.tos)))
				return -EINVAL;

			blog_p->tx.tuple.tos &= ~BLOG_IPDSCP2TOS(FPI_DSCP_MAX);
			blog_p->tx.tuple.tos |= BLOG_IPDSCP2TOS(ctx_p->dscp);
			do_icsum = true;
		}
	}

	if (ctx_p->gre_prepend) {
		if (fpi_db.gre_mode != fpi_gre_mode_proprietary) {
			pr_err("Currently only support proprietary GRE\n");
			return -EINVAL;
		}

		/* can't perform GRE prepend if not IPv4/v6 flow */
		if ((is_flow_ipv4 == false) && (is_flow_ipv6 == false))
			return -EINVAL;

#if 0
		/* FIXME! in discussion, trying to convince that outer L2Hdr
		 * insertion should be based on a complete L2 Hdr information,
		 * rather than src_mac, dst_mac and such.  This approach can
		 * support multiple VLAN tag insertion of outer L2 Hdr easier.
		 */
		/* must state the insert mode */
		if (ctx_p->outer_l2hdr_insert_mode == 0){
			return -EINVAL;
#endif

		blog_p->tx.info.bmap.GRE = 1;

		/* L2 GRE specific START */
		blog_p->tx.info.bmap.GRE_ETH = 1;
		if (is_l2_updated) {
			/* L2 GRE inner L2 header length is either ETH_HLEN
			 * or ETH_HLEN + BLOG_VLAN_HDR_LEN
			 */
			blog_p->gretx.l2_hlen = ETH_HLEN;
			if ((blog_p->tx.length > ETH_HLEN) &&
			    ctx_p->vlan_8021q_prepend) {
				blog_p->gretx.l2_hlen += BLOG_VLAN_HDR_LEN;
			}

			memcpy(&blog_p->gretx.l2hdr[0], &blog_p->tx.l2hdr[0],
			       blog_p->gretx.l2_hlen);
		} else {
			blog_p->gretx.l2_hlen = 0;
		}

		/* new L2 header. TX L2 header will be fixed to 14 bytes
		 * for tunnel outer L2, so we will need to compute
		 * tunl_l2tx_adj, which basically is the size of the byte
		 * from ORG TX L2 header not included in GreTX->L2_Header
		 */
		/* tunl_l2tx_adj could have been set in
		 * outer_l2_hdr_insert_mode case
		 */
		blog_p->tunl_l2tx_adj += blog_p->tx.length -
					 blog_p->gretx.l2_hlen;
		if (ctx_p->outer_l2hdr_insert_mode == 0) {
			/* the old legacy that doesn't need
			 * outer_l2hdr_insert_mode and assumes outer_l2hdr is
			 * 14 Bytes.  It also uses outer_dst_mac and
			 * outer_src_mac, rather than outer_l2hdr.
			 * FIXME! in discussion.
			 */
			blog_p->tx.length = ETH_HLEN;
			memcpy(&blog_p->tx.l2hdr[0], ctx_p->outer_dst_mac,
			       ETH_ALEN);
			memcpy(&blog_p->tx.l2hdr[6], ctx_p->outer_src_mac,
			       ETH_ALEN);
		} else {
			blog_p->tx.length = (ctx_p->outer_l2hdr_insert_mode -
					fpi_l2hdr_mode_14b) * VLAN_HLEN +
					ETH_HLEN;
			memcpy(blog_p->tx.l2hdr, ctx_p->outer_l2hdr,
			       blog_p->tx.length);
		}
		/* L2 GRE specific END */

		if (is_flow_ipv6) {
			blog_p->tx.info.bmap.DEL_IPv6 = 1;

			/* IPv6 update. just do copy from the provided
			 * encap_ip_hdr.  This should update the required
			 * fields: saddr, daddr, and tx_hop_limit.
			 */
			memcpy(&blog_p->del_tupleV6, ctx_p->encap_ip_hdr,
			       sizeof(struct ipv6hdr));
			/* no need to update TOS */

			/* L2 GRE specific */
			blog_p->tx.info.bmap.GRE_ETH_IPv6 = 1;

			/* L2->ethtype */
			blog_p->tx.l2hdr[12] = (ETH_P_IPV6 >> 8) & 0xff;
			blog_p->tx.l2hdr[13] = ETH_P_IPV6 & 0xff;
			/* L2 GRE specific END */
		} else {
			iph_ptr = (struct iphdr *)ctx_p->encap_ip_hdr;
			blog_p->tx.info.bmap.DEL_IPv4 = 1;

			/* IPv4 update */
			blog_p->deltx_tuple.saddr = iph_ptr->saddr;
			blog_p->deltx_tuple.daddr = iph_ptr->daddr;
			blog_p->deltx_tuple.ttl = iph_ptr->ttl;
			blog_p->deltx_tuple.tos = iph_ptr->tos;

			/* we don't support iph_ptr->frag_off */

			/* L2 GRE specific */
			blog_p->tx.info.bmap.GRE_ETH_IPv4 = 1;

			/* L2->ethtype */
			blog_p->tx.l2hdr[12] = (ETH_P_IP >> 8) & 0xff;
			blog_p->tx.l2hdr[13] = ETH_P_IP & 0xff;
			/* L2 GRE specific END */
		}

		blog_p->gretx.gre_flags.u16 = ctx_p->gre_hdr >> 16;
		blog_p->gretx.hlen = gre_calc_hlen(htons(
					blog_p->gretx.gre_flags.u16));

		if (fpi_db.gre_mode == fpi_gre_mode_proprietary) {
			blog_p->spGre = 1;
			/* temporarily use ipid to pass the remaining 16
			 * bytes of the special mode GRE
			 */
			blog_p->gretx.ipid = (uint16_t)(ctx_p->gre_hdr &
							0xffff);
		}
	}

	if (ctx_p->gre_remove) {
		if (fpi_db.gre_mode != fpi_gre_mode_proprietary) {
			pr_err("Currently only support proprietary GRE\n");
			return -EINVAL;
		}

		/* can't perform GRE remove if not IPv4/v6 flow */
		if ((is_flow_ipv4 == false) && (is_flow_ipv6 == false))
			return -EINVAL;

		if (key_p->mode != fpi_mode_l3l4)
			return -EINVAL;

		/* must state the remove mode */
		if (ctx_p->outer_l2hdr_remove_mode == 0)
			return -EINVAL;

		blog_p->rx.info.bmap.GRE = 1;

		/* since we don't know much about the option in the GRE
		 * header, so we are assuming it is just the base GRE HDR
		 * that is getting removed
		 */
		blog_p->grerx.gre_flags.u16 = 0;
		blog_p->grerx.hlen = gre_calc_hlen(htons(
					blog_p->grerx.gre_flags.u16));

		/* L2 GRE specific */
		blog_p->rx.info.bmap.GRE_ETH = 1;
		if (is_l2_updated) {
			blog_p->grerx.l2_hlen = blog_p->rx.length;

			if (ctx_p->outer_l2hdr_insert_mode == 0) {
				if ((blog_p->tx.length > ETH_HLEN) &&
				    ctx_p->vlan_8021q_prepend) {
					blog_p->tunl_l2tx_adj =
							blog_p->tx.length -
							ETH_HLEN -
							BLOG_VLAN_HDR_LEN;
					blog_p->tx.length = ETH_HLEN +
							BLOG_VLAN_HDR_LEN;
				} else {
					blog_p->tunl_l2tx_adj =
							blog_p->tx.length -
							ETH_HLEN;
					blog_p->tx.length = ETH_HLEN;
				}
			}
		} else {
			blog_p->grerx.l2_hlen = 0;
			blog_p->tx.length = 0;
		}
		blog_p->rx.length = (ctx_p->outer_l2hdr_remove_mode -
				fpi_l2hdr_mode_14b) * VLAN_HLEN + ETH_HLEN;
		/* L2 GRE specific END */

		if (is_flow_ipv6) {
			blog_p->rx.info.bmap.DEL_IPv6 = 1;

			/* L2 GRE specific */
			blog_p->rx.info.bmap.GRE_ETH_IPv6 = 1;
			/* L2 GRE specific END */
		} else {
			blog_p->rx.info.bmap.DEL_IPv4 = 1;

			/* L2 GRE specific */
			blog_p->rx.info.bmap.GRE_ETH_IPv4 = 1;
			/* L2 GRE specific END */
		}
	}

	/* building incremental checksum */
	if (do_icsum == true) {
		/* Build the delta L4 checksum and save in TX Tuple */
		csum16 = _compute_icsum(0, blog_p->rx.tuple.saddr,
					blog_p->tx.tuple.saddr,
					blog_p->rx.tuple.port.source,
					blog_p->tx.tuple.port.source);
		csum16 = _compute_icsum(csum16, blog_p->rx.tuple.daddr,
					blog_p->tx.tuple.daddr,
					blog_p->rx.tuple.port.dest,
					blog_p->tx.tuple.port.dest);
		blog_p->tx.tuple.check = csum16;

		/* Build the delta L3 checksum and save in RX Tuple */
		csum16 = _compute_icsum32(0, blog_p->rx.tuple.saddr,
					  blog_p->tx.tuple.saddr);
		csum16 = _compute_icsum32(csum16, blog_p->rx.tuple.daddr,
					  blog_p->tx.tuple.daddr);
		/* Delta checksum: IP TOS mangling */
		if (blog_p->rx.tuple.tos != blog_p->tx.tuple.tos) {
			/* Compute delta checksum from tos mangling */
			csum16 = _compute_icsum16(csum16,
				htons((0x45 << 8) | (blog_p->rx.tuple.tos)),
				htons((0x45 << 8) | (blog_p->tx.tuple.tos)));
		}
		blog_p->rx.tuple.check = csum16;    /* L3 checksum delta */
	}

	return 0;
}

static int fpi_update_flow_stat(uint32_t fpi_handle)
{
	fpi_flow_entry_t *flow_entry_p;
	int rc;

	flow_entry_p = local_db_get_flow(fpi_handle);
	if (flow_entry_p == NULL)
		return -EINVAL;

	FPI_LOCK_BH();

	rc = g_hw_info.get_stat(flow_entry_p->hw_handle,
				&flow_entry_p->stat.pkt,
				&flow_entry_p->stat.byte);

	FPI_UNLOCK_BH();
	return rc;
}

int fpi_add_flow(fpi_flow_t *flow_p, uint32_t *fpi_handle_p)
{
	Blog_t blog;
	fpi_wlan_egress_info_t wl_info;
	int rc, hw_handle;
	uint32_t fpi_handle;

	if (unlikely(g_hw_info.registered == 0))
		return -EPERM;

	/* convert key and context to blog */
	memset(&blog, 0x0, sizeof(Blog_t));
	blog.vtag[0] = 0xFFFFFFFF;
	blog.vtag[1] = 0xFFFFFFFF;
	blog.dpi_queue = ~0;
	memset(&wl_info, 0x0, sizeof(fpi_wlan_egress_info_t));

	rc = fpi_key_to_blog(&flow_p->key, &blog);
	if (rc)
		return rc;
	rc = fpi_ctx_to_blog(flow_p, &wl_info, &blog);
	if (rc)
		return rc;
	rc = local_db_find_flow_by_key(&flow_p->key, &fpi_handle);
	if (rc == 0) {	/* an existing entry is found */
		rc = -EEXIST;
		goto done;
	} else if (rc != -ENOENT) {
		goto done;
	}

	FPI_LOCK_BH();

	/* using platform specific method to add the blog */
	rc = g_hw_info.add_flow(&blog, &hw_handle);
	if (rc) {
		FPI_UNLOCK_BH();
		BCM_LOG_ERROR(BCM_LOG_ID_FPI,
			      "fail to add entry to HW table\n");
		goto done;
	}

	rc = local_db_add_flow(flow_p, hw_handle, &wl_info, fpi_handle_p);
	if (rc) {
		BCM_LOG_ERROR(BCM_LOG_ID_FPI,
			      "fail to add entry to local shadow table\n");
		g_hw_info.delete_flow(hw_handle);
	}

	FPI_UNLOCK_BH();

done:
	fpi_cleanup_blog_spu(&blog);
	return rc;
}
EXPORT_SYMBOL(fpi_add_flow);

int fpi_delete_flow_by_handle(uint32_t fpi_handle)
{
	int rc;
	uint32_t hw_handle = 0;

	if (unlikely(g_hw_info.registered == 0))
		return -EPERM;

	rc = local_db_get_hw_handle(fpi_handle, &hw_handle);

	FPI_LOCK_BH();

	rc = rc ? rc : g_hw_info.delete_flow(hw_handle);
	rc = rc ? rc : local_db_delete_flow(fpi_handle);

	FPI_UNLOCK_BH();
	return rc;
}
EXPORT_SYMBOL(fpi_delete_flow_by_handle);

int fpi_delete_flow_by_key(fpi_key_t *key_p)
{
	int rc;
	uint32_t fpi_handle;
	Blog_t blog;

	if (unlikely(g_hw_info.registered == 0))
		return -EPERM;


	/* If HW provides ability to delete flow by blog, then we will use
	 * the approach, otherwise, we will search for the fpi_handle from local
	 * database and delete via fpi_handle.
	 */
	if (g_hw_info.delete_flow_by_blog != NULL) {
		/* convert key to blog */
		memset(&blog, 0x0, sizeof(Blog_t));
		rc = fpi_key_to_blog(key_p, &blog);
		rc = rc ? rc : local_db_find_flow_by_key(key_p, &fpi_handle);

		FPI_LOCK_BH();
		rc = rc ? rc : g_hw_info.delete_flow_by_blog(&blog);
		rc = rc ? rc : local_db_delete_flow(fpi_handle);
		FPI_UNLOCK_BH();

		if (rc)
			BCM_LOG_ERROR(BCM_LOG_ID_FPI, "fail to delete flow\n");

		return rc;
	}

	/* Search through local database for matching entry to find
	 * the fpi_handle, and delete if find the fpi_handle
	 */
	rc = local_db_find_flow_by_key(key_p, &fpi_handle);
	rc = rc ? rc : fpi_delete_flow_by_handle(fpi_handle);
	return rc;

}
EXPORT_SYMBOL(fpi_delete_flow_by_key);

int fpi_get_stat(uint32_t fpi_handle, fpi_stat_t *stat_p)
{
	fpi_flow_entry_t *flow_entry_p;
	int rc;

	if (unlikely(g_hw_info.registered == 0))
		return -EPERM;

	rc = fpi_update_flow_stat(fpi_handle);
	if (rc)
		return rc;

	flow_entry_p = local_db_get_flow(fpi_handle);
	if (flow_entry_p == NULL)
		return -ENOENT;

	memcpy(stat_p, &flow_entry_p->stat, sizeof(fpi_stat_t));

	return 0;
}
EXPORT_SYMBOL(fpi_get_stat);

int fpi_get_flow(uint32_t fpi_handle, fpi_flow_t *flow_p)
{
	fpi_flow_entry_t *flow_entry_p;

	if (unlikely(g_hw_info.registered == 0))
		return -EPERM;

	flow_entry_p = local_db_get_flow(fpi_handle);
	if (flow_entry_p == NULL)
		return -ENOENT;

	memcpy(flow_p, &flow_entry_p->flow, sizeof(fpi_flow_t));
	return 0;
}
EXPORT_SYMBOL(fpi_get_flow);

static int get_root_dev_by_name(char *ifname, struct net_device **_dev)
{
	if (_dev == NULL)
		return -EINVAL;

	*_dev = dev_get_by_name(&init_net, ifname);
	if (!*_dev)
		return -EINVAL;

	*_dev = netdev_path_get_root(*_dev);

	return 0;
}

int fpi_add_ap_mac(uint8_t *mac_addr_p)
{
	int i, avail_idx = -1, rc;

	if (unlikely(g_hw_info.registered == 0))
		return -EPERM;

	FPI_LOCK_BH();
	for (i = 0; i < FPI_MAX_AP_MAC; i++) {
		if ((fpi_db.mac_table[i].ref_cnt > 0) &&
		    (memcmp(fpi_db.mac_table[i].mac,
			    mac_addr_p, ETH_ALEN) == 0)) {
			fpi_db.mac_table[i].ref_cnt++;
			FPI_UNLOCK_BH();
			return 0;
		}

		if ((fpi_db.mac_table[i].ref_cnt == 0) &&
		    (avail_idx == -1))
			avail_idx = i;
	}

	if (avail_idx != -1) {
		rc = g_hw_info.add_ap_mac(mac_addr_p);
		if (rc) {
			FPI_UNLOCK_BH();
			return rc;
		}

		fpi_db.mac_table[avail_idx].ref_cnt = 1;
		memcpy(fpi_db.mac_table[avail_idx].mac, mac_addr_p, ETH_ALEN);
	}

	FPI_UNLOCK_BH();
	return 0;
}
EXPORT_SYMBOL(fpi_add_ap_mac);

int fpi_delete_ap_mac(uint8_t *mac_addr_p)
{
	int i, rc = 0;

	if (unlikely(g_hw_info.registered == 0))
		return -EPERM;

	FPI_LOCK_BH();
	for (i = 0; i < FPI_MAX_AP_MAC; i++) {
		if ((fpi_db.mac_table[i].ref_cnt > 0) &&
		    (memcmp(fpi_db.mac_table[i].mac,
			    mac_addr_p, ETH_ALEN) == 0)) {
			fpi_db.mac_table[i].ref_cnt--;

			if (fpi_db.mac_table[i].ref_cnt == 0)
				rc = g_hw_info.delete_ap_mac(mac_addr_p);

			FPI_UNLOCK_BH();
			return rc;
		}
	}

	/* no entry found */
	FPI_UNLOCK_BH();
	return -ENOENT;
}
EXPORT_SYMBOL(fpi_delete_ap_mac);

bool fpi_check_ap_mac(uint8_t *mac_addr_p)
{
	int i = 0;

	FPI_LOCK_BH();
	for (i = 0; i < FPI_MAX_AP_MAC; i++) {
		if ((fpi_db.mac_table[i].ref_cnt > 0) &&
		    (memcmp(fpi_db.mac_table[i].mac,
			    mac_addr_p, ETH_ALEN) == 0)) {

			FPI_UNLOCK_BH();
			return true;
		}
	}

	/* no entry found */
	FPI_UNLOCK_BH();
	return false;
}
EXPORT_SYMBOL(fpi_check_ap_mac);

int fpi_set_gre_mode(fpi_gre_mode_t mode)
{
	int rc;

	if (unlikely(g_hw_info.registered == 0))
		return -EPERM;

	if (unlikely(g_hw_info.set_gre_mode == NULL))
		return -EPERM;

	if (mode >= fpi_gre_mode_num)
		return -EINVAL;


	fpi_db.gre_mode = mode;
	rc = g_hw_info.set_gre_mode(mode);

	return rc;
}
EXPORT_SYMBOL(fpi_set_gre_mode);

int fpi_set_l2lkp_on_etype(bool enable, uint16_t etype)
{
	int rc;

	if (unlikely(g_hw_info.registered == 0))
		return -EPERM;

	if (unlikely(g_hw_info.set_l2lkp_on_etype == NULL))
		return -EPERM;

	rc = g_hw_info.set_l2lkp_on_etype(enable, etype);
	if (rc)
		return rc;

	fpi_db.l2lkp_on_etype = enable;
	fpi_db.etype_for_l2lkp = etype;

	return rc;
}
EXPORT_SYMBOL(fpi_set_l2lkp_on_etype);

static int fpi_set_lkp_enable(bool enable)
{
	int rc;

	if (unlikely(g_hw_info.registered == 0))
		return -EPERM;

	if (unlikely(g_hw_info.set_lkp_enable == NULL))
		return -EPERM;

	rc = g_hw_info.set_lkp_enable(enable);
	if (rc)
		return rc;

	fpi_db.lkp_enable = enable;

	return rc;
}

#undef FPI_DECL
#define FPI_DECL(_x)	#_x,

const char *fpictl_ioctl_name[] = {
	FPI_DECL(fpictl_ioctl_sys)
	FPI_DECL(fpictl_ioctl_max)
};

const char *fpictl_subsys_name[] = {
	FPI_DECL(fpictl_subsys_mode)
	FPI_DECL(fpictl_subsys_flow)
	FPI_DECL(fpictl_subsys_stat)
	FPI_DECL(fpictl_subsys_apmac)
	FPI_DECL(fpictl_subsys_gre_mode)
	FPI_DECL(fpictl_subsys_l2lkp_on_etype)
	FPI_DECL(fpictl_subsys_max)
};

const char *fpictl_op_name[] = {
	FPI_DECL(fpictl_op_set)
	FPI_DECL(fpictl_op_get)
	FPI_DECL(fpictl_op_add)
	FPI_DECL(fpictl_op_del_by_handle)
	FPI_DECL(fpictl_op_del_by_key)
	FPI_DECL(fpictl_op_getnext)
	FPI_DECL(fpictl_op_max)
};

static int fpi_ioctl_subsys_flow(fpictl_data_t *fpi_p, unsigned long arg)
{
	fpi_key_t *key_p = &fpi_p->flow.key;
	fpi_context_t *ctx_p = &fpi_p->flow.context;
	struct net_device *in_dev, *out_dev = NULL;
	int index;
	fpi_flow_entry_t *flow_entry_p;

	switch (fpi_p->op) {
	case fpictl_op_get:
		fpi_p->rc = fpi_get_flow(fpi_p->handle, &fpi_p->flow);
		if (fpi_p->rc == 0) {
			in_dev = (struct net_device *)(uintptr_t)
					key_p->l2_key.ingress_device_ptr;
			out_dev = (struct net_device *)(uintptr_t)
					ctx_p->egress_device_ptr;
			if (in_dev == NULL)
				return -EINVAL;
			memcpy(key_p->l2_key.ingress_device_name,
			       in_dev->name, IFNAMSIZ);

			if (out_dev != NULL)
				memcpy(ctx_p->egress_device_name,
				       out_dev->name, IFNAMSIZ);

			flow_entry_p = local_db_get_flow(fpi_p->handle);
			memcpy(&fpi_p->wl_info, &flow_entry_p->wl_info,
			       sizeof(fpi_wlan_egress_info_t));
		}
		break;
	case fpictl_op_add:
		fpi_p->rc = get_root_dev_by_name(
				key_p->l2_key.ingress_device_name, &in_dev);
		if (fpi_p->rc)
			return fpi_p->rc;

		if (ctx_p->drop == 0) {
			fpi_p->rc = get_root_dev_by_name(
					ctx_p->egress_device_name, &out_dev);
			if (fpi_p->rc)
				return fpi_p->rc;
		}

		key_p->l2_key.ingress_device_ptr = (uint64_t)(uintptr_t)in_dev;
		ctx_p->egress_device_ptr = (uint64_t)(uintptr_t)out_dev;

		fpi_p->rc = fpi_add_flow(&fpi_p->flow, &fpi_p->handle);
		break;
	case fpictl_op_del_by_handle:
		fpi_p->rc = fpi_delete_flow_by_handle(fpi_p->handle);
		break;
	case fpictl_op_del_by_key:
		fpi_p->rc = get_root_dev_by_name(
					key_p->l2_key.ingress_device_name,
					&in_dev);
		if (fpi_p->rc)
			return fpi_p->rc;

		key_p->l2_key.ingress_device_ptr = (uint64_t)(uintptr_t)in_dev;
		fpi_p->rc = fpi_delete_flow_by_key(key_p);
		break;
	case fpictl_op_getnext:
		index = fpi_p->next_idx;

		if (index == -1)
			fpi_p->rc = idx_pool_first_in_use(&fpi_db.idx_pool,
							  &index);
		else
			fpi_p->rc = idx_pool_next_in_use(&fpi_db.idx_pool,
							 index, &index);
		if (fpi_p->rc < 0) {
			fpi_p->next_idx = -1;
			break;
		}

		fpi_p->next_idx = index;
		flow_entry_p = local_db_get_flow_by_index(index);
		/* skip flow_entry_p == NULL check, it shouldn't happen */
		fpi_p->handle = FPI_HANDLE(flow_entry_p->flow.key.mode, index);
		fpi_update_flow_stat(fpi_p->handle);
		memcpy(&fpi_p->flow, &flow_entry_p->flow, sizeof(fpi_flow_t));
		memcpy(&fpi_p->stat, &flow_entry_p->stat, sizeof(fpi_stat_t));
		memcpy(&fpi_p->wl_info, &flow_entry_p->wl_info,
		       sizeof(fpi_wlan_egress_info_t));
		in_dev = (struct net_device *)(uintptr_t)
			 key_p->l2_key.ingress_device_ptr;
		out_dev = (struct net_device *)(uintptr_t)
			  ctx_p->egress_device_ptr;
		memcpy(key_p->l2_key.ingress_device_name, in_dev->name,
		       IFNAMSIZ);
		if (out_dev != NULL)
			memcpy(ctx_p->egress_device_name, out_dev->name,
			       IFNAMSIZ);
		break;
	default:
		BCM_LOG_ERROR(BCM_LOG_ID_FPI, "Invalid op[%u]", fpi_p->op);
		return -EINVAL;
	}

	if (copy_to_user((uint8_t *)arg, fpi_p, sizeof(fpictl_data_t)))
		return -EFAULT;
	return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fpi_ioctl
 * Description	: Main entry point to handle user applications IOCTL requests
 *		  Flow Provisioning Interface Utility.
 * Returns	: 0 - success or error
 *------------------------------------------------------------------------------
 */
static long fpi_ioctl(struct file *filep, unsigned int command,
		      unsigned long arg)
{
	fpictl_ioctl_t cmd;
	fpictl_data_t pa;
	fpictl_data_t *fpi_p = &pa;
	int ret = 0;
	fpi_mode_t prov_mode = fpi_mode_l2;
	int index, i;

	if (command > fpictl_ioctl_max)
		cmd = fpictl_ioctl_max;
	else
		cmd = (fpictl_ioctl_t)command;

	if (copy_from_user(fpi_p, (uint8_t *)arg, sizeof(pa)) != 0) {
		BCM_LOG_ERROR(BCM_LOG_ID_FPI, "copy_from_user error!");
		return -EINVAL;
	}
	if ((command > fpictl_ioctl_max) ||
	    (fpi_p->subsys > fpictl_subsys_max) ||
	    (fpi_p->op > fpictl_op_max)) {
		BCM_LOG_ERROR(BCM_LOG_ID_FPI, "parameter error! cmd=%d "
			      "subsys=%d op=%d",
			      command, fpi_p->subsys, fpi_p->op);
		return -EINVAL;
	}
	BCM_LOG_DEBUG(BCM_LOG_ID_FPI, "cmd<%d>%s subsys<%d>%s op<%d>%s "
		      "next_idx <%d> arg<0x%lx>",
		      command, fpictl_ioctl_name[command-fpictl_ioctl_sys],
		      fpi_p->subsys, fpictl_subsys_name[fpi_p->subsys],
		      fpi_p->op, fpictl_op_name[fpi_p->op], fpi_p->next_idx,
		      arg);

	if (cmd == fpictl_ioctl_sys) {
		switch (fpi_p->subsys) {
		case fpictl_subsys_mode:
			switch (fpi_p->op) {
			case fpictl_op_get:
				fpi_p->rc = fpi_get_mode(&prov_mode);
				fpi_p->mode = prov_mode;
				if (copy_to_user((uint8_t *)arg, fpi_p,
						 sizeof(pa)))
					ret = -EFAULT;
				break;
			case fpictl_op_set:
				ret = fpi_set_mode(fpi_p->mode);
				fpi_p->rc = ret;
				if (copy_to_user((uint8_t *)arg, fpi_p,
						 sizeof(pa)))
					ret = -EFAULT;
				break;
			default:
				BCM_LOG_ERROR(BCM_LOG_ID_FPI, "Invalid op[%u]",
					      fpi_p->op);
			}
			break;

		case fpictl_subsys_def_prio:
			switch (fpi_p->op) {
			case fpictl_op_get:
				fpi_p->rc = fpi_get_default_priority(
							&fpi_p->def_prio);
				if (copy_to_user((uint8_t *)arg, fpi_p,
						 sizeof(pa)))
					ret = -EFAULT;
				break;
			case fpictl_op_set:
				fpi_set_default_priority(fpi_p->def_prio);
				break;
			default:
				BCM_LOG_ERROR(BCM_LOG_ID_FPI, "Invalid op[%u]",
					      fpi_p->op);
			}
			break;

		case fpictl_subsys_flow:
			ret = fpi_ioctl_subsys_flow(fpi_p, arg);
			break;

		case fpictl_subsys_stat:
			if (fpi_p->op == fpictl_op_get) {
				ret = fpi_get_stat(
						fpi_p->handle, &fpi_p->stat);
				fpi_p->rc = ret;
				if (copy_to_user((uint8_t *)arg, fpi_p,
						 sizeof(pa)))
					ret = -EFAULT;
			} else {
				BCM_LOG_ERROR(BCM_LOG_ID_FPI, "Invalid op[%u]",
					      fpi_p->op);
				ret = -EINVAL;
			}
			break;

		case fpictl_subsys_apmac:
			switch (fpi_p->op) {
			case fpictl_op_add:
				ret = fpi_add_ap_mac(
						fpi_p->flow.context.src_mac);
				fpi_p->rc = ret;
				if (copy_to_user((uint8_t *)arg, fpi_p,
						 sizeof(pa)))
					ret = -EFAULT;
				break;
			case fpictl_op_del_by_key:
				ret = fpi_delete_ap_mac(
						fpi_p->flow.context.src_mac);
				fpi_p->rc = ret;
				if (copy_to_user((uint8_t *)arg, fpi_p,
						 sizeof(pa)))
					ret = -EFAULT;
				break;
			case fpictl_op_getnext:
				index = fpi_p->next_idx + 1;
				ret = -ENOENT;
				for (i = index; i < FPI_MAX_AP_MAC; i++) {
					if (fpi_db.mac_table[i].ref_cnt > 0) {
						memcpy(fpi_p->flow.context.src_mac,
						       fpi_db.mac_table[i].mac,
						       ETH_ALEN);
						fpi_p->next_idx = i;
						ret = 0;
						break;
					}
				}
				fpi_p->rc = ret;
				if (ret == -ENOENT) {
					fpi_p->next_idx = -1;
					ret = 0;
				}
				if (copy_to_user((uint8_t *)arg, fpi_p,
						 sizeof(pa)))
					ret = -EFAULT;
				break;
			default:
				BCM_LOG_ERROR(BCM_LOG_ID_FPI, "Invalid op[%u]",
					      fpi_p->op);
				ret = -EINVAL;
			}
			break;

		case fpictl_subsys_gre_mode:
			switch (fpi_p->op) {
			case fpictl_op_get:
				fpi_p->rc = 0;
				fpi_p->gre_mode = fpi_db.gre_mode;
				if (copy_to_user((uint8_t *)arg, fpi_p,
						 sizeof(pa)))
					ret = -EFAULT;
				break;
			case fpictl_op_set:
				fpi_p->rc = fpi_set_gre_mode(fpi_p->gre_mode);
				if (copy_to_user((uint8_t *)arg, fpi_p,
						 sizeof(pa)))
					ret = -EFAULT;
				break;
			default:
				BCM_LOG_ERROR(BCM_LOG_ID_FPI, "Invalid op[%u]",
					      fpi_p->op);
			}
			break;

		case fpictl_subsys_l2lkp_on_etype:
			switch (fpi_p->op) {
			case fpictl_op_get:
				fpi_p->rc = 0;
				fpi_p->enable = (uint8_t)fpi_db.l2lkp_on_etype;
				fpi_p->flow.key.l2_key.eth_type =
						fpi_db.etype_for_l2lkp;
				if (copy_to_user((uint8_t *)arg, fpi_p,
						 sizeof(pa)))
					ret = -EFAULT;
				break;
			case fpictl_op_set:
				fpi_p->rc = fpi_set_l2lkp_on_etype(
					(bool)fpi_p->enable,
					fpi_p->flow.key.l2_key.eth_type);
				if (copy_to_user((uint8_t *)arg, fpi_p,
						 sizeof(pa)))
					ret = -EFAULT;
				break;
			default:
				BCM_LOG_ERROR(BCM_LOG_ID_FPI, "Invalid op[%u]",
					      fpi_p->op);
			}
			break;

		case fpictl_subsys_lkp_enable:
			switch (fpi_p->op) {
			case fpictl_op_get:
				fpi_p->rc = 0;
				fpi_p->enable = fpi_db.lkp_enable;
				if (copy_to_user((uint8_t *)arg, fpi_p,
						 sizeof(pa)))
					ret = -EFAULT;
				break;
			case fpictl_op_set:
				ret = fpi_set_lkp_enable(fpi_p->enable);
				fpi_p->rc = ret;
				if (copy_to_user((uint8_t *)arg, fpi_p,
						 sizeof(pa)))
					ret = -EFAULT;
				break;
			default:
				BCM_LOG_ERROR(BCM_LOG_ID_FPI, "Invalid op[%u]",
					      fpi_p->op);
			}
			break;

		default:
			BCM_LOG_ERROR(BCM_LOG_ID_FPI, "Invalid subsys[%u]",
				      fpi_p->subsys);
		}
	} else {
		BCM_LOG_ERROR(BCM_LOG_ID_FPI, "Invalid cmd[%u]", command);
		ret = -EINVAL;
	}
	/* TODO! maybe we will always copy to user at the end, doesn't matter
	 * which op / action is taken
	 */

	return ret;
} /* fpi_ioctl */

/*
 *------------------------------------------------------------------------------
 * Function Name: fpi_open
 * Description  : Called when a user application opens this device.
 * Returns      : 0 - success
 *------------------------------------------------------------------------------
 */
static int fpi_open(struct inode *inode, struct file *filp)
{
	BCM_LOG_DEBUG(BCM_LOG_ID_FPI, "Access Flow Provisioning Interface "
				      "Char Device");
	return 0;
} /* fpi_open */

/* Global file ops */
static const struct file_operations fpi_fops = {
	.unlocked_ioctl = fpi_ioctl,
#if defined(CONFIG_COMPAT)
	.compat_ioctl = fpi_ioctl,
#endif
	.open = fpi_open,
};

static void fpi_drv_deinit(void)
{
	if (!IS_ERR(fpi_db.device)) {
		device_destroy(fpi_db.class, MKDEV(fpi_db.major, 0));
		cdev_del(&fpi_db.cdev);
	}

	if (!IS_ERR(fpi_db.class))
		class_destroy(fpi_db.class);

	if (fpi_db.major)
		unregister_chrdev_region(MKDEV(fpi_db.major, 0),
					 FPI_NUM_DEVICES);
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fpi_drv_construct
 * Description  : Initial function that is called at system startup that
 *                registers this device.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
static int fpi_drv_construct(void)
{
	dev_t dev = 0;
	dev_t devno;
	int rc;

	bcmLog_setLogLevel(BCM_LOG_ID_FPI, BCM_LOG_LEVEL_NOTICE);

	rc = alloc_chrdev_region(&dev, 0, FPI_NUM_DEVICES, FPI_DEV_NAME);
	if (rc < 0) {
		pr_err("%s:alloc_chrdev_region() failed\n", __func__);
		return -ENODEV;
	}
	fpi_db.major = MAJOR(dev);

	/* create device and class */
	fpi_db.class = class_create(THIS_MODULE, FPI_DEV_NAME);
	if (IS_ERR(fpi_db.class)) {
		rc = PTR_ERR(fpi_db.class);
		pr_err("%s:Fail to create class %s, rc = %d\n", __func__,
		       FPI_DEV_NAME, rc);
		goto fail;
	}

	devno = MKDEV(fpi_db.major, 0);
	cdev_init(&fpi_db.cdev, &fpi_fops);
	fpi_db.cdev.owner = THIS_MODULE;

	rc = cdev_add(&fpi_db.cdev, devno, 1);
	if (rc) {
		pr_err("%s:Fail to add cdev %s, rc = %d\n", __func__,
		       FPI_DEV_NAME, rc);
		goto fail;
	}

	fpi_db.device = device_create(fpi_db.class, NULL, devno, NULL,
				      FPI_DEV_NAME);
	if (IS_ERR(fpi_db.device)) {
		rc = PTR_ERR(fpi_db.device);
		pr_err("%s:Fail to create device %s, rc = %d\n", __func__,
		       FPI_DEV_NAME, rc);
		goto fail;
	}

	pr_info(FPI_MODNAME " Char Driver " FPI_VERSION " Registered<%d>"
		CLRnl, fpi_db.major);

	return fpi_db.major;

fail:
	fpi_drv_deinit();
	return rc;
}

/*
 *------------------------------------------------------------------------------
 * Function     : fpi_init
 * Description  : Static construction of Flow Provisioning Interface subsystem.
 *------------------------------------------------------------------------------
 */
int __init fpi_init(void)
{
	int rc;
	char owner_name[16];

	memset(&fpi_db, 0, sizeof(fpi_db_t));
	snprintf(owner_name, sizeof(owner_name), "FPI");
	/* clear the HW acceleartor hook */
	memset(&g_hw_info, 0, sizeof(g_hw_info));

	/* in the future, the size of supported entry should be configurable */
	/* adjust max_flow to power of 2, so the mask would work */
	fpi_db.max_flow = FPI_MAX_FLOWS;

	rc = idx_pool_init(&fpi_db.idx_pool,
			    fpi_db.max_flow, owner_name);
	if (rc)
		return rc;

	atomic_set(&fpi_db.num_flow, 0);

	fpi_db.table_p = vmalloc(sizeof(fpi_flow_entry_t) * fpi_db.max_flow);
	if (fpi_db.table_p == NULL) {
		idx_pool_exit(&fpi_db.idx_pool);
		return -ENOMEM;
	}

	rc = fpi_drv_construct();
	if (rc < 0) {
		vfree(fpi_db.table_p);
		idx_pool_exit(&fpi_db.idx_pool);
		return rc;
	}

	pr_info("\nBroadcom Flow Provisioning Interface %s initialized\n",
		FPI_VERSION);

	return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function	: fpi_exit
 * Description  : Destruction of Flow Provisioning Interface subsystem.
 *------------------------------------------------------------------------------
 */
void fpi_exit(void)
{
	fpi_drv_deinit();
	pr_info(FPI_MODNAME " Char Driver " FPI_VERSION " Unregistered" CLRnl);

	vfree(fpi_db.table_p);
	idx_pool_exit(&fpi_db.idx_pool);
	pr_info("\nBroadcom Flow Provisioning Interface uninitialized\n");
}

int fpi_register_hw(const fpi_hw_info_t *hw_info)
{
	if (g_hw_info.registered == 1)
		return -EPERM;

	/* making sure some essential functions are provided by HW */
	if ((hw_info->set_mode == NULL) ||
	    (hw_info->set_default_priority == NULL) ||
	    (hw_info->add_flow == NULL) ||
	    (hw_info->delete_flow == NULL) ||
	    (hw_info->add_ap_mac == NULL) ||
	    (hw_info->delete_ap_mac == NULL) ||
	    (hw_info->get_stat == NULL) ||
	    (hw_info->set_gre_mode == NULL) ||
	    (hw_info->set_lkp_enable == NULL))
		return -EPERM;


	memcpy(&g_hw_info, hw_info, sizeof(g_hw_info));
	g_hw_info.registered = 1;

	/* setting the default mode */
	fpi_set_lkp_enable(true);
	fpi_set_mode(FPI_DEFAULT_MODE);
	fpi_set_default_priority(FPI_DEFAULT_PRIORITY);
	fpi_set_gre_mode(FPI_DEFAULT_GRE_MODE);

	BCM_LOG_INFO(BCM_LOG_ID_FPI, "Done registering HW to Flow "
			"Provisioning Interface");
	return 0;
}
EXPORT_SYMBOL(fpi_register_hw);

int fpi_unregister_hw(fpi_hw_info_t *hw_info)
{
	if (g_hw_info.registered == 0)
		return -EPERM;

	/* making sure the given info matched with the registered one */
	hw_info->registered = 1;
	if (memcmp(hw_info, &g_hw_info, sizeof(g_hw_info)))
		return -EINVAL;

	/* flush all the flows */

	memset(&g_hw_info, 0, sizeof(g_hw_info));

	return 0;
}
EXPORT_SYMBOL(fpi_unregister_hw);

int fpi_register_wl_get_metadata(fpi_get_wl_metadata_func_t func_p)
{
	fpi_db.wl_get_metadata_func_p = func_p;
	return 0;
}
EXPORT_SYMBOL(fpi_register_wl_get_metadata);

int fpi_register_dhd_get_metadata(fpi_get_wl_metadata_func_t func_p)
{
	fpi_db.dhd_get_metadata_func_p = func_p;
	return 0;
}
EXPORT_SYMBOL(fpi_register_dhd_get_metadata);

module_init(fpi_init);
module_exit(fpi_exit);
MODULE_DESCRIPTION(FPI_MODNAME);
MODULE_VERSION(FPI_VERSION);
MODULE_LICENSE("GPL");

