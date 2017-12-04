typedef struct wlc_conf {
	char *key_assign;
	char *key_from;
}wlc_conf_t;

const char *conf_ap[] = {
	"sw_mode=3", "lan_proto=dhcp", "lan_dnsenable_x=1", "w_Setting=1",
	NULL
};
const char *conf_rp_2g[] = {
	"sw_mode=2", "wlc_band=0", "lan_proto=dhcp", "lan_dnsenable_x=1", "w_Setting=1",
	NULL
};
const char *conf_rp_5g[] = {
	"sw_mode=2", "wlc_band=1", "lan_proto=dhcp", "lan_dnsenable_x=1", "w_Setting=1",
	NULL
};

const char *hold_ap[] = {
	"wl_ssid", "wl0_ssid", "wl1_ssid", "wl2_ssid",
	"wl_crypto", "wl0_crypto", "wl1_crypto", "wl2_crypto",
	"wl_wpa_psk", "wl0_wpa_psk", "wl1_wpa_psk", "wl2_wpa_psk",
	"wl_auth_mode_x", "wl0_auth_mode_x", "wl1_auth_mode_x", "wl2_auth_mode_x",
#ifdef RTCONFIG_CFGSYNC
	"cfg_group",
#endif
	NULL
};
const char *hold_rp_2g[] = {
	"wl_ssid", "wl0_ssid", "wl1_ssid", "wl2_ssid",
	"wl_crypto", "wl0_crypto", "wl1_crypto", "wl2_crypto",
	"wl_wpa_psk", "wl0_wpa_psk", "wl1_wpa_psk", "wl2_wpa_psk",
	"wl_auth_mode_x", "wl0_auth_mode_x", "wl1_auth_mode_x", "wl2_auth_mode_x",
#ifdef RTCONFIG_CFGSYNC
	"cfg_group",
#endif
	NULL
};
const char *hold_rp_5g[] = {
	"wl_ssid", "wl0_ssid", "wl1_ssid", "wl2_ssid",
	"wl_crypto", "wl0_crypto", "wl1_crypto", "wl2_crypto",
	"wl_wpa_psk", "wl0_wpa_psk", "wl1_wpa_psk", "wl2_wpa_psk",
	"wl_auth_mode_x", "wl0_auth_mode_x", "wl1_auth_mode_x", "wl2_auth_mode_x",
#ifdef RTCONFIG_CFGSYNC
	"cfg_group",
#endif
	NULL
};

const wlc_conf_t wlc_rp_2g[] = {
	{ "wlc_ssid", "wl0_ssid" },
	{ "wlc_wep", "wl0_wep_x" },
	{ "wlc_key", "wl0_key" },
	{ "wlc_auth_mode", "wl0_auth_mode_x" },
	{ "wlc_crypto", "wl0_crypto" },
	{ "wlc_wpa_psk", "wl0_wpa_psk" },
	{ "wlc0_ssid", "wl0_ssid"},
	{ "wlc0_wep", "wl0_wep_x"},
	{ "wlc0_key", "wl0_key"},
	{ "wlc0_auth_mode", "wl0_auth_mode_x"},
	{ "wlc0_crypto", "wl0_crypto"},
	{ "wlc0_wpa_psk", "wl0_wpa_psk"},
	{ "wlc1_ssid", "wl1_ssid"},
	{ "wlc1_wep", "wl1_wep_x"},
	{ "wlc1_key", "wl1_key"},
	{ "wlc1_auth_mode", "wl1_auth_mode_x"},
	{ "wlc1_crypto", "wl1_crypto"},
	{ "wlc1_wpa_psk", "wl1_wpa_psk"},
	{ "wl0.1_ssid", "wl0_ssid" },
	{ "wl0.1_crypto", "wl0_crypto" },
	{ "wl0.1_wpa_psk", "wl0_wpa_psk" },
	{ "wl0.1_auth_mode_x", "wl0_auth_mode_x" },
	{ NULL }
};
const wlc_conf_t wlc_rp_5g[] = {
	{ "wlc_ssid", "wl1_ssid" },
	{ "wlc_wep", "wl1_wep_x" },
	{ "wlc_key", "wl1_key" },
	{ "wlc_auth_mode", "wl1_auth_mode_x" },
	{ "wlc_crypto", "wl1_crypto" },
	{ "wlc_wpa_psk", "wl1_wpa_psk" },
	{ "wlc0_ssid", "wl0_ssid"},
	{ "wlc0_wep", "wl0_wep_x"},
	{ "wlc0_key", "wl0_key"},
	{ "wlc0_auth_mode", "wl0_auth_mode_x"},
	{ "wlc0_crypto", "wl0_crypto"},
	{ "wlc0_wpa_psk", "wl0_wpa_psk"},
	{ "wlc1_ssid", "wl1_ssid"},
	{ "wlc1_wep", "wl1_wep_x"},
	{ "wlc1_key", "wl1_key"},
	{ "wlc1_auth_mode", "wl1_auth_mode_x"},
	{ "wlc1_crypto", "wl1_crypto"},
	{ "wlc1_wpa_psk", "wl1_wpa_psk"},
	{ "wl1.1_ssid", "wl1_ssid" },
	{ "wl1.1_crypto", "wl1_crypto" },
	{ "wl1.1_wpa_psk", "wl1_wpa_psk" },
	{ "wl1.1_auth_mode_x", "wl1_auth_mode_x" },
	{ NULL }
};
const wlc_conf_t wlc_rp_5g2[] = {
	{ "wlc_ssid", "wl2_ssid" },
	{ "wlc_wep", "wl2_wep_x" },
	{ "wlc_key", "wl2_key" },
	{ "wlc_auth_mode", "wl2_auth_mode_x" },
	{ "wlc_crypto", "wl2_crypto" },
	{ "wlc_wpa_psk", "wl2_wpa_psk" },
	{ "wlc0_ssid", "wl0_ssid"},
	{ "wlc0_wep", "wl0_wep_x"},
	{ "wlc0_key", "wl0_key"},
	{ "wlc0_auth_mode", "wl0_auth_mode_x"},
	{ "wlc0_crypto", "wl0_crypto"},
	{ "wlc0_wpa_psk", "wl0_wpa_psk"},
	{ "wlc1_ssid", "wl2_ssid"},
	{ "wlc1_wep", "wl2_wep_x"},
	{ "wlc1_key", "wl2_key"},
	{ "wlc1_auth_mode", "wl2_auth_mode_x"},
	{ "wlc1_crypto", "wl2_crypto"},
	{ "wlc1_wpa_psk", "wl2_wpa_psk"},
	{ "wl2.1_ssid", "wl2_ssid" },
	{ "wl2.1_crypto", "wl2_crypto" },
	{ "wl2.1_wpa_psk", "wl2_wpa_psk" },
	{ "wl2.1_auth_mode_x", "wl2_auth_mode_x" },
	{ NULL }
};
