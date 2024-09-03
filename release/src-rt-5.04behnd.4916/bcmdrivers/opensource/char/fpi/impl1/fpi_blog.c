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
 * File Name : fpi_blog.c
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/skbuff.h>
#include <linux/nbuff.h>
#include <linux/blog.h>
#include <linux/bcm_skb_defines.h>
#include <linux/bcm_netdev_path.h>

#include "fpi.h"
#include "idx_pool_util.h"
#include "bcm_OS_Deps.h"

#define FPI_BLOG_FLOW_MAX	256
#define FPI_BLOG_DEL_INACTIVE	10
#define FPI_BLOG_TIMER_INTERVAL	msecs_to_jiffies(10 * 1000)

typedef struct {
	uint32_t handle;
	fpi_stat_t stat;
	int inactive_cnt;	/* to track if the entry is used */
} fpi_blog_entry_t;

typedef struct {
	bool enable;
	atomic_t num_flow;
	IdxPool_t idx_pool;
	fpi_blog_entry_t *table_p;
	struct timer_list timer;
	struct notifier_block netdev_notifier;
} fpi_blog_db_t;

static fpi_blog_db_t fpi_blog_db;

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
DEFINE_SPINLOCK(FPI_BLOG_LOCK_BH);
#define FPI_BLOG_LOCK_BH()	spin_lock_bh(&FPI_BLOG_LOCK_BH)
#define FPI_BLOG_UNLOCK_BH()	spin_unlock_bh(&FPI_BLOG_LOCK_BH)
#else
#define FPI_BLOG_LOCK_BH()	do {} while (0)
#define FPI_BLOG_UNLOCK_BH()	do {} while (0)
#endif

/* return index (>= 0) when succeeds, -1 when fails */
static int fpi_blog_entry_add(uint32_t handle)
{
	int index;
	fpi_blog_entry_t *fpi_blog_entry_p;

	index = idx_pool_get_index(&fpi_blog_db.idx_pool);
	if (index < 0)
		return -EINVAL;

	fpi_blog_entry_p = &fpi_blog_db.table_p[index];
	memset(fpi_blog_entry_p, 0x0, sizeof(fpi_blog_entry_t));
	fpi_blog_entry_p->handle = handle;
	atomic_inc(&fpi_blog_db.num_flow);

	return index;
}

/* 0 when succeeds.  < 0 when fails */
static int fpi_blog_entry_refresh(int index)
{
	fpi_blog_entry_t *fpi_blog_entry_p = &fpi_blog_db.table_p[index];
	fpi_stat_t curr_stat;
	int rc;

	rc = fpi_get_stat(fpi_blog_entry_p->handle, &curr_stat);
	if (rc)
		return rc;

	if (fpi_blog_entry_p->stat.pkt != curr_stat.pkt) {
		/* it's active, so reset inactive_cnt */
		fpi_blog_entry_p->inactive_cnt = 0;
		fpi_blog_entry_p->stat.pkt = curr_stat.pkt;
		fpi_blog_entry_p->stat.byte = curr_stat.byte;
	} else if (++fpi_blog_entry_p->inactive_cnt >= FPI_BLOG_DEL_INACTIVE) {
		/* entry has been inactive for a while, delete it */
		fpi_delete_flow_by_handle(fpi_blog_entry_p->handle);
		pr_info("%s:%d:flow with handle#%08x is deleted\n",
		        __func__, __LINE__, fpi_blog_entry_p->handle);
		memset(fpi_blog_entry_p, 0x0, sizeof(fpi_blog_entry_t));
		idx_pool_return_index(&fpi_blog_db.idx_pool, index);
		atomic_dec(&fpi_blog_db.num_flow);
	}
	return 0;
}

/* a simple timer task should be able to handle it. will see if it needs
 * to use worker thread */
void fpi_blog_timer_callback(struct timer_list *timer_p)
{
	uint32_t index;
	int rc;

	FPI_BLOG_LOCK_BH();

	if (atomic_read(&fpi_blog_db.num_flow) == 0)
		goto reschedule_timer;

	rc = idx_pool_first_in_use(&fpi_blog_db.idx_pool, &index);
	if (rc != 0)
		goto reschedule_timer;

	do {
		fpi_blog_entry_refresh(index);
	} while (idx_pool_next_in_use(&fpi_blog_db.idx_pool, index,
				      &index) == 0);

reschedule_timer:

	FPI_BLOG_UNLOCK_BH();
	mod_timer(&fpi_blog_db.timer, jiffies + FPI_BLOG_TIMER_INTERVAL);

	return;
}

