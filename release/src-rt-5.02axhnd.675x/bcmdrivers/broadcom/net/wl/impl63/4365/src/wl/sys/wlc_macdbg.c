/*
 * MAC debug and print functions
 * Broadcom 802.11abg Networking Device Driver
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

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <sbchipc.h>
#include <bcmendian.h>
#include <wlc_types.h>
#include <wlioctl.h>
#include <proto/802.11.h>
#include <d11.h>
#include <hnddma.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wlc_ampdu.h>
#include <wlc_macdbg.h>
#include <wlc_bsscfg.h>
#include <wlc_phy_hal.h>
#include "d11reglist.h"
#ifdef VASIP_HW_SUPPORT
#include <phy_misc_api.h>
#include <wlc_hw_priv.h>
#define SVMP_ACCESS_VIA_PHYTBL
#endif // endif

#define SC_NUM_OPTNS_GE50	4
#define SC_OPTN_LT50NA		0

/* Module private states */
struct wlc_macdbg_info {
	wlc_info_t *wlc;
	uint16 smpl_ctrl;	/* Sample Capture / Play Contrl */
	void *smpl_info;	/* Sample Capture Setup Params */
	CONST d11regs_list_t *pd11regs; /* dump register list */
	CONST d11regs_list_t *pd11regs_x; /* dump register list for second core if exists */
	uint d11regs_sz;
	uint d11regsx_sz;
	uint log_done;		/* reason bitmap */
};

/* this is for dump_ucode_fatal */
typedef struct _d11print_list {
	char name[16]; /* maximum 16 chars */
	uint16 addr;
} d11print_list_t;

/* this is for dump_shmem */
typedef struct _shmem_list {
	uint16	start;
	uint16	cnt;
} shmem_list_t;

#define	PRREG(name)	bcm_bprintf(b, #name " 0x%x ", R_REG(wlc->osh, &regs->name))
#define PRREG_INDEX(name, reg) bcm_bprintf(b, #name " 0x%x ", R_REG(wlc->osh, &reg))

typedef enum {
	 SMPL_CAPTURE_GE50 = 0,
	 SMPL_CAPTURE_LT50 = 1
} smpl_capture_corerev_t;

/* Compile flag validity check */
#if defined(WLC_HOSTPMAC)
#ifndef DONGLEBUILD
#error "WLC_HOSTPMAC are only for DONGLEBUILD!"
#endif // endif
#endif /* WLC_HOSTPMAC */

