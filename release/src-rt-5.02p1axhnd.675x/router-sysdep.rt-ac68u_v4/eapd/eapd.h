/*
 * Broadcom EAP dispatcher (EAPD) module include file
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
 * $Id: eapd.h 789955 2020-08-12 10:21:09Z $
 */

#ifndef _EAPD_H_
#define _EAPD_H_

#include <typedefs.h>
#include <ethernet.h>
#include <wlioctl.h>
#include <eapol.h>
#include <eap.h>

#ifndef EAPD_WKSP_AUTO_CONFIG
#define EAPD_WKSP_AUTO_CONFIG	1
#endif // endif

/* Message levels */
#define EAPD_ERROR_VAL		0x00000001
#define EAPD_INFO_VAL		0x00000002

extern uint eapd_msg_level;
extern bool eapd_ceventd_enable;

#define EAPDBANNER(fmt, arg...)	do { \
		printf(" EAPD>> %s(%d): "fmt, __FUNCTION__, __LINE__ , ##arg);} while (0)

#define EAPD_ERROR(fmt, arg...)
#define EAPD_INFO(fmt, arg...)
#define EAPD_INFO_LEAN(fmt, arg...)
#define EAPD_PRINT(fmt, arg...)	printf(fmt , ##arg)

#define EAPD_WKSP_FLAG_SHUTDOWN			0x1
#define EAPD_WKSP_FLAG_DUMP			0x2

#define EAPD_WKSP_RECV_DATA_MAX_LEN		4096

#define EAPD_WKSP_MIN_CMD_LINE_ARGS		16
#define EAPD_WKSP_MAX_CMD_LINE_ARGS		128
#define EAPD_WKSP_MAX_NO_BRIDGE                 WLIFU_MAX_NO_BRIDGE
#define EAPD_WKSP_MAX_NO_BRCM			58
#define EAPD_WKSP_MAX_NO_IFNAMES		66

#define EAPD_WKSP_MAX_SUPPLICANTS		128
/* Supplicant cache */
#define EAPD_PAE_HASH(ea) \
((((unsigned char *) ea)[3] ^ ((unsigned char *) ea)[4] ^ ((unsigned char *) ea)[5]) & \
(EAPD_WKSP_MAX_SUPPLICANTS - 1))

#define EAPD_WL_EVENTING_MASK_LEN \
	MAX(WL_EVENTING_MASK_LEN, (ROUNDUP(WLC_E_LAST, NBBY)/NBBY))

#define CEVENT_PKT_DUMP_LEN		32 /* bytes to dump */

typedef struct eapd_sta {
	bool		used;		/* flags use of item */
	time_t		last_use;	/* use timestamp */
	struct eapd_sta	*next;
	struct ether_addr	ea;	/* STA's ethernet address */
	struct ether_addr	bssid;	/* wl if hwaddr which sta comes in */
	char		ifname[IFNAMSIZ];
	ushort		pae_state;
	ushort		pae_id;
	uint32		mode;		/* Authentication mode */
	uint8		eapol_version;	/* eapol version */
} eapd_sta_t;

typedef struct eapd_socket {
	char		ifname[IFNAMSIZ];
	int		drvSocket;		/* raw socket to communicate with driver */
	int		ifindex;
	int		inuseCount;
	int		flag;
} eapd_brcm_socket_t, eapd_preauth_socket_t;

typedef struct eapd_cb {
	char			ifname[IFNAMSIZ];
	int			flags;
	eapd_brcm_socket_t	*brcmSocket;
	eapd_preauth_socket_t	preauthSocket;	/* only need by NAS */
	struct eapd_cb		*next;
} eapd_cb_t;

typedef struct eapd_app {
	char	ifnames[IFNAMSIZ * EAPD_WKSP_MAX_NO_IFNAMES]; /* interface names */
	int	appSocket; /* loopback socket to communicate with application */
	uchar	bitvec[EAPD_WL_EVENTING_MASK_LEN]; /* for each application which need brcmevent */
	eapd_cb_t	*cb; /* for each interface which running application */
} eapd_app_t, eapd_wps_t, eapd_nas_t, eapd_wai_t, eapd_dcs_t, eapd_mevent_t, eapd_bsd_t,
eapd_ssd_t, eapd_eventd_t, eapd_drsdbd_t, eapd_aspm_t, eapd_visdcoll_t, eapd_cevent_t, eapd_wlevent_t,
eapd_wlceventd_t, eapd_wbd_t, eapd_evt_t, eapd_ecbd_t, eapd_rgd_t;

typedef struct eapd_wksp {
	uchar			packet[EAPD_WKSP_RECV_DATA_MAX_LEN];
	int			brcmSocketCount;
	eapd_brcm_socket_t	brcmSocket[EAPD_WKSP_MAX_NO_BRCM];
	eapd_sta_t		sta[EAPD_WKSP_MAX_SUPPLICANTS];
	eapd_sta_t		*sta_hashed[EAPD_WKSP_MAX_SUPPLICANTS];
	eapd_wps_t		wps;
	eapd_nas_t		nas;
	eapd_wai_t		wai;
	eapd_dcs_t		dcs;
	eapd_mevent_t		mevent;
	eapd_bsd_t		bsd;
	eapd_drsdbd_t		drsdbd;
	eapd_ssd_t		ssd;
	eapd_eventd_t		eventd;
	eapd_ecbd_t		ecbd;
	eapd_aspm_t		aspm;
	eapd_visdcoll_t		visdcoll;
	eapd_cevent_t		cevent;
	eapd_wbd_t		wbd;
	eapd_evt_t		evt;
	eapd_rgd_t		rgd;
	eapd_wlevent_t          wlevent;
	eapd_wlceventd_t	wlceventd;
	int			flags;
	fd_set			fdset;
	int			fdmax;
	int			difSocket; /* socket to receive dynamic interface events */
	int			foreground;
	unsigned long		s_addr;
	int			psta_enabled;
} eapd_wksp_t;

typedef enum {
	EAPD_SEARCH_ONLY = 0,
	EAPD_SEARCH_ENTER
} eapd_lookup_mode_t;

typedef enum {
	EAPD_STA_MODE_UNKNOW = 0,
	EAPD_STA_MODE_WPS,
	EAPD_STA_MODE_WPS_ENR,
	EAPD_STA_MODE_NAS,
	EAPD_STA_MODE_NAS_PSK,
	EAPD_STA_MODE_WAI
} eapd_sta_mode_t;

typedef enum {
	EAPD_APP_UNKNOW = 0,
	EAPD_APP_WPS,
	EAPD_APP_NAS,
	EAPD_APP_WAI,
	EAPD_APP_DCS,
	EAPD_APP_MEVENT,
	EAPD_APP_BSD,
	EAPD_APP_DRSDBD,
	EAPD_APP_SSD,
	EAPD_APP_EVENTD,
	EAPD_APP_ASPM,
	EAPD_APP_VISDCOLL,
	EAPD_APP_WBD,
	EAPD_APP_CEVENT,
	EAPD_APP_EVT,
	EAPD_APP_ECBD,
	EAPD_APP_RGD,
	EAPD_APP_WLEVENT,
	EAPD_APP_WLCEVENTD
} eapd_app_mode_t;

/* PAE states */
typedef enum {
	EAPD_INITIALIZE = 0,
	EAPD_IDENTITY
} eapd_pae_state_t;

/* EAPD interface application capability */
#define EAPD_CAP_WLCEVENTD 0x1
#define EAPD_CAP_NAS	0x2
#define EAPD_CAP_WPS	0x4
#define EAPD_CAP_WAI	0x8
#define EAPD_CAP_DCS	0x10
#define EAPD_CAP_MEVENT	0x20
#define EAPD_CAP_BSD	0x40
#define EAPD_CAP_SSD	0x80
#define EAPD_CAP_DRSDBD	0x100
#define EAPD_CAP_ASPM	0x200
#define EAPD_CAP_VISDCOLL 0x400
#define EAPD_CAP_CEVENT	0x800
#define EAPD_CAP_EVENTD	0x1000
#define EAPD_CAP_ECBD   0x2000
#define EAPD_CAP_WBD	0x10000
#define EAPD_CAP_RGD	0x20000
#define EAPD_CAP_WLEVENT 0x40000
/* Apps */
int wps_app_init(eapd_wksp_t *nwksp);
int wps_app_deinit(eapd_wksp_t *nwksp);
int wps_app_monitor_sendup(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
#if EAPD_WKSP_AUTO_CONFIG
int wps_app_enabled(char *name);
#endif // endif
void wps_app_set_eventmask(eapd_app_t *app);
int wps_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void wps_app_recv_handler(eapd_wksp_t *nwksp, char *wlifname, eapd_cb_t *from,
	uint8 *pData, int *pLen, struct ether_addr *ap_ea);

int nas_app_init(eapd_wksp_t *nwksp);
int nas_app_deinit(eapd_wksp_t *nwksp);
int nas_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *fromlan);
#if EAPD_WKSP_AUTO_CONFIG
int nas_app_enabled(char *name);
#endif // endif
void nas_app_set_eventmask(eapd_app_t *app);
int nas_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void nas_app_recv_handler(eapd_wksp_t *nwksp, char *wlifname, eapd_cb_t *from,
	uint8 *pData, int *pLen);
int nas_open_dif_sockets(eapd_wksp_t *nwksp, eapd_cb_t *cb);
void nas_close_dif_sockets(eapd_wksp_t *nwksp, eapd_cb_t *cb);

int wai_app_init(eapd_wksp_t *nwksp);
int wai_app_deinit(eapd_wksp_t *nwksp);
int wai_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *fromlan);
#if EAPD_WKSP_AUTO_CONFIG
int wai_app_enabled(char *name);
#endif // endif
void wai_app_set_eventmask(eapd_app_t *app);
int wai_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void wai_app_recv_handler(eapd_wksp_t *nwksp, eapd_cb_t *from, uint8 *pData, int *pLen);

