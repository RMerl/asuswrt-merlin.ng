/*
 * Forward declarations for commonly used wl driver structs
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: wlc_types.h 831530 2023-10-19 14:53:44Z $
 */

#ifndef _wlc_types_h_
#define _wlc_types_h_
#include <wlioctl.h>

/* Version of WLC interface to be returned as a part of wl_wlc_version structure.
 * WLC_API_VERSION_MINOR is currently not in use.
 */

#define WLC_API_VERSION_MAJOR      9
#define WLC_API_VERSION_MINOR      0

/* forward declarations */
typedef struct wlc_info wlc_info_t;
typedef struct wlc_tx_runtime_context wlc_tx_rtc_t;
typedef struct wlcband wlcband_t;
typedef struct wlc_cmn_info wlc_cmn_info_t;
typedef struct wlc_assoc_info wlc_assoc_info_t;
typedef struct wlc_pm_info wlc_pm_info_t;
typedef struct scb_txc_info scb_txc_info_t;
typedef struct wlc_bsscfg wlc_bsscfg_t;
typedef struct wlc_mbss_info wlc_mbss_info_t;
typedef struct wlc_spt wlc_spt_t;
typedef struct scb scb_t;
typedef struct scb_iter scb_iter_t;
typedef struct vndr_ie_listel vndr_ie_listel_t;
typedef struct wlc_if wlc_if_t;
typedef struct wl_if wl_if_t;
typedef struct led_info led_info_t;
typedef struct bmac_led bmac_led_t;
typedef struct bmac_led_info bmac_led_info_t;
typedef struct seq_cmds_info wlc_seq_cmds_info_t;
typedef struct ota_test_info ota_test_info_t;
typedef struct apps_wlc_psinfo apps_wlc_psinfo_t;
typedef struct scb_module scb_module_t;
typedef struct wlc_frminfo wlc_frminfo_t;
typedef struct wlc_rxhinfo wlc_rxhinfo_t;
typedef struct amsdu_info amsdu_info_t;
typedef struct txq_info txq_info_t;
typedef struct txq txq_t;
typedef struct flow_ctx_info wlc_flow_ctx_info_t;
typedef struct wlc_txq_info wlc_txq_info_t;
typedef struct wlc_cac wlc_cac_t;
typedef struct ampdu_tx_info ampdu_tx_info_t;
typedef struct ampdu_rx_info ampdu_rx_info_t;
typedef struct wlc_ratesel_info wlc_ratesel_info_t;
typedef struct ratesel_info ratesel_info_t;
typedef struct wl_fips_info wl_fips_info_t;
typedef struct wlc_ap_info wlc_ap_info_t;
typedef struct wlc_scan_info wlc_scan_info_t;
typedef struct wlc_scan_cmn_info wlc_scan_cmn_t;
typedef struct wl_bcn_report_cfg wl_bcn_report_cfg_t;
typedef struct dls_info dls_info_t;
typedef struct l2_filter_info l2_filter_info_t;
typedef struct wlc_auth_info wlc_auth_info_t;
typedef struct wlc_sup_info wlc_sup_info_t;
typedef struct wlc_fbt_info wlc_fbt_info_t;
typedef struct wlc_assoc_mgr_info wlc_assoc_mgr_info_t;
typedef struct wlc_psta_info wlc_psta_info_t;
typedef struct wowl_info wowl_info_t;
typedef struct wowlpf_info wowlpf_info_t;
typedef struct wmf_info wmf_info_t;
typedef struct wlc_rrm_info wlc_rrm_info_t;
typedef struct rm_info rm_info_t;

typedef struct wlc_txs_dyntxc_info wlc_txs_dyntxc_info_t;
typedef struct wlc_11h_info wlc_11h_info_t;
typedef struct wlc_tpc_info wlc_tpc_info_t;
typedef struct wlc_csa_info wlc_csa_info_t;
typedef struct wlc_quiet_info wlc_quiet_info_t;
typedef struct cca_info cca_info_t;
typedef struct itfr_info itfr_info_t;

