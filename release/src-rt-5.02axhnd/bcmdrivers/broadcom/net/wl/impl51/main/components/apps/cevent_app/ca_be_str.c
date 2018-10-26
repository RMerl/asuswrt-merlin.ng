/*
 * Auto-generated using ./gen_be_str.sh
 *
 *
 * $Id:$
 */

/* string array of bcm_event_t types defined in bcmevent.h */
char *ca_bcm_event_str[185 + 1] = {
	/* fillers from 0 to 185 */
	"E_0",
	"E_1",
	"E_2",
	"E_3",
	"E_4",
	"E_5",
	"E_6",
	"E_7",
	"E_8",
	"E_9",
	"E_10",
	"E_11",
	"E_12",
	"E_13",
	"E_14",
	"E_15",
	"E_16",
	"E_17",
	"E_18",
	"E_19",
	"E_20",
	"E_21",
	"E_22",
	"E_23",
	"E_24",
	"E_25",
	"E_26",
	"E_27",
	"E_28",
	"E_29",
	"E_30",
	"E_31",
	"E_32",
	"E_33",
	"E_34",
	"E_35",
	"E_36",
	"E_37",
	"E_38",
	"E_39",
	"E_40",
	"E_41",
	"E_42",
	"E_43",
	"E_44",
	"E_45",
	"E_46",
	"E_47",
	"E_48",
	"E_49",
	"E_50",
	"E_51",
	"E_52",
	"E_53",
	"E_54",
	"E_55",
	"E_56",
	"E_57",
	"E_58",
	"E_59",
	"E_60",
	"E_61",
	"E_62",
	"E_63",
	"E_64",
	"E_65",
	"E_66",
	"E_67",
	"E_68",
	"E_69",
	"E_70",
	"E_71",
	"E_72",
	"E_73",
	"E_74",
	"E_75",
	"E_76",
	"E_77",
	"E_78",
	"E_79",
	"E_80",
	"E_81",
	"E_82",
	"E_83",
	"E_84",
	"E_85",
	"E_86",
	"E_87",
	"E_88",
	"E_89",
	"E_90",
	"E_91",
	"E_92",
	"E_93",
	"E_94",
	"E_95",
	"E_96",
	"E_97",
	"E_98",
	"E_99",
	"E_100",
	"E_101",
	"E_102",
	"E_103",
	"E_104",
	"E_105",
	"E_106",
	"E_107",
	"E_108",
	"E_109",
	"E_110",
	"E_111",
	"E_112",
	"E_113",
	"E_114",
	"E_115",
	"E_116",
	"E_117",
	"E_118",
	"E_119",
	"E_120",
	"E_121",
	"E_122",
	"E_123",
	"E_124",
	"E_125",
	"E_126",
	"E_127",
	"E_128",
	"E_129",
	"E_130",
	"E_131",
	"E_132",
	"E_133",
	"E_134",
	"E_135",
	"E_136",
	"E_137",
	"E_138",
	"E_139",
	"E_140",
	"E_141",
	"E_142",
	"E_143",
	"E_144",
	"E_145",
	"E_146",
	"E_147",
	"E_148",
	"E_149",
	"E_150",
	"E_151",
	"E_152",
	"E_153",
	"E_154",
	"E_155",
	"E_156",
	"E_157",
	"E_158",
	"E_159",
	"E_160",
	"E_161",
	"E_162",
	"E_163",
	"E_164",
	"E_165",
	"E_166",
	"E_167",
	"E_168",
	"E_169",
	"E_170",
	"E_171",
	"E_172",
	"E_173",
	"E_174",
	"E_175",
	"E_176",
	"E_177",
	"E_178",
	"E_179",
	"E_180",
	"E_181",
	"E_182",
	"E_183",
	"E_184",
	"E_185",
	/* explicit overrides for available defines (has gaps) */
	[0] =	"E_SET_SSID",		/* indicates status of set SSID */
	[1] =	"E_JOIN",		/* differentiates join IBSS from found (WLC_E_START) IBSS */
	[2] =	"E_START",		/* STA founded an IBSS or AP started a BSS */
	[3] =	"E_AUTH",		/* 802.11 AUTH request */
	[4] =	"E_AUTH_IND",		/* 802.11 AUTH indication */
	[5] =	"E_DEAUTH",		/* 802.11 DEAUTH request */
	[6] =	"E_DEAUTH_IND",		/* 802.11 DEAUTH indication */
	[7] =	"E_ASSOC",		/* 802.11 ASSOC request */
	[8] =	"E_ASSOC_IND",		/* 802.11 ASSOC indication */
	[9] =	"E_REASSOC",		/* 802.11 REASSOC request */
	[10] =	"E_REASSOC_IND",	/* 802.11 REASSOC indication */
	[11] =	"E_DISASSOC",		/* 802.11 DISASSOC request */
	[12] =	"E_DISASSOC_IND",	/* 802.11 DISASSOC indication */
	[13] =	"E_QUIET_START",	/* 802.11h Quiet period started */
	[14] =	"E_QUIET_END",		/* 802.11h Quiet period ended */
	[15] =	"E_BEACON_RX",		/* BEACONS received/lost indication */
	[16] =	"E_LINK",		/* generic link indication */
	[17] =	"E_MIC_ERROR",		/* TKIP MIC error occurred */
	[18] =	"E_NDIS_LINK",		/* NDIS style link indication */
	[19] =	"E_ROAM",		/* roam complete: indicate status & reason */
	[20] =	"E_TXFAIL",		/* change in dot11FailedCount (txfail) */
	[21] =	"E_PMKID_CACHE",	/* WPA2 pmkid cache indication */
	[22] =	"E_RETROGRADE_TSF",	/* current AP's TSF value went backward */
	[23] =	"E_PRUNE",		/* AP was pruned from join list for reason */
	[24] =	"E_AUTOAUTH",		/* report AutoAuth table entry match for join attempt */
	[25] =	"E_EAPOL_MSG",		/* Event encapsulating an EAPOL message */
	[26] =	"E_SCAN_COMPLETE",	/* Scan results are ready or scan was aborted */
	[27] =	"E_ADDTS_IND",		/* indicate to host addts fail/success */
	[28] =	"E_DELTS_IND",		/* indicate to host delts fail/success */
	[29] =	"E_BCNSENT_IND",	/* indicate to host of beacon transmit */
	[30] =	"E_BCNRX_MSG",		/* Send the received beacon up to the host */
	[31] =	"E_BCNLOST_MSG",	/* indicate to host loss of beacon */
	[32] =	"E_ROAM_PREP",		/* before attempting to roam association */
	[33] =	"E_PFN_NET_FOUND",	/* PFN network found event */
	[34] =	"E_PFN_NET_LOST",	/* PFN network lost event */
	[35] =	"E_RESET_COMPLETE",	/* */
	[36] =	"E_JOIN_START",		/* */
	[37] =	"E_ROAM_START",		/* roam attempt started: indicate reason */
	[38] =	"E_ASSOC_START",	/* */
	[39] =	"E_IBSS_ASSOC",		/* */
	[40] =	"E_RADIO",		/* */
	[41] =	"E_PSM_WATCHDOG",	/* PSM microcode watchdog fired */
	[44] =	"E_PROBREQ_MSG",	/* probe request received */
	[45] =	"E_SCAN_CONFIRM_IND",	/* */
	[46] =	"E_PSK_SUP",		/* WPA Handshake fail */
	[47] =	"E_COUNTRY_CODE_CHANGED", /* */
	[48] =	"E_EXCEEDED_MEDIUM_TIME", /* WMMAC excedded medium time */
	[49] =	"E_ICV_ERROR",		/* WEP ICV error occurred */
	[50] =	"E_UNICAST_DECODE_ERROR", /* Unsupported unicast encrypted frame */
	[51] =	"E_MULTICAST_DECODE_ERROR", /* Unsupported multicast encrypted frame */
	[52] =	"E_TRACE",		/* */
	[54] =	"E_IF",			/* I/F change (for dongle host notification) */
	[55] =	"E_P2P_DISC_LISTEN_COMPLETE", /* listen state expires */
	[56] =	"E_RSSI",		/* indicate RSSI change based on configured levels */
	[57] =	"E_PFN_BEST_BATCHING",	/* PFN best network batching event */
	[58] =	"E_EXTLOG_MSG",		/* */
	[59] =	"E_ACTION_FRAME",	/* Action frame Rx */
	[60] =	"E_ACTION_FRAME_COMPLETE", /* Action frame Tx complete */
	[61] =	"E_PRE_ASSOC_IND",	/* assoc request received */
	[62] =	"E_PRE_REASSOC_IND",	/* re-assoc request received */
	[63] =	"E_CHANNEL_ADOPTED",	/* */
	[64] =	"E_AP_STARTED",		/* AP started */
	[65] =	"E_DFS_AP_STOP",	/* AP stopped due to DFS */
	[66] =	"E_DFS_AP_RESUME",	/* AP resumed due to DFS */
	[67] =	"E_WAI_STA_EVENT",	/* WAI stations event */
	[68] =	"E_WAI_MSG",		/* event encapsulating an WAI message */
	[69] =	"E_ESCAN_RESULT",	/* escan result event */
	[70] =	"E_ACTION_FRAME_OFF_CHAN_COMPLETE", /* action frame off channel complete */
	[71] =	"E_PROBRESP_MSG",	/* probe response received */
	[72] =	"E_P2P_PROBREQ_MSG",	/* P2P Probe request received */
	[73] =	"E_DCS_REQUEST",	/* */
	[74] =	"E_FIFO_CREDIT_MAP",	/* credits for D11 FIFOs. [AC0,AC1,AC2,AC3,BC_MC,ATIM] */
	[75] =	"E_ACTION_FRAME_RX",	/* */
	[76] =	"E_WAKE_EVENT",		/* Wake Event timer fired, used for wake WLAN test mode */
	[77] =	"E_RM_COMPLETE",	/* Radio measurement complete */
	[78] =	"E_HTSFSYNC",		/* Synchronize TSF with the host */
	[79] =	"E_OVERLAY_REQ",	/* request an overlay IOCTL/iovar from the host */
	[80] =	"E_CSA_COMPLETE_IND",	/* 802.11 CHANNEL SWITCH ACTION completed */
	[81] =	"E_EXCESS_PM_WAKE_EVENT", /* excess PM Wake Event to inform host  */
	[82] =	"E_PFN_SCAN_NONE",	/* no PFN networks around */
	[82] =	"E_PFN_BSSID_NET_FOUND", /* */
	[83] =	"E_PFN_SCAN_ALLGONE",	/* last found PFN network gets lost */
	[83] =	"E_PFN_BSSID_NET_LOST",	/* */
	[84] =	"E_GTK_PLUMBED",	/* */
	[85] =	"E_ASSOC_IND_NDIS",	/* 802.11 ASSOC indication for NDIS only */
	[86] =	"E_REASSOC_IND_NDIS",	/* 802.11 REASSOC indication for NDIS only */
	[87] =	"E_ASSOC_REQ_IE",	/* */
	[88] =	"E_ASSOC_RESP_IE",	/* */
	[89] =	"E_ASSOC_RECREATED",	/* association recreated on resume */
	[90] =	"E_ACTION_FRAME_RX_NDIS", /* rx action frame event for NDIS only */
	[91] =	"E_AUTH_REQ",		/* authentication request received */
	[92] =	"E_TDLS_PEER_EVENT",	/* discovered peer, connected/disconnected peer */
	[93] =	"E_SPEEDY_RECREATE_FAIL", /* fast assoc recreation failed */
	[94] =	"E_NATIVE",		/* port-specific event and payload (e.g. NDIS) */
	[95] =	"E_PKTDELAY_IND",	/* event for tx pkt delay suddently jump */
	[96] =	"E_AWDL_AW",		/* AWDL AW period starts */
	[97] =	"E_AWDL_ROLE",		/* AWDL Master/Slave/NE master role event */
	[98] =	"E_AWDL_EVENT",		/* Generic AWDL event */
	[99] =	"E_PSTA_PRIMARY_INTF_IND", /* psta primary interface indication */
	[100] =	"E_NAN",		/* NAN event - Reserved for future */
	[101] =	"E_BEACON_FRAME_RX",	/* */
	[102] =	"E_SERVICE_FOUND",	/* desired service found */
	[103] =	"E_GAS_FRAGMENT_RX",	/* GAS fragment received */
	[104] =	"E_GAS_COMPLETE",	/* GAS sessions all complete */
	[105] =	"E_P2PO_ADD_DEVICE",	/* New device found by p2p offload */
	[106] =	"E_P2PO_DEL_DEVICE",	/* device has been removed by p2p offload */
	[107] =	"E_WNM_STA_SLEEP",	/* WNM event to notify STA enter sleep mode */
	[108] =	"E_TXFAIL_THRESH",	/* */
	[109] =	"E_PROXD",		/* Proximity Detection event */
	[110] =	"E_IBSS_COALESCE",	/* IBSS Coalescing */
	[110] =	"E_AIBSS_TXFAIL",	/* TXFAIL event for AIBSS, re using event 110 */
	[114] =	"E_BSS_LOAD",		/* Inform host of beacon bss load */
	[115] =	"E_MIMO_PWR_SAVE",	/* Inform host MIMO PWR SAVE learning events */
	[116] =	"E_LEAKY_AP_STATS",	/* Inform host leaky Ap stats events */
	[117] =	"E_ALLOW_CREDIT_BORROW", /* Allow or disallow wlfc credit borrowing in DHD */
	[120] =	"E_MSCH",		/* Multiple channel scheduler event */
	[121] =	"E_CSA_START_IND",	/* */
	[122] =	"E_CSA_DONE_IND",	/* */
	[123] =	"E_CSA_FAILURE_IND",	/* */
	[124] =	"E_CCA_CHAN_QUAL",	/* CCA based channel quality report */
	[125] =	"E_BSSID",		/* to report change in BSSID while roaming */
	[126] =	"E_TX_STAT_ERROR",	/* tx error indication */
	[127] =	"E_BCMC_CREDIT_SUPPORT", /* credit check for BCMC supported */
	[128] =	"E_PEER_TIMEOUT",	/* silently drop a STA because of inactivity */
	[130] =	"E_BT_WIFI_HANDOVER_REQ", /* Handover Request Initiated */
	[131] =	"E_SPW_TXINHIBIT",	/* Southpaw TxInhibit notification */
	[132] =	"E_FBT_AUTH_REQ_IND",	/* FBT Authentication Request Indication */
	[133] =	"E_RSSI_LQM",		/* Enhancement addition for WLC_E_RSSI */
	[134] =	"E_PFN_GSCAN_FULL_RESULT", /* Full probe/beacon (IEs etc) results */
	[135] =	"E_PFN_SWC",		/* Significant change in rssi of bssids being tracked */
	[136] =	"E_AUTHORIZED",		/* a STA been authroized for traffic */
	[137] =	"E_PROBREQ_MSG_RX",	/* probe req with wl_event_rx_frame_data_t header */
	[138] =	"E_PFN_SCAN_COMPLETE",	/* PFN completed scan of network list */
	[139] =	"E_RMC_EVENT",		/* RMC Event */
	[140] =	"E_DPSTA_INTF_IND",	/* DPSTA interface indication */
	[141] =	"E_RRM",		/* RRM Event */
	[142] =	"E_PFN_SSID_EXT",	/* SSID EXT event */
	[143] =	"E_ROAM_EXP_EVENT",	/* Expanded roam event */
	[146] =	"E_ULP",		/* ULP entered indication */
	[147] =	"E_MACDBG",		/* Ucode debugging event */
	[148] =	"E_RESERVED",		/* reserved */
	[149] =	"E_PRE_ASSOC_RSEP_IND",	/* assoc resp received */
	[150] =	"E_PSK_AUTH",		/* PSK AUTH WPA2-PSK 4 WAY Handshake failure */
	[151] =	"E_TKO",		/* TCP keepalive offload */
	[152] =	"E_SDB_TRANSITION",	/* SDB mode-switch event */
	[153] =	"E_NATOE_NFCT",		/* natoe event */
	[154] =	"E_TEMP_THROTTLE",	/* Temperature throttling control event */
	[155] =	"E_LINK_QUALITY",	/* Link quality measurement complete */
	[156] =	"E_BSSTRANS_RESP",	/* BSS Transition Response received */
	[157] =	"E_TWT_SETUP",		/* TWT Setup Complete event */
	[157] =	"E_HE_TWT_SETUP",	/* TODO:Remove after merging TWT changes to trunk */
	[158] =	"E_NAN_CRITICAL",	/* NAN Critical Event */
	[159] =	"E_NAN_NON_CRITICAL",	/* NAN Non-Critical Event */
	[160] =	"E_RADAR_DETECTED",	/* Radar Detected event */
	[161] =	"E_RANGING_EVENT",	/* Ranging event */
	[162] =	"E_INVALID_IE",		/* Received invalid IE */
	[163] =	"E_MODE_SWITCH",	/* Mode switch event */
	[164] =	"E_PKT_FILTER",		/* Packet filter event */
	[165] =	"E_DMA_TXFLUSH_COMPLETE", /* */
	[166] =	"E_FBT",		/* FBT event */
	[167] =	"E_PFN_SCAN_BACKOFF",	/* PFN SCAN Backoff event */
	[168] =	"E_PFN_BSSID_SCAN_BACKOFF", /* PFN BSSID SCAN BAckoff event */
	[169] =	"E_AGGR_EVENT",		/* Aggregated event */
	[170] =	"E_AP_CHAN_CHANGE",	/* AP channe change event propagate to use */
	[171] =	"E_PSTA_CREATE_IND",	/* Indication for PSTA creation */
	[172] =	"E_DFS_HIT",		/* Found radar on channel */
	[173] =	"E_FRAME_FIRST_RX",	/* RX pkt from STA */
	[174] =	"E_BCN_STUCK",		/* Beacon Stuck */
	[175] =	"E_PROBSUP_IND",	/* */
	[176] =	"E_SAS_RSSI",		/* WL_EAP_SAS. An AI's RSSI has notable change */
	[177] =	"E_RATE_CHANGE",	/* WL_EAP_AP. A client's rate has changed. */
	[178] =	"E_AMT_CHANGE",		/* WL_EAP_AP. An AMT entry has changed. */
	[179] =	"E_LTE_U_EVENT",	/* LTE_U driver event */
	[180] =	"E_CEVENT",		/* connectivity event logging */
	[181] =	"E_HWA_EVENT",		/* HWA event */
	[182] =	"E_AIRIQ_EVENT",	/* AIRIQ driver event */
	[183] =	"E_TXFAIL_TRFTHOLD",	/* Indication of MAC tx failures */
	[184] =	"E_CAC_STATE_CHANGE",	/* Indication of CAC Status change */
	[185] =	"E_LAST",		/* */
};
