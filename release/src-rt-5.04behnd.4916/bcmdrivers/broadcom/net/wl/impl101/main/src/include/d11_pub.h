/*
 * Broadcom d11 open definitions
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
 * <<Broadcom-WL-IPTag/Dual>>
 *
 * $Id$
 */

#ifndef	_d11_pub_
#define	_d11_pub_

#include <802.11.h>
#include <packed_section_start.h>
#define HE_EHT_FMT_MAX		3
#define HE_EHT_FMT_SU		0	/* SU */
#define HE_EHT_FMT_SUER		1	/* SU Extended-Range */
#define HE_EHT_FMT_MU		2	/* MU */
#define HE_EHT_FMT_TB		3	/* TB */

/** 802.11a PLCP header def */
typedef struct ofdm_phy_hdr ofdm_phy_hdr_t;
struct BWL_PRE_PACKED_STRUCT ofdm_phy_hdr {
	uint8	rlpt[3];	/**< rate, length, parity, tail */
	uint16	service;
	uint8	pad;
} BWL_POST_PACKED_STRUCT;

/** 802.11b PLCP header def */
typedef struct cck_phy_hdr cck_phy_hdr_t;
struct BWL_PRE_PACKED_STRUCT cck_phy_hdr {
	uint8	signal;
	uint8	service;
	uint16	length;
	uint16	crc;
} BWL_POST_PACKED_STRUCT;
#include <packed_section_end.h>

/* Frame Types */
enum {
	FT_LEGACY = -1,
	FT_CCK = 0,
	FT_OFDM,
	FT_HT,
	FT_VHT,
	FT_HE,
	FT_EHT,
	FT_AZ,
	FT_NUM  /* intentionally do not count FT_LEGACY */
};

/*
 * nss = ULRTINFO[13:14]
 * ru[0:2] = ULRTINFO[10:12]
 * ru_pri = ULRTINFO[9] -bit set to 1 indicates secondary 80MHZ
 * mcs = ULRTINFO[5:8]
 * ldpc = ULRTINFO[4]
 * ru[3:6] = ULRTINFO[0:3]
 * mmu_type = ULRTINFO[15] (0 for rx_ofdma, 1 for rx_mumimo)
 */

#define ULRTINFO_RUIDLSB3B_MASK   0x1C00
#define ULRTINFO_RUIDLSB3B_SHIFT  10
#define ULRTINFO_RUIDMSB4B_MASK  0xf
#define ULRTINFO_RUIDMSB4B_SHIFT  3
#define ULRTINFO_RUID(val_16bit) \
		((((val_16bit) & ULRTINFO_RUIDLSB3B_MASK) >> ULRTINFO_RUIDLSB3B_SHIFT)| \
		(((val_16bit) & ULRTINFO_RUIDMSB4B_MASK) << ULRTINFO_RUIDMSB4B_SHIFT))

#define ULRTINFO_RUID_PRIMSEC_MASK  0x0200
#define ULRTINFORUID_PRIMSEC_SHIFT 9
#define ULRTINFO_RUID_PRIMSEC(val_16bit) \
	(((val_16bit) & ULRTINFO_RUID_PRIMSEC_MASK) >> ULRTINFORUID_PRIMSEC_SHIFT)

#define ULRTINFO_NSS_MASK    0x6000
#define ULRTINFO_NSS_SHIFT   13
#define ULRTINFO_NSS(val_16bit)    ((((val_16bit) & ULRTINFO_NSS_MASK) >> ULRTINFO_NSS_SHIFT) + 1)

#define ULRTINFO_LDPC_MASK   0x0010
#define ULRTINFO_LDPC_SHIFT  4
#define ULRTINFO_LDPC(val_16bit)    (((val_16bit) & ULRTINFO_LDPC_MASK) >> ULRTINFO_LDPC_SHIFT)

#define ULRTINFO_MCS_MASK    0x01E0
#define ULRTINFO_MCS_SHIFT   5
#define ULRTINFO_MCS(val_16bit)    (((val_16bit) & ULRTINFO_MCS_MASK) >> ULRTINFO_MCS_SHIFT)

#define ULRTINFO_MMUTYPE_MASK   0x8000
#define ULRTINFO_MMUTYPE_SHIFT  15
#define ULRTINFO_MMUTYPE(val_16bit) \
	(((val_16bit) & ULRTINFO_MMUTYPE_MASK) >> ULRTINFO_MMUTYPE_SHIFT)
#define ULRTINFO_MMUTYPE_MUMIMO   1

#endif /* _d11_pub_ */
