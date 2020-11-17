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
 * for Asuswrt-Merlin's OpenVPN support.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

#include <rtconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <bcmnvram.h>
#include <bcmparams.h>
#include <utils.h>
#include <shutils.h>
#include <shared.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include "openvpn_config.h"
#include "openvpn_setup.h"
#include "openvpn_control.h"

void write_ovpn_resolv_dnsmasq(FILE* dnsmasq_conf) {
	int unit;
	char filename[40], prefix[16];
	char *buffer;

	for (unit = 1; unit <= OVPN_CLIENT_MAX; unit++) {
		sprintf(filename, "/etc/openvpn/client%d/client.resolv", unit);
		if (f_exists(filename)) {
			sprintf(prefix, "vpn_client%d_", unit);

			// Don't add servers if policy routing is enabled and dns mode set to "Exclusive"
			// Handled by iptables on a case-by-case basis
			if ((nvram_pf_get_int(prefix, "rgw") >= OVPN_RGW_POLICY ) && (nvram_pf_get_int(prefix, "adns") == OVPN_DNSMODE_EXCLUSIVE))
				continue;

			buffer = read_whole_file(filename);
			if (buffer) {
				fwrite(buffer, 1, strlen(buffer),dnsmasq_conf);
				free(buffer);
			}
		}
	}
}


void write_ovpn_dnsmasq_config(FILE* dnsmasq_conf) {
	char prefix[16], filename[40], varname[32];
	int unit, modeset = 0;
	char *buffer;

	// Add interfaces for servers that provide DNS services
	for (unit = 1; unit <= OVPN_SERVER_MAX; unit++) {
		sprintf(prefix, "vpn_server%d_", unit);
		if (nvram_pf_get_int(prefix, "pdns"))
			fprintf(dnsmasq_conf, "interface=%s%d\n", nvram_pf_safe_get(prefix, "if"), OVPN_SERVER_BASEIF + unit);
	}

	for (unit = 1; unit <= OVPN_CLIENT_MAX; unit++) {
		// Add strict-order if any client is set to "strict" and we haven't done so yet
		if (!modeset) {
			sprintf(filename, "/etc/openvpn/client%d/client.resolv", unit);
			if (f_exists(filename)) {
				snprintf(varname, sizeof(varname), "vpn_client%d_adns", unit);
				if (nvram_get_int(varname) == OVPN_DNSMODE_STRICT) {
					fprintf(dnsmasq_conf, "strict-order\n");
					modeset = 1;
				}
			}

		}

		// Add WINS entries if any client provides it
		sprintf(filename, "/etc/openvpn/client%d/client.conf", unit);
		if (f_exists(filename)) {
			buffer = read_whole_file(filename);
			if (buffer) {
				fwrite(buffer, 1, strlen(buffer),dnsmasq_conf);
				free(buffer);
			}
		}
	}
}

char *get_ovpn_remote_address(char *buf, int len) {
	const char *address;
	char hostname[64];

	strlcpy(hostname, nvram_safe_get("ddns_hostname_x"), sizeof (hostname));

	if (nvram_get_int("ddns_enable_x") && nvram_get_int("ddns_status") &&
	    *hostname &&
	    strcmp(hostname, "all.dnsomatic.com") &&
	    nvram_invmatch("ddns_server_x", "WWW.TUNNELBROKER.NET") )
	{
		if (nvram_match("ddns_server_x","WWW.NAMECHEAP.COM"))
			snprintf(buf, len, "%s.%s", hostname, nvram_safe_get("ddns_username_x"));
		else
			strlcpy(buf, hostname, len);
	}
	else {
		address = get_wanip();
		if (inet_addr_(address) == INADDR_ANY)
			address = "0.0.0.0";
		strlcpy(buf, address, len);
	}

	return buf;
}


void update_ovpn_profie_remote()
{
	char file_path[128], address[64], buffer[256];
	int unit;

	for (unit = 1; unit <= OVPN_SERVER_MAX; unit++) {
		snprintf(file_path, sizeof(file_path), "/etc/openvpn/server%d/client.ovpn", unit);
		if (f_exists(file_path)) {
			snprintf(buffer, sizeof(buffer), "sed -i 's/remote [A-Za-z0-9.-]*/remote %s/ ' %s", get_ovpn_remote_address(address, sizeof(address)), file_path);
			system(buffer);
		}
	}
}


char *ovpn_get_runtime_filename(ovpn_type_t type, int unit, ovpn_key_t key_type, char *buffer, int len) {
	char *filename;

	switch (key_type) {
		case OVPN_CLIENT_STATIC:
		case OVPN_SERVER_STATIC:
			filename = "static.key";
			break;
		case OVPN_CLIENT_CA:
		case OVPN_SERVER_CA:
			filename = "ca.crt";
			break;
		case OVPN_CLIENT_CERT:
		case OVPN_SERVER_CLIENT_CERT:
			filename = "client.crt";
			break;
		case OVPN_SERVER_CERT:
			filename = "server.crt";
			break;
		case OVPN_CLIENT_KEY:
		case OVPN_SERVER_CLIENT_KEY:
			filename = "client.key";
			break;
		case OVPN_SERVER_KEY:
			filename = "server.key";
			break;
		case OVPN_CLIENT_CRL:
		case OVPN_SERVER_CRL:
			filename = "crl.pem";
			break;
		case OVPN_SERVER_CA_KEY:
			filename = "ca.key";
			break;
		case OVPN_SERVER_DH:
			filename = "dh.pem";
			break;
		case OVPN_CLIENT_EXTRA:
		case OVPN_SERVER_EXTRA:
			filename = "extra.pem";
			break;
		default:
			filename = "unknown";
	}

	snprintf(buffer, len, "/etc/openvpn/%s%d/%s",
			type == OVPN_TYPE_CLIENT ? "client" : "server", unit, filename);

	return buffer;
}


