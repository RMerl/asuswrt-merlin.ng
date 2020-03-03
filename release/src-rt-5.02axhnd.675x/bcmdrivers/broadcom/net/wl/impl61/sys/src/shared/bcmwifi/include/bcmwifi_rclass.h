/*
 * Common interface to channel definitions.
 * Copyright (C) 2019, Broadcom. All Rights Reserved.
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
 * $Id: bcmwifi_rclass.h 696710 2017-04-27 20:12:28Z $
 */

#ifndef _BCMWIFI_RCLASS_H_
#define _BCMWIFI_RCLASS_H_

#include <typedefs.h>
#include <bcmutils.h>
#include <bcmwifi_channels.h>
#include <wlioctl.h>

#define BCMWIFI_MAX_VEC_SIZE	(255/8)

/* Actual bandwidth in Annex E - x tables */
#define BCMWIFI_BW_20	20
#define BCMWIFI_BW_40	40
#define BCMWIFI_BW_80	80
#define BCMWIFI_BW_160	160

/*
 * Band constants representing op class channel starting
 * frequency in 500KHz units. i.e. 2 = 1 MHz, 10,000 = 5 GHz
 */
enum {
	BCMWIFI_BAND_2G = (2407 * 2),
	BCMWIFI_BAND_5G = (5000 * 2)
};

enum {
	BCMWIFI_RCLASS_CHAN_TYPE_CHAN_PRIMARY = 0,
	BCMWIFI_RCLASS_CHAN_TYPE_CNTR_FREQ = 1
};
typedef uint8 bcmwifi_rclass_chan_type_t;

/*
 * Constants for the domain of op classess defined in
 * IEEE 802.11REVmc_D8.0, Annex E.
 */
enum {
	BCMWIFI_RCLASS_TYPE_NONE = 0,
	BCMWIFI_RCLASS_TYPE_US = 1,	/* United States, Table E-1 */
	BCMWIFI_RCLASS_TYPE_EU = 2,	/* Europe, Table E-2 */
	BCMWIFI_RCLASS_TYPE_JP = 3,	/* Japan, Table E-3 */
	BCMWIFI_RCLASS_TYPE_GBL = 4,	/* Global, Table E-4 */
	BCMWIFI_RCLASS_TYPE_CH = 5	/* China, Table E-5 */
};
typedef uint8 bcmwifi_rclass_type_t;

enum {
	BCMWIFI_RCLASS_FLAGS_NONE = 0x0000,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_LOWER = 0x0001,
	BCMWIFI_RCLASS_FLAGS_PRIMARY_UPPER = 0x0002,
	BCMWIFI_RCLASS_FLAGS_EIRP = 0x0004,
	BCMWIFI_RCLASS_FLAGS_DFS = 0x0008,
	BCMWIFI_RCLASS_FLAGS_NOMADIC = 0x0010,
	BCMWIFI_RCLASS_FLAGS_LIC_EXMPT = 0x0020,
	BCMWIFI_RCLASS_FLAGS_80PLUS = 0x0040,
	BCMWIFI_RCLASS_FLAGS_ITS_MOB_OPS = 0x0080,
	BCMWIFI_RCLASS_FLAGS_ITS_NONMOB_OPS = 0x0100
};
typedef uint16 bcmwifi_rclass_flags_t;

/* an operating class number 0 - 255 */
typedef uint8 bcmwifi_rclass_opclass_t;
/* a channel number from 0 - 200 */
typedef uint8 bcmwifi_rclass_channel_t;
/* band represented as a Starting Channel frequency in 500KHz units */
typedef uint32 bcmwifi_rclass_band_t;
/* bandwidth in MHz */
typedef uint16 bcmwifi_rclass_bw_t;
/* an array of bcmwifi_rclass_channel_t */
typedef uint8 bcmwifi_rclass_chan_set_t;

/*
 * Op class table info
 */
typedef struct bcmwifi_rclass_info {
	bcmwifi_rclass_opclass_t rclass;	/* op class */
	bcmwifi_rclass_chan_type_t chan_type;	/* channel type */
	bcmwifi_rclass_bw_t bw;			/* bandwidth */
	bcmwifi_rclass_band_t band;		/* starting frequency */
	bcmwifi_rclass_flags_t flags;		/* behavior info */
	uint8 chan_set_len;			/* len of the list chan_set */
	const bcmwifi_rclass_chan_set_t *chan_set;	/* channel set or list of channels */
} bcmwifi_rclass_info_t;

/*
 * Operating class table bit vector of supported
 * op classes
 */
typedef struct bcmwifi_rclass_bvec {
	uint8 len;	/* length of bit vector array */
	uint8 bvec[0];	/* array of bits */
} bcmwifi_rclass_bvec_t;

/*
 * Return the operating class for a given chanspec
 * Input: chanspec
 *        type (op class type is US, EU, JP, GBL, or CH). Currently only GBL
 *        is supported.
 * Output: rclass (op class)
 * On success return status BCME_OK.
 * On error return status BCME_UNSUPPORTED, BCME_BADARG, BCME_ERROR.
 */
int bcmwifi_rclass_get_opclass(bcmwifi_rclass_type_t type, chanspec_t chanspec,
	bcmwifi_rclass_opclass_t *rclass);

/*
 * Return op class info (starting freq, bandwidth, sideband, channel set and behavior)
 * for given op class and type.
 * Input(s): type (op class type - US, EU, JP, GBL, or CH)
 *           rclass (op class)
 * Output: rcinfo (op class info entry)
 * Return status BCME_OK when no error. On error, BCME_ERROR.
 */
int bcmwifi_rclass_get_rclass_info(bcmwifi_rclass_type_t type,
	bcmwifi_rclass_opclass_t rclass, const bcmwifi_rclass_info_t **rcinfo);

/*
 * Return supported op class encoded as a bitvector.
 * Input: type (op class type US, EU, JP, GBL, or CH)
 *
 * Output: rclass_bvec
 * Return status BCME_OK when no error.
 * On error, return status BCME_ERROR, BCME_BADARG, BCME_UNSUPPORTED
 */
int bcmwifi_rclass_get_supported_opclasses(bcmwifi_rclass_type_t type,
	const bcmwifi_rclass_bvec_t **rclass_bvec);

/*
 * Return chanspec given op class type, op class and channel index
 * Input: type (op class type US, EU, JP, GBL, CH)
 *        rclass (op class)
 *        chn_idx (index in channel list)
 * Output: chanspec
 * Return BCME_OK on success and BCME_ERROR on error
 */
int bcmwifi_rclass_get_chanspec(bcmwifi_rclass_type_t type, bcmwifi_rclass_opclass_t rclass,
	uint8 chan_idx, chanspec_t *cspec);

/*
 * Input: country code abbreviation (abbrev)
 *        rclass (op class)
 * Output: Channel List (list)
 * Return BCME_OK on success and BCME_ERROR on error
 */

int
bcmwifi_rclass_get_chanlist(const char *abbrev, uint8 rclass,
	wl_uint32_list_t *list);
#endif /* _BCMWIFI_RCLASS_H */
