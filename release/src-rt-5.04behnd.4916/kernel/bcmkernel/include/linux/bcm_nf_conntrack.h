/*
 * <:copyright-BRCM:2012:DUAL/GPL:standard
 *
 *    Copyright (c) 2012 Broadcom
 *    All Rights Reserved
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 */
#ifndef _BCM_NF_CONNTRACK_H
#define _BCM_NF_CONNTRACK_H

#include <linux/types.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>

#include <linux/blog.h>
#include <linux/dpi.h>
#include <linux/iqos.h>
#include <linux/kqoe.h>

void bcm_conntrack_init_end(void);
void bcm_conntrack_cleanup_end(void);
int bcm_conntrack_event(int eventmask, struct nf_conn *ct, u32 portid,
			int report);

typedef int __rcu (*bcm_ct_event_fn)(enum ip_conntrack_events event,
				     struct nf_conn *ct, struct sk_buff *skb);

extern bcm_ct_event_fn bcm_hwf_ct_event_fn;
extern bcm_ct_event_fn bcm_bnltrace_ct_event_fn;
extern bcm_ct_event_fn bcm_scoretrace_ct_event_fn;

static bcm_ct_event_fn *bcm_ct_event_fns[] = {
#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
	&bcm_hwf_ct_event_fn,
#endif
#if IS_ENABLED(CONFIG_BCM_BNL)
	&bcm_bnltrace_ct_event_fn,
#endif
#if IS_ENABLED(CONFIG_BCM_SCORE)
	&bcm_scoretrace_ct_event_fn,
#endif
};

static inline int ipv6_is_multicast(const __be32 ip6[4])
{
	return ((ip6[0] & htonl(0xFF000000)) == htonl(0xFF000000));
}

#if IS_ENABLED(CONFIG_BLOG)
static inline void bcm_nf_blog_ct_init(struct nf_conn *ct)
{
	/* no blog lock needed here */
	set_bit(IPS_BLOG_BIT, &ct->status);  /* Enable conntrack blogging */
	ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL] = BLOG_KEY_FC_INVALID;
	ct->bcm_ext.blog_key[IP_CT_DIR_REPLY]    = BLOG_KEY_FC_INVALID;
	ct->bcm_ext.blog_learned = 0;
}

static inline int bcm_nf_blog_destroy_conntrack(struct nf_conn *ct)
{
	blog_lock();

	pr_debug("%s(%px) blog keys[0x%08x,0x%08x]\n", __func__, ct,
		 ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL],
		 ct->bcm_ext.blog_key[IP_CT_DIR_REPLY]);

	/* Conntrack going away, notify blog client */
	if (ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL] != BLOG_KEY_FC_INVALID ||
	    ct->bcm_ext.blog_key[IP_CT_DIR_REPLY] != BLOG_KEY_FC_INVALID) {
		blog_notify(DESTROY_FLOWTRACK, ct,
			    ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL],
			    ct->bcm_ext.blog_key[IP_CT_DIR_REPLY]);
	}

	/* Disable further blogging */
	clear_bit(IPS_BLOG_BIT, &ct->status);

	blog_unlock();

	return 0;
}

static inline int
bcm_nf_blog_link_ct(struct nf_conn *ct, enum ip_conntrack_info ctinfo,
		    struct sk_buff *skb, u_int16_t l3num, u_int8_t protonum)
{
	/* here we dont need blog lock as this blog is owned by this skb */
	struct nf_conn_help *help = nfct_help(ct);

	if (help && help->helper && strcmp(help->helper->name, "BCM-NAT")) {
		pr_debug("nf_conntrack_in: skb<%px> ct<%px> helper<%s> found\n",
			 skb, ct, help->helper->name);
		clear_bit(IPS_BLOG_BIT, &ct->status);
	}

	if (test_bit(IPS_BLOG_BIT, &ct->status)) {
		/* OK to blog */
		pr_debug("nf_conntrack_in: skb<%px> blog<%px> ct<%px>\n",
			 skb, blog_ptr(skb), ct);

		blog_link(FLOWTRACK, blog_ptr(skb), ct, CTINFO2DIR(ctinfo), 0);

#if IS_ENABLED(CONFIG_BCM_BNL)
		if (ct->bcm_ext.bnl_idx) {
			blog_link(BNLTRACE, blog_ptr(skb), ct,
				  CTINFO2DIR(ctinfo), ct->bcm_ext.bnl_idx);
		}
#endif
	} else {
		pr_debug("nf_conntrack_in: skb<%px> ct<%px> NOT BLOGible<%px>\n",
			 skb, ct, blog_ptr(skb));
		blog_skip(skb, blog_skip_reason_ct_status_donot_blog);
	}

	return 0;
}

