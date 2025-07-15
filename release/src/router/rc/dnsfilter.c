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
 *
 * Copyright 2014-2022 Eric Sauvageau.
 *
 */

#include <rc.h>
#include <net/ethernet.h>
#include <shared.h>
#ifdef RTCONFIG_DNSFILTER
#include "dnsfilter.h"
#ifdef RTCONFIG_MULTILAN_CFG
#include "mtlan_utils.h"
#endif
#ifdef RTCONFIG_OPENVPN
#include <openvpn_config.h>
#endif

static int _get_table_size(const int server6)
{
	int i = 0;
	if(!server6)
	{
		while(server_table[i][0] != NULL)
			++i;
	}
#ifdef RTCONFIG_IPV6
	else
	{
		while(server6_table[i][0] != NULL)
			++i;
	}
#endif
	return i;
}

// Return 1 if selected mode supports DNS over TLS
int dnsfilter_support_dot(int mode)
{
	switch (mode){
		case DNSF_SRV_CUSTOM1:
		case DNSF_SRV_CUSTOM2:
		case DNSF_SRV_CUSTOM3:	// Custom 1, 2 and 3 - assume they might support it
		case DNSF_SRV_QUAD9:
		case DNSF_SRV_ROUTER:	// Router (in case end-user implements it locally)
		case DNSF_SRV_CLEANBROWSING_SECURITY:
		case DNSF_SRV_CLEANBROWSING_ADULT:
		case DNSF_SRV_CLEANBROWSING_FAMILY:
			return 1;
			break;
		default:
			return 0;
	}
}


int get_dns_filter(int proto, int mode, dnsf_srv_entry_t *dnsfsrv)
{
	int count = 0;

// Initialize
	if (mode >= _get_table_size(0)) mode = 0;

#ifdef RTCONFIG_IPV6
	if (proto == AF_INET6)
#ifdef HND_ROUTER
	{
		switch (mode) {
			case DNSF_SRV_CUSTOM1:
				strlcpy(dnsfsrv->server1, nvram_safe_get("dnsfilter_custom61"), 46);
				dnsfsrv->server2[0] = '\0';
				break;
			case DNSF_SRV_CUSTOM2:
				strlcpy(dnsfsrv->server1, nvram_safe_get("dnsfilter_custom62"), 46);
				dnsfsrv->server2[0] = '\0';
				break;
			case DNSF_SRV_CUSTOM3:
				strlcpy(dnsfsrv->server1, nvram_safe_get("dnsfilter_custom63"), 46);
				dnsfsrv->server2[0] = '\0';
				break;
#if defined(BCM4912) || defined(RTCONFIG_HND_ROUTER_BE_4916)	// Causes kernel checksum errors on HND 5.02
			case DNSF_SRV_ROUTER:
				strlcpy(dnsfsrv->server1, nvram_safe_get("ipv6_rtr_addr"), 46);
				dnsfsrv->server2[0] = '\0';
				break;
#endif
			default:
				strlcpy(dnsfsrv->server1, server6_table[mode][0], 46);
				strlcpy(dnsfsrv->server2, server6_table[mode][1], 46);
		}
	} else
#else
	{
		strlcpy(dnsfsrv->server1, server6_table[mode][0], 46);
		strlcpy(dnsfsrv->server2, server6_table[mode][1], 46);
	} else
#endif
#endif	// RTCONFIG_IPV6
	{
		switch (mode) {
			case DNSF_SRV_CUSTOM1:
				strlcpy(dnsfsrv->server1, nvram_safe_get("dnsfilter_custom1"), 46);
				dnsfsrv->server2[0] = '\0';
				break;
			case DNSF_SRV_CUSTOM2:
				strlcpy(dnsfsrv->server1, nvram_safe_get("dnsfilter_custom2"), 46);
				dnsfsrv->server2[0] = '\0';
				break;
			case DNSF_SRV_CUSTOM3:
				strlcpy(dnsfsrv->server1, nvram_safe_get("dnsfilter_custom3"), 46);
				dnsfsrv->server2[0] = '\0';
				break;
			case DNSF_SRV_ROUTER:
				strlcpy(dnsfsrv->server1, nvram_safe_get("lan_ipaddr"), 46);
				dnsfsrv->server2[0] = '\0';
				break;
			default:
				strlcpy(dnsfsrv->server1, server_table[mode][0], 46);
				strlcpy(dnsfsrv->server2, server_table[mode][1], 46);
		}
	}

// Make sure it's valid
	if ( (*dnsfsrv->server1) &&
#ifdef RTCONFIG_IPV6
	     ( ((proto == AF_INET6) && !is_valid_ip6(dnsfsrv->server1)) ||
#endif
	       ((proto == AF_INET) && !is_valid_ip4(dnsfsrv->server1)))
	){
		logmessage("dnsdirector", "Invalid server1 for mode %d!", mode);
		dnsfsrv->server1[0] = '\0';
	}

        if ( (*dnsfsrv->server2) &&
#ifdef RTCONFIG_IPV6
             ( ((proto == AF_INET6) && !is_valid_ip6(dnsfsrv->server2)) ||
#endif
               ((proto == AF_INET) && !is_valid_ip4(dnsfsrv->server2)))
        ){
		logmessage("dnsdirector", "Invalid server2 for mode %d!", mode);
                dnsfsrv->server2[0] = '\0';
        }

// Report how many non-empty server we are returning
	if (*dnsfsrv->server1) count++;
	if (*dnsfsrv->server2) count++;
	return count;
}


