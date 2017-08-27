/*
 * netlink/route/cls/ematch.h		Extended Matches
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2010 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_CLS_EMATCH_H_
#define NETLINK_CLS_EMATCH_H_

#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <netlink/route/classifier.h>
#include <linux/pkt_cls.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FIXME: Should be moved to the kernel header at some point */
#define RTNL_EMATCH_PROGID	2

struct rtnl_ematch;
struct rtnl_ematch_tree;

/**
 * Extended Match Operations
 */
struct rtnl_ematch_ops
{
	int			eo_kind;
	const char *		eo_name;
	size_t			eo_minlen;
	size_t			eo_datalen;

	int		      (*eo_parse)(struct rtnl_ematch *, void *, size_t);
	void		      (*eo_dump)(struct rtnl_ematch *,
					 struct nl_dump_params *);
	int		      (*eo_fill)(struct rtnl_ematch *, struct nl_msg *);
	void		      (*eo_free)(struct rtnl_ematch *);
	struct nl_list_head	eo_list;
};

extern int			rtnl_ematch_register(struct rtnl_ematch_ops *);
extern struct rtnl_ematch_ops *	rtnl_ematch_lookup_ops(int);
extern struct rtnl_ematch_ops *	rtnl_ematch_lookup_ops_by_name(const char *);

extern struct rtnl_ematch *	rtnl_ematch_alloc(void);
extern int			rtnl_ematch_add_child(struct rtnl_ematch *,
						      struct rtnl_ematch *);
extern void			rtnl_ematch_unlink(struct rtnl_ematch *);
extern void			rtnl_ematch_free(struct rtnl_ematch *);

extern void *			rtnl_ematch_data(struct rtnl_ematch *);
extern void			rtnl_ematch_set_flags(struct rtnl_ematch *,
						      uint16_t);
extern void			rtnl_ematch_unset_flags(struct rtnl_ematch *,
							uint16_t);
extern uint16_t			rtnl_ematch_get_flags(struct rtnl_ematch *);
extern int			rtnl_ematch_set_ops(struct rtnl_ematch *,
						    struct rtnl_ematch_ops *);
extern int			rtnl_ematch_set_kind(struct rtnl_ematch *,
						     uint16_t);
extern int			rtnl_ematch_set_name(struct rtnl_ematch *,
						     const char *);

extern struct rtnl_ematch_tree *rtnl_ematch_tree_alloc(uint16_t);
extern void			rtnl_ematch_tree_free(struct rtnl_ematch_tree *);
extern void			rtnl_ematch_tree_add(struct rtnl_ematch_tree *,
						     struct rtnl_ematch *);

extern int			rtnl_ematch_parse_attr(struct nlattr *,
						       struct rtnl_ematch_tree **);
extern int			rtnl_ematch_fill_attr(struct nl_msg *, int,
						      struct rtnl_ematch_tree *);
extern void			rtnl_ematch_tree_dump(struct rtnl_ematch_tree *,
						      struct nl_dump_params *);


extern int			rtnl_ematch_parse_expr(const char *, char **,
						       struct rtnl_ematch_tree **);

extern char *			rtnl_ematch_offset2txt(uint8_t, uint16_t,
						       char *, size_t);
extern char *			rtnl_ematch_opnd2txt(uint8_t, char *, size_t);

#ifdef __cplusplus
}
#endif

#endif
