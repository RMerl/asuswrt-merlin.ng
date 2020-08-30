/*
 * <:copyright-BRCM:2017:DUAL/GPL:standard 
 * 
 *    Copyright (c) 2017 Broadcom 
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

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/percpu.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netlink.h>
#include <linux/udp.h>
#include <net/dst.h>
#include <net/neighbour.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <net/netfilter/nf_conntrack_ecache.h>

#include <linux/dpi.h>
#include <linux/blog.h>
#include <bcmdpi.h>

#include <tdts.h>

#include "dpi_local.h"

#define DPI_DEFAULT_MAX_PKT	15

/* ----- local functions ----- */
static unsigned int dpi_nf_hook(const struct nf_hook_ops *ops,
				struct sk_buff *skb,
				const struct nf_hook_state *state);
static unsigned int dpi_nf_block(const struct nf_hook_ops *ops,
				 struct sk_buff *skb,
				 const struct nf_hook_state *state);

/* ----- global variables ----- */
struct dpi_stats dpi_stats;

struct proc_dir_entry *dpi_dir;
EXPORT_SYMBOL(dpi_dir);

int dpi_enabled = 1;
EXPORT_SYMBOL(dpi_enabled);

/* ----- local variables ----- */
#define DECLARE_HOOK(_fun, _pf, _hooknum, _priority) \
	{ \
		.hook		= _fun, \
		.owner		= THIS_MODULE, \
		.pf		= _pf, \
		.hooknum	= _hooknum, \
		.priority	= _priority, \
	}
static struct nf_hook_ops hooks[] __read_mostly = {
	/* Forward */
	DECLARE_HOOK(dpi_nf_hook, NFPROTO_IPV4, NF_INET_FORWARD, NF_IP_PRI_FILTER),
	DECLARE_HOOK(dpi_nf_hook, NFPROTO_BRIDGE, NF_INET_FORWARD, NF_IP_PRI_FILTER),
#if defined(CONFIG_IPV6)
	DECLARE_HOOK(dpi_nf_hook, NFPROTO_IPV6, NF_INET_FORWARD, NF_IP6_PRI_FILTER),
#endif
	/* Local in */
	DECLARE_HOOK(dpi_nf_hook, NFPROTO_IPV4, NF_INET_LOCAL_IN, NF_IP_PRI_FILTER),
	DECLARE_HOOK(dpi_nf_hook, NFPROTO_BRIDGE, NF_INET_LOCAL_IN, NF_IP_PRI_FILTER),
#if defined(CONFIG_IPV6)
	DECLARE_HOOK(dpi_nf_hook, NFPROTO_IPV6, NF_INET_LOCAL_IN, NF_IP6_PRI_FILTER),
#endif
	/* Local out */
	DECLARE_HOOK(dpi_nf_hook, NFPROTO_IPV4, NF_INET_LOCAL_OUT, NF_IP_PRI_FILTER),
	DECLARE_HOOK(dpi_nf_hook, NFPROTO_BRIDGE, NF_INET_LOCAL_OUT, NF_IP_PRI_FILTER),
#if defined(CONFIG_IPV6)
	DECLARE_HOOK(dpi_nf_hook, NFPROTO_IPV6, NF_INET_LOCAL_OUT, NF_IP6_PRI_FILTER),
#endif
	/* Pre-routing (blocking) */
	DECLARE_HOOK(dpi_nf_block, NFPROTO_IPV4, NF_INET_PRE_ROUTING, NF_IP_PRI_MANGLE),
	DECLARE_HOOK(dpi_nf_block, NFPROTO_BRIDGE, NF_INET_PRE_ROUTING, NF_IP_PRI_MANGLE),
#if defined(CONFIG_IPV6)
	DECLARE_HOOK(dpi_nf_block, NFPROTO_IPV6, NF_INET_PRE_ROUTING, NF_IP6_PRI_MANGLE),
#endif
};

static struct sock *nl_sk;
static struct dpi_hooks dpi_hooks;
static int dpi_max_pkt = DPI_DEFAULT_MAX_PKT;
static DEFINE_PER_CPU(tdts_pkt_parameter_t, pkt_params);


/* ----- local functions ----- */
static uint64_t pkt_count(struct nf_conn *ct)
{
	struct nf_conn_acct *acct;
	struct nf_conn_counter *counter;

	if (!ct)
		return 0;

	acct = nf_conn_acct_find(ct);
	if (!acct)
		return 0;

	counter = acct->counter;
	return atomic64_read(&counter[0].packets) +
		atomic64_read(&counter[1].packets);
}

