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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <httpd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <dirent.h>

#include <typedefs.h>
#include <bcmutils.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <bcmnvram_f.h>
#include <common.h>
#include <shared.h>
#include <rtstate.h>
#ifdef HND_ROUTER
#include "bcmwifi_rates.h"
#include "wlioctl_defs.h"
#endif

#include <wlioctl.h>
#include <wlutils.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <linux/version.h>

#include "data_arrays.h"
#include "httpd.h"
#include "iptraffic.h"

#include <net/route.h>

#ifdef RTCONFIG_BWDPI
#define __USE_GNU
#include "bwdpi_common.h"
#include <search.h>
#endif

#include <json.h>

int
ej_get_leases_array(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	struct in_addr addr4;
	struct in6_addr addr6;
	char line[256];
	char *hwaddr, *ipaddr, *name, *next;
	unsigned int expires;
	int ret=0;
#ifdef RTCONFIG_MULTILAN_CFG
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i, vlanid = 0;
	char path[128] = {0};
#endif

	killall("dnsmasq", SIGUSR2);
	sleep(1);

	ret += websWrite(wp, "leasearray= [");

#ifdef RTCONFIG_MULTILAN_CFG
	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl) {
		get_mtlan(pmtl, &mtl_sz);
		for (i = 0; i < mtl_sz; i++) {
			if (pmtl[i].enable && pmtl[i].nw_t.dhcp_enable) {
				if (i) {
					snprintf(path, sizeof(path), "/var/lib/misc/dnsmasq-%d.leases", pmtl[i].sdn_t.sdn_idx);
					vlanid = pmtl[i].vid;
				} else
					strlcpy(path, "/var/lib/misc/dnsmasq.leases", sizeof(path));
				fp = fopen(path, "r");
				if (!fp) continue;
#else
	if (!(fp = fopen("/var/lib/misc/dnsmasq.leases", "r")))
		return ret + websWrite(wp, "];\n");
#endif

	while ((next = fgets(line, sizeof(line), fp)) != NULL) {
		/* line should start with a numeric value */
		if (sscanf(next, "%u ", &expires) != 1)
			continue;

		strsep(&next, " ");
		hwaddr = strsep(&next, " ") ? : "";
		ipaddr = strsep(&next, " ") ? : "";
		name = strsep(&next, " ") ? : "";

		if (strlen(name) > 32) {
			strcpy(name + 29, "...");
			name[32] = '\0';
		}

		if (inet_pton(AF_INET6, ipaddr, &addr6) != 0) {
			/* skip ipv6 leases, they have no hwaddr, but client id */
			// hwaddr = next ? : "";
			continue;
		} else if (inet_pton(AF_INET, ipaddr, &addr4) == 0)
			continue;
#ifdef RTCONFIG_MULTILAN_CFG
		ret += websWrite(wp, "[\"%d\", \"%s\", \"%s\", \"%s\", \"%d\"],\n", expires, hwaddr, ipaddr, name, vlanid);
#else
		ret += websWrite(wp, "[\"%d\", \"%s\", \"%s\", \"%s\"],\n", expires, hwaddr, ipaddr, name);
#endif
	}
	fclose(fp);

#ifdef RTCONFIG_MULTILAN_CFG
			}
		}
		FREE_MTLAN((void *)pmtl);
	}
#endif
	ret += websWrite(wp, "[]];\n");

	return ret;
}


#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_IGD2
int
ej_ipv6_pinhole_array(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	char proto[4], raddr[45], rport[6], iaddr[45], iport[6], timestamp[15], desc[200], desc2[256];
	char line[256];
	int ret = 0;
	char *token, *line_dup;
	int len, count;

	ret = websWrite(wp, "var pinholesarray = [");

	if (!(ipv6_enabled() && is_routing_enabled())) {
		ret += websWrite(wp, "[]];\n");
		return ret;
	}

	killall("miniupnpd", SIGUSR2);
	sleep(1);

	fp = fopen("/tmp/upnp.leases6", "r");
	if (fp == NULL) {
		ret += websWrite(wp, "[]];\n");
		return ret;
	}

	while (fgets(line, sizeof(line), fp) != NULL)
	{
// TCP;2600:1000:100:200:300:1234:5678:9abc;40003;::;0;1;1710906282;IGD2 pinhole

		line_dup = strdup(line);
			if (line_dup == NULL) {
			break;
		}

		count = 0;
		while ((count < 8) && (token = strsep(&line_dup, ";")) != NULL) {

			switch (count) {
				case 0:
					strlcpy(proto, token, sizeof(proto));
					break;
				case 1:
		                        strlcpy(iaddr, token, sizeof(iaddr));
		                        break;
				case 2:
		                        strlcpy(iport, token, sizeof(iport));
		                        break;
				case 3:
		                        strlcpy(raddr, token, sizeof(raddr));
		                        break;
				case 4:
		                        strlcpy(rport, token, sizeof(rport));
		                        break;
				case 5:	// Skip UUID
					break;
				case 6:
					strlcpy(timestamp, token, sizeof(timestamp));
					count++;
					// fallthrough so we get the remainder, even including delimiter chars
				case 7:
					token = strsep(&line_dup, "");
					if (token)
						strlcpy(desc, token, sizeof(desc));
					else
						strcpy(desc, "");
					break;
			}

			count++;
		}

		if (count != 8) continue;	// Incomplete entry

		len = strlen(desc);
		if (len > 0 && desc[len-1] == '\n')
			desc[len-1] = '\0';

		if (str_escape_quotes(desc2, desc, sizeof(desc2)) == 0)
			strlcpy(desc2, desc, sizeof(desc2));

		/* parse remote ip */
		if (raddr[0] == '\0' || strcmp(raddr, "::") == 0)
			strcpy(raddr, "ANY");

		/* parse internal ip */
		if (iaddr[0] == '\0' || strcmp(iaddr, "::") == 0)
			strcpy(iaddr, "ANY");

		/* parse remote port */
		if (strcmp(rport, "0") == 0)
			strcpy(rport, "ANY");

		ret += websWrite(wp, "[\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"],\n",
		                      proto, raddr, rport, iaddr, iport, timestamp, desc2);

		free(line_dup);
	}

	ret += websWrite(wp, "[]];\n");

	fclose(fp);
	return ret;
}
#endif
#endif


