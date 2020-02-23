/*
 * Broadcom IEEE1905 library include file
 *
 * Copyright (C) 2019, Broadcom. All Rights Reserved.
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
 * $Id: ieee1905.h 776889 2019-07-12 08:07:32Z $
 */

#ifndef __IEEE1905_H__
#define __IEEE1905_H__

/*#ifdef __cplusplus
extern "C" {
#endif*/ /* __cplusplus */

#include <typedefs.h>
#include <bcmutils.h>
#include "ieee1905_datamodel.h"

#define IEEE1905_VERSION	1

#define BCM_REFERENCE(data)	((void)(data))

/* Error Codes */
#define IEEE1905_OK             0 /* Success */
#define IEEE1905_FAIL           1 /* Failed */
#define IEEE1905_AL_MAC_NOT_FOUND 2 /* AL MAC Address not Found */
#define IEEE1905_RADIO_NOT_FOUND 3 /* Interface ID not Found */
#define IEEE1905_BSS_NOT_FOUND  4 /* BSS Not found */
#define IEEE1905_STA_NOT_FOUND  5 /* STA Not found */
#define IEEE1905_STA_ALREADY_EXISTS  6 /* STA Already Exists */
#define IEEE1905_NOT_AGENT  7 /* Operation not supported as it is not agent */
#define IEEE1905_CONTROLLER_NOT_FOUND  8 /* Controller not found */
#define IEEE1905_NOT_CONTROLLER  9 /* Operation not supported as it is not controller */

#define IEEE1905_MAC_ADDR_LEN 6

#ifdef MULTIAP

/* IEEE1905 Library Initialization Flags */
#define I5_INIT_FLAG_MCHAN              0x0001  /* Multi Channel enabled or not */
#define I5_INIT_FLAG_GUEST_ENABLED      0x0002  /* Guest Network is enabled */

/* Device Flags */
#define I5_CONFIG_FLAG_CONTROLLER       0x0001  /* Supports Contoller */
#define I5_CONFIG_FLAG_AGENT            0x0002  /* Supports Agent */
#define I5_CONFIG_FLAG_REGISTRAR        0x0004  /* Supports Registrar */
#define I5_CONFIG_FLAG_START_MESSAGE    0x0008  /* Start Messaging. Only for internal usage */
#define I5_CONFIG_FLAG_CONTROLLER_FOUND 0x0010  /* Controller found after AP auto config search */
#define I5_CONFIG_FLAG_DWDS             0x0020  /* Its DWDS. WiFi onboarded */
#define I5_CONFIG_FLAG_CTRLAGENT        0x0040  /* Agent running on Controller */
#define I5_CONFIG_FLAG_MCHAN            0x0080  /* Multi channel enabled or not */
#define I5_CONFIG_FLAG_DEDICATED_BK     0x0100  /* Dedicated backhaul */
#define I5_CONFIG_FLAG_HAS_BH_BSS       0x0200  /* Agent has backhaul BSS */
#define I5_CONFIG_FLAG_GUEST_ENABLED    0x0400  /* Guest Network is enabled */

#define I5_IS_MULTIAP_CONTROLLER(flags) ((flags) & I5_CONFIG_FLAG_CONTROLLER)
#define I5_IS_MULTIAP_AGENT(flags)      ((flags) & I5_CONFIG_FLAG_AGENT)
#define I5_IS_REGISTRAR(flags)          ((flags) & I5_CONFIG_FLAG_REGISTRAR)
#define I5_IS_START_MESSAGE(flags)      ((flags) & I5_CONFIG_FLAG_START_MESSAGE)
#define I5_IS_CONTROLLER_FOUND(flags)   ((flags) & I5_CONFIG_FLAG_CONTROLLER_FOUND)
#define I5_IS_DWDS(flags)               ((flags) & I5_CONFIG_FLAG_DWDS)
#define I5_IS_CTRLAGENT(flags)          ((flags) & I5_CONFIG_FLAG_CTRLAGENT)
#define I5_IS_MCHAN_ENAB(flags)         ((flags) & I5_CONFIG_FLAG_MCHAN)
#define I5_IS_DEDICATED_BK(flags)       ((flags) & I5_CONFIG_FLAG_DEDICATED_BK)
#define I5_HAS_BH_BSS(flags)            ((flags) & I5_CONFIG_FLAG_HAS_BH_BSS)
#define I5_IS_GUEST_ENABLED(flags)      ((flags) & I5_CONFIG_FLAG_GUEST_ENABLED)

/* Ap capabilities bit flags. */
/* HT caps bit 0 is reserved. */
#define IEEE1905_AP_HTCAP_40MHZ			0x00000002	/* Bit 1 (00000010) */
#define IEEE1905_AP_HTCAP_SGI_40MHZ		0x00000004	/* Bit 2 (00000100) */
#define IEEE1905_AP_HTCAP_SGI_20MHZ		0x00000008	/* Bit 3 (00001000) */
/* HT RX Streams is represented by bits 5 and 4  */
#define IEEE1905_AP_HTCAP_RX_NSS_1		0x00000000	/* Bit 5 = 0 and bit 4 = 0 (00000000) */
#define IEEE1905_AP_HTCAP_RX_NSS_2		0x00000010	/* Bit 5 = 0 and bit 4 = 1 (00010000) */
#define IEEE1905_AP_HTCAP_RX_NSS_3		0x00000020	/* Bit 5 = 1 and bit 4 = 0 (00100000) */
#define IEEE1905_AP_HTCAP_RX_NSS_4		0x00000030	/* Bit 5 = 1 and bit 4 = 1 (00110000) */
/* HT TX Streams is represented by bits 7 and 6  */
#define IEEE1905_AP_HTCAP_TX_NSS_1		0x00000000	/* Bit 7 = 0 and bit 6 = 0 (00000000) */
#define IEEE1905_AP_HTCAP_TX_NSS_2		0x00000040	/* Bit 7 = 0 and bit 6 = 1 (01000000) */
#define IEEE1905_AP_HTCAP_TX_NSS_3		0x00000080	/* Bit 7 = 1 and bit 6 = 0 (10000000) */
#define IEEE1905_AP_HTCAP_TX_NSS_4		0x000000C0	/* Bit 7 = 0 and bit 6 = 0 (11000000) */

/* VHT caps represented by 2 bytes */
/* First byte bits 0 to 3 are reserved. */
#define IEEE1905_AP_VHTCAP_MU_BEAMFMR		0x00000010	/* Bit 4 (00010000) */
#define IEEE1905_AP_VHTCAP_SU_BEAMFMR		0x00000020	/* Bit 5 (00100000) */
#define IEEE1905_AP_VHTCAP_160MHZ		0x00000040	/* Bit 6 (01000000) */
#define IEEE1905_AP_VHTCAP_80p80MHZ		0x00000080	/* Bit 7 (10000000) */
/* VHT cap second byte */
#define IEEE1905_AP_VHTCAP_SGI_160MHZ		0x00000001	/* Bit 0 (00000001) */
#define IEEE1905_AP_VHTCAP_SGI_80MHZ		0x00000002	/* Bit 1 (00000010) */
/* VHT RX Streams is represented by bits 4, 3 and 2 */
#define IEEE1905_AP_VHTCAP_RX_NSS_1		0x00000000	/* Bit 4 = 0, Bit 3 = 0, Bit 2 = 0 (00000000) */
#define IEEE1905_AP_VHTCAP_RX_NSS_2		0x00000004	/* Bit 4 = 0, Bit 3 = 0, Bit 2 = 1 (00000100) */
#define IEEE1905_AP_VHTCAP_RX_NSS_3		0x00000008	/* Bit 4 = 0, Bit 3 = 1, Bit 2 = 0 (00001000) */
#define IEEE1905_AP_VHTCAP_RX_NSS_4		0x0000000C	/* Bit 4 = 1, Bit 3 = 1, Bit 2 = 0 (00001100) */
#define IEEE1905_AP_VHTCAP_RX_NSS_8		0x0000001C	/* Bit 4 = 0, Bit 3 = 0, Bit 2 = 0 (00011100) */
/* VHT RX Streams is represented by bits 7, 6 and 5 */
#define IEEE1905_AP_VHTCAP_TX_NSS_1		0x00000000	/* Bit 7 = 0, Bit 6 = 0, Bit 5 = 0 (00000000) */
#define IEEE1905_AP_VHTCAP_TX_NSS_2		0x00000020	/* Bit 7 = 0, Bit 6 = 0, Bit 5 = 1 (00100000) */
#define IEEE1905_AP_VHTCAP_TX_NSS_3		0x00000040	/* Bit 7 = 0, Bit 6 = 1, Bit 5 = 0 (01000000) */
#define IEEE1905_AP_VHTCAP_TX_NSS_4		0x00000060	/* Bit 7 = 0, Bit 6 = 1, Bit 5 = 1 (01100000) */
#define IEEE1905_AP_VHTCAP_TX_NSS_8		0x000000E0	/* Bit 7 = 1, Bit 6 = 1, Bit 5 = 1 (11100000) */

