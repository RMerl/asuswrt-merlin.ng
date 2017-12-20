/*

	Copyright (C) 2008-2010 Keith Moyer, tomatovpn@keithmoyer.com
	Portions Copyright (C) 2012-2017 Eric Sauvageau

	No part of this file may be used without permission.

*/

#include <rc.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

#include <openvpn_config.h>
#include <openvpn_control.h>

// Line number as text string
#define __LINE_T__ __LINE_T_(__LINE__)
#define __LINE_T_(x) __LINE_T(x)
#define __LINE_T(x) # x

#define VPN_LOG_ERROR -1
#define VPN_LOG_NOTE 0
#define VPN_LOG_INFO 1
#define VPN_LOG_EXTRA 2
#define vpnlog(level,x...) if(nvram_get_int("vpn_debug")>=level) syslog(LOG_INFO, #level ": " __LINE_T__ ": " x)

#define CLIENT_IF_START 10
#define SERVER_IF_START 20

extern struct nvram_tuple router_defaults[];

#define PUSH_LAN_METRIC 500

static int ovpn_waitfor(const char *name)
{
	int pid, n = 5;

	killall_tk_period_wait(name, 10); //wait time in seconds
	while ( (pid = pidof(name)) >= 0 && (n-- > 0) )
	{
		// Reap the zombie if it has terminated
		waitpid(pid, NULL, WNOHANG);
		sleep(1);
	}
	return (pid >= 0);
}

