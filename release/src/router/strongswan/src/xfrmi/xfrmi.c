/*
 * Copyright (C) 2019 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "kernel_netlink_shared.h"

#ifndef IFLA_XFRM_MAX
enum {
	IFLA_XFRM_UNSPEC,
	IFLA_XFRM_LINK,
	IFLA_XFRM_IF_ID,
	__IFLA_XFRM_MAX
};
#define IFLA_XFRM_MAX (__IFLA_XFRM_MAX - 1)
#endif

/**
 * Create an XFRM interface with the given ID and underlying interface
 */
static int add_xfrm_interface(char *name, uint32_t xfrm_id, uint32_t ifindex)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr;
	struct ifinfomsg *msg;
	struct rtattr *linkinfo, *info_data;
	netlink_socket_t *socket;
	int status = 1;

	socket = netlink_socket_create(NETLINK_ROUTE, NULL, FALSE);
	if (!socket)
	{
		return 1;
	}

	memset(&request, 0, sizeof(request));

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
	hdr->nlmsg_type = RTM_NEWLINK;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));

	msg = NLMSG_DATA(hdr);
	msg->ifi_family = AF_UNSPEC;

	netlink_add_attribute(hdr, IFLA_IFNAME, chunk_from_str(name),
						  sizeof(request));

	linkinfo = netlink_nested_start(hdr, sizeof(request), IFLA_LINKINFO);

	netlink_add_attribute(hdr, IFLA_INFO_KIND, chunk_from_str("xfrm"),
						  sizeof(request));

	info_data = netlink_nested_start(hdr, sizeof(request), IFLA_INFO_DATA);

	netlink_add_attribute(hdr, IFLA_XFRM_IF_ID, chunk_from_thing(xfrm_id),
						  sizeof(request));
	netlink_add_attribute(hdr, IFLA_XFRM_LINK, chunk_from_thing(ifindex),
						  sizeof(request));

	netlink_nested_end(hdr, info_data);
	netlink_nested_end(hdr, linkinfo);

	switch (socket->send_ack(socket, hdr))
	{
		case SUCCESS:
			status = 0;
			break;
		case ALREADY_DONE:
			fprintf(stderr, "XFRM interface already exists\n");
			break;
		default:
			fprintf(stderr, "failed to create XFRM interface\n");
			break;
	}

	socket->destroy(socket);
	return status;
}

/**
 * Parse attributes nested in IFLA_INFO_DATA
 */
static void parse_info_data(struct rtattr *rta, size_t rtasize, char *phys,
							uint32_t *if_id)
{
	uint32_t ifindex;

	while (RTA_OK(rta, rtasize))
	{
		switch (rta->rta_type)
		{
			case IFLA_XFRM_IF_ID:
				if (RTA_PAYLOAD(rta) == sizeof(*if_id))
				{
					*if_id = *(uint32_t*)RTA_DATA(rta);
				}
				break;
			case IFLA_XFRM_LINK:
				if (RTA_PAYLOAD(rta) == sizeof(ifindex))
				{
					ifindex = *(uint32_t*)RTA_DATA(rta);
					if_indextoname(ifindex, phys);
				}
				break;
			default:
				break;
		}
		rta = RTA_NEXT(rta, rtasize);
	}
}

/**
 * Parse attributes nested in IFLA_LINKINFO
 */
static void parse_linkinfo(struct rtattr *rta, size_t rtasize, char *phys,
						   uint32_t *if_id)
{
	while (RTA_OK(rta, rtasize))
	{
		switch (rta->rta_type)
		{
			case IFLA_INFO_DATA:
				parse_info_data(RTA_DATA(rta), RTA_PAYLOAD(rta), phys, if_id);
				break;
			default:
				break;
		}
		rta = RTA_NEXT(rta, rtasize);
	}
}

/**
 * List all installed XFRM interfaces
 */
