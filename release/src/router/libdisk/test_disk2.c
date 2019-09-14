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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define VERSION 1
//#define PC

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <limits.h>
#include <dirent.h>
#include <bcmnvram.h>

#include <shared.h>

#include "disk_initial.h"
#include "usb_info.h"

#if defined(usb_dbg)
#undef usb_dbg
#endif
#define usb_dbg printf

int isSerialNode(const char *device_name)
{
	if(strstr(device_name, "ttyUSB") == NULL)
		return 0;

	return 1;
}

int isACMNode(const char *device_name)
{
	if(strstr(device_name, "ttyACM") == NULL)
		return 0;

	return 1;
}

#ifdef RTCONFIG_USB_BECEEM
int isBeceemNode(const char *device_name)
{
	if(strstr(device_name, "usbbcm") == NULL)
		return 0;

	return 1;
}
#endif

int isStorageDevice(const char *device_name){
	if(!strncmp(device_name, "sd", 2))
		return 1;

	return 0;
}

#if defined(RTCONFIG_M2_SSD)
int isM2SSDDevice(const char *device_name)
{
	char disk_name[32], *p;
	char disk_path[PATH_MAX], path[PATH_MAX];

	if(strncmp(device_name, "sd", 2))
		return 0;

	strlcpy(disk_name, device_name, sizeof(disk_name));
	for (p = disk_name + strlen(disk_name) - 1; isdigit(*p) && p > disk_name; p--)
		*p = '\0';

	/* /sys/block/sda:
	 * kernel v3.4: sda -> ../devices/platform/soc/29000000.sata/ata1/host0/target0:0:0/0:0:0:0/block/sda
	 * kernel v4.4: sda -> ../devices/platform/msm_sata.0/ahci.0/ata1/host0/target0:0:0/0:0:0:0/block/sda
	 */
	snprintf(disk_path, sizeof(disk_path), "/sys/block/%s", disk_name);
	if (readlink(disk_path, path, sizeof(path)) <= 0 || !strstr(path, "sata"))
		return 0;

	return 1;
}
#endif

#ifdef BCM_MMC
int isMMCDevice(const char *device_name){
	if(!strncmp(device_name, "mmcblk", 6))
		return 1;

	return 0;
}
#endif

#if defined(RTCONFIG_USB_MODEM) || defined(RTCONFIG_USB_CDROM)
int isCDROMDevice(const char *device_name){
	if(!strncmp(device_name, "sr", 2) && isdigit(device_name[2]))
		return 1;

	return 0;
}
#endif

int get_device_type_by_device(const char *device_name)
{
	if(device_name == NULL || strlen(device_name) <= 0){
		usb_dbg("(%s): The device name is not correct.\n", device_name);
		return DEVICE_TYPE_UNKNOWN;
	}

	if(isStorageDevice(device_name)
#ifdef BCM_MMC
			|| isMMCDevice(device_name) // SD card.
#endif
			){
		return DEVICE_TYPE_DISK;
	}
#ifdef RTCONFIG_USB_PRINTER
	if(!strncmp(device_name, "lp", 2)){
		return DEVICE_TYPE_PRINTER;
	}
#endif
#if defined(RTCONFIG_USB_MODEM) || defined(RTCONFIG_USB_CDROM)
	if(!strncmp(device_name, "sg", 2)){
		return DEVICE_TYPE_SG;
	}
	if(isCDROMDevice(device_name)){
		return DEVICE_TYPE_CD;
	}
#endif
#ifdef RTCONFIG_USB_MODEM
	if(isSerialNode(device_name) || isACMNode(device_name)){
		return DEVICE_TYPE_MODEM;
	}
#endif
#ifdef RTCONFIG_USB_BECEEM
	if(isBeceemNode(device_name)){
		return DEVICE_TYPE_BECEEM;
	}
#endif

	return DEVICE_TYPE_UNKNOWN;
}

#ifdef PC
char *get_usb_port_by_device(const char *device_name, char *buf, const int buf_size){
	int device_type = get_device_type_by_device(device_name);

	if(device_type == DEVICE_TYPE_UNKNOWN)
		return NULL;

	memset(buf, 0, buf_size);
	strcpy(buf, "1-1");

	return buf;
}
#else
char *get_usb_port_by_string(const char *target_string, char *buf, const int buf_size)
{
	memset(buf, 0, buf_size);

	if(strstr(target_string, USB_XHCI_PORT_1))
		strcpy(buf, USB_XHCI_PORT_1);
	else if(strstr(target_string, USB_XHCI_PORT_2))
		strcpy(buf, USB_XHCI_PORT_2);
	else if(strstr(target_string, USB_EHCI_PORT_1))
		strcpy(buf, USB_EHCI_PORT_1);
	else if(strstr(target_string, USB_EHCI_PORT_2))
		strcpy(buf, USB_EHCI_PORT_2);
	else if(strstr(target_string, USB_OHCI_PORT_1))
		strcpy(buf, USB_OHCI_PORT_1);
	else if(strstr(target_string, USB_OHCI_PORT_2))
		strcpy(buf, USB_OHCI_PORT_2);
	else if(strstr(target_string, USB_EHCI_PORT_3))
		strcpy(buf, USB_EHCI_PORT_3);
	else if(strstr(target_string, USB_OHCI_PORT_3))
		strcpy(buf, USB_OHCI_PORT_3);
	else
		return NULL;

	return buf;
}

