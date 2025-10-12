/*
 * Broadcom IEEE1905 library include file
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
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
 * $Id: ieee1905.h 836688 2024-02-19 08:03:37Z $
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
#define I5_INIT_FLAG_DPP_ENABLED        0x0001  /* DPP enabled or not */
#define I5_INIT_FLAG_TS_SUPPORTED       0x0002  /* Guest Network is enabled */
#define I5_INIT_FLAG_TS_ACTIVE          0x0004  /* Traffic Separation Policy is present */
#define I5_INIT_FLAG_TS_USING_IPTABLE   0x0008  /* Traffic separation using iptables */

#define I5_INIT_FLAG_DONT_UPDATE_MEDIA_INFO    0x00000010  /* Dont update medio info
							    * in Topology response
							    */
#define I5_INIT_FLAG_LOG_IN_SYSLOGD_INFO    0x00000020  /* IEEE1905 logs in syslogd enabled or not */

/* Self Flags in i5_config. These flags only for the self agents/controller */
#define I5_SELF_FLAG_TS_POLICY_IN_M2_OR_ASSOC    0x00000001   /* Traffic separation policy received
                                                               * in M2 or (Re)Association Response
                                                               */
#define I5_CFG_SELF_FLAG_SIGNAL    0x00000002   /* Signal handler is issued */

#define I5_IS_TS_POLICY_RECVD_IN_M2_OR_ASSOC(flags) ((flags) & \
                                                      I5_SELF_FLAG_TS_POLICY_IN_M2_OR_ASSOC)
#define I5_IS_SELF_FLAG_SIGNAL(flags) ((flags) & I5_CFG_SELF_FLAG_SIGNAL)

/* i5_config_type Flags, can be directly set/get through NVRAM */
#define I5_NV_FLAG_QM_POL_WITH_RESERVED        I5_BIT(0)  /* Add Last 20 Resevred Octets
                                                     * in QoS Management Policy TLV (0xDB)
                                                     */
#define I5_IS_QM_POL_WITH_RESERVED(nv_flags) ((nv_flags) & I5_NV_FLAG_QM_POL_WITH_RESERVED)

/* Device Flags */
#define I5_CONFIG_FLAG_CONTROLLER       0x00000001  /* Supports Contoller */
#define I5_CONFIG_FLAG_AGENT            0x00000002  /* Supports Agent */
#define I5_CONFIG_FLAG_REGISTRAR        0x00000004  /* Supports Registrar */
#define I5_CONFIG_FLAG_START_MESSAGE    0x00000008  /* Start Messaging. Only for internal usage */
#define I5_CONFIG_FLAG_CONTROLLER_FOUND 0x00000010  /* Controller found after AP auto config
                                                     * search
                                                     */
#define I5_CONFIG_FLAG_DWDS             0x00000020  /* Its DWDS. WiFi onboarded */
#define I5_CONFIG_FLAG_CTRLAGENT        0x00000040  /* Agent running on Controller */
#define I5_CONFIG_FLAG_DPP_ENABLED      0x00000080  /* DPP enabled or not */
#define I5_CONFIG_FLAG_DEDICATED_BK     0x00000100  /* Dedicated backhaul */
#define I5_CONFIG_FLAG_HAS_BH_BSS       0x00000200  /* Agent has backhaul BSS */
#define I5_CONFIG_FLAG_TS_SUPPORTED     0x00000400  /* Traffic Separation is Supported or not */
#define I5_CONFIG_FLAG_TS_ACTIVE        0x00000800  /* Traffic Separation Policy is present */
#define I5_CONFIG_FLAG_TS_USING_IPTABLE 0x00001000  /* Traffic separation using iptables */
#define I5_CONFIG_FLAG_TS_ADD_FH_RULE   0x00002000  /* Add ebtable rule on fronthaul to block VLAN
                                                     * tagged packets from stations if the traffic
                                                     * separation is active
                                                     */
#define I5_CONFIG_FLAG_QUERY_AP_CAPS    0x00004000  /* AP capability needs to be queried from this
                                                     * device
                                                     */
#define I5_CONFIG_FLAG_ACCEPT_RENEW     0x00008000  /* Accept renew process from contoller */
#define I5_CONFIG_FLAG_VNDR_BROADCOM    0x00010000  /* Vendor is broadcom */
#define I5_CONFIG_FLAG_MAP_CERT         0x00020000  /* MAP certification enabled or not */
#define I5_CONFIG_FLAG_SPLIT_VNDR_MSG	0x00040000  /* Split vendor message supported at agent */
#define I5_CONFIG_FLAG_R1_CERT_COMPATIBLE 0x00080000  /* For profile 1 certification some of the
                                                       * testbed devices drop messages which has
                                                       * R2 TLVs. So, just to pass those
                                                       * certification, using this flag and based
                                                       * on this dropping some of the TLVs
                                                       */
#define I5_CONFIG_FLAG_R2_CERT_COMPATIBLE 0x00100000  /* To pass R2 certification, some hacks are
                                                       * required, because of some testbed issues.
                                                       * So, using this flag for those hacks
                                                       */

#define I5_CONFIG_FLAG_CTRL_CHAN_SELECT	0x00200000  /* Channel selection with control channel */

#define I5_CONFIG_FLAG_TS_ENABLE_ETH_VTAG 0x00400000  /* Traffic separation using iptables,
                                                       * enable vlan tag on primary
                                                       * LAN ethernet port
                                                       */

#define I5_CONFIG_FLAG_DONT_UPDATE_MEDIA_INFO    0x00800000  /* Dont update medio info
							      * in Topology response
							      */

#define I5_CONFIG_FLAG_LOG_IN_SYSLOGD_INFO    0x01000000  /* IEEE1905 logs in syslogd info */
#define I5_CONFIG_FLAG_CTR_CHAN_MEDIA_INFO    0x02000000  /* IEEE1905 Send Center Channel in Media Info */
#define I5_CONFIG_FLAG_MAP_ONBOARDED	0x04000000  /* NVRAM value for map_onboarded */
#define I5_CONFIG_FLAG_RESTART_REQUIRED	0x08000000  /* Indicates if nvram restart is required or not */
#define I5_CONFIG_FLAG_ACCEPT_RECONFIG_TRIGGER  0x10000000u /* Accept Reconfiguration Trigger
                                                             * message from contoller. Also, this
                                                             * means that, it has to send BSS
                                                             * confiuration request not WSC M1
                                                             */
#define I5_CONFIG_FLAG_TEARDOWN_AGENT  0x20000000u /* Accept M2 Tear Down Trigger message from Controller.
						    * To indiciate Controller reached max repeater onboard
						    * count to block other agents to onboard
						    */
#define I5_CONFIG_FLAG_EARLY_AP_CAP_PREF 0x40000000u /* Controller prefers to receive Early AP
						      *	capability report message or not.
						      *	(i5_config flag, Agent only)
						      */

#define I5_IS_MULTIAP_CONTROLLER(flags) ((flags) & I5_CONFIG_FLAG_CONTROLLER)
#define I5_IS_MULTIAP_AGENT(flags)      ((flags) & I5_CONFIG_FLAG_AGENT)
#define I5_IS_REGISTRAR(flags)          ((flags) & I5_CONFIG_FLAG_REGISTRAR)
#define I5_IS_START_MESSAGE(flags)      ((flags) & I5_CONFIG_FLAG_START_MESSAGE)
#define I5_IS_CONTROLLER_FOUND(flags)   ((flags) & I5_CONFIG_FLAG_CONTROLLER_FOUND)
#define I5_IS_DWDS(flags)               ((flags) & I5_CONFIG_FLAG_DWDS)
#define I5_IS_CTRLAGENT(flags)          ((flags) & I5_CONFIG_FLAG_CTRLAGENT)
#define I5_IS_DPP_ENABLED(flags)        ((flags) & I5_CONFIG_FLAG_DPP_ENABLED)
#define I5_IS_DEDICATED_BK(flags)       ((flags) & I5_CONFIG_FLAG_DEDICATED_BK)
#define I5_HAS_BH_BSS(flags)            ((flags) & I5_CONFIG_FLAG_HAS_BH_BSS)
#define I5_IS_TS_SUPPORTED(flags)       ((flags) & I5_CONFIG_FLAG_TS_SUPPORTED)
#define I5_IS_TS_ACTIVE(flags)          ((flags) & I5_CONFIG_FLAG_TS_ACTIVE)
#define I5_IS_TS_USING_IPTABLE(flags)   ((flags) & I5_CONFIG_FLAG_TS_USING_IPTABLE)
#define I5_IS_TS_ADD_FH_RULE(flags)     ((flags) & I5_CONFIG_FLAG_TS_ADD_FH_RULE)
#define I5_IS_TS_ENABLE_ETH_VTAG(flags) ((flags) & I5_CONFIG_FLAG_TS_ENABLE_ETH_VTAG)
#define I5_IS_QUERY_AP_CAP(flags)       ((flags) & I5_CONFIG_FLAG_QUERY_AP_CAPS)
#define I5_IS_ACCEPT_RENEW(flags)       ((flags) & I5_CONFIG_FLAG_ACCEPT_RENEW)
#define I5_IS_VENDOR_BROADCOM(flags)    ((flags) & I5_CONFIG_FLAG_VNDR_BROADCOM)
#define I5_IS_MAP_CERT(flags)           ((flags) & I5_CONFIG_FLAG_MAP_CERT)
#define I5_IS_SPLIT_VNDR_MSG(flags)	((flags) & I5_CONFIG_FLAG_SPLIT_VNDR_MSG)
#define I5_IS_R1_CERT_COMPATIBLE(flags) ((flags) & I5_CONFIG_FLAG_R1_CERT_COMPATIBLE)
#define I5_IS_R2_CERT_COMPATIBLE(flags) ((flags) & I5_CONFIG_FLAG_R2_CERT_COMPATIBLE)
#define I5_IS_CTRL_CHAN_SELECT(flags)	((flags) & I5_CONFIG_FLAG_CTRL_CHAN_SELECT)
#define I5_DONT_ADD_MEDIA_INFO(flags)   ((flags) & I5_CONFIG_FLAG_DONT_UPDATE_MEDIA_INFO)
#define I5_IS_LOG_IN_SYSLOGD_INFO(flags)((flags) & I5_CONFIG_FLAG_LOG_IN_SYSLOGD_INFO)
#define I5_IS_CTR_CHAN_MEDIA_INFO(flags)((flags) & I5_CONFIG_FLAG_CTR_CHAN_MEDIA_INFO)
#define I5_IS_MAP_ONBOARDED(flags)	((flags) & I5_CONFIG_FLAG_MAP_ONBOARDED)
#define I5_IS_RESTART_REQUIRED(flags)	((flags) & I5_CONFIG_FLAG_RESTART_REQUIRED)
#define I5_IS_ACCEPT_RECONFIG_TRIGGER(flags)  ((flags) & I5_CONFIG_FLAG_ACCEPT_RECONFIG_TRIGGER)
#define I5_IS_MAP_AGENT_TEARDOWN(flags) ((flags) & I5_CONFIG_FLAG_TEARDOWN_AGENT)
#define I5_IS_EARLY_AP_CAP_PREF(flags)	((flags) & I5_CONFIG_FLAG_EARLY_AP_CAP_PREF)

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
/* HE TX Streams is represented by bits 7, 6 and 5 */
#define IEEE1905_AP_HECAP_TX_NSS_1		0x00000000	/* Bit 7 = 0, Bit 6 = 0, Bit 5 = 0 (00000000) */
#define IEEE1905_AP_HECAP_TX_NSS_2		0x00000020	/* Bit 7 = 0, Bit 6 = 0, Bit 5 = 1 (00100000) */
#define IEEE1905_AP_HECAP_TX_NSS_3		0x00000040	/* Bit 7 = 0, Bit 6 = 1, Bit 5 = 0 (01000000) */
#define IEEE1905_AP_HECAP_TX_NSS_4		0x00000060	/* Bit 7 = 0, Bit 6 = 1, Bit 5 = 1 (01100000) */
#define IEEE1905_AP_HECAP_TX_NSS_8		0x000000E0	/* Bit 7 = 1, Bit 6 = 1, Bit 5 = 1 (11100000) */

