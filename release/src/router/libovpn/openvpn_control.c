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
 */

/*
 * OpenVPN utility library for Asuswrt-Merlin
 * Provides some of the functions found in Asuswrt's
 * proprietary libvpn, either re-implemented, or
 * implemented as wrappers around AM's functions.
 * Also includes additional functions developed
 * for Asuswrt-Merlin.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <shutils.h>
#include <shared.h>
#include "openvpn_config.h"
#include "openvpn_control.h"
#include "openvpn_setup.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int ovpn_skip_dnsmasq() {
	int unit;
	char filename[40], varname[32];

	for (unit = 1; unit <= OVPN_CLIENT_MAX; unit++ ) {
		sprintf(filename, "/etc/openvpn/client%d/client.resolv", unit);
		if (f_exists(filename)) {
			sprintf(varname, "vpn_client%d_", unit);

			// Skip DNS setup if we have a running client that uses exclusive mode and not VPN Director
			if ((nvram_pf_get_int(varname, "rgw") == OVPN_RGW_ALL) &&
			    (nvram_pf_get_int(varname, "adns") == OVPN_DNSMODE_EXCLUSIVE) &&
			    (nvram_pf_get_int(varname, "state") > OVPN_STS_STOP))	// Include OVPN_STS_INIT/STOPPING
				return 1;
		}
	}
	return 0;
}


// Any running client using strict mode (which requires restarting dnsmasq)?
int ovpn_need_dnsmasq_restart() {
	int unit;
	char filename[40], varname[32];

	for (unit = 1; unit <= OVPN_CLIENT_MAX; unit++ ) {
		sprintf(filename, "/etc/openvpn/client%d/client.resolv", unit);
		if (f_exists(filename)) {
			sprintf(varname, "vpn_client%d_", unit);
			if ((nvram_pf_get_int(varname, "adns") == OVPN_DNSMODE_STRICT) &&
			    (nvram_pf_get_int(varname, "state") > OVPN_STS_STOP))
				return 1;
		}
	}
	return 0;
}


int _check_ovpn_enabled(int unit, ovpn_type_t type){
	char tmp[2];
	char* varname = NULL;

	switch (type) {
		case OVPN_TYPE_SERVER:
			if (unit <= OVPN_SERVER_MAX)
				varname = "vpn_serverx_start";
			break;
		case OVPN_TYPE_CLIENT:
			if (unit <= OVPN_CLIENT_MAX)
				varname = "vpn_clientx_eas";
			break;
	}

	if (varname) {
		sprintf(tmp,"%d", unit);
		if (strstr(nvram_safe_get(varname), tmp)) return 1;
	}

	return 0;
}

int check_ovpn_server_enabled(int unit) {
	return _check_ovpn_enabled(unit, OVPN_TYPE_SERVER);
}

int check_ovpn_client_enabled(int unit) {
	return _check_ovpn_enabled(unit, OVPN_TYPE_CLIENT);
}


int ovpn_run_instance(ovpn_type_t type, int unit){
	char buffer[64], buffer2[64], cpulist[8];
	char *instanceType;
	int cpuCores;

	if (type == OVPN_TYPE_SERVER)
		instanceType = "server";
	else
		instanceType = "client";

//	logmessage("openvpn", "Starting OpenVPN %s %d...", instanceType, unit);

	// Start the VPN instance
	sprintf(buffer, "/etc/openvpn/vpn%s%d", instanceType, unit);
	sprintf(buffer2, "/etc/openvpn/%s%d", instanceType, unit);

	// Spread instances on cpu 1,0 or 1,2,3,0 (in that order)
	cpuCores = sysconf(_SC_NPROCESSORS_CONF) - 1;
	if (cpuCores < 0) cpuCores = 0;
	snprintf(cpulist, sizeof(cpulist), "%d", (unit & cpuCores));

	if (cpu_eval(NULL, cpulist, buffer, "--cd", buffer2, "--config", "config.ovpn")) {
		return -1;
	}

	return 0;
}


void ovpn_run_fw_scripts(){
	char buffer[64];
	int unit;

	for (unit = 1; unit <= OVPN_SERVER_MAX; unit++) {
		snprintf(buffer, sizeof(buffer), "/etc/openvpn/server%d/fw.sh", unit);
		if (f_exists(buffer))
			eval(buffer);
	}

	// Reverse order because of DNSVPN rules
	for (unit = OVPN_CLIENT_MAX; unit > 0; unit--) {
		snprintf(buffer, sizeof(buffer), "/etc/openvpn/client%d/fw.sh", unit);
		if (f_exists(buffer))
			eval(buffer);

		snprintf(buffer, sizeof(buffer), "/etc/openvpn/client%d/dns.sh", unit);
		if (f_exists(buffer))
			eval(buffer);

		snprintf(buffer, sizeof(buffer), "/etc/openvpn/client%d/qos.sh", unit);
		if (f_exists(buffer))
			eval(buffer);

	}
}

void update_ovpn_status(ovpn_type_t type, int unit, ovpn_status_t status_type, ovpn_errno_t err_no)
{
	char varname[32];

	sprintf(varname, "vpn_%s%d_state", (type == OVPN_TYPE_SERVER ? "server" : "client"), unit);
	nvram_set_int(varname, status_type);

        sprintf(varname, "vpn_%s%d_errno", (type == OVPN_TYPE_SERVER ? "server" : "client"), unit);
        nvram_set_int(varname, err_no);

	if (type == OVPN_TYPE_CLIENT && (status_type == OVPN_STS_INIT || status_type == OVPN_STS_STOP)) {
		sprintf(varname, "vpn_client%d_rip", unit);
		nvram_set(varname, "");
	}
}

ovpn_status_t get_ovpn_status(ovpn_type_t type, int unit)
{
	char varname[32];

	sprintf(varname, "vpn_%s%d_state", (type == OVPN_TYPE_SERVER ? "server" : "client"), unit);
	return nvram_get_int(varname);
}

ovpn_errno_t get_ovpn_errno(ovpn_type_t type, int unit)
{
	char varname[32];

	sprintf(varname, "vpn_%s%d_errno", (type == OVPN_TYPE_SERVER ? "server" : "client"), unit);
	return nvram_get_int(varname);
}

void ovpn_server_up_handler(int unit)
{
	_ovpn_run_event_script();
}

void ovpn_server_down_handler(int unit)
{
	_ovpn_run_event_script();
}

void ovpn_client_route_up_handler()
{
        _ovpn_run_event_script();
}

void ovpn_client_route_pre_down_handler()
{
        _ovpn_run_event_script();
}


void ovpn_client_down_handler(int unit)
{
	char buffer[64];
	char dirname[64];

	if ((unit < 1) || (unit > OVPN_CLIENT_MAX))
		return;

	ovpn_set_killswitch(unit);
	_flush_routing_cache();

	ovpn_clear_exclusive_dns(unit);

	sprintf(dirname, "/etc/openvpn/client%d", unit);

	sprintf(buffer, "%s/qos.sh", dirname);
	if (f_exists(buffer)) {
		eval("sed", "-i", "s/-A/-D/g", buffer);
		eval(buffer);
		unlink(buffer);
	}

	sprintf(buffer, "%s/client.resolv", dirname);
	if (f_exists(buffer))
		unlink(buffer);

	sprintf(buffer, "%s/client.conf", dirname);
	if (f_exists(buffer))
		unlink(buffer);
}


void ovpn_clear_exclusive_dns(int unit)
{
	char buffer[32];

	sprintf(buffer, "DNSVPN%d", unit);
	eval("/usr/sbin/iptables", "-t", "nat", "-D", "PREROUTING", "-p", "udp", "-m", "udp", "--dport", "53", "-j", buffer);
	eval("/usr/sbin/iptables", "-t", "nat", "-D", "PREROUTING", "-p", "tcp", "-m", "tcp", "--dport", "53", "-j", buffer);
	eval("/usr/sbin/iptables", "-t", "nat", "-F", buffer);
	eval("/usr/sbin/iptables", "-t", "nat", "-X", buffer);

	sprintf(buffer, "/etc/openvpn/client%d/dns.sh", unit);
	if (f_exists(buffer))
		unlink(buffer);
}


// Recreate the port 53 PREROUTING rules to ensure they are in the correct order (OVPN1 first, OVPN5 last)
void ovpn_update_exclusive_dns_rules()
{
	int unit;
	char buffer[100];

	for (unit = OVPN_CLIENT_MAX; unit > 0; unit--) {
		snprintf(buffer, sizeof (buffer), "/etc/openvpn/client%d/dns.sh", unit);
		if (f_exists(buffer)) {
			// Remove and re-add to ensure proper order
			snprintf(buffer, sizeof (buffer), "DNSVPN%d", unit);

			eval("/usr/sbin/iptables", "-t", "nat", "-D", "PREROUTING", "-p", "udp", "-m", "udp", "--dport", "53", "-j", buffer);
			eval("/usr/sbin/iptables", "-t", "nat", "-D", "PREROUTING", "-p", "tcp", "-m", "tcp", "--dport", "53", "-j", buffer);

			eval("/usr/sbin/iptables", "-t", "nat", "-I", "PREROUTING", "-p", "udp", "-m", "udp", "--dport", "53", "-j", buffer);
			eval("/usr/sbin/iptables", "-t", "nat", "-I", "PREROUTING", "-p", "tcp", "-m", "tcp", "--dport", "53", "-j", buffer);
		}
	}
}


void ovpn_client_up_handler(int unit)
{
	char buffer[128], buffer2[128], buffer3[128];
	char dirname[64];
	char prefix[32];
	FILE *fp_resolv = NULL, *fp_conf = NULL, *fp_qos = NULL, *fp_route = NULL;;
	int i, j, verb, rgw, lock;
	char *option, *option2;
	char *network_env, *netmask_env, *gateway_env, *metric_env, *remotegw_env, *dev_env;
	char *remote_env, *localgw;
	struct in_addr network, netmask;

	if ((unit < 1) || (unit > OVPN_CLIENT_MAX))
		return;

	snprintf(prefix, sizeof(prefix), "vpn_client%d_", unit);
	sprintf(dirname, "/etc/openvpn/client%d", unit);

	// tQOS fix
	if ((nvram_pf_get_int(prefix, "rgw") >= 1) &&
	    (nvram_get_int("qos_enable") == 1) &&
	    (nvram_get_int("qos_type") == 1)) {
		sprintf(buffer, "%s/qos.sh", dirname);
		fp_qos = fopen(buffer, "w");
		if (fp_qos) {
			fprintf(fp_qos, "#!/bin/sh\n"
				        "/usr/sbin/iptables -t mangle -A POSTROUTING -o br0 -m mark --mark 0x40000000/0xc0000000 -j MARK --set-xmark 0x80000000/0xC0000000\n");
			fclose(fp_qos);
			chmod(buffer, 0755);
			eval(buffer);
		}
	}

	verb = nvram_pf_get_int(prefix, "verb");

	// Routing handling, only for TUN
	if (!strncmp(_safe_getenv("dev"),"tun", 3)) {
		snprintf(buffer, sizeof (buffer), "/usr/sbin/ip route flush table ovpnc%d", unit);
		system(buffer);

		// Copy main table routes
		snprintf(buffer, sizeof (buffer), "/usr/sbin/ip route show table main > /tmp/vpnroute%d_tmp", unit);
		system(buffer);

		snprintf(buffer, sizeof (buffer), "/tmp/vpnroute%d_tmp", unit);
		fp_route = fopen(buffer, "r");

		if (fp_route) {
			while (fgets(buffer2, sizeof(buffer2), fp_route) != NULL) {
				if (buffer2[strlen(buffer2)-1] == '\n')
					buffer2[strlen(buffer2)-1] = '\0';
				snprintf(buffer3, sizeof (buffer3), "/usr/sbin/ip route add %s table ovpnc%d", buffer2, unit);
				system(buffer3);
				if (verb >= 6)
					logmessage("openvpn-routing", "Copy main table route: %s", buffer3);
			}
			fclose(fp_route);
		}
		unlink(buffer);

		// Apply pushed routes
		dev_env = _safe_getenv("dev");

		i = 0;
		while (1) {
			i++;
			sprintf(buffer, "route_network_%d", i);
			network_env = getenv(buffer);
			sprintf(buffer, "route_netmask_%d", i);
			netmask_env = getenv(buffer);
			sprintf(buffer, "route_gateway_%d", i);
			gateway_env = getenv(buffer);
			sprintf(buffer, "route_metric_%d", i);
			metric_env = getenv(buffer);

			if (!network_env || !netmask_env || !gateway_env)
				break;

			if ( (inet_pton(AF_INET, network_env, &network) == 1)
			    && (inet_pton(AF_INET, netmask_env, &netmask) == 1)) {

				snprintf(buffer, sizeof (buffer),"/usr/sbin/ip route add %s/%s via %s dev %s %s %s table ovpnc%d",
			                network_env, netmask_env, gateway_env, dev_env, (metric_env ? "metric" : ""), (metric_env ? metric_env : ""), unit);
				if (verb >= 3)
					logmessage("openvpn-routing","Add pushed route: %s", buffer);
				system(buffer);
			}
		}

		// Handle traffic redirection
		rgw = nvram_pf_get_int(prefix, "rgw");

		if (rgw != OVPN_RGW_NONE) {
			// Force traffic to remote VPN server to go through local GW
			remote_env = getenv("trusted_ip");
			localgw = getenv("route_net_gateway");

			if (remote_env && localgw) {
				snprintf(buffer, sizeof (buffer), "/usr/sbin/ip route add %s/32 via %s table ovpnc%d",
					remote_env, localgw, unit);
				if (verb >= 6)
					logmessage("openvpn-routing", "Add route to remote endpoint: %s", buffer);
				system(buffer);
			} else {
				logmessage("openvpn-routing", "Missing remote IP or local gateway - cannot configure route");
			}

			// Use VPN as default gateway
			remotegw_env = getenv("route_vpn_gateway");
			if (remotegw_env) {
				snprintf(buffer, sizeof (buffer), "/usr/sbin/ip route replace default via %s table ovpnc%d",
				         remotegw_env, unit);
				if (verb >= 3)
					logmessage("openvpn-routing","Setting client %d routing table's default route through the tunnel", unit);
				system(buffer);
			} else {
				logmessage("openvpn-routing","WARNING: no VPN gateway provided, routing might not work properly!");
			}
		}

		ovpn_set_routing_rules(unit);
		_flush_routing_cache();

	}	// end IF_TUN


	if (nvram_pf_get_int(prefix, "adns") == OVPN_DNSMODE_IGNORE)
		goto exit;

	// Parse foreign options
	i = 1;
	while (1) {
		sprintf(buffer, "foreign_option_%d", i++);
		option = getenv(buffer);
		if (!option)
			break;

		if (!strncmp(option, "dhcp-option WINS ", 17)) {
			if (!inet_aton(&option[17], &network))
				continue;

			if (!fp_conf) {
				sprintf(buffer, "%s/client.conf", dirname);
				if ((fp_conf = fopen(buffer, "w")) == NULL)
					goto exit;
			}
			fprintf(fp_conf, "dhcp-option=44,%s\n", &option[17]);

		} else if (!strncmp(option, "dhcp-option DNS ", 16)) {
			if (!inet_aton(&option[16], &network))
				continue;

			if (!fp_resolv) {
				sprintf(buffer, "%s/client.resolv", dirname);
				if ((fp_resolv = fopen(buffer, "w")) == NULL)
					goto exit;
			}
			fprintf(fp_resolv, "server=%s\n", &option[16]);

			// Any search domains for that server
			j = 1;
			while (1) {
				sprintf(buffer, "foreign_option_%d", j++);
				option2 = getenv(buffer);
				if (!option2)
					break;
				if (!strncmp(option2, "dhcp-option DOMAIN ", 19)) {
					fprintf(fp_resolv, "server=/%s/%s\n", &option2[19], &option[16]);
				}
			}
		}
	}
exit:
	if (fp_resolv) {
		fclose(fp_resolv);

		// Set exclusive DNS iptables
		if ((nvram_pf_get_int(prefix, "rgw") == OVPN_RGW_POLICY) && (nvram_pf_get_int(prefix, "adns") == OVPN_DNSMODE_EXCLUSIVE)) {
			lock = file_lock(VPNROUTING_LOCK);
			ovpn_set_exclusive_dns(unit);
			// Refresh prerouting rules to ensure correct order
			ovpn_update_exclusive_dns_rules();
			file_unlock(lock);
		}
	}

	if (fp_conf)
		fclose(fp_conf);
}


// Remove all rules pointing to a specific client table
// If unit is 0, then remove rules targetting main (i.e. WAN)
void _clear_routing_rules(int unit) {
	FILE *fp;
	char buffer[128], buffer2[128], buffer3[128];
	int prio, verb;
	char table[12], *lookup;
	char target_table[12];

	snprintf(buffer, sizeof (buffer), "vpn_client%d_verb", unit);
	verb = nvram_get_int(buffer);

	snprintf(buffer, sizeof (buffer), "/usr/sbin/ip rule show > /tmp/vpnrules%d_tmp", unit);
	system(buffer);

	snprintf(buffer, sizeof (buffer), "/tmp/vpnrules%d_tmp", unit);
	fp = fopen(buffer, "r");
	if (fp) {
		if (unit == 0)
			strlcpy(target_table, "main", sizeof (target_table));
		else
			snprintf(target_table, sizeof (target_table), "ovpnc%d", unit);

		while (fgets(buffer2, sizeof(buffer2), fp) != NULL) {
			if (buffer2[strlen(buffer2)-1] == '\n')
				buffer2[strlen(buffer2)-1] = '\0';

			if (sscanf(buffer2, "%u", &prio) != 1)
				continue;

			// Only remove rules within our official range
			if ((prio < 10000) || (prio > (10209 + (OVPN_CLIENT_MAX * 200))))
				continue;

			if ((lookup = strstr(buffer2, "lookup")) == NULL)
				continue;

			if (sscanf(lookup, "lookup %11s", table) != 1)
				continue;
			if (strcmp(table, target_table))
				continue;

			snprintf(buffer3, sizeof (buffer3), "/usr/sbin/ip rule del prio %d", prio);
			if (verb >= 6)
				logmessage("openvpn-routing", "Removed rule %d", prio);
			system(buffer3);
		}
		fclose(fp);
	}
	unlink(buffer);
}

/*
	Rule priority allocations:

	10000-10009: clients set to OVPN_RGW_ALL

	10010-10209: WAN rules

	10210-10409: OVPN 1
	10510-10609: OVPN 2
	10710-10809: OVPN 3
	10910-11009: OVPN 4
	11100-11209: OVPN 5
*/

