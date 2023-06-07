/*
 * bearer.c	TIPC bearer functionality.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Richard Alpe <richard.alpe@ericsson.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>

#include <linux/tipc_netlink.h>
#include <linux/tipc.h>
#include <linux/genetlink.h>
#include <linux/if.h>

#include <libmnl/libmnl.h>
#include <sys/socket.h>

#include "utils.h"
#include "cmdl.h"
#include "msg.h"
#include "bearer.h"

#define UDP_PROP_IP 1
#define UDP_PROP_PORT 2

struct cb_data {
	int attr;
	int prop;
	struct nlmsghdr *nlh;
};

static void _print_bearer_opts(void)
{
	fprintf(stderr,
		"OPTIONS\n"
		" priority		- Bearer link priority\n"
		" tolerance		- Bearer link tolerance\n"
		" window		- Bearer link window\n"
		" mtu			- Bearer link mtu\n");
}

void print_bearer_media(void)
{
	fprintf(stderr,
		"\nMEDIA\n"
		" udp			- User Datagram Protocol\n"
		" ib			- Infiniband\n"
		" eth			- Ethernet\n");
}

static void cmd_bearer_enable_l2_help(struct cmdl *cmdl, char *media)
{
	fprintf(stderr,
		"Usage: %s bearer enable media %s device DEVICE [OPTIONS]\n"
		"\nOPTIONS\n"
		" domain DOMAIN		- Discovery domain\n"
		" priority PRIORITY	- Bearer priority\n",
		cmdl->argv[0], media);
}

static void cmd_bearer_enable_udp_help(struct cmdl *cmdl, char *media)
{
	fprintf(stderr,
		"Usage: %s bearer enable [OPTIONS] media %s name NAME [localip IP|device DEVICE] [UDP OPTIONS]\n\n"
		"OPTIONS\n"
		" domain DOMAIN		- Discovery domain\n"
		" priority PRIORITY	- Bearer priority\n\n"
		"UDP OPTIONS\n"
		" localport PORT	- Local UDP port (default 6118)\n"
		" remoteip IP		- Remote IP address\n"
		" remoteport PORT	- Remote UDP port (default 6118)\n",
		cmdl->argv[0], media);
}

static int get_netid_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_NET_MAX + 1] = {};
	int *netid = (int*)data;

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);
	if (!info[TIPC_NLA_NET])
		return MNL_CB_ERROR;
	mnl_attr_parse_nested(info[TIPC_NLA_NET], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_NET_ID])
		return MNL_CB_ERROR;
	*netid = mnl_attr_get_u32(attrs[TIPC_NLA_NET_ID]);

	return MNL_CB_OK;
}

static int generate_multicast(short af, char *buf, int bufsize)
{
	int netid;
	char mnl_msg[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;

	if (!(nlh = msg_init(mnl_msg, TIPC_NL_NET_GET))) {
		fprintf(stderr, "error, message initialization failed\n");
		return -1;
	}
	if (msg_dumpit(nlh, get_netid_cb, &netid)) {
		fprintf(stderr, "error, failed to fetch TIPC network id from kernel\n");
		return -EINVAL;
	}
	if (af == AF_INET)
		snprintf(buf, bufsize, "228.0.%u.%u", (netid>>8) & 0xFF, netid & 0xFF);
	else
		snprintf(buf, bufsize, "ff02::%u", netid);

	return 0;
}

static struct ifreq ifr;
static int nl_dump_req_filter(struct nlmsghdr *nlh, int reqlen)
{
	struct ifaddrmsg *ifa = NLMSG_DATA(nlh);

	ifa->ifa_index = ifr.ifr_ifindex;

	return 0;
}

static int nl_dump_addr_filter(struct nlmsghdr *nlh, void *arg)
{
	struct ifaddrmsg *ifa = NLMSG_DATA(nlh);
	char *r_addr = (char *)arg;
	int len = nlh->nlmsg_len;
	struct rtattr *addr_attr;

	if (ifr.ifr_ifindex != ifa->ifa_index)
		return 0;

	if (strlen(r_addr) > 0)
		return 0;

	addr_attr = parse_rtattr_one(IFA_ADDRESS, IFA_RTA(ifa),
				     len - NLMSG_LENGTH(sizeof(*ifa)));
	if (!addr_attr)
		return 0;

	if (ifa->ifa_family == AF_INET) {
		struct sockaddr_in ip4addr;
		memcpy(&ip4addr.sin_addr, RTA_DATA(addr_attr),
		       sizeof(struct in_addr));
		inet_ntop(AF_INET, &ip4addr.sin_addr, r_addr,
			  INET_ADDRSTRLEN);
	} else if (ifa->ifa_family == AF_INET6) {
		struct sockaddr_in6 ip6addr;
		memcpy(&ip6addr.sin6_addr, RTA_DATA(addr_attr),
		       sizeof(struct in6_addr));
		inet_ntop(AF_INET6, &ip6addr.sin6_addr, r_addr,
			  INET6_ADDRSTRLEN);
	}
	return 0;
}

static int cmd_bearer_validate_and_get_addr(const char *name, char *r_addr)
{
	struct rtnl_handle rth = { .fd = -1 };
	int err = -1;

	memset(&ifr, 0, sizeof(ifr));
	if (!name || !r_addr || get_ifname(ifr.ifr_name, name))
		return err;

	ifr.ifr_ifindex = ll_name_to_index(ifr.ifr_name);
	if (!ifr.ifr_ifindex)
		return err;

	/* remove from cache */
	ll_drop_by_index(ifr.ifr_ifindex);

	if ((err = rtnl_open(&rth, 0)) < 0)
		return err;

	if ((err = rtnl_addrdump_req(&rth, AF_UNSPEC, nl_dump_req_filter)) > 0)
		err = rtnl_dump_filter(&rth, nl_dump_addr_filter, r_addr);

	rtnl_close(&rth);
	return err;
}

