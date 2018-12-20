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

#include "vpnc_fusion.h"


VPNC_PROFILE vpnc_profile[MAX_VPNC_PROFILE] = {0};

int vpnc_profile_num = 0;

static int vpnc_get_dev_policy_list(VPNC_DEV_POLICY *list, const int list_size, const int tmp_flag);
int vpnc_set_policy_by_ifname(const char *vpnc_ifname, const int action);
int stop_vpnc_by_unit(const int unit);
int set_routing_table(const int cmd, const int vpnc_id);
#if 0
static void vpnc_dump_vpnc_profile(const VPNC_PROFILE *profile);
#endif
static VPNC_PROFILE* vpnc_get_profile_by_vpnc_id(VPNC_PROFILE *list, const int list_size, const int vpnc_id);
int set_default_routing_table(const VPNC_ROUTE_CMD cmd, const int table_id);
int set_routing_rule(const VPNC_ROUTE_CMD cmd, const char *source_ip, const int vpnc_id);
int clean_routing_rule_by_vpnc_idx(const int vpnc_idx);
int clean_vpnc_setting_value(const int vpnc_idx);


int vpnc_pppstatus(const int unit)
{
	char statusfile[sizeof("/var/run/ppp-vpnXXXXXXXXXX.status")];

	snprintf(statusfile, sizeof(statusfile), "/var/run/ppp-vpn%d.status", unit);
	return _pppstatus(statusfile);
}

int
start_vpnc(void)	//start up all active vpnc profile
{
	int i;
	
	//init vpnc profile
	vpnc_init();

	for(i = 0; i < vpnc_profile_num; ++i)
	{
		if(vpnc_profile[i].active)
			start_vpnc_by_unit(i);
	}
	return 0;
}

void
stop_vpnc(void)	//start up all active vpnc profile
{
	int i;
	
	//init vpnc profile
	vpnc_init();

	for(i = 0; i < vpnc_profile_num; ++i)
	{
		if(vpnc_profile[i].active)
			stop_vpnc_by_unit(i);
	}
	
	return;
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

	if(nvram_match("vpnc_default_wan", tmp))
	{
		if(up)
			set_default_routing_table(VPNC_ROUTE_ADD, unit);
		else
			set_default_routing_table(VPNC_ROUTE_DEL, unit);
	}
	return 0;
}

/*******************************************************************
* NAME: change_default_wan
* AUTHOR: Andy Chiu
* CREATE DATE: 2017/1/24
* DESCRIPTION: when web ui change default wan, call this function to handle default wan
* INPUT:  
* OUTPUT:  
* RETURN:  0:success, -1:failed
* NOTE:
*******************************************************************/
int change_default_wan()
{
	int default_wan_new;

	//get default_wan
	default_wan_new = nvram_get_int("vpnc_default_wan_tmp");

	set_default_routing_table(VPNC_ROUTE_ADD, default_wan_new);
	
	nvram_set_int("vpnc_default_wan", default_wan_new);
	return 0;
}

int vpnc_ppp_linkunit(const char *linkname)
{
	if(!linkname)
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
	if(!ifname)
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

	//create prefix
	snprintf(prefix, sizeof(prefix), "vpnc%d_", vpnc_idx);

	nvram_set_int(strlcat_r(prefix, "state_t", tmp, sizeof(tmp)), state);
	nvram_set_int(strlcat_r(prefix, "sbstate_t", tmp, sizeof(tmp)), 0);

	if (state == WAN_STATE_STOPPED) {
		// Save Stopped Reason
		// keep ip info if it is stopped from connected
		nvram_set_int(strlcat_r(prefix, "sbstate_t", tmp, sizeof(tmp)), reason);
	}
	else if(state == WAN_STATE_STOPPING) {
		snprintf(tmp, sizeof(tmp), "/var/run/ppp-vpn%d.status", vpnc_idx);
		unlink(tmp);
	}
}

int vpnc_update_resolvconf(const int unit)
{
	FILE *fp;
#ifdef NORESOLV /* dnsmasq uses no resolv.conf */
	FILE *fp_servers;
#endif
	char tmp[100], prefix[sizeof("vpncXXXXXXXXXX_")];
	char *wan_dns, *next;
	int lock;
#ifdef RTCONFIG_YANDEXDNS
	int yadns_mode = nvram_get_int("yadns_enable_x") ? nvram_get_int("yadns_mode") : YADNS_DISABLED;
#endif

	lock = file_lock("resolv");

	if (!(fp = fopen("/tmp/resolv.conf", "w+"))) {
		perror("/tmp/resolv.conf");
		goto error;
	}
#ifdef NORESOLV /* dnsmasq uses no resolv.conf */
#ifdef RTCONFIG_YANDEXDNS
	if (yadns_mode != YADNS_DISABLED) {
		/* keep yandex.dns servers */
		fp_servers = NULL;
	} else
#endif
	if (!(fp_servers = fopen("/tmp/resolv.dnsmasq", "w+"))) {
		perror("/tmp/resolv.dnsmasq");
		fclose(fp);
		goto error;
	}
#endif

	snprintf(prefix, sizeof(prefix), "vpnc%d_", unit);

	wan_dns = nvram_safe_get(strcat_r(prefix, "dns", tmp));
	foreach(tmp, wan_dns, next) {
		fprintf(fp, "nameserver %s\n", tmp);
#ifdef NORESOLV /* dnsmasq uses no resolv.conf */
#ifdef RTCONFIG_YANDEXDNS
		if (yadns_mode != YADNS_DISABLED)
			continue;
#endif
		fprintf(fp_servers, "server=%s\n", tmp);
#endif
	}

	fclose(fp);
#ifdef NORESOLV /* dnsmasq uses no resolv.conf */
	if (fp_servers)
		fclose(fp_servers);
#endif
	file_unlock(lock);

	reload_dnsmasq();

	return 0;

error:
	file_unlock(lock);
	return -1;
}

void vpnc_add_firewall_rule(const int unit, const char *vpnc_ifname)
{
	char tmp[100], vpnc_prefix[] = "vpncXXXX_", wan_prefix[] = "wanXXXXXXXXXX_";
	char *wan_proto = NULL;
	char lan_if[IFNAMSIZ+1];

	if(!vpnc_ifname)
		return;

	//generate prefix, vpncX_
	snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", unit);

	//get lan interface name
	snprintf(lan_if, sizeof(lan_if), "%s", nvram_safe_get("lan_ifname"));

	if (check_if_file_exist(strlcat_r("/tmp/ppp/link.", vpnc_ifname, tmp, sizeof(tmp))))
	{
		snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_primary_ifunit());
		wan_proto = nvram_safe_get(strlcat_r(wan_prefix, "proto", tmp, sizeof(tmp)));
		if (!strcmp(wan_proto, "dhcp") || !strcmp(wan_proto, "static"))
			//eval("iptables", "-I", "FORWARD", "-p", "tcp", "--syn", "-j", "TCPMSS", "--clamp-mss-to-pmtu");
			eval("iptables", "-I", "FORWARD", "-p", "tcp", "--tcp-flags", "SYN,RST", "SYN", "-j", "TCPMSS", "--clamp-mss-to-pmtu");
#ifdef RTCONFIG_BCMARM
		else	/* mark tcp connection to bypass CTF */
#ifdef HND_ROUTER
			if (nvram_match("fc_disable", "0") && nvram_match("fc_pt_war", "1"))
#else
			if (nvram_match("ctf_disable", "0"))
#endif
			eval("iptables", "-t", "mangle", "-A", "FORWARD", "-p", "tcp", 
				"-m", "state", "--state", "NEW","-j", "MARK", "--set-mark", "0x01/0x7");
#endif

		eval("iptables", "-A", "FORWARD", "-o", (char*)vpnc_ifname, "!", "-i", lan_if, "-j", "DROP");
		eval("iptables", "-t", "nat", "-I", "PREROUTING", "-d", 
			nvram_safe_get(strlcat_r(vpnc_prefix, "ipaddr", tmp, sizeof(tmp))), "-j", "VSERVER");
		eval("iptables", "-t", "nat", "-I", "POSTROUTING", "-o", 
			(char*)vpnc_ifname, "!", "-s", nvram_safe_get(strlcat_r(vpnc_prefix, "ipaddr", tmp, sizeof(tmp))), "-j", "MASQUERADE");

	}
	//Add dev policy
	vpnc_set_policy_by_ifname(vpnc_ifname, 1);
}

