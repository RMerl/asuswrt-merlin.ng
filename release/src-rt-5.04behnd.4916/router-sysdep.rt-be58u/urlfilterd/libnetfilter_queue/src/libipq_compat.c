/*
 * libipq - backwards compatibility library for libnetfilter_queue
 *
 * (C) 2005 by Harald Welte <laforge@netfilter.org>
 *
 * Based on original libipq.c, 
 * Author: James Morris <jmorris@intercode.com.au>
 * 07-11-2001 Modified by Fernando Anton to add support for IPv6.
 * Copyright (c) 2000-2001 Netfilter Core Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/libipq.h>

/****************************************************************************
 *
 * Private interface
 *
 ****************************************************************************/

enum {
	IPQ_ERR_NONE = 0,
	IPQ_ERR_IMPL,
	IPQ_ERR_HANDLE,
	IPQ_ERR_SOCKET,
	IPQ_ERR_BIND,
	IPQ_ERR_BUFFER,
	IPQ_ERR_RECV,
	IPQ_ERR_NLEOF,
	IPQ_ERR_ADDRLEN,
	IPQ_ERR_STRUNC,
	IPQ_ERR_RTRUNC,
	IPQ_ERR_NLRECV,
	IPQ_ERR_SEND,
	IPQ_ERR_SUPP,
	IPQ_ERR_RECVBUF,
	IPQ_ERR_TIMEOUT,
        IPQ_ERR_PROTOCOL
};
#define IPQ_MAXERR IPQ_ERR_PROTOCOL

struct ipq_errmap_t {
	int errcode;
	char *message;
} ipq_errmap[] = {
	{ IPQ_ERR_NONE, "Unknown error" },
	{ IPQ_ERR_IMPL, "Implementation error" },
	{ IPQ_ERR_HANDLE, "Unable to create netlink handle" },
	{ IPQ_ERR_SOCKET, "Unable to create netlink socket" },
	{ IPQ_ERR_BIND, "Unable to bind netlink socket" },
	{ IPQ_ERR_BUFFER, "Unable to allocate buffer" },
	{ IPQ_ERR_RECV, "Failed to receive netlink message" },
	{ IPQ_ERR_NLEOF, "Received EOF on netlink socket" },
	{ IPQ_ERR_ADDRLEN, "Invalid peer address length" },
	{ IPQ_ERR_STRUNC, "Sent message truncated" },
	{ IPQ_ERR_RTRUNC, "Received message truncated" },
	{ IPQ_ERR_NLRECV, "Received error from netlink" },
	{ IPQ_ERR_SEND, "Failed to send netlink message" },
	{ IPQ_ERR_SUPP, "Operation not supported" },
	{ IPQ_ERR_RECVBUF, "Receive buffer size invalid" },
	{ IPQ_ERR_TIMEOUT, "Timeout"},
	{ IPQ_ERR_PROTOCOL, "Invalid protocol specified" }
};

static int ipq_errno = IPQ_ERR_NONE;

static char *ipq_strerror(int errcode)
{
	if (errcode < 0 || errcode > IPQ_MAXERR)
		errcode = IPQ_ERR_IMPL;
	return ipq_errmap[errcode].message;
}

/****************************************************************************
 *
 * Public interface
 *
 ****************************************************************************/

/*
 * Create and initialise an ipq handle.
 */
struct ipq_handle *ipq_create_handle(u_int32_t flags, u_int32_t protocol)
{
	int status;
	struct ipq_handle *h;

	h = (struct ipq_handle *)malloc(sizeof(struct ipq_handle));
	if (h == NULL) {
		ipq_errno = IPQ_ERR_HANDLE;
		return NULL;
	}
	
	memset(h, 0, sizeof(struct ipq_handle));

	h->nfqnlh = nfq_open();
	if (!h->nfqnlh) {
		ipq_errno = IPQ_ERR_SOCKET;
		goto err_free;
	}
	
        if (protocol == PF_INET)
		status = nfq_bind_pf(h->nfqnlh, PF_INET);
        else if (protocol == PF_INET6)
		status = nfq_bind_pf(h->nfqnlh, PF_INET6);
        else {
		ipq_errno = IPQ_ERR_PROTOCOL;
		goto err_close;
        }
	h->family = protocol;
	if (status < 0) {
		ipq_errno = IPQ_ERR_BIND;
		goto err_close;
	}

	h->qh = nfq_create_queue(h->nfqnlh, 0, NULL, NULL);
	if (!h->qh) {
		ipq_errno = IPQ_ERR_BIND;
		goto err_close;
	}

	return h;

err_close:
	nfq_close(h->nfqnlh);
err_free:
	free(h);
	return NULL;
}

/*
 * No error condition is checked here at this stage, but it may happen
 * if/when reliable messaging is implemented.
 */
int ipq_destroy_handle(struct ipq_handle *h)
{
	if (h) {
		nfq_close(h->nfqnlh);
		free(h);
	}
	return 0;
}

int ipq_set_mode(const struct ipq_handle *h,
                 u_int8_t mode, size_t range)
{
	return nfq_set_mode(h->qh, mode, range);
}

/*
 * timeout is in microseconds (1 second is 1000000 (1 million) microseconds)
 *
 */
ssize_t ipq_read(const struct ipq_handle *h,
                 unsigned char *buf, size_t len, int timeout)
{
	struct nfattr *tb[NFQA_MAX];
	struct nlmsghdr *nlh = (struct nlmsghdr *)buf;
	struct nfgenmsg *msg = NULL;
	struct nfattr *nfa;

	//return ipq_netlink_recvfrom(h, buf, len, timeout);
	
	/* This really sucks.  We have to copy the whole packet
	 * in order to build a data structure that is compatible to
	 * the old ipq interface... */

	nfa = nfnl_parse_hdr(nfq_nfnlh(h->nfqnlh), nlh, &msg);
	if (!msg || !nfa)
		return 0;

	if (msg->nfgen_family != h->family)
		return 0;
	
	nfnl_parse_attr(tb, NFQA_MAX, nfa, 0xffff);


	return 0;
}

int ipq_message_type(const unsigned char *buf)
{
	return ((struct nlmsghdr*)buf)->nlmsg_type;
}

int ipq_get_msgerr(const unsigned char *buf)
{
	struct nlmsghdr *h = (struct nlmsghdr *)buf;
	struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(h);
	return -err->error;
}

ipq_packet_msg_t *ipq_get_packet(const unsigned char *buf)
{
	return NLMSG_DATA((struct nlmsghdr *)(buf));
}

int ipq_set_verdict(const struct ipq_handle *h,
                    ipq_id_t id,
                    unsigned int verdict,
                    size_t data_len,
                    unsigned char *buf)
{
	return nfq_set_verdict(h->qh, id, verdict, data_len, buf);
}

/* Not implemented yet */
int ipq_ctl(const struct ipq_handle *h, int request, ...)
{
	return 1;
}

char *ipq_errstr(void)
{
	return ipq_strerror(ipq_errno);
}

void ipq_perror(const char *s)
{
	if (s)
		fputs(s, stderr);
	else
		fputs("ERROR", stderr);
	if (ipq_errno)
		fprintf(stderr, ": %s", ipq_errstr());
	if (errno)
		fprintf(stderr, ": %s", strerror(errno));
	fputc('\n', stderr);
}