static int dir_downstream(struct nf_conn *ct, int dir)
{
	unsigned long flags = ct ? ct->dpi.flags : 0;

	if (!flags && ct && ct->master)
		flags = ct->master->dpi.flags;

	/*
	 * If the connection was initiated from the LAN side, then the reply
	 * direction is downstream. If the connection was initiated from the
	 * WAN side, then the originating direction is downstream.
	 */
	if (!test_bit(DPI_CT_INIT_FROM_WAN_BIT, &flags))
		return (dir == IP_CT_DIR_REPLY);
	else
		return (dir == IP_CT_DIR_ORIGINAL);
}

static int calculate_lookup_flags(struct sk_buff *skb, struct nf_conn *ct,
				  unsigned long flags, int lookup_flags)
{
	struct ethhdr *h = eth_hdr(skb);

	/* don't classify packets which have no source MAC */
	if (!skb_mac_header_was_set(skb) || is_zero_ether_addr(h->h_source))
		return 0;

	/* don't classify WAN-side devices */
	if (is_wan_dev(skb))
		lookup_flags &= ~SW_DEVID;

	if (!ct)
		goto out;

	/* filter out classification based on current classification status */
	if (test_bit(DPI_APPID_STOP_CLASSIFY_BIT, &flags) &&
	    !blog_request(FLOWTRACK_ALG_HELPER, ct, 0, 0))
		lookup_flags &= ~SW_APP;
	if (test_bit(DPI_DEVID_STOP_CLASSIFY_BIT, &flags))
		lookup_flags &= ~SW_DEVID;
#ifdef DPI_URL_RECORD
	if (test_bit(DPI_URL_STOP_CLASSIFY_BIT, &flags))
#endif
		lookup_flags &= ~SW_URL_QUERY;

out:
	return lookup_flags;
}

static void update_classification_stats(struct sk_buff *skb,
					unsigned long old_flags,
					unsigned long flags,
					int classified)
{
	/* if we have newly changed a classification status, save results */
	flags ^= old_flags;

	if (classified) {
		if (test_bit(DPI_APPID_STOP_CLASSIFY_BIT, &flags))
			dpi_stats.apps.classified++;
		if (test_bit(DPI_DEVID_STOP_CLASSIFY_BIT, &flags))
			dpi_stats.devs.classified++;
#ifdef DPI_URL_RECORD
		if (test_bit(DPI_URL_STOP_CLASSIFY_BIT, &flags))
			dpi_stats.urls.classified++;
#endif
	} else {
		if (!test_bit(DPI_APPID_STOP_CLASSIFY_BIT, &flags))
			dpi_stats.apps.unclassified++;
		if (!test_bit(DPI_DEVID_STOP_CLASSIFY_BIT, &flags))
			dpi_stats.devs.unclassified++;
#ifdef DPI_URL_RECORD
		if (!test_bit(DPI_URL_STOP_CLASSIFY_BIT, &flags))
			dpi_stats.urls.unclassified++;
#endif
	}
}

static struct dpi_dev *dpi_classify_device(struct dpi_dev *dev,
					   struct sk_buff *skb,
					   tdts_pkt_parameter_t *pkt_param,
					   unsigned long *flags,
					   int lookup_flags)
{
	const struct dst_entry *dst = skb_dst(skb);
	struct neighbour *n = NULL;

	/* find or allocate device info */
	if (!dev) {
		/*
		 * For LAN-initiated flows, use the source MAC.
		 * For WAN-initiated flows, we have to try a neighbour lookup
		 * from the destination IP to find a appropriate MAC.
		 */
		if (!is_wan_dev(skb)) {
			dev = dpi_dev_find_or_alloc(eth_hdr(skb)->h_source);
		} else {
			rcu_read_lock_bh();
			if (dst)
				n = dst_neigh_lookup_skb(dst, skb);
			if (n && (n->nud_state & NUD_VALID))
				dev = dpi_dev_find_or_alloc(n->ha);
			rcu_read_unlock_bh();
			if (n)
				neigh_release(n);
		}
		if (!dev)
			return NULL;
	}

	if (!(lookup_flags & SW_DEVID))
		return dev;

	dpi_stats.devs.lookups++;

	/* if device is already classified, we're done */
	if (dev->classified)
		goto done;

	if (!tdts_check_pkt_parameter_res(pkt_param, TDTS_RES_TYPE_DEVID)) {
		dpi_stats.devs.misses++;
		return dev;
	}

	/* save device classification information */
	dev->vendor_id	= TDTS_PKT_PARAMETER_RES_DEVID_VENDOR_ID(pkt_param);
	dev->os_id	= TDTS_PKT_PARAMETER_RES_DEVID_OS_NAME_ID(pkt_param);
	dev->class_id	= TDTS_PKT_PARAMETER_RES_DEVID_OS_CLASS_ID(pkt_param);
	dev->type_id	= TDTS_PKT_PARAMETER_RES_DEVID_DEV_CAT_ID(pkt_param);
	dev->dev_id	= TDTS_PKT_PARAMETER_RES_DEVID_DEV_ID(pkt_param);
	dev->classified	= 1;

done:
	set_bit(DPI_DEVID_STOP_CLASSIFY_BIT, flags);
	dpi_stats.devs.hits++;

	pr_debug("devid<%08x> type<%d> class<%d> vendor<%d> os<%d>\n",
		 dev->dev_id, dev->type_id, dev->class_id, dev->vendor_id,
		 dev->os_id);
	return dev;
}

