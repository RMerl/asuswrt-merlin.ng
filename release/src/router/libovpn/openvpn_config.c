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
#include "openvpn_control.h"
#include "openvpn_setup.h"

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

	strlcpy(cconf->ncp_ciphers, nvram_pf_safe_get(prefix, "ncp_ciphers"), sizeof (cconf->ncp_ciphers));
	strlcpy(cconf->cipher, nvram_pf_safe_get(prefix, "cipher"), sizeof(cconf->cipher));

	strlcpy(cconf->digest, nvram_pf_safe_get(prefix, "digest"), sizeof(cconf->digest));

	cconf->redirect_gateway = nvram_pf_get_int(prefix, "rgw");
	strlcpy(cconf->gateway, nvram_pf_safe_get(prefix, "gw"), sizeof(cconf->gateway));

	cconf->verb = nvram_pf_get_int(prefix, "verb");

	cconf->reneg = nvram_pf_get_int(prefix, "reneg");

	cconf->direction = nvram_pf_get_int(prefix, "hmac");
	switch (cconf->direction) {
		case 3:
			cconf->tlscrypt = 1;
			break;
		case 4:
			cconf->tlscrypt = 2;
			break;
		default:
			cconf->tlscrypt = 0;
	}

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
