/*
 * Copyright (C) 2021-2024 Tobias Brunner
 * Copyright (C) 2020-2023 Dan James <sddj@me.com>
 * Copyright (C) 2010 Martin Willi
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

#include "farp_spoofer.h"

#include <errno.h>
#include <unistd.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>

#if !defined(__APPLE__) && !defined(__FreeBSD__)
#include <sys/socket.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <linux/filter.h>
#else
#include <net/bpf.h>
#include <net/if_arp.h>
#include <net/if_dl.h>
#endif /* !defined(__APPLE__) && !defined(__FreeBSD__) */

#include <daemon.h>
#include <threading/thread.h>
#include <processing/jobs/callback_job.h>
#include <network/pf_handler.h>

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
	 * BPF handler for ARP requests
	 */
	pf_handler_t *pf_handler;
};

/**
 * IP over Ethernet ARP message
 */
typedef struct __attribute__((packed)) {
	uint16_t hardware_type;
	uint16_t protocol_type;
	uint8_t hardware_size;
	uint8_t protocol_size;
	uint16_t opcode;
	uint8_t sender_mac[ETHER_ADDR_LEN];
	uint8_t sender_ip[4];
	uint8_t target_mac[ETHER_ADDR_LEN];
	uint8_t target_ip[4];
} arp_t;

#if !defined(__APPLE__) && !defined(__FreeBSD__)

/**
 * Send faked ARP response
 */
static void send_arp(char *if_name, int if_index, chunk_t mac, int fd,
					 arp_t *arp, host_t *sender, host_t *target)
{
	struct sockaddr_ll addr = {
		.sll_family = AF_PACKET,
		.sll_protocol = htons(ETH_P_ARP),
		.sll_ifindex = if_index,
		.sll_halen = ETHER_ADDR_LEN,
	};
	char tmp[4];
	ssize_t len;

#if DEBUG_LEVEL >= 2
	chunk_t sender_mac = chunk_create((u_char*)arp->sender_mac, ETHER_ADDR_LEN);

	DBG2(DBG_NET, "replying with %#B to ARP request for %H from %H (%#B) on %s",
		 &mac, target, sender, &sender_mac, if_name);
#endif

	memcpy(addr.sll_addr, arp->sender_mac, ETHER_ADDR_LEN);

	memcpy(arp->target_mac, arp->sender_mac, 6);
	memcpy(arp->sender_mac, mac.ptr, 6);

	memcpy(tmp, arp->sender_ip, 4);
	memcpy(arp->sender_ip, arp->target_ip, 4);
	memcpy(arp->target_ip, tmp, 4);

	arp->opcode = htons(ARPOP_REPLY);

	len = sendto(fd, arp, sizeof(*arp), 0,
				 (const struct sockaddr*)&addr, sizeof(addr));

	if (len != sizeof(*arp))
	{
		DBG1(DBG_NET, "failed to send ARP reply: %s", strerror(errno));
	}
}

#else /* !defined(__APPLE__) && !defined(__FreeBSD__) */

/**
 * An Ethernet frame for an ARP packet.
 */
struct frame_t {
	struct ether_header e;
	arp_t a;
};

typedef struct frame_t frame_t;

/**
 * Send an ARP response for the given ARP request.
 */
static void send_arp(char *if_name, int if_index, chunk_t mac, int fd,
					 const arp_t *arpreq, host_t *sender, host_t *target)
{
	frame_t frame;
	ssize_t n;

#if DEBUG_LEVEL >= 2
	chunk_t sender_mac = chunk_create((u_char*)arpreq->sender_mac, ETHER_ADDR_LEN);

	DBG2(DBG_NET, "replying with %#B to ARP request for %H from %H (%#B) on %s",
		 &mac, target, sender, &sender_mac, if_name);
#endif

	memcpy(frame.e.ether_dhost, arpreq->sender_mac, ETHER_ADDR_LEN);
	memcpy(frame.e.ether_shost, mac.ptr, ETHER_ADDR_LEN);
	frame.e.ether_type = htons(ETHERTYPE_ARP);

	frame.a.hardware_type = htons(1);
	frame.a.protocol_type = htons(ETHERTYPE_IP);
	frame.a.hardware_size = arpreq->hardware_size;
	frame.a.protocol_size = arpreq->protocol_size;
	frame.a.opcode = htons(ARPOP_REPLY);
	memcpy(frame.a.sender_mac, mac.ptr, ETHER_ADDR_LEN);
	memcpy(frame.a.sender_ip, arpreq->target_ip, sizeof(arpreq->target_ip));
	memcpy(frame.a.target_mac, arpreq->sender_mac, sizeof(arpreq->sender_mac));
	memcpy(frame.a.target_ip, arpreq->sender_ip, sizeof(arpreq->sender_ip));

	n = write(fd, &frame, sizeof(frame));
	if (n != sizeof(frame))
	{
		DBG1(DBG_NET, "sending ARP reply failed: %s", strerror(errno));
	}
}

