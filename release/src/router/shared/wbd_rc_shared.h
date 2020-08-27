/*
 * Broadcom Home Gateway Reference Design
 * Broadcom Wi-Fi Blanket shared functions
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 * $Id: wbd_rc_shared.h 764230 2018-05-24 08:40:37Z $
 */

#ifndef _WBD_RC_SHARED_H_
#define _WBD_RC_SHARED_H_

/* RC & NVRAM operation Flags */
#define WBD_FLG_RC_RESTART			0X0010
#define WBD_FLG_REBOOT				0X0020
#define WBD_FLG_NV_COMMIT			0X0040

/* bit masks for WBD weak_sta_policy flags */
#define WBD_WEAK_STA_POLICY_FLAG_RULE		0x00000001	/* logic AND chk */
#define WBD_WEAK_STA_POLICY_FLAG_ACTIVE_STA	0x00000002	/* Active STA */
#define WBD_WEAK_STA_POLICY_FLAG_RSSI		0x00000004	/* RSSI & Hysterisis */
#define WBD_WEAK_STA_POLICY_FLAG_TX_RATE	0x00000008	/* Tx Rate */
#define WBD_WEAK_STA_POLICY_FLAG_TX_FAIL	0x00000010	/* Tx Failure */

/* Rules and Threshold parameters for finding weak clients */
typedef struct wbd_weak_sta_policy {
	int t_idle_rate;	/* data rate threshold to measure STA is idle */
	int t_rssi;		/* STA RSSI threshold */
	int t_hysterisis;	/* RSSI Hysterisis threshold */
	uint32 t_tx_rate;	/* Uplink rate threshold in Mbps */
	int t_tx_failures;	/* Tx failures threshold */
	uint32 flags;		/* extension flags (Rules) type : WBD_WEAK_STA_POLICY_FLAG_XXX */
} wbd_weak_sta_policy_t;

/* Associated STA Metrics, which are used to identify weak client in BSD or Controller
 * It includes both Associated STA Metrics mentioned in Multi_AP Spec & BRCM Vendor data
 */
typedef struct wbd_weak_sta_metrics {
	int idle_rate;		/* data rate to measure STA is idle */
	int rssi;		/* STA RSSI */
	int last_weak_rssi;	/* Last RSSI reported when STA was Weak */
	int hysterisis;		/* RSSI Hysterisis */
	uint32 tx_rate;		/* Uplink rate in Mbps */
	int tx_failures;	/* Tx failures */
} wbd_weak_sta_metrics_t;

/* ----------------------------- WBD shared Routines --------------------------------- */

/* Check if Interface is Virtual Interface, if Disabled, Enable it */
extern uint32 wbd_enable_vif(char *ifname);

/* Check if Interface is DWDS Primary Interface, with mode = STA, Change name1 */
extern int wbd_check_dwds_sta_primif(char *ifname, char *ifname1, int len1);

/* Find First DWDS Primary Interface, with mode = STA */
extern int wbd_find_dwds_sta_primif(char *ifname, int len, char *ifname1, int len1);

/* Get "wbd_ifnames" from "lan_ifnames" */
extern int wbd_ifnames_fm_lan_ifnames(char *wbd_ifnames, int len, char *wbd_ifnames1, int len1);

/* Get next available Virtual Interface Subunit */
extern int wbd_get_next_vif_subunit(int in_unit, int *error);

/* Create & Configure Virtual AP Interface, if WBD is ON and AP is WBD DWDS Slave */
extern int wbd_create_vif(int unit, int subunit);

/* Read "wbd_ifnames" NVRAM and get actual ifnames */
extern int wbd_read_actual_ifnames(char *wbd_ifnames, int len, bool isWBD);

/* Find Number of valid interfaces */
extern int wbd_count_interfaces(void);

#ifdef WLHOSTFBT
/* Get R0KHID from NVRAM, if not present generate and set the R0KHID */
extern char *wbd_get_r0khid(char *prefix, char *r0khid, int r0khid_len, int set_nvram);

/* Get R0KHKEY from NVRAM, if not present generate and set the R0KHKEY */
extern char *wbd_get_r0khkey(char *prefix, char *r0khkey, int r0khkey_len, int set_nvram);

/* Get R1KH_ID from NVRAM, if not present generate and set the R1KH_ID */
extern char* wbd_get_r1khid(char *prefix, char *r1khid, int r1khid_len, int set_nvram);

/* Get MDID from NVRAM, if not present generate and set the mdid */
extern uint16 wbd_get_mdid(char *prefix);

/* Enable FBT */
extern int wbd_enable_fbt(char *prefix);

/* Check whether FBT enabling is possible or not. First it checks for psk2 and then wbd_fbt */
extern int wbd_is_fbt_possible(char *prefix);
#endif /* WLHOSTFBT */

/* Executes nvram commit/rc restart/reboot commands */
extern void wbd_do_rc_restart_reboot(uint32 rc_flags);

/* Find Backhaul Prim Ifr Configured on this Device (RootAP/Repeater), Check if its Dedicated */
extern int wbd_find_backhaul_primif_on_device(char *backhaul_if,
	int backhaul_if_len, int *repeat_backhaul);

/* Common algo to compare STA Stats and Thresholds, & identify if STA is Weak or not */
extern int wbd_weak_sta_identification(struct ether_addr *sta_mac,
	wbd_weak_sta_metrics_t *sta_stats, wbd_weak_sta_policy_t *thresholds,
	int *out_fail_cnt, int *out_weak_flag);

/* ----------------------------- WBD shared Routines --------------------------------- */

#endif /* _WBD_RC_SHARED_H_ */