void
vpnc_up(const int unit, const char *vpnc_ifname)
{
	char tmp[100], prefix[] = "vpnc_", wan_prefix[] = "wanXXXXXXXXXX_", vpnc_prefix[] = "vpncXXXX_";
	char *wan_ifname = NULL, *wan_proto = NULL;
	int default_wan;	
	
	if(!vpnc_ifname)
		return;

	_dprintf("[%s, %d]unit=%d, vpnc_ifname=%s\n", __FUNCTION__, __LINE__, unit, vpnc_ifname);
	
	//get vpnc_prefix and wan_prefix
	snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", unit);
	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_primary_ifunit());

	//get wan interface name
	wan_proto = nvram_safe_get(strlcat_r(wan_prefix, "proto", tmp, sizeof(tmp)));

	if (!strcmp(wan_proto, "dhcp") || !strcmp(wan_proto, "static"))
		wan_ifname = nvram_safe_get(strlcat_r(wan_prefix, "ifname", tmp, sizeof(tmp)));
	else
		wan_ifname = nvram_safe_get(strlcat_r(wan_prefix, "pppoe_ifname", tmp, sizeof(tmp)));

	//get default_wan
	default_wan = nvram_get_int("default_wan");	

	if(default_wan == unit)
	{
		/* Reset default gateway route via PPPoE interface */
		if (!strcmp(wan_proto, "dhcp") || !strcmp(wan_proto, "static")) {			
			route_del(wan_ifname, 0, "0.0.0.0", nvram_safe_get(strlcat_r(wan_prefix, "gateway", tmp, sizeof(tmp))), "0.0.0.0");
			route_add(wan_ifname, 2, "0.0.0.0", nvram_safe_get(strlcat_r(wan_prefix, "gateway", tmp, sizeof(tmp))), "0.0.0.0");
		}
		else if (!strcmp(wan_proto, "pppoe") || !strcmp(wan_proto, "pptp") || !strcmp(wan_proto,  "l2tp"))
		{
			char *wan_xgateway = nvram_safe_get(strlcat_r(wan_prefix, "xgateway", tmp, sizeof(tmp)));
			route_del(wan_ifname, 0, "0.0.0.0", nvram_safe_get(strlcat_r(wan_prefix, "gateway", tmp, sizeof(tmp))), "0.0.0.0");
			route_add(wan_ifname, 2, "0.0.0.0", nvram_safe_get(strlcat_r(wan_prefix, "gateway", tmp, sizeof(tmp))), "0.0.0.0");

			if (strlen(wan_xgateway) > 0 && strcmp(wan_xgateway, "0.0.0.0")) {
				char *wan_xifname =  nvram_safe_get(strlcat_r(wan_prefix, "ifname", tmp, sizeof(tmp)));
				route_del(wan_xifname, 2, "0.0.0.0", nvram_safe_get(strlcat_r(wan_prefix, "xgateway", tmp, sizeof(tmp))), "0.0.0.0");
				route_add(wan_xifname, 3, "0.0.0.0", nvram_safe_get(strlcat_r(wan_prefix, "xgateway", tmp, sizeof(tmp))), "0.0.0.0");
			}
		}
		/* Add the default gateway of VPN client */
		route_add((char*)vpnc_ifname, 0, "0.0.0.0", nvram_safe_get(strlcat_r(prefix, "gateway", tmp, sizeof(tmp))), "0.0.0.0");
		/* Remove route to the gateway - no longer needed */
		route_del((char*)vpnc_ifname, 0, nvram_safe_get(strlcat_r(prefix, "gateway", tmp, sizeof(tmp))), NULL, "255.255.255.255");
	}
	
	/* Add dns servers to resolv.conf */
	if (nvram_invmatch(strcat_r(vpnc_prefix, "dns", tmp), ""))
		vpnc_update_resolvconf(unit);

	/* Add firewall rules for VPN client */
	vpnc_add_firewall_rule(unit, vpnc_ifname);

	update_vpnc_state(unit, WAN_STATE_CONNECTED, 0);	
}

int
vpnc_ipup_main(int argc, char **argv)
{
	FILE *fp;
	char *vpnc_ifname = safe_getenv("IFNAME");
	char *vpnc_linkname = safe_getenv("LINKNAME");
	char tmp[100], prefix[] = "vpnc_", vpnc_prefix[] ="vpncXXXX_";
	char buf[256], *value;
	int unit;

	_dprintf("%s():: %s\n", __FUNCTION__, argv[0]);

	/* Get unit from LINKNAME: vpn[UNIT] */
	if ((unit = vpnc_ppp_linkunit(vpnc_linkname)) < 0)
		return 0;

	snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", unit);

	_dprintf("%s: unit=%d ifname=%s\n", __FUNCTION__, unit, vpnc_ifname);

	/* Touch connection file */
	if (!(fp = fopen(strlcat_r("/tmp/ppp/link.", vpnc_ifname, tmp, sizeof(tmp)), "a"))) {
		perror(tmp);
		return errno;
	}
	fclose(fp);

	if ((value = getenv("IPLOCAL"))) {
		if (nvram_invmatch(strlcat_r(vpnc_prefix, "ipaddr", tmp, sizeof(tmp)), value))
			ifconfig(vpnc_ifname, IFUP, "0.0.0.0", NULL);
		_ifconfig(vpnc_ifname, IFUP, value, "255.255.255.255", getenv("IPREMOTE"), 0);
		nvram_set(strlcat_r(vpnc_prefix, "ipaddr", tmp, sizeof(tmp)), value);
		//nvram_set(strlcat_r(vpnc_prefix, "netmask", tmp, sizeof(tmp)), "255.255.255.255");
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

	vpnc_up(unit, vpnc_ifname);

	//add routing table
#ifdef USE_MULTIPATH_ROUTE_TABLE	
	set_routing_table(1, unit);
#endif

	//set up default wan
	change_default_wan_as_vpnc_updown(unit, 1);

	_dprintf("%s:: done\n", __FUNCTION__);
	return 0;
}

void vpnc_del_firewall_rule(const int vpnc_idx, const char *vpnc_ifname)
{
	char tmp[100], prefix[] = "vpncXXXX_", wan_prefix[] = "wanXXXXXXXXXX_";
	char *wan_proto = NULL;
	char lan_if[IFNAMSIZ+1];
	
	if(!vpnc_ifname)
		return;

	snprintf(prefix, sizeof(prefix), "vpnc%d_", vpnc_idx);

	strcpy(lan_if, nvram_safe_get("lan_ifname"));

	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_primary_ifunit());
	wan_proto = nvram_safe_get(strlcat_r(wan_prefix, "proto", tmp, sizeof(tmp)));
	if (!strcmp(wan_proto, "dhcp") || !strcmp(wan_proto, "static"))
		//eval("iptables", "-D", "FORWARD", "-p", "tcp", "--syn", "-j", "TCPMSS", "--clamp-mss-to-pmtu");
		eval("iptables", "-D", "FORWARD", "-p", "tcp", "--tcp-flags", "SYN,RST", "SYN", "-j", "TCPMSS", "--clamp-mss-to-pmtu");
#ifdef RTCONFIG_BCMARM
	else
#ifdef HND_ROUTER
		if (nvram_match("fc_disable", "0") && nvram_match("fc_pt_war", "1"))
#else
		if (nvram_match("ctf_disable", "0"))
#endif
		eval("iptables", "-t", "mangle", "-D", "FORWARD", "-p", "tcp", 
			"-m", "state", "--state", "NEW","-j", "MARK", "--set-mark", "0x01/0x7");
#endif

	eval("iptables", "-D", "FORWARD", "-o", (char*)vpnc_ifname, "!", "-i", lan_if, "-j", "DROP");
	eval("iptables", "-t", "nat", "-D", "PREROUTING", "-d", 
		nvram_safe_get(strlcat_r(prefix, "ipaddr", tmp, sizeof(tmp))), "-j", "VSERVER");
	eval("iptables", "-t", "nat", "-D", "POSTROUTING", "-o", 
		(char*)vpnc_ifname, "!", "-s", nvram_safe_get(strlcat_r(prefix, "ipaddr", tmp, sizeof(tmp))), "-j", "MASQUERADE");

	//delete device policies
	vpnc_set_policy_by_ifname(vpnc_ifname, 0);
	
}

void
vpnc_down(char *vpnc_ifname)
{
	char tmp[100], wan_prefix[] = "wanXXXXXXXXXX_", vpnc_prefix[] = "vpncXXXX_";
	char *wan_ifname = NULL, *wan_proto = NULL;
	int unit, default_wan;

	if(!vpnc_ifname)
		return;

	unit = vpnc_ppp_linkunit_by_ifname(vpnc_ifname);

	if(unit == -1)
		return;

	//get default_wan
	default_wan = nvram_get_int("default_wan");
	
	//init prefix
	snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", unit);
	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_primary_ifunit());

	///get wan interface name
	wan_proto = nvram_safe_get(strlcat_r(wan_prefix, "proto", tmp, sizeof(tmp)));

	if (!strcmp(wan_proto, "dhcp") || !strcmp(wan_proto, "static"))
		wan_ifname = nvram_safe_get(strlcat_r(wan_prefix, "ifname", tmp, sizeof(tmp)));
	else
		wan_ifname = nvram_safe_get(strlcat_r(wan_prefix, "pppoe_ifname", tmp, sizeof(tmp)));

