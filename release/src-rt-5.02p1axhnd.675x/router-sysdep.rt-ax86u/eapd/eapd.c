/*
 * Broadcom EAP dispatcher (EAPD) module main loop
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
 * $Id: eapd.c 785069 2020-03-12 06:34:37Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <ethernet.h>
#include <eapol.h>
#include <eap.h>
#include <wlutils.h>
#include <time.h>
#include <eapd.h>
#include <shutils.h>
#include <wlif_utils.h>
#include <bcmconfig.h>
#include <security_ipc.h>
#include <bcmnvram.h>

uint eapd_msg_level =
#ifdef BCMDBG
	EAPD_ERROR_VAL;
#else
	0;
#endif /* BCMDBG */
bool eapd_ceventd_enable = FALSE;

#define EAPD_WKSP_MAX_EAP_USER_IDENT	32
#define EAPD_WKSP_EAP_USER_NUM			2
#define EAP_TYPE_WSC						EAP_EXPANDED

struct eapd_user_list {
	unsigned char identity[EAPD_WKSP_MAX_EAP_USER_IDENT];
	unsigned int	identity_len;
	unsigned char method;
};

static struct eapd_user_list  eapdispuserlist[EAPD_WKSP_EAP_USER_NUM] = {
	{"WFA-SimpleConfig-Enrollee-1-0", 29, EAP_TYPE_WSC},
	{"WFA-SimpleConfig-Registrar-1-0", 30, EAP_TYPE_WSC}
};

static int eapd_wksp_inited = 0;

/* Static function protype define */
static int sta_init(eapd_wksp_t *nwksp);
static int sta_deinit(eapd_wksp_t *nwksp);
static unsigned char eapd_get_method(unsigned char *user);
static int event_init(eapd_wksp_t *nwksp);
static int event_deinit(eapd_wksp_t *nwksp);

#ifdef __CONFIG_GMAC3__
static bool eapd_add_br_interface(eapd_wksp_t *nwksp, char *ifname, eapd_app_mode_t mode,
	eapd_cb_t **cbp);
#endif // endif
static bool eapd_add_interface(eapd_wksp_t *nwksp, char *ifname, eapd_app_mode_t mode,
	eapd_cb_t **cbp);
static bool _eapd_add_interface(eapd_wksp_t *nwksp, char *ifname, eapd_app_mode_t mode,
	eapd_cb_t **cbp, bool bridge_interface);
static bool eapd_valid_eapol_start(eapd_wksp_t *nwksp, eapd_brcm_socket_t *from, char *ifname);
static eapd_cb_t * eapd_add_app_cb(eapd_app_t *app, void (*app_set_eventmask)(eapd_app_t *app),
		char *lanifname);
static bool eapd_intf_bridged(char *wlifname, char *lanifname);

#ifdef BCM_NETXL
#ifndef WL_NETXL_SUFFIX
#define WL_NETXL_SUFFIX "slave"
#endif // endif

static void replace_wl_netxl_ifname(char *wl_netxl_ifname, char *wl_netxl_suffix_str)
{
	char *p;
	/* If 'slave' even in the ifname. i.e wl1slave,
	 * Null out 'slave' so that the brcmevent pkts payload
	 * have only the wlx name and not wlxslave name
	 */
	if ((p = strstr(wl_netxl_ifname, wl_netxl_suffix_str))) {
		memset(p, 0, sizeof(wl_netxl_suffix_str));
	}

	return;
}
#endif /* BCM_NETXL */

#ifdef BCMDBG
/* #define HEXDUMP */
#ifdef  HEXDUMP
extern int isprint(char i);
static void eapd_hexdump_ascii(const char *title, const unsigned char *buf,
	unsigned int len)
{
	int i, llen;
	const unsigned char *pos = buf;
	const int line_len = 16;

	EAPD_INFO("%s - (data len=%lu):\n", title, (unsigned long) len);
	while (len) {
		llen = len > line_len ? line_len : len;
		EAPD_PRINT("    ");
		for (i = 0; i < llen; i++)
			EAPD_PRINT(" %02x", pos[i]);
		for (i = llen; i < line_len; i++)
			EAPD_PRINT("   ");
		EAPD_PRINT("   ");
		for (i = 0; i < llen; i++) {
			if (isprint(pos[i]))
				EAPD_PRINT("%c", pos[i]);
			else
				EAPD_PRINT("*");
		}
		for (i = llen; i < line_len; i++)
			EAPD_PRINT(" ");
		EAPD_PRINT("\n");
		pos += llen;
		len -= llen;
	}
}

#define HEXDUMP_ASCII(title, buf, len)		eapd_hexdump_ascii(title, buf, len)
#else
#define HEXDUMP_ASCII(title, buf, len)
#endif /* HEXDUMP */
#endif /* BCMDBG */

#ifdef EAPDDUMP
/* dump brcm and preauth socket information */
static void
eapd_dump(eapd_wksp_t *nwksp)
{
	int i, j, flag;
	eapd_brcm_socket_t	*brcmSocket;
	eapd_preauth_socket_t	*preauthSocket;
	eapd_wps_t		*wps;
	eapd_nas_t		*nas;
#ifdef BCMWAPI_WAI
	eapd_wai_t		*wai;
#endif /* BCMWAPI_WAI */
#ifdef BCM_DCS
	eapd_dcs_t		*dcs;
#endif // endif
#ifdef BCM_BSD
	eapd_bsd_t		*bsd;
#endif // endif
#ifdef BCM_DRSDBD
	eapd_drsdbd_t		*drsdbd;
#endif // endif
#ifdef BCM_ASPMD
	eapd_aspm_t		*aspm;
#endif /* BCM_ASPMD */
#ifdef BCM_WLCEVENTD
	eapd_wlceventd_t	*wlceventd;
#endif
	eapd_cb_t		*cb;
	eapd_sta_t		*sta;
	char eabuf[ETHER_ADDR_STR_LEN], bssidbuf[ETHER_ADDR_STR_LEN];

	if (nwksp == NULL) {
		EAPD_PRINT("Wrong argument...\n");
		return;
	}

	EAPD_PRINT("\n***************************\n");

	EAPD_PRINT("WPS:\n");
	wps = &nwksp->wps;
	cb = wps->cb;
	if (cb) {
		EAPD_PRINT("     wps-monitor appSocket %d for %s", wps->appSocket, cb->ifname);
		/* print each cb ifname */
		cb = cb->next;
		while (cb) {
			EAPD_PRINT(" %s", cb->ifname);
			cb = cb->next;
		}
		EAPD_PRINT("\n");

		EAPD_PRINT("       interested wireless interfaces [%s]\n", wps->ifnames);

		/* bitvec for brcmevent */
		flag = 1;
		for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
		for (j = 0; j < 8; j++) {
			if (isset(&wps->bitvec[i], j)) {
				if (flag) {
					EAPD_PRINT("       interested event message id [%d",
						(i*8+j));
					flag = 0;
				}
				else {
					EAPD_PRINT(" %d", (i*8+j));
				}
			}
		}
		}
		EAPD_PRINT("]\n");
	}
	cb = wps->cb;
	while (cb) {
		if (cb->brcmSocket) {
			brcmSocket = cb->brcmSocket;
			EAPD_PRINT("         [rfp=0x%x] drvSocket %d on %s for brcm event packet\n",
				(uint) brcmSocket, brcmSocket->drvSocket, brcmSocket->ifname);
		}
		cb = cb->next;
		if (cb)
			EAPD_PRINT("         ----\n");
	}
	EAPD_PRINT("\n");

	EAPD_PRINT("NAS:\n");
	nas = &nwksp->nas;
	cb = nas->cb;
	if (cb) {
		EAPD_PRINT("     nas appSocket %d for %s", nas->appSocket, cb->ifname);
		/* print each cb ifname */
		cb = cb->next;
		while (cb) {
			EAPD_PRINT(" %s", cb->ifname);
			cb = cb->next;
		}
		EAPD_PRINT("\n");

		EAPD_PRINT("       interested wireless interfaces [%s]\n", nas->ifnames);

		/* bitvec for brcmevent */
		flag = 1;
		for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
		for (j = 0; j < 8; j++) {
			if (isset(&nas->bitvec[i], j)) {
				if (flag) {
					EAPD_PRINT("       interested event message id [%d",
						(i*8+j));
					flag = 0;
				}
				else {
					EAPD_PRINT(" %d", (i*8+j));
				}
			}
		}
		}
		EAPD_PRINT("]\n");
	}
	cb = nas->cb;
	while (cb) {
		preauthSocket = &cb->preauthSocket;
		if (preauthSocket->drvSocket >= 0) {
			EAPD_PRINT("         [0x%x] drvSocket %d on %s for preauth packet\n",
			        (uint) preauthSocket, preauthSocket->drvSocket,
			        preauthSocket->ifname);
		}
		if (cb->brcmSocket) {
			brcmSocket = cb->brcmSocket;
			EAPD_PRINT("         [rfp=0x%x] drvSocket %d on %s for brcm event packet\n",
				(uint) brcmSocket, brcmSocket->drvSocket, brcmSocket->ifname);
		}
		cb = cb->next;
		if (cb)
			EAPD_PRINT("         ----\n");
	}
	EAPD_PRINT("\n");

#ifdef BCMWAPI_WAI
	EAPD_PRINT("WAI:\n");
	wai = &nwksp->wai;
	cb = wai->cb;
	if (cb) {
		EAPD_PRINT("	 wai appSocket %d for %s", wai->appSocket, cb->ifname);
		/* print each cb ifname */
		cb = cb->next;
		while (cb) {
			EAPD_PRINT(" %s", cb->ifname);
			cb = cb->next;
		}
		EAPD_PRINT("\n");

		EAPD_PRINT("	   interested wireless interfaces [%s]\n", wai->ifnames);

		/* bitvec for brcmevent */
		flag = 1;
		for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
		for (j = 0; j < 8; j++) {
			if (isset(&wai->bitvec[i], j)) {
				if (flag) {
					EAPD_PRINT("	   interested event message id [%d",
						(i*8+j));
					flag = 0;
				}
				else {
					EAPD_PRINT(" %d", (i*8+j));
				}
			}
		}
		}
		EAPD_PRINT("]\n");
	}
	cb = wai->cb;
	while (cb) {
		if (cb->brcmSocket) {
			brcmSocket = cb->brcmSocket;
			EAPD_PRINT("	     [rfp=0x%x] drvSocket %d on %s for brcm event packet\n",
				(uint) brcmSocket, brcmSocket->drvSocket, brcmSocket->ifname);
		}
		cb = cb->next;
		if (cb)
			EAPD_PRINT("	     ----\n");
	}
	EAPD_PRINT("\n");
#endif /* BCMWAPI_WAI */

#ifdef BCM_DCS
	EAPD_PRINT("DCS:\n");
	dcs = &nwksp->dcs;
	cb = dcs->cb;
	if (cb) {
		EAPD_PRINT("	 dcs appSocket %d for %s", dcs->appSocket, cb->ifname);
		/* print each cb ifname */
		cb = cb->next;
		while (cb) {
			EAPD_PRINT(" %s", cb->ifname);
			cb = cb->next;
		}
		EAPD_PRINT("\n");

		EAPD_PRINT("	   interested wireless interfaces [%s]\n", dcs->ifnames);

		/* bitvec for brcmevent */
		flag = 1;
		for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
		for (j = 0; j < 8; j++) {
			if (isset(&dcs->bitvec[i], j)) {
				if (flag) {
					EAPD_PRINT("	   interested event message id [%d",
						(i*8+j));
					flag = 0;
				}
				else {
					EAPD_PRINT(" %d", (i*8+j));
				}
			}
		}
		}
		EAPD_PRINT("]\n");
	}
	cb = dcs->cb;
	while (cb) {
		if (cb->brcmSocket) {
			brcmSocket = cb->brcmSocket;
			EAPD_PRINT("	     [rfp=0x%x] drvSocket %d on %s for brcm event packet\n",
				(uint) brcmSocket, brcmSocket->drvSocket, brcmSocket->ifname);
		}
		cb = cb->next;
		if (cb)
			EAPD_PRINT("	     ----\n");
	}
	EAPD_PRINT("\n");
#endif /* BCM_DCS */

#ifdef BCM_BSD
	EAPD_PRINT("BSD:\n");
	bsd = &nwksp->bsd;
	cb = bsd->cb;
	if (cb) {
		EAPD_PRINT("	 bsd appSocket %d for %s", bsd->appSocket, cb->ifname);
		/* print each cb ifname */
		cb = cb->next;
		while (cb) {
			EAPD_PRINT(" %s", cb->ifname);
			cb = cb->next;
		}
		EAPD_PRINT("\n");

		EAPD_PRINT("	   interested wireless interfaces [%s]\n", bsd->ifnames);

		/* bitvec for brcmevent */
		flag = 1;
		for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
		for (j = 0; j < 8; j++) {
			if (isset(&bsd->bitvec[i], j)) {
				if (flag) {
					EAPD_PRINT("	   interested event message id [%d",
						(i*8+j));
					flag = 0;
				}
				else {
					EAPD_PRINT(" %d", (i*8+j));
				}
			}
		}
		}
		EAPD_PRINT("]\n");
	}
	cb = bsd->cb;
	while (cb) {
		if (cb->brcmSocket) {
			brcmSocket = cb->brcmSocket;
			EAPD_PRINT("	     [rfp=0x%x] drvSocket %d on %s for brcm event packet\n",
				(uint) brcmSocket, brcmSocket->drvSocket, brcmSocket->ifname);
		}
		cb = cb->next;
		if (cb)
			EAPD_PRINT("	     ----\n");
	}
	EAPD_PRINT("\n");
#endif /* BCM_BSD */

#ifdef BCM_DRSDBD
	EAPD_PRINT("DRSDBD:\n");
	drsdbd = &nwksp->drsdbd;
	cb = drsdbd->cb;
	if (cb) {
		EAPD_PRINT("	 drsdbd appSocket %d for %s", drsdbd->appSocket, cb->ifname);
		/* print each cb ifname */
		cb = cb->next;
		while (cb) {
			EAPD_PRINT(" %s", cb->ifname);
			cb = cb->next;
		}
		EAPD_PRINT("\n");

		EAPD_PRINT("	   interested wireless interfaces [%s]\n", drsdbd->ifnames);

		/* bitvec for brcmevent */
		flag = 1;
		for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
		for (j = 0; j < 8; j++) {
			if (isset(&drsdbd->bitvec[i], j)) {
				if (flag) {
					EAPD_PRINT("	   interested event message id [%d",
						(i*8+j));
					flag = 0;
				}
				else {
					EAPD_PRINT(" %d", (i*8+j));
				}
			}
		}
		}
		EAPD_PRINT("]\n");
	}
	cb = drsdbd->cb;
	while (cb) {
		if (cb->brcmSocket) {
			brcmSocket = cb->brcmSocket;
			EAPD_PRINT("	     [rfp=0x%x] drvSocket %d on %s for brcm event packet\n",
				(uint) brcmSocket, brcmSocket->drvSocket, brcmSocket->ifname);
		}
		cb = cb->next;
		if (cb)
			EAPD_PRINT("	     ----\n");
	}
	EAPD_PRINT("\n");
#endif /* BCM_DRSDBD */

#ifdef BCM_ASPMD
	EAPD_PRINT("ASPM:\n");
	aspm = &nwksp->aspm;
	cb = aspm->cb;
	if (cb) {
		EAPD_PRINT("     aspm appSocket %d for %s", aspm->appSocket, cb->ifname);
		/* print each cb ifname */
		cb = cb->next;
		while (cb) {
			EAPD_PRINT(" %s", cb->ifname);
			cb = cb->next;
		}
		EAPD_PRINT("\n");

		EAPD_PRINT("       interested wireless interfaces [%s]\n", aspm->ifnames);

		/* bitvec for brcmevent */
		flag = 1;
		for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
		for (j = 0; j < 8; j++) {
			if (isset(&aspm->bitvec[i], j)) {
				if (flag) {
					EAPD_PRINT("       interested event message id [%d",
						(i*8+j));
					flag = 0;
				}
				else {
					EAPD_PRINT(" %d", (i*8+j));
				}
			}
		}
		}
		EAPD_PRINT("]\n");
	}
	cb = aspm->cb;
	while (cb) {
		if (cb->brcmSocket) {
			brcmSocket = cb->brcmSocket;
			EAPD_PRINT("         [rfp=0x%x] drvSocket %d on %s for brcm event packet\n",
				(uint) brcmSocket, brcmSocket->drvSocket, brcmSocket->ifname);
		}
		cb = cb->next;
		if (cb)
			EAPD_PRINT("         ----\n");
	}
	EAPD_PRINT("\n");
