/*
 * Proxy STA
 *
 * This module implements Proxy STA as well as the Wireless Repeater
 * features.
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
 * $Id: wlc_psta.c 769205 2018-11-06 10:34:56Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WirelessProxyRepeater]
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <proto/802.3.h>
#include <proto/vlan.h>
#include <proto/bcmip.h>
#include <proto/bcmicmp.h>
#include <proto/bcmarp.h>
#include <proto/bcmudp.h>
#include <proto/bcmdhcp.h>

#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_assoc.h>
#include <wlc_scb.h>
#include <wlc_addrmatch.h>
#include <bcmwpa.h>

#include <wlc_psta.h>
#include <bcm_psta.h>

#include <proto/802.11.h>
#include <wlc_ie_misc_hndlrs.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_reg.h>
typedef struct saved_psta {
	struct saved_psta	*next;
	struct ether_addr	ds_ea;
} saved_psta_t;

/* PSTA private info structure */
struct wlc_psta_info {
	wlc_info_t	*wlc;		/* Pointer to wlc info struct */
	wlc_pub_t	*pub;		/* Pointer to wlc public data */
	saved_psta_t	*saved_clients; /* list of saved psta clients (during primary roams) */
	uint8		rcmap[CEIL(RCMTA_SIZE, NBBY)];
	uint8		rcmta_idx;	/* RCMTA index */
	int32		inactivity;	/* Inactivity counter configured */
	bool		mrpt;		/* Support multi repeating */
	int32		cfgh;		/* PSTA bsscfg cubby handle */
#ifdef WLCNT
	uint32		pstatxdhcpc;	/* DHCP client to server frames in tx dir */
	uint32		pstarxdhcpc;	/* DHCP client to server frames in rx dir */
	uint32		pstatxdhcps;	/* DHCP server to client frames in tx dir */
	uint32		pstarxdhcps;	/* DHCP server to client frames in rx dir */
	uint32		pstatxdhcpc6;	/* DHCP client to server frames in tx dir */
	uint32		pstarxdhcpc6;	/* DHCP client to server frames in rx dir */
	uint32		pstatxdhcps6;	/* DHCP server to client frames in tx dir */
	uint32		pstarxdhcps6;	/* DHCP server to client frames in rx dir */
	uint32		pstadupdetect;	/* Duplicate alias detected count */
#endif // endif
};

/* Proxy STA Association info */
struct wlc_psa {
	bool		primary;	/* True if psta instance is primary */
	bool		ds_sta;		/* Indicates if it is wired/wireless client */
	wlc_bsscfg_t	*pcfg;		/* Pointer to primary psta bsscfg */
	uint32		inactivity;	/* Num of sec the psta link is inactive */
	uint8		rcmta_idx;	/* RCMTA index */
	bool		allow_join;	/* Flag set to TRUE after config is done */
	struct ether_addr ds_ea;	/* MAC address of the client we are proxying */
#ifdef WLCNT
	uint32		txucast;	/* Frames sent on psta assoc */
	uint32		txnoassoc;	/* Frames dropped due to no assoc */
	uint32		rxucast;	/* Frames received on psta assoc */
	uint32		txbcmc;		/* BCAST/MCAST frames sent on all psta assoc */
	uint32		rxbcmc;		/* BCAST/MCAST frames recvd on all psta assoc */
#endif /* WLCNT */
	int8		saved_psta_mode; /* Save PSTA mode if it is overridden by dwds */
};

/* Proxy STA bsscfg cubby */
typedef struct psta_bsscfg_cubby {
	wlc_psa_t	*psa;		/* PSTA assoc info */
} psta_bsscfg_cubby_t;

#define PSTA_BSSCFG_CUBBY(ps, cfg) ((psta_bsscfg_cubby_t *)BSSCFG_CUBBY((cfg), (ps)->cfgh))
#define PSTA_IS_PRIMARY(cfg)	((cfg) == wlc_bsscfg_primary((cfg)->wlc))

#define	PSTA_IS_DS_STA(s)	((s)->ds_sta)
#define	PSTA_JOIN_ALLOWED(s)	((s)->allow_join)

#define PSTA_MAX_INACT		600	/* PSTA inactivity timeout in seconds */

#define EA_CMP(e1, e2) \
	(!((((uint16 *)(e1))[0] ^ ((uint16 *)(e2))[0]) | \
	   (((uint16 *)(e1))[1] ^ ((uint16 *)(e2))[1]) | \
	   (((uint16 *)(e1))[2] ^ ((uint16 *)(e2))[2])))

#define	PSTA_SET_ALIAS(psta, psa, bc, ea) \
do { \
	ASSERT(EA_CMP((psa)->ds_ea.octet, (ea))); \
	*((struct ether_addr *)(ea)) = (bc)->cur_etheraddr; \
} while (0)

#define	PSTA_CLR_ALIAS(psta, psa, bc, ea) \
do { \
	ASSERT(EA_CMP((bc)->cur_etheraddr.octet, (ea))); \
	*((struct ether_addr *)(ea)) = (psa)->ds_ea; \
} while (0)

#define	PSTA_IS_ALIAS(psta, bc, ea)	EA_CMP((bc)->cur_etheraddr.octet, (ea))

/* IOVar table */
enum {
	IOV_PSTA,
	IOV_IS_PSTA_IF,
	IOV_PSTA_INACT,
	IOV_PSTA_ALLOW_JOIN,
	IOV_PSTA_MRPT
};

static const bcm_iovar_t psta_iovars[] = {
	{"psta", IOV_PSTA, (IOVF_SET_DOWN), IOVT_INT8, 0 },
	{"psta_if", IOV_IS_PSTA_IF, (IOVF_GET_UP), IOVT_BOOL, 0 },
	{"psta_inact", IOV_PSTA_INACT, (0), IOVT_UINT32, 0 },
	{"psta_allow_join", IOV_PSTA_ALLOW_JOIN, (0), IOVT_UINT32, 0 },
	{"psta_mrpt", IOV_PSTA_MRPT, (IOVF_SET_DOWN), IOVT_BOOL, 0 },
	{NULL, 0, 0, 0, 0 }
};

/* Forward declaration of local functions */
static int32 wlc_psta_rcmta_alloc(wlc_psta_info_t *psta, uint8 *idx);
static void wlc_psta_rcmta_free(wlc_psta_info_t *psta, uint8 idx);
static void wlc_psta_watchdog(void *arg);
static int32 wlc_psta_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
                              const char *name, void *p, uint plen, void *a,
                              int32 alen, int vsize, struct wlc_if *wlcif);
static int32 wlc_psta_icmp6_proc(wlc_psta_info_t *psta, wlc_bsscfg_t *cfg, void **p,
                                 uint8 *ih, uint8 *icmph, uint16 icmplen, uint8 **shost,
                                 bool tx, bool is_bcmc);
static int32 wlc_psta_dhcp_proc(void *psta_cb, void *cfg_cb, void **p,
	uint8 *uh, uint8 *dhcph, uint16 dhcplen, uint16 port,
	uint8 **shost, bool tx, bool is_bcmc);
static int32 wlc_psta_arp_proc(wlc_psta_info_t *psta, wlc_bsscfg_t *cfg, void **p,
                               uint8 *ah, uint8 **shost, bool tx, bool is_bcast);
static int32 wlc_psta_proto_proc(wlc_psta_info_t *psta, wlc_bsscfg_t *pcfg, void **p,
                                 uint8 *eh, wlc_bsscfg_t **cfg, uint8 **shost, bool tx);
static int32 wlc_psta_create(wlc_psta_info_t *psta, wlc_info_t *wlc, struct ether_addr *ea,
                             wlc_bsscfg_t *pcfg);
static wlc_bsscfg_t *wlc_psta_find_by_ds_ea(wlc_psta_info_t *psta, uint8 *mac);
static struct scb *wlc_psta_client_is_ds_sta(wlc_psta_info_t *psta,
                                             struct ether_addr *mac);
static void *wlc_psta_pkt_alloc_copy(wlc_pub_t *pub, void *p);
static void wlc_psta_alias_create(wlc_psta_info_t *psta, wlc_bsscfg_t *pcfg,
                                  struct ether_addr *shost);
static int wlc_psta_bsscfg_init(void *context, wlc_bsscfg_t *cfg);
static void wlc_psta_bsscfg_deinit(void *context, wlc_bsscfg_t *cfg);
static int32 wlc_psta_up(void *ctx);
static uint wlc_psta_brcm_psta_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_psta_brcm_psta_ie(void *ctx, wlc_iem_build_data_t *data);
static uint8 *wlc_psta_write_psta_ie(wlc_psta_info_t *psta, uint8 *cp, int buflen);
static void wlc_psta_deinit(wlc_psta_info_t *psta, wlc_bsscfg_t *pcfg);
static void wlc_psta_mode_enable(wlc_psta_info_t *psta, bool enable);
static void wlc_psta_mode_save(wlc_psta_info_t *psta, wlc_bsscfg_t *cfg, struct ether_addr *addr);
static void wlc_psta_mode_restore(wlc_psta_info_t *psta, wlc_bsscfg_t *cfg);
static void wlc_psta_scb_state_upd(bcm_notif_client_data ctx, bcm_notif_server_data scb_data);

static uint
wlc_psta_brcm_psta_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_bsscfg_t *bsscfg = data->cfg;

	if (BSSCFG_PSTA(bsscfg))
		return MEMBER_OF_BRCM_PROP_IE_HDRLEN;

	return 0;
}

static int
wlc_psta_brcm_psta_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_psta_info_t *psta = (wlc_psta_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;

	if (BSSCFG_PSTA(bsscfg))
		wlc_psta_write_psta_ie(psta, data->buf, data->buf_len);

	return BCME_OK;
}

