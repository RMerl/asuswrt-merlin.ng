/*
   WSDD - Web Service Dynamic Discovery protocol server

   LLMNR responder

	Copyright (c) 2016 NETGEAR
	Copyright (c) 2016 Hiro Sugawara

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   This code is based on the wsdd implementation for Samba by
   community contributers.

   The original copyright comment follows.
 */

/*
   Unix SMB/CIFS implementation.

   Link-Local Multicast Name Resolution (LLMNR) helper functions
   (https://tools.ietf.org/html/rfc4795)

   Copyright (C) Tobias Waldvogel 2013
   Copyright (C) Jose M. Prieto 2015

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "wsdd.h"

#include <stdio.h> // FILE, fopen(), fscanf(), snprintf()
#include <stdlib.h> // realloc(), free()
#include <unistd.h> // gethostname()
#include <string.h> // memcpy(), strcat(), strdup()
#include <errno.h> // errno, EINVAL
#include <arpa/inet.h> // inet_ntop()

#define DNS_TYPE_ANY	0x00FF
#define DNS_TYPE_A	0x0001	/* rfc 1035 */
#define DNS_TYPE_AAAA	0x001C	/* rfc 3596 */
#define DNS_CLASS_IN	0x0001	/* rfc 1035 */

#ifdef NL_DEBUG
static void dumphex(const char *label, const void *p, size_t len)
{
	if (debug_L >= 5)
		dump(p, len, 0, label);
}
#endif