static inline int
bcm_nf_blog_update_timeout(struct nf_conn *ct, unsigned long extra_jiffies)
{
	unsigned int blog_key = BLOG_KEY_FC_INVALID;

	blog_lock();

	if (ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL] != BLOG_KEY_FC_INVALID)
		blog_key = ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL];
	else if (ct->bcm_ext.blog_key[IP_CT_DIR_REPLY] == BLOG_KEY_FC_INVALID)
		blog_key = ct->bcm_ext.blog_key[IP_CT_DIR_REPLY];

	if (blog_key != BLOG_KEY_FC_INVALID) {
		blog_notify(UPDATE_FLOWTRACK_IDLE_TIMEOUT, ct, blog_key,
			    (uint32_t)(extra_jiffies / HZ));
	}

	blog_unlock();

	return 0;
}
#endif /* CONFIG_BLOG */

static inline void bcm_nf_scs_destroy_conntrack(struct nf_conn *ct)
{
#if IS_ENABLED(CONFIG_BCM_NF_SCS)
	spinlock_t *lock;

	local_bh_disable();

	lock = ct->bcm_ext.scs.lock;
	if (lock) {
		spin_lock_bh(lock);
		list_del(&ct->bcm_ext.scs.entry);
		ct->bcm_ext.scs.lock = NULL;
		spin_unlock_bh(lock);
	}

	local_bh_enable();
#endif /* CONFIG_BCM_NF_SCS */
}

static inline void bcm_nf_mscs_destroy_conntrack(struct nf_conn *ct)
{
#if IS_ENABLED(CONFIG_NF_MSCS)
	spinlock_t *lock;

	local_bh_disable();

	lock = ct->bcm_ext.mscs.lock;
	if (lock) {
		spin_lock_bh(lock);
		list_del(&ct->bcm_ext.mscs.entry);
		ct->bcm_ext.mscs.lock = NULL;
		spin_unlock_bh(lock);
	}

	local_bh_enable();
#endif /* CONFIG_NF_MSCS */
}

static inline void bcm_nf_score_destroy_conntrack(struct nf_conn *ct)
{
#if IS_ENABLED(CONFIG_BCM_NFT_SCORE)
	if (ct->bcm_ext.score)
	{
		kfree(ct->bcm_ext.score);
	}
#endif
}

static inline void bcm_nf_ct_set_iq_prio(struct nf_conn *ct, struct sk_buff *skb)
{
#if IS_ENABLED(CONFIG_BCM_INGRESS_QOS)
#if IS_ENABLED(CONFIG_BLOG)
	if (skb && skb->blog_p) {
		ct->bcm_ext.iq_prio = (blog_iq(skb) == BLOG_IQ_PRIO_HIGH)
			? IQOS_PRIO_HIGH : IQOS_PRIO_LOW;
		return;
	}
#endif

	switch (nf_ct_l3num(ct)) {
	case AF_INET:
		ct->bcm_ext.iq_prio =
			ipv4_is_multicast(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip)
				? IQOS_PRIO_HIGH : IQOS_PRIO_LOW;
		break;
	case AF_INET6:
		ct->bcm_ext.iq_prio =
			ipv6_is_multicast(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip6)
				? IQOS_PRIO_HIGH : IQOS_PRIO_LOW;
		break;
	default:
		ct->bcm_ext.iq_prio = IQOS_PRIO_LOW;
		break;
	}
#endif
}

static inline void bcm_nf_iqos_destroy_conntrack(struct nf_conn *ct)
{
#if IS_ENABLED(CONFIG_BCM_INGRESS_QOS)
	if (!test_bit(IPS_IQOS_BIT, &ct->status))
		return;

	clear_bit(IPS_IQOS_BIT, &ct->status);
	iqos_rem_L4port(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum,
			ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all),
			IQOS_ENT_DYN);
	iqos_rem_L4port(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.protonum,
			ntohs(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all),
			IQOS_ENT_DYN);
#endif
}

static inline void
bcm_ct_event(int __rcu (*fn)(enum ip_conntrack_events event,
			     struct nf_conn *ct, struct sk_buff *skb),
	     enum ip_conntrack_events eventmask, struct nf_conn *ct,
	     struct sk_buff *skb)
{
	rcu_read_lock();
	fn = rcu_dereference(fn);
	if (fn)
		fn(eventmask, ct, skb);
	rcu_read_unlock();
}

#if defined(CONFIG_BCM_NF_DERIVED_CONN)
static inline void bcm_nf_ct_derived_list_add(struct nf_conn *ct)
{
	BCM_DERIVED_CONN_LOCK_BH();

	/* master ref count incremented before calling this function
	 * via exp->master safe, refcnt bumped in nf_ct_find_expectation
	 */
	if (ct->master) {
		list_add(&ct->bcm_ext.derived_list,
			 &ct->master->bcm_ext.derived_connections);
	}

	BCM_DERIVED_CONN_UNLOCK_BH();
}

static inline void bcm_nf_ct_derived_list_del(struct nf_conn *ct)
{
	BCM_DERIVED_CONN_LOCK_BH();

	if (ct->master)
		list_del(&ct->bcm_ext.derived_list);

	BCM_DERIVED_CONN_UNLOCK_BH();
}