/* HE caps represented by 2 bytes */
/* HE Caps First byte bits 0 is reserved. */
#define IEEE1905_AP_HECAP_DL_OFDMA		0x00000002	/* Bit 1 (00000010) */
#define IEEE1905_AP_HECAP_UL_OFDMA		0x00000004	/* Bit 2 (00000100) */
#define IEEE1905_AP_HECAP_DL_MUMIMO_OFDMA	0x00000008	/* Bit 3 (00001000) */
#define IEEE1905_AP_HECAP_UL_MUMIMO_OFDMA	0x00000010	/* Bit 4 (00010000) */
#define IEEE1905_AP_HECAP_UL_MUMIMO		0x00000020	/* Bit 5 (00100000) */
#define IEEE1905_AP_HECAP_MU_BEAMFMR		0x00000040	/* Bit 6 (01000000) */
#define IEEE1905_AP_HECAP_SU_BEAMFMR		0x00000080	/* Bit 7 (10000000) */
/* HE cap second byte */
#define IEEE1905_AP_HECAP_160MHZ		0x00000001	/* Bit 0 (00000001) */
#define IEEE1905_AP_HECAP_80P80MHZ		0x00000002	/* Bit 1 (00000010) */
/* HE RX Streams is represented by bits 4, 3 and 2 */
#define IEEE1905_AP_HECAP_RX_NSS_1		0x00000000	/* Bit 4 = 0, Bit 3 = 0, Bit 2 = 0 (00000000) */
#define IEEE1905_AP_HECAP_RX_NSS_2		0x00000004	/* Bit 4 = 0, Bit 3 = 0, Bit 2 = 1 (00000100) */
#define IEEE1905_AP_HECAP_RX_NSS_3		0x00000008	/* Bit 4 = 0, Bit 3 = 1, Bit 2 = 0 (00001000) */
#define IEEE1905_AP_HECAP_RX_NSS_4		0x0000000C	/* Bit 4 = 0, Bit 3 = 1, Bit 2 = 1 (00001100) */
#define IEEE1905_AP_HECAP_RX_NSS_8		0x0000001C	/* Bit 4 = 1, Bit 3 = 1, Bit 2 = 1 (00011100) */
/* HE RX Streams is represented by bits 7, 6 and 5 */
#define IEEE1905_AP_HECAP_TX_NSS_1		0x00000000	/* Bit 7 = 0, Bit 6 = 0, Bit 5 = 0 (00000000) */
#define IEEE1905_AP_HECAP_TX_NSS_2		0x00000020	/* Bit 7 = 0, Bit 6 = 0, Bit 5 = 1 (00100000) */
#define IEEE1905_AP_HECAP_TX_NSS_3		0x00000040	/* Bit 7 = 0, Bit 6 = 1, Bit 5 = 0 (01000000) */
#define IEEE1905_AP_HECAP_TX_NSS_4		0x00000060	/* Bit 7 = 0, Bit 6 = 1, Bit 5 = 1 (01100000) */
#define IEEE1905_AP_HECAP_TX_NSS_8		0x000000E0	/* Bit 7 = 1, Bit 6 = 1, Bit 5 = 1 (11100000) */

/* Flags for Including bit for the Estimated Service Parameters Information field */
#define IEEE1905_INCL_BIT_ESP_BE  0x80  /* AC = BE */
#define IEEE1905_INCL_BIT_ESP_BK  0x40  /* AC = BK */
#define IEEE1905_INCL_BIT_ESP_VO  0x20  /* AC = VO */
#define IEEE1905_INCL_BIT_ESP_VI  0x10  /* AC = VI */

/* Flags for MultiAp extension subelement  */
#define IEEE1905_TEAR_DOWN	0x10	/* Bit 4 */
#define IEEE1905_FRONTHAUL_BSS	0x20	/* Bit 5 */
#define IEEE1905_BACKHAUL_BSS	0x40	/* Bit 6 */
#define IEEE1905_BACKHAUL_STA	0x80	/* Bit 7 */

enum {
  IEEE1905_CHAN_PREF_NON_OP = 0,
  IEEE1905_CHAN_PREF_1 = 1,
  IEEE1905_CHAN_PREF_2 = 2,
  IEEE1905_CHAN_PREF_3 = 3,
  IEEE1905_CHAN_PREF_4 = 4,
  IEEE1905_CHAN_PREF_5 = 5,
  IEEE1905_CHAN_PREF_6 = 6,
  IEEE1905_CHAN_PREF_7 = 7,
  IEEE1905_CHAN_PREF_8 = 8,
  IEEE1905_CHAN_PREF_9 = 9,
  IEEE1905_CHAN_PREF_10 = 10,
  IEEE1905_CHAN_PREF_11 = 11,
  IEEE1905_CHAN_PREF_12 = 12,
  IEEE1905_CHAN_PREF_13 = 13,
  IEEE1905_CHAN_PREF_14 = 14,
  IEEE1905_CHAN_PREF_15 = 15,
};
/* Encryption Types */
#define IEEE1905_ENCR_NONE  0x0001
#define IEEE1905_ENCR_WEP   0x0002
#define IEEE1905_ENCR_TKIP  0x0004
#define IEEE1905_ENCR_AES   0x0008

/* Authentication Types */
#define IEEE1905_AUTH_OPEN    0x0001
#define IEEE1905_AUTH_WPAPSK  0x0002
#define IEEE1905_AUTH_SHARED  0x0004
#define IEEE1905_AUTH_WPA     0x0008
#define IEEE1905_AUTH_WPA2    0x0010
#define IEEE1905_AUTH_WPA2PSK 0x0020
#define IEEE1905_AUTH_SAE     0x0040

#define IEEE1905_MAX_KEY_LEN    64

typedef enum i5_bh_sta_cmd {
  IEEE1905_BH_STA_ROAM_DISB_VAP_UP,
  IEEE1905_BH_STA_ROAM_ENAB_VAP_FOLLOW,
} t_i5_bh_sta_cmd;

#endif /* MULTIAP */

typedef struct ieee1905_msglevel {
  uint8 module_count; /* Number of modules */
  int *module;  /* List of modules where the log should be enabled */
  int level;    /* Print message level, Error - 1, Normal - 2, Info - 3 */
  int ifindex;
  unsigned char *ifmacaddr;
} ieee1905_msglevel_t;

#ifdef MULTIAP

/* AP Capability Flags */
/* Support Unassociated STA Link Metrics reporting on the channels its BSSs are
 * currently operating on
 * 0: Not supported
 * 1 : Supported
 */
#define IEEE1905_AP_CAPS_FLAGS_UNASSOC_RPT            0x80
/* Support Unassociated STA Link Metrics reporting on channels its BSSs are not
 * currently operating on.
 * 0: Not supported
 * 1 : Supported
 */
#define IEEE1905_AP_CAPS_FLAGS_UNASSOC_RPT_NON_CH     0x40
/* Support Agent-initiated RSSI-based Steering
 * 0: Not supported
 * 1 : Supported
 */
#define IEEE1905_AP_CAPS_FLAGS_AGENT_INIT_RSSI_STEER  0x20

