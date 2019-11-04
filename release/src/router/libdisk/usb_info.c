#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <shared.h>
#include "usb_info.h"
#include "disk_initial.h"
#include <shutils.h>

//#ifdef RTN56U
//#include <nvram/bcmnvram.h>
//#else
#include <bcmnvram.h>
//#endif


char *supported_types[] = {"printer", "modem", "storage", NULL};


int find_str_host_info(const char *str, int *host, int *channel, int *id, int *lun)
{
	int order = 0;
	char word[PATH_MAX], *next, *ptr;

	ptr = (char *)str;

	foreach_58(word, ptr, next){
		switch(order){
			case 0:
				if(host != NULL)
					*host = atoi(word);
				break;
			case 1:
				if(channel != NULL)
					*channel = atoi(word);
				break;
			case 2:
				if(id != NULL)
					*id = atoi(word);
				break;
			case 3:
				if(lun != NULL)
					*lun = atoi(word);
				break;
		}

		++order;
	}

	if(order != 4){
		usb_dbg("(%s): Didn't get the scsi info enough.\n", str);
		return -1;
	}

	return 0;
}

int find_disk_host_info(const char *dev, int *host, int *channel, int *id, int *lun)
{
	DIR *disk_dir;
	struct dirent *dp;
	char *ptr, buf[128];

	snprintf(buf, sizeof(buf), "/sys/block/%s/device", dev);

	if((disk_dir = opendir(buf)) == NULL){
		usb_dbg("(%s): The path isn't existed: %s.\n", dev, buf);
		return -1;
	}

	while((dp = readdir(disk_dir))){
		if(!strncmp(dp->d_name, "scsi_disk:", 10)){
			ptr = dp->d_name+10;
			find_str_host_info(ptr, host, channel, id, lun);

			break;
		}
	}
	closedir(disk_dir);

	return 0;
}

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
	if(isTTYNode(device_name)
			|| !strncmp(device_name, "usb", 3)
			|| !strncmp(device_name, "wwan", 4)
			|| !strncmp(device_name, "lte", 3)
			|| !strncmp(device_name, "eth", 3)
			){
		return DEVICE_TYPE_MODEM;
	}
#endif
#ifdef RTCONFIG_USB_BECEEM
	if(isBeceemNode(device_name)
			|| !strncmp(device_name, "wimax", 5)
			){
		return DEVICE_TYPE_BECEEM;
	}
#endif

	return DEVICE_TYPE_UNKNOWN;
}

