#if defined(CONFIG_BCM_KF_MPTCP) && defined(CONFIG_BCM_MPTCP)
// SPDX-License-Identifier: GPL-2.0
/*	MPTCP implementation - Netlink Path Manager
 *
 *	Analysis, Design and Implementation:
 *	- Gregory Detal <gregory.detal@tessares.net>
 *	- Sébastien Barré <sebastien.barre@tessares.net>
 *	- Matthieu Baerts <matthieu.baerts@tessares.net>
 *	- Pau Espin Pedrol <pau.espin@tessares.net>
 *	- Detlev Casanova <detlev.casanova@tessares.net>
 *	- David Verbeiren <david.verbeiren@tessares.net>
 *	- Frank Vanbever <frank.vanbever@tessares.net>
 *	- Antoine Maes <antoine.maes@tessares.net>
 *	- Tim Froidcoeur <tim.froidcoeur@tessares.net>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/module.h>
#include <linux/mptcp.h>
#include <net/genetlink.h>
#include <net/mptcp.h>
#include <net/mptcp_v4.h>
#if IS_ENABLED(CONFIG_IPV6)
#include <net/mptcp_v6.h>
#endif

#define MPTCP_MAX_ADDR	8

struct mptcp_nl_priv {
	/* Unfortunately we need to store this to generate MP_JOINs in case
	 * of the peer generating a subflow (see get_local_id).
	 */
	u8			loc4_bits;
	u8			announced4;
	struct mptcp_loc4	locaddr4[MPTCP_MAX_ADDR];

#if IS_ENABLED(CONFIG_IPV6)
	u8			loc6_bits;
	u8			announced6;
	struct mptcp_loc6	locaddr6[MPTCP_MAX_ADDR];
#endif

	u16			remove_addrs;

	bool			is_closed;
};

static struct genl_family mptcp_genl_family;

#define MPTCP_GENL_EV_GRP_OFFSET	0
#define MPTCP_GENL_CMD_GRP_OFFSET	1

static const struct genl_multicast_group mptcp_mcgrps[] = {
	[MPTCP_GENL_EV_GRP_OFFSET]	= { .name = MPTCP_GENL_EV_GRP_NAME,  },
	[MPTCP_GENL_CMD_GRP_OFFSET]	= { .name = MPTCP_GENL_CMD_GRP_NAME, },
};

static const struct nla_policy mptcp_nl_genl_policy[MPTCP_ATTR_MAX + 1] = {
	[MPTCP_ATTR_TOKEN]	= { .type	= NLA_U32,	},
	[MPTCP_ATTR_FAMILY]	= { .type	= NLA_U16,	},
	[MPTCP_ATTR_LOC_ID]	= { .type	= NLA_U8,	},
	[MPTCP_ATTR_REM_ID]	= { .type	= NLA_U8,	},
	[MPTCP_ATTR_SADDR4]	= { .type	= NLA_U32,	},
	[MPTCP_ATTR_SADDR6]	= { .type	= NLA_BINARY,
				    .len	= sizeof(struct in6_addr), },
	[MPTCP_ATTR_DADDR4]	= { .type	= NLA_U32,	},
	[MPTCP_ATTR_DADDR6]	= { .type	= NLA_BINARY,
				    .len	= sizeof(struct in6_addr), },
	[MPTCP_ATTR_SPORT]	= { .type	= NLA_U16,	},
	[MPTCP_ATTR_DPORT]	= { .type	= NLA_U16,	},
	[MPTCP_ATTR_BACKUP]	= { .type	= NLA_U8,	},
	[MPTCP_ATTR_TIMEOUT]	= { .type	= NLA_U32,	},
	[MPTCP_ATTR_IF_IDX]	= { .type	= NLA_S32,	},
};

/* Defines the userspace PM filter on events. Set events are ignored. */
static u16 mptcp_nl_event_filter;

static inline struct mptcp_nl_priv *
mptcp_nl_priv(const struct sock *meta_sk)
{
	return (struct mptcp_nl_priv *)&tcp_sk(meta_sk)->mpcb->mptcp_pm[0];
}

static inline bool
mptcp_nl_must_notify(u16 event, const struct sock *meta_sk)
{
	struct mptcp_nl_priv *priv = mptcp_nl_priv(meta_sk);

	/* close_session() can be called before other events because it is
	 * also called when doing a fallback to TCP. We don't want to send
	 * events to the user-space after having sent the CLOSED event.
	 */
	if (priv->is_closed)
		return false;

	if (event == MPTCPF_EVENT_CLOSED)
		priv->is_closed = true;

	if (mptcp_nl_event_filter & event)
		return false;

	if (!genl_has_listeners(&mptcp_genl_family, sock_net(meta_sk), 0))
		return false;

	return true;
}

/* Find the first free index in the bitfield starting from 0 */
static int
mptcp_nl_find_free_index(u8 bitfield)
{
	int i;

	/* There are anyways no free bits... */
	if (bitfield == 0xff)
		return -1;

	i = ffs(~bitfield) - 1;
	if (i < 0)
		return -1;

	return i;
}

