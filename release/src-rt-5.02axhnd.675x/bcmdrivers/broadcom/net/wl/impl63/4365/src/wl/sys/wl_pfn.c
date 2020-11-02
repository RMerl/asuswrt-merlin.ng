/*
 * Preferred network source file
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
 * $Id: wl_pfn.c 708017 2017-06-29 14:11:45Z $
 *
 */

/**
 * @file
 * @brief
 * Preferred network offload is for host to configure dongle with a list of preferred networks.
 * So dongle can perform PNO scan based on these preferred networks and notify host of its
 * findings.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [PreferredNetworkOffload] [PreferredNetworkOffloadEnhance]
 */

/* This define is to help mogrify the code */
/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifdef WLPFN

#include <typedefs.h>
#include <osl.h>
#include <wl_dbg.h>

#include <bcmutils.h>
#include <siutils.h>
#include <bcmwpa.h>
#include <proto/ethernet.h>
#include <proto/802.11.h>

#include <wlioctl.h>
#include <wlc_pub.h>
#include <wl_dbg.h>

#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scan.h>

#include <wl_export.h>

#include <wl_pfn.h>
#include <event_trace.h>

/* Useful defines and enums */
#define MIN_RSSI		-32768
#define MAX_BSSID_LIST		180
#define MAX_BDCAST_NUM		50

#define DEFAULT_SCAN_FREQ	60	/* in sec */
#define DEFAULT_LOST_DURATION	120	/* in sec */
#define DEFAULT_RSSI_MARGIN	30	/* in db */
#define DEFAULT_BKGROUND_SCAN	FALSE
#define DEFAULT_BD_SCAN		FALSE

#define WL_PFN_RSSI_ONCHANNEL	0x01

#define EVENT_DATABUF_MAXLEN	(512-sizeof(bcm_event_t))
#define EVENT_MAX_NETCNT \
((EVENT_DATABUF_MAXLEN - sizeof(wl_pfn_scanresults_t)) / sizeof(wl_pfn_net_info_t) + 1)
#define EVENT_DATA_MAXLEN \
(sizeof(wl_pfn_scanresults_t) +  (EVENT_MAX_NETCNT - 1) * sizeof(wl_pfn_net_info_t))

#define EVENT_MAX_LNETCNT \
((EVENT_DATABUF_MAXLEN - sizeof(wl_pfn_lscanresults_t)) / sizeof(wl_pfn_lnet_info_t) + 1)
#define EVENT_LDATA_MAXLEN \
(sizeof(wl_pfn_lscanresults_t) +  (EVENT_MAX_NETCNT - 1) * sizeof(wl_pfn_lnet_info_t))

#define CLEAR_WLPFN_AC_FLAG(flags)	(flags &= ~AUTO_CONNECT_MASK)
#define	SET_WLPFN_AC_FLAG(flags)	(flags |= AUTO_CONNECT_MASK)

#define	NONEED_REPORTLOST		0xfe
#define STOP_REPORTLOST			0xff
enum {
	IOV_PFN_SET_PARAM,
	IOV_PFN_CFG,
	IOV_PFN_ADD,
	IOV_PFN_ADD_BSSID,
	IOV_PFN,
	IOV_PFN_CLEAR,
	IOV_PFN_BEST,
	IOV_PFN_SUSPEND,
	IOV_PFN_LBEST,
	IOV_PFN_MEM,
	IOV_PFN_RTTN,
	IOV_PFN_LAST
};

typedef enum tagPFNNETSTATE {
	PFN_NET_DISASSOCIATED,
	PFN_NET_ASSOCIATED
} PFNNETSTATE;

typedef enum tagPFNSCANSTATE {
	PFN_SCAN_DISABLED,
	PFN_SCAN_IDLE,
	PFN_SCAN_INPROGRESS
} PFNSCANSTATE;

typedef struct wl_pfn_bestinfo {
	wl_pfn_subnet_info_t pfnsubnet;
	uint16	flags;
	int16	RSSI; /* receive signal strength (in dBm) */
	uint32	timestamp;	/* laps in mseconds */
} wl_pfn_bestinfo_t;

typedef struct wl_pfn_bestnet {
	struct wl_pfn_bestnet *next;
	wl_pfn_bestinfo_t bestnetinfo[1]; /* bestn number of this */
} wl_pfn_bestnet_t;

#define PFN_NET_NOT_FOUND       0x1
#define PFN_NET_JUST_FOUND      0x2
#define PFN_NET_ALREADY_FOUND   0x4
#define PFN_NET_JUST_LOST       0x8

typedef struct wl_pfn_bssidinfo {
	uint8		network_found;
	uint8		flags;
	uint16		channel;
	int16		rssi;
	/* updated to the current system time in milisec upon finding the network */
	uint32		time_stamp;
	wlc_ssid_t	ssid;
} wl_pfn_bssidinfo_t;

typedef struct wl_pfn_bssid_list {
	/* link to next node */
	struct wl_pfn_bssid_list	*next;
	struct  ether_addr		bssid;
	/* bit4: suppress_lost, bit3: suppress_found */
	uint16				flags;
	wl_pfn_bssidinfo_t		*pbssidinfo;
} wl_pfn_bssid_list_t;

/* structure defination */
struct wl_pfn_bdcast_list {
	/* link to next node */
	struct wl_pfn_bdcast_list	* next;
	struct  ether_addr		bssid;
	wl_pfn_bssidinfo_t		bssidinfo;
};

typedef struct wl_pfn_internal {
	struct  ether_addr	bssid;
	uint8			network_found;
	/* True if network was found during broad cast scan */
	uint8			flags;
	uint16			channel;
	int16			rssi;
	/* updated to the current system time in milisec upon finding the network */
	uint32			time_stamp;
	/* PFN data sent by user */
	wl_pfn_t		pfn;
#if defined(BCMROMBUILD) || defined(BCMROMOFFLOAD)
	void			*pad;
#endif // endif
} wl_pfn_internal_t;

#define BSSIDLIST_MAX	16
#define REPEAT_MAX	100
#define EXP_MAX		5
#define WLC_BESTN_TUNABLE(wlc)		(wlc)->pub->tunables->maxbestn
#define WLC_MSCAN_TUNABLE(wlc)		(wlc)->pub->tunables->maxmscan

/* bit map in pfn internal flag */
#define SUSPEND_BIT		0
#define PROHIBITED_BIT		1
struct wl_pfn_info {
	/* Broadcast or directed scan */
	int16			cur_scan_type;
	uint16			bdcastnum;
	/* Link list to hold network found as part of broadcast and not from PFN list */
	wl_pfn_bdcast_list_t	* bdcast_list;
	/* PFN task timer */
	struct wl_timer		* p_pfn_timer;
	wlc_info_t		* wlc;
	/* Number of SSID based PFN network registered by user */
	int16			count;
	/* Pointer array for SSID based pfn network private data */
	wl_pfn_internal_t	**pfn_internal;
	/* Current index into the PFN list where scanning in progress */
	int16			cur_scan_index;
	/* Index to PFN array of associated network */
	int16			associated_idx;
	/* Current state of scan state machine */
	PFNSCANSTATE		pfn_scan_state;
	/* Current state of network state */
	PFNNETSTATE		pfn_network_state;
	/* PFN parameter data sent by user */
	wl_pfn_param_t		*param;
	/* SSID arrays for directed scanning */
	wlc_ssid_t		*ssid_list;
    uint8			nbss, nibss;
	uint32			ssidlist_len;
	/* pointer to memory holding bestnet for current scan */
	wl_pfn_bestnet_t	*current_bestn;
	/* best networks in history */
	wl_pfn_bestnet_t	*bestnethdr;
	wl_pfn_bestnet_t	*bestnettail;
	/* number of scan in bestnet */
	uint16			numofscan;
	/* count of BSSID based PFN networks */
	uint16			bssidNum;
	/* Pointer array for BSSID based PFN network */
	wl_pfn_bssid_list_t	*bssid_list[BSSIDLIST_MAX];
	/* counter for adaptive scanning */
	uint8			adaptcnt;
	/* number of found and lost networks in each scan */
	uint8			foundcnt;
	uint8			lostcnt;
	/* channel spec list */
	int8			chanspec_count;
	chanspec_t		chanspec_list[WL_NUMCHANNELS];
	/* size fo bestn bestnetinfo in bestnet */
	uint16			bestnetsize;
	/* number of hidden networks */
	uint8			hiddencnt;
	uint8			currentadapt;
	uint32			reporttype;
	uint8			availbsscnt;
	uint8			none_cnt;
	uint8			reportloss;
	/* internal flag: bit 0 (suspend/resume) */
	uint16			intflag;
	int32			slow_freq; /* slow scan period */
#ifdef NLO
	/* saved suppress ssid setting */
	bool			suppress_ssid_saved;
#endif /* NLO */
	uint8					foundcnt_ssid;
	uint8					lostcnt_ssid;
	uint8					foundcnt_bssid;
	uint8					lostcnt_bssid;
	uint8					rttn;
};

#define	BSSID_RPTLOSS_BIT	0
#define	SSID_RPTLOSS_BIT	1
#define	BSSID_RPTLOSS_MASK	1
#define	SSID_RPTLOSS_MASK	2

/* Local function prototypes */
static int wl_pfn_set_params(wl_pfn_info_t * pfn_info, void * buf);
static int wl_pfn_enable(wl_pfn_info_t * pfn_info, int enable);
static int wl_pfn_best(wl_pfn_info_t * pfn_info, void * buf, int len);
static int wl_pfn_lbest(wl_pfn_info_t * pfn_info, void * buf, int len);
static void wl_pfn_scan_complete(void * arg, int status, wlc_bsscfg_t *cfg);
static void wl_pfn_timer(void * arg);
static int wl_pfn_start_scan(wl_pfn_info_t * pfn_info, wlc_ssid_t	* ssid,
                              int nssid, int bss_type);
static wl_pfn_bestinfo_t* wl_pfn_get_best_networks(wl_pfn_info_t *pfn_info,
	wl_pfn_bestinfo_t *bestap_ptr, int16 rssi, struct ether_addr bssid);
static void wl_pfn_updatenet(wl_pfn_info_t *pfn_info, wl_pfn_net_info_t *foundptr,
	wl_pfn_net_info_t *lostptr, uint8 foundcnt, uint8 lostcnt, uint8 reportnet);
static void wl_pfn_attachbestn(wl_pfn_info_t * pfn_info, bool partial);

/* Rommable function prototypes */
static int wl_pfn_allocate_bdcast_node(wl_pfn_info_t * pfn_info,
	uint8 ssid_len, uint8 * ssid, uint32 now);
static int wl_pfn_clear(wl_pfn_info_t * pfn_info);
static int wl_pfn_validate_set_params(wl_pfn_info_t* pfn_info, void* buf);
static int wl_pfn_add(wl_pfn_info_t * pfn_info, void * buf, int len);
static int wl_pfn_add_bssid(wl_pfn_info_t * pfn_info, void * buf, int len);
static int wl_pfn_cfg(wl_pfn_info_t * pfn_info, void * buf, int len);
static void wl_pfn_free_bdcast_list(wl_pfn_info_t * pfn_info);
static void wl_pfn_prepare_for_scan(wl_pfn_info_t* pfn_info,
	wlc_ssid_t** ssid, int* nssid, int32* bss_type);
static void wl_pfn_ageing(wl_pfn_info_t * pfn_info, uint32 now);
static int wl_pfn_send_event(wl_pfn_info_t * pfn_info,
	void *data, uint32 datalen, int event_type);
void wl_pfn_process_scan_result(wl_pfn_info_t * pfn_info, wlc_bss_info_t * bi);
static void wl_pfn_cipher2wsec(uint8 ucount, uint8 * unicast, uint32 * wsec);
static void wl_pfn_free_ssidlist(wl_pfn_info_t * pfn_info, bool freeall, bool clearjustfoundonly);
static void wl_pfn_free_bssidlist(wl_pfn_info_t * pfn_info, bool freeall, bool clearjustfoundonly);
static int wl_pfn_setscanlist(wl_pfn_info_t *pfn_info, int enable);
static int wl_pfn_updatecfg(wl_pfn_info_t * pfn_info, wl_pfn_cfg_t * pcfg);
static int wl_pfn_suspend(wl_pfn_info_t * pfn_info, int suspend);