typedef struct wlc_wnm_info wlc_wnm_info_t;
typedef struct wlc_11d_info wlc_11d_info_t;
typedef struct wlc_cntry_info wlc_cntry_info_t;

typedef struct wlc_dfs_info wlc_dfs_info_t;

typedef struct bsscfg_module bsscfg_module_t;

typedef struct wlc_prot_info wlc_prot_info_t;
typedef struct wlc_prot_g_info wlc_prot_g_info_t;
typedef struct wlc_prot_n_info wlc_prot_n_info_t;
typedef struct wlc_prot_obss_info wlc_prot_obss_info_t;
typedef struct wlc_obss_dynbw wlc_obss_dynbw_t;
typedef struct wlc_11u_info wlc_11u_info_t;
typedef struct wlc_probresp_info wlc_probresp_info_t;
typedef struct wlc_wapi_info wlc_wapi_info_t;

typedef struct wlc_tbtt_info wlc_tbtt_info_t;

typedef struct wlc_bssload_info wlc_bssload_info_t;

typedef struct wlc_pcb_info wlc_pcb_info_t;
typedef struct wlc_txc_info wlc_txc_info_t;
typedef struct wlc_cfp_info wlc_cfp_info_t;
typedef struct wlc_sqs_info wlc_sqs_info_t;
typedef struct wlc_spp_info wlc_spp_info_t;

typedef struct wlc_lpc_info wlc_lpc_info_t;
typedef struct lpc_info lpc_info_t;
typedef struct rate_lcb_info rate_lcb_info_t;
typedef struct wlc_txbf_info wlc_txbf_info_t;
typedef struct wlc_mutx_info wlc_mutx_info_t;
typedef struct wlc_murx_info wlc_murx_info_t;

typedef struct wlc_olpc_eng_info_t wlc_olpc_eng_info_t;
/* used by olpc to register for callbacks from stf */
typedef void (*wlc_stf_txchain_evt_notify)(wlc_info_t *wlc);

typedef struct wlc_rfc wlc_rfc_t;
typedef struct wlc_pktc_info wlc_pktc_info_t;

typedef struct wlc_mfp_info wlc_mfp_info_t;

typedef struct wlc_macfltr_info wlc_macfltr_info_t;

typedef struct wlc_nar_info wlc_nar_info_t;
typedef struct wlc_bs_data_info wlc_bs_data_info_t;
typedef struct wlc_bs_data_counters wlc_bs_data_counters_t;

typedef struct wlc_txrx_summary_info wlc_txrx_summary_info_t;
typedef struct wlc_txrx_summary_counters wlc_txrx_summary_counters_t;

typedef struct wlc_keymgmt wlc_keymgmt_t;
typedef struct wlc_key	wlc_key_t;
typedef struct wlc_key_info wlc_key_info_t;

typedef struct wlc_hw wlc_hw_t;
typedef struct wlc_hw_info wlc_hw_info_t;
typedef struct wlc_hwband wlc_hwband_t;
typedef struct wlc_rx_stall_info wlc_rx_stall_info_t;

typedef struct wlc_txs_hist_info wlc_txs_hist_info_t;
typedef struct wlc_txs_hist wlc_txs_hist_t;

typedef struct wlc_dstall_info wlc_dstall_info_t;
typedef struct wlc_dstall_counters wlc_dstall_counters_t;
typedef struct wlc_txstall_fail_counters wlc_txstall_fail_counters_t;
typedef struct wlc_rxstall_fail_counters wlc_rxstall_fail_counters_t;

typedef struct wlc_rmc_info wlc_rmc_info_t;

typedef struct wlc_iem_info wlc_iem_info_t;

typedef struct wlc_ier_info wlc_ier_info_t;
typedef struct wlc_ier_reg wlc_ier_reg_t;

typedef struct wlc_ht_info wlc_ht_info_t;
typedef struct wlc_obss_info wlc_obss_info_t;
typedef struct wlc_vht_info wlc_vht_info_t;
typedef struct wlc_akm_info wlc_akm_info_t;

typedef struct wlc_bss_info wlc_bss_info_t;

typedef struct wlc_multibssid_info wlc_multibssid_info_t;