/* WiFi6 cap Roleflag  */
#define IEEE1905_AP_WiFi6CAP_MCSLENGTH		0x0f	/* Bit 0-3 (00001111) */
#define IEEE1905_AP_WiFi6CAP_80P80MHZ		0x10	/* Bit 4 (00010000) */
#define IEEE1905_AP_WiFi6CAP_160MHZ		0x20	/* Bit 5 (00100000) */
#define IEEE1905_AP_WiFi6CAP_BSTA_AGENTROLE	0x80	/* Bit 7 = 1, Bit 6 = 0  (10000000) */
#define IEEE1905_AP_WiFi6CAP_AP_AGENTROLE	0x00	/* Bit 7 = 0, Bit 6 = 0  (00000000) */

/* WiFi6 cap HE support flag  */
#define IEEE1905_AP_WiFi6CAP_DL_OFDMA		0x01	/* Bit 0 (00000001) */
#define IEEE1905_AP_WiFi6CAP_UL_OFDMA		0x02	/* Bit 1 (00000010) */
#define IEEE1905_AP_WiFi6CAP_UL_MUMIMO		0x04	/* Bit 2 (00000100) */
#define IEEE1905_AP_WiFi6CAP_BEAMFME_GT_80	0x08	/* Bit 3 (00001000) */
#define IEEE1905_AP_WiFi6CAP_BEAMFME_LT_80	0x10	/* Bit 4 (00010000) */
#define IEEE1905_AP_WiFi6CAP_MU_BEAMFMR_STS	0x20	/* Bit 5 (00100000) */
#define IEEE1905_AP_WiFi6CAP_SU_BEAMFME		0x40	/* Bit 6 (01000000) */
#define IEEE1905_AP_WiFi6CAP_SU_BEAMFMR		0x80	/* Bit 7 (10000000) */

/* WiFi6 cap General support flag  */
#define IEEE1905_AP_WiFi6CAP_ACU		0x01	/* Bit 0 (00000001) */
#define IEEE1905_AP_WiFi6CAP_SPATIAL_REUSE	0x02	/* Bit 1 (00000010) */
#define IEEE1905_AP_WiFi6CAP_TWT_RESPONDER	0x04	/* Bit 2 (00000100) */
#define IEEE1905_AP_WiFi6CAP_TWT_REQUESTER	0x08	/* Bit 3 (00001000) */
#define IEEE1905_AP_WiFi6CAP_MU_EDCA		0x10	/* Bit 4 (00010000) */
#define IEEE1905_AP_WiFi6CAP_MU_BSSID	        0x20	/* Bit 5 (00100000) */
#define IEEE1905_AP_WiFi6CAP_MU_RTS		0x40	/* Bit 6 (01000000) */
#define IEEE1905_AP_WiFi6CAP_RTS		0x80	/* Bit 7 (10000000) */

/* EHT Capabilities flags */
#define IEEE1905_AP_EHTCAP_320MHZ		0x01u	/* Bit 0 (00000001) */
#define IEEE1905_AP_EHTCAP_160MHZ		0x02u	/* Bit 1 (00000010) */

/* Flags for Including bit for the Estimated Service Parameters Information field */
#define IEEE1905_INCL_BIT_ESP_BE  0x80  /* AC = BE */
#define IEEE1905_INCL_BIT_ESP_BK  0x40  /* AC = BK */
#define IEEE1905_INCL_BIT_ESP_VO  0x20  /* AC = VO */
#define IEEE1905_INCL_BIT_ESP_VI  0x10  /* AC = VI */

/* Flags for MultiAp extension subelement  */
#define IEEE1905_PROFILE2_BHSTA_DIS	0x04	/* Bit 2 */
#define IEEE1905_PROFILE1_BHSTA_DIS	0x08	/* Bit 3 */
#define IEEE1905_TEAR_DOWN	0x10	/* Bit 4 */
#define IEEE1905_FRONTHAUL_BSS	0x20	/* Bit 5 */
#define IEEE1905_BACKHAUL_BSS	0x40	/* Bit 6 */
#define IEEE1905_BACKHAUL_STA	0x80	/* Bit 7 */

/* Wi-Fi 7 Agent capability - Max number of Affiliated AP/bSTAs */
#define IEEE1905_WiFi7_BSTA_MAX_LINKS_MASK	0x0Fu
#define IEEE1905_WiFi7_BSTA_MAX_LINKS_SHIFT	0
#define IEEE1905_WiFi7_AP_MAX_LINKS_MASK	0xF0u
#define IEEE1905_WiFi7_AP_MAX_LINKS_SHIFT	4

/* Wi-Fi 7 Agent capability - TID to Link Mapping */
#define IEEE1905_WiFi7_TIDMAP_NOTSUPP		0x00u  /* Bit 7 = 0, Bit 6 = 0 (00000000) */
#define IEEE1905_WiFi7_TIDMAP_TIDTOANYLINK	0x40u  /* Bit 7 = 0, Bit 6 = 1 (01000000) */
#define IEEE1905_WiFi7_TIDMAP_TIDTOSAMELINK	0x80u  /* Bit 7 = 1, Bit 6 = 0 (10000000) */

/* Wi-Fi 7 Radio specific capability */
#define IEEE1905_AP_ALLOW_NONMLD_STA_ASSOC	0x80u  /* Bit 7 (10000000) */

/* Wi-Fi 7 Agent capability - AP Radio modes supported */
#define IEEE1905_WiFi7_AP_STR_SUPPORT		0x80u  /* Bit 7 (10000000) */
#define IEEE1905_WiFi7_AP_NSTR_SUPPORT		0x40u  /* Bit 6 (01000000) */
#define IEEE1905_WiFi7_AP_EMLSR_SUPPORT		0x20u  /* Bit 5 (00100000) */
#define IEEE1905_WiFi7_AP_EMLMR_SUPPORT		0x10u  /* Bit 4 (00010000) */

/* Wi-Fi 7 Agent capability - bSTA Radio modes supported */
#define IEEE1905_WiFi7_BSTA_STR_SUPPORT		0x80u  /* Bit 7 (10000000) */
#define IEEE1905_WiFi7_BSTA_NSTR_SUPPORT	0x40u  /* Bit 6 (01000000) */
#define IEEE1905_WiFi7_BSTA_EMLSR_SUPPORT	0x20u  /* Bit 5 (00100000) */
#define IEEE1905_WiFi7_BSTA_EMLMR_SUPPORT	0x10u  /* Bit 4 (00010000) */

/* Wi-Fi 7 Agent capability - Frequency separation */
#define IEEE1905_WiFi7_FREQ_SEP_MASK		0xF8u
#define IEEE1905_WiFi7_FREQ_SEP_SHIFT		3

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

/* cac message from controller */
enum {
  MAP_CAC_RQST = 0,
  MAP_CAC_TERMINATE = 1,
};

/* CAC method in cac reqsuet message from controller */
#define MAP_CAC_METHOD_CONTINOUS_CAC			0x00	/* Bit 7 = 0, Bit 6 = 0, Bit 5 = 0 */
#define MAP_CAC_METHOD_CONTINOUS_WITH_DEDICATED_RADIO	0x01	/* Bit 7 = 0, Bit 6 = 0, Bit 5 = 1 */
#define MAP_CAC_METHOD_MIMO_DIMENSION_REDUCED_CAC	0x02	/* Bit 7 = 0, Bit 6 = 1, Bit 5 = 0 */
#define MAP_CAC_METHOD_TIME_SLICED_CAC			0x03	/* Bit 7 = 0, Bit 6 = 1, Bit 5 = 1 */

#define MAP_IS_CAC_METHOD_ENABLED(flags, method)	((flags) & (1 << (method)))
#define MAP_ENABLE_CAC_METHOD(flags, method)		(flags |= (1 << (method)))

/* CAC action after serving cac request from controller */
#define MAP_CAC_ACTION_STAY_ON_NEW_CHANNEL		0x00	/* Bit 4 = 0, Bit 3 = 0 */
#define MAP_CAC_ACTION_RETURN_TO_PREV_CHANNEL		0x01	/* Bit 4 = 0, Bit 3 = 1 */

#define MAP_CAC_METHOD_MASK				0XE0 /* 5 - 7 bits */
#define MAP_CAC_ACTION_MASK				0X18 /* 3 - 4 bits */

#define GET_MAP_CAC_METHOD(flags)		(((flags & MAP_CAC_METHOD_MASK) >> 5))
#define GET_MAP_CAC_COMPLETION_ACTION(flags)	(((flags & MAP_CAC_ACTION_MASK) >> 3))
#define SET_MAP_CAC_METHOD(flags, method)		((flags) |= ((method) << 5))
#define SET_MAP_CAC_COMPLETION_ACTION(flags, action)	((flags) |= ((action) << 3))

/* Encryption Types */
#define IEEE1905_ENCR_NONE  0x0001
#define IEEE1905_ENCR_WEP   0x0002
#define IEEE1905_ENCR_TKIP  0x0004
#define IEEE1905_ENCR_AES   0x0008
#define IEEE1905_ENCR_GCMP256   0x0010

/* Authentication Types */
#define IEEE1905_AUTH_OPEN    0x0001
#define IEEE1905_AUTH_WPAPSK  0x0002
#define IEEE1905_AUTH_SHARED  0x0004
#define IEEE1905_AUTH_WPA     0x0008
#define IEEE1905_AUTH_WPA2    0x0010
#define IEEE1905_AUTH_WPA2PSK 0x0020
#define IEEE1905_AUTH_SAE     0x0040
#define IEEE1905_AUTH_DPP     0x0080  /* Do not use this in M1/M2 */
#define IEEE1905_AUTH_SAE_EXT 0x0100  /* SAE with AKM24 */

/* Password required for these auth types */
#define IEEE1905_AUTH_PASS_REQ  (IEEE1905_AUTH_WPA2PSK | IEEE1905_AUTH_SAE | IEEE1905_AUTH_SAE_EXT)

#define IEEE1905_MAX_KEY_LEN    127

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
#define IEEE1905_AP_CAPS_FLAGS_UNASSOC_RPT            0x80u
/* Support Unassociated STA Link Metrics reporting on channels its BSSs are not
 * currently operating on.
 * 0: Not supported
 * 1 : Supported
 */
#define IEEE1905_AP_CAPS_FLAGS_UNASSOC_RPT_NON_CH     0x40u
/* Support Agent-initiated RSSI-based Steering
 * 0: Not supported
 * 1 : Supported
 */
#define IEEE1905_AP_CAPS_FLAGS_AGENT_INIT_RSSI_STEER  0x20u

/* Support backhaul STA Reconfiguration with 1905 AP-Autoconfiguration WSC (M8).
 * 0: Not supported
 * 1 : Supported
 */
#define IEEE1905_AP_CAPS_FLAGS_M8_BSTA_RECONFIG       0x10u

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
#define IEEE1905_MAP_FLAG_STA       0x04  /* bSTA */
#define IEEE1905_MAP_FLAG_GUEST     0x08  /* Guest BSS */
#define IEEE1905_MAP_FLAG_ROOTP_ONLY            0x10  /* RootAP only BSS */
#define IEEE1905_MAP_FLAG_PROF1_DISALLOWED      0x20
#define IEEE1905_MAP_FLAG_PROF2_DISALLOWED      0x40
#define IEEE1905_MAP_FLAG_BLOCK_STA_ON_BH       0x80  /* block normal STA on BH BSS */

