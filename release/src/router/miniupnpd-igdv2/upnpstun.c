/* $Id: upnpstun.c,v 1.9 2025/01/12 23:31:19 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2020-2025 Thomas Bernard
 * (c) 2018 Pali Rohár
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

/*! \file upnpstun.c
 * \brief STUN client implementation
 *
 * - https://datatracker.ietf.org/doc/html/rfc3489 (obsolete)
 * - https://datatracker.ietf.org/doc/html/rfc5389
 * - https://datatracker.ietf.org/doc/html/rfc5780 (experimental)
 */
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>

#include "config.h"
#include "upnputils.h"
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

/*! \brief Generate random STUN Transaction Id
 * \param[out] transaction_id */
static void generate_transaction_id(unsigned char transaction_id[12])
{
	size_t i;

	for (i = 0; i < 12; i++)
		transaction_id[i] = random() & 255;
}

/*! \brief Create and fill STUN Binding Request
 * \param[out] buffer
 * \param[in] change_ip
 * \param[in] change_port */
static void fill_request(unsigned char buffer[28], int change_ip, int change_port)
{
	/* Type: Binding Request */
	buffer[0] = 0x00;
	buffer[1] = 0x01;

	/* Length: One 8-byte attribute */
	buffer[2] = 0x00;
	buffer[3] = 0x08;

	/* RFC5389 Magic Cookie: 0x2120A442 */
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
	if (change_ip) buffer[27] |= 0x4;

	/* Change Port */
	if (change_port) buffer[27] |= 0x2;
}

/*! \brief Resolve STUN host+port and return sockaddr_in structure
 * When port is 0 then use default STUN port
 * \param[in] stun_host
 * \param[in] stun_port
 * \param[out] sock_addr
 * \return -1 for error */
static int resolve_stun_host(const char *stun_host, unsigned short stun_port, struct sockaddr_in *sock_addr)
{
	int have_sock;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	char service[6];
	int r;

	if (stun_port == 0)
		stun_port = (unsigned short)3478;
	snprintf(service, sizeof(service), "%hu", stun_port);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_NUMERICSERV;

	r = getaddrinfo(stun_host, service, &hints, &result);
	if (r != 0) {
		syslog(LOG_ERR, "%s: getaddrinfo(%s, %s, ...) failed : %s",
		       "resolve_stun_host", stun_host, service, gai_strerror(r));
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
		syslog(LOG_WARNING, "%s: failed to resolve IPv4 address for %s:%s",
		       "resolve_stun_host", stun_host, service);
		errno = EHOSTUNREACH;
		return -1;
	} else {
		char addr_str[48];
		if (sockaddr_to_string((struct sockaddr *)sock_addr, addr_str, sizeof(addr_str)) > 0) {
			syslog(LOG_DEBUG, "%s: %s:%s => %s",
			       "resolve_stun_host", stun_host, service, addr_str);
		}
	}

	return 0;
}

/*! \brief Create a new UDP socket for STUN connection
 * \param[out] local_port local UDP port
 * \return file descriptor */
static int stun_socket(unsigned short *local_port)
{
	int fd;
	socklen_t addr_len;
	struct sockaddr_in local_addr;

	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		syslog(LOG_ERR, "%s: socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP): %m",
		       "stun_socket");
		return -1;
	}

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = 0;

	if (bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) != 0) {
		syslog(LOG_ERR, "%s: bind(): %m",
		       "stun_socket");
		close(fd);
		return -1;
	}

	addr_len = sizeof(local_addr);
	if (getsockname(fd, (struct sockaddr *)&local_addr, &addr_len) != 0) {
		syslog(LOG_ERR, "%s: getsockname(): %m",
		       "stun_socket");
		close(fd);
		return -1;
	}

	*local_port = ntohs(local_addr.sin_port);

	return fd;
}

/*! \brief Receive STUN response message for specified Transaction Id
 * \param[in] fd socket
 * \param[out] buffer receiving buffer
 * \param[in] transaction_id transaction id to check for
 * \param[in] buffer_len receiving buffer length
 * \param[out] peer_addr peer address
 * \return message size or 0 for error */
