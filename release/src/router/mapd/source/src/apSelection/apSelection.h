/*
 * ***************************************************************************
 * *  Mediatek Inc.
 * * 4F, No. 2 Technology 5th Rd.
 * * Science-based Industrial Park
 * * Hsin-chu, Taiwan, R.O.C.
 * *
 * * (c) Copyright 2002-2018, Mediatek, Inc.
 * *
 * * All rights reserved. Mediatek's source code is an unpublished work and the
 * * use of a copyright notice does not imply otherwise. This source code
 * * contains confidential trade secret material of Ralink Tech. Any attemp
 * * or participation in deciphering, decoding, reverse engineering or in any
 * * way altering the source code is stricitly prohibited, unless the prior
 * * written consent of Mediatek, Inc. is obtained.
 * ***************************************************************************
 *
 *  Module Name:
 *  AP selection
 *
 *  Abstract:
 *  AP selection
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Kapil.Gupta 2018/05/02    First implementation of the AP selection
 * */
#ifndef AP_SELECTION_H
#define AP_SELECTION_H

//#define EqualMemory(Source1, Source2, Length)	(!os_memcmp(Source1, Source2, Length))
//#define SSID_EQUAL(ssid1, len1, ssid2, len2)	((len1 == len2) && (EqualMemory(ssid1, ssid2, len1)))
#define MAX_BL_TIMEOUT	300

typedef enum _SEC_AKM_MODE {
	SEC_AKM_OPEN,
	SEC_AKM_SHARED,
	SEC_AKM_AUTOSWITCH,
	SEC_AKM_WPA1, /* Enterprise security over 802.1x */
	SEC_AKM_WPA1PSK,
	SEC_AKM_WPANone, /* For Win IBSS, directly PTK, no handshark */
	SEC_AKM_WPA2, /* Enterprise security over 802.1x */
	SEC_AKM_WPA2PSK,
	SEC_AKM_FT_WPA2,
	SEC_AKM_FT_WPA2PSK,
	SEC_AKM_WPA2_SHA256,
	SEC_AKM_WPA2PSK_SHA256,
	SEC_AKM_TDLS,
	SEC_AKM_SAE_SHA256,
	SEC_AKM_FT_SAE_SHA256,
	SEC_AKM_SUITEB_SHA256,
	SEC_AKM_SUITEB_SHA384,
	SEC_AKM_FT_WPA2_SHA384,
	SEC_AKM_WAICERT, /* WAI certificate authentication */
	SEC_AKM_WAIPSK, /* WAI pre-shared key */
	SEC_AKM_OWE,
	SEC_AKM_MAX /* Not a real mode, defined as upper bound */
} SEC_AKM_MODE, *PSEC_AKM_MODE;


#define IS_AKM_OPEN(_AKMMap)                           ((_AKMMap & (1 << SEC_AKM_OPEN)) > 0)
#define IS_AKM_SHARED(_AKMMap)                       ((_AKMMap & (1 << SEC_AKM_SHARED)) > 0)
#define IS_AKM_AUTOSWITCH(_AKMMap)              ((_AKMMap & (1 << SEC_AKM_AUTOSWITCH)) > 0)
#define IS_AKM_WPA1(_AKMMap)                           ((_AKMMap & (1 << SEC_AKM_WPA1)) > 0)
#define IS_AKM_WPA1PSK(_AKMMap)                    ((_AKMMap & (1 << SEC_AKM_WPA1PSK)) > 0)
#define IS_AKM_WPANONE(_AKMMap)                  ((_AKMMap & (1 << SEC_AKM_WPANone)) > 0)
#define IS_AKM_WPA2(_AKMMap)                          ((_AKMMap & (1 << SEC_AKM_WPA2)) > 0)
#define IS_AKM_WPA2PSK(_AKMMap)                    ((_AKMMap & (1 << SEC_AKM_WPA2PSK)) > 0)
#define IS_AKM_FT_WPA2(_AKMMap)                     ((_AKMMap & (1 << SEC_AKM_FT_WPA2)) > 0)
#define IS_AKM_FT_WPA2PSK(_AKMMap)              ((_AKMMap & (1 << SEC_AKM_FT_WPA2PSK)) > 0)
#define IS_AKM_WPA2_SHA256(_AKMMap)            ((_AKMMap & (1 << SEC_AKM_WPA2_SHA256)) > 0)
#define IS_AKM_WPA2PSK_SHA256(_AKMMap)      ((_AKMMap & (1 << SEC_AKM_WPA2PSK_SHA256)) > 0)
#define IS_AKM_TDLS(_AKMMap)                             ((_AKMMap & (1 << SEC_AKM_TDLS)) > 0)
#define IS_AKM_SAE_SHA256(_AKMMap)                ((_AKMMap & (1 << SEC_AKM_SAE_SHA256)) > 0)
#define IS_AKM_FT_SAE_SHA256(_AKMMap)          ((_AKMMap & (1 << SEC_AKM_FT_SAE_SHA256)) > 0)
#define IS_AKM_SUITEB_SHA256(_AKMMap)          ((_AKMMap & (1 << SEC_AKM_SUITEB_SHA256)) > 0)
#define IS_AKM_SUITEB_SHA384(_AKMMap)          ((_AKMMap & (1 << SEC_AKM_SUITEB_SHA384)) > 0)
#define IS_AKM_FT_WPA2_SHA384(_AKMMap)      ((_AKMMap & (1 << SEC_AKM_FT_WPA2_SHA384)) > 0)
#define IS_AKM_OWE(_AKMMap)      ((_AKMMap & (1 << SEC_AKM_OWE)) > 0)
/* Authentication types */
#define WSC_AUTHTYPE_OPEN        0x0001
#define WSC_AUTHTYPE_WPAPSK      0x0002
#define WSC_AUTHTYPE_SHARED      0x0004
#define WSC_AUTHTYPE_WPA         0x0008
#define WSC_AUTHTYPE_WPA2        0x0010
#define WSC_AUTHTYPE_WPA2PSK     0x0020
#define WSC_AUTHTYPE_SAE         0x0040
#define WSC_AUTHTYPE_WPANONE     0x0080

