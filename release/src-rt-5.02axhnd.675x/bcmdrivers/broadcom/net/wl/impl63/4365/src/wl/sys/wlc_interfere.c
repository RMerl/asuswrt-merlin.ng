/**
 * @file
 * Interference stats module source file
 * Broadcom 802.11abgn Networking Device Driver
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
 * $Id: wlc_interfere.c 708017 2017-06-29 14:11:45Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifdef CCA_STATS
#ifdef ISID_STATS

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <bcmdevs.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_ap.h>
#include <wlc_cca.h>
#include <wlc_interfere.h>
#include <wlc_rm.h>
#include <wlc_rrm.h>
#include <wlc_scan.h>
#include <wlc_bmac.h>
#include <wlc_assoc.h>
#include <wl_export.h>
#ifdef BCMCCX
#include <wlc_ccx.h>
#endif // endif

/* interference data pattern */
typedef struct {
	uint32 source;	/* interference source */
	uint32 min_intensity;	/* min interference intensity to trigger matching */
	uint32 max_intensity;	/* max valid interference intensity */
	uint32 intensity_floor;	/* clean interference intensity gap below min_intensity */
	uint32 min_intensity_diff;	/* min min-max interference intensity diff */
	uint32 min_crsglitch;	/* min CRS glitchs */
	uint32 max_crsglitch;	/* max CRS glitchs */
	uint32 min_badplcp;		/* min bad plcp */
	uint32 max_badplcp;		/* max bad plcp */
	uint8 min_trigger_chs;	/* min num of chs interference over min intensity */
	uint8 max_trigger_chs;	/* max num of chs interference over min intensity */
	uint8 min_passing_chs;	/* min num of chs passing all checks */
	bool max_intensity_on_passing_ch;	/* max intensity must be on a passing channel */
	bool seq_passing_chs;	/* passing channels must be sequential */
} itfr_data_pattern_t;

/* 4312 interference data pattern */
static const itfr_data_pattern_t itfr_4312_pattern[] = {
/* Uniden phone */
{ITFR_PHONE, 800, 1500, 200, 700, 300, 1000, 0, 300, 1, 2, 1, FALSE, TRUE},
/* Lorex and Swann video camera or Vtech phone */
{ITFR_VIDEO_CAMERA_OR_PHONE, 700, 1500, 700, 0, 10, (uint32)(-1), 0, (uint32)(-1), 3, 8, 3,
FALSE, TRUE},
/* Microwave oven */
{ITFR_MICROWAVE_OVEN, 300, 700, 300, 250, 150, 3000, 0, (uint32)(-1), 2, 4, 2, FALSE, TRUE},
/* Baby monitor */
{ITFR_BABY_MONITOR, 200, 1500, 200, 0, 200, (uint32)(-1), 0, (uint32)(-1), 4, CH_MAX_2G_CHANNEL,
5, FALSE, FALSE},
/* Bluetooth */
{ITFR_BLUETOOTH, 100, 400, 100, 0, 100, 500, 0, 1000, 1, CH_MAX_2G_CHANNEL, 1, FALSE, FALSE},
/* Unidentified */
{ITFR_UNIDENTIFIED, 0, 1500, (uint32)(-1), 0, 0, (uint32)(-1), 0, (uint32)(-1), 0,
CH_MAX_2G_CHANNEL, 0, FALSE, FALSE}
};

/* 4313 interference data pattern */
static const itfr_data_pattern_t itfr_4313_pattern[] = {
/* Uniden phone */
{ITFR_PHONE, 850, 1500, 150, 700, 300, 1000, 0, 10, 1, 1, 1, FALSE, FALSE},
/* VTech phone */
{ITFR_PHONE, 800, 1500, 100, 700, 300, 1000, 0, 100, 3, 3, 3, FALSE, TRUE},
/* Lorex and Swann video camera */
{ITFR_VIDEO_CAMERA, 700, 1500, 700, 0, 10, (uint32)(-1), 0, (uint32)(-1), 3, 8, 3, FALSE, TRUE},
/* Microwave oven */
{ITFR_MICROWAVE_OVEN, 500, 900, 500, 250, 150, 20000, 0, 1000, 2, 6, 2, FALSE, TRUE},
/* Microwave oven */
{ITFR_MICROWAVE_OVEN, 350, 700, 350, 250, 50, 3000, 0, (uint32)(-1), 2, 4, 2, FALSE, TRUE},
/* Baby monitor */
{ITFR_BABY_MONITOR, 200, 1500, 200, 0, 200, (uint32)(-1), 0, (uint32)(-1), 5, CH_MAX_2G_CHANNEL,
5, FALSE, FALSE},
/* Bluetooth */
{ITFR_BLUETOOTH, 100, 400, 100, 0, 100, 500, 0, 1000, 1, CH_MAX_2G_CHANNEL, 1, FALSE, FALSE},
/* Unidentified */
{ITFR_UNIDENTIFIED, 0, 1500, (uint32)(-1), 0, 0, (uint32)(-1), 0, (uint32)(-1), 0,
CH_MAX_2G_CHANNEL, 0, FALSE, FALSE}
};