void dnsfilter_settings(FILE *fp) {
	char *name, *mac, *mode;
	unsigned char ea[ETHER_ADDR_LEN];
	char *nv, *nvp, *rule;
	int dnsmode;
	dnsf_srv_entry_t dnsfsrv;

#ifdef RTCONFIG_MULTILAN_CFG
	int i,j;
	char buf[32] = {0};
	char lan_ipaddr[INET_ADDRSTRLEN];
	MTLAN_T *pmtl = NULL;
	size_t  mtl_sz = 0;
#endif

	if (nvram_get_int("dnsfilter_enable_x")) {
		/* Reroute all DNS requests from LAN */
		fprintf(fp, "-A PREROUTING -i br+ -p udp -m udp --dport 53 -j DNSFILTER\n"
			    "-A PREROUTING -i br+ -p tcp -m tcp --dport 53 -j DNSFILTER\n");
#ifdef RTCONFIG_MULTILAN_CFG
		/* Reroute all DNS requests from clients of VPN server */
#ifdef RTCONFIG_OPENVPN
//		if (nvram_get_int("VPNServer_enable")) {
			char word[8] = {0};
			char nvservers[32] = {0};
			char *next = NULL;
			int unit;
			nvram_safe_get_r("vpn_serverx_start", nvservers, sizeof(nvservers));
			foreach_44(word, nvservers, next) {
				unit = atoi(word);
				fprintf(fp, "-A PREROUTING -i tun%d -p udp -m udp --dport 53 -j DNSFILTER\n"
					"-A PREROUTING -i tun%d -p tcp -m tcp --dport 53 -j DNSFILTER\n"
					, OVPN_SERVER_BASE + unit, OVPN_SERVER_BASE + unit);
			}
//		}
#endif
#ifdef RTCONFIG_WIREGUARD
		{
			int i;
			char prefix[8] = {0};
			for (i = 1; i <= WG_SERVER_MAX; i++) {
				snprintf(prefix, sizeof(prefix), "%s%d_", WG_SERVER_NVRAM_PREFIX, i);
				if (nvram_pf_get_int(prefix, "enable")) {
					fprintf(fp, "-A PREROUTING -i wgs%d -p udp -m udp --dport 53 -j DNSFILTER\n"
						"-A PREROUTING -i wgs%d -p tcp -m tcp --dport 53 -j DNSFILTER\n", i, i);
				}
			}
		}
#endif
#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
		if (nvram_get_int("pptpd_enable")) {
			fprintf(fp, "-A PREROUTING -i pptp+ -p udp -m udp --dport 53 -j DNSFILTER\n"
					"-A PREROUTING -i pptp+ -p tcp -m tcp --dport 53 -j DNSFILTER\n");
		}
#endif
#ifdef RTCONFIG_IPSEC
		if (nvram_get_int("ipsec_server_enable")) {
			char virtual_subnet[16] = {0};
			get_virtual_subnet("ipsec_profile_1", virtual_subnet, sizeof(virtual_subnet));
			if(virtual_subnet[0] != '\0') {
				fprintf(fp, "-A PREROUTING --src %s.0/24 -p udp -m udp --dport 53 -j DNSFILTER\n"
					"-A PREROUTING --src %s.0/24 -p tcp -m tcp --dport 53 -j DNSFILTER\n"
					, virtual_subnet, virtual_subnet);
			}
		}
#endif
#endif

		/* Protection level per client */

#ifdef HND_ROUTER
		nv = nvp = malloc(255 * 6 + 1);
		if (nv) nvram_split_get("dnsfilter_rulelist", nv, 255 * 6 + 1, 5);
#else
		nv = nvp = strdup(nvram_safe_get("dnsfilter_rulelist"));
#endif
		while (nv && (rule = strsep(&nvp, "<")) != NULL) {
			if (vstrsep(rule, ">", &name, &mac, &mode) != 3)
				continue;
			if (!*mac || !*mode || !ether_atoe(mac, ea))
				continue;
			dnsmode = atoi(mode);
			if (dnsmode == DNSF_SRV_UNFILTERED) {
				fprintf(fp, "-A DNSFILTER -m mac --mac-source %s -j RETURN\n",
					mac);
			} else if (dnsmode == DNSF_SRV_ROUTER) {
				fprintf(fp, "-A DNSFILTER -m mac --mac-source %s -j REDIRECT\n",
					mac);
			} else if (get_dns_filter(AF_INET, dnsmode, &dnsfsrv)) {
				fprintf(fp,"-A DNSFILTER -m mac --mac-source %s -j DNAT --to-destination %s\n",
					mac, dnsfsrv.server1);
			}
		}

		free(nv);

#ifdef RTCONFIG_MULTILAN_CFG
		pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
		if(pmtl)
		{
			if(get_mtlan(pmtl, &mtl_sz))
			{
				for(i = 1; i < mtl_sz; ++i)
				{
					if(pmtl[i].enable && pmtl[i].sdn_t.sdn_idx && pmtl[i].nw_t.idx)
					{
						if(pmtl[i].sdn_t.dnsf_idx == DNSF_SRV_UNFILTERED)
						{
							fprintf(fp, "-A DNSFILTER -i %s -j RETURN\n", pmtl[i].nw_t.ifname);
						}
						else if (pmtl[i].sdn_t.dnsf_idx == DNSF_SRV_ROUTER)
						{
							strlcpy(lan_ipaddr, pmtl[i].nw_t.addr, sizeof(lan_ipaddr));
							if (*lan_ipaddr)
							{
								fprintf(fp, "-A DNSFILTER -i %s -j DNAT --to-destination %s\n", pmtl[i].nw_t.ifname, lan_ipaddr);
							}
							else
							{
								fprintf(fp, "-A DNSFILTER -i %s -j REDIRECT\n", pmtl[i].nw_t.ifname);
							}
						}
						else if (get_dns_filter(AF_INET, pmtl[i].sdn_t.dnsf_idx, &dnsfsrv))
						{
							fprintf(fp, "-A DNSFILTER -i %s -j DNAT --to-destination %s\n", pmtl[i].nw_t.ifname, dnsfsrv.server1);

							// VPN server
							for (j = 0; j < MTLAN_VPNS_MAXINUM; j++)
							{
								if (pmtl[i].sdn_t.vpns_idx_rl[j])
								{
									if (get_vpns_ifname_by_vpns_idx(pmtl[i].sdn_t.vpns_idx_rl[j], buf, sizeof(buf)))
									{
										fprintf(fp, "-A DNSFILTER -i %s -j DNAT --to-destination %s\n", buf, dnsfsrv.server1);
									}
									else if (get_vpns_iprange_by_vpns_idx(pmtl[i].sdn_t.vpns_idx_rl[j], buf, sizeof(buf)))
									{
										fprintf(fp, "-A DNSFILTER -m iprange --src-range %s -j DNAT --to-destination %s\n", buf, dnsfsrv.server1);
									}
									else
									{
										_dprintf("[%s] get_vpns_x_by_vpns_idx FAIL!!\n", __FUNCTION__);
									}
								}
							}
						}
					}
				}
			}
			else
			{
				_dprintf("[%s] get_mtlan_by_idx FAIL!!\n", __FUNCTION__);
			}
			FREE_MTLAN((void *)pmtl);
		}
		else
		{
			_dprintf("[%s] CANNOT init MTLAN_T!!!\n", __FUNCTION__);
		}
#endif

		/* Send other queries to the default server */
		dnsmode = nvram_get_int("dnsfilter_mode");
		if (dnsmode == DNSF_SRV_ROUTER) {
			fprintf(fp, "-A DNSFILTER -j REDIRECT\n");
		} else if ((dnsmode != DNSF_SRV_UNFILTERED) && get_dns_filter(AF_INET, dnsmode, &dnsfsrv)) {
			fprintf(fp, "-A DNSFILTER -j DNAT --to-destination %s\n", dnsfsrv.server1);
		}
	}
}