static size_t receive_stun_response(int fd, unsigned char *buffer, unsigned char transaction_id[12], size_t buffer_len, struct sockaddr_in *peer_addr)
{
	ssize_t len;
	socklen_t peer_addr_len = sizeof(*peer_addr);

	len = recvfrom(fd, buffer, buffer_len, 0, (struct sockaddr *)peer_addr, &peer_addr_len);
	if (len < 0) {
		syslog(LOG_ERR, "%s: recvfrom(): %m", "receive_stun_response");
		return 0;
	}
	if (peer_addr_len != sizeof(*peer_addr)) {
		syslog(LOG_ERR, "%s: recvfrom(): peer_addr_len mismatch",
		       "receive_stun_response");
		return 0;
	}
	if (len < 20) {
		syslog(LOG_WARNING, "%s: response too short : %ld",
		       "receive_stun_response", (long)len);
		return 0;
	}

	/* Check that buffer is STUN message with class Response
	 * and Binding method with transaction id */
	if ((buffer[0] & 0xFF) != 0x01 || (buffer[1] & 0xEF) != 0x01) {
		syslog(LOG_WARNING, "%s: STUN message type is 0x%02x%02x",
		       "receive_stun_response", buffer[0], buffer[1]);
		return 0;
	}
	if (memcmp(buffer+8, transaction_id, 12) != 0) {
		syslog(LOG_WARNING, "%s: transaction id mismatch",
		       "receive_stun_response");
		return 0;
	}

	return len;
}

/*! \brief Wait for STUN response messages and try to receive them
 * \param[in] fds
 * \param[in] transaction_ids
 * \param[out] buffers
 * \param[in] buffer_lens
 * \param[out] peer_addrs
 * \param[out] lens
 * \return -1 for error, 0 for success */
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

		syslog(LOG_DEBUG, "%s: waiting %ld secs and %ld usecs", "wait_for_stun_responses", (long)timeout.tv_sec, (long)timeout.tv_usec);
		ret = select(max_fd+1, &fdset, NULL, NULL, &timeout);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			syslog(LOG_ERR, "%s: select(): %m", "wait_for_stun_responses");
			return -1;
		}
		if (ret == 0) {
			syslog(LOG_DEBUG, "%s: select(): no more responses", "wait_for_stun_responses");
			return 0;
		}

		for (i = 0; i < 4; ++i)
			if (FD_ISSET(fds[i], &fdset))
				lens[i] = receive_stun_response(fds[i], buffers[i], transaction_ids[i], buffers_lens[i], &peer_addrs[i]);

		syslog(LOG_DEBUG, "%s: received responses: %u", "wait_for_stun_responses", (unsigned)(!!lens[0] + !!lens[1] + !!lens[2] + !!lens[3]));

		if (lens[0] && lens[1] && lens[2] && lens[3])
			return 0;
	}

	return 0;
}

/*! \brief Convert STUN Attribute type to name
 * see :
 * - RFC 3489 11.2 Message Attributes
 * - RFC 5389 18.2 STUN Attribute Registry
 * - RFC 5780 9.1 STUN Attribute Registry
 * \param[in] attr_type 16 bits attribute type
 * \return the attribute name */