static struct dpi_app *dpi_classify_app(struct dpi_app *app,
					struct sk_buff *skb,
					tdts_pkt_parameter_t *pkt_param,
					unsigned long *flags,
					int lookup_flags)
{
	uint32_t app_id;

	if (!(lookup_flags & SW_APP))
		return app;

	dpi_stats.apps.lookups++;

	/* update flags */
	if (TDTS_PKT_PARAMETER_RES_APPID_CHECK_NOMORE(pkt_param) ||
	    TDTS_PKT_PARAMETER_RES_APPID_CHECK_NOINT(pkt_param)) {
		set_bit(DPI_APPID_STOP_CLASSIFY_BIT, flags);
		clear_bit(DPI_APPID_ONGOING_BIT, flags);
	}

	if (!tdts_check_pkt_parameter_res(pkt_param, TDTS_RES_TYPE_APPID)) {
		dpi_stats.apps.misses++;
		return app;
	}
	app_id = dpi_get_app_id(TDTS_PKT_PARAMETER_RES_APPID_CAT_ID(pkt_param),
				TDTS_PKT_PARAMETER_RES_APPID_APP_ID(pkt_param),
				0);
	if (!app_id) {
		dpi_stats.apps.misses++;
		return app;
	}

	/* if this is a reclassification, set the current app and appinst to
	 * NULL to search again for new entries */
	if (app && app->app_id != app_id) {
		pr_debug("reclassify from <%08x> to <%08x>\n", app->app_id,
			 app_id);
		app = NULL;
	}

	/* find or allocate application info */
	if (!app) {
		app = dpi_app_find_or_alloc(app_id);
		if (!app)
			return NULL;
	}

	set_bit(DPI_APPID_IDENTIFIED_BIT, flags);
	if (TDTS_PKT_PARAMETER_RES_APPID_CHECK_FINAL(pkt_param)) {
		set_bit(DPI_APPID_FINAL_BIT, flags);
		set_bit(DPI_APPID_STOP_CLASSIFY_BIT, flags);
		clear_bit(DPI_APPID_ONGOING_BIT, flags);
	} else {
		set_bit(DPI_APPID_ONGOING_BIT, flags);
	}
	dpi_stats.apps.hits++;

	pr_debug("appid<%08x> [app<%u>(%s) cat<%u>(%s) beh<%u>(%s)] flags<%lx> [final<%d> nomore<%d> noint<%d>]\n",
		 app->app_id,
		 TDTS_PKT_PARAMETER_RES_APPID_APP_ID(pkt_param),
		 TDTS_PKT_PARAMETER_RES_APPID_APP_NAME(pkt_param),
		 TDTS_PKT_PARAMETER_RES_APPID_CAT_ID(pkt_param),
		 TDTS_PKT_PARAMETER_RES_APPID_CAT_NAME(pkt_param),
		 TDTS_PKT_PARAMETER_RES_APPID_BEH_ID(pkt_param),
		 TDTS_PKT_PARAMETER_RES_APPID_BEH_NAME(pkt_param),
		 *flags,
		 TDTS_PKT_PARAMETER_RES_APPID_CHECK_FINAL(pkt_param) ? 1 : 0,
		 TDTS_PKT_PARAMETER_RES_APPID_CHECK_NOMORE(pkt_param) ? 1 : 0,
		 TDTS_PKT_PARAMETER_RES_APPID_CHECK_NOINT(pkt_param) ? 1 : 0);
	return app;
}

static struct dpi_url *dpi_classify_url(struct dpi_url *url,
					struct sk_buff *skb,
					tdts_pkt_parameter_t *pkt_param,
					unsigned long *flags,
					int lookup_flags)
{
#ifdef DPI_URL_RECORD
	char *hostname;
	int len;

	if (!(lookup_flags & SW_URL_QUERY))
		return url;

	dpi_stats.urls.lookups++;

	hostname = TDTS_PKT_PARAMETER_RES_URL_DOMAIN(pkt_param);
	len = TDTS_PKT_PARAMETER_RES_URL_DOMAIN_LEN(pkt_param);
	if (!hostname || !len) {
		dpi_stats.urls.misses++;
		return url;
	}

	/* find or allocate url info */
	if (!url) {
		url = dpi_url_find_or_alloc(hostname, len);
		if (!url)
			return NULL;
	}

	dpi_stats.urls.hits++;

	set_bit(DPI_URL_STOP_CLASSIFY_BIT, flags);
#endif /* DPI_URL_RECORD */
	return url;
}