static uint8 *
wlc_psta_write_psta_ie(wlc_psta_info_t *psta, uint8 *cp, int buflen)
{
	wlc_info_t *wlc = psta->wlc;
	wlc_bsscfg_t *bsscfg = wlc_bsscfg_primary(wlc);
	member_of_brcm_prop_ie_t *member_of_brcm_prop_ie;
	/* perform buffer length check. */
	/* if not big enough, return buffer untouched */
	BUFLEN_CHECK_AND_RETURN((MEMBER_OF_BRCM_PROP_IE_HDRLEN), buflen, cp);

	member_of_brcm_prop_ie =  (member_of_brcm_prop_ie_t *)cp;
	member_of_brcm_prop_ie->id = DOT11_MNG_PROPR_ID;
	member_of_brcm_prop_ie->len = MEMBER_OF_BRCM_PROP_IE_LEN;
	bcopy(BRCM_PROP_OUI, &member_of_brcm_prop_ie->oui[0], DOT11_OUI_LEN);
	member_of_brcm_prop_ie->type = MEMBER_OF_BRCM_PROP_IE_TYPE;
	bcopy(&bsscfg->cur_etheraddr, &member_of_brcm_prop_ie->ea, sizeof(struct ether_addr));
	cp += (MEMBER_OF_BRCM_PROP_IE_HDRLEN);

	return cp;
}

/*
 * Initialize psta private context. It returns a pointer to the
 * psta private context if succeeded. Otherwise it returns NULL.
 */
wlc_psta_info_t *
BCMATTACHFN(wlc_psta_attach)(wlc_info_t *wlc)
{
	wlc_psta_info_t *psta;
	uint16 arqfstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);

	/* Allocate psta private info struct */
	psta = MALLOCZ(wlc->pub->osh, sizeof(wlc_psta_info_t));
	if (!psta) {
		WL_ERROR(("wl%d: %s: MALLOCZ failed, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->pub->osh)));
		return NULL;
	}

	/* Init psta private info struct */
	psta->wlc = wlc;
	psta->pub = wlc->pub;
	psta->inactivity = PSTA_MAX_INACT;
	psta->mrpt = FALSE;

	/* Register module */
	if (wlc_module_register(wlc->pub, psta_iovars, "psta", psta, wlc_psta_doiovar,
	                    wlc_psta_watchdog, wlc_psta_up, NULL)) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		MFREE(wlc->pub->osh, psta, sizeof(wlc_psta_info_t));
		return NULL;
	}

#ifdef BCMDBG
	wlc_dump_register(wlc->pub, "psta", (dump_fn_t)wlc_psta_dump, (void *)psta);
#endif // endif

	/* Reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((psta->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(psta_bsscfg_cubby_t),
		wlc_psta_bsscfg_init, wlc_psta_bsscfg_deinit,
		NULL, (void *)psta)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve failed\n",
			wlc->pub->unit, __FUNCTION__));
		wlc_module_unregister(psta->pub, "psta", psta);
		MFREE(wlc->pub->osh, psta, sizeof(wlc_psta_info_t));
		return NULL;
	}

	WL_PSTA(("wl%d: psta attach done\n", psta->pub->unit));

	/* assocreq/reassocreq */
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_BRCM_PSTA,
	      wlc_psta_brcm_psta_ie_len, wlc_psta_brcm_psta_ie, psta) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn() failed, brcm ie\n",
		          wlc->pub->unit, __FUNCTION__));
		wlc_module_unregister(psta->pub, "psta", psta);
		MFREE(wlc->pub->osh, psta, sizeof(wlc_psta_info_t));
		return NULL;
	}

	if (wlc_scb_state_upd_register(wlc,
		wlc_psta_scb_state_upd, (void*)psta) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_scb_state_upd_register failed\n",
			wlc->pub->unit, __FUNCTION__));
		return NULL;
	}

	return psta;
}

static int
wlc_psta_bsscfg_init(void *context, wlc_bsscfg_t *cfg)
{
	wlc_psta_info_t *psta = (wlc_psta_info_t *)context;
	psta_bsscfg_cubby_t *psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);
	wlc_pub_t *pub = psta->pub;

	psta_cfg->psa = MALLOCZ(pub->osh, sizeof(wlc_psa_t));
	if (psta_cfg->psa == NULL)
		return BCME_NOMEM;

	return BCME_OK;
}

static void
wlc_psta_bsscfg_deinit(void *context, wlc_bsscfg_t *cfg)
{
	wlc_psta_info_t *psta = (wlc_psta_info_t *)context;
	psta_bsscfg_cubby_t *psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);

	if (psta_cfg->psa != NULL) {
		MFREE(psta->pub->osh, psta_cfg->psa, sizeof(wlc_psa_t));
	}
}

static int32
wlc_psta_up(void *context)
{
	wlc_psta_info_t *psta = (wlc_psta_info_t *)context;

	if (PSTA_ENAB(psta->pub)) {
		/* Initialize and enable PSTA */
		wlc_psta_init(psta, psta->wlc->cfg);
		wlc_mhf(psta->wlc, MHF4, MHF4_PROXY_STA, MHF4_PROXY_STA, WLC_BAND_AUTO);
	} else
		/* Disable PSTA */
		wlc_mhf(psta->wlc, MHF4, MHF4_PROXY_STA, 0, WLC_BAND_AUTO);

	return BCME_OK;
}

/* Alloc rcmta slot. First half of the table is assigned to TAs and the
 * second half to RAs.
 */
static int32
wlc_psta_rcmta_alloc(wlc_psta_info_t *psta, uint8 *idx)
{
	uint8 i, start_off;

	start_off = PSTA_RA_STRT_INDX;

	/* Find the first available slot and assign it */
	for (i = 0; i < PSTA_MAX_ASSOC(psta->wlc); i++) {
		if (isset(&psta->rcmap[0], start_off + i))
			continue;

		/* Confirm amt_idx by keymgmt as well */
		if (wlc_keymgmt_amt_idx_isset(psta->wlc->keymgmt, start_off + i)) {
			continue;
		}

		*idx = start_off + i;
		setbit(&psta->rcmap[0], *idx);
		WL_PSTA(("wl%d: Allocing RCMTA index %d\n", psta->pub->unit, *idx));
		ASSERT(*idx < RCMTA_SIZE);
		return BCME_OK;
	}

	return BCME_NORESOURCE;
}

/* Free rcmta slot */
static void
wlc_psta_rcmta_free(wlc_psta_info_t *psta, uint8 idx)
{
	if (idx >= RCMTA_SIZE)
		return;

	clrbit(&psta->rcmap[0], idx);

	WL_PSTA(("wl%d: Freed RCMTA index %d\n", psta->pub->unit, idx));

	return;
}

static void
wlc_psta_mode_enable(wlc_psta_info_t *psta, bool enable)
{
	WL_PSTA(("wl%d: %s: enable=%d\n", psta->pub->unit, __FUNCTION__, enable));

	if (enable == TRUE) {
		/* Initialize and enable PSTA */
		wlc_psta_init(psta, psta->wlc->cfg);
		/* Enable PSTA in ucode */
		wlc_mhf(psta->wlc, MHF4, MHF4_PROXY_STA, MHF4_PROXY_STA, WLC_BAND_AUTO);
	} else {
		wlc_psta_deinit(psta, psta->wlc->cfg);
		/* Disable PSTA in ucode */
		wlc_mhf(psta->wlc, MHF4, MHF4_PROXY_STA, 0, WLC_BAND_AUTO);
	}
}

/* Cleanup psta private context */
void
BCMATTACHFN(wlc_psta_detach)(wlc_psta_info_t *psta)
{
	if (!psta)
		return;

	/* Clear primary bss rmcta entry */
	wlc_psta_rcmta_free(psta, psta->rcmta_idx);

	/* Clear scb state update */
	wlc_scb_state_upd_unregister(psta->wlc, wlc_psta_scb_state_upd, (void*) psta);
	/* Clear psta rcmta entries */
	wlc_module_unregister(psta->pub, "psta", psta);

	/* Release eventual saved clients */
	while (psta->saved_clients) {
		saved_psta_t *psa;

		psa = psta->saved_clients;
		psta->saved_clients = psa->next;
		MFREE(psta->pub->osh, psa, sizeof(*psa));
	}

	MFREE(psta->pub->osh, psta, sizeof(wlc_psta_info_t));
}

uint8
wlc_psta_rcmta_idx(wlc_psta_info_t *psta, const wlc_bsscfg_t *cfg)
{
	psta_bsscfg_cubby_t *psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);
	return psta_cfg->psa->rcmta_idx;
}

void
wlc_psta_deauth_client(wlc_psta_info_t *psta, struct ether_addr *addr)
{
	wlc_bsscfg_t *cfg;
	struct scb *scb;
	struct ether_addr psta_ha;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */

	WL_PSTA(("wl%d: Rcvd deauth from client %s\n",
	         psta->pub->unit, bcm_ether_ntoa(addr, eabuf)));

	/* See if the mac address belongs to a downstream client */
	scb = wlc_psta_client_is_ds_sta(psta, addr);
	if (scb == NULL)
		return;

	if (SCB_PS(scb)) {
		WL_ERROR(("wl%d: Proxy STA %s ignoring bsscfg_free while in PS\n",
			psta->pub->unit, bcm_ether_ntoa(addr, eabuf)));
		return;
	}

	WL_PSTA(("wl%d: Sending deauth for ds client %s\n",
	         psta->pub->unit, bcm_ether_ntoa(addr, eabuf)));

	/* Find the psta using the client's original address */
	psta_ha = *addr;
	cfg = wlc_psta_find_by_ds_ea(psta, (uint8 *)&psta_ha);
	if (cfg == NULL) {
		WL_ERROR(("wl%d: Proxy STA %s link is already gone !!??\n",
		          psta->pub->unit, bcm_ether_ntoa(&psta_ha, eabuf)));
		return;
	}

	/* Send deauth to our AP with unspecified reason code */
	scb = wlc_scbfind(psta->wlc, cfg, &cfg->BSSID);
	(void)wlc_senddeauth(psta->wlc, cfg, scb, &cfg->BSSID, &cfg->BSSID,
	                     &cfg->cur_etheraddr, DOT11_RC_UNSPECIFIED);

	/* Cleanup the proxy client state */
	wlc_disassociate_client(cfg, FALSE);

	/* Down the bss */
	wlc_bsscfg_disable(psta->wlc, cfg);
	wlc_bsscfg_free(psta->wlc, cfg);

	WL_PSTA(("wl%d: Deauth for client %s complete\n",
	         psta->pub->unit, bcm_ether_ntoa(addr, eabuf)));
}

