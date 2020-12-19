/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/

#include "rc.h"

#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/reboot.h>
#include <errno.h>
#if defined(RTCONFIG_PSISTLOG) || defined(RTCONFIG_JFFS2LOG)
#include <limits.h>
#endif
#ifndef MNT_DETACH
#define MNT_DETACH	0x00000002
#endif

//	#define TEST_INTEGRITY

#ifdef RTCONFIG_JFFSV1
#define JFFS_NAME	"jffs"
#else
#define JFFS_NAME	"jffs2"
#endif

#ifdef RTCONFIG_BRCM_NAND_JFFS2
#ifdef HND_ROUTER
#define JFFS2_PARTITION	"misc2"
#else
#define JFFS2_PARTITION	"brcmnand"
#endif
#else
#define JFFS2_PARTITION	"jffs2"
#endif

#ifdef RTCONFIG_BRCM_NAND_JFFS2
#define JFFS2_MTD_NAME	JFFS2_PARTITION
#else
#define JFFS2_MTD_NAME	JFFS_NAME
#endif

#define SECOND_JFFS2_PATH	"/asus_jffs"

int jffs2_fail = 0;

static void error(const char *message)
{
	char s[512];

	snprintf(s, sizeof(s), "Error %s JFFS. Check the logs to see if they contain more details about this error.", message);
	notice_set("jffs", s);
}

unsigned int get_root_type(void)
{
	int model;

	model = get_model();

	switch(model) {
		case MODEL_RTAC56S:
		case MODEL_RTAC56U:
		case MODEL_RTAC3200:
		case MODEL_DSLAC68U:
		case MODEL_RTAC68U:
		case MODEL_RTAC88U:
		case MODEL_RTAC3100:
		case MODEL_RTAC5300:
		case MODEL_RTAC87U:
		case MODEL_RTN18U:
		case MODEL_RTN65U:
		case MODEL_RTN14U: // it should be better to use LINUX_KERNEL_VERSION >= KERNEL_VERSION(2,6,36)
		case MODEL_RTAC51U:
		case MODEL_RTAC1200G:
		case MODEL_RTAC1200GP:
		case MODEL_RTAC51UP:
		case MODEL_RTAC53:
		case MODEL_RTAC1200GA1:
		case MODEL_RTAC1200GU:
		case MODEL_RTAC1200:
		case MODEL_RTAC1200V2:
		case MODEL_RTN11P_B1:
		case MODEL_RPAC53:
		case MODEL_RPAC55:
		case MODEL_RPAC92:
		case MODEL_RTN19:
		case MODEL_RTAC59U:
		case MODEL_MAPAC1750:
		case MODEL_RTAC59CD6R:
		case MODEL_RTAC59CD6N:
			return 0x73717368;      /* squashfs */
		case MODEL_GTAC5300:
		case MODEL_RTAC86U:
		case MODEL_RTAX88U:
		case MODEL_GTAX11000:
		case MODEL_RTAX92U:
		case MODEL_RTAX95Q:
		case MODEL_RTAX56_XD4:
		case MODEL_CTAX56_XD4:
		case MODEL_RTAX58U:
		case MODEL_RTAX55:
		case MODEL_RTAX56U:
		case MODEL_RPAX56:
		case MODEL_GTAXE11000:
		
			return 0x24051905;      /* ubifs */
		case MODEL_DSLAX82U:
		{
			struct statfs sf;
			statfs("/", &sf);
			return sf.f_type;
		}
	}
#ifdef HND_ROUTER
	return 0x24051905;      /* ubifs */
#else
	return 0x71736873;      /* squashfs */
#endif
}

int check_in_rootfs(const char *mount_point, const char *msg_title, int format)
{
	struct statfs sf;
	if (statfs(mount_point, &sf) == 0) {
		if (sf.f_type != get_root_type()) {
			// already mounted
			notice_set(msg_title, format ? "Formatted" : "Loaded");
			return 0;
		}
	}
	return 1;
}


#if defined(RTCONFIG_JFFS2ND_BACKUP)
#define SECOND_JFFS2_PARTITION  "asus"

void mount_2nd_jffs2(void)
{
	int format = 0;
	char s[256];
	int size;
	int part;
	int i = 0;

	_dprintf("Mount 2nd jffs2...\n");
	while (1) {
		if (wait_action_idle(10)) break;
		else i++;

		if (i>=10) {
			_dprintf("Mount 2nd jffs2 failed!");
			return;
		}
	}

	if (!mtd_getinfo(SECOND_JFFS2_PARTITION, &part, &size)) {
		_dprintf("Can not get 2nd jffs2 information!");
		return;
	}
	_dprintf("2nd jffs2: %d, %d\n", part, size);

	if (!check_in_rootfs(SECOND_JFFS2_PATH, "2nd_jffs", format))
		return;

	modprobe(JFFS_NAME);
	sprintf(s, MTD_BLKDEV(%d), part);

	i = 0;
	while (mount(s, SECOND_JFFS2_PATH, JFFS_NAME, MS_RDONLY, "") != 0) {
		_dprintf("Mount 2nd jffs failed! Try again...\n");
		if (i >= 10) {
			_dprintf("Mount 2nd jffs 10 times failed, stop mount!");
			break;
		}
		i++;
	}

	return;
}

