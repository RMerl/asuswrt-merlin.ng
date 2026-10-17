#ifndef _BCM_NFNETLINK_CONNTRACK_H
#define _BCM_NFNETLINK_CONNTRACK_H

#include <linux/types.h>
#include <linux/blog.h>
#include <net/netfilter/nf_conntrack.h>
#include <uapi/linux/netfilter/bcm_nfnetlink_conntrack.h>

int bcm_ctnetlink_size(const struct nf_conn *ct);
int bcm_ctnetlink_dump(struct sk_buff *skb, const struct nf_conn *ct,
		       unsigned int events);
int bcm_ctnetlink_change(struct nf_conn *ct, const struct nlattr * const cda[]);

/* helpers */
static inline void
bcm_ctnetlink_dump_blog_counters(const struct nf_conn *ct, int dir, u64 *pkts,
				 u64 *bytes)
{
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	/*TODO: support CTRZERO for fast_stats */
	BlogFcStats_t fast_stats = {0};

	blog_ct_get_stats(ct, ct->bcm_ext.blog_key[dir], dir, &fast_stats);

	*pkts += fast_stats.rx_packets;
	*bytes += fast_stats.rx_bytes;
#endif
}

#endif /* _BCM_NFNETLINK_CONNTRACK_H */
