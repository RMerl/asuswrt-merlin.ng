#ifndef _BCM_NF_DEFRAG_IPV6_H
#define	_BCM_NF_DEFRAG_IPV6_H

extern void
nf_ct_frag6_ident_reuse(struct frag_queue *fq, struct sk_buff *skb,
			struct net_device *dev);

#endif /* _BCM_NF_DEFRAG_IPV6_H */
