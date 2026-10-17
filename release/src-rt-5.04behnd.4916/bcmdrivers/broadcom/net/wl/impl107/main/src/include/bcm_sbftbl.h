/*
 * +--------------------------------------------------------------------------+
 * bcm_sbftbl.h
 *
 * External Interface for Station Buffer Fairness Table (SBFTBL) module,
 * between dongle firmware (producer) and host driver (consumer).
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 * $Id: bcm_sbftbl.h 848516 2024-11-22 18:36:08Z $
 *
 * vim: set ts=4 noet sw=4 tw=80:
 * -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * +--------------------------------------------------------------------------+
 */
#ifndef __bcm_sbftbl_h__
#define __bcm_sbftbl_h__

/**
 * +--------------------------------------------------------------------------+
 *  Section: BCM_SBFTBL Shared Definitions between Dongle and Host
 * +--------------------------------------------------------------------------+
 */
enum sbftbl_ver_e {
	SBFTBL_VER_NOTSET,
	/* version numbers starts with 1 */
	SBFTBL_VER1 = 1,
	SBFTBL_VER_MIN = SBFTBL_VER1,
	SBFTBL_VER2 = 2,
	/* add more versions here */
	SBFTBL_VER_MAX = SBFTBL_VER2
};

/** Section: BCM_SBFTBL Compatability Control <version, release, patch> */
/** 32 bit [ <16 bit version> : <8 bit release> : <8 bit patch> ] */
#define SBFTBL_VERSION              SBFTBL_VER2
#define SBFTBL_RELEASE              (0)
#define SBFTBL_PATCH                (0)
#define SBFTBL_VERSIONCODE          \
	(((SBFTBL_VERSION) << 16) + ((SBFTBL_RELEASE) << 8) + ((SBFTBL_PATCH) << 0))

#define SBFTBL_VERSIONCODE_TO_VERSION(vc) ((((uint32)(vc)) >> 16) & 0xFFFF)
#define SBFTBL_VERSIONCODE_TO_RELEASE(vc) ((((uint32)(vc)) >>  8) & 0x00FF)
#define SBFTBL_VERSIONCODE_TO_PATCH(vc)   ((((uint32)(vc)) >>  0) & 0x00FF)

/** SBF Start signature SBFS => 5BF5 */
#define SBFTBL_START_SIGNATURE      (0x5BF5)

/** Version Release Patch: Dot Notation Form */
#define SBFTBL_VRP_FMT              " %s[%u.%u.%u]"
#define SBFTBL_VRP_VAL(vercode)     "SBFTBL", \
		SBFTBL_VERSIONCODE_TO_VERSION((vercode)),	\
		SBFTBL_VERSIONCODE_TO_RELEASE((vercode)),	\
		SBFTBL_VERSIONCODE_TO_PATCH((vercode))

/** Section: BCM_SBFTBL PCIE IPC Shared Structure */
#define SBFTBL_ALIGNMENT            (8)
#define __sbftbl_aligned            __attribute__ ((aligned (SBFTBL_ALIGNMENT)))

#define SBFTBL_INDEX_TYPE_STAID     0
#define SBFTBL_INDEX_TYPE_FLOWID    1

#define SBFTBL_WEIGHT_MIN           0x0000    /* Minimum Value of SBF Weight */
#define SBFTBL_WEIGHT_MAX           0xFFFF    /* Maximum Value of SBF Weight */

/* SBF Table entry types */

/* SBFv1: Entry has Station EA and BFW, bioth per SCB */
#define SBFTBL_ENTRY_TYPE_STAEABFW  0
/* SBFv2: Entry has badwidth table only per flowring id */
#define SBFTBL_ENTRY_TYPE_FLOWBFW   1

/** SBF weight */
typedef uint16            sbftbl_weight_t;
/** SBF MAC addr table element - version 1 only */
typedef struct ether_addr sbftbl_addr_t;

/** SBFTBL Header context */
/** Both versions use the same table; fields that are "not applicable"
 * for a version are to be set to 0.
 * An example of such fields is mactbl_offset field, which will be
 * set to 0 and not used for SBFv2
 */
struct sbftbl_header {
	uint32  version_code;           /* SBFTBL <version,release,patch> */
	uint16  num_entries;            /* # of SBFTBL entries */
	uint16  entry_type;             /* SBFTBL_ENTRY_TYPE_XXX type */
	uint16  bfwtbl_offset;          /* Offset of BFW table */
	uint16  mactbl_offset;          /* Offset of Station MAC table */
	uint16  signature;              /* SBFTBL signature */
	uint16  reserved;               /* Reserved */
} __sbftbl_aligned;
typedef struct sbftbl_header sbftbl_header_t;

/** SBFTBL version 1 (type: STAEABFW) HME AREA
 *  |----------------------------- -----------|
 *  | Offset |  Section   |    Description    |
 *  |----------------------------- -----------|
 *  | 00     |  HDR       |      Version      |
 *  | 04     |            |    N    |  size   |
 *  | 08     |            | offset1 | offset2 |
 *  | 0C     |            |signature|  rsvd   |
 *  |-----------------------------------------|
 *  | 10     |            |   BFW1  |  BFW2   |
 *  | 14     |  BFWTBL    |   BFW3  |  BFW4   |
 *  | ..     |            |   ....  |  ....   |
 *  | 16+2*N |            | BFW[N-1]| BFW[N]  |
 *  |-----------------------------------------|
 *  | +04    |            |       MAC1        |
 *  | +08    |            |   MAC1  |  MAC2   |
 *  | +0C    |            |       MAC2        |
 *  | +10    |            |       MAC3        |
 *  | +14    |   MACTBL   |   MAC3  |  MAC4   |
 *  | +18    |            |       MAC4        |
 *  | ...    |            |       ....        |
 *  | ...    |            |     MAC[N-1]      |
 *  | ...    |            | MAC[N-1]|  MAC[N] |
 *  | ...    |            |      MAC[N]       |
 *  |-----------------------------------------|
 */

/** SBFTBL version 2 (type: FLOWBFW) HME AREA
 *  |----------------------------- -----------|
 *  | Offset |  Section   |    Description    |
 *  |----------------------------- -----------|
 *  | 00     |  HDR       |      Version      |
 *  | 04     |            |    N    |  size   |
 *  | 08     |            | offset1 | offset2 |
 *  | 0C     |            |signature|  rsvd   |
 *  |-----------------------------------------|
 *  | 10     |            |   BFW1  |  BFW2   |
 *  | 14     |  BFWTBL    |   BFW3  |  BFW4   |
 *  | ..     | (N=1024)   |   ....  |  ....   |
 *  | 16+2*N |            | BFW[N-1]| BFW[N]  |
 *  |-----------------------------------------|
 */

#define SBFTBL_HEADER_LEN           16 /* bytes */
#define SBFTBL_WEIGHT_LEN           2 /* bytes */
#define SBFTBL_ADDRESS_LEN          6 /* bytes */
#define SBFTBL_STAEABFW_ENTRY_LEN   (SBFTBL_WEIGHT_LEN + SBFTBL_ADDRESS_LEN)
#define SBFTBL_FLOWBFW_ENTRY_LEN    (SBFTBL_WEIGHT_LEN)

#endif /* __bcm_sbftbl_h__ */