void fpi_blog_table_flush(void)
{
	uint32_t index;
	fpi_blog_entry_t *fpi_blog_entry_p;
	int rc;

	FPI_BLOG_LOCK_BH();

	rc = idx_pool_first_in_use(&fpi_blog_db.idx_pool, &index);
	if (rc < 0)
		goto done_flush;

	do {
		fpi_blog_entry_p = &fpi_blog_db.table_p[index];
		fpi_delete_flow_by_handle(fpi_blog_entry_p->handle);
		pr_info("%s:flow with handle#%08x is deleted\n", __func__,
		        fpi_blog_entry_p->handle);
		memset(fpi_blog_entry_p, 0, sizeof(fpi_blog_entry_t));
		idx_pool_return_index(&fpi_blog_db.idx_pool, index);
		atomic_dec(&fpi_blog_db.num_flow);
	} while (idx_pool_next_in_use(&fpi_blog_db.idx_pool, index,
				      &index) == 0);

done_flush:

	FPI_BLOG_UNLOCK_BH();

	if (atomic_read(&fpi_blog_db.num_flow) != 0)
		pr_warn("%s:flush isn't complete with %d entry left\n",
			__func__, atomic_read(&fpi_blog_db.num_flow));
	else
		pr_info("%s:successfully flush the table\n", __func__);

	return;
}

/* return the number of VLAN tag.  -1, if error. */
static int _fpi_blog_common_parsing(char *hdr_p, BlogHeader_t *bHdr_p,
				    BlogFcArgs_t *args, Blog_t *blog_p)
{
	BlogTuple_t *rx4_p;
	BlogTupleV6_t *rx6_p;
	uint32_t h_proto;
	uint16_t *ul_p;
	BlogIpv4Hdr_t *ip_p;
	BlogIpv6Hdr_t *ip6_p;
	int i, vtag_num = 0;


	bHdr_p->length = 0;
	bHdr_p->count = 0;
	bHdr_p->info.bmap.VLAN_8021Q = 0;

	switch (args->h_proto) {
	case TYPE_ETH:
		break;
	case TYPE_IP:
		h_proto = htons(BLOG_ETH_P_IPV4);
		goto parse_l3;
	default:
		return -1;
	}

	/* Parser Start.  L2/ETH header */
	/* we only copy the MAC addresses but not EthType */
	bHdr_p->info.hdrs |= (1U << ETH_802x);
	bHdr_p->encap[bHdr_p->count++] = ETH_802x;
	memcpy(&bHdr_p->l2hdr[bHdr_p->length], hdr_p,
	       (BLOG_ETH_ADDR_LEN * 2 + BLOG_ETH_TYPE_LEN));
	bHdr_p->length += BLOG_ETH_ADDR_LEN * 2 + BLOG_ETH_TYPE_LEN;
	hdr_p += BLOG_ETH_ADDR_LEN * 2;
	h_proto = *((uint16_t *)hdr_p);
	hdr_p += BLOG_ETH_TYPE_LEN;

	/* learn VLAN tags. we only support 1 tag */
	switch (h_proto) {
	case htons(BLOG_ETH_P_8021Q):
	case htons(BLOG_ETH_P_8021AD):
	case htons(BLOG_ETH_QINQ1):
	case htons(BLOG_ETH_QINQ2):
		bHdr_p->vlan_8021ad = bHdr_p->info.bmap.VLAN_8021Q;
		bHdr_p->info.hdrs |= (1U << VLAN_8021Q);
		bHdr_p->encap[bHdr_p->count++] = VLAN_8021Q;

		memcpy(&bHdr_p->l2hdr[bHdr_p->length], hdr_p,
		       BLOG_VLAN_HDR_LEN);
		bHdr_p->length += BLOG_VLAN_HDR_LEN;

		vtag_num = 1;

		blog_p->vtag[0] = blog_read32_align16((uint16_t *)hdr_p);
		h_proto = *((uint16_t *)hdr_p + 1);
		hdr_p += BLOG_VLAN_HDR_LEN;
		break;

	case htons(BLOG_ETH_P_IPV4):
	case htons(BLOG_ETH_P_IPV6):
		break;

	default:
		return -1;
	};

parse_l3:
	/* parse L3 */
	switch (h_proto) {
	case htons(BLOG_ETH_P_IPV4):
		rx4_p = &bHdr_p->tuple;
		ip_p = (BlogIpv4Hdr_t *)hdr_p;

		/* do not handle IPv4 with option */
		if (unlikely(*(uint8_t *)ip_p != 0x45))
			return -1;

		bHdr_p->info.bmap.PLD_IPv4 = 1;
		rx4_p->daddr = blog_read32_align16((uint16_t *)&ip_p->dAddr);
		rx4_p->saddr = blog_read32_align16((uint16_t *)&ip_p->sAddr);
		ul_p = (uint16_t *)(hdr_p + BLOG_IPV4_HDR_LEN);
		rx4_p->ports = blog_read32_align16(ul_p);
		rx4_p->tos = ip_p->tos;
		blog_p->key.protocol = ip_p->proto;
		break;

	case htons(BLOG_ETH_P_IPV6):
		rx4_p = &bHdr_p->tuple;
		rx6_p = &blog_p->tupleV6;
		ip6_p = (BlogIpv6Hdr_t *)hdr_p;

		bHdr_p->info.bmap.PLD_IPv6 = 1;
		rx6_p->word0 = blog_read32_align16((uint16_t *)ip6_p);

		for (i = 0; i < 4; i++) {
			rx6_p->saddr.p32[i] = blog_read32_align16(
				(uint16_t *)&ip6_p->sAddr.u32[i]);
			rx6_p->daddr.p32[i] = blog_read32_align16(
				(uint16_t *)&ip6_p->dAddr.u32[i]);
		}

		/* we don't support multicast IPv6 */
		if (rx6_p->daddr.p8[0] == 0xFF)
			return -1;

		blog_p->key.protocol = ip6_p->nextHdr;
		ul_p = (uint16_t *)((BlogIpv6Hdr_t *)ip6_p + 1);
		rx6_p->ports = blog_read32_align16(ul_p);

		/* even though rx4_p is used for IPv4 header, but purposely
		 * use it to simplify the logic for later TOS-mangle check */
		rx4_p->tos = PKT_IPV6_GET_TOS_WORD(rx6_p->word0);
		break;

	default:
		/* will only get here if there is one VLAN Tag,
		 * and the inside the VLAN, packet is not IPV4 nor IPV6 */
		return -1;
	};

	return vtag_num;
}


