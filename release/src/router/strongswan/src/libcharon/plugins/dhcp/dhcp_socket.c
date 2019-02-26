/*
 * Copyright (C) 2012-2018 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
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

#include "dhcp_socket.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/filter.h>

#include <collections/linked_list.h>
#include <utils/identification.h>
#include <threading/mutex.h>
#include <threading/condvar.h>
#include <threading/thread.h>

#include <daemon.h>
#include <processing/jobs/callback_job.h>

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68
#define DHCP_TRIES 5

typedef struct private_dhcp_socket_t private_dhcp_socket_t;

/**
 * Private data of an dhcp_socket_t object.
 */
struct private_dhcp_socket_t {

	/**
	 * Public dhcp_socket_t interface.
	 */
	dhcp_socket_t public;

	/**
	 * Random number generator
	 */
	rng_t *rng;

	/**
	 * List of transactions in DISCOVER
	 */
	linked_list_t *discover;

	/**
	 * List of transactions in REQUEST
	 */
	linked_list_t *request;

	/**
	 * List of successfully completed transactions
	 */
	linked_list_t *completed;

	/**
	 * Lock for transactions
	 */
	mutex_t *mutex;

	/**
	 * Condvar to wait for transaction completion
	 */
	condvar_t *condvar;

	/**
	 * Threads waiting in condvar
	 */
	int waiting;

	/**
	 * DHCP send socket
	 */
	int send;

	/**
	 * DHCP receive socket
	 */
	int receive;

	/**
	 * Do we use per-identity or random leases (and MAC addresses)
	 */
	bool identity_lease;

	/**
	 * DHCP server address, or broadcast
	 */
	host_t *dst;

	/**
	 * Force configured destination address
	 */
	bool force_dst;
};

/**
 * DHCP opcode (or BOOTP actually)
 */
typedef enum {
	BOOTREQUEST = 1,
	BOOTREPLY = 2,
} dhcp_opcode_t;

/**
 * Some DHCP options used
 */
typedef enum {
	DHCP_DNS_SERVER = 6,
	DHCP_HOST_NAME = 12,
	DHCP_NBNS_SERVER = 44,
	DHCP_REQUESTED_IP = 50,
	DHCP_MESSAGE_TYPE = 53,
	DHCP_SERVER_ID = 54,
	DHCP_PARAM_REQ_LIST = 55,
	DHCP_CLIENT_ID = 61,
	DHCP_OPTEND = 255,
} dhcp_option_type_t;

/**
 * DHCP messages types in the DHCP_MESSAGE_TYPE option
 */
typedef enum {
	DHCP_DISCOVER = 1,
	DHCP_OFFER = 2,
	DHCP_REQUEST = 3,
	DHCP_DECLINE = 4,
	DHCP_ACK = 5,
	DHCP_NAK = 6,
	DHCP_RELEASE = 7,
	DHCP_INFORM = 8,
} dhcp_message_type_t;
/**
 * DHCP option encoding, a TLV
 */
typedef struct __attribute__((packed)) {
	uint8_t type;
	uint8_t len;
	char data[];
} dhcp_option_t;

/**
 * DHCP message format, with a minimum size options buffer
 */
typedef struct __attribute__((packed)) {
	uint8_t opcode;
	uint8_t hw_type;
	uint8_t hw_addr_len;
	uint8_t hop_count;
	uint32_t transaction_id;
	uint16_t number_of_seconds;
	uint16_t flags;
	uint32_t client_address;
	uint32_t your_address;
	uint32_t server_address;
	uint32_t gateway_address;
	char client_hw_addr[6];
	char client_hw_padding[10];
	char server_hostname[64];
	char boot_filename[128];
	uint32_t magic_cookie;
	u_char options[308];
} dhcp_t;

/**
 * Check if the given address equals the broadcast address
 */
static inline bool is_broadcast(host_t *host)
{
	chunk_t broadcast = chunk_from_chars(0xFF,0xFF,0xFF,0xFF);

	return chunk_equals(broadcast, host->get_address(host));
}