int
ej_get_upnp_array(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	char proto[4], eport[6], iaddr[sizeof("255.255.255.255")], iport[6], timestamp[15], desc[200], desc2[256];
	int ret=0;
	char line[256];

	killall("miniupnpd", SIGUSR2);
        sleep(1);

	ret += websWrite(wp, "var upnparray = [");

	fp = fopen("/tmp/upnp.leases", "r");
	if (fp == NULL) {
		ret += websWrite(wp, "[]];\n");
		return ret;
	}

	while (fgets(line, sizeof(line), fp) != NULL)
	{
		desc[0] = '\0';

		if (sscanf(line,
			"%3[^:]:"
			"%5[^:]:"
			"%15[^:]:"
			"%5[^:]:"
			"%14[^:]:"
			"%199[^\n]",
			proto, eport, iaddr, iport, timestamp, desc) < 5) continue;

		if (str_escape_quotes(desc2, desc, sizeof(desc2)) == 0)
			strlcpy(desc2, desc, sizeof(desc2));

		ret += websWrite(wp, "[\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"],\n",
			proto, eport, iaddr, iport, timestamp, desc2);
	}

	fclose(fp);

	ret += websWrite(wp, "[]];\n");
	return ret;

}

int
ej_get_vserver_array(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	char *nat_argv[] = {"iptables", "-t", "nat", "-nxL", NULL};
	char line[256], tmp[256];
	char target[16], proto[16];
	char src[19];
	char dst[19];
	char *range, *host, *port, *ptr, *val;
	int ret = 0;
	char chain[16];

	/* dump nat table including VSERVER and VUPNP chains */
	_eval(nat_argv, ">/tmp/vserver.log", 10, NULL);

	ret += websWrite(wp, "var vserverarray = [");

	fp = fopen("/tmp/vserver.log", "r");

	if (fp == NULL) {
                ret += websWrite(wp, "[]];\n");
                return ret;
        }

	while (fgets(line, sizeof(line), fp) != NULL)
	{

		// If it's a chain definition then store it for following rules
		if (!strncmp(line, "Chain",  5)){
			if (sscanf(line, "%*s%*[ \t]%15s%*[ \t]%*s", chain) == 1)
				continue;
		}
		tmp[0] = '\0';
		if (sscanf(line,
		    "%15s%*[ \t]"		// target
		    "%15s%*[ \t]"		// prot
		    "%*s%*[ \t]"		// opt
		    "%18s%*[ \t]"		// source
		    "%15[^/]/%*d%*[ \t]"	// destination
		    "%255[^\n]",		// options
		    target, proto, src, dst, tmp) < 5) continue;

		/* TODO: add port trigger, portmap, etc support */
		if (strcmp(target, "DNAT") != 0)
			continue;

		/* Don't list DNS redirections  from DNSFilter or UPNP */
		if ((strcmp(chain, "DNSFILTER") == 0) || (strcmp(chain, "VUPNP") == 0) || (strcmp(chain, "PUPNP") == 0) || (strncmp(chain, "DNSVPN", 6) == 0))
			continue;

		/* uppercase proto */
		for (ptr = proto; *ptr; ptr++)
			*ptr = toupper(*ptr);
		/* parse source */
		if (strcmp(src, "0.0.0.0/0") == 0)
			strcpy(src, "ALL");
		/* parse destination */
		if (strcmp(dst, "0.0.0.0/0") == 0)
			strcpy(dst, "ALL");

		/* parse options */
		port = host = range = "";
		ptr = tmp;
		while ((val = strsep(&ptr, " ")) != NULL) {
			if (strncmp(val, "dpt:", 4) == 0)
				range = val + 4;
			else if (strncmp(val, "dpts:", 5) == 0)
				range = val + 5;
			else if (strncmp(val, "to:", 3) == 0) {
				port = host = val + 3;
				strsep(&port, ":");
			}
		}

		ret += websWrite(wp, "["
			"\"%s\", "
			"\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"],\n",
			src,
			dst, proto, range, host, port ? : range, chain);
	}
	fclose(fp);
	unlink("/tmp/vserver.log");

        ret += websWrite(wp, "[]];\n");
	return ret;
}



