#include <rc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <shared.h>
#ifdef RTCONFIG_VPN_FUSION
#include "vpnc_fusion.h"
#endif
#ifdef RTCONFIG_MULTIWAN_IF
#include "multi_wan.h"
#endif
#if defined(RTCONFIG_DNSFILTER)
#include "dnsfilter.h"
#endif

extern const char dmservers[];

const char sdn_dir[] = "/tmp/.sdn";
static int _handle_sdn_stubby(const MTLAN_T *pmtl, const int action);
static int _handle_sdn_dnsmasq(const MTLAN_T *pmtl, const int action);
static int _handle_sdn_wan(const MTLAN_T *pmtl, const char *logdrop, const char *logaccept);
static int _remove_sdn_routing_table(const MTLAN_T *pmtl);
static int _remove_sdn_routing_rule(const MTLAN_T *pmtl, int v6);
static int _handle_sdn_routing(const MTLAN_T *pmtl);

int handle_sdn_feature(const int sdn_idx, const unsigned long features, const int action)
{
	FILE *fp_filter = NULL, *fp_nat = NULL, *fp_mangle = NULL;
	char file_filter[128], file_nat[128], file_mangle[128];

#ifdef RTCONFIG_IPV6
	FILE *fpv6_filter = NULL, *fpv6_nat = NULL, *fpv6_mangle = NULL;
	char filev6_filter[128], filev6_nat[128], filev6_mangle[128];
#endif

	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i;
	char logaccept[32], logdrop[32];
	int restart_all_sdn;

	//_dprintf("[%s, %d]<%d, %x, %d>\n", __FUNCTION__, __LINE__, sdn_idx, features, action);

	// check dir exist
	if (!d_exists(sdn_dir))
	{
		mkdir(sdn_dir, 0777);
	}

	get_drop_accept(logdrop, sizeof(logdrop), logaccept, sizeof(logaccept));

	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl)
	{
		// feature active by independent sdn
		if (sdn_idx == ALL_SDN)
		{
			get_mtlan(pmtl, &mtl_sz);
			restart_all_sdn = 1;
		}
		else
		{
			get_mtlan_by_idx(SDNFT_TYPE_SDN, sdn_idx, pmtl, &mtl_sz);
			restart_all_sdn = 0;
		}

		for (i = 0; i < mtl_sz; ++i)
		{
			if (features & SDN_FEATURE_ALL_FIREWALL)
			{
				reset_sdn_firewall(features, pmtl[i].sdn_t.sdn_idx);
				create_iptables_file(IPTABLE_TYPE_FILTER, pmtl[i].sdn_t.sdn_idx, &fp_filter, file_filter, sizeof(file_filter), 0);
				create_iptables_file(IPTABLE_TYPE_NAT, pmtl[i].sdn_t.sdn_idx, &fp_nat, file_nat, sizeof(file_nat), 0);
				create_iptables_file(IPTABLE_TYPE_MANGLE, pmtl[i].sdn_t.sdn_idx, &fp_mangle, file_mangle, sizeof(file_mangle), 0);
#ifdef RTCONFIG_IPV6
				if (ipv6_enabled())
				{
					create_iptables_file(IPTABLE_TYPE_FILTER, pmtl[i].sdn_t.sdn_idx, &fpv6_filter, filev6_filter, sizeof(filev6_filter), 1);
					create_iptables_file(IPTABLE_TYPE_NAT, pmtl[i].sdn_t.sdn_idx, &fpv6_nat, filev6_nat, sizeof(filev6_nat), 1);
					create_iptables_file(IPTABLE_TYPE_MANGLE, pmtl[i].sdn_t.sdn_idx, &fpv6_mangle, filev6_mangle, sizeof(filev6_mangle), 1);
				}
#endif
			}

			if (features & SDN_FEATURE_WAN)
			{
				_dprintf("[%s][%d]DO: SDN WAN\n", __FUNCTION__, pmtl[i].sdn_t.sdn_idx);
				_handle_sdn_wan(&pmtl[i], logdrop, logaccept);
				if (pmtl[i].sdn_t.sdn_idx == 0)	//LAN
					update_resolvconf();
#ifdef RTCONFIG_MULTIWAN_PROFILE
				update_sdn_mtwan_iptables(&pmtl[i]);
				update_sdn_resolvconf();
#endif
			}
			if(features & SDN_FEATURE_ROUTING)
			{
				_dprintf("[%s][%d]DO: SDN ROUTING\n", __FUNCTION__, pmtl[i].sdn_t.sdn_idx);
				_handle_sdn_routing(&pmtl[i]);
			}
			if (features & SDN_FEATURE_DNSMASQ)
			{
				if (pmtl[i].sdn_t.sdn_idx) // ignore  main LAN
				{
					_dprintf("[%s][%d]DO: SDN DNSMASQ\n", __FUNCTION__, pmtl[i].sdn_t.sdn_idx);
					_handle_sdn_dnsmasq(&pmtl[i], action);
				}
			}
			if (features & SDN_FEATURE_DNSPRIV)
			{
				_dprintf("[%s][%d]DO: SDN DNSPRIV(stubby)\n", __FUNCTION__, pmtl[i].sdn_t.sdn_idx);
				_handle_sdn_stubby(&pmtl[i], action);
			}
			if (features & SDN_FEATURE_SDN_IPTABLES)
			{
				if (pmtl[i].sdn_t.sdn_idx) // ignore  main LAN
				{
					_dprintf("[%s][%d]DO: SDN_FEATURE_SDN_IPTABLES\n", __FUNCTION__, pmtl[i].sdn_t.sdn_idx);
					update_SDN_iptables(&pmtl[i], logdrop, logaccept);
				}
			}
			if (features & SDN_FEATURE_URL_FILTER)
			{
				_dprintf("[%s][%d]DO: URL FILTER\n", __FUNCTION__, pmtl[i].sdn_t.sdn_idx);
#ifdef RTCONFIG_IPV6
				write_URLFilter_SDN(&pmtl[i], logdrop, fp_filter, fpv6_filter);
#else
				write_URLFilter_SDN(&pmtl[i], logdrop, fp_filter);
#endif
			}
			if (features & SDN_FEATURE_NWS_FILTER)
			{
				_dprintf("[%s][%d]DO: NETWORK SERVICE  FILTER\n", __FUNCTION__, pmtl[i].sdn_t.sdn_idx);
#ifdef RTCONFIG_IPV6
				write_NwServiceFilter_SDN(&pmtl[i], logdrop, logaccept, fp_filter, fpv6_filter);
#else
				write_NwServiceFilter_SDN(&pmtl[i], logdrop, logaccept, fp_filter);
#endif
			}
			if (features & SDN_FEATURE_FIREWALL)
			{
			}
			if (features & SDN_FEATURE_VPNC)
			{
				// set routing rule
				update_sdn_by_vpnc(pmtl[i].sdn_t.vpnc_idx);
			}
			if (features & SDN_FEATURE_ALL_FIREWALL)
			{
				close_n_restore_iptables_file(IPTABLE_TYPE_FILTER, &fp_filter, file_filter, 0);
				close_n_restore_iptables_file(IPTABLE_TYPE_NAT, &fp_nat, file_nat, 0);
				close_n_restore_iptables_file(IPTABLE_TYPE_MANGLE, &fp_mangle, file_mangle, 0);
#ifdef RTCONFIG_IPV6
				if (ipv6_enabled())
				{
					close_n_restore_iptables_file(IPTABLE_TYPE_FILTER, &fpv6_filter, filev6_filter, 1);
					close_n_restore_iptables_file(IPTABLE_TYPE_NAT, &fpv6_nat, filev6_nat, 1);
					close_n_restore_iptables_file(IPTABLE_TYPE_MANGLE, &fpv6_mangle, filev6_mangle, 1);
				}
#endif
				// Handle jump rule here.
				if (features & SDN_FEATURE_URL_FILTER)
				{
					handle_URLFilter_jump_rule(&pmtl[i]);
				}
				if (features & SDN_FEATURE_NWS_FILTER)
				{
					handle_NwServiceFilter_jump_rule(&pmtl[i]);
				}
			}
		}

		// common feature, not for a specific SDN
		if (features & SDN_FEATURE_SDN_INTERNAL_ACCESS)
		{
			handle_SDN_internal_access(logdrop, logaccept);
		}
		// some vpn handled by script
		if (features & SDN_FEATURE_VPNC)
		{
			_dprintf("[%s]DO: VPNC\n", __FUNCTION__);
#ifdef RTCONFIG_VPN_FUSION
			vpnc_set_iptables_rule_by_sdn(pmtl, mtl_sz, restart_all_sdn);
#endif
#ifdef RTCONFIG_OPENVPN
			update_ovpn_client_by_sdn(pmtl, mtl_sz, restart_all_sdn);
#endif
#ifdef RTCONFIG_WIREGUARD
			update_wgc_by_sdn(pmtl, mtl_sz, restart_all_sdn);
#endif
		}
		if (features & SDN_FEATURE_VPNS)
		{
			_dprintf("[%s]DO: VPNS\n", __FUNCTION__);
#ifdef RTCONFIG_WIREGUARD
			update_wgs_by_sdn(pmtl, mtl_sz, restart_all_sdn);
#endif
#ifdef RTCONFIG_IPSEC
			update_ipsec_server_by_sdn(pmtl, mtl_sz, restart_all_sdn);
#endif
#ifdef RTCONFIG_OPENVPN
			update_ovpn_server_by_sdn(pmtl, mtl_sz, restart_all_sdn);
#endif
		}
#ifdef RTCONFIG_GRE
		if (features & SDN_FEATURE_GRE)
		{
			_dprintf("[%s]DO: GRE\n", __FUNCTION__);
			update_gre_by_sdn(pmtl, mtl_sz, restart_all_sdn);
		}
#endif

		FREE_MTLAN((void *)pmtl);
		return 0;
	}
	return -1;
}

