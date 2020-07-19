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

#include "openvpn_config.h"

extern struct nvram_tuple router_defaults[];


void reset_ovpn_setting(ovpn_type_t type, int unit, int full){
        struct nvram_tuple *t;
        char prefix[]="vpn_serverX_", tmp[100];
	char *service;
	char start[12], remove[2];
	char *cur;

	if (type == OVPN_TYPE_SERVER) {
		service = "server";
		strlcpy(start, nvram_safe_get("vpn_serverx_start"), sizeof(start));
	}
	else if (type == OVPN_TYPE_CLIENT) {
		service = "client";
		strlcpy(start, nvram_safe_get("vpn_clientx_eas"), sizeof(start));
	}
	else
		return;

	logmessage("openvpn","Resetting %s (unit %d) to default settings", service, unit);

	snprintf(remove, sizeof(remove), "%d", unit);
	tmp[0] = '\0';

	// Remove auto-start
	for (cur = strtok(start, ","); cur != NULL; cur = strtok(NULL, ",")) {
		if (strcmp(cur,remove)) {
			strlcat(tmp, cur, sizeof(tmp));
			strlcat(tmp, ",", sizeof(tmp));
		}
	}
	if (type == OVPN_TYPE_SERVER)
		nvram_set("vpn_serverx_start", tmp);
	else	// Client
		nvram_set("vpn_clientx_eas", tmp);

	// Reset vars
	snprintf(prefix, sizeof(prefix), "vpn_%s%d_", service, unit);

	for (t = router_defaults; t->name; t++) {
		if (strncmp(t->name, prefix, 12)==0)
		{
			// Don't reset these settings unless asked to
			if (!full && (!strcmp(t->name + 12, "desc") ||
				      !strncmp(t->name + 12, "clientlist", 10) ||	/* handle clientlist1 through 5 */
				      !strcmp(t->name + 12, "rgw") ||
				      !strcmp(t->name + 12, "enforce")))
				continue;
			nvram_set(t->name, t->value);
		}
	}

        if (type == OVPN_TYPE_SERVER)  // server-only files
        {
		set_ovpn_key(type, unit, OVPN_SERVER_CA_KEY, NULL, NULL);
		set_ovpn_key(type, unit, OVPN_SERVER_CLIENT_CERT, NULL, NULL);
		set_ovpn_key(type, unit, OVPN_SERVER_CLIENT_KEY, NULL, NULL);
		set_ovpn_key(type, unit, OVPN_SERVER_DH, NULL, NULL);
		set_ovpn_key(type, unit, OVPN_SERVER_STATIC, NULL, NULL);
		set_ovpn_key(type, unit, OVPN_SERVER_CA, NULL, NULL);
		set_ovpn_key(type, unit, OVPN_SERVER_EXTRA, NULL, NULL);
		set_ovpn_key(type, unit, OVPN_SERVER_CERT, NULL, NULL);
		set_ovpn_key(type, unit, OVPN_SERVER_KEY, NULL, NULL);
		set_ovpn_key(type, unit, OVPN_SERVER_CRL, NULL, NULL);
	} else {
		set_ovpn_key(type, unit, OVPN_CLIENT_STATIC, NULL, NULL);
		set_ovpn_key(type, unit, OVPN_CLIENT_CA, NULL, NULL);
		set_ovpn_key(type, unit, OVPN_CLIENT_CERT, NULL, NULL);
		set_ovpn_key(type, unit, OVPN_CLIENT_KEY, NULL, NULL);
		set_ovpn_key(type, unit, OVPN_CLIENT_CRL, NULL, NULL);
		set_ovpn_key(type, unit, OVPN_CLIENT_EXTRA, NULL, NULL);
	}

	nvram_commit();
}