static int ipv4_route_table_array(webs_t wp)
{
	FILE *fp;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char buf[256], *dev, *sflags, *str;
	struct in_addr dest, gateway, mask;
	int flags, ref, use, metric;
	int unit, ret = 0;

	fp = fopen("/proc/net/route", "r");
	if (fp == NULL) {
		ret += websWrite(wp, "[]];\n");
		return ret;
	}

	while ((str = fgets(buf, sizeof(buf), fp)) != NULL) {
		dev = strsep(&str, " \t");
		if (!str || dev == str)
			continue;
		if (sscanf(str, "%x%x%x%d%u%d%x", &dest.s_addr, &gateway.s_addr,
			&flags, &ref, &use, &metric, &mask.s_addr) != 7)
			continue;

		/* Parse flags, reuse buf */
		sflags = str;
		if (flags & RTF_UP)
			*str++ = 'U';
		if (flags & RTF_GATEWAY)
			*str++ = 'G';
		if (flags & RTF_HOST)
			*str++ = 'H';
		*str++ = '\0';

		/* Skip interfaces here */
		if (strcmp(dev, "lo") == 0)
			continue;

		/* Replace known interfaces with LAN/WAN/MAN */
		if (nvram_match("lan_ifname", dev)) /* br0, wl0, etc */
			dev = "LAN";
		else {
			/* Tricky, it's better to move wan_ifunit/wanx_ifunit to shared instead */
			for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; unit++) {
				snprintf(prefix, sizeof(prefix), "wan%d_", unit);
				if (nvram_match(strcat_r(prefix, "pppoe_ifname", tmp), dev)) {
					dev = "WAN";
					break;
				}
				if (nvram_match(strcat_r(prefix, "ifname", tmp), dev)) {
					char *wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));
					dev = (strcmp(wan_proto, "dhcp") == 0 ||
						strcmp(wan_proto, "static") == 0 ) ? "WAN" : "MAN";
					break;
				}
			}
		}

		ret += websWrite(wp, "[\"%s\",",  dest.s_addr == INADDR_ANY ? "default" : inet_ntoa(dest));
		ret += websWrite(wp, "\"%s\", ", gateway.s_addr == INADDR_ANY ? "*" : inet_ntoa(gateway));
		ret += websWrite(wp, "\"%s\", \"%s\", \"%d\", \"%d\", \"%d\", \"%s\"],\n",
                       inet_ntoa(mask), sflags, metric, ref, use, dev);

	}
	fclose(fp);

	return ret;
}


#ifndef RTF_PREFIX_RT
#define RTF_PREFIX_RT  0x00080000
#endif
#ifndef RTF_EXPIRES
#define RTF_EXPIRES    0x00400000
#endif
#ifndef RTF_ROUTEINFO
#define RTF_ROUTEINFO  0x00800000
#endif

const static struct {
       unsigned int flag;
       char name;
} route_flags[] = {
       { RTF_UP,        'U' },
       { RTF_GATEWAY,   'G' },
       { RTF_HOST,      'H' },
//     { RTF_DYNAMIC,   'N' },
//     { RTF_MODIFIED,  'M' },
       { RTF_REJECT,    'R' },
#ifdef RTCONFIG_IPV6
       { RTF_DEFAULT,   'D' },
       { RTF_ADDRCONF,  'A' },
       { RTF_PREFIX_RT, 'P' },
       { RTF_EXPIRES,   'E' },
       { RTF_ROUTEINFO, 'I' },
//     { RTF_CACHE,     'C' },
//     { RTF_LOCAL,     'L' },
#endif
};