char *get_device_type_by_interface(const char *usb_interface, char *type, const int type_size, char *dev, const int dev_size)
{
	char cmd[128], buf[PATH_MAX], *ptr = NULL, *ret = NULL;
	int len;
	FILE *fp;
	char new_path[PATH_MAX], real_path[PATH_MAX];
	DIR *dir;
	struct dirent *d;

	if(!type && !dev)
		return NULL;
	if(type){
		memset(type, 0, type_size);
		ret = type;
	}
	if(dev){
		memset(dev, 0, dev_size);
		if(!type)
			ret = dev;
	}

	snprintf(cmd, sizeof(cmd), "/usr/bin/find %s/ -name %s", USB_DRIVER_PATH, usb_interface);
	if((fp = popen(cmd, "r")) == NULL)
		return NULL;

	ptr = fgets(buf, sizeof(buf), fp);
	pclose(fp);

	if(ptr == NULL)
		return NULL;

	len = strlen(buf);
	buf[len-1] = 0;

	if((ptr = strstr(buf, "usblp")) != NULL){
		if(type)
			snprintf(type, type_size, "printer");

		snprintf(new_path, sizeof(new_path), "%s/usbmisc", buf);
		if(d_exists(new_path)){
			if((dir = opendir(new_path)) == NULL)
				return ret;

			memset(buf, 0, sizeof(buf));
			while((d = readdir(dir)) != NULL){
				if(!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
					continue;

				snprintf(buf, sizeof(buf), "%s", d->d_name);
				ptr = buf;
			}
			closedir(dir);

			if(!(*buf))
				ptr = NULL;
		}
		else{
			snprintf(cmd, sizeof(cmd), "/usr/bin/find %s/ -name usb:lp*", buf);
			if((fp = popen(cmd, "r")) == NULL)
				return ret;

			ptr = fgets(buf, sizeof(buf), fp);
			pclose(fp);

			if(ptr == NULL)
				return ret;

			if((ptr = rindex(buf, ':')) == NULL)
				return ret;

			++ptr;
			len = strlen(ptr);
			ptr[len-1] = 0;
		}
	}
	else if((ptr = strstr(buf, "option")) != NULL
			|| (ptr = strstr(buf, "usbserial")) != NULL
			|| (ptr = strstr(buf, "usbserial_generic")) != NULL
			){
		if(type)
			snprintf(type, type_size, "modem");

		snprintf(cmd, sizeof(cmd), "/usr/bin/find %s/ -name ttyUSB*", buf);
		if((fp = popen(cmd, "r")) == NULL)
			return ret;

		ptr = fgets(buf, sizeof(buf), fp);
		pclose(fp);

		if(ptr == NULL)
			return ret;

		if((ptr = rindex(buf, '/')) == NULL)
			return ret;

		++ptr;
		len = strlen(ptr);
		ptr[len-1] = 0;
	}
	else if((ptr = strstr(buf, "cdc_acm")) != NULL
			|| (ptr = strstr(buf, "acm")) != NULL
			){
		if(type)
			snprintf(type, type_size, "modem");

		snprintf(cmd, sizeof(cmd), "/usr/bin/find %s/ -name tty:ttyACM*", buf);
		if((fp = popen(cmd, "r")) == NULL)
			return ret;

		ptr = fgets(buf, sizeof(buf), fp);
		pclose(fp);

		if(ptr == NULL){
			snprintf(cmd, sizeof(cmd), "/usr/bin/find %s/ -name ttyACM*", buf);
			if((fp = popen(cmd, "r")) == NULL)
				return ret;

			ptr = fgets(buf, sizeof(buf), fp);
			pclose(fp);

			if(ptr == NULL)
				return ret;

			ptr = rindex(buf, '/');
		}
		else
			ptr = rindex(buf, ':');

		if(ptr == NULL)
			return ret;

		++ptr;
		len = strlen(ptr);
		ptr[len-1] = 0;
	}
	else if((ptr = strstr(buf, "rndis_host")) != NULL
			|| (ptr = strstr(buf, "qmi_wwan")) != NULL
			|| (ptr = strstr(buf, "GobiNet")) != NULL
			|| (ptr = strstr(buf, "cdc_ether")) != NULL
			|| (ptr = strstr(buf, "cdc_ncm")) != NULL
			|| (ptr = strstr(buf, "asix")) != NULL
			|| (ptr = strstr(buf, "ax88179_178a")) != NULL
			){
		if(type)
			snprintf(type, type_size, "modem");

		snprintf(new_path, sizeof(new_path), "%s/net", buf);
		if(d_exists(new_path)){
			if((dir = opendir(new_path)) == NULL)
				return ret;

			memset(buf, 0, sizeof(buf));
			while((d = readdir(dir)) != NULL){
				if(!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
					continue;

				snprintf(buf, sizeof(buf), "%s", d->d_name);
				ptr = buf;
			}
			closedir(dir);

			if(!(*buf))
				ptr = NULL;
		}
		else{
			snprintf(cmd, sizeof(cmd), "/usr/bin/find %s/ -name net:*", buf);
			if((fp = popen(cmd, "r")) == NULL)
				return ret;

			ptr = fgets(buf, sizeof(buf), fp);
			pclose(fp);

			if(ptr == NULL)
				return ret;

			if((ptr = rindex(buf, ':')) == NULL)
				return ret;

			++ptr;
			len = strlen(ptr);
			ptr[len-1] = 0;
		}
	}
	else if((ptr = strstr(buf, "usb-storage")) != NULL){
		if(type)
			snprintf(type, type_size, "storage");

		snprintf(cmd, sizeof(cmd), "/usr/bin/find %s/ -name block:*", buf);
		if((fp = popen(cmd, "r")) == NULL)
			return ret;

		ptr = fgets(buf, sizeof(buf), fp);
		pclose(fp);

		if(ptr == NULL){
			if((dir = opendir(SYS_BLOCK)) == NULL)
				return ret;

			memset(buf, 0, sizeof(buf));
			while((d = readdir(dir)) != NULL){
				if(!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
					continue;
				if(!strncmp(d->d_name, "loop", 4) || !strncmp(d->d_name, "mtd", 3) || !strncmp(d->d_name, "ram", 3))
					continue;

				snprintf(new_path, sizeof(new_path), "%s/%s/device", SYS_BLOCK, d->d_name);
				if(realpath(new_path, real_path) == NULL)
					continue;

				snprintf(cmd, sizeof(cmd), "/%s/", usb_interface);
				if(strstr(real_path, cmd) == NULL)
					continue;

				snprintf(buf, sizeof(buf), "%s", d->d_name);
				ptr = buf;
			}
			closedir(dir);

			if(!(*buf))
				ptr = NULL;
		}
		else{
			if((ptr = rindex(buf, ':')) == NULL)
				return ret;

			++ptr;
			len = strlen(ptr);
			ptr[len-1] = 0;
		}
	}

	if(ptr){
		if(dev)
			snprintf(dev, dev_size, "%s", ptr);
	}

	return ret;
}

char *get_device_type_by_node(const char *usb_node, char *type, const int type_size, char *dev, const int dev_size)
{
	int interface_num, interface_count;
	char interface_name[16];
	char buf[PATH_MAX];
#ifdef RTCONFIG_USB_PRINTER
	int got_printer = 0;
#endif
#ifdef RTCONFIG_USB_MODEM
	int got_modem = 0;
#endif
	int got_disk = 0;

	if(!type && !dev)
		return NULL;
	if(type)
		memset(type, 0, type_size);
	if(dev)
		memset(dev, 0, dev_size);

	interface_num = get_usb_interface_number(usb_node);
	if(interface_num <= 0)
		return NULL;

	for(interface_count = 0; interface_count < interface_num; ++interface_count){
		snprintf(interface_name, sizeof(interface_name), "%s:1.%d", usb_node, interface_count);

		snprintf(buf, sizeof(buf), "%s/%s", USB_DEVICE_PATH, interface_name);
		if(!d_exists(buf))
			continue;

#ifdef RTCONFIG_USB_PRINTER
		if(isPrinterInterface(interface_name)){
			get_device_type_by_interface(interface_name, NULL, 0, dev, dev_size);

			++got_printer;
			break;
		}
		else
#endif
#ifdef RTCONFIG_USB_MODEM
		if(isSerialInterface(interface_name, 0, 0, 0) || isACMInterface(interface_name, 0, 0, 0)
				|| isRNDISInterface(interface_name, 0, 0)
				|| isQMIInterface(interface_name)
				|| isGOBIInterface(interface_name)
				|| isCDCETHInterface(interface_name)
				|| isNCMInterface(interface_name)
				|| isASIXInterface(interface_name)
#ifdef RTCONFIG_USB_BECEEM
				|| isGCTInterface(interface_name)
#endif
				){
			get_device_type_by_interface(interface_name, NULL, 0, buf, sizeof(buf));
			if(dev){
				if(got_modem && *dev && !strncmp(buf, "tty", 3))
					continue;

				snprintf(dev, dev_size, "%s", buf);
			}

			++got_modem;
		}
		else
#endif
		if(isStorageInterface(interface_name)){
			get_device_type_by_interface(interface_name, NULL, 0, dev, dev_size);

			++got_disk;
		}
	}

	if(
#ifdef RTCONFIG_USB_PRINTER
			!got_printer
#else
			1
#endif
			&&
#ifdef RTCONFIG_USB_MODEM
			!got_modem
#else
			1
#endif
			&& !got_disk
			)
		return NULL;

	if(type){
#ifdef RTCONFIG_USB_PRINTER
		if(got_printer > 0) // Top priority
			snprintf(type, type_size, "printer");
		else
#endif
#ifdef RTCONFIG_USB_MODEM
		if(got_modem > 0) // 2nd priority
			snprintf(type, type_size, "modem");
		else
#endif
		if(got_disk > 0)
			snprintf(type, type_size, "storage");
		else
			return NULL;

		return type;
	}

	if(dev)
		return dev;

	return NULL;
}

/**
 * Get USB node.
 * @target_string:
 *  USB:	1-1, 1-1:1.0, 1-1.4, 1-1.4:1.0, 1-1.4.4, 1-1.4.4:1.0, etc
 *  M.2 SSD:	ata1
 * @return:
 *  USB:	1-1, 1-1,     1-1.4, 1-1.4,     1-1.4.4, 1-1.4.4, etc
 *  M.2 SSD:	ata1
 */
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
		get_disk_name(device_name, disk_name, sizeof(disk_name));
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
		if(isTTYNode(device_name))
			snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_TTY, device_name);
		else
			snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_NET, device_name);
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

/**
 * Get USB port.
 * @target_string:
 *  USB:	../devices/platform/ipq-dwc3.0/dwc3.0/xhci-hcd.0/usb1/1-1/1-1:1.0/host1/target1:0:0/1:0:0:0/block/sdb
 *       	1-1, 1-1:1.0, 1-1.4, 1-1.4:1.0, 1-1.4.4, 1-1.4.4:1.0,
 *       	2-1, 2-1.3.2, etc
 *  M.2 SSD:	../devices/platform/msm_sata.0/ahci.0/ata1/host0/target0:0:0/0:0:0:0/block/sda, ata1
 * @return:
 *  USB:	1-1
 *  		1-1, 1-1, 1-1, 1-1, 1-1, 1-1
 *  		2-1, 2-1, etc
 *  M.2 SSD:	ata1, ata1
 */
char *get_usb_port_by_string(const char *target_string, char *buf, const int buf_size)
{
	memset(buf, 0, buf_size);

	if(strstr(target_string, (const char *)USB_XHCI_PORT_1))
		strcpy(buf, (const char *)USB_XHCI_PORT_1);
	else if(strstr(target_string, (const char *)USB_XHCI_PORT_2))
		strcpy(buf, (const char *)USB_XHCI_PORT_2);
	else if(strstr(target_string, (const char *)USB_EHCI_PORT_1))
		strcpy(buf, (const char *)USB_EHCI_PORT_1);
	else if(strstr(target_string, (const char *)USB_EHCI_PORT_2))
		strcpy(buf, (const char *)USB_EHCI_PORT_2);
	else if(strstr(target_string, (const char *)USB_OHCI_PORT_1))
		strcpy(buf, (const char *)USB_OHCI_PORT_1);
	else if(strstr(target_string, (const char *)USB_OHCI_PORT_2))
		strcpy(buf, (const char *)USB_OHCI_PORT_2);
#ifdef BCM_MMC
	else if(strstr(target_string, SDCARD_PORT))
		strcpy(buf, SDCARD_PORT);
#endif
#if defined(RTCONFIG_M2_SSD)
	else if(strstr(target_string, (const char *)M2_SSD_PORT))
		strcpy(buf, M2_SSD_PORT);
#endif
	else if(strstr(target_string, (const char *)USB_EHCI_PORT_3))
		strcpy(buf, (const char *)USB_EHCI_PORT_3);
	else if(strstr(target_string, (const char *)USB_OHCI_PORT_3))
		strcpy(buf, (const char *)USB_OHCI_PORT_3);
	else{
		usb_dbg("%s: wrong 1. target_string=%s.\n", __func__, target_string);
		return NULL;
	}

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
		get_disk_name(device_name, disk_name, sizeof(disk_name));
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
		if(isTTYNode(device_name))
			snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_TTY, device_name);
		else
			snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_NET, device_name);
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
	if(get_usb_port_by_string(usb_path, buf, buf_size) == NULL){
		usb_dbg("(%s): Fail to get usb port: %s.\n", device_name, usb_path);
		return NULL;
	}

	return buf;
}

