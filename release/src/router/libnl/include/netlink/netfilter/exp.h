/*
 * netlink/netfilter/exp.h   Conntrack Expectation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation version 2.1
 *  of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2007 Philip Craig <philipc@snapgear.com>
 * Copyright (c) 2007 Secure Computing Corporation
 * Copyright (c) 2012 Rich Fought <rich.fought@watchguard.com>
 */

#ifndef NETLINK_EXP_H_
#define NETLINK_EXP_H_

#include <netlink/netlink.h>
#include <netlink/addr.h>
#include <netlink/cache.h>
#include <netlink/msg.h>

#include <linux/version.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nfnl_exp;

enum nfnl_exp_tuples {
	NFNL_EXP_TUPLE_EXPECT,
	NFNL_EXP_TUPLE_MASTER,
	NFNL_EXP_TUPLE_MASK,
	NFNL_EXP_TUPLE_NAT,
	NFNL_EXP_TUPLE_MAX
};

extern struct nl_object_ops exp_obj_ops;

extern struct nfnl_exp * nfnl_exp_alloc(void);
extern int  nfnl_exp_alloc_cache(struct nl_sock *, struct nl_cache **);

extern int  nfnlmsg_exp_group(struct nlmsghdr *);
extern int  nfnlmsg_exp_parse(struct nlmsghdr *, struct nfnl_exp **);

extern void nfnl_exp_get(struct nfnl_exp *);
extern void nfnl_exp_put(struct nfnl_exp *);

extern int  nfnl_exp_dump_request(struct nl_sock *);

extern int  nfnl_exp_build_add_request(const struct nfnl_exp *, int,
						struct nl_msg **);
extern int  nfnl_exp_add(struct nl_sock *, const struct nfnl_exp *, int);

extern int  nfnl_exp_build_delete_request(const struct nfnl_exp *, int,
						struct nl_msg **);
extern int  nfnl_exp_del(struct nl_sock *, const struct nfnl_exp *, int);

extern int  nfnl_exp_build_query_request(const struct nfnl_exp *, int,
						struct nl_msg **);
extern int  nfnl_exp_query(struct nl_sock *, const struct nfnl_exp *, int);

extern void nfnl_exp_set_family(struct nfnl_exp *, uint8_t);
extern uint8_t  nfnl_exp_get_family(const struct nfnl_exp *);

extern void nfnl_exp_set_timeout(struct nfnl_exp *, uint32_t);
extern int  nfnl_exp_test_timeout(const struct nfnl_exp *);
extern uint32_t nfnl_exp_get_timeout(const struct nfnl_exp *);

extern void nfnl_exp_set_id(struct nfnl_exp *, uint32_t);
extern int  nfnl_exp_test_id(const struct nfnl_exp *);
extern uint32_t nfnl_exp_get_id(const struct nfnl_exp *);

extern int  nfnl_exp_set_helper_name(struct nfnl_exp *, void *);
extern int  nfnl_exp_test_helper_name(const struct nfnl_exp *);
extern const char * nfnl_exp_get_helper_name(const struct nfnl_exp *);

extern void nfnl_exp_set_zone(struct nfnl_exp *, uint16_t);
extern int  nfnl_exp_test_zone(const struct nfnl_exp *);
extern uint16_t nfnl_exp_get_zone(const struct nfnl_exp *);

extern void nfnl_exp_set_flags(struct nfnl_exp *, uint32_t);
extern int  nfnl_exp_test_flags(const struct nfnl_exp *);
extern uint32_t nfnl_exp_get_flags(const struct nfnl_exp *);

extern void nfnl_exp_set_class(struct nfnl_exp *, uint32_t);
extern int  nfnl_exp_test_class(const struct nfnl_exp *);
extern uint32_t nfnl_exp_get_class(const struct nfnl_exp *);

extern int  nfnl_exp_set_fn(struct nfnl_exp *, void *);
extern int  nfnl_exp_test_fn(const struct nfnl_exp *);
extern const char * nfnl_exp_get_fn(const struct nfnl_exp *);

extern void nfnl_exp_set_nat_dir(struct nfnl_exp *, uint8_t);
extern int  nfnl_exp_test_nat_dir(const struct nfnl_exp *);
extern uint8_t nfnl_exp_get_nat_dir(const struct nfnl_exp *);

// The int argument specifies which nfnl_exp_dir (expect, master, mask or nat)
// Expectation objects only use orig, not reply

extern int  nfnl_exp_set_src(struct nfnl_exp *, int, struct nl_addr *);
extern int  nfnl_exp_test_src(const struct nfnl_exp *, int);
extern struct nl_addr * nfnl_exp_get_src(const struct nfnl_exp *, int);

extern int  nfnl_exp_set_dst(struct nfnl_exp *, int, struct nl_addr *);
extern int  nfnl_exp_test_dst(const struct nfnl_exp *, int);
extern struct nl_addr * nfnl_exp_get_dst(const struct nfnl_exp *, int);

extern void  nfnl_exp_set_l4protonum(struct nfnl_exp *, int, uint8_t);
extern int  nfnl_exp_test_l4protonum(const struct nfnl_exp *, int);
extern uint8_t nfnl_exp_get_l4protonum(const struct nfnl_exp *, int);

extern void nfnl_exp_set_ports(struct nfnl_exp *, int, uint16_t, uint16_t);
extern int nfnl_exp_test_ports(const struct nfnl_exp *, int);
extern uint16_t nfnl_exp_get_src_port(const struct nfnl_exp *, int);
extern uint16_t nfnl_exp_get_dst_port(const struct nfnl_exp *, int);

extern void nfnl_exp_set_icmp(struct nfnl_exp *, int, uint16_t, uint8_t, uint8_t);
extern int nfnl_exp_test_icmp(const struct nfnl_exp *, int);
extern uint16_t nfnl_exp_get_icmp_id(const struct nfnl_exp *, int);
extern uint8_t  nfnl_exp_get_icmp_type(const struct nfnl_exp *, int);
extern uint8_t  nfnl_exp_get_icmp_code(const struct nfnl_exp *, int);

#ifdef __cplusplus
}
#endif

#endif
