/*
 * msgbuff.c - netlink message buffer
 *
 * Data structures and code for flexible message buffer abstraction.
 */

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#include "../internal.h"
#include "netlink.h"
#include "msgbuff.h"

#define MAX_MSG_SIZE (4 << 20)		/* 4 MB */

/**
 * msgbuff_realloc() - reallocate buffer if needed
 * @msgbuff:  message buffer
 * @new_size: requested minimum size (add MNL_SOCKET_BUFFER_SIZE if zero)
 *
 * Make sure allocated buffer has size at least @new_size. If @new_size is
 * shorter than current size, do nothing. If @new_size is 0, grow buffer by
 * MNL_SOCKET_BUFFER_SIZE. Fail if new size would exceed MAX_MSG_SIZE.
 *
 * Return: 0 on success or negative error code
 */
int msgbuff_realloc(struct nl_msg_buff *msgbuff, unsigned int new_size)
{
	unsigned int nlhdr_off, genlhdr_off, payload_off;
	unsigned int old_size = msgbuff->size;
	char *nbuff;

	nlhdr_off = (char *)msgbuff->nlhdr - msgbuff->buff;
	genlhdr_off = (char *)msgbuff->genlhdr - msgbuff->buff;
	payload_off = (char *)msgbuff->payload - msgbuff->buff;

	if (!new_size)
		new_size = old_size + MNL_SOCKET_BUFFER_SIZE;
	if (new_size <= old_size)
		return 0;
	if (new_size > MAX_MSG_SIZE)
		return -EMSGSIZE;
	nbuff = realloc(msgbuff->buff, new_size);
	if (!nbuff) {
		msgbuff->buff = NULL;
		msgbuff->size = 0;
		msgbuff->left = 0;
		return -ENOMEM;
	}
	if (nbuff != msgbuff->buff) {
		if (new_size > old_size)
			memset(nbuff + old_size, '\0', new_size - old_size);
		msgbuff->nlhdr = (struct nlmsghdr *)(nbuff + nlhdr_off);
		msgbuff->genlhdr = (struct genlmsghdr *)(nbuff + genlhdr_off);
		msgbuff->payload = nbuff + payload_off;
		msgbuff->buff = nbuff;
	}
	msgbuff->size = new_size;
	msgbuff->left += (new_size - old_size);

	return 0;
}

/**
 * msgbuff_append() - add contents of another message buffer
 * @dest: target message buffer
 * @src:  source message buffer
 *
 * Append contents of @src at the end of @dest. Fail if target buffer cannot
 * be reallocated to sufficient size.
 *
 * Return: 0 on success or negative error code.
 */
int msgbuff_append(struct nl_msg_buff *dest, struct nl_msg_buff *src)
{
	unsigned int src_len = mnl_nlmsg_get_payload_len(src->nlhdr);
	unsigned int dest_len = MNL_ALIGN(msgbuff_len(dest));
	int ret;

	src_len -= GENL_HDRLEN;
	ret = msgbuff_realloc(dest, dest_len + src_len);
	if (ret < 0)
		return ret;
	memcpy(mnl_nlmsg_get_payload_tail(dest->nlhdr), src->payload, src_len);
	msgbuff_reset(dest, dest_len + src_len);

	return 0;
}

/**
 * ethnla_put - write a netlink attribute to message buffer
 * @msgbuff: message buffer
 * @type:    attribute type
 * @len:     attribute payload length
 * @data:    attribute payload
 *
 * Appends a netlink attribute with header to message buffer, reallocates
 * if needed. This is mostly used via specific ethnla_put_* wrappers for
 * basic data types.
 *
 * Return: false on success, true on error (reallocation failed)
 */
bool ethnla_put(struct nl_msg_buff *msgbuff, uint16_t type, size_t len,
		const void *data)
{
	struct nlmsghdr *nlhdr = msgbuff->nlhdr;

	while (!mnl_attr_put_check(nlhdr, msgbuff->left, type, len, data)) {
		int ret = msgbuff_realloc(msgbuff, 0);

		if (ret < 0)
			return true;
	}

	return false;
}

/**
 * ethnla_nest_start - start a nested attribute
 * @msgbuff: message buffer
 * @type:    nested attribute type (NLA_F_NESTED is added automatically)
 *
 * Return: pointer to the nest attribute or null of error
 */