void ovpn_set_routing_rules(int unit) {
	char prefix[32], buffer[8000];
	int rgw, state, verb;

	if (unit < 1 || unit > OVPN_CLIENT_MAX)
		return;

	snprintf(prefix, sizeof(prefix), "vpn_client%d_", unit);
	verb = nvram_pf_get_int(prefix, "verb");
	rgw = nvram_pf_get_int(prefix, "rgw");

	_clear_routing_rules(unit);

	/* Refresh WAN rules */
	_clear_routing_rules(0);
	ovpn_get_policy_rules(0, buffer, sizeof (buffer));
	_write_routing_rules(0, buffer, verb);

	switch (rgw) {
		case OVPN_RGW_NONE:
		case OVPN_RGW_ALL:
			// Set client rules if running or currently connecting
			state = get_ovpn_status(OVPN_TYPE_CLIENT, unit);
			if (state == OVPN_STS_RUNNING || state == OVPN_STS_INIT) {
				snprintf(buffer, sizeof (buffer), "/usr/sbin/ip rule add table ovpnc%d priority %d", unit, 10000 + unit);
				system(buffer);
				if (verb >= 3)
					logmessage("openvpn-routing","Routing all traffic through ovpnc%d", unit);
			}
			break;

		case OVPN_RGW_POLICY:
			ovpn_get_policy_rules(unit, buffer, sizeof (buffer));
			_write_routing_rules(unit, buffer, verb);
			break;
	}
}