#endif /* BCM_ASPMD */

#ifdef BCM_WLCEVENTD
	EAPD_PRINT("WLCEVENTD:\n");
	wlceventd = &nwksp->wlceventd;
	cb = wlceventd->cb;
	if (cb) {
		EAPD_PRINT("	 wlceventd appSocket %d for %s", wlceventd->appSocket, cb->ifname);
		/* print each cb ifname */
		cb = cb->next;
		while (cb) {
			EAPD_PRINT(" %s", cb->ifname);
			cb = cb->next;
		}
		EAPD_PRINT("\n");

		EAPD_PRINT("	   interested wireless interfaces [%s]\n", wlceventd->ifnames);

		/* bitvec for brcmevent */
		flag = 1;
		for (i = 0; i < WL_EVENTING_MASK_LEN; i++) {
		for (j = 0; j < 8; j++) {
			if (isset(&wlceventd->bitvec[i], j)) {
				if (flag) {
					EAPD_PRINT("	   interested event message id [%d",
						(i*8+j));
					flag = 0;
				}
				else {
					EAPD_PRINT(" %d", (i*8+j));
				}
			}
		}
		}
		EAPD_PRINT("]\n");
	}
	cb = wlceventd->cb;
	while (cb) {
		if (cb->brcmSocket) {
			brcmSocket = cb->brcmSocket;
			EAPD_PRINT("	     [rfp=0x%x] drvSocket %d on %s for brcm event packet\n",
				(uint) brcmSocket, brcmSocket->drvSocket, brcmSocket->ifname);
		}
		cb = cb->next;
		if (cb)
			EAPD_PRINT("	     ----\n");
	}
	EAPD_PRINT("\n");
#endif /* BCM_WLCEVENTD */

	EAPD_PRINT("BRCM (brcm event):\n");
	for (i = 0; i < nwksp->brcmSocketCount; i++) {
		brcmSocket = &nwksp->brcmSocket[i];
		EAPD_PRINT("     [0x%x] [inuseCount=%d] drvSocket %d on %s for "
		        "brcm event packet\n",	(uint) brcmSocket,
		        brcmSocket->inuseCount, brcmSocket->drvSocket,
		        brcmSocket->ifname);
	}

	EAPD_PRINT("\n");

	EAPD_PRINT("Stations Info:\n");
	j = 0;
	for (i = 0; i < EAPD_WKSP_MAX_SUPPLICANTS; i++) {
		for (sta = nwksp->sta_hashed[i]; sta; sta = sta->next) {
			EAPD_PRINT("     [%d] %s from %s[%s]\n", j++,
				ether_etoa((uchar *)&sta->ea, eabuf), sta->ifname,
				ether_etoa((uchar *)&sta->bssid, bssidbuf));
		}
	}
	EAPD_PRINT("***************************\n");
}
#endif /* EAPDDUMP */

#ifdef BCMDBG
void
eapd_wksp_display_usage(void)
{
	EAPD_PRINT("\nUsage: eapd [options]\n\n");
	EAPD_PRINT("\n-wps ifname(s)\n");
	EAPD_PRINT("\n-nas ifname(s)\n");
#ifdef BCMWAPI_WAI
	EAPD_PRINT("\n-wai ifname(s)\n");
#endif /* BCMWAPI_WAI */
#ifdef BCM_DCS
	EAPD_PRINT("\n-dcs ifname(s)\n");
#endif /* BCM_DCS */
#ifdef BCM_BSD
	EAPD_PRINT("\n-bsd ifname(s)\n");
#endif /* BCM_BSD */
#ifdef BCM_DRSDBD
	EAPD_PRINT("\n-drsdbd ifname(s)\n");
#endif /* BCM_DRSDBD */
#ifdef BCM_ASPMD
	EAPD_PRINT("\n-asp ifname(s)\n");
#endif /* BCM_ASPMD */
#ifdef BCM_WLCEVENTD
	EAPD_PRINT("\n-wlceventd ifname(s)\n");
#endif /* BCM_WLCEVENTD */
#ifdef BCM_CEVENT
	EAPD_PRINT("\n-cevent ifname(s)\n");
#endif /* BCM_CEVENT */

	EAPD_PRINT("\n\n");
};
#endif	/* BCMDBG */

/* A small wrapper to avoid calling pstaif iovars */
static bool
eapd_wl_wlif_is_psta(eapd_wksp_t *nwksp, char *ifname)
{
	if (nwksp->psta_enabled == 0)
		return FALSE;

	return wl_wlif_is_psta(ifname);
}

#ifdef EAPD_WKSP_AUTO_CONFIG
int
eapd_wksp_auto_config(eapd_wksp_t *nwksp)
{
	int i;
	char ifnames[256], tmp_ifname[128], tmp_ifnames[128], name[IFNAMSIZ];
	char *next;
	bool needStart = FALSE;
	eapd_cb_t *cb = NULL;

	/* lan */
	for (i = 0; i < EAPD_WKSP_MAX_NO_BRIDGE; i++) {
		memset(tmp_ifname, 0, sizeof(tmp_ifname));
		memset(tmp_ifnames, 0, sizeof(tmp_ifnames));
		if (i == 0) {
			sprintf(tmp_ifname, "lan_ifname");
			sprintf(tmp_ifnames, "lan_ifnames");
		}
		else {
			sprintf(tmp_ifname, "lan%d_ifname", i);
			sprintf(tmp_ifnames, "lan%d_ifnames", i);
		}

		memset(ifnames, 0, sizeof(ifnames));
		memset(name, 0, sizeof(name));
		eapd_safe_get_conf(ifnames, sizeof(ifnames), tmp_ifnames);
		if (!strcmp(ifnames, "")) {
			eapd_safe_get_conf(ifnames, sizeof(ifnames), tmp_ifname);
			if (!strcmp(ifnames, ""))
				continue;
		}
		foreach(name, ifnames, next) {
			if (wps_app_enabled(name))
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_WPS, &cb);
			if (nas_app_enabled(name)) {
#ifdef __CONFIG_GMAC3__
				needStart |= eapd_add_br_interface(nwksp, name, EAPD_APP_NAS, &cb);
#endif // endif
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_NAS, &cb);
			}
#ifdef BCMWAPI_WAI
			if (wai_app_enabled(name))
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_WAI, &cb);
#endif /* BCMWAPI_WAI */

#ifdef BCM_DCS
			if (dcs_app_enabled(name))
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_DCS, &cb);
#endif /* BCM_DCS */

#ifdef BCM_CUSTOM_EVENT
			EAPD_INFO("Start EVT interface...name=%s\n", name);
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_EVT, &cb);
#endif // endif

#ifdef BCM_MEVENT
			if (mevent_app_enabled(name))
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_MEVENT, &cb);
#endif /* BCM_MEVENT */

#ifdef BCM_BSD
			if (bsd_app_enabled(name))
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_BSD, &cb);
#endif /* BCM_BSD */

#ifdef BCM_DRSDBD
			if (drsdbd_app_enabled(name))
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_DRSDBD, &cb);
#endif /* BCM_DRSDBD */

#ifdef BCM_SSD
			if (ssd_app_enabled(name))
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_SSD, &cb);
#endif /* BCM_SSD */

#ifdef BCM_EVENTD
			if (eventd_app_enabled(name))
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_EVENTD, &cb);
#endif /* BCM_EVENTD */

#ifdef BCM_ECBD
			if (ecbd_app_enabled(name))
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_ECBD, &cb);
#endif /* BCM_ECBD */

#ifdef BCM_ASPMD
			if (aspm_app_enabled(name))
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_ASPM, &cb);
#endif /* BCM_ASPMD */

#ifdef BCM_WLEVENT
			if (wlevent_app_enabled(name))
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_WLEVENT, &cb);
#endif /* BCM_WLEVENT */

#ifdef BCM_WLCEVENTD
			if (wlceventd_app_enabled(name))
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_WLCEVENTD, &cb);
#endif /* BCM_WLCEVENTD */

#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
			if (visdcoll_app_enabled(name))
				needStart |= eapd_add_interface(nwksp, name,
					EAPD_APP_VISDCOLL, &cb);
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */

#ifdef BCM_CEVENT
			if (cevent_app_enabled(name))
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_CEVENT, &cb);
#endif /* BCM_CEVENT */

#ifdef BCM_RGD
			if (rgd_app_enabled(name))
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_RGD, &cb);
#endif /* BCM_RGD */

#ifdef BCM_WBD
			if (wbd_app_enabled(name)) {
				needStart |= eapd_add_interface(nwksp, name, EAPD_APP_WBD, &cb);
			}
#endif /* BCM_WBD */
		}
	}

	/* wan */
	memset(ifnames, 0, sizeof(ifnames));
	memset(name, 0, sizeof(name));
	eapd_safe_get_conf(ifnames, sizeof(ifnames), "wan_ifnames");
	foreach(name, ifnames, next) {
		if (wps_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_WPS, &cb);
		if (nas_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_NAS, &cb);
#ifdef BCMWAPI_WAI
		if (wai_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_WAI, &cb);
#endif /* BCMWAPI_WAI */
#ifdef BCM_DCS
		if (dcs_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_DCS, &cb);
#endif /* BCM_DCS */
#ifdef BCM_MEVENT
		if (mevent_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_MEVENT, &cb);
#endif /* BCM_MEVENT */

#ifdef BCM_BSD
		if (bsd_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_BSD, &cb);
#endif /* BCM_BSD */

#ifdef BCM_DRSDBD
		if (drsdbd_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_DRSDBD, &cb);
#endif /* BCM_DRSDBD */

#ifdef BCM_SSD
		if (ssd_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_SSD, &cb);
#endif /* BCM_SSD */

#ifdef BCM_EVENTD
		if (eventd_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_EVENTD, &cb);
#endif /* BCM_EVENTD */

#ifdef BCM_ECBD
		if (ecbd_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_ECBD, &cb);
#endif /* BCM_ECBD */

#ifdef BCM_ASPMD
		if (aspm_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_ASPM, &cb);
#endif /* BCM_ASPMD */

#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
		if (visdcoll_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_VISDCOLL, &cb);
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */

#ifdef BCM_CEVENT
		if (cevent_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_CEVENT, &cb);
#endif /* BCM_CEVENT */

#ifdef BCM_WLEVENT
		if (wlevent_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_WLEVENT, &cb);
#endif /* BCM_WLEVENT */

#ifdef BCM_WLCEVENTD
		if (wlceventd_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_WLCEVENTD, &cb);
#endif /* BCM_WLCEVENTD */

#ifdef BCM_RGD
		if (rgd_app_enabled(name))
			needStart |= eapd_add_interface(nwksp, name, EAPD_APP_RGD, &cb);
#endif /* BCM_RGD */
	}
	return ((needStart == TRUE) ? 0 : -1);
}
#endif	/* EAPD_WKSP_AUTO_CONFIG */

static eapd_cb_t *
eapd_add_app_cb(eapd_app_t *app, void (*app_set_eventmask)(eapd_app_t *app), char *lanifname)
{
	eapd_cb_t *cb;

	cb = app->cb;
	while (cb) {
		if (!strcmp(cb->ifname, lanifname))
			break;
		cb = cb->next;
	}

	if (!cb) {
		/* prepare application structure */
		cb = (eapd_cb_t *)malloc(sizeof(eapd_cb_t));
		if (cb == NULL) {
			EAPD_ERROR("app cb allocate fail for %s ...\n", lanifname);
			return NULL;
		}
		memset(cb, 0, sizeof(eapd_cb_t));

		/* add cb to the head */
		cb->next = app->cb;
		app->cb = cb;

		/* save ifname */
		strncpy(cb->ifname, lanifname, IFNAMSIZ - 1);

		/* save bcm event bitvec */
		if (app_set_eventmask)
			app_set_eventmask(app);
	}

	return cb;
}

static bool
_eapd_add_interface(eapd_wksp_t *nwksp, char *ifname, eapd_app_mode_t mode, eapd_cb_t **cbp,
	bool bridge_interface)
{
	int unit;
	char ifnames[256];
	char tmp[100];
	char *str;
	char os_name[IFNAMSIZ], prefix[8];
	uchar mac[ETHER_ADDR_LEN];
	char *lanifname = NULL;
	eapd_app_t *app;
	eapd_cb_t *cb;
	void (*app_set_eventmask)(eapd_app_t *app) = NULL;

	if ((ifname == NULL) || (cbp == NULL))
		return FALSE;

	*cbp = NULL;

	switch (mode) {
		case EAPD_APP_NAS:
			app = &nwksp->nas;
			app_set_eventmask = (void*) nas_app_set_eventmask;
			break;
		case EAPD_APP_WPS:
			app = &nwksp->wps;
			app_set_eventmask = (void*) wps_app_set_eventmask;
			break;
#ifdef BCMWAPI_WAI
		case EAPD_APP_WAI:
			app = &nwksp->wai;
			app_set_eventmask = (void*) wai_app_set_eventmask;
			break;
#endif /* BCMWAPI_WAI */
#ifdef BCM_CUSTOM_EVENT
		case EAPD_APP_EVT:
			EAPD_INFO("Set evt eventmask...\n");
			app = &nwksp->evt;
			app_set_eventmask = (void*) evt_app_set_eventmask;
			break;
#endif // endif
#ifdef BCM_DCS
		case EAPD_APP_DCS:
			app = &nwksp->dcs;
			app_set_eventmask = (void*) dcs_app_set_eventmask;
			break;
#endif /* BCM_DCS */
#ifdef BCM_MEVENT
		case EAPD_APP_MEVENT:
			app = &nwksp->mevent;
			app_set_eventmask = (void*) mevent_app_set_eventmask;
			break;
#endif /* BCM_MEVENT */
#ifdef BCM_BSD
		case EAPD_APP_BSD:
			app = &nwksp->bsd;
			app_set_eventmask = (void*) bsd_app_set_eventmask;
			break;
#endif /* BCM_BSD */

#ifdef BCM_DRSDBD
		case EAPD_APP_DRSDBD:
			app = &nwksp->drsdbd;
			app_set_eventmask = (void*) drsdbd_app_set_eventmask;
			break;
#endif /* BCM_DRSDBD */

#ifdef BCM_SSD
		case EAPD_APP_SSD:
			app = &nwksp->ssd;
			app_set_eventmask = (void*) ssd_app_set_eventmask;
			break;
#endif /* BCM_SSD */
#ifdef BCM_EVENTD
		case EAPD_APP_EVENTD:
			EAPD_INFO("%s:EAPD_APP_EVENTD", __FUNCTION__);
			app = &nwksp->eventd;
			app_set_eventmask = (void*) eventd_app_set_eventmask;
			break;
#endif /* BCM_EVENTD */
#ifdef BCM_ECBD
		case EAPD_APP_ECBD:
			app = &nwksp->ecbd;
			app_set_eventmask = (void*) ecbd_app_set_eventmask;
			break;
#endif /* BCM_ECBD */
#ifdef BCM_ASPMD
		case EAPD_APP_ASPM:
			app = &nwksp->aspm;
			app_set_eventmask = (void*) aspm_app_set_eventmask;
			break;
#endif /* BCM_ASPMD */

#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
		case EAPD_APP_VISDCOLL:
			app = &nwksp->visdcoll;
			app_set_eventmask = (void*) visdcoll_app_set_eventmask;
			break;
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */

#ifdef BCM_CEVENT
		case EAPD_APP_CEVENT:
			app = &nwksp->cevent;
			app_set_eventmask = (void*) cevent_app_set_eventmask;
			break;
#endif /* BCM_CEVENT */

#ifdef BCM_RGD
		case EAPD_APP_RGD:
			app = &nwksp->rgd;
			app_set_eventmask = (void*) rgd_app_set_eventmask;
			break;
#endif /* BCM_RGD */

#ifdef BCM_WBD
		case EAPD_APP_WBD:
			app = &nwksp->wbd;
			app_set_eventmask = (void*) wbd_app_set_eventmask;
			break;
#endif /* BCM_WBD */

#ifdef BCM_WLEVENT
		case EAPD_APP_WLEVENT:
			app = &nwksp->wlevent;
			app_set_eventmask = (void*) wlevent_app_set_eventmask;
			break;
#endif /* BCM_WLEVENT */

#ifdef BCM_WLCEVENTD
		case EAPD_APP_WLCEVENTD:
			app = &nwksp->wlceventd;
			app_set_eventmask = (void*) wlceventd_app_set_eventmask;
			break;
#endif /* BCM_WLCEVENTD */
		default:
			return FALSE;
	}

	/* verify ifname */
	if (nvifname_to_osifname(ifname, os_name, sizeof(os_name)) < 0)
		return FALSE;

	if (!bridge_interface) {
		if (wl_probe(os_name) ||
			wl_ioctl(os_name, WLC_GET_INSTANCE, &unit, sizeof(unit)))
			return FALSE;
		/* convert eth name to wl name */
		if (osifname_to_nvifname(ifname, prefix, sizeof(prefix)) != 0)
			return FALSE;

		/* check ifname in ifnames list */
		if (find_in_list(app->ifnames, prefix))
			return FALSE; /* duplicate */

		/* Check if PSTA/PSR is enabled on this interface and mark it */
		str = nvram_safe_get(strcat_r(prefix, "_mode", tmp));
		if (!strcmp(str, "psta") || !strcmp(str, "psr")) {
			nwksp->psta_enabled = 1;
		}
	}
	/* find ifname in which lan?_ifnames or wan_ifnames */
	(void) wl_hwaddr(os_name, mac);

	/* for psta i/f set lanifname same as ifname, because we know it is
	 * not attached to the bridge.
	 */
	if (wl_wlif_is_psta(os_name))
		lanifname = ifname;

	/* Check ifname in which bridge and assign the brX to lanifname */
	if (!lanifname)
		lanifname = get_ifname_by_wlmac(mac, ifname);

#ifdef __CONFIG_GMAC3__
	/* In 3GMAC mode, each wl interfaces in "fwd_wlandevs" don't attach to the bridge. */
	if (!lanifname) {
		memset(ifnames, 0, sizeof(ifnames));
		eapd_safe_get_conf(ifnames, sizeof(ifnames), "fwd_wlandevs");
		if ((find_in_list(ifnames, ifname)) || (bridge_interface && !lanifname))
			lanifname = ifname;
	}
#endif // endif

	if (!lanifname)
		return FALSE;

	EAPD_INFO("lanifname %s\n", lanifname);

	/* find ifname in which lan?_ifnames or wan_ifnames */
	/* check lanifname in cb list */
	if ((cb = eapd_add_app_cb(app, app_set_eventmask, lanifname)) == NULL)
		return FALSE;
	else
		EAPD_INFO("add one %s interface cb for %s (%s)\n", prefix, cb->ifname,
			ifname);

	if (!bridge_interface) {
		/* save prefix name to ifnames */
		add_to_list(prefix, app->ifnames, sizeof(app->ifnames));
	} else {
		/* save ifname= "brX" name to ifnames */
		add_to_list(ifname, app->ifnames, sizeof(app->ifnames));
	}

	/* Add additional ifname to app cb if ifname is not equal to lanifname */
	if (strncmp(lanifname, ifname, IFNAMSIZ)) {
		char dpsta_ifname[] = "dpsta";

		memset(ifnames, 0, sizeof(ifnames));
		eapd_safe_get_conf(ifnames, sizeof(ifnames), "dpsta_ifnames");

		if (find_in_list(ifnames, ifname)) {
			lanifname = dpsta_ifname;
		} else {
			lanifname = ifname;
		}

		if ((cb = eapd_add_app_cb(app, app_set_eventmask, lanifname)) != NULL) {
			EAPD_INFO("add additional %s interface cb for %s (%s)\n",
				prefix,	cb->ifname, ifname);
			add_to_list(ifname, app->ifnames, sizeof(app->ifnames));
		}
	}

	/* return the new/existing cb struct pointer */
	*cbp = cb;

	return TRUE;
}