#if !defined(CONFIG_BCMWL5) && defined(RTCONFIG_DUALWAN)
	if (get_nr_wan_unit() > 1 && nvram_match("wans_mode", "lb")) {
		/* Reset default gateway route */
		if (!strcmp(wan_proto, "dhcp") || !strcmp(wan_proto, "static")) {
			route_del(wan_ifname, 2, "0.0.0.0", nvram_pf_safe_get(wan_prefix, "gateway"), "0.0.0.0");
		}
		else if (!strcmp(wan_proto, "pppoe") || !strcmp(wan_proto, "pptp") || !strcmp(wan_proto, "l2tp"))
		{
			char *wan_xgateway = nvram_pf_safe_get(wan_prefix, "xgateway");

			route_del(wan_ifname, 2, "0.0.0.0", nvram_pf_safe_get(wan_prefix, "gateway"), "0.0.0.0");
			if (strlen(wan_xgateway) > 0 && strcmp(wan_xgateway, "0.0.0.0")) {
				char *wan_xifname = nvram_pf_safe_get(wan_prefix, "ifname");

				route_del(wan_xifname, 3, "0.0.0.0", nvram_pf_safe_get(wan_prefix, "xgateway"), "0.0.0.0");
			}

			/* Delete route to pptp/l2tp's server */
			if (nvram_pf_get_int(vpnc_prefix, "dut_disc") && strcmp(wan_proto, "pppoe"))
				route_del(wan_ifname, 0, nvram_pf_safe_get(wan_prefix, "gateway"), "0.0.0.0", "255.255.255.255");
		}

		/* default route via default gateway */
		add_multi_routes();
	} else {
#endif
		if(default_wan != unit)
		{
			/* Delete route to pptp/l2tp's server */
			if (nvram_get_int(strlcat_r(vpnc_prefix, "dut_disc", tmp, sizeof(tmp))) && strcmp(wan_proto, "pppoe"))
				route_del(wan_ifname, 0, nvram_safe_get(strlcat_r(wan_prefix, "gateway", tmp, sizeof(tmp))), "0.0.0.0", "255.255.255.255");			
		}
		else
		{
			/* Reset default gateway route */
			if (!strcmp(wan_proto, "dhcp") || !strcmp(wan_proto, "static")) {
				route_del(wan_ifname, 2, "0.0.0.0", nvram_safe_get(strlcat_r(wan_prefix, "gateway", tmp, sizeof(tmp))), "0.0.0.0");
				route_add(wan_ifname, 0, "0.0.0.0", nvram_safe_get(strlcat_r(wan_prefix, "gateway", tmp, sizeof(tmp))), "0.0.0.0");
			}
			else if (!strcmp(wan_proto, "pppoe") || !strcmp(wan_proto, "pptp") || !strcmp(wan_proto, "l2tp"))
			{
				char *wan_xgateway = nvram_safe_get(strlcat_r(wan_prefix, "xgateway", tmp, sizeof(tmp)));

				route_del(wan_ifname, 2, "0.0.0.0", nvram_safe_get(strlcat_r(wan_prefix, "gateway", tmp, sizeof(tmp))), "0.0.0.0");
				route_add(wan_ifname, 0, "0.0.0.0", nvram_safe_get(strlcat_r(wan_prefix, "gateway", tmp, sizeof(tmp))), "0.0.0.0");

				if (strlen(wan_xgateway) > 0 && strcmp(wan_xgateway, "0.0.0.0")) {
					char *wan_xifname = nvram_safe_get(strlcat_r(wan_prefix, "ifname", tmp, sizeof(tmp)));

					route_del(wan_xifname, 3, "0.0.0.0", nvram_safe_get(strlcat_r(wan_prefix, "xgateway", tmp, sizeof(tmp))), "0.0.0.0");
					route_add(wan_xifname, 2, "0.0.0.0", nvram_safe_get(strlcat_r(wan_prefix, "xgateway", tmp, sizeof(tmp))), "0.0.0.0");
				}

				/* Delete route to pptp/l2tp's server */
				if (nvram_get_int(strlcat_r(vpnc_prefix, "dut_disc", tmp, sizeof(tmp))) && strcmp(wan_proto, "pppoe"))
					route_del(wan_ifname, 0, nvram_safe_get(strlcat_r(wan_prefix, "gateway", tmp, sizeof(tmp))), "0.0.0.0", "255.255.255.255");
			}
		}
#if !defined(CONFIG_BCMWL5) && defined(RTCONFIG_DUALWAN)
	}
#endif

	/* Delete firewall rules for VPN client */
	vpnc_del_firewall_rule(unit, vpnc_ifname);

	//Andy Chiu, 2016/11/25. Delete iptables rule for device policy list.
	//vpnc_set_policy_by_ifname(vpnc_ifname, 0);

}

/*
 * Called when link goes down
 */
int
vpnc_ipdown_main(int argc, char **argv)
{
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

	vpnc_down(vpnc_ifname);

	/* Add dns servers to resolv.conf */
	update_resolvconf();

	unlink(strlcat_r("/tmp/ppp/link.", vpnc_ifname, tmp, sizeof(tmp)));

#ifdef USE_MULTIPATH_ROUTE_TABLE
	//clean routing rule
	clean_routing_rule_by_vpnc_idx(unit);

	//del routing table
	set_routing_table(0, unit);
#endif	

	//set up default wan
	change_default_wan_as_vpnc_updown(unit, 0);

	//clean setting value
	clean_vpnc_setting_value(unit);
	
	_dprintf("%s:: done\n", __FUNCTION__);
	return 0;
}

/*
 * Called when interface comes up
 */
int
vpnc_ippreup_main(int argc, char **argv)
{
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
}

/*
 * Called when link closing with auth fail
 */
int
vpnc_authfail_main(int argc, char **argv)
{
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
	int i;
	VPNC_PROFILE *prof;
	for(i = 0; i < vpnc_profile_num; ++i)
	{
		prof = vpnc_profile + i;
		if(prof->protocol == VPNC_PROTO_OVPN && prof->config.ovpn.ovpn_idx == ovpn_unit)
			return prof->vpnc_idx;
	}
	return -1;
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

	snprintf(nvname, sizeof(nvname), "vpnc%d_dns", _find_vpnc_idx_by_ovpn_unit(ovpn_unit));

	fp = fopen("/etc/openvpn/resolv.conf", "r");
	if (!fp) {
		//_dprintf("read /etc/openvpn/resolv.conf fail\n");
		return;
	}
	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		//nameserver xxx.xxx.xxx.xxx
		if(sscanf (buf,"nameserver %15s", addr) != 1)
		{
			_dprintf("\n=====\nunknown %s\n=====\n", buf);
			continue;
		}

		trim_r(addr);

		old = nvram_get(nvname);
		if (*old)
		{
			size = strlen(old) + strlen(addr) + 2;
			new = malloc(size);
			if(new)
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
	char tmp[32];
	char *vpn_dns, *next;

	snprintf(nvname, sizeof(nvname), "vpnc%d_dns", vpnc_id);
	snprintf(cmd_str, sizeof(cmd_str), "%s", (cmd == VPNC_ROUTE_DEL)? "del": "add");
	snprintf(id_str, sizeof(id_str), "%d", vpnc_id);

	vpn_dns = nvram_safe_get(nvname);
	foreach(tmp, vpn_dns, next) {
		//_dprintf("dns %s\n", tmp);
		eval("ip", "rule", cmd_str, "from", "0/0", "to", tmp, "table", id_str, "priority", VPNC_RULE_PRIORITY);
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
	int unit, vpnc_idx, cnt = 0;
	char *ifname = safe_getenv("dev");
	char *ipaddr = safe_getenv("ifconfig_local");
	char *vpn_gateway = safe_getenv("route_vpn_gateway");
	char *route_network = NULL, *route_netmask = NULL, *route_gateway=NULL, *route_metric=NULL;
	char *remote = NULL;
	
	char tmp[100], prefix[] = "vpncXXXX_", tmp2[100];

	if(argc < 2)
	{
		_dprintf("[%s, %d]parameters error!\n", __FUNCTION__, __LINE__);
		return 0;
	}
	
	unit = atoi(argv[1]);

	// load vpnc profile list	
	vpnc_init();

	vpnc_idx = _find_vpnc_idx_by_ovpn_unit(unit);
		
	if(vpnc_idx != -1 )
	{
		snprintf(prefix, sizeof(prefix), "vpnc%d_", vpnc_idx);
		nvram_set(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), ifname);
		if(ipaddr)
			nvram_set(strlcat_r(prefix, "ipaddr", tmp, sizeof(tmp)), ipaddr);
		if(vpn_gateway)
			nvram_set(strlcat_r(prefix, "gateway", tmp, sizeof(tmp)), vpn_gateway);
		nvram_set(strlcat_r(prefix, "dns", tmp, sizeof(tmp)), "");	//clean dns

		ovpn_up_handler(unit);
		vpnc_ovpn_set_dns(unit);
		update_resolvconf();

		//set route table
		cnt = 0;
		while(1)
		{
			snprintf(tmp, sizeof(tmp), "route_network_%d", cnt + 1);
			route_network = safe_getenv(tmp);
			if(!route_network || route_network[0] == '\0')
				break;
			snprintf(tmp, sizeof(tmp), "route_netmask_%d", cnt + 1);
			route_netmask = safe_getenv(tmp);
			snprintf(tmp, sizeof(tmp), "route_gateway_%d", cnt + 1);
			route_gateway = safe_getenv(tmp);
			snprintf(tmp, sizeof(tmp), "route_metric_%d", cnt + 1);
			route_metric = safe_getenv(tmp);
			
			snprintf(tmp2, sizeof(tmp2), "route_network_%d", cnt);
			nvram_set(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)), route_network);
			snprintf(tmp2, sizeof(tmp2), "route_netmask_%d", cnt);
			nvram_set(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)), route_netmask);
			snprintf(tmp2, sizeof(tmp2), "route_gateway_%d", cnt);
			nvram_set(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)), route_gateway);
			snprintf(tmp2, sizeof(tmp2), "route_metric_%d", cnt);
			nvram_set(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)), route_metric);

			++cnt;			
		}
		
		nvram_set_int(strlcat_r(prefix, "route_num", tmp, sizeof(tmp)), cnt);

		//set remote ip
		cnt = 0;
		while(1)
		{
			snprintf(tmp, sizeof(tmp), "remote_%d", cnt + 1);
			remote = safe_getenv(tmp);
			if(!remote || remote[0] == '\0')
				break;

			snprintf(tmp2, sizeof(tmp2), "remote_%d", cnt);
			nvram_set(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)), remote);

			++cnt;
		}
		nvram_set_int(strlcat_r(prefix, "remote_num", tmp, sizeof(tmp)), cnt);

#ifdef USE_MULTIPATH_ROUTE_TABLE
		//set dns server policy rule
		vpnc_handle_dns_policy_rule(VPNC_ROUTE_ADD, vpnc_idx);
