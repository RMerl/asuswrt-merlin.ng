/*
 * msgbuff.h - netlink message buffer
 *
 * Declarations of netlink message buffer and related functions.
 */

#ifndef ETHTOOL_NETLINK_MSGBUFF_H__
#define ETHTOOL_NETLINK_MSGBUFF_H__

#include <string.h>
#include <../../libmnl-1.0.4/include/libmnl/libmnl.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>

struct nl_context;

/**
 * struct nl_msg_buff - message buffer abstraction
 * @buff:    pointer to buffer
 * @size:    total size of allocated buffer
 * @left:    remaining length current message end to end of buffer
 * @nlhdr:   pointer to netlink header of current message
 * @genlhdr: pointer to genetlink header of current message
 * @payload: pointer to message payload (after genetlink header)
 */
struct nl_msg_buff {
	char			*buff;
	unsigned int		size;
	unsigned int		left;
	struct nlmsghdr		*nlhdr;
	struct genlmsghdr	*genlhdr;
	void			*payload;
};

void msgbuff_init(struct nl_msg_buff *msgbuff);
void msgbuff_done(struct nl_msg_buff *msgbuff);
int msgbuff_realloc(struct nl_msg_buff *msgbuff, unsigned int new_size);
int msgbuff_append(struct nl_msg_buff *dest, struct nl_msg_buff *src);

int __msg_init(struct nl_msg_buff *msgbuff, int family, int cmd,
	       unsigned int flags, int version);
int msg_init(struct nl_context *nlctx, struct nl_msg_buff *msgbuff, int cmd,
	     unsigned int flags);

bool ethnla_put(struct nl_msg_buff *msgbuff, uint16_t type, size_t len,
		const void *data);
struct nlattr *ethnla_nest_start(struct nl_msg_buff *msgbuff, uint16_t type);
bool ethnla_fill_header(struct nl_msg_buff *msgbuff, uint16_t type,
			const char *devname, uint32_t flags);

/* length of current message */
static inline unsigned int msgbuff_len(const struct nl_msg_buff *msgbuff)
{
	return msgbuff->nlhdr->nlmsg_len;
}

/* reset message length to position returned by msgbuff_len() */
static inline void msgbuff_reset(const struct nl_msg_buff *msgbuff,
				 unsigned int len)
{
	msgbuff->nlhdr->nlmsg_len = len;
}

/* put data wrappers */

static inline void ethnla_nest_end(struct nl_msg_buff *msgbuff,
				   struct nlattr *nest)
{
	mnl_attr_nest_end(msgbuff->nlhdr, nest);
}

static inline void ethnla_nest_cancel(struct nl_msg_buff *msgbuff,
				      struct nlattr *nest)
{
	mnl_attr_nest_cancel(msgbuff->nlhdr, nest);
}

static inline bool ethnla_put_u32(struct nl_msg_buff *msgbuff, uint16_t type,
				  uint32_t data)
{
	return ethnla_put(msgbuff, type, sizeof(uint32_t), &data);
}

static inline bool ethnla_put_u16(struct nl_msg_buff *msgbuff, uint16_t type,
				  uint16_t data)
{
	return ethnla_put(msgbuff, type, sizeof(uint16_t), &data);
}

static inline bool ethnla_put_u8(struct nl_msg_buff *msgbuff, uint16_t type,
				 uint8_t data)
{
	return ethnla_put(msgbuff, type, sizeof(uint8_t), &data);
}

static inline bool ethnla_put_flag(struct nl_msg_buff *msgbuff, uint16_t type,
				   bool val)
{
	if (val)
		return ethnla_put(msgbuff, type, 0, &val);
	else
		return false;
}

static inline bool ethnla_put_bitfield32(struct nl_msg_buff *msgbuff,
					 uint16_t type, uint32_t value,
					 uint32_t selector)
{
	struct nla_bitfield32 val = {
		.value		= value,
		.selector	= selector,
	};

	return ethnla_put(msgbuff, type, sizeof(val), &val);
}

static inline bool ethnla_put_strz(struct nl_msg_buff *msgbuff, uint16_t type,
				   const char *data)
{
	return ethnla_put(msgbuff, type, strlen(data) + 1, data);
}

#endif /* ETHTOOL_NETLINK_MSGBUFF_H__ */
