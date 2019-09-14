/*
 * lib/nl.c		Core Netlink Interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @defgroup core Core Library (libnl)
 *
 * Socket handling, connection management, sending and receiving of data,
 * message construction and parsing, object caching system, ...
 *
 * This is the API reference of the core library. It is not meant as a guide
 * but as a reference. Please refer to the core library guide for detailed
 * documentation on the library architecture and examples:
 *
 * * @ref_asciidoc{core,_,Netlink Core Library Development Guide}
 *
 *
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/socket.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/handlers.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

/**
 * @defgroup core_types Data Types
 *
 * Core library data types
 * @{
 * @}
 *
 * @defgroup send_recv Send & Receive Data
 *
 * Connection management, sending & receiving of data
 *
 * Related sections in the development guide:
 * - @core_doc{core_send_recv, Sending & Receiving}
 * - @core_doc{core_sockets, Sockets}
 *
 * @{
 *
 * Header
 * ------
 * ~~~~{.c}
 * #include <netlink/netlink.h>
 * ~~~~
 */

/**
 * @name Connection Management
 * @{
 */

/**
 * Create file descriptor and bind socket.
 * @arg sk		Netlink socket (required)
 * @arg protocol	Netlink protocol to use (required)
 *
 * Creates a new Netlink socket using `socket()` and binds the socket to the
 * protocol and local port specified in the `sk` socket object. Fails if
 * the socket is already connected.
 *
 * @note If available, the `close-on-exec` (`SOCK_CLOEXEC`) feature is enabled
 *       automatically on the new file descriptor. This causes the socket to
 *       be closed automatically if any of the `exec` family functions succeed.
 *       This is essential for multi threaded programs.
 *
 * @note The local port (`nl_socket_get_local_port()`) is unspecified after
 *       creating a new socket. It only gets determined when accessing the
 *       port the first time or during `nl_connect()`. When nl_connect()
 *       fails during `bind()` due to `ADDRINUSE`, it will retry with
 *       different ports if the port is unspecified. Unless you want to enforce
 *       the use of a specific local port, don't access the local port (or
 *       reset it to `unspecified` by calling `nl_socket_set_local_port(sk, 0)`).
 *       This capability is indicated by
 *       `%NL_CAPABILITY_NL_CONNECT_RETRY_GENERATE_PORT_ON_ADDRINUSE`.
 *
 * @see nl_socket_alloc()
 * @see nl_close()
 *
 * @return 0 on success or a negative error code.
 *
 * @retval -NLE_BAD_SOCK Socket is already connected
 */
int nl_connect(struct nl_sock *sk, int protocol)
{
	int err, flags = 0;
	int errsv;
	socklen_t addrlen;

#ifdef SOCK_CLOEXEC
	flags |= SOCK_CLOEXEC;
#endif

        if (sk->s_fd != -1)
                return -NLE_BAD_SOCK;

	sk->s_fd = socket(AF_NETLINK, SOCK_RAW | flags, protocol);
	if (sk->s_fd < 0) {
		errsv = errno;
		NL_DBG(4, "nl_connect(%p): socket() failed with %d\n", sk, errsv);
		err = -nl_syserr2nlerr(errsv);
		goto errout;
	}

	if (!(sk->s_flags & NL_SOCK_BUFSIZE_SET)) {
		err = nl_socket_set_buffer_size(sk, 0, 0);
		if (err < 0)
			goto errout;
	}

	if (_nl_socket_is_local_port_unspecified (sk)) {
		uint32_t port;
		uint32_t used_ports[32] = { 0 };

		while (1) {
			port = _nl_socket_generate_local_port_no_release(sk);

			if (port == UINT32_MAX) {
				NL_DBG(4, "nl_connect(%p): no more unused local ports.\n", sk);
				_nl_socket_used_ports_release_all(used_ports);
				err = -NLE_EXIST;
				goto errout;
			}
			err = bind(sk->s_fd, (struct sockaddr*) &sk->s_local,
				   sizeof(sk->s_local));
			if (err == 0)
				break;

			errsv = errno;
			if (errsv == EADDRINUSE) {
				NL_DBG(4, "nl_connect(%p): local port %u already in use. Retry.\n", sk, (unsigned) port);
				_nl_socket_used_ports_set(used_ports, port);
			} else {
				NL_DBG(4, "nl_connect(%p): bind() for port %u failed with %d\n", sk, (unsigned) port, errsv);
				_nl_socket_used_ports_release_all(used_ports);
				err = -nl_syserr2nlerr(errsv);
				goto errout;
			}
		}
		_nl_socket_used_ports_release_all(used_ports);
	} else {
		err = bind(sk->s_fd, (struct sockaddr*) &sk->s_local,
			   sizeof(sk->s_local));
		if (err != 0) {
			errsv = errno;
			NL_DBG(4, "nl_connect(%p): bind() failed with %d\n", sk, errsv);
			err = -nl_syserr2nlerr(errsv);
			goto errout;
		}
	}

	addrlen = sizeof(sk->s_local);
	err = getsockname(sk->s_fd, (struct sockaddr *) &sk->s_local,
			  &addrlen);
	if (err < 0) {
		err = -nl_syserr2nlerr(errno);
		goto errout;
	}

	if (addrlen != sizeof(sk->s_local)) {
		err = -NLE_NOADDR;
		goto errout;
	}

	if (sk->s_local.nl_family != AF_NETLINK) {
		err = -NLE_AF_NOSUPPORT;
		goto errout;
	}

	sk->s_proto = protocol;

	return 0;
errout:
        if (sk->s_fd != -1) {
    		close(sk->s_fd);
    		sk->s_fd = -1;
        }

	return err;
}

