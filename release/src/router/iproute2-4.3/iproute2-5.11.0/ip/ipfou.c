/*
 * ipfou.c	FOU (foo over UDP) support
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:	Tom Herbert <therbert@google.com>
 */

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <linux/fou.h>
#include <linux/genetlink.h>
#include <linux/ip.h>
#include <arpa/inet.h>

#include "libgenl.h"
#include "utils.h"
#include "ip_common.h"
#include "json_print.h"

static void usage(void)
{
	fprintf(stderr,
		"Usage: ip fou add port PORT { ipproto PROTO  | gue }\n"
		"		   [ local IFADDR ] [ peer IFADDR ]\n"
		"		   [ peer_port PORT ] [ dev IFNAME ]\n"
		"       ip fou del port PORT [ local IFADDR ]\n"
		"		   [ peer IFADDR ] [ peer_port PORT ]\n"
		"		   [ dev IFNAME ]\n"
		"       ip fou show\n"
		"\n"
		"Where: PROTO { ipproto-name | 1..255 }\n"
		"       PORT { 1..65535 }\n"
		"       IFADDR { addr }\n");

	exit(-1);
}

/* netlink socket */
static struct rtnl_handle genl_rth = { .fd = -1 };
static int genl_family = -1;

#define FOU_REQUEST(_req, _bufsiz, _cmd, _flags)	\
	GENL_REQUEST(_req, _bufsiz, genl_family, 0,	\
		     FOU_GENL_VERSION, _cmd, _flags)

static int fou_parse_opt(int argc, char **argv, struct nlmsghdr *n,
			 bool adding)
{
	const char *local = NULL, *peer = NULL;
	__u16 port, peer_port = 0;
	__u8 family = preferred_family;
	bool gue_set = false;
	int ipproto_set = 0;
	__u8 ipproto, type;
	int port_set = 0;
	int index = 0;

	if (preferred_family == AF_UNSPEC) {
		family = AF_INET;
	}

	while (argc > 0) {
		if (!matches(*argv, "port")) {
			NEXT_ARG();

			if (get_be16(&port, *argv, 0) || port == 0)
				invarg("invalid port", *argv);
			port_set = 1;
		} else if (!matches(*argv, "ipproto")) {
			struct protoent *servptr;

			NEXT_ARG();

			servptr = getprotobyname(*argv);
			if (servptr)
				ipproto = servptr->p_proto;
			else if (get_u8(&ipproto, *argv, 0) || ipproto == 0)
				invarg("invalid ipproto", *argv);
			ipproto_set = 1;
		} else if (!matches(*argv, "gue")) {
			gue_set = true;
		} else if (!matches(*argv, "-6")) {
			family = AF_INET6;
		} else if (!matches(*argv, "local")) {
			NEXT_ARG();

			local = *argv;
		} else if (!matches(*argv, "peer")) {
			NEXT_ARG();

			peer = *argv;
		} else if (!matches(*argv, "peer_port")) {
			NEXT_ARG();

			if (get_be16(&peer_port, *argv, 0) || peer_port == 0)
				invarg("invalid peer port", *argv);
		} else if (!matches(*argv, "dev")) {
			const char *ifname;

			NEXT_ARG();

			ifname = *argv;

			if (check_ifname(ifname)) {
				fprintf(stderr, "fou: invalid device name\n");
				exit(EXIT_FAILURE);
			}

			index = ll_name_to_index(ifname);

			if (!index) {
				fprintf(stderr, "fou: unknown device name\n");
				exit(EXIT_FAILURE);
			}
		} else {
			fprintf(stderr
				, "fou: unknown command \"%s\"?\n", *argv);
			usage();
			return -1;
		}
		argc--, argv++;
	}

	if (!port_set) {
		fprintf(stderr, "fou: missing port\n");
		return -1;
	}

	if (!ipproto_set && !gue_set && adding) {
		fprintf(stderr, "fou: must set ipproto or gue\n");
		return -1;
	}

	if (ipproto_set && gue_set) {
		fprintf(stderr, "fou: cannot set ipproto and gue\n");
		return -1;
	}

	if ((peer_port && !peer) || (peer && !peer_port)) {
		fprintf(stderr, "fou: both peer and peer port must be set\n");
		return -1;
	}

	type = gue_set ? FOU_ENCAP_GUE : FOU_ENCAP_DIRECT;

	addattr16(n, 1024, FOU_ATTR_PORT, port);
	addattr8(n, 1024, FOU_ATTR_TYPE, type);
	addattr8(n, 1024, FOU_ATTR_AF, family);

	if (ipproto_set)
		addattr8(n, 1024, FOU_ATTR_IPPROTO, ipproto);

	if (local) {
		inet_prefix local_addr;
		__u8 attr_type = family == AF_INET ? FOU_ATTR_LOCAL_V4 :
						     FOU_ATTR_LOCAL_V6;

		if (get_addr(&local_addr, local, family)) {
			fprintf(stderr, "fou: parsing local address failed\n");
			exit(EXIT_FAILURE);
		}
		addattr_l(n, 1024, attr_type, &local_addr.data,
			  local_addr.bytelen);
	}

	if (peer) {
		inet_prefix peer_addr;
		__u8 attr_type = family == AF_INET ? FOU_ATTR_PEER_V4 :
						     FOU_ATTR_PEER_V6;

		if (get_addr(&peer_addr, peer, family)) {
			fprintf(stderr, "fou: parsing peer address failed\n");
			exit(EXIT_FAILURE);
		}
		addattr_l(n, 1024, attr_type, &peer_addr.data,
			  peer_addr.bytelen);

		if (peer_port)
			addattr16(n, 1024, FOU_ATTR_PEER_PORT, peer_port);
	}

	if (index)
		addattr32(n, 1024, FOU_ATTR_IFINDEX, index);

	return 0;
}

