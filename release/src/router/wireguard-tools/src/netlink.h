// SPDX-License-Identifier: LGPL-2.1+
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 * Copyright (C) 2008-2012 Pablo Neira Ayuso <pablo@netfilter.org>.
 */

/* This is a minimized version of libmnl meant to be #include'd */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>

#define MNL_SOCKET_AUTOPID 0
#define MNL_ALIGNTO 4
#define MNL_ALIGN(len) (((len)+MNL_ALIGNTO-1) & ~(MNL_ALIGNTO-1))
#define MNL_NLMSG_HDRLEN MNL_ALIGN(sizeof(struct nlmsghdr))
#define MNL_ATTR_HDRLEN MNL_ALIGN(sizeof(struct nlattr))

enum mnl_attr_data_type {
	MNL_TYPE_UNSPEC,
	MNL_TYPE_U8,
	MNL_TYPE_U16,
	MNL_TYPE_U32,
	MNL_TYPE_U64,
	MNL_TYPE_STRING,
	MNL_TYPE_FLAG,
	MNL_TYPE_MSECS,
	MNL_TYPE_NESTED,
	MNL_TYPE_NESTED_COMPAT,
	MNL_TYPE_NUL_STRING,
	MNL_TYPE_BINARY,
	MNL_TYPE_MAX,
};

#define mnl_attr_for_each(attr, nlh, offset) \
	for ((attr) = mnl_nlmsg_get_payload_offset((nlh), (offset)); \
	     mnl_attr_ok((attr), (char *)mnl_nlmsg_get_payload_tail(nlh) - (char *)(attr)); \
	     (attr) = mnl_attr_next(attr))

#define mnl_attr_for_each_nested(attr, nest) \
	for ((attr) = mnl_attr_get_payload(nest); \
	     mnl_attr_ok((attr), (char *)mnl_attr_get_payload(nest) + mnl_attr_get_payload_len(nest) - (char *)(attr)); \
	     (attr) = mnl_attr_next(attr))

#define mnl_attr_for_each_payload(payload, payload_size) \
	for ((attr) = (payload); \
	     mnl_attr_ok((attr), (char *)(payload) + payload_size - (char *)(attr)); \
	     (attr) = mnl_attr_next(attr))

#define MNL_CB_ERROR	-1
#define MNL_CB_STOP	0
#define MNL_CB_OK	1

typedef int (*mnl_attr_cb_t)(const struct nlattr *attr, void *data);
typedef int (*mnl_cb_t)(const struct nlmsghdr *nlh, void *data);

#ifndef MNL_ARRAY_SIZE
#define MNL_ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif

static size_t mnl_ideal_socket_buffer_size(void)
{
	static size_t size = 0;

	if (size)
		return size;
	size = (size_t)sysconf(_SC_PAGESIZE);
	if (size > 8192)
		size = 8192;
	return size;
}

static size_t mnl_nlmsg_size(size_t len)
{
	return len + MNL_NLMSG_HDRLEN;
}

static struct nlmsghdr *mnl_nlmsg_put_header(void *buf)
{
	int len = MNL_ALIGN(sizeof(struct nlmsghdr));
	struct nlmsghdr *nlh = buf;

	memset(buf, 0, len);
	nlh->nlmsg_len = len;
	return nlh;
}

static void *mnl_nlmsg_put_extra_header(struct nlmsghdr *nlh, size_t size)
{
	char *ptr = (char *)nlh + nlh->nlmsg_len;
	size_t len = MNL_ALIGN(size);
	nlh->nlmsg_len += len;
	memset(ptr, 0, len);
	return ptr;
}

static void *mnl_nlmsg_get_payload(const struct nlmsghdr *nlh)
{
	return (void *)nlh + MNL_NLMSG_HDRLEN;
}

static void *mnl_nlmsg_get_payload_offset(const struct nlmsghdr *nlh, size_t offset)
{
	return (void *)nlh + MNL_NLMSG_HDRLEN + MNL_ALIGN(offset);
}

static bool mnl_nlmsg_ok(const struct nlmsghdr *nlh, int len)
{
	return len >= (int)sizeof(struct nlmsghdr) &&
	       nlh->nlmsg_len >= sizeof(struct nlmsghdr) &&
	       (int)nlh->nlmsg_len <= len;
}