/* IOVar table */
static const bcm_iovar_t wl_pfn_iovars[] = {
	{"pfn_set", IOV_PFN_SET_PARAM,
#if defined(WLC_PATCH) && defined(BCM43362A2)
	(0), IOVT_BUFFER, sizeof(wl_pfn_param_t)
#else
	(0), IOVT_BUFFER, OFFSETOF(wl_pfn_param_t, slow_freq)
#endif // endif
	},
	{"pfn_cfg", IOV_PFN_CFG,
	(0), IOVT_BUFFER, OFFSETOF(wl_pfn_cfg_t, flags)
	},
	{"pfn_add", IOV_PFN_ADD,
	(0), IOVT_BUFFER, sizeof(wl_pfn_t)
	},
	{"pfn_add_bssid", IOV_PFN_ADD_BSSID,
	(0), IOVT_BUFFER, sizeof(wl_pfn_bssid_t)
	},
	{"pfn", IOV_PFN,
	(0), IOVT_BOOL, 0
	},
	{"pfnclear", IOV_PFN_CLEAR,
	(0), IOVT_VOID, 0
	},
	{"pfnbest", IOV_PFN_BEST,
	(0), IOVT_BUFFER, sizeof(wl_pfn_scanresults_t)
	},
	{"pfn_suspend", IOV_PFN_SUSPEND,
	(0), IOVT_BOOL, 0
	},
	{"pfnlbest", IOV_PFN_LBEST,
	(0), IOVT_BUFFER, sizeof(wl_pfn_lscanresults_t)
	},
	{"pfnmem", IOV_PFN_MEM,
	(0), IOVT_BUFFER, 0
	},
	{"pfnrttn", IOV_PFN_RTTN,
	(0), IOVT_BUFFER, 0
	},
	{NULL, 0, 0, 0, 0 }
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

static int
wl_pfn_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wl_pfn_info_t *pfn_info = (wl_pfn_info_t *)hdl;
	int enable, suspend, rtt;
	int err = BCME_UNSUPPORTED;
	int32 *ret_int_ptr = (int32*)arg;
	int bestn, maxmscan;

	ASSERT(pfn_info);

	switch (actionid) {
	case IOV_SVAL(IOV_PFN_SET_PARAM):
		err = wl_pfn_set_params(pfn_info, arg);
		break;
	case IOV_SVAL(IOV_PFN_CFG):
		err = wl_pfn_cfg(pfn_info, arg, len);
		break;
	case IOV_SVAL(IOV_PFN_ADD):
		err = wl_pfn_add(pfn_info, arg, len);
		break;
	case IOV_SVAL(IOV_PFN_ADD_BSSID):
		err = wl_pfn_add_bssid(pfn_info, arg, len);
		break;
	case IOV_SVAL(IOV_PFN):
		bcopy(arg, &enable, sizeof(enable));
		err = wl_pfn_enable(pfn_info, enable);
		break;
	case IOV_GVAL(IOV_PFN):
		*ret_int_ptr = (int32)(PFN_SCAN_DISABLED != pfn_info->pfn_scan_state);
		err = BCME_OK;
		break;
	case IOV_SVAL(IOV_PFN_CLEAR):
		err = wl_pfn_clear(pfn_info);
		break;
	case IOV_GVAL(IOV_PFN_BEST):
		err = wl_pfn_best(pfn_info, arg, len);
		break;
	case IOV_SVAL(IOV_PFN_SUSPEND):
		bcopy(arg, &suspend, sizeof(suspend));
		err = wl_pfn_suspend(pfn_info, suspend);
		break;
	case IOV_GVAL(IOV_PFN_SUSPEND):
		*ret_int_ptr = ((pfn_info->intflag & (ENABLE << SUSPEND_BIT))? 1 : 0);
		err = BCME_OK;
		break;
	case IOV_GVAL(IOV_PFN_LBEST):
		err = wl_pfn_lbest(pfn_info, arg, len);
		break;
	case IOV_SVAL(IOV_PFN_MEM):
		/* Make sure PFN is not active already */
		if (PFN_SCAN_DISABLED != pfn_info->pfn_scan_state) {
			WL_ERROR(("wl%d: PFN is already active, can't service IOV_PFN_MEM\n",
				pfn_info->wlc->pub->unit));
			return BCME_EPERM;
		}
		bcopy(arg, &bestn, sizeof(bestn));
		if (bestn > 0 && bestn <= WLC_BESTN_TUNABLE(pfn_info->wlc)) {
			pfn_info->param->bestn = (uint8)bestn;
			pfn_info->bestnetsize = sizeof(wl_pfn_bestnet_t) +
			    sizeof(wl_pfn_bestinfo_t) * (pfn_info->param->bestn - 1);
			err = BCME_OK;
		} else
			err = BCME_RANGE;
		break;
	case IOV_GVAL(IOV_PFN_MEM):
		if (pfn_info->bestnetsize) {
#ifdef DONGLEBUILD
			maxmscan = OSL_MEM_AVAIL() / pfn_info->bestnetsize;
			*ret_int_ptr = MIN(maxmscan, WLC_MSCAN_TUNABLE(pfn_info->wlc));
#else
			*ret_int_ptr = WLC_MSCAN_TUNABLE(pfn_info->wlc);
#endif // endif
			err = BCME_OK;
		} else {
			err = BCME_ERROR;
		}
		break;
	case IOV_SVAL(IOV_PFN_RTTN): /* set # of bestn that needs to report rtt */
		bcopy(arg, &rtt, sizeof(rtt));
		if (rtt > BESTN_MAX)
			err = BCME_RANGE;
		else {
			pfn_info->rttn = (uint8)*ret_int_ptr;
			err = BCME_OK;
		}
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;

	}

	return err;
}

