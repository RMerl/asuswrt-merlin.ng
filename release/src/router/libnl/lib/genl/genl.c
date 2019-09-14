/*
 * lib/genl/genl.c		Generic Netlink
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @defgroup genl Generic Netlink Library (libnl-genl)
 *
 * @{
 */

#include <netlink-private/genl.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/utils.h>

/**
 * @name Generic Netlink Socket
 * @{
 */

/**
 * Connect a Generic Netlink socket
 * @arg sk		Unconnected Netlink socket
 *
 * This function expects a struct nl_socket object previously allocated via
 * nl_socket_alloc(). It calls nl_connect() to create the local socket file
 * descriptor and binds the socket to the \c NETLINK_GENERIC Netlink protocol.
 *
 * Using this function is equivalent to:
 * @code
 * nl_connect(sk, NETLINK_GENERIC);
 * @endcode
 *
 * @see nl_connect()
 *
 * @return 0 on success or a negative error code.
 */
int genl_connect(struct nl_sock *sk)
{
	return nl_connect(sk, NETLINK_GENERIC);
}

/** @} */

/**
 * @name Sending Data
 * @{
 */

/**
 * Send a Generic Netlink message consisting only of a header
 * @arg sk		Generic Netlink socket
 * @arg family		Numeric family identifier
 * @arg cmd		Numeric command identifier
 * @arg version		Interface version
 * @arg flags		Additional Netlink message flags (optional)
 *
 * This function is a shortcut for sending a Generic Netlink message without
 * any message payload. The message will only consist of the Netlink and
 * Generic Netlink headers. The header is constructed based on the specified
 * parameters and passed on to nl_send_simple() to send it on the specified
 * socket.
 *
 * @par Example:
 * @code
 * #include <netlink/genl/genl.h>
 * #include <linux/genetlink.h>
 *
 * err = genl_send_simple(sk, GENL_ID_CTRL, CTRL_CMD_GETFAMILY, CTRL_VERSION,
 *                        NLM_F_DUMP);
 * @endcode
 *
 * @see nl_send_simple()
 *
 * @return 0 on success or a negative error code.
 */
int genl_send_simple(struct nl_sock *sk, int family, int cmd,
		     int version, int flags)
{
	struct genlmsghdr hdr = {
		.cmd = cmd,
		.version = version,
	};

	return nl_send_simple(sk, family, flags, &hdr, sizeof(hdr));
}

/** @} */

/**
 * @name Message Parsing
 * @{
 */

/**
 * Validate Generic Netlink message headers
 * @arg nlh		Pointer to Netlink message header
 * @arg hdrlen		Length of user header
 *
 * Verifies the integrity of the Netlink and Generic Netlink headers by
 * enforcing the following requirements:
 *  - Valid Netlink message header (nlmsg_valid_hdr())
 *  - Presence of a complete Generic Netlink header
 *  - At least \c hdrlen bytes of payload included after the generic
 *    netlink header.
 *
 * @return A positive integer (true) if the headers are valid or
 *         0 (false) if not.
 */
int genlmsg_valid_hdr(struct nlmsghdr *nlh, int hdrlen)
{
	struct genlmsghdr *ghdr;

	if (!nlmsg_valid_hdr(nlh, GENL_HDRLEN))
		return 0;

	ghdr = nlmsg_data(nlh);
	if (genlmsg_len(ghdr) < NLMSG_ALIGN(hdrlen))
		return 0;

	return 1;
}

/**
 * Validate Generic Netlink message including attributes
 * @arg nlh		Pointer to Netlink message header
 * @arg hdrlen		Length of user header
 * @arg maxtype		Maximum attribtue id expected
 * @arg policy		Attribute validation policy
 *
 * Verifies the validity of the Netlink and Generic Netlink headers using
 * genlmsg_valid_hdr() and calls nla_validate() on the message payload to
 * verify the integrity of eventual attributes.
 *
 * @note You may call genlmsg_parse() directly to perform validation and
 *       parsing in a single step. 
 *
 * @see genlmsg_valid_hdr()
 * @see nla_validate()
 * @see genlmsg_parse()
 *
 * @return 0 on success or a negative error code.
 */