#ifdef RTCONFIG_IPV6

#ifdef HND_ROUTER
void dnsfilter6_settings_dnat(FILE *fp) {
	char *name, *mac, *mode;
	unsigned char ea[ETHER_ADDR_LEN];
	char *nv, *nvp, *rule;
	int dnsmode;
	dnsf_srv_entry_t dnsfsrv;

	if (nvram_get_int("dnsfilter_enable_x")) {
		/* Reroute all DNS requests from LAN */
		fprintf(fp, "-A PREROUTING -i br+ -p udp -m udp --dport 53 -j DNSFILTER\n"
			    "-A PREROUTING -i br+ -p tcp -m tcp --dport 53 -j DNSFILTER\n");

		/* Protection level per client */

		nv = nvp = malloc(255 * 6 + 1);
		if (nv) nvram_split_get("dnsfilter_rulelist", nv, 255 * 6 + 1, 5);

		while (nv && (rule = strsep(&nvp, "<")) != NULL) {
			if (vstrsep(rule, ">", &name, &mac, &mode) != 3)
				continue;
			if (!*mac || !*mode || !ether_atoe(mac, ea))
				continue;
			dnsmode = atoi(mode);
			if (dnsmode == DNSF_SRV_UNFILTERED) {
				fprintf(fp, "-A DNSFILTER -m mac --mac-source %s -j RETURN\n", mac);
			} else if (dnsmode == DNSF_SRV_ROUTER) {
				fprintf(fp, "-A DNSFILTER -m mac --mac-source %s -j REDIRECT\n", mac);
			} else if (get_dns_filter(AF_INET6, dnsmode, &dnsfsrv)) {
				fprintf(fp,"-A DNSFILTER -m mac --mac-source %s -j DNAT --to-destination [%s]\n",
					mac, dnsfsrv.server1);
			}
		}

		free(nv);

		/* Default behaviour */
		dnsmode = nvram_get_int("dnsfilter_mode");
		if (dnsmode == DNSF_SRV_UNFILTERED) {
			return;
		} else if (dnsmode == DNSF_SRV_ROUTER) {
			fprintf(fp, "-A DNSFILTER -j REDIRECT\n");
		} else if (get_dns_filter(AF_INET6, dnsmode, &dnsfsrv)) {	// Default server (if one exists)
			fprintf(fp, "-A DNSFILTER -j DNAT --to-destination [%s]\n", dnsfsrv.server1);
		}
	}
}
#endif

