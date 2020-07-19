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
   until both httpd and rc are linked against the same
   OpenSSL version.  Until then, libvpn cannot use
   OpenSSL functions. */

#include <rc.h>

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

	logmessage("openvpn", "Setting up OpenVPN client %d...", unit);

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

        // Start the VPN client
	if (ovpn_start_instance(OVPN_TYPE_CLIENT, unit)) {
		logmessage("openvpn", "Starting OpenVPN client %d failed!", unit);
		stop_ovpn_client(unit);
		return;
	}
	logmessage("openvpn", "OpenVPN client %d launch completed.", unit);
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

	logmessage("openvpn", "Setting up OpenVPN server %d...", unit);

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
	if (ovpn_start_instance(OVPN_TYPE_SERVER, unit)) {
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

	logmessage("openvpn", "OpenVPN server %d launch completed.", unit);
}


void stop_ovpn_client(int unit) {
	char buffer[256];

	sprintf(buffer, "stop_vpnclient%d", unit);
	if (getpid() != 1) {
                notify_rc(buffer);
		return;
	}

	logmessage("openvpn", "Stopping OpenVPN client %d", unit);

	// Stop the VPN client
	sprintf(buffer, "vpnclient%d", unit);

	killall_tk_period_wait(buffer, 10);

	ovpn_remove_iface(OVPN_TYPE_CLIENT, unit);

	// Remove firewall rules after VPN exit
	sprintf(buffer, "/etc/openvpn/client%d/fw.sh", unit);
	eval("sed", "-i", "s/-A/-D/g;s/-I/-D/g", buffer);
	eval(buffer);

	// Delete all files for this client
	sprintf(buffer, "/etc/openvpn/client%d",unit);
	eval("rm", "-rf", buffer);
	sprintf(buffer, "/etc/openvpn/vpnclient%d",unit);
	eval("rm", "-rf", buffer);

	update_ovpn_status(OVPN_TYPE_CLIENT, unit, OVPN_STS_STOP, OVPN_ERRNO_NONE);
	update_resolvconf();

	logmessage("openvpn", "OpenVPN client %d stopped.", unit);
}


void stop_ovpn_server(int unit) {
	char buffer[256];

	sprintf(buffer, "stop_vpnserver%d", unit);
	if (getpid() != 1) {
		notify_rc(buffer);
		return;
	}

	logmessage("openvpn", "Stopping OpenVPN server %d", unit);
	// Remove cron job
	sprintf(buffer, "CheckVPNServer%d", unit);
	eval("cru", "d", buffer);

	// Stop the VPN server
	sprintf(buffer, "vpnserver%d", unit);
	killall_tk_period_wait(buffer, 10);

	ovpn_remove_iface(OVPN_TYPE_SERVER, unit);

	// Remove firewall rules after VPN exit
	sprintf(buffer, "/etc/openvpn/server%d/fw.sh", unit);

	if (!eval("sed", "-i", "s/-A/-D/g;s/-I/-D/g", buffer))
		eval(buffer);

	// Delete all files for this server
	sprintf(buffer, "/etc/openvpn/server%d",unit);
	eval("rm", "-rf", buffer);
	sprintf(buffer, "/etc/openvpn/vpnserver%d",unit);
	eval("rm", "-rf", buffer);

	update_ovpn_status(OVPN_TYPE_SERVER, unit, OVPN_STS_STOP, OVPN_ERRNO_NONE);

	logmessage("openvpn", "OpenVPN server %d stopped.", unit);
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

	OpenSSL_add_all_algorithms();

	if (!ovpn_key_exists(OVPN_TYPE_SERVER, unit, OVPN_SERVER_CLIENT_KEY)) return 0;

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