static int nl_add_udp_enable_opts(struct nlmsghdr *nlh, struct opt *opts,
				  struct cmdl *cmdl)
{
	int err;
	struct opt *opt;
	struct nlattr *nest;
	char buf[INET6_ADDRSTRLEN];
	char *locport = "6118";
	char *remport = "6118";
	char *locip = NULL;
	char *remip = NULL;
	struct addrinfo *loc = NULL;
	struct addrinfo *rem = NULL;
	struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_DGRAM
	};
	char addr[INET6_ADDRSTRLEN] = {0};

	opt = get_opt(opts, "device");
	if (opt && cmd_bearer_validate_and_get_addr(opt->val, addr) < 0) {
		fprintf(stderr, "error, no device name available\n");
		return -EINVAL;
	}

	if (strlen(addr) > 0) {
		locip = addr;
	} else {
		opt = get_opt(opts, "localip");
		if (!opt) {
			fprintf(stderr, "error, udp bearer localip/device missing\n");
			cmd_bearer_enable_udp_help(cmdl, "udp");
			return -EINVAL;
		}
		locip = opt->val;
	}

	if ((opt = get_opt(opts, "remoteip")))
		remip = opt->val;

	if ((opt = get_opt(opts, "localport")))
		locport = opt->val;

	if ((opt = get_opt(opts, "remoteport")))
		remport = opt->val;

	if ((err = getaddrinfo(locip, locport, &hints, &loc))) {
		fprintf(stderr, "UDP local address error: %s\n",
			gai_strerror(err));
		return err;
	}

	if (!remip) {
		if (generate_multicast(loc->ai_family, buf, sizeof(buf))) {
			fprintf(stderr, "Failed to generate multicast address\n");
			freeaddrinfo(loc);
			return -EINVAL;
		}
		remip = buf;
	}

	if ((err = getaddrinfo(remip, remport, &hints, &rem))) {
		fprintf(stderr, "UDP remote address error: %s\n",
			gai_strerror(err));
		freeaddrinfo(loc);
		return err;
	}

	if (rem->ai_family != loc->ai_family) {
		fprintf(stderr, "UDP local and remote AF mismatch\n");
		freeaddrinfo(rem);
		freeaddrinfo(loc);
		return -EINVAL;
	}

	nest = mnl_attr_nest_start(nlh, TIPC_NLA_BEARER_UDP_OPTS);
	mnl_attr_put(nlh, TIPC_NLA_UDP_LOCAL, loc->ai_addrlen, loc->ai_addr);
	mnl_attr_put(nlh, TIPC_NLA_UDP_REMOTE, rem->ai_addrlen, rem->ai_addr);
	mnl_attr_nest_end(nlh, nest);

	freeaddrinfo(rem);
	freeaddrinfo(loc);

	return 0;
}

static char *cmd_get_media_type(const struct cmd *cmd, struct cmdl *cmdl,
				struct opt *opts)
{
	struct opt *opt = get_opt(opts, "media");

