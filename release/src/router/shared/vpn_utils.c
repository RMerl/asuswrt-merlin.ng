#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <shared.h>
#include <vpn_utils.h>
#include <openvpn_config.h>

#if defined(RTCONFIG_VPN_FUSION)
/*******************************************************************
 * NAME: vpnc_set_basic_conf
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2016/12/07
 * DESCRIPTION: set basic config dat structure
 * INPUT:  server: string. server ip. username: string. password: string.
 * OUTPUT:  basic_conf: a pointer of VPNC_BASIC_CONF
 * RETURN:  0: success, -1: fialed
 * NOTE:
 *******************************************************************/
static int
vpnc_set_basic_conf(const char *server, const char *username, const char *passwd, VPNC_BASIC_CONF *basic_conf)
{
	if (!basic_conf)
		return -1;

	memset(basic_conf, 0, sizeof(VPNC_BASIC_CONF));

	if (server)
		snprintf(basic_conf->server, sizeof(basic_conf->server), "%s", server);
	if (username)
		snprintf(basic_conf->username, sizeof(basic_conf->username), "%s", username);
	if (passwd)
		snprintf(basic_conf->password, sizeof(basic_conf->password), "%s", passwd);

	return 0;
}

static void _update_ovpn_client_enable(int unit, int enable)
{
	char buf[32] = {0};
	char *cp;
	char unit_str[4] = {0};
	int i;
	int ovpnc_enable[OVPN_CLIENT_MAX] = {0};

	nvram_safe_get_r("vpn_clientx_eas", buf, sizeof(buf));
	for( cp = strtok(buf, ","); cp != NULL; cp = strtok(NULL, ",")) {
		i = atoi(cp);
		if(i > OVPN_CLIENT_MAX || i <=0)
			continue;
		ovpnc_enable[i-1] = 1;
	}

	if (ovpnc_enable[unit-1] != ((enable) ? 1 : 0)) {
		ovpnc_enable[unit-1] = ((enable) ? 1 : 0);
		memset(buf, 0, sizeof(buf));
		for(i = 1; i <= OVPN_CLIENT_MAX; i++) {
			if (ovpnc_enable[i-1]) {
				snprintf(unit_str, sizeof(unit_str), "%d,", i);
				strlcat(buf, unit_str, sizeof(buf));
			}
		}
		nvram_set("vpn_clientx_eas", buf);
	}
}

/*******************************************************************
 * NAME: vpnc_load_profile
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2016/12/07
 * DESCRIPTION: Parser the nvram setting and load the VPNC_PROFILE list
 * INPUT:  list: an array to store vpnc profile, list_size: size of list
 * OUTPUT:
 * RETURN:  number of profiles, -1: fialed
 * NOTE:
 *******************************************************************/