static struct nlmsghdr *mnl_nlmsg_next(const struct nlmsghdr *nlh, int *len)
{
	*len -= MNL_ALIGN(nlh->nlmsg_len);
	return (struct nlmsghdr *)((void *)nlh + MNL_ALIGN(nlh->nlmsg_len));
}

static void *mnl_nlmsg_get_payload_tail(const struct nlmsghdr *nlh)
{
	return (void *)nlh + MNL_ALIGN(nlh->nlmsg_len);
}

static bool mnl_nlmsg_seq_ok(const struct nlmsghdr *nlh, unsigned int seq)
{
	return nlh->nlmsg_seq && seq ? nlh->nlmsg_seq == seq : true;
}

static bool mnl_nlmsg_portid_ok(const struct nlmsghdr *nlh, unsigned int portid)
{
	return nlh->nlmsg_pid && portid ? nlh->nlmsg_pid == portid : true;
}

static uint16_t mnl_attr_get_type(const struct nlattr *attr)
{
	return attr->nla_type & NLA_TYPE_MASK;
}

static uint16_t mnl_attr_get_payload_len(const struct nlattr *attr)
{
	return attr->nla_len - MNL_ATTR_HDRLEN;
}

static void *mnl_attr_get_payload(const struct nlattr *attr)
{
	return (void *)attr + MNL_ATTR_HDRLEN;
}

static bool mnl_attr_ok(const struct nlattr *attr, int len)
{
	return len >= (int)sizeof(struct nlattr) &&
	       attr->nla_len >= sizeof(struct nlattr) &&
	       (int)attr->nla_len <= len;
}

static struct nlattr *mnl_attr_next(const struct nlattr *attr)
{
	return (struct nlattr *)((void *)attr + MNL_ALIGN(attr->nla_len));
}

static int mnl_attr_type_valid(const struct nlattr *attr, uint16_t max)
{
	if (mnl_attr_get_type(attr) > max) {
		errno = EOPNOTSUPP;
		return -1;
	}
	return 1;
}

static int __mnl_attr_validate(const struct nlattr *attr,
			       enum mnl_attr_data_type type, size_t exp_len)
{
	uint16_t attr_len = mnl_attr_get_payload_len(attr);
	const char *attr_data = mnl_attr_get_payload(attr);

	if (attr_len < exp_len) {
		errno = ERANGE;
		return -1;
	}
	switch(type) {
	case MNL_TYPE_FLAG:
		if (attr_len > 0) {
			errno = ERANGE;
			return -1;
		}
		break;
	case MNL_TYPE_NUL_STRING:
		if (attr_len == 0) {
			errno = ERANGE;
			return -1;
		}
		if (attr_data[attr_len-1] != '\0') {
			errno = EINVAL;
			return -1;
		}
		break;
	case MNL_TYPE_STRING:
		if (attr_len == 0) {
			errno = ERANGE;
			return -1;
		}
		break;
	case MNL_TYPE_NESTED:

		if (attr_len == 0)
			break;

		if (attr_len < MNL_ATTR_HDRLEN) {
			errno = ERANGE;
			return -1;
		}
		break;
	default:

		break;
	}
	if (exp_len && attr_len > exp_len) {
		errno = ERANGE;
		return -1;
	}
	return 0;
}

static const size_t mnl_attr_data_type_len[MNL_TYPE_MAX] = {
	[MNL_TYPE_U8]		= sizeof(uint8_t),
	[MNL_TYPE_U16]		= sizeof(uint16_t),
	[MNL_TYPE_U32]		= sizeof(uint32_t),
	[MNL_TYPE_U64]		= sizeof(uint64_t),
	[MNL_TYPE_MSECS]	= sizeof(uint64_t),
};

static int mnl_attr_validate(const struct nlattr *attr, enum mnl_attr_data_type type)
{
	int exp_len;

	if (type >= MNL_TYPE_MAX) {
		errno = EINVAL;
		return -1;
	}
	exp_len = mnl_attr_data_type_len[type];
	return __mnl_attr_validate(attr, type, exp_len);
}

static int mnl_attr_parse(const struct nlmsghdr *nlh, unsigned int offset,
			  mnl_attr_cb_t cb, void *data)
{
	int ret = MNL_CB_OK;
	const struct nlattr *attr;

	mnl_attr_for_each(attr, nlh, offset)
		if ((ret = cb(attr, data)) <= MNL_CB_STOP)
			return ret;
	return ret;
}

