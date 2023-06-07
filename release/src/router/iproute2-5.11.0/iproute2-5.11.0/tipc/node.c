/*
 * node.c	TIPC node functionality.
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
#include <errno.h>

#include <linux/tipc_netlink.h>
#include <linux/tipc.h>
#include <linux/genetlink.h>
#include <libmnl/libmnl.h>

#include "cmdl.h"
#include "msg.h"
#include "misc.h"
#include "node.h"

static int node_list_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_NODE_MAX + 1] = {};
	char str[33] = {};
	uint32_t addr;

	mnl_attr_parse(nlh, sizeof(struct genlmsghdr), parse_attrs, info);
	if (!info[TIPC_NLA_NODE])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_NODE], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_NODE_ADDR])
		return MNL_CB_ERROR;

	addr = mnl_attr_get_u32(attrs[TIPC_NLA_NODE_ADDR]);
	hash2nodestr(addr, str);
	printf("%-32s %08x ", str, addr);
	if (attrs[TIPC_NLA_NODE_UP])
		printf("up\n");
	else
		printf("down\n");
	return MNL_CB_OK;
}

static int cmd_node_list(struct nlmsghdr *nlh, const struct cmd *cmd,
			 struct cmdl *cmdl, void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];

	if (help_flag) {
		fprintf(stderr, "Usage: %s node list\n", cmdl->argv[0]);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_NODE_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}
	printf("Node Identity                    Hash     State\n");
	return msg_dumpit(nlh, node_list_cb, NULL);
}

static int cmd_node_set_addr(struct nlmsghdr *nlh, const struct cmd *cmd,
			     struct cmdl *cmdl, void *data)
{
	char *str;
	uint32_t addr;
	struct nlattr *nest;
	char buf[MNL_SOCKET_BUFFER_SIZE];

	if (cmdl->argc != cmdl->optind + 1) {
		fprintf(stderr, "Usage: %s node set address ADDRESS\n",
			cmdl->argv[0]);
		return -EINVAL;
	}

	str = shift_cmdl(cmdl);
	addr = str2addr(str);
	if (!addr)
		return -1;

	if (!(nlh = msg_init(buf, TIPC_NL_NET_SET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	nest = mnl_attr_nest_start(nlh, TIPC_NLA_NET);
	mnl_attr_put_u32(nlh, TIPC_NLA_NET_ADDR, addr);
	mnl_attr_nest_end(nlh, nest);

	return msg_doit(nlh, NULL, NULL);
}

static int cmd_node_get_addr(struct nlmsghdr *nlh, const struct cmd *cmd,
			     struct cmdl *cmdl, void *data)
{
	int sk;
	socklen_t sz = sizeof(struct sockaddr_tipc);
	struct sockaddr_tipc addr;

	sk = socket(AF_TIPC, SOCK_RDM, 0);
	if (sk < 0) {
		fprintf(stderr, "opening TIPC socket: %s\n", strerror(errno));
		return -1;
	}

	if (getsockname(sk, (struct sockaddr *)&addr, &sz) < 0) {
		fprintf(stderr, "getting TIPC socket address: %s\n",
			strerror(errno));
		close(sk);
		return -1;
	}
	close(sk);

	printf("%08x\n", addr.addr.id.node);
	return 0;
}

static int cmd_node_set_nodeid(struct nlmsghdr *nlh, const struct cmd *cmd,
			       struct cmdl *cmdl, void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	uint8_t id[16] = {0,};
	uint64_t *w0 = (uint64_t *) &id[0];
	uint64_t *w1 = (uint64_t *) &id[8];
	struct nlattr *nest;
	char *str;

	if (cmdl->argc != cmdl->optind + 1) {
		fprintf(stderr, "Usage: %s node set nodeid NODE_ID\n",
			cmdl->argv[0]);
		return -EINVAL;
	}

	str = shift_cmdl(cmdl);
	if (str2nodeid(str, id)) {
		fprintf(stderr, "Invalid node identity\n");
		return -EINVAL;
	}

	nlh = msg_init(buf, TIPC_NL_NET_SET);
	if (!nlh) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}
	nest = mnl_attr_nest_start(nlh, TIPC_NLA_NET);
	mnl_attr_put_u64(nlh, TIPC_NLA_NET_NODEID, *w0);
	mnl_attr_put_u64(nlh, TIPC_NLA_NET_NODEID_W1, *w1);
	mnl_attr_nest_end(nlh, nest);
	return msg_doit(nlh, NULL, NULL);
}

static void cmd_node_set_key_help(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Usage: %s node set key KEY [algname ALGNAME] [PROPERTIES]\n"
		"       %s node set key rekeying REKEYING\n\n"
		"KEY\n"
		"  Symmetric KEY & SALT as a composite ASCII or hex string (0x...) in form:\n"
		"  [KEY: 16, 24 or 32 octets][SALT: 4 octets]\n\n"
		"ALGNAME\n"
		"  Cipher algorithm [default: \"gcm(aes)\"]\n\n"
		"PROPERTIES\n"
		"  master                - Set KEY as a cluster master key\n"
		"  <empty>               - Set KEY as a cluster key\n"
		"  nodeid NODEID         - Set KEY as a per-node key for own or peer\n\n"
		"REKEYING\n"
		"  INTERVAL              - Set rekeying interval (in minutes) [0: disable]\n"
		"  now                   - Trigger one (first) rekeying immediately\n\n"
		"EXAMPLES\n"
		"  %s node set key this_is_a_master_key master\n"
		"  %s node set key 0x746869735F69735F615F6B657931365F73616C74\n"
		"  %s node set key this_is_a_key16_salt algname \"gcm(aes)\" nodeid 1001002\n"
		"  %s node set key rekeying 600\n\n",
		cmdl->argv[0], cmdl->argv[0], cmdl->argv[0], cmdl->argv[0],
		cmdl->argv[0], cmdl->argv[0]);
}

static int cmd_node_set_key(struct nlmsghdr *nlh, const struct cmd *cmd,
			    struct cmdl *cmdl, void *data)
{
	struct {
		union {
			struct tipc_aead_key key;
			char mem[TIPC_AEAD_KEY_SIZE_MAX];
		};
	} input = {};
	struct opt opts[] = {
		{ "algname",	OPT_KEYVAL,	NULL },
		{ "nodeid",	OPT_KEYVAL,	NULL },
		{ "master",	OPT_KEY,	NULL },
		{ "rekeying",	OPT_KEYVAL,	NULL },
		{ NULL }
	};
	struct nlattr *nest;
	struct opt *opt_algname, *opt_nodeid, *opt_master, *opt_rekeying;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	uint8_t id[TIPC_NODEID_LEN] = {0,};
	uint32_t rekeying = 0;
	bool has_key = false;
	int keysize;
	char *str;

	if (help_flag || cmdl->optind >= cmdl->argc) {
		(cmd->help)(cmdl);
		return -EINVAL;
	}

	/* Check if command starts with opts i.e. "rekeying" opt without key */
	if (find_opt(opts, cmdl->argv[cmdl->optind]))
		goto get_ops;

	/* Get user key */
	has_key = true;
	str = shift_cmdl(cmdl);
	if (str2key(str, &input.key)) {
		fprintf(stderr, "error, invalid key input\n");
		return -EINVAL;
	}