static int _write_dhcp_reservation(FILE *fp, const int dhcpres_idx)
{
	char *nv, *nvp, *b;
	char *mac, *ip, *dns, *hostname;
	char name[32];

	if (!fp)
		return -1;

	// get matched dhcpresX_rl
	snprintf(name, sizeof(name), "dhcpres%d_rl", dhcpres_idx);
	nv = nvp = strdup(nvram_safe_get(name));

	// parser the rule list
	while (nv && (b = strsep(&nvp, "<")) != NULL)
	{
		if (vstrsep(b, ">", &mac, &ip, &dns, &hostname) < 2) // mac and ip are mandatory
			continue;

		if (!mac)
			continue;

		// write dns
		if (dns)
		{
			struct in_addr in4;
#ifdef RTCONFIG_IPV6
			struct in6_addr in6;

			if (*dns && inet_pton(AF_INET6, dns, &in6) > 0 &&
				!IN6_IS_ADDR_UNSPECIFIED(&in6) && !IN6_IS_ADDR_LOOPBACK(&in6))
			{
				fprintf(fp, "dhcp-option=tag:%s,option6:23,%s\n", mac, dns);
			}
			else
#endif
				if (*dns && inet_pton(AF_INET, dns, &in4) > 0 &&
					in4.s_addr != INADDR_ANY && in4.s_addr != INADDR_LOOPBACK && in4.s_addr != INADDR_BROADCAST)
			{
				fprintf(fp, "dhcp-option=tag:%s,6,%s\n", mac, dns);
			}
			else
				dns = NULL;
		}

		// write dhcp static lease
		if (*ip == '\0')
		{
			if (dns)
				fprintf(fp, "dhcp-host=%s,set:%s\n", mac, mac);
			continue;
		}

		// if ((sdn_ipaddr & sdn_netmask_addr) == sdn_net_addr)
		{
			if (hostname && hostname[0] != '\0')
				fprintf(fp, "dhcp-host=%s,set:%s,%s,%s\n", mac, mac, hostname, ip);
			else
				fprintf(fp, "dhcp-host=%s,set:%s,%s\n", mac, mac, ip);
			continue;
		}
	}
	if (nv)
		free(nv);

	return 0;
}