/* Beacon metrics report response status. bit 7 and bit 6 */
#define IEEE1905_BEACON_REPORT_RESP_FLAG_SUCCESS  0x00  /* Beacon report received from STA(00) */
#define IEEE1905_BEACON_REPORT_RESP_FLAG_NO_REPORT  0x40  /* No report from STA(01) */
#define IEEE1905_BEACON_REPORT_RESP_FLAG_NO_SUPPORT  0x80 /* STA dont support beacon report(10) */
#define IEEE1905_BEACON_REPORT_RESP_FLAG_UNSPECIFIED  0xC0  /* Unspecified failure(11) */

/* STEER Request Flags */
#define IEEE1905_STEER_FLAGS_MANDATE        0x0001  /* Request is a steering mandate */
#define IEEE1905_STEER_FLAGS_OPPORTUNITY    0x0002  /* Request is a steering opportunity */
#define IEEE1905_STEER_FLAGS_RSSI           0x0004 /* Request is a RSSI based */
#define IEEE1905_STEER_FLAGS_DISASSOC_IMNT  0x0008  /* BTM Disassociation Imminent bit */
#define IEEE1905_STEER_FLAGS_BTM_ABRIDGED   0x0010  /* BTM Abridged bit */

/* STEER Request Flag Helper */
#define IEEE1905_IS_STEER_MANDATE(flag) ((flag) & IEEE1905_STEER_FLAGS_MANDATE)
#define IEEE1905_IS_STEER_OPPORTUNITY(flag) ((flag) & IEEE1905_STEER_FLAGS_OPPORTUNITY)
#define IEEE1905_IS_STEER_RSSI(flag) ((flag) & IEEE1905_STEER_FLAGS_RSSI)
#define IEEE1905_IS_DISASSOC_IMNT(flag) ((flag) & IEEE1905_STEER_FLAGS_DISASSOC_IMNT)
#define IEEE1905_IS_BTM_ABRIDGED(flag) ((flag) & IEEE1905_STEER_FLAGS_BTM_ABRIDGED)

/* MAP Flags */
#define IEEE1905_MAP_FLAG_FRONTHAUL 0x01  /* Fronthaul BSS */
#define IEEE1905_MAP_FLAG_BACKHAUL  0x02  /* Backhaul BSS */
#define IEEE1905_MAP_FLAG_STA       0x04  /* STA */
#define IEEE1905_MAP_FLAG_GUEST     0x08  /* Guest BSS */

#define I5_IS_BSS_FRONTHAUL(flags)	((flags) & IEEE1905_MAP_FLAG_FRONTHAUL)
#define I5_IS_BSS_BACKHAUL(flags)	((flags) & IEEE1905_MAP_FLAG_BACKHAUL)
#define I5_IS_BSS_STA(flags)		((flags) & IEEE1905_MAP_FLAG_STA)
#define I5_IS_BSS_GUEST(flags)		((flags) & IEEE1905_MAP_FLAG_GUEST)

/* MAP policy receied flags */
#define MAP_POLICY_RCVD_FLAG_STEER          0x01  /* Recieved Steering Policy */
#define MAP_POLICY_RCVD_FLAG_METRIC_REPORT  0x02  /* Recieved Metrics Reporting Policy */

/* Define a Generic List */
typedef struct ieee1905_glist {
	uint count;			/* Count of list of objects */
	dll_t head;			/* Head Node of list of objects */
} ieee1905_glist_t;

/* Initialize generic list */
static inline void ieee1905_glist_init(ieee1905_glist_t *list)
{
	list->count = 0;
	dll_init(&(list->head));
}

/* Append a node to generic list */
static inline void ieee1905_glist_append(ieee1905_glist_t *list, dll_t *new_obj)
{
	dll_append((dll_t *)&(list->head), new_obj);
	++(list->count);
}

/* Delete a node from generic list */
static inline void ieee1905_glist_delete(ieee1905_glist_t *list, dll_t *obj)
{
	dll_delete(obj);
	--(list->count);
}

/* All the configurations required for IEEE1905 */
typedef struct {
  unsigned char basic_caps; /* AP Basic caps of Type IEEE1905_AP_CAPS_FLAGS_XXX */
  unsigned int flags;       /* Flags of type I5_INIT_FLAG_XXX */
  unsigned short prim_vlan_id;  /* Primary VLAN ID */
  unsigned short sec_vlan_id; /* Secondary VLAN ID */
  unsigned short vlan_ether_type; /* VLAN ether type */
} ieee1905_config;

typedef struct {
  dll_t node;			/* self referencial (next,prev) pointers of type dll_t */
  unsigned char mac[IEEE1905_MAC_ADDR_LEN];
} ieee1905_sta_list;

typedef struct {
  dll_t node;			/* self referencial (next,prev) pointers of type dll_t */
  unsigned char bssid[IEEE1905_MAC_ADDR_LEN]; /* Indicates a target BSSID for steering
                                               * Wildcard BSSID is represented by
                                               * FF:FF:FF:FF:FF:FF
                                               */
  unsigned char target_op_class;  /* Target BSS Operating Class */
  unsigned char target_channel; /* Target BSS Channel Number for channel on which the Target BSS
                                 * is transmitting Beacon frames
                                 */
} ieee1905_bss_list;

typedef struct {
  unsigned char neighbor_al_mac[IEEE1905_MAC_ADDR_LEN]; /* AL ID from where request came */
  unsigned char source_bssid[IEEE1905_MAC_ADDR_LEN]; /* BSSID where the STA is associated */
  unsigned short request_flags;  /* IEEE1905_STEER_FLAGS_XXX values */
  unsigned short opportunity_window;  /* Steering opportunity window. Time period in seconds
                                       * (from reception of the Steering Request message) for which
                                       * the request is valid.
                                       */
  unsigned short dissassociation_timer; /* BTM Disassociation Timer. Time period in TUs for the
                                         * disassociation timer in BTM Request.
                                         */
  ieee1905_glist_t sta_list;  /* List of ieee1905_sta_list type objects */
  ieee1905_glist_t bss_list;  /* List of ieee1905_bss_list type objects */
} ieee1905_steer_req;

/* Contains the STAs which needs to be blocked or unblocked */
typedef struct {
  unsigned char source_bssid[IEEE1905_MAC_ADDR_LEN];	/* BSS for which the client blocking
							 * request applies
							 */
  unsigned char unblock;  /* 0 to block and 1 to unblock */
  unsigned short time_period; /* How much time to block */
  ieee1905_glist_t sta_list;  /* List of ieee1905_sta_list type objects */
} ieee1905_block_unblock_sta;

/* BSS Transition response */
typedef struct {
  unsigned char neighbor_al_mac[IEEE1905_MAC_ADDR_LEN]; /* AL ID from where steer request came */
  unsigned short request_flags;  /* IEEE1905_STEER_FLAGS_XXX values which we got from request */
  unsigned char source_bssid[IEEE1905_MAC_ADDR_LEN]; /* BSSID where the STA is associated */
  unsigned char sta_mac[IEEE1905_MAC_ADDR_LEN]; /* STA MAC address */
  unsigned char status; /* BTM Status code as reported by the STA in the BTM Response */
  unsigned char trgt_bssid[IEEE1905_MAC_ADDR_LEN]; /* Target BSSID from BTM Response */
} ieee1905_btm_report;

typedef struct {
  dll_t node;		/* Self refrential pointer dll_t */
  unsigned char mac[IEEE1905_MAC_ADDR_LEN];	/* Radio mac address */
  unsigned char policy;		/* Steering policy */
  unsigned char bssload_thld;	/* Channel utilization (BSS load) threshhold */
  unsigned char rssi_thld;		/* RCPI values. 0 if P < -109.5 dBm
                               * 1-219. Power levels in the range –109.5<=P<0. RCPI=(2 X (P+110))
                               * 220 if P >= 0 dBm
                               * 255 if measurement not available
                               */
} ieee1905_bss_steer_config;

/* STA Metric Policy Flags */
#define MAP_STA_MTRC_TRAFFIC_STAT 0x80  /* Associated STA Traffic Stats Inclusion Policy.
                                         * 0 : Dont send AssocSTA Traffic Stats in AP Metrics Resp
                                         * 1 : Send AssocSTA Traffic Stats
                                         */
