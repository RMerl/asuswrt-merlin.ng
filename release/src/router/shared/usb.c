/*

	Tomato Firmware
	USB Support Module

*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <dirent.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <linux/version.h>
#include <limits.h>		//PATH_MAX, LONG_MIN, LONG_MAX

#include <bcmnvram.h>
#include <bcmdevs.h>
#include <wlutils.h>

#include <disk_io_tools.h>

#include "shutils.h"
#include "shared.h"

#include <linux/version.h>
#ifndef LINUX_KERNEL_VERSION
#define LINUX_KERNEL_VERSION LINUX_VERSION_CODE
#endif

/* Execute a function for each disc partition on the specified controller.
 *
 * Directory /dev/discs/ looks like this:
 * disc0 -> ../scsi/host0/bus0/target0/lun0/
 * disc1 -> ../scsi/host1/bus0/target0/lun0/
 * disc2 -> ../scsi/host2/bus0/target0/lun0/
 * disc3 -> ../scsi/host2/bus0/target0/lun1/
 *
 * Scsi host 2 supports multiple drives.
 * Scsi host 0 & 1 support one drive.
 *
 * For attached drives, like this.  If not attached, there is no "part#" item.
 * Here, only one drive, with 2 partitions, is plugged in.
 * /dev/discs/disc0/disc
 * /dev/discs/disc0/part1
 * /dev/discs/disc0/part2
 * /dev/discs/disc1/disc
 * /dev/discs/disc2/disc
 *
 * Which is the same as:
 * /dev/scsi/host0/bus0/target0/lun0/disc
 * /dev/scsi/host0/bus0/target0/lun0/part1
 * /dev/scsi/host0/bus0/target0/lun0/part2
 * /dev/scsi/host1/bus0/target0/lun0/disc
 * /dev/scsi/host2/bus0/target0/lun0/disc
 * /dev/scsi/host2/bus0/target0/lun1/disc
 *
 * Implementation notes:
 * Various mucking about with a disc that just got plugged in or unplugged
 * will make the scsi subsystem try a re-validate, and read the partition table of the disc.
 * This will make sure the partitions show up.
 *
 * It appears to try to do the revalidate and re-read & update the partition
 * information when this code does the "readdir of /dev/discs/disc0/?".  If the
 * disc has any mounted partitions the revalidate will be rejected.  So the
 * current partition info will remain.  On an unplug event, when it is doing the
 * readdir's, it will try to do the revalidate as we are doing the readdir's.
 * But luckily they'll be rejected, otherwise the later partitions will disappear as
 * soon as we get the first one.
 * But be very careful!  If something goes not exactly right, the partition entries
 * will disappear before we've had a chance to unmount from them.
 *
 * To avoid this automatic revalidation, we go through /proc/partitions looking for the partitions
 * that /dev/discs point to.  That will avoid the implicit revalidate attempt.
 * 
 * If host < 0, do all hosts.   If >= 0, it is the host number to do.
 *
 */

#ifdef RTCONFIG_USB_CDROM
/* check if the block device is cdrom device */
int is_cdrom_device(const char *discname)
{
	if (strncmp(discname, "sr", 2) == 0 && isdigit(discname[2]))
		return 1;
	return 0;
}
#endif

/* check if the block device has no partition */
int is_no_partition(const char *discname)
{
	FILE *procpt;
	char line[128], ptname[32];
	int ma, mi, sz;
	int count = 0;

#ifdef RTCONFIG_USB_CDROM
	if (is_cdrom_device(discname))
		return 1;
#endif

	if ((procpt = fopen("/proc/partitions", "r"))) {
		while (fgets(line, sizeof(line), procpt)) {
			if (sscanf(line, " %d %d %d %[^\n ]", &ma, &mi, &sz, ptname) != 4)
				continue;
			if (strstr(ptname, discname))
				count++;
		}
	}

	return (count == 1);
}

