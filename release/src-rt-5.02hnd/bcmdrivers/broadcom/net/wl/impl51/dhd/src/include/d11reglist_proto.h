/* D11reglist prototype for Broadcom 802.11abgn
 * Networking Adapter Device Drivers.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id: d11reglist_proto.h 649388 2016-07-15 22:54:42Z $
 */
#ifndef _d11reglist_proto_h_
#define _d11reglist_proto_h_

/* this is for dump_mac */
enum {
	D11REG_TYPE_IHR16  = 0,
	D11REG_TYPE_IHR32  = 1,
	D11REG_TYPE_SCR    = 2,
	D11REG_TYPE_SHM    = 3,
	D11REG_TYPE_TPL    = 4,
	D11REG_TYPE_GE64   = 5,
	D11REG_TYPE_KEYTB  = D11REG_TYPE_GE64,
	D11REG_TYPE_IHRX16 = 6,
	D11REG_TYPE_SCRX   = 7,
	D11REG_TYPE_SHMX   = 8,
	D11REG_TYPE_MAX    = 9
};

#define D11REGTYPENAME {		\
	"ihr", "ihr", "scr", "shm",	\
	"tpl", "keytb", "ihrx", "scrx",	\
	"shmx"				\
}

typedef struct _d11regs_bmp_list {
	uint8 type;
	uint16 addr;
	uint32 bitmap;
	uint8 step;
	uint16 cnt; /* can be used together with bitmap or by itself */
} d11regs_list_t;

#define D11REG_BLK_SIZE		32
typedef struct _d11regs_addr_list {
	uint8 type;
	uint16 cnt;
	uint16 addr[D11REG_BLK_SIZE]; /* allow up to 32 per list */
} d11regs_addr_t;

typedef struct _d11obj_cache_t {
	uint32 sel;
	uint32 val;
	uint16 addr32;
	bool cache_valid;
} d11obj_cache_t;

typedef struct _svmp_list {
	uint32 addr;
	uint16 cnt;
} svmp_list_t;

#endif /* _d11reglist_proto_h_ */