static void
wlc_psta_mode_save(wlc_psta_info_t *psta, wlc_bsscfg_t *cfg, struct ether_addr *addr)
{
	wlc_info_t *wlc = psta->wlc;
	psta_bsscfg_cubby_t *psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);
	struct scb *scb;

	if (PSTA_ENAB(wlc->pub) && (cfg == wlc_bsscfg_primary(wlc))) {
		scb = addr ? wlc_scbfind(wlc, cfg, addr) : NULL;
		if ((scb != NULL) && SCB_DWDS_CAP(scb)) {
			/* If we are reassociating to DWDS capable UAP
			 * and have previously created secondary associations
			 * than clearing all of them.
			 * SCB_DWDS_CAP being TRUE implies that DWDS is enabled.
			 */
			/* deinit psta mode */
			wlc_psta_mode_enable(psta, FALSE);
			/* Save psta mode */
			psta_cfg->psa->saved_psta_mode = wlc->pub->_psta;
			/* disable psta mode */
			wlc->pub->_psta = PSTA_MODE_DISABLED;
		}
	}
}

static void
wlc_psta_mode_restore(wlc_psta_info_t *psta, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = psta->wlc;
	psta_bsscfg_cubby_t *psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);

	if (DWDS_ENAB(cfg) && !PSTA_ENAB(wlc->pub) && BSSCFG_STA(cfg) &&
		(psta_cfg->psa->saved_psta_mode != PSTA_MODE_DISABLED)) {
		/* init psta mode */
		wlc_psta_mode_enable(wlc->psta, TRUE);
		/* enable psta mode */
		wlc->pub->_psta = psta_cfg->psa->saved_psta_mode;
	}
}

void
wlc_psta_mode_update(wlc_psta_info_t *psta, wlc_bsscfg_t *cfg, struct ether_addr *addr,
	psta_mode_update_action_t action)
{
	switch (action) {
	case PSTA_MODE_UPDATE_ACTION_SAVE:
		wlc_psta_mode_save(psta, cfg, addr);
		break;
	case PSTA_MODE_UPDATE_ACTION_RESTORE:
		wlc_psta_mode_restore(psta, cfg);
		break;
	default:
		break;
	}
}

/*
 * Create the primary proxy sta association. Primry proxy sta connection is
 * initiated using the hardware address of the interface.
 */
int32
wlc_psta_init(wlc_psta_info_t *psta, wlc_bsscfg_t *pcfg)
{
	int32 i;
	psta_bsscfg_cubby_t *psta_cfg;
#if defined(DPSTA) || defined(DWDS)
	wl_dpsta_intf_event_t dpsta_prim_event;
#endif // endif

	if (!BSSCFG_STA(pcfg))
		return BCME_OK;

	WL_PSTA(("wl%d: %s: PSTA init\n", psta->pub->unit, __FUNCTION__));

	/* Clear RCMTA entries */
	for (i = 0; i < RCMTA_SIZE; i++)
		wlc_clear_addrmatch(psta->wlc, i);

	/* Set the the primary bss mac */
	psta->rcmta_idx = PSTA_RA_PRIM_INDX;
	wlc_set_addrmatch(psta->wlc, psta->rcmta_idx, &pcfg->cur_etheraddr,
		AMT_ATTR_VALID | AMT_ATTR_A1 | AMT_ATTR_A2);

	/* Mark as primary */
	psta_cfg = PSTA_BSSCFG_CUBBY(psta, pcfg);
	psta_cfg->psa->primary = TRUE;
	psta_cfg->psa->rcmta_idx = PSTA_RA_PRIM_INDX;

	/* Initialize the mac address of primary */
	bcopy(&pcfg->cur_etheraddr, &psta_cfg->psa->ds_ea, ETHER_ADDR_LEN);

	WLCNTSET(psta->pstatxdhcpc, 0);
	WLCNTSET(psta->pstarxdhcpc, 0);
	WLCNTSET(psta->pstatxdhcps, 0);
	WLCNTSET(psta->pstarxdhcps, 0);
	WLCNTSET(psta->pstatxdhcpc6, 0);
	WLCNTSET(psta->pstarxdhcpc6, 0);
	WLCNTSET(psta->pstatxdhcps6, 0);
	WLCNTSET(psta->pstarxdhcps6, 0);
	WLCNTSET(psta->pstadupdetect, 0);

#if defined(DPSTA) || defined(DWDS)

	/* Send WLC_E_DWDS_INTF_IND event for dpsta psta register */
	if (pcfg) {
		dpsta_prim_event.intf_type = WL_INTF_PSTA;
		wlc_bss_mac_event(pcfg->wlc, pcfg, WLC_E_DPSTA_INTF_IND, NULL, 0, 0, 0,
				&dpsta_prim_event, sizeof(wl_dpsta_intf_event_t));
	}
#endif // endif

	return BCME_OK;
}

static void
wlc_psta_deinit(wlc_psta_info_t *psta, wlc_bsscfg_t *pcfg)
{
	psta_bsscfg_cubby_t *psta_cfg;

	if (!BSSCFG_STA(pcfg))
		return;

	WL_PSTA(("wl%d: %s: PSTA de-init\n", psta->pub->unit, __FUNCTION__));

	/* Disabling all PSTAs */
	wlc_psta_disable_all(psta);

	/* Clear primary bss rmcta entry */
	wlc_psta_rcmta_free(psta, psta->rcmta_idx);

	/* Un-mark as a primary */
	psta_cfg = PSTA_BSSCFG_CUBBY(psta, pcfg);
	psta_cfg->psa->primary = FALSE;
	psta_cfg->psa->rcmta_idx = 0;

	/* Clean the mac address of primary */
	bcopy(&ether_null, &psta_cfg->psa->ds_ea, ETHER_ADDR_LEN);

	/* Release eventual saved clients */
	while (psta->saved_clients) {
		saved_psta_t *psa;

		psa = psta->saved_clients;
		psta->saved_clients = psa->next;
		MFREE(psta->pub->osh, psa, sizeof(*psa));
	}
}

static INLINE bool
wlc_psta_is_datapath_available(wlc_psta_info_t *psta)
{
	wlc_bsscfg_t *primary_bss = psta->wlc->cfg;

	/*
	 * Return TRUE if the datapth is available for operation, ie, not roaming or some such.
	 *
	 * This is a bit of a pain as there does not seem to be some simple way of checking.
	 * When roaming, the BSS still pretends to be associated (for 9 seconds), we can thus
	 * check whether a roam is in progress. However, between roams the roam state is idle,
	 * so we need some other way - such as whether we are receiving beacons.
	 */

	return (wlc_bss_connected(primary_bss) &&		/* The BSS is associated, */
		(primary_bss->roam->bcns_lost == FALSE) &&	/* beacons are not lost, */
		(primary_bss->assoc->state == AS_IDLE));	/* and we are not roaming. */

}

static INLINE bool
wlc_psta_prim_associated(wlc_psta_info_t *psta)
{
	wlc_bsscfg_t *primary_bss = psta->wlc->cfg;

	/*
	 * Return TRUE if the primary interface is operational, ie, not roaming or some such.
	 *
	 * This is a bit of a pain as there does not seem to be some simple way of checking.
	 * When roaming, the BSS still pretends to be associated (for 9 seconds), we can thus
	 * check whether a roam is in progress. However, between roams the roam state is idle,
	 * so we need some other way - such as whether we are receiving beacons.
	 */

	return (wlc_psta_is_datapath_available(psta) &&		/* The datapath is available, */
		(primary_bss->roam->time_since_bcn == 0));	/* and beacons are received. */

}

static void
wlc_psta_watchdog(void *arg)
{
	wlc_psta_info_t *psta = arg;
	int32 idx;
	wlc_bsscfg_t *cfg;
	struct scb *scb;
	psta_bsscfg_cubby_t *psta_cfg;
	int primary_is_up;

	if (!PSTA_ENAB(psta->pub))
		return;

	primary_is_up = wlc_psta_prim_associated(psta);

	/* Cleanup the active proxy stas, and those that never came up too */
	FOREACH_PSTA(psta->wlc, idx, cfg) {
		psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);
		if (!psta->inactivity ||
		    psta_cfg->psa->inactivity++ <= psta->inactivity) {
			/* Re-trigger PSTA clients that are not up yet */
			if (!cfg->associated && primary_is_up &&
				PSTA_JOIN_ALLOWED(psta_cfg->psa) &&
				(cfg->assoc->state == AS_IDLE)) {
				WL_PSTA(("wl%d: Kickstarting PSTA "MACF" ...\n",
					psta->pub->unit,
					ETHER_TO_MACF(cfg->cur_etheraddr)));

					wlc_psta_create(psta, psta->wlc, &cfg->cur_etheraddr,
						psta_cfg->psa->pcfg);
			}
			continue;
		}
		/* Cleanup the proxy client state */
		wlc_disassociate_client(cfg, FALSE);

		/* Send deauth to our AP with unspecified reason code */
		scb = wlc_scbfind(psta->wlc, cfg, &cfg->BSSID);
		(void)wlc_senddeauth(psta->wlc, cfg, scb, &cfg->BSSID, &cfg->BSSID,
		                     &cfg->cur_etheraddr, DOT11_RC_UNSPECIFIED);

		wlc_bsscfg_disable(psta->wlc, cfg);
		wlc_bsscfg_free(psta->wlc, cfg);
	}
}

