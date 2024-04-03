// SPDX-License-Identifier: LGPL-2.1+
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 * Copyright (C) 2008-2012 Pablo Neira Ayuso <pablo@netfilter.org>.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <linux/genetlink.h>
#include <linux/if_link.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include "wireguard.h"

/* wireguard.h netlink uapi: */

#define WG_GENL_NAME "wireguard"
#define WG_GENL_VERSION 1

enum wg_cmd {
	WG_CMD_GET_DEVICE,
	WG_CMD_SET_DEVICE,
	__WG_CMD_MAX
};

enum wgdevice_flag {
	WGDEVICE_F_REPLACE_PEERS = 1U << 0
};
enum wgdevice_attribute {
	WGDEVICE_A_UNSPEC,
	WGDEVICE_A_IFINDEX,
	WGDEVICE_A_IFNAME,
	WGDEVICE_A_PRIVATE_KEY,
	WGDEVICE_A_PUBLIC_KEY,
	WGDEVICE_A_FLAGS,
	WGDEVICE_A_LISTEN_PORT,
	WGDEVICE_A_FWMARK,
	WGDEVICE_A_PEERS,
	__WGDEVICE_A_LAST
};

enum wgpeer_flag {
	WGPEER_F_REMOVE_ME = 1U << 0,
	WGPEER_F_REPLACE_ALLOWEDIPS = 1U << 1
};
enum wgpeer_attribute {
	WGPEER_A_UNSPEC,
	WGPEER_A_PUBLIC_KEY,
	WGPEER_A_PRESHARED_KEY,
	WGPEER_A_FLAGS,
	WGPEER_A_ENDPOINT,
	WGPEER_A_PERSISTENT_KEEPALIVE_INTERVAL,
	WGPEER_A_LAST_HANDSHAKE_TIME,
	WGPEER_A_RX_BYTES,
	WGPEER_A_TX_BYTES,
	WGPEER_A_ALLOWEDIPS,
	WGPEER_A_PROTOCOL_VERSION,
	__WGPEER_A_LAST
};

enum wgallowedip_attribute {
	WGALLOWEDIP_A_UNSPEC,
	WGALLOWEDIP_A_FAMILY,
	WGALLOWEDIP_A_IPADDR,
	WGALLOWEDIP_A_CIDR_MASK,
	__WGALLOWEDIP_A_LAST
};

/* libmnl mini library: */

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

/* mnlg mini library: */

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

/* wireguard-specific parts: */

struct string_list {
	char *buffer;
	size_t len;
	size_t cap;
};

static int string_list_add(struct string_list *list, const char *str)
{
	size_t len = strlen(str) + 1;

	if (len == 1)
		return 0;

	if (len >= list->cap - list->len) {
		char *new_buffer;
		size_t new_cap = list->cap * 2;

		if (new_cap <  list->len +len + 1)
			new_cap = list->len + len + 1;
		new_buffer = realloc(list->buffer, new_cap);
		if (!new_buffer)
			return -errno;
		list->buffer = new_buffer;
		list->cap = new_cap;
	}
	memcpy(list->buffer + list->len, str, len);
	list->len += len;
	list->buffer[list->len] = '\0';
	return 0;
}

struct interface {
	const char *name;
	bool is_wireguard;
};

static int parse_linkinfo(const struct nlattr *attr, void *data)
{
	struct interface *interface = data;

	if (mnl_attr_get_type(attr) == IFLA_INFO_KIND && !strcmp(WG_GENL_NAME, mnl_attr_get_str(attr)))
		interface->is_wireguard = true;
	return MNL_CB_OK;
}

static int parse_infomsg(const struct nlattr *attr, void *data)
{
	struct interface *interface = data;

	if (mnl_attr_get_type(attr) == IFLA_LINKINFO)
		return mnl_attr_parse_nested(attr, parse_linkinfo, data);
	else if (mnl_attr_get_type(attr) == IFLA_IFNAME)
		interface->name = mnl_attr_get_str(attr);
	return MNL_CB_OK;
}

static int read_devices_cb(const struct nlmsghdr *nlh, void *data)
{
	struct string_list *list = data;
	struct interface interface = { 0 };
	int ret;

	ret = mnl_attr_parse(nlh, sizeof(struct ifinfomsg), parse_infomsg, &interface);
	if (ret != MNL_CB_OK)
		return ret;
	if (interface.name && interface.is_wireguard)
		ret = string_list_add(list, interface.name);
	if (ret < 0)
		return ret;
	if (nlh->nlmsg_type != NLMSG_DONE)
		return MNL_CB_OK + 1;
	return MNL_CB_OK;
}

