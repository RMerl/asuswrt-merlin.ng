/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
#include "rc.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>

#include <openvpn_config.h>
#include <openvpn_utils.h>

#ifdef RTCONFIG_MULTILAN_CFG
#include "mtlan_utils.h"
#endif
#ifdef RTCONFIG_MULTIWAN_IF
#include "multi_wan.h"
#endif

#include <vpn_utils.h>
#include <vpnc_fusion.h>

char vpnc_resolv_path[] = "/tmp/resolv.vpnc%d";

static VPNC_PROFILE vpnc_profile[MAX_VPNC_PROFILE] = {{0}};
static int vpnc_profile_num = 0;

extern int start_firewall(int wanunit, int lanunit);

extern int vpnc_get_dev_policy_list(VPNC_DEV_POLICY *list, const int list_size, const int tmp_flag);
int vpnc_set_policy_by_ifname(const char *vpnc_ifname, const int action);
int stop_vpnc_by_unit(const int unit);
static int _set_routing_table(const int cmd, const int vpnc_id);
//static void vpnc_dump_vpnc_profile(const VPNC_PROFILE *profile);
static VPNC_PROFILE *vpnc_get_profile_by_vpnc_id(VPNC_PROFILE *list, const int list_size, const int vpnc_id);
VPNC_PROTO vpnc_get_proto_in_profile_by_vpnc_id(const int vpnc_id);
static int _set_default_routing_table(const VPNC_ROUTE_CMD cmd, const int table_id);
static int _set_routing_rule(const VPNC_ROUTE_CMD cmd, const VPNC_DEV_POLICY *policy);
int clean_routing_rule_by_vpnc_idx(const int vpnc_idx);
int clean_vpnc_setting_value(const int vpnc_idx);
static int _get_vpnc_state(const int vpnc_idx);
int vpnc_update_resolvconf(const int unit);
static int _set_network_routing_rule(const VPNC_ROUTE_CMD cmd, const int vpnc_idx);
extern int _gen_vpnc_resolv_conf(const int vpnc_idx);

int vpnc_pppstatus(const int unit)
{
	char statusfile[sizeof("/var/run/ppp-vpnXXXXXXXXXX.status")];

	snprintf(statusfile, sizeof(statusfile), "/var/run/ppp-vpn%d.status", unit);
	return _pppstatus(statusfile);
}

int start_vpnc(void) // start up all active vpnc profile
{
#ifdef RTCONFIG_VPN_FUSION_MERLIN
	return legacy_start_vpnc();
#else
	int i;

	// init vpnc profile
	vpnc_init();

	for (i = 0; i < vpnc_profile_num; ++i)
	{
		if (vpnc_profile[i].active)
			start_vpnc_by_unit(i);
	}
	return 0;
#endif
}

void stop_vpnc(void) // start up all active vpnc profile
{
#ifdef RTCONFIG_VPN_FUSION_MERLIN
	legacy_stop_vpnc();
#else
	int i;

	// init vpnc profile
	vpnc_init();

	for (i = 0; i < vpnc_profile_num; ++i)
	{
		if (vpnc_profile[i].active)
			stop_vpnc_by_unit(i);
	}

	return;
#endif
}

/*******************************************************************
 * NAME: change_default_wan_as_vpnc_updown
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2017/1/24
 * DESCRIPTION: change the default wan when a vpn client connection established or lost.
 * INPUT:  unit: index of vpn client. up: 1: connected, 0:disconnected
 * OUTPUT:
 * RETURN:  0: success, -1:failed
 * NOTE:
 *******************************************************************/
int change_default_wan_as_vpnc_updown(const int unit, const int up)
{
	char tmp[100];

	snprintf(tmp, sizeof(tmp), "%d", unit);

	if (nvram_match("vpnc_default_wan", tmp))
	{
		if (up)
			_set_default_routing_table(VPNC_ROUTE_ADD, unit);
		else
			_set_default_routing_table(VPNC_ROUTE_DEL, unit);
	}
	return 0;
}

#ifdef	RTCONFIG_MULTILAN_CFG
static void _set_sdn0_vpnc_idx(int vpnc_idx)
{
	char *sdn_rl = NULL, *sdn_rl_new = NULL;
	size_t s1, s2;
	int v;
	char *p;

	sdn_rl = strdup(nvram_safe_get("sdn_rl"));
	if (sdn_rl)
	{
		s1 = strlen(sdn_rl);
		if (s1)
		{
			s2 = s1 + 16;
			sdn_rl_new = calloc(s2, 1);
			if (sdn_rl_new)
			{
				p = sdn_rl;
				v = 6;	// vpnc_idx is the 7th field.
				while (v && (p = strchr(p+1, '>')))
					v--;
				if (v == 0)
				{
					strncpy(sdn_rl_new, sdn_rl, p - sdn_rl + 1);
					snprintf(sdn_rl_new + strlen(sdn_rl_new), s2 - strlen(sdn_rl_new), "%d", vpnc_idx);
					p = strchr(p+1, '>');
					if (p)
						strlcat(sdn_rl_new, p, s2);
					_dprintf("new sdn_rl=\n%s\n", sdn_rl_new);
					nvram_set("sdn_rl", sdn_rl_new);

				}
				free(sdn_rl_new);
			}
		}
		free(sdn_rl);
	}
}

// adjust for firmware upgrade
void adjust_sdn0_vpnc_idx()
{
	int vpnc_default_wan = nvram_get_int("vpnc_default_wan");
	if (vpnc_default_wan)
		_set_sdn0_vpnc_idx(vpnc_default_wan);
}
#endif

/*******************************************************************
 * NAME: change_default_wan
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2017/1/24
 * DESCRIPTION: when web ui change default wan, call this function to handle default wan
 * INPUT:
 * OUTPUT:
 * RETURN:  0:success, -1:failed
 * NOTE:	2020/3/27, Andy Chiu. Add nvram commit to save the vhanged default wan value.
 *******************************************************************/
int change_default_wan()
{
	int default_wan_new;

	// get default_wan
	default_wan_new = nvram_get_int("vpnc_default_wan_tmp");

#ifdef RTCONFIG_TUNNEL
	stop_aae_sip_conn(1);
#endif

#if defined(RTCONFIG_WIREGUARD) && (defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_BCM_502L07P2) || defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_HND_ROUTER_BE_4916))
	int i;
	for (i = 0; i < vpnc_profile_num; ++i)
	{
		if (vpnc_profile[i].active
		 && vpnc_profile[i].protocol == VPNC_PROTO_WG
		 && vpnc_profile[i].vpnc_idx == default_wan_new
		) {
			hnd_skip_wg_all_lan(1);
			break;
		}
	}
	if (i == vpnc_profile_num)
		hnd_skip_wg_all_lan(0);
#endif

#ifdef	RTCONFIG_MULTILAN_CFG
	nvram_set_int("vpnc_default_wan", default_wan_new);
	// _set_default_routing_table(VPNC_ROUTE_ADD, default_wan_new);
	_set_sdn0_vpnc_idx(default_wan_new);
	remove_ip_rules(IP_RULE_PREF_DEFAULT_CONN, 0);
	remove_ip_rules(IP_RULE_PREF_DEFAULT_CONN, 1);
	handle_sdn_feature(LAN_IN_SDN_IDX, SDN_FEATURE_VPNC, 0);
#else
	_set_default_routing_table(VPNC_ROUTE_ADD, default_wan_new);
	nvram_set_int("vpnc_default_wan", default_wan_new);
#endif
	vpnc_update_resolvconf(default_wan_new);

	nvram_set("vpnc_default_wan_tmp", "");

	nvram_commit_x();
	return 0;
}

int vpnc_ppp_linkunit(const char *linkname)
{
	if (!linkname)
		return -1;
	else if (strncmp(linkname, "vpn", 3))
		return -1;
	else if (!isdigit(linkname[3]))
		return -1;
	else
		return atoi(linkname + 3);
}

int vpnc_ppp_linkunit_by_ifname(const char *ifname)
{
	if (!ifname)
		return -1;
	else if (strncmp(ifname, "ppp", 3))
		return -1;
	else if (!isdigit(ifname[3]))
		return -1;
	else
		return atoi(ifname + 3);
}

void update_vpnc_state(const int vpnc_idx, const int state, const int reason)
{
	char tmp[100];
	char prefix[12];

	_dprintf("%s(%d, %d, %d)\n", __FUNCTION__, vpnc_idx, state, reason);

	// create prefix
	snprintf(prefix, sizeof(prefix), "vpnc%d_", vpnc_idx);

	nvram_set_int(strlcat_r(prefix, "state_t", tmp, sizeof(tmp)), state);
	nvram_set_int(strlcat_r(prefix, "sbstate_t", tmp, sizeof(tmp)), 0);

	if (state == WAN_STATE_STOPPED)
	{
		// Save Stopped Reason
		// keep ip info if it is stopped from connected
		nvram_set_int(strlcat_r(prefix, "sbstate_t", tmp, sizeof(tmp)), reason);
	}
	else if (state == WAN_STATE_STOPPING)
	{
		snprintf(tmp, sizeof(tmp), "/var/run/ppp-vpn%d.status", vpnc_idx);
		unlink(tmp);
	}
}

void update_ovpn_vpnc_state(const int vpnc_idx, const int state, const int reason)
{
	char tmp[100];
	char prefix[16];
	VPNC_PROFILE *prof = NULL;

	_dprintf("%s(%d, %d, %d)\n", __FUNCTION__, vpnc_idx, state, reason);

	prof = vpnc_get_profile_by_vpnc_id(vpnc_profile, MAX_VPNC_PROFILE, vpnc_idx);

	if (prof && prof->protocol == VPNC_PROTO_OVPN)
	{
		snprintf(prefix, sizeof(prefix), "vpn_client%d_", prof->config.ovpn.ovpn_idx);
		if (state == WAN_STATE_STOPPED && reason == WAN_STOPPED_REASON_IPGATEWAY_CONFLICT)
		{
			nvram_set_int(strlcat_r(prefix, "state", tmp, sizeof(tmp)), OVPN_STS_ERROR);
			nvram_set_int(strlcat_r(prefix, "errno", tmp, sizeof(tmp)), OVPN_ERRNO_ROUTE);
		}
	}
}

// Andy Chiu, 2019/12/27. Only the DNS setting of default WAN could be set in resolve.dnsmasq.
// Don't modify resolve.conf.
// If the default WAN does not include dns, copy resolve.conf to resolve.dnsmasq.
int vpnc_update_resolvconf(const int unit)
{
	FILE *fp = NULL, *fp_servers = NULL;
	char tmp[100], prefix[sizeof("vpncXXXXXXXXXX_")];
	char lan_ip[sizeof("192.168.123.456")];
	char *wan_dns, *next;
	int lock, reload_dns = 0, nr_server = 0;

	strlcpy(lan_ip, nvram_safe_get("lan_ipaddr"), sizeof(lan_ip));
	_gen_vpnc_resolv_conf(unit);

	// only default WAN need to be handled.
	if (nvram_get_int("vpnc_default_wan") == unit)
	{
		// get dns setting
		snprintf(prefix, sizeof(prefix), "vpnc%d_", unit);
		wan_dns = nvram_safe_get(strlcat_r(prefix, "dns", tmp, sizeof(tmp)));

#ifdef RTCONFIG_YANDEXDNS
		int yadns_mode = nvram_get_int("yadns_enable_x") ? nvram_get_int("yadns_mode") : YADNS_DISABLED;
#endif
#ifdef RTCONFIG_DNSPRIVACY
		int dnspriv_enable = nvram_get_int("dnspriv_enable");
#endif

		lock = file_lock("resolv");
#ifdef RTCONFIG_YANDEXDNS
		if (yadns_mode != YADNS_DISABLED)
		{
			/* keep yandex.dns servers */
			fp_servers = NULL;
		}
		else
#endif
#ifdef RTCONFIG_DNSPRIVACY
			if (dnspriv_enable)
		{
			/* keep dns privacy servers */
			fp_servers = NULL;
		}
		else
#endif
			if (!(fp_servers = fopen("/tmp/resolv.dnsmasq", "w+")))
		{
			perror("/tmp/resolv.dnsmasq");
			file_unlock(lock);
			return -1;
		}

		if (fp_servers)
		{
			if (wan_dns[0] != '\0') // write dns setting of VPN client to resolv.dnsmasq
			{
				foreach (tmp, wan_dns, next)
				{
					if (!strcmp(tmp, lan_ip))
						continue;
					fprintf(fp_servers, "server=%s\n", tmp);
					nr_server++;
				}
				reload_dns = 1;
			}

			if (!nr_server) { // write resolv.conf to resolv.dnsmasq
				fp = fopen("/tmp/resolv.conf", "r");
				if (fp)
				{
					while (fgets(tmp, sizeof(tmp), fp))
					{
						if (!strncmp(tmp, "nameserver ", 11))
						{
							if (tmp[strlen(tmp) - 1] == '\n')
								tmp[strlen(tmp) - 1] = '\0';
							fprintf(fp_servers, "server=%s\n", tmp + 11);
						}
					}
					reload_dns = 1;
					fclose(fp);
				}
			}
			fclose(fp_servers);
		}

		file_unlock(lock);
		if (reload_dns)
		{
#ifdef RTCONFIG_MULTILAN_CFG
			reload_dnsmasq(ALL_SDN);
#else
			reload_dnsmasq();
#endif
		}
		return 0;
	}
	return 0;
}

