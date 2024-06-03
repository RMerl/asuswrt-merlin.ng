/** \file bcm_btrace_shared.h
 *
 * Binary event tracing shared structure definitions.
 *
 * See https://confluence.broadcom.net/display/BCAWLAN/Binary+event+tracing
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: bcm_btrace_shared.h 824946 2023-05-09 11:43:50Z $
 */

#ifndef _bcm_btrace_shared_h_
#define _bcm_btrace_shared_h_

#include <typedefs.h>

#define BTRACE_IPC_D2H_FLAGS_INITIALIZED	0x01
#define BTRACE_IPC_D2H_FLAGS_ENABLED		0x02
#define BTRACE_IPC_D2H_FLAGS_ROTATING_TRACE	0x04

#define BTRACE_IPC_H2D_FLAGS_ENABLED		0x01

#define BTRACE_FILE_NAME			"/var/btrace_wl%u_%u.bin"
#define BTRACE_FILE_MODE			0666
#define BTRACE_FILE_MAGIC			"btrace"
#define BTRACE_VERSION				1

/* Shared memory area */
typedef struct btrace_ipc
{
	uint8	flags_d2h;	/* Dongle-to-host flags, @see BTRACE_IPC_D2H_FLAGS_* */
	uint8	flags_h2d;	/* Host-to-dongle flags, @see BTRACE_IPC_H2D_FLAGS_* */
	uint8	trace_seq;	/* Trace sequence number, used for rotating and flushing */
	uint8	file_seq;	/* Trace file sequence number, appended to trace filename */
	uint32	rd_offset;	/* Shared buffer read offset, in bytes */
	uint32	wr_offset;	/* Shared buffer write offset, in bytes */
	uint32	buf_size;	/* Shared buffer size, in bytes */
	uint64  buf;		/* Address of shared buffer. We are using 64-bits here because
				 * this file is shared between NIC, FD and DHD.
				 */
} btrace_ipc_t;

/* Trace file header format */
typedef struct trace_header
{
	char	magic[6];	/* @see BTRACE_FILE_MAGIC */
	uint8	version;	/* @see BTRACE_VERSION */
	uint8	unit;		/* Radio index */
	uint32	base_address;	/* Kernel module base address (NIC), or 0 (FD) */
} btrace_header_t;

#endif /* _bcm_btrace_shared_h_ */
