/*
 * wlc_bs_data.c
 *
 * This module implements the Band Steering Daemon "bs_data" IOVAR functionality.
 *
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
 * $Id$
 */

/**
 * @file
 * @brief
 * Mechanism and feasibility for an AP to "band steer" an associated STA to another band.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [BandSteering]
 */

/*
 * Include files.
 */
#include <wlc_cfg.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <d11.h>
#include <wlioctl.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_bs_data.h>

const char module_name[] = "bs_data";

/* The module context structure. Not available to the outside world. */
struct wlc_bs_data_info {
	wlc_info_t	*wlc;				/* Backlink to owning wlc context */
	int		scb_handle;			/* SCB cubby handle */
};

typedef wlc_bs_data_counters_t wlc_bs_data_scb_cubby_t; /* The cubby only holds our counters. */

static inline wlc_bs_data_scb_cubby_t **
wlc_bs_data_scb_cubby_ptr(struct wlc_bs_data_info *bdt, struct scb *scb);
static inline wlc_bs_data_scb_cubby_t *
wlc_bs_data_scb_cubby(struct wlc_bs_data_info *bdt, struct scb *scb);

inline wlc_bs_data_scb_cubby_t **
wlc_bs_data_scb_cubby_ptr(struct wlc_bs_data_info *bdt, struct scb *scb)
{
	return (wlc_bs_data_scb_cubby_t **)SCB_CUBBY(scb, bdt->scb_handle);
}

inline wlc_bs_data_scb_cubby_t *
wlc_bs_data_scb_cubby(struct wlc_bs_data_info *bdt, struct scb *scb)
{
	return *wlc_bs_data_scb_cubby_ptr(bdt, scb);
}

/*
 * Externally available function to return a pointer to the counter structure.
 * May return NULL if the SCB is NULL or the counters have not been allocated.
 */
wlc_bs_data_counters_t *
wlc_bs_data_counters(struct wlc_info *wlc, struct scb *scb)
{
	return (scb) ? wlc_bs_data_scb_cubby(wlc->bs_data_handle, scb) : NULL;
}

/*
 * IOVAR definitions and IOVAR handler.
 */
enum bs_data_iovar_numbers {
	IOV_SCB_BS_DATA = 0
};

static const bcm_iovar_t bs_data_iovars[] = {
	{"bs_data", IOV_SCB_BS_DATA, (IOVF_GET_UP), IOVT_BUFFER, 0 },
	{NULL, 0, 0, 0, 0}
};

