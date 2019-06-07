/* $Id: $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2018 Pali Rohár
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifndef TEST_LINUX_DEBUG_APP
#include "config.h"
#endif

#include "upnpstun.h"

#if defined(USE_NETFILTER)
#include "netfilter/iptcrdr.h"
#endif
#if defined(USE_PF)
#include "pf/obsdrdr.h"
#endif
#if defined(USE_IPF)
#include "ipf/ipfrdr.h"
#endif
#if defined(USE_IPFW)
#include "ipfw/ipfwrdr.h"
#endif

#ifdef TEST_LINUX_DEBUG_APP
static int add_filter_rule2(const char *ifname, const char *rhost, const char *iaddr, unsigned short eport, unsigned short iport, int proto, const char *desc);
static int delete_filter_rule(const char * ifname, unsigned short port, int proto);
#endif

/* Generate random STUN Transaction Id */
static void generate_transaction_id(unsigned char transaction_id[12])
{
	size_t i;

	for (i = 0; i < 12; i++)
		transaction_id[i] = random() & 255;
}

/* Create and fill STUN Binding Request */
static void fill_request(unsigned char buffer[28], int change_ip, int change_port)
{
	/* Type: Binding Request */
	buffer[0] = 0x00;
	buffer[1] = 0x01;

	/* Length: One 8-byte attribute */
	buffer[2] = 0x00;
	buffer[3] = 0x08;

	/* Magic Cookie: 0x2120A442 */
	buffer[4] = 0x21;
	buffer[5] = 0x12;
	buffer[6] = 0xA4;
	buffer[7] = 0x42;

	/* Transaction Id */
	generate_transaction_id(buffer+8);

	/* Attribute Type: Change Request */
	buffer[20] = 0x00;
	buffer[21] = 0x03;

	/* Attribute Length: 4 bytes */
	buffer[22] = 0x00;
	buffer[23] = 0x04;

	buffer[24] = 0x00;
	buffer[25] = 0x00;
	buffer[26] = 0x00;
	buffer[27] = 0x00;

	/* Change IP */
	buffer[27] |= change_ip ? 0x4 : 0x00;

	/* Change Port */
	buffer[27] |= change_port ? 0x2 : 0x00;
}

/* Resolve STUN host+port and return sockaddr_in structure */
/* When port is 0 then use default STUN port */
static int resolve_stun_host(const char *stun_host, unsigned short stun_port, struct sockaddr_in *sock_addr)
{
	int have_sock;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	char service[6];

	if (stun_port == 0)
		stun_port = (unsigned short)3478;
	snprintf(service, sizeof(service), "%hu", stun_port);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_NUMERICSERV;

	if (getaddrinfo(stun_host, service, &hints, &result) != 0) {
		errno = EHOSTUNREACH;
		return -1;
	}

	have_sock = 0;
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		if (rp->ai_addrlen > sizeof(*sock_addr) || rp->ai_addr->sa_family != AF_INET)
			continue;
		memcpy(sock_addr, rp->ai_addr, rp->ai_addrlen);
		have_sock = 1;
		break;
	}

	freeaddrinfo(result);

	if (!have_sock) {
		errno = EHOSTUNREACH;
		return -1;
	}

	return 0;
}

/* Create a new UDP socket for STUN connection and return file descriptor and local UDP port */
static int stun_socket(unsigned short *local_port)
{
	int fd;
	socklen_t addr_len;
	struct sockaddr_in local_addr;

	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0)
		return -1;

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = 0;

	if (bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) != 0) {
		close(fd);
		return -1;
	}

	addr_len = sizeof(local_addr);
	if (getsockname(fd, (struct sockaddr *)&local_addr, &addr_len) != 0) {
		close(fd);
		return -1;
	}

	*local_port = ntohs(local_addr.sin_port);

	return fd;
}

/* Receive STUN response message for specified Transaction Id and returns message and peer address */
static size_t receive_stun_response(int fd, unsigned char *buffer, unsigned char transaction_id[12], size_t buffer_len, struct sockaddr_in *peer_addr)
{
	ssize_t len;
	socklen_t peer_addr_len = sizeof(*peer_addr);

	len = recvfrom(fd, buffer, buffer_len, 0, (struct sockaddr *)peer_addr, &peer_addr_len);
	if (len < 20 || peer_addr_len != sizeof(*peer_addr))
		return 0;

	/* Check that buffer is STUN message with class Response and Binding method with transaction id */
	if ((buffer[0] & 0xFF) != 0x01 || (buffer[1] & 0xEF) != 0x01 || memcmp(buffer+8, transaction_id, 12) != 0)
		return 0;

	return len;
}

