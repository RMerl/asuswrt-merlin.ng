/*
 * Common (OS-independent) portion of
 * Broadcom support for Intel NetDetect interface.
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
 * $Id: wlc_net_detect.c 708017 2017-06-29 14:11:45Z $
 */

#if defined(NET_DETECT)

#include <epivers.h>
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>

#include <d11.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scan.h>
#include <wlc_scb.h>
#include <wl_export.h>
#ifdef WLOFFLD
#include <wlc_offloads.h>
#endif // endif
#include <wlc_wowl.h>
#include <wlc_net_detect.h>

/* NetDetect context structure */
typedef struct wlc_net_detect_ctxt {
    wlc_info_t		    *wlc;
    net_detect_config_t	    *nd_config;
    uint32		    nd_config_len;
} wlc_net_detect_ctxt_t;

typedef struct filtered_profile_list {
	uint32			num_nd_profiles;
	net_detect_profile_t	*nd_profile[NET_DETECT_MAX_PROFILES];
} filtered_profile_list_t;

/* wlc access macros */
#define WLCUNIT(net_detect_ctxt)    ((net_detect_ctxt)->wlc->pub->unit)
#define WLCPUB(net_detect_ctxt)	    ((net_detect_ctxt)->wlc->pub)
#define WLCOSH(net_detect_ctxt)	    ((net_detect_ctxt)->wlc->osh)
#define WLCCFG(net_detect_ctxt)	    ((net_detect_ctxt)->wlc->cfg)
#define WLCWLCIF(net_detect_ctxt)   ((net_detect_ctxt)->wlc->cfg->wlcif)
#define WLCWL(net_detect_ctxt)	    ((net_detect_ctxt)->wlc->wl)

/* IOVar table */
enum {
    IOV_NET_DETECT_FEATURES,
    IOV_NET_DETECT_STATE,
    IOV_NET_DETECT_ENABLE,
    IOV_NET_DETECT_DOWNLOAD_CONFIG,
    IOV_NET_DETECT_WAKE_DATA
};

static const bcm_iovar_t net_detect_iovars[] = {
	{"net_detect_features",
	IOV_NET_DETECT_FEATURES,
	(0),
	IOVT_BUFFER,
	sizeof(net_detect_adapter_features_t)
	},

	{"net_detect_state",
	IOV_NET_DETECT_STATE,
	(0),
	IOVT_BUFFER,
	sizeof(net_detect_config_t)
	},

	{"net_detect",
	IOV_NET_DETECT_ENABLE,
	(IOVF_WHL),
	IOVT_UINT32,
	0
	},

	{"net_detect_download_config",
	IOV_NET_DETECT_DOWNLOAD_CONFIG,
	(IOVF_WHL),
	IOVT_UINT32,
	0
	},

	{"net_detect_wake_data",
	IOV_NET_DETECT_WAKE_DATA,
	(0),
	IOVT_BUFFER,
	sizeof(net_detect_wake_data_t)
	},

	{NULL, 0, 0, 0, 0 }
};

/* Forward declarations for functions registered for this module */
static int wlc_net_detect_doiovar(
		    void                *hdl,
		    const bcm_iovar_t   *vi,
		    uint32              actionid,
		    const char          *name,
		    void                *p,
		    uint                plen,
		    void                *a,
		    int                 alen,
		    int                 vsize,
		    struct wlc_if       *wlcif);

/* Forward declarations for wlc_net_detect_ctxt_t-based functions */
static int wlc_net_detect_adapter_get_features(
		    wlc_net_detect_ctxt_t *net_detect_ctxt,
		    net_detect_adapter_features_t *nd_adapter_features);

static int wlc_net_detect_set_config(
		    wlc_net_detect_ctxt_t *net_detect_ctxt,
		    net_detect_config_t *config);

static int wlc_net_detect_download_config(wlc_net_detect_ctxt_t *net_detect_ctxt);

static int wlc_net_detect_filter_profile_list(
		    wlc_net_detect_ctxt_t	*net_detect_ctxt,
		    wl_pfn_list_t		*pfn_list,
		    filtered_profile_list_t	*filtered_list);

static int wlc_net_detect_get_wake_data(
		    wlc_net_detect_ctxt_t *net_detect_ctxt,
		    net_detect_wake_data_t *nd_wake_data);