int vpnc_up(const int unit, const char *vpnc_ifname)
{
	char tmp[100], wan_prefix[] = "wanXXXXXXXXXX_", vpnc_prefix[] = "vpncXXXX_";
	struct in_addr wan_ip, lan_ip, netmask;

	if (!vpnc_ifname)
		return -1;

	_dprintf("[%s]unit=%d, vpnc_ifname=%s\n", __FUNCTION__, unit, vpnc_ifname);

	// get vpnc_prefix and wan_prefix
	snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", unit);
	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_primary_ifunit());

	// check ip conflict
	_dprintf("[%s]vpnc_ip=%s\n", __FUNCTION__, nvram_safe_get(strlcat_r(vpnc_prefix, "ipaddr", tmp, sizeof(tmp))));
	inet_aton(nvram_safe_get(strlcat_r(vpnc_prefix, "ipaddr", tmp, sizeof(tmp))), &wan_ip);
	inet_aton(nvram_safe_get("lan_ipaddr"), &lan_ip);
	inet_aton(nvram_safe_get("lan_netmask"), &netmask);

	//_dprintf("[%s, %d]wan_ip=%x, lan_ip=%x, lan_netmask=%x\n", __FUNCTION__, __LINE__, wan_ip.s_addr, lan_ip.s_addr, netmask.s_addr);
	if ((wan_ip.s_addr & netmask.s_addr) == (lan_ip.s_addr & netmask.s_addr))
	{
		_dprintf("%s: ip conflict\n", __FUNCTION__);
		update_vpnc_state(unit, WAN_STATE_STOPPED, WAN_STOPPED_REASON_IPGATEWAY_CONFLICT);
		update_ovpn_vpnc_state(unit, WAN_STATE_STOPPED, WAN_STOPPED_REASON_IPGATEWAY_CONFLICT);
		return -1;
	}

	/* Add dns servers to resolv.conf */
	if (nvram_invmatch(strlcat_r(vpnc_prefix, "dns", tmp, sizeof(tmp)), ""))
		vpnc_update_resolvconf(unit);

	update_vpnc_state(unit, WAN_STATE_CONNECTED, 0);

	/* Add firewall rules for VPN client */
	// vpnc_add_firewall_rule(unit, vpnc_ifname);
	start_firewall(wan_primary_ifunit(), 0);
	vpnc_set_policy_by_ifname(vpnc_ifname, 1);

	_set_network_routing_rule(VPNC_ROUTE_ADD, unit);

	// set up default wan
	change_default_wan_as_vpnc_updown(unit, 1);
#if defined(RTCONFIG_WIREGUARD) && (defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_BCM_502L07P2) || defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_HND_ROUTER_BE_4916))
	if (!strncmp(vpnc_ifname, WG_CLIENT_IF_PREFIX, strlen(WG_CLIENT_IF_PREFIX))
	 && unit == nvram_get_int("vpnc_default_wan")
	){
		hnd_skip_wg_all_lan(1);
	}
#endif

	return 0;
}

int vpnc_ipup_main(int argc, char **argv)
{
#ifdef RTCONFIG_VPN_FUSION_MERLIN
	return legacy_vpnc_ipup_main(argc, argv);
#else
	FILE *fp;
	char *vpnc_ifname = safe_getenv("IFNAME");
	char *vpnc_linkname = safe_getenv("LINKNAME");
	char tmp[100], vpnc_prefix[] = "vpncXXXX_";
	char buf[256], *value;
	int unit;

	_dprintf("%s():: %s\n", __FUNCTION__, argv[0]);

	/* Get unit from LINKNAME: vpn[UNIT] */
	if ((unit = vpnc_ppp_linkunit(vpnc_linkname)) < 0)
		return 0;

	snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", unit);

	_dprintf("%s: unit=%d ifname=%s\n", __FUNCTION__, unit, vpnc_ifname);

	/* Touch connection file */
	if (!(fp = fopen(strlcat_r("/tmp/ppp/link.", vpnc_ifname, tmp, sizeof(tmp)), "a")))
	{
		perror(tmp);
		return errno;
	}
	fclose(fp);

	if ((value = getenv("IPLOCAL")))
	{
		if (nvram_invmatch(strlcat_r(vpnc_prefix, "ipaddr", tmp, sizeof(tmp)), value))
			ifconfig(vpnc_ifname, IFUP, "0.0.0.0", NULL);
		_ifconfig(vpnc_ifname, IFUP, value, "255.255.255.255", getenv("IPREMOTE"), 0);
		nvram_set(strlcat_r(vpnc_prefix, "ipaddr", tmp, sizeof(tmp)), value);
		// nvram_set(strlcat_r(vpnc_prefix, "netmask", tmp, sizeof(tmp)), "255.255.255.255");
	}

	if ((value = getenv("IPREMOTE")))
	{
		nvram_set(strlcat_r(vpnc_prefix, "gateway", tmp, sizeof(tmp)), value);
	}

	strcpy(buf, "");
	if ((value = getenv("DNS1")))
		snprintf(buf, sizeof(buf), "%s", value);
	if ((value = getenv("DNS2")))
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s%s", strlen(buf) ? " " : "", value);
	nvram_set(strlcat_r(vpnc_prefix, "dns", tmp, sizeof(tmp)), buf);

	// load vpnc profile list
	vpnc_init();

	if (!vpnc_up(unit, vpnc_ifname))
	{
		// add routing table
		_set_routing_table(1, unit);
	}

	_dprintf("%s:: done\n", __FUNCTION__);
	return 0;
#endif
}

void vpnc_down(const int vpnc_idx, char *vpnc_ifname)
{
	if (!vpnc_ifname)
		return;

	vpnc_set_policy_by_ifname(vpnc_ifname, 0);
	start_firewall(wan_primary_ifunit(), 0);
	_set_network_routing_rule(VPNC_ROUTE_DEL, vpnc_idx);
	update_resolvconf();

	// clean routing rule
	clean_routing_rule_by_vpnc_idx(vpnc_idx);

	// set up default wan
	change_default_wan_as_vpnc_updown(vpnc_idx, 0);
#if defined(RTCONFIG_WIREGUARD) && (defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_BCM_502L07P2) || defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_HND_ROUTER_BE_4916))
	if (!strncmp(vpnc_ifname, WG_CLIENT_IF_PREFIX, strlen(WG_CLIENT_IF_PREFIX))
	 && vpnc_idx == nvram_get_int("vpnc_default_wan")
	){
		hnd_skip_wg_all_lan(0);
	}
#endif

	// clean setting value.
	clean_vpnc_setting_value(vpnc_idx);
}

/*
 * Called when link goes down
 */
int vpnc_ipdown_main(int argc, char **argv)
{
#ifdef RTCONFIG_VPN_FUSION_MERLIN
	return legacy_vpnc_ipdown_main(argc, argv);
#else
	char *vpnc_ifname = safe_getenv("IFNAME");
	char *vpnc_linkname = safe_getenv("LINKNAME");
	char tmp[100];
	int unit;

	_dprintf("%s():: %s\n", __FUNCTION__, argv[0]);

	/* Get unit from LINKNAME: vpn[UNIT] */
	if ((unit = vpnc_ppp_linkunit(vpnc_linkname)) < 0)
		return 0;

	_dprintf("%s: unit=%d ifname=%s\n", __FUNCTION__, unit, vpnc_ifname);

	/* override wan_state to get real reason */
	update_vpnc_state(unit, WAN_STATE_STOPPED, vpnc_pppstatus(unit));

	// load vpnc profile list
	vpnc_init();

	vpnc_down(unit, vpnc_ifname);

	unlink(strlcat_r("/tmp/ppp/link.", vpnc_ifname, tmp, sizeof(tmp)));

	// del routing table
	_set_routing_table(0, unit);

	_dprintf("%s:: done\n", __FUNCTION__);
	return 0;
#endif
}

/*
 * Called when interface comes up
 */
int vpnc_ippreup_main(int argc, char **argv)
{
#ifdef RTCONFIG_VPN_FUSION_MERLIN
	return legacy_vpnc_ippreup_main(argc, argv);
#else
	char *vpnc_ifname = safe_getenv("IFNAME");
	char *vpnc_linkname = safe_getenv("LINKNAME");
	char tmp[100], prefix[] = "vpncXXXX_";
	int unit;

	_dprintf("%s():: (%d)%s\n", __FUNCTION__, argc, argv[0]);

	/* Get unit from LINKNAME: vpn[UNIT] */
	if ((unit = vpnc_ppp_linkunit(vpnc_linkname)) < 0)
		return 0;

	_dprintf("%s: unit=%d ifname=%s\n", __FUNCTION__, unit, vpnc_ifname);

	/* Set vpnc_pppoe_ifname to real interface name */
	snprintf(prefix, sizeof(prefix), "vpnc%d_", unit);
	nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), vpnc_ifname);

	_dprintf("%s:: done\n", __FUNCTION__);
	return 0;
#endif
}

/*
 * Called when link closing with auth fail
 */
int vpnc_authfail_main(int argc, char **argv)
{
#ifdef RTCONFIG_VPN_FUSION_MERLIN
	return legacy_vpnc_authfail_main(argc, argv);
#else
	char *vpnc_ifname = safe_getenv("IFNAME");
	char *vpnc_linkname = safe_getenv("LINKNAME");
	int unit;

	_dprintf("%s():: %s\n", __FUNCTION__, argv[0]);

	/* Get unit from LINKNAME: ppp[UNIT] */
	if ((unit = vpnc_ppp_linkunit(vpnc_linkname)) < 0)
		return 0;

	_dprintf("%s: unit=%d ifname=%s\n", __FUNCTION__, unit, vpnc_ifname);

	/* override vpnc_state */
	update_vpnc_state(unit, WAN_STATE_STOPPED, WAN_STOPPED_REASON_PPP_AUTH_FAIL);

	_dprintf("%s:: done\n", __FUNCTION__);
	return 0;
#endif
}

/*******************************************************************
 * NAME: _find_vpnc_idx_by_ovpn_unit
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2017/2/23
 * DESCRIPTION: get vpnc_idx by ovpn unit
 * INPUT:  ovpn_unit: the unit of OpenVPN config
 * OUTPUT:
 * RETURN:  -1: not found. Otherwise, matched profile vpnc_idx
 * NOTE:
 *******************************************************************/
static int _find_vpnc_idx_by_ovpn_unit(const int ovpn_unit)
{
#ifdef RTCONFIG_VPN_FUSION_MERLIN
	return ovpn_unit + 5;
#else
	int i;
	VPNC_PROFILE *prof;
	for (i = 0; i < vpnc_profile_num; ++i)
	{
		prof = vpnc_profile + i;
		if (prof->protocol == VPNC_PROTO_OVPN && prof->config.ovpn.ovpn_idx == ovpn_unit + 5)
			return prof->vpnc_idx;
	}
	return -1;
#endif
}

int find_vpnc_idx_by_wgc_unit(int wgc_unit)
{
#ifdef RTCONFIG_VPN_FUSION_MERLIN
	return wgc_unit;	// WG uses same unit number
#else
	int i;
	VPNC_PROFILE *prof;
	for (i = 0; i < vpnc_profile_num; ++i)
	{
		prof = vpnc_profile + i;
		if (prof->protocol == VPNC_PROTO_WG && prof->config.wg.wg_idx == wgc_unit)
			return prof->vpnc_idx;
	}
	return -1;
#endif
}

void vpnc_ovpn_set_dns(int ovpn_unit)
{
	char nvname[16] = {0};
	char *old = NULL;
	char *new = NULL;
	size_t size = 0;
	char buf[128];
	char addr[16];
	FILE *fp = NULL;
	char path[128] = {0};

	snprintf(nvname, sizeof(nvname), "vpnc%d_dns", _find_vpnc_idx_by_ovpn_unit(ovpn_unit));
	snprintf(path, sizeof(path), "/etc/openvpn/client%d/resolv.dnsmasq", ovpn_unit);

	fp = fopen(path, "r");
	if (!fp)
	{
		_dprintf("[%s] no %s file\n", __FUNCTION__, path);
		return;
	}
	while (fgets(buf, sizeof(buf), fp) != NULL)
	{
		// e.g. server=1.1.1.1
		if (sscanf(buf, "server=%15s", addr) != 1)
		{
			_dprintf("\n=====\nunknown %s\n=====\n", buf);
			continue;
		}

		trim_r(addr);

		old = nvram_safe_get(nvname);
		if (*old)
		{
			size = strlen(old) + strlen(addr) + 2;
			new = malloc(size);
			if (new)
			{
				snprintf(new, size, "%s %s", old, addr);
				nvram_set(nvname, new);
				free(new);
			}
			else
			{
				_dprintf("vpnc_ovpn_update_dns fail\n");
				continue;
			}
		}
		else
		{
			nvram_set(nvname, addr);
		}
	}
	fclose(fp);
}

void vpnc_handle_dns_policy_rule(const VPNC_ROUTE_CMD cmd, const int vpnc_id)
{
	char nvname[16], cmd_str[8], id_str[8];
	char tmp[32], priority[8];
	char *vpn_dns, *next;

	snprintf(nvname, sizeof(nvname), "vpnc%d_dns", vpnc_id);
	snprintf(cmd_str, sizeof(cmd_str), "%s", (cmd == VPNC_ROUTE_DEL) ? "del" : "add");
	snprintf(id_str, sizeof(id_str), "%d", IP_ROUTE_TABLE_ID_VPNC_BASE + vpnc_id);
	snprintf(priority, sizeof(priority), "%d", IP_RULE_PREF_VPNC_POLICY_CLIENT);

	vpn_dns = nvram_safe_get(nvname);
	foreach (tmp, vpn_dns, next)
	{
		//_dprintf("dns %s\n", tmp);
		eval("ip", "rule", cmd_str, "iif", "lo", "to", tmp, "table", id_str, "priority", priority);
	}
}

/*******************************************************************
 * NAME: vpnc_ovpn_up_main
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2017/2/22
 * DESCRIPTION: callback for openvpn
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTE:
 *******************************************************************/
