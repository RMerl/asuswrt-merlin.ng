/*
 * Copyright (C) 2007-2016  B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>, Marek Lindner <mareklindner@neomailbox.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */


#include <netinet/ether.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdint.h>
#include <linux/netlink.h>
#include <net/ethernet.h>
#include <linux/rtnetlink.h>
#include <linux/neighbour.h>
#include <errno.h>
#include <net/if.h>
#include <netlink/socket.h>
#include <netlink/netlink.h>
#include <netlink/handlers.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "main.h"
#include "functions.h"
#include "bat-hosts.h"
#include "sys.h"
#include "debug.h"
#include "debugfs.h"

static struct timeval start_time;
static char *host_name;
char *line_ptr = NULL;

const char *fs_compile_out_param[] = {
	SYS_LOG,
	SYS_LOG_LEVEL,
	batctl_settings[BATCTL_SETTINGS_BLA].sysfs_name,
	batctl_settings[BATCTL_SETTINGS_DAT].sysfs_name,
	batctl_settings[BATCTL_SETTINGS_NETWORK_CODING].sysfs_name,
	batctl_settings[BATCTL_SETTINGS_MULTICAST_MODE].sysfs_name,
	batctl_debug_tables[BATCTL_TABLE_BLA_CLAIMS].debugfs_name,
	batctl_debug_tables[BATCTL_TABLE_BLA_BACKBONES].debugfs_name,
	batctl_debug_tables[BATCTL_TABLE_DAT].debugfs_name,
	batctl_debug_tables[BATCTL_TABLE_NETWORK_CODING_NODES].debugfs_name,
	NULL,
};

void start_timer(void)
{
	gettimeofday(&start_time, NULL);
}

double end_timer(void)
{
	struct timeval end_time, diff;

	gettimeofday(&end_time, NULL);
	diff.tv_sec = end_time.tv_sec - start_time.tv_sec;
	diff.tv_usec = end_time.tv_usec - start_time.tv_usec;

	if (diff.tv_usec < 0) {
		diff.tv_sec--;
		diff.tv_usec += 1000000;
	}

	return (((double)diff.tv_sec * 1000) + ((double)diff.tv_usec / 1000));
}

char *ether_ntoa_long(const struct ether_addr *addr)
{
	static char asc[18];

	sprintf(asc, "%02x:%02x:%02x:%02x:%02x:%02x",
		addr->ether_addr_octet[0], addr->ether_addr_octet[1],
		addr->ether_addr_octet[2], addr->ether_addr_octet[3],
		addr->ether_addr_octet[4], addr->ether_addr_octet[5]);

	return asc;
}

char *get_name_by_macaddr(struct ether_addr *mac_addr, int read_opt)
{
	struct bat_host *bat_host = NULL;

	if (read_opt & USE_BAT_HOSTS)
		bat_host = bat_hosts_find_by_mac((char *)mac_addr);

	if (!bat_host)
		host_name = ether_ntoa_long((struct ether_addr *)mac_addr);
	else
		host_name = bat_host->name;

	return host_name;
}

char *get_name_by_macstr(char *mac_str, int read_opt)
{
	struct ether_addr *mac_addr;

	mac_addr = ether_aton(mac_str);
	if (!mac_addr)
		return mac_str;

	return get_name_by_macaddr(mac_addr, read_opt);
}

int file_exists(const char *fpath)
{
	struct stat st;

	return stat(fpath, &st) == 0;
}