static void send_nfct_update(struct sk_buff *skb)
{
	if (!nfct(skb))
		return;

	pr_debug("ct<%p> reporting to userspace\n", nfct(skb));
	nf_conntrack_eventmask_report(1 << IPCT_DPI,
				      nfct(skb),
				      NETLINK_CB(skb).portid,
				      0);

	if (!nfct(skb)->master)
		return;

	pr_debug("ct<%p> reporting to userspace\n", nfct(skb)->master);
	nf_conntrack_eventmask_report(1 << IPCT_DPI,
				      nfct(skb)->master,
				      NETLINK_CB(skb).portid,
				      0);
}

extern int tdts_shell_tcp_conn_remove(uint8_t ip_ver, uint8_t *sip,
		uint8_t *dip, uint16_t sport, uint16_t dport);

static void dpi_stop_classification(struct sk_buff *skb, unsigned long *flags)
{
	int ret = 0;

	/* ignore all further classifications for this flow */
	set_bit(DPI_CLASSIFICATION_STOP_BIT, flags);

	/* remove connection from Trend's internal database */
	if (skb->protocol == htons(ETH_P_IP) && ip_hdr(skb)->protocol == IPPROTO_TCP)
		ret = tdts_shell_tcp_conn_remove(ip_hdr(skb)->version,
						 (uint8_t*) &ip_hdr(skb)->saddr,
						 (uint8_t*) &ip_hdr(skb)->daddr,
						 ntohs(tcp_hdr(skb)->source),
						 ntohs(tcp_hdr(skb)->dest));
	else if (skb->protocol == htons(ETH_P_IPV6) &&
		 ipv6_hdr(skb)->nexthdr == IPPROTO_TCP)
		ret = tdts_shell_tcp_conn_remove(ipv6_hdr(skb)->version,
						 (uint8_t*) &ipv6_hdr(skb)->saddr,
						 (uint8_t*) &ipv6_hdr(skb)->daddr,
						 ntohs(tcp_hdr(skb)->source),
						 ntohs(tcp_hdr(skb)->dest));
}

static void dpi_classify(struct sk_buff *skb, int lookup_flags)
{
	tdts_pkt_parameter_t *pkt_param;
	struct nf_conn *ct = nfct(skb);
	struct nf_conn *ct_master = ct ? ct->master : NULL;
	struct dpi_appinst *appinst = NULL;
	struct dpi_dev *dev = NULL;
	struct dpi_app *app = NULL;
	struct dpi_url *url = NULL;
	unsigned long flags = 0;
	unsigned long old_flags;
	int nfct_update = 0;

	/* populate classification data based on the conntrack entry */
	if (ct) {
		appinst	= ct->dpi.appinst;
		dev	= ct->dpi.dev;
		app	= ct->dpi.app;
		url	= ct->dpi.url;
		flags	= ct->dpi.flags;

		/* if the parent conntrack has classification data and we
		 * don't, copy the classification data to the child */
		if (ct_master) {
			appinst	= appinst ? appinst : ct_master->dpi.appinst;
			dev	= dev ? dev : ct_master->dpi.dev;
			app	= app ? app : ct_master->dpi.app;
			url	= url ? url : ct_master->dpi.url;
			flags	= flags ? flags : ct_master->dpi.flags;
			nfct_update = 1;
		}
	}
	old_flags = flags;

	if (test_bit(DPI_CLASSIFICATION_STOP_BIT, &flags))
		goto out;

	lookup_flags = calculate_lookup_flags(skb, ct, flags, lookup_flags);
	if (!lookup_flags)
		goto out;

	/* if we have classified too many packets, ignore further */
	if (pkt_count(ct) > dpi_max_pkt) {
		pr_debug("ct<%p> exceeded classification window<%d>, flags<%lx>\n",
			 ct, dpi_max_pkt, flags);

		/* save stats and stop classification */
		update_classification_stats(skb, old_flags, flags, 0);

		dpi_stop_classification(skb, &flags);
		ct->dpi.flags = flags;
		nfct_update = 1;
		goto out;
	}

	/* classify packet */
	pkt_param = &get_cpu_var(pkt_params);
	dpi_stats.total_lookups++;
	tdts_init_pkt_parameter(pkt_param, lookup_flags, 0);
	pr_debug("ct<%p> flags<%lx> lookup<%x> pktcnt<%lld>\n",
		 ct, flags, lookup_flags, pkt_count(ct));
	if (tdts_shell_dpi_l3_skb(skb, pkt_param)) {
		if (pkt_param->results.pkt_decoder_verdict == -1)
			dpi_stats.engine_errors++;
		put_cpu_var(pkt_params);
		return;
	}

	/* handle classification results based on our lookup flags */
	dev = dpi_classify_device(dev, skb, pkt_param, &flags, lookup_flags);
	app = dpi_classify_app(app, skb, pkt_param, &flags, lookup_flags);
	url = dpi_classify_url(url, skb, pkt_param, &flags, lookup_flags);

	put_cpu_var(pkt_params);

	/* reset appinst if app is reclassified */
	if (appinst) {
		if (!app || app->app_id != appinst->app->app_id)
			appinst = NULL;
	}

	/* if there is no application instance but both app and device info are
	 * available, find or allocate an appinst. */
	if (!appinst && app && dev) {
		appinst = dpi_appinst_find_or_alloc(app, dev);
		nfct_update = 1;
	}

	/* save new classification data */
	if (ct) {
		ct->dpi.appinst	= appinst;
		ct->dpi.dev	= dev;
		ct->dpi.app	= app;
		ct->dpi.url	= url;
		ct->dpi.flags	= flags;
	}

	update_classification_stats(skb, old_flags, flags, 1);

	/* if we have not yet identified everything, skip accelerating */
	if (ct) {
		if (!test_bit(DPI_APPID_STOP_CLASSIFY_BIT, &flags) ||
		    !test_bit(DPI_DEVID_STOP_CLASSIFY_BIT, &flags))
			blog_skip(skb, blog_skip_reason_dpi);
	}

	/* if we have finished identifying everything, explicitly stop further
	 * classification calls and remove from Trend internal list (if TCP) */
	if (test_bit(DPI_APPID_STOP_CLASSIFY_BIT, &flags) &&
	    test_bit(DPI_DEVID_STOP_CLASSIFY_BIT, &flags) &&
	    test_bit(DPI_URL_STOP_CLASSIFY_BIT, &flags)) {
		dpi_stop_classification(skb, &flags);
		if (ct)
			ct->dpi.flags = flags;
		nfct_update = 1;
		goto out;
	}

out:
	/* update userspace */
	if (nfct_update)
		send_nfct_update(skb);
}

