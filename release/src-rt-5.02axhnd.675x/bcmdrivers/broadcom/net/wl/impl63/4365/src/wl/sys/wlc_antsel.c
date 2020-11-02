/*
 * wlc_antsel.c: Antenna Selection code
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
 * $Id: wlc_antsel.c 708017 2017-06-29 14:11:45Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifdef WLANTSEL

#include <typedefs.h>
#include <qmath.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <bitfuncs.h>
#include <bcmdevs.h>
#include <proto/802.11.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wl_dbg.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wlc_phy_hal.h>
#include <wl_export.h>
#include <wlc_antsel.h>
#include <wlc_scb_ratesel.h>

/* iovar table */
enum {
	IOV_ANTSEL_11N,
	IOV_ANTSEL_11N_OVR,	/* antsel override */
};

static const bcm_iovar_t antsel_iovars[] = {
	{"phy_antsel", IOV_ANTSEL_11N, (0), IOVT_BUFFER, sizeof(wlc_antselcfg_t)},
	{"phy_antsel_override", IOV_ANTSEL_11N_OVR, (IOVF_SET_DOWN), IOVT_INT8, 0},
	{NULL, 0, 0, 0, 0}
};

/* useful macros */
#define WLC_ANTSEL_11N_1(ant)     ((((ant) & ANT_SELCFG_MASK) >> 4) & 0xf)
#define WLC_ANTSEL_11N_0(ant)	  (((ant) & ANT_SELCFG_MASK) & 0xf)
#define WLC_ANTIDX_11N(ant)	  (((WLC_ANTSEL_11N_0(ant)) << 2) + (WLC_ANTSEL_11N_1(ant)))
#define WLC_ANT_ISAUTO_11N(ant)   (((ant) & ANT_SELCFG_AUTO) == ANT_SELCFG_AUTO)
#define WLC_ANTSEL_11N(ant)	  ((ant) & ANT_SELCFG_MASK)

/* antenna switch */
/* defines for no boardlevel antenna diversity */
#define ANT_SELCFG_DEF_2x2	0x10	/* default antenna configuration */

/* 2x3 antdiv defines and tables for GPIO communication */
#define ANT_SELCFG_NUM_2x3	3
#define ANT_SELCFG_DEF_2x3	0x10	/* default antenna configuration */

/* 2x4 antdiv rev4 defines and tables for GPIO communication */
#define ANT_SELCFG_NUM_2x4	4
#define ANT_SELCFG_DEF_2x4	0x20	/* default antenna configuration */

/* 1x2 antdiv defines and tables for GPIO communication */
#define ANT_SELCFG_NUM_1x2      2

/* static functions */
static int wlc_antsel_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
static int wlc_antsel_cfgupd(antsel_info_t *asi, wlc_antselcfg_t *antsel);
static uint8 wlc_antsel_id2antcfg(antsel_info_t *asi, uint8 id);
static uint16 wlc_antsel_antcfg2antsel(antsel_info_t *asi, uint8 ant_cfg);
static void wlc_antsel_init_cfg(antsel_info_t *asi, wlc_antselcfg_t *antsel,
	bool auto_sel);
static int wlc_antsel(antsel_info_t *asi, wlc_antselcfg_t *antsel);
static bool wlc_antsel_cfgverify(antsel_info_t *asi, uint8 antcfg);
static int wlc_antsel_cfgcheck(antsel_info_t *asi, wlc_antselcfg_t *antsel,
	uint8 uni, uint8 def);

const uint16 mimo_2x4_div_antselpat_tbl[] = {
	0, 0, 0x9, 0xa, /* ant0: 0 ant1: 2,3 */
	0, 0, 0x5, 0x6, /* ant0: 1 ant1: 2,3 */
	0, 0, 0,   0,   /* n.a.              */
	0, 0, 0,   0    /* n.a.              */
};

const uint8 mimo_2x4_div_antselid_tbl[16] = {
	0, 0, 0, 0, 0, 2, 3, 0,
	0, 0, 1, 0, 0, 0, 0, 0 /* pat to antselid */
};