typedef struct wlc_hs20_info wlc_hs20_info_t;
typedef struct wlc_hs20_cmn wlc_hs20_cmn_t;
typedef struct wlc_pmkid_info	wlc_pmkid_info_t;
typedef struct wlc_btc_info wlc_btc_info_t;

typedef struct wlc_txh_info wlc_txh_info_t;
typedef struct wlc_txs wlc_txs_t;
typedef struct wlc_txs_info wlc_txs_info_t;

typedef struct wlc_stamon_info wlc_stamon_info_t;
typedef struct wlc_monitor_info wlc_monitor_info_t;
#ifdef BCM_PCAP
typedef struct wlc_pcap_info wlc_pcap_info_t;
#endif /* BCM_PCAP */

typedef struct wlc_csimon_info wlc_csimon_info_t;
typedef struct wlc_csimon_sta wlc_csimon_sta_t;

typedef struct wlc_sbf_info wlc_sbf_info_t;
typedef struct wlc_sbfn_info wlc_sbfn_info_t;

typedef struct wlc_wds_info wlc_wds_info_t;
typedef struct okc_info okc_info_t;
typedef struct wlc_stats_info wlc_stats_info_t;

typedef struct wlc_pps_info wlc_pps_info_t;

typedef struct duration_info duration_info_t;

typedef struct wlc_taf_info wlc_taf_info_t;
typedef struct taf_txpkt_stats taf_txpkt_stats_t;

typedef struct wlc_pdsvc_info wlc_pdsvc_info_t;

typedef struct wlc_supp_channels wlc_supp_channels_t;

/* For LTE Coex */
typedef struct wlc_ltecx_info wlc_ltecx_info_t;

typedef struct mws_scanreq_bms mws_scanreq_bms_t;

typedef struct wlc_probresp_mac_filter_info wlc_probresp_mac_filter_info_t;

typedef struct wlc_ltr_info wlc_ltr_info_t;

typedef struct bwte_info bwte_info_t;

typedef struct wlc_modesw_info wlc_modesw_info_t;

typedef struct wlc_pm_mute_tx_info wlc_pm_mute_tx_t;

typedef struct wlc_bcntrim_info wlc_bcntrim_info_t;
typedef wl_bcntrim_cfg_v1_t    wl_bcntrim_cfg_t;
typedef wl_bcntrim_status_query_v1_t    wl_bcntrim_status_query_t;
typedef wl_bcntrim_status_v1_t    wl_bcntrim_status_t;
#define WL_BCNTRIM_STATUS_VERSION WL_BCNTRIM_STATUS_VERSION_1
#define WL_BCNTRIM_CFG_VERSION	WL_BCNTRIM_CFG_VERSION_1

typedef struct wlc_smfs_info wlc_smfs_info_t;
typedef struct wlc_misc_info wlc_misc_info_t;

typedef struct wlc_eventq wlc_eventq_t;
typedef struct wlc_event wlc_event_t;
typedef struct wlc_ulp_info wlc_ulp_info_t;

typedef struct wlc_bsscfg_viel_info wlc_bsscfg_viel_info_t;

typedef struct wlc_lq_info wlc_lq_info_t;
typedef struct chanim_info chanim_info_t;

typedef struct wlc_wlfc_info wlc_wlfc_info_t;

typedef struct wlc_frag_info wlc_frag_info_t;
typedef struct wlc_bss_list wlc_bss_list_t;

typedef struct wlc_msch_info wlc_msch_info_t;
typedef struct wlc_msch_req_handle wlc_msch_req_handle_t;

typedef struct wlc_randmac_info wlc_randmac_info_t;

typedef struct wlc_sta_info wlc_sta_info_t;

typedef struct wlc_act_frame_info wlc_act_frame_info_t;

typedef struct wlc_qos_info wlc_qos_info_t;

typedef struct wlc_assoc wlc_assoc_t;
typedef struct wlc_roam wlc_roam_t;
typedef struct wlc_pm_st wlc_pm_st_t;
typedef struct wlc_wme wlc_wme_t;

typedef struct wlc_link_qual wlc_link_qual_t;

