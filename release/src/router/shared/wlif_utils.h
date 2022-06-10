/*
 * Shell-like utility functions
 *
 * Copyright (C) 2021, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wlif_utils.h 801286 2021-07-20 05:32:02Z $
 */

#ifndef _wlif_utils_h_
#define _wlif_utils_h_

#ifdef RTCONFIG_BCMWL6
#include "bcmwifi_channels.h"
#endif
#if defined(RTCONFIG_BCM_502L07P2)
#include "ethernet.h"
#include <wpsdefs.h>
#else
#include "bcm_usched.h"
#include "proto/ethernet.h"
#endif
#include <wlioctl.h>

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6
#endif

#ifdef RTCONFIG_AMAS_WGN
#define WLIFU_MAX_NO_BRIDGE		8
#else
#define WLIFU_MAX_NO_BRIDGE		5
#endif

#define WLIFU_MAX_NO_WAN		2

#define MAX_USER_KEY_LEN	80			/* same as NAS_WKSP_MAX_USER_KEY_LEN */
#define MAX_SSID_LEN		32			/* same as DOT11_MAX_SSID_LEN */

#if defined(RTCONFIG_BCM_502L07P2)
#ifdef __CONFIG_RSDB__
/* Please keep the below enum in sync with wlc_rsdb_modes in wlc_rsdb.h */
enum wlif_rsdb_modes {
	WLIF_RSDB_MODE_AUTO = AUTO,
	WLIF_RSDB_MODE_2X2,
	WLIF_RSDB_MODE_RSDB,
	WLIF_RSDB_MODE_80P80,
	WLIF_RSDB_MODE_MAX
};
#endif /* __CONFIG_RSDB__ */
#endif

typedef struct wsec_info_s {
	int unit;					/* interface unit */
	int ibss;					/* IBSS vs. Infrastructure mode */
	int gtk_rekey_secs;		/* group key rekey interval */
	int wep_index;			/* wep key index */
	int ssn_to;				/* ssn timeout value */
	int debug;				/* verbose - 0:no | others:yes */
	int preauth;				/* preauth */
	int auth_blockout_time;	/* update auth blockout retry interval */
	unsigned int auth;	/* shared key authentication optional (0) or required (1) */
	unsigned int akm;			/* authentication mode */
	unsigned int wsec;			/* encryption */
	unsigned int mfp;			/* mfp */
	unsigned int flags;			/* flags */
	char osifname[IFNAMSIZ];	/* interface name */
	unsigned char ea[ETHER_ADDR_LEN];			/* interface hw address */
	unsigned char remote[ETHER_ADDR_LEN];	/* wds remote address */
	unsigned short radius_port;				/* radius server port number */
	char ssid[MAX_SSID_LEN + 1];				/* ssid info */
	char psk[MAX_USER_KEY_LEN + 1];			/* user-supplied psk passphrase */
	char *secret;				/* user-supplied radius secret */
	char *wep_key;			/* user-supplied wep key */
	char *radius_addr;		/* radius server address */
	char *nas_id;			/* nas mac address */
} wsec_info_t;

#define WLIFU_WSEC_SUPPL			0x00000001	/* role is supplicant */
#define WLIFU_WSEC_AUTH			0x00000002	/* role is authenticator */
#define WLIFU_WSEC_WDS			0x00000004	/* WDS mode */

#define WLIFU_AUTH_RADIUS			0x20	/* same as nas_mode_t RADIUS in nas.h */

/* get wsec return code */
#define WLIFU_WSEC_SUCCESS			0
#define WLIFU_ERR_INVALID_PARAMETER	1
#define WLIFU_ERR_NOT_WL_INTERFACE	2
#define WLIFU_ERR_NOT_SUPPORT_MODE	4
#define WLIFU_ERR_WL_REMOTE_HWADDR	3
#define WLIFU_ERR_WL_WPA_ROLE		5

/* BSS transition return code */
#define WLIFU_BSS_TRANS_RESP_ACCEPT	0
#define WLIFU_BSS_TRANS_RESP_REJECT	1
#define WLIFU_BSS_TRANS_RESP_UNKNOWN	2

/* NVRAM names */
#define WLIFU_NVRAM_BSS_TRANS_NO_DEAUTH	"bss_trans_no_deauth"

