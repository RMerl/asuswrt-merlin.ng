/*
	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/

#include "rc.h"

#if LINUX_KERNEL_VERSION >= KERNEL_VERSION(3,10,14)
#define __packed __attribute__((__packed__))
#endif
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <mtd/ubi-user.h>
#include <errno.h>
#ifndef MNT_DETACH
#define MNT_DETACH	0x00000002
#endif
#include <limits.h>		//PATH_MAX, LONG_MIN, LONG_MAX

#define UBI_SYSFS_DIR	"/sys/class/ubi"

#define UBIFS_VOL_NAME	"jffs2"
#define UBIFS_MNT_DIR	"/jffs"
#define UBIFS_FS_TYPE	"ubifs"

#ifdef RTCONFIG_MTK_NAND
#define JFFS2_MTD_NAME	"jffs2"
#define UBI_DEV_NUM	"0"
#define UBI_DEV_PATH	"/dev/ubi"UBI_DEV_NUM
#define LEBS		0x1F000		/* 124 KiB */
#define NUM_OH_LEB	24		/* for ubifs overhead */
#endif

#ifdef RTK3
#define JFFS2_MTD_NAME	"brcmnand"
#define UBI_DEV_NUM	0
#define UBI_DEV_PATH	"/dev/ubi0"
#define LEBS		0x1F000		/* 124 KiB */
#define NUM_OH_LEB	24		/* for ubifs overhead */
#endif

static void error(const char *message)
{
	char s[512];

	snprintf(s, sizeof(s),
		 "Error %s JFFS. Check the logs to see if they contain more details about this error.",
		 message);
	notice_set("ubifs", s);
}

/* erase and format specified UBI volume that is used to provide container of UBIFS
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 *     -2:	get total number of erase blocks fail
 *     -3:	invalid reserved ebs of the ubi volume
 *     -4:	open /dev/ubiX_Y failed.
 *     -5:	erase LEB fail
 *     -6:	got upd_marker flag fail
 */
static int ubifs_unlock(int dev, int part)
{
	int i, r, fd, nr_ebs = 0, upd_marker = 0;
	int32_t lnum;
	char *s, path[PATH_MAX];

	if (dev < 0 || part < 0)
		return -1;

	snprintf(path, sizeof(path), "%s/ubi%d_%d/upd_marker", UBI_SYSFS_DIR, dev, part);
	if (!(s = file2str(path)))
		return -6;
	if (atoi(s) == 1)
		upd_marker = 1;
	free(s);

	snprintf(path, sizeof(path), "%s/ubi%d_%d/reserved_ebs", UBI_SYSFS_DIR, dev, part);
	if (!(s = file2str(path)))
		return -2;
	nr_ebs = atoi(s);
	free(s);
	if (nr_ebs <= 0)
		return -3;

	snprintf(path, sizeof(path), "/dev/ubi%d_%d", dev, part);
	if (!(fd = open(path, O_RDWR)))
		return -4;

	if (upd_marker) {
		int64_t bytes = 0;

		if ((r = ioctl(fd, UBI_IOCVOLUP, &bytes)) != 0)
			_dprintf("%s: clean upd_marker fail!. (ret %d errno %d %s)\n",
				__func__, r, errno, strerror(errno));
	}

#if !defined(RTCONFIG_ISP_CUSTOMIZE) && !defined(RTCONFIG_ISP_CUSTOMIZE_TOOL)
	for (i = 0; i < nr_ebs; ++i) {
		lnum = i;
		if (!(r = ioctl(fd, UBI_IOCEBER, &lnum)))
			continue;
		_dprintf("%s: erase leb %d of ubi%d_%d fail. (ret %d errno %d %s)\n",
			__func__, lnum, dev, part, r, errno, strerror(errno));
	}
#endif

	close(fd);

	snprintf(path, sizeof(path), "/dev/ubi%d_%d", dev, part);
	eval("mkfs.ubifs", "-x", "favor_lzo", path);
	return 0;
}

/* format specified UBI volume that is used to provide container of UBIFS
 * @return:
 */
static inline int ubifs_erase(int dev, int part)
{
	return ubifs_unlock(dev, part);
}