void update_ovpn_status(ovpn_type_t type, int unit, ovpn_status_t status_type, ovpn_errno_t err_no)
{
	char varname[32];

	sprintf(varname, "vpn_%s%d_state", (type == OVPN_TYPE_SERVER ? "server" : "client"), unit);
	nvram_set_int(varname, status_type);
        sprintf(varname, "vpn_%s%d_errno", (type == OVPN_TYPE_SERVER ? "server" : "client"), unit);
        nvram_set_int(varname, err_no);

	if (type == OVPN_TYPE_SERVER && (status_type == OVPN_STS_INIT || status_type == OVPN_STS_STOP)) {
		sprintf(varname, "vpn_server%d_rip", unit);
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


/* Get filename for cert/key storage in jffs */
char *get_ovpn_filename(ovpn_type_t type, int unit, ovpn_key_t key_type, char *buf, size_t buf_len)
{
	char *typeStr, *keyStr;

	switch (type) {
		case OVPN_TYPE_SERVER:
			typeStr = "server";
			break;
		case OVPN_TYPE_CLIENT:
			typeStr = "client";
			break;
		default:
			buf[0] = '\0';
			return buf;
	}

	switch (key_type) {
		case OVPN_CLIENT_STATIC:
		case OVPN_SERVER_STATIC:
			keyStr = "static";
			break;
		case OVPN_CLIENT_CA:
		case OVPN_SERVER_CA:
			keyStr = "ca";
			break;
		case OVPN_CLIENT_CERT:
		case OVPN_SERVER_CERT:
			keyStr = "crt";
			break;
		case OVPN_CLIENT_KEY:
		case OVPN_SERVER_KEY:
			keyStr = "key";
			break;
		case OVPN_CLIENT_CRL:
		case OVPN_SERVER_CRL:
			keyStr = "crl";
			break;
		case OVPN_SERVER_CA_KEY:
			keyStr = "ca_key";
			break;
		case OVPN_SERVER_DH:
			keyStr = "dh";
			break;
		case OVPN_SERVER_CLIENT_CERT:
			keyStr = "client_crt";
			break;
		case OVPN_SERVER_CLIENT_KEY:
			keyStr = "client_key";
			break;
		case OVPN_CLIENT_EXTRA:
		case OVPN_SERVER_EXTRA:
			keyStr = "extra";
			break;
		default:
			buf[0] = '\0';
			return buf;
	}

	snprintf(buf, buf_len, "vpn_crt_%s%d_%s", typeStr, unit, keyStr);
	return buf;
}


char *get_ovpn_key(ovpn_type_t type, int unit, ovpn_key_t key_type, char *buf, size_t len)
{
	char varname[64];
	get_ovpn_filename(type, unit, key_type, varname, sizeof (varname));

	if (strlen(varname)) {
		return get_parsed_crt(varname, buf, len);
	} else {
		buf[0] = '\0';
		return buf;
	}
}


char *get_parsed_crt(const char *name, char *buf, size_t buf_len)
{
	int datalen;
	char filename[128];

	snprintf(filename, sizeof(filename), "%s/%s", OVPN_FS_PATH, name);

	datalen = f_read(filename, buf, buf_len-1);
	if (datalen < 0) {
		buf[0] = '\0';
	} else {
		buf[datalen] = '\0';
	}

	return buf;
}


int set_ovpn_key(ovpn_type_t type, int unit, ovpn_key_t key_type, char *buf, char *path)
{
	char varname[64];
	char filename[128];
	char *data;
	FILE *fp;

	get_ovpn_filename(type, unit, key_type, varname, sizeof (varname));
	snprintf(filename, sizeof(filename), "%s/%s", OVPN_FS_PATH, varname);

	if (path) {
		return _set_crt_parsed(varname, path);
	} else if ((!buf) || (!*buf)) {
		unlink(filename);
		return -1;
	}

	if ((key_type == OVPN_SERVER_DH) && !strncmp(buf, "none", 4)) {
		data = buf;
	} else {
		data = strstr(buf, PEM_START_TAG);
		if (!data) return -1;
	}

	if(!d_exists(OVPN_FS_PATH))
		mkdir(OVPN_FS_PATH, S_IRWXU);

	fp = fopen(filename, "w");
	if(fp) {
		chmod(filename, S_IRUSR|S_IWUSR);
		fwrite(data, 1, strlen(data), fp);
		fclose(fp);
	}

	return 0;
}

int _set_crt_parsed(const char *name, char *file_path)
{
	char target_file_path[128] ={0};
	char *buffer, *data;
	int result;

	if(!d_exists(OVPN_FS_PATH))
		mkdir(OVPN_FS_PATH, S_IRWXU);

	if(f_exists(file_path)) {
		snprintf(target_file_path, sizeof(target_file_path), "%s/%s", OVPN_FS_PATH, name);

		buffer = read_whole_file(file_path);
		if (!buffer) return -1;

		data = strstr(buffer, PEM_START_TAG);
		if (data) {
			result = f_write(target_file_path, data, strlen(data), 0, S_IRUSR|S_IWUSR);
		} else {
			result = -1;
		}
		free(buffer);
		return result;
	}
	else {
		return -1;
	}
}


int ovpn_key_exists(ovpn_type_t type, int unit, ovpn_key_t key_type)
{
	char varname[64];

	get_ovpn_filename(type, unit, key_type, varname, sizeof (varname));

	if (ovpn_crt_is_empty(varname))
		return 0;
	else
		return 1;
}


int ovpn_crt_is_empty(const char *name)
{
	char file_path[128] ={0};
	struct stat st;

	//check file
	if(d_exists(OVPN_FS_PATH)) {
		snprintf(file_path, sizeof(file_path), "%s/%s", OVPN_FS_PATH, name);
		if(stat(file_path, &st) == 0) {
			if( !S_ISDIR(st.st_mode) && st.st_size ) {
				return 0;
			}
			else {
				return 1;
			}
		}
		else {
			return 1;
		}
	}
	else {
		mkdir(OVPN_FS_PATH, S_IRWXU);
	}

	return 1;
}


char *get_ovpn_custom(ovpn_type_t type, int unit, char* buffer, int bufferlen)
{
	char varname[32];
	char *nvcontent;
	char *typeStr;
	int datalen, declen;

	buffer[0] = '\0';

	switch (type) {
		case OVPN_TYPE_SERVER:
			typeStr = "server";
			break;
		case OVPN_TYPE_CLIENT:
			typeStr = "client";
			break;
                default:
			return buffer;
        }

	snprintf(varname, sizeof (varname), "vpn_%s%d_cust2", typeStr, unit);

#ifdef HND_ROUTER
	nvcontent = malloc(255 * 3 + 1);
	if (nvcontent)
		nvram_split_get(varname, nvcontent, 255 * 3 + 1, 2);
	else
		return buffer;
#else
	nvcontent = strdup(nvram_safe_get(varname));
#endif
	datalen = strlen(nvcontent);

	if (datalen) {
		if (datalen >= bufferlen) datalen = bufferlen-1;
		declen = base64_decode(nvcontent, (unsigned char *) buffer, datalen);
		buffer[declen] = '\0';
	}

	free(nvcontent);
	return buffer;
}


int set_ovpn_custom(ovpn_type_t type, int unit, char* buffer)
{
	char varname[32];
	char *encbuffer;
	char *typeStr;
	int datalen, enclen;

	switch (type) {
		case OVPN_TYPE_SERVER:
			typeStr = "server";
			break;
		case OVPN_TYPE_CLIENT:
			typeStr = "client";
			break;
                default:
			return -1;
        }

	datalen = strlen(buffer);

	if (datalen) {
		encbuffer = malloc(base64_encoded_len(datalen) + 1);
		if (encbuffer) {
			enclen = base64_encode((unsigned char*)buffer, encbuffer, datalen);
			encbuffer[enclen] = '\0';

			snprintf(varname, sizeof (varname), "vpn_%s%d_cust2", typeStr, unit);

#ifdef HND_ROUTER
			nvram_split_set(varname, encbuffer, 255, 2);
#else
			nvram_set(varname, encbuffer);
#endif

			free(encbuffer);
			return 0;
		}
	}

	return -1;
}

// Determine how to handle dnsmasq server list based on
// highest active dnsmode
int get_max_dnsmode() {
	int unit, maxlevel = 0, level;
	char filename[40];
	char varname[32];

	for( unit = 1; unit <= OVPN_CLIENT_MAX; unit++ ) {
		sprintf(filename, "/etc/openvpn/dns/client%d.resolv", unit);
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


void write_ovpn_resolv_dnsmasq(FILE* dnsmasq_conf) {
	int unit;
	char filename[40], prefix[16];
	char *buffer;

	for (unit = 1; unit <= OVPN_CLIENT_MAX; unit++) {
		sprintf(filename, "/etc/openvpn/dns/client%d.resolv", unit);
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
			sprintf(filename, "/etc/openvpn/dns/client%d.resolv", unit);
			if (f_exists(filename)) {
				snprintf(varname, sizeof(varname), "vpn_client%d_adns", unit);
				if (nvram_get_int(varname) == OVPN_DNSMODE_STRICT) {
					fprintf(dnsmasq_conf, "strict-order\n");
					modeset = 1;
				}
			}

		}

		// Add WINS entries if any client provides it
		sprintf(filename, "/etc/openvpn/dns/client%d.conf", unit);
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


int ovpn_write_key(ovpn_type_t type, int unit, ovpn_key_t key_type) {
	FILE *fp;
	char buffer[8192];

	if (ovpn_key_exists(type, unit, key_type)) {
		fp = fopen(ovpn_get_runtime_filename(type, unit, key_type, buffer, sizeof(buffer)), "w");
		if (!fp)
			return -1;
		chmod(buffer, S_IRUSR|S_IWUSR);
		fprintf(fp, "%s", get_ovpn_key(type, unit, key_type, buffer, sizeof(buffer)));
		fclose(fp);
		return 0;
	}
	return -1;
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
			filename = "client.crt";
			break;
		case OVPN_SERVER_CERT:
			filename = "server.crt";
			break;
		case OVPN_CLIENT_KEY:
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

ovpn_cconf_t *ovpn_get_cconf(int unit) {
	ovpn_cconf_t *cconf;
	char prefix[16], buffer[10000];

	cconf = malloc(sizeof (ovpn_cconf_t));
	if (!cconf) return NULL;


	snprintf(prefix, sizeof(prefix), "vpn_client%d_", unit);

	strlcpy(buffer, nvram_pf_safe_get(prefix, "if"), sizeof (buffer));
	if (!strcmp(buffer, "tap"))
		cconf->if_type = OVPN_IF_TAP;
	else
		cconf->if_type = OVPN_IF_TUN;
	snprintf(cconf->if_name, sizeof (cconf->if_name), "%s%d", buffer, unit + OVPN_CLIENT_BASEIF);

	if (!strcmp(nvram_pf_safe_get(prefix, "crypt"), "secret"))
		cconf->auth_mode = OVPN_AUTH_STATIC;
	else
		cconf->auth_mode = OVPN_AUTH_TLS;

	cconf->bridge = nvram_pf_get_int(prefix, "bridge");
	cconf->nat = nvram_pf_get_int(prefix, "nat");

	cconf->userauth = nvram_pf_get_int(prefix, "userauth");
	cconf->useronly = nvram_pf_get_int(prefix, "useronly");

	strlcpy(cconf->proto, nvram_pf_safe_get(prefix, "proto"), sizeof(cconf->proto));
	strlcpy(cconf->addr,  nvram_pf_safe_get(prefix, "addr"), sizeof(cconf->addr));
	cconf->port = nvram_pf_get_int(prefix, "port");

	strlcpy(cconf->local, nvram_pf_safe_get(prefix, "local"), sizeof(cconf->local));
	strlcpy(cconf->remote, nvram_pf_safe_get(prefix, "remote"), sizeof(cconf->remote));
	strlcpy(cconf->netmask, nvram_pf_safe_get(prefix, "nm"), sizeof(cconf->netmask));

	cconf->retry = nvram_pf_get_int(prefix, "connretry");

	strlcpy(cconf->comp, nvram_pf_safe_get(prefix, "comp"), sizeof (cconf->comp));

	cconf->ncp = nvram_pf_get_int(prefix, "ncp_enable");
	strlcpy(cconf->ncp_ciphers, nvram_pf_safe_get(prefix, "ncp_ciphers"), sizeof (cconf->ncp_ciphers));
	strlcpy(cconf->cipher, nvram_pf_safe_get(prefix, "cipher"), sizeof(cconf->cipher));

	strlcpy(cconf->digest, nvram_pf_safe_get(prefix, "digest"), sizeof(cconf->digest));

	cconf->redirect_gateway = nvram_pf_get_int(prefix, "rgw");
	strlcpy(cconf->gateway, nvram_pf_safe_get(prefix, "gw"), sizeof(cconf->gateway));

	cconf->verb = nvram_pf_get_int(prefix, "verb");

	cconf->reneg = nvram_pf_get_int(prefix, "reneg");

	cconf->direction = nvram_pf_get_int(prefix, "hmac");
	cconf->tlscrypt = (cconf->direction == 3 ? 1 : 0);
	cconf->verify_x509_type = nvram_pf_get_int(prefix, "tlsremote");
	strlcpy(cconf->verify_x509_name, nvram_pf_safe_get(prefix, "cn"), sizeof(cconf->verify_x509_name));

	cconf->adns = nvram_pf_get_int(prefix, "adns");

	strlcpy(cconf->username, nvram_pf_safe_get(prefix, "username"), sizeof(cconf->username));
	strlcpy(cconf->password, nvram_pf_safe_get(prefix, "password"), sizeof(cconf->password));

	cconf->fw = nvram_pf_get_int(prefix, "fw");

	strlcpy(cconf->custom, get_ovpn_custom(OVPN_TYPE_CLIENT, unit, buffer, sizeof (buffer)), sizeof(cconf->custom));

	return cconf;
}


ovpn_sconf_t *ovpn_get_sconf(int unit){
	ovpn_sconf_t *sconf;
	char prefix[16], buffer[10000];

	sconf = malloc(sizeof (ovpn_sconf_t));
	if (!sconf) return NULL;


	snprintf(prefix, sizeof(prefix), "vpn_server%d_", unit);

	// Determine interface
	strlcpy(buffer, nvram_pf_safe_get(prefix, "if"), sizeof (buffer));

	if (!strcmp(buffer, "tap"))
		sconf->if_type = OVPN_IF_TAP;
	else
		sconf->if_type = OVPN_IF_TUN;

	snprintf(sconf->if_name, sizeof (sconf->if_name), "%s%d", buffer, unit + OVPN_SERVER_BASEIF);

	if (!strcmp(nvram_pf_safe_get(prefix, "crypt"), "secret"))
		sconf->auth_mode = OVPN_AUTH_STATIC;
	else
		sconf->auth_mode = OVPN_AUTH_TLS;


	strlcpy(sconf->network, nvram_pf_safe_get(prefix, "sn"), sizeof(sconf->network));
	strlcpy(sconf->netmask,	nvram_pf_safe_get(prefix, "nm"), sizeof(sconf->netmask));
	sconf->dhcp = nvram_pf_get_int(prefix, "dhcp");
	strlcpy(sconf->pool_start, nvram_pf_safe_get(prefix, "r1"), sizeof(sconf->pool_start));
	strlcpy(sconf->pool_end, nvram_pf_safe_get(prefix, "r2"), sizeof(sconf->pool_end));
	strlcpy(sconf->local, nvram_pf_safe_get(prefix, "local"), sizeof(sconf->local));
	strlcpy(sconf->remote, nvram_pf_safe_get(prefix, "remote"), sizeof(sconf->remote));

	strlcpy(sconf->proto, nvram_pf_safe_get(prefix, "proto"), sizeof (sconf->proto));
	sconf->port = nvram_pf_get_int(prefix, "port");

	sconf->ncp = nvram_pf_get_int(prefix, "ncp_enable");
	strlcpy(sconf->ncp_ciphers, nvram_pf_safe_get(prefix, "ncp_ciphers"), sizeof (sconf->ncp_ciphers));
	strlcpy(sconf->cipher, nvram_pf_safe_get(prefix, "cipher"), sizeof (sconf->cipher));
	strlcpy(sconf->digest, nvram_pf_safe_get(prefix, "digest"), sizeof (sconf->digest));

	strlcpy(sconf->comp, nvram_pf_safe_get(prefix, "comp"), sizeof (sconf->comp));

	sconf->verb = nvram_pf_get_int(prefix, "verb");

	sconf->push_lan = nvram_pf_get_int(prefix, "client_access");

	strlcpy(sconf->lan_ipaddr, nvram_safe_get("lan_ipaddr"), sizeof(sconf->lan_ipaddr));
	strlcpy(sconf->lan_netmask, nvram_safe_get("lan_netmask"), sizeof(sconf->lan_netmask));

	sconf->ccd = nvram_pf_get_int(prefix, "ccd");
	sconf->c2c = nvram_pf_get_int(prefix, "c2c");
	sconf->ccd_excl = nvram_pf_get_int(prefix, "ccd_excl");
	strlcpy(sconf->ccd_val, nvram_pf_safe_get(prefix, "ccd_val"), sizeof(sconf->ccd_val));

	sconf->push_dns = nvram_pf_get_int(prefix, "pdns");

	sconf->direction = nvram_pf_get_int(prefix, "hmac");
	sconf->tlscrypt = (sconf->direction == 3 ? 1 : 0);

	sconf->userauth = nvram_pf_get_int(prefix, "userpass_auth");
	sconf->useronly = nvram_pf_get_int(prefix, "igncrt");

	sconf->tls_keysize = nvram_pf_get_int(prefix, "tls_keysize");

	strlcpy(sconf->custom, get_ovpn_custom(OVPN_TYPE_SERVER, unit, buffer, sizeof (buffer)), sizeof(sconf->custom));

	return sconf;
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
		if ((sconf->ncp > 0) && (*sconf->ncp_ciphers)) {
			fprintf(fp, "ncp-ciphers %s\n", sconf->ncp_ciphers);
			fprintf(fp_client, "ncp-ciphers %s\n", sconf->ncp_ciphers);
		} else {
			fprintf(fp, "ncp-disable\n");
		}
	}
	if ((sconf->ncp < 2) ||
	    (sconf->ncp == 2 && !*sconf->ncp_ciphers) ||
	    (sconf->auth_mode == OVPN_AUTH_STATIC)) {
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
			fprintf(fp_client, "comp-lzo yes\n");
		} else if (!strcmp(sconf->comp, "adaptive")) {
			fprintf(fp, "comp-lzo adaptive\n");
			fprintf(fp_client, "comp-lzo adaptive\n");
		} else if (!strcmp(sconf->comp, "no")) {
			fprintf(fp, "compress\n");	// Disable, but client can override if desired
			fprintf(fp_client, "comp-lzo no\n");
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
			else
				fprintf(fp, "duplicate-cn\n");

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

				fp_ccd = fopen(cname, "w");
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

	sprintf(buffer, "/etc/openvpn/server%d/updown.sh", unit);
	symlink("/usr/sbin/updown-server.sh", buffer);
	fprintf(fp, "script-security 2\n");
	fprintf(fp, "up updown.sh\n");
	fprintf(fp, "down updown.sh\n");

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
		}
	}

	// cipher
	if (cconf->auth_mode == OVPN_AUTH_TLS) {
		if ((cconf->ncp > 0) && (*cconf->ncp_ciphers))
			fprintf(fp, "ncp-ciphers %s\n", cconf->ncp_ciphers);
		else
			fprintf(fp, "ncp-disable\n");
	}
	if ((cconf->ncp < 2) ||
	    (cconf->ncp == 2 && !*cconf->ncp_ciphers) ||
	    (cconf->auth_mode == OVPN_AUTH_STATIC)) {
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
			if (cconf->tlscrypt)
				fprintf(fp, "tls-crypt static.key\n");
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

	sprintf(buffer, "/etc/openvpn/client%d/updown.sh", unit);
	symlink("/usr/sbin/updown-client.sh", buffer);
	fprintf(fp, "up updown.sh\n");
	fprintf(fp, "down updown.sh\n");

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


void ovpn_write_server_keys(ovpn_sconf_t *sconf, int unit, int valid_client_cert) {
	char buffer[64], buffer2[8000];
	FILE *fp, *fp_client;

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
#ifdef RTCONFIG_OPENSSL11
				fprintf(fp, "export OPENSSL=\"openssl11\"\n");
#else
				fprintf(fp, "export OPENSSL=\"openssl\"\n");
#endif
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

		// Client key and certificate - if not using user-only authentication
		if (!(sconf->userauth && sconf->useronly)) {
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
			eval("openvpn", "--genkey", "--secret", buffer);
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
		fprintf(fp, "iptables -t nat -I POSTROUTING -s %s/%s -o %s -j MASQUERADE\n",
		             inet_ntoa(netaddr), nvram_safe_get("lan_netmask"), cconf->if_name);
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

	sprintf(buffer, "/etc/openvpn/server%d/vpns-watchdog%d.sh", unit, unit);

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
