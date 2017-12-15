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


void reset_ovpn_setting(ovpn_type_t type, int unit){
        struct nvram_tuple *t;
        char prefix[]="vpn_serverX_", tmp[100];
	char service[7];

	if (type == OVPN_TYPE_SERVER)
		strcpy(service, "server");
	else if (type == OVPN_TYPE_CLIENT)
		strcpy(service, "client");
	else
		return;

	logmessage("openvpn","Resetting %s (unit %d) to default settings", service, unit);

	snprintf(prefix, sizeof(prefix), "vpn_%s%d_", service, unit);

	for (t = router_defaults; t->name; t++) {
		if (strncmp(t->name, prefix, 12)==0) {
			nvram_set(t->name, t->value);
		}
	}

#if 0	// Rename
	logmessage("openvpn", "Preserving backup of key/certs as .old files");
	if (type == OVPN_TYPE_SERVER)	// server-only files
	{
		sprintf(tmp, "mv %s/vpn_crt_%s%d_ca_key %s/vpn_crt_%s%d_ca_key.old", OVPN_FS_PATH, service, unit, service, unit);
		system(tmp);
		sprintf(tmp, "mv %s/vpn_crt_%s%d_client_crt %s/vpn_crt_%s%d_client_crt.old", OVPN_FS_PATH, service, unit, service, unit);
		system(tmp);
		sprintf(tmp, "mv %s/vpn_crt_%s%d_client_key %s/vpn_crt_%s%d_client_key.old", OVPN_FS_PATH, service, unit, service, unit);
		system(tmp);
		sprintf(tmp, "mv %s/vpn_crt_%s%d_dh %s/vpn_crt_%s%d_dh.old", OVPN_FS_PATH, service, unit, service, unit);
		system(tmp);
	}

	sprintf(tmp, "mv %s/vpn_crt_%s%d_ca %s/vpn_crt_%s%d_ca.old", OVPN_FS_PATH, service, unit, service, unit);
	system(tmp);
	sprintf(tmp, "mv %s/vpn_crt_%s%d_crt %s/vpn_crt_%s%d_crt.old", OVPN_FS_PATH, service, unit, service, unit);
	system(tmp);
	sprintf(tmp, "mv %s/vpn_crt_%s%d_key %s/vpn_crt_%s%d_key.old", OVPN_FS_PATH, service, unit, service, unit);
	system(tmp);
	sprintf(tmp, "mv %s/vpn_crt_%s%d_crl %s/vpn_crt_%s%d_crl.old", OVPN_FS_PATH, service, unit, service, unit);
	system(tmp);
	sprintf(tmp, "mv %s/vpn_crt_%s%d_static %s/vpn_crt_%s%d_static.old", OVPN_FS_PATH, service, unit, service, unit);
	system(tmp);
	sprintf(tmp, "mv %s/vpn_crt_%s%d_extra %s/vpn_crt_%s%d_extra.old", OVPN_FS_PATH, service, unit, service, unit);
	system(tmp);

#else	// Delete
	if (type == OVPN_TYPE_SERVER)  // server-only files
	{
		sprintf(tmp, "%s/vpn_crt_%s%d_ca_key", OVPN_FS_PATH, service, unit);
		unlink(tmp);
		sprintf(tmp, "%s/vpn_crt_%s%d_client_crt", OVPN_FS_PATH, service, unit);
		unlink(tmp);
		sprintf(tmp, "%s/vpn_crt_%s%d_client_key", OVPN_FS_PATH, service, unit);
		unlink(tmp);
		sprintf(tmp, "%s/vpn_crt_%s%d_dh", OVPN_FS_PATH, service, unit);
		unlink(tmp);
	}

	sprintf(tmp, "%s/vpn_crt_%s%d_ca", OVPN_FS_PATH, service, unit);
	unlink(tmp);
	sprintf(tmp, "%s/vpn_crt_%s%d_crt", OVPN_FS_PATH, service, unit);
	unlink(tmp);
	sprintf(tmp, "%s/vpn_crt_%s%d_key", OVPN_FS_PATH, service, unit);
	unlink(tmp);
	sprintf(tmp, "%s/vpn_crt_%s%d_crl", OVPN_FS_PATH, service, unit);
	unlink(tmp);
	sprintf(tmp, "%s/vpn_crt_%s%d_static", OVPN_FS_PATH, service, unit);
	unlink(tmp);
	sprintf(tmp, "%s/vpn_crt_%s%d_extra", OVPN_FS_PATH, service, unit);
	unlink(tmp);
#endif

}


void update_ovpn_status(ovpn_type_t type, int unit, ovpn_status_t status_type)
{
	char varname[32];

	sprintf(varname, "vpn_%s%d_state", (type == OVPN_TYPE_SERVER ? "server" : "client"), unit);
	nvram_set_int(varname, status_type);
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
		case OVPN_CLIENT_CA_EXTRA:
		case OVPN_SERVER_CA_EXTRA:
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

	if (path) {
		return _set_crt_parsed(varname, path);
	} else if (!buf) {
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
	snprintf(filename, sizeof(filename), "%s/%s", OVPN_FS_PATH, varname);
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
			result = f_write(target_file_path, data, strlen(data), NULL, S_IRUSR|S_IWUSR);
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

	snprintf(varname, sizeof (varname), "vpn_%s%d_custom2", typeStr, unit);

#ifdef HND_ROUTER
	nvcontent = malloc(255 * 3 + 1);
	if (nvcontent) nvram_split_get(varname, nvcontent, 255 * 3 + 1, 2);
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

			snprintf(varname, sizeof (varname), "vpn_%s%d_custom2", typeStr, unit);

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
