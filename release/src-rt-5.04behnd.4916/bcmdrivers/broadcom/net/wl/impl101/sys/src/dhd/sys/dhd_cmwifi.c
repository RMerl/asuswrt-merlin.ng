/*
 * Broadcom Dongle Host Driver (DHD), CM WiFi Linux-specific impl.
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_cmwifi.c 826042 2023-06-08 00:25:08Z $
 */

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/rtnetlink.h>
#include <linux/etherdevice.h>
#include <linux/random.h>
#include <linux/spinlock.h>
#include <linux/ethtool.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/ip.h>
#include <linux/reboot.h>
#include <linux/notifier.h>
#include <net/addrconf.h>
#include <net/sch_generic.h>
#ifdef ENABLE_ADAPTIVE_SCHED
#include <linux/cpufreq.h>
#endif /* ENABLE_ADAPTIVE_SCHED */

#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>

#include <bcmipv6.h>
#include <dngl_stats.h>
#include <dhd_linux_wq.h>
#include <dhd.h>
#include <dhd_linux.h>

#include <ethernet.h>

#include <dhd_flowring.h>
#include <dhd_bus.h>
#include <dhd_proto.h>
#include <dhd_dbg.h>
#include <wl_core.h>

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

#ifdef DHD_WMF
#include <dhd_wmf_linux.h>
#endif /* DHD_WMF */

#ifdef CMWIFI
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#ifdef CMWIFI_EROUTER
#include <bcmnethooks.h>
#include <dqnet.h>
#include <dhd_cmwifi.h>
#include <wl_erouter.h>
#endif /* CMWIFI_EROUTER */
#endif /* CMWIFI */

#if defined(BCM_DHD_RUNNER)
#include <dhd_runner.h>
#endif /* BCM_DHD_RUNNER */

#if defined(DHD_TCP_WINSIZE_ADJUST)
#include <linux/tcp.h>
#include <net/tcp.h>
#endif /* DHD_TCP_WINSIZE_ADJUST */

#if defined(CMWIFI) && defined(BCM_DHD_RUNNER)

/* CM dqnet and wl integ defines */
#define DQNET_PRIORITY(skb)                   (skb->priority & 0x7)
#define DQNET_GET_FLOWRING(skb)               (skb->priority)
#define DQNET_SET_FLOWRING(skb, prio, flowid) (skb->priority = prio << 16 | flowid)
#define DQNET_INVALID_FLOWRING(skb)           (skb->priority = DHD_OF_BADPARAM)

void
dhd_offload_wmf_init(dhd_pub_t *dhdp)
{
	dhd_cm_ctx_t *cm_ctx = MALLOC(dhdp->osh, sizeof(dhd_cm_ctx_t));
	ASSERT(cm_ctx != NULL);

	if (cm_ctx == NULL) {
		DHD_ERROR(("%s: wl%d cm_ctx MALLOC error\n", __FUNCTION__, dhdp->unit));
		return;
	}

	/* initial CM DoR stuffs */
	memset(cm_ctx, 0, sizeof(dhd_cm_ctx_t));

	spin_lock_init(&cm_ctx->lock);
	cm_ctx->cm_dor_wmf_txq = MALLOC(dhdp->osh, sizeof(struct sk_buff_head));
	ASSERT(cm_ctx->cm_dor_wmf_txq != NULL);

	if (cm_ctx->cm_dor_wmf_txq == NULL) {
		MFREE(dhdp->osh, cm_ctx, sizeof(dhd_cm_ctx_t));
		DHD_ERROR(("%s: wl%d cm_dor_wmf_txq MALLOC error\n", __FUNCTION__, dhdp->unit));
		return;
	}

	cm_ctx->pub = dhdp;

	if (cm_ctx->cm_dor_wmf_txq) {
		memset(cm_ctx->cm_dor_wmf_txq, 0, sizeof(struct sk_buff_head));

		skb_queue_head_init(cm_ctx->cm_dor_wmf_txq);
		cm_ctx->cm_dor_state = 1;

#if defined(BCM_DHD_TX_ON_RUNNER) && defined(DQNET_DOR_API_V2)
		/* DQNET v2 DoR WMF APIs, 0 if not defined */
		cm_ctx->cm_dor_ver = 2;
#endif /* BCM_DHD_TX_ON_RUNNER && DQNET_DOR_API_V2 */
	}

	DHD_CM_SET_CTX(dhdp, cm_ctx);
}