char *get_interface_by_string(const char *target_string, char *ret, const int ret_size)
{
	char buf[32], *ptr, *ptr_end, *ptr2, *ptr2_end;
	int len;

	if((ptr = strstr(target_string, (const char *)USB_XHCI_PORT_1)) != NULL)
		ptr += strlen((const char *)USB_XHCI_PORT_1);
	else if((ptr = strstr(target_string, (const char *)USB_XHCI_PORT_2)) != NULL)
		ptr += strlen((const char *)USB_XHCI_PORT_2);
	else if((ptr = strstr(target_string, (const char *)USB_EHCI_PORT_1)) != NULL)
		ptr += strlen((const char *)USB_EHCI_PORT_1);
	else if((ptr = strstr(target_string, (const char *)USB_EHCI_PORT_2)) != NULL)
		ptr += strlen((const char *)USB_EHCI_PORT_2);
	else if((ptr = strstr(target_string, (const char *)USB_OHCI_PORT_1)) != NULL)
		ptr += strlen((const char *)USB_OHCI_PORT_1);
	else if((ptr = strstr(target_string, (const char *)USB_OHCI_PORT_2)) != NULL)
		ptr += strlen((const char *)USB_OHCI_PORT_2);
	else if((ptr = strstr(target_string, (const char *)USB_EHCI_PORT_3)) != NULL)
		ptr += strlen((const char *)USB_EHCI_PORT_3);
	else if((ptr = strstr(target_string, (const char *)USB_OHCI_PORT_3)) != NULL)
		ptr += strlen((const char *)USB_OHCI_PORT_3);
	else
		return NULL;
	++ptr;

	if((ptr_end = strchr(ptr, ':')) == NULL)
		ptr_end = ptr+strlen(ptr);

	if((ptr2_end = strchr(ptr_end, '/')) == NULL)
		ptr2_end = ptr_end+strlen(ptr_end);

	len = strlen(ptr)-strlen(ptr2_end);
	if(len > 32)
		len = 32;
	memset(buf, 0, sizeof(buf));
	strncpy(buf, ptr, len);

	if((ptr2 = strrchr(buf, '/')) == NULL)
		ptr2 = buf;
	else
		++ptr2;

	if((len = ptr_end-ptr) < 0)
		len = ptr-ptr_end;

	snprintf(ret, ret_size, "%s", ptr2); // ex: 1-1:1.0/~

	return ret;
}

char *get_interface_by_device(const char *device_name, char *buf, const int buf_size)
{
	int device_type = get_device_type_by_device(device_name);
	char device_path[128], usb_path[PATH_MAX];
	char disk_name[16];

	if(device_type == DEVICE_TYPE_UNKNOWN)
		return NULL;

	memset(usb_path, 0, sizeof(usb_path));

	if(device_type == DEVICE_TYPE_DISK){
		get_disk_name(device_name, disk_name, sizeof(disk_name));
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
		if(isTTYNode(device_name))
			snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_TTY, device_name);
		else
			snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_NET, device_name);
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

	if(get_interface_by_string(usb_path, buf, buf_size) == NULL){
		usb_dbg("(%s): Fail to get usb port: %s.\n", device_name, usb_path);
		return NULL;
	}

	return buf;
}

char *get_path_by_node(const char *usb_node, char *buf, const int buf_size){
	char usb_port[32], *hub_path;
	int port_num = 0, len;

	if(usb_node == NULL || buf == NULL || buf_size <= 0){
		usb_dbg("%s: wrong 1.\n", __func__);
		return NULL;
	}

	// Get USB port.
	if(get_usb_port_by_string(usb_node, usb_port, sizeof(usb_port)) == NULL){
		usb_dbg("%s: wrong 2. usb_node=%s.\n", __func__, usb_node);
		return NULL;
	}

	port_num = get_usb_port_number(usb_port);
	if(port_num == 0){
		usb_dbg("%s: wrong 3. usb_port=%s.\n", __func__, usb_port);
		return NULL;
	}

	if(strlen(usb_node) > (len = strlen(usb_port))){
		hub_path = (char *)usb_node+len;
		snprintf(buf, buf_size, "%d%s", port_num, hub_path);
	}
	else
		snprintf(buf, buf_size, "%d", port_num);

	return buf;
}

static FILE *open_usb_target(const char *usb_node, const char *target, const int wait)
{
	FILE *fp;
	char usb_port[32], target_file[128];
	int retry = wait;

	if(usb_node == NULL || target == NULL ||
	   get_usb_port_by_string(usb_node, usb_port, sizeof(usb_port)) == NULL)
		return NULL;

	snprintf(target_file, sizeof(target_file), "%s/%s/%s", USB_DEVICE_PATH, usb_node, target);

	while((fp = fopen(target_file, "r")) == NULL && retry-- > 0)
		sleep(1);

	return fp;
}

unsigned int get_usb_vid(const char *usb_node)
{
	FILE *fp;
	unsigned int val;

	if((fp = open_usb_target(usb_node, "idVendor", 0)) == NULL)
		return 0;

	if(fscanf(fp, "%x", &val) < 1)
		val = 0;
	fclose(fp);

	return val;
}

unsigned int get_usb_pid(const char *usb_node)
{
	FILE *fp;
	unsigned int val;

	if((fp = open_usb_target(usb_node, "idProduct", 0)) == NULL)
		return 0;

	if(fscanf(fp, "%x", &val) < 1)
		val = 0;
	fclose(fp);

	return val;
}

char *get_usb_manufacturer(const char *usb_node, char *buf, const int buf_size)
{
	FILE *fp;
	char *ptr;

	if((fp = open_usb_target(usb_node, "manufacturer", 0)) == NULL)
		return NULL;

	memset(buf, 0, buf_size);
	ptr = fgets(buf, buf_size, fp);
	fclose(fp);
	if(ptr == NULL)
		return NULL;

	return strsep(&ptr, "\r\n");
}

char *get_usb_product(const char *usb_node, char *buf, const int buf_size)
{
	FILE *fp;
	char *ptr;

	if((fp = open_usb_target(usb_node, "product", 0)) == NULL)
		return NULL;

	memset(buf, 0, buf_size);
	ptr = fgets(buf, buf_size, fp);
	fclose(fp);
	if(ptr == NULL)
		return NULL;

	return strsep(&ptr, "\r\n");
}

char *get_usb_serial(const char *usb_node, char *buf, const int buf_size)
{
	FILE *fp;
	char *ptr;

	if((fp = open_usb_target(usb_node, "serial", 0)) == NULL)
		return NULL;

	memset(buf, 0, buf_size);
	ptr = fgets(buf, buf_size, fp);
	fclose(fp);
	if(ptr == NULL)
		return NULL;

	return strsep(&ptr, "\r\n");
}

char *get_usb_speed(const char *usb_node, char *buf, const int buf_size)
{
	FILE *fp;
	char *ptr;

	if((fp = open_usb_target(usb_node, "speed", 0)) == NULL)
		return NULL;

	memset(buf, 0, buf_size);
	ptr = fgets(buf, buf_size, fp);
	fclose(fp);
	if(ptr == NULL)
		return NULL;

	return strsep(&ptr, "\r\n");
}

int get_usb_interface_number(const char *usb_node)
{
	FILE *fp;
	int val;

	if((fp = open_usb_target(usb_node, "bNumInterfaces", 0)) == NULL)
		return 0;

	if(fscanf(fp, "%d", &val) < 1)
		val = 0;
	fclose(fp);

	return val;
}