void _write_routing_rules(int unit, char *rules, int verb) {
	char *buffer_tmp, *buffer_tmp2, *rule;
	char buffer[128], table[16];
	int ruleprio, vpnprio, wanprio;
	char *enable, *desc, *target, *src, *dst;
	char srcstr[64], dststr[64];

	wanprio = 10010;
	vpnprio = 10010 + (200 * unit);

	buffer_tmp = buffer_tmp2 = strdup(rules);

	while (buffer_tmp && (rule = strsep(&buffer_tmp2, "<")) != NULL) {
		if((vstrsep(rule, ">", &enable, &desc, &src, &dst, &target)) != 5)
			 continue;

		if (!atoi(&enable[0]))
			continue;

		if (!*src && !*dst)
			continue;

		if (!strcmp(target,"WAN")) {
			strcpy(table, "main");
			ruleprio = wanprio++;
		}
		else if (!strncmp(target, "OVPN", 4)) {
			snprintf(table, sizeof (table), "ovpnc%d", unit);
			ruleprio = vpnprio++;
		}
		else
			continue;

		if (*src && strcmp(src, "0.0.0.0"))
			snprintf(srcstr, sizeof (srcstr), "from %s", src);
		else
			*srcstr = '\0';

		if (*dst && strcmp(dst, "0.0.0.0"))
			snprintf(dststr, sizeof (dststr), "to %s", dst);
		else
			*dststr = '\0';

		snprintf(buffer, sizeof (buffer), "/usr/sbin/ip rule add %s %s table %s priority %d",
				                                   srcstr, dststr, table, ruleprio);

		if (verb >= 3)
			logmessage("openvpn-routing","Routing %s from %s to %s through %s", desc, (*src ? src : "any"), (*dst ? dst : "any"), table);

		system(buffer);
	}
	free(buffer_tmp);
}