static BlogAction_t fpi_blog_receive(FkBuff_t *fkb_p, void *dev_p,
				     BlogFcArgs_t *args)
{
	Blog_t *blog_p;
	char *hdr_p;
	BlogHeader_t *bHdr_p;
	BlogHash_t blogKey;
	int vtag_num;

	if (fpi_blog_db.enable == false) {
		if (unlikely(_IS_BPTR_(fkb_p->blog_p)))
			WARN_ON(fkb_p->blog_p != NULL);

		return PKT_NORM;
	}

	FPI_BLOG_LOCK_BH();

	/* the following is a simplified parser */
	blog_p = blog_fkb(fkb_p);
	if (unlikely(blog_p == NULL))
		goto pkt_norm_return;

	hdr_p = fkb_p->data;
	bHdr_p = &blog_p->rx;
	blogKey.match = args->key_match;

	if (blogKey.l1_tuple.phyType == BLOG_ENETPHY)
		bHdr_p->info.bmap.BCM_SWC = 1;
	else if ((blogKey.l1_tuple.phyType == BLOG_SPU_DS) ||
			(blogKey.l1_tuple.phyType == BLOG_SPU_US) ||
			(blogKey.l1_tuple.phyType == BLOG_WLANPHY)) {
		/* SPU and WLAN interfaces are allowed; No action */
	} else	/* Unsupported Interfaces; return packet normal */
		goto blog_norm_return;

	bHdr_p->info.channel = blogKey.l1_tuple.channel;
	bHdr_p->info.phyHdr = blogKey.l1_tuple.phy;

	/* for Mcast or Bcast, we always return */
	if (args->h_proto == TYPE_ETH &&
			(((BlogEthHdr_t *)hdr_p)->macDa.u8[0] & 0x1))
		goto blog_norm_return;

	vtag_num = _fpi_blog_common_parsing(hdr_p, bHdr_p, args, blog_p);
	if (vtag_num < 0)
		goto blog_norm_return;
	blog_p->vtag_num = vtag_num;
	blog_p->eth_type = ((BlogEthHdr_t *)
			    (hdr_p + vtag_num * BLOG_VLAN_HDR_LEN))->ethType;

	FPI_BLOG_UNLOCK_BH();
	return PKT_BLOG;

blog_norm_return:

	fkb_release_blog(fkb_p);

pkt_norm_return:

	fkb_p->blog_p = NULL;
	FPI_BLOG_UNLOCK_BH();
	return PKT_NORM;
}