	if (!opt) {
		if (help_flag)
			(cmd->help)(cmdl);
		else
			fprintf(stderr, "error, missing bearer media\n");
		return NULL;
	}
	return opt->val;
}

static int nl_add_bearer_name(struct nlmsghdr *nlh, const struct cmd *cmd,
			      struct cmdl *cmdl, struct opt *opts,
			      const struct tipc_sup_media *sup_media)
{
	char bname[TIPC_MAX_BEARER_NAME];
	int err;

	if ((err = cmd_get_unique_bearer_name(cmd, cmdl, opts, bname, sup_media)))
		return err;

	mnl_attr_put_strz(nlh, TIPC_NLA_BEARER_NAME, bname);
	return 0;
}

int cmd_get_unique_bearer_name(const struct cmd *cmd, struct cmdl *cmdl,
			       struct opt *opts, char *bname,
			       const struct tipc_sup_media *sup_media)
{
	char *media;
	char *identifier;
	struct opt *opt;
	const struct tipc_sup_media *entry;

	if (!(media = cmd_get_media_type(cmd, cmdl, opts)))
		return -EINVAL;

	for (entry = sup_media; entry->media; entry++) {
		if (strcmp(entry->media, media))
			continue;

		if (!(opt = get_opt(opts, entry->identifier))) {
			if (help_flag)
				(entry->help)(cmdl, media);
			else
				fprintf(stderr, "error, missing bearer %s\n",
					entry->identifier);
			return -EINVAL;
		}

		identifier = opt->val;
		snprintf(bname, TIPC_MAX_BEARER_NAME, "%s:%s", media, identifier);

		return 0;
	}

	fprintf(stderr, "error, invalid media type %s\n", media);

	return -EINVAL;
}

static void cmd_bearer_add_udp_help(struct cmdl *cmdl, char *media)
{
	fprintf(stderr, "Usage: %s bearer add media %s name NAME remoteip REMOTEIP\n\n",
		cmdl->argv[0], media);
}

static void cmd_bearer_add_help(struct cmdl *cmdl)
{
	fprintf(stderr, "Usage: %s bearer add media udp name NAME remoteip REMOTEIP\n",
		cmdl->argv[0]);
}

static int udp_bearer_add(struct nlmsghdr *nlh, struct opt *opts,
			  struct cmdl *cmdl)
{
	int err;
	struct opt *opt;
	struct nlattr *opts_nest;
	char *remport = "6118";

	opts_nest = mnl_attr_nest_start(nlh, TIPC_NLA_BEARER_UDP_OPTS);

	if ((opt = get_opt(opts, "remoteport")))
		remport = opt->val;

	if ((opt = get_opt(opts, "remoteip"))) {
		char *ip = opt->val;
		struct addrinfo *addr = NULL;
		struct addrinfo hints = {
			.ai_family = AF_UNSPEC,
			.ai_socktype = SOCK_DGRAM
		};

		if ((err = getaddrinfo(ip, remport, &hints, &addr))) {
			fprintf(stderr, "UDP address error: %s\n",
				gai_strerror(err));
			freeaddrinfo(addr);
			return err;
		}

		mnl_attr_put(nlh, TIPC_NLA_UDP_REMOTE, addr->ai_addrlen,
			     addr->ai_addr);
		freeaddrinfo(addr);
	} else {
		fprintf(stderr, "error, missing remoteip\n");
		return -EINVAL;
	}
	mnl_attr_nest_end(nlh, opts_nest);

	return 0;
}

static int cmd_bearer_add_media(struct nlmsghdr *nlh, const struct cmd *cmd,
				struct cmdl *cmdl, void *data)
{
	int err;
	char *media;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct opt *opt;
	struct nlattr *attrs;
	struct opt opts[] = {
		{ "remoteip",		OPT_KEYVAL,	NULL },
		{ "remoteport",		OPT_KEYVAL,	NULL },
		{ "name",		OPT_KEYVAL,	NULL },
		{ "media",		OPT_KEYVAL,	NULL },
		{ NULL }
	};
	const struct tipc_sup_media sup_media[] = {
		{ "udp",	"name",		cmd_bearer_add_udp_help},
		{ NULL, },
	};

	/* Rewind optind to include media in the option list */
	cmdl->optind--;
	if (parse_opts(opts, cmdl) < 0)
		return -EINVAL;

	if (!(opt = get_opt(opts, "media"))) {
		fprintf(stderr, "error, missing media value\n");
		return -EINVAL;
	}
	media = opt->val;

