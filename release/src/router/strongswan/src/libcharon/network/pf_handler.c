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

#include "pf_handler.h"

#include <library.h>
#include <unistd.h>
#include <errno.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#if !defined(__APPLE__) && !defined(__FreeBSD__)
#include <net/if_arp.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <linux/filter.h>
#else
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/bpf.h>
#include <net/if_dl.h>
#endif /* !defined(__APPLE__) && !defined(__FreeBSD__) */

#if !defined(__APPLE__) && !defined(__FreeBSD__)

/**
 * Number of interfaces to cache in LFU cache
 */
#define IFACE_CACHE_SIZE	8

/**
 * Data for a cached interface on which packets were received
 */
struct cached_iface_t {

	/**
	 * Index of the interface
	 */
	int if_index;

	/**
	 * Name of the interface
	 */
	char if_name[IFNAMSIZ];

	/**
	 * Hardware (mac) address of the interface
	 */
	u_char hwaddr[ETHER_ADDR_LEN];

	/**
	 * Number of the times this info has been used, for LFU cache
	 */
	int used;
};

typedef struct cached_iface_t cached_iface_t;

#endif

typedef struct private_pf_handler_t private_pf_handler_t;

struct private_pf_handler_t {

	/**
	 * Public interface
	 */
	pf_handler_t public;

	/**
	 * Name for this handler
	 */
	const char *name;

	/**
	 * Registered callback
	 */
	pf_packet_handler_t handler;

	/**
	 * Context to pass to callback
	 */
	void *ctx;

#if !defined(__APPLE__) && !defined(__FreeBSD__)

	/**
	 * AF_PACKET receive socket
	 */
	int receive;

	/**
	 * Cache of frequently used interface information
	 */
	cached_iface_t ifaces[IFACE_CACHE_SIZE];

	/**
	 * Number of currently cached interface information
	 */
	int cached;

#else

	/**
	 * BPF sockets (one per interface), pf_socket_t
	 */
	linked_list_t *pf_sockets;

#endif /* !defined(__APPLE__) && !defined(__FreeBSD__) */
};

#if !defined(__APPLE__) && !defined(__FreeBSD__)

/**
 * Find the index of the slot that was least frequently used
 */
static int find_least_used_cache_entry(private_pf_handler_t *this)
{
	int i, idx = 0, least_used = 0;

	if (this->cached < IFACE_CACHE_SIZE)
	{
		/* not all slots used, choose the next unused slot */
		idx = this->cached++;
	}
	else
	{
		/* all slots in use, choose the one with the lowest usage */
		for (i = 0; i < this->cached; i++)
		{
			if (this->ifaces[i].used < least_used)
			{
				idx = i;
				least_used = this->ifaces[i].used;
			}
		}
	}
	return idx;
}

/**
 * Retrieve information about the interface on which a packet was received
 */
static cached_iface_t *find_interface(private_pf_handler_t *this, int fd,
									  int ifindex)
{
	struct ifreq req = {
		.ifr_ifindex = ifindex,
	};
	int idx;

	for (idx = 0; idx < this->cached; idx++)
	{
		if (this->ifaces[idx].if_index == ifindex)
		{
			this->ifaces[idx].used++;
			return &this->ifaces[idx];
		}
	}

	if (ioctl(fd, SIOCGIFNAME, &req) == 0 &&
		ioctl(fd, SIOCGIFHWADDR, &req) == 0 &&
		(req.ifr_hwaddr.sa_family == ARPHRD_ETHER ||
		 req.ifr_hwaddr.sa_family == ARPHRD_LOOPBACK))
	{
		idx = find_least_used_cache_entry(this);

		this->ifaces[idx].if_index = ifindex;
		memcpy(this->ifaces[idx].if_name, req.ifr_name, IFNAMSIZ);
		memcpy(this->ifaces[idx].hwaddr, req.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
		this->ifaces[idx].used = 1;
		return &this->ifaces[idx];
	}
	return NULL;
}

CALLBACK(receive_packet, bool,
	private_pf_handler_t *this, int fd, watcher_event_t event)
{
	cached_iface_t *iface;
	struct sockaddr_ll addr;
	socklen_t addr_len = sizeof(addr);
	uint8_t packet[1500];
	ssize_t len;

	len = recvfrom(fd, &packet, sizeof(packet), MSG_DONTWAIT,
				   (struct sockaddr*)&addr, &addr_len);

	if (len >= 0)
	{
		iface = find_interface(this, fd, addr.sll_ifindex);
		if (iface)
		{
			this->handler(this->ctx, iface->if_name, iface->if_index,
						  chunk_create(iface->hwaddr, ETHER_ADDR_LEN), fd,
						  chunk_create(packet, len));
		}
	}
	return TRUE;
}

METHOD(pf_handler_t, destroy, void,
	private_pf_handler_t *this)
{
	if (this->receive >= 0)
	{
		lib->watcher->remove(lib->watcher, this->receive);
		close(this->receive);
	}
	free(this);
}

/**
 * Bind the given packet socket to the a named device
 */
static bool bind_packet_socket_to_device(int fd, char *iface)
{
	struct sockaddr_ll addr = {
		.sll_family = AF_PACKET,
		.sll_ifindex = if_nametoindex(iface),
	};

	if (!addr.sll_ifindex)
	{
		DBG1(DBG_CFG, "unable to bind socket to '%s': not found", iface);
		return FALSE;
	}
	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		DBG1(DBG_CFG, "binding socket to '%s' failed: %s",
			 iface, strerror(errno));
		return FALSE;
	}
	return TRUE;
}