static BlogAction_t fpi_blog_transmit(struct sk_buff *skb_p, void *dev_p,
				      uint32_t h_proto, uint32_t key_match,
				      BlogFcArgs_t *args)
{
	Blog_t *blog_p;
	char *hdr_p;
	BlogHeader_t *tx_bHdr_p, *rx_bHdr_p;
	uint16_t rx_protocol;
	uint32_t rx_vtag;
	BlogTupleV6_t rxTupleV6;
	fpi_flow_t new_flow;
	fpi_context_t *ctx_p;
	uint32_t new_handle;
	uint8_t packet_priority = 0, rx_phyHdrType, tx_phyHdrType;
	int rc, i, vtag_num;

	if (fpi_blog_db.enable == false)
		return PKT_NORM;

	/* quick sanity check */
	if ((skb_p == NULL) || (skb_p->blog_p == NULL) ||
		(h_proto != TYPE_ETH && h_proto != TYPE_IP))
		return PKT_NORM;

	blog_p = skb_p->blog_p;
	args->h_proto = h_proto;

	/* device check */
	if (blog_p->rx_dev_p == NULL)
		return PKT_NORM;
	else {
		rx_phyHdrType = netdev_path_get_hw_port_type(
			(struct net_device *)blog_p->rx_dev_p);
		if ((BLOG_GET_PHYTYPE(rx_phyHdrType) != BLOG_ENETPHY) &&
		    (BLOG_GET_PHYTYPE(rx_phyHdrType) != BLOG_WLANPHY) &&
		    (BLOG_GET_PHYTYPE(rx_phyHdrType) != BLOG_SPU_US) &&
		    (BLOG_GET_PHYTYPE(rx_phyHdrType) != BLOG_SPU_DS))
			return PKT_NORM;
	}

	if (blog_p->tx_dev_p == NULL)
		return PKT_NORM;
	else {
		tx_phyHdrType = netdev_path_get_hw_port_type(
			(struct net_device *)blog_p->tx_dev_p);
		if ((BLOG_GET_PHYTYPE(tx_phyHdrType) != BLOG_ENETPHY) &&
		    (BLOG_GET_PHYTYPE(tx_phyHdrType) != BLOG_WLANPHY) &&
		    (BLOG_GET_PHYTYPE(tx_phyHdrType) != BLOG_SPU_US) &&
		    (BLOG_GET_PHYTYPE(tx_phyHdrType) != BLOG_SPU_DS))
			return PKT_NORM;
	}

	FPI_BLOG_LOCK_BH();

	hdr_p = skb_p->data;
	rx_bHdr_p = &blog_p->rx;
	tx_bHdr_p = &blog_p->tx;

	if (!tx_bHdr_p->info.bmap.ESP) {
		tx_bHdr_p->word = 0; /* clear pktlen */
		tx_bHdr_p->word1 = 0; /* clear pktlen */
	}
	tx_bHdr_p->info.bmap.PLD_L2 = blog_p->rx.info.bmap.PLD_L2;

	blog_p->tupleV6.tunnel = 0; /* initialize tunnel bit */

	/* back up the followings, the common parsing function overwrites them.
	 * blog_p->key.protocol, blog_p->vtag[0], and blog_p->tupleV6 */
	rx_protocol = blog_p->key.protocol;
	rx_vtag = blog_p->vtag[0];
	memcpy(&rxTupleV6, &blog_p->tupleV6, sizeof(BlogTupleV6_t));

	/* parse the TX packet */
	vtag_num = _fpi_blog_common_parsing(hdr_p, tx_bHdr_p, args, blog_p);
	if (vtag_num < 0)
		goto skip_learning;
	blog_p->vtag_tx_num = vtag_num;

	memset(&new_flow, 0x0, sizeof(fpi_flow_t));

	/* decision making */
	rc = fpi_get_mode(&new_flow.key.mode);
	if (rc)
		goto skip_learning;

	/* adjust the mode. if h_proto is TYPE_IP, force it to L3L4 mode */
	if ((new_flow.key.mode != fpi_mode_l3l4) && h_proto == TYPE_IP)
		new_flow.key.mode = fpi_mode_l3l4;

	/* if rxHdr->DST_MAC passes AP_MAC check, force the mode to L3L4. */
	if ((new_flow.key.mode != fpi_mode_l3l4) &&
	    fpi_check_ap_mac(rx_bHdr_p->l2hdr))
		new_flow.key.mode = fpi_mode_l3l4;

	/* first, let's get the RX packet priority */
	if (rx_bHdr_p->info.bmap.PLD_IPv4 == 1)
		packet_priority = BLOG_IPTOS2DSCP(rx_bHdr_p->tuple.tos);
	else if (rx_bHdr_p->info.bmap.PLD_IPv6 == 1)
		packet_priority = BLOG_IPTOS2DSCP(
				PKT_IPV6_GET_TOS_WORD(rxTupleV6.word0));
	else if (rx_bHdr_p->info.bmap.VLAN_8021Q == 1)
		packet_priority = (ntohl(rx_vtag) >> 29) & 0x7;

	if (new_flow.key.mode == fpi_mode_l2) {
		new_flow.key.l2_key.ingress_device_ptr =
				(uint64_t)(uintptr_t)blog_p->rx_dev_p;
		memcpy(new_flow.key.l2_key.dst_mac,
		       &rx_bHdr_p->l2hdr[0], BLOG_ETH_ADDR_LEN);
		memcpy(new_flow.key.l2_key.src_mac,
		       &rx_bHdr_p->l2hdr[BLOG_ETH_ADDR_LEN],
		       BLOG_ETH_ADDR_LEN);
		new_flow.key.l2_key.eth_type = ntohs(blog_p->eth_type);
		new_flow.key.l2_key.vtag_num = blog_p->vtag_num;
		new_flow.key.l2_key.packet_priority = packet_priority;

		goto prepare_context;
	}

	/* now we check if L3L4 flow can be created */
	/* note: small assumption here, supposedly we need to check if
	 * DST MAC == AP MAC, but it is skipped. */

	/* we only support case where RX and TX IP Version are the same.
	 * no 6in4 nor 4in6 support yet */
	if ((rx_bHdr_p->info.bmap.PLD_IPv4 != tx_bHdr_p->info.bmap.PLD_IPv4) ||
	    (rx_bHdr_p->info.bmap.PLD_IPv6 != tx_bHdr_p->info.bmap.PLD_IPv6))
		goto skip_learning;

	/* it has to be either IPv4 or IPv6 */
	if ((rx_bHdr_p->info.bmap.PLD_IPv4 == 0) &&
	    (rx_bHdr_p->info.bmap.PLD_IPv6 == 0))
		goto skip_learning;

	if (((rx_protocol != blog_p->key.protocol) &&
			(tx_phyHdrType != BLOG_SPU_US)) ||
	    ((rx_protocol != BLOG_IPPROTO_UDP) &&
			(rx_protocol != BLOG_IPPROTO_TCP) &&
			(rx_protocol != BLOG_IPPROTO_ESP)))
		goto skip_learning;

	/* we can now create L3L4 Flow.  Fill in Key info. */
	new_flow.key.l3l4_key.ingress_device_ptr =
			(uint64_t)(uintptr_t)blog_p->rx_dev_p;
	new_flow.key.l3l4_key.vtag_num = blog_p->vtag_num;
	new_flow.key.l3l4_key.l4_proto = rx_protocol;
	new_flow.key.l3l4_key.packet_priority = packet_priority;

	if (rx_bHdr_p->info.bmap.PLD_IPv4) {
		new_flow.key.l3l4_key.src_ip[0] = ntohl(rx_bHdr_p->tuple.saddr);
		new_flow.key.l3l4_key.dst_ip[0] = ntohl(rx_bHdr_p->tuple.daddr);
		new_flow.key.l3l4_key.src_port =
				ntohs(rx_bHdr_p->tuple.port.source);
		new_flow.key.l3l4_key.dst_port =
				ntohs(rx_bHdr_p->tuple.port.dest);
	} else { /* if (rx_bHdr_p->info.bmap.PLD_IPv6) */
		new_flow.key.l3l4_key.is_ipv6 = 1;
		for (i = 0; i < 4; i++) {
			new_flow.key.l3l4_key.src_ip[i] =
					ntohl(rxTupleV6.saddr.p32[i]);
			new_flow.key.l3l4_key.dst_ip[i] =
					ntohl(rxTupleV6.daddr.p32[i]);
		}
		new_flow.key.l3l4_key.src_port = ntohs(rxTupleV6.port.source);
		new_flow.key.l3l4_key.dst_port = ntohs(rxTupleV6.port.dest);
	}

	if (args->esp_over_udp) {
		new_flow.key.l3l4_key.esp_spi_mode = FPI_ESP_IN_UDP;
		new_flow.key.l3l4_key.esp_spi = ntohl(args->esp_spi);
	} else if (tx_phyHdrType == BLOG_SPU_DS || tx_phyHdrType == BLOG_SPU_US) {
		new_flow.key.l3l4_key.esp_spi_mode = FPI_ESP_IN_IP;
		new_flow.key.l3l4_key.esp_spi = ntohl(tx_bHdr_p->tuple.esp_spi);
	}


prepare_context:

	ctx_p = &new_flow.context;
	ctx_p->egress_device_ptr = (uint64_t)(uintptr_t)blog_p->tx_dev_p;

	if (tx_phyHdrType == BLOG_WLANPHY) {
		if (blog_p->wfd.nic_ucast.is_wfd == 1) {
			ctx_p->egress_priority = blog_p->wfd.nic_ucast.wfd_prio;
			if (blog_p->wfd.nic_ucast.is_chain == 1)
				ctx_p->wl_user_priority =
					blog_p->wfd.nic_ucast.priority >> 1;
			else
				ctx_p->wl_user_priority =
					blog_p->wfd.dhd_ucast.priority;
		} else {
			ctx_p->wl_user_priority = blog_p->rnr.flow_prio << 1;
			ctx_p->egress_priority = 0;
		}
	} else
		ctx_p->egress_priority = SKBMARK_GET_Q_PRIO(blog_p->mark);

	if (blog_p->vtag_num) {
		ctx_p->vtag_check = 1;
		ctx_p->vtag_value = (uint16_t)(ntohl(rx_vtag) >> 16) & 0x0fff;
	}

	/* the only case that we support inserting vlan tag */
	if ((blog_p->vtag_num == 0) && (blog_p->vtag_tx_num == 1)) {
		ctx_p->vlan_8021q_prepend = 1;
		ctx_p->vlan_8021q_hdr = ntohl(blog_read32_align16(
			(uint16_t *)&tx_bHdr_p->l2hdr[BLOG_ETH_ADDR_LEN * 2]));
	}

	if ((blog_p->vtag_num == 1) && (blog_p->vtag_tx_num == 0)) {
		ctx_p->vlan_8021q_remove = 1;
	}

	if ((blog_p->vtag_num == 1) && (blog_p->vtag_num == 1) &&
	    (rx_vtag != blog_p->vtag[0])) {
		ctx_p->vlan_8021q_remove = 1;
		ctx_p->vlan_8021q_prepend = 1;
		ctx_p->vlan_8021q_hdr = ntohl(blog_read32_align16(
			(uint16_t *)&tx_bHdr_p->l2hdr[BLOG_ETH_ADDR_LEN * 2]));
	}

	if (BLOG_IPTOS2DSCP(rx_bHdr_p->tuple.tos) !=
            BLOG_IPTOS2DSCP(tx_bHdr_p->tuple.tos)) {
		ctx_p->dscp_rewrite = 1;
		ctx_p->dscp = BLOG_IPTOS2DSCP(tx_bHdr_p->tuple.tos);
	}

	ctx_p->max_ingress_packet_size = blog_getTxMtu(blog_p) +
					 blog_p->rx.length;

	if (new_flow.key.mode != fpi_mode_l3l4)
		goto add_flow;

	if (new_flow.key.l3l4_key.esp_spi_mode == FPI_ESP_IGNORED) {
		memcpy(ctx_p->dst_mac, &tx_bHdr_p->l2hdr[0], BLOG_ETH_ADDR_LEN);
		memcpy(ctx_p->src_mac, &tx_bHdr_p->l2hdr[BLOG_ETH_ADDR_LEN],
		       BLOG_ETH_ADDR_LEN);

		/* check if SRC/DST IP/PORT got changed, if so, napt_enable */
		if (new_flow.key.l3l4_key.is_ipv6 == 1) {
			if ((memcmp(rxTupleV6.saddr.p8, blog_p->tupleV6.saddr.p8, 16)) ||
			    (memcmp(rxTupleV6.daddr.p8, blog_p->tupleV6.daddr.p8, 16)) ||
			    (rxTupleV6.ports != blog_p->tupleV6.ports)) {
				ctx_p->napt_enable = 1;
				for (i = 0; i < 4; i++) {
					ctx_p->src_ip[i] = ntohl(
						blog_p->tupleV6.saddr.p32[i]);
					ctx_p->dst_ip[i] = ntohl(
						blog_p->tupleV6.daddr.p32[i]);
				}
				ctx_p->src_port =
					ntohs(blog_p->tupleV6.port.source);
				ctx_p->dst_port =
					ntohs(blog_p->tupleV6.port.dest);
			} else
				ctx_p->napt_enable = 0;
		} else { /* IPv4 */
			if ((rx_bHdr_p->tuple.saddr != tx_bHdr_p->tuple.saddr) ||
			    (rx_bHdr_p->tuple.daddr != tx_bHdr_p->tuple.daddr) ||
			    (rx_bHdr_p->tuple.ports != tx_bHdr_p->tuple.ports)) {
				ctx_p->napt_enable = 1;
				ctx_p->src_ip[0] =
					ntohl(tx_bHdr_p->tuple.saddr);
				ctx_p->dst_ip[0] =
					ntohl(tx_bHdr_p->tuple.daddr);
				ctx_p->src_port =
					ntohs(tx_bHdr_p->tuple.port.source);
				ctx_p->dst_port =
					ntohs(tx_bHdr_p->tuple.port.dest);
			} else
				ctx_p->napt_enable = 0;
		}
	} else {
		if (tx_phyHdrType == BLOG_SPU_US) {
			if (new_flow.key.l3l4_key.is_ipv6 == 1) {
				for (i = 0; i < 4; i++) {
					ctx_p->src_ip[i] = ntohl(
						blog_p->tupleV6.saddr.p32[i]);
					ctx_p->dst_ip[i] = ntohl(
						blog_p->tupleV6.daddr.p32[i]);
				}
			} else {
				ctx_p->src_ip[0] =
					ntohl(tx_bHdr_p->tuple.saddr);
				ctx_p->dst_ip[0] =
					ntohl(tx_bHdr_p->tuple.daddr);
			}
			if (args->esp_over_udp) {
				ctx_p->src_port =
					ntohs(tx_bHdr_p->tuple.port.source);
				ctx_p->dst_port =
					ntohs(tx_bHdr_p->tuple.port.dest);
			}
		}
	}

add_flow:
	/* Trigger FPI to create the flow!! */
	rc = fpi_add_flow(&new_flow, &new_handle);
	if (rc)
		goto skip_learning;

	rc = fpi_blog_entry_add(new_handle);
	if (rc < 0) {
		/* fails to add it to the local table. delete the entry */
		fpi_delete_flow_by_handle(new_handle);
		/* goto skip_learning; comment out as this jump is redudant */
	} else
		pr_info("%s:%d:successfully added new flow with handle#%08x\n",
		        __func__, __LINE__, new_handle);

skip_learning:

	FPI_BLOG_UNLOCK_BH();
	return PKT_NORM;
}

