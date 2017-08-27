/*
 * netlink/cli/ct.h	CLI Conntrack Helper
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2009 Thomas Graf <tgraf@suug.ch>
 */

#ifndef __NETLINK_CLI_CT_H_
#define __NETLINK_CLI_CT_H_

#include <netlink/netfilter/ct.h>
#include <linux/netfilter/nf_conntrack_common.h>

extern struct nfnl_ct *nl_cli_ct_alloc(void);
extern struct nl_cache *nl_cli_ct_alloc_cache(struct nl_sock *);

extern void nl_cli_ct_parse_family(struct nfnl_ct *, char *);
extern void nl_cli_ct_parse_protocol(struct nfnl_ct *, char *);
extern void nl_cli_ct_parse_mark(struct nfnl_ct *, char *);
extern void nl_cli_ct_parse_timeout(struct nfnl_ct *, char *);
extern void nl_cli_ct_parse_id(struct nfnl_ct *, char *);
extern void nl_cli_ct_parse_use(struct nfnl_ct *, char *);
extern void nl_cli_ct_parse_src(struct nfnl_ct *, int, char *);
extern void nl_cli_ct_parse_dst(struct nfnl_ct *, int, char *);
extern void nl_cli_ct_parse_src_port(struct nfnl_ct *, int, char *);
extern void nl_cli_ct_parse_dst_port(struct nfnl_ct *, int, char *);
extern void nl_cli_ct_parse_tcp_state(struct nfnl_ct *, char *);
extern void nl_cli_ct_parse_status(struct nfnl_ct *, char *);

#endif