char *get_usb_port_by_device(const char *device_name, char *buf, const int buf_size)
{
	int device_type = get_device_type_by_device(device_name);
	char device_path[128], usb_path[PATH_MAX];
	char disk_name[16];

	if(device_type == DEVICE_TYPE_UNKNOWN)
		return NULL;

	memset(usb_path, 0, sizeof(usb_path));

	if(device_type == DEVICE_TYPE_DISK){
		get_disk_name(device_name, disk_name, 16);
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_BLOCK, disk_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
#ifdef RTCONFIG_USB_PRINTER
	if(device_type == DEVICE_TYPE_PRINTER){
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_USB, device_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
#endif
#if defined(RTCONFIG_USB_MODEM) || defined(RTCONFIG_USB_CDROM)
	if(device_type == DEVICE_TYPE_SG){
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_SG, device_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
	if(device_type == DEVICE_TYPE_CD){
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_BLOCK, device_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
#endif
#ifdef RTCONFIG_USB_MODEM
	if(device_type == DEVICE_TYPE_MODEM){
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_TTY, device_name);
		if(realpath(device_path, usb_path) == NULL){
			sleep(1); // Sometimes link would be built slowly, so try again.

			if(realpath(device_path, usb_path) == NULL){
				usb_dbg("(%s)(2/2): Fail to get link: %s.\n", device_name, device_path);
				return NULL;
			}
		}
	}
	else
#endif
#ifdef RTCONFIG_USB_BECEEM
	if(device_type == DEVICE_TYPE_BECEEM){
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_USB, device_name);
		if(realpath(device_path, usb_path) == NULL){
			if(realpath(device_path, usb_path) == NULL){
				usb_dbg("(%s)(2/2): Fail to get link: %s.\n", device_name, device_path);
				return NULL;
			}
		}
	}
	else
#endif
		return NULL;

#ifdef BCM_MMC
	if(isMMCDevice(device_name)){ // SD card.
		snprintf(buf, buf_size, "%s", SDCARD_PORT);
	}
	else
#endif
	if(get_usb_port_by_string(usb_path, buf, buf_size) == NULL){
		usb_dbg("(%s): Fail to get usb port: %s.\n", device_name, usb_path);
		return NULL;
	}

	return buf;
}
#endif

char *get_usb_node_by_string(const char *target_string, char *ret, const int ret_size)
{
	char usb_port[32], buf[16];
	char *ptr, *ptr2, *ptr3;
	int len;

#if defined(RTCONFIG_M2_SSD)
	if (isM2SSDDevice(target_string)) {
		strlcpy(ret, M2_SSD_PORT, ret_size);
		return ret;
	}
#endif
	memset(usb_port, 0, sizeof(usb_port));
	if(get_usb_port_by_string(target_string, usb_port, sizeof(usb_port)) == NULL)
		return NULL;
	if((ptr = strstr(target_string, usb_port)) == NULL)
		return NULL;
	if(ptr != target_string)
		ptr += strlen(usb_port)+1;

	if((ptr2 = strchr(ptr, ':')) == NULL)
		return NULL;
	ptr3 = ptr2;
	*ptr3 = 0;

	if((ptr2 = strrchr(ptr, '/')) == NULL)
		ptr2 = ptr;
	else
		ptr = ptr2+1;

	len = strlen(ptr);
	if(len > 16)
		len = 16;

	memset(buf, 0, sizeof(buf));
	strncpy(buf, ptr, len);

	len = strlen(buf);
	if(len > ret_size)
		len = ret_size;

	memset(ret, 0, ret_size);
	strncpy(ret, buf, len);

	*ptr3 = ':';

	return ret;
}

char *get_usb_node_by_device(const char *device_name, char *buf, const int buf_size)
{
	int device_type = get_device_type_by_device(device_name);
	char device_path[128], usb_path[PATH_MAX];
	char disk_name[16];

	if(device_type == DEVICE_TYPE_UNKNOWN)
		return NULL;

	memset(usb_path, 0, sizeof(usb_path));

	if(device_type == DEVICE_TYPE_DISK){
		get_disk_name(device_name, disk_name, 16);
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_BLOCK, disk_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
#ifdef RTCONFIG_USB_PRINTER
	if(device_type == DEVICE_TYPE_PRINTER){
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_USB, device_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
#endif
#ifdef RTCONFIG_USB_MODEM
	if(device_type == DEVICE_TYPE_SG){
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_SG, device_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
	if(device_type == DEVICE_TYPE_CD){
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_BLOCK, device_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
	if(device_type == DEVICE_TYPE_MODEM){
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_TTY, device_name);
		if(realpath(device_path, usb_path) == NULL){
			sleep(1); // Sometimes link would be built slowly, so try again.

			if(realpath(device_path, usb_path) == NULL){
				usb_dbg("(%s)(2/2): Fail to get link: %s.\n", device_name, device_path);
				return NULL;
			}
		}
	}
	else
#endif
#ifdef RTCONFIG_USB_BECEEM
	if(device_type == DEVICE_TYPE_BECEEM){
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_USB, device_name);
		if(realpath(device_path, usb_path) == NULL){
			if(realpath(device_path, usb_path) == NULL){
				usb_dbg("(%s)(2/2): Fail to get link: %s.\n", device_name, device_path);
				return NULL;
			}
		}
	}
	else
#endif
		return NULL;

#ifdef BCM_MMC
	if(isMMCDevice(device_name)){ // SD card.
		snprintf(buf, buf_size, "%s", SDCARD_PORT);
	}
	else
#endif
#if defined(RTCONFIG_M2_SSD)
	/* M.2 SATA SSD */
	if (isM2SSDDevice(device_name)) {
		strlcpy(buf, M2_SSD_PORT, buf_size);
	}
	else
#endif
	if(get_usb_node_by_string(usb_path, buf, buf_size) == NULL){
		usb_dbg("(%s): Fail to get usb node: %s.\n", device_name, usb_path);
		return NULL;
	}

	return buf;
}

char *get_path_by_node(const char *usb_node, char *buf, const int buf_size){
	char usb_port[32], *hub_path;
	int port_num = 0, len;

	if(usb_node == NULL || buf == NULL || buf_size <= 0)
		return NULL;

	// Get USB port.
	if(get_usb_port_by_string(usb_node, usb_port, sizeof(usb_port)) == NULL)
		return NULL;

	port_num = get_usb_port_number(usb_port);
	if(port_num == 0)
		return NULL;

	if(strlen(usb_node) > (len = strlen(usb_port))){
		hub_path = (char *)usb_node+len;
		snprintf(buf, buf_size, "%d%s", port_num, hub_path);
	}
	else
		snprintf(buf, buf_size, "%d", port_num);

	return buf;
}

void strntrim(char *str){
	register char *start, *end;
	int len;

	if(str == NULL)
		return;

	len = strlen(str);
	start = str;
	end = start+len-1;

	while(start < end && isspace(*start))
		++start;
	while(start <= end && isspace(*end))
		--end;

	end++;

	if((int)(end-start) < len){
		memcpy(str, start, (end-start));
		str[end-start] = 0;
	}

	return;
}

char *read_whole_file(const char *target) {
	FILE *fp = fopen(target, "r");
	char *buffer, *new_str;
	int i;
	unsigned int read_bytes = 0;
	unsigned int each_size = 1024;
	
	if (fp == NULL)
		return NULL;
	
	buffer = (char *)malloc(sizeof(char)*each_size+read_bytes);
	if (buffer == NULL) {
		usb_dbg("No memory \"buffer\".\n");
		fclose(fp);
		return NULL;
	}
	memset(buffer, 0, sizeof(char)*each_size+read_bytes);
	
	while ((i = fread(buffer+read_bytes, each_size * sizeof(char), 1, fp)) == 1){
		read_bytes += each_size;
		new_str = (char *)malloc(sizeof(char)*each_size+read_bytes);
		if (new_str == NULL) {
			usb_dbg("No memory \"new_str\".\n");
			free(buffer);
			fclose(fp);
			return NULL;
		}
		memset(new_str, 0, sizeof(char)*each_size+read_bytes);
		memcpy(new_str, buffer, read_bytes);
		
		free(buffer);
		buffer = new_str;
	}
	
	fclose(fp);
	return buffer;
}

char *get_line_from_buffer(const char *buf, char *line, const int line_size){
	int buf_len, len;
	char *ptr;

	if(buf == NULL || (buf_len = strlen(buf)) <= 0)
		return NULL;

	if((ptr = strchr(buf, '\n')) == NULL)
		ptr = (char *)(buf+buf_len);

	if((len = ptr-buf) < 0)
		len = buf-ptr;
	++len; // include '\n'.

	memset(line, 0, line_size);
	strncpy(line, buf, len);

	return line;
}

#include <disk_initial.c>

int main(int argc, char *argv[]){
	disk_info_t *disk_info, *disk_list;
	partition_info_t *partition_info;
	char buf[PATH_MAX];

	usb_dbg("%d: Using myself to get information:\n", VERSION);

	if(argc == 3){
		get_mount_path("sdb1", buf, PATH_MAX);
		printf("buf=%s.\n", buf);
	}
	else if(argc == 2){
		if(is_disk_name(argv[1])){ // Disk
			usb_dbg("%d: Get disk(%s)'s information:\n", VERSION, argv[1]);

			create_disk(argv[1], &disk_info);

			print_disk(disk_info);

			free_disk_data(&disk_info);
		}
		else{
			usb_dbg("%d: Get partition(%s)'s information:\n", VERSION, argv[1]);

			create_partition(argv[1], &partition_info);

			print_partition(partition_info);

			free_partition_data(&partition_info);
		}
	}
	else{
		usb_dbg("%d: Get all Disk information:\n", VERSION);

		disk_list = read_disk_data();

		print_disks(disk_list);

		free_disk_data(&disk_list);
	}

	return 0;
}