const uint16 mimo_2x3_div_antselpat_tbl[] = {
	16,  0,  1, 16, /* ant0: 0 ant1: 1,2 */
	16, 16, 16, 16, /* n.a.              */
	16,  2, 16, 16, /* ant0: 2 ant1: 1   */
	16, 16, 16, 16  /* n.a.              */
};

const uint16 mimo_2x3_div_antselpat_tbl_nrev7p[] = {
	16,  0,  2, 16, /* ant0: 0 ant1: 1,2 */
	16, 16, 16, 16, /* n.a.              */
	16,  1, 16, 16, /* ant0: 2 ant1: 1   */
	16, 16, 16, 16  /* n.a.              */
};

const uint8 mimo_2x3_div_antselid_tbl[16] = {
	0, 1, 2, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0 /* pat to antselid */
};

const uint8 mimo_2x3_div_antselid_tbl_nrev7p[16] = {
	0, 2, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0 /* pat to antselid */
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

uint8
wlc_antsel_antseltype_get(antsel_info_t *asi)
{
	return (asi->antsel_type);
}

antsel_info_t *
BCMNMIATTACHFN(wlc_antsel_attach)(wlc_info_t *wlc)
{
	antsel_info_t *asi;
	osl_t *osh = wlc->osh;
	wlc_pub_t *pub = wlc->pub;
	wlc_hw_info_t *wlc_hw = wlc->hw;

	if (!(asi = (antsel_info_t *)MALLOCZ(osh, sizeof(antsel_info_t)))) {
		WL_ERROR(("wl%d: wlc_antsel_attach: out of mem, malloced %d bytes\n",
			pub->unit, MALLOCED(osh)));
		return NULL;
	}

	asi->wlc = wlc;
	asi->pub = pub;
	asi->antsel_type = ANTSEL_NA;
	asi->antsel_avail = FALSE;
	asi->antsel_antswitch = (uint8)getintvar(asi->pub->vars, "antswitch");

	if ((asi->pub->sromrev >= 4) && (asi->antsel_antswitch != 0)) {
		switch (asi->antsel_antswitch) {
		case ANTSWITCH_TYPE_1:
		case ANTSWITCH_TYPE_2:
		case ANTSWITCH_TYPE_3:
		case ANTSWITCH_TYPE_5:
			/* NPHY board with 2x3 switch logic. Either SW only or SWTX/HWRX */
			asi->antsel_type = (asi->antsel_antswitch <= ANTSWITCH_TYPE_3) ?
			        ANTSEL_2x3 : ANTSEL_2x3_HWRX;
			/* Antenna selection availability */
			if (((uint16)getintvar(asi->pub->vars, "aa2g") == 7) ||
			    ((uint16)getintvar(asi->pub->vars, "aa5g") == 7)) {
				asi->antsel_avail = TRUE;
			} else if (((uint16)getintvar(asi->pub->vars, "aa2g") == 3) ||
				((uint16)getintvar(asi->pub->vars, "aa5g") == 3)) {
				asi->antsel_avail = FALSE;
			} else {
				asi->antsel_avail = FALSE;
				WL_ERROR(("wlc_antsel_attach: 2o3 board cfg invalid\n"));
				ASSERT(0);
			}
			break;
		case ANTSWITCH_TYPE_4:
		case ANTSWITCH_TYPE_6:
		case ANTSWITCH_TYPE_7:
			/* 43234 board with 1x2 switch logic. Either SW only or SWTX/HWRX */
			/* 43234 board uses core1 1x2 and  5356c0 board uses core 0 1x2   */
			if (asi->antsel_antswitch == ANTSWITCH_TYPE_7)
				asi->antsel_type = ANTSEL_1x2_CORE0;
			else
				asi->antsel_type = (asi->antsel_antswitch <= ANTSWITCH_TYPE_4) ?
				ANTSEL_1x2_CORE1 : ANTSEL_1x2_HWRX;
			/* Antenna selection availability */
			if (((uint16)getintvar(asi->pub->vars, "aa2g") == 6) ||
			    ((uint16)getintvar(asi->pub->vars, "aa5g") == 6)) {
				asi->antsel_avail = TRUE;
			} else {
				asi->antsel_avail = FALSE;
			}
			break;
		default:
			break;
		}
	} else if ((asi->pub->sromrev == 4) &&
		((uint16)getintvar(asi->pub->vars, "aa2g") == 7) &&
		((uint16)getintvar(asi->pub->vars, "aa5g") == 0)) {
		/* hack to match old 4321CB2 cards with 2of3 antenna switch */
		asi->antsel_type = ANTSEL_2x3;
		asi->antsel_avail = TRUE;
	} else if (asi->pub->boardflags2 & BFL2_2X4_DIV) {
		asi->antsel_type = ANTSEL_2x4;
		asi->antsel_avail = TRUE;
	}

	/* Set the antenna selection type for the low driver */
	wlc_bmac_antsel_type_set(wlc_hw, asi->antsel_type);

	/* Init (auto/manual) antenna selection */
	wlc_antsel_init_cfg(asi, &asi->antcfg_11n, TRUE);
	wlc_antsel_init_cfg(asi, &asi->antcfg_cur, TRUE);

	/* register module */
	if (wlc_module_register(asi->pub, antsel_iovars, "antsel", asi, wlc_antsel_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: antsel wlc_module_register() failed\n",
			pub->unit));
		goto fail;
	}

	return asi;

fail:
	MFREE(osh, asi, sizeof(antsel_info_t));
	return NULL;
}

