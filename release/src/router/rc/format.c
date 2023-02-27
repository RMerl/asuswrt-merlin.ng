#include <rc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef RTCONFIG_OPENVPN
#include <openvpn_config.h>
#endif
#if defined(RTCONFIG_VPN_FUSION)
#include <vpnc_fusion.h>
extern int vpnc_load_profile(VPNC_PROFILE *list, const int list_size, const int prof_ver);
#endif
#if defined(RTCONFIG_NOTIFICATION_CENTER)
#include <libnt.h>
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void adjust_merlin_config(void)
{
#ifdef RTCONFIG_OPENVPN
	int unit;
	char varname_ori[32], varname_ori2[32], varname_new[32];
	int rgw, plan, converted;
	char *desc, *source, *dest, *iface, newiface[8];
#endif
	char buffer[65536];
	char *dhcp_hostnames;
	char *nv, *nvp, *entry;
	char *nv2, *nvp2, *entry2;
	char *name, *mac, *mac2, *ipaddr, *dns;
	char tmp[100];
	int count, len = 0, found;
	int need_commit = 0;

#ifdef RTCONFIG_OPENVPN
/* Migrate OVPN RGW + clientlist rules to VPN Director (386.3) */
	*buffer = '\0';

	for (unit = 1; unit <= OVPN_CLIENT_MAX; unit++) {
		sprintf(varname_ori, "vpn_client%d_rgw", unit);
		if (nvram_get_int(varname_ori) == OVPN_RGW_POLICY_STRICT) {
			nvram_set_int(varname_ori, OVPN_RGW_POLICY);
			need_commit = 1;
		}

		sprintf(varname_ori, "vpn_client%d_clientlist", unit);
		if (!nvram_is_empty(varname_ori)) {
			need_commit = 1;
#ifdef HND_ROUTER
			nv = nvp = malloc(255 * 6 + 1);
			if (nv) nvram_split_get(varname_ori, nv, 255 * 6 + 1, 5);
#else
			nv = nvp = strdup(nvram_safe_get(varname_ori));
#endif
			while (nv && (entry = strsep(&nvp, "<")) != NULL) {
				if (vstrsep(entry, ">", &desc, &source, &dest, &iface) != 4)
					continue;

				if (!strcmp(iface, "WAN"))
					strcpy(newiface, "WAN");
				else if (!strcmp(iface, "VPN"))
					sprintf(newiface, "OVPN%d", unit);
				else
					continue;	// invalid rule

				snprintf(tmp, sizeof(tmp), "<1>%s>%s>%s>%s", desc, source, dest, newiface);
				strlcat(buffer, tmp, 8000);
			}

			nvram_unset(varname_ori);
#ifdef HND_ROUTER
			sprintf(varname_ori, "vpn_client%d_clientlist1", unit);
			nvram_unset(varname_ori);
			sprintf(varname_ori, "vpn_client%d_clientlist2", unit);
			nvram_unset(varname_ori);
			sprintf(varname_ori, "vpn_client%d_clientlist3", unit);
			nvram_unset(varname_ori);
			sprintf(varname_ori, "vpn_client%d_clientlist4", unit);
			nvram_unset(varname_ori);
			sprintf(varname_ori, "vpn_client%d_clientlist5", unit);
			nvram_unset(varname_ori);
#endif
			free(nv);
		}
	}

	if (*buffer)
		ovpn_set_policy_rules(buffer);


/* Migrate OVPN custom settings, either from stock _custom, or previous AM _custom2 and _cust2 (386.3) */
	for (unit = 1; unit <= OVPN_SERVER_MAX; unit++) {

/* Handle custom2 migration */
		converted = 0;
		sprintf(varname_ori, "vpn_server%d_custom2", unit);
		if(!nvram_is_empty(varname_ori)) {
			strlcpy(buffer, nvram_safe_get(varname_ori), sizeof (buffer));
			nvram_unset(varname_ori);
#ifdef HND_ROUTER
			sprintf(varname_ori, "vpn_server%d_custom21", unit);
			strlcat(buffer, nvram_safe_get(varname_ori), sizeof (buffer));
			nvram_unset(varname_ori);

			sprintf(varname_ori, "vpn_server%d_custom22", unit);
			strlcat(buffer, nvram_safe_get(varname_ori), sizeof (buffer));
			nvram_unset(varname_ori);
#endif
			set_ovpn_custom(OVPN_TYPE_SERVER, unit, buffer);
			converted = 1;
		}

/* Handle cust2 base64 migration */
		sprintf(varname_ori, "vpn_server%d_cust2", unit);
		if(!converted && !nvram_is_empty(varname_ori)) {
			get_ovpn_custom_old(OVPN_TYPE_SERVER, unit, buffer, sizeof (buffer));
			set_ovpn_custom(OVPN_TYPE_SERVER, unit, buffer);
			nvram_unset(varname_ori);
			converted = 1;
		}

/* Handle stock or very old migration */
		sprintf(varname_ori, "vpn_server%d_custom", unit);
		if(!converted && !nvram_is_empty(varname_ori)) {
			set_ovpn_custom(OVPN_TYPE_SERVER, unit, nvram_safe_get(varname_ori));
			nvram_unset(varname_ori);
		}
	}
	need_commit |= converted;

	for (unit = 1; unit <= OVPN_CLIENT_MAX; unit++) {

/* Handle custom2 migration */
		converted = 0;
		sprintf(varname_ori, "vpn_client%d_custom2", unit);
		if(!nvram_is_empty(varname_ori)) {
			strlcpy(buffer, nvram_safe_get(varname_ori), sizeof (buffer));
			nvram_unset(varname_ori);
#ifdef HND_ROUTER
			sprintf(varname_ori, "vpn_client%d_custom21", unit);
			strlcat(buffer, nvram_safe_get(varname_ori), sizeof (buffer));
			nvram_unset(varname_ori);

			sprintf(varname_ori, "vpn_client%d_custom22", unit);
			strlcat(buffer, nvram_safe_get(varname_ori), sizeof (buffer));
			nvram_unset(varname_ori);
#endif
			set_ovpn_custom(OVPN_TYPE_CLIENT, unit, buffer);
			converted = 1;
		}

/* Handle cust2 base64 migration */
		sprintf(varname_ori, "vpn_client%d_cust2", unit);
		if(!converted && !nvram_is_empty(varname_ori)) {
			get_ovpn_custom_old(OVPN_TYPE_CLIENT, unit, buffer, sizeof (buffer));
			set_ovpn_custom(OVPN_TYPE_CLIENT, unit, buffer);
			nvram_unset(varname_ori);
			converted = 1;
		}

/* Handle stock or very old migration */
		sprintf(varname_ori, "vpn_client%d_custom", unit);
		if(!converted && !nvram_is_empty(varname_ori)) {
			set_ovpn_custom(OVPN_TYPE_CLIENT, unit, nvram_safe_get(varname_ori));
			nvram_unset(varname_ori);
		}
	}
	need_commit |= converted;

/* Migrate "remote gateway" and "push lan" to "client_access" (384.5) */
	for (unit = 1; unit <= OVPN_SERVER_MAX; unit++) {
		sprintf(varname_ori, "vpn_server%d_rgw", unit);

		if(!nvram_is_empty(varname_ori)) {
			need_commit = 1;
			sprintf(varname_new, "vpn_server%d_client_access", unit);
			sprintf(varname_ori2, "vpn_server%d_plan", unit);

			rgw = nvram_get_int(varname_ori);
			plan = nvram_get_int(varname_ori2);

			if (rgw && plan)
				nvram_set(varname_new, "2");
			else if (rgw && !plan)
				nvram_set(varname_new, "1");
			else
				nvram_set(varname_new, "0");

			nvram_unset(varname_ori);
			nvram_unset(varname_ori2);
		}
	}

/* Migrate vpn_server_nm6 (386.6) */
#ifdef RTCONFIG_IPV6
        for (unit = 1; unit <= OVPN_SERVER_MAX; unit++) {
                sprintf(varname_ori, "vpn_server%d_nm6", unit);

		if(!nvram_is_empty(varname_ori)) {
			sprintf(varname_new, "vpn_server%d_sn6", unit);
			need_commit = 1;
			if (!strchr(nvram_safe_get(varname_new), '/')) {
				snprintf(tmp, sizeof (tmp), "%s/%s", nvram_safe_get(varname_new), nvram_safe_get(varname_ori));
				nvram_set(varname_new, tmp);
			}
			nvram_unset(varname_ori);
		}
	}
#endif

#endif	// RTCONFIG_OPENVPN

/* migrate dhcpc_options to wanxxx_clientid */
	char *oldclientid = nvram_safe_get("wan0_dhcpc_options");
	if (*oldclientid) {
		need_commit = 1;
		nvram_set("wan0_clientid", oldclientid);
		nvram_unset("wan0_dhcpc_options");
	}

	oldclientid = nvram_safe_get("wan1_dhcpc_options");
	if (*oldclientid) {
		need_commit = 1;
		nvram_set("wan1_clientid", oldclientid);
		nvram_unset("wan1_dhcpc_options");
	}

#ifdef RTCONFIG_SSH
/* Migrate to Asus's new tri-state sshd_enable to our dual nvram setup */
	if (nvram_match("sshd_enable", "1")) {
		if (nvram_match("sshd_wan", "0"))
			nvram_set("sshd_enable", "2");  // LAN-only
		// else stay WAN+LAN
		nvram_unset("sshd_wan");
	}
#endif

/* Adjust automatic reboot count on failed radio - reduce from 3 to 1 reboot */
	if (nvram_match("dev_fail_reboot", "3")) {
		nvram_set("dev_fail_reboot", "1");
	}

/* Migrate lan_dns_fwd_local (384.11) */
	if (nvram_get_int("lan_dns_fwd_local")) {
		need_commit = 1;
		nvram_set("dns_fwd_local", "1");
		nvram_unset("lan_dns_fwd_local");
	}

/* Migrate dhcp_hostnames into dhcp_staticlist (386.4) */
#ifdef HND_ROUTER
	dhcp_hostnames = read_whole_file("/jffs/nvram/dhcp_hostnames");
	if (dhcp_hostnames)
		len = strlen(dhcp_hostnames) + 1;
#else
	dhcp_hostnames = strdup(nvram_safe_get("dhcp_hostnames"));
	len = strlen(dhcp_hostnames) + 1;
#endif

	if (len > 1) {
		converted = 0;
		*buffer = '\0';
		nv = nvp = strdup(nvram_safe_get("dhcp_staticlist"));
		nv2 = malloc(len);

		while ((entry = strsep(&nvp, "<")) != NULL) {
			count = vstrsep(entry, ">", &mac, &ipaddr, &dns, &name);

			if (count < 2)		// Invalid entry, skip it
				continue;
			else if (count == 2) {	// Very old entry
				snprintf(tmp, sizeof (tmp), "<%s>%s>>", mac, ipaddr);
			}
			else if (count == 3) {	// Missing hostname
				converted = 1;

				/* Find hostname if we have one */
				found = 0;
				if (nv2) {
					strlcpy(nv2, dhcp_hostnames, len);
					nvp2 = nv2;
					while ((entry2 = strsep(&nvp2, "<")) != NULL) {
						if (vstrsep(entry2, ">", &mac2, &name) == 2) {
							if (!strcasecmp(mac, mac2)) {
								found = 1;
								break;
							}
						}
					}
				}
				if (!found)
					name = "";
				snprintf(tmp, sizeof (tmp), "<%s>%s>%s>%s",mac, ipaddr, dns, name);
			}
			else if (count == 4) {	// Complete entry
				strlcpy(tmp, entry, sizeof(tmp));
			}
			strlcat(buffer, tmp, sizeof(buffer));
		}

		if (nv2) free(nv2);
		free(nv);

		if (converted) {
			need_commit = 1;
#ifdef HND_ROUTER
			jffs_nvram_unset("dhcp_hostnames");
#else
			nvram_unset("dhcp_hostnames");
#endif
			nvram_set("dhcp_staticlist", buffer);
		}
	}
	free(dhcp_hostnames);


/* Migrade DDNS external IP check (386.1) */
	if(!nvram_is_empty("ddns_ipcheck")) {
		need_commit = 1;
		nvram_set("ddns_realip_x", nvram_get("ddns_ipcheck"));
		nvram_unset("ddns_ipcheck");
	}

#ifdef RTCONFIG_SSH
/* Migrate SSH keys from nvram (386.1) */
	if (!d_exists("/jffs/.ssh")) {
		need_commit = 1;
		mkdir("/jffs/.ssh", 0700);
		if (nvram_get_file("sshd_hostkey", "/jffs/.ssh/dropbear_rsa_host_key", 2048))
			nvram_unset("sshd_hostkey");
		if (nvram_get_file("sshd_dsskey", "/jffs/.ssh/dropbear_dss_host_key", 2048))
			nvram_unset("sshd_dsskey");
		if (nvram_get_file("sshd_ecdsakey", "/jffs/.ssh/dropbear_ecdsa_host_key", 2048))
			nvram_unset("sshd_ecdsakey");
	}
#endif

	if (need_commit)
		nvram_commit();
}

