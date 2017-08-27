/*
 * src/nf-log.c     Monitor netfilter log events
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2007 Philip Craig <philipc@snapgear.com>
 * Copyright (c) 2007 Secure Computing Corporation
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/link.h>
#include <linux/netfilter/nfnetlink_log.h>
#include <netlink/netfilter/nfnl.h>
#include <netlink/netfilter/log.h>

static struct nfnl_log *alloc_log(void)
{
	struct nfnl_log *log;

	log = nfnl_log_alloc();
	if (!log)
		nl_cli_fatal(ENOMEM, "Unable to allocate log object");

	return log;
}

static void obj_input(struct nl_object *obj, void *arg)
{
	struct nl_dump_params dp = {
		.dp_type = NL_DUMP_STATS,
		.dp_fd = stdout,
		.dp_dump_msgtype = 1,
	};

	nl_object_dump(obj, &dp);
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
	struct nl_sock *nf_sock;
	struct nl_sock *rt_sock;
        struct nl_cache *link_cache;
	struct nfnl_log *log;
	enum nfnl_log_copy_mode copy_mode;
	uint32_t copy_range;
	int err;
	int family;

	nf_sock = nl_cli_alloc_socket();
	nl_socket_disable_seq_check(nf_sock);
	nl_socket_modify_cb(nf_sock, NL_CB_VALID, NL_CB_CUSTOM, event_input, NULL);

	if ((argc > 1 && !strcasecmp(argv[1], "-h")) || argc < 3) {
		printf("Usage: nf-log family group [ copy_mode ] "
		       "[copy_range] \n");
		return 2;
	}

	nl_cli_connect(nf_sock, NETLINK_NETFILTER);

	family = nl_str2af(argv[1]);
	if (family == AF_UNSPEC)
		nl_cli_fatal(NLE_INVAL, "Unknown family \"%s\": %s",
			     argv[1], nl_geterror(family));

	nfnl_log_pf_unbind(nf_sock, family);
	if ((err = nfnl_log_pf_bind(nf_sock, family)) < 0)
		nl_cli_fatal(err, "Unable to bind logger: %s",
			     nl_geterror(err));

	log = alloc_log();
	nfnl_log_set_group(log, atoi(argv[2]));

	copy_mode = NFNL_LOG_COPY_META;
	if (argc > 3) {
		copy_mode = nfnl_log_str2copy_mode(argv[3]);
		if (copy_mode < 0)
			nl_cli_fatal(copy_mode,
				     "Unable to parse copy mode \"%s\": %s",
				     argv[3], nl_geterror(copy_mode));
	}
	nfnl_log_set_copy_mode(log, copy_mode);

	copy_range = 0xFFFF;
	if (argc > 4)
		copy_mode = atoi(argv[4]);
	nfnl_log_set_copy_range(log, copy_range);

	if ((err = nfnl_log_create(nf_sock, log)) < 0)
		nl_cli_fatal(err, "Unable to bind instance: %s",
			     nl_geterror(err));

	{
		struct nl_dump_params dp = {
			.dp_type = NL_DUMP_STATS,
			.dp_fd = stdout,
			.dp_dump_msgtype = 1,
		};

		printf("log params: ");
		nl_object_dump((struct nl_object *) log, &dp);
	}

	rt_sock = nl_cli_alloc_socket();
	nl_cli_connect(rt_sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(rt_sock);

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

		/* wait for an incoming message on the netlink nf_socket */
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