#define MAP_STA_MTRC_LINK_MTRC  0x40    /* Associated STA Link Metrics Inclusion Policy.
                                         * 0 : Dont send STA Link Metrics TLV in AP Metrics Resp
                                         * 1 : Send STA Link Metrics TLV
                                         */
typedef struct {
  dll_t node;		/* Self refrential pointer of type dll_t*/
  unsigned char mac[IEEE1905_MAC_ADDR_LEN];	/* Radio mac address */
  unsigned char sta_mtrc_rssi_thld; /* STA Metrics Reporting RSSI Threshold
                                     * 0 - Do not report STA Metrics based on RSSI Threshold
                                     * 1 - 220: RSSI threshold (RCPI)
                                     */
  unsigned char sta_mtrc_rssi_hyst; /* STA Metrics Reporting RSSI Hysteresis Margin Override
                                     * coded as an unsigned integer in units of decibels (dB)
                                     * 0 : Use Agents implementation specific default RSSI
                                     * Hysteresis margin
                                     * >0 : RSSI Hysteresis margin value
                                     */
  unsigned char ap_mtrc_chan_util;	/* AP Metrics Channel Utilization Reporting Threshold
                                     * 0 : Do not report AP Metrics based on channel utilization
                                     * Threshold
                                     * >0 : AP Metrics channel utilization threshold
                                     */
  unsigned char sta_mtrc_policy_flag; /* This is of type MAP_STA_MTRC_XXX_XXX */
} ieee1905_ifr_metricrpt;

typedef struct {
  unsigned char ap_rpt_intvl;		/* Time Interval to send ap metric reports in seconds */
  ieee1905_glist_t ifr_list;		/* List of ieee1905_ifr_metricrpt type objects */
} ieee1905_metricrpt_config;

typedef struct {
  ieee1905_glist_t no_steer_sta_list;	/* List of ieee1905_sta_list type objects */
  ieee1905_glist_t no_btm_steer_sta_list; /* List of ieee1905_sta_list type objects */
  ieee1905_glist_t steercfg_bss_list;	/* List of ieee1905_bss_steer_config type objects */
  ieee1905_metricrpt_config  metricrpt_config; /* Metric Reporting Policy */
} ieee1905_policy_config;

/* Client association control information */
typedef struct {
  unsigned char source_bssid[IEEE1905_MAC_ADDR_LEN];  /* BSS where the STA is associated */
  unsigned char trgt_bssid[IEEE1905_MAC_ADDR_LEN]; /* BSS where the STA is going to steer to */
  unsigned char unblock;  /* 0 to block and 1 to unblock */
  unsigned short time_period; /* Blocking validity period in seconds */
  unsigned char sta_mac[IEEE1905_MAC_ADDR_LEN]; /* STA to be blocked or unblocked */
} ieee1905_client_assoc_cntrl_info;

typedef struct {
  unsigned char chan;
  unsigned char n_sta; /* number of sta for the channel */
  unsigned char *mac_list; /* list of mac addresses of sta to be monitor */
}  __attribute__((__packed__)) unassoc_query_per_chan_rqst;

/* UnAssociated STA Link Metrics Query information */
typedef struct {
  unsigned char neighbor_al_mac[IEEE1905_MAC_ADDR_LEN]; /* AL ID from where request came (Used only at agent) */
  unsigned char opClass;  /* Operating Class */
  unsigned char chCount; /* number of channels */
  unassoc_query_per_chan_rqst *data;
} ieee1905_unassoc_sta_link_metric_query;

/* Containns each unassociated STAs link metric */
typedef struct {
  dll_t node;
  unsigned char mac[IEEE1905_MAC_ADDR_LEN];  /* STA MAC address */
  unsigned char channel;  /* Channel on which the RSSI is measured */
  struct timespec queried;  /* Time at which the link metric calculated (Used only at agent) */
  unsigned int delta; /* The time delta in ms between queried and report was sent */
  unsigned char rcpi; /* uplink RCPI */
} ieee1905_unassoc_sta_link_metric_list;

/* UnAssociated STA Link Metrics */
typedef struct {
  unsigned char neighbor_al_mac[IEEE1905_MAC_ADDR_LEN]; /* AL ID from where request came (Used only at agent) */
  unsigned char opClass;  /* Operating Class */
  ieee1905_glist_t sta_list; /* List of STAs of type ieee1905_unassoc_sta_link_metric_list */
} ieee1905_unassoc_sta_link_metric;

typedef struct {
  unsigned char op_class;
  unsigned char chan;
}  __attribute__((__packed__)) operating_rpt_opclass_chan_list;

typedef struct {
  unsigned char radio_mac[IEEE1905_MAC_ADDR_LEN];
  unsigned char n_op_class;
  operating_rpt_opclass_chan_list *list;
  unsigned char tx_pwr;
} ieee1905_operating_chan_report;

/* Network Key Type */
typedef struct {
  unsigned char	key_len;
  unsigned char	key[IEEE1905_MAX_KEY_LEN];
} ieee1905_network_key_type;

/* BSS Info of M2 WSC Message */
typedef struct {
  dll_t                     node;
  unsigned char             ALID[IEEE1905_MAC_ADDR_LEN];
  unsigned char             band_flag;
  ieee1905_ssid_type        ssid;
  unsigned short            AuthType; /* Of Type IEEE1905_AUTH_XXX */
  unsigned short            EncryptType;  /* Of Type IEEE1905_ENCR_XXX */
  ieee1905_network_key_type NetworkKey;
  unsigned char             BackHaulBSS;
  unsigned char             FrontHaulBSS;
  unsigned char		    TearDown;
  unsigned char		    Guest;
} ieee1905_client_bssinfo_type;

/* Interface Info */
typedef struct {
  unsigned short chanspec;  /* Chanspec of the interface */
  ieee1905_ap_caps_type ap_caps;  /* AP capability */
  unsigned char mapFlags; /* Of Type IEEE1905_MAP_FLAG_XXX */
  unsigned char bssid[IEEE1905_MAC_ADDR_LEN]; /* BSSID of the AP where the bSTA is connected */
} ieee1905_ifr_info;

typedef struct {
  unsigned char neighbor_al_mac[IEEE1905_MAC_ADDR_LEN]; /* AL ID from where request came (Used only at agent) */
  unsigned char bh_sta_mac[IEEE1905_MAC_ADDR_LEN]; /* Backhaul STA mac adress to be steered */
  unsigned char trgt_bssid[IEEE1905_MAC_ADDR_LEN]; /* BSS where the backhaul STA is going to steer to */
  unsigned char opclass; /* Target BSS operating class */
  unsigned char channel; /* Target BSS channel */
  unsigned char resp_status_code; /* status code for backhaul steering response */
} ieee1905_backhaul_steer_msg;

typedef struct {
  unsigned char neighbor_al_mac[IEEE1905_MAC_ADDR_LEN]; /* AL ID from where request came (Used only at agent) */
  unsigned char sta_mac[IEEE1905_MAC_ADDR_LEN]; /* MAC address of the associated STA */
  unsigned char opclass; /* Operating Class field to be specified in the Beacon request */
  unsigned char channel; /* Channel Number field to be specified in the Beacon request */
  unsigned char bssid[IEEE1905_MAC_ADDR_LEN]; /* BSSID field to be specified in the Beacon request */
  unsigned char reporting_detail; /* Reporting Detail */
  ieee1905_ssid_type ssid;/* SSID */
  unsigned char ap_chan_report_count; /* Number of AP Channel report */
  unsigned char ap_chan_report_len; /* Length of the AP channel report */
  unsigned char *ap_chan_report;  /* All AP Channel report. Each report has,
                                   * 1 octet Length of Each AP Channel report (k). Which in turn has,
                                   *    1 octet Operating class
                                   *    (k - 1) length of Channel list. Each channel is 1 octet
                                   */
  unsigned char element_ids_count;  /* Number of element IDs */
  unsigned char *element_list; /* Comprises a list of Element IDs to be included */
} ieee1905_beacon_request;

