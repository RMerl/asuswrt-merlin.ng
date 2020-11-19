#ifndef __AMAS_DWB_H__
#define __AMAS_DWB_H__

#include <stdio.h>
#include <stddef.h>
#include <rtconfig.h>

#define GUEST_WL_INDEX  1
#define DUAL_BAND       2
#define TRI_BAND        3

#define DWB_DISABLED_FROM_CFG   0
#define DWB_ENABLED_FROM_CFG    1
#define DWB_DISABLED_FROM_GUI   2
#define DWB_ENABLED_FROM_GUI    3

#ifdef SMART_CONNECT
extern const char *sc_basic_param[];
extern const char *sc_detailed_param[];

extern void cm_revertSmartConnectParameters(int bandNum);
#endif

extern void dwb_init_settings(void);

struct connect_param_mapping_s {
	char *param;
};

extern struct connect_param_mapping_s connect_param_mapping_list[];

#ifdef RTCONFIG_FRONTHAUL_DWB
extern int fronthaul_DWB_profile_generated(int unit, int subunit);
struct basic_wireless_setting_s {
	char *param;
};
extern struct basic_wireless_setting_s basic_wireless_settings[];
#endif

#endif // __AMAS_DWB_H__