#ifdef RTCONFIG_IPV6
static
int INET6_displayroutes_array(webs_t wp)
{
	FILE *fp;
	char buf[256], *str, *dev, *sflags;
	char sdest[INET6_ADDRSTRLEN], snexthop[INET6_ADDRSTRLEN], ifname[16];
	struct in6_addr dest, nexthop;
	int flags, ref, use, metric, prefix;
	int i, pass, maxlen, routing, ret = 0;

	fp = fopen("/proc/net/ipv6_route", "r");
	if (fp == NULL)
		return 0;

	pass = maxlen = 0;
	routing = is_routing_enabled();
again:
	while ((str = fgets(buf, sizeof(buf), fp)) != NULL) {
		if (sscanf(str, "%32s%x%*s%*x%32s%x%x%x%x%15s",
			   sdest, &prefix, snexthop,
			   &metric, &ref, &use, &flags, ifname) != 8)
			continue;

		/* Skip down and cache routes */
		if ((flags & (RTF_UP | RTF_CACHE)) != RTF_UP)
			continue;
		/* Skip interfaces here */
		if (strcmp(ifname, "lo") == 0)
			continue;

		/* Parse dst, reuse buf */
		if (inet_raddr6_pton(sdest, &dest, str) < 1)
			break;
		if (prefix || !IN6_IS_ADDR_UNSPECIFIED(&dest)) {
			inet_ntop(AF_INET6, &dest, sdest, sizeof(sdest));
			if (prefix != 128) {
				i = strlen(sdest);
				snprintf(sdest + i, sizeof(sdest) - i, "/%d", prefix);
			}
		} else
			snprintf(sdest, sizeof(sdest), "default");

		/* Parse nexthop, reuse buf */
		if (inet_raddr6_pton(snexthop, &nexthop, str) < 1)
			break;
		inet_ntop(AF_INET6, &nexthop, snexthop, sizeof(snexthop));

		/* Format addresses, reuse buf */
		i = snprintf(str, buf + sizeof(buf) - str, ((flags & RTF_NONEXTHOP) ||
			     IN6_IS_ADDR_UNSPECIFIED(&nexthop)) ? "%s" : "%s via %s",
			     sdest, snexthop);
		if (pass == 0) {
			if (maxlen < i)
				maxlen = i;
			continue;
		} else
			str += i + 1;

		/* Parse flags, reuse buf */
		sflags = str;
		for (i = 0; i < ARRAY_SIZE(route_flags); i++) {
			if (flags & route_flags[i].flag)
				*str++ = route_flags[i].name;
		}
		*str++ = '\0';

		/* Replace known interfaces with LAN/WAN/MAN */
		dev = NULL;
		if (nvram_match("lan_ifname", ifname)) /* br0, wl0, etc */
			dev = "LAN";
		else if (routing && strcmp(get_wan6face(), ifname) == 0)
			dev = "WAN";

		/* Print the info. */
		ret += websWrite(wp, "[\"%3s\", \"%s\", \"%s\", \"%d\", \"%d\", \"%d\", \"%s\", \"%s\"],\n",
		                      sdest, ((flags & RTF_NONEXTHOP) || IN6_IS_ADDR_UNSPECIFIED(&nexthop)) ? "" : snexthop,
                                      sflags, metric, ref, use, dev ? : "", ifname);

	}
	if (pass++ == 0) {
		if (maxlen > 0)
			rewind(fp);
		goto again;
	}
	fclose(fp);

	return ret;
}
#endif


int
ej_get_route_array(int eid, webs_t wp, int argc, char_t **argv)
{
	int ret = 0;

	ret += websWrite(wp, "var routearray = [");
	ret += ipv4_route_table_array(wp);
	ret += websWrite(wp, "[]];\n");

	ret += websWrite(wp, "var routev6array = [");
#ifdef RTCONFIG_IPV6
	if (get_ipv6_service() != IPV6_DISABLED) {
		INET6_displayroutes_array(wp);
	}
#endif

	ret += websWrite(wp, "[]];\n");
	return ret;
}