typedef struct {
  unsigned char neighbor_al_mac[IEEE1905_MAC_ADDR_LEN]; /* AL ID from where request came (Used only at agent) */
  unsigned char sta_mac[IEEE1905_MAC_ADDR_LEN]; /* MAC address of the associated STA */
  unsigned char response; /* Response status of type IEEE1905_BEACON_REPORT_RESP_FLAG_XXX */
  unsigned char report_element_count; /* Number of measurement report element TLV */
  unsigned short report_element_len;  /* Length of all measurement report elements */
  unsigned char *report_element;  /* All the measurement report elements */
} ieee1905_beacon_report;

#define IEEE1905_MAX_VNDR_DATA_BUF    I5_PACKET_BUF_LEN

typedef struct {
  unsigned char neighbor_al_mac[IEEE1905_MAC_ADDR_LEN]; /* While Sending/Encoding a Vendor MSG/TLV this is Destination AL_MAC
                                                         * While Receiving/Decoding a Vendor MSG/TLV this is Source AL_MAC */
  unsigned char *vendorSpec_msg; /* Allocate Dynamic mem for Vendor data from App */
  unsigned int vendorSpec_len; /* Vendor data length */
} ieee1905_vendor_data;

/* MAP Higher Layer Data structure : HLE can use it while sending HLD */
typedef struct i5_higher_layer_data_send {
  uint32 tag;		/* Higher Layer Data TAG. Its value is fixed to I5_API_CMD_HLE_SEND_HL_DATA (44)
			* tag field is in little endian format */
  uint32 length;	/* Length : Total Length of all the fields of this structure object, which is =
			* 4 + 4 + 6 + 1 + Paylaod (Variable)
			* length field is in little endian format */
  uint8 dst_al_mac[IEEE1905_MAC_ADDR_LEN];
			/* Destination AL MAC Address : AL Mac address of receiver of HLD */
  uint8 protocol;	/* Protocol of Higher Layer Data */
  uint8* payload;	/* Higher layer protocol payload to be received from other MAP entity */

} i5_higher_layer_data_send_t;

/* MAP Higher Layer Data structure : HLE can use it while receiving HLD */
typedef struct i5_higher_layer_data_recv {
  uint32 tag;		/* Higher Layer Data TAG. Its value is fixed to I5_API_CMD_HLE_SEND_HL_DATA (44)
			* tag field is in little endian format */
  uint32 length;	/* Length : Total Length of all the fields of this structure object, which is =
			* 4 + 4 + 6 + 1 + 1 + Header length (22) + Paylaod (Variable)
			* length field is in little endian format */
  uint8 src_al_mac[IEEE1905_MAC_ADDR_LEN];
			/* Source AL MAC Address : AL Mac address of sender of HLD */
  uint8 protocol;	/* Protocol of Higher Layer Data */
  uint8 header_length;	/* Length of the header = 22 Octets; which is Ethernet frame Header Length
			* (14 octets) + 1905.1 CMDU header Length (8 octets)
			*/
  uint8* header;	/* It is the whole Ethernet
			* frame header (14 octets) + 1905.1 CMDU header (8 octets)
			*/
  uint8* payload;	/* Higher layer protocol payload to be received from other MAP entity */

} i5_higher_layer_data_recv_t;

/* Made common across 1905 and WBD2 stack */
#define BAND_INV  0x0
#define BAND_2G   0x1
#define BAND_5GL  0x2
#define BAND_5GH  0x4

#define REGCLASS_40MHZ_LAST	127
#define REGCLASS_MAX_COUNT	 20
#define REGCLASS_24G_FIRST	 81
#define REGCLASS_24G_LAST	 84
#define REGCLASS_5GL_FIRST	115
#define REGCLASS_5GL_LAST	120
#define REGCLASS_5GH_FIRST	121
#define REGCLASS_5GH_LAST	127
#define CHANNEL_24G_FIRST	  1
#define CHANNEL_24G_LAST	 14
#define CHANNEL_5GL_FIRST	 36
#define CHANNEL_5GL_LAST	 64
#define CHANNEL_5GH_FIRST	100
#define CHANNEL_5GH_LAST	165
#define REGCLASS_160MHZ		129

typedef enum ieee1905_tlv_err_codes {
  ieee1905_tlv_err_reserved = 0x00,
  ieee1905_tlv_err_sta_associated = 0x01,
  ieee1905_tlv_err_sta_not_associated = 0x02,
  ieee1905_tlv_err_client_cap_report_error = 0x03,
  ieee1905_tlv_err_backhaul_steer_reject_wrong_chan = 0x04,
  ieee1905_tlv_err_backhaul_steer_reject_low_signal = 0x05,
  ieee1905_tlv_err_backhaul_steer_reject_auth = 0x06,
} ieee1905_tlv_err_codes_t;

typedef enum i5_msg_types_with_vndr_tlv {

  i5MsgMultiAPPolicyConfigRequestValue = 1,
  i5MsgMultiAPGuestSsidValue = 2,

} i5_msg_types_with_vndr_tlv_t;

/** @brief Callback to inform about the device creation(i5_dm_device_type) in the data model
 *
 * @param pdevice	Pointer to the device created. Can be self device also
 *
 */
typedef void ieee1905_device_init(i5_dm_device_type *pdevice);

/** @brief Callback to inform about the device leave(i5_dm_device_type) in the data model
 *
 * @param pdevice	Pointer to the device leaving. Can be self device also
 *
 */
typedef void ieee1905_device_deinit(i5_dm_device_type *pdevice);

/** @brief Callback to inform about the neighbor creation(i5_dm_1905_neighbor_type) in the data model
 *
 * @param pneighbor	Pointer to the neighbor created
 *
 */
typedef void ieee1905_neighbor_init(i5_dm_1905_neighbor_type *pneighbor);

/** @brief Callback to inform about the neighbor leave(i5_dm_1905_neighbor_type) in the data model
 *
 * @param pneighbor	Pointer to the neighbor leaving
 *
 */
typedef void ieee1905_neighbor_deinit(i5_dm_1905_neighbor_type *pneighbor);

/** @brief Callback to inform about the interface creation(i5_dm_interface_type) in the data model
 *
 * @param pdevice	Pointer to the interface created. Can be self interface also
 *
 */
typedef void ieee1905_interface_init(i5_dm_interface_type *pinterface);

/** @brief Callback to inform about the interface leaving(i5_dm_interface_type) in the data model
 *
 * @param pdevice	Pointer to the interface leave. Can be self interface also
 *
 */
typedef void ieee1905_interface_deinit(i5_dm_interface_type *pinterface);

/** @brief Callback to inform about the BSS creation(i5_dm_bss_type) in the data model
 *
 * @param pdevice	Pointer to the BSS created. Can be self BSS also
 *
 */
typedef void ieee1905_bss_init(i5_dm_bss_type *pbss);

/** @brief Callback to inform about the BSS leave(i5_dm_bss_type) in the data model
 *
 * @param pdevice	Pointer to the BSS leaving. Can be self BSS also
 *
 */
typedef void ieee1905_bss_deinit(i5_dm_bss_type *pbss);

/** @brief Callback to inform about the Client creation(i5_dm_clients_type) in the data model
 *
 * @param pdevice	Pointer to the Client created. Can be the STA joined on the same device also
 *
 */
typedef void ieee1905_client_init(i5_dm_clients_type *pclient);

/** @brief Callback to inform about the Client leave(i5_dm_clients_type) in the data model
 *
 * @param pdevice	Pointer to the Client leaving. Can be the STA joined on the same device also
 *
 */
typedef void ieee1905_client_deinit(i5_dm_clients_type *pclient);

/**
 * Callback function to notify assoc/disassoc
 *
 * @param bssid	  BSSID of the BSS on which client notification came
 * @param mac		  MAC address of the client
 * @param isAssoc 1 If Assoc Notification. 0 If Disassoc Notification
 */
typedef void ieee1905_assoc_disassoc(unsigned char *bssid, unsigned char *mac, int isAssoc);

/** @brief Callback function to steer the STA(s)
 *
 * @param steer_req	steer request information
 *
 */
typedef void ieee1905_steer_request(ieee1905_steer_req *steer_req, ieee1905_vendor_data *msg_data);