static void file_open_problem_dbg(const char *dir, const char *fname,
				  const char *full_path)
{
	const char **ptr;
	struct stat st;

	if (strstr(dir, "/sys/")) {
		if (stat("/sys/", &st) != 0) {
			fprintf(stderr, "Error - the folder '/sys/' was not found on the system\n");
			fprintf(stderr, "Please make sure that the sys filesystem is properly mounted\n");
			return;
		}
	}

	if (!file_exists(module_ver_path)) {
		fprintf(stderr, "Error - batman-adv module has not been loaded\n");
		return;
	}

	if (!file_exists(dir)) {
		fprintf(stderr, "Error - mesh has not been enabled yet\n");
		fprintf(stderr, "Activate your mesh by adding interfaces to batman-adv\n");
		return;
	}

	for (ptr = fs_compile_out_param; *ptr; ptr++) {
		if (strcmp(*ptr, fname) != 0)
			continue;

		break;
	}

	fprintf(stderr, "Error - can't open file '%s': %s\n", full_path, strerror(errno));
	if (*ptr) {
		fprintf(stderr, "The option you called seems not to be compiled into your batman-adv kernel module.\n");
		fprintf(stderr, "Consult the README if you wish to learn more about compiling options into batman-adv.\n");
	}
}

static int str_is_mcast_addr(char *addr)
{
	struct ether_addr *mac_addr = ether_aton(addr);

	return !mac_addr ? 0 :
		mac_addr->ether_addr_octet[0] & 0x01;
}

int read_file(const char *dir, const char *fname, int read_opt,
	      float orig_timeout, float watch_interval, size_t header_lines)
{
	struct ether_addr *mac_addr;
	struct bat_host *bat_host;
	int res = EXIT_FAILURE;
	float last_seen;
	char full_path[500], *buff_ptr, *space_ptr, extra_char;
	size_t len = 0;
	FILE *fp = NULL;
	size_t line;

	if (read_opt & USE_BAT_HOSTS)
		bat_hosts_init(read_opt);

	strncpy(full_path, dir, sizeof(full_path));
	full_path[sizeof(full_path) - 1] = '\0';
	strncat(full_path, fname, sizeof(full_path) - strlen(full_path) - 1);

open:
	line = 0;
	fp = fopen(full_path, "r");

	if (!fp) {
		if (!(read_opt & SILENCE_ERRORS))
			file_open_problem_dbg(dir, fname, full_path);

		goto out;
	}

	if (read_opt & CLR_CONT_READ)
		/* clear screen, set cursor back to 0,0 */
		printf("\033[2J\033[0;0f");

read:
	while (getline(&line_ptr, &len, fp) != -1) {
		if (line++ < header_lines && read_opt & SKIP_HEADER)
			continue;

		/* the buffer will be handled elsewhere */
		if (read_opt & USE_READ_BUFF)
			break;

		/* skip timed out originators */
		if (read_opt & NO_OLD_ORIGS)
			if (sscanf(line_ptr, "%*s %f", &last_seen)
			    && (last_seen > orig_timeout))
				continue;

		/* translation table: skip multicast */
		if (line > header_lines &&
		    read_opt & UNICAST_ONLY &&
		    strlen(line_ptr) > strlen(" * xx:xx:xx:") &&
		    str_is_mcast_addr(line_ptr+3))
			continue;

		/* translation table: skip unicast */
		if (line > header_lines &&
		    read_opt & MULTICAST_ONLY &&
		    strlen(line_ptr) > strlen(" * xx:xx:xx:") &&
		    !str_is_mcast_addr(line_ptr+3))
			continue;

		if (!(read_opt & USE_BAT_HOSTS)) {
			printf("%s", line_ptr);
			continue;
		}

		/* replace mac addresses with bat host names */
		buff_ptr = line_ptr;

		while ((space_ptr = strchr(buff_ptr, ' ')) != NULL) {

			*space_ptr = '\0';
			extra_char = '\0';

			if (strlen(buff_ptr) == ETH_STR_LEN + 1) {
				extra_char = buff_ptr[ETH_STR_LEN];
				switch (extra_char) {
				case ',':
				case ')':
					buff_ptr[ETH_STR_LEN] = '\0';
					break;
				default:
					extra_char = '\0';
					break;
				}
			}

			if (strlen(buff_ptr) != ETH_STR_LEN)
				goto print_plain_buff;

			mac_addr = ether_aton(buff_ptr);

			if (!mac_addr)
				goto print_plain_buff;

			bat_host = bat_hosts_find_by_mac((char *)mac_addr);

			if (!bat_host)
				goto print_plain_buff;

			if (read_opt & LOG_MODE)
				printf("%s", bat_host->name);
			else
				/* keep table format */
				printf("%17s", bat_host->name);

			goto written;

print_plain_buff:
			printf("%s", buff_ptr);

written:
			if (extra_char != '\0')
				printf("%c", extra_char);

			printf(" ");
			buff_ptr = space_ptr + 1;
		}

		printf("%s", buff_ptr);
	}

	if (read_opt & CONT_READ) {
		usleep(1000000 * watch_interval);
		goto read;
	}

	if (read_opt & CLR_CONT_READ) {
		if (fp)
			fclose(fp);
		usleep(1000000 * watch_interval);
		goto open;
	}

	if (line_ptr)
		res = EXIT_SUCCESS;

out:
	if (fp)
		fclose(fp);

	if (read_opt & USE_BAT_HOSTS)
		bat_hosts_free();

	return res;
}