get_ops:
	if (parse_opts(opts, cmdl) < 0)
		return -EINVAL;

	/* Get rekeying time */
	opt_rekeying = get_opt(opts, "rekeying");
	if (opt_rekeying) {
		if (!strcmp(opt_rekeying->val, "now"))
			rekeying = TIPC_REKEYING_NOW;
		else
			rekeying = atoi(opt_rekeying->val);
	}

	/* Get algorithm name, default: "gcm(aes)" */
	opt_algname = get_opt(opts, "algname");
	if (!opt_algname)
		strcpy(input.key.alg_name, "gcm(aes)");
	else
		strcpy(input.key.alg_name, opt_algname->val);

	/* Get node identity */
	opt_nodeid = get_opt(opts, "nodeid");
	if (opt_nodeid && str2nodeid(opt_nodeid->val, id)) {
		fprintf(stderr, "error, invalid node identity\n");
		return -EINVAL;
	}

	/* Get master key indication */
	opt_master = get_opt(opts, "master");

	/* Sanity check if wrong option */
	if (opt_nodeid && opt_master) {
		fprintf(stderr, "error, per-node key cannot be master\n");
		return -EINVAL;
	}

	/* Init & do the command */
	nlh = msg_init(buf, TIPC_NL_KEY_SET);
	if (!nlh) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	nest = mnl_attr_nest_start(nlh, TIPC_NLA_NODE);
	if (has_key) {
		keysize = tipc_aead_key_size(&input.key);
		mnl_attr_put(nlh, TIPC_NLA_NODE_KEY, keysize, &input.key);
		if (opt_nodeid)
			mnl_attr_put(nlh, TIPC_NLA_NODE_ID, TIPC_NODEID_LEN, id);
		if (opt_master)
			mnl_attr_put(nlh, TIPC_NLA_NODE_KEY_MASTER, 0, NULL);
	}
	if (opt_rekeying)
		mnl_attr_put_u32(nlh, TIPC_NLA_NODE_REKEYING, rekeying);

	mnl_attr_nest_end(nlh, nest);
	return msg_doit(nlh, NULL, NULL);
}