/* Wait for STUN response messages and try to receive them */
static int wait_for_stun_responses(int fds[4], unsigned char *transaction_ids[4], unsigned char *buffers[4], size_t buffers_lens[4], struct sockaddr_in peer_addrs[4], size_t lens[4])
{
	fd_set fdset;
	struct timeval timeout;
	int max_fd;
	int ret;
	int i;

	max_fd = fds[0];
	for (i = 1; i < 4; i++) {
		if (fds[i] > max_fd)
			max_fd = fds[i];
	}

	timeout.tv_sec = 3;
	timeout.tv_usec = 0;

	while (timeout.tv_sec > 0 || timeout.tv_usec > 0) {

		FD_ZERO(&fdset);
		for (i = 0; i < 4; i++) {
			FD_SET(fds[i], &fdset);
		}

		ret = select(max_fd+1, &fdset, NULL, NULL, &timeout);
		if (ret < 0)
			return -1;
		if (ret == 0)
			return 0;

		for (i = 0; i < 4; ++i)
			if (FD_ISSET(fds[i], &fdset))
				lens[i] = receive_stun_response(fds[i], buffers[i], transaction_ids[i], buffers_lens[i], &peer_addrs[i]);

		if (lens[0] && lens[1] && lens[2] && lens[3])
			return 0;
	}

	return 0;
}

/* Parse Mapped Address (with port) from STUN response message */
static int parse_stun_response(unsigned char *buffer, size_t len, struct sockaddr_in *mapped_addr)
{
	unsigned char *ptr, *end;
	uint16_t attr_type;
	uint16_t attr_len;
	int have_address;

	if (len < 20)
		return -1;

	/* Check that buffer is STUN message with class Success Response and Binding method */
	if (buffer[0] != 0x01 || buffer[1] != 0x01)
		return -1;

	/* Check that STUN message is not longer as buffer length */
	if (((size_t)buffer[2] << 8) + buffer[3] + 20 > len)
		return -1;

	ptr = buffer + 20;
	end = buffer + len;
	have_address = 0;

	while (ptr + 4 <= end) {

		attr_type = ((uint16_t)ptr[0] << 8) + ptr[1];
		attr_len = ((uint16_t)ptr[2] << 8) + ptr[3];
		ptr += 4;

		if (ptr + attr_len > end)
			break;

		if (attr_type == 0x0001 || attr_type == 0x8020) {
			/* Mapped Address or XOR Mapped Address */
			if (attr_len == 8 && ptr[1] == 1) {
				/* IPv4 address */
				if (attr_type == 0x8020) {
					/* Restore XOR Mapped Address */
					ptr[2] ^= buffer[4];
					ptr[3] ^= buffer[5];
					ptr[4] ^= buffer[4];
					ptr[5] ^= buffer[5];
					ptr[6] ^= buffer[6];
					ptr[7] ^= buffer[7];
				}

				mapped_addr->sin_family = AF_INET;
				mapped_addr->sin_port = htons(((uint16_t)ptr[2] << 8) + ptr[3]);
				mapped_addr->sin_addr.s_addr = htonl(((uint32_t)ptr[4] << 24) + (ptr[5] << 16) + (ptr[6] << 8) + ptr[7]);

				/* Prefer XOR Mapped Address, some NATs change IP addresses in UDP packets */
				if (attr_type == 0x8020)
					return 0;

				have_address = 1;
			}
		}

		ptr += attr_len;
	}

	return have_address ? 0 : -1;
}