static inline int
mptcp_nl_put_subsk(struct sk_buff *msg, struct sock *sk)
{
	struct inet_sock	*isk	= inet_sk(sk);
	struct sock		*meta_sk = mptcp_meta_sk(sk);
	u8			backup;
	u8			sk_err;

	if (nla_put_u16(msg, MPTCP_ATTR_FAMILY, sk->sk_family))
		goto nla_put_failure;

	if (nla_put_u8(msg, MPTCP_ATTR_LOC_ID, tcp_sk(sk)->mptcp->loc_id))
		goto nla_put_failure;

	if (nla_put_u8(msg, MPTCP_ATTR_REM_ID, tcp_sk(sk)->mptcp->rem_id))
		goto nla_put_failure;

	switch (sk->sk_family) {
	case AF_INET:
		if (nla_put_u32(msg, MPTCP_ATTR_SADDR4, isk->inet_saddr))
			goto nla_put_failure;

		if (nla_put_u32(msg, MPTCP_ATTR_DADDR4, isk->inet_daddr))
			goto nla_put_failure;
		break;
#if IS_ENABLED(CONFIG_IPV6)
	case AF_INET6: {
		struct ipv6_pinfo *np = inet6_sk(sk);

		if (nla_put(msg, MPTCP_ATTR_SADDR6, sizeof(np->saddr),
			    &np->saddr))
			goto nla_put_failure;

		if (nla_put(msg, MPTCP_ATTR_DADDR6, sizeof(sk->sk_v6_daddr),
			    &sk->sk_v6_daddr))
			goto nla_put_failure;
		break;
	}
#endif
	default:
		goto nla_put_failure;
	}

	if (nla_put_u16(msg, MPTCP_ATTR_SPORT, ntohs(isk->inet_sport)))
		goto nla_put_failure;

	if (nla_put_u16(msg, MPTCP_ATTR_DPORT, ntohs(isk->inet_dport)))
		goto nla_put_failure;

	backup = !!(tcp_sk(sk)->mptcp->rcv_low_prio ||
		    tcp_sk(sk)->mptcp->low_prio);

	if (nla_put_u8(msg, MPTCP_ATTR_BACKUP, backup))
		goto nla_put_failure;

	if (nla_put_s32(msg, MPTCP_ATTR_IF_IDX, sk->sk_bound_dev_if))
		goto nla_put_failure;

	sk_err = sk->sk_err ? : tcp_sk(sk)->mptcp->sk_err;
	if (unlikely(sk_err != 0) && meta_sk->sk_state == TCP_ESTABLISHED &&
	    nla_put_u8(msg, MPTCP_ATTR_ERROR, sk_err))
		goto nla_put_failure;

	return 0;

nla_put_failure:
	return -1;
}

static inline struct sk_buff *
mptcp_nl_mcast_prepare(struct mptcp_cb *mpcb, struct sock *sk, int cmd,
		       void **hdr)
{
	struct sk_buff *msg;

	/* possible optimisation: use the needed size */
	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_ATOMIC);
	if (!msg)
		return NULL;

	*hdr = genlmsg_put(msg, 0, 0, &mptcp_genl_family, 0, cmd);
	if (!*hdr)
		goto free_msg;

	if (nla_put_u32(msg, MPTCP_ATTR_TOKEN, mpcb->mptcp_loc_token))
		goto nla_put_failure;

	if (sk && mptcp_nl_put_subsk(msg, sk))
		goto nla_put_failure;

	return msg;

nla_put_failure:
	genlmsg_cancel(msg, *hdr);
free_msg:
	nlmsg_free(msg);
	return NULL;
}

static inline int
mptcp_nl_mcast_send(struct mptcp_cb *mpcb, struct sk_buff *msg, void *hdr)
{
	int		ret;
	struct sock	*meta_sk = mpcb->meta_sk;

	genlmsg_end(msg, hdr);

	ret = genlmsg_multicast_netns(&mptcp_genl_family, sock_net(meta_sk),
				      msg, 0, MPTCP_GENL_EV_GRP_OFFSET,
				      GFP_ATOMIC);
	if (ret && ret != -ESRCH)
		pr_err("%s: genlmsg_multicast failed with %d\n", __func__, ret);
	return ret;
}

static inline void
mptcp_nl_mcast(struct mptcp_cb *mpcb, struct sock *sk, int cmd)
{
	void		*hdr;
	struct sk_buff	*msg;

	msg = mptcp_nl_mcast_prepare(mpcb, sk, cmd, &hdr);
	if (msg)
		mptcp_nl_mcast_send(mpcb, msg, hdr);
	else
		pr_warn("%s: unable to prepare multicast message\n", __func__);
}

static inline void
mptcp_nl_mcast_fail(struct sk_buff *msg, void *hdr)
{
	genlmsg_cancel(msg, hdr);
	nlmsg_free(msg);
}

static void
mptcp_nl_new(const struct sock *meta_sk, bool established)
{
	struct mptcp_cb *mpcb = tcp_sk(meta_sk)->mpcb;

	mptcp_nl_mcast(mpcb, mpcb->master_sk,
		       established ? MPTCP_EVENT_ESTABLISHED
		       : MPTCP_EVENT_CREATED);
}

static void
mptcp_nl_pm_new_session(const struct sock *meta_sk)
{
	if (!mptcp_nl_must_notify(MPTCPF_EVENT_CREATED, meta_sk))
		return;

	mptcp_nl_new(meta_sk, false);
}

static inline int
mptcp_nl_loc_id_to_index_lookup(struct sock *meta_sk, sa_family_t family,
				u8 addr_id)
{
	struct mptcp_nl_priv	*priv = mptcp_nl_priv(meta_sk);
	int			i;

	switch (family) {
	case AF_INET:
		mptcp_for_each_bit_set(priv->loc4_bits, i) {
			if (priv->locaddr4[i].loc4_id == addr_id)
				return i;
		}
		break;
#if IS_ENABLED(CONFIG_IPV6)
	case AF_INET6:
		mptcp_for_each_bit_set(priv->loc6_bits, i) {
			if (priv->locaddr6[i].loc6_id == addr_id)
				return i;
		}
		break;
#endif
	}
	return -1;
}