/**
 * Close Netlink socket
 * @arg sk		Netlink socket (required)
 *
 * Closes the Netlink socket using `close()`.
 *
 * @note The socket is closed automatically if a `struct nl_sock` object is
 *       freed using `nl_socket_free()`.
 *
 * @see nl_connect()
 */
void nl_close(struct nl_sock *sk)
{
	if (sk->s_fd >= 0) {
		close(sk->s_fd);
		sk->s_fd = -1;
	}

	sk->s_proto = 0;
}

/** @} */

/**
 * @name Send
 * @{
 */

/**
 * Transmit raw data over Netlink socket.
 * @arg sk		Netlink socket (required)
 * @arg buf		Buffer carrying data to send (required)
 * @arg size		Size of buffer (required)
 *
 * Transmits "raw" data over the specified Netlink socket. Unlike the other
 * transmit functions it does not modify the data in any way. It directly
 * passes the buffer \c buf of \c size to sendto().
 *
 * The message is addressed to the peer as specified in the socket by either
 * the nl_socket_set_peer_port() or nl_socket_set_peer_groups() function.
 *
 * @note Because there is no indication on the message boundaries of the data
 *       being sent, the \c NL_CB_MSG_OUT callback handler will not be invoked
 *       for data that is being sent using this function.
 *
 * @see nl_socket_set_peer_port()
 * @see nl_socket_set_peer_groups()
 * @see nl_sendmsg()
 *
 * @return Number of bytes sent or a negative error code.
 */
int nl_sendto(struct nl_sock *sk, void *buf, size_t size)
{
	int ret;

	if (!buf)
		return -NLE_INVAL;

	if (sk->s_fd < 0)
		return -NLE_BAD_SOCK;

	ret = sendto(sk->s_fd, buf, size, 0, (struct sockaddr *)
		     &sk->s_peer, sizeof(sk->s_peer));
	if (ret < 0)
		return -nl_syserr2nlerr(errno);

	return ret;
}

/**
 * Transmit Netlink message using sendmsg()
 * @arg sk		Netlink socket (required)
 * @arg msg		Netlink message to be sent (required)
 * @arg hdr		sendmsg() message header (required)
 *
 * Transmits the message specified in \c hdr over the Netlink socket using the
 * sendmsg() system call.
 *
 * @attention
 * The `msg` argument will *not* be used to derive the message payload that
 * is being sent out. The `msg` argument is *only* passed on to the
 * `NL_CB_MSG_OUT` callback. The caller is responsible to initialize the
 * `hdr` struct properly and have it point to the message payload and
 * socket address.
 *
 * @note
 * This function uses `nlmsg_set_src()` to modify the `msg` argument prior to
 * invoking the `NL_CB_MSG_OUT` callback to provide the local port number.
 *
 * @callback This function triggers the `NL_CB_MSG_OUT` callback.
 *
 * @attention
 * Think twice before using this function. It provides a low level access to
 * the Netlink socket. Among other limitations, it does not add credentials
 * even if enabled or respect the destination address specified in the `msg`
 * object.
 *
 * @see nl_socket_set_local_port()
 * @see nl_send_auto()
 * @see nl_send_iovec()
 *
 * @return Number of bytes sent on success or a negative error code.
 *
 * @lowlevel
 */