#define cf_l3v4(flags, skb, l4proto, srcport, dstport, new_flags) \
	cf_l3(flags, skb, ETH_P_IP, l4proto, srcport, dstport, new_flags)
#define cf_l3v6(flags, skb, l4proto, srcport, dstport, new_flags) \
	cf_l3(flags, skb, ETH_P_IPV6, l4proto, srcport, dstport, new_flags)
static inline void cf_l3(int *flags, struct sk_buff *skb, int l3proto,
			 int l4proto, int srcport, int dstport, int new_flags)
{
	int hdr_src = 0, hdr_dst = 0;

	/* validate l3 proto */
	if (skb->protocol != htons(l3proto))
		return;
	/* validate l4 proto */
	if (l3proto == ETH_P_IP && ip_hdr(skb)->protocol != l4proto)
		return;
	if (l3proto == ETH_P_IPV6 && ipv6_hdr(skb)->nexthdr != l4proto)
		return;
	/* validate ports */
	if (l4proto == IPPROTO_UDP) {
		hdr_src = udp_hdr(skb)->source;
		hdr_dst = udp_hdr(skb)->dest;
	} else if (l4proto == IPPROTO_TCP) {
		hdr_src = tcp_hdr(skb)->source;
		hdr_dst = tcp_hdr(skb)->dest;
	}
	if ((srcport && hdr_src != htons(srcport)) ||
	    (dstport && hdr_dst != htons(dstport)))
		return;

	(*flags) |= new_flags;
}

static int get_classification_flags(const struct nf_hook_ops *ops,
				    struct sk_buff *skb)
{
	int flags = 0;
	int new_flags;

	switch (ops->hooknum) {
	case NF_INET_FORWARD:
		flags |= SW_APP | SW_URL_QUERY | SW_DEVID;
		break;

	case NF_INET_LOCAL_IN:
		/* ignore packets from the WAN */
		if (is_wan_dev(skb))
			break;

		/* accept DHCP */
		new_flags = SW_DEVID;
		cf_l3v4(&flags, skb, IPPROTO_UDP,  67,  68, new_flags);
		cf_l3v6(&flags, skb, IPPROTO_UDP, 546, 547, new_flags);

		/* accept DNS */
		new_flags = SW_APP | SW_URL_QUERY | SW_DEVID;
		cf_l3v4(&flags, skb, IPPROTO_UDP, 0, 53, new_flags);
		cf_l3v6(&flags, skb, IPPROTO_UDP, 0, 53, new_flags);
		break;

	case NF_INET_LOCAL_OUT:
		/* accept DNS */
		new_flags = SW_APP;
		cf_l3v4(&flags, skb, IPPROTO_UDP, 53, 0, new_flags);
		cf_l3v6(&flags, skb, IPPROTO_UDP, 53, 0, new_flags);
		break;
	}