void start_ovpn_client(int clientNum)
{
	FILE *fp;
	char iface[6];
	char buffer[256];
	char buffer2[4000];
	char *argv[6];
	int argc = 0;
	enum { TLS, SECRET } cryptMode = TLS;
	enum { TAP, TUN } ifType = TUN;
	enum { BRIDGE, NAT, NONE } routeMode = NONE;
	int nvi, ip[4], nm[4];
	long int nvl;
	int pid;
	int userauth, useronly;
	int taskset_ret;
	int i;
	char prefix[16];

	snprintf(prefix, sizeof(prefix), "vpn_client%d_", clientNum);

	sprintf(buffer, "start_vpnclient%d", clientNum);
	if (getpid() != 1) {
		notify_rc(buffer);
		return;
	}

	i = 0;
	while ((!nvram_get_int("ntp_ready")) && (i++ < 10)) {
		sleep(i*i);
	}

	vpnlog(VPN_LOG_INFO,"VPN GUI client backend starting...");

	if ( (pid = pidof(&buffer[6])) >= 0 )
	{
		vpnlog(VPN_LOG_NOTE, "VPN Client %d already running...", clientNum);
		vpnlog(VPN_LOG_INFO,"PID: %d", pid);
		return;
	}

	nvram_pf_set(prefix, "state", "1");	// Initializing
	nvram_pf_set(prefix, "errno", "0");
	nvram_pf_set(prefix, "rip", "");

	// Determine interface
	strlcpy(buffer, nvram_pf_safe_get(prefix, "if"), sizeof (buffer));

	if ( !strcmp(buffer, "tap") )
		ifType = TAP;
	else if ( !strcmp(buffer, "tun") )
		ifType = TUN;
	else
	{
		vpnlog(VPN_LOG_ERROR, "Invalid interface type, %.3s", buffer);
		return;
	}

	// Build interface name
	snprintf(iface, sizeof (iface), "%s%d", buffer, clientNum+CLIENT_IF_START);

	// Determine encryption mode
	strlcpy(buffer, nvram_pf_safe_get(prefix, "crypt"), sizeof(buffer) );

	if ( !strcmp(buffer, "tls") )
		cryptMode = TLS;
	else if ( !strcmp(buffer, "secret") )
		cryptMode = SECRET;
	else
	{
		vpnlog(VPN_LOG_ERROR,"Invalid encryption mode, %.6s", buffer);
		return;
	}

	// Determine if we should bridge the tunnel
	if ( ifType == TAP && nvram_pf_get_int(prefix, "bridge") == 1 )
		routeMode = BRIDGE;

	// Determine if we should NAT the tunnel
	if ( (ifType == TUN || routeMode != BRIDGE) && nvram_pf_get_int(prefix, "nat") == 1 )
		routeMode = NAT;

	// Make sure openvpn directory exists
	mkdir("/etc/openvpn", 0700);
	sprintf(buffer, "/etc/openvpn/client%d", clientNum);
	mkdir(buffer, 0700);

	// Make sure symbolic link exists
	sprintf(buffer, "/etc/openvpn/vpnclient%d", clientNum);
	unlink(buffer);
	if ( symlink("/usr/sbin/openvpn", buffer) )
	{
		vpnlog(VPN_LOG_ERROR,"Creating symlink failed...");
		stop_ovpn_client(clientNum);
		return;
	}

	// Make sure module is loaded
	modprobe("tun");
	f_wait_exists("/dev/net/tun", 5);

	// Create tap/tun interface
	sprintf(buffer, "openvpn --mktun --dev %s", iface);
	for (argv[argc=0] = strtok(buffer, " "); argv[argc] != NULL; argv[++argc] = strtok(NULL, " "));
	if ( _eval(argv, NULL, 0, NULL) )
	{
		vpnlog(VPN_LOG_ERROR,"Creating tunnel interface %s failed...",iface);
		stop_ovpn_client(clientNum);
		return;
	}

	// Bring interface up (TAP only)
	if( ifType == TAP )
	{
		if ( routeMode == BRIDGE )
		{
			snprintf(buffer, sizeof (buffer), "brctl addif %s %s", nvram_safe_get("lan_ifname"), iface);
			for (argv[argc=0] = strtok(buffer, " "); argv[argc] != NULL; argv[++argc] = strtok(NULL, " "));
			if ( _eval(argv, NULL, 0, NULL) )
			{
				vpnlog(VPN_LOG_ERROR,"Adding tunnel interface to bridge failed...");
				stop_ovpn_client(clientNum);
				return;
			}
		}

		snprintf(buffer, sizeof (buffer), "ifconfig %s promisc up", iface);
		for (argv[argc=0] = strtok(buffer, " "); argv[argc] != NULL; argv[++argc] = strtok(NULL, " "));
		if ( _eval(argv, NULL, 0, NULL) )
		{
			vpnlog(VPN_LOG_ERROR,"Bringing interface up failed...");
			stop_ovpn_client(clientNum);
			return;
		}
	}

	userauth = nvram_pf_get_int(prefix, "userauth");
	useronly = userauth && nvram_pf_get_int(prefix, "useronly");

	// Build and write config file
	vpnlog(VPN_LOG_EXTRA,"Writing config file");
	sprintf(buffer, "/etc/openvpn/client%d/config.ovpn", clientNum);
	fp = fopen(buffer, "w");
	chmod(buffer, S_IRUSR|S_IWUSR);
	fprintf(fp, "# Automatically generated configuration\n");
	fprintf(fp, "daemon ovpn-client%d\n", clientNum);
	if ( cryptMode == TLS )
		fprintf(fp, "client\n");
	fprintf(fp, "dev %s\n", iface);
	fprintf(fp, "proto %s\n", nvram_pf_safe_get(prefix, "proto"));
	fprintf(fp, "remote %s ", nvram_pf_safe_get(prefix, "addr"));
	fprintf(fp, "%d\n", nvram_pf_get_int(prefix, "port"));
	if ( cryptMode == SECRET )
	{
		if ( ifType == TUN )
		{
			fprintf(fp, "ifconfig %s ", nvram_pf_safe_get(prefix, "local"));
			fprintf(fp, "%s\n", nvram_pf_safe_get(prefix, "remote"));
		}
		else if ( ifType == TAP )
		{
			fprintf(fp, "ifconfig %s ", nvram_pf_safe_get(prefix, "local"));
			fprintf(fp, "%s\n", nvram_pf_safe_get(prefix, "nm"));
		}
	}
	if ( (nvi = nvram_pf_get_int(prefix, "retry")) >= 0 )
		fprintf(fp, "resolv-retry %d\n", nvi);
	else
		fprintf(fp, "resolv-retry infinite\n");
	fprintf(fp, "nobind\n");
	fprintf(fp, "persist-key\n");
	fprintf(fp, "persist-tun\n");

	strlcpy(buffer2, nvram_pf_safe_get(prefix, "comp"), sizeof (buffer2));
	if (strcmp(buffer2, "-1")) {
		if (!strcmp(buffer2, "lz4")) {
			fprintf(fp, "compress lz4\n");
		} else if (!strcmp(buffer2, "yes")) {
			fprintf(fp, "compress lzo\n");
		} else if (!strcmp(buffer2, "adaptive")) {
			fprintf(fp, "comp-lzo adaptive\n");
		} else if (!strcmp(buffer2, "no")) {
			fprintf(fp, "compress\n");      // Disable, but can be overriden
		}
	}

	if ( cryptMode == TLS ) {
		nvi = nvram_pf_get_int(prefix, "ncp_enable");
		strlcpy(buffer2, nvram_pf_safe_get(prefix, "ncp_ciphers"), sizeof (buffer2));
		if ((nvi > 0) && (buffer2[0] != '\0')) {
			fprintf(fp, "ncp-ciphers %s\n", buffer2);
		} else {
			nvi = 0;
			fprintf(fp, "ncp-disable\n");
		}
	} else {
		nvi = 0;
	}

	if (nvi != 2) {
		strlcpy(buffer, nvram_pf_safe_get(prefix, "cipher"), sizeof(buffer));
		if ( strcmp(buffer,"default") )
			fprintf(fp, "cipher %s\n", buffer);
	}

	strlcpy(buffer, nvram_pf_safe_get(prefix, "digest"), sizeof(buffer));
	if ( strcmp(buffer, "default"))
		fprintf(fp, "auth %s\n", buffer);

	nvi = nvram_pf_get_int(prefix, "rgw");
	if (nvi == 1)
	{
		if ( ifType == TAP && nvram_pf_safe_get(prefix, "gw")[0] != '\0' )
			fprintf(fp, "route-gateway %s\n", nvram_pf_safe_get(prefix, "gw"));
		fprintf(fp, "redirect-gateway def1\n");
	}

	// For selective routing
	sprintf(buffer, "/etc/openvpn/client%d/vpnrouting.sh", clientNum);
	symlink("/usr/sbin/vpnrouting.sh", buffer);
	fprintf(fp, "script-security 2\n");	// also for up/down scripts
	fprintf(fp, "route-delay 2\n");
	fprintf(fp, "route-up vpnrouting.sh\n");
	fprintf(fp, "route-pre-down vpnrouting.sh\n");

	fprintf(fp, "verb %d\n", nvram_pf_get_int(prefix, "verb"));

	if ( cryptMode == TLS )
	{
		if ( (nvl = atol(nvram_pf_safe_get(prefix, "reneg"))) >= 0 )
			fprintf(fp, "reneg-sec %ld\n", nvl);

		if ( nvram_pf_get_int(prefix, "adns") > 0 )
		{
			sprintf(buffer, "/etc/openvpn/client%d/updown.sh", clientNum);
			symlink("/usr/sbin/updown.sh", buffer);
			fprintf(fp, "up updown.sh\n");
			fprintf(fp, "down updown.sh\n");
		}

		nvi = nvram_pf_get_int(prefix, "hmac");
		if (ovpn_key_exists(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_STATIC) && (nvi >= 0))
		{
			if (nvi == 3)
				fprintf(fp, "tls-crypt static.key");
			else
				fprintf(fp, "tls-auth static.key");

			if ( nvi < 2 )
				fprintf(fp, " %d", nvi);
			fprintf(fp, "\n");
		}

		if (ovpn_key_exists(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_CA))
			fprintf(fp, "ca ca.crt\n");

		if (!useronly)
		{
			if (ovpn_key_exists(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_CERT))
				fprintf(fp, "cert client.crt\n");
			if (ovpn_key_exists(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_KEY))
				fprintf(fp, "key client.key\n");
		}
		if (nvram_pf_get_int(prefix, "tlsremote"))
		{
			fprintf(fp, "verify-x509-name \"%s\" name\n", nvram_pf_safe_get(prefix, "cn"));
		}
		if (userauth)
			fprintf(fp, "auth-user-pass up\n");

		if (ovpn_key_exists(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_CRL))
			fprintf(fp, "crl-verify crl.pem\n");

		if (ovpn_key_exists(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_CA_EXTRA))
			fprintf(fp, "extra-certs extra.pem\n");
	}
	else if ( cryptMode == SECRET )
	{
		if (ovpn_key_exists(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_STATIC))
			fprintf(fp, "secret static.key\n");

	}

	// All other cryptmodes need a default up/down script set
	if ( (cryptMode != TLS) && (check_if_file_exist("/jffs/scripts/openvpn-event")) )
	{
		sprintf(buffer, "/etc/openvpn/client%d/updown.sh", clientNum);
		symlink("/jffs/scripts/openvpn-event", buffer);
		fprintf(fp, "up updown.sh\n");
		fprintf(fp, "down updown.sh\n");
	}

	fprintf(fp, "status-version 2\n");
	fprintf(fp, "status status 5\n");
	fprintf(fp, "\n# Custom Configuration\n");
	fprintf(fp, "%s", get_ovpn_custom(OVPN_TYPE_CLIENT, clientNum, buffer2, sizeof (buffer2) ));
	fclose(fp);

	vpnlog(VPN_LOG_EXTRA,"Done writing config file");

	// Write certification and key files
	vpnlog(VPN_LOG_EXTRA,"Writing certs/keys");
	if ( cryptMode == TLS )
	{
		if (ovpn_key_exists(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_CA))
		{
			sprintf(buffer, "/etc/openvpn/client%d/ca.crt", clientNum);
			fp = fopen(buffer, "w");
			chmod(buffer, S_IRUSR|S_IWUSR);
			fprintf(fp, "%s", get_ovpn_key(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_CA, buffer2, sizeof(buffer2)));
			fclose(fp);
		}

		if (!useronly)
		{
			if (ovpn_key_exists(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_KEY))
			{
				sprintf(buffer, "/etc/openvpn/client%d/client.key", clientNum);
				fp = fopen(buffer, "w");
				chmod(buffer, S_IRUSR|S_IWUSR);
				fprintf(fp, "%s", get_ovpn_key(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_KEY, buffer2, sizeof(buffer2)));
				fclose(fp);
			}

			if (ovpn_key_exists(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_CERT))
			{
				sprintf(buffer, "/etc/openvpn/client%d/client.crt", clientNum);
				fp = fopen(buffer, "w");
				chmod(buffer, S_IRUSR|S_IWUSR);
				fprintf(fp, "%s", get_ovpn_key(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_CERT, buffer2, sizeof(buffer2)));
				fclose(fp);
			}
		}
		if (userauth)
		{
			sprintf(buffer, "/etc/openvpn/client%d/up", clientNum);
			fp = fopen(buffer, "w");
			chmod(buffer, S_IRUSR|S_IWUSR);
			fprintf(fp, "%s\n", nvram_pf_safe_get(prefix, "username"));
			fprintf(fp, "%s\n", nvram_pf_safe_get(prefix, "password"));
			fclose(fp);
		}

		if (ovpn_key_exists(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_CRL))
		{
			sprintf(buffer, "/etc/openvpn/client%d/crl.pem", clientNum);
			fp = fopen(buffer, "w");
			chmod(buffer, S_IRUSR|S_IWUSR);
			fprintf(fp, "%s", get_ovpn_key(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_CRL, buffer2, sizeof(buffer2)));
			fclose(fp);
		}

		if (ovpn_key_exists(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_CA_EXTRA))
		{
			sprintf(buffer, "/etc/openvpn/client%d/extra.pem", clientNum);
			fp = fopen(buffer, "w");
			chmod(buffer, S_IRUSR|S_IWUSR);
			fprintf(fp, "%s", get_ovpn_key(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_CA_EXTRA, buffer2, sizeof(buffer2)));
			fclose(fp);
		}
	}
	if ( cryptMode == SECRET || (cryptMode == TLS && nvram_pf_get_int(prefix, "hmac") >= 0) )
	{
		if (ovpn_key_exists(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_STATIC))
		{
			sprintf(buffer, "/etc/openvpn/client%d/static.key", clientNum);
			fp = fopen(buffer, "w");
			chmod(buffer, S_IRUSR|S_IWUSR);
			fprintf(fp, "%s", get_ovpn_key(OVPN_TYPE_CLIENT, clientNum, OVPN_CLIENT_STATIC, buffer2, sizeof(buffer2)));
			fclose(fp);
		}
	}
	vpnlog(VPN_LOG_EXTRA,"Done writing certs/keys");

	// Run postconf custom script on it if it exists
	sprintf(buffer, "openvpnclient%d", clientNum);
	sprintf(buffer2, "/etc/openvpn/client%d/config.ovpn", clientNum);
	run_postconf(buffer, buffer2);

	// Handle firewall rules if appropriate
	if ( !nvram_pf_match(prefix, "firewall", "custom") )
	{
		// Create firewall rules
		vpnlog(VPN_LOG_EXTRA,"Creating firewall rules");
		mkdir("/etc/openvpn/fw", 0700);
		sprintf(buffer, "/etc/openvpn/fw/client%d-fw.sh", clientNum);
		fp = fopen(buffer, "w");
		chmod(buffer, S_IRUSR|S_IWUSR|S_IXUSR);
		fprintf(fp, "#!/bin/sh\n");
		fprintf(fp, "iptables -I OVPN -i %s -j ACCEPT\n", iface);
#ifdef HND_ROUTER
		if (nvram_match("fc_disable", "0")) {
#else
		if (nvram_match("ctf_disable", "0")) {
#endif
			fprintf(fp, "iptables -t mangle -I PREROUTING -i %s -j MARK --set-mark 0x01/0x7\n", iface);
		}
#if !defined(HND_ROUTER)
		// Setup traffic accounting
		if (nvram_match("cstats_enable", "1")) {
			ipt_account(fp, iface);
		}
#endif
		if ( routeMode == NAT )
		{
			sscanf(nvram_safe_get("lan_ipaddr"), "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
			sscanf(nvram_safe_get("lan_netmask"), "%d.%d.%d.%d", &nm[0], &nm[1], &nm[2], &nm[3]);
			fprintf(fp, "iptables -t nat -I POSTROUTING -s %d.%d.%d.%d/%s -o %s -j MASQUERADE\n",
			        ip[0]&nm[0], ip[1]&nm[1], ip[2]&nm[2], ip[3]&nm[3], nvram_safe_get("lan_netmask"), iface);
		}
		// Disable rp_filter when in policy mode - firewall restart would re-enable it
		if (nvram_pf_get_int(prefix, "rgw") > 1) {
			fprintf(fp, "for i in /proc/sys/net/ipv4/conf/*/rp_filter ; do\n"); /* */
			fprintf(fp, "echo 0 > $i\n");
			fprintf(fp, "done\n");
		}

		fclose(fp);
		vpnlog(VPN_LOG_EXTRA,"Done creating firewall rules");

		// Run the firewall rules
		vpnlog(VPN_LOG_EXTRA,"Running firewall rules");
		sprintf(buffer, "/etc/openvpn/fw/client%d-fw.sh", clientNum);
		argv[0] = buffer;
		argv[1] = NULL;
		_eval(argv, NULL, 0, NULL);
		vpnlog(VPN_LOG_EXTRA,"Done running firewall rules");
	}

        // Start the VPN client
	sprintf(buffer, "/etc/openvpn/vpnclient%d", clientNum);
	sprintf(buffer2, "/etc/openvpn/client%d", clientNum);
	taskset_ret = cpu_eval(NULL, (clientNum % 2 == 0 ? CPU0 : CPU1), buffer, "--cd", buffer2, "--config", "config.ovpn");

	vpnlog(VPN_LOG_INFO,"Starting OpenVPN client %d", clientNum);

	if (taskset_ret)
	{
		vpnlog(VPN_LOG_ERROR,"Starting OpenVPN failed...");
		stop_ovpn_client(clientNum);
		return;
	}
	vpnlog(VPN_LOG_EXTRA,"Done starting openvpn");

	// Set up cron job
	if ( (nvi = nvram_pf_get_int(prefix, "poll")) > 0 )
	{
		vpnlog(VPN_LOG_EXTRA,"Adding cron job");
		argv[0] = "cru";
		argv[1] = "a";
		sprintf(buffer, "CheckVPNClient%d", clientNum);
		argv[2] = buffer;
		sprintf(&buffer[strlen(buffer)+1], "*/%d * * * * service start_vpnclient%d", nvi, clientNum);
		argv[3] = &buffer[strlen(buffer)+1];
		argv[4] = NULL;
		_eval(argv, NULL, 0, NULL);
		vpnlog(VPN_LOG_EXTRA,"Done adding cron job");
	}

	vpnlog(VPN_LOG_INFO,"VPN GUI client backend complete.");
}

void stop_ovpn_client(int clientNum)
{
	int argc;
	char *argv[7];
	char buffer[256];

	sprintf(buffer, "stop_vpnclient%d", clientNum);
	if (getpid() != 1) {
                notify_rc(buffer);
		return;
	}

	vpnlog(VPN_LOG_INFO,"Stopping VPN GUI client backend.");

	// Remove cron job
	vpnlog(VPN_LOG_EXTRA,"Removing cron job");
	argv[0] = "cru";
	argv[1] = "d";
	sprintf(buffer, "CheckVPNClient%d", clientNum);
	argv[2] = buffer;
	argv[3] = NULL;
	_eval(argv, NULL, 0, NULL);
	vpnlog(VPN_LOG_EXTRA,"Done removing cron job");

	// Stop the VPN client
	vpnlog(VPN_LOG_EXTRA,"Stopping OpenVPN client.");
	sprintf(buffer, "vpnclient%d", clientNum);
	if ( !ovpn_waitfor(buffer) )
		vpnlog(VPN_LOG_EXTRA,"OpenVPN client stopped.");

	// NVRAM setting for device type could have changed, just try to remove both
	vpnlog(VPN_LOG_EXTRA,"Removing VPN device.");
	sprintf(buffer, "openvpn --rmtun --dev tap%d", clientNum+CLIENT_IF_START);
	for (argv[argc=0] = strtok(buffer, " "); argv[argc] != NULL; argv[++argc] = strtok(NULL, " "));
	_eval(argv, NULL, 0, NULL);

	sprintf(buffer, "openvpn --rmtun --dev tun%d", clientNum+CLIENT_IF_START);
	for (argv[argc=0] = strtok(buffer, " "); argv[argc] != NULL; argv[++argc] = strtok(NULL, " "));
	_eval(argv, NULL, 0, NULL);
	vpnlog(VPN_LOG_EXTRA,"VPN device removed.");

	// Remove firewall rules after VPN exit
	vpnlog(VPN_LOG_EXTRA,"Removing firewall rules.");
	sprintf(buffer, "/etc/openvpn/fw/client%d-fw.sh", clientNum);
	argv[0] = "sed";
	argv[1] = "-i";
	argv[2] = "s/-A/-D/g;s/-I/-D/g";
	argv[3] = buffer;
	argv[4] = NULL;
	if (!_eval(argv, NULL, 0, NULL))
	{
		argv[0] = buffer;
		argv[1] = NULL;
		_eval(argv, NULL, 0, NULL);
	}
	vpnlog(VPN_LOG_EXTRA,"Done removing firewall rules.");

	if ( nvram_get_int("vpn_debug") <= VPN_LOG_EXTRA )
	{
		vpnlog(VPN_LOG_EXTRA,"Removing generated files.");
		// Delete all files for this client
		sprintf(buffer, "rm -rf /etc/openvpn/client%d /etc/openvpn/fw/client%d-fw.sh /etc/openvpn/vpnclient%d /etc/openvpn/dns/client%d.resolv",clientNum,clientNum,clientNum,clientNum);
		for (argv[argc=0] = strtok(buffer, " "); argv[argc] != NULL; argv[++argc] = strtok(NULL, " "));
		_eval(argv, NULL, 0, NULL);

		// Attempt to remove directories.  Will fail if not empty
		rmdir("/etc/openvpn/fw");
		rmdir("/etc/openvpn/dns");
		rmdir("/etc/openvpn");
		vpnlog(VPN_LOG_EXTRA,"Done removing generated files.");
	}

	snprintf(buffer, sizeof (buffer), "vpn_client%d_", clientNum);
	nvram_pf_set(buffer, "state","0");
	nvram_pf_set(buffer, "errno", "0");
	nvram_pf_set(buffer, "rip", "");
	update_resolvconf();

	vpnlog(VPN_LOG_INFO,"VPN GUI client backend stopped.");
}

void start_ovpn_server(int serverNum)
{
	FILE *fp, *ccd, *fp_client;;
	char iface[6];
	char buffer[256];
	char buffer2[4000];
	char *argv[6], *chp, *route;
	int argc = 0;
	int c2c = 0;
	enum { TAP, TUN } ifType = TUN;
	enum { TLS, SECRET } cryptMode = TLS;
	int nvi, ip[4], nm[4];
	long int nvl;
	int pid;
	int taskset_ret;
	char fpath[128];
	int valid = 0;
	int userauth = 0, useronly = 0;
	int i, len;
	char prefix[16];

	snprintf(prefix, sizeof (prefix), "vpn_server%d_", serverNum);

	sprintf(buffer, "start_vpnserver%d", serverNum);
	if (getpid() != 1) {
		notify_rc(buffer);
		return;
	}

	i = 0;
	while ((!nvram_get_int("ntp_ready")) && (i++ < 10)) {
		sleep(i*i);
	}

	vpnlog(VPN_LOG_INFO,"VPN GUI server backend starting...");

	if ( (pid = pidof(&buffer[6])) >= 0 )
	{
		vpnlog(VPN_LOG_NOTE, "VPN Server %d already running...", serverNum);
		vpnlog(VPN_LOG_INFO,"PID: %d", pid);
		return;
	}

	nvram_pf_set(prefix, "state", "1");	//initializing
	nvram_pf_set(prefix, "errno", "0");

	// Determine interface type
	strlcpy(buffer, nvram_pf_safe_get(prefix, "if"), sizeof (buffer) );
	if ( !strcmp(buffer, "tap") )
		ifType = TAP;
	else if ( !strcmp(buffer, "tun") )
		ifType = TUN;
	else
	{
		vpnlog(VPN_LOG_ERROR,"Invalid interface type, %.3s", buffer);
		return;
	}

	// Build interface name
	snprintf(iface, sizeof (iface), "%s%d", buffer, serverNum+SERVER_IF_START);

	if(is_intf_up(iface) && ifType == TAP) {
		eval("brctl", "delif", nvram_safe_get("lan_ifname"), iface);
	}

	// Determine encryption mode
	strlcpy(buffer, nvram_pf_safe_get(prefix, "crypt"), sizeof (buffer) );

	if ( !strcmp(buffer, "tls") )
		cryptMode = TLS;
	else if ( !strcmp(buffer, "secret") )
		cryptMode = SECRET;
	else
	{
		vpnlog(VPN_LOG_ERROR,"Invalid encryption mode, %.6s", buffer);
		return;
	}

	// Make sure openvpn directory exists
	mkdir("/etc/openvpn", 0700);
	sprintf(buffer, "/etc/openvpn/server%d", serverNum);
	mkdir(buffer, 0700);

	// Make sure symbolic link exists
	sprintf(buffer, "/etc/openvpn/vpnserver%d", serverNum);
	unlink(buffer);
	if ( symlink("/usr/sbin/openvpn", buffer) )
	{
		vpnlog(VPN_LOG_ERROR,"Creating symlink failed...");
		stop_ovpn_server(serverNum);
		return;
	}

	// Make sure module is loaded
	modprobe("tun");
	f_wait_exists("/dev/net/tun", 5);

	// Create tap/tun interface
	sprintf(buffer, "openvpn --mktun --dev %s", iface);
	for (argv[argc=0] = strtok(buffer, " "); argv[argc] != NULL; argv[++argc] = strtok(NULL, " "));
	if ( _eval(argv, NULL, 0, NULL) )
	{
		vpnlog(VPN_LOG_ERROR,"Creating tunnel interface %s failed...",iface);
		stop_ovpn_server(serverNum);
		return;
	}


	// Add interface to LAN bridge (TAP only)
	if( ifType == TAP )
	{
		snprintf(buffer, sizeof (buffer), "brctl addif %s %s", nvram_safe_get("lan_ifname"), iface);
		for (argv[argc=0] = strtok(buffer, " "); argv[argc] != NULL; argv[++argc] = strtok(NULL, " "));
		if ( _eval(argv, NULL, 0, NULL) )
		{
			vpnlog(VPN_LOG_ERROR,"Adding tunnel interface to bridge failed...");
			stop_ovpn_server(serverNum);
			return;
		}
	}

	// Bring interface up
	sprintf(buffer, "ifconfig %s 0.0.0.0 promisc up", iface);
	for (argv[argc=0] = strtok(buffer, " "); argv[argc] != NULL; argv[++argc] = strtok(NULL, " "));
	if ( _eval(argv, NULL, 0, NULL) )
	{
		vpnlog(VPN_LOG_ERROR,"Bringing up tunnel interface failed...");
		stop_ovpn_server(serverNum);
		return;
	}

	// Build and write config files
	vpnlog(VPN_LOG_EXTRA,"Writing config file");
	sprintf(buffer, "/etc/openvpn/server%d/config.ovpn", serverNum);
	fp = fopen(buffer, "w");
	chmod(buffer, S_IRUSR|S_IWUSR);
	fprintf(fp, "# Automatically generated configuration\n");
	fprintf(fp, "daemon ovpn-server%d\n", serverNum);

	sprintf(buffer, "/etc/openvpn/server%d/client.ovpn", serverNum);
	fp_client = fopen(buffer, "w");

	if ( cryptMode == TLS )
	{
		fprintf(fp_client, "client\n");
		if ( ifType == TUN )
		{
			fprintf(fp, "topology subnet\n");
			fprintf(fp, "server %s ", nvram_pf_safe_get(prefix, "sn"));
			fprintf(fp, "%s\n", nvram_pf_safe_get(prefix, "nm"));
			fprintf(fp_client, "dev tun\n");
		}
		else if ( ifType == TAP )
		{
			fprintf(fp, "server-bridge");
			if ( nvram_pf_get_int(prefix, "dhcp") == 0 )
			{
				fprintf(fp, " %s ", nvram_safe_get("lan_ipaddr"));
				fprintf(fp, "%s ", nvram_safe_get("lan_netmask"));
				fprintf(fp, "%s ", nvram_pf_safe_get(prefix, "r1"));
				fprintf(fp, "%s", nvram_pf_safe_get(prefix, "r2"));
			}
			else
			{
				fprintf(fp, "\npush \"route 0.0.0.0 255.255.255.255 net_gateway\"");
			}

			fprintf(fp, "\n");
			fprintf(fp_client, "dev tap\n");
			fprintf(fp_client, "# Windows needs the TAP-Win32 adapter name\n");
			fprintf(fp_client, "# from the Network Connections panel\n");
			fprintf(fp_client, "# if you have more than one.  On XP SP2,\n");
			fprintf(fp_client, "# you may need to disable the firewall\n");
			fprintf(fp_client, "# for the TAP adapter.\n");
			fprintf(fp_client, ";dev-node MyTap\n");
		}
	}
	else if ( cryptMode == SECRET )
	{
		fprintf(fp_client, "mode p2p\n");

		if ( ifType == TUN )
		{
			fprintf(fp, "ifconfig %s ", nvram_pf_safe_get(prefix, "local"));
			fprintf(fp, "%s\n", nvram_pf_safe_get(prefix, "remote"));

			fprintf(fp_client, "dev tun\n");
			fprintf(fp_client, "ifconfig %s ", nvram_pf_safe_get(prefix, "remote"));
			fprintf(fp_client, "%s\n", nvram_pf_safe_get(prefix, "local"));
		}
		else if ( ifType == TAP )
		{
			fprintf(fp_client, "dev tap\n");
		}
	}
	//protocol
	strlcpy(buffer, nvram_pf_safe_get(prefix, "proto"), sizeof (buffer));
	fprintf(fp, "proto %s\n", buffer);
	if(!strcmp(buffer, "udp"))
		fprintf(fp_client, "proto %s\n", buffer);
	else
		fprintf(fp_client, "proto tcp-client\n");

	//port
	nvi = nvram_pf_get_int(prefix, "port");
	fprintf(fp, "port %d\n", nvi);

	if(nvram_get_int("ddns_enable_x"))
	{
		if (nvram_match("ddns_server_x","WWW.NAMECHEAP.COM"))
			fprintf(fp_client, "remote %s.%s %d\n", nvram_safe_get("ddns_hostname_x"), nvram_safe_get("ddns_username_x"), nvi);
		else
			fprintf(fp_client, "remote %s %d\n",
			    (strlen(nvram_safe_get("ddns_hostname_x")) ? nvram_safe_get("ddns_hostname_x") : nvram_safe_get("wan0_ipaddr")),
			    nvi);
	}
	else
	{
		fprintf(fp_client, "remote %s %d\n", nvram_safe_get("wan0_ipaddr"), nvi);
	}
	fprintf(fp_client, "float\n");
	fprintf(fp, "dev %s\n", iface);

	//cipher
	if ( cryptMode == TLS ) {
		nvi = nvram_pf_get_int(prefix, "ncp_enable");
		strlcpy(buffer2, nvram_pf_safe_get(prefix, "ncp_ciphers"), sizeof (buffer2));
		if ((nvi > 0) && (buffer2[0] != '\0')) {
			fprintf(fp, "ncp-ciphers %s\n", buffer2);
			fprintf(fp_client, "ncp-ciphers %s\n", buffer2);
		} else {
			nvi = 0;
			fprintf(fp, "ncp-disable\n");
		}
	} else {
		nvi = 0;
	}
	if (nvi != 2) {
		strlcpy(buffer, nvram_pf_safe_get(prefix, "cipher"), sizeof (buffer));
		if ( strcmp(buffer, "default") ) {
			fprintf(fp, "cipher %s\n", buffer);
			fprintf(fp_client, "cipher %s\n", buffer);
		}
	}

	//digest
	strlcpy(buffer, nvram_pf_safe_get(prefix, "digest"), sizeof (buffer));
	if ( strcmp(buffer, "default") ) {
		fprintf(fp, "auth %s\n", buffer);
		fprintf(fp_client, "auth %s\n", buffer);
	}

	//compression
	strlcpy(buffer, nvram_pf_safe_get(prefix, "comp"), sizeof (buffer));
	if (strcmp(buffer, "-1")) {
		if (!strcmp(buffer, "lz4")) {
			fprintf(fp, "compress lz4\n");
			fprintf(fp_client, "compress lz4\n");
		} else if (!strcmp(buffer, "yes")) {
			fprintf(fp, "compress lzo\n");
			fprintf(fp_client, "comp-lzo yes\n");
		} else if (!strcmp(buffer, "adaptive")) {
			fprintf(fp, "comp-lzo adaptive\n");
			fprintf(fp_client, "comp-lzo adaptive\n");
		} else if (!strcmp(buffer, "no")) {
			fprintf(fp, "compress\n");	// Disable, but client can override if desired
			fprintf(fp_client, "comp-lzo no\n");
		}
	}

	fprintf(fp, "keepalive 15 60\n");
	fprintf(fp_client, "keepalive 15 60\n");

	fprintf(fp, "verb %d\n", nvram_pf_get_int(prefix, "verb"));

	if ( cryptMode == TLS )
	{
		//TLS Renegotiation Time
		if ( (nvl = atol(nvram_pf_safe_get(prefix, "reneg"))) >= 0 ) {
			fprintf(fp, "reneg-sec %ld\n", nvl);
			fprintf(fp_client, "reneg-sec %ld\n", nvl);
		}

		if ( ifType == TUN && nvram_pf_get_int(prefix, "plan") )
		{
			sscanf(nvram_safe_get("lan_ipaddr"), "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
			sscanf(nvram_safe_get("lan_netmask"), "%d.%d.%d.%d", &nm[0], &nm[1], &nm[2], &nm[3]);
			fprintf(fp, "push \"route %d.%d.%d.%d %s vpn_gateway %d\"\n",
				ip[0]&nm[0], ip[1]&nm[1], ip[2]&nm[2], ip[3]&nm[3],
				nvram_safe_get("lan_netmask"), PUSH_LAN_METRIC);
		}

		if ( nvram_pf_get_int(prefix, "ccd") )
		{
			fprintf(fp, "client-config-dir ccd\n");

			if ( (c2c = nvram_pf_get_int(prefix, "c2c")) )
				fprintf(fp, "client-to-client\n");

			if ( nvram_pf_get_int(prefix, "ccd_excl") )
				fprintf(fp, "ccd-exclusive\n");
			else
				fprintf(fp, "duplicate-cn\n");

			sprintf(buffer, "/etc/openvpn/server%d/ccd", serverNum);
			mkdir(buffer, 0700);
			chdir(buffer);

			strcpy(buffer, nvram_pf_safe_get(prefix, "ccd_val"));
			chp = strtok(buffer,"<");
			while ( chp != NULL )
			{
				nvi = strlen(chp);

				chp[strcspn(chp,">")] = '\0';
				vpnlog(VPN_LOG_EXTRA,"CCD: enabled: %d", atoi(chp));
				if ( atoi(chp) == 1 )
				{
					nvi -= strlen(chp)+1;
					chp += strlen(chp)+1;

					ccd = NULL;
					route = NULL;
					if ( nvi > 0 )
					{
						chp[strcspn(chp,">")] = '\0';
						vpnlog(VPN_LOG_EXTRA,"CCD: Common name: %s", chp);
						ccd = fopen(chp, "w");
						chmod(chp, S_IRUSR|S_IWUSR);

						nvi -= strlen(chp)+1;
						chp += strlen(chp)+1;
					}
					if ( nvi > 0 && ccd != NULL && strcspn(chp,">") != strlen(chp) )
					{
						chp[strcspn(chp,">")] = ' ';
						chp[strcspn(chp,">")] = '\0';
						route = chp;
						vpnlog(VPN_LOG_EXTRA,"CCD: Route: %s", chp);
						if ( strlen(route) > 1 )
						{
							fprintf(ccd, "iroute %s\n", route);
							fprintf(fp, "route %s\n", route);
						}

						nvi -= strlen(chp)+1;
						chp += strlen(chp)+1;
					}
					if ( ccd != NULL )
						fclose(ccd);
					if ( nvi > 0 && route != NULL )
					{
						chp[strcspn(chp,">")] = '\0';
						vpnlog(VPN_LOG_EXTRA,"CCD: Push: %d", atoi(chp));
						if ( c2c && atoi(chp) == 1 && strlen(route) > 1 )
							fprintf(fp, "push \"route %s\"\n", route);

						nvi -= strlen(chp)+1;
						chp += strlen(chp)+1;
					}

					vpnlog(VPN_LOG_EXTRA,"CCD leftover: %d", nvi+1);
				}
				// Advance to next entry
				chp = strtok(NULL, "<");
			}
			// Copy any user-configured client config files
			sprintf(buffer, "/jffs/configs/openvpn/ccd%d", serverNum);

			if(check_if_dir_exist(buffer))
			{
				vpnlog(VPN_LOG_EXTRA,"CCD - copying user files");
				sprintf(buffer2, "cp %s/* .", buffer); /* */
				system(buffer2);
			}

			vpnlog(VPN_LOG_EXTRA,"CCD processing complete");
		}
		else
			fprintf(fp, "duplicate-cn\n");

		if ( nvram_pf_get_int(prefix, "pdns") )
		{
			if ( nvram_safe_get("wan_domain")[0] != '\0' )
				fprintf(fp, "push \"dhcp-option DOMAIN %s\"\n", nvram_safe_get("wan_domain"));
			fprintf(fp, "push \"dhcp-option DNS %s\"\n", nvram_safe_get("lan_ipaddr"));
		}

		if ( nvram_pf_get_int(prefix, "rgw") )
		{
			if ( ifType == TAP )
				fprintf(fp, "push \"route-gateway %s\"\n", nvram_safe_get("lan_ipaddr"));
			fprintf(fp, "push \"redirect-gateway def1\"\n");
		}

		nvi = nvram_pf_get_int(prefix, "hmac");
		//if (ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_STATIC))
		if ( nvi >= 0 )
		{
			if (nvi == 3)
				fprintf(fp, "tls-crypt static.key");
			else
				fprintf(fp, "tls-auth static.key");

			if ( nvi < 2 )
				fprintf(fp, " %d", nvi);
			fprintf(fp, "\n");
		}

		// Enable username/pass authentication (for Asus's PAM support)
		userauth = nvram_pf_get_int(prefix, "userpass_auth");
		useronly = userauth && nvram_pf_get_int(prefix, "igncrt");

		if ( userauth ) {
			//authentication
			fprintf(fp, "plugin /usr/lib/openvpn-plugin-auth-pam.so openvpn\n");
			fprintf(fp_client, "auth-user-pass\n");

			//ignore client certificate, but only if user/pass auth is enabled
			//That way, existing configuration (pre-Asus OVPN)
			//will remain unchanged.
			if ( useronly ) {
				fprintf(fp, "verify-client-cert none\n");
				fprintf(fp, "username-as-common-name\n");
			}
		}

		fprintf(fp_client, "remote-cert-tls server\n");
		//if (ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CA))
			fprintf(fp, "ca ca.crt\n");
		if ( !strncmp( get_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_DH, buffer2, sizeof(buffer2)), "none", 4))
			fprintf(fp, "dh none\n");
		else
			fprintf(fp, "dh dh.pem\n");
		//if (ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CERT))
			fprintf(fp, "cert server.crt\n");
		//if (ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_KEY))
			fprintf(fp, "key server.key\n");
		if (ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CRL))
			fprintf(fp, "crl-verify crl.pem\n");
		if (ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CA_EXTRA))
			fprintf(fp, "extra-certs extra.pem\n");
	}
	else if ( cryptMode == SECRET )
	{
		//if (ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_STATIC))
			fprintf(fp, "secret static.key\n");
	}

	if (check_if_file_exist("/jffs/scripts/openvpn-event"))
	{
		sprintf(buffer, "/etc/openvpn/server%d/updown.sh", serverNum);
		symlink("/jffs/scripts/openvpn-event", buffer);
		fprintf(fp, "script-security 2\n");
		fprintf(fp, "up updown.sh\n");
		fprintf(fp, "down updown.sh\n");
	}

	fprintf(fp, "status-version 2\n");
	fprintf(fp, "status status 5\n");
	fprintf(fp, "\n# Custom Configuration\n");
	fprintf(fp, "%s", get_ovpn_custom(OVPN_TYPE_SERVER, serverNum, buffer2, sizeof (buffer2) ));
	fclose(fp);

	vpnlog(VPN_LOG_EXTRA,"Done writing server config file");

	// Write certification and key files
	vpnlog(VPN_LOG_EXTRA,"Writing certs/keys");
	if ( cryptMode == TLS )
	{
		//generate certification and key

		if ( !ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CA) ||
		     !ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_KEY) ||
		     !ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CERT))
		{

			sprintf(fpath, "/tmp/genvpncert.sh");
			fp = fopen(fpath, "w");
			if(fp) {
				fprintf(fp, "#!/bin/sh\n");
				//fprintf(fp, ". /rom/easy-rsa/vars\n");
				fprintf(fp, "export OPENSSL=\"openssl\"\n");
				fprintf(fp, "export GREP=\"grep\"\n");
				fprintf(fp, "export KEY_CONFIG=\"/rom/easy-rsa/openssl-1.0.0.cnf\"\n");
				fprintf(fp, "export KEY_DIR=\"/etc/openvpn/server%d\"\n", serverNum);
				fprintf(fp, "export KEY_SIZE=%d\n", (nvram_pf_get_int(prefix, "tls_keysize") ? 2048 : 1024));
				fprintf(fp, "export CA_EXPIRE=3650\n");
				fprintf(fp, "export KEY_EXPIRE=3650\n");
				fprintf(fp, "export KEY_COUNTRY=\"TW\"\n");
				fprintf(fp, "export KEY_PROVINCE=\"TW\"\n");
				fprintf(fp, "export KEY_CITY=\"Taipei\"\n");
				fprintf(fp, "export KEY_ORG=\"ASUS\"\n");
				fprintf(fp, "export KEY_EMAIL=\"me@myhost.mydomain\"\n");
				fprintf(fp, "export KEY_CN=\"%s\"\n", nvram_safe_get("productid"));
				fprintf(fp, "touch /etc/openvpn/server%d/index.txt\n", serverNum);
				fprintf(fp, "echo 01 >/etc/openvpn/server%d/serial\n", serverNum);
				fprintf(fp, "/rom/easy-rsa/pkitool --initca\n");
				fprintf(fp, "/rom/easy-rsa/pkitool --server server\n");

				//undefined common name, default use username-as-common-name
				fprintf(fp, "export KEY_CN=\"\"\n");
				fprintf(fp, "/rom/easy-rsa/pkitool client\n");

				fclose(fp);
				chmod(fpath, 0700);
				eval(fpath);
				unlink(fpath);
			}

			//set certification and key to nvram
			sprintf(fpath, "/etc/openvpn/server%d/ca.key", serverNum);
			set_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CA_KEY, NULL, fpath);

			sprintf(fpath, "/etc/openvpn/server%d/ca.crt", serverNum);
			set_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CA, NULL, fpath);

			sprintf(fpath, "/etc/openvpn/server%d/server.key", serverNum);
			set_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_KEY, NULL, fpath);

			sprintf(fpath, "/etc/openvpn/server%d/server.crt", serverNum);
			set_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CERT, NULL, fpath);

			sprintf(fpath, "/etc/openvpn/server%d/client.key", serverNum);
			set_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CLIENT_KEY, NULL, fpath);

			sprintf(fpath, "/etc/openvpn/server%d/client.crt", serverNum);
			set_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CLIENT_CERT, NULL, fpath);
		}
		else {
				sprintf(buffer, "/etc/openvpn/server%d/ca.key", serverNum);
				fp = fopen(buffer, "w");
				chmod(buffer, S_IRUSR|S_IWUSR);
				fprintf(fp, "%s", get_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CA_KEY, buffer2, sizeof(buffer2)));
				fclose(fp);

			//if (ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CA))
			//{
				sprintf(buffer, "/etc/openvpn/server%d/ca.crt", serverNum);
				fp = fopen(buffer, "w");
				chmod(buffer, S_IRUSR|S_IWUSR);
				fprintf(fp, "%s", get_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CA, buffer2, sizeof(buffer2)));
				fclose(fp);
			//}

			//if (ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_KEY))
			//{
				sprintf(buffer, "/etc/openvpn/server%d/server.key", serverNum);
				fp = fopen(buffer, "w");
				chmod(buffer, S_IRUSR|S_IWUSR);
				fprintf(fp, "%s", get_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_KEY, buffer2, sizeof(buffer2)));
				fclose(fp);
			//}

			//if (ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CRT))
			//{
				sprintf(buffer, "/etc/openvpn/server%d/server.crt", serverNum);
				fp = fopen(buffer, "w");
				chmod(buffer, S_IRUSR|S_IWUSR);
				fprintf(fp, "%s", get_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CERT, buffer2, sizeof(buffer2)));
				fclose(fp);
			//}

			if (ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CRL))
			{
				sprintf(buffer, "/etc/openvpn/server%d/crl.pem", serverNum);
				fp = fopen(buffer, "w");
				chmod(buffer, S_IRUSR|S_IWUSR);
				fprintf(fp, "%s", get_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CRL, buffer2, sizeof(buffer2)));
				fclose(fp);
			}

			if (ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CA_EXTRA))
			{
				sprintf(buffer, "/etc/openvpn/server%d/extra.pem", serverNum);
				fp = fopen(buffer, "w");
				chmod(buffer, S_IRUSR|S_IWUSR);
				fprintf(fp, "%s", get_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CA_EXTRA, buffer2, sizeof(buffer2)));
				fclose(fp);
			}

		}

		fprintf(fp_client, "<ca>\n");
		fprintf(fp_client, "%s", get_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CA, buffer2, sizeof(buffer2)));
		len = strlen(buffer2);
		if ((len) && (buffer2[len-1] != '\n'))
			fprintf(fp_client, "\n");	// Append newline if missing
		fprintf(fp_client, "</ca>\n");

		if (ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CA_EXTRA)) {
			fprintf(fp_client, "<extra-certs>\n");
			fprintf(fp_client, "%s", get_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CA_EXTRA, buffer2, sizeof(buffer2)));
			len = strlen(buffer2);
			if ((len) && (buffer2[len-1] != '\n'))
				fprintf(fp_client, "\n");       // Append newline if missing
			fprintf(fp_client, "</extra-certs>\n");
		}

		// Only do this if we do not have both userauth and useronly enabled at the same time
		if ( !(userauth && useronly) )
		{
			/*
			   See if stored client cert was signed with our stored CA.  If not, it means
			   the CA was changed by the user and the current client crt/key no longer match,
			   so we should not insert them in the exported client ovp file.
			*/
			fp = fopen("/tmp/test.crt", "w");
			fprintf(fp, "%s", get_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CLIENT_CERT, buffer2, sizeof(buffer2)));
			fclose(fp);

			sprintf(buffer, "/usr/sbin/openssl verify -CAfile /etc/openvpn/server%d/ca.crt /tmp/test.crt > /tmp/output.txt", serverNum);
			system(buffer);
			f_read_string("/tmp/output.txt", buffer, 64);
	                unlink("/tmp/test.crt");

			if (!strncmp(buffer,"/tmp/test.crt: OK",17))
				valid = 1;

			fprintf(fp_client, "<cert>\n");
			if ((valid == 1) && ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CLIENT_CERT)) {
				fprintf(fp_client, "%s", get_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CLIENT_CERT, buffer2, sizeof(buffer2)));
				len = strlen(buffer2);
				if ((len) && (buffer2[len-1] != '\n'))
					fprintf(fp_client, "\n");       // Append newline if missing
			} else {
				fprintf(fp_client, "    paste client certificate data here\n");
			}
			fprintf(fp_client, "</cert>\n");

			fprintf(fp_client, "<key>\n");
			if ((valid == 1) && ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CLIENT_KEY) ) {
				fprintf(fp_client, "%s", get_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_CLIENT_KEY, buffer2, sizeof(buffer2)));
				len = strlen(buffer2);
				if ((len) && (buffer2[len-1] != '\n'))
					fprintf(fp_client, "\n");       // Append newline if missing
			} else {
				fprintf(fp_client, "    paste client key data here\n");
			}
			fprintf(fp_client, "</key>\n");
		}

		valid = 0;

		if ( ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_DH))
		{
			sprintf(buffer, "/etc/openvpn/server%d/dh.pem", serverNum);
			fp = fopen(buffer, "w");
			chmod(buffer, S_IRUSR|S_IWUSR);
			fprintf(fp, "%s", get_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_DH, buffer2, sizeof(buffer2)));
			fclose(fp);
			valid = 1;	// Tentative state
			if (strncmp(buffer2, "none", 4))	// If not set to "none" then validate it
			{
				// Validate DH strength
				sprintf(buffer, "/usr/sbin/openssl dhparam -in /etc/openvpn/server%d/dh.pem -text | grep \"DH Parameters:\" > /tmp/output.txt", serverNum);
				system(buffer);
				if (f_read_string("/tmp/output.txt", buffer, 64) > 0) {
					if (sscanf(strstr(buffer,"DH Parameters"),"DH Parameters: (%d bit)", &i)) {
						if (i < 1024) {
							logmessage("openvpn","WARNING: DH for server %d is too weak (%d bit, must be at least 1024 bit). Using a pre-generated 2048-bit PEM.", serverNum, i);
							valid = 0;      // Not valid after all, must regenerate
						}
					}
				}
			}
		}
		if (valid == 0)
		{	// Provide a 2048-bit PEM, from RFC 3526.
			sprintf(fpath, "/etc/openvpn/server%d/dh.pem", serverNum);
			eval("cp", "/etc/ssl/certs/dh2048.pem", fpath);
			set_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_DH, NULL, fpath);
		}
	}

	nvi = nvram_pf_get_int(prefix, "hmac");
	if ( cryptMode == SECRET || (cryptMode == TLS && nvi >= 0) )
	{
		if (ovpn_key_exists(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_STATIC) )
		{
			sprintf(buffer, "/etc/openvpn/server%d/static.key", serverNum);
			fp = fopen(buffer, "w");
			chmod(buffer, S_IRUSR|S_IWUSR);
			fprintf(fp, "%s", get_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_STATIC, buffer2, sizeof(buffer2)));
			fclose(fp);
		}
		else
		{	//generate openvpn static key
			sprintf(fpath, "/etc/openvpn/server%d/static.key", serverNum);
			eval("openvpn", "--genkey", "--secret", fpath);
			sleep(2);
			set_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_STATIC, NULL, fpath);
		}

		if(cryptMode == TLS)
			if (nvi == 3)
				fprintf(fp_client, "<tls-crypt>\n");
			else
				fprintf(fp_client, "<tls-auth>\n");
		else if(cryptMode == SECRET)
			fprintf(fp_client, "<secret>\n");
		fprintf(fp_client, "%s", get_ovpn_key(OVPN_TYPE_SERVER, serverNum, OVPN_SERVER_STATIC, buffer2, sizeof(buffer2)));
		len = strlen(buffer2);
		if ((len) && (buffer2[len-1] != '\n'))
			fprintf(fp_client, "\n");       // Append newline if missing
		if(cryptMode == TLS) {
			if (nvi == 3)
				fprintf(fp_client, "</tls-crypt>\n");
			else
				fprintf(fp_client, "</tls-auth>\n");

			if(nvi == 1)
				fprintf(fp_client, "key-direction 0\n");
			else if(nvi == 0)
				fprintf(fp_client, "key-direction 1\n");
		}
		else if(cryptMode == SECRET)
			fprintf(fp_client, "</secret>\n");
	}

	vpnlog(VPN_LOG_EXTRA,"Done writing certs/keys");
	nvram_commit();

	fprintf(fp_client, "resolv-retry infinite\n");
	fprintf(fp_client, "nobind\n");
	fclose(fp_client);

	// Format client file so Windows Notepad can edit it
	sprintf(buffer, "/etc/openvpn/server%d/client.ovpn", serverNum);
	eval("/usr/bin/unix2dos", buffer);
	vpnlog(VPN_LOG_EXTRA,"Done writing client config file");

	// Run postconf custom script on it if it exists
	sprintf(buffer, "openvpnserver%d", serverNum);
	sprintf(buffer2, "/etc/openvpn/server%d/config.ovpn", serverNum);
	run_postconf(buffer, buffer2);

	// Handle firewall rules if appropriate
	if ( !nvram_pf_match(prefix, "firewall", "custom") )
	{
		// Create firewall rules
		vpnlog(VPN_LOG_EXTRA,"Creating firewall rules");
		mkdir("/etc/openvpn/fw", 0700);
		sprintf(buffer, "/etc/openvpn/fw/server%d-fw.sh", serverNum);
		fp = fopen(buffer, "w");
		chmod(buffer, S_IRUSR|S_IWUSR|S_IXUSR);
		fprintf(fp, "#!/bin/sh\n");
		strlcpy(buffer, nvram_pf_safe_get(prefix, "proto"), sizeof (buffer));
		fprintf(fp, "iptables -t nat -I PREROUTING -p %s ", strtok(buffer, "-"));
		fprintf(fp, "--dport %d -j ACCEPT\n", nvram_pf_get_int(prefix, "port"));
		strlcpy(buffer, nvram_pf_safe_get(prefix, "proto"), sizeof (buffer));
		fprintf(fp, "iptables -I INPUT -p %s ", strtok(buffer, "-"));
		fprintf(fp, "--dport %d -j ACCEPT\n", nvram_pf_get_int(prefix, "port"));
		if ( !nvram_pf_match(prefix, "firewall", "external") )
		{
			fprintf(fp, "iptables -I OVPN -i %s -j ACCEPT\n", iface);
#ifdef HND_ROUTER
			if (nvram_match("fc_disable", "0")) {
#else
			if (nvram_match("ctf_disable", "0")) {
#endif
				fprintf(fp, "iptables -t mangle -I PREROUTING -i %s -j MARK --set-mark 0x01/0x7\n", iface);
			}
		}
#if !defined(HND_ROUTER)
		if (nvram_match("cstats_enable", "1")) {
			ipt_account(fp, iface);
		}
#endif
		fclose(fp);
		vpnlog(VPN_LOG_EXTRA,"Done creating firewall rules");

		// Run the firewall rules
		vpnlog(VPN_LOG_EXTRA,"Running firewall rules");
		sprintf(buffer, "/etc/openvpn/fw/server%d-fw.sh", serverNum);
		argv[0] = buffer;
		argv[1] = NULL;
		_eval(argv, NULL, 0, NULL);
		vpnlog(VPN_LOG_EXTRA,"Done running firewall rules");
	}

	// Start the VPN server
	sprintf(buffer, "/etc/openvpn/vpnserver%d", serverNum);
	sprintf(buffer2, "/etc/openvpn/server%d", serverNum);

	taskset_ret = cpu_eval(NULL, (serverNum == 1 ? CPU1 : CPU0), buffer, "--cd", buffer2, "--config", "config.ovpn");

	vpnlog(VPN_LOG_INFO,"Starting OpenVPN server %d", serverNum);
	if (taskset_ret)
	{
		vpnlog(VPN_LOG_ERROR,"Starting VPN instance failed...");
		stop_ovpn_server(serverNum);
		return;
	}
	vpnlog(VPN_LOG_EXTRA,"Done starting openvpn");

	// Set up cron job
	if ( (nvi = nvram_pf_get_int(prefix, "poll")) > 0 )
	{
		vpnlog(VPN_LOG_EXTRA,"Adding cron job");
		argv[0] = "cru";
		argv[1] = "a";
		sprintf(buffer, "CheckVPNServer%d", serverNum);
		argv[2] = buffer;
		sprintf(&buffer[strlen(buffer)+1], "*/%d * * * * service start_vpnserver%d", nvi, serverNum);
		argv[3] = &buffer[strlen(buffer)+1];
		argv[4] = NULL;
		_eval(argv, NULL, 0, NULL);
		vpnlog(VPN_LOG_EXTRA,"Done adding cron job");
	}

	if ( cryptMode == SECRET )
	{
		nvram_pf_set(prefix, "state", "2");	//running
		nvram_pf_set(prefix, "errno", "0");
	}

	vpnlog(VPN_LOG_INFO,"VPN GUI server backend complete.");
}