void adjust_url_urlelist(void)
{
	char *nv, *nvp, *b, *chk, *chkp = NULL;
	char *url;
	char  replacebox[2048], rerule[256];
	int   cnt = 0;
	int   RESAVE = OFF;
	
	nv = nvp = strdup(nvram_safe_get("url_rulelist"));
	
	/* Change setting format 
	From:[<str1<str2<str3]
	To:  [<1>ALL>str1<1>ALL>str2<1>ALL>str3] 
	*/
	memset(replacebox, 0, sizeof(replacebox));
	while (nvp && (b = strsep(&nvp, "<")) != NULL) {
		chkp = chk = strdup(b);
		//dbg("[%s(%d)] %s\n", __FUNCTION__, __LINE__, chk);
		while( *chk != '\0') {
			if(*chk == '>') cnt++;
			chk++;
		}
		if (cnt != 2) {
			if (vstrsep(b, ">", &url) != 1) continue;
			if (*url) {
				memset(rerule, 0, sizeof(rerule));
				snprintf(rerule, sizeof(rerule), "<1>ALL>%s", url);
				//dbg("[%s(%d)] %s\n", __FUNCTION__, __LINE__, rerule);
				strcat(replacebox, rerule);
				RESAVE = ON;
			}
		}
		cnt = 0;
	}
	if (RESAVE) 
		nvram_set("url_rulelist", replacebox);
	if(nv) free(nv);
	if(chkp) free(chkp);
}