static inline void
mptcp_nl_sk_setup_locaddr(struct sock *meta_sk, struct sock *sk)
{
	struct mptcp_nl_priv	*priv	= mptcp_nl_priv(meta_sk);
	bool			backup	= !!(tcp_sk(sk)->mptcp->rcv_low_prio ||
					     tcp_sk(sk)->mptcp->low_prio);
	sa_family_t family = mptcp_v6_is_v4_mapped(sk) ? AF_INET
			     : sk->sk_family;
	u8	addr_id = tcp_sk(sk)->mptcp->loc_id;
	int	idx	= mptcp_nl_loc_id_to_index_lookup(meta_sk, family,
							  addr_id);

	/* Same as in mptcp_fullmesh.c: exception for transparent sockets */
	int if_idx = inet_sk(sk)->transparent ? inet_sk(sk)->rx_dst_ifindex :
							sk->sk_bound_dev_if;

	switch (family) {
	case AF_INET: {
		struct inet_sock *isk = inet_sk(sk);

		if (idx == -1)
			idx = mptcp_nl_find_free_index(priv->loc4_bits);
		if (idx == -1) {
			pr_warn("No free index for sk loc_id v4\n");
			return;
		}
		priv->locaddr4[idx].addr.s_addr = isk->inet_saddr;
		priv->locaddr4[idx].loc4_id	= addr_id;
		priv->locaddr4[idx].low_prio	= backup;
		priv->locaddr4[idx].if_idx	= if_idx;
		priv->loc4_bits			|= 1 << idx;
		priv->announced4		|= 1 << idx;
		break;
	}
#if IS_ENABLED(CONFIG_IPV6)
	case AF_INET6: {
		struct ipv6_pinfo *np = inet6_sk(sk);

		if (idx == -1)
			idx = mptcp_nl_find_free_index(priv->loc6_bits);
		if (idx == -1) {
			pr_warn("No free index for sk loc_id v6\n");
			return;
		}
		priv->locaddr6[idx].addr	= np->saddr;
		priv->locaddr6[idx].loc6_id	= addr_id;
		priv->locaddr6[idx].low_prio	= backup;
		priv->locaddr6[idx].if_idx	= if_idx;
		priv->loc6_bits			|= 1 << idx;
		priv->announced6		|= 1 << idx;
		break;
	}
#endif
	}
}

static void
mptcp_nl_pm_fully_established(struct sock *meta_sk)
{
	mptcp_nl_sk_setup_locaddr(meta_sk, tcp_sk(meta_sk)->mpcb->master_sk);

	if (!mptcp_nl_must_notify(MPTCPF_EVENT_ESTABLISHED, meta_sk))
		return;

	mptcp_nl_new(meta_sk, true);
}

static void
mptcp_nl_pm_close_session(struct sock *meta_sk)
{
	if (!mptcp_nl_must_notify(MPTCPF_EVENT_CLOSED, meta_sk))
		return;

	mptcp_nl_mcast(tcp_sk(meta_sk)->mpcb, NULL, MPTCP_EVENT_CLOSED);
}

static void
mptcp_nl_pm_established_subflow(struct sock *sk)
{
	struct sock *meta_sk = mptcp_meta_sk(sk);

	mptcp_nl_sk_setup_locaddr(meta_sk, sk);

	if (!mptcp_nl_must_notify(MPTCPF_EVENT_SUB_ESTABLISHED, meta_sk))
		return;

	mptcp_nl_mcast(tcp_sk(meta_sk)->mpcb, sk, MPTCP_EVENT_SUB_ESTABLISHED);
}

static void
mptcp_nl_pm_delete_subflow(struct sock *sk)
{
	struct sock *meta_sk = mptcp_meta_sk(sk);

	if (!mptcp_nl_must_notify(MPTCPF_EVENT_SUB_CLOSED, meta_sk))
		return;

	mptcp_nl_mcast(tcp_sk(meta_sk)->mpcb, sk, MPTCP_EVENT_SUB_CLOSED);
}

static void
mptcp_nl_pm_add_raddr(struct mptcp_cb *mpcb, const union inet_addr *addr,
		      sa_family_t family, __be16 port, u8 id)
{
	struct sk_buff	*msg;
	void		*hdr;

	if (!mptcp_nl_must_notify(MPTCPF_EVENT_ANNOUNCED, mpcb->meta_sk))
		return;

	msg = mptcp_nl_mcast_prepare(mpcb, NULL, MPTCP_EVENT_ANNOUNCED, &hdr);
	if (!msg)
		return;

	if (nla_put_u8(msg, MPTCP_ATTR_REM_ID, id))
		goto nla_put_failure;

	if (nla_put_u16(msg, MPTCP_ATTR_FAMILY, family))
		goto nla_put_failure;

	switch (family) {
	case AF_INET:
		if (nla_put_u32(msg, MPTCP_ATTR_DADDR4, addr->ip))
			goto nla_put_failure;
		break;
#if IS_ENABLED(CONFIG_IPV6)
	case AF_INET6:
		if (nla_put(msg, MPTCP_ATTR_DADDR6, sizeof(addr->ip6),
			    &addr->ip6))
			goto nla_put_failure;
		break;
#endif
	default:
		goto nla_put_failure;
	}

	if (nla_put_u16(msg, MPTCP_ATTR_DPORT, ntohs(port)))
		goto nla_put_failure;

	mptcp_nl_mcast_send(mpcb, msg, hdr);

	return;

nla_put_failure:
	mptcp_nl_mcast_fail(msg, hdr);
}

static void
mptcp_nl_pm_rem_raddr(struct mptcp_cb *mpcb, u8 id)
{
	struct sk_buff	*msg;
	void		*hdr;

	if (!mptcp_nl_must_notify(MPTCPF_EVENT_REMOVED, mpcb->meta_sk))
		return;

	msg = mptcp_nl_mcast_prepare(mpcb, NULL, MPTCP_EVENT_REMOVED, &hdr);

	if (!msg)
		return;

	if (nla_put_u8(msg, MPTCP_ATTR_REM_ID, id))
		goto nla_put_failure;

	mptcp_nl_mcast_send(mpcb, msg, hdr);

	return;

nla_put_failure:
	mptcp_nl_mcast_fail(msg, hdr);
}