void
BCMATTACHFN(wlc_antsel_detach)(antsel_info_t *asi)
{
	if (!asi)
		return;

	wlc_module_unregister(asi->pub, "antsel", asi);
	MFREE(asi->pub->osh, asi, sizeof(antsel_info_t));
}

void
wlc_antsel_init(antsel_info_t *asi)
{
	if ((asi->antsel_type == ANTSEL_2x3) ||
	    (asi->antsel_type == ANTSEL_2x4) ||
	    (asi->antsel_type == ANTSEL_1x2_CORE1) ||
	    (asi->antsel_type == ANTSEL_1x2_CORE0) ||
	    (asi->antsel_type == ANTSEL_2x3_HWRX) ||
	    (asi->antsel_type == ANTSEL_1x2_HWRX))
		wlc_antsel_cfgupd(asi, &asi->antcfg_11n);
}

/* handle antsel related iovars */
static int
wlc_antsel_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	antsel_info_t *asi = (antsel_info_t *)hdl;
	int32 int_val = 0;
	int err = 0;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	switch (actionid) {
	case IOV_SVAL(IOV_ANTSEL_11N):
		/* antenna diversity control */
		err = wlc_antsel(asi, (wlc_antselcfg_t *)a);
		break;

	case IOV_GVAL(IOV_ANTSEL_11N): {
		uint8 i;

		/* report currently used antenna configuration */
		for (i = ANT_SELCFG_TX_UNICAST; i < ANT_SELCFG_MAX; i++) {
			if (WLC_ANT_ISAUTO_11N(asi->antcfg_11n.ant_config[i]))
				asi->antcfg_11n.ant_config[i] = ANT_SELCFG_AUTO |
					asi->antcfg_cur.ant_config[i];
		}

		bcopy((char*)&asi->antcfg_11n, (char*)a, sizeof(wlc_antselcfg_t));

		break;
	}

	case IOV_SVAL(IOV_ANTSEL_11N_OVR):	/* antenna selection override */
		if (int_val == -1) {
			/* no override, use auto selection if available, else use default */
			wlc_antselcfg_t at;
			wlc_antsel_init_cfg(asi, &at, TRUE);
			err = wlc_antsel(asi, &at);
			break;
		} else if (int_val == 0) {
			/* override to use fixed antennas, no auto selection */
			wlc_antselcfg_t at;
			wlc_antsel_init_cfg(asi, &at, FALSE);
			err = wlc_antsel(asi, &at);
		}
		break;

	default:
		err = BCME_UNSUPPORTED;
	}
	return err;
}