wl_pfn_info_t *
BCMATTACHFN(wl_pfn_attach)(wlc_info_t * wlc)
{
	wl_pfn_info_t *pfn_info;

	/* Allocate for pfn data */
	if (!(pfn_info = (wl_pfn_info_t *) MALLOCZ(wlc->osh, sizeof(wl_pfn_info_t)))) {
		WL_ERROR(("wl%d: PFN: MALLOCZ failed, size = %d\n", wlc->pub->unit,
			sizeof(wl_pfn_info_t)));
		goto error;
	}
	pfn_info->wlc = wlc;
	wlc->pfn = pfn_info;

	/* Allocate resource for timer, but don't start it */
	pfn_info->p_pfn_timer = wl_init_timer(pfn_info->wlc->wl,
		wl_pfn_timer, pfn_info, "pfn");
	if (!pfn_info->p_pfn_timer) {
		WL_ERROR(("wl%d: Failed to allocate resoure for timer\n", wlc->pub->unit));
		goto error;
	}

	if (!(pfn_info->pfn_internal =
	    (wl_pfn_internal_t **)MALLOCZ(wlc->osh,
	    sizeof(wl_pfn_internal_t *) * MAX_PFN_LIST_COUNT))) {
		WL_ERROR(("wl%d: PFN: MALLOC failed, size = %d\n", wlc->pub->unit,
			sizeof(wl_pfn_internal_t *) * MAX_PFN_LIST_COUNT));
		goto error;
	}

	if (!(pfn_info->param =
	    (wl_pfn_param_t *)MALLOCZ(wlc->osh, sizeof(wl_pfn_param_t)))) {
		WL_ERROR(("wl%d: PFN: MALLOC failed, size = %d\n", wlc->pub->unit,
			sizeof(wl_pfn_param_t)));
		goto error;
	}

	/* register a module (to handle iovars) */
	if (wlc_module_register(wlc->pub, wl_pfn_iovars, "wl_rte_iovars", pfn_info,
		wl_pfn_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: Error registering pfn iovar\n", wlc->pub->unit));
		goto error;
	}

	wlc->pub->_wlpfn = TRUE;
	return pfn_info;

error:
	wl_pfn_detach(pfn_info);

	return 0;
}

int
BCMATTACHFN(wl_pfn_detach)(wl_pfn_info_t * pfn_info)
{
	int callbacks = 0;

	/* Clean up memory */
	if (pfn_info) {

		/* Cancel the scanning timer */
		if (pfn_info->p_pfn_timer) {
			if (!wl_del_timer(pfn_info->wlc->wl, pfn_info->p_pfn_timer))
				callbacks++;

			wl_free_timer(pfn_info->wlc->wl, pfn_info->p_pfn_timer);
		}

		/* Disable before calling 'wl_pfn_clear' */
		pfn_info->pfn_scan_state = PFN_SCAN_DISABLED;

		/* Free ssid list for directed scan */
		if (pfn_info->ssid_list != NULL) {
			MFREE(pfn_info->wlc->osh, pfn_info->ssid_list,
			      pfn_info->ssidlist_len);
			pfn_info->ssid_list = NULL;
		}

		wl_pfn_clear(pfn_info);

		wlc_module_unregister(pfn_info->wlc->pub, "wl_rte_iovars", pfn_info);

		MFREE(pfn_info->wlc->osh, pfn_info->pfn_internal,
		      sizeof(wl_pfn_internal_t *) * MAX_PFN_LIST_COUNT);
		MFREE(pfn_info->wlc->osh, pfn_info->param, sizeof(wl_pfn_param_t));
		MFREE(pfn_info->wlc->osh, pfn_info, sizeof(wl_pfn_info_t));
		pfn_info = NULL;
	}

	return callbacks;
}

static int
wl_pfn_validate_set_params(wl_pfn_info_t* pfn_info, void* buf)
{
	/* Make sure PFN is not active already */
	if (PFN_SCAN_DISABLED != pfn_info->pfn_scan_state) {
		WL_ERROR(("wl%d: PFN is already active, can't service IOV_PFN_SET\n",
			pfn_info->wlc->pub->unit));
		return BCME_EPERM;
	}

	/* Copy the user/host data into internal structure */
	bcopy(buf, pfn_info->param, sizeof(wl_pfn_param_t));

	/* Make sure we have valid current version */
	if (PFN_VERSION != pfn_info->param->version) {
		WL_ERROR(("wl%d: Incorrect version expected %d, found %d\n",
			pfn_info->wlc->pub->unit, PFN_VERSION, pfn_info->param->version));
		return BCME_VERSION;
	}

	/* Assign default values for user parameter if necessary */
	/* Default is derived based on bzero ing of the structure from the user */
	if (!pfn_info->param->scan_freq)
		pfn_info->param->scan_freq = DEFAULT_SCAN_FREQ;

	/* Convert from sec to ms */
	pfn_info->param->scan_freq *= 1000;

	if (!pfn_info->param->lost_network_timeout)
		pfn_info->param->lost_network_timeout = DEFAULT_LOST_DURATION;
	/* Convert from sec to ms */
	if (pfn_info->param->lost_network_timeout != -1)
		pfn_info->param->lost_network_timeout *= 1000;

#if defined(WLC_PATCH) && defined(BCM43362A2)
	bcopy(buf + sizeof(wl_pfn_param_t), &pfn_info->slow_freq, sizeof(pfn_info->slow_freq));
	pfn_info->slow_freq *= 1000;
#else
	pfn_info->param->slow_freq *= 1000;
#endif /* WLC_PATCH && BCM43362A2 */

	if (!pfn_info->param->mscan)
		pfn_info->param->mscan = DEFAULT_MSCAN;
	else if (pfn_info->param->mscan > WLC_MSCAN_TUNABLE(pfn_info->wlc))
		return BCME_RANGE;

	if (!pfn_info->param->bestn)
		pfn_info->param->bestn = DEFAULT_BESTN;
	else if (pfn_info->param->bestn > WLC_BESTN_TUNABLE(pfn_info->wlc))
		return BCME_RANGE;

	if (pfn_info->param->mscan)
		pfn_info->bestnetsize = sizeof(wl_pfn_bestnet_t) +
		    sizeof(wl_pfn_bestinfo_t) * (pfn_info->param->bestn - 1);
	else
		pfn_info->bestnetsize = 0;

	if (!pfn_info->param->repeat)
		pfn_info->param->repeat = DEFAULT_REPEAT;
	else if (pfn_info->param->repeat > REPEAT_MAX)
		return BCME_RANGE;

	if (!pfn_info->param->exp)
		pfn_info->param->exp = DEFAULT_EXP;
	else if (pfn_info->param->exp > EXP_MAX)
		return BCME_RANGE;

	return BCME_OK;
}

static int
wl_pfn_set_params(wl_pfn_info_t * pfn_info, void * buf)
{
	int err;

	if ((err = wl_pfn_validate_set_params(pfn_info, buf)) != BCME_OK)
		return err;

	/* if no attempt to enable autoconnect, we're done */
	if ((pfn_info->param->flags & AUTO_CONNECT_MASK) == 0)
		return BCME_OK;
	CLEAR_WLPFN_AC_FLAG(pfn_info->param->flags);
	return BCME_BADARG;
}

static int
wl_pfn_add(wl_pfn_info_t * pfn_info, void * buf, int len)
{
	wl_pfn_internal_t   * pfn_internal;
	wl_pfn_t            *pfn_ssidnet;

	/* Make sure PFN is not active already */
	if (PFN_SCAN_DISABLED != pfn_info->pfn_scan_state) {
		WL_ERROR(("wl%d: PFN is already active, can't service IOV_PFN_ADD\n",
			pfn_info->wlc->pub->unit));
		return BCME_EPERM;
	}

	ASSERT(!((uintptr)buf & 0x3));
	pfn_ssidnet = (wl_pfn_t *)buf;
	while (len >= sizeof(wl_pfn_t)) {
		/* Check for max pfn element allowed */
		if (pfn_info->count >= MAX_PFN_LIST_COUNT) {
			WL_ERROR(("wl%d: PFN element count %d exceeded limit of %d\n",
			   pfn_info->wlc->pub->unit, pfn_info->count + 1, MAX_PFN_LIST_COUNT));
			return BCME_RANGE;
		}

		/* Allocate memory for pfn internal structure type 'wl_pfn_internal_t' */
		if (!(pfn_internal = (wl_pfn_internal_t *)MALLOCZ(pfn_info->wlc->osh,
		                      sizeof(wl_pfn_internal_t)))) {
			WL_ERROR(("wl%d: PFN MALLOC failed, size = %d\n",
			          pfn_info->wlc->pub->unit, sizeof(wl_pfn_internal_t)));
			return BCME_NOMEM;
		}

		/* Copy the user/host data into intenal structre */
		bcopy(pfn_ssidnet, &pfn_internal->pfn, sizeof(wl_pfn_t));
		/* set network_found to NOT_FOUND */
		pfn_internal->network_found = PFN_NET_NOT_FOUND;
		pfn_ssidnet++;
		len -= sizeof(wl_pfn_t);

		if (pfn_internal->pfn.flags & WL_PFN_HIDDEN_MASK)
			pfn_info->hiddencnt++;

		if (!(pfn_internal->pfn.flags & WL_PFN_SUPPRESSLOST_MASK))
			pfn_info->reportloss |= ENABLE << SSID_RPTLOSS_BIT;

		/* Store the pointer to the block and increment the count */
		pfn_info->pfn_internal[pfn_info->count++] = pfn_internal;
	}
	ASSERT(!len);

	return BCME_OK;
}

static int
wl_pfn_add_bssid(wl_pfn_info_t * pfn_info, void * buf, int len)
{
	wl_pfn_bssid_t      *bssidptr;
	wl_pfn_bssid_list_t * bssid_list_ptr;
	int                 index;

	/* Make sure PFN is not active already */
	if (PFN_SCAN_DISABLED != pfn_info->pfn_scan_state) {
		WL_ERROR(("wl%d: PFN is already active, can't service IOV_PFN_ADD_BSSID\n",
			pfn_info->wlc->pub->unit));
		return BCME_EPERM;
	}

	/* make sure alignment of 2 */
	ASSERT(!((uintptr)buf & 0x1));
	bssidptr = (wl_pfn_bssid_t *)buf;
	while (len >= sizeof (wl_pfn_bssid_t)) {
		index = bssidptr->macaddr.octet[ETHER_ADDR_LEN - 1] & 0xf;
		/* create a new node in bssid_list for new BSSID */
		if (pfn_info->bssidNum >= MAX_BSSID_LIST) {
			WL_ERROR(("wl%d: BSSID count %d exceeded limit of %d\n",
				pfn_info->wlc->pub->unit, pfn_info->bssidNum + 1, MAX_BSSID_LIST));
			return BCME_RANGE;
		}

		if (!(bssid_list_ptr = (wl_pfn_bssid_list_t *)MALLOCZ(pfn_info->wlc->osh,
		                        sizeof(wl_pfn_bssid_list_t)))) {
			WL_ERROR(("wl%d: malloc failed, size = %d\n",
			   pfn_info->wlc->pub->unit, sizeof(wl_pfn_bssid_list_t)));
			return BCME_NOMEM;
		}
		bcopy(bssidptr->macaddr.octet, bssid_list_ptr->bssid.octet, ETHER_ADDR_LEN);
		bssid_list_ptr->flags = bssidptr->flags;
		if (!(bssid_list_ptr->flags & WL_PFN_SUPPRESSLOST_MASK))
			pfn_info->reportloss |= ENABLE << BSSID_RPTLOSS_BIT;

		bssid_list_ptr->next = pfn_info->bssid_list[index];
		pfn_info->bssid_list[index] = bssid_list_ptr;
		pfn_info->bssidNum++;
		bssidptr++;
		len -= sizeof(wl_pfn_bssid_t);
	}
	ASSERT(!len);

	return BCME_OK;
}

static int
wl_pfn_updatecfg(wl_pfn_info_t * pfn_info, wl_pfn_cfg_t * pcfg)
{
	uint16          *ptr;
	int             i, j;
	uint16		chspec;

	if (pcfg->channel_num >= WL_NUMCHANNELS)
		return BCME_RANGE;

	/* validate channel list */
	for (i = 0, ptr = pcfg->channel_list; i < pcfg->channel_num; i++, ptr++) {
		if (pcfg->flags & WL_PFN_CFG_FLAGS_PROHIBITED) {
			chspec = *ptr;
			if (wf_chspec_valid(chspec) == FALSE)
				return BCME_BADARG;
		} else if (!wlc_valid_chanspec_db(pfn_info->wlc->cmi, CH20MHZ_CHSPEC(*ptr))) {
			return BCME_BADARG;
		}
	}

	bzero(pfn_info->chanspec_list, sizeof(chanspec_t) * WL_NUMCHANNELS);
	pfn_info->chanspec_count = 0;
	if (pcfg->channel_num) {
		for (i = 0, ptr = pcfg->channel_list;
			i < pcfg->channel_num; i++, ptr++) {
			for (j = 0; j < pfn_info->chanspec_count; j++) {
				if (*ptr < wf_chspec_ctlchan(pfn_info->chanspec_list[j])) {
					/* insert new channel in the middle */
					memmove(&pfn_info->chanspec_list[j + 1],
					        &pfn_info->chanspec_list[j],
					    (pfn_info->chanspec_count - j) * sizeof(chanspec_t));
					pfn_info->chanspec_list[j] = CH20MHZ_CHSPEC(*ptr);
					pfn_info->chanspec_count++;
					break;
				} else if (*ptr == wf_chspec_ctlchan(pfn_info->chanspec_list[j])) {
					break;
				}
			}

			if (j == pfn_info->chanspec_count) {
				/* add new channel at the end */
				pfn_info->chanspec_list[j] = CH20MHZ_CHSPEC(*ptr);
				pfn_info->chanspec_count++;
			}
		}
	}

	/* Channel list accepted and updated; accept the prohibited flag from user too */
	pfn_info->intflag &= ~(ENABLE << PROHIBITED_BIT);
	if (pcfg->flags & WL_PFN_CFG_FLAGS_PROHIBITED) {
		pfn_info->intflag |= (ENABLE << PROHIBITED_BIT);
	}

	return 0;
}

static int
wl_pfn_cfg(wl_pfn_info_t * pfn_info, void * buf, int len)
{
	wl_pfn_cfg_t        pfncfg;
	int                 err;
	uint8               cur_bsscnt;

	/* Local pfncfg for size adjustment, compatibility */
	memset(&pfncfg, 0, sizeof(wl_pfn_cfg_t));
	bcopy(buf, &pfncfg, MIN(len, sizeof(wl_pfn_cfg_t)));

	if ((err = wl_pfn_updatecfg(pfn_info, &pfncfg)))
		return err;

	if (PFN_SCAN_DISABLED != pfn_info->pfn_scan_state) {
		/* Stop the scanning timer */
		wl_del_timer(pfn_info->wlc->wl, pfn_info->p_pfn_timer);
		/* Abort any pending PFN scan */
		if (pfn_info->pfn_scan_state == PFN_SCAN_INPROGRESS) {
			wlc_scan_abort(pfn_info->wlc->scan, WLC_E_STATUS_ABORT);
		}
		/* free ssid scan list */
		if ((err = wl_pfn_setscanlist(pfn_info, FALSE)))
			return err;

		cur_bsscnt = pfn_info->availbsscnt;
		/* reset ssidlist or bssidlist according to new report type in pfncfg */
		if (pfncfg.reporttype == WL_PFN_REPORT_SSIDNET) {
			wl_pfn_free_ssidlist(pfn_info, FALSE, TRUE);
			wl_pfn_free_bssidlist(pfn_info, FALSE, FALSE);
			wl_pfn_free_bdcast_list(pfn_info);
		} else if (pfncfg.reporttype == WL_PFN_REPORT_BSSIDNET) {
			wl_pfn_free_bssidlist(pfn_info, FALSE, TRUE);
			wl_pfn_free_ssidlist(pfn_info, FALSE, FALSE);
			wl_pfn_free_bdcast_list(pfn_info);
		} else {
			wl_pfn_free_ssidlist(pfn_info, FALSE, TRUE);
			wl_pfn_free_bssidlist(pfn_info, FALSE, TRUE);
		}
		/* reset foundcnt, lostcnt for new scan */
		ASSERT(!pfn_info->lostcnt);
		pfn_info->foundcnt = 0;
		pfn_info->foundcnt_ssid = 0;
		pfn_info->foundcnt_bssid = 0;
		/* set non_cnt according to various condition */
		if (pfn_info->reporttype != pfncfg.reporttype) {
			if ((pfncfg.reporttype == WL_PFN_REPORT_ALLNET &&
				!pfn_info->reportloss) ||
				((pfncfg.reporttype == WL_PFN_REPORT_BSSIDNET) &&
				!(pfn_info->reportloss & BSSID_RPTLOSS_MASK)) ||
				((pfncfg.reporttype == WL_PFN_REPORT_SSIDNET) &&
				!(pfn_info->reportloss & SSID_RPTLOSS_MASK))) {
				pfn_info->none_cnt = NONEED_REPORTLOST;
			} else if ((pfncfg.reporttype && pfn_info->reporttype) ||
			           ((pfncfg.reporttype == WL_PFN_REPORT_ALLNET) &&
			            (pfn_info->reporttype == WL_PFN_REPORT_SSIDNET) &&
			            (pfn_info->reportloss & BSSID_RPTLOSS_MASK)) ||
			           ((pfncfg.reporttype == WL_PFN_REPORT_ALLNET) &&
			            (pfn_info->reporttype == WL_PFN_REPORT_BSSIDNET) &&
			            (pfn_info->reportloss & SSID_RPTLOSS_MASK)) ||
			           ((pfn_info->reporttype == WL_PFN_REPORT_ALLNET) &&
			            (cur_bsscnt > pfn_info->availbsscnt))) {
				pfn_info->none_cnt = 0;
			}
			pfn_info->reporttype = pfncfg.reporttype;
		}

		/* setup ssid scan list */
		if ((err = wl_pfn_setscanlist(pfn_info, TRUE)))
			return err;
		/* kick off scan immediately */
		pfn_info->pfn_scan_state = PFN_SCAN_IDLE;
		wl_pfn_timer(pfn_info);
		/* Start the scanning timer */
		wl_add_timer(pfn_info->wlc->wl, pfn_info->p_pfn_timer,
			pfn_info->param->scan_freq, TRUE);
	} else {
		pfn_info->reporttype = pfncfg.reporttype;
	}

	return 0;
}

static int
wl_pfn_suspend(wl_pfn_info_t *pfn_info, int suspend)
{
	if (suspend) {
		pfn_info->intflag |= ENABLE << SUSPEND_BIT;
		/* Abort any pending PFN scan */
		if (pfn_info->pfn_scan_state == PFN_SCAN_INPROGRESS)
			wlc_scan_abort(pfn_info->wlc->scan, WLC_E_STATUS_SUPPRESS);
	} else {
		if (!(pfn_info->intflag & (ENABLE << SUSPEND_BIT)))
			return BCME_OK;

		pfn_info->intflag &= ~(ENABLE << SUSPEND_BIT);
		if (PFN_SCAN_DISABLED == pfn_info->pfn_scan_state)
			return BCME_OK;

		/* kick off scan immediately */
		pfn_info->pfn_scan_state = PFN_SCAN_IDLE;
		wl_pfn_timer(pfn_info);
	}

	return BCME_OK;
}

static int
wl_pfn_best(wl_pfn_info_t * pfn_info, void * buf, int len)
{
	wl_pfn_scanresults_t *pfnbest;
	wl_pfn_net_info_t *pfnbestnet;
	wl_pfn_bestinfo_t *bestnetinfo;
	wl_pfn_bestnet_t *ptr;
	uint32 now = OSL_SYSUPTIME();
	int i, buflen = len;
	int scancnt, cnt;

	if (PFN_VERSION != pfn_info->param->version)
		return BCME_EPERM;

	/* Make sure PFN is not in-progress */
	if (PFN_SCAN_IDLE != pfn_info->pfn_scan_state) {
		WL_ERROR(("wl%d: report pfnbest only during PFN_SCAN_IDLE\n",
			pfn_info->wlc->pub->unit));
		return BCME_EPERM;
	}

	/* make sure alignment of 4 */
	ASSERT(!((uintptr)buf & 0x3));

	pfnbest = (wl_pfn_scanresults_t *)buf;
	pfnbest->version = PFN_SCANRESULT_VERSION;
	pfnbest->count = 0;
	pfnbestnet = pfnbest->netinfo;
	/* pre-calculate until which scan result this report is going to end */
	buflen -= sizeof(wl_pfn_scanresults_t) - sizeof(wl_pfn_net_info_t);
	scancnt = buflen / (sizeof(wl_pfn_net_info_t) * pfn_info->param->bestn);
	cnt = 0;
	while (cnt < scancnt && pfn_info->bestnethdr) {
		bestnetinfo = pfn_info->bestnethdr->bestnetinfo;
		/* get the bestn from one scan */
		for (i = 0; i < pfn_info->param->bestn; i++) {
			if (bestnetinfo->pfnsubnet.SSID_len ||
				!ETHER_ISNULLADDR(&bestnetinfo->pfnsubnet.BSSID)) {
				bcopy(bestnetinfo, pfnbestnet, sizeof(wl_pfn_subnet_info_t));
				pfnbestnet->RSSI = bestnetinfo->RSSI;
				/* elapsed time, msecond to second */
				pfnbestnet->timestamp = (uint16)((now - bestnetinfo->timestamp)
				                        / 1000);
				pfnbest->count++;
				pfnbestnet++;
				buflen -= sizeof(wl_pfn_net_info_t);
				ASSERT(buflen >= sizeof(wl_pfn_net_info_t));
			}
			bestnetinfo++;
		}
		ptr = pfn_info->bestnethdr;
		pfn_info->bestnethdr = pfn_info->bestnethdr->next;
		MFREE(pfn_info->wlc->osh, ptr, pfn_info->bestnetsize);
		ASSERT(pfn_info->numofscan);
		pfn_info->numofscan--;
		cnt++;
	}
	ASSERT(buflen >= 0);
	if (!pfn_info->bestnethdr) {
		pfnbest->status = PFN_COMPLETE;
		pfn_info->bestnettail = NULL;
	} else {
		pfnbest->status = PFN_INCOMPLETE;
	}
	return 0;
}

static int
wl_pfn_lbest(wl_pfn_info_t * pfn_info, void * buf, int len)
{
	wl_pfn_lscanresults_t *pfnbest;
	wl_pfn_lnet_info_t *pfnbestnet;
	wl_pfn_bestinfo_t *bestnetinfo;
	wl_pfn_bestnet_t *ptr;
	uint32 now = OSL_SYSUPTIME();
	int i, buflen = len;
	int scancnt, cnt;
	bool partial = FALSE;

	if (PFN_VERSION != pfn_info->param->version)
		return BCME_EPERM;

	/* Make sure PFN is not in-progress */
	if (PFN_SCAN_IDLE != pfn_info->pfn_scan_state) {
		WL_ERROR(("wl%d: report pfnbest only during PFN_SCAN_IDLE\n",
			pfn_info->wlc->pub->unit));
		return BCME_EPERM;
	}

	/* make sure alignment of 4 */
	ASSERT(!((uintptr)buf & 0x3));

	pfnbest = (wl_pfn_lscanresults_t *)buf;
	pfnbest->version = PFN_SCANRESULT_VERSION;
	pfnbest->count = 0;
	pfnbestnet = pfnbest->netinfo;
	/* pre-calculate until which scan result this report is going to end */
	buflen -= sizeof(wl_pfn_lscanresults_t) - sizeof(wl_pfn_lnet_info_t);
	scancnt = buflen / (sizeof(wl_pfn_lnet_info_t) * pfn_info->param->bestn);
	cnt = 0;
	while (cnt < scancnt && pfn_info->bestnethdr) {
		bestnetinfo = pfn_info->bestnethdr->bestnetinfo;
		if (bestnetinfo->flags & PFN_PARTIAL_SCAN_MASK)
			partial = TRUE;
		else
			partial = FALSE;
		/* get the bestn from one scan */
		for (i = 0; i < pfn_info->param->bestn; i++) {
			if (bestnetinfo->pfnsubnet.SSID_len ||
				!ETHER_ISNULLADDR(&bestnetinfo->pfnsubnet.BSSID)) {
				bcopy(bestnetinfo, pfnbestnet, sizeof(wl_pfn_subnet_info_t));
				pfnbestnet->RSSI = bestnetinfo->RSSI;
				if (partial == TRUE)
					pfnbestnet->flags |= 1 << PFN_PARTIAL_SCAN_BIT;
				else
					pfnbestnet->flags &= ~PFN_PARTIAL_SCAN_MASK;
				pfnbestnet->timestamp = (uint32)(now - bestnetinfo->timestamp);
				pfnbestnet->rtt0 = 0;
				pfnbestnet->rtt1 = 0;
				pfnbest->count++;
				pfnbestnet++;
				buflen -= sizeof(wl_pfn_lnet_info_t);
				ASSERT(buflen >= sizeof(wl_pfn_lnet_info_t));
			}
			bestnetinfo++;
		}
		ptr = pfn_info->bestnethdr;
		pfn_info->bestnethdr = pfn_info->bestnethdr->next;
		MFREE(pfn_info->wlc->osh, ptr, pfn_info->bestnetsize);
		ASSERT(pfn_info->numofscan);
		pfn_info->numofscan--;
		cnt++;
	}
	ASSERT(buflen >= 0);
	if (!pfn_info->bestnethdr) {
		pfnbest->status = PFN_COMPLETE;
		pfn_info->bestnettail = NULL;
	} else {
		pfnbest->status = PFN_INCOMPLETE;
	}
	return 0;
}

static void
wl_pfn_free_ssidlist(wl_pfn_info_t * pfn_info, bool freeall, bool clearjustfoundonly)
{
	int i;
	for (i = 0; i < pfn_info->count; i++) {
		if (freeall) {
			MFREE(pfn_info->wlc->osh, pfn_info->pfn_internal[i],
			      sizeof(wl_pfn_internal_t));
			pfn_info->pfn_internal[i] = NULL;
		} else {
			if (!clearjustfoundonly ||
			    (pfn_info->pfn_internal[i]->network_found ==
			    PFN_NET_JUST_FOUND)) {
				if (!(pfn_info->pfn_internal[i]->pfn.flags &
				      WL_PFN_SUPPRESSLOST_MASK) &&
				    (pfn_info->pfn_internal[i]->network_found &
				    (PFN_NET_JUST_FOUND | PFN_NET_ALREADY_FOUND))) {
					ASSERT(pfn_info->availbsscnt);
					pfn_info->availbsscnt--;
				}
				pfn_info->pfn_internal[i]->network_found = PFN_NET_NOT_FOUND;
				bzero(&pfn_info->pfn_internal[i]->bssid, ETHER_ADDR_LEN);
			}
		}
	}
	if (freeall)
		pfn_info->count = 0;
}

static void
wl_pfn_free_bssidlist(wl_pfn_info_t * pfn_info, bool freeall, bool clearjustfoundonly)
{
	int i;
	wl_pfn_bssid_list_t *bssid_list, *ptr;

	for (i = 0; i < BSSIDLIST_MAX; i++) {
		bssid_list = pfn_info->bssid_list[i];
		while (bssid_list) {
			if (bssid_list->pbssidinfo) {
				if (!clearjustfoundonly ||
				    (bssid_list->pbssidinfo->network_found ==
				      PFN_NET_JUST_FOUND)) {
					MFREE(pfn_info->wlc->osh, bssid_list->pbssidinfo,
					      sizeof(wl_pfn_bssidinfo_t));
					bssid_list->pbssidinfo = NULL;
					if (!(bssid_list->flags & WL_PFN_SUPPRESSLOST_MASK)) {
						ASSERT(pfn_info->availbsscnt);
						pfn_info->availbsscnt--;
					}
				}
			}
			ptr = bssid_list;
			bssid_list = bssid_list->next;
			if (freeall)
				MFREE(pfn_info->wlc->osh, ptr, sizeof(wl_pfn_bssid_list_t));
		}
		if (freeall)
			pfn_info->bssid_list[i] = NULL;
	}
	if (freeall)
		pfn_info->bssidNum = 0;
}

static void
wl_pfn_free_bestnet(wl_pfn_info_t * pfn_info)
{
	wl_pfn_bestnet_t *bestnet;

	/* clear bestnet */
	while (pfn_info->bestnethdr != pfn_info->bestnettail) {
		bestnet = pfn_info->bestnethdr;
		pfn_info->bestnethdr = pfn_info->bestnethdr->next;
		MFREE(pfn_info->wlc->osh, bestnet, pfn_info->bestnetsize);
	}
	if (pfn_info->bestnethdr) {
		MFREE(pfn_info->wlc->osh, pfn_info->bestnethdr, pfn_info->bestnetsize);
		pfn_info->bestnethdr = pfn_info->bestnettail = NULL;
	}
	pfn_info->numofscan = 0;
	/* clear current bestn */
	if (pfn_info->current_bestn) {
		MFREE(pfn_info->wlc->osh, pfn_info->current_bestn, pfn_info->bestnetsize);
		pfn_info->current_bestn = NULL;
	}
}

static int
wl_pfn_setscanlist(wl_pfn_info_t *pfn_info, int enable)
{
	int i, j;
	uint8 directcnt;

	if (enable) {
		/* find # of directed scans */
		if (pfn_info->reporttype == WL_PFN_REPORT_BSSIDNET)
			directcnt = 0;
		else
			directcnt = pfn_info->hiddencnt;

		ASSERT(!pfn_info->ssid_list);
		pfn_info->ssidlist_len = (directcnt + 1) * sizeof(wlc_ssid_t);
		if (!(pfn_info->ssid_list = MALLOC(pfn_info->wlc->osh,
		     pfn_info->ssidlist_len))) {
			WL_ERROR(("wl%d: ssid list allocation failed. %d bytes\n",
				pfn_info->wlc->pub->unit, (directcnt + 1) * sizeof(wlc_ssid_t)));
			pfn_info->ssidlist_len = 0;
			if (pfn_info->param->mscan && pfn_info->numofscan &&
				(pfn_info->param->flags & REPORT_SEPERATELY_MASK))
				wl_pfn_send_event(pfn_info, NULL, 0, WLC_E_PFN_BEST_BATCHING);
			return BCME_NOMEM;
		}
		/* copy hidden ssid to ssid list(s) */
		for (i = j = 0; i < pfn_info->count && j < directcnt; i++) {
			if (pfn_info->pfn_internal[i]->pfn.flags & WL_PFN_HIDDEN_MASK) {
				bcopy(&pfn_info->pfn_internal[i]->pfn.ssid,
					&pfn_info->ssid_list[j++], sizeof(wlc_ssid_t));
			}
		}
		/* add 0 length SSID to end of ssid list, if broadcast scan is needed */
		if ((pfn_info->hiddencnt < pfn_info->count) || pfn_info->bssidNum ||
			(pfn_info->param->flags & (ENABLE_BKGRD_SCAN_MASK | ENABLE_BD_SCAN_MASK))) {
			/* broadcast scan needed and used last */
			pfn_info->ssid_list[j].SSID_len = 0;
		}
	} else {
		/* Free ssid list for directed scan */
		if (pfn_info->ssid_list != NULL) {
			MFREE(pfn_info->wlc->osh, pfn_info->ssid_list,
			      pfn_info->ssidlist_len);
			pfn_info->ssid_list = NULL;
		}
	}

	return 0;
}

static int
wl_pfn_enable(wl_pfn_info_t * pfn_info, int enable)
{
	int err;

	/* If the request to enable make sure we have required data from user/host */
	if (enable && (PFN_SCAN_DISABLED == pfn_info->pfn_scan_state)) {

		/* Check to see if the user has provided correct set of parameters by checking
		* version field. Also 'pfn_info->count or pfn_info->bssidNum' being zero is
		* vaild parameter, if user wants to just turn on back ground scanning or
		* broadcast scan without PFN list
		*/
		if (!pfn_info->param->version ||
		    (!pfn_info->count && !pfn_info->bssidNum &&
		!(pfn_info->param->flags & (ENABLE_BD_SCAN_MASK | ENABLE_BKGRD_SCAN_MASK)))) {
			WL_ERROR(("wl%d: Incomplete parameter setting\n",
			           pfn_info->wlc->pub->unit));
			return BCME_BADOPTION;
		}
		/* current_bestn expected NULL */
		if (pfn_info->current_bestn)
			WL_ERROR(("current_bestn not empty!\n"));

		if ((err = wl_pfn_setscanlist(pfn_info, enable)))
			return err;

		if ((pfn_info->reporttype == WL_PFN_REPORT_ALLNET &&
			!pfn_info->reportloss) ||
			((pfn_info->reporttype == WL_PFN_REPORT_BSSIDNET) &&
			!(pfn_info->reportloss & BSSID_RPTLOSS_MASK)) ||
			((pfn_info->reporttype == WL_PFN_REPORT_SSIDNET) &&
			!(pfn_info->reportloss & SSID_RPTLOSS_MASK))) {
			pfn_info->none_cnt = NONEED_REPORTLOST;
		} else {
			pfn_info->none_cnt = 0;
		}

		/* Start the scanning timer */
		pfn_info->pfn_scan_state = PFN_SCAN_IDLE;
		wl_add_timer(pfn_info->wlc->wl, pfn_info->p_pfn_timer,
			pfn_info->param->scan_freq, TRUE);

		/* If immediate scan enabled,
		* kick off the first scan as soon as PFN is enabled,
		* rather than wait till first interval to elapse
		*/
		if (((pfn_info->param->flags & IMMEDIATE_SCAN_MASK) >>
			IMMEDIATE_SCAN_BIT) == ENABLE) {
			wl_pfn_timer(pfn_info);
		}
	}

	/* Handle request to disable */
	if (!enable && (PFN_SCAN_DISABLED != pfn_info->pfn_scan_state)) {
		/* Stop the scanning timer */
		wl_del_timer(pfn_info->wlc->wl, pfn_info->p_pfn_timer);
		/* Abort any pending PFN scan */
		if (pfn_info->pfn_scan_state == PFN_SCAN_INPROGRESS) {
			wlc_scan_abort(pfn_info->wlc->scan, WLC_E_STATUS_ABORT);
		}
		pfn_info->pfn_scan_state = PFN_SCAN_DISABLED;

		if ((err = wl_pfn_setscanlist(pfn_info, enable)))
			return err;

		/* free all bestnet related info */
		wl_pfn_free_bestnet(pfn_info);
		/* Reset all the found/lost networks */
		/* so that events are sent to host after pfn enable */
		wl_pfn_free_ssidlist(pfn_info, FALSE, FALSE);
		wl_pfn_free_bssidlist(pfn_info, FALSE, FALSE);
		wl_pfn_free_bdcast_list(pfn_info);
		/* reset none_cnt, available network count, found count, lost count */
		pfn_info->none_cnt = 0;
		pfn_info->availbsscnt = 0;
		pfn_info->foundcnt = 0;
		pfn_info->lostcnt = 0;
		pfn_info->foundcnt_ssid = 0;
		pfn_info->foundcnt_bssid = 0;
		pfn_info->lostcnt_ssid = 0;
		pfn_info->lostcnt_bssid = 0;
	}

	return BCME_OK;
}

static int
wl_pfn_clear(wl_pfn_info_t * pfn_info)
{
	int i;

	/* Make sure PFN is not active already */
	if (PFN_SCAN_DISABLED != pfn_info->pfn_scan_state) {
		WL_ERROR(("wl%d: PFN is already active, can't service IOV_PFN_CLEAR\n",
			pfn_info->wlc->pub->unit));
		return BCME_EPERM;
	}

	/* Free resources allocated during IOV_PFN_ADD */
	wl_pfn_free_ssidlist(pfn_info, TRUE, FALSE);
	wl_pfn_free_bssidlist(pfn_info, TRUE, FALSE);

	/* Free the link list of BSS for given ESS from PFN list */
	wl_pfn_free_bdcast_list(pfn_info);

	/* free all bestnet related info */
	wl_pfn_free_bestnet(pfn_info);

	/* Zero out the param */
	bzero(pfn_info->param, sizeof(wl_pfn_param_t));

	pfn_info->associated_idx = 0;
	/* Disable before calling 'wl_pfn_clear' */
	pfn_info->pfn_scan_state = PFN_SCAN_DISABLED;

	pfn_info->numofscan = 0;
	pfn_info->adaptcnt = 0;
	pfn_info->foundcnt = pfn_info->lostcnt = 0;
	pfn_info->foundcnt_ssid = pfn_info->lostcnt_ssid = 0;
	pfn_info->foundcnt_bssid = pfn_info->lostcnt_bssid = 0;
	/* clear channel spec list */
	for (i = 0; i < WL_NUMCHANNELS; i++) {
		pfn_info->chanspec_list [i] = 0;
	}
	pfn_info->chanspec_count = 0;
	pfn_info->bestnetsize = 0;
	pfn_info->hiddencnt = 0;
	pfn_info->currentadapt = 0;
	pfn_info->reporttype = 0;
	pfn_info->availbsscnt = 0;
	pfn_info->none_cnt = 0;
	pfn_info->reportloss = 0;

	return BCME_OK;
}

static void
wl_pfn_free_bdcast_list(wl_pfn_info_t * pfn_info)
{
	struct wl_pfn_bdcast_list *bdcast_node;

	/* Free the entire linked list */
	while (pfn_info->bdcast_list) {
		bdcast_node = pfn_info->bdcast_list;
		pfn_info->bdcast_list = pfn_info->bdcast_list->next;
		MFREE(pfn_info->wlc->osh, bdcast_node, sizeof(struct wl_pfn_bdcast_list));
	}
	pfn_info->bdcastnum = 0;
}

static void
wl_pfn_cipher2wsec(uint8 ucount, uint8 * unicast, uint32 * wsec)
{
	int i;

	for (i = 0; i < ucount; i++) {
		switch (unicast[i]) {
		case WPA_CIPHER_WEP_40:
		case WPA_CIPHER_WEP_104:
			*wsec |= WEP_ENABLED;
			break;
		case WPA_CIPHER_TKIP:
			*wsec |= TKIP_ENABLED;
			break;
		case WPA_CIPHER_AES_OCB:
		case WPA_CIPHER_AES_CCM:
			*wsec |= AES_ENABLED;
			break;
		}
	}
	return;
}

static int
wl_pfn_sendonenet(wl_pfn_info_t *pfn_info, wlc_bss_info_t * bi, uint32 now)
{
	wlc_info_t *wlc = pfn_info->wlc;
	wl_pfn_scanresult_t *result;
	uint evt_data_size = sizeof(wl_pfn_scanresult_t) +
		ROUNDUP(bi->bcn_prb_len - DOT11_BCN_PRB_LEN, 4);

	result = (wl_pfn_scanresult_t *)MALLOCZ(wlc->osh,
		evt_data_size);
	if (!result) {
		WL_INFORM(("wl%d: found result allcation failed\n",
		            wlc->pub->unit));
		return BCME_NOMEM;
	}

	result->version = PFN_SCANRESULT_VERSION;
	result->status = PFN_COMPLETE;
	result->count = 1;
	bcopy(bi->BSSID.octet, result->netinfo.pfnsubnet.BSSID.octet, ETHER_ADDR_LEN);
	result->netinfo.pfnsubnet.channel = wf_chspec_ctlchan(bi->chanspec);
	result->netinfo.pfnsubnet.SSID_len = bi->SSID_len;
	bcopy(bi->SSID, result->netinfo.pfnsubnet.SSID, bi->SSID_len);
	result->netinfo.RSSI = bi->RSSI;
	/* timestamp is only meaningful when reporting best network info,
	   but not useful when reporting netfound
	*/
	result->netinfo.timestamp = 0;

	/* now put the bss_info in */
	wlc_bss2wl_bss(wlc, bi, &result->bss_info, evt_data_size, TRUE);

	if (BCME_OK != wl_pfn_send_event(pfn_info, (void *)result,
		evt_data_size, WLC_E_PFN_NET_FOUND)) {
		WL_ERROR(("wl%d: send event fails\n",
		          pfn_info->wlc->pub->unit));
		MFREE(pfn_info->wlc->osh, result, evt_data_size);
		return BCME_ERROR;
	}

	return BCME_OK;
}

void
wl_pfn_process_scan_result(wl_pfn_info_t* pfn_info, wlc_bss_info_t * bi)
{
	int k;
	uint8 *ssid;
	uint8 ssid_len;
	wl_pfn_internal_t *pfn_internal;
	wl_pfn_bdcast_list_t *bdcast_node_list;
	uint32	now;
	uint scanresult_wpa_auth = WPA_AUTH_DISABLED;
	uint32 scanresult_wsec = 0;
	struct ether_addr *bssid;
	wl_pfn_bssid_list_t *bssidlist = NULL;
	wl_pfn_bestinfo_t *bestptr;
	int16 oldrssi, rssidiff;
	uint8 index, chan;
	int retval;
#ifdef BCMDBG
	char ssidbuf[SSID_FMT_BUF_LEN];
#endif /* BCMDBG */

	ASSERT(pfn_info);

	now = OSL_SYSUPTIME();

	ssid = bi->SSID;
	ssid_len = bi->SSID_len;
	bssid = &bi->BSSID;
	chan = wf_chspec_ctlchan(bi->chanspec);

	WL_TRACE(("wl%d: Scan result: RSSI = %d, SSID = %s\n",
		pfn_info->wlc->pub->unit, bi->RSSI,
		(wlc_format_ssid(ssidbuf, ssid, ssid_len), ssidbuf)));

	/* see if this AP is one of the bestn */
	/* best network is not limited to on channel */
	if (pfn_info->current_bestn) {
		bestptr = wl_pfn_get_best_networks(pfn_info,
		     pfn_info->current_bestn->bestnetinfo, bi->RSSI, bi->BSSID);
		if (bestptr) {
			bcopy(bi->BSSID.octet, bestptr->pfnsubnet.BSSID.octet, ETHER_ADDR_LEN);
			bestptr->pfnsubnet.SSID_len = ssid_len;
			bcopy(ssid, bestptr->pfnsubnet.SSID, bestptr->pfnsubnet.SSID_len);
			bestptr->pfnsubnet.channel = chan;
			bestptr->RSSI = bi->RSSI;
			bestptr->timestamp = now;
		}
	}
	/* check BSSID list in pfn_info and see if any match there */
	if ((pfn_info->reporttype == WL_PFN_REPORT_BSSIDNET) ||
		(pfn_info->reporttype == WL_PFN_REPORT_ALLNET)) {
		index = bssid->octet[ETHER_ADDR_LEN -1] & 0xf;
		bssidlist = pfn_info->bssid_list[index];
		while (bssidlist)
		{
			int8 rssi = (bssidlist->flags & WL_PFN_RSSI_MASK)
				>> WL_PFN_RSSI_SHIFT;

			if (!bcmp(bssidlist->bssid.octet, bssid->octet, ETHER_ADDR_LEN) &&
				(rssi == 0 || bi->RSSI >= rssi)) {
				if (!bssidlist->pbssidinfo) {
				     bssidlist->pbssidinfo =
				        (wl_pfn_bssidinfo_t *)MALLOC(pfn_info->wlc->osh,
				                              sizeof(wl_pfn_bssidinfo_t));
					if (!bssidlist->pbssidinfo) {
						WL_ERROR(("wl%d: pbssidinfo MALLOC failed\n",
						          pfn_info->wlc->pub->unit));
						if (pfn_info->param->mscan && pfn_info->numofscan &&
						  (pfn_info->param->flags & REPORT_SEPERATELY_MASK))
							wl_pfn_send_event(pfn_info,
							      NULL, 0, WLC_E_PFN_BEST_BATCHING);
						break;
					}
					bssidlist->pbssidinfo->ssid.SSID_len = ssid_len;
					bcopy(ssid, bssidlist->pbssidinfo->ssid.SSID, ssid_len);
					bssidlist->pbssidinfo->rssi = bi->RSSI;
					if (!(bssidlist->flags &
					 (WL_PFN_SUPPRESSFOUND_MASK | WL_PFN_SUPPRESSLOST_MASK))) {
						bssidlist->pbssidinfo->network_found =
						                               PFN_NET_JUST_FOUND;
						pfn_info->foundcnt++;
						pfn_info->foundcnt_bssid++;
						pfn_info->availbsscnt++;
					} else {
						if (!(bssidlist->flags &
						      WL_PFN_SUPPRESSFOUND_MASK)) {
							bssidlist->pbssidinfo->network_found =
							                       PFN_NET_JUST_FOUND;
							pfn_info->foundcnt++;
							pfn_info->foundcnt_bssid++;
						} else {
							bssidlist->pbssidinfo->network_found =
							                    PFN_NET_ALREADY_FOUND;
							if (!(bssidlist->flags &
							      WL_PFN_SUPPRESSLOST_MASK))
								pfn_info->availbsscnt++;
						}
					}
				} else if (bssidlist->pbssidinfo &&
				           bssidlist->pbssidinfo->network_found ==
				           PFN_NET_JUST_LOST) {
					bssidlist->pbssidinfo->network_found =
					    PFN_NET_ALREADY_FOUND;
					if (!(bssidlist->flags & WL_PFN_SUPPRESSLOST_MASK)) {
						ASSERT(pfn_info->lostcnt);
						ASSERT(pfn_info->lostcnt_bssid);
						pfn_info->lostcnt--;
						pfn_info->lostcnt_bssid--;
					}
					if ((pfn_info->param->flags & ENABLE_ADAPTSCAN_MASK) ==
					    (SMART_ADAPT << ENABLE_ADAPTSCAN_BIT))
						pfn_info->adaptcnt = 0;
				}
				oldrssi = bssidlist->pbssidinfo->rssi;
				/* update rssi */
				if (((bssidlist->pbssidinfo->flags & WL_PFN_RSSI_ONCHANNEL) &&
				    (bi->flags & WLC_BSS_RSSI_ON_CHANNEL)) ||
				    (!(bssidlist->pbssidinfo->flags & WL_PFN_RSSI_ONCHANNEL) &&
				    !(bi->flags & WLC_BSS_RSSI_ON_CHANNEL))) {
					/* preserve max RSSI if the measurements are
					 * both on-channel or both off-channel
					 */
					bssidlist->pbssidinfo->rssi =
					                 MAX(bssidlist->pbssidinfo->rssi, bi->RSSI);
				} else if ((bi->flags & WLC_BSS_RSSI_ON_CHANNEL) &&
				    (bssidlist->pbssidinfo->flags & WL_PFN_RSSI_ONCHANNEL) == 0) {
					/* preserve the on-channel rssi measurement
					 * if the new measurement is off channel
					 */
					bssidlist->pbssidinfo->rssi = bi->RSSI;
					bssidlist->pbssidinfo->flags |= WL_PFN_RSSI_ONCHANNEL;
				}
				/* detect any change to turn off adaptive scan */
				if ((bssidlist->pbssidinfo->network_found == PFN_NET_JUST_FOUND) ||
				 (bssidlist->pbssidinfo->network_found == PFN_NET_ALREADY_FOUND)) {
					rssidiff = bssidlist->pbssidinfo->rssi - oldrssi;
					if (((pfn_info->param->flags & ENABLE_ADAPTSCAN_MASK) ==
					    (SMART_ADAPT << ENABLE_ADAPTSCAN_BIT)) &&
					    ((bssidlist->pbssidinfo->channel != chan) ||
					     (rssidiff < -(pfn_info->param->rssi_margin) ||
					     rssidiff > pfn_info->param->rssi_margin)))
						pfn_info->adaptcnt = 0;
				}
				/* update channel and time_stamp */
				bssidlist->pbssidinfo->channel = chan;
				bssidlist->pbssidinfo->time_stamp = now;
				break;
			}
			bssidlist = bssidlist->next;
		}
	}
	if ((pfn_info->reporttype == WL_PFN_REPORT_SSIDNET) ||
		(pfn_info->reporttype == WL_PFN_REPORT_ALLNET)) {
		if (bi->wpa.flags) {
			for (k = 0; k < bi->wpa.acount; k++) {
				if (bi->wpa.auth[k] == RSN_AKM_UNSPECIFIED)
					scanresult_wpa_auth |= WPA_AUTH_UNSPECIFIED;
				else
					scanresult_wpa_auth |= WPA_AUTH_PSK;
			}
			wl_pfn_cipher2wsec(bi->wpa.ucount, bi->wpa.unicast, &scanresult_wsec);
		}
		if (bi->wpa2.flags) {
			for (k = 0; k < bi->wpa2.acount; k++) {
				if (bi->wpa2.auth[k] == RSN_AKM_UNSPECIFIED)
					scanresult_wpa_auth |= WPA2_AUTH_UNSPECIFIED;
				else
					scanresult_wpa_auth |= WPA2_AUTH_PSK;
			}
			wl_pfn_cipher2wsec(bi->wpa2.ucount, bi->wpa2.unicast, &scanresult_wsec);
		}

		/* When psk/psk2 set, privacy bit is set, too */
		if ((bi->capability & DOT11_CAP_PRIVACY) &&
			(scanresult_wpa_auth == WPA_AUTH_DISABLED))
			scanresult_wsec = WEP_ENABLED;

		/* Walk through the PFN element array first to find a match */
		for (k = 0; k < pfn_info->count; k++) {
			int8 rssi;
			pfn_internal = pfn_info->pfn_internal[k];

			rssi = (pfn_internal->pfn.flags & WL_PFN_RSSI_MASK)
				>> WL_PFN_RSSI_SHIFT;

			/* Bail if requested security match fails */
			if (pfn_internal->pfn.wpa_auth != WPA_AUTH_PFN_ANY) {
				if (pfn_internal->pfn.wpa_auth) {
					if (!(pfn_internal->pfn.wpa_auth & scanresult_wpa_auth))
						continue;
					if (pfn_internal->pfn.wsec &&
					   !(pfn_internal->pfn.wsec & scanresult_wsec))
						continue;
				}
				else if (!scanresult_wpa_auth) {
					if (pfn_internal->pfn.wsec &&
						!(pfn_internal->pfn.wsec & scanresult_wsec))
						continue;
					if (!pfn_internal->pfn.wsec && scanresult_wsec)
						continue;
				}
				else
					continue;
			}

			/* Bail if requested imode match fails */
			if ((pfn_internal->pfn.infra && (bi->capability & DOT11_CAP_IBSS)) ||
				(!pfn_internal->pfn.infra && (bi->capability & DOT11_CAP_ESS)))
				continue;

			if ((pfn_internal->pfn.ssid.SSID_len == ssid_len) &&
				!bcmp(ssid, pfn_internal->pfn.ssid.SSID, ssid_len) &&
				(rssi == 0 || bi->RSSI >= rssi)) {
				if (pfn_internal->network_found == PFN_NET_NOT_FOUND) {
					if (!(pfn_internal->pfn.flags &
					 (WL_PFN_SUPPRESSFOUND_MASK | WL_PFN_SUPPRESSLOST_MASK))) {
						pfn_internal->network_found = PFN_NET_JUST_FOUND;
						pfn_info->foundcnt++;
						pfn_info->foundcnt_ssid++;
						pfn_info->availbsscnt++;
					} else if (!(pfn_internal->pfn.flags &
					      WL_PFN_SUPPRESSFOUND_MASK)) {
						if (pfn_info->param->flags &
							IMMEDIATE_EVENT_MASK) {
							if (wl_pfn_sendonenet(pfn_info, bi, now) ==
							    BCME_OK)
								pfn_internal->network_found =
								   PFN_NET_ALREADY_FOUND;
						} else {
							pfn_internal->network_found =
							   PFN_NET_JUST_FOUND;
							pfn_info->foundcnt++;
							pfn_info->foundcnt_ssid++;
						}
					} else {
						pfn_internal->network_found =
						           PFN_NET_ALREADY_FOUND;
						if (!(pfn_internal->pfn.flags &
						      WL_PFN_SUPPRESSLOST_MASK))
							pfn_info->availbsscnt++;
					}

					pfn_internal->rssi = bi->RSSI;
					if (bi->flags & WLC_BSS_RSSI_ON_CHANNEL)
						pfn_internal->flags |= WL_PFN_RSSI_ONCHANNEL;
					else
						pfn_internal->flags &= ~WL_PFN_RSSI_ONCHANNEL;
					/* fill BSSID */
					bcopy(bi->BSSID.octet, pfn_internal->bssid.octet,
					      ETHER_ADDR_LEN);
					if ((pfn_info->param->flags & ENABLE_ADAPTSCAN_MASK) ==
					    (SMART_ADAPT << ENABLE_ADAPTSCAN_BIT))
						pfn_info->adaptcnt = 0;
					/* update channel */
					pfn_internal->channel = chan;
				} else {
					if (!bcmp(bi->BSSID.octet, pfn_internal->bssid.octet,
					          ETHER_ADDR_LEN)) {
						oldrssi = pfn_internal->rssi;
						/* update rssi */
						if (((pfn_internal->flags &
						                     WL_PFN_RSSI_ONCHANNEL) &&
						    (bi->flags & WLC_BSS_RSSI_ON_CHANNEL)) ||
						    (!(pfn_internal->flags &
						                     WL_PFN_RSSI_ONCHANNEL) &&
						    !(bi->flags & WLC_BSS_RSSI_ON_CHANNEL))) {
							/* preserve max RSSI if the measurements are
							  * both on-channel or both off-channel
							  */
							pfn_internal->rssi =
							          MAX(pfn_internal->rssi, bi->RSSI);
						} else if ((bi->flags & WLC_BSS_RSSI_ON_CHANNEL) &&
						           (pfn_internal->flags &
						                   WL_PFN_RSSI_ONCHANNEL) == 0) {
							/* preserve the on-channel rssi measurement
							  * if the new measurement is off channel
							  */
							pfn_internal->rssi = bi->RSSI;
							pfn_internal->flags |=
							             WL_PFN_RSSI_ONCHANNEL;
						}
						if (pfn_internal->network_found ==
						                        PFN_NET_JUST_LOST) {
							pfn_internal->network_found =
							                    PFN_NET_ALREADY_FOUND;
							ASSERT(pfn_info->lostcnt);
							ASSERT(pfn_info->lostcnt_ssid);
							pfn_info->lostcnt--;
							pfn_info->lostcnt_ssid--;
							if ((pfn_info->param->flags
							     & ENABLE_ADAPTSCAN_MASK) ==
							     (SMART_ADAPT << ENABLE_ADAPTSCAN_BIT))
								pfn_info->adaptcnt = 0;
						} else if (pfn_internal->network_found ==
						           PFN_NET_JUST_FOUND) {
							; /* do nothing */
						} else {
							rssidiff = pfn_internal->rssi - oldrssi;
							if (((pfn_info->param->flags &
							    ENABLE_ADAPTSCAN_MASK) ==
							   (SMART_ADAPT << ENABLE_ADAPTSCAN_BIT)) &&
							    ((pfn_internal->channel != chan) ||
								(rssidiff <
								-(pfn_info->param->rssi_margin) ||
								rssidiff >
								pfn_info->param->rssi_margin))) {
								/* reset adaptive scan */
								pfn_info->adaptcnt = 0;
							}
						}
						/* update channel */
						pfn_internal->channel = chan;
					} else {
						if (bi->RSSI - pfn_internal->rssi >
							pfn_info->param->rssi_margin) {
							/* replace BSSID */
							bcopy(bi->BSSID.octet,
							      pfn_internal->bssid.octet,
							      ETHER_ADDR_LEN);
							/* update rssi, channel */
							pfn_internal->rssi = bi->RSSI;
							if (bi->flags & WLC_BSS_RSSI_ON_CHANNEL)
								pfn_internal->flags |=
								             WL_PFN_RSSI_ONCHANNEL;
							else
								pfn_internal->flags &=
								            ~WL_PFN_RSSI_ONCHANNEL;
							if ((pfn_info->param->flags &
							     ENABLE_ADAPTSCAN_MASK) ==
							    (SMART_ADAPT << ENABLE_ADAPTSCAN_BIT))
								pfn_info->adaptcnt = 0;
							pfn_internal->channel = chan;
						}
					}
				}
				pfn_internal->time_stamp = now;
				return;
			}
		}
	}
	/* If match is found as a part of PFN element array, don't need
	* to search in broadcast list. Also don't process the broadcast
	* list if back ground scanning is disabled
	*/
	if (!bssidlist &&
	    (pfn_info->param->flags & ENABLE_BKGRD_SCAN_MASK) &&
	    (pfn_info->reporttype == WL_PFN_REPORT_ALLNET)) {
		if (ssid_len > 32) {
			WL_ERROR(("too long ssid %d\n", ssid_len));
			return;
		}

		bdcast_node_list = pfn_info->bdcast_list;
		/* Traverse ssid list found through background scan */
		while (bdcast_node_list) {
			/* Match ssid name */
			if ((bdcast_node_list->bssidinfo.ssid.SSID_len == ssid_len) &&
				!bcmp(ssid, &bdcast_node_list->bssidinfo.ssid.SSID, ssid_len))
				break;
			bdcast_node_list = bdcast_node_list->next;
		}

		/* Not found in the list, add to the list */
		if (!bdcast_node_list) {
			if (pfn_info->bdcastnum >= MAX_BDCAST_NUM) {
				WL_ERROR(("wl%d: too many bdcast nodes\n",
				          pfn_info->wlc->pub->unit));
				return;
			}
			if ((retval = wl_pfn_allocate_bdcast_node(pfn_info, ssid_len, ssid, now))
			     != BCME_OK) {
				WL_ERROR(("wl%d: fail to allocate new bdcast node\n",
				           pfn_info->wlc->pub->unit));
			     return;
			}
			pfn_info->bdcast_list->bssidinfo.network_found = PFN_NET_JUST_FOUND;
			pfn_info->foundcnt++;
			pfn_info->availbsscnt++;
			bcopy(bi->BSSID.octet, pfn_info->bdcast_list->bssid.octet, ETHER_ADDR_LEN);
			pfn_info->bdcast_list->bssidinfo.channel = chan;
			pfn_info->bdcast_list->bssidinfo.rssi = bi->RSSI;
			if (bi->flags & WLC_BSS_RSSI_ON_CHANNEL)
				pfn_info->bdcast_list->bssidinfo.flags |= WL_PFN_RSSI_ONCHANNEL;
			pfn_info->bdcastnum++;
			WL_TRACE(("wl%d: Added to broadcast list RSSI = %d\n",
				pfn_info->wlc->pub->unit, bi->RSSI));
		} else {
			/* update channel, time_stamp */
			pfn_info->bdcast_list->bssidinfo.rssi = bi->RSSI;
			bdcast_node_list->bssidinfo.channel = chan;
			bdcast_node_list->bssidinfo.time_stamp = now;
		}
	}
}

static wl_pfn_bestinfo_t*
wl_pfn_get_best_networks(wl_pfn_info_t *pfn_info, wl_pfn_bestinfo_t *bestap_ptr,
                              int16 rssi, struct ether_addr bssid)
{
	int i, j;
	wl_pfn_bestinfo_t *ptr = bestap_ptr;

	for (i = 0; i < pfn_info->param->bestn; i++) {
		if (!bcmp(ptr->pfnsubnet.BSSID.octet, bssid.octet, ETHER_ADDR_LEN)) {
			if (rssi > ptr->RSSI) {
				for (j = 0; j < i; j++)
				{
					if (rssi > (bestap_ptr + j)->RSSI)
					{
						memmove(bestap_ptr + j + 1, bestap_ptr + j,
						        sizeof(wl_pfn_bestinfo_t) * (i - j));
						return (bestap_ptr + j);
					}
				}
				ptr->RSSI = rssi;
			}
			return NULL;
		}
		ptr++;
	}
	ptr = bestap_ptr;
	for (i = 0; i < pfn_info->param->bestn; i++) {
		if (ETHER_ISNULLADDR(&ptr->pfnsubnet.BSSID)) {
			return ptr;
		}
		if (rssi > ptr->RSSI) {
			if (i < pfn_info->param->bestn - 1) {
				memmove(ptr + 1, ptr,
				    sizeof(wl_pfn_bestinfo_t) * (pfn_info->param->bestn - 1 - i));
			}
			return ptr;
		}
		ptr++;
	}
	return NULL;
}

#ifdef NLO
/* config suppress ssid based on pfn option */
static void
wl_pfn_cfg_supprs_ssid(wl_pfn_info_t *pfn_info, bool pre_scan)
{
	int suppress_ssid;
	int pfn_suppress_ssid;

	/* get current suppress ssid setting */
	wlc_iovar_op(pfn_info->wlc, "scan_suppress_ssid", NULL, 0,
		&suppress_ssid, sizeof(suppress_ssid), IOV_GET, NULL);
	suppress_ssid = suppress_ssid ? TRUE : FALSE;

	/* get pfn suppress ssid config */
	pfn_suppress_ssid = (((pfn_info->param->flags & SUPPRESS_SSID_MASK) >>
		SUPPRESS_SSID_BIT) == ENABLE) ? TRUE : FALSE;

	if (pre_scan) {
		/* save current setting */
		pfn_info->suppress_ssid_saved = (bool)suppress_ssid;
		if (suppress_ssid == pfn_suppress_ssid)
			/* if current setting matches pfn setting, done */
			return;
		/* need change */
		suppress_ssid = pfn_suppress_ssid;
	} else {
		/* make no change if current setting is different than
		 * pfn setting(interrupted by other scan req) or current
		 * setting is same as pre-scan saved setting
		 */
		if (suppress_ssid != pfn_suppress_ssid ||
			suppress_ssid == pfn_info->suppress_ssid_saved)
			/* if current setting matches saved setting, done */
			return;
		/* need change */
		suppress_ssid = pfn_info->suppress_ssid_saved;
	}

	/* configure suppress ssid */
	wlc_iovar_op(pfn_info->wlc, "scan_suppress_ssid", NULL, 0,
		&(suppress_ssid), sizeof(suppress_ssid), IOV_SET, NULL);
}
#endif /* NLO */

static void
wl_pfn_create_event(wl_pfn_info_t *pfn_info, uint8 pfnfdcnt, uint8 pfnltcnt, uint8 reportnet)
{
	wlc_info_t * wlc;
	wl_pfn_scanresults_t *foundresult = NULL, *lostresult = NULL;
	uint8 foundcnt = 0, lostcnt = 0;
	int32 thisfoundlen = 0, thislostlen = 0;
	wl_pfn_net_info_t *foundptr, *lostptr;
	int event_type;

	ASSERT(pfn_info);

	wlc = pfn_info->wlc;

	while (pfnfdcnt || pfnltcnt) {
		foundresult = lostresult = NULL;
		foundcnt = MIN(pfnfdcnt, EVENT_MAX_NETCNT);
		lostcnt = MIN(pfnltcnt, EVENT_MAX_NETCNT);
		thisfoundlen = sizeof(wl_pfn_scanresults_t) +
		         (foundcnt - 1) * sizeof(wl_pfn_net_info_t);
		thislostlen = sizeof(wl_pfn_scanresults_t) +
		         (lostcnt - 1) * sizeof(wl_pfn_net_info_t);

		if (foundcnt) {
			foundresult = (wl_pfn_scanresults_t *)MALLOC(wlc->osh,
			                                      thisfoundlen);
			if (!foundresult) {
				WL_INFORM(("wl%d: PFN found scanresults allcation failed\n",
				           wlc->pub->unit));
				if (pfn_info->param->mscan && pfn_info->numofscan &&
					(pfn_info->param->flags & REPORT_SEPERATELY_MASK))
					wl_pfn_send_event(pfn_info, NULL,
					          0, WLC_E_PFN_BEST_BATCHING);
				break;
			}
			bzero(foundresult, thisfoundlen);
			foundresult->version = PFN_SCANRESULT_VERSION;
			foundresult->count = foundcnt;
			foundptr = foundresult->netinfo;
			if (foundcnt < pfnfdcnt)
				foundresult->status = PFN_INCOMPLETE;
			else
				foundresult->status = PFN_COMPLETE;
		} else {
			foundptr = NULL;
		}

		if (lostcnt) {
			lostresult = (wl_pfn_scanresults_t *)MALLOC(wlc->osh,
			                                     thislostlen);
			if (!lostresult) {
				WL_INFORM(("wl%d: PFN lost scanresults allcation failed\n",
				           wlc->pub->unit));
				MFREE(pfn_info->wlc->osh, foundresult, thisfoundlen);
				if (pfn_info->param->mscan && pfn_info->numofscan &&
					(pfn_info->param->flags & REPORT_SEPERATELY_MASK))
					wl_pfn_send_event(pfn_info, NULL,
					       0, WLC_E_PFN_BEST_BATCHING);
				break;
			}
			bzero(lostresult, thislostlen);
			lostresult->version = PFN_SCANRESULT_VERSION;
			lostresult->count = lostcnt;
			lostptr = lostresult->netinfo;
			if (lostcnt < pfnltcnt)
				lostresult->status = PFN_INCOMPLETE;
			else
				lostresult->status = PFN_COMPLETE;
		} else {
			lostptr = NULL;
		}

		wl_pfn_updatenet(pfn_info, foundptr, lostptr, foundcnt, lostcnt, reportnet);

		if (foundcnt) {
			event_type = (reportnet == WL_PFN_REPORT_BSSIDNET) ?
			             WLC_E_PFN_BSSID_NET_FOUND : WLC_E_PFN_NET_FOUND;
			if (BCME_OK != wl_pfn_send_event(pfn_info, (void *)foundresult,
				thisfoundlen, event_type)) {
				WL_ERROR(("wl%d: scan found network event fail\n",
				          pfn_info->wlc->pub->unit));
				MFREE(pfn_info->wlc->osh, foundresult, thisfoundlen);
			}
		}

		if (lostcnt) {
			event_type = (reportnet == WL_PFN_REPORT_BSSIDNET) ?
			              WLC_E_PFN_BSSID_NET_LOST : WLC_E_PFN_NET_LOST;
			if (BCME_OK != wl_pfn_send_event(pfn_info, (void *)lostresult,
				thislostlen, event_type)) {
				WL_ERROR(("wl%d: scan lost network event fail\n",
				          pfn_info->wlc->pub->unit));
				MFREE(pfn_info->wlc->osh, lostresult, thislostlen);
			}
		}
		/* update found and lost count */
		pfnfdcnt -= foundcnt;
		pfn_info->foundcnt -= foundcnt;
		pfnltcnt -= lostcnt;
		pfn_info->lostcnt -= lostcnt;
	}
}

/* attach bestn from current scan to the best network linked-list */
static void wl_pfn_attachbestn(wl_pfn_info_t * pfn_info, bool partial)
{
	wl_pfn_bestinfo_t *bestinfo = pfn_info->current_bestn->bestnetinfo;
	wl_pfn_bestnet_t *ptr;
	wl_pfn_bestinfo_t *pcurbest, *plastbest;
	int i;

	if (bestinfo->pfnsubnet.channel) {
		if (partial)
			bestinfo->flags |= 1 << PFN_PARTIAL_SCAN_BIT;
		else
			bestinfo->flags &= ~PFN_PARTIAL_SCAN_MASK;
		/* current_bestn has valid information */
		if (pfn_info->numofscan < pfn_info->param->mscan) {
			pfn_info->numofscan++;
		} else {
			/* remove the oldest one at pfnbesthdr */
			ptr = pfn_info->bestnethdr;
			pfn_info->bestnethdr = pfn_info->bestnethdr->next;
			MFREE(pfn_info->wlc->osh, ptr, pfn_info->bestnetsize);
			if (!pfn_info->bestnethdr)
				pfn_info->bestnettail = NULL;
		}

		if (pfn_info->bestnettail &&
			(pfn_info->param->flags & ENABLE_ADAPTSCAN_MASK) ==
			(SMART_ADAPT << ENABLE_ADAPTSCAN_BIT)) {
			/* compare with the previous one, see if any change */
			pcurbest = pfn_info->current_bestn->bestnetinfo;
			plastbest = pfn_info->bestnettail->bestnetinfo;
			for (i = 0; i < pfn_info->param->bestn; i++) {
				if (bcmp(pcurbest->pfnsubnet.BSSID.octet,
				    plastbest->pfnsubnet.BSSID.octet,
				    ETHER_ADDR_LEN)) {
					/* reset scan frequency */
					pfn_info->adaptcnt = 0;
					break;
				}
				pcurbest++;
				plastbest++;
			}
		}
		/* add to bestnettail */
		if (pfn_info->bestnettail) {
			pfn_info->bestnettail->next = pfn_info->current_bestn;
		} else {
			pfn_info->bestnethdr = pfn_info->current_bestn;
		}
		pfn_info->bestnettail = pfn_info->current_bestn;
		pfn_info->current_bestn = NULL;
		if ((pfn_info->numofscan == pfn_info->param->mscan) &&
			(pfn_info->param->flags & REPORT_SEPERATELY_MASK))
			wl_pfn_send_event(pfn_info, NULL, 0, WLC_E_PFN_BEST_BATCHING);
	}
}

static void
wl_pfn_scan_complete(void * data, int status, wlc_bsscfg_t *cfg)
{
	wl_pfn_info_t *pfn_info = (wl_pfn_info_t *)data;
	uint32 now;
	int i;

	ASSERT(pfn_info);
	ASSERT(pfn_info->ssid_list);

#ifdef NLO
	/* disable network offload */
	wlc_iovar_setint(pfn_info->wlc, "nlo", FALSE);

	/* handling suppress ssid option */
	wl_pfn_cfg_supprs_ssid(pfn_info, FALSE);
#endif /* NLO */

	if (status != WLC_E_STATUS_SUCCESS) {
		if ((status == WLC_E_STATUS_NEWSCAN) ||
		    (status == WLC_E_STATUS_SUPPRESS)) {
			wl_pfn_free_ssidlist(pfn_info, FALSE, TRUE);
			wl_pfn_free_bssidlist(pfn_info, FALSE, TRUE);
			pfn_info->lostcnt = 0;
			pfn_info->foundcnt = 0;
			pfn_info->lostcnt_ssid = 0;
			pfn_info->foundcnt_ssid = 0;
			pfn_info->lostcnt_bssid = 0;
			pfn_info->foundcnt_bssid = 0;
			if ((pfn_info->param->flags & REPORT_SEPERATELY_MASK) &&
				pfn_info->param->mscan && pfn_info->current_bestn) {
				wl_pfn_attachbestn(pfn_info, TRUE);
			}
		}
		pfn_info->pfn_scan_state = PFN_SCAN_IDLE;

		return;
	}

	/* PFN_SCAN_INPROGRESS is the only vaild state this function can process */
	if (PFN_SCAN_INPROGRESS != pfn_info->pfn_scan_state)
		return;

	/* get current ms count for timestamp/timeout processing */
	now = OSL_SYSUPTIME();

	/* If all done scanning, age the SSIDs and do the join if necessary */
	if (pfn_info->param->lost_network_timeout != -1)
		wl_pfn_ageing(pfn_info, now);

	pfn_info->pfn_scan_state = PFN_SCAN_IDLE;

	/* link the bestn from this scan */
	if (pfn_info->param->mscan && pfn_info->current_bestn) {
		wl_pfn_attachbestn(pfn_info, FALSE);
	}

	/* generate event at the end of scan */
	if (pfn_info->foundcnt || pfn_info->lostcnt) {
		if ((pfn_info->param->flags & ENABLE_ADAPTSCAN_MASK) ==
			(SMART_ADAPT << ENABLE_ADAPTSCAN_BIT))
			pfn_info->adaptcnt = 0;

		if (pfn_info->param->flags & REPORT_SEPERATELY_MASK) {
			if (pfn_info->foundcnt_ssid || pfn_info->lostcnt_ssid)
				wl_pfn_create_event(pfn_info, pfn_info->foundcnt_ssid,
				        pfn_info->lostcnt_ssid, WL_PFN_REPORT_SSIDNET);
			if (pfn_info->foundcnt_bssid || pfn_info->lostcnt_bssid) {
				wl_pfn_create_event(pfn_info, pfn_info->foundcnt_bssid,
				        pfn_info->lostcnt_bssid, WL_PFN_REPORT_BSSIDNET);
			}
		}
		else
			wl_pfn_create_event(pfn_info, pfn_info->foundcnt,
			                pfn_info->lostcnt, WL_PFN_REPORT_ALLNET);
	}

	/* handle the case where non of report-lost PNO networks are ever found */
	if (pfn_info->param->lost_network_timeout != -1 &&
	    pfn_info->none_cnt != NONEED_REPORTLOST &&
	    pfn_info->none_cnt != STOP_REPORTLOST &&
	    pfn_info->availbsscnt == 0) {
		if (++pfn_info->none_cnt ==
		   (pfn_info->param->lost_network_timeout / pfn_info->param->scan_freq)) {
			if (!(pfn_info->param->flags & REPORT_SEPERATELY_MASK) &&
			    BCME_OK != wl_pfn_send_event(pfn_info, NULL,
				0, WLC_E_PFN_SCAN_ALLGONE)) {
				WL_ERROR(("wl%d: ALLGONE event fail\n",
				           pfn_info->wlc->pub->unit));
			}
			pfn_info->none_cnt = STOP_REPORTLOST;
		}
	}

	/* adaptive scanning */
	if (pfn_info->param->flags & ENABLE_ADAPTSCAN_MASK) {
		if ((pfn_info->param->flags & ENABLE_ADAPTSCAN_MASK) !=
		    (SLOW_ADAPT << ENABLE_ADAPTSCAN_BIT)) {
			if (pfn_info->adaptcnt <= pfn_info->param->repeat * pfn_info->param->exp)
				pfn_info->adaptcnt++;

			if (pfn_info->currentadapt &&
			    (pfn_info->adaptcnt < pfn_info->param->repeat)) {
				ASSERT(pfn_info->p_pfn_timer);
				wl_del_timer(pfn_info->wlc->wl, pfn_info->p_pfn_timer);
				wl_add_timer(pfn_info->wlc->wl, pfn_info->p_pfn_timer,
				             pfn_info->param->scan_freq, TRUE);
				pfn_info->param->lost_network_timeout >>= pfn_info->currentadapt;
				pfn_info->currentadapt = 0;
			} else {
				for (i = 1; i <= pfn_info->param->exp; i++) {
					if (pfn_info->adaptcnt == pfn_info->param->repeat * i) {
						ASSERT(pfn_info->p_pfn_timer);
						wl_del_timer(pfn_info->wlc->wl,
						         pfn_info->p_pfn_timer);
						wl_add_timer(pfn_info->wlc->wl,
							pfn_info->p_pfn_timer,
						    pfn_info->param->scan_freq << i,
						    TRUE);
						pfn_info->currentadapt = (uint8)i;
						pfn_info->param->lost_network_timeout <<= 1;
						break;
					}
				}
			}
		} else {
#if defined(WLC_PATCH) && defined(BCM43362A2)
			if (pfn_info->slow_freq &&
#else
			if (pfn_info->param->slow_freq &&
#endif /* WLC_PATCH && BCM43362A2 */
				(pfn_info->adaptcnt < pfn_info->param->repeat) &&
				(++pfn_info->adaptcnt == pfn_info->param->repeat)) {
					wl_del_timer(pfn_info->wlc->wl, pfn_info->p_pfn_timer);
					wl_add_timer(pfn_info->wlc->wl, pfn_info->p_pfn_timer,
#if defined(WLC_PATCH) && defined(BCM43362A2)
					             pfn_info->slow_freq, TRUE);
#else
					             pfn_info->param->slow_freq, TRUE);
#endif /* WLC_PATCH && BCM43362A2 */
			}
		}
	}
}

static void
wl_pfn_updatenet(wl_pfn_info_t *pfn_info, wl_pfn_net_info_t *foundptr,
                    wl_pfn_net_info_t *lostptr, uint8 foundcnt, uint8 lostcnt, uint8 reportnet)
{
	int i;
	wl_pfn_internal_t *pfn_internal;
	wl_pfn_bssid_list_t *bssidlist;
	wl_pfn_net_info_t *inptr;
	wl_pfn_bdcast_list_t *bdcast_node_list, *prev_bdcast_node;

	/* check PFN list */
	if (((pfn_info->reporttype == WL_PFN_REPORT_SSIDNET) ||
		(pfn_info->reporttype == WL_PFN_REPORT_ALLNET)) &&
		((reportnet == WL_PFN_REPORT_SSIDNET) ||
		(reportnet == WL_PFN_REPORT_ALLNET))) {
		for (i = 0; i < pfn_info->count; i++) {
			if (!foundcnt && !lostcnt)
				return;

			pfn_internal = pfn_info->pfn_internal[i];
			inptr = NULL;
			if (foundcnt && foundptr &&
			    pfn_internal->network_found == PFN_NET_JUST_FOUND) {
				pfn_internal->network_found = PFN_NET_ALREADY_FOUND;
				inptr = foundptr;
				foundptr++;
				foundcnt--;
				pfn_info->foundcnt_ssid--;
			} else if (lostcnt && lostptr &&
			    pfn_internal->network_found == PFN_NET_JUST_LOST) {
				pfn_internal->network_found = PFN_NET_NOT_FOUND;
				inptr = lostptr;
				lostptr++;
				lostcnt--;
				pfn_info->lostcnt_ssid--;
			}
			if (inptr) {
				bcopy(pfn_internal->bssid.octet, inptr->pfnsubnet.BSSID.octet,
				      ETHER_ADDR_LEN);
				inptr->pfnsubnet.SSID_len =
				      (uint8)pfn_internal->pfn.ssid.SSID_len;
				bcopy(pfn_internal->pfn.ssid.SSID,
				      inptr->pfnsubnet.SSID, inptr->pfnsubnet.SSID_len);
				inptr->pfnsubnet.channel = (uint8)pfn_internal->channel;
				inptr->RSSI = pfn_internal->rssi;
				inptr->timestamp = 0;
				inptr++;
			}
		}
	}
	/* check BSSID list */
	if (((pfn_info->reporttype == WL_PFN_REPORT_BSSIDNET) ||
		(pfn_info->reporttype == WL_PFN_REPORT_ALLNET)) &&
		((reportnet == WL_PFN_REPORT_BSSIDNET) ||
		(reportnet == WL_PFN_REPORT_ALLNET))) {
		for (i = 0; i < BSSIDLIST_MAX; i++) {
			bssidlist = pfn_info->bssid_list[i];
			while (bssidlist) {
				if (!foundcnt && !lostcnt)
					return;

				inptr = NULL;
				if (foundcnt && foundptr && bssidlist->pbssidinfo &&
				    bssidlist->pbssidinfo->network_found == PFN_NET_JUST_FOUND) {
					bssidlist->pbssidinfo->network_found =
					                         PFN_NET_ALREADY_FOUND;
					inptr = foundptr;
					foundptr++;
					foundcnt--;
					pfn_info->foundcnt_bssid--;
				} else if (lostcnt && lostptr && bssidlist->pbssidinfo &&
				 bssidlist->pbssidinfo->network_found == PFN_NET_JUST_LOST) {
					bssidlist->pbssidinfo->network_found = PFN_NET_NOT_FOUND;
					inptr = lostptr;
					lostptr++;
					lostcnt--;
					pfn_info->lostcnt_bssid--;
				}
				if (inptr) {
					bcopy(bssidlist->bssid.octet, inptr->pfnsubnet.BSSID.octet,
					      ETHER_ADDR_LEN);
					inptr->pfnsubnet.SSID_len =
					    (uint8)bssidlist->pbssidinfo->ssid.SSID_len;
					bcopy(bssidlist->pbssidinfo->ssid.SSID,
					      inptr->pfnsubnet.SSID,
					      inptr->pfnsubnet.SSID_len);
					inptr->pfnsubnet.channel =
						(uint8)bssidlist->pbssidinfo->channel;
					inptr->RSSI = bssidlist->pbssidinfo->rssi;
					inptr->timestamp = 0;
					inptr++;
				}
				if (bssidlist->pbssidinfo &&
					bssidlist->pbssidinfo->network_found == PFN_NET_NOT_FOUND) {
					MFREE(pfn_info->wlc->osh, bssidlist->pbssidinfo,
					      sizeof(wl_pfn_bssidinfo_t));
					bssidlist->pbssidinfo = NULL;
				}
				bssidlist = bssidlist->next;
			}
		}
	}
	/* check bdcast_list */
	if ((pfn_info->reporttype == WL_PFN_REPORT_ALLNET) &&
		(reportnet == WL_PFN_REPORT_ALLNET) &&
		(pfn_info->param->flags & ENABLE_BKGRD_SCAN_MASK)) {
		bdcast_node_list = pfn_info->bdcast_list;
		prev_bdcast_node = bdcast_node_list;
		/* Traverse ssid list found through background scan */
		while (bdcast_node_list) {
			if (!foundcnt && !lostcnt)
				return;

			inptr = NULL;
			if (foundcnt && foundptr &&
			    (bdcast_node_list->bssidinfo.network_found == PFN_NET_JUST_FOUND)) {
				bdcast_node_list->bssidinfo.network_found = PFN_NET_ALREADY_FOUND;
				inptr = foundptr;
				foundptr++;
				foundcnt--;
			}  else if (lostcnt && lostptr &&
			    bdcast_node_list->bssidinfo.network_found == PFN_NET_JUST_LOST) {
				bdcast_node_list->bssidinfo.network_found = PFN_NET_NOT_FOUND;
				inptr = lostptr;
				lostptr++;
				lostcnt--;
			}
			if (inptr) {
				bcopy(bdcast_node_list->bssid.octet, inptr->pfnsubnet.BSSID.octet,
				      ETHER_ADDR_LEN);
				inptr->pfnsubnet.SSID_len =
				      (uint8)bdcast_node_list->bssidinfo.ssid.SSID_len;
				bcopy(bdcast_node_list->bssidinfo.ssid.SSID,
				      inptr->pfnsubnet.SSID, inptr->pfnsubnet.SSID_len);
				inptr->pfnsubnet.channel =
					(uint8)bdcast_node_list->bssidinfo.channel;
				inptr->RSSI = bdcast_node_list->bssidinfo.rssi;
				inptr->timestamp = 0;
				inptr++;
			}
			if (bdcast_node_list->bssidinfo.network_found == PFN_NET_NOT_FOUND) {
				if (bdcast_node_list == pfn_info->bdcast_list) {
					pfn_info->bdcast_list = pfn_info->bdcast_list->next;
					MFREE(pfn_info->wlc->osh, bdcast_node_list,
					      sizeof(wl_pfn_bdcast_list_t));
					prev_bdcast_node = bdcast_node_list = pfn_info->bdcast_list;
				} else {
					prev_bdcast_node->next = bdcast_node_list->next;
					MFREE(pfn_info->wlc->osh, bdcast_node_list,
					      sizeof(wl_pfn_bdcast_list_t));
					bdcast_node_list = prev_bdcast_node->next;
				}
			} else {
				prev_bdcast_node = bdcast_node_list;
				bdcast_node_list = bdcast_node_list->next;
			}
		}
	}
}

static void
wl_pfn_prepare_for_scan(wl_pfn_info_t* pfn_info, wlc_ssid_t** ssidp, int* nssid,
                                  int32* bss_type)
{
	uint8 directcnt;

	ASSERT(ssidp != NULL);

	*bss_type = DOT11_BSSTYPE_ANY;
	*ssidp = &pfn_info->ssid_list[0];
	if (pfn_info->reporttype == WL_PFN_REPORT_BSSIDNET)
		directcnt = 0;
	else
		directcnt = pfn_info->hiddencnt;
	if ((pfn_info->hiddencnt < pfn_info->count &&
	     pfn_info->reporttype != WL_PFN_REPORT_BSSIDNET) ||
	    (pfn_info->bssidNum && pfn_info->reporttype != WL_PFN_REPORT_SSIDNET) ||
		(pfn_info->param->flags & (ENABLE_BKGRD_SCAN_MASK | ENABLE_BD_SCAN_MASK)))
		*nssid = directcnt + 1; /* broadcast scan needed */
	else
		*nssid = directcnt;

	/* allocate space for current bestn */
	if (pfn_info->param->mscan && !pfn_info->current_bestn && *nssid) {
		pfn_info->current_bestn = (wl_pfn_bestnet_t *)MALLOC(pfn_info->wlc->osh,
		                           pfn_info->bestnetsize);
		if (!pfn_info->current_bestn) {
			WL_INFORM(("wl%d: PFN bestn allocation failed\n",
			           pfn_info->wlc->pub->unit));
			if (pfn_info->param->mscan && pfn_info->numofscan &&
				(pfn_info->param->flags & REPORT_SEPERATELY_MASK))
				wl_pfn_send_event(pfn_info, NULL, 0, WLC_E_PFN_BEST_BATCHING);
			return;
		}
	}
	if (pfn_info->current_bestn)
		bzero(pfn_info->current_bestn, pfn_info->bestnetsize);
}

static void
wl_pfn_timer(void * data)
{
	wl_pfn_info_t		* pfn_info = (wl_pfn_info_t *)data;
	int32				bss_type;
	int				nssid;
	wlc_ssid_t			*ssidp;

	ASSERT(pfn_info);
	/* Allow processing only if the state is  PFN_SCAN_IDLE,
	  * scan is not suspended,  and driver is up
	  */
	if ((PFN_SCAN_IDLE != pfn_info->pfn_scan_state) ||
	    (pfn_info->intflag & (ENABLE << SUSPEND_BIT)))
		return;

	wl_pfn_prepare_for_scan(pfn_info, &ssidp, &nssid, &bss_type);
	/* Kick off scan when nssid != 0 */
	if (nssid &&
		wl_pfn_start_scan(pfn_info, ssidp, nssid, bss_type) == BCME_OK) {
		pfn_info->pfn_scan_state = PFN_SCAN_INPROGRESS;
	}
}

static int
wl_pfn_start_scan(wl_pfn_info_t * pfn_info, wlc_ssid_t	* ssid, int nssid, int bss_type)
{
	uint scanflags = 0;
#ifdef BCMDBG
	char				ssidbuf[SSID_FMT_BUF_LEN];
#endif /* BCMDBG */

	WL_TRACE(("wl%d: Scan request : Nssid = %d, SSID = %s, SSID len = %d BSS type = %d\n",
		pfn_info->wlc->pub->unit, nssid, (wlc_format_ssid(ssidbuf,
		ssid->SSID, ssid->SSID_len), ssidbuf), ssid->SSID_len, bss_type));

	pfn_info->cur_scan_type = (int16)bss_type;

#ifdef NLO
	/* handling suppress ssid option */
	wl_pfn_cfg_supprs_ssid(pfn_info, TRUE);
	/* handling network offload option */
	wlc_iovar_setint(pfn_info->wlc, "nlo",
		(pfn_info->param->flags & ENABLE_NET_OFFLOAD_BIT) ? 1 : 0);
#endif /* NLO */

	scanflags |= WL_SCANFLAGS_OFFCHAN;
	if (pfn_info->intflag & (ENABLE << PROHIBITED_BIT))
		scanflags |= WL_SCANFLAGS_PROHIBITED;

	/* Issue scan a scan which could be broadcast or directed scan */
	/* We don't check return value here because if wlc_scan_request() returns an error,
	 * the callback function wl_pfn_scan_complete() should have been called and
	 * PFN_SCAN_INPROGRESS flag should have been cleared
	 */
	if (!pfn_info->chanspec_count) { /* all channels */
		return wlc_scan_request_ex(pfn_info->wlc, bss_type, &ether_bcast, nssid, ssid,
		    DOT11_SCANTYPE_ACTIVE,	-1, -1,	-1,	-1, NULL, 0, 0, FALSE,
		    wl_pfn_scan_complete, pfn_info, WLC_ACTION_PNOSCAN, scanflags,
		    NULL, NULL, NULL);
	} else {
		return wlc_scan_request_ex(pfn_info->wlc, bss_type, &ether_bcast, nssid, ssid,
		    DOT11_SCANTYPE_ACTIVE,	-1, -1, -1, -1,	pfn_info->chanspec_list,
		    pfn_info->chanspec_count, 0, FALSE, wl_pfn_scan_complete, pfn_info,
		    WLC_ACTION_PNOSCAN, scanflags, NULL, NULL, NULL);
	}
}

int
wl_pfn_scan_in_progress(wl_pfn_info_t * pfn_info)
{
	return (pfn_info->pfn_scan_state == PFN_SCAN_INPROGRESS);
}

static void
wl_pfn_ageing(wl_pfn_info_t * pfn_info, uint32 now)
{
	wl_pfn_internal_t	    * pfn_internal;
	wl_pfn_bdcast_list_t    * bdcast_list;
	int                     i;
	wl_pfn_bssid_list_t *bssid_list;

#ifdef BCMDBG
	char					ssidbuf[SSID_FMT_BUF_LEN];
#endif /* BCMDBG */

	/* Evaluate PFN list for disappearence of network */
	if ((pfn_info->reporttype == WL_PFN_REPORT_SSIDNET) ||
		(pfn_info->reporttype == WL_PFN_REPORT_ALLNET)) {
		for (i = 0; i < pfn_info->count; i++) {
			pfn_internal = pfn_info->pfn_internal[i];
			if ((pfn_internal->network_found == PFN_NET_ALREADY_FOUND) &&
				((now - pfn_internal->time_stamp) >=
				(uint32)pfn_info->param->lost_network_timeout)) {
				WL_TRACE(("wl%d: Lost SSID = %s elapsed time = %d\n",
				    pfn_info->wlc->pub->unit,
				    (wlc_format_ssid(ssidbuf, pfn_internal->pfn.ssid.SSID,
				    pfn_internal->pfn.ssid.SSID_len), ssidbuf),
				    now - pfn_internal->time_stamp));
				if (!(pfn_internal->pfn.flags & WL_PFN_SUPPRESSLOST_MASK)) {
					pfn_internal->network_found = PFN_NET_JUST_LOST;
					pfn_info->lostcnt++;
					pfn_info->lostcnt_ssid++;
					ASSERT(pfn_info->availbsscnt);
					if (--pfn_info->availbsscnt == 0) {
						if (BCME_OK != wl_pfn_send_event(pfn_info, NULL,
							0, WLC_E_PFN_SCAN_ALLGONE)) {
							WL_ERROR(("wl%d: ALLGONE fail\n",
							      pfn_info->wlc->pub->unit));
						}
						pfn_info->none_cnt = 0xff;
					}
				} else {
					pfn_internal->network_found = PFN_NET_NOT_FOUND;
				}
			} else if ((pfn_internal->network_found == PFN_NET_JUST_FOUND) &&
				((now - pfn_internal->time_stamp) >=
				(uint32)pfn_info->param->lost_network_timeout)) {
				pfn_internal->network_found = PFN_NET_NOT_FOUND;
				ASSERT(pfn_info->foundcnt);
				pfn_info->foundcnt--;
				pfn_info->foundcnt_ssid--;
				if (!(pfn_internal->pfn.flags & WL_PFN_SUPPRESSLOST_MASK)) {
					ASSERT(pfn_info->availbsscnt);
					pfn_info->availbsscnt--;
				}
			}
		}
	}
	/* Evaluate bssid_list for disappearence of network */
	if ((pfn_info->reporttype == WL_PFN_REPORT_BSSIDNET) ||
		(pfn_info->reporttype == WL_PFN_REPORT_ALLNET)) {
		for (i = 0; i < BSSIDLIST_MAX; i++) {
			bssid_list = pfn_info->bssid_list[i];
			while (bssid_list) {
				if (bssid_list->pbssidinfo &&
				    (bssid_list->pbssidinfo->network_found ==
				                        PFN_NET_ALREADY_FOUND) &&
				    ((now - bssid_list->pbssidinfo->time_stamp) >=
					(uint32)pfn_info->param->lost_network_timeout)) {
					if (!(bssid_list->flags & WL_PFN_SUPPRESSLOST_MASK)) {
						bssid_list->pbssidinfo->network_found =
						                    PFN_NET_JUST_LOST;
						pfn_info->lostcnt++;
						pfn_info->lostcnt_bssid++;
						ASSERT(pfn_info->availbsscnt);
						if (--pfn_info->availbsscnt == 0) {
							if (BCME_OK != wl_pfn_send_event(pfn_info,
							    NULL, 0, WLC_E_PFN_SCAN_ALLGONE)) {
								WL_ERROR(("wl%d: ALLGONE fail\n",
								      pfn_info->wlc->pub->unit));
							}
							pfn_info->none_cnt = STOP_REPORTLOST;
						}
					} else {
						MFREE(pfn_info->wlc->osh, bssid_list->pbssidinfo,
						      sizeof(wl_pfn_bssidinfo_t));
						bssid_list->pbssidinfo = NULL;
					}
				} else if (bssid_list->pbssidinfo &&
				    (bssid_list->pbssidinfo->network_found == PFN_NET_JUST_FOUND) &&
					((now - bssid_list->pbssidinfo->time_stamp) >=
					(uint32)pfn_info->param->lost_network_timeout)) {
					bssid_list->pbssidinfo->network_found = PFN_NET_NOT_FOUND;
					ASSERT(pfn_info->foundcnt);
					pfn_info->foundcnt--;
					ASSERT(pfn_info->availbsscnt);
					pfn_info->availbsscnt--;
				}
				bssid_list = bssid_list->next;
			}
		}
	}
	/* Evaluate broadcast list for disappearence of network */
	if ((pfn_info->reporttype == WL_PFN_REPORT_ALLNET) &&
		(pfn_info->param->flags & ENABLE_BKGRD_SCAN_MASK)) {
		bdcast_list = pfn_info->bdcast_list;
		while (bdcast_list) {
			/* Check to see if given SSID is gone */
			if ((now - bdcast_list->bssidinfo.time_stamp) >=
				(uint32)pfn_info->param->lost_network_timeout) {
				WL_TRACE(("wl%d: Broadcast lost SSID = %s elapsed time = %d\n",
				    pfn_info->wlc->pub->unit, (wlc_format_ssid(ssidbuf,
				    bdcast_list->bssidinfo.ssid.SSID,
				    bdcast_list->bssidinfo.ssid.SSID_len), ssidbuf),
				    now - bdcast_list->bssidinfo.time_stamp));
				if (bdcast_list->bssidinfo.network_found == PFN_NET_ALREADY_FOUND) {
					bdcast_list->bssidinfo.network_found = PFN_NET_JUST_LOST;
					pfn_info->lostcnt++;
					ASSERT(pfn_info->availbsscnt);
					if (--pfn_info->availbsscnt == 0) {
						if (BCME_OK != wl_pfn_send_event(pfn_info, NULL,
							0, WLC_E_PFN_SCAN_ALLGONE)) {
							WL_ERROR(
							   ("wl%d: PFN_SCAN_ALLGONE event fail\n",
							     pfn_info->wlc->pub->unit));
						}
						pfn_info->none_cnt = STOP_REPORTLOST;
					}
				} else if (bdcast_list->bssidinfo.network_found ==
				                                        PFN_NET_JUST_FOUND) {
					bdcast_list->bssidinfo.network_found = PFN_NET_NOT_FOUND;
					ASSERT(pfn_info->foundcnt);
					pfn_info->foundcnt--;
					ASSERT(pfn_info->availbsscnt);
					pfn_info->availbsscnt--;
				}
			}
			bdcast_list = bdcast_list->next;
		}
	}
}

void
wl_pfn_event(wl_pfn_info_t * pfn_info, wlc_event_t * e)
{
	return;
}

static int
wl_pfn_allocate_bdcast_node(wl_pfn_info_t * pfn_info, uint8 ssid_len, uint8 * ssid,
                                      uint32 now)
{
	wl_pfn_bdcast_list_t * node_ptr;

	/* Allocate for node */
	if (!(node_ptr = (wl_pfn_bdcast_list_t *) MALLOC(pfn_info->wlc->osh,
		sizeof(wl_pfn_bdcast_list_t)))) {
		WL_ERROR(("wl%d: PFN: MALLOC failed, size = %d\n", pfn_info->wlc->pub->unit,
			sizeof(wl_pfn_bdcast_list_t)));
		if (pfn_info->param->mscan && pfn_info->numofscan &&
			(pfn_info->param->flags & REPORT_SEPERATELY_MASK))
			wl_pfn_send_event(pfn_info, NULL, 0, WLC_E_PFN_BEST_BATCHING);
		return BCME_NOMEM;
	}

	/* Copy the ssid info */
	memcpy(node_ptr->bssidinfo.ssid.SSID, ssid, ssid_len);
	node_ptr->bssidinfo.ssid.SSID_len = ssid_len;
	node_ptr->bssidinfo.time_stamp = now;

	/* Add the new node to begining of the list */
	node_ptr->next = pfn_info->bdcast_list;
	pfn_info->bdcast_list = node_ptr;

	return BCME_OK;
}

/* free data when return due to failure, should done in scan_complete */
static int
wl_pfn_send_event(wl_pfn_info_t * pfn_info, void *data, uint32 datalen, int event_type)
{
	wlc_info_t		* wlc;
	wlc_event_t		* e;

	wlc = pfn_info->wlc;

	e = wlc_event_alloc(wlc->eventq);

	if (e == NULL) {
		WL_ERROR(("wl%d: PFN wlc_event_alloc failed\n", wlc->pub->unit));
		return BCME_NOMEM;
	}

	e->event.event_type = event_type;
	e->event.status = WLC_E_STATUS_SUCCESS;

	/* Send the SSID name and length as a data */
	e->event.datalen = datalen;
	e->data = data;

	wlc_event_if(wlc, wlc->cfg, e, NULL);

#ifdef NLO
	wlc_process_event(wlc, e);
#else
	wlc_eventq_enq(wlc->eventq, e);
#endif /* NLO */
	return BCME_OK;
}
bool
wl_pfn_scan_state_enabled(wlc_info_t *wlc)
{
	wl_pfn_info_t *pfn_info = (wl_pfn_info_t *)wlc->pfn;
	return !(pfn_info->pfn_scan_state == PFN_SCAN_DISABLED);
}
#endif /* WLPFN */
