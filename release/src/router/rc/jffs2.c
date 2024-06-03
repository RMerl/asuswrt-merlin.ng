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
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)
#define JFFS2_PARTITION "jffs"
#else
#define JFFS2_PARTITION	"misc2"
#endif
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
		case MODEL_RTACRH18:
		case MODEL_RT4GAC86U:
		case MODEL_RTAX53U:
		case MODEL_XD4S:
		case MODEL_RTAX54:
		case MODEL_RT4GAX56:
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
		case MODEL_BC109:
		case MODEL_BC105:
		case MODEL_EBG19:
		case MODEL_GTAX11000:
		case MODEL_RTAX92U:
		case MODEL_RTAX95Q:
		case MODEL_XT8PRO:
		case MODEL_BT12:
		case MODEL_BT10:
		case MODEL_BQ16:
		case MODEL_BQ16_PRO:
		case MODEL_BM68:
		case MODEL_XT8_V2:
		case MODEL_RTAXE95Q:
		case MODEL_ET8PRO:
		case MODEL_ET8_V2:
		case MODEL_RTAX56_XD4:
		case MODEL_XD4PRO:
		case MODEL_XC5:
		case MODEL_CTAX56_XD4:
		case MODEL_EBA63:
		case MODEL_RTAX58U:
		case MODEL_RTAX82_XD6S:
		case MODEL_RTAX58U_V2:
		case MODEL_RTAX3000N:
		case MODEL_BR63:
		case MODEL_RTAX55:
		case MODEL_RTAX56U:
		case MODEL_RPAX56:
		case MODEL_RPAX58:	// need to chk if still ubifs
		case MODEL_GTAXE11000:
		case MODEL_GTAX6000:
		case MODEL_GTAX11000_PRO:
		case MODEL_GTAXE16000:
		case MODEL_GTBE98:
		case MODEL_GTBE98_PRO:
		case MODEL_ET12:
		case MODEL_XT12:
		case MODEL_RTAX86U:
		case MODEL_RTAX68U:
		case MODEL_RTAX86U_PRO:
		case MODEL_RTAX88U_PRO:
		case MODEL_RTBE96U:
		case MODEL_GTBE96:
		case MODEL_RTBE88U:
		case MODEL_GTBE19000:
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

	if (!check_mountpoint((char *)mount_point)) return 1;

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

enum {
	JFFS2_NO = 0,
	JFFS2_BEGIN,
	JFFS2_MOUNT,
	JFFS2_END
};

#if !defined(RTCONFIG_UBIFS) && !defined(RTCONFIG_YAFFS)
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_JFFSV1) || defined(RTCONFIG_BRCM_NAND_JFFS2)
void start_jffs2(void)
{
#if 0
	if (!nvram_match("jffs2_on", "1")) {
		notice_set("jffs", "");
		return;
	}
#endif

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

#ifdef RTCONFIG_BCMARM
	int jffs2_state = nvram_get_int("jffs2_state");
#define JFFS2_AUTO_ERASE_MAX 1
	int jffs2_auto_erase_max = nvram_get_int("jffs2_auto_erase_max");
	int jffs2_auto_erase = nvram_get_int("jffs2_auto_erase");
	int jffs2_ever_erase = nvram_get_int("jffs2_ever_erase");

	if(jffs2_auto_erase_max == 0)
		jffs2_auto_erase_max = JFFS2_AUTO_ERASE_MAX;

	if(jffs2_auto_erase < jffs2_auto_erase_max && (jffs2_state != JFFS2_NO && jffs2_state != JFFS2_END)){ // ever fail to start jffs2
		_dprintf("%s: erase the jffs2 partition because it ever failed to be started\n", __func__);
		logmessage("jffs2", "erase the jffs2 partition because it ever failed to be started");
		nvram_set_int("jffs2_state", JFFS2_NO);
		++jffs2_auto_erase;
		nvram_set_int("jffs2_auto_erase", jffs2_auto_erase);
		++jffs2_ever_erase;
		nvram_set_int("jffs2_ever_erase", jffs2_ever_erase);
		nvram_commit();
#ifdef RTCONFIG_HND_ROUTER
		mtd_erase_misc2();
#else
		system("mtd-erase2 brcmnand");
#endif
		_dprintf("%s: rebooting because DUT had ever erased jffs2 %d times (ever %d times)\n", __func__, jffs2_auto_erase, jffs2_ever_erase);
		logmessage("jffs2", "rebooting because DUT had ever erased jffs2 %d times (ever %d times)", jffs2_auto_erase, jffs2_ever_erase);
		reboot(RB_AUTOBOOT);
		return;
	}
	else
#endif
	{
		nvram_set_int("jffs2_state", JFFS2_BEGIN);
		nvram_commit();
	}

	_dprintf("%s: getting the information of jffs2\n", __func__);
	logmessage("jffs2", "getting the information of jffs2");
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

	_dprintf("%s: mounting jffs2\n", __func__);
	logmessage("jffs2", "mounting jffs2");
	nvram_set_int("jffs2_state", JFFS2_MOUNT);
	nvram_commit();
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
#if defined(RTCONFIG_ISP_CUSTOMIZE_TOOL) || defined(RTCONFIG_ISP_CUSTOMIZE)
			// Remove hidden folder but excluding /jffs/.ac and /jffs/.package.
			system("find /jffs/ -name '.*' -a ! -name '.ict' -a ! -name '.package' -a ! -name '.package.tar.gz' -a ! -name 'package.tar.gz' -exec rm -rf {} \\;");
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

#ifdef RTCONFIG_JFFS_NVRAM
	system("rm -rf /jffs/nvram_war");
	jffs_nvram_init();
	system("touch /jffs/nvram_war");
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

	_dprintf("%s: create jffs2 successfully\n", __func__);
	logmessage("jffs2", "create jffs2 successfully");
	nvram_set_int("jffs2_state", JFFS2_END);
#ifdef RTCONFIG_BCMARM
	nvram_set_int("jffs2_auto_erase", 0);
#endif
	nvram_commit();
}

void stop_jffs2(int stop)
{
	struct statfs sf;
#if defined(RTCONFIG_PSISTLOG) || defined(RTCONFIG_JFFS2LOG)
	int restart_syslogd = 0;
#endif

	if (!wait_action_idle(10)) return;

	if ((statfs("/jffs", &sf) == 0) && (sf.f_type != 0x73717368)) {
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

#endif // defined(RTCONFIG_JFFS2) || defined(RTCONFIG_JFFSV1) || defined(RTCONFIG_BRCM_NAND_JFFS2)
#endif // !defined(RTCONFIG_UBIFS) && !defined(RTCONFIG_YAFFS)
