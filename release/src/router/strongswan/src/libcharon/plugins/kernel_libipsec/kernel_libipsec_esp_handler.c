/*
 * Copyright (C) 2023 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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

/* for struct in6_pktinfo */
#define _GNU_SOURCE

#include "kernel_libipsec_esp_handler.h"

#ifdef __linux__

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <ipsec.h>
#include <collections/blocking_queue.h>
#include <processing/jobs/callback_job.h>

typedef struct private_kernel_libipsec_esp_handler_t private_kernel_libipsec_esp_handler_t;

/**
 * Private data
 */
struct private_kernel_libipsec_esp_handler_t {

	/**
	 * Public interface
	 */
	kernel_libipsec_esp_handler_t public;

	/**
	 * Queue for outbound ESP packets (esp_packet_t*)
	 */
	blocking_queue_t *queue;

	/**
	 * Socket to send/receive IPv4 ESP packets
	 */
	int skt_v4;

	/**
	 * Socket to send/receive IPv6 ESP packets
	 */
	int skt_v6;
};

METHOD(kernel_libipsec_esp_handler_t, send_, void,
	private_kernel_libipsec_esp_handler_t *this, esp_packet_t *packet)
{
	this->queue->enqueue(this->queue, packet);
}

CALLBACK(send_esp, job_requeue_t,
	private_kernel_libipsec_esp_handler_t *this)
{
	packet_t *packet;
	host_t *source, *destination;
	chunk_t data;
	struct msghdr msg = {};
	struct cmsghdr *cmsg;
	struct iovec iov;
	char ancillary[64] = {};
	ssize_t len;
	int skt;

	packet = (packet_t*)this->queue->dequeue(this->queue);

	data = packet->get_data(packet);
	source = packet->get_source(packet);
	destination = packet->get_destination(packet);
	DBG2(DBG_NET, "sending raw ESP packet: from %H to %H (%zu data bytes)",
		 source, destination, data.len);

	/* the port of the destination address acts as protocol selector for RAW
	 * sockets, for IPv4 the kernel ignores it, for IPv6 it does not and
	 * complains if it isn't zero or doesn't match the one set on the socket */
	destination->set_port(destination, 0);

	msg.msg_name = destination->get_sockaddr(destination);
	msg.msg_namelen = *destination->get_sockaddr_len(destination);
	iov.iov_base = data.ptr;
	iov.iov_len = data.len;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_flags = 0;
	msg.msg_control = ancillary;

	if (source->get_family(source) == AF_INET)
	{
		struct in_pktinfo *pktinfo;
		const struct sockaddr_in *sin;

		msg.msg_controllen = CMSG_SPACE(sizeof(struct in_pktinfo));
		cmsg = CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_level = IPPROTO_IP;
		cmsg->cmsg_type = IP_PKTINFO;
		cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));

		pktinfo = (struct in_pktinfo*)CMSG_DATA(cmsg);
		sin = (struct sockaddr_in*)source->get_sockaddr(source);
		memcpy(&pktinfo->ipi_spec_dst, &sin->sin_addr, sizeof(struct in_addr));
		skt = this->skt_v4;
	}
	else
	{
		struct in6_pktinfo *pktinfo;
		const struct sockaddr_in6 *sin;

		msg.msg_controllen = CMSG_SPACE(sizeof(struct in6_pktinfo));
		cmsg = CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_level = IPPROTO_IPV6;
		cmsg->cmsg_type = IPV6_PKTINFO;
		cmsg->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));

		pktinfo = (struct in6_pktinfo*)CMSG_DATA(cmsg);
		sin = (struct sockaddr_in6*)source->get_sockaddr(source);
		memcpy(&pktinfo->ipi6_addr, &sin->sin6_addr, sizeof(struct in6_addr));
		skt = this->skt_v6;
	}

	len = sendmsg(skt, &msg, 0);
	if (len != data.len)
	{
		DBG1(DBG_KNL, "error writing to ESP socket: %s", strerror(errno));
	}
	packet->destroy(packet);
	return JOB_REQUEUE_DIRECT;
}