#ifdef RTCONFIG_DNSPRIVACY
void _start_sdn_stubby(const MTLAN_T *pmtl, char *config_file, const size_t path_len)
{
	char stubby_log[64] = "";
	char *stubby_argv[] = { "/usr/sbin/stubby",
		"-C", config_file,
		NULL,				/* -l */
		NULL
	};
	int index = 4;
	FILE *fp;
	char *nv, *nvp, *b;
	char *server, *tlsport, *hostname, *spkipin, *digest;
	int tls_required, tls_possible, max_queries, port;
	char server_addr[INET6_ADDRSTRLEN ];
	size_t server_len;
	char buf[64] = {0};

	union {
		struct in_addr addr4;
#ifdef RTCONFIG_IPV6
		struct in6_addr addr6;
#endif
	} addr;

	TRACE_PT("begin\n");

	if (!pmtl || !config_file)
		return;

	if (getpid() != 1) {
		notify_rc("start_stubby");
		return;
	}

	/* Check if enabled */
	if (!pmtl->nw_t.dot_enable || !is_routing_enabled()) { //dnspriv_enable
		_dprintf("%s: DoT_%d not enable, return.\n", __FUNCTION__, pmtl->nw_t.idx);
		return;
	}

	//stop_stubby(pmtl->sdn_t.sdn_idx);
	snprintf(buf, sizeof(buf), sdn_stubby_pid_path, pmtl->nw_t.idx);
	kill_pidfile_tk(buf);
	unlink(buf);

	mkdir_if_none("/etc/stubby");

	// create /etc/stubby/stubby-<subnet_idx>.yml
	snprintf(config_file, path_len, "/etc/stubby/stubby-%d.yml", pmtl->nw_t.idx);
	unlink(config_file);
	if ((fp = fopen(config_file, "w")) == NULL) {
		_dprintf("%s: cannot create %s.\n", __FUNCTION__, config_file);
		return;
	}

	tls_required = pmtl->nw_t.dot_tls; //dnspriv_profile
	tls_possible = nvram_get_int("ntp_ready");

	/* Basic & privacy settings */
	fprintf(fp,
		"resolution_type: GETDNS_RESOLUTION_STUB\n"
		"dns_transport_list:\n"
		"%s%s"
		"tls_authentication: %s\n"
		"tls_query_padding_blocksize: 128\n"
		"tls_ca_file: \"/etc/ssl/certs/ca-certificates.crt\"\n"
		"appdata_dir: \"/var/lib/misc\"\n"
		"resolvconf: \"%s\"\n"
		"edns_client_subnet_private: 1\n",
		tls_possible ?
			"  - GETDNS_TRANSPORT_TLS\n" : "",
		tls_required && tls_possible ? "" :
			"  - GETDNS_TRANSPORT_UDP\n"
			"  - GETDNS_TRANSPORT_TCP\n",
		tls_required && tls_possible ?
			"GETDNS_AUTHENTICATION_REQUIRED" : "GETDNS_AUTHENTICATION_NONE",
		"/tmp/resolv.conf");

#ifdef RTCONFIG_DNSSEC
	/* DNSSEC settings */
	if (nvram_get_int("dnssec_enable") == 2 && tls_possible) {
		fprintf(fp,
			"dnssec_return_status: GETDNS_EXTENSION_TRUE\n");
	}
#endif

	/* Connection settings */
	fprintf(fp,
		"round_robin_upstreams: 1\n"
		"idle_timeout: 9000\n"
		"tls_connection_retries: 2\n"
		"tls_backoff_time: 900\n"
		"timeout: 3000\n");

	/* Limit number of outstanding requests */
	max_queries = nvram_get_int("max_dns_queries");
#if defined(RTCONFIG_SOC_IPQ8064)
	if (max_queries == 0)
		max_queries = 1500;
#endif
	if (max_queries)
		fprintf(fp, "limit_outstanding_queries: %d\n", max(150, min(max_queries, 10000)));

	/* Listen address */
	if (pmtl->nw_t.idx) {
		fprintf(fp,
			"listen_addresses:\n"
			"  - %s@53%02d\n", pmtl->nw_t.addr, pmtl->nw_t.idx);
	} else { //br0
		fprintf(fp,
			"listen_addresses:\n"
			"  - 127.0.1.1@53\n");
	}

	/* Upstreams */
	fprintf(fp,
		"upstream_recursive_servers:\n");
	if (pmtl->nw_t.idx) {
		snprintf(buf, sizeof(buf), "dot%d_rl", pmtl->nw_t.idx);
	} else { //br0
		strlcpy(buf, "dnspriv_rulelist", sizeof(buf));
	}
	nv = nvp = strdup(nvram_safe_get(buf));
	while (nvp && (b = strsep(&nvp, "<")) != NULL) {
		server = tlsport = hostname = spkipin = NULL;

		/* <server>port>hostname>[digest:]spkipin */
		if ((vstrsep(b, ">", &server, &tlsport, &hostname, &spkipin)) < 4)
			continue;

		/* Check server, can be IPv4/IPv6 address */
		if (*server == '\0')
			continue;
		else if (inet_pton(AF_INET, server, &addr) <= 0
#ifdef RTCONFIG_IPV6
			&& (inet_pton(AF_INET6, server, &addr) <= 0 || !ipv6_enabled())
#endif
		)	continue;

		/* Check port, if specified */
		port = *tlsport ? atoi(tlsport) : 0;
		if (port < 0 || port > 65535)
			continue;

		//Workaround. stubby cannot accept the last character of IPv6 address with as ':'. It would be crashed.
		server_len = strlen(server);
		if(server_len > 1 && server[server_len - 1] == ':')
			snprintf(server_addr, sizeof(server_addr), "%s0", server);
		else
			strlcpy(server_addr, server, sizeof(server_addr));

		fprintf(fp, "  - address_data: %s\n", server_addr);
		if (port)
			fprintf(fp, "    tls_port: %d\n", port);
		if (*hostname)
			fprintf(fp, "    tls_auth_name: \"%s\"\n", hostname);
		if (*spkipin) {
			digest = strchr(spkipin, ':') ? strsep(&spkipin, ":") : "sha256";
			fprintf(fp, "    tls_pubkey_pinset:\n"
				    "      - digest: \"%s\"\n"
				    "        value: %s\n", digest, spkipin);
		}
	}
	if (nv)
		free(nv);

	snprintf(buf, sizeof(buf), "stubby-%d.yml", pmtl->nw_t.idx);
	append_custom_config(buf, fp);
	fclose(fp);
	snprintf(buf, sizeof(buf), "%d", pmtl->sdn_t.sdn_idx);
	run_custom_script("stubby-sdn.postconf", 120, config_file, buf);
	chmod(config_file, 0644);

	if (nvram_get_int("stubby_debug")) {
		stubby_argv[index++] = "-l";
		snprintf(stubby_log, sizeof(stubby_log), ">/tmp/stubby-%d.log", pmtl->nw_t.idx);
	}

	pid_t pid;
	_eval(stubby_argv, stubby_log, 0, &pid);
	if (pid > 1) {
		// write pid in /var/run/stubby-<subnet_idx>.pid
		snprintf(buf, sizeof(buf), sdn_stubby_pid_path, pmtl->nw_t.idx);
		unlink(buf);
		FILE *fp = fopen(buf, "w");
		if (fp) {
			fprintf(fp, "%d", pid);
			fclose(fp);
		}
	}

	TRACE_PT("end\n");
}

static int _handle_sdn_stubby(const MTLAN_T *pmtl, const int action)
{
	char config_path[128] = "";

	if (!pmtl)
		return -1;

	if (action & RC_SERVICE_STOP)
	{
		snprintf(config_path, sizeof(config_path), sdn_stubby_pid_path, pmtl->nw_t.idx);
		kill_pidfile_tk(config_path);
		unlink(config_path);
	}

	if (action & RC_SERVICE_START)
	{
		if (pmtl->enable)
		{
			memset(config_path, 0, sizeof(config_path));
			_start_sdn_stubby(pmtl, config_path, sizeof(config_path));
		}
	}
	return 0;
}
#endif

