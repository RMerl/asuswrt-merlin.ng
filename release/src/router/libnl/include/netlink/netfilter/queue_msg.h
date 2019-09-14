/*
 * netlink/netfilter/queue_msg.h	Netfilter Queue Messages
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2007, 2008 Patrick McHardy <kaber@trash.net>
 */

#ifndef NETLINK_QUEUE_MSG_H_
#define NETLINK_QUEUE_MSG_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nl_sock;
struct nlmsghdr;
struct nfnl_queue_msg;

extern struct nl_object_ops queue_msg_obj_ops;

/* General */
extern struct nfnl_queue_msg *	nfnl_queue_msg_alloc(void);
extern int			nfnlmsg_queue_msg_parse(struct nlmsghdr *,
						struct nfnl_queue_msg **);

extern void			nfnl_queue_msg_get(struct nfnl_queue_msg *);
extern void			nfnl_queue_msg_put(struct nfnl_queue_msg *);

extern void			nfnl_queue_msg_set_group(struct nfnl_queue_msg *, uint16_t);
extern int			nfnl_queue_msg_test_group(const struct nfnl_queue_msg *);
extern uint16_t			nfnl_queue_msg_get_group(const struct nfnl_queue_msg *);

extern void			nfnl_queue_msg_set_family(struct nfnl_queue_msg *, uint8_t);
extern int			nfnl_queue_msg_test_family(const struct nfnl_queue_msg *);
extern uint8_t			nfnl_queue_msg_get_family(const struct nfnl_queue_msg *);

extern void			nfnl_queue_msg_set_packetid(struct nfnl_queue_msg *, uint32_t);
extern int			nfnl_queue_msg_test_packetid(const struct nfnl_queue_msg *);
extern uint32_t			nfnl_queue_msg_get_packetid(const struct nfnl_queue_msg *);

extern void			nfnl_queue_msg_set_hwproto(struct nfnl_queue_msg *, uint16_t);
extern int			nfnl_queue_msg_test_hwproto(const struct nfnl_queue_msg *);
extern uint16_t			nfnl_queue_msg_get_hwproto(const struct nfnl_queue_msg *);

extern void			nfnl_queue_msg_set_hook(struct nfnl_queue_msg *, uint8_t);
extern int			nfnl_queue_msg_test_hook(const struct nfnl_queue_msg *);
extern uint8_t			nfnl_queue_msg_get_hook(const struct nfnl_queue_msg *);

extern void			nfnl_queue_msg_set_mark(struct nfnl_queue_msg *, uint32_t);
extern int			nfnl_queue_msg_test_mark(const struct nfnl_queue_msg *);
extern uint32_t			nfnl_queue_msg_get_mark(const struct nfnl_queue_msg *);

extern void			nfnl_queue_msg_set_timestamp(struct nfnl_queue_msg *,
							      struct timeval *);
extern int			nfnl_queue_msg_test_timestamp(const struct nfnl_queue_msg *);
extern const struct timeval *	nfnl_queue_msg_get_timestamp(const struct nfnl_queue_msg *);

extern void			nfnl_queue_msg_set_indev(struct nfnl_queue_msg *, uint32_t);
extern int			nfnl_queue_msg_test_indev(const struct nfnl_queue_msg *);
extern uint32_t			nfnl_queue_msg_get_indev(const struct nfnl_queue_msg *);

extern void			nfnl_queue_msg_set_outdev(struct nfnl_queue_msg *, uint32_t);
extern int			nfnl_queue_msg_test_outdev(const struct nfnl_queue_msg *);
extern uint32_t			nfnl_queue_msg_get_outdev(const struct nfnl_queue_msg *);

extern void			nfnl_queue_msg_set_physindev(struct nfnl_queue_msg *, uint32_t);
extern int			nfnl_queue_msg_test_physindev(const struct nfnl_queue_msg *);
extern uint32_t			nfnl_queue_msg_get_physindev(const struct nfnl_queue_msg *);

extern void			nfnl_queue_msg_set_physoutdev(struct nfnl_queue_msg *, uint32_t);
extern int			nfnl_queue_msg_test_physoutdev(const struct nfnl_queue_msg *);
extern uint32_t			nfnl_queue_msg_get_physoutdev(const struct nfnl_queue_msg *);

extern void			nfnl_queue_msg_set_hwaddr(struct nfnl_queue_msg *, uint8_t *, int);
extern int			nfnl_queue_msg_test_hwaddr(const struct nfnl_queue_msg *);
extern const uint8_t *		nfnl_queue_msg_get_hwaddr(const struct nfnl_queue_msg *, int *);

extern int			nfnl_queue_msg_set_payload(struct nfnl_queue_msg *, uint8_t *, int);
extern int			nfnl_queue_msg_test_payload(const struct nfnl_queue_msg *);
extern const void *		nfnl_queue_msg_get_payload(const struct nfnl_queue_msg *, int *);

extern void			nfnl_queue_msg_set_verdict(struct nfnl_queue_msg *,
							   unsigned int);
extern int			nfnl_queue_msg_test_verdict(const struct nfnl_queue_msg *);
extern unsigned int		nfnl_queue_msg_get_verdict(const struct nfnl_queue_msg *);

extern struct nl_msg *		nfnl_queue_msg_build_verdict(const struct nfnl_queue_msg *);
extern int			nfnl_queue_msg_send_verdict(struct nl_sock *,
							    const struct nfnl_queue_msg *);
extern int			nfnl_queue_msg_send_verdict_batch(struct nl_sock *,
							    const struct nfnl_queue_msg *);
extern int			nfnl_queue_msg_send_verdict_payload(struct nl_sock *,
						const struct nfnl_queue_msg *,
						const void *, unsigned );
#ifdef __cplusplus
}
#endif

#endif

