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

/* RC OpenVPN code for Asuswrt-Merlin. */

#include "rc.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>

#include <openvpn_config.h>
#include <openvpn_control.h>

#if 0
void _ovpn_update_routing_rules(int add_cmd, const int unit);
#endif


int ovpn_up_main(int argc, char **argv) {
	int unit;
	char buffer[64];

	if(argc < 3)
		return -1;

	unit = atoi(argv[1]);

	if (!strcmp(argv[2], "server"))
		ovpn_server_up_handler(unit);
	else if (!strcmp(argv[2], "client")) {
		ovpn_client_up_handler(unit);

		update_resolvconf();

		sprintf(buffer, "/etc/openvpn/client%d/client.conf", unit);
		// We got client.conf && update_resolvconf() won't restart it for us
		if ((f_exists(buffer)) && (ovpn_need_dnsmasq_restart()))
			notify_rc("start_dnsmasq");

#ifdef RTCONFIG_MULTILAN_CFG
		update_sdn_by_vpnc( get_vpnc_idx_by_proto_unit(VPN_PROTO_OVPN, unit) );
#endif

	} else
		return -1;

	return 0;
}

int ovpn_down_main(int argc, char **argv) {
	int unit, restart_dnsmasq = 0;
	char buffer[64];
	int vpnc_idx;

	if(argc < 3)
		return -1;

	unit = atoi(argv[1]);
	if (!strcmp(argv[2], "server"))
		ovpn_server_down_handler(unit);
	else if (!strcmp(argv[2], "client")) {
#ifdef RTCONFIG_MULTILAN_CFG
		update_sdn_by_vpnc( get_vpnc_idx_by_proto_unit(VPN_PROTO_OVPN, unit) );
#endif

#if 0	// for dev_policy
		// clean routing rule
		clean_routing_rule_by_vpnc_idx(vpnc_idx);	// from vpnc_fusion.c
#endif

		// Check before handler removes client.conf
		sprintf(buffer, "/etc/openvpn/client%d/client.conf", unit);
		// We got client.conf && update_resolvconf() won't restart it for us
		if ((f_exists(buffer)) && (ovpn_need_dnsmasq_restart()))
			restart_dnsmasq = 1;

		ovpn_client_down_handler(unit);

		update_resolvconf();

		if (restart_dnsmasq)
			notify_rc("start_dnsmasq");

	} else
		return -1;

	return 0;
}


#if 0	// Can handle SDN and dev_policy
void _ovpn_update_routing_rules(int add_cmd, const int unit) {
#ifdef RTCONFIG_VPN_FUSION_SUPPORT_INTERFACE
	char cmd_str[8], priority[8];
	int policy_cnt, i;
	VPNC_DEV_POLICY dev_policy[MAX_DEV_POLICY] = {{0}};
#endif
	int vpnc_idx;

	vpnc_idx = get_vpnc_idx_by_proto_unit(VPN_PROTO_OVPN, unit);

#ifdef RTCONFIG_MULTILAN_CFG
	update_sdn_by_vpnc(vpnc_idx);
#endif

#if defined(RTCONFIG_VPN_FUSION_SUPPORT_INTERFACE)
	policy_cnt = vpnc_get_dev_policy_list(dev_policy, MAX_DEV_POLICY, 0);
	snprintf(cmd_str, sizeof(cmd_str), "%s", (add_cmd == 0 ? "del" : "add");
	snprintf(id_str, sizeof(id_str), "%d", IP_ROUTE_TABLE_ID_VPNC_BASE + vpnc_idx);
	snprintf(priority, sizeof(priority), "%d", IP_RULE_PREF_VPNC_POLICY_IF + vpnc_idx * 3);

	for (i = 0; i < policy_cnt; ++i) {
		if(dev_policy[i].active && dev_policy[i].vpnc_idx == vpnc_idx && dev_policy[i].iif[0] != '\0') {
			if (add_cmd)
				eval("ip", "rule", "del", "priority", priority);

			eval("ip", "rule", (add_cmd ? "add" : "del"), "iif", dev_policy[i].iif, "table", id_str, "priority", priority);
		}
	}
#endif
}
#endif


int ovpn_route_up_main(int argc, char **argv) {
	ovpn_client_route_up_handler();
	return 0;
}

int ovpn_route_pre_down_main(int argc, char **argv) {
	ovpn_client_route_pre_down_handler();
	return 0;
}


/* Wrappers for library functions */
void inline start_ovpn_eas() {
	ovpn_process_eas(1);
}

void inline stop_ovpn_eas() {
	ovpn_process_eas(0);
}

void inline stop_ovpn_client(int unit) {
	ovpn_stop_client(unit);
#ifdef RTCONFIG_MULTILAN_CFG
	_vpnc_ipset_destroy( get_vpnc_idx_by_proto_unit(VPN_PROTO_OVPN, unit) );
#endif
}

void inline start_ovpn_client(int unit) {
#ifdef RTCONFIG_MULTILAN_CFG
	_vpnc_ipset_create( get_vpnc_idx_by_proto_unit(VPN_PROTO_OVPN, unit) );
#endif
	ovpn_start_client(unit);
}

void inline stop_ovpn_server(int unit) {
	ovpn_stop_server(unit);
}

void inline start_ovpn_server(int unit) {
	ovpn_start_server(unit);
}
