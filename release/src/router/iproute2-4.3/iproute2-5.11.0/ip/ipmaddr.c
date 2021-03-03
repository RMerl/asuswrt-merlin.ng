/*
 * ipmaddr.c		"ip maddress".
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/sockios.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"
#include "json_print.h"

static struct {
	char *dev;
	int  family;
} filter;

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr,
		"Usage: ip maddr [ add | del ] MULTIADDR dev STRING\n"
		"       ip maddr show [ dev STRING ]\n");
	exit(-1);
}

static int parse_hex(char *str, unsigned char *addr, size_t size)
{
	int len = 0;

	while (*str && (len < 2 * size)) {
		int tmp;

		if (str[1] == 0)
			return -1;
		if (sscanf(str, "%02x", &tmp) != 1)
			return -1;
		addr[len] = tmp;
		len++;
		str += 2;
	}
	return len;
}

struct ma_info {
	struct ma_info *next;
	int		index;
	int		users;
	char		*features;
	char		name[IFNAMSIZ];
	inet_prefix	addr;
};

static void maddr_ins(struct ma_info **lst, struct ma_info *m)
{
	struct ma_info *mp;

	for (; (mp = *lst) != NULL; lst = &mp->next) {
		if (mp->index > m->index)
			break;
	}
	m->next = *lst;
	*lst = m;
}

static void read_dev_mcast(struct ma_info **result_p)
{
	char buf[256];
	FILE *fp = fopen("/proc/net/dev_mcast", "r");

	if (!fp)
		return;

	while (fgets(buf, sizeof(buf), fp)) {
		char hexa[256];
		struct ma_info m = { .addr.family = AF_PACKET };
		int len;
		int st;

		sscanf(buf, "%d%s%d%d%s", &m.index, m.name, &m.users, &st,
		       hexa);
		if (filter.dev && strcmp(filter.dev, m.name))
			continue;

		len = parse_hex(hexa, (unsigned char *)&m.addr.data, sizeof(m.addr.data));
		if (len >= 0) {
			struct ma_info *ma = malloc(sizeof(m));

			memcpy(ma, &m, sizeof(m));
			ma->addr.bytelen = len;
			ma->addr.bitlen = len<<3;
			if (st)
				ma->features = "static";
			maddr_ins(result_p, ma);
		}
	}
	fclose(fp);
}

static void read_igmp(struct ma_info **result_p)
{
	struct ma_info m = {
		.addr.family = AF_INET,
		.addr.bitlen = 32,
		.addr.bytelen = 4,
	};
	char buf[256];
	FILE *fp = fopen("/proc/net/igmp", "r");

	if (!fp)
		return;
	if (!fgets(buf, sizeof(buf), fp)) {
		fclose(fp);
		return;
	}

	while (fgets(buf, sizeof(buf), fp)) {
		struct ma_info *ma;

		if (buf[0] != '\t') {
			size_t len;

			sscanf(buf, "%d%s", &m.index, m.name);
			len = strlen(m.name);
			if (m.name[len - 1] == ':')
				m.name[len - 1] = '\0';
			continue;
		}

		if (filter.dev && strcmp(filter.dev, m.name))
			continue;

		sscanf(buf, "%08x%d", (__u32 *)&m.addr.data, &m.users);

		ma = malloc(sizeof(m));
		memcpy(ma, &m, sizeof(m));
		maddr_ins(result_p, ma);
	}
	fclose(fp);
}


static void read_igmp6(struct ma_info **result_p)
{
	char buf[256];
	FILE *fp = fopen("/proc/net/igmp6", "r");

	if (!fp)
		return;

	while (fgets(buf, sizeof(buf), fp)) {
		char hexa[256];
		struct ma_info m = { .addr.family = AF_INET6 };
		int len;

		sscanf(buf, "%d%s%s%d", &m.index, m.name, hexa, &m.users);

		if (filter.dev && strcmp(filter.dev, m.name))
			continue;

		len = parse_hex(hexa, (unsigned char *)&m.addr.data, sizeof(m.addr.data));
		if (len >= 0) {
			struct ma_info *ma = malloc(sizeof(m));

			memcpy(ma, &m, sizeof(m));

			ma->addr.bytelen = len;
			ma->addr.bitlen = len<<3;
			maddr_ins(result_p, ma);
		}
	}
	fclose(fp);
}

static void print_maddr(FILE *fp, struct ma_info *list)
{
	print_string(PRINT_FP, NULL, "\t", NULL);

	open_json_object(NULL);
	if (list->addr.family == AF_PACKET) {
		SPRINT_BUF(b1);

		print_string(PRINT_FP, NULL, "link  ", NULL);
		print_color_string(PRINT_ANY, COLOR_MAC, "link", "%s",
				   ll_addr_n2a((void *)list->addr.data, list->addr.bytelen,
					       0, b1, sizeof(b1)));
	} else {
		print_string(PRINT_ANY, "family", "%-5s ",
			     family_name(list->addr.family));
		print_color_string(PRINT_ANY, ifa_family_color(list->addr.family),
				   "address", "%s",
				   format_host(list->addr.family,
					       -1, list->addr.data));
	}

	if (list->users != 1)
		print_uint(PRINT_ANY, "users", " users %u", list->users);

	if (list->features)
		print_string(PRINT_ANY, "features", " %s", list->features);

	print_string(PRINT_FP, NULL, "\n", NULL);
	close_json_object();
}

static void print_mlist(FILE *fp, struct ma_info *list)
{
	int cur_index = 0;

	new_json_obj(json);
	for (; list; list = list->next) {

		if (list->index != cur_index || oneline) {
			if (cur_index) {
				close_json_array(PRINT_JSON, NULL);
				close_json_object();
			}
			open_json_object(NULL);

			print_uint(PRINT_ANY, "ifindex", "%d:", list->index);
			print_color_string(PRINT_ANY, COLOR_IFNAME,
					   "ifname", "\t%s", list->name);
			print_nl();
			cur_index = list->index;

			open_json_array(PRINT_JSON, "maddr");
		}

		print_maddr(fp, list);
	}
	if (cur_index) {
		close_json_array(PRINT_JSON, NULL);
		close_json_object();
	}

	delete_json_obj();
}

static int multiaddr_list(int argc, char **argv)
{
	struct ma_info *list = NULL;

	if (!filter.family)
		filter.family = preferred_family;

	while (argc > 0) {
		if (1) {
			if (strcmp(*argv, "dev") == 0) {
				NEXT_ARG();
			} else if (matches(*argv, "help") == 0)
				usage();
			if (filter.dev)
				duparg2("dev", *argv);
			filter.dev = *argv;
		}
		argv++; argc--;
	}

	if (!filter.family || filter.family == AF_PACKET)
		read_dev_mcast(&list);
	if (!filter.family || filter.family == AF_INET)
		read_igmp(&list);
	if (!filter.family || filter.family == AF_INET6)
		read_igmp6(&list);
	print_mlist(stdout, list);
	return 0;
}

static int multiaddr_modify(int cmd, int argc, char **argv)
{
	struct ifreq ifr = {};
	int family;
	int fd, len;

	if (cmd == RTM_NEWADDR)
		cmd = SIOCADDMULTI;
	else
		cmd = SIOCDELMULTI;

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			if (ifr.ifr_name[0])
				duparg("dev", *argv);
			if (get_ifname(ifr.ifr_name, *argv))
				invarg("\"dev\" not a valid ifname", *argv);
		} else {
			if (matches(*argv, "address") == 0) {
				NEXT_ARG();
			}
			if (matches(*argv, "help") == 0)
				usage();
			if (ifr.ifr_hwaddr.sa_data[0])
				duparg("address", *argv);
			len = ll_addr_a2n(ifr.ifr_hwaddr.sa_data,
					  sizeof(ifr.ifr_hwaddr.sa_data),
					  *argv);
			if (len < 0)
				exit(1);

			if (len != ETH_ALEN) {
				fprintf(stderr, "Error: Invalid address length %d - must be %d bytes\n", len, ETH_ALEN);
				exit(1);
			}
		}
		argc--; argv++;
	}
	if (ifr.ifr_name[0] == 0) {
		fprintf(stderr, "Not enough information: \"dev\" is required.\n");
		exit(-1);
	}

	switch (preferred_family) {
	case AF_INET6:
	case AF_PACKET:
	case AF_INET:
		family = preferred_family;
		break;
	default:
		family = AF_INET;
	}

	fd = socket(family, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("Cannot create socket");
		exit(1);
	}
	if (ioctl(fd, cmd, (char *)&ifr) != 0) {
		perror("ioctl");
		exit(1);
	}
	close(fd);

	exit(0);
}


int do_multiaddr(int argc, char **argv)
{
	if (argc < 1)
		return multiaddr_list(0, NULL);
	if (matches(*argv, "add") == 0)
		return multiaddr_modify(RTM_NEWADDR, argc-1, argv+1);
	if (matches(*argv, "delete") == 0)
		return multiaddr_modify(RTM_DELADDR, argc-1, argv+1);
	if (matches(*argv, "list") == 0 || matches(*argv, "show") == 0
	    || matches(*argv, "lst") == 0)
		return multiaddr_list(argc-1, argv+1);
	if (matches(*argv, "help") == 0)
		usage();
	fprintf(stderr, "Command \"%s\" is unknown, try \"ip maddr help\".\n", *argv);
	exit(-1);
}