static void
dhd_dor_wmf_txq_reset(dhd_pub_t *dhdp)
{
	dhd_cm_ctx_t *cm_ctx = DHD_CM_CTX(dhdp);

	cm_ctx->cm_dor_state = 0;
	cm_ctx->cm_dor_wmf_pkt_cnt = 0;
}

void
dhd_offload_wmf_dump(dhd_pub_t *dhdp)
{
}

void
dhd_offload_wmf_deinit(dhd_pub_t *dhdp)
{
	dhd_cm_ctx_t *cm_ctx = DHD_CM_CTX(dhdp);
	unsigned long flags;

	/* free up cm_dor_wmf_txq */
	if (cm_ctx != NULL) {
		struct sk_buff *skb;

		DHD_CM_CTX_LOCK(cm_ctx, flags);
		/* free up skb */
		if (cm_ctx->cm_dor_wmf_txq != NULL) {
			while ((skb = skb_dequeue(cm_ctx->cm_dor_wmf_txq)) != NULL) {
				PKTFREE(dhdp->osh, skb, TRUE);
			}
		}

		MFREE(dhdp->osh, cm_ctx->cm_dor_wmf_txq, sizeof(struct sk_buff_head));
		DHD_CM_CTX_UNLOCK(cm_ctx, flags);

		MFREE(dhdp->osh, cm_ctx, sizeof(dhd_cm_ctx_t));
		DHD_CM_SET_CTX(dhdp, NULL);
	}
}

#define DHD_OF_BADPARAM (-1)

/* Map a packet buffer into a flow ring id
 * TBD: replace dhd_get_flowid with dhd_flowid_update
 */
int
dhd_offload_map_flowring(void *ctx, u8 *buf, int buf_len, unsigned char priority,
	int if_idx, int *radio, int *flowring)
{
	int ret = BCME_OK;
	struct ether_header *eh = (struct ether_header *)buf;
	dhd_cm_ctx_t *cm_ctx = (dhd_cm_ctx_t *) ctx;
	dhd_pub_t *dhdp = (dhd_pub_t *) cm_ctx->pub;
	uint8 prio = priority;
	uint16 flowid;
	struct sk_buff skb;

	ASSERT(if_idx < DHD_MAX_IFS);

	DHD_INFO(("Mapping flow: len=%d radio=%d prio=%d if=%d src=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x"
		" dst=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
		buf_len, dhdp->unit, prio, if_idx,
		eh->ether_shost[0], eh->ether_shost[1], eh->ether_shost[2],
		eh->ether_shost[3], eh->ether_shost[4], eh->ether_shost[5],
		eh->ether_dhost[0], eh->ether_dhost[1], eh->ether_dhost[2],
		eh->ether_dhost[3], eh->ether_dhost[4], eh->ether_dhost[5]));

	if (!dhdp->flowid_allocator) {
		DHD_ERROR(("%s: Flow ring not intited yet  \n", __FUNCTION__));
		*radio    = DHD_OF_BADPARAM;
		*flowring = DHD_OF_BADPARAM;
		return BCME_ERROR;
	}

	skb.data = buf;
	skb.priority = prio;

#ifndef PKTPRIO_OVERRIDE
	if (PKTPRIO(&skb) == 0)
#endif
	{
		pktsetprio(&skb, FALSE);
		prio = PKTPRIO(&skb);
	}

	prio = dhdp->flow_prio_map[prio];

	if (dhd_get_flowid(dhdp, if_idx, prio, eh->ether_shost, eh->ether_dhost,
		&flowid) != BCME_OK) {
		*radio    = DHD_OF_BADPARAM;
		*flowring = DHD_OF_BADPARAM;
		ret = BCME_ERROR;
	} else {
		*radio = dhdp->unit;
		*flowring = (int)((prio << 16) | flowid);
	}

	DHD_INFO(("Flow mapped: radio=%d, flowring=%08x flowid=%d prio=%d ret=%d\n",
		*radio, (unsigned int)*flowring, flowid, prio, ret));

	return (ret);
}