void dnsfilter6_settings_mangle(FILE *fp) {
	char *nv, *nvp, *rule;
	char *name, *mac, *mode;
	unsigned char ea[ETHER_ADDR_LEN];
	int dnsmode, count;
	dnsf_srv_entry_t dnsfsrv;
#ifdef RTCONFIG_MULTILAN_CFG
	int i;
	MTLAN_T *pmtl = NULL;
	size_t  mtl_sz = 0;
#endif

	fprintf(fp, "-A INPUT -i br+ -p udp -m udp --dport 53 -j DNSFILTERI\n"
		    "-A INPUT -i br+ -p tcp -m tcp --dport 53 -j DNSFILTERI\n"
		    "-A FORWARD -i br+ -p udp -m udp --dport 53 -j DNSFILTERF\n"
		    "-A FORWARD -i br+ -p tcp -m tcp --dport 53 -j DNSFILTERF\n");

#ifdef HND_ROUTER
	nv = nvp = malloc(255 * 6 + 1);
	if (nv) nvram_split_get("dnsfilter_rulelist", nv, 255 * 6 + 1, 5);
#else
	nv = nvp = strdup(nvram_safe_get("dnsfilter_rulelist"));
#endif
	while (nv && (rule = strsep(&nvp, "<")) != NULL) {
		if (vstrsep(rule, ">", &name, &mac, &mode) != 3)
			continue;
		dnsmode = atoi(mode);
		if (!*mac || !ether_atoe(mac, ea))
			continue;
		if (dnsmode == DNSF_SRV_UNFILTERED) {
			fprintf(fp, "-A DNSFILTERI -m mac --mac-source %s -j ACCEPT\n"
				    "-A DNSFILTERF -m mac --mac-source %s -j ACCEPT\n",
					mac, mac);
		} else {	// Filtered
			count = get_dns_filter(AF_INET6, dnsmode, &dnsfsrv);
			if (count) {
				fprintf(fp, "-A DNSFILTERF -m mac --mac-source %s -d %s -j ACCEPT\n", mac, dnsfsrv.server1);
			}
			if (count == 2) {
				fprintf(fp, "-A DNSFILTERF -m mac --mac-source %s -d %s -j ACCEPT\n", mac, dnsfsrv.server2);
			}
			// Reject other dnsfsrv for that client
			fprintf(fp, "-A DNSFILTERI -m mac --mac-source %s -j %s\n"
			            "-A DNSFILTERF -m mac --mac-source %s -j DROP\n",
			            mac, (dnsmode == DNSF_SRV_ROUTER ? "ACCEPT" : "DROP"), mac);
		}
	}
	free(nv);

#ifdef RTCONFIG_MULTILAN_CFG
		pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
		if(pmtl)
		{
			if(get_mtlan(pmtl, &mtl_sz))
			{
				for(i = 1; i < mtl_sz; ++i)
				{
					if(pmtl[i].enable && pmtl[i].sdn_t.sdn_idx && pmtl[i].nw_t.idx)
					{
						if(pmtl[i].sdn_t.dnsf_idx == DNSF_SRV_UNFILTERED)
						{
							fprintf(fp, "-A DNSFILTERI -i %s -j ACCEPT\n"
								"-A DNSFILTERF -i %s -j ACCEPT\n"
								"-A DNSFILTER_DOT -i %s -j ACCEPT\n"
							, pmtl[i].nw_t.ifname, pmtl[i].nw_t.ifname, pmtl[i].nw_t.ifname);
						}
						else
						{
							count = get_dns_filter(AF_INET6, pmtl[i].sdn_t.dnsf_idx, &dnsfsrv);
							if (count) {
								fprintf(fp, "-A DNSFILTERF -i %s -d %s -j ACCEPT\n", pmtl[i].nw_t.ifname, dnsfsrv.server1);
								if (dnsfilter_support_dot(pmtl[i].sdn_t.dnsf_idx))
									fprintf(fp, "-A DNSFILTER_DOT -i %s -d %s -j ACCEPT\n", pmtl[i].nw_t.ifname, dnsfsrv.server1);
							}
							if (count == 2) {
								fprintf(fp, "-A DNSFILTERF -i %s -d %s -j ACCEPT\n", pmtl[i].nw_t.ifname, dnsfsrv.server2);
								if (dnsfilter_support_dot(pmtl[i].sdn_t.dnsf_idx))
									fprintf(fp, "-A DNSFILTER_DOT -i %s -d %s -j ACCEPT\n", pmtl[i].nw_t.ifname, dnsfsrv.server2);
							}
							// Reject other dnsfsrv for that client
							fprintf(fp, "-A DNSFILTERI -i %s -j DROP\n"
										"-A DNSFILTERF -i %s -j DROP\n"
										"-A DNSFILTER_DOT -i %s -j DROP\n",
										pmtl[i].nw_t.ifname, pmtl[i].nw_t.ifname, pmtl[i].nw_t.ifname);
						}
					}
				}
			}
			else
			{
				_dprintf("[%s] get_mtlan_by_idx FAIL!!\n", __FUNCTION__);
			}
			FREE_MTLAN((void *)pmtl);
		}
		else
		{
			_dprintf("[%s] CANNOT init MTLAN_T!!!\n", __FUNCTION__);
		}
#endif

	dnsmode = nvram_get_int("dnsfilter_mode");
	if (dnsmode != DNSF_SRV_UNFILTERED) {
		/* Allow other queries to the default server, and drop the rest */
		count = get_dns_filter(AF_INET6, dnsmode, &dnsfsrv);
		if (count) {
			fprintf(fp, "-A DNSFILTERI -d %s -j ACCEPT\n"
				    "-A DNSFILTERF -d %s -j ACCEPT\n",
				dnsfsrv.server1, dnsfsrv.server1);
		}
		if (count == 2) {
			fprintf(fp, "-A DNSFILTERI -d %s -j ACCEPT\n"
				    "-A DNSFILTERF -d %s -j ACCEPT\n",
				dnsfsrv.server2, dnsfsrv.server2);
		}
		fprintf(fp, "-A DNSFILTERI -j %s\n"
			    "-A DNSFILTERF -j DROP\n",
		            (dnsmode == DNSF_SRV_ROUTER ? "ACCEPT" : "DROP"));
	}
}