static int fetch_device_names(struct string_list *list)
{
	struct mnl_socket *nl = NULL;
	char *rtnl_buffer = NULL;
	size_t message_len;
	unsigned int portid, seq;
	ssize_t len;
	int ret = 0;
	struct nlmsghdr *nlh;
	struct ifinfomsg *ifm;

	ret = -ENOMEM;
	rtnl_buffer = calloc(mnl_ideal_socket_buffer_size(), 1);
	if (!rtnl_buffer)
		goto cleanup;

	nl = mnl_socket_open(NETLINK_ROUTE);
	if (!nl) {
		ret = -errno;
		goto cleanup;
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		ret = -errno;
		goto cleanup;
	}

	seq = time(NULL);
	portid = mnl_socket_get_portid(nl);
	nlh = mnl_nlmsg_put_header(rtnl_buffer);
	nlh->nlmsg_type = RTM_GETLINK;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP;
	nlh->nlmsg_seq = seq;
	ifm = mnl_nlmsg_put_extra_header(nlh, sizeof(*ifm));
	ifm->ifi_family = AF_UNSPEC;
	message_len = nlh->nlmsg_len;

	if (mnl_socket_sendto(nl, rtnl_buffer, message_len) < 0) {
		ret = -errno;
		goto cleanup;
	}

another:
	if ((len = mnl_socket_recvfrom(nl, rtnl_buffer, mnl_ideal_socket_buffer_size())) < 0) {
		ret = -errno;
		goto cleanup;
	}
	if ((len = mnl_cb_run(rtnl_buffer, len, seq, portid, read_devices_cb, list)) < 0) {
		/* Netlink returns NLM_F_DUMP_INTR if the set of all tunnels changed
		 * during the dump. That's unfortunate, but is pretty common on busy
		 * systems that are adding and removing tunnels all the time. Rather
		 * than retrying, potentially indefinitely, we just work with the
		 * partial results. */
		if (errno != EINTR) {
			ret = -errno;
			goto cleanup;
		}
	}
	if (len == MNL_CB_OK + 1)
		goto another;
	ret = 0;

cleanup:
	free(rtnl_buffer);
	if (nl)
		mnl_socket_close(nl);
	return ret;
}

static int add_del_iface(const char *ifname, bool add)
{
	struct mnl_socket *nl = NULL;
	char *rtnl_buffer;
	ssize_t len;
	int ret;
	struct nlmsghdr *nlh;
	struct ifinfomsg *ifm;
	struct nlattr *nest;

	rtnl_buffer = calloc(mnl_ideal_socket_buffer_size(), 1);
	if (!rtnl_buffer) {
		ret = -ENOMEM;
		goto cleanup;
	}

	nl = mnl_socket_open(NETLINK_ROUTE);
	if (!nl) {
		ret = -errno;
		goto cleanup;
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		ret = -errno;
		goto cleanup;
	}

	nlh = mnl_nlmsg_put_header(rtnl_buffer);
	nlh->nlmsg_type = add ? RTM_NEWLINK : RTM_DELLINK;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | (add ? NLM_F_CREATE | NLM_F_EXCL : 0);
	nlh->nlmsg_seq = time(NULL);
	ifm = mnl_nlmsg_put_extra_header(nlh, sizeof(*ifm));
	ifm->ifi_family = AF_UNSPEC;
	mnl_attr_put_strz(nlh, IFLA_IFNAME, ifname);
	nest = mnl_attr_nest_start(nlh, IFLA_LINKINFO);
	mnl_attr_put_strz(nlh, IFLA_INFO_KIND, WG_GENL_NAME);
	mnl_attr_nest_end(nlh, nest);

	if (mnl_socket_sendto(nl, rtnl_buffer, nlh->nlmsg_len) < 0) {
		ret = -errno;
		goto cleanup;
	}
	if ((len = mnl_socket_recvfrom(nl, rtnl_buffer, mnl_ideal_socket_buffer_size())) < 0) {
		ret = -errno;
		goto cleanup;
	}
	if (mnl_cb_run(rtnl_buffer, len, nlh->nlmsg_seq, mnl_socket_get_portid(nl), NULL, NULL) < 0) {
		ret = -errno;
		goto cleanup;
	}
	ret = 0;

cleanup:
	free(rtnl_buffer);
	if (nl)
		mnl_socket_close(nl);
	return ret;
}

