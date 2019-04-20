/*
 * (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Vyatta Inc. <http://www.vyatta.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include <libmnl/libmnl.h>
#include <linux/netfilter/nfnetlink_cttimeout.h>
#include <libnetfilter_cttimeout/libnetfilter_cttimeout.h>

#include "nfct.h"

static void
nfct_cmd_timeout_usage(char *argv[])
{
	fprintf(stderr, "nfct v%s: Missing command\n"
			"%s <list|add|delete|get|default-set|default-get|flush> timeout "
			"[<parameters>, ...]\n", VERSION, argv[0]);
}

static int nfct_cmd_timeout_list(struct mnl_socket *nl, int argc, char *argv[]);
static int nfct_cmd_timeout_add(struct mnl_socket *nl, int argc, char *argv[]);
static int nfct_cmd_timeout_delete(struct mnl_socket *nl, int argc, char *argv[]);
static int nfct_cmd_timeout_get(struct mnl_socket *nl, int argc, char *argv[]);
static int nfct_cmd_timeout_flush(struct mnl_socket *nl, int argc, char *argv[]);
static int nfct_cmd_timeout_default_set(struct mnl_socket *nl, int argc, char *argv[]);
static int nfct_cmd_timeout_default_get(struct mnl_socket *nl, int argc, char *argv[]);

static int
nfct_timeout_parse_params(struct mnl_socket *nl, int argc, char *argv[], int cmd)
{
	int ret;

	if (argc < 3) {
		nfct_cmd_timeout_usage(argv);
		return -1;
	}

	switch (cmd) {
	case NFCT_CMD_LIST:
	case NFCT_CMD_ADD:
	case NFCT_CMD_DELETE:
	case NFCT_CMD_GET:
	case NFCT_CMD_FLUSH:
	case NFCT_CMD_DEFAULT_SET:
	case NFCT_CMD_DEFAULT_GET:
		break;
	default:
		nfct_cmd_timeout_usage(argv);
		return -1;
	}

	switch (cmd) {
	case NFCT_CMD_LIST:
		ret = nfct_cmd_timeout_list(nl, argc, argv);
		break;
	case NFCT_CMD_ADD:
		ret = nfct_cmd_timeout_add(nl, argc, argv);
		break;
	case NFCT_CMD_DELETE:
		ret = nfct_cmd_timeout_delete(nl, argc, argv);
		break;
	case NFCT_CMD_GET:
		ret = nfct_cmd_timeout_get(nl, argc, argv);
		break;
	case NFCT_CMD_FLUSH:
		ret = nfct_cmd_timeout_flush(nl, argc, argv);
		break;
	case NFCT_CMD_DEFAULT_SET:
		ret = nfct_cmd_timeout_default_set(nl, argc, argv);
		break;
	case NFCT_CMD_DEFAULT_GET:
		ret = nfct_cmd_timeout_default_get(nl, argc, argv);
		break;
	default:
		nfct_cmd_timeout_usage(argv);
		return -1;
	}

	return ret;
}

static int nfct_timeout_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nfct_timeout *t;
	char buf[4096];

	t = nfct_timeout_alloc();
	if (t == NULL) {
		nfct_perror("OOM");
		goto err;
	}

	if (nfct_timeout_nlmsg_parse_payload(nlh, t) < 0) {
		nfct_perror("nfct_timeout_nlmsg_parse_payload");
		goto err_free;
	}

	nfct_timeout_snprintf(buf, sizeof(buf), t, NFCT_TIMEOUT_O_DEFAULT, 0);
	printf("%s\n", buf);

err_free:
	nfct_timeout_free(t);
err:
	return MNL_CB_OK;
}

static int nfct_cmd_timeout_list(struct mnl_socket *nl, int argc, char *argv[])
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	unsigned int seq, portid;

	if (argc > 3) {
		nfct_perror("too many arguments");
		return -1;
	}

	seq = time(NULL);
	nlh = nfct_timeout_nlmsg_build_hdr(buf, IPCTNL_MSG_TIMEOUT_GET,
						NLM_F_DUMP, seq);

	portid = mnl_socket_get_portid(nl);
	if (nfct_mnl_talk(nl, nlh, seq, portid, nfct_timeout_cb, NULL) < 0) {
		nfct_perror("netlink error");
		return -1;
	}

	return 0;
}

static uint32_t nfct_timeout_attr_max[IPPROTO_MAX] = {
	[IPPROTO_ICMP]		= NFCT_TIMEOUT_ATTR_ICMP_MAX,
	[IPPROTO_TCP]		= NFCT_TIMEOUT_ATTR_TCP_MAX,
	[IPPROTO_UDP]		= NFCT_TIMEOUT_ATTR_UDP_MAX,
	[IPPROTO_UDPLITE]	= NFCT_TIMEOUT_ATTR_UDPLITE_MAX,
	[IPPROTO_SCTP]		= NFCT_TIMEOUT_ATTR_SCTP_MAX,
	[IPPROTO_DCCP]		= NFCT_TIMEOUT_ATTR_DCCP_MAX,
	[IPPROTO_ICMPV6]	= NFCT_TIMEOUT_ATTR_ICMPV6_MAX,
	[IPPROTO_GRE]		= NFCT_TIMEOUT_ATTR_GRE_MAX,
	[IPPROTO_RAW]		= NFCT_TIMEOUT_ATTR_GENERIC_MAX,
};

static int nfct_cmd_get_l3proto(char *argv[])
{
	int l3proto;

	if (strcmp(*argv, "inet") == 0)
		l3proto = AF_INET;
	else if (strcmp(*argv, "inet6") == 0)
		l3proto = AF_INET6;
	else {
		nfct_perror("unknown layer 3 protocol");
		return -1;
	}
	return l3proto;
}

static int nfct_cmd_get_l4proto(char *argv[])
{
	int l4proto;
	struct protoent *pent;

	pent = getprotobyname(*argv);
	if (!pent) {
		/* In Debian, /etc/protocols says ipv6-icmp. Support icmpv6
		 * as well not to break backward compatibility.
		 */
		if (strcmp(*argv, "icmpv6") == 0)
			l4proto = IPPROTO_ICMPV6;
		else if (strcmp(*argv, "generic") == 0)
			l4proto = IPPROTO_RAW;
		else {
			nfct_perror("unknown layer 4 protocol");
			return -1;
		}
	} else
		l4proto = pent->p_proto;

	return l4proto;
}