struct nlattr *ethnla_nest_start(struct nl_msg_buff *msgbuff, uint16_t type)
{
	struct nlmsghdr *nlhdr = msgbuff->nlhdr;
	struct nlattr *attr;

	do {
		attr = mnl_attr_nest_start_check(nlhdr, msgbuff->left, type);
		if (attr)
			return attr;
	} while (msgbuff_realloc(msgbuff, 0) == 0);

	return NULL;
}

/**
 * ethnla_fill_header() - write standard ethtool request header to message
 * @msgbuff: message buffer
 * @type:    attribute type for header nest
 * @devname: device name (NULL to omit)
 * @flags:   request flags (omitted if 0)
 *
 * Return: pointer to the nest attribute or null of error
 */
bool ethnla_fill_header(struct nl_msg_buff *msgbuff, uint16_t type,
			const char *devname, uint32_t flags)
{
	struct nlattr *nest;

	nest = ethnla_nest_start(msgbuff, type);
	if (!nest)
		return true;

	if ((devname &&
	     ethnla_put_strz(msgbuff, ETHTOOL_A_HEADER_DEV_NAME, devname)) ||
	    (flags &&
	     ethnla_put_u32(msgbuff, ETHTOOL_A_HEADER_FLAGS, flags)))
		goto err;

	ethnla_nest_end(msgbuff, nest);
	return false;

err:
	ethnla_nest_cancel(msgbuff, nest);
	return true;
}

/**
 * __msg_init() - init a genetlink message, fill netlink and genetlink header
 * @msgbuff: message buffer
 * @family:  genetlink family
 * @cmd:     genetlink command (genlmsghdr::cmd)
 * @flags:   netlink flags (nlmsghdr::nlmsg_flags)
 * @version: genetlink family version (genlmsghdr::version)
 *
 * Initialize a new genetlink message, fill netlink and genetlink header and
 * set pointers in struct nl_msg_buff.
 *
 * Return: 0 on success or negative error code.
 */
int __msg_init(struct nl_msg_buff *msgbuff, int family, int cmd,
	       unsigned int flags, int version)
{
	struct nlmsghdr *nlhdr;
	struct genlmsghdr *genlhdr;
	int ret;

	ret = msgbuff_realloc(msgbuff, MNL_SOCKET_BUFFER_SIZE);
	if (ret < 0)
		return ret;
	memset(msgbuff->buff, '\0', NLMSG_HDRLEN + GENL_HDRLEN);

	nlhdr = mnl_nlmsg_put_header(msgbuff->buff);
	nlhdr->nlmsg_type = family;
	nlhdr->nlmsg_flags = flags;
	msgbuff->nlhdr = nlhdr;

	genlhdr = mnl_nlmsg_put_extra_header(nlhdr, sizeof(*genlhdr));
	genlhdr->cmd = cmd;
	genlhdr->version = version;
	msgbuff->genlhdr = genlhdr;

	msgbuff->payload = mnl_nlmsg_get_payload_offset(nlhdr, GENL_HDRLEN);

	return 0;
}

/**
 * msg_init() - init an ethtool netlink message
 * @msgbuff: message buffer
 * @cmd:     genetlink command (genlmsghdr::cmd)
 * @flags:   netlink flags (nlmsghdr::nlmsg_flags)
 *
 * Initialize a new ethtool netlink message, fill netlink and genetlink header
 * and set pointers in struct nl_msg_buff.
 *
 * Return: 0 on success or negative error code.
 */
int msg_init(struct nl_context *nlctx, struct nl_msg_buff *msgbuff, int cmd,
	     unsigned int flags)
{
	return __msg_init(msgbuff, nlctx->ethnl_fam, cmd, flags,
			  ETHTOOL_GENL_VERSION);
}

/**
 * msgbuff_init() - initialize a message buffer
 * @msgbuff: message buffer
 *
 * Initialize a message buffer structure before first use. Buffer length is
 * set to zero and the buffer is not allocated until the first call to
 * msgbuff_reallocate().
 */
void msgbuff_init(struct nl_msg_buff *msgbuff)
{
	memset(msgbuff, '\0', sizeof(*msgbuff));
}

/**
 * msg_done() - destroy a message buffer
 * @msgbuff: message buffer
 *
 * Free the buffer and reset size and remaining size.
 */
void msgbuff_done(struct nl_msg_buff *msgbuff)
{
	free(msgbuff->buff);
	msgbuff->buff = NULL;
	msgbuff->size = 0;
	msgbuff->left = 0;
}
