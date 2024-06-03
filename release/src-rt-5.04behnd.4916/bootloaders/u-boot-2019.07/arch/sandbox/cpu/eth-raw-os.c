// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015-2018 National Instruments
 * Copyright (c) 2015-2018 Joe Hershberger <joe.hershberger@ni.com>
 */

#include <asm/eth-raw-os.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

#include <os.h>

struct sandbox_eth_raw_if_nameindex *sandbox_eth_raw_if_nameindex(void)
{
	return (struct sandbox_eth_raw_if_nameindex *)if_nameindex();
}

void sandbox_eth_raw_if_freenameindex(struct sandbox_eth_raw_if_nameindex *ptr)
{
	if_freenameindex((struct if_nameindex *)ptr);
}

int sandbox_eth_raw_os_is_local(const char *ifname)
{
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct ifreq ifr;
	int ret = 0;

	if (fd < 0)
		return -errno;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ret = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if (ret < 0) {
		ret = -errno;
		goto out;
	}
	ret = !!(ifr.ifr_flags & IFF_LOOPBACK);
out:
	close(fd);
	return ret;
}

int sandbox_eth_raw_os_idx_to_name(struct eth_sandbox_raw_priv *priv)
{
	if (!if_indextoname(priv->host_ifindex, priv->host_ifname))
		return -errno;
	return 0;
}

static int _raw_packet_start(struct eth_sandbox_raw_priv *priv,
			     unsigned char *ethmac)
{
	struct sockaddr_ll *device;
	struct packet_mreq mr;
	int ret;
	int flags;

	/* Prepare device struct */
	priv->local_bind_sd = -1;
	priv->device = os_malloc(sizeof(struct sockaddr_ll));
	if (priv->device == NULL)
		return -ENOMEM;
	device = priv->device;
	memset(device, 0, sizeof(struct sockaddr_ll));
	device->sll_ifindex = if_nametoindex(priv->host_ifname);
	priv->host_ifindex = device->sll_ifindex;
	device->sll_family = AF_PACKET;
	memcpy(device->sll_addr, ethmac, 6);
	device->sll_halen = htons(6);

	/* Open socket */
	priv->sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (priv->sd < 0) {
		printf("Failed to open socket: %d %s\n", errno,
		       strerror(errno));
		return -errno;
	}
	/* Bind to the specified interface */
	ret = setsockopt(priv->sd, SOL_SOCKET, SO_BINDTODEVICE,
			 priv->host_ifname, strlen(priv->host_ifname) + 1);
	if (ret < 0) {
		printf("Failed to bind to '%s': %d %s\n", priv->host_ifname,
		       errno, strerror(errno));
		return -errno;
	}

	/* Make the socket non-blocking */
	flags = fcntl(priv->sd, F_GETFL, 0);
	fcntl(priv->sd, F_SETFL, flags | O_NONBLOCK);

	/* Enable promiscuous mode to receive responses meant for us */
	mr.mr_ifindex = device->sll_ifindex;
	mr.mr_type = PACKET_MR_PROMISC;
	ret = setsockopt(priv->sd, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
		   &mr, sizeof(mr));
	if (ret < 0) {
		struct ifreq ifr;

		printf("Failed to set promiscuous mode: %d %s\n"
		       "Falling back to the old \"flags\" way...\n",
			errno, strerror(errno));
		if (strlen(priv->host_ifname) >= IFNAMSIZ) {
			printf("Interface name %s is too long.\n",
			       priv->host_ifname);
			return -EINVAL;
		}
		strncpy(ifr.ifr_name, priv->host_ifname, IFNAMSIZ);
		if (ioctl(priv->sd, SIOCGIFFLAGS, &ifr) < 0) {
			printf("Failed to read flags: %d %s\n", errno,
			       strerror(errno));
			return -errno;
		}
		ifr.ifr_flags |= IFF_PROMISC;
		if (ioctl(priv->sd, SIOCSIFFLAGS, &ifr) < 0) {
			printf("Failed to write flags: %d %s\n", errno,
			       strerror(errno));
			return -errno;
		}
	}
	return 0;
}