static int cmd_node_flush_key(struct nlmsghdr *nlh, const struct cmd *cmd,
			      struct cmdl *cmdl, void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];

	if (help_flag) {
		(cmd->help)(cmdl);
		return -EINVAL;
	}

	/* Init & do the command */
	nlh = msg_init(buf, TIPC_NL_KEY_FLUSH);
	if (!nlh) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}
	return msg_doit(nlh, NULL, NULL);
}

static int nodeid_get_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_NET_MAX + 1] = {};
	char str[33] = {0,};
	uint8_t id[16] = {0,};
	uint64_t *w0 = (uint64_t *) &id[0];
	uint64_t *w1 = (uint64_t *) &id[8];

	mnl_attr_parse(nlh, sizeof(struct genlmsghdr), parse_attrs, info);
	if (!info[TIPC_NLA_NET])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_NET], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_NET_ID])
		return MNL_CB_ERROR;

	*w0 = mnl_attr_get_u64(attrs[TIPC_NLA_NET_NODEID]);
	*w1 = mnl_attr_get_u64(attrs[TIPC_NLA_NET_NODEID_W1]);
	nodeid2str(id, str);
	printf("Node Identity                    Hash\n");
	printf("%-33s", str);
	cmd_node_get_addr(NULL, NULL, NULL, NULL);
	return MNL_CB_OK;
}

static int cmd_node_get_nodeid(struct nlmsghdr *nlh, const struct cmd *cmd,
			       struct cmdl *cmdl, void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];

	if (help_flag) {
		(cmd->help)(cmdl);
		return -EINVAL;
	}

	nlh = msg_init(buf, TIPC_NL_NET_GET);
	if (!nlh) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	return msg_dumpit(nlh, nodeid_get_cb, NULL);
}


static int netid_get_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *info[TIPC_NLA_MAX + 1] = {};
	struct nlattr *attrs[TIPC_NLA_NET_MAX + 1] = {};

	mnl_attr_parse(nlh, sizeof(struct genlmsghdr), parse_attrs, info);
	if (!info[TIPC_NLA_NET])
		return MNL_CB_ERROR;

	mnl_attr_parse_nested(info[TIPC_NLA_NET], parse_attrs, attrs);
	if (!attrs[TIPC_NLA_NET_ID])
		return MNL_CB_ERROR;

	printf("%u\n", mnl_attr_get_u32(attrs[TIPC_NLA_NET_ID]));

	return MNL_CB_OK;
}

