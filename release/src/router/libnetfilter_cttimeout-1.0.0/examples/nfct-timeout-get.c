/*
 * (C) 2005-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2012 by Vyatta Inc. <http://www.vyatta.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>

#include <linux/netfilter/nfnetlink_cttimeout.h>
#include <libmnl/libmnl.h>
#include <libnetfilter_cttimeout/libnetfilter_cttimeout.h>

static int timeout_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nfct_timeout *t;
	char buf[4096];

	t = nfct_timeout_alloc();
	if (t == NULL) {
		perror("OOM");
		goto err;
	}

	if (nfct_timeout_nlmsg_parse_payload(nlh, t) < 0) {
		perror("nfct_timeout_nlmsg_parse_payload");
		goto err_free;
	}

	nfct_timeout_snprintf(buf, sizeof(buf), t, NFCT_TIMEOUT_O_DEFAULT, 0);
	printf("%s\n", buf);

err_free:
	nfct_timeout_free(t);
err:
	return MNL_CB_OK;
}

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq;
	struct nfct_timeout *t = NULL;
	int ret;

	if (argc == 2) {
		t = nfct_timeout_alloc();
		if (t == NULL) {
			perror("OOM");
			exit(EXIT_FAILURE);
		}
	}

	seq = time(NULL);
	if (t == NULL) {
		nlh = nfct_timeout_nlmsg_build_hdr(buf, IPCTNL_MSG_TIMEOUT_GET,
						NLM_F_DUMP, seq);
	} else {
		nlh = nfct_timeout_nlmsg_build_hdr(buf, IPCTNL_MSG_TIMEOUT_GET,
						NLM_F_ACK, seq);
		nfct_timeout_attr_set(t, NFCT_TIMEOUT_ATTR_NAME, argv[1]);
		nfct_timeout_nlmsg_build_payload(nlh, t);
		nfct_timeout_free(t);
	}

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}
	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		perror("mnl_socket_send");
		exit(EXIT_FAILURE);
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, portid, timeout_cb, NULL);
		if (ret <= 0)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1) {
		perror("error");
		exit(EXIT_FAILURE);
	}
	mnl_socket_close(nl);

	return EXIT_SUCCESS;
}