static bool
eapd_add_interface(eapd_wksp_t *nwksp, char *ifname, eapd_app_mode_t mode, eapd_cb_t **cbp)
{
	return _eapd_add_interface(nwksp, ifname, mode, cbp, FALSE);
}

#ifdef __CONFIG_GMAC3__
static bool
eapd_add_br_interface(eapd_wksp_t *nwksp, char *br_if_name, eapd_app_mode_t mode, eapd_cb_t **cbp)
{
	char *br_name = NULL;
	char *ifnames, *ifname;
	int i;
	/* find for lan */
	for (i = 0; i < WLIFU_MAX_NO_BRIDGE; i++) {
		if (i == 0) {
			ifnames = nvram_get("lan_ifnames");
			ifname = nvram_get("lan_ifname");
			if (ifname) {
				/* the name in ifnames may nvifname or osifname */
				if (find_in_list(ifnames, br_if_name) ||
				    find_in_list(ifnames, br_if_name)) {
					br_name = ifname;
					break;
				}
			}
		}
		else {
			char tmptr[] = "lanXX_ifnames";
			char if_name[16];
			sprintf(if_name, "lan%d_ifnames", i);
			sprintf(tmptr, "lan%d_ifname", i);
			ifnames = nvram_get(if_name);
			ifname = nvram_get(tmptr);
			if (ifname) {
				/* the name in ifnames may nvifname or osifname */
				if (find_in_list(ifnames, br_if_name) ||
				    find_in_list(ifnames, br_if_name)) {
					br_name = ifname;
					break;
				}
			}
		}
	}
	if (br_name) {
		return _eapd_add_interface(nwksp, br_name, EAPD_APP_NAS, cbp, TRUE);
	} else {
		return FALSE;
	}
}
#endif /* __CONFIG_GMAC3__ */

int
eapd_wksp_parse_cmd(int argc, char *argv[], eapd_wksp_t *nwksp)
{
	int i = 1;
	bool needStart = FALSE;
	eapd_app_mode_t current_mode = EAPD_APP_UNKNOW;
	eapd_cb_t *cb = NULL;

	if (nwksp == NULL)
		return -1;

	/* dispatch parse command */
	while (i < argc) {
		if (!strncmp(argv[i], "-wps", 4)) {
			current_mode = EAPD_APP_WPS;
		}
		else if (!strncmp(argv[i], "-nas", 4)) {
			current_mode = EAPD_APP_NAS;
		}
#ifdef BCMWAPI_WAI
		else if (!strncmp(argv[i], "-wai", 4)) {
			current_mode = EAPD_APP_WAI;
		}
#endif /* BCMWAPI_WAI */
#ifdef BCM_DCS
		else if (!strncmp(argv[i], "-dcs", 4)) {
			current_mode = EAPD_APP_DCS;
		}
#endif /* BCM_DCS */
#ifdef BCM_CUSTOM_EVENT
		else if (!strncmp(argv[i], "-evt", 4)) {
			current_mode = EAPD_APP_EVT;
		}
#endif // endif
#ifdef BCM_MEVENT
		else if (!strncmp(argv[i], "-mev", 4)) {
			current_mode = EAPD_APP_MEVENT;
		}
#endif /* BCM_MEVENT */
#ifdef BCM_BSD
		else if (!strncmp(argv[i], "-bsd", 4)) {
			current_mode = EAPD_APP_BSD;
		}
#endif /* BCM_BSD */
#ifdef BCM_DRSDBD
		else if (!strncmp(argv[i], "-drsdbd", 6)) {
			current_mode = EAPD_APP_DRSDBD;
		}
#endif /* BCM_DRSDBD */
#ifdef BCM_SSD
		else if (!strncmp(argv[i], "-ssd", 4)) {
			current_mode = EAPD_APP_SSD;
		}
#endif /* BCM_SSD */
#ifdef BCM_EVENTD
		else if (!strncmp(argv[i], "-eventd", 7)) {
			current_mode = EAPD_APP_EVENTD;
		}
#endif /* BCM_EVENTD */
#ifdef BCM_ECBD
		else if (!strncmp(argv[i], "-ecbd", 5)) {
			current_mode = EAPD_APP_ECBD;
		}
#endif /* BCM_ECBD */
#ifdef BCM_ASPMD
		else if (!strncmp(argv[i], "-asp", 4)) {
			current_mode = EAPD_APP_ASPM;
		}
#endif /* BCM_ASPMD */
#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
		else if (!strncmp(argv[i], "-visdcoll", 9)) {
			current_mode = EAPD_APP_VISDCOLL;
		}
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */
#ifdef BCM_CEVENT
		else if (!strncmp(argv[i], "-cevent", 7)) {
			current_mode = EAPD_APP_CEVENT;
		}
#endif /* BCM_CEVENT */
#ifdef BCM_RGD
		else if (!strncmp(argv[i], "-rgd", 4)) {
			current_mode = EAPD_APP_RGD;
		}
#endif /* BCM_RGD */
#ifdef BCM_WBD
		else if (!strncmp(argv[i], "-wbd", 4)) {
			current_mode = EAPD_APP_WBD;
		}
#endif /* BCM_WBD */
		else {
			needStart |= eapd_add_interface(nwksp, argv[i], current_mode, &cb);
		}
		i++;
	}
	return ((needStart == TRUE) ? 0 : -1);
}

int
eapd_add_dif(eapd_wksp_t *nwksp, char *ifname)
{
	char os_name[16], name[16];
	eapd_cb_t *cb = NULL;
	int unit;

	EAPD_INFO("Adding dif %s\n", ifname);

	/* Get the os interface name */
	if (nvifname_to_osifname(ifname, os_name, sizeof(os_name)))
		return -1;

	EAPD_INFO("os_name: %s\n", os_name);

	if (wl_probe(os_name) || wl_ioctl(os_name, WLC_GET_INSTANCE, &unit, sizeof(unit)))
		return -1;

	EAPD_INFO("unit: %d\n", unit);

	snprintf(name, sizeof(name), "wl%d", unit);
	if (!nas_app_enabled(name)) {
		/* We get here if the auth is open or shared wep. For wep we want
		 * to configure the encr keys.
		 */
#ifdef linux
		/* FIXME: This needs to be moved to hotplug. After that we
		 * can get rid of __ECOS.
		 */
		eval("wlconf", ifname, "security");
#endif // endif

		EAPD_INFO("Allow joins from now\n");

		/* If security is not configured then indicate to driver that joins
		 * are allowed for psta.
		 */
		if (wl_iovar_setint(ifname, "psta_allow_join", ON))
			EAPD_ERROR("iovar psta_allow_join failed for %s\n", ifname);

		return -1;
	}

	/* Configure security options for the new interface */
#ifdef linux
	/* FIXME: This needs to be moved to hotplug. After that we
	 * can get rid of __ECOS.
	 */
	eval("wlconf", ifname, "security");
#endif // endif

	/* Add new nas interface cb */
	if (!eapd_add_interface(nwksp, ifname, EAPD_APP_NAS, &cb) || (cb == NULL)) {
		EAPD_ERROR("New NAS interface add for %s failed, cb %p\n", ifname, cb);
		return -1;
	}

	EAPD_INFO("new nas interface for %s added\n", ifname);

	/* Add bcmevent bitvec for nas */
	if (event_init(nwksp) < 0) {
		EAPD_ERROR("event_init failed\n");
		return -1;
	}

	EAPD_INFO("open new sockets for this interface\n");

	/* Add brcm and preauth sockets to talk to the nas daemon
	 * over this new interface.
	 */
	if (nas_open_dif_sockets(nwksp, cb) < 0) {
		EAPD_ERROR("sockets open failed\n");
		return -1;
	}

	/* After security is configured and sockets are created indicate to
	 * driver that joins can be allowed for psta.
	 */
	if (wl_iovar_setint(ifname, "psta_allow_join", ON)) {
		EAPD_ERROR("iovar psta_allow_join failed for %s\n", ifname);
		return -1;
	}

	EAPD_INFO("Allow joins from now on\n");

	return 0;
}

void
eapd_delete_dif(eapd_wksp_t *nwksp, char *ifname)
{
	eapd_app_t *app;
	eapd_cb_t *prev = NULL, *tmp_cb, *cb;
	char os_name[IFNAMSIZ];

	app = &nwksp->nas;

	EAPD_INFO("Deleting dif %s\n", ifname);

	/* look for ifname in cb list */
	cb = app->cb;
	while (cb) {
		if (!strcmp(cb->ifname, ifname))
			break;
		prev = cb;
		cb = cb->next;
	}

	if (!cb)
		return;

	/* Found the cb struct for this interface */
	nas_close_dif_sockets(nwksp, cb);
	tmp_cb = cb;
	if (prev != NULL)
		prev->next = cb->next;
	else
		app->cb = cb->next;
	free(tmp_cb);

	if (nvifname_to_osifname(ifname, os_name, sizeof(os_name)) < 0)
		return;

	EAPD_INFO("Removing %s from app ifnames\n", os_name);

	remove_from_list(os_name, app->ifnames, sizeof(app->ifnames));

	return;
}

eapd_wksp_t *
eapd_wksp_alloc_workspace(void)
{
	eapd_wksp_t *nwksp = (eapd_wksp_t *)malloc(sizeof(eapd_wksp_t));

	if (!nwksp)
		return NULL;
	memset(nwksp, 0, sizeof(eapd_wksp_t));
	FD_ZERO(&nwksp->fdset);
	nwksp->fdmax = -1;
	nwksp->wps.appSocket = -1;
	nwksp->nas.appSocket = -1;
#ifdef BCMWAPI_WAI
	nwksp->wai.appSocket = -1;
#endif /* BCMWAPI_WAI */
#ifdef BCM_DCS
	nwksp->dcs.appSocket = -1;
#endif /* BCM_DCS */
#ifdef BCM_MEVENT
	nwksp->mevent.appSocket = -1;
#endif /* BCM_MEVENT */
#ifdef BCM_BSD
	nwksp->bsd.appSocket = -1;
#endif /* BCM_BSD */
#ifdef BCM_DRSDBD
	nwksp->drsdbd.appSocket = -1;
#endif /* BCM_DRSDBD */
#ifdef BCM_SSD
	nwksp->ssd.appSocket = -1;
#endif /* BCM_SSD */
#ifdef BCM_EVENTD
	nwksp->eventd.appSocket = -1;
#endif /* BCM_EVENTD */
#ifdef BCM_ECBD
	nwksp->ecbd.appSocket = -1;
#endif /* BCM_ECBD */
#ifdef BCM_ASPMD
	nwksp->aspm.appSocket = -1;
#endif /* BCM_ASPMD */
#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
	nwksp->visdcoll.appSocket = -1;
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */
#ifdef BCM_CEVENT
	nwksp->cevent.appSocket = -1;
#endif /* BCM_CEVENT */
#ifdef BCM_RGD
	nwksp->rgd.appSocket = -1;
#endif /* BCM_RGD */
#ifdef BCM_WBD
	nwksp->wbd.appSocket = -1;
#endif /* BCM_WBD */

#ifdef BCM_WLEVENT
	nwksp->wlevent.appSocket = -1;
#endif /* BCM_WLEVENT */
#ifdef BCM_WLCEVENTD
	nwksp->wlceventd.appSocket = -1;
#endif /* BCM_WLCEVENTD */
	nwksp->difSocket = -1;

	nwksp->psta_enabled = 0;

	EAPD_INFO("allocated EAPD workspace\n");

	return nwksp;
}

void
eapd_wksp_free_workspace(eapd_wksp_t * nwksp)
{
	if (!nwksp)
		return;
	free(nwksp);
	EAPD_INFO("free EAPD workspace\n");
	return;
}