	return flags;
}

static unsigned int dpi_nf_hook(const struct nf_hook_ops *ops,
				struct sk_buff *skb,
				const struct nf_hook_state *state)
{
	if (!dpi_enabled)
		goto out;

	if (unlikely(!skb || !skb->dev)) {
		pr_debug("no skb or data, skipping packet\n");
		goto out;
	}

	dpi_classify(skb, get_classification_flags(ops, skb));

out:
	return NF_ACCEPT;
}

static unsigned int dpi_nf_block(const struct nf_hook_ops *ops,
				 struct sk_buff *skb,
				 const struct nf_hook_state *state)
{
	if (unlikely(!skb || !skb->dev || !skb->nfct))
		goto out;

	if (test_bit(DPI_CT_BLOCK_FLOW_BIT, &dpi_info(skb).flags)) {
		dpi_stats.blocked_pkts++;
		return NF_DROP;
	}

out:
	return NF_ACCEPT;
}


/* ----- netlink handling functions ----- */
#define dpi_nl_data_ptr(_skb)  \
	(NLMSG_DATA((struct nlmsghdr *)(_skb)->data) + sizeof(DpictlMsgHdr_t))

static void dpi_nl_process_maxpkt(struct sk_buff *skb)
{
	int *cfg = (int *) dpi_nl_data_ptr(skb);

	if (!cfg)
		return;
	dpi_max_pkt = *cfg;
}

static void dpi_nl_process_status(struct sk_buff *skb)
{
	dpi_nl_msg_reply(skb, DPICTL_NLMSG_STATUS,
			 sizeof(dpi_enabled), &dpi_enabled);
}

static void dpi_nl_handler(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = (struct nlmsghdr *)skb->data;
	DpictlMsgHdr_t *msg;

	if (skb->len < NLMSG_SPACE(0))
		return;

	msg = (DpictlMsgHdr_t *) NLMSG_DATA(nlh);
	if (nlh->nlmsg_len < sizeof(*nlh) || skb->len < nlh->nlmsg_len)
		return;

	switch (msg->type) {
	case DPICTL_NLMSG_ENABLE:
		dpi_enabled = 1;
		break;

	case DPICTL_NLMSG_DISABLE:
		dpi_enabled = 0;
		break;

	case DPICTL_NLMSG_STATUS:
		dpi_nl_process_status(skb);
		break;

	case DPICTL_NLMSG_MAXPKT:
		dpi_nl_process_maxpkt(skb);
		break;

	case DPICTL_NLMSG_RESET_STATS:
		dpi_reset_stats();
		break;

	default:
		if (dpi_hooks.nl_handler && !dpi_hooks.nl_handler(skb))
			break;
		pr_err("unknown msg type '%d'\n", msg->type);
		break;
	}
}

/* ----- public interface functions ----- */
noinline struct dpi_info *dpi_info_get(struct nf_conn *conn)
{
	return &conn->dpi;
}
EXPORT_SYMBOL(dpi_info_get);

uint32_t dpi_app_id(struct dpi_app *app)
{
	return app ? app->app_id : 0;
}
EXPORT_SYMBOL(dpi_app_id);

uint32_t dpi_dev_id(struct dpi_dev *dev)
{
	return dev ? dev->dev_id : 0;
}
EXPORT_SYMBOL(dpi_dev_id);

uint8_t *dpi_mac(struct dpi_dev *dev)
{
	return dev ? dev->mac : NULL;
}
EXPORT_SYMBOL(dpi_mac);

int dpi_url_len(struct dpi_url *url)
{
	return url ? url->len : 0;
}
EXPORT_SYMBOL(dpi_url_len);

char *dpi_url(struct dpi_url *url)
{
	return url ? url->hostname : NULL;
}
EXPORT_SYMBOL(dpi_url);

uint32_t dpi_appinst_app_id(struct dpi_appinst *appinst)
{
	return appinst->app ? appinst->app->app_id : 0;
}
EXPORT_SYMBOL(dpi_appinst_app_id);

uint8_t *dpi_appinst_mac(struct dpi_appinst *appinst)
{
	return appinst->dev ? appinst->dev->mac : NULL;
}
EXPORT_SYMBOL(dpi_appinst_mac);

uint32_t dpi_appinst_dev_id(struct dpi_appinst *appinst)
{
	return appinst->dev ? appinst->dev->dev_id : 0;
}
EXPORT_SYMBOL(dpi_appinst_dev_id);