void stop_ovpn_server(int serverNum)
{
	int argc;
	char *argv[9];
	char buffer[256];

	sprintf(buffer, "stop_vpnserver%d", serverNum);
	if (getpid() != 1) {
		notify_rc(buffer);
		return;
	}

	vpnlog(VPN_LOG_INFO,"Stopping VPN GUI server backend.");

	// Remove cron job
	vpnlog(VPN_LOG_EXTRA,"Removing cron job");
	argv[0] = "cru";
	argv[1] = "d";
	sprintf(buffer, "CheckVPNServer%d", serverNum);
	argv[2] = buffer;
	argv[3] = NULL;
	_eval(argv, NULL, 0, NULL);
	vpnlog(VPN_LOG_EXTRA,"Done removing cron job");

	// Stop the VPN server
	vpnlog(VPN_LOG_EXTRA,"Stopping OpenVPN server.");
	sprintf(buffer, "vpnserver%d", serverNum);
	if ( !ovpn_waitfor(buffer) )
		vpnlog(VPN_LOG_EXTRA,"OpenVPN server stopped.");

	// NVRAM setting for device type could have changed, just try to remove both
	vpnlog(VPN_LOG_EXTRA,"Removing VPN device.");
	sprintf(buffer, "openvpn --rmtun --dev tap%d", serverNum+SERVER_IF_START);
	for (argv[argc=0] = strtok(buffer, " "); argv[argc] != NULL; argv[++argc] = strtok(NULL, " "));
	_eval(argv, NULL, 0, NULL);

	sprintf(buffer, "openvpn --rmtun --dev tun%d", serverNum+SERVER_IF_START);
	for (argv[argc=0] = strtok(buffer, " "); argv[argc] != NULL; argv[++argc] = strtok(NULL, " "));
	_eval(argv, NULL, 0, NULL);
	vpnlog(VPN_LOG_EXTRA,"VPN device removed.");

	// Remove firewall rules after VPN exit
	vpnlog(VPN_LOG_EXTRA,"Removing firewall rules.");
	sprintf(buffer, "/etc/openvpn/fw/server%d-fw.sh", serverNum);
	argv[0] = "sed";
	argv[1] = "-i";
	argv[2] = "s/-A/-D/g;s/-I/-D/g";
	argv[3] = buffer;
	argv[4] = NULL;
	if (!_eval(argv, NULL, 0, NULL))
	{
		argv[0] = buffer;
		argv[1] = NULL;
		_eval(argv, NULL, 0, NULL);
	}
	vpnlog(VPN_LOG_EXTRA,"Done removing firewall rules.");

	if ( nvram_get_int("vpn_debug") <= VPN_LOG_EXTRA )
	{
		vpnlog(VPN_LOG_EXTRA,"Removing generated files.");
		// Delete all files for this server
		sprintf(buffer, "rm -rf /etc/openvpn/server%d /etc/openvpn/fw/server%d-fw.sh /etc/openvpn/vpnserver%d",serverNum,serverNum,serverNum);
		for (argv[argc=0] = strtok(buffer, " "); argv[argc] != NULL; argv[++argc] = strtok(NULL, " "));
		_eval(argv, NULL, 0, NULL);

		// Attempt to remove directories.  Will fail if not empty
		rmdir("/etc/openvpn/fw");
		rmdir("/etc/openvpn");
		vpnlog(VPN_LOG_EXTRA,"Done removing generated files.");
	}

	sprintf(buffer, "vpn_server%d_", serverNum);
	nvram_pf_set(buffer, "state", "0");
	nvram_pf_set(buffer, "errno", "0");

	vpnlog(VPN_LOG_INFO,"VPN GUI server backend stopped.");
}