typedef struct wlc_macdbg_info wlc_macdbg_info_t;
typedef struct wlc_cpudbg_xfer_info wlc_cpudbg_xfer_info_t;
typedef struct wlc_rspec_info wlc_rspec_info_t;
typedef struct wlc_ndis_info wlc_ndis_info_t;

typedef struct wlc_join_pref wlc_join_pref_t;

typedef struct wlc_scan_utils wlc_scan_utils_t;
typedef struct wlc_addrmatch_info wlc_addrmatch_info_t;

typedef struct cca_chan_qual cca_chan_qual_t;

typedef struct wlc_perf_utils wlc_perf_utils_t;
typedef struct wlc_test_info wlc_test_info_t;

typedef struct wlc_dump_info wlc_dump_info_t;

typedef struct wlc_stf wlc_stf_info_t;

typedef struct wlc_he_info wlc_he_info_t;
typedef struct wlc_eht_info wlc_eht_info_t;
typedef struct wlc_mlo_info wlc_mlo_info_t;

typedef struct wlc_twt_info wlc_twt_info_t;

typedef struct wlc_muscheduler_info wlc_muscheduler_info_t;

typedef struct wlc_ulmu_info wlc_ulmu_info_t;

typedef struct resv_info resv_info_t;

typedef struct wlc_stf_arb wlc_stf_arb_t;

typedef struct wlc_stf_nss_request_st wlc_stf_nss_request_t;

typedef struct wlc_stf_nss_request_q_st wlc_stf_nss_request_q_t;

typedef struct wlc_mimo_ps_cfg wlc_mimo_ps_cfg_t;

typedef struct wlc_hw_config wlc_hw_config_t;

typedef struct wlc_stf_arb_mps_info wlc_stf_arb_mps_info_t;

typedef struct wlc_mbo_info wlc_mbo_info_t;

typedef struct wlc_oce_info wlc_oce_info_t;

typedef struct wlc_fils_info wlc_fils_info_t;

typedef struct wlc_vasip_info wlc_vasip_info_t;

typedef struct wlc_phydsp_info wlc_phydsp_info_t;

typedef struct wlc_mbo_oce_info wlc_mbo_oce_info_t;

typedef struct wlc_esp_info wlc_esp_info_t;

typedef sta_info_v8_t sta_info_t;

#if defined(WL_BSS_STATS) || defined(WL_EAP_STATS) || defined(BCM_CPEROUTER_EXTSTATS)
typedef wl_if_stats_v2_t wl_if_stats_t;
#define WL_IF_STATS_T_VERSION WL_IF_STATS_VER_2
#else
typedef wl_if_stats_v1_t wl_if_stats_t;
#define WL_IF_STATS_T_VERSION WL_IF_STATS_VER_1
#endif /* WL_BSS_STATS  || WL_EAP_STATS || BCM_CPEROUTER_EXTSTATS */

typedef struct wl_roam_prof_band_v2 wl_roam_prof_band_t;
typedef struct wl_roam_prof_v2 wl_roam_prof_t;

typedef struct wl_dfs_ap_move_status_v2 wl_dfs_ap_move_status_t;

typedef struct wl_utrace_capture_args_v2 wl_utrace_capture_args_t;
typedef struct wl_pmalert_ucode_dbg_v2 wl_pmalert_ucode_dbg_t;

typedef struct wl_proxd_collect_data_v3 wl_proxd_collect_data_t;
typedef struct wl_proxd_collect_event_data_v3 wl_proxd_collect_event_data_t;

typedef struct wlc_chctx_info wlc_chctx_info_t;

typedef struct wlc_pmq_info wlc_pmq_info_t;

typedef struct wlc_ftm_hw_info wlc_ftm_hw_info_t;