/** @brief Callback function to Block/Unblock STA(s)
 *
 * @param block_unblock_sta	Block Unblock STAs information
 *
 */
typedef void ieee1905_block_unblock_sta_request(ieee1905_block_unblock_sta *block_unblock_sta);

/** @brief Callback function for preparing channel preference
 *
 * @param i5_intf		Prepare channel preference for this interface
 * @param cp			Channel preference Array
 *
 */
typedef void ieee1905_prepare_channel_pref(i5_dm_interface_type *i5_intf,
	ieee1905_chan_pref_rc_map_array *cp);

/** @brief Callback function to receive Channel selection request
 *
 * @param al_mac		Device Mac address
 * @param interface_mac		Interface Mac address
 * @param cp			Channel prefernce information from controller
 * @param rclass_local_count	Number of local preferences
 * @param local_chan_pref	Local Channel Preferences
 *
 */
typedef void ieee1905_recv_channel_selection_request(unsigned char *al_mac, unsigned char *interface_mac,
  ieee1905_chan_pref_rc_map_array *cp, unsigned char rclass_local_count,
  ieee1905_chan_pref_rc_map *local_chan_pref);

/** @brief Callback function to Send operating channel report
 */
typedef void ieee1905_send_opchannel_report(void);

/** @brief Callback function to set Tx power limit
 *
 * @param ifname          Interface Name
 * @param tx_power_limit	Tx power limit
 *
 */
typedef void ieee1905_tx_power_limit(char *ifname, unsigned char tx_power_limit);

/** @brief Callback function to get the backhaul link metric
 *
 * @param ifname          Interface Name
 * @param interface_mac		Neighbor Interface Mac address
 * @param metric          Link metric to be filled
 *
 * @return		            status of the call. 0 Success. Non Zero Failure
 */
typedef int ieee1905_get_backhaul_link_metric(char *ifname, unsigned char *interface_mac,
  ieee1905_backhaul_link_metric *metric);

/** @brief Callback function to get Interface metrics.
 *
 * @param ifname  Interface name
 * @param ifr_mac Interface MAC address
 * @param metric  Metric to be filled
 *
 * @return        status of the call. 0 Success. Non Zero Failure
 */
typedef int ieee1905_get_interface_metric(char *ifname, unsigned char *ifr_mac,
  ieee1905_interface_metric *metric);

/** @brief Callback function to get AP metrics.
 *
 * @param ifname  Interface name of the BSS
 * @param bssid   BSSID
 * @param metric  Metric to be filled
 *
 * @return        status of the call. 0 Success. Non Zero Failure
 */
typedef int ieee1905_get_ap_metric(char *ifname, unsigned char *bssid, ieee1905_ap_metric *metric);

/** @brief Callback function to get the associated STA link metrics and STA traffic stats.
 *
 * @param ifname            Interface name where the STA is associated
 * @param bssid             BSSID where the STA is associated
 * @param sta_mac           MAC address of the STA
 * @param metric            Metric to be filled
 * @param traffic_stats     Traffic stats to be filled
 * @param out_vndr_data     Out Obj : Vendor Specific Message data in TLV format
 *
 * @return        status of the call. 0 Success. Non Zero Failure
 */
typedef int ieee1905_get_assoc_sta_metric(char *ifname, unsigned char *bssid, unsigned char *sta_mac,
  ieee1905_sta_link_metric *metric, ieee1905_sta_traffic_stats *traffic_stats,
  ieee1905_vendor_data *out_vndr_data);

/** @brief Callback function to get the un associated STA link metrics. Once the metrics is
 * get from the driver, the application should call send "ieee1905_send_unassoc_sta_link_metric"
 * funtion to actually send the data to requested neighbor
 *
 * @param query   Query to be sent
 *
 * @return        status of the call. 0 Success. Non Zero Failure
 */
typedef int ieee1905_get_unassoc_sta_metric(ieee1905_unassoc_sta_link_metric_query *query);

/** @brief Callback function to Create BSS on the interface
 *
 * @param ifname      Interface Name
 * @param bssinfo_list         List of all BSS in this radio to be added, if List is passed as NULL
 * just needed a reboot after all M2 are processed and all interfaces are configured.
 * It is a list of type ieee1905_client_bssinfo_type objects.
 *
 * @return		            status of the call. 0 Success. Non Zero Failure
 */
typedef int ieee1905_create_bss_on_ifr(char *ifname,
  ieee1905_glist_t *bssinfo_list);

/** @brief Callback function to get Interface details
 *
 * @param ifname      Interface Name
 * @param erase       Interface Info structure to be filled
 *
 * @return            status of the call. 0 Success. Non Zero Failure
 */
typedef int ieee1905_get_interface_info(char *ifname, ieee1905_ifr_info *info);

/** @brief Callback function to steer the backhaul STA
 *
 * @param ifname      Interface Name
 * @param bh_steer_req	backhaul steer request information
 *
 */
typedef void ieee1905_backhaul_steer_request(char *ifname, ieee1905_backhaul_steer_msg *bh_steer_req);

/** @brief Callback function to send beacon request to a station
 *
 * @param ifname      Interface Name
 * @param bssid       BSS on which STA is associated
 * @param query       beacon metrics query data
 *
 * @return            status of the call. 0 Success. Non Zero Failure
 */
typedef int ieee1905_beacon_metrics_query(char *ifname, unsigned char *bssid,
  ieee1905_beacon_request *query);

/** @brief Callback function to inform about new policy configuration
 *
 * @param policy          New Policy Configuration recieved from the controller
 * @param rcvd_policies   Flag of type MAP_POLICY_RCVD_FLAG_XXX which tells the policy recieved
 * @param in_vendor_tlv       In Obj : Vendor Specific TLV data in TLV format
 *
 * @return            status of the call. 0 Success. Non Zero Failure
 */
typedef void ieee1905_configure_policy(ieee1905_policy_config *policy, unsigned short rcvd_policies,
	ieee1905_vendor_data *in_vendor_tlv);

/** @brief Callback function to process Vendor Specific Messages at Application Layer
  *
 * @param msg_data  Vendor Specific Message data
 *
 * @return            status of the call. 0 on Success, Non zero on Failure.
 */
typedef int ieee1905_process_vendor_specific_msg(ieee1905_vendor_data *msg_data);

/** @brief Callback function to get Vendor Specific TLV from Application Layer, for Exisiting 1905 messsge types
  *
 * @param msg_type  Exisiting 1905 Message Type which can send Vendor Specific TLV
 * @param out_vendor_tlv  Out Obj : Vendor Specific TLV data
 *
 * @return            void
 */
typedef void ieee1905_get_vendor_specific_tlv(i5_msg_types_with_vndr_tlv_t msg_type,
  ieee1905_vendor_data *out_vendor_tlv);

/** @brief Callback function to inform the associated STA link metrics response from agent.
 *
 * @param al_mac        AL MAC address of the device from where the STA link metrics came
 * @param bssid         BSSID where the STA is associated
 * @param sta_mac       MAC address of the STA
 * @param metric        Metric got from the Agent
 */
typedef void ieee1905_asoc_sta_metric_resp(unsigned char *al_mac, unsigned char *bssid,
  unsigned char *sta_mac, ieee1905_sta_link_metric *metric);

/** @brief Callback function to inform the unassociated STA link metrics response from agent.
 *
 * @param al_mac    AL MAC address of the device from where the Unassociated STA link metrics came
 * @param metric    Metric got from the Agent
 */
typedef void ieee1905_unasoc_sta_metric_resp(unsigned char *al_mac,
  ieee1905_unassoc_sta_link_metric *metric);

/** @brief Callback function to inform the beacon metrics response from agent.
 *
 * @param al_mac    AL MAC address of the device from where the Beacon metrics came
 * @param report    Beacon report got from the Agent
 */
typedef void ieee1905_beacon_metric_resp(unsigned char *al_mac, ieee1905_beacon_report *report);

/** @brief Callback function to inform AP auto configuration
 *
 * @param al_mac	  AL Mac address of the remote device
 * @param radio_mac       Mac address of the radio which is configured
 * @param if_band	  Band in which the radio is confiugred (INV, 2G, 5GL, 5GH)
 */
