/*
 * netlink/socket.h		Netlink Socket
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_SOCKET_H_
#define NETLINK_SOCKET_H_

#include <netlink/types.h>
#include <netlink/handlers.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct nl_sock *	nl_socket_alloc(void);
extern struct nl_sock *	nl_socket_alloc_cb(struct nl_cb *);
extern void		nl_socket_free(struct nl_sock *);

extern uint32_t		nl_socket_get_local_port(const struct nl_sock *);
extern void		nl_socket_set_local_port(struct nl_sock *, uint32_t);

extern int		nl_socket_add_memberships(struct nl_sock *, int, ...);
extern int		nl_socket_add_membership(struct nl_sock *, int);
extern int		nl_socket_drop_memberships(struct nl_sock *, int, ...);
extern int		nl_socket_drop_membership(struct nl_sock *,
							  int);
extern void		nl_join_groups(struct nl_sock *, int);


extern uint32_t		nl_socket_get_peer_port(const struct nl_sock *);
extern void		nl_socket_set_peer_port(struct nl_sock *,
							uint32_t);
extern uint32_t 	nl_socket_get_peer_groups(const struct nl_sock *sk);
extern void 		nl_socket_set_peer_groups(struct nl_sock *sk, uint32_t groups);
extern struct nl_cb *	nl_socket_get_cb(const struct nl_sock *);
extern void		nl_socket_set_cb(struct nl_sock *,
						 struct nl_cb *);
extern int		nl_socket_modify_cb(struct nl_sock *, enum nl_cb_type,
					    enum nl_cb_kind,
					    nl_recvmsg_msg_cb_t, void *);
extern int nl_socket_modify_err_cb(struct nl_sock *, enum nl_cb_kind,
				   nl_recvmsg_err_cb_t, void *);

extern int		nl_socket_set_buffer_size(struct nl_sock *, int, int);
extern int		nl_socket_set_passcred(struct nl_sock *, int);
extern int		nl_socket_recv_pktinfo(struct nl_sock *, int);

extern void		nl_socket_disable_seq_check(struct nl_sock *);
extern unsigned int	nl_socket_use_seq(struct nl_sock *);
extern void		nl_socket_disable_auto_ack(struct nl_sock *);
extern void		nl_socket_enable_auto_ack(struct nl_sock *);

extern int		nl_socket_get_fd(const struct nl_sock *);
extern int		nl_socket_set_nonblocking(const struct nl_sock *);
extern void		nl_socket_enable_msg_peek(struct nl_sock *);
extern void		nl_socket_disable_msg_peek(struct nl_sock *);

#ifdef __cplusplus
}
#endif

#endif