int genlmsg_validate(struct nlmsghdr *nlh, int hdrlen, int maxtype,
		     struct nla_policy *policy)
{
	struct genlmsghdr *ghdr;

	if (!genlmsg_valid_hdr(nlh, hdrlen))
		return -NLE_MSG_TOOSHORT;

	ghdr = nlmsg_data(nlh);
	return nla_validate(genlmsg_attrdata(ghdr, hdrlen),
			    genlmsg_attrlen(ghdr, hdrlen), maxtype, policy);
}

/**
 * Parse Generic Netlink message including attributes
 * @arg nlh		Pointer to Netlink message header
 * @arg hdrlen		Length of user header
 * @arg tb		Array to store parsed attributes
 * @arg maxtype		Maximum attribute id expected
 * @arg policy		Attribute validation policy
 *
 * Verifies the validity of the Netlink and Generic Netlink headers using
 * genlmsg_valid_hdr() and calls nla_parse() on the message payload to
 * parse eventual attributes.
 *
 * @par Example:
 * @code
 * struct nlattr *attrs[MY_TYPE_MAX+1];
 *
 * if ((err = genlsmg_parse(nlmsg_nlh(msg), sizeof(struct my_hdr), attrs,
 *                          MY_TYPE_MAX, attr_policy)) < 0)
 * 	// ERROR
 * @endcode
 *
 * @see genlmsg_valid_hdr()
 * @see genlmsg_validate()
 * @see nla_parse()
 *
 * @return 0 on success or a negative error code.
 */
int genlmsg_parse(struct nlmsghdr *nlh, int hdrlen, struct nlattr *tb[],
		  int maxtype, struct nla_policy *policy)
{
	struct genlmsghdr *ghdr;

	if (!genlmsg_valid_hdr(nlh, hdrlen))
		return -NLE_MSG_TOOSHORT;

	ghdr = nlmsg_data(nlh);
	return nla_parse(tb, maxtype, genlmsg_attrdata(ghdr, hdrlen),
			 genlmsg_attrlen(ghdr, hdrlen), policy);
}

/**
 * Return pointer to Generic Netlink header
 * @arg nlh		Netlink message header
 *
 * @return Pointer to Generic Netlink message header
 */
struct genlmsghdr *genlmsg_hdr(struct nlmsghdr *nlh)
{
	return nlmsg_data(nlh);
}

/**
 * Return length of message payload including user header
 * @arg gnlh		Generic Netlink message header
 *
 * @see genlmsg_data()
 *
 * @return Length of user payload including an eventual user header in
 *         number of bytes.
 */
int genlmsg_len(const struct genlmsghdr *gnlh)
{
	const struct nlmsghdr *nlh;

	nlh = (const struct nlmsghdr *)((const unsigned char *) gnlh - NLMSG_HDRLEN);
	return (nlh->nlmsg_len - GENL_HDRLEN - NLMSG_HDRLEN);
}


/**
 * Return pointer to user header
 * @arg gnlh		Generic Netlink message header
 *
 * Calculates the pointer to the user header based on the pointer to
 * the Generic Netlink message header.
 *
 * @return Pointer to the user header
 */
void *genlmsg_user_hdr(const struct genlmsghdr *gnlh)
{
	return genlmsg_data(gnlh);
}

/**
 * Return pointer to user data
 * @arg gnlh		Generic netlink message header
 * @arg hdrlen		Length of user header
 *
 * Calculates the pointer to the user data based on the pointer to
 * the Generic Netlink message header.
 *
 * @see genlmsg_user_datalen()
 *
 * @return Pointer to the user data
 */
void *genlmsg_user_data(const struct genlmsghdr *gnlh, const int hdrlen)
{
	return genlmsg_user_hdr(gnlh) + NLMSG_ALIGN(hdrlen);
}

