/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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

/*
 * Copyright (C) 2016 secunet Security Networks AG
 * Copyright (C) 2016 Thomas Egerer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/xfrm.h>
#include <errno.h>
#include <unistd.h>

#include "kernel_netlink_shared.h"

#include <utils/debug.h>
#include <threading/mutex.h>
#include <threading/condvar.h>
#include <collections/array.h>
#include <collections/hashtable.h>

typedef struct private_netlink_socket_t private_netlink_socket_t;

/**
 * Private variables and functions of netlink_socket_t class.
 */
struct private_netlink_socket_t {

	/**
	 * public part of the netlink_socket_t object.
	 */
	netlink_socket_t public;

	/**
	 * mutex to lock access entries
	 */
	mutex_t *mutex;

	/**
	 * Netlink request entries currently active, uintptr_t seq => entry_t
	 */
	hashtable_t *entries;

	/**
	 * Current sequence number for Netlink requests
	 */
	refcount_t seq;

	/**
	 * netlink socket
	 */
	int socket;

	/**
	 * Netlink protocol
	 */
	int protocol;

	/**
	 * Enum names for Netlink messages
	 */
	enum_name_t *names;

	/**
	 * Timeout for Netlink replies, in ms
	 */
	u_int timeout;

	/**
	 * Number of times to repeat timed out queries
	 */
	u_int retries;

	/**
	 * Buffer size for received Netlink messages
	 */
	u_int buflen;

	/**
	 * Use parallel netlink queries
	 */
	bool parallel;

	/**
	 * Ignore errors potentially resulting from a retransmission
	 */
	bool ignore_retransmit_errors;
};

/**
 * #definable hook to simulate request message loss
 */
#ifdef NETLINK_MSG_LOSS_HOOK
bool NETLINK_MSG_LOSS_HOOK(struct nlmsghdr *msg);
#define msg_loss_hook(msg) NETLINK_MSG_LOSS_HOOK(msg)
#else
#define msg_loss_hook(msg) FALSE
#endif

/**
 * Request entry the answer for a waiting thread is collected in
 */
typedef struct {
	/** Condition variable thread is waiting */
	condvar_t *condvar;
	/** Array of hdrs in a multi-message response, as struct nlmsghdr* */
	array_t *hdrs;
	/** All response messages received? */
	bool complete;
} entry_t;

/**
 * Clean up a thread waiting entry
 */
static void destroy_entry(entry_t *entry)
{
	entry->condvar->destroy(entry->condvar);
	array_destroy_function(entry->hdrs, (void*)free, NULL);
	free(entry);
}

/**
 * Write a Netlink message to socket
 */
static bool write_msg(private_netlink_socket_t *this, struct nlmsghdr *msg)
{
	struct sockaddr_nl addr = {
		.nl_family = AF_NETLINK,
	};
	int len;

	if (msg_loss_hook(msg))
	{
		return TRUE;
	}

	while (TRUE)
	{
		len = sendto(this->socket, msg, msg->nlmsg_len, 0,
					 (struct sockaddr*)&addr, sizeof(addr));
		if (len != msg->nlmsg_len)
		{
			if (errno == EINTR)
			{
				continue;
			}
			DBG1(DBG_KNL, "netlink write error: %s", strerror(errno));
			return FALSE;
		}
		return TRUE;
	}
}

/**
 * Read a single Netlink message from socket, return 0 on error, -1 on timeout
 */
static ssize_t read_msg(private_netlink_socket_t *this,
						char *buf, size_t buflen, bool block)
{
	ssize_t len;

	if (block)
	{
		fd_set set;
		timeval_t tv = {};

		FD_ZERO(&set);
		FD_SET(this->socket, &set);
		timeval_add_ms(&tv, this->timeout);

		if (select(this->socket + 1, &set, NULL, NULL,
				   this->timeout ? &tv : NULL) <= 0)
		{
			return -1;
		}
	}
	len = recv(this->socket, buf, buflen, MSG_TRUNC|(block ? 0 : MSG_DONTWAIT));
	if (len > buflen)
	{
		DBG1(DBG_KNL, "netlink response exceeds buffer size");
		return 0;
	}
	if (len < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
		{
			DBG1(DBG_KNL, "netlink read error: %s", strerror(errno));
		}
		return 0;
	}
	return len;
}

/**
 * Queue received response message
 */
