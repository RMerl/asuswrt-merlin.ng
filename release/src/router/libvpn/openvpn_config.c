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
	char *value;
	int datalen, i;
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	FILE *fp;
	char filename[128];
	char *p = buf;
#endif
	char tmpBuf[256] = {0};


	value = nvram_safe_get(name);
	datalen = strlen(value);

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	if(!d_exists(OVPN_FS_PATH))
		mkdir(OVPN_FS_PATH, S_IRWXU);
	snprintf(filename, sizeof(filename), "%s/%s", OVPN_FS_PATH, name);
#endif

	if (datalen) {
		for (i=0; (i < (datalen-1) && i < (buf_len-1)); i++) {
			if (value[i] == '>')
				buf[i] = '\n';
			else
				buf[i] = value[i];
		}
		buf[i] = '\0';

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
		//save to file and then clear nvram value
		fp = fopen(filename, "w");
		if(fp) {
			chmod(filename, S_IRUSR|S_IWUSR);
			fprintf(fp, "%s", buf);
			fclose(fp);
			nvram_set(name, "");
		}
#endif
	}
	else {
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
		datalen = 0;
		fp = fopen(filename, "r");
		if(fp) {
			while(fgets(buf, buf_len, fp)) {
				if(!strncmp(buf, PEM_START_TAG, strlen(PEM_START_TAG)) || !strncmp(buf, "none", 4))
					break;
			}
			if(feof(fp) &&  strncmp(buf, "none", 4)) {
				fclose(fp);
				buf[0] = '\0';
				return buf;
			}
			p += strlen(buf);
			memset(tmpBuf, 0, sizeof(tmpBuf));
			while(fgets(tmpBuf, sizeof(tmpBuf), fp)) {
				datalen = strlen(tmpBuf);
				if (p + datalen < buf + buf_len + 1) {
					strcpy(p, tmpBuf);
					p += datalen;
				}
			}
			fclose(fp);
		}
#endif
		*p = '\0';
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

	data = strstr(buf, PEM_START_TAG);
	if (!data) return -1;

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	snprintf(filename, sizeof(filename), "%s/%s", OVPN_FS_PATH, varname);
	fp = fopen(filename, "w");
	if(fp) {
		chmod(filename, S_IRUSR|S_IWUSR);
		fprintf(fp, "%s", data);
		fclose(fp);
	}
	else
#endif
	nvram_set(varname, data);

	return 0;
}

int _set_crt_parsed(const char *name, char *file_path)
{
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	char target_file_path[128] ={0};

	if(!d_exists(OVPN_FS_PATH))
		mkdir(OVPN_FS_PATH, S_IRWXU);

	if(f_exists(file_path)) {
		snprintf(target_file_path, sizeof(target_file_path), "%s/%s", OVPN_FS_PATH, name);
		return eval("cp", file_path, target_file_path);
	}
	else {
		return -1;
	}
#else
	FILE *fp=fopen(file_path, "r");
	char buffer[4000] = {0};
	char buffer2[256] = {0};
	char *p = buffer;

// TODO: Ensure that Asus's routine can handle CRLF too, otherwise revert to
//       the code we currently use in httpd.

	if(fp) {
		while(fgets(buffer, sizeof(buffer), fp)) {
			if(!strncmp(buffer, PEM_START_TAG, strlen(PEM_START_TAG)))
				break;
		}
		if(feof(fp)) {
			fclose(fp);
			return -EINVAL;
		}
		p += strlen(buffer);
		//if( *(p-1) == '\n' )
			//*(p-1) = '>';
		while(fgets(buffer2, sizeof(buffer2), fp)) {
			strncpy(p, buffer2, strlen(buffer2));
			p += strlen(buffer2);
			//if( *(p-1) == '\n' )
				//*(p-1) = '>';
		}
		*p = '\0';
		nvram_set(name, buffer);
		fclose(fp);
		return 0;
	}
	else
		return -ENOENT;
#endif
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

	if( nvram_is_empty(name) ) {
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
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
			return 1;
		}
#else
		return 1;
#endif
	}
	else {
		return 0;
	}
}