int exec_for_host(int host, int obsolete, uint flags, host_exec func)
{
	DIR *usb_dev_disc;
	char ptname[32];/* Will be: discDN_PN	 					*/
	char dsname[16];/* Will be: discDN	 					*/
	int host_no;	/* SCSI controller/host # */
	struct dirent *dp;
	FILE *prt_fp;
	int siz;
	char line[256];
	int result = 0;
	int len;
#ifdef LINUX26
	int ret;
	char hostbuf[16], device_path[PATH_MAX], linkbuf[PATH_MAX], *h;
#else
	char link[256];	/* Will be: ../scsi/host#/bus0/target0/lun#  that bfr links to. */
			/* When calling the func, will be: /dev/discs/disc#/part#	*/
	char bfr[256];	/* Will be: /dev/discs/disc#					*/
	char bfr2[128];	/* Will be: /dev/discs/disc#/disc     for the BLKRRPART.	*/
	char *cp;
	int disc_num;	/* Disc # */
	int part_num;	/* Parition # */
	char *mp;	/* Ptr to after any leading ../ path */
#endif

	_dprintf("exec_for_host(%d, %d, %d, %p)\n", host, obsolete, flags, func);
	if (!func)
		return 0;

	flags |= EFH_1ST_HOST;

#ifdef LINUX26
	/* /sys/bus/scsi/devices/X:X:X:X/block:sdX doesn't exist in kernel 3.0
	 * 1. Enumerate sub-directory, DIR, of /sys/block.
	 * 2. Skip ., .., loop*, mtdblock*, ram*, etc.
	 * 3. read DIR/device link. Check whether X:X:X:X exist. e.g.
	 *    56U: ../../devices/platform/rt3xxx-ehci/usb1/1-1/1-1:1.0/host1/target1:0:0/1:0:0:0
	 *    65U: ../../devices/pci0000:00/0000:00:00.0/0000:01:00.0/usb1/1-2/1-2:1.0/host1/target1:0:0/1:0:0:0
	 * 4. If yes, DIR would be sda, sdb, etc.
	 * 5. Search DIR in /proc/partitions.
	 */
	snprintf(hostbuf, sizeof(hostbuf), "%d:", host);
	if (!(usb_dev_disc = opendir("/sys/block")))
		return 0;
	while ((dp = readdir(usb_dev_disc))) {
		if (!strncmp(dp->d_name, "loop", 4) ||
		    !strncmp(dp->d_name, "mtdblock", 8) ||
		    !strncmp(dp->d_name, "ram", 3) ||
		    !strcmp(dp->d_name, ".") ||
		    !strcmp(dp->d_name, "..")
		   )
			continue;

		snprintf(device_path, sizeof(device_path), "/sys/block/%s", dp->d_name);
		len = readlink(device_path, linkbuf, sizeof(linkbuf) - 1);
		if (len == -1) {
			snprintf(device_path, sizeof(device_path), "/sys/block/%s/device", dp->d_name);
			len = readlink(device_path, linkbuf, sizeof(linkbuf) - 1);
			if (len == -1)
				continue;
		}
		linkbuf[len] = '\0';

		h = strstr(linkbuf, "/host");
		if (!h)	continue;
		if ((ret = sscanf(h, "/host%*d/target%*d:%*d:%*d/%d:%*d:%*d:%*d", &host_no)) != 1) {
			_dprintf("%s(): sscanf can't distinguish host_no from [%s]. ret %d\n", __func__, linkbuf, ret);
			continue;
		}
		if (host >= 0 && host != host_no)
			continue;
		snprintf(dsname, sizeof(dsname), dp->d_name);
		siz = strlen(dsname);
		flags |= EFH_1ST_DISC;
		if (!(prt_fp = fopen("/proc/partitions", "r")))
			continue;
		while (fgets(line, sizeof(line) - 2, prt_fp)) {
			if (sscanf(line, " %*s %*s %*s %s", ptname) != 1)
				continue;

			if (!strncmp(ptname, dsname, siz)) {
				if (!strcmp(ptname, dsname) && !is_no_partition(dsname))
					continue;
				snprintf(line, sizeof(line), "/dev/%s", ptname);
				result = (*func)(line, host_no, dsname, ptname, flags) || result;
				flags &= ~(EFH_1ST_HOST | EFH_1ST_DISC);
			}
		}
		fclose(prt_fp);
	}
	closedir(usb_dev_disc);

#else	/* !LINUX26 */

	if ((usb_dev_disc = opendir(DEV_DISCS_ROOT))) {
		while ((dp = readdir(usb_dev_disc))) {
			snprintf(bfr, sizeof(bfr), "%s/%s", DEV_DISCS_ROOT, dp->d_name);
			if (strncmp(dp->d_name, "disc", 4) != 0)
				continue;

			disc_num = atoi(dp->d_name + 4);
			len = readlink(bfr, link, sizeof(link) - 1);
			if (len < 0)
				continue;

			link[len] = 0;
			cp = strstr(link, "/scsi/host");
			if (!cp)
				continue;

			host_no = atoi(cp + 10);
			if (host >= 0 && host_no != host)
				continue;

			/* We have found a disc that is on this controller.
			 * Loop thru all the partitions on this disc.
			 * The new way, reading thru /proc/partitions.
			 */
			mp = link;
			if ((cp = strstr(link, "../")) != NULL)
				mp = cp + 3;
			siz = strlen(mp);

			flags |= EFH_1ST_DISC;
			if (func && (prt_fp = fopen("/proc/partitions", "r"))) {
				while (fgets(line, sizeof(line) - 2, prt_fp)) {
					if (sscanf(line, " %*s %*s %*s %s", bfr2) == 1 &&
					    strncmp(bfr2, mp, siz) == 0)
					{
						if ((cp = strstr(bfr2, "/part"))) {
							part_num = atoi(cp + 5);
							snprintf(line, sizeof(line), "%s/part%d", bfr, part_num);
							snprintf(dsname, sizeof(dsname), "disc%d", disc_num);
							snprintf(ptname, sizeof(ptname), "disc%d_%d", disc_num, part_num);
						}
						else if ((cp = strstr(bfr2, "/disc"))) {
							*(++cp) = 0;
							if (!is_no_partition(bfr2))
								continue;
							snprintf(line, sizeof(line), "%s/disc", bfr);
							snprintf(dsname, sizeof(dsname), "disc%d", disc_num);
							strlcpy(ptname, dsname, sizeof(ptname));
						}
						else {
							continue;
						}
						result = (*func)(line, host_no, dsname, ptname, flags) || result;
						flags &= ~(EFH_1ST_HOST | EFH_1ST_DISC);
					}
				}
				fclose(prt_fp);
			}
		}
		closedir(usb_dev_disc);
	}

#endif	/* LINUX26 */

	return result;
}