void ovpn_set_killswitch(int unit) {
	char buffer[64];

	snprintf(buffer, sizeof (buffer), "vpn_client%d_enforce", unit);
	if (nvram_get_int(buffer)) {
		snprintf(buffer, sizeof (buffer), "/usr/sbin/ip route del default table ovpnc%d", unit);
		system(buffer);
		snprintf(buffer, sizeof (buffer), "/usr/sbin/ip route add prohibit default table ovpnc%d", unit);
		logmessage("openvpn-routing", "Configured killswitch on VPN client %d", unit);
		system(buffer);
	}
}


inline void _flush_routing_cache() {
	system("/usr/sbin/ip route flush cache");
}


void ovpn_set_exclusive_dns(int unit) {
	char rules[8000], wanrules[8000], buffer[64], buffer2[64], server[20], iface_match[8];
	char *nvp, *entry;
	char *src, *dst, *iface, *desc, *enable, *netptr;
	struct in_addr addr;
	int mask;

	FILE *fp_resolv, *fp_dns;

	snprintf(buffer, sizeof (buffer), "/etc/openvpn/client%d/client.resolv", unit);
	fp_resolv = fopen(buffer, "r");
	snprintf(buffer, sizeof (buffer), "/etc/openvpn/client%d/dns.sh", unit);
	fp_dns = fopen(buffer, "w");

	if (!fp_resolv || !fp_dns) {
		if (fp_resolv)
			fclose(fp_resolv);
		if (fp_dns)
			fclose(fp_dns);
		return;
	}

	fprintf(fp_dns, "#!/bin/sh\n"
	                "/usr/sbin/iptables -t nat -N DNSVPN%d\n",
	                 unit);

	ovpn_get_policy_rules(unit, rules, sizeof (rules));
	ovpn_get_policy_rules(0, wanrules, sizeof (wanrules));
	strlcat(rules, wanrules, sizeof (rules));

	nvp = rules;

	snprintf(iface_match, sizeof (iface_match), "OVPN%d", unit);

	while ((entry = strsep(&nvp, "<")) != NULL) {
		if (vstrsep(entry, ">", &enable, &desc, &src, &dst, &iface) != 5)
			continue;

		if (atoi(&enable[0]) == 0)
			continue;

		if (*src && !*dst) {
			strlcpy(buffer, src, sizeof(buffer));

			if ((netptr = strchr(buffer, '/'))) {
				*netptr = '\0';
				mask = atoi(&netptr[1]);
			} else {
				mask = 32;
			}

			if ((mask >= 0) &&
			    (mask <= 32) &&
			    (inet_aton(buffer, &addr))) {
				if (!strcmp(iface, iface_match)) {

					// Iterate through servers
					rewind(fp_resolv);
					while (fgets(buffer2, sizeof(buffer2), fp_resolv) != NULL) {
						if (sscanf(buffer2, "server=%16s", server) != 1)
							continue;

						if (!inet_aton(server, &addr))
							continue;

		                                fprintf(fp_dns, "/usr/sbin/iptables -t nat -A DNSVPN%d -s %s -j DNAT --to-destination %s\n", unit, src, server);
		                                logmessage("openvpn", "Forcing %s to use DNS server %s", src, server);
						// Only configure first server found, as others would never get used
						break;
					}
	                        } else if (!strcmp(iface, "WAN")) {
	                                fprintf(fp_dns, "/usr/sbin/iptables -t nat -I DNSVPN%d -s %s -j RETURN\n", unit, src);
	                                logmessage("openvpn", "Excluding %s from forced DNS routing", src);
	                        }
			}
		}
	}

	fprintf(fp_dns, "/usr/sbin/iptables -t nat -I PREROUTING -p udp -m udp --dport 53 -j DNSVPN%d\n"
	                "/usr/sbin/iptables -t nat -I PREROUTING -p tcp -m tcp --dport 53 -j DNSVPN%d\n",
	                 unit, unit);

	fclose(fp_resolv);
	fclose(fp_dns);
	sprintf(buffer, "/etc/openvpn/client%d/dns.sh", unit);
	if (f_exists(buffer)) {
		chmod(buffer, 0755);
		eval(buffer);
	}
}


