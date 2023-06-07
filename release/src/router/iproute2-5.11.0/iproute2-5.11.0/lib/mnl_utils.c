// SPDX-License-Identifier: GPL-2.0+
/*
 * mnl_utils.c	Helpers for working with libmnl.
 */

#include <errno.h>
#include <string.h>
#include <time.h>
#include <libmnl/libmnl.h>
#include <linux/genetlink.h>

#include "libnetlink.h"
#include "mnl_utils.h"
#include "utils.h"

struct mnl_socket *mnlu_socket_open(int bus)
{
	struct mnl_socket *nl;
	int one = 1;

	nl = mnl_socket_open(bus);
	if (nl == NULL)
		return NULL;

	mnl_socket_setsockopt(nl, NETLINK_CAP_ACK, &one, sizeof(one));
	mnl_socket_setsockopt(nl, NETLINK_EXT_ACK, &one, sizeof(one));

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0)
		goto err_bind;

	return nl;

err_bind:
	mnl_socket_close(nl);
	return NULL;
}

struct nlmsghdr *mnlu_msg_prepare(void *buf, uint32_t nlmsg_type, uint16_t flags,
				  void *extra_header, size_t extra_header_size)
{
	struct nlmsghdr *nlh;
	void *eh;

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = nlmsg_type;
	nlh->nlmsg_flags = flags;
	nlh->nlmsg_seq = time(NULL);

	eh = mnl_nlmsg_put_extra_header(nlh, extra_header_size);
	memcpy(eh, extra_header, extra_header_size);

	return nlh;
}

static int mnlu_cb_noop(const struct nlmsghdr *nlh, void *data)
{
	return MNL_CB_OK;
}

static int mnlu_cb_error(const struct nlmsghdr *nlh, void *data)
{
	const struct nlmsgerr *err = mnl_nlmsg_get_payload(nlh);

	/* Netlink subsystems returns the errno value with different signess */
	if (err->error < 0)
		errno = -err->error;
	else
		errno = err->error;

	if (nl_dump_ext_ack(nlh, NULL))
		return MNL_CB_ERROR;

	return err->error == 0 ? MNL_CB_STOP : MNL_CB_ERROR;
}

static int mnlu_cb_stop(const struct nlmsghdr *nlh, void *data)
{
	int len = *(int *)NLMSG_DATA(nlh);

	if (len < 0) {
		errno = -len;
		nl_dump_ext_ack_done(nlh, len);
		return MNL_CB_ERROR;
	}
	return MNL_CB_STOP;
}

static mnl_cb_t mnlu_cb_array[NLMSG_MIN_TYPE] = {
	[NLMSG_NOOP]	= mnlu_cb_noop,
	[NLMSG_ERROR]	= mnlu_cb_error,
	[NLMSG_DONE]	= mnlu_cb_stop,
	[NLMSG_OVERRUN]	= mnlu_cb_noop,
};

int mnlu_socket_recv_run(struct mnl_socket *nl, unsigned int seq, void *buf, size_t buf_size,
			 mnl_cb_t cb, void *data)
{
	unsigned int portid = mnl_socket_get_portid(nl);
	int err;

	do {
		err = mnl_socket_recvfrom(nl, buf, buf_size);
		if (err <= 0)
			break;
		err = mnl_cb_run2(buf, err, seq, portid,
				  cb, data, mnlu_cb_array,
				  ARRAY_SIZE(mnlu_cb_array));
	} while (err > 0);

	return err;
}

static int get_family_id_attr_cb(const struct nlattr *attr, void *data)
{
	int type = mnl_attr_get_type(attr);
	const struct nlattr **tb = data;

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
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[CTRL_ATTR_MAX + 1] = {};
	uint32_t *p_id = data;

	mnl_attr_parse(nlh, sizeof(*genl), get_family_id_attr_cb, tb);
	if (!tb[CTRL_ATTR_FAMILY_ID])
		return MNL_CB_ERROR;
	*p_id = mnl_attr_get_u16(tb[CTRL_ATTR_FAMILY_ID]);
	return MNL_CB_OK;
}

static int family_get(struct mnlu_gen_socket *nlg, const char *family_name)
{
	struct genlmsghdr hdr = {};
	struct nlmsghdr *nlh;
	int err;

	hdr.cmd = CTRL_CMD_GETFAMILY;
	hdr.version = 0x1;

	nlh = mnlu_msg_prepare(nlg->buf, GENL_ID_CTRL,
			       NLM_F_REQUEST | NLM_F_ACK,
			       &hdr, sizeof(hdr));

	mnl_attr_put_strz(nlh, CTRL_ATTR_FAMILY_NAME, family_name);

	err = mnl_socket_sendto(nlg->nl, nlh, nlh->nlmsg_len);
	if (err < 0)
		return err;

	err = mnlu_socket_recv_run(nlg->nl, nlh->nlmsg_seq, nlg->buf,
				   MNL_SOCKET_BUFFER_SIZE,
				   get_family_id_cb, &nlg->family);
	return err;
}

int mnlu_gen_socket_open(struct mnlu_gen_socket *nlg, const char *family_name,
			 uint8_t version)
{
	int err;

	nlg->buf = malloc(MNL_SOCKET_BUFFER_SIZE);
	if (!nlg->buf)
		goto err_buf_alloc;

	nlg->nl = mnlu_socket_open(NETLINK_GENERIC);
	if (!nlg->nl)
		goto err_socket_open;

	err = family_get(nlg, family_name);
	if (err)
		goto err_socket;

	return 0;

err_socket:
	mnl_socket_close(nlg->nl);
err_socket_open:
	free(nlg->buf);
err_buf_alloc:
	return -1;
}

void mnlu_gen_socket_close(struct mnlu_gen_socket *nlg)
{
	mnl_socket_close(nlg->nl);
	free(nlg->buf);
}

struct nlmsghdr *mnlu_gen_socket_cmd_prepare(struct mnlu_gen_socket *nlg,
					     uint8_t cmd, uint16_t flags)
{
	struct genlmsghdr hdr = {};
	struct nlmsghdr *nlh;

	hdr.cmd = cmd;
	hdr.version = nlg->version;
	nlh = mnlu_msg_prepare(nlg->buf, nlg->family, flags, &hdr, sizeof(hdr));
	nlg->seq = nlh->nlmsg_seq;
	return nlh;
}

int mnlu_gen_socket_sndrcv(struct mnlu_gen_socket *nlg, const struct nlmsghdr *nlh,
			   mnl_cb_t data_cb, void *data)
{
	int err;

	err = mnl_socket_sendto(nlg->nl, nlh, nlh->nlmsg_len);
	if (err < 0) {
		perror("Failed to send data");
		return -errno;
	}

	err = mnlu_socket_recv_run(nlg->nl, nlh->nlmsg_seq, nlg->buf,
				   MNL_SOCKET_BUFFER_SIZE,
				   data_cb, data);
	if (err < 0) {
		fprintf(stderr, "kernel answers: %s\n", strerror(errno));
		return -errno;
	}
	return 0;
}