int write_file(const char *dir, const char *fname, const char *arg1,
	       const char *arg2)
{
	int fd = -1, res = EXIT_FAILURE;
	char full_path[500];
	ssize_t write_len;

	strncpy(full_path, dir, sizeof(full_path));
	full_path[sizeof(full_path) - 1] = '\0';
	strncat(full_path, fname, sizeof(full_path) - strlen(full_path) - 1);

	fd = open(full_path, O_WRONLY);

	if (fd < 0) {
		file_open_problem_dbg(dir, fname, full_path);
		goto out;
	}

	if (arg2)
		write_len = dprintf(fd, "%s %s", arg1, arg2);
	else
		write_len = write(fd, arg1, strlen(arg1) + 1);

	if (write_len < 0) {
		fprintf(stderr, "Error - can't write to file '%s': %s\n", full_path, strerror(errno));
		goto out;
	}

	res = EXIT_SUCCESS;

out:
	if (fd >= 0)
		close(fd);
	return res;
}

struct ether_addr *translate_mac(char *mesh_iface, struct ether_addr *mac)
{
	enum {
		tg_start,
		tg_mac,
		tg_via,
		tg_originator,
	} pos;
	char full_path[MAX_PATH+1];
	char *debugfs_mnt;
	static struct ether_addr in_mac;
	struct ether_addr *mac_result, *mac_tmp;
	FILE *f = NULL;
	size_t len = 0;
	char *line = NULL;
	char *input, *saveptr, *token;
	int line_invalid;

	memcpy(&in_mac, mac, sizeof(in_mac));
	mac_result = &in_mac;

	debugfs_mnt = debugfs_mount(NULL);
	if (!debugfs_mnt)
		goto out;

	debugfs_make_path(DEBUG_BATIF_PATH_FMT "/" DEBUG_TRANSTABLE_GLOBAL, mesh_iface, full_path, sizeof(full_path));

	f = fopen(full_path, "r");
	if (!f)
		goto out;

	while (getline(&line, &len, f) != -1) {
		line_invalid = 0;
		pos = tg_start;
		input = line;

		while ((token = strtok_r(input, " \t", &saveptr))) {
			input = NULL;

			switch (pos) {
			case tg_start:
				if (strcmp(token, "*") != 0)
					line_invalid = 1;
				else
					pos = tg_mac;
				break;
			case tg_mac:
				mac_tmp = ether_aton(token);
				if (!mac_tmp || memcmp(mac_tmp, &in_mac,
						       sizeof(in_mac)) != 0)
					line_invalid = 1;
				else
					pos = tg_via;
				break;
			case tg_via:
				if (strcmp(token, "via") == 0)
					pos = tg_originator;
				break;
			case tg_originator:
				mac_tmp = ether_aton(token);
				if (!mac_tmp) {
					line_invalid = 1;
				} else {
					mac_result = mac_tmp;
					goto out;
				}
				break;
			}

			if (line_invalid)
				break;
		}
	}

out:
	if (f)
		fclose(f);
	free(line);
	return mac_result;
}