static int do_add(int argc, char **argv)
{
	FOU_REQUEST(req, 1024, FOU_CMD_ADD, NLM_F_REQUEST);

	fou_parse_opt(argc, argv, &req.n, true);

	if (rtnl_talk(&genl_rth, &req.n, NULL) < 0)
		return -2;

	return 0;
}

static int do_del(int argc, char **argv)
{
	FOU_REQUEST(req, 1024, FOU_CMD_DEL, NLM_F_REQUEST);

	fou_parse_opt(argc, argv, &req.n, false);

	if (rtnl_talk(&genl_rth, &req.n, NULL) < 0)
		return -2;

	return 0;
}

static int print_fou_mapping(struct nlmsghdr *n, void *arg)
{
	__u8 family = AF_INET, local_attr_type, peer_attr_type, byte_len;
	struct rtattr *tb[FOU_ATTR_MAX + 1];
	__u8 empty_buf[16] = {0};
	struct genlmsghdr *ghdr;
	int len = n->nlmsg_len;

	if (n->nlmsg_type != genl_family)
		return 0;

	len -= NLMSG_LENGTH(GENL_HDRLEN);
	if (len < 0)
		return -1;

	ghdr = NLMSG_DATA(n);
	parse_rtattr(tb, FOU_ATTR_MAX, (void *) ghdr + GENL_HDRLEN, len);

	open_json_object(NULL);
	if (tb[FOU_ATTR_PORT])
		print_uint(PRINT_ANY, "port", "port %u",
			   ntohs(rta_getattr_u16(tb[FOU_ATTR_PORT])));

	if (tb[FOU_ATTR_TYPE] &&
	    rta_getattr_u8(tb[FOU_ATTR_TYPE]) == FOU_ENCAP_GUE)
		print_null(PRINT_ANY, "gue", " gue", NULL);
	else if (tb[FOU_ATTR_IPPROTO])
		print_uint(PRINT_ANY, "ipproto",
			   " ipproto %u", rta_getattr_u8(tb[FOU_ATTR_IPPROTO]));

	if (tb[FOU_ATTR_AF]) {
		family = rta_getattr_u8(tb[FOU_ATTR_AF]);

		print_string(PRINT_JSON, "family", NULL,
			     family_name(family));

		if (family == AF_INET6)
			print_string(PRINT_FP, NULL,
				     " -6", NULL);
	}

	local_attr_type = family == AF_INET ? FOU_ATTR_LOCAL_V4 :
					      FOU_ATTR_LOCAL_V6;
	peer_attr_type = family == AF_INET ? FOU_ATTR_PEER_V4 :
					     FOU_ATTR_PEER_V6;
	byte_len = af_bit_len(family) / 8;

	if (tb[local_attr_type] && memcmp(RTA_DATA(tb[local_attr_type]),
					  empty_buf, byte_len)) {
		print_string(PRINT_ANY, "local", " local %s",
			     format_host_rta(family, tb[local_attr_type]));
	}

	if (tb[peer_attr_type] && memcmp(RTA_DATA(tb[peer_attr_type]),
					 empty_buf, byte_len)) {
		print_string(PRINT_ANY, "peer", " peer %s",
			     format_host_rta(family, tb[peer_attr_type]));
	}

	if (tb[FOU_ATTR_PEER_PORT]) {
		__u16 p_port = ntohs(rta_getattr_u16(tb[FOU_ATTR_PEER_PORT]));

		if (p_port)
			print_uint(PRINT_ANY, "peer_port", " peer_port %u",
				   p_port);

	}

	if (tb[FOU_ATTR_IFINDEX]) {
		int index = rta_getattr_s32(tb[FOU_ATTR_IFINDEX]);

		if (index) {
			const char *ifname;

			ifname = ll_index_to_name(index);

			if (ifname)
				print_string(PRINT_ANY, "dev", " dev %s",
					     ifname);
		}
	}

	print_string(PRINT_FP, NULL, "\n", NULL);
	close_json_object();

	return 0;
}

static int do_show(int argc, char **argv)
{
	FOU_REQUEST(req, 4096, FOU_CMD_GET, NLM_F_REQUEST | NLM_F_DUMP);

	if (argc > 0) {
		fprintf(stderr,
			"\"ip fou show\" does not take any arguments.\n");
		return -1;
	}

	if (rtnl_send(&genl_rth, &req.n, req.n.nlmsg_len) < 0) {
		perror("Cannot send show request");
		exit(1);
	}

	new_json_obj(json);
	if (rtnl_dump_filter(&genl_rth, print_fou_mapping, stdout) < 0) {
		fprintf(stderr, "Dump terminated\n");
		return 1;
	}
	delete_json_obj();
	fflush(stdout);

	return 0;
}

int do_ipfou(int argc, char **argv)
{
	if (argc < 1)
		usage();

	if (matches(*argv, "help") == 0)
		usage();

	if (genl_init_handle(&genl_rth, FOU_GENL_NAME, &genl_family))
		exit(1);

	if (matches(*argv, "add") == 0)
		return do_add(argc-1, argv+1);
	if (matches(*argv, "delete") == 0)
		return do_del(argc-1, argv+1);
	if (matches(*argv, "show") == 0)
		return do_show(argc-1, argv+1);

	fprintf(stderr,
		"Command \"%s\" is unknown, try \"ip fou help\".\n", *argv);
	exit(-1);
}