#define I5_IS_BSS_FRONTHAUL(flags)	((flags) & IEEE1905_MAP_FLAG_FRONTHAUL)
#define I5_IS_BSS_BACKHAUL(flags)	((flags) & IEEE1905_MAP_FLAG_BACKHAUL)
#define I5_IS_BSS_STA(flags)		((flags) & IEEE1905_MAP_FLAG_STA)
#define I5_IS_BSS_GUEST(flags)		((flags) & IEEE1905_MAP_FLAG_GUEST)
#define I5_IS_BSS_ROOTAP_ONLY(flags)		((flags) & IEEE1905_MAP_FLAG_ROOTP_ONLY)
#define I5_IS_BSS_PROF1_DISALLOWED(flags)	((flags) & IEEE1905_MAP_FLAG_PROF1_DISALLOWED)
#define I5_IS_BSS_PROF2_DISALLOWED(flags)	((flags) & IEEE1905_MAP_FLAG_PROF2_DISALLOWED)
#define I5_IS_BSS_BLOCK_STA_ON_BH(flags)	((flags) & IEEE1905_MAP_FLAG_BLOCK_STA_ON_BH)

/* Device profile */
#define I5_IS_DEVICE_PROFILE1(profile)     ((profile) == ieee1905_map_profile0)
#define I5_IS_DEVICE_PROFILE2(profile)     ((profile) == ieee1905_map_profile2)

#define I5_IS_BSTA_DISALLOWED(profile, mapFlags) ((I5_IS_DEVICE_PROFILE1(profile) \
							&& I5_IS_BSS_PROF1_DISALLOWED(mapFlags)) \
							|| (I5_IS_DEVICE_PROFILE2(profile) \
							&& I5_IS_BSS_PROF2_DISALLOWED(mapFlags)))

/* MAP policy Type flags */
#define MAP_POLICY_TYPE_FLAG_STEER          I5_BIT(0)  /* Steering Policy */
#define MAP_POLICY_TYPE_FLAG_METRIC_REPORT  I5_BIT(1)  /* Metrics Reporting Policy */
#define MAP_POLICY_TYPE_FLAG_TS_8021QSET    I5_BIT(2)  /* Default 802.1Q Settings Policy */
#define MAP_POLICY_TYPE_FLAG_TS_POLICY      I5_BIT(3)  /* Traffic Separation Policy */
#define MAP_POLICY_TYPE_FLAG_CHSCAN_REPORT  I5_BIT(4)  /* Channel Scan Reporting Policy */
#define MAP_POLICY_TYPE_FLAG_UNSUCCESSFUL_ASSOCIATION  I5_BIT(5)  /* Recieved Unsuccessful Association Policy */
#define MAP_POLICY_TYPE_FLAG_BACKHAUL_BSS   I5_BIT(6)  /* Backhaul BSS Config Policy Received */
#define MAP_POLICY_TYPE_FLAG_QOSMGMT_POLICY I5_BIT(7)  /* QoS Management Policy */

/* MBO BTM Transition Reason Code used for steering from Table 18 of MBO spec*/
#define MBO_BTM_REASONCODE_FLAG_UNSPECIFIED	0x0	/* Unspecified */
#define MBO_BTM_REASONCODE_FLAG_FRAME_LOSS	0x1	/* Excessive frame loss rate */
#define MBO_BTM_REASONCODE_FLAG_TRAFFIC_DELAY	0x2	/* Excessive delay current traffic stream */
#define MBO_BTM_REASONCODE_FLAG_LOW_BANDWIDTH	0x3	/* Insufficient Bandwidth for current traffic stream */
#define MBO_BTM_REASONCODE_FLAG_LOAD_BALANCE	0x4	/* Load Balancing */
#define MBO_BTM_REASONCODE_FLAG_LOW_RSSI	0x5	/* Low RSSI */
#define MBO_BTM_REASONCODE_FLAG_RETRANSMISSION	0x6	/* Received excessive number of retransmissions */
#define MBO_BTM_REASONCODE_FLAG_INTERFERENCE	0x7	/* High Interferene */
#define MBO_BTM_REASONCODE_FLAG_GRAY_ZONE	0x8	/* Imbalance between PHY Downlink and Uplink */
#define MBO_BTM_REASONCODE_FLAG_PREMIUM_AP	0x9	/* Premium AP */
#define MBO_BTM_REASCONCODE_FLAG_RESERVED	0x10	/* Reserved 10-255 */

typedef enum {
	MAP_CAC_STATUS_SUCCESS = 0,		/* CAC Success */
	MAP_CAC_STATUS_RADAR_DETECTED = 1,	/* Radar detected */
	MAP_CAC_STATUS_RQST_NOT_SUPPORTED = 2,	/* CAC not supported as requested */
	MAP_CAC_STATUS_RADIO_BUSY = 3,		/* Radio too busy to perform CAC */
	MAP_CAC_STATUS_RQST_NONCONFORM = 4,	/* Request is non conformant to regulations */
	MAP_CAC_STATUS_OTHER_ERROR = 5,		/* Other error */
} map_cac_status_t;

/* Forward declaration */
typedef struct ieee1905_call_bks ieee1905_call_bks_t;

/* QoS Management Policy Configuration Flags */
#define I5_QOSMGMT_POL_FLAGS_MSCS_ALL    I5_BIT(0) /* All STAs are Disallowed to do MSCS */
#define I5_QOSMGMT_POL_FLAGS_SCS_ALL     I5_BIT(1) /* All STAs are Disallowed to do SCS */

/* QoS Management Policy Configuration */
typedef struct {
  ieee1905_glist_t mscs_disallowed_sta_list; /* List of ieee1905_sta_list type objects for MSCS */
  ieee1905_glist_t scs_disallowed_sta_list; /* List of ieee1905_sta_list type objects for SCS */
  unsigned int flags;       /* Flags of type I5_QOSMGMT_POL_FLAGS_XXX */
} ieee1905_qosmgmt_policy_t;

/* All the configurations required for IEEE1905 */
typedef struct {
  unsigned char map_profile;  /* MultiAP Profile */
  unsigned char basic_caps; /* AP Basic caps of Type IEEE1905_AP_CAPS_FLAGS_XXX */
  unsigned int flags;       /* Flags of type I5_INIT_FLAG_XXX */
  unsigned short prim_vlan_id;  /* Primary VLAN ID */
  unsigned char default_pcp;  /* Default pcp for this R2 network */
  unsigned short vlan_ether_type; /* VLAN ether type */
  ieee1905_glist_t ts_policy_list;	/* List of ieee1905_ts_policy_t type object */
  ieee1905_call_bks_t *cbs;
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
  unsigned char reason_code;  /* Reason code for steering. Valid in Profile-2 */
} ieee1905_bss_list;

/* Multi-AP Steering request profile */
typedef enum map_steer_req_profile {
  map_steer_req_profile1  = 1,	/* Profile-1 Steering Request */
  map_steer_req_profile2  = 2,	/* Profile-2 Steering Request */
} map_steer_req_profile_t;

typedef struct {
  map_steer_req_profile_t profile;  /* Steering request profile. Profile-1 or Profile-2 */
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
  i5_dm_device_type *pDeviceController; /* Controller device pointer */
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
#define MAP_STA_MTRC_WIFI6_STA_STATUS 0x20  /* Associated WiFi6 STA Status Report Inclusion Policy
                                             * 0: Do not include
                                             * 1: Include
                                             */
#define MAP_MAX_AGENT_COUNT 6u  /* Maximum agents supported by the controller.
				* 5 repeaters plus one agent on the root AP
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

/* Channel Scan Reporting Policy Flags */
#define MAP_CHSCAN_RPT_POL_INDEP_SCAN 0x80  /* Report Independent Channel Scans Flag
	* 0 : Do not report Independent Channel Scans
	* 1 : Report Independent Channel Scans
	*/
/* Channel Scan Reporting Policy TLV Configuration */
typedef struct {
  unsigned char chscan_rpt_policy_flag;  /* Channel Scan Reporting Policy Flags
	* of type MAP_CHSCAN_RPT_POL_XXX_XXX */
} ieee1905_chscanrpt_config;

#define I5_IS_CHSCAN_INDEP_SCAN_ENAB(config)	(((config)->chscan_rpt_policy_flag) & \
                                                  MAP_CHSCAN_RPT_POL_INDEP_SCAN)

/* ssid list */
typedef struct {
  dll_t			node;
  ieee1905_ssid_type	ssid;	/* ssid */
} ieee1905_ssid_list_type;

/* Traffic Separation policy */
typedef struct {
  dll_t			node;
  unsigned short	vlan_id;      /* vlan id corresponding to ssid */
  ieee1905_glist_t	ssid_list;  /* list of ssid's corresponding to vlan id. List of
                                 * ieee1905_ssid_list_type objects
                                 */
} ieee1905_ts_policy_t;

/* Protocol list */
typedef struct {
  dll_t		node;
  uint8		protocol;	/* protocol */
} ieee1905_hld_protocol_list_t;

/* Port list */
typedef struct {
  dll_t		node;
  uint16	port;	/* port */
  ieee1905_glist_t     protocol_list; /* list of ieee1905_hld_protocol_list_t */
} ieee1905_hld_port_list_t;

/* Backhaul BSS configuration policy */
typedef struct {
  dll_t			node;
  unsigned char bssid[IEEE1905_MAC_ADDR_LEN]; /* Backhaul BSSID */
  unsigned char bsta_flag;  /* BSTA disallowed flag Of Type IEEE1905_MAP_FLAG_XXX */
} ieee1905_bh_bss_list;

typedef enum map_p2_err_codes {
  map_p2_err_reserved_0				= 0x00,
  map_p2_err_sp_servprio_rule_not_found		= 0x01,
  map_p2_err_sp_servprio_rules_exceeded_max	= 0x02,
  map_p2_err_pcp_or_vlanid_not_provided		= 0x03,
  map_p2_err_reserved_4				= 0x04,
  map_p2_err_unique_vlanid_max_supported	= 0x05,
  map_p2_err_reserved_6				= 0x06,
  map_p2_err_ts_fh_prof1_not_supported		= 0x07,
  map_p2_err_ts_prof1_prof2_not_supported	= 0x08,
  map_p2_err_reserved_9				= 0x09,
  map_p2_err_ts_not_supported			= 0x0A,
  map_p2_err_sp_cant_config_req_qosmgmt_pol	= 0x0B,
  map_p2_err_sp_qosmgmt_dscp_pol_req_rejected	= 0x0C,
  map_p2_err_sp_cant_onboard_others_via_dpp	= 0x0D
} map_p2_err_codes_t;

/* Profile 2 Error Code */
typedef struct {
  dll_t			node;
  map_p2_err_codes_t    err_code; /* Reason code */
  unsigned char bssid[IEEE1905_MAC_ADDR_LEN]; /* BSSID this error refers to.
                                              * included if Reason code = 0x07 or 0x08
                                              */
  unsigned int service_prio_rule_id; /* Service Prioritization Rule ID.
                                              * included if Reason Code = 0x01 or 0x02
                                              */
  unsigned short qm_id; /* An identifier that uniquely identifies a QoS Management rule
                                            * included if Reason Code = 0x0B
                                            */
} ieee1905_err_code_t;

/*  Service Prioritization Rule Operation Flags */
#define MAP_SERV_PRIO_RULE_ADD I5_BIT(7) /* Add-Remove Rule :
                                    * bit 7 : 0: remove this rule, 1: add this rule
                                    */
#define I5_IS_SERV_PRIO_RULE_ADD(flags) \
	((flags) & MAP_SERV_PRIO_RULE_ADD)

/*  Service Prioritization Rule Flags */
#define MAP_SERV_PRIO_RULE_ALWAYS_MATCH I5_BIT(7) /* Always Match :
                                    * bit 7 : 0 or 1 : Rule Always Matches
                                    */
#define I5_IS_SERV_PRIO_RULE_ALWAYS_MATCH(flags) \
	((flags) & MAP_SERV_PRIO_RULE_ALWAYS_MATCH)