/**
 * Prepare a DHCP message for a given transaction
 */
static int prepare_dhcp(private_dhcp_socket_t *this,
						dhcp_transaction_t *transaction,
						dhcp_message_type_t type, dhcp_t *dhcp)
{
	chunk_t chunk;
	identification_t *identity;
	dhcp_option_t *option;
	int optlen = 0, remaining;
	host_t *src;
	uint32_t id;

	memset(dhcp, 0, sizeof(*dhcp));
	dhcp->opcode = BOOTREQUEST;
	dhcp->hw_type = ARPHRD_ETHER;
	dhcp->hw_addr_len = 6;
	dhcp->transaction_id = transaction->get_id(transaction);
	if (is_broadcast(this->dst))
	{
		/* Set broadcast flag to get broadcasted replies, as we actually
		 * do not own the MAC we request an address for. */
		dhcp->flags = htons(0x8000);
		/* TODO: send with 0.0.0.0 source address */
	}
	else
	{
		/* act as relay agent */
		src = charon->kernel->get_source_addr(charon->kernel, this->dst, NULL);
		if (src)
		{
			memcpy(&dhcp->gateway_address, src->get_address(src).ptr,
				   sizeof(dhcp->gateway_address));
			src->destroy(src);
		}
	}

	identity = transaction->get_identity(transaction);
	chunk = identity->get_encoding(identity);
	/* magic bytes, a locally administered unicast MAC */
	dhcp->client_hw_addr[0] = 0x7A;
	dhcp->client_hw_addr[1] = 0xA7;
	/* with ID specific postfix */
	if (this->identity_lease)
	{
		id = htonl(chunk_hash_static(chunk));
	}
	else
	{
		id = transaction->get_id(transaction);
	}
	memcpy(&dhcp->client_hw_addr[2], &id, sizeof(id));

	dhcp->magic_cookie = htonl(0x63825363);

	option = (dhcp_option_t*)&dhcp->options[optlen];
	option->type = DHCP_MESSAGE_TYPE;
	option->len = 1;
	option->data[0] = type;
	optlen += sizeof(dhcp_option_t) + option->len;

	/* the REQUEST message has the most static overhead in the 'options' field
	 * with 17 bytes */
	remaining = sizeof(dhcp->options) - optlen - 17;

	if (identity->get_type(identity) == ID_FQDN)
	{
		option = (dhcp_option_t*)&dhcp->options[optlen];
		option->type = DHCP_HOST_NAME;
		option->len = min(min(chunk.len, remaining-sizeof(dhcp_option_t)), 255);
		memcpy(option->data, chunk.ptr, option->len);
		optlen += sizeof(dhcp_option_t) + option->len;
		remaining -= sizeof(dhcp_option_t) + option->len;
	}

	if (this->identity_lease &&
		remaining >= sizeof(dhcp_option_t) + 2)
	{
		option = (dhcp_option_t*)&dhcp->options[optlen];
		option->type = DHCP_CLIENT_ID;
		option->len = min(min(chunk.len, remaining-sizeof(dhcp_option_t)), 255);
		memcpy(option->data, chunk.ptr, option->len);
		optlen += sizeof(dhcp_option_t) + option->len;
	}
	return optlen;
}

/**
 * Send a DHCP message with given options length
 */
static bool send_dhcp(private_dhcp_socket_t *this,
					  dhcp_transaction_t *transaction, dhcp_t *dhcp, int optlen)
{
	host_t *dst;
	ssize_t len;

	dst = transaction->get_server(transaction);
	if (!dst || this->force_dst)
	{
		dst = this->dst;
	}
	len = offsetof(dhcp_t, magic_cookie) + optlen + 4;
	return sendto(this->send, dhcp, len, 0, dst->get_sockaddr(dst),
				  *dst->get_sockaddr_len(dst)) == len;
}

/**
 * Send DHCP discover using a given transaction
 */
