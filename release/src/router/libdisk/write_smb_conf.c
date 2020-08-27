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
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>		//PATH_MAX, LONG_MIN, LONG_MAX
#include <bcmnvram.h>
#include <shutils.h>
#include <shared.h>

#include "usb_info.h"
#include "disk_initial.h"
#include "disk_share.h"
#include <shared.h>
#include <linux/version.h>

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
#include <PMS_DBAPIs.h>
#endif

#define SAMBA_CONF "/etc/smb.conf"
#define SAMBA_LOG "/var/log/samba.log"

/* @return:
 * 	If mount_point is equal to one of partition of all disks case-insensitivity, return true.
 */
static int check_mount_point_icase(const disk_info_t *d_info, const partition_info_t *p_info, const disk_info_t *disk, const u32 part_nr, const char *m_point)
{
	int v = 0;
	const disk_info_t *d;
	const partition_info_t *p;

	if(!d_info || !p_info || !disk || part_nr > 15 || !m_point || *m_point == '\0')
		return 0;

	for(d = d_info; !v && d != NULL; d = d->next){
		for(p = d->partitions; !v && p != NULL; p = p->next){
			if(!p->mount_point || (d == disk && p->partition_order == part_nr))
				continue;

			if(strcasecmp(p->mount_point, m_point))
				continue;

			v = 1;
		}
	}

	return v;
}

/* For NETBIOS name,
 * 1. NetBIOS names are a sequence of alphanumeric characters.
 * 2. The hyphen ("-") and full-stop (".") characters may also be used
 *     in the NetBIOS name, but not as the first or last character.
 * 3. The NetBIOS name is 16 ASCII characters, however Microsoft limits
 *     the host name to 15 characters and reserves the 16th character
 *     as a NetBIOS Suffix
 */
int
is_valid_netbios_name(const char *name)
{
	int i, valid = 1;
	size_t len;

	if(!name)
		return 0;

	len = strlen(name);
	if(!len || len > 15)
		return 0;

	for(i = 0; valid && i < len; ++i){
		if(isalnum(name[i]))
			continue;
		else if((name[i] == '-' || name[i] == '.') && (i > 0 && i < (len - 1)))
			continue;

		valid = 0;
	}

	return valid;
}

int check_existed_share(const char *string)
{
	FILE *tp;
	char buf[PATH_MAX], target[256];

	if((tp = fopen(SAMBA_CONF, "r")) == NULL)
		return 0;

	if(string == NULL || strlen(string) <= 0)
		return 0;

	snprintf(target, sizeof(target), "[%s]", string);

	memset(buf, 0, sizeof(buf));
	while(fgets(buf, sizeof(buf), tp) != NULL){
		if(strstr(buf, target)){
			fclose(tp);
			return 1;
		}
	}

	fclose(tp);
	return 0;
}

int get_list_strings_count(char **list, int size, char *str)
{
	int i, count = 0;

	for(i = 0; i < size; i++)
		if(strcmp(list[i], str) == 0) count++;
	return count;
}