/* **** Public Functions *** */
/*
 * Initialize the net detect private context and resources.
 * Returns a pointer to the net detect private context, NULL on failure.
 */
wlc_net_detect_ctxt_t *
BCMATTACHFN(wlc_net_detect_attach)(wlc_info_t *wlc)
{
	wlc_net_detect_ctxt_t *net_detect_ctxt;

	/* allocate net detect context struct */
	net_detect_ctxt = MALLOC(wlc->osh, sizeof(wlc_net_detect_ctxt_t));
	if (net_detect_ctxt == NULL) {
	    WL_ERROR(("wl%d: %s: net_detect_ctxt MALLOC failed; total mallocs %d bytes\n",
	        WLCUNIT(net_detect_ctxt),
	        __FUNCTION__,
	        MALLOCED(wlc->osh)));

	    return NULL;
	}

	/* init net detect context struct */
	bzero(net_detect_ctxt, sizeof(wlc_net_detect_ctxt_t));

	net_detect_ctxt->wlc = wlc;

	/* allocate cache for ND parameters */
	net_detect_ctxt->nd_config_len =
	    OFFSETOF(net_detect_config_t, nd_profile_list) +
	    (NET_DETECT_MAX_PROFILES * sizeof(net_detect_profile_t));

	net_detect_ctxt->nd_config = MALLOC(wlc->osh, net_detect_ctxt->nd_config_len);

	if (net_detect_ctxt->nd_config == NULL) {
	    WL_ERROR(("wl%d: %s: nd_config MALLOC failed; total mallocs %d bytes\n",
	        WLCUNIT(net_detect_ctxt),
	        __FUNCTION__,
	        MALLOCED(wlc->osh)));

	    MFREE(wlc->osh, net_detect_ctxt, sizeof(wlc_net_detect_ctxt_t));

	    return NULL;
	}

	/* init net detect config struct */
	bzero(net_detect_ctxt->nd_config, net_detect_ctxt->nd_config_len);

	/* register module */
	if (wlc_module_register(
			    wlc->pub,
			    net_detect_iovars,
			    "net_detect",
			    net_detect_ctxt,
			    wlc_net_detect_doiovar,
			    NULL,
			    NULL,
			    NULL)) {
				WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
				    WLCUNIT(net_detect_ctxt),
				    __FUNCTION__));

				goto fail;
			    }

	/* Enable the module. */
	NET_DETECT_ENAB(WLCPUB(net_detect_ctxt)) = TRUE;

	return net_detect_ctxt;

fail:
	MFREE(wlc->osh, net_detect_ctxt->nd_config, net_detect_ctxt->nd_config_len);

	MFREE(wlc->osh, net_detect_ctxt, sizeof(wlc_net_detect_ctxt_t));

	return NULL;
}

/*
 * Release net detect private context and resources.
 */
void
BCMATTACHFN(wlc_net_detect_detach)(wlc_net_detect_ctxt_t *net_detect_ctxt)
{
	wlc_bsscfg_t    *bsscfg;
	uint32          i;

	WL_NET_DETECT(("wl%d: wlc_net_detect_detach()\n", WLCUNIT(net_detect_ctxt)));

	if (!net_detect_ctxt)
	    return;

	/* Disable the module. */
	NET_DETECT_ENAB(WLCPUB(net_detect_ctxt)) = FALSE;

	/* Unregister the module */
	wlc_module_unregister(WLCPUB(net_detect_ctxt), "net_detect", net_detect_ctxt);

	MFREE(WLCOSH(net_detect_ctxt), net_detect_ctxt->nd_config, net_detect_ctxt->nd_config_len);

	MFREE(WLCOSH(net_detect_ctxt), net_detect_ctxt, sizeof(wlc_net_detect_ctxt_t));
}

/* **** Private Functions *** */
/*
 * Handle net detect-related iovars
 */
static int
BCMATTACHFN(wlc_net_detect_doiovar)(
	void                *hdl,
	const bcm_iovar_t   *vi,
	uint32              actionid,
	const char          *name,
	void                *p,
	uint                plen,
	void                *a,
	int                 alen,
	int                 vsize,
	struct wlc_if       *wlcif)
{
	wlc_net_detect_ctxt_t	*net_detect_ctxt = hdl;
	int32			*ret_int_ptr = (int32 *)a;
	int			err = BCME_OK;