static bool discover(private_dhcp_socket_t *this,
					 dhcp_transaction_t *transaction)
{
	dhcp_option_t *option;
	dhcp_t dhcp;
	int optlen;

	optlen = prepare_dhcp(this, transaction, DHCP_DISCOVER, &dhcp);

	DBG1(DBG_CFG, "sending DHCP DISCOVER to %H", this->dst);

	option = (dhcp_option_t*)&dhcp.options[optlen];
	option->type = DHCP_PARAM_REQ_LIST;
	option->len = 2;
	option->data[0] = DHCP_DNS_SERVER;
	option->data[1] = DHCP_NBNS_SERVER;
	optlen += sizeof(dhcp_option_t) + option->len;

	dhcp.options[optlen++] = DHCP_OPTEND;

	if (!send_dhcp(this, transaction, &dhcp, optlen))
	{
		DBG1(DBG_CFG, "sending DHCP DISCOVER failed: %s", strerror(errno));
		return FALSE;
	}
	return TRUE;
}

/**
 * Send DHCP request using a given transaction
 */
static bool request(private_dhcp_socket_t *this,
					dhcp_transaction_t *transaction)
{
	dhcp_option_t *option;
	dhcp_t dhcp;
	host_t *offer, *server;
	chunk_t chunk;
	int optlen;

	optlen = prepare_dhcp(this, transaction, DHCP_REQUEST, &dhcp);

	offer = transaction->get_address(transaction);
	server = transaction->get_server(transaction);
	if (!offer || !server)
	{
		return FALSE;
	}
	DBG1(DBG_CFG, "sending DHCP REQUEST for %H to %H", offer, server);

	option = (dhcp_option_t*)&dhcp.options[optlen];
	option->type = DHCP_REQUESTED_IP;
	option->len = 4;
	chunk = offer->get_address(offer);
	memcpy(option->data, chunk.ptr, min(chunk.len, option->len));
	optlen += sizeof(dhcp_option_t) + option->len;

	option = (dhcp_option_t*)&dhcp.options[optlen];
	option->type = DHCP_SERVER_ID;
	option->len = 4;
	chunk = server->get_address(server);
	memcpy(option->data, chunk.ptr, min(chunk.len, option->len));
	optlen += sizeof(dhcp_option_t) + option->len;

	option = (dhcp_option_t*)&dhcp.options[optlen];
	option->type = DHCP_PARAM_REQ_LIST;
	option->len = 2;
	option->data[0] = DHCP_DNS_SERVER;
	option->data[1] = DHCP_NBNS_SERVER;
	optlen += sizeof(dhcp_option_t) + option->len;

	dhcp.options[optlen++] = DHCP_OPTEND;

	if (!send_dhcp(this, transaction, &dhcp, optlen))
	{
		DBG1(DBG_CFG, "sending DHCP REQUEST failed: %s", strerror(errno));
		return FALSE;
	}
	return TRUE;
}

METHOD(dhcp_socket_t, enroll, dhcp_transaction_t*,
	private_dhcp_socket_t *this, identification_t *identity)
{
	dhcp_transaction_t *transaction;
	uint32_t id;
	int try;

	if (!this->rng->get_bytes(this->rng, sizeof(id), (uint8_t*)&id))
	{
		DBG1(DBG_CFG, "DHCP DISCOVER failed, no transaction ID");
		return NULL;
	}
	transaction = dhcp_transaction_create(id, identity);

	this->mutex->lock(this->mutex);
	this->discover->insert_last(this->discover, transaction);
	try = 1;
	while (try <= DHCP_TRIES && discover(this, transaction))
	{
		if (!this->condvar->timed_wait(this->condvar, this->mutex, 1000 * try) &&
			this->request->find_first(this->request, NULL, (void**)&transaction))
		{
			break;
		}
		try++;
	}
	if (this->discover->remove(this->discover, transaction, NULL))
	{	/* no OFFER received */
		this->mutex->unlock(this->mutex);
		transaction->destroy(transaction);
		DBG1(DBG_CFG, "DHCP DISCOVER timed out");
		return NULL;
	}

	try = 1;
	while (try <= DHCP_TRIES && request(this, transaction))
	{
		if (!this->condvar->timed_wait(this->condvar, this->mutex, 1000 * try) &&
			this->completed->remove(this->completed, transaction, NULL))
		{
			break;
		}
		try++;
	}
	if (this->request->remove(this->request, transaction, NULL))
	{	/* no ACK received */
		this->mutex->unlock(this->mutex);
		transaction->destroy(transaction);
		DBG1(DBG_CFG, "DHCP REQUEST timed out");
		return NULL;
	}
	this->mutex->unlock(this->mutex);

	return transaction;
}

