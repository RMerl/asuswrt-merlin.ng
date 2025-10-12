/*
 * netlink/idiag/req.h		Inetdiag Netlink Request
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Sassano Systems LLC <joe@sassanosystems.com>
 */

#ifndef NETLINK_IDIAGNL_REQ_H_
#define NETLINK_IDIAGNL_REQ_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct idiagnl_req;
extern struct nl_object_ops	idiagnl_req_obj_ops;

extern struct idiagnl_req * idiagnl_req_alloc(void);
extern void		    idiagnl_req_get(struct idiagnl_req *);
extern void		    idiagnl_req_put(struct idiagnl_req *);
extern uint8_t		    idiagnl_req_get_family(const struct idiagnl_req *);
extern void		    idiagnl_req_set_family(struct idiagnl_req *,
                                                   uint8_t);
extern uint8_t		    idiagnl_req_get_ext(const struct idiagnl_req *);
extern void		    idiagnl_req_set_ext(struct idiagnl_req *, uint8_t);
extern uint32_t		    idiagnl_req_get_ifindex(const struct idiagnl_req *);
extern void		    idiagnl_req_set_ifindex(struct idiagnl_req *,
                                                    uint32_t);
extern uint32_t		    idiagnl_req_get_states(const struct idiagnl_req *);
extern void		    idiagnl_req_set_states(struct idiagnl_req *,
                                                   uint32_t);
extern uint32_t		    idiagnl_req_get_dbs(const struct idiagnl_req *);
extern void		    idiagnl_req_set_dbs(struct idiagnl_req *, uint32_t);
extern struct nl_addr *	    idiagnl_req_get_src(const struct idiagnl_req *);
extern int		    idiagnl_req_set_src(struct idiagnl_req *,
                                                struct nl_addr *);
extern struct nl_addr *	    idiagnl_req_get_dst(const struct idiagnl_req *);
extern int		    idiagnl_req_set_dst(struct idiagnl_req *,
                                                struct nl_addr *);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NETLINK_IDIAGNL_REQ_H_ */
