/*
 * nlsock.h - netlink socket
 *
 * Declarations of netlink socket structure and related functions.
 */

#ifndef ETHTOOL_NETLINK_NLSOCK_H__
#define ETHTOOL_NETLINK_NLSOCK_H__

#include <../../libmnl-1.0.4/include/libmnl/libmnl.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <linux/ethtool_netlink.h>
#include "msgbuff.h"

struct nl_context;

/**
 * struct nl_socket - netlink socket abstraction
 * @nlctx:   netlink context
 * @sk:      libmnl socket handle
 * @msgbuff: embedded message buffer used by default
 * @port:    port number for netlink header
 * @seq:     autoincremented sequence number for netlink header
 * @nl_fam:  netlink family (e.g. NETLINK_GENERIC or NETLINK_ROUTE)
 */
struct nl_socket {
	struct nl_context	*nlctx;
	struct mnl_socket	*sk;
	struct nl_msg_buff	msgbuff;
	unsigned int		port;
	unsigned int		seq;
	int			nl_fam;
};

int nlsock_init(struct nl_context *nlctx, struct nl_socket **__nlsk,
		int nl_fam);
void nlsock_done(struct nl_socket *nlsk);
int nlsock_prep_get_request(struct nl_socket *nlsk, unsigned int nlcmd,
			    uint16_t hdr_attrtype, u32 flags);
ssize_t nlsock_sendmsg(struct nl_socket *nlsk, struct nl_msg_buff *__msgbuff);
int nlsock_send_get_request(struct nl_socket *nlsk, mnl_cb_t cb);
int nlsock_process_reply(struct nl_socket *nlsk, mnl_cb_t reply_cb, void *data);

#endif /* ETHTOOL_NETLINK_NLSOCK_H__ */