/** Is packet transmission allowed to a STA? */
void *
dhd_find_intrabss_sta(void *ctx, int ifidx, void *ea)
{
	dhd_cm_ctx_t *cm_ctx = (dhd_cm_ctx_t *) ctx;
	dhd_pub_t *pub = cm_ctx->pub;

	ASSERT(ea != NULL);

	/*  Not allowed when AP isolation is on or intf role is not AP */
	if (DHD_IFP_AP_ISOLATE(pub, ifidx) || !DHD_IF_ROLE_AP(pub, ifidx)) {
		return NULL;
	}

	/* Always allowed for multicast and broadcast  */
	if (ETHER_ISMULTI(ea))
		return ea;

	/* For unicast, only allowed when STA has been learned */
	return (void *) dhd_find_sta(pub, ifidx, ea);
}

int
dhd_tx_skb_nethook(void *ctx, int ifidx, void *p, struct sk_buff_head *txq)
{
	dhd_cm_ctx_t *cm = (dhd_cm_ctx_t *) ctx;
	dhd_pub_t *dhdp = (dhd_pub_t *) cm->pub;
#ifdef DOR_USE_FLOWRING_SKB
	uint8 prio;
	uint16 flowid;
	struct sk_buff skb_prio;
#endif /* DOR_USE_FLOWRING_SKB */
	int wmf_ret = -1, ret = BCM_NETHOOK_PASS, dor_txq_len;
	struct sk_buff *skb = (struct sk_buff *)p;
	struct ether_header *eh;
	unsigned long flags;

	ASSERT(dhdp != NULL);
	ASSERT(skb != NULL);
	ASSERT(txq != NULL);
	ASSERT(ifidx < DHD_MAX_IFS);

	if (!dhdp->flowid_allocator ||
		(DHD_IF_ROLE_AP(dhdp, ifidx) && !DHD_IF_BSS_UP(dhdp, ifidx))) {
		DHD_INFO(("%s: Flow ring 0x%p not initiated, or bss ifidx %d not up yet\n",
			__FUNCTION__, dhdp->flowid_allocator, ifidx));
		return BCM_NETHOOK_DROP;
	}

#ifdef DOR_USE_FLOWRING_SKB
	/* default DoR enable handling, priority */
	prio = DQNET_PRIORITY(skb);
	skb_prio.data = skb->data;
	skb_prio.priority = prio;

#ifndef PKTPRIO_OVERRIDE
	if (PKTPRIO(&skb_prio) == 0)
#endif
	{
		pktsetprio(&skb_prio, FALSE);
		prio = PKTPRIO(&skb_prio);
	}

	prio = dhdp->flow_prio_map[prio];
#endif /* DOR_USE_FLOWRING_SKB */

	eh = (struct ether_header *) skb->data;

	DHD_INFO(("%s,%d dhdp->unit=%d ifidx=%d src=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x"
		" dst=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x ether_type=0x%x\n",
		__FUNCTION__, __LINE__,
		dhdp->unit, ifidx,
		eh->ether_shost[0], eh->ether_shost[1], eh->ether_shost[2],
		eh->ether_shost[3], eh->ether_shost[4], eh->ether_shost[5],
		eh->ether_dhost[0], eh->ether_dhost[1], eh->ether_dhost[2],
		eh->ether_dhost[3], eh->ether_dhost[4], eh->ether_dhost[5],
		ntoh16(eh->ether_type)));

	if (DHD_IF_WMF_ENABLE(dhdp, ifidx) && ETHER_ISMULTI(eh)) {
		/* WMF converted unicast frames handling */
		wmf_ret = dhd_wmf_packets_handle(dhdp, skb, DHD_IFP(dhdp, ifidx), ifidx, 0);

		if (wmf_ret == WMF_DROP) {
			/* Packet DROP decision by WMF. Toss it, but with CM DoR,  */
			DHD_INFO(("%s: WMF decides to drop packet 0\n", __FUNCTION__));
			/* dqnet will print and free this skb. */
			ret = BCM_NETHOOK_DROP;
			return ret;
		} else if (wmf_ret == WMF_TAKEN) {
			/* Either taken by WMF or we should drop it. Exiting send path */
			/* dqnet will do only statistic counting, skb will be freed by owner */
			ret = BCM_NETHOOK_CONSUMED;
			/* Continue to check cm_dor_wmf_txq */
		}
	}

	/* BCM_NETHOOK_PASS, the dqnet will process txq. */
	DHD_CM_CTX_LOCK(cm, flags);
	dor_txq_len = skb_queue_len(cm->cm_dor_wmf_txq);
	DHD_INFO(("%s,%d: cm_dor_wmf_txq length=%d wmf_pkt_cnt=%d wmf_ret=%d ret=%d\n",
		__FUNCTION__, __LINE__,
		dor_txq_len, cm->cm_dor_wmf_pkt_cnt, wmf_ret, ret));

	if (dor_txq_len) {
		struct sk_buff *skb_tmp;
		int txq_len = skb_queue_len(txq);

		/* Some skbs are in cm_dor_wmf_txq. Move them into txq for transmission */
		while ((skb_tmp = skb_dequeue(cm->cm_dor_wmf_txq)) != NULL) {
			/* update DoR tx queue */
#ifdef DOR_USE_FLOWRING_SKB
			eh = (struct ether_header *) skb_tmp->data;
			if (dhd_get_flowid(dhdp, ifidx, prio,
				eh->ether_shost, eh->ether_dhost, &flowid) != BCME_OK) {
				DQNET_INVALID_FLOWRING(skb_tmp);
				DHD_INFO(("get flowid err\n"));
			} else {
				DQNET_SET_FLOWRING(skb_tmp, prio, flowid);
				DHD_INFO(("forward STA flowid %d %d %x lookup good\n",
					prio, flowid, skb_tmp->priority));
			}
#endif /* DOR_USE_FLOWRING_SKB */
			skb_queue_tail(txq, skb_tmp);
		}

		if (cm->cm_dor_wmf_pkt_cnt != skb_queue_len(txq)) {
			/* not an error condition as other callings exist */
			/* which can fill up txq */
			DHD_INFO(("%s,%d DoR WMF txq mismatch condition %d %d %d %d\n",
				__FUNCTION__, __LINE__,
				txq_len, dor_txq_len,
				cm->cm_dor_wmf_pkt_cnt, skb_queue_len(txq)));
		}

		dhd_dor_wmf_txq_reset(dhdp);
		ret = BCM_NETHOOK_PASS;	/* default, ask caller dqnet to send pkts for CM DoR */
	}
	DHD_CM_CTX_UNLOCK(cm, flags);

	if (wmf_ret == WMF_TAKEN) {
		/* Do not queue this skb, owner will take care of it. */
		return ret;
	}

#ifdef DOR_USE_FLOWRING_SKB
	eh = (struct ether_header *) skb->data;

	if (dhd_get_flowid(dhdp, ifidx, prio, eh->ether_shost, eh->ether_dhost,
		&flowid) != BCME_OK) {
		ret = BCM_NETHOOK_DROP;
		DQNET_INVALID_FLOWRING(skb);
		DHD_INFO(("%s,%d: forward drop STA due to flowid lookup error\n",
			__FUNCTION__, __LINE__));
	} else {
		DQNET_SET_FLOWRING(skb, prio, flowid);
	}
#endif /* DOR_USE_FLOWRING_SKB */

	/* Append this skb to the end of txq */
	skb_queue_tail(txq, skb);

	DHD_INFO(("%s,%d: txq qlen=%d flowring=0x%x ret=%d\n",
		__FUNCTION__, __LINE__, skb_queue_len(txq), DQNET_GET_FLOWRING(skb), ret));

	return (ret);
}

