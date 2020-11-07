/* D11reglist prototype for Broadcom 802.11abgn
 * Networking Adapter Device Drivers.
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
 * $Id: d11reglist_proto.h 732834 2017-11-21 17:28:50Z $
 */
#ifndef _d11reglist_proto_h_
#define _d11reglist_proto_h_

/* this is for dump_mac */
enum {
	D11REG_TYPE_IHR16    = 0,
	D11REG_TYPE_IHR32    = 1,
	D11REG_TYPE_SCR      = 2,
	D11REG_TYPE_SHM      = 3,
	D11REG_TYPE_TPL      = 4,
	D11REG_TYPE_GE64     = 5,
	D11REG_TYPE_KEYTB    = D11REG_TYPE_GE64,
	D11REG_TYPE_IHRX16   = 6,
	D11REG_TYPE_SCRX     = 7,
	D11REG_TYPE_SHMX     = 8,
	D11REG_TYPE_IHR116   = 9,
	D11REG_TYPE_SCR1     = 10,
	D11REG_TYPE_SHM1     = 11,
	D11REG_TYPE_MAX      = 12
};

#define D11REGTYPENAME {		\
	"ihr", "ihr", "scr", "shm",	\
	"tpl", "keytb", "ihrx", "scrx",	\
	"shmx", "ihr1", "scr1", "shm1",	\
}

typedef enum _d11reg_xtlv_type {
	D11REG_XTLV_ALL = 0x0,
	D11REG_XTLV_PSMR = 0x1,
	D11REG_XTLV_PSMX = 0x2,
	D11REG_XTLV_SVMP = 0x3,
	D11REG_XTLV_SCTPL = 0x4,
	D11REG_XTLV_PSMR1 = 0x5
} d11reg_xtlv_type;

typedef struct _d11regs_list {
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

typedef struct _sctplregs_list {
	uint16 corerev;		/* this one is not address */
	/* below are ihr/shm addrs for each info */
	uint16 revmajor_shm;
	uint16 revminor_shm;
	uint16 ucfeature_shm;
	uint16 smpctrl_ihr;
	uint16 mctrl1_ihr;
	uint16 scpstrtptr_ihr;
	uint16 scpstopptr_ihr;
	uint16 scpcurptr_ihr;
	uint16 tplwrptr_ihr;
	uint16 tplwrdata_ihr;
	uint16 utracesptr_shm;
	uint16 utraceeptr_shm;
	uint16 uptr_scr;
	uint16 utracesptr_shmx;
	uint16 utraceeptr_shmx;
	uint16 uptr_scrx;
	uint16 pad; /* for 4bytes alignment */
} sctplregs_list_t;

#endif /* _d11reglist_proto_h_ */
