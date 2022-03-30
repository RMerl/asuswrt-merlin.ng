/*
 * netlink/cache.h		Caching Module
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_CACHE_H_
#define NETLINK_CACHE_H_

#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <netlink/utils.h>
#include <netlink/object.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	NL_ACT_UNSPEC,
	NL_ACT_NEW,
	NL_ACT_DEL,
	NL_ACT_GET,
	NL_ACT_SET,
	NL_ACT_CHANGE,
	__NL_ACT_MAX,
};

#define NL_ACT_MAX (__NL_ACT_MAX - 1)

struct nl_cache;
typedef void (*change_func_t)(struct nl_cache *, struct nl_object *, int, void *);

/**
 * @ingroup cache
 * Explicitely iterate over all address families when updating the cache
 */
#define NL_CACHE_AF_ITER	0x0001

/* Access Functions */
extern int			nl_cache_nitems(struct nl_cache *);
extern int			nl_cache_nitems_filter(struct nl_cache *,
						       struct nl_object *);
extern struct nl_cache_ops *	nl_cache_get_ops(struct nl_cache *);
extern struct nl_object *	nl_cache_get_first(struct nl_cache *);
extern struct nl_object *	nl_cache_get_last(struct nl_cache *);
extern struct nl_object *	nl_cache_get_next(struct nl_object *);
extern struct nl_object *	nl_cache_get_prev(struct nl_object *);

extern struct nl_cache *	nl_cache_alloc(struct nl_cache_ops *);
extern int			nl_cache_alloc_and_fill(struct nl_cache_ops *,
							struct nl_sock *,
							struct nl_cache **);
extern int			nl_cache_alloc_name(const char *,
						    struct nl_cache **);
extern struct nl_cache *	nl_cache_subset(struct nl_cache *,
						struct nl_object *);
extern struct nl_cache *	nl_cache_clone(struct nl_cache *);
extern void			nl_cache_clear(struct nl_cache *);
extern void			nl_cache_get(struct nl_cache *);
extern void			nl_cache_free(struct nl_cache *);
extern void			nl_cache_put(struct nl_cache *cache);

/* Cache modification */
extern int			nl_cache_add(struct nl_cache *,
					     struct nl_object *);
extern int			nl_cache_parse_and_add(struct nl_cache *,
						       struct nl_msg *);
extern int			nl_cache_move(struct nl_cache *,
					      struct nl_object *);
extern void			nl_cache_remove(struct nl_object *);
extern int			nl_cache_refill(struct nl_sock *,
						struct nl_cache *);
extern int			nl_cache_pickup(struct nl_sock *,
						struct nl_cache *);
extern int			nl_cache_resync(struct nl_sock *,
						struct nl_cache *,
						change_func_t,
						void *);
extern int			nl_cache_include(struct nl_cache *,
						 struct nl_object *,
						 change_func_t,
						 void *);
extern void			nl_cache_set_arg1(struct nl_cache *, int);
extern void			nl_cache_set_arg2(struct nl_cache *, int);
extern void			nl_cache_set_flags(struct nl_cache *, unsigned int);

/* General */
extern int			nl_cache_is_empty(struct nl_cache *);
extern struct nl_object *	nl_cache_search(struct nl_cache *,
						struct nl_object *);
extern struct nl_object *nl_cache_find(struct nl_cache *,
				       struct nl_object *);
extern void			nl_cache_mark_all(struct nl_cache *);

/* Dumping */
extern void			nl_cache_dump(struct nl_cache *,
					      struct nl_dump_params *);
extern void			nl_cache_dump_filter(struct nl_cache *,
						     struct nl_dump_params *,
						     struct nl_object *);

/* Iterators */
extern void			nl_cache_foreach(struct nl_cache *,
						 void (*cb)(struct nl_object *,
							    void *),
						 void *arg);
extern void			nl_cache_foreach_filter(struct nl_cache *,
							struct nl_object *,
							void (*cb)(struct
								   nl_object *,
								   void *),
							void *arg);

/* --- cache management --- */

/* Cache type management */
extern struct nl_cache_ops *	nl_cache_ops_lookup(const char *);
extern struct nl_cache_ops *	nl_cache_ops_lookup_safe(const char *);
extern struct nl_cache_ops *	nl_cache_ops_associate(int, int);
extern struct nl_cache_ops *	nl_cache_ops_associate_safe(int, int);
extern struct nl_msgtype *	nl_msgtype_lookup(struct nl_cache_ops *, int);
extern void			nl_cache_ops_foreach(void (*cb)(struct nl_cache_ops *, void *), void *);
extern int			nl_cache_mngt_register(struct nl_cache_ops *);
extern int			nl_cache_mngt_unregister(struct nl_cache_ops *);

/* Global cache provisioning/requiring */
extern void			nl_cache_mngt_provide(struct nl_cache *);
extern void			nl_cache_mngt_unprovide(struct nl_cache *);
extern struct nl_cache *	nl_cache_mngt_require(const char *);
extern struct nl_cache *	nl_cache_mngt_require_safe(const char *);
extern struct nl_cache *	__nl_cache_mngt_require(const char *);

struct nl_cache_mngr;

#define NL_AUTO_PROVIDE		1
#define NL_ALLOCATED_SOCK	2  /* For internal use only, do not use */

extern int			nl_cache_mngr_alloc(struct nl_sock *,
						    int, int,
						    struct nl_cache_mngr **);
extern int			nl_cache_mngr_add(struct nl_cache_mngr *,
						  const char *,
						  change_func_t,
						  void *,
						  struct nl_cache **);
extern int			nl_cache_mngr_add_cache(struct nl_cache_mngr *mngr,
							struct nl_cache *cache,
							change_func_t cb, void *data);
extern int			nl_cache_mngr_get_fd(struct nl_cache_mngr *);
extern int			nl_cache_mngr_poll(struct nl_cache_mngr *,
						   int);
extern int			nl_cache_mngr_data_ready(struct nl_cache_mngr *);
extern void			nl_cache_mngr_info(struct nl_cache_mngr *,
						   struct nl_dump_params *);
extern void			nl_cache_mngr_free(struct nl_cache_mngr *);

extern void			nl_cache_ops_get(struct nl_cache_ops *);
extern void			nl_cache_ops_put(struct nl_cache_ops *);

#ifdef __cplusplus
}
#endif

#endif