int nl_sendmsg(struct nl_sock *sk, struct nl_msg *msg, struct msghdr *hdr)
{
	struct nl_cb *cb;
	int ret;

	if (sk->s_fd < 0)
		return -NLE_BAD_SOCK;

	nlmsg_set_src(msg, &sk->s_local);

	cb = sk->s_cb;
	if (cb->cb_set[NL_CB_MSG_OUT])
		if ((ret = nl_cb_call(cb, NL_CB_MSG_OUT, msg)) != NL_OK)
			return ret;

	ret = sendmsg(sk->s_fd, hdr, 0);
	if (ret < 0)
		return -nl_syserr2nlerr(errno);

	NL_DBG(4, "sent %d bytes\n", ret);
	return ret;
}


/**
 * Transmit Netlink message (taking IO vector)
 * @arg sk		Netlink socket (required)
 * @arg msg		Netlink message to be sent (required)
 * @arg iov		IO vector to be sent (required)
 * @arg iovlen		Number of struct iovec to be sent (required)
 *
 * This function is identical to nl_send() except that instead of taking a
 * `struct nl_msg` object it takes an IO vector. Please see the description
 * of `nl_send()`.
 *
 * @callback This function triggers the `NL_CB_MSG_OUT` callback.
 *
 * @see nl_send()
 *
 * @return Number of bytes sent on success or a negative error code.
 *
 * @lowlevel
 */
int nl_send_iovec(struct nl_sock *sk, struct nl_msg *msg, struct iovec *iov, unsigned iovlen)
{
	struct sockaddr_nl *dst;
	struct ucred *creds;
	struct msghdr hdr = {
		.msg_name = (void *) &sk->s_peer,
		.msg_namelen = sizeof(struct sockaddr_nl),
		.msg_iov = iov,
		.msg_iovlen = iovlen,
	};

	/* Overwrite destination if specified in the message itself, defaults
	 * to the peer address of the socket.
	 */
	dst = nlmsg_get_dst(msg);
	if (dst->nl_family == AF_NETLINK)
		hdr.msg_name = dst;

	/* Add credentials if present. */
	creds = nlmsg_get_creds(msg);
	if (creds != NULL) {
		char buf[CMSG_SPACE(sizeof(struct ucred))];
		struct cmsghdr *cmsg;

		hdr.msg_control = buf;
		hdr.msg_controllen = sizeof(buf);

		cmsg = CMSG_FIRSTHDR(&hdr);
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_CREDENTIALS;
		cmsg->cmsg_len = CMSG_LEN(sizeof(struct ucred));
		memcpy(CMSG_DATA(cmsg), creds, sizeof(struct ucred));
	}

	return nl_sendmsg(sk, msg, &hdr);
}

/**
 * Transmit Netlink message
 * @arg sk		Netlink socket (required)
 * @arg msg		Netlink message (required)
 *
 * Transmits the Netlink message `msg` over the Netlink socket using the
 * `sendmsg()` system call. This function is based on `nl_send_iovec()` but
 * takes care of initializing a `struct iovec` based on the `msg` object.
 *
 * The message is addressed to the peer as specified in the socket by either
 * the nl_socket_set_peer_port() or nl_socket_set_peer_groups() function.
 * The peer address can be overwritten by specifying an address in the `msg`
 * object using nlmsg_set_dst().
 *
 * If present in the `msg`, credentials set by the nlmsg_set_creds() function
 * are added to the control buffer of the message.
 *
 * @par Overwriting Capability:
 * Calls to this function can be overwritten by providing an alternative using
 * the nl_cb_overwrite_send() function.
 *
 * @callback This function triggers the `NL_CB_MSG_OUT` callback.
 *
 * @attention
 * Unlike `nl_send_auto()`, this function does *not* finalize the message in
 * terms of automatically adding needed flags or filling out port numbers.
 *
 * @see nl_send_auto()
 * @see nl_send_iovec()
 * @see nl_socket_set_peer_port()
 * @see nl_socket_set_peer_groups()
 * @see nlmsg_set_dst()
 * @see nlmsg_set_creds()
 * @see nl_cb_overwrite_send()
 *
 * @return Number of bytes sent on success or a negative error code.
*/
int nl_send(struct nl_sock *sk, struct nl_msg *msg)
{
	struct nl_cb *cb = sk->s_cb;

	if (cb->cb_send_ow)
		return cb->cb_send_ow(sk, msg);
	else {
		struct iovec iov = {
			.iov_base = (void *) nlmsg_hdr(msg),
			.iov_len = nlmsg_hdr(msg)->nlmsg_len,
		};

		return nl_send_iovec(sk, msg, &iov, 1);
	}
}