int ovpn_setup_iface(char *iface, ovpn_if_t iface_type, int bridge) {
	modprobe("tun");
	f_wait_exists("/dev/net/tun", 5);

	if (eval("openvpn", "--mktun", "--dev", iface)) {
		logmessage("openvpn","Unable to create tunnel interface %s!", iface);
		return -1;
	}

	if (iface_type == OVPN_IF_TAP) {
		if (bridge) {
			if (eval("brctl", "addif", nvram_safe_get("lan_ifname"), iface)) {
				logmessage("openvpn", "Unable to add interface %s to bridge!", iface);
				return -1;
			}
		}

		if (eval("ifconfig", iface, "promisc", "up")) {
			logmessage("openvpn", "Unable to bring tunnel interface %s up!", iface);
			return -1;
		}
	}
	return 0;
}

void ovpn_remove_iface(ovpn_type_t type, int unit) {
	char tmp[8];

	snprintf(tmp, sizeof(tmp), "tap%d", (type == OVPN_TYPE_CLIENT ? OVPN_CLIENT_BASEIF : OVPN_SERVER_BASEIF) + unit);
	eval("openvpn", "--rmtun", "--dev", tmp);

	snprintf(tmp, sizeof(tmp), "tun%d", (type == OVPN_TYPE_CLIENT ? OVPN_CLIENT_BASEIF : OVPN_SERVER_BASEIF) + unit);
	eval("openvpn", "--rmtun", "--dev", tmp);
}


void ovpn_setup_dirs(ovpn_type_t type, int unit) {
	char buffer[64];

	mkdir("/etc/openvpn", 0700);
	sprintf(buffer, "/etc/openvpn/%s%d", (type == OVPN_TYPE_SERVER ? "server" : "client"), unit);
	mkdir(buffer, 0700);

	sprintf(buffer, "/etc/openvpn/vpn%s%d", (type == OVPN_TYPE_SERVER ? "server" : "client"), unit);
	unlink(buffer);
	symlink("/usr/sbin/openvpn", buffer);

	if (type == OVPN_TYPE_CLIENT) {
	        sprintf(buffer, "/etc/openvpn/client%d/vpnrouting.sh", unit);
	        symlink("/usr/sbin/vpnrouting.sh", buffer);
	}
}

void ovpn_cleanup_dirs(ovpn_type_t type, int unit) {
	char buffer[64];

	sprintf(buffer, "/etc/openvpn/%s%d", (type == OVPN_TYPE_SERVER ? "server" : "client"), unit);
	eval("rm", "-rf", buffer);

	sprintf(buffer, "/etc/openvpn/vpn%s%d", (type == OVPN_TYPE_SERVER ? "server" : "client"), unit);
	eval("rm", "-rf", buffer);
}