int vpnc_ovpn_up_main(int argc, char **argv)
{
	int unit, vpnc_idx;
	char *ifname = safe_getenv("dev");
	char *ipaddr = safe_getenv("ifconfig_local");
	char *vpn_gateway = safe_getenv("route_vpn_gateway");
	char tmp[100], prefix[] = "vpncXXXX_";

	if (argc < 2)
	{
		_dprintf("[%s]parameters error!\n", __FUNCTION__);
		return 0;
	}

	unit = atoi(argv[1]);

	ovpn_up_handler();

	// load vpnc profile list
	vpnc_init();

	vpnc_idx = _find_vpnc_idx_by_ovpn_unit(unit);

	if (vpnc_idx != -1)
	{
		snprintf(prefix, sizeof(prefix), "vpnc%d_", vpnc_idx);
		nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), ifname);
		if (ipaddr)
			nvram_set(strlcat_r(prefix, "ipaddr", tmp, sizeof(tmp)), ipaddr);
		if (vpn_gateway)
			nvram_set(strlcat_r(prefix, "gateway", tmp, sizeof(tmp)), vpn_gateway);
		nvram_set(strlcat_r(prefix, "dns", tmp, sizeof(tmp)), ""); // clean dns

		vpnc_ovpn_set_dns(unit);
		vpnc_update_resolvconf(vpnc_idx);
		// update_resolvconf();
		start_firewall(wan_primary_ifunit(), 0);
	}
	return 0;
}

/*******************************************************************
 * NAME: vpnc_ovpn_down_main
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2017/2/22
 * DESCRIPTION: callback for openvpn
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTE:
 *******************************************************************/
int vpnc_ovpn_down_main(int argc, char **argv)
{
	int unit, vpnc_idx;
	char *ifname = safe_getenv("dev");
	if (argc < 2)
	{
		_dprintf("[%s]parameters error!\n", __FUNCTION__);
		return 0;
	}

	unit = atoi(argv[1]);

	_dprintf("[%s]unit=%d\n", __FUNCTION__, unit);

	ovpn_down_handler();

	// load vpnc profile list
	vpnc_init();

	vpnc_idx = _find_vpnc_idx_by_ovpn_unit(unit);
	if (vpnc_idx != -1)
	{
		/* override wan_state to get real reason */
		update_vpnc_state(vpnc_idx, WAN_STATE_STOPPED, 0);

		vpnc_down(vpnc_idx, ifname);
	}
	return 0;
}

/*******************************************************************
 * NAME: _check_ovpn_route
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2019/4/11
 * DESCRIPTION: check the routing rule sent from vpn server
 * INPUT:  vpnc_idx: int. the index of vpn client.
 * OUTPUT:
 * RETURN:  0: routing rules are okay. -1: routing rule is error.
 * NOTE:
 *******************************************************************/
int _check_ovpn_route(int vpnc_idx)
{
	int n = 1, flag = -1;
	char env[32] = "route_parm_n";
	char tag[32];
	char *route_network = NULL;
	char *route_netmask = NULL;
	char *route_gateway = NULL;
	struct in_addr network, netmask;

	snprintf(tag, sizeof(tag), "vpnclient%d", vpnc_idx);

	while (1)
	{
		snprintf(env, sizeof(env), "route_network_%d", n);
		route_network = safe_getenv(env);
		if (!route_network || route_network[0] == '\0')
			break;
		snprintf(env, sizeof(env), "route_netmask_%d", n);
		route_netmask = safe_getenv(env);
		snprintf(env, sizeof(env), "route_gateway_%d", n);
		route_gateway = safe_getenv(env);

		if (inet_pton(AF_INET, route_network, &network) > 0 && inet_pton(AF_INET, route_netmask, &netmask) > 0)
		{
			_dprintf("route: %s / %s\n", route_network, route_netmask);
			if (current_route(network.s_addr, netmask.s_addr) || current_addr(network.s_addr))
			{
				if (network.s_addr & netmask.s_addr)
				{
					_dprintf("route conflict: %s/%s\n", route_network, route_netmask);
					logmessage_normal(tag, "WARNING: Ignore conflicted routing rule: %s %s gw %s", route_network, route_netmask, route_gateway);
				}
				else
				{
					_dprintf("Detect default gateway: %s/%s\n", route_network, route_netmask);
					// logmessage_normal(tag, "WARNING: Replace default vpn gateway by using 0.0.0.0/1 and 128.0.0.0/1");
				}
				update_ovpn_vpnc_state(vpnc_idx, WAN_STATE_STOPPED, WAN_STOPPED_REASON_IPGATEWAY_CONFLICT);
				flag = 0;
			}
		}

		n++;
	}
	return flag;

}

/*******************************************************************
 * NAME: vpnc_ovpn_route_up_main
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2017/2/22
 * DESCRIPTION: callback for openvpn
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTE:
 *******************************************************************/
int vpnc_ovpn_route_up_main(int argc, char **argv)
{
	int unit, vpnc_idx;
	char *ifname = safe_getenv("dev");
	char rt_table[8] = {0};

	if (argc < 2)
	{
		_dprintf("[%s]parameters error!\n", __FUNCTION__);
		return 0;
	}

	unit = atoi(argv[1]);

	_dprintf("[%s]openvpn unit = %d, ifname = %s\n", __FUNCTION__, unit, ifname);

	// load vpnc profile list
	vpnc_init();

	vpnc_idx = _find_vpnc_idx_by_ovpn_unit(unit);

	if (vpnc_idx != -1)
	{
		if (!vpnc_up(vpnc_idx, ifname))
		{
			snprintf(rt_table, sizeof(rt_table), "%d", IP_ROUTE_TABLE_ID_VPNC_BASE + vpnc_idx);
			setenv("rt_table", rt_table, 1);
			ovpn_route_up_handler();
		}
	}
	return 0;
}

int vpnc_ovpn_route_pre_down_main(int argc, char **argv)
{
	int unit, vpnc_idx;
	char *ifname = safe_getenv("dev");
	char rt_table[8] = {0};

	if (argc < 2)
	{
		_dprintf("[%s]parameters error!\n", __FUNCTION__);
		return 0;
	}

	unit = atoi(argv[1]);

	_dprintf("[%s]openvpn unit = %d, ifname = %s\n", __FUNCTION__, unit, ifname);

	// load vpnc profile list
	vpnc_init();

	vpnc_idx = _find_vpnc_idx_by_ovpn_unit(unit);

	if (vpnc_idx != -1)
	{
		snprintf(rt_table, sizeof(rt_table), "%d", IP_ROUTE_TABLE_ID_VPNC_BASE + vpnc_idx);
		setenv("rt_table", rt_table, 1);
		ovpn_route_pre_down_handler();
	}

	return 0;
}

/*******************************************************************
 * NAME: vpnc_dump_dev_policy_list
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2016/11/23
 * DESCRIPTION: dump VPNC_DEV_POLICY list
 * INPUT:  list: an array to store policy, list_size: size of list
 * OUTPUT:
 * RETURN:
 * NOTE:
 *******************************************************************/
static int
vpnc_ovpn_sync_account(const VPNC_PROFILE *prof)
{
	char attr[64];

	if (!prof || prof->protocol != VPNC_PROTO_OVPN)
		return -1;

	if (prof->basic.username[0] != '\0')
	{
		snprintf(attr, sizeof(attr), "vpn_client%d_username", prof->config.ovpn.ovpn_idx);
		nvram_set(attr, prof->basic.username);
	}

	if (prof->basic.password[0] != '\0')
	{
		snprintf(attr, sizeof(attr), "vpn_client%d_password", prof->config.ovpn.ovpn_idx);
		nvram_set(attr, prof->basic.password);
	}
	return 0;
}

#if 0
static void
vpnc_dump_vpnc_profile(const VPNC_PROFILE *profile)
{
	if (!profile)
		return;
	_dprintf("[%s]<active:%d><vpnc_idx:%d><proto:%d><server:%s><username:%s><password:%s>", __FUNCTION__, profile->active, profile->vpnc_idx, profile->protocol, profile->basic.server, profile->basic.username, profile->basic.password);
	switch (profile->protocol)
	{
	case VPNC_PROTO_PPTP:
		_dprintf("<option:%d>\n", profile->config.pptp.option);
		break;
	case VPNC_PROTO_OVPN:
		_dprintf("<ovpn_idx:%d>\n", profile->config.ovpn.ovpn_idx);
		break;
	case VPNC_PROTO_IPSEC:
	default:
		_dprintf("\n");
		break;
	}
}
#endif

/*******************************************************************
 * NAME: vpnc_get_profile_by_vpnc_id
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2016/12/07
 * DESCRIPTION: Parser the nvram setting and load the VPNC_PROFILE list
 * INPUT:  list: an array to store vpnc profile, list_size: size of list
 * OUTPUT:
 * RETURN:  number of profiles, -1: fialed
 * NOTE:
 *******************************************************************/
static VPNC_PROFILE *vpnc_get_profile_by_vpnc_id(VPNC_PROFILE *list, const int list_size, const int vpnc_id)
{
	int i;
	if (!list || !list_size)
		return NULL;

	for (i = 0; i < list_size; ++i)
	{
		if (list[i].vpnc_idx == vpnc_id)
			return &(list[i]);
	}
	return NULL;
}

/*******************************************************************
 * NAME: vpnc_get_proto_in_profile_by_vpnc_id
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2020/6/11
 * DESCRIPTION: get the protocol of the profile by vpnc_id
 * INPUT:  vpnc_id: number. The index of the vpnc profile
 * OUTPUT:
 * RETURN:  a value in VPNC_PROTO
 * NOTE:
 *******************************************************************/
VPNC_PROTO vpnc_get_proto_in_profile_by_vpnc_id(const int vpnc_id)
{
	VPNC_PROFILE *prof = NULL;
	prof = vpnc_get_profile_by_vpnc_id(vpnc_profile, MAX_VPNC_PROFILE, vpnc_id);
	if (prof)
	{
		return prof->protocol;
	}
	return VPNC_PROTO_UNDEF;
}


#if 0
static void vpnc_dump_dev_policy_list(VPNC_DEV_POLICY *list, const int list_size)
{
	int i;
	if(list)
	{
		_dprintf("[%s]\n", __FUNCTION__);
		for(i = 0; i < list_size; ++i)
		{
			_dprintf("\tactive(%d), src_ip(%s), dst_ip(%s), vpnc_idx(%d))", list[i].active, list[i].src_ip? list[i].src_ip: "", list[i].dst_ip? list[i].dst_ip: "", list[i].vpnc_idx);
#ifdef RTCONFIG_VPN_FUSION_SUPPORT_INTERFACE
			_dprintf(", iif(%s)\n", list[i].iif? list[i].iif: "");
#endif
			_dprintf("\n");
		}
	}
}
#endif

/*******************************************************************
 * NAME: vpnc_get_dev_policy_list
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2016/11/23
 * DESCRIPTION: parser vpnc_dev_policy_list
 * INPUT:  list_size: size of list
 * OUTPUT:  list: an array to store policy
 * RETURN:  number of the list.
 * NOTE:
 *******************************************************************/
int
vpnc_get_dev_policy_list(VPNC_DEV_POLICY *list, const int list_size, const int tmp_flag)
{
	int cnt;
	VPNC_DEV_POLICY *policy = list;

	char *nv, *nvp, *b;
	char *active, *dst_ip, *vpnc_idx;
	char *src_ip;
#ifdef RTCONFIG_VPN_FUSION_SUPPORT_INTERFACE
	char *iif;
#endif

	if (!list || list_size <= 0)
		return 0;

	memset(list, 0, sizeof(VPNC_DEV_POLICY) * list_size);

	/* Protection level per client */
	if (!tmp_flag)
		nv = nvp = strdup(nvram_safe_get("vpnc_dev_policy_list"));
	else
		nv = nvp = strdup(nvram_safe_get("vpnc_dev_policy_list_tmp"));

	cnt = 0;
	while (nv && (b = strsep(&nvp, "<")) != NULL && cnt < list_size)
	{
#ifdef RTCONFIG_VPN_FUSION_SUPPORT_INTERFACE
		if (vstrsep(b, ">", &active, &src_ip, &dst_ip, &vpnc_idx, &iif) < 3)
			continue;

		if (!active || !vpnc_idx || (!src_ip && !iif))
			continue;

		if(iif)
			snprintf(policy->iif, sizeof(policy->iif), "%s", iif);
#else
		if (vstrsep(b, ">", &active, &src_ip, &dst_ip, &vpnc_idx) < 3)
			continue;

		if (!active || !src_ip || !vpnc_idx)
			continue;
#endif
		if(src_ip)
			snprintf(policy->src_ip, sizeof(policy->src_ip), "%s", src_ip);

		policy->active = atoi(active);

		if (dst_ip)
			snprintf(policy->dst_ip, sizeof(policy->dst_ip), "%s", dst_ip);

		policy->vpnc_idx = atoi(vpnc_idx);
		++cnt;
		++policy;
	}
	free(nv);

	// vpnc_dump_dev_policy_list(list, list_size);
	return cnt;
}

/*******************************************************************
 * NAME: vpnc_find_index_by_ifname
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2016/11/25
 * DESCRIPTION: Find vpnc index by interface name
 * INPUT:  vpnc_ifname: interface name
 * OUTPUT:
 * RETURN:  index of vpnc. -1 is not found.
 * NOTE:
 *******************************************************************/