static int
mptcp_nl_pm_get_local_id(const struct sock *meta_sk, sa_family_t family,
			 union inet_addr *addr, bool *low_prio)
{
	struct mptcp_nl_priv	*priv = mptcp_nl_priv(meta_sk);
	int			i, id = 0;

	switch (family) {
	case AF_INET:
		mptcp_for_each_bit_set(priv->loc4_bits, i) {
			if (addr->in.s_addr == priv->locaddr4[i].addr.s_addr) {
				id		= priv->locaddr4[i].loc4_id;
				*low_prio	= priv->locaddr4[i].low_prio;
				goto out;
			}
		}
		break;
#if IS_ENABLED(CONFIG_IPV6)
	case AF_INET6:
		mptcp_for_each_bit_set(priv->loc6_bits, i) {
			if (ipv6_addr_equal(&addr->in6,
					    &priv->locaddr6[i].addr)) {
				id		= priv->locaddr6[i].loc6_id;
				*low_prio	= priv->locaddr6[i].low_prio;
				goto out;
			}
		}
		break;
#endif
	}
	return -1;

out:
	return id;
}

static void
mptcp_nl_pm_addr_signal(struct sock *sk, unsigned *size,
			struct tcp_out_options *opts, struct sk_buff *skb)
{
	struct mptcp_nl_priv	*priv	= mptcp_nl_priv(sk);
	struct mptcp_cb		*mpcb	= tcp_sk(sk)->mpcb;
	u8			unannounced;
	int			remove_addr_len;

	unannounced = (~priv->announced4) & priv->loc4_bits;
	if (unannounced &&
	    MAX_TCP_OPTION_SPACE - *size >= MPTCP_SUB_LEN_ADD_ADDR4_ALIGN) {
		int i = mptcp_nl_find_free_index(~unannounced);

		opts->options		|= OPTION_MPTCP;
		opts->mptcp_options	|= OPTION_ADD_ADDR;
		opts->add_addr4.addr_id = priv->locaddr4[i].loc4_id;
		opts->add_addr4.addr	= priv->locaddr4[i].addr;
		opts->add_addr_v4	= 1;

		if (skb)
			priv->announced4 |= (1 << i);
		*size += MPTCP_SUB_LEN_ADD_ADDR4_ALIGN;
	}

#if IS_ENABLED(CONFIG_IPV6)
	unannounced = (~priv->announced6) & priv->loc6_bits;
	if (unannounced &&
	    MAX_TCP_OPTION_SPACE - *size >= MPTCP_SUB_LEN_ADD_ADDR6_ALIGN) {
		int i = mptcp_nl_find_free_index(~unannounced);

		opts->options		|= OPTION_MPTCP;
		opts->mptcp_options	|= OPTION_ADD_ADDR;
		opts->add_addr6.addr_id = priv->locaddr6[i].loc6_id;
		opts->add_addr6.addr	= priv->locaddr6[i].addr;
		opts->add_addr_v6	= 1;

		if (skb)
			priv->announced6 |= (1 << i);
		*size += MPTCP_SUB_LEN_ADD_ADDR6_ALIGN;
	}
#endif

	if (likely(!priv->remove_addrs))
		goto exit;

	remove_addr_len = mptcp_sub_len_remove_addr_align(priv->remove_addrs);
	if (MAX_TCP_OPTION_SPACE - *size < remove_addr_len)
		goto exit;

	opts->options		|= OPTION_MPTCP;
	opts->mptcp_options	|= OPTION_REMOVE_ADDR;
	opts->remove_addrs	= priv->remove_addrs;

	if (skb)
		priv->remove_addrs = 0;
	*size += remove_addr_len;

exit:
	mpcb->addr_signal = !!((~priv->announced4) & priv->loc4_bits ||
#if IS_ENABLED(CONFIG_IPV6)
			       (~priv->announced6) & priv->loc6_bits ||
#endif
			       priv->remove_addrs);
}

static void
mptcp_nl_pm_prio_changed(struct sock *sk, int low_prio)
{
	struct sock *meta_sk = mptcp_meta_sk(sk);

	if (!mptcp_nl_must_notify(MPTCPF_EVENT_SUB_PRIORITY, meta_sk))
		return;

	mptcp_nl_mcast(tcp_sk(meta_sk)->mpcb, sk, MPTCP_EVENT_SUB_PRIORITY);
}