/* 43224 interference data pattern */
static const itfr_data_pattern_t itfr_43224_pattern[] = {
/* Uniden phone */
{ITFR_PHONE, 250, 1500, 100, 200, 8000, (uint32)(-1), 0, 100, 1, 1, 1, FALSE, FALSE},
/* VTech phone */
{ITFR_PHONE, 250, 1500, 250, 200, 1000, (uint32)(-1), 0, 1000, 2, 3, 2, FALSE, TRUE},
/* Microwave oven */
{ITFR_MICROWAVE_OVEN, 100, 500, 100, 100, 9000, (uint32)(-1), 0, 1000, 1, 5, 1, TRUE, TRUE},
/* Microwave oven */
{ITFR_MICROWAVE_OVEN, 300, 600, 300, 200, 10000, (uint32)(-1), 1000, (uint32)(-1), 1, 2, 1,
TRUE, TRUE},
/* Video camera or baby monitor */
{ITFR_VIDEO_CAMERA_OR_BABY_MONITOR, 300, 1500, 300, 0, 400, 1000000, 300, (uint32)(-1), 1,
CH_MAX_2G_CHANNEL, 1, FALSE, FALSE},
/* Bluetooth or baby video monitor */
{ITFR_BLUETOOTH_OR_BABY_MONITOR, 50, 400, 50, 0, 400, 1000000, 300, (uint32)(-1), 1,
CH_MAX_2G_CHANNEL, 1, FALSE, FALSE},
/* Unidentified */
{ITFR_UNIDENTIFIED, 0, 1500, (uint32)(-1), 0, 0, (uint32)(-1), 0, (uint32)(-1), 0,
CH_MAX_2G_CHANNEL, 0, FALSE, FALSE}
};

#define ITFR_MODULE_NAME	"interference"
static const char BCMATTACHDATA(rstr_interference)[] = ITFR_MODULE_NAME;

/* interference info collected from a channel */
typedef struct {
	uint32 intensity;	/* interference intensity */
	uint32 crsglitch;	/* CRS glitchs */
	uint32 badplcp;		/* bad plcps */
	bool interference;	/* set if intesity over value in data pattern */
} itfr_sample_t;

/* default definitions.  time definitions are in seconds except specified */
#define ITFR_SAMPLE_TM		5	/* sample time */
#define ITFR_SCAN_SAMPLE_TM	50	/* sample time in ms per channel during detection scan */
#define ITFR_MAX_VERIFIES	3	/* max number of verifies */
#define ITFR_THRESHOLD		50	/* interference threshold to trigger source detection */
#define ITFR_ID_HOLD_TM		60	/* hold time for identified source before re-detection */
#define ITFR_NON_ID_HOLD_TM	10	/* hold time for non-identified interference source */
#define ITFR_STOP_HOLD_TM	180	/* hold time for stop before restart */
#define ITFR_MAX_NETS		20	/* max nets allowed or feature disabled */
#define ITFR_MIN_CLEAN_TM	10	/* min clean time before changing state to clean */

/* min/max settings for iovars */
#define ITFR_MAX_MODE	ITFR_MODE_AUTO_ENABLE
#define ITFR_MIN_THRESHOLD	30
#define ITFR_MAX_THRESHOLD	0xffff
#define ITFR_MIN_ID_HOLD_TM	1
#define ITFR_MAX_ID_HOLD_TM	0xffff
#define ITFR_MIN_NON_ID_HOLD_TM	1
#define ITFR_MAX_NON_ID_HOLD_TM	0xffff
#define ITFR_MIN_STOP_HOLD_TM	1
#define ITFR_MAX_STOP_HOLD_TM	0xffff

/* state */
typedef enum {
	ITFR_STATE_CLEAN,
	ITFR_STATE_SCAN,
	ITFR_STATE_SCAN_COMPLETED,
	ITFR_STATE_IDENTIFIED,
	ITFR_STATE_STOPPED
} itfr_state_t;

/* structure for interference handling */
struct itfr_info {
	wlc_info_t *wlc;	/* backpointer to main struct */
	uint16 threshold;	/* interference threshold for triggering source detection */
	uint8 source;		/* current interference source */
	uint8 last_source;	/* last interference source */
	uint8 state;		/* operating state  */
	uint reset_tm;		/* time to reset operating */
	uint8 verify_pending;	/* pending result verification times */
	uint16 clean_tm;	/* clean time */
	uint16 min_clean_tm;	/* min clean time before changing state to clean */
	uint16 id_hold_tm;	/* hold time for source identified before re-detection */
	uint16 non_id_hold_tm;	/* hold time for source not identified */
	uint16 stop_hold_tm;	/* stop state(due to num of ap over limit) hold time */
	uint16 max_nets;	/* max num of networks allowed */
	uint op_mode;		/* operating mode */
	int org_aci_val;	/* original aci setting */
	bool aci_mode_configed;	/* aci mode configured for detection */
	bool no_aci;		/* need disabling aci when detecting interference */
	bool detect_req;	/* detection request pending */
	chanspec_t chanlist[CH_MAX_2G_CHANNEL];	/* scan channels */
	uint8 chan_num;		/* number of scan channels */
	interference_source_rep_t report;	/* interference report */
};

static void itfr_init_stats(itfr_info_t *itfr);
static void itfr_reset_state(itfr_info_t *itfr, bool verify);
static int itfr_set_mode(itfr_info_t *itfr, int mode);
static int itfr_detect_req(itfr_info_t *itfr);
static int itfr_abort_req(itfr_info_t *itfr);
static int itfr_set_net_thres_req(itfr_info_t *itfr, int val);
static int itfr_get_stats(itfr_info_t *itfr, interference_source_rep_t* rep);
static void itfr_scan_complete(void *arg, int status, wlc_bsscfg_t *cfg);
static int itfr_scan_request(itfr_info_t *itfr);
static void itfr_report(itfr_info_t *itfr, bool interference, bool home_channel, uint32 source);
static void itfr_get_valid_2g_channels(itfr_info_t *itfr);
static bool itfr_get_sample(itfr_info_t *itfr, itfr_sample_t *itfr_sample, uint sample_time);
static bool itfr_match_pattern(itfr_info_t *itfr, itfr_sample_t *itfr_sample,
	const itfr_data_pattern_t *itfr_pattern);