char *_safe_getenv(const char* name) {
	char *value;

	value = getenv(name);
	return (value ? value : "");
}


void _ovpn_run_event_script() {
	ovpn_if_t type;

	if (!strncmp(_safe_getenv("dev"),"tun", 3))
		type = OVPN_IF_TUN;
	else if (!strncmp(_safe_getenv("dev"),"tap", 3))
		type = OVPN_IF_TAP;
	else
		return;

	if (f_exists("/jffs/scripts/openvpn-event")) {
		if (nvram_get_int("jffs2_scripts") == 0) {
			logmessage("custom_script", "Found openvpn-event, but custom script execution is disabled!");
		} else {
			eval("/jffs/scripts/openvpn-event", _safe_getenv("dev"), (type == OVPN_IF_TUN ? _safe_getenv("tun_mtu") : _safe_getenv("tap_mtu")), _safe_getenv("link_mtu"),
			      _safe_getenv("ifconfig_local"), _safe_getenv("ifconfig_remote"), _safe_getenv("script_context"));
			logmessage("custom_script", "Running openvpn-event");
		}
	}
}

void ovpn_start_client(int unit) {
	char buffer[64], buffer2[64];
	ovpn_cconf_t *cconf;

	sprintf(buffer, "start_vpnclient%d", unit);
	if (getpid() != 1) {
		notify_rc(buffer);
		return;
	}

	if ( (pidof(&buffer[6])) >= 0 )
	{
		logmessage("openvpn", "OpenVPN client %d start attempt - already running.", unit);
		return;
	}

	update_ovpn_status(OVPN_TYPE_CLIENT, unit, OVPN_STS_INIT, OVPN_ERRNO_NONE);

	// Retrieve instance configuration
	cconf = ovpn_get_cconf(unit);

        // Setup directories and symlinks
	ovpn_setup_dirs(OVPN_TYPE_CLIENT, unit);

	// Setup interface
	if (ovpn_setup_iface(cconf->if_name, cconf->if_type, cconf->bridge)) {
		ovpn_stop_client(unit);
		free(cconf);
		return;
	}

	// Write config file
	ovpn_write_client_config(cconf, unit);

	// Write certificate and key files
	ovpn_write_client_keys(cconf, unit);

	// Run postconf custom script if it exists
	sprintf(buffer, "openvpnclient%d", unit);
	sprintf(buffer2, "/etc/openvpn/client%d/config.ovpn", unit);
	run_postconf(buffer, buffer2);

	// Setup firewall
	ovpn_setup_client_fw(cconf, unit);

	free(cconf);

        // Start the VPN client
	if (ovpn_run_instance(OVPN_TYPE_CLIENT, unit)) {
		logmessage("openvpn", "Starting OpenVPN client %d failed!", unit);
		ovpn_stop_client(unit);
		if (get_ovpn_status(OVPN_TYPE_CLIENT, unit) != OVPN_STS_ERROR)
			update_ovpn_status(OVPN_TYPE_CLIENT, unit, OVPN_STS_ERROR, OVPN_ERRNO_CONF);
	}
}