typedef enum _SEC_CIPHER_MODE {
	SEC_CIPHER_NONE,
	SEC_CIPHER_WEP40,
	SEC_CIPHER_WEP104,
	SEC_CIPHER_WEP128,
	SEC_CIPHER_TKIP,
	SEC_CIPHER_CCMP128,
	SEC_CIPHER_CCMP256,
	SEC_CIPHER_GCMP128,
	SEC_CIPHER_GCMP256,
	SEC_CIPHER_BIP_CMAC128,
	SEC_CIPHER_BIP_CMAC256,
	SEC_CIPHER_BIP_GMAC128,
	SEC_CIPHER_BIP_GMAC256,
	SEC_CIPHER_WPI_SMS4, /* WPI SMS4 support */
	SEC_CIPHER_MAX /* Not a real mode, defined as upper bound */
} SEC_CIPHER_MODE;


#define IS_CIPHER_NONE(_Cipher)          (((_Cipher) & (1 << SEC_CIPHER_NONE)) > 0)
#define IS_CIPHER_WEP40(_Cipher)          (((_Cipher) & (1 << SEC_CIPHER_WEP40)) > 0)
#define IS_CIPHER_WEP104(_Cipher)        (((_Cipher) & (1 << SEC_CIPHER_WEP104)) > 0)
#define IS_CIPHER_WEP128(_Cipher)        (((_Cipher) & (1 << SEC_CIPHER_WEP128)) > 0)
#define IS_CIPHER_WEP(_Cipher)              (((_Cipher) & ((1 << SEC_CIPHER_WEP40) | (1 << SEC_CIPHER_WEP104) | (1 << SEC_CIPHER_WEP128))) > 0)
#define IS_CIPHER_TKIP(_Cipher)              (((_Cipher) & (1 << SEC_CIPHER_TKIP)) > 0)
#define IS_CIPHER_WEP_TKIP_ONLY(_Cipher)     ((IS_CIPHER_WEP(_Cipher) || IS_CIPHER_TKIP(_Cipher)) && (_Cipher < (1 << SEC_CIPHER_CCMP128)))
#define IS_CIPHER_CCMP128(_Cipher)      (((_Cipher) & (1 << SEC_CIPHER_CCMP128)) > 0)
#define IS_CIPHER_CCMP256(_Cipher)      (((_Cipher) & (1 << SEC_CIPHER_CCMP256)) > 0)
#define IS_CIPHER_GCMP128(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_GCMP128)) > 0)
#define IS_CIPHER_GCMP256(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_GCMP256)) > 0)
#define IS_CIPHER_BIP_CMAC128(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_CMAC128)) > 0)
#define IS_CIPHER_BIP_CMAC256(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_CMAC256)) > 0)
#define IS_CIPHER_BIP_GMAC128(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_GMAC128)) > 0)
#define IS_CIPHER_BIP_GMAC256(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_GMAC256)) > 0)

/* Encryption type */
#define WSC_ENCRTYPE_NONE    0x0001
#define WSC_ENCRTYPE_WEP     0x0002
#define WSC_ENCRTYPE_TKIP    0x0004
#define WSC_ENCRTYPE_AES     0x0008


#define MAX_RATE 10000

struct scan_SSID
{
	char ssid[MAX_SSID_LEN + 1];
	unsigned char SsidLen;
};
#define MAX_PROFILE_CNT 4