static int
nfct_cmd_timeout_parse(struct nfct_timeout *t, int argc, char *argv[])
{
	int l3proto, l4proto;
	unsigned int j;
	const char *proto_name;

	l3proto = nfct_cmd_get_l3proto(argv);
	if (l3proto < 0)
		return -1;

	nfct_timeout_attr_set_u16(t, NFCT_TIMEOUT_ATTR_L3PROTO, l3proto);

	argc--;
	argv++;
	proto_name = *argv;

	l4proto = nfct_cmd_get_l4proto(argv);
	if (l4proto < 0)
		return -1;

	nfct_timeout_attr_set_u8(t, NFCT_TIMEOUT_ATTR_L4PROTO, l4proto);
	argc--;
	argv++;

	for (; argc>1; argc-=2, argv+=2) {
		int matching = -1;

		for (j=0; j<nfct_timeout_attr_max[l4proto]; j++) {
			const char *state_name;

			state_name =
				nfct_timeout_policy_attr_to_name(l4proto, j);
			if (state_name == NULL) {
				nfct_perror("state name is NULL");
				return -1;
			}
			if (strcasecmp(*argv, state_name) != 0)
				continue;

			matching = j;
			break;
		}
		if (matching != -1) {
			nfct_timeout_policy_attr_set_u32(t, matching,
							 atoi(*(argv+1)));
		} else {
			fprintf(stderr, "nfct v%s: Wrong state name: `%s' "
					"for protocol `%s'\n",
					VERSION, *argv, proto_name);
			return -1;
		}
	}
	if (argc > 0) {
		nfct_perror("missing value for this timeout");
		return -1;
	}

	return 0;
}

int nfct_cmd_timeout_add(struct mnl_socket *nl, int argc, char *argv[])
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq;
	struct nfct_timeout *t;

	if (argc < 6) {
		nfct_perror("missing parameters\n"
			    "syntax: nfct add timeout name family protocol state1 timeout1 ...");
		return -1;
	}

	t = nfct_timeout_alloc();
	if (t == NULL) {
		nfct_perror("OOM");
		return -1;
	}

	nfct_timeout_attr_set(t, NFCT_TIMEOUT_ATTR_NAME, argv[3]);

	if (nfct_cmd_timeout_parse(t, argc-4, &argv[4]) < 0)
		return -1;

	seq = time(NULL);
	nlh = nfct_timeout_nlmsg_build_hdr(buf, IPCTNL_MSG_TIMEOUT_NEW,
				     NLM_F_CREATE | NLM_F_ACK, seq);
	nfct_timeout_nlmsg_build_payload(nlh, t);

	nfct_timeout_free(t);

	portid = mnl_socket_get_portid(nl);
	if (nfct_mnl_talk(nl, nlh, seq, portid, NULL, NULL) < 0) {
		nfct_perror("netlink error");
		return -1;
	}

	return 0;
}

int nfct_cmd_timeout_delete(struct mnl_socket *nl, int argc, char *argv[])
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq;
	struct nfct_timeout *t;

	if (argc < 4) {
		nfct_perror("missing timeout policy name");
		return -1;
	} else if (argc > 4) {
		nfct_perror("too many arguments");
		return -1;
	}

	t = nfct_timeout_alloc();
	if (t == NULL) {
		nfct_perror("OOM");
		return -1;
	}

	nfct_timeout_attr_set(t, NFCT_TIMEOUT_ATTR_NAME, argv[3]);

	seq = time(NULL);
	nlh = nfct_timeout_nlmsg_build_hdr(buf, IPCTNL_MSG_TIMEOUT_DELETE,
				     NLM_F_ACK, seq);
	nfct_timeout_nlmsg_build_payload(nlh, t);

	nfct_timeout_free(t);

	portid = mnl_socket_get_portid(nl);
	if (nfct_mnl_talk(nl, nlh, seq, portid, NULL, NULL) < 0) {
		nfct_perror("netlink error");
		return -1;
	}

	return 0;
}