void start_ovpn_eas()
{
	char buffer[16], *cur;
	int nums[OVPN_CLIENT_MAX], i;

	if (strlen(nvram_safe_get("vpn_serverx_start")) == 0 && strlen(nvram_safe_get("vpn_clientx_eas")) == 0) return;

	// wait for time sync for a while
	i = 0;
	while ((!nvram_get_int("ntp_ready")) && (i++ < 10)) {
		sleep(i*i);
	}

	// Parse and start servers
	strlcpy(buffer, nvram_safe_get("vpn_serverx_start"), sizeof(buffer));
	if ( strlen(buffer) != 0 ) vpnlog(VPN_LOG_INFO, "Starting OpenVPN servers (eas): %s", buffer);
	i = 0;
	for( cur = strtok(buffer,","); cur != NULL && i < OVPN_CLIENT_MAX; cur = strtok(NULL, ",")) { nums[i++] = atoi(cur); }
	if(i < OVPN_CLIENT_MAX) nums[i] = 0;
	for( i = 0; nums[i] > 0 && i < OVPN_CLIENT_MAX; i++ )
	{

		sprintf(buffer, "vpnserver%d", nums[i]);
		if ( pidof(buffer) >= 0 )
		{
			vpnlog(VPN_LOG_INFO, "Stopping OpenVPN server %d (eas)", nums[i]);
			stop_ovpn_server(nums[i]);
		}

		vpnlog(VPN_LOG_INFO, "Starting OpenVPN server %d (eas)", nums[i]);
		start_ovpn_server(nums[i]);
	}

	// Setup client routing in case some are set to be blocked when tunnel is down
	for( i = 1; i < 6; i++ ) {
		update_ovpn_routing(i);
	}

	// Parse and start clients
	strlcpy(buffer, nvram_safe_get("vpn_clientx_eas"), sizeof(buffer));
	if ( strlen(buffer) != 0 ) vpnlog(VPN_LOG_INFO, "Starting clients (eas): %s", buffer);
	i = 0;
	for( cur = strtok(buffer,","); cur != NULL && i < OVPN_CLIENT_MAX; cur = strtok(NULL, ",")) { nums[i++] = atoi(cur); }
	if(i < OVPN_CLIENT_MAX) nums[i] = 0;
	for( i = 0; nums[i] > 0 && i < OVPN_CLIENT_MAX; i++ )
	{
		sprintf(buffer, "vpnclient%d", nums[i]);
		if ( pidof(buffer) >= 0 )
		{
			vpnlog(VPN_LOG_INFO, "Stopping OpenVPN client %d (eas)", nums[i]);
			stop_ovpn_client(nums[i]);
		}

		vpnlog(VPN_LOG_INFO, "Starting OpenVPN client %d (eas)", nums[i]);
		start_ovpn_client(nums[i]);

	}
}