int ovpn_write_server_config(ovpn_sconf_t *sconf, int unit) {
	char buffer[256], buffer2[2048];
	FILE *fp, *fp_client;
	char *entry, *nvp;
	char *enable, *cname, *addr, *netmask, *push;
	FILE *fp_ccd;

	sprintf(buffer, "/etc/openvpn/server%d/config.ovpn", unit);
	fp = fopen(buffer, "w");
	chmod(buffer, S_IRUSR|S_IWUSR);

	sprintf(buffer, "/etc/openvpn/server%d/client.ovpn", unit);
	fp_client = fopen(buffer, "w");

	if (!fp || !fp_client) {
		if (fp) fclose(fp);
		if (fp_client) fclose(fp_client);
		return -1;
	}

	fprintf(fp_client, "# Config generated by Asuswrt-Merlin %s, requires OpenVPN 2.4.0 or newer.\n\n", nvram_safe_get("buildno"));

	fprintf(fp, "daemon ovpn-server%d\n", unit);
	if (sconf->auth_mode == OVPN_AUTH_TLS) {
		fprintf(fp_client, "client\n");
		if (sconf->if_type == OVPN_IF_TUN) {
			fprintf(fp, "topology subnet\n");
			fprintf(fp, "server %s ", sconf->network);
			fprintf(fp, "%s\n", sconf->netmask);
			fprintf(fp_client, "dev tun\n");
		}
		else if (sconf->if_type == OVPN_IF_TAP) {
			fprintf(fp, "server-bridge");
			if (sconf->dhcp == 0) {
				fprintf(fp, " %s ", sconf->lan_ipaddr);
				fprintf(fp, "%s ", sconf->lan_netmask);
				fprintf(fp, "%s ", sconf->pool_start);
				fprintf(fp, "%s", sconf->pool_end);
			} else {
				fprintf(fp, "\npush \"route 0.0.0.0 255.255.255.255 net_gateway\"");
			}

			fprintf(fp, "\n");
			fprintf(fp_client, "dev tap\n");
			fprintf(fp_client, "# Windows needs the TAP-Win32 adapter name\n");
			fprintf(fp_client, "# from the Network Connections panel\n");
			fprintf(fp_client, "# if you have more than one.\n");
			fprintf(fp_client, ";dev-node MyTap\n");
		}
	}
	else if (sconf->auth_mode == OVPN_AUTH_STATIC) {
		fprintf(fp_client, "mode p2p\n");

		if (sconf->if_type == OVPN_IF_TUN) {
			fprintf(fp, "ifconfig %s ", sconf->local);
			fprintf(fp, "%s\n", sconf->remote);

			fprintf(fp_client, "dev tun\n");
			fprintf(fp_client, "ifconfig %s ", sconf->remote);
			fprintf(fp_client, "%s\n", sconf->local);
		}
		else if (sconf->if_type == OVPN_IF_TAP) {
			fprintf(fp_client, "dev tap\n");
		}
	}

	// Proto
	fprintf(fp, "proto %s\n", sconf->proto);
	if(!strcmp(sconf->proto, "udp"))
		fprintf(fp_client, "proto udp\n");
	else
		fprintf(fp_client, "proto tcp-client\n");

	// Port
	fprintf(fp, "port %d\n", sconf->port);

	// Interface
	fprintf(fp, "dev %s\n", sconf->if_name);
	fprintf(fp, "txqueuelen 1000\n");

	// Remote address
	fprintf(fp_client, "remote %s %d\n", get_ovpn_remote_address(buffer, sizeof(buffer)), sconf->port);

	fprintf(fp_client, "resolv-retry infinite\n");
	fprintf(fp_client, "nobind\n");
	fprintf(fp_client, "float\n");

	// cipher
	if (sconf->auth_mode == OVPN_AUTH_TLS) {
		if (*sconf->ncp_ciphers) {
			fprintf(fp, "data-ciphers %s\n", sconf->ncp_ciphers);
			fprintf(fp_client, "ncp-ciphers %s\n", sconf->ncp_ciphers);
		}
	} else {	// OVPN_AUTH_STATIC
		if (strcmp(sconf->cipher, "default")) {
			fprintf(fp, "cipher %s\n", sconf->cipher);
			fprintf(fp_client, "cipher %s\n", sconf->cipher);
		}
	}

	// Digest
	if (strcmp(sconf->digest, "default")) {
		fprintf(fp, "auth %s\n", sconf->digest);
		fprintf(fp_client, "auth %s\n", sconf->digest);
	}

	// Compression
	if (strcmp(sconf->comp, "-1")) {
		if (!strncmp(sconf->comp, "lz4", 3)) {
			fprintf(fp, "compress %s\n", sconf->comp);
			fprintf(fp_client, "compress %s\n", sconf->comp);
		} else if (!strcmp(sconf->comp, "yes")) {
			fprintf(fp, "compress lzo\n");
			fprintf(fp_client, "compress lzo\n");
		} else if (!strcmp(sconf->comp, "adaptive")) {
			fprintf(fp, "comp-lzo adaptive\n");
			fprintf(fp_client, "comp-lzo adaptive\n");
		} else if (!strcmp(sconf->comp, "no")) {
			fprintf(fp, "compress\n");	// Disable, but client can override if desired
			fprintf(fp_client, "compress\n");
		}
	}

	fprintf(fp, "keepalive 15 60\n");
	fprintf(fp_client, "keepalive 15 60\n");

	fprintf(fp, "verb %d\n", sconf->verb);

	if (sconf->auth_mode == OVPN_AUTH_TLS) {
		struct in_addr netaddr;
		netaddr.s_addr = inet_addr_(sconf->lan_ipaddr) & inet_addr_(sconf->lan_netmask);

		if ((sconf->if_type == OVPN_IF_TUN) && (sconf->push_lan != OVPN_CLT_ACCESS_WAN) ) {
			fprintf(fp, "push \"route %s %s vpn_gateway %d\"\n",
				inet_ntoa(netaddr), sconf->lan_netmask, PUSH_LAN_METRIC);
		}

		if (sconf->ccd) {
			fprintf(fp, "client-config-dir ccd\n");

			if (sconf->c2c)
				fprintf(fp, "client-to-client\n");

			if (sconf->ccd_excl)
				fprintf(fp, "ccd-exclusive\n");

			sprintf(buffer, "/etc/openvpn/server%d/ccd", unit);
			mkdir(buffer, 0700);
			chdir(buffer);

			strlcpy(buffer2, sconf->ccd_val, sizeof(buffer2));

			nvp = buffer2;
			while ((entry = strsep(&nvp, "<")) != NULL) {
				if (vstrsep(entry, ">", &enable, &cname, &addr, &netmask, &push) != 5)
					continue;

				if (!atoi(enable)) {
					continue;
				} else if (!*cname || !*addr || !*netmask) {
					continue;
				}

				fp_ccd = fopen(cname, "a");
				chmod(cname, S_IRUSR|S_IWUSR);

				if (fp_ccd) {
					fprintf(fp_ccd, "iroute %s %s\n", addr, netmask);
					fprintf(fp, "route %s %s\n", addr, netmask);
					fclose(fp_ccd);
				}

				if (atoi(push)) {
					if (sconf->c2c)
						fprintf(fp, "push \"route %s %s\"\n", addr, netmask);
				}
			}

			// Copy any user-configured client config files
			sprintf(buffer, "/jffs/configs/openvpn/ccd%d", unit);

			if (check_if_dir_exist(buffer)) {
				sprintf(buffer2, "cp %s/* .", buffer); /* */
				system(buffer2);
			}
		}
		else
			fprintf(fp, "duplicate-cn\n");

		if (sconf->push_dns) {
			if (nvram_safe_get("lan_domain")[0] != '\0')
				fprintf(fp, "push \"dhcp-option DOMAIN %s\"\n", nvram_safe_get("lan_domain"));

			strlcpy(buffer, nvram_safe_get("dhcp_dns1_x"), sizeof (buffer));
			strlcpy(buffer2, nvram_safe_get("dhcp_dns2_x"), sizeof (buffer2));

			if (*buffer)
				fprintf(fp, "push \"dhcp-option DNS %s\"\n", buffer);
			if (*buffer2)
				fprintf(fp, "push \"dhcp-option DNS %s\"\n", buffer2);

			if (nvram_get_int("dhcpd_dns_router") ||  (*buffer == '\0' && *buffer2 == '\0'))
				fprintf(fp, "push \"dhcp-option DNS %s\"\n", sconf->lan_ipaddr);
		}

		if (sconf->push_lan != OVPN_CLT_ACCESS_LAN) {
			if (sconf->if_type == OVPN_IF_TAP)
				fprintf(fp, "push \"route-gateway %s\"\n", sconf->lan_ipaddr);
			fprintf(fp, "push \"redirect-gateway def1\"\n");
		}

		// tls-crypt/tls-auth
		if (sconf->tlscrypt)
			fprintf(fp, "tls-crypt static.key\n");
		else if (sconf->direction != -1) {
			fprintf(fp, "tls-auth static.key");

			if (sconf->direction < 2)
				fprintf(fp, " %d", sconf->direction);
			fprintf(fp, "\n");
		}

		if (sconf->userauth) {
			// authentication
			fprintf(fp, "plugin /usr/lib/openvpn-plugin-auth-pam.so openvpn\n");
			fprintf(fp_client, "auth-user-pass\n");

			//ignore client certificate, but only if user/pass auth is enabled
			//That way, existing configuration (pre-Asus OVPN)
			//will remain unchanged.
			if (sconf->useronly) {
				fprintf(fp, "verify-client-cert none\n");
				fprintf(fp, "username-as-common-name\n");
			}
		}

		fprintf(fp_client, "remote-cert-tls server\n");

		// Key and certs
		fprintf(fp, "ca ca.crt\n");

		if (!strncmp(get_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_DH, buffer2, sizeof(buffer2)), "none", 4))
			fprintf(fp, "dh none\n");
		else
			fprintf(fp, "dh dh.pem\n");
		fprintf(fp, "cert server.crt\n");
		fprintf(fp, "key server.key\n");

		if (ovpn_key_exists(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CRL))
			fprintf(fp, "crl-verify crl.pem\n");

		if (ovpn_key_exists(OVPN_TYPE_SERVER, unit, OVPN_SERVER_EXTRA))
			fprintf(fp, "extra-certs extra.pem\n");
	}
	else if (sconf->auth_mode == OVPN_AUTH_STATIC)
		fprintf(fp, "secret static.key\n");

	fprintf(fp, "script-security 2\n");

	sprintf(buffer, "/etc/openvpn/server%d/ovpn-up", unit);
	symlink("/sbin/rc", buffer);
	sprintf(buffer, "/etc/openvpn/server%d/ovpn-down", unit);
	symlink("/sbin/rc", buffer);

	fprintf(fp, "up 'ovpn-up %d server'\n", unit);
	fprintf(fp, "down 'ovpn-down %d server'\n", unit);

	fprintf(fp, "status-version 2\n");
	fprintf(fp, "status status 5\n");
	fprintf(fp, "\n# Custom Configuration\n");
	fprintf(fp, "%s", sconf->custom);
	fclose(fp);
	fclose(fp_client);

	return 0;
}