int wg_set_device(wg_device *dev)
{
	int ret = 0;
	wg_peer *peer = NULL;
	wg_allowedip *allowedip = NULL;
	struct nlattr *peers_nest, *peer_nest, *allowedips_nest, *allowedip_nest;
	struct nlmsghdr *nlh;
	struct mnlg_socket *nlg;

	nlg = mnlg_socket_open(WG_GENL_NAME, WG_GENL_VERSION);
	if (!nlg)
		return -errno;

again:
	nlh = mnlg_msg_prepare(nlg, WG_CMD_SET_DEVICE, NLM_F_REQUEST | NLM_F_ACK);
	mnl_attr_put_strz(nlh, WGDEVICE_A_IFNAME, dev->name);

	if (!peer) {
		uint32_t flags = 0;

		if (dev->flags & WGDEVICE_HAS_PRIVATE_KEY)
			mnl_attr_put(nlh, WGDEVICE_A_PRIVATE_KEY, sizeof(dev->private_key), dev->private_key);
		if (dev->flags & WGDEVICE_HAS_LISTEN_PORT)
			mnl_attr_put_u16(nlh, WGDEVICE_A_LISTEN_PORT, dev->listen_port);
		if (dev->flags & WGDEVICE_HAS_FWMARK)
			mnl_attr_put_u32(nlh, WGDEVICE_A_FWMARK, dev->fwmark);
		if (dev->flags & WGDEVICE_REPLACE_PEERS)
			flags |= WGDEVICE_F_REPLACE_PEERS;
		if (flags)
			mnl_attr_put_u32(nlh, WGDEVICE_A_FLAGS, flags);
	}
	if (!dev->first_peer)
		goto send;
	peers_nest = peer_nest = allowedips_nest = allowedip_nest = NULL;
	peers_nest = mnl_attr_nest_start(nlh, WGDEVICE_A_PEERS);
	for (peer = peer ? peer : dev->first_peer; peer; peer = peer->next_peer) {
		uint32_t flags = 0;

		peer_nest = mnl_attr_nest_start_check(nlh, mnl_ideal_socket_buffer_size(), 0);
		if (!peer_nest)
			goto toobig_peers;
		if (!mnl_attr_put_check(nlh, mnl_ideal_socket_buffer_size(), WGPEER_A_PUBLIC_KEY, sizeof(peer->public_key), peer->public_key))
			goto toobig_peers;
		if (peer->flags & WGPEER_REMOVE_ME)
			flags |= WGPEER_F_REMOVE_ME;
		if (!allowedip) {
			if (peer->flags & WGPEER_REPLACE_ALLOWEDIPS)
				flags |= WGPEER_F_REPLACE_ALLOWEDIPS;
			if (peer->flags & WGPEER_HAS_PRESHARED_KEY) {
				if (!mnl_attr_put_check(nlh, mnl_ideal_socket_buffer_size(), WGPEER_A_PRESHARED_KEY, sizeof(peer->preshared_key), peer->preshared_key))
					goto toobig_peers;
			}
			if (peer->endpoint.addr.sa_family == AF_INET) {
				if (!mnl_attr_put_check(nlh, mnl_ideal_socket_buffer_size(), WGPEER_A_ENDPOINT, sizeof(peer->endpoint.addr4), &peer->endpoint.addr4))
					goto toobig_peers;
			} else if (peer->endpoint.addr.sa_family == AF_INET6) {
				if (!mnl_attr_put_check(nlh, mnl_ideal_socket_buffer_size(), WGPEER_A_ENDPOINT, sizeof(peer->endpoint.addr6), &peer->endpoint.addr6))
					goto toobig_peers;
			}
			if (peer->flags & WGPEER_HAS_PERSISTENT_KEEPALIVE_INTERVAL) {
				if (!mnl_attr_put_u16_check(nlh, mnl_ideal_socket_buffer_size(), WGPEER_A_PERSISTENT_KEEPALIVE_INTERVAL, peer->persistent_keepalive_interval))
					goto toobig_peers;
			}
		}
		if (flags) {
			if (!mnl_attr_put_u32_check(nlh, mnl_ideal_socket_buffer_size(), WGPEER_A_FLAGS, flags))
				goto toobig_peers;
		}
		if (peer->first_allowedip) {
			if (!allowedip)
				allowedip = peer->first_allowedip;
			allowedips_nest = mnl_attr_nest_start_check(nlh, mnl_ideal_socket_buffer_size(), WGPEER_A_ALLOWEDIPS);
			if (!allowedips_nest)
				goto toobig_allowedips;
			for (; allowedip; allowedip = allowedip->next_allowedip) {
				allowedip_nest = mnl_attr_nest_start_check(nlh, mnl_ideal_socket_buffer_size(), 0);
				if (!allowedip_nest)
					goto toobig_allowedips;
				if (!mnl_attr_put_u16_check(nlh, mnl_ideal_socket_buffer_size(), WGALLOWEDIP_A_FAMILY, allowedip->family))
					goto toobig_allowedips;
				if (allowedip->family == AF_INET) {
					if (!mnl_attr_put_check(nlh, mnl_ideal_socket_buffer_size(), WGALLOWEDIP_A_IPADDR, sizeof(allowedip->ip4), &allowedip->ip4))
						goto toobig_allowedips;
				} else if (allowedip->family == AF_INET6) {
					if (!mnl_attr_put_check(nlh, mnl_ideal_socket_buffer_size(), WGALLOWEDIP_A_IPADDR, sizeof(allowedip->ip6), &allowedip->ip6))
						goto toobig_allowedips;
				}
				if (!mnl_attr_put_u8_check(nlh, mnl_ideal_socket_buffer_size(), WGALLOWEDIP_A_CIDR_MASK, allowedip->cidr))
					goto toobig_allowedips;
				mnl_attr_nest_end(nlh, allowedip_nest);
				allowedip_nest = NULL;
			}
			mnl_attr_nest_end(nlh, allowedips_nest);
			allowedips_nest = NULL;
		}

		mnl_attr_nest_end(nlh, peer_nest);
		peer_nest = NULL;
	}
	mnl_attr_nest_end(nlh, peers_nest);
	peers_nest = NULL;
	goto send;
toobig_allowedips:
	if (allowedip_nest)
		mnl_attr_nest_cancel(nlh, allowedip_nest);
	if (allowedips_nest)
		mnl_attr_nest_end(nlh, allowedips_nest);
	mnl_attr_nest_end(nlh, peer_nest);
	mnl_attr_nest_end(nlh, peers_nest);
	goto send;
toobig_peers:
	if (peer_nest)
		mnl_attr_nest_cancel(nlh, peer_nest);
	mnl_attr_nest_end(nlh, peers_nest);
	goto send;
send:
	if (mnlg_socket_send(nlg, nlh) < 0) {
		ret = -errno;
		goto out;
	}
	errno = 0;
	if (mnlg_socket_recv_run(nlg, NULL, NULL) < 0) {
		ret = errno ? -errno : -EINVAL;
		goto out;
	}
	if (peer)
		goto again;

out:
	mnlg_socket_close(nlg);
	errno = -ret;
	return ret;
}