void stop_ovpn_eas()
{
	char buffer[16], *cur;
	int nums[6], i;

	// Parse and stop servers
	strlcpy(buffer, nvram_safe_get("vpn_serverx_start"), sizeof(buffer));
	if ( strlen(buffer) != 0 ) vpnlog(VPN_LOG_INFO, "Stopping OpenVPN servers (eas): %s", buffer);
	i = 0;
	for( cur = strtok(buffer,","); cur != NULL && i <= OVPN_SERVER_MAX; cur = strtok(NULL, ",")) { nums[i++] = atoi(cur); }
	nums[i] = 0;
	for( i = 0; nums[i] > 0; i++ )
	{
		sprintf(buffer, "vpnserver%d", nums[i]);
		if ( pidof(buffer) >= 0 )
		{
			vpnlog(VPN_LOG_INFO, "Stopping OpenVPN server %d (eas)", nums[i]);
			stop_ovpn_server(nums[i]);
		}
	}

	// Parse and stop clients
	strlcpy(buffer, nvram_safe_get("vpn_clientx_eas"), sizeof(buffer));
	if ( strlen(buffer) != 0 ) vpnlog(VPN_LOG_INFO, "Stopping OpenVPN clients (eas): %s", buffer);
	i = 0;
	for( cur = strtok(buffer,","); cur != NULL && i <= OVPN_CLIENT_MAX; cur = strtok(NULL, ",")) { nums[i++] = atoi(cur); }
	nums[i] = 0;
	for( i = 0; nums[i] > 0; i++ )
	{
		sprintf(buffer, "vpnclient%d", nums[i]);
		if ( pidof(buffer) >= 0 )
		{
			vpnlog(VPN_LOG_INFO, "Stopping OpenVPN client %d (eas)", nums[i]);
			stop_ovpn_client(nums[i]);
		}
	}
}