static bool queue(private_netlink_socket_t *this, struct nlmsghdr *buf)
{
	struct nlmsghdr *hdr;
	entry_t *entry;
	uintptr_t seq;

	seq = (uintptr_t)buf->nlmsg_seq;

	this->mutex->lock(this->mutex);
	entry = this->entries->get(this->entries, (void*)seq);
	if (entry)
	{
		hdr = malloc(buf->nlmsg_len);
		memcpy(hdr, buf, buf->nlmsg_len);
		array_insert(entry->hdrs, ARRAY_TAIL, hdr);
		if (hdr->nlmsg_type == NLMSG_DONE || !(hdr->nlmsg_flags & NLM_F_MULTI))
		{
			entry->complete = TRUE;
			entry->condvar->signal(entry->condvar);
		}
	}
	else
	{
		DBG1(DBG_KNL, "received unknown netlink seq %u, ignored", seq);
	}
	this->mutex->unlock(this->mutex);

	return entry != NULL;
}

/**
 * Read and queue response message, optionally blocking, returns TRUE on timeout
 */
static bool read_and_queue(private_netlink_socket_t *this, bool block)
{
	struct nlmsghdr *hdr;
	char buf[this->buflen];
	ssize_t len, read_len;
	bool wipe = FALSE;

	len = read_len = read_msg(this, buf, sizeof(buf), block);
	if (len == -1)
	{
		return TRUE;
	}
	if (len)
	{
		hdr = (struct nlmsghdr*)buf;
		while (NLMSG_OK(hdr, len))
		{
			if (this->protocol == NETLINK_XFRM &&
				hdr->nlmsg_type == XFRM_MSG_NEWSA)
			{	/* wipe potential IPsec SA keys */
				wipe = TRUE;
			}
			if (!queue(this, hdr))
			{
				break;
			}
			hdr = NLMSG_NEXT(hdr, len);
		}
	}
	if (wipe)
	{
		memwipe(buf, read_len);
	}
	return FALSE;
}

CALLBACK(watch, bool,
	private_netlink_socket_t *this, int fd, watcher_event_t event)
{
	if (event == WATCHER_READ)
	{
		read_and_queue(this, FALSE);
	}
	return TRUE;
}

/**
 * Send a netlink request, try once
 */
static status_t send_once(private_netlink_socket_t *this, struct nlmsghdr *in,
						  uintptr_t seq, struct nlmsghdr **out, size_t *out_len)
{
	struct nlmsghdr *hdr;
	entry_t *entry;
	u_char *ptr;
	int i;

	in->nlmsg_seq = seq;
	in->nlmsg_pid = getpid();

	if (this->names)
	{
		DBG3(DBG_KNL, "sending %N %u: %b", this->names, in->nlmsg_type,
			 (u_int)seq, in, in->nlmsg_len);
	}

	this->mutex->lock(this->mutex);
	if (!write_msg(this, in))
	{
		this->mutex->unlock(this->mutex);
		return FAILED;
	}

	INIT(entry,
		.condvar = condvar_create(CONDVAR_TYPE_DEFAULT),
		.hdrs = array_create(0, 0),
	);
	this->entries->put(this->entries, (void*)seq, entry);

	while (!entry->complete)
	{
		if (this->parallel &&
			lib->watcher->get_state(lib->watcher) != WATCHER_STOPPED &&
			lib->processor->get_total_threads(lib->processor))
		{
			if (this->timeout)
			{
				if (entry->condvar->timed_wait(entry->condvar, this->mutex,
											   this->timeout))
				{
					break;
				}
			}
			else
			{
				entry->condvar->wait(entry->condvar, this->mutex);
			}
		}
		else
		{	/* During (de-)initialization, no watcher thread is active.
			 * collect responses ourselves. */
			if (read_and_queue(this, TRUE))
			{
				break;
			}
		}
	}
	this->entries->remove(this->entries, (void*)seq);

	this->mutex->unlock(this->mutex);

	if (!entry->complete)
	{	/* timeout */
		destroy_entry(entry);
		return OUT_OF_RES;
	}

	for (i = 0, *out_len = 0; i < array_count(entry->hdrs); i++)
	{
		array_get(entry->hdrs, i, &hdr);
		*out_len += NLMSG_ALIGN(hdr->nlmsg_len);
	}
	ptr = malloc(*out_len);
	*out = (struct nlmsghdr*)ptr;

	while (array_remove(entry->hdrs, ARRAY_HEAD, &hdr))
	{
		if (this->names)
		{
			DBG3(DBG_KNL, "received %N %u: %b", this->names, hdr->nlmsg_type,
				 hdr->nlmsg_seq, hdr, hdr->nlmsg_len);
		}
		memcpy(ptr, hdr, hdr->nlmsg_len);
		ptr += NLMSG_ALIGN(hdr->nlmsg_len);
		free(hdr);
	}
	destroy_entry(entry);
	return SUCCESS;
}

