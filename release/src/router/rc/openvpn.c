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

/* RC OpenVPN code for Asuswrt-Merlin.
   This also Contains SSL routines which can't be moved to libvpn
   yet.
*/

#include "rc.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>

#include <openvpn_config.h>
#include <openvpn_control.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

void ovpn_process_eas(int start);


int ovpn_up_main(int argc, char **argv)
{
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
		if ((f_exists(buffer)) && (ovpn_max_dnsmode() != OVPN_DNSMODE_STRICT))
			notify_rc("start_dnsmasq");

	} else
		return -1;

	return 0;
}

int ovpn_down_main(int argc, char **argv)
{
	int unit, restart_dnsmasq = 0;
	char buffer[64];

	if(argc < 3)
		return -1;

	unit = atoi(argv[1]);
	if (!strcmp(argv[2], "server"))
		ovpn_server_down_handler(unit);
	else if (!strcmp(argv[2], "client")) {
		// Check before handler removes client.conf

		sprintf(buffer, "/etc/openvpn/client%d/client.conf", unit);
		// We got client.conf && update_resolvconf() won't restart it for us
		if ((f_exists(buffer)) && (ovpn_max_dnsmode() != OVPN_DNSMODE_STRICT))
			restart_dnsmasq = 1;

		ovpn_client_down_handler(unit);

		update_resolvconf();

		if (restart_dnsmasq)
			notify_rc("start_dnsmasq");

	} else
		return -1;

	return 0;
}


void start_ovpn_client(int unit) {
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

//	logmessage("openvpn", "Setting up OpenVPN client %d...", unit);

	update_ovpn_status(OVPN_TYPE_CLIENT, unit, OVPN_STS_INIT, OVPN_ERRNO_NONE);

	// Retrieve instance configuration
	cconf = ovpn_get_cconf(unit);

        // Setup directories and symlinks
	ovpn_setup_dirs(OVPN_TYPE_CLIENT, unit);

	// Setup interface
	if (ovpn_setup_iface(cconf->if_name, cconf->if_type, cconf->bridge)) {
		stop_ovpn_client(unit);
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
		stop_ovpn_client(unit);
		if (get_ovpn_status(OVPN_TYPE_CLIENT, unit) != OVPN_STS_ERROR)
			update_ovpn_status(OVPN_TYPE_CLIENT, unit, OVPN_STS_ERROR, OVPN_ERRNO_CONF);
	}
}


void start_ovpn_server(int unit) {
	char buffer[256], buffer2[8000];
	int valid = 0;
	ovpn_sconf_t *sconf;

	sprintf(buffer, "start_vpnserver%d", unit);
	if (getpid() != 1) {
		notify_rc(buffer);
		return;
	}

//	logmessage("openvpn", "Setting up OpenVPN server %d...", unit);

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
		stop_ovpn_server(unit);
		free(sconf);
		return;
	}

	// Write config files
	ovpn_write_server_config(sconf, unit);

	// TODO: move to libvpn once that library is linked against OpenSSL
	if (sconf->auth_mode == OVPN_AUTH_TLS) {
		// Client key and certificate - if not using user-only authentication
		// Pre-validate
		if (!(sconf->userauth && sconf->useronly))
			valid = ovpn_is_clientcert_valid(unit);
		else
			valid = 1;
		// DH
		ovpn_write_dh(sconf, unit);
	}

	// Write key/certs
	ovpn_write_server_keys(sconf, unit, valid);

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
		stop_ovpn_server(unit);
		update_ovpn_status(OVPN_TYPE_SERVER, unit, OVPN_STS_ERROR, OVPN_ERRNO_CONF);
		free(sconf);
		return;
	}

	if (sconf->auth_mode == OVPN_AUTH_STATIC)
		update_ovpn_status(OVPN_TYPE_SERVER, unit, OVPN_STS_RUNNING, OVPN_ERRNO_NONE);

	ovpn_setup_server_watchdog(sconf, unit);

	free(sconf);

//	logmessage("openvpn", "OpenVPN server %d launch completed.", unit);
}


void stop_ovpn_client(int unit) {
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

void stop_ovpn_server(int unit) {
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
					len = BN_num_bits(dhparams->p);
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

void start_ovpn_eas() {
	ovpn_process_eas(1);
}

void stop_ovpn_eas() {
	ovpn_process_eas(0);
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
				stop_ovpn_server(unit);

			if (start)
				start_ovpn_server(unit);
		}
	}

	// Update all clients routing (in case some are using a kill switch)
	for( unit = 1; unit <= OVPN_CLIENT_MAX; unit++ ) {
		ovpn_update_routing(unit);
	}

	// Process clients
	strlcpy(enabled, nvram_safe_get("vpn_clientx_eas"), sizeof(enabled));

	for (ptr = enabled; *ptr != '\0'; ptr++) {
		if (!isdigit(*ptr))
			continue;

		unit = atoi(ptr);

		if (unit > 0 && unit <= OVPN_CLIENT_MAX) {
			sprintf(buffer2, "vpnclient%d", unit);
			if (pidof(buffer2) >= 0)
				stop_ovpn_client(unit);

			if (start)
				start_ovpn_client(unit);
		}
	}
}