void format_mount_2nd_jffs2(void)
{
	int format = 0;
	char s[256];
	int size;
	int part;
	const char *p;
	int model;

	if (!wait_action_idle(10)) return;

	if (!mtd_getinfo(SECOND_JFFS2_PARTITION, &part, &size)) return;
	_dprintf("Format 2nd jffs2: %d, %d\n", part, size);

	if (!check_in_rootfs(SECOND_JFFS2_PATH, "2nd_jffs", format))
		return;

	if (mtd_unlock(SECOND_JFFS2_PARTITION)) {
		error("unlocking");
		return;
	}

	modprobe(JFFS_NAME);
	sprintf(s, MTD_BLKDEV(%d), part);
	model = get_model();
	if (mount(s, SECOND_JFFS2_PATH, JFFS_NAME, MS_NOATIME, "") != 0) {
		if (mtd_erase(JFFS2_MTD_NAME)){
			error("formatting");
			return;
		}

		format = 1;
		if (mount(s, SECOND_JFFS2_PATH, JFFS_NAME, MS_NOATIME, "") != 0) {
			_dprintf("*** jffs2 2-nd mount error\n");
			//modprobe_r(JFFS_NAME);
			error("mounting");
			return;
		}
	}

	sprintf(s, "rm -rf %s/*", SECOND_JFFS2_PATH);

	system(s);
	userfs_prepare(SECOND_JFFS2_PATH);
	notice_set("2nd_jffs", format ? "Formatted" : "Loaded");

#if 0 /* disable legacy & asus autoexec */
	if (((p = nvram_get("jffs2_exec")) != NULL) && (*p != 0)) {
		chdir(SECOND_JFFS2_PATH);
		system(p);
		chdir("/");
	}
	run_userfile(SECOND_JFFS2_PATH, ".asusrouter", SECOND_JFFS2_PATH, 3);
#endif
}
#endif