/* Handling psta related iovars */
static int32
wlc_psta_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
                 void *p, uint plen, void *a, int32 alen, int vsize, struct wlc_if *wlcif)
{
	wlc_psta_info_t *psta = hdl;
	wlc_info_t *wlc;
	int32 err = 0;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 *ret_int_ptr;
	wlc_bsscfg_t *cfg;
	bool bool_val;
	psta_bsscfg_cubby_t *psta_cfg;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	wlc = psta->wlc;

	/* Convenience int and bool vals for first 8 bytes of buffer */
	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	if (plen >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)p + sizeof(int_val)), &int_val2, sizeof(int_val));

	/* Convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)a;

	/* bool conversion to avoid duplication below */
	bool_val = int_val != 0;

	cfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(cfg != NULL);

	switch (actionid) {
	case IOV_GVAL(IOV_PSTA):
		*ret_int_ptr = (int32)wlc->pub->_psta;
		break;

	case IOV_SVAL(IOV_PSTA):
		if ((int_val < PSTA_MODE_DISABLED) || (int_val > PSTA_MODE_REPEATER))
			err = BCME_RANGE;
		else {
			wlc->pub->_psta = (int8)int_val;
#ifdef DWDS
			/* if the new psta mode is assigned than make sure previously saved
			 * psta mode is cleared in order to not change it back to the previous
			 * mode in wlc_process_brcm_ie function.
			 */
			if (DWDS_ENAB(cfg)) {
				psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);
				psta_cfg->psa->saved_psta_mode = PSTA_MODE_DISABLED;
			}
#endif // endif
		}
		break;

	case IOV_GVAL(IOV_IS_PSTA_IF):
		*ret_int_ptr = (int32)cfg->_psta;
		break;

	case IOV_SVAL(IOV_PSTA_INACT):
		psta->inactivity = int_val;
		break;

	case IOV_GVAL(IOV_PSTA_INACT):
		*ret_int_ptr = psta->inactivity;
		break;

	case IOV_SVAL(IOV_PSTA_ALLOW_JOIN):
		/* Only valid for dynamic proxy sta instances. It doesn't apply for
		 * sta and ap because they are configured before hand.
		 */
		if (!BSSCFG_PSTA(cfg))
			err = BCME_BADARG;

		if (int_val) {
			WL_PSTA(("wl%d: Allow the join for %s now\n",
			         psta->pub->unit,
			         bcm_ether_ntoa(&cfg->cur_etheraddr, eabuf)));
		}

		psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);
		psta_cfg->psa->allow_join = int_val;

		/* Disable bss when joins are denied */
		if (!PSTA_JOIN_ALLOWED(psta_cfg->psa))
			wlc_bsscfg_disable(wlc, cfg);
		break;

	case IOV_GVAL(IOV_PSTA_ALLOW_JOIN):
		psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);
		*ret_int_ptr = psta_cfg->psa->allow_join;
		break;

	case IOV_SVAL(IOV_PSTA_MRPT):
		/* Silently ignore duplicate requests */
		if (psta->mrpt == bool_val)
			break;

		/* Multi repeater mode supported only when we are repeater */
		if (PSTA_IS_PROXY(wlc)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		psta->mrpt = bool_val;
		break;

	case IOV_GVAL(IOV_PSTA_MRPT):
		*ret_int_ptr = (int32)psta->mrpt;
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* Called when the bss is disabled to clear the psta state of the bss */
void
wlc_psta_disable(wlc_psta_info_t *psta, wlc_bsscfg_t *cfg)
{
	psta_bsscfg_cubby_t *psta_cfg;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	/* Nothing to do if the bss is not sta */
	if (!BSSCFG_STA(cfg))
		return;

	/* Turn roam off */
	cfg->roam->off = FALSE;

	/* Clear the rmcta entry of psta bss mac */
	psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);
	wlc_clear_addrmatch(psta->wlc, psta_cfg->psa->rcmta_idx);

	WL_PSTA(("wl%d: Freeing RCMTA idx %d for etheraddr %s\n", psta->pub->unit,
	         psta_cfg->psa->rcmta_idx, bcm_ether_ntoa(&cfg->cur_etheraddr, eabuf)));

	/* Free the rcmta index */
	wlc_psta_rcmta_free(psta, psta_cfg->psa->rcmta_idx);
	psta_cfg->psa->rcmta_idx = 0;
	psta_cfg->psa->ds_sta = FALSE;
	psta_cfg->psa->allow_join = FALSE;
	psta_cfg->psa->pcfg = NULL;

	/* Clear the counters */
	psta_cfg->psa->txucast = 0;
	psta_cfg->psa->txnoassoc = 0;
	psta_cfg->psa->rxucast = 0;
	psta_cfg->psa->txbcmc = 0;
	psta_cfg->psa->rxbcmc = 0;

	return;
}

void
wlc_psta_disable_all(wlc_psta_info_t *psta)
{
	int32 idx;
	wlc_bsscfg_t *bsscfg;

	WL_PSTA(("wl%d: Disabling all PSTAs\n", psta->pub->unit));

	/* Cleanup all active proxy stas */
	FOREACH_PSTA(psta->wlc, idx, bsscfg) {
		wlc_psta_disable(psta, bsscfg);
		wlc_bsscfg_free(psta->wlc, bsscfg);
	}

	return;
}

static int32
wlc_psta_create(wlc_psta_info_t *psta, wlc_info_t *wlc, struct ether_addr *ea,
                wlc_bsscfg_t *pcfg)
{
	int32 err, idx;
	wl_assoc_params_t assoc_params;
	wlc_bsscfg_t *cfg;
	struct ether_addr ds_ea;
	chanspec_t chanspec;
	psta_bsscfg_cubby_t *psta_cfg;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG  || WLMSG_ASSOC */

	/* Do the bss lookup using the psta's mac address */
	cfg = wlc_bsscfg_find_by_hwaddr(wlc, ea);

	WL_PSTA(("wl%d: Checking PSTA %s: assoc count is %d/%d, PSTA cfg %p\n",
		psta->pub->unit, bcm_ether_ntoa(ea, eabuf),
		PSTA_BSS_COUNT(wlc), PSTA_MAX_ASSOC(wlc), cfg));

	if (cfg == NULL) {
		/* Disallow new assocs after reaching the max psta connection limit */
		if (PSTA_BSS_COUNT(wlc) >= PSTA_MAX_ASSOC(wlc)) {
			WL_PSTA(("wl%d: PSTA assoc limit %d reached.\n",
				psta->pub->unit, PSTA_MAX_ASSOC(wlc)));
			return BCME_NOTASSOCIATED;
		}

		if (ETHER_ISNULLADDR(ea) || ETHER_ISMULTI(ea))
			return BCME_NOTASSOCIATED;

		/* Save packet src mac address */
		bcopy(ea, &ds_ea, ETHER_ADDR_LEN);

		/* Create an alias for this client. Use this alias while talking
		 * to the upstream ap/rptr. Hopefully we generate random enough
		 * numbers so that there won't be duplicate mac addresses.
		 */
		if (PSTA_IS_REPEATER(psta->wlc)) {
			if (psta->mrpt)
				wlc_psta_alias_create(psta, pcfg, ea);
			else
				ETHER_SET_LOCALADDR(ea);
		}

		/* Get the first available bss index */
		idx = wlc_bsscfg_get_free_idx(wlc);
		if (idx < 0) {
			WL_ERROR(("wl%d: PSTA connections limit exceeded\n",
			          psta->pub->unit));
			return BCME_NORESOURCE;
		}

		/* Allocate bsscfg */
		cfg = wlc_bsscfg_alloc(wlc, idx, 0, ea, FALSE);
		if (cfg == NULL)
			return BCME_NOMEM;
		else if ((err = wlc_bsscfg_init(wlc, cfg))) {
			WL_ERROR(("wl%d: wlc_bsscfg_init failed, err %d\n",
			          psta->pub->unit, err));
			wlc_bsscfg_free(wlc, cfg);
			return err;
		}

		WL_PSTA(("wl%d: Allocated bsscfg for idx %d\n", psta->pub->unit, idx));

		psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);

		/* Mark the client as ds sta if we are creating proxy assoc for a
		 * downstream wireless sta.
		 */
		psta_cfg->psa->ds_sta = (wlc_psta_client_is_ds_sta(psta, &ds_ea) != NULL);

		/* Save the original address of downstream client */
		bcopy(&ds_ea, &psta_cfg->psa->ds_ea, ETHER_ADDR_LEN);

	} else {
		ASSERT(!PSTA_IS_PRIMARY(cfg));

		/* Nothing to do if bss is already enabled and assoc state is
		 * not idle.
		 */
		if (cfg->enable && (cfg->assoc->state != AS_IDLE))
			return BCME_NOTASSOCIATED;

		wlc_bsscfg_enable(wlc, cfg);

		WL_PSTA(("wl%d: Enabled bsscfg for idx %d\n", psta->pub->unit,
		         WLC_BSSCFG_IDX(cfg)));

		psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);
	}

	/* Turn psta on */
	cfg->_psta = TRUE;
#ifdef MCAST_REGEN
	cfg->mcast_regen_enable = pcfg->mcast_regen_enable;