static void fpi_blog_bind_blog(int enable_f)
{
	BlogBind_t hook_info;

	if (enable_f != 0) {
		hook_info.bmap.RX_HOOK = 1;
		hook_info.bmap.TX_HOOK = 1;
		hook_info.bmap.XX_HOOK = 0;
		hook_info.bmap.QR_HOOK = 0;
		hook_info.bmap.SC_HOOK = 0;
		hook_info.bmap.SD_HOOK = 0;
		hook_info.bmap.FA_HOOK = 0;
		hook_info.bmap.FD_HOOK = 0;
		hook_info.bmap.BM_HOOK = 0;
		hook_info.bmap.HC_HOOK = 0;
		hook_info.bmap.GLC_HOOK = 0;

		/* Bind handlers */
		blog_bind(fpi_blog_receive, fpi_blog_transmit, NULL, NULL, NULL,
			  NULL, hook_info);
		blog_bind_config(NULL, NULL, NULL, hook_info);
	} else {
		/* unbind RX and TX hooks with blog */
		hook_info.bmap.RX_HOOK = 1;
		hook_info.bmap.TX_HOOK = 1;
		blog_bind(NULL, NULL, NULL, NULL, NULL, NULL, hook_info);
	}
}

/*
 * To receive notifications of link state changes and device down, so we
 * can update AP MAC address.
 *------------------------------------------------------------------------------
 */