static int mnl_attr_parse_nested(const struct nlattr *nested, mnl_attr_cb_t cb,
				 void *data)
{
	int ret = MNL_CB_OK;
	const struct nlattr *attr;

	mnl_attr_for_each_nested(attr, nested)
		if ((ret = cb(attr, data)) <= MNL_CB_STOP)
			return ret;
	return ret;
}

static uint8_t mnl_attr_get_u8(const struct nlattr *attr)
{
	return *((uint8_t *)mnl_attr_get_payload(attr));
}

static uint16_t mnl_attr_get_u16(const struct nlattr *attr)
{
	return *((uint16_t *)mnl_attr_get_payload(attr));
}

static uint32_t mnl_attr_get_u32(const struct nlattr *attr)
{
	return *((uint32_t *)mnl_attr_get_payload(attr));
}

static uint64_t mnl_attr_get_u64(const struct nlattr *attr)
{
	uint64_t tmp;
	memcpy(&tmp, mnl_attr_get_payload(attr), sizeof(tmp));
	return tmp;
}

static const char *mnl_attr_get_str(const struct nlattr *attr)
{
	return mnl_attr_get_payload(attr);
}

static void mnl_attr_put(struct nlmsghdr *nlh, uint16_t type, size_t len,
			 const void *data)
{
	struct nlattr *attr = mnl_nlmsg_get_payload_tail(nlh);
	uint16_t payload_len = MNL_ALIGN(sizeof(struct nlattr)) + len;
	int pad;

	attr->nla_type = type;
	attr->nla_len = payload_len;
	memcpy(mnl_attr_get_payload(attr), data, len);
	nlh->nlmsg_len += MNL_ALIGN(payload_len);
	pad = MNL_ALIGN(len) - len;
	if (pad > 0)
		memset(mnl_attr_get_payload(attr) + len, 0, pad);
}

static void mnl_attr_put_u16(struct nlmsghdr *nlh, uint16_t type, uint16_t data)
{
	mnl_attr_put(nlh, type, sizeof(uint16_t), &data);
}

static void mnl_attr_put_u32(struct nlmsghdr *nlh, uint16_t type, uint32_t data)
{
	mnl_attr_put(nlh, type, sizeof(uint32_t), &data);
}

static void mnl_attr_put_strz(struct nlmsghdr *nlh, uint16_t type, const char *data)
{
	mnl_attr_put(nlh, type, strlen(data)+1, data);
}

static struct nlattr *mnl_attr_nest_start(struct nlmsghdr *nlh, uint16_t type)
{
	struct nlattr *start = mnl_nlmsg_get_payload_tail(nlh);

	start->nla_type = NLA_F_NESTED | type;
	nlh->nlmsg_len += MNL_ALIGN(sizeof(struct nlattr));
	return start;
}

static bool mnl_attr_put_check(struct nlmsghdr *nlh, size_t buflen,
			       uint16_t type, size_t len, const void *data)
{
	if (nlh->nlmsg_len + MNL_ATTR_HDRLEN + MNL_ALIGN(len) > buflen)
		return false;
	mnl_attr_put(nlh, type, len, data);
	return true;
}

static bool mnl_attr_put_u8_check(struct nlmsghdr *nlh, size_t buflen,
				  uint16_t type, uint8_t data)
{
	return mnl_attr_put_check(nlh, buflen, type, sizeof(uint8_t), &data);
}

static bool mnl_attr_put_u16_check(struct nlmsghdr *nlh, size_t buflen,
				   uint16_t type, uint16_t data)
{
	return mnl_attr_put_check(nlh, buflen, type, sizeof(uint16_t), &data);
}

static bool mnl_attr_put_u32_check(struct nlmsghdr *nlh, size_t buflen,
				   uint16_t type, uint32_t data)
{
	return mnl_attr_put_check(nlh, buflen, type, sizeof(uint32_t), &data);
}

static struct nlattr *mnl_attr_nest_start_check(struct nlmsghdr *nlh, size_t buflen,
						uint16_t type)
{
	if (nlh->nlmsg_len + MNL_ATTR_HDRLEN > buflen)
		return NULL;
	return mnl_attr_nest_start(nlh, type);
}

static void mnl_attr_nest_end(struct nlmsghdr *nlh, struct nlattr *start)
{
	start->nla_len = mnl_nlmsg_get_payload_tail(nlh) - (void *)start;
}