static int
_vpnc_find_index_by_ifname(const char *vpnc_ifname)
{
	int unit;
	if (!vpnc_ifname)
		return -1;

	if (!strncmp(vpnc_ifname, "ppp", 3)) // pptp/l2tp
	{
		return vpnc_ppp_linkunit_by_ifname(vpnc_ifname);
	}
	else if (!strncmp(vpnc_ifname, "tun", 3)) // openvpn
	{
		unit = atoi(vpnc_ifname + 3) - OVPN_CLIENT_BASE;
		return _find_vpnc_idx_by_ovpn_unit(unit);
	}
#ifdef RTCONFIG_WIREGUARD
	else if (!strncmp(vpnc_ifname, WG_CLIENT_IF_PREFIX, 3)) // wireguard
	{
		unit = atoi(vpnc_ifname + 3);
		return find_vpnc_idx_by_wgc_unit(unit);
	}
#endif
	return -1;
}

/*******************************************************************
 * NAME: vpnc_set_policy_by_ifname
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2016/11/25
 * DESCRIPTION: set device policy by vpnc interface name
 * INPUT:  vpnc_ifname: interface name
 *		action: 1: add rules for interface up. 0: remove rule for interface down.
 * OUTPUT:
 * RETURN:  -1: failed, 0: success
 * NOTE:
 *******************************************************************/
int vpnc_set_policy_by_ifname(const char *vpnc_ifname, const int action)
{
	int policy_cnt, vpnc_idx, i;
	VPNC_DEV_POLICY *policy;
	char *lan_ifname = nvram_safe_get("lan_ifname");
	VPNC_DEV_POLICY dev_policy[MAX_DEV_POLICY] = {{0}};
	policy_cnt = vpnc_get_dev_policy_list(dev_policy, MAX_DEV_POLICY, 0);

	if (!vpnc_ifname || !policy_cnt || !lan_ifname || lan_ifname[0] == '\0')
		return -1;

	vpnc_idx = _vpnc_find_index_by_ifname(vpnc_ifname);

	for (i = 0, policy = dev_policy; i < policy_cnt; ++i, ++policy)
	{
		if (policy->active && vpnc_idx == policy->vpnc_idx)
		{
			if (!action) // remove rule
			{
				// Can not support destination ip
				_set_routing_rule(VPNC_ROUTE_DEL, policy);
			}
			else // add value
			{
				// Can not support destination ip
				_set_routing_rule(VPNC_ROUTE_ADD, policy);
			}
#if defined(RTCONFIG_WIREGUARD) && (defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_BCM_502L07P2) || defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_HND_ROUTER_BE_4916))
			if (!strncmp(vpnc_ifname, WG_CLIENT_IF_PREFIX, strlen(WG_CLIENT_IF_PREFIX)))
			{
				hnd_skip_wg_network(action, policy->src_ip);
			}
#endif
		}
	}
	return 0;
}

/*******************************************************************
 * NAME: vpnc_handle_policy_rule
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2016/1/25
 * DESCRIPTION: add/del iptable rule
 * INPUT:  action: 0:delete rule, 1: add rule.
 * 		policy: pointer of structure VPNC_DEV_POLICY
 * OUTPUT:
 * RETURN:  0:success, -1:failed.
 * NOTE:
 *******************************************************************/
int vpnc_handle_policy_rule(const int action, const VPNC_DEV_POLICY *policy)
{
	if (!policy)
	{
		_dprintf("[%s] parameters error!\n", __FUNCTION__);
		return -1;
	}

	if (!action) // delete
	{
		//_dprintf("[%s, %d]remove rule. src_ip=%s, vpnc_idx=%d\n", __FUNCTION__, __LINE__,  src_ip, vpnc_idx);
		if (policy->vpnc_idx != -1)
		{
			_set_routing_rule(VPNC_ROUTE_DEL, policy);
		}
	}
	else // add
	{
		//_dprintf("[%s, %d]add rule. src_ip=%s, vpnc_idx=%d\n", __FUNCTION__, __LINE__, src_ip, vpnc_idx);
		if (policy->vpnc_idx != -1 && _get_vpnc_state(policy->vpnc_idx) == WAN_STATE_CONNECTED)
		{
			_set_routing_rule(VPNC_ROUTE_ADD, policy);
		}
	}

#if defined(RTCONFIG_WIREGUARD) && (defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_BCM_502L07P2) || defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_HND_ROUTER_BE_4916))
	int i;
	for (i = 0; i < vpnc_profile_num; ++i)
	{
		if (vpnc_profile[i].protocol == VPNC_PROTO_WG
		 && vpnc_profile[i].vpnc_idx == policy->vpnc_idx
		) {
			hnd_skip_wg_network(action, policy->src_ip);
			break;
		}
	}
#endif

	return 0;
}

#ifdef RTCONFIG_MULTILAN_CFG
static void _vpnc_ipset_dev_x(const VPNC_DEV_POLICY *policy, int add)
{
	char ipset_name[32] = {0};

	if (!policy)
		return;

	snprintf(ipset_name, sizeof(ipset_name), "%s%d", VPNC_IPSET_PREFIX, policy->vpnc_idx);
	eval("ipset", (add)?"add":"del", ipset_name, policy->src_ip);
}
#endif

/*******************************************************************
 * NAME: vpnc_set_dev_policy_rule
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2017/4/18
 * DESCRIPTION: compare vpnc_dev_policy_list_tmp and vpnc_dev_policy_list_tmp,
 *				remove old rules and add new rules
 * INPUT:
 * OUTPUT:
 * RETURN:  0:success, -1:failed.
 * NOTE:
 *******************************************************************/
int vpnc_set_dev_policy_rule()
{
	int policy_cnt_new, policy_cnt_old, i, j, flag;
	VPNC_DEV_POLICY *policy_ptr;
	VPNC_DEV_POLICY dev_policy_new[MAX_DEV_POLICY] = {{0}}, dev_policy_old[MAX_DEV_POLICY] = {{0}};

	policy_cnt_old = vpnc_get_dev_policy_list(dev_policy_old, MAX_DEV_POLICY, 1);
	policy_cnt_new = vpnc_get_dev_policy_list(dev_policy_new, MAX_DEV_POLICY, 0);

	// check old policy list, if it does not exist in the new list or different, remove the old rule.
	for (i = 0; i < policy_cnt_old; ++i)
	{
		policy_ptr = &dev_policy_old[i];
		if (policy_ptr->active)
		{
			flag = 1;
			for (j = 0; j < policy_cnt_new; ++j)
			{
				if (!memcmp(policy_ptr, &dev_policy_new[j], sizeof(VPNC_DEV_POLICY))) // policy is not changed
				{
					flag = 0;
					break;
				}
			}
			if (flag)
			{
				vpnc_handle_policy_rule(0, policy_ptr);
#ifdef RTCONFIG_MULTILAN_CFG
				_vpnc_ipset_dev_x(policy_ptr, 0);
#endif
			}
		}
	}

	// check new policy list, if it does not exist in the old list, add the rule.
	for (i = 0; i < policy_cnt_new; ++i)
	{
		policy_ptr = &dev_policy_new[i];
		if (policy_ptr->active)
		{
			flag = 1;
			for (j = 0; j < policy_cnt_old; ++j)
			{
				if (!memcmp(policy_ptr, &dev_policy_old[j], sizeof(VPNC_DEV_POLICY))) // policy is not changed
				{
					flag = 0;
					break;
				}
			}
			if (flag)
			{
				vpnc_handle_policy_rule(1, policy_ptr);
#ifdef RTCONFIG_MULTILAN_CFG
				_vpnc_ipset_dev_x(policy_ptr, 1);
#endif
			}
		}
	}
	start_firewall(wan_primary_ifunit(), 0);
	return 0;
}
/*******************************************************************
 * NAME: vpnc_init
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2016/12/07
 * DESCRIPTION: initialize variables
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTE:
 *******************************************************************/
void vpnc_init()
{
	// load profile
	vpnc_profile_num = vpnc_load_profile(vpnc_profile, MAX_VPNC_PROFILE, VPNC_PROFILE_VER1);
	//_dprintf("[%s, %d]vpnc_profile_num=%d\n", __FUNCTION__, __LINE__, vpnc_profile_num);
}

#ifdef RTCONFIG_MULTIWAN_IF
static void _set_mtwan_routing_rule(const VPNC_PROFILE *prof)
{
	int pref = IP_RULE_PREF_VPNC_OVER_MTWAN + prof->vpnc_idx;
	char pref_str[8];
	char table[8];
	int wan_unit;
	// char wan_prefix[16];
	char nvname[32];
	char server[128];
	char server_addrs[512];
	char server_ip[INET6_ADDRSTRLEN];
	char *next = NULL;

	if (prof->wan_idx == 0)
		return;
	if (!is_mtwan_unit(wan_unit = mtwan_get_mapped_unit(prof->wan_idx)))
		return;

	snprintf(pref_str, sizeof(pref_str), "%d", pref);
	remove_ip_rules(pref, 0);
	remove_ip_rules(pref, 1);

	mtwan_get_route_table_id(wan_unit, table, sizeof(table));

	switch(prof->protocol) {
#if 0
		case VPNC_PROTO_PPTP:
		case VPNC_PROTO_L2TP: {
			if (!is_valid_ip4(prof->basic.server))
			{
				resolv_addr4(prof->basic.server, server_ip, sizeof(server_ip));
				if (is_valid_ip4(server_ip))
					strlcpy(prof->basic.server, server_ip, sizeof(prof->basic.server));
			}
			eval("ip", "rule", "add", "to", prof->basic.server, "table", table, "pref", pref_str);

			snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_unit);
			eval("ip", "route", "del", prof->basic.server, "via", nvram_pf_safe_get(wan_prefix, "gateway"), "dev", get_wan_ifname(wan_unit));
			break;
		}
#endif
		case VPNC_PROTO_OVPN: {
			snprintf(nvname, sizeof(nvname), "vpn_client%d_addr", prof->config.ovpn.ovpn_idx);
			nvram_safe_get_r(nvname, server, sizeof(server));
			resolv_addr_all(server, server_addrs, sizeof(server_addrs));
			foreach(server_ip, server_addrs, next)
			{
				eval("ip", is_valid_ip6(server_ip)?"-6":"-4", "rule", "add", "to", server_ip, "iif", "lo", "table", table, "pref", pref_str);
			}
			break;
		}
		case VPNC_PROTO_WG: {
			snprintf(nvname, sizeof(nvname), "wgc%d_ep_addr", prof->config.ovpn.ovpn_idx);
			nvram_safe_get_r(nvname, server, sizeof(server));
			resolv_addr_all(server, server_addrs, sizeof(server_addrs));
			foreach(server_ip, server_addrs, next)
			{
				eval("ip", is_valid_ip6(server_ip)?"-6":"-4", "rule", "add", "to", server_ip, "iif", "lo", "table", table, "pref", pref_str);
			}
			break;
		}
		case VPNC_PROTO_IPSEC: {
			char *nv = NULL, *nvp = NULL, *b = NULL;
			char *vpn_type, *profilename, *remote_gateway_method, *remote_gateway;

			snprintf(nvname, sizeof(nvname), "ipsec_profile_client_%d", prof->config.ipsec.prof_idx);
			nv = nvp = strdup(nvram_safe_get(nvname));
			while (nv && (b = strsep(&nvp, "<")))
			{
				if (vstrsep(b, ">", &vpn_type, &profilename, &remote_gateway_method, &remote_gateway) >= 4) {
					strlcpy(server, remote_gateway, sizeof(server));
					break;
				}
			}
			free(nv);

			resolv_addr4_all(server, server_addrs, sizeof(server_addrs));
			foreach(server_ip, server_addrs, next)
			{
				eval("ip", "rule", "add", "to", server_ip, "iif", "lo", "table", table, "pref", pref_str);
			}
		}
		default:
			break;
	}
}

static void _del_mtwan_routing_rule(const VPNC_PROFILE *prof)
{
	int pref = IP_RULE_PREF_VPNC_OVER_MTWAN + prof->vpnc_idx;
	remove_ip_rules(pref, 0);
	remove_ip_rules(pref, 1);
}
#endif

#ifdef RTCONFIG_MULTILAN_CFG
void _vpnc_ipset_create(int vpnc_idx)
{
	char ipset_name[32] = {0};
	int policy_cnt;
	VPNC_DEV_POLICY dev_policy[MAX_DEV_POLICY] = {{0}};
	int i;

	snprintf(ipset_name, sizeof(ipset_name), "%s%d", VPNC_IPSET_PREFIX, vpnc_idx);
	eval("ipset", "create", ipset_name, "hash:ip");

	policy_cnt = vpnc_get_dev_policy_list(dev_policy, MAX_DEV_POLICY, 0);
	for (i = 0; i < policy_cnt; i++)
	{
		if (dev_policy[i].active && dev_policy[i].vpnc_idx == vpnc_idx)
		{
			eval("ipset", "add", ipset_name, dev_policy[i].src_ip);
		}
	}
}

void _vpnc_ipset_destroy(int vpnc_idx)
{
	char ipset_name[32] = {0};

	snprintf(ipset_name, sizeof(ipset_name), "%s%d", VPNC_IPSET_PREFIX, vpnc_idx);
	eval("ipset", "destroy", ipset_name);
}
#endif

/*******************************************************************
 * NAME: start_vpnc_by_unit
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2016/12/07
 * DESCRIPTION: start to connect the vpnc by profile
 * INPUT:  unit: index of vpnc client list. (NOT vpnc_idx)
 * OUTPUT:
 * RETURN:  0: success, -1: failed
 * NOTE:
 *******************************************************************/