static inline void bcm_nf_ct_derived_conn_init(struct nf_conn *ct)
{
	INIT_LIST_HEAD(&ct->bcm_ext.derived_connections);
	INIT_LIST_HEAD(&ct->bcm_ext.derived_list);
	ct->bcm_ext.derived_timeout = 0;
}
#else
static inline void bcm_nf_ct_derived_list_add(struct nf_conn *ct) {}
static inline void bcm_nf_ct_derived_list_del(struct nf_conn *ct) {}
static inline void bcm_nf_ct_derived_conn_init(struct nf_conn *ct) {}
#endif

/* --- hooks called by netfilter --- */
static inline void bcm_nf_ct_alloc(struct nf_conn *ct, struct sk_buff *skb)
{
	memset(&ct->bcm_ext, 0, sizeof(ct->bcm_ext));

#if defined(CONFIG_BLOG)
	bcm_nf_blog_ct_init(ct);
#endif
	/* REGARDLESS_DROP */
	INIT_LIST_HEAD(&ct->bcm_ext.safe_list);

	bcm_nf_ct_derived_conn_init(ct);
	bcm_nf_ct_set_iq_prio(ct, skb);

#if IS_ENABLED(CONFIG_BCM_DPI)
	ct->bcm_ext.dpi.eg_prio = EG_PRIO_NORMAL;
	if (skb && skb->dev && is_netdev_wan(skb->dev))
		set_bit(DPI_CT_INIT_FROM_WAN_BIT, &ct->bcm_ext.dpi.flags);
#endif

#if IS_ENABLED(CONFIG_BCM_KQOE)
	ct->bcm_ext.qos.orig_queue = KQ_BYPASS_QUEUE;
	ct->bcm_ext.qos.reply_queue = KQ_BYPASS_QUEUE;
	ct->bcm_ext.qos.eg_prio = EG_PRIO_NORMAL;
#endif
}

static inline void bcm_nf_ct_init(struct nf_conn *ct)
{
	bcm_nf_ct_derived_list_add(ct);
}

static inline void bcm_nf_ct_confirm(struct nf_conn *ct, struct sk_buff *skb)
{
	int i;

#if IS_ENABLED(CONFIG_BCM_KQOE)
	if (skb->blog_p && skb->blog_p->rx.info.phyHdrType == BLOG_WLANPHY)
		ct->bcm_ext.qos.in_tid = skb->wl.ucast.nic.wl_prio;
#endif

	for (i = 0; i < ARRAY_SIZE(bcm_ct_event_fns); i++)
		bcm_ct_event(*bcm_ct_event_fns[i], 1 << IPCT_NEW, ct, skb);
}

static inline void bcm_nf_ct_seenreply(struct nf_conn *ct, struct sk_buff *skb)
{
#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
	bcm_ct_event(bcm_hwf_ct_event_fn, 1 << IPCT_REPLY, ct, skb);
#endif

#if IS_ENABLED(CONFIG_BCM_SCORE)
	bcm_ct_event(bcm_scoretrace_ct_event_fn, 1 << IPCT_REPLY, ct, skb);
#endif

#if IS_ENABLED(CONFIG_BCM_KQOE)
	if (skb->blog_p && skb->blog_p->rx.info.phyHdrType == BLOG_WLANPHY)
		ct->bcm_ext.qos.in_tid = skb->wl.ucast.nic.wl_prio;
#endif
}

static inline unsigned long
bcm_nf_ct_refresh(struct nf_conn *ct, unsigned long extra_jiffies)
{
#if defined(CONFIG_BCM_NF_DERIVED_CONN)
	/* when derived time out is set always use it */
	if (ct->bcm_ext.derived_timeout)
		extra_jiffies = ct->bcm_ext.derived_timeout;
#endif

#if IS_ENABLED(CONFIG_BLOG)
	if (ct->bcm_ext.extra_jiffies != extra_jiffies) {
		ct->bcm_ext.extra_jiffies = extra_jiffies;
		/*notify accelerator */
		bcm_nf_blog_update_timeout(ct, extra_jiffies);
	}
#endif

	return extra_jiffies;
}

static inline void bcm_nf_ct_delete_from_lists(struct nf_conn *ct)
{
#if IS_ENABLED(CONFIG_BCM_DPI)
	dpi_nf_ct_delete_from_lists(ct);
#endif
#if IS_ENABLED(CONFIG_BCM_SGS)
	sgs_nf_ct_delete_from_lists(ct);
#endif
}

static inline void bcm_nf_ct_destroy(struct nf_conn *ct)
{
	int i;

#if IS_ENABLED(CONFIG_BLOG)
	bcm_nf_blog_destroy_conntrack(ct);
#endif

	for (i = 0; i < ARRAY_SIZE(bcm_ct_event_fns); i++)
		bcm_ct_event(*bcm_ct_event_fns[i], 1 << IPCT_DESTROY, ct, NULL);

	bcm_nf_scs_destroy_conntrack(ct);
	bcm_nf_mscs_destroy_conntrack(ct);
	bcm_nf_iqos_destroy_conntrack(ct);
	bcm_nf_ct_derived_list_del(ct);
	bcm_nf_score_destroy_conntrack(ct);
}

#endif /* _BCM_NF_CONNTRACK_H */