void stop_ovpn_all()
{
	char buffer[16];
	int i;

	// stop servers
	for( i = 1; i <= OVPN_SERVER_MAX; i++ )
	{
		sprintf(buffer, "vpnserver%d", i);
		if ( pidof(buffer) >= 0 )
		{
			vpnlog(VPN_LOG_INFO, "Stopping OpenVPN server %d", i);
			stop_ovpn_server(i);
		}
	}

	// stop clients
	for( i = 1; i <= OVPN_CLIENT_MAX; i++ )
	{
		sprintf(buffer, "vpnclient%d", i);
		if ( pidof(buffer) >= 0 )
		{
			vpnlog(VPN_LOG_INFO, "Stopping OpenVPN client %d", i);
			stop_ovpn_client(i);
		}
	}

	// Remove tunnel interface module
	modprobe_r("tun");
}

void run_ovpn_fw_script()
{
	DIR *dir;
	struct dirent *file;
	char *fn;
	char *argv[3];

	if ( chdir("/etc/openvpn/fw") )
		return;

	dir = opendir("/etc/openvpn/fw");

	vpnlog(VPN_LOG_EXTRA,"Beginning all firewall scripts...");
	while ( (file = readdir(dir)) != NULL )
	{
		fn = file->d_name;
		if ( fn[0] == '.' )
			continue;
		vpnlog(VPN_LOG_INFO,"Running firewall script: %s", fn);
		argv[0] = "/bin/sh";
		argv[1] = fn;
		argv[2] = NULL;
		_eval(argv, NULL, 0, NULL);
	}
	vpnlog(VPN_LOG_EXTRA,"Done with all firewall scripts...");

	closedir(dir);
}