static const char * get_stun_attr_name(uint16_t attr_type)
{
	switch(attr_type) {
	case 0x0001:	/* RFC 3489 */
		return "MAPPED-ADDRESS";
	case 0x0002:	/* RFC 3489, removed in RFC 5389 */
		return "RESPONSE-ADDRESS";
	case 0x0003:	/* RFC 3489, removed in RFC 5389, added again in RFC 5780*/
		return "CHANGE-REQUEST";
	case 0x0004:	/* RFC 3489, removed in RFC 5389 */
		return "SOURCE-ADDRESS";
	case 0x0005:	/* RFC 3489, removed in RFC 5389*/
		return "CHANGED-ADDRESS";
	case 0x0006:	/* RFC 3489 */
		return "USERNAME";
	case 0x0007:	/* RFC 3489, removed in RFC 5389 */
		return "PASSWORD";
	case 0x0008:	/* RFC 3489 */
		return "MESSAGE-INTEGRITY";
	case 0x0009:	/* RFC 3489 */
		return "ERROR-CODE";
	case 0x000a:	/* RFC 3489 */
		return "UNKNOWN-ATTRIBUTES";
	case 0x000b:	/* RFC 3489, removed in RFC 5389 */
		return "REFLECTED-FROM";
	case 0x0014:	/* RFC 5389 */
		return "REALM";
	case 0x0015:	/* RFC 5389 */
		return "NONCE";
	case 0x0020:	/* RFC 5389 */
	case 0x8020:	/* Was XOR-MAPPED-ADDRESS (draft 2 of 2005) */
		return "XOR-MAPPED-ADDRESS";
	case 0x0026:	/* RFC 5780 */
		return "PADDING";
	case 0x0027:	/* RFC 5780 */
		return "RESPONSE-PORT";

	case 0x8022:	/* RFC 5389 */
		return "SOFTWARE";
	case 0x8023:	/* RFC 5389 */
		return "ALTERNATE-SERVER";
	case 0x8027:	/* RFC 5780 */
		return "CACHE-TIMEOUT";
	case 0x8028:	/* RFC 5389 */
		return "FINGERPRINT";
	case 0x802b:	/* RFC 5780 */
		return "RESPONSE-ORIGIN";
	case 0x802c:	/* RFC 5780 */
		return "OTHER-ADDRESS";
	default:
		return "!UNKOWN!";
	}
}

/*! \brief parse STUN response message
 * Parse Mapped Address (with port) from STUN response message
 * \param[in] buffer STUN message
 * \param[in] len STUN message length
 * \param[out] mapped_addr Mapped Address
 * \return -1 for error, 0 for success */