int ovpn_write_client_config(ovpn_cconf_t *cconf, int unit) {
	char buffer[256];
	FILE *fp;

	sprintf(buffer, "/etc/openvpn/client%d/config.ovpn", unit);
	fp = fopen(buffer, "w");

	if (!fp)
		return -1;

	chmod(buffer, S_IRUSR|S_IWUSR);
	fprintf(fp, "daemon ovpn-client%d\n", unit);
	if (cconf->auth_mode == OVPN_AUTH_TLS )
		fprintf(fp, "client\n");
	fprintf(fp, "dev %s\n", cconf->if_name);
	fprintf(fp, "txqueuelen 1000\n");
	fprintf(fp, "proto %s\n", cconf->proto);
	fprintf(fp, "remote %s %d\n", cconf->addr, cconf->port);
	if (cconf->auth_mode == OVPN_AUTH_STATIC) {
		fprintf(fp, "ifconfig %s ", cconf->local);
		if (cconf->if_type == OVPN_IF_TUN)
			fprintf(fp, "%s\n", cconf->remote);
		else if (cconf->if_type == OVPN_IF_TAP)
			fprintf(fp, "%s\n", cconf->netmask);
	}
	if (cconf->retry)
		fprintf(fp, "connect-retry-max %d\n", cconf->retry);

	fprintf(fp, "nobind\n");
	fprintf(fp, "persist-key\n");
	fprintf(fp, "persist-tun\n");

	if (strcmp(cconf->comp, "-1")) {
		if (!strncmp(cconf->comp, "lz4", 3)) {
			fprintf(fp, "compress %s\n", cconf->comp);
		} else if (!strcmp(cconf->comp, "yes")) {
			fprintf(fp, "compress lzo\n");
		} else if (!strcmp(cconf->comp, "adaptive")) {
			fprintf(fp, "comp-lzo adaptive\n");
		} else if (!strcmp(cconf->comp, "no")) {
			fprintf(fp, "compress\n");      // Disable, but can be overriden
		} else if (!strncmp(cconf->comp, "stub", 4)) {
			fprintf(fp, "compress %s\n", cconf->comp);
		}
	}

	// cipher
	if (cconf->auth_mode == OVPN_AUTH_TLS) {
		if (*cconf->ncp_ciphers)
			fprintf(fp, "data-ciphers %s\n", cconf->ncp_ciphers);
	} else {	// OVPN_AUTH_STATIC
		if (strcmp(cconf->cipher, "default")) {
			fprintf(fp, "cipher %s\n", cconf->cipher);
		}
	}

	if (strcmp(cconf->digest, "default"))
		fprintf(fp, "auth %s\n", cconf->digest);

	if (cconf->redirect_gateway == OVPN_RGW_ALL) {
		if ((cconf->if_type == OVPN_IF_TAP) && *cconf->gateway )
			fprintf(fp, "route-gateway %s\n", cconf->gateway);
		fprintf(fp, "redirect-gateway def1\n");
	} else if (cconf->redirect_gateway == OVPN_RGW_POLICY_STRICT) {
		fprintf(fp, "route-noexec\n");
	}

	if (cconf->auth_mode == OVPN_AUTH_TLS) {
		if (cconf->reneg >= 0)
			fprintf(fp, "reneg-sec %d\n", cconf->reneg);

		if ((cconf->direction >= 0) && ovpn_key_exists(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_STATIC)) {
			if (cconf->tlscrypt == 1)
				fprintf(fp, "tls-crypt static.key\n");
			else if (cconf->tlscrypt == 2)
				fprintf(fp, "tls-crypt-v2 static.key\n");
			else {
				fprintf(fp, "tls-auth static.key");
				if (cconf->direction < 2)
					fprintf(fp, " %d", cconf->direction);
				fprintf(fp, "\n");
			}
		}

		if (ovpn_key_exists(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_CA))
			fprintf(fp, "ca ca.crt\n");

		if (!(cconf->useronly && cconf->userauth)) {
			if (ovpn_key_exists(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_CERT))
				fprintf(fp, "cert client.crt\n");
			if (ovpn_key_exists(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_KEY))
				fprintf(fp, "key client.key\n");
		}

		if (cconf->verify_x509_type) {
			fprintf(fp, "verify-x509-name \"%s\" ",  cconf->verify_x509_name);
			switch(cconf->verify_x509_type) {
				case 1:
					fprintf(fp, "name\n");
					break;
				case 2:
					fprintf(fp, "name-prefix\n");
					break;
				case 3:
					fprintf(fp, "subject\n");
					break;
				default:
					fprintf(fp, "name\n");
					break;
			}
		}
		if (cconf->userauth)
			fprintf(fp, "auth-user-pass auth\n");

		if (ovpn_key_exists(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_CRL))
			fprintf(fp, "crl-verify crl.pem\n");

		if (ovpn_key_exists(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_EXTRA))
			fprintf(fp, "extra-certs extra.pem\n");
	}
	else if (cconf->auth_mode == OVPN_AUTH_STATIC) {
		if (ovpn_key_exists(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_STATIC))
			fprintf(fp, "secret static.key\n");
	}

	sprintf(buffer, "/etc/openvpn/client%d/ovpn-up", unit);
	symlink("/sbin/rc", buffer);
	sprintf(buffer, "/etc/openvpn/client%d/ovpn-down", unit);
	symlink("/sbin/rc", buffer);

	fprintf(fp, "up 'ovpn-up %d client'\n", unit);
	fprintf(fp, "down 'ovpn-down %d client'\n", unit);

	// For selective routing
	fprintf(fp, "script-security 2\n");     // also for up/down scripts
	fprintf(fp, "route-delay 2\n");
	fprintf(fp, "route-up vpnrouting.sh\n");
	fprintf(fp, "route-pre-down vpnrouting.sh\n");

	fprintf(fp, "verb %d\n", cconf->verb);
	fprintf(fp, "status-version 2\n");
	fprintf(fp, "status status 5\n");
	fprintf(fp, "\n# Custom Configuration\n");
	fprintf(fp, "%s", cconf->custom);
	fclose(fp);

	return 0;
}