/* Inteface version mapping for versioned pfn structures */
#undef PFN_SCANRESULT_VERSION
#define PFN_SCANRESULT_VERSION PFN_SCANRESULT_VERSION_V2
#define PFN_SCANRESULTS_VERSION PFN_SCANRESULTS_VERSION_V2
#define PFN_LBEST_SCAN_RESULT_VERSION PFN_LBEST_SCAN_RESULT_VERSION_V2
typedef wl_pfn_subnet_info_v2_t wl_pfn_subnet_info_t;
typedef wl_pfn_net_info_v2_t wl_pfn_net_info_t;
typedef wl_pfn_lnet_info_v2_t wl_pfn_lnet_info_t;
typedef wl_pfn_lscanresults_v2_t wl_pfn_lscanresults_t;
typedef wl_pfn_scanresults_v2_t wl_pfn_scanresults_t;
typedef wl_pfn_scanresult_v2_1_t wl_pfn_scanresult_t;

#define WL_INTERFACE_CREATE_VER		WL_INTERFACE_CREATE_VER_3
typedef wl_interface_create_v3_t wl_interface_create_t;

#define WL_INTERFACE_INFO_VER		WL_INTERFACE_INFO_VER_2
typedef wl_interface_info_v2_t wl_interface_info_t;

#define WL_PKTENG_RU_FILL_CURRENT_VER	WL_PKTENG_RU_FILL_VER_1

#define BTCX_STATS_VER BTCX_STATS_VER_3
typedef wlc_btc_stats_v3_t wlc_btc_stats_t;

typedef struct wlc_ratelinkmem_info wlc_ratelinkmem_info_t;
typedef union d11linkmem_entry d11linkmem_entry_t;

/* proxd typedefs */
#ifdef FTM
typedef wl_proxd_params_tof_tune_v4_t wl_proxd_params_tof_tune_t;
typedef wl_proxd_params_iovar_v3_t wl_proxd_params_iovar_ext_t;
typedef wl_proxd_collect_header_v3_t wl_proxd_collect_header_ext_t;
#else
typedef wl_proxd_params_tof_tune_v4_t wl_proxd_params_tof_tune_t;
typedef wl_proxd_params_iovar_v2_t wl_proxd_params_iovar_ext_t;
typedef wl_proxd_collect_header_v2_t wl_proxd_collect_header_ext_t;
#endif /* FTM */

/* ranging context */
typedef struct wlc_ftm_ranging_ctx wlc_ftm_ranging_ctx_t;

#define AWD_DATA_VERSION AWD_DATA_VERSION_V1
typedef awd_data_v1_t awd_data_t;
typedef awd_tag_data_v1_t awd_tag_data_t;
typedef join_classification_info_v1_t join_classification_info_t;
typedef join_target_classification_info_v1_t join_target_classification_info_t;
typedef join_assoc_state_v1_t join_assoc_state_t;
typedef join_channel_v1_t join_channel_t;
typedef join_total_attempts_num_v1_t join_total_attempts_num_t;

typedef rmc_bss_info_v1_t rmc_bss_info_t;
typedef rmc_candidate_info_v1_t rmc_candidate_info_t;

typedef struct chanswitch_hist chanswitch_hist_t;
typedef struct wlc_chansw_info wlc_chansw_info_t;

typedef struct wl_proxd_rtt_sample_v2 wl_proxd_rtt_sample_t;
typedef struct wl_proxd_rtt_result_v2 wl_proxd_rtt_result_t;

#ifdef FTM
#define WL_FTM_TLV_ID_COUNTERS WL_FTM_TLV_ID_COUNTERS_V2
typedef wl_ftm_counters_v2_t wl_ftm_counters_t;

/* wl_proxd_iov_t, wl_proxd_event_t, wl_ftm_iov_t, wl_ftm_event_t version */
#define WL_PROXD_API_VERSION WL_PROXD_API_VERSION_3

/* wl_proxd_iov_t, wl_proxd_event_t, wl_ftm_iov_t, wl_ftm_event_t minimum version */
#define WL_PROXD_API_MIN_VERSION WL_PROXD_API_MIN_VERSION_3
#endif /* FTM */

#ifdef RATESET_VERSION_ENABLED
/* all rateset_args structures and version updates will come here */
#ifdef WL11BE
#define RATESET_ARGS_VERSION    (RATESET_ARGS_V3)
typedef wl_rateset_args_v3_t wl_rateset_args_t;
#else
#define RATESET_ARGS_VERSION	(RATESET_ARGS_V2)
typedef wl_rateset_args_v2_t wl_rateset_args_t;
#endif /* WL11BE */
#endif /* RATESET_VERSION_ENABLED */