int
eapd_wksp_init(eapd_wksp_t *nwksp)
{
	int reuse = 1;
	struct sockaddr_in addr;

	if (nwksp == NULL)
		return -1;

	/* initial sta list */
	if (sta_init(nwksp)) {
		EAPD_ERROR("sta_init fail...\n");
		return -1;
	}

	/* initial wps */
	if (wps_app_init(nwksp)) {
		EAPD_ERROR("wps_app_init fail...\n");
		return -1;
	}

#ifdef BCM_CUSTOM_EVENT
	if (evt_app_init(nwksp)) {
		EAPD_ERROR("evt_app_init fail...\n");
		return -1;
	}
#endif // endif

	/* initial nas */
	if (nas_app_init(nwksp)) {
		EAPD_ERROR("nas_app_init fail...\n");
		return -1;
	}

#ifdef BCMWAPI_WAI
	/* initial wai */
	if (wai_app_init(nwksp)) {
		EAPD_ERROR("wai_app_init fail...\n");
		return -1;
	}
#endif /* BCMWAPI_WAI */

#ifdef BCM_DCS
	/* initial dcs */
	if (dcs_app_init(nwksp)) {
		EAPD_ERROR("dcs_app_init fail...\n");
		return -1;
	}
#endif /* BCM_DCS */

#ifdef BCM_MEVENT
	/* initial mevent */
	if (mevent_app_init(nwksp)) {
		EAPD_ERROR("mevent_app_init fail...\n");
		return -1;
	}
#endif /* BCM_MEVENT */

#ifdef BCM_BSD
	/* initial bsd */
	if (bsd_app_init(nwksp)) {
		EAPD_ERROR("bsd_app_init fail...\n");
		return -1;
	}
#endif /* BCM_BSD */

#ifdef BCM_DRSDBD
	/* initial drsdbd */
	if (drsdbd_app_init(nwksp)) {
		EAPD_ERROR("drsdbd_app_init fail...\n");
		return -1;
	}
#endif /* BCM_DRSDBD */

#ifdef BCM_SSD
	/* initial ssd */
	if (ssd_app_init(nwksp)) {
		EAPD_ERROR("ssd_app_init fail...\n");
		return -1;
	}
#endif /* BCM_SSD */

#ifdef BCM_EVENTD
	/* initial eventd */
	if (eventd_app_init(nwksp)) {
		EAPD_ERROR("eventdd_app_init fail...\n");
		return -1;
	}
#endif /* BCM_EVENTD */

#ifdef BCM_ECBD
	/* initial ecbd */
	if (ecbd_app_init(nwksp)) {
		EAPD_ERROR("ecbd_app_init fail...\n");
		return -1;
	}
#endif /* BCM_ECBD */

#ifdef BCM_ASPMD
	/* initial aspm */
	if (aspm_app_init(nwksp)) {
		EAPD_ERROR("aspm_app_init fail...\n");
		return -1;
	}
#endif /* BCM_ASPMD */

#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
	/* initial visdcoll */
	if (visdcoll_app_init(nwksp)) {
		EAPD_ERROR("visdcoll_app_init fail...\n");
		return -1;
	}
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */

#ifdef BCM_CEVENT
	/* initial cevent */
	if (cevent_app_init(nwksp)) {
		EAPD_ERROR("cevent_app_init fail...\n");
		return -1;
	}
#endif /* BCM_CEVENT */

#ifdef BCM_RGD
	/* initial rgd */
	if (rgd_app_init(nwksp)) {
		EAPD_ERROR("rgd_app_init fail...\n");
		return -1;
	}
#endif /* BCM_RGD */

#ifdef BCM_WBD
	/* init WBD */
	if (wbd_app_init(nwksp)) {
		EAPD_ERROR("wbd_app_init fail...\n");
		return -1;
	}
#endif /* BCM_WBD */

#ifdef BCM_WLEVENT
	/* initial wlevent */
	if (wlevent_app_init(nwksp)) {
		EAPD_ERROR("wlevent_app_init fail...\n");
		return -1;
	}
#endif /* BCM_WLEVENT */

#ifdef BCM_WLCEVENTD
	/* initial wlceventd */
	if (wlceventd_app_init(nwksp)) {
		EAPD_ERROR("wlceventd_app_init fail...\n");
		return -1;
	}
#endif /* BCM_WLCEVENTD */

	/* apply bcmevent bitvec */
	if (event_init(nwksp)) {
		EAPD_ERROR("event_init fail...\n");
		return -1;
	}

	/* create a socket to receive dynamic i/f events */
	nwksp->difSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (nwksp->difSocket < 0) {
		EAPD_ERROR("UDP Open failed.\n");
		return -1;
	}
	if (setsockopt(nwksp->difSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse,
		sizeof(reuse)) < 0) {
		EAPD_ERROR("UDP setsockopt failed.\n");
		close(nwksp->difSocket);
		nwksp->difSocket = -1;
		return -1;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = EAPD_UDP_SIN_ADDR(nwksp);
	addr.sin_port = htons(EAPD_WKSP_DIF_UDP_PORT);
	if (bind(nwksp->difSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		EAPD_ERROR("UDP Bind failed, close nas appSocket %d\n", nwksp->difSocket);
		close(nwksp->difSocket);
		nwksp->difSocket = -1;
		return -1;
	}
	EAPD_INFO("NAS difSocket %d opened\n", nwksp->difSocket);

	return 0;
}

int
eapd_wksp_deinit(eapd_wksp_t *nwksp)
{
	if (nwksp == NULL) {
		EAPD_ERROR("Wrong argument...\n");
		return -1;
	}

	sta_deinit(nwksp);
	wps_app_deinit(nwksp);
#ifdef BCM_CUSTOM_EVENT
	evt_app_deinit(nwksp);
#endif // endif
	nas_app_deinit(nwksp);
#ifdef BCMWAPI_WAI
	wai_app_deinit(nwksp);
#endif /* BCMWAPI_WAI */
#ifdef BCM_DCS
	dcs_app_deinit(nwksp);
#endif /* BCM_DCS */
#ifdef BCM_MEVENT
	mevent_app_deinit(nwksp);
#endif /* BCM_MEVENT */
#ifdef BCM_BSD
	bsd_app_deinit(nwksp);
#endif /* BCM_BSD */
#ifdef BCM_DRSDBD
	drsdbd_app_deinit(nwksp);
#endif /* BCM_DRSDBD */
#ifdef BCM_SSD
	ssd_app_deinit(nwksp);
#endif /* BCM_SSD */
#ifdef BCM_EVENTD
	eventd_app_deinit(nwksp);
#endif /* BCM_EVENTD */
#ifdef BCM_ECBD
	ecbd_app_deinit(nwksp);
#endif /* BCM_ECBD */
#ifdef BCM_ASPMD
	aspm_app_deinit(nwksp);
#endif /* BCM_ASPMD */
#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
	visdcoll_app_deinit(nwksp);
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */
#ifdef BCM_CEVENT
	cevent_app_deinit(nwksp);
#endif /* BCM_CEVENT */
#ifdef BCM_RGD
	rgd_app_deinit(nwksp);
#endif /* BCM_RGD */
#ifdef BCM_WBD
	wbd_app_deinit(nwksp);
#endif /* BCM_WBD */
#ifdef BCM_WLEVENT
	wlevent_app_deinit(nwksp);
#endif /* BCM_WLEVENT */
#ifdef BCM_WLCEVENTD
	wlceventd_app_deinit(nwksp);
#endif /* BCM_WLCEVENTD */

	event_deinit(nwksp);

	/* close  difSocket */
	if (nwksp->difSocket >= 0) {
		EAPD_INFO("close difSocket %d\n", nwksp->difSocket);
		close(nwksp->difSocket);
		nwksp->difSocket = -1;
	}

	return 0;
}

void
eapd_wksp_cleanup(eapd_wksp_t *nwksp)
{
	eapd_wksp_deinit(nwksp);
	eapd_wksp_free_workspace(nwksp);
}

static eapd_cb_t *
eapd_wksp_find_cb(eapd_wksp_t *nwksp, eapd_app_t *app, char *wlifname, uint8 *mac)
{
	eapd_cb_t *cb = NULL;
	char *ifname;
	char ifnames[NVRAM_MAX_VALUE_LEN];
	char dpsta_ifname[] = "dpsta";

	if (!app)
		return NULL;

#ifdef __CONFIG_GMAC3__
	/* In 3GMAC mode, each wl interfaces in "fwd_wlandevs" don't attach to the bridge. */
	memset(ifnames, 0, sizeof(ifnames));
	eapd_safe_get_conf(ifnames, sizeof(ifnames), "fwd_wlandevs");
#endif // endif

	if (eapd_wl_wlif_is_psta(nwksp, wlifname) ||
		wl_wlif_is_wet_ap(wlifname) ||
#ifdef __CONFIG_GMAC3__
		find_in_list(ifnames, wlifname) ||
#endif // endif
		FALSE)
		ifname = wlifname;
	else {
		memset(ifnames, 0, sizeof(ifnames));
		eapd_safe_get_conf(ifnames, sizeof(ifnames), "dpsta_ifnames");
		if (find_in_list(ifnames, wlifname)) {
			ifname = dpsta_ifname;
		} else {
			ifname = get_ifname_by_wlmac(mac, wlifname);

			/* use original wlifname if LAN/WAN ifname is not bridged */
			if (ifname && !eapd_intf_bridged(wlifname, ifname)) {
				ifname = wlifname;
			}
		}
	}

	if (ifname) {
		cb = app->cb;
		while (cb) {
			if (!strcmp(ifname, cb->ifname))
				break;
			cb = cb->next;
		}
	}

	if (!cb)
		EAPD_ERROR("No cb found\n");

	return cb;
}

void
eapd_wksp_dispatch(eapd_wksp_t *nwksp)
{
	fd_set fdset;
	struct timeval tv = {1, 0};    /* timed out every second */
	int status, len, i, bytes;
	uint8 *pkt;
	eapd_brcm_socket_t *brcmSocket;
	eapd_preauth_socket_t *preauthSocket;
	eapd_cb_t *cb;
	eapd_wps_t *wps;
	eapd_nas_t *nas;
#ifdef BCM_CUSTOM_EVENT
	eapd_evt_t *evt;
#endif // endif

#ifdef BCM_NETXL
	bcm_event_t *dpkt;
#endif // endif

#ifdef BCMWAPI_WAI
	eapd_wai_t *wai;
#endif /* BCMWAPI_WAI */
#ifdef BCM_DCS
	eapd_dcs_t *dcs;
#endif /* BCM_DCS */
#ifdef BCM_MEVENT
	eapd_mevent_t *mevent;
#endif /* BCM_MEVENT */
#ifdef BCM_BSD
	eapd_bsd_t *bsd;
#endif /* BCM_BSD */
#ifdef BCM_DRSDBD
	eapd_drsdbd_t *drsdbd;
#endif /* BCM_DRSDBD */
#ifdef BCM_SSD
	eapd_ssd_t *ssd;
#endif /* BCM_SSD */
#ifdef BCM_EVENTD
	eapd_eventd_t *eventd;
#endif /* BCM_EVENTD */
#ifdef BCM_ECBD
	eapd_ecbd_t *ecbd;
#endif /* BCM_ECBD */
#ifdef BCM_ASPMD
	eapd_aspm_t *aspm;
#endif /* BCM_ASPMD */
#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
	eapd_visdcoll_t *visdcoll;
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */
#ifdef BCM_CEVENT
	eapd_cevent_t *cevent;
#endif /* BCM_CEVENT */
#ifdef BCM_RGD
	eapd_rgd_t *rgd;
#endif /* BCM_RGD */
#ifdef BCM_WBD
	eapd_wbd_t *wbd;
#endif /* BCM_WBD */
#ifdef BCM_WLEVENT
	eapd_wlevent_t *wlevent;
#endif /* BCM_WLEVENT */
#ifdef BCM_WLCEVENTD
	eapd_wlceventd_t *wlceventd;
#endif /* BCM_WLCEVENTD */

	FD_ZERO(&nwksp->fdset);
	nwksp->fdmax = -1;

	pkt = &nwksp->packet[IFNAMSIZ];
	len = sizeof(nwksp->packet) - IFNAMSIZ;

	/* add brcm drvSocket */
	for (i = 0; i < nwksp->brcmSocketCount; i++) {
		brcmSocket = &nwksp->brcmSocket[i];
		if (brcmSocket->inuseCount > 0) {
			FD_SET(brcmSocket->drvSocket, &nwksp->fdset);
			if (brcmSocket->drvSocket > nwksp->fdmax)
				nwksp->fdmax = brcmSocket->drvSocket;
		}
	}

	/* add wps appSocket */
	wps = &nwksp->wps;
	if (wps->appSocket >= 0) {
		FD_SET(wps->appSocket, &nwksp->fdset);
		if (wps->appSocket > nwksp->fdmax)
			nwksp->fdmax = wps->appSocket;
	}

	/* add nas appSocket */
	nas = &nwksp->nas;
	if (nas->appSocket >= 0) {
		FD_SET(nas->appSocket, &nwksp->fdset);
		if (nas->appSocket > nwksp->fdmax)
			nwksp->fdmax = nas->appSocket;
	}

#ifdef BCM_CUSTOM_EVENT
	/* add evt appSocket */
	evt = &nwksp->evt;
	if (evt->appSocket >= 0) {
		FD_SET(evt->appSocket, &nwksp->fdset);
		if (evt->appSocket > nwksp->fdmax)
			nwksp->fdmax = evt->appSocket;
	}
#endif // endif

	/* add nas preauth drvSocket */
	cb = nas->cb;
	while (cb) {
		preauthSocket = &cb->preauthSocket;
		if (preauthSocket->drvSocket >= 0) {
			FD_SET(preauthSocket->drvSocket, &nwksp->fdset);
			if (preauthSocket->drvSocket > nwksp->fdmax)
				nwksp->fdmax = preauthSocket->drvSocket;
		}
		cb = cb->next;
	}

#ifdef BCMWAPI_WAI
	/* add wai appSocket */
	wai = &nwksp->wai;
	if (wai->appSocket >= 0) {
		FD_SET(wai->appSocket, &nwksp->fdset);
		if (wai->appSocket > nwksp->fdmax)
			nwksp->fdmax = wai->appSocket;
	}
#endif /* BCMWAPI_WAI */

#ifdef BCM_DCS
	/* add dcs appSocket */
	dcs = &nwksp->dcs;
	if (dcs->appSocket >= 0) {
		FD_SET(dcs->appSocket, &nwksp->fdset);
		if (dcs->appSocket > nwksp->fdmax)
			nwksp->fdmax = dcs->appSocket;
	}
#endif /* BCM_DCS */

#ifdef BCM_MEVENT
	/* add mevent appSocket */
	mevent = &nwksp->mevent;
	if (mevent->appSocket >= 0) {
		FD_SET(mevent->appSocket, &nwksp->fdset);
		if (mevent->appSocket > nwksp->fdmax)
			nwksp->fdmax = mevent->appSocket;
	}
#endif /* BCM_MEVENT */

#ifdef BCM_BSD
	/* add bsd appSocket */
	bsd = &nwksp->bsd;
	if (bsd->appSocket >= 0) {
		FD_SET(bsd->appSocket, &nwksp->fdset);
		if (bsd->appSocket > nwksp->fdmax)
			nwksp->fdmax = bsd->appSocket;
	}
#endif /* BCM_BSD */

#ifdef BCM_DRSDBD
	/* add drsdbd appSocket */
	drsdbd = &nwksp->drsdbd;
	if (drsdbd->appSocket >= 0) {
		FD_SET(drsdbd->appSocket, &nwksp->fdset);
		if (drsdbd->appSocket > nwksp->fdmax)
			nwksp->fdmax = drsdbd->appSocket;
	}
#endif /* BCM_DRSDBD */

#ifdef BCM_SSD
	/* add ssd appSocket */
	ssd = &nwksp->ssd;
	if (ssd->appSocket >= 0) {
		FD_SET(ssd->appSocket, &nwksp->fdset);
		if (ssd->appSocket > nwksp->fdmax)
			nwksp->fdmax = ssd->appSocket;
	}
#endif /* BCM_SSD */

#ifdef BCM_EVENTD
		/* add eventd appSocket */
		eventd = &nwksp->eventd;
		if (eventd->appSocket >= 0) {
			FD_SET(eventd->appSocket, &nwksp->fdset);
			if (eventd->appSocket > nwksp->fdmax)
				nwksp->fdmax = eventd->appSocket;
		}
#endif /* BCM_EVENTD */

#ifdef BCM_ECBD
	/* add ecbd appSocket */
	ecbd = &nwksp->ecbd;
	if (ecbd->appSocket >= 0) {
		FD_SET(ecbd->appSocket, &nwksp->fdset);
		if (ecbd->appSocket > nwksp->fdmax)
			nwksp->fdmax = ecbd->appSocket;
	}
#endif /* BCM_ECBD */

#ifdef BCM_ASPMD
	/* add aspm appSocket */
	aspm = &nwksp->aspm;
	if (aspm->appSocket >= 0) {
		FD_SET(aspm->appSocket, &nwksp->fdset);
		if (aspm->appSocket > nwksp->fdmax)
			nwksp->fdmax = aspm->appSocket;
	}
#endif /* BCM_ASPMD */

#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
		/* add visdcoll appSocket */
		visdcoll = &nwksp->visdcoll;
		if (visdcoll->appSocket >= 0) {
			FD_SET(visdcoll->appSocket, &nwksp->fdset);
			if (visdcoll->appSocket > nwksp->fdmax)
				nwksp->fdmax = visdcoll->appSocket;
		}
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */

#ifdef BCM_CEVENT
	/* add cevent appSocket */
	cevent = &nwksp->cevent;
	if (cevent->appSocket >= 0) {
		FD_SET(cevent->appSocket, &nwksp->fdset);
		if (cevent->appSocket > nwksp->fdmax)
			nwksp->fdmax = cevent->appSocket;
	}
#endif /* BCM_CEVENT */

#ifdef BCM_RGD
	/* add rgd appSocket */
	rgd = &nwksp->rgd;
	if (rgd->appSocket >= 0) {
		FD_SET(rgd->appSocket, &nwksp->fdset);
		if (rgd->appSocket > nwksp->fdmax)
			nwksp->fdmax = rgd->appSocket;
	}
#endif /* BCM_RGD */

#ifdef BCM_WBD
	/* add wbd appSocket */
	wbd = &nwksp->wbd;
	if (wbd->appSocket >= 0) {
		FD_SET(wbd->appSocket, &nwksp->fdset);
		if (wbd->appSocket > nwksp->fdmax)
			nwksp->fdmax = wbd->appSocket;
	}
#endif /* BCM_WBD */

#ifdef BCM_WLEVENT
	/* add wlevent appSocket */
	wlevent = &nwksp->wlevent;
	if (wlevent->appSocket >= 0) {
		FD_SET(wlevent->appSocket, &nwksp->fdset);
		if (wlevent->appSocket > nwksp->fdmax)
			nwksp->fdmax = wlevent->appSocket;
	}
#endif

#ifdef BCM_WLCEVENTD
	/* add wlceventd appSocket */
	wlceventd = &nwksp->wlceventd;
	if (wlceventd->appSocket >= 0) {
		FD_SET(wlceventd->appSocket, &nwksp->fdset);
		if (wlceventd->appSocket > nwksp->fdmax)
			nwksp->fdmax = wlceventd->appSocket;
	}
#endif /* BCM_WLCEVENTD */

	/* add difSocket */
	if (nwksp->difSocket >= 0) {
		FD_SET(nwksp->difSocket, &nwksp->fdset);
		if (nwksp->difSocket > nwksp->fdmax)
			nwksp->fdmax = nwksp->difSocket;
	}

	if (nwksp->fdmax == -1) {
		/* do shutdown procedure */
		nwksp->flags  = EAPD_WKSP_FLAG_SHUTDOWN;
		EAPD_ERROR("There is no any sockets in the fd set, shutdown...\n");
		return;
	}

	fdset = nwksp->fdset;
	status = select(nwksp->fdmax+1, &fdset, NULL, NULL, &tv);
	if (status > 0) {
		/* check brcm drvSocket */
		for (i = 0; i < nwksp->brcmSocketCount; i++) {
			brcmSocket = &nwksp->brcmSocket[i];
			if (brcmSocket->inuseCount > 0 &&
			     FD_ISSET(brcmSocket->drvSocket, &fdset)) {
				/*
				 * Use eapd_message_read read to receive BRCM packets,
				 * this change allows the drvSocket is a file descriptor, and
				 * handle any other local io differences
				 */
				bytes = eapd_message_read(brcmSocket->drvSocket, pkt, len);
				if (bytes > 0)  {
#ifdef BCM_NETXL
					dpkt = (bcm_event_t *)pkt;
					replace_wl_netxl_ifname(dpkt->event.ifname,
						WL_NETXL_SUFFIX);
#endif // endif
					/* call brcm recv handler */
					eapd_brcm_recv_handler(nwksp, brcmSocket, pkt, &bytes);
				}
			}
		}

		/* check wps appSocket */
		if (wps->appSocket >= 0 &&
		     FD_ISSET(wps->appSocket, &fdset)) {
			bytes = recv(wps->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;
#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from WPS app", pkt, bytes);
#endif // endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

				cb = eapd_wksp_find_cb(nwksp, wps, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb) {
					wps_app_recv_handler(nwksp, ifname, cb, (uint8 *)eth,
						&bytes, (struct ether_addr *)&eth->ether_shost);
#ifdef BCM_CEVENT
					/* origin from WPS to driver, forward to cevent */
					cevent_generic_hub(nwksp, ifname, cb,
							(uint8 *)eth, bytes, FALSE, CEVENT_WPS);
#endif /* BCM_CEVENT */
				}
			}
		}

#ifdef BCM_CUSTOM_EVENT
		/* check evt appSocket */
		if (evt->appSocket >= 0 &&
		     FD_ISSET(evt->appSocket, &fdset)) {
			bytes = recv(evt->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;
#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from EVT app", pkt, bytes);
#endif // endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

				cb = eapd_wksp_find_cb(wps, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb)
					evt_app_recv_handler(nwksp, ifname, cb, (uint8 *)eth,
						&bytes, (struct ether_addr *)&eth->ether_shost);
			}
		}
#endif /* BCM_CUSTOM_EVENT */

		/* check difSocket */
		if (nwksp->difSocket >= 0 &&
		     FD_ISSET(nwksp->difSocket, &fdset)) {
			bytes = recv(nwksp->difSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				uint32 event;

#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from hotplug", pkt, bytes);
#endif // endif
				event = *(uint32 *)(ifname + IFNAMSIZ);
				if (event == 0)
					eapd_add_dif(nwksp, ifname);
				else if (event == 1)
					eapd_delete_dif(nwksp, ifname);
			}
		}

		/* check nas appSocket */
		if (nas->appSocket >= 0 &&
		     FD_ISSET(nas->appSocket, &fdset)) {
			bytes = recv(nas->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;

#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from NAS app", pkt, bytes);
#endif // endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

#ifdef __CONFIG_GMAC3__
				{
					eapol_header_t *eapol = (eapol_header_t*) eth;
					if (ntohs(eapol->eth.ether_type) ==
						ETHER_TYPE_802_1X_PREAUTH) {
						ifname = get_ifname_by_wlmac(eth->ether_shost,
							ifname);
					}
				}
#endif /* __CONFIG_GMAC3__ */
				cb = eapd_wksp_find_cb(nwksp, nas, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb) {
					nas_app_recv_handler(nwksp, ifname, cb, (uint8 *)eth,
							&bytes);
#ifdef BCM_CEVENT
					/* origin from NAS to driver, forward to cevent */
					cevent_generic_hub(nwksp, ifname, cb,
							(uint8 *)eth, bytes, FALSE, CEVENT_NAS);
#endif /* BCM_CEVENT */

				}
			}
		}

		/* check preauth drvSocket */
		cb = nas->cb;
		while (cb) {
			preauthSocket = &cb->preauthSocket;
			if (preauthSocket->drvSocket >= 0 &&
			     FD_ISSET(preauthSocket->drvSocket, &fdset)) {
				/*
				 * Use eapd_message_read instead to receive PREAUTH packets,
				 * this change allows the drvSocket is a file descriptor
				 */
				bytes = eapd_message_read(preauthSocket->drvSocket, pkt, len);
				if (bytes > 0) {
#ifdef BCMDBG
					HEXDUMP_ASCII("EAPD:: data from PREAUTH Driver",
						pkt, bytes);
#endif // endif
					/* call preauth recv handler */
					eapd_preauth_recv_handler(nwksp, cb->ifname,
						pkt, &bytes);
				}
			}

			cb = cb->next;
		}

#ifdef BCMWAPI_WAI
		/* check wai appSocket */
		if (wai->appSocket >= 0 &&
		     FD_ISSET(wai->appSocket, &fdset)) {
			bytes = recv(wai->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;
#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from WAI app", pkt, bytes);
#endif // endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

				cb = eapd_wksp_find_cb(nwksp, wai, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb)
					wai_app_recv_handler(nwksp, cb, (uint8 *)eth, &bytes);
			}
		}
#endif /* BCMWAPI_WAI */

#ifdef BCM_DCS
		/* check dcs appSocket */
		if (dcs->appSocket >= 0 &&
		     FD_ISSET(dcs->appSocket, &fdset)) {
			bytes = recv(dcs->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;
#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from DCS app", pkt, bytes);
#endif // endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

				cb = eapd_wksp_find_cb(nwksp, dcs, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb)
					dcs_app_recv_handler(nwksp, cb, (uint8 *)eth, &bytes);
			}
		}
#endif /* BCM_DCS */

#ifdef BCM_MEVENT
		/* check mevent appSocket */
		if (mevent->appSocket >= 0 &&
		     FD_ISSET(mevent->appSocket, &fdset)) {
			bytes = recv(mevent->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;
#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from MEVENT app", pkt, bytes);
#endif // endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

				cb = eapd_wksp_find_cb(nwksp, mevent, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb)
					mevent_app_recv_handler(nwksp, cb, (uint8 *)eth, &bytes);
			}
		}
#endif /* BCM_MEVENT */

#ifdef BCM_BSD
		/* check bsd appSocket */
		if (bsd->appSocket >= 0 &&
		     FD_ISSET(bsd->appSocket, &fdset)) {
			bytes = recv(bsd->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;
#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from DCS app", pkt, bytes);
#endif // endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

				cb = eapd_wksp_find_cb(nwksp, bsd, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb)
					bsd_app_recv_handler(nwksp, cb, (uint8 *)eth, &bytes);
			}
		}
#endif /* BCM_BSD */

#ifdef BCM_DRSDBD
		/* check drsdbd appSocket */
		if (drsdbd->appSocket >= 0 &&
		     FD_ISSET(drsdbd->appSocket, &fdset)) {
			bytes = recv(drsdbd->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;
#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from DCS app", pkt, bytes);
#endif // endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

				cb = eapd_wksp_find_cb(nwksp, drsdbd, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb)
					drsdbd_app_recv_handler(nwksp, cb, (uint8 *)eth, &bytes);
			}
		}
#endif /* BCM_DRSDBD */

#ifdef BCM_SSD
		/* check ssd appSocket */
		if (ssd->appSocket >= 0 &&
		     FD_ISSET(ssd->appSocket, &fdset)) {
			bytes = recv(ssd->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;
#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from SSD app", pkt, bytes);
#endif // endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

				cb = eapd_wksp_find_cb(nwksp, ssd, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb)
					ssd_app_recv_handler(nwksp, cb, (uint8 *)eth, &bytes);
			}
		}
#endif /* BCM_SSD */

#ifdef BCM_EVENTD
			/* check eventd appSocket */
			if (eventd->appSocket >= 0 &&
				FD_ISSET(eventd->appSocket, &fdset)) {
				bytes = recv(eventd->appSocket, pkt, len, 0);
				if (bytes > ETHER_HDR_LEN) {
					char *ifname = (char *) pkt;
					struct ether_header *eth;
#ifdef BCMDBG
					HEXDUMP_ASCII("EAPD:: data from EVENTD app", pkt, bytes);
#endif // endif
					/* ether header */
					eth = (struct ether_header*)(ifname + IFNAMSIZ);
					bytes -= IFNAMSIZ;

					cb = eapd_wksp_find_cb(nwksp, eventd, ifname, eth->ether_shost);
					/* send message data out. */
					if (cb) {
						eventd_app_recv_handler(nwksp, cb,
							(uint8 *)eth, &bytes);
					}
				}
			}
#endif /* BCM_EVENTD */

#ifdef BCM_ECBD
		/* check ecbd appSocket */
		if (ecbd->appSocket >= 0 &&
		     FD_ISSET(ecbd->appSocket, &fdset)) {
			bytes = recv(ecbd->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;
#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from ECBD app", pkt, bytes);
#endif // endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

				cb = eapd_wksp_find_cb(nwksp, ecbd, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb)
					ecbd_app_recv_handler(nwksp, cb, (uint8 *)eth, &bytes);
			}
		}
#endif /* BCM_ECBD */

#ifdef BCM_ASPMD
		/* check aspm appSocket */
		if (aspm->appSocket >= 0 &&
		     FD_ISSET(aspm->appSocket, &fdset)) {
			bytes = recv(aspm->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;
#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from ASPM app", pkt, bytes);
#endif // endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

				cb = eapd_wksp_find_cb(nwksp, aspm, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb) {
					aspm_app_recv_handler(nwksp, ifname, cb, (uint8 *)eth,
						&bytes, (struct ether_addr *)&eth->ether_shost);
				}
			}
		}
#endif /* BCM_ASPMD */

#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
			/* check visdcoll appSocket */
			if (visdcoll->appSocket >= 0 &&
				FD_ISSET(visdcoll->appSocket, &fdset)) {
				bytes = recv(visdcoll->appSocket, pkt, len, 0);
				if (bytes > ETHER_HDR_LEN) {
					char *ifname = (char *) pkt;
					struct ether_header *eth;
#ifdef BCMDBG
					HEXDUMP_ASCII("EAPD::data from visdcoll app", pkt, bytes);
#endif // endif
					/* ether header */
					eth = (struct ether_header*)(ifname + IFNAMSIZ);
					bytes -= IFNAMSIZ;
					cb = eapd_wksp_find_cb(nwksp, visdcoll, ifname, eth->ether_shost);
					/* send message data out */
					if (cb) {
						visdcoll_app_recv_handler(nwksp, cb,
							(uint8 *)eth, &bytes);
					}
				}
			}
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */

#ifdef BCM_CEVENT
		/* check cevent appSocket */
		if (cevent->appSocket >= 0 &&
		     FD_ISSET(cevent->appSocket, &fdset)) {
			bytes = recv(cevent->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;
#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from CEVENT app", pkt, bytes);
#endif // endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

				cb = eapd_wksp_find_cb(nwksp, cevent, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb) {
					cevent_app_recv_handler(nwksp, ifname, cb,
							(uint8 *)eth, &bytes);
				}
			}
		}
#endif /* BCM_CEVENT */

#ifdef BCM_RGD
		/* check rgd appSocket */
		if (rgd->appSocket >= 0 &&
		     FD_ISSET(rgd->appSocket, &fdset)) {
			bytes = recv(rgd->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;
#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from rgd app", pkt, bytes);
#endif // endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

				cb = eapd_wksp_find_cb(nwksp, rgd, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb) {
				  rgd_app_recv_handler(nwksp, cb, (uint8 *)eth, &bytes);
				}
			}
		}
#endif /* BCM_RGD */

#ifdef BCM_WBD
		/* check wbd appSocket */
		if (wbd->appSocket >= 0 &&
		     FD_ISSET(wbd->appSocket, &fdset)) {
			bytes = recv(wbd->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;
#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from WBD app", pkt, bytes);
#endif // endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

				cb = eapd_wksp_find_cb(nwksp, wbd, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb) {
					wbd_app_recv_handler(nwksp, ifname, cb, (uint8 *)eth,
						&bytes, (struct ether_addr *)&eth->ether_shost);
				}
			}
		}
#endif /* BCM_WBD */

#ifdef BCM_WLEVENT
	/* check wlevent appSocket */
	if (wlevent->appSocket >= 0 &&
		FD_ISSET(wlevent->appSocket, &fdset)) {
			bytes = recv(wlevent->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;
#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from WLEVENT app", pkt, bytes);
#endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

				cb = eapd_wksp_find_cb(nwksp, wlevent, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb)
					wlevent_app_recv_handler(nwksp, cb, (uint8 *)eth, &bytes);
		}
	}
#endif /* BCM_WLEVENT */

#ifdef BCM_WLCEVENTD
		/* check wlceventd appSocket */
		if (wlceventd->appSocket >= 0 &&
		     FD_ISSET(wlceventd->appSocket, &fdset)) {
			bytes = recv(wlceventd->appSocket, pkt, len, 0);
			if (bytes > ETHER_HDR_LEN) {
				char *ifname = (char *) pkt;
				struct ether_header *eth;
#ifdef BCMDBG
				HEXDUMP_ASCII("EAPD:: data from WLCEVENTD app", pkt, bytes);
#endif
				/* ether header */
				eth = (struct ether_header*)(ifname + IFNAMSIZ);
				bytes -= IFNAMSIZ;

				cb = eapd_wksp_find_cb(nwksp, wlceventd, ifname, eth->ether_shost);
				/* send message data out. */
				if (cb)
					wlceventd_app_recv_handler(nwksp, cb, (uint8 *)eth, &bytes);
			}
		}
#endif /* BCM_WLCEVENTD */
	}

	return;
}

int
eapd_wksp_main_loop(eapd_wksp_t *nwksp)
{
	int ret;
	char *argv[] = {"/bin/eapd"};

	/* init eapd */
	ret = eapd_wksp_init(nwksp);

	/* eapd wksp initialization finished */
	eapd_wksp_inited = 1;
	if (ret) {
		EAPD_ERROR("Unable to initialize EAPD. Quitting...\n");
		eapd_wksp_cleanup(nwksp);
		return -1;
	}

#if !defined(DEBUG)
	if (!nwksp->foreground) {
	    /* Daemonize */
	    if (daemon(1, 1) == -1) {
		eapd_wksp_cleanup(nwksp);
		perror("eapd_wksp_main_loop: daemon\n");
		exit(errno);
	    }
	}
#endif // endif
	/* Provide necessary info to debug_monitor for service restart */
#if 0
	dm_register_app_restart_info(getpid(), 1, argv, NULL);
#endif
	while (1) {
		/* check user command for shutdown */
		if (nwksp->flags & EAPD_WKSP_FLAG_SHUTDOWN) {
			eapd_wksp_cleanup(nwksp);
			EAPD_INFO("NAS shutdown...\n");
			return 0;
		}

#ifdef EAPDDUMP
		/* check dump */
		if (nwksp->flags & EAPD_WKSP_FLAG_DUMP) {
			eapd_dump(nwksp);
			nwksp->flags &= ~EAPD_WKSP_FLAG_DUMP;
		}
#endif // endif

		/* do packets dispatch */
		eapd_wksp_dispatch(nwksp);
	}
}

static int
eapd_brcm_dispatch(eapd_wksp_t *nwksp, eapd_brcm_socket_t *from, uint8 *pData, int Len)
{
	if (nwksp == NULL || pData == NULL) {
		EAPD_ERROR("Wrong arguments...\n");
		return -1;
	}

	/* check nas application */
	nas_app_handle_event(nwksp, pData, Len, from->ifname);

	/* check wps application */
	wps_app_handle_event(nwksp, pData, Len, from->ifname);

#ifdef BCMWAPI_WAI
	/* check wai application */
	wai_app_handle_event(nwksp, pData, Len, from->ifname);
#endif /* BCMWAPI_WAI */

#ifdef BCM_CUSTOM_EVENT
	evt_app_handle_event(nwksp, pData, Len, from->ifname);
#endif // endif

#ifdef BCM_DCS
	/* check dcs application */
	dcs_app_handle_event(nwksp, pData, Len, from->ifname);
#endif /* BCM_DCS */

#ifdef BCM_MEVENT
	/* check mevent application */
	mevent_app_handle_event(nwksp, pData, Len, from->ifname);
#endif /* BCM_MEVENT */

#ifdef BCM_BSD
	/* check bsd application */
	bsd_app_handle_event(nwksp, pData, Len, from->ifname);
#endif /* BCM_BSD */

#ifdef BCM_DRSDBD
	/* check drsdbd application */
	drsdbd_app_handle_event(nwksp, pData, Len, from->ifname);
#endif /* BCM_DRSDBD */

#ifdef BCM_SSD
	/* check ssd application */
	ssd_app_handle_event(nwksp, pData, Len, from->ifname);
#endif /* BCM_SSD */

#ifdef BCM_EVENTD
	/* check eventd application */
	eventd_app_handle_event(nwksp, pData, Len, from->ifname);
#endif /* BCM_EVENTD */

#ifdef BCM_ECBD
	/* check ecbd application */
	ecbd_app_handle_event(nwksp, pData, Len, from->ifname);
#endif /* BCM_ECBD */

#ifdef BCM_ASPMD
	/* check aspm application */
	aspm_app_handle_event(nwksp, pData, Len, from->ifname);
#endif /* BCM_ASPMD */

#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
	visdcoll_app_handle_event(nwksp, pData, Len, from->ifname);
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */

#ifdef BCM_CEVENT
	/* Driver generated brcmevent dispatched to all the apps to check if they
	 * are interested in this event. No clue which apps are interested in this
	 * event. Hence send CEVENT_ANY in below function.
	 */
	cevent_app_handle_event(nwksp, pData, Len, from->ifname, TRUE, CEVENT_ANY);
#endif /* BCM_CEVENT */

#ifdef BCM_RGD
	/* check RGD application */
	rgd_app_handle_event(nwksp, pData, Len, from->ifname);
#endif /* BCM_RGD */

#ifdef BCM_WBD
	/* check WBD application */
	wbd_app_handle_event(nwksp, pData, Len, from->ifname);
#endif /* BCM_WBD */

#ifdef BCM_WLEVENT
	/* check wlevent application */
	wlevent_app_handle_event(nwksp, pData, Len, from->ifname);
#endif /* BCM_WLEVENT */

#ifdef BCM_WLCEVENTD
	/* check wlceventd application */
	wlceventd_app_handle_event(nwksp, pData, Len, from->ifname);
#endif /* BCM_WLCEVENTD */

	return 0;
}

/*
 * look for a raw brcm event socket connected to
 * interface "ifname".
 */
eapd_brcm_socket_t*
eapd_find_brcm(eapd_wksp_t *nwksp, char *ifname)
{
	int i, brcmSocketCount;
	eapd_brcm_socket_t *brcmSocket;

	if (nwksp == NULL || ifname == NULL) {
		EAPD_ERROR("Wrong arguments...\n");
		return NULL;
	}

	brcmSocketCount = nwksp->brcmSocketCount;
	for (i = 0; i < brcmSocketCount; i++) {
		brcmSocket = &nwksp->brcmSocket[i];
		if (brcmSocket->inuseCount > 0) {
			if (!strcmp(brcmSocket->ifname, ifname)) {
				EAPD_INFO("Find brcm interface %s\n", ifname);
				return brcmSocket;
			}
		}
	}

	EAPD_INFO("Not found brcm interface %s\n", ifname);
	return NULL;
}

eapd_brcm_socket_t*
eapd_add_brcm(eapd_wksp_t *nwksp, char *ifname)
{
	int i;
	eapd_brcm_socket_t *brcmSocket;

	if (nwksp == NULL || ifname == NULL) {
		EAPD_ERROR("Wrong arguments...\n");
		return NULL;
	}

	brcmSocket = eapd_find_brcm(nwksp, ifname);
	if (brcmSocket) {
		brcmSocket->inuseCount++;
		return brcmSocket;
	}

	/* not found, find inuseCount is zero first */
	for (i = 0; i < nwksp->brcmSocketCount; i++) {
		brcmSocket = &nwksp->brcmSocket[i];
		if (brcmSocket->inuseCount == 0)
			break;
	}

	 /* not found inuseCount is zero */
	if (i == nwksp->brcmSocketCount) {
		if (nwksp->brcmSocketCount >= EAPD_WKSP_MAX_NO_BRCM) {
			EAPD_ERROR("brcmSocket number is not enough, max %d\n",
				EAPD_WKSP_MAX_NO_BRCM);
			return NULL;
		}
		nwksp->brcmSocketCount++;
	}

	brcmSocket = &nwksp->brcmSocket[i];

	memset(brcmSocket, 0, sizeof(eapd_brcm_socket_t));
	strncpy(brcmSocket->ifname, ifname, IFNAMSIZ -1);
	if (eapd_brcm_open(nwksp, brcmSocket) < 0) {
		EAPD_ERROR("open brcm socket on %s error!!\n", ifname);
		return NULL;
	}

	return brcmSocket;
}

int
eapd_del_brcm(eapd_wksp_t *nwksp, eapd_brcm_socket_t *sock)
{
	int i;
	eapd_brcm_socket_t *brcmSocket;

	if (nwksp == NULL || sock == NULL) {
		EAPD_ERROR("Wrong arguments...\n");
		return -1;
	}

	/* find it first. */
	for (i = 0; i < nwksp->brcmSocketCount; i++) {
		brcmSocket = &nwksp->brcmSocket[i];
		if (brcmSocket->inuseCount > 0 &&
		     !strcmp(brcmSocket->ifname, sock->ifname)) {
			EAPD_INFO("Find brcm interface %s, brcmSocket->inuseCount=%d\n",
				sock->ifname, brcmSocket->inuseCount);
			brcmSocket->inuseCount--;
			if (brcmSocket->inuseCount == 0) {
				EAPD_INFO("close brcm drvSocket %d\n", brcmSocket->drvSocket);
				eapd_brcm_close(brcmSocket->drvSocket);
				brcmSocket->drvSocket = -1;
			}
			return 0;
		}
	}

	/* not found */
	EAPD_INFO("Not found brcm interface to del %s\n", sock->ifname);
	return -1;
}

/* dispatch EAPOL packet from brcmevent. */
static void
eapd_eapol_dispatch(eapd_wksp_t *nwksp, eapd_brcm_socket_t *from, uint8 *pData, int *pLen)
{
	eapd_sta_t *sta;
	eapol_header_t *eapol;
	eap_header_t *eap;
	char *ifname;

#ifdef BCM_CEVENT
	int eapol_start = 0;
	uint32 dest_app = 0; /* bitmask of application(s) interested in receving the EAPOL packet */
	eapd_cb_t *cb = NULL;
#endif /* BCM_CEVENT */

	if (!nwksp || !from || !pData) {
		EAPD_ERROR("Wrong argument...\n");
		return;
	}

#ifdef BCM_CEVENT
	if ((nwksp->cevent.cb) == NULL) {
		EAPD_ERROR("cevent cb is NULL...\n");
		return;
	}
	cb = nwksp->cevent.cb;
#endif /* BCM_CEVENT */

#ifdef BCMDBG
	HEXDUMP_ASCII("EAPD:: eapol data from BRCM driver", pData, *pLen);
#endif // endif

	/* incoming ifname */
	ifname = (char *) pData;

	/* eapol header */
	eapol = (eapol_header_t *)(ifname + IFNAMSIZ);
	eap = (eap_header_t *) eapol->body;

#ifdef NAS_GTK_PER_STA
	if (ETHER_ISNULLADDR(eapol->eth.ether_shost) && (eapol->type == 0xFF)) {
		nas_app_sendup(nwksp, pData, *pLen, from->ifname);

#ifdef BCM_CEVENT
		if (eapd_ceventd_enable) {
			dest_app |= CEVENT_NAS;
			cevent_generic_hub(nwksp, ifname, cb, (uint8 *)eapol,
				(*pLen - IFNAMSIZ), TRUE, dest_app);
		}
#endif /* BCM_CEVENT */
		return;
	}
#endif /* NAS_GTK_PER_STA */

	sta = sta_lookup(nwksp, (struct ether_addr *) eapol->eth.ether_shost,
		(struct ether_addr *) eapol->eth.ether_dhost, ifname, EAPD_SEARCH_ENTER);
	if (!sta) {
		EAPD_ERROR("no STA struct available\n");
		return;
	}

	if (eapol->version < sta->eapol_version) {
		EAPD_ERROR("EAPOL version %d packet received, current version is %d\n",
		            eapol->version, sta->eapol_version);
	}

	EAPD_INFO("sta->pae_state=%d sta->mode=%d\n", sta->pae_state, sta->mode);

	/* Pass EAP_RESPONSE to WPS module. */
	if (sta->pae_state >= EAPD_IDENTITY &&
	    (sta->mode == EAPD_STA_MODE_WPS || sta->mode == EAPD_STA_MODE_WPS_ENR) &&
	    eapol->type == EAP_PACKET &&
	    (eap->type == EAP_EXPANDED || eap->code == EAP_FAILURE)) {
		switch (eap->code) {
		case EAP_REQUEST:
		case EAP_FAILURE:
			/* in case of router running enr */
			EAPD_INFO("EAP %s Packet received...\n",
				eap->code == EAP_REQUEST ? "Request" : "Failure");

			/* Send to wps-monitor */
			if (sta->mode == EAPD_STA_MODE_WPS_ENR) {
				/* monitor eapol packet */
				if (eap->code == EAP_FAILURE || eap->code == EAP_SUCCESS) {
					sta_remove(nwksp, sta);
				}
#ifdef BCMDBG
				HEXDUMP_ASCII("Receive, EAP Request", pData, *pLen);
#endif // endif
				wps_app_monitor_sendup(nwksp, pData, *pLen, from->ifname);

#ifdef BCM_CEVENT
				dest_app |= CEVENT_WPS;
#endif /* BCM_CEVENT */

			}
			else
				return;
			break;

		case EAP_RESPONSE:
			EAPD_INFO("EAP Response Packet received...\n");

			/* Send to wps-monitor */
			if (sta->mode == EAPD_STA_MODE_WPS) {
				wps_app_monitor_sendup(nwksp, pData, *pLen, from->ifname);

#ifdef BCM_CEVENT
				dest_app |= CEVENT_WPS;
#endif /* BCM_CEVENT */
			} else {
				return;
			}
			break;

		default:
			/* Do nothing */
			break;
		}
#ifdef BCM_CEVENT
		if (dest_app && eapd_ceventd_enable) {
			cevent_generic_hub(nwksp, ifname, cb, (uint8 *)eapol,
					(*pLen - IFNAMSIZ), TRUE, dest_app);
		}

#endif /* BCM_CEVENT */
		return;
	}

	/* We handle
	  * 1. Receive EAPOL-Start and send EAP_REQUEST for EAP_IDENTITY
	  * 2. Receive EAP_IDENTITY and pass it to NAS if not a WPS IDENTITY,
	  *     NAS need to record identity.
	  * 3. Pass NAS other EAPOL type
	  */
	switch (eapol->type) {
	case EAP_PACKET:
		if (ntohs(eapol->length) >= (EAP_HEADER_LEN + 1)) {
			EAPD_INFO("STA State=%d EAP Packet Type=%d Id=%d code=%d\n",
				sta->pae_state, eap->type, eap->id, eap->code);

			switch (eap->type) {
			case EAP_IDENTITY:
				EAPD_INFO("Receive , eap code=%d, id = %d, length=%d, type=%d\n",
					eap->code, eap->id, ntohs(eap->length), eap->type);
#ifdef BCMDBG
				HEXDUMP_ASCII("Receive, EAP Identity", eap->data,
					ntohs(eap->length) - EAP_HEADER_LEN - 1);
#endif // endif
				/* Store which interface sta come from */
				memcpy(&sta->bssid, &eapol->eth.ether_dhost, ETHER_ADDR_LEN);
				memcpy(&sta->ifname, ifname, IFNAMSIZ);

				if (eapd_get_method(eap->data) == EAP_TYPE_WSC) {
					EAPD_INFO("This is a wps eap identity response!\n");

					/* Send to WPS-Monitor module. */
					wps_app_monitor_sendup(nwksp, pData, *pLen, from->ifname);
#ifdef BCM_CEVENT
					dest_app |= CEVENT_WPS;
#endif /* BCM_EVENT */

					sta->mode = EAPD_STA_MODE_WPS;
				} else if (sta->mode == EAPD_STA_MODE_WPS_ENR) {
					EAPD_INFO("This is a wps eap identity request!\n");

					/* Send to WPS-Monitor module. */
					wps_app_monitor_sendup(nwksp, pData, *pLen, from->ifname);
#ifdef BCM_CEVENT
					dest_app |= CEVENT_WPS;
#endif /* BCM_EVENT */

					/*
					  * sta mode EAPD_STA_MODE_WPS_ENR set in
					  * wps_app_recv_handler, when enr initial send
					  * EAPOL-START.
					  */
				}
				else {
					/* Send to NAS module. */
					nas_app_sendup(nwksp, pData, *pLen, from->ifname);
					sta->mode = EAPD_STA_MODE_NAS;
#ifdef BCM_CEVENT
					dest_app |= CEVENT_NAS;
#endif /* BCM_EVENT */
				}
				sta->pae_state = EAPD_IDENTITY;
				break;
/*
#if 1
*/
#ifdef __CONFIG_WFI__
			case EAP_NAK:
				if (sta->mode == EAPD_STA_MODE_UNKNOW &&
				    sta->pae_state == EAPD_INITIALIZE &&
				    eap->code == EAP_RESPONSE) {
					/*
					 * EAP-RESPONSE-NAK for WFI reject.
					 * Send to WPS-Monitor module.
					 */
					wps_app_monitor_sendup(nwksp, pData, *pLen, from->ifname);
#ifdef BCM_CEVENT
					dest_app |= CEVENT_WPS;
#endif /* BCM_EVENT */

					break;
				}
				/* Fall through */
#endif /* __CONFIG_WFI__ */
/* #endif */

			default:
				/* Send to NAS module. */
				nas_app_sendup(nwksp, pData, *pLen, from->ifname);
				sta->mode = EAPD_STA_MODE_UNKNOW;
#ifdef BCM_CEVENT
				dest_app |= CEVENT_NAS;
#endif /* BCM_EVENT */

				break;
			}
		}
		break;

	case EAPOL_START:
		EAPD_INFO("EAPOL Start\n");

#ifdef BCM_CEVENT
		if (eapd_ceventd_enable) {
			dest_app |= CEVENT_EAPD;  /* EAPOL Start packet is consumed by EAPD */
			/* Forward EAPOL Start to cevent and set eapol_start to 1 */
			cevent_generic_hub(nwksp, ifname, cb, (uint8 *)eapol, (*pLen - IFNAMSIZ),
					TRUE, dest_app);
			eapol_start = 1;
		}
#endif /* BCM_CEVENT */
		sta->pae_id = 0;
		memcpy(&sta->bssid, &eapol->eth.ether_dhost, ETHER_ADDR_LEN);
		memcpy(&sta->ifname, ifname, IFNAMSIZ);

		/* check EAPD interface application capability first */
		if (!eapd_valid_eapol_start(nwksp, from, ifname))
			break;

		/* break out if STA is only PSK */
		if (sta->mode == EAPD_STA_MODE_NAS_PSK)
			break;

		eapd_eapol_canned_send(nwksp, from, sta, EAP_REQUEST, EAP_IDENTITY);
		sta->mode = EAPD_STA_MODE_UNKNOW;
		break;

	case EAPOL_LOGOFF:
		EAPD_INFO("EAPOL Logoff sta mode %d\n", sta->mode);

		/* Send EAPOL_LOGOFF to application. */
		switch (sta->mode) {
		case EAPD_STA_MODE_WPS:
		case EAPD_STA_MODE_WPS_ENR:
			wps_app_monitor_sendup(nwksp, pData, *pLen, from->ifname);
#ifdef BCM_CEVENT
			dest_app |= CEVENT_WPS;
#endif /* BCM_CEVENT */
			break;

		case EAPD_STA_MODE_NAS:
		case EAPD_STA_MODE_NAS_PSK:
			nas_app_sendup(nwksp, pData, *pLen, from->ifname);
#ifdef BCM_CEVENT
			dest_app |= CEVENT_NAS;
#endif /* BCM_CEVENT */
			break;

		default:
			EAPD_INFO("Ignore EAPOL Logoff\n");
			break;
		}
		break;

	default:
		EAPD_INFO("unknown EAPOL type %d\n", eapol->type);

		/* Send to NAS module. */
		nas_app_sendup(nwksp, pData, *pLen, from->ifname);
#ifdef BCM_CEVENT
		dest_app |= CEVENT_NAS;
#endif /* BCM_CEVENT */
		break;
	}
#ifdef BCM_CEVENT
	if (!eapol_start) {
		/* Not an EAPOL Start Pkt. EAPOL Start pkt is already forwarded to cevent up above.
		 * Only other EAPOL pkts should be forwarded to cevent.
		 */
		if (eapd_ceventd_enable) {
			cevent_generic_hub(nwksp, ifname, cb, (uint8 *)eapol, (*pLen - IFNAMSIZ),
					TRUE, dest_app);
		}
	}
#endif /* BCM_CEVENT */

	return;
}

#ifdef BCMWAPI_WAI
/* dispatch WAI packet from brcmevent. Just pass to wai application */
static void
eapd_wai_dispatch(eapd_wksp_t *nwksp, eapd_brcm_socket_t *from, uint8 *pData, int *pLen)
{
	eapd_sta_t *sta;
	struct ether_header *eth;
	char *ifname = pData;
#ifdef BCM_CEVENT
	eapd_cb_t *cb = NULL;
#endif /* BCM_CEVENT */

	if (!nwksp || !from || !pData) {
		EAPD_ERROR("Wrong argument...\n");
		return;
	}

#ifdef BCM_CEVENT
	if ((nwksp->cevent.cb) == NULL) {
		EAPD_ERROR("cevent cb NULL...\n");
		return;
	}
	cb = nwksp->cevent.cb;
#endif /* BCM_CEVENT */

#ifdef BCMDBG
	HEXDUMP_ASCII("EAPD:: wai(+eth header) data from BRCM driver", pData, *pLen);
#endif // endif

	/* Ether header */
	eth = (struct ether_header *)(ifname + IFNAMSIZ);

	sta = sta_lookup(nwksp, (struct ether_addr *)eth->ether_shost,
		(struct ether_addr *)eth->ether_dhost, ifname, EAPD_SEARCH_ENTER);
	if (!sta) {
		EAPD_ERROR("no STA struct available\n");
		return;
	}

	EAPD_INFO("sta->mode=%d\n", sta->mode);

	/* Send to WAI module. */
	wai_app_sendup(nwksp, pData, *pLen, from->ifname);

#ifdef BCM_CEVENT
	cevent_generic_hub(nwksp, from->ifname, cb, (uint8 *)eapol, len, TRUE, CEVENT_WAI);
#endif /* BCM_CEVENT */
	return;
}
#endif /* BCMWAPI_WAI */

/* Handle brcmevent type packet from any interface */
void
eapd_brcm_recv_handler(eapd_wksp_t *nwksp, eapd_brcm_socket_t *from, uint8 *pData, int *pLen)
{
	eapol_header_t *eapol;
	bcm_event_t *dpkt = (bcm_event_t *)pData;
	unsigned int len;
	char *ifname, ifname_tmp[BCM_MSG_IFNAME_MAX];

#ifdef BCMWAPI_WAI
	struct ether_header *eth;
#endif /* BCMWAPI_WAI */

	if (nwksp == NULL || from == NULL || dpkt == NULL) {
		EAPD_ERROR("Wrong argument...\n");
		return;
	}

	switch (ntohs(dpkt->bcm_hdr.usr_subtype)) {
	case BCMILCP_BCM_SUBTYPE_EVENT:

		switch (ntohl(dpkt->event.event_type)) {
		case WLC_E_EAPOL_MSG:
			EAPD_INFO("%s: recved wl eapol packet in brcmevent bytes: %d\n",
			       dpkt->event.ifname, *pLen);

			len = ntohl(dpkt->event.datalen);

			/* Reconstructs a EAPOL packet from the received packet	*/
			/* Point the EAPOL packet to the start of the data portion of the
			  * received packet minus some space to add the ethernet header to the
			  * EAPOL packet
			  */
			eapol = (eapol_header_t *)((char *)(dpkt + 1) - ETHER_HDR_LEN);
			ifname = (char *)eapol - BCM_MSG_IFNAME_MAX;

			/* Save incoming interface name to temp_ifname */
			bcopy(dpkt->event.ifname, ifname_tmp, BCM_MSG_IFNAME_MAX);

			/* Now move the received packet's ethernet header to the head of the
			  * EAPOL packet
			  */
			memmove((char *)eapol, (char *)pData, ETHER_HDR_LEN);

			/* Set the EAPOL packet type correctly */
			eapol->eth.ether_type = htons(ETHER_TYPE_802_1X);

			/* The correct shost address was encapsulated to the event struct by the
			  * driver, copy it to the EAPOL packet's ethernet header
			  */
			bcopy(dpkt->event.addr.octet, eapol->eth.ether_shost, ETHER_ADDR_LEN);

			/* Save incoming interface name */
			bcopy(ifname_tmp, ifname, BCM_MSG_IFNAME_MAX);

			len = len + ETHER_HDR_LEN + BCM_MSG_IFNAME_MAX;
			eapd_eapol_dispatch(nwksp, from, (void *)ifname, (int *)&len);
			return;
#ifdef BCMWAPI_WAI
		case WLC_E_WAI_MSG:
			EAPD_INFO("%s: recved wl wai packet in brcmevent bytes: %d\n",
			       dpkt->event.ifname, *pLen);

			len = ntohl(dpkt->event.datalen);

			eth = (struct ether_header*)((char *)(dpkt + 1) - ETHER_HDR_LEN);
			ifname = (char *)eth - BCM_MSG_IFNAME_MAX;

			/* Save incoming interface name to temp_ifname */
			bcopy(dpkt->event.ifname, ifname_tmp, BCM_MSG_IFNAME_MAX);

			/* Move the received packet's ethernet header to the append header */
			memmove((char *)eth, (char *)pData, ETHER_HDR_LEN);

			/* Set the WAI packet type correctly */
			eth->ether_type = htons(ETHER_TYPE_WAI);

			/*
			 * The correct shost address was encapsulated to the event struct by the
			 * driver, copy it to the packet's ethernet header
			 */
			bcopy(dpkt->event.addr.octet, eth->ether_shost, ETHER_ADDR_LEN);

			/* Save incoming interface name */
			bcopy(ifname_tmp, ifname, BCM_MSG_IFNAME_MAX);

			len = len + ETHER_HDR_LEN + BCM_MSG_IFNAME_MAX;
			/* pass (ifname) + (ether header) + (WAI) */
			eapd_wai_dispatch(nwksp, from, (void *)ifname, &len);
			return;
#endif /* BCMWAPI_WAI */

		default:
			/* dispatch brcnevent to wps, nas, wapid */
			EAPD_INFO("%s: dispatching brcmevent %d to nas/wps...\n",
			          dpkt->event.ifname, ntohl(dpkt->event.event_type));

			/* Driver generated brcmevent. Dispatch to wps, nas
			 * cevent etc. Eventually calls cevent_generic_hub
			 */
			eapd_brcm_dispatch(nwksp, from, pData, *pLen);
			return;
		}
		break;

	default: /* not a NAS supported message so return an error */
		EAPD_ERROR("%s: ERROR: recved unknown packet interface subtype "
		        "0x%x bytes: %d\n", dpkt->event.ifname,
		        ntohs(dpkt->bcm_hdr.usr_subtype), *pLen);
		return;
	}

	return;
}

/* Handle PreAuth packet from any interface for nas
 * Receive it and just pass to nas
 */
void
eapd_preauth_recv_handler(eapd_wksp_t *nwksp, char *from, uint8 *pData, int *pLen)
{
#ifdef BCM_CEVENT
	eapd_cb_t *cb = NULL;
#endif /* BCM_CEVENT */
	if (!nwksp || !from || !pData) {
		EAPD_ERROR("Wrong argument...\n");
		return;
	}

#ifdef BCM_CEVENT
	if (nwksp->cevent.cb == NULL) {
		EAPD_ERROR("cevent cb is NULL...\n");
		return;
	}
	cb = nwksp->cevent.cb;
#endif /* BCM_ CEVENT */

	/* prepend ifname,  we reserved IFNAMSIZ length already */
	/*
	 * XXX,PR77030 the "from" may not a exactly receive interface for example "br0".
	 * But in fact, we want a exactly receive interface, so the upper application
	 * need to handle it. For example, just use DA w/o ifname to get nwcb.
	*/
	pData -= IFNAMSIZ;
	*pLen += IFNAMSIZ;
	memcpy(pData, from, IFNAMSIZ);

	/* Do not parse it right now, just pass to NAS */
	nas_app_sendup(nwksp, pData, *pLen, from);

#ifdef BCM_CEVENT
	/* origin from driver to NAS, forward to cevent */
	cevent_generic_hub(nwksp, NULL, cb, pData, *pLen, TRUE, CEVENT_NAS);
#endif /* BCM_CEVENT */

	return;
}

static unsigned char
eapd_get_method(unsigned char *user)
{
	unsigned char ret = 0;
	int i;

	for (i = 0; i < EAPD_WKSP_EAP_USER_NUM; i++) {
		if (memcmp(user, eapdispuserlist[i].identity,
			eapdispuserlist[i].identity_len) == 0) {
			ret = eapdispuserlist[i].method;
			break;
		}
	}

	return ret;
}

static int
sta_init(eapd_wksp_t *nwksp)
{
	return 0;
}

static int
sta_deinit(eapd_wksp_t *nwksp)
{
	return 0;
}

void
sta_remove(eapd_wksp_t *nwksp, eapd_sta_t *sta)
{
	eapd_sta_t *sta_list;
	uint hash;

	EAPD_INFO("sta %s remove\n", ether_etoa((uchar *)&sta->ea, eabuf));

	if (sta == NULL) {
		EAPD_ERROR("called with NULL STA ponter\n");
		return;
	}

	/* Remove this one from its hashed list. */
	hash = EAPD_PAE_HASH(&sta->ea);
	sta_list = nwksp->sta_hashed[hash];

	if (sta_list == sta) {
		/* It was the head, so its next is the new head. */
		nwksp->sta_hashed[hash] = sta->next;

	}
	else {
		/* Find the one that points to it and change the pointer. */
		while ((sta_list != NULL) && (sta_list->next != sta))
			sta_list = sta_list->next;
		if (sta_list == NULL) {
			EAPD_INFO("sta %s not in hash list\n",
				ether_etoa((uchar *)&sta->ea, eabuf));
		}
		else {
			sta_list->next = sta->next;
		}
	}
	sta->used = FALSE;
	return;
}

/*
 * Search for or create a STA struct.
 * If `mode' is not EAPD_SEARCH_ENTER, do not create it when one is not found.
 * NOTE: bssid_ea is a spoof mac.
 */
eapd_sta_t*
sta_lookup(eapd_wksp_t *nwksp, struct ether_addr *sta_ea, struct ether_addr *bssid_ea,
               char *ifname, eapd_lookup_mode_t mode)
{
	unsigned int hash;
	eapd_sta_t *sta;
	time_t now, oldest;

	EAPD_INFO("lookup for sta %s\n", ether_etoa((uchar *)sta_ea, eabuf));

	hash = EAPD_PAE_HASH(sta_ea);

	/* Search for entry in the hash table */
	for (sta = nwksp->sta_hashed[hash];
	     sta && memcmp(&sta->ea, sta_ea, ETHER_ADDR_LEN);
	     sta = sta->next);

	/* One second resolution is probably good enough. */
	(void) time(&now);

	/* Not found in sta_hashed, allocate a new entry */
	if (!sta) {
		int i, old_idx = -1;

		/* Don't make an unwanted entry. */
		if (mode == EAPD_SEARCH_ONLY)
			return NULL;

		oldest = now;
		for (i = 0; i < EAPD_WKSP_MAX_SUPPLICANTS; i++) {
			if (!nwksp->sta[i].used)
				break;
			else if (nwksp->sta[i].last_use < oldest) {
				oldest = nwksp->sta[i].last_use;
				old_idx = i;
			}
		}

		if (i < EAPD_WKSP_MAX_SUPPLICANTS) {
			sta = &nwksp->sta[i];
		}
		else if (old_idx == -1) {
			/* Full up with all the same timestamp? Can
			 * this really happen?
			 */
			return NULL;
		}
		else {
			/* Didn't find one unused, so age out LRU not wps entry. */
			sta = &nwksp->sta[old_idx];
			sta_remove(nwksp, sta);
		}

		/* Initialize entry */
		memset(sta, 0, (sizeof(eapd_sta_t)));
		memcpy(&sta->ea, sta_ea, ETHER_ADDR_LEN);
		memcpy(&sta->bssid, bssid_ea, ETHER_ADDR_LEN);
		memcpy(&sta->ifname, ifname, IFNAMSIZ);
		sta->used = TRUE;
		sta->eapol_version = WPA_EAPOL_VERSION;

		/* Initial STA state */
		sta->pae_state = EAPD_INITIALIZE;

		/* initial mode */
		sta->mode = EAPD_STA_MODE_UNKNOW;
		EAPD_INFO("Create eapd sta %s\n", ether_etoa((uchar *)&sta->ea, eabuf));

		/* Add entry to the cache */
		sta->next = nwksp->sta_hashed[hash];
		nwksp->sta_hashed[hash] = sta;
	}
	else if (bssid_ea &&
			!eapd_wl_wlif_is_psta(nwksp, sta->ifname) &&
			!eapd_wl_wlif_is_psta(nwksp, ifname) &&
			(memcmp(&sta->bssid, bssid_ea, ETHER_ADDR_LEN) ||
			strcmp(sta->ifname, ifname))) {
		/* from different wl */
		memcpy(&sta->bssid, bssid_ea, ETHER_ADDR_LEN);
		memcpy(&sta->ifname, ifname, IFNAMSIZ);

		/* Initial STA state */
		sta->pae_state = EAPD_INITIALIZE;

		/* initial mode */
		sta->mode = EAPD_STA_MODE_UNKNOW;
		EAPD_INFO("sta %s come from changed.\n", ether_etoa((uchar *)&sta->ea, eabuf));
	}

	sta->last_use = now;
	return sta;
}

static int
event_init(eapd_wksp_t *nwksp)
{
	int i, ret, unit, wlunit[16];
	eapd_wps_t *wps;
	eapd_nas_t *nas;
#ifdef BCM_CUSTOM_EVENT
	eapd_evt_t *evt;
#endif // endif
#ifdef BCMWAPI_WAI
	eapd_wai_t *wai;
#endif /* BCMWAPI_WAI */
#ifdef BCM_DCS
	eapd_dcs_t *dcs;
#endif /* BCM_DCS */
#ifdef BCM_MEVENT
	eapd_mevent_t *mevent;
#endif /* BCM_MEVENT */
#ifdef BCM_BSD
	eapd_bsd_t *bsd;
#endif /* BCM_BSD */
#ifdef BCM_DRSDBD
	eapd_drsdbd_t *drsdbd;
#endif /* BCM_DRSDBD */
#ifdef BCM_SSD
	eapd_ssd_t *ssd;
#endif /* BCM_SSD */
#ifdef BCM_EVENTD
	eapd_eventd_t *eventd;
#endif /* BCM_EVENTD */
#ifdef BCM_ECBD
	eapd_ecbd_t *ecbd;
#endif /* BCM_ECBD */
#ifdef BCM_ASPMD
	eapd_aspm_t *aspm;
#endif /* BCM_ASPMD */
#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
	eapd_visdcoll_t *visdcoll;
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */
#ifdef BCM_CEVENT
	eapd_cevent_t *cevent;
#endif /* BCM_CEVENT */
#ifdef BCM_RGD
	eapd_wbd_t *rgd;
#endif /* BCM_RGD */
#ifdef BCM_WBD
	eapd_wbd_t *wbd;
#endif /* BCM_WBD */

#ifdef BCM_WLEVENT
	eapd_wlevent_t *wlevent;
#endif /* BCM_WLEVENT */
#ifdef BCM_WLCEVENTD
	eapd_wlceventd_t *wlceventd;
#endif /* BCM_WLCEVENTD */

	char name[IFNAMSIZ], os_name[IFNAMSIZ], all_ifnames[IFNAMSIZ * EAPD_WKSP_MAX_NO_IFNAMES];
	char *next;
	uchar bitvec[EAPD_WL_EVENTING_MASK_LEN];
	uchar buf_param[WLC_IOCTL_MAXLEN], buf[WLC_IOCTL_MAXLEN];
	eventmsgs_ext_t *eventmask;
	memset(wlunit, -1, sizeof(wlunit));
	memset(name, 0, sizeof(name));
	memset(all_ifnames, 0, sizeof(all_ifnames));
	wps = &nwksp->wps;
	nas = &nwksp->nas;
#ifdef BCM_CUSTOM_EVENT
	evt = &nwksp->evt;
#endif // endif
#ifdef BCMWAPI_WAI
	wai = &nwksp->wai;
#endif /* BCMWAPI_WAI */
#ifdef BCM_DCS
	dcs = &nwksp->dcs;
#endif /* BCM_DCS */
#ifdef BCM_MEVENT
	mevent = &nwksp->mevent;
#endif /* BCM_MEVENT */
#ifdef BCM_BSD
	bsd = &nwksp->bsd;
#endif /* BCM_BSD */
#ifdef BCM_DRSDBD
	drsdbd = &nwksp->drsdbd;
#endif /* BCM_DRSDBD */
#ifdef BCM_SSD
	ssd = &nwksp->ssd;
#endif /* BCM_SSD */
#ifdef BCM_EVENTD
	eventd = &nwksp->eventd;
#endif /* BCM_EVENTD */
#ifdef BCM_ECBD
	ecbd = &nwksp->ecbd;
#endif /* BCM_ECBD */
#ifdef BCM_ASPMD
	aspm = &nwksp->aspm;
#endif /* BCM_ASPMD */
#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
	visdcoll = &nwksp->visdcoll;
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */
#ifdef BCM_CEVENT
	cevent = &nwksp->cevent;
#endif /* BCM_CEVENT */
#ifdef BCM_RGD
	rgd = &nwksp->rgd;
#endif /* BCM_RGD */
#ifdef BCM_WBD
	wbd = &nwksp->wbd;
#endif /* BCM_WBD */
#ifdef BCM_WLEVENT
	wlevent = &nwksp->wlevent;
#endif /* BCM_WLEVENT */
#ifdef BCM_WLCEVENTD
	wlceventd = &nwksp->wlceventd;
#endif /* BCM_WLCEVENTD */

	/* add all application ifnames to all_ifnames */
	strcpy(all_ifnames, wps->ifnames);
#ifdef BCM_CUSTOM_EVENT
	memset(name, 0, sizeof(name));
	EAPD_INFO("%s@%d evt->ifname=%s\n", __FUNCTION__, __LINE__, evt->ifnames);
	foreach(name, evt->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif // endif
	foreach(name, nas->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#ifdef BCMWAPI_WAI
	memset(name, 0, sizeof(name));
	foreach(name, wai->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif /* BCMWAPI_WAI */
#ifdef BCM_DCS
	memset(name, 0, sizeof(name));
	foreach(name, dcs->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif /* BCM_DCS */
#ifdef BCM_MEVENT
	memset(name, 0, sizeof(name));
	foreach(name, mevent->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif /* BCM_MEVENT */
#ifdef BCM_BSD
	memset(name, 0, sizeof(name));
	foreach(name, bsd->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif /* BCM_BSD */
#ifdef BCM_DRSDBD
	memset(name, 0, sizeof(name));
	foreach(name, drsdbd->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif /* BCM_DRSDBD */
#ifdef BCM_SSD
	memset(name, 0, sizeof(name));
	foreach(name, ssd->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif /* BCM_SSD */

#ifdef BCM_EVENTD
	memset(name, 0, sizeof(name));
	foreach(name, eventd->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif /* BCM_EVENTD */

#ifdef BCM_ECBD
	memset(name, 0, sizeof(name));
	foreach(name, ecbd->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif /* BCM_ECBD */

#ifdef BCM_ASPMD
	memset(name, 0, sizeof(name));
	foreach(name, aspm->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif /* BCM_ASPMD */
#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
	memset(name, 0, sizeof(name));
	foreach(name, visdcoll->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */
#ifdef BCM_CEVENT
	memset(name, 0, sizeof(name));
	foreach(name, cevent->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif /* BCM_CEVENT */

#ifdef BCM_RGD
	memset(name, 0, sizeof(name));
	foreach(name, rgd->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif /* BCM_RGD */

#ifdef BCM_WBD
	memset(name, 0, sizeof(name));
	foreach(name, wbd->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif /* BCM_WBD */
#ifdef BCM_WLEVENT
	memset(name, 0, sizeof(name));
	foreach(name, wlevent->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif /* BCM_WLEVENT */
#ifdef BCM_WLCEVENTD
	memset(name, 0, sizeof(name));
	foreach(name, wlceventd->ifnames, next) {
		add_to_list(name, all_ifnames, sizeof(all_ifnames));
	}
#endif /* BCM_WLCEVENTD */

	/* check each name in all_ifnames */
	memset(name, 0, sizeof(name));
	foreach(name, all_ifnames, next) {
		/* apply bitvec to driver */
		memset(bitvec, 0, EAPD_WL_EVENTING_MASK_LEN);
		memset(os_name, 0, sizeof(os_name));
		if (nvifname_to_osifname(name, os_name, sizeof(os_name)) < 0)
			continue;
		if (wl_probe(os_name) ||
			wl_ioctl(os_name, WLC_GET_INSTANCE, &unit, sizeof(unit)))
			continue;

		/* get current bitvec value */
		memset(buf_param, 0, sizeof(buf_param));
		eventmask = (eventmsgs_ext_t *)buf_param;
		eventmask->ver = EVENTMSGS_VER;
		eventmask->command = EVENTMSGS_NONE;
		eventmask->len = EAPD_WL_EVENTING_MASK_LEN;
		ret = wl_iovar_getbuf(os_name, "event_msgs_ext",
			(void *)buf_param, EVENTMSGS_EXT_STRUCT_SIZE,
			(void *)buf, sizeof(buf));
		if (ret) {
			EAPD_ERROR("Get event_msg error %d on %s[%s]\n",
			           ret, name, os_name);
			continue;
		}

		eventmask = (eventmsgs_ext_t *)buf;
		memcpy(bitvec, eventmask->mask, eventmask->len);

		/* is wps have this name */
		if (find_in_list(wps->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= wps->bitvec[i];
			}
		}
		/* is nas have this name */
		if (find_in_list(nas->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= nas->bitvec[i];
			}
		}
#ifdef BCMWAPI_WAI
		/* is wai have this name */
		if (find_in_list(wai->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= wai->bitvec[i];
			}
		}
#endif /* BCMWAPI_WAI */

#ifdef BCM_DCS
		/* is dcs have this name */
		if (find_in_list(dcs->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= dcs->bitvec[i];
			}
		}
#endif /* BCM_DCS */

#ifdef BCM_CUSTOM_EVENT
		/* is evt have this name */
		if (find_in_list(evt->ifnames, name)) {
			for (i = 0; i < WL_EVENTING_MASK_LEN; i++) {
				EAPD_INFO("%s@%d bitvec[%d]=%x\n", __FUNCTION__, __LINE__,
					i, evt->bitvec[i]);

				bitvec[i] |= evt->bitvec[i];
			}
		}
#endif /* BCM_CUSTOM_EVENT */

#ifdef BCM_MEVENT
		/* is mevent have this name */
		if (find_in_list(mevent->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= mevent->bitvec[i];
			}
		}
#endif /* BCM_MEVENT */

#ifdef BCM_BSD
		/* is bsd have this name */
		if (find_in_list(bsd->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= bsd->bitvec[i];
			}
		}
#endif /* BCM_BSD */

#ifdef BCM_DRSDBD
		/* is bsd have this name */
		if (find_in_list(drsdbd->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= drsdbd->bitvec[i];
			}
		}
#endif /* BCM_DRSDBDD */

#ifdef BCM_SSD
		/* is ssd have this name */
		if (find_in_list(ssd->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= ssd->bitvec[i];
			}
		}
#endif /* BCM_SSD */

#ifdef BCM_EVENTD
		/* is eventd have this name */
		if (find_in_list(eventd->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= eventd->bitvec[i];
			}
		}
#endif /* BCM_EVENTD */

#ifdef BCM_ECBD
		/* is ecbd have this name */
		if (find_in_list(ecbd->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= ecbd->bitvec[i];
			}
		}
#endif /* BCM_ECBD */

#ifdef BCM_ASPMD
		/* is aspm have this name */
		if (find_in_list(aspm->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= aspm->bitvec[i];
			}
		}
#endif /* BCM_ASPMD */

#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
		/* is visdcoll have this name */
		if (find_in_list(visdcoll->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= visdcoll->bitvec[i];
			}
		}
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */

#ifdef BCM_CEVENT
		/* is cevent have this name */
		if (find_in_list(cevent->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= cevent->bitvec[i];
			}
		}
#endif /* BCM_CEVENT */

#ifdef BCM_RGD
		/* is rgd have this name */
		if (find_in_list(rgd->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= rgd->bitvec[i];
			}
		}
#endif /* BCM_RGD */

#ifdef BCM_WBD
		/* is wbd have this name */
		if (find_in_list(wbd->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= wbd->bitvec[i];
			}
		}
#endif /* BCM_WBD */

#ifdef BCM_WLEVENT
		/* is wlevent have this name */
		if (find_in_list(wlevent->ifnames, name)) {
			for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= wlevent->bitvec[i];
			}
		}
#endif /* BCM_WLEVENT */

#ifdef BCM_WLCEVENTD
		/* is wlceventd have this name */
		if (find_in_list(wlceventd->ifnames, name)) {
			for (i = 0; i < WL_EVENTING_MASK_LEN; i++) {
				bitvec[i] |= wlceventd->bitvec[i];
			}
		}
#endif /* BCM_WLCEVENTD */

		memset(buf_param, 0, sizeof(buf_param));
		eventmask = (eventmsgs_ext_t*)buf_param;
		eventmask->ver = EVENTMSGS_VER;
		eventmask->command = EVENTMSGS_SET_MASK;
		eventmask->len = EAPD_WL_EVENTING_MASK_LEN;
		memcpy(eventmask->mask, bitvec, sizeof(bitvec));
		ret = wl_iovar_setbuf(os_name, "event_msgs_ext",
			(void *)buf_param, sizeof(bitvec) + EVENTMSGS_EXT_STRUCT_SIZE,
			(void *)buf, sizeof(buf));

		wlunit[unit] = unit;
		if (ret) {
			EAPD_ERROR("Set event_msg error %d on %s[%s]\n", ret, name, os_name);
		}
		else {
#ifdef BCMDBG
			int j, flag;
			if (eapd_msg_level) {
				flag = 1;
				for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
				for (j = 0; j < 8; j++) {
					if (isset(&bitvec[i], j)) {
						if (flag) {
							EAPD_PRINT("Set event_msg bitvec [%d",
								(i*8+j));
							flag = 0;
						}
						else {
							EAPD_PRINT(" %d", (i*8+j));
						}
					}
				}
				}
				EAPD_PRINT("] on %s[%s]\n", name, os_name);
			}
#endif /* BCMDBG */
		}
	}

	return 0;
}

static int
event_deinit(eapd_wksp_t *nwksp)
{
	return 0;
}

/* Validate handling EAPOL START packet on this receive interface */
static bool
eapd_valid_eapol_start(eapd_wksp_t *nwksp, eapd_brcm_socket_t *from, char *ifname)
{
	eapd_nas_t *nas;
	eapd_wps_t *wps;
	char nv_name[IFNAMSIZ];
	bool needHandle = FALSE;

	if (!nwksp || !from || !ifname)
		return FALSE;

	/* convert eth ifname to wl name (nv name) */
	if (osifname_to_nvifname(ifname, nv_name, sizeof(nv_name)) != 0)
		return FALSE;

	nas = &nwksp->nas;
	wps = &nwksp->wps;

	/* Is this brcmSocket have to handle EAPOL START */
	if (from->flag & (EAPD_CAP_NAS | EAPD_CAP_WPS)) {
		/* Is this receive interface have to handle EAPOL START */
		/* check nas */
		if (find_in_list(nas->ifnames, nv_name))
			needHandle = TRUE;

		/* check wps */
		if (find_in_list(wps->ifnames, nv_name))
			needHandle = TRUE;
	}

	return needHandle;
}

void
eapd_wksp_clear_inited()
{
	eapd_wksp_inited = 0;
}

int eapd_wksp_is_inited()
{
	return eapd_wksp_inited;
}

static bool
eapd_intf_bridged(char *wlifname, char *lanifname)
{
	FILE* fp;
	char brif[64] = {0};
	char ifnames[64] = {0};
	bool is_dpsta = FALSE;
	bool is_bridged = FALSE;

	memset(ifnames, 0, sizeof(ifnames));
	eapd_safe_get_conf(ifnames, sizeof(ifnames), "dpsta_ifnames");

	if (find_in_list(ifnames, wlifname)) {
		/* wlifname was included by dpsta */
		is_dpsta = TRUE;
		sprintf(brif, "/sys/class/net/%s/brif/dpsta", lanifname);
	} else {
		sprintf(brif, "/sys/class/net/%s/brif/%s", lanifname, wlifname);
	}

	if ((fp = fopen(brif, "r")) != NULL) {
		is_bridged = TRUE;
		fclose(fp);
	}

	EAPD_ERROR("eapd: %s %s bridged to %s, send to %s\n",
		is_dpsta ? "dpsta" : wlifname,
		is_bridged ? "is" : "is not",
		lanifname,
		is_bridged ? lanifname : wlifname);

	return is_bridged;
}