int start_vpnc_by_unit(const int unit)
{
	FILE *fp;
	char options[80], l2tp_conf[128], l2tp_ctrl[128], l2tp_pid[128];
	char *pppd_argv[] = {"/usr/sbin/pppd", "file", options, NULL};
	char tmp[100], prefix[] = "vpnc_", vpnc_prefix[] = "vpncXXXXXXXXXX_", wan_prefix[] = "wanXXXXXXXXXX_";
	char buf[256]; /* although maximum length of pppoe_username/pppoe_passwd is 64. pppd accepts up to 256 characters. */
	mode_t mask;
	int ret = 0;
	VPNC_PROFILE *prof;

	if (unit >= vpnc_profile_num)
		return -1;

	_dprintf("[%s]Start unit(%d)!\n", __FUNCTION__, unit);

	prof = vpnc_profile + unit;

	// vpnc_dump_vpnc_profile(prof);

	// stop if connection exist.
	stop_vpnc_by_unit(unit);

#ifdef RTCONFIG_MULTIWAN_IF
	_set_mtwan_routing_rule(prof);
#endif

#ifdef RTCONFIG_MULTILAN_CFG
	_vpnc_ipset_create(prof->vpnc_idx);
#endif

	// init prefix
	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_primary_ifunit());
	snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", prof->vpnc_idx);

	/* unset vpnc_dut_disc */
	nvram_unset(strlcat_r(vpnc_prefix, "dut_disc", tmp, sizeof(tmp)));

	// init option path
	if (VPNC_PROTO_PPTP == prof->protocol)
		snprintf(options, sizeof(options), "/tmp/ppp/vpnc%d_options.pptp", prof->vpnc_idx);
	else if (VPNC_PROTO_L2TP == prof->protocol)
		snprintf(options, sizeof(options), "/tmp/ppp/vpnc%d_options.l2tp", prof->vpnc_idx);
	else if (VPNC_PROTO_OVPN == prof->protocol)
	{
		_dprintf("[%s]Start to connect OpenVPN(%d).\n", __FUNCTION__, prof->config.ovpn.ovpn_idx);
		vpnc_ovpn_sync_account(prof);
		start_ovpn_client(prof->config.ovpn.ovpn_idx);
		return 0;
	}
#ifdef RTCONFIG_IPSEC
	else if (VPNC_PROTO_IPSEC == prof->protocol)
	{
		_dprintf("[%s]Start to connect IPSec(%d).\n", __FUNCTION__, prof->config.ipsec.prof_idx);
		rc_ipsec_ctrl(0, prof->config.ipsec.prof_idx, 1);
		return 0;
	}
#endif
#ifdef RTCONFIG_WIREGUARD
	else if (VPNC_PROTO_WG == prof->protocol)
	{
		char vpnc_ifname[IFNAMSIZ] = {0};
		_dprintf("[%s]Start to connect WireGuard(%d).\n", __FUNCTION__, prof->config.wg.wg_idx);
		start_wgc(prof->config.wg.wg_idx);
		snprintf(vpnc_ifname, sizeof(vpnc_ifname), "%s%d", WG_CLIENT_IF_PREFIX, prof->config.wg.wg_idx);
		vpnc_up(prof->vpnc_idx, vpnc_ifname);
		return 0;
	}
	else if (VPNC_PROTO_NORDVPN == prof->protocol)
	{
		char cmd[256] = {0};
		_dprintf("[%s]Start to connect NordVPN(%d).\n", __FUNCTION__, prof->config.tpvpn.tpvpn_idx);
		snprintf(cmd, sizeof(cmd), "nordvpn setconf '%s' %d %d &", prof->config.tpvpn.region, prof->config.tpvpn.tpvpn_idx, unit);
		system(cmd);
		return 0;
	}
#endif
	else if (VPNC_PROTO_HMA == prof->protocol)
	{
		char cmd[256] = {0};
		_dprintf("[%s]Start to connect HMA(%d).\n", __FUNCTION__, prof->config.tpvpn.tpvpn_idx);
		snprintf(cmd, sizeof(cmd), "hmavpn setconf '%s' '%s' %d %d &", prof->config.tpvpn.region, prof->config.tpvpn.conntype, prof->config.tpvpn.tpvpn_idx, unit);
		system(cmd);
		return 0;
	}
	else
	{
		_dprintf("[%s]Unknown protocol\n", __FUNCTION__);
		return -1;
	}

	// init vpncX_ state.
	update_vpnc_state(prof->vpnc_idx, WAN_STATE_INITIALIZING, 0);

	// Run PPTP/L2TP
	if (VPNC_PROTO_PPTP == prof->protocol || VPNC_PROTO_L2TP == prof->protocol)
	{
		mask = umask(0000);

		/* Generate options file */
		if (!(fp = fopen(options, "w")))
		{
			perror(options);
			umask(mask);
			return -1;
		}

#ifdef HND_ROUTER
		/* workaround for ppp packets are dropped by fc GRE learning when pptp server / client enabled */
		char wan_proto[16];
		snprintf(wan_proto, sizeof(wan_proto), "%s", nvram_safe_get(strlcat_r(wan_prefix, "proto", tmp, sizeof(tmp))));
		if (nvram_match("fc_disable", "0") &&
			(!strcmp(wan_proto, "pppoe") ||
			 !strcmp(wan_proto, "pptp") ||
			 !strcmp(wan_proto, "l2tp")))
		{
			dbg("[%s, %d] Flow Cache Learning of GRE flows Tunnel: DISABLED, PassThru: ENABLED\n", __FUNCTION__, __LINE__);
			eval("fc", "config", "--gre", "0");
		}
#endif

		umask(mask);

		/* route for pptp/l2tp's server */
		char *wan_ifname = nvram_safe_get(strlcat_r(wan_prefix, "pppoe_ifname", tmp, sizeof(tmp)));
		route_add(wan_ifname, 0, nvram_safe_get(strlcat_r(wan_prefix, "gateway", tmp, sizeof(tmp))), "0.0.0.0", "255.255.255.255");

		// generate ppp profile
		/* do not authenticate peer and do not use eap */
		fprintf(fp, "noauth\n");
		fprintf(fp, "refuse-eap\n");
		fprintf(fp, "user '%s'\n",
				ppp_safe_escape(prof->basic.username, buf, sizeof(buf)));
		fprintf(fp, "password '%s'\n",
				ppp_safe_escape(prof->basic.password, buf, sizeof(buf)));

		if (VPNC_PROTO_PPTP == prof->protocol)
		{
			fprintf(fp, "plugin pptp.so\n");
			fprintf(fp, "pptp_server '%s'\n", prof->basic.server);
			/* see KB Q189595 -- historyless & mtu */
			if (nvram_match(strlcat_r(wan_prefix, "proto", tmp, sizeof(tmp)), "pptp") || nvram_match(strlcat_r(wan_prefix, "proto", tmp, sizeof(tmp)), "l2tp"))
				fprintf(fp, "nomppe-stateful mtu 1300\n");
			else
				fprintf(fp, "nomppe-stateful mtu 1400\n");

			if (VPNC_PPTP_OPT_MPPC == prof->config.pptp.option)
			{
				fprintf(fp, "nomppe nomppc\n");
			}
			else if (VPNC_PPTP_OPT_MPPE40 == prof->config.pptp.option)
			{
				fprintf(fp, "require-mppe\n"
							"require-mppe-40\n");
			}
			else if (VPNC_PPTP_OPT_MPPE56 == prof->config.pptp.option)
			{
				fprintf(fp, "nomppe-40\n"
							"require-mppe\n"
							"require-mppe-56\n");
			}
			else if (VPNC_PPTP_OPT_MPPE128 == prof->config.pptp.option)
			{
				fprintf(fp, "nomppe-40\n"
							"nomppe-56\n"
							"require-mppe\n"
							"require-mppe-128\n");
			}
			else if (VPNC_PPTP_OPT_AUTO == prof->config.pptp.option)
			{
				fprintf(fp, "require-mppe-40\n"
							"require-mppe-56\n"
							"require-mppe-128\n");
			}
		}
		else
		{
			fprintf(fp, "nomppe nomppc\n");

			if (nvram_match(strlcat_r(wan_prefix, "proto", tmp, sizeof(tmp)), "pptp") || nvram_match(strlcat_r(wan_prefix, "proto", tmp, sizeof(tmp)), "l2tp"))
				fprintf(fp, "mtu 1300\n");
			else
				fprintf(fp, "mtu 1400\n");
		}

		if (VPNC_PROTO_L2TP != prof->protocol)
		{
			ret = nvram_get_int(strlcat_r(prefix, "pppoe_idletime", tmp, sizeof(tmp)));
			if (ret && nvram_get_int(strlcat_r(prefix, "pppoe_demand", tmp, sizeof(tmp))))
			{
				fprintf(fp, "idle %d ", ret);
				if (nvram_invmatch(strlcat_r(prefix, "pppoe_txonly_x", tmp, sizeof(tmp)), "0"))
					fprintf(fp, "tx_only ");
				fprintf(fp, "demand\n");
			}
			fprintf(fp, "persist\n");
		}

		fprintf(fp, "holdoff %d\n", nvram_get_int(strlcat_r(prefix, "pppoe_holdoff", tmp, sizeof(tmp))) ?: 10);
		fprintf(fp, "maxfail %d\n", nvram_get_int(strlcat_r(prefix, "pppoe_maxfail", tmp, sizeof(tmp))));

		if (nvram_invmatch(strlcat_r(prefix, "dnsenable_x", tmp, sizeof(tmp)), "0"))
			fprintf(fp, "usepeerdns\n");

		fprintf(fp, "ipcp-accept-remote ipcp-accept-local noipdefault\n");
		fprintf(fp, "ktune\n");

		/* pppoe set these options automatically */
		/* looks like pptp also likes them */
		fprintf(fp, "default-asyncmap nopcomp noaccomp\n");

		/* pppoe disables "vj bsdcomp deflate" automagically */
		/* ccp should still be enabled - mppe/mppc requires this */
		fprintf(fp, "novj nobsdcomp nodeflate\n");

		/* echo failures */
		fprintf(fp, "lcp-echo-interval 6\n");
		fprintf(fp, "lcp-echo-failure 10\n");

		/* pptp has Echo Request/Reply, l2tp has Hello packets */
		if (VPNC_PROTO_PPTP == prof->protocol || VPNC_PROTO_L2TP == prof->protocol)
			fprintf(fp, "lcp-echo-adaptive\n");

		fprintf(fp, "unit %d\n", prof->vpnc_idx);
		fprintf(fp, "linkname vpn%d\n", prof->vpnc_idx);
		fprintf(fp, "ip-up-script %s\n", "/tmp/ppp/vpnc-ip-up");
		fprintf(fp, "ip-down-script %s\n", "/tmp/ppp/vpnc-ip-down");
		fprintf(fp, "ip-pre-up-script %s\n", "/tmp/ppp/vpnc-ip-pre-up");
		fprintf(fp, "auth-fail-script %s\n", "/tmp/ppp/vpnc-auth-fail");

		/* user specific options */
		fprintf(fp, "%s\n",
				nvram_safe_get(strlcat_r(prefix, "pppoe_options_x", tmp, sizeof(tmp))));

		fclose(fp);

		if (VPNC_PROTO_L2TP == prof->protocol)
		{
			snprintf(l2tp_conf, sizeof(l2tp_conf), L2TP_VPNC_CONF, prof->vpnc_idx);
			snprintf(l2tp_ctrl, sizeof(l2tp_ctrl), L2TP_VPNC_CTRL, prof->vpnc_idx);
			snprintf(l2tp_pid, sizeof(l2tp_pid), L2TP_VPNC_PID, prof->vpnc_idx);

			// generate l2tp profile
			if (!(fp = fopen(l2tp_conf, "w")))
			{
				perror(l2tp_conf);
				return -1;
			}

			fprintf(fp, "# automagically generated\n"
						"global\n\n"
						"load-handler \"sync-pppd.so\"\n"
						"load-handler \"cmd.so\"\n\n"
						"section sync-pppd\n\n"
						"lac-pppd-opts \"file %s\"\n\n"
						"section peer\n"
						"port 1701\n"
						"peername %s\n"
						"hostname %s\n"
						"lac-handler sync-pppd\n"
						"persist yes\n"
						"maxfail %d\n"
						"holdoff %d\n"
						"hide-avps no\n"
						"section cmd\n"
						"socket-path "
						"%s"
						"\n\n",
					options,
					prof->basic.server,
					nvram_invmatch(strlcat_r(prefix, "hostname", tmp, sizeof(tmp)), "") ? nvram_safe_get(strlcat_r(prefix, "hostname", tmp, sizeof(tmp))) : "localhost",
					nvram_get_int(strlcat_r(prefix, "pppoe_maxfail", tmp, sizeof(tmp))) ?: 32767,
					nvram_get_int(strlcat_r(prefix, "pppoe_holdoff", tmp, sizeof(tmp))) ?: 10, l2tp_ctrl);

			fclose(fp);

			/* launch l2tp */
			eval("/usr/sbin/l2tpd", "-c", l2tp_conf, "-p", l2tp_pid);

			ret = 3;
			do
			{
				_dprintf("%s: wait l2tpd up at %d seconds...\n", __FUNCTION__, ret);
				usleep(1000 * 1000);
			} while (!pids("l2tpd") && ret--);

			/* start-session */
			ret = eval("/usr/sbin/l2tp-control", "-s", l2tp_ctrl, "start-session 0.0.0.0");

			/* pppd sync nodetach noaccomp nobsdcomp nodeflate */
			/* nopcomp novj novjccomp file /tmp/ppp/options.l2tp */
		}
		else
		{
			ret = _eval(pppd_argv, NULL, 0, NULL); // launch pppd
		}
	}
	update_vpnc_state(prof->vpnc_idx, WAN_STATE_CONNECTING, 0);

	return ret;
}

/*******************************************************************
 * NAME: stop_vpnc_by_unit
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2016/12/07
 * DESCRIPTION: stop connecting the vpnc by profile
 * INPUT:  unit: index of vpnc client list. (NOT vpnc_idx)
 * OUTPUT:
 * RETURN:  0: success, -1: failed
 * NOTE:
 *******************************************************************/
