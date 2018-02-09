/*
 * Miscellaneous services
 *
 * Copyright (C) 2009, Broadcom Corporation. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: services.c,v 1.100 2010/03/04 09:39:18 Exp $
 */

#include <rc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/vfs.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <sys/reboot.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/param.h>
#include <net/ethernet.h>
#ifdef RTCONFIG_TOR
#include <pwd.h>
#endif

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
#include <PMS_DBAPIs.h>
#endif

#ifndef RTCONFIG_DUALWAN
#include <rtstate.h>	/* for get_wanunit_by_type inline function */
#endif
#ifdef RTCONFIG_RALINK
#include <ralink.h>
#endif
#ifdef RTCONFIG_QCA
#include <qca.h>
#endif
#include <shared.h>
#ifdef RTCONFIG_DSL
#include <dsl-upg.h>
#endif
#ifdef RTCONFIG_USB
#include <disk_io_tools.h>	//mkdir_if_none()
#ifdef RTCONFIG_USB_SMS_MODEM
#include "libsmspdu.h"
#endif
#else
#ifdef RTCONFIG_MDNS
int mkdir_if_none(const char *path)
{
	DIR *dp;
	char cmd[PATH_MAX];

	if (!(dp = opendir(path))) {
		memset(cmd, 0, PATH_MAX);
		sprintf(cmd, "mkdir -m 0777 -p '%s'", (char *)path);
		system(cmd);
		return 1;
	}
	closedir(dp);

	return 0;
}
#endif
#endif	/* RTCONFIG_USB */

#ifdef RTCONFIG_QTN
#include "web-qtn.h"
#endif

#ifdef RTCONFIG_HTTPS
#include <errno.h>
#include <getopt.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#endif

#ifdef RTCONFIG_IPSEC
#include "rc_ipsec.h"
#define DBG(args) _dprintf args
#endif

#if defined(RTCONFIG_NOTIFICATION_CENTER)
#include <libnt.h>
#include <nt_eInfo.h>
#endif

#ifdef RTCONFIG_QCA_PLC_UTILS
#include <plc_utils.h>
#endif

#ifdef RTCONFIG_REALTEK
#include "../shared/sysdeps/realtek/realtek.h"
#endif

#if defined(RTCONFIG_BT_CONN)
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#endif /* RTCONFIG_BT_CONN */

#if defined(RTCONFIG_LP5523)
#include <lp5523led.h>
#endif

#ifdef RTCONFIG_CFGSYNC
#include <cfg_lib.h>
#include <cfg_event.h>
#endif

#ifdef RTCONFIG_AMAS
#include <amas-utils.h>
#endif

extern char *crypt __P((const char *, const char *)); //should be defined in unistd.h with _XOPEN_SOURCE defined
#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

/* The g_reboot global variable is used to skip several unnecessary delay
 * and redundant steps during reboot / restore to default procedure.
 */
int g_reboot = 0;
int g_upgrade = 0;

// Pop an alarm to recheck pids in 500 msec.
static const struct itimerval pop_tv = { {0,0}, {0, 500 * 1000} };

// Pop an alarm to reap zombies.
static const struct itimerval zombie_tv = { {0,0}, {307, 0} };

// -----------------------------------------------------------------------------

static const char dmhosts[] = "/etc/hosts.dnsmasq";
static const char dmresolv[] = "/tmp/resolv.conf";
static const char dmservers[] = "/tmp/resolv.dnsmasq";

#ifdef RTCONFIG_TOAD
static void start_toads(void);
static void stop_toads(void);
#endif

#ifdef RTCONFIG_CROND
void stop_cron(void);
void start_cron(void);
#endif
void start_wlcscan(void);
void stop_wlcscan(void);


#ifndef MS_MOVE
#define MS_MOVE		8192
#endif
#ifndef MNT_DETACH
#define MNT_DETACH	0x00000002
#endif

#ifdef BCMDBG
#include <assert.h>
#else
#define assert(a)
#endif

#define logs(s) syslog(LOG_NOTICE, s)

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_JFFSV1) || defined(RTCONFIG_BRCM_NAND_JFFS2)
int jffs2_fail;
#endif

void
sanity_logs()
{
//#if defined(RTAC56U) || defined(RTAC56S)
//	logmessage("ATE", "valid user mode(%d)", !nvram_get_int(ATE_BRCM_FACTORY_MODE_STR()));
//#endif
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_JFFSV1) || defined(RTCONFIG_BRCM_NAND_JFFS2)
	logmessage("jffs2", "valid logs(%d)", !jffs2_fail);
#endif
}

#if 0
static char
*make_var(char *prefix, int index, char *name)
{
	static char buf[100];

	assert(prefix);
	assert(name);

	if (index)
		snprintf(buf, sizeof(buf), "%s%d%s", prefix, index, name);
	else
		snprintf(buf, sizeof(buf), "%s%s", prefix, name);
	return buf;
}

static int is_wet(int idx, int unit, int subunit, void *param)
{
	return nvram_match(wl_nvname("mode", unit, subunit), "wet");
}
#endif

#define TMP_ROOTFS_MNT_POINT	"/sysroot"
/* Build temporarily rootfilesystem
 * @newroot:	Mount point of new root filesystem.
 * @return:
 * 	0:	success
 *     -1:	mount tmp filesystem fail
 *     -2:	copy files fail
 *     -3:	make directory fail
 *
 * WARNING
 * ==========================================================================
 *  YOU HAVE TO HANDLE THIS FUNCTION VERY CAREFUL.  IF YOU MISS FILE(S) THAT
 *  ARE NEED BY init PROCESS, e.g. /usr/lib/libnvram.so,
 *  KERNEL REBOOTS SYSTEM IN 3 SECONDS.  ERROR MESSAGE IS SHOWN BELOW:
 * ==========================================================================
 * Image successfully flashed
 * Kernel panic - not syncing: Attempted to kill init!
 * Rebooting in 3 seconds..
 * /sbin/init: can't load library 'libnvram.so'
 */
#if defined(RTCONFIG_TEMPROOTFS)
static int remove_tail_char(char *str, char c)
{
	char *p;

	if (!str || *str == '\0' || c == '\0')
		return -1;

	for (p = str + strlen(str); p >= str && *p == c; p--)
		*p = '\0';
	if (p == str)
		return -2;

	return 0;
}

/*
 * @param:	extra parameter of cp command
 * @dir:	base directory
 * @files:	files, e.g., "fileA fileB fileC"
 * @newroot:	new root directory
 * @return:
 * 	0:	success
 *
 * 1. cp("", "/bin", "fileA fileB fileC", "/sysroot") equals to below commands:
 *    a. mkdir -p /sysroot/bin
 *    b. cp -a /bin/fileA /bin/fileB /bin/fileC /sysroot/bin
 *
 * 2. cp("L", "/usr/bin", "" or NULL, "/sysroot") equals to below commands:
 *    a. if [ -e "/sysroot/usr/bin" ] ; then rmdir "/sysroot/usr/bin" ; fi
 *    b. mkdir -p `dirname /sysroot/usr/bin`
 *    c. cp -aL /usr/bin /sysroot
 */
static int __cp(const char *param, const char *dir, const char *files, const char *newroot)
{
	struct stat st;
	const char *sep = "/";
	int i, l, len = 0, len2 = 0, mode = 0;	/* copy files and sub-directory */
	char cmd[2048], *f, *p, *token, *ptr1;
	char d[PATH_MAX], dest_dir[PATH_MAX];
	char str1[] = "cp -afXXXXXXXXXXXXXYYY";
	const char delim[] = " ";

	if (!dir || !newroot || *newroot == '\0')
		return -1;

	if (!files || *files == '\0')
		mode = 1;	/* copy a directory recursive */

	if (!param)
		param = "";

	sprintf(str1, "cp -af%s", param);
	if (dir && *dir == '/')
		sep = "";
	sprintf(dest_dir, "%s", newroot);
	if (stat(dest_dir, &st) || !S_ISDIR(st.st_mode))
		return -2;

	switch (mode) {
	case 0:	/* copy files and sub-directory */
		if (!(f = strdup(files)))
			return -3;
		if (*dir != '\0') {
			sprintf(dest_dir, "%s%s%s", newroot, sep, dir);
			if (!d_exists(dest_dir))
				eval("mkdir", "-p", dest_dir);
			else if (!S_ISDIR(st.st_mode)) {
				_dprintf("%s exist and is not a directory!\n", dest_dir);
				return -4;
			}
		}

		strcpy(cmd, str1);
		len = strlen(cmd);
		p = cmd + len;
		len2 = strlen(dest_dir) + 2;	/* leading space + tail NULL */
		len += len2;
		for (i = l = 0, token = strtok_r(f, delim, &ptr1);
			token != NULL;
			token = strtok_r(NULL, delim, &ptr1), p += l, len += l)
		{
			sprintf(d, "%s/%s", dir, token);
			/* don't check existence if '?' or '*' exist */
			if (!strchr(d, '?') && !strchr(d, '*') && stat(d, &st) < 0) {
				_dprintf("%s: %s not exist, skip!\n", __func__, d);
				l = 0;
				continue;
			}

			l = strlen(d) + 1;
			if ((len + l) < sizeof(cmd)) {
				strcat(p, " ");
				strcat(p, d);
				++i;
				continue;
			}

			/* cmd buffer is not enough, flush */
			strcat(p, " ");
			strcat(p, dest_dir);
			system(cmd);

			strcpy(cmd, str1);
			len = strlen(cmd);
			p = cmd + len;
			len += len2;

			strcat(p, " ");
			strcat(p, d);
			i = 1;
		}

		if (i > 0) {
			strcat(p, " ");
			strcat(p, dest_dir);
			system(cmd);
		}
		free(f);
		break;
	case 1:	/* copy a directory recursive */
		/* If /newroot/bin exist and is directory, rmdir /newroot/bin.
		 * If not, /bin would be copied to newroot/bin/bin.
		 */
		if (*dir == '\0')
			return -10;

		sprintf(dest_dir, "%s%s%s", newroot, sep, dir);
		if (d_exists(dest_dir)) {
			_dprintf("%s exist, remove it\n", dest_dir);
			if (rmdir(dest_dir)) {
				_dprintf("rmdir %s (%s)\n", dest_dir, strerror(errno));
				return -11;
			}
		}

		/* remove tail '/' */
		strcpy(d, dest_dir);
		remove_tail_char(d, '/');

		/* make sure parent directory of destination directory exist */
		p = strrchr(d, '/');
		if (p && p != d) {
			*p = '\0';
			remove_tail_char(d, '/');
			if (!d_exists(d))
				eval("mkdir", "-p", d);
		}
		sprintf(cmd, "%s %s %s", str1, dir, dest_dir);
		system(cmd);

		break;
	default:
		_dprintf("%s: mode %d is not defined!\n", __func__, mode);
		return -4;
	}

	return 0;
}

/* Build a temporary rootfilesystem.
 *
 * If you add new binary to temp. rootfilesystem, check whether it needs another library!
 * For example, iwpriv needs libiw.so.29, libgcc_s.so.1, and libc.so.0:
 * $ mipsel-linux-objdump -x bin/iwpriv | grep NEED
 *   NEEDED               libiw.so.29
 *   NEEDED               libgcc_s.so.1
 *   NEEDED               libc.so.0
 *   VERNEED              0x004008d4
 *   VERNEEDNUM           0x00000001
 */
static int build_temp_rootfs(const char *newroot)
{
	int i, r;
	struct stat st;
	char d1[PATH_MAX], d2[1024];
	const char *mdir[] = { "/proc", "/tmp", "/sys", "/usr", "/var", "/var/lock" };
	const char *bin = "ash busybox cat cp dd df echo grep iwpriv kill ls ps mkdir mount nvram ping sh tar umount uname rm";
	const char *sbin = "init rc hotplug2 insmod lsmod modprobe reboot rmmod rtkswitch";
	const char *lib = "librt*.so* libnsl* libdl* libm* ld-* libiw* libgcc* libpthread* libdisk* libc*"
#if defined(RTCONFIG_PUSH_EMAIL)
			     " libws* libpush_log*"
#endif
		;
	const char *usrbin = "killall";
#ifdef RTCONFIG_BCMARM
	const char *usrsbin = ""
			     " nvram"
#endif
#if defined(RTCONFIG_BWDPI)
			     " shn_ctrl"
#endif
		;
	const char *usrlib = "libnvram.so libshared.so libcrypto.so* libbcm* libjson*"
#if defined(RTCONFIG_BWDPI)
			     " libbwdpi.so libbwdpi_sql.so"
#endif
#ifdef RTCONFIG_USB_SMS_MODEM
			     " libsmspdu.so"
#endif
#if defined(RTCONFIG_HTTPS) || defined(RTCONFIG_PUSH_EMAIL)
			     " libssl* libmssl*"
#if defined(RTCONFIG_PUSH_EMAIL)
			     " libcurl* libxml2*"
#endif
#endif
#if defined(RTCONFIG_NOTIFICATION_CENTER) || defined(RTCONFIG_BWDPI) || \
    defined(RTCONFIG_TRAFFIC_LIMITER) || defined(RTCONFIG_MEDIA_SERVER) || \
    defined(RTCONFIG_FREERADIUS) || defined(RTCONFIG_CAPTIVE_PORTAL) || \
    defined(RTCONFIG_WEBDAV)
			     " libsqlite*"
#endif
#if defined(RTCONFIG_PERMISSION_MANAGEMENT)
			     " libsqlcipher.so* libpms_sql.so"
#endif
#if defined(RTCONFIG_FBWIFI)
			     " libfbwifi.so"
#endif
#ifdef RTCONFIG_CFGSYNC
			     " libcfgmnt.so"
#endif
#ifdef RTCONFIG_OPENVPN
			     " libvpn.so"
#endif
#if defined(RTCONFIG_PROTECTION_SERVER)
			     " libptcsrv.so"
#endif
#if defined(RTCONFIG_LETSENCRYPT)
			     " libletsencrypt.so"
#endif
		;
	const char *kmod = "find /lib/modules -name '*.ko'|"
		"grep '\\("
#if defined(RTCONFIG_BLINK_LED)
		"bled\\|"			/* bled.ko */
		"usbcore\\|"			/* usbcore.ko */
#if LINUX_KERNEL_VERSION >= KERNEL_VERSION(3,2,0)
		"usb-common\\|"			/* usb-common.ko, kernel 3.2 or above */
#endif
#endif
		"nvram_linux\\)";		/* nvram_linux.ko */

	if (!newroot || *newroot == '\0')
		newroot = TMP_ROOTFS_MNT_POINT;

	if ((r = mount("tmpfs", newroot, "tmpfs", MS_NOATIME, "")) != 0)
		return -1;

	_dprintf("Build temp rootfs\n");
	__cp("", "/dev", "", newroot);
	__cp("", "/lib/modules", "modules.dep", newroot);
	__cp("", "/bin", bin, newroot);
	__cp("", "/sbin", sbin, newroot);
	__cp("", "/lib", lib, newroot);
	__cp("", "/lib", "libcrypt*", newroot);
	__cp("", "/usr/bin", usrbin, newroot);
#ifdef RTCONFIG_BCMARM
	__cp("", "/usr/sbin", usrsbin, newroot);
#endif
	__cp("", "/usr/lib", usrlib, newroot);
	__cp("L", "/etc", "", newroot);		/* don't creat symbolic link (/tmp/etc/foo) that will be broken soon */

#if defined(RTCONFIG_QCA)
	__cp("", "/sbin", "wlanconfig", newroot);
	__cp("", "/usr/lib", "libnl-tiny.so", newroot);
	__cp("", "/usr/sbin", "swconfig", newroot);
#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) || defined(RTCONFIG_WIFI_QCA9994_QCA9994)
	__cp("", "/usr/sbin", "thermald", newroot);
#endif
#endif

	/* copy mandatory kernel modules */
	sprintf(d2, "tar cvf - `%s` | tar xf - -C %s", kmod, newroot);
	system(d2);

	/* make directory, if not exist */
	for (i = 0; i < ARRAY_SIZE(mdir); ++i) {
		sprintf(d1, "%s/%s", newroot, mdir[i]);
		if (stat(d1, &st) && (r = mkdir(d1, 0755) == -1))
			return -3;
	}

	return 0;
}

/* Switch rootfilesystem to newroot.
 * @newroot:	Mount point of new root filesystem.
 * @return:
 * 	-1:	Not init process.
 * 	-2:	chdir to newroot fail
 * 	-3:	Newroot is not a mount point
 * 	-4:	Move mount point to / fail
 * 	-5:	exec new init process fail
 */
static int switch_root(const char *newroot)
{
	int r;
	dev_t rdev;
	struct stat st;
	char *const argv[] = { "/sbin/init", "reboot", NULL };

	if (!newroot || *newroot == '\0')
		newroot = TMP_ROOTFS_MNT_POINT;

	if (getpid() != 1) {
		_dprintf("%s: PID != 1\n", __func__);
		return -1;
	}

	if (chdir(newroot))
		return -2;
	stat("/", &st);
	rdev = st.st_dev;
	stat(".", &st);
	if (rdev == st.st_dev)
		return -3;

	/* emulate switch_root command */
	if ((r = mount(".", "/", NULL, MS_MOVE, NULL)) != 0)
		return -4;

	chroot(".");
	chdir("/");

	/* WARNING:
	 * If new rootfilesystem lacks libraries that are need by init process,
	 * kernel reboots system in 3 seconds.
	 */
	if ((r = execv(argv[0], argv)))
		return -5;

	/* NEVER REACH HERE */
	return 0;
}
#else	/* !RTCONFIG_TEMPROOTFS */
static inline int build_temp_rootfs(const char *newroot) { return -999; }
static inline int switch_root(const char *newroot) { return -999; }
#endif	/* RTCONFIG_TEMPROOTFS */

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
void PMS_Init_Database()
{
	if (f_exists(PMS_DB_FILE)) return;

	char para[256];

	// create all tables
	if (PMS_CreateAllTables() != 0)
	{
		printf("%s : create table error!\n", __FUNCTION__);
		return;
	}

	// setup default account from nvram
	PMS_ACCOUNT_INFO_T *tmp1 = NULL;
	if ((tmp1 = PMS_list_Account_new(5, "1", nvram_safe_get("http_username"), nvram_safe_get("http_passwd"), "", "")) != NULL)
	{
		//printf("%s : %d>%s>%s>%s>%s\n", __FUNCTION__, tmp1->active, tmp1->name, tmp1->passwd, tmp1->desc, tmp1->email);
		PMS_ActionAccountInfo(PMS_ACTION_UPDATE, (void *)tmp1, 0);
		PMS_list_ACCOUNT_free(tmp1);
	}

	// setup default account group : Adminstrator / FrontDesk / default
	PMS_ACCOUNT_GROUP_INFO_T *tmp2 = NULL;
	if ((tmp2 = PMS_list_AccountGroup_new(3, "1", "Administrator", "")) != NULL)
	{
		PMS_ActionAccountInfo(PMS_ACTION_UPDATE, (void *)tmp2, 1);
		PMS_list_ACCOUNT_GROUP_free(tmp2);
	}

	/* remove in 2016/11/21 due to FrontDesk function isn't ready, after function done, please rollback this */	
#if 0
	if ((tmp2 = PMS_list_AccountGroup_new(3, "0", "FrontDesk", "")) != NULL)
	{
		PMS_ActionAccountInfo(PMS_ACTION_UPDATE, (void *)tmp2, 1);
		PMS_list_ACCOUNT_GROUP_free(tmp2);
	}
#endif

	if ((tmp2 = PMS_list_AccountGroup_new(3, "1", "default", "")) != NULL)
	{
		PMS_ActionAccountInfo(PMS_ACTION_UPDATE, (void *)tmp2, 1);
		PMS_list_ACCOUNT_GROUP_free(tmp2);
	}

	// setup account / group matching table from account
	snprintf(para, sizeof(para), "%s>Administrator>FrontDesk", nvram_safe_get("http_username"));
	PMS_ActAccMatchInfo(PMS_ACTION_UPDATE, 3, para);

	// setup default device group : Adminstrator / default
	PMS_DEVICE_GROUP_INFO_T *tmp3 = NULL;
	if ((tmp3 = PMS_list_DeviceGroup_new(3, "1", "Administrator", "")) != NULL)
	{
		PMS_ActionDeviceInfo(PMS_ACTION_UPDATE, (void *)tmp3, 1);
		PMS_list_DEVICE_GROUP_free(tmp3);
	}

	if ((tmp3 = PMS_list_DeviceGroup_new(3, "1", "default", "")) != NULL)
	{
		PMS_ActionDeviceInfo(PMS_ACTION_UPDATE, (void *)tmp3, 1);
		PMS_list_DEVICE_GROUP_free(tmp3);
	}
}
#endif

void setup_passwd(void)
{
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
	PMS_Init_Database();
#endif
	create_passwd();
}

void create_passwd(void)
{
	char s[512];
	char *p;
	char salt[32];
	FILE *f;
	mode_t m;
	char *http_user;

#ifdef RTCONFIG_SAMBASRV	//!!TB
	char *smbd_user;

	create_custom_passwd();
#endif
#ifdef RTCONFIG_OPENVPN
	mkdir_if_none("/etc/pam.d");
	f_write_string("/etc/pam.d/openvpn",
		"auth required pam_unix.so\n",
		0, 0644);
	create_ovpn_passwd();
#endif

	strcpy(salt, "$1$");
	f_read("/dev/urandom", s, 6);
	base64_encode((unsigned char *) s, salt + 3, 6);
	salt[3 + 8] = 0;
	p = salt;
	while (*p) {
		if (*p == '+') *p = '.';
		++p;
	}

	if (((p = nvram_get("http_passwd")) == NULL) || (*p == 0)) p = "admin";
#ifdef RTCONFIG_NVRAM_ENCRYPT
	int declen = pw_dec_len(p);
	char dec_passwd[declen];
	memset(dec_passwd, 0, sizeof(dec_passwd));
	pw_dec(p, dec_passwd);
	p = dec_passwd;
#endif
	if (((http_user = nvram_get("http_username")) == NULL) || (*http_user == 0)) http_user = "admin";

#ifdef RTCONFIG_SAMBASRV	//!!TB
	if (((smbd_user = nvram_get("smbd_user")) == NULL) || (*smbd_user == 0) || !strcmp(smbd_user, "root"))
		smbd_user = "nas";
#endif

	m = umask(0777);
	if ((f = fopen("/etc/shadow", "w")) != NULL) {
		p = crypt(p, salt);
		fprintf(f,
				"%s:%s::0:99999:7:0::\n"
#ifdef RTCONFIG_TOR
				"tor:*::0:99999:7:0::\n"
#endif
#ifdef RTCONFIG_SAMBASRV	//!!TB
				"%s:*::0:99999:7:0::\n"
#endif
				"nobody:*::0:99999:7:0::\n"
				, http_user, p
#ifdef RTCONFIG_SAMBASRV	//!!TB
				, smbd_user
#endif
				);

		fappend(f, "/etc/shadow.custom");
#ifdef RTCONFIG_OPENVPN
		fappend(f, "/etc/shadow.openvpn");
#endif
		append_custom_config("shadow", f);
		fclose(f);
		run_postconf("shadow", "/etc/shadow");
	}
	umask(m);
	chmod("/etc/shadow", 0600);

	sprintf(s,
			"%s:x:0:0:%s:/root:/bin/sh\n"
#ifdef RTCONFIG_SAMBASRV	//!!TB
			"%s:x:100:100:nas:/dev/null:/dev/null\n"
#endif	//!!TB
			"nobody:x:65534:65534:nobody:/dev/null:/dev/null\n"
#ifdef RTCONFIG_IPSEC
			"admin:x:0:0:%s:/root:/dev/null\n"
#endif
#ifdef RTCONFIG_TOR
		"tor:x:65533:65533:tor:/dev/null:/dev/null\n"
#endif
			, http_user, http_user
#ifdef RTCONFIG_SAMBASRV	//!!TB
			, smbd_user
#endif	//!!TB
#ifdef RTCONFIG_IPSEC
			,http_user
#endif
			);
	f_write_string("/etc/passwd", s, 0, 0644);
	fappend_file("/etc/passwd", "/etc/passwd.custom");
	fappend_file("/etc/passwd", "/jffs/configs/passwd.add");
	run_postconf("passwd","/etc/passwd");
#ifdef RTCONFIG_OPENVPN
// TODO: figure out if libvpn does anything special here?
//	append_ovpn_accnt("/etc/passwd", "/etc/passwd.openvpn");
	fappend_file("/etc/passwd", "/etc/passwd.openvpn");
#endif

	sprintf(s,
		"root:*:0:\n"
#ifdef RTCONFIG_SAMBASRV	//!!TB
		"nas:*:100:\n"
#endif
		"nobody:*:65534:\n"
#ifdef RTCONFIG_TOR
		"tor:*:65533:\n"
#endif
		);
	f_write_string("/etc/gshadow", s, 0, 0644);
	fappend_file("/etc/gshadow", "/etc/gshadow.custom");
	fappend_file("/etc/gshadow", "/jffs/configs/gshadow.add");
	run_postconf("gshadow","/etc/gshadow");

	f_write_string("/etc/group",
		"root:x:0:\n"
#ifdef RTCONFIG_SAMBASRV	//!!TB
		"nas:x:100:\n"
#endif
		"nobody:x:65534:\n"
#ifdef RTCONFIG_TOR
		"tor:x:65533:\n"
#endif
		,0 , 0644);
	fappend_file("/etc/group", "/etc/group.custom");
#ifdef RTCONFIG_OPENVPN
	fappend_file("/etc/group", "/etc/group.openvpn");
#endif
	fappend_file("/etc/group", "/jffs/configs/group.add");
	run_postconf("group","/etc/group");
}

void get_dhcp_pool(char **dhcp_start, char **dhcp_end, char *buffer)
{
	if (dhcp_start == NULL || dhcp_end == NULL || buffer == NULL)
		return;

#ifdef RTCONFIG_WIRELESSREPEATER
#ifdef RTCONFIG_REALTEK
/* [MUST] : Need to discuss to add new mode for Media Bridge */
	if((repeater_mode() || mediabridge_mode()) && nvram_get_int("wlc_state") != WLC_STATE_CONNECTED){
#else
	if(sw_mode() == SW_MODE_REPEATER && nvram_get_int("wlc_state") != WLC_STATE_CONNECTED){
#endif
		if(nvram_match("lan_proto", "static")) {
			unsigned int lan_ipaddr, lan_netmask;
			char *p = buffer;
			struct in_addr lan1, lan2;
			unsigned offset;

			lan_ipaddr = ntohl(inet_addr("192.168.1.1"));
			lan_netmask = ntohl(inet_addr("255.255.255.0"));
//			cprintf("#### lan_ipaddr(%08x) lan_netmask(%08x)\n", lan_ipaddr, lan_netmask);

			//start
			if (((lan_ipaddr & lan_netmask) | 1) == lan_ipaddr)
				offset = 2;
			else
				offset = 1;
			lan1.s_addr = htonl((lan_ipaddr & lan_netmask) | offset);
			*dhcp_start = p;
			p += sprintf(p, "%s", inet_ntoa(lan1));
			p += 1;

			//end
			if (((lan_ipaddr & lan_netmask) | 254) == lan_ipaddr)
				offset = 253;
			else
				offset = 254;
			lan2.s_addr = htonl((lan_ipaddr & lan_netmask) | offset);
			*dhcp_end = p;
			p += sprintf(p, "%s", inet_ntoa(lan2));
			p += 1;

//			cprintf("#### dhcp_start(%s) dhcp_end(%s)\n", *dhcp_start, *dhcp_end);
		} else {
			*dhcp_start = nvram_default_get("dhcp_start");
			*dhcp_end = nvram_default_get("dhcp_end");
		}
	}
	else
#endif
	{
		*dhcp_start = nvram_safe_get("dhcp_start");
		*dhcp_end = nvram_safe_get("dhcp_end");
	}
}

#if 0
int get_dhcpd_lmax()
{
	unsigned int lstart, lend, lip;
	int dhlease_size, invalid_ipnum, except_lanip;
	char *dhcp_start, *dhcp_end, *lan_netmask, *lan_ipaddr;
	char buffer[64];

	get_dhcp_pool(&dhcp_start, &dhcp_end, buffer);
#ifdef RTCONFIG_WIRELESSREPEATER
	if(sw_mode() == SW_MODE_REPEATER && nvram_get_int("wlc_state") != WLC_STATE_CONNECTED){
		lan_netmask = nvram_default_get("lan_netmask");
		lan_ipaddr = nvram_default_get("lan_ipaddr");
	}
	else
#endif
	{
		lan_netmask = nvram_safe_get("lan_netmask");
		lan_ipaddr = nvram_safe_get("lan_ipaddr");
	}

	lstart = htonl(inet_addr(dhcp_start)) & ~htonl(inet_addr(lan_netmask));
	lend = htonl(inet_addr(dhcp_end)) & ~htonl(inet_addr(lan_netmask));
	lip = htonl(inet_addr(lan_ipaddr)) & ~htonl(inet_addr(lan_netmask));

	dhlease_size = lend - lstart + 1;
	invalid_ipnum = dhlease_size / 256 * 2;
	except_lanip = (lip >= lstart && lip <= lend)? 1:0;
	dhlease_size -= invalid_ipnum + except_lanip;

	return dhlease_size;
}
#endif

void stop_hour_monitor_service()
{
	//logmessage("hour monitor", "stop_hour_monitor_service");
	killall("hour_monitor", SIGTERM);
}

void start_hour_monitor_service()
{
	char *cmd[] = {"hour_monitor", NULL};
	int pid;

	if (!is_router_mode())
		return;

	if (!pids("hour_monitor")) {
		_eval(cmd, NULL, 0, &pid);
	}
}

void check_hour_monitor_service()
{
	if(hour_monitor_function_check()) start_hour_monitor_service();
}

#ifdef RTCONFIG_DHCP_OVERRIDE
static int chk_same_subnet(char *ip1, char *ip2, char *sub)
{
	unsigned int addr1, addr2, submask;

	if (!*ip1 || !*ip2 || !*sub)
		return 0;

	addr1 = ntohl(inet_addr(ip1));
	addr2 = ntohl(inet_addr(ip2));
	submask = ntohl(inet_addr(sub));

	return (addr1 & submask) == (addr2 & submask);
}

static void simple_dhcp_range(char *ip, char *dip1, char *dip2, char *mask)
{
	struct in_addr ina;
	unsigned int new_start, new_end, lmask;

	lmask = ntohl(inet_addr(mask));
	new_start = (ntohl(inet_addr(ip)) & lmask) + 1;
	new_end = ((ntohl(inet_addr(ip)) & lmask) | ~(lmask)) - 1;

	ina.s_addr = htonl(new_start);
	strcpy(dip1, inet_ntoa(ina));
	ina.s_addr = htonl(new_end);
	strcpy(dip2, inet_ntoa(ina));
}

static int chk_valid_startend(char *ip, char *ip1, char *ip2, char *sub)
{
	int result1, result2;

	result1 = chk_same_subnet(ip, ip1, sub);
	result2 = chk_same_subnet(ip, ip2, sub);

	if (!result1 || !result2) {
		simple_dhcp_range(ip, ip1, ip2, sub);
		return 0;
	}
	return 1;
}

static void link_down(void)
{
	char word[256], *next, ifnames[128];

	/* link down LAN ports */
#ifdef RTCONFIG_REALTEK
	lanport_ctrl(0);
#else
	eval("rtkswitch", "15");
#endif

#ifdef RTCONFIG_CONCURRENTREPEATER
#ifdef RTCONFIG_REALTEK
	if(access_point_mode()) {
#else
	if(sw_mode() == SW_MODE_AP) {
#endif
				/* ifconfig down wireless */
		strcpy(ifnames, nvram_safe_get("wl_ifnames"));
		foreach (word, ifnames, next) {
			SKIP_ABSENT_FAKE_IFACE(word);
#ifdef RTCONFIG_REALTEK
				eval("iwpriv", word, "set_mib", "func_off=1");
				eval("iwpriv", word, "del_sta", "all");
#else
				eval("iwpriv",word,"set", "RadioOn=0");
#endif
		}
	}
	else
#endif
	{
		/* ifconfig down wireless */
		strcpy(ifnames, nvram_safe_get("wl_ifnames"));
		foreach (word, ifnames, next) {
			SKIP_ABSENT_FAKE_IFACE(word);
			ifconfig(word, 0, NULL, NULL);
		}
	}

}

static void link_up(void)
{
	char word[256], *next, ifnames[128];
#ifdef RTCONFIG_CONCURRENTREPEATER
	char  tmp[100], prefix[]="wlXXXXXXX_";
	int  band = 0;
#endif
	/* link up LAN ports */
#ifdef RTCONFIG_REALTEK
	lanport_ctrl(1);
#else
	eval("rtkswitch", "14");
#endif

#ifdef RTCONFIG_CONCURRENTREPEATER
#ifdef RTCONFIG_REALTEK
	if(access_point_mode()) {
#else
	if(sw_mode() == SW_MODE_AP) {
#endif
				/* ifconfig down wireless */
		strcpy(ifnames, nvram_safe_get("wl_ifnames"));
		foreach (word, ifnames, next) {
			SKIP_ABSENT_BAND_AND_INC_UNIT(band);
			snprintf(prefix, sizeof(prefix), "wl%d_", band);
			if (nvram_match(strcat_r(prefix, "radio", tmp), "1"))
#ifdef RTCONFIG_REALTEK
				eval("iwpriv",word,"set_mib", "func_off=0");
#else
				eval("iwpriv",word,"set", "RadioOn=1");
#endif
			band++;
		}
	}
	else
#endif
	{
		/* ifconfig up wireless */
		strcpy(ifnames, nvram_safe_get("wl_ifnames"));
		foreach (word, ifnames, next) {
			SKIP_ABSENT_FAKE_IFACE(word);
			ifconfig(word, IFUP, NULL, NULL);
		}
	}
}

int restart_dnsmasq(int need_link_DownUp)
{
	if (need_link_DownUp) {
#if (defined(PLN12) || defined(PLAC56) || defined(PLAC66))
		nvram_set("plc_ready", "0");
#endif
#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_RALINK)
		nvram_set("lan_ready","0");
#endif
		link_down();
		sleep(9);
	}

	stop_dnsmasq();
	sleep(1);
	start_dnsmasq();

	if (need_link_DownUp) {
		link_up();
#if (defined(PLN12) || defined(PLAC56) || defined(PLAC66))
		nvram_set("plc_ready", "1");
#endif
#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_RALINK)
		int wlc_wait_time = nvram_get_int("wl_time") ? : 5;
		sleep(wlc_wait_time);
		nvram_set("lan_ready","1");
#endif
	}

	return 0;
}
#endif


#ifdef RTCONFIG_WIFI_SON
void gen_apmode_dnsmasq(void)
{
	FILE *fp;
 	char glan[24];
	char *value;
	unsigned int dip;
	struct in_addr guest;
	int count = nvram_get_int("dhcp_num");

	if ((fp = fopen("/etc/dnsmasq.conf", "w")) == NULL)
		return;
	dip= ntohl(inet_addr(APMODE_BRGUEST_IP));
	guest.s_addr = htonl(dip);
	strlcpy(glan, inet_ntoa(guest), sizeof(glan));
	if ((value = strrchr(glan, '.')) != NULL) *(value + 1) = 0;

	fprintf(fp, "pid-file=/var/run/dnsmasq.pid\n"
		    "user=nobody\n"
		    "bind-dynamic\n"		// listen only on interface & lo
		);
	fprintf(fp,"interface=%s\n",BR_GUEST);
	fprintf(fp,"resolv-file=/tmp/resolv.conf\n");
	fprintf(fp,"servers-file=/tmp/resolv.dnsmasq\n");
	fprintf(fp,"no-poll\n");
	fprintf(fp,"no-negcache\n");
	fprintf(fp,"cache-size=1500\n");
	fprintf(fp,"min-port=4096\n");
	fprintf(fp,"dhcp-range=guest,%s2,%s254,%s,%ds\n",
                                        glan,glan, nvram_safe_get("lan_netmask_rt"), 86400);
	fprintf(fp,"dhcp-option=lan,3,%s\n",APMODE_BRGUEST_IP);
	fprintf(fp,"dhcp-option=lan,252,\n");
	fprintf(fp,"dhcp-authoritative\n");
 	fprintf(fp,"listen-address=127.0.0.1,%s\n",nvram_safe_get("lan_ipaddr"));
 	fprintf(fp,"no-dhcp-interface=%s\n",nvram_safe_get("lan_ifname"));

	fclose(fp);
	stop_dnsmasq();
	eval("dnsmasq", "--log-async");
}
#endif

void start_dnsmasq(void)
{
	FILE *fp;
	char *lan_ifname, *lan_ipaddr;
	char *value, *value2;
	int i, have_dhcp = 0;
#ifdef RTCONFIG_IPSEC
	int unit;
	char tmpStr[20];
#endif

	TRACE_PT("begin\n");

	if (getpid() != 1) {
		notify_rc("start_dnsmasq");
		return;
	}

	stop_dnsmasq();

	if(f_exists("/etc/dnsmasq.conf"))
		unlink("/etc/dnsmasq.conf");

	lan_ifname = nvram_safe_get("lan_ifname");
#ifdef RTCONFIG_WIRELESSREPEATER
	if (sw_mode() == SW_MODE_REPEATER && nvram_get_int("wlc_state") != WLC_STATE_CONNECTED && !nvram_match("lan_proto", "static")) {
		lan_ipaddr = nvram_default_get("lan_ipaddr");
	} else
#ifdef RTCONFIG_REALTEK
/* [MUST] : Need to clarify .... */
	if (sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 1 && nvram_get_int("wlc_state") != WLC_STATE_CONNECTED && !nvram_match("lan_proto", "static")) { // Media bridge
		lan_ipaddr = nvram_default_get("lan_ipaddr");
	}
	else
#endif
#endif
	{
		lan_ipaddr = nvram_safe_get("lan_ipaddr");
	}

	/* write /etc/hosts */
	if ((fp = fopen("/etc/hosts", "w")) != NULL) {
		/* loclhost ipv4 */
		fprintf(fp, "127.0.0.1 localhost.localdomain localhost\n");
		/* lan hostname.domain hostname */
		if (nvram_invmatch("lan_hostname", "")) {
			fprintf(fp, "%s %s.%s %s\n", lan_ipaddr,
				    nvram_safe_get("lan_hostname"),
				    nvram_safe_get("lan_domain"),
				    nvram_safe_get("lan_hostname"));
		}
		/* productid/samba name */
		if (is_valid_hostname(value = nvram_safe_get("computer_name")) ||
			is_valid_hostname(value = get_productid()))
				fprintf(fp, "%s %s.%s %s\n", lan_ipaddr,
					    value, nvram_safe_get("lan_domain"), value);
		fprintf(fp, "%s %s\n", lan_ipaddr, DUT_DOMAIN_NAME);
		fprintf(fp, "%s %s\n", lan_ipaddr, OLD_DUT_DOMAIN_NAME1);
		fprintf(fp, "%s %s\n", lan_ipaddr, OLD_DUT_DOMAIN_NAME2);
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled()) {
			/* localhost ipv6 */
			fprintf(fp, "::1 ip6-localhost ip6-loopback\n");
			/* multicast ipv6 */
			fprintf(fp, "fe00::0 ip6-localnet\n"
				    "ff00::0 ip6-mcastprefix\n"
				    "ff02::1 ip6-allnodes\n"
				    "ff02::2 ip6-allrouters\n");
                       /* lan6 hostname.domain hostname */
                        value = (char*) ipv6_router_address(NULL);
			if (*value && nvram_invmatch("lan_hostname", "")) {
				fprintf(fp, "%s %s.%s %s\n", value,
					    nvram_safe_get("lan_hostname"),
					    nvram_safe_get("lan_domain"),
					    nvram_safe_get("lan_hostname"));
			}
		}
#endif
		append_custom_config("hosts", fp);
		fclose(fp);
		use_custom_config("hosts", "/etc/hosts");
		run_postconf("hosts","/etc/hosts");
	} else
		perror("/etc/hosts");


#ifdef RTCONFIG_REDIRECT_DNAME
	if (sw_mode() == SW_MODE_AP
#ifdef RTCONFIG_REALTEK
	&& nvram_get_int("wlc_psta") == 0
#endif
	)
	{
#ifdef RTCONFIG_DHCP_OVERRIDE
		if (nvram_match("dhcp_enable_x", "1") && nvram_match("dnsqmode", "2")
#ifdef RTCONFIG_DEFAULT_AP_MODE
				&& !nvram_match("ate_flag", "1")
#endif
		) {
			if ((fp = fopen("/etc/dnsmasq.conf", "w+")) != NULL) {
				/* DHCP range */
				char dhcp_start[16], dhcp_end[16], lan_netmask[16];

				strcpy(dhcp_start, nvram_safe_get("dhcp_start"));
				strcpy(dhcp_end, nvram_safe_get("dhcp_end"));
				strcpy(lan_netmask, nvram_safe_get("lan_netmask"));

				if (!chk_valid_startend(lan_ipaddr, dhcp_start, dhcp_end, lan_netmask)) {
					dbg("reset DHCP range: %s ~ %s\n", dhcp_start, dhcp_end);
					nvram_set("dhcp_start", dhcp_start);
					nvram_set("dhcp_end", dhcp_end);
				}

				fprintf(fp, "interface=%s\n", nvram_safe_get("lan_ifname"));
				fprintf(fp, "dhcp-range=lan,%s,%s,%s,%ss\n",
								dhcp_start,
								dhcp_end,
								lan_netmask,
								nvram_safe_get("dhcp_lease"));
				fprintf(fp, "dhcp-leasefile=/tmp/dnsmasq.leases\n");
				/* Gateway */
				fprintf(fp, "dhcp-option=lan,3,%s\n", lan_ipaddr);
				/* Faster for moving clients, if authoritative */
				fprintf(fp, "dhcp-authoritative\n");
				/* caching */
				fprintf(fp, "cache-size=1500\n"
					    "no-negcache\n");
				fclose(fp);
			}
			else
				perror("/etc/dnsmasq.conf");
		}
#endif

#ifdef RTCONFIG_MODEM_BRIDGE
		if(!(sw_mode() == SW_MODE_AP && nvram_get_int("modem_bridge")))
#endif
			eval("dnsmasq", "--log-async");
	}
#endif
	if (!is_routing_enabled()
#ifdef RTCONFIG_WIRELESSREPEATER
	 && sw_mode() != SW_MODE_REPEATER
#endif
	)
#ifdef RTCONFIG_REALTEK
/* [MUST]: Need to clarify ..... */
	{
		if (!(sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 1)) // Skip media bridge mode
			return;
	}
#else
#ifdef RTCONFIG_WIFI_SON
                if(sw_mode() == SW_MODE_AP && nvram_match("cfg_master", "1"))
                {
                        if(nvram_get_int("wl0.1_bss_enabled"))
				gen_apmode_dnsmasq();
                        return;
                }
                else
#endif
			return;
#endif

	// we still need dnsmasq in wet
	//if (foreach_wif(1, NULL, is_wet)) return;

	if ((fp = fopen("/etc/dnsmasq.conf", "w")) == NULL)
		return;

	fprintf(fp, "pid-file=/var/run/dnsmasq.pid\n"
		    "user=nobody\n"
		    "bind-dynamic\n"		// listen only on interface & lo
		);

#if defined(RTCONFIG_REDIRECT_DNAME)
	if(sw_mode() != SW_MODE_REPEATER
#ifdef RTCONFIG_REALTEK
/* [MUST] : Need to clarify ..... */
	&& !(sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 1) // skip media bridge
#endif
	)
#endif
	{
		fprintf(fp,"interface=%s\n",		// dns & dhcp on LAN interface
			lan_ifname);

#ifdef RTCONFIG_WIFI_SON
		fprintf(fp,"interface=%s\n",BR_GUEST);	// guest-network
#endif
#ifdef RTCONFIG_IPSEC
	if(nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")){
		for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
			sprintf(tmpStr,"wan%d_gw_ifname",unit);
			if(0 != strlen(nvram_safe_get(tmpStr)))
				fprintf(fp,"interface=%s\n", nvram_safe_get(tmpStr));	// dns on IPSec VPN interfaces
		}
	}
#endif
#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
		if (is_routing_enabled())
		fprintf(fp, "interface=%s\n"		// dns on VPN clients interfaces
		    	"no-dhcp-interface=%s\n",	// no dhcp for VPN clients
			"pptp*", "pptp*");
#endif
	}

#ifdef  __CONFIG_NORTON__
	/* TODO: dnsmasq doesn't support a single hostname across multiple interfaces */
	if (nvram_get_int("nga_enable"))
		fprintf(fp, "interface-name=norton.local,%s\n", lan_ifname);
#endif /* __CONFIG_NORTON__ */

#ifdef NORESOLV /* dnsmasq uses no resolv.conf */
	fprintf(fp, is_routing_enabled() ?
		"no-resolv\n" :			// no resolv, use only additional list
		"resolv-file=%s\n", dmresolv);	// the real stuff is here
#else
#ifdef RTCONFIG_YANDEXDNS
	if (nvram_get_int("yadns_enable_x") && nvram_get_int("yadns_mode") != YADNS_DISABLED) {
		fprintf(fp, "no-resolv\n");	// no resolv
	} else
#endif
	fprintf(fp, "resolv-file=%s\n",		// the real stuff is here
		dmresolv);
#endif

	fprintf(fp, "servers-file=%s\n"		// additional servers list
		    "no-poll\n"			// don't poll resolv file
		    "no-negcache\n"		// don't cace nxdomain
		    "cache-size=%u\n"		// dns cache size
		    "min-port=%u\n",		// min port used for random src port
		dmservers, 1500, nvram_get_int("dns_minport") ? : 4096);

	/* limit number of outstanding requests */
	{
		int max_queries = nvram_get_int("max_dns_queries");
#if defined(RTCONFIG_SOC_IPQ8064)
		if (max_queries == 0)
			max_queries = 1500;
#endif
		if (max_queries)
			fprintf(fp, "dns-forward-max=%d\n", max(150, min(max_queries, 10000)));
	}

	/* lan domain */
	value = nvram_safe_get("lan_domain");
	if (*value) {
		fprintf(fp, "domain=%s\n"
			    "expand-hosts\n", value);	// expand hostnames in hosts file
		if (nvram_get_int("lan_dns_fwd_local") != 1)
			fprintf(fp, "bogus-priv\n"
			            "local=/%s/\n", value);	// Don't forward local queries upstream
	}

	if (
		(is_routing_enabled() && nvram_get_int("dhcp_enable_x"))
#ifdef RTCONFIG_WIRELESSREPEATER
	 || (sw_mode() == SW_MODE_REPEATER && nvram_get_int("wlc_state") != WLC_STATE_CONNECTED)
#ifdef RTCONFIG_REALTEK
/* [MUST] : Need to Clarify ... */
	 || (sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 1 && nvram_get_int("wlc_state") != WLC_STATE_CONNECTED)
#endif
#endif
	) {
		char *dhcp_start, *dhcp_end;
		int dhcp_lease;
		char buffer[64];
#if defined(RTCONFIG_TR069) && !defined(RTCONFIG_TR181)
		unsigned char hwaddr[6];
#endif
#ifdef RTCONFIG_WIFI_SON
		int i;
		unsigned int dip[2];
		struct in_addr guest;
#endif
		have_dhcp |= 1; /* DHCPv4 */

		get_dhcp_pool(&dhcp_start, &dhcp_end, buffer);
#ifdef RTCONFIG_WIRELESSREPEATER
		if ((nvram_get_int("wlc_state") != WLC_STATE_CONNECTED) &&
			(sw_mode() == SW_MODE_REPEATER
#ifdef RTCONFIG_REALTEK
/* [MUST] : Need to Clarify ...  */
				|| (sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 1)	// Media bridge
#endif
			)
		)
		{
			dhcp_lease = atoi(nvram_default_get("dhcp_lease"));
		}
		else
#endif
		{
			dhcp_lease = nvram_get_int("dhcp_lease");
		}

		if (dhcp_lease <= 0)
			dhcp_lease = 86400;

		/* LAN range */
		if (*dhcp_start && *dhcp_end) {
#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_REALTEK)
			if(!nvram_match("lan_proto", "static")) // Dynamic. get_dhcp_pool() will return default dhcp_start & dhcp_end.
				fprintf(fp, "dhcp-range=lan,%s,%s,%s,%ds\n",
					dhcp_start, dhcp_end, nvram_default_get("lan_netmask"), dhcp_lease);
			else
#endif
			fprintf(fp, "dhcp-range=lan,%s,%s,%s,%ds\n",
				dhcp_start, dhcp_end, nvram_safe_get("lan_netmask"), dhcp_lease);

#ifdef RTCONFIG_WIFI_SON
			//if(nvram_get_int("wl0.1_bss_enabled"))
			{

				dip[0] = ntohl(inet_addr(dhcp_start))+0x100;
				dip[1] = ntohl(inet_addr(dhcp_end))+0x100;
				fprintf(fp, "dhcp-range=guest");
				for(i=0;i<2;i++)
				{
					guest.s_addr = htonl(dip[i]);
					fprintf(fp, ",%s",inet_ntoa(guest));
				}
				fprintf(fp, ",%s,%ds\n",nvram_safe_get("lan_netmask"), dhcp_lease);
			}
#endif

		} else {
			/* compatibility */
			char lan[24];
			int start = nvram_get_int("dhcp_start");
			int count = nvram_get_int("dhcp_num");

			strlcpy(lan, lan_ipaddr, sizeof(lan));
			if ((value = strrchr(lan, '.')) != NULL) *(value + 1) = 0;

			fprintf(fp, "dhcp-range=lan,%s%d,%s%d,%s,%ds\n",
				lan, start, lan, start + count - 1, nvram_safe_get("lan_netmask"), dhcp_lease);

#ifdef RTCONFIG_WIFI_SON
			//if(nvram_get_int("wl0.1_bss_enabled"))
			{
				char glan[24];
				dip[0] = ntohl(inet_addr(lan_ipaddr))+0x100;
				guest.s_addr = htonl(dip[0]);
				strlcpy(glan, inet_ntoa(guest), sizeof(glan));
				if ((value = strrchr(glan, '.')) != NULL) *(value + 1) = 0;

				fprintf(fp, "dhcp-range=guest,%s%d,%s%d,%s,%ds\n",
					glan, start, glan, start + count - 1, nvram_safe_get("lan_netmask"), dhcp_lease);

			}
#endif
		}

		/* Gateway, if not set, force use lan ipaddr to avoid repeater issue */
		value = nvram_safe_get("dhcp_gateway_x");
		value = (*value && inet_addr(value)) ? value : lan_ipaddr;
		fprintf(fp, "dhcp-option=lan,3,%s\n", value);

		/* DNS server and additional router address */
		value = nvram_safe_get("dhcp_dns1_x");
		value2 = nvram_safe_get("dhcp_dns2_x");
		if ((*value && inet_addr(value)) || (*value2 && inet_addr(value2)))
			fprintf(fp, "dhcp-option=lan,6%s%s%s%s%s\n",
			             (*value && inet_addr(value) ? "," : ""),
			             (*value && inet_addr(value) ? value : ""),
			             (*value2 && inet_addr(value2) ? "," : ""),
			             (*value2 && inet_addr(value2) ? value2 : ""),
			             (nvram_match("dhcpd_dns_router","1") ? ",0.0.0.0" : ""));

		/* LAN Domain */
		value = nvram_safe_get("lan_domain");
		if (*value)
			fprintf(fp, "dhcp-option=lan,15,%s\n", value);

		/* WINS server */
		value = nvram_safe_get("dhcp_wins_x");
		if (*value && inet_addr(value)) {
			fprintf(fp, "dhcp-option=lan,44,%s\n"
			/*	    "dhcp-option=lan,46,8\n"*/, value);
		}
#ifdef RTCONFIG_SAMBASRV
		/* Samba will serve as a WINS server */
		else if (nvram_invmatch("lan_domain", "") && nvram_get_int("smbd_wins")) {
			fprintf(fp, "dhcp-option=lan,44,%s\n"
			/*	    "dhcp-option=lan,46,8\n"*/, lan_ipaddr);
		}
#endif
		/* Shut up WPAD info requests */
		fprintf(fp, "dhcp-option=lan,252,\"\\n\"\n");

#if defined(RTCONFIG_TR069) && !defined(RTCONFIG_TR181)
		if (ether_atoe(get_lan_hwaddr(), hwaddr)) {
			snprintf(buffer, sizeof(buffer), "%02X%02X%02X", hwaddr[0], hwaddr[1], hwaddr[2]);
			fprintf(fp, "dhcp-option-force=cpewan-id,vi-encap:%d,%d,\"%s\"\n",
				3561, 4, buffer);
			snprintf(buffer, sizeof(buffer), "%02X%02X%02X%02X%02X%02X",
				 hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
			fprintf(fp, "dhcp-option-force=cpewan-id,vi-encap:%d,%d,\"%s\"\n",
				3561, 5, buffer);
			fprintf(fp, "dhcp-option-force=cpewan-id,vi-encap:%d,%d,\"%s\"\n",
				3561, 6, nvram_safe_get("productid"));
		}
#endif
	}

#ifdef RTCONFIG_IPV6
	if (ipv6_enabled() && is_routing_enabled()) {
		struct in6_addr addr;
		int ra_lifetime, dhcp_lifetime;
		int service, stateful, announce, dhcp_start, dhcp_end;

		service = get_ipv6_service();
		stateful = (service == IPV6_NATIVE_DHCP || service == IPV6_MANUAL) ?
			nvram_get_int(ipv6_nvname("ipv6_autoconf_type")) : 0;
		announce =
#ifdef RTCONFIG_6RELAYD
			service != IPV6_PASSTHROUGH &&
#endif
			nvram_get_int(ipv6_nvname("ipv6_radvd"));
		ra_lifetime = 600; /* 10 minutes for now */
		dhcp_lifetime = nvram_get_int(ipv6_nvname("ipv6_dhcp_lifetime"));
		if (dhcp_lifetime <= 0)
			dhcp_lifetime = 86400;

		if (announce) {
			fprintf(fp, "ra-param=%s,%d,%d\n"
				    "enable-ra\n"
				    "quiet-ra\n",
				lan_ifname, 10, ra_lifetime);
		}

		/* LAN prefix or range */
		if (stateful) {
			/* TODO: rework WEB UI to specify ranges without prefix
			 * TODO: add size checking, now range takes all of 16 bit */
			dhcp_start = (inet_pton(AF_INET6, nvram_safe_get(ipv6_nvname("ipv6_dhcp_start")), &addr) > 0) ?
			    ntohs(addr.s6_addr16[7]) : 0x1000;
			dhcp_end = (inet_pton(AF_INET6, nvram_safe_get(ipv6_nvname("ipv6_dhcp_end")), &addr) > 0) ?
			    ntohs(addr.s6_addr16[7]) : 0x2000;
			fprintf(fp, "dhcp-range=lan,::%04x,::%04x,constructor:%s,%d\n",
				(dhcp_start < dhcp_end) ? dhcp_start : dhcp_end,
				(dhcp_start < dhcp_end) ? dhcp_end : dhcp_start,
				lan_ifname, dhcp_lifetime);
			have_dhcp |= 2; /* DHCPv6 */
		} else if (announce) {
			if (nvram_get_int("ipv6_dhcp6s_enable")) {
				fprintf(fp, "dhcp-range=lan,::,constructor:%s,ra-stateless,%d,%d\n",
					lan_ifname, 64, ra_lifetime);
				have_dhcp |= 2; /* DHCPv6 */
			} else {
				fprintf(fp, "dhcp-range=lan,::,constructor:%s,ra-only,%d,%d\n",
					lan_ifname, 64, ra_lifetime);
			}
		}

#ifdef RTCONFIG_YANDEXDNS
		if (nvram_get_int("yadns_enable_x")) {
			unsigned char ea[ETHER_ADDR_LEN];
			char *name, *mac, *mode, *enable, *server[2];
			char *nv, *nvp, *b;
			int i, count, dnsmode, defmode = nvram_get_int("yadns_mode");

			for (dnsmode = YADNS_FIRST; dnsmode < YADNS_COUNT; dnsmode++) {
				if (dnsmode == defmode)
					continue;
				count = get_yandex_dns(AF_INET6, dnsmode, server, sizeof(server)/sizeof(server[0]));
				if (count <= 0)
					continue;
				fprintf(fp, "dhcp-option=yadns%u,option6:23", dnsmode);
				for (i = 0; i < count; i++)
					fprintf(fp, ",[%s]", server[i]);
				fprintf(fp, "\n");
			}

			/* DNS server per client */
			nv = nvp = strdup(nvram_safe_get("yadns_rulelist"));
			while (nv && (b = strsep(&nvp, "<")) != NULL) {
				if (vstrsep(b, ">", &name, &mac, &mode, &enable) < 3)
					continue;
				if (enable && atoi(enable) == 0)
					continue;
				if (!*mac || !*mode || !ether_atoe(mac, ea))
					continue;
				dnsmode = atoi(mode);
				/* Skip incorrect and default levels */
				if (dnsmode < YADNS_FIRST || dnsmode >= YADNS_COUNT || dnsmode == defmode)
					continue;
				fprintf(fp, "dhcp-host=%s,set:yadns%u\n", mac, dnsmode);
			}
			free(nv);
		}
#endif /* RTCONFIG_YANDEXDNS */

#ifdef RTCONFIG_DNSFILTER
		if (nvram_get_int("dnsfilter_enable_x"))
			dnsfilter_setup_dnsmasq(fp);
#endif

		/* DNS server */
		fprintf(fp, "dhcp-option=lan,option6:23,[::]\n");

		/* LAN Domain */
		value = nvram_safe_get("lan_domain");
		if (*value)
			fprintf(fp, "dhcp-option=lan,option6:24,%s\n", value);
	}
#endif

	if (have_dhcp) {
#if 0	//this would limit the total count of dhcp client (including dhcp pool and manually assigned static IP).
		/* Maximum leases */
		if ((i = get_dhcpd_lmax()) > 0)
			fprintf(fp, "dhcp-lease-max=%d\n", i);
#endif

		/* Faster for moving clients, if authoritative */
		if (nvram_get_int("dhcpd_auth") >= 0)
			fprintf(fp, "dhcp-authoritative\n");
#ifdef RTCONFIG_MULTICAST_IPTV
		/* Rawny: Add vendor class ID and DNS info for Movistar IPTV */
		if (nvram_get_int("switch_stb_x") > 6 &&
			nvram_match("switch_wantag", "movistar")) {
			fprintf(fp, "dhcp-vendorclass=ial,IAL\n");
			fprintf(fp, "dhcp-option=ial,6,172.26.23.3\n");
			fprintf(fp, "dhcp-option=ial,240,:::::239.0.2.10:22222:v6.0:239.0.2.30:22222\n");
		}
#endif
	} else
		fprintf(fp, "no-dhcp-interface=%s\n", lan_ifname);

#ifdef RTCONFIG_FINDASUS
	fprintf(fp, "address=/findasus.local/%s\n", lan_ipaddr);
#endif
#ifdef RTCONFIG_OPENVPN
	write_ovpn_dnsmasq_config(fp);
#endif

#ifdef RTCONFIG_PORT_BASED_VLAN
	/* Write dhcpd config for vlan's subnet */
	vlan_subnet_dnsmasq_conf(fp);
#endif
#ifdef RTCONFIG_TAGGED_BASED_VLAN
	/* Write dhcpd config for vlan's subnet */
	vlan_subnet_dnsmasq_conf(fp);
#endif
#if defined(RTCONFIG_COOVACHILLI)
	if (nvram_get_int("sw_mode") == SW_MODE_ROUTER) {
		fprintf(fp, "interface=%s\n", "tun22");
		fprintf(fp, "no-dhcp-interface=%s\n", "tun22");
		fprintf(fp, "interface=%s\n", "tun23");
		fprintf(fp, "no-dhcp-interface=%s\n", "tun23");
	}
#endif

	/* Static IP MAC binding */
	if (nvram_match("dhcp_static_x","1")) {
		write_static_leases(fp);
	}

	/* Don't log DHCP queries */
	if (nvram_match("dhcpd_querylog","0")) {
		fprintf(fp,"quiet-dhcp\n");
#ifdef RTCONFIG_IPV6
		fprintf(fp,"quiet-dhcp6\n");
#endif
	}

#ifdef RTCONFIG_DNSSEC
	if (nvram_match("dnssec_enable", "1")) {
		fprintf(fp, "trust-anchor=.,19036,8,2,49AAC11D7B6F6446702E54A1607371607A1A41855200FD2CE1CDDE32F24E8FB5\n"
		            "trust-anchor=.,20326,8,2,E06D44B80B8F1D39A95C0B0D7C65D08458E880409BBC683457104237C7F8EC8D\n"
		            "dnssec\n");

		/* If NTP isn't set yet, wait until rc's ntp signals us to start validating time */
		if (!nvram_get_int("ntp_ready"))
			fprintf(fp, "dnssec-no-timecheck\n");
	}
#endif

	append_custom_config("dnsmasq.conf",fp);
	fclose(fp);

	use_custom_config("dnsmasq.conf","/etc/dnsmasq.conf");
	run_postconf("dnsmasq","/etc/dnsmasq.conf");

	/* Create resolv.conf with empty nameserver list */
	f_write(dmresolv, NULL, 0, FW_APPEND, 0666);
	/* Create resolv.dnsmasq with empty server list */
	f_write(dmservers, NULL, 0, FW_APPEND, 0666);

#if (defined(RTCONFIG_TR069) && !defined(RTCONFIG_TR181))
	eval("dnsmasq", "--log-async", "-6", "/sbin/dhcpc_lease");
#else
	eval("dnsmasq", "--log-async");
#endif

	for ( i = 1; i < 4; i++ ) {
                if (!pids("dnsmasq")) {
			sleep(i);
		} else {
			// Make the router use dnsmasq for its own local resolution if it did start
			unlink("/etc/resolv.conf");
			symlink("/rom/etc/resolv.conf", "/etc/resolv.conf");	// nameserver 127.0.0.1
			i = 4;
		}
	}

	TRACE_PT("end\n");
}

void stop_dnsmasq(void)
{
	TRACE_PT("begin\n");

	if (getpid() != 1) {
		notify_rc("stop_dnsmasq");
		return;
	}

	// Revert back to ISP-filled resolv.conf
        unlink("/etc/resolv.conf");
        symlink(dmresolv, "/etc/resolv.conf");

	killall_tk("dnsmasq");

	TRACE_PT("end\n");
}

void reload_dnsmasq(void)
{

#ifdef RTCONFIG_DNSSEC
	if ((nvram_get_int("dnssec_enable") == 1) && (nvram_get_int("ntp_ready") == 1)) {
		/* Don't reload, as it would prematurely enable timestamp validation */
		stop_dnsmasq();
		sleep(1);
		start_dnsmasq();
	}
	else
#endif

	/* notify dnsmasq */
	kill_pidfile_s("/var/run/dnsmasq.pid", SIGHUP);
}

#ifdef RTCONFIG_IPV6
void add_ip6_lanaddr(void)
{
	char ip[INET6_ADDRSTRLEN + 4];
	const char *p;

	p = ipv6_router_address(NULL);
	if (*p) {
		snprintf(ip, sizeof(ip), "%s/%d", p, nvram_get_int(ipv6_nvname("ipv6_prefix_length")) ? : 64);
		eval("ip", "-6", "addr", "add", ip, "dev", nvram_safe_get("lan_ifname"));
		if (!nvram_match(ipv6_nvname("ipv6_rtr_addr"), (char*)p))
			nvram_set(ipv6_nvname("ipv6_rtr_addr"), (char*)p);
	}

	switch (get_ipv6_service()) {
	case IPV6_NATIVE_DHCP:
		if (nvram_get_int(ipv6_nvname("ipv6_dhcp_pd")))
			break;
		/* fall through */
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
#endif
	case IPV6_MANUAL:
		p = ipv6_prefix(NULL);
		if (*p && !nvram_match(ipv6_nvname("ipv6_prefix"), (char*)p))
			nvram_set(ipv6_nvname("ipv6_prefix"), (char*)p);
		break;
	}
}

void start_ipv6_tunnel(void)
{
	char ip[INET6_ADDRSTRLEN + 4];
	char router[INET6_ADDRSTRLEN + 1];
	struct in_addr addr4;
	struct in6_addr addr;
	char *wanip, *mtu, *tun_dev, *gateway;
	int service;

	// for one wan only now
	service = get_ipv6_service();
	tun_dev = (char*)get_wan6face();
	wanip = (char*)get_wanip();

	mtu = (nvram_get_int(ipv6_nvname("ipv6_tun_mtu")) > 0) ? nvram_safe_get(ipv6_nvname("ipv6_tun_mtu")) : "1480";
	modprobe("sit");

	eval("ip", "tunnel", "add", tun_dev, "mode", "sit",
		"remote", (service == IPV6_6IN4) ? nvram_safe_get(ipv6_nvname("ipv6_tun_v4end")) : "any",
		"local", wanip,
		"ttl", nvram_safe_get(ipv6_nvname("ipv6_tun_ttl")));
	eval("ip", "link", "set", tun_dev, "mtu", mtu, "up");

	switch (service) {
	case IPV6_6TO4: {
		int prefixlen = 16;
		int mask4size = 0;

		/* address */
		addr4.s_addr = 0;
		memset(&addr, 0, sizeof(addr));
		inet_aton(wanip, &addr4);
		addr.s6_addr16[0] = htons(0x2002);
		ipv6_mapaddr4(&addr, prefixlen, &addr4, mask4size);
		//addr.s6_addr16[7] |= htons(0x0001);
		inet_ntop(AF_INET6, &addr, ip, sizeof(ip));
		snprintf(ip, sizeof(ip), "%s/%d", ip, prefixlen);

		/* gateway */
		snprintf(router, sizeof(router), "::%s", nvram_safe_get(ipv6_nvname("ipv6_relay")));
		gateway = router;

		add_ip6_lanaddr();
		break;
	}
	case IPV6_6RD: {
		int prefixlen = nvram_get_int(ipv6_nvname("ipv6_6rd_prefixlen"));
		int mask4size = nvram_get_int(ipv6_nvname("ipv6_6rd_ip4size"));
		char brprefix[sizeof("255.255.255.255/32")];

		/* 6rd domain */
		addr4.s_addr = 0;
		if (mask4size) {
			inet_aton(wanip, &addr4);
			addr4.s_addr &= htonl(0xffffffffUL << (32 - mask4size));
		} else	addr4.s_addr = 0;
		snprintf(ip, sizeof(ip), "%s/%d", nvram_safe_get(ipv6_nvname("ipv6_6rd_prefix")), prefixlen);
		snprintf(brprefix, sizeof(brprefix), "%s/%d", inet_ntoa(addr4), mask4size);
		eval("ip", "tunnel", "6rd", "dev", tun_dev,
		     "6rd-prefix", ip, "6rd-relay_prefix", brprefix);

		/* address */
		addr4.s_addr = 0;
		memset(&addr, 0, sizeof(addr));
		inet_aton(wanip, &addr4);
		inet_pton(AF_INET6, nvram_safe_get(ipv6_nvname("ipv6_6rd_prefix")), &addr);
		ipv6_mapaddr4(&addr, prefixlen, &addr4, mask4size);
		//addr.s6_addr16[7] |= htons(0x0001);
		inet_ntop(AF_INET6, &addr, ip, sizeof(ip));
		snprintf(ip, sizeof(ip), "%s/%d", ip, prefixlen);

		/* gateway */
		snprintf(router, sizeof(router), "::%s", nvram_safe_get(ipv6_nvname("ipv6_6rd_router")));
		gateway = router;

		add_ip6_lanaddr();
		break;
	}
	default:
		/* address */
		snprintf(ip, sizeof(ip), "%s/%d",
			nvram_safe_get(ipv6_nvname("ipv6_tun_addr")),
			nvram_get_int(ipv6_nvname("ipv6_tun_addrlen")) ? : 64);

		/* gateway */
		gateway = nvram_safe_get(ipv6_nvname("ipv6_tun_peer"));
		if (gateway && *gateway)
			eval("ip", "-6", "route", "add", gateway, "dev", tun_dev, "metric", "1");
	}

	eval("ip", "-6", "addr", "add", ip, "dev", tun_dev);
	eval("ip", "-6", "route", "del", "::/0");

	if (gateway && *gateway)
		eval("ip", "-6", "route", "add", "::/0", "via", gateway, "dev", tun_dev, "metric", "1");
	else	eval("ip", "-6", "route", "add", "::/0", "dev", tun_dev, "metric", "1");
}

void stop_ipv6_tunnel(void)
{
	int service = get_ipv6_service();

	if (service == IPV6_6TO4 || service == IPV6_6RD || service == IPV6_6IN4) {
		eval("ip", "tunnel", "del", (char *)get_wan6face());
	}
	if (service == IPV6_6TO4 || service == IPV6_6RD) {
		// get rid of old IPv6 address from lan iface
		eval("ip", "-6", "addr", "flush", "dev", nvram_safe_get("lan_ifname"), "scope", "global");
		nvram_set(ipv6_nvname("ipv6_rtr_addr"), "");
		nvram_set(ipv6_nvname("ipv6_prefix"), "");
	}
	modprobe_r("sit");
}

void start_ipv6(void)
{
	int service = get_ipv6_service();

#if defined(RTCONFIG_MULTIWAN_CFG)
	int i;
	char prefix[sizeof("ipv6X_YYY")];

	for (i = WAN_UNIT_FIRST; i < WAN_UNIT_MAX; ++i) {
		if (!i)
			strcpy(prefix, "ipv6_");
		else
			snprintf(prefix, sizeof(prefix), "ipv6%d_", i);

		if (nvram_pf_match(prefix, "service", "disabled") ||
		    nvram_pf_match(prefix, "service", "dhcp6") ||
		    nvram_pf_match(prefix, "service", "6to4") ||
		    nvram_pf_match(prefix, "service", "6rd")
		   )
		{
			nvram_pf_set(prefix, "prefix", "");
			nvram_pf_set(prefix, "prefix_length", "");
			nvram_pf_set(prefix, "rtr_addr", "");
		}
		else if (nvram_pf_match(prefix, "service", "other"))	/* Static IPv6 */
		{
			nvram_pf_set(prefix, "prefix", "");
		}
		else if (nvram_pf_match(prefix, "service", "6in4"))
		{
			nvram_pf_set(prefix, "prefix", "");
			nvram_pf_set(prefix, "prefix_length", "");
		}
	}
#endif

	// Check if turned on
	switch (service) {
	case IPV6_NATIVE_DHCP:
		nvram_set(ipv6_nvname("ipv6_prefix"), "");
		if (nvram_get_int(ipv6_nvname("ipv6_dhcp_pd"))) {
			nvram_set(ipv6_nvname("ipv6_prefix_length"), "");
			nvram_set(ipv6_nvname("ipv6_rtr_addr"), "");
		} else
			add_ip6_lanaddr();
		nvram_set(ipv6_nvname("ipv6_get_dns"), "");
		nvram_set(ipv6_nvname("ipv6_get_domain"), "");
		break;
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
		nvram_set(ipv6_nvname("ipv6_prefix"), "");
		nvram_set(ipv6_nvname("ipv6_prefix_length"), "");
		nvram_set(ipv6_nvname("ipv6_rtr_addr"), "");
		nvram_set(ipv6_nvname("ipv6_get_dns"), "");
		nvram_set(ipv6_nvname("ipv6_get_domain"), "");
		break;
#endif
	case IPV6_MANUAL:
		nvram_set(ipv6_nvname("ipv6_prefix"), "");
		nvram_set(ipv6_nvname("ipv6_prefix_length"), nvram_safe_get(ipv6_nvname("ipv6_prefix_length_s")));
		nvram_set(ipv6_nvname("ipv6_rtr_addr"), nvram_safe_get(ipv6_nvname("ipv6_rtr_addr_s")));
		add_ip6_lanaddr();
		break;
	case IPV6_6IN4:
		nvram_set(ipv6_nvname("ipv6_prefix"), nvram_safe_get(ipv6_nvname("ipv6_prefix_s")));
		nvram_set(ipv6_nvname("ipv6_prefix_length"), nvram_safe_get(ipv6_nvname("ipv6_prefix_length_s")));
		nvram_set(ipv6_nvname("ipv6_rtr_addr"), "");
		add_ip6_lanaddr();
		break;
	case IPV6_6TO4:
	case IPV6_6RD:
		nvram_set(ipv6_nvname("ipv6_prefix"), "");
		nvram_set(ipv6_nvname("ipv6_prefix_length"), "");
		nvram_set(ipv6_nvname("ipv6_rtr_addr"), "");
		break;
	}
}

void stop_ipv6(void)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");
	char *wan_ifname = (char *) get_wan6face();
	char prefix[sizeof("ffff::/xxx")];
	int i;

#ifdef RTCONFIG_6RELAYD
	stop_6relayd();
#endif
	stop_dhcp6c();
	stop_ipv6_tunnel();

	eval("ip", "-6", "route", "flush", "default");
	for (i = 1; i < 8; i++) {
		snprintf(prefix, sizeof(prefix), "%04x::/%d", (0xfe0000 >> i) & 0xffff, i);
		eval("ip", "-6", "route", "flush", "root", prefix, "table", "main");
	}
	eval("ip", "-6", "addr", "flush", "scope", "global", "dev", lan_ifname);
	eval("ip", "-6", "addr", "flush", "scope", "global", "dev", wan_ifname);
	eval("ip", "-6", "neigh", "flush", "dev", lan_ifname);
}
#endif

// -----------------------------------------------------------------------------

int no_need_to_start_wps(void)
{
	int i, j, wps_band, multiband = get_wps_multiband();
	char tmp[100], tmp2[100], prefix[] = "wlXXXXXXXXXXXXXX", prefix_mssid[] = "wlXXXXXXXXXX_mssid_";
	char word[256], *next, ifnames[128];
	int c = 0, ret = 0;

#ifdef RTCONFIG_DSL
	if (nvram_match("asus_mfg", "1")) /* Paul add 2012/12/13 */
		return 0;
#endif

#ifdef RTCONFIG_CONCURRENTREPEATER
	if (sw_mode() != SW_MODE_REPEATER)
		return 0;
	if ((sw_mode() != SW_MODE_ROUTER) &&
		(sw_mode() != SW_MODE_AP) &&
		(sw_mode() != SW_MODE_REPEATER))
		return 1;
#else
	if ((sw_mode() != SW_MODE_ROUTER) &&
		(sw_mode() != SW_MODE_AP))
		return 1;
#endif

	i = 0;
	wps_band = nvram_get_int("wps_band_x");
	strcpy(ifnames, nvram_safe_get("wl_ifnames"));
	foreach (word, ifnames, next) {
		if (i >= MAX_NR_WL_IF)
			break;
		if (!multiband && wps_band != i) {
			++i;
			continue;
		}
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
		++c;
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		if ((nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "open") &&
		     nvram_get_int(strcat_r(prefix, "wep_x", tmp2))) ||
		     nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "shared") ||
		     strstr(nvram_safe_get(strcat_r(prefix, "auth_mode_x", tmp)), "wpa") ||
		     nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "radius"))
			ret++;

#ifdef RTCONFIG_RALINK
		if (nvram_match("wl_mssid", "1"))
#endif
		for (j = 1; j < MAX_NO_MSSID; j++) {
			sprintf(prefix_mssid, "wl%d.%d_", wps_band, j);
			if (!nvram_match(strcat_r(prefix_mssid, "bss_enabled", tmp), "1"))
				continue;
			++c;
			if ((nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "open") &&
			     nvram_get_int(strcat_r(prefix, "wep_x", tmp2))) ||
			     nvram_match(strcat_r(prefix_mssid, "auth_mode_x", tmp), "shared") ||
			     strstr(nvram_safe_get(strcat_r(prefix_mssid, "auth_mode_x", tmp)), "wpa") ||
			     nvram_match(strcat_r(prefix_mssid, "auth_mode_x", tmp), "radius"))
				ret++;
		}

		i++;
	}

	if (multiband && ret < c)
		ret = 0;

	return ret;
}

/* @wps_band:	if wps_band < 0 and RTCONFIG_WPSMULTIBAND is defined, check radio of all band */
int wps_band_radio_off(int wps_band)
{
	int i, c = 0, ret = 0;
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX";
	char word[256], *next, ifnames[128];

	i = 0;
	strcpy(ifnames, nvram_safe_get("wl_ifnames"));
	foreach (word, ifnames, next) {
		if (i >= MAX_NR_WL_IF)
			break;
		if (wps_band >= 0 && wps_band != i) {
			++i;
			continue;
		}
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
		++c;
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		if (nvram_match(strcat_r(prefix, "radio", tmp), "0"))
			ret++;

		i++;
	}

	if (wps_band < 0 && ret < c)
		ret = 0;

	return ret;
}

/* @wps_band:	if wps_band < 0 and RTCONFIG_WPSMULTIBAND is defined, check ssid broadcast status of all band */
int wps_band_ssid_broadcast_off(int wps_band)
{
	int i, c = 0, ret = 0;
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX";
	char word[256], *next, ifnames[128];

	i = 0;
#ifdef RTCONFIG_REALTEK
	if(sw_mode() == SW_MODE_REPEATER)
		return 0;
#endif
	strcpy(ifnames, nvram_safe_get("wl_ifnames"));
	foreach (word, ifnames, next) {
		if (i >= MAX_NR_WL_IF)
			break;
		if (wps_band >= 0 && wps_band != i) {
			++i;
			continue;
		}
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
		++c;
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		if (nvram_match(strcat_r(prefix, "closed", tmp), "1"))
			ret++;

		i++;
	}

	if (wps_band < 0 && ret < c)
		ret = 0;

	return ret;
}

int
wl_wpsPincheck(char *pin_string)
{
	unsigned long PIN = strtoul(pin_string, NULL, 10);
	unsigned long int accum = 0;
	unsigned int len = strlen(pin_string);

	if (len != 4 && len != 8)
		return 	-1;

	if (len == 8) {
		accum += 3 * ((PIN / 10000000) % 10);
		accum += 1 * ((PIN / 1000000) % 10);
		accum += 3 * ((PIN / 100000) % 10);
		accum += 1 * ((PIN / 10000) % 10);
		accum += 3 * ((PIN / 1000) % 10);
		accum += 1 * ((PIN / 100) % 10);
		accum += 3 * ((PIN / 10) % 10);
		accum += 1 * ((PIN / 1) % 10);

		if (0 == (accum % 10))
			return 0;
	}
	else if (len == 4)
		return 0;

	return -1;
}

void
check_wps_enable()
{
	int wps_enable = nvram_get_int("wps_enable_x");
	int unit = nvram_get_int("wps_band_x");

	if (no_need_to_start_wps() ||
		wps_band_ssid_broadcast_off(get_radio_band(unit))) {
		nvram_set("wps_enable_x", "0");
		wps_enable = 0;
	}

	nvram_set("wps_enable", (!wps_enable || wps_band_radio_off(get_radio_band(unit))) ? "0" : "1");
}

int
start_wps_pbc(int unit)
{
	if (no_need_to_start_wps()) return 1;

	if (wps_band_radio_off(get_radio_band(unit))) return 1;

	if (wps_band_ssid_broadcast_off(get_radio_band(unit))) return 1;

	if (nvram_match("wps_enable_x", "0"))
	{
		nvram_set("wps_enable_x", "1");
#ifdef CONFIG_BCMWL5
#ifdef RTCONFIG_BCMWL6
		restart_wireless();
		int delay_count = 10;
		while ((delay_count-- > 0) && !nvram_get_int("wlready"))
			sleep(1);
#else
		restart_wireless();
#endif
#else
		stop_wps();
#if !defined(RTCONFIG_WPSMULTIBAND)
		nvram_set_int("wps_band_x", unit);
#endif
		start_wps();
#endif
	}

#if !defined(RTCONFIG_WPSMULTIBAND)
	nvram_set_int("wps_band_x", unit);
#endif
	nvram_set("wps_sta_pin", "00000000");

	return start_wps_method();
}

int
start_wps_pin(int unit)
{
	if (!strlen(nvram_safe_get("wps_sta_pin"))) return 0;

	if (wl_wpsPincheck(nvram_safe_get("wps_sta_pin"))) return 0;

	nvram_set_int("wps_band_x", unit);

	return start_wps_method();
}

#ifdef RTCONFIG_WPS
int
stop_wpsaide()
{
	if (pids("wpsaide"))
		killall("wpsaide", SIGTERM);

	return 0;
}

int
start_wpsaide()
{
	char *wpsaide_argv[] = {"wpsaide", NULL};
	pid_t pid;
	int ret = 0;

	stop_wpsaide();

	ret = _eval(wpsaide_argv, NULL, 0, &pid);
	return ret;
}
#endif

#if defined(RTCONFIG_CONCURRENTREPEATER)
int
stop_led_monitor()
{
	if (pids("led_monitor"))
		killall("led_monitor", SIGTERM);

	return 0;
}
int
start_led_monitor()
{
	char *led_monitor_argv[] = {"led_monitor", NULL};
	pid_t pid;
	int ret = 0;

	stop_led_monitor();

	ret = _eval(led_monitor_argv, NULL, 0, &pid);
	return ret;
}

#if defined(RTCONFIG_RALINK)
int
stop_re_wpsc()
{
	if (pids("re_wpsc"))
		killall("re_wpsc", SIGTERM);

	return 0;
}
int
start_re_wpsc()
{
	char *re_wpsc_argv[] = {"re_wpsc", NULL};
	pid_t pid;
	int ret = 0;

	stop_re_wpsc();

	ret = _eval(re_wpsc_argv, NULL, 0, &pid);
	return ret;
}
#endif	/* RTCONFIG_RALINK */
#endif

extern int restore_defaults_g;

#ifdef HND_ROUTER

#endif

int
start_wps(void)
{
#ifdef RTCONFIG_WPS
#ifdef CONFIG_BCMWL5
	char *wps_argv[] = {"/bin/wps_monitor", NULL};
	pid_t pid;
#endif

	check_wps_enable();

	if (wps_band_radio_off(get_radio_band(nvram_get_int("wps_band_x"))))
		return 1;

#ifdef CONFIG_BCMWL5
	if (nvram_match("wps_restart", "1")) {
#ifdef HND_ROUTER
		wait_lan_port_to_forward_state();
#endif /* BCA_HNDROUTER */
		nvram_set("wps_restart", "0");
	}
	else {
		nvram_set("wps_restart", "0");
		nvram_set("wps_proc_status", "0");
		nvram_set("wps_proc_status_x", "0");
	}
#endif
	nvram_set("wps_sta_pin", "00000000");
	if (nvram_match("w_Setting", "1"))
		nvram_set("lan_wps_oob", "disabled");
	else
		nvram_set("lan_wps_oob", "enabled");
#ifdef CONFIG_BCMWL5
	killall_tk("wps_monitor");
	killall_tk("wps_ap");
	killall_tk("wps_enr");
	unlink("/tmp/wps_monitor.pid");
#endif
	if (nvram_match("wps_enable", "1"))
	{
#ifdef CONFIG_BCMWL5
		nvram_set("wl_wps_mode", "enabled");
#ifdef CONFIG_BCMWL5
		if (!restore_defaults_g)
#endif
		_eval(wps_argv, NULL, 0, &pid);
#elif defined RTCONFIG_RALINK
		start_wsc_pin_enrollee();
		if (f_exists("/var/run/watchdog.pid"))
		{
			doSystem("iwpriv %s set WatchdogPid=`cat %s`", WIF_2G, "/var/run/watchdog.pid");
#if defined(RTCONFIG_HAS_5G)
			doSystem("iwpriv %s set WatchdogPid=`cat %s`", WIF_5G, "/var/run/watchdog.pid");
#endif	/* RTCONFIG_HAS_5G */
		}
#elif defined RTCONFIG_REALTEK
		rtk_start_wsc();
#endif
	}
#ifdef CONFIG_BCMWL5
	else
		nvram_set("wl_wps_mode", "disabled");
#endif
#else
	/* if we don't support WPS, make sure we unset any remaining wl_wps_mode */
	nvram_unset("wl_wps_mode");
#endif /* RTCONFIG_WPS */
	return 0;
}

int
stop_wps(void)
{
	int ret = 0;
#ifdef RTCONFIG_WPS
#ifdef CONFIG_BCMWL5
	killall_tk("wps_monitor");
	killall_tk("wps_ap");
#elif defined RTCONFIG_RALINK
	stop_wsc_both();
#elif defined RTCONFIG_REALTEK
	rtk_stop_wsc();
#endif
#endif /* RTCONFIG_WPS */
	return ret;
}

/* check for dual band case */
void
reset_wps(void)
{
#if defined(CONFIG_BCMWL5) || defined(RTCONFIG_REALTEK)
	stop_wps_method();

	stop_wps();

	nvram_set("w_Setting", "0");

	nvram_set("wps_reset", "1");

	restart_wireless();
#elif defined(RTCONFIG_LANTIQ)
	nvram_set("w_Setting", "0");

	stop_wps_method();

	wps_oob();
#elif defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)
	wps_oob_both();
#endif
}

#ifdef RTCONFIG_HSPOT
void
stop_hspotap(void)
{
	killall_tk("hspotap");
}

int
start_hspotap(void)
{
	char *hs_argv[] = {"/bin/hspotap", NULL};
	pid_t pid;

	if (!check_if_file_exist("/bin/hspotap")) return 0;

	stop_hspotap();

	_eval(hs_argv, NULL, 0, &pid);

	return 0;
}
#endif


#ifdef RTCONFIG_WLCEVENTD
int start_wlceventd(void)
{
	char *ev_argv[] = {"/usr/sbin/wlceventd", NULL};
	pid_t pid = 0;
	int ret = 0;

#ifdef CONFIG_BCMWL5
#if !(defined(HND_ROUTER) && defined(RTCONFIG_HNDMFG))
	if (factory_debug())
#endif
#else
	if (IS_ATE_FACTORY_MODE())
#endif
	return ret;

	stop_wlceventd();

#ifdef RTCONFIG_CONCURRENTREPEATER
	if (mediabridge_mode())
		return;
#endif

	ret = _eval(ev_argv, NULL, 0, &pid);

	return ret;
}

void stop_wlceventd(void)
{
	killall_tk("wlceventd");
}
#endif

#ifdef RTCONFIG_NOTIFICATION_CENTER
int start_wlc_nt(void)
{
	char *ev_argv[] = {"/usr/sbin/wlc_nt", NULL};
	pid_t pid = 0;
	int ret = 0;

#ifdef CONFIG_BCMWL5
#if !(defined(HND_ROUTER) && defined(RTCONFIG_HNDMFG))
	if (factory_debug())
#endif
#else
	if (IS_ATE_FACTORY_MODE())
#endif
	return ret;

	stop_wlc_nt();

#ifdef RTCONFIG_CONCURRENTREPEATER
	if (mediabridge_mode())
		return;
#endif

	ret = _eval(ev_argv, NULL, 0, &pid);

	return ret;
}

void stop_wlc_nt(void)
{
	killall_tk("wlc_nt");
}
#endif

#ifdef CONFIG_BCMWL5
#ifdef BCM_ASPMD
int
start_aspmd(void)
{
	int ret = eval("/usr/sbin/aspmd");

	return ret;
}

void
stop_aspmd(void)
{
	killall_tk("aspmd");
}
#endif /* BCM_ASPMD */

#ifdef BCM_EVENTD
int start_eventd(void)
{
	int ret = 0;
	char *ssd_argv[] = {"/usr/sbin/eventd", NULL};
	pid_t pid;

	if (nvram_match("eventd_enable", "1"))
		ret = _eval(ssd_argv, NULL, 0, &pid);

	return ret;
}

void stop_eventd(void)
{
	killall_tk("eventd");
}
#endif /* BCM_EVENTD */

int
start_eapd(void)
{
	int ret = 0;

	stop_eapd();

	if (!restore_defaults_g) {
#ifdef RTCONFIG_PORT_BASED_VLAN
		change_lan_ifnames();
#endif
		ret = eval("/bin/eapd");
	}

	return ret;
}

void
stop_eapd(void)
{
	killall_tk("eapd");
}

#ifdef BCM_DCS
int
start_dcsd(void)
{
	int ret = eval("/usr/sbin/dcsd");

	return ret;
}

void
stop_dcsd(void)
{
	killall_tk("dcsd");
}
#endif /* BCM_DCS */

#ifdef RTCONFIG_BCM_7114
void
start_dfs()
{
	int unit = 0;
	char word[256], *next;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";

	if (!nvram_match("wl1_country_code", "E0") && !nvram_match("wl1_country_code", "JP") && !nvram_match("wl1_country_code", "AU"))
		return ;

	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		if (nvram_match(strcat_r(prefix, "nband", tmp), "1"))
		{
			eval("wl", "-i", word, "down");
			eval("wl", "-i", word, "spect", "1");
			eval("wl", "-i", word, "up");
		}
		unit++;
	}
}
#endif

#ifdef  __CONFIG_VISUALIZATION__
static void
start_visualization_tool(void)
{
	eval("vis-dcon");
	eval("vis-datacollector");
}

static void
stop_visualization_tool(void)
{
	killall_tk("vis-dcon");
	killall_tk("vis-datacollector");
}
#endif /* __CONFIG_VISUALIZATION__ */
#endif

//2008.10 magic{
int start_networkmap(int bootwait)
{
	char *networkmap_argv[] = {"networkmap", NULL, NULL};
	pid_t pid;

	//if (!is_routing_enabled())
	//	return 0;

#ifdef RTCONFIG_DISABLE_NETWORKMAP
	if (nvram_match("networkmap_enable", "0"))
		return 0;
#endif
#ifdef RTCONFIG_WIFI_SON
	if (sw_mode() == SW_MODE_AP && !nvram_match("cfg_master", "1"))
		return 0;
#endif

	stop_networkmap();

	if (bootwait)
		networkmap_argv[1] = "--bootwait";

#ifdef RTCONFIG_UPNPC
	start_miniupnpc();
#endif
#ifdef RTCONFIG_BONJOUR
	start_netmonitor();
#endif
	_eval(networkmap_argv, NULL, 0, &pid);

	return 0;
}

//2008.10 magic}

void stop_networkmap(void)
{
	killall_tk("networkmap");
#ifdef RTCONFIG_BONJOUR
	stop_netmonitor();
#endif
#ifdef RTCONFIG_UPNPC
	stop_miniupnpc();
#endif
}

#ifdef RTCONFIG_JFFS2USERICON
void stop_lltdc(void)
{
	if (pids("lld2c"))
		killall_tk("lld2c");
}

int start_lltdc(void)
{
	char *lld2c_argv[] = {"lld2c", "br0", NULL};
	pid_t pid;

	if (pids("lld2c"))
		return 0;

	_eval(lld2c_argv, NULL, 0, &pid);

	return 0;
}
#endif

#ifdef RTCONFIG_UPNPC
void stop_miniupnpc(void)
{
	if (pids("miniupnpc"))
		killall_tk("miniupnpc");
}

int start_miniupnpc(void)
{
	if (pids("miniupnpc"))
		return 0;

	return xstart("miniupnpc", "-m", "br0", "-t");
}
#endif

#ifdef RTCONFIG_BONJOUR
void stop_netmonitor(void)
{
	if (pids("mDNSNetMonitor"))
		killall_tk("mDNSNetMonitor");
}

int start_netmonitor(void)
{
	char *netmonitor_argv[] = {"mDNSNetMonitor", NULL};
	pid_t pid;

	if (pids("mDNSNetMonitor"))
		return 0;

	_eval(netmonitor_argv, NULL, 0, &pid);

	return 0;
}
#endif

#ifdef RTCONFIG_LLDP
int start_lldpd(void)
{
	char *lldpd_argv[] = {"lldpd", nvram_safe_get("lan_ifname"), NULL};
	pid_t pid;

	_eval(lldpd_argv, NULL, 0, &pid);

	return 0;
}
#endif
// -----------------------------------------------------------------------------
#ifdef LINUX26

static pid_t pid_hotplug2 = -1;

void start_hotplug2(void)
{
	stop_hotplug2();

	xstart("hotplug2", "--persistent", "--no-coldplug");
	// FIXME: Don't remember exactly why I put "sleep" here -
	// but it was not for a race with check_services()... - TB
	sleep(1);

	if (!nvram_contains_word("debug_norestart", "hotplug2")) {
		pid_hotplug2 = -2;
	}
}

void stop_hotplug2(void)
{
	pid_hotplug2 = -1;
	killall_tk("hotplug2");
}

#endif	/* LINUX26 */


void
stop_infosvr()
{
	killall_tk("infosvr");
}

int
start_infosvr()
{
	char *infosvr_argv[] = {"/usr/sbin/infosvr", "br0", NULL};
	pid_t pid;

#ifdef RTCONFIG_MODEM_BRIDGE
	if(sw_mode() == SW_MODE_AP && nvram_get_int("modem_bridge"))
		return 0;
#endif

	return _eval(infosvr_argv, NULL, 0, &pid);
}

#ifdef RTCONFIG_RALINK
int
exec_8021x_start(int band, int is_iNIC)
{
	char tmp[100], prefix[] = "wlXXXXXXX_";
	char *str;
	int flag_8021x = 0;
	int i;

	if (sw_mode() == SW_MODE_REPEATER && nvram_get_int("wlc_band") == band)
		return 0;

	for (i = 0; i < MAX_NO_MSSID; i++)
	{
		if (i)
		{
			sprintf(prefix, "wl%d.%d_", band, i);

			if (!nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1"))
				continue;
		}
		else
			sprintf(prefix, "wl%d_", band);

		str = nvram_safe_get(strcat_r(prefix, "auth_mode_x", tmp));

		if(str && strlen(str) > 0)
		{
			if (    !strcmp(str, "radius") ||
				!strcmp(str, "wpa") ||
				!strcmp(str, "wpa2") ||
				!strcmp(str, "wpawpa2") )
			{ //need daemon
				flag_8021x = 1;
				break;
			}
		}
	}

	if(flag_8021x)
	{
		if (is_iNIC)
			return xstart("rtinicapd");
		else
			return xstart("rt2860apd");
	}
	return 0;
}

int
start_8021x(void)
{
	char word[256], *next;

	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		if (!strcmp(word, WIF_2G))
		{
			if (!strncmp(word, "rai", 3))	// iNIC
				exec_8021x_start(0, 1);
			else
				exec_8021x_start(0, 0);
		}
#if defined(RTCONFIG_HAS_5G)
		else if (!strcmp(word, WIF_5G))
		{
			if (!strncmp(word, "rai", 3))	// iNIC
				exec_8021x_start(1, 1);
			else
				exec_8021x_start(1, 0);
		}
#endif	/* RTCONFIG_HAS_5G */
	}

	return 0;
}

int
exec_8021x_stop(int band, int is_iNIC)
{
		if (is_iNIC)
			return killall("rtinicapd", SIGTERM);
		else
			return killall("rt2860apd", SIGTERM);
}

int
stop_8021x(void)
{
	char word[256], *next;

	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		if (!strcmp(word, WIF_2G))
		{
			if (!strncmp(word, "rai", 3))	// iNIC
				exec_8021x_stop(0, 1);
			else
				exec_8021x_stop(0, 0);
		}
#if defined(RTCONFIG_HAS_5G)
		else if (!strcmp(word, WIF_5G))
		{
			if (!strncmp(word, "rai", 3))	// iNIC
				exec_8021x_stop(1, 1);
			else
				exec_8021x_stop(1, 0);
		}
#endif	/* RTCONFIG_HAS_5G */
	}

	return 0;
}
#endif

#ifdef RTCONFIG_REALTEK
int exec_8021x_start(int band, int type)
{
	TRACE_PT("start 802.1x daemon here\n");
	return 0;
}

int start_8021x(void)
{
	rtk_start_auth();
	TRACE_PT("start 802.1x daemon here\n");
	return 0;
}

int exec_8021x_stop(int band, int type)
{
	TRACE_PT("kill 802.1x daemon here\n");
	doSystem("killall -9 auth");
	doSystem("killall -9 iwcontrol");
	return 0;
}

int stop_8021x(void)
{
	TRACE_PT("stop 802.1x daemon here\n");
	doSystem("killall -9 auth");
	doSystem("killall -9 iwcontrol");
	return 0;

}
#endif

/**
 * Use "dhcp-host=INTERFACE,MAC,IP" to specify static IP list of each LAN/VLANs.
 * @fp:	FILE pointer to /etc/dnsmasq.conf
 */
void write_static_leases(FILE *fp)
{
	FILE *fp2;
	char *nv, *nvp, *b;
	char *mac, *ip, *name;
	char lan_if[IFNAMSIZ];
	int vars;
	in_addr_t ip1, lan_net, lan_mask;
#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
	int i, nr_vnets = 0, host_bits;
	char *p, ip_str[sizeof("192.168.100.200XXX")];
	struct vlan_rules_s *vlan_rules;
	struct vlan_rule_s *r;
	struct vlan_config_s {
		in_addr_t net;
		in_addr_t mask;
		char br_if[IFNAMSIZ];
	} vlan_nets[VLAN_MAX_NUM], *v;
#endif

	if (!fp)
		return;

	fp2 = fopen(dmhosts, "w");
	if (!fp2)
		return;

	nv = nvp = strdup(nvram_safe_get("dhcp_staticlist"));
	if (!nv) {
		fclose(fp2);
		return;
	}

	fprintf(fp, "addn-hosts=%s\n", dmhosts);

	/* Initialize LAN network and mask */
	strlcpy(lan_if, nvram_get("lan_ifname")? : nvram_default_get("lan_ifname"), sizeof(lan_if));
	ip1 = inet_network(nvram_safe_get("lan_ipaddr"));
	lan_mask = inet_network(nvram_safe_get("lan_netmask"));
	if (ip1 == -1 || lan_mask == -1)
		return;
	lan_net = ip1 & lan_mask;

#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
	/* Get network and netmasks of each enabled and VID != 1 VLANs. */
	memset(&vlan_nets, 0, sizeof(vlan_nets));
	vlan_rules = get_vlan_rules();
	if (vlan_rules != NULL) {
		for (i = 0, nr_vnets = 0, v = &vlan_nets[0], r = &vlan_rules->rules[0];
		     i < vlan_rules->nr_rules;
		     ++i, ++r)
		{
			if (!r->enable || safe_atoi(r->vid) == 1 || !strcmp(r->subnet_name, "default"))
				continue;

			strlcpy(v->br_if, r->br_if, sizeof(v->br_if));
			if (!(p = strchr(r->subnet_name, '/')) || illegal_ipv4_netmask(p + 1))
				continue;

			if (strchr(p + 1, '.')) {
				/* A.B.C.D */
				v->mask = inet_network(p + 1);
				if (v->mask == -1)
					continue;
			} else {
				/* CIDR */
				host_bits = 32 - safe_atoi(p + 1);
				if (host_bits <= 0 || host_bits > 32)
					continue;
				v->mask = (~0U >> host_bits) << host_bits;
			}
			strlcpy(ip_str, r->subnet_name, min(p - r->subnet_name + 1, sizeof(ip_str)));
			if ((ip1 = inet_network(ip_str)) == -1)
				continue;
			v->net = ip1 & v->mask;
			v++;
			nr_vnets++;
		}
		free(vlan_rules);
	}
#endif

	/* Parsing dhcp_staticlist nvram variable. */
	while ((b = strsep(&nvp, "<")) != NULL) {
		vars = vstrsep(b, ">", &mac, &ip, &name);
		if ((vars != 2) && (vars != 3))
			continue;
		if (!strlen(mac) || !strlen(ip) || (ip1 = inet_network(ip)) == -1)
			continue;

		if ((vars == 3) && (strlen(name)) && (is_valid_hostname(name))) {
			fprintf(fp2, "%s %s\n", ip, name);
		}

		if ((ip1 & lan_mask) == lan_net) {
			fprintf(fp, "dhcp-host=%s,%s\n", mac, ip);
			continue;
		}
#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
		for (i = 0, v = &vlan_nets[0]; i < nr_vnets; ++i, ++v) {
			if ((ip1 & v->mask) != v->net || *v->br_if == '\0')
				continue;

			fprintf(fp, "dhcp-host=%s,%s\n", mac, ip);
			break;
		}
#endif
	}
	free(nv);
	fclose(fp2);
}

int
ddns_updated_main(int argc, char *argv[])
{
	FILE *fp;
	char buf[64], *ip;

	if (!(fp=fopen("/tmp/ddns.cache", "r"))) return 0;

	fgets(buf, sizeof(buf), fp);
	fclose(fp);

	if (!(ip=strchr(buf, ','))) return 0;

	nvram_set("ddns_cache", buf);
	nvram_set("ddns_ipaddr", ip+1);
	nvram_set("ddns_status", "1");
	nvram_set("ddns_server_x_old", nvram_safe_get("ddns_server_x"));
	nvram_set("ddns_hostname_old", nvram_safe_get("ddns_hostname_x"));
	nvram_set("ddns_updated", "1");

	logmessage("ddns", "ddns update ok");

#ifdef RTCONFIG_LETSENCRYPT
	if (nvram_match("le_rc_notify", "1")) {
		nvram_set("le_rc_notify", "0");
		stop_letsencrypt();
		start_letsencrypt();
	}
#endif

#ifdef RTCONFIG_OPENVPN
	update_ovpn_profie_remote();
#endif

	_dprintf("done\n");

	return 0;
}

// TODO: handle wan0 only now
int
start_ddns(void)
{
	FILE *fp;
	FILE *time_fp;
	char *wan_ip, *wan_ifname;
//	char *ddns_cache;
	char *server;
	char *user;
	char *passwd;
	char *host;
	char *service;
	char usrstr[64];
	int wild = nvram_get_int("ddns_wildcard_x");
	int unit, asus_ddns = 0;
	char tmp[32], prefix[] = "wanXXXXXXXXXX_";
	time_t now;
	pid_t pid;

	if (!is_routing_enabled()) {
		_dprintf("return -1\n");
		return -1;
	}
	if (nvram_invmatch("ddns_enable_x", "1"))
		return -1;

	unit = wan_primary_ifunit();
#if defined(RTCONFIG_DUALWAN)
	if (nvram_match("wans_mode", "lb")) {
		int ddns_wan_unit = nvram_get_int("ddns_wan_unit");

		if (ddns_wan_unit >= WAN_UNIT_FIRST && ddns_wan_unit < WAN_UNIT_MAX) {
			unit = ddns_wan_unit;
		} else {
			int u = get_first_configured_connected_wan_unit();

			if (u < WAN_UNIT_FIRST || u >= WAN_UNIT_MAX)
				return -2;

			unit = u;
		}
	}
#endif
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	wan_ip = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));
	wan_ifname = get_wan_ifname(unit);

	if (!wan_ip || strcmp(wan_ip, "") == 0 || !inet_addr(wan_ip)) {
		logmessage("ddns", "WAN IP is empty.");
		return -1;
	}

#if 0 //Move the ddns check mechanism to UI
	if(!nvram_match("ddns_update_by_wdog", "1")) {
		if ((inet_addr(wan_ip) == inet_addr(nvram_safe_get("ddns_ipaddr"))) &&
		    (strcmp(nvram_safe_get("ddns_server_x"), nvram_safe_get("ddns_server_x_old")) == 0) &&
		    (strcmp(nvram_safe_get("ddns_hostname_x"), nvram_safe_get("ddns_hostname_old")) == 0) &&
		    (nvram_match("ddns_updated", "1"))) {
			nvram_set("ddns_return_code", "no_change");
			logmessage("ddns", "IP address, server and hostname have not changed since the last update.");
			_dprintf("IP address, server and hostname have not changed since the last update.");
			return -1;
		}

		// TODO : Check /tmp/ddns.cache to see current IP in DDNS
		// update when,
		// 	1. if ipaddr!= ipaddr in cache
		//
		// update
		// * nvram ddns_cache, the same with /tmp/ddns.cache

		if (	(!nvram_match("ddns_server_x_old", "") &&
			strcmp(nvram_safe_get("ddns_server_x"), nvram_safe_get("ddns_server_x_old"))) ||
			(!nvram_match("ddns_hostname_x_old", "") &&
			strcmp(nvram_safe_get("ddns_hostname_x"), nvram_safe_get("ddns_hostname_x_old")))
		) {
			logmessage("ddns", "clear ddns cache file for server/hostname change");
			unlink("/tmp/ddns.cache");
		}
		else if (!(fp = fopen("/tmp/ddns.cache", "r")) && (ddns_cache = nvram_get("ddns_cache"))) {
			if ((fp = fopen("/tmp/ddns.cache", "w+"))) {
				fprintf(fp, "%s", ddns_cache);
				fclose(fp);
			}
		}
	}
	else
		nvram_unset("ddns_update_by_wdog");
#endif

	server = nvram_safe_get("ddns_server_x");
	user = nvram_safe_get("ddns_username_x");
	passwd = nvram_safe_get("ddns_passwd_x");
	host = nvram_safe_get("ddns_hostname_x");
	unlink("/tmp/ddns.cache");

	if (strcmp(server, "WWW.DYNDNS.ORG")==0)
		service = "dyndns";
	else if (strcmp(server, "WWW.DYNDNS.ORG(CUSTOM)")==0)
		service = "dyndns-custom";
	else if (strcmp(server, "WWW.DYNDNS.ORG(STATIC)")==0)
		service = "dyndns-static";
	else if (strcmp(server, "WWW.TZO.COM")==0)
		service = "tzo";
	else if (strcmp(server, "WWW.ZONEEDIT.COM")==0)
		service = "zoneedit";
	else if (strcmp(server, "WWW.JUSTLINUX.COM")==0)
		service = "justlinux";
	else if (strcmp(server, "WWW.EASYDNS.COM")==0)
		service = "easydns";
	else if (strcmp(server, "WWW.DNSOMATIC.COM")==0)
		service = "dnsomatic";
	else if (strcmp(server, "WWW.TUNNELBROKER.NET")==0) {
		service = "heipv6tb";
		eval("iptables", "-t", "filter", "-D", "INPUT", "-p", "icmp", "-s", "66.220.2.74", "-j", "ACCEPT");
		eval("iptables", "-t", "filter", "-I", "INPUT", "1", "-p", "icmp", "-s", "66.220.2.74", "-j", "ACCEPT");
		nvram_set("ddns_tunbkrnet", "1");
	}
	else if (strcmp(server, "WWW.NO-IP.COM")==0)
		service = "noip";
	else if (strcmp(server, "WWW.NAMECHEAP.COM")==0)
		service = "namecheap";
        else if (strcmp(server, "CUSTOM")==0)
                service = "";
	else if (strcmp(server, "WWW.SELFHOST.DE") == 0)
		service = "selfhost";
	else if (strcmp(server, "WWW.ASUS.COM")==0) {
		service = "dyndns", asus_ddns = 1;
	}
	else if (strcmp(server, "DOMAINS.GOOGLE.COM") == 0)
		service = "dyndns", asus_ddns=3;
	else if (strcmp(server, "WWW.ORAY.COM")==0) {
		service = "peanuthull", asus_ddns = 2;
	} else {
		logmessage("start_ddns", "Error ddns server name: %s\n", server);
		return 0;
	}

	snprintf(usrstr, sizeof(usrstr), "%s:%s", user, passwd);

	/* Show WAN unit used by ddns client to console and syslog. */
	_dprintf("start_ddns update %s %s, wan_unit %d\n", server, service, unit);
	logmessage("start_ddns", "update %s %s, wan_unit %d\n", server, service, unit);

	nvram_set("ddns_return_code", "ddns_query");

	if (pids("ez-ipupdate")) {
		killall("ez-ipupdate", SIGINT);
		sleep(1);
	}
	if (pids("phddns")) {
		killall("phddns", SIGINT);
		sleep(1);
	}

	nvram_unset("ddns_cache");
	nvram_unset("ddns_ipaddr");
	nvram_unset("ddns_status");
	nvram_unset("ddns_updated");

	_dprintf("asus_ddns : %d\n",asus_ddns);

	if(3 == asus_ddns)
	{
		if((time_fp=fopen("/tmp/ddns.cache","w")))
		{
			fprintf(time_fp,"%ld,%s",time(&now),wan_ip);
			fclose(time_fp);
		}
		eval("GoogleDNS_Update.sh", user, passwd, host, wan_ip);
	}
	else if (asus_ddns == 2) { //Peanuthull DDNS
		if( (fp = fopen("/etc/phddns.conf", "w")) != NULL ) {
			fprintf(fp, "[settings]\n");
			fprintf(fp, "szHost = phddns60.oray.net\n");
			fprintf(fp, "szUserID = %s\n", user);
			fprintf(fp, "szUserPWD = %s\n", passwd);
			fprintf(fp, "nicName = %s\n", wan_ifname);
			fprintf(fp, "szLog = /var/log/phddns.log\n");
			fclose(fp);

			eval("phddns", "-c", "/etc/phddns.conf", "-d");
		}
	}
	else if (asus_ddns == 1) {
		char *nserver = nvram_invmatch("ddns_serverhost_x", "") ?
			nvram_safe_get("ddns_serverhost_x") :
			"nwsrv-ns1.asus.com";
		char *argv[] = { "ez-ipupdate", "-S", service, "-i", wan_ifname,
				"-h", host, "-A", "2", "-s", nserver,
				"-e", "/sbin/ddns_updated", "-b", "/tmp/ddns.cache", NULL };
		_eval(argv, NULL, 0, &pid);
	} else if (*service) {
		char *argv[] = { "ez-ipupdate", "-S", service, "-i", wan_ifname, "-h", host,
		     "-u", usrstr, wild ? "-w" : "", "-e", "/sbin/ddns_updated",
		     "-b", "/tmp/ddns.cache", NULL };
		_eval(argv, NULL, 0, &pid);
	} else {	// Custom DDNS
		// Block until it completes and updates the DDNS update results in nvram
		run_custom_script_blocking("ddns-start", wan_ip);
		return 0;
	}

	run_custom_script("ddns-start", wan_ip);
	return 0;
}

void
stop_ddns(void)
{
	if (pids("ez-ipupdate"))
		killall("ez-ipupdate", SIGINT);
	if (pids("phddns"))
		killall("phddns", SIGINT);
	if (nvram_match("ddns_tunbkrnet", "1")) {
		eval("iptables-restore", "/tmp/filter_rules");
		nvram_unset("ddns_tunbkrnet");
	}
#ifdef RTCONFIG_OPENVPN
	update_ovpn_profie_remote();
#endif
}

int
asusddns_reg_domain(int reg)
{
	FILE *fp;
	char *wan_ip, *wan_ifname;
	char *ddns_cache;
	char *nserver;
	int unit;
	char tmp[32], prefix[] = "wanXXXXXXXXXX_";

	if (!is_routing_enabled()) {
		_dprintf("return -1\n");
		return -1;
	}

	if (reg) { //0:Aidisk, 1:Advanced Setting
		if (nvram_invmatch("ddns_enable_x", "1"))
			return -1;
	}

	unit = wan_primary_ifunit();
#if defined(RTCONFIG_DUALWAN)
	if (nvram_match("wans_mode", "lb")) {
		int ddns_wan_unit = nvram_get_int("ddns_wan_unit");

		if (ddns_wan_unit >= WAN_UNIT_FIRST && ddns_wan_unit < WAN_UNIT_MAX) {
			unit = ddns_wan_unit;
		} else {
			int u = get_first_configured_connected_wan_unit();
			if (u < WAN_UNIT_FIRST || u >= WAN_UNIT_MAX)
				return -2;

			unit = u;
		}
	}
#endif
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	wan_ip = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));
	wan_ifname = get_wan_ifname(unit);

	if (!wan_ip || strcmp(wan_ip, "") == 0 || !inet_addr(wan_ip)) {
		logmessage("asusddns", "WAN IP is empty.");
		return -1;
	}

	// TODO : Check /tmp/ddns.cache to see current IP in DDNS,
	// update when ipaddr!= ipaddr in cache.
	// nvram ddns_cache, the same with /tmp/ddns.cache

	if ((inet_addr(wan_ip) == inet_addr(nvram_safe_get("ddns_ipaddr"))) &&
		(strcmp(nvram_safe_get("ddns_server_x"), nvram_safe_get("ddns_server_x_old")) == 0) &&
		(strcmp(nvram_safe_get("ddns_hostname_x"), nvram_safe_get("ddns_hostname_old")) == 0)) {
		nvram_set("ddns_return_code", "no_change");
		logmessage("asusddns", "IP address, server and hostname have not changed since the last update.");
		return -1;
	}

	if (	(!nvram_match("ddns_server_x_old", "") &&
		strcmp(nvram_safe_get("ddns_server_x"), nvram_safe_get("ddns_server_x_old"))) ||
		(!nvram_match("ddns_hostname_x_old", "") &&
		strcmp(nvram_safe_get("ddns_hostname_x"), nvram_safe_get("ddns_hostname_x_old")))
	) {
		logmessage("asusddns", "clear ddns cache file for server/hostname change");
		unlink("/tmp/ddns.cache");
	}
	else if (!(fp = fopen("/tmp/ddns.cache", "r")) && (ddns_cache = nvram_get("ddns_cache"))) {
		if ((fp = fopen("/tmp/ddns.cache", "w+"))) {
			fprintf(fp, "%s", ddns_cache);
			fclose(fp);
		}
	}

	nvram_set("ddns_return_code", "ddns_query");

	if (pids("ez-ipupdate"))
	{
		killall("ez-ipupdate", SIGINT);
		sleep(1);
	}

	nserver = nvram_invmatch("ddns_serverhost_x", "") ?
		    nvram_safe_get("ddns_serverhost_x") :
		    "nwsrv-ns1.asus.com";

	eval("ez-ipupdate",
	     "-S", "dyndns", "-i", wan_ifname, "-h", nvram_safe_get("ddns_hostname_x"),
	     "-A", "1", "-s", nserver,
	     "-e", "/sbin/ddns_updated", "-b", "/tmp/ddns.cache");

	return 0;
}

void
stop_syslogd(void)
{
#if defined(RTCONFIG_JFFS2LOG) && (defined(RTCONFIG_JFFS2)||defined(RTCONFIG_BRCM_NAND_JFFS2))
	int running = pids("syslogd");
#endif

	killall_tk("syslogd");

#if defined(RTCONFIG_JFFS2LOG) && (defined(RTCONFIG_JFFS2)||defined(RTCONFIG_BRCM_NAND_JFFS2))
	if (running)
		eval("cp", "/tmp/syslog.log", "/tmp/syslog.log-1", "/jffs");
#endif
}

void
stop_klogd(void)
{
	killall_tk("klogd");
}

int
start_syslogd(void)
{
	int argc;
	char syslog_path[PATH_MAX];
	char syslog_addr[sizeof("255.255.255.255:65535")];
	char *syslogd_argv[] = {"/sbin/syslogd",
		"-m", "0",				/* disable marks */
		"-S",					/* small log */
//		"-D",					/* suppress dups */
		"-O", syslog_path,			/* /tmp/syslog.log or /jffs/syslog.log */
		NULL, NULL,				/* -s log_size */
		NULL, NULL,				/* -l log_level */
		NULL, NULL,				/* -R log_ipaddr[:port] */
		NULL,					/* -L log locally too */
		NULL
	};

	snprintf(syslog_path, sizeof(syslog_path), "%s", get_syslog_fname(0));
	for (argc = 0; syslogd_argv[argc]; argc++);

	if (nvram_invmatch("log_size", "")) {
		syslogd_argv[argc++] = "-s";
		syslogd_argv[argc++] = nvram_safe_get("log_size");
	}
	if (nvram_invmatch("log_level", "")) {
		syslogd_argv[argc++] = "-l";
		syslogd_argv[argc++] = nvram_safe_get("log_level");
	}
	if (nvram_invmatch("log_ipaddr", "")) {
		char *addr = nvram_safe_get("log_ipaddr");
		int port = nvram_get_int("log_port");

		if (port) {
			snprintf(syslog_addr, sizeof(syslog_addr), "%s:%d", addr, port);
			addr = syslog_addr;
		}
		syslogd_argv[argc++] = "-R";
		syslogd_argv[argc++] = addr;
		syslogd_argv[argc++] = "-L";
	}

//#if defined(RTCONFIG_JFFS2LOG) && defined(RTCONFIG_JFFS2)
#if defined(RTCONFIG_JFFS2LOG) && (defined(RTCONFIG_JFFS2)||defined(RTCONFIG_BRCM_NAND_JFFS2))
	eval("touch", "-c", "/jffs/syslog.log", "/jffs/syslog.log-1");
	eval("cp", "/jffs/syslog.log", "/jffs/syslog.log-1", "/tmp");
#endif

	// TODO: make sure is it necessary?
	//time_zone_x_mapping();
	//setenv("TZ", nvram_safe_get("time_zone_x"), 1);

	return _eval(syslogd_argv, NULL, 0, NULL);
}

int
start_klogd(void)
{
	int argc;
	char *klogd_argv[] = {"/sbin/klogd",
		NULL, NULL,				/* -c console_loglevel */
		NULL
	};

	for (argc = 0; klogd_argv[argc]; argc++);

	if (nvram_invmatch("console_loglevel", "")) {
		klogd_argv[argc++] = "-c";
		klogd_argv[argc++] = nvram_safe_get("console_loglevel");
	}

	return _eval(klogd_argv, NULL, 0, NULL);
}

#ifdef HND_ROUTER
extern int dump_prev_oops(void);
#endif

int
start_logger(void)
{
_dprintf("%s:\n", __FUNCTION__);
	start_syslogd();
	start_klogd();

#if defined(DUMP_PREV_OOPS_MSG) && defined(RTCONFIG_BCMARM)
#if defined(HND_ROUTER) && defined(RTCONFIG_BRCM_NAND_JFFS2)
	if (f_exists("/jffs/oops")) {
		dump_prev_oops();
		unlink("/jffs/oops");
	}
#else
#if defined(RTCONFIG_BCM_7114) || defined(RTCONFIG_BCM9)
	eval("et", "dump", "oops");
#else
	eval("et", "dump_oops");
#endif
#endif
#endif

	return 0;
}

#ifdef RTCONFIG_BCMWL6
int
wl_igs_enabled(void)
{
	int i;
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX";
	char word[256], *next, ifnames[128];

	i = 0;
	strcpy(ifnames, nvram_safe_get("wl_ifnames"));
	foreach (word, ifnames, next) {
		if (i >= MAX_NR_WL_IF)
			break;

		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		if (nvram_match(strcat_r(prefix, "radio", tmp), "1") &&
		    nvram_match(strcat_r(prefix, "igs", tmp), "1"))
			return 1;

		i++;
	}

	return 0;
}

void
start_igmp_proxy(void)
{
	/* Start IGMP Proxy in Router mode only */
#if 0
	if (sw_mode() == SW_MODE_ROUTER)
		eval("igmp", nvram_get("wan_ifname"));
	else
#endif
	if (sw_mode() == SW_MODE_AP)
	{
#if defined(HND_ROUTER) && defined(MCPD_PROXY)
		start_mcpd_proxy();
#else
		if (nvram_get_int("emf_enable") || wl_igs_enabled()) {
			/* Start IGMP proxy in AP mode */
			eval("igmp", nvram_get("lan_ifname"));
		}
#endif
	}
}

void
stop_igmp_proxy(void)
{
	if (sw_mode() == SW_MODE_AP)
	{
#if defined(HND_ROUTER) && defined(MCPD_PROXY)
		stop_mcpd_proxy();
#else
		killall_tk("igmp");
#endif
	}
}

#if defined(BCA_HNDROUTER) && defined(MCPD_PROXY)
static void
mcpd_conf(void)
{
	FILE *fp;
	char *conf_file = "/var/mcpd.conf";
	char *proxy_ifname = NULL;

	/* Start MCPD Proxy in router mode */
	if (is_router_mode()) {
		proxy_ifname = nvram_get("igmp_ifname") ? : "eth0";
	} else {
#if 0
		if (!nvram_match("igmp_enable", "0"))
#endif
		{
			/* Start MCPD proxy in AP mode for media router build */
			proxy_ifname = nvram_get("lan_ifname") ? : "br0";
		}
	}

	if (!(fp = fopen(conf_file, "w+"))) {
		perror(conf_file);
	}

	if (!fp || !proxy_ifname)
		return;

	/* set mcast interface to eth0.v0 when using vlanctl net interface(IPTV) 
	   to enable mcpd bring up							*/
	if (!nvram_match("switch_wantag", "") && nvram_get_int("switch_stb_x") > 0 &&
		nvram_get_int("switch_stb_x") <= 6) {
		proxy_ifname = "eth0.v0";
	}

	/* IGMP configuration */
	fprintf(fp, "##### IGMP configuration #####\n");
	fprintf(fp, "igmp-default-version 3\n");
	fprintf(fp, "igmp-query-interval 20\n");
	fprintf(fp, "igmp-query-response-interval 100\n");
	fprintf(fp, "igmp-last-member-query-interval 10\n");
	fprintf(fp, "igmp-robustness-value 2\n");
	fprintf(fp, "igmp-max-groups 25\n");
	fprintf(fp, "igmp-max-sources 25\n");
	fprintf(fp, "igmp-max-members 25\n");
#ifdef RTCONFIG_MULTICAST_IPTV
	if (!(nvram_get_int("switch_stb_x") > 6 &&
		nvram_match("switch_wantag", "movistar")))
		fprintf(fp, "igmp-fast-leave 1\n");
#else
	fprintf(fp, "igmp-fast-leave 1\n");
#endif
	fprintf(fp, "igmp-admission-required 0\n");
	fprintf(fp, "igmp-admission-bridging-filter 0\n");
	/* Do not set igmp proxy in IPTV bridge case */
	if (nvram_get_int("switch_stb_x") == 0 || nvram_get_int("switch_stb_x") > 6) {
		if(nvram_match("switch_wantag", "movistar"))
			fprintf(fp, "igmp-proxy-interfaces %s\n", "vlan2");
		else
			fprintf(fp, "igmp-proxy-interfaces %s\n", proxy_ifname);
	}
	/* set downstream interface to vlan bridge to enable mcast traffic passthrough */
	if (!nvram_match("switch_wantag", "") && nvram_get_int("switch_stb_x") > 0 &&
		nvram_get_int("switch_stb_x") <= 6) {
		fprintf(fp, "igmp-snooping-interfaces %s\n", "br1");
	}
	else
		fprintf(fp, "igmp-snooping-interfaces %s\n", nvram_safe_get("lan_ifname"));
	if(nvram_match("switch_wantag", "movistar"))
		fprintf(fp, "igmp-mcast-interfaces %s\n", "vlan2");
	else
		fprintf(fp, "igmp-mcast-interfaces %s\n", proxy_ifname);

	/* Mcast configuration */
	fprintf(fp, "\n##### MCAST configuration #####\n");
	fprintf(fp, "igmp-mcast-snoop-exceptions "
		"239.255.255.250/255.255.255.255 "
		"224.0.255.135/255.255.255.255\n");
	fprintf(fp, "mld-mcast-snoop-exceptions "
		"ff05::0001:0003/ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff\n");

	append_custom_config("mcpd.conf", fp);
	if (fp)
		fclose(fp);

        use_custom_config("mcpd.conf",conf_file);
        run_postconf("mcpd",conf_file);

}

void
start_mcpd_proxy(void)
{
	int lock = file_lock("mcpd");
	char *lan_ifname = nvram_safe_get("lan_ifname");

	stop_mcpd_proxy();

	/* Create mcpd.conf */
	mcpd_conf();

	/* Run mcpd */
	system("/bin/mcpd &");

	/* Enable LAN-to-LAN snooping in Router mode */
	if (is_router_mode()) {
		eval("/bin/bmc", "l2l", "-e", "1", "-p", "1", "-i", lan_ifname);

		eval("/bin/bmc", "l2l", "-e", "1", "-p", "2", "-i", lan_ifname);
	}

	file_unlock(lock);

	_dprintf("done\n");
	return;
}

void
stop_mcpd_proxy(void)
{
	if (pids("mcpd"))
	killall_tk("mcpd");
}

#if 0
void
restart_mcpd_proxy(void)
{
	/* stop mcpd */
	stop_mcpd_proxy();

	mcpd_conf();

	/* Run mcpd */
	system("/bin/mcpd &");

	_dprintf("done\n");
	return;
}
#endif
#endif /* BCA_HNDROUTER && MCPD_PROXY */

void
stop_acsd(void)
{
	killall_tk("acsd");
}

int
start_acsd()
{
	int ret = 0;

#ifdef RTCONFIG_PROXYSTA
	if (psta_exist()
		|| dpsr_mode()
#ifdef RTCONFIG_DPSTA
		|| dpsta_mode()
#endif
	)
		return 0;
#endif

	stop_acsd();

	if (!restore_defaults_g && strlen(nvram_safe_get("acs_ifnames")))
		ret = eval("/usr/sbin/acsd");

	return ret;
}
#endif

#ifdef BLUECAVE
static void
mcast_conf(void)
{
	FILE *fp;
	char *conf_file = "/tmp/mcast.conf";
	char *proxy_ifname = NULL;

	/* Start MCASTD Proxy in router mode */
	if (is_router_mode()) {
		proxy_ifname = nvram_get("igmp_ifname") ? : "eth1";
	} else {
#if 0
		if (!nvram_match("igmp_enable", "0"))
		{
			/* Start MCASTD proxy in AP mode for media router build */
			proxy_ifname = nvram_get("lan_ifname") ? : "br0";
		}
#endif
	}

	if (!(fp = fopen(conf_file, "w+"))) {
		perror(conf_file);
	}

	if (!fp || !proxy_ifname)
		return;

	/* IGMP configuration */
	fprintf(fp, "mcast_grp_entries=\"0\"\n");
	fprintf(fp, "mcast_upstream=\"%s\"\n", proxy_ifname);
	fprintf(fp, "mcast_upstream_wan=\"%s\"\n", proxy_ifname);
	fprintf(fp, "mcast_downstream=\"%s\"\n", nvram_safe_get("lan_ifname"));
	fprintf(fp, "mcast_igmp_query_interval=\"125\"\n");
	fprintf(fp, "mcast_igmp_query_resp_interval=\"10\"\n");
	fprintf(fp, "mcast_igmp_last_mem_query_interval=\"2\"\n");
	fprintf(fp, "mcast_igmp_last_mem_query_count=\"2\"\n");
	fprintf(fp, "mcast_igmp_query_interval_status=\"1\"\n");
	fprintf(fp, "mcast_igmp_query_resp_interval_status=\"1\"\n");
	fprintf(fp, "mcast_igmp_last_mem_query_interval_status=\"1\"\n");
	fprintf(fp, "mcast_igmp_last_mem_query_count_status=\"1\"\n");
	fprintf(fp, "mcast_igmp_fast_leave_status=\"1\"\n");
	fprintf(fp, "mcast_igmp_snooping_status=\"1\"\n");
	fprintf(fp, "mcast_igmp_proxy_status=\"1\"\n");
	fprintf(fp, "mcast_igmp_snooping_modee=\"2\"\n");
	fprintf(fp, "mcast_mld_query_interval=\"125\"\n");
	fprintf(fp, "mcast_mld_query_resp_interval=\"10\"\n");
	fprintf(fp, "mcast_mld_last_mem_query_interval=\"5\"\n");
	fprintf(fp, "mcast_mld_last_mem_query_count=\"2\"\n");
	fprintf(fp, "mcast_mld_query_interval_status=\"1\"\n");
	fprintf(fp, "mcast_mld_query_resp_interval_status=\"1\"\n");
	fprintf(fp, "mcast_mld_last_mem_query_interval_status=\"1\"\n");
	fprintf(fp, "mcast_mld_last_mem_query_count_status=\"1\"\n");
	fprintf(fp, "mcast_mld_fast_leave_status=\"1\"\n");
	fprintf(fp, "mcast_mld_snooping_status=\"1\"\n");
	fprintf(fp, "mcast_mld_proxy_status=\"1\"\n");
	fprintf(fp, "mcast_mld_snooping_mode=\"2\"\n");

	if (fp)
		fclose(fp);
}

void
start_mcast_proxy(void)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");

	stop_mcast_proxy();

	/* Create mcast.conf */
	mcast_conf();

	/* Run mcast */
	system("/opt/lantiq/usr/sbin/mcastd -b -c /tmp/mcast.conf");
#if 0
	/* Enable LAN-to-LAN snooping in Router mode */
	if (is_router_mode()) {
	}
#endif

	_dprintf("done\n");
	return;
}

void
stop_mcast_proxy(void)
{
	killall_tk("mcastd");
}
#endif

#ifdef RTCONFIG_SYSSTATE
void stop_sysstate(void)
{
	if (pids("sysstate"))
		killall_tk("sysstate");
}

void start_sysstate(void)
{
	stop_sysstate();
	xstart("sysstate");
}
#endif

void
stop_misc(void)
{
	fprintf(stderr, "stop_misc()\n");

	if (pids("infosvr"))
		killall_tk("infosvr");

#ifdef RTCONFIG_SMALL_FW_UPDATE
	if (pids("watchdog02"))
		killall_tk("watchdog02");
#endif
#ifdef RTCONFIG_LANTIQ
	if (pids("wave_monitor"))
		killall_tk("wave_monitor");
#endif
#ifdef SW_DEVLED
	if (pids("sw_devled"))
		killall_tk("sw_devled");
#endif
#if defined(RTAC1200G) || defined(RTAC1200GP)
	if (pids("wdg_monitor"))
		killall_tk("wdg_monitor");
#endif
	if (pids("watchdog")
#if defined(RTAC68U) || defined(RTCONFIG_FORCE_AUTO_UPGRADE)
		&& !nvram_get_int("auto_upgrade")
#endif
	)
	{
#ifdef RTL_WTDOG
		stop_rtl_watchdog();
#endif
		killall_tk("watchdog");
	}

#ifdef RTCONFIG_FANCTRL
	if (pids("phy_tempsense"))
		killall_tk("phy_tempsense");
#endif
#ifdef RTCONFIG_BCMWL6
#ifdef RTCONFIG_PROXYSTA
	if (pids("psta_monitor"))
		killall_tk("psta_monitor");
#endif
#ifdef RTCONFIG_IPERF
	if (pids("monitor"))
		killall_tk("monitor");
#endif
#endif
#ifdef RTCONFIG_QTN
	if (pids("qtn_monitor"))
		killall_tk("qtn_monitor");
#endif
	if (pids("ntp"))
		killall_tk("ntp");
	if (pids("ntpclient"))
		killall_tk("ntpclient");

#ifdef RTCONFIG_BCMWL6
#ifdef BCM_ASPMD
	stop_aspmd();
#endif
#ifdef RTCONFIG_DHDAP
	stop_dhd_monitor();
#endif
	stop_acsd();
#ifdef BCM_EVENTD
	stop_eventd();
#endif
#ifdef BCM_SSD
	stop_ssd();
#endif
#ifdef BCM_APPEVENTD
	stop_appeventd();
#endif
#ifdef BCM_BSD
	stop_bsd();
#endif
	stop_igmp_proxy();
#ifdef RTCONFIG_HSPOT
	stop_hspotap();
#endif
#ifdef RTCONFIG_AMAS
        stop_obd();
#endif
#endif
#if defined(RTCONFIG_WLCEVENTD)
	stop_wlceventd();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
	stop_wlc_nt();
#endif
	stop_wps();
	stop_upnp();
	stop_lltd();
	stop_snooper();
	stop_rstats();
#if !defined(HND_ROUTER)
	stop_cstats();
#endif
#ifdef RTCONFIG_DSL
	stop_spectrum(); //Ren
#endif //For DSL-N55U
#ifdef RTCONFIG_JFFS2USERICON
	stop_lltdc();
#endif
	stop_networkmap();

#ifdef RTCONFIG_NEW_USER_LOW_RSSI
	stop_roamast();
#endif
}

void
stop_misc_no_watchdog(void)
{
	_dprintf("done\n");
}

int
chpass(char *user, char *pass)
{
//	FILE *fp;

	if (!user)
		user = "admin";

	if (!pass)
		pass = "admin";
/*
	if ((fp = fopen("/etc/passwd", "a")))
	{
		fprintf(fp, "%s::0:0::/:\n", user);
		fclose(fp);
	}

	if ((fp = fopen("/etc/group", "a")))
	{
		fprintf(fp, "%s:x:0:%s\n", user, user);
		fclose(fp);
	}
*/
	eval("chpasswd.sh", user, pass);
	return 0;
}

void
set_hostname(void)
{
	FILE *fp;
	char hostname[32];

	strncpy(hostname, nvram_safe_get("computer_name"), 31);

	if (*hostname == 0) {
		if (get_productid()) {
			strncpy(hostname, get_productid(), 31);
		}
	}

	if ((fp=fopen("/proc/sys/kernel/hostname", "w+"))) {
		fputs(hostname, fp);
		fclose(fp);
	}
}

int
_start_telnetd(int force)
{
	char *telnetd_argv[] = { "telnetd",
		NULL, NULL,	/* -b address */
#if defined(HND_ROUTER) && defined(RTCONFIG_HNDMFG)
		NULL, NULL,	/* -l shell */
#endif
		NULL };
	int index = 1;

	if (!force) {
		if (!nvram_get_int("telnetd_enable"))
			return 0;

		if (getpid() != 1) {
			notify_rc("start_telnetd");
			return 0;
		}
	}

	if (pids("telnetd"))
		killall_tk("telnetd");

	setup_passwd();
	//chpass(nvram_safe_get("http_username"), nvram_safe_get("http_passwd"));	// vsftpd also needs

	if (is_routing_enabled()) {
		telnetd_argv[index++] = "-b";
		telnetd_argv[index++] = nvram_safe_get("lan_ipaddr");
	}

#if defined(HND_ROUTER) && defined(RTCONFIG_HNDMFG)
	telnetd_argv[index++] = "-l";
	telnetd_argv[index++] = "/bin/sh";
#endif

	return _eval(telnetd_argv, NULL, 0, NULL);
}

int
start_telnetd(void)
{
	return _start_telnetd(0);
}

void
stop_telnetd(void)
{
	if (getpid() != 1) {
		notify_rc("stop_telnetd");
		return;
	}

	if (pids("telnetd"))
		killall_tk("telnetd");
}

void
start_httpd(void)
{
	char *httpd_argv[] = { "httpd",
		NULL, NULL,	/* -i ifname */
		NULL, NULL,	/* -p port */
		NULL };
	int httpd_index = 1;
#ifdef RTCONFIG_HTTPS
	char *https_argv[] = { "httpds", "-s",
		NULL, NULL,	/* -i ifname */
		NULL, NULL,	/* -p port */
		NULL };
	int https_index = 2;
	int enable;
#endif
	char *cur_dir;
	pid_t pid;
#ifdef DEBUG_RCTEST
	char *httpd_dir;
#endif

	if (getpid() != 1) {
		notify_rc("start_httpd");
		return;
	}

	cur_dir = getcwd(NULL, 0);
#ifdef DEBUG_RCTEST // Left for UI debug
	httpd_dir = nvram_safe_get("httpd_dir");
	if(strlen(httpd_dir)) chdir(httpd_dir);
	else
#endif
	chdir("/www");

	if (is_routing_enabled()) {
		httpd_argv[httpd_index++] = "-i";
		httpd_argv[httpd_index++] = nvram_safe_get("lan_ifname");
#ifdef RTCONFIG_HTTPS
		https_argv[https_index++] = "-i";
		https_argv[https_index++] = nvram_safe_get("lan_ifname");
#endif
	}

#ifdef RTCONFIG_HTTPS
#ifdef RTCONFIG_LETSENCRYPT
	if(nvram_match("le_enable", "1")) {
		if(!is_le_cert(HTTPD_CERT) || !cert_key_match(HTTPD_CERT, HTTPD_KEY)) {
			cp_le_cert(LE_FULLCHAIN, HTTPD_CERT);
			cp_le_cert(LE_KEY, HTTPD_KEY);
		}
	}
	else if(nvram_match("le_enable", "2")) {
                unlink(HTTPD_CERT);
                unlink(HTTPD_KEY);
                if(f_exists(UPLOAD_CERT) && f_exists(UPLOAD_KEY)) {
                        eval("cp", UPLOAD_CERT, HTTPD_CERT);
                        eval("cp", UPLOAD_KEY, HTTPD_KEY);
		}
	}
#else
	if(f_exists(UPLOAD_CERT) && f_exists(UPLOAD_KEY)){
		eval("cp", UPLOAD_CERT, HTTPD_CERT);
		eval("cp", UPLOAD_KEY, HTTPD_KEY);
	}
#endif

	enable = nvram_get_int("http_enable");
	if (enable != 0) {
		pid = nvram_get_int("https_lanport") ? : 443;
		if (pid != 443) {
			https_argv[https_index++] = "-p";
			https_argv[https_index++] = nvram_safe_get("https_lanport");
		}
		logmessage(LOGNAME, "start https:%d", pid);
		_eval(https_argv, NULL, 0, &pid);
#if defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
		sleep(1);
#endif
	}
#ifndef RTCONFIG_AIHOME_TUNNEL
	if (enable != 1)
#endif
#endif
	{
		pid = nvram_get_int("http_lanport") ? : 80;
		if (pid != 80) {
			httpd_argv[httpd_index++] = "-p";
			httpd_argv[httpd_index++] = nvram_safe_get("http_lanport");
		}
		logmessage(LOGNAME, "start httpd:%d", pid);
		_eval(httpd_argv, NULL, 0, &pid);
#if defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
		sleep(1);
#endif
	}

	chdir(cur_dir ? : "/");
	free(cur_dir);
}

void
stop_httpd(void)
{
	if (getpid() != 1) {
		notify_rc("stop_httpd");
		return;
	}

	if (pids("httpd"))
		killall_tk("httpd");
#ifdef RTCONFIG_HTTPS
	if (pids("httpds"))
		killall_tk("httpds");
#endif
}

//////////vvvvvvvvvvvvvvvvvvvvvjerry5 2009.07
void
stop_rstats(void)
{
	if (pids("rstats"))
		killall_tk("rstats");
}

void
start_rstats(int new)
{
	if (!is_routing_enabled()) return;

	stop_rstats();
	if (new) xstart("rstats", "--new");
	else xstart("rstats");
}

void
restart_rstats()
{
	if (nvram_match("rstats_new", "1"))
	{
		start_rstats(1);
		nvram_set("rstats_new", "0");
		nvram_commit();		// Otherwise it doesn't get written back to mtd
	}
	else
	{
		start_rstats(0);
	}
}
////////^^^^^^^^^^^^^^^^^^^jerry5 2009.07

//Ren.B
#ifdef RTCONFIG_DSL
void stop_spectrum(void)
{
	if (getpid() != 1) {
		notify_rc("stop_spectrum");
		return;
	}

	if (pids("spectrum"))
		killall_tk("spectrum");
}

void start_spectrum(void)
{
	if (getpid() != 1) {
		notify_rc("start_spectrum");
		return;
	}

	stop_spectrum();
	xstart("spectrum");
}
#endif
//Ren.E

// TODO: so far, support wan0 only

void start_upnp(void)
{
	FILE *f;
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	char et0macaddr[18];
	char *proto, *port, *lport, *srcip, *dstip, *desc;
	char *nv, *nvp, *b;
	int upnp_enable, upnp_mnp_enable, upnp_port;
	int unit, i, httpx_port, cnt;
#if defined(RTCONFIG_APP_PREINSTALLED) || defined(RTCONFIG_APP_NETINSTALLED)
	FILE *ifp = NULL;
	char tmpstr[80];
	int statDownloadMaster = 0;
#endif
	int min_lifetime, max_lifetime;

	if (getpid() != 1) {
		notify_rc("start_upnp");
		return;
	}

	if (!is_routing_enabled())
		return;

	unit = wan_primary_ifunit();
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	upnp_enable = nvram_get_int("upnp_enable");
	upnp_mnp_enable = nvram_get_int("upnp_mnp");

	if (nvram_get_int(strcat_r(prefix, "upnp_enable", tmp))) {
		mkdir("/etc/upnp", 0777);
		if (f_exists("/etc/upnp/config.alt")) {
			xstart("miniupnpd", "-f", "/etc/upnp/config.alt");
		} else {
			if ((f = fopen("/etc/upnp/config", "w")) != NULL) {
				char *lanip = nvram_safe_get("lan_ipaddr");
				char *lanmask = nvram_safe_get("lan_netmask");

				upnp_port = nvram_get_int("upnp_port");
				if (upnp_port < 0 || upnp_port > 65535)
					upnp_port = 0;

				strlcpy(et0macaddr, get_lan_hwaddr(), sizeof (et0macaddr));

				if (strlen(et0macaddr))
					for (i = 0; i < strlen(et0macaddr); i++)
						et0macaddr[i] = tolower(et0macaddr[i]);;

				fprintf(f,
					"ext_ifname=%s\n"
					"listening_ip=%s\n"
					"port=%d\n"
					"enable_upnp=%s\n"
					"enable_natpmp=%s\n"
					"secure_mode=%s\n"
					"upnp_nat_postrouting_chain=PUPNP\n"
					"upnp_forward_chain=FUPNP\n"
					"upnp_nat_chain=VUPNP\n"
					"notify_interval=%d\n"
					"system_uptime=yes\n"
					"friendly_name=%s\n"
					"model_name=%s\n"
					"model_description=%s\n"
					"model_number=%s\n"
					"serial=%s\n"
					"lease_file=%s\n",
					get_wan_ifname(wan_primary_ifunit()),
					nvram_safe_get("lan_ifname"),
					upnp_port,
					upnp_enable ? "yes" : "no",	// upnp enable
					upnp_mnp_enable ? "yes" : "no",	// natpmp enable
					nvram_get_int("upnp_secure") ? "yes" : "no",	// secure_mode (only forward to self)
					nvram_get_int("upnp_ssdp_interval"),
					get_productid(),
					get_productid(),
					"ASUS Wireless Router",
					rt_serialno,
					nvram_get("serial_no") ? : et0macaddr,
					"/tmp/upnp.leases");

				if (nvram_get_int("upnp_clean")) {
					int interval = nvram_get_int("upnp_clean_interval");
					if (interval < 60)
						interval = 60;
					fprintf(f,
						"clean_ruleset_interval=%d\n"
						"clean_ruleset_threshold=%d\n",
						interval,
						nvram_get_int("upnp_clean_threshold"));
				} else
					fprintf(f,"clean_ruleset_interval=%d\n", 0);


				// Empty parameters are not included into XML service description
				fprintf(f, "presentation_url=");
#ifdef RTCONFIG_HTTPS
				if (nvram_get_int("http_enable") == 1) {
					fprintf(f, "%s://%s:%d/\n", "https", lanip, nvram_get_int("https_lanport") ? : 443);
				} else
#endif
				{
					fprintf(f, "%s://%s:%d/\n", "http", lanip, nvram_get_int("http_lanport") ? : 80);
				}

				char uuid[45];
				f_read_string("/proc/sys/kernel/random/uuid", uuid, sizeof(uuid));
				fprintf(f, "uuid=%s\n", uuid);

				if (is_nat_enabled() && nvram_match("vts_enable_x", "1")) {
					nvp = nv = strdup(nvram_safe_get("vts_rulelist"));
					while (nv && (b = strsep(&nvp, "<")) != NULL) {
						char *portv, *portp, *c;

						if ((cnt = vstrsep(b, ">", &desc, &port, &dstip, &lport, &proto, &srcip)) < 5)
							continue;
						else if (cnt < 6)
							srcip = "";

						// Handle port1,port2,port3 format
						portp = portv = strdup(port);
						while (portv && (c = strsep(&portp, ",")) != NULL) {
							if (strcmp(proto, "TCP") == 0 || strcmp(proto, "BOTH") == 0) {
								fprintf(f, "deny %s 0.0.0.0/0 0-65535\n", c);

								int local_ftpport = nvram_get_int("vts_ftpport");
								if (!strcmp(c, "21") && local_ftpport != 0 && local_ftpport != 21)
									fprintf(f, "deny %d 0.0.0.0/0 0-65535\n", local_ftpport);
							}
							if (strcmp(proto, "UDP") == 0 || strcmp(proto, "BOTH") == 0)
								fprintf(f, "deny %s 0.0.0.0/0 0-65535\n", c);
						}
						free(portv);
					}
					free(nv);
				}

				if (nvram_get_int("misc_http_x")) {
#ifdef RTCONFIG_HTTPS
					int enable = nvram_get_int("http_enable");
					if (enable != 0) {
						httpx_port = nvram_get_int("misc_httpsport_x") ? : 8443;
						fprintf(f, "deny %d 0.0.0.0/0 0-65535\n", httpx_port);
					}
					if (enable != 1)
#endif
					{
						httpx_port = nvram_get_int("misc_httpport_x") ? : 8080;
						fprintf(f, "deny %d 0.0.0.0/0 0-65535\n", httpx_port);
					}
				}

#ifdef RTCONFIG_WEBDAV
				if (nvram_match("enable_webdav", "1") && nvram_match("webdav_aidisk", "1")) {
					httpx_port = nvram_get_int("webdav_https_port");
					if (!httpx_port || httpx_port > 65535)
						httpx_port = 443;
					fprintf(f, "deny %d 0.0.0.0/0 0-65535\n", httpx_port);

					httpx_port = nvram_get_int("webdav_http_port");
					if (!httpx_port || httpx_port > 65535)
						httpx_port = 8082;
					fprintf(f, "deny %d 0.0.0.0/0 0-65535\n", httpx_port);
				}
#endif

#if defined(RTCONFIG_APP_PREINSTALLED) || defined(RTCONFIG_APP_NETINSTALLED)
				ifp = fopen("/opt/lib/ipkg/status", "r");
				if (ifp) {
					while (fgets(tmpstr, 80, ifp)) {
						if (strstr(tmpstr, "downloadmaster")) {
							statDownloadMaster = 1; //installed
							break;
						}
					}
					fclose(ifp);
				}

				if (statDownloadMaster == 1) {
					ifp = fopen("/opt/lib/ipkg/info/downloadmaster.control", "r");
					if (ifp) {
						while (fgets(tmpstr, 80, ifp)) {
							if (strstr(tmpstr, "Enabled") && strstr(tmpstr, "yes")) {
								statDownloadMaster = 2; //installed and enabled
								break;
							}
						}
						fclose(ifp);
					}
				}

				if (statDownloadMaster == 2) {
					// Transmisson
					fprintf(f, "deny %d 0.0.0.0/0 0-65535\n", nvram_get_int("trs_peer_port"));
					// amuled
					fprintf(f, "deny %d 0.0.0.0/0 0-65535\n", 4662);
					fprintf(f, "deny %d 0.0.0.0/0 0-65535\n", 4665);
					fprintf(f, "deny %d 0.0.0.0/0 0-65535\n", 4672);
					// lighttpd
					if (nvram_match("gen_http_x", "1"))
						fprintf(f, "deny %d 0.0.0.0/0 0-65535\n", nvram_get_int("dm_http_port"));
				}
#endif

				int ports[4];
				if ((ports[0] = nvram_get_int("upnp_min_port_ext")) > 0 &&
				    (ports[1] = nvram_get_int("upnp_max_port_ext")) > 0 &&
				    (ports[2] = nvram_get_int("upnp_min_port_int")) > 0 &&
				    (ports[3] = nvram_get_int("upnp_max_port_int")) > 0) {
					fprintf(f,
						"allow %d-%d %s/%s %d-%d\n",
						ports[0], ports[1],
						lanip, lanmask,
						ports[2], ports[3]
					);
				}
				else {
					// by default allow only redirection of ports above 1024
					fprintf(f, "allow 1024-65535 %s/%s 1024-65535\n", lanip, lanmask);
				}

				/* For PCP */
				min_lifetime = nvram_get_int("upnp_min_lifetime");
				max_lifetime = nvram_get_int("upnp_max_lifetime");

				fprintf(f, "min_lifetime=%d\n"
					   "max_lifetime=%d\n",
					   (min_lifetime > 0 ? min_lifetime : 120),
					   (max_lifetime > 0 ? max_lifetime : 86400));

				fprintf(f, "\ndeny 0-65535 0.0.0.0/0 0-65535\n");

				fappend(f, "/etc/upnp/config.custom");
				append_custom_config("upnp", f);

				fclose(f);
				use_custom_config("upnp", "/etc/upnp/config");
				run_postconf("upnp", "/etc/upnp/config");
				xstart("miniupnpd", "-f", "/etc/upnp/config");
			}
		}
	}
}

void stop_upnp(void)
{
	if (getpid() != 1) {
		notify_rc("stop_upnp");
		return;
	}

	killall_tk("miniupnpd");
}

void reload_upnp(void)
{
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	int unit;

	if (!is_routing_enabled())
		return;

	unit = wan_primary_ifunit();
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	if (nvram_get_int(strcat_r(prefix, "upnp_enable", tmp)))
		killall("miniupnpd", SIGUSR1);
}

int
start_ntpc(void)
{
	char *ntp_argv[] = {"ntp", NULL};
	int pid;

	if (pids("ntpclient"))
		killall_tk("ntpclient");

	if (!pids("ntp"))
		_eval(ntp_argv, NULL, 0, &pid);

	return 0;
}

void
stop_ntpc(void)
{
	if (pids("ntpclient"))
		killall_tk("ntpclient");
}


void refresh_ntpc(void)
{
	setup_timezone();

	if (pids("ntpclient"))
		killall_tk("ntpclient");

	if (!pids("ntp"))
	{
		stop_ntpc();
		start_ntpc();
	}
	else
		kill_pidfile_s("/var/run/ntp.pid", SIGALRM);
}

#ifdef RTCONFIG_BCMARM
int write_lltd_conf(void)
{
	FILE *fp;

	if (!(fp = fopen("/tmp/lld2d.conf", "w"))) {
		perror("/tmp/lld2d.conf");
		return -1;
	}

	fprintf(fp, "icon = /usr/sbin/icon.ico\n");
	fprintf(fp, "jumbo-icon = /usr/sbin/icon.large.ico\n");

	fclose(fp);

#ifdef RTAC68U
	doSystem("cp /usr/sbin/lltd/icon%s.ico /tmp/icon.ico", is_ac66u_v2_series() ? "_alt" : "");
	doSystem("cp /usr/sbin/lltd/icon%S.large.ico /tmp/icon.large.ico", is_ac66u_v2_series() ? "_alt" : "");
#endif

	return 0;
}
#endif

int start_lltd(void)
{
	chdir("/usr/sbin");

#if defined(CONFIG_BCMWL5) && !defined(RTCONFIG_BCMARM)
	char *odmpid = nvram_safe_get("odmpid");
	int model = get_model();

	if (strlen(odmpid) && is_valid_hostname(odmpid))
	{
		switch (model) {
		case MODEL_RTN66U:
			eval("lld2d.rtn66r", "br0");
			break;
		case MODEL_RTAC66U:
			if (!strcmp(odmpid, "RT-AC66R"))
				eval("lld2d.rtac66r", "br0");
			else if (!strcmp(odmpid, "RT-AC66W"))
				eval("lld2d.rtac66w", "br0");
			else if (!strcmp(odmpid, "RT-AC1750"))
				eval("lld2d.rtac1750", "br0");
			break;
		default:
			eval("lld2d", "br0");
			break;
		}
	}
	else
#endif
	{
#ifdef RTCONFIG_BCMARM
		write_lltd_conf();
		nvram_set("lld2d_hostname", get_productid());
#endif
		eval("lld2d", "br0");
	}

	chdir("/");

	return 0;
}

void stop_lltd(void)
{
#if defined(CONFIG_BCMWL5) && !defined(RTCONFIG_BCMARM)
	char *odmpid = nvram_safe_get("odmpid");
	int model = get_model();

	if (strlen(odmpid) && is_valid_hostname(odmpid))
	{
		switch (model) {
		case MODEL_RTN66U:
			kill_pidfile_s("/var/run/lld2d.rtn66r-br0.pid", SIGTERM);
			break;
		case MODEL_RTAC66U:
			kill_pidfile_s("/var/run/lld2d.rtac66r-br0.pid", SIGTERM);
			break;
		default:
			killall_tk("lld2d");
			break;
		}
	}
	else
#endif
	killall_tk("lld2d");
}

#if defined(RTCONFIG_MDNS)

#define AVAHI_CONFIG_PATH	"/tmp/avahi"
#define AVAHI_SERVICES_PATH	"/tmp/avahi/services"
#define AVAHI_CONFIG_FN		"avahi-daemon.conf"
#define AVAHI_AFPD_SERVICE_FN	"afpd.service"
#define AVAHI_ADISK_SERVICE_FN	"adisk.service"
#define AVAHI_ITUNE_SERVICE_FN  "mt-daap.service"
#define TIMEMACHINE_BACKUP_NAME	"Backups.backupdb"

int generate_mdns_config(void)
{
	FILE *fp;
	char avahi_config[80];
	char et0macaddr[18];
	int ret = 0;
	char *wan1_ifname;

	sprintf(avahi_config, "%s/%s", AVAHI_CONFIG_PATH, AVAHI_CONFIG_FN);

	strlcpy(et0macaddr, get_lan_hwaddr(), sizeof (et0macaddr));

	/* Generate avahi configuration file */
	if (!(fp = fopen(avahi_config, "w"))) {
		perror(avahi_config);
		return -1;
	}

	/* Set [server] configuration */
	fprintf(fp, "[Server]\n");
	fprintf(fp, "host-name=%s-%c%c%c%c\n", get_productid(),et0macaddr[12],et0macaddr[13],et0macaddr[15],et0macaddr[16]);
#ifdef RTCONFIG_FINDASUS
	fprintf(fp, "aliases=findasus,%s\n",get_productid());
	fprintf(fp, "aliases_llmnr=findasus,%s\n",get_productid());
#else
	fprintf(fp, "aliases=%s\n",get_productid());
	fprintf(fp, "aliases_llmnr=%s\n",get_productid());
#endif
	fprintf(fp, "use-ipv4=yes\n");
	fprintf(fp, "use-ipv6=no\n");
	fprintf(fp, "deny-interfaces=%s", nvram_safe_get("wan0_ifname"));
#ifdef RTCONFIG_DUALWAN
	wan1_ifname = nvram_safe_get("wan1_ifname");
	if (*wan1_ifname)
		fprintf(fp, ",%s", wan1_ifname);
#endif
	fprintf(fp, "\n");
	fprintf(fp, "ratelimit-interval-usec=1000000\n");
	fprintf(fp, "ratelimit-burst=1000\n");

	/* Set [publish] configuration */
	fprintf(fp, "\n[publish]\n");
	fprintf(fp, "publish-a-on-ipv6=no\n");
	fprintf(fp, "publish-aaaa-on-ipv4=no\n");

	/* Set [wide-area] configuration */
	fprintf(fp, "\n[wide-area]\n");
	fprintf(fp, "enable-wide-area=yes\n");

	/* Set [rlimits] configuration */
	fprintf(fp, "\n[rlimits]\n");
	fprintf(fp, "rlimit-core=0\n");
	fprintf(fp, "rlimit-data=4194304\n");
	fprintf(fp, "rlimit-fsize=0\n");
	fprintf(fp, "rlimit-nofile=768\n");
	fprintf(fp, "rlimit-stack=4194304\n");
	fprintf(fp, "rlimit-nproc=3\n");

	append_custom_config(AVAHI_CONFIG_FN, fp);
	fclose(fp);
	use_custom_config(AVAHI_CONFIG_FN, avahi_config);
	run_postconf("avahi-daemon", avahi_config);

	return ret;
}

int generate_afpd_service_config(void)
{
	FILE *fp;
	char afpd_service_config[80];
	int ret = 0;

	sprintf(afpd_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_AFPD_SERVICE_FN);

	/* Generate afpd service configuration file */
	if (!(fp = fopen(afpd_service_config, "w"))) {
		perror(afpd_service_config);
		return -1;
	}

	fprintf(fp, "<service-group>\n");
	fprintf(fp, "<name replace-wildcards=\"yes\">%%h</name>\n");
	fprintf(fp, "<service>\n");
	fprintf(fp, "<type>_afpovertcp._tcp</type>\n");
	fprintf(fp, "<port>548</port>\n");
	fprintf(fp, "</service>\n");
	fprintf(fp, "<service>\n");
	fprintf(fp, "<type>_device-info._tcp</type>\n");
	fprintf(fp, "<port>0</port>\n");
	fprintf(fp, "<txt-record>model=Xserve</txt-record>\n");
	fprintf(fp, "</service>\n");
	fprintf(fp, "</service-group>\n");

	append_custom_config(AVAHI_AFPD_SERVICE_FN, fp);
	fclose(fp);
	use_custom_config(AVAHI_AFPD_SERVICE_FN, afpd_service_config);
	run_postconf("afpd", afpd_service_config);

	return ret;
}

int generate_adisk_service_config(void)
{
	FILE *fp;
	char adisk_service_config[80];
	int ret = 0;

	sprintf(adisk_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_ADISK_SERVICE_FN);

	/* Generate adisk service configuration file */
	if (!(fp = fopen(adisk_service_config, "w"))) {
		perror(adisk_service_config);
		return -1;
	}

	fprintf(fp, "<service-group>\n");
	fprintf(fp, "<name replace-wildcards=\"yes\">%%h</name>\n");
	fprintf(fp, "<service>\n");
	fprintf(fp, "<type>_adisk._tcp</type>\n");
	fprintf(fp, "<port>9</port>\n");
	fprintf(fp, "<txt-record>dk0=adVN=%s,adVF=0x81</txt-record>\n", TIMEMACHINE_BACKUP_NAME);
	fprintf(fp, "</service>\n");
	fprintf(fp, "</service-group>\n");

	append_custom_config(AVAHI_ADISK_SERVICE_FN, fp);
	fclose(fp);
	use_custom_config(AVAHI_ADISK_SERVICE_FN, adisk_service_config);
	run_postconf("adisk", adisk_service_config);

	return ret;
}

int generate_itune_service_config(void)
{
	FILE *fp;
	char itune_service_config[80];
	int ret = 0;
	char servername[32];

	sprintf(itune_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_ITUNE_SERVICE_FN);

	/* Generate afpd service configuration file */
	if (!(fp = fopen(itune_service_config, "w"))) {
		perror(itune_service_config);
		return -1;
	}

	if (is_valid_hostname(nvram_safe_get("daapd_friendly_name")))
		strncpy(servername, nvram_safe_get("daapd_friendly_name"), sizeof(servername));
	else
		servername[0] = '\0';
	if(strlen(servername)==0) strncpy(servername, get_productid(), sizeof(servername));


	fprintf(fp, "<service-group>\n");
	fprintf(fp, "<name replace-wildcards=\"yes\">%s</name>\n",servername);
	fprintf(fp, "<service>\n");
	fprintf(fp, "<type>_daap._tcp</type>\n");
	fprintf(fp, "<port>3689</port>\n");
	fprintf(fp, "<txt-record>txtvers=1 iTShVersion=131073 Version=196610</txt-record>\n");
	fprintf(fp, "</service>\n");
	fprintf(fp, "</service-group>\n");

	append_custom_config(AVAHI_ITUNE_SERVICE_FN, fp);
	fclose(fp);
	use_custom_config(AVAHI_ITUNE_SERVICE_FN, itune_service_config);
	run_postconf("mt-daap", itune_service_config);

	return ret;
}

int start_mdns(void)
{
	char afpd_service_config[80];
	char adisk_service_config[80];
	char itune_service_config[80];

	sprintf(afpd_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_AFPD_SERVICE_FN);
	sprintf(adisk_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_ADISK_SERVICE_FN);
	sprintf(itune_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_ITUNE_SERVICE_FN);

	mkdir_if_none(AVAHI_CONFIG_PATH);
	mkdir_if_none(AVAHI_SERVICES_PATH);

	generate_mdns_config();

	if (pids("afpd") && nvram_match("timemachine_enable", "1"))
	{
		if (!f_exists(afpd_service_config))
			generate_afpd_service_config();
		if (!f_exists(adisk_service_config))
			generate_adisk_service_config();
	}else{
		if (f_exists(afpd_service_config)){
			unlink(afpd_service_config);
		}
		if (f_exists(adisk_service_config)){
			unlink(adisk_service_config);
		}
	}

	if(nvram_match("daapd_enable", "1") && pids("mt-daapd")){
		if (!f_exists(itune_service_config)){
			generate_itune_service_config();
		}
	}else{
		if (f_exists(itune_service_config)){
			unlink(itune_service_config);
		}
	}

	// Execute avahi_daemon daemon
	//xstart("avahi-daemon");
	char *avahi_daemon_argv[] = {"avahi-daemon", NULL};
	pid_t pid;

	return _eval(avahi_daemon_argv, NULL, 0, &pid);
}

void stop_mdns(void)
{
	if (pids("avahi-daemon"))
		killall("avahi-daemon", SIGTERM);
}

void restart_mdns(void)
{
	char afpd_service_config[80];
	char itune_service_config[80];
	sprintf(afpd_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_AFPD_SERVICE_FN);
	sprintf(itune_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_ITUNE_SERVICE_FN);

	if (nvram_match("timemachine_enable", "1") == f_exists(afpd_service_config)){
		if(nvram_match("daapd_enable", "1") == f_exists(itune_service_config)){
			unlink(itune_service_config);
			generate_itune_service_config();
			return;
		}
	}

	stop_mdns();
	sleep(2);
	start_mdns();
}

#endif

#ifdef  __CONFIG_NORTON__

int start_norton(void)
{
	eval("/opt/nga/init/bootstrap.sh", "start", "rc");

	return 0;
}

int stop_norton(void)
{
	int ret;

	ret = eval("/opt/nga/init/bootstrap.sh", "stop", "rc");

	return ret;
}

#endif /* __CONFIG_NORTON__ */


#ifdef RTCONFIG_IXIAEP
int
stop_ixia_endpoint(void)
{
	if (pids("endpoint"))
		killall_tk("endpoint");
	return 0;
}

int
start_ixia_endpoint(void)
{
	eval("start_endpoint");
}
#endif

#ifdef RTCONFIG_IPERF
int
stop_iperf(void)
{
	if (pids("iperf"))
		killall_tk("iperf");
	return 0;
}

int
start_iperf(void)
{
	char *iperf_argv[] = {"iperf", "-s", "-w", "1024k", NULL};
	pid_t pid;

	return _eval(iperf_argv, NULL, 0, &pid);
}
#endif

#ifdef  __CONFIG_WBD__
static int
start_wbd(void)
{
	int ret;

	ret = eval("wbd_master");
	ret = eval("wbd_slave");

	_dprintf("done\n");
	return ret;
}

void
stop_wbd(void)
{
	killall_tk("wbd_master");
	killall_tk("wbd_slave");
}
#endif /* __CONFIG_WBD__ */

#ifdef RTCONFIG_QCA_PLC_UTILS
int
stop_plchost(void)
{
	if (pids("plchost"))
		killall_tk("plchost");
	return 0;
}

int
start_plchost(void)
{
	char *plchost_argv[] = {"/usr/local/bin/plchost", "-i", "br0", "-N", BOOT_NVM_PATH, "-P", BOOT_PIB_PATH, NULL};
	pid_t pid;

	nvram_set("plc_wake", "1");
	return _eval(plchost_argv, NULL, 0, &pid);
}

void
reset_plc(void)
{
	FILE *fp;
	int rlen;
	char buf[1024], plc_mac[18];

	if (nvram_match("plc_ready", "0"))
		return;

#if defined(PLN12)
	if (!get_qca8337_PHY_power(1))
		doSystem("swconfig dev %s port 1 set power 1", MII_IFNAME);
#elif defined(PLAC56)
	int wake_gpio = nvram_get_int("plc_wake_gpio") & 0xff;

	if (get_gpio(wake_gpio))
		set_gpio(wake_gpio, 0);
#endif

	memset(plc_mac, 0, sizeof(plc_mac));
	strcpy(plc_mac, nvram_safe_get("plc_macaddr"));

	while (1) {
		fp = popen("/usr/local/bin/plctool -i br0 -I -e", "r");
		rlen = fread(buf, 1, sizeof(buf), fp);
		pclose(fp);
		if (rlen > 1) {
			buf[rlen-1] = '\0';
			if (strstr(buf, plc_mac))
				break;
		}
		dbg("%s: wait Powerline wake up...\n", __func__);
		sleep(1);
	}

	stop_plchost();
	eval("/usr/local/bin/plctool", "-i", "br0", "-R");
	nvram_set("plc_ready", "0");
}
#endif

#ifdef RTCONFIG_DHCP_OVERRIDE
int
stop_detectWAN_arp(void)
{
	if (pids("detectWAN_arp"))
		killall_tk("detectWAN_arp");
	return 0;
}

int
start_detectWAN_arp(void)
{
	char *detectWAN_arp_argv[] = {"detectWAN_arp", NULL};
	pid_t pid;

	return _eval(detectWAN_arp_argv, NULL, 0, &pid);
}
#endif

#ifdef RTCONFIG_CAPTIVE_PORTAL
#define UAM_WEB_PAGE	"/usr/lighttpd/css/uam.html"
#define UAM_JS_SCRIPT	"/usr/lighttpd/js/jquery-1.7.1.min.js"
#define UAM_JS	"/usr/lighttpd/css/uam.js"
#define BYPASS_PAGE	"/usr/lighttpd/css/Bypass.html"
#define	UAM_SRV_CONF	"/tmp/uamsrv.conf"
#define UAM_WEB_DIR	"/tmp/uamsrv/www"
int gen_uam_srv_conf()
{
	FILE *fp;

	/* write /tmp/lighttpd.conf */
	if ((fp=fopen(UAM_SRV_CONF, "r"))) {
		fclose(fp);
		unlink(UAM_SRV_CONF);
	}

	fp = fopen(UAM_SRV_CONF, "w");
	if (fp==NULL) return -1;

	/* Load modules */
	fprintf(fp, "server.modules+=(\"mod_alias\")\n");
	fprintf(fp, "server.modules+=(\"mod_userdir\")\n");
	fprintf(fp, "server.modules+=(\"mod_redirect\")\n");
	fprintf(fp, "server.modules+=(\"mod_compress\")\n");
	fprintf(fp, "server.modules+=(\"mod_usertrack\")\n");
	fprintf(fp, "server.modules+=(\"mod_rewrite\")\n");
	fprintf(fp, "server.modules+=(\"mod_captive_portal_uam\")\n");

	/* Basic setting */
	//fprintf(fp, "server.port=%s\n",get_webdav_http_port()); // defult setting, but no use
	fprintf(fp, "server.port=8083\n"); // defult setting, but no use
	if(strlen(nvram_safe_get("usbUIpath")) > 0)
		fprintf(fp, "server.document-root=\"%s/%s\"\n", UAM_WEB_DIR, "USB");
	else
		fprintf(fp, "server.document-root=\"%s\"\n", UAM_WEB_DIR);
	//fprintf(fp, "server.upload-dirs=(\"/tmp/lighttpd/uploads\")\n");
	fprintf(fp, "server.errorlog=\"/tmp/uamsrv/err.log\"\n");
	fprintf(fp, "server.pid-file=\"/tmp/uamsrv/uamsrv.pid\"\n");
	//fprintf(fp, "server.errorfile-prefix=\"/usr/lighttpd/css/status-\"\n");
	fprintf(fp, "server.syslog=\"/tmp/uamsrv/syslog.log\"\n");
	fprintf(fp, "server.error-handler-404=\"/index.html\"\n");

	//	**** Minetype setting **** //
	fprintf(fp, "mimetype.assign = (\n");
	fprintf(fp, "\".html\" => \"text/html\",\n");
	fprintf(fp, "\".htm\" => \"text/html\",\n");
	fprintf(fp, "\".css\" => \"text/css\",\n");
	fprintf(fp, "\".js\" => \"text/javascript\",\n");
/*
	fprintf(fp, "\".swf\" => \"application/x-shockwave-flash\",\n");
	fprintf(fp, "\".txt\" => \"text/plain\",\n");
	fprintf(fp, "\".jpg\" => \"image/jpeg\",\n");
	fprintf(fp, "\".gif\" => \"image/gif\",\n");
	fprintf(fp, "\".png\" => \"image/png\",\n");
	fprintf(fp, "\".pdf\" => \"application/pdf\",\n");
	fprintf(fp, "\".mp4\" => \"video/mp4\",\n");
	fprintf(fp, "\".m4v\" => \"video/mp4\",\n");
	fprintf(fp, "\".wmv\" => \"video/wmv\",\n");
	fprintf(fp, "\".mp3\" => \"audio/mpeg\",\n");
	fprintf(fp, "\".avi\" => \"video/avi\",\n");
	fprintf(fp, "\".mov\" => \"video/mov\"");
	fprintf(fp, "\"\" => \"application/x-octet-stream\"");
*/	
	fprintf(fp, ")\n");

	// **** Index file names **** //
	fprintf(fp, "index-file.names = ( \"index.php\", \"index.html\",\n");
	fprintf(fp, "\"index.htm\", \"default.htm\",\n");
	fprintf(fp, " \" index.lighttpd.html\" )\n");

	// **** url access deny
	//fprintf(fp, " url.access-deny             = ( \"~\", \".inc\" )\n");

	// **** static-file.exclude extensions
	//fprintf(fp," static-file.exclude-extensions = ( \".php\", \".pl\", \".fcgi\" )\n");

	// ****
	//fprintf(fp, "compress.cache-dir          = \"/tmp/lighttpd/compress/\"\n");
	fprintf(fp, "compress.filetype           = ( \"application/x-javascript\", \"text/css\", \"text/html\", \"text/plain\" )\n");

	/*** SSL ***/
	/* default : https://192.168.1.1:443/webdav */
	//fprintf(fp, "$SERVER[\"socket\"]==\":%s\"{\n",get_webdav_https_port());
	fprintf(fp, "$SERVER[\"socket\"]==\":8083\"{\n");
	fprintf(fp, "   server.document-root=\"%s\"\n", UAM_WEB_DIR);
//	fprintf(fp, "	ssl.pemfile=\"/etc/server.pem\"\n");
//	fprintf(fp, "	ssl.engine=\"enable\"\n");
	//fprintf(fp, "   ssl.use-compression=\"disable\"\n");
//	fprintf(fp, "   ssl.use-sslv2=\"disable\"\n");
//	fprintf(fp, "   ssl.use-sslv3=\"disable\"\n");
	//fprintf(fp, "   ssl.honor-cipher-order=\"enable\"\n");
	//fprintf(fp, "   ssl.cipher-list=\"ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA256:ECDHE-RSA-AES256-SHA:ECDHE-RSA-AES128-SHA:DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!MD5:!PSK:!RC4\"\n");
//	fprintf(fp, "   ssl.cipher-list=\"ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:AES256-GCM-SHA384:AES256-SHA256:AES256-SHA:AES128-GCM-SHA256:AES128-SHA256:AES128-SHA:ECDHE-RSA-DES-CBC3-SHA:DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!CAMELLIA:!DES:!MD5:!PSK:!RC4;\"\n");
	fprintf(fp, "   $HTTP[\"url\"] =~ \"^/Uam($|/)\"{\n");
	fprintf(fp, "       alias.url = ( \"/Uam\" => \"%s/Uam.html\" ) \n", UAM_WEB_DIR);
	fprintf(fp, "   }\n");
	fprintf(fp, "   $HTTP[\"url\"] =~ \"^/FreeUam($|/)\"{\n");
	fprintf(fp, "       alias.url = ( \"/FreeUam\" => \"%s/FreeUam.html\" ) \n", UAM_WEB_DIR);
	fprintf(fp, "   }\n");
	fprintf(fp, "   $HTTP[\"url\"] =~ \"^/uam.js($|/)\"{\n");
	fprintf(fp, "       alias.url = ( \"/uam.js\" => \"%s/uam.js\" ) \n", UAM_WEB_DIR);
	fprintf(fp, "   }\n");
	fprintf(fp, "   $HTTP[\"url\"] =~ \"^/jquery-1.7.1.min.js($|/)\"{\n");
	fprintf(fp, "       alias.url = ( \"/jquery-1.7.1.min.js\" => \"%s/jquery-1.7.1.min.js\" ) \n", UAM_WEB_DIR);
	fprintf(fp, "   }\n");
	fprintf(fp, "   $HTTP[\"url\"] =~ \"^/Bypass.html($|/)\"{\n");
	fprintf(fp, "       alias.url = ( \"/Bypass.html\" => \"%s/Bypass.html\" ) \n", UAM_WEB_DIR);
	fprintf(fp, "   }\n");
	fprintf(fp, "   $HTTP[\"url\"] =~ \"^/uam.html($|/)\"{\n");
	fprintf(fp, "       alias.url = ( \"/uam.html\" => \"%s/uam.html\" ) \n", UAM_WEB_DIR);
	fprintf(fp, "   }\n");
	fprintf(fp, "   $HTTP[\"url\"] =~ \"^/Uam.css($|/)\"{\n");
	fprintf(fp, "       alias.url = ( \"/Uam.css\" => \"%s/Uam.css\" ) \n", UAM_WEB_DIR);
	fprintf(fp, "   }\n");
	fprintf(fp, "   $HTTP[\"url\"] =~ \"^/FreeUam.css($|/)\"{\n");
	fprintf(fp, "       alias.url = ( \"/FreeUam.css\" => \"%s/FreeUam.css\" ) \n", UAM_WEB_DIR);
	fprintf(fp, "   }\n");
	fprintf(fp, "}\n"); /*** SSL ***/

	/* debugging */
	fprintf(fp, "debug.log-request-header=\"disable\"\n");
	fprintf(fp, "debug.log-response-header=\"disable\"\n");
	fprintf(fp, "debug.log-request-handling=\"disable\"\n");
	fprintf(fp, "debug.log-file-not-found=\"disable\"\n");
	fprintf(fp, "debug.log-condition-handling=\"disable\"\n");

	fclose(fp);
	return 0;
}

void start_uam_srv()
{
	pid_t pid;
	char *lighttpd_argv[] = { "/usr/sbin/uamsrv", "-f", "/tmp/uamsrv.conf", "-D", NULL };
	
//	if (nvram_match("enable_webdav", "1")) return;	/* don't  start uam server, start it in webdav process */

#if 0 // will be skipped during boot with the USB modem.
	if(!check_if_file_exist("/etc/server.pem")) {
		if(f_exists("/etc/cert.pem") && f_exists("/etc/key.pem")){
			system("cat /etc/key.pem /etc/cert.pem > /etc/server.pem");
		}else{
			notify_rc_and_wait("start_uam_srv");
			return;
		}
	}
#endif

	if (getpid()!=1) {
		notify_rc("start_uam_srv");
		return;
	}

	/* lighttpd directory */
	mkdir_if_none("/tmp/uamsrv");
	mkdir_if_none("/tmp/uamsrv/www");
	mkdir_if_none("/tmp/uamsrv/www/USB");
	chmod("/tmp/uamsrv/www", 0777);

	/* copy web page to lighttpd dir */
	eval("cp", UAM_WEB_PAGE, UAM_WEB_DIR);
	eval("cp", UAM_JS_SCRIPT, UAM_WEB_DIR);
	eval("cp", UAM_JS, UAM_WEB_DIR);
	eval("cp", BYPASS_PAGE, UAM_WEB_DIR);

	if (gen_uam_srv_conf()) return;
	//_dprintf("%s %d\n", __FUNCTION__, __LINE__);	// tmp test;
	if (!pids("uamsrv"))
		_eval(lighttpd_argv, NULL, 0, &pid);

	if (pids("uamsrv"))
		logmessage("UAM server", "daemon is started");
}

void stop_uam_srv()
{
	if (getpid() != 1) {
		notify_rc("stop_uam_srv");
		return;
	}

	if (pids("uamsrv")){
		kill_pidfile_tk("/tmp/uamsrv/uamsrv.pid");
		unlink("/tmp/uamsrv/uamsrv.pid");
		logmessage("UAM Server", "daemon is stoped");
	}
}
#endif

#if defined(RTCONFIG_BT_CONN)
void start_dbus_daemon(void)
{
	pid_t pid;
	char *dbusd_argv[] = { "dbus-daemon", "--system", NULL };

	if (getpid()!=1) {
		notify_rc("start_dbus_daemon");
		return;
	}

	mkdir_if_none("/var/lib/dbus");
	mkdir_if_none("/var/run/dbus");

	if (!pids("dbus-daemon")) {
		_eval(dbusd_argv, NULL, 0, &pid);
		logmessage("dbus", "daemon is started");
	}
}

void stop_dbus_daemon(void)
{
	if (getpid() != 1) {
		notify_rc("stop_dbus_daemon");
		return;
	}

	if (pids("dbus-daemon")){
		kill_pidfile_tk("/var/run/dbus.pid");
		logmessage("dbus", "daemon is stoped");
	}
}

int check_bluetooth_device(const char *bt_dev)
{
	struct hci_dev_info di;
	int fd;
	int ret = -1;

	if(bt_dev == NULL || strlen(bt_dev) < 4)
		return -1;

	if((fd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI)) >= 0)
	{
		memset(&di, 0, sizeof(di));
		di.dev_id = safe_atoi(bt_dev + 3);	// hci interface: hci0

		if (ioctl(fd, HCIGETDEVINFO, (void *) &di) == 0)
		{
			ret = 0;
		}
		close(fd);
	}

	if(ret == 0){
		char cmd[64];
		snprintf(cmd, sizeof(cmd), "hciconfig %s | grep 'UP RUNNING' -q", bt_dev);
		if(system(cmd) != 0)
		{
			ret = -1;
		}
	}
	return ret;
}

int isValidMac(unsigned char mac[]) {
	if (mac[0] == 0xff
	 ||(mac[0] == 0 && mac[1] == 0 && mac[2] == 0 && mac[3] == 0 && mac[4] == 0 && mac[5] == 0)
	   )
		return 0;

	/* valid mac */
	return 1;
}

void change_bt_mac(void)
{
#if defined(MAPAC2200) || defined(MAPAC1300) || defined(VZWAC1300) || defined(MAPAC1750)
	unsigned char bt_mac[6], mac2[6], mac5[6];
	unsigned char mac5_2[6];
	int plus = 0;

	extern int get_mac_2g(unsigned char dst[]);
	extern int get_mac_5g(unsigned char dst[]);

	memset(mac2, 0, sizeof(mac2));
	if(get_mac_2g(mac2) < 0 || isValidMac(mac2) != 1) {
		cprintf("invalid mac2(%02x:%02x:%02x:%02x:%02x:%02x)\n", mac2[0], mac2[1], mac2[2], mac2[3], mac2[4], mac2[5]);
		return;
	}

	memset(mac5, 0, sizeof(mac5));
	if(get_mac_5g(mac5) < 0 || isValidMac(mac5) != 1) {
		cprintf("invalid mac5(%02x:%02x:%02x:%02x:%02x:%02x)\n", mac5[0], mac5[1], mac5[2], mac5[3], mac5[4], mac5[5]);
		return;
	}

	memset(mac5_2, 0, sizeof(mac5_2));

	if(mac5[5] == mac2[5] +2)	// final version
	{
#if defined(MAPAC2200)
		plus = 5;
#elif defined(MAPAC1300) || defined(VZWAC1300)
		plus = 4;
#elif defined(MAPAC1750)
		plus = 3;
#endif
	}
	else if(mac5[5] == mac2[5] +4)	// need to check type
	{
#if defined(MAPAC2200)
		extern int get_mac_5g_2(unsigned char dst[]);
		if(get_mac_5g_2(mac5_2) < 0 || isValidMac(mac5_2) != 1) {
			cprintf("invalid mac5_2(%02x:%02x:%02x:%02x:%02x:%02x)\n", mac5_2[0], mac5_2[1], mac5_2[2], mac5_2[3], mac5_2[4], mac5_2[5]);
			return;
		}
		if(mac5_2[5] == mac5[5] +4)
			plus = 0xc;	// 1st version
		else if(mac5_2[5] < mac5[5])
			plus = 5;	// wrong high/low band version
#elif defined(MAPAC1300) || defined(VZWAC1300)
		plus = 8;
#endif
	}
#if defined(MAPAC2200)
	else if(mac5[5] == mac2[5] +8)
	{
		plus = 0xc;
	}
#endif

	if(plus != 0) {
		char mac_str[32];
		memcpy(bt_mac, mac2, 6);
		bt_mac[5] += plus;
		ether_etoa(bt_mac, mac_str);
		logmessage("BLE", "Set BT MAC plus(%d): %s", plus, mac_str);
		system("hciconfig hci0 up");
		doSystem("btconfig wba %s", mac_str);
	}
	else
	{
		logmessage("BLE", "## mac2(%02x:%02x:%02x:%02x:%02x:%02x)\n", mac2[0], mac2[1], mac2[2], mac2[3], mac2[4], mac2[5]);
		logmessage("BLE", "## mac5(%02x:%02x:%02x:%02x:%02x:%02x)\n", mac5[0], mac5[1], mac5[2], mac5[3], mac5[4], mac5[5]);
		logmessage("BLE", "## mac5_2(%02x:%02x:%02x:%02x:%02x:%02x)\n", mac5_2[0], mac5_2[1], mac5_2[2], mac5_2[3], mac5_2[4], mac5_2[5]);
	}
#endif
}

void start_bluetooth_service(void)
{
	pid_t pid;
	char *ble_argv[]= { "bluetoothd", "-n", "--plugin=autopair", NULL };
	const char *str_inf = "hci0";
	int retry=0;

#if defined(BLUECAVE)
	if(nvram_get_int("x_Setting") == 1){
		_dprintf("BLE: skip start_bluetooth_service\n");
		return;
	}
#endif

#ifndef BLUECAVE
	while (1) {
		if(check_bluetooth_device(str_inf))
		{
			printf("Failed to get HCI Device! Retry after 1 sec!\n");
			sleep(1);
			if (++retry > 30) {
				printf("Disable BLE daemon!\n");
				return;
			}
		}
		else
			break;
	}
#endif
	if (getpid()!=1) {
		notify_rc("start_bluetooth_service");
		return;
	}

	if (!pids("bluetoothd")) {
		change_bt_mac();
		system("hciconfig hci0 down");
		system("hciconfig hci0 reset");
		system("hciconfig hci0 up");
		system("hciconfig hci0 leadv 0");

		_eval(ble_argv, NULL, 0, &pid);
		logmessage("BLE", "daemon is started");
#if defined(RTCONFIG_LP5523)
		lp55xx_leds_proc(LP55XX_WHITE_LEDS, LP55XX_ACT_NONE);
#elif defined(MAPAC1750)
		set_rgbled(RGBLED_WHITE);
#endif /* RTCONFIG_LP5523 */
	}
}

void stop_bluetooth_service(void)
{
	if (pids("bluetoothd")) {
		_dprintf("Turn off Bluetooth\n");
		killall_tk("bluetoothd");
		system("hciconfig hci0 reset down");
		logmessage("bluetoothd", "daemon is stoped");
	}

}
#endif	/* RTCONFIG_BT_CONN */

#ifdef RTCONFIG_WIFI_SON
#if defined(RTCONFIG_ETHBACKHAUL) && defined(RTCONFIG_QCA_ORG_UPDOWN_SEPARATE)
void start_ethbl_lldpd(void)
{
	char ifname[32],buf[120];
	char *next;
	//char *lldpd[] = {"lldpd", nvram_safe_get("lan_ifname"), NULL};
	char *lldpd[] = {"lldpd", NULL};
	pid_t pid;

	if (pids("lldpd"))
		killall_tk("lldpd");

	if(sw_mode() == SW_MODE_ROUTER || nvram_match("cfg_master", "1")){
		eval("lldpd","-I","lo,br0","-c","-f","-s","-e","-M","4","-S","MAP-CAP");
	}
	else if(sw_mode() == SW_MODE_AP && !nvram_match("cfg_master", "1")){
		eval("lldpd","-I","lo,br0","-c","-f","-s","-e","-M","4","-S","MAP-RE");
	}
	sleep(2);
	memset(buf,0,sizeof(buf));
	foreach(ifname, nvram_safe_get("lan_ifnames"), next)
	{
		if(strlen(buf))
			sprintf(buf,"%s,%s",buf,ifname);	
		else
			sprintf(buf,"%s",ifname);	
	}
	sprintf(buf,"%s,%s",buf,"br0");
	eval("lldpcli","configure","system","interface","pattern",buf);
	eval("lldpcli","configure","lldp","tx-interval","10");
	eval("lldpcli","configure","lldp","tx-hold","2");
}
#endif

void start_hyfi_process(void)
{
	hyfi_process();
}

void start_hyfi_sync(void)
{
	pid_t pid;
	char *argv[]={"/sbin/delay_exec","4","hive_cap config_change",NULL};

	_eval(argv, NULL, 0, &pid);
}

void start_chg_swmode(void)
{
	FILE *fp = NULL;
	char path[]="/tmp/chg_swmode.sh";
	int sw_mode=nvram_get_int("sw_mode");
	int delay=30;

	if (!(fp = fopen(path, "w+")))
		return;
	else {
		fprintf(fp, "hive_cmd reboot\n"
				"sleep %d\n"
				"reboot\n", delay	);

		fclose(fp);
		chmod(path, 0777);
	}

	if (sw_mode==3) {
		nvram_set("lan_proto", "dhcp");
		nvram_set("lan_dnsenable_x", "1");
	}
	else if (sw_mode==1) {
		nvram_set("lan_proto", nvram_default_get("lan_proto"));
		nvram_set("lan_ipaddr", nvram_default_get("lan_ipaddr"));
		nvram_set("lan_ipaddr_rt", nvram_default_get("lan_ipaddr_rt"));
		nvram_set("dhcp_start", nvram_default_get("dhcp_start"));
		nvram_set("dhcp_end", nvram_default_get("dhcp_end"));
		nvram_set("lan_dnsenable_x", nvram_default_get("lan_dnsenable_x"));
	}

	nvram_commit();

	doSystem(path);
}

void start_spcmd(void)
{
	if (nvram_match("x_Setting", "1")) {
		char *cmd=nvram_get("spcmd");
		pid_t pid;
		if (!strcmp(cmd, "xx")) {
			char *argv[]={"/sbin/delay_exec","60","reboot",NULL};
			_eval(argv, NULL, 0, &pid);
		}
	}
}

#if defined(MAPAC2200)
void start_bhblock(void)
{
	int enable=nvram_get_int("ncb_enable");

	if (nvram_match("x_Setting", "1")) {
		logmessage("start_bhblock", "%s 5G backhaul client connection.", enable?"Enable":"Disable");
		doSystem("iwpriv wifi1 ncb_enable %d", enable);
	}
}
#endif
#endif

#ifdef RTCONFIG_FREERADIUS
void radiusd_updateDB(void)
{

		char *nv=NULL, *nvp=NULL, *b=NULL;
		char *profile_idx=NULL, *userlist=NULL, *tmp=NULL;
		char *cli_act=NULL, *cli_name=NULL, *cli_passwd=NULL, *cli_ip=NULL;
		char tmpcmd[256];
		char name[128];
		char passwd[128];

		unlink("/tmp/freeradius.db");
		/* create database for radius */
		system("sqlite3 /tmp/freeradius.db '.read /usr/freeradius/raddb/mods-config/sql/main/sqlite/schema.sql'");
		if(!access("/jffs/.sys/Permission/PMS_data.db3", F_OK)){
		//add user permission data to freeradius database.
			system("sqlite3 /tmp/freeradius.db 'attach \"/jffs/.sys/Permission/PMS_data.db3\" as pms;insert into main.radcheck select NULL, Name, \"Cleartext-Password\", \":=\", passwd from pms.user;'");
		}
		//add local user (for captive portal) to freeradius database.
		nv = nvp = strdup(nvram_safe_get("captive_portal_adv_local_clientlist"));
       		if (nv) {
			while ((b = strsep(&nvp, "<")) != NULL) {
				if ((vstrsep(b, ">", &profile_idx, &userlist) != 2 ))
					continue;
				while((tmp = strsep(&userlist, ",")) != NULL ){
					memset(name, 0, sizeof(name));
					memset(passwd, 0, sizeof(passwd));
					sscanf(tmp, "%[^:]:%s", name, passwd);

			  		snprintf(tmpcmd, sizeof(tmpcmd), "sqlite3 /tmp/freeradius.db 'insert into radcheck(username,attribute,op,value) values(\"%s\",\"Cleartext-Password\",\":=\",\"%s\");'", name, passwd);
			  		system(tmpcmd);

			  		snprintf(tmpcmd, sizeof(tmpcmd), "sqlite3 /tmp/freeradius.db 'insert into radcheck(username,attribute,op,value) values(\"%s\",\"NAS-Identifier\",\"==\",\"%s\");'", name, "radius");
			  		system(tmpcmd);
				}
	     		}
                        free(nv);
        	}
		snprintf(tmpcmd, sizeof(tmpcmd), "sqlite3 /tmp/freeradius.db 'delete from nas;'");
		system(tmpcmd);

		//add radius client to freeradius database
		nv = nvp = strdup(nvram_safe_get("radius_serv_list"));
       		if (nv) {
			while ((b = strsep(&nvp, "<")) != NULL) {
				if ((vstrsep(b, ">", &cli_act, &cli_name, &cli_passwd, &cli_ip) != 4 ))
					continue;
				if(atoi(cli_act) == 1){
					snprintf(tmpcmd, sizeof(tmpcmd), "sqlite3 /tmp/freeradius.db 'insert into nas(id,nasname,shortname,secret) values(NULL,\"%s\",\"%s\",\"%s\");'", cli_ip, cli_name, cli_passwd);
			  	system(tmpcmd);

				}
	     		}
			free(nv);
		}

#if 0
		/* add group and radgroupreply for auth check */
		system("sqlite3 /tmp/freeradius.db 'insert into radusergroup(username,groupname) values(\"noauth\",\"user\");'");
		system("sqlite3 /tmp/freeradius.db 'insert into radgroupreply(groupname,attribute,op,value) values(\"user\",\"Auth-Type\",\":=\",\"Local\");'");
		system("sqlite3 /tmp/freeradius.db 'insert into radgroupreply(groupname,attribute,op,value) values(\"user\",\"Service-Type\",\":=\",\"Framed-User\");'");

		/* set idle timeout */
		system("sqlite3 /tmp/freeradius.db 'insert into radgroupreply(groupname,attribute,op,value) values(\"user\",\"Idle-Timeout\",\":=\",\"600'\");'");	//10 minutes for idle timeout
		system("sqlite3 /tmp/freeradius.db 'insert into radgroupreply(groupname,attribute,op,value) values(\"limit_time\",\"Session-Timeout\",\":=\",\"7200'\");'");	//120 minutes for session timeout

		/* add nas for radius check */
		system("sqlite3 /tmp/freeradius.db 'insert into nas(id,nasname,shortname,secret) values(1,\"127.0.0.1\",\"radius\",\"radius\");'");

#endif
}

void start_radiusd(void)
{
	pid_t pid;
	char *radiusd_argv[] = { "radiusd", "-s", "-P", NULL };

	if (nvram_get_int("radius_serv_enable")) {
		if (getpid()!=1) {
			notify_rc("start_radiusd");
			return;
		}

		mkdir_if_none("/var/run/radiusd");
		mkdir_if_none("/var/log/radius");

		radiusd_updateDB();
		if (!pids("radiusd")) {
			_eval(radiusd_argv, NULL, 0, &pid);
			logmessage("Radius Server", "daemon is started");
		}
		start_firewall(wan_primary_ifunit(), 0);
	}
}


void stop_radiusd()
{
	if (getpid() != 1) {
		notify_rc("stop_radiusd");
		return;
	}

	if (pids("radiusd")){
		kill_pidfile_tk("/var/run/radiusd/radiusd.pid");
		logmessage("Radius Server", "daemon is stoped");
	}
	start_firewall(wan_primary_ifunit(), 0);
}

#endif

//chillispot
#if defined(RTCONFIG_CHILLISPOT) || defined(RTCONFIG_COOVACHILLI)
void chilli_localUser()
{
	char *nv=NULL, *nvp=NULL, *b=NULL;
	char *profile_idx=NULL, *userlist=NULL, *tmp=NULL;
#if 0
	PMS_ACCOUNT_INFO_T *account_list, *follow_account;
	PMS_ACCOUNT_GROUP_INFO_T *group_list, *follow_group;
	PMS_OWNED_INFO_T *owned_account=NULL;
	int acc_num=0, group_num=0;
#endif

	FILE *fp;	

	fp=fopen("/tmp/localusers_cp", "w");

        if (fp==NULL){
	   perror("open local user file failed\n");
	   return;
	}
#if 0
        if(PMS_GetAccountInfo(PMS_ACTION_GET_FULL, &account_list, &group_list, &acc_num, &group_num) < 0){
                printf("Can't read the account list.\n");
        	goto EXIT;
	}
        for(follow_group = group_list; follow_group != NULL; follow_group = follow_group->next){
                owned_account=follow_group->owned_account;
                if(follow_group->active  == 1 && (!strcmp(follow_group->name, nvram_safe_get("captivpeGroup")))){
			 break;
		}
	}
	if(follow_group!=NULL){
        	while(owned_account!=NULL){
                        PMS_ACCOUNT_INFO_T *Account_owned=(PMS_ACCOUNT_INFO_T *)owned_account->member;
			fprintf(fp, "%s:%s\n", Account_owned->name, Account_owned->passwd );
			printf("Owned Account: %s\t", Account_owned->name);
                        owned_account=owned_account->next;
        	}
	}
	PMS_FreeAccInfo(&account_list, &group_list);	
#endif
	fprintf(fp, "noauth:noauth\n");
	nv = nvp = strdup(nvram_safe_get("captive_portal_adv_local_clientlist"));
    if (nv) {
		while ((b = strsep(&nvp, "<")) != NULL) {
        	if ((vstrsep(b, ">", &profile_idx, &userlist) != 2 ))
            	continue;
			   
			while((tmp = strsep(&userlist, ",")) != NULL ){
				if(strlen(tmp) > 0){
					fprintf(fp, "%s\n", tmp);
				}
			}
	    }
        free(nv);
    }
//EXIT:
	if(fp!=NULL) fclose(fp);

}

#ifdef RTCONFIG_CAPTIVE_PORTAL

#define FREEWIFIIF "br1"
#define CPIF "br2"
void chilli_addif2br0(const char *ifname)
{
    char *nv = NULL, *nvp = NULL, *b = NULL;

    nv = nvp = strdup(nvram_safe_get("lan_ifnames"));

    if(nv)
    {
	while ((b = strsep(&nvp, " ")) != NULL)
	{
          SKIP_ABSENT_FAKE_IFACE(b);
	  printf("b=%s\n",b);
	  if(b && strcmp(b,ifname))
		eval("brctl","addif","br0",b);
	}
	free(nv);
    }
}


void main_config(void)
{
	char *iface = NULL;

	eval("modprobe","tun"); //load tun mod

        if(nvram_match("chilli_enable", "1"))
	{
		  iface = nvram_safe_get("chilli_interface");
		  if (nvram_match("chilli_nowifibridge", "1"))
		  {
		     if(iface)
		     {
			   eval("brctl","delif","br0",iface); //del interface from br0
			   chilli_addif2br0(iface);
		     }
		  }
		  else
		  {
			if(iface)
			{
			   eval("brctl","addif","br0",iface); //add interface to br0
			}
		  }
	}else if(nvram_match("hotss_enable", "1"))
	{
             	  iface = nvram_safe_get("hotss_interface");
		  if (nvram_match("hotss_nowifibridge", "1"))
		  {
		     if(iface)
		     {
			   eval("brctl","delif","br0",iface); //del interface from br0
			   chilli_addif2br0(iface);
		     }
		  }
		  else
		  {
			if(iface)
			{
			   eval("brctl","addif","br0",iface); //add interface to br0
			}
		  }
	}

	/*if (nvram_match("chilli_nowifibridge", "1"))
	{
		if(nvram_match("chilli_enable", "1"))
			iface = nvram_safe_get("chilli_interface");
		else if(nvram_match("hotss_enable", "1"))
			iface = nvram_safe_get("hotss_interface");

		if(iface)
		{
		   eval("brctl","delif","br0",iface); //del interface from br0
		   chilli_addif2br0(iface);
		}
	}
	else
	{
		iface = nvram_safe_get("chilli_interface");
		if(iface)
		{
		   eval("brctl","addif","br0",iface); //add interface to br0
		}
	}*/
}

void chilli_config(void)
{
	FILE *fp;
	FILE *local_fp;
	int i, k=0;
	char lan_ip[64];	
	char chilli_url[100];
	char domain[64];
	char *tmp_str=NULL;
	char *tmp=NULL, *passwd=NULL;;
	int time=0;
	char ip_mask[32] = "192.168.182.0/24";
	int gw[4] = { 192, 168, 182, 1}, cidr = 24;

	if (!(fp = fopen("/tmp/chilli.conf", "w"))) {
		perror("/tmp/chilli.conf");
		return;
	}
	
	/* Get network and generate gateway IP. */
	strlcpy(ip_mask, nvram_get("chilli_net")? : nvram_default_get("chilli_net"), sizeof(ip_mask));
	if (sscanf(ip_mask, "%d.%d.%d.%d/%d", &gw[0], &gw[1], &gw[2], &gw[3], &cidr) == 5) {
		gw[3]++;
	}

        //if(nvram_match("chilli_Radius", "1")){	
		fprintf(fp, "radiusserver1 %s\n", nvram_get("chilli_radius"));
		fprintf(fp, "radiusserver2 %s\n", nvram_get("chilli_backup"));
		if (nvram_invmatch("chilli_radiusauthport", ""))
		    fprintf(fp, "radiusauthport %s\n", nvram_get("chilli_radiusauthport"));
		//fprintf(fp, "radiussecret %s\n", nvram_get("chilli_pass"));
		if (nvram_invmatch("chilli_radiusnasid", ""))
		    fprintf(fp, "radiusnasid %s\n", nvram_get("chilli_radiusnasid"));

//	}else{
		fprintf(fp, "localusers %s\n", "/tmp/localusers");   
		if (!(local_fp = fopen("/tmp/localusers", "w"))) {
			perror("/tmp/localusers");
			return;
		}
		passwd=nvram_safe_get("captive_portal_passcode");
		if(strlen(passwd) >0 ) fprintf(local_fp, "noauth:%s", passwd);
	    else fprintf(local_fp, "noauth:noauth");
		fclose(local_fp);
//	}

	if (nvram_match("chilli_nowifibridge", "1"))
		fprintf(fp, "dhcpif %s\n", FREEWIFIIF);
	else
		fprintf(fp, "dhcpif %s\n", "br0");
	
	memset(lan_ip, 0, sizeof(lan_ip));
	memset(chilli_url,0, sizeof(chilli_url));
	strcpy(lan_ip, nvram_safe_get("lan_ipaddr"));

	if(!nvram_match("chilli_url", lan_ip))
	   nvram_set("chilli_url", lan_ip);
	//sprintf(chilli_url, "https://%s/Uam", lan_ip);
	sprintf(chilli_url, "%s://%s:8083/FreeUam", nvram_safe_get("chilli_protocol"), lan_ip);
	fprintf(fp, "uamserver %s\n", chilli_url);
	memset(domain, 0, sizeof(domain));
	strcpy(domain, nvram_safe_get("brdName"));

	if(strlen(domain) > 0 ){
		while(domain[k] != '\0'){
			domain[k]=tolower(domain[k]);
			k++;
		}
		if(strlen(domain) > 0){
			fprintf(fp, "domain %s\n", domain);
			fprintf(fp, "domaindnslocal\n");
		}
	}
	fprintf(fp, "cmdsocketport %s\n", "42424");
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) fprintf(fp, "ipv6\n");
#endif
	fprintf(fp, "tundev %s\n", "tun22");   
	fprintf(fp, "uamaliasip %d.%d.%d.%d\n", gw[0], gw[1], gw[2], gw[3]);
	fprintf(fp, "redirssl\n");  
	fprintf(fp, "sslcertfile %s\n", "/etc/cert.pem");  
	fprintf(fp, "sslkeyfile %s\n", "/etc/key.pem"); 
	if((time=nvram_get_int("chilli_authtime")) > 0)
		fprintf(fp, "challengetimeout2 %d\n", time);
	tmp_str=nvram_safe_get("chilli_awaytime");
	if(strcmp(tmp_str,""))
		fprintf(fp, "defidletimeout %lu\n", strtoul(tmp_str, &tmp, 10));
	tmp_str=nvram_safe_get("chilli_sessiontime");
	if(strcmp(tmp_str,""))
	        fprintf(fp, "defsessiontimeout %lu\n", strtoul(tmp_str, &tmp, 10));
#endif

	fprintf(fp, "dns1 %s\n", nvram_safe_get("lan_ipaddr"));

	char *chilli_uamsecret =  nvram_get("chilli_uamsecret");
#ifdef RTCONFIG_NVRAM_ENCRYPT
	int declen = pw_dec_len(chilli_uamsecret);
	char dec_passwd[declen];
	memset(dec_passwd, 0, sizeof(dec_passwd));
	pw_dec(chilli_uamsecret, dec_passwd);
	chilli_uamsecret = dec_passwd;
#endif
	if (nvram_invmatch("chilli_uamsecret", ""))
		fprintf(fp, "uamsecret %s\n", chilli_uamsecret);
	if (nvram_invmatch("chilli_uamanydns", "0"))
		fprintf(fp, "uamanydns\n");
	if (nvram_invmatch("chilli_uamallowed", ""))
		fprintf(fp, "uamallowed %s\n", nvram_get("chilli_uamallowed"));

	if (nvram_match("chilli_nowifibridge", "1"))
	{
		if (nvram_invmatch("chilli_net", ""))
			fprintf(fp, "net %s\n", nvram_get("chilli_net"));
	}
	else
		fprintf(fp, "net %s\n", ip_mask);
	if (nvram_match("chilli_macauth", "1")) {
		fprintf(fp, "macauth\n");
		char *chilli_macpasswd = nvram_safe_get("chilli_macpasswd");
#ifdef RTCONFIG_NVRAM_ENCRYPT
		int declen2 = pw_dec_len(chilli_macpasswd);
		char dec_passwd2[declen2];
		memset(dec_passwd2, 0, sizeof(dec_passwd2));
		pw_dec(chilli_macpasswd, dec_passwd2;);
		chilli_macpasswd = dec_passwd2;
#endif
		if (strlen(chilli_macpasswd) > 0)
			fprintf(fp, "macpasswd %s\n", chilli_macpasswd);
		else
			fprintf(fp, "macpasswd password\n");
	}
	if (nvram_match("chilli_802.1Xauth", "1"))
		fprintf(fp, "eapolenable\n");

	tmp_str=nvram_safe_get("chilli_bandwidthMaxUp");
	if ((strcmp(tmp_str,"")!=0) && (strcmp(tmp_str,"0")!=0)){
		fprintf(fp, "defbandwidthmaxup %ld\n", strtol(tmp_str, NULL, 10)*1024);
	}
	tmp_str=nvram_safe_get("chilli_bandwidthMaxDown");
	if ((strcmp(tmp_str,"")!=0) && (strcmp(tmp_str,"0")!=0)){
		fprintf(fp, "defbandwidthmaxdown %ld\n", strtol(tmp_str, NULL, 10)*1024);
	}
	
	if (nvram_invmatch("chilli_lease", ""))
			fprintf(fp, "lease %s\n", nvram_get("chilli_lease"));	
	
	if (nvram_invmatch("chilli_additional", "")) {
		char *add = nvram_safe_get("chilli_additional");

		i = 0;
		do {
			if (add[i] != 0x0D)
				fprintf(fp, "%c", add[i]);
		}
		while (add[++i]);
		i = 0;
		int a = 0;
		char *filter = strdup(add);

		do {
			if (add[i] != 0x0D)
				filter[a++] = add[i];
		}
		while (add[++i]);

		filter[a] = 0;
		if (strcmp(filter, add)) {
			nvram_set("chilli_additional", filter);
			nvram_commit();
		}
		free(filter);
	}
	fflush(fp);
	fclose(fp);

	return;
}
void chilli_config_CP(void)
{
	FILE *fp;
	int i, k=0;
	char lan_ip[64];	
	char domain[64];
	char chilli_url[100];
	char *tmp_str=NULL;
	char *tmp=NULL;
	int time=0;
	char ip_mask[32] = "192.168.183.0/24";
	int gw[4] = { 192, 168, 183, 1}, cidr = 24;

	if (!(fp = fopen("/tmp/chilli-cp.conf", "w"))) {
		perror("/tmp/chilli-cp.conf");
		return;
	}
	
	/* Get network and generate gateway IP. */
	strlcpy(ip_mask, nvram_get("cp_net")? : nvram_default_get("cp_net"), sizeof(ip_mask));
	if (sscanf(ip_mask, "%d.%d.%d.%d/%d", &gw[0], &gw[1], &gw[2], &gw[3], &cidr) == 5) {
		gw[3]++;
	}

	if(nvram_match("cp_Radius", "0")){
		fprintf(fp, "radiusserver1 %s\n", "127.0.0.1");
		fprintf(fp, "radiusserver2 %s\n", "127.0.0.1");
		chilli_localUser();
		fprintf(fp, "localusers %s\n", "/tmp/localusers_cp");
	}else{
		fprintf(fp, "radiusserver1 %s\n", nvram_get("cp_radius"));
		fprintf(fp, "radiusserver2 %s\n", nvram_get("cp_backup"));
		if (nvram_invmatch("cp_radiusauthport", "")){
	    		fprintf(fp, "radiusauthport %s\n", nvram_get("cp_radiusauthport"));
		}
		if (nvram_invmatch("cp_radiusnasid", "")){
	    		fprintf(fp, "radiusnasid %s\n", nvram_get("cp_radiusnasid"));
		}
		if (nvram_invmatch("cp_radiussecret", "")){
	    		fprintf(fp, "radiussecret %s\n", nvram_get("cp_radiussecret"));
		}
	}

	if (nvram_match("chilli_nowifibridge", "1"))
		fprintf(fp, "dhcpif %s\n", CPIF);
	else
		fprintf(fp, "dhcpif %s\n", "br0");
	memset(lan_ip, 0, sizeof(lan_ip));
	memset(chilli_url,0, sizeof(chilli_url));
	strcpy(lan_ip, nvram_safe_get("lan_ipaddr"));

	if(!nvram_match("chilli_url", lan_ip))
		nvram_set("chilli_url", lan_ip);
	//sprintf(chilli_url, "https://%s/Uam", lan_ip);
	sprintf(chilli_url, "%s://%s:8083/Uam", nvram_safe_get("cp_protocol"), lan_ip);
	fprintf(fp, "uamserver %s\n", chilli_url);
	memset(domain, 0, sizeof(domain));
	strcpy(domain, nvram_safe_get("brdName"));

	if(strlen(domain) > 0 ){
		while(domain[k] != '\0'){
			domain[k]=tolower(domain[k]);
			k++;
		}
		if(strlen(domain) > 0){
			fprintf(fp, "domain %s\n", domain);
			fprintf(fp, "domaindnslocal\n");
		}
	}
	fprintf(fp, "unixipc %s\n", "chilli-cp.ipc");
	fprintf(fp, "cmdsocketport %s\n", "42425");
#ifdef RTCONFIG_IPV6
       if (ipv6_enabled()) fprintf(fp, "ipv6\n");
#endif
	fprintf(fp, "uamport %s\n", "3998");
	fprintf(fp, "tundev %s\n", "tun23");   
	fprintf(fp, "uamaliasip %d.%d.%d.%d\n", gw[0], gw[1], gw[2], gw[3]);
	fprintf(fp, "redirssl\n");  
	fprintf(fp, "sslcertfile %s\n", "/etc/cert.pem");  
	fprintf(fp, "sslkeyfile %s\n", "/etc/key.pem"); 
	if((time=nvram_get_int("cp_authtime")) > 0)
		fprintf(fp, "challengetimeout2 %d\n", time);
	tmp_str=nvram_safe_get("cp_awaytime");
	if(strcmp(tmp_str,""))
		fprintf(fp, "defidletimeout %lu\n", strtoul(tmp_str, &tmp, 10));
	tmp_str=nvram_safe_get("cp_sessiontime");
	if(strcmp(tmp_str,""))
	        fprintf(fp, "defsessiontimeout %lu\n", strtoul(tmp_str, &tmp, 10));

	fprintf(fp, "dns1 %s\n", nvram_safe_get("lan_ipaddr"));

	char *enc_value =  nvram_get("cp_uamsecret");
#ifdef RTCONFIG_NVRAM_ENCRYPT
	int declen = pw_dec_len(enc_value);
	char dec_passwd[declen];
	memset(dec_passwd, 0, sizeof(dec_passwd));
	pw_dec(enc_value, dec_passwd);
	enc_value = dec_passwd;
#endif
	if (nvram_invmatch("cp_uamsecret", ""))
		fprintf(fp, "uamsecret %s\n", enc_value);
	if (nvram_invmatch("cp_uamanydns", "0"))
		fprintf(fp, "uamanydns\n");
	if (nvram_invmatch("cp_uamallowed", "")){
		char *tmp=NULL, *allowed_domain=NULL, *allowed_IP=NULL;
		char *sep=NULL, *tmpStr=NULL;
		int length=0;
		tmpStr=tmp=strdup(nvram_get("cp_uamallowed"));
		length=strlen(tmp)+1;
		allowed_domain=calloc(length+1, sizeof(char));
		allowed_IP=calloc(length+1, sizeof(char));	
		while((sep=strsep(&tmpStr, ",")) != NULL ){
			if(*sep !='\0'){
				if(illegal_ipv4_address(sep) !=0){
					if(!strncmp(sep, "www.", 4)) sep+=4;
					strncat(allowed_domain, sep, length-strlen(allowed_domain)-1);
					strncat(allowed_domain, ",", length-strlen(allowed_domain)-1);
				}else{
					strncat(allowed_IP, sep, length-strlen(allowed_IP)-1);
					strncat(allowed_IP, ",", length-strlen(allowed_IP)-1);
				}
			}
     	}	
		if(strlen(allowed_IP) > 0) fprintf(fp, "uamallowed %s\n", allowed_IP);
		if(strlen(allowed_domain) > 0) fprintf(fp, "uamdomain %s\n", allowed_domain);
		free(tmp);
		free(allowed_domain);
		free(allowed_IP);
	}
	if (nvram_match("cp_nowifibridge", "1"))
	{
		if (nvram_invmatch("cp_net", ""))
			fprintf(fp, "net %s\n", nvram_get("cp_net"));
	}
	else
		fprintf(fp, "net %s\n", ip_mask);
	if (nvram_match("cp_macauth", "1")) {
		fprintf(fp, "macauth\n");
		if (strlen(nvram_safe_get("cp_macpasswd")) > 0)
			fprintf(fp, "macpasswd %s\n", nvram_get("cp_macpasswd"));
		else
			fprintf(fp, "macpasswd password\n");
	}
	if (nvram_match("cp_802.1Xauth", "1"))
		fprintf(fp, "eapolenable\n");

	tmp_str=nvram_safe_get("cp_bandwidthMaxUp");
	if ((strcmp(tmp_str,"")!=0) && (strcmp(tmp_str,"0")!=0)){
		fprintf(fp, "defbandwidthmaxup %ld\n", strtol(tmp_str, NULL, 10)*1024);
	}
	tmp_str=nvram_safe_get("cp_bandwidthMaxDown");
	if ((strcmp(tmp_str,"")!=0) && (strcmp(tmp_str,"0")!=0)){
		fprintf(fp, "defbandwidthmaxdown %ld\n", strtol(tmp_str, NULL, 10)*1024);
	}
	if (nvram_invmatch("cp_lease", ""))
			fprintf(fp, "lease %s\n", nvram_get("cp_lease"));
	
	if (nvram_invmatch("cp_additional", "")) {
		char *add = nvram_safe_get("chilli_additional");

		i = 0;
		do {
			if (add[i] != 0x0D)
				fprintf(fp, "%c", add[i]);
		}
		while (add[++i]);
		i = 0;
		int a = 0;
		char *filter = strdup(add);

		do {
			if (add[i] != 0x0D)
				filter[a++] = add[i];
		}
		while (add[++i]);

		filter[a] = 0;
		if (strcmp(filter, add)) {
			nvram_set("cp_additional", filter);
			nvram_commit();
		}
		free(filter);
	}
	fflush(fp);
	fclose(fp);

	return;
}

typedef struct md5_ctx_t {
	uint32_t A;
	uint32_t B;
	uint32_t C;
	uint32_t D;
	uint64_t total;
	uint32_t buflen;
	char buffer[128];
} md5_ctx_t;

#ifdef __BIG_ENDIAN__
# define BB_BIG_ENDIAN 1
# define BB_LITTLE_ENDIAN 0
#elif __BYTE_ORDER == __BIG_ENDIAN
# define BB_BIG_ENDIAN 1
# define BB_LITTLE_ENDIAN 0
#else
# define BB_BIG_ENDIAN 0
# define BB_LITTLE_ENDIAN 1
#endif


#if BB_BIG_ENDIAN
#define SWAP_BE16(x) (x)
#define SWAP_BE32(x) (x)
#define SWAP_BE64(x) (x)
#define SWAP_LE16(x) bswap_16(x)
#define SWAP_LE32(x) bswap_32(x)
#define SWAP_LE64(x) bswap_64(x)
#else
#define SWAP_BE16(x) bswap_16(x)
#define SWAP_BE32(x) bswap_32(x)
#define SWAP_BE64(x) bswap_64(x)
#define SWAP_LE16(x) (x)
#define SWAP_LE32(x) (x)
#define SWAP_LE64(x) (x)
#endif

#define FAST_FUNC

/* 0: fastest, 3: smallest */
#if CONFIG_MD5_SIZE_VS_SPEED < 0
# define MD5_SIZE_VS_SPEED 0
#elif CONFIG_MD5_SIZE_VS_SPEED > 3
# define MD5_SIZE_VS_SPEED 3
#else
# define MD5_SIZE_VS_SPEED CONFIG_MD5_SIZE_VS_SPEED
#endif

/* Initialize structure containing state of computation.
 * (RFC 1321, 3.3: Step 3)
 */
void FAST_FUNC md5_begin(md5_ctx_t *ctx)
{
	ctx->A = 0x67452301;
	ctx->B = 0xefcdab89;
	ctx->C = 0x98badcfe;
	ctx->D = 0x10325476;
	ctx->total = 0;
	ctx->buflen = 0;
}

/* These are the four functions used in the four steps of the MD5 algorithm
 * and defined in the RFC 1321.  The first function is a little bit optimized
 * (as found in Colin Plumbs public domain implementation).
 * #define FF(b, c, d) ((b & c) | (~b & d))
 */
#define FF(b, c, d) (d ^ (b & (c ^ d)))
#define FG(b, c, d) FF(d, b, c)
#define FH(b, c, d) (b ^ c ^ d)
#define FI(b, c, d) (c ^ (b | ~d))

#define rotl32(w, s) (((w) << (s)) | ((w) >> (32 - (s))))

/* Hash a single block, 64 bytes long and 4-byte aligned. */
static void md5_hash_block(const void *buffer, md5_ctx_t *ctx)
{
	uint32_t correct_words[16];
	const uint32_t *words = buffer;

#if MD5_SIZE_VS_SPEED > 0
	static const uint32_t C_array[] = {
		/* round 1 */
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
		/* round 2 */
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x2441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
		/* round 3 */
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x4881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		/* round 4 */
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
	};
	static const char P_array[] ALIGN1 = {
# if MD5_SIZE_VS_SPEED > 1
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,	/* 1 */
# endif
		1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12,	/* 2 */
		5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2,	/* 3 */
		0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9	/* 4 */
	};
# if MD5_SIZE_VS_SPEED > 1
	static const char S_array[] ALIGN1 = {
		7, 12, 17, 22,
		5, 9, 14, 20,
		4, 11, 16, 23,
		6, 10, 15, 21
	};
# endif	/* MD5_SIZE_VS_SPEED > 1 */
#endif
	uint32_t A = ctx->A;
	uint32_t B = ctx->B;
	uint32_t C = ctx->C;
	uint32_t D = ctx->D;

	/* Process all bytes in the buffer with 64 bytes in each round of
	   the loop.  */
	uint32_t *cwp = correct_words;
	uint32_t A_save = A;
	uint32_t B_save = B;
	uint32_t C_save = C;
	uint32_t D_save = D;

#if MD5_SIZE_VS_SPEED > 1
	const uint32_t *pc;
	const char *pp;
	const char *ps;
	int i;
	uint32_t temp;

	for (i = 0; i < 16; i++)
		cwp[i] = SWAP_LE32(words[i]);
	words += 16;

# if MD5_SIZE_VS_SPEED > 2
	pc = C_array;
	pp = P_array;
	ps = S_array - 4;

	for (i = 0; i < 64; i++) {
		if ((i & 0x0f) == 0)
			ps += 4;
		temp = A;
		switch (i >> 4) {
		case 0:
			temp += FF(B, C, D);
			break;
		case 1:
			temp += FG(B, C, D);
			break;
		case 2:
			temp += FH(B, C, D);
			break;
		case 3:
			temp += FI(B, C, D);
		}
		temp += cwp[(int) (*pp++)] + *pc++;
		temp = rotl32(temp, ps[i & 3]);
		temp += B;
		A = D;
		D = C;
		C = B;
		B = temp;
	}
# else
	pc = C_array;
	pp = P_array;
	ps = S_array;

	for (i = 0; i < 16; i++) {
		temp = A + FF(B, C, D) + cwp[(int) (*pp++)] + *pc++;
		temp = rotl32(temp, ps[i & 3]);
		temp += B;
		A = D;
		D = C;
		C = B;
		B = temp;
	}
	ps += 4;
	for (i = 0; i < 16; i++) {
		temp = A + FG(B, C, D) + cwp[(int) (*pp++)] + *pc++;
		temp = rotl32(temp, ps[i & 3]);
		temp += B;
		A = D;
		D = C;
		C = B;
		B = temp;
	}
	ps += 4;
	for (i = 0; i < 16; i++) {
		temp = A + FH(B, C, D) + cwp[(int) (*pp++)] + *pc++;
		temp = rotl32(temp, ps[i & 3]);
		temp += B;
		A = D;
		D = C;
		C = B;
		B = temp;
	}
	ps += 4;
	for (i = 0; i < 16; i++) {
		temp = A + FI(B, C, D) + cwp[(int) (*pp++)] + *pc++;
		temp = rotl32(temp, ps[i & 3]);
		temp += B;
		A = D;
		D = C;
		C = B;
		B = temp;
	}

# endif /* MD5_SIZE_VS_SPEED > 2 */
#else
	/* First round: using the given function, the context and a constant
	   the next context is computed.  Because the algorithms processing
	   unit is a 32-bit word and it is determined to work on words in
	   little endian byte order we perhaps have to change the byte order
	   before the computation.  To reduce the work for the next steps
	   we store the swapped words in the array CORRECT_WORDS.  */
# define OP(a, b, c, d, s, T) \
	do { \
		a += FF(b, c, d) + (*cwp++ = SWAP_LE32(*words)) + T; \
		++words; \
		a = rotl32(a, s); \
		a += b; \
	} while (0)

	/* Before we start, one word to the strange constants.
	   They are defined in RFC 1321 as
	   T[i] = (int)(4294967296.0 * fabs(sin(i))), i=1..64
	 */

# if MD5_SIZE_VS_SPEED == 1
	const uint32_t *pc;
	const char *pp;
	int i;
# endif	/* MD5_SIZE_VS_SPEED */

	/* Round 1.  */
# if MD5_SIZE_VS_SPEED == 1
	pc = C_array;
	for (i = 0; i < 4; i++) {
		OP(A, B, C, D, 7, *pc++);
		OP(D, A, B, C, 12, *pc++);
		OP(C, D, A, B, 17, *pc++);
		OP(B, C, D, A, 22, *pc++);
	}
# else
	OP(A, B, C, D, 7, 0xd76aa478);
	OP(D, A, B, C, 12, 0xe8c7b756);
	OP(C, D, A, B, 17, 0x242070db);
	OP(B, C, D, A, 22, 0xc1bdceee);
	OP(A, B, C, D, 7, 0xf57c0faf);
	OP(D, A, B, C, 12, 0x4787c62a);
	OP(C, D, A, B, 17, 0xa8304613);
	OP(B, C, D, A, 22, 0xfd469501);
	OP(A, B, C, D, 7, 0x698098d8);
	OP(D, A, B, C, 12, 0x8b44f7af);
	OP(C, D, A, B, 17, 0xffff5bb1);
	OP(B, C, D, A, 22, 0x895cd7be);
	OP(A, B, C, D, 7, 0x6b901122);
	OP(D, A, B, C, 12, 0xfd987193);
	OP(C, D, A, B, 17, 0xa679438e);
	OP(B, C, D, A, 22, 0x49b40821);
# endif /* MD5_SIZE_VS_SPEED == 1 */

	/* For the second to fourth round we have the possibly swapped words
	   in CORRECT_WORDS.  Redefine the macro to take an additional first
	   argument specifying the function to use.  */
# undef OP
# define OP(f, a, b, c, d, k, s, T) \
	do { \
		a += f(b, c, d) + correct_words[k] + T; \
		a = rotl32(a, s); \
		a += b; \
	} while (0)

	/* Round 2.  */
# if MD5_SIZE_VS_SPEED == 1
	pp = P_array;
	for (i = 0; i < 4; i++) {
		OP(FG, A, B, C, D, (int) (*pp++), 5, *pc++);
		OP(FG, D, A, B, C, (int) (*pp++), 9, *pc++);
		OP(FG, C, D, A, B, (int) (*pp++), 14, *pc++);
		OP(FG, B, C, D, A, (int) (*pp++), 20, *pc++);
	}
# else
	OP(FG, A, B, C, D, 1, 5, 0xf61e2562);
	OP(FG, D, A, B, C, 6, 9, 0xc040b340);
	OP(FG, C, D, A, B, 11, 14, 0x265e5a51);
	OP(FG, B, C, D, A, 0, 20, 0xe9b6c7aa);
	OP(FG, A, B, C, D, 5, 5, 0xd62f105d);
	OP(FG, D, A, B, C, 10, 9, 0x02441453);
	OP(FG, C, D, A, B, 15, 14, 0xd8a1e681);
	OP(FG, B, C, D, A, 4, 20, 0xe7d3fbc8);
	OP(FG, A, B, C, D, 9, 5, 0x21e1cde6);
	OP(FG, D, A, B, C, 14, 9, 0xc33707d6);
	OP(FG, C, D, A, B, 3, 14, 0xf4d50d87);
	OP(FG, B, C, D, A, 8, 20, 0x455a14ed);
	OP(FG, A, B, C, D, 13, 5, 0xa9e3e905);
	OP(FG, D, A, B, C, 2, 9, 0xfcefa3f8);
	OP(FG, C, D, A, B, 7, 14, 0x676f02d9);
	OP(FG, B, C, D, A, 12, 20, 0x8d2a4c8a);
# endif /* MD5_SIZE_VS_SPEED == 1 */

	/* Round 3.  */
# if MD5_SIZE_VS_SPEED == 1
	for (i = 0; i < 4; i++) {
		OP(FH, A, B, C, D, (int) (*pp++), 4, *pc++);
		OP(FH, D, A, B, C, (int) (*pp++), 11, *pc++);
		OP(FH, C, D, A, B, (int) (*pp++), 16, *pc++);
		OP(FH, B, C, D, A, (int) (*pp++), 23, *pc++);
	}
# else
	OP(FH, A, B, C, D, 5, 4, 0xfffa3942);
	OP(FH, D, A, B, C, 8, 11, 0x8771f681);
	OP(FH, C, D, A, B, 11, 16, 0x6d9d6122);
	OP(FH, B, C, D, A, 14, 23, 0xfde5380c);
	OP(FH, A, B, C, D, 1, 4, 0xa4beea44);
	OP(FH, D, A, B, C, 4, 11, 0x4bdecfa9);
	OP(FH, C, D, A, B, 7, 16, 0xf6bb4b60);
	OP(FH, B, C, D, A, 10, 23, 0xbebfbc70);
	OP(FH, A, B, C, D, 13, 4, 0x289b7ec6);
	OP(FH, D, A, B, C, 0, 11, 0xeaa127fa);
	OP(FH, C, D, A, B, 3, 16, 0xd4ef3085);
	OP(FH, B, C, D, A, 6, 23, 0x04881d05);
	OP(FH, A, B, C, D, 9, 4, 0xd9d4d039);
	OP(FH, D, A, B, C, 12, 11, 0xe6db99e5);
	OP(FH, C, D, A, B, 15, 16, 0x1fa27cf8);
	OP(FH, B, C, D, A, 2, 23, 0xc4ac5665);
# endif /* MD5_SIZE_VS_SPEED == 1 */

	/* Round 4.  */
# if MD5_SIZE_VS_SPEED == 1
	for (i = 0; i < 4; i++) {
		OP(FI, A, B, C, D, (int) (*pp++), 6, *pc++);
		OP(FI, D, A, B, C, (int) (*pp++), 10, *pc++);
		OP(FI, C, D, A, B, (int) (*pp++), 15, *pc++);
		OP(FI, B, C, D, A, (int) (*pp++), 21, *pc++);
	}
# else
	OP(FI, A, B, C, D, 0, 6, 0xf4292244);
	OP(FI, D, A, B, C, 7, 10, 0x432aff97);
	OP(FI, C, D, A, B, 14, 15, 0xab9423a7);
	OP(FI, B, C, D, A, 5, 21, 0xfc93a039);
	OP(FI, A, B, C, D, 12, 6, 0x655b59c3);
	OP(FI, D, A, B, C, 3, 10, 0x8f0ccc92);
	OP(FI, C, D, A, B, 10, 15, 0xffeff47d);
	OP(FI, B, C, D, A, 1, 21, 0x85845dd1);
	OP(FI, A, B, C, D, 8, 6, 0x6fa87e4f);
	OP(FI, D, A, B, C, 15, 10, 0xfe2ce6e0);
	OP(FI, C, D, A, B, 6, 15, 0xa3014314);
	OP(FI, B, C, D, A, 13, 21, 0x4e0811a1);
	OP(FI, A, B, C, D, 4, 6, 0xf7537e82);
	OP(FI, D, A, B, C, 11, 10, 0xbd3af235);
	OP(FI, C, D, A, B, 2, 15, 0x2ad7d2bb);
	OP(FI, B, C, D, A, 9, 21, 0xeb86d391);
# endif	/* MD5_SIZE_VS_SPEED == 1 */
#endif	/* MD5_SIZE_VS_SPEED > 1 */

	/* Add the starting values of the context.  */
	A += A_save;
	B += B_save;
	C += C_save;
	D += D_save;

	/* Put checksum in context given as argument.  */
	ctx->A = A;
	ctx->B = B;
	ctx->C = C;
	ctx->D = D;
}

/* Feed data through a temporary buffer to call md5_hash_aligned_block()
 * with chunks of data that are 4-byte aligned and a multiple of 64 bytes.
 * This function's internal buffer remembers previous data until it has 64
 * bytes worth to pass on.  Call md5_end() to flush this buffer. */
void FAST_FUNC md5_hash(const void *buffer, size_t len, md5_ctx_t *ctx)
{
	char *buf = (char *)buffer;

	/* RFC 1321 specifies the possible length of the file up to 2^64 bits,
	 * Here we only track the number of bytes.  */
	ctx->total += len;

	/* Process all input. */
	while (len) {
		unsigned i = 64 - ctx->buflen;

		/* Copy data into aligned buffer. */
		if (i > len)
			i = len;
		memcpy(ctx->buffer + ctx->buflen, buf, i);
		len -= i;
		ctx->buflen += i;
		buf += i;

		/* When buffer fills up, process it. */
		if (ctx->buflen == 64) {
			md5_hash_block(ctx->buffer, ctx);
			ctx->buflen = 0;
		}
	}
}

/* Process the remaining bytes in the buffer and put result from CTX
 * in first 16 bytes following RESBUF.  The result is always in little
 * endian byte order, so that a byte-wise output yields to the wanted
 * ASCII representation of the message digest.
 */
void FAST_FUNC md5_end(void *resbuf, md5_ctx_t *ctx)
{
	char *buf = ctx->buffer;
	int i;

	/* Pad data to block size.  */
	buf[ctx->buflen++] = 0x80;
	memset(buf + ctx->buflen, 0, 128 - ctx->buflen);

	/* Put the 64-bit file length in *bits* at the end of the buffer.  */
	ctx->total <<= 3;
	if (ctx->buflen > 56)
		buf += 64;
	for (i = 0; i < 8; i++)
		buf[56 + i] = ctx->total >> (i*8);

	/* Process last bytes.  */
	if (buf != ctx->buffer)
		md5_hash_block(ctx->buffer, ctx);
	md5_hash_block(buf, ctx);

	/* The MD5 result is in little endian byte order.
	 * We (ab)use the fact that A-D are consecutive in memory.
	 */
#if BB_BIG_ENDIAN
	ctx->A = SWAP_LE32(ctx->A);
	ctx->B = SWAP_LE32(ctx->B);
	ctx->C = SWAP_LE32(ctx->C);
	ctx->D = SWAP_LE32(ctx->D);
#endif
	memcpy(resbuf, &ctx->A, sizeof(ctx->A) * 4);
}

void hotspotsys_config(void)
{
	FILE *fp;
	char *next;
	char var[64];
	char *dnslist;
	int i;
	char protocol[8];

	md5_ctx_t MD;
	//_dprintf("aa\n");
	if (strlen(nvram_safe_get("hotss_remotekey")) != 12) {
		unsigned char hash[32];
		char *et0 = nvram_safe_get("0:macaddr");

		md5_begin(&MD);
		md5_hash(et0, 17, &MD);
		md5_end((unsigned char *)hash, &MD);
		char idkey[16];
		int i;

		for (i = 0; i < 6; i++)
			sprintf(&idkey[2 * i], "%02d", (hash[i] + hash[i + 1]) % 100);
		idkey[12] = '\0';
		nvram_set("hotss_remotekey", idkey);
		nvram_commit();
		char sendid[256];
		sprintf(sendid,
			"/usr/bin/wget http://tech.hotspotsystem.com/up.php?mac=`nvram get wl0_hwaddr|sed s/:/-/g`\\&operator=%s\\&location=%s\\&remotekey=%s",
			nvram_get("hotss_operatorid"), nvram_get("hotss_locationid"), nvram_get("hotss_remotekey"));
		system(sendid);
	}
	//_dprintf("bb\n");
	if (!(fp = fopen("/tmp/chilli.conf", "w"))) {
		perror("/tmp/chilli.conf");
		return;
	}
	//_dprintf("cc\n");
	fprintf(fp, "ipup /tmp/chilli/ip-up.sh\n");
	fprintf(fp, "ipdown /tmp/chilli/ip-down.sh\n");
	fprintf(fp, "radiusserver1 radius.hotspotsystem.com\n");
	fprintf(fp, "radiusserver2 radius2.hotspotsystem.com\n");
	fprintf(fp, "radiussecret hotsys123\n");

	if (nvram_match("hotss_nowifibridge", "1"))
		fprintf(fp, "dhcpif %s\n", nvram_safe_get("hotss_interface"));
	else
		fprintf(fp, "dhcpif %s\n", "br0");
	//fprintf(fp, "dhcpif %s\n", nvram_safe_get("hotss_interface"));
	if (nvram_match("hotss_nowifibridge", "1"))
	{
		if (nvram_invmatch("hotss_net", ""))
			fprintf(fp, "net %s\n", nvram_get("hotss_net"));
	}
	else
		fprintf(fp, "net %s\n", "192.168.182.0/24");

	char *uamdomain = "customer.hotspotsystem.com";
	if (!nvram_match("hotss_customuam", "")) {
		uamdomain = nvram_safe_get("hotss_customuam");
	}

	if(nvram_match("hotss_customuamproto", "1"))
		strcpy(protocol,"http");
	else
		strcpy(protocol,"https");

	fprintf(fp, "uamserver %s://%s/customer/hotspotlogin.php\n",protocol, uamdomain);

	if (nvram_invmatch("wan_get_dns", "0.0.0.0")
	    && nvram_invmatch("wan_get_dns", "")) {
		dnslist = nvram_safe_get("wan_get_dns");
		i = 1;
		foreach(var, dnslist, next) {
			if (i > 2)
				break;
			fprintf(fp, "dns%d %s\n", i, var);
			i++;
		}
	} else if (nvram_invmatch("wan_dns", "0.0.0.0")
		   && nvram_invmatch("wan_dns", "")) {
		dnslist = nvram_safe_get("wan_dns");
		i = 1;
		foreach(var, dnslist, next) {
			if (i > 2)
				break;
			fprintf(fp, "dns%d %s\n", i, var);
			i++;
		}
	} else if (nvram_invmatch("sv_localdns", "0.0.0.0")
		   && nvram_invmatch("sv_localdns", "")) {
		fprintf(fp, "dns1 %s\n", nvram_get("sv_localdns"));
		if (nvram_invmatch("altdns1", "0.0.0.0")
		    && nvram_invmatch("altdns1", ""))
			fprintf(fp, "dns2 %s\n", nvram_get("altdns1"));
	}
	fprintf(fp, "uamsecret hotsys123\n");
	fprintf(fp, "uamanydns\n");
	fprintf(fp, "radiusnasid %s_%s\n", nvram_get("hotss_operatorid"), nvram_get("hotss_locationid"));
	if (!nvram_match("hotss_loginonsplash", "1")) {
		fprintf(fp,
			"uamhomepage %s://%s/customer/index.php?operator=%s&location=%s%s\n",
			protocol, uamdomain, nvram_get("hotss_operatorid"), nvram_get("hotss_locationid"), nvram_match("hotss_customsplash", "1") ? "&forward=1" : "");
	}
	fprintf(fp, "coaport 3799\n");
	fprintf(fp, "coanoipcheck\n");
	fprintf(fp, "domain key.chillispot.info\n");

	if (nvram_invmatch("hotss_uamallowed", "")
	    && nvram_match("hotss_uamenable", "1"))
		fprintf(fp, "uamallowed %s\n", nvram_get("hotss_uamallowed"));

	fprintf(fp, "uamallowed live.adyen.com,%s\n", uamdomain);
	fprintf(fp, "uamallowed 66.211.128.0/17,216.113.128.0/17\n");
	fprintf(fp, "uamallowed 70.42.128.0/17,128.242.125.0/24\n");
	fprintf(fp, "uamallowed 62.249.232.74,155.136.68.77,155.136.66.34,66.4.128.0/17,66.211.128.0/17,66.235.128.0/17\n");
	fprintf(fp, "uamallowed 88.221.136.146,195.228.254.149,195.228.254.152,203.211.140.157,203.211.150.204\n");
	fprintf(fp, "uamallowed 82.199.90.0/24,91.212.42.0/24\n");

	fprintf(fp, "uamallowed www.paypal.com,www.paypalobjects.com\n");
	fprintf(fp, "uamallowed www.worldpay.com,select.worldpay.com,secure.ims.worldpay.com,www.rbsworldpay.com,secure.wp3.rbsworldpay.com\n");
	fprintf(fp, "uamallowed hotspotsystem.com,www.hotspotsystem.com,tech.hotspotsystem.com\n");
	fprintf(fp, "uamallowed a1.hotspotsystem.com,a2.hotspotsystem.com,a3.hotspotsystem.com,a4.hotspotsystem.com,a5.hotspotsystem.com,a6.hotspotsystem.com\n");
	fprintf(fp, "uamallowed a7.hotspotsystem.com,a8.hotspotsystem.com,a9.hotspotsystem.com,a10.hotspotsystem.com,a11.hotspotsystem.com,a12.hotspotsystem.com\n");
	fprintf(fp, "uamallowed a13.hotspotsystem.com,a14.hotspotsystem.com,a15.hotspotsystem.com,a16.hotspotsystem.com,a17.hotspotsystem.com,a18.hotspotsystem.com\n");
	fprintf(fp, "uamallowed a19.hotspotsystem.com,a20.hotspotsystem.com,a21.hotspotsystem.com,a22.hotspotsystem.com,a23.hotspotsystem.com,a24.hotspotsystem.com\n");
	fprintf(fp, "uamallowed a25.hotspotsystem.com,a26.hotspotsystem.com,a27.hotspotsystem.com,a28.hotspotsystem.com,a29.hotspotsystem.com,a30.hotspotsystem.com\n");

	fprintf(fp, "interval 300\n");

	fflush(fp);
	fclose(fp);
	//_dprintf("dd\n");
	return;
}

void stop_chilli(void)
{
	char *iface = NULL;
	
	if(!nvram_match("captive_portal_enable", "on")){
		nvram_set("chilli_enable", "0");
	}

	kill_pidfile_tk("/var/run/chilli.pid");
	unlink("/tmp/chilli.conf");
	unlink("/var/run/chilli.pid");
	if(pids("chilli")) {
		unlink("/tmp/uamsrv/www/FreeUam.html");
		unlink("/tmp/uamsrv/www/FreeUam.css");
		unlink("/tmp/chilli/ip-up.sh");
		unlink("/tmp/chilli/ip-down.sh");
		system("rm -rf /var/run/chilli");
		unlink("/tmp/localusers");
	}

	iface = nvram_safe_get("chilli_interface");
	if(iface){
		ifconfig(FREEWIFIIF, 0, NULL, NULL);
		eval("brctl","delbr", FREEWIFIIF); //delete interface br1
		nvram_set("lan1_ifname", "");
		nvram_set("lan1_ifnames", "");
	}
	start_firewall(wan_primary_ifunit(), 0);

	return;
}
void stop_CP(void)
{
	char *iface = NULL;
	
	if(!nvram_match("captive_portal_adv_enable", "on")){
	   nvram_set("cp_enable", "0");
	}

	kill_pidfile_tk("/var/run/chilli-cp.pid");
	unlink("/tmp/chilli-cp.conf");
	unlink("/var/run/chilli-cp.pid");
	if(pids("chilli")) {
		unlink("/tmp/chilli-cp.conf");
		unlink("/tmp/uamsrv/www/Uam.html");
		unlink("/tmp/uamsrv/www/Uam.css");
		unlink("/tmp/chilli/ip-up.sh");
		unlink("/tmp/chilli/ip-down.sh");
		system("rm -rf /var/run/chilli");
		unlink("/tmp/localusers_cp");
	}

	iface = nvram_safe_get("cp_interface");
	if(iface){
		ifconfig(CPIF, 0, NULL, NULL);
		eval("brctl","delbr", CPIF); //delete interface br2
		nvram_set("lan2_ifname", "");
		nvram_set("lan2_ifnames", "");
	}
	
	start_firewall(wan_primary_ifunit(), 0);

	return;
}
void Checkifnames(char *nvifnames, char *ifname)
{
	char tmp_ifnames[128], *next=NULL;
	char ifnames[16];
	memset(tmp_ifnames, 0, sizeof(tmp_ifnames));
	memset(ifnames, 0, sizeof(ifnames));
	foreach(ifnames, nvifnames, next){
		if(strcmp(ifname, ifnames))
			strcat(tmp_ifnames, ifnames);
			strcat(tmp_ifnames, " ");
	//		sprintf(tmp_ifnames, "%s ", ifnames);
	}
	strcpy(nvifnames, tmp_ifnames);
}


void bridge_ifByA(char *ifs, char *brif, int flag)
{
	char _ifs[64];
	char ifname[16], lan_ifnames[128], tmp_ifname[32];
	char brifnames[64];
	int count=0;
	int tmp1=0, idx=0;
	char hw_ifname[16];

	memset(_ifs, 0, sizeof(_ifs));
	memset(lan_ifnames, 0, sizeof(lan_ifnames));
	memset(brifnames, 0, sizeof(brifnames));
	memset(hw_ifname, 0, sizeof(hw_ifname));
	memset(tmp_ifname, 0, sizeof(tmp_ifname));
	strcpy(lan_ifnames, nvram_safe_get("lan_ifnames"));
	strcpy(_ifs, ifs);

	for(idx=0;idx<=strlen(ifs);idx++){
	    	if(_ifs[idx] == 'w'){
	       		count++;
	       		if(2 == count){
		   		memset(ifname, 0, sizeof(ifname));
		   		strncpy(ifname, &_ifs[tmp1], idx-tmp1);
		   		sprintf(tmp_ifname,"%s_ifname", ifname);
		   		strcpy(hw_ifname, nvram_safe_get(tmp_ifname));
		   		eval("brctl", "delif", "br0", hw_ifname);
		   		eval("brctl", "addif", brif, hw_ifname);
		   		strcat(brifnames,hw_ifname);
		   		strcat(brifnames," ");
		   		count=1;
		   		Checkifnames(lan_ifnames, hw_ifname);
	       		}
	       		tmp1=idx;
	    	}
	}
	if(1 == count){
	   	sprintf(tmp_ifname, "%s_ifname",&_ifs[tmp1]);
	   	strcpy(hw_ifname, nvram_safe_get(tmp_ifname));
	   	eval("brctl", "delif", "br0", hw_ifname);
	   	eval("brctl", "addif", brif, hw_ifname);
	   	strcat(brifnames, hw_ifname);
	   	Checkifnames(lan_ifnames, hw_ifname);
	}
	if(flag == 0){ //free wifi
		nvram_set("lan1_ifnames", brifnames);
		nvram_set("lan1_ifname", brif);
		nvram_set("chilli_interface", brifnames);
	}else{       //captive portal
		nvram_set("lan2_ifnames", brifnames);
		nvram_set("lan2_ifname", brif);
		nvram_set("cp_interface", brifnames);
	}
	nvram_set("lan_ifnames", lan_ifnames);
}

void DN2tmpfile(char *name)
{
	FILE *fp;
	if((fp = fopen("/tmp/chilli_domain", "w")) != NULL) {
             fprintf(fp, "%s", name);
             fclose(fp);
        }
}

void start_CP(void)
{
	char htmlPath[128];
	char cssPath[128];
	char jsonPath[128];
	char *nv=NULL, *nvp=NULL, *b=NULL;
	int brCount=0;
	char *profileName=NULL, *htmlIdx=NULL,    *awayTime=NULL,   *sessionTime=NULL;
	char *UIPath=NULL,      *ifName=NULL,     *UamAllowed=NULL, *authType=NULL, *RadiusOrNot=NULL;
	char *RadiusIP=NULL,    *RadiusPort=NULL, *RadiusScrt=NULL, *RadiusNas=NULL;
	char  *enService=NULL, *bandwidthMaxUp=NULL, *bandwidthMaxDown=NULL;
	
	if(!nvram_match("captive_portal_adv_enable", "on"))
		return;
	if (nvram_get_int("sw_mode") != SW_MODE_ROUTER)	
		return;

	stop_CP();		//ensure that its stopped
	nv = nvp = strdup(nvram_safe_get("captive_portal_adv_profile"));
        if (nv) {
		   //prevent uam server not start yet
		   mkdir_if_none("/tmp/uamsrv");
		   mkdir_if_none("/tmp/uamsrv/www");
		   chmod("/tmp/uamsrv/www", 0777);

                   while ((b = strsep(&nvp, "<")) != NULL) {
                          if ((vstrsep(b, ">", &htmlIdx, &profileName, &awayTime, &sessionTime,
				&UIPath, &ifName, &UamAllowed, &authType, &RadiusOrNot, &RadiusIP,
				&RadiusPort, &RadiusScrt, &RadiusNas, &enService, &bandwidthMaxDown, &bandwidthMaxUp) < 15 ))
                              continue;
			  brCount++;
			  if(!is_intf_up(CPIF)){
		  	     eval("brctl", "addbr", CPIF);
		  	     eval("ifconfig", CPIF, "up");
			  }
			  memset(htmlPath, 0, sizeof(htmlPath));
			  memset(cssPath, 0, sizeof(cssPath));
			  sprintf(htmlPath, "/jffs/customized_splash/%s.html", htmlIdx);
			  sprintf(cssPath, "/jffs/customized_splash/%s.css", htmlIdx);
			  sprintf(jsonPath, "/jffs/customized_splash/%s.json", htmlIdx);
			  bridge_ifByA(ifName, CPIF, 1);

			  nvram_set("cp_sessiontime", sessionTime);
			  nvram_set("cp_awaytime", awayTime);
			  nvram_set_int("cp_authtime", atoi(nvram_get("captive_portal_adv_idle_timeout")));
			  nvram_set_int("cp_authtype", atoi(authType));

			  if (strlen(UIPath) > 0)
			      nvram_set("cp_external_UI", UIPath);
			  else
			      nvram_unset("cp_external_UI");

			  nvram_set("cp_uamallowed", UamAllowed);
			  nvram_set("cp_Radius", RadiusOrNot);
			  if(atoi(RadiusOrNot) == 1){
			     nvram_set("cp_radius", RadiusIP);
			     nvram_set("cp_radiusauthport", RadiusPort);
			     nvram_set("cp_radiussecret", RadiusScrt);
			     nvram_set("cp_radiusnasid", RadiusNas);
			  }
			  
			  nvram_set("cp_bandwidthMaxUp", bandwidthMaxUp);
			  nvram_set("cp_bandwidthMaxDown", bandwidthMaxDown);
 
			  if(brCount > 0)
           		     nvram_set("cp_enable", "1");
			  else
           		     nvram_set("cp_enable", "0");

		   }
                         free(nv);
        }

	if (!nvram_match("cp_enable", "1") && !nvram_match("hotss_enable", "1"))
		return;

	//stop_CP();		//ensure that its stopped

	eval("modprobe","tun"); //load tun mod
    	if (nvram_match("cp_enable", "1")) {
		int r;
		char ip_cidr[32];

		strlcpy(ip_cidr, nvram_get("cp_net"), sizeof(ip_cidr));
		r = test_and_get_free_char_network(7, ip_cidr, EXCLUDE_NET_CAPTIVE_PORTAL);
		if (r == 1) {
			dbg("Network conflicts. Choose %s for Captive Portal.\n", ip_cidr);
			logmessage("Captive Portal", "%s.\n", ip_cidr);
			nvram_set("cp_net", ip_cidr);
		}
		chilli_config_CP();
	}
	else if(nvram_match("hotss_enable", "1"))
		hotspotsys_config();

	kill_pidfile_tk("/var/run/chilli-cp.pid");
	
	if(htmlIdx!=NULL){
	   unlink("/tmp/uamsrv/www/Uam.html");
	   symlink(htmlPath, "/tmp/uamsrv/www/Uam.html");
	   unlink("/tmp/uamsrv/www/Uam.css");
	   symlink(cssPath, "/tmp/uamsrv/www/Uam.css");
	   unlink("/tmp/uamsrv/www/Uam.json");
	   symlink(jsonPath, "/tmp/uamsrv/www/Uam.json");
	}
	start_firewall(wan_primary_ifunit(), 0);

	char *argv[] = {"chilli", "-c", "/tmp/chilli-cp.conf", "--pidfile=/var/run/chilli-cp.pid", NULL};

	pid_t pid;
	_eval(argv, NULL, 0, &pid);
	int unit=0;
	int conn_flag=0;
	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
		if(is_wan_connect(unit)){
			conn_flag=1;
			break;
		}
	}
	if(conn_flag){
		nvram_set_int("nat_state", NAT_STATE_UPDATE);
		_dprintf("nat_rule: start_nat_rules CP.\n");
		start_nat_rules();
	}else{
		_dprintf("nat_rule: stop_nat_rules CP.\n");
		stop_nat_rules();
	}
	//dd_syslog(LOG_INFO, "chilli : chilli daemon successfully started\n");
#ifdef CONFIG_BCMWL5
	start_eapd();
#endif
	return;
}

void start_chilli(void)
{
	//char htmlIdx[64];
	char htmlPath[128];
	char cssPath[128];
	char tmpCmd[256];
	char *nv=NULL, *nvp=NULL, *b=NULL;
	int brCount=0;
	char *ifName=NULL, /* *typeIdx=NULL,*/ *htmlIdx=NULL, *sessionTime=NULL;
	char *UIType=NULL, *UIPath=NULL, *enService=NULL, *brdName=NULL, *enPass;
	char *bandwidthMaxUp=NULL, *bandwidthMaxDown=NULL;
	char BrandName[32];
	int k=0;
	#define BYPASS_CP 0
	#define SIMPLE_CP 1

	#define NONE	    0
	#define EXTERNAL_UI 1
	#define INTERNAL_UI 2
	#define TEMPLATE_UI 3
	#define BypassUI "/tmp/uamsrv/www/Bypass.html"

	if(!nvram_match("captive_portal_enable", "on"))
        	return;

	if (nvram_get_int("sw_mode") != SW_MODE_ROUTER)	
		return;

	stop_chilli();		//ensure that its stopped
	nv = nvp = strdup(nvram_safe_get("captive_portal"));
        if (nv) {
		   //prevent uam server not start yet
		   mkdir_if_none("/tmp/uamsrv");
		   mkdir_if_none("/tmp/uamsrv/www");
		   chmod("/tmp/uamsrv/www", 0777);

              while ((b = strsep(&nvp, "<")) != NULL) {
                    if ((vstrsep(b, ">", &htmlIdx, &brdName, &sessionTime, &UIType, &UIPath, &ifName, &enService, &enPass, &bandwidthMaxDown, &bandwidthMaxUp) < 9 ))
                        continue;
			  brCount++;
			  if(!is_intf_up(FREEWIFIIF)){
		  	     eval("brctl", "addbr", FREEWIFIIF);
		  	     eval("ifconfig", FREEWIFIIF, "up");
			  }
			  memset(htmlPath, 0, sizeof(htmlPath));
			  memset(cssPath, 0, sizeof(cssPath));
#if 0
			  if (BYPASS_CP == atoi(typeIdx))
			      sprintf(htmlPath, "%s",  BypassUI);
			  else
#endif
			  sprintf(htmlPath, "/jffs/customized_splash/%s.html", htmlIdx);
			  sprintf(cssPath, "/jffs/customized_splash/%s.css", htmlIdx);
			  bridge_ifByA(ifName, FREEWIFIIF, 0);
			  memset(BrandName, 0, sizeof(BrandName));
			  strcpy(BrandName, brdName);
			  while(BrandName[k] != '\0'){
				if(BrandName[k] == ' ')
		   		   BrandName[k]='-';
				   k++;
			   }
			  nvram_set("brdName", BrandName);  //coovachilli added
			  nvram_set_int("chilli_sessiontime", 60 * atoi(sessionTime));
			  if (EXTERNAL_UI == atoi(UIType)){
			      nvram_set("external_UI", UIPath);
			  }else
			      nvram_unset("external_UI");
				
			  if (INTERNAL_UI == atoi(UIType)){
			      sprintf(tmpCmd, "http://%s", BrandName);
			      nvram_set("external_UI", tmpCmd);
			      nvram_set("usbUIpath", UIPath);
			      if(d_exists(UIPath) || f_exists(UIPath)){
	      			 remove("/tmp/uamsrv/www/USB");
	      			 unlink("/tmp/uamsrv/www/USB");
	      			 symlink(UIPath, "/tmp/uamsrv/www/USB");			
	   		      }
			  }else{
			      nvram_unset("usbUIpath");
			  }
			  
			  if((NULL == enPass) || (0 == atoi(enPass))) nvram_set("captive_portal_passcode", "");
#ifndef RTCONFIG_COOVACHILLI
			   DN2tmpfile(BrandName);
#endif
			   nvram_set("chilli_bandwidthMaxUp", bandwidthMaxUp);
			   nvram_set("chilli_bandwidthMaxDown", bandwidthMaxDown);
			   if(brCount > 0)
           		     nvram_set("chilli_enable", "1");
			   else
           		     nvram_set("chilli_enable", "0");
		   }
                         free(nv);
        }

	//_dprintf("11\n");
	if (!nvram_match("chilli_enable", "1") && !nvram_match("hotss_enable", "1"))
		return;

	//_dprintf("22\n");
	//stop_chilli();		//ensure that its stopped

	//_dprintf("33\n");
#if 0
	if(nvram_match("chilli_enable", "1"))
	{
		if (!strlen(nvram_safe_get("chilli_interface")))
			nvram_set("chilli_interface", "br0");
	}else if(nvram_match("hotss_enable", "1"))
	{
		if (!strlen(nvram_safe_get("hotss_interface")))
			nvram_set("hotss_interface", "br0");
	}
#endif
	//_dprintf("44\n");

	eval("modprobe","tun"); //load tun mod
	//_dprintf("55\n");
    	if (nvram_match("chilli_enable", "1")) {
		int r;
		char ip_cidr[32];

		strlcpy(ip_cidr, nvram_get("chilli_net"), sizeof(ip_cidr));
		r = test_and_get_free_char_network(7, ip_cidr, EXCLUDE_NET_FREE_WIFI);
		if (r == 1) {
			dbg("Network conflicts. Choose %s for Free Wi-Fi.\n", ip_cidr);
			logmessage("Free Wi-Fi", "%s.\n", ip_cidr);
			nvram_set("chilli_net", ip_cidr);
		}
		chilli_config();
	}
	else if(nvram_match("hotss_enable", "1"))
		hotspotsys_config();
	//_dprintf("66\n");

	kill_pidfile_tk("/var/run/chilli.pid");
	//_dprintf("77\n");

	if(htmlIdx!=NULL){
	   	unlink("/tmp/uamsrv/www/FreeUam.html");
	   	symlink(htmlPath, "/tmp/uamsrv/www/FreeUam.html"); 
	   	unlink("/tmp/uamsrv/www/FreeUam.css");
	   	symlink(cssPath, "/tmp/uamsrv/www/FreeUam.css"); 
	}
	start_firewall(wan_primary_ifunit(), 0);
	//_dprintf("88\n");

	char *argv[] = {"chilli", "-c", "/tmp/chilli.conf", NULL};

	pid_t pid;
	//_dprintf("99\n");
	_eval(argv, NULL, 0, &pid);

	int unit=0;
	int conn_flag=0;
	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
		if(is_wan_connect(unit)){
			conn_flag=1;
			break;
		}
	}
	if(conn_flag){
		nvram_set_int("nat_state", NAT_STATE_UPDATE);
		_dprintf("nat_rule: start_nat_rules chilli.\n");
		start_nat_rules();
	}else{
		_dprintf("nat_rule: stop_nat_rules chilli.\n");
		stop_nat_rules();
	}
	//dd_syslog(LOG_INFO, "chilli : chilli daemon successfully started\n");
#ifdef CONFIG_BCMWL5
	start_eapd();
#endif
	return;

}
#endif // RTCONFIG_COOVACHILLI

#ifdef RTCONFIG_NOTIFICATION_CENTER
void
setup_nc_event_conf()
{
	FILE *fp;
	char setConf[2048];
	char tmp[20];
	int  action;
	int  bufsize;
	int  i;

	mkdir_if_none(NOTIFY_CENTER_TEMP_DIR);
	/* Setting Default Config */
	if (!strcmp(nvram_safe_get("nc_setting_conf"), "")) {
		memset(setConf, 0, sizeof(setConf));
		for (i=0; mapInfo[i].value != 0; i++) {
			memset(tmp, 0, sizeof(tmp));
			action = mapInfo[i].action;
			NC_ACTION_CLR(action, NC_ACT_IFTTT_BIT);
			
			if (mapInfo[i].value == PROTECTION_VULNERABILITY_EVENT ||
			    mapInfo[i].value == PROTECTION_CC_EVENT ||
			    mapInfo[i].value == PROTECTION_MALICIOUS_SITE_EVENT) {
				
				snprintf(tmp, sizeof(tmp), "<%x>%d>%d",
					 mapInfo[i].value,
					 action,
					 mapInfo[i].eType);
			} else {
				NC_ACTION_CLR(action, NC_ACT_EMAIL_BIT);
				snprintf(tmp, sizeof(tmp), "<%x>%d>%d",
					 mapInfo[i].value,
					 action,
					 mapInfo[i].eType);
			}
			bufsize = sizeof(setConf) - strlen(setConf);
			strncat(setConf, tmp, bufsize-1);
		}
		//dbg("[%s:(%d)]: %s\n", __FUNCTION__, __LINE__, setConf);
		nvram_set("nc_setting_conf", setConf);
	}

	if ((fp = fopen(NOTIFY_SETTING_CONF, "w")) == NULL){
		_dprintf("fail to open %s\n", NOTIFY_SETTING_CONF);
		return;
	}

	fprintf(fp,"%s", nvram_safe_get("nc_setting_conf"));
	fclose(fp);
}
int
start_notification_center(void)
{
	char *nt_monitor_argv[] = {"nt_monitor", NULL};
	pid_t pid;

#ifdef RTCONFIG_CONCURRENTREPEATER
	if (mediabridge_mode())
		return;
#endif

	setup_nc_event_conf();
	am_setup_email_conf();
	am_setup_email_info();

	return _eval(nt_monitor_argv, NULL, 0, &pid);
}

int
stop_notification_center(void)
{
	killall_tk("nt_monitor");
	killall_tk("nt_actMail");
	killall_tk("nt_center");
	return 0;
}

void
update_nc_setting_conf()
{
	setup_nc_event_conf();
	kill_pidfile_s(NOTIFY_CENTER_PID_PATH, SIGUSR1);
}
#endif
#ifdef RTCONFIG_PROTECTION_SERVER
int
start_ptcsrv(void)
{
	char *ptcsrv_argv[] = {"protect_srv", NULL};
	pid_t pid;

	return _eval(ptcsrv_argv, NULL, 0, &pid);
}
void
stop_ptcsrv(void)
{
	killall_tk("protect_srv");
}

#endif
#ifdef RTCONFIG_NETOOL
int
start_netool(void)
{
	char *netool_argv[] = {"/sbin/netool", NULL};
	pid_t pid;

	return _eval(netool_argv, NULL, 0, &pid);
}

void
stop_netool(void)
{
	killall_tk("netool");
}
#endif
int
start_services(void)
{
#ifdef RTCONFIG_LANTIQ
	start_wave_monitor();
#endif
#if defined(RTCONFIG_RALINK_MT7621)
	setup_smp();	/* for adjust smp_affinity of cpu */
#endif

#ifdef __CONFIG_NORTON__
	start_norton();
#endif /* __CONFIG_NORTON__ */

#ifdef RTCONFIG_NOTIFICATION_CENTER
	start_notification_center();
#endif
#ifdef RTCONFIG_PROTECTION_SERVER
	start_ptcsrv();
#endif
#ifdef RTCONFIG_NETOOL
	start_netool();
#endif
	start_telnetd();
#ifdef RTCONFIG_SSH
	start_sshd();
#endif
#ifdef CONFIG_BCMWL5
	start_eapd();
	start_nas();
#elif defined(RTCONFIG_RALINK) || defined(RTCONFIG_REALTEK)
	start_8021x();
#endif
	start_wps();
#ifdef RTCONFIG_WPS
	start_wpsaide();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
	start_wlc_nt();
#endif
#if defined(RTCONFIG_WLCEVENTD)
	start_wlceventd();
#endif
#ifdef RTCONFIG_BCMWL6
#ifdef RTCONFIG_HSPOT
	start_hspotap();
#endif
	start_igmp_proxy();
#ifdef BCM_BSD
	start_bsd();
#endif
#ifdef BCM_APPEVENTD
	start_appeventd();
#endif
#ifdef BCM_SSD
	start_ssd();
#endif
#ifdef BCM_EVENTD
	start_eventd();
#endif
	start_acsd();
#ifdef RTCONFIG_DHDAP
	start_dhd_monitor();
#endif
#ifdef BCM_ASPMD
	start_aspmd();
#endif /* BCM_ASPMD */

#ifdef  __CONFIG_WBD__
	start_wbd();
#endif /* __CONFIG_WBD__ */

#endif	// RTCONFIG_BCMWL6
	start_dnsmasq();
#ifdef RTCONFIG_DHCP_OVERRIDE
	start_detectWAN_arp();
#endif
#if defined(RTCONFIG_MDNS)
	start_mdns();
#endif
	/* Link-up LAN ports after DHCP server ready. */
	start_lan_port(0);
#ifdef RTCONFIG_CROND
	start_cron();
#endif
#ifdef RTCONFIG_LETSENCRYPT
	start_letsencrypt();
#endif
	start_cifs();

	start_httpd();
#ifdef  __CONFIG_VISUALIZATION__
	start_visualization_tool();
#endif /* __CONFIG_VISUALIZATION__ */

	start_infosvr();
	restart_rstats();
#if !defined(HND_ROUTER)
	restart_cstats();
#endif
#ifdef RTCONFIG_DSL
	start_spectrum(); //Ren
#endif
#ifdef RTCONFIG_SYSSTATE
	start_sysstate();
#endif
#ifdef RTCONFIG_TRAFFIC_LIMITER
	init_traffic_limiter();
#endif
	start_watchdog();
#ifdef RTAC87U
	start_watchdog02();
#endif
#ifdef SW_DEVLED
	start_sw_devled();
#endif
#if defined(RTAC1200G) || defined(RTAC1200GP)
	start_wdg_monitor();
#endif
#ifdef RTCONFIG_DUALWAN
	if(!nvram_get_int("stop_adv_lb"))
		restart_dualwan();
#endif
#ifdef RTCONFIG_FANCTRL
	start_phy_tempsense();
#endif
#ifdef RTCONFIG_AMAS
	start_amas_wlcconnect();
	start_amas_bhctrl();	
	start_amas_lanctrl();
	start_amas_lldpd();
#endif
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)	
	start_psta_monitor();
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_BCMWL6)
	start_obd();	
#endif
	start_snooper();
#if 0
	start_lldpd();
#else
	start_lltd();
#endif
#ifdef RTCONFIG_TOAD
	start_toads();
#endif

#if defined(RTCONFIG_IPSEC)
	rc_ipsec_nvram_convert_check();
	if(nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable"))
		rc_ipsec_config_init();
#if defined(HND_ROUTER)
	rc_set_ipsec_stack_block_size();
#endif
#endif

#ifdef RTCONFIG_JFFS2USERICON
	start_lltdc();
#endif
	start_networkmap(1);

#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
	start_pptpd();
#endif

#ifdef RTCONFIG_USB
//	_dprintf("restart_nas_services(%d): test 8.\n", getpid());
	//restart_nas_services(0, 1);
#ifdef RTCONFIG_DISK_MONITOR
	start_diskmon();
#endif
#endif

#ifdef RTCONFIG_WEBDAV
	start_webdav();
#else
	if(f_exists("/opt/etc/init.d/S50aicloud"))
		system("sh /opt/etc/init.d/S50aicloud scan");
#endif

#ifdef RTCONFIG_TUNNEL
	start_mastiff();
#endif

#ifdef RTCONFIG_SNMPD
	start_snmpd();
#endif

#if defined(RTCONFIG_RALINK) && defined(RTCONFIG_WIRELESSREPEATER)
	apcli_start();
#endif

#if defined(RTCONFIG_BWDPI)
	start_bwdpi_check();
#endif
	start_hour_monitor_service();

#ifdef RTCONFIG_IPERF
	start_iperf();
	start_monitor();
#endif

#ifdef RTCONFIG_FBWIFI
	//start_fb_wifi();
	if(sw_mode() == SW_MODE_ROUTER){
		start_fbwifi();
	}
#endif

#ifdef RTCONFIG_IXIAEP
	start_ixia_endpoint();
#endif

#ifdef RTCONFIG_SAMBASRV
	start_samba();	// We might need it for wins/browsing services
#endif

#ifdef RTCONFIG_INTERNAL_GOBI
	start_lteled();
#endif

#ifdef RTCONFIG_PARENTALCTRL
	start_pc_block();
#endif

#ifdef RTCONFIG_TOR
	start_Tor_proxy();
#endif

#ifdef RTCONFIG_CLOUDCHECK
	start_cloudcheck();
#endif

#ifdef RTCONFIG_QCA_PLC_UTILS
	start_plchost();
#endif
#ifdef RTCONFIG_NEW_USER_LOW_RSSI
	start_roamast();
#endif

#if defined(RTCONFIG_KEY_GUARD)
	start_keyguard();
#endif

#if 0//defined(RTCONFIG_WTFAST)
	start_wtfast();
#endif

#ifdef RTCONFIG_COOVACHILLI
	start_chilli();
	start_CP();
#endif
#ifdef RTCONFIG_CAPTIVE_PORTAL
	start_uam_srv(); //move to init.c	/* start internal uam server */
#endif
#ifdef RTCONFIG_FREERADIUS
	start_radiusd();
#endif

#ifdef RTCONFIG_CAPTIVE_PORTAL
	start_chilli();
	start_CP();
	start_uam_srv(); //move to init.c	/* start internal uam server */
#endif

#if defined(RTCONFIG_BT_CONN)
	start_dbus_daemon();
#endif

	start_ecoguard();

	run_custom_script("services-start", NULL);

//	start_upnp();

	sanity_logs();
#if defined(HND_ROUTER) && defined(RTCONFIG_HNDMFG)
	hnd_mfg_services();
#endif
#ifdef RTCONFIG_CFGSYNC
	start_cfgsync();
#endif

#if !(defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK) || defined(RTCONFIG_REALTEK))
	start_erp_monitor();
#endif
#ifdef RTCONFIG_HD_SPINDOWN
#ifdef LINUX26
	start_usb_idle();
#endif
#endif

#ifdef RTCONFIG_ADTBW
	start_adtbw();
#endif

#ifdef RTCONFIG_PUSH_EMAIL
#ifdef RTCONFIG_DBLOG
	start_dblog(0);
#endif /* RTCONFIG_DBLOG */
#endif /* RTCONFIG_PUSH_EMAIL */
	return 0;
}

void
stop_logger(void)
{
	if (pids("klogd"))
		killall("klogd", SIGTERM);
	if (pids("syslogd"))
		killall("syslogd", SIGTERM);
}

void
stop_services(void)
{
	run_custom_script("services-stop", NULL);

#ifdef RTCONFIG_ADTBW
	stop_adtbw();
#endif
#if !(defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK) || defined(RTCONFIG_REALTEK))
	stop_erp_monitor();
#endif
#ifdef RTCONFIG_INTERNAL_GOBI
	stop_lteled();
#endif
	stop_hour_monitor_service();
#if defined(RTCONFIG_BWDPI)
	stop_bwdpi_wred_alive();
	stop_bwdpi_check();
	stop_dpi_engine_service(1);
#endif
#ifdef RTCONFIG_IXIAEP
	stop_ixia_endpoint();
#endif
#ifdef RTCONFIG_IPERF
	stop_monitor();
	stop_iperf();
#endif

#ifdef RTCONFIG_CAPTIVE_PORTAL
	stop_chilli();
	stop_CP();
#endif

#ifdef RTCONFIG_WEBDAV
	stop_webdav();
#else
	if(f_exists("/opt/etc/init.d/S50aicloud"))
		system("sh /opt/etc/init.d/S50aicloud scan");
#endif
#ifdef RTCONFIG_TUNNEL
	stop_mastiff();
#endif
#ifdef RTCONFIG_USB
//_dprintf("restart_nas_services(%d): test 9.\n", getpid());
	restart_nas_services(1, 0);
#ifdef RTCONFIG_DISK_MONITOR
	stop_diskmon();
#endif
#ifdef RTCONFIG_USB_PRINTER
	stop_lpd();
	stop_u2ec();
#endif
#endif
	stop_upnp();
	stop_lltd();
	stop_snooper();
#ifdef RTCONFIG_AMAS
	stop_amas_bhctrl();
	stop_amas_lanctrl();
	stop_amas_wlcconnect();
	stop_amas_lldpd();
#endif
	stop_watchdog();
#ifdef RTCONFIG_FANCTRL
	stop_phy_tempsense();
#endif
#ifdef RTCONFIG_BCMWL6
	stop_igmp_proxy();
#ifdef RTCONFIG_AMAS
	stop_obd();
#endif
#ifdef RTCONFIG_PROXYSTA
	stop_psta_monitor();
#endif
#endif
#if !defined(HND_ROUTER)
	stop_cstats();
#endif
	stop_rstats();
#ifdef RTCONFIG_DSL
	stop_spectrum(); //Ren
#endif
#ifdef RTCONFIG_JFFS2USERICON
	stop_lltdc();
#endif
	stop_networkmap();
	stop_infosvr();
#ifdef  __CONFIG_VISUALIZATION__
	stop_visualization_tool();
#endif /* __CONFIG_VISUALIZATION__ */
	stop_httpd();
	stop_cifs();
#ifdef RTCONFIG_LETSENCRYPT
	stop_letsencrypt();
#endif
#ifdef RTCONFIG_CROND
	stop_cron();
#endif
#ifdef RTCONFIG_DHCP_OVERRIDE
	stop_detectWAN_arp();
#endif
	stop_dnsmasq();
#if defined(RTCONFIG_MDNS)
	stop_mdns();
#endif
#ifdef RTCONFIG_IPV6
	/* what? */
#endif
#ifdef RTCONFIG_BCMWL6
#ifdef BCM_ASPMD
	stop_aspmd();
#endif
#ifdef RTCONFIG_DHDAP
	stop_dhd_monitor();
#endif
	stop_acsd();
#ifdef BCM_EVENTD
	stop_eventd();
#endif
#ifdef BCM_SSD
	stop_ssd();
#endif
#ifdef BCM_APPEVENTD
	stop_appeventd();
#endif
#ifdef BCM_BSD
	stop_bsd();
#endif
	stop_igmp_proxy();
#ifdef RTCONFIG_HSPOT
	stop_hspotap();
#endif
#endif
#if defined(RTCONFIG_WLCEVENTD)
	stop_wlceventd();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
	stop_wlc_nt();
#endif
#ifdef RTCONFIG_WPS
	stop_wpsaide();
#endif
	stop_wps();
#ifdef CONFIG_BCMWL5
	stop_nas();
	stop_eapd();
#elif defined RTCONFIG_RALINK
	stop_8021x();
#endif
#ifdef RTCONFIG_TOAD
	stop_toads();
#endif
	stop_telnetd();
#ifdef RTCONFIG_SSH
	stop_sshd();
#endif
#ifdef RTCONFIG_PROTECTION_SERVER
	stop_ptcsrv();
#endif
#ifdef RTCONFIG_SNMPD
	stop_snmpd();
#endif
#ifdef  __CONFIG_NORTON__
	stop_norton();
#endif /* __CONFIG_NORTON__ */
#ifdef RTCONFIG_QTN
	stop_qtn_monitor();
#endif
#ifdef RTCONFIG_FBWIFI
	if(sw_mode() == SW_MODE_ROUTER){
		stop_fbwifi();
	}
#endif
#ifdef RTCONFIG_PARENTALCTRL
	stop_pc_block();
#endif
#ifdef RTCONFIG_TOR
	stop_Tor_proxy();
#endif
#ifdef RTCONFIG_CLOUDCHECK
	stop_cloudcheck();
#endif
#ifdef RTCONFIG_TUNNEL
	stop_mastiff();
#endif
#ifdef RTCONFIG_KEY_GUARD
	stop_keyguard();
#endif

#ifdef RTCONFIG_CAPTIVE_PORTAL
	stop_uam_srv();	/* stop internal uam server */
#endif
#ifdef RTCONFIG_FREERADIUS
	stop_radiusd();
#endif
#ifdef __CONFIG_WBD__
	stop_wbd();
#endif /* __CONFIG_WBD__ */
#ifdef RTCONFIG_NOTIFICATION_CENTER
	stop_notification_center();
#endif
#ifdef RTCONFIG_SYSSTATE
	stop_sysstate();
#endif
#ifdef RTCONFIG_CFGSYNC
	stop_cfgsync();
#endif
}

#ifdef RTCONFIG_QCA
int stop_wifi_service(void)
{
   	int is_unload;
	int i, unit = -1, sunit = 0;
	unsigned int m;	/* bit0~3: 2G, bit4~7: 5G */
	char wif[256];
	char pid_path[] = "/var/run/hostapd_athXXX.pidYYYYYY";
	char path[] = "/sys/class/net/ath001XXXXXX";

	for (i = 0, unit = 0, sunit = 0, m = 0xFF; m > 0; ++i, ++sunit, m >>= 1) {
		if (i == 4) {
			unit = 1;
			sunit -= 4;
		}
		__get_wlifname(unit, sunit, wif);
		sprintf(path, "/sys/class/net/%s", wif);
		if (d_exists(path))
			ifconfig(wif, 0, NULL, NULL);
		sprintf(pid_path, "/var/run/hostapd_%s.pid", wif);
		if (!f_exists(pid_path))
			continue;

		kill_pidfile_tk(pid_path);
	}

#if defined(RTCONFIG_WIRELESSREPEATER)
#if defined(RTCONFIG_CONCURRENTREPEATER)
	eval("killall", "-SIGKILL", "wpa_supplicant");
#endif
#endif

#if defined(QCA_WIFI_INS_RM)
	if(sw_mode()==SW_MODE_REPEATER)
		is_unload=0;
	else
		is_unload=1;
#else
	is_unload=1;
#endif
	if(is_unload)
	{
		if (module_loaded("umac")) {
			modprobe_r("umac");
			sleep(2);
		}
		if (module_loaded("ath_dev"))
			modprobe_r("ath_dev");

		if (module_loaded("ath_hal"))
			modprobe_r("ath_hal");
	}
	return 0;
}
#endif

void
stop_services_mfg(void)
{
#ifdef RTCONFIG_USB
#ifdef RTCONFIG_DISK_MONITOR
	stop_diskmon();
#endif
#ifdef RTCONFIG_USB_PRINTER
	stop_lpd();
	stop_u2ec();
#endif
#ifdef RTCONFIG_USB_MODEM
#ifdef RTCONFIG_INTERNAL_GOBI
	stop_lteled();
#endif
#endif
#ifndef RTCONFIG_NO_USBPORT
	stop_usbled();
#endif
#endif
	stop_rstats();
#ifdef RTCONFIG_JFFS2USERICON
	stop_lltdc();
#endif
	stop_networkmap();
#if !(defined(HND_ROUTER) && defined(RTCONFIG_HNDMFG))
	stop_dnsmasq();
#endif
#if defined(RTCONFIG_MDNS)
	stop_mdns();
#endif
	stop_upnp();
	stop_lltd();
	stop_snooper();
#ifdef RTCONFIG_BCMWL6
#ifdef RTCONFIG_AMAS
	stop_obd();
#endif
#ifdef BCM_ASPMD
	stop_aspmd();
#endif
#ifdef RTCONFIG_DHDAP
	stop_dhd_monitor();
#endif
	stop_acsd();
#ifdef BCM_EVENTD
	stop_eventd();
#endif
#ifdef BCM_SSD
	stop_ssd();
#endif
#ifdef BCM_APPEVENTD
	stop_appeventd();
#endif
#ifdef BCM_BSD
	stop_bsd();
#endif
	stop_igmp_proxy();
#ifdef RTCONFIG_HSPOT
	stop_hspotap();
#endif
#endif
#if defined(RTCONFIG_WLCEVENTD)
	stop_wlceventd();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
	stop_wlc_nt();
#endif
#ifdef RTCONFIG_WPS
	stop_wpsaide();
#endif
	stop_wps();

	stop_wanduck();
	stop_logger();
	stop_ots();
#if defined(RTCONFIG_BWDPI)
	stop_bwdpi_check();
#endif
	stop_ntpc();
	stop_udhcpc(-1);
	platform_start_ate_mode();
#ifdef RTCONFIG_QCA_PLC_UTILS
	ate_ctl_plc_led();
#endif
#ifdef SW_DEVLED
	stop_sw_devled();
#endif
#if defined(RTCONFIG_BT_CONN)
#if defined(RTCONFIG_LP5523)
	lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_ACT_NONE);
#endif /* RTCONFIG_LP5523 */
	stop_bluetooth_service();
#endif	/* RTCONFIG_BT_CONN */
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
	start_envrams();
#endif

#ifdef RTCONFIG_AMAS
	stop_amas_wlcconnect();
	stop_amas_bhctrl();		
	stop_amas_lanctrl();
	stop_amas_lldpd();
#else
#ifdef RTCONFIG_WIRELESSREPEATER	
	stop_wlcconnect();
#endif
#endif
#ifdef RTCONFIG_QCA
	stop_wifi_service();
#endif
#ifdef RTCONFIG_SYSSTATE
	stop_sysstate();
#endif
#ifdef RTCONFIG_PROTECTION_SERVER
	stop_ptcsrv();
#endif
#if !(defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK) || defined(RTCONFIG_REALTEK))
	stop_erp_monitor();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
	stop_notification_center();
#endif
}

// 2008.10 magic

int start_wanduck(void)
{
	char *argv[] = {"/sbin/wanduck", NULL};
	pid_t pid;

#if 0
	int sw_mode = sw_mode();
	if(sw_mode != SW_MODE_ROUTER && sw_mode != SW_MODE_REPEATER)
		return -1;
#endif

#if defined(HND_ROUTER) && defined(RTCONFIG_HNDMFG)
	return 0;
#endif

	if(!strcmp(nvram_safe_get("wanduck_down"), "1"))
		return 0;

#ifdef RTCONFIG_MODEM_BRIDGE
	if(sw_mode() == SW_MODE_AP && nvram_get_int("modem_bridge"))
		return 0;
#endif

	return _eval(argv, NULL, 0, &pid);
}

void stop_wanduck(void)
{
	killall("wanduck", SIGTERM);
}

#ifdef RTL_WTDOG
void
stop_rtl_watchdog(void)
{
	// Stop Realtek watchdog
	FILE *file;
#ifdef RTCONFIG_RTL8197F
	file = fopen("/proc/watchdog_kick", "w+");
	if (file)
	{
		fputs("1", file);
		fclose(file);
	}
	file = fopen("/proc/watchdog_cmd", "w+");
	if (file)
	{
		fputs("enable 0 interval 0", file);
		fclose(file);
	}
#else
	file = fopen("/proc/watchdog_kick","w+");
	if(file)
	{
		fputs("111", file);
		fclose(file);
	}

	file = fopen("/proc/watchdog_start","w+");
	if(file)
	{
		fputs("222", file);
		fclose(file);
	}
#endif
}

void
start_rtl_watchdog(void)
{
	stop_rtl_watchdog();
	// Start Realtek watchdog
	FILE *file;
#ifdef RTCONFIG_RTL8197F
	file = fopen("/proc/watchdog_cmd", "w+");
	if (file)
	{
		fputs("enable 1 interval 33", file);
		fclose(file);
	}
	file = fopen("/proc/watchdog_kick", "w+");
	if (file)
	{
		fputs("1", file);
		fclose(file);
	}
#else
	file = fopen("/proc/watchdog_start","w+");
	if(file)
	{
		fputs("111", file);
		fclose(file);
	}

	file = fopen("/proc/watchdog_kick","w+");
	if(file)
	{
		fputs("111", file);
		fclose(file);
	}
#endif
}
#endif

void
stop_watchdog(void)
{
#ifdef RTL_WTDOG
	stop_rtl_watchdog();
#endif
	killall_tk("watchdog");
}

#if ! (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
void
stop_watchdog02(void)
{
	/* do nothing */
	return;
}
#endif  /* ! (RTCONFIG_QCA || RTCONFIG_RALINK) */

#ifdef SW_DEVLED
void
stop_sw_devled(void)
{
	killall_tk("sw_devled");
	return;
}
#endif

#if defined(RTAC1200G) || defined(RTAC1200GP)
void
stop_wdg_monitor(void)
{
	killall_tk("wdg_monitor");
	return;
}
#endif

#ifdef RTCONFIG_DUALWAN
int restart_dualwan(void)
{
	char *dualwan_argv[] = {"dualwan", NULL};
	pid_t dualpid;

	if(nvram_get_int("dualwan_down") == 1)
		return 0;

	killall("dualwan", SIGTERM);

	return _eval(dualwan_argv, NULL, 0, &dualpid);
}
#endif

int
start_watchdog(void)
{
	char *watchdog_argv[] = {"watchdog", NULL};
	pid_t whpid;

	return _eval(watchdog_argv, NULL, 0, &whpid);
}

#ifdef RTAC87U
int
start_watchdog02(void)
{
	char *watchdog_argv[] = {"watchdog02", NULL};
	pid_t whpid;

	if (pidof("watchdog02") > 0) return -1;

	return _eval(watchdog_argv, NULL, 0, &whpid);
}
#endif

#ifdef RTCONFIG_LANTIQ
int
start_wave_monitor(void)
{
	char *wave_monitor_argv[] = {"wave_monitor", NULL};
	pid_t wavepid;

	if (pidof("wave_monitor") > 0) return -1;

	return _eval(wave_monitor_argv, NULL, 0, &wavepid);
}
#endif
#ifdef SW_DEVLED
int
start_sw_devled(void)
{
	char *sw_devled_argv[] = {"sw_devled", NULL};
	pid_t whpid;

	if (pidof("sw_devled") > 0) return -1;

	return _eval(sw_devled_argv, NULL, 0, &whpid);
}
#endif

#if defined(RTAC1200G) || defined(RTAC1200GP)
int
start_wdg_monitor(void)
{
	char *wdg_monitor_argv[] = {"wdg_monitor", NULL};
	pid_t whpid;

	if (pidof("wdg_monitor") > 0) return -1;

	return _eval(wdg_monitor_argv, NULL, 0, &whpid);

}
#endif

#ifdef RTCONFIG_DSL
#ifdef RTCONFIG_RALINK
//Ren.B
int check_tc_upgrade(void)
{
	int ret_val_sep = 0;

	TRACE_PT("check_tc_upgrade\n");

	ret_val_sep = separate_tc_fw_from_trx();
	TRACE_PT("check_tc_upgrade, ret_val_sep=%d\n", ret_val_sep);
	if(ret_val_sep)
	{
		if(check_tc_firmware_crc() == 0)
		{
			TRACE_PT("check_tc_upgrade ret=1\n");
			return 1; //success
		}
	}
	TRACE_PT("check_tc_upgrade ret=0\n");
	return 0; //fail
}

//New version will rename downloaded firmware to /tmp/linux.trx as default.
int start_tc_upgrade(void)
{
	int ret_val_trunc = 0;
	int ret_val_comp = 0;

	TRACE_PT("start_tc_upgrade\n");

	if(check_tc_upgrade())
	{
		ret_val_trunc = truncate_trx();
		TRACE_PT("start_tc_upgrade ret_val_trunc=%d\n", ret_val_trunc);
		if(ret_val_trunc)
		{
			do_upgrade_adsldrv();
			ret_val_comp = compare_linux_image();
			TRACE_PT("start_tc_upgrade ret_val_comp=%d\n", ret_val_comp);
			if (ret_val_comp == 0)
			{
				// same trx
				TRACE_PT("same firmware\n");
				unlink("/tmp/linux.trx");
			}
			else
			{
				// different trx
				TRACE_PT("different firmware\n");
			}
		}
	}
	return 0;
}
//Ren.E
#endif
#endif

#ifdef RTCONFIG_FANCTRL
int
stop_phy_tempsense()
{
	if (pids("phy_tempsense")) {
		killall_tk("phy_tempsense");
	}
	return 0;
}

int
start_phy_tempsense()
{
	char *phy_tempsense_argv[] = {"phy_tempsense", NULL};
	pid_t pid;

	return _eval(phy_tempsense_argv, NULL, 0, &pid);
}
#endif

#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
int
stop_psta_monitor()
{
	if (pids("psta_monitor")) {
		killall_tk("psta_monitor");
	}
	return 0;
}

int
start_psta_monitor()
{
	char *psta_monitor_argv[] = {"psta_monitor", NULL};
	pid_t pid;

	
#ifdef RTCONFIG_AMAS
		if (dpsr_mode()
#ifdef RTCONFIG_DPSTA
			|| dpsta_mode()
#endif
		) {
			_dprintf("It is dpstr or dpsta mode, don't start psta_monitor\n");
			return 0;
		}
#endif
	
	return _eval(psta_monitor_argv, NULL, 0, &pid);
}
#endif

#if defined(RTCONFIG_AMAS) && (defined(RTCONFIG_BCMWL6) || defined(RTCONFIG_LANTIQ))
void
stop_obd(void)
{
	if (pids("obd"))
		killall_tk("obd");
}

void
start_obd(void)
{
#ifndef RTCONFIG_DISABLE_REPEATER_UI
	char *obd_argv[] = {"obd", NULL};
	pid_t pid;

	_eval(obd_argv, NULL, 0, &pid);
#else
	return;
#endif
}

static int no_need_obd(void)
{
#ifdef RTCONFIG_DISABLE_REPEATER_UI
	return -1;
#else
	if (g_reboot || g_upgrade)
		return -1;

	if (!is_router_mode() || (nvram_get_int("w_Setting") == 1) || (nvram_get_int("x_Setting") == 1))
		return -1;

	if (nvram_get_int("wlready") == 0)
		return -1;

	return pids("obd");
#endif
}
#endif

#ifdef RTCONFIG_IPERF
int
stop_monitor()
{
	if (pids("monitor")) {
		killall_tk("monitor");
	}
	return 0;
}

int
start_monitor()
{
	char *monitor_argv[] = {"monitor", NULL};
	pid_t pid;

	return _eval(monitor_argv, NULL, 0, &pid);
}
#endif

#ifdef RTCONFIG_QTN
int
stop_qtn_monitor()
{
	if (pids("qtn_monitor")) {
		killall_tk("qtn_monitor");
	}
	return 0;
}

int
start_qtn_monitor()
{
	char *qtn_monitor_argv[] = {"qtn_monitor", NULL};
	pid_t pid;

	return _eval(qtn_monitor_argv, NULL, 0, &pid);
}
#endif

#if defined(RTCONFIG_USB) && !defined(RTCONFIG_NO_USBPORT)
int
start_usbled(void)
{
	char *usbled_argv[] = {"usbled", NULL};
	pid_t whpid;

#if defined(HND_ROUTER) && defined(RTCONFIG_HNDMFG)
	return 0;
#endif

	stop_usbled();
	return _eval(usbled_argv, NULL, 0, &whpid);
}

int
stop_usbled(void)
{
	if (pids("usbled"))
		killall("usbled", SIGTERM);

	return 0;
}
#endif

#ifdef RTCONFIG_CROND
void start_cron(void)
{
	stop_cron();
	eval("crond");
}


void stop_cron(void)
{
	killall_tk("crond");
}
#endif

#ifdef RTCONFIG_QUAGGA
void stop_quagga(void)
{
	if (pids("zebra")){
		killall("zebra", SIGINT);
	}
	if (pids("ripd")){
		killall("ripd", SIGINT);
	}
}

int start_quagga(void)
{
	FILE *fp, *fp2;
	char *zebra_hostname;
	char *zebra_passwd;
	char *zebra_enpasswd;
	char *rip_hostname;
	char *rip_passwd;
/*
	char *wan_ip, *wan_ifname;
	int   unit;
	char tmp[32], prefix[] = "wanXXXXXXXXXX_";
*/

	if (!is_routing_enabled()) {
		_dprintf("return -1\n");
		return -1;
	}
	if (nvram_invmatch("quagga_enable", "1"))
		return -1;

/*
	unit = wan_primary_ifunit();
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	wan_ip = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));
	wan_ifname = get_wan_ifname(unit);

	if (!wan_ip || strcmp(wan_ip, "") == 0 || !inet_addr(wan_ip)) {
		logmessage("quagga", "WAN IP is empty.");
		return -1;
	}
*/

	zebra_passwd = nvram_safe_get("zebra_passwd");
	zebra_enpasswd = nvram_safe_get("zebra_enpasswd");
	rip_passwd = nvram_safe_get("rip_passwd");
	zebra_hostname = nvram_safe_get("productid");
	rip_hostname = nvram_safe_get("productid");
#ifdef RTCONFIG_NVRAM_ENCRYPT
	int declen = pw_dec_len(zebra_passwd);
	char dec_passwd[declen];
	memset(dec_passwd, 0, sizeof(dec_passwd));
	pw_dec(zebra_passwd, dec_passwd);
	zebra_passwd = dec_passwd;

	int declen2 = pw_dec_len(zebra_enpasswd);
	char dec_passwd2[declen2];
	memset(dec_passwd2, 0, sizeof(dec_passwd2));
	pw_dec(zebra_enpasswd, dec_passwd2);
	zebra_enpasswd = dec_passwd2;

	int declen3 = pw_dec_len(rip_passwd);
	char dec_passwd3[declen3];
	memset(dec_passwd3, 0, sizeof(dec_passwd3));
	pw_dec(rip_passwd, dec_passwd3);
	rip_passwd = dec_passwd3;
#endif
	if (pids("zebra")){
		killall("zebra", SIGINT);
		sleep(1);
	}
	if (pids("ripd")){
		killall("ripd", SIGINT);
		sleep(1);
	}
	if ((fp = fopen("/etc/zebra.conf", "w"))){
		fprintf(fp, "hostname %s\n", zebra_hostname);
		fprintf(fp, "password %s\n", zebra_passwd);
		fprintf(fp, "enable password %s\n", zebra_enpasswd);
		fprintf(fp, "log file /etc/zebra.log informational\n");
		append_custom_config("zebra.conf",fp);
		fclose(fp);
		use_custom_config("zebra.conf","/etc/zebra.conf");
		run_postconf("zebra","/etc/zebra.conf");
		eval("zebra", "-d", "-f", "/etc/zebra.conf");
	}
	if ((fp2 = fopen("/etc/ripd.conf", "w"))){
		fprintf(fp2, "hostname %s\n", rip_hostname);
		fprintf(fp2, "password %s\n", rip_passwd);
//		fprintf(fp2, "debug rip events\n");
//		fprintf(fp2, "debug rip packet\n");
		fprintf(fp2, "router rip\n");
		fprintf(fp2, " version 2\n");
#if !defined(BLUECAVE)
		fprintf(fp2, " network vlan2\n");
		fprintf(fp2, " network vlan3\n");
		fprintf(fp2, " passive-interface vlan2\n");
		fprintf(fp2, " passive-interface vlan3\n");
#else
		fprintf(fp2, " network eth1.2\n");
		fprintf(fp2, " network eth1.3\n");
		fprintf(fp2, " passive-interface eth1.2\n");
		fprintf(fp2, " passive-interface eth1.3\n");
#endif
		fprintf(fp2, "log file /etc/ripd.log informational\n");
		fprintf(fp2, "log stdout\n");

		append_custom_config("ripd.conf",fp2);
		fclose(fp2);
		use_custom_config("ripd.conf","/etc/ripd.conf");
		run_postconf("ripd","/etc/ripd.conf");
		eval("ripd", "-d", "-f", "/etc/ripd.conf");
	}
	return 0;
}
#endif

void start_script(int argc, char *argv[])
{
	int pid;

	argv[argc] = NULL;
	_eval(argv, NULL, 0, &pid);

}

// -----------------------------------------------------------------------------

/* -1 = Don't check for this program, it is not expected to be running.
 * Other = This program has been started and should be kept running.  If no
 * process with the name is running, call func to restart it.
 * Note: At startup, dnsmasq forks a short-lived child which forks a
 * long-lived (grand)child.  The parents terminate.
 * Many daemons use this technique.
 */
static void _check(pid_t pid, const char *name, void (*func)(void))
{
	if (pid == -1) return;

	if (pidof(name) > 0) return;

	syslog(LOG_DEBUG, "%s terminated unexpectedly, restarting.\n", name);
	func();

	// Force recheck in 500 msec
	setitimer(ITIMER_REAL, &pop_tv, NULL);
}

static void QOS_CONTROL()
{
#ifdef RTCONFIG_LANTIQ
	char ppa_cmd[255] = {0};
#endif

	add_iQosRules(get_wan_ifname(wan_primary_ifunit()));
#if defined(RTCONFIG_BWDPI)
	start_dpi_engine_service();
#endif
	// force to rebuild firewall to avoid some loopback issue
	if (nvram_match("fw_nat_loopback", "2"))
		start_firewall(wan_primary_ifunit(), 0);

	start_iQos();

#ifdef RTCONFIG_LANTIQ
	if (ppa_support(wan_primary_ifunit()) == 0) {
		snprintf(ppa_cmd, sizeof(ppa_cmd), "ppacmd delwan -i %s", get_wan_ifname(wan_primary_ifunit()));
		_dprintf("%s : remove ppa wan interface: %s\n", __FUNCTION__, ppa_cmd);
		system(ppa_cmd);
	}
#endif
}

void check_services(void)
{
//	TRACE_PT("keep alive\n");

	// Periodically reap any zombies
	setitimer(ITIMER_REAL, &zombie_tv, NULL);

#ifdef LINUX26
	_check(pids("hotplug2"), "hotplug2", start_hotplug2);
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_BCMWL6)
	_check(no_need_obd(), "obd", start_obd);
#endif
}

#define RC_SERVICE_STOP 0x01
#define RC_SERVICE_START 0x02

void factory_reset(void)
{
#ifdef RTCONFIG_CONCURRENTREPEATER
	nvram_set_int("led_status", LED_FACTORY_RESET);
#endif
	g_reboot = 1;
#ifdef RTCONFIG_REALTEK
/* [MUST] : Need to Clarify ... */
	set_led(LED_BLINK_SLOW, LED_BLINK_SLOW);
	nvram_commit();
#endif
#ifdef RTCONFIG_DSL
	eval("adslate", "sysdefault");
#endif
	stop_wan();

#ifdef RTCONFIG_NOTIFICATION_CENTER
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	eval("rm", "-rf", NOTIFY_DB_FOLDER);
#endif
#endif

#ifdef RTCONFIG_USB
#ifdef RTCONFIG_USB_MODEM
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	eval("rm", "-rf", "/jffs/sim");
#endif
#endif
#ifdef RTCONFIG_OPENVPN
#if defined(RTCONFIG_UBIFS)
	eval("rm", "-rf", OVPN_DIR_SAVE);
#endif
#endif

	if(get_model() == MODEL_RTN53)
	{
		eval("wlconf", "eth2", "down");
		modprobe_r("wl_high");
	}

#if !defined(RTN56UB1) && !defined(RTN56UB2)
#ifndef RTCONFIG_ERPTEST
	stop_usb();
#else
	stop_usb(0);
#endif
#ifndef RTCONFIG_NO_USBPORT
	stop_usbled();
#endif
#endif
#endif

	sleep(3);
	nvram_set(ASUS_STOP_COMMIT, "1");
#if defined(RTCONFIG_NVRAM_FILE)
	ResetDefault();
#else
	if(nvram_contains_word("rc_support", "nandflash"))	/* RT-AC56S,U/RT-AC68U/RT-N18U */
	{
#if defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
		system("mtd-erase -d nvram");
#else
#ifdef HND_ROUTER
		eval("hnd-erase", "nvram");
#else
		eval("mtd-erase2", "nvram");
#endif
#endif
	}
	else
	{
#if defined(RTAC1200G) || defined(RTAC1200GP)
		eval("mtd-erase2", "nvram");
#elif defined(RTCONFIG_REALTEK)
		ResetDefault();
#else
		eval("mtd-erase", "-d", "nvram");
#endif
#endif
	}
#ifdef RTCONFIG_QCA_PLC_UTILS
	reset_plc();
	eval("mtd-erase", "-d", "plc");
#endif

	kill(1, SIGTERM);
}

void handle_notifications(void)
{
	char nv[256], nvtmp[32], *cmd[8], *script;
	char *nvp, *b, *nvptr;
	int action = 0;
	int count;
	int i;
	int wan_unit;
#ifdef RTCONFIG_USB_MODEM
	int modem_unit = 0;
	char tmp2[100], prefix2[32];
	char env_unit[32];
#endif

#if defined(RTCONFIG_LANTIQ)
	f_write_string("/proc/sys/vm/drop_caches", "1", 0, 0);
#endif
	// handle command one by one only
	// handle at most 7 parameters only
	// maximum rc_service strlen is 256
	strcpy(nv, nvram_safe_get("rc_service"));
	nvptr = nv;
again:
	nvp = strsep(&nvptr, ";");

	count = 0;
	while ((b = strsep(&nvp, " ")) != NULL)
	{
		_dprintf("cmd[%d]=%s\n", count, b);
		cmd[count] = b;
		count ++;
		if(count == 7) break;
	}
	cmd[count] = 0;

	if(cmd[0]==0 || strlen(cmd[0])==0) {
		nvram_set("rc_service", "");
		return;
	}

	if(strncmp(cmd[0], "start_", 6)==0) {
		action |= RC_SERVICE_START;
		script = &cmd[0][6];
	}
	else if(strncmp(cmd[0], "stop_", 5)==0) {
		action |= RC_SERVICE_STOP;
		script = &cmd[0][5];
	}
	else if(strncmp(cmd[0], "restart_", 8)==0) {
		action |= (RC_SERVICE_START | RC_SERVICE_STOP);
		script = &cmd[0][8];
	}
	else {
		action = 0;
		script = cmd[0];
	}

	TRACE_PT("running: %d %s\n", action, script);

#ifdef RTCONFIG_USB_MODEM
	if(!strcmp(script, "simauth")
			|| !strcmp(script, "simpin")
			|| !strcmp(script, "simpuk")
			|| !strcmp(script, "lockpin")
			|| !strcmp(script, "pwdpin")
			|| !strcmp(script, "modemscan")
			){
		if(cmd[1] == NULL)
			modem_unit = 0;
		else
			modem_unit = atoi(cmd[1]);

		snprintf(env_unit, 32, "unit=%d", modem_unit);
		putenv(env_unit);

		usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));
	}
#endif

	if (strcmp(script, "reboot") == 0 || strcmp(script,"rebootandrestore")==0) {
		g_reboot = 1;

#ifdef RTCONFIG_QCA_PLC_UTILS
		reset_plc();
		sleep(1);
#endif

		stop_wan();
#ifdef RTCONFIG_USB
#if defined(RTCONFIG_USB_MODEM) && (defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS))
		_dprintf("modem data: save the data during the reboot service\n");
		eval("/usr/sbin/modem_status.sh", "bytes+");
#endif

		if (get_model() == MODEL_RTN53){
			eval("wlconf", "eth2", "down");
			modprobe_r("wl_high");
		}

#if !(defined(RTN56UB1) || defined(RTN56UB2))
#ifndef RTCONFIG_ERPTEST
		stop_usb();
#else
		stop_usb(0);
#endif
#ifndef RTCONFIG_NO_USBPORT
		stop_usbled();
#endif
#endif
#endif
//#if defined(RTCONFIG_JFFS2LOG) && defined(RTCONFIG_JFFS2)
#if defined(RTCONFIG_JFFS2LOG) && (defined(RTCONFIG_JFFS2)||defined(RTCONFIG_BRCM_NAND_JFFS2))
		eval("cp", "/tmp/syslog.log", "/tmp/syslog.log-1", "/jffs");
#endif
		if(strcmp(script,"rebootandrestore")==0) {
			for(i=1;i<count;i++) {
				if(cmd[i]) restore_defaults_module(cmd[i]);
			}
		}

		/* Fall through to signal handler of init process. */
	}
	else if(strcmp(script, "resetdefault_erase") == 0)
	{
		/* remove /jffs/.sys hidden folder */
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
		eval("touch", "/jffs/remove_hidden_flag");
#endif
		factory_reset();
	}
	else if(strcmp(script, "resetdefault") == 0)
	{
		factory_reset();
	}
	else if (strcmp(script, "all") == 0) {
#ifdef RTCONFIG_WIFI_SON
		action |= RC_SERVICE_STOP | RC_SERVICE_START;
		goto script_allnet;
#else
#ifdef RTCONFIG_QCA_PLC_UTILS
		reset_plc();
#endif
		sleep(2); // wait for all httpd event done
		stop_lan_port();
		start_lan_port(6);
		kill(1, SIGTERM);
#endif
	}
	else if(strcmp(script, "upgrade_ate") == 0) {
		FILE *fp;
		int ate_upgrade_reboot;
		int ate_upgrade_reset;
		char upgrade_file[64] = "/tmp/linux.trx";

#ifdef RTCONFIG_SMALL_FW_UPDATE
		snprintf(upgrade_file,sizeof(upgrade_file),"/tmp/mytmpfs/linux.trx");
#endif

		if(cmd[1] == NULL)	ate_upgrade_reboot = 1;
		else			ate_upgrade_reboot = atoi(cmd[1]);
		if(cmd[2] == NULL)	ate_upgrade_reset = 0;
		else			ate_upgrade_reset = atoi(cmd[2]);
		_dprintf("REBOOT = %d, RESET = %d\n", ate_upgrade_reboot, ate_upgrade_reset);
#ifdef CONFIG_BCMWL5
		if (!factory_debug() && !nvram_match(ATE_UPGRADE_MODE_STR(), "1"))
#else
		if (!IS_ATE_FACTORY_MODE() && !nvram_match(ATE_UPGRADE_MODE_STR(), "1"))
#endif
		{
			_dprintf("Only support under ATE test mode, Skip...\n");
			if ((fp = fopen("/tmp/ate_upgrade_state", "w")) != NULL) {
				fprintf(fp, "Not ATE Mode\n");
				fclose(fp);
			}
			else	_dprintf("Fail to open /tmp/ate_upgrade_state\n");

			if (f_exists(upgrade_file))
				unlink(upgrade_file);

			goto skip;
		}

		if(action & RC_SERVICE_STOP) {
			if ((fp = fopen("/tmp/ate_upgrade_state", "w")) != NULL) {
				fprintf(fp, "stop_upgarde_ate\n");
				fclose(fp);
			}
			else	_dprintf("Fail to open /tmp/ate_upgrade_state\n");

#ifdef RTCONFIG_WIRELESSREPEATER
			if(sw_mode() == SW_MODE_REPEATER)
				stop_wlcconnect();
#endif

			stop_hour_monitor_service();

			// what process need to stop to free memory or
			// to avoid affecting upgrade
			//stop_misc();
			stop_logger();
			stop_upnp();
#if defined(RTCONFIG_MDNS)
			stop_mdns();
#endif
			stop_all_webdav();
#ifdef RTCONFIG_CAPTIVE_PORTAL
			stop_uam_srv();
#endif
#if defined(RTN56U)
			stop_if_misc();
#endif
#ifdef RTCONFIG_USB
			/* fix upgrade fail issue : remove wl_high before rmmod ehci_hcd */
			if (get_model() == MODEL_RTAC53U){
				eval("wlconf", "eth1", "down");
				eval("wlconf", "eth2", "down");
				modprobe_r("wl_high");
				modprobe_r("wl");
			}

#if !defined(RTN53) && !defined(RTN56UB1) && !defined(RTN56UB2) && !defined(RTAC1200GA1) && !defined(RTAC1200GU)
#ifndef RTCONFIG_ERPTEST
			stop_usb();
#else
			stop_usb(0);
#endif
#ifndef RTCONFIG_NO_USBPORT
			stop_usbled();
#endif
			remove_storage_main(1);
			remove_usb_module();
#endif

#endif
			remove_conntrack();
			stop_udhcpc(-1);
#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_6RELAYD
			stop_6relayd();
#endif
			stop_dhcp6c();
#endif

#ifdef RTCONFIG_TR069
			stop_tr();
#endif
			stop_jffs2(1);
#ifdef RTCONFIG_JFFS2USERICON
			stop_lltdc();
#endif
			stop_networkmap();

#ifdef RTCONFIG_QCA_PLC_UTILS
			reset_plc();
#endif
#ifdef RTCONFIG_CFGSYNC
			stop_cfgsync();
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_BCMWL6)
			stop_obd();
#endif
		}
		if(action & RC_SERVICE_START) {
			int sw = 0, r;

			if ((fp = fopen("/tmp/ate_upgrade_state", "w")) != NULL) {
				fprintf(fp, "start_upgarde_ate\n");
				fclose(fp);
			}
			else	_dprintf("Fail to open /tmp/ate_upgrade_state\n");

#ifdef RTCONFIG_DSL
#ifdef RTCONFIG_RALINK
			_dprintf("to do start_tc_upgrade\n");
			start_tc_upgrade();
#else
			do_upgrade_adsldrv();
#endif
#endif

			limit_page_cache_ratio(90);

			/* flash it if exists */
			if (f_exists(upgrade_file)) {
#ifdef RTCONFIG_CONCURRENTREPEATER
#ifdef RPAC68U
				set_led(LED_BLINK_SLOW, LED_BLINK_SLOW);
#else
				nvram_set_int("led_status", LED_FIRMWARE_UPGRADE);
#endif
#endif
				/* stop wireless here */
#ifdef RTCONFIG_SMALL_FW_UPDATE
/* TODO should not depend on platform, move to stop_lan_wl()?
 * cope with stop_usb() above for BRCM AP dependencies */
#ifdef CONFIG_BCMWL5
/* TODO should not depend on exact interfaces */
				eval("wlconf", "eth1", "down");
				eval("wlconf", "eth2", "down");
/* TODO fix fini_wl() for BCM USBAP */
				modprobe_r("wl_high");
				modprobe_r("wl");
#ifdef RTCONFIG_USB
#if defined(RTN53)
#ifndef RTCONFIG_ERPTEST
				stop_usb();
#else
				stop_usb(0);
#endif
#ifndef RTCONFIG_NO_USBPORT
				stop_usbled();
#endif
				remove_storage_main(1);
				remove_usb_module();
#endif
#endif
#endif
#elif defined(RTCONFIG_TEMPROOTFS)
				stop_lan_wl();
				stop_dnsmasq();
				stop_networkmap();
				stop_wpsaide();
#if defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK)
#ifdef RTCONFIG_CONCURRENTREPEATER
				stop_wlcconnect();
#endif	//RTCONFIG_CONCURRENTREPEATE
#if defined(RTCONFIG_QCA) || defined(RTCONFIG_LANTIQ)
				stop_wifi_service();
#endif	//RTCONFIG_QCA
#endif
#endif
				if (!(r = build_temp_rootfs(TMP_ROOTFS_MNT_POINT)))
					sw = 1;
#ifdef RTCONFIG_DUAL_TRX
				if (!nvram_match("nflash_swecc", "1"))
				{
					_dprintf(" Write FW to the 2nd partition.\n");
					if (nvram_contains_word("rc_support", "nandflash"))	/* RT-AC56S,U/RT-AC68U/RT-N16UHP */
						eval("mtd-write2", upgrade_file, "linux2");
					else
						eval("mtd-write", "-i", upgrade_file, "-d", "linux2");
				}
#endif
				if (nvram_contains_word("rc_support", "nandflash")) {	/* RT-AC56S,U/RT-AC68U/RT-N16UHP */
#ifdef HND_ROUTER
					eval("hnd-write", upgrade_file);
#elif defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
					update_trx("/tmp/linux.trx");
#else
					eval("mtd-write2", upgrade_file, "linux");
#endif
				}
				else
				{
#if defined(RTAC1200G) || defined(RTAC1200GP)
					eval("mtd-write2", upgrade_file, "linux");
#else
#ifdef RTCONFIG_REALTEK
#ifndef CONFIG_MTD_NAND
					eval("mtd-write", "-i", "/tmp/linux.trx", "-d", "linux");
#else
#ifdef CONFIG_ASUS_DUAL_IMAGE_ENABLE
					eval("mtd-write", "-i", "/tmp/linux.trx", "-d", "linux");
#else
					eval("mtd-write", "-i", "/tmp/linux.trx", "-d", "linux");
					eval("mtd-write", "-i", "/tmp/root.trx", "-d", "rootfs");
#endif
#endif
#else /* !RTCONFIG_REALTEK */
#if defined(RTCONFIG_QCA) && defined(RTCONFIG_FITFDT)
					{
						char *upgrade_file2 = "/tmp/linux2.trx";
						char command_buf[256];
						sprintf(command_buf,"dd if=%s of=%s bs=%d skip=1", upgrade_file, upgrade_file2, get_imageheader_size());
						system(command_buf);
						unlink(upgrade_file);
						eval("mtd-write", "-i", upgrade_file2, "-d", "linux");
					}
#else
					eval("mtd-write", "-i", upgrade_file, "-d", "linux");
#endif /* RTCONFIG_QCA && RTCONFIG_FITFDT */
#endif /* RTCONFIG_REALTEK */
#endif // RTAC1200G
				}

				if ((fp = fopen("/tmp/ate_upgrade_state", "w")) != NULL) {
					fprintf(fp, "Upgarde Complete\n");
					fclose(fp);
				}
				else	_dprintf("Fail to open /tmp/ate_upgrade_state\n");

				/* erase trx and free memory on purpose */
				unlink(upgrade_file);

				if(ate_upgrade_reset) {
					_dprintf("[ATE] Reset Default...\n");
#ifndef HND_ROUTER
					nvram_set("restore_defaults", "1");
					nvram_set(ASUS_STOP_COMMIT, "1");
#endif
					ResetDefault();
				}

				if (sw) {
					_dprintf("switch to temp rootfilesystem\n");
					if (!(r = switch_root(TMP_ROOTFS_MNT_POINT))) {
						/* Do nothing. If switch_root() success, never reach here. */
					} else {
						if(ate_upgrade_reboot)	{
							_dprintf("[ATE] REBOOT...\n");
							kill(1, SIGTERM);
						}
					}
				} else {
					if(ate_upgrade_reboot) {
						_dprintf("[ATE] REBOOT...\n");
						kill(1, SIGTERM);
					}
				}
			}
			else {
				// recover? or reboot directly
				//kill(1, SIGTERM);
				if ((fp = fopen("/tmp/ate_upgrade_state", "w")) != NULL) {
						fprintf(fp, "Can't find firmware image\n");
						fclose(fp);
				}
				else	_dprintf("Fail to open /tmp/ate_upgrade_state\n");

				_dprintf("firmware image not found...\n");

			}
		}
	}
	else if(strcmp(script, "upgrade") == 0) {
		if(action&RC_SERVICE_STOP) {
			g_upgrade = 1;
#ifdef RTCONFIG_WIRELESSREPEATER
			if(sw_mode() == SW_MODE_REPEATER)
			stop_wlcconnect();		
#endif

			stop_hour_monitor_service();
#if defined(RTCONFIG_USB_MODEM) && (defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS))
			_dprintf("modem data: save the data during upgrading\n");
			eval("/usr/sbin/modem_status.sh", "bytes+");
#endif

#ifdef RTCONFIG_USB
			eval("/sbin/ejusb", "-1", "0");
			logmessage("usb", "USB is ejected");
#endif
		   if(!(nvram_match("webs_state_flag", "1") && nvram_match("webs_state_upgrade", "0")))
			stop_wanduck();

			// what process need to stop to free memory or
			// to avoid affecting upgrade
#ifdef RTCONFIG_BWDPI
			/* dpi engine needs 40-50MB memory, it will make some platform run out of memory before upgrade, so stop it to release memory */
			stop_dpi_engine_service(1);
#endif
			stop_misc();
			stop_logger();
			stop_upnp();
#if defined(RTCONFIG_MDNS)
			stop_mdns();
#endif
			stop_all_webdav();
#ifdef RTCONFIG_CAPTIVE_PORTAL
			stop_uam_srv();
#endif
#if defined(RTN56U)
			stop_if_misc();
#endif
#ifdef RTCONFIG_USB
			/* fix upgrade fail issue : remove wl_high before rmmod ehci_hcd */
			if (get_model() == MODEL_RTAC53U){
				eval("wlconf", "eth1", "down");
				eval("wlconf", "eth2", "down");
				modprobe_r("wl_high");
				modprobe_r("wl");
			}

#if !defined(RTN53) && !defined(RTN56UB1) && !defined(RTN56UB2) && !defined(RTAC1200GA1) && !defined(RTAC1200GU)
#ifndef RTCONFIG_ERPTEST
			stop_usb();
#else
			stop_usb(0);
#endif
#ifndef RTCONFIG_NO_USBPORT
			stop_usbled();
#endif
			remove_storage_main(1);
			remove_usb_module();
#endif

#endif
			remove_conntrack();
			stop_udhcpc(-1);
#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_6RELAYD
			stop_6relayd();
#endif
			stop_dhcp6c();
#endif

#ifdef RTCONFIG_TR069
			stop_tr();
#endif
			stop_jffs2(1);
#ifdef RTCONFIG_JFFS2USERICON
			stop_lltdc();
#endif
			stop_networkmap();

#ifdef RTCONFIG_QCA_PLC_UTILS
			reset_plc();
#endif
#ifdef RTCONFIG_CFGSYNC
			stop_cfgsync();
#endif
#ifdef RTCONFIG_AMAS
			stop_amas_lldpd();
#if defined(RTCONFIG_BCMWL6)		
			stop_obd();			
#endif
#endif
			// TODO free necessary memory here
			// Free kernel page cache
			system("echo 1 > /proc/sys/vm/drop_caches");
			sleep(2);
		}
		if(action & RC_SERVICE_START) {
			int sw = 0, r;
			char upgrade_file[64] = "/tmp/linux.trx";
			char *webs_state_info = nvram_safe_get("webs_state_info");

#ifdef RTCONFIG_SMALL_FW_UPDATE
			snprintf(upgrade_file,sizeof(upgrade_file),"/tmp/mytmpfs/linux.trx");
#endif

		#ifdef RTCONFIG_DSL
		#ifdef RTCONFIG_RALINK
			_dprintf("to do start_tc_upgrade\n");
			start_tc_upgrade();
		#else
			do_upgrade_adsldrv();
		#endif
		#endif

			limit_page_cache_ratio(90);

			/* /tmp/linux.trx has the priority */
			if (!f_exists(upgrade_file) && strlen(webs_state_info) > 5) {
				snprintf(upgrade_file, sizeof(upgrade_file),
					"/tmp/%s_%c.%c.%c.%c_%s.trx",
					nvram_safe_get("productid"),
					webs_state_info[0],
					webs_state_info[1],
					webs_state_info[2],
					webs_state_info[3],
					webs_state_info+5);
				_dprintf("upgrade file : %s \n", upgrade_file);
			}

			/* flash it if exists */
			if (f_exists(upgrade_file)) {
#ifdef RTCONFIG_CONCURRENTREPEATER
#ifdef RPAC68U
				set_led(LED_BLINK_SLOW, LED_BLINK_SLOW);
#else
				nvram_set_int("led_status", LED_FIRMWARE_UPGRADE);
#endif
#endif
				/* stop wireless here */
#ifdef RTCONFIG_SMALL_FW_UPDATE
/* TODO should not depend on platform, move to stop_lan_wl()?
 * cope with stop_usb() above for BRCM AP dependencies */
#ifdef CONFIG_BCMWL5
/* TODO should not depend on exact interfaces */
				eval("wlconf", "eth1", "down");
				eval("wlconf", "eth2", "down");
/* TODO fix fini_wl() for BCM USBAP */
				modprobe_r("wl_high");
				modprobe_r("wl");
#ifdef RTCONFIG_USB
#if defined(RTN53)
#ifndef RTCONFIG_ERPTEST
				stop_usb();
#else
				stop_usb(0);
#endif
#ifndef RTCONFIG_NO_USBPORT
				stop_usbled();
#endif
				remove_storage_main(1);
				remove_usb_module();
#endif
#endif
#endif
#elif defined(RTCONFIG_TEMPROOTFS)
				stop_lan_wl();
				stop_dnsmasq();
				stop_networkmap();
				stop_wpsaide();
#if defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK)
#ifdef RTCONFIG_CONCURRENTREPEATER					
				stop_wlcconnect();			
#endif	//RTCONFIG_CONCURRENTREPEATE
#if defined(RTCONFIG_QCA)
				stop_wifi_service();
#endif	//RTCONFIG_QCA
#ifdef RTCONFIG_AMAS
				stop_amas_lldpd();
#endif
#endif
#endif
				if (!(r = build_temp_rootfs(TMP_ROOTFS_MNT_POINT)))
					sw = 1;
#ifdef RTCONFIG_DUAL_TRX
				if (!nvram_match("nflash_swecc", "1"))
				{
					_dprintf(" Write FW to the 2nd partition.\n");
					if (nvram_contains_word("rc_support", "nandflash"))     /* RT-AC56S,U/RT-AC68U/RT-N16UHP */
						eval("mtd-write2", upgrade_file, "linux2");
					else
						eval("mtd-write", "-i", upgrade_file, "-d", "linux2");
				}
#endif
				if (nvram_contains_word("rc_support", "nandflash")) {	/* RT-AC56S,U/RT-AC68U/RT-N16UHP */
#ifdef HND_ROUTER
					eval("hnd-write", upgrade_file);
#elif defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
					update_trx("/tmp/linux.trx");
#else
					eval("mtd-write2", upgrade_file, "linux");
#endif
				}
				else
				{
#if defined(RTAC1200G) || defined(RTAC1200GP)
					eval("mtd-write2", upgrade_file, "linux");
#else
#ifdef RTCONFIG_REALTEK
#ifndef CONFIG_MTD_NAND
					eval("mtd-write", "-i", "/tmp/linux.trx", "-d", "linux");
#else
#ifdef CONFIG_ASUS_DUAL_IMAGE_ENABLE
					eval("mtd-write", "-i", "/tmp/linux.trx", "-d", "linux");
#else
					eval("mtd-write", "-i", "/tmp/linux.trx", "-d", "linux");
					eval("mtd-write", "-i", "/tmp/root.trx", "-d", "rootfs");
#endif
#endif
#else /* !RTCONFIG_REALTEK */
#if defined(RTCONFIG_QCA) && defined(RTCONFIG_FITFDT)
					{
						char *upgrade_file2 = "/tmp/linux2.trx";
						char command_buf[256];
						sprintf(command_buf,"dd if=%s of=%s bs=%d skip=1", upgrade_file, upgrade_file2, get_imageheader_size());
						system(command_buf);
						unlink(upgrade_file);
						eval("mtd-write", "-i", upgrade_file2, "-d", "linux");
					}
#else
					eval("mtd-write", "-i", upgrade_file, "-d", "linux");
#endif /* RTCONFIG_QCA && RTCONFIG_FITFDT */
#endif /* RTCONFIG_REALTEK */
#endif // RTAC1200G
				}
				/* erase trx and free memory on purpose */
				unlink(upgrade_file);
				if (sw) {
					_dprintf("switch to temp rootfilesystem\n");
					if (!(r = switch_root(TMP_ROOTFS_MNT_POINT))) {
						/* Do nothing. If switch_root() success, never reach here. */
					} else {
						kill(1, SIGTERM);
					}
				} else {
					kill(1, SIGTERM);
				}
			}
			else {
				// recover? or reboot directly
				kill(1, SIGTERM);
			}
		}
	}
	else if(strcmp(script, "mfgmode") == 0) {
		nvram_set("asus_mfg", "2");
		stop_services_mfg();
	}
	else if(strcmp(script, "wltest") == 0) {
		nvram_set("asus_mfg", "3");
		stop_watchdog();
		stop_infosvr();
		stop_services_mfg();
	}
	else if (strcmp(script, "allnet") == 0) {
#ifdef RTCONFIG_WIFI_SON
script_allnet:
#endif
		if(action&RC_SERVICE_STOP) {
			// including switch setting
			// used for system mode change and vlan setting change
			sleep(2); // wait for all httpd event done
#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_PRINTER)
			stop_lpd();
			stop_u2ec();
#endif
#ifdef RTCONFIG_JFFS2USERICON
			stop_lltdc();
#endif
			stop_networkmap();
			stop_httpd();
			stop_telnetd();
#ifdef RTCONFIG_SSH
			stop_sshd();
#endif
			stop_dnsmasq();
#if defined(RTCONFIG_MDNS)
			stop_mdns();
#endif
			stop_upnp();
			stop_lltd();
			stop_snooper();
#ifdef RTCONFIG_BCMWL6
#ifdef BCM_ASPMD
			stop_aspmd();
#endif
#ifdef RTCONFIG_DHDAP
			stop_dhd_monitor();
#endif
			stop_acsd();
#ifdef BCM_EVENTD
			stop_eventd();
#endif
#ifdef BCM_SSD
			stop_ssd();
#endif
#ifdef BCM_APPEVENTD
			stop_appeventd();
#endif
#ifdef BCM_BSD
			stop_bsd();
#endif
			stop_igmp_proxy();
#ifdef RTCONFIG_HSPOT
			stop_hspotap();
#endif
#endif
#if defined(RTCONFIG_WLCEVENTD)
			stop_wlceventd();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
			stop_wlc_nt();
#endif
			stop_wps();
#ifdef CONFIG_BCMWL5
			stop_nas();
			stop_eapd();
#elif defined RTCONFIG_RALINK
			stop_8021x();
#endif
			stop_wan();
			stop_lan();
#ifdef RTCONFIG_DSL_TCLINUX
			stop_dsl();
#endif
			stop_vlan();
#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_PRINTER)
			stop_lpd();
			stop_u2ec();
#endif
#ifdef RTCONFIG_CFGSYNC
			stop_cfgsync();
#endif
#ifdef RTCONFIG_CAPTIVE_PORTAL
			stop_chilli();
			stop_CP();
			stop_uam_srv();
#endif

			// TODO free memory here
		}
		if(action & RC_SERVICE_START) {
#if defined(RTCONFIG_RALINK_MT7628)
			start_vlan();
			config_switch();
#else
#if !defined(HND_ROUTER) && !defined(BLUECAVE)
			config_switch();
			start_vlan();
#endif
#endif
#ifdef RTCONFIG_DSL_TCLINUX
			start_dsl();
#endif
			start_lan();
			start_dnsmasq();
#if defined(RTCONFIG_MDNS)
			start_mdns();
#endif
#if defined(HND_ROUTER) || defined(BLUECAVE)
			start_vlan();
#endif
			start_wan();
#ifdef HND_ROUTER
			if (is_router_mode()) start_mcpd_proxy();
#endif
#ifdef RTCONFIG_USB_MODEM
			for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit)
				if(dualwan_unit__usbif(wan_unit) &&
						(wan_unit == wan_primary_ifunit()
#ifdef RTCONFIG_DUALWAN
								|| nvram_match("wans_mode", "lb")
#endif
								)
						)
					start_wan_if(wan_unit);
#endif
#ifdef CONFIG_BCMWL5
			start_eapd();
			start_nas();
#elif defined RTCONFIG_RALINK
			start_8021x();
#endif
			start_wps();
#ifdef RTCONFIG_NOTIFICATION_CENTER
			start_wlc_nt();
#endif
#if defined(RTCONFIG_WLCEVENTD)
			start_wlceventd();
#endif
#ifdef RTCONFIG_BCMWL6
			start_igmp_proxy();
#ifdef BCM_BSD
			start_bsd();
#endif
#ifdef BCM_APPEVENTD
			start_appeventd();
#endif
#ifdef BCM_SSD
			start_ssd();
#endif
#ifdef BCM_EVENTD
			start_eventd();
#endif
			start_acsd();
#ifdef RTCONFIG_DHDAP
			start_dhd_monitor();
#endif
#ifdef BCM_ASPMD
			start_aspmd();
#endif
#endif
			start_snooper();
			start_lltd();
			/* Link-up LAN ports after DHCP server ready. */
			start_lan_port(0);

			start_upnp();

			start_httpd();
			start_telnetd();
#ifdef RTCONFIG_SSH
			start_sshd();
#endif
#ifdef RTCONFIG_JFFS2USERICON
			start_lltdc();
#endif
			start_networkmap(0);
#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_PRINTER)
			start_usblpsrv();
#endif
			start_wl();
			lanaccess_wl();
#ifdef RTCONFIG_CAPTIVE_PORTAL
			start_chilli();
			start_CP();
			start_uam_srv();
#endif
#ifdef RTCONFIG_BCMWL6
#ifdef RTCONFIG_HSPOT
			start_hspotap();
#endif
#endif
#ifdef RTCONFIG_CFGSYNC
			start_cfgsync();
#endif
		}
	}
	else if (strcmp(script, "net") == 0) {
		if(action & RC_SERVICE_STOP) {
			sleep(2); // wait for all httpd event done
#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_PRINTER)
			stop_lpd();
			stop_u2ec();
#endif
#ifdef RTCONFIG_JFFS2USERICON
			stop_lltdc();
#endif
			stop_networkmap();
			stop_httpd();
			stop_telnetd();
#ifdef RTCONFIG_SSH
			stop_sshd();
#endif
			stop_dnsmasq();
#if defined(RTCONFIG_MDNS)
			stop_mdns();
#endif
			stop_upnp();
			stop_lltd();
			stop_snooper();
#ifdef RTCONFIG_BCMWL6
#ifdef BCM_ASPMD
			stop_aspmd();
#endif
#ifdef RTCONFIG_DHDAP
			stop_dhd_monitor();
#endif
			stop_acsd();
#ifdef BCM_EVENTD
			stop_eventd();
#endif
#ifdef BCM_SSD
			stop_ssd();
#endif
#ifdef BCM_APPEVENTD
			stop_appeventd();
#endif
#ifdef BCM_BSD
			stop_bsd();
#endif
			stop_igmp_proxy();
#ifdef RTCONFIG_HSPOT
			stop_hspotap();
#endif
#endif
#if defined(RTCONFIG_WLCEVENTD)
			stop_wlceventd();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
			stop_wlc_nt();
#endif
			stop_wps();
#ifdef CONFIG_BCMWL5
			stop_nas();
			stop_eapd();
#elif defined RTCONFIG_RALINK
			stop_8021x();
#endif
#ifdef RTCONFIG_CFGSYNC
			stop_cfgsync();
#endif
			stop_wan();
			stop_lan();
 			//stop_vlan();

#ifdef RTCONFIG_CAPTIVE_PORTAL
			stop_chilli();
			stop_CP();
			stop_uam_srv();
#endif

			// free memory here
		}
		if(action & RC_SERVICE_START) {
			//start_vlan();
			start_lan();
			start_dnsmasq();
#if defined(RTCONFIG_MDNS)
			start_mdns();
#endif
			start_wan();
#ifdef HND_ROUTER
			if (is_router_mode()) start_mcpd_proxy();
#endif
#ifndef RTCONFIG_INTERNAL_GOBI
#ifdef RTCONFIG_USB_MODEM
			for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit)
				if(dualwan_unit__usbif(wan_unit) &&
						(wan_unit == wan_primary_ifunit()
#ifdef RTCONFIG_DUALWAN
								|| nvram_match("wans_mode", "lb")
#endif
								)
						)
					start_wan_if(wan_unit);
#endif
#endif
#ifdef CONFIG_BCMWL5
			start_eapd();
			start_nas();
#elif defined RTCONFIG_RALINK
			start_8021x();
#endif
			start_wps();
#ifdef RTCONFIG_NOTIFICATION_CENTER
			start_wlc_nt();
#endif
#if defined(RTCONFIG_WLCEVENTD)
			start_wlceventd();
#endif
#ifdef RTCONFIG_BCMWL6
			start_igmp_proxy();
#ifdef BCM_BSD
			start_bsd();
#endif
#ifdef BCM_APPEVENTD
			start_appeventd();
#endif
#ifdef BCM_SSD
			start_ssd();
#endif
#ifdef BCM_EVENTD
			start_eventd();
#endif
			start_acsd();
#ifdef RTCONFIG_DHDAP
			start_dhd_monitor();
#endif
#ifdef BCM_ASPMD
			start_aspmd();
#endif
#endif
			start_snooper();
			start_lltd();
			/* Link-up LAN ports after DHCP server ready. */
			start_lan_port(0);

			start_upnp();

			start_httpd();
			start_telnetd();
#ifdef RTCONFIG_SSH
			start_sshd();
#endif
#ifdef RTCONFIG_JFFS2USERICON
			start_lltdc();
#endif
			start_networkmap(0);
#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_PRINTER)
			start_usblpsrv();
#endif
#ifdef RTCONFIG_CAPTIVE_PORTAL
			start_chilli();
			start_CP();
			start_uam_srv();
#endif
			start_wl();
			lanaccess_wl();
#ifdef RTCONFIG_BCMWL6
#ifdef RTCONFIG_HSPOT
			start_hspotap();
#endif
#endif
#ifdef RTCONFIG_CFGSYNC
			start_cfgsync();
#endif
		}
	}
	else if (strcmp(script, "net_and_phy") == 0) {
	script_net_and_phy:
		if(action & RC_SERVICE_STOP) {
			sleep(2); // wait for all httpd event done

#ifdef RTCONFIG_QCA_PLC_UTILS
			stop_plchost();
#endif
#ifdef RTCONFIG_MEDIA_SERVER
			force_stop_dms();
			stop_mt_daapd();
#endif
#if defined(RTCONFIG_SAMBASRV) && defined(RTCONFIG_FTP)
			stop_ftpd();
			stop_samba();
#endif
#ifdef RTCONFIG_LANTIQ
#ifdef RTCONFIG_WIRELESSREPEATER
			if(client_mode()){
				nvram_set("restart_wifi", "1");
				stop_wlcconnect();
				kill_pidfile_s("/var/run/wanduck.pid", SIGUSR1);
			}
#endif
#endif
#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_PRINTER)
			stop_lpd();
			stop_u2ec();
#endif
#ifdef RTCONFIG_JFFS2USERICON
			stop_lltdc();
#endif
			stop_networkmap();
			stop_httpd();
			stop_telnetd();
#ifdef RTCONFIG_SSH
			stop_sshd();
#endif
#ifdef RTCONFIG_DHCP_OVERRIDE
			stop_detectWAN_arp();
#endif
			stop_dnsmasq();
#if defined(RTCONFIG_MDNS)
			stop_mdns();
#endif
			//Andy Chiu, 2015/09/16.
			stop_upnp();
			stop_lltd();
			stop_snooper();
#ifdef RTCONFIG_BCMWL6
#ifdef BCM_ASPMD
			stop_aspmd();
#endif
#ifdef RTCONFIG_DHDAP
			stop_dhd_monitor();
#endif
			stop_acsd();
#ifdef BCM_EVENTD
			stop_eventd();
#endif
#ifdef BCM_SSD
			stop_ssd();
#endif
#ifdef BCM_APPEVENTD
			stop_appeventd();
#endif
#ifdef BCM_BSD
			stop_bsd();
#endif
			stop_igmp_proxy();
#ifdef RTCONFIG_HSPOT
			stop_hspotap();
#endif
#endif
#if defined(RTCONFIG_WLCEVENTD)
			stop_wlceventd();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
			stop_wlc_nt();
#endif
			stop_wps();
#ifdef CONFIG_BCMWL5
			stop_nas();
			stop_eapd();
#elif defined RTCONFIG_RALINK
			stop_8021x();
			stop_lan_wl();
#endif

#ifdef RTCONFIG_CAPTIVE_PORTAL
			stop_chilli();
			stop_CP();
			stop_uam_srv();
#endif
#ifdef RTCONFIG_FREERADIUS
			stop_radiusd();
#endif

#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
			//stop_pptpd();
#endif
#ifdef RTCONFIG_CFGSYNC
			stop_cfgsync();
#endif
			stop_wan();
			stop_lan();
			//stop_vlan();
			stop_lan_port();

			// free memory here
		}
#ifdef RTCONFIG_LANTIQ
		if((action & RC_SERVICE_STOP) && (action & RC_SERVICE_START)) {
			if(client_mode())
				restart_wireless();
		}
#endif
		if(action & RC_SERVICE_START) {
			//start_vlan();
#ifdef RTCONFIG_LACP
			config_lacp();
#endif
			start_lan();
			start_dnsmasq();
#ifdef RTCONFIG_DHCP_OVERRIDE
			start_detectWAN_arp();
#endif
#if defined(RTCONFIG_MDNS)
			start_mdns();
#endif
			start_wan();
#ifdef HND_ROUTER
			if (is_router_mode()) start_mcpd_proxy();
#endif
#ifdef RTCONFIG_USB_MODEM
			for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit)
				if(dualwan_unit__usbif(wan_unit) &&
						(wan_unit == wan_primary_ifunit()
#ifdef RTCONFIG_DUALWAN
								|| nvram_match("wans_mode", "lb")
#endif
								)
						)
					start_wan_if(wan_unit);
#endif
#ifdef CONFIG_BCMWL5
			start_eapd();
			start_nas();
#elif defined RTCONFIG_RALINK
			start_lan_wl();
			start_8021x();
#endif
#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
			//start_pptpd();
#endif
			start_wps();
#ifdef RTCONFIG_NOTIFICATION_CENTER
			start_wlc_nt();
#endif
#if defined(RTCONFIG_WLCEVENTD)
			start_wlceventd();
#endif
#ifdef RTCONFIG_BCMWL6
			start_igmp_proxy();
#ifdef BCM_BSD
			start_bsd();
#endif
#ifdef BCM_APPEVENTD
			start_appeventd();
#endif
#ifdef BCM_SSD
			start_ssd();
#endif
#ifdef BCM_EVENTD
			start_eventd();
#endif
			start_acsd();
#ifdef RTCONFIG_DHDAP
			start_dhd_monitor();
#endif
#ifdef BCM_ASPMD
			start_aspmd();
#endif
#endif
			start_snooper();
			start_lltd();
			/* Link-up LAN ports after DHCP server ready. */
			start_lan_port(6);

			//Andy Chiu, 2015/09/16
			start_upnp();

			start_httpd();
			start_telnetd();
#ifdef RTCONFIG_SSH
			start_sshd();
#endif
#ifdef RTCONFIG_JFFS2USERICON
			start_lltdc();
#endif
			start_networkmap(0);
#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_PRINTER)
			start_usblpsrv();
#endif
#ifdef RTCONFIG_FREERADIUS
			start_radiusd();
#endif

#if defined(RTCONFIG_SAMBASRV) && defined(RTCONFIG_FTP)
			setup_passwd();
			start_samba();
			start_ftpd();
#endif
			start_wl();
			lanaccess_wl();
#ifdef RTCONFIG_CAPTIVE_PORTAL
			start_chilli();
			start_CP();
			start_uam_srv();
#endif
#ifdef RTCONFIG_BCMWL6
#ifdef RTCONFIG_HSPOT
			start_hspotap();
#endif
#endif
#ifdef RTCONFIG_MEDIA_SERVER
			start_dms();
			start_mt_daapd();
#endif
#ifdef RTCONFIG_QCA_PLC_UTILS
			start_plchost();
#endif
#ifdef RTCONFIG_CFGSYNC
			start_cfgsync();
#endif
#ifdef RTCONFIG_LANTIQ
			if(client_mode()){
				start_ecoguard(); //for app eco mode

#ifdef RTCONFIG_WIRELESSREPEATER
				start_wlcconnect();
#endif
				nvram_set("restart_wifi", "0");
			}
#endif
		}
	}
	else if (!strcmp(script, "subnet")) {
		struct in_addr addr;
		in_addr_t new_addr;
#if defined(RTCONFIG_TAGGED_BASED_VLAN)
		int r;
		char ip_mask[32], new_ip[sizeof("192.168.100.200XXX")], *p;
		char *lan_netmask;

		lan_netmask = nvram_safe_get("lan_netmask");
		snprintf(ip_mask, sizeof(ip_mask), "%s/%s", nvram_safe_get("lan_ipaddr"), lan_netmask);
		r = test_and_get_free_char_network(7, ip_mask, EXCLUDE_NET_LAN);
		if (r == 1) {
			p = strchr(ip_mask, '/');
			strlcpy(new_ip, ip_mask, min(p - ip_mask, sizeof(new_ip)));
			strlcat(new_ip, "1", sizeof(new_ip));
			inet_aton(new_ip , &addr);
		}
		else
			goto skip;
#else		
		char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
		/* no multilan support so far *//*
		char prefix_lan[sizeof("lanXXXXXXXXXX_")]; */
		char *lan_ipaddr, *lan_netmask;
		char *wan_ipaddr, *wan_netmask;
		int unit;

		lan_ipaddr = nvram_safe_get("lan_ipaddr");
		lan_netmask = nvram_safe_get("lan_netmask");

		if(nvram_match("wans_mode", "lb")){
			for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
				if (!dualwan_unit__usbif(unit)) {
					snprintf(prefix, sizeof(prefix), "wan%d_", unit);
					wan_ipaddr = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));
					wan_netmask = nvram_safe_get(strcat_r(prefix, "netmask", tmp));
				}
				else{
					/* force conflict per original design */
					wan_ipaddr = lan_ipaddr;
					wan_netmask = lan_netmask;
				}

				if (inet_deconflict(lan_ipaddr, lan_netmask, wan_ipaddr, wan_netmask, &addr))
					lan_ipaddr = inet_ntoa(addr);
			}

			if (!strcmp(lan_ipaddr, nvram_safe_get("lan_ipaddr")))
				goto skip;
		}
		else{
			unit = get_primaryif_dualwan_unit();
			if (unit < 0)
				goto skip;

			/* no multilan support so far *//*
			snprintf(prefix_lan, sizeof(prefix_lan), "lan_");
			lan_ipaddr = nvram_safe_get(strcat_r(prefix_lan, "ipaddr", tmp));
			lan_netmask = nvram_safe_get(strcat_r(prefix_lan, "netmask", tmp)); */
			//lan_ipaddr = nvram_safe_get("lan_ipaddr");
			//lan_netmask = nvram_safe_get("lan_netmask");

			if (!dualwan_unit__usbif(unit)) {
				snprintf(prefix, sizeof(prefix), "wan%d_", unit);
				wan_ipaddr = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));
				wan_netmask = nvram_safe_get(strcat_r(prefix, "netmask", tmp));
			} else {
				/* force conflict per original design */
				wan_ipaddr = lan_ipaddr;
				wan_netmask = lan_netmask;
			}

			if (!inet_deconflict(lan_ipaddr, lan_netmask, wan_ipaddr, wan_netmask, &addr))
				goto skip;
		}
#endif

		/* no multilan support so far *//*
		nvram_set(strcat_r(prefix_lan, "ipaddr", tmp), inet_ntoa(addr));
		nvram_set(strcat_r(prefix_lan, "ipaddr_rt", tmp), inet_ntoa(addr)); */ // Sync to lan_ipaddr_rt, added by jerry5.
		nvram_set("lan_ipaddr", inet_ntoa(addr));
		nvram_set("lan_ipaddr_rt", inet_ntoa(addr));

		new_addr = ntohl(addr.s_addr);
		addr.s_addr = htonl(new_addr + 1);
		nvram_set("dhcp_start", inet_ntoa(addr));
		addr.s_addr = htonl((new_addr | ~inet_network(lan_netmask)) & 0xfffffffe);
		nvram_set("dhcp_end", inet_ntoa(addr));
		nvram_commit();
		nvram_set("freeze_duck", "15");

		/* direct restart_net_and_phy call */
		action |= RC_SERVICE_STOP | RC_SERVICE_START;
		goto script_net_and_phy;
	}
#ifdef RTCONFIG_DUALWAN
	else if(!strcmp(script, "multipath")){
		char mode[4], if_now[16], if_next[16];
		int unit_now = wan_primary_ifunit();
		int unit_next = (unit_now+1)%WAN_UNIT_MAX;
		int state_now = is_wan_connect(unit_now);
		int state_next = is_wan_connect(unit_next);

		snprintf(mode, 4, "%s", nvram_safe_get("wans_mode"));
		snprintf(if_now, 16, "%s", get_wan_ifname(unit_now));
		snprintf(if_next, 16, "%s", get_wan_ifname(unit_next));
_dprintf("multipath(%s): unit_now: (%d, %d, %s), unit_next: (%d, %d, %s).\n", mode, unit_now, state_now, if_now, unit_next, state_next, if_next);

		if(!strcmp(mode, "lb")){
			if(state_now == 1 && state_next == 1){
				wan_up(if_now);
			}
			else if(state_now == 1 && state_next == 0){
				stop_wan_if(unit_next);
				start_wan_if(unit_next);
			}
			else if(state_now == 0 && state_next == 1){
				wan_up(if_next);
				stop_wan_if(unit_now);
				start_wan_if(unit_now);
			}
			else{ // state_now == 0 && state_next == 0
				for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit){
					stop_wan_if(wan_unit);
					start_wan_if(wan_unit);
				}
			}
		}
		else if(!strcmp(mode, "fb")){
			if(state_now == 1 && state_next == 1){
				if(unit_next == WAN_UNIT_FIRST){
					set_wan_primary_ifunit(unit_next);
					wan_up(if_next);
					stop_wan_if(unit_now);
				}
				else{ // unit_now == WAN_UNIT_FIRST
					wan_up(if_now);
					stop_wan_if(unit_next);
				}
			}
			else if(state_now == 1 && state_next == 0){
				wan_up(if_now);
				if(unit_next == WAN_UNIT_FIRST){
					stop_wan_if(unit_next);
					start_wan_if(unit_next);
				}
			}
			else if(state_now == 0 && state_next == 1){
				set_wan_primary_ifunit(unit_next);
				wan_up(if_next);
				if(unit_now == WAN_UNIT_FIRST){
					stop_wan_if(unit_now);
					start_wan_if(unit_now);
				}
			}
			else{ // state_now == 0 && state_next == 0
				if(unit_next == WAN_UNIT_FIRST){
					set_wan_primary_ifunit(unit_next);
					stop_wan_if(unit_next);
					start_wan_if(unit_next);
				}
				else{
					stop_wan_if(unit_now);
					start_wan_if(unit_now);
				}
			}
		}
		else if(!strcmp(mode, "fo")){
			if(state_now == 1 && state_next == 1){
				wan_up(if_now);
				stop_wan_if(unit_next);
			}
			else if(state_now == 1 && state_next == 0){
				wan_up(if_now);
			}
			else if(state_now == 0 && state_next == 1){
				set_wan_primary_ifunit(unit_next);
				wan_up(if_next);
			}
			else{ // state_now == 0 && state_next == 0
				stop_wan_if(unit_now);
				start_wan_if(unit_now);
			}
		}
	}
#endif
	else if (strcmp(script, "wireless") == 0) {
		nvram_set("restart_wifi", "1");
		if(action&RC_SERVICE_STOP) {
#ifdef RTCONFIG_ADTBW
			stop_adtbw();
#endif

#ifdef RTCONFIG_AMAS
			stop_amas_wlcconnect();
#else
#ifdef RTCONFIG_WIRELESSREPEATER
			stop_wlcconnect();
#endif
#endif
			kill_pidfile_s("/var/run/wanduck.pid", SIGUSR1);

#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_PRINTER)
			stop_lpd();
			stop_u2ec();
#endif
			stop_networkmap();
		}
		if((action & RC_SERVICE_STOP) && (action & RC_SERVICE_START)) {
			// TODO: free memory here
#if defined(RTCONFIG_QCA) || \
		(defined(RTCONFIG_RALINK) && !defined(RTCONFIG_DSL) && !defined(RTN13U))
			reinit_hwnat(-1);
#endif
			restart_wireless();
		}
		if(action & RC_SERVICE_START) {
			start_networkmap(0);
#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_PRINTER)
			start_usblpsrv();
#endif

#ifdef RTCONFIG_ADTBW
			start_adtbw();
#endif
			start_ecoguard(); //for app eco mode
			
#ifdef RTCONFIG_AMAS
			start_amas_wlcconnect();
#else
#ifdef RTCONFIG_WIRELESSREPEATER
			start_wlcconnect();
#endif
#endif
		}
		setup_leds();
		nvram_set("restart_wifi", "0");
	}
#if defined(RTCONFIG_POWER_SAVE)
	else if (!strcmp(script, "pwrsave")) {
		set_power_save_mode();
	}
#endif
#ifdef CONFIG_BCMWL5
#ifdef RTCONFIG_BCMWL6A
	else if (strcmp(script, "clkfreq") == 0) {
		dbG("clkfreq: %s\n", nvram_safe_get("clkfreq"));

		char *string = nvram_safe_get("clkfreq");
		char *cpu, *ddr, buf[100];
		unsigned int cpu_clock = 0, ddr_clock = 0;
		static unsigned int cpu_clock_table[] = {600, 800, 1000, 1200, 1400, 1600};
		static unsigned int ddr_clock_table[] = {333, 389, 400, 533, 666, 775, 800};

		if (strchr(string, ','))
		{
			strncpy(ddr = buf, string, sizeof(buf));
			cpu = strsep(&ddr, ",");
			cpu_clock=atoi(cpu);
			ddr_clock=atoi(ddr);
		}
		else
			cpu_clock=atoi(string);


		for (i = 0; i < (sizeof(cpu_clock_table)/sizeof(cpu_clock_table[0])); i++)
		{
			if (cpu_clock == cpu_clock_table[i])
				goto check_ddr_clock;
		}
		cpu_clock = 800;
check_ddr_clock:
		for (i = 0; i < (sizeof(ddr_clock_table)/sizeof(ddr_clock_table[0])); i++)
		{
			if (ddr_clock == ddr_clock_table[i])
				goto check_ddr_done;
		}
		ddr_clock = 533;
check_ddr_done:
		if (cpu_clock) dbG("target CPU clock: %d\n", cpu_clock);
		if (ddr_clock) dbG("target DDR clock: %d\n", ddr_clock);

		nvram_unset("sdram_ncdl");
		nvram_commit();
	}
#endif
	else if (strcmp(script, "set_wltxpower") == 0) {
		switch (get_model()) {
		case MODEL_RTAC66U:
		case MODEL_RTAC56S:
		case MODEL_RTAC56U:
		case MODEL_RTAC3200:
		case MODEL_RTAC68U:
		case MODEL_DSLAC68U:
		case MODEL_RTAC87U:
		case MODEL_RTN12HP:
		case MODEL_RTN12HP_B1:
		case MODEL_APN12HP:
		case MODEL_RTN66U:
		case MODEL_RTN18U:
		case MODEL_RTAC5300:
		case MODEL_GTAC5300:
		case MODEL_RTAC3100:
		case MODEL_RTAC88U:
		case MODEL_RTAC86U:
			set_wltxpower();
			break;
		default:
			dbG("\n\tDon't do this!\n\n");
			break;
		}
	}
#endif
#ifdef RTCONFIG_FANCTRL
	else if (strcmp(script, "fanctrl") == 0) {
		if((action & RC_SERVICE_STOP)&&(action & RC_SERVICE_START)) restart_fanctrl();
	}
#endif
	else if (strcmp(script, "wan") == 0) {
		if(action & RC_SERVICE_STOP)
		{
			stop_upnp();
			stop_wan();
		}
		if(action & RC_SERVICE_START)
		{
			start_wan();
#ifdef HND_ROUTER
			if (is_router_mode()) start_mcpd_proxy();
#endif
			start_upnp();
		}
	}
	else if (strcmp(script, "wan_if") == 0) {
#ifdef RTCONFIG_IPV6
		int restart_ipv6 = atoi(cmd[1]) == wan_primary_ifunit_ipv6();
#endif
		_dprintf("%s: wan_if: %s.\n", __FUNCTION__, cmd[1]);
		if(cmd[1]) {
			if(action & RC_SERVICE_STOP)
			{
				stop_wan_if(atoi(cmd[1]));
#ifdef RTCONFIG_IPV6
				if (restart_ipv6)
					stop_lan_ipv6();
#endif
			}
			if(action & RC_SERVICE_START)
			{
#if defined(RTCONFIG_DETWAN)
				extern void detwan_set_net_block(int add);
				detwan_set_net_block(0);
#endif	/* RTCONFIG_DETWAN */
#ifdef RTCONFIG_IPV6
				if (restart_ipv6)
					start_lan_ipv6();
#endif
#ifdef DSL_AC68U	//Andy Chiu, 2015/09/15.
				//Check the vlan config of ethernet wan, reset the config by new vlan id.
				check_wan_if(atoi(cmd[1]));

#endif
				start_wan_if(atoi(cmd[1]));
#if defined(RTCONFIG_IPV6) && defined(RTCONFIG_DUALWAN)
				restart_dnsmasq_ipv6();
#endif
			}
		}
	}
#ifdef RTCONFIG_DSL
	else if (strcmp(script, "dslwan_if") == 0) {
		_dprintf("%s: restart_dslwan_if: %s.\n", __FUNCTION__, cmd[1]);
		if(cmd[1]) {
			if(action & RC_SERVICE_STOP)
			{
				stop_wan_if(atoi(cmd[1]));
			}
			if(action & RC_SERVICE_START)
			{
				remove_dsl_autodet();
				dsl_configure(2);
				start_wan_if(atoi(cmd[1]));
			}
		}
	}
	else if (strcmp(script, "dsl_wireless") == 0) {
		if(action&RC_SERVICE_STOP) {
#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_PRINTER)
			stop_lpd();
			stop_u2ec();
#endif
			stop_networkmap();
		}
		if((action&RC_SERVICE_STOP) && (action & RC_SERVICE_START)) {
// qis
			restart_wireless();
			remove_dsl_autodet();
			stop_wan_if(atoi(cmd[1]));
			dsl_configure(2);
			start_wan_if(atoi(cmd[1]));
		}
		if(action & RC_SERVICE_START) {
			start_networkmap(0);
#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_PRINTER)
			start_usblpsrv();
#endif
		}
	}
	else if (strcmp(script, "dsl_setting") == 0) {
		if((action & RC_SERVICE_STOP) && (action & RC_SERVICE_START)) {
			eval("req_dsl_drv", "dslsetting");
		}
	}
#ifdef RTCONFIG_DSL_TCLINUX
	else if (strcmp(script, "dsl_autodet") == 0) {
		if(action & RC_SERVICE_STOP) stop_dsl_autodet();
		if(action & RC_SERVICE_START) start_dsl_autodet();
	}
	else if (strcmp(script, "dsl_diag") == 0) {
		if(action & RC_SERVICE_STOP) stop_dsl_diag();
		if(action & RC_SERVICE_START) start_dsl_diag();
	}
#endif
#endif
#ifdef RTCONFIG_PUSH_EMAIL
#ifdef RTCONFIG_DBLOG
	else if (strcmp(script, "dblog") == 0) {
		if(action & RC_SERVICE_STOP) stop_dblog();
		if(action & RC_SERVICE_START) start_dblog(1);
	}
#endif /* RTCONFIG_DBLOG */
#endif /* RTCONFIG_PUSH_EMAIL */
	else if (strcmp(script, "wan_line") == 0) {
		_dprintf("%s: restart_wan_line: %s.\n", __FUNCTION__, cmd[1]);
		if(cmd[1]) {
			wan_unit = atoi(cmd[1]);
			char *current_ifname = get_wan_ifname(wan_unit);

			wan_up(current_ifname);
		}
	}
#ifdef CONFIG_BCMWL5
	else if (strcmp(script, "nas") == 0) {
		if(action & RC_SERVICE_STOP) stop_nas();
		if(action & RC_SERVICE_START) {
			start_eapd();
			start_nas();
			start_wps();
#ifdef RTCONFIG_NOTIFICATION_CENTER
			start_wlc_nt();
#endif
#if defined(RTCONFIG_WLCEVENTD)
			start_wlceventd();
#endif
#ifdef RTCONFIG_BCMWL6
			start_igmp_proxy();
#ifdef BCM_BSD
			start_bsd();
#endif
#ifdef BCM_APPEVENTD
			start_appeventd();
#endif
#ifdef BCM_SSD
			start_ssd();
#endif
#ifdef BCM_EVENTD
			start_eventd();
#endif
			start_acsd();
#ifdef RTCONFIG_DHDAP
			start_dhd_monitor();
#endif
#ifdef BCM_ASPMD
			start_aspmd();
#endif
#endif
			start_wl();
			lanaccess_wl();
#ifdef RTCONFIG_BCMWL6
#ifdef RTCONFIG_HSPOT
			start_hspotap();
#endif
#endif
		}
	}
#endif
#ifdef RTCONFIG_USB
	else if (strcmp(script, "nasapps") == 0)
	{
		if(action&RC_SERVICE_STOP){
//_dprintf("restart_nas_services(%d): test 10.\n", getpid());
			restart_nas_services(1, 0);
		}
		if(action&RC_SERVICE_START){
			int restart_upnp = 0;
			if (pidof("miniupnpd") != -1) {
				stop_upnp();
				restart_upnp = 1;
			}
//_dprintf("restart_nas_services(%d): test 11.\n", getpid());
			restart_nas_services(0, 1);
			if (restart_upnp) start_upnp();
		}
	}
#if defined(RTCONFIG_SAMBASRV) && defined(RTCONFIG_FTP)
	else if (strcmp(script, "ftpsamba") == 0)
	{
		if(action & RC_SERVICE_STOP) {
			stop_ftpd();
			stop_samba();
		}
		if(action & RC_SERVICE_START) {
			start_dnsmasq();	// this includes stop_dnsmasq
			setup_passwd();
			set_hostname();
			start_samba();
			start_ftpd();
		}
	}
#endif
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
	else if (strcmp(script, "pms_account") == 0)
	{
		if(action & RC_SERVICE_STOP) {
#if defined(RTCONFIG_SAMBASRV) && defined(RTCONFIG_FTP)
			stop_ftpd();
			stop_samba();
#endif
		}
		if(action & RC_SERVICE_START) {
#if defined(RTCONFIG_SAMBASRV) && defined(RTCONFIG_FTP)
			start_dnsmasq();	// this includes stop_dnsmasq
			setup_passwd();
			start_samba();
			start_ftpd();
#endif
		}
	}
	else if (strcmp(script, "pms_device") == 0)
	{
		if(action&RC_SERVICE_STOP) {
			stop_iQos();
#if defined(RTCONFIG_BWDPI)
			stop_dpi_engine_service(0);
#endif
			del_iQosRules();
		}
		if(action & RC_SERVICE_START) {
#if defined(RTCONFIG_QCA) || \
		(defined(RTCONFIG_RALINK) && !defined(RTCONFIG_DSL) && !defined(RTN13U))
			reinit_hwnat(-1);
#endif
			QOS_CONTROL();

			// multiple instance is handled, but 0 is used
			start_default_filter(0);
#ifdef RTCONFIG_PARENTALCTRL
			start_pc_block();
#endif
			// TODO handle multiple wan
			start_firewall(wan_primary_ifunit(), 0);
		}
	}
#endif
#ifdef RTCONFIG_FTP
	else if (strcmp(script, "ftpd") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_ftpd();
		if(action & RC_SERVICE_START) start_ftpd();

		/* for security concern, even if you stop ftp daemon, it is better to restart firewall to clean FTP port: 21. */
		start_firewall(wan_primary_ifunit(), 0);
	}
	else if (strcmp(script, "ftpd_force") == 0)
	{
		nvram_set("st_ftp_force_mode", nvram_safe_get("st_ftp_mode"));
		nvram_commit();

		if(action & RC_SERVICE_STOP) stop_ftpd();
		if(action & RC_SERVICE_START) start_ftpd();

		/* for security concern, even if you stop ftp daemon, it is better to restart firewall to clean FTP port: 21. */
		start_firewall(wan_primary_ifunit(), 0);
	}
#endif
#ifdef RTCONFIG_TFTP_SERVER
	else if (strcmp(script, "tftpd") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_tftpd();
		if(action & RC_SERVICE_START) start_tftpd();
	}
#endif
#ifdef RTCONFIG_SAMBASRV
	else if (strcmp(script, "samba") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_samba();
		if(action & RC_SERVICE_START) {
			start_dnsmasq();	// this includes stop_dnsmasq
			start_samba();
		}
	}
	else if (strcmp(script, "samba_force") == 0)
	{
		nvram_set("st_samba_force_mode", nvram_safe_get("st_samba_mode"));
		nvram_commit();

		if(action & RC_SERVICE_STOP) stop_samba();
		if(action & RC_SERVICE_START) {
			start_dnsmasq();	// this includes stop_dnsmasq
			start_samba();
		}
	}
#endif
#ifdef RTCONFIG_CAPTIVE_PORTAL
	else if (strcmp(script, "chilli") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_chilli();
		if(action&RC_SERVICE_START) start_chilli();
	}
	else if (strcmp(script, "CP") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_CP();
		if(action&RC_SERVICE_START) start_CP();
	}

	else if (strcmp(script, "uam_srv") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_uam_srv();
		if(action&RC_SERVICE_START) start_uam_srv();
	}
#endif
#ifdef RTCONFIG_FREERADIUS
	else if (strcmp(script, "radiusd") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_radiusd();
		if(action&RC_SERVICE_START) start_radiusd();
	}
#endif
#ifdef RTCONFIG_WEBDAV
	else if (strcmp(script, "webdav") == 0)
	{
		if(action & RC_SERVICE_STOP){
			stop_webdav();
		}
		if(action & RC_SERVICE_START) {
			stop_upnp();
			start_firewall(wan_primary_ifunit(), 0);
			start_webdav();
			start_upnp();
		}
	}
#else
	else if (strcmp(script, "webdav") == 0){
		if(f_exists("/opt/etc/init.d/S50aicloud"))
			system("sh /opt/etc/init.d/S50aicloud scan");
	}
	else if (strcmp(script, "setting_webdav") == 0){
		if(f_exists("/opt/etc/init.d/S50aicloud"))
			system("sh /opt/etc/init.d/S50aicloud restart");
	}
#endif
	else if (strcmp(script, "enable_webdav") == 0)
	{
		stop_upnp();
		stop_ddns();
#ifdef RTCONFIG_WEBDAV
		stop_webdav();
#endif
		start_firewall(wan_primary_ifunit(), 0);
		start_webdav();
		start_ddns();
		start_upnp();

	}
#ifdef RTCONFIG_TUNNEL
	else if (strcmp(script, "aae") == 0 || strcmp(script, "mastiff") == 0)
	{
		if(action&RC_SERVICE_STOP){
			stop_mastiff();
		}
		if(action&RC_SERVICE_START) {
			start_mastiff();
		}
	}
#endif

//#endif
//#ifdef RTCONFIG_CLOUDSYNC
	else if (strcmp(script, "cloudsync") == 0)
	{
#ifdef RTCONFIG_CLOUDSYNC
		int fromUI = 0;

		if(action & RC_SERVICE_STOP && action & RC_SERVICE_START)
			fromUI = 1;

		if(action&RC_SERVICE_STOP){
			if(cmd[1])
				stop_cloudsync(atoi(cmd[1]));
			else
				stop_cloudsync(-1);
		}
		if(action & RC_SERVICE_START) start_cloudsync(fromUI);
#else
		system("sh /opt/etc/init.d/S50smartsync restart");
#endif
	}
#ifdef RTCONFIG_WTFAST
	else if (strcmp(script, "wtfast") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_wtfast();
		if(action & RC_SERVICE_START) start_wtfast();
	}
	else if(strcmp(script, "wtfast_rule") == 0){
		//_dprintf("send SIGHUP to wtfast_rule SIGHUP = %d\n", SIGHUP);
		killall("wtfslhd", SIGHUP);
	}
#endif
#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_PRINTER)
	else if (strcmp(script, "lpd") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_lpd();
		if(action & RC_SERVICE_START) start_lpd();
	}
	else if (strcmp(script, "u2ec") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_u2ec();
		if(action & RC_SERVICE_START) start_u2ec();
	}
#endif
#ifdef RTCONFIG_MEDIA_SERVER
	else if (strcmp(script, "media") == 0)
	{
		if(action & RC_SERVICE_STOP) {
			force_stop_dms();
			stop_mt_daapd();
		}
		if(action & RC_SERVICE_START) {
			start_dms();
			start_mt_daapd();
		}
	}
	else if (strcmp(script, "dms") == 0)
	{
		if(action & RC_SERVICE_STOP) force_stop_dms();
		if(action & RC_SERVICE_START) start_dms();
	}
	else if (strcmp(script, "mt_daapd") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_mt_daapd();
		if(action & RC_SERVICE_START) start_mt_daapd();
	}
#endif
#ifdef RTCONFIG_DISK_MONITOR
	else if (strcmp(script, "diskmon")==0)
	{
		if(action & RC_SERVICE_STOP) stop_diskmon();
		if(action & RC_SERVICE_START) start_diskmon();
	}
	else if (strcmp(script, "diskscan")==0)
	{
		if(action & RC_SERVICE_START)
			kill_pidfile_s("/var/run/disk_monitor.pid", SIGUSR2);
	}
	else if (strcmp(script, "diskformat")==0)
	{
		if(action & RC_SERVICE_START)
			kill_pidfile_s("/var/run/disk_monitor.pid", SIGUSR1);
	}
#endif
	else if(!strncmp(script, "apps_", 5))
	{
		if(action & RC_SERVICE_START) {
			if(strcmp(script, "apps_update")==0)
				strcpy(nvtmp, "/usr/sbin/app_update.sh");
			else if(strcmp(script, "apps_stop")==0)
				strcpy(nvtmp, "/usr/sbin/app_stop.sh");
			else if(strcmp(script, "apps_upgrade")==0)
				strcpy(nvtmp, "/usr/sbin/app_upgrade.sh");
			else if(strcmp(script, "apps_install")==0)
				strcpy(nvtmp, "/usr/sbin/app_install.sh");
			else if(strcmp(script, "apps_remove")==0)
				strcpy(nvtmp, "/usr/sbin/app_remove.sh");
			else if(strcmp(script, "apps_enable")==0)
				strcpy(nvtmp, "/usr/sbin/app_set_enabled.sh");
			else if(strcmp(script, "apps_switch")==0)
				strcpy(nvtmp, "/usr/sbin/app_switch.sh");
			else if(strcmp(script, "apps_cancel")==0)
				strcpy(nvtmp, "/usr/sbin/app_cancel.sh");
			else strcpy(nvtmp, "");

			if(strlen(nvtmp) > 0) {
				nvram_set("apps_state_autorun", "");
				nvram_set("apps_state_install", "");
				nvram_set("apps_state_remove", "");
				nvram_set("apps_state_switch", "");
				nvram_set("apps_state_stop", "");
				nvram_set("apps_state_enable", "");
				nvram_set("apps_state_update", "");
				nvram_set("apps_state_upgrade", "");
				nvram_set("apps_state_cancel", "");
				nvram_set("apps_state_error", "");

				free_caches(FREE_MEM_PAGE, 1, 0);

				cmd[0] = nvtmp;
				start_script(count, cmd);
			}
		}
	}
#ifdef RTCONFIG_USB_MODEM
	else if(!strncmp(script, "simauth", 7)){
		if(cmd[1]){
			char *at_cmd[] = {"/usr/sbin/modem_status.sh", "simauth", NULL};

			_eval(at_cmd, NULL, 0, NULL);
		}
	}
	else if(!strncmp(script, "simpin", 6)){
		if(cmd[1] && cmd[2]){
			char pincode[8];
			char *at_cmd[] = {"/usr/sbin/modem_status.sh", "simpin", pincode, NULL};
			char *at_cmd2[] = {"/usr/sbin/modem_status.sh", "simauth", NULL};

			if(nvram_get_int(strcat_r(prefix2, "act_sim", tmp2)) == 2){
				snprintf(pincode, 8, "%s", cmd[2]);

				_eval(at_cmd, ">/tmp/modem_action.ret", 0, NULL);
				_eval(at_cmd2, NULL, 0, NULL);
			}
		}
	}
	else if(!strncmp(script, "simpuk", 6)){
		if(cmd[1] && cmd[2] && cmd[3]){
			char pukcode[10], pincode[8];
			char *at_cmd[] = {"/usr/sbin/modem_status.sh", "simpuk", pukcode, pincode, NULL};
			char *at_cmd2[] = {"/usr/sbin/modem_status.sh", "simauth", NULL};

			if(nvram_get_int(strcat_r(prefix2, "act_sim", tmp2)) == 3){
				snprintf(pukcode, 10, "%s", cmd[2]);
				snprintf(pincode, 8, "%s", cmd[3]);

				_eval(at_cmd, ">/tmp/modem_action.ret", 0, NULL);
				_eval(at_cmd2, NULL, 0, NULL);
			}
		}
	}
	else if(!strncmp(script, "lockpin", 7)){
		if(cmd[1] && cmd[2]){
			char lock[4], pincode[8];
			char *at_cmd[] = {"/usr/sbin/modem_status.sh", "lockpin", lock, pincode, NULL};
			char *at_cmd2[] = {"/usr/sbin/modem_status.sh", "simauth", NULL};

			if(nvram_get_int(strcat_r(prefix2, "act_sim", tmp2)) == 1){
				snprintf(pincode, 8, "%s", cmd[2]);

				if(action & RC_SERVICE_STOP){ // unlock
					snprintf(lock, 4, "%s", "0");
					_eval(at_cmd, ">/tmp/modem_action.ret", 0, NULL);
					_eval(at_cmd2, NULL, 0, NULL);
				}
				else if(action & RC_SERVICE_START){ // lock
					snprintf(lock, 4, "%s", "1");
					_eval(at_cmd, ">/tmp/modem_action.ret", 0, NULL);
					_eval(at_cmd2, NULL, 0, NULL);
				}
			}
		}
	}
	else if(!strncmp(script, "pwdpin", 6)){
		if(cmd[1] && cmd[2] && cmd[3]){
			char pincode[8], pincode_new[8];
			char *at_cmd[] = {"/usr/sbin/modem_status.sh", "pwdpin", pincode, pincode_new, NULL};
			char *at_cmd2[] = {"/usr/sbin/modem_status.sh", "simauth", NULL};

			if(nvram_get_int(strcat_r(prefix2, "act_sim", tmp2)) == 1){
				snprintf(pincode, 8, "%s", cmd[2]);
				snprintf(pincode_new, 8, "%s", cmd[3]);

				_eval(at_cmd, ">/tmp/modem_action.ret", 0, NULL);
				_eval(at_cmd2, NULL, 0, NULL);
			}
		}
	}
	else if(!strncmp(script, "modemscan", 9)){
		if(cmd[1]){
			char *at_cmd[] = {"/usr/sbin/modem_status.sh", "scan", NULL};

			wan_unit = get_wanunit_by_type(get_wantype_by_modemunit(modem_unit));
			if(wan_unit != WAN_UNIT_NONE && wan_unit != WAN_UNIT_MAX){
				nvram_set(strcat_r(prefix2, "act_scanning", tmp2), "3");

				stop_wan_if(wan_unit);

				_eval(at_cmd, ">/tmp/modem_action.ret", 0, NULL);
			}
		}
	}
	else if(!strncmp(script, "modemsta", 8)){
		char isp[32];
		char *at_cmd[] = {"/usr/sbin/modem_status.sh", "station", isp, NULL};

		snprintf(isp, 32, "%s", nvram_safe_get("modem_roaming_isp"));

		if(strlen(isp) > 0)
			_eval(at_cmd, ">/tmp/modem_action.ret", 0, NULL);
	}
	else if(!strncmp(script, "sendSMS", 7)){
		char phone[32], message[PATH_MAX];
		char *at_cmd[] = {"/usr/sbin/modem_status.sh", "send_sms", phone, message, NULL};

		snprintf(phone, 32, "%s", nvram_safe_get("modem_sms_phone"));
		if(!strcmp(cmd[1], "alert"))
			snprintf(message, PATH_MAX, "%s %s bytes.", nvram_safe_get("modem_sms_message1"), nvram_safe_get("modem_bytes_data_warning"));
		else
			snprintf(message, PATH_MAX, "%s %s bytes.", nvram_safe_get("modem_sms_message2"), nvram_safe_get("modem_bytes_data_limit"));

#ifdef RTCONFIG_INTERNAL_GOBI
		stop_lteled();
#endif
		_eval(at_cmd, ">/tmp/modem_action.ret", 0, NULL);
#ifdef RTCONFIG_INTERNAL_GOBI
		start_lteled();
#endif
	}
#ifdef RTCONFIG_USB_SMS_MODEM
	else if(!strncmp(script, "savesms", 7)){
		// cmd[1]: Destination number, cmd[2]: the SMS file
		char ttynode[32], smsc[32];
		char buf[PATH_MAX];
		int sms_index;
		int ret;

		printf("~~~~~~~~~~ Saving the SMS: %s, %s. !!! ~~~~~~~~~~\n", cmd[1], cmd[2]);
		snprintf(ttynode, 32, "%s", nvram_safe_get("usb_modem_act_int"));
		snprintf(smsc, 32, "%s", nvram_safe_get("usb_modem_act_smsc"));

		if((sms_index = saveSMSPDUtoSIM(ttynode, smsc, cmd[1], cmd[2], buf, PATH_MAX)) < 0)
			printf("%s: SMS-SUBMIT: Failed to saveSMS.\n", script);
		else if((ret = getSMSPDUbyIndex(ttynode, sms_index, NULL, 0)) < 0)
			printf("%s: SMS-SUBMIT: Failed to getSMS.\n", script);
		else{
			printf("%s: index=%d.\n", script, sms_index);
			printf("%s: type=%d.\n", script, ret);
			printf("%s: SMS=%s.\n", script, buf);
		}
	}
	else if(!strncmp(script, "sendsmsbyindex", 14)){
		// cmd[1]: SMS index.
		char ttynode[32];
		int sms_index;
#ifdef SAVESMS
		char sms_file[PATH_MAX], sms_file2[PATH_MAX];
#endif
		int ret;

		printf("~~~~~~~~~~ Sending the %sth SMS. !!! ~~~~~~~~~~\n", cmd[1]);
		snprintf(ttynode, 32, "%s", nvram_safe_get("usb_modem_act_int"));
		sms_index = strtod(cmd[1], NULL);

		if((ret = sendSMSPDUfromSIM(ttynode, sms_index)) < 0)
			printf("%s: SMS-SUBMIT: Failed to send the index(%d) SMS.\n", cmd[0], sms_index);
		else{
			printf("%s: done.\n", script);

#ifdef SAVESMS
			if(getSMSFileName(2, sms_index, sms_file, PATH_MAX) > 0 && getSMSFileName(3, sms_index, sms_file2, PATH_MAX) > 0)
				rename(sms_file, sms_file2);
#endif
		}
	}
	else if(!strncmp(script, "sendsmsnow", 10)){
		// cmd[1]: Destination number, cmd[2]: the SMS file
		char ttynode[32], smsc[32];
		char buf[PATH_MAX];
		int sms_index, sms_type;
#ifdef SAVESMS
		char sms_file[PATH_MAX], sms_file2[PATH_MAX];
#endif
		int ret;

		printf("~~~~~~~~~~ Sending the SMS: %s, %s. !!! ~~~~~~~~~~\n", cmd[1], cmd[2]);
		snprintf(ttynode, 32, "%s", nvram_safe_get("usb_modem_act_int"));
		snprintf(smsc, 32, "%s", nvram_safe_get("usb_modem_act_smsc"));

		if((sms_index = saveSMSPDUtoSIM(ttynode, smsc, cmd[1], cmd[2], buf, PATH_MAX)) < 0)
			printf("%s: SMS-SUBMIT: Failed to saveSMS.\n", script);
		else if((sms_type = getSMSPDUbyIndex(ttynode, sms_index, NULL, 0)) < 0)
			printf("%s: SMS-SUBMIT: Failed to getSMS.\n", script);
		else if((ret = sendSMSPDUfromSIM(ttynode, sms_index)) < 0){
			printf("%s: SMS-SUBMIT: Failed to sendSMS.\n", script);
		}
		else{
			printf("%s: SMS-SUBMIT(%d)=%s.\n", script, ret, buf);

#ifdef SAVESMS
			if(getSMSFileName(2, sms_index, sms_file, PATH_MAX) > 0 && getSMSFileName(3, sms_index, sms_file2, PATH_MAX) > 0)
				rename(sms_file, sms_file2);
#endif
		}
	}
	else if(!strncmp(script, "delsms", 6)){
		// cmd[1]: SMS index.
		char ttynode[32];
		int sms_index, sms_type;
#ifdef SAVESMS
		char sms_file[PATH_MAX];
#endif
		int ret;

		printf("~~~~~~~~~~ Deleting the %sth SMS:. !!! ~~~~~~~~~~\n", cmd[1]);
		snprintf(ttynode, 32, "%s", nvram_safe_get("usb_modem_act_int"));
		sms_index = strtod(cmd[1], NULL);

		if((sms_type = getSMSPDUbyIndex(ttynode, sms_index, NULL, 0)) < 0)
			printf("%s: Failed to get the type of index(%d)'s SMS.\n", script, sms_index);
		else if((ret = delSMSPDUbyIndex(ttynode, sms_index)) < 0)
			printf("%s: Failed to delSMS.\n", script);
#ifdef SAVESMS
		else if((ret = getSMSFileName(sms_type, sms_index, sms_file, PATH_MAX)) < 0)
			printf("%s: Failed to getSMSFile.\n", script);
		else{
			unlink(sms_file);
			if(sms_type == 0){
				if(getSMSFileName(1, sms_index, sms_file, PATH_MAX) > 0)
					unlink(sms_file);
			}
			else if(sms_type == 1){
				if(getSMSFileName(0, sms_index, sms_file, PATH_MAX) > 0)
					unlink(sms_file);
			}
		}
#endif
	}
	else if(!strncmp(script, "modsmsdraft", 11)){
		// cmd[1]: SMS index, cmd[2]: Destination number, cmd[3]: the SMS file
		char ttynode[32], smsc[32];
		char buf[PATH_MAX];
		int sms_index, sms_type;
#ifdef SAVESMS
		char sms_file[PATH_MAX];
#endif
		int ret;

		printf("~~~~~~~~~~ Modifying the SMS: %s, %s, %s. !!! ~~~~~~~~~~\n", cmd[1], cmd[2], cmd[3]);
		sms_index = strtod(cmd[1], NULL);
		snprintf(ttynode, 32, "%s", nvram_safe_get("usb_modem_act_int"));
		snprintf(smsc, 32, "%s", nvram_safe_get("usb_modem_act_smsc"));

		if((sms_type = getSMSPDUbyIndex(ttynode, sms_index, NULL, 0)) < 0)
			printf("%s: Failed to get the type of index(%d)'s SMS.\n", script, sms_index);
		else if((ret = delSMSPDUbyIndex(ttynode, sms_index)) < 0)
			printf("%s: Failed to delSMS.\n", script);
#ifdef SAVESMS
		else if((ret = getSMSFileName(sms_type, sms_index, sms_file, PATH_MAX)) < 0)
			printf("%s: Failed to getSMSFile.\n", script);
		else{
			unlink(sms_file);
			if(sms_type == 0){
				if(getSMSFileName(1, sms_index, sms_file, PATH_MAX) > 0)
					unlink(sms_file);
			}
			else if(sms_type == 1){
				if(getSMSFileName(0, sms_index, sms_file, PATH_MAX) > 0)
					unlink(sms_file);
			}
		}
#endif

		if((sms_index = saveSMSPDUtoSIM(ttynode, smsc, cmd[2], cmd[3], buf, PATH_MAX)) < 0)
			printf("%s: SMS-SUBMIT: Failed to saveSMS.\n", script);
		else if((ret = getSMSPDUbyIndex(ttynode, sms_index, NULL, 0)) < 0)
			printf("%s: SMS-SUBMIT: Failed to getSMS.\n", script);
		else{
			printf("%s: index=%d.\n", script, sms_index);
			printf("%s: type=%d.\n", script, ret);
			printf("%s: SMS=%s.\n", script, buf);
		}
	}
	else if(!strncmp(script, "savephonenum", 12)){
		// cmd[1]: Phone number, cmd[2]: Phone name
		char ttynode[32];
		int phone_index;

		printf("~~~~~~~~~~ Saving the Phone: %s, %s. !!! ~~~~~~~~~~\n", cmd[1], cmd[2]);
		snprintf(ttynode, 32, "%s", nvram_safe_get("usb_modem_act_int"));

		if((phone_index = savePhonenum(ttynode, cmd[1], cmd[2])) < 0)
			printf("%s: Failed to savePhonenum.\n", script);
	}
	else if(!strncmp(script, "delphonenum", 11)){
		// cmd[1]: Phone index
		char ttynode[32];
		int phone_index;

		printf("~~~~~~~~~~ Deleting the Phone: %s. !!! ~~~~~~~~~~\n", cmd[1]);
		snprintf(ttynode, 32, "%s", nvram_safe_get("usb_modem_act_int"));
		phone_index = strtod(cmd[1], NULL);

		if((phone_index = delPhonenum(ttynode, phone_index)) < 0)
			printf("%s: Failed to delPhonenum.\n", script);
	}
	else if(!strncmp(script, "modphonenum", 11)){
		// cmd[1]: Phone index, cmd[2]: Phone number, cmd[3]: Phone name
		char ttynode[32];
		int phone_index;

		printf("~~~~~~~~~~ Modifying the Phone: %s, %s, %s. !!! ~~~~~~~~~~\n", cmd[1], cmd[2], cmd[3]);
		snprintf(ttynode, 32, "%s", nvram_safe_get("usb_modem_act_int"));
		phone_index = strtod(cmd[1], NULL);

		if((phone_index = modPhonenum(ttynode, phone_index, cmd[2], cmd[3])) < 0)
			printf("%s: Failed to modPhonenum.\n", script);
	}
#endif // RTCONFIG_USB_SMS_MODEM
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	else if(!strncmp(script, "datacount", 9)){
		char *at_cmd[] = {"/usr/sbin/modem_status.sh", "bytes", NULL};

		_eval(at_cmd, ">/tmp/modem_action.ret", 0, NULL);
	}
	else if(!strncmp(script, "resetcount", 10)){
		time_t now;
		char timebuf[32];
		char *at_cmd[] = {"/usr/sbin/modem_status.sh", "bytes-", NULL};

		time(&now);
		snprintf(timebuf, 32, "%d", (int)now);
		nvram_set("modem_bytes_data_start", timebuf);

		_eval(at_cmd, ">/tmp/modem_action.ret", 0, NULL);
	}
	else if(!strncmp(script, "sim_del", 7)){
		char sim_order[32];
		char *at_cmd[] = {"/usr/sbin/modem_status.sh", "imsi_del", sim_order, NULL};

		snprintf(sim_order, 32, "%s", cmd[1]);

		_eval(at_cmd, ">/tmp/modem_action.ret", 0, NULL);
	}
	else if(!strncmp(script, "set_dataset", 11)){
		char *at_cmd[] = {"/usr/sbin/modem_status.sh", "set_dataset", NULL};

		_eval(at_cmd, ">/tmp/modem_action.ret", 0, NULL);
	}
#endif
#ifdef RTCONFIG_INTERNAL_GOBI
	else if(!strncmp(script, "simdetect", 9)){
		// Need to reboot after this.
		char buf[4];
		char *at_cmd1[] = {"/usr/sbin/modem_status.sh", "simdetect", NULL};
		char *at_cmd2[] = {"/usr/sbin/modem_status.sh", "simdetect", buf, NULL};

		if(cmd[1]){
			snprintf(buf, 4, "%s", cmd[1]);
			_eval(at_cmd2, ">/tmp/modem_action.ret", 0, NULL);
		}
		else
			_eval(at_cmd1, ">/tmp/modem_action.ret", 0, NULL);
	}
	else if(!strncmp(script, "getband", 7)){
		char *at_cmd[] = {"/usr/sbin/modem_status.sh", "band", NULL};

		_eval(at_cmd, ">/tmp/modem_action.ret", 0, NULL);
	}
	else if(!strncmp(script, "setband", 7)){
		char buf[8];
		char *at_cmd[] = {"/usr/sbin/modem_status.sh", "setband", buf, NULL};

		snprintf(buf, 8, "%s", nvram_safe_get("modem_lte_band"));
		if(strlen(buf) <= 0)
			snprintf(buf, 8, "%s", "auto");

		_eval(at_cmd, ">/tmp/modem_action.ret", 0, NULL);
	}
#endif
#endif // RTCONFIG_USB_MODEM
#endif // RTCONFIG_USB
#if defined(RTCONFIG_BT_CONN)
	else if (strcmp(script, "dbus_daemon") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_dbus_daemon();
		if(action&RC_SERVICE_START) start_dbus_daemon();
	}
	else if (strcmp(script, "bluetooth_service") == 0)
	{
		if((action & RC_SERVICE_STOP) && (action & RC_SERVICE_START))
		{
			stop_bluetooth_service();
			start_bluetooth_service();
		}
		else if(action&RC_SERVICE_STOP) stop_bluetooth_service();
		else if(action&RC_SERVICE_START) start_bluetooth_service();
	}
#endif	/* RTCONFIG_BT_CONN */
#ifdef RTCONFIG_WIFI_SON
	else if (strcmp(script, "hyfi_process") == 0)
	{
		if(action&RC_SERVICE_START) start_hyfi_process();
	}
	else if (strcmp(script, "hyfi_sync") == 0)
	{
		if(action&RC_SERVICE_START) start_hyfi_sync();
	}
	else if (strcmp(script, "chg_swmode") == 0)
	{
		if(action&RC_SERVICE_START) start_chg_swmode();
	}
	else if (strcmp(script, "spcmd") == 0)
	{
		if(action&RC_SERVICE_START) start_spcmd();
	}
#if defined(MAPAC2200)
	else if (strcmp(script, "bhblock") == 0)
	{
		if(action&RC_SERVICE_START) start_bhblock();
	}
#endif
#endif
	else if(!strncmp(script, "webs_", 5) || !strncmp(script, "gobi_", 5))
	{
		if(action & RC_SERVICE_START) {
#ifdef DEBUG_RCTEST // Left for UI debug
			char *webscript_dir;
			webscript_dir = nvram_safe_get("webscript_dir");
			if(strlen(webscript_dir))
				sprintf(nvtmp, "%s/%s.sh", webscript_dir, script);
			else
#endif
			sprintf(nvtmp, "%s.sh", script);
			cmd[0] = nvtmp;
			start_script(count, cmd);
		}
	}
#ifdef RTCONFIG_LETSENCRYPT
	else if (strcmp(script, "ddns_le") == 0)
	{
		nvram_set("le_rc_notify", "1");
		if(action & RC_SERVICE_STOP) stop_ddns();
		if(action & RC_SERVICE_START) start_ddns();
	}
#endif
	else if (strcmp(script, "ddns") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_ddns();
		if(action & RC_SERVICE_START) start_ddns();
	}
	else if (strcmp(script, "aidisk_asusddns_register") == 0)
	{
		asusddns_reg_domain(0);
	}
	else if (strcmp(script, "adm_asusddns_register") == 0)
	{
		asusddns_reg_domain(1);
	}
	else if (strcmp(script, "httpd") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_httpd();
		if(action & RC_SERVICE_START){
			start_httpd();
#if defined(RTCONFIG_BWDPI)
			setup_wrs_conf();
#endif
		}
	}
	else if (strcmp(script, "telnetd") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_telnetd();
		if(action & RC_SERVICE_START) start_telnetd();
	}
#ifdef RTCONFIG_SSH
	else if (strcmp(script, "sshd") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_sshd();
		if(action & RC_SERVICE_START) start_sshd();
	}
#endif
#ifdef RTCONFIG_IPV6
	else if (strcmp(script, "ipv6") == 0) {
		if (action & RC_SERVICE_STOP)
			stop_ipv6();
		if (action & RC_SERVICE_START)
			start_ipv6();
	}
	else if (strcmp(script, "dhcp6c") == 0) {
		if (action & RC_SERVICE_STOP) {
			stop_dhcp6c();
		}
		if (action & RC_SERVICE_START) {
			start_dhcp6c();
		}
	}
	else if (strcmp(script, "wan6") == 0) {
		if (action & RC_SERVICE_STOP) {
			stop_wan6();
			stop_ipv6();
		}
		if (action & RC_SERVICE_START) {
			start_ipv6();
			// when no option from ipv4, restart wan entirely
			if(update_6rd_info()==0)
			{
				stop_wan_if(wan_primary_ifunit_ipv6());
				start_wan_if(wan_primary_ifunit_ipv6());
			}
			else
			{
				start_wan6();
			}
		}
	}
#endif
	else if (strcmp(script, "dns") == 0)
	{
		if(action & RC_SERVICE_START) reload_dnsmasq();
	}
	else if (strcmp(script, "dnsmasq") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_dnsmasq();
		if(action & RC_SERVICE_START) start_dnsmasq();
	}
#ifdef RTCONFIG_DHCP_OVERRIDE
	else if (strcmp(script, "dhcpd") == 0)
	{
		if (nvram_match("dhcp_enable_x", "0")) {
			// disable dhcp server
			if (nvram_match("dnsqmode", "2")) {
				nvram_set("dnsqmode", "1");
				restart_dnsmasq(0);
			}
		}
		else {
			// enable dhcp server
			if (nvram_match("dnsqmode", "1")) {
				nvram_set("dnsqmode", "2");
				restart_dnsmasq(0);
			}
		}

	}
#endif
	else if (strcmp(script, "upnp") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_upnp();
		if(action & RC_SERVICE_START) start_upnp();
	}
	else if (strcmp(script, "qos") == 0)
	{
		nvram_set("restart_qo", "1");
		if(action&RC_SERVICE_STOP) {
			stop_iQos();
#if defined(RTCONFIG_BWDPI)
			stop_dpi_engine_service(0);
#endif
			del_iQosRules();
		}
		if(action & RC_SERVICE_START) {
#ifdef HND_ROUTER
			hnd_nat_ac_init(0);
#endif
			if (nvram_match("qos_enable", "1") &&
			   !nvram_match("qos_type", "2")) {
				ForceDisableWLan_bw();
			} else if (nvram_match("qos_enable", "0")) {
				ForceDisableWLan_bw();
			}
#if defined(RTCONFIG_QCA) || \
		(defined(RTCONFIG_RALINK) && !defined(RTCONFIG_DSL) && !defined(RTN13U))
			reinit_hwnat(-1);
#endif
// TODO: check if I must reapply my codel patch differently since 18991?
			QOS_CONTROL();
		}
		nvram_set("restart_qo", "0");

#if defined(RTCONFIG_BWDPI)
		// force to rebuild firewall to avoid some loopback issue
		if (nvram_match("fw_nat_loopback", "2"))
			start_firewall(wan_primary_ifunit(), 0);
#endif
	}
#if defined(RTCONFIG_BWDPI)
	else if (strcmp(script, "wrs") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_dpi_engine_service(0);
		if(action & RC_SERVICE_START) {
			start_dpi_engine_service();
			// force to rebuild firewall to avoid some loopback issue
			if (nvram_match("fw_nat_loopback", "2"))
				start_firewall(wan_primary_ifunit(), 0);
		}
	}
	else if (strcmp(script, "wrs_force") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_dpi_engine_service(1);
	}
	else if (strcmp(script, "sig_check") == 0)
	{
		if(action & RC_SERVICE_START){
			eval("sig_update.sh");
			if(nvram_get_int("sig_state_flag")) eval("sig_upgrade.sh", "1");
			stop_dpi_engine_service(0);
			start_dpi_engine_service();
		}
	}
	else if (strcmp(script, "dpi_disable") == 0)
	{
		disable_dpi_engine_setting();
	}
	else if (strcmp(script, "reset_cc_db") == 0)
	{
		eval("AiProtectionMonitor", "-z", "-t", "1");
	}
	else if (strcmp(script, "reset_mals_db") == 0)
	{
		eval("AiProtectionMonitor", "-z", "-t", "2");
	}
	else if (strcmp(script, "reset_vp_db") == 0)
	{
		eval("AiProtectionMonitor", "-z", "-t", "3");
	}
#endif
	else if (strcmp(script, "traffic_analyzer") == 0)
	{
		// only stop service need to save database
		if(action & RC_SERVICE_STOP) hm_traffic_analyzer_save();
	}
#ifdef RTCONFIG_TRAFFIC_LIMITER
	else if (strcmp(script, "reset_traffic_limiter") == 0)
	{
		hm_traffic_limiter_save();
		reset_traffic_limiter_counter(0);
	}
	else if (strcmp(script, "reset_traffic_limiter_force") == 0)
	{
		reset_traffic_limiter_counter(1);
	}
	else if (strcmp(script, "reset_tl_count") == 0)
	{
		f_write_string(traffic_limtier_count_path(), "0", 0, 0);
	}
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
	else if (strcmp(script, "send_confirm_mail") == 0)
	{
		char str[32];
		snprintf(str, 32, "0x%x", RESERVATION_MAIL_CONFIRM_EVENT);
		eval("Notify_Event2NC", str, "");
	}
	else if (strcmp(script, "email_conf") == 0)
	{
		am_setup_email_conf();
	}
	else if (strcmp(script, "email_info") == 0)
	{
		am_setup_email_info();
		char buf[8];
		if (f_read_string(NOTIFY_ACTION_MAIL_PID_PATH, buf, sizeof(buf)) > 0) {
			kill(atoi(buf), SIGUSR2);
		}
	}
	else if (strcmp(script, "update_nc_setting_conf") == 0)
	{
		update_nc_setting_conf();
	}
#endif
	else if (strcmp(script, "logger") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_logger();
		if(action & RC_SERVICE_START) start_logger();
	}
#ifdef RTCONFIG_CROND
	else if (strcmp(script, "crond") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_cron();
		if(action & RC_SERVICE_START) start_cron();
	}
#endif
	else if (strcmp(script, "firewall") == 0)
	{
		nvram_set("restart_fwl", "1");
		if(action & RC_SERVICE_START)
		{
#if defined(RTCONFIG_QCA) || \
		(defined(RTCONFIG_RALINK) && !defined(RTCONFIG_DSL) && !defined(RTN13U))
			reinit_hwnat(-1);
#endif
			// multiple instance is handled, but 0 is used
			start_default_filter(0);
#ifdef RTCONFIG_PARENTALCTRL
			start_pc_block();
#endif
			// TODO handle multiple wan
			start_firewall(wan_primary_ifunit(), 0);
		}
		nvram_set("restart_fwl", "0");
	}
	else if (strcmp(script, "iptrestore") == 0)
	{
		// center control for iptable restore, called by process out side of rc
		_dprintf("%s: restart_iptrestore: %s.\n", __FUNCTION__, cmd[1]);
		if(cmd[1]) {
			if(action&RC_SERVICE_START){
				for ( i = 1; i <= 5; i++ ) {
					if (eval("iptables-restore", cmd[1])) {
						_dprintf("iptables-restore failed - attempt: %d ...\n", i);
						sleep(1);
					} else {
						i = 6;
					}
				}
			}
		}
	}
	else if (strcmp(script, "pppoe_relay") == 0)
	{
		int pppoerelay_unit = wan_primary_ifunit(), unit = nvram_get_int("pppoerelay_unit");

		if (nvram_match("wans_mode", "lb") && get_nr_wan_unit() > 1 &&
		    unit >= WAN_UNIT_FIRST && unit < WAN_UNIT_MAX)
			pppoerelay_unit = unit;
		if(action & RC_SERVICE_STOP) stop_pppoe_relay();
		if(action & RC_SERVICE_START) start_pppoe_relay(get_wanx_ifname(pppoerelay_unit));
	}
	else if (strcmp(script, "ntpc") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_ntpc();
		if(action & RC_SERVICE_START) start_ntpc();
	}
	else if (strcmp(script, "rebuild_cifs_config_and_password") ==0)
	{
		fprintf(stderr, "rc rebuilding CIFS config and password databases.\n");
//		regen_passwd_files(); /* Must be called before regen_cifs_config_file(). */
//		regen_cifs_config_file();
	}
	else if (strcmp(script, "time") == 0)
	{
		if(action & RC_SERVICE_STOP) {
			stop_hour_monitor_service();
			stop_telnetd();
#ifdef RTCONFIG_SSH
			stop_sshd();
#endif
			stop_logger();
			//stop_httpd();
		}
		if(action & RC_SERVICE_START) {
			setup_timezone();
			refresh_ntpc();
			start_logger();
			start_telnetd();
#ifdef RTCONFIG_SSH
			start_sshd();
#endif
			//start_httpd();
			start_firewall(wan_primary_ifunit(), 0);
			start_hour_monitor_service();
		}
	}
	else if (strcmp(script, "wps_method")==0)
	{
		if(action & RC_SERVICE_STOP) {
			stop_wps_method();
			if (!nvram_match("wps_ign_btn", "1"))
				kill_pidfile_s("/var/run/watchdog.pid", SIGUSR2);
		}
		if(action & RC_SERVICE_START) {
			if (!wps_band_radio_off(get_radio_band(nvram_get_int("wps_band_x"))) &&
			    !wps_band_ssid_broadcast_off(get_radio_band(nvram_get_int("wps_band_x")))) {
				start_wps_method();
				if(!nvram_match("wps_ign_btn", "1"))
					kill_pidfile_s("/var/run/watchdog.pid", SIGUSR1);
				else
					kill_pidfile_s("/var/run/watchdog.pid", SIGTSTP);
			}
			nvram_unset("wps_ign_btn");
		}
	}
#if defined(RTCONFIG_AMAS) && defined(CONFIG_BCMWL5)
	else if (strcmp(script, "wps_enr")==0)
	{
		if (is_router_mode()) {
			int unit = nvram_get_int("wps_band_x");
			char tmp[100], prefix[] = "wlXXXXXXXXXX_";
			char *ifname;

			snprintf(prefix, sizeof(prefix), "wl%d_", unit);
			nvram_set(strcat_r(prefix, "mode", tmp), "psta");
			ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
			eval("wlconf", ifname, "down");
			eval("wlconf", ifname, "up");
			eval("wlconf", ifname, "start");
			eval("wl", "-i", ifname, "disassoc");
			start_wps();
			sleep(1);

			start_wps_enr();
		}
	}
#endif
	else if (strcmp(script, "reset_wps")==0)
	{
		reset_wps();
		kill_pidfile_s("/var/run/watchdog.pid", SIGUSR2);
	}
	else if (strcmp(script, "wps")==0)
	{
		if(action & RC_SERVICE_STOP) stop_wps();
		if(action & RC_SERVICE_START) start_wps();
		kill_pidfile_s("/var/run/watchdog.pid", SIGUSR2);
	}
	else if (strcmp(script, "autodet")==0)
	{
		if(action & RC_SERVICE_STOP) stop_autodet();
		if(action & RC_SERVICE_START) start_autodet();
	}
#ifdef RTCONFIG_QCA_PLC_UTILS
	else if (strcmp(script, "plcdet")==0)
	{
		if(action & RC_SERVICE_STOP) stop_plcdet();
		if(action & RC_SERVICE_START) start_plcdet();
	}
#endif
#if defined(CONFIG_BCMWL5) \
		|| (defined(RTCONFIG_RALINK) && defined(RTCONFIG_WIRELESSREPEATER)) \
		|| defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK) \
		|| defined(RTCONFIG_QSR10G) || defined(RTCONFIG_LANTIQ)
	else if (strcmp(script, "wlcscan")==0)
	{
		if(action & RC_SERVICE_STOP) stop_wlcscan();
		if(action & RC_SERVICE_START) start_wlcscan();
	}
#endif
#ifdef RTCONFIG_WIRELESSREPEATER
	else if (strcmp(script, "wlcconnect")==0)
	{
		if(action & RC_SERVICE_STOP) stop_wlcconnect();

#ifdef WEB_REDIRECT
		_dprintf("%s: notify wanduck: wlc_state=%d.\n", __FUNCTION__, nvram_get_int("wlc_state"));
		// notify the change to wanduck.
		kill_pidfile_s("/var/run/wanduck.pid", SIGUSR1);
#endif

		if(action & RC_SERVICE_START) {
			restart_wireless();
			sleep(1);
#if defined(RTCONFIG_AMAS)
			start_amas_wlcconnect();
			start_amas_bhctrl();
#else				
			start_wlcconnect();
#endif
		}
	}
	else if (strcmp(script, "wlcmode")==0)
	{
		if(cmd[1]&& (atoi(cmd[1]) != nvram_get_int("wlc_mode"))) {
			nvram_set_int("wlc_mode", atoi(cmd[1]));
			if(nvram_match("lan_proto", "dhcp") && atoi(cmd[1])==0) {
				nvram_set("lan_ipaddr", nvram_default_get("lan_ipaddr"));
			}

#if defined(RTCONFIG_SAMBASRV) && defined(RTCONFIG_FTP)
			stop_ftpd();
			stop_samba();
#endif
#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_PRINTER)
			stop_lpd();
			stop_u2ec();
#endif
#ifdef RTCONFIG_JFFS2USERICON
			stop_lltdc();
#endif
			stop_networkmap();
			stop_httpd();
			stop_telnetd();
#ifdef RTCONFIG_SSH
			stop_sshd();
#endif
#ifdef RTCONFIG_NEW_USER_LOW_RSSI
			stop_roamast();
#endif
#ifdef RTCONFIG_CFGSYNC
			stop_cfgsync();
#endif
			stop_dnsmasq();
			stop_lan_wlc();
			stop_lan_port();
			stop_lan_wlport();
			start_lan_wlport();
			start_lan_port(8);
			start_lan_wlc();
			start_dnsmasq();
			start_httpd();
			start_telnetd();
#ifdef RTCONFIG_NEW_USER_LOW_RSSI
			start_roamast();
#endif

#ifdef RTCONFIG_SSH
			start_sshd();
#endif
#ifdef RTCONFIG_JFFS2USERICON
			start_lltdc();
#endif
			start_networkmap(0);
#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_PRINTER)
			start_usblpsrv();
#endif
#if defined(RTCONFIG_SAMBASRV) && defined(RTCONFIG_FTP)
			setup_passwd();
			start_samba();
			start_ftpd();
#endif
		}
	}
#endif
	else if (strcmp(script, "restore") == 0) {
		if(cmd[1]) restore_defaults_module(cmd[1]);
	}
	else if (strcmp(script, "chpass") == 0) {
			setup_passwd();
	}
#if RTCONFIG_SPEEDTEST
	else if (strcmp(script, "speedtest") == 0) {
		wan_bandwidth_detect();
	}
#endif
	// handle button action
	else if (strcmp(script, "wan_disconnect")==0) {
		logmessage("wan", "disconnected manually");
		stop_upnp();
		stop_wan();
	}
	else if (strcmp(script,"wan_connect")==0)
	{
		logmessage("wan", "connected manually");

		rename("/tmp/ppp/log", "/tmp/ppp/log.~");
		start_wan();
#ifdef HND_ROUTER
		if (is_router_mode()) start_mcpd_proxy();
#endif
		sleep(2);
		// TODO: function to force ppp connection
		start_upnp();
	}
#ifdef RTCONFIG_SNMPD
	else if (strcmp(script, "snmpd") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_snmpd();
		if(action & RC_SERVICE_START) {
			start_snmpd();
			start_firewall(wan_primary_ifunit(), 0);
		}
	}
#endif

#ifdef RTCONFIG_OPENVPN
	else if (strncmp(script, "vpnclient", 9) == 0) {
		if (action & RC_SERVICE_STOP) stop_ovpn_client(atoi(&script[9]));
		if (action & RC_SERVICE_START) start_ovpn_client(atoi(&script[9]));
	}
	else if (strncmp(script, "vpnserver" ,9) == 0) {
		if (action & RC_SERVICE_STOP) stop_ovpn_server(atoi(&script[9]));
		if (action & RC_SERVICE_START) start_ovpn_server(atoi(&script[9]));
	}
	else if (strncmp(script, "vpnrouting" ,10) == 0) {
		if (action & RC_SERVICE_START) update_ovpn_routing(atoi(&script[10]));
	}
#endif
#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
	else if (strcmp(script, "vpnd") == 0 || strcmp(script, "pptpd") == 0) {
		if (action & RC_SERVICE_STOP){
			stop_pptpd();
		}
		if (action & RC_SERVICE_START){
			start_pptpd();
			start_firewall(wan_primary_ifunit(), 0);
		}
	}
#endif
#if defined(RTCONFIG_OPENVPN)
	else if (strcmp(script, "openvpnd") == 0)
	{
		int openvpn_unit = nvram_get_int("vpn_server_unit");
		if (action & RC_SERVICE_STOP){
			stop_ovpn_server(openvpn_unit);
		}
		if (action & RC_SERVICE_START){
 			start_ovpn_server(openvpn_unit);
 		}
 	}
	else if (strncmp(script, "clearvpnserver", 14) == 0)
	{
		reset_ovpn_setting(OVPN_TYPE_SERVER, nvram_get_int("vpn_server_unit"));
	}
        else if (strncmp(script, "clearvpnclient", 14) == 0)
	{
                reset_ovpn_setting(OVPN_TYPE_CLIENT, nvram_get_int("vpn_client_unit"));
	}
#endif
#ifdef RTCONFIG_YANDEXDNS
	else if (strcmp(script, "yadns") == 0)
	{
		if (action & RC_SERVICE_STOP)
			stop_dnsmasq();
		if (action & RC_SERVICE_START) {
			update_resolvconf();
 			start_dnsmasq();
 		}
		start_firewall(wan_primary_ifunit(), 0);
	}
#endif
#ifdef RTCONFIG_DNSFILTER
	else if (strcmp(script, "dnsfilter") == 0)
	{
		if(action & RC_SERVICE_START) {
			start_dnsmasq();
			start_firewall(wan_primary_ifunit(), 0);
		}
	}
#endif
#ifdef RTCONFIG_ISP_METER
	else if (strcmp(script, "isp_meter") == 0) {
		_dprintf("%s: isp_meter: %s\n", __FUNCTION__, cmd[1]);
		if(strcmp(cmd[1], "down")==0) {
			stop_wan_if(0);
			update_wan_state("wan0_", WAN_STATE_STOPPED, WAN_STOPPED_REASON_METER_LIMIT);
		}
		else if(strcmp(cmd[1], "up")==0) {
			_dprintf("notify wan up!\n");
			start_wan_if(0);
		}
	}
#endif

#ifdef RTCONFIG_TIMEMACHINE
	else if (strcmp(script, "timemachine") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_timemachine();
		if(action & RC_SERVICE_START) start_timemachine();
	}
	else if (strcmp(script, "afpd") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_afpd();
		if(action & RC_SERVICE_START) start_afpd();
	}
	else if (strcmp(script, "cnid_metad") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_cnid_metad();
		if(action & RC_SERVICE_START) start_cnid_metad();
	}
#endif
#if defined(RTCONFIG_MDNS)
	else if (strcmp(script, "mdns") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_mdns();
		if(action & RC_SERVICE_START) start_mdns();
	}
#endif

#ifdef RTCONFIG_PUSH_EMAIL
	else if (strcmp(script, "sendmail") == 0)
	{
		start_DSLsendmail();
	}
#ifdef RTCONFIG_DBLOG
	else if (strcmp(script, "senddblog") == 0)
	{
		if(cmd[1])
		{
			start_senddblog(cmd[1]);
		}
		else
		{
			printf("Usage: rc rc_service start_senddblog log_file\n");
		}
	}
#endif /* RTCONFIG_DBLOG */
#ifdef RTCONFIG_DSL_TCLINUX
	else if (strcmp(script, "DSLsenddiagmail") == 0)
	{
		start_DSLsenddiagmail();
	}
#endif
#endif

#ifdef RTCONFIG_VPNC
#if defined(RTCONFIG_VPN_FUSION)
	else if (strcmp(script, "vpnc") == 0)	//Activate/inactivate a VPN profile
	{
		int vpnc_unit = nvram_get_int("vpnc_unit");

		_dprintf("[%s, %d]vpnc_unit=%d\n", __FUNCTION__, __LINE__, vpnc_unit);
		//init
		vpnc_init();

		if(action & RC_SERVICE_STOP)
		{
			_dprintf("[%s, %d]stop vpnc %d\n", __FUNCTION__, __LINE__, vpnc_unit);
			stop_vpnc_by_unit(vpnc_unit);
		}

		if(action & RC_SERVICE_START)
		{
			_dprintf("[%s, %d]start vpnc %d\n", __FUNCTION__, __LINE__, vpnc_unit);
			start_vpnc_by_unit(vpnc_unit);
		}
	}
	else if (strcmp(script, "default_wan") == 0)	//change default WAN
	{
		_dprintf("[%s, %d]change default wan\n", __FUNCTION__, __LINE__);

		//init vpnc profile list
		vpnc_init();

		change_default_wan();
	}
	else if (strcmp(script, "vpnc_dev_policy") == 0)
	{
		vpnc_set_dev_policy_rule();
	}
#endif	//endif defined(RTCONFIG_VPN_FUSION)
	else if (strcmp(script, "vpncall") == 0)
	{
#if defined(RTCONFIG_OPENVPN)
		char buf[32] = {0};
		int i;
		int openvpnc_unit = nvram_get_int("vpn_client_unit");
#endif
		if (action & RC_SERVICE_STOP){
			stop_vpnc();
#if defined(RTCONFIG_OPENVPN)
			for( i = 1; i <= OVPN_CLIENT_MAX; i++ )
			{
				sprintf(buf, "vpnclient%d", i);
				if ( pidof(buf) >= 0 )
				{
					stop_ovpn_client(i);
				}
			}
#endif
		}

		if (action & RC_SERVICE_START){
#ifdef RTCONFIG_DUALWAN
			set_load_balance();
#endif
#if defined(RTCONFIG_OPENVPN)
			if(nvram_match("vpnc_proto", "openvpn")){
				if (check_ovpn_client_enabled(openvpnc_unit)) {
					start_ovpn_client(openvpnc_unit);
				}
				stop_vpnc();
			}
			else{
				for( i = 1; i <= OVPN_CLIENT_MAX; i++ )
				{
					sprintf(buf, "vpnclient%d", i);
					if ( pidof(buf) >= 0 )
					{
						stop_ovpn_client(i);
					}
				}
#endif
				start_vpnc();
#if defined(RTCONFIG_OPENVPN)
			}
#endif
		}
	}
#endif
#ifdef RTCONFIG_TR069
	else if (strncmp(script, "tr", 2) == 0) {
		if (action & RC_SERVICE_STOP) stop_tr();
		if (action & RC_SERVICE_START) start_tr();
	}
#endif
	else if (strcmp(script, "sh") == 0) {
		_dprintf("%s: shell: %s\n", __FUNCTION__, cmd[1]);
		if(cmd[1]) system(cmd[1]);
	}

	else if (strcmp(script, "rstats") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_rstats();
		if(action & RC_SERVICE_START) restart_rstats();
	}
#if !defined(HND_ROUTER)
        else if (strcmp(script, "cstats") == 0)
        {
                if(action & RC_SERVICE_STOP) stop_cstats();
                if(action & RC_SERVICE_START) restart_cstats();
        }
#endif
	else if (strcmp(script, "conntrack") == 0)
	{
		setup_conntrack();
		setup_udp_timeout(TRUE);
//            start_firewall(wan_primary_ifunit(), 0);
	}
	else if (strcmp(script, "leds") == 0) {
		setup_leds();
	}
	else if (strcmp(script, "updateresolv") == 0) {
		update_resolvconf();
	}
	else if (strcmp(script, "app") == 0) {
#if defined(RTCONFIG_APP_PREINSTALLED) || defined(RTCONFIG_APP_NETINSTALLED)
		if(action & RC_SERVICE_STOP)
			stop_app();
#endif
	}
#ifdef RTCONFIG_USBRESET
	else if (strcmp(script, "usbreset") == 0) {
#define MAX_USBRESET_NUM 5
		char reset_seconds[] = {2, 4, 6, 8, 10};
		char *usbreset_active = nvram_safe_get("usbreset_active");
		char *usbreset_num = nvram_safe_get("usbreset_num");
		char buf[4];
		int reset_num = 0;
_dprintf("test 1. usbreset_active=%s, usbreset_num=%s.\n", usbreset_active, usbreset_num);

		if(strlen(usbreset_num) > 0 && strlen(usbreset_active) > 0 && strcmp(usbreset_active, "0")){
			reset_num = atoi(usbreset_num);
			if(reset_num < MAX_USBRESET_NUM){
				stop_usb_program(1);

_dprintf("test 2. turn off the USB power during %d seconds.\n", reset_seconds[reset_num]);
				set_pwr_usb(0);
				sleep(reset_seconds[reset_num]);

				++reset_num;
				memset(buf, 0, 4);
				sprintf(buf, "%d", reset_num);
				nvram_set("usbreset_num", buf);
				nvram_set("usbreset_active", "0");

				set_pwr_usb(1);
			}
		}
	}
#endif
#if defined (RTCONFIG_USB_XHCI)
#ifdef RTCONFIG_XHCIMODE
	else if(!strcmp(script, "xhcimode")){
		char param[32];
		int usb2enable = nvram_get_int("usb_usb2");
		int uhcienable = nvram_get_int("usb_uhci");
		int ohcienable = nvram_get_int("usb_ohci");
		int i;

		_dprintf("xhcimode: stop_usb_program...\n");
		stop_usb_program(1);

		_dprintf("xhcimode: remove xhci...\n");
#if defined(RTCONFIG_SOC_IPQ8064)
		modprobe_r("dwc3-ipq");
		modprobe_r("udc-core");
#endif
		modprobe_r(USB30_MOD);

		if(usb2enable){
			_dprintf("xhcimode: remove ehci...\n");
			modprobe_r(USB20_MOD);
		}

		if(ohcienable){
			_dprintf("xhcimode: remove ohci...\n");
			modprobe_r(USBOHCI_MOD);
		}

		if(uhcienable){
			_dprintf("xhcimode: remove uhci...\n");
			modprobe_r(USBUHCI_MOD);
		}

		// It's necessary to wait the device being ready.
		int sec = nvram_get_int("xhcimode_waitsec");
		_dprintf("xhcimode: sleep %d second...\n", sec);
		sleep(sec);

		memset(param, 0, 32);
		sprintf(param, "usb2mode=%s", cmd[1]);
		_dprintf("xhcimode: insert xhci %s...\n", param);
		modprobe(USB30_MOD, param);
#if defined(RTCONFIG_SOC_IPQ8064)
		modprobe("udc-core");
		modprobe("dwc3-ipq");
#endif

		if(usb2enable){
			i = nvram_get_int("usb_irq_thresh");
			if(i < 0 || i > 6)
				i = 0;
			memset(param, 0, 32);
			sprintf(param, "log2_irq_thresh=%d", i);
			_dprintf("xhcimode: insert ehci %s...\n", param);
			modprobe(USB20_MOD, param);
		}

		if(ohcienable){
			_dprintf("xhcimode: insert ohci...\n");
			modprobe(USBOHCI_MOD);
		}

		if(uhcienable){
			_dprintf("xhcimode: insert uhci...\n");
			modprobe(USBUHCI_MOD);
		}
	}
#endif
#endif
	else if (strcmp(script, "lltd") == 0) {
		if(action&RC_SERVICE_STOP) stop_lltd();
		if(action&RC_SERVICE_START) start_lltd();
	}
#ifdef RTCONFIG_JFFS2USERICON
	else if (strcmp(script, "lltdc") == 0) {
		if(action&RC_SERVICE_START) start_lltdc();
	}
#endif
#ifdef RTCONFIG_UPNPC
	else if (strcmp(script, "miniupnpc") == 0) {
		if(action&RC_SERVICE_STOP) stop_miniupnpc();
		if(action&RC_SERVICE_START) start_miniupnpc();
	}
#endif
#ifdef RTCONFIG_TOR
	else if (strcmp(script, "tor") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_Tor_proxy();
		if(action & RC_SERVICE_START) start_Tor_proxy();
		start_firewall(wan_primary_ifunit(), 0);
	}
#endif
#ifdef RTCONFIG_CLOUDCHECK
        else if(!strcmp(script, "cloudcheck")){
                if(action & RC_SERVICE_STOP) stop_cloudcheck();
                if(action & RC_SERVICE_START) start_cloudcheck();
        }
#endif
#ifdef RTCONFIG_GETREALIP
	else if(!strcmp(script, "getrealip")){
		char tmp[128], prefix[] = "wlXXXXXXXXXX_";
		wan_unit = atoi(cmd[1]);
		char *getip[] = {"getrealip.sh", NULL};
		pid_t pid;

		snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);

		if(action & RC_SERVICE_STOP){
			nvram_set(strcat_r(prefix, "realip_state", tmp), "0");
			nvram_set(strcat_r(prefix, "realip_ip", tmp), "");
		}
		if(action & RC_SERVICE_START){
			_eval(getip, ">>/dev/null", 0, &pid);
		}
	}
#endif
#ifdef RTCONFIG_QCA_PLC_UTILS
	else if (!strcmp(script, "plc_upgrade")) {
		if (nvram_match("asus_mfg", "0"))
			save_plc_setting();
	}
#endif
#ifdef RTCONFIG_KEY_GUARD
	else if (!strcmp(script, "key_guard")) {
		start_keyguard();
	}
#endif
	else if (!strcmp(script, "eco_guard")) {
		start_ecoguard();
	}
#if defined(RTCONFIG_IPSEC)
        else if(0 == strcmp(script, "ipsec_set")){
            rc_ipsec_set(IPSEC_SET,PROF_SVR);
			start_dnsmasq();
        } else if(0 == strcmp(script, "ipsec_start")){
            rc_ipsec_set(IPSEC_START,PROF_SVR);
			start_dnsmasq();
        } else if(0 == strcmp(script, "ipsec_stop")){
            rc_ipsec_set(IPSEC_STOP,PROF_SVR);
        } else if(0 == strcmp(script, "ipsec_set_cli")){
            rc_ipsec_set(IPSEC_SET,PROF_CLI);
        } else if(0 == strcmp(script, "ipsec_start_cli")){
            rc_ipsec_set(IPSEC_START,PROF_CLI);
        } else if(0 == strcmp(script, "ipsec_stop_cli")){
            rc_ipsec_set(IPSEC_STOP,PROF_CLI);
        } else if(strcmp(script, "generate_ca") == 0) {
		char mode[SZ_4BUF];
		char profile[SZ_TMP];
		char upload_type[SZ_BUF];
		char upload_value[SZ_MAX];
		char filename[SZ_64BUF];
		char file_path[SZ_128BUF];
                int idx = 0;
		memset(mode, '\0', sizeof(char) * SZ_4BUF);
		memset(profile, '\0', sizeof(char) * SZ_TMP);
		memset(upload_type, '\0', sizeof(char) * SZ_BUF);
		memset(upload_value, '\0', sizeof(char) * SZ_MAX);
		memset(filename, '\0', sizeof(char) * SZ_64BUF);
		memset(file_path, '\0', sizeof(char) * SZ_128BUF);

		sprintf(mode, "%s", nvram_safe_get("ca_manage_mode"));
		switch (atoi(mode)) {
                    case CA_MANUAL_GEN :
                        idx = rc_ipsec_ca_gen();
                        rc_ipsec_pki_gen_exec(idx);
                    break;
                    case CA_IMPORT :
                        DBG(("Import CA\n"));
                        sprintf(upload_type, "%s",
                            nvram_safe_get("ca_manage_upload_type"));
                        FILE *fp;
                        switch (atoi(upload_type)) {
                            case CERT_PRIVATEKEY :
                                sprintf(filename, "%s",
                                      nvram_safe_get("ca_manage_file_name_ca"));
                                sprintf(file_path, "/jffs/ca_files/%s",
                                        filename);
			        sprintf(upload_value, "%s",
                                      nvram_safe_get("ca_manage_upload_ca"));

                                if((fp = fopen(file_path, "w")) != NULL) {
                                    fprintf(fp, "%s", upload_value);
                                    fclose(fp);
                                }
                                rc_ipsec_cert_import(filename, filename,
                                                     NULL, NULL);
                                memset(filename, '\0', sizeof(char) * SZ_64BUF);
                                memset(file_path, '\0',
                                       sizeof(char) * SZ_128BUF);
                                memset(upload_value, '\0',
                                       sizeof(char) * SZ_MAX);
                                sprintf(filename, "%s",
                                        nvram_safe_get(
                                            "ca_manage_file_name_private_key")
                                       );
                                sprintf(file_path, "/jffs/ca_files/%s",
                                        filename);
                                sprintf(upload_value, "%s",
                                nvram_safe_get("ca_manage_upload_private_key"));
                                if((fp = fopen(file_path, "w")) != NULL) {
                                    fprintf(fp, "%s", upload_value);
                                    fclose(fp);
                                }
                                rc_ipsec_cert_import(NULL, NULL,
                                                     filename, NULL);
                            break;
                            case P12 :
                                sprintf(filename, "%s",
                                     nvram_safe_get("ca_manage_file_name_p12"));
                                sprintf(file_path, "/jffs/ca_files/%s",
                                        filename);
                                memcpy(upload_value,
                                       nvram_safe_get("ca_manage_upload_p12"),
                                       sizeof(char) * SZ_MAX);
                                DBG(("file_path: %s\n", file_path));
                                DBG(("verify code: %s\n",
                                     nvram_safe_get("ca_manage_profile")));
                                if((fp = fopen(file_path, "w")) != NULL) {
                                    //fprintf(fp, "%s", upload_value);
                                    fwrite(upload_value, sizeof(char),
                                           sizeof(char) * SZ_MAX, fp);
                                    fclose(fp);
				}
                                rc_ipsec_cert_import(NULL, NULL,
                                                     NULL, filename);
                            break;
			}
                    break;
                    case CA_AUTO_GEN :
                        idx = rc_ipsec_ca_gen();
                        rc_ipsec_pki_gen_exec(idx);
                    break;
                }

                nvram_set("ca_manage_mode", "");
                nvram_set("ca_manage_profile", "");
                nvram_unset("ca_manage_file_name_ca");
                nvram_unset("ca_manage_file_name_private_key");
                nvram_unset("ca_manage_file_name_p12");
                nvram_unset("ca_manage_upload_ca");
                nvram_unset("ca_manage_upload_private_key");
                nvram_unset("ca_manage_upload_p12");
                nvram_commit();
	}
#endif /*the end of #if defined(RTCONFIG_IPSEC)*/
#ifdef RTCONFIG_QUAGGA
	else if (strcmp(script, "quagga") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_quagga();
		if(action & RC_SERVICE_START) start_quagga();
	}
#endif
#ifdef RTCONFIG_CAPTIVE_PORTAL
	else if (strcmp(script, "uam_srv") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_uam_srv();
		if(action&RC_SERVICE_START) start_uam_srv();
	}
	else if (strcmp(script, "set_captive_portal_wl") == 0)
	{
		_dprintf("set_captive_portal_wl: start\n");
		set_captive_portal_wl();
		_dprintf("set_captive_portal_wl: end\n");
	}
	else if (strcmp(script, "set_captive_portal_adv_wl") == 0)
	{
		_dprintf("set_captive_portal_adv_wl: start\n");
		set_captive_portal_adv_wl();
		_dprintf("set_captive_portal_adv_wl: end\n");
	}
	else if (strcmp(script, "overwrite_captive_portal_ssid") == 0)
	{
		_dprintf("overwrite_captive_portal_ssid: start\n");
		overwrite_captive_portal_ssid();
		_dprintf("overwrite_captive_portal_ssid: end\n");
	}
	else if (strcmp(script, "overwrite_captive_portal_adv_ssid") == 0)
	{
		_dprintf("overwrite_captive_portal_adv_ssid: start\n");
		overwrite_captive_portal_adv_ssid();
		_dprintf("overwrite_captive_portal_adv_ssid: end\n");
	}
#endif
#ifdef RTCONFIG_FBWIFI
	else if (strcmp(script, "set_fbwifi_profile") == 0)
	{
		_dprintf("set_fbwifi_profile: start\n");
		set_fbwifi_profile();
		_dprintf("set_fbwifi_profile: end\n");
	}
	else if (strcmp(script, "overwrite_fbwifi_ssid") == 0)
	{
		_dprintf("overwrite_fbwifi_ssid: start\n");
		overwrite_fbwifi_ssid();
		_dprintf("overwrite_fbwifi_ssid: end\n");
	}
#endif
#ifdef RTCONFIG_LETSENCRYPT
	else if (strcmp(script, "letsencrypt") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_letsencrypt();
		if(action & RC_SERVICE_START) start_letsencrypt();
	}
#endif
#ifdef RTCONFIG_DISABLE_NETWORKMAP
	else if(!strcmp(script, "networkmap")){
		if(action & RC_SERVICE_STOP) stop_networkmap();
		if(action & RC_SERVICE_START) start_networkmap(0);;
	}
#endif
#ifdef RTCONFIG_FBWIFI
	else if (strcmp(script, "fbwifi")==0)
	{
		if(action&RC_SERVICE_STOP) stop_fbwifi();
		if(action&RC_SERVICE_START) start_fbwifi();
	}
	else if (strcmp(script, "set_fbwifi_profile") == 0)
	{
		_dprintf("set_fbwifi_profile: start\n");
		set_fbwifi_profile();
		_dprintf("set_fbwifi_profile: end\n");
	}
	else if (strcmp(script, "overwrite_fbwifi_ssid") == 0)
	{
		_dprintf("overwrite_fbwifi_ssid: start\n");
		overwrite_fbwifi_ssid();
		_dprintf("overwrite_fbwifi_ssid: end\n");
	}
#endif
#ifdef RTCONFIG_CFGSYNC
	else if (strcmp(script, "cfgsync")==0)
	{
		if(action&RC_SERVICE_STOP) stop_cfgsync();
		if(action&RC_SERVICE_START) start_cfgsync();
	}
	else if (strcmp(script, "release_note")==0)
	{
		snprintf(nvtmp, sizeof(nvtmp), "webs_note.sh");
		cmd[0] = nvtmp;
		start_script(count, cmd);
	}
#endif
#ifdef RTCONFIG_AMAS
	else if (strcmp(script,"amas_bhctrl") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_amas_bhctrl();
		if(action&RC_SERVICE_START) start_amas_bhctrl();	
	}
	else if (strcmp(script,"amas_wlcconnect") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_amas_wlcconnect();
		if(action&RC_SERVICE_START) start_amas_wlcconnect();	
	}
	else if (strcmp(script,"amas_lanctrl") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_amas_lanctrl();
		if(action&RC_SERVICE_START) start_amas_lanctrl();	
	}
	else if (strcmp(script,"amas_lldpd") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_amas_lldpd();
		if(action&RC_SERVICE_START) start_amas_lldpd();	
	}
#endif
#ifdef RTCONFIG_HD_SPINDOWN
#ifdef LINUX26
	else if (strcmp(script, "usb_idle") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_usb_idle();
		if(action & RC_SERVICE_START) start_usb_idle();
	}
#endif
#endif
#ifdef BLUECAVE
	else if (strcmp(script, "reset_led") == 0)
	{
		reset_led();
	}
#endif
	else if (strcmp(script, "clean_web_history") == 0)
	{
		remove("/jffs/.sys/WebHistory/WebHistory.db");
	}
	else if (strcmp(script, "clean_traffic_analyzer") == 0)
	{
		remove("/jffs/.sys/TrafficAnalyzer/TrafficAnalyzer.db");
	}
	else if (strcmp(script, "clean_backup_log") == 0)
	{
		// TODO : add path here
	}
 	else
	{
		fprintf(stderr,
			"WARNING: rc notified of unrecognized event `%s'.\n",
					script);
	}

skip:
	if(nvptr){
_dprintf("goto again(%d)...\n", getpid());
		goto again;
	}

#ifdef RTCONFIG_USB_MODEM
	if(!strcmp(script, "simauth")
			|| !strcmp(script, "simpin")
			|| !strcmp(script, "simpuk")
			|| !strcmp(script, "lockpin")
			|| !strcmp(script, "pwdpin")
			|| !strcmp(script, "modemscan")
			){
		unsetenv("unit");
	}
#endif

	nvram_set("rc_service", "");
	nvram_set("rc_service_pid", "");
_dprintf("handle_notifications() end\n");
}

#if defined(CONFIG_BCMWL5) \
		|| (defined(RTCONFIG_RALINK) && defined(RTCONFIG_WIRELESSREPEATER)) \
		|| defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK) \
		|| defined(RTCONFIG_QSR10G) || defined(RTCONFIG_LANTIQ)
void
start_wlcscan(void)
{
#ifdef RTCONFIG_REALTEK
	char *wlcscan_argv[] = {"wlcscan", NULL};
	pid_t pid;
#endif

	if(getpid()!=1) {
		notify_rc("start_wlcscan");
		return;
	}

	killall("wlcscan", SIGTERM);
#ifdef RTCONFIG_REALTEK
	_eval(wlcscan_argv, NULL, 0, &pid);
#else
	system("wlcscan");
#endif
}

void
stop_wlcscan(void)
{
	if(getpid()!=1) {
		notify_rc("stop_wlcscan");
		return;
	}

	killall("wlcscan", SIGTERM);
}
#endif
#ifdef RTCONFIG_WIRELESSREPEATER
void
start_wlcconnect(void)
{
	char *wlcconnect_argv[] = {"wlcconnect", NULL};
	pid_t pid;

	if(sw_mode()!=SW_MODE_REPEATER
#ifdef RTCONFIG_REALTEK
		&& !mediabridge_mode()
#endif
	)
	{
		_dprintf("Not repeater mode, do not start_wlcconnect\n");
		return;
	}

	if(getpid()!=1) {
		notify_rc("start_wlcconnect");
		return;
	}

	killall("wlcconnect", SIGTERM);

	_eval(wlcconnect_argv, NULL, 0, &pid);
}

void
stop_wlcconnect(void)
{
	if(getpid()!=1) {
		notify_rc("stop_wlcconnect");
		return;
	}

	killall("wlcconnect", SIGTERM);
}
#endif

#ifdef RTCONFIG_AMAS
void start_amas_wlcconnect(void)
{
#ifndef DISABLE_REPEATER_UI	
	char *amas_wlcconnect_argv[] = {"amas_wlcconnect", NULL};
	pid_t pid;

	
	if(!dpsr_mode()
#ifdef RTCONFIG_DPSTA
		&& !dpsta_mode()
#endif
	)
	{
		_dprintf("Not AMAS RE mode, do not start_amas_wlcconnect\n");
		return;
	}

	if(getpid()!=1) {
		notify_rc("start_amas_wlcconnect");
		return;
	}

	killall("amas_wlcconnect", SIGTERM);

	_eval(amas_wlcconnect_argv, NULL, 0, &pid);
#endif
	return;
}

void stop_amas_wlcconnect(void)
{
#ifndef DISABLE_REPEATER_UI		
	if(getpid()!=1) {
		notify_rc("stop_amas_wlcconnect");
		return;
	}

	killall("amas_wlcconnect", SIGTERM);
#endif
	return;
}

void start_amas_bhctrl(void)
{
#ifndef DISABLE_REPEATER_UI		
	char *amas_bhctrl_argv[] = {"amas_bhctrl", NULL};
	pid_t pid;

	if(!dpsr_mode()
#ifdef RTCONFIG_DPSTA
		&& !dpsta_mode()
#endif
	)
	{
		_dprintf("Not AMAS RE mode, do not start_amas_wlcconnect\n");
		return;
	}

	if(getpid()!=1) {
		notify_rc("start_amas_bhctrl");
		return;
	}

	killall("amas_bhctrl", SIGTERM);

	_eval(amas_bhctrl_argv, NULL, 0, &pid);
#endif
	return;	
}

void stop_amas_bhctrl(void)
{
#ifndef DISABLE_REPEATER_UI		
	if(getpid()!=1) {
		notify_rc("stop_amas_bhctrl");
		return;
	}

	killall("amas_bhctrl", SIGTERM);
#endif
	return;	
}
void start_amas_lanctrl(void)
{
	char *amas_lanctrl_argv[] = {"amas_lanctrl", NULL};
	pid_t pid;

	if(!dpsr_mode()
#ifdef RTCONFIG_DPSTA
		&& !dpsta_mode()
#endif
	)
	{
		_dprintf("Not AMAS RE mode, do not start_amas_lanctrl.\n");
		return;
	}

	if(getpid()!=1) {
		notify_rc("start_amas_lanctrl");
		return;
	}

	killall("amas_lanctrl", SIGTERM);

	_eval(amas_lanctrl_argv, NULL, 0, &pid);
}

void stop_amas_lanctrl(void)
{
	if(getpid()!=1) {
		notify_rc("stop_amas_lanctrl");
		return;
	}

	killall("amas_lanctrl", SIGTERM);
}

void gen_lldpd_if(char *bind_ifnames)
{
	char word[64], *next;
	int i = 0;

	/* prepare binding interface list */
#if defined(RTCONFIG_BCMARM) && defined(RTCONFIG_PROXYSTA)
	if (dpsr_mode()
#ifdef RTCONFIG_DPSTA
		|| dpsta_mode()
#endif
	)
	{		
		/* for lan_ifnames */
		foreach (word, nvram_safe_get("lan_ifnames"), next) {

			if (i == 0)
				i = 1;
			else
				bind_ifnames += sprintf(bind_ifnames, ",");
			
			bind_ifnames += sprintf(bind_ifnames, "%s", word);
		}		

		/* for sta_phy_ifnames */
		foreach (word, nvram_safe_get("sta_phy_ifnames"), next) {
			if (i == 0)
				i = 1;
			else
				bind_ifnames += sprintf(bind_ifnames, ",");

			bind_ifnames += sprintf(bind_ifnames, "%s", word);
		}	
	}
	else
#endif
	{
		/* for lan_ifnames */
		foreach (word, nvram_safe_get("lan_ifnames"), next) {
		if (i == 0)
		i = 1;
		else
		bind_ifnames += sprintf(bind_ifnames, ",");
		bind_ifnames += sprintf(bind_ifnames, "%s", word);
		}
	}

#if defined(RTCONFIG_BCMARM) && defined(RTCONFIG_PROXYSTA) && defined(RTCONFIG_DPSTA)
	if (i == 1)
		bind_ifnames += sprintf(bind_ifnames, ",");
#if defined(HND_ROUTER)
	bind_ifnames += sprintf(bind_ifnames, "wds0.*.*,wds1.*.*,wds2.*.*");
#else
	bind_ifnames += sprintf(bind_ifnames, "wds0.*,wds1.*,wds2.*");
#endif
#endif
}

void gen_lldpd_desc(char *bind_desc)
{
	int i = 0;
	char word[64], *next = NULL;

	/* for sta_ifnames */
	foreach (word, nvram_safe_get("sta_ifnames"), next) {
		if (i == 0)
			bind_desc += sprintf(bind_desc,"%d:%s,wds%d,wl%d", i, word, i, i);
		else {
			if(nvram_get_int("lldpd_dbg") == 1) 
				bind_desc += sprintf(bind_desc,"\\;%d:%s,wds%d,wl%d", i, word, i, i);
			else
				bind_desc += sprintf(bind_desc,";%d:%s,wds%d,wl%d", i, word, i, i);
		}
		i++;
	}	
}

void start_amas_lldpd(void)
{
	char bind_ifnames[128] = {0};

	memset(bind_ifnames, 0x00, sizeof(bind_ifnames));

	if(nvram_match("stop_amas_lldpd", "1")) {
		_dprintf("stop_amas_lldpd = 1, don't start amas lldpd.\n");
		return;
	}

	gen_lldpd_if(&bind_ifnames[0]);

	if (strlen(bind_ifnames) == 0) {
		_dprintf("rc: ==> not binding interface for lldpd\n");
		return;
	}

	_dprintf("rc: ==> binding interface(%s) for lldpd\n", bind_ifnames);

	stop_amas_lldpd();

	if(nvram_get_int("lldpd_dbg") == 1) {
			char exec_lldpd[1024] = {0};
			sprintf(exec_lldpd, "lldpd -L /usr/sbin/lldpcli -I %s -dddd", bind_ifnames);
			dbG("exec lldpd debug mode(%s)\n", exec_lldpd);
			system(exec_lldpd);
	}
	else
			eval("lldpd", "-L", "/usr/sbin/lldpcli", "-I", bind_ifnames);

	sleep(2);

	eval("lldpcli", "configure", "lldp", "tx-interval", "10");
	eval("lldpcli", "configure", "lldp", "tx-hold", "2");

	if(dpsta_mode()
#ifdef RTCONFIG_DPSTA
		|| dpsr_mode()
#endif
	)
	{
		if(nvram_get_int("cfg_cost") == 0 && strstr(nvram_safe_get("sta_phy_ifnames"), nvram_safe_get("amas_ifname"))){
			amas_set_cost(-1);
			return;
		}
	}
	amas_set_cost(nvram_get_int("cfg_cost"));
}

void stop_amas_lldpd(void)
{
	if(getpid()!=1) {
		notify_rc("stop_amas_lldpd");
		return;
	}

	killall_tk("lldpcli");
	killall_tk("lldpd");
}
#endif

#ifdef RTCONFIG_QCA_PLC_UTILS
void
start_plcdet(void)
{
	char *autodet_argv[] = {"autodet_plc", NULL};
	pid_t pid;

	if(getpid()!=1) {
		notify_rc("start_plcdet");
		return;
	}

	killall_tk("autodet_plc");

	_eval(autodet_argv, NULL, 0, &pid);

	_dprintf("rc: ==> start_plcdet!!!\n");
	return;
}

void
stop_plcdet(void)
{
	if(getpid()!=1) {
		notify_rc("stop_plcdet");
		return;
	}

	killall_tk("autodet_plc");
	_dprintf("rc: ==> stop_plcdet!!!\n");
}
#endif

void
start_autodet(void)
{
	char *autodet_argv[] = {"autodet", NULL};
	pid_t pid;

	if(getpid()!=1) {
		notify_rc("start_autodet");
		return;
	}

	killall_tk("autodet");

	_eval(autodet_argv, NULL, 0, &pid);
}

void
stop_autodet(void)
{
	if(getpid()!=1) {
		notify_rc("stop_autodet");
		return;
	}

	killall_tk("autodet");
}

// string = S20transmission -> return value = transmission.
int get_apps_name(const char *string)
{
	char *ptr;

	if(string == NULL)
		return 0;

	if((ptr = rindex(string, '/')) != NULL)
		++ptr;
	else
		ptr = (char*) string;
	if(ptr[0] != 'S')
		return 0;
	++ptr; // S.

	while(ptr != NULL){
		if(isdigit(ptr[0]))
			++ptr;
		else
			break;
	}

	printf("%s", ptr);

	return 1;
}

int run_app_script(const char *pkg_name, const char *pkg_action)
{
	char app_name[128];
	int restart_upnp = 0;

	if(pkg_action == NULL || strlen(pkg_action) <= 0)
		return -1;

	if (pidof("miniupnpd") != -1) {
		stop_upnp();
		restart_upnp = 1;
	}

	memset(app_name, 0, 128);
	if(pkg_name == NULL)
		strcpy(app_name, "allpkg");
	else
		strcpy(app_name, pkg_name);

	doSystem("/usr/sbin/app_init_run.sh %s %s", app_name, pkg_action);

	sleep(5);
	if (restart_upnp) start_upnp();

	return 0;
}

int start_nat_rules(void)
{
	char *fn = NAT_RULES, ln[PATH_MAX];
	struct stat s;
	int ret, retry, nat_state;

	// all rules applied directly according to currently status, wanduck help to triger those not cover by normal flow
#if defined(RTAC58U)
	if (!strncmp(nvram_safe_get("territory_code"), "CX", 2))
		;
	else
#endif
 	if (nvram_match("x_Setting", "0"))
		return stop_nat_rules();

	nat_state = nvram_get_int("nat_state");
	if (nat_state == NAT_STATE_NORMAL)
		return nat_state;

	retry = 6;
	while (lstat(NAT_RULES, &s) || (ret = S_ISLNK(s.st_mode) ? readlink(NAT_RULES, ln, sizeof(ln) - 1) : s.st_size) <= 0) {
		if (retry <= 0) {
_dprintf("nat_rule: the nat rule file was gone.\n");
			return nvram_get_int("nat_state");
		}

_dprintf("nat_rule: the nat rule file was not ready. wait %d seconds...\n", retry);
		sleep(2);
		retry--;
	}
	if (S_ISLNK(s.st_mode)) {
		ln[ret] = '\0';
		fn = ln;
	}

	ret = eval("iptables-restore", NAT_RULES);

	_dprintf("%s: apply the nat_rules (%s) state %d ret %d\n", __FUNCTION__, fn, nat_state, ret);
	logmessage("nat", "apply nat rules (%s)%s", fn, ret ? " error!" : "");

	if (ret != 0)
		return nvram_get_int("nat_state");

	nvram_set_int("nat_state", NAT_STATE_NORMAL);

	setup_ct_timeout(TRUE);
	setup_udp_timeout(TRUE);

	run_custom_script("nat-start", NULL);

	return NAT_STATE_NORMAL;
}

int stop_nat_rules(void)
{
	int ret, nat_state;

	nat_state = nvram_get_int("nat_state");
	if (nat_state == NAT_STATE_REDIRECT)
		return nat_state;

	if (!nvram_get_int("nat_redirect_enable"))
		return nat_state;

	ret = eval("iptables-restore", REDIRECT_RULES);

	_dprintf("%s: apply the redirect_rules state %d ret %d\n", __FUNCTION__, nat_state, ret);
	logmessage("nat", "apply redirect rules%s", ret ? " error!" : "");

	if (ret != 0)
		return nvram_get_int("nat_state");

	nvram_set_int("nat_state", NAT_STATE_REDIRECT);

	setup_ct_timeout(FALSE);
	setup_udp_timeout(FALSE);

	return NAT_STATE_REDIRECT;
}

#ifdef RTCONFIG_TOAD
static void
start_toads(void)
{
	char toad_ifname[16];
	char *next;

	stop_toads();

	foreach(toad_ifname, nvram_safe_get("toad_ifnames"), next) {
		eval("/usr/sbin/toad", "-i", toad_ifname);
	}
}

static void
stop_toads(void)
{
	killall_tk("toad");
}
#endif

#if defined(BCM_BSD)
int start_bsd(void)
{
	int ret = 0;

	stop_bsd();

	if (!nvram_get_int("smart_connect_x"))
		ret = -1;
	else {
#if 0
		nvram_unset("bsd_ifnames");
#endif
		ret = eval("/usr/sbin/bsd");
	}

	return ret;
}

void stop_bsd(void)
{
	killall_tk("bsd");
}
#endif /* BCM_BSD */

#if defined(LANTIQ_BSD)
static pid_t bsd_pid = 0;
int start_bsd(void)
{
	int ret = 0;
	char *bsd_argv[] = { "/usr/lib/fapi_wlan_beerock_cli", "BAND_STEERING", "-50", "-70", "3", "10", "10", NULL};
	stop_bsd();

	if (!nvram_get_int("smart_connect_x"))
		ret = -1;
	else {
                ret = _eval(bsd_argv, NULL, 0, &bsd_pid);
	}

	return ret;
}

void stop_bsd(void)
{
	if(bsd_pid) {
		kill(bsd_pid, SIGTERM);
		bsd_pid = 0;
	}
}
#endif /* LANTIQ_BSD */

#ifdef BCM_APPEVENTD
int start_appeventd(void)
{
	int ret = 0;
	char *appeventd_argv[] = {"/usr/sbin/appeventd", NULL};
	pid_t pid;

	if (nvram_match("appeventd_enable", "1"))
		ret = _eval(appeventd_argv, NULL, 0, &pid);

	return ret;
}

void stop_appeventd(void)
{
	killall_tk("appeventd");
}
#endif /* BCM_APPEVENTD */

#if defined(BCM_SSD)
int start_ssd(void)
{
	int ret = 0;
	char *ssd_argv[] = {"/usr/sbin/ssd", NULL};
	pid_t pid;

	stop_ssd();

	if (nvram_match("ssd_enable", "1"))
		ret = _eval(ssd_argv, NULL, 0, &pid);

	return ret;
}

void stop_ssd(void)
{
	killall_tk("ssd");
}
#endif /* BCM_SSD */

#if defined(RTCONFIG_DHDAP)
int start_dhd_monitor(void)
{
	int ret = 0;

#if defined(RTCONFIG_BCM7)
	return ret;
#endif
	killall_tk("dhd_monitor");

	nvram_set("fast_restart", "1");
	ret = eval("/usr/sbin/dhd_monitor");

	return ret;
}

int stop_dhd_monitor(void)
{
	/* Don't kill dhd_monitor here */
	return 0;
}
#endif /* RTCONFIG_DHDAP */

#ifdef RTCONFIG_INTERNAL_GOBI
// Only one modem can activate the modem's LEDs.
int start_lteled(void)
{
	char str_unit[8];
	char *lteled_argv[] = {"lteled", str_unit, NULL};
	pid_t pid;
	int modem_unit;
	int ret = 0;

	stop_lteled();

	if(nvram_get_int("lteled_down")){
#if defined(RT4GAC55U) || defined(RT4GAC68U)
		led_control(LED_LTE, LED_ON);
#elif defined(RT4GAC53U)
		led_control(LED_LTE_OFF, LED_OFF);
#endif
		led_control(LED_SIG1, LED_ON);
		led_control(LED_SIG2, LED_ON);
		led_control(LED_SIG3, LED_ON);
#if defined(RT4GAC53U)
		led_control(LED_SIG4, LED_ON);
#endif
#ifdef RT4GAC68U
		led_control(LED_3G, LED_ON);
#endif
		return 0;
	}

	for(modem_unit = MODEM_UNIT_FIRST; modem_unit < MODEM_UNIT_MAX; ++modem_unit){
#if 0
		char tmp2[100], prefix2[32];

		usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));

		if(!strcmp(nvram_safe_get(strcat_r(prefix2, "act_type", tmp2)), "gobi"))
#endif
		{
			snprintf(str_unit, 8, "%d", modem_unit);
			ret = _eval(lteled_argv, NULL, 0, &pid);
			break;
		}
	}

	return ret;
}

void stop_lteled(void)
{
	int modem_unit;
	char tmp2[100], prefix2[32];

	for(modem_unit = MODEM_UNIT_FIRST; modem_unit < MODEM_UNIT_MAX; ++modem_unit){
		usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));

		if(!strcmp(nvram_safe_get(strcat_r(prefix2, "act_type", tmp2)), "gobi")){
			killall_tk("lteled");
			break;
		}
	}
}
#endif	/* RTCONFIG_INTERNAL_GOBI */

int
firmware_check_main(int argc, char *argv[])
{
	if(argc!=2)
		return -1;

	_dprintf("FW: %s\n", argv[1]);

#ifdef RTCONFIG_DSL
#ifdef RTCONFIG_RALINK
#else
	int isTcFwExist = 0;
	isTcFwExist = separate_tc_fw_from_trx(argv[1]);
#endif
#endif

	if(!check_imagefile(argv[1])) {
		_dprintf("FW OK\n");
		nvram_set("firmware_check", "1");
	}
	else {
		_dprintf("FW Fail\n");
		nvram_set("firmware_check", "0");
	}

#ifdef RTCONFIG_DSL
#ifdef RTCONFIG_RALINK
#else
	if(isTcFwExist) {
		if(check_tc_firmware_crc()) // return 0 when pass
		{
			_dprintf("FW Fail\n");
			nvram_set("firmware_check", "0");
		}
	}
#endif
#endif

	return 0;

}

#ifdef RTCONFIG_HTTPS
int
rsasign_check_main(int argc, char *argv[])
{
	if(argc!=2)
		return -1;

	_dprintf("rsa fw: %s\n", argv[1]);

	if(check_rsasign(argv[1])) {
		_dprintf("rsasign check FW OK\n");
		nvram_set("rsasign_check", "1");
	}
	else {
		_dprintf("rsasign check FW Fail\n");
		nvram_set("rsasign_check", "0");
	}
	return 0;
}

int
rsarootca_check_main(int argc, char *argv[])
{
	if(argc!=2)
		return -1;

	_dprintf("rsa RooCA: %s\n", argv[1]);

	if(check_rsasign(argv[1])) {
		_dprintf("rsasign check RooCA OK\n");
		nvram_set("rootca_check", "1");
	}
	else {
		_dprintf("rsasign check RootCA Fail\n");
		nvram_set("rootca_check", "0");
	}
	return 0;
}
#endif

#if defined(RTCONFIG_BWDPI)
int
rsasign_sig_check_main(int argc, char *argv[])
{
	if(argc!=2)
		return -1;

	_dprintf("rsa fw: %s\n", argv[1]);

#ifdef RTCONFIG_HTTPS
	if(check_rsasign(argv[1])) {
		_dprintf("rsasign check sig OK\n");
		nvram_set("bwdpi_rsa_check", "1");
	}
	else
#endif
	{
		_dprintf("rsasign check sig Fail\n");
		nvram_set("bwdpi_rsa_check", "0");
	}
	return 0;
}
#endif

#ifdef RTCONFIG_DSL_TCLINUX
void
start_dsl_autodet(void)
{
	char *autodet_argv[] = {"auto_det", NULL};
	pid_t pid;

	if(getpid()!=1) {
		notify_rc("start_dsl_autodet");
		return;
	}

	killall_tk("auto_det");
	nvram_set("dsltmp_adslatequit", "0");
	nvram_set("dsltmp_autodet_state", "Detecting");
	sleep(1);
	_eval(autodet_argv, NULL, 0, &pid);

	return;
}

void
stop_dsl_autodet(void)
{
	if(getpid()!=1) {
		notify_rc("stop_dsl_autodet");
		return;
	}

	killall_tk("auto_det");
}
#endif

#ifdef RTCONFIG_HTTPS
int check_rsasign(char *fname)
{

    RSA *rsa_pkey = NULL;
    EVP_PKEY *pkey;
    EVP_MD_CTX ctx;
    unsigned char buffer[16*1024];
    size_t len;
    unsigned char *sig;
    unsigned int siglen;
    struct stat stat_buf;

	FILE * publicKeyFP;
	FILE * dataFileFP;
	FILE * sigFileFP;

	publicKeyFP = fopen( "/usr/sbin/public.pem", "r" );
    	if (publicKeyFP == NULL){
    	    _dprintf( "Open publicKeyFP failure\n" );
    	    return 0;
    	}

    if (!PEM_read_RSA_PUBKEY(publicKeyFP, &rsa_pkey, NULL, NULL)) {
        _dprintf("Error loading RSA public Key File.\n");
        return 0;
    }

	fclose(publicKeyFP);

    pkey = EVP_PKEY_new();
    if (!EVP_PKEY_assign_RSA(pkey, rsa_pkey)) {
        _dprintf("EVP_PKEY_assign_RSA: failed.\n");
        return 0;
    }

	sigFileFP = fopen( "/tmp/rsasign.bin", "r" );
    	if (sigFileFP == NULL){
    	    _dprintf( "Open sigFileFP failure\n" );
    	    return 0;
    	}

    /* Read the signature */
    if (fstat(fileno(sigFileFP), &stat_buf) == -1) {
        _dprintf("Unable to read signature \n");
        return 0;
    }

    siglen = stat_buf.st_size;
    sig = (unsigned char *)malloc(siglen);
    if (sig == NULL) {
        _dprintf("Unable to allocated %d bytes for signature\n",
            siglen);
        return 0;
    }

    if ((fread(sig, 1, siglen, sigFileFP)) != siglen) {
        _dprintf("Unable to read %d bytes for signature\n",
            siglen);
        free(sig);
        return 0;
    }
	fclose(sigFileFP);

    EVP_MD_CTX_init(&ctx);
    if (!EVP_VerifyInit(&ctx, EVP_sha1())) {
        _dprintf("EVP_SignInit: failed.\n");
        EVP_PKEY_free(pkey);
        free(sig);
        return 0;
    }

	dataFileFP = fopen( fname, "r" );
    	if (dataFileFP == NULL){
    	    _dprintf( "Open dataFileFP failure\n" );
            free(sig);
    	    return 0;
    	}

    while ((len = fread(buffer, 1, sizeof buffer, dataFileFP)) > 0) {
        if (!EVP_VerifyUpdate(&ctx, buffer, len)) {
            _dprintf("EVP_SignUpdate: failed.\n");
            EVP_PKEY_free(pkey);
	    fclose(dataFileFP);
            free(sig);
            return 0;
        }
    }

    if (ferror(dataFileFP)) {
        _dprintf("input file");
        EVP_PKEY_free(pkey);
        fclose(dataFileFP);
        free(sig);
        return 0;
    }

    fclose(dataFileFP);

    if (!EVP_VerifyFinal(&ctx, sig, siglen, pkey)) {
        _dprintf("EVP_VerifyFinal: failed.\n");
        free(sig);
        EVP_PKEY_free(pkey);
        return 0;
    }else
	_dprintf("EVP_VerifyFinal: ok.\n");

    free(sig);
    EVP_PKEY_free(pkey);
    return 1;
}
#endif

#ifdef RTCONFIG_PARENTALCTRL
void stop_pc_block(void)
{
	if (pids("pc_block"))
		killall("pc_block", SIGTERM);
}

void start_pc_block(void)
{
	char *pc_block_argv[] = {"pc_block", NULL};
	pid_t pid;
	pc_s *pc_list = NULL;

	stop_pc_block();

	get_all_pc_list(&pc_list);

	if(nvram_get_int("MULTIFILTER_ALL") != 0 && count_pc_rules(pc_list) > 0)
		_eval(pc_block_argv, NULL, 0, &pid);

	free_pc_list(&pc_list);
}
#endif

#ifdef RTCONFIG_TOR
void stop_Tor_proxy(void)
{
	if (pids("Tor"))
		killall("Tor", SIGTERM);
	sleep(1);
	remove("/tmp/torlog");

#if (defined(RTCONFIG_JFFS2)||defined(RTCONFIG_BRCM_NAND_JFFS2))
	if (f_exists("/tmp/.tordb/cached-microdesc-consensus") &&
	    !f_exists("/jffs/.tordb/cached-microdesc-consensus"))
	{
		//logmessage("Tor", "Backing up database");
		eval("cp", "-fa", "/tmp/.tordb", "/jffs/.tordb");
	}
#endif
}

void start_Tor_proxy(void)
{
	FILE *fp;
	pid_t pid;
	char *Tor_argv[] = { "Tor",
		"-f", "/tmp/torrc", "--quiet", NULL};
	char *Socksport;
	char *Transport;
	char *Dnsport;
	struct stat mdstat_jffs;
	struct passwd *pw;

	stop_Tor_proxy();

	if(!nvram_get_int("Tor_enable"))
		return;

	if ((fp = fopen("/tmp/torrc", "w")) == NULL)
		return;

#if (defined(RTCONFIG_JFFS2)||defined(RTCONFIG_BRCM_NAND_JFFS2))
	if (stat("/jffs/.tordb/cached-microdesc-consensus", &mdstat_jffs) != -1) {
		if(difftime(time(NULL), mdstat_jffs.st_mtime) > 60*60*24*7) {
			logmessage("Tor", "Removing stale DB backup");
			eval("rm", "-rf", "/jffs/.tordb");
		} else if (!f_exists("/tmp/.tordb/cached-microdesc-consensus")) {
			_dprintf("Tor: restore microdescriptor directory\n");
			pw = getpwuid(mdstat_jffs.st_uid);
			if ((pw) && (strcmp(pw->pw_name, "tor"))){
				eval("chown", "-R", "tor.tor","/jffs/.tordb");
			}
			eval("cp", "-fa", "/jffs/.tordb", "/tmp/.tordb");
			sleep(1);
		}
	}
#endif
	if ((Socksport = nvram_get("Tor_socksport")) == NULL)	Socksport = "9050";
	if ((Transport = nvram_get("Tor_transport")) == NULL)   Transport = "9040";
	if ((Dnsport = nvram_get("Tor_dnsport")) == NULL)   	Dnsport = "9053";

	fprintf(fp, "SocksPort %s\n", Socksport);
	fprintf(fp, "Log notice file /tmp/torlog\n");
	fprintf(fp, "VirtualAddrNetwork 10.192.0.0/10\n");
	fprintf(fp, "AutomapHostsOnResolve 1\n");
	fprintf(fp, "TransPort %s:%s\n", nvram_safe_get( "lan_ipaddr" ), Transport);
	fprintf(fp, "DNSPort %s:%s\n", nvram_safe_get( "lan_ipaddr" ), Dnsport);
	fprintf(fp, "RunAsDaemon 1\n");
	fprintf(fp, "DataDirectory /tmp/.tordb\n");
	fprintf(fp, "AvoidDiskWrites 1\n");
	fprintf(fp, "User tor\n");

	append_custom_config("torrc", fp);
	fclose(fp);
	use_custom_config("torrc", "/tmp/torrc");
	run_postconf("torrc", "/tmp/torrc");

	_eval(Tor_argv, NULL, 0, &pid);
}
#endif

#ifdef RTCONFIG_CLOUDCHECK
void stop_cloudcheck(void){
	if(getpid() != 1){
		notify_rc("stop_cloudcheck");
		return;
	}

	killall("booster_watchdo", SIGTERM); // only 15 characters can be identified.
}

void start_cloudcheck(void){
	char *cmd[] = {"/bin/booster_watchdog", NULL};
	pid_t pid;

	if(getpid() != 1){
		notify_rc("start_cloudcheck");
		return;
	}

	if(nvram_get_int("enable_cloudcheck") != 1)
		return;

	_eval(cmd, NULL, 0, &pid);
}
#endif

#ifdef RTCONFIG_NEW_USER_LOW_RSSI
void stop_roamast(void){
	if (pids("roamast"))
		killall_tk("roamast");
}

void start_roamast(void){
	char *cmd[] = {"roamast", NULL};
	char prefix[] = "wl_XXXXX";
	char tmp[32];
	pid_t pid;
	int i;

	stop_roamast();
#ifdef RTCONFIG_CONCURRENTREPEATER
	if (mediabridge_mode())
		return;
#endif

	for (i = 0; i <= DEV_NUMIFS; i++) {
		sprintf(prefix, "wl%d_", i);
		if( nvram_get_int(strcat_r(prefix, "user_rssi", tmp)) != 0 ) {
			_eval(cmd, NULL, 0, &pid);
			break;
		}
	}
}
#endif

#if defined(RTCONFIG_KEY_GUARD)
void stop_keyguard(void){
	if(pids("keyguard"))
		killall("keyguard", SIGTERM);
}

void start_keyguard(void){
	char *cmd[] = {"keyguard", NULL};
	pid_t pid;

	if(!nvram_get_int("kg_enable")) {
		stop_keyguard();
		return;
	}
	else {
		if(pids("keyguard"))
			killall("keyguard", SIGUSR1);
		else
			_eval(cmd, NULL, 0, &pid);
	}
}
#endif

/* for APP ECO mode changing RF to 1x1 */
void start_ecoguard(void)
{
	char *next = NULL;
	char ifname[32];

	if ( !nvram_get_int("wlready") || nvram_get_int("guard_mode") != 1)	//guard_mode 0:default 1:eco mode
		return;
	else{
		foreach(ifname, nvram_safe_get("wl_ifnames"), next) {
#if defined(RTCONFIG_RALINK)
#elif defined(RTCONFIG_QCA)
#else /* BCM */

#if defined(RTCONFIG_QTN)
#else
		eval("wl", "-i", ifname, "txchain", "1");
		eval("wl", "-i", ifname, "rxchain", "1");
		eval("wl", "-i", ifname, "down");
		eval("wl", "-i", ifname, "up");
#endif
#endif
		}
	}
}

int service_main(int argc, char *argv[])
{
	if (argc != 2) usage_exit(argv[0], "<action_service>");
	notify_rc(argv[1]);
	printf("\nDone.\n");
	return 0;
}

void setup_leds()
{
	int model;

	model = get_model();

	if (nvram_get_int("led_disable") == 1) {
		if ((model == MODEL_RTAC56U) || (model == MODEL_RTAC56S) ||
		    (model == MODEL_RTAC68U) || (model == MODEL_RTAC87U) ||
		    (model == MODEL_RTAC3200) || (model == MODEL_RTAC88U) ||
		    (model == MODEL_RTAC3100) || (model == MODEL_RTAC5300) ||
		    (model == MODEL_RTAC86U)) {
			setAllLedOff();
			if (model == MODEL_RTAC87U)
				led_control_atomic(LED_5G, LED_OFF);
		} else {        // TODO: Can other routers also use the same code?
			led_control_atomic(LED_2G, LED_OFF);
			led_control_atomic(LED_5G, LED_OFF);
			led_control_atomic(LED_POWER, LED_OFF);
			led_control_atomic(LED_SWITCH, LED_OFF);
			led_control_atomic(LED_LAN, LED_OFF);
			led_control_atomic(LED_WAN, LED_OFF);
		}
#ifdef RTCONFIG_USB
		stop_usbled();
		led_control_atomic(LED_USB, LED_OFF);
#endif

	} else {
#ifdef RTCONFIG_USB
		start_usbled();
#endif
#ifdef RTCONFIG_LED_ALL
		led_control_atomic(LED_ALL, LED_ON);
#endif

/* LAN */
#if defined(HND_ROUTER) && defined(RTCONFIG_LAN4WAN_LED)
		setLANLedOn();
#endif

/* WAN */
#if defined(RTAC3200) || defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
#ifndef HND_ROUTER
		eval("et", "-i", "eth0", "robowr", "0", "0x18", "0x01ff");
		eval("et", "-i", "eth0", "robowr", "0", "0x1a", "0x01ff");
#else
		led_control(LED_WAN_NORMAL, LED_ON);
#endif
#else
		eval("et", "robowr", "0", "0x18", "0x01ff");
		eval("et", "robowr", "0", "0x1a", "0x01ff");
#endif

/* Wifi */
		if (nvram_match("wl1_radio", "1")
#if defined(RTAC3200) || defined(RTAC5300)
		    || nvram_match("wl2_radio", "1")
#endif
		   ) {
			led_control_atomic(LED_5G_FORCED, LED_ON);
		}
		if (nvram_match("wl0_radio", "1")) {
			led_control_atomic(LED_2G, LED_ON);
		}
#ifdef RTCONFIG_QTN
		setAllLedOn_qtn();
#endif
		led_control_atomic(LED_SWITCH, LED_ON);
		led_control_atomic(LED_POWER, LED_ON);

#if defined(RTAC3200) || defined(RTAC88U) || defined(RTAC3100) || defined(RTAC5300)
		kill_pidfile_s("/var/run/wanduck.pid", SIGUSR2);
#endif
	}
}

#if !defined(HND_ROUTER)
void stop_cstats(void)
{
	int n, m;
	int pid;
	int pidz;
	int ppidz;
	int w = 0;

	n = 60;
	m = 15;
	while ((n-- > 0) && ((pid = pidof("cstats")) > 0)) {
		w = 1;
		pidz = pidof("gzip");
		if (pidz < 1) pidz = pidof("cp");
		ppidz = ppid(ppid(pidz));
		if ((m > 0) && (pidz > 0) && (pid == ppidz)) {
			syslog(LOG_DEBUG, "cstats(PID %d) shutting down, waiting for helper process to complete(PID %d, PPID %d).\n", pid, pidz, ppidz);
			--m;
		} else {
			kill(pid, SIGTERM);
		}
		sleep(1);
	}
	if ((w == 1) && (n > 0))
		syslog(LOG_DEBUG, "cstats stopped.\n");
}

void start_cstats(int new)
{
	if (nvram_match("cstats_enable", "1")) {
		stop_cstats();
		if (new) {
			syslog(LOG_DEBUG, "starting cstats (new datafile).\n");
			xstart("cstats", "--new");
		} else {
			syslog(LOG_DEBUG, "starting cstats.\n");
			xstart("cstats");
		}
	}
}

void restart_cstats(void)
{
        if (nvram_match("cstats_new", "1"))
        {
                start_cstats(1);
                nvram_set("cstats_new", "0");
		nvram_commit();		// Otherwise it doesn't get written back to mtd
        }
        else
        {
                start_cstats(0);
        }
}
#endif


// Takes one argument:  0 = update failure
//                      1 (or missing argument) = update success
int
ddns_custom_updated_main(int argc, char *argv[])
{
	if ((argc == 2 && !strcmp(argv[1], "1")) || (argc == 1)) {
		nvram_set("ddns_status", "1");
		nvram_set("ddns_updated", "1");
		nvram_set("ddns_return_code", "200");
		nvram_set("ddns_return_code_chk", "200");
		nvram_set("ddns_server_x_old", nvram_safe_get("ddns_server_x"));
		nvram_set("ddns_hostname_old", nvram_safe_get("ddns_hostname_x"));
		logmessage("ddns", "Completed custom ddns update");
	} else {
		nvram_set("ddns_return_code", "unknown_error");
		nvram_set("ddns_return_code_chk", "unknown_error");
		logmessage("ddns", "Custom ddns update failed");
	}

        return 0;
}

#ifdef RTCONFIG_CAPTIVE_PORTAL
void
set_captive_portal_wl(void) {
	char wl_unit[10];
	char wl_unit_temp[10];
	char wl_item[30];

	memset(wl_unit, 0, sizeof(wl_unit));
	memset(wl_unit_temp, 0, sizeof(wl_unit_temp));

	//setting 2.4G to wl0.X
	sprintf(wl_unit, "%s", nvram_safe_get("captive_portal_2g_if"));
	sprintf(wl_unit_temp, "%s", nvram_safe_get("captive_portal_2g_if_temp"));
	if(strcmp(wl_unit, "off")) {
		_dprintf("Set captive portal 2.4G\n");

		//ssid
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_ssid", wl_item);
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("captive_portal_2g"))) {
			nvram_set(wl_item, nvram_safe_get("captive_portal_2g"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("captive_portal_2g"));
		}

		//auth_mode_x
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_auth_mode_x", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "open")) {
			nvram_set(wl_item, "open");
			_dprintf("set %s : %s\n", wl_item, "open");
		}

		//crypto
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_crypto", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "aes")) {
			nvram_set(wl_item, "aes");
			_dprintf("set %s : %s\n", wl_item, "aes");
		}

		//wpa_psk
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_wpa_psk", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//macmode
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_macmode", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "disabled")) {
			nvram_set(wl_item, "disabled");
			_dprintf("set %s : %s\n", wl_item, "disabled");
		}

		//lanaccess
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_lanaccess", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "off")) {
			nvram_set(wl_item, "off");
			_dprintf("set %s : %s\n", wl_item, "off");
		}

		//expire
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_expire", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bss_enable
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "1")) {
			nvram_set(wl_item, "1");
			_dprintf("set %s : %s\n", wl_item, "1");
		}

		//guest_num
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_guest_num", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "10")) {
			nvram_set(wl_item, "10");
			_dprintf("set %s : %s\n", wl_item, "10");
		}

		//bw_enabled
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bw_dl
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_dl", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//bw_ul
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_ul", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//captive_portal_2g_if_temp
		if(strcmp(nvram_safe_get("captive_portal_2g_if_temp"),wl_unit)) {
			nvram_set("captive_portal_2g_if_temp", wl_unit);
			_dprintf("set captive_portal_2g_if_temp : %s\n",wl_unit);
		}

	}
	else {
		_dprintf("Reset 2G profile\n");
		//reset 2.4G profile
		memset(wl_unit, 0, sizeof(wl_unit));
		sprintf(wl_unit, "%s", nvram_safe_get("captive_portal_2g_if_temp"));
		if(strcmp(wl_unit, "off")) {
			//captive_portal_2g_if_temp
			nvram_set("captive_portal_2g_if_temp", "off");
			_dprintf("set captive_portal_2g_if_temp : %s\n", "off");

			//bss_enable
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}
	}

	//setting 5G to wl1.X
	sprintf(wl_unit, "%s", nvram_safe_get("captive_portal_5g_if"));
	sprintf(wl_unit_temp, "%s", nvram_safe_get("captive_portal_5g_if_temp"));
	if(strcmp(wl_unit, "off")) {
		_dprintf("Set captive portal 5G\n");

		//ssid
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_ssid", wl_item);
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("captive_portal_5g"))) {
			nvram_set(wl_item, nvram_safe_get("captive_portal_5g"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("captive_portal_5g"));
		}

		//auth_mode_x
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_auth_mode_x", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "open")) {
			nvram_set(wl_item, "open");
			_dprintf("set %s : %s\n", wl_item, "open");
		}

		//crypto
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_crypto", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "aes")) {
			nvram_set(wl_item, "aes");
			_dprintf("set %s : %s\n", wl_item, "aes");
		}

		//wpa_psk
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_wpa_psk", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//macmode
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_macmode", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "disabled")) {
			nvram_set(wl_item, "disabled");
			_dprintf("set %s : %s\n", wl_item, "disabled");
		}

		//lanaccess
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_lanaccess", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "off")) {
			nvram_set(wl_item, "off");
			_dprintf("set %s : %s\n", wl_item, "off");
		}

		//expire
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_expire", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bss_enable
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "1")) {
			nvram_set(wl_item, "1");
			_dprintf("set %s : %s\n", wl_item, "1");
		}

		//guest_num
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_guest_num", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "10")) {
			nvram_set(wl_item, "10");
			_dprintf("set %s : %s\n", wl_item, "10");
		}

		//bw_enabled
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bw_dl
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_dl", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//bw_ul
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_ul", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//captive_portal_2g_if_temp
		if(strcmp(nvram_safe_get("captive_portal_5g_if_temp"),wl_unit)) {
			nvram_set("captive_portal_5g_if_temp", wl_unit);
			_dprintf("set captive_portal_5g_if_temp : %s\n",wl_unit);
		}

	}
	else {
		_dprintf("Reset 5G profile\n");
		//reset 2.4G profile
		memset(wl_unit, 0, sizeof(wl_unit));
		sprintf(wl_unit, "%s", nvram_safe_get("captive_portal_5g_if_temp"));
		if(strcmp(wl_unit, "off")) {
			//captive_portal_2g_if_temp
			nvram_set("captive_portal_5g_if_temp", "off");
			_dprintf("set captive_portal_5g_if_temp : %s\n", "off");

			//bss_enable
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}
	}

	//setting 5G-2 to wl2.X
	sprintf(wl_unit, "%s", nvram_safe_get("captive_portal_5g_2_if"));
	sprintf(wl_unit_temp, "%s", nvram_safe_get("captive_portal_5g_2_if_temp"));
	if(strcmp(wl_unit, "off")) {
		_dprintf("Set captive portal 5G-2\n");

		//ssid
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_ssid", wl_item);
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("captive_portal_5g_2"))) {
			nvram_set(wl_item, nvram_safe_get("captive_portal_5g_2"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("captive_portal_5g_2"));
		}

		//auth_mode_x
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_auth_mode_x", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "open")) {
			nvram_set(wl_item, "open");
			_dprintf("set %s : %s\n", wl_item, "open");
		}

		//crypto
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_crypto", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "aes")) {
			nvram_set(wl_item, "aes");
			_dprintf("set %s : %s\n", wl_item, "aes");
		}

		//wpa_psk
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_wpa_psk", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//macmode
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_macmode", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "disabled")) {
			nvram_set(wl_item, "disabled");
			_dprintf("set %s : %s\n", wl_item, "disabled");
		}

		//lanaccess
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_lanaccess", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "off")) {
			nvram_set(wl_item, "off");
			_dprintf("set %s : %s\n", wl_item, "off");
		}

		//expire
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_expire", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bss_enable
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "1")) {
			nvram_set(wl_item, "1");
			_dprintf("set %s : %s\n", wl_item, "1");
		}

		//guest_num
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_guest_num", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "10")) {
			nvram_set(wl_item, "10");
			_dprintf("set %s : %s\n", wl_item, "10");
		}

		//bw_enabled
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bw_dl
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_dl", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//bw_ul
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_ul", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//captive_portal_2g_if_temp
		if(strcmp(nvram_safe_get("captive_portal_5g_2_if_temp"),wl_unit)) {
			nvram_set("captive_portal_5g_2_if_temp", wl_unit);
			_dprintf("set captive_portal_5g_2_if_temp : %s\n",wl_unit);
		}

	}
	else {
		_dprintf("Reset 5G-2 profile\n");
		//reset 2.4G profile
		memset(wl_unit, 0, sizeof(wl_unit));
		sprintf(wl_unit, "%s", nvram_safe_get("captive_portal_5g_2_if_temp"));
		if(strcmp(wl_unit, "off")) {
			//captive_portal_2g_if_temp
			nvram_set("captive_portal_5g_2_if_temp", "off");
			_dprintf("set captive_portal_5g_2_if_temp : %s\n", "off");

			//bss_enable
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}
	}
}

void
set_captive_portal_adv_wl(void) {
	char *buf_en_p, *buf_dis_p, *buf_en, *buf_dis, *wl_if_en, *wl_if_dis;
	char wl_item[30];

	buf_dis = buf_dis_p = strdup(nvram_safe_get("captive_portal_adv_wl_dis"));
	while (buf_dis_p) {
		if ((wl_if_dis = strsep(&buf_dis, ">")) == NULL) break;
		if(strcmp(wl_if_dis, "")) {
			//bss_enable
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_if_dis, "_bss_enabled", wl_item);
			if(strcmp(nvram_safe_get(wl_item), "0")) {
				nvram_set(wl_item, "0");
				_dprintf("set %s : %s\n", wl_item, "0");
			}
		}
	}
	nvram_set("captive_portal_adv_wl_dis", "");
	free(buf_dis_p);
	buf_en = buf_en_p = strdup(nvram_safe_get("captive_portal_adv_wl_en"));
	while (buf_en_p) {
		if ((wl_if_en = strsep(&buf_en, ">")) == NULL) break;
		if(strcmp(wl_if_en, "")) {
			//ssid
			switch(wl_if_en[2]) {
				case '0' :
					if(strcmp(nvram_safe_get("captive_portal_adv_2g_ssid"), "")) {
						memset(wl_item, 0, sizeof(wl_item));
						(void)strcat_r(wl_if_en, "_ssid", wl_item);
						if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("captive_portal_adv_2g_ssid"))) {
							nvram_set(wl_item, nvram_safe_get("captive_portal_adv_2g_ssid"));
							_dprintf("set %s : %s\n", wl_item, nvram_safe_get("captive_portal_adv_2g_ssid"));
						}
					}
					break;
				case '1' :
					if(strcmp(nvram_safe_get("captive_portal_adv_5g_ssid"), "")) {
						memset(wl_item, 0, sizeof(wl_item));
						(void)strcat_r(wl_if_en, "_ssid", wl_item);
						if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("captive_portal_adv_5g_ssid"))) {
							nvram_set(wl_item, nvram_safe_get("captive_portal_adv_5g_ssid"));
							_dprintf("set %s : %s\n", wl_item, nvram_safe_get("captive_portal_adv_5g_ssid"));
						}
					}
					break;
				case '2' :
					if(strcmp(nvram_safe_get("captive_portal_adv_5g_2_ssid"), "")) {
						memset(wl_item, 0, sizeof(wl_item));
						(void)strcat_r(wl_if_en, "_ssid", wl_item);
						if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("captive_portal_adv_5g_2_ssid"))) {
							nvram_set(wl_item, nvram_safe_get("captive_portal_adv_5g_2_ssid"));
							_dprintf("set %s : %s\n", wl_item, nvram_safe_get("captive_portal_adv_5g_2_ssid"));
						}
					}
					break;
			}

			//bss_enable
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_if_en, "_bss_enabled", wl_item);
			if(strcmp(nvram_safe_get(wl_item), "1")) {
				nvram_set(wl_item, "1");
				_dprintf("set %s : %s\n", wl_item, "1");
			}

			//auth_mode_x
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_if_en, "_auth_mode_x", wl_item);
			if(strcmp(nvram_safe_get(wl_item), "open")) {
				nvram_set(wl_item, "open");
				_dprintf("set %s : %s\n", wl_item, "open");
			}

			//crypto
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_if_en, "_crypto", wl_item);
			if(strcmp(nvram_safe_get(wl_item), "aes")) {
				nvram_set(wl_item, "aes");
				_dprintf("set %s : %s\n", wl_item, "aes");
			}

			//wpa_psk
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_if_en, "_wpa_psk", wl_item);
			if(strcmp(nvram_safe_get(wl_item), "")) {
				nvram_set(wl_item, "");
				_dprintf("set %s : %s\n", wl_item, "");
			}

			//macmode
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_if_en, "_macmode", wl_item);
			if(strcmp(nvram_safe_get(wl_item), "disabled")) {
				nvram_set(wl_item, "disabled");
				_dprintf("set %s : %s\n", wl_item, "disabled");
			}

			//lanaccess
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_if_en, "_lanaccess", wl_item);
			if(strcmp(nvram_safe_get(wl_item), "off")) {
				nvram_set(wl_item, "off");
				_dprintf("set %s : %s\n", wl_item, "off");
			}

			//expire
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_if_en, "_expire", wl_item);
			if(strcmp(nvram_safe_get(wl_item), "0")) {
				nvram_set(wl_item, "0");
				_dprintf("set %s : %s\n", wl_item, "0");
			}

			//guest_num
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_if_en, "_guest_num", wl_item);
			if(strcmp(nvram_safe_get(wl_item), "10")) {
				nvram_set(wl_item, "10");
				_dprintf("set %s : %s\n", wl_item, "10");
			}

			//bw_enabled
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_if_en, "_bw_enabled", wl_item);
			if(strcmp(nvram_safe_get(wl_item), "0")) {
				nvram_set(wl_item, "0");
				_dprintf("set %s : %s\n", wl_item, "0");
			}

			//bw_dl
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_if_en, "_bw_dl", wl_item);
			if(strcmp(nvram_safe_get(wl_item), "")) {
				nvram_set(wl_item, "");
				_dprintf("set %s : %s\n", wl_item, "");
			}

			//bw_ul
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_if_en, "_bw_ul", wl_item);
			if(strcmp(nvram_safe_get(wl_item), "")) {
				nvram_set(wl_item, "");
				_dprintf("set %s : %s\n", wl_item, "");
			}
		}
	}
	nvram_set("captive_portal_adv_wl_en", "");
	free(buf_en_p);
}
void
overwrite_captive_portal_ssid(void) {
	char wl_item[30];
	char wl_if[5];
	memset(wl_item, 0, sizeof(wl_item));
	memset(wl_if, 0, sizeof(wl_if));

	sprintf(wl_if, "wl%s.%s", nvram_safe_get("wl_unit"), nvram_safe_get("wl_subunit"));

	//ssid
	(void)strcat_r(wl_if, "_ssid", wl_item);
	switch(nvram_get_int("wl_unit")) {
		case 0 :
			nvram_set("captive_portal_2g", nvram_safe_get(wl_item));
			_dprintf("set captive_portal_2g : %s\n", nvram_safe_get(wl_item));
			break;
		case 1 :
			nvram_set("captive_portal_5g", nvram_safe_get(wl_item));
			_dprintf("set captive_portal_5g : %s\n", nvram_safe_get(wl_item));
			break;
		case 2 :
			nvram_set("captive_portal_5g_2", nvram_safe_get(wl_item));
			_dprintf("set captive_portal_5g_2 : %s\n", nvram_safe_get(wl_item));
			break;
	}
}
void
overwrite_captive_portal_adv_ssid(void) {
	char wl_item[30];
	char wl_if[5];
	memset(wl_item, 0, sizeof(wl_item));
	memset(wl_if, 0, sizeof(wl_if));

	sprintf(wl_if, "wl%s.%s", nvram_safe_get("wl_unit"), nvram_safe_get("wl_subunit"));

	//ssid
	(void)strcat_r(wl_if, "_ssid", wl_item);
	switch(nvram_get_int("wl_unit")) {
		case 0 :
			nvram_set("captive_portal_adv_2g_ssid", nvram_safe_get(wl_item));
			_dprintf("set captive_portal_adv_2g_ssid : %s\n", nvram_safe_get(wl_item));
			break;
		case 1 :
			nvram_set("captive_portal_adv_5g_ssid", nvram_safe_get(wl_item));
			_dprintf("set captive_portal_adv_5g_ssid : %s\n", nvram_safe_get(wl_item));
			break;
		case 2 :
			nvram_set("captive_portal_adv_5g_2_ssid", nvram_safe_get(wl_item));
			_dprintf("set captive_portal_adv_5g_2_ssid : %s\n", nvram_safe_get(wl_item));
			break;
	}
}

/**
 * Restart Free Wi-Fi and Captive Portal,
 * if @wan_ip/@wan_mask conflicts with Free Wi-Fi/Captive Portal's network.
 * @wan_ip:	WAN IP address of a WAN unit.
 * @wan_mask:	WAN netmask of a WAN unit.
 */
int restart_coovachilli_if_conflicts(char *wan_ip, char *wan_mask)
{
	int r;
	pid_t pid;
	char ip_mask[sizeof("192.168.100.200/255.255.255.255XXX")];
	char *restart_chilli[] = { "rc", "rc_service", "restart_chilli", NULL };
	char *restart_cp[] = { "rc", "rc_service", "restart_CP", NULL };

	if (!wan_ip || !wan_mask)
		return -1;
	/* If WAN IP conflicts with Free Wi-Fi/Captive Portal,
	 * restart Free Wi-Fi/Captive Portal in background.
	 * They will find new networks instead.
	 */
	snprintf(ip_mask, sizeof(ip_mask), "%s/%s", wan_ip, wan_mask);
	
	r = test_and_get_free_char_network(7, ip_mask, EXCLUDE_NET_ALL_EXCEPT_COOVACHILLI);
	if (r != 1)
		return r;
	//kill_pidfile_s("/var/run/chilli-cp.pid", SIGUSR2);
	//kill_pidfile_s("/var/run/chilli.pid", SIGUSR2);
	_eval(restart_chilli, NULL, 0, &pid);
	_eval(restart_cp, NULL, 0, &pid);

	return r;
}

#endif

#ifdef RTCONFIG_FBWIFI
void start_httpd_uam()
{
	char *httpd_argv[] = {"httpd_uam", NULL};
	pid_t pid;

	chdir("/www/fbwifi");


	_eval(httpd_argv, NULL, 0, &pid);
	logmessage(LOGNAME, "start httpd_uam");

	chdir("/");

	return;
}

void stop_httpd_uam()
{

	if (pids("httpd_uam"))
		killall_tk("httpd_uam");
}

void stop_fbwifi_register()
{

	if (pids("fb_wifi_register"))
		killall_tk("fb_wifi_register");
}

void clean_certificaion_rules()
{
	int count = nvram_get_int("fbwifi_user_conut");

    char name[128] = {0};
    int i = 0;
    for (i=0 ; i< count ; i++)
    {
        memset(name, 0x0, sizeof(name));
        sprintf(name, "fbwifi_user_%d", i);
        nvram_unset(name);
    }
    nvram_set_int("fbwifi_user_conut", 0);

    nvram_commit();
}

void stop_fbwifi()
{
	stop_fbwifi_check();
	clean_certificaion_rules();
	stop_httpd_uam();
	start_firewall(wan_primary_ifunit(), 0);
}

void my_mkdir(char *path)
{
    DIR *pDir;
    pDir=opendir(path);
    if(NULL == pDir)
    {
        if(-1 == mkdir(path,0777))
        {
            return ;
        }
    }
    else
        closedir(pDir);
}

void start_fbwifi()
{
	my_mkdir("/tmp/fbwifi");
	if(nvram_match("fbwifi_enable","on"))
	{
		char *gw_id = nvram_safe_get("fbwifi_id");
		_dprintf("gw_id = %s\n",gw_id);
//		if(gw_id == NULL || gw_id == "")
		if(nvram_match("fbwifi_id","off"))
		{
			_dprintf("restart_fbwifi_register\n");
			restart_fbwifi_register();
		}
		char *fbwifi_2g = nvram_safe_get("fbwifi_2g");
		char *fbwifi_5g = nvram_safe_get("fbwifi_5g");
		char *fbwifi_5g_2 = nvram_safe_get("fbwifi_5g_2");
		if(strcmp(fbwifi_2g,"off") || strcmp(fbwifi_5g,"off") || strcmp(fbwifi_5g_2,"off") )
		{
			start_fbwifi_check();
			start_httpd_uam();
		}
	}
	start_firewall(wan_primary_ifunit(), 0);
}

void restart_fbwifi()
{
	if(nvram_match("fbwifi_enable","on"))
	{
		char *gw_id = nvram_safe_get("fbwifi_id");
		_dprintf("gw_id = %s\n",gw_id);
//		if(gw_id == NULL || gw_id == "")
		if(nvram_match("fbwifi_id","off"))
		{
			stop_fbwifi_register();
			_dprintf("restart_fbwifi_register\n");

			restart_fbwifi_register();
		}

	}

}

void
set_fbwifi_profile(void) {
	char wl_unit[10];
	char wl_unit_temp[10];
	char wl_item[30];

	//backup 2.4G profile and set fbwifi profile to wl0.X
	memset(wl_unit, 0, sizeof(wl_unit));
	memset(wl_unit_temp, 0, sizeof(wl_unit_temp));

	sprintf(wl_unit, "%s", nvram_safe_get("fbwifi_2g"));
	sprintf(wl_unit_temp, "%s", nvram_safe_get("fbwifi_2g_temp"));
	if(strcmp(wl_unit, "off") && !strcmp(nvram_safe_get("fbwifi_enable"), "on")) {
		_dprintf("Backup 2G profile and set fbwifi profile\n");

		//ssid
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_ssid", wl_item);
		if(!strcmp(wl_unit_temp, "off")) { //first time enable fbwifi need backup wl0.x value
			nvram_set("fbwifi_ssid_0_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_ssid_0_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("fbwifi_ssid"))) { //set fbwifi value to wl0.x
			nvram_set(wl_item, nvram_safe_get("fbwifi_ssid"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_ssid"));
		}

		//auth_mode_x
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_auth_mode_x", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_auth_mode_x_0_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_auth_mode_x_0_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("fbwifi_auth_mode_x"))) {
			nvram_set(wl_item, nvram_safe_get("fbwifi_auth_mode_x"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_auth_mode_x"));
		}

		//crypto
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_crypto", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_crypto_0_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_crypto_0_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("fbwifi_crypto"))) {
			nvram_set(wl_item, nvram_safe_get("fbwifi_crypto"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_crypto"));
		}

		//wpa_psk
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_wpa_psk", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_wpa_psk_0_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_wpa_psk_0_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("fbwifi_wpa_psk"))) {
			nvram_set(wl_item, nvram_safe_get("fbwifi_wpa_psk"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_wpa_psk"));
		}

		//macmode
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_macmode", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_macmode_0_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_macmode_0_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), "disabled")) {
			nvram_set(wl_item, "disabled");
			_dprintf("set %s : %s\n", wl_item, "disabled");
		}

		//lanaccess
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_lanaccess", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_lanaccess_0_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_lanaccess_0_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), "off")) {
			nvram_set(wl_item, "off");
			_dprintf("set %s : %s\n", wl_item, "off");
		}

		//expire
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_expire", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_expire_0_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_expire_0_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bss_enable
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "1")) {
			nvram_set(wl_item, "1");
			_dprintf("set %s : %s\n", wl_item, "1");
		}

		//bw_enabled
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bw_dl
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_dl", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//bw_ul
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_ul", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//fbwifi_2g
		if(strcmp(nvram_safe_get("fbwifi_2g_temp"),wl_unit)) {
			nvram_set("fbwifi_2g_temp", wl_unit);
			_dprintf("set fbwifi_2g_temp : %s\n",wl_unit);
		}

	}
	else {
		_dprintf("Reset 2G profile\n");
		//reset 2.4G profile
		memset(wl_unit, 0, sizeof(wl_unit));
		sprintf(wl_unit, "%s", nvram_safe_get("fbwifi_2g_temp"));
		if(strcmp(wl_unit, "off")) {
			//fbwifi_2g
			nvram_set("fbwifi_2g_temp", "off");
			_dprintf("set fbwifi_2g_temp : %s\n", "off");

			//ssid
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_ssid", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_ssid_0_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_ssid_0_temp"));

			//auth_mode_x
			 memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_auth_mode_x", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_auth_mode_x_0_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_auth_mode_x_0_temp"));

			//crypto
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_crypto", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_crypto_0_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_crypto_0_temp"));

			//wpa_psk
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_wpa_psk", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_wpa_psk_0_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_wpa_psk_0_temp"));

			//macmode
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_macmode", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_macmode_0_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_macmode_0_temp"));

			//lanaccess
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_lanaccess", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_lanaccess_0_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_lanaccess_0_temp"));

			//expire
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_expire", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_expire_0_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_expire_0_temp"));

			//bss_enable
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}
	}

	//backup 5G profile and set fbwifi profile to wl1.X
	memset(wl_unit, 0, sizeof(wl_unit));
	memset(wl_unit_temp, 0, sizeof(wl_unit_temp));

	sprintf(wl_unit, "%s", nvram_safe_get("fbwifi_5g"));
	sprintf(wl_unit_temp, "%s", nvram_safe_get("fbwifi_5g_temp"));
	if(strcmp(wl_unit, "off") && !strcmp(nvram_safe_get("fbwifi_enable"), "on")) {
		_dprintf("Backup 5G profile and set fbwifi profile\n");

		//ssid
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_ssid", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_ssid_1_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_ssid_1_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("fbwifi_ssid"))) {
			nvram_set(wl_item, nvram_safe_get("fbwifi_ssid"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_ssid"));
		}

		//auth_mode_x
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_auth_mode_x", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_auth_mode_x_1_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_auth_mode_x_1_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("fbwifi_auth_mode_x"))) {
			nvram_set(wl_item, nvram_safe_get("fbwifi_auth_mode_x"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_auth_mode_x"));
		}

		//crypto
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_crypto", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_crypto_1_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_crypto_1_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("fbwifi_crypto"))) {
			nvram_set(wl_item, nvram_safe_get("fbwifi_crypto"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_crypto"));
		}

		//wpa_psk
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_wpa_psk", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_wpa_psk_1_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_wpa_psk_1_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("fbwifi_wpa_psk"))) {
			nvram_set(wl_item, nvram_safe_get("fbwifi_wpa_psk"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_wpa_psk"));
		}

		//macmode
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_macmode", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_macmode_1_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_macmode_1_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), "disabled")) {
			nvram_set(wl_item, "disabled");
			_dprintf("set %s : %s\n", wl_item, "disabled");
		}

		//lanaccess
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_lanaccess", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_lanaccess_1_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_lanaccess_1_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), "off")) {
			nvram_set(wl_item, "off");
			_dprintf("set %s : %s\n", wl_item, "off");
		}

		//expire
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_expire", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_expire_1_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_expire_1_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bss_enable
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "1")) {
			nvram_set(wl_item, "1");
			_dprintf("set %s : %s\n", wl_item, "1");
		}

		//bw_enabled
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bw_dl
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_dl", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//bw_ul
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_ul", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//fbwifi_5g
		if(strcmp(nvram_safe_get("fbwifi_5g_temp"), wl_unit)) {
			nvram_set("fbwifi_5g_temp", wl_unit);
			_dprintf("set fbwifi_5g_temp : %s\n",wl_unit);
		}

	}
	else {
		 _dprintf("Reset 5G profile\n");
		//reset 5G profile
		memset(wl_unit, 0, sizeof(wl_unit));
		sprintf(wl_unit, "%s", nvram_safe_get("fbwifi_5g_temp"));
		if(strcmp(wl_unit, "off")) {
			//fbwifi_5g
			nvram_set("fbwifi_5g_temp", "off");
			_dprintf("set fbwifi_5g_temp : %s\n", "off");

			//ssid
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_ssid", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_ssid_1_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_ssid_1_temp"));

			//auth_mode_x
			 memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_auth_mode_x", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_auth_mode_x_1_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_auth_mode_x_1_temp"));

			//crypto
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_crypto", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_crypto_1_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_crypto_1_temp"));

			//wpa_psk
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_wpa_psk", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_wpa_psk_1_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_wpa_psk_1_temp"));

			//macmode
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_macmode", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_macmode_1_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_macmode_1_temp"));

			//lanaccess
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_lanaccess", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_lanaccess_1_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_lanaccess_1_temp"));

			//expire
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_expire", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_expire_1_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_expire_1_temp"));

			//bss_enable
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}
	}

#ifdef RTAC3200
//backup 5G-2 profile and set fbwifi profile to wl2.X
	memset(wl_unit, 0, sizeof(wl_unit));
	memset(wl_unit_temp, 0, sizeof(wl_unit_temp));

	sprintf(wl_unit, "%s", nvram_safe_get("fbwifi_5g_2"));
	sprintf(wl_unit_temp, "%s", nvram_safe_get("fbwifi_5g_2_temp"));
	if(strcmp(wl_unit, "off") && !strcmp(nvram_safe_get("fbwifi_enable"), "on")) {
		_dprintf("Backup 5G-2 profile and set fbwifi profile\n");

		//ssid
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_ssid", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_ssid_2_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_ssid_2_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("fbwifi_ssid"))) {
			nvram_set(wl_item, nvram_safe_get("fbwifi_ssid"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_ssid"));
		}

		//auth_mode_x
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_auth_mode_x", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_auth_mode_x_2_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_auth_mode_x_2_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("fbwifi_auth_mode_x"))) {
			nvram_set(wl_item, nvram_safe_get("fbwifi_auth_mode_x"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_auth_mode_x"));
		}

		//crypto
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_crypto", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_crypto_2_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_crypto_2_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("fbwifi_crypto"))) {
			nvram_set(wl_item, nvram_safe_get("fbwifi_crypto"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_crypto"));
		}

		//wpa_psk
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_wpa_psk", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_wpa_psk_2_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_wpa_psk_2_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("fbwifi_wpa_psk"))) {
			nvram_set(wl_item, nvram_safe_get("fbwifi_wpa_psk"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_wpa_psk"));
		}

		//macmode
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_macmode", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_macmode_2_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_macmode_2_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), "disabled")) {
			nvram_set(wl_item, "disabled");
			_dprintf("set %s : %s\n", wl_item, "disabled");
		}

		//lanaccess
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_lanaccess", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_lanaccess_2_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_lanaccess_2_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), "off")) {
			nvram_set(wl_item, "off");
			_dprintf("set %s : %s\n", wl_item, "off");
		}

		//expire
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_expire", wl_item);
		if(!strcmp(wl_unit_temp, "off")) {
			nvram_set("fbwifi_expire_2_temp", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_expire_2_temp : %s\n", nvram_safe_get(wl_item));
		}
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bss_enable
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "1")) {
			nvram_set(wl_item, "1");
			_dprintf("set %s : %s\n", wl_item, "1");
		}

		//bw_enabled
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bw_dl
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_dl", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//bw_ul
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_ul", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//fbwifi_5g_2
		if(strcmp(nvram_safe_get("fbwifi_5g_2_temp"), wl_unit)) {
			nvram_set("fbwifi_5g_2_temp", wl_unit);
			_dprintf("set fbwifi_5g_2_temp : %s\n",wl_unit);
		}

	}
	else {
		 _dprintf("Reset 5G-2 profile\n");
		//reset 5G-2 profile
		memset(wl_unit, 0, sizeof(wl_unit));
		sprintf(wl_unit, "%s", nvram_safe_get("fbwifi_5g_2_temp"));
		if(strcmp(wl_unit, "off")) {
			//fbwifi_5g_2
			nvram_set("fbwifi_5g_2_temp", "off");
			_dprintf("set fbwifi_5g_2_temp : %s\n", "off");

			//ssid
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_ssid", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_ssid_2_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_ssid_2_temp"));

			//auth_mode_x
			 memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_auth_mode_x", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_auth_mode_x_2_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_auth_mode_x_2_temp"));

			//crypto
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_crypto", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_crypto_2_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_crypto_2_temp"));

			//wpa_psk
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_wpa_psk", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_wpa_psk_2_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_wpa_psk_2_temp"));

			//macmode
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_macmode", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_macmode_2_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_macmode_2_temp"));

			//lanaccess
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_lanaccess", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_lanaccess_2_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_lanaccess_2_temp"));

			//expire
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_expire", wl_item);
			nvram_set(wl_item, nvram_safe_get("fbwifi_expire_2_temp"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("fbwifi_expire_2_temp"));

			//bss_enable
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
			nvram_set(wl_item, "0");
			_dprintf("set %s : %s\n", wl_item, "0");
		}
	}
#endif
}
void
overwrite_fbwifi_ssid(void) {
	char wl_item[30];
	char wl_if[5];
	memset(wl_item, 0, sizeof(wl_item));
	memset(wl_if, 0, sizeof(wl_if));

	sprintf(wl_if, "wl%s.%s", nvram_safe_get("wl_unit"), nvram_safe_get("wl_subunit"));

	//ssid
	(void)strcat_r(wl_if, "_ssid", wl_item);
	switch(nvram_get_int("wl_unit")) {
		case 0 :
			nvram_set("fbwifi_ssid", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_ssid : %s\n", nvram_safe_get(wl_item));
			break;
		case 1 :
			nvram_set("fbwifi_ssid", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_ssid : %s\n", nvram_safe_get(wl_item));
			break;
#ifdef RTAC3200
		case 2 :
			nvram_set("fbwifi_ssid", nvram_safe_get(wl_item));
			_dprintf("set fbwifi_ssid : %s\n", nvram_safe_get(wl_item));
			break;
#endif
	}

	//sync ssid to other interface
	memset(wl_item, 0, sizeof(wl_item));
	memset(wl_if, 0, sizeof(wl_if));
	sprintf(wl_if, "%s", nvram_safe_get("fbwifi_2g"));
	if(strcmp(wl_if, "off") && strcmp(wl_if, "")) {
		(void)strcat_r(wl_if, "_ssid", wl_item);
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("fbwifi_ssid"))) {
			nvram_set(wl_item,  nvram_safe_get("fbwifi_ssid"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get(wl_item));
		}
	}
	memset(wl_item, 0, sizeof(wl_item));
	memset(wl_if, 0, sizeof(wl_if));
	sprintf(wl_if, "%s", nvram_safe_get("fbwifi_5g"));
	if(strcmp(wl_if, "off") && strcmp(wl_if, "")) {
		(void)strcat_r(wl_if, "_ssid", wl_item);
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("fbwifi_ssid"))) {
			nvram_set(wl_item,  nvram_safe_get("fbwifi_ssid"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get(wl_item));
		}
	}
#ifdef RTAC3200
	memset(wl_item, 0, sizeof(wl_item));
	memset(wl_if, 0, sizeof(wl_if));
	sprintf(wl_if, "%s", nvram_safe_get("fbwifi_5g_2"));
	if(strcmp(wl_if, "off") && strcmp(wl_if, "")) {
		(void)strcat_r(wl_if, "_ssid", wl_item);
		if(strcmp(nvram_safe_get(wl_item), nvram_safe_get("fbwifi_ssid"))) {
			nvram_set(wl_item,  nvram_safe_get("fbwifi_ssid"));
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get(wl_item));
		}
	}
#endif
}
#endif


#ifdef RTCONFIG_CFGSYNC
int start_cfgsync(void)
{
	char *cfg_server_argv[] = {"cfg_server", NULL};
	char *cfg_client_argv[] = {"cfg_client", NULL};
	pid_t pid;
	int ret = 0;

	if (!f_exists("/etc/cfg_pub.pem"))
	{
		eval("/usr/sbin/gencfgcert.sh");
	}

#ifdef RTCONFIG_MASTER_DET
	if (nvram_match("cfg_master", "1") && (!repeater_mode() && !mediabridge_mode()))
#else
	if (is_router_mode())
#endif	/* RTCONFIG_MASTER_DET */
	{
		stop_cfgsync();
		ret = _eval(cfg_server_argv, NULL, 0, &pid);
	}
#ifdef RTCONFIG_MASTER_DET
	else if (nvram_match("x_Setting", "1") &&
		strlen(nvram_safe_get("cfg_group")) == 0 &&
		is_router_mode())
	{
		stop_cfgsync();
		nvram_set("cfg_master", "1");
		nvram_commit();
		ret = _eval(cfg_server_argv, NULL, 0, &pid);
	}
#endif
	else if (
#ifdef RTCONFIG_MASTER_DET
		is_router_mode() ||
#endif
		((access_point_mode()
		|| dpsr_mode()
#ifdef RTCONFIG_DPSTA
		|| dpsta_mode()
#endif
		) && nvram_get_int("lan_state_t") == LAN_STATE_CONNECTED) ||
		((repeater_mode() || mediabridge_mode()) && nvram_get_int("wlc_state") == WLC_STATE_CONNECTED))
	{
		stop_cfgsync();
		ret = _eval(cfg_client_argv, NULL, 0, &pid);
	}

	return ret;
}

void stop_cfgsync(void)
{
	if (pids("cfg_server"))
		killall_tk("cfg_server");

        if (pids("cfg_client"))
                killall_tk("cfg_client");
}

void send_event_to_cfgmnt(int event_id)
{
	char msg[64] = {0};

	if(nvram_get_int("x_Setting") == 1) {
		snprintf(msg, sizeof(msg), RC_GENERIC_MSG, event_id);
		send_cfgmnt_event(msg);
	}
}
#endif

#ifdef RTCONFIG_USB_SWAP
int start_usb_swap(char *path)
{
	int ret;
	ret = eval("/usr/sbin/usb_swap.sh", path);
	return ret;
}

int stop_usb_swap(char *path)
{
	int ret;
	ret = eval("/usr/sbin/usb_swap.sh", path, "0");
	return ret;
}
#endif

#ifdef RTCONFIG_HD_SPINDOWN
#ifdef LINUX26
void start_usb_idle(void)
{
	char usb_idle_timeout[10], exclude2[20];
	char *exclude = nvram_get("usb_idle_exclude");
	if((nvram_match("usb_idle_enable", "0")) || (nvram_match("usb_idle_timeout", "0")))
		return;

	memset(exclude2, 0, sizeof(exclude2));
	snprintf(usb_idle_timeout, sizeof(usb_idle_timeout), "%d", nvram_get_int("usb_idle_timeout"));
	if(*exclude)
		snprintf(exclude2, sizeof(exclude2), "!%s",nvram_safe_get("usb_idle_exclude"));
	eval("/usr/sbin/sd-idle-2.6" , "-i" , usb_idle_timeout, *exclude ? "-d" : NULL,  exclude2, NULL);
}

void stop_usb_idle(void)
{
	killall_tk("sd-idle-2.6");
}
#endif
#endif
#ifdef BLUECAVE
void reset_led(void)
{
	int brightness_level = nvram_get_int("bc_ledLv");
	if(brightness_level < 0 || brightness_level > 3)
		brightness_level = 2;
	setCentralLedLv(brightness_level);
}
#endif
