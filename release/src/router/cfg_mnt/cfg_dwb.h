#ifndef __CFG_DWB_H__
#define __CFG_DWB_H__
#include <sysdeps/amas/amas_dwb.h>

#define WL_KEY_LEN              10
#define WL_SSID_RANDOM_LEN      4

#ifdef RTCONFIG_MLO
#define mlo_cap_dwb_subunit 4
#define mlo_re_dwb_subunit 5
#endif

extern int cm_dwbIsEnabled();

struct convert_wlc_mapping_s {
	char *name;
	char *converted_name;
};

static const struct convert_wlc_mapping_s convert_wlc_mapping_list[] = {
	{ "auth_mode_x",	"auth_mode" },
	{ "wep_x",		"wep" },
	{ NULL, 		NULL }
};

#ifndef IsNULL_PTR
#define IsNULL_PTR(__PTR) ((__PTR == NULL))
#endif  /* !IsNULL_PTR */

#ifdef RTCONFIG_MLO
extern int cm_check_mlo_dwb_profile(int dwb_subunit,int profile_type);
#endif
#endif /* __CFG_DWB_H__ */
/* End of cfg_dwb.h */