int vpnc_load_profile(VPNC_PROFILE *list, const int list_size, const int prof_ver)
{
	char *nv = NULL, *nvp = NULL, *b = NULL;
	int cnt = 0, i;
	char *desc, *proto, *server, *username, *passwd, *active, *vpnc_idx;
	char *region, *conntype;

	if (!list || list_size <= 0)
		return -1;

	// load "vpnc_clientlist" to set username, password and server ip
	nv = nvp = strdup(nvram_safe_get("vpnc_clientlist"));

	cnt = 0;
	memset(list, 0, sizeof(VPNC_PROFILE) * list_size);
	while (nv && (b = strsep(&nvp, "<")) != NULL && cnt <= list_size)
	{
		if (VPNC_PROFILE_VER1 == prof_ver)
		{
			// proto, server, active and vpnc_idx are mandatory
			if (vstrsep(b, ">", &desc, &proto, &server, &username, &passwd, &active, &vpnc_idx, &region, &conntype) < 4)
				continue;

			if (!active || !vpnc_idx)
				continue;

			list[cnt].active = atoi(active);
			list[cnt].vpnc_idx = atoi(vpnc_idx);
		}
		else
		{
			// proto and server are mandatory
			if (vstrsep(b, ">", &desc, &proto, &server, &username, &passwd) < 2)
				continue;
		}

		if (proto && server)
		{
			vpnc_set_basic_conf(server, username, passwd, &(list[cnt].basic));

			if (!strcmp(proto, PROTO_PPTP))
			{
				list[cnt].protocol = VPNC_PROTO_PPTP;
			}
			else if (!strcmp(proto, PROTO_L2TP))
			{
				list[cnt].protocol = VPNC_PROTO_L2TP;
			}
			else if (!strcmp(proto, PROTO_OVPN))
			{
				list[cnt].protocol = VPNC_PROTO_OVPN;
				list[cnt].config.ovpn.ovpn_idx = atoi(server);
				_update_ovpn_client_enable(list[cnt].config.ovpn.ovpn_idx, list[cnt].active);
			}
#ifdef RTCONFIG_WIREGUARD
			else if (!strcmp(proto, PROTO_WG))
			{
				char prefix[16] = {0};
				list[cnt].protocol = VPNC_PROTO_WG;
				list[cnt].config.wg.wg_idx = atoi(server);
				snprintf(prefix, sizeof(prefix), "%s%d_", WG_CLIENT_NVRAM_PREFIX, list[cnt].config.wg.wg_idx);
				nvram_pf_set_int(prefix, "enable", list[cnt].active);
			}
#endif
#ifdef RTCONFIG_TPVPN
			else if (!strcmp(proto, PROTO_HMA))
			{
				if (is_tpvpn_configured(TPVPN_HMA, region, conntype, atoi(server)))
				{
					list[cnt].protocol = VPNC_PROTO_OVPN;
					list[cnt].config.ovpn.ovpn_idx = atoi(server);
					_update_ovpn_client_enable(list[cnt].config.ovpn.ovpn_idx, list[cnt].active);
				}
				else
				{
					list[cnt].protocol = VPNC_PROTO_HMA;
					list[cnt].config.tpvpn.tpvpn_idx = atoi(server);
					if (region && conntype)
					{
						strlcpy(list[cnt].config.tpvpn.region, region, sizeof(list[cnt].config.tpvpn.region));
						strlcpy(list[cnt].config.tpvpn.conntype, conntype, sizeof(list[cnt].config.tpvpn.conntype));
					}
					else
						logmessage_normal("VPN", "no data for HMA\n");
				}
			}
			else if (!strcmp(proto, PROTO_NORDVPN))
			{
				list[cnt].protocol = VPNC_PROTO_NORDVPN;
				list[cnt].config.tpvpn.tpvpn_idx = atoi(server);
				if (region)
					strlcpy(list[cnt].config.tpvpn.region, region, sizeof(list[cnt].config.tpvpn.region));
				else
					logmessage_normal("VPN", "no data for NordVPN\n");
			}
#endif
			else if (!strcmp(proto, PROTO_IPSec))
			{
				list[cnt].protocol = VPNC_PROTO_IPSEC;
				list[cnt].config.ipsec.prof_idx = atoi(server);
			}
			++cnt;
		}
	}
	SAFE_FREE(nv);

	// load "vpnc_pptp_options_x_list" to set pptp option
	nv = nvp = strdup(nvram_safe_get("vpnc_pptp_options_x_list"));
	i = 0;
	while (nv && (b = strsep(&nvp, "<")) != NULL && i <= cnt)
	{

		if (i > 0 && VPNC_PROTO_PPTP == list[i - 1].protocol)
		{
			if (!strcmp(b, "auto"))
				list[i - 1].config.pptp.option = VPNC_PPTP_OPT_AUTO;
			else if (!strcmp(b, "-mppc"))
				list[i - 1].config.pptp.option = VPNC_PPTP_OPT_MPPC;
			else if (!strcmp(b, "+mppe-40"))
				list[i - 1].config.pptp.option = VPNC_PPTP_OPT_MPPE40;
			else if (!strcmp(b, "+mppe-56"))
				list[i - 1].config.pptp.option = VPNC_PPTP_OPT_MPPE56;
			else if (!strcmp(b, "+mppe-128"))
				list[i - 1].config.pptp.option = VPNC_PPTP_OPT_MPPE128;
			else
				list[i - 1].config.pptp.option = VPNC_PPTP_OPT_UNDEF;
		}
		++i;
	}
	SAFE_FREE(nv);
	if (i != cnt + 1)
		_dprintf("[%s]the numbers of vpnc_clientlist(%d) and vpnc_pptp_options_x_list(%d) are different!\n", __FUNCTION__, cnt, i);

	return cnt;
}

int _get_new_vpnc_index(void)
{
	VPNC_PROFILE prof[MAX_VPNC_PROFILE];
	unsigned char idx_array[MAX_VPNC_PROFILE] ;
	int prof_cnt = 0, i;

	prof_cnt = vpnc_load_profile(prof, MAX_VPNC_PROFILE, VPNC_LOAD_CLIENT_LIST);

	memset(idx_array, 0, sizeof(idx_array));

	for(i = 0; i < prof_cnt; ++i)
	{
		if((prof[i].vpnc_idx - VPNC_UNIT_BASIC) >= MAX_VPNC_PROFILE || 
			(prof[i].vpnc_idx - VPNC_UNIT_BASIC) < 0)
			continue;
		idx_array[prof[i].vpnc_idx - VPNC_UNIT_BASIC] = 1;
	}

	for(i = 0; i < MAX_VPNC_PROFILE; ++i)
	{
		if(!idx_array[i])
			return i + VPNC_UNIT_BASIC;
	}
	return 0;
}
#endif