/**
 * Ignore errors for message types that might have completed previously
 */
static void ignore_retransmit_error(private_netlink_socket_t *this,
									struct nlmsgerr *err, int type)
{
	switch (err->error)
	{
		case -EEXIST:
			switch (this->protocol)
			{
				case NETLINK_XFRM:
					switch (type)
					{
						case XFRM_MSG_NEWPOLICY:
						case XFRM_MSG_NEWSA:
							err->error = 0;
							break;
					}
					break;
				case NETLINK_ROUTE:
					switch (type)
					{
						case RTM_NEWADDR:
						case RTM_NEWLINK:
						case RTM_NEWNEIGH:
						case RTM_NEWROUTE:
						case RTM_NEWRULE:
							err->error = 0;
							break;
					}
					break;
			}
			break;
		case -ENOENT:
			switch (this->protocol)
			{
				case NETLINK_XFRM:
					switch (type)
					{
						case XFRM_MSG_DELPOLICY:
						case XFRM_MSG_DELSA:
							err->error = 0;
							break;
					}
					break;
				case NETLINK_ROUTE:
					switch (type)
					{
						case RTM_DELADDR:
						case RTM_DELLINK:
						case RTM_DELNEIGH:
						case RTM_DELROUTE:
						case RTM_DELRULE:
							err->error = 0;
							break;
					}
					break;
			}
			break;
	}
}

METHOD(netlink_socket_t, netlink_send, status_t,
	private_netlink_socket_t *this, struct nlmsghdr *in, struct nlmsghdr **out,
	size_t *out_len)
{
	uintptr_t seq;
	u_int try;

	seq = ref_get(&this->seq);

	for (try = 0; try <= this->retries; ++try)
	{
		struct nlmsghdr *hdr;
		status_t status;
		size_t len;

		if (try > 0)
		{
			DBG1(DBG_KNL, "retransmitting Netlink request (%u/%u)",
				 try, this->retries);
		}
		status = send_once(this, in, seq, &hdr, &len);
		switch (status)
		{
			case SUCCESS:
				break;
			case OUT_OF_RES:
				continue;
			default:
				return status;
		}
		if (hdr->nlmsg_type == NLMSG_ERROR)
		{
			struct nlmsgerr* err;

			err = NLMSG_DATA(hdr);
			if (err->error == -EBUSY)
			{
				free(hdr);
				try--;
				continue;
			}
			if (this->ignore_retransmit_errors && try > 0)
			{
				ignore_retransmit_error(this, err, in->nlmsg_type);
			}
		}
		*out = hdr;
		*out_len = len;
		return SUCCESS;
	}
	DBG1(DBG_KNL, "Netlink request timed out after %u retransmits",
		 this->retries);
	return OUT_OF_RES;
}

METHOD(netlink_socket_t, netlink_send_ack, status_t,
	private_netlink_socket_t *this, struct nlmsghdr *in)
{
	struct nlmsghdr *out, *hdr;
	size_t len;

	if (netlink_send(this, in, &out, &len) != SUCCESS)
	{
		return FAILED;
	}
	hdr = out;
	while (NLMSG_OK(hdr, len))
	{
		switch (hdr->nlmsg_type)
		{
			case NLMSG_ERROR:
			{
				struct nlmsgerr* err = NLMSG_DATA(hdr);

				if (err->error)
				{
					if (-err->error == EEXIST)
					{	/* do not report existing routes */
						free(out);
						return ALREADY_DONE;
					}
					if (-err->error == ESRCH)
					{	/* do not report missing entries */
						free(out);
						return NOT_FOUND;
					}
					DBG1(DBG_KNL, "received netlink error: %s (%d)",
						 strerror(-err->error), -err->error);
					free(out);
					return FAILED;
				}
				free(out);
				return SUCCESS;
			}
			default:
				hdr = NLMSG_NEXT(hdr, len);
				continue;
			case NLMSG_DONE:
				break;
		}
		break;
	}
	DBG1(DBG_KNL, "netlink request not acknowledged");
	free(out);
	return FAILED;
}

METHOD(netlink_socket_t, destroy, void,
	private_netlink_socket_t *this)
{
	if (this->socket != -1)
	{
		if (this->parallel)
		{
			lib->watcher->remove(lib->watcher, this->socket);
		}
		close(this->socket);
	}
	this->entries->destroy(this->entries);
	this->mutex->destroy(this->mutex);
	free(this);
}

/*
 * Described in header
 */