/* boardlevel antenna selection: helper function to validate ant_cfg specification */
static bool
wlc_antsel_cfgverify(antsel_info_t *asi, uint8 antcfg)
{
	uint8 idx = WLC_ANTIDX_11N(antcfg);

	/* 1x2 is a subset of 2x3 and uses the same table */
	if (asi->antsel_type == ANTSEL_2x3 || asi->antsel_type == ANTSEL_1x2_CORE1 ||
	    asi->antsel_type == ANTSEL_1x2_CORE0 ||
	    asi->antsel_type == ANTSEL_2x3_HWRX || asi->antsel_type == ANTSEL_1x2_HWRX) {
		if ((NREV_GE(asi->wlc->band->phyrev, 3) && (NREV_LE(asi->wlc->band->phyrev, 6)))) {
			if ((idx >= ARRAYSIZE(mimo_2x3_div_antselpat_tbl)) ||
			    (mimo_2x3_div_antselpat_tbl[idx] > 15)) {
				return FALSE;
			}
		} else {
			if ((idx >= ARRAYSIZE(mimo_2x3_div_antselpat_tbl_nrev7p)) ||
			    (mimo_2x3_div_antselpat_tbl_nrev7p[idx] > 15)) {
				return FALSE;
			}
		}

	} else if (asi->antsel_type == ANTSEL_2x4) {
		if ((idx >= ARRAYSIZE(mimo_2x4_div_antselpat_tbl)) ||
		    ((mimo_2x4_div_antselpat_tbl[idx] & 0xf) == 0)) {
			return FALSE;
		}

	} else {
		ASSERT(0);
		return FALSE;
	}

	return TRUE;
}

/* boardlevel antenna selection: validate ant_cfg specification */
static int
wlc_antsel_cfgcheck(antsel_info_t *asi, wlc_antselcfg_t *antsel, uint8 uni_ant, uint8 def_ant)
{
	uint8 def_ant_auto = 0, uni_ant_auto = 0;
	bool uni_auto, def_auto;

	uni_auto = (antsel->ant_config[uni_ant] == (uint8)AUTO);
	def_auto = (antsel->ant_config[def_ant] == (uint8)AUTO);

	/* check if auto selection enable and dependencies */
	if (uni_auto && def_auto) {
		def_ant_auto = ANT_SELCFG_AUTO;
		uni_ant_auto = ANT_SELCFG_AUTO;
	} else if (uni_auto && !def_auto) {
		if (!wlc_antsel_cfgverify(asi, antsel->ant_config[def_ant]))
			return BCME_RANGE;
		uni_ant_auto = ANT_SELCFG_AUTO;
	} else if (!uni_auto && !def_auto) {
		if (!wlc_antsel_cfgverify(asi, antsel->ant_config[uni_ant]))
			return BCME_RANGE;
		if (!wlc_antsel_cfgverify(asi, antsel->ant_config[def_ant]))
			return BCME_RANGE;
	} else
		return BCME_RANGE;

	if (uni_ant_auto)
		antsel->ant_config[uni_ant] = (asi->antcfg_11n.ant_config[uni_ant] &
			ANT_SELCFG_MASK) | uni_ant_auto;

	if (def_ant_auto)
		antsel->ant_config[def_ant] = (asi->antcfg_11n.ant_config[def_ant] &
			ANT_SELCFG_MASK) | def_ant_auto;

	return BCME_OK;
}