static int _gen_sdn_dnsmasq_conf(const MTLAN_T *pmtl, char *config_file, const size_t path_len)
{
	FILE *fp;
	char resolv_path[64];
	int resolv_flag = 0, n;
#if defined(RTCONFIG_DNSFILTER)
	int count;
	dnsf_srv_entry_t dnsfsrv;
#endif
#ifdef RTCONFIG_IPV6
	int ipv6_service;
	int wan6_unit;
#endif
	char buf[32];

	if (!pmtl || !config_file)
		return -1;

	if(pmtl->sdn_t.vpnc_idx != 0)
	{
		snprintf(resolv_path, sizeof(resolv_path), vpnc_resolv_path, pmtl->sdn_t.vpnc_idx);
		if (!access(resolv_path, F_OK))
			resolv_flag = 1;
	}

#ifdef RTCONFIG_MULTIWAN_IF
	if(!resolv_flag)
	{
		resolv_flag = 1;
		update_sdn_resolvconf();
	}
#endif

	// create /etc/dnsmasq-<sdn_ifname>.conf
	snprintf(config_file, path_len, "/etc/dnsmasq-%d.conf", pmtl->sdn_t.sdn_idx);
	unlink(config_file);
	fp = fopen(config_file, "w");

	if (fp)
	{
		fprintf(fp, "pid-file=" sdn_dnsmasq_pid_path "\n", pmtl->sdn_t.sdn_idx);
		fprintf(fp, "user=nobody\n");
		fprintf(fp, "bind-interfaces\n");
		fprintf(fp, "listen-address=%s\n", pmtl->nw_t.addr);
		fprintf(fp, "no-resolv\n");
		fprintf(fp, "servers-file=%s\n", resolv_flag ? resolv_path : dmservers);
		fprintf(fp, "no-poll\n");
		fprintf(fp, "no-negcache\n");
		fprintf(fp, "cache-size=1500\n");
		fprintf(fp, "min-port=4096\n");

		if (pmtl->nw_t.dhcp_enable)
		{
			fprintf(fp, "dhcp-authoritative\n");
			fprintf(fp, "dhcp-range=%s,%s,%s,%s,%ds\n", pmtl->nw_t.ifname, pmtl->nw_t.dhcp_min, pmtl->nw_t.dhcp_max, pmtl->nw_t.netmask, pmtl->nw_t.dhcp_lease);
			fprintf(fp, "dhcp-option=%s,3,%s\n", pmtl->nw_t.ifname, pmtl->nw_t.addr);
			if (pmtl->nw_t.dns[0][0] != '\0' && pmtl->nw_t.dns[1][0] != '\0')
			{			
				fprintf(fp, "dhcp-option=%s,6,%s,%s\n", pmtl->nw_t.ifname, pmtl->nw_t.dns[0], pmtl->nw_t.dns[1]);
			}
			else if(pmtl->nw_t.dns[0][0] != '\0')
			{
				fprintf(fp, "dhcp-option=%s,6,%s\n", pmtl->nw_t.ifname, pmtl->nw_t.dns[0]);
			}
			else if(pmtl->nw_t.dns[1][0] != '\0')
			{
				fprintf(fp, "dhcp-option=%s,6,%s\n", pmtl->nw_t.ifname, pmtl->nw_t.dns[1]);
			}
		}
		else
		{
			fprintf(fp, "no-dhcp-interface=%s\n", pmtl->nw_t.ifname);
		}

		/* limit number of outstanding requests */
		{
			int max_queries = nvram_get_int("max_dns_queries");
#if defined(RTCONFIG_SOC_IPQ8064)
			if (max_queries == 0)
				max_queries = 1500;
#endif
			if (max_queries)
				fprintf(fp, "dns-forward-max=%d\n", max(150, min(max_queries, 10000)));
		}

		if (pmtl->nw_t.domain_name[0] != '\0')
		{
			fprintf(fp, "domain=%s\n"
						"expand-hosts\n",
					pmtl->nw_t.domain_name); // expand hostnames in hosts file
			fprintf(fp, "dhcp-option=%s,15,%s\n", pmtl->nw_t.ifname, pmtl->nw_t.domain_name);
		}

		if (pmtl->nw_t.wins[0] != '\0')
		{
			fprintf(fp, "dhcp-option=%s,44,%s\n", pmtl->nw_t.ifname, pmtl->nw_t.wins);
		}

		if (nvram_get_int("dns_fwd_local") != 1)
		{
			fprintf(fp, "bogus-priv\n"		// don't forward private reverse lookups upstream
						"domain-needed\n"); // don't forward plain name queries upstream
			if (pmtl->nw_t.domain_name[0] != '\0')
				fprintf(fp, "local=/%s/\n", pmtl->nw_t.domain_name); // don't forward local domain queries upstream
		}

		// write dhcp reservation
		if (pmtl->nw_t.dhcp_res)
		{
			_write_dhcp_reservation(fp, pmtl->nw_t.dhcp_res_idx);
		}

#ifdef RTCONFIG_DNSSEC
#ifdef RTCONFIG_DNSPRIVACY
		if (nvram_get_int("dnspriv_enable") && nvram_get_int("dnssec_enable") == 2)
		{
			fprintf(fp, "proxy-dnssec\n");
		}
		else
#endif
			if (nvram_get_int("dnssec_enable"))
		{
			fprintf(fp, "trust-anchor=.,20326,8,2,E06D44B80B8F1D39A95C0B0D7C65D08458E880409BBC683457104237C7F8EC8D\n"
						"dnssec\n");

			/* If NTP isn't set yet, wait until rc's ntp signals us to start validating time */
			if (!nvram_get_int("ntp_ready"))
			{
				fprintf(fp, "dnssec-no-timecheck\n");
			}

			if (nvram_match("dnssec_check_unsigned_x", "0"))
			{
				fprintf(fp, "dnssec-check-unsigned=no\n");
			}
		}
#endif
		if (nvram_match("dns_norebind", "1"))
		{
			fprintf(fp, "stop-dns-rebind\n");
			fprintf(fp, "rebind-domain-ok=dns.msftncsi.com\n");
		}

		/* Instruct clients like Firefox to not auto-enable DoH */
		n = nvram_get_int("dns_priv_override");
		if ((n == 1) ||
			(n == 0 && (
#ifdef RTCONFIG_DNSPRIVACY
						   nvram_get_int("dnspriv_enable") ||
#endif
						   (nvram_get_int("dnsfilter_enable_x") && nvram_get_int("dnsfilter_mode"))) // DNSFilter enabled in Global mode
			 ))
		{
			fprintf(fp, "address=/use-application-dns.net/\n");
			fprintf(fp, "address=/_dns.resolver.arpa/\n");
			fprintf(fp, "address=/mask.icloud.com/mask-h2.icloud.com/\n");
		}

		/* Protect against VU#598349 */
		fprintf(fp, "dhcp-name-match=set:wpad-ignore,wpad\n");
		fprintf(fp, "dhcp-ignore-names=tag:wpad-ignore\n");

		/* dhcp-script */
		fprintf(fp, "dhcp-script=/sbin/dhcpc_lease\n");
		/* dhcp-lease */
		fprintf(fp, "dhcp-leasefile=/var/lib/misc/dnsmasq-%d.leases\n", pmtl->sdn_t.sdn_idx);

#if defined(RTCONFIG_AMAS)
		fprintf(fp, "script-arp\n");
#endif

#if defined(RTCONFIG_DNSFILTER) && defined(RTCONFIG_IPV6)
		if (pmtl->sdn_t.dnsf_idx != DNSF_SRV_UNFILTERED)
		{
			count = get_dns_filter(AF_INET6, pmtl->sdn_t.dnsf_idx, &dnsfsrv);
			if (count > 0)
			{
				fprintf(fp, "dhcp-option=lan,option6:23,[%s]", dnsfsrv.server1);
				if (count > 1)
				{
					fprintf(fp, ",[%s]", dnsfsrv.server2);
				}
				fprintf(fp, "\n");
			}
		}
#endif

#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_MULTIWAN_IF
		wan6_unit = mtwan_get_mapped_unit(pmtl->sdn_t.wan6_idx) ? : wan_primary_ifunit_ipv6();
#else
		wan6_unit = wan_primary_ifunit_ipv6();
#endif
		ipv6_service = get_ipv6_service_by_unit(wan6_unit);
		if (ipv6_service != IPV6_DISABLED && pmtl->nw_t.v6_enable && ipv6_service != IPV6_PASSTHROUGH)
		{
			char v6_prefix[INET6_ADDRSTRLEN] = {0};
			char v6_prefix_length = 0;
			char sdn_prefix[INET6_ADDRSTRLEN] = {0};
			int sdn_prefix_length = 0;
			int ra_lifetime;
			int dhcp_lifetime;
			uint16_t dhcp_start, dhcp_end;
			int share_subnet = 1;
#ifdef RTCONFIG_MULTIWAN_IF
			MTLAN_T *tmp_pmtl = NULL;
			size_t tmp_mtl_sz = 0;

			tmp_pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
			if (tmp_pmtl) {
				get_mtlan_by_idx(SDNFT_TYPE_WAN6, pmtl->sdn_t.wan6_idx, tmp_pmtl, &tmp_mtl_sz);
				if (tmp_mtl_sz == 1)
					share_subnet = 0;
				FREE_MTLAN((void *)tmp_pmtl);
			}
#endif

			strlcpy(v6_prefix, nvram_safe_get(ipv6_nvname_by_unit("ipv6_prefix", wan6_unit)), sizeof(v6_prefix));
			v6_prefix_length = nvram_get_int(ipv6_nvname_by_unit("ipv6_prefix_length", wan6_unit)) ?:64;
			ra_lifetime = nvram_get_int("ra_lifetime") ? : 600;
			dhcp_lifetime = nvram_get_int(ipv6_nvname_by_unit("ipv6_dhcp_lifetime", wan6_unit));

			fprintf(fp, "interface=%s\n", pmtl->nw_t.ifname);
			fprintf(fp, "except-interface=lo\n");
			fprintf(fp, "enable-ra\n"
				"ra-param=%s,%d,%d\n"
				, pmtl->nw_t.ifname, 10, ra_lifetime);

			if (v6_prefix[0] && v6_prefix_length <= 104)
			{
				if (share_subnet) {
					sdn_prefix_length = mtlan_extend_prefix_by_subnet_idx(
						v6_prefix, v6_prefix_length, pmtl->sdn_t.sdn_idx, 8, sdn_prefix, sizeof(sdn_prefix));
				}
				else {
					sdn_prefix_length = v6_prefix_length;
					strlcpy(sdn_prefix, v6_prefix, sizeof(sdn_prefix));
				}
			}
			else
			{
				sdn_prefix_length = mtlan_extend_prefix_by_subnet_idx(
					nvram_safe_get("ipv6_ula_random"), 48, pmtl->sdn_t.sdn_idx, 16, sdn_prefix, sizeof(sdn_prefix));
			}

			if (sdn_prefix_length > 64 || pmtl->nw_t.v6_autoconf)
			{
				dhcp_start = (strtoul(pmtl->nw_t.dhcp6_min, NULL, 16)) ?: 0x1000;
				dhcp_end = (strtoul(pmtl->nw_t.dhcp6_max, NULL, 16)) ?: 0x2000;
				fprintf(fp, "dhcp-range=%s,%s%04x,%s%04x,%d,%ds\n"
					, pmtl->nw_t.ifname
					, sdn_prefix, (dhcp_start < dhcp_end) ? dhcp_start : dhcp_end
					, sdn_prefix, (dhcp_start < dhcp_end) ? dhcp_end : dhcp_start
					, sdn_prefix_length, dhcp_lifetime);
			}
			else
			{
				fprintf(fp, "dhcp-range=%s,::,constructor:%s,ra-stateless,64,%d\n"
					, pmtl->nw_t.ifname, pmtl->nw_t.ifname, ra_lifetime);
			}

			if (pmtl->nw_t.dns6[0][0])
			{
				fprintf(fp, "dhcp-option=%s,option6:23,%s", pmtl->nw_t.ifname, pmtl->nw_t.dns6[0]);
				if (pmtl->nw_t.dns6[1][0])
					fprintf(fp, ",%s", pmtl->nw_t.dns6[1]);
				if (pmtl->nw_t.dns6[2][0])
					fprintf(fp, ",%s", pmtl->nw_t.dns6[2]);
				fprintf(fp, "\n");
			}
			else
			{
				fprintf(fp, "dhcp-option=%s,option6:23,[::]\n", pmtl->nw_t.ifname);
			}
		}
#endif
		/* SNTP & NTP server */
		if (nvram_get_int("ntpd_enable")) {
			fprintf(fp, "dhcp-option=lan,option6:31,%s\n", "[::]");
			fprintf(fp, "dhcp-option=lan,option6:56,%s\n", "[::]");
		}

		/* Don't log DHCP queries */
		if (nvram_match("dhcpd_querylog","0")) {
			fprintf(fp,"quiet-dhcp\n");
#ifdef RTCONFIG_IPV6
			fprintf(fp,"quiet-dhcp6\n");
#endif
		}

#ifdef RTCONFIG_OPENVPN
		/* OpenVPN clients: WINS and strict-mode support */
		write_ovpn_client_dnsmasq_config(fp);
#endif

		// TODO: TR-069 related.

		// TODO: Set VPN server

		snprintf(buf, sizeof(buf), "dnsmasq-%d.conf", pmtl->sdn_t.sdn_idx);
		append_custom_config(buf, fp);
		fclose(fp);

		snprintf(buf, sizeof(buf), "%d", pmtl->sdn_t.sdn_idx);
		run_custom_script("dnsmasq-sdn.postconf", 120, config_file, buf);

		return 0;
	}
	return -1;
}