void ovpn_start_server(int unit) {
	char buffer[256], buffer2[8000];
	ovpn_sconf_t *sconf;

	sprintf(buffer, "start_vpnserver%d", unit);
	if (getpid() != 1) {
		notify_rc(buffer);
		return;
	}

	if ((pidof(&buffer[6])) >= 0) {
		logmessage("openvpn", "OpenVPN server %d start attempt - already running.", unit);
		return;
	}

	update_ovpn_status(OVPN_TYPE_SERVER, unit, OVPN_STS_INIT, OVPN_ERRNO_NONE);

	// Retrieve instance configuration
	sconf = ovpn_get_sconf(unit);

	if(is_intf_up(sconf->if_name) > 0 && sconf->if_type == OVPN_IF_TAP)
		eval("brctl", "delif", nvram_safe_get("lan_ifname"), sconf->if_name);

	// Setup directories and symlinks
	ovpn_setup_dirs(OVPN_TYPE_SERVER, unit);

	// Setup interface
        if (ovpn_setup_iface(sconf->if_name, sconf->if_type, 1)) {
		ovpn_stop_server(unit);
		free(sconf);
		return;
	}

	// Write config files
	ovpn_write_server_config(sconf, unit);

	// Write key/certs
	ovpn_write_server_keys(sconf, unit);

	// Format client file so Windows Notepad can edit it
	sprintf(buffer, "/etc/openvpn/server%d/client.ovpn", unit);
	eval("/usr/bin/unix2dos", buffer);

        // Setup firewall
        ovpn_setup_server_fw(sconf, unit);

	// Run postconf custom script on it if it exists
	sprintf(buffer, "openvpnserver%d", unit);
	sprintf(buffer2, "/etc/openvpn/server%d/config.ovpn", unit);
	run_postconf(buffer, buffer2);

	// Start the VPN server
	if (ovpn_run_instance(OVPN_TYPE_SERVER, unit)) {
		logmessage("openvpn", "Starting OpenVPN server %d failed!", unit);
		ovpn_stop_server(unit);
		update_ovpn_status(OVPN_TYPE_SERVER, unit, OVPN_STS_ERROR, OVPN_ERRNO_CONF);
		free(sconf);
		return;
	}

	if (sconf->auth_mode == OVPN_AUTH_STATIC)
		update_ovpn_status(OVPN_TYPE_SERVER, unit, OVPN_STS_RUNNING, OVPN_ERRNO_NONE);

	ovpn_setup_server_watchdog(sconf, unit);

	free(sconf);
}