/* boardlevel antenna selection: init antenna selection structure */
static void
wlc_antsel_init_cfg(antsel_info_t *asi, wlc_antselcfg_t *antsel, bool auto_sel)
{
	if (asi->antsel_type == ANTSEL_2x3 || asi->antsel_type == ANTSEL_1x2_CORE1 ||
	    asi->antsel_type == ANTSEL_1x2_CORE0 ||
	    asi->antsel_type == ANTSEL_2x3_HWRX || asi->antsel_type == ANTSEL_1x2_HWRX) {
		/* 1x2 reuses default for 2x3 */
		uint8 antcfg_def = ANT_SELCFG_DEF_2x3 |
			((asi->antsel_avail && auto_sel) ? ANT_SELCFG_AUTO : 0);
		antsel->ant_config[ANT_SELCFG_TX_DEF] = antcfg_def;
		antsel->ant_config[ANT_SELCFG_TX_UNICAST] = antcfg_def;
		antsel->ant_config[ANT_SELCFG_RX_DEF] = antcfg_def;
		antsel->ant_config[ANT_SELCFG_RX_UNICAST] = antcfg_def;

		if (antsel->num_antcfg == ANT_SELCFG_NUM_2x3)
			antsel->num_antcfg = ANT_SELCFG_NUM_2x3;
		else
			antsel->num_antcfg = ANT_SELCFG_NUM_1x2;

	} else if (asi->antsel_type == ANTSEL_2x4) {

		antsel->ant_config[ANT_SELCFG_TX_DEF] = ANT_SELCFG_DEF_2x4;
		antsel->ant_config[ANT_SELCFG_TX_UNICAST] = ANT_SELCFG_DEF_2x4;
		antsel->ant_config[ANT_SELCFG_RX_DEF] = ANT_SELCFG_DEF_2x4;
		antsel->ant_config[ANT_SELCFG_RX_UNICAST] = ANT_SELCFG_DEF_2x4;
		antsel->num_antcfg = ANT_SELCFG_NUM_2x4;

	} else {	/* no antenna selection available */

		antsel->ant_config[ANT_SELCFG_TX_DEF] = ANT_SELCFG_DEF_2x2;
		antsel->ant_config[ANT_SELCFG_TX_UNICAST] = ANT_SELCFG_DEF_2x2;
		antsel->ant_config[ANT_SELCFG_RX_DEF] = ANT_SELCFG_DEF_2x2;
		antsel->ant_config[ANT_SELCFG_RX_UNICAST] = ANT_SELCFG_DEF_2x2;
		antsel->num_antcfg = 0;
	}
}

