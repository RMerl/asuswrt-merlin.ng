/*
	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/

#include "rc.h"

#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <errno.h>
#ifndef MNT_DETACH
#define MNT_DETACH	0x00000002
#endif

#define YAFFS_VOL_NAME	"jffs2"
#define YAFFS_MNT_DIR	"/jffs"
#define YAFFS_FS_TYPE	"yaffs2"

static void error(const char *message)
{
	char s[512];

	snprintf(s, sizeof(s),
		 "Error %s JFFS. Check the logs to see if they contain more details about this error.",
		 message);
	notice_set("yaffs", s);
}

void start_yaffs(void)
{
	int format = 0;
	char s[256];
	int dev, part, size;
	const char *p;
	struct statfs sf;

	if (!nvram_match("yaffs_on", "1")) {
		notice_set("yaffs", "");
		return;
	}

	if (!wait_action_idle(10))
		return;

	if (!mtd_getinfo(YAFFS_VOL_NAME, &part, &size))
		return;

	_dprintf("*** yaffs: %d, %d, %d\n", dev, part, size);
	if (nvram_match("yaffs_format", "1")) {
		nvram_set("yaffs_format", "0");

		if (mtd_erase(YAFFS_VOL_NAME)) {
			error("formatting");
			return;
		}

		format = 1;
	}

	sprintf(s, "%d", size);
	p = nvram_get("yaffs_size");
	if ((p == NULL) || (strcmp(p, s) != 0)) {
		if (format) {
			nvram_set("yaffs_size", s);
			nvram_commit_x();
		} else if ((p != NULL) && (*p != 0)) {
			error("verifying known size of");
			return;
		}
	}

	if ((statfs(YAFFS_MNT_DIR, &sf) == 0)
	    && (sf.f_type != 0x73717368 /* squashfs */ )) {
		// already mounted
		notice_set("yaffs", format ? "Formatted" : "Loaded");
		return;
	}
	if (nvram_get_int("yaffs_clean_fs")) {
		if (!mtd_unlock(YAFFS_VOL_NAME)) {
			error("unlocking");
			return;
		}
	}
	sprintf(s, MTD_BLKDEV(%d), part);

	if (mount(s, YAFFS_MNT_DIR, YAFFS_FS_TYPE, MS_NOATIME, "tags-ecc-off") != 0) {
		_dprintf("*** yaffs mount error\n");
		if (mtd_erase(YAFFS_VOL_NAME)) {
			error("formatting");
			return;
		}

		format = 1;
		if (mount(s, YAFFS_MNT_DIR, YAFFS_FS_TYPE, MS_NOATIME, "tags-ecc-off") != 0) {
			_dprintf("*** yaffs 2-nd mount error\n");
			error("mounting");
			return;
		}
	}

	if (nvram_get_int("yaffs_clean_fs")) {
		_dprintf("Clean /jffs/*\n");
		system("rm -fr /jffs/*");
		nvram_unset("yaffs_clean_fs");
		nvram_commit_x();
	}

	notice_set("yaffs", format ? "Formatted" : "Loaded");

	if (((p = nvram_get("yaffs_exec")) != NULL) && (*p != 0)) {
		chdir(YAFFS_MNT_DIR);
		system(p);
		chdir("/");
	}
	run_userfile(YAFFS_MNT_DIR, ".asusrouter", YAFFS_MNT_DIR, 3);

}

void stop_yaffs(int stop)
{
	struct statfs sf;
#if defined(RTCONFIG_PSISTLOG)
	int restart_syslogd = 0;
#endif

	if (!wait_action_idle(10))
		return;

	if ((statfs(YAFFS_MNT_DIR, &sf) == 0) && (sf.f_type != 0x73717368)) {
		// is mounted
		run_userfile(YAFFS_MNT_DIR, ".autostop", YAFFS_MNT_DIR, 5);
		run_nvscript("script_autostop", YAFFS_MNT_DIR, 5);
	}
#if defined(RTCONFIG_PSISTLOG)
	if (!stop && !strncmp(get_syslog_fname(0), YAFFS_MNT_DIR "/", sizeof(YAFFS_MNT_DIR) + 1)) {
		restart_syslogd = 1;
		stop_syslogd();
		eval("cp", YAFFS_MNT_DIR "/syslog.log", YAFFS_MNT_DIR "/syslog.log-1", "/tmp");
	}
#endif

	notice_set("yaffs", "Stopped");
	if (umount(YAFFS_MNT_DIR))
		umount2(YAFFS_MNT_DIR, MNT_DETACH);

#if defined(RTCONFIG_PSISTLOG)
	if (restart_syslogd)
		start_syslogd();
#endif
}
