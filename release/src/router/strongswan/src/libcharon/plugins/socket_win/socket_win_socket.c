/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

/* for WSAID_WSASENDMSG, Windows 7 */
#define _WIN32_WINNT 0x0601

#include "socket_win_socket.h"

#include <library.h>
#include <threading/thread.h>
#include <daemon.h>

#include <mswsock.h>

/* number of sockets in use */
#define SOCKET_COUNT 2

/* missing on MinGW */
#ifndef IPV6_V6ONLY
# define IPV6_V6ONLY 27
#endif

/* GUIDS to lookup WSASend/RecvMsg */
static GUID WSARecvMsgGUID = WSAID_WSARECVMSG;
static GUID WSASendMsgGUID = WSAID_WSASENDMSG;

typedef struct private_socket_win_socket_t private_socket_win_socket_t;

/**
 * Private data of an socket_t object
 */
struct private_socket_win_socket_t {

	/**
	 * public functions
	 */
	socket_win_socket_t public;

	/**
	 * Port for each socket
	 */
	uint16_t ports[SOCKET_COUNT];

	/**
	 * IPv4/IPv6 dual-use sockets
	 */
	SOCKET socks[SOCKET_COUNT];

	/**
	 * Events to wait for socket data
	 */
	HANDLE events[SOCKET_COUNT];

	/**
	 * Maximum packet size to receive
	 */
	int max_packet;

	/**
	 * WSASendMsg function
	 */
	int WINAPI (*WSASendMsg)(SOCKET, LPWSAMSG, DWORD, LPDWORD,
					LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);