/* Service Prioritization Rule TLV */
typedef struct i5_serv_prio_rule {
  dll_t			node;
  unsigned int rule_id; /* Service Prioritization Rule Identifier */
  unsigned char rule_operation_flag; /* Add-Remove Rule :
                                    * bit 7 : 0: remove this rule, 1: add this rule
                                    */
  unsigned char rule_precedence; /* Rule Precedence :
                                    * higher number means higher priority
                                    * 0x00 - 0xFE : Valid Values , 0xFF: Reserved Value
                                    */
  unsigned char rule_output; /* Rule Output : The value of, or method
                                    * used to select, the 802.1Q C-TAG PCP Output Value
                                    * 0x00 - 0x09 : Valid Values , 0x0A - 0xFF: Reserved Values
                                    */
  unsigned char rule_flag; /* Always Match :
                                    * bit 7 : 0 or 1 : Rule Always Matches
                                    */
} i5_serv_prio_rule_t;

#define MAP_DSCP_PCP_MAP_SIZE	64

/* DSCP Mapping Table TLV */
typedef struct i5_dscp_pcp_map {
  unsigned char pcp_to_dscp[MAP_DSCP_PCP_MAP_SIZE]; /* PCP Values ordered by
                                    * increasing DSCP Values. List of 64 PCP values (one octet per value)
                                    * corresponding to the DSCP markings 0x00 to 0x3F,
                                    * ordered by increasing DSCP Value. This table is used to select a PCP value
                                    * if a Service Prioritization Rule specifies Rule Output: 0x08
                                    */
  bool is_valid; /* Flag indicating DSCP Mapping Table TLV Inserted/Extracted */
} i5_dscp_pcp_map_t;

/* QoS Management Descriptor TLV */
typedef struct i5_qosmgmt_desc {
  dll_t			node;
  unsigned short qm_id; /* An identifier
                                    * that uniquely identifies a QoS Management rule
                                    */
  unsigned char bssid[IEEE1905_MAC_ADDR_LEN]; /* BSSID of BSS
                                    * for which this descriptor applies
                                    */
  unsigned char client_mac[IEEE1905_MAC_ADDR_LEN]; /* MAC address of STA
                                    * for which this descriptor applies
                                    */
  unsigned char *descriptor; /* One Of : Variable Length :
                                    * MSCS / SCS / QoS Management - Descriptor Element
                                    */
  unsigned int descriptor_len; /* Length of the Descriptor Element
                                    */
} i5_qosmgmt_desc_t;

/* Structure for Service Prioritization Request Message */
typedef struct i5_serv_prio_req_msg {

  ieee1905_glist_t serv_prio_rule_list; /* List of i5_serv_prio_rule_t type objects */

  i5_dscp_pcp_map_t dscp_pcp_map; /* DSCP Mapping Table TLV */

  ieee1905_glist_t qosmgmt_desc_list; /* List of i5_qosmgmt_desc_t type objects */

} i5_serv_prio_req_msg_t;

/* Structure for QoS Management Notification Message */
typedef struct i5_qosmgmt_notif_msg {

  ieee1905_glist_t qosmgmt_desc_list; /* List of i5_qosmgmt_desc_t type objects */

} i5_qosmgmt_notif_msg_t;

/* For Unsuccessful Association count value to make it reset after this Interval */
#define IEEE1905_RESET_UNSUCCESSFUL_ASSOC_COUNT_TIEMOUT		60

/* Report Unsuccessful Associations flags */
#define MAP_UNSUCCESSFUL_ASSOC_FLAG_REPORT	0x80 /* Indicates whether Multi-AP Agent should
* report unsuccessful association attempts of client STAs to the Multi-AP Controller
*  0: Do not report unsuccessful association attempts
*  1: Report unsuccessful association attempts
*/
/* Structure to hold unsuccessful association config params */
typedef struct {
  unsigned char report_flag; /* Unsuccessful assoc policy report flags of type
			    * MAP_UNSUCCESSFUL_ASSOC_FLAG_XXX */
  unsigned int max_reporting_rate; /*Maximum rate for reporting unsuccessful association
			  * attempts  */
  int count;    /* To count the number of report send to controller */
} ieee1905_unsuccessful_assoc_config_t;

