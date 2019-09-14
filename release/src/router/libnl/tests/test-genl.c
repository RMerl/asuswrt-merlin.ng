#include <netlink/cli/utils.h>
#include <linux/taskstats.h>

static struct nla_policy attr_policy[TASKSTATS_TYPE_MAX+1] = {
	[TASKSTATS_TYPE_PID]	= { .type = NLA_U32 },
	[TASKSTATS_TYPE_TGID]	= { .type = NLA_U32 },
	[TASKSTATS_TYPE_STATS]	= { .minlen = sizeof(struct taskstats) },
	[TASKSTATS_TYPE_AGGR_PID] = { .type = NLA_NESTED },
	[TASKSTATS_TYPE_AGGR_TGID] = { .type = NLA_NESTED },
};


static int parse_cmd_new(struct nl_cache_ops *unused, struct genl_cmd *cmd,
			 struct genl_info *info, void *arg)
{
	struct nlattr *attrs[TASKSTATS_TYPE_MAX+1];
	struct nlattr *nested;
	int err;

	if (info->attrs[TASKSTATS_TYPE_AGGR_PID])
		nested = info->attrs[TASKSTATS_TYPE_AGGR_PID];
	else if (info->attrs[TASKSTATS_TYPE_AGGR_TGID])
		nested = info->attrs[TASKSTATS_TYPE_AGGR_TGID];
	else {
		fprintf(stderr, "Invalid taskstats message: Unable to find "
				"nested attribute/\n");
		return NL_SKIP;
	}

	err = nla_parse_nested(attrs, TASKSTATS_TYPE_MAX, nested, attr_policy);
	if (err < 0) {
		nl_perror(err, "Error while parsing generic netlink message");
		return err;
	}


	if (attrs[TASKSTATS_TYPE_STATS]) {
		struct taskstats *stats = nla_data(attrs[TASKSTATS_TYPE_STATS]);

		printf("%s pid %u uid %u gid %u parent %u\n",
		       stats->ac_comm, stats->ac_pid, stats->ac_uid,
		       stats->ac_gid, stats->ac_ppid);
	}

	return 0;
}

static int parse_cb(struct nl_msg *msg, void *arg)
{
	return genl_handle_msg(msg, NULL);
}

static struct genl_cmd cmds[] = {
	{
		.c_id		= TASKSTATS_CMD_NEW,
		.c_name		= "taskstats_new()",
		.c_maxattr	= TASKSTATS_TYPE_MAX,
		.c_attr_policy	= attr_policy,
		.c_msg_parser	= &parse_cmd_new,
	},
};

#define ARRAY_SIZE(X) (sizeof(X) / sizeof((X)[0]))

static struct genl_ops ops = {
	.o_name = TASKSTATS_GENL_NAME,
	.o_cmds = cmds,
	.o_ncmds = ARRAY_SIZE(cmds),
};

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct nl_msg *msg;
	void *hdr;
	int err;

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_GENERIC);

	if ((err = genl_register_family(&ops)) < 0)
		nl_cli_fatal(err, "Unable to register Generic Netlink family");

	if ((err = genl_ops_resolve(sock, &ops)) < 0)
		nl_cli_fatal(err, "Unable to resolve family name");

	if (genl_ctrl_resolve(sock, "nlctrl") != GENL_ID_CTRL)
		nl_cli_fatal(NLE_INVAL, "Resolving of \"nlctrl\" failed");

	msg = nlmsg_alloc();
	if (msg == NULL)
		nl_cli_fatal(NLE_NOMEM, "Unable to allocate netlink message");

	hdr = genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, ops.o_id,
			  0, 0, TASKSTATS_CMD_GET, TASKSTATS_GENL_VERSION);
	if (hdr == NULL)
		nl_cli_fatal(ENOMEM, "Unable to write genl header");

	if ((err = nla_put_u32(msg, TASKSTATS_CMD_ATTR_PID, 1)) < 0)
		nl_cli_fatal(err, "Unable to add attribute: %s", nl_geterror(err));

	if ((err = nl_send_auto_complete(sock, msg)) < 0)
		nl_cli_fatal(err, "Unable to send message: %s", nl_geterror(err));

	nlmsg_free(msg);

	if ((err = nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM,
			parse_cb, NULL)) < 0)
		nl_cli_fatal(err, "Unable to modify valid message callback");

	if ((err = nl_recvmsgs_default(sock)) < 0)
		nl_cli_fatal(err, "Unable to receive message: %s", nl_geterror(err));

	nl_close(sock);
	nl_socket_free(sock);

	return 0;
}