void start_ubifs(void)
{
	int format = 0;
	char s[256];
	int dev, part, size;
	const char *p;
	struct statfs sf;
#if defined(RTCONFIG_TEST_BOARDDATA_FILE)
	int r;
#endif
#if defined(RTCONFIG_MTK_NAND) || defined(RTK3)
	int mtd_part = 0, mtd_size = 0;
	char dev_mtd[] = "/dev/mtdXXX";
#endif

	if (!nvram_match("ubifs_on", "1")) {
		notice_set("ubifs", "");
		return;
	}

#if defined(RTCONFIG_LANTIQ)
	if (!wait_action_idle(1))
		return;
#else
	if (!wait_action_idle(10))
		return;
#endif

#if defined(RTCONFIG_MTK_NAND) || defined(RTK3)
	if (!mtd_getinfo(JFFS2_MTD_NAME, &mtd_part, &mtd_size)) return;

	_dprintf("*** ubifs: %s (%d, %d)\n", UBIFS_VOL_NAME, mtd_part, mtd_size);

	if (!nvram_get_int("ubifs_clean_fs")) {
		/* attach ubi */
		snprintf(dev_mtd, sizeof(dev_mtd), "/dev/mtd%d", mtd_part);
		_dprintf("*** ubifs: attach (%s, %d)\n", dev_mtd, UBI_DEV_NUM);
		//eval("ubiattach", "-p", dev_mtd, "-d", UBI_DEV_NUM);
		doSystem("ubiattach -p %s -d %d", dev_mtd, UBI_DEV_NUM);
	}

	if (ubi_getinfo(UBIFS_VOL_NAME, &dev, &part, &size) == 1) {	//ubi volume not found, format it and create volume
		unsigned int num_leb = 0, num_avail_leb = 0, vol_size = 0;

		_dprintf("*** ubifs: ubi volume not found\n");

#ifdef RTK3
		/* format ubi */
		snprintf(dev_mtd, sizeof(dev_mtd), "/dev/mtd%d", mtd_part);
		_dprintf("*** ubifs: format (%s)\n", dev_mtd);
		doSystem("ubiformat -y %s", dev_mtd);
#else
		/* mtd erase on UBIFS_VOL_NAME first */
		if (mtd_erase(JFFS2_MTD_NAME)) {
			error("formatting");
			return;
		}
#endif

		/* compute jffs2's volume size */
		num_leb = mtd_size >> 17;			/* compute number of leb divde by 128KiB */
		num_avail_leb = num_leb - NUM_OH_LEB;
		vol_size = (num_avail_leb * LEBS) >> 10;	/* convert to KiB unit */
		if (vol_size > 0) {
			char vol_size_s[32] = {0};

			snprintf(vol_size_s, sizeof(vol_size_s), "%dKiB", vol_size);
			_dprintf("*** ubifs: mtd_part(%02x), num_leb(%d), num_avail_leb(%d), vol_size(%s)\n", mtd_part, num_leb, num_avail_leb, vol_size_s);

			/* attach ubi */
			snprintf(dev_mtd, sizeof(dev_mtd), "/dev/mtd%d", mtd_part);
			_dprintf("*** ubifs: attach (%s, %d)\n", dev_mtd, UBI_DEV_NUM);
			//eval("ubiattach", "-p", dev_mtd, "-d", UBI_DEV_NUM);
			doSystem("ubiattach -p %s -d %d", dev_mtd, UBI_DEV_NUM);

			/* make ubi volume */
			_dprintf("*** ubifs: create jffs2 volume\n");
			//eval("ubimkvol", UBI_DEV_PATH, "-s", vol_size_s, "-N", UBIFS_VOL_NAME);
			doSystem("ubimkvol %s -s %s -N %s", UBI_DEV_PATH, vol_size_s, UBIFS_VOL_NAME);
		}
	}
#endif

	if (ubi_getinfo(UBIFS_VOL_NAME, &dev, &part, &size) < 0)
		return;

	_dprintf("*** ubifs: %s %d, %d, %d\n", UBIFS_VOL_NAME, dev, part, size);
	if (nvram_match("ubifs_format", "1")) {
		nvram_set("ubifs_format", "0");

		if (ubifs_erase(dev, part)) {
			error("formatting");
			return;
		}

		format = 1;
	}

	sprintf(s, "%d", size);
	p = nvram_get("ubifs_size");

	if ((p == NULL) || (strcmp(p, s) != 0)) {
		if (format) {
			nvram_set("ubifs_size", s);
			nvram_commit_x();
		} else if ((p != NULL) && (*p != 0)) {
			error("verifying known size of");
			return;
		}
	}

	if ((statfs(UBIFS_MNT_DIR, &sf) == 0)
	    && (sf.f_type != 0x73717368 /* squashfs */ )) {
		// already mounted
		notice_set("ubifs", format ? "Formatted" : "Loaded");
#if defined(RTCONFIG_HND_ROUTER_AX_6756)
		goto skip_mnt;
#endif
		return;
	}
	if (nvram_get_int("ubifs_clean_fs")) {
		if (ubifs_unlock(dev, part)) {
			error("unlocking");
			return;
		}
#if defined(RTCONFIG_MTK_NAND) || defined(RTK3)
		nvram_unset("ubifs_clean_fs");
		nvram_commit_x();
#endif
	}
	sprintf(s, "/dev/ubi%d_%d", dev, part);

	if (mount(s, UBIFS_MNT_DIR, UBIFS_FS_TYPE, MS_NOATIME, "") != 0) {
		_dprintf("*** ubifs mount error\n");
		if (ubifs_erase(dev, part)) {
			error("formatting");
			return;
		}

		format = 1;
		if (mount(s, UBIFS_MNT_DIR, UBIFS_FS_TYPE, MS_NOATIME, "") != 0) {
			_dprintf("*** ubifs 2-nd mount error\n");
			error("mounting");
			return;
		}
	}


#if defined(RTCONFIG_HND_ROUTER_AX_6756)
skip_mnt:
#endif

#if defined(RTCONFIG_ISP_CUSTOMIZE)
	load_customize_package();
#endif

	if (nvram_get_int("ubifs_clean_fs")) {
		// This refer to jffs2.c. 
		// Because ubifs_unlock (erase) doesn't be called if ISP_CUSTOMIZE=y.
		// We use rm command (remove file includes hidden files.) instead.
		if((0 == nvram_get_int("x_Setting")) && (check_if_file_exist("/jffs/remove_hidden_flag")))
		{
#if defined(RTCONFIG_ISP_CUSTOMIZE_TOOL) || defined(RTCONFIG_ISP_CUSTOMIZE)
			// Remove hidden folder but excluding /jffs/.ac and /jffs/.package.
			system("find /jffs/ -name '.*' -a ! -name '.ict' -a ! -name '.package' -a ! -name '.package.tar.gz' -a ! -name 'package.tar.gz' -exec rm -rf {} \\;");
			_dprintf("Clean /jffs/.*\n");
#else
			system("rm -rf /jffs/.*");
#endif
		}
		_dprintf("Clean /jffs/*\n");
		system("rm -fr /jffs/*");
		nvram_unset("ubifs_clean_fs");
		nvram_commit_x();
	}

	userfs_prepare(UBIFS_MNT_DIR);

	notice_set("ubifs", format ? "Formatted" : "Loaded");

#if 0 /* disable legacy & asus autoexec */
	if (((p = nvram_get("ubifs_exec")) != NULL) && (*p != 0)) {
		chdir(UBIFS_MNT_DIR);
		system(p);
		chdir("/");
	}
	run_userfile(UBIFS_MNT_DIR, ".asusrouter", UBIFS_MNT_DIR, 3);
#endif

	if (!check_if_dir_exist("/jffs/scripts/")) mkdir("/jffs/scripts/", 0755);
	if (!check_if_dir_exist("/jffs/configs/")) mkdir("/jffs/configs/", 0755);
	if (!check_if_dir_exist("/jffs/addons/")) mkdir("/jffs/addons/", 0755);

#if defined(RTCONFIG_TEST_BOARDDATA_FILE)
	/* Copy /lib/firmware to /tmp/firmware, and
	 * bind mount /tmp/firmware to /lib/firmware.
	 */
	if (!d_exists(UBIFS_MNT_DIR "/firmware")) {
		_dprintf("Rebuild /lib/firmware on " UBIFS_MNT_DIR "/firmware!\n");
		eval("cp", "-a", "/lib/firmware", UBIFS_MNT_DIR "/firmware");
		sync();
	}
	if ((r = mount(UBIFS_MNT_DIR "/firmware", "/lib/firmware", NULL, MS_BIND, NULL)) != 0)
		_dprintf("%s: bind mount " UBIFS_MNT_DIR "/firmware fail! (r = %d)\n", __func__, r);
#endif

#ifdef RTCONFIG_JFFS_NVRAM
	system("rm -rf /jffs/nvram_war");
	jffs_nvram_init();
	system("touch /jffs/nvram_war");
#endif
}