	/**
	 * WSARecvMsg function
	 */
	int WINAPI (*WSARecvMsg)(SOCKET, LPWSAMSG, LPDWORD,
					LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
};

METHOD(socket_t, receiver, status_t,
	private_socket_win_socket_t *this, packet_t **out)
{
	char buf[this->max_packet], cbuf[128];
	bool old;
	DWORD i, len, err;
	WSAMSG msg;
	WSABUF data;
	WSACMSGHDR *cmsg;
	SOCKADDR_IN6 addr;
	host_t *src = NULL, *dst = NULL;
	packet_t *pkt;

	data.buf = buf;
	data.len = sizeof(buf);

	memset(&msg, 0, sizeof(msg));
	msg.name = (struct sockaddr*)&addr;
	msg.namelen = sizeof(addr);
	msg.lpBuffers = &data;
	msg.dwBufferCount = 1;
	msg.Control.buf = cbuf;
	msg.Control.len = sizeof(cbuf);

	/* wait for socket events */
	old = thread_cancelability(TRUE);
	i = WSAWaitForMultipleEvents(SOCKET_COUNT, this->events,
								 FALSE, INFINITE, TRUE);
	thread_cancelability(old);
	if (i < WSA_WAIT_EVENT_0 || i >= WSA_WAIT_EVENT_0 + SOCKET_COUNT)
	{
		DBG1(DBG_NET, "waiting on sockets failed: %d", WSAGetLastError());
		return FAILED;
	}
	i -= WSA_WAIT_EVENT_0;

	/* WSAEvents must be reset manually */
	WSAResetEvent(this->events[i]);

	if (this->WSARecvMsg(this->socks[i], &msg, &len,
						 NULL, NULL) == SOCKET_ERROR)
	{
		err = WSAGetLastError();
		/* ignore WSAECONNRESET; this is returned for any ICMP port unreachable,
		 * for a packet we sent, but is most likely not related to the packet
		 * we try to receive. */
		if (err != WSAECONNRESET)
		{
			DBG1(DBG_NET, "reading from socket failed: %d", WSAGetLastError());
		}
		return FAILED;
	}

	DBG3(DBG_NET, "received packet %b", buf, (int)len);

	for (cmsg = WSA_CMSG_FIRSTHDR(&msg); dst == NULL && cmsg != NULL;
		 cmsg = WSA_CMSG_NXTHDR(&msg, cmsg))
	{
		if (cmsg->cmsg_level == IPPROTO_IP &&
			cmsg->cmsg_type == IP_PKTINFO)
		{
			struct in_pktinfo *pktinfo;
			struct sockaddr_in sin = {
				.sin_family = AF_INET,
			};

			pktinfo = (struct in_pktinfo*)WSA_CMSG_DATA(cmsg);
			sin.sin_addr = pktinfo->ipi_addr;
			sin.sin_port = htons(this->ports[i]);
			dst = host_create_from_sockaddr((struct sockaddr*)&sin);
		}
		if (cmsg->cmsg_level == IPPROTO_IPV6 &&
			cmsg->cmsg_type == IPV6_PKTINFO)
		{
			struct in6_pktinfo *pktinfo;
			struct sockaddr_in6 sin = {
				.sin6_family = AF_INET6,
			};

			pktinfo = (struct in6_pktinfo*)WSA_CMSG_DATA(cmsg);
			sin.sin6_addr = pktinfo->ipi6_addr;
			sin.sin6_port = htons(this->ports[i]);
			dst = host_create_from_sockaddr((struct sockaddr*)&sin);
		}
	}

	if (!dst)
	{
		DBG1(DBG_NET, "receiving IP destination address failed");
		return FAILED;
	}

	switch (dst->get_family(dst))
	{
		case AF_INET6:
			src = host_create_from_sockaddr((struct sockaddr*)&addr);
			break;
		case AF_INET:
			/* extract v4 address from mapped v6 */
			src = host_create_from_chunk(AF_INET,
								chunk_create(addr.sin6_addr.u.Byte + 12, 4),
								ntohs(addr.sin6_port));
			break;
	}
	if (!src)
	{
		DBG1(DBG_NET, "receiving IP source address failed");
		dst->destroy(dst);
		return FAILED;
	}

	pkt = packet_create();
	pkt->set_source(pkt, src);
	pkt->set_destination(pkt, dst);
	DBG2(DBG_NET, "received packet: from %#H to %#H", src, dst);
	pkt->set_data(pkt, chunk_clone(chunk_create(buf, len)));

	*out = pkt;
	return SUCCESS;
}

METHOD(socket_t, sender, status_t,
	private_socket_win_socket_t *this, packet_t *packet)
{
	uint16_t port;
	int i = -1, j;
	host_t *src, *dst;
	WSAMSG msg;
	DWORD len;
	WSABUF data;
	WSACMSGHDR *cmsg;
	SOCKADDR_IN6 addr = {
		.sin6_family = AF_INET6,
		.sin6_addr = {
			.u = {
				.Byte = {
					/* v6-mapped-v4 by default */
					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
					0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,
				},
			},
		},
	};
	char buf[WSA_CMSG_SPACE(max(sizeof(struct in6_pktinfo),
								sizeof(struct in_pktinfo)))];

	src = packet->get_source(packet);
	dst = packet->get_destination(packet);
	data.len = packet->get_data(packet).len;
	data.buf = packet->get_data(packet).ptr;

	DBG2(DBG_NET, "sending packet: from %#H to %#H", src, dst);

	DBG3(DBG_NET, "sending packet %b", data.buf, (int)data.len);

	port = src->get_port(src);
	for (j = 0; j < SOCKET_COUNT; j++)
	{
		if (!port || this->ports[j] == port)
		{
			i = j;
			break;
		}
	}
	if (i == -1)
	{
		DBG1(DBG_NET, "no socket found to send packet from port %u", port);
		return FAILED;
	}

	/* copy destination IPv6, or last 32 bits of mapped IPv4 address */
	len = dst->get_address(dst).len;
	if (len > sizeof(addr.sin6_addr))
	{
		return FAILED;
	}
	memcpy(addr.sin6_addr.u.Byte + sizeof(addr.sin6_addr) - len,
		   dst->get_address(dst).ptr, len);
	addr.sin6_port = htons(dst->get_port(dst));

	memset(&msg, 0, sizeof(msg));
	msg.name = (struct sockaddr*)&addr;
	msg.namelen = sizeof(addr);
	msg.lpBuffers = &data;
	msg.dwBufferCount = 1;

	if (!src->is_anyaddr(src))
	{
		memset(buf, 0, sizeof(buf));
		msg.Control.buf = buf;

		switch (src->get_family(src))
		{
			case AF_INET:
			{
				struct in_pktinfo *pktinfo;
				SOCKADDR_IN *sin;

				msg.Control.len = WSA_CMSG_SPACE(sizeof(*pktinfo));
				cmsg = WSA_CMSG_FIRSTHDR(&msg);
				cmsg->cmsg_level = IPPROTO_IP;
				cmsg->cmsg_type = IP_PKTINFO;
				cmsg->cmsg_len = WSA_CMSG_LEN(sizeof(*pktinfo));
				pktinfo = (struct in_pktinfo*)WSA_CMSG_DATA(cmsg);
				sin = (SOCKADDR_IN*)src->get_sockaddr(src);
				pktinfo->ipi_addr = sin->sin_addr;
				break;
			}
			case AF_INET6:
			{
				struct in6_pktinfo *pktinfo;
				SOCKADDR_IN6 *sin;

				msg.Control.len = WSA_CMSG_SPACE(sizeof(*pktinfo));
				cmsg = WSA_CMSG_FIRSTHDR(&msg);
				cmsg->cmsg_level = IPPROTO_IPV6;
				cmsg->cmsg_type = IPV6_PKTINFO;
				cmsg->cmsg_len = WSA_CMSG_LEN(sizeof(*pktinfo));
				pktinfo = (struct in6_pktinfo*)WSA_CMSG_DATA(cmsg);
				sin = (SOCKADDR_IN6*)src->get_sockaddr(src);
				pktinfo->ipi6_addr = sin->sin6_addr;
				break;
			}
		}
	}

	if (this->WSASendMsg(this->socks[i], &msg, 0, &len,
						 NULL, NULL) == SOCKET_ERROR)
	{
		DBG1(DBG_NET, "sending packet failed: %d", WSAGetLastError());
		return FAILED;
	}
	return SUCCESS;
}

METHOD(socket_t, get_port, uint16_t,
	private_socket_win_socket_t *this, bool nat)
{
	return this->ports[nat != 0];
}

METHOD(socket_t, supported_families, socket_family_t,
	private_socket_win_socket_t *this)
{
	return SOCKET_FAMILY_IPV4 | SOCKET_FAMILY_IPV6;
}

/**
 * Open an IPv4/IPv6 dual-use socket to send and receive packets
 */
static SOCKET open_socket(private_socket_win_socket_t *this, int i)
{
	SOCKADDR_IN6 addr = {
		.sin6_family = AF_INET6,
		.sin6_port = htons(this->ports[i]),
	};
	int addrlen = sizeof(addr);
	BOOL on = TRUE, off = FALSE;
	DWORD dwon = TRUE;
	SOCKET s;

	s = WSASocket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, 0);
	if (s == INVALID_SOCKET)
	{
		DBG1(DBG_NET, "creating socket failed: %d", WSAGetLastError());
		return INVALID_SOCKET;
	}
	/* enable IPv4 on IPv6 socket */
	if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY,
				   (const char*)&off, sizeof(off)) == SOCKET_ERROR)
	{
		DBG1(DBG_NET, "using dual-mode socket failed: %d", WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
				   (const char*)&on, sizeof(on)) == SOCKET_ERROR)
	{
		DBG1(DBG_NET, "enabling SO_REUSEADDR failed: %d", WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}
	if (bind(s, (const struct sockaddr*)&addr, addrlen) == SOCKET_ERROR)
	{
		DBG1(DBG_NET, "unable to bind socket: %d", WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}
	/* retrieve randomly allocated port if needed */
	if (this->ports[i] == 0)
	{
		if (getsockname(s, (struct sockaddr*)&addr,
						&addrlen) == SOCKET_ERROR)
		{
			DBG1(DBG_NET, "unable to determine port: %d", WSAGetLastError());
			closesocket(s);
			return INVALID_SOCKET;
		}
		this->ports[i] = ntohs(addr.sin6_port);
	}
	/* PKTINFO is required for both protocol families */
	if (setsockopt(s, IPPROTO_IP, IP_PKTINFO,
				   (char*)&dwon, sizeof(dwon)) == SOCKET_ERROR)
	{
		DBG1(DBG_NET, "unable to set IP_PKTINFO: %d", WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}
	if (setsockopt(s, IPPROTO_IPV6, IPV6_PKTINFO,
				   (char*)&dwon, sizeof(dwon)) == SOCKET_ERROR)
	{
		DBG1(DBG_NET, "unable to set IP6_PKTINFO: %d", WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}
	if (!charon->kernel->bypass_socket(charon->kernel, s, AF_INET))
	{
		DBG1(DBG_NET, "installing IPv4 IKE bypass policy failed");
	}
	if (!charon->kernel->bypass_socket(charon->kernel, s, AF_INET6))
	{
		DBG1(DBG_NET, "installing IPv6 IKE bypass policy failed");
	}
	return s;
}

METHOD(socket_t, destroy, void,
	private_socket_win_socket_t *this)
{
	int i;

	for (i = 0; i < SOCKET_COUNT; i++)
	{
		if (this->socks[i] != INVALID_SOCKET)
		{
			closesocket(this->socks[i]);
		}
		if (this->events[i] != WSA_INVALID_EVENT)
		{
			WSACloseEvent(this->events[i]);
		}
	}
	free(this);
}

/*
 * See header for description
 */
socket_win_socket_t *socket_win_socket_create()
{
	private_socket_win_socket_t *this;
	DWORD len;
	int i;

	INIT(this,
		.public = {
			.socket = {
				.send = _sender,
				.receive = _receiver,
				.get_port = _get_port,
				.supported_families = _supported_families,
				.destroy = _destroy,
			},
		},
		.ports = {
			lib->settings->get_int(lib->settings,
							"%s.port", CHARON_UDP_PORT, lib->ns),
			lib->settings->get_int(lib->settings,
							"%s.port_nat_t", CHARON_NATT_PORT, lib->ns),
		},
		.max_packet = lib->settings->get_int(lib->settings,
							"%s.max_packet", PACKET_MAX_DEFAULT, lib->ns),
	);

	for (i = 0; i < SOCKET_COUNT; i++)
	{
		this->socks[i] = open_socket(this, i);
		this->events[i] = WSACreateEvent();
	}

	for (i = 0; i < SOCKET_COUNT; i++)
	{
		if (this->events[i] == WSA_INVALID_EVENT ||
			this->socks[i] == INVALID_SOCKET)
		{
			DBG1(DBG_NET, "creating socket failed: %d", WSAGetLastError());
			destroy(this);
			return NULL;
		}
		if (WSAEventSelect(this->socks[i], this->events[i],
						   FD_READ) == SOCKET_ERROR)
		{
			DBG1(DBG_NET, "WSAEventSelect() failed: %d", WSAGetLastError());
			destroy(this);
			return NULL;
		}
	}

	if (WSAIoctl(this->socks[0], SIO_GET_EXTENSION_FUNCTION_POINTER,
			&WSASendMsgGUID, sizeof(WSASendMsgGUID), &this->WSASendMsg,
			sizeof(this->WSASendMsg), &len, NULL, NULL) != 0 ||
		WSAIoctl(this->socks[0], SIO_GET_EXTENSION_FUNCTION_POINTER,
			&WSARecvMsgGUID, sizeof(WSARecvMsgGUID), &this->WSARecvMsg,
			sizeof(this->WSARecvMsg), &len, NULL, NULL) != 0)
	{
		DBG1(DBG_NET, "send/recvmsg() lookup failed: %d", WSAGetLastError());
		destroy(this);
		return NULL;
	}

	return &this->public;
}
