/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2008 Vincent Bernat <bernat@luffy.cx>
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

#ifndef _EDP_H
#define _EDP_H

#define EDP_MULTICAST_ADDR	{						\
	0x00, 0xe0, 0x2b, 0x00, 0x00, 0x00					\
}
#define LLC_ORG_EXTREME { 0x00, 0xe0, 0x2b }
#define LLC_PID_EDP 0x00bb

#define EDP_TLV_MARKER	 0x99

enum {
	EDP_TLV_NULL			= 0,
	EDP_TLV_DISPLAY			= 1,
	EDP_TLV_INFO			= 2,
	EDP_TLV_VLAN			= 5,
	EDP_TLV_ESRP			= 8,
};

#endif /* _EDP_H */