typedef void ieee1905_ap_configured(unsigned char *al_mac, unsigned char *radio_mac, int if_band);

/** @brief Callback function to inform AP auto configuration
 *
 * @param al_mac			AL Mac address of the remote device
 * @param operating_chan_report
 */
typedef void ieee1905_operating_channel_report(unsigned char *al_mac, ieee1905_operating_chan_report *chan_report);

/** @brief Callback function to inform steering btm report
 *
 * @param btm_report	btm report received from the agent.
 */
typedef void ieee1905_steering_btm_report(ieee1905_btm_report *btm_report);

/** @brief Callback function to inform higher layer data
 *
 * @param protocol	Higher layer data protocol field
 * @param data      Higher layer data
 * @param len       Length of the data
 * @param hdr_len   Length of the ethernet header and message header
 */
typedef void ieee1905_higher_layer_data(unsigned char *al_mac, unsigned char protocol,
  unsigned char *payload, unsigned int payload_length, unsigned char *header, unsigned char header_length);

/** @brief Callback to inform about the interface channel change in the data model
 *
 * @param pinterface	Pointer to the interface created. Can be self interface also
 *
 */
typedef void ieee1905_interface_chan_change(i5_dm_interface_type *pinterface);

/** @brief Callback to inform 1905 AP-Autoconfiguration Response message from controller
 *
 * @param pDeviceController   Pointer to the controller device model
 *
 */
typedef void ieee1905_ap_auto_config_resp(i5_dm_device_type *pDeviceController);

/** @brief Callback to inform 1905 AP-Autoconfiguration Search message sent
 */
typedef void ieee1905_ap_auto_config_search_sent();

/** @brief Generic callback to set backhaul sta parameters
 *
 * @param cmd   Selects the command to backhaul STA
 *
 */
typedef void ieee1905_set_bh_sta_params(t_i5_bh_sta_cmd cmd);

typedef struct ieee1905_call_bks {
  ieee1905_device_init *device_init;
  ieee1905_device_deinit *device_deinit;
  ieee1905_neighbor_init *neighbor_init;
  ieee1905_neighbor_deinit *neighbor_deinit;
  ieee1905_interface_init *interface_init;
  ieee1905_interface_deinit *interface_deinit;
  ieee1905_bss_init *bss_init;
  ieee1905_bss_deinit *bss_deinit;
  ieee1905_client_init *client_init;
  ieee1905_client_deinit *client_deinit;
  ieee1905_assoc_disassoc *assoc_disassoc;
  ieee1905_steer_request *steer_req;
  ieee1905_block_unblock_sta_request *block_unblock_sta_req;
  ieee1905_prepare_channel_pref *prepare_channel_pref;
  ieee1905_recv_channel_selection_request *recv_chan_selection_req;
  ieee1905_send_opchannel_report *send_opchannel_rpt;
  ieee1905_tx_power_limit *set_tx_power_limit;
  ieee1905_get_backhaul_link_metric *backhaul_link_metric;
  ieee1905_get_interface_metric *interface_metric;
  ieee1905_get_ap_metric *ap_metric;
  ieee1905_get_assoc_sta_metric *assoc_sta_metric;
  ieee1905_get_unassoc_sta_metric *unassoc_sta_metric;
  ieee1905_create_bss_on_ifr *create_bss_on_ifr;
  ieee1905_get_interface_info *get_interface_info;
  ieee1905_backhaul_steer_request *backhaul_steer_req;
  ieee1905_beacon_metrics_query *beacon_metrics_query;
  ieee1905_configure_policy *configure_policy;
  ieee1905_process_vendor_specific_msg *process_vendor_specific_msg;
  ieee1905_asoc_sta_metric_resp *assoc_sta_metric_resp;
  ieee1905_unasoc_sta_metric_resp *unassoc_sta_metric_resp;
  ieee1905_beacon_metric_resp *beacon_metric_resp;
  ieee1905_ap_configured *ap_configured;
  ieee1905_operating_channel_report *operating_chan_report;
  ieee1905_get_vendor_specific_tlv *get_vendor_specific_tlv;
  ieee1905_steering_btm_report *steering_btm_rpt;
  ieee1905_higher_layer_data *higher_layer_data;
  ieee1905_interface_chan_change *interface_chan_change;
  ieee1905_ap_auto_config_resp	*ap_auto_config_resp;
  ieee1905_ap_auto_config_search_sent	*ap_auto_config_search_sent;
  ieee1905_set_bh_sta_params *set_bh_sta_params;
} ieee1905_call_bks_t;

#endif /* MULTIAP */

/**
 * Initialize the IEEE1905.
 *
 * @param usched_hdl	    handle to the microscheduler
 * @param supServiceFlag	Flag to indicate supported service(I5_CONFIG_FLAG_CONTROLLER (Controller)
 *                        and/or I5_CONFIG_FLAG_AGENT (Agent))
 * @param isRegistrar     1 If it is registrar
 * @param msglevel        Message Level
 * @param config          All Configurations
 *
 * @return		            status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_init(void *usched_hdl, unsigned int supServiceFlag, int isRegistrar,
  ieee1905_msglevel_t *msglevel, ieee1905_config *config);

/**
 * De-Initialize the IEEE1905.
 */
void ieee1905_deinit();

/**
 * Get the AL MAC address
 *
 * @return    AL MAC of the self device
 */
unsigned char *ieee1905_get_al_mac();

/**
 * Get Data model
 *
 * @return		      Pointer to a datamodel of type "i5_dm_network_topology_type"
 */
i5_dm_network_topology_type *ieee1905_get_datamodel();

/**
 * Start IEEE1905 Messaging.
 */
void ieee1905_start();

#ifdef MULTIAP
/**
 * Register Callbacks.
 *
 * @param cbs	    Structure containing callbacks
 *
 */
void ieee1905_register_callbacks(ieee1905_call_bks_t *cbs);

/**
 * Add the BSS
 *
 * @param ifr_mac   MAC Address of the radio
 * @param bssid     BSSID of the BSS
 * @param ssid      SSID of the BSS
 * @param ssid_len  Length of the SSID
 * @param chanspec  Channel Specification
 * @param ifname    Interface Name
 * @param mapFlags  MAP Flags of type IEEE1905_MAP_FLAG_XXX
 *
 * @return		      status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_add_bss(unsigned char *radio_mac, unsigned char *bssid, unsigned char *ssid,
  unsigned char ssid_len, unsigned short chanspec, char *ifname, unsigned char mapFlags);

/**
 * Add the BSS to Controller table. only required in controller
 *
 * @param bss   BSS to be added
 *
 * @return      status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_add_bssto_controller_table(ieee1905_client_bssinfo_type *bss);

/**
 * STA has Associated or Disassociated on this device
 *
 * @param bssid         BSSID of the BSS on which STA  associated or disassociated
 * @param mac           MAC address of the STA
 * @param isAssoc       1 if STA Associated, 0 if STA Disassociated
 * @param time_elapsed  Seconds since assoc
 * @param notify        Send Topology Notification
 * @param assoc_frame   Frame body of the (Re)Association request frame
 * @param assoc_frame_len Length of the (Re)Association Request frame length
 *
 * @return		      status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_sta_assoc_disassoc(unsigned char *bssid, unsigned char *mac, int isAssoc,
  unsigned short time_elapsed, unsigned char notify, unsigned char *assoc_frame,
  unsigned int assoc_frame_len);

/**
 * Send the BTM report to the controller
 *
 * @param btm_report  BTM Report to be sent to controller
 *
 * @return            status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_btm_report(ieee1905_btm_report *btm_report);

/**
 * Send steering completed message to the controller
 *
 * @param steer_req     steer_req structure which we passed while steering
 *
 * @return              status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_steering_completed_message(ieee1905_steer_req *steer_req);

/**
 * Send the Client assocaition control messages to all the BSS except the source and target
 *
 * @param assoc_cntrl  Client association control information
 *
 * @return            status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_client_association_control(ieee1905_client_assoc_cntrl_info *assoc_cntrl);

/**
 * Check if the STA is in BTM Steering Disallowed STA list
 *
 * @param mac   STA MAC
 *
 * @return      status of the call. 1 STA exists, 0 Does not exists
 */