#endif /* MCAST_REGEN */

	/* Turn roam off */
	cfg->roam->off = TRUE;

	bcopy(ea, &cfg->cur_etheraddr, ETHER_ADDR_LEN);

	/* inherit extend capability bits and extend capability length
	 * from primary bsscfg.
	 */
	bcopy(&pcfg->ext_cap[0], &cfg->ext_cap[0], DOT11_EXTCAP_LEN_MAX);
	cfg->ext_cap_len = pcfg->ext_cap_len;

	/* Link the primary cfg */
	psta_cfg->psa->pcfg = pcfg;

	/* Allocate rcmta entry for the psta bss mac */
	if ((psta_cfg->psa->rcmta_idx == 0) &&
	    (err = wlc_psta_rcmta_alloc(psta, &psta_cfg->psa->rcmta_idx)) != BCME_OK)
		return err;

	WL_PSTA(("wl%d: Use RCMTA idx %d for etheraddr %s\n", psta->pub->unit,
		psta_cfg->psa->rcmta_idx, bcm_ether_ntoa(ea, eabuf)));

	/* Set the psta bss mac */
	wlc_set_addrmatch(psta->wlc, psta_cfg->psa->rcmta_idx, &cfg->cur_etheraddr,
		AMT_ATTR_VALID | AMT_ATTR_A2 | AMT_ATTR_A1);

	/*
	 * Do not allow overlapping joins. Although they appear to work correctly and speed up
	 * (re-)associating, there may be some historical reason for not doing so.
	 */
	if (AS_IN_PROGRESS(wlc)) {
		WL_PSTA(("wl%d: Hold the join for %s, association in progress\n",
		         psta->pub->unit, bcm_ether_ntoa(ea, eabuf)));
		return BCME_NOTASSOCIATED;
	}

	/*
	 * Make sure the primary is associated before triggering clients in order not to interfere
	 * with an eventual roam scan.
	 */
	if (!wlc_psta_prim_associated(psta)) {
		WL_ASSOC(("wl%d: Hold the join for %s, primary not associated.\n",
		         psta->pub->unit, bcm_ether_ntoa(ea, eabuf)));
		return BCME_NOTASSOCIATED;
	}

	/* When security is enabled we want to make sure that the psta is
	 * configured before initiating the joins. The configuration layer
	 * is reponsible for signaling to the psta driver.
	 */
	if (!PSTA_JOIN_ALLOWED(psta_cfg->psa)) {
		WL_PSTA(("wl%d: Hold the join for %s until config is done\n",
		         psta->pub->unit, bcm_ether_ntoa(ea, eabuf)));
		return BCME_NOTASSOCIATED;
	}

	/* Inform host about original mac (ds_ea) of PSTA */
	wlc_bss_mac_event(wlc, cfg, WLC_E_PSTA_CREATE_IND, &psta_cfg->psa->ds_ea,
		0, 0, 0, 0, 0);

	/* Join the UAP */
	bcopy(&pcfg->BSSID, &assoc_params.bssid, ETHER_ADDR_LEN);
	assoc_params.bssid_cnt = 0; /* just use chanspec num */
	chanspec = pcfg->current_bss->chanspec;
	assoc_params.chanspec_list[0] = CH20MHZ_CHSPEC(wf_chspec_ctlchan(chanspec));
	assoc_params.chanspec_num = 1;
	WL_PSTA(("wl%d: Requesting join for %s on chanspec %x (%x)\n",
		psta->pub->unit, bcm_ether_ntoa(ea, eabuf),
		chanspec, assoc_params.chanspec_list[0]));
	wlc_join(wlc, cfg, pcfg->SSID, pcfg->SSID_len, NULL,
	         &assoc_params, sizeof(wl_assoc_params_t));

	return BCME_NOTASSOCIATED;
}

static void *
wlc_psta_pkt_alloc_copy(wlc_pub_t *pub, void *p)
{
	void *n;
	uint32 totlen;
	wlc_info_t *wlc = (wlc_info_t *)pub->wlc;

	totlen = pkttotlen(pub->osh, p);

	WL_PSTA(("wl%d: %s: Copying %d bytes\n", pub->unit, __FUNCTION__, totlen));

	if ((n = PKTGET(pub->osh, TXOFF + totlen, TRUE)) == NULL) {
		WL_ERROR(("wl%d: %s: PKTGET of length %zd failed\n",
		          pub->unit, __FUNCTION__, TXOFF + PKTLEN(pub->osh, p)));
		return NULL;
	}
	PKTPULL(pub->osh, n, TXOFF);

	wlc_pkttag_info_move(wlc, p, n);
	PKTSETPRIO(n, PKTPRIO(p));

	/* Copy packet data to new buffer */
	pktcopy(pub->osh, p, 0, totlen, PKTDATA(pub->osh, n));

	return n;
}

/* Process ARP request and replies in tx and rx directions */
static int32
wlc_psta_arp_proc(wlc_psta_info_t *psta, wlc_bsscfg_t *cfg, void **p, uint8 *ah,
                  uint8 **shost, bool tx, bool is_bcast)
{
	uint8  *cli_mac, *mod_mac;
	psta_bsscfg_cubby_t *psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);

	/* get cli_mac & mod_mac for arp processing */
	cli_mac = (uint8 *)psta_cfg->psa->ds_ea.octet;
	mod_mac = (uint8 *)cfg->cur_etheraddr.octet;

	if (tx && is_bcast) {
		/* Since we are going to modify the address in arp header
		 * let's make a copy of the whole packet. Otherwise we will
		 * end up modifying arp header of the frame that is being
		 * broadcast to other bridged interfaces.
		 */
		void *n;
		if ((n = wlc_psta_pkt_alloc_copy(psta->pub, *p)) == NULL) {
			WLCNTINCR(psta->pub->_cnt->txnobuf);
			return BCME_NOMEM;
		}
		/* First buffer contains only l2 header */
		ah = PKTDATA(psta->pub->osh, n) + PKTLEN(psta->pub->osh, *p);
		PKTFREE(psta->pub->osh, *p, TRUE);
		*p = n;
		*shost = PKTDATA(psta->pub->osh, n) + ETHER_SRC_OFFSET;
	}

	/* Modify the source mac address in ARP header */
	if (bcm_psta_arp_proc(ah, cli_mac, mod_mac, tx) < 0)
		return BCME_ERROR;

	return BCME_OK;
}

static int32
wlc_psta_dhcp_proc(void *psta_cb, void *cfg_cb, void **p,
                   uint8 *uh, uint8 *dhcph, uint16 dhcplen, uint16 port,
                   uint8 **shost, bool tx, bool is_bcmc)
{
	uint8  *cli_mac, *mod_mac;
	wlc_psta_info_t *psta = (wlc_psta_info_t *)psta_cb;
	wlc_bsscfg_t *cfg = (wlc_bsscfg_t *)cfg_cb;
	psta_bsscfg_cubby_t *psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);

	/* get cli_mac & mod_mac for dhcp processing */
	cli_mac = (uint8 *)psta_cfg->psa->ds_ea.octet;
	mod_mac = (uint8 *)cfg->cur_etheraddr.octet;

	/* Since we are going to modify the address in dhcp header
	 * let's make a copy of the whole packet. Otherwise we will
	 * end up modifying dhcp header of the frame that is being
	 * broadcast to other bridged interfaces.
	 */
	if (tx && is_bcmc) {
		void *n;
		uint8 *ih, proto;
		int32 hlen = 0;

		if ((n = wlc_psta_pkt_alloc_copy(psta->pub, *p)) == NULL) {
			WLCNTINCR(psta->pub->_cnt->txnobuf);
			return BCME_NOMEM;
		}

		/* First buffer contains only l2 header */
		ih = PKTDATA(psta->pub->osh, n) + PKTLEN(psta->pub->osh, *p);

		if (IP_VER(ih) == IP_VER_6) {
			proto = ih[IPV6_NEXT_HDR_OFFSET];
			if (IPV6_EXTHDR(proto)) {
				hlen = ipv6_exthdr_len(ih, &proto);
				if (hlen < 0) {
					PKTFREE(psta->pub->osh, n, TRUE);
					return BCME_OK;
				}
			}
			hlen += IPV6_MIN_HLEN;
		} else
			hlen = IPV4_HLEN(ih);

		uh = ih + hlen;
		dhcph = uh + UDP_HDR_LEN;
		PKTFREE(psta->pub->osh, *p, TRUE);
		*p = n;
		*shost = PKTDATA(psta->pub->osh, n) + ETHER_SRC_OFFSET;
	}

	if (bcm_psta_dhcp_proc(port, uh, dhcph, dhcplen, cli_mac, mod_mac, tx, is_bcmc) < 0)
		return BCME_ERROR;

	return BCME_OK;
}

static int32
wlc_psta_icmp6_proc(wlc_psta_info_t *psta, wlc_bsscfg_t *cfg, void **p, uint8 *ih,
                    uint8 *icmph, uint16 icmplen, uint8 **shost, bool tx, bool is_bcmc)
{
	uint8 *cli_mac, *mod_mac;
	psta_bsscfg_cubby_t *psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);

	BCM_REFERENCE(psta_cfg);

	/* get cli_mac & mod_mac for icmp6 processing */
	cli_mac = (uint8 *)psta_cfg->psa->ds_ea.octet;
	mod_mac = (uint8 *)cfg->cur_etheraddr.octet;

	/* Since we are going to modify the address in icmp option
	 * let's make a copy of the whole packet.
	 */
	if (tx && is_bcmc) {
		void *n;
		uint8 proto;
		int32 hlen = 0;

		if ((n = wlc_psta_pkt_alloc_copy(psta->pub, *p)) == NULL) {
			WLCNTINCR(psta->pub->_cnt->txnobuf);
			return BCME_NOMEM;
		}

		/* First buffer contains only l2 header */
		ih = PKTDATA(psta->pub->osh, n) + PKTLEN(psta->pub->osh, *p);

		if (IP_VER(ih) == IP_VER_6) {
			proto = ih[IPV6_NEXT_HDR_OFFSET];
			if (IPV6_EXTHDR(proto)) {
				hlen = ipv6_exthdr_len(ih, &proto);
				if (hlen < 0) {
					PKTFREE(psta->pub->osh, n, TRUE);
					return BCME_OK;
				}
			}
			hlen += IPV6_MIN_HLEN;
		} else
			hlen = IPV4_HLEN(ih);

		icmph = ih + hlen;
		PKTFREE(psta->pub->osh, *p, TRUE);
		*p = n;
		*shost = PKTDATA(psta->pub->osh, n) + ETHER_SRC_OFFSET;
	}

	/* Modify the source mac address in icmp6 header */
	if (bcm_psta_icmp6_proc(ih, icmph, icmplen, cli_mac, mod_mac, tx) < 0)
		return BCME_ERROR;

	return BCME_OK;
}