static int parse_allowedip(const struct nlattr *attr, void *data)
{
	wg_allowedip *allowedip = data;

	switch (mnl_attr_get_type(attr)) {
	case WGALLOWEDIP_A_UNSPEC:
		break;
	case WGALLOWEDIP_A_FAMILY:
		if (!mnl_attr_validate(attr, MNL_TYPE_U16))
			allowedip->family = mnl_attr_get_u16(attr);
		break;
	case WGALLOWEDIP_A_IPADDR:
		if (mnl_attr_get_payload_len(attr) == sizeof(allowedip->ip4))
			memcpy(&allowedip->ip4, mnl_attr_get_payload(attr), sizeof(allowedip->ip4));
		else if (mnl_attr_get_payload_len(attr) == sizeof(allowedip->ip6))
			memcpy(&allowedip->ip6, mnl_attr_get_payload(attr), sizeof(allowedip->ip6));
		break;
	case WGALLOWEDIP_A_CIDR_MASK:
		if (!mnl_attr_validate(attr, MNL_TYPE_U8))
			allowedip->cidr = mnl_attr_get_u8(attr);
		break;
	}

	return MNL_CB_OK;
}

static int parse_allowedips(const struct nlattr *attr, void *data)
{
	wg_peer *peer = data;
	wg_allowedip *new_allowedip = calloc(1, sizeof(wg_allowedip));
	int ret;

	if (!new_allowedip)
		return MNL_CB_ERROR;
	if (!peer->first_allowedip)
		peer->first_allowedip = peer->last_allowedip = new_allowedip;
	else {
		peer->last_allowedip->next_allowedip = new_allowedip;
		peer->last_allowedip = new_allowedip;
	}
	ret = mnl_attr_parse_nested(attr, parse_allowedip, new_allowedip);
	if (!ret)
		return ret;
	if (!((new_allowedip->family == AF_INET && new_allowedip->cidr <= 32) || (new_allowedip->family == AF_INET6 && new_allowedip->cidr <= 128))) {
		errno = EAFNOSUPPORT;
		return MNL_CB_ERROR;
	}
	return MNL_CB_OK;
}