static void itfr_identify_source(itfr_info_t *itfr, itfr_sample_t *itfr_sample);
static void itfr_stats_watchdog(void *ctx);
static int wlc_itfr_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
#if defined(BCMDBG_DUMP)
static int wlc_itfr_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

/* IOVar table */
enum {
	IOV_ITFR_STATS,		/* report interference source state */
	IOV_ITFR_ENAB,		/* interference source detection and identification */
	IOV_ITFR_DETECT,	/* trigger detection for interference on different channel */
	IOV_ITFR_ABORT,		/* abort current detection */
	IOV_ITFR_THRES,		/* interference threshold */
	IOV_ITFR_NET_THRES,	/* max allowed num of networks in working environment */
	IOV_ITFR_STOP_TM,	/* stop duration(in second) when num of AP over limit */
	IOV_ITFR_ID_HOLD_TM,	/* hold duration(in second) for identified source */
	IOV_ITFR_NON_ID_HOLD_TM,	/* hold duration(in second) fo non-identified source */
	IOV_ITFR_CLEAN_TM,	/* clean duration(in second) before changing state to clean */
	IOV_LAST
};

static const bcm_iovar_t wlc_itfr_iovars[] = {
	{"itfr_get_stats", IOV_ITFR_STATS,
	(0), IOVT_BUFFER, sizeof(interference_source_rep_t),
	},
	{"itfr_enab", IOV_ITFR_ENAB,
	(0), IOVT_UINT32, 0,
	},
	{"itfr_detect", IOV_ITFR_DETECT,
	(0), IOVT_VOID, 0,
	},
	{"itfr_abort", IOV_ITFR_ABORT,
	(0), IOVT_VOID, 0,
	},
	{"itfr_thres", IOV_ITFR_THRES,
	0, IOVT_UINT32, 0
	},
	{"itfr_net_thres", IOV_ITFR_NET_THRES,
	0, IOVT_UINT32, 0
	},
	{"itfr_stop_tm", IOV_ITFR_STOP_TM,
	0, IOVT_UINT32, 0
	},
	{"itfr_id_hold_tm", IOV_ITFR_ID_HOLD_TM,
	0, IOVT_UINT32, 0
	},
	{"itfr_non_id_hold_tm", IOV_ITFR_NON_ID_HOLD_TM,
	0, IOVT_UINT32, 0
	},
	{"itfr_clean_tm", IOV_ITFR_CLEAN_TM,
	0, IOVT_UINT32, 0
	},
	{NULL, 0, 0, 0, 0}
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

itfr_info_t *
BCMATTACHFN(wlc_itfr_attach)(wlc_info_t *wlc)
{
	itfr_info_t *itfr = NULL;

	if (D11REV_LT(wlc->pub->corerev, 15) ||
		CHIPID(wlc->pub->sih->chip) == BCM4312_CHIP_ID) {
		WL_ERROR(("%s: Chips revisions < 15 are not supported\n", __FUNCTION__));
		goto fail;
	}

	if ((itfr = MALLOCZ(wlc->osh, sizeof(itfr_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	itfr->wlc = wlc;

	itfr_init_stats(itfr);

#if defined(BCMDBG_DUMP)
	if (wlc_dump_register(wlc->pub, ITFR_MODULE_NAME, wlc_itfr_dump, itfr) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_dumpe_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif

	if (wlc_module_register(wlc->pub, wlc_itfr_iovars, rstr_interference,
	    (void *)itfr, wlc_itfr_doiovar, itfr_stats_watchdog, NULL,
	    NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		wlc->pub->unit, __FUNCTION__));
		goto fail;
	};
	return itfr;
fail:
	if (itfr != NULL)
		MFREE(wlc->osh, itfr, sizeof(itfr_info_t));
	return NULL;
}

void
BCMATTACHFN(wlc_itfr_detach)(itfr_info_t *itfr)
{
	wlc_info_t *wlc = itfr->wlc;
	wlc_module_unregister(wlc->pub, rstr_interference, itfr);
	MFREE(wlc->osh, itfr, sizeof(itfr_info_t));
}

static int
wlc_itfr_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	itfr_info_t *itfr = (itfr_info_t *)ctx;
	int err = 0;
	int32 int_val = 0;
	int32 *ret_int_ptr;
	bool bool_val;

	BCM_REFERENCE(bool_val);
	if (ctx == NULL) {
		WL_ERROR(("%s: Invalid NULL context\n", __FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	if (itfr->wlc == NULL) {
		WL_ERROR(("%s: Invalid wlc context\n", __FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	/* convenience int and bool vals for first 4 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bool_val = (int_val != 0) ? TRUE : FALSE;

	switch (actionid) {
	case IOV_GVAL(IOV_ITFR_STATS):
		if (len < (int)sizeof(interference_source_rep_t))
			err = BCME_BUFTOOSHORT;
		else
			err = itfr_get_stats(itfr, (interference_source_rep_t*)arg);

		break;

	case IOV_GVAL(IOV_ITFR_ENAB):
		*ret_int_ptr = (int)itfr->op_mode;
		break;

	case IOV_SVAL(IOV_ITFR_ENAB):
		err = itfr_set_mode(itfr, int_val);
		break;

	case IOV_SVAL(IOV_ITFR_DETECT):
		err = itfr_detect_req(itfr);
		break;

	case IOV_SVAL(IOV_ITFR_ABORT):
		err = itfr_abort_req(itfr);
		break;

	case IOV_GVAL(IOV_ITFR_THRES):
		*ret_int_ptr = (int)itfr->threshold;
		break;

	case IOV_SVAL(IOV_ITFR_THRES):
		if (int_val < ITFR_MIN_THRESHOLD ||
			int_val > ITFR_MAX_THRESHOLD)
			err = BCME_BADARG;
		else
			itfr->threshold = (uint16)int_val;
		break;

	case IOV_GVAL(IOV_ITFR_NET_THRES):
		*ret_int_ptr = (int)itfr->max_nets;
		break;

	case IOV_SVAL(IOV_ITFR_NET_THRES):
		err = itfr_set_net_thres_req(itfr, int_val);
		break;

	case IOV_GVAL(IOV_ITFR_STOP_TM):
		*ret_int_ptr = (int)itfr->stop_hold_tm;
		break;

	case IOV_SVAL(IOV_ITFR_STOP_TM):
		if (int_val < ITFR_MIN_STOP_HOLD_TM ||
			int_val > ITFR_MAX_STOP_HOLD_TM)
			err = BCME_BADARG;
		else
			itfr->stop_hold_tm = (uint16)int_val;
		break;

	case IOV_GVAL(IOV_ITFR_ID_HOLD_TM):
		*ret_int_ptr = (int)itfr->id_hold_tm;
		break;

	case IOV_SVAL(IOV_ITFR_ID_HOLD_TM):
		if (int_val < ITFR_MIN_ID_HOLD_TM ||
			int_val > ITFR_MAX_ID_HOLD_TM)
			err = BCME_BADARG;
		else
			itfr->id_hold_tm = (uint16)int_val;
		break;

	case IOV_GVAL(IOV_ITFR_NON_ID_HOLD_TM):
		*ret_int_ptr = (int)itfr->non_id_hold_tm;
		break;

	case IOV_SVAL(IOV_ITFR_NON_ID_HOLD_TM):
		if (int_val < ITFR_MIN_NON_ID_HOLD_TM ||
			int_val > ITFR_MAX_NON_ID_HOLD_TM)
			err = BCME_BADARG;
		else
			itfr->non_id_hold_tm = (uint16)int_val;
		break;

	case IOV_GVAL(IOV_ITFR_CLEAN_TM):
		*ret_int_ptr = (int)itfr->clean_tm;
		break;

	case IOV_SVAL(IOV_ITFR_CLEAN_TM):
		itfr->clean_tm = (uint16)int_val;
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}
	return err;
}

#ifdef BCMDBG
/* need keeping sync with interference_source in wlioctl.h */
static const char *itf_source_str[] = {
	"no interference",
	"wireless phone",
	"video camera",
	"microwave oven",
	"baby monitor",
	"bluetooth",
	"video camera or baby monitor",
	"bluetooth or baby monitor",
	"video camera or phone",
	"unidentified"
};

static const char *itfr_state_str[] =
	{"clean", "scan", "scan completed", "identified", "stopped"};
#endif /* BCMDBG */

static void
BCMATTACHFN(itfr_init_stats)(itfr_info_t *itfr)
{
	/* init variables */
	itfr->source = ITFR_NONE;
	itfr->last_source = ITFR_NONE;
	itfr->state = ITFR_STATE_CLEAN;
	itfr->threshold = ITFR_THRESHOLD;
	itfr->max_nets = ITFR_MAX_NETS;
	itfr->min_clean_tm = ITFR_MIN_CLEAN_TM;
	itfr->id_hold_tm = ITFR_ID_HOLD_TM;
	itfr->non_id_hold_tm = ITFR_NON_ID_HOLD_TM;
	itfr->stop_hold_tm = ITFR_STOP_HOLD_TM;
	if (CHIPID(itfr->wlc->pub->sih->chip) == BCM4312_CHIP_ID)
		itfr->no_aci = TRUE;

	/* init report */
	itfr->report.flags = 0;
	itfr->report.source = ITFR_NONE;
	itfr->report.timestamp = OSL_SYSUPTIME();

	/* feature is disabled by default */
	itfr->op_mode = ITFR_MODE_DISABLE;
}

/* reset interference detection and identification state */
static void
itfr_reset_state(itfr_info_t *itfr, bool verify)
{
	wlc_info_t *wlc = itfr->wlc;

	itfr->detect_req = FALSE;

	/* if in scan, abort it */
	if (itfr->state == ITFR_STATE_SCAN)
		wlc_scan_abort(wlc->scan, WLC_E_STATUS_ABORT);

	/* reset variables */
	itfr->clean_tm = 0;
	itfr->reset_tm = 0;
	itfr->source = ITFR_NONE;
	if (!verify) {
		/* clean verifying variables */
		itfr->verify_pending = 0;
		itfr->last_source = ITFR_NONE;
		if (itfr->no_aci && itfr->aci_mode_configed) {
			/* restore aci setting */
			wlc_set(wlc, WLC_SET_INTERFERENCE_MODE, itfr->org_aci_val);
			itfr->aci_mode_configed = FALSE;
		}
	}
	/* reset state */
	itfr->state = ITFR_STATE_CLEAN;
}

/* set operation mode */
static int
itfr_set_mode(itfr_info_t *itfr, int mode)
{
	if (!itfr)
		return BCME_UNSUPPORTED;
	else if (mode > ITFR_MAX_MODE)
		return BCME_BADARG;

	/* reset state */
	itfr_reset_state(itfr, FALSE);
	/* set mode */
	itfr->op_mode = mode;

	return BCME_OK;
}

/* handle interference detection request from iovar */
static int
itfr_detect_req(itfr_info_t *itfr)
{
	wlc_info_t *wlc = itfr->wlc;

	if (itfr->op_mode == ITFR_MODE_DISABLE)
		return BCME_UNSUPPORTED;

	/* reset state */
	itfr_reset_state(itfr, FALSE);
	/* set flag for pending detection */
	itfr->detect_req = TRUE;

	wlc->mpc_scan = TRUE;

	if (!wlc->pub->up)
		/* make wlc up for scan and watchdog */
		wlc_radio_mpc_upd(wlc);

	if (!wlc->pub->up) {
		WL_ERROR(("%s: wl up failed\n", __FUNCTION__));
		return BCME_NOTUP;
	}

	WL_ITFR(("%s: detect interference per user's request\n", __FUNCTION__));

	return BCME_OK;
}

/* handle interference detection abortion request from iovar */
static int
itfr_abort_req(itfr_info_t *itfr)
{
	if (itfr->op_mode == ITFR_MODE_DISABLE)
		return BCME_UNSUPPORTED;

	/* reset state to abort any activities */
	itfr_reset_state(itfr, FALSE);

	return BCME_OK;
}

static int
itfr_set_net_thres_req(itfr_info_t *itfr, int val)
{
	/* reset state */
	itfr_reset_state(itfr, FALSE);
	/* set net threshold */
	itfr->max_nets = (uint16)val;

	return BCME_OK;
}

static int
itfr_get_stats(itfr_info_t *itfr, interference_source_rep_t* rep)
{
	if (!itfr || itfr->op_mode == ITFR_MODE_DISABLE)
		return BCME_UNSUPPORTED;

	/* if request is not completed */
	if (itfr->detect_req)
		return BCME_BUSY;

	*rep = itfr->report;

	return BCME_OK;
}

/* interference detection scan completion handler */
static void
itfr_scan_complete(void *arg, int status, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;
	itfr_info_t *itfr = wlc->itfr_info;

	if (itfr->state != ITFR_STATE_SCAN)
		return;

	itfr->state = ITFR_STATE_SCAN_COMPLETED;

	if (status == WLC_E_STATUS_SUCCESS) {
		/* check num of networks */
		if (wlc->scan_results->count > itfr->max_nets) {
			itfr_reset_state(itfr, FALSE);
			itfr->reset_tm = wlc->pub->now + itfr->stop_hold_tm;
			itfr->state = ITFR_STATE_STOPPED;
			/* report noisy environment */
			itfr->report.flags |= ITFR_NOISY_ENVIRONMENT;
		}
	} else {
		if (itfr->detect_req)
			/* re-issue request */
			itfr_detect_req(itfr);
		else
			/* restart */
			itfr_reset_state(itfr, (itfr->verify_pending != 0));
	}
}

static int
itfr_scan_request(itfr_info_t *itfr)
{
	wlc_info_t *wlc = itfr->wlc;
	wlc_ssid_t req_ssid;

	/* disable aci if needed */
	if (itfr->no_aci && !itfr->aci_mode_configed) {
		if (!wlc_get(wlc, WLC_GET_INTERFERENCE_MODE, &itfr->org_aci_val)) {
			if (itfr->org_aci_val == NON_WLAN ||
				((itfr->org_aci_val == WLAN_AUTO ||
				itfr->org_aci_val == WLAN_AUTO_W_NOISE) &&
				itfr->org_aci_val & AUTO_ACTIVE)) {
				itfr->org_aci_val &= ~AUTO_ACTIVE;
				/* disable aci */
				wlc_set(wlc, WLC_SET_INTERFERENCE_MODE, INTERFERE_NONE);
				itfr->aci_mode_configed = TRUE;
			}
		} else {
			WL_ERROR(("%s: get aci mode error\n", __FUNCTION__));
			return BCME_ERROR;
		}
	}

	bzero(&req_ssid, sizeof(req_ssid));
	/* start active scan to collect interference info and ap info */
	return wlc_scan_request(wlc, DOT11_BSSTYPE_ANY, &ether_bcast, 1,
		&req_ssid, DOT11_SCANTYPE_ACTIVE, -1, ITFR_SCAN_SAMPLE_TM,
		ITFR_SCAN_SAMPLE_TM, ITFR_SCAN_SAMPLE_TM, itfr->chanlist,
		itfr->chan_num, FALSE, itfr_scan_complete, wlc);
}

/* fill out interference report */
static void
itfr_report(itfr_info_t *itfr, bool interference, bool home_channel,
	uint32 source)
{
	interference_source_rep_t *report = &itfr->report;

	if (itfr->report.flags & ITFR_NOISY_ENVIRONMENT)
		/* clear noisy environment flag since we have report */
		itfr->report.flags &= ~ITFR_NOISY_ENVIRONMENT;
	/* done if no change */
	else if ((interference && (report->flags & ITFR_INTERFERENCED) &&
		source == report->source &&
		(home_channel == ((report->flags & ITFR_HOME_CHANNEL) != 0))) ||
		(!interference && !(report->flags & ITFR_INTERFERENCED)))
		return;

	/* if not interference, update interference flag and timestamp only
	 * and keep last report info
	 */
	if (interference) {
		report->source = source;
		report->flags |= ITFR_INTERFERENCED;
		if (home_channel)
			report->flags |= ITFR_HOME_CHANNEL;
		else
			report->flags &= ~ITFR_HOME_CHANNEL;
	} else
		report->flags &= ~ITFR_INTERFERENCED;

	report->timestamp = OSL_SYSUPTIME();

	WL_ITFR(("wl%d: itfr_report: interference %s. last source is %s on %s channel"
		" at time %d\n", itfr->wlc->pub->unit,
		(report->flags & ITFR_INTERFERENCED) ? "detected" : "not detected",
		itf_source_str[source], (report->flags & ITFR_HOME_CHANNEL) ? "home" : "non-home",
		report->timestamp));
}

static void
itfr_get_valid_2g_channels(itfr_info_t *itfr)
{
	chanspec_t chanspec;
	wlc_info_t *wlc = itfr->wlc;
	uint8 i, j;

	/* init scan channels */
	for (i = 0, j = 0; i < CCA_CHANNELS_NUM; i++) {
		chanspec = wlc_cca_get_chanspec(wlc, i);
		if (CHSPEC_IS2G(chanspec) &&
			wlc_valid_chanspec_db(wlc->cmi, chanspec)) {
			itfr->chanlist[j] = chanspec;
			j++;
		}
	}
	itfr->chan_num = j;
}

static bool
itfr_get_sample(itfr_info_t *itfr, itfr_sample_t *itfr_sample, uint sample_time)
{
	wlc_info_t *wlc = itfr->wlc;
	wlc_congest_channel_req_t *result;
	uint result_len;
	uint i, j;

	result_len = sizeof(wlc_congest_channel_req_t) +
		((sample_time - 1) * sizeof(wlc_congest_t));
	if (!(result = (wlc_congest_channel_req_t*)MALLOC(wlc->osh, result_len))) {
		itfr_reset_state(itfr, FALSE);
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}

	/* sample channels */
	for (i = 0; i < itfr->chan_num; i++) {
		uint32 interference_sum = 0;
		uint32 crsglitch_sum = 0;
		uint32 badplcp_sum = 0;
		uint32 duration_sum = 0;
		uint8 ch = CHSPEC_CHANNEL(itfr->chanlist[i]);
		bzero(result, result_len);
		cca_query_stats(wlc, CH20MHZ_CHSPEC(ch), sample_time, result, result_len);

		for (j = 0; j < sample_time; j++) {
			/* update sumaries for this channel */
			interference_sum += result->secs[j].interference;
			crsglitch_sum += result->secs[j].crsglitch;
			badplcp_sum += result->secs[j].badplcp;
			duration_sum += result->secs[j].duration;
		}
		if (duration_sum) {
			/* save averaged values for this channel */
			itfr_sample[ch - 1].intensity = (interference_sum * 1000) / duration_sum;
			itfr_sample[ch - 1].crsglitch = (crsglitch_sum * 1000) / duration_sum;
			itfr_sample[ch - 1].badplcp = (badplcp_sum * 1000) / duration_sum;
		} else
			itfr_sample[ch - 1].intensity = 0;
		itfr_sample[ch - 1].interference =
			(itfr_sample[ch - 1].intensity > itfr->threshold) ? TRUE : FALSE;
		if (itfr->state == ITFR_STATE_SCAN_COMPLETED)
			WL_ITFR(("wl%d: itfr_get_sample: ch %d: intensity %d, crsglitch %d,"
				" badplcp %d, interferenced %d\n", wlc->pub->unit, ch,
				itfr_sample[ch - 1].intensity, itfr_sample[ch - 1].crsglitch,
				itfr_sample[ch - 1].badplcp, itfr_sample[ch - 1].interference));
	}
	MFREE(wlc->osh, result, result_len);

	return BCME_OK;
}

static bool
itfr_match_pattern(itfr_info_t *itfr, itfr_sample_t *itfr_sample,
	const itfr_data_pattern_t *itfr_pattern)
{
	uint i, max_match_ch;
	bool ch_source_identified = FALSE, source_identified = FALSE;
	uint32 last_passing_channel = 0, trigger_channels = 0;
	uint32 passing_channels = 0, zero_itfr_channels = 0;
	uint32 min_intensity = (uint32)(-1), max_intensity = 0;
	uint32 max_intensity_on_passing_chs = 0;

	/* limit pattern match up to channel 11 */
	max_match_ch = MIN(11, itfr->chan_num);
	for (i = 0; i < max_match_ch; i++) {
		uint8 ch = CHSPEC_CHANNEL(itfr->chanlist[i]);
		uint32 intensity = itfr_sample[ch - 1].intensity;
		uint32 crsglitch = itfr_sample[ch - 1].crsglitch;
		uint32 badplcp = itfr_sample[ch - 1].badplcp;
		bool interference = itfr_sample[ch - 1].interference;
		if (interference && intensity >= itfr_pattern->min_intensity) {
			if (intensity > itfr_pattern->max_intensity) {
				/* check failed.  check next interference type */
				ch_source_identified = FALSE;
				break;
			}

			/* check if meet per channel pattern */
			if (crsglitch >= itfr_pattern->min_crsglitch &&
				crsglitch <= itfr_pattern->max_crsglitch &&
				badplcp >= itfr_pattern->min_badplcp &&
				badplcp <= itfr_pattern->max_badplcp) {
				if (itfr_pattern->seq_passing_chs &&
					last_passing_channel != 0 &&
					(last_passing_channel + 1) != ch) {
					/* check failed.  check next interference type */
					ch_source_identified = FALSE;
					break;
				}
				last_passing_channel = ch;
				passing_channels++;
				if (intensity > max_intensity_on_passing_chs)
					max_intensity_on_passing_chs = intensity;
				ch_source_identified = TRUE;
			}
			trigger_channels++;
		} else if (intensity > itfr_pattern->intensity_floor) {
			/* check failed.  check next interference type */
			ch_source_identified = FALSE;
			break;
		} else if (intensity == 0)
			zero_itfr_channels++;

		/* update min and max interference intensity variables */
		if (intensity) {
			if (intensity < min_intensity)
				min_intensity = intensity;
			if (intensity > max_intensity)
				max_intensity = intensity;
		}
	}

	/* check if meet per source pattern in addtion to channel pattern */
	if (ch_source_identified &&
		trigger_channels >= itfr_pattern->min_trigger_chs &&
		trigger_channels <= itfr_pattern->max_trigger_chs &&
		passing_channels >= itfr_pattern->min_passing_chs &&
		(max_intensity - min_intensity) >= itfr_pattern->min_intensity_diff &&
		(!itfr_pattern->max_intensity_on_passing_ch ||
		max_intensity == max_intensity_on_passing_chs) &&
		(itfr_pattern->source == ITFR_UNIDENTIFIED ||
		zero_itfr_channels < ((uint32)itfr->chan_num - 2))) {
		itfr->source = (uint8)itfr_pattern->source;
		source_identified = TRUE;
	}

	return source_identified;
}

/* return true if home channel has interference */
static void
itfr_identify_source(itfr_info_t *itfr, itfr_sample_t *itfr_sample)
{
	wlc_info_t *wlc = itfr->wlc;
	uint i, patterns;
	const itfr_data_pattern_t *itfr_pattern;

	BCM_REFERENCE(wlc);
	if (CHIPID(wlc->pub->sih->chip) == BCM4312_CHIP_ID) {
		itfr_pattern = itfr_4312_pattern;
		patterns = ARRAYSIZE(itfr_4312_pattern);
	} else if (CHIPID(wlc->pub->sih->chip) == BCM4313_CHIP_ID) {
		itfr_pattern = itfr_4313_pattern;
		patterns = ARRAYSIZE(itfr_4313_pattern);
	} else {
		itfr_pattern = itfr_43224_pattern;
		patterns = ARRAYSIZE(itfr_43224_pattern);
	}
	/* detecting interference source */
	for (i = 0; i < patterns; i++) {
		if (itfr_match_pattern(itfr, itfr_sample, &itfr_pattern[i]))
			break;
	}
}
#if defined(BCMDBG_DUMP)
static int wlc_itfr_dump(void *ctx, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "%s:  If I had something to say, it would go here\n",
		__FUNCTION__);
	return BCME_OK;
}
#endif // endif

/* monitor interference on b/g band */
static void
itfr_stats_watchdog(void *ctx)
{
	itfr_info_t *itfr = (itfr_info_t *)ctx;
	wlc_info_t *wlc;
	itfr_sample_t *itfr_sample;
	uint sample_time;
	uint8 ch, home_ch;
	bool home_ch_interference;
	uint8 interference_chs = 0, zero_interference_chs = 0;
	uint i;

	/* give time to collect cca info when power on */
	if (!itfr)
		return;

	ASSERT(itfr->wlc);
	wlc = itfr->wlc;

	if (wlc->pub->now < MAX_CCA_SECS && !itfr->detect_req)
		return;

	if (itfr->op_mode == ITFR_MODE_DISABLE || (!itfr->detect_req &&
		((itfr->op_mode == ITFR_MODE_AUTO_ENABLE && !wlc->pub->associated) ||
		(itfr->op_mode == ITFR_MODE_MANUAL_ENABLE))) ||
		(home_ch = wf_chspec_ctlchan(wlc->home_chanspec)) == 0 ||
		home_ch > CH_MAX_2G_CHANNEL) {
		if (itfr->state != ITFR_STATE_CLEAN || itfr->verify_pending)
			itfr_reset_state(itfr, FALSE);
		return;
	}

	WL_ITFR(("wl%d: %s: state is %s\n", wlc->pub->unit, __FUNCTION__,
		itfr_state_str[itfr->state]));

	if (itfr->state == ITFR_STATE_IDENTIFIED || itfr->state == ITFR_STATE_STOPPED) {
		/* if holding time expired */
		if (itfr->reset_tm == wlc->pub->now) {
			if (itfr->state == ITFR_STATE_STOPPED)
				/* clear noisy environment flag */
				itfr->report.flags &= ~ITFR_NOISY_ENVIRONMENT;
			/* allow restarting operation */
			itfr_reset_state(itfr, FALSE);
		}

		if (itfr->no_aci && itfr->aci_mode_configed) {
			/* restore aci setting */
			wlc_set(wlc, WLC_SET_INTERFERENCE_MODE, itfr->org_aci_val);
			itfr->aci_mode_configed = FALSE;
		}

		if (itfr->state == ITFR_STATE_STOPPED)
			return;
	}

	if (itfr->state == ITFR_STATE_SCAN)
		/* if scan not complete, wait */
		return;

	/* no interference detection when scan in progress */
	if (itfr->state == ITFR_STATE_CLEAN &&
		(AS_IN_PROGRESS(wlc) || SCAN_IN_PROGRESS(wlc->scan) ||
#if defined(BCMCCX) && defined(CCX_SDK)
		wlc->ccx->diag_mode ||
#endif // endif
#ifdef WL11K
		wlc_rrm_in_progress(wlc) ||
#endif /* WL11K */
	WLC_RM_IN_PROGRESS(wlc))) {
		WL_ITFR(("%s: exit due to other scan in progress\n", __FUNCTION__));
		return;
	}

	/* get current valid b/g channels */
	itfr_get_valid_2g_channels(itfr);

	/* allocate buffer for interference sample */
	if (!(itfr_sample = (itfr_sample_t*)
		MALLOCZ(wlc->osh, sizeof(itfr_sample_t) * itfr->chan_num))) {
		itfr_reset_state(itfr, FALSE);
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return;
	}

	/* select sample time for interference */
	sample_time = (itfr->state == ITFR_STATE_SCAN_COMPLETED) ? 2 : ITFR_SAMPLE_TM;

	/* get interference sample */
	if (itfr_get_sample(itfr, itfr_sample, sample_time) != BCME_OK)
		goto itfr_exit;

	/* collect affected and non-affected channel info */
	for (i = 0; i < itfr->chan_num; i++) {
		ch = CHSPEC_CHANNEL(itfr->chanlist[i]);
		if (itfr_sample[ch - 1].interference)
			interference_chs++;
		if (itfr_sample[ch - 1].intensity == 0)
			zero_interference_chs++;
	}
	home_ch_interference = itfr_sample[home_ch - 1].interference;

	if (itfr->state == ITFR_STATE_SCAN_COMPLETED) {
		/* if sample is invalid */
		if (zero_interference_chs >= (itfr->chan_num - 1)) {
			if (itfr->detect_req)
				/* re-issue request */
				itfr_detect_req(itfr);
			else
				itfr_reset_state(itfr, (itfr->verify_pending != 0));
			goto itfr_exit;
		}

		/* if no interference detected on user request,
		 * report and complete the request
		 */
		if (itfr->detect_req && interference_chs == 0) {
			itfr_report(itfr, FALSE, FALSE, ITFR_NONE);
			itfr_reset_state(itfr, FALSE);
			goto itfr_exit;
		}
	}

	/* check if interference source removed after report interference */
	if (itfr->report.flags & ITFR_INTERFERENCED) {
		/* update clean count */
		if (itfr->report.flags & ITFR_HOME_CHANNEL) {
			if (home_ch_interference)
				itfr->clean_tm = 0;
			else
				itfr->clean_tm++;
		} else if (itfr->state == ITFR_STATE_SCAN_COMPLETED) {
			if ((interference_chs - (home_ch_interference ? 1 : 0)) != 0)
				itfr->clean_tm = 0;
			else
				itfr->clean_tm++;
		}

		/* report and reset state if interference gone for given amount of time */
		if (itfr->clean_tm == itfr->min_clean_tm) {
			itfr_report(itfr, FALSE, FALSE, ITFR_NONE);
			itfr_reset_state(itfr, FALSE);
			goto itfr_exit;
		}
	}

	/* exit if in identified state */
	if (itfr->state == ITFR_STATE_IDENTIFIED)
		goto itfr_exit;

	/* exit if no interference */
	if (interference_chs == 0 && !itfr->detect_req) {
		itfr_reset_state(itfr, FALSE);
		goto itfr_exit;
	}

	/* issue scan if interference detected or on user request */
	if ((itfr_sample[home_ch - 1].interference || itfr->detect_req) &&
		itfr->state == ITFR_STATE_CLEAN) {
		if (itfr_scan_request(itfr) == BCME_OK)
			itfr->state = ITFR_STATE_SCAN;

		goto itfr_exit;
	}

	/* detecting interference source after scan */
	if (itfr->state == ITFR_STATE_SCAN_COMPLETED) {
		/* identify interference source based on samples */
		itfr_identify_source(itfr, itfr_sample);

		/* verify result or identified */
		if (itfr->verify_pending == 0) {
			itfr->verify_pending = ITFR_MAX_VERIFIES;
		} else {
			if (itfr->last_source == itfr->source) {
				itfr->state = ITFR_STATE_IDENTIFIED;
			} else if (--itfr->verify_pending == 0) {
				itfr->source = ITFR_UNIDENTIFIED;
				itfr->state = ITFR_STATE_IDENTIFIED;
			}
		}

		if (itfr->state == ITFR_STATE_IDENTIFIED) {
			/* update report */
			itfr_report(itfr, TRUE, home_ch_interference, itfr->source);
			itfr->verify_pending = 0;
			itfr->clean_tm = 0;
			itfr->detect_req = FALSE;
			if (itfr->op_mode == ITFR_MODE_AUTO_ENABLE) {
				/* hold result for a while */
				if (itfr->source == ITFR_UNIDENTIFIED)
					itfr->reset_tm = wlc->pub->now + itfr->non_id_hold_tm;
				else
					itfr->reset_tm = wlc->pub->now + itfr->id_hold_tm;
			}
			WL_ITFR(("wl%d: %s: interference source is %s\n",
				wlc->pub->unit, __FUNCTION__, itf_source_str[itfr->source]));
		} else {
			bool detect_req = itfr->detect_req;
			/* verify result */
			itfr->last_source = itfr->source;
			itfr_reset_state(itfr, TRUE);
			/* restore flag cleared when reset state */
			itfr->detect_req = detect_req;
			/* issue scan request */
			WL_ITFR(("wl%d: %s: interference source"
				" could be %s. verify again\n",
				wlc->pub->unit, __FUNCTION__, itf_source_str[itfr->last_source]));
		}
	}

itfr_exit:
	MFREE(wlc->osh, itfr_sample, sizeof(itfr_sample_t) * itfr->chan_num);
}

#endif /* ISID_STATS */
#endif /* CCA_STATS */
