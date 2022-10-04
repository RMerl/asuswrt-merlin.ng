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
#include "amvpn_routing.h"


// Remove all rules pointing to a specific client table
// If unit is 0, then remove rules targetting main (i.e. WAN)
void amvpn_clear_routing_rules(int unit, vpndir_proto_t proto) {
	FILE *fp;
	char buffer[128], buffer2[128], buffer3[128];
	int prio, verb;
	char table[12], *lookup;
	char target_table[12];

	if (proto == VPNDIR_PROTO_OPENVPN) {
		snprintf(buffer, sizeof (buffer), "vpn_client%d_verb", unit);
		verb = nvram_get_int(buffer);
#ifdef RTCONFIG_WIREGUARD
	} else if (proto == VPNDIR_PROTO_WIREGUARD) {
		verb = 3;
#endif
	} else {
		return;
	}

	snprintf(buffer, sizeof (buffer), "/usr/sbin/ip rule show > /tmp/vpnrules%d_tmp", unit);
	system(buffer);

	snprintf(buffer, sizeof (buffer), "/tmp/vpnrules%d_tmp", unit);
	fp = fopen(buffer, "r");
	if (fp) {
		if (unit == 0)
			strlcpy(target_table, "main", sizeof (target_table));
		else if (proto == VPNDIR_PROTO_OPENVPN)
			snprintf(target_table, sizeof (target_table), "ovpnc%d", unit);
#ifdef RTCONFIG_WIREGUARD
		else if (proto == VPNDIR_PROTO_WIREGUARD)
			snprintf(target_table, sizeof (target_table), "wgc%d", unit);
#endif
		while (fgets(buffer2, sizeof(buffer2), fp) != NULL) {
			if (buffer2[strlen(buffer2)-1] == '\n')
				buffer2[strlen(buffer2)-1] = '\0';

			if (sscanf(buffer2, "%u", &prio) != 1)
				continue;

			// Only remove rules within our official range
			if ((prio < 10000) || (prio > (10209 + (10 * 200))))
				continue;

			if ((lookup = strstr(buffer2, "lookup")) == NULL)
				continue;

			if (sscanf(lookup, "lookup %11s", table) != 1)
				continue;
			if (strcmp(table, target_table))
				continue;

			snprintf(buffer3, sizeof (buffer3), "/usr/sbin/ip rule del prio %d", prio);
			if (verb >= 6)
				logmessage("vpndirector", "Removed rule %d", prio);
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
	10410-10609: OVPN 2
	10610-10809: OVPN 3
	10810-11009: OVPN 4
	11010-11209: OVPN 5

	11210-11409: WGC 1
	11410-11609: WGC 2
	11610-11809: WGC 3
	11810-12009: WGC 4
	12010-12209: WGC 5
*/

void amvpn_set_routing_rules(int unit, vpndir_proto_t proto) {
	char prefix[32], buffer[8000];
	int rgw, state, verb;

	if (unit < 1)
		return;

	if (proto == VPNDIR_PROTO_OPENVPN && unit <= OVPN_CLIENT_MAX) {
		snprintf(prefix, sizeof(prefix), "vpn_client%d_", unit);
		verb = nvram_pf_get_int(prefix, "verb");
		rgw = nvram_pf_get_int(prefix, "rgw");
#ifdef RTCONFIG_WIREGUARD
	} else if (proto == VPNDIR_PROTO_WIREGUARD && unit <= WG_CLIENT_MAX) {
		snprintf(prefix, sizeof(prefix), "wgc%d_", unit);
		verb = 3;
		rgw = OVPN_RGW_POLICY;
#endif
	} else {
		return;
	}

	amvpn_clear_routing_rules(unit, proto);

	/* Refresh WAN rules */
	amvpn_clear_routing_rules(0, proto);
	amvpn_get_policy_rules(0, buffer, sizeof (buffer), proto);
	_write_routing_rules(0, buffer, verb, proto);

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
			amvpn_get_policy_rules(unit, buffer, sizeof (buffer), proto);
			_write_routing_rules(unit, buffer, verb, proto);
			break;
	}
}


void _write_routing_rules(int unit, char *rules, int verb, vpndir_proto_t proto) {
	char *buffer_tmp, *buffer_tmp2, *rule;
	char buffer[128], table[16];
	int ruleprio, vpnprio, wanprio;
	char *enable, *desc, *target, *src, *dst;
	char srcstr[64], dststr[64];

	wanprio = VPNDIR_PRIO_WAN;

	if (proto == VPNDIR_PROTO_OPENVPN)
		vpnprio = VPNDIR_PRIO_OPENVPN + (VPNDIR_PRIO_MAX_RULES * (unit-1));
#ifdef RTCONFIG_WIREGUARD
	else if (proto == VPNDIR_PROTO_WIREGUARD)
		vpnprio = VPNDIR_PRIO_WIREGUARD + (VPNDIR_PRIO_MAX_RULES * (unit-1));
#endif
	else
		return;

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
#ifdef RTCONFIG_WIREGUARD
		else if (!strncmp(target, "WGC", 3)) {
			snprintf(table, sizeof (table), "wgc%d", unit);
			ruleprio = vpnprio++;
		}
#endif

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
			logmessage("vpndirector","Routing %s from %s to %s through %s", desc, (*src ? src : "any"), (*dst ? dst : "any"), table);

		system(buffer);
	}
	free(buffer_tmp);
}

inline void _flush_routing_cache() {
        system("/usr/sbin/ip route flush cache");
}


/* Remove/add server routes from all client routing tables when stopping/starting a client */

void update_client_routes(char *server_iface, int addroute) {
	int unit;
	char buffer[32];

	for (unit = 1; unit <= OVPN_CLIENT_MAX; unit++ ) {
		sprintf(buffer, "vpnclient%d", unit);
		if ( pidof(buffer) >= 0 ) {
			if (addroute)
				_add_server_routes(server_iface, unit, VPNDIR_PROTO_OPENVPN);
			else
				_del_server_routes(server_iface, unit, VPNDIR_PROTO_OPENVPN);
		}
	}

#ifdef RTCONFIG_WIREGUARD
	for (unit = 1; unit <= WG_CLIENT_MAX; unit++ ) {
		sprintf(buffer, "wgc%d_enable", unit);
		if (nvram_get_int(buffer)) {
			if (addroute)
				_add_server_routes(server_iface, unit, VPNDIR_PROTO_WIREGUARD);
			else
				_del_server_routes(server_iface, unit, VPNDIR_PROTO_WIREGUARD);
		}
	}
#endif
}


/* Add / remove OpenVPN server routes from client tables */
/* Server-agnostic, could eventually be reused for other servers like WG/IPSEC */

void _add_server_routes(char *server_iface, int client_unit, vpndir_proto_t proto) {
	char buffer[128], routecmd[128], line[128], table[8];
	FILE *fp_route;

	if (proto == VPNDIR_PROTO_OPENVPN)
		snprintf(table, sizeof (table), "ovpnc%d", client_unit);
#ifdef RTCONFIG_WIREGUARD
	else if (proto == VPNDIR_PROTO_WIREGUARD)
		snprintf(table, sizeof (table), "wgc%d", client_unit);
#endif
	else
		return;

	snprintf(buffer, sizeof (buffer), "/usr/sbin/ip route list table main | grep %s > /tmp/vpnroute_%s_tmp", server_iface, table);
	system(buffer);

	snprintf(buffer, sizeof (buffer), "/tmp/vpnroute_%s_tmp", table);
	fp_route = fopen(buffer, "r");

	if (fp_route) {
		while (fgets(line, sizeof(line), fp_route) != NULL) {
			snprintf(routecmd, sizeof (routecmd), "/usr/sbin/ip route add %s table %s", trimNL(line), table);
			system(routecmd);
		}
		fclose(fp_route);
	}
	unlink(buffer);
}


void _del_server_routes(char *server_iface, int client_unit, vpndir_proto_t proto) {
	char buffer[128], routecmd[128], line[128], table[8];
	FILE *fp_route;

	if (proto == VPNDIR_PROTO_OPENVPN)
		snprintf(table, sizeof (table), "ovpnc%d", client_unit);
#ifdef RTCONFIG_WIREGUARD
	else if (proto == VPNDIR_PROTO_WIREGUARD)
		snprintf(table, sizeof (table), "wgc%d", client_unit);
#endif
	else
		return;

	snprintf(buffer, sizeof (buffer), "/usr/sbin/ip route list table %s | grep %s > /tmp/vpnroute_%s_tmp", table, server_iface, table);
	system(buffer);

	snprintf(buffer, sizeof (buffer), "/tmp/vpnroute_%s_tmp", table);
	fp_route = fopen(buffer, "r");

	if (fp_route) {
		while (fgets(line, sizeof(line), fp_route) != NULL) {
			snprintf(routecmd, sizeof (routecmd), "/usr/sbin/ip route del %s table %s", trimNL(line), table);
			system(routecmd);
		}
		fclose(fp_route);
	}
	unlink(buffer);
}

// Unit -1 = all rules;  unit 0 = WAN rules,  unit 1-5: OVPN or WG instance rules
char *amvpn_get_policy_rules(int unit, char *buffer, int bufferlen, vpndir_proto_t proto)
{
	char filename[128];
	int datalen;
	char *buffer_tmp, *buffer_tmp2;
	char *rule, *enable, *desc, *src, *dst, *target;
	char entry[128];

	snprintf(filename, sizeof(filename), "%s/vpndirector_rulelist", OVPN_FS_PATH);

	datalen = f_read(filename, buffer, bufferlen-1);
	if (datalen < 0) {
		buffer[0] = '\0';
	} else {
		buffer[datalen] = '\0';
	}

	if (unit == -1)
		return buffer;

	buffer_tmp = buffer_tmp2 = strdup(buffer);
	buffer[0] = '\0';

	while (buffer_tmp && (rule = strsep(&buffer_tmp2, "<")) != NULL) {
		if((vstrsep(rule, ">", &enable, &desc, &src, &dst, &target)) != 5)
			continue;

		snprintf(entry, sizeof(entry),"<%s>%s>%s>%s>%s",enable, desc, src, dst, target);

		if (unit == 0 && !strcmp(target, "WAN")) {
			strlcat(buffer, entry, bufferlen);
		}
		else if (unit != 0 && !strncmp(target, "OVPN", 4) &&
		    strlen(target) > 4 &&
		    atoi(&target[4]) == unit &&
		    proto == VPNDIR_PROTO_OPENVPN) {
			strlcat(buffer, entry, bufferlen);
		}
#ifdef RTCONFIG_WIREGUARD
		else if (unit != 0 && !strncmp(target, "WGC", 3) &&
		    strlen(target) > 3 &&
		    atoi(&target[3]) == unit &&
		    proto == VPNDIR_PROTO_WIREGUARD) {
			strlcat(buffer, entry, bufferlen);
		}
#endif
	}
	free(buffer_tmp);

	return buffer;
}


int amvpn_set_policy_rules(char* buffer)
{
	char filename[128];

	if (!d_exists(OVPN_FS_PATH))
		mkdir(OVPN_FS_PATH, S_IRWXU);

	snprintf(filename, sizeof(filename), "%s/vpndirector_rulelist", OVPN_FS_PATH);
	if (f_write(filename, buffer, strlen(buffer), 0, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP) < 0)
		return -1;

	return 0;
}
