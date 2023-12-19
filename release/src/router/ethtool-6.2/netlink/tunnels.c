/*
 * tunnel.c - device tunnel information
 *
 * Implementation of "ethtool --show-tunnels <dev>"
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "../common.h"
#include "../internal.h"

#include "bitset.h"
#include "netlink.h"
#include "strset.h"

/* TUNNEL_INFO_GET */

static int
tunnel_info_strings_load(struct nl_context *nlctx,
			 const struct stringset **strings)
{
	int ret;

	if (*strings)
		return 0;
	ret = netlink_init_ethnl2_socket(nlctx);
	if (ret < 0)
		return ret;
	*strings = global_stringset(ETH_SS_UDP_TUNNEL_TYPES,
				    nlctx->ethnl2_socket);
	return 0;
}

static int
tunnel_info_dump_udp_entry(struct nl_context *nlctx, const struct nlattr *entry,
			   const struct stringset **strings)
{
	const struct nlattr *entry_tb[ETHTOOL_A_TUNNEL_UDP_ENTRY_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(entry_tb);
	const struct nlattr *attr;
	unsigned int port, type;
	int ret;

	if (tunnel_info_strings_load(nlctx, strings))
		return 1;

	ret = mnl_attr_parse_nested(entry, attr_cb, &entry_tb_info);
	if (ret < 0) {
		fprintf(stderr, "malformed netlink message (udp entry)\n");
		return 1;
	}

	attr = entry_tb[ETHTOOL_A_TUNNEL_UDP_ENTRY_PORT];
	if (!attr || mnl_attr_validate(attr, MNL_TYPE_U16) < 0) {
		fprintf(stderr, "malformed netlink message (port)\n");
		return 1;
	}
	port = ntohs(mnl_attr_get_u16(attr));

	attr = entry_tb[ETHTOOL_A_TUNNEL_UDP_ENTRY_TYPE];
	if (!attr || mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
		fprintf(stderr, "malformed netlink message (tunnel type)\n");
		return 1;
	}
	type = mnl_attr_get_u32(attr);

	printf("        port %d, %s\n", port, get_string(*strings, type));

	return 0;
}

static void tunnel_info_dump_cb(unsigned int idx, const char *name, bool val,
				void *data)
{
	bool *first = data;

	if (!val)
		return;

	if (!*first)
		putchar(',');
	*first = false;

	if (name)
		printf(" %s", name);
	else
		printf(" bit%u", idx);
}

static int
tunnel_info_dump_list(struct nl_context *nlctx, const struct nlattr *attr,
		      const struct stringset **strings)
{
	bool first = true;
	int ret;

	if (bitset_is_empty(attr, false, &ret) || ret) {
		if (ret)
			return 1;

		printf("    Types: none (static entries)\n");
		return 0;
	}

	if (bitset_is_compact(attr) &&
	    tunnel_info_strings_load(nlctx, strings))
		return 1;

	printf("    Types:");
	ret = walk_bitset(attr, *strings, tunnel_info_dump_cb, &first);
	putchar('\n');
	return ret;
}

static int
tunnel_info_dump_udp_table(const struct nlattr *table, struct nl_context *nlctx,
			   const struct stringset **strings)
{
	const struct nlattr *table_tb[ETHTOOL_A_TUNNEL_UDP_TABLE_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(table_tb);
	const struct nlattr *attr;
	int i, ret;

	ret = mnl_attr_parse_nested(table, attr_cb, &table_tb_info);
	if (ret < 0) {
		fprintf(stderr, "malformed netlink message (udp table)\n");
		return 1;
	}

	attr = table_tb[ETHTOOL_A_TUNNEL_UDP_TABLE_SIZE];
	if (!attr || mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
		fprintf(stderr, "malformed netlink message (table size)\n");
		return 1;
	}
	printf("    Size: %d\n", mnl_attr_get_u32(attr));

	attr = table_tb[ETHTOOL_A_TUNNEL_UDP_TABLE_TYPES];
	if (!attr || tunnel_info_dump_list(nlctx, attr, strings)) {
		fprintf(stderr, "malformed netlink message (types)\n");
		return 1;
	}

	if (!table_tb[ETHTOOL_A_TUNNEL_UDP_TABLE_ENTRY]) {
		printf("    No entries\n");
		return 0;
	}

	i = 0;
	mnl_attr_for_each_nested(attr, table) {
		if (mnl_attr_get_type(attr) == ETHTOOL_A_TUNNEL_UDP_TABLE_ENTRY)
			i++;
	}

	printf("    Entries (%d):\n", i++);
	mnl_attr_for_each_nested(attr, table) {
		if (mnl_attr_get_type(attr) == ETHTOOL_A_TUNNEL_UDP_TABLE_ENTRY)
			if (tunnel_info_dump_udp_entry(nlctx, attr, strings))
				return 1;
	}

	return 0;
}

static int
tunnel_info_dump_udp(const struct nlattr *udp_ports, struct nl_context *nlctx)
{
	const struct stringset *strings = NULL;
	const struct nlattr *table;
	int i, err;

	i = 0;
	mnl_attr_for_each_nested(table, udp_ports) {
		if (mnl_attr_get_type(table) != ETHTOOL_A_TUNNEL_UDP_TABLE)
			continue;

		printf("  UDP port table %d: \n", i++);
		err = tunnel_info_dump_udp_table(table, nlctx, &strings);
		if (err)
			return err;
	}

	return 0;
}

static int tunnel_info_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_TUNNEL_INFO_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	const struct nlattr *udp_ports;
	struct nl_context *nlctx = data;
	bool silent;
	int err_ret;
	int ret;

	silent = nlctx->is_dump;
	err_ret = silent ? MNL_CB_OK : MNL_CB_ERROR;
	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return err_ret;
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_TUNNEL_INFO_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	printf("Tunnel information for %s:\n", nlctx->devname);

	udp_ports = tb[ETHTOOL_A_TUNNEL_INFO_UDP_PORTS];
	if (udp_ports)
		if (tunnel_info_dump_udp(udp_ports, nlctx))
			return err_ret;

	return MNL_CB_OK;
}

int nl_gtunnels(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_TUNNEL_INFO_GET, true))
		return -EOPNOTSUPP;
	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_TUNNEL_INFO_GET,
				      ETHTOOL_A_TUNNEL_INFO_HEADER, 0);
	if (ret < 0)
		return ret;
	return nlsock_send_get_request(nlsk, tunnel_info_reply_cb);
}
