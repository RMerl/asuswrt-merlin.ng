/*
 * permaddr.c - netlink implementation of permanent address request
 *
 * Implementation of "ethtool -P <dev>"
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <linux/rtnetlink.h>
#include <linux/if_link.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"

/* PERMADDR_GET */

static int permaddr_prep_request(struct nl_socket *nlsk)
{
	unsigned int nlm_flags = NLM_F_REQUEST | NLM_F_ACK;
	struct nl_context *nlctx = nlsk->nlctx;
	const char *devname = nlctx->ctx->devname;
	struct nl_msg_buff *msgbuff = &nlsk->msgbuff;
	struct ifinfomsg *ifinfo;
	struct nlmsghdr *nlhdr;
	int ret;

	if (devname && !strcmp(devname, WILDCARD_DEVNAME)) {
		devname = NULL;
		nlm_flags |= NLM_F_DUMP;
	}
	nlctx->is_dump = !devname;

        ret = msgbuff_realloc(msgbuff, MNL_SOCKET_BUFFER_SIZE);
        if (ret < 0)
                return ret;
        memset(msgbuff->buff, '\0', NLMSG_HDRLEN + sizeof(*ifinfo));

	nlhdr = mnl_nlmsg_put_header(msgbuff->buff);
	nlhdr->nlmsg_type = RTM_GETLINK;
	nlhdr->nlmsg_flags = nlm_flags;
	msgbuff->nlhdr = nlhdr;
	ifinfo = mnl_nlmsg_put_extra_header(nlhdr, sizeof(*ifinfo));

	if (devname) {
		uint16_t type = IFLA_IFNAME;

		if (strlen(devname) >= IFNAMSIZ)
			type = IFLA_ALT_IFNAME;
		if (ethnla_put_strz(msgbuff, type, devname))
			return -EMSGSIZE;
	}
	if (ethnla_put_u32(msgbuff, IFLA_EXT_MASK, RTEXT_FILTER_SKIP_STATS))
		return -EMSGSIZE;

	return 0;
}

int permaddr_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[__IFLA_MAX] = {};
	DECLARE_ATTR_TB_INFO(tb);
	struct nl_context *nlctx = data;
	const uint8_t *permaddr;
	unsigned int i;
	int ret;

	if (nlhdr->nlmsg_type != RTM_NEWLINK)
		goto err;
	ret = mnl_attr_parse(nlhdr, sizeof(struct ifinfomsg), attr_cb,
			     &tb_info);
	if (ret < 0 || !tb[IFLA_IFNAME])
		goto err;
	nlctx->devname = mnl_attr_get_str(tb[IFLA_IFNAME]);
	if (!dev_ok(nlctx))
		goto err;

	if (!tb[IFLA_PERM_ADDRESS]) {
		if (!nlctx->is_dump)
			printf("Permanent address: not set\n");
		return MNL_CB_OK;
	}

	if (nlctx->is_dump)
		printf("Permanent address of %s:", nlctx->devname);
	else
		printf("Permanent address:");
	permaddr = mnl_attr_get_payload(tb[IFLA_PERM_ADDRESS]);
	for (i = 0; i < mnl_attr_get_payload_len(tb[IFLA_PERM_ADDRESS]); i++)
		printf("%c%02x", i ? ':' : ' ', permaddr[i]);
	putchar('\n');
	return MNL_CB_OK;

err:
	if (nlctx->is_dump || nlctx->is_monitor)
		return MNL_CB_OK;
	nlctx->exit_code = 2;
	return MNL_CB_ERROR;
}

int nl_permaddr(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	int ret;

	ret = netlink_init_rtnl_socket(nlctx);
	if (ret < 0)
		return ret;
	ret = permaddr_prep_request(nlctx->rtnl_socket);
	if (ret < 0)
		return ret;
	return nlsock_send_get_request(nlctx->rtnl_socket, permaddr_reply_cb);
}