int stop_vpnc_by_unit(const int unit)
{
	VPNC_PROFILE *prof;
	char pidfile[sizeof("/var/run/ppp-vpnXXXXXXXXXX.pid")];
	char l2tp_pid[128];
	char tmp[100], vpnc_prefix[] = "vpncXXXXX_";

	if (unit >= vpnc_profile_num)
		return -1;

	prof = vpnc_profile + unit;

#ifdef RTCONFIG_TUNNEL
	if (nvram_get_int("vpnc_default_wan") == prof->vpnc_idx)
		stop_aae_sip_conn(1);
#endif

	snprintf(pidfile, sizeof(pidfile), "/var/run/ppp-vpn%d.pid", prof->vpnc_idx);

	snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", prof->vpnc_idx);

	/* Reset the state of vpnc_dut_disc */
	nvram_set_int(strlcat_r(vpnc_prefix, "dut_disc", tmp, sizeof(tmp)), prof->vpnc_idx);

	if (VPNC_PROTO_PPTP == prof->protocol || VPNC_PROTO_L2TP == prof->protocol)
	{
		update_vpnc_state(prof->vpnc_idx, WAN_STATE_STOPPING, 0);
		if (VPNC_PROTO_L2TP == prof->protocol)
		{
			// stop l2tp
			snprintf(l2tp_pid, sizeof(l2tp_pid), L2TP_VPNC_PID, prof->vpnc_idx);
			if (check_if_file_exist(l2tp_pid))
			{
				kill_pidfile_tk(l2tp_pid);
				usleep(1000 * 10000);
			}
		}
		/* Stop pppd */
		if (kill_pidfile_s(pidfile, SIGHUP) == 0 &&
			kill_pidfile_s(pidfile, SIGTERM) == 0)
		{
			usleep(3000 * 1000);
			kill_pidfile_tk(pidfile);
		}
#ifdef HND_ROUTER
		/* workaround for ppp packets are dropped by fc GRE learning when pptp server / client enabled */
		if (nvram_match("fc_disable", "0"))
			eval("fc", "config", "--gre", "1");
#endif
	}
	else if (VPNC_PROTO_OVPN == prof->protocol)
	{
		_dprintf("[%s]Stop OpenVPN(%d).\n", __FUNCTION__, prof->config.ovpn.ovpn_idx);
		stop_ovpn_client(prof->config.ovpn.ovpn_idx);
	}
#ifdef RTCONFIG_WIREGUARD
	else if (VPNC_PROTO_WG == prof->protocol)
	{
		char vpnc_ifname[IFNAMSIZ] = {0};
		_dprintf("[%s]Stop WireGuard(%d).\n", __FUNCTION__, prof->config.wg.wg_idx);
		stop_wgc(prof->config.wg.wg_idx);
		snprintf(vpnc_ifname, sizeof(vpnc_ifname), "%s%d", WG_CLIENT_IF_PREFIX, prof->config.wg.wg_idx);
		vpnc_down(prof->vpnc_idx, vpnc_ifname);
#ifdef RTCONFIG_NORDVPN
		char wgc_prefix[16] = {0};
		snprintf(wgc_prefix, sizeof(wgc_prefix), "%s%d_", WG_CLIENT_NVRAM_PREFIX, prof->config.wg.wg_idx);
		if (!prof->active && !strcmp(nvram_pf_safe_get(wgc_prefix, "tp"), TPVPN_PSZ_NORDVPN))
		{
			_dprintf("[%s]Clear NordVPN region setting.\n", __FUNCTION__);
			nvram_pf_set(wgc_prefix, "tp", "");
			nvram_pf_set(wgc_prefix, "tp_region", "");
		}
#endif
	}
	else if (VPNC_PROTO_NORDVPN == prof->protocol)
	{
		_dprintf("[%s]Stop NordVPN(%d).\n", __FUNCTION__, prof->config.tpvpn.tpvpn_idx);
		stop_wgc(prof->config.tpvpn.tpvpn_idx);
	}
#endif
#ifdef RTCONFIG_IPSEC
	else if (VPNC_PROTO_IPSEC == prof->protocol)
	{
		_dprintf("[%s]Stop IPSec(%d).\n", __FUNCTION__, prof->config.ipsec.prof_idx);
		rc_ipsec_ctrl(0, prof->config.ipsec.prof_idx, 0);
	}
#endif

#ifdef RTCONFIG_MULTIWAN_IF
	_del_mtwan_routing_rule(prof);
#endif

#ifdef RTCONFIG_MULTILAN_CFG
	_vpnc_ipset_destroy(prof->vpnc_idx);
#endif

	return 0;
}

/*******************************************************************
 * NAME: _clean_routing_table
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2017/2/7
 * DESCRIPTION: set multipath routing table
 * INPUT:  cmd: 1:add, 0:delete.  vpnc_id: index of vpnc profile
 * OUTPUT:
 * RETURN:  0: success, -1: failed
 * NOTE:
 *******************************************************************/
static int _clean_routing_table(const int vpnc_id)
{
	FILE *fp = NULL;
	char tmp[256], tmp2[256];

	// delete all rules in vpnc routing table
	snprintf(tmp, sizeof(tmp), "ip route show table %d > /tmp/route_tmp", vpnc_id);
	system(tmp);
	fp = fopen("/tmp/route_tmp", "r");
	if (fp)
	{
		while (fgets(tmp2, sizeof(tmp2), fp))
		{
			char *ptr = strchr(tmp2, '\n');
			if (ptr)
			{
				*ptr = '\0';
			}
			snprintf(tmp, sizeof(tmp), "ip route del %s table %d", tmp2, vpnc_id);
			system(tmp);
		}
		fclose(fp);
		unlink("/tmp/route_tmp");
	}
	else
	{
		_dprintf("[%s]Can not get route table %d\n", __FUNCTION__, vpnc_id);
		unlink("/tmp/route_tmp");
		return -1;
	}
	return 0;
}

static int _set_routing_table(const int cmd, const int vpnc_id)
{
	char tmp[256], tmp2[256], tag[32];
	char prefix[] = "vpncXXXXX_", id_str[16], wan_prefix[] = "wanXXXXXX_";
	char *route_network = NULL, *route_netmask = NULL, *route_gateway = NULL, *route_metric = NULL;
	VPNC_PROFILE *prof = NULL;
	FILE *fp;
	int cnt, i;
	struct in_addr network, netmask;

	snprintf(tag, sizeof(tag), "vpnc%d", vpnc_id);

	//_dprintf("[%s, %d]cmd=%d, vpnc_id=%d\n", __FUNCTION__, __LINE__, cmd, vpnc_id);

	// get protocol
	prof = vpnc_get_profile_by_vpnc_id(vpnc_profile, MAX_VPNC_PROFILE, vpnc_id);

	if (!prof)
	{
		_dprintf("[%s]Can not get vpnc profile(%d)\n", __FUNCTION__, vpnc_id);
		return -1;
	}

	if (vpnc_id >= VPNC_UNIT_BASIC) // VPNC
	{
		snprintf(id_str, sizeof(id_str), "%d", IP_ROUTE_TABLE_ID_VPNC_BASE + vpnc_id);
		snprintf(prefix, sizeof(prefix), "vpnc%d_", vpnc_id);
	}
	else // internet
	{
		_dprintf("[%s]Invalid vpnc id(%d)\n", __FUNCTION__, vpnc_id);
		return -1;
	}

	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_primary_ifunit());

	_clean_routing_table(vpnc_id);

	if (cmd)
	{
		// get main route table and set to vpnc route table
		system("ip route show table main> /tmp/route_tmp");
		fp = fopen("/tmp/route_tmp", "r");
		if (fp)
		{
			while (fgets(tmp2, sizeof(tmp2), fp))
			{
				char *ptr = strchr(tmp2, '\n');
				if (ptr)
				{
					*ptr = '\0';
				}
				if (!strncmp(tmp2, "default", 7) && (prof->protocol == VPNC_PROTO_PPTP || prof->protocol == VPNC_PROTO_L2TP))
				{
					snprintf(tmp, sizeof(tmp), "ip route add %s metric 1 table %d", tmp2, vpnc_id);
				}
				else
					snprintf(tmp, sizeof(tmp), "ip route add %s table %d", tmp2, vpnc_id);
				system(tmp);
			}
			fclose(fp);
			unlink("/tmp/route_tmp");
		}
		else
		{
			_dprintf("[%s]Can not get main routing table\n", __FUNCTION__);
			unlink("/tmp/route_tmp");
			return -1;
		}

		// set vpnc route table
			if (prof->protocol == VPNC_PROTO_PPTP || prof->protocol == VPNC_PROTO_L2TP)
			{
				// set default routing
				//				eval("ip", "route", "add", "default", "via", nvram_safe_get(strlcat_r(prefix, "gateway", tmp, sizeof(tmp))),
				//					"dev", nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), "table", id_str);
				eval("ip", "route", "add", "0.0.0.0/1", "via", nvram_safe_get(strlcat_r(prefix, "gateway", tmp, sizeof(tmp))),
					 "dev", nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), "table", id_str);
				eval("ip", "route", "add", "128.0.0.0/1", "via", nvram_safe_get(strlcat_r(prefix, "gateway", tmp, sizeof(tmp))),
					 "dev", nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), "table", id_str);
			}
			else if (prof->protocol == VPNC_PROTO_OVPN)
			{
				// set default routing
				eval("ip", "route", "add", "0.0.0.0/1", "via", nvram_safe_get(strlcat_r(prefix, "gateway", tmp, sizeof(tmp))),
					 "dev", nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), "table", id_str);
				eval("ip", "route", "add", "128.0.0.0/1", "via", nvram_safe_get(strlcat_r(prefix, "gateway", tmp, sizeof(tmp))),
					 "dev", nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), "table", id_str);

				// set remote routing rule
				eval("ip", "route", "add", nvram_safe_get("trusted_ip"),
					 "via", nvram_safe_get(strlcat_r(wan_prefix, "gateway", tmp, sizeof(tmp))),
					 "dev", nvram_safe_get(strlcat_r(wan_prefix, "gw_ifname", tmp, sizeof(tmp))),
					 "table", id_str);

				// set route
				cnt = nvram_get_int(strlcat_r(prefix, "route_num", tmp, sizeof(tmp)));
				for (i = 0; i < cnt; ++i)
				{
					snprintf(tmp2, sizeof(tmp2), "route_network_%d", i);
					route_network = nvram_safe_get(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)));
					snprintf(tmp2, sizeof(tmp2), "route_netmask_%d", i);
					route_netmask = nvram_safe_get(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)));
					snprintf(tmp2, sizeof(tmp2), "route_gateway_%d", i);
					route_gateway = nvram_safe_get(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)));
					snprintf(tmp2, sizeof(tmp2), "route_metric_%d", i);
					route_metric = nvram_safe_get(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)));

					if (inet_pton(AF_INET, route_network, &network) > 0 && inet_pton(AF_INET, route_netmask, &netmask) > 0)
					{
						_dprintf("route: %s / %s\n", route_network, route_netmask);
						if (current_route(network.s_addr, netmask.s_addr) || current_addr(network.s_addr))
						{
							if (network.s_addr & netmask.s_addr)
							{
								_dprintf("route conflict: %s/%s\n", route_network, route_netmask);
								logmessage_normal(tag, "WARNING: Ignore conflicted routing rule: %s %s gw %s", route_network, route_netmask, route_gateway);
							}
							else
							{
								_dprintf("Detect default gateway: %s/%s\n", route_network, route_netmask);
								// logmessage_normal(tag, "WARNING: Replace default vpn gateway by using 0.0.0.0/1 and 128.0.0.0/1");
							}
							update_ovpn_vpnc_state(vpnc_id, WAN_STATE_STOPPED, WAN_STOPPED_REASON_IPGATEWAY_CONFLICT);
						}
						else
						{
							int cidr = convert_subnet_mask_to_cidr(route_netmask);

							if (cidr < 32 && cidr > 0)
								snprintf(tmp2, sizeof(tmp2), "%s/%d", route_network, cidr);
							else
								snprintf(tmp2, sizeof(tmp2), "%s", route_network);

							if (route_metric && route_metric[0] != '\0')
								eval("ip", "route", "add", tmp2, "via", route_gateway, "dev", nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), "metric", route_metric, "table", id_str);
							else
								eval("ip", "route", "add", tmp2, "via", route_gateway, "dev", nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), "table", id_str);
					}
				}
			}
		}
	}
	return 0;
}

/*******************************************************************
 * NAME: _set_routing_rule
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2017/2/7
 * DESCRIPTION: set multipath routing table
 * INPUT:  cmd: VPNC_ROUTE_CMD.  vpnc_id: index of vpnc profile
 * OUTPUT:
 * RETURN:  0: success, -1: failed
 * NOTE:
 *******************************************************************/
static int _set_routing_rule(const VPNC_ROUTE_CMD cmd, const VPNC_DEV_POLICY *policy)
{
	char cmd_str[8], id_str[8], priority[8];

	if (!policy)
		return -1;

	//_dprintf("[%s, %d]<%d><%s><%d>\n", __FUNCTION__, __LINE__, cmd, source_ip, vpnc_id);
	snprintf(cmd_str, sizeof(cmd_str), "%s", (cmd == VPNC_ROUTE_DEL) ? "del" : "add");
	snprintf(priority, sizeof(priority), "%d", IP_RULE_PREF_VPNC_POLICY_CLIENT);

	if (policy->vpnc_idx >= VPNC_UNIT_BASIC)
		snprintf(id_str, sizeof(id_str), "%d", IP_ROUTE_TABLE_ID_VPNC_BASE + policy->vpnc_idx );
	else
		snprintf(id_str, sizeof(id_str), "main");

	if(policy->src_ip[0] != '\0')
	{
		eval("ip", "rule", cmd_str, "from", (char*)policy->src_ip, "table", id_str, "priority", priority);
	}

#ifdef RTCONFIG_VPN_FUSION_SUPPORT_INTERFACE
	snprintf(priority, sizeof(priority), "%d", IP_RULE_PREF_VPNC_POLICY_IF + policy->vpnc_idx * 3);

	if(policy->iif[0] != '\0')
	{
		eval("ip", "rule", cmd_str, "iif", (char*)policy->iif, "table", id_str, "priority", priority);
	}
#endif
	return 0;
}