// Non-HND cannot DNAT to the configured server, so as a partial solution,
// we have dnsmasq provide it through dhcp instead.
void dnsfilter_setup_dnsmasq(FILE *fp) {

	unsigned char ea[ETHER_ADDR_LEN];
	char *name, *mac, *mode, *enable;
	char *nv, *nvp, *b;
	int count, dnsmode, defmode, table_size;
	dnsf_srv_entry_t dnsfsrv;

	defmode = nvram_get_int("dnsfilter_mode");
	table_size = _get_table_size(1);

	for (dnsmode = 1; dnsmode < table_size; dnsmode++) {
		if (dnsmode == defmode)
			continue;
		count = get_dns_filter(AF_INET6, dnsmode, &dnsfsrv);
		if (count == 0 && dnsmode == DNSF_SRV_ROUTER) {
			/* Workaround dynamic router address */
			strcpy(dnsfsrv.server1, "::");
			count = 1;
		}
		if (count == 0)
			continue;
		fprintf(fp, "dhcp-option=dnsf%u,option6:23,[%s]", dnsmode, dnsfsrv.server1);
		if (count == 2)
			fprintf(fp, ",[%s]", dnsfsrv.server2);
		fprintf(fp, "\n");
	}

	/* DNS server per client */
#ifdef HND_ROUTER
	nv = nvp = malloc(255 * 6 + 1);
	if (nv) nvram_split_get("dnsfilter_rulelist", nv, 255 * 6 + 1, 5);
#else
	nv = nvp = strdup(nvram_safe_get("dnsfilter_rulelist"));
#endif

	while (nv && (b = strsep(&nvp, "<")) != NULL) {
		if (vstrsep(b, ">", &name, &mac, &mode, &enable) < 3)
			continue;
		if (enable && atoi(enable) == 0)
			continue;
		if (!*mac || !*mode || !ether_atoe(mac, ea))
			continue;
		dnsmode = atoi(mode);
		/* Skip unfiltered, default, or non-IPv6 capable levels */
		if ((dnsmode == DNSF_SRV_UNFILTERED) || (dnsmode == defmode) || (get_dns_filter(AF_INET6, dnsmode, &dnsfsrv) == 0))
			continue;
		fprintf(fp, "dhcp-host=%s,set:dnsf%u\n", mac, dnsmode);
	}
	free(nv);
}