#ifdef BCM_DCS
int dcs_app_init(eapd_wksp_t *nwksp);
int dcs_app_deinit(eapd_wksp_t *nwksp);
int dcs_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *fromlan);
#if EAPD_WKSP_AUTO_CONFIG
int dcs_app_enabled(char *name);
#endif // endif
void dcs_app_set_eventmask(eapd_app_t *app);
int dcs_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void dcs_app_recv_handler(eapd_wksp_t *nwksp, eapd_cb_t *from,
	uint8 *pData, int *pLen);
#endif /* BCM_DCS */

#ifdef BCM_MEVENT
int mevent_app_init(eapd_wksp_t *nwksp);
int mevent_app_deinit(eapd_wksp_t *nwksp);
int mevent_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *fromlan);
#if EAPD_WKSP_AUTO_CONFIG
int mevent_app_enabled(char *name);
#endif // endif
void mevent_app_set_eventmask(eapd_app_t *app);
int mevent_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void mevent_app_recv_handler(eapd_wksp_t *nwksp, eapd_cb_t *from,
	uint8 *pData, int *pLen);
#endif /* BCM_MEVENT */

#ifdef BCM_WLEVENT
int wlevent_app_init(eapd_wksp_t *nwksp);
int wlevent_app_deinit(eapd_wksp_t *nwksp);
int wlevent_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *fromlan);
#if EAPD_WKSP_AUTO_CONFIG
int wlevent_app_enabled(char *name);
#endif
void wlevent_app_set_eventmask(eapd_app_t *app);
int wlevent_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void wlevent_app_recv_handler(eapd_wksp_t *nwksp, eapd_cb_t *from,
	uint8 *pData, int *pLen);