static int _handle_sdn_dnsmasq(const MTLAN_T *pmtl, const int action)
{
	char config_path[128] = "";

	if (!pmtl)
		return -1;

	if (action & RC_SERVICE_STOP)
	{
		snprintf(config_path, sizeof(config_path), sdn_dnsmasq_pid_path, pmtl->sdn_t.sdn_idx);
		kill_pidfile_tk(config_path);
#ifdef RTCONFIG_DNSPRIVACY
		snprintf(config_path, sizeof(config_path), sdn_stubby_pid_path, pmtl->nw_t.idx);
		kill_pidfile_tk(config_path);
		unlink(config_path);
#endif
	}

	if (action & RC_SERVICE_START)
	{
		if (pmtl->enable)
		{
			if (pmtl->sdn_t.sdn_idx && pmtl->nw_t.idx) // ignore main LAN or IoT SDN
			{
				// generate dnsmasq config file
				if (!_gen_sdn_dnsmasq_conf(pmtl, config_path, sizeof(config_path)))
				{
					eval("dnsmasq", "-C", config_path, "--log-async");
				}
			}
#ifdef RTCONFIG_DNSPRIVACY
			memset(config_path, 0, sizeof(config_path));
			_start_sdn_stubby(pmtl, config_path, sizeof(config_path));
#endif
		}
	}
	return 0;
}

int remove_sdn()
{
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i = 0;
	char logaccept[32], logdrop[32], path[256];

	get_drop_accept(logdrop, sizeof(logdrop), logaccept, sizeof(logaccept));
	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	get_rm_mtlan(pmtl, &mtl_sz);

	for (i = 0; i < mtl_sz; ++i)
	{
		if (!strcmp(pmtl[i].nw_t.ifname, nvram_safe_get("lan_ifname")))
			continue;

		// kill dnsmasq
		snprintf(path, sizeof(path), sdn_dnsmasq_pid_path, pmtl[i].sdn_t.sdn_idx);
		kill_pidfile_tk(path);

		// remove iptables rules
		reset_sdn_iptables(pmtl[i].sdn_t.sdn_idx);

		// remove routing rules
		_remove_sdn_routing_table(&pmtl[i]);
		_remove_sdn_routing_rule(&pmtl[i], 0);
		_remove_sdn_routing_rule(&pmtl[i], 1);
	}

	// sdn_access_rl
	handle_SDN_internal_access(logdrop, logaccept);

#ifdef RTCONFIG_OPENVPN
	update_ovpn_client_by_sdn_remove(pmtl, mtl_sz);
	update_ovpn_server_by_sdn_remove(pmtl, mtl_sz);
#endif
#ifdef RTCONFIG_WIREGUARD
	update_wgc_by_sdn_remove(pmtl, mtl_sz);
	update_wgs_by_sdn_remove(pmtl, mtl_sz);
#endif
#ifdef RTCONFIG_IPSEC
	update_ipsec_server_by_sdn_remove(pmtl, mtl_sz);
#endif
#ifdef RTCONFIG_VPN_FUSION
	vpnc_set_iptables_rule_by_sdn_remove(pmtl, mtl_sz);
#endif
#ifdef RTCONFIG_GRE
	update_gre_by_sdn_remove(pmtl, mtl_sz);
#endif

	FREE_MTLAN((void *)pmtl);
	return 0;
}

/**********************************************************************************************************
 *  WAN related
 * ********************************************************************************************************/
static int _remove_sdn_routing_table(const MTLAN_T *pmtl)
{
	const char tmpfile[64] = "/tmp/iptmpfile";
	FILE *fp;
	char cmd[1024], buf[1024], tmp[128];

	if (!pmtl)
		return -1;

	// remove rules in routing table
	snprintf(cmd, sizeof(cmd), "ip route show table all | grep %s > %s", pmtl->nw_t.addr, tmpfile);
	system(cmd);

	fp = fopen(tmpfile, "r");
	if (fp)
	{
		snprintf(tmp, sizeof(tmp), "table %d", IP_ROUTE_TABLE_ID_SDN);
		while (fgets(buf, sizeof(buf), fp))
		{
			snprintf(cmd, sizeof(cmd), "ip route del %s", buf);
			system(cmd);
		}
		fclose(fp);
	}
	unlink(tmpfile);
	return 0;
}

static int _remove_sdn_routing_rule(const MTLAN_T *pmtl, int v6)
{
	const char tmpfile[64] = "/tmp/iptmpfile";
	FILE *fp;
	char cmd[1024], buf[1024];

	if (!pmtl)
		return -1;

	// remove ip rule
	snprintf(cmd, sizeof(cmd), "ip %s rule show | grep %s > %s", (v6) ? "-6" : "", pmtl->nw_t.ifname, tmpfile);
	system(cmd);

	fp = fopen(tmpfile, "r");
	if (fp)
	{
		while (fgets(buf, sizeof(buf), fp))
		{
			snprintf(cmd, sizeof(cmd), "ip %s rule del iif %s", (v6) ? "-6" : "", pmtl->nw_t.ifname);
			system(cmd);
		}
		fclose(fp);
	}
	unlink(tmpfile);
	return 0;
}