// Block DOT if the configured server isn't known to support DOT, to prevent bypassing dnsfilter with DOT
void dnsfilter6_dot_rules(FILE *fp)
{
	char *name, *mac, *mode;
	unsigned char ea[ETHER_ADDR_LEN];
	char *nv, *nvp, *rule;
	int dnsmode;
	dnsf_srv_entry_t dnsfsrv;

	if (nvram_get_int("dnsfilter_enable_x") == 0) return;

	fprintf(fp, "-A FORWARD -i br+ -m tcp -p tcp --dport 853 -j DNSFILTER_DOT\n");

	nv = nvp = malloc(255 * 6 + 1);
	if (nv) nvram_split_get("dnsfilter_rulelist", nv, 255 * 6 + 1, 5);

	while (nv && (rule = strsep(&nvp, "<")) != NULL) {
		if (vstrsep(rule, ">", &name, &mac, &mode) != 3)
			continue;
		if (!*mac || !*mode || !ether_atoe(mac, ea))
			continue;
		dnsmode = atoi(mode);
		if (dnsmode == DNSF_SRV_UNFILTERED)
			fprintf(fp, "-A DNSFILTER_DOT -m mac --mac-source %s -j RETURN\n", mac);
		else if (dnsfilter_support_dot(dnsmode) && get_dns_filter(AF_INET6, dnsmode, &dnsfsrv) > 0 )	// Filter supports DOT
			fprintf(fp, "-A DNSFILTER_DOT -m mac --mac-source %s ! -d %s -j REJECT\n", mac, dnsfsrv.server1);
		else	// Reject DOT access
			fprintf(fp, "-A DNSFILTER_DOT -m mac --mac-source %s -j REJECT\n", mac);
	}
	free(nv);

	/* Global filtering */
	dnsmode = nvram_get_int("dnsfilter_mode");
	if (dnsmode != DNSF_SRV_UNFILTERED) {
		if (dnsfilter_support_dot(dnsmode) && get_dns_filter(AF_INET6, dnsmode, &dnsfsrv) > 0 )
			fprintf(fp, "-A DNSFILTER_DOT ! -d %s -j REJECT\n", dnsfsrv.server1);
		else
			fprintf(fp, "-A DNSFILTER_DOT -j REJECT\n");
	}
}
#endif	// RTCONFIG_IPV6