int get_usb_interface_order(const char *interface_name)
{
	FILE *fp;
	int val;

	// Sometimes the class file would be built slowly, so try again during MAX_WAIT_FILE
	if((fp = open_usb_target(interface_name, "bInterfaceNumber", MAX_WAIT_FILE)) == NULL)
		return -1;

	if(fscanf(fp, "%d", &val) < 1)
		val = -1;
	fclose(fp);

	return val;
}

char *get_usb_interface_class(const char *interface_name, char *buf, const int buf_size)
{
	FILE *fp;
	char *ptr;

	// Sometimes the class file would be built slowly, so try again during MAX_WAIT_FILE
	if((fp = open_usb_target(interface_name, "bInterfaceClass", MAX_WAIT_FILE)) == NULL){
		usb_dbg("(%s): Fail to open the class file really!\n", interface_name);
		return NULL;
	}

	memset(buf, 0, buf_size);
	ptr = fgets(buf, buf_size, fp);
	fclose(fp);
	if(ptr == NULL)
		return NULL;

	return strsep(&ptr, "\r\n");
}

char *get_usb_interface_subclass(const char *interface_name, char *buf, const int buf_size)
{
	FILE *fp;
	char *ptr;

	// Sometimes the class file would be built slowly, so try again during MAX_WAIT_FILE
	if((fp = open_usb_target(interface_name, "bInterfaceSubClass", MAX_WAIT_FILE)) == NULL){
		usb_dbg("(%s): Fail to open the class file really!\n", interface_name);
		return NULL;
	}

	memset(buf, 0, buf_size);
	ptr = fgets(buf, buf_size, fp);
	fclose(fp);
	if(ptr == NULL)
		return NULL;

	return strsep(&ptr, "\r\n");
}

int get_interface_numendpoints(const char *interface_name)
{
	FILE *fp;
	int val;

	if((fp = open_usb_target(interface_name, "bNumEndpoints", 0)) == NULL)
		return 0;

	if(fscanf(fp, "%d", &val) < 1)
		val = 0;
	fclose(fp);

	return val;
}

int get_interface_Int_endpoint(const char *interface_name)
{
	FILE *fp;
	char interface_path[128], bmAttributes_file[128], buf[8], *ptr;
	DIR *interface_dir = NULL;
	struct dirent *end_name;
	int bNumEndpoints, end_count, got_Int = 0;

	if(interface_name == NULL || get_usb_port_by_string(interface_name, buf, sizeof(buf)) == NULL){
		usb_dbg("(%s): The device is not a interface.\n", interface_name);
		return 0;
	}

	snprintf(interface_path, sizeof(interface_path), "%s/%s", USB_DEVICE_PATH, interface_name);
	if((interface_dir = opendir(interface_path)) == NULL){
		usb_dbg("(%s): Fail to open dir: %s.\n", interface_name, interface_path);
		return 0;
	}

	// Get bNumEndpoints.
	bNumEndpoints = get_interface_numendpoints(interface_name);
	if(bNumEndpoints <= 0){
		usb_dbg("(%s): No endpoints: %d.\n", interface_name, bNumEndpoints);
		goto leave;
	}
	else if(bNumEndpoints == 1){ // ex: GL04P
		usb_dbg("(%s): It's a little impossible to be the control interface with a endpoint.\n", interface_name);
		goto leave;
	}

	end_count = 0;
	while((end_name = readdir(interface_dir)) != NULL){
		if(strncmp(end_name->d_name, "ep_", 3))
			continue;

		++end_count;

		snprintf(bmAttributes_file, sizeof(bmAttributes_file), "%s/%s/bmAttributes", interface_path, end_name->d_name);

		if((fp = fopen(bmAttributes_file, "r")) == NULL){
			usb_dbg("(%s): Fail to open file: %s.\n", interface_name, bmAttributes_file);
			continue;
		}

		memset(buf, 0, sizeof(buf));
		ptr = fgets(buf, sizeof(buf), fp);
		fclose(fp);
		if(ptr == NULL)
			goto leave;

		if(!strncmp(buf, "03", 2)){
			got_Int = 1;
			break;
		}
		else if(end_count == bNumEndpoints)
			break;
	}

leave:
	closedir(interface_dir);

	return got_Int;
}

// 1: add, 0: remove.
int check_hotplug_action(const char *action)
{
	if(!strcmp(action, "remove"))
		return 0;
	else
		return 1;
}