int
dhd_rx_skb_nethook(void *ctx, int ifidx, void *p)
{
	dhd_cm_ctx_t *cm = (dhd_cm_ctx_t *) ctx;
	dhd_pub_t *dhdp = (dhd_pub_t *) cm->pub;
	dhd_sta_t *sta = NULL;
	dhd_sta_t *dst_sta;
	int ret = BCME_OK;
	struct sk_buff *skb = NULL;
	/*	struct ether_header *eh = (struct ether_header *)PKTDATA(dhdp->osh, p); */
	struct ether_header *eh;
	int result = BCM_NETHOOK_PASS;
	uint8 *iph;
#ifdef CMWIFI_WMF_IPV6
	bool is_igmp_mld = 0;
#endif /* CMWIFI_WMF_IPV6 */

	ASSERT(dhdp != NULL);
	ASSERT(p != NULL);

	skb = PKTTONATIVE(dhdp->osh, p);
	eh = (struct ether_header *) skb_mac_header(skb);

	DHD_INFO(("%s,%d dhdp->unit=%d ifidx=%d src=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x"
		" dst=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x ether_type=0x%x\n",
		__FUNCTION__, __LINE__,
		dhdp->unit, ifidx,
		eh->ether_shost[0], eh->ether_shost[1], eh->ether_shost[2],
		eh->ether_shost[3], eh->ether_shost[4], eh->ether_shost[5],
		eh->ether_dhost[0], eh->ether_dhost[1], eh->ether_dhost[2],
		eh->ether_dhost[3], eh->ether_dhost[4], eh->ether_dhost[5],
		ntoh16(eh->ether_type)));

	if (DHD_IF_ROLE_AP(dhdp, ifidx) && !DHD_IF_BSS_UP(dhdp, ifidx)) {
		DHD_INFO(("%s: bss ifidx %d not up yet\n", __FUNCTION__, ifidx));
		return BCM_NETHOOK_DROP;
	}

	/*  Not allowed when intf role is not AP */
	if (!DHD_IF_ROLE_AP(dhdp, ifidx)) {
		return BCM_NETHOOK_DROP;
	}

	result = DHD_IFP_AP_ISOLATE(dhdp, ifidx) ? BCM_NETHOOK_DROP : BCM_NETHOOK_PASS;

	/* multicast and broadcast IGMP snooping handling DoR in RX direction */
	if (ETHER_ISMULTI(eh->ether_dhost)) {
		if (!DHD_IF_WMF_ENABLE(dhdp, ifidx)) {
			/* Always allowed for multicast and broadcast */
			return result;
		}

		sta = dhd_find_sta(dhdp, ifidx, (void *)eh->ether_shost);
		if (sta == NULL) {
			return result;
		}

		iph = (uint8 *)eh + ETHER_HDR_LEN;

#ifdef CMWIFI_WMF_IPV6
		if ((IP_VER(iph)) == IP_VER_6) {
			struct ipv6_hdr *hdr = (struct ipv6_hdr *)iph;
			u8 *nextHdr = (u8 *)((u8*)hdr + sizeof(struct ipv6_hdr));
			if ((hdr->nexthdr == 0) && (*nextHdr) == IPPROTO_ICMPV6)
				is_igmp_mld = 1;
		}

		if ((IP_VER(iph) == IP_VER_4) && (IPV4_PROT(iph) == IP_PROT_IGMP))
			is_igmp_mld = 1;

		if (!is_igmp_mld)
			return result;
#else
		if ((IP_VER(iph) != IP_VER_4) || (IPV4_PROT(iph) != IP_PROT_IGMP))
			return result;
#endif /* CMWIFI_WMF_IPV6 */

		/* IGMP snooping, need to adjust skb offset */
		skb_push(skb, ETH_HLEN);

		ret = dhd_wmf_packets_handle(dhdp, p, sta, ifidx, 1);
		switch (ret) {
			case WMF_TAKEN:
				/* WMF can't take packets here as rxhook function can't handle it */
				DHD_INFO(("%s: WMF should never take packets\n", __FUNCTION__));
				return BCM_NETHOOK_CONSUMED;
			case WMF_DROP:
				/* Packet DROP decision by WMF. Toss it */
				DHD_INFO(("%s: WMF decides to drop packet\n", __FUNCTION__));
				break;
			default:
				break;
		}
		/* old logic to for DoR multicast and broadcast, casting */
		skb_pull(skb, ETH_HLEN);	/* DoR readjust skb offset */

		/* Always allowed for multicast and broadcast */
		return result;
	} else if (DHD_IFP_AP_ISOLATE(dhdp, ifidx)) {
		/* Not allowed when AP isolation is on */
		return BCM_NETHOOK_DROP;
	}

	dst_sta = dhd_find_sta(dhdp, ifidx, (void *)eh->ether_dhost);
	DHD_INFO(("%s,%d DoR Rx sta=0x%p dst_sta=0x%p\n",
		__FUNCTION__, __LINE__, sta, dst_sta));

	return (dst_sta != NULL ? BCM_NETHOOK_PASS : BCM_NETHOOK_DROP);
}