/**
 * Setup capturing via AF_PACKET socket
 */
static bool setup_internal(private_pf_handler_t *this, char *iface,
						   struct sock_fprog *packet_filter)
{
	int protocol = strcmp(this->name, "ARP") ? ETH_P_IP : ETH_P_ARP;

	this->receive = socket(AF_PACKET, SOCK_DGRAM, htons(protocol));
	if (this->receive == -1)
	{
		DBG1(DBG_NET, "opening %s packet socket failed: %s", this->name,
			 strerror(errno));
		return FALSE;
	}
	if (setsockopt(this->receive, SOL_SOCKET, SO_ATTACH_FILTER, packet_filter,
				   sizeof(struct sock_fprog)) < 0)
	{
		DBG1(DBG_NET, "installing %s packet socket filter failed: %s",
			 this->name, strerror(errno));
		return FALSE;
	}
	if (iface && iface[0] && !bind_packet_socket_to_device(this->receive, iface))
	{
		return FALSE;
	}
	lib->watcher->add(lib->watcher, this->receive, WATCHER_READ,
					  receive_packet, this);
	DBG2(DBG_NET, "listening for %s (protocol=0x%04x) requests on fd=%d bound "
		 "to %s", this->name, protocol, this->receive,
		 iface && iface[0] ? iface : "no interface");
	return TRUE;
}

/*
 * Described in header
 */
bool bind_to_device(int fd, char *iface)
{
	int status;
	struct ifreq ifreq = {};

	if (strlen(iface) > sizeof(ifreq.ifr_name))
	{
		DBG1(DBG_CFG, "name for interface too long: '%s'", iface);
		return FALSE;
	}
	memcpy(ifreq.ifr_name, iface, sizeof(ifreq.ifr_name));
	status = setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifreq, sizeof(ifreq));
	if (status)
	{
		DBG1(DBG_CFG, "binding socket to '%s' failed: %s",
			 iface, strerror(errno));
		return FALSE;
	}
	return TRUE;
}

#else /* !defined(__APPLE__) && !defined(__FreeBSD__) */

/**
 * A BPF socket is required for each interface.
 */
struct pf_socket_t {

	/**
	 * Reference to the private packet filter handler
	 */
	private_pf_handler_t *this;

	/**
	 * The name of the interface
	 */
	char *if_name;

	/**
	 * Index of the interface
	 */
	int if_index;

	/**
	 * The Ethernet MAC address of the interface
	 */
	chunk_t mac;

	/**
	 * The IPv4 address of the interface
	 */
	host_t *ipv4;

	/**
	 * The BPF file descriptor for this interface
	 */
	int fd;

	/**
	 * The BPF packet buffer length as read from the BPF fd
	 */
	size_t buflen;

	/**
	 * An allocated buffer for receiving packets from BPF
	 */
	uint8_t *bufdat;
};

typedef struct pf_socket_t pf_socket_t;

/**
 * Free resources used by a socket.
 */
CALLBACK(destroy_pf_socket, void,
	pf_socket_t *socket)
{
	if (socket->fd >= 0)
	{
		lib->watcher->remove(lib->watcher, socket->fd);
		close(socket->fd);
	}
	DESTROY_IF(socket->ipv4);
	chunk_free(&socket->mac);
	free(socket->bufdat);
	free(socket->if_name);
	free(socket);
}

/**
 * Find the handler for the named interface, creating one if needed.
 */