#ifdef RTCONFIG_IPV6
int
ej_lan_ipv6_network_array(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	char buf[64+32+8192+1];
	char *hostname, *macaddr, ipaddrs[8192+1];
	char ipv6_dns_str[1024];
	char *wan_type, *wan_dns, *p;
	int service, i, ret = 0;

	ret += websWrite(wp, "var ipv6cfgarray = [");

	if (!(ipv6_enabled() && is_routing_enabled())) {
		ret += websWrite(wp, "[]];\n");
		ret += websWrite(wp, "var ipv6clientarray = [");
		ret += websWrite(wp, "[]];\n");
		return ret;
	}

	service = get_ipv6_service();
	switch (service) {
	case IPV6_NATIVE_DHCP:
		wan_type = "Native with DHCP-PD"; break;
	case IPV6_6TO4:
		wan_type = "Tunnel 6to4"; break;
	case IPV6_6IN4:
		wan_type = "Tunnel 6in4"; break;
	case IPV6_6RD:
		wan_type = "Tunnel 6rd"; break;
	case IPV6_MANUAL:
		wan_type = "Static"; break;
	default:
		wan_type = "Disabled"; break;
	}

	ret += websWrite(wp, "[\"IPv6 Connection Type\",\"%s\"],", wan_type);

	ret += websWrite(wp, "[\"WAN IPv6 Address\",\"%s\"],",
			 getifaddr((char *) get_wan6face(), AF_INET6, GIF_PREFIXLEN) ? : nvram_safe_get("ipv6_rtr_addr"));


	ret += websWrite(wp, "[\"WAN IPv6 Gateway\",\"%s\"],",
			 ipv6_gateway_address() ? : "");

	ret += websWrite(wp, "[\"LAN IPv6 Address\",\"%s/%d\"],",
			 nvram_safe_get("ipv6_rtr_addr"), nvram_get_int("ipv6_prefix_length"));

	ret += websWrite(wp, "[\"LAN IPv6 Link-Local Address\",\"%s\"],",
			 getifaddr(nvram_safe_get("lan_ifname"), AF_INET6, GIF_LINKLOCAL | GIF_PREFIXLEN) ? : "");

	if (service == IPV6_NATIVE_DHCP) {
		ret += websWrite(wp, "[\"DHCP-PD\",\"%s\"],",
			 nvram_get_int("ipv6_dhcp_pd") ? "Enabled" : "Disabled");
	}

	ret += websWrite(wp, "[\"LAN IPv6 Prefix\",\"%s/%d\"],",
			 nvram_safe_get("ipv6_prefix"), nvram_get_int("ipv6_prefix_length"));

	if (service == IPV6_NATIVE_DHCP &&
	    nvram_get_int("ipv6_dnsenable")) {
		wan_dns = nvram_safe_get("ipv6_get_dns");
	} else {
		char nvname[sizeof("ipv6_dnsXXX")];
		char *next = ipv6_dns_str;

		ipv6_dns_str[0] = '\0';
		for (i = 1; i <= 3; i++) {
			snprintf(nvname, sizeof(nvname), "ipv6_dns%d", i);
			wan_dns = nvram_safe_get(nvname);
			if (*wan_dns)
				next += sprintf(next, *ipv6_dns_str ? " %s" : "%s", wan_dns);
		}
		wan_dns = ipv6_dns_str;
	}

	ret += websWrite(wp, "[\"DNS Address\",\"%s\"],", wan_dns);
	ret += websWrite(wp, "[]];\n");

	ret += websWrite(wp, "var ipv6clientarray = [");

	/* Refresh lease file to get actual expire time */
	killall("dnsmasq", SIGUSR2);
	usleep(100 * 1000);

	get_ipv6_client_info();
	get_ipv6_client_list();

	if ((fp = fopen(IPV6_CLIENT_LIST, "r")) == NULL) {
		_dprintf("can't open %s: %s", IPV6_CLIENT_LIST, strerror(errno));
		return ret;
	}

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		char *ptr = buf;

		ptr = strsep(&ptr, "\n");
		hostname = strsep(&ptr, " ");
		macaddr = strsep(&ptr, " ");
		if (!macaddr || *macaddr == '\0' ||
		    !ptr || *ptr == '\0')
			continue;

		if (strlen(hostname) > 32)
			sprintf(hostname + 29, "...");

		ipaddrs[0] = '\0';
		p = ipaddrs;
		while (ptr && *ptr) {
			char *next = strsep(&ptr, ",\n");
			if (next && *next)
				p += snprintf(p, sizeof(ipaddrs) + ipaddrs - p, "%s%s", *ipaddrs ? ", " : "", next);
		}

		ret += websWrite(wp, "[\"%s\", \"%s\", \"%s\"],",
				 hostname, macaddr, ipaddrs);
	}
	fclose(fp);

	ret += websWrite(wp, "[]];\n");
	return ret;
}

int
ej_lan_ipv6_clients_array(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	char buf[64+32+8192+1];
	char addrbuf[64];
	char *hostname, *macaddr;
	int ret = 0;
#if 0
	int len;
#endif

	ret += websWrite(wp, "var ipv6clientarray = {};\n");

	if (!(ipv6_enabled() && is_routing_enabled()))
		return ret;

	/* Refresh lease file to get actual expire time */
	killall("dnsmasq", SIGUSR2);
	usleep(100 * 1000);

	get_ipv6_client_info();
	get_ipv6_client_list();

	if ((fp = fopen(IPV6_CLIENT_LIST, "r")) == NULL) {
		_dprintf("can't open %s: %s", IPV6_CLIENT_LIST, strerror(errno));
		return ret;
	}

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		char *ptr = buf;

		ptr = strsep(&ptr, "\n");
		hostname = strsep(&ptr, " ");
		macaddr = strsep(&ptr, " ");
		if (!macaddr || *macaddr == '\0' ||
		    !ptr || *ptr == '\0' ||
		    !strcmp(hostname, "*") ||
		    !strcmp(hostname, "<unknown>"))
			continue;

		if (strlen(hostname) > 32)
			sprintf(hostname + 29, "...");

		while (ptr && *ptr) {
			char *next = strsep(&ptr, ",\n");
			if (next && *next) {
				/* Workaround - TrendMicro truncates last two bytes, and we pad them with "00" to generate a valid IP  */
				strlcpy(addrbuf, next, sizeof(addrbuf));
#if 0	// Disable workaround
				len = strlen(addrbuf);
				if (len > 2) {
					addrbuf[len-1] = '0';
					addrbuf[len-2] = '0';
				}
#endif
				ret += websWrite(wp, "ipv6clientarray[\"%s\"] = \"%s\";\n", addrbuf, hostname);
			}
		}
	}
	fclose(fp);

	return ret;
}
#endif


