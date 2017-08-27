/*
 * netlink/genl/family.h	Generic Netlink Family
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_GENL_FAMILY_H_
#define NETLINK_GENL_FAMILY_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>

#ifdef __cplusplus
extern "C" {
#endif

struct genl_family;

extern struct genl_family *	genl_family_alloc(void);
extern void			genl_family_put(struct genl_family *);

extern unsigned int		genl_family_get_id(struct genl_family *);
extern void			genl_family_set_id(struct genl_family *,
						   unsigned int);
extern char *			genl_family_get_name(struct genl_family *);
extern void			genl_family_set_name(struct genl_family *,
						     const char *name);
extern uint8_t			genl_family_get_version(struct genl_family *);
extern void			genl_family_set_version(struct genl_family *,
							uint8_t);
extern uint32_t			genl_family_get_hdrsize(struct genl_family *);
extern void			genl_family_set_hdrsize(struct genl_family *,
							uint32_t);
extern uint32_t			genl_family_get_maxattr(struct genl_family *);
extern void			genl_family_set_maxattr(struct genl_family *,
							uint32_t);

extern int			genl_family_add_op(struct genl_family *,
						   int, int);
extern int 			genl_family_add_grp(struct genl_family *,
					uint32_t , const char *);


#ifdef __cplusplus
}
#endif

#endif
