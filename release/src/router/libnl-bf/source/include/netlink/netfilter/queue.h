/*
 * netlink/netfilter/queue.h	Netfilter Queue
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2007, 2008 Patrick McHardy <kaber@trash.net>
 */

#ifndef NETLINK_QUEUE_H_
#define NETLINK_QUEUE_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nl_sock;
struct nlmsghdr;
struct nfnl_queue;

extern struct nl_object_ops queue_obj_ops;

enum nfnl_queue_copy_mode {
	NFNL_QUEUE_COPY_NONE,
	NFNL_QUEUE_COPY_META,
	NFNL_QUEUE_COPY_PACKET,
};

/* General */
extern struct nl_sock *		nfnl_queue_socket_alloc(void);

extern struct nfnl_queue *	nfnl_queue_alloc(void);

extern void			nfnl_queue_get(struct nfnl_queue *);
extern void			nfnl_queue_put(struct nfnl_queue *);

/* Attributes */
extern void			nfnl_queue_set_group(struct nfnl_queue *, uint16_t);
extern int			nfnl_queue_test_group(const struct nfnl_queue *);
extern uint16_t			nfnl_queue_get_group(const struct nfnl_queue *);

extern void			nfnl_queue_set_maxlen(struct nfnl_queue *, uint32_t);
extern int			nfnl_queue_test_maxlen(const struct nfnl_queue *);
extern uint32_t			nfnl_queue_get_maxlen(const struct nfnl_queue *);

extern void			nfnl_queue_set_copy_mode(struct nfnl_queue *,
							 enum nfnl_queue_copy_mode);
extern int			nfnl_queue_test_copy_mode(const struct nfnl_queue *);
extern enum nfnl_queue_copy_mode nfnl_queue_get_copy_mode(const struct nfnl_queue *);

extern char *			nfnl_queue_copy_mode2str(enum nfnl_queue_copy_mode,
							 char *, size_t);
extern enum nfnl_queue_copy_mode nfnl_queue_str2copy_mode(const char *);

extern void			nfnl_queue_set_copy_range(struct nfnl_queue *,
							  uint32_t);
extern int			nfnl_queue_test_copy_range(const struct nfnl_queue *);
extern uint32_t			nfnl_queue_get_copy_range(const struct nfnl_queue *);

extern int	nfnl_queue_build_pf_bind(uint8_t, struct nl_msg **);
extern int	nfnl_queue_pf_bind(struct nl_sock *, uint8_t);

extern int	nfnl_queue_build_pf_unbind(uint8_t, struct nl_msg **);
extern int	nfnl_queue_pf_unbind(struct nl_sock *, uint8_t);

extern int	nfnl_queue_build_create_request(const struct nfnl_queue *,
						struct nl_msg **);
extern int	nfnl_queue_create(struct nl_sock *,
				  const struct nfnl_queue *);

extern int	nfnl_queue_build_change_request(const struct nfnl_queue *,
						struct nl_msg **);
extern int	nfnl_queue_change(struct nl_sock *,
				  const struct nfnl_queue *);

extern int	nfnl_queue_build_delete_request(const struct nfnl_queue *,
						struct nl_msg **);
extern int	nfnl_queue_delete(struct nl_sock *,
				  const struct nfnl_queue *);

#ifdef __cplusplus
}
#endif

#endif