struct dpi_ct_stats *dpi_appinst_stats(struct nf_conn *ct, int dir)
{
	if (dir_downstream(ct, dir))
		return &ct->dpi.appinst->ds;
	else
		return &ct->dpi.appinst->us;
}
EXPORT_SYMBOL(dpi_appinst_stats);

struct dpi_ct_stats *dpi_dev_stats(struct nf_conn *ct, int dir)
{
	if (dir_downstream(ct, dir))
		return &ct->dpi.dev->ds;
	else
		return &ct->dpi.dev->us;
}
EXPORT_SYMBOL(dpi_dev_stats);

void dpi_print_flow(struct seq_file *s, struct nf_conn *ct)
{
	seq_printf(s, "%u %pM %u",
		   ct->dpi.app->app_id,
		   ct->dpi.dev->mac,
		   ct->dpi.dev->dev_id);
}
EXPORT_SYMBOL(dpi_print_flow);

int dpi_bind(struct dpi_hooks *hooks)
{
	dpi_hooks.nl_handler  = hooks->nl_handler;
	dpi_hooks.cpu_enqueue = hooks->cpu_enqueue;
	return 0;
}
EXPORT_SYMBOL(dpi_bind);

void dpi_block(struct nf_conn *ct)
{
	blog_lock();

	if (test_bit(DPI_CT_BLOCK_FLOW_BIT, &ct->dpi.flags)) {
		/* if conntrack is not programmed, skip flush */
		if (ct->blog_key[IP_CT_DIR_ORIGINAL] == BLOG_KEY_FC_INVALID &&
		    ct->blog_key[IP_CT_DIR_REPLY]    == BLOG_KEY_FC_INVALID)
			goto out;

		blog_notify(DESTROY_FLOWTRACK, (void*)ct,
			    (uint32_t)ct->blog_key[IP_CT_DIR_ORIGINAL],
			    (uint32_t)ct->blog_key[IP_CT_DIR_REPLY]);

		ct->blog_key[IP_CT_DIR_ORIGINAL] = BLOG_KEY_FC_INVALID;
		ct->blog_key[IP_CT_DIR_REPLY]    = BLOG_KEY_FC_INVALID;
	} else {
		/* enable conntrack blogging */
		set_bit(IPS_BLOG_BIT, &ct->status);
	}

out:
	blog_unlock();
}
EXPORT_SYMBOL(dpi_block);

void dpi_nl_msg_reply(struct sk_buff *skb, int type, unsigned short len,
		      void *data)
{
	DpictlMsgHdr_t  *hdr;
	struct nlmsghdr *nlh = (struct nlmsghdr *)skb->data;
	struct nlmsghdr *new_nlh;
	struct sk_buff  *new_skb;
	int pid = nlh->nlmsg_pid;
	int buf_size;

	buf_size = NLMSG_SPACE(sizeof(DpictlMsgHdr_t) + len);
	new_skb = alloc_skb(buf_size, GFP_ATOMIC);
	if (!new_skb) {
		pr_err("couldn't allocate memory for skb\n");
		return;
	}

	new_nlh = nlmsg_put(new_skb, 0, 0, NLMSG_DONE, buf_size, 0);
	hdr = NLMSG_DATA(new_nlh);

	hdr->type = type;
	hdr->len = len;
	hdr++;

	memcpy(hdr, data, len);

	NETLINK_CB(new_skb).dst_group = 0;

	if (netlink_unicast(nl_sk, new_skb, pid, MSG_DONTWAIT) < 0)
		pr_err("error while sending DPI status\n");
}
EXPORT_SYMBOL(dpi_nl_msg_reply);

int dpi_cpu_enqueue(pNBuff_t pNBuff, struct net_device *dev)
{
	if (dpi_hooks.cpu_enqueue)
		return dpi_hooks.cpu_enqueue(pNBuff, dev);
	return -1;
}
EXPORT_SYMBOL(dpi_cpu_enqueue);