int main(int argc, char *argv[])
{
	FILE *fp;
	int n=0, spnego = 0;
	char p_computer_name[16]; // computer_name's len is CKN_STR15.
	disk_info_t *follow_disk, *disks_info = NULL;
	partition_info_t *follow_partition;
	char *mount_folder;
	int samba_right;
	int sh_num;
	char **folder_list = NULL;
	int acc_num, first;
#if defined(RTCONFIG_SOC_IPQ8074) || defined(RTCONFIG_SOC_IPQ8064)
	int max_user = 32;
#endif
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
	PMS_ACCOUNT_INFO_T *account_list, *follow_account;
	int group_num;
	PMS_ACCOUNT_GROUP_INFO_T *group_list;
	PMS_OWNED_INFO_T *owned_group;
	PMS_ACCOUNT_GROUP_INFO_T *group_member;
	int samba_right_group;
	char char_user[64];
#else
	char **account_list;
	int i;
#endif
	int dup, same_m_pt = 0;
	char unique_share_name[PATH_MAX];
	int st_samba_mode = nvram_get_int("st_samba_mode");
#if defined(RTCONFIG_SAMBA36X)
	spnego = 1;
#endif

	if (access(SAMBA_CONF, F_OK) == 0)
		unlink(SAMBA_CONF);
	if ((fp = fopen(SAMBA_CONF, "w")) == NULL)
		goto confpage;

	unlink(SAMBA_LOG);

	fprintf(fp, "[global]\n");

	strlcpy(p_computer_name, nvram_safe_get("computer_name"), sizeof(p_computer_name));
	if (*p_computer_name == '\0' || !is_valid_netbios_name(p_computer_name)) {
		strlcpy(p_computer_name, get_lan_hostname(), sizeof(p_computer_name));
		toUpperCase(p_computer_name);
	}
	if (*p_computer_name) {
		fprintf(fp, "netbios name = %s\n", p_computer_name);
		fprintf(fp, "server string = %s\n", get_productid());
	}

	strlcpy(p_computer_name, nvram_safe_get("st_samba_workgroup"), sizeof(p_computer_name));
	if (*p_computer_name == '\0' || !is_valid_netbios_name(p_computer_name)) {
		strlcpy(p_computer_name, nvram_safe_get("lan_domain"), sizeof(p_computer_name));
		*strchrnul(p_computer_name, '.') = '\0';
		toUpperCase(p_computer_name);
	}
	if (*p_computer_name)
		fprintf(fp, "workgroup = %s\n", p_computer_name);

#if defined(RTCONFIG_SAMBA36X)
#if 0
	fprintf(fp, "max protocol = SMB2\n"); /* enable SMB1 & SMB2 simultaneously, rewrite when GUI is ready!! */
	fprintf(fp, "passdb backend = smbpasswd\n");
#endif //0
//#endif
//#if defined(RTCONFIG_SAMBA36X) && defined(RTCONFIG_QCA)
#if defined(RTCONFIG_QCA)
	/* min protocol = SMB2, min protocol = LANMAN2, max protocol = SMB3 ... */
	fprintf(fp, "smb encrypt = disabled\n");
	fprintf(fp, "min receivefile size = 16384\n");
	fprintf(fp, "smb passwd file = /etc/samba/smbpasswd\n");
#endif
//#if defined(RTCONFIG_SAMBA36X)
	fprintf(fp, "username level = 20\n");
#endif

	fprintf(fp, "unix charset = UTF8\n");		// ASUS add
	fprintf(fp, "display charset = UTF8\n");	// ASUS add
	fprintf(fp, "load printers = no\n");	//Andy Chiu, 2017/1/20. Add for Samba printcap issue.
	fprintf(fp, "printing = bsd\n");
	fprintf(fp, "printcap name = /dev/null\n");
	fprintf(fp, "log file = %s\n", SAMBA_LOG);
	fprintf(fp, "log level = 0\n");
	fprintf(fp, "max log size = 5\n");

	// account mode
	if(st_samba_mode == 2 || st_samba_mode == 4
			|| (st_samba_mode == 1 && nvram_get("st_samba_force_mode") == NULL)
			){
		fprintf(fp, "security = USER\n");
		fprintf(fp, "guest ok = no\n");
		fprintf(fp, "map to guest = Bad User\n");
	}
	// share mode
	else if(st_samba_mode == 1 || st_samba_mode == 3){
//#if defined(RTCONFIG_SAMBA3) && defined(RTCONFIG_SAMBA36X)
#if defined(RTCONFIG_SAMBA36X)
		fprintf(fp, "auth methods = guest\n");
		fprintf(fp, "guest account = %s\n", nvram_get("http_username")? : "admin");
		fprintf(fp, "map to guest = Bad Password\n");
		fprintf(fp, "guest ok = yes\n");
#else
		fprintf(fp, "security = SHARE\n");
		fprintf(fp, "guest only = yes\n");
#endif
	}
	else{
		usb_dbg("samba mode: no\n");
		goto confpage;
	}

	fprintf(fp, "encrypt passwords = yes\n");
	fprintf(fp, "pam password change = no\n");
	fprintf(fp, "null passwords = yes\n");		// ASUS add

	fprintf(fp, "force directory mode = 0777\n");
	fprintf(fp, "force create mode = 0777\n");

	/* max users */
#if defined(RTCONFIG_SOC_IPQ8074) || defined(RTCONFIG_SOC_IPQ8064)
	if (nvram_get_int("st_max_user") > max_user)
		max_user = nvram_get_int("st_max_user");
	fprintf(fp, "max connections = %d\n", max_user);
#else
	if(strcmp(nvram_safe_get("st_max_user"), "") != 0){
		fprintf(fp, "max connections = %s\n", nvram_safe_get("st_max_user"));
	}
#endif

	if(!nvram_get_int("stop_samba_speedup")){
#if defined(RTCONFIG_SAMBA36X) || defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
		fprintf(fp, "socket options = IPTOS_LOWDELAY TCP_NODELAY SO_KEEPALIVE\n");
#elif defined(RTCONFIG_ALPINE)
		fprintf(fp, "socket options = TCP_NODELAY IPTOS_LOWDELAY IPTOS_THROUGHPUT SO_RCVBUF=5048576 SO_SNDBUF=5048576\n");
#elif defined(RTCONFIG_BCMARM)
#ifdef RTCONFIG_BCM_7114
		fprintf(fp, "socket options = IPTOS_LOWDELAY TCP_NODELAY SO_RCVBUF=131072 SO_SNDBUF=131072\n");
#endif
#else
		fprintf(fp, "socket options = TCP_NODELAY SO_KEEPALIVE SO_RCVBUF=65536 SO_SNDBUF=65536\n");
#endif
	}
	fprintf(fp, "obey pam restrictions = no\n");
	fprintf(fp, "use spnego = %s\n", spnego? "yes" : "no");		// ASUS add
	fprintf(fp, "client use spnego = no\n");	// ASUS add
//	fprintf(fp, "client use spnego = yes\n");	// ASUS add
	fprintf(fp, "disable spoolss = yes\n");		// ASUS add
	fprintf(fp, "host msdfs = no\n");		// ASUS add
	fprintf(fp, "strict allocate = no\n");		// ASUS add
//	fprintf(fp, "mangling method = hash2\n");	// ASUS add
	fprintf(fp, "wide links = no\n"); 		// ASUS add
	fprintf(fp, "bind interfaces only = yes\n");	// ASUS add
	fprintf(fp, "interfaces = lo br0 %s/%s %s\n", nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"), (is_routing_enabled() && nvram_get_int("smbd_wanac")) ? nvram_safe_get("wan0_ifname") : "");
#if 0
#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD) || defined(RTCONFIG_OPENVPN)
	int ip[5];
	char pptpd_subnet[16];
	char openvpn_subnet[32];

	memset(pptpd_subnet, 0, sizeof(pptpd_subnet));
	memset(openvpn_subnet, 0, sizeof(openvpn_subnet));
	if (is_routing_enabled()) {
#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
		if (nvram_get_int("pptpd_enable")) {
			sscanf(nvram_safe_get("pptpd_clients"), "%d.%d.%d.%d-%d", &ip[0], &ip[1], &ip[2], &ip[3], &ip[4]);
			snprintf(pptpd_subnet, sizeof(pptpd_subnet), "%d.%d.%d.", ip[0], ip[1], ip[2]);
		}
#endif
#ifdef RTCONFIG_OPENVPN
		if (nvram_get_int("VPNServer_enable") && strstr(nvram_safe_get("vpn_server1_if"), "tun") && nvram_get_int("vpn_server1_plan"))
			snprintf(openvpn_subnet, sizeof(openvpn_subnet), "%s/%s", nvram_safe_get("vpn_server1_sn"), nvram_safe_get("vpn_server1_nm"));
#endif
	}
	fprintf(fp, "hosts allow = 127.0.0.1 %s/%s %s %s\n", nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"), pptpd_subnet, openvpn_subnet);
#else
	fprintf(fp, "hosts allow = 127.0.0.1 %s/%s\n", nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));
#endif
	fprintf(fp, "hosts deny = 0.0.0.0/0\n");
#endif //#if 0
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
	fprintf(fp, "use sendfile = no\n");
#else
	fprintf(fp, "use sendfile = yes\n");
#endif

#ifdef RTCONFIG_RECVFILE
	if(!nvram_get_int("stop_samba_recv")
#if defined(RTCONFIG_SAMBA36X)
			&& 0
#endif
			)
		fprintf(fp, "use recvfile = yes\n");
#endif

	fprintf(fp, "map archive = no\n");
	fprintf(fp, "map hidden = no\n");
	fprintf(fp, "map read only = no\n");
	fprintf(fp, "map system = no\n");
#ifdef RTCONFIG_SAMBA36X
	fprintf(fp, "store dos attributes = no\n");
#else
	fprintf(fp, "store dos attributes = yes\n");
#endif
	fprintf(fp, "dos filemode = yes\n");
	fprintf(fp, "oplocks = yes\n");
	fprintf(fp, "level2 oplocks = yes\n");
	fprintf(fp, "kernel oplocks = no\n");

#if 0	// Conflicts with openvpn clients
#if !defined(RTCONFIG_SAMBA36X)
	fprintf(fp, "[ipc$]\n");
#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD) || defined(RTCONFIG_OPENVPN)
	fprintf(fp, "hosts allow = 127.0.0.1 %s/%s %s %s\n", nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"), pptpd_subnet, openvpn_subnet);
#else
	fprintf(fp, "hosts allow = 127.0.0.1 %s/%s\n", nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));
#endif
	fprintf(fp, "hosts deny = 0.0.0.0/0\n");
#endif
#endif	// if 0

	if (nvram_get_int("smbd_wins"))
		fprintf(fp, "wins support = yes\n");

	if (nvram_get_int("smbd_master")) {
		fprintf(fp, "os level = 255\n");
		fprintf(fp, "domain master = yes\n");
		fprintf(fp, "local master = yes\n");
		fprintf(fp, "preferred master = yes\n");
	}

#if defined(RTCONFIG_SAMBA36X)
	fprintf(fp, "enable core files = no\n");
	fprintf(fp, "deadtime = 30\n");
	fprintf(fp, "load printers = no\n");
	fprintf(fp, "printable = no\n");

// 0 - smb1, 1 = smb2, 2 = smb1 + smb2
        if (nvram_get_int("smbd_protocol") == 0)
                fprintf(fp, "max protocol = NT1\n");
        else
                fprintf(fp, "max protocol = SMB2\n");
        if (nvram_get_int("smbd_protocol") == 1)
                fprintf(fp, "min protocol = SMB2\n");

	fprintf(fp, "smb encrypt = disabled\n");
	fprintf(fp, "min receivefile size = 16384\n");
	fprintf(fp, "passdb backend = smbpasswd\n");
	fprintf(fp, "smb passwd file = /etc/samba/smbpasswd\n");
#endif

	disks_info = read_disk_data();
	if(disks_info == NULL){
		usb_dbg("Couldn't get disk list when writing smb.conf!\n");
		goto confpage;
	}

	/* share */
	if(st_samba_mode == 0){
		;
	}
	else if(st_samba_mode == 1 && nvram_match("st_samba_force_mode", "1")){
		usb_dbg("samba mode: share\n");

		for(follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next){
			for(follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next){
				if(follow_partition->mount_point == NULL)
					continue;
				
				snprintf(unique_share_name, sizeof(unique_share_name), "%s", follow_partition->mount_point);
				do {
					dup = check_mount_point_icase(disks_info, follow_partition, follow_disk, follow_partition->partition_order, unique_share_name);
					if(dup)
						snprintf(unique_share_name, sizeof(unique_share_name), "%s(%d)", follow_partition->mount_point, ++same_m_pt);
				} while(dup);
				mount_folder = strrchr(unique_share_name, '/')+1;

				fprintf(fp, "[%s]\n", mount_folder);
				fprintf(fp, "comment = %s's %s\n", follow_disk->tag, mount_folder);
#ifdef RTCONFIG_USB_CDROM
				if(!is_cdrom_name(follow_partition->device))
#endif
				fprintf(fp, "veto files = /.__*.txt*/asusware*/asus_lighttpdpasswd/\n");
				fprintf(fp, "path = %s\n", follow_partition->mount_point);
				fprintf(fp, "writeable = %s\n", strcmp(follow_partition->permission, "rw") == 0 ? "yes" : "no");
				fprintf(fp, "dos filetimes = yes\n");
				fprintf(fp, "fake directory create times = yes\n");
			}
		}
	}
#if 0
	else if(st_samba_mode == 2){
		usb_dbg("samba mode: share\n");

		for(follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next){
			for(follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next){
				if(follow_partition->mount_point == NULL)
					continue;

				snprintf(unique_share_name, sizeof(unique_share_name), "%s", follow_partition->mount_point);
				do {
					dup = check_mount_point_icase(disks_info, follow_partition, follow_disk, follow_partition->partition_order, unique_share_name);
					if(dup)
						snprintf(unique_share_name, sizeof(unique_share_name), "%s(%d)", follow_partition->mount_point, ++same_m_pt);
				} while(dup);
				mount_folder = strrchr(unique_share_name, '/')+1;

				int node_layer = get_permission(NULL, follow_partition->mount_point, NULL, "cifs", 0);
				if(node_layer == 3){
					fprintf(fp, "[%s]\n", mount_folder);
					fprintf(fp, "comment = %s's %s\n", follow_disk->tag, mount_folder);
					fprintf(fp, "path = %s\n", follow_partition->mount_point);
					fprintf(fp, "writeable = %s\n", strcmp(follow_partition->permission, "rw") == 0 ? "yes" : "no");
					fprintf(fp, "dos filetimes = yes\n");
					fprintf(fp, "fake directory create times = yes\n");
				}
				else{
#ifdef RTCONFIG_USB_CDROM
					//if(is_cdrom_name(follow_partition->device)){
					//	if(get_all_folder(follow_partition->mount_point, &sh_num, &folder_list) < 0){
					//		free_2_dimension_list(&sh_num, &folder_list);
					//		continue;
					//	}
					//} else
#endif
					//if(get_all_folder(follow_partition->mount_point, &sh_num, &folder_list) < 0)
					if(get_folder_list(follow_partition->mount_point, &sh_num, &folder_list) < 0)
					{
						free_2_dimension_list(&sh_num, &folder_list);
						continue;
					}

					for(n = 0; n < sh_num; ++n){
						samba_right = get_permission(NULL, follow_partition->mount_point, folder_list[n], "cifs", 0);
						if(samba_right < 0 || samba_right > 3)
							samba_right = DEFAULT_SAMBA_RIGHT;

						if(samba_right > 0){
							int count = get_list_strings_count(folder_list, sh_num, folder_list[n]);
							if ((!strcmp(nvram_safe_get("smbd_simpler_naming"), "1")) && (count <= 1))
								fprintf(fp, "[%s]\n", folder_list[n]);
							else
								fprintf(fp, "[%s (at %s)]\n", folder_list[n], mount_folder);
							fprintf(fp, "comment = %s's %s in %s\n", mount_folder, folder_list[n], follow_disk->tag);
							fprintf(fp, "path = %s/%s\n", follow_partition->mount_point, folder_list[n]);
							if(samba_right == 3 && strcmp(follow_partition->permission, "rw") == 0)
								fprintf(fp, "writeable = yes\n");
							else
								fprintf(fp, "writeable = no\n");

							fprintf(fp, "dos filetimes = yes\n");
							fprintf(fp, "fake directory create times = yes\n");
						}
					}

					free_2_dimension_list(&sh_num, &folder_list);
				}
			}
		}
	}
	else if(st_samba_mode == 3){
		usb_dbg("samba mode: user\n");

		// get the account list
		if(get_account_list(&acc_num, &account_list) < 0){
			usb_dbg("Can't read the account list.\n");
			free_2_dimension_list(&acc_num, &account_list);
			goto confpage;
		}

		for(follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next){
			for(follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next){
				if(follow_partition->mount_point == NULL)
					continue;

				mount_folder = strrchr(follow_partition->mount_point, '/')+1;

				// 1. get the folder list
#ifdef RTCONFIG_USB_CDROM
				//if(is_cdrom_name(follow_partition->device)){
				//	if(get_all_folder(follow_partition->mount_point, &sh_num, &folder_list) < 0)
				//		free_2_dimension_list(&sh_num, &folder_list);
				//} else
#endif
				if(get_folder_list(follow_partition->mount_point, &sh_num, &folder_list) < 0){
					free_2_dimension_list(&sh_num, &folder_list);
				}

				// 2. start to get every share
				for(n = -1; n < sh_num; ++n){
					int i, first;

					if(n == -1){
						fprintf(fp, "[%s]\n", mount_folder);
						fprintf(fp, "comment = %s's %s\n", follow_disk->tag, mount_folder);
						fprintf(fp, "path = %s\n", follow_partition->mount_point);
					}
					else{
						int count = get_list_strings_count(folder_list, sh_num, folder_list[n]);
						if ((!strcmp(nvram_safe_get("smbd_simpler_naming"), "1")) && (count <= 1))
							fprintf(fp, "[%s]\n", folder_list[n]);
						else
							fprintf(fp, "[%s (at %s)]\n", folder_list[n], mount_folder);
						fprintf(fp, "comment = %s's %s in %s\n", mount_folder, folder_list[n], follow_disk->tag);
						fprintf(fp, "path = %s/%s\n", follow_partition->mount_point, folder_list[n]);
					}

					fprintf(fp, "dos filetimes = yes\n");
					fprintf(fp, "fake directory create times = yes\n");

					fprintf(fp, "valid users = ");
					first = 1;
					for(i = 0; i < acc_num; ++i){
						if(n == -1)
							samba_right = get_permission(account_list[i], follow_partition->mount_point, NULL, "cifs", 0);
						else
							samba_right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "\cifs");
						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");

					fprintf(fp, "invalid users = ");
					first = 1;
					for(i = 0; i < acc_num; ++i){
						if(n == -1)
							samba_right = get_permission(account_list[i], follow_partition->mount_point, NULL, "cifs", 0);
						else
							samba_right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs", 0);
						if(samba_right >= 1)
							continue;
						
						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");

					fprintf(fp, "read list = ");
					first = 1;
					for(i = 0; i < acc_num; ++i){
						if(n == -1)
							samba_right = get_permission(account_list[i], follow_partition->mount_point, NULL, "cifs", 0);
						else
							samba_right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs", 0);
						if(samba_right < 1)
							continue;

						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");

					fprintf(fp, "write list = ");
					first = 1;
					for(i = 0; i < acc_num; ++i){
						if(n == -1)
							samba_right = get_permission(account_list[i], follow_partition->mount_point, NULL, "cifs", 0);
						else
							samba_right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs", 0);
						if(samba_right < 2)
							continue;

						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
				}

				free_2_dimension_list(&sh_num, &folder_list);
			}
		}

		free_2_dimension_list(&acc_num, &account_list);
	}
#endif
	else if(st_samba_mode == 4
			|| (st_samba_mode == 1 && nvram_get("st_samba_force_mode") == NULL)
			){
		usb_dbg("samba mode: user\n");

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
		// get the account list
		if(PMS_GetAccountInfo(PMS_ACTION_GET_FULL, &account_list, &group_list, &acc_num, &group_num) < 0){
			usb_dbg("Can't read the account list.\n");
			PMS_FreeAccInfo(&account_list, &group_list);
			goto confpage;
		}
#else
		// get the account list
		if(get_account_list(&acc_num, &account_list) < 0){
			usb_dbg("Can't read the account list.\n");
			free_2_dimension_list(&acc_num, &account_list);
			goto confpage;
		}
#endif

		for(follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next){
			for(follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next){
				if(follow_partition->mount_point == NULL)
					continue;

#ifdef RTCONFIG_USB_CDROM
				if(is_cdrom_name(follow_partition->device)){
					snprintf(unique_share_name, sizeof(unique_share_name), "%s", follow_partition->mount_point);
					do {
						dup = check_mount_point_icase(disks_info, follow_partition, follow_disk, follow_partition->partition_order, unique_share_name);
						if(dup)
							snprintf(unique_share_name, sizeof(unique_share_name), "%s(%d)", follow_partition->mount_point, ++same_m_pt);
					} while(dup);
					mount_folder = strrchr(unique_share_name, '/')+1;

					fprintf(fp, "[%s]\n", mount_folder);
					fprintf(fp, "comment = %s's %s\n", follow_disk->tag, mount_folder);
					fprintf(fp, "path = %s\n", follow_partition->mount_point);
					fprintf(fp, "writeable = %s\n", strcmp(follow_partition->permission, "rw") == 0 ? "yes" : "no");
					fprintf(fp, "dos filetimes = yes\n");
					fprintf(fp, "fake directory create times = yes\n");

					fprintf(fp, "valid users = ");
					first = 1;
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
					for(follow_account = account_list; follow_account != NULL; follow_account = follow_account->next){
						memset(char_user, 0, sizeof(char_user));
						ascii_to_char_safe(char_user, follow_account->name, sizeof(char_user));

						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", char_user);
					}
#else
					for(i = 0; i < acc_num; ++i){
						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", account_list[i]);
					}
#endif
					fprintf(fp, "\n");

					fprintf(fp, "invalid users = \n");

					fprintf(fp, "read list = ");
					first = 1;
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
					for(follow_account = account_list; follow_account != NULL; follow_account = follow_account->next){
						memset(char_user, 0, sizeof(char_user));
						ascii_to_char_safe(char_user, follow_account->name, sizeof(char_user));

						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", char_user);
					}
#else
					for(i = 0; i < acc_num; ++i){
						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", account_list[i]);
					}
#endif
					fprintf(fp, "\n");

					fprintf(fp, "write list = \n");

					continue;
				}
#endif

				mount_folder = strrchr(follow_partition->mount_point, '/')+1;

				// 1. get the folder list
				if(get_folder_list(follow_partition->mount_point, &sh_num, &folder_list) < 0){
					free_2_dimension_list(&sh_num, &folder_list);
					continue;
				}

				// 2. start to get every share
				for(n = 0; n < sh_num; ++n){
					int count = get_list_strings_count(folder_list, sh_num, folder_list[n]);
					if ((!strcmp(nvram_safe_get("smbd_simpler_naming"), "1")) && (count <= 1))
						fprintf(fp, "[%s]\n", folder_list[n]);
					else
						fprintf(fp, "[%s (at %s)]\n", folder_list[n], mount_folder);
					fprintf(fp, "comment = %s's %s in %s\n", mount_folder, folder_list[n], follow_disk->tag);
					fprintf(fp, "path = %s/%s\n", follow_partition->mount_point, folder_list[n]);

					fprintf(fp, "dos filetimes = yes\n");
					fprintf(fp, "fake directory create times = yes\n");

					fprintf(fp, "valid users = ");
					first = 1;
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
					for(follow_account = account_list; follow_account != NULL; follow_account = follow_account->next){
						memset(char_user, 0, sizeof(char_user));
						ascii_to_char_safe(char_user, follow_account->name, sizeof(char_user));

						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", char_user);
					}
#else
					for(i = 0; i < acc_num; ++i){
						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", account_list[i]);
					}
#endif
					fprintf(fp, "\n");

					fprintf(fp, "invalid users = ");
					first = 1;
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
					for(follow_account = account_list; follow_account != NULL; follow_account = follow_account->next){
						owned_group = follow_account->owned_group;
						while(owned_group != NULL){
							group_member = (PMS_ACCOUNT_GROUP_INFO_T *)owned_group->member;

							memset(char_user, 0, sizeof(char_user));
							ascii_to_char_safe(char_user, group_member->name, sizeof(char_user));

							samba_right_group = get_permission(char_user, follow_partition->mount_point, folder_list[n], "cifs", 1);

#ifdef UNION_PERMISSION
							if(samba_right_group >= 1)
#else
							if(samba_right_group < 1)
#endif
								break;

							owned_group = owned_group->next;
						}

						memset(char_user, 0, sizeof(char_user));
						ascii_to_char_safe(char_user, follow_account->name, sizeof(char_user));

						samba_right = get_permission(char_user, follow_partition->mount_point, folder_list[n], "cifs", 0);

#ifdef UNION_PERMISSION
						if(samba_right >= 1 || samba_right_group >= 1)
#else
						if(samba_right >= 1 && samba_right_group >= 1)
#endif
							continue;

						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", char_user);
					}
#else
					for(i = 0; i < acc_num; ++i){
						samba_right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs", 0);
						if(samba_right >= 1)
							continue;
						
						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", account_list[i]);
					}
#endif
					fprintf(fp, "\n");

					fprintf(fp, "read list = ");
					first = 1;
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
					for(follow_account = account_list; follow_account != NULL; follow_account = follow_account->next){
						owned_group = follow_account->owned_group;
						while(owned_group != NULL){
							group_member = (PMS_ACCOUNT_GROUP_INFO_T *)owned_group->member;

							memset(char_user, 0, sizeof(char_user));
							ascii_to_char_safe(char_user, group_member->name, sizeof(char_user));

							samba_right_group = get_permission(char_user, follow_partition->mount_point, folder_list[n], "cifs", 1);

#ifdef UNION_PERMISSION
							if(samba_right_group >= 1)
#else
							if(samba_right_group < 1)
#endif
								break;

							owned_group = owned_group->next;
						}
#ifndef UNION_PERMISSION
						if(samba_right_group < 1)
								continue;
#endif

						memset(char_user, 0, sizeof(char_user));
						ascii_to_char_safe(char_user, follow_account->name, sizeof(char_user));

						samba_right = get_permission(char_user, follow_partition->mount_point, folder_list[n], "cifs", 0);

#ifdef UNION_PERMISSION
						if(samba_right < 1 && samba_right_group < 1)
#else
						if(samba_right < 1)
#endif
							continue;

						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", char_user);
					}
#else
					for(i = 0; i < acc_num; ++i){
						samba_right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs", 0);
						if(samba_right < 1)
							continue;

						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", account_list[i]);
					}
#endif
					fprintf(fp, "\n");

					fprintf(fp, "write list = ");
					first = 1;
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
					for(follow_account = account_list; follow_account != NULL; follow_account = follow_account->next){
						owned_group = follow_account->owned_group;
						while(owned_group != NULL){
							group_member = (PMS_ACCOUNT_GROUP_INFO_T *)owned_group->member;

							memset(char_user, 0, sizeof(char_user));
							ascii_to_char_safe(char_user, group_member->name, sizeof(char_user));

							samba_right_group = get_permission(char_user, follow_partition->mount_point, folder_list[n], "cifs", 1);

#ifdef UNION_PERMISSION
							if(samba_right_group >= 2)
#else
							if(samba_right_group < 2)
#endif
								break;

							owned_group = owned_group->next;
						}
#ifndef UNION_PERMISSION
						if(samba_right_group < 2)
								continue;
#endif

						memset(char_user, 0, sizeof(char_user));
						ascii_to_char_safe(char_user, follow_account->name, sizeof(char_user));

						samba_right = get_permission(char_user, follow_partition->mount_point, folder_list[n], "cifs", 0);

#ifdef UNION_PERMISSION
						if(samba_right < 2 && samba_right_group < 2)
#else
						if(samba_right < 2)
#endif
							continue;

						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", char_user);
					}
#else
					for(i = 0; i < acc_num; ++i){
						samba_right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs", 0);
						if(samba_right < 2)
							continue;

						if(first == 1)
							first = 0;
						else
							fprintf(fp, ", ");

						fprintf(fp, "%s", account_list[i]);
					}
#endif
					fprintf(fp, "\n");
				}

				free_2_dimension_list(&sh_num, &folder_list);
			}
		}

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
		PMS_FreeAccInfo(&account_list, &group_list);
#else
		free_2_dimension_list(&acc_num, &account_list);
#endif
	}

confpage:
	if(fp != NULL) {

		append_custom_config("smb.conf", fp);
		fclose(fp);

		use_custom_config("smb.conf", SAMBA_CONF);
		run_postconf("smb", SAMBA_CONF);
		chmod(SAMBA_CONF, 0644);
	}

	free_disk_data(&disks_info);
	return 0;
}