#ifdef RTCONFIG_BWDPI
int parseTcFilter(webs_t wp, const char *interface) {
	char command[256];
	char buffer[1024];
	int foundmark=-1, foundflowid=-1, lastmark=-1;
	FILE *fp;
	int ret;
	char *mark = NULL;
	char *flowid = NULL;
	int value, value2;

	ret = websWrite(wp, "var tcdata_filter_array = [];\n");

	if (nvram_get_int("qos_type") != 1 || nvram_get_int("qos_enable") == 0)
		return ret;

	snprintf(command, sizeof(command), "tc filter show dev %s", interface);

	if ((fp = popen(command, "r")) == NULL)
		return ret;

	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		flowid = strstr(buffer, "flowid ");
		if (flowid) {
			sscanf(flowid, "flowid 1:%d", &foundflowid);
			if (foundflowid < 10 || foundflowid > 20) {
				foundflowid = -1;
			}
			continue;
		}

		mark = strstr(buffer, "mark ");
		if (mark) {
			if (sscanf(mark, "mark %x %x", &value, &value2) != 2 || value2 != 0xc03f0000) {
				foundflowid = -1;
				continue;
			}
			foundmark = (value & 0x3F0000)/0xFFFF;
			if (foundmark == lastmark) {
				foundmark += 50;
			}
		}

		if (foundflowid != -1 && foundmark != -1) {
			ret += websWrite(wp, "tcdata_filter_array[%d] = %d;\n", foundmark,foundflowid);
			lastmark = foundmark;
			foundflowid = -1;
			foundmark = -1;
		}
	}

	pclose(fp);
	return ret;
}


int ej_tcfilter_array(int eid, webs_t wp, int argc, char_t **argv) {

	return parseTcFilter(wp, "br0");
}
#endif

int ej_tcclass_dump_array(int eid, webs_t wp, int argc, char_t **argv) {
	FILE *fp;
	int ret = 0;
	char tmp[64];
	int qos_type;
	char wan_ifname[16];

	strlcpy(wan_ifname, nvram_safe_get("wan_ifname"), sizeof(wan_ifname));
	if (*wan_ifname == '\0')
		strlcpy(wan_ifname, "eth0", sizeof(wan_ifname));

	qos_type = nvram_get_int("qos_type");

	if (nvram_get_int("qos_enable") == 0) {
		ret += websWrite(wp, "var tcdata_lan_array = [[]];\nvar tcdata_wan_array = [[]];\n");
		return ret;
	}

	if (qos_type == 0 || qos_type == 1 || qos_type == 3) {	// Traditional, Adaptive or Geforce Now
		system("tc -s class show dev br0 > /tmp/tcclass.txt");
		system("tc -s class show dev imq0 >> /tmp/tcclass.txt");

		ret += websWrite(wp, "var tcdata_lan_array = [\n");

		fp = fopen("/tmp/tcclass.txt","r");
		if (fp) {
			ret += tcclass_dump(fp, wp);
			fclose(fp);
		} else {
			ret += websWrite(wp, "[]];\n");
		}
		unlink("/tmp/tcclass.txt");

	}

	if (qos_type != 2) {	// Must not be BW Limiter
		snprintf(tmp, sizeof(tmp), "tc -s class show dev %s > /tmp/tcclass.txt", wan_ifname);
		system(tmp);

	        ret += websWrite(wp, "var tcdata_wan_array = [\n");

	        fp = fopen("/tmp/tcclass.txt","r");
	        if (fp) {
	                ret += tcclass_dump(fp, wp);
			fclose(fp);
		} else {
			ret += websWrite(wp, "[]];\n");
	        }
		unlink("/tmp/tcclass.txt");
	}

	return ret;
}


int tcclass_dump(FILE *fp, webs_t wp) {
	char buf[256], ratebps[16], ratepps[16];
	int tcclass = 0;
	int stage = 0;
	unsigned long long traffic;
	int ret = 0;
	const char *class_template;
	int qos_type = nvram_get_int("qos_type");

	if (qos_type == 0 || qos_type == 3) {	//Traditional and geforce now, have both 1:* and 2:*
		class_template = "class htb %*c:%d %*s";
	} else {
		class_template = "class htb 1:%d %*s";
	}

	while (fgets(buf, sizeof(buf) , fp)) {
		switch (stage) {
			case 0:	// class
				if (sscanf(buf, class_template, &tcclass) == 1) {
					// Skip roots 1:1 and 1:2, and skip 1:60 in tQoS since it's BCM's download class
					if ( (tcclass < 10) || ((qos_type == 0) && (tcclass == 60))) {
						continue;
					}
					ret += websWrite(wp, "[\"%d\",", tcclass);
					stage = 1;
				}
				break;
			case 1: // Total data
				if (sscanf(buf, " Sent %llu bytes %*d pkt %*s)", &traffic) == 1) {
					ret += websWrite(wp, " \"%llu\",", traffic);
					stage = 2;
				}
				break;
			case 2: // Rates
				if (sscanf(buf, " rate %15s %15s backlog %*s", ratebps, ratepps) == 2) {
					ret += websWrite(wp, " \"%s\", \"%s\"],\n", ratebps, ratepps);
					stage = 0;
				}
				break;
			default:
				break;
		}
	}
	ret += websWrite(wp, "[]];\n");
	return ret;
}


