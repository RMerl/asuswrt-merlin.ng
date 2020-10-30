/*
 * Copyright 2020, ASUSTeK Inc.
 * All Rights Reserved.
 *
 */

#include <string.h>
#include <sys/ioctl.h>

#include "rc.h"

void start_oam()
{
	oam_srv_t param_srv;
	char wan_ifnames[32];
	char *p = NULL;
	int i;
	char oam_prefix[8] = {0};
	int s;
	struct ifreq ifr;
	int enable = 0;

	memset(&param_srv, 0, sizeof(param_srv));

	// primary wan phy interface (not virtual interface)
	nvram_safe_get_r("wan_ifnames", wan_ifnames, sizeof(wan_ifnames));
	p = strchr(wan_ifnames, ' ');
	if (p) *p = '\0';
	strlcpy(param_srv.ifname, wan_ifnames, sizeof(param_srv.ifname));

	// check interface exist
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return;
	strncpy(ifr.ifr_name, wan_ifnames, IFNAMSIZ);
	if (ioctl(s, SIOCGIFFLAGS, &ifr))
	{
		_dprintf("%s: Interface not exist\n", __FUNCTION__);
		close(s);
		return;
	}

	param_srv.oam_3ah_enable = nvram_get_int("oam_3ah_enable");
	if (param_srv.oam_3ah_enable)
	{
		param_srv.id_3ah = nvram_get_int("oam_3ah_id");
		param_srv.auto_event = nvram_get_int("oam_auto_event");
		param_srv.variable_retrieval = nvram_get_int("oam_var_retrvl");
		param_srv.link_event = nvram_get_int("oam_link_event");
		param_srv.remote_loopback = nvram_get_int("oam_remote_lb");
		param_srv.active_mode = nvram_get_int("oam_active_mode");
		enable = 1;
	}

	for (i = 0; i < OAM_MODE_MAX; i++)
	{
		snprintf(oam_prefix, sizeof(oam_prefix), "oam%d_", i);
		param_srv.srv_enable[i] = nvram_pf_get_int(oam_prefix, "srv_enable");
		if (param_srv.srv_enable[i])
		{
			param_srv.mode[i] = nvram_pf_get_int(oam_prefix, "mode");
			strlcpy(param_srv.id[i], nvram_pf_safe_get(oam_prefix, "id"), sizeof(param_srv.id[i]));
			strlcpy(param_srv.md_name[i], nvram_pf_safe_get(oam_prefix, "md_name"), sizeof(param_srv.md_name[i]));
			param_srv.level[i] = nvram_pf_get_int(oam_prefix, "level");
			param_srv.local_mep_id[i] = nvram_pf_get_int(oam_prefix, "lmep_id");
			param_srv.local_mep_vid[i] = nvram_pf_get_int(oam_prefix, "lmep_vid");
			param_srv.remote_mep_id[i] = nvram_pf_get_int(oam_prefix, "rmep_id");
			param_srv.ccm_interval[i] = nvram_pf_get_int(oam_prefix, "ccm_itvl");
			enable = 1;
		}
	}

	if (enable)
		start_oam_service(&param_srv);
}

void stop_oam()
{
	stop_oam_service();
}