static int parse_stun_response(unsigned char *buffer, size_t len, struct sockaddr_in *mapped_addr)
{
	unsigned char *ptr, *end;
	uint16_t attr_type;
	uint16_t attr_len;
	int have_address;
	int have_xor_mapped_address;
	int have_other_address;

	if (len < 20)
		return -1;

	syslog(LOG_DEBUG, "%s: Type 0x%04x, Length %hu, Magic Cookie %02x%02x%02x%02x",
	       "parse_stun_response", ((uint16_t)buffer[0] << 8) + buffer[1],
	       (uint16_t)((buffer[2] << 8) + buffer[3]),
	       buffer[4], buffer[5], buffer[6], buffer[7]);

	/* Check that buffer is STUN message with class Response and Binding method */
	if (buffer[0] != 0x01 || (buffer[1] & 0xEF) != 0x01)
		return -1;

	/* Check that STUN message is not longer as buffer length */
	if (((size_t)buffer[2] << 8) + buffer[3] + 20 > len) {
		syslog(LOG_ERR, "%s: truncated STUN response", "parse_stun_response");
		return -1;
	}

	ptr = buffer + 20;
	end = buffer + len;
	have_address = 0;
	have_xor_mapped_address = 0;
	have_other_address = 0;

	while (ptr + 4 <= end) {

		attr_type = ((uint16_t)ptr[0] << 8) + ptr[1];
		attr_len = ((uint16_t)ptr[2] << 8) + ptr[3];
		ptr += 4;

		if (ptr + attr_len > end) {
			syslog(LOG_WARNING, "%s: truncated attribute", "parse_stun_response");
			break;
		}

		switch (attr_type) {
		case 0x0001:	/* MAPPED-ADDRESS */
		case 0x0020:	/* XOR-MAPPED-ADDRESS (RFC 5389) */
		case 0x8020:	/* Was XOR-MAPPED-ADDRESS (draft 2 of 2005) */
			/* Mapped Address or XOR Mapped Address */
			if (attr_len == 8 && ptr[1] == 1) {
				/* IPv4 address */
				if ((attr_type & 0x7fff) == 0x0020) {
					/* Restore XOR Mapped Address */
					ptr[2] ^= buffer[4];
					ptr[3] ^= buffer[5];
					ptr[4] ^= buffer[4];
					ptr[5] ^= buffer[5];
					ptr[6] ^= buffer[6];
					ptr[7] ^= buffer[7];
				}

				syslog(LOG_DEBUG, "%s: %s %hhu.%hhu.%hhu.%hhu:%hu",
				       "parse_stun_response",
				       get_stun_attr_name(attr_type),
				       ptr[4], ptr[5], ptr[6], ptr[7],
				       (uint16_t)((ptr[2] << 8) + ptr[3]));

				/* Prefer XOR Mapped Address, some NATs change IP addresses in UDP packets */
				if (!have_xor_mapped_address) {
					mapped_addr->sin_family = AF_INET;
					mapped_addr->sin_port = htons(((uint16_t)ptr[2] << 8) + ptr[3]);
					mapped_addr->sin_addr.s_addr = htonl(((uint32_t)ptr[4] << 24) + (ptr[5] << 16) + (ptr[6] << 8) + ptr[7]);
				}

				if ((attr_type & 0x7fff) == 0x0020)
					have_xor_mapped_address = 1;

				have_address = 1;
			}
			break;
		case 0x0009:	/* ERROR-CODE */
			if (attr_len >= 4) {
				syslog(LOG_WARNING, "%s: ERROR-CODE %u %.*s",
			       "parse_stun_response", (unsigned)ptr[2] * 100 + ptr[3],
			       attr_len - 4, ptr + 4);
			}
			break;
		case 0x802b:	/* RESPONSE-ORIGIN (RFC 5780) */
		case 0x0004:	/* Was SOURCE-ADDRESS (RFC 3489) */
		case 0x802c:	/* OTHER-ADDRESS (RFC 5780) */
		case 0x0005:	/* Was CHANGED-ADDRESS (RFC 3489) */
			if (attr_len == 8 && ptr[1] == 1) {
				if (attr_type == 0x802c || attr_type == 0x0005) have_other_address = 1;
				syslog(LOG_DEBUG, "%s: %s %hhu.%hhu.%hhu.%hhu:%hu",
				       "parse_stun_response",
				       get_stun_attr_name(attr_type),
				       ptr[4], ptr[5], ptr[6], ptr[7],
				       (uint16_t)((ptr[2] << 8) + ptr[3]));
			}
			break;
		case 0x8022:	/* SOFTWARE (RFC 5389) */
			syslog(LOG_DEBUG, "%s: SOFTWARE %.*s", "parse_stun_response", attr_len, ptr);
			break;
		default:
			/* Attributes 0x0000-0x7FFF are Comprehension-required */
			syslog((attr_type & 0x8000) ? LOG_DEBUG : LOG_WARNING,
			       "%s: ignored attribute type 0x%04x %s (len=%hu)",
			       "parse_stun_response",
			       attr_type, get_stun_attr_name(attr_type), attr_len);
		}

		ptr += attr_len;
	}

	if (!have_other_address && have_address) {
		syslog(LOG_ERR, "STUN server not supported, not returning "
			"OTHER-ADDRESS / support CHANGE-REQUEST's required for "
			"endpoint-independent (1:1) CGNAT filtering tests per RFC 5780.");
		return -1;
	}
	return (have_address && have_other_address) ? 0 : -1;
}

/* Perform main STUN operation, return external IP address and check
 * if host is behind restrictive, symmetric NAT or behind firewall.
 * Restrictive NAT means any NAT which do some filtering and
 * which is not static full-cone NAT 1:1, basically NAT which is not usable
 * for port forwarding */