void ovpn_write_client_keys(ovpn_cconf_t *cconf, int unit) {
	char buffer[64];
	FILE *fp;

	if (cconf->auth_mode == OVPN_AUTH_TLS) {
		ovpn_write_key(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_CA);
		ovpn_write_key(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_CRL);
		ovpn_write_key(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_EXTRA);

		if (!(cconf->useronly && cconf->userauth)) {
			ovpn_write_key(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_KEY);
			ovpn_write_key(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_CERT);
		}
		if (cconf->userauth) {
			sprintf(buffer, "/etc/openvpn/client%d/auth", unit);
			fp = fopen(buffer, "w");
			if (fp) {
				fprintf(fp, "%s\n", cconf->username);
				fprintf(fp, "%s\n", cconf->password);
				fclose(fp);
				chmod(buffer, S_IRUSR|S_IWUSR);
			}
		}
	}

	if (cconf->auth_mode == OVPN_AUTH_STATIC || (cconf->auth_mode == OVPN_AUTH_TLS && cconf->direction >= 0))
		ovpn_write_key(OVPN_TYPE_CLIENT, unit, OVPN_CLIENT_STATIC);
}


void ovpn_write_server_keys(ovpn_sconf_t *sconf, int unit) {
	char buffer[64], buffer2[8000];
	FILE *fp, *fp_client;
	int valid_client_cert = 0;

	// Need to append inline key/certs
	sprintf(buffer, "/etc/openvpn/server%d/client.ovpn", unit);
	fp_client = fopen(buffer, "a");

	if (!fp_client)
		return;

	if (sconf->auth_mode == OVPN_AUTH_TLS) {
		if ( !ovpn_key_exists(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CA) &&
		     !ovpn_key_exists(OVPN_TYPE_SERVER, unit, OVPN_SERVER_KEY) &&
		     !ovpn_key_exists(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CERT))
		{
			sprintf(buffer, "/tmp/genvpncert.sh");
			fp = fopen(buffer, "w");
			if(fp) {
				fprintf(fp, "#!/bin/sh\n");
				fprintf(fp, "export OPENSSL=\"openssl\"\n");
				fprintf(fp, "export GREP=\"grep\"\n");
				fprintf(fp, "export KEY_CONFIG=\"/rom/easy-rsa/openssl-1.0.0.cnf\"\n");
				fprintf(fp, "export KEY_DIR=\"/etc/openvpn/server%d\"\n", unit);
				fprintf(fp, "export KEY_SIZE=%d\n", sconf->tls_keysize ? 2048 : 1024);
				fprintf(fp, "export CA_EXPIRE=3650\n");
				fprintf(fp, "export KEY_EXPIRE=3650\n");
				fprintf(fp, "export KEY_COUNTRY=\"TW\"\n");
				fprintf(fp, "export KEY_PROVINCE=\"TW\"\n");
				fprintf(fp, "export KEY_CITY=\"Taipei\"\n");
				fprintf(fp, "export KEY_ORG=\"ASUS\"\n");
				fprintf(fp, "export KEY_EMAIL=\"me@asusrouter.lan\"\n");
				fprintf(fp, "export KEY_OU=\"Home/Office\"\n");
				fprintf(fp, "export KEY_CN=\"%s\"\n", nvram_safe_get("productid"));

				fprintf(fp, "touch /etc/openvpn/server%d/index.txt\n", unit);
				fprintf(fp, "$OPENSSL rand -hex 16  >/etc/openvpn/server%d/serial\n", unit);
				fprintf(fp, "/rom/easy-rsa/pkitool --initca\n");
				fprintf(fp, "/rom/easy-rsa/pkitool --server server\n");

				fprintf(fp, "export KEY_CN=\"\"\n");
				fprintf(fp, "/rom/easy-rsa/pkitool client\n");

				fclose(fp);
				chmod(buffer, 0700);
				eval(buffer);
				unlink(buffer);
			}

			// save certificates and keys
			ovpn_get_runtime_filename(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CA_KEY, buffer2, sizeof(buffer2));
			set_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CA_KEY, NULL, buffer2);

			ovpn_get_runtime_filename(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CA, buffer2, sizeof(buffer2));
			set_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CA, NULL, buffer2);

			ovpn_get_runtime_filename(OVPN_TYPE_SERVER, unit, OVPN_SERVER_KEY, buffer2, sizeof(buffer2));
			set_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_KEY, NULL, buffer2);

			ovpn_get_runtime_filename(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CERT, buffer2, sizeof(buffer2));
			set_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CERT, NULL, buffer2);

			ovpn_get_runtime_filename(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CLIENT_KEY, buffer2, sizeof(buffer2));
			set_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CLIENT_KEY, NULL, buffer2);

			ovpn_get_runtime_filename(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CLIENT_CERT, buffer2, sizeof(buffer2));
			set_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CLIENT_CERT, NULL, buffer2);

			valid_client_cert = 1;
		} else {
			ovpn_write_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CA_KEY);
			ovpn_write_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CA);
			ovpn_write_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_KEY);
			ovpn_write_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CERT);
			ovpn_write_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CRL);
			ovpn_write_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_EXTRA);
		}

		fprintf(fp_client, "<ca>\n%s\n</ca>\n",
		                   get_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CA, buffer2, sizeof(buffer2)));

		if (ovpn_key_exists(OVPN_TYPE_SERVER, unit, OVPN_SERVER_EXTRA)) {
			fprintf(fp_client, "<extra-certs>\n%s\n</extra-certs>\n",
			                   get_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_EXTRA, buffer2, sizeof(buffer2)));
		}

		ovpn_write_dh(sconf, unit);

		// Client key and certificate - if not using user-only authentication
		if (!(sconf->userauth && sconf->useronly)) {
			if (!valid_client_cert) {
				valid_client_cert = ovpn_is_clientcert_valid(unit);
			}
			fprintf(fp_client, "<cert>\n");
			if (valid_client_cert)
				fprintf(fp_client, "%s\n", get_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CLIENT_CERT, buffer2, sizeof(buffer2)));
			else
				fprintf(fp_client, "    paste client certificate data here\n");
			fprintf(fp_client, "</cert>\n");

			fprintf(fp_client, "<key>\n");
			if (valid_client_cert)
				fprintf(fp_client, "%s\n", get_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CLIENT_KEY, buffer2, sizeof(buffer2)));
			else
				fprintf(fp_client, "    paste client key data here\n");
			fprintf(fp_client, "</key>\n");
		}
	}

	if (sconf->auth_mode == OVPN_AUTH_STATIC || (sconf->auth_mode == OVPN_AUTH_TLS && sconf->direction >= 0)) {
		if (ovpn_key_exists(OVPN_TYPE_SERVER, unit, OVPN_SERVER_STATIC)) {
			ovpn_write_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_STATIC);
		} else {
			ovpn_get_runtime_filename(OVPN_TYPE_SERVER, unit, OVPN_SERVER_STATIC, buffer, sizeof(buffer));
			eval("openvpn", "--genkey", "secret", buffer);
			sleep(2);
			set_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_STATIC, NULL, buffer);
		}

		if (sconf->auth_mode == OVPN_AUTH_TLS) {
			if (sconf->direction == 3)
				fprintf(fp_client, "<tls-crypt>\n");
			else
				fprintf(fp_client, "<tls-auth>\n");

			fprintf(fp_client, "%s\n", get_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_STATIC, buffer2, sizeof(buffer2)));

			if (sconf->direction == 3)
				fprintf(fp_client, "</tls-crypt>\n");
			else
				fprintf(fp_client, "</tls-auth>\n");

			if (sconf->direction == 1)
				fprintf(fp_client, "key-direction 0\n");
			else if (sconf->direction == 0)
				fprintf(fp_client, "key-direction 1\n");

		} else if (sconf->auth_mode == OVPN_AUTH_STATIC) {
			fprintf(fp_client, "<secret>\n%s\n</secret>\n",
					    get_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_STATIC, buffer2, sizeof(buffer2)));
		}
	}

	fclose(fp_client);
}


