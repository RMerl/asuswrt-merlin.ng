/*
 * netlink.c - basic infrastructure for netlink code
 *
 * Heart of the netlink interface implementation.
 */

#include <errno.h>

#include "../internal.h"
#include "netlink.h"
#include "extapi.h"
#include "msgbuff.h"
#include "nlsock.h"
#include "strset.h"

/* Used as reply callback for requests where no reply is expected (e.g. most
 * "set" type commands)
 */
int nomsg_reply_cb(const struct nlmsghdr *nlhdr, void *data __maybe_unused)
{
	const struct genlmsghdr *ghdr = (const struct genlmsghdr *)(nlhdr + 1);

	fprintf(stderr, "received unexpected message: len=%u type=%u cmd=%u\n",
		nlhdr->nlmsg_len, nlhdr->nlmsg_type, ghdr->cmd);
	return MNL_CB_OK;
}

/* standard attribute parser callback; it fills provided array with pointers
 * to attributes like kernel nla_parse(). We must expect to run on top of
 * a newer kernel which may send attributes that we do not know (yet). Rather
 * than treating them as an error, just ignore them.
 */
int attr_cb(const struct nlattr *attr, void *data)
{
	const struct attr_tb_info *tb_info = data;
	uint16_t type = mnl_attr_get_type(attr);

	if (type <= tb_info->max_type)
		tb_info->tb[type] = attr;

	return MNL_CB_OK;
}

/* misc helpers */