bool wg_key_is_zero(const wg_key key)
{
	volatile uint8_t acc = 0;
	unsigned int i;

	for (i = 0; i < sizeof(wg_key); ++i) {
		acc |= key[i];
		__asm__ ("" : "=r" (acc) : "0" (acc));
	}
	return 1 & ((acc - 1) >> 8);
}

static int parse_peer(const struct nlattr *attr, void *data)
{
	wg_peer *peer = data;

	switch (mnl_attr_get_type(attr)) {
	case WGPEER_A_UNSPEC:
		break;
	case WGPEER_A_PUBLIC_KEY:
		if (mnl_attr_get_payload_len(attr) == sizeof(peer->public_key)) {
			memcpy(peer->public_key, mnl_attr_get_payload(attr), sizeof(peer->public_key));
			peer->flags |= WGPEER_HAS_PUBLIC_KEY;
		}
		break;
	case WGPEER_A_PRESHARED_KEY:
		if (mnl_attr_get_payload_len(attr) == sizeof(peer->preshared_key)) {
			memcpy(peer->preshared_key, mnl_attr_get_payload(attr), sizeof(peer->preshared_key));
			if (!wg_key_is_zero(peer->preshared_key))
				peer->flags |= WGPEER_HAS_PRESHARED_KEY;
		}
		break;
	case WGPEER_A_ENDPOINT: {
		struct sockaddr *addr;

		if (mnl_attr_get_payload_len(attr) < sizeof(*addr))
			break;
		addr = mnl_attr_get_payload(attr);
		if (addr->sa_family == AF_INET && mnl_attr_get_payload_len(attr) == sizeof(peer->endpoint.addr4))
			memcpy(&peer->endpoint.addr4, addr, sizeof(peer->endpoint.addr4));
		else if (addr->sa_family == AF_INET6 && mnl_attr_get_payload_len(attr) == sizeof(peer->endpoint.addr6))
			memcpy(&peer->endpoint.addr6, addr, sizeof(peer->endpoint.addr6));
		break;
	}
	case WGPEER_A_PERSISTENT_KEEPALIVE_INTERVAL:
		if (!mnl_attr_validate(attr, MNL_TYPE_U16))
			peer->persistent_keepalive_interval = mnl_attr_get_u16(attr);
		break;
	case WGPEER_A_LAST_HANDSHAKE_TIME:
		if (mnl_attr_get_payload_len(attr) == sizeof(peer->last_handshake_time))
			memcpy(&peer->last_handshake_time, mnl_attr_get_payload(attr), sizeof(peer->last_handshake_time));
		break;
	case WGPEER_A_RX_BYTES:
		if (!mnl_attr_validate(attr, MNL_TYPE_U64))
			peer->rx_bytes = mnl_attr_get_u64(attr);
		break;
	case WGPEER_A_TX_BYTES:
		if (!mnl_attr_validate(attr, MNL_TYPE_U64))
			peer->tx_bytes = mnl_attr_get_u64(attr);
		break;
	case WGPEER_A_ALLOWEDIPS:
		return mnl_attr_parse_nested(attr, parse_allowedips, peer);
	}

	return MNL_CB_OK;
}

static int parse_peers(const struct nlattr *attr, void *data)
{
	wg_device *device = data;
	wg_peer *new_peer = calloc(1, sizeof(wg_peer));
	int ret;

	if (!new_peer)
		return MNL_CB_ERROR;
	if (!device->first_peer)
		device->first_peer = device->last_peer = new_peer;
	else {
		device->last_peer->next_peer = new_peer;
		device->last_peer = new_peer;
	}
	ret = mnl_attr_parse_nested(attr, parse_peer, new_peer);
	if (!ret)
		return ret;
	if (!(new_peer->flags & WGPEER_HAS_PUBLIC_KEY)) {
		errno = ENXIO;
		return MNL_CB_ERROR;
	}
	return MNL_CB_OK;
}