static int cmd_node_get_netid(struct nlmsghdr *nlh, const struct cmd *cmd,
			      struct cmdl *cmdl, void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];

	if (help_flag) {
		(cmd->help)(cmdl);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_NET_GET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	return msg_dumpit(nlh, netid_get_cb, NULL);
}

static int cmd_node_set_netid(struct nlmsghdr *nlh, const struct cmd *cmd,
			      struct cmdl *cmdl, void *data)
{
	int netid;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlattr *nest;

	if (help_flag) {
		(cmd->help)(cmdl);
		return -EINVAL;
	}

	if (!(nlh = msg_init(buf, TIPC_NL_NET_SET))) {
		fprintf(stderr, "error, message initialisation failed\n");
		return -1;
	}

	if (cmdl->argc != cmdl->optind + 1) {
		fprintf(stderr, "Usage: %s node set netid NETID\n",
			cmdl->argv[0]);
		return -EINVAL;
	}
	netid = atoi(shift_cmdl(cmdl));

	nest = mnl_attr_nest_start(nlh, TIPC_NLA_NET);
	mnl_attr_put_u32(nlh, TIPC_NLA_NET_ID, netid);
	mnl_attr_nest_end(nlh, nest);

	return msg_doit(nlh, NULL, NULL);
}

static void cmd_node_flush_help(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Usage: %s node flush PROPERTY\n\n"
		"PROPERTIES\n"
		" key                   - Flush all symmetric-keys\n",
		cmdl->argv[0]);
}

static int cmd_node_flush(struct nlmsghdr *nlh, const struct cmd *cmd,
			  struct cmdl *cmdl, void *data)
{
	const struct cmd cmds[] = {
		{ "key",        cmd_node_flush_key,     NULL },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}

static void cmd_node_set_help(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Usage: %s node set PROPERTY\n\n"
		"PROPERTIES\n"
		" identity NODEID       - Set node identity\n"
		" clusterid CLUSTERID   - Set local cluster id\n"
		" key PROPERTY          - Set symmetric-key\n",
		cmdl->argv[0]);
}

static int cmd_node_set(struct nlmsghdr *nlh, const struct cmd *cmd,
			struct cmdl *cmdl, void *data)
{
	const struct cmd cmds[] = {
		{ "address",    cmd_node_set_addr,      NULL },
		{ "identity",	cmd_node_set_nodeid,	NULL },
		{ "netid",	cmd_node_set_netid,	NULL },
		{ "clusterid",	cmd_node_set_netid,	NULL },
		{ "key",	cmd_node_set_key,	cmd_node_set_key_help },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}

static void cmd_node_get_help(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Usage: %s node get PROPERTY\n\n"
		"PROPERTIES\n"
		" identity              - Get node identity\n"
		" clusterid             - Get local clusterid\n",
		cmdl->argv[0]);
}

static int cmd_node_get(struct nlmsghdr *nlh, const struct cmd *cmd,
			struct cmdl *cmdl, void *data)
{
	const struct cmd cmds[] = {
		{ "address",	cmd_node_get_addr,	NULL },
		{ "identity",	cmd_node_get_nodeid,	NULL },
		{ "netid",	cmd_node_get_netid,	NULL },
		{ "clusterid",	cmd_node_get_netid,	NULL },
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}

void cmd_node_help(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Usage: %s node COMMAND [ARGS] ...\n\n"
		"COMMANDS\n"
		" list                  - List remote nodes\n"
		" get                   - Get local node parameters\n"
		" set                   - Set local node parameters\n"
		" flush                 - Flush local node parameters\n",
		cmdl->argv[0]);
}

int cmd_node(struct nlmsghdr *nlh, const struct cmd *cmd, struct cmdl *cmdl,
	     void *data)
{
	const struct cmd cmds[] = {
		{ "list",	cmd_node_list,	NULL },
		{ "get",	cmd_node_get,	cmd_node_get_help },
		{ "set",	cmd_node_set,	cmd_node_set_help },
		{ "flush",	cmd_node_flush, cmd_node_flush_help},
		{ NULL }
	};

	return run_cmd(nlh, cmd, cmds, cmdl, NULL);
}