int nfct_cmd_timeout_get(struct mnl_socket *nl, int argc, char *argv[])
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq;
	struct nfct_timeout *t;

	if (argc < 4) {
		nfct_perror("missing timeout policy name");
		return -1;
	} else if (argc > 4) {
		nfct_perror("too many arguments");
		return -1;
	}

	t = nfct_timeout_alloc();
	if (t == NULL) {
		nfct_perror("OOM");
		return -1;
	}
	nfct_timeout_attr_set(t, NFCT_TIMEOUT_ATTR_NAME, argv[3]);

	seq = time(NULL);
	nlh = nfct_timeout_nlmsg_build_hdr(buf, IPCTNL_MSG_TIMEOUT_GET,
				     NLM_F_ACK, seq);

	nfct_timeout_nlmsg_build_payload(nlh, t);

	nfct_timeout_free(t);

	portid = mnl_socket_get_portid(nl);
	if (nfct_mnl_talk(nl, nlh, seq, portid, nfct_timeout_cb, NULL) < 0) {
		nfct_perror("netlink error");
		return -1;
	}

	return 0;
}

int nfct_cmd_timeout_flush(struct mnl_socket *nl, int argc, char *argv[])
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq;

	if (argc > 3) {
		nfct_perror("too many arguments");
		return -1;
	}

	seq = time(NULL);
	nlh = nfct_timeout_nlmsg_build_hdr(buf, IPCTNL_MSG_TIMEOUT_DELETE,
					   NLM_F_ACK, seq);

	portid = mnl_socket_get_portid(nl);
	if (nfct_mnl_talk(nl, nlh, seq, portid, NULL, NULL) < 0) {
		nfct_perror("netlink error");
		return -1;
	}

	return 0;
}

static int
nfct_cmd_timeout_default_set(struct mnl_socket *nl, int argc, char *argv[])
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq;
	struct nfct_timeout *t;

	if (argc < 6) {
		nfct_perror("missing parameters\n"
			    "syntax: nfct default-set timeout family protocol state1 timeout1...");
		return -1;
	}

	t = nfct_timeout_alloc();
	if (t == NULL)
		return -1;

	if (nfct_cmd_timeout_parse(t, argc-3, &argv[3]) < 0)
		return -1;

	seq = time(NULL);
	nlh = nfct_timeout_nlmsg_build_hdr(buf, IPCTNL_MSG_TIMEOUT_DEFAULT_SET,
					   NLM_F_ACK, seq);
	nfct_timeout_nlmsg_build_payload(nlh, t);
	nfct_timeout_free(t);

	portid = mnl_socket_get_portid(nl);
	if (nfct_mnl_talk(nl, nlh, seq, portid, nfct_timeout_cb, NULL) < 0) {
		nfct_perror("netlink error");
		return -1;
	}

	return 0;
}

static int
nfct_cmd_timeout_default_get(struct mnl_socket *nl, int argc, char *argv[])
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq;
	struct nfct_timeout *t;
	int l3proto, l4proto;

	if (argc < 5) {
		nfct_perror("missing parameters\n"
			    "syntax: nfct default-get timeout family protocol");
		return -1;
	}

	t = nfct_timeout_alloc();
	if (t == NULL)
		return -1;

	argc-=3;
	argv+=3;

	l3proto = nfct_cmd_get_l3proto(argv);
	if (l3proto < 0)
		return -1;

	nfct_timeout_attr_set_u16(t, NFCT_TIMEOUT_ATTR_L3PROTO, l3proto);
	argc--;
	argv++;

	l4proto = nfct_cmd_get_l4proto(argv);
	if (l4proto < 0)
		return -1;

	nfct_timeout_attr_set_u8(t, NFCT_TIMEOUT_ATTR_L4PROTO, l4proto);

	seq = time(NULL);
	nlh = nfct_timeout_nlmsg_build_hdr(buf, IPCTNL_MSG_TIMEOUT_DEFAULT_GET,
					   NLM_F_ACK, seq);
	nfct_timeout_nlmsg_build_payload(nlh, t);
	nfct_timeout_free(t);

	portid = mnl_socket_get_portid(nl);
	if (nfct_mnl_talk(nl, nlh, seq, portid, nfct_timeout_cb, NULL) < 0) {
		nfct_perror("netlink error");
		return -1;
	}

	return 0;
}

static struct nfct_extension timeout = {
	.type		= NFCT_SUBSYS_TIMEOUT,
	.parse_params	= nfct_timeout_parse_params,
};

static void __init timeout_init(void)
{
	nfct_extension_register(&timeout);
}