extern int get_wlname_by_mac(unsigned char *mac, char *wlname);
extern char *get_ifname_by_wlmac(unsigned char *mac, char *name);
extern int get_wsec(wsec_info_t *info, unsigned char *mac, char *osifname);
extern bool wl_wlif_is_psta(char *ifname);
extern bool wl_wlif_is_dwds(char *ifname);
#if defined(RTCONFIG_HND_ROUTER_AX)
extern int wl_wlif_get_chip_cap(char *ifname, char *cap);
#ifdef __CONFIG_RSDB__
extern int wl_wlif_get_rsdb_mode();
#endif /* __CONFIG_RSDB__ */
#endif
extern bool wl_wlif_is_wet_ap(char *ifname);

/*
 * Routine for getting the nss value.
 * Params:
 * @bi: BSS info.
 */
extern int wl_wlif_get_max_nss(wl_bss_info_t* bi);
#if !defined(RTCONFIG_HND_ROUTER_AX)
extern bool wl_wlif_is_psr_ap(char *ifname);
extern int wl_wlif_do_bss_trans(void *hdl, char *ifname, uint8 rclass, chanspec_t chanspec, struct ether_addr bssid, struct ether_addr addr, int timeout, int event_fd);
extern int wl_wlif_block_mac(void *hdl, char *ifname, struct ether_addr addr, int timeout);
extern int wl_wlif_unblock_mac(char *ifname, struct ether_addr addr, int flag);

extern int get_spoof_mac(const char *osifname, char *mac, int maclen);
extern int get_spoof_ifname(char *mac, char *osifname, int osifnamelen);
extern int get_real_mac(char *mac, int maclen);
extern int get_lan_mac(unsigned char *mac);
extern unsigned char *get_wlmacstr_by_unit(char *unit);
#endif

#define WLIF_STRNCPY(dst, src, count) \
	do { \
		strncpy(dst, src, count - 1); \
		dst[count - 1] = '\0'; \
	} while (0)

#define WLIF_MAX_BUF			256

#ifdef CONFIG_HOSTAPD

// Key mgmt types
#define WLIF_WPA_AKM_PSK	0x1	// WPA-PSK
#define WLIF_WPA_AKM_PSK2	0x2	// WPA2-PSK

// Auth algo type
#define WLIF_WPA_AUTH_OPEN	0x1	// OPEN
#define WLIF_WPA_AUTH_SHARED	0x2	// SHARED

// Encryption types
#define WLIF_WPA_ENCR_TKIP	0x1	// TKIP
#define WLIF_WPA_ENCR_AES	0x2	// AES

// SSID and psk size
#define WLIF_SSID_MAX_SZ		32
#define WLIF_PSK_MAX_SZ			64

#ifdef RTCONFIG_HND_ROUTER_AX
#define WLIF_DPP_PARAMS_MAX_SIZE	512
#endif

#define DPP_AKM		"dpp"
#define PSK_AKM		"psk"
#define SAE_AKM		"sae"
#define PSK_SAE_AKM	"psk+sae"
#define DPP_SAE_AKM	"dpp+sae"
#define DPP_PSK_SAE_AKM	"dpp+psk+sae"

// WPS states to update the UI
typedef enum wlif_wps_ui_status_code_id {
	WLIF_WPS_UI_INVALID		= -1,
	WLIF_WPS_UI_INIT		= 0,
	WLIF_WPS_UI_FINDING_PBC_STA	= 1,
	WLIF_WPS_UI_OK			= 2,
	WLIF_WPS_UI_ERR			= 3,
	WLIF_WPS_UI_TIMEOUT		= 4,
	WLIF_WPS_UI_PBCOVERLAP		= 5,
	WLIF_WPS_UI_FIND_PBC_AP		= 6,
	WLIF_WPS_UI_FIND_PIN_AP		= 7
} wlif_wps_ui_status_code_id_t;

// Thread states
typedef enum states {
	WLIF_THREAD_CLOSED	= 0,
	WLIF_THREAD_TOBE_CLOSED	= 1,
	WLIF_THREAD_RUNNING	= 2
} wlif_thread_states;

/* Different types of wps led blink types based on wps process state */
typedef enum wlif_wps_blinktype {
	WLIF_WPS_BLINKTYPE_INVALID	= -1,
	WLIF_WPS_BLINKTYPE_INPROGRESS	= 0,
	WLIF_WPS_BLINKTYPE_ERROR	= 1,
	WLIF_WPS_BLINKTYPE_OVERLAP	= 2,
	WLIF_WPS_BLINKTYPE_SUCCESS	= 3,
	WLIF_WPS_BLINKTYPE_STOP		= 4
} wlif_wps_blinktype_t;