	WL_NET_DETECT(("wl%d: net_detect_doiovar()\n", WLCUNIT(net_detect_ctxt)));

	BCM_REFERENCE(vi);
	BCM_REFERENCE(name);
	BCM_REFERENCE(p);
	BCM_REFERENCE(plen);
	BCM_REFERENCE(vsize);

	switch (actionid) {
	    case IOV_GVAL(IOV_NET_DETECT_FEATURES):
		if (alen < sizeof(net_detect_adapter_features_t))
		    return BCME_BUFTOOSHORT;

		return wlc_net_detect_adapter_get_features(
			    net_detect_ctxt,
			    (net_detect_adapter_features_t *)a);

	    case IOV_GVAL(IOV_NET_DETECT_STATE):
		if (alen < sizeof(net_detect_config_t))
		    return BCME_BUFTOOSHORT;

		bcopy(&net_detect_ctxt->nd_config, a, sizeof(net_detect_config_t));

		break;

	    case IOV_SVAL(IOV_NET_DETECT_STATE):
		if (alen < sizeof(net_detect_config_t))
		    return BCME_BUFTOOSHORT;

		return wlc_net_detect_set_config(net_detect_ctxt, (net_detect_config_t *)a);

	    case IOV_GVAL(IOV_NET_DETECT_ENABLE):
	        *ret_int_ptr = (int32)net_detect_ctxt->nd_config->nd_enabled;
	        break;

	    case IOV_SVAL(IOV_NET_DETECT_ENABLE):
		if (*ret_int_ptr != 0) {
			WL_ERROR(("%s: NetDetect cannot be enabled via IOVAR\n", __FUNCTION__));

			return BCME_BADARG;
		}

	        net_detect_ctxt->nd_config->nd_enabled = FALSE;
	        break;

	    case IOV_SVAL(IOV_NET_DETECT_DOWNLOAD_CONFIG):
	        return wlc_net_detect_download_config(net_detect_ctxt);

	    case IOV_GVAL(IOV_NET_DETECT_WAKE_DATA):
		if (alen < sizeof(net_detect_wake_data_t))
		    return BCME_BUFTOOSHORT;

		return wlc_net_detect_get_wake_data(net_detect_ctxt, (net_detect_wake_data_t *)a);

	    default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/*
 * Return the NetDetect features for the adapter
 */
static int
wlc_net_detect_adapter_get_features(
	wlc_net_detect_ctxt_t *net_detect_ctxt,
	net_detect_adapter_features_t *nd_adapter_features)
{
	wlc_info_t  *wlc = net_detect_ctxt->wlc;

	bzero(nd_adapter_features, sizeof(net_detect_adapter_features_t));

	if (wlc_wowl_cap(wlc) == TRUE) {
		nd_adapter_features->wowl_enabled		= TRUE;
		nd_adapter_features->net_detect_enabled = TRUE;
	} else
	{
		nd_adapter_features->wowl_enabled		= FALSE;
		nd_adapter_features->net_detect_enabled = FALSE;
	}

	/*
	 * Return NLO state to SDK. This is not passed up to the ISCT/NetDetect stack.
	 */
#if (NDISVER >= 0x0630)
	nd_adapter_features->nlo_enabled = TRUE;
#else
	nd_adapter_features->nlo_enabled = FALSE;
#endif	/* (NDISVER >= 0x0630) */

	return BCME_OK;
}

static int
wlc_net_detect_set_config(
	wlc_net_detect_ctxt_t *net_detect_ctxt,
	net_detect_config_t *nd_config)
{
	wlc_info_t		  *wlc;
	net_detect_profile_list_t *nd_profile_list;

	if (nd_config->nd_enabled == TRUE) {
		nd_profile_list = &nd_config->nd_profile_list;
		wlc		= net_detect_ctxt->wlc;

		/*
		 * Verify the parameters.
		 */
		if (WLOFFLD_CAP(wlc) != TRUE) {
			/*
			 * We're running on a non-ARM chip that isn't capable of supporting offloads
			 */
			WL_ERROR(("%s: Not running on an 11AC ARM chip\n", __FUNCTION__));

			return BCME_UNSUPPORTED;
		}

		if ((nd_config->wake_if_connected == FALSE) &&
			(nd_config->wake_if_disconnected == FALSE)) {
			/* At least one setting must be enabled */
			WL_ERROR(("%s: invalid wake settings\n",  __FUNCTION__));

			return BCME_BADARG;
		}

		if (nd_config->wake_if_connected == TRUE) {
			/* We don't support this */
			WL_ERROR(("%s: wake_if_connected is set\n", __FUNCTION__));

			return BCME_BADARG;
		}

		if ((nd_profile_list->num_nd_profiles == 0) ||
		    (nd_profile_list->num_nd_profiles > NET_DETECT_MAX_PROFILES)) {
			WL_ERROR(("%s: Invalid num_nd_profiles value: %d\n",
			    __FUNCTION__, nd_profile_list->num_nd_profiles));

			return BCME_BADARG;
		}

		/*
		 * Save off the entire config struct. We do this to cache settings
		 * across S0/S3 transitions.
		 */
		bcopy(
			nd_config,
			net_detect_ctxt->nd_config,
			OFFSETOF(net_detect_config_t, nd_profile_list) +
			(nd_profile_list->num_nd_profiles * sizeof(net_detect_profile_t)));
	} else {
		/*
		 * Clear out the previous ND settings.
		 */
		bzero(net_detect_ctxt->nd_config, net_detect_ctxt->nd_config_len);
	}

	return BCME_OK;
}

/*
 * Filter NetDetect profile list based on PFN settings
 */
static int
wlc_net_detect_filter_profile_list(
	wlc_net_detect_ctxt_t	*net_detect_ctxt,
	wl_pfn_list_t		*pfn_list,
	filtered_profile_list_t *filtered_list)
{
	net_detect_config_t	    *nd_config = net_detect_ctxt->nd_config;
	net_detect_profile_list_t   *nd_profile_list = &nd_config->nd_profile_list;
	net_detect_profile_t	    *nd_profile;
	wl_pfn_t		    *pfn_profile;
	uint32			    i, j, num_profiles;

	num_profiles = 0;

	for (i = 0; i < nd_profile_list->num_nd_profiles; i++) {
		nd_profile = &nd_profile_list->nd_profile[i];

		for (j = 0; j < pfn_list->count; j++) {
			pfn_profile = &pfn_list->pfn[j];

			if ((pfn_profile->ssid.SSID_len == nd_profile->ssid.SSID_len) &&
			    (!bcmp(
				&pfn_profile->ssid.SSID,
				&nd_profile->ssid.SSID,
				nd_profile->ssid.SSID_len))) {
					/*
					 * TODO: Check crypto settings to make sure we have
					 * a match.
					 */
					filtered_list->nd_profile[num_profiles++] =
						nd_profile;

					break;
			}
		}
	}

	if (num_profiles == 0) {
		ASSERT(num_profiles > 0);

		return BCME_ERROR;
	}

	filtered_list->num_nd_profiles = num_profiles;

	return BCME_OK;
}

/*
 * Plumb the NetDetect parameters to ARM before we go to D3.
 */
static int
wlc_net_detect_download_config(wlc_net_detect_ctxt_t *net_detect_ctxt)
{
	wlc_info_t		    *wlc = net_detect_ctxt->wlc;
	net_detect_config_t	    *nd_config = net_detect_ctxt->nd_config;
	net_detect_profile_list_t   *nd_profile_list;
	filtered_profile_list_t     filtered_profile_list;
	net_detect_profile_t	    *nd_profile;
	scanol_params_t		    *scan_params;
	chanspec_t		    *chanspec_list, *scan_channels;
	uint32			    num_channels;
	uint32			    i, err;
	uint32			    ssid_bytes_needed, total_bytes_needed;
	wl_pfn_list_t		    *pfn_list;
	uint32			    pfn_list_bytes_needed;
	int			    pfn_enabled;

	/*
	 * Initialize the variables
	 */
	err		= BCME_OK;
	chanspec_list	= NULL;
	scan_params	= NULL;
	pfn_list	= NULL;

	WL_TRACE(("%s\n", __FUNCTION__));
	bzero(&filtered_profile_list, sizeof(filtered_profile_list_t));

	/*
	 * Verify the parameters to make sure we can do what we've been asked to do.
	 */
	if (nd_config->nd_enabled == FALSE) {
		/* Invalid state to plumb ND settings */
		WL_ERROR(("%s: failed. nd_enabled: %d\n",
			__FUNCTION__, nd_config->nd_enabled));

		err = BCME_ERROR;

		goto done;
	}

	if (wlc->cfg->associated == FALSE) {
		if (nd_config->wake_if_disconnected != TRUE) {
			WL_ERROR(("%s: Not enabled to wake when not associated state\n",
				__FUNCTION__));

			err = BCME_ERROR;

			goto done;
		}
	} else {
		if (nd_config->wake_if_connected == FALSE) {
			WL_ERROR(("%s: Not enabled to wake when associated state\n",
				__FUNCTION__));

			err = BCME_ERROR;

			goto done;
		}
	}

	/*
	 * Get the PFN list if PFN is enabled. If not, we'll use the profile list that was
	 * previously plumbed to us from the library.
	 */
	nd_profile_list = &nd_config->nd_profile_list;

	/* See if PFN scans are enabled */
	wlc_iovar_getint(wlc, "pfn", &pfn_enabled);

	if (pfn_enabled != FALSE) {
		pfn_list_bytes_needed =
			OFFSETOF(wl_pfn_list_t, pfn) + (MAX_PFN_LIST_COUNT * sizeof(wl_pfn_t));

		pfn_list = (wl_pfn_list_t *)MALLOC(wlc->osh, pfn_list_bytes_needed);

		err = wlc_iovar_op(
				wlc,
				"pfn_get",
				NULL,
				0,
				pfn_list,
				pfn_list_bytes_needed,
				IOV_GET,
				NULL);

		if (err != BCME_OK) {
			WL_ERROR(("%s: Failed to get PFN list: %d\n",  __FUNCTION__, err));

			goto done;
		}

		err = wlc_net_detect_filter_profile_list(
			net_detect_ctxt,
			pfn_list,
			&filtered_profile_list);

		if (err != BCME_OK) {
			WL_ERROR(("%s: Failed to filter PFN list: %d\n",  __FUNCTION__, err));

			goto done;
		}

	} else {
		filtered_profile_list.num_nd_profiles = nd_profile_list->num_nd_profiles;

		for (i = 0; i < filtered_profile_list.num_nd_profiles; i++) {
			filtered_profile_list.nd_profile[i] = &nd_profile_list->nd_profile[i];
		}
	}

	/*
	 * Get the channel list is a separate buffer
	 */
	chanspec_list = (chanspec_t *)MALLOC(wlc->osh, sizeof(chanspec_t) * MAXCHANNEL);
	if (!chanspec_list) {
		WL_ERROR(("%s: alloc buffer for chanspec_list failed\n",
			__FUNCTION__));

		err = BCME_NOMEM;

		goto done;
	}

	wlc_scan_default_channels(wlc->scan, wf_chspec_ctlchspec(wlc->home_chanspec),
	WLC_BAND_ALL, chanspec_list, &num_channels);

	/*
	 * Adjust the num channels based on the scanol limits.
	 */
	if (num_channels > NET_DETECT_MAX_CHANNELS)
		num_channels = NET_DETECT_MAX_CHANNELS;

	/*
	 * Since scanol_params_t reserves space for 1 SSID, we need to
	 * allocate  space for all SSIDs in the profile list.
	 */
	ssid_bytes_needed =
	    sizeof(scanol_params_t) +
	    (sizeof(wlc_ssid_t) * (nd_profile_list->num_nd_profiles - 1));

	ssid_bytes_needed = ROUNDUP(ssid_bytes_needed, sizeof(uint32));

	total_bytes_needed = ssid_bytes_needed + (sizeof(chanspec_t) * num_channels);

	scan_params = MALLOC(wlc->osh, total_bytes_needed);
	if (!scan_params) {
		WL_ERROR(("%s: alloc buffer for scan_params failed\n",
			__FUNCTION__));

		err = BCME_NOMEM;

		goto done;
	}

	bzero(scan_params, total_bytes_needed);

	/* Prepare the scan params and issue the IOVAR. -1 means "use default" */
	scan_params->version        = SCANOL_PARAMS_VERSION;
	scan_params->flags          = SCANOL_ENABLED;
	scan_params->active_time    = 0;
	scan_params->passive_time   = -1;

	scan_params->idle_rest_time                     = -1;
	scan_params->idle_rest_time_multiplier          = 0;
	scan_params->active_rest_time                   = -1;
	scan_params->active_rest_time_multiplier        = 0;
	scan_params->scan_cycle_idle_rest_time          = nd_config->scan_interval * 1000;
	scan_params->scan_cycle_idle_rest_multiplier    = 0;
	scan_params->scan_cycle_active_rest_time        = -1;
	scan_params->scan_cycle_active_rest_multiplier  = 0;
	scan_params->max_rest_time                      = nd_config->scan_interval * 1000;
	scan_params->max_scan_cycles                    = -1;
	scan_params->nprobes                            = -1;
	scan_params->scan_start_delay                   = nd_config->wait_period;

	/*
	 * We need to add the SSIDs from the profile list into the scan parameter list. We
	 * will also set the scanol preferred SSIDs to these SSIDs.
	 */
	scan_params->ssid_count = nd_profile_list->num_nd_profiles;

	for (i = 0; i < filtered_profile_list.num_nd_profiles; i++) {
		nd_profile = filtered_profile_list.nd_profile[i];

		bcopy(&nd_profile->ssid, &scan_params->ssidlist[i], sizeof(wlc_ssid_t));
		WL_WOWL(("pref ssid%d: %s \n", i, scan_params->ssidlist[i].SSID));
	}

	/*
	 * Specify the full set of channels we're capable of. This list must come after
	 * the SSID list.
	 */
	scan_params->nchannels = (uint16)num_channels;
	scan_channels	       = (chanspec_t *)((uint8 *)scan_params + ssid_bytes_needed);

	for (i = 0; i < num_channels; i++) {
		scan_channels[i] = chanspec_list[i];
	}

	/* Issue the IOVAR to set the scanol parameters */
	err = wlc_iovar_op(
			wlc,
			"ol_scanparams",
			NULL,
			0,
			scan_params,
			total_bytes_needed,
			IOV_SET,
			NULL);

	if (err != BCME_OK) {
		WL_ERROR(("%s: ol_scanparams IOVAR failed: 0x%x\n",
		    __FUNCTION__, err));

		goto done;
	}

done:
	if (scan_params != NULL)
		MFREE(wlc->osh, scan_params, total_bytes_needed);

	if (chanspec_list != NULL)
		MFREE(wlc->osh, chanspec_list, sizeof(chanspec_t) * MAXCHANNEL);

	if (pfn_list != NULL)
		MFREE(wlc->osh, pfn_list, pfn_list_bytes_needed);

	return err;
}

static int
wlc_net_detect_get_wake_data(
	wlc_net_detect_ctxt_t *net_detect_ctxt,
	net_detect_wake_data_t *nd_wake_data)
{
	net_detect_config_t *nd_config = net_detect_ctxt->nd_config;
	wl_wowl_wakeind_t   wowl_ind;
	int		    err;

	bzero(nd_wake_data, sizeof(net_detect_wake_data_t));

	/* Query wake indication from the WoWL interface */
	err = wlc_iovar_op(
	                net_detect_ctxt->wlc,
	                "wowl_wakeind",
	                NULL,
	                0,
	                &wowl_ind,
	                sizeof(wl_wowl_wakeind_t),
	                FALSE,
	                NULL);

	if (err != 0) {
	    WL_ERROR(("%s: wl_iovar_getbuf returned %d\n",  __FUNCTION__, err));

	    return BCME_ERROR;
	}

	WL_WOWL(("%s: wowl_ind.ucode_wakeind = 0x%08X\n",  __FUNCTION__, wowl_ind.ucode_wakeind));

	if (wowl_ind.ucode_wakeind == 0) {
		nd_wake_data->nd_wake_reason = nd_reason_unknown;
	} else
	if (wowl_ind.ucode_wakeind & WL_WOWL_SCANOL) {
		nd_wake_data->nd_wake_reason = nd_net_detected;
	}
	else {
		nd_wake_data->nd_wake_reason = nd_wowl_event;
	}

	return BCME_OK;
}

#endif /* defined(NET_DETECT) */
