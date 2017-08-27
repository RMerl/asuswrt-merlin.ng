/*
 * FLOW test code to suspend/resume/delete a 5 tuple
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>

#include "flow_api.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

static int get_address(char *str, int *family, ip_address_t *ipa)
{
	struct addrinfo hint, *res = NULL;
	int ret;
	ip_address_t addr;

	memset(&hint, '\0', sizeof(hint));
	memset(&addr, '\0', sizeof(addr));

	hint.ai_family = PF_UNSPEC;
	hint.ai_flags = AI_NUMERICHOST;

	ret = getaddrinfo(str, NULL, &hint, &res);
	if (ret) {
		/* Invalid IP address */
		return FALSE;
	}

	if (!inet_pton(res->ai_family, str, &addr)) {
		/* Parse error */
		return FALSE;
	}

	*family = res->ai_family;
	*ipa = addr;

	return TRUE;
}

static int get_port(char *str, uint16_t *port)
{
	char *ptr = NULL;
	unsigned long val;

	val = strtoul(str, &ptr, 0);

	if (ptr && *ptr != '\0') {
		return FALSE;
	}

	if (val > USHRT_MAX) {
		return FALSE;
	}

	*port = htons((uint16_t) val);

	return TRUE;
}

static int check_user_args(ctf_tuple_t *tp,
                           char *src_ip,
                           char *src_port,
                           char *dst_ip,
                           char *dst_port,
                           char *protocol)
{
	unsigned long val;
	char *ptr = NULL;
	ctf_tuple_t t;
	int src_family, dst_family;

	if (!get_address(src_ip, &src_family, &t.src_addr)) {
		fprintf(stderr, "Invalid source IP '%s'\n", src_ip);
		return FALSE;
	}

	if (!get_port(src_port, &t.src_port)) {
		fprintf(stderr, "Invalid source port '%s'\n", src_port);
		return FALSE;
	}

	if (!get_address(dst_ip, &dst_family, &t.dst_addr)) {
		fprintf(stderr, "Invalid destination IP '%s'\n", dst_ip);
		return FALSE;
	}

	if (!get_port(dst_port, &t.dst_port)) {
		fprintf(stderr, "Invalid destination port '%s'\n", dst_port);
		return FALSE;
	}

	if ((src_family == AF_INET6) || (dst_family == AF_INET6)) {
		t.family = AF_INET6;
	} else {
		t.family = AF_INET;
	}

	val = strtoul(protocol, &ptr, 0);

	if (ptr && (*ptr != '\0')) {
		/* Parse error */
		return FALSE;
	} else {
		if (val >= IPPROTO_MAX) {
			fprintf(stderr, "Protocol %ld out of range\n", val);
			return FALSE;
		}

		t.protocol = (uint8_t) val;
	}

	*tp = t;

	return TRUE;
}

static void usage(char *prog)
{
	fprintf(stderr,
	        "Usage: %s <src-ip> <src-port> <dst-ip> <dst-port> <protocol> <suspend|resume|delete|valid>\n",
	        prog);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	ctf_tuple_t tuple;
	char *prog = basename(argv[0]);
	char *cmd;
	char buf1[INET6_ADDRSTRLEN];
	char buf2[INET6_ADDRSTRLEN];
	int ret;

	if (argc != 7) {
		usage(prog);
	}

	if (!check_user_args(&tuple, argv[1], argv[2], argv[3], argv[4], argv[5])) {
		usage(prog);
	}

	cmd = argv[6];

	printf("SRC=%s (%d) DST=%s (%d) PROTO=%d cmd=%s\n",
	       inet_ntop(tuple.family, &tuple.src_addr, buf1, sizeof(buf1)),
	       ntohs(tuple.src_port),
	       inet_ntop(tuple.family, &tuple.dst_addr, buf2, sizeof(buf2)),
	       ntohs(tuple.dst_port),
	       tuple.protocol, cmd);

	if (strcmp(cmd, "resume") == 0) {
		ret = flow_resume(tuple.family,
		                  &tuple.src_addr,
		                  tuple.src_port,
		                  &tuple.dst_addr,
		                  tuple.dst_port,
		                  tuple.protocol);
	} else if (strcmp(cmd, "suspend") == 0) {
		ret = flow_suspend(tuple.family,
		                   &tuple.src_addr,
		                   tuple.src_port,
		                   &tuple.dst_addr,
		                   tuple.dst_port,
		                   tuple.protocol);
	} else if (strcmp(cmd, "delete") == 0) {
		ret = flow_delete(tuple.family,
		                  &tuple.src_addr,
		                  tuple.src_port,
		                  &tuple.dst_addr,
		                  tuple.dst_port,
		                  tuple.protocol);
	} else if (strcmp(cmd, "valid") == 0) {
		ret = flow_valid(tuple.family,
		                 &tuple.src_addr,
		                 tuple.src_port,
		                 &tuple.dst_addr,
		                 tuple.dst_port,
		                 tuple.protocol);
	} else {
		fprintf(stderr, "%s not supported\n", cmd);
		exit(EXIT_FAILURE);
	}

	return ret;
}