/**
 * Finalize Netlink message
 * @arg sk		Netlink socket (required)
 * @arg msg		Netlink message (required)
 *
 * This function finalizes a Netlink message by completing the message with
 * desirable flags and values depending on the socket configuration.
 *
 *  - If not yet filled out, the source address of the message (`nlmsg_pid`)
 *    will be set to the local port number of the socket.
 *  - If not yet specified, the next available sequence number is assigned
 *    to the message (`nlmsg_seq`).
 *  - If not yet specified, the protocol field of the message will be set to
 *    the protocol field of the socket.
 *  - The `NLM_F_REQUEST` Netlink message flag will be set.
 *  - The `NLM_F_ACK` flag will be set if Auto-ACK mode is enabled on the
 *    socket.
 */
void nl_complete_msg(struct nl_sock *sk, struct nl_msg *msg)
{
	struct nlmsghdr *nlh;

	nlh = nlmsg_hdr(msg);
	if (nlh->nlmsg_pid == NL_AUTO_PORT)
		nlh->nlmsg_pid = nl_socket_get_local_port(sk);

	if (nlh->nlmsg_seq == NL_AUTO_SEQ)
		nlh->nlmsg_seq = sk->s_seq_next++;

	if (msg->nm_protocol == -1)
		msg->nm_protocol = sk->s_proto;

	nlh->nlmsg_flags |= NLM_F_REQUEST;

	if (!(sk->s_flags & NL_NO_AUTO_ACK))
		nlh->nlmsg_flags |= NLM_F_ACK;
}

/**
 * Finalize and transmit Netlink message
 * @arg sk		Netlink socket (required)
 * @arg msg		Netlink message (required)
 *
 * Finalizes the message by passing it to `nl_complete_msg()` and transmits it
 * by passing it to `nl_send()`.
 *
 * @callback This function triggers the `NL_CB_MSG_OUT` callback.
 *
 * @see nl_complete_msg()
 * @see nl_send()
 *
 * @return Number of bytes sent or a negative error code.
 */
int nl_send_auto(struct nl_sock *sk, struct nl_msg *msg)
{
	nl_complete_msg(sk, msg);

	return nl_send(sk, msg);
}

/**
 * Finalize and transmit Netlink message and wait for ACK or error message
 * @arg sk		Netlink socket (required)
 * @arg msg		Netlink message (required)
 *
 * Passes the `msg` to `nl_send_auto()` to finalize and transmit it. Frees the
 * message and waits (sleeps) for the ACK or error message to be received.
 *
 * @attention
 * Disabling Auto-ACK (nl_socket_disable_auto_ack()) will cause this function
 * to return immediately after transmitting the message. However, the peer may
 * still be returning an error message in response to the request. It is the
 * responsibility of the caller to handle such messages.
 *
 * @callback This function triggers the `NL_CB_MSG_OUT` callback.
 *
 * @attention
 * This function frees the `msg` object after transmitting it by calling
 * `nlmsg_free()`.
 *
 * @see nl_send_auto().
 * @see nl_wait_for_ack()
 *
 * @return 0 on success or a negative error code.
 */
