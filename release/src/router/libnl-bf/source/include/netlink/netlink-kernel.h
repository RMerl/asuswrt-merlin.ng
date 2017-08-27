#ifndef __NETLINK_KERNEL_H_
#define __NETLINK_KERNEL_H_

#if 0

/*
 * FIXME: Goal is to preseve the documentation but make it simple
 * to keep linux/netlink.h in sync. Maybe use named documentation
 * sections.
 */

/**
 * Netlink socket address
 * @ingroup nl
 */
struct sockaddr_nl
{
	/** socket family (AF_NETLINK) */
	sa_family_t     nl_family;

	/** Padding (unused) */
	unsigned short  nl_pad;

	/** Unique process ID  */
	uint32_t        nl_pid;

	/** Multicast group subscriptions */
	uint32_t        nl_groups;
};

/**
 * @addtogroup msg
 * @{
 */


/**
 * Netlink message header
 */
struct nlmsghdr
{
	/** Length of message including header and padding. */
	uint32_t	nlmsg_len;

	/** Message type (content type) */
	uint16_t	nlmsg_type;

	/** Message flags */
	uint16_t	nlmsg_flags;

	/** Sequence number of message \see core_sk_seq_num. */
	uint32_t	nlmsg_seq;

	/** Netlink port */
	uint32_t	nlmsg_pid;
};

/**
 * @name Standard message flags
 * @{
 */

/**
 * Must be set on all request messages (typically from user space to
 * kernel space).
 */
#define NLM_F_REQUEST		1

/**
 * Indicates the message is part of a multipart message terminated
 * by NLMSG_DONE.
 */
#define NLM_F_MULTI		2

/**
 * Request for an acknowledgment on success.
 */
#define NLM_F_ACK		4

/**
 * Echo this request
 */
#define NLM_F_ECHO		8

/** @} */

/**
 * @name Additional message flags for GET requests
 * @{
 */

/**
 * Return the complete table instead of a single entry.
 */
#define NLM_F_ROOT	0x100

/**
 * Return all entries matching criteria passed in message content.
 */
#define NLM_F_MATCH	0x200

/**
 * Return an atomic snapshot of the table being referenced. This
 * may require special privileges because it has the potential to
 * interrupt service in the FE for a longer time.
 */
#define NLM_F_ATOMIC	0x400

/**
 * Dump all entries
 */
#define NLM_F_DUMP	(NLM_F_ROOT|NLM_F_MATCH)

/** @} */

/**
 * @name Additional messsage flags for NEW requests
 * @{
 */

/**
 * Replace existing matching config object with this request.
 */
#define NLM_F_REPLACE	0x100

/**
 * Don't replace the config object if it already exists.
 */
#define NLM_F_EXCL	0x200

/**
 * Create config object if it doesn't already exist.
 */
#define NLM_F_CREATE	0x400

/**
 * Add to the end of the object list.
 */
#define NLM_F_APPEND	0x800

/** @} */

/**
 * @name Standard Message types
 * @{
 */

/**
 * No operation, message must be ignored
 */
#define NLMSG_NOOP		0x1

/**
 * The message signals an error and the payload contains a nlmsgerr
 * structure. This can be looked at as a NACK and typically it is
 * from FEC to CPC.
 */
#define NLMSG_ERROR		0x2

/**
 * Message terminates a multipart message.
 */
#define NLMSG_DONE		0x3

/**
 * The message signals that data got lost
 */
#define NLMSG_OVERRUN		0x4

/**
 * Lower limit of reserved message types
 */
#define NLMSG_MIN_TYPE		0x10

/** @} */

/**
 * Netlink error message header
 */
struct nlmsgerr
{
	/** Error code (errno number) */
	int		error;

	/** Original netlink message causing the error */
	struct nlmsghdr	msg;
};

struct nl_pktinfo
{
	__u32	group;
};

/**
 * Netlink alignment constant, all boundries within messages must be align to this.
 *
 * See \ref core_msg_fmt_align for more information on message alignment.
 */
#define NLMSG_ALIGNTO	4

/**
 * Returns \p len properly aligned to NLMSG_ALIGNTO.
 *
 * See \ref core_msg_fmt_align for more information on message alignment.
 */
#define NLMSG_ALIGN(len) ( ((len)+NLMSG_ALIGNTO-1) & ~(NLMSG_ALIGNTO-1) )

/**
 * Length of a netlink message header including padding.
 *
 * See \ref core_msg_fmt_align for more information on message alignment.
 */
#define NLMSG_HDRLEN	 ((int) NLMSG_ALIGN(sizeof(struct nlmsghdr)))

/** @} */

/**
 * @addtogroup attr
 * @{
 */

/*
 */

/**
 * Netlink attribute structure
 *
 * @code
 *  <------- NLA_HDRLEN ------> <-- NLA_ALIGN(payload)-->
 * +---------------------+- - -+- - - - - - - - - -+- - -+
 * |        Header       | Pad |     Payload       | Pad |
 * |   (struct nlattr)   | ing |                   | ing |
 * +---------------------+- - -+- - - - - - - - - -+- - -+
 *  <-------------- nlattr->nla_len -------------->
 * @endcode
 */
struct nlattr {
	/**
	 * Attribute length in bytes including header
	 */
	__u16           nla_len;

	/**
	 * Netlink attribute type
	 */
	__u16           nla_type;
};

/**
 * @name Attribute Type Flags
 *
 * @code
 * nla_type (16 bits)
 * +---+---+-------------------------------+
 * | N | O | Attribute Type                |
 * +---+---+-------------------------------+
 * N := Carries nested attributes
 * O := Payload stored in network byte order
 * @endcode
 *
 * @note The N and O flag are mutually exclusive.
 *
 * @{
 */

/*
 */
#define NLA_F_NESTED		(1 << 15)
#define NLA_F_NET_BYTEORDER	(1 << 14)
#define NLA_TYPE_MASK		~(NLA_F_NESTED | NLA_F_NET_BYTEORDER)

/** @} */

#define NLA_ALIGNTO		4

/**
 * Returns \p len properly aligned to NLA_ALIGNTO.
 *
 * See \ref core_msg_fmt_align for more information on message alignment.
 */
#define NLA_ALIGN(len)		(((len) + NLA_ALIGNTO - 1) & ~(NLA_ALIGNTO - 1))

/**
 * Length of a netlink attribute header including padding.
 *
 * See \ref core_msg_fmt_align for more information on message alignment.
 */
#define NLA_HDRLEN		((int) NLA_ALIGN(sizeof(struct nlattr)))

/** @} */

#endif
#endif	/* __LINUX_NETLINK_H */