	if (strcmp(media, "udp") != 0) {
		fprintf(stderr, "error, no \"%s\" media specific options available\n",
			media);
		return -EINVAL;
	}
	if (!(opt = get_opt(opts, "name"))) {
		fprintf(stderr, "error, missing media name\n");
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_BEARER_ADD))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	attrs = mnl_attr_nest_start(nlh, TIPC_NLA_BEARER);
	err = nl_add_bearer_name(nlh, cmd, cmdl, opts, sup_media);
	if (err)
		return err;

	err = udp_bearer_add(nlh, opts, cmdl);
	if (err)
		return err;

	mnl_attr_nest_end(nlh, attrs);

	return msg_doit(nlh, NULL, NULL);
}

static int cmd_bearer_add(struct nlmsghdr *nlh, const struct cmd *cmd,
			  struct cmdl *cmdl, void *data)
{
	const struct cmd cmds[] = {
		{ "media",	cmd_bearer_add_media,	cmd_bearer_add_help },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}

static void cmd_bearer_enable_help(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Usage: %s bearer enable [OPTIONS] media MEDIA ARGS...\n\n"
		"OPTIONS\n"
		" domain DOMAIN         - Discovery domain\n"
		" priority PRIORITY     - Bearer priority\n",
		cmdl->argv[0]);
	print_bearer_media();
}

static int cmd_bearer_enable(struct nlmsghdr *nlh, const struct cmd *cmd,
			     struct cmdl *cmdl, void *data)
{
	int err;
	struct opt *opt;
	struct nlattr *nest;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct opt opts[] = {
		{ "device",		OPT_KEYVAL,	NULL },
		{ "domain",		OPT_KEYVAL,	NULL },
		{ "localip",		OPT_KEYVAL,	NULL },
		{ "localport",		OPT_KEYVAL,	NULL },
		{ "media",		OPT_KEYVAL,	NULL },
		{ "name",		OPT_KEYVAL,	NULL },
		{ "priority",		OPT_KEYVAL,	NULL },
		{ "remoteip",		OPT_KEYVAL,	NULL },
		{ "remoteport",		OPT_KEYVAL,	NULL },
		{ NULL }
	};
	struct tipc_sup_media sup_media[] = {
		{ "udp",        "name",         cmd_bearer_enable_udp_help},
		{ "eth",        "device",       cmd_bearer_enable_l2_help },
		{ "ib",         "device",       cmd_bearer_enable_l2_help },
		{ NULL, },
	};

	if (parse_opts(opts, cmdl) < 0) {
		if (help_flag)
			(cmd->help)(cmdl);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_BEARER_ENABLE))) {
		fprintf(stderr, "error: message initialisation failed\n");
		return -1;
	}
	nest = mnl_attr_nest_start(nlh, TIPC_NLA_BEARER);

	if ((opt = get_opt(opts, "domain")))
		mnl_attr_put_u32(nlh, TIPC_NLA_BEARER_DOMAIN, atoi(opt->val));

	if ((opt = get_opt(opts, "priority"))) {
		struct nlattr *props;

		props = mnl_attr_nest_start(nlh, TIPC_NLA_BEARER_PROP);
		mnl_attr_put_u32(nlh, TIPC_NLA_PROP_PRIO, atoi(opt->val));
		mnl_attr_nest_end(nlh, props);
	}

	err = nl_add_bearer_name(nlh, cmd, cmdl, opts, sup_media);
	if (err)
		return err;

	opt = get_opt(opts, "media");
	if (opt && strcmp(opt->val, "udp") == 0) {
		err = nl_add_udp_enable_opts(nlh, opts, cmdl);
		if (err)
			return err;
	}
	mnl_attr_nest_end(nlh, nest);

	return msg_doit(nlh, NULL, NULL);
}

static void cmd_bearer_disable_l2_help(struct cmdl *cmdl, char *media)
{
	fprintf(stderr, "Usage: %s bearer disable media %s device DEVICE\n",
		cmdl->argv[0], media);
}

static void cmd_bearer_disable_udp_help(struct cmdl *cmdl, char *media)
{
	fprintf(stderr, "Usage: %s bearer disable media %s name NAME\n",
		cmdl->argv[0], media);
}

static void cmd_bearer_disable_help(struct cmdl *cmdl)
{
	fprintf(stderr, "Usage: %s bearer disable media MEDIA ARGS...\n",
		cmdl->argv[0]);
	print_bearer_media();
}

