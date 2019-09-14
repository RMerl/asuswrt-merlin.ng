/*
 * netlink/object.c	Generic Cacheable Object
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_OBJECT_H_
#define NETLINK_OBJECT_H_

#include <netlink/netlink.h>
#include <netlink/utils.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nl_cache;
struct nl_object;
struct nl_object_ops;

#define OBJ_CAST(ptr)		((struct nl_object *) (ptr))

/* General */
extern struct nl_object *	nl_object_alloc(struct nl_object_ops *);
extern int			nl_object_alloc_name(const char *,
						     struct nl_object **);
extern void			nl_object_free(struct nl_object *);
extern struct nl_object *	nl_object_clone(struct nl_object *obj);
extern int			nl_object_update(struct nl_object *dst,
						 struct nl_object *src);
extern void			nl_object_get(struct nl_object *);
extern void			nl_object_put(struct nl_object *);
extern int			nl_object_shared(struct nl_object *);
extern void			nl_object_dump(struct nl_object *,
					       struct nl_dump_params *);
extern void			nl_object_dump_buf(struct nl_object *, char *, size_t);
extern int			nl_object_identical(struct nl_object *,
						    struct nl_object *);
extern uint32_t			nl_object_diff(struct nl_object *,
					       struct nl_object *);
extern int			nl_object_match_filter(struct nl_object *,
						       struct nl_object *);
extern char *			nl_object_attrs2str(struct nl_object *,
						    uint32_t attrs, char *buf,
						    size_t);
extern char *			nl_object_attr_list(struct nl_object *,
						    char *, size_t);
extern void			nl_object_keygen(struct nl_object *,
						 uint32_t *, uint32_t);

/* Marks */
extern void			nl_object_mark(struct nl_object *);
extern void			nl_object_unmark(struct nl_object *);
extern int			nl_object_is_marked(struct nl_object *);

/* Access Functions */
extern int			nl_object_get_refcnt(struct nl_object *);
extern struct nl_cache *	nl_object_get_cache(struct nl_object *);
extern const char *		nl_object_get_type(const struct nl_object *);
extern int			nl_object_get_msgtype(const struct nl_object *);
struct nl_object_ops *		nl_object_get_ops(const struct nl_object *);
uint32_t			nl_object_get_id_attrs(struct nl_object *obj);


static inline void *		nl_object_priv(struct nl_object *obj)
{
	return obj;
}


#ifdef __cplusplus
}
#endif

#endif