static int
mptcp_nl_genl_announce(struct sk_buff *skb, struct genl_info *info)
{
	struct sock		*meta_sk, *subsk;
	struct mptcp_cb		*mpcb;
	struct mptcp_nl_priv	*priv;
	u32			token;
	u8			addr_id, backup = 0;
	u16			family;
	int			i, ret = 0;
	union inet_addr		saddr;
	int			if_idx = 0;
	bool			useless; /* unused out parameter "low_prio" */

	if (!info->attrs[MPTCP_ATTR_TOKEN] || !info->attrs[MPTCP_ATTR_FAMILY] ||
	    !info->attrs[MPTCP_ATTR_LOC_ID])
		return -EINVAL;

	token	= nla_get_u32(info->attrs[MPTCP_ATTR_TOKEN]);
	meta_sk = mptcp_hash_find(genl_info_net(info), token);
	if (!meta_sk)
		return -EINVAL;

	mpcb	= tcp_sk(meta_sk)->mpcb;
	priv	= mptcp_nl_priv(meta_sk);
	family	= nla_get_u16(info->attrs[MPTCP_ATTR_FAMILY]);
	addr_id = nla_get_u8(info->attrs[MPTCP_ATTR_LOC_ID]);

	if (info->attrs[MPTCP_ATTR_BACKUP])
		backup = nla_get_u8(info->attrs[MPTCP_ATTR_BACKUP]);

	if (info->attrs[MPTCP_ATTR_IF_IDX])
		if_idx = nla_get_s32(info->attrs[MPTCP_ATTR_IF_IDX]);

	mutex_lock(&mpcb->mpcb_mutex);
	lock_sock_nested(meta_sk, SINGLE_DEPTH_NESTING);

	switch (family) {
	case AF_INET:
		if (!info->attrs[MPTCP_ATTR_SADDR4]) {
			ret = -EINVAL;
			goto exit;
		}

		saddr.in.s_addr = nla_get_u32(info->attrs[MPTCP_ATTR_SADDR4]);
		i		= mptcp_nl_pm_get_local_id(meta_sk, family,
							   &saddr, &useless);
		if (i < 0) {
			i = mptcp_nl_find_free_index(priv->loc4_bits);
			if (i < 0) {
				ret = -ENOBUFS;
				goto exit;
			}
		} else if (i != addr_id) {
			ret = -EINVAL;
			goto exit;
		}

		priv->locaddr4[i].addr.s_addr	= saddr.in.s_addr;
		priv->locaddr4[i].loc4_id	= addr_id;
		priv->locaddr4[i].low_prio	= !!backup;
		priv->locaddr4[i].if_idx	= if_idx;
		priv->loc4_bits			|= 1 << i;
		priv->announced4		&= ~(1 << i);
		break;
#if IS_ENABLED(CONFIG_IPV6)
	case AF_INET6:
		if (!info->attrs[MPTCP_ATTR_SADDR6]) {
			ret = -EINVAL;
			goto exit;
		}

		saddr.in6 = *(struct in6_addr *)
			    nla_data(info->attrs[MPTCP_ATTR_SADDR6]);
		i = mptcp_nl_pm_get_local_id(meta_sk, family, &saddr, &useless);
		if (i < 0) {
			i = mptcp_nl_find_free_index(priv->loc6_bits);
			if (i < 0) {
				ret = -ENOBUFS;
				goto exit;
			}
		} else if (i != addr_id) {
			ret = -EINVAL;
			goto exit;
		}

		priv->locaddr6[i].addr		= saddr.in6;
		priv->locaddr6[i].loc6_id	= addr_id;
		priv->locaddr6[i].low_prio	= !!backup;
		priv->locaddr6[i].if_idx	= if_idx;
		priv->loc6_bits			|= 1 << i;
		priv->announced6		&= ~(1 << i);
		break;
#endif
	default:
		ret = -EINVAL;
		goto exit;
	}

	mpcb->addr_signal = 1;

	rcu_read_lock_bh();
	subsk = mptcp_select_ack_sock(meta_sk);
	if (subsk)
		tcp_send_ack(subsk);
	rcu_read_unlock_bh();

exit:
	release_sock(meta_sk);
	mutex_unlock(&mpcb->mpcb_mutex);
	sock_put(meta_sk);
	return ret;
}

static int
mptcp_nl_genl_remove(struct sk_buff *skb, struct genl_info *info)
{
	struct sock		*meta_sk, *subsk;
	struct mptcp_cb		*mpcb;
	struct mptcp_nl_priv	*priv;
	u32			token;
	u8			addr_id;
	int			i;
	int			retcode;
	bool			found = false;

	if (!info->attrs[MPTCP_ATTR_TOKEN] || !info->attrs[MPTCP_ATTR_LOC_ID])
		return -EINVAL;

	token	= nla_get_u32(info->attrs[MPTCP_ATTR_TOKEN]);
	meta_sk = mptcp_hash_find(genl_info_net(info), token);
	if (!meta_sk)
		return -EINVAL;

	mpcb	= tcp_sk(meta_sk)->mpcb;
	priv	= mptcp_nl_priv(meta_sk);
	addr_id = nla_get_u8(info->attrs[MPTCP_ATTR_LOC_ID]);

	mutex_lock(&mpcb->mpcb_mutex);
	lock_sock_nested(meta_sk, SINGLE_DEPTH_NESTING);

	mptcp_for_each_bit_set(priv->loc4_bits, i) {
		if (priv->locaddr4[i].loc4_id == addr_id) {
			priv->loc4_bits &= ~(1 << i);
			found		= true;
			break;
		}
	}

#if IS_ENABLED(CONFIG_IPV6)
	if (!found) {
		mptcp_for_each_bit_set(priv->loc6_bits, i) {
			if (priv->locaddr6[i].loc6_id == addr_id) {
				priv->loc6_bits &= ~(1 << i);
				found		= true;
				break;
			}
		}
	}
#endif

	if (found) {
		priv->remove_addrs	|= 1 << addr_id;
		mpcb->addr_signal	= 1;

		rcu_read_lock_bh();
		subsk = mptcp_select_ack_sock(meta_sk);
		if (subsk)
			tcp_send_ack(subsk);
		rcu_read_unlock_bh();
		retcode = 0;
	} else {
		retcode = -EINVAL;
	}

	release_sock(meta_sk);
	mutex_unlock(&mpcb->mpcb_mutex);
	sock_put(meta_sk);
	return retcode;
}