int nl_send_sync(struct nl_sock *sk, struct nl_msg *msg)
{
	int err;

	err = nl_send_auto(sk, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	return wait_for_ack(sk);
}

/**
 * Construct and transmit a Netlink message
 * @arg sk		Netlink socket (required)
 * @arg type		Netlink message type (required)
 * @arg flags		Netlink message flags (optional)
 * @arg buf		Data buffer (optional)
 * @arg size		Size of data buffer (optional)
 *
 * Allocates a new Netlink message based on `type` and `flags`. If `buf`
 * points to payload of length `size` that payload will be appended to the
 * message.
 *
 * Sends out the message using `nl_send_auto()` and frees the message
 * afterwards.
 *
 * @see nl_send_auto()
 *
 * @return Number of characters sent on success or a negative error code.
 * @retval -NLE_NOMEM Unable to allocate Netlink message
 */
int nl_send_simple(struct nl_sock *sk, int type, int flags, void *buf,
		   size_t size)
{
	int err;
	struct nl_msg *msg;

	msg = nlmsg_alloc_simple(type, flags);
	if (!msg)
		return -NLE_NOMEM;

	if (buf && size) {
		err = nlmsg_append(msg, buf, size, NLMSG_ALIGNTO);
		if (err < 0)
			goto errout;
	}

	err = nl_send_auto(sk, msg);
errout:
	nlmsg_free(msg);

	return err;
}

/** @} */

/**
 * @name Receive
 * @{
 */

/**
 * Receive data from netlink socket
 * @arg sk		Netlink socket (required)
 * @arg nla		Netlink socket structure to hold address of peer (required)
 * @arg buf		Destination pointer for message content (required)
 * @arg creds		Destination pointer for credentials (optional)
 *
 * Receives data from a connected netlink socket using recvmsg() and returns
 * the number of bytes read. The read data is stored in a newly allocated
 * buffer that is assigned to \c *buf. The peer's netlink address will be
 * stored in \c *nla.
 *
 * This function blocks until data is available to be read unless the socket
 * has been put into non-blocking mode using nl_socket_set_nonblocking() in
 * which case this function will return immediately with a return value of 0.
 *
 * The buffer size used when reading from the netlink socket and thus limiting
 * the maximum size of a netlink message that can be read defaults to the size
 * of a memory page (getpagesize()). The buffer size can be modified on a per
 * socket level using the function nl_socket_set_msg_buf_size().
 *
 * If message peeking is enabled using nl_socket_enable_msg_peek() the size of
 * the message to be read will be determined using the MSG_PEEK flag prior to
 * performing the actual read. This leads to an additional recvmsg() call for
 * every read operation which has performance implications and is not
 * recommended for high throughput protocols.
 *
 * An eventual interruption of the recvmsg() system call is automatically
 * handled by retrying the operation.
 *
 * If receiving of credentials has been enabled using the function
 * nl_socket_set_passcred(), this function will allocate a new struct ucred
 * filled with the received credentials and assign it to \c *creds. The caller
 * is responsible for freeing the buffer.
 *
 * @note The caller is responsible to free the returned data buffer and if
 *       enabled, the credentials buffer.
 *
 * @see nl_socket_set_nonblocking()
 * @see nl_socket_set_msg_buf_size()
 * @see nl_socket_enable_msg_peek()
 * @see nl_socket_set_passcred()
 *
 * @return Number of bytes read, 0 on EOF, 0 on no data event (non-blocking
 *         mode), or a negative error code.
 */
int nl_recv(struct nl_sock *sk, struct sockaddr_nl *nla,
	    unsigned char **buf, struct ucred **creds)
{
	ssize_t n;
	int flags = 0;
	static int page_size = 0;
	struct iovec iov;
	struct msghdr msg = {
		.msg_name = (void *) nla,
		.msg_namelen = sizeof(struct sockaddr_nl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};
	struct ucred* tmpcreds = NULL;
	int retval = 0;

	if (!buf || !nla)
		return -NLE_INVAL;

	if (sk->s_flags & NL_MSG_PEEK)
		flags |= MSG_PEEK | MSG_TRUNC;

	if (page_size == 0)
		page_size = getpagesize() * 4;

	iov.iov_len = sk->s_bufsize ? : page_size;
	iov.iov_base = malloc(iov.iov_len);

	if (!iov.iov_base) {
		retval = -NLE_NOMEM;
		goto abort;
	}

	if (creds && (sk->s_flags & NL_SOCK_PASSCRED)) {
		msg.msg_controllen = CMSG_SPACE(sizeof(struct ucred));
		msg.msg_control = malloc(msg.msg_controllen);
		if (!msg.msg_control) {
			retval = -NLE_NOMEM;
			goto abort;
		}
	}
retry:

	n = recvmsg(sk->s_fd, &msg, flags);
	if (!n) {
		retval = 0;
		goto abort;
	}
	if (n < 0) {
		if (errno == EINTR) {
			NL_DBG(3, "recvmsg() returned EINTR, retrying\n");
			goto retry;
		}
		retval = -nl_syserr2nlerr(errno);
		goto abort;
	}

	if (msg.msg_flags & MSG_CTRUNC) {
		void *tmp;
		msg.msg_controllen *= 2;
		tmp = realloc(msg.msg_control, msg.msg_controllen);
		if (!tmp) {
			retval = -NLE_NOMEM;
			goto abort;
		}
		msg.msg_control = tmp;
		goto retry;
	}

	if (iov.iov_len < n || (msg.msg_flags & MSG_TRUNC)) {
		void *tmp;
		/* Provided buffer is not long enough, enlarge it
		 * to size of n (which should be total length of the message)
		 * and try again. */
		iov.iov_len = n;
		tmp = realloc(iov.iov_base, iov.iov_len);
		if (!tmp) {
			retval = -NLE_NOMEM;
			goto abort;
		}
		iov.iov_base = tmp;
		flags = 0;
		goto retry;
	}

        if (flags != 0) {
		/* Buffer is big enough, do the actual reading */
		flags = 0;
		goto retry;
	}

	if (msg.msg_namelen != sizeof(struct sockaddr_nl)) {
		retval =  -NLE_NOADDR;
		goto abort;
	}

	if (creds && (sk->s_flags & NL_SOCK_PASSCRED)) {
		struct cmsghdr *cmsg;

		for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
			if (cmsg->cmsg_level != SOL_SOCKET)
				continue;
			if (cmsg->cmsg_type != SCM_CREDENTIALS)
				continue;
			tmpcreds = malloc(sizeof(*tmpcreds));
			if (!tmpcreds) {
				retval = -NLE_NOMEM;
				goto abort;
			}
			memcpy(tmpcreds, CMSG_DATA(cmsg), sizeof(*tmpcreds));
			break;
		}
	}

	retval = n;
abort:
	free(msg.msg_control);

	if (retval <= 0) {
		free(iov.iov_base);
		iov.iov_base = NULL;
		free(tmpcreds);
		tmpcreds = NULL;
	} else
		*buf = iov.iov_base;

	if (creds)
		*creds = tmpcreds;

	return retval;
}

/** @cond SKIP */
#define NL_CB_CALL(cb, type, msg) \
do { \
	err = nl_cb_call(cb, type, msg); \
	switch (err) { \
	case NL_OK: \
		err = 0; \
		break; \
	case NL_SKIP: \
		goto skip; \
	case NL_STOP: \
		goto stop; \
	default: \
		goto out; \
	} \
} while (0)
/** @endcond */

static int recvmsgs(struct nl_sock *sk, struct nl_cb *cb)
{
	int n, err = 0, multipart = 0, interrupted = 0, nrecv = 0;
	unsigned char *buf = NULL;
	struct nlmsghdr *hdr;

	/*
	nla is passed on to not only to nl_recv() but may also be passed
	to a function pointer provided by the caller which may or may not
	initialize the variable. Thomas Graf.
	*/
	struct sockaddr_nl nla = {0};
	struct nl_msg *msg = NULL;
	struct ucred *creds = NULL;

continue_reading:
	NL_DBG(3, "Attempting to read from %p\n", sk);
	if (cb->cb_recv_ow)
		n = cb->cb_recv_ow(sk, &nla, &buf, &creds);
	else
		n = nl_recv(sk, &nla, &buf, &creds);

	if (n <= 0)
		return n;

	NL_DBG(3, "recvmsgs(%p): Read %d bytes\n", sk, n);

	hdr = (struct nlmsghdr *) buf;
	while (nlmsg_ok(hdr, n)) {
		NL_DBG(3, "recvmsgs(%p): Processing valid message...\n", sk);

		nlmsg_free(msg);
		msg = nlmsg_convert(hdr);
		if (!msg) {
			err = -NLE_NOMEM;
			goto out;
		}

		nlmsg_set_proto(msg, sk->s_proto);
		nlmsg_set_src(msg, &nla);
		if (creds)
			nlmsg_set_creds(msg, creds);

		nrecv++;

		/* Raw callback is the first, it gives the most control
		 * to the user and he can do his very own parsing. */
		if (cb->cb_set[NL_CB_MSG_IN])
			NL_CB_CALL(cb, NL_CB_MSG_IN, msg);

		/* Sequence number checking. The check may be done by
		 * the user, otherwise a very simple check is applied
		 * enforcing strict ordering */
		if (cb->cb_set[NL_CB_SEQ_CHECK]) {
			NL_CB_CALL(cb, NL_CB_SEQ_CHECK, msg);

		/* Only do sequence checking if auto-ack mode is enabled */
		} else if (!(sk->s_flags & NL_NO_AUTO_ACK)) {
			if (hdr->nlmsg_seq != sk->s_seq_expect) {
				if (cb->cb_set[NL_CB_INVALID])
					NL_CB_CALL(cb, NL_CB_INVALID, msg);
				else {
					err = -NLE_SEQ_MISMATCH;
					goto out;
				}
			}
		}

		if (hdr->nlmsg_type == NLMSG_DONE ||
		    hdr->nlmsg_type == NLMSG_ERROR ||
		    hdr->nlmsg_type == NLMSG_NOOP ||
		    hdr->nlmsg_type == NLMSG_OVERRUN) {
			/* We can't check for !NLM_F_MULTI since some netlink
			 * users in the kernel are broken. */
			sk->s_seq_expect++;
			NL_DBG(3, "recvmsgs(%p): Increased expected " \
			       "sequence number to %d\n",
			       sk, sk->s_seq_expect);
		}

		if (hdr->nlmsg_flags & NLM_F_MULTI)
			multipart = 1;

		if (hdr->nlmsg_flags & NLM_F_DUMP_INTR) {
			if (cb->cb_set[NL_CB_DUMP_INTR])
				NL_CB_CALL(cb, NL_CB_DUMP_INTR, msg);
			else {
				/*
				 * We have to continue reading to clear
				 * all messages until a NLMSG_DONE is
				 * received and report the inconsistency.
				 */
				interrupted = 1;
			}
		}
	
		/* Other side wishes to see an ack for this message */
		if (hdr->nlmsg_flags & NLM_F_ACK) {
			if (cb->cb_set[NL_CB_SEND_ACK])
				NL_CB_CALL(cb, NL_CB_SEND_ACK, msg);
			else {
				/* FIXME: implement */
			}
		}

		/* messages terminates a multipart message, this is
		 * usually the end of a message and therefore we slip
		 * out of the loop by default. the user may overrule
		 * this action by skipping this packet. */
		if (hdr->nlmsg_type == NLMSG_DONE) {
			multipart = 0;
			if (cb->cb_set[NL_CB_FINISH])
				NL_CB_CALL(cb, NL_CB_FINISH, msg);
		}

		/* Message to be ignored, the default action is to
		 * skip this message if no callback is specified. The
		 * user may overrule this action by returning
		 * NL_PROCEED. */
		else if (hdr->nlmsg_type == NLMSG_NOOP) {
			if (cb->cb_set[NL_CB_SKIPPED])
				NL_CB_CALL(cb, NL_CB_SKIPPED, msg);
			else
				goto skip;
		}

		/* Data got lost, report back to user. The default action is to
		 * quit parsing. The user may overrule this action by retuning
		 * NL_SKIP or NL_PROCEED (dangerous) */
		else if (hdr->nlmsg_type == NLMSG_OVERRUN) {
			if (cb->cb_set[NL_CB_OVERRUN])
				NL_CB_CALL(cb, NL_CB_OVERRUN, msg);
			else {
				err = -NLE_MSG_OVERFLOW;
				goto out;
			}
		}

		/* Message carries a nlmsgerr */
		else if (hdr->nlmsg_type == NLMSG_ERROR) {
			struct nlmsgerr *e = nlmsg_data(hdr);

			if (hdr->nlmsg_len < nlmsg_size(sizeof(*e))) {
				/* Truncated error message, the default action
				 * is to stop parsing. The user may overrule
				 * this action by returning NL_SKIP or
				 * NL_PROCEED (dangerous) */
				if (cb->cb_set[NL_CB_INVALID])
					NL_CB_CALL(cb, NL_CB_INVALID, msg);
				else {
					err = -NLE_MSG_TRUNC;
					goto out;
				}
			} else if (e->error) {
				/* Error message reported back from kernel. */
				if (cb->cb_err) {
					err = cb->cb_err(&nla, e,
							   cb->cb_err_arg);
					if (err < 0)
						goto out;
					else if (err == NL_SKIP)
						goto skip;
					else if (err == NL_STOP) {
						err = -nl_syserr2nlerr(e->error);
						goto out;
					}
				} else {
					err = -nl_syserr2nlerr(e->error);
					goto out;
				}
			} else if (cb->cb_set[NL_CB_ACK])
				NL_CB_CALL(cb, NL_CB_ACK, msg);
		} else {
			/* Valid message (not checking for MULTIPART bit to
			 * get along with broken kernels. NL_SKIP has no
			 * effect on this.  */
			if (cb->cb_set[NL_CB_VALID])
				NL_CB_CALL(cb, NL_CB_VALID, msg);
		}
skip:
		err = 0;
		hdr = nlmsg_next(hdr, &n);
	}
	
	nlmsg_free(msg);
	free(buf);
	free(creds);
	buf = NULL;
	msg = NULL;
	creds = NULL;

	if (multipart) {
		/* Multipart message not yet complete, continue reading */
		goto continue_reading;
	}
stop:
	err = 0;
out:
	nlmsg_free(msg);
	free(buf);
	free(creds);

	if (interrupted)
		err = -NLE_DUMP_INTR;

	if (!err)
		err = nrecv;

	return err;
}

/**
 * Receive a set of messages from a netlink socket and report parsed messages
 * @arg sk		Netlink socket.
 * @arg cb		set of callbacks to control behaviour.
 *
 * This function is identical to nl_recvmsgs() to the point that it will
 * return the number of parsed messages instead of 0 on success.
 *
 * @see nl_recvmsgs()
 *
 * @return Number of received messages or a negative error code from nl_recv().
 */
int nl_recvmsgs_report(struct nl_sock *sk, struct nl_cb *cb)
{
	if (cb->cb_recvmsgs_ow)
		return cb->cb_recvmsgs_ow(sk, cb);
	else
		return recvmsgs(sk, cb);
}

/**
 * Receive a set of messages from a netlink socket.
 * @arg sk		Netlink socket.
 * @arg cb		set of callbacks to control behaviour.
 *
 * Repeatedly calls nl_recv() or the respective replacement if provided
 * by the application (see nl_cb_overwrite_recv()) and parses the
 * received data as netlink messages. Stops reading if one of the
 * callbacks returns NL_STOP or nl_recv returns either 0 or a negative error code.
 *
 * A non-blocking sockets causes the function to return immediately if
 * no data is available.
 *
 * @see nl_recvmsgs_report()
 *
 * @return 0 on success or a negative error code from nl_recv().
 */
int nl_recvmsgs(struct nl_sock *sk, struct nl_cb *cb)
{
	int err;

	if ((err = nl_recvmsgs_report(sk, cb)) > 0)
		err = 0;

	return err;
}

/**
 * Receive a set of message from a netlink socket using handlers in nl_sock.
 * @arg sk		Netlink socket.
 *
 * Calls nl_recvmsgs() with the handlers configured in the netlink socket.
 */
int nl_recvmsgs_default(struct nl_sock *sk)
{
	return nl_recvmsgs(sk, sk->s_cb);

}

static int ack_wait_handler(struct nl_msg *msg, void *arg)
{
	return NL_STOP;
}

/**
 * Wait for ACK.
 * @arg sk		Netlink socket.
 * @pre The netlink socket must be in blocking state.
 *
 * Waits until an ACK is received for the latest not yet acknowledged
 * netlink message.
 */
int nl_wait_for_ack(struct nl_sock *sk)
{
	int err;
	struct nl_cb *cb;

	cb = nl_cb_clone(sk->s_cb);
	if (cb == NULL)
		return -NLE_NOMEM;

	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_wait_handler, NULL);
	err = nl_recvmsgs(sk, cb);
	nl_cb_put(cb);

	return err;
}