void ovpn_stop_client(int unit) {
	char buffer[64];

	sprintf(buffer, "stop_vpnclient%d", unit);
	if (getpid() != 1) {
		notify_rc(buffer);
		return;
	}

	sprintf(buffer, "vpnclient%d", unit);

	// Stop running client
	if (pidof(buffer) != -1)
		killall_tk_period_wait(buffer, 10);

	// Manual stop, so remove rules
	_clear_routing_rules(unit);

	// Clear routing table, also freeing from killswitch set by down handler
	snprintf(buffer, sizeof (buffer),"/usr/sbin/ip route flush table ovpnc%d", unit);
	logmessage("openvpn-routing", "Clearing routing table for VPN client %d", unit);
	system(buffer);

	ovpn_remove_iface(OVPN_TYPE_CLIENT, unit);

	// Remove firewall rules after VPN exit
	sprintf(buffer, "/etc/openvpn/client%d/fw.sh", unit);
	if (f_exists(buffer)) {
		if (!eval("sed", "-i", "s/-A/-D/g;s/-I/-D/g", buffer))
			eval(buffer);
	}

	// Delete all files for this client
	sprintf(buffer, "/etc/openvpn/client%d",unit);
	eval("rm", "-rf", buffer);
	sprintf(buffer, "/etc/openvpn/vpnclient%d",unit);
	eval("rm", "-rf", buffer);

	update_ovpn_status(OVPN_TYPE_CLIENT, unit, OVPN_STS_STOP, OVPN_ERRNO_NONE);

//	logmessage("openvpn", "OpenVPN client %d stopped.", unit);
}


