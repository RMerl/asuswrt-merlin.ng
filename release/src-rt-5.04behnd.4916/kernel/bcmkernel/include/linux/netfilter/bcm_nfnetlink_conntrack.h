#ifndef _BCM_NFNETLINK_CONNTRACK_H
#define _BCM_NFNETLINK_CONNTRACK_H

extern int bcm_ctnetlink_size(const struct nf_conn *ct);
extern int bcm_ctnetlink_dump(struct sk_buff *skb, const struct nf_conn *ct);
extern int bcm_ctnetlink_change(struct nf_conn *ct,
				const struct nlattr * const cda[]);

#endif /* _BCM_NFNETLINK_CONNTRACK_H */