int ej_connlist_array(int eid, webs_t wp, int argc, char **argv) {
	FILE *fp;
	char line[150];
	char proto[6], address[INET6_ADDRSTRLEN], dest[INET6_ADDRSTRLEN], state[15];
	int ret = 0, parsed, port1, port2;

	ret += websWrite(wp, "var connarray = [");

	system("/usr/sbin/netstat-nat -no -X50 > /tmp/connect.log 2>&1");

	fp = fopen("/tmp/connect.log", "r");
	if (fp == NULL) {
		websWrite(wp, "[]];\n");
		return ret;
	}

	while (fgets(line, sizeof(line), fp)) {
		if (!strncmp(line,"icmp",4)) {
			if (sscanf(line,
			    "%5s%*[ \t]"
			    "%15[0-9.]%*[ \t]"
			    "%15[0-9.]%*[ \t]",
			    proto, address, dest) != 3) continue;
			state[0] = '\0';
			port2 = 0;
			port1 = 0;
		} else {
			// Clear it as it might be absent
			state[0] = '\0';

			// Try IPv6 format (brackets present)
			if (strchr(line, '[')) {
				parsed = sscanf(line,
				         "%3s "
                                         "[%47[^]]]:%d "
				         "[%47[^]]]:%d"
				         "%14s%*[ \t]",
				         proto, address, &port1, dest, &port2, state);
			} else {
				parsed = sscanf(line,
				         "%3s "
					 "%47[^:]:%d "
			                 "%47[^:]:%d"
				         "%14s%*[ \t]",
				         proto, address, &port1, dest, &port2, state);
			}

			// 5 + optional state
			if (parsed < 5) continue;

		}

		ret += websWrite(wp, "[\"%s\", \"%s\", \"%d\", \"%s\", \"%d\", \"%s\"],\n",
		                      proto, address, port1, dest, port2, state);
	}
	fclose(fp);
	unlink("/tmp/connect.log");

	ret += websWrite(wp, "[]];\n");

#ifdef RTCONFIG_IPV6
	ret += websWrite(wp, "var connarray_route = [");

	system("/usr/sbin/netstat-nat -Rno -X50 > /tmp/connect.log 2>&1");

	fp = fopen("/tmp/connect.log", "r");
	if (fp == NULL) {
		websWrite(wp, "[]];\n");
		return ret;
	}

	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line,"udp",3) && strncmp(line,"tcp",3))
			continue;

		// Clear it as it might be absent
		state[0] = '\0';

		// Try IPv6 format (brackets present)
		if (strchr(line, '[')) {
			parsed = sscanf(line,
			         "%3s "
                     "[%47[^]]]:%d "
			         "[%47[^]]]:%d"
			         "%14s%*[ \t]",
			         proto, address, &port1, dest, &port2, state);
		} else {
			continue;
		}
		// 5 + optional state
		if (parsed < 5) continue;

		ret += websWrite(wp, "[\"%s\", \"%s\", \"%d\", \"%s\", \"%d\", \"%s\"],\n",
		                      proto, address, port1, dest, port2, state);
	}
	fclose(fp);
	unlink("/tmp/connect.log");

	ret += websWrite(wp, "[]];\n");
#endif

	return ret;
}

