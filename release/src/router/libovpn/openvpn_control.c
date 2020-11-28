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

// Determine how to handle dnsmasq server list based on
// highest active dnsmode
int ovpn_max_dnsmode() {
	int unit, maxlevel = 0, level;
	char filename[40];
	char varname[32];

	for( unit = 1; unit <= OVPN_CLIENT_MAX; unit++ ) {
		sprintf(filename, "/etc/openvpn/client%d/client.resolv", unit);
		if (f_exists(filename)) {
			sprintf(varname, "vpn_client%d_", unit);
			level = nvram_pf_get_int(varname, "adns");

			// Ignore exclusive mode if policy mode is also enabled
			if ((nvram_pf_get_int(varname, "rgw") >= OVPN_RGW_POLICY ) && (level == OVPN_DNSMODE_EXCLUSIVE))
				continue;

			// Only return the highest active level, so one exclusive client
			// will override a relaxed client.
			if (level > maxlevel) maxlevel = level;
		}
	}
	return maxlevel;
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

void ovpn_update_routing(int unit){
	char cmd[56];
	snprintf(cmd, sizeof (cmd), "dev=tun1%d script_type=rmupdate /usr/sbin/vpnrouting.sh", unit);
	system(cmd);
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

	for (unit = 1; unit <= OVPN_CLIENT_MAX; unit++) {
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

void ovpn_client_down_handler(int unit)
{
	char buffer[64];
	char dirname[64];

	if ((unit < 1) || (unit > OVPN_CLIENT_MAX))
		return;

	sprintf(buffer, "DNSVPN%d", unit);
	eval("/usr/sbin/iptables", "-t", "nat", "-D", "PREROUTING", "-p", "udp", "-m", "udp", "--dport", "53", "-j", buffer);
	eval("/usr/sbin/iptables", "-t", "nat", "-D", "PREROUTING", "-p", "tcp", "-m", "tcp", "--dport", "53", "-j", buffer);
	eval("/usr/sbin/iptables", "-t", "nat", "-F", buffer);
	eval("/usr/sbin/iptables", "-t", "nat", "-X", buffer);

	sprintf(dirname, "/etc/openvpn/client%d", unit);

	sprintf(buffer, "%s/qos.sh", dirname);
	if (f_exists(buffer)) {
		eval("sed", "-i", "s/-A/-D/g", buffer);
		eval(buffer);
		unlink(buffer);
	}

	sprintf(buffer, "%s/dns.sh", dirname);
	if (f_exists(buffer))
		unlink(buffer);

	sprintf(buffer, "%s/client.resolv", dirname);
	if (f_exists(buffer))
		unlink(buffer);

	sprintf(buffer, "%s/client.conf", dirname);
	if (f_exists(buffer))
		unlink(buffer);
}


void ovpn_client_up_handler(int unit)
{
	char buffer[64];
	char dirname[64];
	char prefix[32];
	FILE *fp_dns = NULL, *fp_resolv = NULL, *fp_conf = NULL, *fp_qos = NULL;
	int setdns;
	int i, j;
	char *option, *option2;
	struct in_addr addr;

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

	// DNS stuff
	if (nvram_pf_get_int(prefix, "adns") == OVPN_DNSMODE_IGNORE)
		goto exit;

	sprintf(buffer, "%s/dns.sh", dirname);
	fp_dns = fopen(buffer, "w");
	if (!fp_dns)
		goto exit;

	fprintf(fp_dns, "#!/bin/sh\n"
	            "/usr/sbin/iptables -t nat -N DNSVPN%d\n",
	             unit);

	if ((nvram_pf_get_int(prefix, "rgw") >= 2) && (nvram_pf_get_int(prefix, "adns") == OVPN_DNSMODE_EXCLUSIVE))
		setdns = 0;	// Need to configure enforced DNS
	else
		setdns = -1;	// Do not enforce DNS

	// Parse foreign options
	for (i = 1; i < 999; i++) {
		sprintf(buffer, "foreign_option_%d", i);
		option = getenv(buffer);
		//logmessage("openvpn", "Checking %s", buffer);
		if (!option)
			break;

		if (!strncmp(option, "dhcp-option WINS ", 17)) {
			if (!inet_aton(&option[17], &addr))
				continue;

			if (!fp_conf) {
				sprintf(buffer, "%s/client.conf", dirname);
				if ((fp_conf = fopen(buffer, "w")) == NULL)
					goto exit;
			}
			fprintf(fp_conf, "dhcp-option=44,%s\n", &option[17]);

		} else if (!strncmp(option, "dhcp-option DNS ", 16)) {
			if (!inet_aton(&option[16], &addr))
				continue;

			if (!fp_resolv) {
				sprintf(buffer, "%s/client.resolv", dirname);
				if ((fp_resolv = fopen(buffer, "w")) == NULL)
					goto exit;
			}
			fprintf(fp_resolv, "server=%s\n", &option[16]);

			if (!setdns) {
				_set_exclusive_dns(fp_dns, unit, &option[16]);
				setdns = 1;
			}

			// Any search domains for that server
			for (j = 1; j < 999; j++) {
				sprintf(buffer, "foreign_option_%d", j);
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
	if (fp_dns) {
		fclose(fp_dns);
		sprintf(buffer, "%s/dns.sh", dirname);
		chmod(buffer, 0755);
		eval(buffer);
	}

	if (fp_resolv)
		fclose(fp_resolv);

	if (fp_conf)
		fclose(fp_conf);

	_ovpn_run_event_script();
}


void _set_exclusive_dns(FILE *fp, int unit, char *server) {
	char rules[2048], buffer[32];
	char *nvp, *entry;
	char *src, *dst, *iface, *name, *netptr;
	struct in_addr addr;
	int mask;

	if (!fp) return;

	sprintf(buffer, "vpn_client%d_clientlist", unit);
#ifdef HND_ROUTER
	nvram_split_get(buffer, rules, sizeof (rules), 5);
#else
	strlcpy(rules, nvram_safe_get(buffer), sizeof(rules));
#endif

	nvp = rules;

	while ((entry = strsep(&nvp, "<")) != NULL) {
		if (vstrsep(entry, ">", &name, &src, &dst, &iface) != 4)
			continue;

		if (*src) {
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
				if (!strcmp(iface, "VPN")) {
	                                fprintf(fp, "/usr/sbin/iptables -t nat -A DNSVPN%d -s %s -j DNAT --to-destination %s\n", unit, src, server);
	                                logmessage("openvpn", "Forcing %s to use DNS server %s", src, server);
	                        } else if (!strcmp(iface, "WAN")) {
	                                fprintf(fp, "/usr/sbin/iptables -t nat -I DNSVPN%d -s %s -j RETURN\n", unit, src);
	                                logmessage("openvpn", "Excluding %s from forced DNS routing", src);
	                        }
			}
		}
	}

	fprintf(fp, "/usr/sbin/iptables -t nat -I PREROUTING -p udp -m udp --dport 53 -j DNSVPN%d\n"
	            "/usr/sbin/iptables -t nat -I PREROUTING -p tcp -m tcp --dport 53 -j DNSVPN%d\n",
	             unit, unit);
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
		logmessage("openvpn", "OpenVPN client %d start attempt - already running.", unit);
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

	// Are we running?
	if (pidof(buffer) == -1)
		return;

	// Stop the VPN client
	killall_tk_period_wait(buffer, 10);

	ovpn_remove_iface(OVPN_TYPE_CLIENT, unit);

	// Remove firewall rules after VPN exit
	sprintf(buffer, "/etc/openvpn/client%d/fw.sh", unit);
	if (!eval("sed", "-i", "s/-A/-D/g;s/-I/-D/g", buffer))
		eval(buffer);

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
		ovpn_update_routing(unit);

		if (unit > 0 && unit <= OVPN_CLIENT_MAX) {
			sprintf(buffer2, "vpnclient%d", unit);
			if (pidof(buffer2) >= 0)
				ovpn_stop_client(unit);

			if (start)
				ovpn_start_client(unit);
		}
	}
}
