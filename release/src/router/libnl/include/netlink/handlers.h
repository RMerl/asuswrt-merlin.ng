/*
 * netlink/handlers.c	default netlink message handlers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_HANDLERS_H_
#define NETLINK_HANDLERS_H_

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <netlink/netlink-compat.h>
#include <netlink/netlink-kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nl_cb;
struct nl_sock;
struct nl_msg;
struct ucred;

/**
 * @name Callback Typedefs
 * @{
 */

/**
 * nl_recvmsgs() callback for message processing customization
 * @ingroup cb
 * @arg msg		netlink message being processed
 * @arg arg		argument passwd on through caller
 */
typedef int (*nl_recvmsg_msg_cb_t)(struct nl_msg *msg, void *arg);

/**
 * nl_recvmsgs() callback for error message processing customization
 * @ingroup cb
 * @arg nla		netlink address of the peer
 * @arg nlerr		netlink error message being processed
 * @arg arg		argument passed on through caller
 */
typedef int (*nl_recvmsg_err_cb_t)(struct sockaddr_nl *nla,
				   struct nlmsgerr *nlerr, void *arg);

/** @} */

/**
 * Callback actions
 * @ingroup cb
 */
enum nl_cb_action {
	/** Proceed with wathever would come next */
	NL_OK,
	/** Skip this message */
	NL_SKIP,
	/** Stop parsing altogether and discard remaining messages */
	NL_STOP,
};

/**
 * Callback kinds
 * @ingroup cb
 */
enum nl_cb_kind {
	/** Default handlers (quiet) */
	NL_CB_DEFAULT,
	/** Verbose default handlers (error messages printed) */
	NL_CB_VERBOSE,
	/** Debug handlers for debugging */
	NL_CB_DEBUG,
	/** Customized handler specified by the user */
	NL_CB_CUSTOM,
	__NL_CB_KIND_MAX,
};

#define NL_CB_KIND_MAX (__NL_CB_KIND_MAX - 1)

/**
 * Callback types
 * @ingroup cb
 */
enum nl_cb_type {
	/** Message is valid */
	NL_CB_VALID,
	/** Last message in a series of multi part messages received */
	NL_CB_FINISH,
	/** Report received that data was lost */
	NL_CB_OVERRUN,
	/** Message wants to be skipped */
	NL_CB_SKIPPED,
	/** Message is an acknowledge */
	NL_CB_ACK,
	/** Called for every message received */
	NL_CB_MSG_IN,
	/** Called for every message sent out except for nl_sendto() */
	NL_CB_MSG_OUT,
	/** Message is malformed and invalid */
	NL_CB_INVALID,
	/** Called instead of internal sequence number checking */
	NL_CB_SEQ_CHECK,
	/** Sending of an acknowledge message has been requested */
	NL_CB_SEND_ACK,
	/** Flag NLM_F_DUMP_INTR is set in message */
	NL_CB_DUMP_INTR,
	__NL_CB_TYPE_MAX,
};

#define NL_CB_TYPE_MAX (__NL_CB_TYPE_MAX - 1)

extern struct nl_cb *	nl_cb_alloc(enum nl_cb_kind);
extern struct nl_cb *	nl_cb_clone(struct nl_cb *);
extern struct nl_cb *	nl_cb_get(struct nl_cb *);
extern void		nl_cb_put(struct nl_cb *);

extern int  nl_cb_set(struct nl_cb *, enum nl_cb_type, enum nl_cb_kind,
		      nl_recvmsg_msg_cb_t, void *);
extern int  nl_cb_set_all(struct nl_cb *, enum nl_cb_kind,
			  nl_recvmsg_msg_cb_t, void *);
extern int  nl_cb_err(struct nl_cb *, enum nl_cb_kind, nl_recvmsg_err_cb_t,
		      void *);

extern void nl_cb_overwrite_recvmsgs(struct nl_cb *,
				     int (*func)(struct nl_sock *,
						 struct nl_cb *));
extern void nl_cb_overwrite_recv(struct nl_cb *,
				 int (*func)(struct nl_sock *,
					     struct sockaddr_nl *,
					     unsigned char **,
					     struct ucred **));
extern void nl_cb_overwrite_send(struct nl_cb *,
				 int (*func)(struct nl_sock *,
					     struct nl_msg *));

extern enum nl_cb_type nl_cb_active_type(struct nl_cb *cb);

#ifdef __cplusplus
}
#endif

#endif