/* Concept taken from the e2fsprogs/ismounted.c.
 * Find wherever 'file' (actually: device) is mounted.
 * Either the exact same device-name, or another device-name.
 * The latter is detected by comparing the rdev or dev&inode.
 * So aliasing won't fool us---we'll still find if it's mounted.
 * Return its mnt entry.
 * In particular, the caller would look at the mnt->mountpoint.
 *
 * Find the matching devname(s) in mounts or swaps.
 * If func is supplied, call it for each match.  If not, return mnt on the first match.
 */

static inline int is_same_device(char *fsname, dev_t file_rdev, dev_t file_dev, ino_t file_ino)
{
	struct stat st_buf;

	if (stat(fsname, &st_buf) == 0) {
		if (S_ISBLK(st_buf.st_mode)) {
			if (file_rdev && (file_rdev == st_buf.st_rdev))
				return 1;
		}
		else {
			if (file_dev && ((file_dev == st_buf.st_dev) &&
				(file_ino == st_buf.st_ino)))
				return 1;
			/* Check for [swap]file being on the device. */
			if (file_dev == 0 && file_ino == 0 && file_rdev == st_buf.st_dev)
				return 1;
		}
	}
	return 0;
}


struct mntent *findmntents(char *file, int swp, int (*func)(struct mntent *mnt, uint flags), uint flags)
{
	struct mntent	*mnt;
	struct stat	st_buf;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;

	if ((f = setmntent(swp ? "/proc/swaps": "/proc/mounts", "r")) == NULL)
		return NULL;