METHOD(dhcp_socket_t, release, void,
	private_dhcp_socket_t *this, dhcp_transaction_t *transaction)
{
	dhcp_option_t *option;
	dhcp_t dhcp;
	host_t *release, *server;
	chunk_t chunk;
	int optlen;

	optlen = prepare_dhcp(this, transaction, DHCP_RELEASE, &dhcp);

	release = transaction->get_address(transaction);
	server = transaction->get_server(transaction);
	if (!release || !server)
	{
		return;
	}
	DBG1(DBG_CFG, "sending DHCP RELEASE for %H to %H", release, server);

	chunk = release->get_address(release);
	memcpy((char*)&dhcp.client_address, chunk.ptr,
		   min(chunk.len, sizeof(dhcp.client_address)));

	option = (dhcp_option_t*)&dhcp.options[optlen];
	option->type = DHCP_SERVER_ID;
	option->len = 4;
	chunk = server->get_address(server);
	memcpy(option->data, chunk.ptr, min(chunk.len, option->len));
	optlen += sizeof(dhcp_option_t) + option->len;

	dhcp.options[optlen++] = DHCP_OPTEND;

	if (!send_dhcp(this, transaction, &dhcp, optlen))
	{
		DBG1(DBG_CFG, "sending DHCP RELEASE failed: %s", strerror(errno));
	}
}

/**
 * Handle a DHCP OFFER
 */