/* boardlevel antenna selection: configure manual/auto antenna selection */
static int
wlc_antsel(antsel_info_t *asi, wlc_antselcfg_t *antsel)
{
	int i;
	bool is_antcfg_fixed = 0;

	if ((asi->antsel_type == ANTSEL_NA) || (asi->antsel_avail == FALSE))
		return BCME_UNSUPPORTED;

	if (wlc_antsel_cfgcheck(asi, antsel, ANT_SELCFG_TX_UNICAST, ANT_SELCFG_TX_DEF) < 0) {
		WL_ERROR(("wl%d:wlc_antsel: Illegal TX antenna configuration 0x%02x\n",
			asi->pub->unit, antsel->ant_config[ANT_SELCFG_TX_DEF]));
		return BCME_RANGE;
	}

	if (wlc_antsel_cfgcheck(asi, antsel, ANT_SELCFG_RX_UNICAST, ANT_SELCFG_RX_DEF) < 0) {
		WL_ERROR(("wl%d:wlc_antsel: Illegal RX antenna configuration 0x%02x\n",
			asi->pub->unit, antsel->ant_config[ANT_SELCFG_RX_DEF]));
		return BCME_RANGE;
	}

	/* check if down when user enables/disables antenna selection algorithms */
	if (asi->pub->up) {
		for (i = ANT_SELCFG_TX_UNICAST; i < ANT_SELCFG_MAX; i++) {
			if (((asi->antcfg_11n.ant_config[i] & ANT_SELCFG_AUTO) == 0) &&
			    ((antsel->ant_config[i] & ANT_SELCFG_AUTO) == ANT_SELCFG_AUTO)) {
				return BCME_NOTDOWN;
			}
			if (((asi->antcfg_11n.ant_config[i] & ANT_SELCFG_AUTO)) &&
			    ((antsel->ant_config[i] & ANT_SELCFG_AUTO) == ANT_SELCFG_AUTO) == 0) {
				return BCME_NOTDOWN;
			}
		}
	}

	/* finally, update the antcfg */
	for (i = ANT_SELCFG_TX_UNICAST; i < ANT_SELCFG_MAX; i++)
		asi->antcfg_11n.ant_config[i] = antsel->ant_config[i];

	/* determine if antcfg is FIXED or AUTO */
	for (i = ANT_SELCFG_TX_UNICAST; i < ANT_SELCFG_MAX; i++) {
		is_antcfg_fixed |= ((asi->antcfg_11n.ant_config[i] & ANT_SELCFG_AUTO) == 0);
	}

	/* If HWRX antsel is available, disable it and enable ucode Rx antsel when antcfg is FIXED.
	 * If antcfg is AUTO, enable HWRX and disableucode Rx antsel.
	 */
	if ((asi->antsel_type == ANTSEL_2x3_HWRX) || (asi->antsel_type == ANTSEL_1x2_HWRX)) {
		wlc_bmac_mhf(asi->wlc->hw, MHF3, MHF3_ANTSEL_MODE, is_antcfg_fixed ?
		             MHF3_ANTSEL_MODE : 0, WLC_BAND_ALL);
		wlc_iovar_setint(asi->wlc, "phy_rxantsel", !(is_antcfg_fixed));
	}

	if (asi->pub->up)
		wlc_antsel_cfgupd(asi, &asi->antcfg_11n);

	wlc_scb_ratesel_init_all(asi->wlc);

	return BCME_OK;
}

uint16
wlc_antsel_buildtxh(antsel_info_t *asi, uint8 antcfg, uint8 fbantcfg)
{
	return ((wlc_antsel_antcfg2antsel(asi, antcfg)) |
		(wlc_antsel_antcfg2antsel(asi, fbantcfg) << ABI_MAS_FBR_ANT_PTN_SHIFT));
}

void
wlc_antsel_ratesel(antsel_info_t *asi, uint8 *active_antcfg_num, uint8 *antselid_init)
{
	uint8 ant;

	ant = asi->antcfg_11n.ant_config[ANT_SELCFG_TX_UNICAST];

	if ((ant & ANT_SELCFG_AUTO) == ANT_SELCFG_AUTO) {
		*active_antcfg_num = asi->antcfg_11n.num_antcfg;
		*antselid_init = 0;
	} else {
		*active_antcfg_num = 0;
		*antselid_init = 0;
	}
}

void
wlc_antsel_set_unicast(antsel_info_t *asi, uint8 antcfg)
{
	asi->antcfg_cur.ant_config[ANT_SELCFG_TX_UNICAST] = antcfg;
}

void BCMFASTPATH
wlc_antsel_antcfg_get(antsel_info_t *asi, bool usedef, bool sel, uint8 antselid,
	uint8 fbantselid, uint8 *antcfg, uint8 *fbantcfg)
{
	uint8 ant;

	/* if use default, assign it and return */
	if (usedef) {
		*antcfg = asi->antcfg_11n.ant_config[ANT_SELCFG_TX_DEF];
		*fbantcfg = *antcfg;
		return;
	}

	if (!sel) {
		*antcfg = asi->antcfg_11n.ant_config[ANT_SELCFG_TX_UNICAST];
		*fbantcfg = *antcfg;

	} else {
		ant = asi->antcfg_11n.ant_config[ANT_SELCFG_TX_UNICAST];
		if ((ant & ANT_SELCFG_AUTO) == ANT_SELCFG_AUTO) {
			*antcfg = wlc_antsel_id2antcfg(asi, antselid);
			*fbantcfg = wlc_antsel_id2antcfg(asi, fbantselid);
		} else {
			*antcfg = asi->antcfg_11n.ant_config[ANT_SELCFG_TX_UNICAST];
			*fbantcfg = *antcfg;
		}
	}
	return;
}