#endif

#ifdef BCM_BSD
int bsd_app_init(eapd_wksp_t *nwksp);
int bsd_app_deinit(eapd_wksp_t *nwksp);
int bsd_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *fromlan);
#if EAPD_WKSP_AUTO_CONFIG
int bsd_app_enabled(char *name);
#endif // endif
void bsd_app_set_eventmask(eapd_app_t *app);
int bsd_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void bsd_app_recv_handler(eapd_wksp_t *nwksp, eapd_cb_t *from,
	uint8 *pData, int *pLen);
#endif /* BCM_BSD */

#ifdef BCM_DRSDBD
int drsdbd_app_init(eapd_wksp_t *nwksp);
int drsdbd_app_deinit(eapd_wksp_t *nwksp);
int drsdbd_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *fromlan);
#if EAPD_WKSP_AUTO_CONFIG
int drsdbd_app_enabled(char *name);
#endif // endif
void drsdbd_app_set_eventmask(eapd_app_t *app);
int drsdbd_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void drsdbd_app_recv_handler(eapd_wksp_t *nwksp, eapd_cb_t *from,
	uint8 *pData, int *pLen);
#endif /* BCM_DRSDBD */

#ifdef BCM_SSD
int ssd_app_init(eapd_wksp_t *nwksp);
int ssd_app_deinit(eapd_wksp_t *nwksp);
int ssd_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *fromlan);
#if EAPD_WKSP_AUTO_CONFIG
int ssd_app_enabled(char *name);
#endif // endif
void ssd_app_set_eventmask(eapd_app_t *app);
int ssd_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void ssd_app_recv_handler(eapd_wksp_t *nwksp, eapd_cb_t *from,
	uint8 *pData, int *pLen);
