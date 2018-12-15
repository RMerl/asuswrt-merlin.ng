/*
 * netlink/idiag/meminfo.h		Inetdiag Netlink Memory Info
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Sassano Systems LLC <joe@sassanosystems.com>
 */

#ifndef NETLINK_IDIAGNL_MEMINFO_H_
#define NETLINK_IDIAGNL_MEMINFO_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern struct nl_object_ops	idiagnl_meminfo_obj_ops;

extern struct idiagnl_meminfo *idiagnl_meminfo_alloc(void);
extern void idiagnl_meminfo_get(struct idiagnl_meminfo *);
extern void idiagnl_meminfo_put(struct idiagnl_meminfo *);

extern uint32_t	  idiagnl_meminfo_get_rmem(const struct idiagnl_meminfo *);
extern uint32_t   idiagnl_meminfo_get_wmem(const struct idiagnl_meminfo *);
extern uint32_t   idiagnl_meminfo_get_fmem(const struct idiagnl_meminfo *);
extern uint32_t   idiagnl_meminfo_get_tmem(const struct idiagnl_meminfo *);

extern void	  idiagnl_meminfo_set_rmem(struct idiagnl_meminfo *, uint32_t);
extern void	  idiagnl_meminfo_set_wmem(struct idiagnl_meminfo *, uint32_t);
extern void	  idiagnl_meminfo_set_fmem(struct idiagnl_meminfo *, uint32_t);
extern void	  idiagnl_meminfo_set_tmem(struct idiagnl_meminfo *, uint32_t);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NETLINK_IDIAGNL_MEMINFO_H_ */