void stop_ubifs(int stop)
{
	struct statfs sf;
#if defined(RTCONFIG_PSISTLOG)
	int restart_syslogd = 0;
#endif
#if defined(RTCONFIG_TEST_BOARDDATA_FILE)
	struct mntent *mnt;
#endif

	if (!wait_action_idle(10))
		return;

	if ((statfs(UBIFS_MNT_DIR, &sf) == 0) && (sf.f_type != 0x73717368)) {
		// is mounted
#if 0 /* disable legacy & asus autoexec */
		run_userfile(UBIFS_MNT_DIR, ".autostop", UBIFS_MNT_DIR, 5);
		run_nvscript("script_autostop", UBIFS_MNT_DIR, 5);
#endif
	}
#if defined(RTCONFIG_PSISTLOG)
	if (!stop && !strncmp(get_syslog_fname(0), UBIFS_MNT_DIR "/", sizeof(UBIFS_MNT_DIR) + 1)) {
		restart_syslogd = 1;
		stop_syslogd();
		eval("cp", UBIFS_MNT_DIR "/syslog.log", UBIFS_MNT_DIR "/syslog.log-1", "/tmp");
	}
#endif

#if defined(RTCONFIG_TEST_BOARDDATA_FILE)
	if ((mnt = findmntents("/lib/firmware", 0, NULL, 0)) != NULL) {
		_dprintf("Unmount /lib/firmware\n");
		sync();
		if (umount("/lib/firmware"))
			umount2("/lib/firmware", MNT_DETACH);
	}
#endif

	notice_set("ubifs", "Stopped");
	if (umount(UBIFS_MNT_DIR))
		umount2(UBIFS_MNT_DIR, MNT_DETACH);

#if defined(RTCONFIG_PSISTLOG)
	if (restart_syslogd)
		start_syslogd();
#endif
}