static int list_xfrm_interfaces()
{
	netlink_buf_t request;
	struct nlmsghdr *hdr, *out, *current;
	struct ifinfomsg *msg;
	struct rtattr *linkinfo;
	netlink_socket_t *socket;
	size_t len;
	int status = 0;

	socket = netlink_socket_create(NETLINK_ROUTE, NULL, FALSE);
	if (!socket)
	{
		return 1;
	}

	memset(&request, 0, sizeof(request));

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	hdr->nlmsg_type = RTM_GETLINK;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));

	msg = NLMSG_DATA(hdr);
	msg->ifi_family = AF_UNSPEC;

	linkinfo = netlink_nested_start(hdr, sizeof(request), IFLA_LINKINFO);

	netlink_add_attribute(hdr, IFLA_INFO_KIND, chunk_from_str("xfrm"),
						  sizeof(request));

	netlink_nested_end(hdr, linkinfo);

	if (socket->send(socket, hdr, &out, &len) != SUCCESS)
	{
		return FAILED;
	}
	current = out;
	while (NLMSG_OK(current, len))
	{
		switch (current->nlmsg_type)
		{
			case NLMSG_DONE:
				break;
			case RTM_NEWLINK:
				msg = NLMSG_DATA(current);
				struct rtattr *rta = IFLA_RTA(msg);
				size_t rtasize = IFLA_PAYLOAD(current);
				char *name = NULL, phys[IF_NAMESIZE] = {};
				uint32_t if_id = 0;

				while (RTA_OK(rta, rtasize))
				{
					switch (rta->rta_type)
					{
						case IFLA_IFNAME:
							name = RTA_DATA(rta);
							break;
						case IFLA_LINKINFO:
							parse_linkinfo(RTA_DATA(rta), RTA_PAYLOAD(rta),
										   phys, &if_id);
							break;
						default:
							break;
					}
					rta = RTA_NEXT(rta, rtasize);
				}
				if (name)
				{
					printf("%2u: %-16s dev %-8s if_id 0x%.8x [%u]\n",
						   msg->ifi_index, name, phys, if_id, if_id);
				}
				/* fall through */
			default:
				current = NLMSG_NEXT(current, len);
				continue;
		}
		break;
	}
	free(out);

	socket->destroy(socket);
	return status;
}

static void usage(FILE *out, char *name)
{
	fprintf(out, "Create XFRM interfaces\n\n");
	fprintf(out, "%s [OPTIONS]\n\n", name);
	fprintf(out, "Options:\n");
	fprintf(out, "  -h, --help          print this help.\n");
	fprintf(out, "  -v, --debug         set debug level, default: 1.\n");
	fprintf(out, "  -l, --list          list XFRM interfaces.\n");
	fprintf(out, "  -n, --name=NAME     name of the XFRM interface.\n");
	fprintf(out, "  -i, --id=ID         optional numeric XFRM ID.\n");
	fprintf(out, "  -d, --dev=DEVICE    underlying physical interface.\n");
	fprintf(out, "\n");
}

int main(int argc, char *argv[])
{
	char *name = NULL, *dev = NULL, *end;
	uint32_t xfrm_id = 0;
	u_int ifindex;

	library_init(NULL, "xfrmi");
	atexit(library_deinit);

	while (true)
	{
		struct option long_opts[] = {
			{"help",		no_argument,		NULL,	'h' },
			{"debug",		no_argument,		NULL,	'v' },
			{"list",		no_argument,		NULL,	'l' },
			{"name",		required_argument,	NULL,	'n' },
			{"id",			required_argument,	NULL,	'i' },
			{"dev",			required_argument,	NULL,	'd' },
			{0,0,0,0 },
		};
		switch (getopt_long(argc, argv, "hvln:i:d:", long_opts, NULL))
		{
			case EOF:
				break;
			case 'h':
				usage(stdout, argv[0]);
				return 0;
			case 'l':
				list_xfrm_interfaces();
				return 0;
			case 'v':
				dbg_default_set_level(atoi(optarg));
				continue;
			case 'n':
				name = optarg;
				continue;
			case 'i':
				errno = 0;
				xfrm_id = strtoul(optarg, &end, 0);
				if (errno || *end)
				{
					fprintf(stderr, "invalid XFRM ID: %s\n",
							errno ? strerror(errno) : end);
					return 1;
				}
				continue;
			case 'd':
				dev = optarg;
				continue;
			default:
				usage(stderr, argv[0]);
				return 1;
		}
		break;
	}

	if (!name || !dev)
	{
		fprintf(stderr, "please specify a name and a physical interface\n");
		return 1;
	}
	ifindex = if_nametoindex(dev);
	if (!ifindex)
	{
		fprintf(stderr, "physical interface %s not found\n", dev);
		return 1;
	}

	return add_xfrm_interface(name, xfrm_id, ifindex);
}
