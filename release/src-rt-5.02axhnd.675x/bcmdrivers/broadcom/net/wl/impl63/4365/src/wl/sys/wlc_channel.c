/*
 * Common interface to channel definitions.
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: wlc_channel.c 788184 2020-06-23 17:50:17Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <proto/wpa.h>
#include <sbconfig.h>
#include <pcicfg.h>
#include <bcmsrom.h>
#include <wlioctl.h>
#include <epivers.h>
#ifdef BCMSUP_PSK
#include <proto/eapol.h>
#include <bcmwpa.h>
#endif /* BCMSUP_PSK */
#ifdef BCMCCX
#include <bcmcrypto/ccx.h>
#endif /* BCMCCX */
#include <bcmdevs.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wlc_phy_hal.h>
#include <phy_radar_api.h>
#include <phy_utils_api.h>
#include <phy_rxgcrs_api.h>
#include <wl_export.h>
#include <wlc_stf.h>
#include <wlc_channel.h>
#include "wlc_clm_data.h"
#include <wlc_11h.h>
#include <wlc_tpc.h>
#include <wlc_dfs.h>
#include <wlc_11d.h>
#include <wlc_cntry.h>
#include <wlc_prot_g.h>
#include <wlc_prot_n.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_reg.h>
#ifdef WLOFFLD
#include <wlc_offloads.h>
#endif /* WLOFFLD */
#ifdef WLOLPC
#include <wlc_olpc_engine.h>
#endif /* WLOLPC */
#include <wlc_ht.h>
#include <wlc_vht.h>
#include <wlc_objregistry.h>
#include <wlc_ulb.h>
#include <wlc_scb_ratesel.h>
#ifdef WL_MODESW
#include <wlc_modesw.h>
#endif /* WL_MODESW */
#include <wlc_scan.h>

enum {
	DLOAD_STATUS_DOWNLOAD_SUCCESS = 0,
	DLOAD_STATUS_DOWNLOAD_IN_PROGRESS = 1,
	DLOAD_STATUS_IOVAR_ERROR = 2,
	DLOAD_STATUS_BLOB_FORMAT = 3,
	DLOAD_STATUS_BLOB_HEADER_CRC = 4,
	DLOAD_STATUS_BLOB_NOMEM = 5,
	DLOAD_STATUS_BLOB_DATA_CRC = 6,
	DLOAD_STATUS_CLM_BLOB_FORMAT = 7,
	DLOAD_STATUS_CLM_MISMATCH = 8,
	DLOAD_STATUS_CLM_DATA_BAD = 9
};
typedef uint32 dload_error_status_t;

struct wl_segment {
	uint32 type;
	uint32 offset;
	uint32 length;
	uint32 crc32;
	uint32 flags;
};
typedef struct wl_segment wl_segment_t;

struct wl_segment_info {
	uint8        magic[4];
	uint32       hdr_len;
	uint32       crc32;
	uint32       file_type;
	uint32       num_segments;
	wl_segment_t segments[1];
};
typedef struct wl_segment_info wl_segment_info_t;

struct segment {
	uint32 type;
	uint8  *data;
	uint32 length;
};
typedef struct segment segment_t;

typedef struct wlc_blob_info {
	wlc_info_t *wlc;
	/* Parsing data */
	wl_segment_info_t *segment_info;	/* Initially NULL from bzero/init of structure */
	uint32            segment_info_malloc_size;

	uint32 blob_offset;
	uint32 blob_cur_segment;
	uint32 blob_cur_segment_crc32;

	/* Data that is present until final release by client */
	segment_t *segments;			/* Initially NULL from bzero/init of structure */
	uint32    segments_malloc_size;
	uint32    segment_count;
} wlc_blob_info_t;

typedef dload_error_status_t (*wlc_blob_download_complete_fn_t)(wlc_cm_info_t *wlc_cm,
	segment_t *segments, uint32 segment_count);
static wlc_blob_info_t *wlc_blob_attach(wlc_info_t *wlc);
static void wlc_blob_detach(wlc_blob_info_t *wbi);
static void wlc_blob_cleanup(wlc_blob_info_t *wbi);
static dload_error_status_t wlc_blob_download(wlc_blob_info_t *wbi, uint16 flag,
	uint8 *data, uint32 data_len,
	wlc_blob_download_complete_fn_t wlc_blob_download_complete_fn);
#ifdef WL_GLOBAL_RCLASS
static bool wlc_chk_rclass_support_5g(const rcinfo_t *rcinfo, uint8 *setBitBuff);
#endif /* WL_GLOBAL_RCLASS */

static wlc_blob_info_t *
BCMATTACHFN(wlc_blob_attach)(wlc_info_t *wlc)
{
	wlc_blob_info_t *wbi;

	if ((wbi = (wlc_blob_info_t *) MALLOCZ(wlc->pub->osh,
		sizeof(wlc_blob_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes", wlc->pub->unit,
			__FUNCTION__, MALLOCED(wlc->pub->osh)));
		return NULL;
	}
	wbi->wlc = wlc;
	return wbi;
}

static void
BCMATTACHFN(wlc_blob_detach)(wlc_blob_info_t *wbi)
{
	wlc_info_t *wlc;

	if (wbi) {
		wlc = wbi->wlc;
		wlc_blob_cleanup(wbi);
		MFREE(wlc->pub->osh, wbi, sizeof(wlc_blob_info_t));
	}
}

static void
wlc_blob_cleanup(wlc_blob_info_t *wbi)
{
	wlc_info_t *wlc;
	unsigned int i;

	ASSERT(wbi);
	wlc = wbi->wlc;

	if (wbi->segment_info) {
		if (wbi->segments) {

			for (i = 0; i < wbi->segment_count; i++) {
				if (wbi->segments[i].data != NULL)
					MFREE(wlc->pub->osh, wbi->segments[i].data,
						wbi->segments[i].length);
			}
			MFREE(wlc->pub->osh, wbi->segments, wbi->segments_malloc_size);
			wbi->segments = NULL;
		}
		MFREE(wlc->pub->osh, wbi->segment_info, wbi->segment_info_malloc_size);
		wbi->segment_info = NULL;
	}
}

static dload_error_status_t
wlc_blob_download(wlc_blob_info_t *wbi, uint16 flag, uint8 *data, uint32 data_len,
	wlc_blob_download_complete_fn_t wlc_blob_download_complete_fn)
{
	wlc_info_t *wlc;
	dload_error_status_t status = DLOAD_STATUS_DOWNLOAD_IN_PROGRESS;
	unsigned int i;

	ASSERT(wbi);
	wlc = wbi->wlc;

	if (flag & DL_BEGIN) {
		wl_segment_info_t *segment_info_cast_on_data;

		/* Clean up anything not finished */
		wlc_blob_cleanup(wbi);

		/* Setup for a new download stream */
		/* Make sure first chunk constains at least the full header, malloc a temporary
		 * copy of the header during parsing and then setup up parsing and start with any
		 * remaining data in this first chunk.
		 */
		segment_info_cast_on_data = (wl_segment_info_t *)data;

		WL_NONE(("%s: magic %4s data_len %d, cast hdr_len %d\n", __FUNCTION__,
			segment_info_cast_on_data->magic,
			data_len, segment_info_cast_on_data->hdr_len));

		if (data_len < sizeof(wl_segment_t) ||
			data_len < segment_info_cast_on_data->hdr_len) {
			status = DLOAD_STATUS_IOVAR_ERROR;
			goto exit;
		}

		/* Check blob magic string */
#define BLOB_LITERAL "BLOB"
		if (memcmp(data, BLOB_LITERAL, sizeof(BLOB_LITERAL) - 1) != 0) {
			status = DLOAD_STATUS_BLOB_FORMAT;
			goto exit;
		}

		/* Header crc32 check.  It starts with the field after the crc32, i.e. file type */
		{
			uint32 hdr_crc32;
			uint32 crc32_start_offset =
				OFFSETOF(wl_segment_info_t, crc32) + sizeof(hdr_crc32);

			hdr_crc32 = hndcrc32(data + crc32_start_offset,
				segment_info_cast_on_data->hdr_len - crc32_start_offset,
				0xffffffff);
			hdr_crc32 = hdr_crc32 ^ 0xffffffff;

			WL_NONE(("Validating header crc - expected %08x computed %08x\n",
				segment_info_cast_on_data->crc32,
				hdr_crc32));
			if (segment_info_cast_on_data->crc32 != hdr_crc32) {
				status = DLOAD_STATUS_BLOB_HEADER_CRC;
				goto exit;
			}
		}

		wbi->segment_info_malloc_size =  segment_info_cast_on_data->hdr_len;
		if ((wbi->segment_info = MALLOC(wlc->pub->osh, wbi->segment_info_malloc_size))
			== NULL) {
			status = DLOAD_STATUS_BLOB_NOMEM;
			goto exit;
		}
		memcpy(wbi->segment_info, data, wbi->segment_info_malloc_size);

		wbi->segments_malloc_size = wbi->segment_info->num_segments * sizeof(segment_t);
		if ((wbi->segments = MALLOC(wlc->pub->osh, wbi->segments_malloc_size)) == NULL) {
			status = DLOAD_STATUS_BLOB_NOMEM;
			goto exit;
		}

		/* Setup the client description of the segments while making sure the next
		 * segment offset isn't backward.  The segments must be in sequential order
		 * and non-overlapping.
		 */
		{
			uint32 segment_last_offset = wbi->segment_info_malloc_size;

			wbi->segment_count = 0;
			for (i = 0; i < wbi->segment_info->num_segments; i++) {
				if (wbi->segment_info->segments[i].offset < segment_last_offset) {
					status = DLOAD_STATUS_BLOB_FORMAT;
					goto exit;
				}
				segment_last_offset = wbi->segment_info->segments[i].offset +
					wbi->segment_info->segments[i].length;

				wbi->segments[i].type = wbi->segment_info->segments[i].type;
				wbi->segments[i].length = wbi->segment_info->segments[i].length;
				if ((wbi->segments[i].data = MALLOC(wlc->pub->osh,
					wbi->segments[i].length)) == NULL) {
					status = DLOAD_STATUS_BLOB_NOMEM;
					goto exit;
				}
				wbi->segment_count += 1;
			}
		}

		/* Don't check that the first segment offset is greater than the header size.
		 * This allows you to make the first segment all or part of the header if you
		 * wished.
		 */

		wbi->blob_offset = 0;
		wbi->blob_cur_segment = 0;
		wbi->blob_cur_segment_crc32 = 0xffffffff;

		{
			unsigned int j;

			WL_NONE(("Segment info header: "
				"magic %4s len %d crc32 %8x type %8x num_segments %d\n",
				wbi->segment_info->magic,
				wbi->segment_info->hdr_len,
				wbi->segment_info->crc32,
				wbi->segment_info->file_type,
				wbi->segment_info->num_segments));

			for (j = 0; j < wbi->segment_info->num_segments; j++) {
				WL_NONE(("  segment %3d - "
					"type %8d offset %10d length %10d crc32 %8x flags %8x\n",
					j,
					wbi->segment_info->segments[j].type,
					wbi->segment_info->segments[j].offset,
					wbi->segment_info->segments[j].length,
					wbi->segment_info->segments[j].crc32,
					wbi->segment_info->segments[j].flags));
			}
		}
	}

	/* Process the data after validating that we are in the middle of a parse */
	if (wbi->segment_info == NULL) {
		status = DLOAD_STATUS_IOVAR_ERROR;
		goto exit;
	} else {
		uint32 last_offset_of_chunk;
		uint32 amount_to_consume;

		/* Copy any bytes in the current chunk to the remaining segments */
		last_offset_of_chunk = wbi->blob_offset + data_len;
		while (data_len > 0) {
			/* Skip any bytes after the last segment */
			if (wbi->blob_cur_segment >= wbi->segment_count) {
				/* Equals should be sufficient */
				ASSERT(wbi->blob_cur_segment == wbi->segment_count);
				amount_to_consume = data_len;
			} else
			/* Skip any bytes before the next segment */
			if (wbi->segment_info->segments[wbi->blob_cur_segment].offset
				> wbi->blob_offset) {
				amount_to_consume =
					wbi->segment_info->segments[wbi->blob_cur_segment].offset
					- wbi->blob_offset;
				amount_to_consume = MIN(amount_to_consume, data_len);
			} else {
				/* Copy out any bytes to the segment */
				uint32 first_offset_to_copy;
				uint32 last_offset_to_copy;
				uint32 last_offset_of_segment;

				first_offset_to_copy = wbi->blob_offset;
				last_offset_of_segment =
					wbi->segment_info->segments[wbi->blob_cur_segment].offset +
					wbi->segment_info->segments[wbi->blob_cur_segment].length;
				last_offset_to_copy =
					MIN(last_offset_of_chunk, last_offset_of_segment);
				amount_to_consume = last_offset_to_copy - first_offset_to_copy;
				memcpy(wbi->segments[wbi->blob_cur_segment].data +
					(wbi->blob_offset -
					wbi->segment_info->segments[wbi->blob_cur_segment].offset),
					data,
					amount_to_consume);
				/* XXX TODO - Optimize so the memcopy and crc can be done in the
				 * same pass.
				 */
				wbi->blob_cur_segment_crc32 = hndcrc32(data,
					amount_to_consume, wbi->blob_cur_segment_crc32);

				/* Advance to the next segment if we copied the last byte */
				if (last_offset_to_copy >= last_offset_of_segment) {
					/* validate the crc */
					wbi->blob_cur_segment_crc32 ^= 0xffffffff;
					WL_NONE(("Validating crc - "
						"segment %d expected %08x computed %08x\n",
						wbi->blob_cur_segment,
						wbi->segment_info->
						segments[wbi->blob_cur_segment].crc32,
						wbi->blob_cur_segment_crc32));
					if (wbi->segment_info->segments[wbi->blob_cur_segment].crc32
						!= wbi->blob_cur_segment_crc32) {
						status = DLOAD_STATUS_BLOB_DATA_CRC;
						goto exit;
					}

					/* increment the current segment segment counter */
					wbi->blob_cur_segment += 1;

					/* reset the crc32 for the next segment */
					wbi->blob_cur_segment_crc32 = 0xffffffff;
				}
			}

			WL_NONE(("wbi->blob_offset %10d wbi->blob_cur_segment %10d "
				"data_len %10d amount_to_comsume %10d\n",
				wbi->blob_offset, wbi->blob_cur_segment,
				data_len, amount_to_consume));

			wbi->blob_offset += amount_to_consume;
			data += amount_to_consume;
			data_len -= amount_to_consume;
		}
	}

	if (flag & DL_END) {
		if (wbi->blob_cur_segment != wbi->segment_count) {
			/* We didn't get all of the segments */
			status = DLOAD_STATUS_BLOB_FORMAT;
			goto exit;
		} else {
			/* Clean up parse data */
			ASSERT(wbi->segment_info != NULL);
			MFREE(wlc->pub->osh, wbi->segment_info, wbi->segment_info_malloc_size);
			wbi->segment_info = NULL;

			/* Client now uses segments */
			ASSERT(wbi->segments != NULL);

			/* Download complete. Install the new data */
			status = wlc_blob_download_complete_fn(wlc->cmi, wbi->segments,
				wbi->segment_count);

			/* Free up any segment data that wasn't "claimed" by the client and then
			 * the segments allocation which only exists during the parse.
			 */
			for (i = 0; i < wbi->segment_count; i++) {
				if (wbi->segments[i].data != NULL)
					MFREE(wlc->pub->osh, wbi->segments[i].data,
						wbi->segments[i].length);
			}
			MFREE(wlc->pub->osh, wbi->segments, wbi->segments_malloc_size);
			wbi->segments = NULL;
		}
	}

exit:
	if (status != DLOAD_STATUS_DOWNLOAD_IN_PROGRESS &&
	    status != DLOAD_STATUS_DOWNLOAD_SUCCESS) {
		wlc_blob_cleanup(wbi);
	}

	return status;
}

typedef struct wlc_cm_band {
	uint16		locale_flags;		/* locale_info_t flags */
	chanvec_t	valid_channels;		/* List of valid channels in the country */
	chanvec_t	*radar_channels;	/* List of radar sensitive channels */
	struct wlc_channel_txchain_limits chain_limits;	/* per chain power limit */
	uint8		PAD[4];
} wlc_cm_band_t;

typedef struct wlc_cm_data {
	char		srom_ccode[WLC_CNTRY_BUF_SZ];	/* Country Code in SROM */
	uint		srom_regrev;			/* Regulatory Rev for the SROM ccode */
	clm_country_t country;			/* Current country iterator for the CLM data */
	char		ccode[WLC_CNTRY_BUF_SZ];	/* current internal Country Code */
	uint		regrev;				/* current Regulatory Revision */
	char		country_abbrev[WLC_CNTRY_BUF_SZ];	/* current advertised ccode */
	wlc_cm_band_t	bandstate[MAXBANDS];	/* per-band state (one per phy/radio) */
	/* quiet channels currently for radar sensitivity or 11h support */
	chanvec_t	quiet_channels;		/* channels on which we cannot transmit */

	struct clm_data_header* clm_base_dataptr;
	int clm_base_data_len;

	void* clm_inc_dataptr;
	void* clm_inc_headerptr;
	int clm_inc_data_len;

	/* List of radar sensitive channels for the current locale */
	chanvec_t locale_radar_channels;

	/* restricted channels */
	chanvec_t	restricted_channels; 	/* copy of the global restricted channels of the */
						/* current local */
	bool		has_restricted_ch;

	/* regulatory class */
	rcvec_t		valid_rcvec;		/* List of valid regulatory class in the country */
	const rcinfo_t	*rcinfo_list[MAXRCLIST];	/* regulatory class info list */
	bool		bandedge_filter_apply;
	bool		sar_enable;		/* Use SAR as part of regulatory power calc */
#ifdef WL_SARLIMIT
	sar_limit_t	sarlimit;		/* sar limit per band/sub-band */
#endif // endif
	/* List of valid regulatory class in the country */
	chanvec_t	allowed_5g_20channels;	/* List of valid 20MHz channels in the country */
	chanvec_t	allowed_5g_4080channels;	/* List of valid 40 and 80MHz channels */

	uint32		clmload_status;		/* detailed clmload status */
	wlc_blob_info_t *clmload_wbi;
	const rcinfo_t *rcinfo_list_11ac[MAXRCLIST_REG];
#ifdef WL_GLOBAL_RCLASS
	uint8		cur_rclass;		/* current operating class, country or global */
#endif /* WL_GLOBAL_RCLASS */
} wlc_cm_data_t;

struct wlc_cm_info {
	wlc_pub_t	*pub;
	wlc_info_t	*wlc;
	wlc_cm_data_t	*cm;
};

static dload_error_status_t wlc_handle_clm_dload(wlc_cm_info_t *wlc_cm,
	segment_t *segments, uint32 segment_count);

static int wlc_channels_init(wlc_cm_info_t *wlc_cm, clm_country_t country);
static void wlc_set_country_common(
	wlc_cm_info_t *wlc_cmi, const char* country_abbrev, const char* ccode, uint regrev,
	clm_country_t country);
static int wlc_country_aggregate_map(
	wlc_cm_info_t *wlc_cmi, const char *ccode, char *mapped_ccode, uint *mapped_regrev);
static clm_result_t wlc_countrycode_map(wlc_cm_info_t *wlc_cmi, const char *ccode,
	char *mapped_ccode, uint *mapped_regrev, clm_country_t *country);
static void wlc_channels_commit(wlc_cm_info_t *wlc_cmi);
static void wlc_chanspec_list(wlc_info_t *wlc, wl_uint32_list_t *list, chanspec_t chanspec_mask);
static bool wlc_buffalo_map_locale(struct wlc_info *wlc, const char* abbrev);
static bool wlc_japan_ccode(const char *ccode);
static bool wlc_us_ccode(const char *ccode);
static void wlc_regclass_vec_init(wlc_cm_info_t *wlc_cmi);
static void wlc_upd_restricted_chanspec_flag(wlc_cm_info_t *wlc_cmi);
static int wlc_channel_update_txchain_offsets(wlc_cm_info_t *wlc_cmi, ppr_t *txpwr);
static void wlc_channel_set_radar_chanvect(wlc_cm_data_t *wlc_cm, wlcband_t *band, uint16 flags);

/* IE mgmt callbacks */
#ifdef WLTDLS
static uint wlc_channel_tdls_calc_rc_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_channel_tdls_write_rc_ie(void *ctx, wlc_iem_build_data_t *build);
#endif // endif

#if defined(WL_SARLIMIT) && (defined(BCMDBG) || defined(BCMDBG_DUMP))
static void wlc_channel_sarlimit_dump(wlc_cm_info_t *wlc_cmi, sar_limit_t *sar);
#endif /* WL_SARLIMIT && (BCMDBG || BCMDBG_DUMP) */
static void wlc_channel_spurwar_locale(wlc_cm_info_t *wlc_cm, chanspec_t chanspec);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_channel_dump_reg_ppr(void *handle, struct bcmstrbuf *b);
static int wlc_channel_dump_reg_local_ppr(void *handle, struct bcmstrbuf *b);
static int wlc_channel_dump_srom_ppr(void *handle, struct bcmstrbuf *b);
static int wlc_channel_dump_margin(void *handle, struct bcmstrbuf *b);
static int wlc_channel_init_ccode(wlc_cm_info_t *wlc_cmi, char* country_abbrev, int ca_len);

static int wlc_dump_max_power_per_channel(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b);
static int wlc_dump_clm_limits_2G_20M(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b);
static int wlc_dump_clm_limits_2G_40M(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b);
static int wlc_dump_clm_limits_2G_20in40M(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b);
static int wlc_dump_clm_limits_5G_20M(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b);
static int wlc_dump_clm_limits_5G_40M(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b);
static int wlc_dump_clm_limits_5G_20in40M(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b);

static int wlc_dump_country_aggregate_map(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b);
static int wlc_channel_supported_country_regrevs(void *handle, struct bcmstrbuf *b);

static bool wlc_channel_clm_chanspec_valid(wlc_cm_info_t *wlc_cmi, chanspec_t chspec);

const char fraction[4][4] = {"   ", ".25", ".5 ", ".75"};
#define QDB_FRAC(x)	(x) / WLC_TXPWR_DB_FACTOR, fraction[(x) % WLC_TXPWR_DB_FACTOR]
#define QDB_FRAC_TRUNC(x)	(x) / WLC_TXPWR_DB_FACTOR, \
	((x) % WLC_TXPWR_DB_FACTOR) ? fraction[(x) % WLC_TXPWR_DB_FACTOR] : ""
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */

#define	COPY_LIMITS(src, index, dst, cnt)	\
		bcopy(&src.limit[index], txpwr->dst, cnt)
#define	COPY_DSSS_LIMS(src, index, dst)	\
		bcopy(&src.limit[index], txpwr->dst, WL_NUM_RATES_CCK)
#define	COPY_OFDM_LIMS(src, index, dst)	\
		bcopy(&src.limit[index], txpwr->dst, WL_NUM_RATES_OFDM)
#define	COPY_MCS_LIMS(src, index, dst)	\
		bcopy(&src.limit[index], txpwr->dst, WL_NUM_RATES_MCS_1STREAM)
#ifdef WL11AC
#define	COPY_VHT_LIMS(src, index, dst)	\
		bcopy(&src.limit[index], txpwr->dst, WL_NUM_RATES_EXTRA_VHT)
#else
#define	COPY_VHT_LIMS(src, index, dst)
#endif // endif

#define CLM_DSSS_RATESET(src) ((const ppr_dsss_rateset_t*)&src.limit[WL_RATE_1X1_DSSS_1])
#define CLM_OFDM_1X1_RATESET(src) ((const ppr_ofdm_rateset_t*)&src.limit[WL_RATE_1X1_OFDM_6])
#define CLM_MCS_1X1_RATESET(src) ((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_1X1_MCS0])

#define CLM_DSSS_1X2_MULTI_RATESET(src) \
	((const ppr_dsss_rateset_t*)&src.limit[WL_RATE_1X2_DSSS_1])
#define CLM_OFDM_1X2_CDD_RATESET(src) \
	((const ppr_ofdm_rateset_t*)&src.limit[WL_RATE_1X2_CDD_OFDM_6])
#define CLM_MCS_1X2_CDD_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_1X2_CDD_MCS0])

#define CLM_DSSS_1X3_MULTI_RATESET(src) \
	((const ppr_dsss_rateset_t*)&src.limit[WL_RATE_1X3_DSSS_1])
#define CLM_OFDM_1X3_CDD_RATESET(src) \
	((const ppr_ofdm_rateset_t*)&src.limit[WL_RATE_1X3_CDD_OFDM_6])
#define CLM_MCS_1X3_CDD_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_1X3_CDD_MCS0])

#define CLM_DSSS_1X4_MULTI_RATESET(src) \
	((const ppr_dsss_rateset_t*)&src.limit[WL_RATE_1X4_DSSS_1])
#define CLM_OFDM_1X4_CDD_RATESET(src) \
	((const ppr_ofdm_rateset_t*)&src.limit[WL_RATE_1X4_CDD_OFDM_6])
#define CLM_MCS_1X4_CDD_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_1X4_CDD_MCS0])

#define CLM_MCS_2X2_SDM_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_2X2_SDM_MCS8])
#define CLM_MCS_2X2_STBC_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_2X2_STBC_MCS0])

#define CLM_MCS_2X3_STBC_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_2X3_STBC_MCS0])
#define CLM_MCS_2X3_SDM_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_2X3_SDM_MCS8])

#define CLM_MCS_2X4_STBC_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_2X4_STBC_MCS0])
#define CLM_MCS_2X4_SDM_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_2X4_SDM_MCS8])

#define CLM_MCS_3X3_SDM_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_3X3_SDM_MCS16])
#define CLM_MCS_3X4_SDM_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_3X4_SDM_MCS16])

#define CLM_MCS_4X4_SDM_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_4X4_SDM_MCS24])

#define CLM_OFDM_1X2_TXBF_RATESET(src) \
	((const ppr_ofdm_rateset_t*)&src.limit[WL_RATE_1X2_TXBF_OFDM_6])
#define CLM_MCS_1X2_TXBF_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_1X2_TXBF_MCS0])
#define CLM_OFDM_1X3_TXBF_RATESET(src) \
	((const ppr_ofdm_rateset_t*)&src.limit[WL_RATE_1X3_TXBF_OFDM_6])
#define CLM_MCS_1X3_TXBF_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_1X3_TXBF_MCS0])
#define CLM_OFDM_1X4_TXBF_RATESET(src) \
	((const ppr_ofdm_rateset_t*)&src.limit[WL_RATE_1X4_TXBF_OFDM_6])
#define CLM_MCS_1X4_TXBF_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_1X4_TXBF_MCS0])
#define CLM_MCS_2X2_TXBF_RATESET(src) \
	((const ppr_ht_mcs_rateset_t*)&src.limit[WL_RATE_2X2_TXBF_SDM_MCS8])
#define CLM_MCS_2X3_TXBF_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_2X3_TXBF_SDM_MCS8])
#define CLM_MCS_2X4_TXBF_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_2X4_TXBF_SDM_MCS8])
#define CLM_MCS_3X3_TXBF_RATESET(src) \
	((const ppr_ht_mcs_rateset_t*)&src.limit[WL_RATE_3X3_TXBF_SDM_MCS16])
#define CLM_MCS_3X4_TXBF_RATESET(src) \
	((const ppr_vht_mcs_rateset_t*)&src.limit[WL_RATE_3X4_TXBF_SDM_MCS16])
#define CLM_MCS_4X4_TXBF_RATESET(src) \
	((const ppr_ht_mcs_rateset_t*)&src.limit[WL_RATE_4X4_TXBF_SDM_MCS24])

#if defined WLTXPWR_CACHE && defined(WLC_LOW) && defined(WL11N)
static chanspec_t last_chanspec = 0;
#endif /* WLTXPWR_CACHE */

clm_result_t clm_aggregate_country_lookup(const ccode_t cc, unsigned int rev,
	clm_agg_country_t *agg);
clm_result_t clm_aggregate_country_map_lookup(const clm_agg_country_t agg,
	const ccode_t target_cc, unsigned int *rev);

static clm_result_t clm_power_limits(
	const clm_country_locales_t *locales, clm_band_t band,
	unsigned int chan, int ant_gain, clm_limits_type_t limits_type,
	const clm_limits_params_t *params, clm_power_limits_t *limits);

/* QDB() macro takes a dB value and converts to a quarter dB value */
#ifdef QDB
#undef QDB
#endif // endif
#define QDB(n) ((n) * WLC_TXPWR_DB_FACTOR)

/* Regulatory Matrix Spreadsheet (CLM) MIMO v3.8.6.4
 * + CLM v4.1.3
 * + CLM v4.2.4
 * + CLM v4.3.1 (Item-1 only EU/9 and Q2/4).
 * + CLM v4.3.4_3x3 changes(Skip changes for a13/14).
 * + CLMv 4.5.3_3x3 changes for Item-5(Cisco Evora (change AP3500i to Evora)).
 * + CLMv 4.5.3_3x3 changes for Item-3(Create US/61 for BCM94331HM, based on US/53 power levels).
 * + CLMv 4.5.5 3x3 (changes from Create US/63 only)
 * + CLMv 4.4.4 3x3 changes(Create TR/4 (locales Bn7, 3tn), EU/12 (locales 3s, 3sn) for Airties.)
 */

/*
 * Some common channel sets
 */

/* All 2.4 GHz HW channels */
const chanvec_t chanvec_all_2G = {
	{0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00}
};

/* All 5 GHz HW channels */
const chanvec_t chanvec_all_5G = {
	/* 35,36,38/40,42,44,46/48,52/56,60 */
	{0x00, 0x00, 0x00, 0x00, 0x54, 0x55, 0x11, 0x11,
	/* 64/-/-/-/100/104,108/112,116/120,124 */
	0x01, 0x00, 0x00, 0x00, 0x10, 0x11, 0x11, 0x11,
#ifdef WL11AC
	/* /128,132/136,140/144,149/153,157/161,165... */
	0x11, 0x11, 0x21, 0x22, 0x22, 0x00, 0x00, 0x11,
#else
	/* /128,132/136,140/149/153,157/161,165... */
	0x11, 0x11, 0x20, 0x22, 0x22, 0x00, 0x00, 0x11,
#endif // endif
	0x11, 0x11, 0x11, 0x01}
};

/*
 * Radar channel sets
 */

#ifdef BAND5G
static const chanvec_t radar_set1 = { /* Channels 52 - 64, 100 - 144 */
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x11,	/* 52 - 60 */
	0x01, 0x00, 0x00, 0x00, 0x10, 0x11, 0x11, 0x11,		/* 64, 100 - 124 */
	0x11, 0x11, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,		/* 128 - 144 */
	0x00, 0x00, 0x00, 0x00}
};

/* Channels 149-165 are treated as radar channels as per new UK DFS requirements.
 * EN302 502 and EN301 893 Channel 144 is also available.
 */
static const chanvec_t radar_set_uk = { /* Channels 52 - 64, 100 - 144 */
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x11,	/* 52 - 60 */
	0x01, 0x00, 0x00, 0x00, 0x10, 0x11, 0x11, 0x11,		/* 64, 100 - 124 */
	0x11, 0x11, 0x21, 0x22, 0x22, 0x00, 0x00, 0x00,		/* 128 - 144, 149 - 165 */
	0x00, 0x00, 0x00, 0x00}
};
#endif	/* BAND5G */

/*
 * Restricted channel sets
 */

#define WLC_REGCLASS_USA_2G_20MHZ	12
#define WLC_REGCLASS_EUR_2G_20MHZ	4
#define WLC_REGCLASS_JPN_2G_20MHZ	30
#define WLC_REGCLASS_JPN_2G_20MHZ_CH14	31
#define WLC_REGCLASS_GLOBAL_2G_20MHZ		81
#define WLC_REGCLASS_GLOBAL_2G_20MHZ_CH14	82

#ifdef BAND5G
/*
 * channel to Global regulatory class map
 */
static const rcinfo_t rcinfo_global_20 = {
	24,
	{
	{ 36, 115}, { 40, 115}, { 44, 115}, { 48, 115}, {52, 118}, {56, 118}, {60, 118},
	{64, 118}, {100, 121}, {104, 121}, { 108, 121}, {112, 121}, {116, 121}, {120, 121},
	{124, 121}, {128, 121}, {132,  121}, {136, 121}, {140, 121}, {149, 125},
	{ 153, 125}, {157, 125}, {161, 125}, {165, 125},
	}
};
#endif /* BAND5G */

#ifdef WL11N
/* control channel at lower sb */
static const rcinfo_t rcinfo_global_40lower = {
	21,
	{
	{1, 83}, {2, 83}, {3, 83}, {4, 83}, {5, 83}, {6, 83}, {7, 83}, {8, 83}, {9, 83},
	{ 36,  116}, {44,  116}, {52, 119}, {60,  119}, {100, 122}, {108, 122}, {116, 122},
	{124, 122}, {132, 122}, {140, 122}, {149, 126}, {157, 126}, {0,  0}, {0,  0},
	{0,  0},
	}
};
/* control channel at upper sb */
static const rcinfo_t rcinfo_global_40upper = {
	21,
	{
	{5, 84}, {6, 84}, {7, 84}, {8, 84}, {9, 84}, {10, 84}, {11, 84}, {12, 84}, {13, 84},
	{40, 117}, { 48, 117}, {56, 120}, {64, 120}, {104, 123}, {112, 123}, {120, 123},
	{128, 123}, {136, 123}, {144, 123}, {153, 127}, {161, 127}, {  0,  0}, {  0,  0},
	{0,  0}
	}
};
#endif /* WL11N */

#ifdef WL11AC
/* center channel at 80MHZ */
static const rcinfo_t rcinfo_global_center_80 = {
	6,
	{
	{ 42, 128}, { 58, 128}, {106, 128}, {122, 128}, {138, 128}, {155, 128}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}
	}
};
#endif /* WL11AC */

#ifdef WL11AC_160
/* center channel at 160MHZ */
static const rcinfo_t rcinfo_global_center_160 = {
	2,
	{
	{ 50, 129}, {114, 129}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}
	}
};
#endif /* WL11AC_160 */

#ifdef BAND5G
/*
 * channel to regulatory class map for USA
 */
static const rcinfo_t rcinfo_us_20 = {
	24,
	{
	{ 36,  1}, { 40,  1}, { 44,  1}, { 48,  1}, { 52,  2}, { 56,  2}, { 60,  2}, { 64,  2},
	{100,  4}, {104,  4}, {108,  4}, {112,  4}, {116,  4}, {120,  4}, {124,  4}, {128,  4},
	{132,  4}, {136,  4}, {140,  4}, {149,  3}, {153,  3}, {157,  3}, {161,  3}, {165,  5}
	}
};
#endif /* BAND5G */

#ifdef WL11N
/* control channel at lower sb */
static const rcinfo_t rcinfo_us_40lower = {
	19,
	{
	{  1, 32}, {  2, 32}, {  3, 32}, {  4, 32}, {  5, 32}, {  6, 32}, {  7, 32}, { 36, 22},
	{ 44, 22}, { 52, 23}, { 60, 23}, {100, 24}, {108, 24}, {116, 24}, {124, 24}, {132, 24},
	{140, 24}, {149, 25}, {157, 25}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}
	}
};
/* control channel at upper sb */
static const rcinfo_t rcinfo_us_40upper = {
	19,
	{
	{  5, 33}, {  6, 33}, {  7, 33}, {  8, 33}, {  9, 33}, { 10, 33}, { 11, 33}, { 40, 27},
	{ 48, 27}, { 56, 28}, { 64, 28}, {104, 29}, {112, 29}, {120, 29}, {128, 29}, {136, 29},
	{144, 29}, {153, 30}, {161, 30}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}
	}
};
#endif /* WL11N */

#ifdef WL11AC
/* center channel at 80MHZ */
static const rcinfo_t rcinfo_us_center_80 = {
	6,
	{
	{ 42, 128}, { 58, 128}, {106, 128}, {122, 128}, {138, 128}, {155, 128}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}
	}
};
#endif /* WL11AC */

#ifdef WL11AC_160
/* center channel at 160MHZ */
static const rcinfo_t rcinfo_us_center_160 = {
	2,
	{
	{ 50, 129}, {114, 129}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}
	}
};
#endif /* WL11AC_160 */

#ifdef BAND5G
/*
 * channel to regulatory class map for Europe
 */
static const rcinfo_t rcinfo_eu_20 = {
	19,
	{
	{ 36,  1}, { 40,  1}, { 44,  1}, { 48,  1}, { 52,  2}, { 56,  2}, { 60,  2}, { 64,  2},
	{100,  3}, {104,  3}, {108,  3}, {112,  3}, {116,  3}, {120,  3}, {124,  3}, {128,  3},
	{132,  3}, {136,  3}, {140,  3}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}
	}
};
#endif /* BAND5G */

#ifdef WL11N
static const rcinfo_t rcinfo_eu_40lower = {
	18,
	{
	{  1, 11}, {  2, 11}, {  3, 11}, {  4, 11}, {  5, 11}, {  6, 11}, {  7, 11}, {  8, 11},
	{  9, 11}, { 36,  5}, { 44,  5}, { 52,  6}, { 60,  6}, {100,  7}, {108,  7}, {116,  7},
	{124,  7}, {132,  7}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}
	}
};
static const rcinfo_t rcinfo_eu_40upper = {
	18,
	{
	{  5, 12}, {  6, 12}, {  7, 12}, {  8, 12}, {  9, 12}, { 10, 12}, { 11, 12}, { 12, 12},
	{ 13, 12}, { 40,  8}, { 48,  8}, { 56,  9}, { 64,  9}, {104, 10}, {112, 10}, {120, 10},
	{128, 10}, {136, 10}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}
	}
};
#endif /* WL11N */

#ifdef WL11AC
/* center channel at 80MHZ */
static const rcinfo_t rcinfo_eu_center_80 = {
	4,
	{
	{ 42, 128}, { 58, 128}, {106, 128}, {122, 128}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}
	}
};
#endif /* WL11AC */

#ifdef WL11AC_160
/* center channel at 160MHZ */
static const rcinfo_t rcinfo_eu_center_160 = {
	2,
	{
	{ 50, 129}, {114, 129}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}
	}
};
#endif /* WL11AC_160 */

#ifdef BAND5G
/*
 * channel to regulatory class map for Japan
 */
static const rcinfo_t rcinfo_jp_20 = {
	8,
	{
	{ 34,  1}, { 38,  1}, { 42,  1}, { 46,  1}, { 52, 32}, { 56, 32}, { 60, 32}, { 64, 32},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}
	}
};
#endif /* BAND5G */

#ifdef WL11N
static const rcinfo_t rcinfo_jp_40 = {
	0,
	{
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}
	}
};
#endif /* WL11N */

#ifdef WL11AC
/* center channel at 80MHZ */
static const rcinfo_t rcinfo_jp_center_80 = {
	4,
	{
	{ 42, 128}, { 58, 128}, {106, 128}, {122, 128}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}
	}
};
#endif /* WL11Ac */

#ifdef WL11AC_160
/* center channel at 160MHZ */
static const rcinfo_t rcinfo_jp_center_160 = {
	2,
	{
	{ 50, 129}, {114, 129}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0},
	{  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}, {  0,  0}
	}
};
#endif /* WL11AC_160 */

/* iovar table */
enum {
	IOV_RCLASS = 1, /* read rclass */
	IOV_CLMLOAD = 2,
	IOV_CLMLOAD_STATUS = 3,
	IOV_DIS_CH_GRP				= 4,
	IOV_DIS_CH_GRP_CONF			= 5,
	IOV_DIS_CH_GRP_USER			= 6,
	IOV_IS_EDCRS_EU                         = 7,
	IOV_LAST
};

static const bcm_iovar_t cm_iovars[] = {
	{"rclass", IOV_RCLASS, 0, IOVT_UINT16, 0},
	{"clmload", IOV_CLMLOAD, IOVF_SET_DOWN, IOVT_BUFFER, 0},
	{"clmload_status", IOV_CLMLOAD_STATUS, 0, IOVT_UINT32, 0},
	{"dis_ch_grp", IOV_DIS_CH_GRP, 0, IOVT_UINT32, 0},
	{"dis_ch_grp_conf", IOV_DIS_CH_GRP_CONF, 0, IOVT_UINT32, 0},
	{"dis_ch_grp_user", IOV_DIS_CH_GRP_USER, (IOVF_SET_DOWN), IOVT_UINT32, 0},
	{"is_edcrs_eu", IOV_IS_EDCRS_EU, 0, IOVT_BOOL, 0},
	{NULL, 0, 0, 0, 0}
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

clm_result_t
wlc_locale_get_channels(clm_country_locales_t *locales, clm_band_t band,
	chanvec_t *channels, chanvec_t *restricted)
{
	bzero(channels, sizeof(chanvec_t));
	bzero(restricted, sizeof(chanvec_t));

	return clm_country_channels(locales, band, (clm_channels_t *)channels,
		(clm_channels_t *)restricted);
}

clm_result_t wlc_get_flags(clm_country_locales_t *locales, clm_band_t band, uint16 *flags)
{
	unsigned long clm_flags = 0;

	clm_result_t result = clm_country_flags(locales, band, &clm_flags);

	*flags = 0;
	if (result == CLM_RESULT_OK) {
		switch (clm_flags & CLM_FLAG_DFS_MASK) {
		case CLM_FLAG_DFS_UK:
			*flags |= WLC_DFS_UK;
			break;
		case CLM_FLAG_DFS_EU:
			*flags |= WLC_DFS_EU;
			break;
		case CLM_FLAG_DFS_US:
			*flags |= WLC_DFS_FCC;
			break;
		case CLM_FLAG_DFS_NONE:
		default:
			break;
		}

		if (clm_flags & CLM_FLAG_EDCRS_EU) {
			*flags |= WLC_EDCRS_EU;
		}

		if (clm_flags & CLM_FLAG_FILTWAR1)
			*flags |= WLC_FILT_WAR;

		if (clm_flags & CLM_FLAG_TXBF)
			*flags |= WLC_TXBF;

		if (clm_flags & CLM_FLAG_NO_MIMO)
			*flags |= WLC_NO_MIMO;
		else {
			if (clm_flags & CLM_FLAG_NO_40MHZ)
				*flags |= WLC_NO_40MHZ;
			if (clm_flags & CLM_FLAG_NO_80MHZ)
				*flags |= WLC_NO_80MHZ;
			if (clm_flags & CLM_FLAG_NO_160MHZ)
				*flags |= WLC_NO_160MHZ;
		}

		if ((band == CLM_BAND_2G) && (clm_flags & CLM_FLAG_HAS_DSSS_EIRP))
			*flags |= WLC_EIRP;
		if ((band == CLM_BAND_5G) && (clm_flags & CLM_FLAG_HAS_OFDM_EIRP))
			*flags |= WLC_EIRP;
	}

	return result;
}

clm_result_t wlc_get_locale(clm_country_t country, clm_country_locales_t *locales)
{
	return clm_country_def(country, locales);
}

/* 20MHz channel info for 40MHz pairing support */

struct chan20_info {
	uint8	sb;
	uint8	adj_sbs;
};

/* indicates adjacent channels that are allowed for a 40 Mhz channel and
 * those that permitted by the HT
 */
const struct chan20_info chan20_info[] = {
	/* 11b/11g */
/* 0 */		{1,	(CH_UPPER_SB | CH_EWA_VALID)},
/* 1 */		{2,	(CH_UPPER_SB | CH_EWA_VALID)},
/* 2 */		{3,	(CH_UPPER_SB | CH_EWA_VALID)},
/* 3 */		{4,	(CH_UPPER_SB | CH_EWA_VALID)},
/* 4 */		{5,	(CH_UPPER_SB | CH_LOWER_SB | CH_EWA_VALID)},
/* 5 */		{6,	(CH_UPPER_SB | CH_LOWER_SB | CH_EWA_VALID)},
/* 6 */		{7,	(CH_UPPER_SB | CH_LOWER_SB | CH_EWA_VALID)},
/* 7 */		{8,	(CH_UPPER_SB | CH_LOWER_SB | CH_EWA_VALID)},
/* 8 */		{9,	(CH_UPPER_SB | CH_LOWER_SB | CH_EWA_VALID)},
/* 9 */		{10,	(CH_LOWER_SB | CH_EWA_VALID)},
/* 10 */	{11,	(CH_LOWER_SB | CH_EWA_VALID)},
/* 11 */	{12,	(CH_LOWER_SB)},
/* 12 */	{13,	(CH_LOWER_SB)},
/* 13 */	{14,	(CH_LOWER_SB)},

/* 11a japan high */
/* 14 */	{34,	(CH_UPPER_SB)},
/* 15 */	{38,	(CH_LOWER_SB)},
/* 16 */	{42,	(CH_LOWER_SB)},
/* 17 */	{46,	(CH_LOWER_SB)},

/* 11a usa low */
/* 18 */	{36,	(CH_UPPER_SB | CH_EWA_VALID)},
/* 19 */	{40,	(CH_LOWER_SB | CH_EWA_VALID)},
/* 20 */	{44,	(CH_UPPER_SB | CH_EWA_VALID)},
/* 21 */	{48,	(CH_LOWER_SB | CH_EWA_VALID)},
/* 22 */	{52,	(CH_UPPER_SB | CH_EWA_VALID)},
/* 23 */	{56,	(CH_LOWER_SB | CH_EWA_VALID)},
/* 24 */	{60,	(CH_UPPER_SB | CH_EWA_VALID)},
/* 25 */	{64,	(CH_LOWER_SB | CH_EWA_VALID)},

/* 11a Europe */
/* 26 */	{100,	(CH_UPPER_SB | CH_EWA_VALID)},
/* 27 */	{104,	(CH_LOWER_SB | CH_EWA_VALID)},
/* 28 */	{108,	(CH_UPPER_SB | CH_EWA_VALID)},
/* 29 */	{112,	(CH_LOWER_SB | CH_EWA_VALID)},
/* 30 */	{116,	(CH_UPPER_SB | CH_EWA_VALID)},
/* 31 */	{120,	(CH_LOWER_SB | CH_EWA_VALID)},
/* 32 */	{124,	(CH_UPPER_SB | CH_EWA_VALID)},
/* 33 */	{128,	(CH_LOWER_SB | CH_EWA_VALID)},
/* 34 */	{132,	(CH_UPPER_SB | CH_EWA_VALID)},
/* 35 */	{136,	(CH_LOWER_SB | CH_EWA_VALID)},

#ifdef WL11AC
/* 36 */	{140,   (CH_UPPER_SB | CH_EWA_VALID)},
/* 37 */	{144,   (CH_LOWER_SB)},

/* 11a usa high, ref5 only */
/* The 0x80 bit in pdiv means these are REF5, other entries are REF20 */
/* 38 */	{149,   (CH_UPPER_SB | CH_EWA_VALID)},
/* 39 */	{153,   (CH_LOWER_SB | CH_EWA_VALID)},
/* 40 */	{157,   (CH_UPPER_SB | CH_EWA_VALID)},
/* 41 */	{161,   (CH_LOWER_SB | CH_EWA_VALID)},
/* 42 */	{165,   (CH_LOWER_SB)},

/* 11a japan */
/* 43 */	{184,   (CH_UPPER_SB)},
/* 44 */	{188,   (CH_LOWER_SB)},
/* 45 */	{192,   (CH_UPPER_SB)},
/* 46 */	{196,   (CH_LOWER_SB)},
/* 47 */	{200,   (CH_UPPER_SB)},
/* 48 */	{204,   (CH_LOWER_SB)},
/* 49 */	{208,   (CH_UPPER_SB)},
/* 50 */	{212,   (CH_LOWER_SB)},
/* 51 */	{216,   (CH_LOWER_SB)}
};

#else

/* 36 */	{140,	(CH_LOWER_SB)},

/* 11a usa high, ref5 only */
/* The 0x80 bit in pdiv means these are REF5, other entries are REF20 */
/* 37 */	{149,	(CH_UPPER_SB | CH_EWA_VALID)},
/* 38 */	{153,	(CH_LOWER_SB | CH_EWA_VALID)},
/* 39 */	{157,	(CH_UPPER_SB | CH_EWA_VALID)},
/* 40 */	{161,	(CH_LOWER_SB | CH_EWA_VALID)},
/* 41 */	{165,	(CH_LOWER_SB)},

/* 11a japan */
/* 42 */	{184,	(CH_UPPER_SB)},
/* 43 */	{188,	(CH_LOWER_SB)},
/* 44 */	{192,	(CH_UPPER_SB)},
/* 45 */	{196,	(CH_LOWER_SB)},
/* 46 */	{200,	(CH_UPPER_SB)},
/* 47 */	{204,	(CH_LOWER_SB)},
/* 48 */	{208,	(CH_UPPER_SB)},
/* 49 */	{212,	(CH_LOWER_SB)},
/* 50 */	{216,	(CH_LOWER_SB)}
};
#endif /* WL11AC */

/* country code mapping for SPROM rev 1 */
static const char def_country[][WLC_CNTRY_BUF_SZ] = {
	"AU",   /* Worldwide */
	"TH",   /* Thailand */
	"IL",   /* Israel */
	"JO",   /* Jordan */
	"CN",   /* China */
	"JP",   /* Japan */
	"US",   /* USA */
	"DE",   /* Europe */
	"US",   /* US Low Band, use US */
	"JP",   /* Japan High Band, use Japan */
};

/* autocountry default country code list */
static const char def_autocountry[][WLC_CNTRY_BUF_SZ] = {
	"XY",
	"XA",
	"XB",
	"X0",
	"X1",
	"X2",
	"X3",
	"XS",
	"XV",
	"XT"
};

static const char BCMATTACHDATA(rstr_ccode)[] = "ccode";
static const char BCMATTACHDATA(rstr_cc)[] = "cc";
static const char BCMATTACHDATA(rstr_regrev)[] = "regrev";

static bool
wlc_autocountry_lookup(char *cc)
{
	uint i;

	for (i = 0; i < ARRAYSIZE(def_autocountry); i++)
		if (!strcmp(def_autocountry[i], cc))
			return TRUE;

	return FALSE;
}

static bool
wlc_lookup_advertised_cc(char* ccode, const clm_country_t country)
{
	ccode_t advertised_cc;
	bool rv = FALSE;
	if (CLM_RESULT_OK == clm_country_advertised_cc(country, advertised_cc)) {
		memcpy(ccode, advertised_cc, 2);
		ccode[2] = '\0';
		rv = TRUE;
	}

	return rv;
}

static int
wlc_channel_init_ccode(wlc_cm_info_t *wlc_cmi, char* country_abbrev, int ca_len)
{
	wlc_info_t *wlc = wlc_cmi->wlc;

	int result = BCME_OK;
	clm_country_t country;
#ifdef PCOEM_LINUXSTA
	bool use_row = TRUE;
	wlc_pub_t *pub = wlc->pub;
#endif // endif

	result = wlc_country_lookup(wlc, country_abbrev, &country);

	/* default to US if country was not specified or not found */
	if (result != CLM_RESULT_OK) {
		strncpy(country_abbrev, "US", ca_len - 1);
		result = wlc_country_lookup(wlc, country_abbrev, &country);
	}

	/* Default to the NULL country(#n) which has no channels, if country US is not found */
	if (result != CLM_RESULT_OK) {
		strncpy(country_abbrev, "#n", ca_len - 1);
		result = wlc_country_lookup(wlc, country_abbrev, &country);
	}

	ASSERT(result == CLM_RESULT_OK);

	/* save default country for exiting 11d regulatory mode */
	wlc_cntry_set_default(wlc->cntry, country_abbrev);

	/* initialize autocountry_default to driver default */
	if (wlc_autocountry_lookup(country_abbrev))
		wlc_11d_set_autocountry_default(wlc->m11d, country_abbrev);
	else
		wlc_11d_set_autocountry_default(wlc->m11d, "XV");

#ifdef PCOEM_LINUXSTA
	if ((CHIPID(pub->sih->chip) != BCM4311_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM4312_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM4313_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM4321_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM4322_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM43224_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM43225_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM43421_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM4342_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM43131_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM43217_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM43227_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM43228_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM4331_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM43142_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM43428_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM43602_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM4350_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM4354_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM4356_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM4358_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM4360_CHIP_ID) &&
	    (CHIPID(pub->sih->chip) != BCM4352_CHIP_ID)) {
		printf("Broadcom vers %s: Unsupported Chip (%x)\n",
			EPI_VERSION_STR, pub->sih->chip);
		return BCME_ERROR;
	}

	if ((pub->sih->boardvendor == VENDOR_HP) && (!bcmp(country_abbrev, "US", 2) ||
		!bcmp(country_abbrev, "JP", 2) || !bcmp(country_abbrev, "IL", 2)))
		use_row = FALSE;

	/* use RoW locale if set */
	if (use_row && bcmp(country_abbrev, "X", 1)) {
		bzero(country_abbrev, WLC_CNTRY_BUF_SZ);
		strncpy(country_abbrev, "XW", WLC_CNTRY_BUF_SZ);
	}

	/* Enable Auto Country Discovery */
	wlc_11d_set_autocountry_default(wlc->m11d, country_abbrev);

#endif /* PCOEM_LINUXSTA */

	/* Calling set_countrycode() once does not generate any event, if called more than
	 * once generates COUNTRY_CODE_CHANGED event which will cause the driver to crash
	 * at startup since bsscfg structure is still not initialized.
	 */
	wlc_set_countrycode(wlc_cmi, country_abbrev);

	/* update edcrs_eu for the initial country setting */
	wlc->is_edcrs_eu = wlc_is_edcrs_eu(wlc);

	return result;
}

static int
wlc_cm_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_cm_info_t *wlc_cmi = (wlc_cm_info_t *)hdl;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	int err = BCME_OK;
	int32 int_val = 0;
	int32 *ret_int_ptr;

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	switch (actionid) {
	case IOV_GVAL(IOV_RCLASS):
		*ret_int_ptr = wlc_get_regclass(wlc_cmi, (chanspec_t)int_val);
		WL_INFORM(("chspec:%x rclass:%d\n", (chanspec_t)int_val, *ret_int_ptr));

		break;
	case IOV_SVAL(IOV_CLMLOAD): {
		wl_dload_data_t dload_data;
		uint8 *bufptr;
		dload_error_status_t status;

		/* Make sure we have at least a dload data structure */
		if (p_len < sizeof(wl_dload_data_t)) {
			err =  BCME_ERROR;
			wlc_cm->clmload_status = DLOAD_STATUS_IOVAR_ERROR;
			break;
		}

		/* copy to stack so structure wl_data_data has any required processor alignment */
		memcpy(&dload_data, (wl_dload_data_t *)params, sizeof(wl_dload_data_t));

		WL_NONE(("%s: IOV_CLMLOAD flag %04x, type %04x, len %d, crc %08x\n",
			__FUNCTION__, dload_data.flag, dload_data.dload_type,
			dload_data.len, dload_data.crc));

		if (((dload_data.flag & DLOAD_FLAG_VER_MASK) >> DLOAD_FLAG_VER_SHIFT)
			!= DLOAD_HANDLER_VER) {
			err =  BCME_ERROR;
			wlc_cm->clmload_status = DLOAD_STATUS_IOVAR_ERROR;
			break;
		}

		bufptr = ((wl_dload_data_t *)(params))->data;

		status = wlc_blob_download(wlc_cm->clmload_wbi, dload_data.flag,
			bufptr, dload_data.len, wlc_handle_clm_dload);
		switch (status) {
			case DLOAD_STATUS_DOWNLOAD_SUCCESS:
			case DLOAD_STATUS_DOWNLOAD_IN_PROGRESS:
				err = BCME_OK;
				wlc_cm->clmload_status = status;
				break;
			default:
				err = BCME_ERROR;
				wlc_cm->clmload_status = status;
				break;
		}
		break;
	}
	case IOV_GVAL(IOV_CLMLOAD_STATUS): {
		*((uint32 *)arg) = wlc_cm->clmload_status;
		break;
	}

	case IOV_GVAL(IOV_DIS_CH_GRP):
		*((uint32 *)arg) = wlc_cmi->wlc->pub->_dis_ch_grp_conf |
				wlc_cmi->wlc->pub->_dis_ch_grp_user;
		WL_INFORM(("wl%d: ch grp conf: 0x%08x, user: 0x%08x\n", wlc_cmi->wlc->pub->unit,
			wlc_cmi->wlc->pub->_dis_ch_grp_conf, wlc_cmi->wlc->pub->_dis_ch_grp_user));
		break;
	case IOV_GVAL(IOV_DIS_CH_GRP_CONF):
		*((uint32 *)arg) = wlc_cmi->wlc->pub->_dis_ch_grp_conf;
		WL_INFORM(("wl%d: ch grp conf: 0x%08x, user: 0x%08x\n", wlc_cmi->wlc->pub->unit,
			wlc_cmi->wlc->pub->_dis_ch_grp_conf, wlc_cmi->wlc->pub->_dis_ch_grp_user));
		break;
	case IOV_GVAL(IOV_DIS_CH_GRP_USER):
		*((uint32 *)arg) = wlc_cmi->wlc->pub->_dis_ch_grp_user;
		WL_INFORM(("wl%d: ch grp conf: 0x%08x, user: 0x%08x\n", wlc_cmi->wlc->pub->unit,
			wlc_cmi->wlc->pub->_dis_ch_grp_conf, wlc_cmi->wlc->pub->_dis_ch_grp_user));
		break;
	case IOV_SVAL(IOV_DIS_CH_GRP_USER):
		if (IS_DIS_CH_GRP_VALID(int_val | wlc_cmi->wlc->pub->_dis_ch_grp_conf)) {
			wlc_cmi->wlc->pub->_dis_ch_grp_user = (uint32) int_val;
		} else {
			WL_ERROR(("wl%d: ignoring invalid dis_ch_grp from user 0x%x (conf: 0x%x)\n",
					wlc_cmi->wlc->pub->unit, int_val,
					wlc_cmi->wlc->pub->_dis_ch_grp_conf));
			err = BCME_BADARG;
		}
		WL_INFORM(("wl%d: ch grp conf: 0x%08x, user: 0x%08x\n", wlc_cmi->wlc->pub->unit,
			wlc_cmi->wlc->pub->_dis_ch_grp_conf, wlc_cmi->wlc->pub->_dis_ch_grp_user));
		break;

	case IOV_GVAL(IOV_IS_EDCRS_EU):
		*ret_int_ptr = wlc_cmi->wlc->is_edcrs_eu;
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

wlc_cm_info_t *
BCMATTACHFN(wlc_channel_mgr_attach)(wlc_info_t *wlc)
{
	clm_result_t result;
	wlc_cm_info_t *wlc_cmi;
	wlc_cm_data_t *wlc_cm;
	char country_abbrev[WLC_CNTRY_BUF_SZ];
	wlc_pub_t *pub = wlc->pub;
	int ref_cnt;
#ifdef WLCLMINC
	uint32 clm_inc_size;
	uint32 clm_inc_hdr_offset;
#endif // endif
	WL_TRACE(("wl%d: wlc_channel_mgr_attach\n", wlc->pub->unit));

	if ((wlc_cmi = (wlc_cm_info_t *)MALLOCZ(pub->osh, sizeof(wlc_cm_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes", pub->unit,
			__FUNCTION__, MALLOCED(pub->osh)));
		return NULL;
	}

	wlc_cmi->pub = pub;
	wlc_cmi->wlc = wlc;
	/* XXX TEMPORARY HACK.  Since wlc's cmi is set by the caller with the
	 * return value/handle, we can't use it via wlc until we return.  Yet
	 * because are slowly morphy the code in steps, we do indeed use it
	 * from inside wlc_channel.  This won't happen eventually.  So the hack:
	 * set wlc'c cmi from inside right now!
	 */
	wlc->cmi = wlc_cmi;

	wlc_cm = (wlc_cm_data_t *) obj_registry_get(wlc->objr, OBJR_CLM_PTR);

	if (wlc_cm == NULL) {
		if ((wlc_cm = (wlc_cm_data_t *) MALLOCZ(wlc->pub->osh,
			sizeof(wlc_cm_data_t))) == NULL) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes", wlc->pub->unit,
				__FUNCTION__, MALLOCED(wlc->pub->osh)));
			MFREE(pub->osh, wlc_cmi, sizeof(wlc_cm_info_t));
			return NULL;
		}

		if ((wlc_cm->clmload_wbi = wlc_blob_attach(wlc)) == NULL) {
			MFREE(pub->osh, wlc_cm, sizeof(wlc_cm_data_t));
			MFREE(pub->osh, wlc_cmi, sizeof(wlc_cm_info_t));
			return NULL;
		}

		obj_registry_set(wlc->objr, OBJR_CLM_PTR, wlc_cm);
	}
	ref_cnt = obj_registry_ref(wlc->objr, OBJR_CLM_PTR);
	wlc_cmi->cm = wlc_cm;

	if ((wlc_cmi->cm->valid_rcvec.vec = (uint8 *) MALLOCZ(pub->osh, MAXRCVEC)) == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes", wlc->pub->unit,
			__FUNCTION__, MALLOCED(pub->osh)));
		goto fail;
	}

	/* init the per chain limits to max power so they have not effect */
	memset(&wlc_cm->bandstate[BAND_2G_INDEX].chain_limits, WLC_TXPWR_MAX,
	       sizeof(struct wlc_channel_txchain_limits));
	memset(&wlc_cm->bandstate[BAND_5G_INDEX].chain_limits, WLC_TXPWR_MAX,
	       sizeof(struct wlc_channel_txchain_limits));

	/* get the SPROM country code or local, required to initialize channel set below */
	bzero(country_abbrev, WLC_CNTRY_BUF_SZ);
	if (wlc->pub->sromrev > 1) {
		/* get country code */
		const char *ccode = getvar(wlc->pub->vars, rstr_ccode);
		if (ccode) {
#ifndef OPENSRC_IOV_IOCTL
			int err;
#endif /* OPENSRC_IOV_IOCTL */

			strncpy(country_abbrev, ccode, WLC_CNTRY_BUF_SZ - 1);
#ifndef OPENSRC_IOV_IOCTL
			err = wlc_cntry_external_to_internal(country_abbrev,
				sizeof(country_abbrev));
			if (err != BCME_OK) {
				/* XXX Gross Hack. In non-BCMDBG non-WLTEST mode,
				 * we want to reject country abbreviations ALL and RDR.
				 * So convert them to something really bogus, and
				 * rely on other code to reject them in favour
				 * of a default.
				 */
				strncpy(country_abbrev, "__", WLC_CNTRY_BUF_SZ - 1);
			}
#endif /* OPENSRC_IOV_IOCTL */

		}
	} else {
		uint locale_num;

		/* get locale */
		locale_num = (uint)getintvar(wlc->pub->vars, rstr_cc);
		/* get mapped country */
		if (locale_num < ARRAYSIZE(def_country))
			strncpy(country_abbrev, def_country[locale_num],
			        sizeof(country_abbrev) - 1);
	}

#if defined(BCMDBG) || defined(WLTEST) || defined(ATE_BUILD)
	/* convert "ALL/0" country code to #a/0 */
	if (!strncmp(country_abbrev, "ALL", WLC_CNTRY_BUF_SZ)) {
		strncpy(country_abbrev, "#a", sizeof(country_abbrev) - 1);
	}
#endif /* BCMDBG || WLTEST || ATE_BUILD */

	if (ref_cnt > 1) {
		/* Since the whole of cm_data_t is shared,
		* we have no clm_init for second instance
		*/
		goto skip_clm_init;
	}

	strncpy(wlc_cm->srom_ccode, country_abbrev, WLC_CNTRY_BUF_SZ - 1);
	wlc_cm->srom_regrev = getintvar(wlc->pub->vars, rstr_regrev);

	/* For "some" apple boards with KR2, make them as KR3 as they have passed the
	 * FCC test but with wrong SROM contents
	 */
	if ((pub->sih->boardvendor == VENDOR_APPLE) &&
	    ((pub->sih->boardtype == BCM943224M93) ||
	     (pub->sih->boardtype == BCM943224M93A))) {
		if ((wlc_cm->srom_regrev == 2) &&
		    !strncmp(country_abbrev, "KR", WLC_CNTRY_BUF_SZ)) {
			wlc_cm->srom_regrev = 3;
		}
	}

	/* Correct SROM contents of an Apple board */
	if ((pub->sih->boardvendor == VENDOR_APPLE) &&
	    (pub->sih->boardtype == 0x93) &&
	    !strncmp(country_abbrev, "JP", WLC_CNTRY_BUF_SZ) &&
	    (wlc_cm->srom_regrev == 4)) {
		wlc_cm->srom_regrev = 6;
	}

	/* XXX PR118982 WAR to fix mis-programmed regrev in SROM for the X51A
	 * WAR apply for SKU "X0", X2" & "X3" modules; change regrev to 19.
	 */
	if ((pub->sih->boardvendor == VENDOR_APPLE) &&
	    (pub->sih->boardtype == BCM94360X51A)) {
		if ((!strncmp(country_abbrev, "X0", WLC_CNTRY_BUF_SZ)) ||
		    (!strncmp(country_abbrev, "X2", WLC_CNTRY_BUF_SZ)) ||
		    (!strncmp(country_abbrev, "X3", WLC_CNTRY_BUF_SZ)))
		        wlc_cm->srom_regrev = 19;
	}

	result = clm_init(&clm_header);
	ASSERT(result == CLM_RESULT_OK);

	/* these are initialised to zero until they point to malloced data */
	wlc_cm->clm_base_dataptr = NULL;
	wlc_cm->clm_base_data_len = 0;

	wlc_cm->clm_inc_dataptr = NULL;
	wlc_cm->clm_inc_headerptr = NULL;
	wlc_cm->clm_inc_data_len = 0;

#ifdef WLCLMINC
	/* Need to malloc memory for incremental data so existing stuff can be reclaimed.
	   How do we know the size?
	*/

	extern char _clmincstart;
	extern char _clmincend;

	clm_inc_size = (void*)&_clmincend - (void*)&_clmincstart;

	clm_inc_hdr_offset = (uint32)&clm_inc_header - (uint32)&_clmincstart;

	if ((wlc_cm->clm_inc_dataptr = (void*)MALLOC(wlc->pub->osh, clm_inc_size)) != NULL) {
		bcopy((char*)&_clmincstart, (char*)wlc_cm->clm_inc_dataptr, clm_inc_size);
		wlc_cm->clm_inc_data_len = clm_inc_size;
		wlc_cm->clm_inc_headerptr = (struct clm_data_header*)((char*)wlc_cm->clm_inc_dataptr
			+ clm_inc_hdr_offset);
		result = clm_set_inc_data(wlc_cm->clm_inc_headerptr);
		ASSERT(result == CLM_RESULT_OK);

		/* Immediately reclaim CLM incremental data after copying data to malloc'ed
		 * buffer. This avoids holding onto 2 copies of the incremental data during
		 * attach time, and frees up more heap space for attach time allocations.
		 */
		bzero((void*)&_clmincstart, clm_inc_size);
		OSL_ARENA_ADD((uint32)&_clmincstart, clm_inc_size);
	}
#endif /* WLCLMINC */

	wlc_cm->bandedge_filter_apply = ((CHIPID(pub->sih->chip) == BCM4331_CHIP_ID) ||
		(CHIPID(pub->sih->chip) == BCM43431_CHIP_ID));

skip_clm_init:
	result = wlc_channel_init_ccode(wlc_cmi, country_abbrev, sizeof(country_abbrev));

#ifdef PCOEM_LINUXSTA
	if (result != BCME_OK) {
		wlc->cmi = NULL;
		wlc_channel_mgr_detach(wlc_cmi);
		return NULL;
	}
#else
	BCM_REFERENCE(result);
#endif /* PCOEM_LINUXSTA */

#ifdef WLTDLS
	/* setupreq */
	if (TDLS_SUPPORT(wlc->pub)) {
		if (wlc_ier_add_build_fn(wlc->ier_tdls_srq, DOT11_MNG_REGCLASS_ID,
			wlc_channel_tdls_calc_rc_ie_len, wlc_channel_tdls_write_rc_ie, wlc_cmi)
			!= BCME_OK) {
			WL_ERROR(("wl%d: %s wlc_ier_add_build_fn failed, "
				"reg class in tdls setupreq\n", wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
		/* setupresp */
		if (wlc_ier_add_build_fn(wlc->ier_tdls_srs, DOT11_MNG_REGCLASS_ID,
			wlc_channel_tdls_calc_rc_ie_len, wlc_channel_tdls_write_rc_ie, wlc_cmi)
			!= BCME_OK) {
			WL_ERROR(("wl%d: %s wlc_ier_add_build_fn failed, "
				"reg class in tdls setupresp\n", wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
	}
#endif /* WLTDLS */

	/* register module */
	if (wlc_module_register(wlc->pub, cm_iovars, "cm", wlc_cmi, wlc_cm_doiovar,
	    NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n", wlc->pub->unit, __FUNCTION__));

		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(wlc->pub, "txpwr_reg",
	                  (dump_fn_t)wlc_channel_dump_reg_ppr, (void *)wlc_cmi);
	wlc_dump_register(wlc->pub, "txpwr_local",
	                  (dump_fn_t)wlc_channel_dump_reg_local_ppr, (void *)wlc_cmi);
	wlc_dump_register(wlc->pub, "txpwr_srom",
	                  (dump_fn_t)wlc_channel_dump_srom_ppr, (void *)wlc_cmi);
	wlc_dump_register(wlc->pub, "txpwr_margin",
	                  (dump_fn_t)wlc_channel_dump_margin, (void *)wlc_cmi);
	wlc_dump_register(wlc->pub, "country_regrevs",
	                  (dump_fn_t)wlc_channel_supported_country_regrevs, (void *)wlc_cmi);
	wlc_dump_register(wlc->pub, "agg_map",
	                  (dump_fn_t)wlc_dump_country_aggregate_map, (void *)wlc_cmi);
	wlc_dump_register(wlc->pub, "txpwr_reg_max",
	                  (dump_fn_t)wlc_dump_max_power_per_channel, (void *)wlc_cmi);
	wlc_dump_register(wlc->pub, "clm_limits_2G_20M",
	                  (dump_fn_t)wlc_dump_clm_limits_2G_20M, (void *)wlc_cmi);
	wlc_dump_register(wlc->pub, "clm_limits_2G_40M",
	                  (dump_fn_t)wlc_dump_clm_limits_2G_40M, (void *)wlc_cmi);
	wlc_dump_register(wlc->pub, "clm_limits_2G_20in40M",
	                  (dump_fn_t)wlc_dump_clm_limits_2G_20in40M, (void *)wlc_cmi);
	wlc_dump_register(wlc->pub, "clm_limits_5G_20M",
	                  (dump_fn_t)wlc_dump_clm_limits_5G_20M, (void *)wlc_cmi);
	wlc_dump_register(wlc->pub, "clm_limits_5G_40M",
	                  (dump_fn_t)wlc_dump_clm_limits_5G_40M, (void *)wlc_cmi);
	wlc_dump_register(wlc->pub, "clm_limits_5G_20in40M",
	                  (dump_fn_t)wlc_dump_clm_limits_5G_20in40M, (void *)wlc_cmi);
#endif /* BCMDBG || BCMDBG_DUMP */

	return wlc_cmi;

	goto fail;

fail:	/* error handling */
	wlc->cmi = NULL;
	wlc_channel_mgr_detach(wlc_cmi);
	return NULL;
}

static dload_error_status_t
wlc_handle_clm_dload(wlc_cm_info_t *wlc_cmi, segment_t *segments, uint32 segment_count)
{
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	dload_error_status_t status = DLOAD_STATUS_DOWNLOAD_SUCCESS;
	clm_result_t clm_result = CLM_RESULT_OK;

	clm_country_t country;
	char country_abbrev[WLC_CNTRY_BUF_SZ];
	struct clm_data_header* clm_dataptr;
	int clm_data_len;
	uint32 chip;

	/* Make sure we have a chip id segment and clm data segemnt */
	if (segment_count < 2)
		return DLOAD_STATUS_CLM_BLOB_FORMAT;

	/* Check to see if chip id segment is correct length */
	if (segments[1].length != 4)
		return DLOAD_STATUS_CLM_BLOB_FORMAT;

	/* Check to see if chip id matches this chip's actual value */
	chip = load32_ua(segments[1].data);
	if (chip != CHIPID(wlc_cmi->pub->sih->chip))
		return DLOAD_STATUS_CLM_MISMATCH;

	/* Process actual clm data segment */
	clm_dataptr = (struct clm_data_header *)(segments[0].data);
	clm_data_len = segments[0].length;

	/* At this point forward we are responsible for freeing this data pointer */
	segments[0].data = NULL;

	/* Clear out any incremental data */
	if (wlc_cm->clm_inc_dataptr != NULL) {
		clm_set_inc_data(NULL);
		MFREE(wlc_cmi->pub->osh, wlc_cm->clm_inc_dataptr,
			wlc_cm->clm_inc_data_len);
		wlc_cm->clm_inc_data_len = 0;
		wlc_cm->clm_inc_dataptr = NULL;
		wlc_cm->clm_inc_headerptr = NULL;
	}

	/* Free any previously downloaded base data */
	if (wlc_cm->clm_base_dataptr != NULL) {
		MFREE(wlc_cmi->pub->osh, wlc_cm->clm_base_dataptr,
			wlc_cm->clm_base_data_len);
	}
	if (clm_dataptr != NULL) {
		WL_NONE(("wl%d: Pointing API at new base data: v%s\n",
			wlc_cmi->pub->unit, clm_dataptr->clm_version));
		clm_result = clm_init(clm_dataptr);
		if (clm_result != CLM_RESULT_OK) {
			WL_ERROR(("wl%d: %s: Error loading new base CLM"
				" data.\n",
				wlc_cmi->pub->unit, __FUNCTION__));
			status = DLOAD_STATUS_CLM_DATA_BAD;
			MFREE(wlc_cmi->pub->osh, clm_dataptr,
				clm_data_len);
		}
	}
	if ((clm_dataptr == NULL) || (clm_result != CLM_RESULT_OK)) {
		WL_NONE(("wl%d: %s: Reverting to base data.\n",
			wlc_cmi->pub->unit, __FUNCTION__));
		clm_init(&clm_header);
		wlc_cm->clm_base_data_len = 0;
		wlc_cm->clm_base_dataptr = NULL;
	} else {
		wlc_cm->clm_base_dataptr = clm_dataptr;
		wlc_cm->clm_base_data_len = clm_data_len;
	}

	if (wlc_country_lookup_direct(wlc_cm->ccode, wlc_cm->regrev, &country) ==
		CLM_RESULT_OK)
		wlc_cm->country = country;
	else
		wlc_cm->country = 0;

	bzero(country_abbrev, WLC_CNTRY_BUF_SZ);
	strncpy(country_abbrev, wlc_cm->srom_ccode, WLC_CNTRY_BUF_SZ - 1);

	wlc_channel_init_ccode(wlc_cmi, country_abbrev, sizeof(country_abbrev));

	return status;
}

void
BCMATTACHFN(wlc_channel_mgr_detach)(wlc_cm_info_t *wlc_cmi)
{
	if (wlc_cmi) {
		wlc_info_t *wlc = wlc_cmi->wlc;
		wlc_pub_t *pub = wlc->pub;

		wlc_module_unregister(wlc->pub, "cm", wlc_cmi);

		if (wlc_cmi->cm->valid_rcvec.vec != NULL) {
			MFREE(pub->osh, wlc_cmi->cm->valid_rcvec.vec, MAXRCVEC);
		}
		if (obj_registry_unref(wlc->objr, OBJR_CLM_PTR) == 0) {
			wlc_cm_data_t *wlc_cm = wlc_cmi->cm;

			wlc_blob_detach(wlc_cm->clmload_wbi);

			if (wlc_cm->clm_inc_dataptr != NULL) {
				MFREE(pub->osh, wlc_cm->clm_inc_dataptr, wlc_cm->clm_inc_data_len);
			}
			if (wlc_cm->clm_base_dataptr != NULL) {
				MFREE(pub->osh, wlc_cm->clm_base_dataptr,
					wlc_cm->clm_base_data_len);
			}

			obj_registry_set(wlc->objr, OBJR_CLM_PTR, NULL);
			MFREE(pub->osh, wlc_cm, sizeof(wlc_cm_data_t));
		}
		MFREE(pub->osh, wlc_cmi, sizeof(wlc_cm_info_t));
	}
}

const char* wlc_channel_country_abbrev(wlc_cm_info_t *wlc_cmi)
{
	return wlc_cmi->cm->country_abbrev;
}

const char* wlc_channel_ccode(wlc_cm_info_t *wlc_cmi)
{
	return wlc_cmi->cm->ccode;
}

const char* wlc_channel_sromccode(wlc_cm_info_t *wlc_cmi)
{
	return wlc_cmi->cm->srom_ccode;
}

uint wlc_channel_regrev(wlc_cm_info_t *wlc_cmi)
{
	return wlc_cmi->cm->regrev;
}

uint16 wlc_channel_locale_flags(wlc_cm_info_t *wlc_cmi)
{
	wlc_info_t *wlc = wlc_cmi->wlc;

	return wlc_cmi->cm->bandstate[wlc->band->bandunit].locale_flags;
}

uint16 wlc_channel_locale_flags_in_band(wlc_cm_info_t *wlc_cmi, uint bandunit)
{
	return wlc_cmi->cm->bandstate[bandunit].locale_flags;
}

/*
 * return the first valid chanspec for the locale, if one is not found and hw_fallback is true
 * then return the first h/w supported chanspec.
 */
chanspec_t
wlc_default_chanspec(wlc_cm_info_t *wlc_cmi, bool hw_fallback)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	chanspec_t  chspec;

	chspec = wlc_create_chspec(wlc, 0);
	/* try to find a chanspec that's valid in this locale */
	if ((chspec = wlc_next_chanspec(wlc_cmi, chspec, CHAN_TYPE_ANY, 0)) == INVCHANSPEC)
		/* try to find a chanspec valid for this hardware */
		if (hw_fallback)
			chspec = phy_utils_chanspec_band_firstch(
				(phy_info_t *)WLC_PI(wlc),
				wlc->band->bandtype);
	return chspec;
}

/*
 * Return the next channel's chanspec.
 */
chanspec_t
wlc_next_chanspec(wlc_cm_info_t *wlc_cmi, chanspec_t cur_chanspec, int type, bool any_band)
{
	uint8 ch;
	uint8 cur_chan = CHSPEC_CHANNEL(cur_chanspec);
	chanspec_t chspec;

	/* 0 is an invalid chspec, routines trying to find the first available channel should
	 * now be using wlc_default_chanspec (above)
	 */
	ASSERT(cur_chanspec);

	/* current channel must be valid */
	if (cur_chan > MAXCHANNEL) {
		return ((chanspec_t)INVCHANSPEC);
	}
	/* Try all channels in current band */
	ch = cur_chan + 1;
	for (; ch <= MAXCHANNEL; ch++) {
		if (ch == MAXCHANNEL)
			ch = 0;
		if (ch == cur_chan)
			break;
		/* create the next channel spec */
		chspec = cur_chanspec & ~WL_CHANSPEC_CHAN_MASK;
		chspec |= ch;
		if (wlc_valid_chanspec(wlc_cmi, chspec)) {
			if ((type == CHAN_TYPE_ANY) ||
			(type == CHAN_TYPE_CHATTY && !wlc_quiet_chanspec(wlc_cmi, chspec)) ||
			(type == CHAN_TYPE_QUIET && wlc_quiet_chanspec(wlc_cmi, chspec)))
				return chspec;
		}
	}

	if (!any_band)
		return ((chanspec_t)INVCHANSPEC);

	/* Couldn't find any in current band, try other band */
	ch = cur_chan + 1;
	for (; ch <= MAXCHANNEL; ch++) {
		if (ch == MAXCHANNEL)
			ch = 0;
		if (ch == cur_chan)
			break;

		/* create the next channel spec */
		chspec = cur_chanspec & ~(WL_CHANSPEC_CHAN_MASK | WL_CHANSPEC_BAND_MASK);
		chspec |= ch;
		if (CHANNEL_BANDUNIT(wlc, ch) == BAND_2G_INDEX)
			chspec |= WL_CHANSPEC_BAND_2G;
		else
			chspec |= WL_CHANSPEC_BAND_5G;
		if (wlc_valid_chanspec_db(wlc_cmi, chspec)) {
			if ((type == CHAN_TYPE_ANY) ||
			(type == CHAN_TYPE_CHATTY && !wlc_quiet_chanspec(wlc_cmi, chspec)) ||
			(type == CHAN_TYPE_QUIET && wlc_quiet_chanspec(wlc_cmi, chspec)))
				return chspec;
		}
	}

	return ((chanspec_t)INVCHANSPEC);
}

/* return chanvec for a given country code and band */
bool
wlc_channel_get_chanvec(struct wlc_info *wlc, const char* country_abbrev,
	int bandtype, chanvec_t *channels)
{
	clm_band_t band;
	clm_result_t result = CLM_RESULT_ERR;
	clm_country_t country;
	clm_country_locales_t locale;
	chanvec_t unused;
	uint i;
	chanvec_t channels_5g20, channels_5g4080;

	result = wlc_country_lookup(wlc, country_abbrev, &country);
	if (result != CLM_RESULT_OK)
		return FALSE;

	result = wlc_get_locale(country, &locale);
	if (bandtype != WLC_BAND_2G && bandtype != WLC_BAND_5G)
		return FALSE;

	band = (bandtype == WLC_BAND_2G) ? CLM_BAND_2G : CLM_BAND_5G;

	wlc_locale_get_channels(&locale, band, channels, &unused);
	clm_valid_channels_5g(&locale, (clm_channels_t*)&channels_5g20,
	(clm_channels_t*)&channels_5g4080);

	/* don't mask 2GHz channels, but check 5G channels */
	for (i = (CH_MAX_2G_CHANNEL / 8) + 1; i < sizeof(chanvec_t); i++)
	channels->vec[i] &= channels_5g20.vec[i];
	return TRUE;
}

/* set the driver's current country and regulatory information using a country code
 * as the source. Lookup built in country information found with the country code.
 */
int
wlc_set_countrycode(wlc_cm_info_t *wlc_cmi, const char* ccode)
{
	WL_NONE(("wl%d: %s: ccode \"%s\"\n", wlc_cmi->wlc->pub->unit, __FUNCTION__, ccode));
	return wlc_set_countrycode_rev(wlc_cmi, ccode, -1);
}

int
wlc_set_countrycode_rev(wlc_cm_info_t *wlc_cmi, const char* ccode, int regrev)
{
#ifdef BCMDBG
	wlc_info_t *wlc = wlc_cmi->wlc;
#endif // endif
	clm_result_t result = CLM_RESULT_ERR;
	clm_country_t country;
	char mapped_ccode[WLC_CNTRY_BUF_SZ];
	uint mapped_regrev = 0;
	char country_abbrev[WLC_CNTRY_BUF_SZ] = { 0 };

	WL_NONE(("wl%d: %s: (country_abbrev \"%s\", ccode \"%s\", regrev %d) SPROM \"%s\"/%u\n",
	         wlc->pub->unit, __FUNCTION__,
	         country_abbrev, ccode, regrev, wlc_cmi->cm->srom_ccode, wlc_cmi->cm->srom_regrev));

	/* if regrev is -1, lookup the mapped country code,
	 * otherwise use the ccode and regrev directly
	 */
	if (regrev == -1) {
		/* map the country code to a built-in country code, regrev, and country_info */
		result = wlc_countrycode_map(wlc_cmi, ccode, mapped_ccode,
			&mapped_regrev, &country);
		if (result == CLM_RESULT_OK)
			WL_NONE(("wl%d: %s: mapped to \"%s\"/%u\n",
			         wlc->pub->unit, __FUNCTION__, ccode, mapped_regrev));
		else
			WL_NONE(("wl%d: %s: failed lookup\n",
			        wlc->pub->unit, __FUNCTION__));
	} else {
		/* find the matching built-in country definition */
		result = wlc_country_lookup_direct(ccode, regrev, &country);
		strncpy(mapped_ccode, ccode, WLC_CNTRY_BUF_SZ-1);
		mapped_ccode[WLC_CNTRY_BUF_SZ-1] = '\0';
		mapped_regrev = regrev;
	}

	if (result != CLM_RESULT_OK)
		return BCME_BADARG;

	/* Set the driver state for the country.
	 * Getting the advertised country code from CLM.
	 * Else use the one comes from ccode.
	 */
	if (wlc_lookup_advertised_cc(country_abbrev, country))
		wlc_set_country_common(wlc_cmi, country_abbrev,
		mapped_ccode, mapped_regrev, country);
	else
		wlc_set_country_common(wlc_cmi, ccode,
		mapped_ccode, mapped_regrev, country);

	return 0;
}

/* wrapper for wlc_channels_init used during band change */
void wlc_channels_init_ext(wlc_cm_info_t *wlc_cmi)
{
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;

	wlc_channels_init(wlc_cmi, wlc_cm->country);
}

/* set the driver's current country and regulatory information using a country code
 * as the source. Look up built in country information found with the country code.
 */
static void
wlc_set_country_common(wlc_cm_info_t *wlc_cmi,
                       const char* country_abbrev,
                       const char* ccode, uint regrev, clm_country_t country)
{
	clm_result_t result = CLM_RESULT_ERR;
	clm_country_locales_t locale;
	uint16 flags;

	wlc_info_t *wlc = wlc_cmi->wlc;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	char prev_country_abbrev[WLC_CNTRY_BUF_SZ];
	unsigned long clm_flags = 0;

#if defined WLTXPWR_CACHE && defined(WLC_LOW) && defined(WL11N)
	wlc_phy_txpwr_cache_invalidate(wlc_phy_get_txpwr_cache(WLC_PI(wlc)));
#endif	/* WLTXPWR_CACHE */

	/* Ensure NUL string terminator before printing */
	wlc_cm->country_abbrev[WLC_CNTRY_BUF_SZ - 1] = '\0';
	wlc_cm->ccode[WLC_CNTRY_BUF_SZ - 1] = '\0';
	WL_REGULATORY(("wl%d: %s: country/abbrev/ccode/regrev "
			"from 0x%04x/%s/%s/%d to 0x%04x/%s/%s/%d\n",
			wlc->pub->unit, __FUNCTION__,
			wlc_cm->country, wlc_cm->country_abbrev, wlc_cm->ccode, wlc_cm->regrev,
			country, country_abbrev, ccode, regrev));

	if (wlc_cm->country == country && wlc_cm->regrev == regrev &&
			wlc_cm->country_abbrev[0] && wlc_cm->ccode[0] &&
			strncmp(wlc_cm->country_abbrev, country_abbrev, WLC_CNTRY_BUF_SZ) == 0 &&
			strncmp(wlc_cm->ccode, ccode, WLC_CNTRY_BUF_SZ) == 0) {
		WL_REGULATORY(("wl%d: %s: Avoid setting current country again.\n",
			wlc->pub->unit, __FUNCTION__));
		return;
	}

	/* save current country state */
	wlc_cm->country = country;

	bzero(&prev_country_abbrev, WLC_CNTRY_BUF_SZ);
	strncpy(prev_country_abbrev, wlc_cm->country_abbrev, WLC_CNTRY_BUF_SZ - 1);

	strncpy(wlc_cm->country_abbrev, country_abbrev, WLC_CNTRY_BUF_SZ-1);
	strncpy(wlc_cm->ccode, ccode, WLC_CNTRY_BUF_SZ-1);
	wlc_cm->regrev = regrev;

	result = wlc_get_locale(country, &locale);
	ASSERT(result == CLM_RESULT_OK);

	result = wlc_get_flags(&locale, CLM_BAND_2G, &flags);
	ASSERT(result == CLM_RESULT_OK);
	BCM_REFERENCE(result);

	result = clm_country_flags(&locale,  CLM_BAND_2G, &clm_flags);
	ASSERT(result == CLM_RESULT_OK);
	if (clm_flags & CLM_FLAG_EDCRS_EU) {
		wlc_phy_set_locale((phy_info_t *)WLC_PI(wlc), REGION_EU);
	} else if (clm_flags & CLM_FLAG_CHSPRWAR2) {
		wlc_phy_set_locale((phy_info_t *)WLC_PI(wlc), REGION_CHN);
	} else {
		wlc_phy_set_locale((phy_info_t *)WLC_PI(wlc), REGION_OTHER);
	}

#ifdef WL_BEAMFORMING
	if (TXBF_ENAB(wlc->pub)) {
		if (flags & WLC_TXBF) {
			wlc_stf_set_txbf(wlc, TRUE);
		} else {
			wlc_stf_set_txbf(wlc, FALSE);
		}
	}
#endif // endif

#ifdef WL11N
	/* disable/restore nmode based on country regulations */
	if ((flags & WLC_NO_MIMO) && ((NBANDS(wlc) == 2) || IS_SINGLEBAND_5G(wlc->deviceid))) {
		result = wlc_get_flags(&locale, CLM_BAND_5G, &flags);
		ASSERT(result == CLM_RESULT_OK);
	}
	if (flags & WLC_NO_MIMO) {
		wlc_set_nmode(wlc->hti, OFF);
		wlc->stf->no_cddstbc = TRUE;
	} else {
		wlc->stf->no_cddstbc = FALSE;
		wlc_prot_n_mode_reset(wlc->prot_n, FALSE);
	}

	wlc_stf_ss_update(wlc, wlc->bandstate[BAND_2G_INDEX]);
	wlc_stf_ss_update(wlc, wlc->bandstate[BAND_5G_INDEX]);
#endif /* WL11N */

#if defined(AP) && defined(RADAR)
	if ((NBANDS(wlc) == 2) || IS_SINGLEBAND_5G(wlc->deviceid)) {
		phy_radar_detect_mode_t mode;
		result = wlc_get_flags(&locale, CLM_BAND_5G, &flags);

		mode = ISDFS_UK(flags) ? RADAR_DETECT_MODE_UK :
			ISDFS_EU(flags) ? RADAR_DETECT_MODE_EU : RADAR_DETECT_MODE_FCC;

		phy_radar_detect_mode_set((phy_info_t *)WLC_PI(wlc), mode);
	}
#endif /* AP && RADAR */

	wlc_channels_init(wlc_cmi, country);

	/* Country code changed */
	if (strlen(prev_country_abbrev) > 1 &&
	    strncmp(wlc_cm->country_abbrev, prev_country_abbrev,
	            strlen(wlc_cm->country_abbrev)) != 0) {
		/* need to reset chan_blocked */
		if (wlc->dfs)
			wlc_dfs_reset_all(wlc->dfs);
		/* need to reset afe_override */
		wlc_channel_spurwar_locale(wlc_cmi, wlc->chanspec);

		wlc_mac_event(wlc, WLC_E_COUNTRY_CODE_CHANGED, NULL,
		              0, 0, 0, wlc_cm->country_abbrev, strlen(wlc_cm->country_abbrev) + 1);
	} else {
		/* clear channel blocked info when setting country code as "ALL" */
		if ((!strncmp(wlc_cm->country_abbrev, "#a", sizeof("#a") - 1)) && (wlc->dfs))
			wlc_dfs_reset_all(wlc->dfs);
	}
#ifdef WLOLPC
	if (OLPC_ENAB(wlc_cmi->wlc)) {
		WL_RATE(("olpc process: ccrev=%s regrev=%d\n", ccode, regrev));
		/* olpc needs to flush any stored chan info and cal if needed */
		wlc_olpc_eng_reset(wlc_cmi->wlc->olpc_info);
	}
#endif /* WLOLPC */

	wlc_ht_frameburst_limit(wlc->hti);
	return;
}

/*
 * wlc_is_country_edcrs_eu - takes country_code string and
 * returns true if it is an EDCRS_EU country based on a lookup list of countries.
 *
 * EDCRS_EU countries follow harmonized ETSI regulations.
 */
static bool
wlc_is_country_edcrs_eu(char * country_code)
{
	static const char cc_list[][WLC_CNTRY_BUF_SZ] = {
		"AL", "DZ", "AD", "AT",    "AZ", "BE", "BJ", "BT",
		"BA", "BW", "IO", "BG",    "CD", "CI", "HR", "CY",
		"CZ", "DK", "EE", "FO",    "FI", "FR", "GE", "DE",
		"GH", "GI", "GR", "GL",    "GG", "GN", "HU", "IS",
		"IE", "IL", "IT", "JE",    "JO", "KE", "KW", "LV",
		"LB", "LI", "LT", "LU",    "MK", "MG", "MW", "MT",
		"IM", "MU", "MC", "MN",    "ME", "MA", "NL", "NG",
		"NO", "OM", "PK", "PN",    "PL", "PT", "QA", "RO",
		"RW", "SM", "SA", "SN",    "RS", "SK", "SI", "ZA",
		"ES", "SZ", "SE", "CH",    "TH", "TN", "TR", "AE",
		"UG", "GB", "VA", "ZW"
	};
	int i, len = sizeof(cc_list)/sizeof(cc_list[0]);

	if (country_code == NULL || strlen(country_code) >= WLC_CNTRY_BUF_SZ) {
		WL_REGULATORY(("%s: country null or malformed\n", __FUNCTION__));
		return FALSE;
	}

	for (i = 0; i < len; ++i) {
		if (strncmp(cc_list[i], country_code, WLC_CNTRY_BUF_SZ) == 0) {
			WL_REGULATORY(("%s: country %s is a known EDCRS_EU country (@%d/%d)\n",
					__FUNCTION__, country_code, i, len));
			return TRUE;
		}
	}

	WL_REGULATORY(("%s: country %s is not a known EDCRS_EU country (%d)\n",
			__FUNCTION__, country_code, len));
	return FALSE;
}

/* returns true if current country's CLM flag matches EDCRS_EU.
 * (Some countries like Brazil will return false here though using DFS as in EU)
 * EDCRS_EU flag is defined in CLM for countries that follow EU harmonized standards.
 */
bool
wlc_is_edcrs_eu(struct wlc_info *wlc)
{
	wlc_cm_info_t *wlc_cmi = wlc->cmi;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	clm_result_t result;
	clm_country_locales_t locale;
	clm_country_t country;
	uint16 flags;
	bool edcrs_eu_flag_set;

	result = wlc_country_lookup_direct(wlc_cm->ccode, wlc_cm->regrev, &country);

	if (result != CLM_RESULT_OK) {
		goto no_edcrs_eu_flag_fallback;
	}

	result = wlc_get_locale(country, &locale);
	if (result != CLM_RESULT_OK) {
		goto no_edcrs_eu_flag_fallback;
	}

	result = wlc_get_flags(&locale, CLM_BAND_5G, &flags);
	if (result != CLM_RESULT_OK) {
		goto no_edcrs_eu_flag_fallback;
	}

	edcrs_eu_flag_set = ((flags & WLC_EDCRS_EU) == WLC_EDCRS_EU);

	WL_REGULATORY(("wl%d: %s: EDCRS_EU flag is %sset for country %s (flags:0x%02X)\n",
			wlc->pub->unit, __FUNCTION__,
			edcrs_eu_flag_set?"":"not ", wlc_cm->ccode, flags));
	if (edcrs_eu_flag_set) {
		return edcrs_eu_flag_set;
	}

no_edcrs_eu_flag_fallback:
	/* fallback to static list of known EDCRS_EU countries */
	return wlc_is_country_edcrs_eu(wlc_cm->ccode);
}

#ifdef RADAR
/* returns true if current CLM flag matches DFS_EU or DFS_UK for the operation mode/band;
 * (Some countries like Brazil will return true here though not in EU/EDCRS)
 */
bool
wlc_is_dfs_eu_uk(struct wlc_info *wlc)
{
	wlc_cm_info_t *wlc_cmi = wlc->cmi;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	clm_result_t result;
	clm_country_locales_t locale;
	clm_country_t country;
	uint16 flags;

	if (!((NBANDS(wlc) == 2) || IS_SINGLEBAND_5G(wlc->deviceid)))
		return FALSE;

	result = wlc_country_lookup_direct(wlc_cm->ccode, wlc_cm->regrev, &country);
	if (result != CLM_RESULT_OK)
		return FALSE;

	result = wlc_get_locale(country, &locale);
	if (result != CLM_RESULT_OK)
		return FALSE;

	result = wlc_get_flags(&locale, CLM_BAND_5G, &flags);
	if (result != CLM_RESULT_OK)
		return FALSE;

	return (ISDFS_EU(flags) || ISDFS_UK(flags));
}

bool
wlc_is_european_weather_radar_channel(struct wlc_info *wlc, chanspec_t chanspec)
{
	uint8 weather_sb[] = { 120, 124, 128 }; /* EU weather channel 20MHz sidebands */
	uint8 channel, idx, weather_len = sizeof(weather_sb)/sizeof(weather_sb[0]);

	if (!wlc_valid_chanspec_db(wlc->cmi, chanspec) || !wlc_is_dfs_eu_uk(wlc)) {
		return FALSE;
	}

	FOREACH_20_SB(chanspec, channel) {
		for (idx = 0; idx < weather_len; idx++) {
			if (channel == weather_sb[idx]) {
				return TRUE;
			}
		}
	}

	return FALSE;
}
#endif /* RADAR */

/* Lookup a country info structure from a null terminated country code
 * The lookup is case sensitive.
 */
clm_result_t
wlc_country_lookup(struct wlc_info *wlc, const char* ccode, clm_country_t *country)
{
	clm_result_t result = CLM_RESULT_ERR;
	char mapped_ccode[WLC_CNTRY_BUF_SZ];
	uint mapped_regrev;

	WL_NONE(("wl%d: %s: ccode \"%s\", SPROM \"%s\"/%u\n",
	        wlc->pub->unit, __FUNCTION__, ccode,
		wlc->cmi->cm->srom_ccode, wlc->cmi->cm->srom_regrev));

	/* map the country code to a built-in country code, regrev, and country_info struct */
	result = wlc_countrycode_map(wlc->cmi, ccode, mapped_ccode, &mapped_regrev, country);

	if (result == CLM_RESULT_OK)
		WL_NONE(("wl%d: %s: mapped to \"%s\"/%u\n",
		         wlc->pub->unit, __FUNCTION__, mapped_ccode, mapped_regrev));
	else
		WL_NONE(("wl%d: %s: failed lookup\n",
		         wlc->pub->unit, __FUNCTION__));

	return result;
}

static clm_result_t
wlc_countrycode_map(wlc_cm_info_t *wlc_cmi, const char *ccode,
	char *mapped_ccode, uint *mapped_regrev, clm_country_t *country)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	clm_result_t result = CLM_RESULT_ERR;
	uint srom_regrev = wlc_cmi->cm->srom_regrev;
	const char *srom_ccode = wlc_cmi->cm->srom_ccode;
	int mapped;

	/* check for currently supported ccode size */
	if (strlen(ccode) > (WLC_CNTRY_BUF_SZ - 1)) {
		WL_ERROR(("wl%d: %s: ccode \"%s\" too long for match\n",
		          wlc->pub->unit, __FUNCTION__, ccode));
		return CLM_RESULT_ERR;
	}

	/* default mapping is the given ccode and regrev 0 */
	strncpy(mapped_ccode, ccode, WLC_CNTRY_BUF_SZ);
	*mapped_regrev = 0;

	/* Map locale for buffalo if needed */
	if (wlc_buffalo_map_locale(wlc, ccode)) {
		strncpy(mapped_ccode, "J10", WLC_CNTRY_BUF_SZ);
		result = wlc_country_lookup_direct(mapped_ccode, *mapped_regrev, country);
		if (result == CLM_RESULT_OK)
			return result;
	}

	/* If the desired country code matches the srom country code,
	 * then the mapped country is the srom regulatory rev.
	 * Otherwise look for an aggregate mapping.
	 */
	if (!strcmp(srom_ccode, ccode)) {
		WL_NONE(("wl%d: %s: srom ccode and ccode \"%s\" match\n",
		         wlc->pub->unit, __FUNCTION__, ccode));
		*mapped_regrev = srom_regrev;
		mapped = 0;
	} else {
		mapped = wlc_country_aggregate_map(wlc_cmi, ccode, mapped_ccode, mapped_regrev);
		if (mapped)
			WL_NONE(("wl%d: %s: found aggregate mapping \"%s\"/%u\n",
			         wlc->pub->unit, __FUNCTION__, mapped_ccode, *mapped_regrev));
	}

	/* CLM 8.2, JAPAN
	 * Use the regrev=1 Japan country definition by default for chips newer than
	 * our d11 core rev 5 4306 chips, or for AP's of any vintage.
	 * For STAs with a 4306, use the legacy Japan country def "JP/0".
	 * Only do the conversion if JP/0 was not specified by a mapping or by an
	 * sprom with a regrev:
	 * Sprom has no specific info about JP's regrev if it's less than rev 3 or it does
	 * not specify "JP" as its country code =>
	 * (strcmp("JP", srom_ccode) || (wlc->pub->sromrev < 3))
	 */
	if (!strcmp("JP", mapped_ccode) && *mapped_regrev == 0 &&
	    !mapped && (strcmp("JP", srom_ccode) || (wlc->pub->sromrev < 3)) &&
	    (AP_ENAB(wlc->pub) || D11REV_GT(wlc->pub->corerev, 5))) {
		*mapped_regrev = 1;
		WL_NONE(("wl%d: %s: Using \"JP/1\" instead of legacy \"JP/0\" since we %s\n",
		         wlc->pub->unit, __FUNCTION__,
		         AP_ENAB(wlc->pub) ? "are operating as an AP" : "are newer than 4306"));
	}

	WL_NONE(("wl%d: %s: searching for country using ccode/rev \"%s\"/%u\n",
	         wlc->pub->unit, __FUNCTION__, mapped_ccode, *mapped_regrev));

	/* find the matching built-in country definition */
	result = wlc_country_lookup_direct(mapped_ccode, *mapped_regrev, country);

	/* if there is not an exact rev match, default to rev zero */
	if (result != CLM_RESULT_OK && *mapped_regrev != 0) {
		*mapped_regrev = 0;
		WL_NONE(("wl%d: %s: No country found, use base revision \"%s\"/%u\n",
		         wlc->pub->unit, __FUNCTION__, mapped_ccode, *mapped_regrev));
		result = wlc_country_lookup_direct(mapped_ccode, *mapped_regrev, country);
	}

	if (result != CLM_RESULT_OK)
		WL_NONE(("wl%d: %s: No country found, failed lookup\n",
		         wlc->pub->unit, __FUNCTION__));

	return result;
}

clm_result_t
clm_aggregate_country_lookup(const ccode_t cc, unsigned int rev, clm_agg_country_t *agg)
{
	return clm_agg_country_lookup(cc, rev, agg);
}

clm_result_t
clm_aggregate_country_map_lookup(const clm_agg_country_t agg, const ccode_t target_cc,
	unsigned int *rev)
{
	return clm_agg_country_map_lookup(agg, target_cc, rev);
}

int
wlc_is_sromccode_autocountrysupported(struct wlc_info *wlc)
{
	clm_result_t result;
	clm_agg_country_t agg = 0;
	const char *srom_ccode = wlc->cmi->cm->srom_ccode;
	uint srom_regrev = wlc->cmi->cm->srom_regrev;

	/* Check for a match in the aggregate country list */
	WL_NONE(("wl%d: %s: searching for agg map for srom ccode/rev \"%s\"/%u\n",
		wlc->pub->unit, __FUNCTION__, srom_ccode, srom_regrev));

	result = clm_aggregate_country_lookup(srom_ccode, srom_regrev, &agg);

	if (result != CLM_RESULT_OK)
		WL_NONE(("wl%d: %s: no map for \"%s\"/%u\n",
		wlc->pub->unit, __FUNCTION__, srom_ccode, srom_regrev));
	else
		WL_NONE(("wl%d: %s: found map for \"%s\"/%u\n",
		wlc->pub->unit, __FUNCTION__, srom_ccode, srom_regrev));

	return (result == CLM_RESULT_OK);
}

static int
wlc_country_aggregate_map(wlc_cm_info_t *wlc_cmi, const char *ccode,
                          char *mapped_ccode, uint *mapped_regrev)
{
#ifdef BCMDBG
	wlc_info_t *wlc = wlc_cmi->wlc;
#endif // endif
	clm_result_t result;
	clm_agg_country_t agg = 0;
	const char *srom_ccode = wlc_cmi->cm->srom_ccode;
	uint srom_regrev = wlc_cmi->cm->srom_regrev;

	/* Use "ww", WorldWide, for the lookup value for '\0\0' */
	if (srom_ccode[0] == '\0')
		srom_ccode = "ww";

	/* Check for a match in the aggregate country list */
	WL_NONE(("wl%d: %s: searching for agg map for srom ccode/rev \"%s\"/%u\n",
	         wlc->pub->unit, __FUNCTION__, srom_ccode, srom_regrev));

	result = clm_aggregate_country_lookup(srom_ccode, srom_regrev, &agg);

	if (result != CLM_RESULT_OK)
		WL_NONE(("wl%d: %s: no map for \"%s\"/%u\n",
		         wlc->pub->unit, __FUNCTION__, srom_ccode, srom_regrev));
	else
		WL_NONE(("wl%d: %s: found map for \"%s\"/%u\n",
		         wlc->pub->unit, __FUNCTION__, srom_ccode, srom_regrev));

	if (result == CLM_RESULT_OK) {
		result = clm_aggregate_country_map_lookup(agg, ccode, mapped_regrev);
		strncpy(mapped_ccode, ccode, WLC_CNTRY_BUF_SZ);
	}

	return (result == CLM_RESULT_OK);
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_dump_country_aggregate_map(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b)
{
	const char *cur_ccode = wlc_cmi->cm->ccode;
	uint cur_regrev = (uint8)wlc_cmi->cm->regrev;
	clm_agg_country_t agg = 0;
	clm_result_t result;
	int agg_iter;

	/* Use "ww", WorldWide, for the lookup value for '\0\0' */
	if (cur_ccode[0] == '\0')
		cur_ccode = "ww";

	clm_iter_init(&agg_iter);
	if ((result = clm_aggregate_country_lookup(cur_ccode, cur_regrev, &agg)) == CLM_RESULT_OK) {
		clm_agg_map_t map_iter;
		ccode_t cc;
		unsigned int rev;

		bcm_bprintf(b, "Map for %s/%u ->\n", cur_ccode, cur_regrev);
		clm_iter_init(&map_iter);
		while ((result = clm_agg_map_iter(agg, &map_iter, cc, &rev)) == CLM_RESULT_OK) {
			bcm_bprintf(b, "%c%c/%u\n", cc[0], cc[1], rev);
		}
	} else {
		bcm_bprintf(b, "No lookaside table for %s/%u\n", cur_ccode, cur_regrev);
	}
	return 0;

}
#endif /* BCMDBG || BCMDBG_DUMP */

/* Lookup a country info structure from a null terminated country
 * abbreviation and regrev directly with no translation.
 */
clm_result_t
wlc_country_lookup_direct(const char* ccode, uint regrev, clm_country_t *country)
{
	return clm_country_lookup(ccode, regrev, country);
}

#if defined(STA) && defined(WL11D)
/* Lookup a country info structure considering only legal country codes as found in
 * a Country IE; two ascii alpha followed by " ", "I", or "O".
 * Do not match any user assigned application specifc codes that might be found
 * in the driver table.
 */
clm_result_t
wlc_country_lookup_ext(wlc_info_t *wlc, const char *ccode, clm_country_t *country)
{
	clm_result_t result = CLM_RESULT_NOT_FOUND;
	char country_str_lookup[WLC_CNTRY_BUF_SZ] = { 0 };

	/* only allow ascii alpha uppercase for the first 2 chars, and " ", "I", "O" for the 3rd */
	if (!((0x80 & ccode[0]) == 0 && bcm_isupper(ccode[0]) &&
	      (0x80 & ccode[1]) == 0 && bcm_isupper(ccode[1]) &&
	      (ccode[2] == ' ' || ccode[2] == 'I' || ccode[2] == 'O')))
		return result;

	/* for lookup in the driver table of country codes, only use the first
	 * 2 chars, ignore the 3rd character " ", "I", "O" qualifier
	 */
	country_str_lookup[0] = ccode[0];
	country_str_lookup[1] = ccode[1];

	/* do not match ISO 3166-1 user assigned country codes that may be in the driver table */
	if (!strcmp("AA", country_str_lookup) ||	/* AA */
	    !strcmp("ZZ", country_str_lookup) ||	/* ZZ */
	    country_str_lookup[0] == 'X' ||		/* XA - XZ */
	    (country_str_lookup[0] == 'Q' &&		/* QM - QZ */
	     (country_str_lookup[1] >= 'M' && country_str_lookup[1] <= 'Z')))
		return result;

#ifdef MACOSX
	if (!strcmp("NA", country_str_lookup))
		return result;
#endif /* MACOSX */

	return wlc_country_lookup(wlc, country_str_lookup, country);
}
#endif /* STA && WL11D */

static void
wlc_channel_set_radar_chanvect(wlc_cm_data_t *wlc_cm, wlcband_t *band, uint16 flags)
{
	uint j;
	const uint8 *vec = NULL;

	/* Return when No Flag for DFS TPC */
	if (!(flags & WLC_DFS_TPC)) {
		return;
	}

	if (ISDFS_UK(flags)) {
		vec = radar_set_uk.vec;
	} else if (flags & WLC_DFS_TPC) {
		vec = radar_set1.vec;
	}

	for (j = 0; j < sizeof(chanvec_t); j++) {
		wlc_cm->bandstate[band->bandunit].radar_channels->vec[j] =
			vec[j] &
			wlc_cm->bandstate[band->bandunit].
			valid_channels.vec[j];
	}
}

static int
wlc_channels_init(wlc_cm_info_t *wlc_cmi, clm_country_t country)
{
	clm_country_locales_t locale;
	uint16 flags;
	wlc_info_t *wlc = wlc_cmi->wlc;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	uint i, j;
	wlcband_t * band;
	chanvec_t sup_chan, temp_chan;
	bool switch_to_global_opclass = FALSE;

#if defined(WL_GLOBAL_RCLASS)
	switch_to_global_opclass = TRUE;
#endif // endif

	bzero(&wlc_cm->restricted_channels, sizeof(chanvec_t));
	bzero(&wlc_cm->locale_radar_channels, sizeof(chanvec_t));
	bzero(&wlc_cm->allowed_5g_20channels, sizeof(chanvec_t));
	bzero(&wlc_cm->allowed_5g_4080channels, sizeof(chanvec_t));

	band = wlc->band;
	for (i = 0; i < NBANDS(wlc); i++, band = wlc->bandstate[OTHERBANDUNIT(wlc)]) {
		clm_result_t result = wlc_get_locale(country, &locale);
		clm_band_t tmp_band;
		if (result == CLM_RESULT_OK) {
			if (BAND_5G(band->bandtype)) {
				tmp_band = CLM_BAND_5G;
			} else {
				tmp_band = CLM_BAND_2G;
			}
			result = wlc_get_flags(&locale, tmp_band, &flags);
			wlc_cm->bandstate[band->bandunit].locale_flags = flags;

			wlc_locale_get_channels(&locale, tmp_band,
				&wlc_cm->bandstate[band->bandunit].valid_channels,
				&temp_chan);
			/* initialize restricted channels */
			for (j = 0; j < sizeof(chanvec_t); j++) {
				wlc_cm->restricted_channels.vec[j] |= temp_chan.vec[j];
			}
#ifdef BAND5G /* RADAR */
			wlc_cm->bandstate[band->bandunit].radar_channels =
				&wlc_cm->locale_radar_channels;
			if (BAND_5G(band->bandtype)) {
				wlc_channel_set_radar_chanvect(wlc_cm, band, flags);
			}

			if (BAND_5G(band->bandtype)) {
				clm_valid_channels_5g(&locale,
				(clm_channels_t*)&wlc_cm->allowed_5g_20channels,
				(clm_channels_t*)&wlc_cm->allowed_5g_4080channels);
			 }
#endif /* BAND5G */

			/* set the channel availability,
			 * masking out the channels that may not be supported on this phy
			 */
			phy_utils_chanspec_band_validch(
				(phy_info_t *)WLC_PI_BANDUNIT(wlc, band->bandunit),
				band->bandtype, &sup_chan);
			wlc_locale_get_channels(&locale, tmp_band,
				&wlc_cm->bandstate[band->bandunit].valid_channels,
				&temp_chan);
			for (j = 0; j < sizeof(chanvec_t); j++)
				wlc_cm->bandstate[band->bandunit].valid_channels.vec[j] &=
					sup_chan.vec[j];
		}
	}

	wlc_upd_restricted_chanspec_flag(wlc_cmi);
	wlc_quiet_channels_reset(wlc_cmi);
	wlc_channels_commit(wlc_cmi);
	wlc_update_rcinfo(wlc_cmi, switch_to_global_opclass);

	return (0);
}

/* Update the radio state (enable/disable) and tx power targets
 * based on a new set of channel/regulatory information
 */
static void
wlc_channels_commit(wlc_cm_info_t *wlc_cmi)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	wlc_phy_t *pi = WLC_PI(wlc);
	uint chan;
	ppr_t* txpwr;
	/* search for the existence of any valid channel */
	for (chan = 0; chan < MAXCHANNEL; chan++) {
		if (VALID_CHANNEL20_DB(wlc, chan)) {
			break;
		}
	}
	if (chan == MAXCHANNEL)
		chan = INVCHANNEL;

	/* based on the channel search above, set or clear WL_RADIO_COUNTRY_DISABLE */
	if (chan == INVCHANNEL) {
		/* country/locale with no valid channels, set the radio disable bit */
		mboolset(wlc->pub->radio_disabled, WL_RADIO_COUNTRY_DISABLE);
		WL_ERROR(("wl%d: %s: no valid channel for \"%s\" nbands %d bandlocked %d\n",
		          wlc->pub->unit, __FUNCTION__,
		          wlc_cmi->cm->country_abbrev, NBANDS(wlc), wlc->bandlocked));
	} else if (mboolisset(wlc->pub->radio_disabled, WL_RADIO_COUNTRY_DISABLE)) {
		/* country/locale with valid channel, clear the radio disable bit */
		mboolclr(wlc->pub->radio_disabled, WL_RADIO_COUNTRY_DISABLE);
	}

	/* Now that the country abbreviation is set, if the radio supports 2G, then
	 * set channel 14 restrictions based on the new locale.
	 */
	if (NBANDS(wlc) > 1 || BAND_2G(wlc->band->bandtype)) {
		wlc_phy_chanspec_ch14_widefilter_set(pi, wlc_japan(wlc) ? TRUE : FALSE);
	}

	if (wlc->pub->up && chan != INVCHANNEL) {
		/* recompute tx power for new country info */

		/* XXX REVISIT johnvb  What chanspec should we use when changing country
		 * and where do we get it if we don't create/set it ourselves, i.e.
		 * wlc's chanspec or the phy's chanspec?  Also see the REVISIT comment
		 * in wlc_channel_set_txpower_limit().
		 */

		/* Where do we get a good chanspec? wlc, phy, set it ourselves? */

		if ((txpwr = ppr_create(wlc_cmi->pub->osh, PPR_CHSPEC_BW(wlc->chanspec))) == NULL) {
			return;
		}

		wlc_channel_reg_limits(wlc_cmi, wlc->chanspec, txpwr);

		/* XXX REVISIT johvnb  When setting a new country is it OK to erase
		 * the current 11h/local constraint?  If we do need to maintain the 11h
		 * constraint, we could either cache it here in the channel code or just
		 * have the higher level, non wlc_channel.c code, reissue a call to
		 * wlc_channel_set_txpower_limit with the old constraint.  This requires
		 * us to identify which of the few wlc_[channel]_set_countrycode calls could
		 * have a previous constraint constraint active.  This raises the
		 * related question of how a constraint goes away?  Caching the constraint
		 * here in wlc_channel would be logical, but unfortunately it doesn't
		 * currently have its own structure/state.
		 */
		ppr_apply_max(txpwr, WLC_TXPWR_MAX);
		/* Where do we get a good chanspec? wlc, phy, set it ourselves? */
		wlc_phy_txpower_limit_set(pi, txpwr, wlc->chanspec);

		ppr_delete(wlc_cmi->pub->osh, txpwr);
	}
}
chanvec_t *
wlc_quiet_chanvec_get(wlc_cm_info_t *wlc_cmi)
{
	return &wlc_cmi->cm->quiet_channels;
}

chanvec_t *
wlc_valid_chanvec_get(wlc_cm_info_t *wlc_cmi, uint bandunit)
{
	return &wlc_cmi->cm->bandstate[bandunit].valid_channels;
}

/* reset the quiet channels vector to the union of the restricted and radar channel sets */
void
wlc_quiet_channels_reset(wlc_cm_info_t *wlc_cmi)
{
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
#ifdef BAND5G
	wlc_info_t *wlc = wlc_cmi->wlc;
	uint i;
	wlcband_t *band;
#endif /* BAND5G */

	/* initialize quiet channels for restricted channels */
	bcopy(&wlc_cm->restricted_channels, &wlc_cm->quiet_channels, sizeof(chanvec_t));

#ifdef BAND5G /* RADAR */
	band = wlc->band;
	for (i = 0; i < NBANDS(wlc); i++, band = wlc->bandstate[OTHERBANDUNIT(wlc)]) {
		/* initialize quiet channels for radar if we are in spectrum management mode */
		if (WL11H_ENAB(wlc)) {
			uint j;
			const chanvec_t *chanvec;

			chanvec = wlc_cm->bandstate[band->bandunit].radar_channels;
			for (j = 0; j < sizeof(chanvec_t); j++)
				wlc_cm->quiet_channels.vec[j] |= chanvec->vec[j];
		}
	}
#endif /* BAND5G */
}

bool
wlc_quiet_chanspec(wlc_cm_info_t *wlc_cmi, chanspec_t chspec)
{
	wlc_pub_t *pub = wlc_cmi->wlc->pub;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	uint8 channel = CHSPEC_CHANNEL(chspec);

	if (VHT_ENAB(pub) && (CHSPEC_IS8080(chspec) || CHSPEC_IS160(chspec))) {
		uint8 primary_80;
		uint8 secondary_80;
		primary_80 = wf_chspec_primary80_channel(chspec);
		secondary_80 = wf_chspec_secondary80_channel(chspec);

		return (
		isset(wlc_cm->quiet_channels.vec, LL_20_SB(primary_80)) ||
		isset(wlc_cm->quiet_channels.vec, LU_20_SB(primary_80)) ||
		isset(wlc_cm->quiet_channels.vec, UL_20_SB(primary_80)) ||
		isset(wlc_cm->quiet_channels.vec, UU_20_SB(primary_80)) ||
		isset(wlc_cm->quiet_channels.vec, LL_20_SB(secondary_80)) ||
		isset(wlc_cm->quiet_channels.vec, LU_20_SB(secondary_80)) ||
		isset(wlc_cm->quiet_channels.vec, UL_20_SB(secondary_80)) ||
		isset(wlc_cm->quiet_channels.vec, UU_20_SB(secondary_80)));
	}
	else if (VHT_ENAB(pub) && CHSPEC_IS80(chspec)) {
		return (
		isset(wlc_cm->quiet_channels.vec, LL_20_SB(channel)) ||
		isset(wlc_cm->quiet_channels.vec, LU_20_SB(channel)) ||
		isset(wlc_cm->quiet_channels.vec, UL_20_SB(channel)) ||
		isset(wlc_cm->quiet_channels.vec, UU_20_SB(channel)));
	} else if ((VHT_ENAB(pub) || N_ENAB(pub)) && CHSPEC_IS40(chspec)) {
		return (
		isset(wlc_cm->quiet_channels.vec, LOWER_20_SB(channel)) ||
		isset(wlc_cm->quiet_channels.vec, UPPER_20_SB(channel)));
	} else {
		return isset(wlc_cm->quiet_channels.vec, channel);
	}
}

void
wlc_set_quiet_chanspec_exclude(wlc_cm_info_t *wlc_cmi, chanspec_t chspec, chanspec_t chspec_exclude)
{
	uint8 channel, i, idx = 0, exclude_arr[8];
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	bool must_exclude;

	FOREACH_20_SB(chspec_exclude, channel) {
		exclude_arr[idx++] = channel;
	}

	FOREACH_20_SB(chspec, channel) {
		must_exclude = FALSE;
		for (i = 0; i < idx; ++i) {
			if (exclude_arr[i] == channel) {
				must_exclude = TRUE;
				break;
			}
		}

		if (!must_exclude && wlc_radar_chanspec(wlc_cmi, CH20MHZ_CHSPEC(channel))) {
			setbit(wlc_cm->quiet_channels.vec, channel);
			WL_REGULATORY(("%s: Setting quiet bit for channel %d of chanspec 0x%x \n",
				__FUNCTION__, channel, chspec));
		}
	}
}

void
wlc_set_quiet_chanspec(wlc_cm_info_t *wlc_cmi, chanspec_t chspec)
{
	wlc_pub_t *pub = wlc_cmi->wlc->pub;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	uint8 channel = CHSPEC_CHANNEL(chspec);

	if (VHT_ENAB(pub) && (CHSPEC_IS8080(chspec) || CHSPEC_IS160(chspec))) {
		uint8 primary_80;
		uint8 secondary_80;
		primary_80 = wf_chspec_primary80_channel(chspec);
		secondary_80 = wf_chspec_secondary80_channel(chspec);

		setbit(wlc_cm->quiet_channels.vec, LL_20_SB(primary_80));
		setbit(wlc_cm->quiet_channels.vec, LU_20_SB(primary_80));
		setbit(wlc_cm->quiet_channels.vec, UL_20_SB(primary_80));
		setbit(wlc_cm->quiet_channels.vec, UU_20_SB(primary_80));
		setbit(wlc_cm->quiet_channels.vec, LL_20_SB(secondary_80));
		setbit(wlc_cm->quiet_channels.vec, LU_20_SB(secondary_80));
		setbit(wlc_cm->quiet_channels.vec, UL_20_SB(secondary_80));
		setbit(wlc_cm->quiet_channels.vec, UU_20_SB(secondary_80));
	}
	else if (VHT_ENAB(pub) && CHSPEC_IS80(chspec)) {
		setbit(wlc_cm->quiet_channels.vec, LL_20_SB(channel));
		setbit(wlc_cm->quiet_channels.vec, LU_20_SB(channel));
		setbit(wlc_cm->quiet_channels.vec, UL_20_SB(channel));
		setbit(wlc_cm->quiet_channels.vec, UU_20_SB(channel));
	} else if ((VHT_ENAB(pub) || N_ENAB(pub)) && CHSPEC_IS40(chspec)) {
		setbit(wlc_cm->quiet_channels.vec, LOWER_20_SB(channel));
		setbit(wlc_cm->quiet_channels.vec, UPPER_20_SB(channel));
	} else {
		setbit(wlc_cm->quiet_channels.vec, channel);
	}
}

void
wlc_clr_quiet_chanspec(wlc_cm_info_t *wlc_cmi, chanspec_t chspec)
{
	wlc_pub_t *pub = wlc_cmi->wlc->pub;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	uint8 channel = CHSPEC_CHANNEL(chspec);

	if (VHT_ENAB(pub) && (CHSPEC_IS8080(chspec) || CHSPEC_IS160(chspec))) {
		uint8 primary_80;
		uint8 secondary_80;
		primary_80 = wf_chspec_primary80_channel(chspec);
		secondary_80 = wf_chspec_secondary80_channel(chspec);

		clrbit(wlc_cm->quiet_channels.vec, LL_20_SB(primary_80));
		clrbit(wlc_cm->quiet_channels.vec, LU_20_SB(primary_80));
		clrbit(wlc_cm->quiet_channels.vec, UL_20_SB(primary_80));
		clrbit(wlc_cm->quiet_channels.vec, UU_20_SB(primary_80));
		clrbit(wlc_cm->quiet_channels.vec, LL_20_SB(secondary_80));
		clrbit(wlc_cm->quiet_channels.vec, LU_20_SB(secondary_80));
		clrbit(wlc_cm->quiet_channels.vec, UL_20_SB(secondary_80));
		clrbit(wlc_cm->quiet_channels.vec, UU_20_SB(secondary_80));
	}
	else if (VHT_ENAB(pub) && CHSPEC_IS80(chspec)) {
		clrbit(wlc_cm->quiet_channels.vec, LL_20_SB(channel));
		clrbit(wlc_cm->quiet_channels.vec, LU_20_SB(channel));
		clrbit(wlc_cm->quiet_channels.vec, UL_20_SB(channel));
		clrbit(wlc_cm->quiet_channels.vec, UU_20_SB(channel));
	} else if ((VHT_ENAB(pub) || N_ENAB(pub)) && CHSPEC_IS40(chspec)) {
		clrbit(wlc_cm->quiet_channels.vec, LOWER_20_SB(channel));
		clrbit(wlc_cm->quiet_channels.vec, UPPER_20_SB(channel));
	} else {
		clrbit(wlc_cm->quiet_channels.vec, channel);
	}
}

/* Is the channel valid for the current locale? (but don't consider channels not
 *   available due to bandlocking)
 */
bool
wlc_valid_channel2p5_db(wlc_cm_info_t *wlc_cmi, uint val)
{
	wlc_info_t *wlc = wlc_cmi->wlc;

	return (VALID_CHANNEL2P5(wlc, val) ||
		(!wlc->bandlocked && VALID_CHANNEL2P5_IN_BAND(wlc, OTHERBANDUNIT(wlc), val)));
}

/* Is the channel valid for the current locale? (but don't consider channels not
 *   available due to bandlocking)
 */
bool
wlc_valid_channel5_db(wlc_cm_info_t *wlc_cmi, uint val)
{
	wlc_info_t *wlc = wlc_cmi->wlc;

	return (VALID_CHANNEL5(wlc, val) ||
		(!wlc->bandlocked && VALID_CHANNEL5_IN_BAND(wlc, OTHERBANDUNIT(wlc), val)));
}

/* Is the channel valid for the current locale? (but don't consider channels not
 *   available due to bandlocking)
 */
bool
wlc_valid_channel10_db(wlc_cm_info_t *wlc_cmi, uint val)
{
	wlc_info_t *wlc = wlc_cmi->wlc;

	return (VALID_CHANNEL10(wlc, val) ||
		(!wlc->bandlocked && VALID_CHANNEL10_IN_BAND(wlc, OTHERBANDUNIT(wlc), val)));
}

/* Is the channel valid for the current locale? (but don't consider channels not
 *   available due to bandlocking)
 */
bool
wlc_valid_channel20_db(wlc_cm_info_t *wlc_cmi, uint val)
{
	wlc_info_t *wlc = wlc_cmi->wlc;

	return (VALID_CHANNEL20(wlc, val) ||
		(!wlc->bandlocked && VALID_CHANNEL20_IN_BAND(wlc, OTHERBANDUNIT(wlc), val)));
}

/* Is the channel valid for the current locale and specified band? */
bool
wlc_valid_channel2p5_in_band(wlc_cm_info_t *wlc_cmi, uint bandunit, uint val)
{
	return ((val < MAXCHANNEL) &&
		isset(wlc_cmi->cm->bandstate[bandunit].valid_channels.vec, val));
}

/* Is the channel valid for the current locale and specified band? */
bool
wlc_valid_channel5_in_band(wlc_cm_info_t *wlc_cmi, uint bandunit, uint val)
{
	return ((val < MAXCHANNEL) &&
		isset(wlc_cmi->cm->bandstate[bandunit].valid_channels.vec, val));
}

/* Is the channel valid for the current locale and specified band? */
bool
wlc_valid_channel10_in_band(wlc_cm_info_t *wlc_cmi, uint bandunit, uint val)
{
	return ((val < MAXCHANNEL) &&
		isset(wlc_cmi->cm->bandstate[bandunit].valid_channels.vec, val));
}

/* Is the channel valid for the current locale and specified band? */
bool
wlc_valid_channel20_in_band(wlc_cm_info_t *wlc_cmi, uint bandunit, uint val)
{
	return ((val < MAXCHANNEL) &&
		isset(wlc_cmi->cm->bandstate[bandunit].valid_channels.vec, val));
}

/* Is the channel valid for the current locale and current band? */
bool
wlc_valid_channel2p5(wlc_cm_info_t *wlc_cmi, uint val)
{
	wlc_info_t *wlc = wlc_cmi->wlc;

	return ((val < MAXCHANNEL) &&
		isset(wlc_cmi->cm->bandstate[wlc->band->bandunit].valid_channels.vec, val));
}

/* Is the channel valid for the current locale and current band? */
bool
wlc_valid_channel5(wlc_cm_info_t *wlc_cmi, uint val)
{
	wlc_info_t *wlc = wlc_cmi->wlc;

	return ((val < MAXCHANNEL) &&
		isset(wlc_cmi->cm->bandstate[wlc->band->bandunit].valid_channels.vec, val));
}

/* Is the channel valid for the current locale and current band? */
bool
wlc_valid_channel10(wlc_cm_info_t *wlc_cmi, uint val)
{
	wlc_info_t *wlc = wlc_cmi->wlc;

	return ((val < MAXCHANNEL) &&
		isset(wlc_cmi->cm->bandstate[wlc->band->bandunit].valid_channels.vec, val));
}

/* Is the channel valid for the current locale and current band? */
bool
wlc_valid_channel20(wlc_cm_info_t *wlc_cmi, uint val)
{
	wlc_info_t *wlc = wlc_cmi->wlc;

	return ((val < MAXCHANNEL) &&
		isset(wlc_cmi->cm->bandstate[wlc->band->bandunit].valid_channels.vec, val));
}

/* Is the 40 MHz allowed for the current locale and specified band? */
bool
wlc_valid_40chanspec_in_band(wlc_cm_info_t *wlc_cmi, uint bandunit)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;

	return (((wlc_cm->bandstate[bandunit].locale_flags & (WLC_NO_MIMO | WLC_NO_40MHZ)) == 0) &&
	        WL_BW_CAP_40MHZ(wlc->bandstate[bandunit]->bw_cap));
}

/* Is 80 MHz allowed for the current locale and specified band? */
bool
wlc_valid_80chanspec_in_band(wlc_cm_info_t *wlc_cmi, uint bandunit)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;

	return (((wlc_cm->bandstate[bandunit].locale_flags & (WLC_NO_MIMO | WLC_NO_80MHZ)) == 0) &&
	        WL_BW_CAP_80MHZ(wlc->bandstate[bandunit]->bw_cap));
}

/* Is chanspec allowed to use 80Mhz bandwidth for the current locale? */
bool
wlc_valid_80chanspec(struct wlc_info *wlc, chanspec_t chanspec)
{
	uint16 locale_flags;

	locale_flags = wlc_channel_locale_flags_in_band((wlc)->cmi, CHSPEC_WLCBANDUNIT(chanspec));

	return (VHT_ENAB_BAND((wlc)->pub, (CHSPEC2WLC_BAND(chanspec))) &&
	 !(locale_flags & WLC_NO_80MHZ) &&
	 WL_BW_CAP_80MHZ((wlc)->bandstate[BAND_5G_INDEX]->bw_cap) &&
	 wlc_valid_chanspec_db((wlc)->cmi, (chanspec)));
}

/* Is 80+80 MHz allowed for the current locale and specified band? */
bool
wlc_valid_8080chanspec_in_band(wlc_cm_info_t *wlc_cmi, uint bandunit)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;

	return (((wlc_cm->bandstate[bandunit].locale_flags & (WLC_NO_MIMO | WLC_NO_160MHZ)) == 0) &&
	        WL_BW_CAP_160MHZ(wlc->bandstate[bandunit]->bw_cap));
}

/* Is chanspec allowed to use 80+80Mhz bandwidth for the current locale? */
bool
wlc_valid_8080chanspec(struct wlc_info *wlc, chanspec_t chanspec)
{
	uint16 locale_flags;

	locale_flags = wlc_channel_locale_flags_in_band((wlc)->cmi, CHSPEC_WLCBANDUNIT(chanspec));

	return (VHT_ENAB_BAND((wlc)->pub, (CHSPEC2WLC_BAND(chanspec))) &&
	 !(locale_flags & WLC_NO_160MHZ) &&
	 WL_BW_CAP_160MHZ((wlc)->bandstate[BAND_5G_INDEX]->bw_cap) &&
	 wlc_valid_chanspec_db((wlc)->cmi, (chanspec)));
}

/* Is 160 MHz allowed for the current locale and specified band? */
bool
wlc_valid_160chanspec_in_band(wlc_cm_info_t *wlc_cmi, uint bandunit)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;

	return (((wlc_cm->bandstate[bandunit].locale_flags & (WLC_NO_MIMO | WLC_NO_160MHZ)) == 0) &&
	        WL_BW_CAP_160MHZ(wlc->bandstate[bandunit]->bw_cap));
}

/* Is chanspec allowed to use 160Mhz bandwidth for the current locale? */
bool
wlc_valid_160chanspec(struct wlc_info *wlc, chanspec_t chanspec)
{
	uint16 locale_flags;

	locale_flags = wlc_channel_locale_flags_in_band((wlc)->cmi, CHSPEC_WLCBANDUNIT(chanspec));

	return (VHT_ENAB_BAND((wlc)->pub, (CHSPEC2WLC_BAND(chanspec))) &&
	 !(locale_flags & WLC_NO_160MHZ) &&
	 WL_BW_CAP_160MHZ((wlc)->bandstate[BAND_5G_INDEX]->bw_cap) &&
	 wlc_valid_chanspec_db((wlc)->cmi, (chanspec)));
}

static void
wlc_channel_txpower_limits(wlc_cm_info_t *wlc_cmi, ppr_t *txpwr)
{
	uint8 local_constraint;
	wlc_info_t *wlc = wlc_cmi->wlc;

	wlc_channel_reg_limits(wlc_cmi, wlc->chanspec, txpwr);

	local_constraint = wlc_tpc_get_local_constraint_qdbm(wlc->tpc);

	if (!AP_ONLY(wlc->pub)) {
		ppr_apply_constraint_total_tx(txpwr, local_constraint);
	}
}

static void
wlc_channel_spurwar_locale(wlc_cm_info_t *wlc_cm, chanspec_t chanspec)
{
#if ACCONF
	wlc_info_t *wlc = wlc_cm->wlc;
	int override;
	uint rev;
	bool isCN, isX2, isX51A, dBpad;

	isX51A = (wlc->pub->sih->boardtype == BCM94360X51A) ? TRUE : FALSE;
	dBpad = (wlc->pub->boardflags4 & BFL4_SROM12_4dBPAD) ? 1 : 0;
	if (!isX51A && !dBpad)
		return;

	isCN = bcmp("CN", wlc_channel_country_abbrev(wlc_cm), 2) ? FALSE : TRUE;
	isX2 = bcmp("X2", wlc_channel_country_abbrev(wlc_cm), 2) ? FALSE : TRUE;
	rev = wlc_channel_regrev(wlc_cm);

	wlc_iovar_getint(wlc, "phy_afeoverride", &override);
	override &= ~PHY_AFE_OVERRIDE_DRV;
	wlc->stf->coremask_override = SPURWAR_OVERRIDE_OFF;
	if (CHSPEC_IS5G(chanspec) && (isX51A || dBpad) &&
	    ((isCN && rev == 37) || (isX2 && rev == 20) || /* X87 module */
	     (isCN && rev == 40) || (isX2 && rev == 19))) { /* X51A module */
		override |= PHY_AFE_OVERRIDE_DRV;
		if (isX51A || CHSPEC_IS20(chanspec))
			wlc->stf->coremask_override = SPURWAR_OVERRIDE_X51A;
		else if (dBpad)
			wlc->stf->coremask_override = SPURWAR_OVERRIDE_DBPAD;
	}
	wlc_iovar_setint(wlc, "phy_afeoverride", override);
#endif /* ACCONF */
}

void
wlc_channel_set_chanspec(wlc_cm_info_t *wlc_cmi, chanspec_t chanspec)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	wlc_stf_t *stf = wlc->stf;
	wlc_bsscfg_t *bsscfg = wlc->cfg;
	ppr_t* txpwr;
	int8 local_constraint_qdbm;
	bool update_cap = FALSE;
#if defined WLTXPWR_CACHE && defined(WLC_LOW) && defined(WL11N)
	tx_pwr_cache_entry_t* cacheptr = wlc_phy_get_txpwr_cache(WLC_PI(wlc));
#endif // endif

	/* Not to allow change in Op_txstreams when scan is in progress */
	if (!SCAN_IN_PROGRESS(wlc->scan)) {
		/* bw 160/80p80MHz */
		if (WLC_PHY_AS_80P80(wlc, chanspec)) {
			/* For 160Mhz or 80+80Mhz, if phy is 80p80. Only 2x2 or 1x1 are avaiable.
			 * We also need to adjust the mcsmap, 4->2, 2->1.
			 * txstreams and rxstreams must be at least 2.
			 */
			ASSERT((stf->txstreams > 1) && (stf->rxstreams > 1));
			/* Number of tx and rx streams must be even to combine
			 * two 80 streams for 160Mhz.
			 */
			ASSERT((stf->txstreams & 1) == 0 && (stf->rxstreams & 1) == 0);
			if (stf->op_txstreams >= 2) {
				stf->op_txstreams = stf->txstreams >> 1;
				stf->op_rxstreams = stf->rxstreams >> 1;
				update_cap = TRUE;
			}
		}
#ifdef DYN160
		/* When DYN160 is not enabled, NSS is limited to 2 */
		else if (!DYN160_ACTIVE(wlc->pub) && WL_BW_CAP_160MHZ(wlc->band->bw_cap) &&
				BSSCFG_STA(bsscfg)) {
			if (stf->op_txstreams >= 2) {
				stf->op_txstreams = stf->txstreams >> 1;
				stf->op_rxstreams = stf->rxstreams >> 1;
				update_cap = TRUE;
			}
		}
		/* bw <= 80MHz with DYN160 */
		else if (DYN160_ACTIVE(wlc->pub) &&
				((stf->op_txstreams == (stf->txstreams / 2)) &&
				(stf->op_rxstreams == (stf->rxstreams / 2)))) {
			/* If previous chanspec is 160Mhz or 80+80Mhz, restore the original
			 * txstreams and rxstreams for 80Mhz, 40Mhz, and 20Mhz.
			 */
			stf->op_txstreams = stf->txstreams;
			stf->op_rxstreams = stf->rxstreams;
			update_cap = TRUE;
		}
#endif /* DYN160 */
		/* bw <= 80MHz */
		else if ((stf->op_txstreams == (stf->txstreams / 2)) ||
				(stf->op_rxstreams == (stf->rxstreams / 2))) {
			stf->op_txstreams = stf->txstreams;
			stf->op_rxstreams = stf->rxstreams;
			update_cap = TRUE;
		}

#ifdef WL_MODESW
		/* Coumpute the oper_mode based on Chanspec and Streams */
		bsscfg->oper_mode = wlc_modesw_derive_opermode(wlc->modesw, chanspec,
				bsscfg, stf->op_rxstreams);
		if (BSSCFG_AP(bsscfg)) {
			if ((CHSPEC_IS160(chanspec) || WL_BW_CAP_160MHZ(wlc->band->bw_cap)) &&
					DYN160_ACTIVE(wlc->pub)) {
				bsscfg->oper_mode_enabled = TRUE;
			}
		} else {
			/* For STA oper_mode is NOT Enabled */
			bsscfg->oper_mode_enabled = FALSE;
		}
#endif /* WL_MODESW */
	}

	WL_RATE(("wl%d: ch 0x%04x, u:%d, "
			"htc 0x%x, hrc 0x%x, htcc 0x%x, hrcc 0x%x, tc 0x%x, rc 0x%x, "
			"ts 0x%x, rs 0x%x, ots 0x%x, ors 0x%x, op_en:%d, op_m:0x%02x\n",
			WLCWLUNIT(wlc), chanspec, update_cap, stf->hw_txchain, stf->hw_rxchain,
			stf->hw_txchain_cap, stf->hw_rxchain_cap, stf->txchain, stf->rxchain,
			stf->txstreams, stf->rxstreams, stf->op_txstreams, stf->op_rxstreams,
			bsscfg->oper_mode_enabled, bsscfg->oper_mode));

	if (update_cap) {
		wlc_ht_stbc_tx_set(wlc->hti, wlc->band->band_stf_stbc_tx);
		WL_INFORM(("Update STBC TX for VHT and HT cap for chanspec(0x%x)\n", chanspec));
		wlc_default_rateset(wlc, &bsscfg->current_bss->rateset);
	}

#if defined WLTXPWR_CACHE && defined(WLC_LOW) && defined(WL11N)
	if (wlc_phy_txpwr_cache_is_cached(cacheptr, chanspec) != TRUE) {
		int result;
		chanspec_t kill_chan = 0;

		BCM_REFERENCE(result);

		if (last_chanspec != 0)
			kill_chan = wlc_phy_txpwr_cache_find_other_cached_chanspec(cacheptr,
				last_chanspec);

		if (kill_chan != 0) {
#ifndef WLC_LOW
			ppr_t* txpwr_offsets;
			txpwr_offsets = wlc_phy_get_cached_pwr(cacheptr, kill_chan,
				TXPWR_CACHE_STF_OFFSETS);
			/* allow stf to delete its offsets */
			if ((txpwr_offsets != NULL) && (txpwr_offsets == stf->txpwr_ctl)) {
				wlc_phy_uncache_pwr(cacheptr, TXPWR_CACHE_STF_OFFSETS,
					txpwr_offsets);
			}
#else
			/* For dualband chips, we have to set pi->tx_power_offset to NULL before
			 * deleting the memory in cache so released tx_power_offset won't
			 * be referenced during band switching.
			 */
			if (NBANDS(wlc) > 1) {
				ppr_t* txpwr_offsets;
				txpwr_offsets = wlc_phy_get_cached_ppr_ptr(cacheptr, kill_chan,
					TXPWR_CACHE_POWER_OFFSETS);
				wlc_bmac_clear_band_pwr_offset(txpwr_offsets, wlc->hw);
			}
#endif /* !WLC_LOW */
			wlc_phy_txpwr_cache_clear(wlc_cmi->pub->osh, cacheptr, kill_chan);
		}
		result = wlc_phy_txpwr_setup_entry(cacheptr, chanspec);
		ASSERT(result == BCME_OK);
	}

	last_chanspec = chanspec;

	if ((wlc_phy_get_cached_txchain_offsets(cacheptr, chanspec, 0) != WL_RATE_DISABLED) &&
		(wlc_phy_get_cached_pwr(cacheptr, chanspec, TXPWR_CACHE_POWER_OFFSETS) != NULL)) {
		wlc_channel_update_txchain_offsets(wlc_cmi, NULL);
		wlc_bmac_set_chanspec(wlc->hw, chanspec,
			(wlc_quiet_chanspec(wlc_cmi, chanspec) != 0), NULL);

		return;
	}
#endif /* WLTXPWR_CACHE */
	WL_CHANLOG(wlc, __FUNCTION__, TS_ENTER, 0);
	if ((txpwr = ppr_create(wlc_cmi->pub->osh, PPR_CHSPEC_BW(chanspec))) == NULL) {
		WL_CHANLOG(wlc, __FUNCTION__, TS_EXIT, 0);
		return;
	}
#ifdef SRHWVSDB
	/* If txpwr shms are saved, no need to go through power init fns again */
#ifdef WLMCHAN
	if ((MCHAN_ENAB(wlc->pub) && MCHAN_ACTIVE(wlc->pub)) ||
		wlc->srvsdb_info->iovar_force_vsdb) {
#else
	if (wlc->srvsdb_info->iovar_force_vsdb) {
#endif /* WLMCHAN */
		/* Check if power offsets are already saved */
		uint8 offset = (chanspec == wlc->srvsdb_info->vsdb_chans[0]) ? 0 : 1;

		if (wlc->srvsdb_info->vsdb_save_valid[offset]) {
			goto apply_chanspec;
		}

	}
#endif /* SRHWVSDB */

	wlc_channel_spurwar_locale(wlc_cmi, chanspec);

	wlc_channel_reg_limits(wlc_cmi, chanspec, txpwr);

	/* For APs, need to wait until reg limits are set before retrieving constraint. */
	local_constraint_qdbm = wlc_tpc_get_local_constraint_qdbm(wlc->tpc);

	if (!AP_ONLY(wlc->pub)) {
		ppr_apply_constraint_total_tx(txpwr, local_constraint_qdbm);
	}

	wlc_channel_update_txchain_offsets(wlc_cmi, txpwr);

	WL_CHANLOG(wlc, "After wlc_channel_update_txchain_offsets", 0, 0);

#ifdef SRHWVSDB
apply_chanspec:
#endif /* SRHWVSDB */

	wlc_bmac_set_chanspec(wlc->hw, chanspec, (wlc_quiet_chanspec(wlc_cmi, chanspec) != 0),
		txpwr);

	ppr_delete(wlc_cmi->pub->osh, txpwr);
	WL_CHANLOG(wlc, __FUNCTION__, TS_EXIT, 0);
}

int
wlc_channel_set_txpower_limit(wlc_cm_info_t *wlc_cmi, uint8 local_constraint_qdbm)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	ppr_t *txpwr;

	/* XXX REVISIT johnvb  Make sure wlc->chanspec updated first!  This is a hack.
	 * Since the purpose of wlc_channel is to encapsulate the operations related to
	 * country/regulatory including protecting/enforcing the restrictions in partial
	 * open source builds, using wlc's "upper" copy of chanspec breaks the design.
	 * While it would be possible to use the "lower" phy copy via wlc_phy_get_chanspec()
	 * this would cause an extra call/RPC in a BMAC dongle build.  The "correct"
	 * solution is making wlc_channel into a "object" with its own structure/state,
	 * attach, detach, etc.
	 */

	if (!wlc->clk)
		return BCME_NOCLK;
	if ((txpwr = ppr_create(wlc_cmi->pub->osh, PPR_CHSPEC_BW(wlc->chanspec))) == NULL) {
		return BCME_ERROR;
	}

	wlc_channel_reg_limits(wlc_cmi, wlc->chanspec, txpwr);
#if defined WLTXPWR_CACHE && defined(WLC_LOW) && defined(WL11N)
	wlc_phy_txpwr_cache_invalidate(wlc_phy_get_txpwr_cache(WLC_PI(wlc)));
#endif	/* WLTXPWR_CACHE */
	if (!AP_ONLY(wlc->pub)) {
		ppr_apply_constraint_total_tx(txpwr, local_constraint_qdbm);
	}

	wlc_channel_update_txchain_offsets(wlc_cmi, txpwr);

	wlc_phy_txpower_limit_set(WLC_PI(wlc), txpwr, wlc->chanspec);

	ppr_delete(wlc_cmi->pub->osh, txpwr);
	return 0;
}

clm_limits_type_t clm_chanspec_to_limits_type(chanspec_t chspec)
{
	clm_limits_type_t lt = CLM_LIMITS_TYPE_CHANNEL;

	if (CHSPEC_IS40(chspec)) {
		switch (CHSPEC_CTL_SB(chspec)) {
		case WL_CHANSPEC_CTL_SB_L:
			lt = CLM_LIMITS_TYPE_SUBCHAN_L;
			break;
		case WL_CHANSPEC_CTL_SB_U:
			lt = CLM_LIMITS_TYPE_SUBCHAN_U;
			break;
		default:
			break;
		}
	}
#ifdef WL11AC
	else if (CHSPEC_IS80(chspec) || CHSPEC_IS8080(chspec)) {
		switch (CHSPEC_CTL_SB(chspec)) {
		case WL_CHANSPEC_CTL_SB_LL:
			lt = CLM_LIMITS_TYPE_SUBCHAN_LL;
			break;
		case WL_CHANSPEC_CTL_SB_LU:
			lt = CLM_LIMITS_TYPE_SUBCHAN_LU;
			break;
		case WL_CHANSPEC_CTL_SB_UL:
			lt = CLM_LIMITS_TYPE_SUBCHAN_UL;
			break;
		case WL_CHANSPEC_CTL_SB_UU:
			lt = CLM_LIMITS_TYPE_SUBCHAN_UU;
			break;
		default:
			break;
		}
	} else if (CHSPEC_IS160(chspec)) {
		switch (CHSPEC_CTL_SB(chspec)) {
		case WL_CHANSPEC_CTL_SB_LLL:
			lt = CLM_LIMITS_TYPE_SUBCHAN_LLL;
			break;
		case WL_CHANSPEC_CTL_SB_LLU:
			lt = CLM_LIMITS_TYPE_SUBCHAN_LLU;
			break;
		case WL_CHANSPEC_CTL_SB_LUL:
			lt = CLM_LIMITS_TYPE_SUBCHAN_LUL;
			break;
		case WL_CHANSPEC_CTL_SB_LUU:
			lt = CLM_LIMITS_TYPE_SUBCHAN_LUU;
			break;
		case WL_CHANSPEC_CTL_SB_ULL:
			lt = CLM_LIMITS_TYPE_SUBCHAN_ULL;
			break;
		case WL_CHANSPEC_CTL_SB_ULU:
			lt = CLM_LIMITS_TYPE_SUBCHAN_ULU;
			break;
		case WL_CHANSPEC_CTL_SB_UUL:
			lt = CLM_LIMITS_TYPE_SUBCHAN_UUL;
			break;
		case WL_CHANSPEC_CTL_SB_UUU:
			lt = CLM_LIMITS_TYPE_SUBCHAN_UUU;
			break;
		default:
			break;
		}
	}
#endif /* WL11AC */
	return lt;
}

#ifdef WL11AC
/* Converts limits_type of control channel to the limits_type of
 * the larger-BW subchannel(s) enclosing it (e.g. 40in80, 40in160, 80in160)
 */
clm_limits_type_t clm_get_enclosing_subchan(clm_limits_type_t ctl_subchan, uint lvl)
{
	clm_limits_type_t lt = ctl_subchan;
	if (lvl == 1) {
		/* Get 40in80 given 20in80, 80in160 given 20in160, 40in8080 given 20in8080 */
		switch (ctl_subchan) {
			case CLM_LIMITS_TYPE_SUBCHAN_LL:
			case CLM_LIMITS_TYPE_SUBCHAN_LU:
			case CLM_LIMITS_TYPE_SUBCHAN_LLL:
			case CLM_LIMITS_TYPE_SUBCHAN_LLU:
			case CLM_LIMITS_TYPE_SUBCHAN_LUL:
			case CLM_LIMITS_TYPE_SUBCHAN_LUU:
				lt = CLM_LIMITS_TYPE_SUBCHAN_L;
				break;
			case CLM_LIMITS_TYPE_SUBCHAN_UL:
			case CLM_LIMITS_TYPE_SUBCHAN_UU:
			case CLM_LIMITS_TYPE_SUBCHAN_ULL:
			case CLM_LIMITS_TYPE_SUBCHAN_ULU:
			case CLM_LIMITS_TYPE_SUBCHAN_UUL:
			case CLM_LIMITS_TYPE_SUBCHAN_UUU:
				lt = CLM_LIMITS_TYPE_SUBCHAN_U;
				break;
			default:
				break;
		}
	} else if (lvl == 2) {
		/* Get 40in160 given 20in160 */
		switch (ctl_subchan) {
			case CLM_LIMITS_TYPE_SUBCHAN_LLL:
			case CLM_LIMITS_TYPE_SUBCHAN_LLU:
				lt = CLM_LIMITS_TYPE_SUBCHAN_LL;
				break;
			case CLM_LIMITS_TYPE_SUBCHAN_LUL:
			case CLM_LIMITS_TYPE_SUBCHAN_LUU:
				lt = CLM_LIMITS_TYPE_SUBCHAN_LU;
				break;
			case CLM_LIMITS_TYPE_SUBCHAN_ULL:
			case CLM_LIMITS_TYPE_SUBCHAN_ULU:
				lt = CLM_LIMITS_TYPE_SUBCHAN_UL;
				break;
			case CLM_LIMITS_TYPE_SUBCHAN_UUL:
			case CLM_LIMITS_TYPE_SUBCHAN_UUU:
				lt = CLM_LIMITS_TYPE_SUBCHAN_UU;
				break;
			default: break;
		}
	}
	return lt;
}
#endif /* WL11AC */

#ifdef WL_SARLIMIT
#define MAXNUM_SAR_ENTRY	(sizeof(sar_default)/sizeof(sar_default[0]))
const struct {
	uint16	boardtype;
	sar_limit_t sar;
} sar_default[] = {
	{BCM94331X29B, {{QDB(17)+2, QDB(16), QDB(17)+2, WLC_TXPWR_MAX}, /* 2g SAR limits */
	{{QDB(14)+2, QDB(14)+2, QDB(15)+2, WLC_TXPWR_MAX}, /* 5g subband 0 SAR limits */
	{QDB(15), QDB(14)+2, QDB(15), WLC_TXPWR_MAX}, /* 5g subband 1 SAR limits */
	{QDB(17), QDB(15), QDB(17), WLC_TXPWR_MAX}, /* 5g subband 2 SAR limits */
	{QDB(18), QDB(15), QDB(18), WLC_TXPWR_MAX}  /* 5g subband 3 SAR limits */
	}}},
	{BCM94331X29D, {{QDB(18), QDB(18), QDB(18), WLC_TXPWR_MAX}, /* 2g SAR limits */
	{{QDB(16)+2, QDB(16)+2, QDB(16)+2, WLC_TXPWR_MAX}, /* 5g subband 0 SAR limits */
	{QDB(16), QDB(17), QDB(17), WLC_TXPWR_MAX}, /* 5g subband 1 SAR limits */
	{QDB(16)+2, QDB(16)+2, QDB(16)+2, WLC_TXPWR_MAX}, /* 5g subband 2 SAR limits */
	{QDB(17)+2, QDB(16)+2, QDB(17)+2, WLC_TXPWR_MAX}  /* 5g subband 3 SAR limits */
	}}},
	{BCM94360X29C, {{QDB(17)+2, QDB(16)+1, QDB(16)+2, WLC_TXPWR_MAX}, /* 2g SAR limits */
	{{QDB(15)+2, QDB(15)+2, QDB(15)+2, WLC_TXPWR_MAX}, /* 5g subband 0 SAR limits */
	{QDB(15)+2, QDB(15)+2, QDB(15)+2, WLC_TXPWR_MAX}, /* 5g subband 1 SAR limits */
	{QDB(15)+2, QDB(15)+2, QDB(15)+2, WLC_TXPWR_MAX}, /* 5g subband 2 SAR limits */
	{QDB(16)+1, QDB(16)+1, QDB(16)+2, WLC_TXPWR_MAX}  /* 5g subband 3 SAR limits */
	}}},
	{BCM94360X29CP2, {{QDB(16), QDB(17)+1, QDB(17)+2, WLC_TXPWR_MAX}, /* 2g SAR limits */
	{{QDB(15)+2, QDB(16)+2, QDB(16)+2, WLC_TXPWR_MAX}, /* 5g subband 0 SAR limits */
	{QDB(15), QDB(16)+2, QDB(18), WLC_TXPWR_MAX}, /* 5g subband 1 SAR limits */
	{QDB(15)+2, QDB(16)+3, QDB(17)+2, WLC_TXPWR_MAX}, /* 5g subband 2 SAR limits */
	{QDB(16)+2, QDB(18)+2, QDB(18)+2, WLC_TXPWR_MAX}  /* 5g subband 3 SAR limits */
	}}},
	{BCM94360X29CP3,
	{{WLC_TXPWR_MAX, WLC_TXPWR_MAX, WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* disable */
	{{WLC_TXPWR_MAX, WLC_TXPWR_MAX, WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* disable */
	{WLC_TXPWR_MAX, WLC_TXPWR_MAX, WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* disable */
	{WLC_TXPWR_MAX, WLC_TXPWR_MAX, WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* disable */
	{WLC_TXPWR_MAX, WLC_TXPWR_MAX, WLC_TXPWR_MAX, WLC_TXPWR_MAX}  /* disable */
	}}},
	{BCM94360X52C, {{QDB(19), QDB(20), WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* 2g SAR limits */
	{{QDB(16), QDB(15)+2, WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* 5g subband 0 SAR limits */
	{QDB(17)+2, QDB(17)+2, WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* 5g subband 1 SAR limits */
	{QDB(17), QDB(18), WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* 5g subband 2 SAR limits */
	{QDB(17)+2, QDB(18), WLC_TXPWR_MAX, WLC_TXPWR_MAX}  /* 5g subband 3 SAR limits */
	}}},
	{BCM94360X52D, {{QDB(19), QDB(20), WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* 2g SAR limits */
	{{QDB(16), QDB(15)+2, WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* 5g subband 0 SAR limits */
	{QDB(17)+2, QDB(17)+2, WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* 5g subband 1 SAR limits */
	{QDB(17), QDB(18), WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* 5g subband 2 SAR limits */
	{QDB(17)+2, QDB(18), WLC_TXPWR_MAX, WLC_TXPWR_MAX}  /* 5g subband 3 SAR limits */
	}}},
	{BCM943602X87, {{QDB(17)+2, QDB(16)+1, QDB(16)+2, WLC_TXPWR_MAX}, /* 2g SAR limits */
	{{QDB(15)+2, QDB(15)+2, QDB(15)+2, WLC_TXPWR_MAX}, /* 5g subband 0 SAR limits */
	{QDB(15)+2, QDB(15)+2, QDB(15)+2, WLC_TXPWR_MAX}, /* 5g subband 1 SAR limits */
	{QDB(15)+2, QDB(15)+2, QDB(15)+2, WLC_TXPWR_MAX}, /* 5g subband 2 SAR limits */
	{QDB(16)+1, QDB(16)+1, QDB(16)+2, WLC_TXPWR_MAX}  /* 5g subband 3 SAR limits */
	}}},
	{BCM943602X87P2, {{QDB(17)+2, QDB(16)+1, QDB(16)+2, WLC_TXPWR_MAX}, /* 2g SAR limits */
	{{QDB(15)+2, QDB(15)+2, QDB(15)+2, WLC_TXPWR_MAX}, /* 5g subband 0 SAR limits */
	{QDB(15)+2, QDB(15)+2, QDB(15)+2, WLC_TXPWR_MAX}, /* 5g subband 1 SAR limits */
	{QDB(15)+2, QDB(15)+2, QDB(15)+2, WLC_TXPWR_MAX}, /* 5g subband 2 SAR limits */
	{QDB(16)+1, QDB(16)+1, QDB(16)+2, WLC_TXPWR_MAX}  /* 5g subband 3 SAR limits */
	}}},
	{BCM94350X14, {{QDB(19), QDB(20), WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* 2g SAR limits */
	{{QDB(16), QDB(15)+2, WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* 5g subband 0 SAR limits */
	{QDB(17)+2, QDB(17)+2, WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* 5g subband 1 SAR limits */
	{QDB(17), QDB(18), WLC_TXPWR_MAX, WLC_TXPWR_MAX}, /* 5g subband 2 SAR limits */
	{QDB(17)+2, QDB(18), WLC_TXPWR_MAX, WLC_TXPWR_MAX}  /* 5g subband 3 SAR limits */
	}}},
};

static void
wlc_channel_sarlimit_get_default(wlc_cm_info_t *wlc_cmi, sar_limit_t *sar)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	uint idx;

	for (idx = 0; idx < MAXNUM_SAR_ENTRY; idx++) {
		if (sar_default[idx].boardtype == wlc->pub->sih->boardtype) {
			memcpy((uint8 *)sar, (uint8 *)&(sar_default[idx].sar), sizeof(sar_limit_t));
			break;
		}
	}
}

void
wlc_channel_sar_init(wlc_cm_info_t *wlc_cmi)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;

	memset((uint8 *)wlc_cm->sarlimit.band2g,
	       wlc->bandstate[BAND_2G_INDEX]->sar,
	       WLC_TXCORE_MAX);
	memset((uint8 *)wlc_cm->sarlimit.band5g,
	       wlc->bandstate[BAND_5G_INDEX]->sar,
	       (WLC_TXCORE_MAX * WLC_SUBBAND_MAX));

	wlc_channel_sarlimit_get_default(wlc_cmi, &wlc_cm->sarlimit);
#ifdef BCMDBG
	wlc_channel_sarlimit_dump(wlc_cmi, &wlc_cm->sarlimit);
#endif /* BCMDBG */
}

#ifdef BCMDBG
void
wlc_channel_sarlimit_dump(wlc_cm_info_t *wlc_cmi, sar_limit_t *sar)
{
	int i;

	WL_ERROR(("\t2G:    %2d%s %2d%s %2d%s %2d%s\n",
	          QDB_FRAC(sar->band2g[0]), QDB_FRAC(sar->band2g[1]),
	          QDB_FRAC(sar->band2g[2]), QDB_FRAC(sar->band2g[3])));
	for (i = 0; i < WLC_SUBBAND_MAX; i++) {
		WL_ERROR(("\t5G[%1d]  %2d%s %2d%s %2d%s %2d%s\n", i,
		          QDB_FRAC(sar->band5g[i][0]), QDB_FRAC(sar->band5g[i][1]),
		          QDB_FRAC(sar->band5g[i][2]), QDB_FRAC(sar->band5g[i][3])));
	}
}
#endif /* BCMDBG */
int
wlc_channel_sarlimit_get(wlc_cm_info_t *wlc_cmi, sar_limit_t *sar)
{
	memcpy((uint8 *)sar, (uint8 *)&wlc_cmi->cm->sarlimit, sizeof(sar_limit_t));
	return 0;
}

int
wlc_channel_sarlimit_set(wlc_cm_info_t *wlc_cmi, sar_limit_t *sar)
{
	memcpy((uint8 *)&wlc_cmi->cm->sarlimit, (uint8 *)sar, sizeof(sar_limit_t));
	return 0;
}

/* given chanspec and return the subband index */
static uint
wlc_channel_sarlimit_subband_idx(wlc_cm_info_t *wlc_cmi, chanspec_t chanspec)
{
	uint8 chan = CHSPEC_CHANNEL(chanspec);
	if (chan < CHANNEL_5G_MID_START)
		return 0;
	else if (chan >= CHANNEL_5G_MID_START && chan < CHANNEL_5G_HIGH_START)
		return 1;
	else if (chan >= CHANNEL_5G_HIGH_START && chan < CHANNEL_5G_UPPER_START)
		return 2;
	else
		return 3;
}

/* Get the sar limit for the subband containing this channel */
void
wlc_channel_sarlimit_subband(wlc_cm_info_t *wlc_cmi, chanspec_t chanspec, uint8 *sar)
{
	int idx = 0;

	if (CHSPEC_IS5G(chanspec)) {
		idx = wlc_channel_sarlimit_subband_idx(wlc_cmi, chanspec);
		memcpy((uint8 *)sar, (uint8 *)wlc_cmi->cm->sarlimit.band5g[idx], WLC_TXCORE_MAX);
	} else {
		memcpy((uint8 *)sar, (uint8 *)wlc_cmi->cm->sarlimit.band2g, WLC_TXCORE_MAX);
	}
}
#endif /* WL_SARLIMIT */

bool
wlc_channel_sarenable_get(wlc_cm_info_t *wlc_cmi)
{
	return (wlc_cmi->cm->sar_enable);
}

void
wlc_channel_sarenable_set(wlc_cm_info_t *wlc_cmi, bool state)
{
	wlc_cmi->cm->sar_enable = state ? TRUE : FALSE;
}

void
wlc_channel_reg_limits(wlc_cm_info_t *wlc_cmi, chanspec_t chanspec, ppr_t *txpwr)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	unsigned int chan;
	clm_country_t country;
	clm_result_t result = CLM_RESULT_ERR;
	clm_country_locales_t locale;
	clm_power_limits_t limits;
	uint16 flags;
	clm_band_t bandtype;
	wlcband_t * band;
	bool filt_war = FALSE;
	int ant_gain;
	clm_limits_params_t lim_params;
#ifdef WL_SARLIMIT
	uint8 sarlimit[WLC_TXCORE_MAX];
#endif // endif
	uint lim_count = 0;
	clm_limits_type_t lim_types[5];
	wl_tx_bw_t lim_ppr_bw[5];
	uint i;

	ppr_clear(txpwr);

	if (clm_limits_params_init(&lim_params) != CLM_RESULT_OK)
		return;

	/* Lookup channel in autocountry_default if not in current country */
	if (!wlc_valid_chanspec_db(wlc_cmi, chanspec)) {
		if (WLC_AUTOCOUNTRY_ENAB(wlc)) {
			const char *def = wlc_11d_get_autocountry_default(wlc->m11d);
			result = wlc_country_lookup(wlc, def, &country);
		}
		if (result != CLM_RESULT_OK)
			return;
	} else
		country = wlc_cmi->cm->country;

	chan = CHSPEC_CHANNEL(chanspec);
	band = wlc->bandstate[CHSPEC_WLCBANDUNIT(chanspec)];
	bandtype = BAND_5G(band->bandtype) ? CLM_BAND_5G : CLM_BAND_2G;
	ant_gain = band->antgain;
	lim_params.sar = WLC_TXPWR_MAX;
	band->sar = band->sar_cached;
	if (wlc_cmi->cm->sar_enable) {
#ifdef WL_SARLIMIT
		/* when WL_SARLIMIT is enabled, update band->sar = MAX(sarlimit[i]) */
		wlc_channel_sarlimit_subband(wlc_cmi, chanspec, sarlimit);
		if ((CHIPID(wlc->pub->sih->chip) != BCM4360_CHIP_ID) &&
		    (CHIPID(wlc->pub->sih->chip) != BCM4350_CHIP_ID) &&
		    (CHIPID(wlc->pub->sih->chip) != BCM43602_CHIP_ID)) {
			uint i;
			band->sar = 0;
			for (i = 0; i < WLC_BITSCNT(wlc->stf->hw_txchain); i++)
				band->sar = MAX(band->sar, sarlimit[i]);

			WL_NONE(("%s: in %s Band, SAR %d apply\n", __FUNCTION__,
				(BAND_5G(wlc->band->bandtype) ? "5G" : "2G"), band->sar));
		}
		/* Don't write sarlimit to registers when called for reporting purposes */
		if (chanspec == wlc->chanspec) {
			uint32 sar_lims = (uint32)(sarlimit[0] | sarlimit[1] << 8 |
			                           sarlimit[2] << 16 | sarlimit[3] << 24);
#ifdef WLTXPWR_CACHE
			tx_pwr_cache_entry_t* cacheptr = wlc_phy_get_txpwr_cache(WLC_PI(wlc));

			wlc_phy_set_cached_sar_lims(cacheptr, chanspec, sar_lims);
#endif	/* WLTXPWR_CACHE */
			wlc_phy_sar_limit_set(WLC_PI_BANDUNIT(wlc, band->bandunit), sar_lims);
		}
#endif /* WL_SARLIMIT */
		lim_params.sar = band->sar;
	}

	if (strcmp(wlc_cmi->cm->country_abbrev, "#a") == 0) {
		band->sar = WLC_TXPWR_MAX;
		lim_params.sar = WLC_TXPWR_MAX;
#if defined(BCMDBG) || defined(WLTEST)
#ifdef WL_SARLIMIT
		wlc_phy_sar_limit_set(WLC_PI_BANDUNIT(wlc, band->bandunit),
			((WLC_TXPWR_MAX & 0xff) | (WLC_TXPWR_MAX & 0xff) << 8 |
			(WLC_TXPWR_MAX & 0xff) << 16 | (WLC_TXPWR_MAX & 0xff) << 24));
#endif /* WL_SARLIMIT */
#endif /* BCMDBG || WLTEST */
	}
	result = wlc_get_locale(country, &locale);
	if (result != CLM_RESULT_OK)
		return;

	result = wlc_get_flags(&locale, bandtype, &flags);
	if (result != CLM_RESULT_OK)
		return;

	if (wlc_cmi->cm->bandedge_filter_apply &&
	    (flags & WLC_FILT_WAR) &&
	    (chan == 1 || chan == 13))
		filt_war = TRUE;
	wlc_bmac_filter_war_upd(wlc->hw, filt_war);

	/* Need to set the txpwr_local_max to external reg max for
	 * this channel as per the locale selected for AP.
	 */
#ifdef AP
	if (AP_ONLY(wlc->pub)) {
		uint8 ch = wf_chspec_ctlchan(wlc->chanspec);
		uint8 pwr = wlc_get_reg_max_power_for_channel(wlc->cmi, ch, TRUE);
		wlc_tpc_set_local_max(wlc->tpc, pwr);
	}
#endif // endif

	lim_types[0] = CLM_LIMITS_TYPE_CHANNEL;
	switch (CHSPEC_BW(chanspec)) {
#ifdef WL11ULB
	case WL_CHANSPEC_BW_2P5:
		if (ULB_ENAB(wlc->pub)) {
			lim_params.bw = CLM_BW_2_5;
			lim_count = 1;
			lim_ppr_bw[0] = WL_TX_BW_2P5;
		} else {
			ASSERT(0);
		}
		break;
	case WL_CHANSPEC_BW_5:
		if (ULB_ENAB(wlc->pub)) {
			lim_params.bw = CLM_BW_5;
			lim_count = 1;
			lim_ppr_bw[0] = WL_TX_BW_5;
		} else {
			ASSERT(0);
		}
		break;
	case WL_CHANSPEC_BW_10:
		if (ULB_ENAB(wlc->pub)) {
			lim_params.bw = CLM_BW_10;
			lim_count = 1;
			lim_ppr_bw[0] = WL_TX_BW_10;
		} else {
			ASSERT(0);
		}
		break;
#endif /* WL11ULB */
	case WL_CHANSPEC_BW_20:
		lim_params.bw = CLM_BW_20;
		lim_count = 1;
		lim_ppr_bw[0] = WL_TX_BW_20;
		break;

	case WL_CHANSPEC_BW_40:
		lim_params.bw = CLM_BW_40;
		lim_count = 2;
		lim_types[1] = clm_chanspec_to_limits_type(chanspec);
		lim_ppr_bw[0] = WL_TX_BW_40;
		lim_ppr_bw[1] = WL_TX_BW_20IN40;
		break;

#ifdef WL11AC
	case WL_CHANSPEC_BW_80: {
		clm_limits_type_t ctl_limits_type =
			clm_chanspec_to_limits_type(chanspec);

		lim_params.bw = CLM_BW_80;
		lim_count = 3;
		lim_types[1] = clm_get_enclosing_subchan(ctl_limits_type, 1);
		lim_types[2] = ctl_limits_type;
		lim_ppr_bw[0] = WL_TX_BW_80;
		lim_ppr_bw[1] = WL_TX_BW_40IN80;
		lim_ppr_bw[2] = WL_TX_BW_20IN80;
		break;
	}

	case WL_CHANSPEC_BW_160: {
		clm_limits_type_t ctl_limits_type =
			clm_chanspec_to_limits_type(chanspec);

		lim_params.bw = CLM_BW_160;
		lim_count = 4;
		lim_types[1] = clm_get_enclosing_subchan(ctl_limits_type, 1);
		lim_types[2] = clm_get_enclosing_subchan(ctl_limits_type, 2);
		lim_types[3] = ctl_limits_type;
		lim_ppr_bw[0] = WL_TX_BW_160;
		lim_ppr_bw[1] = WL_TX_BW_80IN160;
		lim_ppr_bw[2] = WL_TX_BW_40IN160;
		lim_ppr_bw[3] = WL_TX_BW_20IN160;
		break;
	}

	case WL_CHANSPEC_BW_8080: {
		clm_limits_type_t ctl_limits_type =
			clm_chanspec_to_limits_type(chanspec);

		chan = wf_chspec_primary80_channel(chanspec);
		lim_params.other_80_80_chan = wf_chspec_secondary80_channel(chanspec);

		lim_params.bw = CLM_BW_80_80;
		lim_count = 5;
		lim_types[1] = clm_get_enclosing_subchan(ctl_limits_type, 1);
		lim_types[2] = clm_get_enclosing_subchan(ctl_limits_type, 2);
		lim_types[3] = ctl_limits_type;
		lim_types[4] = CLM_LIMITS_TYPE_CHANNEL;  /* special case: 8080chan2 */
		lim_ppr_bw[0] = WL_TX_BW_8080;
		lim_ppr_bw[1] = WL_TX_BW_80IN8080;
		lim_ppr_bw[2] = WL_TX_BW_40IN8080;
		lim_ppr_bw[3] = WL_TX_BW_20IN8080;
		lim_ppr_bw[4] = WL_TX_BW_8080CHAN2;
		break;
	}
#endif /* WL11AC */

	default:
		ASSERT(0);
		break;
	}

	/* Calculate limits for each (sub)channel */
	for (i = 0; i < lim_count; i++) {
#ifdef WL11AC
		/* For 8080CHAN2, just swap primary and secondary channel
		 * and calculate 80MHz limit
		 */
		if (lim_ppr_bw[i] == WL_TX_BW_8080CHAN2) {
			lim_params.other_80_80_chan = chan;
			chan = wf_chspec_secondary80_channel(chanspec);
		}
#endif /* WL11AC */

		if (clm_power_limits(&locale, bandtype, chan, ant_gain, lim_types[i],
			&lim_params, &limits) == CLM_RESULT_OK) {

			/* Port the values for this bandwidth */
			ppr_set_dsss(txpwr, lim_ppr_bw[i], WL_TX_CHAINS_1,
				CLM_DSSS_RATESET(limits));

			ppr_set_ofdm(txpwr, lim_ppr_bw[i], WL_TX_MODE_NONE, WL_TX_CHAINS_1,
				CLM_OFDM_1X1_RATESET(limits));

#ifdef WL11N
			ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_1, WL_TX_MODE_NONE,
				WL_TX_CHAINS_1, CLM_MCS_1X1_RATESET(limits));

			if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
				ppr_set_dsss(txpwr, lim_ppr_bw[i], WL_TX_CHAINS_2,
					CLM_DSSS_1X2_MULTI_RATESET(limits));

				ppr_set_ofdm(txpwr, lim_ppr_bw[i], WL_TX_MODE_CDD, WL_TX_CHAINS_2,
					CLM_OFDM_1X2_CDD_RATESET(limits));

				ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_1, WL_TX_MODE_CDD,
					WL_TX_CHAINS_2, CLM_MCS_1X2_CDD_RATESET(limits));

				ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_2, WL_TX_MODE_STBC,
					WL_TX_CHAINS_2, CLM_MCS_2X2_STBC_RATESET(limits));

				ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_2, WL_TX_MODE_NONE,
					WL_TX_CHAINS_2, CLM_MCS_2X2_SDM_RATESET(limits));

				if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
					ppr_set_dsss(txpwr, lim_ppr_bw[i], WL_TX_CHAINS_3,
						CLM_DSSS_1X3_MULTI_RATESET(limits));

					ppr_set_ofdm(txpwr, lim_ppr_bw[i], WL_TX_MODE_CDD,
						WL_TX_CHAINS_3,	CLM_OFDM_1X3_CDD_RATESET(limits));

					ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_3,
						CLM_MCS_1X3_CDD_RATESET(limits));

					ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_3,
						CLM_MCS_2X3_STBC_RATESET(limits));

					ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_3,
						CLM_MCS_2X3_SDM_RATESET(limits));

					ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_3,
						CLM_MCS_3X3_SDM_RATESET(limits));

					if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
						ppr_set_dsss(txpwr, lim_ppr_bw[i], WL_TX_CHAINS_4,
							CLM_DSSS_1X4_MULTI_RATESET(limits));

						ppr_set_ofdm(txpwr, lim_ppr_bw[i], WL_TX_MODE_CDD,
							WL_TX_CHAINS_4,
							CLM_OFDM_1X4_CDD_RATESET(limits));

						ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_1,
							WL_TX_MODE_CDD, WL_TX_CHAINS_4,
							CLM_MCS_1X4_CDD_RATESET(limits));

						ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_2,
							WL_TX_MODE_STBC, WL_TX_CHAINS_4,
							CLM_MCS_2X4_STBC_RATESET(limits));

						ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_2,
							WL_TX_MODE_NONE, WL_TX_CHAINS_4,
							CLM_MCS_2X4_SDM_RATESET(limits));

						ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_3,
							WL_TX_MODE_NONE, WL_TX_CHAINS_4,
							CLM_MCS_3X4_SDM_RATESET(limits));

						ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_4,
							WL_TX_MODE_NONE, WL_TX_CHAINS_4,
							CLM_MCS_4X4_SDM_RATESET(limits));
					}
				}
			}
#if defined(WL_BEAMFORMING)
			if (TXBF_ENAB(wlc->pub) && (PHYCORENUM(wlc->stf->op_txstreams) > 1)) {
				ppr_set_ofdm(txpwr, lim_ppr_bw[i], WL_TX_MODE_TXBF, WL_TX_CHAINS_2,
					CLM_OFDM_1X2_TXBF_RATESET(limits));

				ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_1, WL_TX_MODE_TXBF,
					WL_TX_CHAINS_2, CLM_MCS_1X2_TXBF_RATESET(limits));

				ppr_set_ht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_2, WL_TX_MODE_TXBF,
					WL_TX_CHAINS_2, CLM_MCS_2X2_TXBF_RATESET(limits));

				if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
					ppr_set_ofdm(txpwr, lim_ppr_bw[i], WL_TX_MODE_TXBF,
						WL_TX_CHAINS_3,	CLM_OFDM_1X3_TXBF_RATESET(limits));

					ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_1,
						WL_TX_MODE_TXBF, WL_TX_CHAINS_3,
						CLM_MCS_1X3_TXBF_RATESET(limits));

					ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_2,
						WL_TX_MODE_TXBF, WL_TX_CHAINS_3,
						CLM_MCS_2X3_TXBF_RATESET(limits));

					ppr_set_ht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_3,
						WL_TX_MODE_TXBF, WL_TX_CHAINS_3,
						CLM_MCS_3X3_TXBF_RATESET(limits));

					if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
						ppr_set_ofdm(txpwr, lim_ppr_bw[i], WL_TX_MODE_TXBF,
							WL_TX_CHAINS_4,
							CLM_OFDM_1X4_TXBF_RATESET(limits));

						ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_1,
							WL_TX_MODE_TXBF, WL_TX_CHAINS_4,
							CLM_MCS_1X4_TXBF_RATESET(limits));

						ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_2,
							WL_TX_MODE_TXBF, WL_TX_CHAINS_4,
							CLM_MCS_2X4_TXBF_RATESET(limits));

						ppr_set_vht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_3,
							WL_TX_MODE_TXBF, WL_TX_CHAINS_4,
							CLM_MCS_3X4_TXBF_RATESET(limits));

						ppr_set_ht_mcs(txpwr, lim_ppr_bw[i], WL_TX_NSS_4,
							WL_TX_MODE_TXBF, WL_TX_CHAINS_4,
							CLM_MCS_4X4_TXBF_RATESET(limits));
					}
				}
			}
#endif /* defined(WL_BEAMFORMING) */
#endif /* WL11N */
		}
	}
	WL_NONE(("Channel(chanspec) %d (0x%4.4x)\n", chan, chanspec));
#ifdef WLC_LOW
	/* Convoluted WL debug conditional execution of function to avoid warnings. */
	WL_NONE(("%s", (wlc_phy_txpower_limits_dump(txpwr, WLCISHTPHY(wlc->band)), "")));
#endif /* WLC_LOW */

}
static clm_result_t
clm_power_limits(
	const clm_country_locales_t *locales, clm_band_t band,
	unsigned int chan, int ant_gain, clm_limits_type_t limits_type,
	const clm_limits_params_t *params, clm_power_limits_t *limits)
{
	return clm_limits(locales, band, chan, ant_gain, limits_type, params, limits);
}

/* Returns TRUE if currently set country is Japan or variant */
bool
wlc_japan(struct wlc_info *wlc)
{
	return wlc_japan_ccode(wlc->cmi->cm->country_abbrev);
}

/* JP, J1 - J10 are Japan ccodes */
static bool
wlc_japan_ccode(const char *ccode)
{
	return (ccode[0] == 'J' &&
		(ccode[1] == 'P' || (ccode[1] >= '1' && ccode[1] <= '9')));
}

/* Q2 and Q1 are alternate USA ccode */
static bool
wlc_us_ccode(const char *ccode)
{
	return (!strncmp("US", ccode, 3) ||
		!strncmp("Q1", ccode, 3) ||
		!strncmp("Q2", ccode, 3) ||
		!strncmp("ALL", ccode, 3));
}

static void
wlc_regclass_vec_init(wlc_cm_info_t *wlc_cmi)
{
	uint8 i, idx;
	chanspec_t chanspec;
#ifdef WL11N
	wlc_info_t *wlc = wlc_cmi->wlc;
	bool saved_cap_40, saved_db_cap_40 = TRUE;
#endif // endif
#ifdef WL11AC
	bool saved_cap_80;
#endif // endif
#ifdef WL11AC_160
	bool saved_cap_160;
#endif // endif
	uint8 *rcvec = wlc_cmi->cm->valid_rcvec.vec;

#ifdef WL11N
	/* save 40 MHz cap */
	saved_cap_40 = WL_BW_CAP_40MHZ(wlc->band->bw_cap);
	wlc->band->bw_cap |= WLC_BW_40MHZ_BIT;
	if (NBANDS(wlc) > 1) {
		saved_db_cap_40 = WL_BW_CAP_40MHZ(wlc->bandstate[OTHERBANDUNIT(wlc)]->bw_cap);
		wlc->bandstate[OTHERBANDUNIT(wlc)]->bw_cap |= WLC_BW_40MHZ_BIT;
	}
#endif // endif
#ifdef WL11AC
	/* save 80 MHZ cap */
	saved_cap_80 = WL_BW_CAP_80MHZ(wlc->bandstate[BAND_5G_INDEX]->bw_cap);
	wlc->bandstate[BAND_5G_INDEX]->bw_cap |= WLC_BW_80MHZ_BIT;
#endif /* WL11AC */
#ifdef WL11AC_160
	/* save 160 MHZ cap */
	saved_cap_160 = WL_BW_CAP_160MHZ(wlc->bandstate[BAND_5G_INDEX]->bw_cap);
	wlc->bandstate[BAND_5G_INDEX]->bw_cap |= WLC_BW_160MHZ_BIT;
#endif /* WL11AC_160 */

	bzero(rcvec, MAXRCVEC);
	for (i = 0; i < MAXCHANNEL; i++) {
		chanspec = CH20MHZ_CHSPEC(i);
		if (wlc_valid_chanspec_db(wlc_cmi, chanspec)) {
			if ((idx = wlc_get_regclass(wlc_cmi, chanspec)))
				setbit((uint8 *)rcvec, idx);
		}
#if defined(WL11N) && !defined(WL11N_20MHZONLY)
		if (N_ENAB(wlc->pub)) {
			chanspec = CH40MHZ_CHSPEC(i, WL_CHANSPEC_CTL_SB_LOWER);
			if (wlc_valid_chanspec_db(wlc_cmi, chanspec)) {
				if ((idx = wlc_get_regclass(wlc_cmi, chanspec)))
					setbit((uint8 *)rcvec, idx);
			}
			chanspec = CH40MHZ_CHSPEC(i, WL_CHANSPEC_CTL_SB_UPPER);
			if (wlc_valid_chanspec_db(wlc_cmi, chanspec)) {
				if ((idx = wlc_get_regclass(wlc_cmi, chanspec)))
					setbit((uint8 *)rcvec, idx);
			}
		}
#endif /* defined(WL11N) && !defined(WL11N_20MHZONLY) */
#ifdef WL11AC
		if (VHT_ENAB(wlc->pub)) {
			chanspec = CH80MHZ_CHSPEC(i, WL_CHANSPEC_CTL_SB_LL);
			if (wlc_valid_chanspec(wlc_cmi, chanspec)) {
				if ((idx = wlc_get_regclass(wlc_cmi, chanspec)))
					setbit((uint8 *)rcvec, idx);
			}
		}
#endif // endif
	}
/* restore 160 MHZ cap */
#ifdef WL11AC_160
	if (saved_cap_160) {
		wlc->bandstate[BAND_5G_INDEX]->bw_cap |= WLC_BW_160MHZ_BIT;
	} else {
		wlc->bandstate[BAND_5G_INDEX]->bw_cap &= ~WLC_BW_160MHZ_BIT;
	}
#endif /* WL11AC_160 */
/* restore 80 MHZ cap */
#ifdef WL11AC
	if (saved_cap_80) {
		wlc->bandstate[BAND_5G_INDEX]->bw_cap |= WLC_BW_80MHZ_BIT;
	} else {
		wlc->bandstate[BAND_5G_INDEX]->bw_cap &= ~WLC_BW_80MHZ_BIT;
	}
#endif /* WL11AC */
#ifdef WL11N
	/* restore 40 MHz cap */
	if (saved_cap_40) {
		wlc->band->bw_cap |= WLC_BW_40MHZ_BIT;
	} else {
		wlc->band->bw_cap &= ~WLC_BW_40MHZ_BIT;
	}

	if (NBANDS(wlc) > 1) {
		if (saved_db_cap_40) {
			wlc->bandstate[OTHERBANDUNIT(wlc)]->bw_cap |= WLC_BW_40MHZ_BIT;
		} else {
			wlc->bandstate[OTHERBANDUNIT(wlc)]->bw_cap &= ~WLC_BW_40MHZ_BIT;
		}
	}
#endif // endif
}

#ifdef WL11N
uint8
wlc_rclass_extch_get(wlc_cm_info_t *wlc_cmi, uint8 rclass)
{
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	const rcinfo_t *rcinfo;
	uint8 i, extch = DOT11_EXT_CH_NONE;

	if (!isset(wlc_cm->valid_rcvec.vec, rclass)) {
		WL_ERROR(("wl%d: %s %d regulatory class not supported\n",
			wlc_cmi->wlc->pub->unit, wlc_cm->country_abbrev, rclass));
		return extch;
	}

	/* rcinfo consist of control channel at lower sb */
	rcinfo = wlc_cm->rcinfo_list[WLC_RCLIST_40L];
	for (i = 0; rcinfo && i < rcinfo->len; i++) {
		if (rclass == rcinfo->rctbl[i].rclass) {
			/* ext channel is opposite of control channel */
			extch = DOT11_EXT_CH_UPPER;
			goto exit;
		}
	}

	/* rcinfo consist of control channel at upper sb */
	rcinfo = wlc_cm->rcinfo_list[WLC_RCLIST_40U];
	for (i = 0; rcinfo && i < rcinfo->len; i++) {
		if (rclass == rcinfo->rctbl[i].rclass) {
			/* ext channel is opposite of control channel */
			extch = DOT11_EXT_CH_LOWER;
			break;
		}
	}
exit:
	WL_INFORM(("wl%d: %s regulatory class %d has ctl chan %s\n",
		wlc_cmi->wlc->pub->unit, wlc_cm->country_abbrev, rclass,
		((!extch) ? "NONE" : (((extch == DOT11_EXT_CH_LOWER) ? "LOWER" : "UPPER")))));

	return extch;
}
#endif /* WL11N */

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTDLS)
/* get the ordered list of supported reg class, with current reg class
 * as first element
 */
uint8
wlc_get_regclass_list(wlc_cm_info_t *wlc_cmi, uint8 *rclist, uint lsize,
	chanspec_t chspec, bool ie_order)
{
	uint8 i, cur_rc = 0, idx = 0;

	ASSERT(rclist != NULL);
	ASSERT(lsize > 1);

	if (ie_order) {
		cur_rc = wlc_get_regclass(wlc_cmi, chspec);
		if (!cur_rc) {
			WL_ERROR(("wl%d: current regulatory class is not found\n",
				wlc_cmi->wlc->pub->unit));
			return 0;
		}
		rclist[idx++] = cur_rc;	/* first element is current reg class */
	}

	for (i = 0; i < MAXREGCLASS && idx < lsize; i++) {
		if (isset(wlc_cmi->cm->valid_rcvec.vec, i))
			rclist[idx++] = i;
	}

	if (i < MAXREGCLASS && idx == lsize) {
		WL_ERROR(("wl%d: regulatory class list full %d\n", wlc_cmi->wlc->pub->unit, idx));
		ASSERT(0);
	}

	return idx;
}
#endif /* BCMDBG || BCMDBG_DUMP || WLTDLS */

static uint8
wlc_get_2g_regclass(wlc_cm_info_t *wlc_cmi, uint8 chan)
{
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
#ifdef WL_GLOBAL_RCLASS
	if (wlc_cm->cur_rclass == BCMWIFI_RCLASS_TYPE_GBL) {
		if (chan < 14) {
			return WLC_REGCLASS_GLOBAL_2G_20MHZ;
		} else {
			return WLC_REGCLASS_GLOBAL_2G_20MHZ_CH14;
		}
	}
#endif /* WL_GLOBAL_RCLASS */
	if (wlc_us_ccode(wlc_cm->country_abbrev))
		return WLC_REGCLASS_USA_2G_20MHZ;
	else if (wlc_japan_ccode(wlc_cm->country_abbrev)) {
		if (chan < 14)
			return WLC_REGCLASS_JPN_2G_20MHZ;
		else
			return WLC_REGCLASS_JPN_2G_20MHZ_CH14;
	} else
		return WLC_REGCLASS_EUR_2G_20MHZ;
}

uint8
wlc_get_regclass(wlc_cm_info_t *wlc_cmi, chanspec_t chanspec)
{
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	const rcinfo_t *rcinfo = NULL;
	uint8 i;
	uint8 chan;

#ifdef WL11AC
	if (CHSPEC_IS80(chanspec)) {
		chan = wf_chspec_primary80_channel(chanspec);
		rcinfo = wlc_cm->rcinfo_list_11ac[WLC_RCLIST_80];
#ifdef WL11AC_160
	} else if (CHSPEC_IS160(chanspec)) {
		chan = wf_chspec_primary80_channel(chanspec);
		rcinfo = wlc_cm->rcinfo_list_11ac[WLC_RCLIST_160];
#endif /* WL11AC_160 */
	} else
#endif /* WL11AC */

#ifdef WL11N
	if (CHSPEC_IS40(chanspec)) {
		chan = wf_chspec_ctlchan(chanspec);
		if (CHSPEC_SB_UPPER(chanspec))
			rcinfo = wlc_cm->rcinfo_list[WLC_RCLIST_40U];
		else
			rcinfo = wlc_cm->rcinfo_list[WLC_RCLIST_40L];
	} else
#endif /* WL11N */
	/* XXX ULB_TBD: Will need changes if we are defining new regulatory classes for ULB BWs
	 * Will need extension in definition of WLC_RCLIST_40U etc
	 */
	{
		chan = CHSPEC_CHANNEL(chanspec);
		if (CHSPEC_IS2G(chanspec))
			return (wlc_get_2g_regclass(wlc_cmi, chan));
		rcinfo = wlc_cm->rcinfo_list[WLC_RCLIST_20];
	}

	for (i = 0; rcinfo != NULL && i < rcinfo->len; i++) {
		if (chan == rcinfo->rctbl[i].chan)
			return (rcinfo->rctbl[i].rclass);
	}

	WL_INFORM(("wl%d: No regulatory class assigned for %s channel %d\n",
		wlc_cmi->wlc->pub->unit, wlc_cm->country_abbrev, chan));

	return 0;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
int
wlc_dump_rclist(const char *name, uint8 *rclist, uint8 rclen, struct bcmstrbuf *b)
{
	uint i;

	if (!rclen)
		return 0;

	bcm_bprintf(b, "%s [ ", name ? name : "");
	for (i = 0; i < rclen; i++) {
		bcm_bprintf(b, "%d ", rclist[i]);
	}
	bcm_bprintf(b, "]");
	bcm_bprintf(b, "\n");

	return 0;
}

/* format a qdB value as integer and decimal fraction in a bcmstrbuf */
static void
wlc_channel_dump_qdb(struct bcmstrbuf *b, int qdb)
{
	if ((qdb >= 0) || (qdb % WLC_TXPWR_DB_FACTOR == 0))
		bcm_bprintf(b, "%2d%s", QDB_FRAC(qdb));
	else
		bcm_bprintf(b, "%2d%s",
			qdb / WLC_TXPWR_DB_FACTOR + 1,
			fraction[WLC_TXPWR_DB_FACTOR - (qdb % WLC_TXPWR_DB_FACTOR)]);
}

/* helper function for wlc_channel_dump_txppr() to print one set of power targets with label */
static void
wlc_channel_dump_pwr_range(struct bcmstrbuf *b, const char *label, int8 *ptr, uint count)
{
	uint i;

	bcm_bprintf(b, "%s ", label);
	for (i = 0; i < count; i++) {
		if (ptr[i] != WL_RATE_DISABLED) {
			wlc_channel_dump_qdb(b, ptr[i]);
			bcm_bprintf(b, " ");
		} else
			bcm_bprintf(b, "-     ");
	}
	bcm_bprintf(b, "\n");
}

/* helper function to print a target range line with the typical 8 targets */
static void
wlc_channel_dump_pwr_range8(struct bcmstrbuf *b, const char *label, int8* ptr)
{
	wlc_channel_dump_pwr_range(b, label, (int8*)ptr, 8);
}

#ifdef WL11AC

#define NUM_MCS_RATES WL_NUM_RATES_VHT
#define CHSPEC_TO_TX_BW(c)	(\
	CHSPEC_IS8080(c) ? WL_TX_BW_8080 : \
	(CHSPEC_IS160(c) ? WL_TX_BW_160 : \
	(CHSPEC_IS80(c) ? WL_TX_BW_80 : \
	(CHSPEC_IS40(c) ? WL_TX_BW_40 : WL_TX_BW_20))))

#else

#define NUM_MCS_RATES WL_NUM_RATES_MCS_1STREAM
#define CHSPEC_TO_TX_BW(c)	(CHSPEC_IS40(c) ? WL_TX_BW_40 : WL_TX_BW_20)

#endif // endif

/* helper function to print a target range line with the typical 8 targets */
static void
wlc_channel_dump_pwr_range_mcs(struct bcmstrbuf *b, const char *label, int8 *ptr)
{
	wlc_channel_dump_pwr_range(b, label, (int8*)ptr, NUM_MCS_RATES);
}

/* format the contents of a ppr_t structure for a bcmstrbuf */
static void
wlc_channel_dump_txppr(struct bcmstrbuf *b, ppr_t *txpwr, wl_tx_bw_t bw, wlc_info_t *wlc)
{
	ppr_dsss_rateset_t dsss_limits;
	ppr_ofdm_rateset_t ofdm_limits;
	ppr_vht_mcs_rateset_t mcs_limits;

	if (bw == WL_TX_BW_20) {
		bcm_bprintf(b, "\n20MHz:\n");
		ppr_get_dsss(txpwr, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss_limits);
		wlc_channel_dump_pwr_range(b,  "DSSS              ", dsss_limits.pwr,
			WL_RATESET_SZ_DSSS);
		ppr_get_ofdm(txpwr, WL_TX_BW_20, WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm_limits);
		wlc_channel_dump_pwr_range8(b, "OFDM              ", ofdm_limits.pwr);
		ppr_get_vht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
		wlc_channel_dump_pwr_range_mcs(b, "MCS0_7            ", mcs_limits.pwr);

		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_get_dsss(txpwr, WL_TX_BW_20, WL_TX_CHAINS_2, &dsss_limits);
			wlc_channel_dump_pwr_range(b, "DSSS_MULTI1       ", dsss_limits.pwr,
				WL_RATESET_SZ_DSSS);
			ppr_get_ofdm(txpwr, WL_TX_BW_20, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
				&ofdm_limits);
			wlc_channel_dump_pwr_range8(b, "OFDM_CDD1         ", ofdm_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD1       ", mcs_limits.pwr);

			ppr_get_vht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC       ", mcs_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS8_15           ", mcs_limits.pwr);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_get_dsss(txpwr, WL_TX_BW_20, WL_TX_CHAINS_3, &dsss_limits);
				wlc_channel_dump_pwr_range(b,  "DSSS_MULTI2       ",
					dsss_limits.pwr, WL_RATESET_SZ_DSSS);
				ppr_get_ofdm(txpwr, WL_TX_BW_20, WL_TX_MODE_CDD, WL_TX_CHAINS_3,
					&ofdm_limits);
				wlc_channel_dump_pwr_range8(b, "OFDM_CDD2         ",
					ofdm_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD2       ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_2, WL_TX_MODE_STBC,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP1",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_2, WL_TX_MODE_NONE,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP1    ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_3, WL_TX_MODE_NONE,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
					mcs_limits.pwr);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_get_dsss(txpwr, WL_TX_BW_20, WL_TX_CHAINS_4,
						&dsss_limits);
					wlc_channel_dump_pwr_range(b,  "DSSS_MULTI3       ",
						dsss_limits.pwr, WL_RATESET_SZ_DSSS);
					ppr_get_ofdm(txpwr, WL_TX_BW_20, WL_TX_MODE_CDD,
						WL_TX_CHAINS_4, &ofdm_limits);
					wlc_channel_dump_pwr_range8(b, "OFDM_CDD3         ",
						ofdm_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD3       ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP2",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP2    ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_4,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS24_31          ",
						mcs_limits.pwr);
				}
			}
		}
	} else if (bw == WL_TX_BW_40) {

		bcm_bprintf(b, "\n40MHz:\n");
		ppr_get_ofdm(txpwr, WL_TX_BW_40, WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm_limits);
		wlc_channel_dump_pwr_range8(b, "OFDM              ", ofdm_limits.pwr);
		ppr_get_vht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
		wlc_channel_dump_pwr_range_mcs(b, "MCS0_7            ", mcs_limits.pwr);

		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_get_ofdm(txpwr, WL_TX_BW_40, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
				&ofdm_limits);
			wlc_channel_dump_pwr_range8(b, "OFDM_CDD1         ", ofdm_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD1       ", mcs_limits.pwr);

			ppr_get_vht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC       ", mcs_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS8_15           ", mcs_limits.pwr);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_get_ofdm(txpwr, WL_TX_BW_40, WL_TX_MODE_CDD, WL_TX_CHAINS_3,
					&ofdm_limits);
				wlc_channel_dump_pwr_range8(b, "OFDM_CDD2         ",
					ofdm_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_1, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD2       ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_2, WL_TX_MODE_STBC,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP1",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_2, WL_TX_MODE_NONE,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP1    ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_3, WL_TX_MODE_NONE,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
					mcs_limits.pwr);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_get_ofdm(txpwr, WL_TX_BW_40, WL_TX_MODE_CDD,
						WL_TX_CHAINS_4, &ofdm_limits);
					wlc_channel_dump_pwr_range8(b, "OFDM_CDD3         ",
						ofdm_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD3       ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP2",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP2    ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_4,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS24_31          ",
						mcs_limits.pwr);
				}
			}
		}

		bcm_bprintf(b, "\n20in40MHz:\n");
		ppr_get_dsss(txpwr, WL_TX_BW_20IN40, WL_TX_CHAINS_1, &dsss_limits);
		wlc_channel_dump_pwr_range(b,  "DSSS              ", dsss_limits.pwr,
			WL_RATESET_SZ_DSSS);
		ppr_get_ofdm(txpwr, WL_TX_BW_20IN40, WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm_limits);
		wlc_channel_dump_pwr_range8(b, "OFDM              ", ofdm_limits.pwr);
		ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
		wlc_channel_dump_pwr_range_mcs(b, "MCS0_7            ", mcs_limits.pwr);

		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_get_dsss(txpwr, WL_TX_BW_20IN40, WL_TX_CHAINS_2, &dsss_limits);
			wlc_channel_dump_pwr_range(b, "DSSS_MULTI1       ", dsss_limits.pwr,
				WL_RATESET_SZ_DSSS);
			ppr_get_ofdm(txpwr, WL_TX_BW_20IN40, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
				&ofdm_limits);
			wlc_channel_dump_pwr_range8(b, "OFDM_CDD1         ", ofdm_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD1       ", mcs_limits.pwr);

			ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC       ", mcs_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS8_15           ", mcs_limits.pwr);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_get_dsss(txpwr, WL_TX_BW_20IN40, WL_TX_CHAINS_3, &dsss_limits);
				wlc_channel_dump_pwr_range(b,  "DSSS_MULTI2       ",
					dsss_limits.pwr, WL_RATESET_SZ_DSSS);
				ppr_get_ofdm(txpwr, WL_TX_BW_20IN40, WL_TX_MODE_CDD, WL_TX_CHAINS_3,
					&ofdm_limits);
				wlc_channel_dump_pwr_range8(b, "OFDM_CDD2         ",
					ofdm_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_1, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD2       ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP1",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP1    ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_3,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
					mcs_limits.pwr);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_get_dsss(txpwr, WL_TX_BW_20IN40, WL_TX_CHAINS_4,
						&dsss_limits);
					wlc_channel_dump_pwr_range(b,  "DSSS_MULTI3       ",
						dsss_limits.pwr, WL_RATESET_SZ_DSSS);
					ppr_get_ofdm(txpwr, WL_TX_BW_20IN40, WL_TX_MODE_CDD,
						WL_TX_CHAINS_4, &ofdm_limits);
					wlc_channel_dump_pwr_range8(b, "OFDM_CDD3         ",
						ofdm_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD3       ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP2",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP2    ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_4,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS24_31          ",
						mcs_limits.pwr);
				}
			}
		}

#ifdef WL11AC
	} else if (bw == WL_TX_BW_80) {
		bcm_bprintf(b, "\n80MHz:\n");

		ppr_get_ofdm(txpwr, WL_TX_BW_80, WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm_limits);
		wlc_channel_dump_pwr_range8(b, "OFDM              ", ofdm_limits.pwr);
		ppr_get_vht_mcs(txpwr, WL_TX_BW_80, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
		wlc_channel_dump_pwr_range_mcs(b, "MCS0_7            ", mcs_limits.pwr);

		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_get_ofdm(txpwr, WL_TX_BW_80, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
				&ofdm_limits);
			wlc_channel_dump_pwr_range8(b, "OFDM_CDD1         ", ofdm_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_80, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD1       ", mcs_limits.pwr);

			ppr_get_vht_mcs(txpwr, WL_TX_BW_80, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC       ", mcs_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_80, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS8_15           ", mcs_limits.pwr);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_get_ofdm(txpwr, WL_TX_BW_80, WL_TX_MODE_CDD, WL_TX_CHAINS_3,
					&ofdm_limits);
				wlc_channel_dump_pwr_range8(b, "OFDM_CDD2         ",
					ofdm_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_80, WL_TX_NSS_1, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD2       ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_80, WL_TX_NSS_2, WL_TX_MODE_STBC,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP1",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_80, WL_TX_NSS_2, WL_TX_MODE_NONE,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP1    ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_80, WL_TX_NSS_3, WL_TX_MODE_NONE,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
					mcs_limits.pwr);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_get_ofdm(txpwr, WL_TX_BW_80, WL_TX_MODE_CDD,
						WL_TX_CHAINS_4, &ofdm_limits);
					wlc_channel_dump_pwr_range8(b, "OFDM_CDD3         ",
						ofdm_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_80, WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD3       ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_80, WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP2",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_80, WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP2    ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_80, WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_80, WL_TX_NSS_4,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS24_31          ",
						mcs_limits.pwr);
				}
			}
		}
		bcm_bprintf(b, "\n20in80MHz:\n");
		ppr_get_dsss(txpwr, WL_TX_BW_20IN80, WL_TX_CHAINS_1, &dsss_limits);
		wlc_channel_dump_pwr_range(b,  "DSSS              ", dsss_limits.pwr,
			WL_RATESET_SZ_DSSS);
		ppr_get_ofdm(txpwr, WL_TX_BW_20IN80, WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm_limits);
		wlc_channel_dump_pwr_range8(b, "OFDM              ", ofdm_limits.pwr);
		ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN80, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
		wlc_channel_dump_pwr_range_mcs(b, "MCS0_7            ", mcs_limits.pwr);
		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_get_dsss(txpwr, WL_TX_BW_20IN80, WL_TX_CHAINS_2, &dsss_limits);
			wlc_channel_dump_pwr_range(b, "DSSS_MULTI1       ", dsss_limits.pwr,
				WL_RATESET_SZ_DSSS);
			ppr_get_ofdm(txpwr, WL_TX_BW_20IN80, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
				&ofdm_limits);
			wlc_channel_dump_pwr_range8(b, "OFDM_CDD1         ", ofdm_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN80, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD1       ", mcs_limits.pwr);

			ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN80, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC       ", mcs_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN80, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS8_15           ", mcs_limits.pwr);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_get_dsss(txpwr, WL_TX_BW_20IN80, WL_TX_CHAINS_3, &dsss_limits);
				wlc_channel_dump_pwr_range(b,  "DSSS_MULTI2       ",
					dsss_limits.pwr, WL_RATESET_SZ_DSSS);
				ppr_get_ofdm(txpwr, WL_TX_BW_20IN80, WL_TX_MODE_CDD, WL_TX_CHAINS_3,
					&ofdm_limits);
				wlc_channel_dump_pwr_range8(b, "OFDM_CDD2         ",
					ofdm_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN80, WL_TX_NSS_1, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD2       ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN80, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP1",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN80, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP1    ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN80, WL_TX_NSS_3,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
					mcs_limits.pwr);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_get_dsss(txpwr, WL_TX_BW_20IN80, WL_TX_CHAINS_4,
						&dsss_limits);
					wlc_channel_dump_pwr_range(b,  "DSSS_MULTI3       ",
						dsss_limits.pwr, WL_RATESET_SZ_DSSS);
					ppr_get_ofdm(txpwr, WL_TX_BW_20IN80, WL_TX_MODE_CDD,
						WL_TX_CHAINS_4, &ofdm_limits);
					wlc_channel_dump_pwr_range8(b, "OFDM_CDD3         ",
						ofdm_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN80, WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD3       ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN80, WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP2",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN80, WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP2    ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN80, WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN80, WL_TX_NSS_4,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS24_31          ",
						mcs_limits.pwr);
				}
			}
		}

		bcm_bprintf(b, "\n40in80MHz:\n");

		ppr_get_ofdm(txpwr, WL_TX_BW_40IN80, WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm_limits);
		wlc_channel_dump_pwr_range8(b, "OFDM              ", ofdm_limits.pwr);
		ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN80, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
		wlc_channel_dump_pwr_range_mcs(b, "MCS0_7            ", mcs_limits.pwr);

		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_get_ofdm(txpwr, WL_TX_BW_40IN80, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
				&ofdm_limits);
			wlc_channel_dump_pwr_range8(b, "OFDM_CDD1         ", ofdm_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN80, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD1       ", mcs_limits.pwr);

			ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN80, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC       ", mcs_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN80, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS8_15           ", mcs_limits.pwr);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_get_ofdm(txpwr, WL_TX_BW_40IN80, WL_TX_MODE_CDD, WL_TX_CHAINS_3,
					&ofdm_limits);
				wlc_channel_dump_pwr_range8(b, "OFDM_CDD2         ",
					ofdm_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN80, WL_TX_NSS_1, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD2       ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN80, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP1",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN80, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP1    ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN80, WL_TX_NSS_3,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
					mcs_limits.pwr);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_get_ofdm(txpwr, WL_TX_BW_40IN80, WL_TX_MODE_CDD,
						WL_TX_CHAINS_4, &ofdm_limits);
					wlc_channel_dump_pwr_range8(b, "OFDM_CDD3         ",
						ofdm_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN80, WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD3       ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN80, WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP2",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN80, WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP2    ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN80, WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN80, WL_TX_NSS_4,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS24_31          ",
						mcs_limits.pwr);
				}
			}
		}
#endif /* WL11AC */

#ifdef WL11AC_160
	} else if (bw == WL_TX_BW_160) {
		bcm_bprintf(b, "\n160MHz:\n");

		ppr_get_ofdm(txpwr, WL_TX_BW_160, WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm_limits);
		wlc_channel_dump_pwr_range8(b, "OFDM              ", ofdm_limits.pwr);
		ppr_get_vht_mcs(txpwr, WL_TX_BW_160, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
		wlc_channel_dump_pwr_range_mcs(b, "MCS0_7            ", mcs_limits.pwr);

		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_get_ofdm(txpwr, WL_TX_BW_160, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
				&ofdm_limits);
			wlc_channel_dump_pwr_range8(b, "OFDM_CDD1         ", ofdm_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_160, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD1       ", mcs_limits.pwr);

			ppr_get_vht_mcs(txpwr, WL_TX_BW_160, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC       ", mcs_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_160, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS8_15           ", mcs_limits.pwr);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_get_ofdm(txpwr, WL_TX_BW_160, WL_TX_MODE_CDD, WL_TX_CHAINS_3,
					&ofdm_limits);
				wlc_channel_dump_pwr_range8(b, "OFDM_CDD2         ",
					ofdm_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_160, WL_TX_NSS_1, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD2       ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_160, WL_TX_NSS_2, WL_TX_MODE_STBC,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP1",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_160, WL_TX_NSS_2, WL_TX_MODE_NONE,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP1    ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_160, WL_TX_NSS_3, WL_TX_MODE_NONE,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
					mcs_limits.pwr);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_get_ofdm(txpwr, WL_TX_BW_160, WL_TX_MODE_CDD,
						WL_TX_CHAINS_4, &ofdm_limits);
					wlc_channel_dump_pwr_range8(b, "OFDM_CDD3         ",
						ofdm_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_160, WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD3       ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_160, WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP2",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_160, WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP2    ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_160, WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_160, WL_TX_NSS_4,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS24_31          ",
						mcs_limits.pwr);
				}
			}
		}
		bcm_bprintf(b, "\n20in160MHz:\n");
		ppr_get_dsss(txpwr, WL_TX_BW_20IN160, WL_TX_CHAINS_1, &dsss_limits);
		wlc_channel_dump_pwr_range(b,  "DSSS              ", dsss_limits.pwr,
			WL_RATESET_SZ_DSSS);
		ppr_get_ofdm(txpwr, WL_TX_BW_20IN160, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
			&ofdm_limits);
		wlc_channel_dump_pwr_range8(b, "OFDM              ", ofdm_limits.pwr);
		ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN160, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
		wlc_channel_dump_pwr_range_mcs(b, "MCS0_7            ", mcs_limits.pwr);
		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_get_dsss(txpwr, WL_TX_BW_20IN160, WL_TX_CHAINS_2, &dsss_limits);
			wlc_channel_dump_pwr_range(b, "DSSS_MULTI1       ", dsss_limits.pwr,
				WL_RATESET_SZ_DSSS);
			ppr_get_ofdm(txpwr, WL_TX_BW_20IN160, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
				&ofdm_limits);
			wlc_channel_dump_pwr_range8(b, "OFDM_CDD1         ", ofdm_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN160, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD1       ", mcs_limits.pwr);

			ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN160, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC       ", mcs_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN160, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS8_15           ", mcs_limits.pwr);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_get_dsss(txpwr, WL_TX_BW_20IN160, WL_TX_CHAINS_3,
					&dsss_limits);
				wlc_channel_dump_pwr_range(b,  "DSSS_MULTI2       ",
					dsss_limits.pwr, WL_RATESET_SZ_DSSS);
				ppr_get_ofdm(txpwr, WL_TX_BW_20IN160, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3, &ofdm_limits);
				wlc_channel_dump_pwr_range8(b, "OFDM_CDD2         ",
					ofdm_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN160, WL_TX_NSS_1,
					WL_TX_MODE_CDD, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD2       ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN160, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP1",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN160, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP1    ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN160, WL_TX_NSS_3,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
					mcs_limits.pwr);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_get_dsss(txpwr, WL_TX_BW_20IN160, WL_TX_CHAINS_4,
						&dsss_limits);
					wlc_channel_dump_pwr_range(b,  "DSSS_MULTI3       ",
						dsss_limits.pwr, WL_RATESET_SZ_DSSS);
					ppr_get_ofdm(txpwr, WL_TX_BW_20IN160, WL_TX_MODE_CDD,
						WL_TX_CHAINS_4, &ofdm_limits);
					wlc_channel_dump_pwr_range8(b, "OFDM_CDD3         ",
						ofdm_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN160, WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD3       ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN160, WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP2",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN160, WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP2    ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN160, WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN160, WL_TX_NSS_4,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS24_31          ",
						mcs_limits.pwr);
				}
			}
		}

		bcm_bprintf(b, "\n40in160MHz:\n");

		ppr_get_ofdm(txpwr, WL_TX_BW_40IN160, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
			&ofdm_limits);
		wlc_channel_dump_pwr_range8(b, "OFDM              ", ofdm_limits.pwr);
		ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN160, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
		wlc_channel_dump_pwr_range_mcs(b, "MCS0_7            ", mcs_limits.pwr);

		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_get_ofdm(txpwr, WL_TX_BW_40IN160, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
				&ofdm_limits);
			wlc_channel_dump_pwr_range8(b, "OFDM_CDD1         ", ofdm_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN160, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD1       ", mcs_limits.pwr);

			ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN160, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC       ", mcs_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN160, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS8_15           ", mcs_limits.pwr);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_get_ofdm(txpwr, WL_TX_BW_40IN160, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3, &ofdm_limits);
				wlc_channel_dump_pwr_range8(b, "OFDM_CDD2         ",
					ofdm_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN160, WL_TX_NSS_1,
					WL_TX_MODE_CDD, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD2       ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN160, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP1",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN160, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP1    ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN160, WL_TX_NSS_3,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
					mcs_limits.pwr);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_get_ofdm(txpwr, WL_TX_BW_40IN160, WL_TX_MODE_CDD,
						WL_TX_CHAINS_4, &ofdm_limits);
					wlc_channel_dump_pwr_range8(b, "OFDM_CDD3         ",
						ofdm_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN160, WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD3       ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN160, WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP2",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN160, WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP2    ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN160, WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN160, WL_TX_NSS_4,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS24_31          ",
						mcs_limits.pwr);
				}
			}
		}

		bcm_bprintf(b, "\n80in160MHz:\n");

		ppr_get_ofdm(txpwr, WL_TX_BW_80IN160, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
			&ofdm_limits);
		wlc_channel_dump_pwr_range8(b, "OFDM              ", ofdm_limits.pwr);
		ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN160, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
		wlc_channel_dump_pwr_range_mcs(b, "MCS0_7            ", mcs_limits.pwr);

		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_get_ofdm(txpwr, WL_TX_BW_80IN160, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
				&ofdm_limits);
			wlc_channel_dump_pwr_range8(b, "OFDM_CDD1         ", ofdm_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN160, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD1       ", mcs_limits.pwr);

			ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN160, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC       ", mcs_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN160, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS8_15           ", mcs_limits.pwr);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_get_ofdm(txpwr, WL_TX_BW_80IN160, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3, &ofdm_limits);
				wlc_channel_dump_pwr_range8(b, "OFDM_CDD2         ",
					ofdm_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN160, WL_TX_NSS_1,
					WL_TX_MODE_CDD, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD2       ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN160, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP1",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN160, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP1    ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN160, WL_TX_NSS_3,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
					mcs_limits.pwr);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_get_ofdm(txpwr, WL_TX_BW_80IN160, WL_TX_MODE_CDD,
						WL_TX_CHAINS_4, &ofdm_limits);
					wlc_channel_dump_pwr_range8(b, "OFDM_CDD3         ",
						ofdm_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN160, WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD3       ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN160, WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP2",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN160, WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP2    ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN160, WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN160, WL_TX_NSS_4,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS24_31          ",
						mcs_limits.pwr);
				}
			}
		}
#endif /* WL11AC_160 */

#ifdef WL11AC_80P80
	} else if (bw == WL_TX_BW_8080) {
		bcm_bprintf(b, "\n80+80MHz:\n");

		ppr_get_ofdm(txpwr, WL_TX_BW_8080, WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm_limits);
		wlc_channel_dump_pwr_range8(b, "OFDM              ", ofdm_limits.pwr);
		ppr_get_vht_mcs(txpwr, WL_TX_BW_8080, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
		wlc_channel_dump_pwr_range_mcs(b, "MCS0_7            ", mcs_limits.pwr);

		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_get_ofdm(txpwr, WL_TX_BW_8080, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
				&ofdm_limits);
			wlc_channel_dump_pwr_range8(b, "OFDM_CDD1         ", ofdm_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_8080, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD1       ", mcs_limits.pwr);

			ppr_get_vht_mcs(txpwr, WL_TX_BW_8080, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC       ", mcs_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_8080, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS8_15           ", mcs_limits.pwr);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_get_ofdm(txpwr, WL_TX_BW_8080, WL_TX_MODE_CDD, WL_TX_CHAINS_3,
					&ofdm_limits);
				wlc_channel_dump_pwr_range8(b, "OFDM_CDD2         ",
					ofdm_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_8080, WL_TX_NSS_1, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD2       ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_8080, WL_TX_NSS_2, WL_TX_MODE_STBC,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP1",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_8080, WL_TX_NSS_2, WL_TX_MODE_NONE,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP1    ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_8080, WL_TX_NSS_3, WL_TX_MODE_NONE,
					WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
					mcs_limits.pwr);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_get_ofdm(txpwr, WL_TX_BW_8080, WL_TX_MODE_CDD,
						WL_TX_CHAINS_4, &ofdm_limits);
					wlc_channel_dump_pwr_range8(b, "OFDM_CDD3         ",
						ofdm_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_8080, WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD3       ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_8080, WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP2",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_8080, WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP2    ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_8080, WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_8080, WL_TX_NSS_4,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS24_31          ",
						mcs_limits.pwr);
				}
			}
		}
		bcm_bprintf(b, "\n80+80MHz chan2:\n");
		ppr_get_dsss(txpwr, WL_TX_BW_8080CHAN2, WL_TX_CHAINS_1, &dsss_limits);
		wlc_channel_dump_pwr_range(b,  "DSSS              ", dsss_limits.pwr,
			WL_RATESET_SZ_DSSS);
		ppr_get_ofdm(txpwr, WL_TX_BW_8080CHAN2, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
			&ofdm_limits);
		wlc_channel_dump_pwr_range8(b, "OFDM              ", ofdm_limits.pwr);
		ppr_get_vht_mcs(txpwr, WL_TX_BW_8080CHAN2, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
		wlc_channel_dump_pwr_range_mcs(b, "MCS0_7            ", mcs_limits.pwr);
		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_get_dsss(txpwr, WL_TX_BW_8080CHAN2, WL_TX_CHAINS_2, &dsss_limits);
			wlc_channel_dump_pwr_range(b, "DSSS_MULTI1       ", dsss_limits.pwr,
				WL_RATESET_SZ_DSSS);
			ppr_get_ofdm(txpwr, WL_TX_BW_8080CHAN2, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
				&ofdm_limits);
			wlc_channel_dump_pwr_range8(b, "OFDM_CDD1         ", ofdm_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_8080CHAN2, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD1       ", mcs_limits.pwr);

			ppr_get_vht_mcs(txpwr, WL_TX_BW_8080CHAN2, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC       ", mcs_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_8080CHAN2, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS8_15           ", mcs_limits.pwr);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_get_dsss(txpwr, WL_TX_BW_8080CHAN2, WL_TX_CHAINS_3,
					&dsss_limits);
				wlc_channel_dump_pwr_range(b,  "DSSS_MULTI2       ",
					dsss_limits.pwr, WL_RATESET_SZ_DSSS);
				ppr_get_ofdm(txpwr, WL_TX_BW_8080CHAN2, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3, &ofdm_limits);
				wlc_channel_dump_pwr_range8(b, "OFDM_CDD2         ",
					ofdm_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_8080CHAN2, WL_TX_NSS_1,
					WL_TX_MODE_CDD, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD2       ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_8080CHAN2, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP1",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_8080CHAN2, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP1    ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_8080CHAN2, WL_TX_NSS_3,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
					mcs_limits.pwr);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_get_dsss(txpwr, WL_TX_BW_8080CHAN2, WL_TX_CHAINS_4,
						&dsss_limits);
					wlc_channel_dump_pwr_range(b,  "DSSS_MULTI3       ",
						dsss_limits.pwr, WL_RATESET_SZ_DSSS);
					ppr_get_ofdm(txpwr, WL_TX_BW_8080CHAN2, WL_TX_MODE_CDD,
						WL_TX_CHAINS_4, &ofdm_limits);
					wlc_channel_dump_pwr_range8(b, "OFDM_CDD3         ",
						ofdm_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_8080CHAN2, WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD3       ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_8080CHAN2, WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP2",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_8080CHAN2, WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP2    ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_8080CHAN2, WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_8080CHAN2, WL_TX_NSS_4,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS24_31          ",
						mcs_limits.pwr);
				}
			}
		}

		bcm_bprintf(b, "\n20in80+80MHz:\n");

		ppr_get_ofdm(txpwr, WL_TX_BW_20IN8080, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
			&ofdm_limits);
		wlc_channel_dump_pwr_range8(b, "OFDM              ", ofdm_limits.pwr);
		ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN8080, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
		wlc_channel_dump_pwr_range_mcs(b, "MCS0_7            ", mcs_limits.pwr);

		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_get_ofdm(txpwr, WL_TX_BW_20IN8080, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
				&ofdm_limits);
			wlc_channel_dump_pwr_range8(b, "OFDM_CDD1         ", ofdm_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN8080, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD1       ", mcs_limits.pwr);

			ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN8080, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC       ", mcs_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN8080, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS8_15           ", mcs_limits.pwr);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_get_ofdm(txpwr, WL_TX_BW_20IN8080, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3, &ofdm_limits);
				wlc_channel_dump_pwr_range8(b, "OFDM_CDD2         ",
					ofdm_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN8080, WL_TX_NSS_1,
					WL_TX_MODE_CDD, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD2       ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN8080, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP1",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN8080, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP1    ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN8080, WL_TX_NSS_3,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
					mcs_limits.pwr);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_get_ofdm(txpwr, WL_TX_BW_20IN8080, WL_TX_MODE_CDD,
						WL_TX_CHAINS_4, &ofdm_limits);
					wlc_channel_dump_pwr_range8(b, "OFDM_CDD3         ",
						ofdm_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN8080, WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD3       ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN8080, WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP2",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN8080, WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP2    ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN8080, WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_20IN8080, WL_TX_NSS_4,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS24_31          ",
						mcs_limits.pwr);
				}
			}
		}

		bcm_bprintf(b, "\n40in80+80MHz:\n");

		ppr_get_ofdm(txpwr, WL_TX_BW_40IN8080, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
			&ofdm_limits);
		wlc_channel_dump_pwr_range8(b, "OFDM              ", ofdm_limits.pwr);
		ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN8080, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
		wlc_channel_dump_pwr_range_mcs(b, "MCS0_7            ", mcs_limits.pwr);

		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_get_ofdm(txpwr, WL_TX_BW_40IN8080, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
				&ofdm_limits);
			wlc_channel_dump_pwr_range8(b, "OFDM_CDD1         ", ofdm_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN8080, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD1       ", mcs_limits.pwr);

			ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN8080, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC       ", mcs_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN8080, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS8_15           ", mcs_limits.pwr);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_get_ofdm(txpwr, WL_TX_BW_40IN8080, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3, &ofdm_limits);
				wlc_channel_dump_pwr_range8(b, "OFDM_CDD2         ",
					ofdm_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN8080, WL_TX_NSS_1,
					WL_TX_MODE_CDD, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD2       ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN8080, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP1",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN8080, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP1    ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN8080, WL_TX_NSS_3,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
					mcs_limits.pwr);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_get_ofdm(txpwr, WL_TX_BW_40IN8080, WL_TX_MODE_CDD,
						WL_TX_CHAINS_4, &ofdm_limits);
					wlc_channel_dump_pwr_range8(b, "OFDM_CDD3         ",
						ofdm_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN8080, WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD3       ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN8080, WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP2",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN8080, WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP2    ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN8080, WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_40IN8080, WL_TX_NSS_4,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS24_31          ",
						mcs_limits.pwr);
				}
			}
		}

		bcm_bprintf(b, "\n80in80+80MHz:\n");

		ppr_get_ofdm(txpwr, WL_TX_BW_80IN8080, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
			&ofdm_limits);
		wlc_channel_dump_pwr_range8(b, "OFDM              ", ofdm_limits.pwr);
		ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN8080, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
		wlc_channel_dump_pwr_range_mcs(b, "MCS0_7            ", mcs_limits.pwr);

		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_get_ofdm(txpwr, WL_TX_BW_80IN8080, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
				&ofdm_limits);
			wlc_channel_dump_pwr_range8(b, "OFDM_CDD1         ", ofdm_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN8080, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD1       ", mcs_limits.pwr);

			ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN8080, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC       ", mcs_limits.pwr);
			ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN8080, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_2, &mcs_limits);
			wlc_channel_dump_pwr_range_mcs(b, "MCS8_15           ", mcs_limits.pwr);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_get_ofdm(txpwr, WL_TX_BW_80IN8080, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3, &ofdm_limits);
				wlc_channel_dump_pwr_range8(b, "OFDM_CDD2         ",
					ofdm_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN8080, WL_TX_NSS_1,
					WL_TX_MODE_CDD, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD2       ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN8080, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP1",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN8080, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP1    ",
					mcs_limits.pwr);
				ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN8080, WL_TX_NSS_3,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3, &mcs_limits);
				wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
					mcs_limits.pwr);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_get_ofdm(txpwr, WL_TX_BW_80IN8080, WL_TX_MODE_CDD,
						WL_TX_CHAINS_4, &ofdm_limits);
					wlc_channel_dump_pwr_range8(b, "OFDM_CDD3         ",
						ofdm_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN8080, WL_TX_NSS_1,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_CDD3       ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN8080, WL_TX_NSS_2,
						WL_TX_MODE_STBC, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS0_7_STBC_SPEXP2",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN8080, WL_TX_NSS_2,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS8_15_SPEXP2    ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN8080, WL_TX_NSS_3,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS16_23          ",
						mcs_limits.pwr);
					ppr_get_vht_mcs(txpwr, WL_TX_BW_80IN8080, WL_TX_NSS_4,
						WL_TX_MODE_NONE, WL_TX_CHAINS_4, &mcs_limits);
					wlc_channel_dump_pwr_range_mcs(b, "MCS24_31          ",
						mcs_limits.pwr);
				}
			}
		}
#endif /* WL11AC_80P80 */
	}

	bcm_bprintf(b, "\n");
}
#endif /* BCMDBG || BCMDBG_DUMP */

/*
 * 	if (wlc->country_list_extended) all country listable.
 *	else J1 - J10 is excluded.
 */
static bool
wlc_country_listable(struct wlc_info *wlc, const char *countrystr)
{
	bool listable = TRUE;

	if (wlc->country_list_extended == FALSE) {
		if (countrystr[0] == 'J' &&
			(countrystr[1] >= '1' && countrystr[1] <= '9'))
			listable = FALSE;
	}

	return listable;
}

static bool
wlc_buffalo_map_locale(struct wlc_info *wlc, const char* abbrev)
{
	if ((wlc->pub->sih->boardvendor == VENDOR_BUFFALO) &&
	    D11REV_GT(wlc->pub->corerev, 5) && !strcmp("JP", abbrev))
		return TRUE;
	else
		return FALSE;
}

clm_country_t
wlc_get_country(struct wlc_info *wlc)
{
	return wlc->cmi->cm->country;
}

int
wlc_get_channels_in_country(struct wlc_info *wlc, void *arg)
{
	chanvec_t channels;
	wl_channels_in_country_t *cic = (wl_channels_in_country_t *)arg;
	chanvec_t sup_chan;
	uint count, need, i;

	if ((cic->band != WLC_BAND_5G) && (cic->band != WLC_BAND_2G)) {
		WL_ERROR(("Invalid band %d\n", cic->band));
		return BCME_BADBAND;
	}

	if ((NBANDS(wlc) == 1) && (cic->band != (uint)wlc->band->bandtype)) {
		WL_ERROR(("Invalid band %d for card\n", cic->band));
		return BCME_BADBAND;
	}

	if (wlc_channel_get_chanvec(wlc, cic->country_abbrev, cic->band, &channels) == FALSE) {
		WL_ERROR(("Invalid country %s\n", cic->country_abbrev));
		return BCME_NOTFOUND;
	}

	phy_utils_chanspec_band_validch((phy_info_t *)WLC_PI(wlc), cic->band, &sup_chan);
	for (i = 0; i < sizeof(chanvec_t); i++)
		sup_chan.vec[i] &= channels.vec[i];

	/* find all valid channels */
	for (count = 0, i = 0; i < sizeof(sup_chan.vec)*NBBY; i++) {
		if (isset(sup_chan.vec, i))
			count++;
	}

	need = sizeof(wl_channels_in_country_t) + count*sizeof(cic->channel[0]);

	if (need > cic->buflen) {
		/* too short, need this much */
		WL_ERROR(("WLC_GET_COUNTRY_LIST: Buffer size: Need %d Received %d\n",
			need, cic->buflen));
		cic->buflen = need;
		return BCME_BUFTOOSHORT;
	}

	for (count = 0, i = 0; i < sizeof(sup_chan.vec)*NBBY; i++) {
		if (isset(sup_chan.vec, i))
			cic->channel[count++] = i;
	}

	cic->count = count;
	return 0;
}

int
wlc_get_country_list(struct wlc_info *wlc, void *arg)
{
	chanvec_t channels;
	chanvec_t unused;
	wl_country_list_t *cl = (wl_country_list_t *)arg;
	clm_country_locales_t locale;
	chanvec_t sup_chan;
	uint count, need, j;
	clm_country_t country_iter;

	ccode_t cc;
	unsigned int regrev;

	if (cl->band_set == FALSE) {
		/* get for current band */
		cl->band = wlc->band->bandtype;
	}

	if ((cl->band != WLC_BAND_5G) && (cl->band != WLC_BAND_2G)) {
		WL_ERROR(("Invalid band %d\n", cl->band));
		return BCME_BADBAND;
	}

	if ((NBANDS(wlc) == 1) && (cl->band != (uint)wlc->band->bandtype)) {
		WL_INFORM(("Invalid band %d for card\n", cl->band));
		cl->count = 0;
		return 0;
	}

	phy_utils_chanspec_band_validch((phy_info_t *)WLC_PI(wlc), cl->band, &sup_chan);

	need = sizeof(wl_country_list_t);
	count = 0;
	if (clm_iter_init(&country_iter) == CLM_RESULT_OK) {
		ccode_t prev_cc = "";
		while (clm_country_iter(&country_iter, cc, &regrev) == CLM_RESULT_OK) {
			if (wlc_get_locale(country_iter, &locale) == CLM_RESULT_OK) {
				if (cl->band == WLC_BAND_5G) {
					wlc_locale_get_channels(&locale, CLM_BAND_5G, &channels,
						&unused);
				} else {
					wlc_locale_get_channels(&locale, CLM_BAND_2G, &channels,
						&unused);
				}
				for (j = 0; j < sizeof(sup_chan.vec); j++) {
					if (sup_chan.vec[j] & channels.vec[j]) {
						if ((wlc_country_listable(wlc, cc)) &&
							memcmp(cc, prev_cc,
							sizeof(ccode_t))) {
							memcpy(prev_cc, cc, sizeof(ccode_t));
							need += WLC_CNTRY_BUF_SZ;
							if (need > cl->buflen) {
								continue;
							}
							strncpy(&cl->country_abbrev
								[count*WLC_CNTRY_BUF_SZ],
								cc, sizeof(ccode_t));
							/* terminate the string */
							cl->country_abbrev[count*WLC_CNTRY_BUF_SZ +
								sizeof(ccode_t)] = 0;
							count++;
						}
						break;
					}
				}
			}
		}
	}

	if (need > cl->buflen) {
		/* too short, need this much */
		WL_ERROR(("WLC_GET_COUNTRY_LIST: Buffer size: Need %d Received %d\n",
			need, cl->buflen));
		cl->buflen = need;
		return BCME_BUFTOOSHORT;
	}

	cl->count = count;
	return 0;
}

/* Get regulatory max power for a given channel in a given locale.
 * for external FALSE, it returns limit for brcm hw
 * ---- for 2.4GHz channel, it returns cck limit, not ofdm limit.
 * for external TRUE, it returns 802.11d Country Information Element -
 * 	Maximum Transmit Power Level.
 */
int8
wlc_get_reg_max_power_for_channel(wlc_cm_info_t *wlc_cmi, int chan, bool external)
{
	int8 maxpwr = WL_RATE_DISABLED;

	clm_country_locales_t locales;
	wlc_info_t *wlc = wlc_cmi->wlc;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	clm_country_t country = wlc_cm->country;
	clm_result_t result;

	if (country == CLM_ITER_NULL) {
		result = wlc_country_lookup_direct(wlc_cm->ccode, wlc_cm->regrev, &country);
		if (result != CLM_RESULT_OK) {
			wlc_cm->country = CLM_ITER_NULL;
			return WL_RATE_DISABLED;
		} else {
			wlc_cm->country = country;
		}
	}

	result = wlc_get_locale(country, &locales);
	if (result != CLM_RESULT_OK) {
		return WL_RATE_DISABLED;
	}

	if (external) {
		int int_limit;
		if (clm_regulatory_limit(&locales, BAND_5G(wlc->band->bandtype) ?
			CLM_BAND_5G : CLM_BAND_2G, chan, &int_limit) == CLM_RESULT_OK) {
			maxpwr = (uint8)int_limit;
		}
	} else {
		clm_power_limits_t limits;
		clm_limits_params_t lim_params;

		if (clm_limits_params_init(&lim_params) == CLM_RESULT_OK) {
			if (clm_limits(&locales,
				chan <= CH_MAX_2G_CHANNEL ? CLM_BAND_2G : CLM_BAND_5G,
				chan, 0, CLM_LIMITS_TYPE_CHANNEL, &lim_params,
				&limits) == CLM_RESULT_OK) {
				int i;

				for (i = 0; i < WL_NUMRATES; i++) {
					if (maxpwr < limits.limit[i])
						maxpwr = limits.limit[i];
				}
			}
		}
	}

	return (maxpwr);
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_dump_max_power_per_channel(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b)
{
	wlc_info_t *wlc = wlc_cmi->wlc;

	int8 ext_pwr = wlc_get_reg_max_power_for_channel(wlc_cmi,
		wf_chspec_ctlchan(wlc->chanspec), TRUE);
	/* int8 int_pwr = wlc_get_reg_max_power_for_channel(wlc_cm,
	   CHSPEC_CHANNEL(wlc->chanspec), FALSE);
	*/

	/* bcm_bprintf(b, "Reg Max Power: %d External: %d\n", int_pwr, ext_pwr); */
	bcm_bprintf(b, "Reg Max Power (External) %d\n", ext_pwr);
	return 0;
}

static int wlc_get_clm_limits(wlc_cm_info_t *wlc_cmi, clm_band_t bandtype,
	clm_bandwidth_t bw, clm_power_limits_t *limits,
	clm_power_limits_t *limits20in40)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	chanspec_t chanspec = wlc->chanspec;
	unsigned int chan;
	clm_country_t country;
	clm_result_t result = CLM_RESULT_ERR;
	clm_country_locales_t locale;
	wlcband_t* band;
	int ant_gain;
	clm_limits_params_t lim_params;

	memset(limits, (unsigned char)WL_RATE_DISABLED, sizeof(clm_power_limits_t));
	memset(limits20in40, (unsigned char)WL_RATE_DISABLED, sizeof(clm_power_limits_t));

	country = wlc_cmi->cm->country;
	chan = CHSPEC_CHANNEL(chanspec);
	band = wlc->bandstate[CHSPEC_WLCBANDUNIT(chanspec)];
	ant_gain = band->antgain;

	result = wlc_get_locale(country, &locale);
	if (result != CLM_RESULT_OK)
		WL_ERROR(("wlc_get_locale failed\n"));

	if ((result == CLM_RESULT_OK) && (clm_limits_params_init(&lim_params) == CLM_RESULT_OK)) {
		lim_params.sar = band->sar;
		lim_params.bw = bw;

		result = clm_limits(&locale, bandtype, chan, ant_gain, CLM_LIMITS_TYPE_CHANNEL,
			&lim_params, limits);
	}

	if ((result == CLM_RESULT_OK) && (bw == CLM_BW_40) && (limits20in40 != NULL))
		result = clm_limits(&locale, bandtype, chan, ant_gain,
			CHSPEC_SB_UPPER(chanspec) ?
			CLM_LIMITS_TYPE_SUBCHAN_U : CLM_LIMITS_TYPE_SUBCHAN_L,
			&lim_params, limits20in40);
	if (result != CLM_RESULT_OK)
		WL_ERROR(("clm_limits failed\n"));

	return 0;
}

static int wlc_dump_clm_limits_2G_20M(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b)
{
	clm_power_limits_t limits;
	clm_power_limits_t limits20in40;
	int i;

	wlc_get_clm_limits(wlc_cmi, CLM_BAND_2G, CLM_BW_20, &limits, &limits20in40);

	for (i = 0; i < WL_NUMRATES; i++) {
		bcm_bprintf(b, "%d\n", limits.limit[i]);
	}

	return 0;
}
static int wlc_dump_clm_limits_2G_40M(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b)
{
	clm_power_limits_t limits;
	clm_power_limits_t limits20in40;
	int i;

	wlc_get_clm_limits(wlc_cmi, CLM_BAND_2G, CLM_BW_40, &limits, &limits20in40);

	for (i = 0; i < WL_NUMRATES; i++) {
		bcm_bprintf(b, "%d\n", limits.limit[i]);
	}

	return 0;
}
static int wlc_dump_clm_limits_2G_20in40M(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b)
{
	clm_power_limits_t limits;
	clm_power_limits_t limits20in40;
	int i;

	wlc_get_clm_limits(wlc_cmi, CLM_BAND_2G, CLM_BW_40, &limits, &limits20in40);

	for (i = 0; i < WL_NUMRATES; i++) {
		bcm_bprintf(b, "%d\n", limits20in40.limit[i]);
	}

	return 0;
}
static int wlc_dump_clm_limits_5G_20M(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b)
{
	clm_power_limits_t limits;
	clm_power_limits_t limits20in40;
	int i;

	wlc_get_clm_limits(wlc_cmi, CLM_BAND_5G, CLM_BW_20, &limits, &limits20in40);

	for (i = 0; i < WL_NUMRATES; i++) {
		bcm_bprintf(b, "%d\n", limits.limit[i]);
	}

	return 0;
}
static int wlc_dump_clm_limits_5G_40M(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b)
{
	clm_power_limits_t limits;
	clm_power_limits_t limits20in40;
	int i;

	wlc_get_clm_limits(wlc_cmi, CLM_BAND_5G, CLM_BW_40, &limits, &limits20in40);

	for (i = 0; i < WL_NUMRATES; i++) {
		bcm_bprintf(b, "%d\n", limits.limit[i]);
	}

	return 0;
}
static int wlc_dump_clm_limits_5G_20in40M(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b)
{
	clm_power_limits_t limits;
	clm_power_limits_t limits20in40;
	int i;

	wlc_get_clm_limits(wlc_cmi, CLM_BAND_5G, CLM_BW_40, &limits, &limits20in40);

	for (i = 0; i < WL_NUMRATES; i++) {
		bcm_bprintf(b, "%d\n", limits20in40.limit[i]);
	}

	return 0;
}
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */

static bool
wlc_channel_defined_80MHz_channel(uint channel)
{
	int i;
	/* 80MHz channels in 5GHz band */
	static const uint8 defined_5g_80m_chans[] =
	        {42, 58, 106, 122, 138, 155};

	for (i = 0; i < (int)ARRAYSIZE(defined_5g_80m_chans); i++) {
		if (channel == defined_5g_80m_chans[i]) {
			return TRUE;
		}
	}
	return FALSE;
}

static bool
wlc_channel_clm_chanspec_valid(wlc_cm_info_t *wlc_cmi, chanspec_t chspec)
{
		wlc_cm_data_t	*wlc_cm = wlc_cmi->cm;

	if (CHSPEC_IS8080(chspec) || CHSPEC_IS160(chspec))
	{
		return (isset(wlc_cm->allowed_5g_4080channels.vec,
			wf_chspec_primary80_channel(chspec))&&
			isset(wlc_cm->allowed_5g_4080channels.vec,
			wf_chspec_secondary80_channel(chspec)));
	}
	return (CHSPEC_IS2G(chspec) ||
	 (CHSPEC_IS20(chspec) &&
	  isset(wlc_cm->allowed_5g_20channels.vec, CHSPEC_CHANNEL(chspec))) ||
	  isset(wlc_cm->allowed_5g_4080channels.vec, CHSPEC_CHANNEL(chspec)));
}

/*
 * Validate the chanspec for this locale, for 40MHz we need to also check that the sidebands
 * are valid 20MHz channels in this locale and they are also a legal HT combination
 */
static bool
wlc_valid_chanspec_ext(wlc_cm_info_t *wlc_cmi, chanspec_t chspec, bool dualband)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	uint8 channel = CHSPEC_CHANNEL(chspec);

	/* check the chanspec */
	if (wf_chspec_malformed(chspec)) {
		WL_ERROR(("wl%d: malformed chanspec 0x%x\n", wlc->pub->unit, chspec));
		ASSERT(0);
		return FALSE;
	}

	/* check channel range is in band range */
	if (CHANNEL_BANDUNIT(wlc_cmi->wlc, channel) != CHSPEC_WLCBANDUNIT(chspec))
		return FALSE;

	if (CHSPEC_IS5G(chspec) && IS_5G_CH_GRP_DISABLED(wlc, channel)) {
		return FALSE;
	}

	/* Check a 20Mhz channel */
	if (CHSPEC_IS20(chspec)) {
		if (dualband)
			return (VALID_CHANNEL20_DB(wlc_cmi->wlc, channel));
		else
			return (VALID_CHANNEL20(wlc_cmi->wlc, channel));
	}

	/* Check a 2.5/5/10Mhz channel */
	if (CHSPEC_IS_ULB(wlc, chspec)) {
		if (CHSPEC_IS2P5(chspec) && !WLC_2P5MHZ_ULB_SUPP_BAND(wlc, CHSPEC_BANDUNIT(chspec)))
			return FALSE;

		if (CHSPEC_IS5(chspec) && !WLC_5MHZ_ULB_SUPP_BAND(wlc, CHSPEC_BANDUNIT(chspec)))
			return FALSE;

		if (CHSPEC_IS10(chspec) && !WLC_10MHZ_ULB_SUPP_BAND(wlc, CHSPEC_BANDUNIT(chspec)))
			return FALSE;

		if (dualband)
			return (WLC_LE20_VALID_CHANNEL_DB(wlc_cmi->wlc, CHSPEC_BW(chspec),
				channel));
		else
			return (WLC_LE20_VALID_CHANNEL(wlc_cmi->wlc, CHSPEC_BW(chspec), channel));
	}

	/* Check a 40Mhz channel */
	if (CHSPEC_IS40(chspec)) {
		uint8 upper_sideband = 0, idx;
		uint8 num_ch20_entries = sizeof(chan20_info)/sizeof(struct chan20_info);

		if (!wlc->pub->phy_bw40_capable) {
			return FALSE;
		}

		if (!VALID_40CHANSPEC_IN_BAND(wlc, CHSPEC_WLCBANDUNIT(chspec)))
			return FALSE;

		if (dualband) {
			if (!VALID_CHANNEL20_DB(wlc, LOWER_20_SB(channel)) ||
			    !VALID_CHANNEL20_DB(wlc, UPPER_20_SB(channel)))
				return FALSE;
		} else {
			if (!VALID_CHANNEL20(wlc, LOWER_20_SB(channel)) ||
			    !VALID_CHANNEL20(wlc, UPPER_20_SB(channel)))
				return FALSE;
		}

		if (!wlc_channel_clm_chanspec_valid(wlc_cmi, chspec))
			return FALSE;

		/* find the lower sideband info in the sideband array */
		for (idx = 0; idx < num_ch20_entries; idx++) {
			if (chan20_info[idx].sb == LOWER_20_SB(channel))
				upper_sideband = chan20_info[idx].adj_sbs;
		}
		/* check that the lower sideband allows an upper sideband */
		if ((upper_sideband & (CH_UPPER_SB | CH_EWA_VALID)) == (CH_UPPER_SB | CH_EWA_VALID))
			return TRUE;
		return FALSE;
	}

	/* Check a 80MHz channel - only 5G band supports 80MHz */

	if (CHSPEC_IS80(chspec)) {
		chanspec_t chspec40;

		/* Only 5G supports 80MHz
		 * Check the chanspec band with BAND_5G() instead of the more straightforward
		 * CHSPEC_IS5G() since BAND_5G() is conditionally compiled on BAND5G support. This
		 * check will turn into a constant check when compiling without BAND5G support.
		 */
		if (!BAND_5G(CHSPEC2WLC_BAND(chspec))) {
			return FALSE;
		}

		/* Make sure that the phy is 80MHz capable and that
		 * we are configured for 80MHz on the band
		 */
		if (!wlc->pub->phy_bw80_capable ||
		    !WL_BW_CAP_80MHZ(wlc->bandstate[BAND_5G_INDEX]->bw_cap)) {
			return FALSE;
		}

		/* Ensure that vhtmode is enabled if applicable */
		if (!VHT_ENAB_BAND(wlc->pub, WLC_BAND_5G)) {
			return FALSE;
		}

		if (!VALID_80CHANSPEC_IN_BAND(wlc, CHSPEC_WLCBANDUNIT(chspec)))
			return FALSE;

		/* Check that the 80MHz center channel is a defined channel */
		if (!wlc_channel_defined_80MHz_channel(channel)) {
			return FALSE;
		}

		if (!wlc_channel_clm_chanspec_valid(wlc_cmi, chspec))
			return FALSE;

		/* Make sure both 40 MHz side channels are valid
		 * Create a chanspec for each 40MHz side side band and check
		 */
		chspec40 = (chanspec_t)((channel - CH_20MHZ_APART) |
		                        WL_CHANSPEC_CTL_SB_L |
		                        WL_CHANSPEC_BW_40 |
		                        WL_CHANSPEC_BAND_5G);
		if (!wlc_valid_chanspec_ext(wlc_cmi, chspec40, dualband)) {
			WL_TMP(("wl%d: %s: 80MHz: chanspec %0X -> chspec40 %0X "
			        "failed valid check\n",
			        wlc->pub->unit, __FUNCTION__, chspec, chspec40));

			return FALSE;
		}

		chspec40 = (chanspec_t)((channel + CH_20MHZ_APART) |
		                        WL_CHANSPEC_CTL_SB_L |
		                        WL_CHANSPEC_BW_40 |
		                        WL_CHANSPEC_BAND_5G);
		if (!wlc_valid_chanspec_ext(wlc_cmi, chspec40, dualband)) {
			WL_TMP(("wl%d: %s: 80MHz: chanspec %0X -> chspec40 %0X "
			        "failed valid check\n",
			        wlc->pub->unit, __FUNCTION__, chspec, chspec40));

			return FALSE;
		}

		return TRUE;
	}
	if (CHSPEC_IS8080(chspec) || CHSPEC_IS160(chspec)) {
		chanspec_t chspec80;

		/* Only 5G supports 80 +80 MHz
		 * Check the chanspec band with BAND_5G() instead of the more straightforward
		 * CHSPEC_IS5G() since BAND_5G() is conditionally compiled on BAND5G support. This
		 * check will turn into a constant check when compiling without BAND5G support.
		 */
		if (!BAND_5G(CHSPEC2WLC_BAND(chspec))) {
			return FALSE;
		}

		/* Make sure that the phy is 80+80 MHz capable and that
		 * we are configured for 160MHz on the band
		 */
		if (CHSPEC_IS8080(chspec)) {
			if (!wlc->pub->phy_bw8080_capable ||
				!WL_BW_CAP_160MHZ(wlc->bandstate[BAND_5G_INDEX]->bw_cap))
				return FALSE;
		}

		/* Make sure that the phy is 160 MHz capable and that
		 * we are configured for 160MHz on the band
		 */
		if (CHSPEC_IS160(chspec)) {
			if (!wlc->pub->phy_bw160_capable ||
				!WL_BW_CAP_160MHZ(wlc->bandstate[BAND_5G_INDEX]->bw_cap))
				return FALSE;
		}

		/* Ensure that vhtmode is enabled if applicable */
		if (!VHT_ENAB_BAND(wlc->pub, WLC_BAND_5G)) {
			return FALSE;
		}

		if (CHSPEC_IS8080(chspec)) {
			if (!VALID_8080CHANSPEC_IN_BAND(wlc, CHSPEC_WLCBANDUNIT(chspec)))
				return FALSE;
		}

		if (CHSPEC_IS160(chspec)) {
			if (!VALID_160CHANSPEC_IN_BAND(wlc, CHSPEC_WLCBANDUNIT(chspec)))
				return FALSE;
		}

		chspec80 = (chanspec_t)(wf_chspec_primary80_channel(chspec) |
		                        WL_CHANSPEC_CTL_SB_L |
		                        WL_CHANSPEC_BW_80 |
		                        WL_CHANSPEC_BAND_5G);
		if (!wlc_valid_chanspec_ext(wlc_cmi, chspec80, dualband)) {
			WL_TMP(("wl%d: %s: 80 + 80 MHz: chanspec %0X -> chspec80 %0X "
			        "failed valid check\n",
			        wlc->pub->unit, __FUNCTION__, chspec, chspec80));

			return FALSE;
		}

		chspec80 = (chanspec_t)(wf_chspec_secondary80_channel(chspec) |
		                        WL_CHANSPEC_CTL_SB_L |
		                        WL_CHANSPEC_BW_80 |
		                        WL_CHANSPEC_BAND_5G);
		if (!wlc_valid_chanspec_ext(wlc_cmi, chspec80, dualband)) {
			WL_TMP(("wl%d: %s: 80 + 80 MHz: chanspec %0X -> chspec80 %0X "
			        "failed valid check\n",
			        wlc->pub->unit, __FUNCTION__, chspec, chspec80));

			return FALSE;
		}
		return TRUE;
	}

	return FALSE;
}

bool
wlc_valid_chanspec(wlc_cm_info_t *wlc_cmi, chanspec_t chspec)
{
	return wlc_valid_chanspec_ext(wlc_cmi, chspec, FALSE);
}

bool
wlc_valid_chanspec_db(wlc_cm_info_t *wlc_cmi, chanspec_t chspec)
{
	return wlc_valid_chanspec_ext(wlc_cmi, chspec, TRUE);
}

/*
 *  Fill in 'list' with validated chanspecs, looping through channels using the chanspec_mask.
 */
static void
wlc_chanspec_list(wlc_info_t *wlc, wl_uint32_list_t *list, chanspec_t chanspec_mask)
{
	uint8 channel;
	chanspec_t chanspec;

	for (channel = 0; channel < MAXCHANNEL; channel++) {
		chanspec = (chanspec_mask | channel);
		if (!wf_chspec_malformed(chanspec) &&
		    ((NBANDS(wlc) > 1) ? wlc_valid_chanspec_db(wlc->cmi, chanspec) :
		     wlc_valid_chanspec(wlc->cmi, chanspec))) {
			list->element[list->count] = chanspec;
			list->count++;
		}
	}
}

/*
 *  Returns TRUE if radio chanspec need to be updated compared to target chanspec
 */
bool
wlc_chk_chanspec_update(struct wlc_info *wlc, chanspec_t target_ch)
{
	if ((WLC_BAND_PI_RADIO_CHANSPEC != target_ch) &&
		(!(WLC_SAME_CTLCHAN(WLC_BAND_PI_RADIO_CHANSPEC, target_ch) &&
#ifdef WL11ULB
		CHSPEC_BW_GT(WLC_BAND_PI_RADIO_CHANSPEC, CHSPEC_BW(target_ch))) ||
		CHSPEC_IS_ULB_EITHER(wlc, WLC_BAND_PI_RADIO_CHANSPEC, target_ch)) &&
#else /* WL11ULB */
		CHSPEC_BW(WLC_BAND_PI_RADIO_CHANSPEC) > CHSPEC_BW(target_ch))) &&
#endif /* WL11ULB */
		TRUE) {
		return TRUE;
	}

	return FALSE;
}

/*
 * Returns a list of valid chanspecs meeting the provided settings
 */
void
wlc_get_valid_chanspecs(wlc_cm_info_t *wlc_cmi, wl_uint32_list_t *list, uint bw, bool band2G,
                        const char *abbrev)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	chanspec_t chanspec;
	clm_country_t country;
	clm_result_t result = CLM_RESULT_ERR;
	clm_result_t flag_result = CLM_RESULT_ERR;
	uint16 flags;
	clm_country_locales_t locale;
	chanvec_t saved_valid_channels, saved_db_valid_channels, unused;
#ifdef WL11N
	uint16 saved_locale_flags = 0,  saved_db_locale_flags = 0;
	bool saved_cap_40 = TRUE, saved_db_cap_40 = TRUE;
#endif /* WL11N */
#ifdef WL11AC
	bool saved_cap_80;
	bool saved_cap_160;
#endif /* WL11AC */

	/* Check if this is a valid band for this card */
	if ((NBANDS(wlc) == 1) &&
	    (BAND_5G(wlc->band->bandtype) == band2G))
		return;

	/* see if we need to look up country. Else, current locale */
	if (strcmp(abbrev, "")) {
		result = wlc_country_lookup(wlc, abbrev, &country);
		if (result != CLM_RESULT_OK) {
			WL_ERROR(("Invalid country \"%s\"\n", abbrev));
			return;
		}
		result = wlc_get_locale(country, &locale);

		flag_result = wlc_get_flags(&locale, band2G ? CLM_BAND_2G : CLM_BAND_5G, &flags);
		BCM_REFERENCE(flag_result);
	}

	/* Save current locales */
	if (result == CLM_RESULT_OK) {
		clm_band_t tmp_band = band2G ? CLM_BAND_2G : CLM_BAND_5G;
		bcopy(&wlc_cm->bandstate[wlc->band->bandunit].valid_channels,
			&saved_valid_channels, sizeof(chanvec_t));
		wlc_locale_get_channels(&locale, tmp_band,
			&wlc_cm->bandstate[wlc->band->bandunit].valid_channels, &unused);
		if (NBANDS(wlc) > 1) {
			bcopy(&wlc_cm->bandstate[OTHERBANDUNIT(wlc)].valid_channels,
			      &saved_db_valid_channels, sizeof(chanvec_t));
			wlc_locale_get_channels(&locale, tmp_band,
			      &wlc_cm->bandstate[OTHERBANDUNIT(wlc)].valid_channels, &unused);
		}
	}

#ifdef WL11N
	if (result == CLM_RESULT_OK) {
		saved_locale_flags = wlc_cm->bandstate[wlc->band->bandunit].locale_flags;
		wlc_cm->bandstate[wlc->band->bandunit].locale_flags = flags;

		if (NBANDS(wlc) > 1) {
			saved_db_locale_flags = wlc_cm->bandstate[OTHERBANDUNIT(wlc)].locale_flags;
			wlc_cm->bandstate[OTHERBANDUNIT(wlc)].locale_flags = flags;
		}
	}

	/* save 40 MHz cap */
	saved_cap_40 = WL_BW_CAP_40MHZ(wlc->band->bw_cap);
	wlc->band->bw_cap |= WLC_BW_40MHZ_BIT;
	if (NBANDS(wlc) > 1) {
		saved_db_cap_40 = WL_BW_CAP_40MHZ(wlc->bandstate[OTHERBANDUNIT(wlc)]->bw_cap);
		wlc->bandstate[OTHERBANDUNIT(wlc)]->bw_cap |= WLC_BW_40MHZ_BIT;
	}

#ifdef WL11AC
	/* save 80 MHz cap */
	saved_cap_80 = WL_BW_CAP_80MHZ(wlc->bandstate[BAND_5G_INDEX]->bw_cap);
	wlc->bandstate[BAND_5G_INDEX]->bw_cap |= WLC_BW_80MHZ_BIT;

	saved_cap_160 = WL_BW_CAP_160MHZ(wlc->bandstate[BAND_5G_INDEX]->bw_cap);
	wlc->bandstate[BAND_5G_INDEX]->bw_cap |= WLC_BW_160MHZ_BIT;

#endif /* WL11AC */

#endif /* WL11N */

#ifdef WL11ULB
	if (WLC_ULB_MODE_ENABLED(wlc)) {
		uint16 ulb_bw = WL_CHANSPEC_BW_20;
		uint16 band   = WL_CHANSPEC_BAND_2G;

		if (!band2G)
			band = WL_CHANSPEC_BAND_5G;

		if (bw == WL_CHANSPEC_BW_2P5 && WLC_2P5MHZ_ULB_SUPP_HW(wlc))
			ulb_bw = WL_CHANSPEC_BW_2P5;

		if (bw == WL_CHANSPEC_BW_5 && WLC_5MHZ_ULB_SUPP_HW(wlc))
			ulb_bw = WL_CHANSPEC_BW_5;

		if (bw == WL_CHANSPEC_BW_10 && WLC_10MHZ_ULB_SUPP_HW(wlc))
			ulb_bw = WL_CHANSPEC_BW_10;

		if (ulb_bw != WL_CHANSPEC_BW_20) {
			chanspec = (band | ulb_bw);
			wlc_chanspec_list(wlc, list, chanspec);
		}
	}
#endif /* WL11ULB */

	/* Go through 2G 20MHZ chanspecs */
	if (band2G && bw == WL_CHANSPEC_BW_20) {
		chanspec = WL_CHANSPEC_BAND_2G | WL_CHANSPEC_BW_20;
		wlc_chanspec_list(wlc, list, chanspec);
	}

	/* Go through 5G 20 MHZ chanspecs */
	if (!band2G && bw == WL_CHANSPEC_BW_20) {
		chanspec = WL_CHANSPEC_BAND_5G | WL_CHANSPEC_BW_20;
		wlc_chanspec_list(wlc, list, chanspec);
	}

#ifdef WL11N
	/* Go through 2G 40MHZ chanspecs only if N mode and PHY is capable of 40MHZ */
	if (band2G && bw == WL_CHANSPEC_BW_40 &&
	    N_ENAB(wlc->pub) && wlc->pub->phy_bw40_capable) {
		chanspec = WL_CHANSPEC_BAND_2G | WL_CHANSPEC_BW_40 | WL_CHANSPEC_CTL_SB_UPPER;
		wlc_chanspec_list(wlc, list, chanspec);
		chanspec = WL_CHANSPEC_BAND_2G | WL_CHANSPEC_BW_40 | WL_CHANSPEC_CTL_SB_LOWER;
		wlc_chanspec_list(wlc, list, chanspec);
	}

	/* Go through 5G 40MHZ chanspecs only if N mode and PHY is capable of 40MHZ  */
	if (!band2G && bw == WL_CHANSPEC_BW_40 &&
	    N_ENAB(wlc->pub) && ((NBANDS(wlc) > 1) || IS_SINGLEBAND_5G(wlc->deviceid)) &&
	    wlc->pub->phy_bw40_capable) {
		chanspec = WL_CHANSPEC_BAND_5G | WL_CHANSPEC_BW_40 | WL_CHANSPEC_CTL_SB_UPPER;
		wlc_chanspec_list(wlc, list, chanspec);
		chanspec = WL_CHANSPEC_BAND_5G | WL_CHANSPEC_BW_40 | WL_CHANSPEC_CTL_SB_LOWER;
		wlc_chanspec_list(wlc, list, chanspec);
	}

#ifdef WL11AC
	/* Go through 5G 80MHZ chanspecs only if VHT mode and PHY is capable of 80MHZ  */
	if (!band2G && bw == WL_CHANSPEC_BW_80 && VHT_ENAB_BAND(wlc->pub, WLC_BAND_5G) &&
		((NBANDS(wlc) > 1) || IS_SINGLEBAND_5G(wlc->deviceid)) &&
		wlc->pub->phy_bw80_capable) {

		int i;
		uint16 ctl_sb[] = {
			WL_CHANSPEC_CTL_SB_LL,
			WL_CHANSPEC_CTL_SB_LU,
			WL_CHANSPEC_CTL_SB_UL,
			WL_CHANSPEC_CTL_SB_UU
		};

		for (i = 0; i < 4; i++) {
			chanspec = WL_CHANSPEC_BAND_5G | WL_CHANSPEC_BW_80 | ctl_sb[i];
			wlc_chanspec_list(wlc, list, chanspec);
		}
	}

	 /* Go through 5G 8080MHZ chanspecs only if VHT mode and PHY is capable of 8080MHZ  */
	if (!band2G && bw == WL_CHANSPEC_BW_8080 && VHT_ENAB_BAND(wlc->pub, WLC_BAND_5G) &&
		((NBANDS(wlc) > 1) || IS_SINGLEBAND_5G(wlc->deviceid)) &&
		wlc->pub->phy_bw8080_capable) {

		int i, j;
		uint16 ctl_sb[] = {
			WL_CHANSPEC_CTL_SB_LLL,
			WL_CHANSPEC_CTL_SB_LLU,
			WL_CHANSPEC_CTL_SB_LUL,
			WL_CHANSPEC_CTL_SB_LUU,
		};

		/* List of all valid channel ID combinations */
		uint8 chan_id[] = {
			0x20,
			0x02,
			0x30,
			0x03,
			0x40,
			0x04,
			0x50,
			0x05,
			0x21,
			0x12,
			0x31,
			0x13,
			0x41,
			0x14,
			0x51,
			0x15,
			0x42,
			0x24,
			0x52,
			0x25,
			0x53,
			0x35,
			0x54,
			0x45
		};

		for (i = 0; i < sizeof(chan_id)/sizeof(uint8); i++) {
			for (j = 0; j < 4; j++) {
				chanspec = WL_CHANSPEC_BAND_5G | WL_CHANSPEC_BW_8080
					| ctl_sb[j]| chan_id[i];
				if (!wf_chspec_malformed(chanspec) &&
					wf_chspec_valid(chanspec) &&
					((NBANDS(wlc) > 1) ?
					wlc_valid_chanspec_db(wlc->cmi, chanspec) :
					wlc_valid_chanspec(wlc->cmi, chanspec))) {
					list->element[list->count] = chanspec;
					list->count++;
				}
			}
		}
	}

	/* Go through 5G 160MHZ chanspecs only if VHT mode and PHY is capable of 160MHZ  */
	if (!band2G && bw == WL_CHANSPEC_BW_160 && VHT_ENAB_BAND(wlc->pub, WLC_BAND_5G) &&
		((NBANDS(wlc) > 1) || IS_SINGLEBAND_5G(wlc->deviceid)) &&
		wlc->pub->phy_bw160_capable) {
		/* Valid 160 channels */
		uint8 chan[] = {50, 114};
		int i, j;
		uint16 ctl_sb[] = {
			WL_CHANSPEC_CTL_SB_LLL,
			WL_CHANSPEC_CTL_SB_LLU,
			WL_CHANSPEC_CTL_SB_LUL,
			WL_CHANSPEC_CTL_SB_LUU,
			WL_CHANSPEC_CTL_SB_ULL,
			WL_CHANSPEC_CTL_SB_ULU,
			WL_CHANSPEC_CTL_SB_UUL,
			WL_CHANSPEC_CTL_SB_UUU
		};

		for (i = 0; i < sizeof(chan)/sizeof(uint8); i++) {
			for (j = 0; j < 8; j++) {
				chanspec = WL_CHANSPEC_BAND_5G | WL_CHANSPEC_BW_160
					| ctl_sb[j]| chan[i];
				if (!wf_chspec_malformed(chanspec) &&
					((NBANDS(wlc) > 1) ?
					wlc_valid_chanspec_db(wlc->cmi, chanspec) :
					wlc_valid_chanspec(wlc->cmi, chanspec))) {
					list->element[list->count] = chanspec;
					list->count++;
				}
			}
		}
	}

	/* restore 80 MHz cap */
	if (saved_cap_80)
		wlc->bandstate[BAND_5G_INDEX]->bw_cap |= WLC_BW_80MHZ_BIT;
	else
		wlc->bandstate[BAND_5G_INDEX]->bw_cap &= ~WLC_BW_80MHZ_BIT;

	if (saved_cap_160)
		wlc->bandstate[BAND_5G_INDEX]->bw_cap |= WLC_BW_160MHZ_BIT;
	else
		wlc->bandstate[BAND_5G_INDEX]->bw_cap &= ~WLC_BW_160MHZ_BIT;

#endif /* WL11AC */

	/* restore 40 MHz cap */
	if (saved_cap_40)
		wlc->band->bw_cap |= WLC_BW_40MHZ_BIT;
	else
		wlc->band->bw_cap &= ~WLC_BW_40MHZ_BIT;

	if (NBANDS(wlc) > 1) {
		if (saved_db_cap_40) {
			wlc->bandstate[OTHERBANDUNIT(wlc)]->bw_cap |= WLC_BW_CAP_40MHZ;
		} else {
			wlc->bandstate[OTHERBANDUNIT(wlc)]->bw_cap &= ~WLC_BW_CAP_40MHZ;
		}
	}

	if (result == CLM_RESULT_OK) {
		wlc_cm->bandstate[wlc->band->bandunit].locale_flags = saved_locale_flags;
		if ((NBANDS(wlc) > 1))
			wlc_cm->bandstate[OTHERBANDUNIT(wlc)].locale_flags = saved_db_locale_flags;
	}
#endif /* WL11N */

	/* Restore the locales if switched */
	if (result == CLM_RESULT_OK) {
		bcopy(&saved_valid_channels,
			&wlc_cm->bandstate[wlc->band->bandunit].valid_channels,
			sizeof(chanvec_t));
		if ((NBANDS(wlc) > 1))
			bcopy(&saved_db_valid_channels,
			      &wlc_cm->bandstate[OTHERBANDUNIT(wlc)].valid_channels,
			      sizeof(chanvec_t));
	}
}

/* query the channel list given a country and a regulatory class */
/* XXX The result list 'list' must be large enough to hold all possible channels available
 * recommend to allocate MAXCHANNEL worth of elements
 */
uint8
wlc_rclass_get_channel_list(wlc_cm_info_t *cmi, const char *abbrev, uint8 rclass,
	bool bw20, wl_uint32_list_t *list)
{
	const rcinfo_t *rcinfo = NULL;
	uint8 ch2g_start = 0, ch2g_end = 0;
	int i;

	if (wlc_us_ccode(abbrev)) {
		if (rclass == WLC_REGCLASS_USA_2G_20MHZ) {
			ch2g_start = 1;
			ch2g_end = 11;
		}
#ifdef BAND5G
		else
			rcinfo = &rcinfo_us_20;
#endif // endif
	} else if (wlc_japan_ccode(abbrev)) {
		if (rclass == WLC_REGCLASS_JPN_2G_20MHZ) {
			ch2g_start = 1;
			ch2g_end = 13;
		}
		else if (rclass == WLC_REGCLASS_JPN_2G_20MHZ_CH14) {
			ch2g_start = 14;
			ch2g_end = 14;
		}
#ifdef BAND5G
		else
			rcinfo = &rcinfo_jp_20;
#endif // endif
	} else {
		if (rclass == WLC_REGCLASS_EUR_2G_20MHZ) {
			ch2g_start = 1;
			ch2g_end = 13;
		}
#ifdef BAND5G
		else
			rcinfo = &rcinfo_eu_20;
#endif // endif
	}

	list->count = 0;
	if (rcinfo == NULL) {
		for (i = ch2g_start; i <= ch2g_end; i ++)
			list->element[list->count ++] = i;
	}
	else {
		for (i = 0; i < rcinfo->len; i ++) {
			if (rclass == rcinfo->rctbl[i].rclass)
				list->element[list->count ++] = rcinfo->rctbl[i].chan;
		}
	}

	return (uint8)list->count;
}

/* Returns the chanspec for given country,rcalss and control channel */
chanspec_t
wlc_channel_rclass_get_chanspec(const char *abbrev, uint8 rclass, uint8 channel)
{
	uint bw = 0;

	/* Global/US/EU/JP and China Rclass is same for 80MHz,
	 * 160MHz, and 80p80
	 */
	if (rclass == 128) {
		bw = WL_CHANSPEC_BW_80;
	} else if (rclass == 129) {
		bw = WL_CHANSPEC_BW_160;
	} else if (rclass == 130) {
		bw = WL_CHANSPEC_BW_8080;
	} else if (wlc_us_ccode(abbrev)) {
		/* For Country code US
		 * rclass 22-33 are 40MHz rclass
		 */
		if (rclass >= 22 && rclass <= 33) {
			bw = WL_CHANSPEC_BW_40;
		} else {
			bw = WL_CHANSPEC_BW_20;
		}
	} else if (wlc_japan_ccode(abbrev)) {
		/* For country code japan
		 * rclass 36-57 are 40 MHz rclass
		 */
		if (rclass >= 36 && rclass <= 57) {
			bw = WL_CHANSPEC_BW_40;
		} else {
			bw = WL_CHANSPEC_BW_20;
		}
	} else {
		/* For country code EU
		 * rclass 5-12 are 40MHz rclass
		 */
		if (rclass >= 5 && rclass <= 12) {
			bw = WL_CHANSPEC_BW_40;
		} else {
			bw = WL_CHANSPEC_BW_20;
		}
	}
	return wf_channel2chspec(channel, bw);
}

/* Get channel width from chanspec */
static uint8
wlc_get_wb_chanwidth(chanspec_t chanspec)
{
	if (CHSPEC_IS8080(chanspec)) {
		return WIDE_BW_CHAN_WIDTH_80_80;
	} else if (CHSPEC_IS160(chanspec)) {
		return WIDE_BW_CHAN_WIDTH_160;
	} else if (CHSPEC_IS80(chanspec)) {
		return WIDE_BW_CHAN_WIDTH_80;
	} else if (CHSPEC_IS40(chanspec)) {
		return WIDE_BW_CHAN_WIDTH_40;
	} else {
		return WIDE_BW_CHAN_WIDTH_20;
	}
}

/* Get wideband channel width and center frequency from chanspec */
void
wlc_get_wbc_se_from_chanspec(dot11_wide_bw_chan_ie_t *wbc_ie,
	chanspec_t chanspec)
{
	if (wbc_ie == NULL) {
		return;
	}

	wbc_ie->channel_width = wlc_get_wb_chanwidth(chanspec);

	if (wbc_ie->channel_width == WIDE_BW_CHAN_WIDTH_80_80) {
		wbc_ie->center_frequency_segment_0 =
			wf_chspec_primary80_channel(chanspec);
		wbc_ie->center_frequency_segment_1 =
			wf_chspec_secondary80_channel(chanspec);
	} else {
		wbc_ie->center_frequency_segment_0 = CHSPEC_CHANNEL(chanspec);
		wbc_ie->center_frequency_segment_1 = 0;
	}
}

/* Return true if the channel is a valid channel that is radar sensitive
 * in the current country/locale
 */
bool
wlc_radar_chanspec(wlc_cm_info_t *wlc_cmi, chanspec_t chspec)
{
#ifdef BAND5G /* RADAR */
	uint channel = CHSPEC_CHANNEL(chspec);
	const chanvec_t *radar_channels;

	/* The radar_channels chanvec may be a superset of valid channels,
	 * so be sure to check for a valid channel first.
	 */

	if (!chspec || !wlc_valid_chanspec_db(wlc_cmi, chspec)) {
		return FALSE;
	}

	if (CHSPEC_IS5G(chspec)) {
		radar_channels = wlc_cmi->cm->bandstate[BAND_5G_INDEX].radar_channels;

		if (CHSPEC_IS8080(chspec)|| CHSPEC_IS160(chspec)) {
			int i;

			channel = wf_chspec_primary80_channel(chspec);
			/* start at the lower edge 20MHz channel */
			channel = LOWER_40_SB(channel); /* low 40MHz sb of the 80 */
			channel = LOWER_20_SB(channel); /* low 20MHz sb of the 40 */

			/* work through each 20MHz channel in the 80MHz */
			for (i = 0; i < 4; i++, channel += CH_20MHZ_APART) {
				if (isset(radar_channels->vec, channel)) {
					return TRUE;
				}
			}

			channel = wf_chspec_secondary80_channel(chspec);
			/* well-formed 80p80 channel should have valid secondary channel */
			ASSERT(channel > 0);

			/* start at the lower edge 20MHz channel */
			channel = LOWER_40_SB(channel); /* low 40MHz sb of the 80 */
			channel = LOWER_20_SB(channel); /* low 20MHz sb of the 40 */

			/* work through each 20MHz channel in the 80MHz */
			for (i = 0; i < 4; i++, channel += CH_20MHZ_APART) {
				if (isset(radar_channels->vec, channel)) {
					return TRUE;
				}
			}
		}
		else if (CHSPEC_IS80(chspec)) {
			int i;

			/* start at the lower edge 20MHz channel */
			channel = LOWER_40_SB(channel); /* low 40MHz sb of the 80 */
			channel = LOWER_20_SB(channel); /* low 20MHz sb of the 40 */

			/* work through each 20MHz channel in the 80MHz */
			for (i = 0; i < 4; i++, channel += CH_20MHZ_APART) {
				if (isset(radar_channels->vec, channel)) {
					return TRUE;
				}
			}
		} else if (CHSPEC_IS40(chspec)) {
			if (isset(radar_channels->vec, LOWER_20_SB(channel)) ||
			    isset(radar_channels->vec, UPPER_20_SB(channel)))
				return TRUE;
		} else if (isset(radar_channels->vec, channel)) {
			return TRUE;
		}
	}
#endif	/* BAND5G */
	return FALSE;
}

/* Return true if the channel is a valid channel that is radar sensitive
 * in the current country/locale
 */
bool
wlc_restricted_chanspec(wlc_cm_info_t *wlc_cmi, chanspec_t chspec)
{
	chanvec_t *restricted_channels;
	uint8 channel;

	/* The restriced_channels chanvec may be a superset of valid channels,
	 * so be sure to check for a valid channel first.
	 */

	if (!chspec || !wlc_valid_chanspec_db(wlc_cmi, chspec)) {
		return FALSE;
	}

	restricted_channels = &wlc_cmi->cm->restricted_channels;

	FOREACH_20_SB(chspec, channel) {
		if (isset(restricted_channels->vec, channel)) {
			return TRUE;
		}
	}

	return FALSE;
}

void
wlc_clr_restricted_chanspec(wlc_cm_info_t *wlc_cmi, chanspec_t chspec)
{
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	uint8 channel;

	FOREACH_20_SB(chspec, channel) {
		clrbit(wlc_cm->restricted_channels.vec, channel);
	}
	wlc_upd_restricted_chanspec_flag(wlc_cmi);
}

static void
wlc_upd_restricted_chanspec_flag(wlc_cm_info_t *wlc_cmi)
{
	uint j;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;

	for (j = 0; j < (int)sizeof(chanvec_t); j++)
		if (wlc_cm->restricted_channels.vec[j]) {
			wlc_cm->has_restricted_ch = TRUE;
			return;
		}

	wlc_cm->has_restricted_ch = FALSE;
}

bool
wlc_has_restricted_chanspec(wlc_cm_info_t *wlc_cmi)
{
	return wlc_cmi->cm->has_restricted_ch;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)

const bcm_bit_desc_t fc_flags[] = {
	{WLC_EIRP, "EIRP"},
	{WLC_DFS_TPC, "DFS/TPC"},
	{WLC_NO_80MHZ, "No 80MHz"},
	{WLC_NO_40MHZ, "No 40MHz"},
	{WLC_NO_MIMO, "No MIMO"},
	{WLC_RADAR_TYPE_EU, "EU_RADAR"},
	{WLC_FILT_WAR, "FILT_WAR"},
	{WLC_TXBF, "TxBF"},
	{0, NULL}
};

/* FTODO need to add 80mhz to this function */
int
wlc_channel_dump_locale(void *handle, struct bcmstrbuf *b)
{
	wlc_info_t *wlc = (wlc_info_t*)handle;
	wlc_cm_info_t* wlc_cmi = wlc->cmi;
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	ppr_t *txpwr;

	char max_ofdm_str[32];
	char max_ht20_str[32];
	char max_ht40_str[32];
	char max_cck_str[32];
	int chan, i;
	int restricted;
	int radar = 0;
	int max_cck, max_ofdm;
	int max_ht20 = 0, max_ht40 = 0;
	char flagstr[64];
	uint8 rclist[MAXRCLISTSIZE], rclen;
	chanspec_t chanspec;
	int quiet;
	ppr_dsss_rateset_t dsss_limits;
	ppr_ofdm_rateset_t ofdm_limits;
#ifdef WL11N
	ppr_ht_mcs_rateset_t mcs_limits;
#endif // endif

	clm_country_locales_t locales;
	clm_country_t country;

	clm_result_t result = wlc_country_lookup_direct(wlc_cm->ccode,
		wlc_cm->regrev, &country);

	if (result != CLM_RESULT_OK) {
		return -1;
	}

	if ((txpwr = ppr_create(wlc->pub->osh, ppr_get_max_bw())) == NULL) {
		return BCME_ERROR;
	}

	bcm_bprintf(b, "srom_ccode \"%s\" srom_regrev %u\n",
	            wlc_cm->srom_ccode, wlc_cm->srom_regrev);

	result = wlc_get_locale(country, &locales);
	if (result != CLM_RESULT_OK) {
		ppr_delete(wlc->pub->osh, txpwr);
		return -1;
	}

	if (NBANDS(wlc) > 1) {
		uint16 flags;
		wlc_get_flags(&locales, CLM_BAND_2G, &flags);
		bcm_format_flags(fc_flags, flags, flagstr, 64);
		bcm_bprintf(b, "2G Flags: %s\n", flagstr);
		wlc_get_flags(&locales, CLM_BAND_5G, &flags);
		bcm_format_flags(fc_flags, flags, flagstr, 64);
		bcm_bprintf(b, "5G Flags: %s\n", flagstr);
	} else {
		uint16 flags;
		if (BAND_2G(wlc->band->bandtype))
			wlc_get_flags(&locales, CLM_BAND_2G, &flags);
		else
			wlc_get_flags(&locales, CLM_BAND_5G, &flags);
		bcm_format_flags(fc_flags, flags, flagstr, 64);
		bcm_bprintf(b, "%dG Flags: %s\n", BAND_2G(wlc->band->bandtype)?2:5, flagstr);
	}

	if (N_ENAB(wlc->pub))
		bcm_bprintf(b, "  Ch Rdr/reS DSSS  OFDM   HT    20/40\n");
	else
		bcm_bprintf(b, "  Ch Rdr/reS DSSS  OFDM\n");

	for (chan = 0; chan < MAXCHANNEL; chan++) {
		chanspec = CH20MHZ_CHSPEC(chan);
		if (!wlc_valid_chanspec_db(wlc->cmi, chanspec)) {
			chanspec = CH40MHZ_CHSPEC(chan, WL_CHANSPEC_CTL_SB_LOWER);
			if (!wlc_valid_chanspec_db(wlc->cmi, chanspec)) {
				chanspec = CH80MHZ_CHSPEC(chan, WL_CHANSPEC_CTL_SB_LOWER);
				if (!wlc_valid_chanspec_db(wlc->cmi, chanspec))
					continue;
			}
		}

		radar = wlc_radar_chanspec(wlc->cmi, chanspec);
		restricted = wlc_restricted_chanspec(wlc->cmi, chanspec);
		quiet = wlc_quiet_chanspec(wlc->cmi, chanspec);

		wlc_channel_reg_limits(wlc->cmi, chanspec, txpwr);

		ppr_get_dsss(txpwr, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss_limits);
		max_cck = dsss_limits.pwr[0];

		ppr_get_ofdm(txpwr, WL_TX_BW_20, WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm_limits);
		max_ofdm = ofdm_limits.pwr[0];

#if defined(WL11N)
		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			if (WLCISHTPHY(wlc->band) && CHSPEC_IS40(chanspec))
				ppr_get_ht_mcs(txpwr, WL_TX_BW_20IN40, WL_TX_NSS_1, WL_TX_MODE_CDD,
					WL_TX_CHAINS_2, &mcs_limits);
			else
				ppr_get_ht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_CDD,
					WL_TX_CHAINS_2, &mcs_limits);
			max_ht20 = mcs_limits.pwr[0];

			ppr_get_ht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2, &mcs_limits);
			max_ht40 = mcs_limits.pwr[0];
		}
#endif /* WL11N */

		if (CHSPEC_IS2G(chanspec))
			if (max_cck != WL_RATE_DISABLED)
			snprintf(max_cck_str, sizeof(max_cck_str),
			         "%2d%s", QDB_FRAC(max_cck));
			else
				strncpy(max_cck_str, "-    ", sizeof(max_cck_str));

		else
			strncpy(max_cck_str, "     ", sizeof(max_cck_str));

		if (max_ofdm != WL_RATE_DISABLED)
			snprintf(max_ofdm_str, sizeof(max_ofdm_str),
		         "%2d%s", QDB_FRAC(max_ofdm));
		else
			strncpy(max_ofdm_str, "-    ", sizeof(max_ofdm_str));

		if (N_ENAB(wlc->pub)) {

			if (max_ht20 != WL_RATE_DISABLED)
				snprintf(max_ht20_str, sizeof(max_ht20_str),
			         "%2d%s", QDB_FRAC(max_ht20));
			else
				strncpy(max_ht20_str, "-    ", sizeof(max_ht20_str));

			if (max_ht40 != WL_RATE_DISABLED)
				snprintf(max_ht40_str, sizeof(max_ht40_str),
			         "%2d%s", QDB_FRAC(max_ht40));
			else
				strncpy(max_ht40_str, "-    ", sizeof(max_ht40_str));

			bcm_bprintf(b, "%s%3d %s%s%s     %s %s  %s %s\n",
			            (CHSPEC_IS40(chanspec)?">":" "), chan,
			            (radar ? "R" : "-"), (restricted ? "S" : "-"),
			            (quiet ? "Q" : "-"),
			            max_cck_str, max_ofdm_str,
			            max_ht20_str, max_ht40_str);
		}
		else
			bcm_bprintf(b, "%s%3d %s%s%s     %s %s\n",
			            (CHSPEC_IS40(chanspec)?">":" "), chan,
			            (radar ? "R" : "-"), (restricted ? "S" : "-"),
			            (quiet ? "Q" : "-"),
			            max_cck_str, max_ofdm_str);
	}

	bzero(rclist, MAXRCLISTSIZE);
	chanspec = wlc->pub->associated ?
	        wlc->home_chanspec : WLC_BAND_PI_RADIO_CHANSPEC;
	rclen = wlc_get_regclass_list(wlc->cmi, rclist, MAXRCLISTSIZE, chanspec, FALSE);
	if (rclen) {
		bcm_bprintf(b, "supported regulatory class:\n");
		for (i = 0; i < rclen; i++)
			bcm_bprintf(b, "%d ", rclist[i]);
		bcm_bprintf(b, "\n");
	}

	bcm_bprintf(b, "has_restricted_ch %s\n", wlc_cm->has_restricted_ch ? "TRUE" : "FALSE");

#if HTCONF
	{
	struct wlc_channel_txchain_limits *lim;

	lim = &wlc_cm->bandstate[BAND_2G_INDEX].chain_limits;
	bcm_bprintf(b, "chain limits 2g:");
	for (i = 0; i < WLC_CHAN_NUM_TXCHAIN; i++)
		bcm_bprintf(b, " %2d%s", QDB_FRAC(lim->chain_limit[i]));
	bcm_bprintf(b, "\n");

	lim = &wlc_cm->bandstate[BAND_5G_INDEX].chain_limits;
	bcm_bprintf(b, "chain limits 5g:");
	for (i = 0; i < WLC_CHAN_NUM_TXCHAIN; i++)
		bcm_bprintf(b, " %2d%s", QDB_FRAC(lim->chain_limit[i]));
	bcm_bprintf(b, "\n");
	}
#endif /* HTCONF */

	ppr_delete(wlc->pub->osh, txpwr);
	return 0;
}
#endif /* BCMDBG || BCMDBG_DUMP */

#ifndef INT8_MIN
#define INT8_MIN 0x80
#endif // endif
#ifndef INT8_MAX
#define INT8_MAX 0x7F
#endif // endif

static void
wlc_channel_margin_summary_mapfn(void *context, uint8 *a, uint8 *b)
{
	uint8 margin;
	uint8 *pmin = (uint8*)context;

	if (*a > *b)
		margin = *a - *b;
	else
		margin = 0;

	*pmin = MIN(*pmin, margin);
}

/* Map the given function with its context value over the power targets
 * appropriate for the given band and bandwidth in two txppr structs.
 * If the band is 2G, DSSS/CCK rates will be included.
 * If the bandwidth is 20MHz, only 20MHz targets are included.
 * If the bandwidth is 40MHz, both 40MHz and 20in40 targets are included.
 */
static void
wlc_channel_map_txppr_binary(ppr_mapfn_t fn, void* context, uint bandtype, uint bw,
	ppr_t *a, ppr_t *b, wlc_info_t *wlc)
{
	if (bw == WL_CHANSPEC_BW_20) {
		if (bandtype == WL_CHANSPEC_BAND_2G)
			ppr_map_vec_dsss(fn, context, a, b, WL_TX_BW_20, WL_TX_CHAINS_1);
		ppr_map_vec_ofdm(fn, context, a, b, WL_TX_BW_20, WL_TX_MODE_NONE, WL_TX_CHAINS_1);
	}

#ifdef WL11N
	/* map over 20MHz rates for 20MHz channels */
	if (bw == WL_CHANSPEC_BW_20) {
		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			if (bandtype == WL_CHANSPEC_BAND_2G) {
				ppr_map_vec_dsss(fn, context, a, b, WL_TX_BW_20, WL_TX_CHAINS_2);
				if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
					ppr_map_vec_dsss(fn, context, a, b,
						WL_TX_BW_20, WL_TX_CHAINS_3);
				}
			}
		}
		ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20, WL_TX_NSS_1,
			WL_TX_MODE_NONE, WL_TX_CHAINS_1);

		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_map_vec_ofdm(fn, context, a, b, WL_TX_BW_20, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2);
			ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20, WL_TX_NSS_1,
				WL_TX_MODE_CDD, WL_TX_CHAINS_2);
			ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20, WL_TX_NSS_2,
				WL_TX_MODE_STBC, WL_TX_CHAINS_2);
			ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20, WL_TX_NSS_2,
				WL_TX_MODE_NONE, WL_TX_CHAINS_2);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20, WL_TX_NSS_1,
					WL_TX_MODE_CDD, WL_TX_CHAINS_3);
				ppr_map_vec_ofdm(fn, context, a, b, WL_TX_BW_20, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3);

				ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_3);
				ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3);

				ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20, WL_TX_NSS_3,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20,
						WL_TX_NSS_1, WL_TX_MODE_CDD, WL_TX_CHAINS_4);
					ppr_map_vec_ofdm(fn, context, a, b, WL_TX_BW_20,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4);

					ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20,
						WL_TX_NSS_2, WL_TX_MODE_STBC, WL_TX_CHAINS_4);
					ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20,
						WL_TX_NSS_2, WL_TX_MODE_NONE, WL_TX_CHAINS_4);

					ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20,
						WL_TX_NSS_3, WL_TX_MODE_NONE, WL_TX_CHAINS_4);

					ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20,
						WL_TX_NSS_4, WL_TX_MODE_NONE, WL_TX_CHAINS_4);
				}
			}
		}
	} else
	/* map over 40MHz and 20in40 rates for 40MHz channels */
	{
		ppr_map_vec_ofdm(fn, context, a, b, WL_TX_BW_40, WL_TX_MODE_NONE, WL_TX_CHAINS_1);
		ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_40, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1);

		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_map_vec_ofdm(fn, context, a, b, WL_TX_BW_40, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2);
			ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_40, WL_TX_NSS_1,
				WL_TX_MODE_CDD, WL_TX_CHAINS_2);
			ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_40, WL_TX_NSS_2,
				WL_TX_MODE_STBC, WL_TX_CHAINS_2);
			ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_40, WL_TX_NSS_2,
				WL_TX_MODE_NONE, WL_TX_CHAINS_2);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_map_vec_ofdm(fn, context, a, b, WL_TX_BW_40, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3);
				ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_40, WL_TX_NSS_1,
					WL_TX_MODE_CDD, WL_TX_CHAINS_3);

				ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_40, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_3);
				ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_40, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3);

				ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_40, WL_TX_NSS_3,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_map_vec_ofdm(fn, context, a, b, WL_TX_BW_40,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4);
					ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_40,
						WL_TX_NSS_1, WL_TX_MODE_CDD, WL_TX_CHAINS_4);

					ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_40,
						WL_TX_NSS_2, WL_TX_MODE_STBC, WL_TX_CHAINS_4);
					ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_40,
						WL_TX_NSS_2, WL_TX_MODE_NONE, WL_TX_CHAINS_4);

					ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_40,
						WL_TX_NSS_3, WL_TX_MODE_NONE, WL_TX_CHAINS_4);

					ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_40,
						WL_TX_NSS_4, WL_TX_MODE_NONE, WL_TX_CHAINS_4);
				}
			}
		}
		/* 20in40 legacy */
		if (bandtype == WL_CHANSPEC_BAND_2G) {
			ppr_map_vec_dsss(fn, context, a, b, WL_TX_BW_20IN40, WL_TX_CHAINS_1);
			if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
				ppr_map_vec_dsss(fn, context, a, b, WL_TX_BW_20IN40,
					WL_TX_CHAINS_2);
				if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
					ppr_map_vec_dsss(fn, context, a, b, WL_TX_BW_20IN40,
						WL_TX_CHAINS_3);
					if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
						ppr_map_vec_dsss(fn, context, a, b, WL_TX_BW_20IN40,
							WL_TX_CHAINS_4);
					}
				}
			}
		}

		ppr_map_vec_ofdm(fn, context, a, b, WL_TX_BW_20IN40, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1);

		/* 20in40 HT */
		ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20IN40, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1);

		if (PHYCORENUM(wlc->stf->op_txstreams) > 1) {
			ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20IN40, WL_TX_NSS_1,
				WL_TX_MODE_CDD,	WL_TX_CHAINS_2);
			ppr_map_vec_ofdm(fn, context, a, b, WL_TX_BW_20IN40, WL_TX_MODE_CDD,
				WL_TX_CHAINS_2);
			ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20IN40, WL_TX_NSS_2,
				WL_TX_MODE_STBC, WL_TX_CHAINS_2);
			ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20IN40, WL_TX_NSS_2,
				WL_TX_MODE_NONE, WL_TX_CHAINS_2);

			if (PHYCORENUM(wlc->stf->op_txstreams) > 2) {
				ppr_map_vec_ofdm(fn, context, a, b, WL_TX_BW_20IN40, WL_TX_MODE_CDD,
					WL_TX_CHAINS_3);
				ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20IN40, WL_TX_NSS_1,
					WL_TX_MODE_CDD, WL_TX_CHAINS_3);

				ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20IN40, WL_TX_NSS_2,
					WL_TX_MODE_STBC, WL_TX_CHAINS_3);
				ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20IN40, WL_TX_NSS_2,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3);
				ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20IN40, WL_TX_NSS_3,
					WL_TX_MODE_NONE, WL_TX_CHAINS_3);

				if (PHYCORENUM(wlc->stf->op_txstreams) > 3) {
					ppr_map_vec_ofdm(fn, context, a, b, WL_TX_BW_20IN40,
						WL_TX_MODE_CDD, WL_TX_CHAINS_4);
					ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20IN40,
						WL_TX_NSS_1, WL_TX_MODE_CDD, WL_TX_CHAINS_4);

					ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20IN40,
						WL_TX_NSS_2, WL_TX_MODE_STBC, WL_TX_CHAINS_4);
					ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20IN40,
						WL_TX_NSS_2, WL_TX_MODE_NONE, WL_TX_CHAINS_4);

					ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20IN40,
						WL_TX_NSS_3, WL_TX_MODE_NONE, WL_TX_CHAINS_4);

					ppr_map_vec_ht_mcs(fn, context, a, b, WL_TX_BW_20IN40,
						WL_TX_NSS_4, WL_TX_MODE_NONE, WL_TX_CHAINS_4);
				}
			}
		}
	}
#endif /* WL11N */
}

/* calculate the offset from each per-rate power target in txpwr to the supplied
 * limit (or zero if txpwr[x] is less than limit[x]), and return the smallest
 * offset of relevant rates for bandtype/bw.
 */
static uint8
wlc_channel_txpwr_margin(ppr_t *txpwr, ppr_t *limit, uint bandtype, uint bw, wlc_info_t *wlc)
{
	uint8 margin = 0xff;

	wlc_channel_map_txppr_binary(wlc_channel_margin_summary_mapfn, &margin,
	                             bandtype, bw, txpwr, limit, wlc);
	return margin;
}

/* return a ppr_t struct with the phy srom limits for the given channel */
static void
wlc_channel_srom_limits(wlc_cm_info_t *wlc_cmi, chanspec_t chanspec,
	ppr_t *srommin, ppr_t *srommax)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	wlc_phy_t *pi = WLC_PI(wlc);
	uint8 min_srom;

	if (srommin != NULL)
		ppr_clear(srommin);
	if (srommax != NULL)
		ppr_clear(srommax);

	if (!PHYTYPE_HT_CAP(wlc_cmi->wlc->band))
		return;

	wlc_phy_txpower_sromlimit(pi, chanspec, &min_srom, srommax, 0);
	if (srommin != NULL)
		ppr_set_cmn_val(srommin, min_srom);
}

/* Set a per-chain power limit for the given band
 * Per-chain offsets will be used to make sure the max target power does not exceed
 * the per-chain power limit
 */
int
wlc_channel_band_chain_limit(wlc_cm_info_t *wlc_cmi, uint bandtype,
                             struct wlc_channel_txchain_limits *lim)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	ppr_t* txpwr;
	int bandunit = (bandtype == WLC_BAND_2G) ? BAND_2G_INDEX : BAND_5G_INDEX;

	if (!PHYTYPE_HT_CAP(wlc_cmi->wlc->band))
		return BCME_UNSUPPORTED;

	wlc_cmi->cm->bandstate[bandunit].chain_limits = *lim;

	if (CHSPEC_WLCBANDUNIT(wlc->chanspec) != bandunit)
		return 0;

#ifdef WLTXPWR_CACHE
		wlc_phy_txpwr_cache_invalidate(wlc_phy_get_txpwr_cache(WLC_PI(wlc)));
#endif // endif

	if ((txpwr = ppr_create(wlc_cmi->pub->osh, PPR_CHSPEC_BW(wlc->chanspec))) == NULL) {
		return 0;
	}

	/* update the current tx chain offset if we just updated this band's limits */
	wlc_channel_txpower_limits(wlc_cmi, txpwr);
	wlc_channel_update_txchain_offsets(wlc_cmi, txpwr);

	ppr_delete(wlc_cmi->pub->osh, txpwr);
	return 0;
}

/* update the per-chain tx power offset given the current power targets to implement
 * the correct per-chain tx power limit
 */
static int
wlc_channel_update_txchain_offsets(wlc_cm_info_t *wlc_cmi, ppr_t *txpwr)
{
	wlc_info_t *wlc = wlc_cmi->wlc;
	struct wlc_channel_txchain_limits *lim;
	wl_txchain_pwr_offsets_t offsets;
	chanspec_t chanspec;
	ppr_t* srompwr;
	int i, err;
	int max_pwr;
	int band, bw;
	int limits_present = FALSE;
	uint8 delta, margin, err_margin;
	wl_txchain_pwr_offsets_t cur_offsets;
#ifdef BCMDBG
	char chanbuf[CHANSPEC_STR_LEN];
#endif // endif
#if defined WLTXPWR_CACHE && defined(WLC_LOW) && defined(WL11N)
	tx_pwr_cache_entry_t* cacheptr = wlc_phy_get_txpwr_cache(WLC_PI(wlc));
#endif // endif

	if (!PHYTYPE_HT_CAP(wlc->band))
		return BCME_UNSUPPORTED;

	chanspec = wlc->chanspec;

#if defined WLTXPWR_CACHE && defined(WLC_LOW) && defined(WL11N)
	if ((wlc_phy_txpwr_cache_is_cached(cacheptr, chanspec) == TRUE) &&
		(wlc_phy_get_cached_txchain_offsets(cacheptr, chanspec, 0) != WL_RATE_DISABLED)) {

		for (i = 0; i < WLC_CHAN_NUM_TXCHAIN; i++) {
			offsets.offset[i] =
				wlc_phy_get_cached_txchain_offsets(cacheptr, chanspec, i);
		}

	/* always set, at least for the moment */
		err = wlc_stf_txchain_pwr_offset_set(wlc, &offsets);

		if (err) {
			WL_ERROR(("wl%d: txchain_pwr_offset failed: error %d\n",
				wlc->pub->unit, err));
		}

		return err;
	}
#endif /* WLTXPWR_CACHE && WLC_LOW && WL11N */

	band = CHSPEC_BAND(chanspec);
	bw = CHSPEC_BW(chanspec);

	if ((srompwr = ppr_create(wlc_cmi->pub->osh, PPR_CHSPEC_BW(chanspec))) == NULL) {
		return BCME_ERROR;
	}
	/* initialize the offsets to a default of no offset */
	memset(&offsets, 0, sizeof(wl_txchain_pwr_offsets_t));

	lim = &wlc_cmi->cm->bandstate[CHSPEC_WLCBANDUNIT(chanspec)].chain_limits;

	/* see if there are any chain limits specified */
	for (i = 0; i < WLC_CHAN_NUM_TXCHAIN; i++) {
		if (lim->chain_limit[i] < WLC_TXPWR_MAX) {
			limits_present = TRUE;
			break;
		}
	}

	/* if there are no limits, we do not need to do any calculations */
	if (limits_present) {

#ifdef WLTXPWR_CACHE
		ASSERT(txpwr != NULL);
#endif // endif
		/* find the max power target for this channel and impose
		 * a txpwr delta per chain to meet the specified chain limits
		 * Bound the delta by the tx power margin
		 */

		/* get the srom min powers */
		wlc_channel_srom_limits(wlc_cmi, wlc->chanspec, srompwr, NULL);

		/* find the dB margin we can use to adjust tx power */
		margin = wlc_channel_txpwr_margin(txpwr, srompwr, band, bw, wlc);

		/* reduce the margin by the error margin 1.5dB backoff */
		err_margin = 6;	/* 1.5 dB in qdBm */
		margin = (margin >= err_margin) ? margin - err_margin : 0;

		/* get the srom max powers */
		wlc_channel_srom_limits(wlc_cmi, wlc->chanspec, NULL, srompwr);

		/* combine the srom limits with the given regulatory limits
		 * to find the actual channel max
		 */
		/* wlc_channel_txpwr_vec_combine_min(srompwr, txpwr); */
		ppr_apply_vector_ceiling(srompwr, txpwr);

		/* max_pwr = (int)wlc_channel_txpwr_max(srompwr, band, bw, wlc); */
		max_pwr = (int)ppr_get_max_for_bw(srompwr, PPR_CHSPEC_BW(chanspec));
		WL_NONE(("wl%d: %s: channel %s max_pwr %d margin %d\n",
		         wlc->pub->unit, __FUNCTION__,
		         wf_chspec_ntoa(wlc->chanspec, chanbuf), max_pwr, margin));

		/* for each chain, calculate an offset that keeps the max tx power target
		 * no greater than the chain limit
		 */
		for (i = 0; i < WLC_CHAN_NUM_TXCHAIN; i++) {
			WL_NONE(("wl%d: %s: chain_limit[%d] %d",
			         wlc->pub->unit, __FUNCTION__,
			         i, lim->chain_limit[i]));
			if (lim->chain_limit[i] < max_pwr) {
				delta = max_pwr - lim->chain_limit[i];

				WL_NONE((" desired delta -%u lim delta -%u",
				         delta, MIN(delta, margin)));

				/* limit to the margin allowed for our adjustmets */
				delta = MIN(delta, margin);

				offsets.offset[i] = -delta;
			}
			WL_NONE(("\n"));
		}
	} else {
		WL_NONE(("wl%d: %s skipping limit calculation since limits are MAX\n",
		         wlc->pub->unit, __FUNCTION__));
	}

#if defined WLTXPWR_CACHE && defined(WLC_LOW) && defined(WL11N)
	if (wlc_phy_txpwr_cache_is_cached(cacheptr, chanspec) == TRUE) {
		for (i = 0; i < WLC_CHAN_NUM_TXCHAIN; i++) {
			wlc_phy_set_cached_txchain_offsets(cacheptr, chanspec, i,
				offsets.offset[i]);
		}
	}
#endif // endif
	err = wlc_iovar_op(wlc, "txchain_pwr_offset", NULL, 0,
	                   &cur_offsets, sizeof(wl_txchain_pwr_offsets_t), IOV_GET, NULL);

	if (!err && bcmp(&cur_offsets.offset, &offsets.offset, WL_NUM_TXCHAIN_MAX)) {

		err = wlc_iovar_op(wlc, "txchain_pwr_offset", NULL, 0,
			&offsets, sizeof(wl_txchain_pwr_offsets_t), IOV_SET, NULL);
	}

	if (err) {
		WL_ERROR(("wl%d: txchain_pwr_offset failed: error %d\n",
		          wlc->pub->unit, err));
	}

	if (srompwr != NULL)
		ppr_delete(wlc_cmi->pub->osh, srompwr);
	return err;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_channel_dump_reg_ppr(void *handle, struct bcmstrbuf *b)
{
	wlc_cm_info_t *wlc_cmi = (wlc_cm_info_t*)handle;
	wlc_info_t *wlc = wlc_cmi->wlc;
	ppr_t* txpwr;
	char chanbuf[CHANSPEC_STR_LEN];
	int ant_gain;
	int sar;

	wlcband_t* band = wlc->band;

	ant_gain = band->antgain;
	sar = band->sar;

	if ((txpwr = ppr_create(wlc_cmi->pub->osh, PPR_CHSPEC_BW(wlc->chanspec))) == NULL) {
		return BCME_ERROR;
	}
	wlc_channel_reg_limits(wlc_cmi, wlc->chanspec, txpwr);

	bcm_bprintf(b, "Regulatory Limits for channel %s (SAR:",
		wf_chspec_ntoa(wlc->chanspec, chanbuf));
	if (sar == WLC_TXPWR_MAX)
		bcm_bprintf(b, " -  ");
	else
		bcm_bprintf(b, "%2d%s", QDB_FRAC_TRUNC(sar));
	bcm_bprintf(b, " AntGain: %2d%s)\n", QDB_FRAC_TRUNC(ant_gain));
	wlc_channel_dump_txppr(b, txpwr, CHSPEC_TO_TX_BW(wlc->chanspec), wlc);

	ppr_delete(wlc_cmi->pub->osh, txpwr);
	return 0;
}

/* dump of regulatory power with local constraint factored in for the current channel */
static int
wlc_channel_dump_reg_local_ppr(void *handle, struct bcmstrbuf *b)
{
	wlc_cm_info_t *wlc_cmi = (wlc_cm_info_t*)handle;
	wlc_info_t *wlc = wlc_cmi->wlc;
	ppr_t* txpwr;
	char chanbuf[CHANSPEC_STR_LEN];

	if ((txpwr = ppr_create(wlc_cmi->pub->osh, PPR_CHSPEC_BW(wlc->chanspec))) == NULL) {
		return BCME_ERROR;
	}

	wlc_channel_txpower_limits(wlc_cmi, txpwr);

	bcm_bprintf(b, "Regulatory Limits with constraint for channel %s\n",
	            wf_chspec_ntoa(wlc->chanspec, chanbuf));
	wlc_channel_dump_txppr(b, txpwr, CHSPEC_TO_TX_BW(wlc->chanspec), wlc);

	ppr_delete(wlc_cmi->pub->osh, txpwr);
	return 0;
}

/* dump of srom per-rate max/min values for the current channel */
static int
wlc_channel_dump_srom_ppr(void *handle, struct bcmstrbuf *b)
{
	wlc_cm_info_t *wlc_cmi = (wlc_cm_info_t*)handle;
	wlc_info_t *wlc = wlc_cmi->wlc;
	ppr_t* srompwr;
	char chanbuf[CHANSPEC_STR_LEN];

	if ((srompwr = ppr_create(wlc_cmi->pub->osh, ppr_get_max_bw())) == NULL) {
		return BCME_ERROR;
	}

	wlc_channel_srom_limits(wlc_cmi, wlc->chanspec, NULL, srompwr);

	bcm_bprintf(b, "PHY/SROM Max Limits for channel %s\n",
	            wf_chspec_ntoa(wlc->chanspec, chanbuf));
	wlc_channel_dump_txppr(b, srompwr, CHSPEC_TO_TX_BW(wlc->chanspec), wlc);

	wlc_channel_srom_limits(wlc_cmi, wlc->chanspec, srompwr, NULL);

	bcm_bprintf(b, "PHY/SROM Min Limits for channel %s\n",
	            wf_chspec_ntoa(wlc->chanspec, chanbuf));
	wlc_channel_dump_txppr(b, srompwr, CHSPEC_TO_TX_BW(wlc->chanspec), wlc);

	ppr_delete(wlc_cmi->pub->osh, srompwr);
	return 0;
}

static void
wlc_channel_margin_calc_mapfn(void *ignore, uint8 *a, uint8 *b)
{
	if (*a > *b)
		*a = *a - *b;
	else
		*a = 0;
}

/* dumps dB margin between a rate an the lowest allowable power target, and
 * summarize the min of the margins for the current channel
 */
static int
wlc_channel_dump_margin(void *handle, struct bcmstrbuf *b)
{
	wlc_cm_info_t *wlc_cmi = (wlc_cm_info_t*)handle;
	wlc_info_t *wlc = wlc_cmi->wlc;
	ppr_t* txpwr;
	ppr_t* srommin;
	chanspec_t chanspec;
	int band, bw;
	uint8 margin;
	char chanbuf[CHANSPEC_STR_LEN];

	chanspec = wlc->chanspec;
	band = CHSPEC_BAND(chanspec);
	bw = CHSPEC_BW(chanspec);

	if ((txpwr = ppr_create(wlc_cmi->pub->osh, PPR_CHSPEC_BW(chanspec))) == NULL) {
		return 0;
	}
	if ((srommin = ppr_create(wlc_cmi->pub->osh, PPR_CHSPEC_BW(chanspec))) == NULL) {
		ppr_delete(wlc_cmi->pub->osh, txpwr);
		return 0;
	}

	wlc_channel_txpower_limits(wlc_cmi, txpwr);

	/* get the srom min powers */
	wlc_channel_srom_limits(wlc_cmi, wlc->chanspec, srommin, NULL);

	/* find the dB margin we can use to adjust tx power */
	margin = wlc_channel_txpwr_margin(txpwr, srommin, band, bw, wlc);

	/* calulate the per-rate margins */
	wlc_channel_map_txppr_binary(wlc_channel_margin_calc_mapfn, NULL,
	                             band, bw, txpwr, srommin, wlc);

	bcm_bprintf(b, "Power margin for channel %s, min = %u\n",
	            wf_chspec_ntoa(wlc->chanspec, chanbuf), margin);
	wlc_channel_dump_txppr(b, txpwr, CHSPEC_TO_TX_BW(wlc->chanspec), wlc);

	ppr_delete(wlc_cmi->pub->osh, srommin);
	ppr_delete(wlc_cmi->pub->osh, txpwr);
	return 0;
}

static int
wlc_channel_supported_country_regrevs(void *handle, struct bcmstrbuf *b)
{
	int iter;
	ccode_t cc;
	unsigned int regrev;

	if (clm_iter_init(&iter) == CLM_RESULT_OK) {
		while (clm_country_iter((clm_country_t*)&iter, cc, &regrev) == CLM_RESULT_OK) {
			bcm_bprintf(b, "%c%c/%u\n", cc[0], cc[1], regrev);

		}
	}
	return 0;
}

#endif /* BCMDBG || BCMDBG_DUMP */

void
wlc_dump_clmver(wlc_cm_info_t *wlc_cmi, struct bcmstrbuf *b)
{
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	const struct clm_data_header* clm_base_headerptr = wlc_cm->clm_base_dataptr;
	const char* verstrptr;
	const char* useridstrptr;
#ifdef WLCLMINC
	struct clm_data_header* clm_inc_headerptr = wlc_cm->clm_inc_headerptr;
#endif // endif

	if (clm_base_headerptr == NULL)
		clm_base_headerptr = &clm_header;

	bcm_bprintf(b, "API: %d.%d\nData: %s\nCompiler: %s\n%s\n",
		clm_base_headerptr->format_major, clm_base_headerptr->format_minor,
		clm_base_headerptr->clm_version, clm_base_headerptr->compiler_version,
		clm_base_headerptr->generator_version);
	verstrptr = clm_get_base_app_version_string();
	if (verstrptr != NULL)
		bcm_bprintf(b, "Customization: %s\n", verstrptr);
	useridstrptr = clm_get_string(CLM_STRING_TYPE_USER_STRING, CLM_STRING_SOURCE_BASE);
	if (useridstrptr != NULL)
		bcm_bprintf(b, "Creation: %s\n", useridstrptr);

#ifdef WLCLMINC
	if (clm_inc_headerptr != NULL) {
		bcm_bprintf(b, "Inc Data: %s\nInc Compiler: %s\nInc %s\n",
			clm_inc_headerptr->clm_version,
			clm_inc_headerptr->compiler_version, clm_inc_headerptr->generator_version);
		verstrptr = clm_get_inc_app_version_string();
		if (verstrptr != NULL)
			bcm_bprintf(b, "Inc Customization: %s\n", verstrptr);
		useridstrptr = clm_get_string(CLM_STRING_TYPE_USER_STRING,
			CLM_STRING_SOURCE_INCREMENTAL);
		if (useridstrptr != NULL)
			bcm_bprintf(b, "Creation: %s\n", useridstrptr);
	}
#endif /* WLCLMINC */
}

void wlc_channel_update_txpwr_limit(wlc_info_t *wlc)
{
	ppr_t *txpwr;
	if ((txpwr = ppr_create(wlc->osh, PPR_CHSPEC_BW(wlc->chanspec))) == NULL) {
		return;
	}
	wlc_channel_reg_limits(wlc->cmi, wlc->chanspec, txpwr);
	wlc_phy_txpower_limit_set(WLC_PI(wlc), txpwr, wlc->chanspec);
	ppr_delete(wlc->osh, txpwr);
}

#ifdef WLTDLS
/* Regulatory Class IE in TDLS Setup frames */
static uint
wlc_channel_tdls_calc_rc_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_cm_info_t *cmi = (wlc_cm_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = calc->cbparm->ft;
	chanspec_t chanspec = ftcbparm->tdls.chspec;
	uint8 rclen;		/* regulatory class length */
	uint8 rclist[32];	/* regulatory class list */

	if (!isset(ftcbparm->tdls.cap, DOT11_TDLS_CAP_CH_SW))
		return 0;

	/* XXX need to resolve if AP & STA req to support
	 * regulatory class IE in beacon/probe so that
	 * associated STA in a BSS able to make a join
	 * decision
	 */
	rclen = wlc_get_regclass_list(cmi, rclist, MAXRCLISTSIZE, chanspec, TRUE);

	return TLV_HDR_LEN + rclen;
}

static int
wlc_channel_tdls_write_rc_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_cm_info_t *cmi = (wlc_cm_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;
	chanspec_t chanspec = ftcbparm->tdls.chspec;
	uint8 rclen;		/* regulatory class length */
	uint8 rclist[32];	/* regulatory class list */

	if (!isset(ftcbparm->tdls.cap, DOT11_TDLS_CAP_CH_SW)) {
		return BCME_OK;
	}

	/* XXX need to resolve if AP & STA req to support
	 * regulatory class IE in beacon/probe so that
	 * associated STA in a BSS able to make a join
	 * decision
	 */
	rclen = wlc_get_regclass_list(cmi, rclist, MAXRCLISTSIZE, chanspec, TRUE);

	bcm_write_tlv_safe(DOT11_MNG_REGCLASS_ID, rclist, rclen,
		build->buf, build->buf_len);

	return BCME_OK;
}
#endif /* WLTDLS */

void
wlc_channel_tx_power_target_min_max(struct wlc_info *wlc,
	chanspec_t chanspec, int *min_pwr, int *max_pwr)
{
	int cur_min = 0xFFFF, cur_max = 0;
	wlc_phy_t *pi = WLC_PI(wlc);
	ppr_t *txpwr;
	ppr_t *srommax;
	int8 min_srom;
	chanspec_t channel = wf_chspec_ctlchan(chanspec) |
		CHSPEC_BAND(chanspec) | WL_CHANSPEC_BW_20;

	if ((txpwr = ppr_create(wlc->osh, PPR_CHSPEC_BW(channel))) == NULL) {
		return;
	}
	if ((srommax = ppr_create(wlc->osh, PPR_CHSPEC_BW(channel))) == NULL) {
		ppr_delete(wlc->osh, txpwr);
		return;
	}
	/* use the control channel to get the regulatory limits and srom max/min */
	wlc_channel_reg_limits(wlc->cmi, channel, txpwr);

	wlc_phy_txpower_sromlimit(pi, channel, (uint8*)&min_srom, srommax, 0);

	/* bound the regulatory limit by srom min/max */
	ppr_apply_vector_ceiling(txpwr, srommax);
	ppr_apply_min(txpwr, min_srom);

	WL_NONE(("min_srom %d\n", min_srom));

	cur_min = ppr_get_min(txpwr, min_srom);
	cur_max = ppr_get_max(txpwr);

	ppr_delete(wlc->osh, srommax);
	ppr_delete(wlc->osh, txpwr);

	*min_pwr = (int)cur_min;
	*max_pwr = (int)cur_max;
}

int8
wlc_channel_max_tx_power_for_band(struct wlc_info *wlc, int band)
{
	int chan;
	chanspec_t chspec;
	int chspec_band;
	int8 max = 0;
	int chan_min;
	int chan_max = 0;
	wlc_cm_info_t *cmi = wlc->cmi;

	chspec_band = (band == WLC_BAND_2G) ? WL_CHANSPEC_BAND_2G : WL_CHANSPEC_BAND_5G;

	for (chan = 0; chan < MAXCHANNEL; chan++) {
		chspec = chan | WL_CHANSPEC_BW_20 | chspec_band;

		if (!wlc_valid_chanspec_db(cmi, chspec))
			continue;

		wlc_channel_tx_power_target_min_max(wlc, chspec, &chan_min, &chan_max);

		max = MAX(chan_max, max);
	}
	return max;
}

void
wlc_channel_set_tx_power(struct wlc_info *wlc,
	int band, int num_chains, int *txcpd_power_offset, int *tx_chain_offset)
{
	int i;
	int8 band_max;
	int8 offset;
	struct wlc_channel_txchain_limits lim;
	wlc_cm_info_t *cmi = wlc->cmi;
	bool offset_diff = FALSE;

	/* init limits to max value for int8 to not impose any limit */
	memset(&lim, 0x7f, sizeof(struct wlc_channel_txchain_limits));

	/* find max target power for the band */
	band_max = wlc_channel_max_tx_power_for_band(wlc, band);

	for (i = 0; i < WLC_CHAN_NUM_TXCHAIN; i++) {
		if (i < num_chains)
			offset =
			(int8) (*(txcpd_power_offset + i) * WLC_TXPWR_DB_FACTOR);
		else
			offset = 0;

		if (offset != 0)
			lim.chain_limit[i] = band_max + offset;

		/* check if the new offsets are equal to the previous offsets */
		if (*(tx_chain_offset + i) != *(txcpd_power_offset + i))
			offset_diff = TRUE;
	}

	if (offset_diff == TRUE) {
		WL_NONE(("%s txcpd_power_offset %d %d %d %d\n", __FUNCTION__,
			txcpd_power_offset[0],
			txcpd_power_offset[1],
			txcpd_power_offset[2],
			txcpd_power_offset[3]));
		wlc_channel_band_chain_limit(cmi, band, &lim);
	}
}

#ifdef WL_AP_CHAN_CHANGE_EVENT
void
wlc_channel_send_chan_event(wlc_info_t *wlc, wl_chan_change_reason_t reason,
	chanspec_t chanspec)
{
	wl_event_change_chan_t evt_data;
	ASSERT(wlc != NULL);

	memset(&evt_data, 0, sizeof(evt_data));

	evt_data.version = WL_CHAN_CHANGE_EVENT_VER_1;
	evt_data.length = WL_CHAN_CHANGE_EVENT_LEN_VER_1;
	evt_data.target_chanspec = chanspec;
	evt_data.reason = reason;

	wlc_bss_mac_event(wlc, wlc->cfg, WLC_E_AP_CHAN_CHANGE, NULL, 0,
		0, 0, (void *)&evt_data, sizeof(evt_data));
	WL_TRACE(("wl%d: CHAN Change event to ch 0x%02x\n", WLCWLUNIT(wlc),
		evt_data.target_chanspec));
}
#endif /* WL_CHAN_CHANGE_EVENT */

#ifdef WL_GLOBAL_RCLASS
/* check client's 2g/5g band and global operating support capability */
uint8
wlc_sta_supports_global_rclass(uint8 *rclass_bitmap)
{
	uint8 global_opclass_support = 0;

	/* MAXRCLIST = 3 for 20, 40 lower and 40 upper RCINFO
	 * MACRCLIST_REG = 2 for 80 and 160 RCINFO
	 */
	const rcinfo_t *rcinfo[MAXRCLIST + MAXRCLIST_REG] = {NULL};

	/* As of now check for Global opclass support, if present update
	 * and return.
	 * TODO: check for country specific opclass if present and update
	 * country specific opclass info, may be useful later
	 */
	/* check for 2G band */
	if (isset(rclass_bitmap, WLC_REGCLASS_GLOBAL_2G_20MHZ) ||
		isset(rclass_bitmap, WLC_REGCLASS_GLOBAL_2G_20MHZ_CH14)) {

		global_opclass_support |= (0X01 << WLC_BAND_2G);
	}
#ifdef BAND5G
#ifdef WL11AC_160
	rcinfo[0] =  &rcinfo_global_center_160;
#endif /* WL11AC_160 */
	rcinfo[1] =  &rcinfo_global_center_80;
	rcinfo[2] =  &rcinfo_global_40upper;
	rcinfo[3] =  &rcinfo_global_40lower;
	rcinfo[4] =  &rcinfo_global_20;

	if ((wlc_chk_rclass_support_5g(rcinfo[0], rclass_bitmap)) ||
		(wlc_chk_rclass_support_5g(rcinfo[1], rclass_bitmap)) ||
		(wlc_chk_rclass_support_5g(rcinfo[2], rclass_bitmap)) ||
		(wlc_chk_rclass_support_5g(rcinfo[3], rclass_bitmap)) ||
		(wlc_chk_rclass_support_5g(rcinfo[4], rclass_bitmap))) {

		global_opclass_support |= (0X01 << WLC_BAND_5G);
	}
	return global_opclass_support;
#endif /* BAND5G */
}

/* query the channel list given a country and a regulatory class */
/* XXX The result list 'list' must be large enough to hold all possible channels available
 * recommend to allocate MAXCHANNEL worth of elements
 */
static bool
wlc_chk_rclass_support_5g(const rcinfo_t *rcinfo, uint8 *setBitBuff)
{
	int count = 0;
	int i = 0;
	bool rclass_support = FALSE;

	/* check for 5G band */
	count = rcinfo->len;
	for (i = 0; i < count; i++) {
		if (isset(setBitBuff, rcinfo->rctbl[i].rclass)) {
			rclass_support = TRUE;
			break;
		}
	}
	return rclass_support;
}

int
wlc_channel_get_cur_rclass(struct wlc_info *wlc)
{
	return wlc->cmi->cm->cur_rclass;
}

void
wlc_channel_set_cur_rclass(struct wlc_info *wlc, uint8 cur_rclass)
{
	wlc->cmi->cm->cur_rclass = cur_rclass;
}
#endif /* WL_GLOBAL_RCLASS */

void
wlc_update_rcinfo(wlc_cm_info_t *wlc_cmi, bool switch_to_global_opclass)
{
	wlc_cm_data_t *wlc_cm = wlc_cmi->cm;
	wlc_pub_t *pub = wlc_cmi->pub;
	BCM_REFERENCE(pub);

	if (switch_to_global_opclass) {
#ifdef BAND5G
		wlc_cm->rcinfo_list[WLC_RCLIST_20] = &rcinfo_global_20;
#endif // endif
#ifdef WL11N
		if (N_ENAB(pub)) {
			wlc_cm->rcinfo_list[WLC_RCLIST_40L] = &rcinfo_global_40lower;
			wlc_cm->rcinfo_list[WLC_RCLIST_40U] = &rcinfo_global_40upper;
		}
#endif // endif
#ifdef WL11AC
		if (VHT_ENAB(pub)) {
			wlc_cm->rcinfo_list_11ac[WLC_RCLIST_80] = &rcinfo_global_center_80;
#ifdef WL11AC_160
			wlc_cm->rcinfo_list_11ac[WLC_RCLIST_160] = &rcinfo_global_center_160;
#endif // endif
		}
#endif /* WL11AC */
	} else if (wlc_us_ccode(wlc_cm->country_abbrev)) {
#ifdef BAND5G
		wlc_cm->rcinfo_list[WLC_RCLIST_20] = &rcinfo_us_20;
#endif // endif
#ifdef WL11N
		if (N_ENAB(pub)) {
			wlc_cm->rcinfo_list[WLC_RCLIST_40L] = &rcinfo_us_40lower;
			wlc_cm->rcinfo_list[WLC_RCLIST_40U] = &rcinfo_us_40upper;
		}
#endif // endif
#ifdef WL11AC
		if (VHT_ENAB(pub)) {
			wlc_cm->rcinfo_list_11ac[WLC_RCLIST_80] = &rcinfo_us_center_80;
#ifdef WL11AC_160
			wlc_cm->rcinfo_list_11ac[WLC_RCLIST_160] = &rcinfo_us_center_160;
#endif // endif
		}
#endif /* WL11AC */
	} else if (wlc_japan_ccode(wlc_cm->country_abbrev)) {
#ifdef BAND5G
		wlc_cm->rcinfo_list[WLC_RCLIST_20] = &rcinfo_jp_20;
#endif // endif
#ifdef WL11N
		if (N_ENAB(pub)) {
			wlc_cm->rcinfo_list[WLC_RCLIST_40L] = &rcinfo_jp_40;
			wlc_cm->rcinfo_list[WLC_RCLIST_40U] = &rcinfo_jp_40;
		}
#endif // endif
#ifdef WL11AC
		if (VHT_ENAB(pub)) {
			wlc_cm->rcinfo_list_11ac[WLC_RCLIST_80] = &rcinfo_jp_center_80;
#ifdef WL11AC_160
			wlc_cm->rcinfo_list_11ac[WLC_RCLIST_160] = &rcinfo_jp_center_160;
#endif // endif
		}
#endif /* WL11AC */
	} else {
#ifdef BAND5G
		wlc_cm->rcinfo_list[WLC_RCLIST_20] = &rcinfo_eu_20;
#endif // endif
#ifdef WL11N
		if (N_ENAB(pub)) {
			wlc_cm->rcinfo_list[WLC_RCLIST_40L] = &rcinfo_eu_40lower;
			wlc_cm->rcinfo_list[WLC_RCLIST_40U] = &rcinfo_eu_40upper;
		}
#endif // endif
#ifdef WL11AC
		if (VHT_ENAB(pub)) {
			wlc_cm->rcinfo_list_11ac[WLC_RCLIST_80] = &rcinfo_eu_center_80;
#ifdef WL11AC_160
			wlc_cm->rcinfo_list_11ac[WLC_RCLIST_160] = &rcinfo_eu_center_160;
#endif // endif
		}
#endif /* WL11AC */
	}
	wlc_regclass_vec_init(wlc_cmi);
}

/* stating from LSB of each octet in bmp of length len, get the position of the n th valid bit
 *	bmp	- bitmap array of octets
 *	len	- length (number of octets in bmp)
 *	n	- n th valid bit's position is required (n must be between 1 and len * 8)
 * Returns the position of the n th valid bit, stating from LSB of each octet in bmp of length len,
 *	returns -1 if not found or on error
 */
static int32 wlc_channel_get_nth_valid_bit_position(uint8 *bmp, int32 len, int32 n)
{
	int32 i = 0;

	if (bmp == NULL || n < 1 || n > len * NBBY) {
		return -1;
	}
	while (i < len * NBBY && n > 0) {
		if (isset(bmp, i)) {
			if (--n == 0) return i;
		}
		i++;
	}

	return -1;
}

/* Returns a valid random DFS/Non-DFS channel or 0 on error */
chanspec_t
wlc_channel_5gchanspec_rand(wlc_info_t *wlc, bool radar_detected)
{
	chanvec_t ch_vec;
	chanspec_t def_chspec = wlc->default_bss->chanspec, chspec, first_valid, rand_chspec;
	uint16 def_sb = CHSPEC_CTL_SB(def_chspec), def_bw = CHSPEC_BW(def_chspec),
	       bw_full, bw_sub, ch1_count, ch1, bw_idx;
#if defined(WL11AC_80P80)
	uint16 ch2_count, ch2;
#endif /* WL11AC_80P80 */
	uint32 rand_tsf = wlc->clk ? R_REG(wlc->osh, &wlc->regs->u.d11regs.tsf_random) : 0;
	int16 rand_idx;
#if defined(BCMDBG)
	char chanbuf[CHANSPEC_STR_LEN];
#endif /* BCMDBG */
	/* list of bandwidth in decreasing order of magnitude */
	uint16 bw_list[] = { WL_CHANSPEC_BW_160, WL_CHANSPEC_BW_80,
			WL_CHANSPEC_BW_40, WL_CHANSPEC_BW_20 };
	/* length of the list of bandwidth */
	uint16 bw_list_len = sizeof(bw_list) / sizeof(bw_list[0]);

	first_valid = rand_chspec = 0;
	/* get to the first acceptable bw */
	for (bw_idx = 0; bw_idx < bw_list_len && bw_list[bw_idx] != def_bw; bw_idx++) /* NO OP */;

	if (bw_idx == bw_list_len) {
		WL_REGULATORY(("radar: %d, def_chspec: 0x%04x, bw_list[%d]: 0x%x, def_bw: 0x%x\n",
				radar_detected, def_chspec, bw_idx,
				(bw_idx < bw_list_len ? bw_list[bw_idx] : -1),
				def_bw));
		return 0;
	}

	while (bw_idx < bw_list_len && rand_chspec == 0) {
		bw_full = bw_sub = bw_list[bw_idx++];
		if (!wlc_is_valid_bw(wlc, wlc->cfg, BAND_5G_INDEX, bw_full)) {
			continue; /* skip unsupported bandwidth */
		}
#if defined(WL11AC_80P80)
		if (bw_full == WL_CHANSPEC_BW_8080) {
			/* for 80p80, look for two 80MHz channels */
			bw_sub = WL_CHANSPEC_BW_80;
		}
#endif /* WL11AC_80P80 */
		memset(&ch_vec, 0, sizeof(ch_vec));
		ch1_count = 0;
		for (ch1 = CH_MIN_5G_CHANNEL; ch1 < MAXCHANNEL; ch1++) {
			chspec = CHBW_CHSPEC(bw_sub, ch1);
			if (!wlc_valid_chanspec_db(wlc->cmi, chspec) ||
				wlc_restricted_chanspec(wlc->cmi, chspec) ||
				!wlc_valid_dfs_chanspec(wlc, chspec)) {
				continue; /* skip invalid chspec */
			}
			if (first_valid == 0) {
				first_valid = chspec; /* valid but still CAC might be required */
			}
			/* just detected radar? Avoid DFS ch unless (pre)cleared eg. in EDCRS_EU */
			if (radar_detected && wlc_quiet_chanspec(wlc->cmi, chspec)) {
				continue; /* CAC is required; skip unavailable DFS radar channels */
			}
			setbit(ch_vec.vec, ch1);
			ch1_count++;
		}
		if (ch1_count == 0) {
			continue; /* no valid channels of given bandwidth found */
		}
		rand_idx = rand_tsf % ch1_count;
#if defined(WL11AC_80P80)
		if (bw_full == WL_CHANSPEC_BW_8080) {
			if (ch1_count < 2) {
				continue; /* at least two 80MHz ch are required for 80p80 */
			}
			/* reduce rand_idx to make way for ch2; if count is 2, just use both 80 */
			rand_idx = ch1_count == 2 ? 0 : (rand_idx > 1 ? rand_idx -1 : rand_idx);
		}
		ch2_count = ch1_count - rand_idx -1; /* count of valid channels left for ch2 */
#endif /* WL11AC_80P80 */
		/* choose 'rand_idx'th channel */
		ch1 = (uint8) (WL_CHANSPEC_CHAN_MASK & wlc_channel_get_nth_valid_bit_position(
					ch_vec.vec, sizeof(ch_vec.vec), rand_idx + 1));
		if (ch1 == 0 || ch1 == MAXCHANNEL) {
			continue; /* failed to find an usable channel */
		}
		rand_chspec = CHBW_CHSPEC(bw_full, ch1);
#if defined(WL11AC_80P80)
		if (bw_full == WL_CHANSPEC_BW_8080) {
			uint8 pri20 = ch1 - CH_20MHZ_APART - CH_10MHZ_APART; /* zeroth side band */
			rand_idx += rand_tsf % ch2_count;
			ch2 = (uint8) (WL_CHANSPEC_CHAN_MASK &
				wlc_channel_get_nth_valid_bit_position(ch_vec.vec,
					sizeof(ch_vec.vec), rand_idx + 1));
			if (ch2 == 0 || ch2 == MAXCHANNEL) {
				continue; /* second valid channel of 80MHz bw not found */
			}
			rand_chspec = wf_chspec_get8080_chspec(pri20, ch1, ch2);
		}
#endif /* WL11AC_80P80 */

		if (bw_full == def_bw) { /* if the bandwidth matches default BSS channel's */
			rand_chspec |= def_sb; /* we can use the default BSS sideband */
		}
	}

	if (rand_chspec == 0 && first_valid != 0) {
		rand_chspec = first_valid;
	}

	if (rand_chspec == 0) {
		return 0;
	}

#if defined(BCMDBG)
	WL_REGULATORY(("wl%d: %s: dfs selected random chanspec %s (%04x)\n", wlc->pub->unit,
			__FUNCTION__, wf_chspec_ntoa(rand_chspec, chanbuf), rand_chspec));
#endif // endif

	ASSERT(wlc_valid_chanspec_db(wlc->cmi, rand_chspec));

	return rand_chspec;
}

/*
 * Return a valid 2g chanspec of current BW. If none are found, returns 0.
 */
chanspec_t
wlc_channel_next_2gchanspec(wlc_cm_info_t *wlc_cmi, chanspec_t cur_chanspec)
{
	chanspec_t chspec;
	uint16 chan;
	uint16 cur_bw = CHSPEC_BW_GE(cur_chanspec, WL_CHANSPEC_BW_80) ? WL_CHANSPEC_BW_40 :
			CHSPEC_BW(cur_chanspec);

	for (chan = 1; chan <= CH_MAX_2G_CHANNEL; chan++) {
		chspec = CHBW_CHSPEC(cur_bw, chan);
		if (wlc_valid_chanspec(wlc_cmi, chspec)) {
			return chspec;
		}
	}
	return 0;
}