static void mnl_attr_nest_cancel(struct nlmsghdr *nlh, struct nlattr *start)
{
	nlh->nlmsg_len -= mnl_nlmsg_get_payload_tail(nlh) - (void *)start;
}

static int mnl_cb_noop(__attribute__((unused)) const struct nlmsghdr *nlh, __attribute__((unused)) void *data)
{
	return MNL_CB_OK;
}

static int mnl_cb_error(const struct nlmsghdr *nlh, __attribute__((unused)) void *data)
{
	const struct nlmsgerr *err = mnl_nlmsg_get_payload(nlh);

	if (nlh->nlmsg_len < mnl_nlmsg_size(sizeof(struct nlmsgerr))) {
		errno = EBADMSG;
		return MNL_CB_ERROR;
	}

	if (err->error < 0)
		errno = -err->error;
	else
		errno = err->error;

	return err->error == 0 ? MNL_CB_STOP : MNL_CB_ERROR;
}

static int mnl_cb_stop(__attribute__((unused)) const struct nlmsghdr *nlh, __attribute__((unused)) void *data)
{
	return MNL_CB_STOP;
}

static const mnl_cb_t default_cb_array[NLMSG_MIN_TYPE] = {
	[NLMSG_NOOP]	= mnl_cb_noop,
	[NLMSG_ERROR]	= mnl_cb_error,
	[NLMSG_DONE]	= mnl_cb_stop,
	[NLMSG_OVERRUN]	= mnl_cb_noop,
};

static int __mnl_cb_run(const void *buf, size_t numbytes,
			unsigned int seq, unsigned int portid,
			mnl_cb_t cb_data, void *data,
			const mnl_cb_t *cb_ctl_array,
			unsigned int cb_ctl_array_len)
{
	int ret = MNL_CB_OK, len = numbytes;
	const struct nlmsghdr *nlh = buf;

	while (mnl_nlmsg_ok(nlh, len)) {

		if (!mnl_nlmsg_portid_ok(nlh, portid)) {
			errno = ESRCH;
			return -1;
		}

		if (!mnl_nlmsg_seq_ok(nlh, seq)) {
			errno = EPROTO;
			return -1;
		}

		if (nlh->nlmsg_flags & NLM_F_DUMP_INTR) {
			errno = EINTR;
			return -1;
		}

		if (nlh->nlmsg_type >= NLMSG_MIN_TYPE) {
			if (cb_data){
				ret = cb_data(nlh, data);
				if (ret <= MNL_CB_STOP)
					goto out;
			}
		} else if (nlh->nlmsg_type < cb_ctl_array_len) {
			if (cb_ctl_array && cb_ctl_array[nlh->nlmsg_type]) {
				ret = cb_ctl_array[nlh->nlmsg_type](nlh, data);
				if (ret <= MNL_CB_STOP)
					goto out;
			}
		} else if (default_cb_array[nlh->nlmsg_type]) {
			ret = default_cb_array[nlh->nlmsg_type](nlh, data);
			if (ret <= MNL_CB_STOP)
				goto out;
		}
		nlh = mnl_nlmsg_next(nlh, &len);
	}
out:
	return ret;
}

static int mnl_cb_run2(const void *buf, size_t numbytes, unsigned int seq,
		       unsigned int portid, mnl_cb_t cb_data, void *data,
		       const mnl_cb_t *cb_ctl_array, unsigned int cb_ctl_array_len)
{
	return __mnl_cb_run(buf, numbytes, seq, portid, cb_data, data,
			    cb_ctl_array, cb_ctl_array_len);
}

static int mnl_cb_run(const void *buf, size_t numbytes, unsigned int seq,
		      unsigned int portid, mnl_cb_t cb_data, void *data)
{
	return __mnl_cb_run(buf, numbytes, seq, portid, cb_data, data, NULL, 0);
}

struct mnl_socket {
	int 			fd;
	struct sockaddr_nl	addr;
};

static unsigned int mnl_socket_get_portid(const struct mnl_socket *nl)
{
	return nl->addr.nl_pid;
}

static struct mnl_socket *__mnl_socket_open(int bus, int flags)
{
	struct mnl_socket *nl;

	nl = calloc(1, sizeof(struct mnl_socket));
	if (nl == NULL)
		return NULL;

	nl->fd = socket(AF_NETLINK, SOCK_RAW | flags, bus);
	if (nl->fd == -1) {
		free(nl);
		return NULL;
	}

