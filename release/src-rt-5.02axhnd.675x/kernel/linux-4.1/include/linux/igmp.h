/*
 *	Linux NET3:	Internet Group Management Protocol  [IGMP]
 *
 *	Authors:
 *		Alan Cox <alan@lxorguk.ukuu.org.uk>
 *
 *	Extended to talk the BSD extended IGMP protocol of mrouted 3.6
 *
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */
#ifndef _LINUX_IGMP_H
#define _LINUX_IGMP_H

#include <linux/skbuff.h>
#include <linux/timer.h>
#include <linux/in.h>
#include <uapi/linux/igmp.h>

static inline struct igmphdr *igmp_hdr(const struct sk_buff *skb)
{
	return (struct igmphdr *)skb_transport_header(skb);
}

static inline struct igmpv3_report *
			igmpv3_report_hdr(const struct sk_buff *skb)
{
	return (struct igmpv3_report *)skb_transport_header(skb);
}

static inline struct igmpv3_query *
			igmpv3_query_hdr(const struct sk_buff *skb)
{
	return (struct igmpv3_query *)skb_transport_header(skb);
}

extern int sysctl_igmp_max_memberships;
extern int sysctl_igmp_max_msf;
extern int sysctl_igmp_qrv;

struct ip_sf_socklist {
	unsigned int		sl_max;
	unsigned int		sl_count;
	struct rcu_head		rcu;
	__be32			sl_addr[0];
};

#define IP_SFLSIZE(count)	(sizeof(struct ip_sf_socklist) + \
	(count) * sizeof(__be32))

#define IP_SFBLOCK	10	/* allocate this many at once */

/* ip_mc_socklist is real list now. Speed is not argument;
   this list never used in fast path code
 */

struct ip_mc_socklist {
	struct ip_mc_socklist __rcu *next_rcu;
	struct ip_mreqn		multi;
	unsigned int		sfmode;		/* MCAST_{INCLUDE,EXCLUDE} */
	struct ip_sf_socklist __rcu	*sflist;
	struct rcu_head		rcu;
};

struct ip_sf_list {
	struct ip_sf_list	*sf_next;
	__be32			sf_inaddr;
	unsigned long		sf_count[2];	/* include/exclude counts */
	unsigned char		sf_gsresp;	/* include in g & s response? */
	unsigned char		sf_oldin;	/* change state */
	unsigned char		sf_crcount;	/* retrans. left to send */
};

struct ip_mc_list {
	struct in_device	*interface;
	__be32			multiaddr;
	unsigned int		sfmode;
	struct ip_sf_list	*sources;
	struct ip_sf_list	*tomb;
#if defined(CONFIG_BCM_KF_MCAST_GR_SUPPRESSION)
	unsigned int		osfmode;
#endif
	unsigned long		sfcount[2];
	union {
		struct ip_mc_list *next;
		struct ip_mc_list __rcu *next_rcu;
	};
	struct ip_mc_list __rcu *next_hash;
	struct timer_list	timer;
	int			users;
	atomic_t		refcnt;
	spinlock_t		lock;
	char			tm_running;
	char			reporter;
	char			unsolicit_count;
	char			loaded;
	unsigned char		gsquery;	/* check source marks? */
	unsigned char		crcount;
	struct rcu_head		rcu;
};

/* V3 exponential field decoding */
#define IGMPV3_MASK(value, nb) ((nb)>=32 ? (value) : ((1<<(nb))-1) & (value))
#define IGMPV3_EXP(thresh, nbmant, nbexp, value) \
	((value) < (thresh) ? (value) : \
        ((IGMPV3_MASK(value, nbmant) | (1<<(nbmant))) << \
         (IGMPV3_MASK((value) >> (nbmant), nbexp) + (nbexp))))

#define IGMPV3_QQIC(value) IGMPV3_EXP(0x80, 4, 3, value)
#define IGMPV3_MRC(value) IGMPV3_EXP(0x80, 4, 3, value)

extern int ip_check_mc_rcu(struct in_device *dev, __be32 mc_addr, __be32 src_addr, u16 proto);
extern int igmp_rcv(struct sk_buff *);
extern int ip_mc_join_group(struct sock *sk, struct ip_mreqn *imr);
extern int ip_mc_leave_group(struct sock *sk, struct ip_mreqn *imr);
extern void ip_mc_drop_socket(struct sock *sk);
extern int ip_mc_source(int add, int omode, struct sock *sk,
		struct ip_mreq_source *mreqs, int ifindex);
extern int ip_mc_msfilter(struct sock *sk, struct ip_msfilter *msf,int ifindex);
extern int ip_mc_msfget(struct sock *sk, struct ip_msfilter *msf,
		struct ip_msfilter __user *optval, int __user *optlen);
extern int ip_mc_gsfget(struct sock *sk, struct group_filter *gsf,
		struct group_filter __user *optval, int __user *optlen);
extern int ip_mc_sf_allow(struct sock *sk, __be32 local, __be32 rmt, int dif);
extern void ip_mc_init_dev(struct in_device *);
extern void ip_mc_destroy_dev(struct in_device *);
extern void ip_mc_up(struct in_device *);
extern void ip_mc_down(struct in_device *);
extern void ip_mc_unmap(struct in_device *);
extern void ip_mc_remap(struct in_device *);
extern void ip_mc_dec_group(struct in_device *in_dev, __be32 addr);
extern void ip_mc_inc_group(struct in_device *in_dev, __be32 addr);

#endif