/* boardlevel antenna selection: update "default" (multi-/broadcast) antenna cfg */
void BCMFASTPATH
wlc_antsel_upd_dflt(antsel_info_t *asi, uint8 antselid)
{
	wlc_antselcfg_t antsel;
	uint8 i;

	ASSERT(asi->antsel_type != ANTSEL_NA);

	/* check if default selection is enabled. if yes, update default */
	for (i = ANT_SELCFG_RX_UNICAST; i < ANT_SELCFG_MAX; i++) {
		if (WLC_ANT_ISAUTO_11N(asi->antcfg_11n.ant_config[i]))
			antsel.ant_config[i] = wlc_antsel_id2antcfg(asi, antselid);
		else
			antsel.ant_config[i] = asi->antcfg_11n.ant_config[i];
	}
	wlc_antsel_cfgupd(asi, &antsel);
}

/* boardlevel antenna selection: convert mimo_antsel (ucode interface) to id */
uint8
wlc_antsel_antsel2id(antsel_info_t *asi, uint16 antsel)
{
	uint8 antselid = 0;

	if (asi->antsel_type == ANTSEL_2x4) {
		/* 2x4 antenna diversity board, 4 cfgs: 0-2 0-3 1-2 1-3 */
		antselid = mimo_2x4_div_antselid_tbl[(antsel & 0xf)];
		return antselid;

	} else if (asi->antsel_type == ANTSEL_2x3 || asi->antsel_type == ANTSEL_1x2_CORE1 ||
	           asi->antsel_type == ANTSEL_2x3_HWRX || asi->antsel_type == ANTSEL_1x2_HWRX) {
		/* 2x3 antenna selection, 3 cfgs: 1-0 2-0 1-2 */
		/* 1x2 antenna selection, 2 cfgs: 1-0 2-0 reuse the same table  */
		if ((NREV_GE(asi->wlc->band->phyrev, 3) && (NREV_LE(asi->wlc->band->phyrev, 6)))) {
			antselid = mimo_2x3_div_antselid_tbl[(antsel & 0xf)];
		} else {
			/* PR 59451: in REV7+, bits 9:8 encode antennas that core1:0 connect to.
			 *           0 = main antenna, 1 = aux antenna (so 11 is not allowed).
			 */
			antselid = mimo_2x3_div_antselid_tbl_nrev7p[(antsel & 0xf)];
		}

		return antselid;
	} else if (asi->antsel_type == ANTSEL_1x2_CORE0) {
		/* Reverse the phyrev logic because Core1 1x2 is the reverse of core0 1x2  */
		/* Reusing the mimo_2x3_div_antselid_tbl here for core 0 1x2  */
		/* For rev. 7+  phy, use the pre-rev7 table instead to save memory */

		if ((NREV_GE(asi->wlc->band->phyrev, 3) && (NREV_LE(asi->wlc->band->phyrev, 6))))
			antselid = mimo_2x3_div_antselid_tbl_nrev7p[(antsel & 0xf)];
		else
			antselid = mimo_2x3_div_antselid_tbl[(antsel & 0xf)];

		return antselid;
	}

	return antselid;
}

/* boardlevel antenna selection: convert id to ant_cfg */
static uint8
wlc_antsel_id2antcfg(antsel_info_t *asi, uint8 id)
{
	uint8 antcfg = ANT_SELCFG_DEF_2x2;

	if (asi->antsel_type == ANTSEL_2x4) {
		/* 2x4 antenna diversity board, 4 cfgs: 0-2 0-3 1-2 1-3 */
		antcfg = (((id & 0x2) << 3) | ((id & 0x1) + 2));
	} else if (asi->antsel_type == ANTSEL_2x3 || asi->antsel_type == ANTSEL_1x2_CORE1 ||
	           asi->antsel_type == ANTSEL_2x3_HWRX || asi->antsel_type == ANTSEL_1x2_HWRX) {
		/* 2x3 antenna selection, 3 cfgs: 1-0 2-0 1-2 */
		/* 1x2 using core1, 2 cfgs: 1-0 2-0 reuse the same table  */
		antcfg = ((((id & 0x01) + 1) << 4) | (id & 0x2));
	} else if (asi->antsel_type == ANTSEL_1x2_CORE0) {
		/* 1x2 using core0, 2 cfgs: 1-0 1-2 */
		antcfg = (id == 0) ? 0x10 : 0x12;
	}
	return antcfg;
}