#endif
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
	if(argc < 2)
	{
		_dprintf("[%s, %d]parameters error!\n", __FUNCTION__, __LINE__);
		return 0;
	}

	unit = atoi(argv[1]);

	// load vpnc profile list	
	vpnc_init();

	vpnc_idx = _find_vpnc_idx_by_ovpn_unit(unit);
	if(vpnc_idx != -1 )
	{
		/* override wan_state to get real reason */
		update_vpnc_state(vpnc_idx, WAN_STATE_STOPPED, 0);

		vpnc_down(ifname);

		/* Add dns servers to resolv.conf */
		ovpn_down_handler(unit);
		update_resolvconf();

#ifdef USE_MULTIPATH_ROUTE_TABLE	
		//clean dns server policy rule
		vpnc_handle_dns_policy_rule(VPNC_ROUTE_DEL, vpnc_idx);

		//clean routing rule
		clean_routing_rule_by_vpnc_idx(vpnc_idx);

		set_routing_table(0, vpnc_idx);
#endif

		//set up default wan
		change_default_wan_as_vpnc_updown(vpnc_idx, 0);

		//clean setting value.
		clean_vpnc_setting_value(vpnc_idx);
		
	}
	return 0;
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

	if(argc < 2)
	{
		_dprintf("[%s, %d]parameters error!\n", __FUNCTION__, __LINE__);
		return 0;
	}

	unit = atoi(argv[1]);

	_dprintf("[%s, %d]openvpn unit = %d, ifname = %s\n", __FUNCTION__, __LINE__, unit, ifname);

	// load vpnc profile list	
	vpnc_init();

	vpnc_idx = _find_vpnc_idx_by_ovpn_unit(unit);

	if(vpnc_idx != -1 )
	{	
#ifdef USE_MULTIPATH_ROUTE_TABLE	
		set_routing_table(1, vpnc_idx);
#endif
		vpnc_up(vpnc_idx, ifname);

		//set up default wan
		change_default_wan_as_vpnc_updown(vpnc_idx, 1);
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

	if(!prof || prof->protocol != VPNC_PROTO_OVPN)
		return -1;

	if(prof->basic.username[0] != '\0')
	{
		snprintf(attr, sizeof(attr), "vpn_client%d_username", prof->config.ovpn.ovpn_idx);
		nvram_set(attr, prof->basic.username);
	}

	if(prof->basic.password[0] != '\0')
	{
		snprintf(attr, sizeof(attr), "vpn_client%d_password", prof->config.ovpn.ovpn_idx);
		nvram_set(attr, prof->basic.password);
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
#if 0
static void
vpnc_dump_dev_policy_list(const VPNC_DEV_POLICY *list, const int list_size)
{
	int i;
	const VPNC_DEV_POLICY *policy;
	
	if(!list || list_size <= 0)
		return;

	_dprintf("[%s, %d]Start to dump!\n", __FUNCTION__, __LINE__);
	for(i = 0, policy = list; i < list_size; ++i, ++policy)
	{
#ifdef USE_IPTABLE_ROUTE_TARGE
		_dprintf("[%s, %d]<%d><active:%d><mac:%s><dst_ip:%s><vpnc_idx:%d>\n", __FUNCTION__, __LINE__,
			i, policy->active, policy->mac, policy->dst_ip, policy->vpnc_idx);
#else
		if(policy->src_ip[0] != '\0')
			_dprintf("[%s, %d]<%d><active:%d><src_ip:%s><dst_ip:%s><vpnc_idx:%d>\n", __FUNCTION__, __LINE__,
				i, policy->active, policy->src_ip, policy->dst_ip, policy->vpnc_idx);
#endif
	}
	_dprintf("[%s, %d]End of dump!\n", __FUNCTION__, __LINE__);
}
#endif

#if 0
static void
vpnc_dump_vpnc_profile(const VPNC_PROFILE *profile)
{
	if(!profile)
		return;
	_dprintf("[%s, %d]<active:%d><vpnc_idx:%d><proto:%d><server:%s><username:%s><password:%s>"
		, __FUNCTION__, __LINE__, profile->active, profile->vpnc_idx, profile->protocol, profile->basic.server, profile->basic.username, profile->basic.password);
	switch(profile->protocol)
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
* NAME: vpnc_dump_vpnc_profile_list
* AUTHOR: Andy Chiu
* CREATE DATE: 2016/12/06
* DESCRIPTION: dump VPNC_PROFILE list
* INPUT:  list: an array to store vpnc profile, list_size: size of list 
* OUTPUT:  
* RETURN:  
* NOTE:
*******************************************************************/
#if 0
static void
vpnc_dump_vpnc_profile_list(const VPNC_PROFILE *list, const int list_size)
{
	int i;
	const VPNC_PROFILE *profile;
	
	if(!list || list_size <= 0)
		return;

	_dprintf("[%s, %d]Start to dump!\n", __FUNCTION__, __LINE__);

	for(i = 0, profile = list; i < list_size; ++i, profile = list + i)
		vpnc_dump_vpnc_profile(profile);

	_dprintf("[%s, %d]End of dump!\n", __FUNCTION__, __LINE__);
}
#endif

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
	if(!basic_conf)
		return -1;

	memset(basic_conf, 0, sizeof(VPNC_BASIC_CONF));

	if(server)
		snprintf(basic_conf->server, sizeof(basic_conf->server), "%s", server);
	if(username)
		snprintf(basic_conf->username, sizeof(basic_conf->username), "%s", username);
	if(passwd)
		snprintf(basic_conf->password, sizeof(basic_conf->password), "%s", passwd);

	return 0;
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
	char * desc, *proto, *server, *username, *passwd, *active, *vpnc_idx;

	if(!list || list_size <= 0)
		return -1;

	// load "vpnc_clientlist" to set username, password and server ip
	nv = nvp = strdup(nvram_safe_get("vpnc_clientlist"));

	cnt = 0;
	memset(list, 0, sizeof(VPNC_PROFILE)*list_size);
	while (nv && (b = strsep(&nvp, "<")) != NULL && cnt <= list_size) {
		if(VPNC_PROFILE_VER1 == prof_ver)
		{
			//proto, server, active and vpnc_idx are mandatory
			if (vstrsep(b, ">", &desc, &proto, &server, &username, &passwd, &active, &vpnc_idx) < 4)
				continue;

			if(!active || !vpnc_idx)
			continue;
			
			list[cnt].active = atoi(active);
			list[cnt].vpnc_idx = atoi(vpnc_idx);		
		}
		else
		{
			//proto and server are mandatory
			if (vstrsep(b, ">", &desc, &proto, &server, &username, &passwd) < 2)
				continue;
		}
		
		if(proto && server)
		{
			vpnc_set_basic_conf(server, username, passwd, &(list[cnt].basic));
			
			if(!strcmp(proto, PROTO_PPTP))
			{
				list[cnt].protocol = VPNC_PROTO_PPTP;
			}
			else if(!strcmp(proto, PROTO_L2TP))
			{
				list[cnt].protocol = VPNC_PROTO_L2TP;
			}
			else if(!strcmp(proto, PROTO_OVPN))
			{
				list[cnt].protocol = VPNC_PROTO_OVPN;
				list[cnt].config.ovpn.ovpn_idx = atoi(server);
			}
			++cnt;
		}
	}
	SAFE_FREE(nv);

	//load "vpnc_pptp_options_x_list" to set pptp option
	nv = nvp = strdup(nvram_safe_get("vpnc_pptp_options_x_list"));
	i = 0;
	while (nv && (b = strsep(&nvp, "<")) != NULL && i <= cnt) {

		if(i > 0 && VPNC_PROTO_PPTP == list[i - 1].protocol)
		{
			if(!strcmp(b, "auto"))
				list[i - 1].config.pptp.option = VPNC_PPTP_OPT_AUTO;
			else if(!strcmp(b, "-mppc"))
				list[i - 1].config.pptp.option = VPNC_PPTP_OPT_MPPC;
			else if(!strcmp(b, "+mppe-40"))
				list[i - 1].config.pptp.option = VPNC_PPTP_OPT_MPPE40;
			else if(!strcmp(b, "+mppe-56"))
				list[i - 1].config.pptp.option = VPNC_PPTP_OPT_MPPE56;
			else if(!strcmp(b, "+mppe-128"))
				list[i - 1].config.pptp.option = VPNC_PPTP_OPT_MPPE128;
			else
				list[i - 1].config.pptp.option = VPNC_PPTP_OPT_UNDEF;
		}
		++i;	
	}
	SAFE_FREE(nv);
	if(i != cnt + 1)
		_dprintf("[%s, %d]the numbers of vpnc_clientlist(%d) and vpnc_pptp_options_x_list(%d) are different!\n", __FUNCTION__, __LINE__, cnt, i);

	//vpnc_dump_vpnc_profile_list(list, cnt);
	return cnt;
}

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
static VPNC_PROFILE* vpnc_get_profile_by_vpnc_id(VPNC_PROFILE *list, const int list_size, const int vpnc_id)
{
	int i;
	if(!list || !list_size)
		return NULL;

	for(i = 0; i < list_size; ++i)
	{
		if(list[i].vpnc_idx == vpnc_id)
			return &(list[i]);
	}
	return NULL;
}

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
static int
vpnc_get_dev_policy_list(VPNC_DEV_POLICY *list, const int list_size, const int tmp_flag)
{
	int cnt;
	VPNC_DEV_POLICY *policy = list;

	char *nv, *nvp, *b;
	char *active, *dst_ip, *vpnc_idx;
#ifdef USE_IPTABLE_ROUTE_TARGE
	char *mac;
#else
	char *src_ip;
#endif
	
	if(!list || list_size <= 0)
		return 0;

	memset(list, 0, sizeof(VPNC_DEV_POLICY) * list_size);
	
	/* Protection level per client */
	if(!tmp_flag)
		nv = nvp = strdup(nvram_safe_get("vpnc_dev_policy_list"));
	else
		nv = nvp = strdup(nvram_safe_get("vpnc_dev_policy_list_tmp"));

	cnt = 0;
	while (nv && (b = strsep(&nvp, "<")) != NULL && cnt <= list_size) {
#ifdef USE_IPTABLE_ROUTE_TARGE		
		if (vstrsep(b, ">", &active, &mac, &dst_ip, &vpnc_idx) < 3)
			continue;
			
		if(!active || !mac || !vpnc_idx)
			continue;

		snprintf(policy->mac, sizeof(policy->mac), "%s", mac);
#else
		if (vstrsep(b, ">", &active, &src_ip, &dst_ip, &vpnc_idx) < 3)
			continue;
			
		if(!active || !src_ip || !vpnc_idx)
			continue;

		snprintf(policy->src_ip, sizeof(policy->src_ip), "%s", src_ip);
#endif
		policy->active = atoi(active);
		if(dst_ip)
			snprintf(policy->dst_ip, sizeof(policy->dst_ip), "%s", dst_ip);
		policy->vpnc_idx = atoi(vpnc_idx);

		++cnt;
		++policy;
	}
	free(nv);

	//vpnc_dump_dev_policy_list(list, list_size);
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
	if(!vpnc_ifname)
		return -1;

	if(!strncmp(vpnc_ifname, "ppp", 3))	//pptp/l2tp
	{
		return vpnc_ppp_linkunit_by_ifname(vpnc_ifname);		
	}
	else if(!strncmp(vpnc_ifname, "tun", 3))	//openvpn
	{
		unit = atoi(vpnc_ifname + 3) - OVPN_CLIENT_BASE;
		return _find_vpnc_idx_by_ovpn_unit(unit);
	}
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
int
vpnc_set_policy_by_ifname(const char *vpnc_ifname, const int action)
{
	int policy_cnt, vpnc_idx, i;
	VPNC_DEV_POLICY *policy;
	char *lan_ifname = nvram_safe_get("lan_ifname");
	VPNC_DEV_POLICY 	dev_policy[MAX_DEV_POLICY] = {0};

	policy_cnt =  vpnc_get_dev_policy_list(dev_policy, MAX_DEV_POLICY, 0);	

	if(!vpnc_ifname || !policy_cnt || !lan_ifname || lan_ifname[0] == '\0')
		return -1;

	vpnc_idx = _vpnc_find_index_by_ifname(vpnc_ifname);

	for(i = 0, policy = dev_policy; i < policy_cnt; ++i, ++policy)
	{
		if(policy->active && vpnc_idx == policy->vpnc_idx)
		{
			if(!action)	//remove rule
			{
#ifdef USE_MULTIPATH_ROUTE_TABLE
				//Can not support destination ip
				set_routing_rule(VPNC_ROUTE_DEL, policy->src_ip, policy->vpnc_idx);
#else
				if(policy->dst_ip[0] != '\0')	//has destination ip
					eval("iptables", "-D", "PREROUTING", "-t", "mangle", "-i", lan_ifname, "-m", "mac", "--mac-source",
						policy->mac, "-d", policy->dst_ip,"-j", "ROUTE", "--oif", (char*)vpnc_ifname);
				else
					eval("iptables", "-D", "PREROUTING", "-t", "mangle", "-i", lan_ifname, "-m", "mac", "--mac-source",
						policy->mac, "-j", "ROUTE", "--oif", (char*)vpnc_ifname);
#endif
			}
			else		//add value
			{
#ifdef USE_MULTIPATH_ROUTE_TABLE
				//Can not support destination ip				
				set_routing_rule(VPNC_ROUTE_ADD, policy->src_ip, policy->vpnc_idx);
#else
				if(policy->dst_ip[0] != '\0')	//has destination ip
					eval("iptables", "-A", "PREROUTING", "-t", "mangle", "-i", lan_ifname, "-m", "mac", "--mac-source",
						policy->mac, "-d", policy->dst_ip,"-j", "ROUTE", "--oif", (char*)vpnc_ifname);
				else
					eval("iptables", "-A", "PREROUTING", "-t", "mangle", "-i", lan_ifname, "-m", "mac", "--mac-source",
						policy->mac, "-j", "ROUTE", "--oif", (char*)vpnc_ifname);
#endif
			}
		}
	}
	return 0;
}

/*******************************************************************
* NAME: vpnc_handle_policy_rule
* AUTHOR: Andy Chiu
* CREATE DATE: 2016/1/25
* DESCRIPTION: add/del iptable rule
* INPUT:  action: 0:delete rule, 1: add rule.  lan_ifname: lan interface name. cliemt mac: client mac address.
* 		vpnc_ifname: vpn client interface name. target_ip(optional): target ip address. target_port(optional): target port. set -1 as unused.
* OUTPUT:  
* RETURN:  0:success, -1:failed.
* NOTE:
*******************************************************************/
#if defined(USE_MULTIPATH_ROUTE_TABLE)
int vpnc_handle_policy_rule(const int action, const char *src_ip, const int vpnc_idx)
{
	if(!src_ip)
	{
		_dprintf("[%s, %d] parameters error!\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if(!action)	//delete
	{
		//_dprintf("[%s, %d]remove rule. src_ip=%s, vpnc_idx=%d\n", __FUNCTION__, __LINE__,  src_ip, vpnc_idx);

		if(vpnc_idx != -1)
		{
			set_routing_rule(VPNC_ROUTE_DEL, src_ip, vpnc_idx);
		}
	}
	else	//add
	{
		//_dprintf("[%s, %d]add rule. src_ip=%s, vpnc_idx=%d\n", __FUNCTION__, __LINE__, src_ip, vpnc_idx);
		
		if(vpnc_idx != -1)
		{
			set_routing_rule(VPNC_ROUTE_ADD, src_ip, vpnc_idx);
		}
	}
	return 0;
}
#else
int vpnc_handle_policy_rule(const int action, const char *lan_ifname, const char *client_mac, 
	const char *vpnc_ifname, const char *target_ip, const int target_port)
{
	char port[8];
	int vpnc_idx;

	if(!lan_ifname || !client_mac || !vpnc_ifname)
	{
		_dprintf("[%s, %d] parameters error!\n", __FUNCTION__, __LINE__);
		return -1;
	}

	vpnc_idx = _vpnc_find_index_by_ifname(vpnc_ifname);
	
	snprintf(port, sizeof(port), "%d", target_port);
	
	if(!action)	//delete
	{
		if(target_ip && target_ip[0] != '\0' && target_port >= 0)	//has destination ip and port
			eval("iptables", "-D", "PREROUTING", "-t", "mangle", "-i", (char*)lan_ifname, "-m", "mac", "--mac-source",
				(char*)client_mac, "-d", (char*)target_ip, "--dport", port, "-j", "ROUTE", "--oif", (char*)vpnc_ifname);
		else if(target_ip && target_ip[0] != '\0')	//has target ip
			eval("iptables", "-D", "PREROUTING", "-t", "mangle", "-i", (char*)lan_ifname, "-m", "mac", "--mac-source",
				(char*)client_mac, "-d", (char*)target_ip, "-j", "ROUTE", "--oif", (char*)vpnc_ifname);
		else if(target_port >= 0)	//has target port			
			eval("iptables", "-D", "PREROUTING", "-t", "mangle", "-i", (char*)lan_ifname, "-m", "mac", "--mac-source",
				(char*)client_mac, "--dport", port, "-j", "ROUTE", "--oif", (char*)vpnc_ifname);
		else	//others
			eval("iptables", "-D", "PREROUTING", "-t", "mangle", "-i", (char*)lan_ifname, "-m", "mac", "--mac-source",
				(char*)client_mac, "-j", "ROUTE", "--oif", (char*)vpnc_ifname);
	}
	else	//add
	{
		if(target_ip && target_ip[0] != '\0' && target_port >= 0)	//has destination ip and port
			eval("iptables", "-A", "PREROUTING", "-t", "mangle", "-i", (char*)lan_ifname, "-m", "mac", "--mac-source",
				(char*)client_mac, "-d", (char*)target_ip, "--dport", port, "-j", "ROUTE", "--oif", (char*)vpnc_ifname);
		else if(target_ip && target_ip[0] != '\0')	//has target ip
			eval("iptables", "-A", "PREROUTING", "-t", "mangle", "-i", (char*)lan_ifname, "-m", "mac", "--mac-source",
				(char*)client_mac, "-d", (char*)target_ip, "-j", "ROUTE", "--oif", (char*)vpnc_ifname);
		else if(target_port >= 0)	//has target port			
			eval("iptables", "-A", "PREROUTING", "-t", "mangle", "-i", (char*)lan_ifname, "-m", "mac", "--mac-source",
				(char*)client_mac, "--dport", port, "-j", "ROUTE", "--oif", (char*)vpnc_ifname);
		else	//others
			eval("iptables", "-A", "PREROUTING", "-t", "mangle", "-i", (char*)lan_ifname, "-m", "mac", "--mac-source",
				(char*)client_mac, "-j", "ROUTE", "--oif", (char*)vpnc_ifname);
	}
	return 0;
}
#endif
#ifdef USE_IPTABLE_ROUTE_TARGE
/*******************************************************************
* NAME: vpnc_active_dev_policy
* AUTHOR: Andy Chiu
* CREATE DATE: 2016/1/24
* DESCRIPTION: when web ui active/inactive one dev policy, this function will handle it
* INPUT:  policy_idx: index of vpnc_dev_policy_list. Start from 0.
* OUTPUT:  
* RETURN:  0:success, -1:failed.
* NOTE:
*******************************************************************/
int vpnc_active_dev_policy(const int policy_idx)
{
	int policy_cnt;
	VPNC_DEV_POLICY *policy;
	VPNC_DEV_POLICY 	dev_policy[MAX_DEV_POLICY] = {0};
#ifdef USE_IPTABLE_ROUTE_TARGE
	char *lan_ifname = nvram_safe_get("lan_ifname");
	char *vpnc_ifname = NULL, tmp[100];
#endif

	policy_cnt =  vpnc_get_dev_policy_list(dev_policy, MAX_DEV_POLICY, 0);	

	_dprintf("[%s, %d]idx<%d> policy cnt<%d>\n", __FUNCTION__, __LINE__, policy_idx, policy_cnt);
	
	if(!policy_cnt || policy_idx >= policy_cnt || policy_idx < 0)
	{
		_dprintf("[%s, %d] policy_idx(%d) is not exist in the policy list(%d)\n", __FUNCTION__, __LINE__, policy_idx, policy_cnt);
		return -1;
	}

	policy = &(dev_policy[policy_idx]);

#ifdef USE_IPTABLE_ROUTE_TARGE
	if!lan_ifname || lan_ifname[0] == '\0'()
		return -1;
	snprintf(tmp, sizeof(tmp), "vpnc%d_ifname", policy->vpnc_idx);
	vpnc_ifname = nvram_safe_get(tmp);

	if(!vpnc_ifname || vpnc_ifname[0] =='\0')
	{
		_dprintf("[%s, %d] Can not find interface name by vpnc_idx(%d)\n", __FUNCTION__, __LINE__, policy_idx);
		return -1;
	}

	if(policy->active)
		vpnc_handle_policy_rule(1, lan_ifname, policy->mac, vpnc_ifname, policy->dst_ip, -1);
	else
		vpnc_handle_policy_rule(0, lan_ifname, policy->mac, vpnc_ifname, policy->dst_ip, -1);
#endif
	return 0;
	
}

/*******************************************************************
* NAME: vpnc_active_dev_policy
* AUTHOR: Andy Chiu
* CREATE DATE: 2016/1/24
* DESCRIPTION: when web ui active/inactive one dev policy, this function will handle it
* INPUT:  policy_idx: index of vpnc_dev_policy_list. Start from 0.
* OUTPUT:  
* RETURN:  0:success, -1:failed.
* NOTE:
*******************************************************************/
int vpnc_remove_tmp_policy_rule()
{
	char *nv;
	char *active, *mac, *dst_ip, *vpnc_idx;
	VPNC_DEV_POLICY policy;
#ifdef USE_IPTABLE_ROUTE_TARGE
	char *lan_ifname = nvram_safe_get("lan_ifname");
	char *vpnc_ifname = NULL, tmp[100];
#endif

	/* Protection level per client */
	nv = strdup(nvram_safe_get("vpnc_tmp_dev_policy"));
	memset(&policy, 0, sizeof(VPNC_DEV_POLICY));

	_dprintf("[%s, %d]remove vpnc_tmp_dev_policy=%s\n", __FUNCTION__, __LINE__, nv);
	
	if (vstrsep(nv, ">", &active, &mac, &dst_ip, &vpnc_idx) >= 3)
	{
		if(!active || !mac || !vpnc_idx)
		{
			free(nv);
			_dprintf("[%s, %d] Invalid value in vpnc_tmp_dev_policy\n", __FUNCTION__, __LINE__);
			return -1;
		}

		policy.active = atoi(active);
		snprintf(policy.mac, sizeof(policy.mac), "%s", mac);
		policy.vpnc_idx = atoi(vpnc_idx);

#ifdef USE_IPTABLE_ROUTE_TARGE
		if(dst_ip)
			snprintf(policy.dst_ip, sizeof(policy.dst_ip), "%s", dst_ip);
		snprintf(tmp, sizeof(tmp), "vpnc%d_ifname", policy.vpnc_idx);
		vpnc_ifname = nvram_safe_get(tmp);

		if(!vpnc_ifname || vpnc_ifname[0] =='\0')
		{
			_dprintf("[%s, %d] Can not find interface name by vpnc_idx(%d)\n", __FUNCTION__, __LINE__, policy.vpnc_idx);
			free(nv);
			return -1;			
		}

		if(policy.active)
			vpnc_handle_policy_rule(0, lan_ifname, policy.mac, vpnc_ifname, policy.dst_ip, -1);
#endif
		
	}

	free(nv);
	return 0;
}
#else
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
	VPNC_DEV_POLICY 	dev_policy_new[MAX_DEV_POLICY] = {0}, dev_policy_old[MAX_DEV_POLICY] = {0};

	policy_cnt_old =  vpnc_get_dev_policy_list(dev_policy_old, MAX_DEV_POLICY, 1);	
	policy_cnt_new =  vpnc_get_dev_policy_list(dev_policy_new, MAX_DEV_POLICY, 0);

	_dprintf("[%s, %d]old policy cnt<%d> new policy cnt<%d>\n", __FUNCTION__, __LINE__, policy_cnt_old, policy_cnt_new);

	//check old policy list, if it does not exist in the new list or different, remove the old rule.
	for(i = 0; i < policy_cnt_old; ++i)
	{
		policy_ptr = &dev_policy_old[i];
		if(policy_ptr->active)
		{
			flag = 1;
			for(j = 0; j < policy_cnt_new; ++j)
			{
				if(!memcmp(policy_ptr, &dev_policy_new[j], sizeof(VPNC_DEV_POLICY)))	//policy is not changed
				{
					flag = 0;
					break;
				}
			}
			if(flag)
			{
				vpnc_handle_policy_rule(0, policy_ptr->src_ip, policy_ptr->vpnc_idx);
			}
		}

	}

	//check new policy list, if it does not exist in the old list, add the rule.
	for(i = 0; i < policy_cnt_new; ++i)
	{
		policy_ptr = &dev_policy_new[i];
		if(policy_ptr->active)
		{
			flag = 1;
			for(j = 0; j < policy_cnt_old; ++j)
			{
				if(!memcmp(policy_ptr, &dev_policy_old[j], sizeof(VPNC_DEV_POLICY)))	//policy is not changed
				{
					flag = 0;
					break;
				}
			}
			if(flag)
			{
				vpnc_handle_policy_rule(1, policy_ptr->src_ip, policy_ptr->vpnc_idx);
			}
		}

	}	
	return 0;
}
#endif
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
void
vpnc_init()
{
	//load profile
	vpnc_profile_num = vpnc_load_profile(vpnc_profile, MAX_VPNC_PROFILE, VPNC_PROFILE_VER1);
	//_dprintf("[%s, %d]vpnc_profile_num=%d\n", __FUNCTION__, __LINE__, vpnc_profile_num);
}

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
int
start_vpnc_by_unit(const int unit)
{
	FILE *fp;
	char options[80], l2tp_conf[128], l2tp_ctrl[128], l2tp_pid[128];
	char *pppd_argv[] = { "/usr/sbin/pppd", "file", options, NULL};
	char tmp[100], prefix[] = "vpnc_", vpnc_prefix[] = "vpncXXXXXXXXXX_", wan_prefix[] = "wanXXXXXXXXXX_";
	char buf[256];	/* although maximum length of pppoe_username/pppoe_passwd is 64. pppd accepts up to 256 characters. */
	mode_t mask;
	int ret = 0;
	VPNC_PROFILE *prof;
	
	if(unit >= vpnc_profile_num)
		return -1;

	_dprintf("[%s, %d]Start unit(%d)!\n", __FUNCTION__, __LINE__, unit);
	
	prof = vpnc_profile + unit;

	//vpnc_dump_vpnc_profile(prof);
	
	//stop if connection exist.
	stop_vpnc_by_unit(unit);

	//init prefix
	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_primary_ifunit());
	snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", prof->vpnc_idx);

	/* unset vpnc_dut_disc */
	nvram_unset(strlcat_r(vpnc_prefix, "dut_disc", tmp, sizeof(tmp)));
	
	//init option path
	if(VPNC_PROTO_PPTP == prof->protocol)
		snprintf(options, sizeof(options), "/tmp/ppp/vpnc%d_options.pptp", prof->vpnc_idx);
	else if(VPNC_PROTO_L2TP == prof->protocol)
		snprintf(options, sizeof(options), "/tmp/ppp/vpnc%d_options.l2tp", prof->vpnc_idx);
	else if(VPNC_PROTO_OVPN == prof->protocol)
	{
		_dprintf("[%s, %d]Start to connect OpenVPN(%d).\n", __FUNCTION__, __LINE__, prof->config.ovpn.ovpn_idx);
		vpnc_ovpn_sync_account(prof);
		start_ovpn_client(prof->config.ovpn.ovpn_idx);
		return 0;
	}
	else if(VPNC_PROTO_IPSEC== prof->protocol)
	{
		//TODO:
		_dprintf("[%s, %d]Start to connect IPSec.\n", __FUNCTION__, __LINE__);
		return 0;
	}
	else
	{
		_dprintf("[%s, %d]Unknown protocol\n", __FUNCTION__, __LINE__);
		return -1;
	}

	//init vpncX_ state.
	update_vpnc_state(prof->vpnc_idx, WAN_STATE_INITIALIZING, 0);

	//Run PPTP/L2TP
	if (VPNC_PROTO_PPTP == prof->protocol || VPNC_PROTO_L2TP == prof->protocol) {
		mask = umask(0000);

		/* Generate options file */
		if (!(fp = fopen(options, "w"))) {
			perror(options);
			umask(mask);
			return -1;
		}

		umask(mask);

		/* route for pptp/l2tp's server */
		char *wan_ifname = nvram_safe_get(strlcat_r(wan_prefix, "pppoe_ifname", tmp, sizeof(tmp)));
		route_add(wan_ifname, 0, nvram_safe_get(strlcat_r(wan_prefix, "gateway", tmp, sizeof(tmp))), "0.0.0.0", "255.255.255.255");

		//generate ppp profile
		/* do not authenticate peer and do not use eap */
		fprintf(fp, "noauth\n");
		fprintf(fp, "refuse-eap\n");
		fprintf(fp, "user '%s'\n",
			ppp_safe_escape(prof->basic.username, buf, sizeof(buf)));
		fprintf(fp, "password '%s'\n",
			ppp_safe_escape(prof->basic.password, buf, sizeof(buf)));

		if (VPNC_PROTO_PPTP == prof->protocol) {
			fprintf(fp, "plugin pptp.so\n");
			fprintf(fp, "pptp_server '%s'\n", prof->basic.server);
			fprintf(fp, "vpnc 1\n");
			/* see KB Q189595 -- historyless & mtu */
			if (nvram_match(strlcat_r(wan_prefix, "proto", tmp, sizeof(tmp)), "pptp") || nvram_match(strlcat_r(wan_prefix, "proto", tmp, sizeof(tmp)), "l2tp"))
				fprintf(fp, "nomppe-stateful mtu 1300\n");
			else
				fprintf(fp, "nomppe-stateful mtu 1400\n");

			if (VPNC_PPTP_OPT_MPPC == prof->config.pptp.option) {
				fprintf(fp, "nomppe nomppc\n");
			} else
			if (VPNC_PPTP_OPT_MPPE40 == prof->config.pptp.option) {
				fprintf(fp, "require-mppe\n"
					    "require-mppe-40\n");
			} else
			if (VPNC_PPTP_OPT_MPPE56 == prof->config.pptp.option) {
				fprintf(fp, "nomppe-40\n"
					    "require-mppe\n"
					    "require-mppe-56\n");
			} else
			if (VPNC_PPTP_OPT_MPPE128 == prof->config.pptp.option) {
				fprintf(fp, "nomppe-40\n"
					    "nomppe-56\n"
					    "require-mppe\n"
					    "require-mppe-128\n");
			} else
			if (VPNC_PPTP_OPT_AUTO == prof->config.pptp.option) {
				fprintf(fp, "require-mppe-40\n"
					    "require-mppe-56\n"
					    "require-mppe-128\n");
			}
		} else {
			fprintf(fp, "nomppe nomppc\n");

			if (nvram_match(strlcat_r(wan_prefix, "proto", tmp, sizeof(tmp)), "pptp") || nvram_match(strlcat_r(wan_prefix, "proto", tmp, sizeof(tmp)), "l2tp"))
				fprintf(fp, "mtu 1300\n");
			else
				fprintf(fp, "mtu 1400\n");			
		}

		if (VPNC_PROTO_L2TP != prof->protocol) {
			ret = nvram_get_int(strlcat_r(prefix, "pppoe_idletime", tmp, sizeof(tmp)));
			if (ret && nvram_get_int(strlcat_r(prefix, "pppoe_demand", tmp, sizeof(tmp)))) {
				fprintf(fp, "idle %d ", ret);
				if (nvram_invmatch(strlcat_r(prefix, "pppoe_txonly_x", tmp, sizeof(tmp)), "0"))
					fprintf(fp, "tx_only ");
				fprintf(fp, "demand\n");
			}
			fprintf(fp, "persist\n");
		}

		fprintf(fp, "holdoff %d\n", nvram_get_int(strlcat_r(prefix, "pppoe_holdoff", tmp, sizeof(tmp))) ? : 10);
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

			//generate l2tp profile
			if (!(fp = fopen(l2tp_conf, "w"))) {
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
				"vpnc 1\n"
				"hostname %s\n"
				"lac-handler sync-pppd\n"
				"persist yes\n"
				"maxfail %d\n"
				"holdoff %d\n"
				"hide-avps no\n"
				"section cmd\n"
				"socket-path " "%s" "\n\n",
				options,
	                        prof->basic.server,
				nvram_invmatch(strlcat_r(prefix, "hostname", tmp, sizeof(tmp)), "") ?
					nvram_safe_get(strlcat_r(prefix, "hostname", tmp, sizeof(tmp))) : "localhost",
				nvram_get_int(strlcat_r(prefix, "pppoe_maxfail", tmp, sizeof(tmp)))  ? : 32767,
				nvram_get_int(strlcat_r(prefix, "pppoe_holdoff", tmp, sizeof(tmp))) ? : 10, l2tp_ctrl);

			fclose(fp);

			/* launch l2tp */
			eval("/usr/sbin/l2tpd", "-c", l2tp_conf, "-p", l2tp_pid);

			ret = 3;
			do {
				_dprintf("%s: wait l2tpd up at %d seconds...\n", __FUNCTION__, ret);
				usleep(1000*1000);
			} while (!pids("l2tpd") && ret--);

			/* start-session */
			ret = eval("/usr/sbin/l2tp-control", "-s", l2tp_ctrl, "start-session 0.0.0.0");

			/* pppd sync nodetach noaccomp nobsdcomp nodeflate */
			/* nopcomp novj novjccomp file /tmp/ppp/options.l2tp */

		} 
		else
		{
			ret = _eval(pppd_argv, NULL, 0, NULL);	//launch pppd
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
int
stop_vpnc_by_unit(const int unit)
{
	VPNC_PROFILE *prof;
	char pidfile[sizeof("/var/run/ppp-vpnXXXXXXXXXX.pid")];
	char l2tp_pid[128];
	char tmp[100], vpnc_prefix[] = "vpncXXXXX_";
	
	if(unit >= vpnc_profile_num)
		return -1;

	prof = vpnc_profile + unit;

	snprintf(pidfile, sizeof(pidfile), "/var/run/ppp-vpn%d.pid", prof->vpnc_idx);

	snprintf(vpnc_prefix, sizeof(vpnc_prefix), "vpnc%d_", prof->vpnc_idx);

	/* Reset the state of vpnc_dut_disc */
	nvram_set_int(strlcat_r(vpnc_prefix, "dut_disc", tmp, sizeof(tmp)), prof->vpnc_idx);

	if(VPNC_PROTO_PPTP == prof->protocol || VPNC_PROTO_L2TP == prof->protocol)
	{
		update_vpnc_state(prof->vpnc_idx, WAN_STATE_STOPPING, 0);
		if(VPNC_PROTO_L2TP == prof->protocol)
		{
			//stop l2tp
			snprintf(l2tp_pid, sizeof(l2tp_pid), L2TP_VPNC_PID, prof->vpnc_idx);
			if(check_if_file_exist(l2tp_pid))
			{
				kill_pidfile_tk(l2tp_pid);
				usleep(1000*10000);
			}
		}
		/* Stop pppd */
		if (kill_pidfile_s(pidfile, SIGHUP) == 0 &&
		    kill_pidfile_s(pidfile, SIGTERM) == 0) {
			usleep(3000*1000);
			kill_pidfile_tk(pidfile);
		}	
	}
	else if(VPNC_PROTO_OVPN == prof->protocol)
	{
		_dprintf("[%s, %d]Stop OpenVPN(%d).\n", __FUNCTION__, __LINE__, prof->config.ovpn.ovpn_idx);
		stop_ovpn_client(prof->config.ovpn.ovpn_idx);		
	}
	return 0;
}

#ifdef USE_MULTIPATH_ROUTE_TABLE
/*******************************************************************
* NAME: set_routing_table
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

	//delete all rules in vpnc routing table
	snprintf(tmp, sizeof(tmp), "ip route show table %d > /tmp/route_tmp", vpnc_id);
	system(tmp);
	fp = fopen("/tmp/route_tmp", "r");
	if(fp)
	{
		while(fgets(tmp2, sizeof(tmp2), fp))
		{
			char *ptr = strchr(tmp2, '\n');
			if(ptr)
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
		_dprintf("[%s, %d]Can not get route table %d\n", __FUNCTION__, __LINE__, vpnc_id);
		unlink("/tmp/route_tmp");
		return -1;
	}		
	return 0;
}

int set_routing_table(const int cmd, const int vpnc_id)
{
	char tmp[256], tmp2[256];
	char prefix[] = "vpncXXXXX_", id_str[16], wan_prefix[]= "wanXXXXXX_";
	char *route_network = NULL, *route_netmask = NULL, *route_gateway=NULL, *route_metric=NULL;
	VPNC_PROFILE *prof = NULL;
	FILE *fp;
	int cnt, i;

	//_dprintf("[%s, %d]cmd=%d, vpnc_id=%d\n", __FUNCTION__, __LINE__, cmd, vpnc_id);

	//get protocol
	prof =  vpnc_get_profile_by_vpnc_id(vpnc_profile, MAX_VPNC_PROFILE, vpnc_id);

	if(!prof && vpnc_id != INTERNET_ROUTE_TABLE_ID)
	{
		_dprintf("[%s, %d]Can not get vpnc profile(%d)\n", __FUNCTION__, __LINE__, vpnc_id);
		return -1;
	}
	
	if(vpnc_id >= VPNC_UNIT_BASIC)	//VPNC
	{
		snprintf(id_str, sizeof(id_str), "%d", vpnc_id);
		snprintf(prefix, sizeof(prefix), "vpnc%d_", vpnc_id);
	}
	else	//internet
	{
		snprintf(id_str, sizeof(id_str), "%d", INTERNET_ROUTE_TABLE_ID);
		snprintf(prefix, sizeof(prefix), "wan%d_", wan_primary_ifunit());
	}

	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_primary_ifunit());

	_clean_routing_table(vpnc_id);
	
	if(cmd)
	{
		//get main route table and set to vpnc route table
		system("ip route show table main> /tmp/route_tmp");
		fp = fopen("/tmp/route_tmp", "r");
		if(fp)
		{
			while(fgets(tmp2, sizeof(tmp2), fp))
			{
				char *ptr = strchr(tmp2, '\n');
				if(ptr)
				{
					*ptr = '\0';
				}
				if(!strncmp(tmp2, "default", 7) && (prof->protocol == VPNC_PROTO_PPTP || prof->protocol == VPNC_PROTO_L2TP))
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
			_dprintf("[%s, %d]Can not get main routing table\n", __FUNCTION__, __LINE__);
			unlink("/tmp/route_tmp");
			return -1;
		}


		//set vpnc route table
		if(vpnc_id != INTERNET_ROUTE_TABLE_ID)
		{
			if(prof->protocol == VPNC_PROTO_PPTP || prof->protocol == VPNC_PROTO_L2TP)
			{
				//set default routing
				eval("ip", "route", "add", "default", "via", nvram_safe_get(strlcat_r(prefix, "gateway", tmp, sizeof(tmp))), 
					"dev", nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), "table", id_str);
			}
			else if(prof->protocol == VPNC_PROTO_OVPN)
			{
				//set default routing
				eval("ip", "route", "add", "0.0.0.0/1", "via", nvram_safe_get(strlcat_r(prefix, "gateway", tmp, sizeof(tmp))), 
					"dev", nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), "table", id_str);
				eval("ip", "route", "add", "128.0.0.0/1", "via", nvram_safe_get(strlcat_r(prefix, "gateway", tmp, sizeof(tmp))), 
					"dev", nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), "table", id_str);

				//set remote routing rule
				cnt = nvram_get_int(strlcat_r(prefix, "remote_num", tmp, sizeof(tmp)));
				for(i = 0; i < cnt; ++i)
				{
					snprintf(tmp2, sizeof(tmp2), "remote_%d", i);
					eval("ip", "route", "add", nvram_safe_get(strlcat_r(prefix, tmp2, tmp, sizeof(tmp))), 
						"via", nvram_safe_get(strlcat_r(wan_prefix, "gateway", tmp, sizeof(tmp))), 
						"dev", nvram_safe_get(strlcat_r(wan_prefix, "ifname", tmp, sizeof(tmp))),
						"table", id_str);
				}

				//set route
				cnt = nvram_get_int(strlcat_r(prefix, "route_num", tmp, sizeof(tmp)));
				for(i = 0; i < cnt; ++i)
				{
					snprintf(tmp2, sizeof(tmp2), "route_network_%d", i);
					route_network = nvram_safe_get(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)));
					snprintf(tmp2, sizeof(tmp2), "route_netmask_%d", i);
					route_netmask = nvram_safe_get(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)));
					snprintf(tmp2, sizeof(tmp2), "route_gateway_%d", i);
					route_gateway = nvram_safe_get(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)));
					snprintf(tmp2, sizeof(tmp2), "route_metric_%d", i);
					route_metric = nvram_safe_get(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)));

					int cidr = convert_subnet_mask_to_cidr(route_netmask);

					if(cidr < 32 && cidr > 0)
						snprintf(tmp2, sizeof(tmp2), "%s/%d", route_network, cidr);
					else
						snprintf(tmp2, sizeof(tmp2), "%s", route_network);

					if(route_metric && route_metric[0] != '\0')
						eval("ip", "route", "add", tmp2, "via", route_gateway, "dev", nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), "metric", route_metric, "table", id_str);
					else
						eval("ip", "route", "add", tmp2, "via", route_gateway, "dev", nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), "table", id_str);
				}
				
			}
		}
	}
	return 0;
	
}

/*******************************************************************
* NAME: set_routing_table
* AUTHOR: Andy Chiu
* CREATE DATE: 2017/2/7
* DESCRIPTION: set multipath routing table 
* INPUT:  cmd: VPNC_ROUTE_CMD.  vpnc_id: index of vpnc profile
* OUTPUT:  
* RETURN:  0: success, -1: failed
* NOTE:
*******************************************************************/
int set_routing_rule(const VPNC_ROUTE_CMD cmd, const char *source_ip, const int vpnc_id)
{
	char cmd_str[8], id_str[8];

	if(!source_ip)
		return -1;

	//_dprintf("[%s, %d]<%d><%s><%d>\n", __FUNCTION__, __LINE__, cmd, source_ip, vpnc_id);
	snprintf(cmd_str, sizeof(cmd_str), "%s", (cmd == VPNC_ROUTE_DEL)? "del": "add");

	if(vpnc_id >= VPNC_UNIT_BASIC)
		snprintf(id_str, sizeof(id_str), "%d", vpnc_id);
	else
		snprintf(id_str, sizeof(id_str), "main");

	eval("ip", "rule", cmd_str, "from", (char *) source_ip, "table", id_str, "priority", VPNC_RULE_PRIORITY);
	return 0;
}


/*******************************************************************
* NAME: set_default_routing_table
* AUTHOR: Andy Chiu
* CREATE DATE: 2017/2/7
* DESCRIPTION: set multipath routing table 
* INPUT:  cmd: VPNC_ROUTE_CMD.  vpnc_id: index of vpnc profile
* OUTPUT:  
* RETURN:  0: success, -1: failed
* NOTE:
*******************************************************************/
int set_default_routing_table(const VPNC_ROUTE_CMD cmd, const int table_id)
{
	char id_str[8];

	//_dprintf("[%s, %d]<%d><%d>\n", __FUNCTION__, __LINE__, cmd, table_id);
	snprintf(id_str, sizeof(id_str), "%d", table_id);

	//remove current default routing table
	eval("ip", "rule", "del", "priority", VPNC_RULE_PRIORITY_DEFAULT);

	if(VPNC_ROUTE_ADD == cmd && table_id > 0)
		eval("ip", "rule", "add", "from", "all", "table", id_str, "priority", VPNC_RULE_PRIORITY_DEFAULT);
	
	return 0;
}

int clean_routing_rule_by_vpnc_idx(const int vpnc_idx)
{
	VPNC_DEV_POLICY 	dev_policy[MAX_DEV_POLICY] = {0};
	int policy_cnt = 0, i, cnt = 0;

	policy_cnt =  vpnc_get_dev_policy_list(dev_policy, MAX_DEV_POLICY, 0);

	for(i = 0; i < policy_cnt; ++i)
	{
		if(dev_policy[i].active && dev_policy[i].vpnc_idx == vpnc_idx)
		{
			set_routing_rule(VPNC_ROUTE_DEL, dev_policy[i].src_ip, vpnc_idx);
			++cnt;
		}
	}
	return cnt;
}

#endif

int clean_vpnc_setting_value(const int vpnc_idx)
{
	char prefix[] = "vpncXXXX_", tmp[128], tmp2[128];
	VPNC_PROFILE *prof = NULL;
	int cnt, i;

	//_dprintf("[%s, %d]idx=%d\n", __FUNCTION__, __LINE__, vpnc_idx);
	
	snprintf(prefix, sizeof(prefix), "vpnc%d_", vpnc_idx);
	
	//unset general settings, dns, gateway, ifname, ipaddr.
	nvram_unset(strlcat_r(prefix, "dns", tmp, sizeof(tmp)));
	nvram_unset(strlcat_r(prefix, "gateway", tmp, sizeof(tmp)));
	nvram_unset(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)));
	nvram_unset(strlcat_r(prefix, "ipaddr", tmp, sizeof(tmp)));

	//unset OpenVPN settings.
	prof = vpnc_get_profile_by_vpnc_id(vpnc_profile, MAX_VPNC_PROFILE, vpnc_idx);

	if(prof && prof->protocol == VPNC_PROTO_OVPN)
	{
		cnt = nvram_get_int(strlcat_r(prefix, "remote_num", tmp, sizeof(tmp)));

		nvram_unset(strlcat_r(prefix, "remote_num", tmp, sizeof(tmp)));
		
		for(i = 0; i < cnt; ++i)
		{
			snprintf(tmp2, sizeof(tmp2), "remote_%d", i);
			nvram_unset(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)));
		}

		cnt = nvram_get_int(strlcat_r(prefix, "route_num", tmp, sizeof(tmp)));

		nvram_unset(strlcat_r(prefix, "route_num", tmp, sizeof(tmp)));
		
		for(i = 0; i < cnt; ++i)
		{
			snprintf(tmp2, sizeof(tmp2), "route_gateway_%d", i);
			nvram_unset(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)));

			snprintf(tmp2, sizeof(tmp2), "route_network_%d", i);
			nvram_unset(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)));

			snprintf(tmp2, sizeof(tmp2), "route_netmask_%d", i);
			nvram_unset(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)));

			snprintf(tmp2, sizeof(tmp2), "route_metric_%d", i);
			nvram_unset(strlcat_r(prefix, tmp2, tmp, sizeof(tmp)));
		}
	}
	
	return 0;
}