static void
wlc_psta_alias_create(wlc_psta_info_t *psta, wlc_bsscfg_t *pcfg, struct ether_addr *shost)
{
	uint32 m, b;
	uint8 oui[3];
#ifdef BCMDBG
	char alias_ea[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	ASSERT(pcfg != NULL);

	bcopy(&shost->octet[0], &oui[0], 3);
	ETHER_CLR_LOCALADDR(oui);

	/* If our oui is different from that of client then we can directly
	 * use the lower 24 bits from client mac addr.
	 */
	if (bcmp(&oui[0], &pcfg->cur_etheraddr.octet[0], 3)) {
		/* Use the oui from primary interface's hardware address */
		shost->octet[0] = pcfg->cur_etheraddr.octet[0];
		shost->octet[1] = pcfg->cur_etheraddr.octet[1];
		shost->octet[2] = pcfg->cur_etheraddr.octet[2];

		/* Set the locally administered bit */
		ETHER_SET_LOCALADDR(shost);

		WL_PSTA(("wl%d: Generated locally unique fixed alias %s\n",
		         psta->pub->unit, bcm_ether_ntoa(shost, alias_ea)));
		return;
	}

	WLCNTINCR(psta->pstadupdetect);

	/* Set the locally administered bit */
	ETHER_SET_LOCALADDR(shost);

	/* Right rotate the octets[1:3] of the mac address. This will make
	 * sure we generate an unique fixed alias for each mac address. If two
	 * client mac addresses have the same octets[1:3] then we will have
	 * a collision. If this happens then generate a random number for the
	 * mac address.
	 */
	m = shost->octet[1] << 16 | shost->octet[2] << 8 | shost->octet[3];

	b = m & 1;
	m >>= 1;
	m |= (b << 23);

	shost->octet[1] = m >> 16;
	shost->octet[2] = (m >> 8) & 0xff;
	shost->octet[3] = m & 0xff;

	/* Generate random value for octets[1:3] of mac address. Make sure
	 * there is no collision locally with already generated ones.
	 */
	while ((wlc_psta_client_is_ds_sta(psta, shost) != NULL) ||
	       (wlc_bsscfg_find_by_hwaddr(psta->wlc, shost) != NULL)) {
		/* We are making sure that our upstream and downstream
		 * instances don't use this address. If there is a match
		 * then try again.
		 */
		WLCNTINCR(psta->pstadupdetect);

		wlc_getrand(psta->wlc, &shost->octet[1], 3);
	}

	WL_PSTA(("wl%d: Generated an alias %s\n", psta->pub->unit,
	         bcm_ether_ntoa(shost, alias_ea)));

	return;
}

/*
 * Process the tx and rx frames based on the protocol type. ARP and DHCP packets are
 * processed by respective handlers.
 */
static int32 BCMFASTPATH
wlc_psta_proto_proc(wlc_psta_info_t *psta, wlc_bsscfg_t *pcfg, void **p, uint8 *eh,
                    wlc_bsscfg_t **cfg, uint8 **shost, bool tx)
{
	uint8 *ih;
	struct ether_addr *ea = NULL;
	uint16 ether_type, pull;
	int32 ea_off = -1;
	bool bcmc, fr_is_1x;
	psta_bsscfg_cubby_t *psta_cfg = NULL;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */
	bool skip_proto_proc = FALSE;

	/* Ignore unknown frames */
	if (bcm_psta_ether_type(psta->pub->osh, eh, &ether_type, &pull, &fr_is_1x) != BCME_OK)
		return BCME_OK;

	if (BCMPCIEDEV_ENAB() &&
		(PKTISTXFRAG(psta->pub->osh, *p) || PKTISRXFRAG(psta->pub->osh, *p))) {
		/* In case of PCIE full dongle split hdr/payload architecture, we do not have
		 * the complete payload in the dongle. So, we should skip all proto/payload
		 * processing here.
		 * Only ethernet header manipulation should be done here.
		 * The payload inspection and manipulation will be done at the host/dhd.
		 */
		 skip_proto_proc = TRUE;
	}
	/* Send 1x frames as is. If primary is not authorized yet then wait for it
	 * before starting wpa handshake for secondary associations.
	 */
	if (fr_is_1x)
		return BCME_OK;

	ih = eh + pull;
	bcmc = ETHER_ISMULTI(eh + ETHER_DEST_OFFSET);

	if (tx) {
		/* Since we are going to modify the header below and the packet
		 * may be shared we allocate a header buffer and prepend it to
		 * the original sdu.
		 */
		if (bcmc && (!skip_proto_proc)) {
			void *n;

			if ((n = PKTGET(psta->pub->osh, TXOFF + pull, TRUE)) == NULL) {
				WL_ERROR(("wl%d: %s: PKTGET headroom %zd failed\n",
				          psta->pub->unit, __FUNCTION__, TXOFF));
				WLCNTINCR(psta->pub->_cnt->txnobuf);
				return BCME_NOMEM;
			}
			PKTPULL(psta->pub->osh, n, TXOFF);

			wlc_pkttag_info_move(psta->wlc, *p, n);
			PKTSETPRIO(n, PKTPRIO(*p));

			/* Copy ether header from data buffer to header buffer */
			memcpy(PKTDATA(psta->pub->osh, n),
			       PKTDATA(psta->pub->osh, *p), pull);
			PKTPULL(psta->pub->osh, *p, pull);

			/* Chain original sdu onto newly allocated header */
			PKTSETNEXT(psta->pub->osh, n, *p);

			eh = PKTDATA(psta->pub->osh, n);
			ih = PKTDATA(psta->pub->osh, *p);
			*p = n;
			*shost = eh + ETHER_SRC_OFFSET;
		}

		/* Find the proxy sta using client's original mac address */
		*cfg = wlc_psta_find_by_ds_ea(psta, *shost);
		if (*cfg == NULL) {
			wlc_psta_create(psta, psta->wlc,
			                (struct ether_addr *)*shost, pcfg);
			WLCNTINCR(psta->pub->_cnt->pstatxnoassoc);
			WL_PSTA(("wl%d: Creating psta for client %s\n",
			         psta->pub->unit,
			         bcm_ether_ntoa((struct ether_addr *)*shost, eabuf)));
			return BCME_NOTASSOCIATED;
		}

		/* Replace the src mac address in the ethernet header with
		 * newly created alias.
		 */
		psta_cfg = PSTA_BSSCFG_CUBBY(psta, *cfg);
		PSTA_SET_ALIAS(psta,  psta_cfg->psa, *cfg, *shost);
		ea = &((*cfg)->cur_etheraddr);
		ea_off = ETHER_SRC_OFFSET;

		WL_PSTA(("wl%d.%d: Translate addr to %s\n", psta->pub->unit,
		         WLC_BSSCFG_IDX(*cfg),
		         bcm_ether_ntoa(&((*cfg)->cur_etheraddr), eabuf)));
	} else {
		/* Restore proxy client address in the ether header */
		if (!bcmc) {
			ASSERT(*cfg != NULL);
#ifdef BCMDBG
			/* If local admin bit is not set then ignore the frame,
			 * do not proxy such address.
			 */
			if (!PSTA_IS_ALIAS(psta, *cfg, eh)) {
				WL_INFORM(("wl%d: Not proxying frame\n",
				           psta->pub->unit));
				return BCME_NOTASSOCIATED;
			}
#endif /* BCMDBG */
			psta_cfg = PSTA_BSSCFG_CUBBY(psta, *cfg);
			PSTA_CLR_ALIAS(psta, psta_cfg->psa, *cfg, eh + ETHER_DEST_OFFSET);
			ea = &psta_cfg->psa->ds_ea;
			ea_off = ETHER_DEST_OFFSET;

			WL_PSTA(("wl%d.%d: Restore addr to %s\n", psta->pub->unit,
			         WLC_BSSCFG_IDX(*cfg),
			         bcm_ether_ntoa(&psta_cfg->psa->ds_ea, eabuf)));
		}
	}

#if (defined(PKTC) || defined(PKTC_DONGLE))
	if (PKTISCHAINED(*p)) {
		void *t;
		int32 ccnt;

		ASSERT((ea != NULL) && (ea_off != -1));
		for (t = PKTCLINK(*p); t != NULL; t = PKTCLINK(t)) {
			eh = PKTDATA(psta->pub->osh, t);
			eacopy(ea, eh + ea_off);
		}
		ASSERT(psta_cfg != NULL);
		ccnt = PKTCCNT(*p) - 1;
		if (tx) {
			WLCNTADD(psta->pub->_cnt->pstatxucast, ccnt);
			WLCNTADD(psta_cfg->psa->txucast, ccnt);
		} else {
			WLCNTADD(psta_cfg->psa->rxucast, ccnt);
			WLCNTADD(psta->pub->_cnt->pstarxucast, ccnt);
		}
	}
#else
	BCM_REFERENCE(ea);
	BCM_REFERENCE(ea_off);
#endif /*  (defined(PKTC) || defined(PKTC_DONGLE)) */

	/* Skip protocol processing if we do not have complete packet */
	if (skip_proto_proc)
		return BCME_OK;

	switch (ether_type) {
	case ETHER_TYPE_IP:
	case ETHER_TYPE_IPV6:
		break;
	case ETHER_TYPE_ARP:
		return wlc_psta_arp_proc(psta, *cfg, p, ih, shost, tx, bcmc);
	default:
		WL_PSTA(("wl%d: Unhandled ether type 0x%x\n",
		         psta->pub->unit, ether_type));
		return BCME_OK;
	}

	if (IP_VER(ih) == IP_VER_4) {
		if (IPV4_PROT(ih) == IP_PROT_UDP)
			return bcm_psta_udp_proc(psta, *cfg, p, ih, ih + IPV4_HLEN(ih),
			                         shost, tx, bcmc, wlc_psta_dhcp_proc);
	} else if (IP_VER(ih) == IP_VER_6) {
		uint8 proto = ih[IPV6_NEXT_HDR_OFFSET];
		int32 exthlen = 0;

		if (IPV6_EXTHDR(proto)) {
			exthlen = ipv6_exthdr_len(ih, &proto);
			if (exthlen < 0)
				return BCME_OK;
		}

		WL_PSTA(("wl%d: IP exthlen %d proto %d\n", psta->pub->unit,
		         exthlen, proto));

		if (proto == IP_PROT_UDP)
			return bcm_psta_udp_proc(psta, *cfg, p, ih, ih + IPV4_HLEN(ih),
			                         shost, tx, bcmc, wlc_psta_dhcp_proc);
		else if (proto == IP_PROT_ICMP6)
			return wlc_psta_icmp6_proc(psta, *cfg, p, ih,
			                           ih + IPV6_MIN_HLEN + exthlen,
			                           IPV6_PAYLOAD_LEN(ih) - exthlen,
			                           shost, tx, bcmc);
	}

	return BCME_OK;
}

/* Proxy assoc lookup - PSTA in any state, also checking on alias. */
static wlc_bsscfg_t * BCMFASTPATH
wlc_psta_find_any(wlc_psta_info_t *psta, struct ether_addr *mac)
{
	uint32 idx;
	wlc_bsscfg_t *cfg;

	FOREACH_PSTA(psta->wlc, idx, cfg) {
		if (EA_CMP(mac, cfg->cur_etheraddr.octet))
			return cfg;
		if (EA_CMP(mac, PSTA_BSSCFG_CUBBY(psta, cfg)->psa->ds_ea.octet))
			return cfg;
	}

	return NULL;
}

/* Proxy assoc lookup - PSTA in state UP only */
wlc_bsscfg_t * BCMFASTPATH
wlc_psta_find(wlc_psta_info_t *psta, uint8 *mac)
{
	uint32 idx;
	wlc_bsscfg_t *cfg;

	FOREACH_UP_PSTA(psta->wlc, idx, cfg) {
		if (EA_CMP(mac, cfg->cur_etheraddr.octet))
			return cfg;
	}

	return NULL;
}

/* Proxy assoc lookup based on the ds client address */
wlc_bsscfg_t * BCMFASTPATH
wlc_psta_find_by_ds_ea(wlc_psta_info_t *psta, uint8 *mac)
{
	uint32 idx;
	wlc_bsscfg_t *cfg;
	psta_bsscfg_cubby_t *psta_cfg;

	FOREACH_PSTA(psta->wlc, idx, cfg) {
		psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);
		if (EA_CMP(mac, psta_cfg->psa->ds_ea.octet))
			return cfg;
	}

	return NULL;
}