/* ----- driver funs ----- */
static int dpi_stat_seq_show(struct seq_file *s, void *v)
{
	seq_printf(s, "%-9s %-10s   %-10s   %-10s   %-10s   %-10s\n",
		   "type", "lookups", "hits", "misses", "classified", "unclassified");
	seq_puts(s, "---------------------------------------------------------------------------\n");
	seq_printf(s, " app      %-10u   %-10u   %-10u   %-10u   %-10u\n",
		   dpi_stats.apps.lookups,
		   dpi_stats.apps.hits,
		   dpi_stats.apps.misses,
		   dpi_stats.apps.classified,
		   dpi_stats.apps.unclassified);
	seq_printf(s, " device   %-10u   %-10u   %-10u   %-10u   %-10u\n",
		   dpi_stats.devs.lookups,
		   dpi_stats.devs.hits,
		   dpi_stats.apps.misses,
		   dpi_stats.devs.classified,
		   dpi_stats.devs.unclassified);
	seq_printf(s, " url      %-10u   %-10u   %-10u   %-10u   %-10u\n",
		   dpi_stats.urls.lookups,
		   dpi_stats.urls.hits,
		   dpi_stats.apps.misses,
		   dpi_stats.urls.classified,
		   dpi_stats.urls.unclassified);
	seq_printf(s, " total    %-10u\n", dpi_stats.total_lookups);
	seq_puts(s, "---------------------------------------------------------------------------\n");
	seq_printf(s, "    apps identified: %u\n", dpi_stats.app_count);
	seq_printf(s, " devices identified: %u\n", dpi_stats.dev_count);
	seq_printf(s, "appinsts identified: %u\n", dpi_stats.appinst_count);
	seq_printf(s, "    urls identified: %u\n", dpi_stats.url_count);
	seq_printf(s, "    packets blocked: %llu\n", dpi_stats.blocked_pkts);
	seq_printf(s, "      engine errors: %u\n", dpi_stats.engine_errors);
	return 0;
}
static int dpi_stat_open(struct inode *inode, struct file *file)
{
	return single_open(file, dpi_stat_seq_show, NULL);
};
static const struct file_operations dpi_stat_fops = {
	.open    = dpi_stat_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release
};

static int dpi_info_seq_show(struct seq_file *s, void *v)
{
	unsigned int val, timeout;

	seq_printf(s, " DPI engine version : %d.%d.%d%s\n",
		      TMCFG_E_MAJ_VER,
		      TMCFG_E_MID_VER,
		      TMCFG_E_MIN_VER,
		      TMCFG_E_LOCAL_VER);
	tdts_shell_system_setting_tcp_conn_max_get(&val);
	tdts_shell_system_setting_tcp_conn_timeout_get(&timeout);
	seq_printf(s, " Max TCP connections: %-8u  Timeout: %u\n", val,
		      timeout);
	tdts_shell_system_setting_udp_conn_max_get(&val);
	seq_printf(s, " Max UDP connections: %u\n", val);
	return 0;
}
static int dpi_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, dpi_info_seq_show, NULL);
};
static const struct file_operations dpi_info_fops = {
	.open    = dpi_info_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release
};

static int __init dpicore_init(void)
{
	struct proc_dir_entry *pde;
	struct netlink_kernel_cfg cfg = {
		.groups = 0,
		.input  = dpi_nl_handler,
	};

	memset(&dpi_stats, 0, sizeof(dpi_stats));
	memset(&dpi_hooks, 0, sizeof(dpi_hooks));

	/* create proc entries */
	dpi_dir = proc_mkdir("dpi", NULL);
	if (!dpi_dir) {
		pr_err("couldn't create dpi proc directory\n");
		goto err;
	}
	pde = proc_create("stats", 0440, dpi_dir, &dpi_stat_fops);
	if (!pde) {
		pr_err("couldn't create proc entry 'stats'\n");
		goto err_free_dpi_dir;
	}
	pde = proc_create("info", 0440, dpi_dir, &dpi_info_fops);
	if (!pde) {
		pr_err("couldn't create proc entry 'info'\n");
		goto err_free_stats;
	}

	/* initialize kernel tables */
	if (dpi_init_tables())
		goto err_free_info;

	/* register netfilter hooks */
	if (nf_register_hooks(hooks, ARRAY_SIZE(hooks))) {
		pr_err("cannot register netfilter hooks\n");
		goto err_free_tables;
	}

	nl_sk = netlink_kernel_create(&init_net, NETLINK_DPI, &cfg);
	if (!nl_sk) {
		pr_err("failed to create kernel netlink socket\n");
		goto err_unreg_nf_hooks;
	}
	return 0;

err_unreg_nf_hooks:
	nf_unregister_hooks(hooks, ARRAY_SIZE(hooks));
err_free_tables:
	dpi_deinit_tables();
err_free_info:
	remove_proc_entry("info", dpi_dir);
err_free_stats:
	remove_proc_entry("stats", dpi_dir);
err_free_dpi_dir:
	proc_remove(dpi_dir);
err:
	return -1;
}

static void __exit dpicore_exit(void)
{
	nf_unregister_hooks(hooks, ARRAY_SIZE(hooks));

	if (nl_sk)
		netlink_kernel_release(nl_sk);

	/* deinitialize tables */
	dpi_deinit_tables();

	/* remove proc entries */
	remove_proc_entry("info", dpi_dir);
	remove_proc_entry("stats", dpi_dir);
	proc_remove(dpi_dir);
}

module_init(dpicore_init);
module_exit(dpicore_exit);

MODULE_LICENSE("GPL");