#ifdef RTCONFIG_BWDPI
int ej_bwdpi_conntrack(int eid, webs_t wp, int argc, char **argv_) {
	char comma;
	char line[256];
	FILE *fp;
	static unsigned int count = 0;
	char src_ip[64], dst_ip[64], prot[4];
	int dport, sport;
	int ret;
	unsigned long mark;
	int id, cat;
	char desc[64], key[9];
	char ipversion;
	static struct hsearch_data *htab;
	ENTRY entry, *resultp;
	struct stat dbattrib;
	static unsigned long lastupd = 0;
	static char **alloctable;
	int allocptr = 0;

	if (stat(APPDB, &dbattrib))
		return websWrite(wp, "bwdpi_conntrack=[];");

	if ((lastupd) && (lastupd != dbattrib.st_ctime)) {
		// free all nodes (2 entries per item)
		while (allocptr < count * 2) {
			free(alloctable[allocptr++]);
		}
		hdestroy_r(htab);
		count = allocptr = lastupd = 0;
		free(alloctable);
	}

// Init hash table
	if (!lastupd) {
		lastupd = dbattrib.st_ctime;

		fp = fopen(APPDB, "r");
		if (!fp) return websWrite(wp, "bwdpi_conntrack=[];");

		while (fgets(line, sizeof(line), fp) != NULL) {
			count++;
		}

		alloctable = calloc(count * 2, sizeof(char *));
		htab = calloc(1, sizeof(struct hsearch_data));

		if ((!alloctable) || (!htab) || (!hcreate_r((unsigned int)count * 1.3, htab))) {
			fclose(fp);
			return websWrite(wp, "bwdpi_conntrack=[];");
		}

// Parse App database
		rewind(fp);
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (sscanf(line,"%d,%d,%*d,%63[^\n]", &id, &cat, desc) == 3) {
				snprintf(key, sizeof(key), "%d-%d",id, cat);
				entry.key = strdup(key);
				entry.data = strdup(desc);
				if (entry.key && entry.data) {
					alloctable[allocptr++] = entry.key;
					alloctable[allocptr++] = entry.data;
					hsearch_r(entry, ENTER, &resultp, htab);
				}
			}
		}
		fclose(fp);
	}

// Parse tracked connections
	if ((fp = fopen("/proc/bw_cte_dump", "r")) == NULL)
		return websWrite(wp, "bwdpi_conntrack=[];");

	ret = websWrite(wp, "bwdpi_conntrack=[");
	comma = ' ';

	while (fgets(line, sizeof(line), fp)) {
		// ipv4 tcp src=192.168.10.156 dst=172.217.13.110 sport=8248 dport=443 index=8510 mark=3cd000f
		if (sscanf(line, "ipv%c %3s src=%63s dst=%63s sport=%d dport=%d index=%*d mark=%lx",
			          &ipversion, prot, src_ip, dst_ip, &sport, &dport, &mark) != 7 ) continue;

		id = (mark & 0x3F0000)/0xFFFF;
		cat = mark & 0xFFFF;

		if ((cat == 0) && (id == 0))
			strcpy(desc, "Untracked");
		else {

			snprintf(key, sizeof(key), "%d-%d", id, cat);
			entry.key = key;
			if (!hsearch_r(entry, FIND, &resultp, htab))
				sprintf(desc, "unknown (AppID=%d, Cat=%d)", id, cat);
			else
				strlcpy(desc, (char *)resultp->data, sizeof(desc));
		}

		if (ipversion == '6') {
			_fix_TM_ipv6(src_ip);
			_fix_TM_ipv6(dst_ip);
		}

		ret += websWrite(wp, "%c[\"%s\", \"%s\", \"%d\", \"%s\", \"%d\", \"%s\", \"%d\", \"%d\"]",
		                      comma, prot, src_ip, sport, dst_ip, dport, desc, cat, id);
		comma = ',';
	}
	fclose(fp);

	ret += websWrite(wp, "];\n");
	return ret;
}

// TM puts columns between every octet pairs
// Reformat that into quads rather than pairs
// Also TM is missing the last two octets, so pad
// with arbitrary "00" octets to get a valid IPv6
void _fix_TM_ipv6(char* str) {
	char *pr = str, *pw = str;
	int found=0;

	while (*pr) {
		*pw = *pr++;
		if (*pw == ':') {
			found++;
			if (found % 2)
				continue;
		}
		pw++;
	}
	*pw = '\0';

	if (strlen(str) == 37)
		strcat(str,"00");
}
#endif


int ej_get_custom_settings(int eid, webs_t wp, int argc, char **argv_) {

	struct json_object *settings_obj;
	int ret = 0;
	char line[3040];
	char name[30];
	char value[3000];
	FILE *fp;

	fp = fopen("/jffs/addons/custom_settings.txt", "r");
	if (fp == NULL) {
		ret += websWrite(wp," new Object()");
		return 0;
	}

	settings_obj = json_object_new_object();
	while (fgets(line, sizeof(line), fp)) {
		if (sscanf(line,"%29s%*[ ]%2999s%*[ \n]",name, value) == 2) {
			json_object_object_add(settings_obj, name, json_object_new_string(value));
		}
	}
	fclose(fp);

	ret += websWrite(wp, "%s", json_object_to_json_string(settings_obj));

	json_object_put(settings_obj);
	return ret;
}


void write_custom_settings(char *jstring) {
	char line[3040];
	FILE *fp;
	struct json_object *settings_obj;

	settings_obj = json_tokener_parse(jstring);
	if (!settings_obj) return;

	fp = fopen("/jffs/addons/custom_settings.txt", "w");
	if (!fp) return;

	json_object_object_foreach(settings_obj, key, val) {
		snprintf(line, sizeof(line), "%s %s\n", key, json_object_get_string(val));
		fwrite(line, 1, strlen(line), fp);
	}
	fclose(fp);

	json_object_put(settings_obj);
}