const char *get_dev_name(const struct nlattr *nest)
{
	const struct nlattr *tb[ETHTOOL_A_HEADER_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	int ret;

	if (!nest)
		return NULL;
	ret = mnl_attr_parse_nested(nest, attr_cb, &tb_info);
	if (ret < 0 || !tb[ETHTOOL_A_HEADER_DEV_NAME])
		return "(none)";
	return mnl_attr_get_str(tb[ETHTOOL_A_HEADER_DEV_NAME]);
}

int get_dev_info(const struct nlattr *nest, int *ifindex, char *ifname)
{
	const struct nlattr *tb[ETHTOOL_A_HEADER_MAX + 1] = {};
	const struct nlattr *index_attr;
	const struct nlattr *name_attr;
	DECLARE_ATTR_TB_INFO(tb);
	int ret;

	if (ifindex)
		*ifindex = 0;
	if (ifname)
		memset(ifname, '\0', ALTIFNAMSIZ);

	if (!nest)
		return -EFAULT;
	ret = mnl_attr_parse_nested(nest, attr_cb, &tb_info);
	index_attr = tb[ETHTOOL_A_HEADER_DEV_INDEX];
	name_attr = tb[ETHTOOL_A_HEADER_DEV_NAME];
	if (ret < 0 || (ifindex && !index_attr) || (ifname && !name_attr))
		return -EFAULT;

	if (ifindex)
		*ifindex = mnl_attr_get_u32(index_attr);
	if (ifname) {
		strncpy(ifname, mnl_attr_get_str(name_attr), ALTIFNAMSIZ);
		if (ifname[ALTIFNAMSIZ - 1]) {
			ifname[ALTIFNAMSIZ - 1] = '\0';
			fprintf(stderr, "kernel device name too long: '%s'\n",
				mnl_attr_get_str(name_attr));
			return -EFAULT;
		}
	}
	return 0;
}

/**
 * netlink_cmd_check() - check support for netlink command
 * @ctx:            ethtool command context
 * @cmd:            netlink command id
 * @devname:        device name from user
 * @allow_wildcard: wildcard dumps supported
 *
 * Check if command @cmd is known to be unsupported based on ops information
 * from genetlink family id request. Set nlctx->ioctl_fallback if ethtool
 * should fall back to ioctl, i.e. when we do not know in advance that
 * netlink request is supported. Set nlctx->wildcard_unsupported if "*" was
 * used as device name but the request does not support wildcards (on either
 * side).
 *
 * Return: true if we know the netlink request is not supported and should
 * fail (and possibly fall back) without actually sending it to kernel.
 */
bool netlink_cmd_check(struct cmd_context *ctx, unsigned int cmd,
		       bool allow_wildcard)
{
	bool is_dump = !strcmp(ctx->devname, WILDCARD_DEVNAME);
	uint32_t cap = is_dump ? GENL_CMD_CAP_DUMP : GENL_CMD_CAP_DO;
	struct nl_context *nlctx = ctx->nlctx;

	if (is_dump && !allow_wildcard) {
		nlctx->wildcard_unsupported = true;
		return true;
	}
	if (!nlctx->ops_info) {
		nlctx->ioctl_fallback = true;
		return false;
	}
	if (cmd > ETHTOOL_MSG_USER_MAX || !nlctx->ops_info[cmd].op_flags) {
		nlctx->ioctl_fallback = true;
		return true;
	}

	if (is_dump && !(nlctx->ops_info[cmd].op_flags & GENL_CMD_CAP_DUMP))
		nlctx->wildcard_unsupported = true;

	return !(nlctx->ops_info[cmd].op_flags & cap);
}

struct ethtool_op_policy_query_ctx {
	struct nl_context *nlctx;
	unsigned int op;
	unsigned int op_hdr_attr;

	bool op_policy_found;
	bool hdr_policy_found;
	unsigned int op_policy_idx;
	unsigned int hdr_policy_idx;
	uint64_t flag_mask;
};

static int family_policy_find_op(struct ethtool_op_policy_query_ctx *policy_ctx,
				 const struct nlattr *op_policy)
{
	const struct nlattr *attr;
	unsigned int type;
	int ret;

	type = policy_ctx->nlctx->is_dump ?
		CTRL_ATTR_POLICY_DUMP : CTRL_ATTR_POLICY_DO;

	mnl_attr_for_each_nested(attr, op_policy) {
		const struct nlattr *tb[CTRL_ATTR_POLICY_DUMP_MAX + 1] = {};
		DECLARE_ATTR_TB_INFO(tb);

		if (mnl_attr_get_type(attr) != policy_ctx->op)
			continue;

		ret = mnl_attr_parse_nested(attr, attr_cb, &tb_info);
		if (ret < 0)
			return ret;

		if (!tb[type])
			continue;

		policy_ctx->op_policy_found = true;
		policy_ctx->op_policy_idx = mnl_attr_get_u32(tb[type]);
		break;
	}

	return 0;
}

static int family_policy_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tba[NL_POLICY_TYPE_ATTR_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tba);
	const struct nlattr *tb[CTRL_ATTR_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	struct ethtool_op_policy_query_ctx *policy_ctx = data;
	const struct nlattr *policy_attr, *attr_attr, *attr;
	unsigned int attr_idx, policy_idx;
	int ret;

	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return MNL_CB_ERROR;

	if (!policy_ctx->op_policy_found) {
		if (!tb[CTRL_ATTR_OP_POLICY]) {
			fprintf(stderr, "Error: op policy map not present\n");
			return MNL_CB_ERROR;
		}
		ret = family_policy_find_op(policy_ctx, tb[CTRL_ATTR_OP_POLICY]);
		return ret < 0 ? MNL_CB_ERROR : MNL_CB_OK;
	}

	if (!tb[CTRL_ATTR_POLICY])
		return MNL_CB_OK;

	policy_attr = mnl_attr_get_payload(tb[CTRL_ATTR_POLICY]);
	policy_idx = mnl_attr_get_type(policy_attr);
	attr_attr = mnl_attr_get_payload(policy_attr);
	attr_idx = mnl_attr_get_type(attr_attr);

	ret = mnl_attr_parse_nested(attr_attr, attr_cb, &tba_info);
	if (ret < 0)
		return MNL_CB_ERROR;

	if (policy_idx == policy_ctx->op_policy_idx &&
	    attr_idx == policy_ctx->op_hdr_attr) {
		attr = tba[NL_POLICY_TYPE_ATTR_POLICY_IDX];
		if (!attr) {
			fprintf(stderr,	"Error: no policy index in what was expected to be ethtool header attribute\n");
			return MNL_CB_ERROR;
		}
		policy_ctx->hdr_policy_found = true;
		policy_ctx->hdr_policy_idx = mnl_attr_get_u32(attr);
	}

	if (policy_ctx->hdr_policy_found &&
	    policy_ctx->hdr_policy_idx == policy_idx &&
	    attr_idx == ETHTOOL_A_HEADER_FLAGS) {
		attr = tba[NL_POLICY_TYPE_ATTR_MASK];
		if (!attr) {
			fprintf(stderr,	"Error: validation mask not reported for ethtool header flags\n");
			return MNL_CB_ERROR;
		}

		policy_ctx->flag_mask = mnl_attr_get_u64(attr);
	}

	return MNL_CB_OK;
}

static int read_flags_policy(struct nl_context *nlctx, struct nl_socket *nlsk,
			     unsigned int nlcmd, unsigned int hdrattr)
{
	struct ethtool_op_policy_query_ctx policy_ctx;
	struct nl_msg_buff *msgbuff = &nlsk->msgbuff;
	int ret;

	if (nlctx->ops_info[nlcmd].hdr_policy_loaded)
		return 0;

	memset(&policy_ctx, 0, sizeof(policy_ctx));
	policy_ctx.nlctx = nlctx;
	policy_ctx.op = nlcmd;
	policy_ctx.op_hdr_attr = hdrattr;

	ret = __msg_init(msgbuff, GENL_ID_CTRL, CTRL_CMD_GETPOLICY,
			 NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP, 1);
	if (ret < 0)
		return ret;
	ret = -EMSGSIZE;
	if (ethnla_put_u16(msgbuff, CTRL_ATTR_FAMILY_ID, nlctx->ethnl_fam))
		return ret;
	if (ethnla_put_u32(msgbuff, CTRL_ATTR_OP, nlcmd))
		return ret;

	nlsock_sendmsg(nlsk, NULL);
	nlsock_process_reply(nlsk, family_policy_cb, &policy_ctx);

	nlctx->ops_info[nlcmd].hdr_policy_loaded = 1;
	nlctx->ops_info[nlcmd].hdr_flags = policy_ctx.flag_mask;
	return 0;
}

u32 get_stats_flag(struct nl_context *nlctx, unsigned int nlcmd,
		   unsigned int hdrattr)
{
	if (!nlctx->ctx->show_stats)
		return 0;
	if (nlcmd > ETHTOOL_MSG_USER_MAX ||
	    !(nlctx->ops_info[nlcmd].op_flags & GENL_CMD_CAP_HASPOL))
		return 0;

	if (read_flags_policy(nlctx, nlctx->ethnl_socket, nlcmd, hdrattr) < 0)
		return 0;

	return nlctx->ops_info[nlcmd].hdr_flags & ETHTOOL_FLAG_STATS;
}

/* initialization */

static int genl_read_ops(struct nl_context *nlctx,
			 const struct nlattr *ops_attr)
{
	struct nl_op_info *ops_info;
	struct nlattr *op_attr;
	int ret;

	ops_info = calloc(__ETHTOOL_MSG_USER_CNT, sizeof(ops_info[0]));
	if (!ops_info)
		return -ENOMEM;

	mnl_attr_for_each_nested(op_attr, ops_attr) {
		const struct nlattr *tb[CTRL_ATTR_OP_MAX + 1] = {};
		DECLARE_ATTR_TB_INFO(tb);
		uint32_t op_id;

		ret = mnl_attr_parse_nested(op_attr, attr_cb, &tb_info);
		if (ret < 0)
			goto err;

		if (!tb[CTRL_ATTR_OP_ID] || !tb[CTRL_ATTR_OP_FLAGS])
			continue;
		op_id = mnl_attr_get_u32(tb[CTRL_ATTR_OP_ID]);
		if (op_id >= __ETHTOOL_MSG_USER_CNT)
			continue;

		ops_info[op_id].op_flags =
			mnl_attr_get_u32(tb[CTRL_ATTR_OP_FLAGS]);
	}

	nlctx->ops_info = ops_info;
	return 0;
err:
	free(ops_info);
	return ret;
}

static void find_mc_group(struct nl_context *nlctx, struct nlattr *nest)
{
	const struct nlattr *grp_tb[CTRL_ATTR_MCAST_GRP_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(grp_tb);
	struct nlattr *grp_attr;
	int ret;

	mnl_attr_for_each_nested(grp_attr, nest) {
		ret = mnl_attr_parse_nested(grp_attr, attr_cb, &grp_tb_info);
		if (ret < 0)
			return;
		if (!grp_tb[CTRL_ATTR_MCAST_GRP_NAME] ||
		    !grp_tb[CTRL_ATTR_MCAST_GRP_ID])
			continue;
		if (strcmp(mnl_attr_get_str(grp_tb[CTRL_ATTR_MCAST_GRP_NAME]),
			   ETHTOOL_MCGRP_MONITOR_NAME))
			continue;
		nlctx->ethnl_mongrp =
			mnl_attr_get_u32(grp_tb[CTRL_ATTR_MCAST_GRP_ID]);
		return;
	}
}

static int __maybe_unused family_info_cb(const struct nlmsghdr *nlhdr,
					 void *data)
{
	struct nl_context *nlctx = data;
	struct nlattr *attr;
	int ret;

	mnl_attr_for_each(attr, nlhdr, GENL_HDRLEN) {
		switch (mnl_attr_get_type(attr)) {
		case CTRL_ATTR_FAMILY_ID:
			nlctx->ethnl_fam = mnl_attr_get_u16(attr);
			break;
		case CTRL_ATTR_OPS:
			ret = genl_read_ops(nlctx, attr);
			if (ret < 0)
				return MNL_CB_ERROR;
			break;
		case CTRL_ATTR_MCAST_GROUPS:
			find_mc_group(nlctx, attr);
			break;
		}
	}

	return MNL_CB_OK;
}

#ifdef TEST_ETHTOOL
static int get_genl_family(struct nl_context *nlctx __maybe_unused,
			   struct nl_socket *nlsk __maybe_unused)
{
	return 0;
}
#else
static int get_genl_family(struct nl_context *nlctx, struct nl_socket *nlsk)
{
	struct nl_msg_buff *msgbuff = &nlsk->msgbuff;
	int ret;

	nlctx->suppress_nlerr = 2;
	ret = __msg_init(msgbuff, GENL_ID_CTRL, CTRL_CMD_GETFAMILY,
			 NLM_F_REQUEST | NLM_F_ACK, 1);
	if (ret < 0)
		goto out;
	ret = -EMSGSIZE;
	if (ethnla_put_strz(msgbuff, CTRL_ATTR_FAMILY_NAME, ETHTOOL_GENL_NAME))
		goto out;

	nlsock_sendmsg(nlsk, NULL);
	nlsock_process_reply(nlsk, family_info_cb, nlctx);
	ret = nlctx->ethnl_fam ? 0 : -EADDRNOTAVAIL;

out:
	nlctx->suppress_nlerr = 0;
	return ret;
}
#endif

int netlink_init(struct cmd_context *ctx)
{
	struct nl_context *nlctx;
	int ret;

	nlctx = calloc(1, sizeof(*nlctx));
	if (!nlctx)
		return -ENOMEM;
	nlctx->ctx = ctx;
	ret = nlsock_init(nlctx, &nlctx->ethnl_socket, NETLINK_GENERIC);
	if (ret < 0)
		goto out_free;
	ret = get_genl_family(nlctx, nlctx->ethnl_socket);
	if (ret < 0)
		goto out_nlsk;

	ctx->nlctx = nlctx;
	return 0;

out_nlsk:
	nlsock_done(nlctx->ethnl_socket);
out_free:
	free(nlctx->ops_info);
	free(nlctx);
	return ret;
}

static void netlink_done(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;

	if (!nlctx)
		return;

	nlsock_done(nlctx->ethnl_socket);
	nlsock_done(nlctx->ethnl2_socket);
	nlsock_done(nlctx->rtnl_socket);
	free(nlctx->ops_info);
	free(nlctx);
	ctx->nlctx = NULL;
	cleanup_all_strings();
}

/**
 * netlink_run_handler() - run netlink handler for subcommand
 * @ctx:         command context
 * @nlchk:       netlink capability check
 * @nlfunc:      subcommand netlink handler to call
 * @no_fallback: there is no ioctl fallback handler
 *
 * This function returns only if ioctl() handler should be run as fallback.
 * Otherwise it exits with appropriate return code.
 */
void netlink_run_handler(struct cmd_context *ctx, nl_chk_t nlchk,
			 nl_func_t nlfunc, bool no_fallback)
{
	bool wildcard = ctx->devname && !strcmp(ctx->devname, WILDCARD_DEVNAME);
	bool wildcard_unsupported, ioctl_fallback;
	struct nl_context *nlctx;
	const char *reason;
	int ret;

	if (nlchk && !nlchk(ctx)) {
		reason = "ioctl-only request";
		goto no_support;
	}
	if (ctx->devname && strlen(ctx->devname) >= ALTIFNAMSIZ) {
		fprintf(stderr, "device name '%s' longer than %u characters\n",
			ctx->devname, ALTIFNAMSIZ - 1);
		exit(1);
	}

	if (!nlfunc) {
		reason = "ethtool netlink support for subcommand missing";
		goto no_support;
	}
	if (netlink_init(ctx)) {
		reason = "netlink interface initialization failed";
		goto no_support;
	}
	nlctx = ctx->nlctx;

	ret = nlfunc(ctx);
	wildcard_unsupported = nlctx->wildcard_unsupported;
	ioctl_fallback = nlctx->ioctl_fallback;
	netlink_done(ctx);

	if (no_fallback || ret != -EOPNOTSUPP || !ioctl_fallback) {
		if (wildcard_unsupported)
			fprintf(stderr, "%s\n",
				"subcommand does not support wildcard dump");
		exit(ret >= 0 ? ret : 1);
	}
	if (wildcard_unsupported)
		reason = "subcommand does not support wildcard dump";
	else
		reason = "kernel netlink support for subcommand missing";

no_support:
	if (no_fallback) {
		fprintf(stderr, "%s, subcommand not supported by ioctl\n",
			reason);
		exit(1);
	}
	if (wildcard) {
		fprintf(stderr, "%s, wildcard dump not supported\n", reason);
		exit(1);
	}
	if (ctx->devname && strlen(ctx->devname) >= IFNAMSIZ) {
		fprintf(stderr,
			"%s, device name longer than %u not supported\n",
			reason, IFNAMSIZ - 1);
		exit(1);
	}

	/* fallback to ioctl() */
}