static int
mptcp_nl_genl_create(struct sk_buff *skb, struct genl_info *info)
{
	struct sock		*meta_sk, *subsk = NULL;
	struct mptcp_cb		*mpcb;
	struct mptcp_nl_priv	*priv;
	u32			token;
	u16			family, sport;
	u8			loc_id, rem_id, backup = 0;
	int			i, ret = 0;
	int			if_idx;

	if (!info->attrs[MPTCP_ATTR_TOKEN] || !info->attrs[MPTCP_ATTR_FAMILY] ||
	    !info->attrs[MPTCP_ATTR_LOC_ID] || !info->attrs[MPTCP_ATTR_REM_ID])
		return -EINVAL;

	token	= nla_get_u32(info->attrs[MPTCP_ATTR_TOKEN]);
	meta_sk = mptcp_hash_find(genl_info_net(info), token);
	if (!meta_sk)
		/* We use a more specific value than EINVAL here so that
		 * userspace can handle this specific case easily. This is
		 * useful to check the case in which userspace tries to create a
		 * subflow for a connection which was already destroyed recently
		 * in kernelspace, but userspace didn't have time to realize
		 * about it because there is a gap of time between kernel
		 * destroying the connection and userspace receiving the event
		 * through Netlink. It can easily happen for short life-time
		 * conns.
		 */
		return -EBADR;

	mpcb = tcp_sk(meta_sk)->mpcb;

	mutex_lock(&mpcb->mpcb_mutex);
	lock_sock_nested(meta_sk, SINGLE_DEPTH_NESTING);

	if (sock_flag(meta_sk, SOCK_DEAD)) {
		/* Same as for the EBADR case. In this case, though, we know for
		 * sure the conn owner of the subflow existed at some point (no
		 * invalid token possibility)
		 */
		ret = -EOWNERDEAD;
		goto unlock;
	}

	if (!mptcp_can_new_subflow(meta_sk)) {
		/* Same as for the EBADR and EOWNERDEAD case but here, the MPTCP
		 * session has just been stopped, it is no longer possible to
		 * create new subflows.
		 */
		ret = -ENOTCONN;
		goto unlock;
	}

	if (mpcb->master_sk &&
	    !tcp_sk(mpcb->master_sk)->mptcp->fully_established) {
		/* First condition is not only in there for safely purposes, it
		 * can also be triggered in the same scenario as in EBADR and
		 * EOWNERDEAD
		 */
		ret = -EAGAIN;
		goto unlock;
	}

	priv = mptcp_nl_priv(meta_sk);

	family	= nla_get_u16(info->attrs[MPTCP_ATTR_FAMILY]);
	loc_id	= nla_get_u8(info->attrs[MPTCP_ATTR_LOC_ID]);
	rem_id	= nla_get_u8(info->attrs[MPTCP_ATTR_REM_ID]);

	sport = info->attrs[MPTCP_ATTR_SPORT]
		? htons(nla_get_u16(info->attrs[MPTCP_ATTR_SPORT])) : 0;
	backup = info->attrs[MPTCP_ATTR_BACKUP]
		 ? nla_get_u8(info->attrs[MPTCP_ATTR_BACKUP]) : 0;
	if_idx = info->attrs[MPTCP_ATTR_IF_IDX]
		 ? nla_get_s32(info->attrs[MPTCP_ATTR_IF_IDX]) : 0;

	switch (family) {
	case AF_INET: {
		struct mptcp_rem4	rem = {
			.rem4_id	= rem_id,
		};
		struct mptcp_loc4	loc = {
			.loc4_id	= loc_id,
		};

		if (!info->attrs[MPTCP_ATTR_DADDR4] ||
		    !info->attrs[MPTCP_ATTR_DPORT]) {
			goto create_failed;
		} else {
			rem.addr.s_addr =
				nla_get_u32(info->attrs[MPTCP_ATTR_DADDR4]);
			rem.port =
				ntohs(nla_get_u16(info->attrs[MPTCP_ATTR_DPORT]));
		}

		if (!info->attrs[MPTCP_ATTR_SADDR4]) {
			bool found = false;

			mptcp_for_each_bit_set(priv->loc4_bits, i) {
				if (priv->locaddr4[i].loc4_id == loc_id) {
					loc.addr	= priv->locaddr4[i].addr;
					loc.low_prio	=
						priv->locaddr4[i].low_prio;
					loc.if_idx =
						priv->locaddr4[i].if_idx;
					found = true;
					break;
				}
			}

			if (!found)
				goto create_failed;
		} else {
			loc.addr.s_addr =
				nla_get_u32(info->attrs[MPTCP_ATTR_SADDR4]);
			loc.low_prio	= backup;
			loc.if_idx	= if_idx;
		}

		ret = __mptcp_init4_subsockets(meta_sk, &loc, sport, &rem,
					       &subsk);
		if (ret < 0)
			goto unlock;
		break;
	}
#if IS_ENABLED(CONFIG_IPV6)
	case AF_INET6: {
		struct mptcp_rem6	rem = {
			.rem6_id	= rem_id,
		};
		struct mptcp_loc6	loc = {
			.loc6_id	= loc_id,
		};

		if (!info->attrs[MPTCP_ATTR_DADDR6] ||
		    !info->attrs[MPTCP_ATTR_DPORT]) {
			goto create_failed;
		} else {
			rem.addr = *(struct in6_addr *)
				   nla_data(info->attrs[MPTCP_ATTR_DADDR6]);
			rem.port =
				ntohs(nla_get_u16(info->attrs[MPTCP_ATTR_DPORT]));
		}

		if (!info->attrs[MPTCP_ATTR_SADDR6]) {
			bool found = false;

			mptcp_for_each_bit_set(priv->loc6_bits, i) {
				if (priv->locaddr6[i].loc6_id == loc_id) {
					loc.addr	= priv->locaddr6[i].addr;
					loc.low_prio	=
						priv->locaddr6[i].low_prio;
					loc.if_idx =
						priv->locaddr6[i].if_idx;

					found = true;
					break;
				}
			}

			if (!found)
				goto create_failed;
		} else {
			loc.addr = *(struct in6_addr *)
				nla_data(info->attrs[MPTCP_ATTR_SADDR6]);
			loc.low_prio	= backup;
			loc.if_idx	= if_idx;
		}

		ret = __mptcp_init6_subsockets(meta_sk, &loc, sport, &rem,
					       &subsk);
		if (ret < 0)
			goto unlock;
		break;
	}
#endif
	default:
		goto create_failed;
	}

unlock:
	release_sock(meta_sk);
	mutex_unlock(&mpcb->mpcb_mutex);
	sock_put(meta_sk);
	return ret;

create_failed:
	ret = -EINVAL;
	goto unlock;
}