static int llmnr_send_response(struct endpoint *ep, _saddr_t *sa,
				const uint8_t *in, size_t inlen)
{
	uint16_t qdcount, ancount, nscount;
	uint16_t qtype, qclass;
	char *in_name, *out = NULL;
	const uint8_t *in_name_p = NULL;
	size_t in_name_len, out_name_len = 0;
	size_t answer_len = 0;
	int ret;
	_saddr_t ci;
	socklen_t slen = (sa->ss.ss_family == AF_INET) ? sizeof sa->in : sizeof sa->in6;
#ifdef NL_DEBUG
	dumphex("LLMNR INPUT: ", in, inlen);
#endif
	if (connected_if(sa, &ci)) {
		char buf[_ADDRSTRLEN];
		DEBUG(1, L, "llmnr: connected_if: %s: %s",
			inet_ntop(sa->ss.ss_family, _SIN_ADDR(sa), buf, sizeof buf),
			strerror(errno));
		return -1;
	}

	/*
	 * LLMNR header format according to RFC 4795:
	 *
	 *   0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
	 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	 * |                   ID                          |
	 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	 * |QR|   Opcode  | C|TC| T| Z| Z| Z| Z|   RCODE   |
	 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	 * |                  QDCOUNT                      |
	 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	 * |                  ANCOUNT                      |
	 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	 * |                  NSCOUNT                      |
	 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	 * |                  ARCOUNT                      |
	 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	 */
	errno = EINVAL;

	/*
	 * LLMNR packet format has a header of 12 bytes
	 * plus at least 1 byte of question section
	 * (see RFCs 4795, 1035, 3596)
	 */
	if (inlen < 13) {
		DEBUG(1, L, "llmnr: packet less than 13 bytes");
		return -1;
	}

	/*
	 * 3rd octect of LLMNR header
	 * check for standard query:
	 * - Q/R (1-bit)     = 0
	 * - OPCODE (4-bits) = 0
	 */
	if (in[2] & 0xF8) {
		DEBUG(1, L, "llmnr: not standard query");
		return -1;
	}

	/* check whether conflict bit (C) is not set */
	if (in[2] & 0x04) {
		DEBUG(1, L, "llmnr: conflict bit set in query");
		return -1;
	}

	/* check whether truncation bit is not set */
	if (in[2] & 0x02) {
		DEBUG(1, L, "llmnr: truncation bit set in query");
		return -1;
	}

	/*
	 * check number of entries in question section (QDCOUNT)
	 * it must be just one
	 */
	qdcount = in[4] * 256 + in[5];
	if (qdcount != 1) {
		DEBUG(1, L, "llmnr: only one question entry allowed, found %u", qdcount);
		return -1;
	}

	/*
	 * check number of entries in answer and nameserver sections
	 * must be zero in the request
	 */
	ancount = in[6] * 256 + in[7];
	nscount = in[8] * 256 + in[9];
	if (ancount > 0 || nscount > 0) {
		DEBUG(1, L, "llmnr: number of answer and/or nameserver entries "
			  "in query is invalid (ancount: %u, nscount: %u)",
			  ancount, nscount);
		return -1;
	}

	/* process all labels in question section */
	in_name = strdup("");
	in_name_len = 0;
	in_name_p = &in[12];
	while (*in_name_p > 0) {
		/*
		 * not supporting message compression
		 * see section 4.1.4 of RFC 1035
		 */
		if (*in_name_p >= 0xC0) {
			DEBUG(1, L, "llmnr: message compression not supported");
			free(in_name);
			return -1;
		}

		/* append to the whole name */
		in_name_len += *in_name_p + (*in_name ? 1 : 0); // '.' if not first

		in_name = (char *) realloc(in_name, in_name_len + 1);
		if (in_name == NULL) {
			DEBUG(1, L, "llmnr: realloc() failed");
			return -1;
		}
		if (*in_name) strcat(in_name, ".");
		strncat(in_name, (const char *) in_name_p + 1, *in_name_p);

		/* next label */
		in_name_p += *in_name_p + 1;
	}

	/* verify in_name_len */
	if (in_name_len != strlen(in_name)) {
		DEBUG(1, L, "llmnr: bad name length %ld != %ld", in_name_len, strlen(in_name));
		free(in_name);
		return -1;
	}

	char abuf[_ADDRSTRLEN];
	DEBUG(2, L, "llmnr: name in query %s (length: %zu) from %s", in_name, in_name_len,
		inet_ntop(sa->ss.ss_family, _SIN_ADDR(sa), abuf, sizeof(abuf)));

	/*
	 * this implementation only supports questions of type A
	 * or AAAA
	 */
	qtype = in_name_p[1] * 256 + in_name_p[2];
	if (qtype != DNS_TYPE_ANY && qtype != DNS_TYPE_A && qtype != DNS_TYPE_AAAA) {
		DEBUG(1, L, "llmnr: record in question not of type ANY or A or AAAA: %#x", qtype);
		free(in_name);
		return -1;
	}

	/* this implementation only supports questions of class IN */
	qclass = in_name_p[3] * 256 + in_name_p[4];
	if (qclass != DNS_CLASS_IN) {
		DEBUG(1, L, "llmnr: record is not of class IN");
		free(in_name);
		return -1;
	}

	/* check whether we are authorized to resolve this query */

	int found = 0;
	if (!found && strlen(netbiosname) == in_name_len &&
	    strncasecmp(netbiosname, in_name, in_name_len) == 0)
		found = 1;

	if (!found && hostname && strlen(hostname) == in_name_len &&
	    strncasecmp(hostname, in_name, in_name_len) == 0)
		found = 1;

	for (const char **pp = &hostaliases; !found && pp != &netbiosaliases; pp = &netbiosaliases) {
	        for (const char *pname = *pp; pname && *pname;) {
			const char *pend = strchr(pname, ' ');
			size_t plen = pend ? (size_t) (pend - pname) : strlen(pname);
			if (plen == in_name_len &&
                            strncasecmp(pname, in_name, in_name_len) == 0) {
				found = 1;
				break;
			}
			pname += plen;
			while (*pname == ' ') pname++;
		}
	}

	if (!found) {
	    DEBUG(2, L, "llmnr: not authoritative for name %s", in_name);
	    free(in_name);
	    return -1;
	}

	free(in_name);

	/*
	 * start building up the LLMNR response
	 */
#if 0
	/* interpret whether IP is IPv4 or IPv6 and build up */
	if (!interpret_string_addr(&ss, ip, AI_NUMERICHOST)) {
		DEBUG(1, ("llmnr: can't interpret address %s\n", ip));
		return out;
	}
#endif
	/*
	 * determine the length of answer section
	 * see RFC 4795 section 2.3 for rules
	 */
	if ((qtype == DNS_TYPE_A && sa->ss.ss_family == AF_INET6) ||
	    (qtype == DNS_TYPE_AAAA && sa->ss.ss_family == AF_INET)) {
		answer_len = 0;
	/*
	 * according to RFC 1035, answer section size will be:
	 * - 2 bytes for pointer a name in query section (we are using a
	 *   referral)
	 * - 2 bytes QTYPE
	 * - 2 bytes QCLASS
	 * - 4 bytes TTL
	 * - 2 bytes RDLENGTH ... up to here 12 bytes
	 * - 4 bytes (AF_INET) or 16 bytes (AF_INET6) RDATA
	 */
	} else if (sa->ss.ss_family == AF_INET) {
		answer_len = 12 + sizeof(ci.in.sin_addr);
	} else if (sa->ss.ss_family == AF_INET6) {
		answer_len = 12 + sizeof(ci.in6.sin6_addr);
	} else {
		DEBUG(1, L, "llmnr: %s: %d", strerror(EAFNOSUPPORT), sa->ss.ss_family);
		return -1;
	}

	/*
	 * allocate output buffer
	 * size will be same one as incoming query plus the answer section
	 */
	out = (char *) calloc(inlen + answer_len, 1);
	if (out == NULL) {
		DEBUG(0, L, "llmnr: no memory for output buffer");
		return -1;
	}

	/* copy incoming message to output buffer */
	memcpy(out, in, inlen);

	/*
	 * set flags in response:
	 * - QR bit sets to 1
	 * - OPCODE sets to 0
	 * - C, TC and T bits set to 0
	 * - RCODE sets to 0
	 */
	out[2] = 0x80;
	out[3] = 0x00;

	/* length of answer section */
	out[6] = 0x00;
	if (answer_len == 0) {
		/* that's it, no answer section to build */
		out[7] = 0x00;
		goto send;
	}
	/* otherwise one answer */
	out[7] = 0x01;

	/* offset to beginning of answer section */
	out_name_len = inlen;

	/*
	 * pointer to name in question section
	 * (offset is 12th bytes from packet beginning)
	 */
	out[out_name_len++] = 0xC0;
	out[out_name_len++] = 0x0C;

	/* record type */
	out[out_name_len++] = 0x00;
	if (sa->ss.ss_family == AF_INET) {
		/* type A */
		out[out_name_len++] = 0x01;
	} else {
		/* type AAAA */
		out[out_name_len++] = 0x1C;
	}

	/* class IN */
	out[out_name_len++] = 0x00;
	out[out_name_len++] = 0x01;

	/* default TTL = 30: RFC 4795 2.8. RR TTL */
	out[out_name_len++] = 0;
	out[out_name_len++] = 0;
	out[out_name_len++] = 0;
	out[out_name_len++] = 30;

	/* RDLENGTH and RDATA in answer section */
	out[out_name_len++] = 0x00;

	if (sa->ss.ss_family == AF_INET) {
		size_t len = out[out_name_len++] = sizeof(ci.in.sin_addr);
		memcpy(out + out_name_len, &ci.in.sin_addr, len);
	} else {
		size_t len = out[out_name_len++] = sizeof(ci.in6.sin6_addr);
		memcpy(out + out_name_len, &ci.in6.sin6_addr, len);
	}
send:
#ifdef NL_DEBUG
	dumphex("LLMNR OUTPUT: ", out, inlen + answer_len);
#endif
	ret = sendto(ep->sock, out, inlen + answer_len, 0, (struct sockaddr *)sa, slen);

	free(out);
	return ret;
}

int llmnr_init(struct endpoint *ep)
{
	(void) ep; // silent "unused" warning
	return 0;
}

int llmnr_recv(struct endpoint *ep)
{
	uint8_t buf[9216+1]; // RFC 4795, Ethernet jumbo frame size
	_saddr_t sa;

	socklen_t slen = sizeof sa;
	ssize_t len = recvfrom(ep->sock, buf, sizeof(buf)-1, 0, (struct sockaddr *)&sa, &slen);

	if (len > 0) {
		buf[len] = '\0';
		llmnr_send_response(ep, &sa, buf, len);
	}

	return len;
}

void llmnr_exit(struct endpoint *ep)
{
	(void) ep; // silent "unused" warning
}