static int resolve_l3addr(int ai_family, const char *asc, void *l3addr)
{
	int ret;
	struct addrinfo hints;
	struct addrinfo *res;
	struct sockaddr_in *inet4;
	struct sockaddr_in6 *inet6;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = ai_family;
	ret = getaddrinfo(asc, NULL, &hints, &res);
	if (ret)
		return -EADDRNOTAVAIL;

	if (res) {
		switch (ai_family) {
		case AF_INET:
			inet4 = (struct sockaddr_in *)res->ai_addr;
			memcpy(l3addr, &inet4->sin_addr.s_addr,
			       sizeof(inet4->sin_addr.s_addr));
			break;
		case AF_INET6:
			inet6 = (struct sockaddr_in6 *)res->ai_addr;
			memcpy(l3addr, &inet6->sin6_addr.s6_addr,
			       sizeof(inet6->sin6_addr.s6_addr));
			break;
		default:
			ret = -EINVAL;
		}
	}

	freeaddrinfo(res);
	return ret;
}

static void request_mac_resolve(int ai_family, const void *l3addr)
{
	const struct sockaddr *sockaddr;
	struct sockaddr_in inet4;
	struct sockaddr_in6 inet6;
	size_t sockaddr_len;
	int sock;
	char t = 0;

	sock = socket(ai_family, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0)
		return;

	switch (ai_family) {
	case AF_INET:
		memset(&inet4, 0, sizeof(inet4));
		inet4.sin_family = ai_family;
		inet4.sin_port = htons(9);
		memcpy(&inet4.sin_addr.s_addr, l3addr,
		       sizeof(inet4.sin_addr.s_addr));
		sockaddr = (const struct sockaddr *)&inet4;
		sockaddr_len = sizeof(inet4);
		break;
	case AF_INET6:
		memset(&inet6, 0, sizeof(inet6));
		inet6.sin6_family = ai_family;
		inet6.sin6_port = htons(9);
		memcpy(&inet6.sin6_addr.s6_addr, l3addr,
		       sizeof(inet6.sin6_addr.s6_addr));
		sockaddr = (const struct sockaddr *)&inet6;
		sockaddr_len = sizeof(inet6);
		break;
	default:
		close(sock);
		return;
	}

	sendto(sock, &t, sizeof(t), 0, sockaddr, sockaddr_len);
	close(sock);
}

struct resolve_mac_nl_arg {
	int ai_family;
	const void *l3addr;
	struct ether_addr *mac_result;
	int found;
};

static struct nla_policy neigh_policy[NDA_MAX+1] = {
	[NDA_CACHEINFO] = { .minlen = sizeof(struct nda_cacheinfo) },
	[NDA_PROBES]    = { .type = NLA_U32 },
};

static int resolve_mac_from_parse(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NDA_MAX + 1];
	struct ndmsg *nm;
	int ret;
	int l3_len;
	struct resolve_mac_nl_arg *nl_arg = arg;
	uint8_t *mac;
	uint8_t *l3addr;

	nm = nlmsg_data(nlmsg_hdr(msg));
	ret = nlmsg_parse(nlmsg_hdr(msg), sizeof(*nm), tb, NDA_MAX,
			  neigh_policy);
	if (ret < 0)
		goto err;

	if (nl_arg->ai_family != nm->ndm_family)
		goto err;

	switch (nl_arg->ai_family) {
	case AF_INET:
		l3_len = 4;
		break;
	case AF_INET6:
		l3_len = 16;
		break;
	default:
		l3_len = 0;
	}

	if (l3_len == 0)
		goto err;

	if (!tb[NDA_LLADDR] || !tb[NDA_DST])
		goto err;

	if (nla_len(tb[NDA_LLADDR]) != ETH_ALEN)
		goto err;

	if (nla_len(tb[NDA_DST]) != l3_len)
		goto err;

	mac = nla_data(tb[NDA_LLADDR]);
	l3addr = nla_data(tb[NDA_DST]);

	if (memcmp(nl_arg->l3addr, l3addr, l3_len) == 0) {
		memcpy(nl_arg->mac_result, mac, ETH_ALEN);
		nl_arg->found = 1;
	}

