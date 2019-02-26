/*
 * Copyright (C) 2008 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#ifndef KERNEL_NETLINK_SHARED_H_
#define KERNEL_NETLINK_SHARED_H_

#include <library.h>

#include <linux/rtnetlink.h>

/**
 * Default buffer size.
 *
 * 1024 byte is currently sufficient for all operations.
 */
#ifndef KERNEL_NETLINK_BUFSIZE
#define KERNEL_NETLINK_BUFSIZE 1024
#endif

/**
 * General purpose netlink buffer.
 *
 * Some platforms require an enforced alignment to four bytes (e.g. ARM).
 */
typedef union {
	struct nlmsghdr hdr;
	u_char bytes[KERNEL_NETLINK_BUFSIZE];
} netlink_buf_t __attribute__((aligned(RTA_ALIGNTO)));

typedef struct netlink_socket_t netlink_socket_t;

/**
 * Wrapper around a netlink socket.
 */
struct netlink_socket_t {

	/**
	 * Send a netlink message and wait for a reply.
	 *
	 * @param	in		netlink message to send
	 * @param	out 	received netlink message
	 * @param	out_len	length of the received message
	 */
	status_t (*send)(netlink_socket_t *this, struct nlmsghdr *in,
					 struct nlmsghdr **out, size_t *out_len);

	/**
	 * Send a netlink message and wait for its acknowledge.
	 *
	 * @param	in		netlink message to send
	 */
	status_t (*send_ack)(netlink_socket_t *this, struct nlmsghdr *in);

	/**
	 * Destroy the socket.
	 */
	void (*destroy)(netlink_socket_t *this);
};

/**
 * Create a netlink_socket_t object.
 *
 * @param protocol	protocol type (e.g. NETLINK_XFRM or NETLINK_ROUTE)
 * @param names		optional enum names for Netlink messages
 * @param parallel	support parallel queries on this Netlink socket
 */
netlink_socket_t *netlink_socket_create(int protocol, enum_name_t *names,
										bool parallel);

/**
 * Creates an rtattr and adds it to the given netlink message.
 *
 * @param hdr			netlink message
 * @param rta_type		type of the rtattr
 * @param data			data to add to the rtattr
 * @param buflen		length of the netlink message buffer
 */
void netlink_add_attribute(struct nlmsghdr *hdr, int rta_type, chunk_t data,
						   size_t buflen);

/**
 * Reserve space in a netlink message for given size and type, returning buffer.
 *
 * @param hdr			netlink message
 * @param buflen		size of full netlink buffer
 * @param type			RTA type
 * @param len			length of RTA data
 * @return				buffer to len bytes of attribute data, NULL on error
 */
void* netlink_reserve(struct nlmsghdr *hdr, int buflen, int type, int len);

/**
 * Determine buffer size for received messages (e.g. events).
 *
 * @return				buffer size
 */
u_int netlink_get_buflen();

#endif /* KERNEL_NETLINK_SHARED_H_ */