/*******************************************************************
 * NAME: _set_default_routing_table
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2017/2/7
 * DESCRIPTION: set multipath routing table
 * INPUT:  cmd: VPNC_ROUTE_CMD.  vpnc_id: index of vpnc profile
 * OUTPUT:
 * RETURN:  0: success, -1: failed
 * NOTE:
 *******************************************************************/
static int _set_default_routing_table(const VPNC_ROUTE_CMD cmd, const int table_id)
{
#if !defined(RTCONFIG_MULTILAN_CFG)
	char id_str[8];
	char tmp[256], *ifname, default_priority[256];
	FILE *fp;
	char word[256], *next;
	int i;
	int policy_cnt;
	VPNC_DEV_POLICY dev_policy[MAX_DEV_POLICY] = {{0}};
	policy_cnt = vpnc_get_dev_policy_list(dev_policy, MAX_DEV_POLICY, 0);
	int flag;
#endif

	remove_ip_rules(IP_RULE_PREF_DEFAULT_CONN, 0);
	remove_ip_rules(IP_RULE_PREF_DEFAULT_CONN, 1);

#ifdef RTCONFIG_MULTILAN_CFG
	update_sdn_by_vpnc(table_id);
#else
	snprintf(id_str, sizeof(id_str), "%d", IP_ROUTE_TABLE_ID_VPNC_BASE + table_id);
	snprintf(default_priority, sizeof(default_priority), "%d", IP_RULE_PREF_DEFAULT_CONN);

	if (VPNC_ROUTE_ADD == cmd && table_id > 0)
	{
		//set routeing rule for lan ifname
		ifname = nvram_safe_get("lan_ifname");
		eval("ip", "rule", "add", "iif", ifname, "table", id_str, "priority", default_priority);
#ifdef RTCONFIG_IPV6
		if(ipv6_enabled())
		{
			VPNC_PROTO proto;
			vpnc_init();
			proto = vpnc_get_proto_in_profile_by_vpnc_id(table_id);
			if (proto == VPNC_PROTO_OVPN || proto == VPNC_PROTO_WG)
				eval("ip", "-6", "rule", "add", "iif", ifname, "table", id_str, "priority", default_priority);
		}
#endif

		//set routing rule for guest network interface
		if(nvram_match("wgn_enabled", "1"))
		{
			foreach(word, nvram_safe_get("wgn_ifnames"), next)
			{
#if defined(RTCONFIG_VPN_FUSION_SUPPORT_INTERFACE)
				flag = 0;

				for(i = 0; i < policy_cnt; ++i)
				{
					if(dev_policy[i].active && !strcmp(dev_policy[i].iif, word))
					{
						flag = 1;
						break;
					}
				}
				if(flag)
					continue;
#endif
				eval("ip", "rule", "add", "iif", word, "table", id_str, "priority", default_priority);
			}
		}
	}
#endif
	return 0;
}

/*******************************************************************
 * NAME: update_default_routing_rule
 * AUTHOR: Andy Chiu
 * CREATE DATE: 2022/6/23
 * DESCRIPTION: update the default ip rule
 * INPUT:
 * OUTPUT:
 * RETURN:  0: success, -1: failed
 * NOTE:
 *******************************************************************/
int update_default_routing_rule()
{
	int unit;

#if defined(RTCONFIG_VPN_FUSION_MERLIN)
	return 0;
#endif
	unit = nvram_get_int("vpnc_default_wan");
	_set_default_routing_table(VPNC_ROUTE_ADD, unit);
	return 0;
}

int clean_routing_rule_by_vpnc_idx(const int vpnc_idx)
{
	VPNC_DEV_POLICY dev_policy[MAX_DEV_POLICY] = {{0}};
	int policy_cnt = 0, i, cnt = 0;

	policy_cnt = vpnc_get_dev_policy_list(dev_policy, MAX_DEV_POLICY, 0);

	for (i = 0; i < policy_cnt; ++i)
	{
		if (dev_policy[i].active && dev_policy[i].vpnc_idx == vpnc_idx)
		{
			_set_routing_rule(VPNC_ROUTE_DEL, &dev_policy[i]);
			++cnt;
		}
	}
	return cnt;
}

int vpnc_set_internet_policy(const int action)
{
	VPNC_DEV_POLICY dev_policy[MAX_DEV_POLICY] = {{0}};
	int policy_cnt = 0, i;

	policy_cnt = vpnc_get_dev_policy_list(dev_policy, MAX_DEV_POLICY, 0);

	for (i = 0; i < policy_cnt; ++i)
	{
		if (dev_policy[i].active && dev_policy[i].vpnc_idx == 0)
		{
			_set_routing_rule(action ? VPNC_ROUTE_ADD : VPNC_ROUTE_DEL, &dev_policy[i]);
		}
	}
	return 0;
}

int clean_vpnc_setting_value(const int vpnc_idx)
{
	char prefix[] = "vpncXXXX_", tmp[128];

	//_dprintf("[%s, %d]idx=%d\n", __FUNCTION__, __LINE__, vpnc_idx);
	snprintf(prefix, sizeof(prefix), "vpnc%d_", vpnc_idx);

	// unset general settings, dns, gateway, ifname, ipaddr.
	nvram_unset(strlcat_r(prefix, "dns", tmp, sizeof(tmp)));
	nvram_unset(strlcat_r(prefix, "gateway", tmp, sizeof(tmp)));
	nvram_unset(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)));
	nvram_unset(strlcat_r(prefix, "ipaddr", tmp, sizeof(tmp)));

	return 0;
}

int is_vpnc_dns_active()
{
	char prefix[] = "vpncXXXX_", tmp[128];
	int default_wan = 0;
	VPNC_PROFILE *prof = NULL;

	vpnc_init();

	// only need to check whether the default wan has dns setting.
	default_wan = nvram_get_int("vpnc_default_wan");
	prof = vpnc_get_profile_by_vpnc_id(vpnc_profile, MAX_VPNC_PROFILE, default_wan);

	if (prof && prof->active)
	{
		snprintf(prefix, sizeof(prefix), "vpnc%d_", default_wan);
		if (nvram_get_int(strlcat_r(prefix, "state_t", tmp, sizeof(tmp))) == WAN_STATE_CONNECTED &&
			nvram_invmatch(strlcat_r(prefix, "dns", tmp, sizeof(tmp)), ""))
		{
			return 1;
		}
	}
	return 0;
}

void reset_vpnc_state(void)
{
	int i;
	char vpnc_prefix[] = "vpncXXXX_";

	for (i = VPNC_UNIT_BASIC; i < VPNC_UNIT_BASIC + MAX_VPNC_PROFILE; i++)
	{
		snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", i);
		nvram_pf_unset(vpnc_prefix, "state_t");
		nvram_pf_unset(vpnc_prefix, "sbstate_t");
	}
}

static int _get_vpnc_state(const int vpnc_idx)
{
	char vpnc_prefix[] = "vpncXXXX_", tmp[128];

	if (vpnc_idx) // vpn client
		snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", vpnc_idx);
	else
		snprintf(vpnc_prefix, sizeof(vpnc_prefix), "wan%d_", wan_primary_ifunit()); // internet

	return nvram_get_int(strlcat_r(vpnc_prefix, "state_t", tmp, sizeof(tmp)));
}

int write_vpn_fusion_nat(FILE *fp, const char *lan_ip)
{
	VPNC_DEV_POLICY dev_policy[MAX_DEV_POLICY] = {{0}};
	int policy_cnt = 0, i;
	char vpnc_prefix[] = "vpncXXXX_", tmp[128], *p, *dns;
	VPNC_PROTO proto;

	if (!fp || !lan_ip)
		return -1;

	vpnc_init();
	if (!vpnc_profile_num)
		return 0;

	// write dns forwarding rule
	fprintf(fp, "-A PREROUTING -p udp -d %s --dport 53 -j VPN_FUSION\n", lan_ip);

	// write vpnc nat rules
	for (i = VPNC_UNIT_BASIC; i < VPNC_UNIT_BASIC + MAX_VPNC_PROFILE; i++)
	{
		snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", i);

		if (nvram_match(strlcat_r(vpnc_prefix, "state_t", tmp, sizeof(tmp)), "2")) // connected
		{
			proto = vpnc_get_proto_in_profile_by_vpnc_id(i);
			if (proto == VPNC_PROTO_PPTP || proto == VPNC_PROTO_L2TP)
			{
				// set PREROUTING
				fprintf(fp, "-I PREROUTING -d %s -j VSERVER\n", nvram_safe_get(strlcat_r(vpnc_prefix, "ipaddr", tmp, sizeof(tmp))));

				// set POSTROUTING
				fprintf(fp, "-I POSTROUTING -o %s ! -s %s -j MASQUERADE\n", nvram_safe_get(strlcat_r(vpnc_prefix, "ifname", tmp, sizeof(tmp))), nvram_safe_get(strlcat_r(vpnc_prefix, "ipaddr", tmp, sizeof(tmp))));
			}
		}
	}

	// write policy rules
	policy_cnt = vpnc_get_dev_policy_list(dev_policy, MAX_DEV_POLICY, 0);

	for (i = 0; i < policy_cnt; ++i)
	{
		if (dev_policy[i].active)
		{
			snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", dev_policy[i].vpnc_idx);

			// check vpnc connetced
			if (nvram_match(strlcat_r(vpnc_prefix, "state_t", tmp, sizeof(tmp)), "2")) // connected
			{
				dns = strdup(nvram_safe_get(strlcat_r(vpnc_prefix, "dns", tmp, sizeof(tmp))));
				if (dns)
				{
					if (dns[0] != '\0') // has dns
					{
						// only redirect to the first dns server.
						p = strchr(dns, ' ');
						if (p)
							*p = '\0';
						fprintf(fp, "-A VPN_FUSION -p udp -s %s -d %s --dport 53 -j DNAT --to-destination %s\n",
								dev_policy[i].src_ip, lan_ip, dns);
					}
					SAFE_FREE(dns);
				}
			}
		}
	}
	return 0;
}

int write_vpn_fusion_filter(FILE *fp, const char *lan_ip)
{
	char vpnc_prefix[] = "vpncXXXX_", tmp[128];
	VPNC_PROTO proto;
	char lan_if[IFNAMSIZ + 1];
	int i;
#ifdef RTCONFIG_VPN_FUSION_SUPPORT_INTERFACE
	int j, policy_cnt;
	VPNC_DEV_POLICY dev_policy[MAX_DEV_POLICY] = {{0}};
	policy_cnt = vpnc_get_dev_policy_list(dev_policy, MAX_DEV_POLICY, 0);
#endif

	if (!fp || !lan_ip)
		return -1;

	vpnc_init();
	if (!vpnc_profile_num)
		return 0;

	// get lan interface name
	snprintf(lan_if, sizeof(lan_if), "%s", nvram_safe_get("lan_ifname"));

	// write vpnc filter rules
	for (i = VPNC_UNIT_BASIC; i < VPNC_UNIT_BASIC + MAX_VPNC_PROFILE; i++)
	{
		snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", i);

		if (nvram_match(strlcat_r(vpnc_prefix, "state_t", tmp, sizeof(tmp)), "2")) // connected
		{
			proto = vpnc_get_proto_in_profile_by_vpnc_id(i);
			if (proto == VPNC_PROTO_PPTP || proto == VPNC_PROTO_L2TP)
			{
#if defined(RTCONFIG_VPN_FUSION_SUPPORT_INTERFACE)
			for(j = 0; j < policy_cnt; ++j)
			{
				if(dev_policy[j].active && dev_policy[j].vpnc_idx == i && dev_policy[j].iif[0] != '\0')
				{
					fprintf(fp, "-A VPNCF -o %s -i %s -j ACCEPT\n", nvram_safe_get(strlcat_r(vpnc_prefix, "ifname", tmp, sizeof(tmp))), dev_policy[j].iif);
				}
			}
#endif
				// set FORWARD
				fprintf(fp, "-I FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");
				//fprintf(fp, "-A FORWARD -o %s ! -i %s -j DROP\n", nvram_safe_get(strlcat_r(vpnc_prefix, "ifname", tmp, sizeof(tmp))), lan_if);
#ifdef RTCONFIG_MULTILAN_CFG
				fprintf(fp, "-A VPNCF -i %s -j DROP\n", nvram_safe_get(strlcat_r(vpnc_prefix, "ifname", tmp, sizeof(tmp))));
				fprintf(fp, "-A VPNCF -o %s -j DROP\n", nvram_safe_get(strlcat_r(vpnc_prefix, "ifname", tmp, sizeof(tmp))));
#else
				fprintf(fp, "-A VPNCF -i %s -j ACCEPT\n", nvram_safe_get(strlcat_r(vpnc_prefix, "ifname", tmp, sizeof(tmp))));
#endif
			}
		}
	}
	return 0;
}