void adjust_ddns_config(void)
{
	char *server;

	server = nvram_safe_get("ddns_server_x");
	if(!strcmp(server, "WWW.GOOGLE-DDNS.COM") || 
	   !strcmp(server, "WWW.GOOGLE-DOMAINS.COM")) {
		nvram_set("ddns_server_x", "DOMAINS.GOOGLE.COM");
	}
	
}

void adjust_access_restrict_config(void)
{
	char *nv, *nvp, *b, *chk, *chkp = NULL;
	char *ipAddr;
	char *http_list;
	char *restrict_list;
	char  replacebox[2048], rerule[256];
	int   cnt = 0;
	int   RESAVE = OFF;
	int   nCount = 0;
	
	http_list = nvram_safe_get("http_clientlist");
	restrict_list = nvram_safe_get("restrict_rulelist");
	
	if ( (restrict_list[0] == '\0' && nvram_get_int("enable_acc_restriction") == 0) && 
	     (http_list[0] != '\0') ) {
		
		nvram_set_int("enable_acc_restriction", nvram_get_int("http_client"));
		
		nv = nvp = strdup(nvram_safe_get("http_clientlist"));
		
		/* Change setting format 
		From:[<ipAddr1<ipAddr2<ipAddr3]
		To:  [<1>ipAddr1>7<1>ipAddr2>7<1>ipAddr3>7]
		*/
		memset(replacebox, 0, sizeof(replacebox));
		while (nvp && (b = strsep(&nvp, "<")) != NULL) {
			chkp = chk = strdup(b);
			//dbg("[%s(%d)] %s\n", __FUNCTION__, __LINE__, chk);
			while( *chk != '\0') {
				if(*chk == '>') cnt++;
				chk++;
			}
			if (cnt != 2) {
				if (vstrsep(b, ">", &ipAddr) != 1) continue;
				if (*ipAddr) {
					memset(rerule, 0, sizeof(rerule));
					snprintf(rerule, sizeof(rerule), "<1>%s>7", ipAddr);
					//dbg("[%s(%d)] %s\n", __FUNCTION__, __LINE__, rerule);
					nCount = sizeof(replacebox) - strlen(replacebox);
					strncat(replacebox, rerule, nCount-1);
					RESAVE = ON;
				}
			}
			cnt = 0;
		}
		if (RESAVE) 
			nvram_set("restrict_rulelist", replacebox);
		if(nv) free(nv);
		if(chkp) free(chkp);
	}
}

