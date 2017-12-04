/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
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

#ifndef _CTL_H
#define _CTL_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdint.h>
#include "marshal.h"

enum hmsg_type {
	NONE,
	GET_CONFIG,	        /* Get global configuration */
	SET_CONFIG,		/* Change global configuration */
	GET_INTERFACES,		/* Get list of interfaces */
	GET_CHASSIS,		/* Get local chassis */
	GET_INTERFACE,		/* Get all information related to an interface */
	GET_DEFAULT_PORT,	/* Get all information related to default port */
	SET_PORT,		/* Set port-related information (location, power, policy) */
	SUBSCRIBE,		/* Subscribe to neighbor changes */
	NOTIFICATION,		/* Notification message (sent by lldpd!) */
};

/** Header for the control protocol.
 *
 * The protocol is pretty simple. We send a single message containing the
 * provided message type with the message length, followed by the message
 * content.
 */
struct hmsg_header {
	enum hmsg_type type;
	size_t         len;
};
#define HMSG_MAX_SIZE (1<<19)

/* ctl.c */
int	 ctl_create(const char *);
int	 ctl_connect(const char *);
void	 ctl_cleanup(const char *);

int	 ctl_msg_send_unserialized(uint8_t **, size_t *,
				       enum hmsg_type,
				       void *, struct marshal_info *);
size_t	 ctl_msg_recv_unserialized(uint8_t **, size_t *,
				       enum hmsg_type,
				       void **, struct marshal_info *);

#endif