int perform_stun(const char *if_name, const char *if_addr, const char *stun_host, unsigned short stun_port, struct in_addr *ext_addr, int *restrictive_nat)
{
	int fds[4];
	size_t responses_lens[4];
	unsigned char responses_bufs[4][1024];
	unsigned char *responses[4];
	size_t responses_sizes[4];
	unsigned char requests[4][28];
	unsigned char *transaction_ids[4];
	int have_mapped_addr, mapped_addrs_count;
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

		/* Determine unrestricted endpoint-independent (1:1) CGNAT in two STUN requests per RFC 5780 4.4 test I/II */
		/* 1. Connectivity (binding, detect public IPv4), 2. CHANGE-REQUEST with change-IP and change-port set */
		/* https://datatracker.ietf.org/doc/html/rfc5780#section-4.4 */
		fill_request(requests[i], i, i);
		transaction_ids[i] = requests[i]+8;
	}

	syslog(LOG_INFO, "%s: local ports %hu %hu %hu %hu",
	       "perform_stun", local_ports[0], local_ports[1],
	       local_ports[2], local_ports[3]);

	/* Unblock local ports */
	for (i = 0; i < 4; ++i) {
		if (add_filter_rule2(if_name, NULL, if_addr, local_ports[i], local_ports[i], IPPROTO_UDP, "stun test") < 0) {
			syslog(LOG_ERR, "%s: add_filter_rule2(..., %hu, ...) FAILED",
			       "perform_stun", local_ports[i]);
		}
	}

	/* Send STUN requests and wait for responses */
	for (j = 0; j < 3; ++j) {

		for (i = 0; i < 4; ++i) {
			ssize_t n;
			if (responses_lens[i])
				continue;
			n = sendto(fds[i], requests[i], sizeof(requests[i]), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
			if (n != sizeof(requests[i])) {
				syslog(LOG_ERR, "%s: #%d,%d sendto(): %m", "perform_stun", j, i);
				break;
			}
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
	have_mapped_addr = 0;
	mapped_addrs_count = 0;
	for (i = 0; i < 4; ++i) {
		if (parse_stun_response(responses[i], responses_lens[i], &mapped_addrs[i]) == 0) {
			mapped_addrs_count++;
			have_mapped_addr |= (1 << i);
			if (!have_ext_addr) {
				memcpy(ext_addr, &mapped_addrs[i].sin_addr, sizeof(*ext_addr));
				have_ext_addr = 1;
			}
		}
	}

	/* We have no external address */
	if (!have_ext_addr) {
		errno = ENXIO;
		return -1;
	}

	*restrictive_nat = 0;

	if (mapped_addrs_count < 4) {
		/* We have not received all four responses,
		 * therefore NAT or firewall is doing some filtering */
		syslog(LOG_NOTICE, "%s: %d response out of 4 received",
		       "perform_stun", mapped_addrs_count);
		*restrictive_nat = 1;
	}

	if (memcmp(&remote_addr, &peer_addrs[0], sizeof(peer_addrs[0])) != 0) {
		/* We received STUN response from different address
		 * even we did not asked for it, so some strange NAT is active */
		syslog(LOG_NOTICE, "%s: address changed",
		       "perform_stun");
		*restrictive_nat = 1;
	}

	for (i = 0; i < 4; ++i) {
		if (!(have_mapped_addr & (1 << i)))
			continue;
		if (ntohs(mapped_addrs[i].sin_port) != local_ports[i] || memcmp(&mapped_addrs[i].sin_addr, ext_addr, sizeof(*ext_addr)) != 0) {
			char mapped_addr_str[32];
			sockaddr_to_string((struct sockaddr *)&mapped_addrs[i], mapped_addr_str, sizeof(mapped_addr_str));
			/* External IP address or port was changed,
			 * therefore symmetric NAT is active */
			syslog(LOG_NOTICE, "%s: #%d external address or port changed : %s:%hu => %s",
			       "perform_stun", i, inet_ntoa(*ext_addr), local_ports[i], mapped_addr_str);
			*restrictive_nat = 1;
		}
	}

	/* Otherwise we are either directly connected or behind unrestricted full-cone NAT 1:1 without filtering */
	/* There is no filtering, so port forwarding would work fine */
	return 0;
}
