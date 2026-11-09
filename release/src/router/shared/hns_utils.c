#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <shared.h>

#if defined(RTCONFIG_HNS)
int check_hns_switch()
{
	if ((HNS_FULL & HNS_MALS) == 0 && (HNS_FULL & HNS_VP) == 0 && (HNS_FULL & HNS_CC) == 0)
		return 0;
	else
		return 1;
}

int check_hns_setting()
{
	int enabled = 1;

	if (((HNS_FULL & HNS_MALS) == 0 && (HNS_FULL & HNS_VP) == 0 && (HNS_FULL & HNS_CC) == 0) &&
		WEB_FILTER == 0 &&
		WEB_HISTORY == 0
		)
		enabled = 0;

	return enabled;
}

const char *hns_feature_name[] = {
	"dpi_mals",
	"dpi_vp",
	"dpi_cc",
	"webs_filter",
	"web_history",
	"adaptive_qos"
};

struct hnsSupport_t s_hnsSupport_tuple[] =
{
	// {model name, dpi_mals, dpi_vp, dpi_cc, webs_filter, web_history, adaptive_qos}
	// this table is auto-generated from table and shell script

#ifdef BT6
	{"ZenWiFi_BT6"     , {1, 1, 1, 1, 1, 1}},
#endif

#ifdef BT8
	{"ZenWiFi_BT8"     , {1, 1, 1, 1, 1, 1}},
	{"RT-BE14000"      , {1, 1, 1, 1, 1, 1}},
	{"ZenWiFi_BE14000" , {1, 1, 1, 1, 1, 1}},
#endif

#ifdef BT7
	{"ZenWiFi_BT7"     , {1, 1, 0, 0, 1, 1}},
#endif

#ifdef BT8P
	{"ZenWiFi_BT8P"    , {1, 1, 1, 1, 1, 1}},
#endif

#ifdef GS7
	{"GS7"             , {1, 1, 1, 0, 1, 1}},
	{"ROG_GS7"         , {1, 1, 1, 0, 1, 1}},
	{"GS-BE7200"       , {1, 1, 1, 0, 1, 1}},
#endif

#ifdef GSBE7200X
	{"GS-BE7200X"      , {1, 1, 1, 0, 1, 1}},
#endif

#ifdef RTBE58_GO
	{"RT-BE58_GO"      , {0, 1, 0, 0, 0, 1}},
	{"RT-BE58_Go"      , {0, 1, 0, 0, 0, 1}},
	{"RT-BE3600_Go"    , {0, 1, 0, 0, 0, 1}},
#endif

#ifdef RTBE58U_V2
	{"RT-BE58U_V2"     , {1, 1, 1, 0, 1, 1}},
	{"RT-BE58U"        , {1, 1, 1, 0, 1, 1}},
	{"RT-BE3600"       , {1, 1, 1, 0, 1, 1}},
#endif

#ifdef TUFBE3600_V2
	{"TUF-BE3600_V2"   , {1, 1, 1, 0, 1, 1}},
	{"TUF-BE3600"      , {1, 1, 1, 0, 1, 1}},
	{"TUF_3600"        , {1, 1, 1, 0, 1, 1}},
	{"TUF_3600_V2"     , {1, 1, 1, 0, 1, 1}},
#endif

#ifdef RTBE55
	{"RT-BE55"         , {1, 1, 1, 0, 1, 1}},
#endif

	// The END
	{NULL, {0}},
};

static int hns_get_feature_index(const char *name)
{
	for (int i = 0; i < HNS_FEATURE_MAX; i++) {
		if (!strcmp(name, hns_feature_name[i])) return i;
	}
	return -1;
}

int HNSisSupport(const char *name)
{
	int feature_idx = hns_get_feature_index(name);
	if (feature_idx < 0) return 0;

	char *odmpid = get_productid();
	if (!odmpid) return 0;

	for (struct hnsSupport_t *p = s_hnsSupport_tuple; p->model != NULL; p++) {
		if (!strcmp(p->model, odmpid)) return p->feature[feature_idx];
	}

	return 0;
}

void HNS_disable_fun_bit()
{
	int idx_dpi_mals    = hns_get_feature_index("dpi_mals");
	int idx_dpi_cc      = hns_get_feature_index("dpi_cc");
	int idx_webs_filter = hns_get_feature_index("webs_filter");
	int idx_web_history = hns_get_feature_index("web_history");

	if (idx_dpi_mals < 0 || idx_dpi_cc < 0 || idx_webs_filter < 0 || idx_web_history < 0) return;

	char *odmpid = get_productid();
	if (!odmpid) return;

	for (struct hnsSupport_t *p = s_hnsSupport_tuple; p->model != NULL; p++) {
		if (!strcmp(p->model, odmpid)) {
			if (p->feature[idx_dpi_mals] == 0 && p->feature[idx_dpi_cc] == 0 && p->feature[idx_webs_filter] == 0 && p->feature[idx_web_history] == 0) {
				char *cmd[] = {"shn_ctrl", "-a", "set_func_bit", "-b", "0x147", NULL};
				_eval(cmd, NULL, 8, NULL);
			}
			break;
		}
	}
}
#endif