/* Perform main STUN operation, return external IP address and check if host is behind restrictive NAT */
/* Restrictive NAT means any NAT which do some filtering and which is not static 1:1, basically NAT which is not usable for port forwarding */
int perform_stun(const char *if_name, const char *if_addr, const char *stun_host, unsigned short stun_port, struct in_addr *ext_addr, int *restrictive_nat)
{
	int fds[4];
	size_t responses_lens[4];
	unsigned char responses_bufs[4][1024];
	unsigned char *responses[4];
	size_t responses_sizes[4];
	unsigned char requests[4][28];
	unsigned char *transaction_ids[4];
	int have_mapped_addrs[4];
	struct sockaddr_in remote_addr, peer_addrs[4], mapped_addrs[4];
	unsigned short local_ports[4];
	int have_ext_addr;
	int i, j;

	if (resolve_stun_host(stun_host, stun_port, &remote_addr) != 0)
		return -1;

	/* Prepare four different STUN requests */
	for (i = 0; i < 4; ++i) {

		responses_lens[i] = 0;
		responses[i] = responses_bufs[i];
		responses_sizes[i] = sizeof(responses_bufs[i]);

		fds[i] = stun_socket(&local_ports[i]);
		if (fds[i] < 0) {
			for (j = 0; j < i; ++j)
				close(fds[j]);
			return -1;
		}

		fill_request(requests[i], i/2, i%2);
		transaction_ids[i] = requests[i]+8;

	}

	/* Unblock local ports */
	for (i = 0; i < 4; ++i)
		add_filter_rule2(if_name, NULL, if_addr, local_ports[i], local_ports[i], IPPROTO_UDP, "stun test");

	/* Send STUN requests and wait for responses */
	for (j = 0; j < 3; ++j) {

		for (i = 0; i < 4; ++i) {
			if (responses_lens[i])
				continue;
			if (sendto(fds[i], requests[i], sizeof(requests[i]), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) != sizeof(requests[i]))
				break;
		}

		if (wait_for_stun_responses(fds, transaction_ids, responses, responses_sizes, peer_addrs, responses_lens) != 0)
			break;

		if (responses_lens[0] && responses_lens[1] && responses_lens[2] && responses_lens[3])
			break;

	}

	/* Remove unblock for local ports */
	for (i = 0; i < 4; ++i) {
		delete_filter_rule(if_name, local_ports[i], IPPROTO_UDP);
		close(fds[i]);
	}

	/* Parse received STUN messages */
	have_ext_addr = 0;
	for (i = 0; i < 4; ++i) {
		if (parse_stun_response(responses[i], responses_lens[i], &mapped_addrs[i]) == 0)
			have_mapped_addrs[i] = 1;
		else
			have_mapped_addrs[i] = 0;
		if (!have_ext_addr && have_mapped_addrs[i]) {
			memcpy(ext_addr, &mapped_addrs[i].sin_addr, sizeof(*ext_addr));
			have_ext_addr = 1;
		}
	}

	/* We have no external address */
	if (!have_ext_addr) {
		errno = ENXIO;
		return -1;
	}

	for (i = 0; i < 4; ++i) {
		if (!have_mapped_addrs[i]) {
			/* We have not received all four responses, therefore NAT or firewall is doing some filtering */
			*restrictive_nat = 1;
			return 0;
		}
	}

	if (memcmp(&remote_addr, &peer_addrs[0], sizeof(peer_addrs[0])) != 0) {
		/* We received STUN response from different address even we did not asked for it, so some strange NAT is active */
		*restrictive_nat = 1;
		return 0;
	}

	for (i = 0; i < 4; ++i) {
		if (ntohs(mapped_addrs[i].sin_port) != local_ports[i] || memcmp(&mapped_addrs[i].sin_addr, ext_addr, sizeof(*ext_addr)) != 0) {
			/* External IP address or port was changed, therefore symmetric NAT is active */
			*restrictive_nat = 1;
			return 0;
		}
	}

	/* Otherwise we are either directly connected or behind unrestricted NAT 1:1 */
	/* There is no filtering, so port forwarding would work fine */
	*restrictive_nat = 0;
	return 0;
}

#ifdef TEST_LINUX_DEBUG_APP

/* This linux test application for debugging purposes can be compiled as: */
/* gcc upnpstun.c -o upnpstun -g3 -W -Wall -O2 -DTEST_LINUX_DEBUG_APP */

#include <arpa/inet.h>
#include <time.h>

static int add_filter_rule2(const char *ifname, const char *rhost, const char *iaddr, unsigned short eport, unsigned short iport, int proto, const char *desc)
{
	char buffer[100];
	ifname = ifname;
	rhost = rhost;
	iaddr = iaddr;
	iport = iport;
	desc = desc;
	snprintf(buffer, sizeof(buffer), "/sbin/iptables -t filter -I INPUT -p %d --dport %hu -j ACCEPT", proto, eport);
	printf("Executing: %s\n", buffer);
	return system(buffer);
}

static int delete_filter_rule(const char * ifname, unsigned short port, int proto)
{
	char buffer[100];
	ifname = ifname;
	snprintf(buffer, sizeof(buffer), "/sbin/iptables -t filter -D INPUT -p %d --dport %hu -j ACCEPT", proto, port);
	printf("Executing: %s\n", buffer);
	return system(buffer);
}

int main(int argc, char *argv[])
{
	struct in_addr ext_addr;
	int restrictive_nat;
	int ret;
	char str[INET_ADDRSTRLEN];

	if (argc != 3 && argc != 2) {
		printf("Usage: %s stun_host [stun_port]\n", argv[0]);
		return 1;
	}

	if (argc == 2)
		argv[2] = "0";

	srandom(time(NULL) * getpid());

	ret = perform_stun(NULL, NULL, argv[1], atoi(argv[2]), &ext_addr, &restrictive_nat);
	if (ret != 0) {
		printf("STUN Failed: %s\n", strerror(errno));
		return 1;
	}

	if (!inet_ntop(AF_INET, &ext_addr, str, INET_ADDRSTRLEN))
		str[0] = 0;

	printf("External IP address: %s\n", str);
	printf("Restrictive NAT: %s\n", restrictive_nat ? "active (port forwarding impossible)" : "not used (ready for port forwarding)");
	return 0;
}

#endif