void dnsfilter_dot_rules(FILE *fp)
{
	char *name, *mac, *mode;
	unsigned char ea[ETHER_ADDR_LEN];
	char *nv, *nvp, *rule;
	int dnsmode;
	dnsf_srv_entry_t dnsfsrv;
#ifdef RTCONFIG_MULTILAN_CFG
	int i;
	MTLAN_T *pmtl = NULL;
	size_t  mtl_sz = 0;
#endif

	if (nvram_get_int("dnsfilter_enable_x") == 0) return;

	fprintf(fp, "-A FORWARD -i br+ -m tcp -p tcp --dport 853 -j DNSFILTER_DOT\n");

#ifdef HND_ROUTER
	nv = nvp = malloc(255 * 6 + 1);
	if (nv) nvram_split_get("dnsfilter_rulelist", nv, 255 * 6 + 1, 5);
#else
	nv = nvp = strdup(nvram_safe_get("dnsfilter_rulelist"));
#endif
	while (nv && (rule = strsep(&nvp, "<")) != NULL) {
		if (vstrsep(rule, ">", &name, &mac, &mode) != 3)
			continue;
		if (!*mac || !*mode || !ether_atoe(mac, ea))
			continue;
		dnsmode = atoi(mode);
		if (dnsmode == DNSF_SRV_UNFILTERED)
			fprintf(fp, "-A DNSFILTER_DOT -m mac --mac-source %s -j RETURN\n", mac);
		else if (dnsfilter_support_dot(dnsmode) && get_dns_filter(AF_INET, dnsmode, &dnsfsrv) > 0 )	// Filter supports DOT
			fprintf(fp, "-A DNSFILTER_DOT -m mac --mac-source %s ! -d %s -j REJECT\n", mac, dnsfsrv.server1);
		else	// Reject DOT access
			fprintf(fp, "-A DNSFILTER_DOT -m mac --mac-source %s -j REJECT\n", mac);
	}
	free(nv);

#ifdef RTCONFIG_MULTILAN_CFG
	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if(pmtl)
	{
		if(get_mtlan(pmtl, &mtl_sz))
		{
			for(i = 1; i < mtl_sz; ++i)
			{
				if(pmtl[i].enable)
				{
					if(pmtl[i].sdn_t.dnsf_idx != DNSF_SRV_UNFILTERED)
					{
						if (dnsfilter_support_dot(pmtl[i].sdn_t.dnsf_idx) && get_dns_filter(AF_INET, pmtl[i].sdn_t.dnsf_idx, &dnsfsrv) > 0 )
						{
							if (pmtl[i].sdn_t.dnsf_idx == DNSF_SRV_ROUTER && *pmtl[i].nw_t.addr)
								fprintf(fp, "-A DNSFILTER_DOT -i %s ! -d %s -j REJECT\n", pmtl[i].nw_t.ifname, pmtl[i].nw_t.addr);
							else
								fprintf(fp, "-A DNSFILTER_DOT -i %s ! -d %s -j REJECT\n", pmtl[i].nw_t.ifname, dnsfsrv.server1);
						}
						else
							fprintf(fp, "-A DNSFILTER_DOT -i %s -j REJECT\n", pmtl[i].nw_t.ifname);
					}
				}
			}
		}
		FREE_MTLAN((void *)pmtl);
	}
#endif

	/* Global filtering */
	dnsmode = nvram_get_int("dnsfilter_mode");
	if (dnsmode != DNSF_SRV_UNFILTERED) {
		if (dnsfilter_support_dot(dnsmode) && get_dns_filter(AF_INET, dnsmode, &dnsfsrv) > 0 )
			fprintf(fp, "-A DNSFILTER_DOT ! -d %s -j REJECT\n", dnsfsrv.server1);
		else
			fprintf(fp, "-A DNSFILTER_DOT -j REJECT\n");
	}
}

#endif	// RTCONFIG_DNSFILTER