u_int netlink_get_buflen()
{
	u_int buflen;

	buflen = lib->settings->get_int(lib->settings,
								"%s.plugins.kernel-netlink.buflen", 0, lib->ns);
	if (!buflen)
	{
		long pagesize = sysconf(_SC_PAGESIZE);

		if (pagesize == -1)
		{
			pagesize = 4096;
		}
		/* base this on NLMSG_GOODSIZE */
		buflen = min(pagesize, 8192);
	}
	return buflen;
}

/*
 * Described in header
 */
netlink_socket_t *netlink_socket_create(int protocol, enum_name_t *names,
										bool parallel)
{
	private_netlink_socket_t *this;
	struct sockaddr_nl addr = {
		.nl_family = AF_NETLINK,
	};
	bool force_buf = FALSE;
	int rcvbuf_size = 0;

	INIT(this,
		.public = {
			.send = _netlink_send,
			.send_ack = _netlink_send_ack,
			.destroy = _destroy,
		},
		.seq = 200,
		.mutex = mutex_create(MUTEX_TYPE_RECURSIVE),
		.socket = socket(AF_NETLINK, SOCK_RAW, protocol),
		.entries = hashtable_create(hashtable_hash_ptr, hashtable_equals_ptr, 4),
		.protocol = protocol,
		.names = names,
		.buflen = netlink_get_buflen(),
		.timeout = lib->settings->get_int(lib->settings,
							"%s.plugins.kernel-netlink.timeout", 0, lib->ns),
		.retries = lib->settings->get_int(lib->settings,
							"%s.plugins.kernel-netlink.retries", 0, lib->ns),
		.ignore_retransmit_errors = lib->settings->get_bool(lib->settings,
							"%s.plugins.kernel-netlink.ignore_retransmit_errors",
							FALSE, lib->ns),
		.parallel = parallel,
	);

	if (this->socket == -1)
	{
		DBG1(DBG_KNL, "unable to create netlink socket: %s (%d)",
			 strerror(errno), errno);
		destroy(this);
		return NULL;
	}
	if (bind(this->socket, (struct sockaddr*)&addr, sizeof(addr)))
	{
		DBG1(DBG_KNL, "unable to bind netlink socket: %s (%d)",
			 strerror(errno), errno);
		destroy(this);
		return NULL;
	}
	rcvbuf_size = lib->settings->get_int(lib->settings,
						"%s.plugins.kernel-netlink.receive_buffer_size",
						rcvbuf_size, lib->ns);
	if (rcvbuf_size)
	{
		int optname;

		force_buf = lib->settings->get_bool(lib->settings,
						"%s.plugins.kernel-netlink.force_receive_buffer_size",
						force_buf, lib->ns);
		optname = force_buf ? SO_RCVBUFFORCE : SO_RCVBUF;

		if (setsockopt(this->socket, SOL_SOCKET, optname, &rcvbuf_size,
					   sizeof(rcvbuf_size)) == -1)
		{
			DBG1(DBG_KNL, "failed to %supdate receive buffer size to %d: %s",
					force_buf ? "forcibly " : "", rcvbuf_size, strerror(errno));
		}
	}
	if (this->parallel)
	{
		lib->watcher->add(lib->watcher, this->socket, WATCHER_READ, watch, this);
	}

	return &this->public;
}

/**
 * Described in header.
 */
void netlink_add_attribute(struct nlmsghdr *hdr, int rta_type, chunk_t data,
						  size_t buflen)
{
	struct rtattr *rta;

	if (NLMSG_ALIGN(hdr->nlmsg_len) + RTA_LENGTH(data.len) > buflen)
	{
		DBG1(DBG_KNL, "unable to add attribute, buffer too small");
		return;
	}

	rta = (struct rtattr*)(((char*)hdr) + NLMSG_ALIGN(hdr->nlmsg_len));
	rta->rta_type = rta_type;
	rta->rta_len = RTA_LENGTH(data.len);
	memcpy(RTA_DATA(rta), data.ptr, data.len);
	hdr->nlmsg_len = NLMSG_ALIGN(hdr->nlmsg_len) + rta->rta_len;
}

/**
 * Described in header.
 */
void* netlink_reserve(struct nlmsghdr *hdr, int buflen, int type, int len)
{
	struct rtattr *rta;

	if (NLMSG_ALIGN(hdr->nlmsg_len) + RTA_LENGTH(len) > buflen)
	{
		DBG1(DBG_KNL, "unable to add attribute, buffer too small");
		return NULL;
	}

	rta = ((void*)hdr) + NLMSG_ALIGN(hdr->nlmsg_len);
	rta->rta_type = type;
	rta->rta_len = RTA_LENGTH(len);
	hdr->nlmsg_len = NLMSG_ALIGN(hdr->nlmsg_len) + rta->rta_len;

	return RTA_DATA(rta);
}