#ifdef WL_BSS_INFO_TYPEDEF_HAS_ALIAS
typedef wl_bss_info_v109_1_t wl_bss_info_t;

typedef wl_gscan_bss_info_v3_t wl_gscan_bss_info_t;
#define WL_GSCAN_INFO_FIXED_FIELD_SIZE   (OFFSETOF(wl_gscan_bss_info_t, info))

typedef wl_scan_results_v2_t wl_scan_results_t;
/** size of wl_scan_results not including variable length array */
#define WL_SCAN_RESULTS_FIXED_SIZE (OFFSETOF(wl_scan_results_t, bss_info))

typedef wl_escan_result_v2_t wl_escan_result_t;
#define WL_ESCAN_RESULTS_FIXED_SIZE (OFFSETOF(wl_escan_result_t, bss_info))

typedef wl_iscan_results_v2_t wl_iscan_results_t;
/** size of wl_iscan_results not including variable length array */
#define WL_ISCAN_RESULTS_FIXED_SIZE \
	(WL_SCAN_RESULTS_FIXED_SIZE + OFFSETOF(wl_iscan_results_t, results))

typedef wl_gscan_result_v2_1_t wl_gscan_result_t;
#define WL_GSCAN_RESULTS_FIXED_SIZE (OFFSETOF(wl_gscan_result_t, bss_info))
#endif /* WL_BSS_INFO_TYPEDEF_HAS_ALIAS */

typedef uint8 wlc_mbsp_sel_t;
typedef uint8 dot11_mbsp_sel_t;

typedef struct airiq_info airiq_info_t;
typedef struct wlc_rx_report_info wlc_rx_report_info_t;

typedef struct wl_gas_info wl_gas_info_t;
typedef struct wl_eventq_info wl_eventq_info_t;

typedef struct bme_info_s bme_info_t;
typedef struct wlc_offload wlc_offload_t;

typedef struct wlc_sts_xfer_info wlc_sts_xfer_info_t;

typedef struct wlc_deauth_send_cbargs wlc_deauth_send_cbargs_t;
typedef struct wlc_macreq_info wlc_macreq_info_t;
typedef struct wlc_txcfg_info wlc_txcfg_info_t;
typedef struct wlc_fifo_info wlc_fifo_info_t;
typedef struct puqmgr puqmgr_t;

typedef struct wlc_dtpc_info wlc_dtpc_info_t;

typedef struct taf_scheduler_public taf_scheduler_public_t;

typedef struct wlc_rlm_rate_store wlc_rlm_rate_store_t;
typedef struct ratesel_txparams ratesel_txparams_t;
typedef wl_mlo_tid_map_neg_v1_t         wl_mlo_tid_map_neg_t;
typedef wl_mlo_info_v1_t		wl_mlo_info_basic_t;
typedef wl_mlo_scb_info_v1_t		wl_mlo_scb_info_t;

typedef struct wlc_qosmgmt_info wlc_qosmgmt_info_t;
typedef struct wlc_lsa_info wlc_lsa_info_t;

#define EVENT_LOG_SET_TYPE_CURRENT_VERSION EVENT_LOG_SET_TYPE_VERSION_0
typedef wl_el_set_type_v1_t wl_el_set_types_t;
#define EVENT_LOG_SET_TYPE_ALL_CURRENT_VERSION EVENT_LOG_SET_TYPE_ALL_V1
typedef wl_el_set_all_type_v1_t wl_el_set_all_type_t;

/* indices to use on the bandstate array (wlc->bandstate[index]) */
enum wlc_bandunit {
	BAND_2G_INDEX =	0,
	BAND_5G_INDEX,
	BAND_6G_INDEX,
	MAXBANDS
};

typedef struct wlc_bcn_prot_info wlc_bcn_prot_info_t;

typedef struct tx_flow_data tx_flow_data_t;
typedef struct rx_flow_data rx_flow_data_t;
#endif	/* _wlc_types_h_ */