static struct sock *
mptcp_nl_subsk_lookup(struct mptcp_cb *mpcb, struct nlattr **attrs)
{
	struct sock		*sk;
	struct mptcp_tcp_sock	*mptcp;
	struct hlist_node	*tmp;
	u16			family;
	__be16			sport, dport;

	if (!attrs[MPTCP_ATTR_FAMILY] || !attrs[MPTCP_ATTR_SPORT] ||
	    !attrs[MPTCP_ATTR_DPORT])
		goto exit;

	family	= nla_get_u16(attrs[MPTCP_ATTR_FAMILY]);
	sport	= htons(nla_get_u16(attrs[MPTCP_ATTR_SPORT]));
	dport	= htons(nla_get_u16(attrs[MPTCP_ATTR_DPORT]));

	switch (family) {
	case AF_INET: {
		__be32 saddr, daddr;

		if (!attrs[MPTCP_ATTR_SADDR4] || !attrs[MPTCP_ATTR_DADDR4])
			break;

		saddr	= nla_get_u32(attrs[MPTCP_ATTR_SADDR4]);
		daddr	= nla_get_u32(attrs[MPTCP_ATTR_DADDR4]);

		mptcp_for_each_sub_safe(mpcb, mptcp, tmp) {
			struct sock *subsk = mptcp_to_sock(mptcp);
			struct inet_sock *isk = inet_sk(subsk);

			if (subsk->sk_family != AF_INET)
				continue;

			if (isk->inet_saddr == saddr &&
			    isk->inet_daddr == daddr &&
			    isk->inet_sport == sport &&
			    isk->inet_dport == dport) {
				sk = subsk;
				goto found;
			}
		}
		break;
	}
#if IS_ENABLED(CONFIG_IPV6)
	case AF_INET6: {
		struct in6_addr saddr, daddr;

		if (!attrs[MPTCP_ATTR_SADDR6] || !attrs[MPTCP_ATTR_DADDR6])
			break;

		saddr	= *(struct in6_addr *)nla_data(attrs[MPTCP_ATTR_SADDR6]);
		daddr	= *(struct in6_addr *)nla_data(attrs[MPTCP_ATTR_DADDR6]);

		mptcp_for_each_sub_safe(mpcb, mptcp, tmp) {
			struct sock		*subsk = mptcp_to_sock(mptcp);
			struct inet_sock	*isk = inet_sk(subsk);
			struct ipv6_pinfo	*np;

			if (subsk->sk_family != AF_INET6)
				continue;

			np = inet6_sk(subsk);
			if (ipv6_addr_equal(&saddr, &np->saddr) &&
			    ipv6_addr_equal(&daddr, &subsk->sk_v6_daddr) &&
			    isk->inet_sport == sport &&
			    isk->inet_dport == dport) {
				sk = subsk;
				goto found;
			}
		}
		break;
	}
#endif
	}

exit:
	sk = NULL;
found:
	return sk;
}

static int
mptcp_nl_genl_destroy(struct sk_buff *skb, struct genl_info *info)
{
	struct sock	*meta_sk, *subsk;
	struct mptcp_cb	*mpcb;
	int		ret = 0;
	u32		token;

	if (!info->attrs[MPTCP_ATTR_TOKEN])
		return -EINVAL;

	token = nla_get_u32(info->attrs[MPTCP_ATTR_TOKEN]);

	meta_sk = mptcp_hash_find(genl_info_net(info), token);
	if (!meta_sk)
		return -EINVAL;

	mpcb = tcp_sk(meta_sk)->mpcb;

	mutex_lock(&mpcb->mpcb_mutex);
	lock_sock_nested(meta_sk, SINGLE_DEPTH_NESTING);

	subsk = mptcp_nl_subsk_lookup(mpcb, info->attrs);
	if (subsk) {
		local_bh_disable();
		mptcp_reinject_data(subsk, 0);
		mptcp_send_reset(subsk);
		local_bh_enable();
	} else {
		ret = -EINVAL;
	}

	release_sock(meta_sk);
	mutex_unlock(&mpcb->mpcb_mutex);
	sock_put(meta_sk);
	return ret;
}

static int
mptcp_nl_genl_conn_exists(struct sk_buff *skb, struct genl_info *info)
{
	struct sock	*meta_sk;
	u32		token;

	if (!info->attrs[MPTCP_ATTR_TOKEN])
		return -EINVAL;

	token = nla_get_u32(info->attrs[MPTCP_ATTR_TOKEN]);

	meta_sk = mptcp_hash_find(genl_info_net(info), token);
	if (!meta_sk)
		return -ENOTCONN;

	sock_put(meta_sk);
	return 0;
}