void ovpn_setup_client_fw(ovpn_cconf_t *cconf, int unit) {
	char filename[64];
	FILE *fp;
	struct in_addr netaddr;

	sprintf(filename, "/etc/openvpn/client%d/fw.sh", unit);

	fp = fopen(filename, "w");
	if (!fp)
		return;

	chmod(filename, S_IRUSR|S_IWUSR|S_IXUSR);

	fprintf(fp, "#!/bin/sh\n");
	fprintf(fp, "iptables -I OVPN -i %s -j %s\n", cconf->if_name, (cconf->fw ? "DROP" : "ACCEPT"));

#if !defined(HND_ROUTER)
	// Setup traffic accounting
	if (nvram_match("cstats_enable", "1")) {
		ipt_account(fp, cconf->if_name);
	}
#endif
	if (cconf->nat) {
		netaddr.s_addr = inet_addr_(nvram_safe_get("lan_ipaddr")) & inet_addr_(nvram_safe_get("lan_netmask"));
		fprintf(fp, "iptables -t nat -I POSTROUTING -o %s -j MASQUERADE\n", cconf->if_name);
	}
	// Disable rp_filter when in policy mode - firewall restart would re-enable it
	if (cconf->redirect_gateway >= OVPN_RGW_POLICY) {
		fprintf(fp, "for i in /proc/sys/net/ipv4/conf/*/rp_filter ; do\n"); /* */
		fprintf(fp, "echo 0 > $i\n");
		fprintf(fp, "done\n");
	}

	fclose(fp);
	chmod(filename, S_IRUSR|S_IWUSR|S_IXUSR);
	eval(filename);
}