	if (stat(file, &st_buf) == 0) {
		if (S_ISBLK(st_buf.st_mode)) {
			file_rdev = st_buf.st_rdev;
		}
		else {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}
	while ((mnt = getmntent(f)) != NULL) {
		/* Always ignore rootfs mount */
		if (strcmp(mnt->mnt_fsname, "rootfs") == 0)
			continue;

		if (strcmp(file, mnt->mnt_fsname) == 0 ||
		    strcmp(file, mnt->mnt_dir) == 0 ||
		    is_same_device(mnt->mnt_fsname, file_rdev , file_dev, file_ino)) {
			if (func == NULL)
				break;
			(*func)(mnt, flags);
		}
	}

	endmntent(f);
	return mnt;
}


//#define SAME_AS_KERNEL
/* Simulate a hotplug event, as if a USB storage device
 * got plugged or unplugged.
 * Either use a hardcoded program name, or the same
 * hotplug program that the kernel uses for a real event.
 */
void add_remove_usbhost(char *host, int add)
{
	setenv("ACTION", add ? "add" : "remove", 1);
	setenv("SCSI_HOST", host, 1);
	setenv("PRODUCT", host, 1);
	setenv("INTERFACE", "TOMATO/0", 1);
#ifdef SAME_AS_KERNEL
	char pgm[256] = "/sbin/hotplug usb";
	char *p;
	int fd = open("/proc/sys/kernel/hotplug", O_RDONLY);
	if (fd) {
		if (read(fd, pgm, sizeof(pgm) - 5) >= 0) {
			if ((p = strchr(pgm, '\n')) != NULL)
				*p = 0;
			strlcat(pgm, " usb", sizeof(pgm));
		}
		close(fd);
	}
	system(pgm);
#else
	// don't use value from /proc/sys/kernel/hotplug 
	// since it may be overriden by a user.
	system("/sbin/hotplug usb");
#endif
	unsetenv("INTERFACE");
	unsetenv("PRODUCT");
	unsetenv("SCSI_HOST");
	unsetenv("ACTION");
}

char *get_usb_storage_path_num(int num, char *path)
{
	char var_name[64];
	char *p;

	/* check type */
	snprintf(var_name, sizeof(var_name), "usb_path%d", num);
	if(!nvram_match(var_name, "storage"))
		return NULL;

	/* check removed or not */
	snprintf(var_name, sizeof(var_name), "usb_path%d_removed", num);
	if(nvram_match(var_name, "1"))
		return NULL;

	snprintf(var_name, sizeof(var_name), "usb_path%d_fs_path0", num);
	if((p = nvram_get(var_name)) == NULL || *p == '\0')
		return NULL;
	snprintf(path, 128, "%s/%s", POOL_MOUNT_ROOT, p);
	if(check_if_dir_exist(path))
		return path;

	snprintf(var_name, sizeof(var_name), "usb_path_%s_label", p);
	if((p = nvram_get(var_name)) == NULL || *p == '\0')
		return NULL;

	snprintf(path, 128, "%s/%s", POOL_MOUNT_ROOT, p);
	if(check_if_dir_exist(path))
		return path;

	return NULL;
}

char *get_usb_storage_path(char *path)
{
	int i;
	char *ret_path = NULL;
	for(i = 1; i <= 3; ++i){
		if((ret_path = get_usb_storage_path_num(i, path)) != NULL)
			break;
	}
	return ret_path;
}



#ifdef RTCONFIG_NEW_PHYMAP
static uint64_t get_usb_mib_by_ifname(char *ifname, char *type)
{
	char tmp[100], buf[32];
	int result = 0;

	if (!ifname || !type)
		return result;

	snprintf(tmp, sizeof(tmp), "/sys/class/net/%s/statistics/%s", ifname, type);

	f_read_string(tmp, buf, sizeof(buf));
	return strtoull(buf, NULL, 10);
}

void get_usb_modem_status(phy_info_list *list)
{
	int i;
	char cap_buf[64] = {0};
	phy_port_mapping port_mapping;
	if (!list)
		return;
	get_phy_port_mapping(&port_mapping);
	// Clean all state and dupex.
	for (i = 0; i < port_mapping.count; i++) {
		if ((port_mapping.port[i].cap & PHY_PORT_CAP_USB) > 0 ||
			(port_mapping.port[i].cap & PHY_PORT_CAP_MOBILE) > 0) {
			if (list->phy_info[i].cap == 0) {
				snprintf(list->phy_info[i].label_name, sizeof(list->phy_info[i].label_name), "%s", 
					port_mapping.port[i].label_name);
				list->phy_info[i].cap = port_mapping.port[i].cap;
				snprintf(list->phy_info[i].cap_name, sizeof(list->phy_info[i].cap_name), "%s", 
					get_phy_port_cap_name(port_mapping.port[i].cap, cap_buf, sizeof(cap_buf)));
				list->count++;
			}
			snprintf(list->phy_info[i].duplex, sizeof(list->phy_info[i].duplex), "none");
			snprintf(list->phy_info[i].state, sizeof(list->phy_info[i].state), "down");
		}
	}

	if (!is_router_mode())
		return;

#if defined(RTCONFIG_DUALWAN)
	char *wans_dualwan = nvram_safe_get("wans_dualwan");
	if (strlen(wans_dualwan)) {
		int unit;
		for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
			int wan = get_dualwan_by_unit(unit);
			if (wan == WANS_DUALWAN_IF_USB) {
				//if (is_wan_connect(unit))
					int i, usb_idx = 1;
					char modem_path[16];
					char usb_path_node[16];
					char prefix[32], tmp[32];
					for (i = 0; i < port_mapping.count; i++) {
						if ((port_mapping.port[i].cap & PHY_PORT_CAP_USB) > 0 ||
							(port_mapping.port[i].cap & PHY_PORT_CAP_MOBILE) > 0) {
							snprintf(prefix, sizeof(prefix), "usb_path%d", usb_idx);
							snprintf(list->phy_info[i].duplex, sizeof(list->phy_info[i].duplex), "none");
							snprintf(list->phy_info[i].state, sizeof(list->phy_info[i].state), "down");
							list->phy_info[i].link_rate = 0;
							snprintf(modem_path, sizeof(modem_path), "%s", nvram_safe_get("usb_modem_act_path"));
							if (nvram_get(prefix)) {
								//_dprintf("%s=%s\n", prefix, nvram_safe_get(prefix));
								snprintf(usb_path_node, sizeof(usb_path_node), "%s", nvram_safe_get(strlcat_r(prefix, "_node", tmp, sizeof(tmp))));
								if (nvram_match(prefix, "modem") && !strcmp(usb_path_node, modem_path)) {
									if (is_wan_connect(unit))
										snprintf(list->phy_info[i].state, sizeof(list->phy_info[i].state), "up");
									else
										snprintf(list->phy_info[i].state, sizeof(list->phy_info[i].state), "down");

									list->phy_info[i].link_rate = nvram_get_int(strlcat_r(prefix, "_speed", tmp, sizeof(tmp)));
									if (list->status_and_speed_only == 0) {
										char usb_ifname[8];
										snprintf(usb_ifname, sizeof(usb_ifname), get_wan_ifname(unit));
										//_dprintf("usb_ifname=%s\n", usb_ifname);
										list->phy_info[i].tx_bytes = get_usb_mib_by_ifname(usb_ifname, "tx_bytes");
										list->phy_info[i].rx_bytes = get_usb_mib_by_ifname(usb_ifname, "rx_bytes");
										list->phy_info[i].tx_packets = get_usb_mib_by_ifname(usb_ifname, "tx_packets");
										list->phy_info[i].rx_packets = get_usb_mib_by_ifname(usb_ifname, "rx_packets");
										list->phy_info[i].crc_errors = get_usb_mib_by_ifname(usb_ifname, "rx_crc_errors");
									}
								} else {
									snprintf(list->phy_info[i].state, sizeof(list->phy_info[i].state), "down");
									list->phy_info[i].link_rate = 0;
									if (!nvram_match(prefix, "modem")) {
										list->phy_info[i].cap &= ~PHY_PORT_CAP_DUALWAN_SECONDARY_WAN;
										list->phy_info[i].cap &= ~PHY_PORT_CAP_DUALWAN_PRIMARY_WAN;
									}
								}
							} else {
								int j;
								int is_modem_connected = 0;
								for (j = 1; j <= MAX_USB_HUB_PORT; j++) {
									snprintf(prefix, sizeof(prefix), "usb_path%d.%d", usb_idx, j);
									snprintf(usb_path_node, sizeof(usb_path_node), "%s", nvram_safe_get(strlcat_r(prefix, "_node", tmp, sizeof(tmp))));
									//_dprintf("prefix=%s, usb_path_node=%s\n", prefix, usb_path_node);
									if (nvram_match(prefix, "modem") && !strcmp(usb_path_node, modem_path)) {
										if (is_wan_connect(unit))
											snprintf(list->phy_info[i].state, sizeof(list->phy_info[i].state), "up");
										else
											snprintf(list->phy_info[i].state, sizeof(list->phy_info[i].state), "down");
										list->phy_info[i].link_rate = nvram_get_int(strlcat_r(prefix, "_speed", tmp, sizeof(tmp)));
										if (list->status_and_speed_only == 0) {
											char usb_ifname[8];
											snprintf(usb_ifname, sizeof(usb_ifname), get_wan_ifname(unit));
											//_dprintf("usb_ifname=%s\n", usb_ifname);
											list->phy_info[i].tx_bytes = get_usb_mib_by_ifname(usb_ifname, "tx_bytes");
											list->phy_info[i].rx_bytes = get_usb_mib_by_ifname(usb_ifname, "rx_bytes");
											list->phy_info[i].tx_packets = get_usb_mib_by_ifname(usb_ifname, "tx_packets");
											list->phy_info[i].rx_packets = get_usb_mib_by_ifname(usb_ifname, "rx_packets");
											list->phy_info[i].crc_errors = get_usb_mib_by_ifname(usb_ifname, "rx_crc_errors");
										}
										is_modem_connected = 1;
										break;
									} else {
										snprintf(list->phy_info[i].state, sizeof(list->phy_info[i].state), "down");
										list->phy_info[i].link_rate = 0;
										if (nvram_match(prefix, "modem")) {
											is_modem_connected = 1;
										}
									}
								}
								if (!is_modem_connected) {
									list->phy_info[i].cap &= ~PHY_PORT_CAP_DUALWAN_SECONDARY_WAN;
									list->phy_info[i].cap &= ~PHY_PORT_CAP_DUALWAN_PRIMARY_WAN;
								}
							}
							usb_idx++;
						}
					}
				//}
			}
		}
	}
#endif
}
#endif


