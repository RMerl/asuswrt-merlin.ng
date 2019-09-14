/*
 * src/nf-queue.c     Monitor netfilter queue events
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2007, 2008 Patrick McHardy <kaber@trash.net>
 * Copyright (c) 2010  Karl Hiramoto <karl@hiramoto.org>
 */


#include <netlink/cli/utils.h>
#include <netlink/cli/link.h>
#include <netinet/in.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink_queue.h>
#include <netlink/netfilter/nfnl.h>
#include <netlink/netfilter/queue.h>
#include <netlink/netfilter/queue_msg.h>

static struct nl_sock *nf_sock;

static struct nfnl_queue *alloc_queue(void)
{
	struct nfnl_queue *queue;

	queue = nfnl_queue_alloc();
	if (!queue)
		nl_cli_fatal(ENOMEM, "Unable to allocate queue object");

	return queue;
}


static void obj_input(struct nl_object *obj, void *arg)
{
	struct nfnl_queue_msg *msg = (struct nfnl_queue_msg *) obj;
	struct nl_dump_params dp = {
		.dp_type = NL_DUMP_STATS,
		.dp_fd = stdout,
		.dp_dump_msgtype = 1,
	};

	nfnl_queue_msg_set_verdict(msg, NF_ACCEPT);
	nl_object_dump(obj, &dp);
	nfnl_queue_msg_send_verdict(nf_sock, msg);
}

static int event_input(struct nl_msg *msg, void *arg)
{
	if (nl_msg_parse(msg, &obj_input, NULL) < 0)
		fprintf(stderr, "<<EVENT>> Unknown message type\n");

	/* Exit nl_recvmsgs_def() and return to the main select() */
	return NL_STOP;
}

int main(int argc, char *argv[])
{
	struct nl_sock *rt_sock;
	struct nl_cache *link_cache;
	struct nfnl_queue *queue;
	enum nfnl_queue_copy_mode copy_mode;
	uint32_t copy_range;
	int err = 1;
	int family;

	nf_sock = nfnl_queue_socket_alloc();
	if (nf_sock == NULL)
		nl_cli_fatal(ENOBUFS, "Unable to allocate netlink socket");

	nl_socket_disable_seq_check(nf_sock);
	nl_socket_modify_cb(nf_sock, NL_CB_VALID, NL_CB_CUSTOM, event_input, NULL);

	if ((argc > 1 && !strcasecmp(argv[1], "-h")) || argc < 3) {
		printf("Usage: nf-queue family group [ copy_mode ] "
		       "[ copy_range ]\n");
		printf("family: [ inet | inet6 | ... ] \n");
		printf("group: the --queue-num arg that you gave to iptables\n");
		printf("copy_mode: [ none | meta | packet ] \n");
		return 2;
	}

	nl_cli_connect(nf_sock, NETLINK_NETFILTER);

	if ((family = nl_str2af(argv[1])) == AF_UNSPEC)
		nl_cli_fatal(NLE_INVAL, "Unknown family \"%s\"", argv[1]);

	nfnl_queue_pf_unbind(nf_sock, family);
	if ((err = nfnl_queue_pf_bind(nf_sock, family)) < 0)
		nl_cli_fatal(err, "Unable to bind logger: %s",
			     nl_geterror(err));

	queue = alloc_queue();
	nfnl_queue_set_group(queue, atoi(argv[2]));

	copy_mode = NFNL_QUEUE_COPY_PACKET;
	if (argc > 3) {
		copy_mode = nfnl_queue_str2copy_mode(argv[3]);
		if (copy_mode < 0)
			nl_cli_fatal(copy_mode,
				     "Unable to parse copy mode \"%s\": %s",
				     argv[3], nl_geterror(copy_mode));
	}
	nfnl_queue_set_copy_mode(queue, copy_mode);

	copy_range = 0xFFFF;
	if (argc > 4)
		copy_range = atoi(argv[4]);
	nfnl_queue_set_copy_range(queue, copy_range);

	if ((err = nfnl_queue_create(nf_sock, queue)) < 0)
		nl_cli_fatal(err, "Unable to bind queue: %s", nl_geterror(err));

	rt_sock = nl_cli_alloc_socket();
	nl_cli_connect(rt_sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(rt_sock);

	nl_socket_set_buffer_size(nf_sock, 1024*127, 1024*127);

	while (1) {
		fd_set rfds;
		int nffd, rtfd, maxfd, retval;

		FD_ZERO(&rfds);

		maxfd = nffd = nl_socket_get_fd(nf_sock);
		FD_SET(nffd, &rfds);

		rtfd = nl_socket_get_fd(rt_sock);
		FD_SET(rtfd, &rfds);
		if (maxfd < rtfd)
			maxfd = rtfd;

		/* wait for an incoming message on the netlink socket */
		retval = select(maxfd+1, &rfds, NULL, NULL, NULL);

		if (retval) {
			if (FD_ISSET(nffd, &rfds))
				nl_recvmsgs_default(nf_sock);
			if (FD_ISSET(rtfd, &rfds))
				nl_recvmsgs_default(rt_sock);
		}
	}

	return 0;
}