struct GNU_PACKED scan_BH_ssids
{
	unsigned long scan_cookie;
	unsigned char scan_channel_count;
	unsigned char scan_channel_list[32];
	unsigned char profile_cnt;
	struct scan_SSID scan_SSID_val[MAX_PROFILE_CNT];
};

struct GNU_PACKED garp_src_addr
{
	unsigned char addr[6];
};

struct GNU_PACKED garp_dev_addr
{
	unsigned char addr[6];
};

/*if you need change this value, need keep align with wappd*/
/*for triband, 3 bhbss + 3bhsta + 4eth(vlan)*/
#define MAX_BHDEV_CNT 10

struct GNU_PACKED garp_req_s
{
	unsigned char dev_cnt;
	unsigned int sta_count;
	struct garp_dev_addr dev_addr_list[MAX_BHDEV_CNT];
	struct garp_src_addr mac_addr_list[0];
};

struct vendor_map_element {
        u8 eid;
        u8 length;
        char oui[3]; /* 0x50 6F 9A */
	char mtk_ie_element[4];
        char type;
	char subtype;
	char root_distance;
	char controller_connectivity;
	short uplink_rate;
	char uplink_bssid[ETH_ALEN];
	char _5g_bssid[ETH_ALEN];
	char _2g_bssid[ETH_ALEN];
} __attribute__ ((packed));

int start_link_monitor_service(struct own_1905_device *ctx, struct bh_link_info *info);
int ap_selection_monitor_link(struct own_1905_device *ctx);
int ap_selection_update_vend_ie(struct own_1905_device *ctx, struct map_vendor_ie *ie, char con_connectivity);
int ap_selection_parse_scan_result(struct own_1905_device *ctx, struct wapp_scan_info *info);
int ap_selection_issue_connect(struct own_1905_device *ctx);
int ap_selection_handle_cli_state_change(struct own_1905_device *ctx, struct wapp_apcli_association_info *assoc);
void ap_selection_issue_scan(struct own_1905_device *ctx);
void ap_selection_reset_scan(struct own_1905_device *ctx);
void ap_selection_retrigger_scan(void *global_ctx, void *timer_ctx);
void issue_connect_timeout_handle(void *global_ctx, void *timer_ctx);
void ap_selection_reconnection_timeout(void *eloop_ctx, void *timeout_ctx);
void register_send_garp_req_to_wapp(void * eloop_ctx, void *user_ctx);
void send_garp_req_to_wapp(struct own_1905_device *ctx);
int is_bh_steer_triggered(struct own_1905_device *ctx);
Boolean is_mbh_conn_triggered(struct own_1905_device *ctx);
#ifdef SUPPORT_MULTI_AP
int get_mbh_state(struct own_1905_device *ctx, struct bh_link_entry **uncon, int *num_links);
struct bh_link_entry *get_connected_bh_entry(struct own_1905_device *ctx);
#endif
struct blacklisted_ap_list *lookup_for_bl_ap(struct own_1905_device *ctx);
#ifdef SUPPORT_MULTI_AP
int check_blacklist_ap(struct blacklisted_ap_list *bl_ap,
	struct own_1905_device *ctx,
	struct scan_bss_list *list_bss);
#endif
void mbh_handle_link_change(struct own_1905_device *ctx);
#ifdef SUPPORT_MULTI_AP
void ap_selection_get_uplink_ap_channel(struct own_1905_device *ctx,
	unsigned char *scan_channel_list,
	struct bh_link_entry *bh_entry);
#endif
int get_phyrate(int wireless_mode, signed char Rssi, int stream, int bw);
unsigned long power(unsigned long x, unsigned long y);
void update_apcli_info_in1905(struct mapd_global *global);
#ifdef SUPPORT_MULTI_AP
int map_estimate_candidate_device_upsteam_score(struct own_1905_device *ctx, struct scan_bss_list *bss_list);
#endif
void relative_score_calculation(struct own_1905_device *dev,struct _1905_map_device *_1905_device,struct _1905_map_device *est_1905_device);
int estimate_bss_phyrate(struct own_1905_device *ctx, struct radio_info_db *radio, struct bss_info *bss);
#ifdef SUPPORT_MULTI_AP
struct bh_link_entry * ap_selection_find_best_ap(struct own_1905_device *ctx, struct scan_bss_list **p_selected_bss);
#endif
void band_switch_by_cu_timeout(void *eloop_ctx, void *timeout_ctx);
void bh_switch_check_by_cu(struct mapd_global *global, u8 radio_idx);
void send_bh_steering_fail(void *eloop_ctx, void *timeout_ctx);
void restore_sec_bh_link(void *eloop_ctx, void *timeout_ctx);
#endif
