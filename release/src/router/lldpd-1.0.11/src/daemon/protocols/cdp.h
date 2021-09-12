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

#ifndef _CDP_H
#define _CDP_H

#define CDP_MULTICAST_ADDR	{						\
	0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc					\
}
#define FDP_MULTICAST_ADDR	{			\
	0x01, 0xe0, 0x52, 0xcc, 0xcc, 0xcc,		\
}
#define LLC_ORG_CISCO { 0x00, 0x00, 0x0c }
#define LLC_ORG_FOUNDRY { 0x00, 0xe0, 0x52 }
#define LLC_PID_CDP 0x2000
/* Other protocols */
#define LLC_PID_DRIP 0x102
#define LLC_PID_PAGP 0x104
#define LLC_PID_PVSTP 0x10b
#define LLC_PID_UDLD 0x111
#define LLC_PID_VTP 0x2003
#define LLC_PID_DTP 0x2004
#define LLC_PID_STP 0x200a

enum {
	CDP_TLV_CHASSIS			= 1,
	CDP_TLV_ADDRESSES		= 2,
	CDP_TLV_PORT			= 3,
	CDP_TLV_CAPABILITIES		= 4,
	CDP_TLV_SOFTWARE		= 5,
	CDP_TLV_PLATFORM		= 6,
	CDP_TLV_NATIVEVLAN		= 10,
	CDP_TLV_POWER_CONSUMPTION	= 0x10,
	CDP_TLV_POWER_REQUESTED		= 0x19,	
	CDP_TLV_POWER_AVAILABLE		= 0x1A
};

#define CDP_ADDRESS_PROTO_IP 0xcc

#define CDP_CAP_ROUTER             0x01
#define CDP_CAP_TRANSPARENT_BRIDGE 0x02
#define CDP_CAP_SOURCE_BRIDGE      0x04
#define CDP_CAP_SWITCH             0x08
#define CDP_CAP_HOST               0x10
#define CDP_CAP_IGMP               0x20
#define CDP_CAP_REPEATER           0x40

#endif /* _CDP_H */