static int parse_device(const struct nlattr *attr, void *data)
{
	wg_device *device = data;

	switch (mnl_attr_get_type(attr)) {
	case WGDEVICE_A_UNSPEC:
		break;
	case WGDEVICE_A_IFINDEX:
		if (!mnl_attr_validate(attr, MNL_TYPE_U32))
			device->ifindex = mnl_attr_get_u32(attr);
		break;
	case WGDEVICE_A_IFNAME:
		if (!mnl_attr_validate(attr, MNL_TYPE_STRING)) {
			strncpy(device->name, mnl_attr_get_str(attr), sizeof(device->name) - 1);
			device->name[sizeof(device->name) - 1] = '\0';
		}
		break;
	case WGDEVICE_A_PRIVATE_KEY:
		if (mnl_attr_get_payload_len(attr) == sizeof(device->private_key)) {
			memcpy(device->private_key, mnl_attr_get_payload(attr), sizeof(device->private_key));
			device->flags |= WGDEVICE_HAS_PRIVATE_KEY;
		}
		break;
	case WGDEVICE_A_PUBLIC_KEY:
		if (mnl_attr_get_payload_len(attr) == sizeof(device->public_key)) {
			memcpy(device->public_key, mnl_attr_get_payload(attr), sizeof(device->public_key));
			device->flags |= WGDEVICE_HAS_PUBLIC_KEY;
		}
		break;
	case WGDEVICE_A_LISTEN_PORT:
		if (!mnl_attr_validate(attr, MNL_TYPE_U16))
			device->listen_port = mnl_attr_get_u16(attr);
		break;
	case WGDEVICE_A_FWMARK:
		if (!mnl_attr_validate(attr, MNL_TYPE_U32))
			device->fwmark = mnl_attr_get_u32(attr);
		break;
	case WGDEVICE_A_PEERS:
		return mnl_attr_parse_nested(attr, parse_peers, device);
	}

	return MNL_CB_OK;
}

static int read_device_cb(const struct nlmsghdr *nlh, void *data)
{
	return mnl_attr_parse(nlh, sizeof(struct genlmsghdr), parse_device, data);
}

static void coalesce_peers(wg_device *device)
{
	wg_peer *old_next_peer, *peer = device->first_peer;

	while (peer && peer->next_peer) {
		if (memcmp(peer->public_key, peer->next_peer->public_key, sizeof(wg_key))) {
			peer = peer->next_peer;
			continue;
		}
		if (!peer->first_allowedip) {
			peer->first_allowedip = peer->next_peer->first_allowedip;
			peer->last_allowedip = peer->next_peer->last_allowedip;
		} else {
			peer->last_allowedip->next_allowedip = peer->next_peer->first_allowedip;
			peer->last_allowedip = peer->next_peer->last_allowedip;
		}
		old_next_peer = peer->next_peer;
		peer->next_peer = old_next_peer->next_peer;
		free(old_next_peer);
	}
}

int wg_get_device(wg_device **device, const char *device_name)
{
	int ret = 0;
	struct nlmsghdr *nlh;
	struct mnlg_socket *nlg;

try_again:
	*device = calloc(1, sizeof(wg_device));
	if (!*device)
		return -errno;

	nlg = mnlg_socket_open(WG_GENL_NAME, WG_GENL_VERSION);
	if (!nlg) {
		wg_free_device(*device);
		*device = NULL;
		return -errno;
	}

	nlh = mnlg_msg_prepare(nlg, WG_CMD_GET_DEVICE, NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP);
	mnl_attr_put_strz(nlh, WGDEVICE_A_IFNAME, device_name);
	if (mnlg_socket_send(nlg, nlh) < 0) {
		ret = -errno;
		goto out;
	}
	errno = 0;
	if (mnlg_socket_recv_run(nlg, read_device_cb, *device) < 0) {
		ret = errno ? -errno : -EINVAL;
		goto out;
	}
	coalesce_peers(*device);

out:
	if (nlg)
		mnlg_socket_close(nlg);
	if (ret) {
		wg_free_device(*device);
		if (ret == -EINTR)
			goto try_again;
		*device = NULL;
	}
	errno = -ret;
	return ret;
}

/* first\0second\0third\0forth\0last\0\0 */
char *wg_list_device_names(void)
{
	struct string_list list = { 0 };
	int ret = fetch_device_names(&list);

	errno = -ret;
	if (errno) {
		free(list.buffer);
		return NULL;
	}
	return list.buffer ?: strdup("\0");
}

int wg_add_device(const char *device_name)
{
	return add_del_iface(device_name, true);
}

int wg_del_device(const char *device_name)
{
	return add_del_iface(device_name, false);
}

void wg_free_device(wg_device *dev)
{
	wg_peer *peer, *np;
	wg_allowedip *allowedip, *na;

	if (!dev)
		return;
	for (peer = dev->first_peer, np = peer ? peer->next_peer : NULL; peer; peer = np, np = peer ? peer->next_peer : NULL) {
		for (allowedip = peer->first_allowedip, na = allowedip ? allowedip->next_allowedip : NULL; allowedip; allowedip = na, na = allowedip ? allowedip->next_allowedip : NULL)
			free(allowedip);
		free(peer);
	}
	free(dev);
}

