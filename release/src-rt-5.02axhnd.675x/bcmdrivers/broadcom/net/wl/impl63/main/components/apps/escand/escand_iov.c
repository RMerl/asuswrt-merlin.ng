/*
 *      escand_iov.c
 *
 *      This module will try to set/get the data from/to driver using IOVAR.
 *
 *	Copyright 2020 Broadcom
 *
 *	This program is the proprietary software of Broadcom and/or
 *	its licensors, and may only be used, duplicated, modified or distributed
 *	pursuant to the terms and conditions of a separate, written license
 *	agreement executed between you and Broadcom (an "Authorized License").
 *	Except as set forth in an Authorized License, Broadcom grants no license
 *	(express or implied), right to use, or waiver of any kind with respect to
 *	the Software, and Broadcom expressly reserves all rights in and to the
 *	Software and all intellectual property rights therein.  IF YOU HAVE NO
 *	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 *	WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 *	THE SOFTWARE.
 *
 *	Except as expressly set forth in the Authorized License,
 *
 *	1. This program, including its structure, sequence and organization,
 *	constitutes the valuable trade secrets of Broadcom, and you shall use
 *	all reasonable efforts to protect the confidentiality thereof, and to
 *	use this information only in connection with your use of Broadcom
 *	integrated circuit products.
 *
 *	2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *	"AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 *	REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 *	OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 *	DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 *	NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 *	ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 *	CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 *	OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *	3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 *	BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 *	SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 *	IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *	IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 *	ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 *	OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 *	NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *	$Id: escand_iov.c 767267 2018-09-03 06:50:15Z $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <assert.h>
#include <typedefs.h>

#include "escand_svr.h"

int escand_get_perband_chanspecs(escand_chaninfo_t *c_info, chanspec_t input, char *buf, int length)
{
	return wl_iovar_getbuf(c_info->name, "chanspecs", &input, sizeof(chanspec_t), buf, length);
}

int escand_get_per_chan_info(escand_chaninfo_t *c_info, chanspec_t sub_chspec, char *buf, int length)
{
	return wl_iovar_getbuf(c_info->name, "per_chan_info", &sub_chspec, sizeof(chanspec_t), buf,
		length);
}

int escand_set_scanresults_minrssi(escand_chaninfo_t *c_info, int minrssi)
{
	return wl_iovar_setint(c_info->name, "scanresults_minrssi", minrssi);
}

int escand_set_escan_params(escand_chaninfo_t *c_info, wl_escan_params_t *params, int params_size)
{
	return wl_iovar_set(c_info->name, "escan", params, params_size);
}

int escand_get_chanspec(escand_chaninfo_t *c_info, int *chanspec)
{
	return wl_iovar_getint(c_info->name, "chanspec", chanspec);
}

int escand_get_isup(escand_chaninfo_t *c_info, int *isup)
{
	return wl_ioctl(c_info->name, WLC_GET_UP, isup, sizeof(int));
}

int escand_set_chanspec(escand_chaninfo_t *c_info, chanspec_t chspec)
{
	return wl_iovar_setint(c_info->name, "chanspec", htod32(chspec));
}

int escand_set_far_sta_rssi(escand_chaninfo_t *c_info, int rssi)
{
	return wl_iovar_setint(c_info->name, "far_sta_rssi", rssi);
}

int escand_get_obss_coex_info(escand_chaninfo_t *c_info, int *coex)
{
	return wl_iovar_getint(c_info->name, "obss_coex", coex);
}

int escand_get_bwcap_info(escand_chaninfo_t *c_info, escand_param_info_t *param, int param_len, char *buf,
	int buf_len)
{
	return wl_iovar_getbuf(c_info->name, "bw_cap", param, param_len, buf, buf_len);
}

int escand_get_cap_info(escand_chaninfo_t *c_info, uint32 *param, int param_len, char *cap_buf,
	int cap_len)
{
	return wl_iovar_getbuf(c_info->name, "cap", param, param_len, cap_buf, cap_len);
}

int escand_get_counters(char *ifname, char cntbuf[ESCAND_WL_CNTBUF_SIZE])
{
	return wl_iovar_get(ifname, "counters", cntbuf, ESCAND_WL_CNTBUF_SIZE);
}

int escand_get_phydyn_switch_status(char *name, int *phy_dyn_switch)
{
	return wl_iovar_getint(name, "phy_dyn_switch", phy_dyn_switch);
}

int escand_get_stainfo(char *name, struct ether_addr *ea, int ether_len,
	char *stabuf, int buf_len)
{
	return wl_iovar_getbuf(name, "sta_info", &ea, sizeof(ea), stabuf, buf_len);
}

int escand_set_noise_metric(char *name, uint8 knoise)
{
	return wl_iovar_setint(name, "noise_metric", knoise);
}

int escand_get_scb_probe(char *ifname, wl_scb_probe_t *scb_probe, int size)
{
	return wl_iovar_get(ifname, "scb_probe", scb_probe, size);
}

int escand_set_scb_probe(char *ifname, wl_scb_probe_t *scb_probe, int size_probe)
{
	return wl_iovar_set(ifname, "scb_probe", scb_probe, size_probe);
}

/* get country details for an interface */
int escand_get_country(escand_chaninfo_t * c_info)
{
	int ret = BCME_OK;

	ret = wl_iovar_get(c_info->name, "country", &c_info->country,
		sizeof(c_info->country));

	/* ensure null termination before logging/using */
	c_info->country.country_abbrev[WLC_CNTRY_BUF_SZ - 1] = '\0';
	c_info->country.ccode[WLC_CNTRY_BUF_SZ - 1] = '\0';

	if (ret != BCME_OK) {
		ESCAND_ERROR("get country on %s returned %d.\n", c_info->name, ret);
	} else {
		int is_edcrs_eu;
		ret = wl_iovar_getint(c_info->name, "is_edcrs_eu", &is_edcrs_eu);
		c_info->country_is_edcrs_eu = dtoh32(is_edcrs_eu);
		ESCAND_INFO("get country on %s returned %d. ca=%s, cr=%d, cc=%s is_edcrs_eu %d\n",
			c_info->name, ret,
			c_info->country.country_abbrev,
			c_info->country.rev, c_info->country.ccode,
			c_info->country_is_edcrs_eu);
	}

	return ret;
}

/* check if there is still associated scbs. reture value: TRUE if yes. */
bool escand_check_assoc_scb(escand_chaninfo_t * c_info)
{
	bool connected = TRUE;
	int result = 0;
	int ret = 0;

	ret = wl_iovar_getint(c_info->name, "scb_assoced", &result);
	if (ret) {
		ESCAND_ERROR("failed to get scb_assoced\n");
		return connected;
	}

	connected = dtoh32(result) ? TRUE : FALSE;
	ESCAND_DEBUG("connected: %d\n",  connected);

	return connected;
}

int dcs_handle_request(char* ifname, wl_bcmdcs_data_t *dcs_data,
	uint8 mode, uint8 count, uint8 csa_mode)
{
	wl_chan_switch_t csa;
	int err = ESCAND_OK;

	ESCAND_INFO("ifname: %s, reason: %d, chanspec: 0x%x, csa:%x\n",
		ifname, dcs_data->reason, dcs_data->chspec, csa_mode);

	csa.mode = mode;
	csa.count = count;
	csa.chspec = dcs_data->chspec;
	csa.reg = 0;
	csa.frame_type = csa_mode;

	err = wl_iovar_set(ifname, "csa", &csa, sizeof(wl_chan_switch_t));

	return err;
}
