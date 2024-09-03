/*
 * netlink/idiag/msg.h		Inetdiag Netlink Message
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Sassano Systems LLC <joe@sassanosystems.com>
 */

#ifndef NETLINK_IDIAGNL_MSG_H_
#define NETLINK_IDIAGNL_MSG_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct idiagnl_msg;
extern struct nl_object_ops  idiagnl_msg_obj_ops;

extern struct idiagnl_msg * idiagnl_msg_alloc(void);
extern int		idiagnl_msg_alloc_cache(struct nl_sock *, int, int,
                                                struct nl_cache**);
extern void		idiagnl_msg_get(struct idiagnl_msg *);
extern void		idiagnl_msg_put(struct idiagnl_msg *);
extern uint8_t		idiagnl_msg_get_family(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_family(struct idiagnl_msg *, uint8_t);
extern uint8_t		idiagnl_msg_get_state(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_state(struct idiagnl_msg *, uint8_t);
extern uint8_t		idiagnl_msg_get_timer(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_timer(struct idiagnl_msg *, uint8_t);
extern uint8_t		idiagnl_msg_get_retrans(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_retrans(struct idiagnl_msg *, uint8_t);
extern uint16_t		idiagnl_msg_get_sport(struct idiagnl_msg *);
extern void		idiagnl_msg_set_sport(struct idiagnl_msg *, uint16_t);
extern uint16_t		idiagnl_msg_get_dport(struct idiagnl_msg *);
extern void		idiagnl_msg_set_dport(struct idiagnl_msg *, uint16_t);
extern struct nl_addr *	idiagnl_msg_get_src(const struct idiagnl_msg *);
extern int		idiagnl_msg_set_src(struct idiagnl_msg *,
                                            struct nl_addr *);
extern struct nl_addr *	idiagnl_msg_get_dst(const struct idiagnl_msg *);
extern int		idiagnl_msg_set_dst(struct idiagnl_msg *,
		                            struct nl_addr *);
extern uint32_t		idiagnl_msg_get_ifindex(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_ifindex(struct idiagnl_msg *, uint32_t);
extern uint32_t		idiagnl_msg_get_expires(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_expires(struct idiagnl_msg *, uint32_t);
extern uint32_t		idiagnl_msg_get_rqueue(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_rqueue(struct idiagnl_msg *, uint32_t);
extern uint32_t		idiagnl_msg_get_wqueue(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_wqueue(struct idiagnl_msg *, uint32_t);
extern uint32_t		idiagnl_msg_get_uid(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_uid(struct idiagnl_msg *, uint32_t);
extern uint32_t		idiagnl_msg_get_inode(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_inode(struct idiagnl_msg *, uint32_t);
extern uint8_t		idiagnl_msg_get_tos(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_tos(struct idiagnl_msg *, uint8_t);
extern uint8_t		idiagnl_msg_get_tclass(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_tclass(struct idiagnl_msg *, uint8_t);
extern uint8_t		idiagnl_msg_get_shutdown(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_shutdown(struct idiagnl_msg *, uint8_t);
extern char *		idiagnl_msg_get_cong(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_cong(struct idiagnl_msg *, char *);
extern struct idiagnl_meminfo *idiagnl_msg_get_meminfo(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_meminfo(struct idiagnl_msg *,
                                                struct idiagnl_meminfo *);
extern struct idiagnl_vegasinfo *idiagnl_msg_get_vegasinfo(const struct idiagnl_msg *);
extern void		idiagnl_msg_set_vegasinfo(struct idiagnl_msg *,
                                                  struct idiagnl_vegasinfo *);
extern struct tcp_info idiagnl_msg_get_tcpinfo(const struct idiagnl_msg *);
extern void	       idiagnl_msg_set_tcpinfo(struct idiagnl_msg *,
                                               struct tcp_info *);

extern int		idiagnl_msg_parse(struct nlmsghdr *,
                                          struct idiagnl_msg **);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NETLINK_IDIAGNL_MSG_H_ */
