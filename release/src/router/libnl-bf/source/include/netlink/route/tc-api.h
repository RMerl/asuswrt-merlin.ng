/*
 * netlink/route/tc-api.h	Traffic Control API
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2011 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_TC_API_H_
#define NETLINK_TC_API_H_

#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <netlink/route/tc.h>

#ifdef __cplusplus
extern "C" {
#endif

enum rtnl_tc_type {
	RTNL_TC_TYPE_QDISC,
	RTNL_TC_TYPE_CLASS,
	RTNL_TC_TYPE_CLS,
	__RTNL_TC_TYPE_MAX,
};

#define RTNL_TC_TYPE_MAX (__RTNL_TC_TYPE_MAX - 1)

/**
 * Traffic control object operations
 * @ingroup tc
 *
 * This structure holds function pointers and settings implementing
 * the features of each traffic control object implementation.
 */
struct rtnl_tc_ops
{
	/**
	 * Name of traffic control module
	 */
	char *to_kind;

	/**
	 * Type of traffic control object
	 */
	enum rtnl_tc_type to_type;


	/**
	 * Size of private data
	 */
	size_t to_size;

	/**
	 * Dump callbacks
	 */
	void (*to_dump[NL_DUMP_MAX+1])(struct rtnl_tc *, void *,
				       struct nl_dump_params *);
	/**
	 * Used to fill the contents of TCA_OPTIONS
	 */
	int (*to_msg_fill)(struct rtnl_tc *, void *, struct nl_msg *);

	/**
	 * Uesd to to fill tc related messages, unlike with to_msg_fill,
	 * the contents is not encapsulated with a TCA_OPTIONS nested
	 * attribute.
	 */
	int (*to_msg_fill_raw)(struct rtnl_tc *, void *, struct nl_msg *);

	/**
	 * TCA_OPTIONS message parser
	 */
	int (*to_msg_parser)(struct rtnl_tc *, void *);

	/**
	 * Called before a tc object is destroyed
	 */
	void (*to_free_data)(struct rtnl_tc *, void *);

	/**
	 * Called whenever a classifier object needs to be cloned
	 */
	int (*to_clone)(void *, void *);

	/**
	 * Internal, don't touch
	 */
	struct nl_list_head to_list;
};

struct rtnl_tc_type_ops
{
	enum rtnl_tc_type tt_type;

	char *tt_dump_prefix;

	/**
	 * Dump callbacks
	 */
	void (*tt_dump[NL_DUMP_MAX+1])(struct rtnl_tc *,
				        struct nl_dump_params *);
};

extern int			rtnl_tc_msg_parse(struct nlmsghdr *,
						  struct rtnl_tc *);
extern int			rtnl_tc_msg_build(struct rtnl_tc *, int,
						  int, struct nl_msg **);

extern void			rtnl_tc_free_data(struct nl_object *);
extern int			rtnl_tc_clone(struct nl_object *,
					      struct nl_object *);
extern void			rtnl_tc_dump_line(struct nl_object *,
						  struct nl_dump_params *);
extern void			rtnl_tc_dump_details(struct nl_object *,
						     struct nl_dump_params *);
extern void			rtnl_tc_dump_stats(struct nl_object *,
						   struct nl_dump_params *);
extern int			rtnl_tc_compare(struct nl_object *,
						struct nl_object *,
						uint32_t, int);

extern void *			rtnl_tc_data(struct rtnl_tc *);
extern void *			rtnl_tc_data_check(struct rtnl_tc *,
						   struct rtnl_tc_ops *);

extern struct rtnl_tc_ops *	rtnl_tc_lookup_ops(enum rtnl_tc_type,
						   const char *);
extern struct rtnl_tc_ops *	rtnl_tc_get_ops(struct rtnl_tc *);
extern int 			rtnl_tc_register(struct rtnl_tc_ops *);
extern void 			rtnl_tc_unregister(struct rtnl_tc_ops *);

extern void			rtnl_tc_type_register(struct rtnl_tc_type_ops *);
extern void			rtnl_tc_type_unregister(struct rtnl_tc_type_ops *);

#ifdef __cplusplus
}
#endif

#endif
