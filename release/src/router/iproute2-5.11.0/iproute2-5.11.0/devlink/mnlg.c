/*
 *   mnlg.c	Generic Netlink helpers for libmnl
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Jiri Pirko <jiri@mellanox.com>
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <libmnl/libmnl.h>
#include <linux/genetlink.h>

#include "libnetlink.h"
#include "mnl_utils.h"
#include "utils.h"
#include "mnlg.h"

struct mnlg_socket {
	struct mnl_socket *nl;
	char *buf;
	uint32_t id;
	uint8_t version;
	unsigned int seq;
};

static struct nlmsghdr *__mnlg_msg_prepare(struct mnlg_socket *nlg, uint8_t cmd,
					   uint16_t flags, uint32_t id,
					   uint8_t version)
{
	struct genlmsghdr genl = {
		.cmd = cmd,
		.version = version,
	};
	struct nlmsghdr *nlh;

	nlh = mnlu_msg_prepare(nlg->buf, id, flags, &genl, sizeof(genl));
	nlg->seq = nlh->nlmsg_seq;
	return nlh;
}

struct nlmsghdr *mnlg_msg_prepare(struct mnlg_socket *nlg, uint8_t cmd,
				  uint16_t flags)
{
	return __mnlg_msg_prepare(nlg, cmd, flags, nlg->id, nlg->version);
}

int mnlg_socket_send(struct mnlg_socket *nlg, const struct nlmsghdr *nlh)
{
	return mnl_socket_sendto(nlg->nl, nlh, nlh->nlmsg_len);
}

int mnlg_socket_recv_run(struct mnlg_socket *nlg, mnl_cb_t data_cb, void *data)
{
	return mnlu_socket_recv_run(nlg->nl, nlg->seq, nlg->buf, MNL_SOCKET_BUFFER_SIZE,
				    data_cb, data);
}

struct group_info {
	bool found;
	uint32_t id;
	const char *name;
};

static int parse_mc_grps_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTRL_ATTR_MCAST_GRP_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case CTRL_ATTR_MCAST_GRP_ID:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			return MNL_CB_ERROR;
		break;
	case CTRL_ATTR_MCAST_GRP_NAME:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			return MNL_CB_ERROR;
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static void parse_genl_mc_grps(struct nlattr *nested,
			       struct group_info *group_info)
{
	struct nlattr *pos;
	const char *name;

	mnl_attr_for_each_nested(pos, nested) {
		struct nlattr *tb[CTRL_ATTR_MCAST_GRP_MAX + 1] = {};

		mnl_attr_parse_nested(pos, parse_mc_grps_cb, tb);
		if (!tb[CTRL_ATTR_MCAST_GRP_NAME] ||
		    !tb[CTRL_ATTR_MCAST_GRP_ID])
			continue;

		name = mnl_attr_get_str(tb[CTRL_ATTR_MCAST_GRP_NAME]);
		if (strcmp(name, group_info->name) != 0)
			continue;

		group_info->id = mnl_attr_get_u32(tb[CTRL_ATTR_MCAST_GRP_ID]);
		group_info->found = true;
	}
}

static int get_group_id_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTRL_ATTR_MAX) < 0)
		return MNL_CB_ERROR;

	if (type == CTRL_ATTR_MCAST_GROUPS &&
	    mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
		return MNL_CB_ERROR;
	tb[type] = attr;
	return MNL_CB_OK;
}

static int get_group_id_cb(const struct nlmsghdr *nlh, void *data)
{
	struct group_info *group_info = data;
	struct nlattr *tb[CTRL_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*genl), get_group_id_attr_cb, tb);
	if (!tb[CTRL_ATTR_MCAST_GROUPS])
		return MNL_CB_ERROR;
	parse_genl_mc_grps(tb[CTRL_ATTR_MCAST_GROUPS], group_info);
	return MNL_CB_OK;
}

int mnlg_socket_group_add(struct mnlg_socket *nlg, const char *group_name)
{
	struct nlmsghdr *nlh;
	struct group_info group_info;
	int err;

	nlh = __mnlg_msg_prepare(nlg, CTRL_CMD_GETFAMILY,
				 NLM_F_REQUEST | NLM_F_ACK, GENL_ID_CTRL, 1);
	mnl_attr_put_u16(nlh, CTRL_ATTR_FAMILY_ID, nlg->id);

	err = mnlg_socket_send(nlg, nlh);
	if (err < 0)
		return err;

	group_info.found = false;
	group_info.name = group_name;
	err = mnlg_socket_recv_run(nlg, get_group_id_cb, &group_info);
	if (err < 0)
		return err;

	if (!group_info.found) {
		errno = ENOENT;
		return -1;
	}

	err = mnl_socket_setsockopt(nlg->nl, NETLINK_ADD_MEMBERSHIP,
				    &group_info.id, sizeof(group_info.id));
	if (err < 0)
		return err;

	return 0;
}

static int get_family_id_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTRL_ATTR_MAX) < 0)
		return MNL_CB_ERROR;

	if (type == CTRL_ATTR_FAMILY_ID &&
	    mnl_attr_validate(attr, MNL_TYPE_U16) < 0)
		return MNL_CB_ERROR;
	tb[type] = attr;
	return MNL_CB_OK;
}

static int get_family_id_cb(const struct nlmsghdr *nlh, void *data)
{
	uint32_t *p_id = data;
	struct nlattr *tb[CTRL_ATTR_MAX + 1] = {};
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*genl), get_family_id_attr_cb, tb);
	if (!tb[CTRL_ATTR_FAMILY_ID])
		return MNL_CB_ERROR;
	*p_id = mnl_attr_get_u16(tb[CTRL_ATTR_FAMILY_ID]);
	return MNL_CB_OK;
}

struct mnlg_socket *mnlg_socket_open(const char *family_name, uint8_t version)
{
	struct mnlg_socket *nlg;
	struct nlmsghdr *nlh;
	int err;

	nlg = malloc(sizeof(*nlg));
	if (!nlg)
		return NULL;

	nlg->buf = malloc(MNL_SOCKET_BUFFER_SIZE);
	if (!nlg->buf)
		goto err_buf_alloc;

	nlg->nl = mnlu_socket_open(NETLINK_GENERIC);
	if (!nlg->nl)
		goto err_socket_open;

	nlh = __mnlg_msg_prepare(nlg, CTRL_CMD_GETFAMILY,
				 NLM_F_REQUEST | NLM_F_ACK, GENL_ID_CTRL, 1);
	mnl_attr_put_strz(nlh, CTRL_ATTR_FAMILY_NAME, family_name);

	err = mnlg_socket_send(nlg, nlh);
	if (err < 0)
		goto err_mnlg_socket_send;

	err = mnlg_socket_recv_run(nlg, get_family_id_cb, &nlg->id);
	if (err < 0)
		goto err_mnlg_socket_recv_run;

	nlg->version = version;
	return nlg;

err_mnlg_socket_recv_run:
err_mnlg_socket_send:
	mnl_socket_close(nlg->nl);
err_socket_open:
	free(nlg->buf);
err_buf_alloc:
	free(nlg);
	return NULL;
}

void mnlg_socket_close(struct mnlg_socket *nlg)
{
	mnl_socket_close(nlg->nl);
	free(nlg->buf);
	free(nlg);
}

int mnlg_socket_get_fd(struct mnlg_socket *nlg)
{
	return mnl_socket_get_fd(nlg->nl);
}