void start_jffs2(void)
{
	if (!nvram_match("jffs2_enable", "1")) {
		notice_set("jffs", "");
		return;
	}

	int format = 0;
	char s[256];
	int size;
	int part;
	const char *p;
	int i = 0;

        while(1) {
		if (wait_action_idle(10)) break;
		else i++;

		if(i>=10) {
			_dprintf("Mount jffs2 failed!");
			return;
		}
	}

	if (!mtd_getinfo(JFFS2_PARTITION, &part, &size)) return;

	jffs2_fail = 0;
	_dprintf("start jffs2: %d, %d\n", part, size);

	if (nvram_match("jffs2_format", "1")) {
		nvram_set("jffs2_format", "0");
		nvram_commit_x();
		if (mtd_erase(JFFS2_MTD_NAME)){
			error("formatting");
			return;
		}
		format = 1;
	}

	sprintf(s, "%d", size);
	p = nvram_get("jffs2_size");
	if ((p == NULL) || (strcmp(p, s) != 0)) {
		if (format) {
			nvram_set("jffs2_size", s);
			nvram_commit_x();
		}
		else if ((p != NULL) && (*p != 0)) {
			error("verifying known size of");
			return;
		}
	}

	if (!check_in_rootfs("/jffs", "jffs", format))
		return;

	if (nvram_get_int("jffs2_clean_fs")) {
		if (mtd_unlock(JFFS2_PARTITION)) {
			error("unlocking");
			return;
		}
	}
	modprobe(JFFS_NAME);
	sprintf(s, MTD_BLKDEV(%d), part);

	if (mount(s, "/jffs", JFFS_NAME, MS_NOATIME, "") != 0) {
		if (mtd_erase(JFFS2_MTD_NAME)){
			jffs2_fail = 1;
			error("formatting");
			return;
		}

		format = 1;
		if (mount(s, "/jffs", JFFS_NAME, MS_NOATIME, "") != 0) {
			_dprintf("*** jffs2 2-nd mount error\n");
			//modprobe_r(JFFS_NAME);
			error("mounting");
			jffs2_fail = 1;
			return;
		}
	}

#if defined(RTCONFIG_ISP_CUSTOMIZE)
	load_customize_package();
#endif

	if(jffs2_fail == 1) {
		nvram_set("jffs2_fail", "1");
		nvram_commit();
	}

	if (nvram_match("force_erase_jffs2", "1")) {
		_dprintf("\n*** force erase jffs2 ***\n");
		mtd_erase(JFFS2_MTD_NAME);
		nvram_set("jffs2_clean_fs", "1");
		nvram_commit();
		reboot(RB_AUTOBOOT);
	}
#ifdef TEST_INTEGRITY
	int test;

	if (format) {
		if (f_write("/jffs/.tomato_do_not_erase", &size, sizeof(size), 0, 0) != sizeof(size)) {
			stop_jffs2(0);
			error("setting integrity test for");
			return;
		}
	}
	if ((f_read("/jffs/.tomato_do_not_erase", &test, sizeof(test)) != sizeof(test)) || (test != size)) {
		stop_jffs2(0);
		error("testing integrity of");
		return;
	}
#endif

	if (nvram_get_int("jffs2_clean_fs")) {
		if((0 == nvram_get_int("x_Setting")) && (check_if_file_exist("/jffs/remove_hidden_flag")))
		{
#ifdef RTCONFIG_ISP_CUSTOMIZE
			// Remove hidden folder but excluding /jffs/.package.
			system("find /jffs/ -name '.*' -a ! -name '.package' -a ! -name '.package.tar.gz' -a ! -name 'package.tar.gz' -exec rm -rf {} \\;");
#else
			system("rm -rf /jffs/.*");
#endif
			_dprintf("Clean /jffs/.*\n");
		}
		_dprintf("Clean /jffs/*\n");
		system("rm -fr /jffs/*");
		nvram_unset("jffs2_clean_fs");
		nvram_commit_x();
	}

	userfs_prepare("/jffs");
	notice_set("jffs", format ? "Formatted" : "Loaded");
	jffs2_fail = 0;


#if defined(HND_ROUTER) || defined(DSL_AC68U)
#ifdef RTCONFIG_JFFS_NVRAM
	system("rm -rf /jffs/nvram_war");
	jffs_nvram_init();
	system("touch /jffs/nvram_war");
#endif
#endif

#if 0 /* disable legacy & asus autoexec */
	if (((p = nvram_get("jffs2_exec")) != NULL) && (*p != 0)) {
		chdir("/jffs");
		system(p);
		chdir("/");
	}

	run_userfile("/jffs", ".asusrouter", "/jffs", 3);
#endif

#ifdef CONFIG_BCMWL5
	check_asus_jffs();
#endif

	if (!check_if_dir_exist("/jffs/scripts/")) mkdir("/jffs/scripts/", 0755);
	if (!check_if_dir_exist("/jffs/configs/")) mkdir("/jffs/configs/", 0755);
	if (!check_if_dir_exist("/jffs/addons/")) mkdir("/jffs/addons/", 0755);
	if (!check_if_dir_exist(UPLOAD_CERT_FOLDER)) mkdir(UPLOAD_CERT_FOLDER, 0600);
}

void stop_jffs2(int stop)
{
	struct statfs sf;
#if defined(RTCONFIG_PSISTLOG) || defined(RTCONFIG_JFFS2LOG)
	int restart_syslogd = 0;
#endif

	if (!wait_action_idle(10)) return;

	if ((statfs("/jffs", &sf) == 0) && (sf.f_type != 0x73717368) && (sf.f_type != 0x71736873)) {
		// is mounted
#if 0 /* disable legacy & asus autoexec */
		run_userfile("/jffs", ".autostop", "/jffs", 5);
		run_nvscript("script_autostop", "/jffs", 5);
#endif
	}

#if defined(RTCONFIG_PSISTLOG) || defined(RTCONFIG_JFFS2LOG)
	char prefix[PATH_MAX], path1[PATH_MAX], path2[PATH_MAX];

	snprintf(prefix, sizeof(prefix), "%s/", nvram_safe_get("log_path"));
	snprintf(path1, sizeof(path1), "%ssyslog.log", prefix);
	snprintf(path2, sizeof(path2), "%ssyslog.log-1", prefix);

	if (!stop && !strncmp(get_syslog_fname(0), prefix, strlen(prefix))) {
		restart_syslogd = 1;
		stop_syslogd();
		eval("cp", path1, path2, "/tmp");
	}
#endif

	notice_set("jffs", "Stopped");
	if (umount("/jffs"))
		umount2("/jffs", MNT_DETACH);
	else
		modprobe_r(JFFS_NAME);

#if defined(RTCONFIG_PSISTLOG) || defined(RTCONFIG_JFFS2LOG)
	if (restart_syslogd)
		start_syslogd();
#endif
}
