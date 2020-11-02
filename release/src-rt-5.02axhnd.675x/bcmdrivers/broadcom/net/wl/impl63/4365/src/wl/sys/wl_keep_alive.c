/*
 * Keep-alive offloading.
 * @file
 * @brief
 * This feature implements periodic keep-alive packet transmission offloading.
 * The intended purpose is to keep an active session within a network address
 * translator (NAT) with a public server. This allows incoming packets sent
 * by the public server to the STA to traverse the NAT.
 *
 * An example application is to keep an active session between the STA and
 * a call control server in order for the STA to be able to receive incoming
 * voice calls.
 *
 * The keep-alive functionality is offloaded from the host processor to the
 * WLAN processor to eliminate the need for the host processor to wake-up while
 * it is idle; therefore, conserving power.
 *
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
 * $Id: wl_keep_alive.c 708017 2017-06-29 14:11:45Z $
 *
 */

/* ---- Include Files ---------------------------------------------------- */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>

#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc_pio.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <proto/ethernet.h>

#include <wl_export.h>
#include <wl_keep_alive.h>
#include <wlc_scb.h>

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#define LOCAL_DEBUG	0

#if LOCAL_DEBUG
#undef WL_TRACE
#define WL_TRACE(arg)	printf arg
#endif // endif

/* wlc_pub_t struct access macros */
#define WLCUNIT(info)	((info)->wlc->pub->unit)
#define WLCOSH(info)	((info)->wlc->osh)

/* Lower bound for periodic transmit interval. */
#define MIN_PERIOD_MSEC		300

/* IOVar table */
enum {
	/* Get/set keep-alive packet and retransmission interval. */
	IOV_KEEP_ALIVE,
	IOV_MKEEP_ALIVE
};

typedef struct mkeep_alive_indiv {
	/* Pointer back to wlc structure */
	wl_keep_alive_info_t		*info;
	uint32			period_msec;
	/* Size, in bytes, of packet to transmit. */
	uint16			len_bytes;
	uint16			pad;
	/* Variable length array of packet to transmit. Packet contents should include */
	/* the entire ethernet packet (ethernet header, IP header, UDP header, and UDP */
	/* payload) specified in network byte order.                                   */
	char			*pkt_data;
	/* Periodic timer used to transmit packet. */
	struct wl_timer		*timer;
	wlc_bsscfg_t* bsscfg;
	uint32			override_period_msec;
} mkeep_alive_indiv_t;

/* Keep-alive private info structure. */
struct wl_keep_alive_info {
	wlc_info_t *wlc;
	int keep_alive_count;
	int apppkt_malloc_failed;
	uint16 txoff;
	uint16 pad;
	mkeep_alive_indiv_t mkeep_alive[1];
};

#define MKEEP_ALIVE_INFOSIZE(n) (OFFSETOF(wl_keep_alive_info_t, mkeep_alive) +\
	((n)*sizeof(mkeep_alive_indiv_t)))

/* ---- Private Variables ------------------------------------------------ */

static const bcm_iovar_t keep_alive_iovars[] = {
	{
		"keep_alive",
		IOV_KEEP_ALIVE,
		(0),
		IOVT_BUFFER,
		WL_KEEP_ALIVE_FIXED_LEN
	},
	{
		"mkeep_alive",
		IOV_MKEEP_ALIVE,
		(0),
		IOVT_BUFFER,
		WL_MKEEP_ALIVE_FIXED_LEN
	},

	{NULL, 0, 0, 0, 0 }
};

/* ---- Private Function Prototypes -------------------------------------- */

static int keep_alive_doiovar
(
	void 			*hdl,
	const bcm_iovar_t	*vi,
	uint32 			actionid,
	const char 		*name,
	void 			*p,
	uint			plen,
	void 			*a,
	int 			alen,
	int 			vsize,
	struct wlc_if 		*wlcif
);