int BCMFASTPATH
dhd_offload_sendpkt(dhd_pub_t *dhdp, int ifidx, void *pktbuf)
{
	int ret = BCME_OK;
	dhd_cm_ctx_t *cm_ctx = DHD_CM_CTX(dhdp);
	struct ether_header *eh = NULL;
	uint8 *pktdata = (uint8 *)PKTDATA(dhdp->osh, pktbuf);
	struct sk_buff *skb = (struct sk_buff *)pktbuf;
	unsigned long flags;

	eh = (struct ether_header *)pktdata;

	DHD_INFO(("wl%d offload forwarding"
		" src=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x dst=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
		dhdp->unit,
		eh->ether_shost[0], eh->ether_shost[1], eh->ether_shost[2],
		eh->ether_shost[3], eh->ether_shost[4], eh->ether_shost[5],
		eh->ether_dhost[0], eh->ether_dhost[1], eh->ether_dhost[2],
		eh->ether_dhost[3], eh->ether_dhost[4], eh->ether_dhost[5]));

	/* DHD DoR enable, just return a list to converted WMF skb list to caller
		cm_runner_DoR_tx(ctx, pktdata);
	*/
	DHD_CM_CTX_LOCK(cm_ctx, flags);
	skb_queue_tail(cm_ctx->cm_dor_wmf_txq, skb);
	cm_ctx->cm_dor_wmf_pkt_cnt++;
	cm_ctx->cm_dor_state |= 0x1;
	DHD_CM_CTX_UNLOCK(cm_ctx, flags);

	return ret;
}

