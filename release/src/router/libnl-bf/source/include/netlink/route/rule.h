/*
 * netlink/route/rule.h		Rules
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2010 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_RULE_H_
#define NETLINK_RULE_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/addr.h>
#include <netlink/route/route.h>
#include <linux/fib_rules.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rtnl_rule;

/* General */
extern struct rtnl_rule *	rtnl_rule_alloc(void);
extern void			rtnl_rule_put(struct rtnl_rule *);

extern int	rtnl_rule_alloc_cache(struct nl_sock *, int,
				      struct nl_cache **);
extern void rtnl_rule_dump(struct rtnl_rule *, FILE *, struct nl_dump_params *);

extern int	rtnl_rule_build_add_request(struct rtnl_rule *, int,
					    struct nl_msg **);
extern int rtnl_rule_add(struct nl_sock *, struct rtnl_rule *, int);
extern int	rtnl_rule_build_delete_request(struct rtnl_rule *, int,
					       struct nl_msg **);
extern int rtnl_rule_delete(struct nl_sock *, struct rtnl_rule *, int);


/* attribute modification */
extern void		rtnl_rule_set_family(struct rtnl_rule *, int);
extern int		rtnl_rule_get_family(struct rtnl_rule *);
extern void		rtnl_rule_set_prio(struct rtnl_rule *, uint32_t);
extern uint32_t		rtnl_rule_get_prio(struct rtnl_rule *);
extern void		rtnl_rule_set_mark(struct rtnl_rule *, uint32_t);
extern uint32_t		rtnl_rule_get_mark(struct rtnl_rule *);
extern void		rtnl_rule_set_mask(struct rtnl_rule *, uint32_t);
extern uint32_t		rtnl_rule_get_mask(struct rtnl_rule *);
extern void		rtnl_rule_set_table(struct rtnl_rule *, uint32_t);
extern uint32_t		rtnl_rule_get_table(struct rtnl_rule *);
extern void		rtnl_rule_set_dsfield(struct rtnl_rule *, uint8_t);
extern uint8_t		rtnl_rule_get_dsfield(struct rtnl_rule *);
extern int		rtnl_rule_set_src(struct rtnl_rule *, struct nl_addr *);
extern struct nl_addr *	rtnl_rule_get_src(struct rtnl_rule *);
extern int		rtnl_rule_set_dst(struct rtnl_rule *, struct nl_addr *);
extern struct nl_addr *	rtnl_rule_get_dst(struct rtnl_rule *);
extern void		rtnl_rule_set_action(struct rtnl_rule *, uint8_t);
extern uint8_t		rtnl_rule_get_action(struct rtnl_rule *);
extern int		rtnl_rule_set_iif(struct rtnl_rule *, const char *);
extern char *		rtnl_rule_get_iif(struct rtnl_rule *);
extern int		rtnl_rule_set_oif(struct rtnl_rule *, const char *);
extern char *		rtnl_rule_get_oif(struct rtnl_rule *);
extern void		rtnl_rule_set_realms(struct rtnl_rule *, uint32_t);
extern uint32_t		rtnl_rule_get_realms(struct rtnl_rule *);
extern void		rtnl_rule_set_goto(struct rtnl_rule *, uint32_t);
extern uint32_t		rtnl_rule_get_goto(struct rtnl_rule *);

#ifdef __cplusplus
}
#endif

#endif
