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

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
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

	vpnlog(VPN_LOG_EXTRA, "Adding DNS entries...");

	for (unit = 1; unit <= OVPN_CLIENT_MAX; unit++) {
		sprintf(filename, "/etc/openvpn/dns/client%d.resolv", unit);
		if (f_exists(filename)) {
			sprintf(prefix, "vpn_client%d_", unit);

			// Don't add servers if policy routing is enabled and dns mode set to "Exclusive"
			// Handled by iptables on a case-by-case basis
			if ((nvram_pf_get_int(prefix, "rgw") >= OVPN_RGW_POLICY ) && (nvram_pf_get_int(prefix, "adns") == OVPN_DNSMODE_EXCLUSIVE))
				continue;

			vpnlog(VPN_LOG_INFO,"Adding DNS entries from %s", filename);

			buffer = read_whole_file(filename);
			if (buffer) {
				fwrite(buffer, 1, strlen(buffer),dnsmasq_conf);
				free(buffer);
			}
		}
	}
	vpnlog(VPN_LOG_EXTRA, "Done with DNS entries...");
}


void write_ovpn_dnsmasq_config(FILE* dnsmasq_conf) {
	char prefix[16], filename[40], varname[32];
	int unit, modeset = 0;
	char *buffer;

	// Add interfaces for servers that provide DNS services
	for (unit = 1; unit <= OVPN_SERVER_MAX; unit++) {
		sprintf(prefix, "vpn_server%d_", unit);
		if (nvram_pf_get_int(prefix, "pdns") ) {
			vpnlog(VPN_LOG_EXTRA, "Adding server %d interface to dns config", unit);
			fprintf(dnsmasq_conf, "interface=%s%d\n", nvram_pf_safe_get(prefix, "if"), OVPN_SERVER_BASEIF + unit);
		}
	}

	for (unit = 1; unit <= OVPN_CLIENT_MAX; unit++) {
		// Add strict-order if any client is set to "strict" and we haven't done so yet
		if (!modeset) {
			sprintf(filename, "/etc/openvpn/dns/client%d.resolv", unit);
			if (f_exists(filename)) {
				vpnlog(VPN_LOG_EXTRA, "Checking ADNS settings for client %d", unit);
				snprintf(varname, sizeof(varname), "vpn_client%d_adns", unit);
				if (nvram_get_int(varname) == OVPN_DNSMODE_STRICT) {
					vpnlog(VPN_LOG_INFO, "Adding strict-order to dnsmasq config for client %d", unit);
					fprintf(dnsmasq_conf, "strict-order\n");
					modeset = 1;
				}
			}

		}

		// Add WINS entries if any client provides it
		sprintf(filename, "/etc/openvpn/dns/client%d.conf", unit);
		if (f_exists(filename)) {
			vpnlog(VPN_LOG_INFO, "Adding Dnsmasq config from %s", filename);
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