#ifdef RTCONFIG_TPVPN
int is_tpvpn_configured(int provider, const char* region, const char* conntype, int unit)
{
	char prefix[16] = {0};

	if (!region || !conntype) return 0;

	switch (provider)
	{
		case TPVPN_HMA:
			snprintf(prefix, sizeof(prefix), "vpn_client%d_", unit);
			if (!strcmp(nvram_pf_safe_get(prefix, "tp"), TPVPN_PSZ_HMA)
			 && !strcmp(nvram_pf_safe_get(prefix, "tp_region"), region)
			 && !strcmp(nvram_pf_safe_get(prefix, "tp_proto"), conntype)
			)
				return 1;
			break;
		case TPVPN_NORDVPN:
			snprintf(prefix, sizeof(prefix), "wgc%d_", unit);
			if (!strcmp(nvram_pf_safe_get(prefix, "tp"), TPVPN_PSZ_NORDVPN)
			 && !strcmp(nvram_pf_safe_get(prefix, "tp_region"), region)
			)
				return 1;
			break;
	}
	return 0;
}
#endif

#if defined(RTCONFIG_VPN_FUSION) || defined(RTCONFIG_WIREGUARD)
static char* _get_wgconf_val(char* buf)
{
	char *p = buf;
	int i = 0, len = 0, j = 0;

	if (!buf)
		return p;
	if ((p = strchr(buf, '='))) p++;

	len = strlen(p);
	for (i = 0; i < len; i++)
	{
		if (p[i] == ' ' || p[i] == '\r' || p[i] == '\n')
		{
			for(j = i; j < len; j++)
			{
				p[j] = p[j+1];
			}
			len--;
		}
	}
	return p;
}

int read_wgc_config_file(const char* file_path, int wgc_unit)
{
	char wgc_prefix[8] = {0};
	FILE *fp;
	char buf[256] = {0};

	if (!file_path || file_path[0] == '\0')
		return -1;

	snprintf(wgc_prefix, sizeof(wgc_prefix), "%s%d_", WG_CLIENT_NVRAM_PREFIX, wgc_unit);

	nvram_pf_restore_default("wgc_", wgc_prefix);

	fp = fopen(file_path, "r");
	if (fp)
	{
		while (fgets(buf, sizeof(buf), fp))
		{
			if (buf[0] == '[' || buf[0] == '#' || buf[0] == '\n')
				continue;
			else if (!strncmp(buf, "PrivateKey", 10))
				nvram_pf_set(wgc_prefix, "priv", _get_wgconf_val(buf));
			else if (!strncmp(buf, "Address", 7))
				nvram_pf_set(wgc_prefix, "addr", _get_wgconf_val(buf));
			else if (!strncmp(buf, "DNS", 3))
				nvram_pf_set(wgc_prefix, "dns", _get_wgconf_val(buf));
			else if (!strncmp(buf, "PublicKey", 9))
				nvram_pf_set(wgc_prefix, "ppub", _get_wgconf_val(buf));
			else if (!strncmp(buf, "PresharedKey", 12))
				nvram_pf_set(wgc_prefix, "psk", _get_wgconf_val(buf));
			else if (!strncmp(buf, "AllowedIPs", 10))
				nvram_pf_set(wgc_prefix, "aips", _get_wgconf_val(buf));
			else if (!strncmp(buf, "Endpoint", 8))
			{
				char *ep, *p;
				ep = _get_wgconf_val(buf);
				p = strchr(ep, ':');
				*p = '\0';
				nvram_pf_set(wgc_prefix, "ep_addr", ep);
				nvram_pf_set(wgc_prefix, "ep_port", p+1);
			}
			else if (!strncmp(buf, "PersistentKeepalive", 19))
				nvram_pf_set(wgc_prefix, "alive", _get_wgconf_val(buf));
			else
			{
				cprintf("[WG] Unsupport: [%s]\n", buf);
				logmessage_normal("WG", "Unsupport: [%s]", buf);
			}
		}
		fclose(fp);
	}
	else
		return -1;

	return 0;
}

int is_wgc_connected(int unit)
{
	char ifname[8] = {0};
	char buf[512] = {0};
	char filename[32] = {0};

	snprintf(ifname, sizeof(ifname), "%s%d", WG_CLIENT_IF_PREFIX, unit);
	snprintf(filename, sizeof(filename), "/etc/wg/%s_status", ifname);
	snprintf(buf, sizeof(buf), "wg show %s |grep handshake > %s 2>&1", ifname, filename);
	system(buf);

	memset(buf, 0 , sizeof(buf));
	if (f_read_string(filename, buf, sizeof(buf))) {
		char *p = strstr(buf, "sec:");
		unsigned long long t = (p) ? strtoull (p + 4, NULL, 0) : 0;
		if (strstr(buf, "Now"))
			return 1;
		else if (t <= 180)
			return 1;
		else
			return 0;
	}
	else
		return 0;
}
#endif