int
dhd_flowmgr_flushwifi(dhd_pub_t *dhdp,  flow_ring_node_t *flow_ring_node)
{
	int iret = 0;
	struct file *proc_file;
	int flags = O_RDWR;
	char buf[64];
	const char *cmd_path = "/proc/driver/flowmgr/cmd";
	loff_t pos = 0;
	int ret = BCME_OK;

	proc_file = filp_open(cmd_path, flags, 0);

	if (!IS_ERR(proc_file)) {
		sprintf(buf, "flow_flushwifi %.2x:%.2x:%.2x:%.2x:%.2x:%.2x %d 2",
			flow_ring_node->flow_info.da[0], flow_ring_node->flow_info.da[1],
			flow_ring_node->flow_info.da[2], flow_ring_node->flow_info.da[3],
			flow_ring_node->flow_info.da[4], flow_ring_node->flow_info.da[5],
			flow_ring_node->flowid);

		iret = kernel_write(proc_file, buf, strlen(buf), &pos);
		if (iret != strlen(buf)) {
			DHD_ERROR(("%s: wl%d failed to write: %s\n", __FUNCTION__,
				dhdp->unit, buf));
			ret = BCME_ERROR;
		}
		filp_close(proc_file, NULL);
	} else {
		DHD_ERROR(("%s: wl%d failed open flowmgr proc file:%s\n", __FUNCTION__,
				dhdp->unit, cmd_path));
		ret = BCME_ERROR;
	}

	return ret;
}

#endif /* CMWIFI && BCM_DHD_RUNNER */