static pf_socket_t *get_pf_socket(private_pf_handler_t *this, char *if_name)
{
	pf_socket_t *socket, *found = NULL;
	enumerator_t *enumerator;

	enumerator = this->pf_sockets->create_enumerator(this->pf_sockets);
	while (enumerator->enumerate(enumerator, &socket))
	{
		if (streq(socket->if_name, if_name))
		{
			found = socket;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (!found)
	{
		INIT(found,
			 .this = this,
			 .if_name = strdup(if_name),
			 .fd = -1,
		);
		this->pf_sockets->insert_last(this->pf_sockets, found);
	}
	return found;
}

/**
 * Find and open an available BPF device.
 */
static int bpf_open()
{
	static int no_cloning_bpf = 0;
	/* enough space for: /dev/bpf000\0 */
	char device[12];
	int n = no_cloning_bpf ? 0 : -1;
	int fd;

	do
	{
		if (n < 0)
		{
			snprintf(device, sizeof(device), "/dev/bpf");
		}
		else
		{
			snprintf(device, sizeof(device), "/dev/bpf%d", n);
		}

		fd = open(device, O_RDWR);

		if (n++ < 0 && fd < 0 && errno == ENOENT)
		{
			no_cloning_bpf = 1;
			errno = EBUSY;
		}
	}
	while (fd < 0 && errno == EBUSY && n < 1000);

	return fd;
}

CALLBACK(handler_onpkt, bool,
	pf_socket_t *socket, int fd, watcher_event_t event)
{
	struct bpf_hdr *bh;
	void *a;
	uint8_t *p = socket->bufdat;
	ssize_t n;
	size_t pktlen;

	n = read(socket->fd, socket->bufdat, socket->buflen);
	if (n <= 0)
	{
		DBG1(DBG_NET, "reading %s request from %s failed: %s",
			 socket->this->name, socket->if_name, strerror(errno));
		return FALSE;
	}

	while (p < socket->bufdat + n)
	{
		bh = (struct bpf_hdr*) p;
		a = (void*)(p + bh->bh_hdrlen + sizeof(struct ether_header));
		pktlen = bh->bh_caplen - sizeof(struct ether_header);

		socket->this->handler(socket->this->ctx, socket->if_name,
							  socket->if_index, socket->mac,
							  socket->fd, chunk_create(a, pktlen));

		p += BPF_WORDALIGN(bh->bh_hdrlen + bh->bh_caplen);
	}
	return TRUE;
}

/**
 * Create and initialize a BPF socket for the interface specified in the given
 * struct. This entails opening a BPF device, binding it to the interface,
 * setting the packet filter, and allocating a buffer for receiving packets.
 */
static bool setup_pf_socket(pf_socket_t *socket, pf_program_t *program)
{
	struct ifreq req;
	uint32_t disable = 1;
	uint32_t enable = 1;
	uint32_t dlt = 0;

	snprintf(req.ifr_name, sizeof(req.ifr_name), "%s", socket->if_name);

	if ((socket->fd = bpf_open()) < 0)
	{
		DBG1(DBG_NET, "bpf_open(%s): %s", socket->if_name, strerror(errno));
		return FALSE;
	}

	if (ioctl(socket->fd, BIOCSETIF, &req) < 0)
	{
		DBG1(DBG_NET, "BIOCSETIF(%s): %s", socket->if_name, strerror(errno));
		return FALSE;
	}

	if (ioctl(socket->fd, BIOCSHDRCMPLT, &enable) < 0)
	{
		DBG1(DBG_NET, "BIOCSHDRCMPLT(%s): %s", socket->if_name, strerror(errno));
		return FALSE;
	}

	if (ioctl(socket->fd, BIOCSSEESENT, &disable) < 0)
	{
		DBG1(DBG_NET, "BIOCSSEESENT(%s): %s", socket->if_name, strerror(errno));
		return FALSE;
	}

	if (ioctl(socket->fd, BIOCIMMEDIATE, &enable) < 0)
	{
		DBG1(DBG_NET, "BIOCIMMEDIATE(%s): %s", socket->if_name, strerror(errno));
		return FALSE;
	}

	if (ioctl(socket->fd, BIOCGDLT, &dlt) < 0)
	{
		DBG1(DBG_NET, "BIOCGDLT(%s): %s", socket->if_name, strerror(errno));
		return FALSE;
	}
	else if (dlt != DLT_EN10MB)
	{
		return FALSE;
	}

	if (ioctl(socket->fd, BIOCSETF, program) < 0)
	{
		DBG1(DBG_NET, "BIOCSETF(%s): %s", socket->if_name, strerror(errno));
		return FALSE;
	}

	if (ioctl(socket->fd, BIOCGBLEN, &socket->buflen) < 0)
	{
		DBG1(DBG_NET, "BIOCGBLEN(%s): %s", socket->if_name, strerror(errno));
		return FALSE;
	}
	socket->bufdat = malloc(socket->buflen);

	lib->watcher->add(lib->watcher, socket->fd, WATCHER_READ,
					  handler_onpkt, socket);
	return TRUE;
}

/**
 * Create a socket for each BPF capable interface.  The interface must have an
 * Ethernet MAC address, an IPv4 address, and use an Ethernet data link layer.
 */
static bool setup_internal(private_pf_handler_t *this, char *iface,
						   pf_program_t *program)
{
	struct ifaddrs *ifas;
	struct ifaddrs *ifa;
	struct sockaddr_dl *dl;
	pf_socket_t *socket;
	enumerator_t *enumerator;
	host_t *ipv4;

	if (getifaddrs(&ifas) < 0)
	{
		DBG1(DBG_NET, "%s cannot find interfaces: %s", this->name, strerror(errno));
		return FALSE;
	}
	this->pf_sockets = linked_list_create();
	for (ifa = ifas; ifa != NULL; ifa = ifa->ifa_next)
	{
		switch (ifa->ifa_addr->sa_family)
		{
			case AF_LINK:
				dl = (struct sockaddr_dl *)ifa->ifa_addr;
				if (dl->sdl_alen == ETHER_ADDR_LEN)
				{
					socket = get_pf_socket(this, ifa->ifa_name);
					socket->if_index = dl->sdl_index;
					socket->mac = chunk_clone(chunk_create(LLADDR(dl),
															dl->sdl_alen));
				}
				break;
			case AF_INET:
				ipv4 = host_create_from_sockaddr(ifa->ifa_addr);
				if (ipv4 && !ipv4->is_anyaddr(ipv4))
				{
					socket = get_pf_socket(this, ifa->ifa_name);
					if (!socket->ipv4)
					{
						socket->ipv4 = ipv4->clone(ipv4);
					}
				}
				DESTROY_IF(ipv4);
				break;
			default:
				break;
		}
	}
	freeifaddrs(ifas);

	enumerator = this->pf_sockets->create_enumerator(this->pf_sockets);
	while (enumerator->enumerate(enumerator, &socket))
	{
		if (socket->mac.ptr && socket->ipv4 &&
			(!iface || streq(socket->if_name, iface)) &&
			setup_pf_socket(socket, program))
		{
			DBG2(DBG_NET, "listening for %s requests on %s (%H, %#B)",
				 this->name, socket->if_name, socket->ipv4, &socket->mac);
		}
		else
		{
			this->pf_sockets->remove_at(this->pf_sockets, enumerator);
			destroy_pf_socket(socket);
		}
	}
	enumerator->destroy(enumerator);

	return this->pf_sockets->get_count(this->pf_sockets) > 0;
}

METHOD(pf_handler_t, destroy, void,
	private_pf_handler_t *this)
{
	DESTROY_FUNCTION_IF(this->pf_sockets, destroy_pf_socket);
	free(this);
}

/*
 * Described in header
 */
bool bind_to_device(int fd, char *iface)
{
#if defined(__FreeBSD__)
	DBG1(DBG_CFG, "binding socket to '%s' failed: IP_SENDIF not implemented yet.", iface);
	return FALSE;
#else /* defined(__FreeBSD__) */
	unsigned int idx = if_nametoindex(iface);
	if (setsockopt(fd, IPPROTO_IP, IP_BOUND_IF, &idx, sizeof(idx)) == -1)
	{
		DBG1(DBG_CFG, "binding socket to '%s' failed: %s",
			 iface, strerror(errno));
		return FALSE;
	}
	return TRUE;
#endif /* defined(__FreeBSD__) */
}

#endif /* !defined(__APPLE__) && !defined(__FreeBSD__) */

/*
 * Described in header
 */
pf_handler_t *pf_handler_create(const char *name, char *iface,
								pf_packet_handler_t handler, void *ctx,
								pf_program_t *program)
{
	private_pf_handler_t *this;

	INIT(this,
		 .public = {
			.destroy = _destroy,
		 },
		 .name = name,
		 .handler = handler,
		 .ctx = ctx,
	);

	if (!setup_internal(this, iface, program))
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}