#if defined(RTCONFIG_VPN_FUSION)
static VPNC_PROFILE vpnc_profile_tmp[MAX_VPNC_PROFILE] = {{0}};
static int vpnc_prof_cnt_tmp;

static int _find_active_vpnc_id()
{
	char *vpnc_proto, *vpnc_heartbeat_x, *vpn_clientx_eas, *vpnc_pppoe_username, *vpnc_pppoe_passwd;
	int i, proto_id = -1, ovpn_idx;
	
	vpnc_proto = nvram_safe_get("vpnc_proto");
	
	if(vpnc_proto[0] == '\0' || !strcmp(vpnc_proto, "disable"))
		return -1;	//no active vpn client

	if(!strcmp(vpnc_proto, "pptp"))
		proto_id = VPNC_PROTO_PPTP;
	if(!strcmp(vpnc_proto, "l2tp"))
		proto_id = VPNC_PROTO_L2TP;
	if(!strcmp(vpnc_proto, "openvpn"))
		proto_id = VPNC_PROTO_OVPN;
	
	switch(proto_id)		
	{
		case VPNC_PROTO_PPTP:
		case VPNC_PROTO_L2TP:
			{
				vpnc_heartbeat_x = nvram_safe_get("vpnc_heartbeat_x");
				vpnc_pppoe_username = nvram_safe_get("vpnc_pppoe_username");
				vpnc_pppoe_passwd = nvram_safe_get("vpnc_pppoe_passwd");
				for(i = 0; i < vpnc_prof_cnt_tmp; ++i)
				{
					if(vpnc_profile_tmp[i].protocol == proto_id && 
						!strcmp(vpnc_profile_tmp[i].basic.server, vpnc_heartbeat_x) &&
						!strcmp(vpnc_profile_tmp[i].basic.username, vpnc_pppoe_username) &&
						!strcmp(vpnc_profile_tmp[i].basic.password, vpnc_pppoe_passwd))
					{
						return i;
					}
				}
			}
			break;
		case VPNC_PROTO_OVPN:
			{
				vpn_clientx_eas = nvram_safe_get("vpn_clientx_eas");
				ovpn_idx = atoi(vpn_clientx_eas);
				
				for(i = 0; i < vpnc_prof_cnt_tmp; ++i)
				{
					if(vpnc_profile_tmp[i].protocol == proto_id && 
						vpnc_profile_tmp[i].config.ovpn.ovpn_idx == ovpn_idx)
					{
						return i;
					}
				}
			}
			break;
		default:
			break;
	}
	return -1;
}