int fpi_blog_netdev_notifier(struct notifier_block *this, unsigned long event,
			     void *dev_ptr)

{
	struct net_device *dev_p = NETDEV_NOTIFIER_GET_DEV(dev_ptr);
	char *dev_addr;

	if (!blog_is_config_netdev_mac(dev_p, 0))
		return NOTIFY_DONE;

	dev_addr = (char *)blog_request(NETDEV_ADDR, dev_p, 0, 0);
	pr_debug("%s:%d:dev<%s> dev_p<%px> event<%lu> <%pM> vs (%pM)\n",
		 __func__, __LINE__, dev_p->name, dev_p, event,
		 dev_p->dev_addr, dev_addr);

	switch (event) {
	case NETDEV_UP:
		fpi_add_ap_mac((uint8_t *)dev_addr);
		break;

	case NETDEV_CHANGE:
		if (blog_request(LINK_NOCARRIER, dev_p, 0, 0))
			fpi_delete_ap_mac((uint8_t *)dev_addr);
		else
			fpi_add_ap_mac((uint8_t *)dev_addr);
		break;

	case NETDEV_DOWN:
		fpi_delete_ap_mac((uint8_t *)dev_addr);
		break;

	case NETDEV_CHANGEADDR:
		/* in this demo, we choose not to implement this event
		 * notifier due to the extra book keeping required to obtain
		 * the old MAC address that needs to be deleted */
#if 0
		fpi_delete_ap_mac((uint8_t *)old_addr);
		fpi_add_ap_mac((uint8_t *)dev_addr);
#endif
		break;

	default:
	case NETDEV_GOING_DOWN:
	case NETDEV_CHANGEMTU:
	case NETDEV_REGISTER:
	case NETDEV_UNREGISTER:
		break;
	}

	return NOTIFY_DONE;
}