#endif /* BCM_SSD */
#ifdef BCM_EVENTD
int eventd_app_init(eapd_wksp_t *nwksp);
int eventd_app_deinit(eapd_wksp_t *nwksp);
int eventd_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *fromlan);
#if EAPD_WKSP_AUTO_CONFIG
int eventd_app_enabled(char *name);
#endif // endif
void eventd_app_set_eventmask(eapd_app_t *app);
int eventd_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void eventd_app_recv_handler(eapd_wksp_t *nwksp, eapd_cb_t *from,
	uint8 *pData, int *pLen);
#endif /* BCM_EVENTD */
#ifdef BCM_ASPMD
int aspm_app_init(eapd_wksp_t *nwksp);
int aspm_app_deinit(eapd_wksp_t *nwksp);
int aspm_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *from);
#if EAPD_WKSP_AUTO_CONFIG
int aspm_app_enabled(char *name);
#endif // endif
void aspm_app_set_eventmask(eapd_app_t *app);
int aspm_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void aspm_app_recv_handler(eapd_wksp_t *nwksp, char *wlifname, eapd_cb_t *from,
	uint8 *pData, int *pLen, struct ether_addr *ap_ea);
#endif /* BCM_ASPMD */

#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
int visdcoll_app_init(eapd_wksp_t *nwksp);
int visdcoll_app_deinit(eapd_wksp_t *nwksp);
int visdcoll_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *fromlan);
#if EAPD_WKSP_AUTO_CONFIG
int visdcoll_app_enabled(char *name);
#endif // endif
void visdcoll_app_set_eventmask(eapd_app_t *app);
int visdcoll_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void visdcoll_app_recv_handler(eapd_wksp_t *nwksp, eapd_cb_t *from,
	uint8 *pData, int *pLen);
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */

#ifdef BCM_CEVENT
int cevent_app_init(eapd_wksp_t *nwksp);
int cevent_app_deinit(eapd_wksp_t *nwksp);
int cevent_app_sendup(eapd_wksp_t *nwksp, bcm_event_t *be, uint16 be_length, char *fromlan);
#if EAPD_WKSP_AUTO_CONFIG
int cevent_app_enabled(char *name);
#endif // endif
void cevent_app_set_eventmask(eapd_app_t *app);
int cevent_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int len, char *fromIfname,
		bool origin, uint32 appmask);
void cevent_generic_hub(eapd_wksp_t *nwksp, char *fromIfname, eapd_cb_t *cb, uint8 *pData,
	int len, bool origin, uint32 appmask);
void cevent_app_recv_handler(eapd_wksp_t *nwksp, char *fromIfname, eapd_cb_t *cb, uint8 *pData,
	int *pLen);
int cevent_to_app(eapd_wksp_t *nwksp, bcm_event_t* be, uint16 be_length, char *fromIfname);
void cevent_copy_eapol_and_forward(eapd_wksp_t *nwksp, char *ifname,
		const eapol_header_t *eapol, const eap_header_t *eap, uint32 app);
#endif /* BCM_CEVENT */

#ifdef BCM_WBD
int wbd_app_init(eapd_wksp_t *nwksp);
int wbd_app_deinit(eapd_wksp_t *nwksp);
int wbd_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *fromlan);
#if EAPD_WKSP_AUTO_CONFIG
int wbd_app_enabled(char *name);
#endif // endif
void wbd_app_set_eventmask(eapd_app_t *app);
int wbd_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void wbd_app_recv_handler(eapd_wksp_t *nwksp, char *wlifname, eapd_cb_t *from,
	uint8 *pData, int *pLen, struct ether_addr *ap_ea);
#endif /* BCM_WBD */

#ifdef BCM_CUSTOM_EVENT
int evt_app_init(eapd_wksp_t *nwksp);
int evt_app_deinit(eapd_wksp_t *nwksp);
int evt_app_monitor_sendup(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void evt_app_set_eventmask(eapd_app_t *app);
int evt_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void evt_app_recv_handler(eapd_wksp_t *nwksp, char *wlifname, eapd_cb_t *from,
	uint8 *pData, int *pLen, struct ether_addr *ap_ea);
#endif /* BCM_CUSTOM_EVENT */

#ifdef BCM_ECBD
int ecbd_app_init(eapd_wksp_t *nwksp);
int ecbd_app_deinit(eapd_wksp_t *nwksp);
int ecbd_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *fromlan);
#if EAPD_WKSP_AUTO_CONFIG
int ecbd_app_enabled(char *name);
#endif // endif
void ecbd_app_set_eventmask(eapd_app_t *app);
int ecbd_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void ecbd_app_recv_handler(eapd_wksp_t *nwksp, eapd_cb_t *from,
	uint8 *pData, int *pLen);