int update_sdn_by_vpnc(const int vpnc_idx)
{
		MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i, default_vpnc;
	char logaccept[32], logdrop[32];
	
	default_vpnc = nvram_get_int("vpnc_default_wan");
	//_dprintf("[%s, %d]wan_idx = %d, vpnc_idx = %d, default_vpnc = %d\n", __FUNCTION__, __LINE__, wan_idx, vpnc_idx, default_vpnc);
	if(!vpnc_idx)
		return -1;

	get_drop_accept(logdrop, sizeof(logdrop), logaccept, sizeof(logaccept));

	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl)
	{
		get_mtlan(pmtl, &mtl_sz);
		for (i = 0; i < mtl_sz; ++i)
		{
			//_dprintf("[%s, %d]pmtl vpnc_idx = %d, wan_idx = %d, default_wan = %d\n", __FUNCTION__, __LINE__, pmtl[i].sdn_t.vpnc_idx, pmtl[i].sdn_t.wan_idx, default_wan);
			if((pmtl[i].sdn_t.vpnc_idx == vpnc_idx) || //match vpnc idx
				(default_vpnc != 0 && vpnc_idx == default_vpnc))	//defualt wan is vpnc and sdn use default wan
			{
				_handle_sdn_wan(&pmtl[i], logdrop, logaccept);
			}
		}
		FREE_MTLAN((void *)pmtl);
	}
	return 0;
}

#ifdef RTCONFIG_MULTIWAN_IF
int update_sdn_by_wan(const int wan_idx)
{
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i, default_wan, wan_unit = wan_idx;
	char logaccept[32], logdrop[32];

	//_dprintf("[%s, %d]wan_idx = %d, vpnc_idx = %d, default_vpnc = %d\n", __FUNCTION__, __LINE__, wan_idx, vpnc_idx, default_vpnc);
	if(!is_mtwan_unit(wan_idx))
	{
		wan_unit = mtwan_get_unit_by_dualwan_unit(wan_idx);
	}

	if(wan_unit == -1)
		return -1;

	get_drop_accept(logdrop, sizeof(logdrop), logaccept, sizeof(logaccept));

	default_wan = mtwan_get_default_wan();

	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl)
	{
		get_mtlan(pmtl, &mtl_sz);
		for (i = 0; i < mtl_sz; ++i)
		{
			//_dprintf("[%s, %d]pmtl vpnc_idx = %d, wan_idx = %d, default_wan = %d\n", __FUNCTION__, __LINE__, pmtl[i].sdn_t.vpnc_idx, pmtl[i].sdn_t.wan_idx, default_wan);
			if((pmtl[i].sdn_t.wan_idx == wan_unit) || 	//match wan idx
					(default_wan == wan_unit))		//update default wan
			{
				_handle_sdn_wan(&pmtl[i], logdrop, logaccept);
			}
		}
		FREE_MTLAN((void *)pmtl);
	}
	return 0;
}

int write_sdnlan_resolv_dnsmasq(FILE* fp)
{
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int ret = 0;

	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl)
	{
		get_mtlan_by_idx(SDNFT_TYPE_SDN, 0, pmtl, &mtl_sz);
		if (pmtl[0].sdn_t.wan_idx && mtwanduck_get_phy_status(pmtl[0].sdn_t.wan_idx))
		{
			wan_add_resolv_dnsmasq(fp, mtwan_get_mapped_unit(pmtl[0].sdn_t.wan_idx));
			ret = 1;
		}
		FREE_MTLAN((void *)pmtl);
	}
	return (ret);
}
#endif

#ifdef RTCONFIG_MULTISERVICE_WAN
static void _handle_sdn_ms_wan(const MTLAN_T *pmtl)
{
	int wan_unit;
	char wan_prefix[16] = {0};
	char wan_ifname[IFNAMSIZ] = {0};

	if (!nvram_match("switch_wantag", "none"))
		return;

	for (wan_unit = WAN_UNIT_MULTISRV_BASE; wan_unit < WAN_UNIT_MULTISRV_MAX; wan_unit++)
	{
		snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_unit);
		if (nvram_pf_get_int(wan_prefix, "enable") == 0)
			continue;

		/// bridge case
		if (!strcmp(nvram_pf_safe_get(wan_prefix, "proto"), "bridge"))
		{
			strlcpy(wan_ifname, nvram_pf_safe_get(wan_prefix, "ifname"), sizeof(wan_ifname));
			if (is_bridged(pmtl->nw_t.ifname, wan_ifname))
			{	// TODO: Not support multiple WAN bridge yet.
#ifdef RTCONFIG_MULTISERVICE_WAN
				if (wan_unit != pmtl->sdn_t.mswan_idx)
#else
				if (wan_unit != pmtl->sdn_t.wan_idx)
#endif
				{
					eval("brctl", "delif", (char*)pmtl->nw_t.br_ifname, wan_ifname);
					eval("brctl", "addif", nvram_safe_get("lan_ifname"), wan_ifname);
				}
			}
			else
			{
#ifdef RTCONFIG_MULTISERVICE_WAN
				if (wan_unit == pmtl->sdn_t.mswan_idx)
#else
				if (wan_unit == pmtl->sdn_t.wan_idx)
#endif
				{
					del_from_bridge(wan_ifname);
					eval("brctl", "addif", (char*)pmtl->nw_t.br_ifname, wan_ifname);
				}
			}
		}
	}
}
#endif