	return nl;
}

static struct mnl_socket *mnl_socket_open(int bus)
{
	return __mnl_socket_open(bus, 0);
}

static int mnl_socket_bind(struct mnl_socket *nl, unsigned int groups, pid_t pid)
{
	int ret;
	socklen_t addr_len;

	nl->addr.nl_family = AF_NETLINK;
	nl->addr.nl_groups = groups;
	nl->addr.nl_pid = pid;

	ret = bind(nl->fd, (struct sockaddr *) &nl->addr, sizeof (nl->addr));
	if (ret < 0)
		return ret;

	addr_len = sizeof(nl->addr);
	ret = getsockname(nl->fd, (struct sockaddr *) &nl->addr, &addr_len);
	if (ret < 0)
		return ret;

	if (addr_len != sizeof(nl->addr)) {
		errno = EINVAL;
		return -1;
	}
	if (nl->addr.nl_family != AF_NETLINK) {
		errno = EINVAL;
		return -1;
	}
	return 0;
}

static ssize_t mnl_socket_sendto(const struct mnl_socket *nl, const void *buf,
				 size_t len)
{
	static const struct sockaddr_nl snl = {
		.nl_family = AF_NETLINK
	};
	return sendto(nl->fd, buf, len, 0,
		      (struct sockaddr *) &snl, sizeof(snl));
}

static ssize_t mnl_socket_recvfrom(const struct mnl_socket *nl, void *buf,
				   size_t bufsiz)
{
	ssize_t ret;
	struct sockaddr_nl addr;
	struct iovec iov = {
		.iov_base	= buf,
		.iov_len	= bufsiz,
	};
	struct msghdr msg = {
		.msg_name	= &addr,
		.msg_namelen	= sizeof(struct sockaddr_nl),
		.msg_iov	= &iov,
		.msg_iovlen	= 1,
		.msg_control	= NULL,
		.msg_controllen	= 0,
		.msg_flags	= 0,
	};
	ret = recvmsg(nl->fd, &msg, 0);
	if (ret == -1)
		return ret;

	if (msg.msg_flags & MSG_TRUNC) {
		errno = ENOSPC;
		return -1;
	}
	if (msg.msg_namelen != sizeof(struct sockaddr_nl)) {
		errno = EINVAL;
		return -1;
	}
	return ret;
}

static int mnl_socket_close(struct mnl_socket *nl)
{
	int ret = close(nl->fd);
	free(nl);
	return ret;
}

/* This is a wrapper for generic netlink, originally from Jiri Pirko <jiri@mellanox.com>: */

struct mnlg_socket {
	struct mnl_socket *nl;
	char *buf;
	uint16_t id;
	uint8_t version;
	unsigned int seq;
	unsigned int portid;
};

static struct nlmsghdr *__mnlg_msg_prepare(struct mnlg_socket *nlg, uint8_t cmd,
					   uint16_t flags, uint16_t id,
					   uint8_t version)
{
	struct nlmsghdr *nlh;
	struct genlmsghdr *genl;

	nlh = mnl_nlmsg_put_header(nlg->buf);
	nlh->nlmsg_type	= id;
	nlh->nlmsg_flags = flags;
	nlg->seq = time(NULL);
	nlh->nlmsg_seq = nlg->seq;

	genl = mnl_nlmsg_put_extra_header(nlh, sizeof(struct genlmsghdr));
	genl->cmd = cmd;
	genl->version = version;

	return nlh;
}

static struct nlmsghdr *mnlg_msg_prepare(struct mnlg_socket *nlg, uint8_t cmd,
					 uint16_t flags)
{
	return __mnlg_msg_prepare(nlg, cmd, flags, nlg->id, nlg->version);
}

static int mnlg_socket_send(struct mnlg_socket *nlg, const struct nlmsghdr *nlh)
{
	return mnl_socket_sendto(nlg->nl, nlh, nlh->nlmsg_len);
}

static int mnlg_cb_noop(const struct nlmsghdr *nlh, void *data)
{
	(void)nlh;
	(void)data;
	return MNL_CB_OK;
}

static int mnlg_cb_error(const struct nlmsghdr *nlh, void *data)
{
	const struct nlmsgerr *err = mnl_nlmsg_get_payload(nlh);
	(void)data;

	if (nlh->nlmsg_len < mnl_nlmsg_size(sizeof(struct nlmsgerr))) {
		errno = EBADMSG;
		return MNL_CB_ERROR;
	}
	/* Netlink subsystems returns the errno value with different signess */
	if (err->error < 0)
		errno = -err->error;
	else
		errno = err->error;

	return err->error == 0 ? MNL_CB_STOP : MNL_CB_ERROR;
}