err:
	if (nl_arg->found)
		return NL_STOP;
	else
		return NL_OK;
}

static struct ether_addr *resolve_mac_from_cache(int ai_family,
						 const void *l3addr)
{
	struct nl_sock *sock;
	struct ether_addr *mac_result = NULL;
	static struct ether_addr mac_tmp;
	int ret;
	struct rtgenmsg gmsg = {
		.rtgen_family = ai_family,
	};
	struct nl_cb *cb = NULL;
	struct resolve_mac_nl_arg arg = {
		.ai_family = ai_family,
		.l3addr = l3addr,
		.mac_result = &mac_tmp,
		.found = 0,
	};

	sock = nl_socket_alloc();
	if (!sock)
		goto err;

	ret = nl_connect(sock, NETLINK_ROUTE);
	if (ret < 0)
		goto err;

	ret = nl_send_simple(sock, RTM_GETNEIGH, NLM_F_REQUEST | NLM_F_DUMP,
			     &gmsg, sizeof(gmsg));
	if (ret < 0)
		goto err;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
		goto err;

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, resolve_mac_from_parse, &arg);
	ret = nl_recvmsgs(sock, cb);
	if (ret < 0)
		goto err;

	if (arg.found)
		mac_result = &mac_tmp;

err:
	if (cb)
		nl_cb_put(cb);
	if (sock)
		nl_socket_free(sock);

	return mac_result;
}

static struct ether_addr *resolve_mac_from_addr(int ai_family, const char *asc)
{
	uint8_t ipv4_addr[4];
	uint8_t ipv6_addr[16];
	void *l3addr;
	int ret;
	int retries = 5;
	struct ether_addr *mac_result = NULL;

	switch (ai_family) {
	case AF_INET:
		l3addr = ipv4_addr;
		break;
	case AF_INET6:
		l3addr = ipv6_addr;
		break;
	default:
		return NULL;
	}

	ret = resolve_l3addr(ai_family, asc, l3addr);
	if (ret < 0)
		return NULL;

	while (retries-- && !mac_result) {
		mac_result = resolve_mac_from_cache(ai_family, l3addr);
		if (!mac_result) {
			request_mac_resolve(ai_family, l3addr);
			usleep(200000);
		}
	}

	return mac_result;
}

struct ether_addr *resolve_mac(const char *asc)
{
	struct ether_addr *mac_result = NULL;
	static const int ai_families[] = {AF_INET, AF_INET6};
	size_t i;

	mac_result = ether_aton(asc);
	if (mac_result)
		goto out;

	for (i = 0; i < sizeof(ai_families) / sizeof(*ai_families); i++) {
		mac_result = resolve_mac_from_addr(ai_families[i], asc);
		if (mac_result)
			goto out;
	}

out:
	return mac_result;
}

struct vlan_get_link_nl_arg {
	char *iface;
	int vid;
};

static struct nla_policy info_data_link_policy[IFLA_MAX + 1] = {
	[IFLA_LINKINFO]	= { .type = NLA_NESTED },
	[IFLA_LINK]	= { .type = NLA_U32 },
};

static struct nla_policy info_data_link_info_policy[IFLA_INFO_MAX + 1] = {
	[IFLA_INFO_DATA]	= { .type = NLA_NESTED },
};

static struct nla_policy vlan_policy[IFLA_VLAN_MAX + 1] = {
	[IFLA_VLAN_ID]		= { .type = NLA_U16 },
};