int set_usb_common_nvram(const char *action, const char *device_name, const char *usb_node, const char *known_type)
{
	char type[16], vid[8], pid[8], manufacturer[256], product[256], serial[256], speed[256];
	char been_type[16], *dyn_type;
	char been_act[16];
	char port_path[8];
	char tmp[100], prefix[32];
	char *ptr, cmd[128], ret[PATH_MAX];
	int len;
	unsigned int val;
#ifdef RTCONFIG_USB_MODEM
	int modem_unit, wan_unit;
	char tmp2[100], prefix2[32];
#endif
	FILE *fp;

	if(!action || !(*action))
		return 0;

	if(!device_name || !(*device_name))
		return 0;

	if(!usb_node || !(*usb_node))
		return 0;

	//usb_dbg("%s %s %s %s.\n", action, device_name, usb_node, known_type);

	if(get_path_by_node(usb_node, port_path, sizeof(port_path)) == NULL){
		usb_dbg("(%s): Fail to get usb path.\n", usb_node);
		return 0;
	}

	if(!check_hotplug_action(action)){
		if(known_type == NULL){
			snprintf(prefix, sizeof(prefix), "usb_path%s", port_path);
			snprintf(been_type, sizeof(been_type), "%s", nvram_safe_get(prefix));

			if(strcmp(been_type, "storage") || strlen(nvram_safe_get(strcat_r(prefix, "_speed", tmp))) <= 0)
				nvram_unset(prefix);
			if(strlen(nvram_safe_get(strcat_r(prefix, "_vid", tmp))) > 0)
				nvram_unset(tmp);
			if(strlen(nvram_safe_get(strcat_r(prefix, "_pid", tmp))) > 0)
				nvram_unset(tmp);
			if(strlen(nvram_safe_get(strcat_r(prefix, "_manufacturer", tmp))) > 0)
				nvram_unset(tmp);
			if(strlen(nvram_safe_get(strcat_r(prefix, "_product", tmp))) > 0)
				nvram_unset(tmp);
			if(strlen(nvram_safe_get(strcat_r(prefix, "_serial", tmp))) > 0)
				nvram_unset(tmp);
			if(strcmp(been_type, "storage") && strlen(nvram_safe_get(strcat_r(prefix, "_speed", tmp))) > 0)
				nvram_unset(tmp);
			if(strlen(nvram_safe_get(strcat_r(prefix, "_node", tmp))) > 0)
				nvram_unset(tmp);
			if(strlen(nvram_safe_get(strcat_r(prefix, "_act_def", tmp))) > 0)
				nvram_unset(tmp);
		}
		else{
			if(!strcmp(known_type, "printer")
					|| !strcmp(known_type, "storage")){
				usb_dbg("clean %s %s %s.\n", device_name, usb_node, known_type);
				unset_usb_nvram(port_path);
			}
		}
	}
	else{
		if(known_type != NULL)
			dyn_type = (char *)known_type;
		else if(get_device_type_by_node(usb_node, type, sizeof(type), NULL, 0) != NULL)
			dyn_type = type;
		else // unknown device.
			return 0;

		snprintf(prefix, sizeof(prefix), "usb_path%s", port_path);
		snprintf(been_type, sizeof(been_type), "%s", nvram_safe_get(prefix));
		snprintf(been_act, sizeof(been_act), "%s", nvram_safe_get(strcat_r(prefix, "_act", tmp)));
		if(strlen(been_type) > 0){
#ifdef RTCONFIG_USB_PRINTER
			if(!strcmp(been_type, "printer")){ // Top priority
				return 0;
			}
			else
#endif
#ifdef RTCONFIG_USB_MODEM
			if(!strcmp(been_type, "modem")){ // 2nd priority
				if(!strcmp(dyn_type, "modem")){
					if(*been_act && !strncmp(device_name, "tty", 3))
						return 0;
				}
				else
#ifdef RTCONFIG_USB_PRINTER
				if(strcmp(dyn_type, "printer"))
#endif
					return 0;
			}
			else
#endif
			if(!strcmp(been_type, "storage")){
#if defined(RTCONFIG_USB_PRINTER) || defined(RTCONFIG_USB_MODEM)
				if(
#ifdef RTCONFIG_USB_PRINTER
						strcmp(dyn_type, "printer")
#else
						1
#endif
						&&
#ifdef RTCONFIG_USB_MODEM
						strcmp(dyn_type, "modem")
#else
						1
#endif
						)
#endif
					return 0;
			}
			else
			{ // unknown device.
				return 0;
			}
		}

		usb_dbg("%s: start to set usb_path...\n", device_name);

		nvram_set(prefix, dyn_type);

#ifdef RTCONFIG_USB_PRINTER
		if(!strcmp(dyn_type, "printer")){
			snprintf(cmd, sizeof(cmd), "%s/%s", SYS_USB, device_name);
			if(d_exists(cmd)){
				snprintf(tmp, sizeof(tmp), "usb_path_%s", device_name);
				nvram_set(tmp, usb_node);
			}

			if(get_device_type_by_device(device_name) == DEVICE_TYPE_PRINTER)
				nvram_set(strcat_r(prefix, "_act", tmp), device_name);
		}
		else
#endif
#ifdef RTCONFIG_USB_MODEM
		if(!strcmp(dyn_type, "modem")){
			for(modem_unit = MODEM_UNIT_FIRST; modem_unit < MODEM_UNIT_MAX; ++modem_unit){
				usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));

				snprintf(ret, sizeof(ret), "%s", nvram_safe_get(strcat_r(prefix2, "act_path", tmp2)));
				if(!(*ret) || !strcmp(ret, usb_node))
					break;
			}

			if(modem_unit != MODEM_UNIT_MAX){
				if(!(*ret)){
					nvram_set(strcat_r(prefix2, "act_path", tmp2), usb_node);
					if((val = get_usb_vid(usb_node)) != 0){
						snprintf(vid, sizeof(vid), "%u", val);
						nvram_set(strcat_r(prefix2, "act_vid", tmp2), vid);
					}
					if((val = get_usb_pid(usb_node)) != 0){
						snprintf(pid, sizeof(pid), "%u", val);
						nvram_set(strcat_r(prefix2, "act_pid", tmp2), pid);
					}
				}

				if((val = get_device_type_by_device(device_name)) == DEVICE_TYPE_MODEM
#ifdef RTCONFIG_USB_BECEEM
						|| val == DEVICE_TYPE_BECEEM
#endif
						){
					nvram_set(strcat_r(prefix, "_act", tmp), device_name);
					nvram_set(strcat_r(prefix2, "act_dev", tmp2), device_name);
#ifdef RTCONFIG_MODEM_BRIDGE
					if(!(sw_mode() == SW_MODE_AP && nvram_get_int("modem_bridge")))
#endif
					{
						if((wan_unit = get_wanunit_by_type(get_wantype_by_modemunit(modem_unit))) != WAN_UNIT_NONE){
							snprintf(prefix2, sizeof(prefix2), "wan%d_", wan_unit);
							nvram_set(strcat_r(prefix2, "ifname", tmp2), device_name);
						}
					}
				}
			}
		}
		else
#endif
		if(!strcmp(dyn_type, "storage")){
			// for ATE.
			snprintf(cmd, sizeof(cmd), "%s/%s", SYS_BLOCK, device_name);
			if(d_exists(cmd)){
				snprintf(tmp, sizeof(tmp), "usb_path_%s", device_name);
				nvram_set(tmp, usb_node);

				snprintf(cmd, sizeof(cmd), "/bin/ls -d %s/%s/%s* 2>/dev/null |sort", SYS_BLOCK, device_name, device_name);
				if((fp = popen(cmd, "r")) != NULL){
					int got_partitions = 0;

					while(fgets(ret, sizeof(ret), fp) != NULL){
						++got_partitions;

						if((ptr = rindex(ret, '/')) != NULL){
							++ptr;
							len = strlen(ptr);
							ptr[len-1] = 0;

							snprintf(tmp, sizeof(tmp), "usb_path_%s", ptr);
							nvram_set(tmp, usb_node);

							if(got_partitions == 1)
								nvram_set(strcat_r(prefix, "_fs_path0", tmp), ptr);
						}
					}

					if(!got_partitions)
						nvram_set(strcat_r(prefix, "_fs_path0", tmp), device_name);

					pclose(fp);
				}
			}

			if((val = get_device_type_by_device(device_name)) == DEVICE_TYPE_DISK
					|| val == DEVICE_TYPE_CD
					)
				nvram_set(strcat_r(prefix, "_act", tmp), device_name);
		}

		strcat_r(prefix, "_vid", tmp);
		if ((val = get_usb_vid(usb_node)) != 0) {
			snprintf(vid, sizeof(vid), "%04x", val);
			nvram_set(tmp, vid);
		}

		strcat_r(prefix, "_pid", tmp);
		if ((val = get_usb_pid(usb_node)) != 0) {
			snprintf(pid, sizeof(pid), "%04x", val);
			nvram_set(tmp, pid);
		}

		strcat_r(prefix, "_manufacturer", tmp);
		if(get_usb_manufacturer(usb_node, manufacturer, 256) != NULL && strlen(manufacturer) > 0)
			nvram_set(tmp, manufacturer);

		strcat_r(prefix, "_product", tmp);
		if(get_usb_product(usb_node, product, 256) != NULL && strlen(product) > 0)
			nvram_set(tmp, product);

		strcat_r(prefix, "_serial", tmp);
		if(get_usb_serial(usb_node, serial, 256) != NULL && strlen(serial) > 0)
			nvram_set(tmp, serial);

		strcat_r(prefix, "_speed", tmp);
#if defined(RTCONFIG_M2_SSD)
		if (isM2SSDDevice(device_name)) {
			char *spd;

			spd = file2str("/sys/class/ata_link/link1/sata_spd");
			if (spd) {
				if (!strncmp(spd, "1.5", 3))
					nvram_set(tmp, "1500");
				else if (!strncmp(spd, "3.0", 3))
					nvram_set(tmp, "3000");
				else if (!strncmp(spd, "6.0", 3))
					nvram_set(tmp, "6000");
				else
					nvram_set(tmp, "0");

				free(spd);
			}
		} else
#endif
		if(get_usb_speed(usb_node, speed, 256) != NULL && strlen(speed) > 0)
			nvram_set(tmp, speed);

		strcat_r(prefix, "_node", tmp);
		nvram_set(tmp, usb_node);
		usb_dbg("%s %s %s %s: End.\n", action, device_name, usb_node, dyn_type);
	}

	return 0;
}