/* Downstream client lookup */
static struct scb *
wlc_psta_client_is_ds_sta(wlc_psta_info_t *psta, struct ether_addr *mac)
{
	uint32 idx;
	wlc_bsscfg_t *cfg;
	struct scb *scb = NULL;

	FOREACH_UP_AP(psta->wlc, idx, cfg) {
		scb = wlc_scbfind(psta->wlc, cfg, mac);
		if (scb != NULL)
			break;
	}

	return scb;
}

#ifdef DPSTA
/* See if the downstream client is associated and authorized */
bool
wlc_psta_is_ds_sta(void *psa, struct ether_addr *mac)
{
	wlc_info_t *wlc = (wlc_info_t *)psa;
	struct scb *scb;
	wlc_bsscfg_t *cfg;
	bool ret = FALSE;

	scb = wlc_psta_client_is_ds_sta(wlc->psta, mac);
	if (scb == NULL)
		return FALSE;

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	/* AP is down */
	if (!wlc_bss_connected(cfg))
		return FALSE;

	ret = (cfg->WPA_auth != WPA_AUTH_DISABLED &&
	        WSEC_ENABLED(cfg->wsec)) ? SCB_AUTHORIZED(scb) : SCB_ASSOCIATED(scb);

	return ret;
}

/* Proxy assoc lookup for dpsta - PSTA in state UP only */
wlc_bsscfg_t * BCMFASTPATH
wlc_psta_find_dpsta(void *psa, uint8 *mac)
{
	uint32 idx;
	wlc_info_t *wlc	= (wlc_info_t *)psa;
	wlc_bsscfg_t *cfg;

	FOREACH_UP_PSTA(wlc, idx, cfg) {
		if (EA_CMP(mac, cfg->cur_etheraddr.octet))
			return cfg;
	}

	return NULL;
}

/* Check if bss is authorized */
bool
wlc_psta_authorized(wlc_bsscfg_t *cfg)
{
	return (wlc_bss_connected(cfg) && WLC_PORTOPEN(cfg));
}

void *
wlc_psta_get_psta(wlc_info_t *wlc)
{
	return wlc->psta;
}
#endif /* DPSTA */

/*
 * Process frames in transmit direction by replacing source MAC with
 * the locally generated alias address.
 */
int32 BCMFASTPATH
wlc_psta_send_proc(wlc_psta_info_t *psta, void **p, wlc_bsscfg_t **cfg)
{
	uint8 *eh = PKTDATA(psta->pub->osh, *p);
	struct ether_addr *shost;
	wlc_bsscfg_t *pcfg = *cfg;
	psta_bsscfg_cubby_t *psta_cfg;
	uint16 ether_type;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	ASSERT(pcfg != NULL);

	ether_type = NTOH16(*(uint16 *)(eh + ETHER_TYPE_OFFSET));

	if (ether_type <= ETHER_MAX_DATA) {
		if (ether_type < ETHER_HDR_LEN + SNAP_HDR_LEN + ETHER_TYPE_LEN)
			return BCME_BADLEN;
		else {
			/* LLC/SNAP frame */
			uint8 llc_snap_hdr[SNAP_HDR_LEN] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
			/* Unknown llc-snap header */
			if (bcmp(llc_snap_hdr, eh + ETHER_HDR_LEN, SNAP_HDR_LEN)) {
				WL_PSTA(("wl%d: %s: unknown llc snap hdr\n",
					psta->pub->unit, __FUNCTION__));
				return BCME_ERROR;
			}
		}
	}

	shost = (struct ether_addr *)(eh + ETHER_SRC_OFFSET);

	WL_PSTA(("wl%d: Try to transmit frame w/ ether type 0x%04x from client %s\n",
	         psta->pub->unit, NTOH16(*(uint16 *)(eh + ETHER_TYPE_OFFSET)),
	         bcm_ether_ntoa(shost, eabuf)));

	/* Make sure the primary is connected before sending data or
	 * before initiating secondary proxy assocs.
	 */
	psta_cfg = PSTA_BSSCFG_CUBBY(psta, pcfg);
	if (!wlc_psta_is_datapath_available(psta)) {
		WL_PSTA(("wl%d: Data path isn't available (%d, %d)\n",
		         psta->pub->unit, pcfg->associated, pcfg->wsec_portopen));
		WLCNTINCR(psta->pub->_cnt->pstatxnoassoc);
		WLCNTINCR(psta_cfg->psa->txnoassoc);
		return BCME_NOTASSOCIATED;
	}

	/* Frame belongs to primary bss */
	if (EA_CMP(shost, &pcfg->cur_etheraddr)) {
		WL_PSTA(("wl%d: Sending frame with %s on primary\n",
		         psta->pub->unit, bcm_ether_ntoa(shost, eabuf)));
		return BCME_OK;
	}

	WL_PSTA(("wl%d: See if %s is one of the proxy clients\n", psta->pub->unit,
	         bcm_ether_ntoa(shost, eabuf)));

	/* Wait for primary to authorize before secondary assoc attempts
	 * wpa handshake.
	 */
	if (!WLC_PORTOPEN(pcfg)) {
		WL_PSTA(("wl%d: Primary not authorized\n", psta->pub->unit));
		return BCME_NOTASSOCIATED;
	}

	if (PSTA_IS_REPEATER(psta->wlc)) {
		int32 err;

		/* Modify the source mac address in proto headers */
		err = wlc_psta_proto_proc(psta, pcfg, p, eh, cfg, (uint8 **)&shost, TRUE);
		if (err != BCME_OK)
			return err;
	} else {
		/* See if a secondary assoc exists to send the frame */
		*cfg = wlc_psta_find(psta, (uint8 *)shost);

		if (*cfg == NULL) {
			WLCNTINCR(psta->pub->_cnt->pstatxnoassoc);
			WL_PSTA(("wl%d: Creating psta for client %s\n",
			         psta->pub->unit, bcm_ether_ntoa(shost, eabuf)));
			return wlc_psta_create(psta, psta->wlc, shost, pcfg);
		}
	}

	/* Send the frame on to secondary proxy bss if allowed */
	psta_cfg = PSTA_BSSCFG_CUBBY(psta, *cfg);
	if (wlc_bss_connected(*cfg)) {
		/* Clear inactivity counter */
		psta_cfg->psa->inactivity = 0;

		if (!ETHER_ISMULTI(eh + ETHER_DEST_OFFSET)) {
			WLCNTINCR(psta->pub->_cnt->pstatxucast);
			WLCNTINCR(psta_cfg->psa->txucast);
		} else {
			WLCNTINCR(psta_cfg->psa->txbcmc);
			WLCNTINCR(psta->pub->_cnt->pstatxbcmc);
		}

		WL_PSTA(("wl%d: Tx for PSTA %s\n", psta->pub->unit,
		         bcm_ether_ntoa(shost, eabuf)));

		return BCME_OK;
	}

	WL_PSTA(("wl%d: PSTA not connected, associated %d portopen %d BSSID %s\n",
	         psta->pub->unit, (*cfg)->associated, WLC_PORTOPEN(*cfg),
	         bcm_ether_ntoa(&((*cfg)->BSSID), eabuf)));

	if ((!(*cfg)->enable) || ((*cfg)->assoc->state == AS_IDLE))
		wlc_psta_create(psta, psta->wlc, shost, pcfg);

	WLCNTINCR(psta->pub->_cnt->pstatxnoassoc);
	WLCNTINCR(psta_cfg->psa->txnoassoc);

	/* Before sending data wait for secondary proxy assoc to
	 * complete.
	 */
	return BCME_NOTASSOCIATED;
}

/*
 * Process frames in receive direction. Restore the original mac and mux
 * the frames on to primary.
 */