static int _local_inet_start(struct eth_sandbox_raw_priv *priv)
{
	struct sockaddr_in *device;
	int ret;
	int flags;
	int one = 1;

	/* Prepare device struct */
	priv->local_bind_sd = -1;
	priv->local_bind_udp_port = 0;
	priv->device = os_malloc(sizeof(struct sockaddr_in));
	if (priv->device == NULL)
		return -ENOMEM;
	device = priv->device;
	memset(device, 0, sizeof(struct sockaddr_in));
	device->sin_family = AF_INET;
	device->sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	/**
	 * Open socket
	 *  Since we specify UDP here, any incoming ICMP packets will
	 *  not be received, so things like ping will not work on this
	 *  localhost interface.
	 */
	priv->sd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (priv->sd < 0) {
		printf("Failed to open socket: %d %s\n", errno,
		       strerror(errno));
		return -errno;
	}

	/* Make the socket non-blocking */
	flags = fcntl(priv->sd, F_GETFL, 0);
	fcntl(priv->sd, F_SETFL, flags | O_NONBLOCK);

	/* Include the UDP/IP headers on send and receive */
	ret = setsockopt(priv->sd, IPPROTO_IP, IP_HDRINCL, &one,
			 sizeof(one));
	if (ret < 0) {
		printf("Failed to set header include option: %d %s\n", errno,
		       strerror(errno));
		return -errno;
	}
	return 0;
}

int sandbox_eth_raw_os_start(struct eth_sandbox_raw_priv *priv,
			     unsigned char *ethmac)
{
	if (priv->local)
		return _local_inet_start(priv);
	else
		return _raw_packet_start(priv, ethmac);
}

int sandbox_eth_raw_os_send(void *packet, int length,
			    struct eth_sandbox_raw_priv *priv)
{
	int retval;
	struct udphdr *udph = packet + sizeof(struct iphdr);

	if (priv->sd < 0 || !priv->device)
		return -EINVAL;

	/*
	 * This block of code came about when testing tftp on the localhost
	 * interface. When using the RAW AF_INET API, the network stack is still
	 * in play responding to incoming traffic based on open "ports". Since
	 * it is raw (at the IP layer, no Ethernet) the network stack tells the
	 * TFTP server that the port it responded to is closed. This causes the
	 * TFTP transfer to be aborted. This block of code inspects the outgoing
	 * packet as formulated by the u-boot network stack to determine the
	 * source port (that the TFTP server will send packets back to) and
	 * opens a typical UDP socket on that port, thus preventing the network
	 * stack from sending that ICMP message claiming that the port has no
	 * bound socket.
	 */
	if (priv->local && (priv->local_bind_sd == -1 ||
			    priv->local_bind_udp_port != udph->source)) {
		struct iphdr *iph = packet;
		struct sockaddr_in addr;

		if (priv->local_bind_sd != -1)
			close(priv->local_bind_sd);

		/* A normal UDP socket is required to bind */
		priv->local_bind_sd = socket(AF_INET, SOCK_DGRAM, 0);
		if (priv->local_bind_sd < 0) {
			printf("Failed to open bind sd: %d %s\n", errno,
			       strerror(errno));
			return -errno;
		}
		priv->local_bind_udp_port = udph->source;

		/**
		 * Bind the UDP port that we intend to use as our source port
		 * so that the kernel will not send an ICMP port unreachable
		 * message to the server
		 */
		addr.sin_family = AF_INET;
		addr.sin_port = udph->source;
		addr.sin_addr.s_addr = iph->saddr;
		retval = bind(priv->local_bind_sd, (struct sockaddr *)&addr,
			      sizeof(addr));
		if (retval < 0)
			printf("Failed to bind: %d %s\n", errno,
			       strerror(errno));
	}

	retval = sendto(priv->sd, packet, length, 0,
			(struct sockaddr *)priv->device,
			sizeof(struct sockaddr_ll));
	if (retval < 0) {
		printf("Failed to send packet: %d %s\n", errno,
		       strerror(errno));
		return -errno;
	}
	return retval;
}

int sandbox_eth_raw_os_recv(void *packet, int *length,
			    const struct eth_sandbox_raw_priv *priv)
{
	int retval;
	int saddr_size;

	if (priv->sd < 0 || !priv->device)
		return -EINVAL;
	saddr_size = sizeof(struct sockaddr);
	retval = recvfrom(priv->sd, packet, 1536, 0,
			  (struct sockaddr *)priv->device,
			  (socklen_t *)&saddr_size);
	*length = 0;
	if (retval >= 0) {
		*length = retval;
		return 0;
	}
	/* The socket is non-blocking, so expect EAGAIN when there is no data */
	if (errno == EAGAIN)
		return 0;
	return -errno;
}

void sandbox_eth_raw_os_stop(struct eth_sandbox_raw_priv *priv)
{
	os_free(priv->device);
	priv->device = NULL;
	close(priv->sd);
	priv->sd = -1;
	if (priv->local) {
		if (priv->local_bind_sd != -1)
			close(priv->local_bind_sd);
		priv->local_bind_sd = -1;
		priv->local_bind_udp_port = 0;
	}
}
