/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "farp_spoofer.h"

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/filter.h>
#include <sys/ioctl.h>

#include <daemon.h>
#include <threading/thread.h>
#include <processing/jobs/callback_job.h>

typedef struct private_farp_spoofer_t private_farp_spoofer_t;

/**
 * Private data of an farp_spoofer_t object.
 */
struct private_farp_spoofer_t {

	/**
	 * Public farp_spoofer_t interface.
	 */
	farp_spoofer_t public;

	/**
	 * Listener that knows active addresses
	 */
	farp_listener_t *listener;

	/**
	 * RAW socket for ARP requests
	 */
	int skt;
};

/**
 * IP over Ethernet ARP message
 */
typedef struct __attribute__((packed)) {
	u_int16_t hardware_type;
	u_int16_t protocol_type;
	u_int8_t hardware_size;
	u_int8_t protocol_size;
	u_int16_t opcode;
	u_int8_t sender_mac[6];
	u_int8_t sender_ip[4];
	u_int8_t target_mac[6];
	u_int8_t target_ip[4];
} arp_t;

/**
 * Send faked ARP response
 */
static void send_arp(private_farp_spoofer_t *this,
					 arp_t *arp, struct sockaddr_ll *addr)
{
	struct ifreq req;
	char tmp[4];

	req.ifr_ifindex = addr->sll_ifindex;
	if (ioctl(this->skt, SIOCGIFNAME, &req) == 0 &&
		ioctl(this->skt, SIOCGIFHWADDR, &req) == 0 &&
		req.ifr_hwaddr.sa_family == ARPHRD_ETHER)
	{
		memcpy(arp->target_mac, arp->sender_mac, 6);
		memcpy(arp->sender_mac, req.ifr_hwaddr.sa_data, 6);

		memcpy(tmp, arp->sender_ip, 4);
		memcpy(arp->sender_ip, arp->target_ip, 4);
		memcpy(arp->target_ip, tmp, 4);

		arp->opcode = htons(ARPOP_REPLY);

		sendto(this->skt, arp, sizeof(*arp), 0,
			   (struct sockaddr*)addr, sizeof(*addr));
	}
}

/**
 * ARP request receiving
 */
static bool receive_arp(private_farp_spoofer_t *this)
{
	struct sockaddr_ll addr;
	socklen_t addr_len = sizeof(addr);
	arp_t arp;
	ssize_t len;
	host_t *local, *remote;

	len = recvfrom(this->skt, &arp, sizeof(arp), MSG_DONTWAIT,
				   (struct sockaddr*)&addr, &addr_len);
	if (len == sizeof(arp))
	{
		local = host_create_from_chunk(AF_INET,
									chunk_create((char*)&arp.sender_ip, 4), 0);
		remote = host_create_from_chunk(AF_INET,
									chunk_create((char*)&arp.target_ip, 4), 0);
		if (this->listener->has_tunnel(this->listener, local, remote))
		{
			send_arp(this, &arp, &addr);
		}
		local->destroy(local);
		remote->destroy(remote);
	}

	return TRUE;
}

METHOD(farp_spoofer_t, destroy, void,
	private_farp_spoofer_t *this)
{
	lib->watcher->remove(lib->watcher, this->skt);
	close(this->skt);
	free(this);
}

/**
 * See header
 */
farp_spoofer_t *farp_spoofer_create(farp_listener_t *listener)
{
	private_farp_spoofer_t *this;
	struct sock_filter arp_request_filter_code[] = {
		BPF_STMT(BPF_LD+BPF_H+BPF_ABS, offsetof(arp_t, protocol_type)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, ETH_P_IP, 0, 9),
		BPF_STMT(BPF_LD+BPF_B+BPF_ABS, offsetof(arp_t, hardware_size)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 6, 0, 7),
		BPF_STMT(BPF_LD+BPF_B+BPF_ABS, offsetof(arp_t, protocol_size)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 4, 0, 4),
		BPF_STMT(BPF_LD+BPF_H+BPF_ABS, offsetof(arp_t, opcode)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, ARPOP_REQUEST, 0, 3),
		BPF_STMT(BPF_LD+BPF_W+BPF_LEN, 0),
		BPF_JUMP(BPF_JMP+BPF_JGE+BPF_K, sizeof(arp_t), 0, 1),
		BPF_STMT(BPF_RET+BPF_K, sizeof(arp_t)),
		BPF_STMT(BPF_RET+BPF_K, 0),
	};
	struct sock_fprog arp_request_filter = {
		sizeof(arp_request_filter_code) / sizeof(struct sock_filter),
		arp_request_filter_code,
	};

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
		.listener = listener,
	);

	this->skt = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ARP));
	if (this->skt == -1)
	{
		DBG1(DBG_NET, "opening ARP packet socket failed: %s", strerror(errno));
		free(this);
		return NULL;
	}

	if (setsockopt(this->skt, SOL_SOCKET, SO_ATTACH_FILTER,
				   &arp_request_filter, sizeof(arp_request_filter)) < 0)
	{
		DBG1(DBG_NET, "installing ARP packet filter failed: %s", strerror(errno));
		close(this->skt);
		free(this);
		return NULL;
	}

	lib->watcher->add(lib->watcher, this->skt, WATCHER_READ,
					  (watcher_cb_t)receive_arp, this);

	return &this->public;
}