void BCMFASTPATH
wlc_psta_recv_proc(wlc_psta_info_t *psta, void *p, struct ether_header *eh,
                   wlc_bsscfg_t **bsscfg)
{
	psta_bsscfg_cubby_t *psta_cfg;
	wlc_bsscfg_t *cfg = *bsscfg;
#ifdef BCMDBG
	char s_eabuf[ETHER_ADDR_STR_LEN];
	char d_eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	ASSERT(ETHER_ISMULTI(eh->ether_dhost) || BSSCFG_PSTA(cfg));

	WL_PSTA(("wl%d.%d: Frame of ether type %04x received for %s with src %s\n",
	         psta->pub->unit, WLC_BSSCFG_IDX(cfg), NTOH16(eh->ether_type),
	         bcm_ether_ntoa((struct ether_addr *)eh->ether_dhost, s_eabuf),
	         bcm_ether_ntoa((struct ether_addr *)eh->ether_shost, d_eabuf)));

	/* Restore original mac address for psta repeater assocs */
	if (PSTA_IS_REPEATER(psta->wlc)) {
		int32 err;
		/* Do the rx processing needed to restore the ds client
		 * address in the payload.
		 */
		err = wlc_psta_proto_proc(psta, NULL, &p, (uint8 *)eh,
		                          &cfg, NULL, FALSE);
		if (err != BCME_OK)
			return;
	}

	if (!ETHER_ISMULTI(eh->ether_dhost)) {
		/* Update the rx counters */
		ASSERT(wlc_bss_connected(cfg));

		if (PSTA_IS_PRIMARY(cfg))
			return;

		psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);

		WLCNTINCR(psta_cfg->psa->rxucast);
		WLCNTINCR(psta->pub->_cnt->pstarxucast);

		/* Clear inactivity counter */
		psta_cfg->psa->inactivity = 0;

		/* Send up the frames on primary */
		if ((eh->ether_type != HTON16(ETHER_TYPE_802_1X)) &&
#ifdef BCMWAPI_WAI
		    (eh->ether_type != HTON16(ETHER_TYPE_WAI)) &&
#endif /* BCMWAPI_WAI */
		    (eh->ether_type != HTON16(ETHER_TYPE_802_1X_PREAUTH))) {
			*bsscfg = psta_cfg->psa->pcfg;
		}
	} else {
		WLCNTINCR(psta->pub->_cnt->pstarxbcmc);

		/* Broadcast Multicast frames go up on primary */
	}

	WL_PSTA(("wl%d.%d: Sending up the frame to cfg %d\n",
	         psta->pub->unit, WLC_BSSCFG_IDX(cfg), WLC_BSSCFG_IDX(*bsscfg)));

	return;
}

/*
 * Disassociate and cleanup all the proxy client's state. This is called when primary roams.
 *
 * We save all current psta clients for re-triggering once the primary interface has finished
 * roaming.
 *
 * Note: this may be called several times in a row, so we need to ensure we add no duplicates.
 */
void
wlc_psta_disassoc_all(wlc_psta_info_t *psta)
{
	int32 idx;
	wlc_bsscfg_t *cfg;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	/* Cleanup all active proxy stas */
	FOREACH_PSTA(psta->wlc, idx, cfg) {
		WL_PSTA(("wl%d: %s: Disassoc for PSTA %s\n", psta->pub->unit,
		         __FUNCTION__, bcm_ether_ntoa(&cfg->cur_etheraddr, eabuf)));

		if (1) { /* Set to zero to disable roam re-trigger */
			saved_psta_t *spsta;
			psta_bsscfg_cubby_t *psta_cfg;
			struct wlc_psa *psa;

			psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);
			psa = psta_cfg->psa;

			spsta = psta->saved_clients;
			while (spsta && (memcmp(&spsta->ds_ea, &psa->ds_ea, ETHER_ADDR_LEN) != 0)) {
				spsta = spsta->next;
			}

			if (!spsta) {
				spsta = MALLOC(psta->wlc->pub->osh, sizeof(*spsta));
				if (spsta) {
					memcpy(&spsta->ds_ea, &psa->ds_ea, ETHER_ADDR_LEN);

					spsta->next = psta->saved_clients;
					psta->saved_clients = spsta;
				} else {
					WL_ERROR(("%s: Failed to allocate saved psta client\n",
						__FUNCTION__));
				}
			}
		}
		/* Cleanup the proxy client state */
		wlc_disassociate_client(cfg, FALSE);
	}
}

/*
 * Re-associate all previously saved psta clients.
 */
void
wlc_psta_reassoc_all(wlc_psta_info_t *psta, wlc_bsscfg_t *pcfg)
{
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */
	saved_psta_t *spsta;

	/*
	 * Walk and clear the list.
	 */
	while ((spsta = psta->saved_clients)) {

		/*
		 * We may or may not have PSTAs created already for this address, or the alias
		 * in case of psta repeater mode. Check for both and only re-create the PSTA
		 * interface if neither mac address or alias is found.
		 */
		if (!wlc_psta_find_any(psta, &spsta->ds_ea)) {

			WL_PSTA(("wl%d: %s: Re-trigger PSTA %s\n", psta->pub->unit,
				__FUNCTION__, bcm_ether_ntoa(&spsta->ds_ea, eabuf)));

			/* Trigger saved PSTA client(s) and restore idle time */
			wlc_psta_create(psta, psta->wlc, &spsta->ds_ea, pcfg);

		}
		psta->saved_clients = spsta->next; /* remove from saved list */
		MFREE(psta->pub->osh, spsta, sizeof(*spsta)); /* and free it. */
	}
}

#ifdef BCMDBG
int
wlc_psta_dump(wlc_psta_info_t *psta, struct bcmstrbuf *b)
{
	int32 idx;
	wlc_bsscfg_t *cfg;
	psta_bsscfg_cubby_t *psta_cfg;
	wlc_psa_t *psa;
	char eabuf[ETHER_ADDR_STR_LEN], ds_eabuf[ETHER_ADDR_STR_LEN];

	bcm_bprintf(b, "PSTA BSS %s is %sconnected, primary is %sassociated, "
		"datapath is %savailable\n",
		bcm_ether_ntoa(&psta->wlc->cfg->cur_etheraddr, eabuf),
		wlc_bss_connected(psta->wlc->cfg) ? "":"NOT ",
		wlc_psta_prim_associated(psta) ? "":"NOT ",
		wlc_psta_is_datapath_available(psta) ? "":"NOT ");

	bcm_bprintf(b, "psta mode: %s\n", PSTA_IS_PROXY(psta->wlc) ? "proxy" :
	            PSTA_IS_REPEATER(psta->wlc) ? "repeater" : "disabled");

	/* Dump the global counters */
	bcm_bprintf(b, "pstatxucast %d pstatxnoassoc %d pstatxbcmc %d\n",
	            WLCNTVAL(psta->pub->_cnt->pstatxucast),
	            WLCNTVAL(psta->pub->_cnt->pstatxnoassoc),
	            WLCNTVAL(psta->pub->_cnt->pstatxbcmc));
	bcm_bprintf(b, "pstarxucast %d pstarxbcmc %d\n",
	            WLCNTVAL(psta->pub->_cnt->pstarxucast),
	            WLCNTVAL(psta->pub->_cnt->pstarxbcmc));
	bcm_bprintf(b, "pstatxdhcpc %d pstarxdhcpc %d pstatxdhcps %d pstarxdhcps %d\n",
	            WLCNTVAL(psta->pstatxdhcpc), WLCNTVAL(psta->pstarxdhcpc),
	            WLCNTVAL(psta->pstatxdhcps), WLCNTVAL(psta->pstarxdhcps));
	bcm_bprintf(b, "pstatxdhcpc6 %d pstarxdhcpc6 %d pstatxdhcps6 %d pstarxdhcps6 %d\n",
	            WLCNTVAL(psta->pstatxdhcpc6), WLCNTVAL(psta->pstarxdhcpc6),
	            WLCNTVAL(psta->pstatxdhcps6), WLCNTVAL(psta->pstarxdhcps6));
	bcm_bprintf(b, "pstadupdetect %d\n", WLCNTVAL(psta->pstadupdetect));

	bcm_bprintf(b, "  MAC\t\t\tAlias\t\t\tRCMTA\tAS\tTxBCMC\t\tTxUcast\t\t"
	            "RxUcast\t\tTxNoAssoc\n");

	/* Dump the proxy links */
	FOREACH_PSTA(psta->wlc, idx, cfg) {
		psta_cfg = PSTA_BSSCFG_CUBBY(psta, cfg);
		psa = psta_cfg->psa;
		bcm_bprintf(b, "%c %s\t%s\t%d\t%d\t%d\t\t%d\t\t%d\t\t%d\n",
		            cfg->up ? cfg->associated ? '*' : '+' : '-',
		            bcm_ether_ntoa(&psa->ds_ea, ds_eabuf),
		            bcm_ether_ntoa(&cfg->cur_etheraddr, eabuf),
		            psa->rcmta_idx, cfg->assoc->state,
		            WLCNTVAL(psa->txbcmc), WLCNTVAL(psa->txucast),
		            WLCNTVAL(psa->rxucast), WLCNTVAL(psa->txnoassoc));
	}

	return 0;
}
#endif	/* BCMDBG */

/* enable during assocation state complete via scb state notif */
static void
wlc_psta_scb_state_upd(bcm_notif_client_data ctx, bcm_notif_server_data scb_data)
{
	wlc_psta_info_t *psta = (wlc_psta_info_t *)ctx;
	scb_state_upd_data_t *data = (scb_state_upd_data_t *)scb_data;
	struct scb *scb = data->scb;
	wlc_bsscfg_t *cfg = scb->bsscfg;
	wlc_bss_info_t *target_bss = cfg->target_bss;

	/* hndl transition from unassoc to assoc */
	if (!(data->oldstate & ASSOCIATED) && (scb->state & ASSOCIATED)) {
		/* In case if one of the PSTA mode and DWDS is enabled
		 * and UAP is DWDS capable then PSTA mode would be
		 * dynamically disabled and saved by the function below.
		 */
		wlc_psta_mode_update(psta, cfg, &target_bss->BSSID,
			PSTA_MODE_UPDATE_ACTION_SAVE);
	}

	if (((data->oldstate & ASSOCIATED) && !(scb->state & ASSOCIATED)) ||
		((data->oldstate & AUTHENTICATED) && !(scb->state & AUTHENTICATED))) {
		/* If the deassociated client is downstream, then deauthenticate
		 * the corresponding proxy client to free dongle memory.
		 */
		wlc_psta_deauth_client(psta, &scb->ea);
	}

}