static int
wlc_bs_data_iovar_handler(void *handle, const bcm_iovar_t * vi, uint32 actionid,
	const char *name, void *params, uint plen, void *arg,
	int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_bs_data_info_t *bdt = handle;
	wlc_info_t *wlc = bdt->wlc;
	int32           int_val = 0;
	int             status = BCME_OK;

	if (plen >= sizeof(int_val)) {
		memcpy(&int_val, params, sizeof(int_val));
	}

	switch (actionid) {
	case IOV_GVAL(IOV_SCB_BS_DATA):
		{
			bool burn_after_reading;
			wlc_bsscfg_t *bsscfg;
			struct scb_iter scbiter;
			struct scb *scb = NULL;
			uint32 tsf_time = (R_REG(wlc->osh, &wlc->regs->tsf_timerlow));
			int	num_records = 0;
			iov_bs_data_struct_t	*output = (iov_bs_data_struct_t *)arg;
			int			rem_len;

			if (alen < sizeof(iov_bs_data_struct_t)) {
				return BCME_BADLEN;	/* way too short */
			}

			rem_len = alen - sizeof(iov_bs_data_struct_t);

			burn_after_reading = ((int_val & SCB_BS_DATA_FLAG_NO_RESET) == 0);

			bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
			ASSERT(bsscfg != NULL);
			if (!bsscfg) {
				return BCME_ERROR;
			}

			FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
				if (SCB_ASSOCIATED(scb)) {
					iov_bs_data_record_t *drec;
					iov_bs_data_counters_t *cd;	/* Counter destination */
					wlc_bs_data_counters_t *cs;	/* Counter source */
					wlc_bs_data_scb_cubby_t *cubby;

					cubby = wlc_bs_data_scb_cubby(bdt, scb);

					/* First time around, init the counters for this SCB. */
					if (!cubby) {
						cubby = MALLOCZ(wlc->osh, sizeof(*cubby));
						if (!cubby) {
							WL_ERROR(("%s: malloc fail bs_data cubby\n",
								__FUNCTION__));
							continue; /* Skip scb if out of memory. */
						}

						cubby->time_delta = tsf_time;
						*wlc_bs_data_scb_cubby_ptr(bdt, scb) = cubby;
					}

					if (rem_len < sizeof(iov_bs_data_record_t)) {
						return BCME_BUFTOOSHORT;
					}
					rem_len -= sizeof(iov_bs_data_record_t);

					drec = &output->structure_record[num_records];
					cd = &drec->station_counters;
					cs = cubby;

					/* Copy station address to output record */
					memcpy(&drec->station_address, &scb->ea, sizeof(scb->ea));

					/* Clear the flag bits (reserved for future use) */
					drec->station_flags = 0;

					/* Copy counters to output record and fix up delta time. */
					cd->retry = cs->retry;
					cd->retry_drop = cs->retry_drop;
					cd->rtsfail = cs->rtsfail;
					cd->acked = cs->acked;
					cd->txrate_main = cs->txrate_main;
					cd->txrate_succ = cs->txrate_succ;
					cd->throughput = cs->throughput;
					cd->time_delta = tsf_time - cs->time_delta;
					cd->airtime = cs->airtime;

					if (burn_after_reading) {
						memset(cs, 0, sizeof(*cs));
						cs->time_delta = tsf_time;
					}
					++num_records;
				}
			}
			output->structure_version = SCB_BS_DATA_STRUCT_VERSION;
			output->structure_count = num_records;
		}
	break;

	default:
		status = BCME_UNSUPPORTED;
		break;
	}

	return status;
}

/*
 * SCB Cubby initialisation and cleanup handlers. Note the cubby itself is a pointer to a struct
 * which is only allocated when the bs_data command is used - until then, it is a NULL pointer.
 */
static int
wlc_bs_data_cubby_init(void *handle, struct scb *scb)
{
	wlc_bs_data_info_t *bdt = handle;

	*wlc_bs_data_scb_cubby_ptr(bdt, scb) = NULL;

	return BCME_OK;
}

static void
wlc_bs_data_cubby_exit(void *handle, struct scb *scb)
{
	wlc_bs_data_info_t *bdt = handle;
	wlc_bs_data_scb_cubby_t *cubby = wlc_bs_data_scb_cubby(bdt, scb);

	if (cubby) {
		MFREE(bdt->wlc->osh, cubby, sizeof(*cubby));
		*wlc_bs_data_scb_cubby_ptr(bdt, scb) = NULL;
	}
}

/*
 * WLC Attach and Detach functions.
 */
wlc_bs_data_info_t *
BCMATTACHFN(wlc_bs_data_attach)(wlc_info_t *wlc)
{
	int status;
	wlc_bs_data_info_t *bdt;

	bdt = MALLOCZ(wlc->osh, sizeof(*bdt));
	if (!bdt) {
		return NULL;
	}
	bdt->wlc = wlc;

	status = wlc_module_register(wlc->pub, bs_data_iovars, module_name, bdt,
		wlc_bs_data_iovar_handler, NULL, NULL, NULL );	/* iovar, watchdog, up, down */

	if (status == BCME_OK) {
		if ((bdt->scb_handle = wlc_scb_cubby_reserve(wlc, sizeof(wlc_bs_data_scb_cubby_t *),
			wlc_bs_data_cubby_init, wlc_bs_data_cubby_exit, NULL, bdt)) < 0) {

			status = BCME_NORESOURCE;
			wlc_module_unregister(wlc->pub, module_name, bdt);
		}
	}

	if (status != BCME_OK) {
		MFREE(wlc->osh, bdt, sizeof(*bdt));
		return NULL;
	}
	return bdt;
}

int
BCMATTACHFN(wlc_bs_data_detach)(wlc_bs_data_info_t *bdt)
{
	wlc_module_unregister(bdt->wlc->pub, module_name, bdt);

	MFREE(bdt->wlc->osh, bdt, sizeof(*bdt));

	return BCME_OK;
}