/**
 * Return length of user data
 * @arg gnlh		Generic Netlink message header
 * @arg hdrlen		Length of user header
 *
 * @see genlmsg_user_data()
 *
 * @return Length of user data in bytes
 */
int genlmsg_user_datalen(const struct genlmsghdr *gnlh, const int hdrlen)
{
	return genlmsg_len(gnlh) - NLMSG_ALIGN(hdrlen);
}

/**
 * Return pointer to message attributes
 * @arg gnlh		Generic Netlink message header
 * @arg hdrlen		Length of user header
 *
 * @see genlmsg_attrlen()
 *
 * @return Pointer to the start of the message's attributes section.
 */
struct nlattr *genlmsg_attrdata(const struct genlmsghdr *gnlh, int hdrlen)
{
	return genlmsg_user_data(gnlh, hdrlen);
}

/**
 * Return length of message attributes
 * @arg gnlh		Generic Netlink message header
 * @arg hdrlen		Length of user header
 *
 * @see genlmsg_attrdata()
 *
 * @return Length of the message section containing attributes in number
 *         of bytes.
 */
int genlmsg_attrlen(const struct genlmsghdr *gnlh, int hdrlen)
{
	return genlmsg_len(gnlh) - NLMSG_ALIGN(hdrlen);
}

/** @} */

/**
 * @name Message Construction
 * @{
 */

/**
 * Add Generic Netlink headers to Netlink message
 * @arg msg		Netlink message object
 * @arg port		Netlink port or NL_AUTO_PORT
 * @arg seq		Sequence number of message or NL_AUTO_SEQ
 * @arg family		Numeric family identifier
 * @arg hdrlen		Length of user header
 * @arg flags		Additional Netlink message flags (optional)
 * @arg cmd		Numeric command identifier
 * @arg version		Interface version
 *
 * Calls nlmsg_put() on the specified message object to reserve space for
 * the Netlink header, the Generic Netlink header, and a user header of
 * specified length. Fills out the header fields with the specified
 * parameters.
 *
 * @par Example:
 * @code
 * struct nl_msg *msg;
 * struct my_hdr *user_hdr;
 *
 * if (!(msg = nlmsg_alloc()))
 * 	// ERROR
 *
 * user_hdr = genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, family_id,
 *                        sizeof(struct my_hdr), 0, MY_CMD_FOO, 0);
 * if (!user_hdr)
 * 	// ERROR
 * @endcode
 *
 * @see nlmsg_put()
 *
 * Returns Pointer to user header or NULL if an error occurred.
 */
void *genlmsg_put(struct nl_msg *msg, uint32_t port, uint32_t seq, int family,
		  int hdrlen, int flags, uint8_t cmd, uint8_t version)
{
	struct nlmsghdr *nlh;
	struct genlmsghdr hdr = {
		.cmd = cmd,
		.version = version,
	};

	nlh = nlmsg_put(msg, port, seq, family, GENL_HDRLEN + hdrlen, flags);
	if (nlh == NULL)
		return NULL;

	memcpy(nlmsg_data(nlh), &hdr, sizeof(hdr));
	NL_DBG(2, "msg %p: Added generic netlink header cmd=%d version=%d\n",
	       msg, cmd, version);

	return nlmsg_data(nlh) + GENL_HDRLEN;
}

/** @} */

/**
 * @name Deprecated
 * @{
 */

/**
 * Return pointer to message payload
 * @arg gnlh		Generic Netlink message header
 *
 * @deprecated This function has been deprecated due to inability to specify
 *             the length of the user header. Use genlmsg_user_hdr()
 *             respectively genlmsg_user_data().
 *
 * @return Pointer to payload section
 */
void *genlmsg_data(const struct genlmsghdr *gnlh)
{
	return ((unsigned char *) gnlh + GENL_HDRLEN);
}

/** @} */
/** @} */