static int mnlg_cb_stop(const struct nlmsghdr *nlh, void *data)
{
	(void)data;
	if (nlh->nlmsg_flags & NLM_F_MULTI && nlh->nlmsg_len == mnl_nlmsg_size(sizeof(int))) {
		int error = *(int *)mnl_nlmsg_get_payload(nlh);
		/* Netlink subsystems returns the errno value with different signess */
		if (error < 0)
			errno = -error;
		else
			errno = error;

		return error == 0 ? MNL_CB_STOP : MNL_CB_ERROR;
	}
	return MNL_CB_STOP;
}

static const mnl_cb_t mnlg_cb_array[] = {
	[NLMSG_NOOP]	= mnlg_cb_noop,
	[NLMSG_ERROR]	= mnlg_cb_error,
	[NLMSG_DONE]	= mnlg_cb_stop,
	[NLMSG_OVERRUN]	= mnlg_cb_noop,
};

static int mnlg_socket_recv_run(struct mnlg_socket *nlg, mnl_cb_t data_cb, void *data)
{
	int err;

	do {
		err = mnl_socket_recvfrom(nlg->nl, nlg->buf,
					  mnl_ideal_socket_buffer_size());
		if (err <= 0)
			break;
		err = mnl_cb_run2(nlg->buf, err, nlg->seq, nlg->portid,
				  data_cb, data, mnlg_cb_array, MNL_ARRAY_SIZE(mnlg_cb_array));
	} while (err > 0);

	return err;
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
	uint16_t *p_id = data;
	struct nlattr *tb[CTRL_ATTR_MAX + 1] = { 0 };

	mnl_attr_parse(nlh, sizeof(struct genlmsghdr), get_family_id_attr_cb, tb);
	if (!tb[CTRL_ATTR_FAMILY_ID])
		return MNL_CB_ERROR;
	*p_id = mnl_attr_get_u16(tb[CTRL_ATTR_FAMILY_ID]);
	return MNL_CB_OK;
}

static struct mnlg_socket *mnlg_socket_open(const char *family_name, uint8_t version)
{
	struct mnlg_socket *nlg;
	struct nlmsghdr *nlh;
	int err;

	nlg = malloc(sizeof(*nlg));
	if (!nlg)
		return NULL;
	nlg->id = 0;

	err = -ENOMEM;
	nlg->buf = malloc(mnl_ideal_socket_buffer_size());
	if (!nlg->buf)
		goto err_buf_alloc;

	nlg->nl = mnl_socket_open(NETLINK_GENERIC);
	if (!nlg->nl) {
		err = -errno;
		goto err_mnl_socket_open;
	}

	if (mnl_socket_bind(nlg->nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		err = -errno;
		goto err_mnl_socket_bind;
	}

	nlg->portid = mnl_socket_get_portid(nlg->nl);

	nlh = __mnlg_msg_prepare(nlg, CTRL_CMD_GETFAMILY,
				 NLM_F_REQUEST | NLM_F_ACK, GENL_ID_CTRL, 1);
	mnl_attr_put_strz(nlh, CTRL_ATTR_FAMILY_NAME, family_name);

	if (mnlg_socket_send(nlg, nlh) < 0) {
		err = -errno;
		goto err_mnlg_socket_send;
	}

	errno = 0;
	if (mnlg_socket_recv_run(nlg, get_family_id_cb, &nlg->id) < 0) {
		errno = errno == ENOENT ? EPROTONOSUPPORT : errno;
		err = errno ? -errno : -ENOSYS;
		goto err_mnlg_socket_recv_run;
	}

	nlg->version = version;
	errno = 0;
	return nlg;

err_mnlg_socket_recv_run:
err_mnlg_socket_send:
err_mnl_socket_bind:
	mnl_socket_close(nlg->nl);
err_mnl_socket_open:
	free(nlg->buf);
err_buf_alloc:
	free(nlg);
	errno = -err;
	return NULL;
}

static void mnlg_socket_close(struct mnlg_socket *nlg)
{
	mnl_socket_close(nlg->nl);
	free(nlg->buf);
	free(nlg);
}
