#ifndef __CFG_DWB_H__
#define __CFG_DWB_H__
#include <sysdeps/amas/amas_dwb.h>

#define WL_KEY_LEN              10
#define WL_SSID_RANDOM_LEN      4

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

#endif /* __CFG_DWB_H__ */
/* End of cfg_dwb.h */