static void encode_base64(char dest[static 4], const uint8_t src[static 3])
{
	const uint8_t input[] = { (src[0] >> 2) & 63, ((src[0] << 4) | (src[1] >> 4)) & 63, ((src[1] << 2) | (src[2] >> 6)) & 63, src[2] & 63 };
	unsigned int i;

	for (i = 0; i < 4; ++i)
		dest[i] = input[i] + 'A'
			  + (((25 - input[i]) >> 8) & 6)
			  - (((51 - input[i]) >> 8) & 75)
			  - (((61 - input[i]) >> 8) & 15)
			  + (((62 - input[i]) >> 8) & 3);

}

void wg_key_to_base64(wg_key_b64_string base64, const wg_key key)
{
	unsigned int i;

	for (i = 0; i < 32 / 3; ++i)
		encode_base64(&base64[i * 4], &key[i * 3]);
	encode_base64(&base64[i * 4], (const uint8_t[]){ key[i * 3 + 0], key[i * 3 + 1], 0 });
	base64[sizeof(wg_key_b64_string) - 2] = '=';
	base64[sizeof(wg_key_b64_string) - 1] = '\0';
}

static int decode_base64(const char src[static 4])
{
	int val = 0;
	unsigned int i;

	for (i = 0; i < 4; ++i)
		val |= (-1
			    + ((((('A' - 1) - src[i]) & (src[i] - ('Z' + 1))) >> 8) & (src[i] - 64))
			    + ((((('a' - 1) - src[i]) & (src[i] - ('z' + 1))) >> 8) & (src[i] - 70))
			    + ((((('0' - 1) - src[i]) & (src[i] - ('9' + 1))) >> 8) & (src[i] + 5))
			    + ((((('+' - 1) - src[i]) & (src[i] - ('+' + 1))) >> 8) & 63)
			    + ((((('/' - 1) - src[i]) & (src[i] - ('/' + 1))) >> 8) & 64)
			) << (18 - 6 * i);
	return val;
}

int wg_key_from_base64(wg_key key, const wg_key_b64_string base64)
{
	unsigned int i;
	int val;
	volatile uint8_t ret = 0;

	if (strlen(base64) != sizeof(wg_key_b64_string) - 1 || base64[sizeof(wg_key_b64_string) - 2] != '=') {
		errno = EINVAL;
		goto out;
	}

	for (i = 0; i < 32 / 3; ++i) {
		val = decode_base64(&base64[i * 4]);
		ret |= (uint32_t)val >> 31;
		key[i * 3 + 0] = (val >> 16) & 0xff;
		key[i * 3 + 1] = (val >> 8) & 0xff;
		key[i * 3 + 2] = val & 0xff;
	}
	val = decode_base64((const char[]){ base64[i * 4 + 0], base64[i * 4 + 1], base64[i * 4 + 2], 'A' });
	ret |= ((uint32_t)val >> 31) | (val & 0xff);
	key[i * 3 + 0] = (val >> 16) & 0xff;
	key[i * 3 + 1] = (val >> 8) & 0xff;
	errno = EINVAL & ~((ret - 1) >> 8);
out:
	return -errno;
}

typedef int64_t fe[16];

static __attribute__((noinline)) void memzero_explicit(void *s, size_t count)
{
	memset(s, 0, count);
	__asm__ __volatile__("": :"r"(s) :"memory");
}

static void carry(fe o)
{
	int i;

	for (i = 0; i < 16; ++i) {
		o[(i + 1) % 16] += (i == 15 ? 38 : 1) * (o[i] >> 16);
		o[i] &= 0xffff;
	}
}

static void cswap(fe p, fe q, int b)
{
	int i;
	int64_t t, c = ~(b - 1);

	for (i = 0; i < 16; ++i) {
		t = c & (p[i] ^ q[i]);
		p[i] ^= t;
		q[i] ^= t;
	}

	memzero_explicit(&t, sizeof(t));
	memzero_explicit(&c, sizeof(c));
	memzero_explicit(&b, sizeof(b));
}