void adjust_vpnc_config(void)
{
	char *vpnc_clientlist;
	char *vpnc_dev_policy_list;
	int active_id, i, default_wan_idx = 0, flag = 0;
	char buf[1024];
	char *nv = NULL, *nvp = NULL, *b = NULL, *mac, *static_ip, *desc, *proto, *server, *username, *passwd, *dns;

	_dprintf("[%s, %d]\n", __FUNCTION__, __LINE__);
	vpnc_clientlist = nvram_safe_get("vpnc_clientlist");
	if(vpnc_clientlist[0] != '\0')
	{
		vpnc_prof_cnt_tmp = vpnc_load_profile(vpnc_profile_tmp, MAX_VPNC_PROFILE, VPNC_PROFILE_VER1);

		if(!vpnc_prof_cnt_tmp)	// there is value in vpnc_clientlist but cannot be parsered. It might be old format.
		{
			vpnc_prof_cnt_tmp = vpnc_load_profile(vpnc_profile_tmp, MAX_VPNC_PROFILE, VPNC_PROFILE_VER_OLD);
		
			//find active id
			active_id = _find_active_vpnc_id();

			nv = nvp = strdup(nvram_safe_get("vpnc_clientlist"));
			i = 0;
			memset(buf, 0, sizeof(buf));
			
			while (nv && (b = strsep(&nvp, "<")) != NULL && i <= MAX_VPNC_PROFILE) {
				if (vstrsep(b, ">", &desc, &proto, &server, &username, &passwd) < 3)
					continue;

				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), 
					i? "<%s>%s>%s>%s>%s>%d>%d": "%s>%s>%s>%s>%s>%d>%d",
					desc? desc: "",
					proto? proto: "",
					server? server: "",
					username? username: "",
					passwd? passwd: "",
					(i == active_id)? 1: 0,
					VPNC_UNIT_BASIC + i);
					
				if(i == active_id)
				{
					default_wan_idx = VPNC_UNIT_BASIC + i;
				}
				++i;
			}
			if(nv) SAFE_FREE(nv);
			
			nvram_set("vpnc_clientlist", buf);
			snprintf(buf, sizeof(buf), "%d", default_wan_idx);
			nvram_set("vpnc_default_wan", buf);			

			flag = 1;
		}
	}

	vpnc_dev_policy_list = nvram_safe_get("vpnc_dev_policy_list");
	
	if(vpnc_dev_policy_list[0] == '\0' || flag)	//have dhcp_staticlist but no vpnc_dev_policy_list
	{
		nv = nvp = strdup(nvram_safe_get("dhcp_staticlist"));
		memset(buf, 0, sizeof(buf));
		i = 0;
		
		while (nv && (b = strsep(&nvp, "<")) != NULL ) {
			if (vstrsep(b, ">", &mac, &static_ip, &dns) < 2)
				continue;

#ifdef USE_IPTABLE_ROUTE_TARGE
			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), i?"<0>%s>>0": "0>%s>>0", mac);