static int cmd_bearer_disable(struct nlmsghdr *nlh, const struct cmd *cmd,
			      struct cmdl *cmdl, void *data)
{
	int err;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlattr *nest;
	struct opt opts[] = {
		{ "device",		OPT_KEYVAL,	NULL },
		{ "name",		OPT_KEYVAL,	NULL },
		{ "media",		OPT_KEYVAL,	NULL },
		{ NULL }
	};
	struct tipc_sup_media sup_media[] = {
		{ "udp",        "name",         cmd_bearer_disable_udp_help},
		{ "eth",        "device",       cmd_bearer_disable_l2_help },
		{ "ib",         "device",       cmd_bearer_disable_l2_help },
		{ NULL, },
	};

	if (parse_opts(opts, cmdl) < 0) {
		if (help_flag)
			(cmd->help)(cmdl);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_BEARER_DISABLE))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	nest = mnl_attr_nest_start(nlh, TIPC_NLA_BEARER);
	err = nl_add_bearer_name(nlh, cmd, cmdl, opts, sup_media);
	if (err)
		return err;
	mnl_attr_nest_end(nlh, nest);

	return msg_doit(nlh, NULL, NULL);

}

static void cmd_bearer_set_help(struct cmdl *cmdl)
{
	fprintf(stderr, "Usage: %s bearer set OPTION media MEDIA ARGS...\n",
		cmdl->argv[0]);
	_print_bearer_opts();
	print_bearer_media();
}

static void cmd_bearer_set_udp_help(struct cmdl *cmdl, char *media)
{
	fprintf(stderr, "Usage: %s bearer set OPTION media %s name NAME\n\n",
		cmdl->argv[0], media);
	_print_bearer_opts();
}

static void cmd_bearer_set_l2_help(struct cmdl *cmdl, char *media)
{
	fprintf(stderr,
		"Usage: %s bearer set [OPTION]... media %s device DEVICE\n",
		cmdl->argv[0], media);
	_print_bearer_opts();
}

static int cmd_bearer_set_prop(struct nlmsghdr *nlh, const struct cmd *cmd,
			       struct cmdl *cmdl, void *data)
{
	int err;
	int val;
	int prop;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlattr *props;
	struct nlattr *attrs;
	struct opt opts[] = {
		{ "device",		OPT_KEYVAL,	NULL },
		{ "media",		OPT_KEYVAL,	NULL },
		{ "name",		OPT_KEYVAL,	NULL },
		{ NULL }
	};
	struct tipc_sup_media sup_media[] = {
		{ "udp",        "name",         cmd_bearer_set_udp_help},
		{ "eth",        "device",       cmd_bearer_set_l2_help },
		{ "ib",         "device",       cmd_bearer_set_l2_help },
		{ NULL, },
	};

	if (strcmp(cmd->cmd, "priority") == 0)
		prop = TIPC_NLA_PROP_PRIO;
	else if ((strcmp(cmd->cmd, "tolerance") == 0))
		prop = TIPC_NLA_PROP_TOL;
	else if ((strcmp(cmd->cmd, "window") == 0))
		prop = TIPC_NLA_PROP_WIN;
	else if ((strcmp(cmd->cmd, "mtu") == 0))
		prop = TIPC_NLA_PROP_MTU;
	else
		return -EINVAL;

	if (cmdl->optind >= cmdl->argc) {
		fprintf(stderr, "error, missing value\n");
		return -EINVAL;
	}
	val = atoi(shift_cmdl(cmdl));

	if (parse_opts(opts, cmdl) < 0)
		return -EINVAL;

	if (prop == TIPC_NLA_PROP_MTU) {
		char *media = cmd_get_media_type(cmd, cmdl, opts);

		if (!media)
			return -EINVAL;
		else if (strcmp(media, "udp")) {
			fprintf(stderr, "error, not supported for media\n");
			return -EINVAL;
		}
	}