static int
mptcp_nl_genl_priority(struct sk_buff *skb, struct genl_info *info)
{
	struct sock	*meta_sk, *subsk;
	struct mptcp_cb	*mpcb;
	int		ret = 0;
	u32		token;
	u8		backup = 0;

	if (!info->attrs[MPTCP_ATTR_TOKEN])
		return -EINVAL;

	token = nla_get_u32(info->attrs[MPTCP_ATTR_TOKEN]);
	if (info->attrs[MPTCP_ATTR_BACKUP])
		backup = nla_get_u8(info->attrs[MPTCP_ATTR_BACKUP]);

	meta_sk = mptcp_hash_find(genl_info_net(info), token);
	if (!meta_sk)
		return -EINVAL;

	mpcb = tcp_sk(meta_sk)->mpcb;

	mutex_lock(&mpcb->mpcb_mutex);
	lock_sock_nested(meta_sk, SINGLE_DEPTH_NESTING);

	subsk = mptcp_nl_subsk_lookup(mpcb, info->attrs);
	if (subsk) {
		tcp_sk(subsk)->mptcp->send_mp_prio	= 1;
		tcp_sk(subsk)->mptcp->low_prio		= !!backup;

		local_bh_disable();
		if (mptcp_sk_can_send_ack(subsk))
			tcp_send_ack(subsk);
		else
			ret = -ENOTCONN;
		local_bh_enable();
	} else {
		ret = -EINVAL;
	}

	release_sock(meta_sk);
	mutex_unlock(&mpcb->mpcb_mutex);
	sock_put(meta_sk);
	return ret;
}

static int
mptcp_nl_genl_set_filter(struct sk_buff *skb, struct genl_info *info)
{
	u16 flags;

	if (!info->attrs[MPTCP_ATTR_FLAGS])
		return -EINVAL;

	flags = nla_get_u16(info->attrs[MPTCP_ATTR_FLAGS]);

	/* Only want to receive events that correspond to these flags */
	mptcp_nl_event_filter = ~flags;

	return 0;
}

static struct genl_ops mptcp_genl_ops[] = {
	{
		.cmd	= MPTCP_CMD_ANNOUNCE,
		.doit	= mptcp_nl_genl_announce,
		.policy = mptcp_nl_genl_policy,
		.flags	= GENL_ADMIN_PERM,
	},
	{
		.cmd	= MPTCP_CMD_REMOVE,
		.doit	= mptcp_nl_genl_remove,
		.policy = mptcp_nl_genl_policy,
		.flags	= GENL_ADMIN_PERM,
	},
	{
		.cmd	= MPTCP_CMD_SUB_CREATE,
		.doit	= mptcp_nl_genl_create,
		.policy = mptcp_nl_genl_policy,
		.flags	= GENL_ADMIN_PERM,
	},
	{
		.cmd	= MPTCP_CMD_SUB_DESTROY,
		.doit	= mptcp_nl_genl_destroy,
		.policy = mptcp_nl_genl_policy,
		.flags	= GENL_ADMIN_PERM,
	},
	{
		.cmd	= MPTCP_CMD_SUB_PRIORITY,
		.doit	= mptcp_nl_genl_priority,
		.policy = mptcp_nl_genl_policy,
		.flags	= GENL_ADMIN_PERM,
	},
	{
		.cmd	= MPTCP_CMD_SET_FILTER,
		.doit	= mptcp_nl_genl_set_filter,
		.policy = mptcp_nl_genl_policy,
		.flags	= GENL_ADMIN_PERM,
	},
	{
		.cmd	= MPTCP_CMD_EXIST,
		.doit	= mptcp_nl_genl_conn_exists,
		.policy = mptcp_nl_genl_policy,
		.flags	= GENL_ADMIN_PERM,
	},
};

static struct mptcp_pm_ops mptcp_nl_pm_ops = {
	.new_session		= mptcp_nl_pm_new_session,
	.close_session		= mptcp_nl_pm_close_session,
	.fully_established	= mptcp_nl_pm_fully_established,
	.established_subflow	= mptcp_nl_pm_established_subflow,
	.delete_subflow		= mptcp_nl_pm_delete_subflow,
	.add_raddr		= mptcp_nl_pm_add_raddr,
	.rem_raddr		= mptcp_nl_pm_rem_raddr,
	.get_local_id		= mptcp_nl_pm_get_local_id,
	.addr_signal		= mptcp_nl_pm_addr_signal,
	.prio_changed		= mptcp_nl_pm_prio_changed,
	.name			= "netlink",
	.owner			= THIS_MODULE,
};

static struct genl_family mptcp_genl_family = {
	.hdrsize	= 0,
	.name		= MPTCP_GENL_NAME,
	.version	= MPTCP_GENL_VER,
	.maxattr	= MPTCP_ATTR_MAX,
	.netnsok	= true,
	.module		= THIS_MODULE,
	.ops		= mptcp_genl_ops,
	.n_ops		= ARRAY_SIZE(mptcp_genl_ops),
	.mcgrps		= mptcp_mcgrps,
	.n_mcgrps	= ARRAY_SIZE(mptcp_mcgrps),
};

static int __init
mptcp_nl_init(void)
{
	int ret;

	BUILD_BUG_ON(sizeof(struct mptcp_nl_priv) > MPTCP_PM_SIZE);

	ret = genl_register_family(&mptcp_genl_family);
	if (ret)
		goto out_genl;

	ret = mptcp_register_path_manager(&mptcp_nl_pm_ops);
	if (ret)
		goto out_pm;

	return 0;
out_pm:
	genl_unregister_family(&mptcp_genl_family);
out_genl:
	return ret;
}

static void __exit
mptcp_nl_exit(void)
{
	mptcp_unregister_path_manager(&mptcp_nl_pm_ops);
	genl_unregister_family(&mptcp_genl_family);
}

module_init(mptcp_nl_init);
module_exit(mptcp_nl_exit);

MODULE_AUTHOR("Gregory Detal <gregory.detal@tessares.net>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MPTCP netlink-based path manager");
MODULE_ALIAS_GENL_FAMILY(MPTCP_GENL_NAME);
#endif