void ovpn_setup_server_fw(ovpn_sconf_t *sconf, int unit) {
	char buffer[64], buffer2[64], filename[32];
	FILE *fp;

	// Create firewall rules
	sprintf(filename, "/etc/openvpn/server%d/fw.sh", unit);
	fp = fopen(filename, "w");
	if (!fp)
		return;

	fprintf(fp, "#!/bin/sh\n");
	strlcpy(buffer, sconf->proto, sizeof (buffer));
	fprintf(fp, "iptables -t nat -I PREROUTING -p %s --dport %d -j ACCEPT\n", strtok(buffer, "-"), sconf->port);
	strlcpy(buffer, sconf->proto, sizeof (buffer));
	fprintf(fp, "iptables -I INPUT -p %s --dport %d -j ACCEPT\n", strtok(buffer, "-"), sconf->port);

	if (sconf->push_lan == OVPN_CLT_ACCESS_WAN) {
		fprintf(fp, "iptables -I OVPN -i %s ! -d %s/%d -j ACCEPT\n",
		             sconf->if_name, sconf->lan_ipaddr,  convert_subnet_mask_to_cidr(sconf->lan_netmask));

		if (sconf->push_dns) {
			strlcpy(buffer, nvram_safe_get("dhcp_dns1_x"), sizeof (buffer));
			strlcpy(buffer2, nvram_safe_get("dhcp_dns2_x"), sizeof (buffer2));
			// Open in the firewall in case they are within the LAN
			if (*buffer) {
				fprintf(fp, "iptables -I OVPN -i %s -p udp -d %s --dport 53 -j ACCEPT\n", sconf->if_name, buffer);
				fprintf(fp, "iptables -I OVPN -i %s -m tcp -p tcp -d %s --dport 53 -j ACCEPT\n", sconf->if_name, buffer);
			}
			if (*buffer2) {
				fprintf(fp, "iptables -I OVPN -i %s -p udp -d %s --dport 53 -j ACCEPT\n", sconf->if_name, buffer2);
				fprintf(fp, "iptables -I OVPN -i %s -m tcp -p tcp -d %s --dport 53 -j ACCEPT\n", sconf->if_name, buffer2);
			}
			if (nvram_get_int("dhcpd_dns_router") ||  (*buffer == '\0' && *buffer2 == '\0')) {
				fprintf(fp, "iptables -I OVPN -i %s -p udp -d %s --dport 53 -j ACCEPT\n", sconf->if_name, sconf->lan_ipaddr);
				fprintf(fp, "iptables -I OVPN -i %s -m tcp -p tcp -d %s --dport 53 -j ACCEPT\n", sconf->if_name, sconf->lan_ipaddr);
			}
		}
	} else	if (sconf->push_lan == OVPN_CLT_ACCESS_LAN) {
		fprintf(fp, "iptables -I OVPN -i %s -d %s/%d -j ACCEPT\n",
		             sconf->if_name, sconf->lan_ipaddr,  convert_subnet_mask_to_cidr(sconf->lan_netmask));
	} else {	// WAN + LAN
		fprintf(fp, "iptables -I OVPN -i %s -j ACCEPT\n", sconf->if_name);
	}

#if !defined(HND_ROUTER)
	if (nvram_match("cstats_enable", "1")) {
		ipt_account(fp, sconf->if_name);
	}
#endif
	fclose(fp);
	chmod(filename, S_IRUSR|S_IWUSR|S_IXUSR);
	eval(filename);
}