void ovpn_stop_server(int unit) {
	char buffer[64];

	sprintf(buffer, "stop_vpnserver%d", unit);
	if (getpid() != 1) {
		notify_rc(buffer);
		return;
	}

	// Remove watchdog
	sprintf(buffer, "CheckVPNServer%d", unit);
	eval("cru", "d", buffer);

	// Stop the VPN server
	sprintf(buffer, "vpnserver%d", unit);
	killall_tk_period_wait(buffer, 10);

	ovpn_remove_iface(OVPN_TYPE_SERVER, unit);

	// Remove firewall rules
	sprintf(buffer, "/etc/openvpn/server%d/fw.sh", unit);
	if (!eval("sed", "-i", "s/-A/-D/g;s/-I/-D/g", buffer))
		eval(buffer);

	// Delete all files for this server
	sprintf(buffer, "/etc/openvpn/server%d",unit);
	eval("rm", "-rf", buffer);
	sprintf(buffer, "/etc/openvpn/vpnserver%d",unit);
	eval("rm", "-rf", buffer);

	update_ovpn_status(OVPN_TYPE_SERVER, unit, OVPN_STS_STOP, OVPN_ERRNO_NONE);

//	logmessage("openvpn", "OpenVPN server %d stopped.", unit);
}


void ovpn_process_eas(int start) {
	char enabled[32], buffer2[32];
	char *ptr;
	int unit;

	// Process servers
	strlcpy(enabled, nvram_safe_get("vpn_serverx_start"), sizeof(enabled));

	for (ptr = enabled; *ptr != '\0'; ptr++) {
		if (!isdigit(*ptr))
			continue;

		unit = atoi(ptr);

		if (unit > 0 && unit <= OVPN_SERVER_MAX) {
			sprintf(buffer2, "vpnserver%d", unit);
			if (pidof(buffer2) >= 0)
				ovpn_stop_server(unit);

			if (start)
				ovpn_start_server(unit);
		}
	}

	// Process clients
	strlcpy(enabled, nvram_safe_get("vpn_clientx_eas"), sizeof(enabled));

	for (ptr = enabled; *ptr != '\0'; ptr++) {
		if (!isdigit(*ptr))
			continue;

		unit = atoi(ptr);

		// Update kill switch states for clients set to auto-start with WAN
		ovpn_set_routing_rules(unit);
		ovpn_set_killswitch(unit);

		if (unit > 0 && unit <= OVPN_CLIENT_MAX) {
			sprintf(buffer2, "vpnclient%d", unit);
			if (pidof(buffer2) >= 0)
				ovpn_stop_client(unit);

			if (start)
				ovpn_start_client(unit);
		}
	}
}