static int _handle_sdn_wan(const MTLAN_T *pmtl, const char *logdrop, const char *logaccept)
{
	int vpnc_default_wan, assigned_wan = 0;
	char table[32], pref[32];

	if (!pmtl)
		return -1;

	_remove_sdn_routing_rule(pmtl, 0);
	vpnc_default_wan = nvram_get_int("vpnc_default_wan");

	if(pmtl->sdn_t.vpnc_idx != 0)	//assigned vpnc
	{
		//get table id and pref
		snprintf(table, sizeof(table), "%d", IP_ROUTE_TABLE_ID_VPNC_BASE + pmtl->sdn_t.vpnc_idx);
		if (pmtl->sdn_t.sdn_idx)
			snprintf(pref, sizeof(pref), "%d", IP_RULE_PREF_VPNC_POLICY_IF + pmtl->sdn_t.vpnc_idx * 3);
		else
			snprintf(pref, sizeof(pref), "%d", IP_RULE_PREF_DEFAULT_CONN);
		eval("ip", "rule", "add", "iif", (char*)pmtl->nw_t.ifname, "table", table, "pref", pref);
#ifdef RTCONFIG_IPV6
		_remove_sdn_routing_rule(pmtl, 1);
		if(ipv6_enabled() && pmtl->nw_t.v6_enable)
		{
			VPNC_PROTO proto;
			vpnc_init();
			proto = vpnc_get_proto_in_profile_by_vpnc_id(pmtl->sdn_t.vpnc_idx);
			if (proto == VPNC_PROTO_OVPN || proto == VPNC_PROTO_WG)
				eval("ip", "-6", "rule", "add", "iif", (char*)pmtl->nw_t.ifname, "table", table, "pref", pref);
		}
#endif
	}

#ifdef RTCONFIG_MULTIWAN_IF
#ifdef RTCONFIG_MULTIWAN_PROFILE
	if(pmtl->sdn_t.mtwan_idx != 0)	//assigned wan
	{
		char mtwan_prefix[16] = {0};
		int group, wan_unit;

		// set routing table
		snprintf(mtwan_prefix, sizeof(mtwan_prefix), "mtwan%d_", pmtl->sdn_t.mtwan_idx);
		if (nvram_pf_get_int(mtwan_prefix, "enable"))
		{
			assigned_wan = 1;
			group = nvram_pf_get_int(mtwan_prefix, "group") ? : nvram_pf_get_int(mtwan_prefix, "order");

			if (is_mtwan_group_lb(pmtl->sdn_t.mtwan_idx, group))
			{
				mtwan_get_lb_route_table_id(pmtl->sdn_t.mtwan_idx, group, table, sizeof(table));
				mtwan_get_lb_rule_pref(pmtl->sdn_t.mtwan_idx, pref, sizeof(pref));
				eval("ip", "rule", "add", "iif", (char*)pmtl->nw_t.ifname, "table", table, "pref", pref);
			}
			else
			{
				wan_unit = mtwan_get_first_wan_unit_by_group(pmtl->sdn_t.mtwan_idx, group);
				snprintf(pref, sizeof(pref), "%d", mtwan_get_route_rule_pref(wan_unit));
				mtwan_get_route_table_id(wan_unit, table, sizeof(table));
				eval("ip", "rule", "add", "iif", (char*)pmtl->nw_t.ifname, "table", table, "pref", pref);
			}

		}
	}
#endif
	if(pmtl->sdn_t.wan_idx != 0 && assigned_wan == 0)	//assigned wan
	{
		assigned_wan = 1;
		// set routing table
		mtwan_get_route_table_id(pmtl->sdn_t.wan_idx, table, sizeof(table));
		snprintf(pref, sizeof(pref), "%d", mtwan_get_route_rule_pref(pmtl->sdn_t.wan_idx));
		eval("ip", "rule", "add", "iif", (char*)pmtl->nw_t.ifname, "table", table, "pref", pref);
	}
#ifdef RTCONFIG_IPV6
	_remove_sdn_routing_rule(pmtl, 1);
	if(pmtl->sdn_t.wan6_idx != 0)	//assigned wan
	{
		// set routing table
		mtwan_get_route_table_id(pmtl->sdn_t.wan6_idx, table, sizeof(table));
		snprintf(pref, sizeof(pref), "%d", mtwan_get_route_rule_pref(pmtl->sdn_t.wan6_idx));
		eval("ip", "-6", "rule", "add", "iif", (char*)pmtl->nw_t.ifname, "table", table, "pref", pref);
	}
#endif

	// set iptables
	if(pmtl->sdn_t.wan_idx || pmtl->sdn_t.wan6_idx)
		update_SDN_iptables(pmtl, logdrop, logaccept);
#ifdef RTCONFIG_MULTIWAN_PROFILE
	if (pmtl->sdn_t.mtwan_idx)
		update_SDN_iptables(pmtl, logdrop, logaccept);
#endif
#endif

#ifdef RTCONFIG_MULTISERVICE_WAN
	_handle_sdn_ms_wan(pmtl);
#endif

	if(!assigned_wan)
	{
		if(vpnc_default_wan && nvram_match("lan_ifname", pmtl->nw_t.ifname))	//default wan is vpnc, new spec: vpnc_default_wan for LAN(br0) only.
		{
			snprintf(table, sizeof(table), "%d", IP_ROUTE_TABLE_ID_VPNC_BASE + vpnc_default_wan);
			snprintf(pref, sizeof(pref), "%d", IP_RULE_PREF_DEFAULT_CONN);
			eval("ip", "rule", "add", "iif", (char*)pmtl->nw_t.ifname, "table", table, "pref", pref);
#ifdef RTCONFIG_IPV6
			if(ipv6_enabled() && pmtl->nw_t.v6_enable)
			{
				VPNC_PROTO proto;
				vpnc_init();
				proto = vpnc_get_proto_in_profile_by_vpnc_id(vpnc_default_wan);
				if (proto == VPNC_PROTO_OVPN || proto == VPNC_PROTO_WG)
					eval("ip", "-6", "rule", "add", "iif", (char*)pmtl->nw_t.ifname, "table", table, "pref", pref);
			}
#endif
		}
		else //default wan is mtwan
		{
			// set iptables
			update_SDN_iptables(pmtl, logdrop, logaccept);
		}
	}
	return 0;
}

static int _handle_sdn_routing(const MTLAN_T *pmtl)
{
	const char iproute_tmp[] = "/tmp/iproute_tmp";
	char cmd[512], buf[512];
	FILE *fp;

	if(pmtl)
	{
		//search ip route rule
		snprintf(cmd, sizeof(cmd), "ip route show table all | grep %s | grep \"table %d\" > %s", pmtl->nw_t.ifname, IP_ROUTE_TABLE_ID_SDN, iproute_tmp);
		system(cmd);

		//remove old rule
		fp = fopen(iproute_tmp, "r");
		if(fp)
		{
			while(fgets(buf, sizeof(buf), fp))
			{
				snprintf(cmd, sizeof(cmd), "ip route del %s", buf);
				system(cmd);
			}
			fclose(fp);
		}

		if(pmtl->enable)
		{
			snprintf(cmd, sizeof(cmd), "ip route add %s/%d dev %s proto kernel scope link src %s table %d", pmtl->nw_t.subnet, pmtl->nw_t.prefixlen, pmtl->nw_t.ifname, pmtl->nw_t.addr, IP_ROUTE_TABLE_ID_SDN);
			system(cmd);
		}
	}

	snprintf(cmd, sizeof(cmd), "ip rule show | grep \"lookup %d\" > %s", IP_ROUTE_TABLE_ID_SDN, iproute_tmp);
	system(cmd);
	if(f_size(iproute_tmp) == 0)
	{
		snprintf(cmd, sizeof(cmd), "ip rule add table %d pref %d", IP_ROUTE_TABLE_ID_SDN, IP_RULE_PREF_SDN_ROUTE);
		system(cmd);
	}
	unlink(iproute_tmp);
	return 0;
}

void set_sdn_ipv6_mtu(int wan_unit, int mtu)
{
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i;

	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl)
	{
		get_mtlan(pmtl, &mtl_sz);
		for (i = 1; i < mtl_sz; i++)
		{
#ifdef RTCONFIG_MULTIWAN_IF
			if (wan_unit == mtwan_get_mapped_unit(pmtl[i].sdn_t.wan6_idx))
#endif
			{
				ipv6_sysconf(pmtl[i].nw_t.ifname, "mtu", mtu);
			}
		}
		FREE_MTLAN((void *)pmtl);
	}
}