void ovpn_setup_server_watchdog(ovpn_sconf_t *sconf, int unit) {
	char buffer[64], buffer2[64];
	char taskname[20];
	FILE *fp;

	sprintf(buffer, "/etc/openvpn/server%d/vpn-watchdog%d.sh", unit, unit);

	if ((fp = fopen(buffer, "w"))) {
		fprintf(fp, "#!/bin/sh\n"
		            "if [ -z $(pidof vpnserver%d) ]\n"
		            "then\n"
		            "   service restart_vpnserver%d\n"
		            "fi\n",
		            unit, unit);
		fclose(fp);
		chmod(buffer, S_IRUSR|S_IWUSR|S_IXUSR);

		sprintf(taskname, "CheckVPNServer%d", unit);
		sprintf(buffer2, "*/2 * * * * %s", buffer);
		eval("cru", "a", taskname, buffer2);
	}
}


int ovpn_is_clientcert_valid(int unit) {
	int valid = 0;
	char buffer[64], buffer2[8000], cafile[64];
	BIO *certbio = NULL;
	X509 *cert = NULL;
	X509_STORE *store = NULL;
	X509_STORE_CTX *vrfy_ctx = NULL;

	/*
	   See if stored client cert was signed with our stored CA.  If not, it means
	   the CA was changed by the user and the current client crt/key no longer match,
	   so we should not insert them in the exported client ovpn file.
	*/

	if (!ovpn_key_exists(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CLIENT_KEY)) return 0;

	OpenSSL_add_all_algorithms();

	get_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CLIENT_CERT, buffer2, sizeof(buffer2));
	if ((certbio = BIO_new(BIO_s_mem())) ) {
		if ((store = X509_STORE_new())) {
			if ((vrfy_ctx = X509_STORE_CTX_new())) {
				BIO_puts(certbio, buffer2);
				if ((cert = PEM_read_bio_X509(certbio, NULL, 0, NULL))) {	// client cert
					snprintf(cafile, sizeof(cafile), "%s/%s", OVPN_FS_PATH, get_ovpn_filename(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CA, buffer, sizeof(buffer)));
					if (X509_STORE_load_locations(store, cafile, NULL) == 1) {	// CA cert
						X509_STORE_CTX_init(vrfy_ctx, store, cert, NULL);
						valid = X509_verify_cert(vrfy_ctx);
					}
					X509_free(cert);
				}
				X509_STORE_CTX_free(vrfy_ctx);
			}
			X509_STORE_free(store);
		}
		BIO_free_all(certbio);
	}
	EVP_cleanup();

	//logmessage("openvpn", "Valid crt = %d", valid);

	return valid;
}


/* Validate and generate or copy DH */
void ovpn_write_dh(ovpn_sconf_t *sconf, int unit) {
	FILE *fp;
	char buffer[256];
	char buffer2[8000];
	int len = 0;
	DH *dhparams = NULL;

	get_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_DH, buffer2, sizeof(buffer2));
	if (*buffer2) {
		if (!strncmp(buffer2, "none", 4)) {
			len = 2048;	// Don't validate, we don't use it
		} else {
			fp = fopen(ovpn_get_runtime_filename(OVPN_TYPE_SERVER, unit, OVPN_SERVER_DH, buffer, sizeof(buffer)), "w+");
			if (fp) {
				chmod(buffer, S_IRUSR|S_IWUSR);
				fprintf(fp, "%s", buffer2);
				fflush(fp);
				rewind(fp);

				dhparams = PEM_read_DHparams(fp, NULL, 0, NULL);
				if (dhparams) {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
					len = DH_bits(dhparams);
#else
					len = BN_num_bits(dhparams->p);
#endif
					OPENSSL_free(dhparams);
				}
				if ((len != 0) && (len < 1024))
					logmessage("openvpn","WARNING: DH for server %d is too weak (%d bit, must be at least 1024 bit). Using a pre-generated 2048-bit PEM.", unit, len);
			}
			fclose(fp);
		}
	}

	if (len < 1024) {	// Provide a 2048-bit PEM, from RFC 3526.
		set_ovpn_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_DH, NULL, "/etc/ssl/certs/dh2048.pem");
		ovpn_write_key(OVPN_TYPE_SERVER, unit, OVPN_SERVER_DH);
	}
}
