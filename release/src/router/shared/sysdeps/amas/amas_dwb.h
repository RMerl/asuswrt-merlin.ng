#ifndef __AMAS_DWB_H__
#define __AMAS_DWB_H__

#include <stdio.h>
#include <stddef.h>

#define GUEST_WL_INDEX  1
#define DUAL_BAND       2
#define TRI_BAND        3

#define DWB_DISABLED_FROM_CFG   0
#define DWB_ENABLED_FROM_CFG    1
#define DWB_DISABLED_FROM_GUI   2
#define DWB_ENABLED_FROM_GUI    3

#ifdef SMART_CONNECT

static const char *sc_detailed_param[] = { "bsd_steering_policy", "bsd_sta_select_policy", "bsd_if_select_policy",
       "bsd_if_qualify_policy"};
static const char *sc_basic_param[] = { "smart_connect_x", "bsd_ifnames", "bsd_bounce_detect"};

void cm_revertSmartConnectParameters(int bandNum);
#endif

void dwb_init_settings(void);

struct connect_param_mapping_s {
	char *param;
};

static const struct connect_param_mapping_s connect_param_mapping_list[] = {
    { "ssid" },
    { "bss_enabled" },
    { "wpa_psk" },
    { "auth_mode_x" },
    { "crypto" },
    { "mbss" },
    { "closed" },
    { "wep_x" },
    { "key" },
    { "key1" },
    { "key2" },
    { "key3" },
    { "key4" },
    { NULL}
};

#endif // __AMAS_DWB_H__