char *find_port_paths_by_nvram(char *target_type, char *str, int str_len){
	char *ptr_type;
	int i;
	char cmd[128], ret[PATH_MAX];
	char *p, w, *ptr;
	int len;
	FILE *fp;
	char port_path[8];
	int first_set = 1;

	memset(str, 0, str_len);
	len = 0;
	ptr = str;

	for(i = 0; supported_types[i] != NULL; ++i){
		ptr_type = supported_types[i];

		if(target_type != NULL && strcmp(ptr_type, target_type))
			continue;

		snprintf(cmd, sizeof(cmd), "nvram show |grep =%s$", ptr_type);
		if((fp = popen(cmd, "r")) == NULL)
			continue;

		while(fgets(ret, sizeof(ret), fp) != NULL){
			if((p = strchr(ret, '=')) != NULL){
				w = *p;
				*p = '\0';
			}
			else
				w = -1;

			if(strncmp(ret, "usb_path", 8) || strlen(ret) == strlen("usb_path")){
				if(w != -1)
					*p = w;
				continue;
			}

			snprintf(port_path, sizeof(port_path), "%s", ret+strlen("usb_path"));

			if(first_set){
				first_set = 0;

				snprintf(ptr, str_len-len, ">");
				len = strlen(str);
				ptr = str+len;
			}

			snprintf(ptr, str_len-len, "%s>", port_path);
			len = strlen(str);
			ptr = str+len;

			if(w != -1)
				*p = w;
		}
		pclose(fp);
	}

	if(!len)
		return NULL;

	return str;
}