void write_ovpn_dnsmasq_config(FILE* f)
{
	char nv[16];
	char buf[24];
	char *pos, ch;
	int cur, ch2;	DIR *dir;
	struct dirent *file;
	FILE *dnsf;

	strlcpy(buf, nvram_safe_get("vpn_serverx_dns"), sizeof(buf));
	for ( pos = strtok(buf,","); pos != NULL; pos=strtok(NULL, ",") )
	{
		cur = atoi(pos);
		if ( cur )
		{
			vpnlog(VPN_LOG_EXTRA, "Adding server %d interface to dns config", cur);
			snprintf(nv, sizeof(nv), "vpn_server%d_if", cur);
			fprintf(f, "interface=%s%d\n", nvram_safe_get(nv), SERVER_IF_START + cur);
		}
	}

	if ( (dir = opendir("/etc/openvpn/dns")) != NULL )
	{
		while ( (file = readdir(dir)) != NULL )
		{
			if ( file->d_name[0] == '.' )
				continue;

			if ( sscanf(file->d_name, "client%d.resol%c", &cur, &ch) == 2 )
			{
				vpnlog(VPN_LOG_EXTRA, "Checking ADNS settings for client %d", cur);
				snprintf(buf, sizeof(buf), "vpn_client%d_adns", cur);
				if ( nvram_get_int(buf) == 2 )
				{
					vpnlog(VPN_LOG_INFO, "Adding strict-order to dnsmasq config for client %d", cur);
					fprintf(f, "strict-order\n");
					break;
				}
			}

			if ( sscanf(file->d_name, "client%d.con%c", &cur, &ch) == 2 )
			{
				if ( (dnsf = fopen(file->d_name, "r")) != NULL )
				{
					vpnlog(VPN_LOG_INFO, "Adding Dnsmasq config from %s", file->d_name);

					while( !feof(dnsf) )
					{
						ch2 = fgetc(dnsf);
						fputc(ch2==EOF?'\n':ch2, f);
					}

					fclose(dnsf);
				}
			}
		}
	}
}