/* boardlevel antenna selection: convert ant_cfg to mimo_antsel (ucode interface) */
static uint16
wlc_antsel_antcfg2antsel(antsel_info_t *asi, uint8 ant_cfg)
{
	uint8 idx = WLC_ANTIDX_11N(WLC_ANTSEL_11N(ant_cfg));
	uint16 mimo_antsel = 0;

	if (asi->antsel_type == ANTSEL_2x4) {
		/* 2x4 antenna diversity board, 4 cfgs: 0-2 0-3 1-2 1-3 */
		mimo_antsel = (mimo_2x4_div_antselpat_tbl[idx] & 0xf);
		return mimo_antsel;

	} else if (asi->antsel_type == ANTSEL_2x3 || asi->antsel_type == ANTSEL_1x2_CORE1 ||
	           asi->antsel_type == ANTSEL_1x2_CORE0 ||
	           asi->antsel_type == ANTSEL_2x3_HWRX || asi->antsel_type == ANTSEL_1x2_HWRX) {
		/* 2x3 antenna selection, 3 cfgs: 1-0 2-0 1-2 */
		/* 1x2 antenna selection, 2 cfgs: 1-0 2-0 reuse the same table  */
		if ((NREV_GE(asi->wlc->band->phyrev, 3) && (NREV_LE(asi->wlc->band->phyrev, 6)))) {
			mimo_antsel = (mimo_2x3_div_antselpat_tbl[idx] & 0xf);
		} else {
			/* PR 59451: in REV7+, bits 9:8 encode antennas that core1:0 connect to.
			 *           0 = main antenna, 1 = aux antenna (so 11 is not allowed).
			 */
			mimo_antsel = (mimo_2x3_div_antselpat_tbl_nrev7p[idx] & 0xf);
		}

		return mimo_antsel;
	}

	return mimo_antsel;
}

/* boardlevel antenna selection: ucode interface control */
static int BCMFASTPATH
wlc_antsel_cfgupd(antsel_info_t *asi, wlc_antselcfg_t *antsel)
{
	wlc_info_t *wlc = asi->wlc;
	uint8 ant_cfg;
	uint16 mimo_antsel;

	ASSERT(asi->antsel_type != ANTSEL_NA);

	/* 1) Update TX antconfig for all frames that are not unicast data
	 *    (aka default TX)
	 */
	ant_cfg = antsel->ant_config[ANT_SELCFG_TX_DEF];
	mimo_antsel = wlc_antsel_antcfg2antsel(asi, ant_cfg);
	wlc_write_shm(wlc, M_MIMO_ANTSEL_TXDFLT, mimo_antsel);
	/* Update driver stats for currently selected default tx/rx antenna config */
	asi->antcfg_cur.ant_config[ANT_SELCFG_TX_DEF] = ant_cfg;

	/* 2) Update RX antconfig for all frames that are not unicast data
	 *    (aka default RX)
	 */
	ant_cfg = antsel->ant_config[ANT_SELCFG_RX_DEF];
	mimo_antsel = wlc_antsel_antcfg2antsel(asi, ant_cfg);
	wlc_write_shm(wlc, M_MIMO_ANTSEL_RXDFLT, mimo_antsel);
	/* Update driver stats for currently selected default tx/rx antenna config */
	asi->antcfg_cur.ant_config[ANT_SELCFG_RX_DEF] = ant_cfg;

	return 0;
}

#endif /* WLANTSEL */