	if (!(nlh = msg_init(buf, TIPC_NL_BEARER_SET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}
	attrs = mnl_attr_nest_start(nlh, TIPC_NLA_BEARER);

	props = mnl_attr_nest_start(nlh, TIPC_NLA_BEARER_PROP);
	mnl_attr_put_u32(nlh, prop, val);
	mnl_attr_nest_end(nlh, props);

	err = nl_add_bearer_name(nlh, cmd, cmdl, opts, sup_media);
	if (err)
		return err;

	mnl_attr_nest_end(nlh, attrs);

	return msg_doit(nlh, NULL, NULL);
}

static int cmd_bearer_set(struct nlmsghdr *nlh, const struct cmd *cmd,
			  struct cmdl *cmdl, void *data)
{
	const struct cmd cmds[] = {
		{ "priority",	cmd_bearer_set_prop,	cmd_bearer_set_help },
		{ "tolerance",	cmd_bearer_set_prop,	cmd_bearer_set_help },
		{ "window",	cmd_bearer_set_prop,	cmd_bearer_set_help },
		{ "mtu",	cmd_bearer_set_prop,	cmd_bearer_set_help },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}

static void cmd_bearer_get_help(struct cmdl *cmdl)
{
	fprintf(stderr, "Usage: %s bearer get [OPTION] media MEDIA ARGS...\n",
		cmdl->argv[0]);
	_print_bearer_opts();
	print_bearer_media();
}

static void cmd_bearer_get_udp_help(struct cmdl *cmdl, char *media)
{
	fprintf(stderr, "Usage: %s bearer get [OPTION] media %s name NAME [UDP OPTIONS]\n\n",
		cmdl->argv[0], media);
	fprintf(stderr,
		"UDP OPTIONS\n"
		" remoteip              - Remote ip address\n"
		" remoteport            - Remote port\n"
		" localip               - Local ip address\n"
		" localport             - Local port\n\n");
	_print_bearer_opts();
}

static void cmd_bearer_get_l2_help(struct cmdl *cmdl, char *media)
{
	fprintf(stderr,
		"Usage: %s bearer get OPTION media %s device DEVICE\n",
		cmdl->argv[0], media);
	_print_bearer_opts();
}


static int bearer_dump_udp_cb(const struct nlmsghdr *nlh, void *data)
{
	struct sockaddr_storage *addr;
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_UDP_MAX + 1] = {};

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);

	if (!info[TIPC_NLA_UDP_REMOTE])
		return MNL_CB_ERROR;

	addr = mnl_attr_get_payload(info[TIPC_NLA_UDP_REMOTE]);

	if (addr->ss_family == AF_INET) {
		struct sockaddr_in *ipv4 = (struct sockaddr_in *) addr;

		printf("%s\n", inet_ntoa(ipv4->sin_addr));
	} else if (addr->ss_family == AF_INET6) {
		char straddr[INET6_ADDRSTRLEN];
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) addr;

		if (!inet_ntop(AF_INET6, &ipv6->sin6_addr, straddr,
			       sizeof(straddr))) {
			fprintf(stderr, "error, parsing IPv6 addr\n");
			return MNL_CB_ERROR;
		}
		printf("%s\n", straddr);

	} else {
		return MNL_CB_ERROR;
	}

	return MNL_CB_OK;
}

static int bearer_get_udp_cb(const struct nlmsghdr *nlh, void *data)
{
	struct cb_data *cb_data = (struct cb_data *) data;
	struct sockaddr_storage *addr;
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_BEARER_MAX + 1] = {};
	struct nlattr *opts[TIPC_NLA_UDP_MAX + 1] = {};

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);
	if (!info[TIPC_NLA_BEARER])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_BEARER], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_BEARER_UDP_OPTS])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(attrs[TIPC_NLA_BEARER_UDP_OPTS], parse_attrs, opts);
	if (!opts[TIPC_NLA_UDP_LOCAL])
		return MNL_CB_ERROR;

	if ((cb_data->attr == TIPC_NLA_UDP_REMOTE) &&
	    (cb_data->prop == UDP_PROP_IP) &&
	    opts[TIPC_NLA_UDP_MULTI_REMOTEIP]) {
		struct genlmsghdr *genl = mnl_nlmsg_get_payload(cb_data->nlh);

		genl->cmd = TIPC_NL_UDP_GET_REMOTEIP;
		return msg_dumpit(cb_data->nlh, bearer_dump_udp_cb, NULL);
	}

	addr = mnl_attr_get_payload(opts[cb_data->attr]);

	if (addr->ss_family == AF_INET) {
		struct sockaddr_in *ipv4 = (struct sockaddr_in *) addr;

		switch (cb_data->prop) {
		case UDP_PROP_IP:
			printf("%s\n", inet_ntoa(ipv4->sin_addr));
			break;
		case UDP_PROP_PORT:
			printf("%u\n", ntohs(ipv4->sin_port));
			break;
		default:
			return MNL_CB_ERROR;
		}

	} else if (addr->ss_family == AF_INET6) {
		char straddr[INET6_ADDRSTRLEN];
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) addr;

		switch (cb_data->prop) {
		case UDP_PROP_IP:
			if (!inet_ntop(AF_INET6, &ipv6->sin6_addr, straddr,
				       sizeof(straddr))) {
				fprintf(stderr, "error, parsing IPv6 addr\n");
				return MNL_CB_ERROR;
			}
			printf("%s\n", straddr);
			break;
		case UDP_PROP_PORT:
			printf("%u\n", ntohs(ipv6->sin6_port));
			break;
		default:
			return MNL_CB_ERROR;
		}

	} else {
		return MNL_CB_ERROR;
	}

	return MNL_CB_OK;
}