void update_sdn_resolvconf()
{
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i, j;
	FILE *fp;
	char path[128] = {0};
#ifdef RTCONFIG_MULTIWAN_IF
	int real_unit;
	char wan_prefix[16] = {0};
#endif
#ifdef RTCONFIG_MULTIWAN_PROFILE
	char nvname[32] = {0};
	int mtwan_group;
	int tmp_group;
	char mt_group[32] = {0};
	char word[4] = {0}, *next = NULL;
#else
	int primary_unit = wan_primary_ifunit();
#endif
#if 0//defined(RTCONFIG_MULTISERVICE_WAN)
	int k;
	int mswan_unit;
#endif
#ifdef RTCONFIG_IPV6
	int wan6_unit;
#endif

	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl)
	{
		get_mtlan(pmtl, &mtl_sz);
		for (i = 0; i < mtl_sz; i++)
		{
			snprintf(path, sizeof(path), sdn_resolv_dnsmasq_path, pmtl[i].sdn_t.sdn_idx);
			fp = fopen(path, "w");
			if(fp)
			{
#if defined(RTCONFIG_MULTIWAN_PROFILE)
				if (pmtl[i].sdn_t.mtwan_idx)
				{
					snprintf(nvname, sizeof(nvname), "mtwan%d_group", pmtl[i].sdn_t.mtwan_idx);
					mtwan_group = nvram_get_int(nvname);
					snprintf(nvname, sizeof(nvname), "mtwan%d_mt_group", pmtl[i].sdn_t.mtwan_idx);
					strlcpy(mt_group, nvram_safe_get(nvname), sizeof(mt_group));
					j = 0;
					foreach(word, mt_group, next)
					{
						tmp_group = atoi(word);
						if (tmp_group == 0
						 || tmp_group != mtwan_group
						 || !mtwanduck_get_phy_status(j + MULTI_WAN_START_IDX)
						 || (real_unit = mtwan_get_real_wan(j + MULTI_WAN_START_IDX, wan_prefix, sizeof(wan_prefix))) < 0
						) {
							j++;
							continue;
						}
						wan_add_resolv_dnsmasq(fp, real_unit);

#if 0//defined(RTCONFIG_MULTISERVICE_WAN)
						for (k = 1; k < WAN_MULTISRV_MAX; k++)
						{
							mswan_unit = get_ms_wan_unit(real_unit, k);
							snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", mswan_unit);
							if (nvram_pf_get_int(wan_prefix, "state_t") != WAN_STATE_CONNECTED)
								continue;
							_wan_add_resolv_dnsmasq(fp, mswan_unit);
						}
#endif

						j++;
					}

				}
				else
#endif
#if defined(RTCONFIG_MULTIWAN_IF)
				if (pmtl[i].sdn_t.wan_idx)
				{
					if (mtwanduck_get_phy_status(pmtl[i].sdn_t.wan_idx))
					{
						real_unit = mtwan_get_real_wan(pmtl[i].sdn_t.wan_idx, wan_prefix, sizeof(wan_prefix));
						if (real_unit >= 0)
						{
							wan_add_resolv_dnsmasq(fp, real_unit);

#if 0//def RTCONFIG_MULTISERVICE_WAN
							for (k = 1; k < WAN_MULTISRV_MAX; k++)
							{
								mswan_unit = get_ms_wan_unit(real_unit, k);
								snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", mswan_unit);
								if (nvram_pf_get_int(wan_prefix, "state_t") != WAN_STATE_CONNECTED)
									continue;
								wan_add_resolv_dnsmasq(fp, mswan_unit);
							}
#endif
						}
					}
				}
				else
#endif
				{
#if defined(RTCONFIG_MULTIWAN_PROFILE)
					if (nvram_get_int("mtwan1_enable"))
					{
						mtwan_group = nvram_get_int("mtwan1_group");
						strlcpy(mt_group, nvram_safe_get("mtwan1_mt_group"), sizeof(mt_group));
						j = 0;
						foreach(word, mt_group, next)
						{
							tmp_group = atoi(word);
							if (tmp_group == 0
							 || tmp_group != mtwan_group
							 || !mtwanduck_get_phy_status(j + MULTI_WAN_START_IDX)
							 || (real_unit = mtwan_get_real_wan(j + MULTI_WAN_START_IDX, wan_prefix, sizeof(wan_prefix))) < 0
							) {
								j++;
								continue;
							}
							wan_add_resolv_dnsmasq(fp, real_unit);

#if 0//defined(RTCONFIG_MULTISERVICE_WAN)
							for (k = 1; k < WAN_MULTISRV_MAX; k++)
							{
								mswan_unit = get_ms_wan_unit(real_unit, k);
								snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", mswan_unit);
								if (nvram_pf_get_int(wan_prefix, "state_t") != WAN_STATE_CONNECTED)
									continue;
								wan_add_resolv_dnsmasq(fp, mswan_unit);
							}
#endif

							j++;
						}
					}
					else
					{
						for (j = MULTI_WAN_START_IDX; j < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; j++)
						{
							real_unit = mtwan_get_real_wan(j, wan_prefix, sizeof(wan_prefix));
							if (real_unit != WAN_UNIT_FIRST)
								continue;
							if (!mtwanduck_get_phy_status(j))
								continue;
							wan_add_resolv_dnsmasq(fp, real_unit);

#if 0//defined(RTCONFIG_MULTISERVICE_WAN)
							for (k = 1; k < WAN_MULTISRV_MAX; k++)
							{
								mswan_unit = get_ms_wan_unit(real_unit, k);
								snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", mswan_unit);
								if (nvram_pf_get_int(wan_prefix, "state_t") != WAN_STATE_CONNECTED)
									continue;
								_wan_add_resolv_dnsmasq(fp, mswan_unit);
							}
#endif
						}
					}
#else
					/// as update_resolvconf()
					for (j = WAN_UNIT_FIRST; j < WAN_UNIT_MAX; j++)
					{
#ifdef RTCONFIG_DUALWAN
						/* skip disconnected WANs in LB mode */
						if (nvram_match("wans_mode", "lb")) {
							if (!is_phy_connect(j))
								continue;
						} else
						/* skip non-primary WANs except not fully connected in FB mode */
						if (nvram_match("wans_mode", "fb")) {
							if (j != primary_unit)
								continue;
						} else
#endif
						/* skip non-primary WANs */
						if (j != primary_unit)
							continue;

						wan_add_resolv_dnsmasq(fp, j);

#if 0//defined(RTCONFIG_MULTISERVICE_WAN)
						for (k = 1; k < WAN_MULTISRV_MAX; k++)
						{
							mswan_unit = get_ms_wan_unit(j, k);
							snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", mswan_unit);
							if (nvram_pf_get_int(wan_prefix, "state_t") != WAN_STATE_CONNECTED)
								continue;
							wan_add_resolv_dnsmasq(fp, mswan_unit);
						}
#endif
					}
#endif	//defined(RTCONFIG_MULTIWAN_PROFILE)
				}

#ifdef RTCONFIG_IPV6
				if (pmtl[i].nw_t.v6_enable)
				{
#if defined(RTCONFIG_MULTIWAN_IF)
					if (pmtl[i].sdn_t.wan6_idx)
						wan6_unit = mtwan_get_mapped_unit(pmtl[i].sdn_t.wan6_idx);
					else
#endif
					wan6_unit = wan_primary_ifunit_ipv6();
					wan_add_resolv_dnsmasq_ipv6(fp, wan6_unit);
				}
#endif
				fclose(fp);
				chmod(path, 0666);

				reload_dnsmasq(pmtl[i].sdn_t.sdn_idx);
			}
		}

		FREE_MTLAN((void *)pmtl);
	}
}

#ifdef RTCONFIG_MULTIWAN_PROFILE
void update_sdn_mtwan_iptables(const MTLAN_T *pmtl)
{
	int mtwan_idx = 0;
	char mtwan_prefix[16] = {0};
	char chain_li[16] = {0}, chain_lb[16] = {0};
	int group;
	char net[INET_ADDRSTRLEN+3] = {0};

	if (!pmtl)
		return;

	snprintf(chain_li, sizeof(chain_li), "MTWAN_IN_L%d", pmtl->sdn_t.sdn_idx);
	eval("iptables", "-t", "mangle", "-F", chain_li);

	mtwan_idx = pmtl->sdn_t.mtwan_idx;
	if (mtwan_idx)
	{
		snprintf(mtwan_prefix, sizeof(mtwan_prefix), "mtwan%d_", mtwan_idx);
		if (nvram_pf_get_int(mtwan_prefix, "enable"))
		{
			group = nvram_pf_get_int(mtwan_prefix, "group");
			if (is_mtwan_group_lb(mtwan_idx, group))
			{
				snprintf(chain_lb, sizeof(chain_lb), "MTWAN_LB_%d_%d", mtwan_idx, group);
				snprintf(net, sizeof(net), "%s/%d", pmtl->nw_t.subnet, pmtl->nw_t.prefixlen);
				eval("iptables", "-t", "mangle", "-A", chain_li, "-d", net, "-j", "RETURN");
				eval("iptables", "-t", "mangle", "-A", chain_li, "-i", (char*)(pmtl->nw_t.ifname), "-j", chain_lb);
			}
		}
	}
	else if (pmtl->sdn_t.wan_idx == 0 && nvram_get_int("mtwan1_enable"))	//follow default Multi-WAN profile
	{
		group = nvram_get_int("mtwan1_group");
		if (is_mtwan_group_lb(1, group))
		{
			snprintf(chain_lb, sizeof(chain_lb), "MTWAN_LB_1_%d", group);
			snprintf(net, sizeof(net), "%s/%d", pmtl->nw_t.subnet, pmtl->nw_t.prefixlen);
			eval("iptables", "-t", "mangle", "-A", chain_li, "-d", net, "-j", "RETURN");
			eval("iptables", "-t", "mangle", "-A", chain_li, "-i", (char*)(pmtl->nw_t.ifname), "-j", chain_lb);
		}
	}
}
#endif	//RTCONFIG_MULTIWAN_PROFILE
