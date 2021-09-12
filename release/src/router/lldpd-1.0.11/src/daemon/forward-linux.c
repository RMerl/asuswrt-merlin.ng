/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2013 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "lldpd.h"

#include <unistd.h>

int
ip_forwarding_enabled(int af)
{
	int fd, rc = -1;
	char *fname;
	char status;

	if (af == LLDPD_AF_IPV4)
		fname = PROCFS_SYS_NET "ipv4/ip_forward";
	else if (af == LLDPD_AF_IPV6)
		fname = PROCFS_SYS_NET "ipv6/conf/all/forwarding";
	else
		return -1;

	if ((fd = priv_open(fname)) < 0)
		return -1;

	if (read(fd, &status, 1) == 1)
		rc = (status == '1');

	close(fd);
	return rc;
}

int
interfaces_routing_enabled(struct lldpd *cfg) {
	(void)cfg;
	int rc;

	rc = ip_forwarding_enabled(LLDPD_AF_IPV4);
	/*
	 * Report being a router if IPv4 forwarding is enabled.
	 * In case of error also stop the execution right away.
	 * If IPv4 forwarding is disabled we'll check the IPv6 status.
	 */
	if (rc != 0)
		return rc;

	return ip_forwarding_enabled(LLDPD_AF_IPV6);
}