#define PROC_DIR		"driver/fpi"
#define ENABLE_PROC_FILE	"enable"
#define FLUSH_PROC_FILE		"flush"

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *enable_proc_file;
static struct proc_dir_entry *flush_proc_file;

static int enable_proc_show(struct seq_file *m, void *v)
{
	int val;

	if (fpi_blog_db.enable)
		val = 1;
	else
		val = 0;

	seq_printf(m, "%d\n", val);

	return 0;
}

static int enable_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, enable_proc_show, PDE_DATA(inode));
}

static ssize_t enable_proc_write(struct file *file, const char *buf,
				 size_t len, loff_t *offset)
{
	char temp_buf[16];
	int count;
	unsigned long input_val;

	if (len >= sizeof(temp_buf)) {
		pr_err("invalid input value\n");
		return len;
	}

	count = min((unsigned long)len, (unsigned long)(sizeof(temp_buf) - 1));

	if (copy_from_user(temp_buf, buf, count) != 0) {
		pr_err("invalid input value\n");
		return len;
	}

	temp_buf[count] = '\0';
	if (kstrtoul(temp_buf, 0, &input_val)) {
		pr_err("invalid input value\n");
		return len;
	}

	/* input_val can only be 0 and 1 */
	if (input_val > 1) {
		pr_err("invalid input value\n");
		return len;
	}

	FPI_BLOG_LOCK_BH();
	if (input_val == 0)
		fpi_blog_db.enable = false;
	else
		fpi_blog_db.enable = true;
	FPI_BLOG_UNLOCK_BH();

	if (fpi_blog_db.enable == false)
		fpi_blog_table_flush();

	return len;
}

static int flush_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "Write anything to flush\n");
	return 0;
}