int write_vpn_fusion_mangle()
{
	int i;
	char vpnc_prefix[] = "vpncXXXX_", tmp[128];
	VPNC_PROTO proto;

	vpnc_init();
	if (!vpnc_profile_num)
		return 0;

	// write vpnc nat rules
	for (i = VPNC_UNIT_BASIC; i < VPNC_UNIT_BASIC + MAX_VPNC_PROFILE; i++)
	{
		snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", i);

		if (nvram_match(strlcat_r(vpnc_prefix, "state_t", tmp, sizeof(tmp)), "2")) // connected
		{
			proto = vpnc_get_proto_in_profile_by_vpnc_id(i);
			if (proto == VPNC_PROTO_PPTP || proto == VPNC_PROTO_L2TP)
			{
				// set FORWARD in mangle table
				eval("iptables", "-t", "mangle", "-A", "FORWARD", "-p", "tcp", "-m", "state", "--state", "NEW", "-j", "MARK", "--set-mark", "0x01/0x7");
				break;
			}
		}
	}
	return 0;
}

int _gen_vpnc_resolv_conf(const int vpnc_idx)
{
	FILE *fp;
	char resolv_file[128], vpnc_prefix[] = "vpncXXXXXX_", *vpnc_dns, *next, tmp[128];

	snprintf(resolv_file, sizeof(resolv_file), vpnc_resolv_path, vpnc_idx);
	snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", vpnc_idx);

	vpnc_dns = nvram_safe_get(strlcat_r(vpnc_prefix, "dns", tmp, sizeof(tmp)));
	if (vpnc_dns[0] == '\0')
	{
		unlink(resolv_file);
		return -1;
	}

	fp = fopen(resolv_file, "w");
	if (fp)
	{
		foreach (tmp, vpnc_dns, next)
		{
			fprintf(fp, "server=%s\n", tmp);
		}
		fclose(fp);
		chmod(resolv_file, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
		return 0;
	}
	return -1;
}

#ifdef RTCONFIG_VPN_FUSION_SUPPORT_INTERFACE
int vpnc_set_iif_routing_rule(const int vpnc_idx, const char *br_ifname)
{
	char priority[8], id_str[8];

	if (!br_ifname || br_ifname[0] == '\0')
		return -1;

	snprintf(priority, sizeof(priority), "%d", IP_RULE_PREF_VPNC_POLICY_IF + vpnc_idx * 3);
	snprintf(id_str, sizeof(id_str), "%d", IP_ROUTE_TABLE_ID_VPNC_BASE + vpnc_idx);

	//TODO: the ip rule for sdn would be conflict on wan and vpnc.
	if (vpnc_idx != 0)
	{
		// remove old rule
		eval("ip", "rule", "del", "iif", (char*)br_ifname);
		// add new rule
		eval("ip", "rule", "add", "iif", (char*)br_ifname, "table", id_str, "priority", priority);
	}
	return 0;
}
#endif

static int _set_network_routing_rule(const VPNC_ROUTE_CMD cmd, const int vpnc_idx)
{
	char tmp[64] = {0}, vpnc_prefix[] = "vpncXXXX_", *vpnc_dns, *next;
	char cmd_str[8], id_str[8], priority[8];
	int i;
#ifdef RTCONFIG_VPN_FUSION_SUPPORT_INTERFACE
	int policy_cnt;
	VPNC_DEV_POLICY dev_policy[MAX_DEV_POLICY] = {{0}};
	policy_cnt = vpnc_get_dev_policy_list(dev_policy, MAX_DEV_POLICY, 0);
#endif

	snprintf(cmd_str, sizeof(cmd_str), "%s", (cmd == VPNC_ROUTE_DEL) ? "del" : "add");
	snprintf(id_str, sizeof(id_str), "%d", IP_ROUTE_TABLE_ID_VPNC_BASE + vpnc_idx);
	snprintf(priority, sizeof(priority), "%d", IP_RULE_PREF_VPNC_POLICY_IF + vpnc_idx * 3);

#ifdef RTCONFIG_MULTILAN_CFG
	update_sdn_by_vpnc(vpnc_idx);
#endif
#if defined(RTCONFIG_VPN_FUSION_SUPPORT_INTERFACE)
			for(i = 0; i < policy_cnt; ++i)
			{
				if(dev_policy[i].active && dev_policy[i].vpnc_idx == vpnc_idx && dev_policy[i].iif[0] != '\0')
				{
					if (cmd == VPNC_ROUTE_ADD)
					{
						eval("ip", "rule", "del", "priority", priority);
					}
					eval("ip", "rule", cmd_str, "iif", dev_policy[i].iif, "table", id_str, "priority", priority);
				}
			}
#endif

	// set dns
	snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", vpnc_idx);
	vpnc_dns = nvram_safe_get(strlcat_r(vpnc_prefix, "dns", tmp, sizeof(tmp)));

	if (vpnc_dns[0] != '\0') // write dns setting of VPN client to resolv.dnsmasq
	{
		//_dprintf("[%s, %d]<%s>\n", __FUNCTION__, __LINE__, vpnc_dns);
		i = 1;
		foreach (tmp, vpnc_dns, next)
		{
			snprintf(priority, sizeof(priority), "%d", IP_RULE_PREF_VPNC_POLICY_IF + vpnc_idx * 3 + i);
			//_dprintf("[%s, %d]<%s><%s>\n", __FUNCTION__, __LINE__, priority, tmp);
			eval("ip", is_valid_ip6(tmp)?"-6":"-4", "rule", cmd_str, "iif", "lo", "to", tmp, "table", id_str, "priority", priority);
			++i;
		}
	}
	return 0;
}

#ifdef RTCONFIG_MULTILAN_CFG
void vpnc_set_iptables_rule_by_sdn(MTLAN_T *pmtl, size_t mtl_sz, int restart_all_sdn)
{
	char vpnc_prefix[8] = {0};
	char vpnc_ifname[IFNAMSIZ] = {0};
	char fpath[128] = {0};
	FILE* fp;
	int vpnc_idx;
	int i;
	VPNC_PROTO proto;
	char ipset_name[32] = {0};

	if (restart_all_sdn)
	{
		eval("iptables", "-F", "VPNCF");
	}

	vpnc_init();
	for (vpnc_idx = VPNC_UNIT_BASIC; vpnc_idx < VPNC_UNIT_BASIC + MAX_VPNC_PROFILE; vpnc_idx++)
	{
		snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", vpnc_idx);
		strlcpy(vpnc_ifname, nvram_pf_safe_get(vpnc_prefix, "ifname"), sizeof(vpnc_ifname));

		if (nvram_pf_get_int(vpnc_prefix, "state_t") == WAN_STATE_CONNECTED)
		{
			proto = vpnc_get_proto_in_profile_by_vpnc_id(vpnc_idx);
			if (proto == VPNC_PROTO_PPTP || proto == VPNC_PROTO_L2TP || proto == VPNC_PROTO_IPSEC)
			{
				// restore default drop rule
				if (restart_all_sdn)
				{
					snprintf(ipset_name, sizeof(ipset_name), "%s%d", VPNC_IPSET_PREFIX, vpnc_idx);
					eval("iptables", "-I", "VPNCF", "-m", "set", "--match-set", ipset_name, "dst", "-i", vpnc_ifname, "-j", "ACCEPT");
					eval("iptables", "-I", "VPNCF", "-m", "set", "--match-set", ipset_name, "src", "-o", vpnc_ifname, "-j", "ACCEPT");
					eval("iptables", "-A", "VPNCF", "-i", vpnc_ifname, "-j", "DROP");
					eval("iptables", "-A", "VPNCF", "-o", vpnc_ifname, "-j", "DROP");
				}
				for (i = 0; i < mtl_sz; i++)
				{
					// delete old rules for specific sdn
					snprintf(fpath, sizeof(fpath), "%s/vpnc_%s_sdn%d.sh", sdn_dir, vpnc_ifname, pmtl[i].sdn_t.sdn_idx);
					if (f_exists(fpath) && vpnc_idx != pmtl[i].sdn_t.vpnc_idx)
					{
						eval("sed", "-i", "s/-I/-D/", fpath);
						eval("sed", "-i", "s/-A/-D/", fpath);
						eval(fpath);
						unlink(fpath);
					}
					// add new rules for specific sdn
					if (!f_exists(fpath) && vpnc_idx == pmtl[i].sdn_t.vpnc_idx)
					{
						fp = fopen(fpath, "w");
						if (fp)
						{
							fprintf(fp, "#!/bin/sh\n\n");
							fprintf(fp, "iptables -I VPNCF -i %s -o %s -j ACCEPT\n", vpnc_ifname, pmtl[i].nw_t.ifname);
							fprintf(fp, "iptables -I VPNCF -o %s -i %s -j ACCEPT\n", vpnc_ifname, pmtl[i].nw_t.ifname);
							fclose(fp);
							chmod(fpath, S_IRUSR|S_IWUSR|S_IXUSR);
							eval(fpath);
						}
					}
					else if (restart_all_sdn)
					{
						eval(fpath);
					}
				}
			}
		}
	}
}

void vpnc_set_iptables_rule_by_sdn_remove(MTLAN_T *pmtl, size_t mtl_sz)
{
	int vpnc_idx;
	char vpnc_prefix[8] = {0};
	char vpnc_ifname[IFNAMSIZ] = {0};
	VPNC_PROTO proto;
	char fpath[128] = {0};
	int i;

	vpnc_init();
	for (vpnc_idx = VPNC_UNIT_BASIC; vpnc_idx < VPNC_UNIT_BASIC + MAX_VPNC_PROFILE; vpnc_idx++)
	{
		snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", vpnc_idx);
		strlcpy(vpnc_ifname, nvram_pf_safe_get(vpnc_prefix, "ifname"), sizeof(vpnc_ifname));

		if (nvram_pf_get_int(vpnc_prefix, "state_t") != WAN_STATE_CONNECTED)
			continue;

		proto = vpnc_get_proto_in_profile_by_vpnc_id(vpnc_idx);
		if (proto != VPNC_PROTO_PPTP && proto != VPNC_PROTO_L2TP && proto != VPNC_PROTO_IPSEC)
			continue;

		/// remove rule if binded with the removed SDN.
		for (i = 0; i < mtl_sz; i++)
		{
			snprintf(fpath, sizeof(fpath), "%s/vpnc_%s_sdn%d.sh", sdn_dir, vpnc_ifname, pmtl[i].sdn_t.sdn_idx);
			if (f_exists(fpath) && vpnc_idx == pmtl[i].sdn_t.vpnc_idx)
			{
				eval("sed", "-i", "s/-I/-D/", fpath);
				eval("sed", "-i", "s/-A/-D/", fpath);
				eval(fpath);
				unlink(fpath);
			}
		}
	}
}
#endif

#if defined(RTCONFIG_SW_BTN)
int find_vpnc_unit_by_idx(int idx)
{
	int unit=-1;
        VPNC_PROFILE *prof;
	if(idx < 0 || idx >= (VPNC_UNIT_BASIC + MAX_VPNC_PROFILE))
		return -1;
 	vpnc_init();
	if (!vpnc_profile_num) 
		return -1;
	for(unit=0;unit<vpnc_profile_num;unit++)
	{
		prof =vpnc_profile + unit;
		if((prof->vpnc_idx < (VPNC_UNIT_BASIC + MAX_VPNC_PROFILE)) && prof->vpnc_idx == idx )
			return unit;
	}	
	return -1;
}

int find_vpnc_idx_by_unit(int unit)
{
        VPNC_PROFILE *prof;
 	vpnc_init();
	if (unit >= vpnc_profile_num || !vpnc_profile_num) 
		return -1;
	prof =vpnc_profile + unit;
	if(prof->vpnc_idx < (VPNC_UNIT_BASIC + MAX_VPNC_PROFILE))
		return prof->vpnc_idx;

	return -1;
}

int set_vpnc_active_by_unit(int unit,char *onoff)
{
	char *desc, *proto, *server, *username, *passwd, *active, *vpnc_idx;
        char *region, *conntype;
        char *wan_idx, *caller, *tunnel;
        int i;
	char buf[1024], *nv = NULL, *nvp = NULL, *b = NULL;
	 
	nv = nvp = strdup(nvram_safe_get("vpnc_clientlist"));
	i = 0;
	memset(buf, 0, sizeof(buf));

	while (nv && (b = strsep(&nvp, "<")) != NULL && i <= MAX_VPNC_PROFILE) {
		if (vstrsep(b, ">", &desc, &proto, &server, &username, &passwd, &active, &vpnc_idx, &region, &conntype, &tunnel, &wan_idx, &caller) < 4)
			 continue;
			
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
	       			i? "<%s>%s>%s>%s>%s>%s>%s>%s>%s>%s>%s>%s": "%s>%s>%s>%s>%s>%s>%s>%s>%s>%s>%s>%s",
       				desc? desc: "",
				proto? proto: "",
				server? server: "",
			       	username? username: "",
	       			passwd? passwd: "",
				(i!=unit)? active:onoff,
				vpnc_idx? vpnc_idx: "",
				region? region: "",
		       		conntype? conntype: "",
       				tunnel? tunnel: "0",
				wan_idx? wan_idx: "0",
				caller? caller: "");
		 ++i;
	 }
	
	 if(nv) free(nv);
	 nvram_set("vpnc_clientlist", buf);

	 //init
	 vpnc_init();
	 if(atoi(onoff))
	 {
		// _dprintf("[%s]restart vpnc %d\n", __FUNCTION__, unit);
	   	 stop_vpnc_by_unit(unit);
	   	 start_vpnc_by_unit(unit);
	 }
	 else
	 {
		 //_dprintf("[%s]stop vpnc %d\n", __FUNCTION__, unit);
	   	 stop_vpnc_by_unit(unit);
	 }
#if defined(RTCONFIG_MT798X) || defined(RTCONFIG_MT799X)
	 reinit_hwnat(-1);
#endif
#if defined(RTCONFIG_BWDPI) && (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
                /* It's a workaround for QCA / MTK platform due to accelerator / module / vpn can't work together */
	 start_dpi_engine_service();
#endif
	return 0;
}	



#endif