int write_ovpn_resolv(FILE* f)
{
	DIR *dir;
	struct dirent *file;
	char *fn, ch, num, buf[24];
	FILE *dnsf;
	int strictlevel = 0, ch2, level;

	if ( chdir("/etc/openvpn/dns") )
		return 0;

	dir = opendir("/etc/openvpn/dns");

	vpnlog(VPN_LOG_EXTRA, "Adding DNS entries...");
	while ( (file = readdir(dir)) != NULL )
	{
		fn = file->d_name;

		if ( fn[0] == '.' )
			continue;

		if ( sscanf(fn, "client%c.resol%c", &num, &ch) == 2 )
		{
			snprintf(buf, sizeof(buf), "vpn_client%c_", num);

			level = nvram_pf_get_int(buf, "adns");

			// Don't modify dnsmasq if policy routing is enabled and dns mode set to "Exclusive"
			if ((nvram_pf_get_int(buf, "rgw") >= 2 ) && (level == 3))
				continue;

			if ( (dnsf = fopen(fn, "r")) == NULL )
				continue;

			vpnlog(VPN_LOG_INFO,"Adding DNS entries from %s", fn);

			while( !feof(dnsf) )
			{
				ch2 = fgetc(dnsf);
				fputc(ch2==EOF?'\n':ch2, f);
			}

			fclose(dnsf);

			// Only return the highest active level, so one exclusive client
			// will override a relaxed client.
			if (level > strictlevel)
				strictlevel = level;
		}
	}
	vpnlog(VPN_LOG_EXTRA, "Done with DNS entries...");

	closedir(dir);

	return strictlevel;
}

void create_ovpn_passwd()
{
	unsigned char s[512];
	char *p;
	char salt[32];
	char *nv, *nvp, *b;
	char *username, *passwd;
	FILE *fp1, *fp2, *fp3;
	int id = 200;
#ifdef RTCONFIG_NVRAM_ENCRYPT
	char dec_passwd[256];
#endif

	strcpy(salt, "$1$");
	f_read("/dev/urandom", s, 6);
	base64_encode(s, salt + 3, 6);
	salt[3 + 8] = 0;
	p = salt;
	while (*p) {
		if (*p == '+') *p = '.';
		++p;
	}

	fp1=fopen("/etc/shadow.openvpn", "w");
	fp2=fopen("/etc/passwd.openvpn", "w");
	fp3=fopen("/etc/group.openvpn", "w");
	if (!fp1 || !fp2 || !fp3) return;

	nv = nvp = strdup(nvram_safe_get("vpn_serverx_clientlist"));

	if(nv) {
		while ((b = strsep(&nvp, "<")) != NULL) {
			if((vstrsep(b, ">", &username, &passwd)!=2)) continue;
			if(strlen(username)==0||strlen(passwd)==0) continue;

#ifdef RTCONFIG_NVRAM_ENCRYPT
			memset(dec_passwd, 0, sizeof(dec_passwd));
			pw_dec(passwd, dec_passwd);
			passwd = dec_passwd;
#endif
			p = crypt(passwd, salt);
			fprintf(fp1, "%s:%s:0:0:99999:7:0:0:\n", username, p);
			fprintf(fp2, "%s:x:%d:%d:::\n", username, id, id);
			fprintf(fp3, "%s:x:%d:\n", username, id);
			id++;
		}
		free(nv);
	}
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
}


void update_ovpn_profie_remote()
{
	char file_path[128];
	char address[64];
	char buffer[256], *cur;
	int nums[OVPN_CLIENT_MAX], i;

	strlcpy(buffer, nvram_safe_get("vpn_serverx_eas"), sizeof(buffer));

	i = 0;
	for( cur = strtok(buffer,","); cur != NULL && i < OVPN_CLIENT_MAX; cur = strtok(NULL, ",")) { nums[i++] = atoi(cur); }
	if(i < OVPN_CLIENT_MAX) nums[i] = 0;
	for( i = 0; nums[i] > 0 && i < OVPN_CLIENT_MAX; i++ )
	{
		if(!nvram_get_int("VPNServer_enable")) continue;

		snprintf(file_path, sizeof(file_path), "/etc/openvpn/server%d/client.ovpn", nums[i]);
		if(f_exists(file_path) && f_size(file_path) > 0)
		{
			if( nvram_match("ddns_enable_x", "1") &&
			    nvram_match("ddns_status", "1"))
			{
				strlcpy(address, nvram_safe_get("ddns_hostname_x"), sizeof(address));
			} else {
				strlcpy(address, nvram_safe_get("wan0_ipaddr"), sizeof(address));
			}
			snprintf(buffer, sizeof(buffer), "sed -i 's/remote [A-Za-z0-9.-]*/remote %s/ ' %s", address, file_path);
			system(buffer);
		}
	}
}
