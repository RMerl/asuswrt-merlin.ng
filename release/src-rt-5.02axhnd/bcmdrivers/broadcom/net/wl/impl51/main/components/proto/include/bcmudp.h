/*
 * Fundamental constants relating to UDP Protocol
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * $Id: bcmudp.h 597933 2015-11-06 18:52:06Z $
 */

#ifndef _bcmudp_h_
#define _bcmudp_h_

#ifndef _TYPEDEFS_H_
#include <typedefs.h>
#endif // endif

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/* UDP header */
#define UDP_DEST_PORT_OFFSET	2	/* UDP dest port offset */
#define UDP_LEN_OFFSET		4	/* UDP length offset */
#define UDP_CHKSUM_OFFSET	6	/* UDP body checksum offset */

#define UDP_HDR_LEN	8	/* UDP header length */
#define UDP_PORT_LEN	2	/* UDP port length */

/* These fields are stored in network order */
BWL_PRE_PACKED_STRUCT struct bcmudp_hdr
{
	uint16	src_port;	/* Source Port Address */
	uint16	dst_port;	/* Destination Port Address */
	uint16	len;		/* Number of bytes in datagram including header */
	uint16	chksum;		/* entire datagram checksum with pseudoheader */
} BWL_POST_PACKED_STRUCT;

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif	/* #ifndef _bcmudp_h_ */