static int bearer_get_cb(const struct nlmsghdr *nlh, void *data)
{
	int *prop = data;
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_BEARER_MAX + 1] = {};
	struct nlattr *props[TIPC_NLA_PROP_MAX + 1] = {};

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);
	if (!info[TIPC_NLA_BEARER])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_BEARER], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_BEARER_PROP])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(attrs[TIPC_NLA_BEARER_PROP], parse_attrs, props);
	if (!props[*prop])
		return MNL_CB_ERROR;

	printf("%u\n", mnl_attr_get_u32(props[*prop]));

	return MNL_CB_OK;
}

static int cmd_bearer_get_media(struct nlmsghdr *nlh, const struct cmd *cmd,
				struct cmdl *cmdl, void *data)
{
	int err;
	char *media;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct opt *opt;
	struct cb_data cb_data = {0};
	struct nlattr *attrs;
	struct opt opts[] = {
		{ "localip",		OPT_KEY,	NULL },
		{ "localport",		OPT_KEY,	NULL },
		{ "remoteip",		OPT_KEY,	NULL },
		{ "remoteport",		OPT_KEY,	NULL },
		{ "name",		OPT_KEYVAL,	NULL },
		{ "media",		OPT_KEYVAL,	NULL },
		{ NULL }
	};
	struct tipc_sup_media sup_media[] = {
		{ "udp",        "name",         cmd_bearer_get_udp_help},
		{ NULL, },
	};

	/* Rewind optind to include media in the option list */
	cmdl->optind--;
	if (parse_opts(opts, cmdl) < 0)
		return -EINVAL;

	if (!(opt = get_opt(opts, "media"))) {
		fprintf(stderr, "error, missing media value\n");
		return -EINVAL;
	}
	media = opt->val;

	if (help_flag) {
		cmd_bearer_get_udp_help(cmdl, media);
		return -EINVAL;
	}
	if (strcmp(media, "udp") != 0) {
		fprintf(stderr, "error, no \"%s\" media specific options\n", media);
		return -EINVAL;
	}
	if (!(opt = get_opt(opts, "name"))) {
		fprintf(stderr, "error, missing media name\n");
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_BEARER_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	attrs = mnl_attr_nest_start(nlh, TIPC_NLA_BEARER);
	err = nl_add_bearer_name(nlh, cmd, cmdl, opts, sup_media);
	if (err)
		return err;
	mnl_attr_nest_end(nlh, attrs);
	cb_data.nlh = nlh;

	if (has_opt(opts, "localip")) {
		cb_data.attr = TIPC_NLA_UDP_LOCAL;
		cb_data.prop = UDP_PROP_IP;
		return msg_doit(nlh, bearer_get_udp_cb, &cb_data);
	} else if (has_opt(opts, "localport")) {
		cb_data.attr = TIPC_NLA_UDP_LOCAL;
		cb_data.prop = UDP_PROP_PORT;
		return msg_doit(nlh, bearer_get_udp_cb, &cb_data);
	} else if (has_opt(opts, "remoteip")) {
		cb_data.attr = TIPC_NLA_UDP_REMOTE;
		cb_data.prop = UDP_PROP_IP;
		return msg_doit(nlh, bearer_get_udp_cb, &cb_data);
	} else if (has_opt(opts, "remoteport")) {
		cb_data.attr = TIPC_NLA_UDP_REMOTE;
		cb_data.prop = UDP_PROP_PORT;
		return msg_doit(nlh, bearer_get_udp_cb, &cb_data);
	}
	fprintf(stderr, "error, missing UDP option\n");
	return -EINVAL;
}

static int cmd_bearer_get_prop(struct nlmsghdr *nlh, const struct cmd *cmd,
			       struct cmdl *cmdl, void *data)
{
	int err;
	int prop;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlattr *attrs;
	struct opt opts[] = {
		{ "device",		OPT_KEYVAL,	NULL },
		{ "media",		OPT_KEYVAL,	NULL },
		{ "name",		OPT_KEYVAL,	NULL },
		{ NULL }
	};
	struct tipc_sup_media sup_media[] = {
		{ "udp",        "name",         cmd_bearer_get_udp_help},
		{ "eth",        "device",       cmd_bearer_get_l2_help },
		{ "ib",         "device",       cmd_bearer_get_l2_help },
		{ NULL, },
	};