CALLBACK(receive_esp, bool,
	private_kernel_libipsec_esp_handler_t *this, int fd, watcher_event_t event)
{
	char buf[2048];
	struct msghdr msg;
	struct cmsghdr *cmsg;
	struct iovec iov;
	char ancillary[64];
	union {
		struct sockaddr_in in4;
		struct sockaddr_in6 in6;
	} src;
	host_t *source, *destination = NULL;
	packet_t *packet;
	chunk_t data;
	ssize_t len;

	msg.msg_name = &src;
	msg.msg_namelen = sizeof(src);
	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = ancillary;
	msg.msg_controllen = sizeof(ancillary);
	msg.msg_flags = 0;

	len = recvmsg(fd, &msg, MSG_DONTWAIT|MSG_TRUNC);
	if (len < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			DBG1(DBG_KNL, "receiving from ESP socket failed: %s",
				 strerror(errno));
		}
		return TRUE;
	}
	else if (msg.msg_flags & MSG_TRUNC)
	{
		DBG1(DBG_KNL, "ESP packet with length %zd exceeds buffer size of %zu",
			 len, sizeof(buf));
		return TRUE;
	}
	data = chunk_create(buf, len);
	/* skip the IP header returned by IPv4 raw sockets */
	if (fd == this->skt_v4)
	{
		data = chunk_skip(data, sizeof(struct iphdr));
	}

	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg))
	{
		if (cmsg->cmsg_level == IPPROTO_IP &&
			cmsg->cmsg_type == IP_PKTINFO)
		{
			const struct in_pktinfo *pktinfo = (struct in_pktinfo*)CMSG_DATA(cmsg);
			struct sockaddr_in dst = {
				.sin_family = AF_INET,
			};

			memcpy(&dst.sin_addr, &pktinfo->ipi_addr, sizeof(dst.sin_addr));
			destination = host_create_from_sockaddr((sockaddr_t*)&dst);
		}
		else if (cmsg->cmsg_level == IPPROTO_IPV6 &&
				 cmsg->cmsg_type == IPV6_PKTINFO)
		{
			const struct in6_pktinfo *pktinfo = (struct in6_pktinfo*)CMSG_DATA(cmsg);
			struct sockaddr_in6 dst = {
				.sin6_family = AF_INET6,
			};

			memcpy(&dst.sin6_addr, &pktinfo->ipi6_addr, sizeof(dst.sin6_addr));
			destination = host_create_from_sockaddr((sockaddr_t*)&dst);
		}
		if (destination)
		{
			break;
		}
	}
	if (!destination)
	{
		DBG1(DBG_KNL, "error reading destination IP address for ESP packet");
		return TRUE;
	}
	source = host_create_from_sockaddr((sockaddr_t*)&src);
	DBG2(DBG_NET, "received raw ESP packet: from %#H to %#H (%zu data bytes)",
		 source, destination, data.len);

	packet = packet_create();
	packet->set_source(packet, source);
	packet->set_destination(packet, destination);
	packet->set_data(packet, chunk_clone(data));
	ipsec->processor->queue_inbound(ipsec->processor,
									esp_packet_create_from_packet(packet));
	return TRUE;
}

METHOD(kernel_libipsec_esp_handler_t, destroy, void,
	private_kernel_libipsec_esp_handler_t *this)
{
	if (this->skt_v4 >= 0)
	{
		lib->watcher->remove(lib->watcher, this->skt_v4);
		close(this->skt_v4);
	}
	if (this->skt_v6 >= 0)
	{
		lib->watcher->remove(lib->watcher, this->skt_v6);
		close(this->skt_v6);
	}
	this->queue->destroy_offset(this->queue, offsetof(esp_packet_t, destroy));
	free(this);
}

/**
 * Create a RAW socket for the given address family
 */
static int create_socket(int family)
{
	const char *fwmark;
	mark_t mark;
	int skt, on = 1;

	skt = socket(family, SOCK_RAW, IPPROTO_ESP);
	if (skt == -1)
	{
		DBG1(DBG_KNL, "opening RAW socket for ESP failed: %s", strerror(errno));
		return -1;
	}
	if (setsockopt(skt, family == AF_INET ? IPPROTO_IP : IPPROTO_IPV6,
				   family == AF_INET ? IP_PKTINFO : IPV6_RECVPKTINFO,
				   &on, sizeof(on)) == -1)
	{
		DBG1(DBG_KNL, "unable to set PKTINFO on ESP socket: %s",
			 strerror(errno));
		close(skt);
		return -1;
	}
	fwmark = lib->settings->get_str(lib->settings,
					"%s.plugins.kernel-libipsec.fwmark",
						lib->settings->get_str(lib->settings,
							"%s.plugins.socket-default.fwmark", NULL, lib->ns),
					lib->ns);
	if (fwmark && mark_from_string(fwmark, MARK_OP_NONE, &mark) &&
		setsockopt(skt, SOL_SOCKET, SO_MARK, &mark.value, sizeof(mark.value)) < 0)
	{
		DBG1(DBG_KNL, "unable to set SO_MARK on ESP socket: %s",
			 strerror(errno));
	}
	return skt;
}

/*
 * Described in header
 */
kernel_libipsec_esp_handler_t *kernel_libipsec_esp_handler_create()
{
	private_kernel_libipsec_esp_handler_t *this;

	if (!lib->caps->keep(lib->caps, CAP_NET_RAW))
	{	/* required to open SOCK_RAW sockets and according to capabilities(7)
		 * it is also required to use the socket */
		DBG1(DBG_KNL, "kernel-libipsec requires CAP_NET_RAW capability to send "
			 "and receive ESP packets without UDP encapsulation");
		return NULL;
	}

	INIT(this,
		.public = {
			.send = _send_,
			.destroy = _destroy,
		},
		.queue = blocking_queue_create(),
		.skt_v4 = create_socket(AF_INET),
		.skt_v6 = create_socket(AF_INET6),
	);

	if (this->skt_v4 == -1 && this->skt_v6  == -1)
	{
		destroy(this);
		return NULL;
	}
	if (this->skt_v4 >= 0)
	{
		lib->watcher->add(lib->watcher, this->skt_v4, WATCHER_READ,
						  receive_esp, this);
	}
	if (this->skt_v6 >= 0)
	{
		lib->watcher->add(lib->watcher, this->skt_v6, WATCHER_READ,
						  receive_esp, this);
	}
	lib->processor->queue_job(lib->processor,
			(job_t*)callback_job_create(send_esp, this, NULL,
										(callback_job_cancel_t)return_false));
	return &this->public;
}

#else /* __linux__ */

kernel_libipsec_esp_handler_t *kernel_libipsec_esp_handler_create()
{
	return NULL;
}

#endif /* __linux__ */