int ieee1905_is_sta_in_BTM_steering_disallowed_list(unsigned char *mac);

/**
 * Check if the STA is in Local Steering Disallowed STA List
 *
 * @param mac   STA MAC
 *
 * @return      status of the call. 1 STA exists, 0 Does not exists
 */
int ieee1905_is_sta_in_local_steering_disallowed_list(unsigned char *mac);

/**
 * Send the Backhaul steering response to the controller
 *
 * @param bh_steer_resp  Backhaul steering response to be sent to controller
 *
 * @return            status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_bh_steering_repsonse(ieee1905_backhaul_steer_msg *bh_steer_resp);

/**
 * Send the associated STA link metrics to the requested neighbor
 *
 * @param neighbor_al_mac   AL MAC of the neighbor
 * @param sta_mac           MAC address of the STA for which this report belongs
 *
 * @return        status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_assoc_sta_link_metric(unsigned char *neighbor_al_mac, unsigned char *sta_mac);

/**
 * Send the Vendor Specific Message to the requested neighbor from Application
 *
 * @param msg_data  Vendor Specific Message data
 *
 * @return        status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_vendor_specific_msg(ieee1905_vendor_data *msg_data);

/**
 * Send the Un associated STA link metrics to the requested neighbor
 *
 * @param metric  Metric to be sent
 *
 * @return        status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_unassoc_sta_link_metric(ieee1905_unassoc_sta_link_metric *metrics);

/**
 * Send the beacon report
 *
 * @param report  Beacon Report to be sent
 *
 * @return        status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_beacon_report(ieee1905_beacon_report *report);

/**
 * Send Associated STA Link Metrics Query message
 *
 * @param neighbor_al_mac   AL MAC of the nieghbor
 * @param sta_mac           MAC address of the STA for which this report belongs
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_assoc_sta_link_metric_query(unsigned char *neighbor_al_mac,
  unsigned char *sta_mac);

/**
 * Send UnAssociated STA Link Metrics Query message
 *
 * @param neighbor_al_mac   AL MAC of the nieghbor
 * @param query             Unassociated STA link metrics query
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_unassoc_sta_link_metric_query(unsigned char *neighbor_al_mac,
  ieee1905_unassoc_sta_link_metric_query *query);

/**
 * Send Beacon Metrics Query message
 *
 * @param neighbor_al_mac   AL MAC of the nieghbor
 * @param query             Beacon metrics query details
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_beacon_metrics_query(unsigned char *neighbor_al_mac,
  ieee1905_beacon_request *query);

/**
 * Add the Metric Reporting Policy for a radio. First add all the metric reporting policy details
 * and call the "ieee1905_send_policy_config" function to send it to a particular neighbor.
 * This function can be called only from controller.
 *
 * @param ap_rpt_intvl    AP Metrics Reporting Interval in seconds
 * @param metricrpt       Metric report policies for a given radio
 *
 * @return                status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_add_metric_reporting_policy_for_radio(unsigned char ap_rpt_intvl,
  ieee1905_ifr_metricrpt *metricrpt);

/**
 * Send channel preference query to an agent.
 *
 * @param neighbor_al_mac_address  al mac of the agent
 *
 */
void ieee1905_send_channel_preference_query( unsigned char *neighbor_al_mac_address);

/**
 * Send ieee1905 message. It adds end of tlv and free the message after sending
 * it
 *
 * @param pmsg			Message to be sent.
 *
 */
void ieee1905_send_message(void *pmsg);

/**
 * Insert channel selection TLV to the given ieee1905 messgae
 *
 * @param pmsg			Message to be sent.
 * @param radio_mac             Interface Mac address for preparing TLV
 * @param chan_pref             Channel Preference Array for preparing TLV
 *
 */
void ieee1905_insert_channel_selection_request_tlv(void *pmsg,
  unsigned char *radio_mac, ieee1905_chan_pref_rc_map_array *chan_pref);

/**
 * Create channel selection request message. Creates only the header without
 * any TLVs and user can add TLVs by calling the insert TLV function.
 *
 * @param neighbor_al_mac_address  al mac of the agent
 *
 * @return                  Returns the created ieee1905 message.
 */
void *ieee1905_create_channel_selection_request( unsigned char *neighbor_al_mac_address);

/**
 * Send Policy Configuration to neighbor
 *
 * @param neighbor_al_mac   AL MAC of the nieghbor
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_policy_config(unsigned char *neighbor_al_mac);

/**
 * Send AP Auto Configuration Renew
 *
 * @param neighbor_al_mac_address  al mac of the agent
 *
 */
void ieee1905_send_ap_autoconfig_renew(unsigned char *neighbor_al_mac_address);

/**
 * Send operating channel report
 *
 * @param report  operating channel report structure be sent
 *
 * @return        status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_operating_chan_report(ieee1905_operating_chan_report *report);

/**
 * Send channel preference report
 *
 * @return        status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_chan_preference_report();

/**
 * To inform ieee1905 about association of bSTA to the backhaul AP
 *
 * @param InterfaceId   MAC address of the radio
 *
 * @return              status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_bSTA_associated_to_backhaul_ap(unsigned char *InterfaceId);

/**
 * Send neighbor link metrics query
 *
 * @param neighbor_al_mac         AL MAC of the nieghbor
 * @param specify_neighbor        1 if neighbor is specified. 0 neighbor not specified
 * @param neighbor_of_recv_device If specify_neighbor is 1 then, it is 1905.1 AL MAC address of a
 *                                neighbor of the receiving device
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_neighbor_link_metric_query(unsigned char *neighbor_al_mac,
  unsigned char specify_neighbor, unsigned char *neighbor_of_recv_device);

/**
 * Send AP Metrics Query
 *
 * @param neighbor_al_mac   AL MAC of the nieghbor
 * @param bssids            BSSIDs stored in a linear array. 6 octets for each BSSID
 * @param bssid_count       Number of BSSIDs in the bssids list
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_ap_metrics_query(unsigned char *neighbor_al_mac,
  unsigned char *bssids, unsigned char bssid_count);

/**
 * Add STA MAC addresses to Local Steering Disallowed List
 *
 * @param bssids            STA MAC addresses for which local steering is disallowed.
 *                          MAC addresses stored in a linear array. 6 octets for each MAC
 * @param bssid_count       Number of STA MAC addresses for which local steering is disallowed
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_add_mac_to_local_steering_disallowed_list(unsigned char *macs,
  unsigned char mac_count);

/**
 * Get supported bands from Radio capabilities
 *
 * @param RadioCaps         Radio capabilities received from agent
 *
 * @return                  bands supported
 */
int ieee1905_get_band_from_radiocaps(ieee1905_radio_caps_type *RadioCaps);

/**
 * Notify about channel change so that media specific info can be updated and
 * Topology notification can be send to other nodes
 *
 * @param pdmif		    Interface data structure
 *
 */
void ieee1905_notify_channel_change(i5_dm_interface_type *pdmif);

/**
 * Send Unsolicitated AP Metrics Response
 *
 * @param neighbor_al_mac   AL MAC of the nieghbor
 * @param ifr_mac           MAC address of the radio
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_ap_metrics_response(unsigned char *neighbor_al_mac, unsigned char *ifr_mac);

/**
 * Send Backhaul Steering Request
 *
 * @param neighbor_al_mac   AL MAC of the nieghbor
 * @param bh_steer_req      Backhaul Steer Request Parameter
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_backhaul_steering_request(unsigned char *neighbor_al_mac,
  ieee1905_backhaul_steer_msg *bh_steer_req);

/**
 * add vendor message tlv
 *
 * @param pmsg		Message to be sent.
 * @param vndr_msg	vendor message to add in pmsg
 *
 * @return      void
 */
void ieee1905_insert_vendor_message_tlv(void *pmsg, ieee1905_vendor_data *vndr_msg);

#endif /* MULTIAP */

/*#ifdef __cplusplus
}
#endif*/ /* __cplusplus */

#endif /* __IEEE1905_H__ */