/**
 * vlan_get_link_parse - parse a get_link rtnl message and extract the important
 *  data
 * @msg: the reply msg
 * @arg: pointer to the buffer which will store the return values
 *
 * Saves the vid  in arg::vid in case of success or -1 otherwise
 */
static int vlan_get_link_parse(struct nl_msg *msg, void *arg)
{
	struct vlan_get_link_nl_arg *nl_arg = arg;
	struct nlmsghdr *n = nlmsg_hdr(msg);
	struct nlattr *tb[IFLA_MAX + 1];
	struct nlattr *li[IFLA_INFO_MAX + 1];
	struct nlattr *vi[IFLA_VLAN_MAX + 1];
	int ret;
	int idx;

	if (!nlmsg_valid_hdr(n, sizeof(struct ifinfomsg)))
		return -NLE_MSG_TOOSHORT;

	ret = nlmsg_parse(n, sizeof(struct ifinfomsg), tb, IFLA_MAX,
			  info_data_link_policy);
	if (ret < 0)
		return ret;

	if (!tb[IFLA_LINK])
		return -NLE_MISSING_ATTR;

	/* parse subattributes linkinfo */
	if (!tb[IFLA_LINKINFO])
		return -NLE_MISSING_ATTR;

	ret = nla_parse_nested(li, IFLA_INFO_MAX, tb[IFLA_LINKINFO],
			       info_data_link_info_policy);
	if (ret < 0)
		return ret;

	if (!li[IFLA_INFO_KIND])
		return -NLE_MISSING_ATTR;

	if (strcmp(nla_data(li[IFLA_INFO_KIND]), "vlan") != 0)
		goto err;

	/* parse subattributes info_data for vlan */
	if (!li[IFLA_INFO_DATA])
		return -NLE_MISSING_ATTR;

	ret = nla_parse_nested(vi, IFLA_VLAN_MAX, li[IFLA_INFO_DATA],
			       vlan_policy);
	if (ret < 0)
		return ret;

	if (!vi[IFLA_VLAN_ID])
		return -NLE_MISSING_ATTR;

	/* get parent link name */
	idx = *(int *)nla_data(tb[IFLA_LINK]);
	free(nl_arg->iface);
	nl_arg->iface = malloc(IFNAMSIZ + 1);
	if (!if_indextoname(idx, nl_arg->iface))
		goto err;

	/* get the corresponding vid */
	nl_arg->vid = *(int *)nla_data(vi[IFLA_VLAN_ID]);

err:
	if (nl_arg->vid >= 0)
		return NL_STOP;
	else
		return NL_OK;
}

/**
 * vlan_get_link - convert a VLAN interface into its parent one
 * @ifname: the interface to convert
 * @parent: buffer where the parent interface name will be written (allocated by
 *  this function)
 *
 * Returns the vlan identifier on success or -1 on error
 */
int vlan_get_link(const char *ifname, char **parent)
{
	struct nl_sock *sock;
	int ret;
	struct ifinfomsg ifinfo = {
		.ifi_family = AF_UNSPEC,
		.ifi_index = if_nametoindex(ifname),
	};
	struct nl_cb *cb = NULL;
	struct vlan_get_link_nl_arg arg = {
		.iface = NULL,
		.vid = -1,
	};

	*parent = NULL;

	sock = nl_socket_alloc();
	if (!sock)
		goto err;

	ret = nl_connect(sock, NETLINK_ROUTE);
	if (ret < 0)
		goto err;

	ret = nl_send_simple(sock, RTM_GETLINK, NLM_F_REQUEST,
			     &ifinfo, sizeof(ifinfo));
	if (ret < 0)
		goto err;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
		goto err;

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, vlan_get_link_parse, &arg);
	ret = nl_recvmsgs(sock, cb);
	if (ret < 0)
		goto err;

	*parent = arg.iface;

err:
	if (cb)
		nl_cb_put(cb);
	if (sock)
		nl_socket_free(sock);

	return arg.vid;
}