typedef struct {
  ieee1905_glist_t no_steer_sta_list;	/* List of ieee1905_sta_list type objects */
  ieee1905_glist_t no_btm_steer_sta_list; /* List of ieee1905_sta_list type objects */
  ieee1905_glist_t steercfg_bss_list;	/* List of ieee1905_bss_steer_config type objects */
  ieee1905_metricrpt_config  metricrpt_config; /* Metric Reporting Policy */
  unsigned short prim_vlan_id;	/* Primary Vlan Id for this R2 network */
  unsigned char	default_pcp;		/* Default pcp for this R2 network */
  ieee1905_glist_t ts_policy_list;	/* List of ieee1905_ts_policy_t type object */
  ieee1905_chscanrpt_config  chscanrpt_config; /* Channel Scan Reporting Policy */
  ieee1905_unsuccessful_assoc_config_t  unsuccessful_assoc_config; /* Unsuccessful Association config Policy */
  ieee1905_glist_t no_bh_bss_list;	/* List of ieee1905_bh_bss_list type objects */
  ieee1905_qosmgmt_policy_t qosmgmt_config; /* Controller's QoS Management Policy config */
  ieee1905_glist_t agent_ap_mld_conf; /* Agent AP MLD Configuration list of type
                                       * i5_agent_ap_mld_conf_t. This will be filled and cleanedup
                                       * after processing. Used while getting in WSC M2,
                                       * BSS Configuration response, Topology Response and
                                       * BSS Configuration result messages
                                       */
  i5_bsta_mld_conf_t *bsta_mld_conf;  /* Backhaul STA MLD Configuration. This will be filled and
                                       * cleanedup after processing. USed while getting in WSC M2,
                                       * BSS Configuration response, Topology Response and
                                       * BSS Configuration result messages
                                       */
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

typedef operating_rpt_opclass_chan_list opclass_chan_list;

typedef struct {
  unsigned char radio_mac[IEEE1905_MAC_ADDR_LEN];
  unsigned char n_op_class;
  operating_rpt_opclass_chan_list *list;
  unsigned char tx_pwr;
} ieee1905_operating_chan_report;

typedef struct {
  unsigned char bssid[IEEE1905_MAC_ADDR_LEN];	/* bss mac */
  unsigned char assoc_allowance_status; /* BSS is capable of accepting assoc req or not */
}  __attribute__((__packed__)) association_status_notification_bss;

typedef struct {
  unsigned char count;
  association_status_notification_bss *list;	/* Number of BSS reporting association status
						 * notification to controller
						 */
} ieee1905_association_status_notification;

/* Types of payloads which can be sent in tunneled msg */
typedef enum ieee1905_tunnel_msg_payload_type {
  ieee1905_tunnel_msg_payload_assoc_rqst	= 0x00,	/* Association Request */
  ieee1905_tunnel_msg_payload_re_assoc_rqst	= 0x01, /* Reassociation Request */
  ieee1905_tunnel_msg_payload_btm_query		= 0x02,	/* BSS Transition Management Query */
  ieee1905_tunnel_msg_payload_wnm_rqst		= 0x03,	/* WNM Notification Request */
  ieee1905_tunnel_msg_payload_anqp_rqst		= 0x04,	/* GAS ANQP Query */
  ieee1905_tunnel_msg_payload_dscp_policy_query	= 0x05,	/* DSCP Policy Query */
  ieee1905_tunnel_msg_payload_dscp_policy_resp	= 0x06	/* DSCP Policy Response */
} ieee1905_tunnel_msg_payload_type_t;

typedef struct ieee1905_tunnel_msg {
  unsigned char source_mac[MAC_ADDR_LEN];	/* source mac address in tunnel message */
  ieee1905_tunnel_msg_payload_type_t payload_type; /* tunnel message type */
  unsigned char *payload;	/* one or more Tunneled TLV */
  uint32 payload_len;	/* length of total Tunneled TLV */
} ieee1905_tunnel_msg_t;

/* per radio cac input params from controller */
typedef struct cac_params {
  unsigned char mac[IEEE1905_MAC_ADDR_LEN];	/* bss mac */
  unsigned char opclass;
  unsigned char chan;
} __attribute__((__packed__)) ieee1905_radio_cac_params_t;

/* per radio cac input params to initiate cac request from controller */
typedef struct cac_rqst {
  unsigned char mac[IEEE1905_MAC_ADDR_LEN];	/* bss mac */
  unsigned char opclass;
  unsigned char chan;
  unsigned char flags; /* bit specific,
			* 2-0 : Reserved
			* 4-3 : cac completion action: of type MAP_CAC_ACTION_XXX
			*	0 - Remain on channel, monitor radar
			*	1 - switch to previous channel
			*
			* 7-5 : cac method to be use: of type MAP_CAC_METHOD_XXX
			*	0 - continous cac
			*	1 - continous with dedicated radio
			*	2 - MIMO dimension reduced
			*	3 - Time Sliced CAC
			*/
} __attribute__((__packed__)) ieee1905_radio_cac_rqst_t;

/* per radio cac information agent provides to controller after serving cac rqst */
typedef struct cac_complete {
  unsigned char mac[IEEE1905_MAC_ADDR_LEN];	/* bss mac */
  unsigned char opclass;
  unsigned char chan;
  unsigned char status; /* status of performing cac on radio */
  unsigned char n_opclass_chan; /* radar detected on number of channels */
  opclass_chan_list list[];	/* opclass and chan list needs to be filled only for n_opclass_chan > 0 */
} __attribute__((__packed__)) ieee1905_radio_cac_completion_t;

/* 1905 standard cac request TLV from controller */
typedef struct ieee1905_cac_rqst {
  unsigned char count;
  ieee1905_radio_cac_rqst_t params[1];
} ieee1905_cac_rqst_list_t;

/* 1905 standard cac terminate TLV from controller */
typedef struct ieee1905_cac_terminate {
  unsigned char count;
  ieee1905_radio_cac_params_t params[1];
} ieee1905_cac_termination_list_t;

/* 1905 standard cac complete TLV from agent to controller */
typedef struct ieee1905_cac_completion {
  unsigned char count;
  ieee1905_radio_cac_completion_t params[1]; /* per 5g radio */
} ieee1905_cac_completion_list_t;

typedef struct ieee1905_rc_chan_list {
  uint8 opclass;	/* regulatory class */
  uint8 n_chan;		/* supported channels in regulatory class */
  uint8 chan[0];	/* list of channels */
} __attribute__ ((__packed__)) ieee1905_rc_chan_list_t;

typedef struct ieee1905_cac_mode_info {
  uint8 mode;		/* type of cac interface supports i.e. ZWDFS/Continous CAC */
  uint8 seconds[3];	/* seconds to complete CAC request */
  uint8 n_opclass;	/* supported operating class for interface */
  ieee1905_rc_chan_list_t list[1]; /* operating class -- n_chan -- list of channels */
} __attribute__ ((__packed__)) ieee1905_cac_mode_info_t;

typedef struct ieee1905_ifr_cac_info {
  uint8 mac[6];		/* Radio mac */
  uint8 n_cac_types;	/* number of CAC types Radio support (cac method + time to complete CAC) */
  ieee1905_cac_mode_info_t mode_info[1];
} __attribute__ ((__packed__)) ieee1905_ifr_cac_info_t;

typedef struct ieee1905_cac_capabilities {
  uint8 country[2];	/* 2 byte char represents country without ascii '\0' */
  uint8 n_radio;	/* number of radio in cac capability */
  ieee1905_ifr_cac_info_t ifr_info[0];
} ieee1905_cac_capabilities_t;

/* Network Key Type */
typedef struct {
  unsigned char	key_len;
  unsigned char	key[IEEE1905_MAX_KEY_LEN + 1]; /* +1 to take care of NULL termination */
} ieee1905_network_key_type;

#define	MAP_BSSINFO_BRCM_VS_TLV           0x1u
#define	MAP_BSSINFO_BRCM_VS_FBT_TLV       0x2u  /* Vendor Specific TLV for FBT data */
#define	MAP_BSSINFO_CONFIG_STEER_DISABLED 0x4u  /* MAP BSS info config flag to disable steering */
#define	MAP_BSSINFO_MLO_DISABLED          0x8u  /* MLO is disabled for this BSS info */
#define	IS_MAP_BSSINFO_BRCM_VS_TLV_RCVD(f)	((f) & MAP_BSSINFO_BRCM_VS_TLV)
#define	IS_MAP_BSSINFO_BRCM_VS_FBT_TLV_RCVD(f)	((f) & MAP_BSSINFO_BRCM_VS_FBT_TLV)
#define	IS_MAP_BSSINFO_CONFIG_STEER_DISABLED(f) ((f) & MAP_BSSINFO_CONFIG_STEER_DISABLED)
#define	IS_MAP_BSSINFO_MLO_DISABLED(f)          ((f) & MAP_BSSINFO_MLO_DISABLED)

/* FBT info */
typedef struct {
	unsigned short mdid;			/* MDID of the blanket */
	unsigned char ft_cap_policy;		/* 9.4.2.47 FBT Capab & Policy, eg FBT over DS */
	unsigned int tie_reassoc_interval;	/* 9.4.2.49 Reassociation deadline interval */
} ieee1905_fbt_info_type;

/* Structure to hold the json encoded dpp config response string */
typedef struct {
  dll_t node;			/* self referencial (next,prev) pointers of type dll_t */
  char *config_resp_str;	/* JSON encoded dpp config response str */
  unsigned int config_resp_len;	/* Dpp config response string len */
} ieee1905_dpp_config_resp_type_t;

#define I5_DPP_PARAMS_MAX_SIZE	512

/* BSS Info of M2 WSC Message */
typedef struct {
  dll_t                     node;
  char                      bss_name[I5_MAX_IFNAME];  /* Name of the BSS which is stored in
                                                       * NVRAM for ex: fh
                                                       */
  unsigned char             ALID[IEEE1905_MAC_ADDR_LEN];
  unsigned char             curALID[IEEE1905_MAC_ADDR_LEN]; /* ALID of the current neighbor when
                                                             * creating DPP config object for BSS
                                                             * configuration response
                                                             */
  unsigned char             band_flag;
  ieee1905_ssid_type        ssid;
  unsigned short            AuthType; /* Of Type IEEE1905_AUTH_XXX */
  unsigned short            EncryptType;  /* Of Type IEEE1905_ENCR_XXX */
  ieee1905_network_key_type NetworkKey;
  unsigned char		    TearDown;
  unsigned char             map_flag;
  unsigned char             Closed;
  unsigned char             isolate;
  unsigned char             flags;	/* of type MAP_BSSINFO_XXX */
  unsigned char             disabled;
  ieee1905_fbt_info_type    fbt_info;
  unsigned char             RUID[IEEE1905_MAC_ADDR_LEN];	/* Interface mac address */
  unsigned char             BSSID[IEEE1905_MAC_ADDR_LEN];	/* Bss mac address */
  char dpp_connector[I5_DPP_PARAMS_MAX_SIZE];		/* DPP connector used only in agent */
  char dpp_csign[I5_DPP_PARAMS_MAX_SIZE];		/* DPP C-sign */
} ieee1905_client_bssinfo_type;

/* Agent info list Message */
typedef struct i5_agentlist{
  dll_t                     node;
  unsigned char             ALID[IEEE1905_MAC_ADDR_LEN]; /* Agents ALMAC address */
  unsigned char             map_profile; /* Agents map profile */
  unsigned char             SecType; /* TODO Agents IEEE1905 security type */
  i5_dm_device_type         *pDevice; /* Pointer to the neighbor device */
} i5_agentlist_t;

/* Interface Info */
typedef struct {
  unsigned short chanspec;  /* Chanspec of the interface */
  ieee1905_ap_caps_type ap_caps;  /* AP capability */
  unsigned char mapFlags; /* Of Type IEEE1905_MAP_FLAG_XXX */
  unsigned char self_mld_addr[MAC_ADDR_LEN];  /* Self MLD MAC address. Only at Agent */
  unsigned char bssid[IEEE1905_MAC_ADDR_LEN]; /* BSSID of the AP where the bSTA is connected */
  unsigned char bsta_peer_mld[MAC_ADDR_LEN];  /* peer MLD MAC address to which MLO bSTA is
                                               * connected. This will be NULL if the primary is AP
                                               */
} ieee1905_ifr_info;

typedef enum ieee1905_tlv_err_codes {
  ieee1905_tlv_err_reserved = 0x00,
  ieee1905_tlv_err_sta_associated = 0x01,
  ieee1905_tlv_err_sta_not_associated = 0x02,
  ieee1905_tlv_err_client_cap_report_error = 0x03,
  ieee1905_tlv_err_backhaul_steer_reject_wrong_chan = 0x04,
  ieee1905_tlv_err_backhaul_steer_reject_low_signal = 0x05,
  ieee1905_tlv_err_backhaul_steer_reject_auth = 0x06,
} ieee1905_tlv_err_codes_t;

typedef struct {
  unsigned char neighbor_al_mac[IEEE1905_MAC_ADDR_LEN]; /* AL ID from where request came (Used only at agent) */
  unsigned char bh_sta_mac[IEEE1905_MAC_ADDR_LEN]; /* Backhaul STA mac adress to be steered */
  unsigned char trgt_bssid[IEEE1905_MAC_ADDR_LEN]; /* BSS where the backhaul STA is going to steer to */
  unsigned char opclass; /* Target BSS operating class */
  unsigned char channel; /* Target BSS channel */
  unsigned char resp_status_code; /* status code for backhaul steering response */
  ieee1905_tlv_err_codes_t error_code; /* error code for backhaul steering response */
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

/* 1905 Encap EAPOL TLV fields */
typedef struct i5_1905_encap_eapol {
  unsigned short frame_length;  /* Length of EAPOL frame payload */
  unsigned char *frame; /* EAPOL frame payload */
} i5_1905_encap_eapol_t;

/* 1905 Encap DPP TLV flag */
#define I5_1905_ENCAP_DPP_FLAGS_ENROLLE_MAC_PRESENT     0x80  /* bit 7. This is set to specify the
                                                               * address of the Enrollee Multi-AP
                                                               * Agent to the Proxy Agent and
                                                               * Multi-AP Controller
                                                               */
/* bit 6 is Reserved */
#define I5_1905_ENCAP_DPP_FLAGS_DPP_FRAME_INDICATOR_GAS 0x20  /* bit 5. Content type of the
                                                               * encapsulated DPP frame. If this bit
                                                               * is set, content is GAS Frame, else
                                                               * DPP Public Action Frame
                                                               */

/* 1905 Encap DPP TLV fields */
typedef struct i5_1905_encap_dpp {
  unsigned char flags;  /* Of type I5_1905_ENCAP_DPP_FLAGS_XXX */
  unsigned char enrollee_mac[MAC_ADDR_LEN]; /* Destination STA MAC Address */
  unsigned char frame_type; /* If the DPP Frame Indicator (bit 5) is 0, this field is set to the
                             * DPP Public Action frame type. Otherwise this field is set to 255
                             */
  unsigned short frame_length;  /* Length of encapsulated frame */
  unsigned char *frame; /* If bit 5=0, this field contains a DPP Public Action frame else this
                         * field contains a GAS frame that carries the DPP Configuration Protocol
                         * payload
                         */
} i5_1905_encap_dpp_t;

/* DPP Message TLV fields */
typedef struct i5_direct_encap_dpp {
  unsigned char frame_type; /* This field is set to the DPP Public Action frame type. */
  unsigned short frame_length;  /* Length of encapsulated frame */
  unsigned char *frame; /* This field contains actual DPP Public Action frame payload */
} i5_direct_encap_dpp_t;

/* Made common across 1905 and WBD2 stack */
#define BAND_INV  0x0
#define BAND_2G   0x1
#define BAND_5GL  0x2
#define BAND_5GH  0x4
#define BAND_6G	  0X8

#define REGCLASS_24G_FIRST	 81
#define REGCLASS_24G_LAST	 84
#define REGCLASS_5GL_20MHZ_1	115
#define REGCLASS_5GL_20MHZ_2	118
#define REGCLASS_5GH_20MHZ_1	121
#define REGCLASS_5GH_20MHZ_2	125
#define REGCLASS_5GL_FIRST	REGCLASS_5GL_20MHZ_1
#define REGCLASS_5GL_40MHZ_LAST	120
#define REGCLASS_5GH_FIRST	REGCLASS_5GH_20MHZ_1
#define REGCLASS_5GH_40MHZ_LAST	127
#define REGCLASS_5G_40MHZ_LAST	REGCLASS_5GH_40MHZ_LAST
#define REGCLASS_5G_80MHZ	128
#define REGCLASS_5G_160MHZ	129
#define REGCLASS_5GH_LAST	130
#define REGCLASS_5G_FIRST	REGCLASS_5GL_FIRST
#define REGCLASS_5G_LAST	REGCLASS_5GH_LAST
#define REGCLASS_6G_20MHZ	131
#define REGCLASS_6G_FIRST	REGCLASS_6G_20MHZ
#define REGCLASS_6G_40MHZ	132
#define REGCLASS_6G_80MHZ	133
#define REGCLASS_6G_160MHZ	134
#define REGCLASS_6G_320MHZ	137
#define REGCLASS_6G_LAST	REGCLASS_6G_320MHZ

#define CHANNEL_24G_FIRST	  1
#define CHANNEL_24G_LAST	 14
#define CHANNEL_5GL_FIRST	 36
#define CHANNEL_5GL_LAST	 64
#define CHANNEL_5GH_FIRST	100
#define CHANNEL_5GH_LAST	177
#define CHANNEL_5G_FIRST	CHANNEL_5GL_FIRST
#define CHANNEL_5G_LAST		CHANNEL_5GH_LAST
#define	CHANNEL_6G_FIRST	1
#define CHANNEL_6G_LAST		233

typedef enum i5_msg_types_with_vndr_tlv {

  i5MsgMultiAPPolicyConfigRequestValue = 1,
  i5MsgMultiAPGuestSsidValue = 2,

} i5_msg_types_with_vndr_tlv_t;

/* Struct to hold the data attributes received from bss config request */
typedef struct {
  char name[I5_BUF_SIZE_128];		/* Device Name */
  char wifi_tech[I5_BUF_SIZE_64];	/* wi-fi_tech */
  char netrole[I5_BUF_SIZE_64];		/* netRole */
} i5_bss_config_req_attrs_t;

/* Struct to hold data for dpp uri notification msg */
typedef struct {
  unsigned char RUID[MAC_ADDR_LEN];	/* Radio unique ID of the Radio */
  unsigned char BSSID[MAC_ADDR_LEN];	/* MAC Address of Local Interface (equal to BSSID)
					 * operating on the radio, on which the URI was
					 * received during PBC onboarding */
  unsigned char bSTA_MAC[MAC_ADDR_LEN];	/* MAC Address of backhaul STA from which the URI
					 * was received during PBC onboarding */
  char *dpp_uri;			/* DPP URI received during PBC onboarding */
  uint16 dpp_uri_len;			/* DPP URI length */
} i5_dpp_bootstrap_uri_notification_t;

/* Multi-AP Profile Values */
typedef enum ieee1905_profiles {
  ieee1905_map_profile0 = 0,  /* Multi AP Profile-0 (MultiAP-R1) */
  ieee1905_map_profile1 = 1,  /* Multi AP Profile-1 (MultiAP-R4) */
  ieee1905_map_profile2 = 2,  /* Multi AP Profile-2 (MultiAP-R2) */
} ieee1905_profiles_t;

/* 1905 Layer Security Capability TLV's Onboarding Protocol Fields */
#define MAP_ONBOARDING_PROTO_DPP  0x00  /* 1905 Device Provisioning Protocol */

/* 1905 Layer Security Capability TLV's MIC Algorithm Fields */
#define MAP_MSG_INTEGRITY_HMAC_SHA256 0x00  /* HMAC-SHA256 */

/* 1905 Layer Security Capability TLV's Encryption Algorithm Fields */
#define MAP_ENCR_ALGO_AES_SIV 0x00  /* AES-SIV */

/* Agent list TLV security field values */
#define MAP_1905_SEC_DISABLED 0x00  /* 1905 Security not enabled */
#define MAP_1905_SEC_ENABLED 0x01   /* 1905 Security enabled */

/* DPP frame attribute header */
typedef struct {
  uint16 id;	/* Attribute ID */
  uint16 length;	/* Length of the attribute. */
} __attribute__((__packed__)) i5_dpp_attribute_t;

/* DPP Atrribute ID's Identifying the type of DPP attribute. */
#define I5_DPP_ATTR_I_BOOTSTRAP_KEY_HASH  0x1001u /* Initiator Bootstrapping Key Hash */
#define I5_DPP_ATTR_R_BOOTSTRAP_KEY_HASH  0x1002u /* Responder Bootstrapping Key Hash */
#define I5_DPP_ATTR_CONFIG_OBJ            0x100Cu /* DPP Configuration Object */
#define I5_DPP_ATTR_CONNECTOR             0x100Du /* DPP Connector */
#define I5_DPP_ATTR_CONFIG_REQ_OBJ        0x100Eu /* DPP Configuration Request object */
#define I5_DPP_ATTR_CSIGNKEY_HASH         0x101Eu /* C-sign-key Hash */

/* Notiifcation command ID to be passed to notify of any action to WBD */
/* BSS info got updated via command line or some other source(Not from wbd_master) */
#define I5_NOTIFY_CMD_BSS_INFO_UPDATE   1U
/* Inform agent to start NIP or WSC M1 */
#define I5_NOTIFY_CMD_START_NIP         2U

#define MAP_DEF_STP_BRIDGE_PRIO	0x7000	/* Default STP bridge priority of controller agent */
#define MAP_MAX_STP_BRIDGE_PRIO	0xFFFF	/* Maximum value of STP bridge priority */

/** @brief Callback to inform about the device creation(i5_dm_device_type) in the data model
 *
 * @param pdevice	Pointer to the device created. Can be self device also
 *
 */
typedef void ieee1905_device_init(i5_dm_device_type *pdevice);

/** @brief Callback to inform after First Topology Resp for newly created device(i5_dm_device_type) in the data model
 *
 * @param pdevice	Pointer to the device created. Cannot be self device
 *
 */
typedef void ieee1905_device_update(i5_dm_device_type *pdevice);

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
typedef void ieee1905_steer_request(ieee1905_steer_req *steer_req);

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
 * @param pDevice       Device from where channel selection request received
 * @param pInterface    Interface for which channel selection request received
 * @param cp			Channel prefernce information from controller
 * @param rclass_local_count	Number of local preferences
 * @param local_chan_pref	Local Channel Preferences
 *
 */
typedef void ieee1905_recv_channel_selection_request(i5_dm_device_type *pDevice,
  i5_dm_interface_type *pInterface, ieee1905_chan_pref_rc_map_array *cp,
  unsigned char rclass_local_count, ieee1905_chan_pref_rc_map *local_chan_pref);

/** @brief Callback function to receive Channel selection response
 *
 * @param pDevice		Device from where channel selection response received
 * @param pInterface		Interface for which this callback is applicable
 * @param chan_sel_resp_code	Channel selection response code
 *
 */
typedef void ieee1905_recv_channel_selection_response(i5_dm_device_type *pDevice,
  i5_dm_interface_type *pInterface, t_I5_CHAN_SEL_RESP_CODE chan_sel_resp_code);

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
  ieee1905_wifi6_sta_status *wifi6_sta_status, ieee1905_vendor_data *out_vndr_data);

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
 * @param policy  New Policy Configuration recieved from the controller
 * @param policy_type Flag of type MAP_POLICY_TYPE_FLAG_XXX which tells the type of policy recieved
 *
 * @return		            status of the call. 0 Success. Non Zero Failure
 */
typedef int ieee1905_create_bss_on_ifr(char *ifname,
  ieee1905_glist_t *bssinfo_list, ieee1905_policy_config *policy, unsigned char policy_type);

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
 * @param policy_type     Flag of type MAP_POLICY_TYPE_FLAG_XXX which tells the policy recieved
 * @param in_vendor_tlv   In Obj : Vendor Specific TLV data in TLV format
 *
 * @return            status of the call. 0 Success. Non Zero Failure
 */
typedef void ieee1905_configure_policy(ieee1905_policy_config *policy, unsigned short policy_type,
	ieee1905_vendor_data *in_vendor_tlv);

/** @brief Callback function to inform the associated STA link metrics response from agent.
 *
 * @param pdmclient     Pointer of the STA for which the STA link metrics came
 * @param metric        Metric got from the Agent
 */
typedef void ieee1905_asoc_sta_metric_resp(i5_dm_clients_type *pdmclient,
  ieee1905_sta_link_metric *metric);

/** @brief Callback function to inform the unassociated STA link metrics response from agent.
 *
 * @param pdevice   Pointer to the device from where the Unassociated STA link metrics came
 * @param metric    Metric got from the Agent
 */
typedef void ieee1905_unasoc_sta_metric_resp(i5_dm_device_type *pdevice,
  ieee1905_unassoc_sta_link_metric *metric);

/** @brief Callback function to inform the beacon metrics response from agent.
 *
 * @param pDevice   Pointer to the device from where the Beacon metrics came
 * @param report    Beacon report got from the Agent
 */
typedef void ieee1905_beacon_metric_resp(i5_dm_device_type *pdevice,
  ieee1905_beacon_report *report);

/** @brief Callback function to inform AP auto configuration
 *
 * @param pDevice	  Device pointer of the remote device
 * @param pdmif     Pointer to the interface of the radio which is configured
 * @param if_band	  Band in which the radio is confiugred (INV, 2G, 5GL, 5GH)
 */
typedef void ieee1905_ap_configured(i5_dm_device_type *pdevice, i5_dm_interface_type *pdmif,
  int if_band);

/** @brief Callback function to inform operating channel report
 *
 * @param pdmif			Pointer to the interface for which operating channel report received
 * @param chan_report Operating channel report information
 * @param operating_chan_report
 */
typedef void ieee1905_operating_channel_report(i5_dm_interface_type *pdmif,
  ieee1905_operating_chan_report *chan_report);

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
 * @param	pDeviceController   Pointer to the controller device model
 * @param recv_dpp_chirp         DPP Chirp TLV values sent by Controller recv by Agent
 * @return	Status code of the function call 0: Success Non-zero: Stop further message processing
 */
typedef int ieee1905_ap_auto_config_resp(i5_dm_device_type *pDeviceController,
  i5_dpp_chirp_value_t *recv_dpp_chirp);

/** @brief Callback to process 1905 AP-Autoconfiguration Search message by controller
 *
 * @param peer_dpp_chirp         DPP Chirp TLV values sent by Peer Device
 * @param neighbor_al_mac   AL MAC address of the neighbor which sent the DPP Chirp Notification
 * @return	Bootstrap ID of Bootstrap matching with Chirp
 *				0 : If matching Bootstrap is NOT found
 *			  Non-zero : If matching Bootstrap is found
 */
typedef int ieee1905_process_ap_auto_config_search_chirp(i5_dpp_chirp_value_t *peer_dpp_chirp,
  unsigned char *neighbor_al_mac);

/** @brief Callback to inform 1905 AP-Autoconfiguration Search message sent
 */
typedef void ieee1905_ap_auto_config_search_sent();

/** @brief Generic callback to set backhaul sta parameters
 *
 * @param cmd     Selects the command to backhaul STA
 * @param i5_ifr  Interface name to disable the roaming on all STAs except this interface
 *
 */
typedef void ieee1905_set_bh_sta_params(t_i5_bh_sta_cmd cmd, i5_dm_interface_type *i5_ifr);

/** @brief Callback to inform the DFS status update on the operating channel of an interface
 *
 * @param i5_ifr      Interface for which the DFS update has happened
 * @param old_flags   Old value of flags of type I5_FLAG_IFR_XXX before updating it
 * @param new_flags   New value of flags of type I5_FLAG_IFR_XXX after updating it
 *
 */
typedef void ieee1905_operating_channel_dfs_update(i5_dm_interface_type *i5_ifr,
  unsigned char old_flags, unsigned char new_flags);

/** @brief Callback to inform the Non-operable Channels update of an interface
 *
 * @param i5_ifr      Interface for which the DFS update has happened
 * @param dynamic     Non-Operable Channel Update is for Static(0) or Dynamic(1) Channels, based
 *                 on where the callback is called, On getting Radio Caps(0) or Chan Pref Report(1)
 *
 */
typedef void ieee1905_nonoperable_channel_update(i5_dm_interface_type *i5_ifr, int dynamic);

/** @brief Callback function to handle Tunnel message in Multi AP
 *
 * @param pDevice Pointer to the remote device
 * @param msg	tunnel msg contains source info TLV mac + tunneled TLV + Tunneled payload
 */
typedef void ieee1905_process_tunneled_msg(i5_dm_device_type *pDevice, ieee1905_tunnel_msg_t *msg);

/** @brief Callback function to Process Channel Scan Request at App Layer
 *
 * @param al_mac	  AL Mac address of the remote device
 * @param chscan_req	Channel Scan Request Information
 *
 */
typedef void ieee1905_channel_scan_request(unsigned char *al_mac, ieee1905_chscan_req_msg *chscan_req);

/** @brief Callback function to Process Channel Scan Report at App Layer
 *
 * @param pDevice	  Agent's device pointer
 * @param ts_chscan_rpt	Timestamp at which Channel Scan Report is Sent
 * @param chscan_rpt	Channel Scan Report Information
 *
 */
typedef void ieee1905_channel_scan_report(i5_dm_device_type *pDevice, time_t ts_chscan_rpt,
  ieee1905_chscan_report_msg *chscan_rpt);

/** @brief Callback function to handle CAC message in Multi AP
 *
 * @param al_mac AL mac address of remote device
 * @param msg	message
 * @param msg_type cac request/cac complete/cac terminate/cac status/cac capability
 */
typedef void ieee1905_process_cac_msg(uint8 *al_mac, void *msg, uint32 msg_type);

/** @brief Callback function to prepare CAC complete TLV
 *
 * @output_param  pbuf
 * @output_param  payload_len
 */
typedef void ieee1905_prepare_cac_completion(uint8 **pbuf, uint32 *payload_len);

/** @brief Callback function to controller for process CAC completion TLV
 *
 * @param pDevice	Pointer to the device of the agent
 * @param cac_list	CAC completion data
 */
typedef void ieee1905_process_cac_completion(i5_dm_device_type *pDevice,
  ieee1905_cac_completion_list_t *cac_list);

/** @brief Callback function to controller for process CAC status TLV
 *
 * @param pdevice	Pointer to the device from which the CAC status TLV is received
 */
typedef void ieee1905_process_cac_status(i5_dm_device_type *pdevice);

/** @brief Callback function to prepare CAC capability for radio
 *
 * @output_param  pbuf		cac capability info of each cac capable radio
 * @output_param  payload_len	length of cac capablity info in pbuf
 */
typedef void ieee1905_prepare_cac_capabilities(uint8 **pbuf, uint16 *payload_len);

/** @brief Callback function to get profile of the device where current device is connected.
 * If any backhaul STA is connected to current backhaul BSS, then it will get the profile
 * of the device from WDS ifname. If the current backhaul STA is connected to any backhaul BSS
 * then it will get profile of the device where backhaul BSS is present
 *
 * @param ifname  Interface name. WDS ifname in case of backhaul BSS or backhaul STA ifname
 *
 * @return			Profile value. 0 - MultiAP-R1, 1 - Profile1(MultiAP-R4) or 2 - Profile2
 */
typedef unsigned char ieee1905_get_bh_sta_profile(char *ifname);

/** @brief Callback function to set or unset association disallowed attribute in beacon
 *
 * @param ifname  Interface name of the BSS
 * @param reason  0 means allow association else reason for setting association disallowed
 *
 * @return        status of the call. 0 Success. Non Zero Failure
 */
typedef unsigned char ieee1905_mbo_assoc_disallowed(char *ifname, unsigned char reason);

/** @brief Callback function to prepare CAC status for MAP agent
 *
 * @output_param  pbuf		cac status info of each cac capable radio
 * @output_param  payload_len	length of cac status info in pbuf
 */
typedef void ieee1905_prepare_cac_status(uint8 **pbuf, uint16 *payload_len);

/** @brief Callback function to remove and deauth STA entry.
 *
 * @param ifname	Interface name of the BSS
 * @param bssid         BSSID where the STA is associated
 * @param sta_mac       MAC address of the STA
*/
typedef void ieee1905_remove_and_deauth_sta_entry(char *ifname, unsigned char *bssid,
  unsigned char *sta_mac);

/** @brief Callback function to set DFS channel clear, as indicated by the controller
 *
 * @param ifname	Interface name
 * @param chan_pref     channel preferce information
*/
typedef void iee1905_set_dfs_chan_clear(char *ifname, ieee1905_chan_pref_rc_map *chan_pref);

/** @brief Callback function to get primary VLAN ID for the backhaul STA
 *
 * @param ifname	Interface name
*/
typedef unsigned short iee1905_get_primary_vlan_id(char *ifname);

/** @brief Callback function to indicate DPP CCE
 *
 * @param enable	Enable/Disable CCE advertisement in beacons and probe responses
*/
typedef void iee1905_dpp_cce_indication(int enable);

/** @brief Callback function to inform DPP chirp notification
 *
 * @param dpp_chirp         DPP Chirp TLV values
 * @param neighbor_al_mac   AL MAC address of the neighbor which sent the DPP Chirp Notification
*/
typedef void iee1905_dpp_chirp_notification(i5_dpp_chirp_value_t *dpp_chirp,
  unsigned char *neighbor_al_mac);

/** @brief Callback function to inform 1905 Encap EAPOL Message
 *
 * @param encap_1905_eapol  1905 Encap EAPOL TLV values
 * @param neighbor_al_mac   AL MAC address of the neighbor which sent the 1905 encap EAPOL Message
*/
typedef void iee1905_encap_1905_eapol(i5_1905_encap_eapol_t *encap_1905_eapol,
  unsigned char *neighbor_al_mac);

/** @brief Callback function to ifnorm Proxied Encap DPP Message
 *
 * @param dpp_1905_encap    1905 Encap DPP TLV values
 * @param dpp_chirp         DPP Chirp TLV values
 * @param neighbor_al_mac   AL MAC address of the neighbor which sent the Proxied Encap DPP Message
*/
typedef void iee1905_dpp_proxied_encap(i5_1905_encap_dpp_t *dpp_1905_encap,
  i5_dpp_chirp_value_t *dpp_chirp, unsigned char *neighbor_al_mac);

/** @brief Callback function to ifnorm Direct Encap DPP Message
 *
 * @param dpp_1905_encap    DPP Message TLV values
 * @param neighbor_al_mac   AL MAC address of the neighbor which sent the Direct Encap DPP Message
*/
typedef void iee1905_dpp_direct_encap(i5_direct_encap_dpp_t *dpp_direct_encap,
  unsigned char *neighbor_al_mac);

/** @brief Callback function to get JSON encoded dpp config request object
 *
 * @param obj		JSON encoded dpp config request object
 * @param len		Length of dpp config request object
*/
typedef void ieee1905_get_dpp_config_req_obj(char **obj, unsigned int *len);

/** @brief Callback function to get JSON encoded dpp config request object
 *
 * @param obj		Dpp config request object to be parsed
 * @param out_attrs	Dpp config request attributes filled fron json object
*/
typedef void ieee1905_parse_dpp_config_req_obj(void *obj, i5_bss_config_req_attrs_t *out_attrs);

/** @brief Callback function to inform AgentList Message
  *
  * @param agent_list    List of Agents and its information
  * @param neighbor_al_mac   AL MAC address of the neighbor which sent the AgentList Message
  */
typedef void iee1905_process_agentlist(ieee1905_glist_t *agent_list, unsigned char *neighbor_al_mac);

/** @brief Callback function to parse DPP Configuration Response received
 * from BSS Configuration Response message
 *
 * @param bss_info	BSS info object for generating JSON encoded dpp configuration response data
 * @param resp_obj	JSON encoded dpp configuration response data
 * @param obj_len	Length of dpp configuration response data
*/
typedef void ieee1905_get_dpp_config_resp_obj(ieee1905_client_bssinfo_type *bss_info,
  char **resp_obj, unsigned int *obj_len);

/** @brief Callback function to parse DPP Configuration Response received
 * from BSS Configuration Response message
 *
 * @param data		JSON encoded dpp configuration response data
 * @param out_bss_info	BSS info object filled using dpp configuration response data
*/
typedef void ieee1905_parse_dpp_config_resp_obj(void *data,
  ieee1905_client_bssinfo_type *out_bss_info);

/** @brief Callback function to parse DPP Configuration Response received
 * from BSS Configuration Response message
 *
 * @param pdevice	Pointer to the device from where dpp uri received
 * @param uri_data	Pointer to DPP bootstrap uri data
*/
typedef void ieee1905_dpp_bootstrap_uri_notification_receive(i5_dm_device_type *pdevice,
  i5_dpp_bootstrap_uri_notification_t *uri_data);

/** @brief Callback function to Notify WBD about some action
 *
 * @param action_id	Notification ID(this is of type MAP_NOTIFY_CMD_XXX)
 * @param arg Pointer to argument if any. Else NULL
*/
typedef void ieee1905_notify_command(uint32 action_id, void *arg);

/** @brief Callback function to Process Error Response Message
*/
typedef void ieee1905_process_err_resp_msg();

/** @brief Callback function to Process Service Prioritization Request Message
*
* @param in_sp_req       In Service Prioritization Request Message
* @param neighbor_al_mac   AL MAC address of the neighbor which sent this Message
*/
typedef void ieee1905_process_serv_prio_req_msg(i5_serv_prio_req_msg_t *in_sp_req,
  unsigned char *neighbor_al_mac);

/** @brief Callback function to Process QoS Management Notification Message
*
* @param in_qosmgmt_notif       In QoS Management Notification Message
* @param neighbor_al_mac   AL MAC address of the neighbor which sent this Message
*/
typedef void ieee1905_process_qosmgmt_notif_msg(i5_qosmgmt_notif_msg_t *in_qosmgmt_notif,
  unsigned char *neighbor_al_mac);

struct ieee1905_call_bks {
  ieee1905_device_init *device_init;
  ieee1905_device_update *device_update;
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
  ieee1905_recv_channel_selection_response *recv_chan_selection_resp;
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
  ieee1905_asoc_sta_metric_resp *assoc_sta_metric_resp;
  ieee1905_unasoc_sta_metric_resp *unassoc_sta_metric_resp;
  ieee1905_beacon_metric_resp *beacon_metric_resp;
  ieee1905_ap_configured *ap_configured;
  ieee1905_operating_channel_report *operating_chan_report;
  ieee1905_steering_btm_report *steering_btm_rpt;
  ieee1905_higher_layer_data *higher_layer_data;
  ieee1905_interface_chan_change *interface_chan_change;
  ieee1905_ap_auto_config_resp	*ap_auto_config_resp;
  ieee1905_process_ap_auto_config_search_chirp *process_ap_auto_config_search_chirp;
  ieee1905_ap_auto_config_search_sent	*ap_auto_config_search_sent;
  ieee1905_set_bh_sta_params *set_bh_sta_params;
  ieee1905_operating_channel_dfs_update *operating_channel_dfs_update;
  ieee1905_nonoperable_channel_update *nonoperable_channel_update;
  ieee1905_remove_and_deauth_sta_entry *remove_and_deauth_sta_entry;
  ieee1905_process_tunneled_msg *process_tunneled_msg;
  ieee1905_channel_scan_request *channel_scan_req;
  ieee1905_channel_scan_report *channel_scan_rpt;
  ieee1905_process_cac_msg *process_cac_msg;
  ieee1905_prepare_cac_completion *prepare_cac_complete;
  ieee1905_process_cac_completion *process_cac_complete;
  ieee1905_process_cac_status *process_cac_status;
  ieee1905_prepare_cac_capabilities *prepare_cac_capabilities;
  ieee1905_get_bh_sta_profile *get_bh_sta_profile;
  ieee1905_mbo_assoc_disallowed *mbo_assoc_disallowed;
  ieee1905_prepare_cac_status *prepare_cac_status;
  iee1905_set_dfs_chan_clear *set_dfs_chan_clear;
  iee1905_get_primary_vlan_id *get_primary_vlan_id;
  iee1905_dpp_cce_indication *dpp_cce_indication;
  iee1905_dpp_chirp_notification *dpp_chirp_notification;
  iee1905_encap_1905_eapol *encap_1905_eapol;
  iee1905_dpp_proxied_encap *dpp_proxied_encap;
  iee1905_dpp_direct_encap *dpp_direct_encap;
  ieee1905_get_dpp_config_req_obj *get_dpp_config_req_obj;
  ieee1905_parse_dpp_config_req_obj *parse_dpp_config_req_obj;
  ieee1905_get_dpp_config_resp_obj *get_dpp_config_resp_obj;
  ieee1905_parse_dpp_config_resp_obj *parse_dpp_config_resp_obj;
  iee1905_process_agentlist *process_agentlist;
  ieee1905_dpp_bootstrap_uri_notification_receive *process_dpp_bootstrap_uri_obj;
  ieee1905_notify_command *notify_command;
  ieee1905_process_err_resp_msg *process_err_resp_msg;
  ieee1905_process_serv_prio_req_msg *process_serv_prio_req_msg;
  ieee1905_process_qosmgmt_notif_msg *process_qosmgmt_notif_msg;
};

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
 * Deinit when exiting due to signal handlers. Just close the sockets and VLAN interfaces
 */
void ieee1905_deinit_on_sig();

/**
 * Load the traffic separation policy to i5_config.policyConfig.ts_policy_list
 *
 * @param config      Configurations containing traffic separation policy
 *
 * @return            None
 */
void ieee1905_load_ts_policy_config(ieee1905_config *config);

/**
 * Get the AL MAC address
 *
 * @return    AL MAC of the self device
 */
unsigned char *ieee1905_get_al_mac();

/**
 * Get the i5_config_type structure
 *
 * @return    Pointer to i5_config_type structure
 */
void *ieee1905_get_i5_config();

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
 * @param mld_addr  MLD MAC of the bss
 * @param mld_unit  MLD unit of this bss
 *
 * @return		      status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_add_bss(unsigned char *radio_mac, unsigned char *bssid, unsigned char *ssid,
  unsigned char ssid_len, unsigned short chanspec, char *ifname, unsigned char mapFlags,
  unsigned char *mld_addr, int8 mld_unit);

/**
 * Remove the BSS
 *
 * @param ifname    Interface Name
 *
 * @return          status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_remove_bss(char *ifname);

/**
 * get count of disabled bss in the bssinfo list
 *
 * @param bssinfo_list   link list of all bssinfo for a radio interface
 *
 * @return      count of the disabled bssinfo
 */
int ieee1905_ctlr_table_get_dis_bss_cnt(ieee1905_glist_t *bssinfo_list);

/**
 * Add the BSS to Controller table. only required in controller
 *
 * @param bss   BSS to be added
 *
 * @return      status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_add_bssto_controller_table(ieee1905_client_bssinfo_type *bss);

/**
 * Cleanup controller's BSS info table
 *
 * @return      status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_cleanup_controller_bss_info_table();

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
 * @param reason	assoc/disassoc status/Reason code
 * @param sta_mld	MLD address of the STA
 * @return		      status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_sta_assoc_disassoc(unsigned char *bssid, unsigned char *mac, int isAssoc,
  unsigned short time_elapsed, unsigned char notify, unsigned char *assoc_frame,
  unsigned int assoc_frame_len, uint16 reason, unsigned char *sta_mld);

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
 * @param pDeviceNeighbor   Neighbor Device Pointer
 * @param pclient           Pointer of the STA for which this report belongs
 *
 * @return        status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_assoc_sta_link_metric(i5_dm_device_type *pDeviceNeighbor,
  i5_dm_clients_type *pclient);

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
 * @param pDeviceNeighbor   Device pointer of the nieghbor
 * @param sta_mac           MAC address of the STA for which this report belongs
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_assoc_sta_link_metric_query(i5_dm_device_type *pDeviceNeighbor,
  unsigned char *sta_mac);

/**
 * Send UnAssociated STA Link Metrics Query message
 *
 * @param pDeviceNeighbor   Device pointer of the nieghbor
 * @param query             Unassociated STA link metrics query
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_unassoc_sta_link_metric_query(i5_dm_device_type *pDeviceNeighbor,
  ieee1905_unassoc_sta_link_metric_query *query);

/**
 * Send Beacon Metrics Query message
 *
 * @param pDeviceNeighbor   Device pointer of the nieghbor
 * @param query             Beacon metrics query details
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_beacon_metrics_query(i5_dm_device_type *pDeviceNeighbor,
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
 * @param pDeviceNeighbor  Agent's Device Pointer
 *
 */
void ieee1905_send_channel_preference_query(i5_dm_device_type *pDeviceNeighbor);

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
 * @param pDeviceNeighbor   Neighbor device pointer
 *
 * @return                  Returns the created ieee1905 message.
 */
void *ieee1905_create_channel_selection_request(i5_dm_device_type *pDeviceNeighbor);

/**
 * Send Policy Configuration to neighbor
 *
 * @param pDeviceNeighbor   Device Pointer to the nieghbor
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_policy_config(i5_dm_device_type *pDeviceNeighbor);

/**
 * Send AP Auto Configuration Renew
 *
 * @param pDeviceNeighbor  Agent's Device Pointer
 *
 */
void ieee1905_send_ap_autoconfig_renew(i5_dm_device_type *pDeviceNeighbor);

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
 * @param ifname        Interface Name
 * @param prim_vlan_id  Default 802.1Q Settings IE's Primary VLAN ID
 * @param is_8021q_present  Whether 802.1Q setting IE was present or not. If present, then only use
 *                          prim_vlan_id  argument.
 *
 * @return              status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_bSTA_associated_to_backhaul_ap(unsigned char *InterfaceId, char *ifname,
  unsigned short prim_vlan_id, int is_8021q_present);

/**
 * Send neighbor link metrics query
 *
 * @param pDeviceNeighbor         Neighbor device pointer
 * @param specify_neighbor        1 if neighbor is specified. 0 neighbor not specified
 * @param neighbor_of_recv_device If specify_neighbor is 1 then, it is 1905.1 AL MAC address of a
 *                                neighbor of the receiving device
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_neighbor_link_metric_query(i5_dm_device_type *pDeviceNeighbor,
  unsigned char specify_neighbor, unsigned char *neighbor_of_recv_device);

/**
 * Send AP Metrics Query
 *
 * @param pDeviceNeighbor   Neighbor device pointer
 * @param bssids            BSSIDs stored in a linear array. 6 octets for each BSSID
 * @param bssid_count       Number of BSSIDs in the bssids list
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_ap_metrics_query(i5_dm_device_type *pDeviceNeighbor,
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
 * Get band from the given opclass and channel
 *
 * @param opclass           operating class from E-4 table
 * @param channel           chanel number
 *
 * @return                  band of the channel
 */
int ieee1905_get_band_from_channel(unsigned char opclass, unsigned char channel);

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
 * @param pDeviceNeighbor   Neighbor device pointer
 * @param ifr_mac           MAC address of the radio
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_ap_metrics_response(i5_dm_device_type *pDeviceNeighbor, unsigned char *ifr_mac);

/**
 * Send Backhaul Steering Request
 *
 * @param pDeviceNeighbor   Neighbor device pointer
 * @param bh_steer_req      Backhaul Steer Request Parameter
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_backhaul_steering_request(i5_dm_device_type *pDeviceNeighbor,
  ieee1905_backhaul_steer_msg *bh_steer_req);

/**
 * To inform ieee1905 about disassociation of bSTA from the backhaul AP
 *
 * @param InterfaceId   MAC address of the radio
 *
 * @return              status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_bSTA_disassociated_from_backhaul_ap(unsigned char *InterfaceId);

/**
 * Get policy config
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
ieee1905_policy_config* ieee1905_get_policy_config();

/**
 * add vendor message tlv
 *
 * @param pmsg		Message to be sent.
 * @param vndr_msg	vendor message to add in pmsg
 *
 * @return      void
 */
void ieee1905_insert_vendor_message_tlv(void *pmsg, ieee1905_vendor_data *vndr_msg);

/**
 * Send m1 for unconfigured radio from wbd
 *
 * @param arg	no argument required
 *
 * @return      void
 */
void ieee1905_start_m1(void);

/**
 * Send Channel Scan Result to Controller from Stored Scan data, for given Channel Scan Request
 *
 * @param chscan_req        Channel Scan Request for which Channel Scan Report to be sent
 *
 * @param in_status_code        Channel Scan Status code, used to indicate Errors like :
 *                                           if radio is too busy, or Request is too soon
 *
 * @return                  status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_requested_stored_channel_scan(ieee1905_chscan_req_msg *chscan_req,
  unsigned char in_status_code);

/** STA has assoc Failed connection
 *
 * @param mac		MAC address of the STA
 * @param status	Status code indicates reason for association or authentication failure
 * @param reason	Reason Code indicating the reason the STA was disassociated or
 *			deauthenticated
 */
int ieee1905_sta_assoc_failed_connection(unsigned char *bssid, unsigned char *mac, uint16 status, uint16 reason);

/** Add the Unsuccessful association Policy for a Device
 *
 * @param unsuccessful_assoc_config		Unsuccessful association policy config
 */
int ieee1905_add_unsuccessful_association_policy(
  ieee1905_unsuccessful_assoc_config_t*unsuccessful_assoc_config);

/**
 * Add the Channel Scan Reporting Policy for a radio. First add all the Channel Scan reporting
 * policy details and call the "ieee1905_send_policy_config" function to send it to a particular
 * neighbor. This function can be called only from controller.
 *
 * @param chscanrpt       Channel Scan report policies for a given radio
 *
 * @return                status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_add_chscan_reporting_policy_for_radio(ieee1905_chscanrpt_config *chscanrpt);

/**
 * Send Association status notification message to controller
 *
 * @param assoc_notif	Info for association status notification per bss.
 * @return	status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_association_status_notification(
	ieee1905_association_status_notification *assoc_notif);

/**
 * Send Tunnel message to controller
 *
 * @param tunnel_msg	Info about source mac, tunneled msg type followed with payload.
 * @return	status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_tunneled_msg(ieee1905_tunnel_msg_t *tunnel_msg);

/**
 * Send CAC request from controller to agents
 *
 * @param pDevice	Pointer to the device to which CAC request is send
 * @param cac_list	CAC request details.
 * @return	status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_cac_request(i5_dm_device_type *pDevice, ieee1905_cac_rqst_list_t *cac_list);

/**
 * Send CAC termination from controller to agents
 *
 * @param pDevice	Pointer to the device to which CAC request is send
 * @param cac_term_list	CAC termination details.
 * @return	status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_cac_termination(i5_dm_device_type *pDevice,
	ieee1905_cac_termination_list_t *cac_term_list);

/**
 * Send Backhaul STA Capability Query message from controller to agent
 *
 * @param pDevice  Pointer to the device to which Backhaul STA Capability Query message to be sent
 *
 * @return        status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_backhaul_sta_capability_query(i5_dm_device_type *pDevice);

/**
 * Find buffer size for the list of all the lan ifnames
 *
 * @param	no arguments required
 * @return	Returns the buffer size
 */
int ieee1905_calc_lanifnames_list_bufsize(void);

/**
 * Get a concatenated list of all the lan ifnames
 *
 * @param	no arguments required
 * @return	Returns the list of all lan ifnames
 */
char *ieee1905_get_all_lanifnames_list(void);

/**
 * Get all lan ifnames list and find if the given interface exist or not
 *
 * @param ifname	Interface name to check if is in any of the lan ifnames
 * @return		found or not. 0 not found 1 found
 */
int ieee1905_find_in_all_lanifnames_list(char const *ifname);

/**
 * Free concatenated list of all the lan ifnames
 *
 * @param	no arguments required
 * @return	void
 */
void ieee1905_free_lanifnames_list(void);

/**
 * Send DPP Chirp TLV in 1905 AutoConfiguration Search
 *
 * @force_send_chirp	Flag to Send DPP Chirp in 1905 AutoConfiguration Search
 * @return        status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_send_controller_search(bool force_send_chirp);

/*
 * Send Higher layer Data to all the registered HLE's for a protocol
 *
 * @protocol		received protocol of HLD
 * @hld_data		Received HLD data
 * @hld_data_len	Received HLD data length
 * @return		status for connection with server. 0 Success, Non-Zero Failure
 */
int
ieee1905SendHLDtoHLE(int protocol, unsigned char *hld_data, unsigned int hld_data_len);

/**
 * Add Service Prioritization Rule Item to list of type i5_serv_prio_rule_t
 *
 * @param serv_prio_rule_list	Dst Service Prioritization Rule TLV List
 * @param rule_id	Service Prioritization Rule Identifier
 * @param rule_operation_flag	Add-Remove Rule Flag
 * @param rule_precedence	Rule Precedence Flag
 * @param rule_output	Rule Output : method to select 802.1Q C-TAG PCP Output Value
 * @param rule_flag	Rule Flag : Always Match
 *
 * @return	New Service Prioritization Rule Item added. Non-NULL Success. NULL Failure
 */
i5_serv_prio_rule_t* ieee1905_add_servprio_rule_to_list(ieee1905_glist_t *serv_prio_rule_list,
  unsigned int rule_id, unsigned char rule_operation_flag,
  unsigned char rule_precedence, unsigned char rule_output, unsigned char rule_flag);

/**
 * Add QoS Management Descriptor Item to list of type i5_qosmgmt_desc_t
 *
 * @param qosmgmt_desc_list	Dst QoS Management Descriptor List
 * @param qm_id	QoS Management Rule Unique Identifier
 * @param bssid	BSSID of BSS for which this descriptor applies
 * @param client_mac	STA MAC for which this descriptor applies
 * @param descriptor	One Of : MSCS / SCS / QoS Management - Descriptor Element
 * @param descriptor_len	Length of the Descriptor Element
 *
 * @return	New QoS Management Descriptor Item added. Non-NULL Success. NULL Failure
 */
i5_qosmgmt_desc_t* ieee1905_add_qosmgmt_desc_to_list(ieee1905_glist_t *qosmgmt_desc_list,
  unsigned short qm_id, unsigned char *bssid, unsigned char *client_mac,
  unsigned char *descriptor, unsigned int descriptor_len);

/**
 * Cleanup Service Prioritization Request Message
 *
 * @param in_sp_req	Dst Service Prioritization Request Message
 *
 * @return      status of the call. 0 Success. Non Zero Failure
 */
int ieee1905_cleanup_serv_prio_req_msg(i5_serv_prio_req_msg_t *in_sp_req);

/**
 * Cleanup QoS Management Notification Message
*
* @param in_qosmgmt_notif     Dst QoS Management Notification Message
*
* @return      status of the call. 0 Success. Non Zero Failure
*/
int ieee1905_cleanup_qosmgmt_notif_msg(i5_qosmgmt_notif_msg_t *in_qosmgmt_notif);

/*
 * Register port and protocol for sending Higher Layer Data to Higher layer application
 * @port		HLD port number to be added
 * @protocol		HLD protocol to be added
 * @return		status for call. 0 Success, Non-Zero Failure
 */
int
ieee1905_add_hld_port_protocol(uint16 port, uint8 protocol);

#endif /* MULTIAP */

/*#ifdef __cplusplus
}
#endif*/ /* __cplusplus */

#endif /* __IEEE1905_H__ */