static void pack(uint8_t *o, const fe n)
{
	int i, j, b;
	fe m, t;

	memcpy(t, n, sizeof(t));
	carry(t);
	carry(t);
	carry(t);
	for (j = 0; j < 2; ++j) {
		m[0] = t[0] - 0xffed;
		for (i = 1; i < 15; ++i) {
			m[i] = t[i] - 0xffff - ((m[i - 1] >> 16) & 1);
			m[i - 1] &= 0xffff;
		}
		m[15] = t[15] - 0x7fff - ((m[14] >> 16) & 1);
		b = (m[15] >> 16) & 1;
		m[14] &= 0xffff;
		cswap(t, m, 1 - b);
	}
	for (i = 0; i < 16; ++i) {
		o[2 * i] = t[i] & 0xff;
		o[2 * i + 1] = t[i] >> 8;
	}

	memzero_explicit(m, sizeof(m));
	memzero_explicit(t, sizeof(t));
	memzero_explicit(&b, sizeof(b));
}

static void add(fe o, const fe a, const fe b)
{
	int i;

	for (i = 0; i < 16; ++i)
		o[i] = a[i] + b[i];
}

static void subtract(fe o, const fe a, const fe b)
{
	int i;

	for (i = 0; i < 16; ++i)
		o[i] = a[i] - b[i];
}

static void multmod(fe o, const fe a, const fe b)
{
	int i, j;
	int64_t t[31] = { 0 };

	for (i = 0; i < 16; ++i) {
		for (j = 0; j < 16; ++j)
			t[i + j] += a[i] * b[j];
	}
	for (i = 0; i < 15; ++i)
		t[i] += 38 * t[i + 16];
	memcpy(o, t, sizeof(fe));
	carry(o);
	carry(o);

	memzero_explicit(t, sizeof(t));
}

static void invert(fe o, const fe i)
{
	fe c;
	int a;

	memcpy(c, i, sizeof(c));
	for (a = 253; a >= 0; --a) {
		multmod(c, c, c);
		if (a != 2 && a != 4)
			multmod(c, c, i);
	}
	memcpy(o, c, sizeof(fe));

	memzero_explicit(c, sizeof(c));
}

static void clamp_key(uint8_t *z)
{
	z[31] = (z[31] & 127) | 64;
	z[0] &= 248;
}

void wg_generate_public_key(wg_key public_key, const wg_key private_key)
{
	int i, r;
	uint8_t z[32];
	fe a = { 1 }, b = { 9 }, c = { 0 }, d = { 1 }, e, f;

	memcpy(z, private_key, sizeof(z));
	clamp_key(z);

	for (i = 254; i >= 0; --i) {
		r = (z[i >> 3] >> (i & 7)) & 1;
		cswap(a, b, r);
		cswap(c, d, r);
		add(e, a, c);
		subtract(a, a, c);
		add(c, b, d);
		subtract(b, b, d);
		multmod(d, e, e);
		multmod(f, a, a);
		multmod(a, c, a);
		multmod(c, b, e);
		add(e, a, c);
		subtract(a, a, c);
		multmod(b, a, a);
		subtract(c, d, f);
		multmod(a, c, (const fe){ 0xdb41, 1 });
		add(a, a, d);
		multmod(c, c, a);
		multmod(a, d, f);
		multmod(d, b, (const fe){ 9 });
		multmod(b, e, e);
		cswap(a, b, r);
		cswap(c, d, r);
	}
	invert(c, c);
	multmod(a, a, c);
	pack(public_key, a);

	memzero_explicit(&r, sizeof(r));
	memzero_explicit(z, sizeof(z));
	memzero_explicit(a, sizeof(a));
	memzero_explicit(b, sizeof(b));
	memzero_explicit(c, sizeof(c));
	memzero_explicit(d, sizeof(d));
	memzero_explicit(e, sizeof(e));
	memzero_explicit(f, sizeof(f));
}

void wg_generate_private_key(wg_key private_key)
{
	wg_generate_preshared_key(private_key);
	clamp_key(private_key);
}

void wg_generate_preshared_key(wg_key preshared_key)
{
	ssize_t ret;
	size_t i;
	int fd;
#if defined(__OpenBSD__) || (defined(__APPLE__) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12) || (defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 25)))
	if (!getentropy(preshared_key, sizeof(wg_key)))
		return;
#endif
#if defined(__NR_getrandom) && defined(__linux__)
	if (syscall(__NR_getrandom, preshared_key, sizeof(wg_key), 0) == sizeof(wg_key))
		return;
#endif
	fd = open("/dev/urandom", O_RDONLY);
	assert(fd >= 0);
	for (i = 0; i < sizeof(wg_key); i += ret) {
		ret = read(fd, preshared_key + i, sizeof(wg_key) - i);
		assert(ret > 0);
	}
	close(fd);
}