/* Different types of possible wps operations from UI like start, stop etc. */
typedef enum wlif_wps_op_type {
	WLIF_WPS_IDLE		= 0,
	WLIF_WPS_START		= 1,
	WLIF_WPS_RESTART	= 2,
	WLIF_WPS_STOP		= 3
} wlif_wps_op_type_t;

/* Different types of wps operating modes */
typedef enum wlif_wps_mode {
	WLIF_WPS_INVALID	= 0,
	WLIF_WPS_ENROLLEE	= 1,
	WLIF_WPS_REGISTRAR	= 2
} wlif_wps_mode_t;

// Struct to hold the network settings received using wps.
typedef struct wlif_wps_nw_settings {
	char ssid[WLIF_SSID_MAX_SZ + /* '\0' */ 1];	// SSID.
	char nw_key[WLIF_PSK_MAX_SZ + /* '\0' */ 1];	// Network Key.
	uint8 akm;		// Key mgmt.
	uint8 auth_type;	// Auth type can be open/shared.
	uint8 encr;		// Encryption types TKIP/AES.
	bool invalid;		// Check for the validity of credentials
} wlif_wps_nw_creds_t;

#ifdef RTCONFIG_HND_ROUTER_AX
// Structure to hold the network settings received in DPP config response.
typedef struct wlif_dpp_config_settings {
	char ssid[WLIF_SSID_MAX_SZ + /* '\0' */ 1];		// SSID
	char akm[WLIF_PSK_MAX_SZ + /* '\0' */ 1];		// AKM
	char dpp_connector[WLIF_DPP_PARAMS_MAX_SIZE];		// DPP connector
	char dpp_csign[WLIF_DPP_PARAMS_MAX_SIZE];		// DPP C-sign
	char dpp_netaccess_key[WLIF_DPP_PARAMS_MAX_SIZE];	// DPP Net access key
#if 0 // TODO: wlan 17.10.157.60
	char dpp_pp_key[WLIF_DPP_PARAMS_MAX_SIZE];		// DPP PP key
	unsigned char dpp_psk[WLIF_PSK_MAX_SZ + /* '\0' */ 1];	// PSK
	char dpp_pass[2*WLIF_PSK_MAX_SZ + /* '\0' */ 1];	// PASS
#endif
} wlif_dpp_creds_t;
#endif

/* Struct to store the bss info */
typedef struct wlif_bss {
	char ifname[IFNAMSIZ];		// Ifname of type ethx
	char nvifname[IFNAMSIZ];	// Nvifname of type wlx, wlx.y etc
} wlif_bss_t;

/* Return TRUE if ap is already configured otherwise retuns FALSE */
bool wl_wlif_does_ap_needs_to_be_configured(char *ifname);
/* Sets the AP state to configured */
void wl_wlif_set_ap_as_configured(char *ifname);
/* Gets the status code from the wps_proc_status nvram value */
int wl_wlif_get_wps_status_code();
/* Updates the nvram value of wps_proc_status which is used to update ui */
void wl_wlif_update_wps_ui(wlif_wps_ui_status_code_id_t idx);
#if 0 // TODO: wlan 17.10.157.60
/* Updates the nvram value of dpp_status which is used to update ui */
void wl_wlif_update_dpp_ui(DPP_UI_SCSTATE idx, char *ifname);
#endif
/* Function to parse the hostapd config file */
int wl_wlif_parse_hapd_config(char *ifname, wlif_wps_nw_creds_t *creds);
/* Function to parse the  wpa_supplicant config file */
int wl_wlif_parse_wpa_config_file(char *prefix, wlif_wps_nw_creds_t *creds);
/* Applies the received network credentials to the interface provided in bss */
int wl_wlif_apply_creds(wlif_bss_t *bss, wlif_wps_nw_creds_t *creds);
/* Invokes the hostapd/wpa_supplicant wps session */
int wl_wlif_wps_pbc_hdlr(char *wps_ifname, char *bh_ifname);
/* Stops the hostapd/wpa_supplicant wps session */
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_BCM_502L07P2)
int wl_wlif_wps_stop_session(char *wps_ifname, bool bUpdateUI);
#else
int wl_wlif_wps_stop_session(char *wps_ifname);
#endif
/* Function pointer to be provided to the thread creation routine */
typedef void* (*wlif_thrd_func)(void *arg);
/* Thread creation routine */
int wl_wlif_create_thrd(pthread_t *thread_id, wlif_thrd_func fptr, void *arg, bool is_detached);
#ifdef MULTIAP
/* Checks whether the wps session for the multiap onboarding or not */
bool wl_wlif_is_map_onboarding(char *prefix);
/* Updates the backhaul station interfcae with network settings after successful wps session  */
int wl_wlif_map_configure_backhaul_sta_interface(wlif_bss_t *bss, wlif_wps_nw_creds_t *creds);
/* Gets the timeout value for the multiap repeter */
int wl_wlif_wps_map_timeout();
#endif	/* MULTIAP */
#ifdef RTCONFIG_HND_ROUTER_AX
/* Applies DPP credentials to the interface provided in bss */
int wl_wlif_apply_dpp_creds(wlif_bss_t *bss, wlif_dpp_creds_t *dpp_creds);
#if 0 // TODO: wlan 17.10.157.60
/* convert ascii string to hex string */
void wl_ascii_str_to_hex_str(char *ascii_str, uint16 ascii_len, char *hex_str, uint16 hex_len);
/* convert hex string to ascii */
int wl_wlif_hexstr2ascii(const char *hex_str, unsigned char *buf, size_t len);