	if (help_flag) {
		(cmd->help)(cmdl);
		return -EINVAL;
	}

	if (strcmp(cmd->cmd, "priority") == 0)
		prop = TIPC_NLA_PROP_PRIO;
	else if ((strcmp(cmd->cmd, "tolerance") == 0))
		prop = TIPC_NLA_PROP_TOL;
	else if ((strcmp(cmd->cmd, "window") == 0))
		prop = TIPC_NLA_PROP_WIN;
	else if ((strcmp(cmd->cmd, "mtu") == 0))
		prop = TIPC_NLA_PROP_MTU;
	else
		return -EINVAL;

	if (parse_opts(opts, cmdl) < 0)
		return -EINVAL;

	if (prop == TIPC_NLA_PROP_MTU) {
		char *media = cmd_get_media_type(cmd, cmdl, opts);

		if (!media)
			return -EINVAL;
		else if (strcmp(media, "udp")) {
			fprintf(stderr, "error, not supported for media\n");
			return -EINVAL;
		}
	}

	if (!(nlh = msg_init(buf, TIPC_NL_BEARER_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	attrs = mnl_attr_nest_start(nlh, TIPC_NLA_BEARER);
	err = nl_add_bearer_name(nlh, cmd, cmdl, opts, sup_media);
	if (err)
		return err;
	mnl_attr_nest_end(nlh, attrs);

	return msg_doit(nlh, bearer_get_cb, &prop);
}

static int cmd_bearer_get(struct nlmsghdr *nlh, const struct cmd *cmd,
			  struct cmdl *cmdl, void *data)
{
	const struct cmd cmds[] = {
		{ "priority",	cmd_bearer_get_prop,	cmd_bearer_get_help },
		{ "tolerance",	cmd_bearer_get_prop,	cmd_bearer_get_help },
		{ "window",	cmd_bearer_get_prop,	cmd_bearer_get_help },
		{ "mtu",	cmd_bearer_get_prop,	cmd_bearer_get_help },
		{ "media",	cmd_bearer_get_media,	cmd_bearer_get_help },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}

static int bearer_list_cb(const struct nlmsghdr *nlh, void *data)
{
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_BEARER_MAX + 1] = {};

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, info);
	if (!info[TIPC_NLA_BEARER]) {
		fprintf(stderr, "No bearer in netlink response\n");
		return MNL_CB_ERROR;
	}

	mnl_attr_parse_nested(info[TIPC_NLA_BEARER], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_BEARER_NAME]) {
		fprintf(stderr, "Bearer name missing in netlink response\n");
		return MNL_CB_ERROR;
	}

	printf("%s\n", mnl_attr_get_str(attrs[TIPC_NLA_BEARER_NAME]));

	return MNL_CB_OK;
}

static int cmd_bearer_list(struct nlmsghdr *nlh, const struct cmd *cmd,
			   struct cmdl *cmdl, void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];

	if (help_flag) {
		fprintf(stderr, "Usage: %s bearer list\n", cmdl->argv[0]);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_BEARER_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	return msg_dumpit(nlh, bearer_list_cb, NULL);
}

void cmd_bearer_help(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Usage: %s bearer COMMAND [ARGS] ...\n"
		"\n"
		"COMMANDS\n"
		" add			- Add data to existing bearer\n"
		" enable		- Enable a bearer\n"
		" disable		- Disable a bearer\n"
		" set			- Set various bearer properties\n"
		" get			- Get various bearer properties\n"
		" list			- List bearers\n", cmdl->argv[0]);
}

int cmd_bearer(struct nlmsghdr *nlh, const struct cmd *cmd, struct cmdl *cmdl,
	       void *data)
{
	const struct cmd cmds[] = {
		{ "add",	cmd_bearer_add,		cmd_bearer_add_help },
		{ "disable",	cmd_bearer_disable,	cmd_bearer_disable_help },
		{ "enable",	cmd_bearer_enable,	cmd_bearer_enable_help },
		{ "get",	cmd_bearer_get,		cmd_bearer_get_help },
		{ "list",	cmd_bearer_list,	NULL },
		{ "set",	cmd_bearer_set,		cmd_bearer_set_help },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}