#endif /* !defined(__APPLE__) && !defined(__FreeBSD__) */

CALLBACK(handle_arp_pkt, void,
	private_farp_spoofer_t *this, char *if_name, int if_index, chunk_t mac,
	int fd, chunk_t packet)
{
	arp_t *a = (arp_t*)packet.ptr;
	host_t *sender, *target;

	if (packet.len == sizeof(arp_t))
	{
		sender = host_create_from_chunk(AF_INET,
										chunk_create((char*)a->sender_ip, 4), 0);
		target = host_create_from_chunk(AF_INET,
										chunk_create((char*)a->target_ip, 4), 0);
		if (this->listener->has_tunnel(this->listener, sender, target))
		{
			send_arp(if_name, if_index, mac, fd, a, sender, target);
		}
		else
		{
			DBG2(DBG_NET, "not sending ARP reply, no tunnel between %H -> %H",
				 sender, target);
		}
		target->destroy(target);
		sender->destroy(sender);
	}
	else
	{
		DBG1(DBG_NET, "ARP request with invalid size %d received (expected: %d)",
			 packet.len, sizeof(arp_t));
	}
}

/**
 * Cleanup the handlers used by this plugin.
 */
METHOD(farp_spoofer_t, destroy, void, private_farp_spoofer_t *this)
{
	this->pf_handler->destroy(this->pf_handler);
	free(this);
}

/**
 * See header
 */
farp_spoofer_t *farp_spoofer_create(farp_listener_t *listener)
{
#if !defined(__APPLE__) && !defined(__FreeBSD__)
	struct sock_filter arp_request_filter_code[] = {
		BPF_STMT(BPF_LD+BPF_H+BPF_ABS, offsetof(arp_t, protocol_type)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, ETH_P_IP, 0, 9),
		BPF_STMT(BPF_LD+BPF_B+BPF_ABS, offsetof(arp_t, hardware_size)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 6, 0, 7),
		BPF_STMT(BPF_LD+BPF_B+BPF_ABS, offsetof(arp_t, protocol_size)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 4, 0, 5),
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
#else
	const size_t skip_eth = sizeof(struct ether_header);
	struct bpf_insn instructions[] = {
		BPF_STMT(BPF_LD+BPF_W+BPF_LEN, 0),
		BPF_JUMP(BPF_JMP+BPF_JGE+BPF_K, skip_eth + sizeof(arp_t), 0, 11),
		BPF_STMT(BPF_LD+BPF_H+BPF_ABS, offsetof(struct ether_header, ether_type)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, ETHERTYPE_ARP, 0, 9),
		BPF_STMT(BPF_LD+BPF_H+BPF_ABS, skip_eth + offsetof(arp_t, protocol_type)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, ETHERTYPE_IP, 0, 7),
		BPF_STMT(BPF_LD+BPF_B+BPF_ABS, skip_eth + offsetof(arp_t, hardware_size)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 6, 0, 5),
		BPF_STMT(BPF_LD+BPF_B+BPF_ABS, skip_eth + offsetof(arp_t, protocol_size)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 4, 0, 3),
		BPF_STMT(BPF_LD+BPF_H+BPF_ABS, skip_eth + offsetof(arp_t, opcode)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, ARPOP_REQUEST, 0, 1),
		BPF_STMT(BPF_RET+BPF_K, 14 + sizeof(arp_t)),
		BPF_STMT(BPF_RET+BPF_K, 0)
	};
	struct bpf_program arp_request_filter = {
		sizeof(instructions) / sizeof(struct bpf_insn),
		&instructions[0]
	};
#endif
	private_farp_spoofer_t *this;

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
		.listener = listener,
	);

	this->pf_handler = pf_handler_create("ARP", NULL, handle_arp_pkt, this,
										 &arp_request_filter);
	if (!this->pf_handler)
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}