int get_all_lanifname_sz(void);
int get_all_lanifname(char *ifnames, int ifnames_sz);
int get_all_lanifnames_listsz(void);
int get_all_lanifnames_list(char *ifnames_list, int ifnames_listsz);
#endif
#endif

/* wps session timeout */
#define WLIF_WPS_TIMEOUT		120

#ifdef BCA_HNDROUTER
/* Routine for changing wps led status */
void wl_wlif_wps_gpio_led_blink(int board_fp, wlif_wps_blinktype_t blinktype);
/* Routine for initializing wps gpio board */
int wl_wlif_wps_gpio_init();
/* Routine to cleanup wps gpio board handle */
void wl_wlif_wps_gpio_cleanup(int board_fp);
#else
#define wl_wlif_wps_gpio_led_blink(a, b)	do {} while (0)
#define wl_wlif_wps_gpio_init()		(0)
#define wl_wlif_wps_gpio_cleanup(a)		do {} while (0)
#endif	/* BCA_HNDROUTER */
#endif	/* CONFIG_HOSTAPD */

#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_BCM_502L07P2)
#if !defined(RTCONFIG_SDK504L02_188_1303)
/* LGI supported rate bitmap control feature */
typedef enum _bits {
	BIT0 = 0,	BIT1,	BIT2,	BIT3,	BIT4,	BIT5,	BIT6,	BIT7,
	BIT8,		BIT9,	BIT10,	BIT11,	BIT12,	BIT13,	BIT14,	BIT15,
	BIT16,		BIT17,	BIT18,	BIT19,	BIT20,	BIT21,	BIT22,	BIT23,
	BIT24,		BIT25,	BIT26,	BIT27,	BIT28,	BIT29,	BIT30,	BIT31
} bits_t;

/*
 * supported rate bitmap definition
 * b19b18b17b16b15b14b13b12b11b10b9b8b1b7b6b5b4b3b2b1b0
 * mcs7mcs6mcs5mcs4mcs3mcs2mcs1mcs054Mbps48Mbps36Mbps24Mbps18Mbps12Mpbs9Mbps6Mbps11Mbps5.5Mbps2Mps1Mbps
 */
typedef struct bit2rate_map {
	bits_t bit;	/* bitmap order */
	int rate;       /* in 500 kbps unit for legacy rate */
} bit2rate_map_t;

/* from wlu_common.h of the same type name */
typedef union wl_rateset_args_u {
	wl_rateset_args_v1_t rsv1;
	wl_rateset_args_v2_t rsv2;
} wl_rateset_args_u_t;

#define WLIF_NVRAM_SUPPORT_RATE_BITMAP	"support_rate_bitmap"

int wl_rateset_get_bitmap_index(bit2rate_map_t *tbl, int tbl_sz, int i, int *l);
void wl_rateset_init_fields(wl_rateset_args_u_t* rs, int rsver);
void wl_rateset_get_fields(wl_rateset_args_u_t *rs, int rsver, uint32 **rscount, uint8 **rsrates,
		uint8 **rsmcs, uint16 **rsvht_mcs, uint16 **rshe_mcs);
int wl_rateset_get_args_info(void *wl, int *rs_len, int *rs_ver);
bool rateset_overwrite_by_supportedRatesBitmap(char *name, char *prefix);
double wl_get_txpwr_target_max(char *name);
double get_wifi_maxpower(int target_unit);
#endif
#endif
#endif /* _wlif_utils_h_ */