#else
			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), i?"<0>%s>>0": "0>%s>>0", static_ip);
#endif
			++i;
		}
		if(nv) SAFE_FREE(nv);

		nvram_set("vpnc_dev_policy_list", buf);
	}
	
}
#endif

#if defined(RTCONFIG_NOTIFICATION_CENTER)
void force_off_push_msg(void)
{
	char *nv, *nvp, *b;
	char *eID, *eAct, *eType;
	char  replacebox[4096], rerule[256];
	int   action;
	int   RESAVE = OFF;
	
	/* force disable push msg. 
	Format:[<eID>eAction>eType]
	*/
	memset(replacebox, 0, sizeof(replacebox));
	nv = nvp = strdup(nvram_safe_get("nc_setting_conf"));
	while(nvp)
	{
		if ((b = strsep(&nvp, "<")) == NULL) break;
		if ((vstrsep(b, ">", &eID, &eAct, &eType)) != 3) continue;
			
			memset(rerule, 0, sizeof(rerule));
			action = atoi(eAct);
			if ((action & ACTION_NOTIFY_APP)) {
				RESAVE = ON;
			}
			NC_ACTION_CLR(action, NC_ACT_APP_BIT);
			snprintf(rerule, sizeof(rerule), "<%s>%d>%s", eID, action, eType);
			//dbg("[%s(%d)] %s\n", __FUNCTION__, __LINE__, rerule);
			strcat(replacebox, rerule);
	}		
	if (RESAVE) 
		nvram_set("nc_setting_conf", replacebox);
	if(nv) free(nv);
}
#endif

void adjust_jffs_content(void)
{
/* migrate httpd/ssh key/cert to same location as Asus */
	if (d_exists("/jffs/ssl")) {
		system("/bin/mv -f /jffs/ssl/* /jffs/.cert/");     /* */
		rmdir("/jffs/ssl");
	}
}