static int wlc_macdbg_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int vsize, struct wlc_if *wlcif);
static int wlc_macdbg_up(void *hdl);
#if WL_MACDBG || defined(WLC_HOSTPMAC)
static int wlc_macdbg_init_dumplist(wlc_macdbg_info_t *macdbg);
#endif /* WL_MACDBG || WLC_HOSTPMAC */
#if WL_MACDBG
static int wlc_dump_mac(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_dump_shmem(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_dump_sctpl(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_dump_bcntpls(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_dump_pio(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_macdbg_pmac(wlc_info_t *wlc, wl_macdbg_pmac_param_t *pmac,
	char *out_buf, int out_len);
/* MAC Sample Capture */
static void wlc_macdbg_smpl_capture_optnreg(wlc_info_t *wlc,
	uint8 *reg_addr, uint32 *val, int reg_size, bool set);
static int wlc_macdbg_smpl_capture_optns(wlc_info_t *wlc, wl_maccapture_params_t *params, bool set);
static int wlc_macdbg_smpl_capture_set(wlc_info_t *wlc, wl_maccapture_params_t *params);
static int wlc_macdbg_smpl_capture_get(wlc_info_t *wlc, char *outbuf, uint outlen);

#ifdef WL_PSMX
static int wlc_dump_shmemx(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_dump_macx(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_dump_sctpl_shmx(wlc_info_t *wlc, struct bcmstrbuf *b);
#endif // endif
#endif /* WL_MACDBG */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_dump_dma(wlc_info_t *wlc, struct bcmstrbuf *b);
#endif // endif
static void wlc_suspend_mac_debug(wlc_info_t *wlc, uint32 phydebug);

/** iovar table */
enum {
	IOV_MACDBG_PMAC,		/* print mac */
	IOV_MACDBG_CAPTURE,		/* MAC Sample Capture */
	IOV_MACDBG_SHMX,		/* set/get shmemx */
	IOV_MACDBG_REGX,		/* set/get psmx regs */
#ifdef DBG_RXDMA_STUCK_WAR
	IOV_MACDBG_TRIG_RXSTUCK, /* trigger rx stuck*/
#endif // endif
	IOV_MACDBG_RXSTUCKTMO, /* rx stuck timeout to trigger reinit*/
	IOV_MACDBG_LAST
};

static const bcm_iovar_t macdbg_iovars[] = {
	{"pmac", IOV_MACDBG_PMAC, (0), IOVT_BUFFER, 0},
	{"mac_capture", IOV_MACDBG_CAPTURE, (0), IOVT_BUFFER, 0},
	{"shmemx", IOV_MACDBG_SHMX, (IOVF_SET_CLK | IOVF_GET_CLK), IOVT_BUFFER, 0},
#ifdef DBG_RXDMA_STUCK_WAR
	{"trig_rxstuck", IOV_MACDBG_TRIG_RXSTUCK, 0, IOVT_BOOL, 0 },
#endif // endif
	{"rxstuck_tmo", IOV_MACDBG_RXSTUCKTMO, 0, IOVT_UINT32, 0 },
	{NULL, 0, 0, 0, 0}
};

void
BCMATTACHFN(wlc_macdbg_detach)(wlc_macdbg_info_t *macdbg)
{
	wlc_info_t *wlc;

	if (!macdbg)
		return;

	wlc = macdbg->wlc;

	wlc_module_unregister(wlc->pub, "macdbg", macdbg);
#if WL_MACDBG
	if (macdbg->smpl_info)
		MFREE(wlc->osh, macdbg->smpl_info, sizeof(wl_maccapture_params_t));
#endif // endif
#if WL_MACDBG || defined(WLC_HOSTPMAC)
	macdbg->pd11regs = NULL;
	macdbg->pd11regs_x = NULL;
#endif /* WL_MACDBG || WLC_HOSTPMAC */
	MFREE(wlc->osh, macdbg, sizeof(*macdbg));
}

wlc_macdbg_info_t *
BCMATTACHFN(wlc_macdbg_attach)(wlc_info_t *wlc)
{
	wlc_pub_t *pub = wlc->pub;
	wlc_macdbg_info_t *macdbg;

	if ((macdbg = MALLOCZ(wlc->osh, sizeof(wlc_macdbg_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: macdbg memory alloc. failed\n",
			wlc->pub->unit, __FUNCTION__));
		return NULL;
	}
	macdbg->wlc = wlc;

	if ((wlc_module_register(pub, macdbg_iovars, "macdbg",
		macdbg, wlc_macdbg_doiovar, NULL, wlc_macdbg_up, NULL)) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

#if WL_MACDBG || defined(WLC_HOSTPMAC)
	if (wlc_macdbg_init_dumplist(macdbg) != BCME_OK) {
		goto fail;
	}
#endif /* WL_MACDBG || WLC_HOSTPMAC */

#if WL_MACDBG
	wlc_dump_register(pub, "mac", (dump_fn_t)wlc_dump_mac, (void *)wlc);
	wlc_dump_register(pub, "shmem", (dump_fn_t)wlc_dump_shmem, (void *)wlc);
	wlc_dump_register(pub, "sctpl", (dump_fn_t)wlc_dump_sctpl, (void *)wlc);
	wlc_dump_register(pub, "bcntpl", (dump_fn_t)wlc_dump_bcntpls, (void *)wlc);
	wlc_dump_register(pub, "pio", (dump_fn_t)wlc_dump_pio, (void *)wlc);
#ifdef WL_PSMX
	wlc_dump_register(pub, "macx", (dump_fn_t)wlc_dump_macx, (void *)wlc);
	wlc_dump_register(pub, "shmemx", (dump_fn_t)wlc_dump_shmemx, (void *)wlc);
#endif // endif
#endif /* WL_MACDBG */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(pub, "dma", (dump_fn_t)wlc_dump_dma, (void *)wlc);
#endif // endif
	return macdbg;
fail:
	MFREE(wlc->osh, macdbg, sizeof(*macdbg));
	return NULL;
}

/* add dump enum here */
static int
wlc_macdbg_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int vsize, struct wlc_if *wlcif)
{
	wlc_macdbg_info_t *macdbg = (wlc_macdbg_info_t*)hdl;
	wlc_info_t *wlc = macdbg->wlc;
	int err = BCME_OK;
	wlc_bsscfg_t *bsscfg;
	int32 *ret_int_ptr;
	uint32 *ret_uint_ptr;
	uint32 uint_val = 0;

	ASSERT(macdbg == wlc->macdbg);
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);
	BCM_REFERENCE(ret_int_ptr);
	BCM_REFERENCE(ret_uint_ptr);
	BCM_REFERENCE(uint_val);
	BCM_REFERENCE(bsscfg);

	if (p_len >= (int)sizeof(uint_val))
		bcopy(params, &uint_val, sizeof(uint_val));

	ret_uint_ptr = (uint32 *)arg;
	ret_int_ptr = (int32 *)arg;

	switch (actionid) {
#if WL_MACDBG
		case IOV_GVAL(IOV_MACDBG_PMAC):
		{
			err = wlc_macdbg_pmac(wlc, params, arg, len);
			break;
		}

	case IOV_GVAL(IOV_MACDBG_CAPTURE):
		err = wlc_macdbg_smpl_capture_get(wlc, arg, (uint)len);
		break;

	case IOV_SVAL(IOV_MACDBG_CAPTURE):
	{
		wl_maccapture_params_t *maccapture_params = (wl_maccapture_params_t *)arg;

		if (len < (int)sizeof(wl_maccapture_params_t)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		err = wlc_macdbg_smpl_capture_set(wlc, maccapture_params);
		break;
	}
#endif /* WL_MACDBG */
#if (defined(BCMDBG) || defined(BCMQT) || defined(WLTEST)) && defined(WL_PSMX)
		case IOV_GVAL(IOV_MACDBG_SHMX):
		{
			rw_reg_t *rwt = params;
			if (rwt->size != 2) {
				err = BCME_BADLEN;
				break;
			}
			if (rwt->byteoff & (rwt->size - 1)) {
				err = BCME_BADADDR;
				break;
			}
			*ret_int_ptr = wlc_read_shmx(wlc, rwt->byteoff);
			break;
		}
		case IOV_SVAL(IOV_MACDBG_SHMX):
		{
			rw_reg_t *rwt = params;
			if (rwt->size != 2) {
				err = BCME_BADLEN;
				break;
			}
			if (rwt->byteoff & (rwt->size - 1)) {
				err = BCME_BADADDR;
				break;
			}
			wlc_write_shmx(wlc, rwt->byteoff, (uint16)rwt->val);
			break;
		}
		case IOV_GVAL(IOV_MACDBG_REGX):
		{
			rw_reg_t *rwt = params;
			if (rwt->size != 2 && rwt->size != 4) {
				err = BCME_BADLEN;
				break;
			}
			if (rwt->byteoff & (rwt->size - 1)) {
				err = BCME_BADADDR;
				break;
			}
			if (rwt->byteoff < D11REG_IHR_BASE) {
				*ret_int_ptr = R_REG(wlc->osh,
					(volatile uint32*)((uint8 *)wlc->regs + rwt->byteoff));
			} else {
				*ret_int_ptr = wlc_read_macregx(wlc, rwt->byteoff);
			}
			break;
		}
		case IOV_SVAL(IOV_MACDBG_REGX):
		{
			rw_reg_t *rwt = params;
			if (rwt->size != 2 && rwt->size != 4) {
				err = BCME_BADLEN;
				break;
			}
			if (rwt->byteoff & (rwt->size - 1)) {
				err = BCME_BADADDR;
				break;
			}
			if (rwt->byteoff < D11REG_IHR_BASE) {
				W_REG(wlc->osh,
					(volatile uint32*)((uint8 *)wlc->regs + rwt->byteoff),
					rwt->val);
			} else {
				wlc_write_macregx(wlc, rwt->byteoff, (uint16)rwt->val);
			}
			break;
		}
#endif /* (BCMDBG || BCMQT || WLTEST) && WL_PSMX */
#ifdef DBG_RXDMA_STUCK_WAR
		case IOV_GVAL(IOV_MACDBG_TRIG_RXSTUCK): {
			if (wlc->pub->trig_rxstuck == FALSE) {
				wlc->pub->trig_rxstuck = TRUE;
			}
			break;
		}
#endif // endif
		case IOV_GVAL(IOV_MACDBG_RXSTUCKTMO): {
			*ret_uint_ptr = (uint32)(wlc->pub->rxstuck_tmo);
			break;
		}
		case IOV_SVAL(IOV_MACDBG_RXSTUCKTMO): {
			wlc->pub->rxstuck_tmo = uint_val;
			break;
		}
		default:
			err = BCME_UNSUPPORTED;
	}
	return err;
}

/* Called during 'wl up' */
static int
wlc_macdbg_up(void *hdl)
{

	wlc_macdbg_info_t *macdbg = (wlc_macdbg_info_t *)hdl;
#if WL_MACDBG
	wlc_info_t *wlc = macdbg->wlc;

	/* If MAC Sample Capture is set-up, start */
	if (((macdbg->smpl_ctrl) & SC_STRT) && (macdbg->smpl_info)) {
		wlc_macdbg_smpl_capture_set(wlc, (wl_maccapture_params_t *)macdbg->smpl_info);
	}
#endif /* WL_MACDBG */

	macdbg->log_done = 0;

	return BCME_OK;
}

#if defined(WLC_HOSTPMAC)
/* Send d11 register lists up to DHD by event */
void
BCMATTACHFN(wlc_macdbg_sendup_d11regs)(wlc_macdbg_info_t *macdbg)
{
	wlc_info_t *wlc = macdbg->wlc;

	/* Enable MACDBG event */
	wlc_eventq_set_ind(wlc->eventq, WLC_E_MACDBG, TRUE);

	if (macdbg->pd11regs && macdbg->d11regs_sz > 0) {
		wlc_mac_event(wlc, WLC_E_MACDBG, NULL, WLC_E_STATUS_SUCCESS,
			WLC_E_MACDBG_LIST_PSM, 0, (void *)macdbg->pd11regs,
			(macdbg->d11regs_sz * sizeof(macdbg->pd11regs[0])));
	}

#if defined(WL_PSMX)
	if (macdbg->pd11regs_x && macdbg->d11regsx_sz > 0) {
		wlc_mac_event(wlc, WLC_E_MACDBG, NULL, WLC_E_STATUS_SUCCESS,
			WLC_E_MACDBG_LIST_PSMX, 0, (void *)macdbg->pd11regs_x,
			(macdbg->d11regsx_sz * sizeof(macdbg->pd11regs_x[0])));
	}
#endif /* WL_PSMX */

#if !WL_MACDBG
	/* DHD will keep the list, and dongle will never need these again. */
	macdbg->pd11regs = NULL;
	macdbg->d11regs_sz = 0;
	macdbg->pd11regs_x = NULL;
	macdbg->d11regsx_sz = 0;
#endif // endif
	return;
}
#endif /* WLC_HOSTPMAC */

#if WL_MACDBG || defined(WLC_HOSTPMAC)
static int
BCMATTACHFN(wlc_macdbg_init_dumplist)(wlc_macdbg_info_t *macdbg)
{
	wlc_info_t *wlc = macdbg->wlc;
	uint32 corerev = wlc->pub->corerev;

#ifdef WLC_MINMACLIST
	if (D11REV_GE(corerev, 40)) {
		macdbg->pd11regs = d11regsmin_ge40;
		macdbg->d11regs_sz = d11regsmin_ge40sz;
	} else {
		macdbg->pd11regs = d11regsmin_pre40;
		macdbg->d11regs_sz = d11regsmin_pre40sz;
	}
	macdbg->pd11regs_x = NULL;
#else /* WLC_MINMACLIST */
	if (D11REV_IS(corerev, 23)) {
		macdbg->pd11regs = d11regs23;
		macdbg->d11regs_sz = d11regs23sz;
	}
	else if (D11REV_LT(corerev, 40)) {
		macdbg->pd11regs = d11regs_pre40;
		macdbg->d11regs_sz = d11regs_pre40sz;
	}
	else if (D11REV_IS(corerev, 48)) {
		macdbg->pd11regs = d11regs48;
		macdbg->d11regs_sz = d11regs48sz;
	}
	else if (D11REV_IS(corerev, 49)) {
		macdbg->pd11regs = d11regs49;
		macdbg->d11regs_sz = d11regs49sz;
	}
	else if (D11REV_IS(corerev, 64)) {
		macdbg->pd11regs = d11regs64;
		macdbg->d11regs_sz = d11regs64sz;
	}
	else if (D11REV_GE(corerev, 65)) {
		macdbg->pd11regs = d11regs65;
		macdbg->d11regs_sz = d11regs65sz;
	}
	else {
		/* Default */
		macdbg->pd11regs = d11regs42;
		macdbg->d11regs_sz = d11regs42sz;
	}
#if defined(WL_PSMX)
	if (D11REV_IS(corerev, 64)) {
		macdbg->pd11regs_x = d11regsx64;
		macdbg->d11regsx_sz = d11regsx64sz;
	}
	else if (D11REV_GE(corerev, 65)) {
		macdbg->pd11regs_x = d11regsx65;
		macdbg->d11regsx_sz = d11regsx65sz;
	} else {
		macdbg->pd11regs_x = NULL;
		macdbg->d11regsx_sz = 0;
	}
#endif /* WL_PSMX */
#endif /* WLC_MINMACLIST */

	WL_TRACE(("%s d11reg %p size %d d11reg_x %p size %d\n", __FUNCTION__,
		macdbg->pd11regs, macdbg->d11regs_sz,
		macdbg->pd11regs_x, macdbg->d11regsx_sz));

	return BCME_OK;
}
#endif /* WL_MACDBG || WLC_HOSTPMAC */

#if WL_MACDBG
/* write functions */
static int
wlc_write_d11reg(wlc_info_t *wlc, int idx, int type, uint16 addr, uint32 w_val)
{
	d11regs_t *regs;
	osl_t *osh;
	uint16 w_val16 = (uint16)w_val;
	volatile uint8 *paddr;

	osh = wlc->osh;
	regs = wlc->regs;
	paddr = (volatile uint8*)(&regs->biststatus) - 0xC;

	switch (type) {
	case D11REG_TYPE_IHR32:
		W_REG(osh, (volatile uint32*)(paddr + addr), w_val);
		break;
	case D11REG_TYPE_IHR16:
		W_REG(osh, (volatile uint16*)(paddr + addr), w_val16);
		break;
	case D11REG_TYPE_SCR:
		wlc_bmac_copyto_objmem(wlc->hw, addr << 2,
			&w_val16, sizeof(w_val16), OBJADDR_SCR_SEL);
		break;
	case D11REG_TYPE_SHM:
		wlc_write_shm(wlc, addr, w_val16);
		break;
	case D11REG_TYPE_TPL:
		W_REG(osh, &regs->tplatewrptr, addr);
		W_REG(osh, &regs->tplatewrdata, w_val);
		break;
#if defined(WL_PSMX)
	case D11REG_TYPE_KEYTB:
		wlc_bmac_copyto_objmem(wlc->hw, addr,
			&w_val, sizeof(w_val), OBJADDR_KEYTBL_SEL);
		break;
	case D11REG_TYPE_IHRX16:
		wlc_write_macregx(wlc, addr, w_val16);
		break;
	case D11REG_TYPE_SCRX:
		wlc_bmac_copyto_objmem(wlc->hw, addr << 2,
			&w_val16, sizeof(w_val16), OBJADDR_SCRX_SEL);
		break;
	case D11REG_TYPE_SHMX:
		wlc_bmac_copyto_objmem(wlc->hw, addr,
			&w_val16, sizeof(w_val16), OBJADDR_SHMX_SEL);
		break;
#endif /* WL_PSMX */
	default:
		printf("%s: unrecognized type %d!\n", __FUNCTION__, type);
		return 0;
	}
	return 1;
}

/* dump functions */
static int
wlc_print_d11reg(wlc_info_t *wlc, int idx, int type, uint16 addr, struct bcmstrbuf *b)
{
	d11regs_t *regs;
	osl_t *osh;
	uint16 val16 = -1;
	uint32 val32 = -1;
	bool print16 = TRUE;
	volatile uint8 *paddr;
	const char *regname[D11REG_TYPE_MAX] = D11REGTYPENAME;

	osh = wlc->osh;
	regs = wlc->regs;
	paddr = (volatile uint8*)(&regs->biststatus) - 0xC;

	switch (type) {
	case D11REG_TYPE_IHR32:
		val32 = R_REG(osh, (volatile uint32*)(paddr + addr));
		print16 = FALSE;
		break;
	case D11REG_TYPE_IHR16:
		val16 = R_REG(osh, (volatile uint16*)(paddr + addr));
		break;
	case D11REG_TYPE_SCR:
		wlc_bmac_copyfrom_objmem(wlc->hw, addr << 2,
			&val16, sizeof(val16), OBJADDR_SCR_SEL);
		break;
	case D11REG_TYPE_SHM:
		val16 = wlc_read_shm(wlc, addr);
		break;
	case D11REG_TYPE_TPL:
		W_REG(osh, &regs->tplatewrptr, addr);
		val32 = R_REG(osh, &regs->tplatewrdata);
		print16 = FALSE;
		break;
#if defined(WL_PSMX)
	case D11REG_TYPE_KEYTB:
		wlc_bmac_copyfrom_objmem(wlc->hw, addr,
			&val32, sizeof(val32), OBJADDR_KEYTBL_SEL);
		print16 = FALSE;
		break;
	case D11REG_TYPE_IHRX16:
		val16 = wlc_read_macregx(wlc, addr);
		break;
	case D11REG_TYPE_SCRX:
		wlc_bmac_copyfrom_objmem(wlc->hw, addr << 2,
			&val16, sizeof(val16), OBJADDR_SCRX_SEL);
		break;
	case D11REG_TYPE_SHMX:
		wlc_bmac_copyfrom_objmem(wlc->hw, addr,
			&val16, sizeof(val16), OBJADDR_SHMX_SEL);
		break;
#endif /* WL_PSMX */
	default:
		if (b) {
			bcm_bprintf(b, "%s: unrecognized type %d!\n", __FUNCTION__, type);
		} else {
			printf("%s: unrecognized type %d!\n", __FUNCTION__, type);
		}
		return 0;
	}
	if (print16) {
		if (b) {
			bcm_bprintf(b, "%-3d %s 0x%-4x = 0x%-4x\n",
				idx, regname[type], addr, val16);
		} else {
			printf("%-3d %s 0x%-4x = 0x%-4x\n",
				idx, regname[type], addr, val16);
		}
	} else {
		if (b) {
			bcm_bprintf(b, "%-3d %s 0x%-4x = 0x%-8x\n",
				idx, regname[type], addr, val32);
		} else {
			printf("%-3d %s 0x%-4x = 0x%-8x\n",
				idx, regname[type], addr, val32);
		}
	}
	return 1;
}

static int
wlc_pw_d11regs(wlc_info_t *wlc, CONST d11regs_list_t *pregs,
	int start_idx, struct bcmstrbuf *b, bool w_en, uint32 w_val)
{
	uint32 lbmp;
	uint16 addr, lcnt;
	int idx;

	addr = pregs->addr;
	idx = start_idx;
	if (pregs->type >= D11REG_TYPE_MAX) {
		if (b && !w_en) {
			bcm_bprintf(b, "%s: wrong type %d\n", __FUNCTION__, pregs->type);
		} else {
			printf("%s: wrong type %d\n", __FUNCTION__, pregs->type);
		}
		return 0;
	}
	lbmp = pregs->bitmap;
	lcnt = pregs->cnt;
	while (lbmp || lcnt) {
		WL_TRACE(("idx %d bitmap %#x cnt %d addr %#x\n",
			idx, lbmp, lcnt, addr));
		if ((lbmp && (lbmp & 0x1)) || (!lbmp && lcnt)) {
			if (w_en) {
				idx += wlc_write_d11reg(wlc, idx, pregs->type, addr, w_val);
			} else {
				idx += wlc_print_d11reg(wlc, idx, pregs->type, addr, b);
			}
			if (lcnt) lcnt --;
		}
		lbmp = lbmp >> 1;
		addr += pregs->step;
	}
	return (idx - start_idx);
}

static int
wlc_pd11regs_bylist(wlc_info_t *wlc, CONST d11regs_list_t *d11dbg1,
	uint d11dbg1_sz, int start_idx, struct bcmstrbuf *b)
{
	CONST d11regs_list_t *pregs;
	int i, idx;

	if (!wlc->clk)
		return BCME_NOCLK;

	WL_INFORM(("%s: ucode compile time 0x%04x 0x%04x\n", __FUNCTION__,
		wlc_read_shm(wlc, 0x4), wlc_read_shm(wlc, 0x6)));

	idx = start_idx;
	for (i = 0; i < (int)d11dbg1_sz; i++) {
		pregs = &(d11dbg1[i]);
		idx += wlc_pw_d11regs(wlc, pregs, idx, b, FALSE, 0);
	}
	return (idx - start_idx);
}

static int
wlc_pw_d11regs_byaddr(wlc_info_t *wlc, CONST d11regs_addr_t *d11addrs,
	uint d11addrs_sz, int start_idx, struct bcmstrbuf *b, bool w_en, uint32 w_val)
{
	CONST d11regs_addr_t *paddrs;
	int i, j, idx;

	if (!wlc->clk)
		return BCME_NOCLK;

	idx = start_idx;
	for (i = 0; i < (int)d11addrs_sz; i++) {
		paddrs = &(d11addrs[i]);
		if (paddrs->type >= D11REG_TYPE_MAX) {
			if (b && !w_en)
				bcm_bprintf(b, "%s: wrong type %d. Skip %d entries.\n",
					__FUNCTION__, paddrs->type, paddrs->cnt);
			else
				printf("%s: wrong type %d. Skip %d entries.\n",
				       __FUNCTION__, paddrs->type, paddrs->cnt);
			continue;
		}
		for (j = 0; j < paddrs->cnt; j++) {
			if (w_en) {
				idx += wlc_write_d11reg(wlc, idx, paddrs->type,
					paddrs->addr[j], w_val);
			} else {
				idx += wlc_print_d11reg(wlc, idx, paddrs->type,
					paddrs->addr[j], b);
			}
		}
	}
	return (idx - start_idx);
}

static int
wlc_dump_mac(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	int cnt = 0;
	if (!wlc->clk) {
		WL_ERROR(("wl%d: %s: clock must be on\n", wlc->pub->unit, __FUNCTION__));
		return BCME_NOCLK;
	}
	ASSERT(wlc->macdbg->pd11regs != NULL && wlc->macdbg->d11regs_sz > 0);
	cnt = wlc_pd11regs_bylist(wlc, wlc->macdbg->pd11regs,
		wlc->macdbg->d11regs_sz, 0, b);
	return cnt;
}

static int
wlc_macdbg_pmac(wlc_info_t *wlc,
	wl_macdbg_pmac_param_t *params, char *out_buf, int out_len)
{
	uint8 i, type;
	int err = BCME_OK, idx = 0;
	struct bcmstrbuf bstr, *b;
	wl_macdbg_pmac_param_t pmac_params;
	wl_macdbg_pmac_param_t *pmac = &pmac_params;
	d11regs_list_t d11dbg1;
	d11regs_addr_t d11dbg2;
	bool skip1st = FALSE;
	bool align4 = FALSE;

	if (!wlc->clk)
		return BCME_NOCLK;

	memcpy(pmac, params, sizeof(wl_macdbg_pmac_param_t));
	bcm_binit(&bstr, out_buf, out_len);
	b = &bstr;

	if (WL_TRACE_ON()) {
		printf("%s:\n", __FUNCTION__);
		printf("type %s\n", pmac->type);
		printf("step %u\n", pmac->step);
		printf("num  %u\n", pmac->num);
		printf("bitmap %#x\n", pmac->bitmap);
		printf("addr_raw %d\n", pmac->addr_raw);
		for (i = 0; i < pmac->addr_num; i++)
			printf("\taddr = %#x\n", pmac->addr[i]);
	}

	if (pmac->addr_num == 0) {
		bcm_bprintf(b, "%s line %d: no address is given!\n", __FUNCTION__, __LINE__);
		//err = BCME_BADARG;
		goto exit;
	}

	if (!strncmp(pmac->type, "shmx", 4)) {
		type = D11REG_TYPE_SHMX;
	} else if (!strncmp(pmac->type, "ihrx", 4)) {
		type = D11REG_TYPE_IHRX16;
	} else if (!strncmp(pmac->type, "keytb", 5)) {
		type = D11REG_TYPE_KEYTB;
		align4 = TRUE;
	} else if (!strncmp(pmac->type, "scrx", 4)) {
		type = D11REG_TYPE_SCRX;
	} else if (!strncmp(pmac->type, "scr", 3)) {
		type = D11REG_TYPE_SCR;
	} else if (!strncmp(pmac->type, "shm", 3)) {
		type = D11REG_TYPE_SHM;
	} else if (!strncmp(pmac->type, "tpl", 3)) {
		type = D11REG_TYPE_TPL;
		align4 = TRUE;
	} else if (!strncmp(pmac->type, "ihr32", 5)) {
		type = D11REG_TYPE_IHR32;
		align4 = TRUE;
	} else if (!strncmp(pmac->type, "ihr", 3)) {
		type = D11REG_TYPE_IHR16;
	} else {
		bcm_bprintf(b, "Unrecognized type: %s!\n", pmac->type);
		err = BCME_BADARG;
		goto exit;
	}
	if (type >= D11REG_TYPE_GE64 && D11REV_LT(wlc->pub->corerev, 64)) {
		bcm_bprintf(b, "%s: unsupported type %s for corerev %d!\n",
			__FUNCTION__, pmac->type, wlc->pub->corerev);
		goto exit;
	}

	if (type == D11REG_TYPE_SCR || type == D11REG_TYPE_SCRX) {
		if (pmac->step == (uint8)(-1)) {
			/* Set the default step when it is not given */
			pmac->step = 1;
		}
	} else {
		uint16 mask = align4 ? 0x3 : 0x1;
		for (i = 0; i < pmac->addr_num; i++) {
			if (pmac->addr_raw && !align4) {
				/* internal address => external address.
				 * only applies to 16-bit access type
				 */
				if ((type == D11REG_TYPE_IHR16) ||
					(type == D11REG_TYPE_IHRX16)) {
					pmac->addr[i] += D11REG_IHR_WBASE;
				}
				pmac->addr[i] <<= 1;
			}
			if ((type == D11REG_TYPE_IHR16 || type == D11REG_TYPE_IHRX16) &&
			    pmac->addr[i] < D11REG_IHR_BASE) {
				/* host address space: convert local type */
				type = D11REG_TYPE_IHR32;
				align4 = TRUE;
			}

			mask = align4 ? 0x3 : 0x1;
			if ((pmac->addr[i] & mask)) {
				/* Odd addr not expected here for external addr */
				WL_ERROR(("%s line %d: addr %#x is not %s aligned!\n",
					__FUNCTION__, __LINE__, pmac->addr[i],
					align4 ? "dword" : "word"));
				pmac->addr[i] &= ~mask;
			}
		}
		if (pmac->step == (uint8)(-1)) {
			/* Set the default step when it is not given */
			pmac->step = align4 ? 4 : 2;
		} else if (pmac->step & mask) {
			/* step size validation check. */
			bcm_bprintf(b, "%s line %d: wrong step size %d for type %s\n",
				__FUNCTION__, __LINE__, pmac->step, pmac->type);
			goto exit;
		}
	}

	/* generate formated lists */
	if (pmac->bitmap || pmac->num) {
		skip1st = TRUE;
		d11dbg1.type = type;
		d11dbg1.addr = pmac->addr[0];
		d11dbg1.bitmap = pmac->bitmap;
		d11dbg1.step = pmac->step;
		d11dbg1.cnt = pmac->num;
		WL_TRACE(("d11dbg1: type %d, addr 0x%x, bitmap 0x%x, step %d, cnt %d\n",
			d11dbg1.type,
			d11dbg1.addr,
			d11dbg1.bitmap,
			d11dbg1.step,
			d11dbg1.cnt));
		idx += wlc_pw_d11regs(wlc, &d11dbg1, idx, b,
			(bool)pmac->w_en, pmac->w_val);
	}
	if (pmac->addr_num) {
		int num = pmac->addr_num;
		uint16 *paddr = d11dbg2.addr;
		if (skip1st) {
			num --;
			paddr ++;
		}
		d11dbg2.type = type;
		d11dbg2.cnt = (uint16)num;
		memcpy(paddr, pmac->addr, (sizeof(uint16) * num));
		WL_TRACE(("d11dbg2: type %d cnt %d\n",
			d11dbg2.type,
			d11dbg2.cnt));
		for (i = 0; i < num; i++) {
			WL_TRACE(("[%d] 0x%x\n", i, d11dbg2.addr[i]));
		}
		if (num > 0) {
			idx += wlc_pw_d11regs_byaddr(wlc, &d11dbg2, 1, idx, b,
				(bool)pmac->w_en, pmac->w_val);
		}
	}
	if (idx == 0) {
		/* print nothing */
		err = BCME_BADARG;
	}
exit:
	return err;
}

static int
wlc_dump_shmem(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	uint i, k = 0;
	uint16 val, addr, end;

	static const shmem_list_t shmem_list[] = {
		{0, 3*1024},
	};

	if (!wlc->clk) {
		WL_ERROR(("wl%d: %s: clock must be on\n", wlc->pub->unit, __FUNCTION__));
		return BCME_NOCLK;
	}
	for (i = 0; i < ARRAYSIZE(shmem_list); i++) {
		end = shmem_list[i].start + 2 * shmem_list[i].cnt;
		for (addr = shmem_list[i].start; addr < end; addr += 2) {
			val = wlc_read_shm(wlc, addr);
			if (b) {
				bcm_bprintf(b, "%d shm 0x%03x = 0x%04x\n",
					k++, addr, val);
			} else {
				printf("%d shm 0x%03x = 0x%04x\n",
				       k++, addr, val);
			}
		}
	}

	return 0;
}

/* wl dump sctpl [-u shm shmx] */
#define DUMP_SC_TYPE_UTSHM	(1 << 0)
#define DUMP_SC_TYPE_UTSHMX	(1 << 1)
#define DUMP_SC_ARGV_MAX	128
static int
wlc_dump_sctpl_parse_args(wlc_info_t *wlc, uint32 *type)
{
	int err = BCME_OK;
	char *args = wlc->dump_args;
	char *p, **argv = NULL;
	uint argc = 0;
	char opt, curr = '\0';

	if (args == NULL || type == NULL) {
		err = BCME_BADARG;
		goto exit;
	}

	/* allocate argv */
	if ((argv = MALLOC(wlc->osh, sizeof(*argv) * DUMP_SC_ARGV_MAX)) == NULL) {
		WL_ERROR(("wl%d: %s: failed to allocate the argv buffer\n",
		          wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	/* get each token */
	p = bcmstrtok(&args, " ", 0);
	while (p && argc < DUMP_SC_ARGV_MAX-1) {
		argv[argc++] = p;
		p = bcmstrtok(&args, " ", 0);
	}
	argv[argc] = NULL;

	/* initial default */
	*type = 0;

	/* parse argv */
	argc = 0;
	while ((p = argv[argc++])) {
		if (!strncmp(p, "-", 1)) {
			if (strlen(p) > 2) {
				err = BCME_BADARG;
				goto exit;
			}
			opt = p[1];

			switch (opt) {
				case 'u':
					curr = 'u';
					break;
				default:
					err = BCME_BADARG;
					goto exit;
			}
		} else {
			switch (curr) {
				case 'u':
					if (!strcmp(p, "shm")) {
						*type = DUMP_SC_TYPE_UTSHM;
					} else if (!strcmp(p, "shmx")) {
						*type |= DUMP_SC_TYPE_UTSHMX;
					}
					break;
				default:
					err = BCME_BADARG;
					goto exit;
			}
		}
	}

exit:
	if (argv) {
		MFREE(wlc->osh, argv, sizeof(*argv) * DUMP_SC_ARGV_MAX);
	}

	return err;
}

static int
wlc_dump_sctpl(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	d11regs_t *regs;
	osl_t *osh;
	wlc_pub_t *pub;
	uint i;
	int gpio_sel;
	uint16 scpctl, addr0, addr1, curptr, len, offset;

	pub = wlc->pub;
	if (D11REV_LT(pub->corerev, 40)) {
		WL_ERROR(("wl%d: %s only supported for corerev >= 40\n",
			pub->unit, __FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	if (!wlc->clk) {
		WL_ERROR(("wl%d: %s: clock must be on\n", pub->unit, __FUNCTION__));
		return BCME_NOCLK;
	}

	regs = wlc->regs;
	osh = wlc->osh;

	/* Parse args if needed */
	if (wlc->dump_args) {
		uint32 type = DUMP_SC_TYPE_UTSHMX;
		int err = BCME_OK;

		err = wlc_dump_sctpl_parse_args(wlc, &type);
		if (err != BCME_OK)
			return err;

		if (type == DUMP_SC_TYPE_UTSHM) {
		} else if (type == DUMP_SC_TYPE_UTSHMX) {
#if defined(WL_PSMX)
			return wlc_dump_sctpl_shmx(wlc, b);
#endif // endif
		}
	}

	/* stop sample capture */
	if (D11REV_IS(pub->corerev, 50) || D11REV_GE(pub->corerev, 54)) {
		scpctl = R_REG(osh, &regs->u.d11acregs.SampleCollectPlayCtrl);
		W_REG(osh, &regs->u.d11acregs.SampleCollectPlayCtrl, scpctl & ~SC_STRT);
	} else {
		scpctl = R_REG(osh, &regs->psm_phy_hdr_param);
		W_REG(osh, &regs->psm_phy_hdr_param, scpctl & ~PHYCTL_SC_STRT);
	}

	if (b) {
		bcm_bprintf(b, "corerev: %d ucode revision %d.%d features 0x%04x\n",
			pub->corerev, wlc_read_shm(wlc, M_BOM_REV_MAJOR),
			wlc_read_shm(wlc, M_BOM_REV_MINOR), wlc_read_shm(wlc, M_UCODE_FEATURES));
	} else {
		printf("corerev: %d ucode revision %d.%d features 0x%04x\n",
			pub->corerev, wlc_read_shm(wlc, M_BOM_REV_MAJOR),
			wlc_read_shm(wlc, M_BOM_REV_MINOR), wlc_read_shm(wlc, M_UCODE_FEATURES));
	}

	gpio_sel = R_REG(osh, &regs->maccontrol1);
	addr0 = R_REG(osh, &regs->u.d11acregs.SampleCollectStartPtr);
	addr1 = R_REG(osh, &regs->u.d11acregs.SampleCollectStopPtr);
	curptr = R_REG(osh, &regs->u.d11acregs.SampleCollectCurPtr);
	len = (addr1 - addr0 + 1) * 4;
	offset = addr0 * 4;

	if (b) {
		bcm_bprintf(b, "Capture mode: maccontrol1 0x%02x scctl 0x%02x\n",
			gpio_sel, scpctl);
		bcm_bprintf(b, "Start/stop/cur 0x%04x 0x%04x 0x%04x byt_offset 0x%04x entries %u\n",
			addr0, addr1, curptr, 4 *(curptr - addr0), len>>2);
		bcm_bprintf(b, "offset: low high\n");
	} else {
		printf("Capture mode: maccontrol1 0x%02x scpctl 0x%02x\n", gpio_sel, scpctl);
		printf("Start/stop/cur 0x%04x 0x%04x 0x%04x byt_offset 0x%04x entries %u\n",
		       addr0, addr1, curptr, 4 *(curptr - addr0), len>>2);
		printf("offset: low high\n");
	}

	W_REG(osh, &regs->tplatewrptr, offset);

	for (i = 0; i < (uint)len; i += 4) {
		uint32 tpldata;
		uint16 low16, hi16;

		tpldata = R_REG(osh, &regs->tplatewrdata);
		hi16 = (tpldata >> 16) & 0xffff;
		low16 = tpldata & 0xffff;
		if (b)
			bcm_bprintf(b, "%04X: %04X %04X\n", i, low16, hi16);
		else
			printf("%04X: %04X %04X\n", i, low16, hi16);
	}
	return BCME_OK;
}

/** dump beacon (from read_txe_ram in d11procs.tcl) */
static void
wlc_dump_bcntpl(wlc_info_t *wlc, struct bcmstrbuf *b, int offset, int len)
{
	d11regs_t *regs = wlc->regs;
	osl_t *osh = wlc->osh;
	uint i;

	len = (len + 3) & ~3;
	W_REG(osh, &regs->tplatewrptr, offset);
	bcm_bprintf(b, "tpl: offset %d len %d\n", offset, len);
	for (i = 0; i < (uint)len; i += 4) {
		bcm_bprintf(b, "%04X: %08X\n", i,
			R_REG(osh, &regs->tplatewrdata));
	}
}

static int
wlc_dump_bcntpls(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	uint16 len;

	if (!wlc->clk) {
		WL_ERROR(("wl%d: %s: clock must be on\n", wlc->pub->unit, __FUNCTION__));
		return BCME_NOCLK;
	}

	len = wlc_read_shm(wlc, M_BCN0_FRM_BYTESZ);
	bcm_bprintf(b, "bcn 0: len %u\n", len);
	wlc_dump_bcntpl(wlc, b, D11AC_T_BCN0_TPL_BASE, len);
	len = wlc_read_shm(wlc, M_BCN1_FRM_BYTESZ);
	bcm_bprintf(b, "bcn 1: len %u\n", len);
	wlc_dump_bcntpl(wlc, b, D11AC_T_BCN1_TPL_BASE, len);

	return 0;
}

static int
wlc_dump_pio(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	int i;

	if (!wlc->clk)
		return BCME_NOCLK;

	if (!PIO_ENAB(wlc->pub))
		return 0;

	for (i = 0; i < NFIFO; i++) {
		pio_t *pio = WLC_HW_PIO(wlc, i);
		bcm_bprintf(b, "PIO %d: ", i);
		if (pio != NULL)
			wlc_pio_dump(pio, b);
		bcm_bprintf(b, "\n");
	}

	return 0;
}
#endif /* WL_MACDBG */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#define DUMP_DMA_TYPE_RX	(1 << 0)
#define DUMP_DMA_TYPE_TX	(1 << 1)
#define DUMP_DMA_TYPE_AQM	(1 << 2)
#define DUMP_DMA_TYPE_ALL	(DUMP_DMA_TYPE_RX|DUMP_DMA_TYPE_TX|DUMP_DMA_TYPE_AQM)
#define DUMP_DMA_FIFO_ALL	(0xFFFFFFFF)
#define DUMP_DMA_ARGV_MAX	128
/* wl dump dma -t [all tx rx aqm] -f [all 0 1 2 3...] -r */
static int
wlc_dump_dma_parse_args(wlc_info_t *wlc, uint32 *type, uint32 *fifo, bool *ring)
{
	int err = BCME_OK;
	char *args = wlc->dump_args;
	char *p, **argv = NULL;
	uint argc = 0;
	char opt, curr = '\0';
	uint8 val;

	if (args == NULL || type == NULL || fifo == NULL || ring == NULL) {
		err = BCME_BADARG;
		goto exit;
	}

	/* allocate argv */
	if ((argv = MALLOC(wlc->osh, sizeof(*argv) * DUMP_DMA_ARGV_MAX)) == NULL) {
		WL_ERROR(("wl%d: %s: failed to allocate the argv buffer\n",
		          wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	/* get each token */
	p = bcmstrtok(&args, " ", 0);
	while (p && argc < DUMP_DMA_ARGV_MAX-1) {
		argv[argc++] = p;
		p = bcmstrtok(&args, " ", 0);
	}
	argv[argc] = NULL;

	/* initial default */
	*type = 0;
	*fifo = 0;
	*ring = FALSE;

	/* parse argv */
	argc = 0;
	while ((p = argv[argc++])) {
		if (!strncmp(p, "-", 1)) {
			if (strlen(p) > 2) {
				err = BCME_BADARG;
				goto exit;
			}
			opt = p[1];

			switch (opt) {
				case 't':
					curr = 't';
					break;
				case 'f':
					curr = 'f';
					break;
				case 'r':
					curr = 'r';
					*ring = TRUE;
					break;
				default:
					err = BCME_BADARG;
					goto exit;
			}
		} else {
			switch (curr) {
				case 't':
					if (!strcmp(p, "all")) {
						*type = DUMP_DMA_TYPE_ALL;
					} else if (!strcmp(p, "rx")) {
						*type |= DUMP_DMA_TYPE_RX;
					} else if (!strcmp(p, "tx")) {
						*type |= DUMP_DMA_TYPE_TX;
					} else if (!strcmp(p, "aqm")) {
						*type |= DUMP_DMA_TYPE_AQM;
					}
					break;
				case 'f':
					if (strcmp(p, "all") == 0) {
						*fifo = DUMP_DMA_FIFO_ALL;
					}
					else {
						val = (uint8)bcm_strtoul(p, NULL, 0);
						if (val < 32)
							*fifo |= (1 << val);
					}
					break;
				default:
					err = BCME_BADARG;
					goto exit;
			}
		}
	}

exit:
	if (argv) {
		MFREE(wlc->osh, argv, sizeof(*argv) * DUMP_DMA_ARGV_MAX);
	}

	return err;
}

static int
wlc_dump_dma(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	uint i;
	wl_cnt_wlc_t *cnt = wlc->pub->_cnt;
	int err = BCME_OK;
	uint32 type = DUMP_DMA_TYPE_ALL;
	uint32 fifo = DUMP_DMA_FIFO_ALL;
	bool ring = TRUE;

	if (!wlc->clk)
		return BCME_NOCLK;

	/* Parse args if needed */
	if (wlc->dump_args) {
		err = wlc_dump_dma_parse_args(wlc, &type, &fifo, &ring);
		if (err != BCME_OK)
			return err;
	}

	/* Start dump */
	for (i = 0; i < WLC_HW_NFIFO_INUSE(wlc); i++) {
		if (i == 6 || i == 7 || !((fifo >> i) & 0x1))
			continue;

		if (i < ARRAYSIZE(wlc->regs->intctrlregs)) {
			PRREG_INDEX(intstatus, wlc->regs->intctrlregs[i].intstatus);
			PRREG_INDEX(intmask, wlc->regs->intctrlregs[i].intmask);
		}
		PRNL();

		if (PIO_ENAB(wlc->pub)) {
			continue;
		}

		if (type & DUMP_DMA_TYPE_ALL) {
			hnddma_t *di = WLC_HW_DI(wlc, i);

			if (di) {
				bcm_bprintf(b, "DMA %d:\n", i);
				if (type & DUMP_DMA_TYPE_RX) {
					if (wlc->dump_args == NULL) {
						if ((i == RX_FIFO) ||
							(D11REV_IS(wlc->pub->corerev, 4) &&
							(i == RX_TXSTATUS_FIFO))) {
							bcm_bprintf(b, "  RX%d: ", i);
							dma_dumprx(di, b, TRUE);
							PRVAL(rxuflo[i]);
							PRNL();
						}
					}
					else {
						bcm_bprintf(b, "  RX%d: ", i);
						dma_dumprx(di, b, ring);
						PRVAL(rxuflo[i]);
						PRNL();
					}
				}

				if (type & DUMP_DMA_TYPE_TX) {
					bcm_bprintf(b, "  TX%d: ", i);
					dma_dumptx(di, b, ring);
					PRVAL(txuflo);
					PRNL();
				}
			}
#ifdef BCM_DMA_CT
			if (BCM_DMA_CT_ENAB(wlc) && (type & DUMP_DMA_TYPE_AQM)) {
				hnddma_t *di = WLC_HW_AQM_DI(wlc, i);
				if (di) {
					bcm_bprintf(b, " AQM%d: ", i);
					dma_dumptx(di, b, ring);
				}
			}
#endif /* BCM_DMA_CT */
			PRNL();
		}
	}
	PRVAL(dmada); PRVAL(dmade); PRVAL(rxoflo); PRVAL(dmape);
	PRNL();

	return err;
}
#endif  /* defined(BCMDBG) || defined(BCMDBG_DUMP) */

/* ************* end of dump_xxx function section *************************** */

/* print upon critical or fatal error */
static void
wlc_suspend_mac_debug(wlc_info_t *wlc, uint32 phydebug)
{
	osl_t *osh;
	d11regs_t *regs;
	wlcband_t *band = wlc->band;
	uint16 val, addr;
	int loop;

	d11print_list_t acphyreg[] = {
		{"fineclk_gatectl", 0x16b},
		{"RfseqStatus0", 0x403},
		{"RfctrlCmd", 0x408}
	};

	/* dump additional regs */
	if (WLCISACPHY(band)) {
		addr = 0x1f0;
	} else if (WLCISHTPHY(band) || WLCISNPHY(band)) {
		addr = 0x183;
	} else {
		return;
	}

	osh = wlc->osh;
	regs = wlc->regs;
	for (loop = 0; loop < 10; loop++) {
		W_REG(osh, &regs->phyregaddr, addr);
		val = R_REG(osh, &regs->phyregdata);
		WL_PRINT(("PHY reg %#x = 0x%x\n", addr, val));
		OSL_DELAY(10);
	}
	if (!WLCISACPHY(band)) {
		return;
	}

	for (loop = 0; loop < ARRAYSIZE(acphyreg); loop ++) {
		W_REG(osh, &regs->phyregaddr, acphyreg[loop].addr);
		val = R_REG(osh, &regs->phyregdata);
		WL_PRINT(("PHY reg %#x = %#x (%s)\n",
			acphyreg[loop].addr, val, acphyreg[loop].name));
	}

#ifdef WAR4360_UCODE
	W_REG(osh, &regs->radioregaddr, (0x16a | (4<<9)));
	val = R_REG(osh, &regs->radioregdata);
	WL_PRINT(("RADIO reg 0x16a = 0x%x (OVR14 - AFE_PU)\n", val));

	W_REG(osh, &regs->radioregaddr, (0x8e | (4<<9)));
	val = R_REG(osh, &regs->radioregdata);
	WL_PRINT(("RADIO reg 0x8e = 0x%x (pll_adc6 - ADC_PU)\n", val));

	WL_PRINT(("PSM_STACK_STATUS : 0x%x\n", R_REG(osh, &regs->psm_srs_status)));
	WL_PRINT(("PSM_STACK_ENTRIES:\n"));
	for (loop = 0; loop < 8; loop++) {
		W_REG(osh, &regs->psm_srs_ptr, loop);
		val = R_REG(osh, &regs->psm_srs_entry);
		WL_PRINT(("0x%04x\n", val));
	}
#endif /* WAR4360_UCODE */
	return;
}

#ifdef NOT_YET
static void
wlc_macdbg_dump_aggr(wlc_info_t *wlc)
{
#if defined(BCMDBG)|| defined(BCMDBG_DUMP) || defined(WLTEST)
	uint8 i, k;
	uint8 max_txc = 1;
	uint16 aggnum[4] = {0, 0, 0, 0}, maxaggnum = 0;
	osl_t *osh;
	d11regs_t *regs;
	uint16 orig_psm_reg_mux;

	if (D11REV_LT(wlc->pub->corerev, 64)) {
		return;
	}

	osh = wlc->osh;
	regs = wlc->regs;
	orig_psm_reg_mux = R_REG(osh, &regs->u.d11acregs.psm_reg_mux);

#ifdef WL_MU_TX
	// read S_CUR_TXMU if MUTX is enabled
	if (MU_TX_ENAB(wlc)) {
		uint16 s_cur_txmu;
		wlc_bmac_copyfrom_objmem(wlc->hw, 0x40 << 2, &s_cur_txmu,
			sizeof(s_cur_txmu), OBJADDR_SCR_SEL);
		WL_PRINT(("S_CUR_TXMU 0x%04x\n", s_cur_txmu));
		max_txc = 4;
	}
#endif /* WL_MU_TX */
	for (i = 0; i < max_txc; i++) {
		uint32 agglen, bytcntintxfrm;
		k = 1 << i;
		W_REG(osh, &regs->u.d11acregs.psm_reg_mux, (0xfff0 + i));
		while (R_REG(osh, &regs->u.d11acregs.psm_reg_mux) != (0xfff0 + i));

		aggnum[i] = R_REG(osh, &regs->u.d11acregs.AQMAggNum);
		if (aggnum[i] > maxaggnum) {
			maxaggnum = aggnum[i];
		}
		agglen = (R_REG(osh, &regs->u.d11acregs.AQMAggLenHi) << 16) |
			R_REG(osh, &regs->u.d11acregs.AQMAggLenLow);
		bytcntintxfrm = (R_REG(osh, &regs->u.d11acregs.BytCntInTxFrmHi) << 16) |
			R_REG(osh, &regs->u.d11acregs.BytCntInTxFrmLo);

		WL_PRINT(("txc[%d] aggstat 0x%04x rptr %d bqfrmcnt %d minmpdu %d\n",
			i, R_REG(osh, &regs->u.d11acregs.AQMAggStats),
			R_REG(osh, &regs->u.d11acregs.AQMAggRptr),
			R_REG(osh, &regs->u.d11acregs.u0.ge64.BMC_BQFrmCnt),
			R_REG(osh, &regs->u.d11acregs.AQMMinMpduLen)));
		WL_PRINT(("agglen 0x%08x aggnum %d bytcntintxfrm 0x%08x\n",
			agglen, aggnum[i], bytcntintxfrm));
#ifdef WL_MU_TX
		if (MU_TX_ENAB(wlc)) {
			uint16 dur, mpdudur, ctxptr, rt500k;
			ctxptr = R_REG(osh, &regs->u.d11acregs.psm_base[0]);
			rt500k = wlc_read_shmx(wlc, (ctxptr+22) * 2);
			dur = rt500k ? ((agglen * 8) / rt500k) : 0;	/* of reaggr */
			mpdudur = aggnum[i] ? (dur / aggnum[i]) : 0;	/* of reaggr */
			WL_PRINT(("rt500k 0x%04x invrt500k 0x%04x dur 0x%04x->0x%04x"
				" mpdudur 0x%04x->0x%04x\n",
				rt500k, wlc_read_shmx(wlc, (ctxptr+56+0) * 2),
				wlc_read_shmx(wlc, (ctxptr+56+1) * 2), dur,
				wlc_read_shmx(wlc, (ctxptr+56+2) * 2), mpdudur));
		}
#endif /* WL_MU_TX */
		WL_PRINT(("\n"));
	}
	for (i = 0; i < maxaggnum; i++) {
		uint16 mpdulen[4] = {0, 0, 0, 0};
#if defined(DONGLEBUILD)
		/* reduce print log for donlge not to overflow logging buffer */
		if ((i & 0x3)) {
			continue;
		}
#endif /* defined(DONGLEBUILD) */
		for (k = 0; k < max_txc; k++) {
			if (i < aggnum[k]) {
				W_REG(osh, &regs->u.d11acregs.psm_reg_mux, (0xfff0 + k));
				while (R_REG(osh, &regs->u.d11acregs.psm_reg_mux) != (0xfff0 + k));
				W_REG(osh, &regs->u.d11acregs.AQMAggRptr, i);
				while (R_REG(osh, &regs->u.d11acregs.AQMAggRptr) != i);
				mpdulen[k] = R_REG(osh, &regs->u.d11acregs.AQMMpduLen);
			}
		}
		WL_PRINT(("[%-2d]\t%-4d\t%-4d\t%-4d\t%-4d\n",
			i, mpdulen[0], mpdulen[1], mpdulen[2], mpdulen[3]));
	}
	W_REG(osh, &regs->u.d11acregs.psm_reg_mux, orig_psm_reg_mux);
#endif /* BCMDBG|| BCMDBG_DUMP || WLTEST */
}
#endif /* NOT_YET */

void
wlc_dump_ucode_fatal(wlc_info_t *wlc, uint reason)
{
	wlc_pub_t *pub;
	osl_t *osh;
	d11regs_t *regs;
	uint32 phydebug, psmdebug;
	uint16 val16[4];
	uint32 val32[4];
	int i, k;
	volatile uint8 *paddr;
	/* two lists: one common, one corerev specific */
	d11print_list_t *plist[3];
	int lsize[3];

	/* common list */
	d11print_list_t cmlist[] = {
		{"ifsstat", 0x690},
		{"ifsstat1", 0x698},
		{"txectl", 0x500},
		{"txestat", 0x50e},
		{"txestat2", 0x51c},
		{"rxestat1", 0x41a},
		{"rxestat2", 0x41c},
		{"rcv_frmcnt", 0x40a},
		{"rxe_rxcnt", 0x418},
		{"wepctl", 0x7c0},
	};
	/* reg specific to corerev < 40 */
	d11print_list_t list_lt40[] = {
		{"xmtfifordy", 0x546},
		{"pcmctl", 0x7d0},
		{"pcmstat", 0x7d2},
	};
	/* reg specific to corerev >= 40 */
	d11print_list_t list_ge40[] = {
		{"aqmfifordy", 0x838},
		{"wepstat", 0x7c2},
		{"wep_ivloc", 0x7c4},
		{"wep_psdulen", 0x7c6},
	};
	d11print_list_t list_ge64[] = {
		{"aqmfifordyL", 0xbbc},
		{"aqmfifordyH", 0xbbe},
		{"rxe_errval", 0x448},
		{"rxe_errmask", 0x44a},
		{"rxe_status3", 0x44c},
	};
	d11print_list_t nphyreg[] = {
		{"pktproc", 0x183}, /* repeat four times */
		{"pktproc", 0x183},
		{"pktproc", 0x183},
		{"pktproc", 0x183}
	};
	d11print_list_t acphyreg[] = {
		{"pktproc", 0x1f0}, /* repeat four times */
		{"pktproc", 0x1f0},
		{"pktproc", 0x1f0},
		{"pktproc", 0x1f0}
	};
	d11print_list_t acphy2reg[] = {
		{"TxFifoStatus0", 0x60d},
		{"TxFifoStatus1", 0x80d},
		{"TxFifoStatus2", 0xa0d},
		{"TxFifoStatus3", 0xc0d}
	};

	pub = wlc->pub;
	osh = wlc->osh;
	regs = wlc->regs;
	BCM_REFERENCE(val16);
	BCM_REFERENCE(val32);

	WL_PRINT(("wl%d: reason = ", pub->unit));
	switch (reason) {
		case 1:
			WL_PRINT(("psm watchdog"));
			break;
		case 2:
			WL_PRINT(("mac suspend failure"));
			break;
		case 3:
			WL_PRINT(("fail to wake up"));
			break;
		case 4:
			WL_PRINT(("txfifo susp/flush"));
			break;
		case 5:
			WL_PRINT(("psmx watchdog"));
			break;
		case 6:
			WL_PRINT(("txstuck"));
			break;
		default:
			WL_PRINT(("any"));

	}
	WL_PRINT((" at %d seconds. corerev %d ", pub->now, pub->corerev));
	if (!wlc->clk) {
		WL_PRINT(("%s: no clk\n", __FUNCTION__));
		return;
	}
	WL_PRINT(("ucode revision %d.%d features 0x%04x\n",
		wlc_read_shm(wlc, M_BOM_REV_MAJOR), wlc_read_shm(wlc, M_BOM_REV_MINOR),
		wlc_read_shm(wlc, M_UCODE_FEATURES)));

	psmdebug = R_REG(osh, &regs->psmdebug);
	phydebug = R_REG(osh, &regs->phydebug);
	val32[0] = R_REG(osh, &regs->maccontrol);
	val32[1] = R_REG(osh, &regs->maccommand);
	val16[0] = R_REG(osh, &regs->psm_brc);
	val16[1] = R_REG(osh, &regs->psm_brc_1);
	val16[2] = wlc_read_shm(wlc, M_UCODE_DBGST);

	wlc_mac_event(wlc, WLC_E_PSM_WATCHDOG, NULL, psmdebug, phydebug, val16[0], NULL, 0);
	WL_PRINT(("psmdebug 0x%08x phydebug 0x%x macctl 0x%x maccmd 0x%x\n"
		 "psm_brc 0x%04x psm_brc_1 0x%04x M_UCODE_DBGST 0x%x\n",
		 psmdebug, phydebug, val32[0], val32[1], val16[0], val16[1], val16[2]));

	if ((wlc->macdbg->log_done) != 0) {
		WL_PRINT(("%s: log_done %#x\n", __FUNCTION__, wlc->macdbg->log_done));
		return;
	}

	paddr = (volatile uint8*)(&regs->biststatus) - 0xC;
	plist[0] = cmlist;
	lsize[0] = ARRAYSIZE(cmlist);
	lsize[1] = 0;
	lsize[2] = 0;
	if (D11REV_LT(pub->corerev, 40)) {
		plist[1] = list_lt40;
		lsize[1] = ARRAYSIZE(list_lt40);
	} else {
		plist[1] = list_ge40;
		lsize[1] = ARRAYSIZE(list_ge40);
	}
	if (D11REV_GE(pub->corerev, 64)) {
		plist[2] = list_ge64;
		lsize[2] = ARRAYSIZE(list_ge64);
	}
	for (i = 0; i < 3; i ++) {
		for (k = 0; k < lsize[i];  k ++) {
			val16[0] = R_REG(osh, (volatile uint16*)(paddr + plist[i][k].addr));
			WL_PRINT(("%-12s 0x%-4x ", plist[i][k].name, val16[0]));
			if ((k % 4) == 3)
				WL_PRINT(("\n"));
		}
		if (k % 4) {
			WL_PRINT(("\n"));
		}
	}

#if defined(WLAMPDU_MAC)
	if (AMPDU_MAC_ENAB(pub))
		wlc_dump_aggfifo(wlc, NULL);
#endif /* WLAMPDU_MAC */

	WL_PRINT(("PC :\n"));
	for (k = 0; k < 64; k += 4) {
		val32[0] = R_REG(osh, &regs->psmdebug);
		val32[1] = R_REG(osh, &regs->psmdebug);
		val32[2] = R_REG(osh, &regs->psmdebug);
		val32[3] = R_REG(osh, &regs->psmdebug);
		WL_PRINT(("0x%-8x 0x%-8x 0x%-8x 0x%-8x\n",
			val32[0], val32[1], val32[2], val32[3]));
	}
	/* phyreg */
	if (phydebug > 0) {
		WL_PRINT(("phydebug :\n"));
		for (k = 0; k < 32; k += 4) {
			val32[0] = R_REG(osh, &regs->phydebug);
			val32[1] = R_REG(osh, &regs->phydebug);
			val32[2] = R_REG(osh, &regs->phydebug);
			val32[3] = R_REG(osh, &regs->phydebug);
			WL_PRINT(("0x%-8x 0x%-8x 0x%-8x 0x%-8x\n",
				val32[0], val32[1], val32[2], val32[3]));
		}
		if (WLCISACPHY(wlc->band)) {
			plist[0] = acphyreg;
			lsize[0] = ARRAYSIZE(acphyreg);
		} else if (WLCISHTPHY(wlc->band) || WLCISNPHY(wlc->band)) {
			plist[0] = nphyreg;
			lsize[0] = ARRAYSIZE(nphyreg);
		} else {
			lsize[0] = 0;
		}
		lsize[1] = 0;
		lsize[2] = 0;

		/* acphy2 debug info */
		if (WLCISACPHY(wlc->band) && (ACREV_GE(wlc->band->phyrev, 32))) {
			plist[1] = acphy2reg;
			lsize[1] = ARRAYSIZE(acphy2reg);
		}
		for (i = 0; i < 3; i ++) {
			for (k = 0; k < lsize[i]; k++) {
				W_REG(osh, &regs->phyregaddr, plist[i][k].addr);
				val16[0] = R_REG(osh, &regs->phyregdata);
				WL_PRINT(("%-12s 0x%-4x ", plist[i][k].name, val16[0]));
				if ((k % 4) == 3)
					WL_PRINT(("\n"));
			}
			if (k % 4) {
				WL_PRINT(("\n"));
			}
		}

		/* acphy2 debug info2 */
		if (WLCISACPHY(wlc->band) && (ACREV_GE(wlc->band->phyrev, 32))) {
			uint8 gpiosel = 2;
			d11print_list_t acphy2_gpioregs[] = {
				{"gpiosel", 0x394},
				{"gpiolo", 0x12},
				{"gpiohi", 0x13}
			};

			for (i = 0; i < gpiosel; i ++) {
				/* select gpio */
				W_REG(osh, &regs->phyregaddr, acphy2_gpioregs[0].addr);
				W_REG(osh, &regs->phyregdata, i);

				for (k = 0; k < ARRAYSIZE(acphy2_gpioregs); k++) {
					W_REG(osh, &regs->phyregaddr, acphy2_gpioregs[k].addr);
					val16[0] = R_REG(osh, &regs->phyregdata);
					WL_PRINT(("%-12s 0x%-4x ",
						acphy2_gpioregs[k].name, val16[0]));
				}
				WL_PRINT(("\n"));
			}
		}
	} /* phydebug */

	WL_PRINT(("psm stack_status : 0x%x\n", R_REG(osh, &regs->psm_srs_status)));
	WL_PRINT(("psm stack_entries:\n"));
	for (k = 0; k < 8; k++) {
		W_REG(osh, &regs->psm_srs_ptr, k);
		val16[0] = R_REG(osh, &regs->psm_srs_entry);
		WL_PRINT(("0x%04x\n", val16[0]));
	}

	k = (reason == PSM_FATAL_SUSP) ? 1 : 0;
#ifdef WAR4360_UCODE
	k = (reason == PSM_FATAL_WAKE) ? 1 : k;
#endif // endif
	if (k) {
		wlc_suspend_mac_debug(wlc, phydebug);
	}

#ifdef NOT_YET
	wlc_macdbg_dump_aggr(wlc);
#endif /* NOT_YET */

#if defined(WLC_HOSTPMAC)
	/* Mac dump for full dongle driver */
	/* triggering DHD to dump d11core */
	if (!D11REV_IS(wlc->pub->corerev, 65) || reason != PSM_FATAL_TXSTUCK) {
		wlc_mac_event(wlc, WLC_E_MACDBG, NULL, WLC_E_STATUS_SUCCESS,
			WLC_E_MACDBG_REGALL, 0, &reason, sizeof(reason));
	}
#endif /* WLC_HOSTPMAC */

	wlc->macdbg->log_done |= (1 << reason);
}

#if defined(WL_PSMX)
void
wlc_bmac_psmx_errors(wlc_info_t *wlc)
{
	d11regs_t *regs = wlc->regs;
	uint32 macintstatus;
	osl_t *osh;

	osh = wlc->osh;

	macintstatus = R_REG(osh, &regs->macintstatus_x);

	/* PSMx watchdog fired */
	if (macintstatus & MI_GP0) {
		W_REG(osh, &regs->macintmask_x, MI_GP0);
		wlc_dump_psmx_fatal(wlc, PSMX_FATAL_PSMWD);
		ASSERT(!"PSMx Watchdog");

		WLCNTINCR(wlc->pub->_cnt->psmxwds);
#ifndef BCMNODOWN
		if (get_g_assert_type() == 3) {
			/* bring the driver down without resetting hardware */
			wlc->down_override = TRUE;
			wlc->psm_watchdog_debug = TRUE;
			wlc_out(wlc);

			/* halt the PSM */
			wlc_bmac_mctrl(wlc->hw, MCTL_PSM_RUN, 0);
			wlc->psm_watchdog_debug = FALSE;
		} else
#endif /* BCMNODOWN */
		{
#if !defined(WLC_HOSTPMAC)
			/* big hammer */
			wlc_fatal_error(wlc);
#endif // endif
		}
	}
}

#ifdef VASIP_HW_SUPPORT
void
wlc_dump_vasip_fatal(wlc_info_t *wlc)
{
#if defined(SVMP_ACCESS_VIA_PHYTBL) && defined(WLC_LOW)
	wlc_hw_info_t *wlc_hw = wlc->hw;
	uint16 i, m, n, c[4], s[4];
	uint32 address[18] = {0x20000, 0x20060, 0x20080, 0x21e10, 0x20300, 0x20700, 0x20b00,
		0x20bff, 0xc000, 0xe000, 0x10000, 0x12000, 0x14000, 0x16000, 0x18000, 0x1a000,
		0x1c000, 0x1e000};
	uint16 size[18] = {16, 1, 1, 16, 16, 16, 16, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	uint16 counter[93];
	uint16 report_idx[10] = {64, 65, 0, 1, 2, 3, 4, 5, 6, 7};

	/*
	 * name           = address, index in array
	 * counter        = 0x20000, 0;
	 * fix_mcs        = 0x20060, 16;
	 * error_code     = 0x20080, 17;
	 * mcs            = 0x21e10, 18;
	 * m2v0           = 0x20300, 34;
	 * m2v1           = 0x20700, 50;
	 * v2m            = 0x20b00, 66;
	 * transfer_done  = 0x20bff, 82;
	 * header0        = 0xc000, 83;
	 * header1        = 0xe000, 84;
	...
	*/

#endif /* SVMP_ACCESS_VIA_PHYTBL && WLC_LOW */

	if (!wlc->clk) {
		WL_PRINT(("%s: no clk\n", __FUNCTION__));
		return;
	}

#if defined(SVMP_ACCESS_VIA_PHYTBL) && defined(WLC_LOW)
	WL_PRINT(("VASIP watchdog is triggered. VASIP code revision %d.%d \n", wlc->vasip_major,
			wlc->vasip_minor));

	n = 0;
	for (i = 0; i < 18; i++) {
		for (m = 0; m < size[i]; m++) {
			counter[n++] = phy_misc_vasip_svmp_read((phy_info_t *)wlc_hw->band->pi,
				address[i]+m);
		}
	}

	for (i = 0; i < 4; i++) {
		s[i] = ((counter[18+i] & 0xf0) >> 4) + 1;
		c[i] = counter[18+i] & 0xf;
	}

	WL_PRINT(("Received Interrupts:\n"
		"      bfr_module_done:0x%x     bfe_module_done:0x%x     bfe_imp_done:0x%x\n"
			"      m2v_transfer_done:0x%x   v2m_transfder_done:0x%x\n"
			"Fixed MCS: %d\n"
			"Number of users: %x\n"
			"Last steered MCS:\n"
			"      user0:c%ds%d;   user1:c%ds%d;   user2:c%ds%d;   user3:c%ds%d\n"
			"Last precoder SNR:\n"
			"      user0:0x%x;   user1:0x%x;   user2:0x%x;   user3:0x%x\n"
			"Error code is 0x%x\n"
			"v2m transfer is done: %d\n",
			counter[2], counter[3], counter[4], counter[8], counter[9], counter[16],
			counter[27], c[0], s[0], c[1], s[1], c[2], s[2], c[3], s[3], counter[23]/4,
			counter[24]/4, counter[25]/4, counter[26]/4, counter[17], counter[82]));

	WL_PRINT(("m2v0 message:\n"));
	for (i = 0; i < 4; i++)
		WL_PRINT(("0x%04x 0x%04x 0x%04x 0x%04x\n", counter[34+i*4], counter[35+4*i],
				counter[36+4*i], counter[37+4*i]));

	WL_PRINT(("m2v1 message:\n"));
	for (i = 0; i < 4; i++)
		WL_PRINT(("0x%04x 0x%04x 0x%04x 0x%04x\n", counter[50+i*4], counter[51+4*i],
				counter[52+4*i], counter[53+4*i]));

	WL_PRINT(("v2m message:\n"));
	for (i = 0; i < 4; i++)
		WL_PRINT(("0x%04x 0x%04x 0x%04x 0x%04x\n", counter[66+i*4], counter[67+4*i],
				counter[68+4*i], counter[69+4*i]));

	for (i = 0; i < 10; i++)
		WL_PRINT(("index %d, addr 0x%04x, header: 0x%04x\n", report_idx[i],
				address[8+i], counter[i+83]));
#endif /* SVMP_ACCESS_VIA_PHYTBL && WLC_LOW */
}
#endif /* VASIP_HW_SUPPORT */

void
wlc_dump_psmx_fatal(wlc_info_t *wlc, uint reason)
{
	d11regs_t *regs;
	osl_t *osh;
	wlc_pub_t *pub;
	uint32 val32[4];
	uint16 val16;
	uint16 k;
	const char reason_str[][20] = {
		"any failure",
		"watchdog",
		"suspend failure",
		"txstuck",
	};

	osh = wlc->osh;
	pub = wlc->pub;
	regs = wlc->regs;

	k = (reason >= PSMX_FATAL_LAST) ? PSMX_FATAL_ANY : reason;
	WL_PRINT(("wl%d: PSMx dump at %d seconds. corerev %d reason:%s ",
		pub->unit, pub->now, pub->corerev, reason_str[k]));

	if (!wlc->clk) {
		WL_PRINT(("%s: no clk\n", __FUNCTION__));
		return;
	}
	WL_PRINT(("ucode revision %d.%d features 0x%04x\n",
		wlc_read_shmx(wlc, M_BOM_REV_MAJOR), wlc_read_shmx(wlc, M_BOM_REV_MINOR),
		wlc_read_shm(wlc, M_UCODE_FEATURES)));

	wlc_mac_event(wlc, WLC_E_PSM_WATCHDOG, NULL, R_REG(osh, &regs->psmdebug_x),
		0, wlc_read_macregx(wlc, 0x490), NULL, 0);
	WL_PRINT(("psmxdebug 0x%08x macctl 0x%x maccmd 0x%x\n"
		 "psm_brc 0x%04x psm_brc_1 0x%04x MX_UCODE_DBGST 0x%x\n",
		  R_REG(osh, &regs->psmdebug_x),
		  R_REG(osh, &regs->maccontrol_x),
		  R_REG(osh, &regs->maccommand_x),
		  wlc_read_macregx(wlc, 0x490),
		  wlc_read_macregx(wlc, 0x4d8),
		  wlc_read_shmx(wlc, MX_UCODE_DBGST)));

	if ((wlc->macdbg->log_done & (1 << PSM_FATAL_PSMXWD)) != 0) {
		WL_PRINT(("%s: log_done %#x\n", __FUNCTION__, wlc->macdbg->log_done));
		return;
	}
	wlc->macdbg->log_done |= (1 << PSM_FATAL_PSMXWD);
	WL_PRINT(("PC (psmdebug_x):\n"));
	for (k = 0; k < 64; k += 4) {
		val32[0] = R_REG(osh, &regs->psmdebug_x);
		val32[1] = R_REG(osh, &regs->psmdebug_x);
		val32[2] = R_REG(osh, &regs->psmdebug_x);
		val32[3] = R_REG(osh, &regs->psmdebug_x);
		WL_PRINT(("0x%-8x 0x%-8x 0x%-8x 0x%-8x\n",
			val32[0], val32[1], val32[2], val32[3]));
	}
	WL_PRINT(("PC (psmdebug):\n"));
	for (k = 0; k < 8; k += 4) {
		val32[0] = R_REG(osh, &regs->psmdebug);
		val32[1] = R_REG(osh, &regs->psmdebug);
		val32[2] = R_REG(osh, &regs->psmdebug);
		val32[3] = R_REG(osh, &regs->psmdebug);
		WL_PRINT(("0x%-8x 0x%-8x 0x%-8x 0x%-8x\n",
			val32[0], val32[1], val32[2], val32[3]));
	}
	val16 = wlc_read_macregx(wlc, 0x4d0); /* psm_srs_status */
	WL_PRINT(("psmx stack_status : 0x%x\n", val16));
	WL_PRINT(("psmx stack_entries:\n"));
	for (k = 0; k < 8; k++) {
		wlc_write_macregx(wlc, 0x4d2, k); /* psm_srs_ptr */
		val16 = wlc_read_macregx(wlc, 0x4d4); /* psm_srs_entry */
		WL_PRINT(("0x%04x\n", val16));
	}

#ifdef VASIP_HW_SUPPORT
	/* bit5 of txe_vasip_intsts indicates vasip watchdog is triggered */
	val16 = wlc_read_macregx(wlc, 0x870);
	WL_PRINT(("txe_vasip_intsts %#x\n", val16));
	if (val16 & (1 << 5))
		wlc_dump_vasip_fatal(wlc);
#endif // endif

#if defined(WLC_HOSTPMAC)
	/* Mac dump for full dongle driver */
	/* triggering DHD to dump d11core */
	wlc_mac_event(wlc, WLC_E_MACDBG, NULL, WLC_E_STATUS_SUCCESS,
		WLC_E_MACDBG_REGALL, 0, &reason, sizeof(reason));
#endif /* WLC_HOSTPMAC */
}

#if WL_MACDBG
/* dump ucode trace capture in shm for psmx */
/* wl dump sctpl [-u shmx ] */
static int
wlc_dump_sctpl_shmx(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	d11regs_t *regs;
	osl_t *osh;
	uint i;
	int gpio_sel;
	uint16 start, end = 0, curr, len;

	if (D11REV_LT(wlc->pub->corerev, 64)) {
		WL_ERROR(("wl%d: %s only supported for corerev >= 64\n",
			wlc->pub->unit, __FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	if (!wlc->clk) {
		WL_ERROR(("wl%d: %s: clock must be on\n", wlc->pub->unit, __FUNCTION__));
		return BCME_NOCLK;
	}

	regs = wlc->regs;
	osh = wlc->osh;

	gpio_sel = R_REG(osh, &regs->maccontrol1);
	start = wlc_read_shmx(wlc, MX_UTRACEX_SPTR) & (~0x3);
	end = (wlc_read_shmx(wlc, MX_UTRACEX_EPTR) + 2) & (~0x3);
	/* stop sample capture for convenience */
	wlc_write_shmx(wlc, MX_UTRACEX_SPTR, start);
	wlc_bmac_copyfrom_objmem(wlc->hw, SX_UPTR << 2,
		&curr, sizeof(curr), OBJADDR_SCRX_SEL);
	len = (end - start);

	if (b) {
		bcm_bprintf(b, "Capture mode: maccontrol1 0x%02x scpctl 0x00\n",
			gpio_sel);
		bcm_bprintf(b, "Start/stop/cur 0x%04x 0x%04x 0x%04x byt_offset 0x%04x entries %u\n",
			start, end, curr, (curr - start), len >> 2);
		bcm_bprintf(b, "offset: low high\n");
	} else {
		printf("Capture mode: maccontrol1 0x%02x scpctl 0x00\n", gpio_sel);
		printf("Start/stop/cur 0x%04x 0x%04x 0x%04x byt_offset 0x%04x entries %u\n",
			start, end, curr, (curr - start), len >> 2);
		printf("offset: low high\n");
	}

	for (i = 0; i < (uint)len; i += 4) {
		uint16 low16, hi16;
		low16 = wlc_read_shmx(wlc, start + i);
		hi16 = wlc_read_shmx(wlc, (start + i + 2));
		if (b)
			bcm_bprintf(b, "%04X: %04X %04X\n", i, low16, hi16);
		else
			printf("%04X: %04X %04X\n", i, low16, hi16);
	}

	return BCME_OK;
}

static int
wlc_dump_macx(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	int cnt = 0;
	if (D11REV_LT(wlc->pub->corerev, 64)) {
		WL_ERROR(("wl%d: %s only supported for corerev >= 64\n",
			wlc->pub->unit, __FUNCTION__));
		return BCME_UNSUPPORTED;
	}
	if (!wlc->clk) {
		WL_ERROR(("wl%d: %s: clock must be on\n", wlc->pub->unit, __FUNCTION__));
		return BCME_NOCLK;
	}
	ASSERT(wlc->macdbg->pd11regs_x != NULL && wlc->macdbg->d11regsx_sz > 0);
	cnt = wlc_pd11regs_bylist(wlc, wlc->macdbg->pd11regs_x,
		wlc->macdbg->d11regsx_sz, 0, b);
	return cnt;
}

static int
wlc_dump_shmemx(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	uint i, k = 0;
	uint16 val, addr, end;

	static const shmem_list_t shmem_list[] = {
		{0, 4*1024},
	};

	if (D11REV_LT(wlc->pub->corerev, 64)) {
		WL_ERROR(("wl%d: %s only supported for corerev >= 64\n",
			wlc->pub->unit, __FUNCTION__));
		return BCME_UNSUPPORTED;
	}
	if (!wlc->clk) {
		WL_ERROR(("wl%d: %s: clock must be on\n", wlc->pub->unit, __FUNCTION__));
		return BCME_NOCLK;
	}

	for (i = 0; i < ARRAYSIZE(shmem_list); i++) {
		end = shmem_list[i].start + 2 * shmem_list[i].cnt;
		for (addr = shmem_list[i].start; addr < end; addr += 2) {
			val = wlc_read_shmx(wlc, addr);
			if (b) {
				bcm_bprintf(b, "%d shmx 0x%03x = 0x%04x\n",
					k++, addr, val);
			} else {
				printf("%d shmx 0x%03x = 0x%04x\n",
					k++, addr, val);
			}
		}
	}

	return 0;
}
#endif /* WL_MACDBG */
#endif /* WL_PSMX */

void
wlc_dump_mac_fatal(wlc_info_t *wlc, uint reason)
{
	wlc_dump_ucode_fatal(wlc, reason);
#ifdef WL_PSMX
	if (MU_TX_ENAB(wlc)) {
		wlc_dump_psmx_fatal(wlc, (reason == PSM_FATAL_TXSTUCK) ?
			PSMX_FATAL_TXSTUCK : PSMX_FATAL_ANY);
	}
#endif // endif
	/* big hammer */
	if (D11REV_IS(wlc->pub->corerev, 65) && reason == PSM_FATAL_TXSTUCK) {
		wlc_fatal_error(wlc);
	}
}

#if WL_MACDBG
/* Depending CoreRev, some reggisters are 2B, some 4B */
static void
wlc_macdbg_smpl_capture_optnreg(wlc_info_t *wlc, uint8 *reg_addr,
	uint32 *val, int reg_size, bool set)
{
	/* Caller checks the clock */
	ASSERT(wlc->clk);

	switch (reg_size) {
		case 2:
			if (set) {
				/* Lo Addr */
				W_REG(wlc->osh, (uint16*)(reg_addr), *(uint16 *)val);
				/* Hi Addr */
				W_REG(wlc->osh, (uint16 *)(reg_addr + reg_size),
					(uint16)(*val >> 16));
			} else {
				*val = R_REG(wlc->osh, (uint16 *)reg_addr) |
					(R_REG(wlc->osh, (uint16 *)(reg_addr + reg_size)) << 16);
			}
			break;
		case 4:
			if (set)
				W_REG(wlc->osh, (uint32 *)reg_addr, *val);
			else
				*val = R_REG(wlc->osh, (uint32 *)reg_addr);
			break;
		default:
			WL_ERROR(("wl%d: %s Wrong register size.\n", wlc->pub->unit, __FUNCTION__));
			ASSERT(0);

	}
}

static int
wlc_macdbg_smpl_capture_optns(wlc_info_t *wlc, wl_maccapture_params_t *params, bool set)
{
	d11regs_t *regs = wlc->regs;
	uint8 opt_bmp = params->optn_bmp;
	uint32 *mask = NULL, *val = NULL;
	uint16 smpl_ctrl = 0, d11_regvalue = 0;
	volatile uint16 *smpl_ctrl_reg;
	int smpl_capture_rev;
	int i = 0, reg_incr, optn_shft = 0;
	int reg_size, addr_size;
	uint8 *reg_block = NULL, **current_reg = NULL;

	//volatile uint16 *sc_optn_regs_ge50[SC_NUM_OPTNREGS_GE50] = {
	volatile uint16 *smpl_capture_optn_regs_ge50[] = {
			&(regs->u.d11acregs.TXE_SCT_MASK_L), &(regs->u.d11acregs.TXE_SCT_MASK_H),
			&(regs->u.d11acregs.TXE_SCT_VAL_L), &(regs->u.d11acregs.TXE_SCT_VAL_H),
			&(regs->u.d11acregs.TXE_SCS_MASK_L), &(regs->u.d11acregs.TXE_SCS_MASK_H),
			&(regs->u.d11acregs.TXE_SCX_MASK_L), &(regs->u.d11acregs.TXE_SCX_MASK_H),
			&(regs->u.d11acregs.TXE_SCM_MASK_L), &(regs->u.d11acregs.TXE_SCM_MASK_H),
			&(regs->u.d11acregs.TXE_SCM_VAL_L), &(regs->u.d11acregs.TXE_SCM_VAL_H)};

	//volatile uint32 *sc_optn_regs_lt50[SC_NUM_OPTNREGS_LT50] = {
	volatile uint32 *smpl_capture_optn_regs_lt50[] = {
			&regs->dbgstrtrigmask,	/* Trigger Mask */
			&regs->dbgstrtrig,	/* Trigger Value */
			&regs->dbgstrmask};	/* Store Mask */
						/* No Transition Mask,
						 * No Match Mask
						 * No Match Value
						 */

	uint16 smpl_ctrl_en[2][SC_NUM_OPTNS_GE50] = {
		{SC_TRIG_EN, SC_STORE_EN, SC_TRANS_EN, SC_MATCH_EN},
		{PHYCTL_SC_TRIG_EN, PHYCTL_SC_STR_EN, PHYCTL_SC_TRANS_EN, SC_OPTN_LT50NA}};

	/* Core Specific Inits */
	if (D11REV_GE(wlc->pub->corerev, 50)) {
		smpl_capture_rev = SMPL_CAPTURE_GE50;
		smpl_ctrl_reg = &regs->u.d11acregs.SampleCollectPlayCtrl;
		reg_incr = 2; /* Per option, there are 2 regs. >= CoreRev 50 */
		reg_block = (void *)smpl_capture_optn_regs_ge50;
		addr_size = sizeof(smpl_capture_optn_regs_ge50[0]);
		reg_size = sizeof(*smpl_capture_optn_regs_ge50[0]);
	} else {
		ASSERT(!(opt_bmp & ((1 << WL_MACCAPT_TRANS) | (1 << WL_MACCAPT_MATCH))));
		smpl_capture_rev = SMPL_CAPTURE_LT50;
		smpl_ctrl_reg = &regs->psm_phy_hdr_param;
		reg_incr = 1; /* Per option, there is 1 reg. < CoreRev 50 */
		reg_block = (void *)smpl_capture_optn_regs_lt50;
		addr_size = sizeof(smpl_capture_optn_regs_lt50[0]);
		reg_size = sizeof(*smpl_capture_optn_regs_lt50[0]);
	}

	while (opt_bmp) {
		current_reg = (uint8 **)(reg_block + i * addr_size);
		if (opt_bmp & 1) {
			mask = (uint32*)(&params->tr_mask + (i / reg_incr));

			if (set) {
				/* Set the mask of the corresponding mode */
				wlc_macdbg_smpl_capture_optnreg(wlc, *current_reg,
				mask, reg_size, set);
			} else {
				/* Get the mask */
				wlc_macdbg_smpl_capture_optnreg(wlc, *current_reg,
				mask, reg_size, FALSE);
			}

			/* Some options have corresponding values */
			if ((optn_shft == WL_MACCAPT_TRIG) ||
			(optn_shft == WL_MACCAPT_MATCH)) {
				/* Value right after mask */
				val = (mask + 1);
				current_reg = (uint8 **)(reg_block + (i + reg_incr) * addr_size);

				if (set)
					wlc_macdbg_smpl_capture_optnreg(wlc, *current_reg, val,
					reg_size, set);
				else
					wlc_macdbg_smpl_capture_optnreg(wlc, *current_reg, val,
					reg_size, FALSE);
			}

			/* Populate soft-reg. value for SampleCollectPlayCtrl */
			smpl_ctrl |= (smpl_ctrl_en[smpl_capture_rev][optn_shft]);
		}
		opt_bmp >>= 1;

		if ((optn_shft == WL_MACCAPT_TRIG) ||
		(optn_shft == WL_MACCAPT_MATCH)) {
			/* WL_MACCAPT_TRIG and WL_MACCAPT_MATCH
			 * have "val" fields after mask so need to
			 * account for that
			 */
			i += reg_incr;
		}
		i += reg_incr;
		optn_shft++;
	}

	/* Enable the SC Options */
	if (smpl_ctrl && set) {
		d11_regvalue = R_REG(wlc->osh, smpl_ctrl_reg);
		W_REG(wlc->osh, smpl_ctrl_reg, d11_regvalue | smpl_ctrl);
	}

	return BCME_OK;
}

/* wlc_macdbg_smpl_capture_set initializes MAC sample capture.
 * If UP, also starts the sample capture.
 * Otherwise, sample capture will be started upon wl up
 */
static int
wlc_macdbg_smpl_capture_set(wlc_info_t *wlc, wl_maccapture_params_t *params)
{
	wlc_macdbg_info_t *macdbg = wlc->macdbg;
	wl_maccapture_params_t *cur_params = NULL;
	d11regs_t *regs = wlc->regs;
	uint8 gpio_sel = params->gpio_sel;
	uint16 d11_regvalue = 0;

	if (macdbg->smpl_info == NULL) {
		if ((macdbg->smpl_info = MALLOCZ(wlc->osh,
				sizeof(wl_maccapture_params_t))) == NULL) {
			WL_ERROR(("wl%d: %s: smp_info memory alloc. failed\n",
				wlc->pub->unit, __FUNCTION__));
			return BCME_NOMEM;
		}
	}

	cur_params = (wl_maccapture_params_t *)macdbg->smpl_info;

	if (D11REV_LT(wlc->pub->corerev, 26)) {
		WL_ERROR(("wl%d: %s only supported for corerev >=26\n",
			wlc->pub->unit, __FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	if (!wlc->clk) {
		memcpy((void *)cur_params, (void *)params, sizeof(*params));
		wlc->macdbg->smpl_ctrl |= SC_STRT; /* Set the start bit of the soft register */
		WL_ERROR(("%s: MAC Sample Capture params. saved. "
			"Will  start upon wl up\n", __FUNCTION__));
		return BCME_OK;
	}

	if (params->cmd == WL_MACCAPT_STOP) {
		if (D11REV_GE(wlc->pub->corerev, 50)) {
			/* Clear Start bit */
			d11_regvalue =	R_REG(wlc->osh, &regs->u.d11acregs.SampleCollectPlayCtrl);
			W_REG(wlc->osh, &regs->u.d11acregs.SampleCollectPlayCtrl,
				d11_regvalue & ~SC_STRT);
		} else {
			/* Clear start bit */
			AND_REG(wlc->osh, &regs->psm_phy_hdr_param, ~PHYCTL_SC_STRT);
		}

		/* Clear the start bit of the soft register */
		wlc->macdbg->smpl_ctrl &= ~SC_STRT;
		return BCME_OK;
	}

	if (params->la_mode) {
		if (D11REV_LT(wlc->pub->corerev, 50)) {
			/* Assign all GPIOs to other cores than ChipCommon */
			si_ccreg(wlc->pub->sih, CC_GPIOCTRL, ~0, 0xFFFFFFFF);

			/* Disable PHY GPIOs */
			wlc_phy_gpiosel_disable(WLC_PI(wlc));
		}
		/* Enable GPIO based on mask */
		W_REG(wlc->osh, &regs->psm_gpio_oe, params->s_mask);
	}

	/* GPIO Output Selection */
	d11_regvalue = (gpio_sel << MCTL1_GPIOSEL_SHIFT)
			& MCTL1_GPIOSEL_MASK; /* GPIO_SEL is 6bits */
	d11_regvalue = (R_REG(wlc->osh, &regs->maccontrol1) & ~MCTL1_GPIOSEL_MASK) | d11_regvalue;
	W_REG(wlc->osh, &regs->maccontrol1, d11_regvalue);

	/* Sample Collect Start & Stop Ptr */
	W_REG(wlc->osh, &regs->u.d11acregs.SampleCollectStartPtr, params->start_ptr);
	W_REG(wlc->osh, &regs->u.d11acregs.SampleCollectStopPtr, params->stop_ptr);

	/* Enable Sample Capture Clock */
	d11_regvalue =  R_REG(wlc->osh, &regs->psm_corectlsts);
	W_REG(wlc->osh, &regs->psm_corectlsts, d11_regvalue | PSM_CORE_CTL_SS);

	/* Setting options bitmap */
	/* Some options are not supported for CoreRev < 50 */
	if (D11REV_LT(wlc->pub->corerev, 50)) {
		if (params->x_mask) {
			WL_ERROR(("%s: Transition Mask not supported below CoreRev 50\n",
				__FUNCTION__));
			params->optn_bmp &= ~(1 << WL_MACCAPT_TRANS);
		}

		if ((params->m_val) || (params->m_mask)) {
			WL_ERROR(("%s: Match Mode not supported below CoreRev 50\n",
				__FUNCTION__));
			params->optn_bmp &= ~(1 << WL_MACCAPT_MATCH);
		}
	}

	/* Sample Capture Options */
	if (params->optn_bmp)
		wlc_macdbg_smpl_capture_optns(wlc, params, TRUE);

	/* Start MAC Sample Capture */
	if (D11REV_GE(wlc->pub->corerev, 50)) {
		/* Sample Capture Source and Start bits */
		d11_regvalue =	R_REG(wlc->osh, &regs->u.d11acregs.SampleCollectPlayCtrl);
		d11_regvalue |= (SC_SRC_MAC << SC_SRC_SHIFT) | SC_STRT;
		W_REG(wlc->osh, &regs->u.d11acregs.SampleCollectPlayCtrl, d11_regvalue);

		WL_TRACE(("%s: SampleCollectPlayCtrl(0xb2e): "
			"0x%x, GPIO Out Sel:0x%02x\n", __FUNCTION__,
			R_REG(wlc->osh, &regs->u.d11acregs.SampleCollectPlayCtrl),
			R_REG(wlc->osh, &regs->maccontrol1)));

	} else {
		d11_regvalue =  R_REG(wlc->osh, &regs->psm_phy_hdr_param);
		d11_regvalue |= (PHYCTL_PHYCLKEN | PHYCTL_SC_STRT | PHYCTL_SC_SRC_LB |
				PHYCTL_SC_TRANS_EN);
		W_REG(wlc->osh, &regs->psm_phy_hdr_param, d11_regvalue);

		WL_TRACE(("%s: PHY_CTL(0x492): 0x%x, GPIO Out Sel:0x%02x\n", __FUNCTION__,
				R_REG(wlc->osh, &regs->psm_phy_hdr_param),
				R_REG(wlc->osh, &regs->maccontrol1)));

	}

	/* Store config info */
	memcpy((void *)cur_params, params, sizeof(*params));
	wlc->macdbg->smpl_ctrl |= SC_STRT; /* Set the start bit of the soft register */

	return BCME_OK;
}

static int
wlc_macdbg_smpl_capture_get(wlc_info_t *wlc, char *outbuf, uint outlen)
{
	d11regs_t *regs = wlc->regs;
	struct bcmstrbuf bstr;
	wlc_macdbg_info_t *macdbg = wlc->macdbg;
	wl_maccapture_params_t *cur_params = NULL;
	wl_maccapture_params_t get_params;
	uint32 cur_ptr = 0; /* Sample Capture cur_ptr */

	/* HW is turned off so don't try to access it */
	if (wlc->pub->hw_off || DEVICEREMOVED(wlc))
		return BCME_RADIOOFF;

	if (D11REV_LT(wlc->pub->corerev, 26)) {
		WL_ERROR(("wl%d: %s only supported for corerev >=26\n",
			wlc->pub->unit, __FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	bcm_binit(&bstr, outbuf, outlen);

	memset((void *)outbuf, 0, outlen);
	memset((void *)&get_params, 0, sizeof(get_params));

	/* Check clock */
	if (!wlc->clk) {
		if (macdbg->smpl_info == NULL) {
			WL_ERROR(("wl%d: %s: MAC Sample Capture NOT set.\n",
				wlc->pub->unit, __FUNCTION__));

			return BCME_NOTREADY;
		}
		bcm_bprintf(&bstr, "MAC Capture\nNo Clock so returning saved state:\n");
		cur_params = (wl_maccapture_params_t *)macdbg->smpl_info;
		memcpy(&get_params, (void *)cur_params, sizeof(wl_maccapture_params_t));
		/* If no clock, return 0 for cur_ptr */
		goto print_values;
	}

	/* GPIO Out Sel */
	get_params.gpio_sel = (R_REG(wlc->osh, &regs->maccontrol1) & MCTL1_GPIOSEL_MASK)
		>> MCTL1_GPIOSEL_SHIFT;

	get_params.start_ptr = R_REG(wlc->osh, &regs->u.d11acregs.SampleCollectStartPtr);
	get_params.stop_ptr = R_REG(wlc->osh, &regs->u.d11acregs.SampleCollectStopPtr);

	/* Read the options from option regs */
	/* Supported options */
	if (D11REV_GE(wlc->pub->corerev, 50)) {
		get_params.optn_bmp = (1 << WL_MACCAPT_TRIG) | (1 << WL_MACCAPT_STORE) |
			(1 << WL_MACCAPT_TRANS) | (1 << WL_MACCAPT_MATCH);
	} else
		get_params.optn_bmp = (1 << WL_MACCAPT_TRIG) | (1 << WL_MACCAPT_STORE);

	wlc_macdbg_smpl_capture_optns(wlc, &get_params, FALSE);

	/* Cur. Ptr */
	cur_ptr = R_REG(wlc->osh, &regs->u.d11acregs.SampleCollectCurPtr);

	bcm_bprintf(&bstr, "MAC Capture Registers:\n");

print_values:
	bcm_bprintf(&bstr, "GPIO Sel:0x%x Logic Analyzer Mode On:%d\n"
			"start_ptr:0x%x, stop_ptr:0x%x, cur_ptr:0x%x\n"
			"store mask:0x%x, match mask:0x%x, match val:0x%x\n"
			"trans mask:0x%x, trig mask:0x%x, trig val:0x%x\n"
			"state:0x%02x\n", get_params.gpio_sel, get_params.la_mode,
			get_params.start_ptr, get_params.stop_ptr, cur_ptr,
			get_params.s_mask, get_params.m_mask, get_params.m_val,
			get_params.x_mask, get_params.tr_mask, get_params.tr_val,
			get_params.cmd);

	return BCME_OK;
}
#endif /* WL_MACDBG */

#ifdef RXDMA_STUCK_WAR
bool
wlc_macdbg_is_rxdma_stuck(wlc_info_t *wlc, uint rxfifo)
{
	hnddma_t *dmah;
	uint16 rxin_curr;

	dmah = WLC_HW_DI(wlc, rxfifo);

	if (!dma_get_curr_rxin(dmah, &rxin_curr) ||
			(wlc->pub->rxin_last != rxin_curr)) {
		wlc->pub->rxin_last = rxin_curr;
		return FALSE;
	}

	wlc->pub->rxin_last = rxin_curr;
	return TRUE;
}
#endif // endif