static int flush_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, flush_proc_show, PDE_DATA(inode));
}

static ssize_t flush_proc_write(struct file *file, const char *buf,
				size_t len, loff_t *offset)
{
	char temp_buf[16];
	int count;

	count = min((unsigned long)len, (unsigned long)(sizeof(temp_buf) - 1));

	if (copy_from_user(temp_buf, buf, count) != 0)
		return -EFAULT;

	fpi_blog_table_flush();

	return len;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5,10,0))
static const struct proc_ops enable_proc_ops = {
	.proc_open	= enable_proc_open,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
	.proc_write	= enable_proc_write,
};

static const struct proc_ops flush_proc_ops = {
	.proc_open	= flush_proc_open,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
	.proc_write	= flush_proc_write,
};
#else
static const struct file_operations enable_proc_ops = {
	.owner		= THIS_MODULE,
	.open 		= enable_proc_open,
	.read 		= seq_read,
	.llseek 	= seq_lseek,
	.release 	= single_release,
	.write		= enable_proc_write,
};

static const struct file_operations flush_proc_ops = {
	.owner		= THIS_MODULE,
	.open 		= flush_proc_open,
	.read 		= seq_read,
	.llseek 	= seq_lseek,
	.release 	= single_release,
	.write		= flush_proc_write,
};
#endif

static void fpi_proc_exit(void)
{
	if (enable_proc_file) {
		remove_proc_entry(ENABLE_PROC_FILE, proc_dir);
		enable_proc_file = NULL;
	}
	if (flush_proc_file) {
		remove_proc_entry(FLUSH_PROC_FILE, proc_dir);
		flush_proc_file = NULL;
	}
	if (proc_dir) {
		remove_proc_entry(PROC_DIR, NULL);
		proc_dir = NULL;
	}
}

static int __init fpi_proc_init(void)
{
	int status = 0;

	proc_dir = proc_mkdir(PROC_DIR, NULL);
	if (!proc_dir) {
		pr_err("Failed to create PROC directory %s.\n",
		       PROC_DIR);
		status = -EIO;
		goto done;
	}

	enable_proc_file = proc_create(ENABLE_PROC_FILE, S_IRUSR | S_IWUSR,
				       proc_dir, &enable_proc_ops);
	if (!enable_proc_file) {
		pr_err("Failed to create %s\n", ENABLE_PROC_FILE);
		status = -EIO;
		fpi_proc_exit();
		goto done;
	}

	flush_proc_file = proc_create(FLUSH_PROC_FILE, S_IRUSR | S_IWUSR,
				      proc_dir, &flush_proc_ops);
	if (!flush_proc_file) {
		pr_err("Failed to create %s\n", FLUSH_PROC_FILE);
		status = -EIO;
		fpi_proc_exit();
		goto done;
	}

done:
	return status;
}

static int __init fpi_blog_init(void)
{
	int rc;
	char owner_name[16];

	memset(&fpi_blog_db, 0, sizeof(fpi_blog_db_t));
	snprintf(owner_name, sizeof(owner_name), "FPI_BLOG");
	rc = idx_pool_init(&fpi_blog_db.idx_pool, FPI_BLOG_FLOW_MAX, owner_name);
	if (rc)
		return rc;

	atomic_set(&fpi_blog_db.num_flow, 0);
	fpi_blog_db.enable = false;

	fpi_blog_db.table_p = vmalloc(sizeof(fpi_blog_entry_t) * FPI_BLOG_FLOW_MAX);
	if (fpi_blog_db.table_p == NULL) {
		idx_pool_exit(&fpi_blog_db.idx_pool);
		return -ENOMEM;
	}

	timer_setup(&fpi_blog_db.timer, fpi_blog_timer_callback, 0);
	mod_timer(&fpi_blog_db.timer, jiffies + FPI_BLOG_TIMER_INTERVAL);

	fpi_blog_bind_blog(1);

	fpi_blog_db.netdev_notifier.notifier_call = fpi_blog_netdev_notifier;
	register_netdevice_notifier(&fpi_blog_db.netdev_notifier);

	fpi_proc_init();

	pr_info("Broadcom Sample FPI Control Dataplane Registered!\n");

	return 0;
}

static void __exit fpi_blog_exit(void)
{
	unregister_netdevice_notifier(&fpi_blog_db.netdev_notifier);

	fpi_blog_bind_blog(0);

	fpi_proc_exit();
	del_timer(&fpi_blog_db.timer);
	fpi_blog_table_flush();
	vfree(fpi_blog_db.table_p);
	idx_pool_exit(&fpi_blog_db.idx_pool);

	pr_info("Broadom Sample FPI Control Dataplane Unregistered!\n");
}

module_init(fpi_blog_init);
module_exit(fpi_blog_exit);
MODULE_DESCRIPTION("FPI_BLOG");
MODULE_VERSION("0.1beta");
MODULE_LICENSE("GPL");

