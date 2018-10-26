#if defined(CONFIG_BCM_MPTCP) && defined(CONFIG_BCM_KF_MPTCP)
/*
 *	MPTCP implementation
 *
 *	Initial Design & Implementation:
 *	Sébastien Barré <sebastien.barre@uclouvain.be>
 *
 *	Current Maintainer & Author:
 *	Jaakko Korkeaniemi <jaakko.korkeaniemi@aalto.fi>
 *
 *	Additional authors:
 *	Jaakko Korkeaniemi <jaakko.korkeaniemi@aalto.fi>
 *	Gregory Detal <gregory.detal@uclouvain.be>
 *	Fabien Duchêne <fabien.duchene@uclouvain.be>
 *	Andreas Seelinger <Andreas.Seelinger@rwth-aachen.de>
 *	Lavkesh Lahngir <lavkesh51@gmail.com>
 *	Andreas Ripke <ripke@neclab.eu>
 *	Vlad Dogaru <vlad.dogaru@intel.com>
 *	Octavian Purdila <octavian.purdila@intel.com>
 *	John Ronan <jronan@tssg.org>
 *	Catalin Nicutar <catalin.nicutar@gmail.com>
 *	Brandon Heller <brandonh@stanford.edu>
 *
 *
 *	This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */

#ifndef _MPTCP_V6_H
#define _MPTCP_V6_H

#include <linux/in6.h>
#include <net/if_inet6.h>

#include <net/mptcp.h>


#ifdef CONFIG_MPTCP
extern const struct inet_connection_sock_af_ops mptcp_v6_mapped;
extern const struct inet_connection_sock_af_ops mptcp_v6_specific;
extern struct request_sock_ops mptcp6_request_sock_ops;
extern struct tcp_request_sock_ops mptcp_request_sock_ipv6_ops;
extern struct tcp_request_sock_ops mptcp_join_request_sock_ipv6_ops;

int mptcp_v6_do_rcv(struct sock *meta_sk, struct sk_buff *skb);
struct sock *mptcp_v6_search_req(const __be16 rport, const struct in6_addr *raddr,
				 const struct in6_addr *laddr, const struct net *net);
int mptcp_init6_subsockets(struct sock *meta_sk, const struct mptcp_loc6 *loc,
			   struct mptcp_rem6 *rem);
int mptcp_pm_v6_init(void);
void mptcp_pm_v6_undo(void);
__u32 mptcp_v6_get_nonce(const __be32 *saddr, const __be32 *daddr,
			 __be16 sport, __be16 dport);
u64 mptcp_v6_get_key(const __be32 *saddr, const __be32 *daddr,
		     __be16 sport, __be16 dport, u32 seed);

#else /* CONFIG_MPTCP */

#define mptcp_v6_mapped ipv6_mapped

static inline int mptcp_v6_do_rcv(struct sock *meta_sk, struct sk_buff *skb)
{
	return 0;
}

#endif /* CONFIG_MPTCP */

#endif /* _MPTCP_V6_H */
#endif
