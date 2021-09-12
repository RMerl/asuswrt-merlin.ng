/*
 * netlink/idiag/vegasinfo.h		Inetdiag Netlink TCP Vegas Info
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Sassano Systems LLC <joe@sassanosystems.com>
 */

#ifndef NETLINK_IDIAGNL_VEGASINFO_H_
#define NETLINK_IDIAGNL_VEGASINFO_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern struct nl_object_ops	  idiagnl_vegasinfo_obj_ops;
extern struct idiagnl_vegasinfo * idiagnl_vegasinfo_alloc(void);
extern void	  idiagnl_vegasinfo_get(struct idiagnl_vegasinfo *);
extern void	  idiagnl_vegasinfo_put(struct idiagnl_vegasinfo *);

extern uint32_t idiagnl_vegasinfo_get_enabled(const struct idiagnl_vegasinfo *);
extern uint32_t	idiagnl_vegasinfo_get_rttcnt(const struct idiagnl_vegasinfo *);
extern uint32_t idiagnl_vegasinfo_get_rtt(const struct idiagnl_vegasinfo *);
extern uint32_t idiagnl_vegasinfo_get_minrtt(const struct idiagnl_vegasinfo *);

extern void	idiagnl_vegasinfo_set_enabled(struct idiagnl_vegasinfo *,
                                              uint32_t);
extern void	idiagnl_vegasinfo_set_rttcnt(struct idiagnl_vegasinfo *,
                                             uint32_t);
extern void	idiagnl_vegasinfo_set_rtt(struct idiagnl_vegasinfo *, uint32_t);
extern void	idiagnl_vegasinfo_set_minrtt(struct idiagnl_vegasinfo *,
                                             uint32_t);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NETLINK_IDIAGNL_VEGASINFO_H_ */
