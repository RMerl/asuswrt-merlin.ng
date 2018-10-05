struct nvram_config {
	char *name;
	char *rev;
};

struct nvram_config timezones_conf[] = {
	{"time_zone_dst" , NULL},
	{"time_zone_x", NULL},
	{"time_zone_dstoff" , NULL},
	{"time_zone", NULL},
	{"ntp_server0", NULL},
	{ NULL, NULL}
};

struct nvram_config guestnetwork_conf[] = {
	{"wl_bss_enabled" , NULL},
	{"wl_ssid" , NULL},
	{"wl_auth_mode_x", NULL},
	{"wl_crypto" , NULL},
	{"wl_wpa_psk", NULL},
	{"wl_expire", NULL},
	{"wl_lanaccess", NULL},
	{"wl_mbss", NULL},
	{"wl_macmode", NULL},
	{"wl_maclist_x", NULL},
	{"wl_bw_enabled", NULL},
	{"qos_enable", NULL},
	{"qos_type", NULL},
	{"wl_bw_dl", NULL},
	{"wl_bw_ul", NULL},
	{ NULL, NULL}
};