static void keep_alive_timer_callback(void *arg);
static void keep_alive_timer_update(mkeep_alive_indiv_t *indiv, int period_msec, bool enable);
static int  mkeep_alive_set(mkeep_alive_indiv_t* indiv,
	                    int len_bytes,
	                    int period_msec,
	                    uint8 *pkt_data,
	                    wlc_bsscfg_t *bss);

/* ---- Functions -------------------------------------------------------- */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* ----------------------------------------------------------------------- */
wl_keep_alive_info_t *
BCMATTACHFN(wl_keep_alive_attach)(wlc_info_t *wlc)
{
	wl_keep_alive_info_t *info;
	int i;
	int keep_alive_count = WL_MKEEP_ALIVE_IDMAX + 1;

	/* Allocate keep-alive private info struct. */
	info = MALLOCZ(wlc->pub->osh, MKEEP_ALIVE_INFOSIZE(keep_alive_count));
	if (info == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	/* Init keep-alive private info struct. */
	info->keep_alive_count = keep_alive_count;
	info->wlc = wlc;
	info->txoff = TXOFF;
	wlc->keepalive = info;

	/* Create periodic timer for packet transmission. */
	for (i = 0; i < info->keep_alive_count; i++) {
		info->mkeep_alive[i].info = info;
		info->mkeep_alive[i].timer = wl_init_timer(wlc->wl,
			keep_alive_timer_callback,
			&info->mkeep_alive[i],
			"mkeep_alive");
		if (info->mkeep_alive[i].timer == NULL) {
			WL_ERROR(("wl%d: %s: wl_init_timer failed\n", wlc->pub->unit,
			__FUNCTION__));
			goto fail;
		}
	}

	/* Register keep-alive module. */
	if (wlc_module_register(wlc->pub,
	                        keep_alive_iovars,
	                        "keep_alive",
	                        info,
	                        keep_alive_doiovar,
	                        NULL,
	                        NULL,
	                        NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	return (info);

fail:
	if (info != NULL) {
		/* free any timer resources that have been allocated */
		for (i = 0; i < info->keep_alive_count; i++) {
			if (info->mkeep_alive[i].timer != NULL) {
				wl_free_timer(info->wlc->wl, info->mkeep_alive[i].timer);
				info->mkeep_alive[i].timer = NULL;
			}
		}
		MFREE(WLCOSH(info), info, MKEEP_ALIVE_INFOSIZE(keep_alive_count));
	}
	return (NULL);
}

/* ----------------------------------------------------------------------- */
void
BCMATTACHFN(wl_keep_alive_detach)(wl_keep_alive_info_t *info)
{
	int i;

	if (info == NULL)
		return;

	wlc_module_unregister(info->wlc->pub, "keep_alive", info);

	for (i = 0; i < info->keep_alive_count; i++) {
		if (info->mkeep_alive[i].timer != NULL) {
			wl_free_timer(info->wlc->wl, info->mkeep_alive[i].timer);
			info->mkeep_alive[i].timer = NULL;
		}
		if (info->mkeep_alive[i].pkt_data != NULL)
			MFREE(WLCOSH(info),
			info->mkeep_alive[i].pkt_data,
			info->mkeep_alive[i].len_bytes);
	}

	if (info != NULL)
		MFREE(WLCOSH(info), info,
		MKEEP_ALIVE_INFOSIZE(info->keep_alive_count));
}

/* ----------------------------------------------------------------------- */
int
wl_keep_alive_up(wl_keep_alive_info_t *info)
{
	int i;
	mkeep_alive_indiv_t *indiv;
	/* Restart the timers if they are configured */
	for (i = 0; i < info->keep_alive_count; i++) {
		indiv = &info->mkeep_alive[i];

		if ((indiv->period_msec != 0) && (indiv->timer != NULL)) {
			keep_alive_timer_update(indiv, indiv->period_msec, TRUE);
		}
	}
	return (0);
}

/* ----------------------------------------------------------------------- */
unsigned int
wl_keep_alive_down(wl_keep_alive_info_t *info)
{
	unsigned int callbacks = 0;
	int i;

	/* Cancel the periodic timer. */
	for (i = 0; i < info->keep_alive_count; i++) {
		if (!wl_del_timer(info->wlc->wl, info->mkeep_alive[i].timer))
			callbacks++;
	}

	return (callbacks);
}

static bool
mkeep_alive_bssconfig_check(wlc_info_t *wlc, wlc_bsscfg_t *bss)
{
	if (bss != wlc->cfg)
		return FALSE;

	if (!BSSCFG_STA(bss))
		return FALSE;

	return TRUE;
}

static int
mkeep_alive_set(mkeep_alive_indiv_t* indiv,
	int len_bytes,
	int period_msec,
	uint8 *pkt_data,
	wlc_bsscfg_t *bss)
{
	int err = BCME_OK;
	uint32 override_period = indiv->override_period_msec;
	wlc_info_t *wlc = indiv->info->wlc;

	/* Validate ethernet packet length. */
	if ((len_bytes != 0) && (len_bytes < ETHER_HDR_LEN)) {
		return BCME_BADARG;
	}

	/* Validate transmit period is greater than min bound. */
	if ((period_msec != 0) && (period_msec < MIN_PERIOD_MSEC)) {
		return BCME_RANGE;
	}
	if (bss == NULL)
		return BCME_BADARG;
	if (!mkeep_alive_bssconfig_check(wlc, bss))
		return BCME_BADARG;
	if (override_period && (len_bytes == 0))
		return BCME_BADARG;

	/* Disable timer while we modify timer state to avoid race-conditions. */
	keep_alive_timer_update(indiv, indiv->period_msec, FALSE);

	/* disable this entry */
	indiv->period_msec = 0;

	/* Free memory used to store user specified packet data contents. */
	if (indiv->pkt_data != NULL) {
		MFREE(WLCOSH(indiv->info), indiv->pkt_data, indiv->len_bytes);
		indiv->pkt_data = NULL;
	}

	if (0 != len_bytes)
	{
		/* Allocate memory used to store user specified packet data contents. */
		indiv->pkt_data = MALLOC(WLCOSH(indiv->info), len_bytes);
		if (indiv->pkt_data == NULL) {
			WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
			WLCUNIT(indiv->info), __FUNCTION__, MALLOCED(WLCOSH(indiv->info))));
			return BCME_NOMEM;
		}
		bcopy(pkt_data, indiv->pkt_data, len_bytes);
	}

	indiv->bsscfg = bss;

	/* Store new timer and packet attributes. */
	bcopy(&len_bytes, &indiv->len_bytes, sizeof(len_bytes));
	bcopy(&period_msec, &indiv->period_msec, sizeof(period_msec));

	if (override_period)
		period_msec = override_period;

	if (period_msec) {
		/* Start the periodic timer with new values. */
		keep_alive_timer_update(indiv, indiv->period_msec, TRUE);
	}
	return err;
}

/*
*****************************************************************************
* Function:   keep_alive_doiovar
*
* Purpose:    Handles keep-alive related IOVars.
*
* Parameters:
*
* Returns:    0 on success.
*****************************************************************************
*/
static int
keep_alive_doiovar
(
	void 			*hdl,
	const bcm_iovar_t	*vi,
	uint32 			actionid,
	const char 		*name,
	void 			*p,
	uint 			plen,
	void 			*a,
	int 			alen,
	int 			vsize,
	struct wlc_if 		*wlcif
)
{
	wl_keep_alive_info_t	*info = hdl;
	wl_keep_alive_pkt_t	*keep_alive_pkt;
	int err = BCME_OK;
	wl_mkeep_alive_pkt_t *mkeep_alive_pkt;
	int mkeepalive_index;
	uint32 period_msec;
	uint16 len_bytes;
	uint16  version;

	switch (actionid) {

	case IOV_SVAL(IOV_KEEP_ALIVE):
		keep_alive_pkt = a;
		memcpy(&period_msec, &keep_alive_pkt->period_msec, sizeof(period_msec));
		memcpy(&len_bytes, &keep_alive_pkt->len_bytes, sizeof(len_bytes));
		err = mkeep_alive_set(&info->mkeep_alive[0], len_bytes, period_msec,
		    keep_alive_pkt->data, wlc_bsscfg_find_by_wlcif(info->wlc, wlcif));
		break;

	case IOV_SVAL(IOV_MKEEP_ALIVE):
		mkeep_alive_pkt = a;
		memcpy(&version, &mkeep_alive_pkt->version, sizeof(version));
		memcpy(&period_msec, &mkeep_alive_pkt->period_msec, sizeof(period_msec));
		memcpy(&len_bytes, &mkeep_alive_pkt->len_bytes, sizeof(len_bytes));

		if (version != WL_MKEEP_ALIVE_VERSION) {
			err = BCME_VERSION;
			break;
		}
		if (mkeep_alive_pkt->keep_alive_id >= info->keep_alive_count) {
			err = BCME_BADARG;
			break;
		}
		err = mkeep_alive_set(&info->mkeep_alive[mkeep_alive_pkt->keep_alive_id],
			len_bytes,
			period_msec,
			mkeep_alive_pkt->data,
			wlc_bsscfg_find_by_wlcif(info->wlc, wlcif));
		break;

	case IOV_GVAL(IOV_MKEEP_ALIVE):
		mkeepalive_index = *((int*)p);
		mkeep_alive_pkt = a;
		if ((mkeepalive_index >= info->keep_alive_count) || (mkeepalive_index < 0)) {
			err = BCME_BADARG;
			break;
		}
		version = WL_MKEEP_ALIVE_VERSION;
		len_bytes = WL_MKEEP_ALIVE_FIXED_LEN;
		memcpy(&mkeep_alive_pkt->length, &len_bytes, sizeof(len_bytes));
		memcpy(&mkeep_alive_pkt->version, &version, sizeof(version));
		memcpy(&mkeep_alive_pkt->len_bytes,
		      &info->mkeep_alive[mkeepalive_index].len_bytes,
		      sizeof(mkeep_alive_pkt->len_bytes));

		memcpy(&mkeep_alive_pkt->keep_alive_id,
		       &mkeepalive_index,
		       sizeof(mkeep_alive_pkt->keep_alive_id));

		memcpy(&mkeep_alive_pkt->period_msec,
		       &info->mkeep_alive[mkeepalive_index].period_msec,
		       sizeof(mkeep_alive_pkt->period_msec));

		/* Check if the memory provided is sufficient */
		if (alen < (int)(WL_MKEEP_ALIVE_FIXED_LEN +
			info->mkeep_alive[mkeepalive_index].len_bytes)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		memcpy(mkeep_alive_pkt->data,
		      info->mkeep_alive[mkeepalive_index].pkt_data,
		      info->mkeep_alive[mkeepalive_index].len_bytes);
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/*
*****************************************************************************
* Function:   keep_alive_timer_update
*
* Purpose:    Enable/disable the periodic keep-alive timer.
*
* Parameters: info   (mod) Keep-alive context data.
*             enable (in)  Enable/disable timer.
*
* Returns:    Nothing.
*****************************************************************************
*/
static void
keep_alive_timer_update(mkeep_alive_indiv_t *indiv, int period_msec, bool enable)
{
	wlc_info_t *wlc = indiv->info->wlc;
	void *wl = wlc->wl;

	WL_TRACE(("wl%d: %s : state %d, timeout %d\n",
	          WLCUNIT(indiv->info), __FUNCTION__, enable, period_msec));

	wl_del_timer(wl, indiv->timer);

	if (!enable || !wlc->pub->up)
		return;

	wl_add_timer(wl, indiv->timer, period_msec, TRUE);
}

/*
 * remaining_time = period - |(now - used)|
 *
 * if (remaining_time <= precision) OK else reschedule(remaining_time)
 *
 */
/* XXX: One possibility for null-packets is to use one-shot timer but
 *      that would require an event/callback from association state
 *      machine to start the timer once the STA is associated.
 */
static bool
keep_alive_idle_time_check(mkeep_alive_indiv_t* indiv)
{
	wlc_info_t *wlc = indiv->info->wlc;
	struct scb_iter scbiter;
	struct scb *scb;
	uint32 period;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (BSSCFG_STA(SCB_BSSCFG(scb)) && SCB_ASSOCIATED(scb) &&
		    (SCB_BSSCFG(scb) == indiv->bsscfg)) {
				return TRUE;
		}
	}
	period = indiv->override_period_msec ? indiv->override_period_msec: indiv->period_msec;
	keep_alive_timer_update(indiv, period, TRUE);
	return FALSE;
}

/*
*****************************************************************************
* Function:   keep_alive_timer_callback
*
* Purpose:    Callback timer function. Send the specified data packet.
*
* Parameters: arg (mode) User registered timer argument.
*
* Returns:    Nothing.
*****************************************************************************
*/
static void
keep_alive_timer_callback(void *arg)
{
	mkeep_alive_indiv_t *indiv;
	void *pkt;
	wlc_bsscfg_t *cfg;
	wlc_info_t *wlc;

	indiv = arg;
	wlc = indiv->info->wlc;

	WL_TRACE(("wl%d: %s : timeout %d\n",
	          WLCUNIT(indiv->info), __FUNCTION__, indiv->period_msec));

	cfg = indiv->bsscfg;
	/* check idle time and get bss if this callback was for a null-packet */
	if (indiv->len_bytes == 0) {
		if (!keep_alive_idle_time_check(indiv))
			return;
		wlc_sendnulldata(wlc, cfg, &cfg->BSSID, 0, 0, -1, NULL, NULL);
		return;
	}

	/* Allocate packet to send. */
	pkt = PKTGET(WLCOSH(indiv->info),
	             indiv->len_bytes + indiv->info->txoff,
	             TRUE);

	if (pkt == NULL) {
		indiv->info->apppkt_malloc_failed++;
		WL_ERROR(("wl%d: %s : failed to allocate tx pkt\n",
		          WLCUNIT(indiv->info), __FUNCTION__));

		return;
	}

	/* Populate packet with user specified data contents. */
	PKTPULL(WLCOSH(indiv->info), pkt, indiv->info->txoff);
	bcopy(indiv->pkt_data, PKTDATA(WLCOSH(indiv->info), pkt), indiv->len_bytes);
	WLPKTTAG(pkt)->flags3 |= WLF3_NO_PMCHANGE;
	wlc_sendpkt(wlc, pkt, cfg->wlcif);
}

int
wl_keep_alive_upd_override_period(wlc_info_t *wlc, uint8 mkeepalive_index, uint32 override_period)
{
	uint32 period_msec;
	mkeep_alive_indiv_t *indiv;
	int i;
	wl_keep_alive_info_t *keepalive = wlc->keepalive;

	if (mkeepalive_index >= keepalive->keep_alive_count)
		return BCME_BADARG;
	indiv = &keepalive->mkeep_alive[mkeepalive_index];
	if (indiv->len_bytes == 0)
		return BCME_BADARG;

	/* Disable timer while we modify timer state to avoid race-conditions */
	keep_alive_timer_update(indiv, indiv->period_msec, FALSE);

	/* Disable override period from any earlier configuration */
	for (i = 0; i < keepalive->keep_alive_count; i++) {
		mkeep_alive_indiv_t *temp = &keepalive->mkeep_alive[i];
		if (i != mkeepalive_index && temp->override_period_msec != 0) {
			/* Disable timer while we modify timer state to avoid race-conditions */
			keep_alive_timer_update(temp, temp->period_msec, FALSE);
			temp->override_period_msec = 0;
			/* Start the periodic timer with new values. */
			keep_alive_timer_update(temp, temp->period_msec, TRUE);
			break;
		}
	}

	indiv->override_period_msec = override_period;

	period_msec = indiv->period_msec;
	if (override_period)
		period_msec = override_period;

	if (period_msec) {
		/* Start the periodic timer with new values. */
		keep_alive_timer_update(indiv, period_msec, TRUE);
	}
	return 0;
}