/** @cond SKIP */
struct pickup_param
{
	int (*parser)(struct nl_cache_ops *, struct sockaddr_nl *,
		      struct nlmsghdr *, struct nl_parser_param *);
	struct nl_object *result;
};

static int __store_answer(struct nl_object *obj, struct nl_parser_param *p)
{
	struct pickup_param *pp = p->pp_arg;
	/*
	 * the parser will put() the object at the end, expecting the cache
	 * to take the reference.
	 */
	nl_object_get(obj);
	pp->result =  obj;

	return 0;
}

static int __pickup_answer(struct nl_msg *msg, void *arg)
{
	struct pickup_param *pp = arg;
	struct nl_parser_param parse_arg = {
		.pp_cb = __store_answer,
		.pp_arg = pp,
	};

	return pp->parser(NULL, &msg->nm_src, msg->nm_nlh, &parse_arg);
}

/** @endcond */

/**
 * Pickup netlink answer, parse is and return object
 * @arg sk		Netlink socket
 * @arg parser		Parser function to parse answer
 * @arg result		Result pointer to return parsed object
 *
 * @return 0 on success or a negative error code.
 */
int nl_pickup(struct nl_sock *sk,
	      int (*parser)(struct nl_cache_ops *, struct sockaddr_nl *,
			    struct nlmsghdr *, struct nl_parser_param *),
	      struct nl_object **result)
{
	struct nl_cb *cb;
	int err;
	struct pickup_param pp = {
		.parser = parser,
	};

	cb = nl_cb_clone(sk->s_cb);
	if (cb == NULL)
		return -NLE_NOMEM;

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, __pickup_answer, &pp);

	err = nl_recvmsgs(sk, cb);
	if (err < 0)
		goto errout;

	*result = pp.result;
errout:
	nl_cb_put(cb);

	return err;
}

/** @} */

/**
 * @name Deprecated
 * @{
 */

/**
 * @deprecated Please use nl_complete_msg()
 */
void nl_auto_complete(struct nl_sock *sk, struct nl_msg *msg)
{
	nl_complete_msg(sk, msg);
}

/**
 * @deprecated Please use nl_send_auto()
 */
int nl_send_auto_complete(struct nl_sock *sk, struct nl_msg *msg)
{
	return nl_send_auto(sk, msg);
}


/** @} */

/** @} */

/** @} */