#endif /* BCM_ECBD */

#ifdef BCM_RGD
int rgd_app_init(eapd_wksp_t *nwksp);
int rgd_app_deinit(eapd_wksp_t *nwksp);
int rgd_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *from);
#if EAPD_WKSP_AUTO_CONFIG
int rgd_app_enabled(char *name);
#endif // endif
void rgd_app_set_eventmask(eapd_app_t *app);
int rgd_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void rgd_app_recv_handler(eapd_wksp_t *nwksp, eapd_cb_t *from,
	uint8 *pData, int *pLen);
#endif /* BCM_RGD */

#ifdef BCM_WLCEVENTD
int wlceventd_app_init(eapd_wksp_t *nwksp);
int wlceventd_app_deinit(eapd_wksp_t *nwksp);
int wlceventd_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *fromlan);
#if EAPD_WKSP_AUTO_CONFIG
int wlceventd_app_enabled(char *name);
#endif
void wlceventd_app_set_eventmask(eapd_app_t *app);
int wlceventd_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from);
void wlceventd_app_recv_handler(eapd_wksp_t *nwksp, eapd_cb_t *from,
        uint8 *pData, int *pLen);
#endif /* BCM_WLCEVENTD */

/* OS independent function */
void eapd_wksp_display_usage(void);
eapd_wksp_t * eapd_wksp_alloc_workspace(void);
int eapd_wksp_auto_config(eapd_wksp_t *nwksp);
int eapd_wksp_parse_cmd(int argc, char *argv[], eapd_wksp_t *nwksp);
int eapd_wksp_init(eapd_wksp_t *nwksp);
void eapd_wksp_dispatch(eapd_wksp_t *nwksp);
int eapd_wksp_deinit(eapd_wksp_t *nwksp);
void eapd_wksp_free_workspace(eapd_wksp_t * nwksp);
int eapd_wksp_main_loop(eapd_wksp_t *nwksp);
void eapd_wksp_cleanup(eapd_wksp_t *nwksp);
void eapd_wksp_clear_inited(void);
int eapd_wksp_is_inited(void);

eapd_sta_t* sta_lookup(eapd_wksp_t *nwksp, struct ether_addr *sta_ea, struct ether_addr *bssid_ea,
	char *ifname, eapd_lookup_mode_t mode);
void sta_remove(eapd_wksp_t *nwksp, eapd_sta_t *sta);

eapd_brcm_socket_t* eapd_add_brcm(eapd_wksp_t *nwksp, char *ifname);
int eapd_del_brcm(eapd_wksp_t *nwksp, eapd_brcm_socket_t *sock);
eapd_brcm_socket_t* eapd_find_brcm(eapd_wksp_t *nwksp, char *ifname);

void eapd_brcm_recv_handler(eapd_wksp_t *nwksp, eapd_brcm_socket_t *from, uint8 *pData, int *pLen);
extern void eapd_preauth_recv_handler(eapd_wksp_t *nwksp, char *from, uint8 *pData, int *pLen);

/* OS dependent function */
void eapd_eapol_canned_send(eapd_wksp_t *nwksp, struct eapd_socket *Socket, eapd_sta_t *sta,
                                                           unsigned char code, unsigned char type);
void eapd_message_send(eapd_wksp_t *nwksp, struct eapd_socket *Socket, uint8 *pData, int pLen);
int eapd_brcm_open(eapd_wksp_t *nwksp, eapd_brcm_socket_t *sock);
int eapd_brcm_close(int drvSocket);
int eapd_preauth_open(eapd_wksp_t *nwksp, eapd_preauth_socket_t *sock);
int eapd_preauth_close(int drvSocket);
int eapd_safe_get_conf(char *outval, int outval_size, char *name);
size_t eapd_message_read(int fd, void *buf, size_t nbytes);

#define EAPD_UDP_SIN_ADDR(nwksp) ((nwksp)->s_addr)
#endif /* _EAPD_H_ */
