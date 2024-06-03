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
 * VPN utility library for Asuswrt-Merlin
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
	}
}

void ovpn_run_fw_nat_scripts(){
	char buffer[64];
	int unit;

	for (unit = 1; unit <= OVPN_SERVER_MAX; unit++) {
		snprintf(buffer, sizeof(buffer), "/etc/openvpn/server%d/fw_nat.sh", unit);
		if (f_exists(buffer))
			eval(buffer);
	}

	// Reverse order because of DNSVPN rules
	for (unit = OVPN_CLIENT_MAX; unit > 0; unit--) {
		snprintf(buffer, sizeof(buffer), "/etc/openvpn/client%d/fw_nat.sh", unit);
		if (f_exists(buffer))
			eval(buffer);

		snprintf(buffer, sizeof(buffer), "/etc/openvpn/client%d/dns.sh", unit);
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

	amvpn_clear_exclusive_dns(unit, VPNDIR_PROTO_OPENVPN);

	sprintf(dirname, "/etc/openvpn/client%d", unit);

	snprintf(buffer, sizeof(buffer), "%s/client.resolv", dirname);
	if (f_exists(buffer))
		unlink(buffer);

	snprintf(buffer, sizeof(buffer), "%s/client.conf", dirname);
	if (f_exists(buffer))
		unlink(buffer);
}


void ovpn_client_up_handler(int unit)
{
	char buffer[128], buffer2[128], buffer3[128];
	char dirname[64];
	char prefix[32];
	FILE *fp_resolv = NULL, *fp_conf = NULL, *fp_route = NULL;;
	int i, j, verb, rgw, lock;
	char *option, *option2;
	char *network_env, *netmask_env, *gateway_env, *metric_env, *remotegw_env, *dev_env;
	char *remote_env, *localgw;
	struct in_addr network, netmask;

	if ((unit < 1) || (unit > OVPN_CLIENT_MAX))
		return;

	snprintf(prefix, sizeof(prefix), "vpn_client%d_", unit);
	sprintf(dirname, "/etc/openvpn/client%d", unit);

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

		if (rgw == OVPN_RGW_NONE) {
			snprintf(buffer, sizeof (buffer), "/usr/sbin/ip route del default table ovpnc%d", unit);
			system(buffer);
			if (verb >= 6)
				logmessage("openvpn-routing", "Remove default gateway for client %d table", unit);
		} else {
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
				snprintf(buffer, sizeof (buffer), "/usr/sbin/ip route replace default via %s dev %s table ovpnc%d",
				         remotegw_env, dev_env, unit);
				if (verb >= 3)
					logmessage("openvpn-routing","Setting client %d routing table's default route through the tunnel", unit);
				system(buffer);
			} else {
				logmessage("openvpn-routing","WARNING: no VPN gateway provided, routing might not work properly!");
			}
		}

		amvpn_set_wan_routing_rules();
		amvpn_set_routing_rules(unit, VPNDIR_PROTO_OPENVPN);
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
		if (((nvram_pf_get_int(prefix, "rgw") == OVPN_RGW_POLICY) || (nvram_pf_get_int(prefix, "rgw") == OVPN_RGW_ALL)) &&
		     (nvram_pf_get_int(prefix, "adns") == OVPN_DNSMODE_EXCLUSIVE)) {
			lock = file_lock(VPNROUTING_LOCK);
			ovpn_set_exclusive_dns(unit);
			// Refresh prerouting rules to ensure correct order
			amvpn_update_exclusive_dns_rules();
			file_unlock(lock);
		}
	}

	if (fp_conf)
		fclose(fp_conf);
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

	// Update running ovpn client tables
	if (sconf->if_type == OVPN_IF_TUN)
		update_client_routes(sconf->if_name, 1);

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
	amvpn_clear_routing_rules(unit, VPNDIR_PROTO_OPENVPN);

	// Clear routing table, also freeing from killswitch set by down handler
	snprintf(buffer, sizeof (buffer),"/usr/sbin/ip route flush table ovpnc%d", unit);
	logmessage("openvpn-routing", "Clearing routing table for VPN client %d", unit);
	system(buffer);

	ovpn_remove_iface(OVPN_TYPE_CLIENT, unit);

	// Remove firewall rules after VPN exit
	sprintf(buffer, "/etc/openvpn/client%d/fw.sh", unit);
	if (f_exists(buffer)) {
		if (!eval("sed", "-i", "s/-[AI]/-D/g", buffer))
			eval(buffer);
	}

	sprintf(buffer, "/etc/openvpn/client%d/fw_nat.sh", unit);
	if (f_exists(buffer)) {
		if (!eval("sed", "-i", "s/-[AI]/-D/g", buffer))
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

	// Remove routes from running ovpn clients
	snprintf(buffer, sizeof(buffer), "vpn_server%d_if", unit);
	if (!strcmp(nvram_safe_get(buffer), "tun")) {
		snprintf(buffer, sizeof(buffer), "tun%d", OVPN_SERVER_BASE + unit);
		update_client_routes(buffer, 0);
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
	if (!eval("sed", "-i", "s/-[AI]/-D/g", buffer))
		eval(buffer);
	sprintf(buffer, "/etc/openvpn/server%d/fw_nat.sh", unit);
	if (!eval("sed", "-i", "s/-[AI]/-D/g", buffer))
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
	stop_ovpn_serverall();
	if (start)
		start_ovpn_serverall();

	// Process clients
	strlcpy(enabled, nvram_safe_get("vpn_clientx_eas"), sizeof(enabled));

	for (ptr = enabled; *ptr != '\0'; ptr++) {
		if (!isdigit(*ptr))
			continue;

		unit = atoi(ptr);

		// Update kill switch states for clients set to auto-start with WAN
		amvpn_set_wan_routing_rules();
		amvpn_set_routing_rules(unit, VPNDIR_PROTO_OPENVPN);
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

void start_ovpn_serverall() {
        char enabled[32];
        char *ptr;
        int unit;

	strlcpy(enabled, nvram_safe_get("vpn_serverx_start"), sizeof(enabled));

	for (ptr = enabled; *ptr != '\0'; ptr++) {
		if (!isdigit(*ptr))
			continue;

		unit = atoi(ptr);
		if (unit > 0 && unit <= OVPN_SERVER_MAX)
			ovpn_start_server(unit);
	}
}


void stop_ovpn_serverall() {
	char enabled[32], buffer[32];
	char *ptr;
	int unit;

	strlcpy(enabled, nvram_safe_get("vpn_serverx_start"), sizeof(enabled));

	for (ptr = enabled; *ptr != '\0'; ptr++) {
		if (!isdigit(*ptr))
			continue;

		unit = atoi(ptr);
		sprintf(buffer, "vpnserver%d", unit);
		if (pidof(buffer) >= 0)
			ovpn_stop_server(unit);
        }
}