char *find_port_paths_by_dir(char *str, int str_len){
	DIR *dir;
	char *colon;
	char usb_node[16], port_path[8], dev_type[16];
	struct dirent *d;
	char *ptr;
	int len;
	char target[16];
	int first_set = 1;

	if((dir = opendir(USB_DEVICE_PATH)) == NULL)
		return NULL;

	memset(str, 0, str_len);
	len = 0;
	ptr = str;

	while((d = readdir(dir)) != NULL){
		if(!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
			continue;
		/* skip non-interface and root-hub*/
		if(!(colon = strchr(d->d_name, ':')) || !strncmp(d->d_name + 1, "-0:", 3))
			continue;

		if(!get_device_type_by_interface(d->d_name, dev_type, sizeof(dev_type), NULL, 0) || !(*dev_type))
			continue;

		// Get USB node.
		if(!get_usb_node_by_string(d->d_name, usb_node, sizeof(usb_node)))
			continue;

		if(!get_path_by_node(usb_node, port_path, sizeof(port_path)))
			continue;

		snprintf(target, sizeof(target), ">%s>", port_path);
		if(strstr(str, target))
			continue;

		if(first_set){
			first_set = 0;

			snprintf(ptr, str_len-len, ">");
			len = strlen(str);
			ptr = str+len;
		}

		snprintf(ptr, str_len-len, "%s>", port_path);
		len = strlen(str);
		ptr = str+len;
	}
	closedir(dir);

	if(!len)
		return NULL;

	return str;
}

int detect_usb_devices(){
	char port_paths_nvram[PATH_MAX], port_paths_dir[PATH_MAX];
	char port_path[256], *next_word = NULL;
	char target[16];
	DIR *dir;
	struct dirent *d;
	char *colon;
	char dev_type[16], dev_name[32];
	char usb_node[16];

	// Unset the nvram for the removed devices
	find_port_paths_by_dir(port_paths_dir, sizeof(port_paths_dir));
	find_port_paths_by_nvram(NULL, port_paths_nvram, sizeof(port_paths_nvram));
	foreach_62(port_path, port_paths_nvram, next_word){
		snprintf(target, sizeof(target), ">%s>", port_path);
		if(*port_paths_dir && strstr(port_paths_dir, target))
			continue;

		usb_dbg("Clean the USB device on usb_path%s...\n", port_path);
		unset_usb_nvram(port_path);
	}

	// Set the nvram for added devices
	if(!(*port_paths_dir))
		return -1;
	if((dir = opendir(USB_DEVICE_PATH)) == NULL)
		return -2;

	while((d = readdir(dir)) != NULL){
		if(!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
			continue;
		/* skip non-interface and root-hub*/
		if(!(colon = strchr(d->d_name, ':')) || !strncmp(d->d_name + 1, "-0:", 3))
			continue;

		// Get USB device.
		if(!get_device_type_by_interface(d->d_name, dev_type, sizeof(dev_type), dev_name, sizeof(dev_name)) || !(*dev_type) || !(*dev_name))
			continue;

		// Get USB node.
		if(!get_usb_node_by_string(d->d_name, usb_node, sizeof(usb_node)))
			continue;

		//usb_dbg("Get the USB device(%s) on usb_node %s...\n", dev_name, usb_node);
		set_usb_common_nvram("add", dev_name, usb_node, dev_type);
	}
	closedir(dir);

	return 0;
}

void unset_usb_nvram(char *port_path){
	char target[32], target2[32];
	int len, len2, clean_all;
	char buf[MAX_NVRAM_SPACE];
	char *name, *p, w;
	char *_n, *_p, _w[32];
#ifdef RTCONFIG_USB_MODEM
	int modem_unit;
	char *ptr;
	char tmp[100], prefix[32];
#endif

	if(port_path != NULL){
		clean_all = 0;
		snprintf(target, sizeof(target), "usb_path%s", port_path);
	}
	else{
		clean_all = 1;
		snprintf(target, sizeof(target), "usb_path");
	}
	len = strlen(target);

	snprintf(target2, sizeof(target2), "usb_path_");
	len2 = strlen(target2);

	nvram_getall(buf, sizeof(buf));
	for(name = buf; *name; name += strlen(name)+1){
		if(!strncmp(name, target, len)){
			if(strstr(name, "_diskmon"))
				continue;

			if((p = strchr(name, '=')) != NULL){
				w = *p;
				*p = '\0';
			}
			else
				w = -1;

			if(!clean_all){
				memset(_w, 0, sizeof(_w));
				_n = name+8; // strlen(usb_path)
				if((_p = strchr(_n, '_')) != NULL)
					strncpy(_w, _n, strlen(_n)-strlen(_p));
				else
					strncpy(_w, _n, strlen(_n));

				if(strcmp(_w, port_path)){
					if(w != -1)
						*p = w;
					continue;
				}
			}
		}
		else if(!strncmp(name, target2, len2)){
			if((p = strchr(name, '=')) != NULL){
				w = *p;
				*p = '\0';
			}
			else
				w = -1;

			if(!clean_all){
				_p = p+1;
				if(_p && get_path_by_node(_p, _w, sizeof(_w))){
					if(strcmp(_w, port_path)){
						if(w != -1)
							*p = w;
						continue;
					}
				}
			}
		}
		else
			continue;

		nvram_unset(name);

		if(w != -1)
			*p = w;
	}

#ifdef RTCONFIG_USB_MODEM
	for(modem_unit = MODEM_UNIT_FIRST; modem_unit < MODEM_UNIT_MAX; ++modem_unit){
		usb_modem_prefix(modem_unit, prefix, sizeof(prefix));
		ptr = nvram_safe_get(strcat_r(prefix, "act_path", tmp));
		if(!(*ptr))
			continue;

		if(get_path_by_node(ptr, buf, sizeof(buf)) == NULL)
			continue;

		if(!clean_all && strcmp(port_path, buf))
			continue;

		clean_modem_state(modem_unit, 1);
	}
#endif
}

#ifdef RTCONFIG_USB_MODEM
void clean_modem_state(int modem_unit, int flag){
	int unit;
	char tmp2[100], prefix2[32];

	for(unit = MODEM_UNIT_FIRST; unit < MODEM_UNIT_MAX; ++unit){
		if(modem_unit != -1 && modem_unit != unit)
			continue;

		usb_modem_prefix(unit, prefix2, sizeof(prefix2));

		// Need to unset after the SIM is removed.
		// auto APN
		nvram_unset(strcat_r(prefix2, "auto_lines", tmp2));
		nvram_unset(strcat_r(prefix2, "auto_running", tmp2));
		nvram_unset(strcat_r(prefix2, "auto_imsi", tmp2));
		nvram_unset(strcat_r(prefix2, "auto_country", tmp2));
		nvram_unset(strcat_r(prefix2, "auto_isp", tmp2));
		nvram_unset(strcat_r(prefix2, "auto_apn", tmp2));
		nvram_unset(strcat_r(prefix2, "auto_spn", tmp2));
		nvram_unset(strcat_r(prefix2, "auto_dialnum", tmp2));
		nvram_unset(strcat_r(prefix2, "auto_user", tmp2));
		nvram_unset(strcat_r(prefix2, "auto_pass", tmp2));

		nvram_unset(strcat_r(prefix2, "act_signal", tmp2));
		nvram_unset(strcat_r(prefix2, "act_cellid", tmp2));
		nvram_unset(strcat_r(prefix2, "act_lac", tmp2));
		nvram_unset(strcat_r(prefix2, "act_rsrq", tmp2));
		nvram_unset(strcat_r(prefix2, "act_rsrp", tmp2));
		nvram_unset(strcat_r(prefix2, "act_rssi", tmp2));
		nvram_unset(strcat_r(prefix2, "act_sinr", tmp2));
		nvram_unset(strcat_r(prefix2, "act_band", tmp2));
		nvram_unset(strcat_r(prefix2, "act_operation", tmp2));
		nvram_unset(strcat_r(prefix2, "act_provider", tmp2));
		nvram_unset(strcat_r(prefix2, "act_imsi", tmp2));
		nvram_unset(strcat_r(prefix2, "act_iccid", tmp2));
		nvram_unset(strcat_r(prefix2, "act_auth", tmp2));
		nvram_unset(strcat_r(prefix2, "act_auth_pin", tmp2));
		nvram_unset(strcat_r(prefix2, "act_auth_puk", tmp2));
		nvram_unset(strcat_r(prefix2, "act_ip", tmp2));
		nvram_unset(strcat_r(prefix2, "act_ipv6", tmp2));
	}

	nvram_unset("modem_sim_order");
	nvram_unset("modem_bytes_rx");
	nvram_unset("modem_bytes_tx");
	nvram_unset("modem_bytes_rx_reset");
	nvram_unset("modem_bytes_tx_reset");

	nvram_unset("modem_sms_alert_send");
	nvram_unset("modem_sms_limit_send");

	if(flag == 2)
		return;

	for(unit = MODEM_UNIT_FIRST; unit < MODEM_UNIT_MAX; ++unit){
		if(modem_unit != -1 && modem_unit != unit)
			continue;

		usb_modem_prefix(unit, prefix2, sizeof(prefix2));

		if(flag){
			nvram_unset(strcat_r(prefix2, "act_path", tmp2));
			nvram_unset(strcat_r(prefix2, "act_type", tmp2));
			nvram_unset(strcat_r(prefix2, "act_dev", tmp2));
		}

		// modem.
		nvram_unset(strcat_r(prefix2, "act_int", tmp2));
		nvram_unset(strcat_r(prefix2, "act_bulk", tmp2));
		nvram_unset(strcat_r(prefix2, "act_vid", tmp2));
		nvram_unset(strcat_r(prefix2, "act_pid", tmp2));
		nvram_unset(strcat_r(prefix2, "act_sim", tmp2));
		nvram_unset(strcat_r(prefix2, "act_imei", tmp2));
		nvram_unset(strcat_r(prefix2, "act_tx", tmp2));
		nvram_unset(strcat_r(prefix2, "act_rx", tmp2));
		nvram_unset(strcat_r(prefix2, "act_hwver", tmp2));
		nvram_unset(strcat_r(prefix2, "act_swver", tmp2));
		nvram_unset(strcat_r(prefix2, "act_scanning", tmp2));
		nvram_unset(strcat_r(prefix2, "act_startsec", tmp2));
		nvram_unset(strcat_r(prefix2, "act_simdetect", tmp2));
		nvram_unset(strcat_r(prefix2, "act_num", tmp2));
#ifdef RTCONFIG_USB_SMS_MODEM
		nvram_unset(strcat_r(prefix2, "act_smsc", tmp2));
#endif
	}

	// modem state.
	nvram_unset("g3state_pin");
	nvram_unset("g3state_z");
	nvram_unset("g3state_q0");
	nvram_unset("g3state_cd");
	nvram_unset("g3state_class");
	nvram_unset("g3state_mode");
	nvram_unset("g3state_apn");
	nvram_unset("g3state_dial");
	nvram_unset("g3state_conn");

	// modem error.
	nvram_unset("g3err_pin");
	nvram_unset("g3err_apn");
	nvram_unset("g3err_conn");
	nvram_unset("g3err_imsi");
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
int hadWWANModule(void)
{
	char target_file[128];
	DIR *module_dir;

	snprintf(target_file, sizeof(target_file), "%s/usb_wwan", SYS_MODULE);
	if((module_dir = opendir(target_file)) != NULL){
		closedir(module_dir);
		return 1;
	}

	return 0;
}
#endif

int hadOptionModule(void)
{
	char target_file[128];
	DIR *module_dir;

	snprintf(target_file, sizeof(target_file), "%s/option", SYS_MODULE);
	if((module_dir = opendir(target_file)) != NULL){
		closedir(module_dir);
		return 1;
	}

	return 0;
}

int hadSerialModule(void)
{
	char target_file[128];
	DIR *module_dir;

	snprintf(target_file, sizeof(target_file), "%s/usbserial", SYS_MODULE);
	if((module_dir = opendir(target_file)) != NULL){
		closedir(module_dir);
		return 1;
	}

	return 0;
}

int hadACMModule(void)
{
	char target_file[128];
	DIR *module_dir;

	snprintf(target_file, sizeof(target_file), "%s/cdc_acm", SYS_MODULE);
	if((module_dir = opendir(target_file)) != NULL){
		closedir(module_dir);
		return 1;
	}

	return 0;
}

// return 1 when there is a bound device.
int hadRNDISModule(){
	char target_file[128];
	DIR *module_dir;

	snprintf(target_file, sizeof(target_file), "%s/rndis_host", SYS_MODULE);
	if((module_dir = opendir(target_file)) != NULL){
		closedir(module_dir);
		return 1;
	}

	return 0;
}

#ifdef RTCONFIG_USB_BECEEM
int hadBeceemModule(void){
	char target_file[128];
	DIR *module_dir;

	snprintf(target_file, sizeof(target_file), "%s/drxvi314", SYS_MODULE);
	if((module_dir = opendir(target_file)) != NULL){
		closedir(module_dir);
		return 1;
	}
	else
		return 0;
}

int hadGCTModule(void){
	char target_file[128];
	DIR *module_dir;

	snprintf(target_file, sizeof(target_file), "%s/tun", SYS_MODULE);
	if((module_dir = opendir(target_file)) != NULL){
		closedir(module_dir);
		return 1;
	}
	else
		return 0;
}
#endif

int isTTYNode(const char *device_name)
{
	if(strncmp(device_name, "tty", 3))
		return 0;

	return 1;
}

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

int isSerialInterface(const char *interface_name, const int specifics, const unsigned int vid, const unsigned int pid)
{
	int ret = 1;
	char interface_class[4];
#if defined(RTCONFIG_QCA)
	char iface_class[4], bus[64], *p, path[sizeof("/sys/bus/usb/devices/1-1.2.3.4.5.6.7:1-1/bInterfaceClassXXXXXX")];
	DIR *d;
	struct dirent *de;
#endif

	if(specifics){
		// HTC M8
		if(vid == 0x0bb4 && (pid == 0x0f64 || pid == 0x0f60))
			return 0;
	}

	if(get_usb_interface_class(interface_name, interface_class, 4) == NULL)
		return 0;

	if(strcmp(interface_class, "ff"))
		return 0;

#if defined(RTCONFIG_QCA)
	/* Find all interface of the USB device, if any one of it is 7, which is USB printer, return 0. */
	strlcpy(bus, interface_name, sizeof(bus));
	if ((p = strrchr(bus, ':')) == NULL)
		return ret;

	/* Remove interface and leave ':' for comparison. */
	*(p + 1) = '\0';
	if ((d = opendir(USB_DEVICE_PATH)) == NULL)
		return ret;
	while ((de = readdir(d)) != NULL) {
		if (!strstr(de->d_name, bus))
			continue;
		snprintf(path, sizeof(path), "%s/%s/bInterfaceClass", USB_DEVICE_PATH, de->d_name);
		if (f_read_string(path, iface_class, sizeof(iface_class)) <= 0) {
			continue;
		}
		if (safe_atoi(iface_class) == 7) {
			ret = 0;
			break;
		}
	}
	closedir(d);
#endif

	return ret;
}

int isACMInterface(const char *interface_name, const int specifics, const unsigned int vid, const unsigned int pid)
{
	char buf[PATH_MAX];

	if(specifics){
		// HTC M8
		if(vid == 0x0bb4 && (pid == 0x0f64 || pid == 0x0f60))
			return 0;
	}

	snprintf(buf, sizeof(buf), "%s/cdc_acm/%s", USB_DRIVER_PATH, interface_name);
	if(l_exists(buf))
		return 1;

	snprintf(buf, sizeof(buf), "%s/acm/%s", USB_DRIVER_PATH, interface_name);
	if(l_exists(buf))
		return 1;

	return 0;
}

int isRNDISInterface(const char *interface_name, const unsigned int vid, const unsigned int pid)
{
	char buf[PATH_MAX];

	if(vid == 0x1076 && (pid == 0x8002 || pid == 0x8003))
		return 1;

	snprintf(buf, sizeof(buf), "%s/rndis_host/%s", USB_DRIVER_PATH, interface_name);
	if(l_exists(buf))
		return 1;

	return 0;
}

int isQMIInterface(const char *interface_name)
{
	char buf[PATH_MAX];

	snprintf(buf, sizeof(buf), "%s/qmi_wwan/%s", USB_DRIVER_PATH, interface_name);
	if(l_exists(buf))
		return 1;

	return 0;
}

int isGOBIInterface(const char *interface_name)
{
	char buf[PATH_MAX];

	snprintf(buf, sizeof(buf), "%s/GobiNet/%s", USB_DRIVER_PATH, interface_name);
	if(l_exists(buf))
		return 1;

	return 0;
}

int isCDCETHInterface(const char *interface_name)
{
	char buf[PATH_MAX];

	snprintf(buf, sizeof(buf), "%s/cdc_ether/%s", USB_DRIVER_PATH, interface_name);
	if(l_exists(buf))
		return 1;

	return 0;
}

int isNCMInterface(const char *interface_name)
{
	char buf[PATH_MAX];

	snprintf(buf, sizeof(buf), "%s/cdc_ncm/%s", USB_DRIVER_PATH, interface_name);
	if(l_exists(buf))
		return 1;

	return 0;
}

int isASIXInterface(const char *interface_name)
{
	char buf[PATH_MAX];

	snprintf(buf, sizeof(buf), "%s/asix/%s", USB_DRIVER_PATH, interface_name);
	if(l_exists(buf))
		return 1;

	snprintf(buf, sizeof(buf), "%s/ax88179_178a/%s", USB_DRIVER_PATH, interface_name);
	if(l_exists(buf))
		return 1;

	return 0;
}

#ifdef RTCONFIG_USB_BECEEM
int isGCTInterface(const char *interface_name){
	char interface_class[4];

	if(get_usb_interface_class(interface_name, interface_class, 4) == NULL)
		return 0;

	if(strcmp(interface_class, "0a"))
		return 0;

	return 1;
}
#endif

// 0: no modem, 1: has modem
int is_usb_modem_ready(int wan_type)
{
	int unit_wan;
	char prefix[32], tmp[100];
	int unit;
	char prefix2[32], tmp2[100];
	char port_paths_nvram[PATH_MAX];
	char usb_act[8];
	char usb_node[32], port_path[8];

	if(nvram_match("modem_enable", "0"))
		return 0;

	unit = get_modemunit_by_type(wan_type);
	unit_wan = get_wanunit_by_type(wan_type);
	usb_modem_prefix(unit, prefix2, sizeof(prefix2));

	find_port_paths_by_nvram("modem", port_paths_nvram, sizeof(port_paths_nvram));

	if(snprintf(usb_node, sizeof(usb_node), "%s", nvram_safe_get(strcat_r(prefix2, "act_path", tmp2))) <= 0
			|| !(*port_paths_nvram)
			)
	{
		detect_usb_devices();
		if(snprintf(usb_node, sizeof(usb_node), "%s", nvram_safe_get(strcat_r(prefix2, "act_path", tmp2))) <= 0)
			return 0;
	}

	if(get_path_by_node(usb_node, port_path, sizeof(port_path)) == NULL)
		return 0;

	snprintf(prefix, sizeof(prefix), "usb_path%s", port_path);
	snprintf(usb_act, sizeof(usb_act), "%s", nvram_safe_get(strcat_r(prefix, "_act", tmp)));

	if(nvram_match(prefix, "modem") && *usb_act)
		return 1;

	return 0;
}
#endif // RTCONFIG_USB_MODEM

#ifdef RTCONFIG_USB_PRINTER
int hadPrinterModule(void)
{
	char target_file[128];
	DIR *module_dir;

	snprintf(target_file, sizeof(target_file), "%s/usblp", SYS_MODULE);
	if((module_dir = opendir(target_file)) != NULL){
		closedir(module_dir);
		return 1;
	}
	else
		return 0;
}

int hadPrinterInterface(const char *usb_node)
{
	char check_usb_node[32], device_name[4];
	int printer_order, got_printer = 0;

	for(printer_order = 0; printer_order < MAX_USB_PRINTER_NUM; ++printer_order){
		snprintf(device_name, sizeof(device_name), "lp%d", printer_order);

		if(get_usb_node_by_device(device_name, check_usb_node, 32) == NULL)
			continue;

		if(!strcmp(usb_node, check_usb_node)){
			got_printer = 1;

			break;
		}
	}

	return got_printer;
}

int isPrinterInterface(const char *interface_name)
{
	char interface_class[4];

	if(get_usb_interface_class(interface_name, interface_class, 4) == NULL)
		return 0;

	if(strcmp(interface_class, "07"))
		return 0;

	return 1;
}
#endif // RTCONFIG_USB_PRINTER

int isStorageInterface(const char *interface_name)
{
	char interface_class[4];

	if(get_usb_interface_class(interface_name, interface_class, 4) == NULL)
		return 0;

	if(strcmp(interface_class, "08"))
		return 0;

	return 1;
}

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

char *find_sg_of_device(const char *device_name, char *buf, const int buf_size)
{
	DIR *dp;
	struct dirent *file;
	char target_usb_port[32], check_usb_port[32];
	int got_sg = 0;

	if(get_usb_port_by_device(device_name, target_usb_port, sizeof(target_usb_port)) == NULL)
		return NULL;

	if((dp = opendir(SYS_SG)) == NULL)
		return NULL;

	while((file = readdir(dp)) != NULL){
		if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
			continue;

		if(strncmp(file->d_name, "sg", 2))
			continue;

		if(get_usb_port_by_device(file->d_name, check_usb_port, sizeof(check_usb_port)) == NULL)
			return NULL;

		if(!strcmp(target_usb_port, check_usb_port)){
			snprintf(buf, buf_size, "%s", file->d_name);
			got_sg = 1;
			break;
		}
	}
	closedir(dp);

	if(!got_sg)
		return NULL;

	return buf;
}

#ifdef RTCONFIG_INTERNAL_GOBI
char *get_gobi_portpath(){
#ifdef RT4GAC68U
	//return "3"; old layout. there is the USB 2.0 port.
	return "2";
#else // 4G-AC55U, 4G-AC53U
	return "2";
#endif

	return "";
}
#endif