static void handle_offer(private_dhcp_socket_t *this, dhcp_t *dhcp, int optlen)
{
	dhcp_transaction_t *transaction = NULL;
	enumerator_t *enumerator;
	host_t *offer, *server = NULL;

	offer = host_create_from_chunk(AF_INET,
					chunk_from_thing(dhcp->your_address), 0);

	if (offer->is_anyaddr(offer))
	{
		server = host_create_from_chunk(AF_INET,
					chunk_from_thing(dhcp->server_address), 0);
		DBG1(DBG_CFG, "ignoring DHCP OFFER %+H from %H", offer, server);
		server->destroy(server);
		offer->destroy(offer);
		return;
	}

	this->mutex->lock(this->mutex);
	enumerator = this->discover->create_enumerator(this->discover);
	while (enumerator->enumerate(enumerator, &transaction))
	{
		if (transaction->get_id(transaction) == dhcp->transaction_id)
		{
			this->discover->remove_at(this->discover, enumerator);
			this->request->insert_last(this->request, transaction);
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (transaction)
	{
		int optsize, optpos = 0, pos;
		dhcp_option_t *option;

		while (optlen > sizeof(dhcp_option_t))
		{
			option = (dhcp_option_t*)&dhcp->options[optpos];
			optsize = sizeof(dhcp_option_t) + option->len;
			if (option->type == DHCP_OPTEND || optlen < optsize)
			{
				break;
			}
			if (option->type == DHCP_DNS_SERVER ||
				option->type == DHCP_NBNS_SERVER)
			{
				for (pos = 0; pos + 4 <= option->len; pos += 4)
				{
					transaction->add_attribute(transaction, option->type ==
						DHCP_DNS_SERVER ? INTERNAL_IP4_DNS : INTERNAL_IP4_NBNS,
						chunk_create((char*)&option->data[pos], 4));
				}
			}
			if (!server && option->type == DHCP_SERVER_ID && option->len == 4)
			{
				server = host_create_from_chunk(AF_INET,
							chunk_create(option->data, 4), DHCP_SERVER_PORT);
			}
			optlen -= optsize;
			optpos += optsize;
		}
		if (!server)
		{
			server = host_create_from_chunk(AF_INET,
				chunk_from_thing(dhcp->server_address), DHCP_SERVER_PORT);
		}
		DBG1(DBG_CFG, "received DHCP OFFER %H from %H", offer, server);
		transaction->set_address(transaction, offer->clone(offer));
		transaction->set_server(transaction, server);
	}
	this->mutex->unlock(this->mutex);
	this->condvar->broadcast(this->condvar);
	offer->destroy(offer);
}

/**
 * Handle a DHCP ACK
 */
static void handle_ack(private_dhcp_socket_t *this, dhcp_t *dhcp, int optlen)
{
	dhcp_transaction_t *transaction;
	enumerator_t *enumerator;
	host_t *offer;

	offer = host_create_from_chunk(AF_INET,
						chunk_from_thing(dhcp->your_address), 0);

	this->mutex->lock(this->mutex);
	enumerator = this->request->create_enumerator(this->request);
	while (enumerator->enumerate(enumerator, &transaction))
	{
		if (transaction->get_id(transaction) == dhcp->transaction_id)
		{
			DBG1(DBG_CFG, "received DHCP ACK for %H", offer);
			this->request->remove_at(this->request, enumerator);
			this->completed->insert_last(this->completed, transaction);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
	this->condvar->broadcast(this->condvar);
	offer->destroy(offer);
}

/**
 * Receive DHCP responses
 */
static bool receive_dhcp(private_dhcp_socket_t *this, int fd,
						 watcher_event_t event)
{
	struct sockaddr_ll addr;
	socklen_t addr_len = sizeof(addr);
	struct __attribute__((packed)) {
		struct iphdr ip;
		struct udphdr udp;
		dhcp_t dhcp;
	} packet;
	int optlen, origoptlen, optsize, optpos = 0;
	ssize_t len;
	dhcp_option_t *option;

	len = recvfrom(fd, &packet, sizeof(packet), MSG_DONTWAIT,
					(struct sockaddr*)&addr, &addr_len);

	if (len >= sizeof(struct iphdr) + sizeof(struct udphdr) +
		offsetof(dhcp_t, options))
	{
		origoptlen = optlen = len - sizeof(struct iphdr) +
					 sizeof(struct udphdr) + offsetof(dhcp_t, options);
		while (optlen > sizeof(dhcp_option_t))
		{
			option = (dhcp_option_t*)&packet.dhcp.options[optpos];
			optsize = sizeof(dhcp_option_t) + option->len;
			if (option->type == DHCP_OPTEND || optlen < optsize)
			{
				break;
			}
			if (option->type == DHCP_MESSAGE_TYPE && option->len == 1)
			{
				switch (option->data[0])
				{
					case DHCP_OFFER:
						handle_offer(this, &packet.dhcp, origoptlen);
						break;
					case DHCP_ACK:
						handle_ack(this, &packet.dhcp, origoptlen);
					default:
						break;
				}
				break;
			}
			optlen -= optsize;
			optpos += optsize;
		}
	}
	return TRUE;
}

METHOD(dhcp_socket_t, destroy, void,
	private_dhcp_socket_t *this)
{
	while (this->waiting)
	{
		this->condvar->signal(this->condvar);
	}
	if (this->send > 0)
	{
		close(this->send);
	}
	if (this->receive > 0)
	{
		lib->watcher->remove(lib->watcher, this->receive);
		close(this->receive);
	}
	this->mutex->destroy(this->mutex);
	this->condvar->destroy(this->condvar);
	this->discover->destroy_offset(this->discover,
								offsetof(dhcp_transaction_t, destroy));
	this->request->destroy_offset(this->request,
								offsetof(dhcp_transaction_t, destroy));
	this->completed->destroy_offset(this->completed,
								offsetof(dhcp_transaction_t, destroy));
	DESTROY_IF(this->rng);
	DESTROY_IF(this->dst);
	free(this);
}

/**
 * Bind a socket to a particular interface name
 */
static bool bind_to_device(int fd, char *iface)
{
	struct ifreq ifreq;

	if (strlen(iface) > sizeof(ifreq.ifr_name))
	{
		DBG1(DBG_CFG, "name for DHCP interface too long: '%s'", iface);
		return FALSE;
	}
	memcpy(ifreq.ifr_name, iface, sizeof(ifreq.ifr_name));
	if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifreq, sizeof(ifreq)))
	{
		DBG1(DBG_CFG, "binding DHCP socket to '%s' failed: %s",
			 iface, strerror(errno));
		return FALSE;
	}
	return TRUE;
}

/**
 * See header
 */
dhcp_socket_t *dhcp_socket_create()
{
	private_dhcp_socket_t *this;
	struct sockaddr_in src = {
		.sin_family = AF_INET,
		.sin_port = htons(DHCP_CLIENT_PORT),
		.sin_addr = {
			.s_addr = INADDR_ANY,
		},
	};
	char *iface;
	int on = 1, rcvbuf = 0;
	struct sock_filter dhcp_filter_code[] = {
		BPF_STMT(BPF_LD+BPF_B+BPF_ABS,
				 offsetof(struct iphdr, protocol)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, IPPROTO_UDP, 0, 16),
		BPF_STMT(BPF_LD+BPF_H+BPF_ABS, sizeof(struct iphdr) +
				 offsetof(struct udphdr, source)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, DHCP_SERVER_PORT, 0, 14),
		BPF_STMT(BPF_LD+BPF_H+BPF_ABS, sizeof(struct iphdr) +
				 offsetof(struct udphdr, dest)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, DHCP_CLIENT_PORT, 2, 0),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, DHCP_SERVER_PORT, 1, 0),
		BPF_JUMP(BPF_JMP+BPF_JA, 10, 0, 0),
		BPF_STMT(BPF_LD+BPF_B+BPF_ABS, sizeof(struct iphdr) +
				 sizeof(struct udphdr) + offsetof(dhcp_t, opcode)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, BOOTREPLY, 0, 8),
		BPF_STMT(BPF_LD+BPF_B+BPF_ABS, sizeof(struct iphdr) +
				 sizeof(struct udphdr) + offsetof(dhcp_t, hw_type)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, ARPHRD_ETHER, 0, 6),
		BPF_STMT(BPF_LD+BPF_B+BPF_ABS, sizeof(struct iphdr) +
				 sizeof(struct udphdr) + offsetof(dhcp_t, hw_addr_len)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 6, 0, 4),
		BPF_STMT(BPF_LD+BPF_W+BPF_ABS, sizeof(struct iphdr) +
				 sizeof(struct udphdr) + offsetof(dhcp_t, magic_cookie)),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 0x63825363, 0, 2),
		BPF_STMT(BPF_LD+BPF_W+BPF_LEN, 0),
		BPF_STMT(BPF_RET+BPF_A, 0),
		BPF_STMT(BPF_RET+BPF_K, 0),
	};
	struct sock_fprog dhcp_filter = {
		sizeof(dhcp_filter_code) / sizeof(struct sock_filter),
		dhcp_filter_code,
	};

	INIT(this,
		.public = {
			.enroll = _enroll,
			.release = _release,
			.destroy = _destroy,
		},
		.rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.condvar = condvar_create(CONDVAR_TYPE_DEFAULT),
		.discover = linked_list_create(),
		.request = linked_list_create(),
		.completed = linked_list_create(),
	);

	if (!this->rng)
	{
		DBG1(DBG_CFG, "unable to create RNG");
		destroy(this);
		return NULL;
	}
	this->identity_lease = lib->settings->get_bool(lib->settings,
								"%s.plugins.dhcp.identity_lease", FALSE,
								lib->ns);
	this->force_dst = lib->settings->get_str(lib->settings,
								"%s.plugins.dhcp.force_server_address", FALSE,
								lib->ns);
	this->dst = host_create_from_string(lib->settings->get_str(lib->settings,
								"%s.plugins.dhcp.server", "255.255.255.255",
								lib->ns), DHCP_SERVER_PORT);
	iface = lib->settings->get_str(lib->settings, "%s.plugins.dhcp.interface",
								   NULL, lib->ns);
	if (!this->dst)
	{
		DBG1(DBG_CFG, "configured DHCP server address invalid");
		destroy(this);
		return NULL;
	}

	this->send = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (this->send == -1)
	{
		DBG1(DBG_CFG, "unable to create DHCP send socket: %s", strerror(errno));
		destroy(this);
		return NULL;
	}
	if (setsockopt(this->send, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
	{
		DBG1(DBG_CFG, "unable to reuse DHCP socket address: %s", strerror(errno));
		destroy(this);
		return NULL;
	}
	if (setsockopt(this->send, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) == -1)
	{
		DBG1(DBG_CFG, "unable to broadcast on DHCP socket: %s", strerror(errno));
		destroy(this);
		return NULL;
	}
	/* we won't read any data from this socket, so reduce the buffer to save
	 * some memory (there is some minimum, still try 0, though).
	 * note that we might steal some packets from other processes if e.g. a DHCP
	 * client (or server) is running on the same host, but by reducing the
	 * buffer size the impact should be minimized */
	if (setsockopt(this->send, SOL_SOCKET, SO_RCVBUF, &rcvbuf,
				   sizeof(rcvbuf)) == -1)
	{
		DBG1(DBG_CFG, "unable to reduce receive buffer on DHCP send socket: %s",
			 strerror(errno));
		destroy(this);
		return NULL;
	}
	if (!is_broadcast(this->dst) &&
		lib->settings->get_bool(lib->settings,
								"%s.plugins.dhcp.use_server_port", FALSE,
								lib->ns))
	{
		/* when setting giaddr (which we do when we don't broadcast), the server
		 * should respond to the server port on that IP, according to RFC 2131,
		 * section 4.1.  while we do receive such messages via raw socket, the
		 * kernel will respond with an ICMP port unreachable if there is no
		 * socket bound to that port, which might be problematic with certain
		 * DHCP servers.  instead of opening an additional socket, that we don't
		 * actually use, we can also just send our requests from port 67.
		 * we don't do this by default, as it might cause conflicts with DHCP
		 * servers running on the same host */
		src.sin_port = htons(DHCP_SERVER_PORT);
	}
	if (bind(this->send, (struct sockaddr*)&src, sizeof(src)) == -1)
	{
		DBG1(DBG_CFG, "unable to bind DHCP send socket: %s", strerror(errno));
		destroy(this);
		return NULL;
	}

	this->receive = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
	if (this->receive == -1)
	{
		DBG1(DBG_NET, "opening DHCP receive socket failed: %s", strerror(errno));
		destroy(this);
		return NULL;
	}
	if (setsockopt(this->receive, SOL_SOCKET, SO_ATTACH_FILTER,
				   &dhcp_filter, sizeof(dhcp_filter)) < 0)
	{
		DBG1(DBG_CFG, "installing DHCP socket filter failed: %s",
			 strerror(errno));
		destroy(this);
		return NULL;
	}
	if (iface)
	{
		if (!bind_to_device(this->send, iface) ||
			!bind_to_device(this->receive, iface))
		{
			destroy(this);
			return NULL;
		}
	}

	lib->watcher->add(lib->watcher, this->receive, WATCHER_READ,
					  (watcher_cb_t)receive_dhcp, this);

	return &this->public;
}
