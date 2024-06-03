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
#include <shared.h>
#include "flash_mtd.h"

#if defined(RTCONFIG_CAPTIVE_PORTAL)
#include "sqlite3.h"
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
#ifdef RTCONFIG_DSL
#include <dsl-upg.h>
#endif
#ifdef RTCONFIG_USB
#include <disk_share.h>
#include <disk_io_tools.h>	//mkdir_if_none()
#ifdef RTCONFIG_USB_SMS_MODEM
#include "libsmspdu.h"
#endif
#else
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
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#endif

#ifdef RTCONFIG_IPSEC
#include "rc_ipsec.h"
#define DBG(args) _dprintf args
#endif

#if defined(RTCONFIG_NOTIFICATION_CENTER)
#include <libnt.h>
#include <nt_eInfo.h>
#endif

#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
#include <plc_utils.h>
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
#include <amas_lib.h>
int init_x_Setting = -1;
void start_amas_lldpd(void);
#endif

#ifdef RTCONFIG_AMAS_WGN
#include <amas_wgn_shared.h>
#endif

#ifdef RTCONFIG_MULTILAN_CFG
#include <amas_apg_shared.h>
#endif

#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)
#include <bcm_flashutil.h>
#include <bcm_hwdefs.h>
#endif

#ifdef RTCONFIG_MULTIWAN_IF
#include "multi_wan.h"
#endif

extern char *crypt __P((const char *, const char *)); //should be defined in unistd.h with _XOPEN_SOURCE defined
#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

#ifdef RTCONFIG_FAST_ACL_SET
extern void set_acl_and_remove_sta(int bssidx, int vifidx);
#endif
void chilli_localUser(void);
void stop_hour_monitor_service();
void start_hour_monitor_service();
#ifdef RTCONFIG_ASD
extern int monitor_asd(void);
#endif

#ifdef RTCONFIG_MTK_BSD
void start_mtk_bs20(void);
void stop_mtk_bs20(void);
#endif	/* RTCONFIG_MTK_BSD */

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

static const char dmresolv[] = "/tmp/resolv.conf";
const char dmservers[] = "/tmp/resolv.dnsmasq";

#define INADYNCONF "/etc/inadyn.conf"

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

#if 0
void start_jitterentropy(void);
void stop_jitterentropy(void);
#else
void start_haveged(void);
void stop_haveged(void);
#endif


#ifndef MS_MOVE
#define MS_MOVE		8192
#endif
#ifndef MNT_DETACH
#define MNT_DETACH	0x00000002
#endif

#if defined(BCMDBG) || defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK)
#include <assert.h>
#else
#define assert(a)
#endif

#define logs(s) syslog(LOG_NOTICE, s)

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_JFFSV1) || defined(RTCONFIG_BRCM_NAND_JFFS2)
int jffs2_fail;
#endif

#define FWUPDATE_DBG(fmt,args...) \
        if(1) { \
                char info[1024]; \
                snprintf(info, sizeof(info), "echo \"[FWUPDATE][%s:(%d)]"fmt"\" >> /tmp/webs_upgrade.log", __FUNCTION__, __LINE__, ##args); \
                system(info); \
        }

void
sanity_logs()
{
#if defined(RTAC56U) || defined(RTAC56S)
	logmessage("ATE", "valid user mode(%d)", !nvram_get_int(ATE_BRCM_FACTORY_MODE_STR()));
#endif
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
int build_temp_rootfs(const char *newroot)
{
	int i, r;
	struct stat st;
	char d1[PATH_MAX], d2[PATH_MAX];
	struct utsname u;
	const char *mdir[] = { "/proc", "/tmp", "/sys", "/usr", "/var", "/var/lock" };
	const char *bin = "ash busybox cat cp dd df echo grep iwpriv kill ls ps mkdir mount nvram ping sh tar umount uname rm chmod mv klogd syslogd"
#if defined(RTCONFIG_HND_ROUTER_AX)
			" bcm_flasher chroot pivot_root bcm_bootstate bash vi ubinfo"			
			" ash bash cat chmod chown cp date dd df dmesg echo egrep false fatattr fgrep fsync grep gunzip gzip ln login mkdir mknod more netsta nice pidof ping ping6i ps pwd rm rmdir"
			" sed sleep stty sync tar touch true usleep watch zcat"
#endif
			;
	const char *sbin = "init rc hotplug2 ifconfig insmod lsmod modprobe reboot rmmod rtkswitch"
#if defined(RTCONFIG_BWDPI)
		" bwdpi* rsasign_sig_check"
#endif
#if defined(RTCONFIG_HW_DOG)
		" hwdog"
#endif
#if defined(RTCONFIG_SCHED_DAEMON)
		" sched_daemon"
#endif
		;
	const char *lib = "librt*.so* libnsl* libdl* libm* ld-* libiw* libgcc* libpthread* libdisk* libc*"
#if defined(RTCONFIG_HND_ROUTER_AX)
			" libnvram.so* libgen_util.so* libsys_util.so* libwlcsm.so* libbcm_flashutil.so* libbcm_util.so* libbcm_boardctl.so*"
#endif
#if defined(RTCONFIG_LIBASUSLOG)
			     " libasuslog.so*"
#endif
		;
	const char *usrbin = "killall which expr find fuser lsof"
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054) || defined(RTCONFIG_QCA_AXCHIP)
			     " cnssdaemon"
#endif
		;
	const char *usrsbin __attribute__((unused)) = "httpd brctl ip iptables ip6tables"
#if defined(RTCONFIG_HTTPS)
			     " httpds"
#endif
#ifdef RTCONFIG_BCMARM
			     " nvram"
#endif
#ifdef RTCONFIG_HND_ROUTER
			     " envram envrams fwupg_flashing.sh"
#endif
#if defined(RTCONFIG_BWDPI)
			     " shn_ctrl bwdpi_sqlite dcd wred AiProtectionMonitor hwinfo TrafficAnalyzer WebHistory"
#endif
		;
	const char *usrlib = "libnvram.so* libshared.so* libcrypto.so* libbcm.so*"
#if defined(RTCONFIG_HND_ROUTER_AX)
			     " libwpa_client.so* "
			     " libsqlite3.so.0* libjson-c.so.2* libasc.so* libovpn.so* "
#else
			     " libjson*"
#endif
#if defined(RTCONFIG_BWDPI)
			     " libbwdpi.so libbwdpi_sql.so"
#endif
#ifdef RTCONFIG_USB_SMS_MODEM
			     " libsmspdu.so"
#endif
#if defined(RTCONFIG_HTTPS) || defined(RTCONFIG_PUSH_EMAIL) || defined(RTCONFIG_FRS_FEEDBACK)
			     " libssl.so* libmssl.so*"
#if defined(RTCONFIG_FRS_FEEDBACK)
			     " libcurl.so* libxml2.so*"
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
#if defined(RTCONFIG_PROTECTION_SERVER)
			     " libptcsrv.so"
#endif
#if defined(RTCONFIG_LETSENCRYPT)
			     " libletsencrypt.so"
#endif
#if defined(RTCONFIG_AMAS)
			     " libamas-utils.so liblldpctl.so* libjansson.so*"
#endif
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054) || defined(RTCONFIG_QCA_AXCHIP)
			     " libqmi_cci.so libqmi_common_so.so"
#endif
#if defined(RTCONFIG_CONNDIAG)
			     " libconn_diag.so"
#endif	/* RTCONFIG_CONNDIAG */
#if defined(RTCONFIG_RALINK_MT7629) || defined(RTCONFIG_RALINK_MT7622)
                 " libnl-tiny.so"
#endif  /* RTCONFIG_RALINK_MT7629 */                 
#if defined(RTCONFIG_TR069) || defined(RTCONFIG_TR181)
				 " libssh2.so* libz.so*"
#endif	/* RTCONFIG_TR069 || RTCONFIG_TR181 */				 
			     " libev.so.4.0.0"
			     " libev.so.4"
			     " libcodb.so"
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
#if defined(RTCONFIG_IPV6)
		"ip6t_REJECT\\|ip6t_ROUTE\\|ip6t_LOG\\|xt_length\\|"
#endif
		"xt_HL\\|xt_hl\\|"
		"nvram_linux\\)'";		/* nvram_linux.ko */
	const char *modules = "find /lib/modules -name 'modules.dep'";

	if (!newroot || *newroot == '\0')
		newroot = TMP_ROOTFS_MNT_POINT;

#ifdef RTCONFIG_HND_ROUTER_AX_6756
	eval("wl", "-i", "wl0", "down");
	eval("wl", "-i", "wl1", "down");
	eval("wl", "-i", "wl2", "down");
	eval("wdtctl", "-d", "stop");
	eval("killall", "debug_monitor");
	sleep(1);

	eval("ubirmvol", "/dev/ubi0", "-n", "12");
	eval("ubimkvol", "/dev/ubi0", "-s", *nvram_safe_get("newroot_size")?nvram_safe_get("newroot_size"):"30000000", "-n", "12", "-N", "tmprootfs", "--type=dynamic");
	eval("mkdir", "-p", newroot);

	if ((r = mount("/dev/ubi0_12", newroot, "ubifs", MS_NOATIME, "")) != 0)
		return -1;
#else
	if ((r = mount("tmpfs", newroot, "tmpfs", MS_NOATIME, "")) != 0)
		return -1;
#endif

	_dprintf("Build temp rootfs\n");
	__cp("", "/dev", "", newroot);
	if (!uname(&u)) {
		snprintf(d1, sizeof(d1), "/lib/modules/%s", u.release);
		__cp("", d1, "modules.dep", newroot);
	}
	__cp("", "/bin", bin, newroot);
	__cp("", "/sbin", sbin, newroot);
	__cp("", "/lib", lib, newroot);
	__cp("", "/lib", "libcrypt*", newroot);
	__cp("", "/usr/bin", usrbin, newroot);
	__cp("", "/usr/sbin", usrsbin, newroot);
	__cp("", "/usr/lib", usrlib, newroot);
	__cp("L", "/etc", "", newroot);		/* don't creat symbolic link (/tmp/etc/foo) that will be broken soon */

#if defined(RTCONFIG_QCA)
	__cp("", "/sbin", "wlanconfig", newroot);
	__cp("", "/usr/lib", "libnl-tiny.so", newroot);
#if !defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033) && \
    !defined(RTCONFIG_SWITCH_QCA8075_PHY_AQR107) && \
    !defined(RTCONFIG_SOC_IPQ60XX) && \
    !defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) && \
    !defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
	__cp("", "/usr/sbin", "swconfig", newroot);
#endif
#if defined(RTCONFIG_SOC_IPQ8074) || \
    defined(RTCONFIG_SOC_IPQ40XX)
	__cp("", "/usr/sbin", "ssdk_sh", newroot);
#endif
#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) || \
    defined(RTCONFIG_WIFI_QCA9994_QCA9994) || \
    defined(RTCONFIG_WIFI_QCN5024_QCN5054) || \
    defined(RTCONFIG_QCA_AXCHIP)
	__cp("", "/usr/sbin", "thermald", newroot);
#endif
#if defined(RTCONFIG_CFG80211)
	__cp("", "/usr/sbin", "cfg80211*", newroot);
	__cp("R", "/lib/wifi", "*.xml", newroot);
#endif
#endif
#if defined(RTCONFIG_PPTPD)
	if (nvram_match("pptpd_enable", "1")) {
		__cp("", "/usr/sbin", "pptpd bcrelay", newroot);
	}
#endif
#if defined(RTCONFIG_OPENVPN)
	__cp("", "/usr/lib", "libovpn.so", newroot);
	if (nvram_match("VPNServer_enable", "1")) {
		__cp("", "/usr/sbin", "openvpn", newroot);
		__cp("", "/usr/lib", "libz.so* liblzo2.so* liblz4.so*", newroot);
	}
#endif
#if defined(RTCONFIG_WIREGUARD)
	/* Use dummy command instead of preparing environment for real one. */
	snprintf(d1, sizeof(d1), "%s/usr/sbin/cru", newroot);
	f_write_string(d1, "#!/bin/sh\n", 0, 0);
	eval("chmod", "a+x", d1);
#endif
#if defined(RTCONFIG_IPSEC)
	if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		|| nvram_get_int("ipsec_ig_enable")
#endif
	) {
		__cp("", "/usr/sbin", "charon-cmd ipsec pki", newroot);
		__cp("", "/usr/lib/ipsec", "starter charon", newroot);
		__cp("R", "/usr/lib/ipsec", "", newroot);
	}
#endif
#if defined(RTCONFIG_QCA_PLC2)
	{
		__cp("", "/usr/local/bin", "plctool", newroot);
	}
#endif
#if defined(RTCONFIG_LIBASC)
	{
		__cp("", "/usr/lib", "libasc.so", newroot);
	}
#endif

	/* copy mandatory kernel modules */
	snprintf(d2, sizeof(d2), "tar cvf - `%s` `%s` | tar xf - -C %s", kmod, modules, newroot);
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
int switch_root(const char *newroot)
{
	int r;
	dev_t rdev;
	struct stat st;
#if defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK)
	char *const argv[] = { "/sbin/init", "reboot", NULL };
#else
	char *const argv[] = { "sh", NULL };
#endif

	if (!newroot || *newroot == '\0')
		newroot = TMP_ROOTFS_MNT_POINT;

#ifndef RTCONFIG_HND_ROUTER_AX_6756
	if (getpid() != 1) {
		_dprintf("%s: PID != 1\n", __func__);
		return -1;
	}
#endif

#if 0
	char oldroot[64];

	sprintf(oldroot, "%s/old-root", newroot);

	eval("mkdir", "-p", oldroot);
#endif	
	_dprintf("%s, chk newroot=%s\n", __func__, newroot);

	if (chdir(newroot))
		return -2;

	stat("/", &st);
	rdev = st.st_dev;
	stat(".", &st);
	if (rdev == st.st_dev)
		return -3;

#if 0
	if(nvram_match("newroot_way", "2")) {
		_dprintf("\npivot root\n");
		eval("pivot_root", ".", oldroot);
	} else {
#endif
	_dprintf("mount / as switch_root\n");
	if ((r = mount(".", "/", NULL, MS_MOVE, NULL)) != 0)
		return -4;
#if 0
	}
#endif

	chroot(".");
	chdir("/");
	/* WARNING:
	 * If new rootfilesystem lacks libraries that are need by init process,
	 * kernel reboots system in 3 seconds.
	 */
	if ((r = execv(argv[0], argv))) {
		_dprintf("%s, execv err:%d\n", __func__, r);
		return -5;
	}

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

static void
nvram_fsafe_get(const char *nvram, char *nvram_buf, size_t len)
{
	int try = 3;
	do {
		if(try != 3)    // dont usleep the first time
		{
			dbg("[%s:(%d)]:get empty nvram(%s)\n", __FUNCTION__, __LINE__, nvram);
			usleep(500);
		}
		strlcpy(nvram_buf, nvram_safe_get(nvram), len);
		try--;
	} while(strlen(nvram_buf) == 0 && try > 0);
}

void create_passwd(void)
{
	FILE *fp;
	mode_t m;
	char salt[32];
	char http_user[128] = {0};
	char http_passwd[128] = {0};
	char crypt_buf[256] = {0};
	char *crypt_string = NULL;
	char *shell_user, *shell_passwd;
#ifdef RTCONFIG_SAMBASRV	//!!TB
	char *smbd_user;
#endif
#ifdef RTCONFIG_NVRAM_ENCRYPT
	char http_passwd_buf[128];
	char shell_passwd_buf[128];
#endif
	char passwd_buf[128] = {0};

#ifdef RTCONFIG_SAMBASRV	//!!TB
	create_custom_passwd();
#endif

#if defined(RTCONFIG_OPENVPN)
	mkdir_if_none("/etc/pam.d");
	f_write_string("/etc/pam.d/openvpn", "auth required pam_unix.so\n", 0, 0644);
	create_ovpn_passwd();
#endif

	/* Admin user */
	nvram_fsafe_get("http_username", http_user, sizeof(http_user));
	if (strlen(http_user) == 0)
		strlcpy(http_user, nvram_default_get("http_username"), sizeof(http_user));
	strlcpy(http_passwd, nvram_safe_get("http_passwd"), sizeof(http_passwd));
	if (*http_passwd == '\0')
		strlcpy(http_passwd, nvram_default_get("http_passwd"), sizeof(http_passwd));
#ifdef RTCONFIG_NVRAM_ENCRYPT
	else {
		memset(http_passwd_buf, 0, sizeof(http_passwd_buf));
		nvram_fsafe_get("http_passwd", passwd_buf, sizeof(passwd_buf));
		pw_dec(passwd_buf, http_passwd_buf, sizeof(http_passwd_buf), 1);
		strlcpy(http_passwd, http_passwd_buf, sizeof(http_passwd));
	}
#endif

#if defined(RTCONFIG_USB) && defined(RTCONFIG_AMAS)
	if(aimesh_re_node()){
		char *nv, *nvp, *b;
		char *first_ascii_user, *first_ascii_passwd;
		int i;

		nv = nvp = strdup(nvram_safe_get("acc_list"));
		i = 0;
		if(nv && strlen(nv) > 0){
			while((b = strsep(&nvp, "<")) != NULL){
				if(vstrsep(b, ">", &first_ascii_user, &first_ascii_passwd) != 2)
					continue;

				mod_account(first_ascii_user, http_user, http_passwd);

				if(++i == 1)
					break;
			}
		}
	}
#endif

	/* Shell user */
	if (nvram_get_int("telnetd_enable")
#ifdef RTCONFIG_SSH
		|| (nvram_get_int("sshd_enable") && nvram_get_int("sshd_pass"))
#endif
	) {
		shell_user = nvram_safe_get("shell_username");
		if (strcmp(shell_user, http_user) == 0)
			shell_user = "";
		shell_passwd = nvram_safe_get("shell_passwd");
#ifdef RTCONFIG_NVRAM_ENCRYPT
		if (*shell_passwd) {
			memset(shell_passwd_buf, 0, sizeof(shell_passwd_buf));
			pw_dec(shell_passwd, shell_passwd_buf, sizeof(shell_passwd_buf), 1);
			shell_passwd = shell_passwd_buf;
		}
#endif
	} else
		shell_user = shell_passwd = ""; /* disable */

#ifdef RTCONFIG_SAMBASRV	//!!TB
	smbd_user = nvram_safe_get("smbd_user");
	if (*smbd_user == '\0' || strcmp(smbd_user, "root") == 0)
		smbd_user = "nas";
#endif

	m = umask(066); /* 600 */

	/* /etc/shadow */
	if ((fp = fopen("/etc/shadow", "w")) != NULL) {
#ifdef RTCONFIG_TRUSTZONE
		atee_get_admin_crypt(make_salt("5", salt, sizeof(salt)), crypt_buf, sizeof(crypt_buf));
		crypt_string = crypt_buf;
#else
		crypt_string = crypt(http_passwd, make_salt("5", salt, sizeof(salt)));
		if(crypt_string == NULL || *crypt_string == '\0'){
			asus_openssl_crypt(http_passwd, salt, crypt_buf, sizeof(crypt_buf));
			crypt_string = crypt_buf;
		}
#endif
		fprintf(fp, "%s:%s:0:0:99999:7:0:0:\n",
			http_user, crypt_string);
		if (*shell_user && *shell_passwd) {
			crypt_string = crypt(shell_passwd, make_salt("5", salt, sizeof(salt)));
			if(crypt_string == NULL || *crypt_string == '\0'){
				asus_openssl_crypt(http_passwd, salt, crypt_buf, sizeof(crypt_buf));
				crypt_string = crypt_buf;
			}
			fprintf(fp, "%s:%s:0:0:99999:7:0:0:\n",
				shell_user, crypt_string);
		}
#ifdef RTCONFIG_SAMBASRV	//!!TB
		fprintf(fp, "%s:%s:0:0:99999:7:0:0:\n", smbd_user, "*");
#endif
		fprintf(fp, "%s:%s:0:0:99999:7:0:0:\n", "nobody", "*");
#ifdef RTCONFIG_TOR
		fprintf(fp, "%s:%s:0:0:99999:7:0:0:\n", "tor", "*");
#endif
		fappend(fp, "/etc/shadow.custom");
#ifdef RTCONFIG_OPENVPN
		fappend(fp, "/etc/shadow.openvpn");
#endif
#ifdef RTCONFIG_COOVACHILLI
		fappend(fp, "/etc/shadow.chilli");
		fappend(fp, "/etc/shadow.chilli-cp");
#endif
		append_custom_config("shadow", fp);
		fclose(fp);
		run_postconf("shadow","/etc/shadow");
		chmod("/etc/shadow", 0600);
	}

	/* /etc/gshadow */
	if ((fp = fopen("/etc/gshadow", "w")) != NULL) {
		fprintf(fp,
			"root:*::\n"
#ifdef RTCONFIG_SAMBASRV	//!!TB
			"nas:*::\n"
#endif
#ifdef RTCONFIG_OPENVPN
			"openvpn:*::\n"	/* OpenVPN GID */
#endif
			"nobody:*::\n"
#ifdef RTCONFIG_TOR
			"tor:*::\n"
#endif
		);
		fappend(fp, "/etc/gshadow.custom");
		fappend(fp, "/jffs/configs/gshadow.add");
		fclose(fp);
		run_postconf("gshadow","/etc/gshadow");
		chmod("/etc/gshadow", 0600);
	}

	umask(022); /* 644 */

	/* /etc/passwd */
	if ((fp = fopen("/etc/passwd", "w")) != NULL) {
		fprintf(fp, "%s:x:0:0:%s:/root:/bin/sh\n", http_user, http_user);
		if (*shell_user && *shell_passwd)
			fprintf(fp, "%s:x:0:0:%s:/root:/bin/sh\n", shell_user, shell_user);
#ifdef RTCONFIG_SAMBASRV
		fprintf(fp, "%s:x:100:100:nas:/dev/null:/dev/null\n", smbd_user);
#endif
		fprintf(fp, "nobody:x:65534:65534:nobody:/dev/null:/dev/null\n");
#ifdef RTCONFIG_TOR
		fprintf(fp, "tor:x:65533:65533:tor:/dev/null:/dev/null\n");
#endif
		fappend(fp, "/etc/passwd.custom");
		fappend(fp, "/jffs/configs/passwd.add");
		fclose(fp);
#ifdef RTCONFIG_OPENVPN
// TODO: figure out if libvpn does anything special here?
//		append_ovpn_accnt("/etc/passwd", "/etc/passwd.openvpn");
		fappend_file("/etc/passwd", "/etc/passwd.openvpn");
#endif
		run_postconf("passwd","/etc/passwd");
		chmod("/etc/passwd", 0644);
	}

	/* /etc/group */
	if ((fp = fopen("/etc/group", "w")) != NULL) {
		fprintf(fp,
			"root:*:0:\n"
#ifdef RTCONFIG_SAMBASRV	//!!TB
			"nas:*:100:\n"
#endif
#ifdef RTCONFIG_OPENVPN
			"openvpn:*:200:\n"	/* OpenVPN GID */
#endif
			"nobody:*:65534:\n"
#ifdef RTCONFIG_TOR
			"tor:*:65533:\n"
#endif
		);
		fappend(fp, "/etc/group.custom");
		fappend(fp, "/jffs/configs/group.add");
		fclose(fp);
		run_postconf("group","/etc/group");

		chmod("/etc/group", 0644);
	}

	umask(m);
}

void get_dhcp_pool(char **dhcp_start, char **dhcp_end, char *buffer)
{
	if (dhcp_start == NULL || dhcp_end == NULL || buffer == NULL)
		return;

        if ((repeater_mode()
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
                || psr_mode() || mediabridge_mode()
#elif defined(RTCONFIG_REALTEK) || defined(RTCONFIG_QCA)
                || mediabridge_mode()
#endif
#if defined(RTCONFIG_DPSTA) || defined(RTCONFIG_DPSR)
		|| ((dpsta_mode() || dpsr_mode()) && nvram_get_int("re_mode") == 0)
#endif
		|| (rp_mode() && nvram_get_int("re_mode") == 0)
                ) && nvram_get_int("wlc_state") != WLC_STATE_CONNECTED) {
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

#ifdef RTCONFIG_CONNTRACK
void stop_pctime_service()
{
        killall("pctime", SIGTERM);
}

void start_pctime_service()
{
	char *cmd[] = {"pctime", NULL};
	int pid;

	if (!is_router_mode())
		return;

	if (!pids("pctime")) {
		_eval(cmd, NULL, 0, &pid);
	}
}
#endif

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
#elif defined(RTCONFIG_RALINK)
			eval("iwpriv",word,"set", "RadioOn=0");
#else /* RTCONFIG_QCA */
			ifconfig(word, 0, NULL, NULL);
#endif
		}
	}
}

static void link_up(void)
{
	char word[256], *next, ifnames[128];
	char  tmp[100], prefix[]="wlXXXXXXX_";
	int  band = 0;
	/* link up LAN ports */
#ifdef RTCONFIG_REALTEK
	lanport_ctrl(1);
#else
	eval("rtkswitch", "14");
#endif

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
#elif defined(RTCONFIG_RALINK)
				eval("iwpriv",word,"set", "RadioOn=1");
#else /* RTCONFIG_QCA */
				ifconfig(word, IFUP, NULL, NULL);
#endif
			band++;
		}
	}
}

int restart_dnsmasq(int need_link_DownUp)
{
	if (need_link_DownUp) {
#if (defined(PLN12) || defined(PLAC56) || defined(PLAC66) || defined(RTCONFIG_QCA_PLC2))
		nvram_set("plc_ready", "0");
#endif
#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_RALINK)
		nvram_set("lan_ready","0");
#endif
		link_down();
		sleep(9);
	}

#ifdef RTCONFIG_MULTILAN_CFG
	stop_dnsmasq(ALL_SDN);
	sleep(1);
	start_dnsmasq(ALL_SDN);
#else
	stop_dnsmasq();
	sleep(1);
	start_dnsmasq();
#endif

	if (need_link_DownUp) {
		link_up();
#if (defined(PLN12) || defined(PLAC56) || defined(PLAC66) || defined(RTCONFIG_QCA_PLC2))
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
	if (nvram_get_int("dhcpd_send_wpad")) {
		fprintf(fp,"dhcp-option=lan,252,\n");
	}
	fprintf(fp,"dhcp-authoritative\n");
 	fprintf(fp,"listen-address=127.0.0.1,%s\n",nvram_safe_get("lan_ipaddr"));
 	fprintf(fp,"no-dhcp-interface=%s\n",nvram_safe_get("lan_ifname"));

	fclose(fp);
	chmod("/etc/dnsmasq.conf", 0644);
#ifdef RTCONFIG_MULTILAN_CFG
	stop_dnsmasq(ALL_SDN);
#else
	stop_dnsmasq();
#endif
	eval("dnsmasq", "--log-async");
}
#endif

#ifdef RTCONFIG_MULTILAN_CFG
void start_dnsmasq(const int sdn_idx)
#else
void start_dnsmasq(void)
#endif
{
	FILE *fp;
	char *lan_ifname, *lan_ipaddr, *lan_hostname;
	char *value, *value2;
	int ii, have_dhcp = 0;
	char *mac, *ip, *dns, *hostname, *lan_domain;
	char *nv, *nvp, *b;
	unsigned char ea[ETHER_ADDR_LEN];
#ifdef RTCONFIG_IPSEC
	int unit;
	char tmpStr[20];
#endif
	char buf[sizeof("/rom/etc/resolv.conf")], *path;
#ifdef RTCONFIG_MULTILAN_CFG
	char v6_prefix[INET6_ADDRSTRLEN] = {0};
	char v6_prefix_length = 0;
	char sdn_prefix[INET6_ADDRSTRLEN] = {0};
	int sdn_prefix_length = 0;
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i, v6_sdn_num = 0;
#endif
	int n;

	TRACE_PT("begin\n");

	if (getpid() != 1) {
#ifdef RTCONFIG_MULTILAN_CFG
		char buf[32];
		snprintf(buf, sizeof(buf), "start_dnsmasq %d", sdn_idx);
		notify_rc_and_wait_2min(buf);
#else
		notify_rc_and_wait_2min("start_dnsmasq");
#endif
		return;
	}

#ifdef RTCONFIG_MULTILAN_CFG
	if(sdn_idx != ALL_SDN && sdn_idx != LAN_IN_SDN_IDX)	//handle one sdn
	{
		handle_sdn_feature(sdn_idx, SDN_FEATURE_DNSMASQ, RC_SERVICE_STOP | RC_SERVICE_START);
		return;
	}
	else
	{
		stop_dnsmasq(sdn_idx);	//stop main Lan or all  sdn(includes main lan)
	}
#else
	stop_dnsmasq();
#endif

	if (f_exists("/etc/dnsmasq.conf"))
		unlink("/etc/dnsmasq.conf");

	lan_ifname = nvram_safe_get("lan_ifname");

	if ((repeater_mode()
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
		|| psr_mode() || mediabridge_mode()
#elif defined(RTCONFIG_REALTEK) || defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK)
		|| mediabridge_mode()
#endif
#if defined(RTCONFIG_DPSTA) || defined(RTCONFIG_DPSR)
		|| ((dpsta_mode() || dpsr_mode()) && nvram_get_int("re_mode") == 0)
#endif
		|| (rp_mode() && nvram_get_int("re_mode") == 0)
		) && nvram_get_int("wlc_state") != WLC_STATE_CONNECTED && !nvram_match("lan_proto", "static"))
		lan_ipaddr = nvram_default_get("lan_ipaddr");
	else
		lan_ipaddr = nvram_safe_get("lan_ipaddr");

	/* write /etc/hosts */
	if ((fp = fopen("/etc/hosts", "w")) != NULL) {
		/* loclhost ipv4 */
		fprintf(fp, "127.0.0.1 localhost.localdomain localhost\n");

		/* lan hostname.domain hostname */
		lan_hostname = get_lan_hostname();
		fprintf(fp, "%s %s.%s %s\n", lan_ipaddr,
			    lan_hostname, nvram_safe_get("lan_domain"),
			    lan_hostname);

		/* mdns fallback */
		fprintf(fp, "%s %s.local\n", lan_ipaddr, lan_hostname);

		/* default names */
		fprintf(fp, "%s %s\n", lan_ipaddr, DUT_DOMAIN_NAME);
		fprintf(fp, "%s %s\n", lan_ipaddr, "asusrouter.com");
		fprintf(fp, "%s %s\n", lan_ipaddr, "www.asusrepeater.com");
		fprintf(fp, "%s %s\n", lan_ipaddr, "asusrepeater.com");
		fprintf(fp, "%s %s\n", lan_ipaddr, "www.asusap.com");
		fprintf(fp, "%s %s\n", lan_ipaddr, "asusap.com");
		fprintf(fp, "%s %s\n", lan_ipaddr, "www.asusswitch.com");
		fprintf(fp, "%s %s\n", lan_ipaddr, "asusswitch.com");
		fprintf(fp, "%s %s\n", lan_ipaddr, "router.asus.com");
		fprintf(fp, "%s %s\n", lan_ipaddr, "repeater.asus.com");
		fprintf(fp, "%s %s\n", lan_ipaddr, "ap.asus.com");
		fprintf(fp, "%s %s\n", lan_ipaddr, "www.asusnetwork.net");
		fprintf(fp, "%s %s\n", lan_ipaddr, "asusswitch.net");
		fprintf(fp, "%s %s\n", lan_ipaddr, "asusrepeater.net");
		fprintf(fp, "%s %s\n", lan_ipaddr, "asusap.net");
		fprintf(fp, "%s %s\n", lan_ipaddr, "zenwifi.net");
#if defined(RTCONFIG_BUSINESS)
		fprintf(fp, "%s %s\n", lan_ipaddr, "www.expertwifi.net");
		fprintf(fp, "%s %s\n", lan_ipaddr, OLD_DUT_DOMAIN_NAME);
#else
		fprintf(fp, "%s %s\n", lan_ipaddr, "expertwifi.net");
#endif

	nv = nvp = strdup(nvram_safe_get("dhcp_staticlist"));
	if (nv)
	{
		lan_domain = nvram_safe_get("lan_domain");		
		/* Parsing dhcp_staticlist nvram variable. */
		while ((b = strsep(&nvp, "<")) != NULL) {
			dns = NULL;
			hostname = NULL;
			if ((vstrsep(b, ">", &mac, &ip, &dns, &hostname) < 2))
				continue;

			if (!ether_atoe(mac, ea))
				continue;

			if(ip && *ip != '\0' && hostname && *hostname != '\0' && is_valid_hostname(hostname))
			{
				fprintf(fp, "%s %s.%s %s\n", ip, hostname, lan_domain, hostname);
			}	
		}
		free(nv);	
	}
		
#ifdef RTCONFIG_USB
		/* samba name */
		if (is_valid_hostname(value = nvram_safe_get("computer_name")) &&
		    strcasecmp(lan_hostname, value) != 0) {
			fprintf(fp, "%s %s.%s %s\n", lan_ipaddr,
				    value, nvram_safe_get("lan_domain"),
				    value);
		}
#endif
#ifdef RTCONFIG_DSL
		fprintf(fp, "192.168.121.70 ntp01.mvp.tivibu.com.tr\n");
		fprintf(fp, "192.168.121.71 ntp02.mvp.tivibu.com.tr\n");
#endif

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
			if (*value) {
				fprintf(fp, "%s %s.%s %s\n", value,
					    lan_hostname, nvram_safe_get("lan_domain"),
					    lan_hostname);

				/* mdns fallback */
				fprintf(fp, "%s %s.local\n", value, lan_hostname);
			}
		}
#endif
		append_custom_config("hosts", fp);
		fclose(fp);
		use_custom_config("hosts", "/etc/hosts");
		run_postconf("hosts","/etc/hosts");
		chmod("/etc/hosts", 0644);
	} else
		perror("/etc/hosts");

#ifdef RTCONFIG_REDIRECT_DNAME
	if (nvram_invmatch("redirect_dname", "0") && access_point_mode()) {
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
		if (!(sw_mode() == SW_MODE_AP && nvram_get_int("modem_bridge")))
#endif
			eval("dnsmasq", "--log-async");
	}
#endif
        if (!is_routing_enabled()
		&& !repeater_mode()
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
		&& !psr_mode() && !mediabridge_mode()
#elif defined(RTCONFIG_REALTEK) || defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK)
		&& !mediabridge_mode()
#endif
#if defined(RTCONFIG_DPSTA) || defined(RTCONFIG_DPSR)
		&& !((dpsta_mode() || dpsr_mode()) && nvram_get_int("re_mode") == 0)
#endif
		&& !(rp_mode() && nvram_get_int("re_mode") == 0)
	) {
#ifdef RTCONFIG_WIFI_SON
		if(nvram_match("wifison_ready", "1") &&
		   (sw_mode() == SW_MODE_AP && nvram_match("cfg_master", "1")))
		{
			if(nvram_get_int("wl0.1_bss_enabled"))
				gen_apmode_dnsmasq();
			return;
		} else
#endif
#ifdef RTCONFIG_RTL8198D
		if(sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")){
			eval("dnsmasq", "--log-async");
		}
#endif
		return;
	}

	// we still need dnsmasq in wet
	//if (foreach_wif(1, NULL, is_wet)) return;

	if ((fp = fopen("/etc/dnsmasq.conf", "w")) == NULL)
		return;

	fprintf(fp, "pid-file=/var/run/dnsmasq.pid\n"
		    "user=nobody\n"
		    "bind-dynamic\n"		// listen only on interface & lo
		);

#if defined(RTCONFIG_REDIRECT_DNAME)
	if (!repeater_mode()
#if defined(RTCONFIG_REALTEK) || defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK)
/* [MUST] : Need to clarify ..... */
	&& !mediabridge_mode() // skip media bridge
#endif
	)
#endif
	{
		fprintf(fp,"interface=%s\n",		// dns & dhcp on LAN interface
			lan_ifname);

#ifdef RTCONFIG_WIFI_SON
		if (sw_mode() != SW_MODE_REPEATER)
			fprintf(fp,"interface=%s\n",BR_GUEST);	// guest-network
#endif
#ifdef RTCONFIG_IPSEC
	if (nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable")
#ifdef RTCONFIG_INSTANT_GUARD
		|| nvram_get_int("ipsec_ig_enable")
#endif
	) {
		for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
			sprintf(tmpStr,"wan%d_gw_ifname",unit);
			if (0 != strlen(nvram_safe_get(tmpStr)))
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

	fprintf(fp, is_routing_enabled() ?
		"no-resolv\n" :			// no resolv, use only additional list
		"resolv-file=%s\n", dmresolv);	// the real stuff is here
	fprintf(fp, "servers-file=%s\n"		// additional servers list
		    "no-poll\n"			// don't poll resolv file
		    "no-negcache\n"		// don't cace nxdomain
		    "cache-size=%u\n"		// dns cache size
		    "min-port=%u\n",		// min port used for random src port
		dmservers, 1500, nvram_get_int("dns_minport") ? : 4096);

	/* limit number of outstanding requests */
	{
		int max_queries = nvram_get_int("max_dns_queries");
//#if defined(RTCONFIG_SOC_IPQ8064)
		if (max_queries == 0)
			max_queries = 1500;
//#endif
		if (max_queries)
			fprintf(fp, "dns-forward-max=%d\n", max(150, min(max_queries, 10000)));
	}

	/* lan domain */
	value = nvram_safe_get("lan_domain");
	if (*value) {
		fprintf(fp, "domain=%s\n"
			    "expand-hosts\n", value);	// expand hostnames in hosts file
	}
	if (nvram_get_int("dns_fwd_local") != 1) {
		fprintf(fp, "bogus-priv\n"			// don't forward private reverse lookups upstream
		            "domain-needed\n");			// don't forward plain name queries upstream
		if (*value)
			fprintf(fp, "local=/%s/\n", value);	// don't forward local domain queries upstream
	}

	if ((is_routing_enabled() && nvram_get_int("dhcp_enable_x"))
		|| ((repeater_mode()
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
			|| psr_mode() || mediabridge_mode()
#elif defined(RTCONFIG_REALTEK) || defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK)
			|| mediabridge_mode()
#endif
#if defined(RTCONFIG_DPSTA) || defined(RTCONFIG_DPSR)
			|| ((dpsta_mode() || dpsr_mode()) && nvram_get_int("re_mode") == 0)
#endif
			|| (rp_mode() && nvram_get_int("re_mode") == 0)
		    ) && nvram_get_int("wlc_state") != WLC_STATE_CONNECTED)
	) {
		char *dhcp_start, *dhcp_end;
		int dhcp_lease;
		char buffer[64]; // 3*20 min for bundlekey
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
		unsigned char bundlekey[20];
#endif
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
		if ((nvram_get_int("wlc_state") != WLC_STATE_CONNECTED) &&
			(repeater_mode()
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
				|| psr_mode() || mediabridge_mode()
#elif defined(RTCONFIG_REALTEK) || defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK)
				|| mediabridge_mode()
#endif
#if defined(RTCONFIG_DPSTA) || defined(RTCONFIG_DPSR)
				|| ((dpsta_mode() || dpsr_mode()) && nvram_get_int("re_mode") == 0)
#endif
				|| (rp_mode() && nvram_get_int("re_mode") == 0)
			)
		)
			dhcp_lease = atoi(nvram_default_get("dhcp_lease"));
		else
			dhcp_lease = nvram_get_int("dhcp_lease");

		if (dhcp_lease <= 0)
			dhcp_lease = 86400;

		/* LAN range */
		if (*dhcp_start && *dhcp_end) {
#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_REALTEK)
			if (!nvram_match("lan_proto", "static")) // Dynamic. get_dhcp_pool() will return default dhcp_start & dhcp_end.
				fprintf(fp, "dhcp-range=lan,%s,%s,%s,%ds\n",
					dhcp_start, dhcp_end, nvram_default_get("lan_netmask"), dhcp_lease);
			else
#endif
			fprintf(fp, "dhcp-range=lan,%s,%s,%s,%ds\n",
				dhcp_start, dhcp_end, nvram_safe_get("lan_netmask"), dhcp_lease);

#ifdef RTCONFIG_WIFI_SON
			//if (nvram_get_int("wl0.1_bss_enabled"))
			if (sw_mode() != SW_MODE_REPEATER)
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
			//if (nvram_get_int("wl0.1_bss_enabled"))
			if (sw_mode() != SW_MODE_REPEATER)
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
		if (nvram_get_int("dhcpd_send_wpad")) {
			fprintf(fp, "dhcp-option=lan,252,\"\\n\"\n");
		}

		/* NTP server */
		if (nvram_get_int("ntpd_enable"))
			fprintf(fp, "dhcp-option=lan,42,%s\n", "0.0.0.0");

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

#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
		if (nvram_invmatch("amas_bdlkey", "") && nvram_match("x_Setting", "0") &&
		    amas_gen_hash_bundle_key(bundlekey) == AMAS_RESULT_SUCCESS) {
			int i, n = 0;
			for (i = 0; i < 20 && n + sizeof(":XX") <= sizeof(buffer); i++)
				n += sprintf(buffer + n, i ? ":%02X" : "%02X", bundlekey[i]);
			if (n)
				fprintf(fp, "dhcp-option-force=lan,vi-encap:%d,%d,%s\n", 2623, 123, buffer);
		}
#endif

		if (nvram_match("switch_wantag", "hinet_mesh")) {
			fprintf(fp, "dhcp-vendorclass=set:cht,CHT\n");
			fprintf(fp, "dhcp-ignore=tag:cht\n");
		}
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
#if defined(RTCONFIG_IPV6) && (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2) || defined(ET12))
		if (!strncmp(nvram_safe_get("territory_code"), "CH", 2))
			ra_lifetime = nvram_get_int("ra_lifetime") ? : 86400;
		else
#endif
		ra_lifetime = nvram_get_int("ra_lifetime") ? : 600; /* 10 minutes for now */
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
#ifdef RTCONFIG_MULTILAN_CFG
		pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
		if (pmtl) {
			get_mtlan_by_idx(SDNFT_TYPE_WAN6, 0, pmtl, &mtl_sz);
			for (i = 0; i < mtl_sz; i++) {
				if (pmtl[i].nw_t.v6_enable && pmtl[i].nw_t.idx != 0) //ignore IoT SDN
					v6_sdn_num++;
			}
			FREE_MTLAN((void *)pmtl);
		}
		strlcpy(v6_prefix, nvram_safe_get(ipv6_nvname("ipv6_prefix")), sizeof(v6_prefix));
		v6_prefix_length = nvram_get_int(ipv6_nvname("ipv6_prefix_length")) ?: 64;
		if (strlen(v6_prefix) && v6_prefix_length <= 104) {
			if (v6_sdn_num > 1) {
				sdn_prefix_length = mtlan_extend_prefix_by_subnet_idx(
					v6_prefix, v6_prefix_length, 0, 8, sdn_prefix, sizeof(sdn_prefix));
			}
			else {
				sdn_prefix_length = v6_prefix_length;
				strlcpy(sdn_prefix, v6_prefix, sizeof(sdn_prefix));
			}
		}
		/*
		else {
			sdn_prefix_length = mtlan_extend_prefix_by_subnet_idx(
				nvram_safe_get("ipv6_ula_random"), 48, 0, 16, sdn_prefix, sizeof(sdn_prefix));
		}
		*/
		if (service != IPV6_PASSTHROUGH && strlen(sdn_prefix) && (stateful || sdn_prefix_length > 64)) {
			dhcp_start = (inet_pton(AF_INET6, nvram_safe_get(ipv6_nvname("ipv6_dhcp_start")), &addr) > 0) ?
					ntohs(addr.s6_addr16[7]) : 0x1000;
			dhcp_end = (inet_pton(AF_INET6, nvram_safe_get(ipv6_nvname("ipv6_dhcp_end")), &addr) > 0) ?
					ntohs(addr.s6_addr16[7]) : 0x2000;
			fprintf(fp, "dhcp-range=lan,%s%04x,%s%04x,%d,%d\n",
				sdn_prefix, (dhcp_start < dhcp_end) ? dhcp_start : dhcp_end,
				sdn_prefix, (dhcp_start < dhcp_end) ? dhcp_end : dhcp_start,
				sdn_prefix_length, dhcp_lifetime);
		}
		else if (announce) {
			fprintf(fp, "dhcp-range=lan,::,constructor:%s,ra-stateless,%d,%d\n",
				lan_ifname, 64, ra_lifetime);
			have_dhcp |= 2; /* DHCPv6 */
		}

#else
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
#endif

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
		value = nvram_safe_get("ipv6_dns1_x");
		value2 = nvram_safe_get("ipv6_dns2_x");
		const char *p = NULL, *p2 = NULL;
		if ((*value && *(p = ipv6_address(value))) || (*value2 && *(p2 = ipv6_address(value2)))) {
#if defined(RTCONFIG_IPV6) && (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2) || defined(ET12))
			if (!strncmp(nvram_safe_get("territory_code"), "CH", 2))
				nvram_unset("ipv6_dns");
#endif
			fprintf(fp, "dhcp-option=lan,option6:23,%s%s%s\n",
				(*value && *p ? value : "0.0.0.0"),
				(*value && *p && *value2 && *p2 ? "," : ""),
				(*value2 && *p2 ? value2 : ""));
		} else
			fprintf(fp, "dhcp-option=lan,option6:23,[::]\n");

		/* LAN Domain */
		value = nvram_safe_get("lan_domain");
		if (*value)
			fprintf(fp, "dhcp-option=lan,option6:24,%s\n", value);
#if defined(RTCONFIG_IPV6) && (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2) || defined(ET12))
		if (!strncmp(nvram_safe_get("territory_code"), "CH", 2))
			fprintf(fp, "rdnss-war\n");
#endif

		/* SNTP & NTP server */
		if (nvram_get_int("ntpd_enable")) {
			fprintf(fp, "dhcp-option=lan,option6:31,%s\n", "[::]");
			fprintf(fp, "dhcp-option=lan,option6:56,%s\n", "[::]");
		}
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
	if (sw_mode() == SW_MODE_ROUTER) {
#ifndef RTCONFIG_MULTILAN_CFG
		fprintf(fp, "interface=%s\n", "tun22");
#endif
		fprintf(fp, "no-dhcp-interface=%s\n", "tun22");
#ifndef RTCONFIG_MULTILAN_CFG
		fprintf(fp, "interface=%s\n", "tun23");
#endif
		fprintf(fp, "no-dhcp-interface=%s\n", "tun23");
	}
#endif
#ifdef RTCONFIG_AMAS_WGN
	wgn_dnsmasq_conf(fp);
#endif
		
#ifdef RTCONFIG_WIREGUARD
	write_wgs_dnsmasq_config(fp);
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
#ifdef RTCONFIG_DNSPRIVACY
	if (nvram_get_int("dnspriv_enable") && nvram_get_int("dnssec_enable") == 2) {
		fprintf(fp, "proxy-dnssec\n");
	} else
#endif
	if (nvram_get_int("dnssec_enable")) {
		fprintf(fp, "trust-anchor=.,20326,8,2,E06D44B80B8F1D39A95C0B0D7C65D08458E880409BBC683457104237C7F8EC8D\n"
		            "dnssec\n");

		/* If NTP isn't set yet, wait until rc's ntp signals us to start validating time */
		if (!nvram_get_int("ntp_ready"))
			fprintf(fp, "dnssec-no-timecheck\n");

		if (nvram_match("dnssec_check_unsigned_x", "0"))
			fprintf(fp, "dnssec-check-unsigned=no\n");
	}
#endif
	if (nvram_match("dns_norebind", "1")) {
		fprintf(fp, "stop-dns-rebind\n");
		fprintf(fp, "rebind-domain-ok=dns.msftncsi.com\n");
	}

	/* Instruct clients like Firefox to not auto-enable DoH */
	n = nvram_get_int("dns_priv_override");
	if ((n == 1) ||
	    (n == 0 && (
#ifdef RTCONFIG_DNSPRIVACY
	       nvram_get_int("dnspriv_enable") ||
#endif
#ifdef RTCONFIG_DNSFILTER
	       (nvram_get_int("dnsfilter_enable_x") && nvram_get_int("dnsfilter_mode"))	// DNSFilter enabled in Global mode
#endif
	    ))
	) {

		fprintf(fp, "address=/use-application-dns.net/\n");
		fprintf(fp, "address=/_dns.resolver.arpa/\n");
		fprintf(fp, "address=/mask.icloud.com/mask-h2.icloud.com/\n");
	}

	/* Protect against VU#598349 */
	fprintf(fp,"dhcp-name-match=set:wpad-ignore,wpad\n"
		   "dhcp-ignore-names=tag:wpad-ignore\n");

	/* dhcp-script */
	fprintf(fp, "dhcp-script=/sbin/dhcpc_lease\n");
#if defined(RTCONFIG_AMAS)
	fprintf(fp, "script-arp\n");
#endif
#if defined(RTCONFIG_DNSMQ_USER)
	if(*nvram_safe_get("dnsmq_user_domain") && *nvram_safe_get("dnsmq_user_ip"))
		fprintf(fp, "address=/%s/%s\n", nvram_safe_get("dnsmq_user_domain"), nvram_safe_get("dnsmq_user_ip"));
#endif
	fprintf(fp, "edns-packet-max=1232\n");

	/* close fp move to the last */
	append_custom_config("dnsmasq.conf",fp);
	fclose(fp);

	use_custom_config("dnsmasq.conf","/etc/dnsmasq.conf");
	run_postconf("dnsmasq","/etc/dnsmasq.conf");
	chmod("/etc/dnsmasq.conf", 0644);

	/* Create resolv.conf with empty nameserver list */
	f_write(dmresolv, NULL, 0, FW_APPEND, 0644);
	/* Create resolv.dnsmasq with empty server list */
	f_write(dmservers, NULL, 0, FW_APPEND, 0644);

#ifdef RTCONFIG_DNSPRIVACY
#ifdef RTCONFIG_MULTILAN_CFG
	start_stubby(sdn_idx);
#else
	start_stubby();
#endif
#endif
	eval("dnsmasq", "--log-async");

#ifdef RTCONFIG_MULTILAN_CFG
	if(sdn_idx != LAN_IN_SDN_IDX)
	{
		//activate all other sdn network
		handle_sdn_feature(sdn_idx, SDN_FEATURE_DNSMASQ, RC_SERVICE_START);
	}
#endif

	/* Update local resolving mode */
	n = readlink("/etc/resolv.conf", buf, sizeof(buf));
	if (nvram_get_int("dns_local_cache")) {
		/* Use dnsmasq for local resolving if it did start,
		 * fallback to wan dns otherwise */
		path = (char *)dmresolv;
		for (ii = 4; ii > 0; ii--) {
			if (pids("dnsmasq")) {
				/* nameserver 127.0.0.1 */
				path = "/rom/etc/resolv.conf"; 
			} else if (ii)
				sleep(1);
		}
	} else
	if (n == sizeof("/rom/etc/resolv.conf") - 1 &&
	    strncmp(buf, "/rom/etc/resolv.conf", n) == 0) {
		/* Use WAN DNS for local resolving only if
		 * nameservers were not changed externally */
		path = (char *)dmresolv;
	} else
		path = NULL;
	if (path && !(n == strlen(path) && strncmp(buf, path, n) == 0)) {
		unlink("/etc/resolv.conf");
		symlink(path, "/etc/resolv.conf");
	}

	TRACE_PT("end\n");
}

#ifdef RTCONFIG_MULTILAN_CFG
void stop_dnsmasq(const int  sdn_idx)
#else
void stop_dnsmasq(void)
#endif
{
	TRACE_PT("begin\n");

	if (getpid() != 1) {
		notify_rc_and_wait_2min("stop_dnsmasq");
		return;
	}

	// Revert back to ISP-filled resolv.conf
        unlink("/etc/resolv.conf");
        symlink(dmresolv, "/etc/resolv.conf");

#ifdef RTCONFIG_MULTILAN_CFG
	char pid_file[256];

	if(sdn_idx == ALL_SDN)
	{
		killall_tk("dnsmasq");
	}
	else if(sdn_idx == LAN_IN_SDN_IDX)
	{
		kill_pidfile_tk("/var/run/dnsmasq.pid");
	}
	else
	{
		snprintf(pid_file, sizeof(pid_file), sdn_dnsmasq_pid_path, sdn_idx);
		kill_pidfile_tk(pid_file);	//don't use handle_sdn_feature() for performance concern.
	}

#else
	killall_tk("dnsmasq");
#endif

#ifdef RTCONFIG_DNSPRIVACY
#ifdef RTCONFIG_MULTILAN_CFG
	stop_stubby(sdn_idx);
#else
	stop_stubby();
#endif
#endif

	TRACE_PT("end\n");
}

#ifdef RTCONFIG_MULTILAN_CFG
void reload_dnsmasq(const int sdn_idx)
{
	char pid_file[256];
	if(sdn_idx == ALL_SDN)
	{
		killall("dnsmasq", SIGHUP);
	}
	else if(sdn_idx == LAN_IN_SDN_IDX)
	{
		kill_pidfile_s("/var/run/dnsmasq.pid", SIGHUP);
	}
	else
	{
		snprintf(pid_file, sizeof(pid_file), sdn_dnsmasq_pid_path, sdn_idx);
		kill_pidfile_s(pid_file, SIGHUP);
	}
}
#else
void reload_dnsmasq(void)
{
	/* notify dnsmasq */
	kill_pidfile_s("/var/run/dnsmasq.pid", SIGHUP);
}
#endif

int dnsmasq_script_main(int argc, char **argv)
{
	// TODO : call function directly
#if defined(RTCONFIG_TR069) && !defined(RTCONFIG_TR181)
	tr_lease_main(argc, argv);
#endif

#if defined(RTCONFIG_AMAS)
	amaslib_lease_main(argc, argv);
#endif
	nmp_get_vendorclass(argc, argv);
	return 0;
}

#ifdef RTCONFIG_DNSPRIVACY
#ifdef RTCONFIG_MULTILAN_CFG
void start_stubby(const int sdn_idx)
#else
void start_stubby(void)
#endif
{
	const static char *stubby_config = "/etc/stubby/stubby.yml";
	const static char *stubby_log = NULL;
	char *stubby_argv[] = { "/usr/sbin/stubby",
		"-g", /* Run in background via _eval, and will not write /var/run/stubby.pid */
		"-C", (char *)stubby_config,
		NULL,				/* -l */
		NULL
	};
	int index = 4;
	FILE *fp;
	char *nv, *nvp, *b;
	char *server, *tlsport, *hostname, *spkipin, *digest;
	int tls_required, tls_possible, max_queries, port;
	char server_addr[INET6_ADDRSTRLEN ];
	size_t server_len;

	union {
		struct in_addr addr4;
#ifdef RTCONFIG_IPV6
		struct in6_addr addr6;
#endif
	} addr;

	TRACE_PT("begin\n");

	if (getpid() != 1) {
		notify_rc("start_stubby");
		return;
	}

#ifdef RTCONFIG_MULTILAN_CFG
	handle_sdn_feature(sdn_idx, SDN_FEATURE_DNSPRIV, RC_SERVICE_STOP | RC_SERVICE_START);
	TRACE_PT("end\n");
	return;
#else
	/* Check if enabled */
	if (!nvram_get_int("dnspriv_enable") || !is_routing_enabled())
		return;

	stop_stubby();

	mkdir_if_none("/etc/stubby");

	if ((fp = fopen(stubby_config, "w")) == NULL)
		return;

	tls_required = nvram_get_int("dnspriv_profile");
	tls_possible = nvram_get_int("ntp_ready");

	/* Basic & privacy settings */
	fprintf(fp,
		"resolution_type: GETDNS_RESOLUTION_STUB\n"
		"dns_transport_list:\n"
		"%s%s"
		"tls_authentication: %s\n"
		"tls_query_padding_blocksize: 128\n"
		"tls_ca_file: \"/etc/ssl/certs/ca-certificates.crt\"\n"
		"appdata_dir: \"/var/lib/misc\"\n"
		"resolvconf: \"%s\"\n"
		"edns_client_subnet_private: 1\n",
		tls_possible ?
			"  - GETDNS_TRANSPORT_TLS\n" : "",
		tls_required && tls_possible ? "" :
			"  - GETDNS_TRANSPORT_UDP\n"
			"  - GETDNS_TRANSPORT_TCP\n",
		tls_required && tls_possible ?
			"GETDNS_AUTHENTICATION_REQUIRED" : "GETDNS_AUTHENTICATION_NONE",
		dmresolv);

#ifdef RTCONFIG_DNSSEC
	/* DNSSEC settings */
	if (nvram_get_int("dnssec_enable") == 2 && tls_possible) {
		fprintf(fp,
			"dnssec_return_status: GETDNS_EXTENSION_TRUE\n");
	}
#endif

	/* Connection settings */
	fprintf(fp,
		"round_robin_upstreams: 1\n"
		"idle_timeout: 9000\n"
		"tls_connection_retries: 2\n"
		"tls_backoff_time: 900\n"
		"timeout: 3000\n");

	/* Limit number of outstanding requests */
	max_queries = nvram_get_int("max_dns_queries");
//#if defined(RTCONFIG_SOC_IPQ8064)
	if (max_queries == 0)
		max_queries = 1500;
//#endif
	if (max_queries)
		fprintf(fp, "limit_outstanding_queries: %d\n", max(150, min(max_queries, 10000)));

	/* Listen address */
	fprintf(fp,
		"listen_addresses:\n"
		"  - 127.0.1.1@53\n");

	/* Upstreams */
	fprintf(fp,
		"upstream_recursive_servers:\n");
	nv = nvp = strdup(nvram_safe_get("dnspriv_rulelist"));
	while (nvp && (b = strsep(&nvp, "<")) != NULL) {
		server = tlsport = hostname = spkipin = NULL;

		/* <server>port>hostname>[digest:]spkipin */
		if ((vstrsep(b, ">", &server, &tlsport, &hostname, &spkipin)) < 4)
			continue;

		/* Check server, can be IPv4/IPv6 address */
		if (*server == '\0')
			continue;
		else if (inet_pton(AF_INET, server, &addr) <= 0
#ifdef RTCONFIG_IPV6
			&& (inet_pton(AF_INET6, server, &addr) <= 0 || !ipv6_enabled())
#endif
		)	continue;

		/* Check port, if specified */
		port = *tlsport ? atoi(tlsport) : 0;
		if (port < 0 || port > 65535)
			continue;

		//Workaround. stubby cannot accept the last character of IPv6 address with as ':'. It would be crashed.
		server_len = strlen(server);
		if(server_len > 1 && server[server_len - 1] == ':')
			snprintf(server_addr, sizeof(server_addr), "%s0", server);
		else
			strlcpy(server_addr, server, sizeof(server_addr));

		fprintf(fp, "  - address_data: %s\n", server_addr);
		if (port)
			fprintf(fp, "    tls_port: %d\n", port);
		if (*hostname)
			fprintf(fp, "    tls_auth_name: \"%s\"\n", hostname);
		if (*spkipin) {
			digest = strchr(spkipin, ':') ? strsep(&spkipin, ":") : "sha256";
			fprintf(fp, "    tls_pubkey_pinset:\n"
				    "      - digest: \"%s\"\n"
				    "        value: %s\n", digest, spkipin);
		}
	}
	if (nv)
		free(nv);

	append_custom_config("stubby.yml", fp);
	fclose(fp);
//	use_custom_config("stubby.yml", (char *)stubby_config);
	run_postconf("stubby", (char *)stubby_config);
	chmod(stubby_config, 0644);

	if (nvram_get_int("stubby_debug")) {
		stubby_argv[index++] = "-l";
		stubby_log = ">/tmp/stubby.log";
	}

	_eval(stubby_argv, stubby_log, 0, NULL);

	TRACE_PT("end\n");
#endif
}

#ifdef RTCONFIG_MULTILAN_CFG
void stop_stubby(const int sdn_idx)
#else
void stop_stubby(void)
#endif
{
	TRACE_PT("begin\n");

#ifdef RTCONFIG_MULTILAN_CFG
	handle_sdn_feature(sdn_idx, SDN_FEATURE_DNSPRIV, RC_SERVICE_STOP);
	TRACE_PT("end\n");
	return;
#else
	if (getpid() != 1) {
		notify_rc("stop_stubby");
		return;
	}

	if (pids("stubby")) {
		kill_pidfile_tk("/var/run/stubby.pid");
		unlink("/var/run/stubby.pid");
	}

	TRACE_PT("end\n");
#endif
}
#endif

#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_MULTILAN_CFG
int set_ip6_sdnaddr(void)
{
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i;
	int cnt = 0;
	char v6_prefix[INET6_ADDRSTRLEN] = {0};
	int v6_prefix_length = 0;
	char v6_rtr_addr[INET6_ADDRSTRLEN] = {0};
	char v6_ula_random[INET6_ADDRSTRLEN] = {0};
	char sdn_prefix[INET6_ADDRSTRLEN] = {0};
	int sdn_prefix_length = 0;
	char ip[INET6_ADDRSTRLEN + 4] = {0};
	char *p;
	char nv[32] = {0};
	int ipv6_service;
	int wan6_unit = wan_primary_ifunit_ipv6();
	int share_subnet = 1;
#ifdef RTCONFIG_MULTIWAN_IF
	int j, wan6_cnt = 0;
#endif

	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));

	if (pmtl) {
		strlcpy(v6_ula_random, nvram_safe_get("ipv6_ula_random"), sizeof(v6_ula_random));
		get_mtlan(pmtl, &mtl_sz);
		// SDN without IoT
		for (i = 1; i < mtl_sz; i++) {
			if (pmtl[i].enable && pmtl[i].nw_t.v6_enable && pmtl[i].nw_t.idx != 0) {
#ifdef RTCONFIG_MULTIWAN_IF
				if (pmtl[i].sdn_t.wan6_idx) {
					wan6_unit = mtwan_get_mapped_unit(pmtl[i].sdn_t.wan6_idx);
					for (j = 0; j < mtl_sz; j++) {
						if (i != j && pmtl[j].enable && pmtl[j].nw_t.v6_enable
						 && mtwan_get_mapped_unit(pmtl[j].sdn_t.wan6_idx) == wan6_unit
						) {
							wan6_cnt++;
							break;
						}
					}
					if (wan6_cnt == 0)
						share_subnet = 0;
				}
#endif
				ipv6_service = get_ipv6_service_by_unit(wan6_unit);
				if (ipv6_service == IPV6_PASSTHROUGH || ipv6_service == IPV6_DISABLED)
					continue;

				strlcpy(v6_prefix, nvram_safe_get(ipv6_nvname_by_unit("ipv6_prefix", wan6_unit)), sizeof(v6_prefix));
				strlcpy(v6_rtr_addr, nvram_safe_get(ipv6_nvname_by_unit("ipv6_rtr_addr", wan6_unit)), sizeof(v6_rtr_addr));
				v6_prefix_length = nvram_get_int(ipv6_nvname_by_unit("ipv6_prefix_length", wan6_unit)) ?:64;

				if (share_subnet) {
					if (v6_prefix[0]) {
						sdn_prefix_length = mtlan_extend_prefix_by_subnet_idx(v6_prefix, v6_prefix_length, i, 8, sdn_prefix, sizeof(sdn_prefix));
						if (sdn_prefix_length) {
							if (pmtl[i].nw_t.addr6[0])
								snprintf(ip, sizeof(ip), "%s%s/%d", sdn_prefix, pmtl[i].nw_t.addr6, sdn_prefix_length);
							else
								snprintf(ip, sizeof(ip), "%s1/%d", sdn_prefix, sdn_prefix_length);
						}
					}
					else if (v6_rtr_addr[0]) {
						strlcpy(v6_prefix, ipv6_prefix_by_unit(NULL, wan6_unit), sizeof(v6_prefix));
						sdn_prefix_length = mtlan_extend_prefix_by_subnet_idx(v6_prefix, v6_prefix_length, i, 8, sdn_prefix, sizeof(sdn_prefix));
						if (sdn_prefix_length) {
							if (pmtl[i].nw_t.addr6[0])
								snprintf(ip, sizeof(ip), "%s%s/%d", sdn_prefix, pmtl[i].nw_t.addr6, sdn_prefix_length);
							else
								snprintf(ip, sizeof(ip), "%s1/%d", sdn_prefix, sdn_prefix_length);
						}
						if (wan6_unit > WAN_UNIT_MAX)
							nvram_set(ipv6_nvname_by_unit("ipv6_prefix", wan6_unit), v6_prefix);
					}
					else {
						sdn_prefix_length = mtlan_extend_prefix_by_subnet_idx(v6_ula_random, 48, 0, 16, sdn_prefix, sizeof(sdn_prefix));
						if (pmtl[i].nw_t.addr6[0])
							snprintf(ip, sizeof(ip), "%s%s/%d", sdn_prefix, pmtl[i].nw_t.addr6, sdn_prefix_length);
						else
							snprintf(ip, sizeof(ip), "%s1/%d", sdn_prefix, sdn_prefix_length);
					}

					if (wan6_unit < WAN_UNIT_MAX) {
						cnt++;
					}
				}
				else {
					if (v6_rtr_addr[0]) {
						snprintf(ip, sizeof(ip), "%s/%d", v6_rtr_addr, v6_prefix_length);
					}
					else if (v6_prefix[0]) {
						if (pmtl[i].nw_t.addr6[0])
							snprintf(ip, sizeof(ip), "%s%s/%d", v6_prefix, pmtl[i].nw_t.addr6, v6_prefix_length);
						else
							snprintf(ip, sizeof(ip), "%s1/%d", v6_prefix, v6_prefix_length);
					}
					else {
						sdn_prefix_length = mtlan_extend_prefix_by_subnet_idx(v6_ula_random, 48, 0, 16, sdn_prefix, sizeof(sdn_prefix));
						if (pmtl[i].nw_t.addr6[0])
							snprintf(ip, sizeof(ip), "%s%s/%d", sdn_prefix, pmtl[i].nw_t.addr6, sdn_prefix_length);
						else
							snprintf(ip, sizeof(ip), "%s1/%d", sdn_prefix, sdn_prefix_length);
					}
				}
				eval("ip", "-6", "addr", "flush", "dev", pmtl[i].nw_t.ifname, "scope", "global");
				eval("ip", "-6", "addr", "add", ip, "dev", pmtl[i].nw_t.ifname);

				p = strchr(ip, '/');
				*p = '\0';
				snprintf(nv, sizeof(nv), "ipv6_rtr_addr_sdn%d", i);
				nvram_set(ipv6_nvname_by_unit(nv, wan6_unit), ip);
			}
		}
		FREE_MTLAN((void *)pmtl);
	}

	return (cnt);
}
char* _add_ip6_lansdnaddr()
{
	char v6_prefix[INET6_ADDRSTRLEN] = {0};
	int v6_prefix_length = 0;
	char v6_rtr_addr[INET6_ADDRSTRLEN] = {0};
	char v6_ula_random[INET6_ADDRSTRLEN] = {0};
	char sdn_prefix[INET6_ADDRSTRLEN] = {0};
	int sdn_prefix_length = 0;
	static char ip[INET6_ADDRSTRLEN + 4] = {0};
	char *p;
	char lan_ifname[IFNAMSIZ] = {0};

	nvram_safe_get_r("lan_ifname", lan_ifname, sizeof(lan_ifname));
	v6_prefix_length = nvram_get_int(ipv6_nvname("ipv6_prefix_length")) ?:64;

	if (set_ip6_sdnaddr()) {
		strlcpy(v6_prefix, nvram_safe_get(ipv6_nvname("ipv6_prefix")), sizeof(v6_prefix));
		strlcpy(v6_rtr_addr, nvram_safe_get(ipv6_nvname("ipv6_rtr_addr")), sizeof(v6_rtr_addr));
		strlcpy(v6_ula_random, nvram_safe_get(ipv6_nvname("ipv6_ula_random")), sizeof(v6_ula_random));

		if (v6_prefix[0]) {
			sdn_prefix_length = mtlan_extend_prefix_by_subnet_idx(v6_prefix, v6_prefix_length, 0, 8, sdn_prefix, sizeof(sdn_prefix));
			if (sdn_prefix_length) {
				snprintf(ip, sizeof(ip), "%s1/%d", sdn_prefix, sdn_prefix_length);
				nvram_set_int(ipv6_nvname("ipv6_prefix_length_sdn"), sdn_prefix_length);
			}
			else if ((p = nvram_get(ipv6_nvname("ipv6_rtr_addr")))) {
				snprintf(ip, sizeof(ip), "%s/%d", p, v6_prefix_length);
			}
		}
		else if (v6_rtr_addr[0]) {
			strlcpy(v6_prefix, ipv6_prefix(NULL), sizeof(v6_prefix));
			sdn_prefix_length = mtlan_extend_prefix_by_subnet_idx(v6_prefix, v6_prefix_length, 0, 8, sdn_prefix, sizeof(sdn_prefix));
			if (sdn_prefix_length) {
				snprintf(ip, sizeof(ip), "%s1/%d", sdn_prefix, sdn_prefix_length);
				nvram_set_int(ipv6_nvname("ipv6_prefix_length_sdn"), sdn_prefix_length);
			}
			else {
				snprintf(ip, sizeof(ip), "%s/%d", v6_prefix, v6_prefix_length);
			}
		}
		else {
			sdn_prefix_length = mtlan_extend_prefix_by_subnet_idx(v6_ula_random, 48, 0, 16, sdn_prefix, sizeof(sdn_prefix));
			snprintf(ip, sizeof(ip), "%s1/%d", sdn_prefix, sdn_prefix_length);
			nvram_set_int(ipv6_nvname("ipv6_prefix_length_sdn"), sdn_prefix_length);
		}
		eval("ip", "-6", "addr", "flush", "dev", lan_ifname, "scope", "global");
		eval("ip", "-6", "addr", "add", ip, "dev", lan_ifname);
		p = strchr(ip, '/');
		*p = '\0';
		if (!nvram_match(ipv6_nvname("ipv6_rtr_addr"), ip))
			nvram_set(ipv6_nvname("ipv6_rtr_addr"), ip);

		return (ip);
	}
	else {
		p = (char*)ipv6_router_address(NULL);
		if (*p) {
			snprintf(ip, sizeof(ip), "%s/%d", p, v6_prefix_length);
			eval("ip", "-6", "addr", "flush", "dev", lan_ifname, "scope", "global");
			eval("ip", "-6", "addr", "add", ip, "dev", lan_ifname);
			if (!nvram_match(ipv6_nvname("ipv6_rtr_addr"), (char*)p))
				nvram_set(ipv6_nvname("ipv6_rtr_addr"), (char*)p);
		}

		return (p);
	}
}

#endif

void add_ip6_lanaddr(void)
{
#ifndef RTCONFIG_MULTILAN_CFG
	char ip[INET6_ADDRSTRLEN + 4];
#endif
	const char *p;

#ifdef RTCONFIG_MULTILAN_CFG
	p = _add_ip6_lansdnaddr();
#else
	p = ipv6_router_address(NULL);
	if (*p) {
		snprintf(ip, sizeof(ip), "%s/%d", p, nvram_get_int(ipv6_nvname("ipv6_prefix_length")) ? : 64);
		eval("ip", "-6", "addr", "add", ip, "dev", nvram_safe_get("lan_ifname"));
		if (!nvram_match(ipv6_nvname("ipv6_rtr_addr"), (char*)p))
			nvram_set(ipv6_nvname("ipv6_rtr_addr"), (char*)p);
	}
#endif

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
#ifdef RTCONFIG_IPV6
	if (nvram_get_int("telnetd_enable") && *p) {
		sleep(2);
		notify_rc("restart_telnetd");
	}
#endif
}

void start_ipv6_tunnel(void)
{
	char ip[INET6_ADDRSTRLEN + 4];
	char router[INET6_ADDRSTRLEN + 1];
	struct in_addr addr4;
	struct in6_addr addr;
	char *wanip, *mtu, *tun_dev, *gateway;
	int service;
	int len;

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
		ip[0] = '\0';
		inet_ntop(AF_INET6, &addr, ip, sizeof(ip));
		len = strlen(ip);
		snprintf(ip+len, sizeof(ip)-len, "/%d", prefixlen);

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
		ip[0] = '\0';
		inet_ntop(AF_INET6, &addr, ip, sizeof(ip));
		len = strlen(ip);
		snprintf(ip+len, sizeof(ip)-len, "/%d", prefixlen);

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

#ifdef RTCONFIG_SOFTWIRE46
void start_s46_tunnel(int unit)
{
	char tmp[256], prefix[sizeof("wanXXXXXXXXXX_")];
	char ipaddr[INET_ADDRSTRLEN], AFTR[INET6_ADDRSTRLEN];
	char *wan_ifname, *wan6_ifname, *elim, *ttl, *end;
	char v6maps[20], draft[4];
#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
	char ip_mask[sizeof("192.168.100.200/255.255.255.255XXX")];
#endif
	int wan_proto, mtu, size = 32;
	struct in_addr netmask;
#ifdef RTCONFIG_MULTILAN_CFG
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i, conflict = 0;
#endif

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	wan_proto = get_wan_proto(prefix);
	switch (wan_proto) {
	case WAN_LW4O6:
	case WAN_MAPE:
	case WAN_V6PLUS:
	case WAN_OCNVC:
		if (sscanf(nvram_safe_get(ipv6_nvname_by_unit("ipv6_s46_addr4", unit)), "%15[^/]/%d", ipaddr, &size) < 1)
			return;
		if (size <= 0 || size > 32)
			size = 32;
		if (wan_proto == WAN_V6PLUS || wan_proto == WAN_OCNVC)
			snprintf(draft, sizeof(draft), "ON");
		else
			snprintf(draft, sizeof(draft), "OFF");
		break;
	case WAN_DSLITE:
		snprintf(ipaddr, sizeof(ipaddr), "%s", nvram_safe_get(ipv6_nvname_by_unit("ipv6_s46_b4addr", unit)));
		snprintf(AFTR, sizeof(AFTR), "%s", nvram_safe_get(ipv6_nvname_by_unit("ipv6_s46_aftr_addr", unit)));
		break;
	default:
		return;
	}

	netmask.s_addr = htonl(INADDR_BROADCAST << (32 - size));
	nvram_set(strcat_r(prefix, "ipaddr", tmp), ipaddr);
	nvram_set(strcat_r(prefix, "netmask", tmp), inet_ntoa(netmask));

#ifdef RTCONFIG_MULTILAN_CFG
	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl) {
		get_mtlan(pmtl, &mtl_sz);
		for (i = 0; i < mtl_sz; i++) {
			if (pmtl[i].enable
			 && inet_equal(ipaddr, inet_ntoa(netmask), pmtl[i].nw_t.addr, pmtl[i].nw_t.netmask)
			) {
				update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_INVALID_IPADDR);
				S46_DBG("[Err] WAN/LAN inet is equal.  [%s] [%s]\n", ipaddr, pmtl[i].nw_t.addr);
				conflict = 1;
				break;
			}
		}
		FREE_MTLAN((void *)pmtl);
		if (conflict)
			return;
	}
#else
	if (inet_equal(nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), nvram_safe_get(strcat_r(prefix, "netmask", tmp)),
		       nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"))) {
		update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_INVALID_IPADDR);
		S46_DBG("[Err] WAN/LAN inet is equal.  [%s] [%s]\n",
			nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), nvram_safe_get("lan_ipaddr"));
		return;
	}
#endif

#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
	/* If return value of test_and_get_free_char_network() is 1 and
	 * we got different IP/netmask from it, the WAN IP/netmask conflicts with known networks.
	 */
	snprintf(ip_mask, sizeof(ip_mask), "%s/%s",
		nvram_pf_safe_get(prefix, "ipaddr"), nvram_pf_safe_get(prefix, "netmask"));
	if (test_and_get_free_char_network(7, ip_mask, EXCLUDE_NET_ALL_EXCEPT_LAN_VLAN) == 1) {
		logmessage("start_wan_if", "%d, %s conflicts with known networks", wan_unit, ip_mask);
		update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_INVALID_IPADDR);
		return;
	}
#endif
#if defined(RTCONFIG_COOVACHILLI)
	restart_coovachilli_if_conflicts(nvram_pf_get(prefix, "ipaddr"), nvram_pf_get(prefix, "netmask"));
#endif

	if (!module_loaded("ip6_tunnel"))
		modprobe("ip6_tunnel");

//#ifdef HND_ROUTER
//	/* TODO: fix tunnel */
//	if (nvram_match("fc_disable", "0"))
//		eval("fc", "config", "--ipv6", "0");
//#endif

	wan_ifname = get_wan_ifname(unit);
	wan6_ifname = get_wan6_ifname(unit);
	mtu = nvram_get_int(ipv6_nvname_by_unit("ipv6_s46_mtu", unit));
	elim = nvram_safe_get(ipv6_nvname_by_unit("ipv6_s46_elim", unit));
	if (strtoul(elim, &end, 0) == 0 && elim == end)
		elim = "none";
	ttl = nvram_safe_get(ipv6_nvname_by_unit("ipv6_s46_ttl", unit));

	if (wan_proto == WAN_DSLITE) {
		eval("ip", "-6", "tunnel", "add", wan_ifname, "mode", "ip4ip6",
		     "remote", AFTR,
		     "local", nvram_safe_get(ipv6_nvname_by_unit("ipv6_ra_addr", unit)),
		     "encaplimit", elim,
		     atoi(ttl) ? "hoplimit" : NULL, ttl);
		S46_DBG("[CMD]:[ip -6 tunnel add %s mode ip4ip6 remote %s local %s encaplimit %s %s %s]\n",
			wan_ifname, AFTR,
			nvram_safe_get(ipv6_nvname_by_unit("ipv6_ra_addr", unit)),
			elim, atoi(ttl) ? "hoplimit" : NULL, ttl);

#if defined(HND_ROUTER) && (defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916))
		/* Disable GSO */
		eval("ethtool", "-K", wan_ifname, "sg", "off");
		S46_DBG("[CMD]:[ethtool -K %s sg off]\n", wan_ifname);
#endif
		eval("ifconfig", wan_ifname, ipaddr, "pointopoint", "192.0.0.1");
		S46_DBG("[CMD]:[ifconfig %s %s pointopoint 192.0.0.1]\n", wan_ifname, ipaddr);
		eval("ip", "link", "set", "dev", wan_ifname, "up");
		S46_DBG("[CMD]:[ip link set dev %s up]\n", wan_ifname);

		/* We are done configuration */
		wan_up(wan_ifname);
		return;
	}

	eval("ip", "-6", "addr", "add", nvram_safe_get(ipv6_nvname_by_unit("ipv6_s46_addr6", unit)),
	     "dev", wan6_ifname, "preferred_lft", "0");

	S46_DBG("[CMD]:[ip -6 addr add %s dev %s preferred_lft 0]\n",
		nvram_safe_get(ipv6_nvname_by_unit("ipv6_s46_addr6", unit)), wan6_ifname);

	eval("ip", "-6", "tunnel", "add", wan_ifname, "mode", "ipip6",
	     "remote", nvram_safe_get(ipv6_nvname_by_unit("ipv6_s46_peer", unit)),
	     "local", nvram_safe_get(ipv6_nvname_by_unit("ipv6_s46_addr6", unit)),
	     "dev", wan6_ifname,
	     "encaplimit", elim,
	     atoi(ttl) ? "hoplimit" : NULL, ttl);

	S46_DBG("[CMD]:[ip -6 tunnel add %s mode ipip6 remote %s local %s dev %s encaplimit %s %s %s]\n",
		wan_ifname, nvram_safe_get(ipv6_nvname_by_unit("ipv6_s46_peer", unit)),
		nvram_safe_get(ipv6_nvname_by_unit("ipv6_s46_addr6", unit)), wan6_ifname,
		elim, atoi(ttl) ? "hoplimit" : NULL, ttl);

	/* Install FMRS into ip6_tunnel module via iproute2*/
	snprintf(v6maps, sizeof(v6maps), S46_MAP_PATH, unit);
	eval("ip", "link", "set", wan_ifname, "type", "ip6tnl", "fmrs", v6maps, "draft", draft);
	S46_DBG("[CMD]:[ip link set %s type ip6tnl fmrs %s draft %s]\n", wan_ifname, v6maps, draft);

#if defined(HND_ROUTER) && (defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916))
	/* Disable GSO */
	eval("ethtool", "-K", wan_ifname, "sg", "off");
	S46_DBG("[CMD]:[ethtool -K %s sg off]\n", wan_ifname);
#endif

#if 0
	char *ports[256] = {0};
	/* Set reserved ports setting */
	snprintf(tmp, sizeof(tmp), "echo \"%s\" > /proc/sys/net/ipv4/ip_local_reserved_ports",
		 calc_s46_port_range(0, nvram_get_int(ipv6_nvname_by_unit("ipv6_s46_psid", unit)),
					nvram_get_int(ipv6_nvname_by_unit("ipv6_s46_psidlen", unit)),
					nvram_get_int(ipv6_nvname_by_unit("ipv6_s46_offset", unit)),
					ports, sizeof(ports)));
	S46_DBG("[CMD]:%s\n", tmp);
	system(tmp);
#endif

	/* Assign static IP address to i/f */
	_ifconfig(wan_ifname, IFUP,
		  nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
		  nvram_safe_get(strcat_r(prefix, "netmask", tmp)), NULL, mtu);

	/* We are done configuration */
	wan_up(wan_ifname);
#ifdef RTCONFIG_TUNNEL
	restart_mastiff();
#endif
}

void stop_s46_tunnel(int unit, int unload)
{
	char prefix[sizeof("wanXXXXXXXXXX_")];
	char /*tmp[256],*/ *wan_ifname, *wan6_ifname;
	int wan_proto;

	//unit = wan_primary_ifunit();
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	wan_proto = get_wan_proto(prefix);
	switch (wan_proto) {
	case WAN_LW4O6:
	case WAN_MAPE:
	case WAN_V6PLUS:
	case WAN_OCNVC:
	case WAN_DSLITE:
		break;
	default:
		return;
	}

	wan_ifname = get_wan_ifname(unit);
	wan6_ifname = get_wan6_ifname(unit);

	wan_down(wan_ifname);

	ifconfig(wan_ifname, 0, NULL, NULL);
	eval("ip", "-6", "tunnel", "del", wan_ifname);

	if (wan_proto != WAN_DSLITE)
		eval("ip", "-6", "addr", "del", nvram_safe_get(ipv6_nvname_by_unit("ipv6_s46_addr6", unit)), "dev", wan6_ifname);

	if (unload) {
		modprobe_r("ip6_tunnel");
//#ifdef HND_ROUTER
//		/* TODO: fix tunnel */
//		if (nvram_match("fc_disable", "0"))
//			eval("fc", "config", "--ipv6", "1");
//#endif
	}

#if 0
	/* Unset reserved ports setting */
	snprintf(tmp, sizeof(tmp), "%s", "echo > /proc/sys/net/ipv4/ip_local_reserved_ports");
	system(tmp);
#endif
}

void
start_v6plusd(int unit)
{
	pid_t pid;
	char buf[64];
	char unit_str[8];
	char *v6plusd_argv[] = {"v6plusd", "-u", unit_str, NULL};

	if (getpid() != 1) {
		snprintf(buf, sizeof(buf), "start_v6plusd %d", unit);
		notify_rc(buf);
		return;
	}

	snprintf(buf, sizeof(buf), V6PLUSD_PIDFILE, unit);
	kill_pidfile_s(buf, SIGTERM);

	snprintf(unit_str, sizeof(unit_str), "%d", unit);
	_eval(v6plusd_argv, NULL, 0, &pid);
}

void stop_v6plusd(int unit)
{
	char buf[64];

	if (getpid() != 1) {
		snprintf(buf, sizeof(buf), "stop_v6plusd %d", unit);
		notify_rc(buf);
		return;
	}

	snprintf(buf, sizeof(buf), V6PLUSD_PIDFILE, unit);
	kill_pidfile_s(buf, SIGTERM);
}

void restart_v6plusd(int unit)
{
	stop_v6plusd(unit);
	start_v6plusd(unit);
}

void start_ocnvcd(int unit)
{
	pid_t pid;
	char buf[64];
	char unit_str[8];
	char *ocnvcd_argv[] = {"ocnvcd", "-u", unit_str, NULL};

	if(getpid() != 1) {
		snprintf(buf, sizeof(buf), "start_ocnvcd %d", unit);
		notify_rc(buf);
		return;
	}

	snprintf(buf, sizeof(buf), OCNVCD_PIDFILE, unit);
	kill_pidfile_s(buf, SIGTERM);

	snprintf(unit_str, sizeof(unit_str), "%d", unit);
	_eval(ocnvcd_argv, NULL, 0, &pid);
}

void stop_ocnvcd(int unit)
{
	char buf[64];

	if (getpid() != 1) {
		snprintf(buf, sizeof(buf), "stop_ocnvcd %d", unit);
		notify_rc(buf);
		return;
	}

	snprintf(buf, sizeof(buf), OCNVCD_PIDFILE, unit);
	kill_pidfile_s(buf, SIGTERM);
}

void restart_ocnvcd(int unit)
{
	stop_ocnvcd(unit);
	start_ocnvcd(unit);
}

void start_dslited(int unit)
{
	pid_t pid;
	char buf[64];
	char unit_str[8];
	char *dslited_argv[] = {"dslited", "-u", unit_str, NULL};

	if(getpid() != 1) {
		snprintf(buf, sizeof(buf), "start_dslited %d", unit);
		notify_rc(buf);
		return;
	}

	snprintf(buf, sizeof(buf), DSLITED_PIDFILE, unit);
	kill_pidfile_s(buf, SIGTERM);

	snprintf(unit_str, sizeof(unit_str), "%d", unit);
	_eval(dslited_argv, NULL, 0, &pid);
}

void stop_dslited(int unit)
{
	char buf[64];

	if (getpid() != 1) {
		snprintf(buf, sizeof(buf), "stop_dslited %d", unit);
		notify_rc(buf);
		return;
	}

	snprintf(buf, sizeof(buf), DSLITED_PIDFILE, unit);
	kill_pidfile_s(buf, SIGTERM);
}

void restart_dslited(int unit)
{
	stop_dslited(unit);
	start_dslited(unit);
}

void s46reset(int unit)
{
	char prefix[sizeof("wanXXXXXXXXXX_")];
	char tmp[100] = {0};

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	nvram_unset("s46_mapsvr_id");
	nvram_unset("s46_mapsvr_state");
	nvram_unset("s46_hgw_name");
	nvram_unset("s46_hgw_ver");
	nvram_unset("s46_hgw_case");
	nvram_unset(strcat_r(prefix, "s46_mapsvr_id", tmp));
	nvram_unset(strcat_r(prefix, "s46_mapsvr_state", tmp));
	nvram_unset(strcat_r(prefix, "s46_hgw_name", tmp));
	nvram_unset(strcat_r(prefix, "s46_hgw_ver", tmp));
	nvram_unset(strcat_r(prefix, "s46_hgw_case", tmp));
	nvram_unset(strcat_r(prefix, "s46_retry", tmp));

	nvram_set(ipv6_nvname_by_unit("ipv6_s46_peer", unit), "");
	nvram_set(ipv6_nvname_by_unit("ipv6_s46_addr6", unit), "");
	nvram_set(ipv6_nvname_by_unit("ipv6_s46_addr4", unit), "");
	nvram_set(ipv6_nvname_by_unit("ipv6_s46_offset", unit), "0");
	nvram_set(ipv6_nvname_by_unit("ipv6_s46_psidlen", unit), "0");
	nvram_set(ipv6_nvname_by_unit("ipv6_s46_psid", unit), "0");
	nvram_set(ipv6_nvname_by_unit("ipv6_s46_mtu", unit), "0");
	nvram_set(ipv6_nvname_by_unit("ipv6_s46_elim", unit), "");
	nvram_set(ipv6_nvname_by_unit("ipv6_s46_ttl", unit), "0");
	nvram_set(ipv6_nvname_by_unit("ipv6_s46_fmrs", unit), "");
	nvram_set(ipv6_nvname_by_unit("ipv6_s46_ports", unit), "");

	nvram_set(strcat_r(prefix, "s46_peer_x", tmp), "");
	nvram_set(strcat_r(prefix, "s46_prefix6_x", tmp), "");
	nvram_set(strcat_r(prefix, "s46_prefix6len_x", tmp), "0");
	nvram_set(strcat_r(prefix, "s46_prefix4_x", tmp), "");
	nvram_set(strcat_r(prefix, "s46_prefix4len_x", tmp), "0");
	nvram_set(strcat_r(prefix, "s46_ealen_x", tmp), "0");
	nvram_set(strcat_r(prefix, "s46_offset_x", tmp), "6");
	nvram_set(strcat_r(prefix, "s46_psidlen_x", tmp), "0");
	nvram_set(strcat_r(prefix, "s46_psid_x", tmp), "0");

	S46_DBG("Unit:[%d] S46 nvram is reset.\n", unit);
	logmessage("[MAP-E]", "Unit:[%d] S46 nvram is reset.", unit);
	return;
}
#endif

void start_rdisc6(void)
{
	int service;
	pid_t pid;
	char *rdisc6_argv[] = { "rdisc6", "-r", RDISC6_RETRY_MAX, (char*) get_wan6face(), NULL };

	if (getpid() != 1) {
		notify_rc_and_wait_1min("start_rdisc6");
		return;
	}

/* Check if enabled */
	service = get_ipv6_service();
	if (
#ifdef RTCONFIG_6RELAYD
	    service != IPV6_PASSTHROUGH &&
#endif
	    service != IPV6_NATIVE_DHCP)
		return;

	if (nvram_get_int("ipv6_no_rdisc6"))
		return;

	stop_rdisc6();

	_eval(rdisc6_argv, NULL, 0, &pid);
}

void stop_rdisc6(void)
{
	killall_tk("rdisc6");
}

void start_ipv6(void)
{
#if defined(RTCONFIG_MULTIWAN_IF)
	int unit, real_unit;
	for (unit = MULTI_WAN_START_IDX; unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; ++unit) {
		real_unit = mtwan_get_mapped_unit(unit);
		start_mtwan_ipv6(real_unit);
	}
#else
	int service = get_ipv6_service();
	char buf[128] = {0};

#if defined(RTCONFIG_MULTIWAN_CFG)
	int i;
	char prefix[sizeof("ipv6X_YYY")];

	for (i = WAN_UNIT_FIRST; i < WAN_UNIT_MAX; ++i) {
		if (!i)
			strcpy(prefix, "ipv6_");
		else
			snprintf(prefix, sizeof(prefix), "ipv6%d_", i);

		if (nvram_pf_match(prefix, "service", "disabled") ||
		    nvram_pf_match(prefix, "service", "dhcp6") && nvram_pf_get_int(prefix, "dhcp_pd") ||
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
		nvram_set(ipv6_nvname("ipv6_llremote"), "");
		nvram_set(ipv6_nvname("ipv6_get_dns"), "");
		nvram_set(ipv6_nvname("ipv6_get_domain"), "");
		break;
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
		nvram_set(ipv6_nvname("ipv6_prefix"), "");
		nvram_set(ipv6_nvname("ipv6_prefix_length"), "");
		nvram_set(ipv6_nvname("ipv6_rtr_addr"), "");
		nvram_set(ipv6_nvname("ipv6_llremote"), "");
		nvram_set(ipv6_nvname("ipv6_get_dns"), "");
		nvram_set(ipv6_nvname("ipv6_get_domain"), "");
		break;
#endif
	case IPV6_MANUAL:
		nvram_set(ipv6_nvname("ipv6_prefix"), "");
		nvram_safe_get_r(ipv6_nvname("ipv6_prefix_length_s"), buf, sizeof(buf));
		nvram_set(ipv6_nvname("ipv6_prefix_length"), buf);
		nvram_safe_get_r(ipv6_nvname("ipv6_rtr_addr_s"), buf, sizeof(buf));
		nvram_set(ipv6_nvname("ipv6_rtr_addr"), buf);
		add_ip6_lanaddr();
		nvram_set(ipv6_nvname("ipv6_llremote"), "");
		break;
	case IPV6_6IN4:
		nvram_safe_get_r(ipv6_nvname("ipv6_prefix_s"), buf, sizeof(buf));
		nvram_set(ipv6_nvname("ipv6_prefix"), buf);
		nvram_safe_get_r(ipv6_nvname("ipv6_prefix_length_s"), buf, sizeof(buf));
		nvram_set(ipv6_nvname("ipv6_prefix_length"), buf);
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
#endif
}

void stop_ipv6(void)
{
#if defined(RTCONFIG_MULTIWAN_IF)
	int unit, real_unit;
	char *lan_ifname = nvram_safe_get("lan_ifname");

	// skip MULTILAN temporarily since destroy/create SDN bridge.
	eval("ip", "-6", "addr", "flush", "scope", "global", "dev", lan_ifname);
	eval("ip", "-6", "neigh", "flush", "dev", lan_ifname);

	for (unit = MULTI_WAN_START_IDX; unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; ++unit) {
		real_unit = mtwan_get_mapped_unit(unit);
		stop_mtwan_ipv6(real_unit);
	}
#else
	char *lan_ifname = nvram_safe_get("lan_ifname");
	char *wan_ifname = (char *) get_wan6face();
	char prefix[sizeof("ffff::/xxx")];
	int i;

#ifdef RTCONFIG_6RELAYD
	stop_6relayd();
#endif
	stop_dhcp6c();
	stop_ipv6_tunnel();

	eval("ip", "-6", "route", "flush", "::/0");
	eval("ip", "-6", "addr", "flush", "scope", "global", "dev", lan_ifname);
	if (wan_ifname && wan_ifname[0] != '\0')
	eval("ip", "-6", "addr", "flush", "scope", "global", "dev", wan_ifname);
	for (i = 1; i < 8; i++) {
		snprintf(prefix, sizeof(prefix), "%04x::/%d", (0xfe00 << (8 - i)) & 0xffff, i);
		eval("ip", "-6", "route", "flush", "root", prefix, "dev", lan_ifname, "table", "main");
		if (wan_ifname && wan_ifname[0] != '\0')
		eval("ip", "-6", "route", "flush", "root", prefix, "dev", wan_ifname, "table", "main");
	}
	eval("ip", "-6", "neigh", "flush", "dev", lan_ifname);
#endif
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
#ifdef RTCONFIG_DPSTA
		!(dpsta_mode() && nvram_get_int("re_mode") == 0) &&
#endif
		!(rp_mode() && nvram_get_int("re_mode") == 0) &&
#ifdef RTCONFIG_AMAS
		!(sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) &&
#endif
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

uint32
wps_gen_pin(char *devPwd, int devPwd_len)
{
	unsigned long PIN;
	unsigned long int accum = 0;
	unsigned char rand_bytes[8];
	int digit;
	char local_devPwd[32];

	/*
	 * buffer size needs to big enough to hold 8 digits plus the string terminition
	 * character '\0'
	*/
	if (devPwd_len < 9)
		return 0;

	/* Generate random bytes and compute the checksum */
	f_read("/dev/urandom", rand_bytes, sizeof(rand_bytes));
	sprintf(local_devPwd, "%08u", *(uint32 *)rand_bytes);
	local_devPwd[7] = '\0';
	PIN = strtoul(local_devPwd, NULL, 10);

	PIN *= 10;
	accum += 3 * ((PIN / 10000000) % 10);
	accum += 1 * ((PIN / 1000000) % 10);
	accum += 3 * ((PIN / 100000) % 10);
	accum += 1 * ((PIN / 10000) % 10);
	accum += 3 * ((PIN / 1000) % 10);
	accum += 1 * ((PIN / 100) % 10);
	accum += 3 * ((PIN / 10) % 10);

	digit = (accum % 10);
	accum = (10 - digit) % 10;

	PIN += accum;
	sprintf(local_devPwd, "%08u", (unsigned int)PIN);
	local_devPwd[8] = '\0';

	/* Output result */
	strncpy(devPwd, local_devPwd, devPwd_len);

	return 1;
}

void
check_wps_enable()
{
	int wps_enable = nvram_get_int("wps_enable_x");
	int unit = nvram_get_int("wps_band_x");

#ifdef RTCONFIG_AMAS
	if (nvram_get_int("re_mode") == 1)
		return;
#endif

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

	if (mediabridge_mode())
		return ret;

	ret = _eval(wpsaide_argv, NULL, 0, &pid);

	return ret;
}
#endif

#if defined(RTCONFIG_CONCURRENTREPEATER) || defined(RTCONFIG_AMAS)
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
#ifdef CONFIG_BCMWL5
	killall_tk("wps_monitor");
	killall_tk("wps_ap");
	killall_tk("wps_enr");
	unlink("/tmp/wps_monitor.pid");
#endif
	if (nvram_match("wps_enable", "1")
#ifdef RTCONFIG_AMAS
		|| nvram_get_int("amesh_wps_enr")
#endif
	) {
#ifdef CONFIG_BCMWL5
		nvram_set("wl_wps_mode", "enabled");
		if (!restore_defaults_g)
		{
#ifdef RTCONFIG_BRCM_HOSTAPD
			if (!nvram_match("hapd_enable", "0") && !pids("wps_pbcd"))
				start_wps_pbcd();
			else
#endif
			_eval(wps_argv, NULL, 0, &pid);
		}
#elif defined RTCONFIG_RALINK
		nvram_set("wps_proc_status", "0");
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
#ifdef RTCONFIG_BRCM_HOSTAPD
	if (!nvram_match("hapd_enable", "0")) {
//		stop_wps_pbcd();
		return ret;
	} else
#endif
	{
		killall_tk("wps_monitor");
		killall_tk("wps_ap");
	}
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
#ifdef RTCONFIG_HND_ROUTER_AX
	int skip_hs = 1;
	int unit = 0;
	char word[256], *next, tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char wl_ifnames[32] = { 0 };

	strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
	foreach (word, wl_ifnames, next) {
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		if (nvram_match(strcat_r(prefix, "mbo_enable", tmp), "1")) {
			skip_hs = 0;
			break;
		}
		unit++;
	}
	if(skip_hs) return 0;
#endif
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
#ifndef RTCONFIG_BCM_MFG
	if (factory_debug())
#endif
#else
#if 0
	if (IS_ATE_FACTORY_MODE())
#endif
#endif
	return ret;

#ifdef RTCONFIG_MFGFW
	if(nvram_match("mfgfw", "1"))
		return 0;
#endif

	stop_wlceventd();

	if (mediabridge_mode())
		return ret;

	ret = _eval(ev_argv, NULL, 0, &pid);

	return ret;
}

void stop_wlceventd(void)
{
	killall_tk("wlceventd");
}
#endif

#ifdef RTCONFIG_HAPDEVENT
int start_hapdevent(void)
{
	char *ev_argv[] = {"/usr/sbin/hapdevent", NULL};
	pid_t pid = 0;
	int ret = 0;

#if defined(RTCONFIG_LANTIQ) || defined(RTCONFIG_QCA)
	if (IS_ATE_FACTORY_MODE())
#endif
	return ret;

	stop_hapdevent();

	if (mediabridge_mode())
		return ret;

	ret = _eval(ev_argv, NULL, 0, &pid);

	return ret;
}

void stop_hapdevent(void)
{
	killall_tk("hapdevent");
}
#endif

#ifdef  RTCONFIG_DNSQUERY_INTERCEPT
void process_dns_dpi_nvram(void)
{
        if(nvram_get_int("dns_dpi_trf_analysis") == 1 || nvram_get_int("dns_dpi_apps_analysis") == 1)
        {
		if(nvram_get_int("nfcm_enable") != 1)
                	nvram_set_int("nfcm_enable", 1); 
        } else {

		if(nvram_get_int("nfcm_enable") != 0)
                	nvram_set_int("nfcm_enable", 0); 

	}
}

int start_dnsqd(void)
{
        char *ev_argv[] = {"/usr/sbin/dnsqd", NULL};
        pid_t pid = 0;
        int ret = 0;
	
	process_dns_dpi_nvram();
        stop_dnsqd();

        if(nvram_get_int("nfcm_enable") == 1)
        {

#ifdef RTCONFIG_AMAS
             if(aimesh_re_node()==0)
#endif
                ret = _eval(ev_argv, NULL, 0, &pid);
        }
        return ret;
}

void stop_dnsqd(void)
{
        killall_tk("dnsqd");
}
#endif

#ifdef RTCONFIG_ROUTERBOOST
int start_asus_rbd(void)
{
        char *ev_argv[] = {"/usr/sbin/asus_rbd", NULL};
        pid_t pid = 0;
        int ret = 0;
        int canRB = IS_RB_QOS() && can_run_router_boost();

        stop_asus_rbd();

#ifdef RTCONFIG_SOC_MT7981
#ifdef RTCONFIG_AMAS

		if(aimesh_re_node() == 1 && nvram_get_int("re_rb_enable") == 1)
			add_routerboost_vsie(ROUTERBOOST_OUI_STR, ROUTERBOOST_OUI_STR);
#endif
#endif

		if(canRB
#ifdef RTCONFIG_AMAS
            &&(aimesh_re_node() == 0)
#endif		
		) 
		{
			if(!module_loaded("deDupTx")) {
				ret = system("insmod deDupTx");
			}
			ret = _eval(ev_argv, NULL, 0, &pid);
		}
        return ret;
}

void stop_asus_rbd(void)
{
        killall_tk("asus_rbd");
		if(module_loaded("deDupTx")){
			system("rmmod deDupTx");
		}
#ifdef RTCONFIG_SOC_MT7981
	del_routerboost_vsie(ROUTERBOOST_OUI_STR, ROUTERBOOST_OUI_STR);
#endif
}
#endif


#ifdef RTCONFIG_AWSIOT
int start_awsiot(void)
{
	char *ev_argv[] = {"/usr/sbin/awsiot", NULL};
	pid_t pid = 0;
	int ret = 0;

	stop_awsiot();


	ret = _eval(ev_argv, NULL, 0, &pid);

	return ret;
}

void stop_awsiot(void)
{
	killall_tk("awsiot");
}
#endif

#ifdef RTCONFIG_NOTIFICATION_CENTER
int start_wlc_nt(void)
{
	char *ev_argv[] = {"/usr/sbin/wlc_nt", NULL};
	pid_t pid = 0;
	int ret = 0;

#ifdef CONFIG_BCMWL5
#ifndef RTCONFIG_BCM_MFG
	if (factory_debug())
#endif
#else
#if 0
	if (IS_ATE_FACTORY_MODE())
#endif
#endif
	return ret;

#ifdef RTCONFIG_MFGFW
	if(nvram_match("mfgfw", "1"))
		return 0;
#endif

	stop_wlc_nt();
	ret = _eval(ev_argv, NULL, 0, &pid);

	return ret;
}

void stop_wlc_nt(void)
{
	killall_tk("wlc_nt");
}

int start_wlc_monitor(void)
{
	char *ev_argv[] = {"/usr/sbin/wlc_monitor", NULL};
	pid_t pid = 0;
	int ret = 0;

#ifdef CONFIG_BCMWL5
#ifndef RTCONFIG_BCM_MFG
	if (factory_debug())
#endif
#else
	if (IS_ATE_FACTORY_MODE())
#endif
	return ret;

#ifdef RTCONFIG_MFGFW
	if(nvram_match("mfgfw", "1"))
		return 0;
#endif

	stop_wlc_monitor();
	ret = _eval(ev_argv, NULL, 0, &pid);

	return ret;
}

void stop_wlc_monitor(void)
{
	killall_tk("wlc_monitor");
}
#endif	/* RTCONFIG_NOTIFICATION_CENTER */

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
	if( nvram_match("wifison_ready", "1") &&
	   (sw_mode() == SW_MODE_AP && !nvram_match("cfg_master", "1")))
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

#ifdef RTCONFIG_AUPNPC
void stop_aupnpc(void)
{
	if (pids("aupnpc"))
		killall_tk("aupnpc");
	start_firewall(wan_primary_ifunit(), 0);
}

int start_aupnpc(void)
{
	if (nvram_match("upnp_enable", "0") || nvram_match("aupnpc_enable", "0") || pids("apnpc"))
		return 0;
	start_firewall(wan_primary_ifunit(), 0);
	return xstart("aupnpc");
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
int exec_8021x_start(int band)
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

	if (flag_8021x) {
		char ifname[8], ifname_prefix[8];

		snprintf(ifname, sizeof(ifname), "%s", get_wififname(band));
		strcpy(ifname_prefix, ifname);
		ifname_prefix[strlen(ifname) - 1] = '\0';

#if defined(RTCONFIG_RALINK_MT7629) 
		return xstart("rtinicapd", "-p", ifname_prefix, "-i", ifname);
#elif defined(RTCONFIG_MT798X) || defined(RTCONFIG_WLMODULE_MT7915D_AP)
		return xstart("8021xd", "-p", ifname_prefix, "-i", ifname);
#else
		if (!strncmp(ifname, "rai", 3))
			return xstart("rtinicapd");
		else
			return xstart("rt2860apd");
#endif
	}
	return 0;
}

int start_8021x(void)
{
	int i = 0;
	char word[8], *next;

	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		exec_8021x_start(i);
		i++;
	}

	return 0;
}

int exec_8021x_stop(int band)
{
#if defined(RTCONFIG_RALINK_MT7629)
	return killall("rtinicapd", SIGTERM);
#elif defined(RTCONFIG_MT798X) || defined(RTCONFIG_WLMODULE_MT7915D_AP)
	return killall("8021xd", SIGTERM);
#else
	char ifname[8];

	snprintf(ifname, sizeof(ifname), "%s", get_wififname(band));
	if (!strncmp(ifname, "rai", 3))
		return killall("rtinicapd", SIGTERM);
	else
		return killall("rt2860apd", SIGTERM);
#endif
}

int stop_8021x(void)
{
	int i = 0;
	char word[8], *next;

	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		exec_8021x_stop(i);
		i++;
	}

	return 0;
}
#endif	/* RTCONFIG_RALINK */

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
	char *nv, *nvp, *b;
	char *mac, *ip, *dns, *hostname;
	char lan_if[IFNAMSIZ];
	unsigned char ea[ETHER_ADDR_LEN];
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

	nv = nvp = strdup(nvram_safe_get("dhcp_staticlist"));
	if (!nv)
		return;

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
		dns = NULL;
		hostname = NULL;
		if ((vstrsep(b, ">", &mac, &ip, &dns, &hostname) < 2))
			continue;

		if (!ether_atoe(mac, ea))
			continue;

		if (dns) {
			struct in_addr in4;
#ifdef RTCONFIG_IPV6
			struct in6_addr in6;

			if (*dns && inet_pton(AF_INET6, dns, &in6) > 0 &&
			    !IN6_IS_ADDR_UNSPECIFIED(&in6) && !IN6_IS_ADDR_LOOPBACK(&in6)) {
				fprintf(fp, "dhcp-option=tag:%s,option6:23,%s\n", mac, dns);
			} else
#endif
			if (*dns && inet_pton(AF_INET, dns, &in4) > 0 &&
			    in4.s_addr != INADDR_ANY && in4.s_addr != INADDR_LOOPBACK && in4.s_addr != INADDR_BROADCAST) {
				fprintf(fp, "dhcp-option=tag:%s,6,%s\n", mac, dns);
			} else
				dns = NULL;
		}

		if (*ip == '\0' || (ip1 = inet_network(ip)) == -1) {
			if (dns)
				fprintf(fp, "dhcp-host=%s,set:%s\n", mac, mac);
			continue;
		}

		if ((ip1 & lan_mask) == lan_net) {
			if(hostname && hostname[0] != '\0' && is_valid_hostname(hostname))
				fprintf(fp, "dhcp-host=%s,set:%s,%s,%s\n", mac, mac, hostname, ip);
			else
				fprintf(fp, "dhcp-host=%s,set:%s,%s\n", mac, mac, ip);
			continue;
		}
#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
		for (i = 0, v = &vlan_nets[0]; i < nr_vnets; ++i, ++v) {
			if ((ip1 & v->mask) != v->net || *v->br_if == '\0')
				continue;
			if(hostname && hostname[0] != '\0' && is_valid_hostname(hostname))
				fprintf(fp, "dhcp-host=%s,set:%s,%s,%s\n", mac, mac, hostname, ip);
			else
				fprintf(fp, "dhcp-host=%s,set:%s,%s\n", mac, mac, ip);
			break;
		}
#endif
	}
	free(nv);
}

#ifdef RTCONFIG_IPV6
#define IPV6_ADDR_GLOBAL        0x0000U
static int _get_ipv6_addr(const char *ifname, char *ipv6addr, const size_t len)
{
	FILE *f;
	int ret = -1, scope, prefix;
	unsigned char ipv6[16];
	char dname[IFNAMSIZ], address[INET6_ADDRSTRLEN];

	if(!ifname || !ipv6addr)
		return ret;

	f = fopen("/proc/net/if_inet6", "r");
	if(!f)
		return ret;

	while (19 == fscanf(f,
						" %2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx %*x %x %x %*x %s",
						&ipv6[0], &ipv6[1], &ipv6[2], &ipv6[3], &ipv6[4], &ipv6[5], &ipv6[6], &ipv6[7], &ipv6[8], &ipv6[9], &ipv6[10], 
						&ipv6[11], &ipv6[12], &ipv6[13], &ipv6[14], &ipv6[15], &prefix, &scope, dname))
	{
		if(strcmp(ifname, dname))
		{
			continue;
		}

		if(inet_ntop(AF_INET6, ipv6, address, sizeof(address)) == NULL)
		{
			continue;
		}

		if(scope == IPV6_ADDR_GLOBAL)
		{
			strlcpy(ipv6addr, address, len);
			ret =0;
			break;
		}
	}

	fclose(f);
	return ret;
}
#endif

int
ddns_updated_main(int argc, char *argv[])
{
	char buf[64], *ip;
#ifdef RTCONFIG_IPV6
	char *ipv6;
#endif
#ifndef RTCONFIG_INADYN
	FILE *fp;

	if (!(fp=fopen("/tmp/ddns.cache", "r"))) return 0;

	fgets(buf, sizeof(buf), fp);
	fclose(fp);

	if (!(ip=strchr(buf, ','))) return 0;

	nvram_set("ddns_cache", buf);
	nvram_set("ddns_ipaddr", ip+1);
#else
	ip = safe_getenv("INADYN_IP");
	if (*ip == '\0')
		return 0;

	/* legacy format */
	snprintf(buf, sizeof(buf), "%ld,%s", time(NULL), ip);

	nvram_set("ddns_cache", buf);
	nvram_set("ddns_ipaddr", ip);
#ifdef RTCONFIG_IPV6
	ipv6 = safe_getenv("INADYN_IPv6");
	if (*ipv6) {
		nvram_set("ddns_ipv6_ipaddr", ipv6);
		nvram_set("ddns_ipv6_updated", "1");
	}
#endif
#endif
	nvram_set("ddns_status", "1");
	nvram_set("ddns_server_x_old", nvram_safe_get("ddns_server_x"));
	nvram_set("ddns_hostname_old", nvram_safe_get("ddns_hostname_x"));
	nvram_set("ddns_updated", "1");

//	logmessage("ddns", "ddns update ok");
	nvram_set("ddns_check_retry", "10"); //Reset DDNS Retry

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
#ifdef RTCONFIG_WIREGUARD
	update_wgs_client_ep();
#endif
#ifdef RTCONFIG_IPSEC
	update_ipsec_s2sc_conf();
#endif

	_dprintf("done\n");

	return 0;
}

// TODO: handle wan0 only now
int
start_ddns(char *caller)
{
	FILE *fp;
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	char *wan_ip, *wan_ifname;
#ifdef RTCONFIG_IPV6
	char ip6_addr[INET6_ADDRSTRLEN] = {0};
#endif
	char *server, *user, *passwd, *host, *service;
#ifndef RTCONFIG_INADYN
	char usrstr[32 + 32 + sizeof(":")];
#else
	int realip = 0; // use Internal WAN IP
#endif
	char cache_path[512] = {0};
	int wild, ret = -1, wan_public = 0;
	int unit, asus_ddns = 0;
	pid_t pid;
	int ddns_check_retry = nvram_get_int("ddns_check_retry");

	if (!is_routing_enabled())
		return 0;

	if (!nvram_get_int("ddns_enable_x")) {
		/* Show the Return Code in UI */
		if (nvram_match("ddns_return_code", "ddns_query")) {
			nvram_set("ddns_return_code", nvram_safe_get("ddns_return_code_chk"));
		}
		return 0;
	}

	/* MAX Retry Count mechanism */
	if (caller == NULL) { // not from watchdog
		//logmessage("ddns", "Reset DDNS Retry.\n");
		nvram_set("ddns_check_retry", "10");
	}

	unit = wan_primary_ifunit();
#if defined(RTCONFIG_DUALWAN)
	if (nvram_match("wans_mode", "lb")) {
		int ddns_wan_unit = nvram_get_int("ddns_wan_unit");
		if (ddns_wan_unit >= WAN_UNIT_FIRST && ddns_wan_unit < WAN_UNIT_MAX) {
			unit = ddns_wan_unit;
		} else {
			int u = get_first_connected_public_wan_unit();
			if (u < WAN_UNIT_FIRST || u >= WAN_UNIT_MAX)
			{
				u = get_first_connected_dual_wan_unit(); /* can use private WAN IP */
				if (u < WAN_UNIT_FIRST || u >= WAN_UNIT_MAX)
				{
					logmessage("ddns", "[%s] None of wan can connect to internet in load balance Dual WAN.", __FUNCTION__);
					return -2;
				}
			}
			unit = u;
		}
	}
#endif
#ifdef RTCONFIG_MULTIWAN_IF
	unit = nvram_get_int("ddns_wan_unit");
	if (unit < WAN_UNIT_FIRST)
		unit = wan_primary_ifunit();
#endif

	server = nvram_safe_get("ddns_server_x");
#ifdef RTCONFIG_INADYN
	if (strcmp(server, "DNS.HE.NET") == 0)
		user = nvram_safe_get("ddns_hostname_x"); /* The username is also the hostname in HE.NET */
	else
#endif
	user = nvram_safe_get("ddns_username_x");
	passwd = nvram_safe_get("ddns_passwd_x");
	host = nvram_safe_get("ddns_hostname_x");
	wild = nvram_get_int("ddns_wildcard_x");

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	wan_ip = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));
	wan_ifname = get_wan_ifname(unit);
#if defined(RTCONFIG_IPV6) && defined(RTCONFIG_INADYN)
	if (nvram_get_int("ddns_ipv6_update")
		&& ipv6_enabled() && (_get_ipv6_addr(wan_ifname, ip6_addr, sizeof(ip6_addr)) != 0))
	{
		logmessage("ddns", "%s has not yet obtained an WAN IPv6 address.(%d)", wan_ifname, ddns_check_retry);
		nvram_unset("ddns_ipv6_updated");
	}
#endif

	wan_public = is_private_subnet(wan_ip); // 0 is public IP; 1, 2, 3, 4 is private IP.
#ifdef RTCONFIG_INADYN
	if (wan_public) { // private WAN IP
#ifdef RTCONFIG_GETREALIP
		if (nvram_get_int(strcat_r(prefix, "realip_state", tmp)) == 2) {
			wan_ip = nvram_safe_get(strcat_r(prefix, "realip_ip", tmp));
			/* use External WAN IP */
			nvram_set_int("ddns_realip_x", 1);
			realip = 1;
		} else {
#ifdef RTCONFIG_IPV6
			if (strnlen(ip6_addr, INET6_ADDRSTRLEN) == 0)
#endif
			{
				/* Trigger watchdog when start fails */
				logmessage("ddns", "%s not find External WAN IP, go retry.(%d)", wan_ifname, ddns_check_retry);
				nvram_unset("ddns_updated");
				nvram_set("ddns_return_code", "ddns_query"); /* for Retry mechanism */
				nvram_set("ddns_return_code_chk", "-1");
				return -1;
			}
		}
#endif
	}
	if (realip == 0) {
		/* use Internal WAN IP */
		nvram_set_int("ddns_realip_x", 0);
	}
#endif

	if (!wan_ip || (inet_addr_(wan_ip) == INADDR_ANY) || (nvram_get_int("link_internet") != 2)) {
		logmessage("ddns", "%s not find WAN IP or link internet, go retry.(%d)", wan_ifname, ddns_check_retry);
		nvram_unset("ddns_updated");
		nvram_set("ddns_return_code", "ddns_query"); /* for Retry mechanism */
		nvram_set("ddns_return_code_chk", "-1");
		return -1;
	}

	if (
#ifdef RTCONFIG_INADYN
		realip == 0 &&
#ifdef RTCONFIG_IPV6
		(!nvram_get_int("ddns_ipv6_update") || (strnlen(ip6_addr, INET6_ADDRSTRLEN) == 0)) &&
#endif
#endif
		wan_public) { // private WAN IP
		logmessage("ddns", "use Private WAN IP (%s).(%d)", wan_ip, ddns_check_retry);
		nvram_unset("ddns_updated");
		nvram_set("ddns_return_code", "299");
		nvram_set("ddns_return_code_chk", "299");
		nvram_set_int("ddns_last_wan_unit", unit);
		return -1;
	}

#ifndef RTCONFIG_INADYN
	snprintf(cache_path, sizeof(cache_path), "/tmp/ddns.cache");
#else
	snprintf(cache_path, sizeof(cache_path), "/var/cache/inadyn/%s.cache", host);
#endif

	if (inet_addr_(wan_ip) == inet_addr_(nvram_safe_get("ddns_ipaddr")) &&
		strcmp(nvram_safe_get("ddns_server_x"), nvram_safe_get("ddns_server_x_old")) == 0 &&
		strcmp(nvram_safe_get("ddns_hostname_x"), nvram_safe_get("ddns_hostname_old")) == 0
#if defined(RTCONFIG_IPV6) && defined(RTCONFIG_INADYN)
		&& (!ipv6_enabled() || nvram_match("ddns_ipv6_update", "0") || (ipv6_enabled() && nvram_match("ddns_ipv6_update", "1") && 
		((strnlen(ip6_addr, INET6_ADDRSTRLEN) == 0 && strnlen(nvram_safe_get("ddns_ipv6_ipaddr"), INET6_ADDRSTRLEN) == 0) || nvram_match("ddns_ipv6_ipaddr", ip6_addr))))
#endif
		&& f_exists(cache_path) /* Not forced to restart DDNS */
	) {
		logmessage("ddns", "IP address, server and hostname have not changed since the last update.");
		nvram_set("ddns_updated", "1");
		/* for UI display */
		nvram_set("ddns_return_code", "");
		nvram_set("ddns_return_code_chk", ",200");
		return 0;
	}

#ifndef RTCONFIG_INADYN
	if (strcmp(server, "WWW.DYNDNS.ORG")==0)
		service = "dyndns";
	else if (strcmp(server, "WWW.DYNDNS.ORG(CUSTOM)")==0)
		service = "dyndns-custom";
	else if (strcmp(server, "WWW.DYNDNS.ORG(STATIC)")==0)
		service = "dyndns-static";
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
	else if (strcmp(server, "WWW.SELFHOST.DE") == 0)
		service = "selfhost";
	else if (strncmp(server, "WWW.ASUS.COM", 12)==0) { /* WWW.ASUS.COM or WWW.ASUS.COM.CN */
		service = "dyndns", asus_ddns = 1;
	}
	else if (strcmp(server, "DOMAINS.GOOGLE.COM") == 0)
		service = "dyndns", asus_ddns=3;
#else
	if (strcmp(server, "WWW.DYNDNS.ORG") == 0 ||
	    strcmp(server, "WWW.DYNDNS.ORG(CUSTOM)") == 0 ||
	    strcmp(server, "WWW.DYNDNS.ORG(STATIC)") == 0)
		service = "default@dyndns.org";
	else if (strcmp(server, "WWW.ZONEEDIT.COM") == 0)
		service = "default@zoneedit.com";
	else if (strcmp(server, "WWW.EASYDNS.COM") == 0)
		service = "default@easydns.com";
	else if (strcmp(server, "WWW.DNSOMATIC.COM") == 0)
		service = "default@dnsomatic.com";
	else if (strcmp(server, "DNS.HE.NET") == 0)
		service = "dyndns@he.net";
	else if (strcmp(server, "FREEDNS.AFRAID.ORG") == 0)
		service = "default@freedns.afraid.org";
	else if (strcmp(server, "FREEMYIP.COM") == 0)
		service = "default@freemyip.com";
	else if (strcmp(server, "WWW.TUNNELBROKER.NET") == 0) {
		service = "default@tunnelbroker.net";
		eval("iptables", "-t", "filter", "-D", "INPUT", "-p", "icmp", "-s", "66.220.2.74", "-j", "ACCEPT");
		eval("iptables", "-t", "filter", "-I", "INPUT", "1", "-p", "icmp", "-s", "66.220.2.74", "-j", "ACCEPT");
		nvram_set("ddns_tunbkrnet", "1");
	}
	else if (strcmp(server, "WWW.NO-IP.COM") == 0)
		service = "default@no-ip.com";
	else if (strcmp(server, "WWW.NAMECHEAP.COM")==0) {
		service = "namecheap";
		asus_ddns = 10;
	}
	else if (strcmp(server, "CUSTOM")==0)
		service = "";
	else if (strcmp(server, "FREEDNS.AFRAID.ORG") == 0)
		service = "default@freedns.afraid.org";
	else if (strcmp(server, "WWW.SELFHOST.DE") == 0)
		service = "default@selfhost.de";
	else if (strncmp(server, "WWW.ASUS.COM", 12) == 0) { /* WWW.ASUS.COM or WWW.ASUS.COM.CN */
#ifdef RTCONFIG_ACCOUNT_BINDING
		if(is_account_bound() && nvram_match("ddns_replace_status", "1") &&
			((strstr(nvram_safe_get("aae_ddnsinfo"), ".asuscomm.com") && (strstr(nvram_safe_get("ddns_hostname_x"), ".asuscomm.com")))
			|| (strstr(nvram_safe_get("aae_ddnsinfo"), ".asuscomm.cn") && (strstr(nvram_safe_get("ddns_hostname_x"), ".asuscomm.cn")))))
			service = "updatev2@asus.com";
		else
#endif			
			service = "update@asus.com";
		user = get_ddns_macaddr();
		passwd = nvram_safe_get("secret_code");
		asus_ddns = 1;
	}
	else if (strcmp(server, "DOMAINS.GOOGLE.COM") == 0)
		service = "default@domains.google.com";
#endif
	else if (strcmp(server, "WWW.ORAY.COM") == 0) {
		service = "peanuthull", asus_ddns = 2;
	} else {
		logmessage("ddns", "Error ddns server name: %s\n", server);
		return 0;
	}

#ifndef RTCONFIG_INADYN
	snprintf(usrstr, sizeof(usrstr), "%s:%s", user, passwd);
#endif

	/* Show WAN unit used by ddns client to console and syslog. */
	_dprintf("start_ddns update %s %s, wan_unit %d\n", server, service, unit);
	logmessage("ddns", "update %s %s, wan_unit %d\n", server, service, unit);

	nvram_set("ddns_return_code", "ddns_query");

#ifndef RTCONFIG_INADYN
	if (pids("ez-ipupdate")) {
		killall("ez-ipupdate", SIGINT);
		sleep(1);
	}
#else
	if (pids("inadyn"))
		killall_tk("inadyn");
#endif
	if (pids("phddns")) {
		killall("phddns", SIGINT);
		sleep(1);
	}

#ifndef RTCONFIG_INADYN
	unlink(cache_path);
	nvram_unset("ddns_cache");
	nvram_unset("ddns_ipaddr");
	nvram_unset("ddns_status");
	nvram_unset("ddns_updated");
	logmessage("ddns", "Clear ddns cache.");
#else
	if ((!nvram_match("ddns_server_x_old", "") && strcmp(nvram_safe_get("ddns_server_x"), nvram_safe_get("ddns_server_x_old")) != 0)
		|| (!nvram_match("ddns_hostname_old", "") && strcmp(nvram_safe_get("ddns_hostname_x"), nvram_safe_get("ddns_hostname_old")) != 0)
		|| (inet_addr_(wan_ip) != inet_addr_(nvram_safe_get("ddns_ipaddr")))
#ifdef RTCONFIG_IPV6
		|| (!nvram_match("ddns_ipv6_ipaddr", ip6_addr))
#endif
		|| (!f_exists(cache_path)) /* Forced to restart DDNS */
	) {
		system("rm -f /var/cache/inadyn/*.cache");/* */
		nvram_unset("ddns_cache");
		nvram_unset("ddns_ipaddr");
#ifdef RTCONFIG_IPV6
		nvram_unset("ddns_ipv6_ipaddr");
		nvram_unset("ddns_ipv6_updated");
#endif
		nvram_unset("ddns_status");
		nvram_unset("ddns_updated");
		logmessage("ddns", "Clear ddns cache.");
	}
#endif

#ifndef RTCONFIG_INADYN
	if(3 == asus_ddns)
	{
		FILE *time_fp;
		time_t now;

		if((time_fp=fopen(cache_path,"w")))
		{
			fprintf(time_fp,"%ld,%s",time(&now),wan_ip);
			fclose(time_fp);
		}
		ret = eval("GoogleDNS_Update.sh", user, passwd, host, wan_ip);
	} else
#endif
	if (asus_ddns == 2) { //Peanuthull DDNS
		if ((fp = fopen("/etc/phddns.conf", "w")) != NULL) {
			fprintf(fp, "[settings]\n");
			fprintf(fp, "szHost = phddns60.oray.net\n");
			fprintf(fp, "szUserID = %s\n", user);
			fprintf(fp, "szUserPWD = %s\n", passwd);
			fprintf(fp, "nicName = %s\n", wan_ifname);
			fprintf(fp, "szLog = /var/log/phddns.log\n");
			fclose(fp);

			ret = eval("phddns", "-c", "/etc/phddns.conf", "-d");
		}
	} else
#ifndef RTCONFIG_INADYN
	if (asus_ddns == 1) {
		char *nserver = nvram_invmatch("ddns_serverhost_x", "") ?
			nvram_safe_get("ddns_serverhost_x") :
			"nwsrv-ns1.asus.com";
		char *argv[] = { "ez-ipupdate", "-S", service, "-i", wan_ifname,
				"-h", host, "-A", "2", "-s", nserver,
				"-e", "/sbin/ddns_updated", "-b", cache_path, NULL };
		ret = _eval(argv, NULL, 0, &pid);
	} else if (*service) {
		char *argv[] = { "ez-ipupdate", "-S", service, "-i", wan_ifname, "-h", host,
		     "-u", usrstr, wild ? "-w" : "", "-e", "/sbin/ddns_updated",
		     "-b", cache_path, NULL };
		ret = _eval(argv, NULL, 0, &pid);
	}
#else
	if (*service) {
#ifdef RTCONFIG_LETSENCRYPT
		int asus_le = (asus_ddns == 1 && nvram_get_int("le_enable") == 1);
#endif
		char *inadyn_argv[] = { "/usr/sbin/inadyn",
			"-e", "/sbin/ddns_updated",
			"-l", nvram_get_int("ddns_debug") ? "debug" : "notice",
#ifdef RTCONFIG_LETSENCRYPT
			asus_le ? "-1" : NULL,
			asus_le ? "--force" : NULL,
#endif
			NULL
		};

		if ((fp = fopen("/etc/inadyn.conf", "w"))) {
			if (asus_ddns == 10) {
				fprintf(fp, "custom namecheap {\n");
				fprintf(fp, "ddns-server = dynamicdns.park-your-domain.com\n");
				// We store the domain.tld in the username nvram
				fprintf(fp, "ddns-path = \"/update?domain=%%u&password=%%p&host=%%h\"\n");
			} else {
				fprintf(fp, "provider %s {\n", service);
			}
			fprintf(fp, "hostname = %s\n", host);
			fprintf(fp, "username = '%s'\n", ppp_safe_escape(user, tmp, sizeof(tmp)));
			fprintf(fp, "password = '%s'\n", ppp_safe_escape(passwd, tmp, sizeof(tmp)));
			if (wild)
				fprintf(fp, "wildcard = true\n");
#ifdef RTCONFIG_GETREALIP
			if (realip)
#ifdef RTCONFIG_MULTIWAN_IF
				fprintf(fp, "checkip-command = 'realip %d'\n", unit);
#else
				fprintf(fp, "checkip-command = '%s'\n", "getrealip.sh");
#endif
#endif
			fprintf(fp, "}\n");

			fprintf(fp, "iterations = 1\n");
			fprintf(fp, "iface = %s\n", wan_ifname); /* External WAN IP also need: Private IPv4 + Public IPv6 */
			fprintf(fp, "ca-trust-file = /etc/ssl/certs/ca-certificates.crt\n");
			if (!nvram_get_int("ntp_ready"))
				fprintf(fp, "broken-rtc = true\n");
			/* temporary for asus server */
			if (asus_ddns == 1)
				fprintf(fp, "secure-ssl = false\n");
#ifdef RTCONFIG_IPV6
			/* For DYNDNS ok, but not for NO-IP and ASUS.COM */
			if(nvram_get_int("ddns_ipv6_update") && (strncmp(server, "WWW.DYNDNS.ORG", 14) == 0))
				fprintf(fp, "allow-ipv6 = true\n");
#endif

			append_custom_config("inadyn.conf", fp);

			fclose(fp);

			use_custom_config("inadyn.conf", "/etc/inadyn.conf");
			run_postconf("inadyn", "/etc/inadyn.conf");

#if defined(RTCONFIG_TUNNEL) && defined(RTCONFIG_ACCOUNT_BINDING)
			if(is_account_bound() && nvram_match("ddns_replace_status", "1") &&
				((strstr(nvram_safe_get("aae_ddnsinfo"), ".asuscomm.com") && (strstr(nvram_safe_get("ddns_hostname_x"), ".asuscomm.com")))
			|| (strstr(nvram_safe_get("aae_ddnsinfo"), ".asuscomm.cn") && (strstr(nvram_safe_get("ddns_hostname_x"), ".asuscomm.cn")))))
			{
				nvram_set("asusddns_token_state", "0");
				if(update_asus_ddns_token() != 1 || nvram_get_int("asusddns_token_state") != 1)
				{
					nvram_set("ddns_return_code", ",390");
					nvram_set("ddns_return_code_chk", ",390");
					nvram_set_int("ddns_last_wan_unit", unit);
					logmessage("ddns", "update ddns token failed(%d)\n", nvram_get_int("asusddns_token_state"));
					return -1;
				}
			}
#endif
			if(!nvram_match("ddns_act", "update"))
			{
				unlink(cache_path);
				nvram_set("ddns_act", "update");
			}
			logmessage("ddns", "Start Inadyn(%d).\n", ddns_check_retry);
			ret = _eval(inadyn_argv, NULL, 0, &pid);
		}
	} else {	// Custom DDNS
		// Block until it completes and updates the DDNS update results in nvram
			run_custom_script("ddns-start", 120, wan_ip, NULL);
			return 0;
	}
#endif

	if (ret == 0)
		nvram_set_int("ddns_last_wan_unit", unit);

	run_custom_script("ddns-start", 0, wan_ip, NULL);

	return ret;
}

void
stop_ddns(void)
{
#ifndef RTCONFIG_INADYN
	if (pids("ez-ipupdate"))
		killall("ez-ipupdate", SIGINT);
#else
	if (pids("inadyn"))
		killall_tk("inadyn");
#endif
	if (pids("phddns"))
		killall("phddns", SIGINT);
	if (nvram_match("ddns_tunbkrnet", "1")) {
		int evalRet = eval("iptables-restore", "/tmp/filter_rules");
		rule_apply_checking("services", __LINE__, "/tmp/filter_rules", evalRet);
		run_custom_script("firewall-start", 0, get_wan_ifname(wan_primary_ifunit()), NULL);
		nvram_unset("ddns_tunbkrnet");
	}
#ifdef RTCONFIG_OPENVPN
	update_ovpn_profie_remote();
#endif
}

void
clean_ddns(void)
{
	nvram_set("ddns_enable_x", "0");
	nvram_set("ddns_server_x", "");
	nvram_set("ddns_server_x_old", "");
	nvram_set("ddns_hostname_x", "");
	nvram_set("ddns_hostname_old", "");
	nvram_set("ddns_cache", "");
	nvram_set("ddns_ipaddr", "");
#if defined(RTCONFIG_IPV6) && defined(RTCONFIG_INADYN)
	nvram_set("ddns_ipv6_ipaddr", "");
#endif
#ifdef RTCONFIG_ACCOUNT_BINDING
	nvram_set("ddns_replace_status", "0");
#endif
	stop_ddns();
#ifdef RTCONFIG_LETSENCRYPT
	if (nvram_get_int("le_enable") == 1) {
		nvram_set("le_enable", "0");
		unlink("/jffs/cert.tgz");
		stop_httpd();
		start_httpd();
	}
#endif
	nvram_commit();
}

int
asusddns_unregister(void)
{
#ifdef RTCONFIG_INADYN
	FILE *fp;
	char cache_path[512] = {0};
#endif
	char *wan_ifname;
#ifndef RTCONFIG_INADYN
	char *nserver;
#else
	int realip;
#endif
	int unit, ret = -1;

	unit = wan_primary_ifunit();
#if defined(RTCONFIG_DUALWAN)
	if (nvram_match("wans_mode", "lb")) {
		int ddns_wan_unit = nvram_get_int("ddns_wan_unit");

		if (ddns_wan_unit >= WAN_UNIT_FIRST && ddns_wan_unit < WAN_UNIT_MAX) {
			unit = ddns_wan_unit;
		} else {
			int u = get_first_connected_public_wan_unit();
			if (u < WAN_UNIT_FIRST || u >= WAN_UNIT_MAX)
			{
				u = get_first_connected_dual_wan_unit(); /* can use private WAN IP */
				if (u < WAN_UNIT_FIRST || u >= WAN_UNIT_MAX)
				{
					logmessage("ddns", "[%s] None of wan can connect to internet in load balance Dual WAN.", __FUNCTION__);
					return -2;
				}
			}
			unit = u;
		}
	}
#endif
#ifdef RTCONFIG_MULTIWAN_IF
	unit = nvram_get_int("ddns_wan_unit");
	if (unit < WAN_UNIT_FIRST)
		unit = wan_primary_ifunit();
#endif

	wan_ifname = get_wan_ifname(unit);
#ifdef RTCONFIG_INADYN
	realip = nvram_get_int("ddns_realip_x");
#endif

	nvram_set("ddns_return_code", "ddns_unregister");

#ifndef RTCONFIG_INADYN
	if (pids("ez-ipupdate"))
	{
		killall("ez-ipupdate", SIGINT);
		sleep(1);
	}
#else
	if (pids("inadyn"))
		killall_tk("inadyn");
#endif

	nvram_unset("asusddns_reg_result");

#ifndef RTCONFIG_INADYN
	nserver = nvram_invmatch("ddns_serverhost_x", "") ?
		    nvram_safe_get("ddns_serverhost_x") :
		    "nwsrv-ns1.asus.com";
_dprintf("%s: do ez-ipupdate to unregister! unit = %d wan_ifname = %s nserver = %s hostname = %s\n", __FUNCTION__, unit, wan_ifname, nserver, nvram_safe_get("ddns_hostname_x"));

	ret = eval("ez-ipupdate",
	     "-S", "dyndns", "-i", wan_ifname, "-h", nvram_safe_get("ddns_hostname_x"),
	     "-A", "3", "-s", nserver);
#else
	{
		char *inadyn_argv[] = { "/usr/sbin/inadyn",
			"-1", "--force",
			"-l", nvram_get_int("ddns_debug") ? "debug" : "notice",
			NULL
		};

		if ((fp = fopen("/etc/inadyn.conf", "w"))) {
#ifdef RTCONFIG_ACCOUNT_BINDING
			if(is_account_bound() && nvram_match("ddns_replace_status", "1") &&
				((strstr(nvram_safe_get("aae_ddnsinfo"), ".asuscomm.com") && (strstr(nvram_safe_get("ddns_hostname_x"), ".asuscomm.com")))
				|| (strstr(nvram_safe_get("aae_ddnsinfo"), ".asuscomm.cn") && (strstr(nvram_safe_get("ddns_hostname_x"), ".asuscomm.cn")))))
				fprintf(fp, "provider %s {\n", "unregisterv2@asus.com");
			else
#endif
				fprintf(fp, "provider %s {\n", "unregister@asus.com");
			fprintf(fp, "hostname = %s\n", nvram_safe_get("ddns_hostname_x"));
			fprintf(fp, "username = '%s'\n", get_ddns_macaddr());
			fprintf(fp, "password = '%s'\n", nvram_safe_get("secret_code"));
#ifdef RTCONFIG_GETREALIP
			if (realip)
#ifdef RTCONFIG_MULTIWAN_IF
				fprintf(fp, "checkip-command = 'realip %d'\n", unit);
#else
				fprintf(fp, "checkip-command = '%s'\n", "getrealip.sh");
#endif
#endif
			fprintf(fp, "}\n");

			fprintf(fp, "iterations = 1\n");
			if (!realip)
				fprintf(fp, "iface = %s\n", wan_ifname);
			fprintf(fp, "ca-trust-file = /etc/ssl/certs/ca-certificates.crt\n");
			if (!nvram_get_int("ntp_ready"))
				fprintf(fp, "broken-rtc = true\n");
			/* temporary for asus server */
			if (1)
				fprintf(fp, "secure-ssl = false\n");
			fclose(fp);

#if defined(RTCONFIG_TUNNEL) && defined(RTCONFIG_ACCOUNT_BINDING)
			if(is_account_bound() && nvram_match("ddns_replace_status", "1") &&
				((strstr(nvram_safe_get("aae_ddnsinfo"), ".asuscomm.com") && (strstr(nvram_safe_get("ddns_hostname_x"), ".asuscomm.com")))
				|| (strstr(nvram_safe_get("aae_ddnsinfo"), ".asuscomm.cn") && (strstr(nvram_safe_get("ddns_hostname_x"), ".asuscomm.cn")))))
			{
				nvram_set("asusddns_token_state", "0");
				if(update_asus_ddns_token() != 1 || nvram_get_int("asusddns_token_state") != 1)
				{
					nvram_set("asusddns_reg_result", "unregister,390");
					return -1;
				}
			}
#endif			
			if(!nvram_match("ddns_act", "unregister"))
			{
				snprintf(cache_path, sizeof(cache_path), "/var/cache/inadyn/%s.cache", nvram_safe_get("ddns_hostname_x"));
				unlink(cache_path);
				nvram_set("ddns_act", "unregister");
			}
			ret = _eval(inadyn_argv, NULL, 0, NULL);
		}
	}
#endif

	return ret;
}

#ifdef RTCONFIG_RSYSLOGD
char *get_loglevel_string(int loglevel){
	if(loglevel == LOG_EMERG)
		return "emerg";
	else if(loglevel == LOG_ALERT)
		return "alert";
	else if(loglevel == LOG_CRIT)
		return "crit";
	else if(loglevel == LOG_ERR)
		return "err";
	else if(loglevel == LOG_WARNING)
		return "warn";
	else if(loglevel == LOG_NOTICE)
		return "notice";
	else if(loglevel == LOG_INFO)
		return "info";
	else if(loglevel == LOG_DEBUG)
		return "debug";
	else
		return "none";
}

void write_rsyslogd_conf(){
	FILE *fp;
	char *conf_file = "/etc/rsyslog.conf";
	int logsize, loglevel;
	char *ptr, logipaddr[256], logport[8];

	if(!(fp = fopen(conf_file, "w+"))){
		perror(conf_file);
		return;
	}

	fprintf(fp, "$ModLoad imuxsock\n");
	fprintf(fp, "$ModLoad imklog\n");

	fprintf(fp, "$KLogPermitNonKernelFacility on\n");
	fprintf(fp, "$SystemLogRateLimitInterval 0\n");
	fprintf(fp, "$SystemLogRateLimitBurst 0\n");
	fprintf(fp, "$IMUXSockRateLimitInterval 0\n");
	fprintf(fp, "$IMUXSockRateLimitBurst 0\n");
	fprintf(fp, "$ActionFileDefaultTemplate RSYSLOG_TraditionalFileFormat\n");

	if((logsize = nvram_get_int("log_size")) > 0)
		fprintf(fp, "$MaxMessageSize\t%d\n", logsize);

	loglevel = nvram_get_int("console_loglevel");
	fprintf(fp, "*.%s\t%s\n", get_loglevel_string(loglevel), get_syslog_fname(0));

	snprintf(logport, sizeof(logport), "%s", nvram_safe_get("log_port"));
	if((ptr = nvram_get("log_ipaddr")) != NULL && *ptr){
		snprintf(logipaddr, sizeof(logipaddr), "%s%s%s", ptr,
				(atoi(logport) > 0)?":":"",
				(atoi(logport) > 0)?logport:"");
		fprintf(fp, "*.*\t@@%s\n", logipaddr);
	}

	if (fp)
		fclose(fp);
}

int
start_syslogd(void)
{
	int pid;
	char *cmd[] = {"/usr/sbin/rsyslogd", NULL};

	write_rsyslogd_conf();

	if(!pids("rsyslogd"))
		_eval(cmd, NULL, 0, &pid);

	return 0;
}

void
stop_syslogd(void)
{
	if(pids("rsyslogd"))
		killall_tk("rsyslogd");
}
#else /* RTCONFIG_RSYSLOGD */

int
start_syslogd(void)
{
	char syslog_path[PATH_MAX];
	char syslog_addr[128];
#ifdef RTCONFIG_AMAS
	char syslog_hostname[128];
#endif
	char *syslogd_argv[] = {"/sbin/syslogd",
		"-m", "0",				/* disable marks */
		"-S",					/* small log */
//		"-D",					/* suppress dups */
		"-O", syslog_path,			/* /tmp/syslog.log or /jffs/syslog.log */
		NULL, NULL,				/* -s log_size */
		NULL, NULL,				/* -l log_level */
		NULL, NULL,				/* -R log_ipaddr[:port] */
		NULL,					/* -L log locally too */
#ifdef RTCONFIG_AMAS
		NULL, NULL,				/* -H hostname */
#endif
		NULL
	};
	int argc;
	for (argc = 0; syslogd_argv[argc]; argc++);

	snprintf(syslog_path, sizeof(syslog_path), "%s", get_syslog_fname(0));

	if (nvram_get_int("log_size")) {
		syslogd_argv[argc++] = "-s";
#if defined(MAPAC2200) || defined(MAPAC1300) || defined(VZWAC1300) || defined(SHAC1300) || defined(RTAC95U)
		if (nvram_get_int("lyra_dbg"))
			syslogd_argv[argc++] = "1024";
		else
#endif
#ifdef RTCONFIG_HND_ROUTER
#ifdef RTCONFIG_DBLOG
		if(nvram_match("dblog_adj_syslog", "1"))
		{
			syslogd_argv[argc++] = "1024";
			nvram_set("dblog_adj_syslog", "0");
		}
		else
#endif /* RTCONFIG_DBLOG */
#endif /* RTCONFIG_HND_ROUTER */
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

#ifdef RTCONFIG_AMAS
		if (1 /* TODO: differ standalone mode from aimesh cap/re to avoid hostname trashing */) {
			char gid[8];

			strlcpy(gid, nvram_safe_get("cfg_group"), sizeof(gid));
			snprintf(syslog_hostname, sizeof(syslog_hostname), "%s-%s-%s",
				 get_lan_hostname(), gid, node_str());

			syslogd_argv[argc++] = "-H";
			syslogd_argv[argc++] = syslog_hostname;
		}
#endif
	}

//#if defined(RTCONFIG_JFFS2LOG) && defined(RTCONFIG_JFFS2)
#if defined(RTCONFIG_JFFS2LOG) && (defined(RTCONFIG_JFFS2)||defined(RTCONFIG_BRCM_NAND_JFFS2))
	char prefix[PATH_MAX], path1[PATH_MAX], path2[PATH_MAX];

	snprintf(prefix, sizeof(prefix), "%s", nvram_safe_get("log_path"));
	snprintf(path1, sizeof(path1), "%s/syslog.log", prefix);
	snprintf(path2, sizeof(path2), "%s/syslog.log-1", prefix);

	eval("touch", "-c", path1, path2);
	eval("cp", path1, path2, "/tmp");
#endif

	// TODO: make sure is it necessary?
	//time_zone_x_mapping();
	//setenv("TZ", nvram_safe_get("time_zone_x"), 1);

	return _eval(syslogd_argv, NULL, 0, NULL);
}

void
stop_syslogd(void)
{
#if defined(RTCONFIG_JFFS2LOG) && (defined(RTCONFIG_JFFS2)||defined(RTCONFIG_BRCM_NAND_JFFS2))
	int running = pids("syslogd");
#endif

#if defined(RTCONFIG_JFFS2LOG) && (defined(RTCONFIG_JFFS2)||defined(RTCONFIG_BRCM_NAND_JFFS2))
	if (running)
#endif
		killall_tk("syslogd");

#if defined(RTCONFIG_JFFS2LOG) && (defined(RTCONFIG_JFFS2)||defined(RTCONFIG_BRCM_NAND_JFFS2))
	char prefix[PATH_MAX];

	snprintf(prefix, sizeof(prefix), "%s", nvram_safe_get("log_path"));

	if (running){
		eval("cp", "/tmp/syslog.log", "/tmp/syslog.log-1", prefix);
		sync();
	}
#endif
}

void
reload_syslogd(void)
{
	/* notify syslogd */
	if (nvram_invmatch("log_ipaddr", ""))
		kill_pidfile_s("/var/run/syslogd.pid", SIGHUP);
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

void
stop_klogd(void)
{
	if (pids("klogd"))
		killall_tk("klogd");
}
#endif

#ifdef HND_ROUTER
extern int dump_prev_oops(void);
#endif

int
start_logger(void)
{
	start_syslogd();
#ifndef RTCONFIG_RSYSLOGD
	start_klogd();
#endif

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

#ifdef RTCONFIG_BCM_HND_CRASHLOG
	char path[32];
	FILE *fp;
	char line[256];
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)
	char line_empty[256];
#endif
	int count = 0;

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2)
	snprintf(path, sizeof(path), "/jffs/crashlog.log");
#else
	snprintf(path, sizeof(path), "/tmp/crashlog.log");
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)
	memset(line_empty, 0xff , sizeof(line_empty) - 1);
#endif
#endif

	if ((fp = fopen(path, "r")) != NULL) {
		while (fgets(line, sizeof(line), fp) && count < 5) {
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)
			if (!memcmp(line, line_empty, sizeof(line_empty) - 1))
				break;
#endif
			logmessage("crashlog", "%s", line);

			if (!strlen(line))
				count++;
		}

		fclose(fp);

		unlink(path);
	}
#endif

	return 0;
}

void
stop_logger(void)
{
	stop_syslogd();
#ifndef RTCONFIG_RSYSLOGD
	stop_klogd();
#endif
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
		if (nvram_match(strcat_r(prefix, "radio", tmp), "1")
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
			&& (nvram_match(strcat_r(prefix, "igs", tmp), "1") || is_psta(i) || is_psr(i))
#endif
		)
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
	int model = get_model();
	char pxy_ifnv[32];

	/* Start MCPD Proxy in router mode */
	if (is_router_mode()) {
		proxy_ifname = nvram_get("igmp_ifname") ? : wan_if_eth();
	} else {
#if 0
		if (!nvram_match("igmp_enable", "0"))
#endif
		{
			snprintf(pxy_ifnv, sizeof(pxy_ifnv), "wl%d_ifname", nvram_get_int("wlc_band"));
			//_dprintf("get proxy_ifname of nv %s=%s\n", pxy_ifnv, nvram_safe_get(pxy_ifnv));

			/* Start MCPD proxy in AP mode for media router build */
#if defined(RPAX56) || defined(RPAX58)
			if((nvram_match("pxy_wlc", "1") || (dpsr_mode() || dpsta_mode() || rp_mode() || psta_exist() || psr_exist())) && *nvram_safe_get(pxy_ifnv))
				proxy_ifname = nvram_safe_get(pxy_ifnv);
			else
#endif
			proxy_ifname = nvram_get("lan_ifname") ? : "br0";
		}
	}

	if (!(fp = fopen(conf_file, "w+"))) {
		perror(conf_file);
	}

	if (!fp || !proxy_ifname)
		return;

#if defined(RTCONFIG_MULTISERVICE_WAN)
	if (!nvram_match("switch_wantag", "none"))
#endif
{
	/* set mcast interface to eth0.v0 when using vlanctl net interface(IPTV)
	   to enable mcpd bring up							*/
	if (!nvram_match("switch_wantag", "") && nvram_get_int("switch_stb_x") > 0 &&
		nvram_get_int("switch_stb_x") <= 6 && !nvram_match("switch_wantag", "free")) {
		if (model == MODEL_RTAX58U || model == MODEL_RTAX82U_V2 || model == MODEL_TUFAX5400_V2 || model == MODEL_XD6_V2 || model == MODEL_RTAX5400)
			proxy_ifname = "eth4.v0";
		else if (model == MODEL_RTAX82_XD6S)
			proxy_ifname = "eth1.v0";
		else
			proxy_ifname = "eth0.v0";
#if defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(RTBE58U) || defined(TUFBE3600) || defined(GTBE19000) || defined(RTBE92U) || defined(RTBE95U)
		/* Adjust mcast interface for bcm + rtkswtch */
		if (nvram_get_int("switch_wan1tagid")) {
			if(nvram_get_int("switch_stb_x") == 6)
				proxy_ifname = "eth0.v2";
			else
				proxy_ifname = "eth0.v1";
		}
		else if (nvram_match("switch_wantag", "vodafone"))
			proxy_ifname = "eth0.v2";
#endif
	}
}

	/* IGMP configuration */
	fprintf(fp, "##### IGMP configuration #####\n");
	fprintf(fp, "igmp-query-interval 20\n");
	fprintf(fp, "igmp-default-version %d\n", nvram_match("switch_wantag", "centurylink") ? 2 : nvram_get_int("mr_igmp_ver") ? : 3);
	fprintf(fp, "igmp-query-response-interval 100\n");
	fprintf(fp, "igmp-last-member-query-interval 10\n");
	fprintf(fp, "igmp-robustness-value 2\n");
	fprintf(fp, "igmp-max-groups 50\n");
	fprintf(fp, "igmp-max-sources 50\n");
	fprintf(fp, "igmp-max-members 50\n");
	fprintf(fp, "igmp-fast-leave %d\n", nvram_get_int("mr_qleave_x"));
	fprintf(fp, "igmp-admission-required 0\n");
	fprintf(fp, "igmp-admission-bridging-filter 0\n");
#ifdef RTCONFIG_MULTISERVICE_WAN
	if (nvram_match("switch_wantag", "none")) {
		fprintf(fp, "igmp-proxy-interfaces %s\n", is_router_mode() ? proxy_ifname : "");
		fprintf(fp, "igmp-snooping-interfaces %s\n", nvram_safe_get("lan_ifname"));
		fprintf(fp, "igmp-mcast-interfaces %s\n", is_router_mode() ? proxy_ifname : "");
	}
	else
#endif
{
	/* Do not set igmp proxy in IPTV bridge case */
	if (nvram_get_int("switch_stb_x") == 0 || nvram_get_int("switch_stb_x") > 6) {
		if (nvram_match("switch_wantag", "movistar"))
			fprintf(fp, "igmp-proxy-interfaces %s\n", is_router_mode() ? "vlan2" : "");
		else if (!nvram_match("switch_wantag", "") && nvram_get_int("switch_stb_x") == 0 && nvram_get_int("switch_wan0tagid") > 0)
			fprintf(fp, "igmp-proxy-interfaces %s\n", is_router_mode() ? nvram_safe_get("wan0_ifname") : "");
		else
			fprintf(fp, "igmp-proxy-interfaces %s\n", is_router_mode() ? proxy_ifname : "");
	}
	/* set downstream interface to vlan bridge to enable mcast traffic passthrough */
	if (!nvram_match("switch_wantag", "") && nvram_get_int("switch_stb_x") > 0 &&
		nvram_get_int("switch_stb_x") <= 6) {
#if defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(RTBE58U) || defined(TUFBE3600) || defined(GTBE19000) || defined(RTBE92U) || defined(RTBE95U)
		/* Adjust snooping interface for bcm + rtkswtch */
		if (nvram_get_int("switch_wan1tagid")) {
			if (nvram_match("switch_wantag", "unifi_home"))
				fprintf(fp, "igmp-snooping-interfaces %s\n", "br101");
			else if(nvram_get_int("switch_stb_x") == 6) // because different VID use different bridge interface, so we use br101(WAN) as snooping interface
				fprintf(fp, "igmp-snooping-interfaces %s\n", "br101");
			else
				fprintf(fp, "igmp-snooping-interfaces %s\n", "br102");
		}
		else if (nvram_match("switch_wantag", "vodafone"))
			fprintf(fp, "igmp-snooping-interfaces %s\n", "br103");
		else
#endif
		fprintf(fp, "igmp-snooping-interfaces %s\n", "br101");
	}
	else
		fprintf(fp, "igmp-snooping-interfaces %s\n", nvram_safe_get("lan_ifname"));
	if(nvram_match("switch_wantag", "movistar"))
		fprintf(fp, "igmp-mcast-interfaces %s\n", is_router_mode() ? "vlan2" : "");
#if defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(RTBE58U) || defined(TUFBE3600) || defined(GTBE19000) || defined(RTBE92U) || defined(RTBE95U)
	else if (nvram_match("switch_wantag", "unifi_home"))
		fprintf(fp, "igmp-mcast-interfaces %s\n", is_router_mode() ? "eth0.600" : "");
#endif
	else if (!nvram_match("switch_wantag", "") && nvram_get_int("switch_stb_x") == 0 && nvram_get_int("switch_wan0tagid") > 0)
		fprintf(fp, "igmp-mcast-interfaces %s\n", is_router_mode() ? nvram_safe_get("wan0_ifname") : "");
	else if (nvram_match("switch_wantag", "free"))
		fprintf(fp, "igmp-mcast-interfaces %s\n", is_router_mode() ? "eth0.100" : "");
	else
		fprintf(fp, "igmp-mcast-interfaces %s\n", is_router_mode() ? proxy_ifname : "");
}

#ifdef RTCONFIG_IPV6
	fprintf(fp, "#\n");
	fprintf(fp, "#Begin MLD configuration\n");
	fprintf(fp, "#\n");

	fprintf(fp, "mld-default-version %d\n", 2);
	fprintf(fp, "mld-query-interval %d\n", 125);
	fprintf(fp, "mld-query-response-interval %d\n", 10);
	fprintf(fp, "mld-last-member-query-interval %d\n", 10);
	fprintf(fp, "mld-robustness-value %d\n", 2);
	fprintf(fp, "mld-max-groups %d\n", 10);
	fprintf(fp, "mld-max-sources %d\n", 10);
	fprintf(fp, "mld-max-members %d\n", 10);
	fprintf(fp, "mld-fast-leave %d\n", 1);
	fprintf(fp, "mld-admission-required %d\n", 0);
	fprintf(fp, "mld-admission-bridging-filter %d\n", 0);
	fprintf(fp, "mld-proxy-enable %d\n", 1);
	fprintf(fp, "mld-snooping-enable %d\n", 1);
	fprintf(fp, "mld-proxy-interfaces %s\n", is_router_mode() ? proxy_ifname : "");
	fprintf(fp, "mld-snooping-interfaces %s\n", "br0");
	fprintf(fp, "mld-mcast-interfaces %s\n", is_router_mode() ? proxy_ifname : "");
	fprintf(fp, "#\n");
	fprintf(fp, "#End MLD configuration\n");
	fprintf(fp, "#\n");
#endif
	/* Mcast configuration */
	fprintf(fp, "##### MCAST configuration #####\n");
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
#ifdef RTCONFIG_HND_ROUTER_AX
	/* irrespective of NVRAM, try to stop all versions of acsd */
	if (pidof("acsd2") > 0)
		doSystem("rm /tmp/dm/%d", pidof("acsd2"));

	killall_tk("acsd2");
#endif
	killall_tk("acsd");
}

int
start_acsd()
{
	int ret = 0;
#if defined(RTCONFIG_BCM_7114) || defined(RTCONFIG_HND_ROUTER_AX) || defined(GTAC5300)
#if defined(RTCONFIG_HND_ROUTER_AX)
	char *acsd_argv[] = { "/usr/sbin/acsd2", NULL };
#else
	char *acsd_argv[] = { "/usr/sbin/acsd", NULL };
#endif
	int pid;
#endif

#ifdef RTAC68U
	if (is_dpsta_repeater() && !nvram_get_int("x_Setting"))
		return 0;
#endif

#ifdef RTCONFIG_PROXYSTA
	if (psta_exist())
		return 0;
#endif

	stop_acsd();

	if (!restore_defaults_g && strlen(nvram_safe_get("acs_ifnames"))) {
#ifdef RTCONFIG_AVBLCHAN
		reset_exclvalid();
#endif
#if defined(RTCONFIG_BCM_7114)
		ret = _eval(acsd_argv, NULL, 0, &pid);
#else
#if defined(RTCONFIG_HND_ROUTER_AX) || defined(GTAC5300)
#if defined(RTCONFIG_HND_ROUTER_AX)
		/* depending on NVRAM start the respective version */
		if (nvram_match("acs_version", "2"))
#elif defined(GTAC5300)
		if (nvram_get_int("re_mode") == 1)
#endif
			ret = _eval(acsd_argv, NULL, 0, &pid);
		/* default acsd version; to use even when the NVRAM is not set */
		else
#endif
		ret = eval("/usr/sbin/acsd");
#endif
	}

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
	fprintf(fp, "mcast_igmp_fast_leave_status=\"%d\"\n", nvram_get_int("mr_qleave_x"));
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
	fprintf(fp, "mcast_mld_fast_leave_status=\"%d\"\n", nvram_get_int("mr_qleave_x"));
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

#ifdef RTCONFIG_AHS
#define AHS_PIDFILE "/var/run/ahs.pid"
void stop_ahs(void)
{
	if (pids("ahs"))
	{
		killall_tk("ahs");
	}
}

void start_ahs(void)
{
#ifdef RTAC68U
	if (is_dpsta_repeater())
		return;
#endif
	stop_ahs();
	xstart("ahs");
}
#endif /* RTCONFIG_AHS */

#ifdef RTCONFIG_ASD
void stop_asd(void)
{
	if (pids("asd"))
	{
		killall("asd", SIGTERM);
	}
}

void start_asd(void)
{
	stop_asd();
	xstart("asd");
}
#endif /* RTCONFIG_ASD */

#ifdef RTCONFIG_GEARUPPLUGIN
void stop_gu_service(int status)
{
	if (pids("guplugin_monitor.sh"))
		doSystem("killall -9 guplugin_monitor.sh");

	if (pids("guplugin"))
		doSystem("killall -9 guplugin");

	// remove binary when status is GU_BLOCK
	if (status == GU_BLOCK && f_exists("/tmp/gu/guplugin"))
		remove("/tmp/gu/guplugin");
}

void start_gu_service()
{
	int ret;
	int status = gu_enable_status();

	// forbidden case : force to killall gu services
	if (status != GU_ENABLED)
		stop_gu_service(status);

	// exec_gu will check all conditions
	ret = exec_gu(status);
	if (ret) logmessage("GU", "GU is disabled, reason code=%d!", ret);
}
#endif

void
stop_misc(void)
{
	fprintf(stderr, "stop_misc()\n");

#ifdef RTCONFIG_ASD
	stop_asd();
#endif
#ifdef RTCONFIG_CONNTRACK
	stop_pctime_service();
#endif
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
	//stop_check_watchdog();
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

#if defined(CONFIG_BCMWL5) && defined(RTCONFIG_FANCTRL)
	if (pids("phy_tempsense"))
		killall_tk("phy_tempsense");
#endif
#ifdef RTCONFIG_BCMWL6
#ifdef RTCONFIG_PROXYSTA
	if (pids("psta_monitor"))
		killall_tk("psta_monitor");
#endif
#if defined(RTCONFIG_IPERF) || defined(RTCONFIG_IPERF)
	if (pids("monitor"))
		killall_tk("monitor");
#endif
#endif
#ifdef RTCONFIG_QTN
	if (pids("qtn_monitor"))
		killall_tk("qtn_monitor");
#endif
#ifdef RTCONFIG_NTPD
	stop_ntpd();
#else
	if (pids("ntp"))
		killall_tk("ntp");
	if (pids("ntpclient"))
		killall_tk("ntpclient");
#endif

	stop_hotplug2();
#ifdef RTCONFIG_BCMWL6
#ifdef BCM_ASPMD
	stop_aspmd();
#endif
#if defined(RTCONFIG_DHDAP) || defined(RTCONFIG_HND_ROUTER_AX)
	stop_dhd_monitor();
#if defined(RTCONFIG_HND_ROUTER_AX)
	killall_tk("debug_monitor");
#else
	killall_tk("dhd_monitor");
#endif
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
#ifdef BCM_CEVENTD
	stop_ceventd();
#endif
#ifdef BCM_BSD
	stop_bsd();
#endif
	stop_igmp_proxy();
#if defined(BCA_HNDROUTER) && defined(MCPD_PROXY)
	stop_mcpd_proxy();
#endif
#ifdef RTCONFIG_HSPOT
	stop_hspotap();
#endif
#ifdef RTCONFIG_AMAS
	stop_obd();
#ifdef RTCONFIG_ETHOBD
	stop_eth_obd();
#endif
#endif
#endif
#if defined(RTCONFIG_WLCEVENTD)
	stop_wlceventd();
#endif
#if defined(RTCONFIG_HAPDEVENT)
	stop_hapdevent();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
	stop_wlc_monitor();
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
#ifdef RTCONFIG_MDNS
	stop_mdns();
#endif
#if !(defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK) || defined(RTCONFIG_REALTEK)) \
 ||  (defined(RTCONFIG_SOC_IPQ8074))
	stop_erp_monitor();
#endif
#ifdef RTCONFIG_CROND
	stop_cron();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
	stop_notification_center();
#endif
#ifdef RTCONFIG_PROTECTION_SERVER
	stop_ptcsrv();
#endif
#ifdef RTCONFIG_SYSSTATE
	stop_sysstate();
#endif
#ifdef RTCONFIG_AHS
	stop_ahs();
#endif /* RTCONFIG_AHS */
	/* stop_logger(); */	//Do it later in webs_upgrade.sh
#ifdef RTCONFIG_DISK_MONITOR
	stop_diskmon();
#endif
#ifdef RTCONFIG_TUNNEL
	stop_mastiff();
#endif
#ifdef RTCONFIG_NFCM
	stop_nfcm();
#endif
#ifdef RTCONFIG_BWDPI
	stop_bwdpi_check();
#endif
#ifdef RTCONFIG_CFGSYNC
#ifdef RTCONFIG_CONNDIAG
	stop_conn_diag();
#endif
#endif
	stop_pppoe_relay();
	if (pids("igmpproxy"))
		killall_tk("igmpproxy");
#ifndef RTCONFIG_BCM_MFG
	if (!nvram_match(ATE_FACTORY_MODE_STR(), "1") && pids("telnetd"))
		killall_tk("telnetd");
#endif
	if (pids("dropbear"))
		killall_tk("dropbear");
	if (pids("alt_watchdog"))
		killall_tk("alt_watchdog");
#if defined(RTCONFIG_AWSIOT)
	if (pids("awsiot"))
		killall_tk("awsiot");
#endif
#ifdef RTCONFIG_FSMD
	if (pids("fsmd"))
		killall_tk("fsmd");
#endif
#ifdef RTCONFIG_AMAS
	stop_amas_lib();
	stop_amas_bhctrl();
	stop_amas_lanctrl();
	stop_amas_wlcconnect();
#ifdef RTCONFIG_BHCOST_OPT
    stop_amas_ssd();
	stop_amas_status();
	stop_amas_misc();
#endif
#if defined(RTCONFIG_BCMWL6) || defined(RTCONFIG_LANTIQ) || defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK)
	stop_obd();
#endif
#ifdef RTCONFIG_ETHOBD
	stop_eth_obd();
#endif
#if defined(RTCONFIG_CONNDIAG)
	stop_amas_portstatus();
#endif
#endif
#ifdef RTCONFIG_NETOOL
	stop_netool();
#endif

	nvram_set_int("stop_misc", 1);
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
		if(*(user = nvram_default_get("http_username")) == '\0')
			user = "admin";

	if (!pass)
		if(*(pass = nvram_default_get("http_passwd")) == '\0')
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
	char *hostname = get_lan_hostname();

	if (*hostname)
		f_write_string("/proc/sys/kernel/hostname", hostname, 0, 0);
}

int
_start_telnetd(int force)
{
	return 1;
#if 0	// Disable it
	char *telnetd_argv[] = { "telnetd",
		NULL, NULL,	/* -b address */
#if defined(RTCONFIG_BCM_MFG) || defined(RTCONFIG_MFGFW)
		NULL, NULL,	/* -l shell */
#endif
		NULL };
#ifdef RTCONFIG_IPV6
	char *telnetd_ipv6_argv[] = { "telnetd", "-b", NULL, NULL };
	const char *p = NULL;
	char *value;
#endif
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
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled()) {
			p = getifaddr(nvram_safe_get("lan_ifname"), AF_INET6, GIF_PREFIXLEN);
			if (p != NULL) {
				value = strchr(p, '/');
				if (value)
					*value = '\0';

				telnetd_ipv6_argv[2] = (char *) p;
			}
		}
#endif
	}
#ifdef RTCONFIG_BCM_MFG
	telnetd_argv[index++] = "-l";
	telnetd_argv[index++] = "/bin/sh";
#endif
#ifdef RTCONFIG_MFGFW
	if(nvram_match("mfgfw", "1")) {
		telnetd_argv[index++] = "-l";
		telnetd_argv[index++] = "/bin/sh";
	}
#endif
#ifdef RTCONFIG_IPV6
	if (is_routing_enabled() && ipv6_enabled())
		_eval(telnetd_ipv6_argv, NULL, 0, NULL);
#endif
	return _eval(telnetd_argv, NULL, 0, NULL);
#endif	// Disable it
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
	int httpd_index = 1, i = 0;
	char httpd_argv_buf[128] = {0};
#ifdef RTCONFIG_HTTPS
	char *https_argv[] = { "httpds", "-s",
		NULL, NULL,	/* -i ifname */
		NULL, NULL,	/* -p port */
		NULL };
	int https_index = 2;
	int enable;
	char https_argv_buf[128] = {0};
#ifdef RTCONFIG_IPV6
	char *https_ipv6_argv[] = { "httpds", "-s",
		NULL, NULL,	/* -i ifname */
		NULL, NULL,	/* -p port */
		NULL, NULL };
	int https_ipv6_index = 2;
	char https_ipv6_argv_buf[128] = {0};
#endif
#endif
	char *cur_dir;
	pid_t pid;
#ifdef RTCONFIG_UIDEBUG
	char *httpd_dir;
#endif

	if (getpid() != 1) {
		notify_rc("start_httpd");
		return;
	}

	cur_dir = getcwd(NULL, 0);
#ifdef RTCONFIG_UIDEBUG // Left for UI debug
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
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled() && nvram_get_int("misc_http_x")) {
			https_ipv6_argv[https_ipv6_index++] = "-i";
			https_ipv6_argv[https_ipv6_index++] = (char *) get_wan6face();
		}
#endif
#endif
	}

#ifdef RTCONFIG_HTTPS
	prepare_cert_in_etc();

	enable = nvram_get_int("http_enable");
	if (enable != 0) {
		pid = nvram_get_int("https_lanport") ? : 443;
		if (pid != 443) {
			https_argv[https_index++] = "-p";
			https_argv[https_index++] = nvram_safe_get("https_lanport");
		}
		logmessage(LOGNAME, "start https:%d", pid);

		for(i=0;i<https_index;i++){
			if(i!=0)
				strlcat(https_argv_buf, " ",sizeof(https_argv_buf));
			strlcat(https_argv_buf, https_argv[i],sizeof(https_argv_buf));
		}

		if (get_pid_by_process_name(https_argv_buf) == -1) {
			if (nvram_get("httpds_reload_cert")) {
				nvram_unset("httpds_reload_cert");
			}
			_eval(https_argv, NULL, 0, &pid);
		}

#ifdef RTCONFIG_IPV6
		if (ipv6_enabled() && nvram_get_int("misc_http_x")
			&& is_intf_up(https_ipv6_argv[3]) > 0) {
			pid = nvram_get_int("misc_httpsport_x") ? : 8443;
			https_ipv6_argv[https_ipv6_index++] = "-p";
			https_ipv6_argv[https_ipv6_index++] = ((pid == 8443) ? "8443" : nvram_safe_get("misc_httpsport_x"));
			https_ipv6_argv[https_ipv6_index++] = "-6";
			logmessage(LOGNAME, "start https:%d", pid);

			for(i=0;i<https_ipv6_index;i++){
				if(i!=0)
					strlcat(https_ipv6_argv_buf, " ",sizeof(https_ipv6_argv_buf));
				strlcat(https_ipv6_argv_buf, https_ipv6_argv[i],sizeof(https_ipv6_argv_buf));
			}

			if (get_pid_by_process_name(https_ipv6_argv_buf) == -1) {
				if (nvram_get("httpds6_reload_cert"))
					nvram_unset("httpds6_reload_cert");
				_eval(https_ipv6_argv, NULL, 0, &pid);
			}
		}
#endif
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

		for(i=0;i<httpd_index;i++){
			if(i!=0)
				strlcat(httpd_argv_buf, " ",sizeof(httpd_argv_buf));
			strlcat(httpd_argv_buf, httpd_argv[i],sizeof(httpd_argv_buf));
		}

		if(get_pid_by_process_name(httpd_argv_buf) == -1)
			_eval(httpd_argv, NULL, 0, &pid);
#if defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
		sleep(1);
#endif
	}

	chdir(cur_dir ? : "/");
	free(cur_dir);

	nvram_set_int("wait_httpd", 0);
}

void
stop_httpd(void)
{
	if (getpid() != 1) {
		notify_rc("stop_httpd");
		return;
	}

	nvram_set_int("wait_httpd", 1);

	if (pids("httpd"))
		killall_tk("httpd");
#ifdef RTCONFIG_HTTPS
	if (pids("httpds"))
		killall_tk("httpds");
#endif
}

#if defined(RTCONFIG_HTTPS) && defined(RTCONFIG_IPV6)
void
start_httpd_ipv6(void)
{
	char *https_ipv6_argv[] = { "httpds", "-s",
		NULL, NULL,	/* -i ifname */
		NULL, NULL,	/* -p port */
		NULL, NULL };
	int https_ipv6_index = 2;
	int enable;
	char *cur_dir;
	pid_t pid;
	char pidfile[32], pif[IFNAMSIZ];

	strlcpy(pif, get_wan6face(), sizeof(pif));
	pid = nvram_get_int("misc_httpsport_x") ? : 8443;
	snprintf(pidfile, sizeof(pidfile), "/var/run/httpd-%s-%d.pid", pif, pid);
	if (f_exists(pidfile)) {
		kill_pidfile_s(pidfile, SIGTERM);
		unlink(pidfile);
	}

	if (!nvram_get_int("misc_http_x"))
		return;

	cur_dir = getcwd(NULL, 0);
	chdir("/www");

	enable = nvram_get_int("http_enable");
	if (enable != 0) {
		https_ipv6_argv[https_ipv6_index++] = "-i";
		https_ipv6_argv[https_ipv6_index++] = pif;
		https_ipv6_argv[https_ipv6_index++] = "-p";
		https_ipv6_argv[https_ipv6_index++] = ((pid == 8443) ? "8443" : nvram_safe_get("misc_httpsport_x"));
		https_ipv6_argv[https_ipv6_index++] = "-6";
		logmessage(LOGNAME, "start https:%d", pid);
		_eval(https_ipv6_argv, NULL, 0, &pid);
#if defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
		sleep(1);
#endif
	}

	chdir(cur_dir ? : "/");
	free(cur_dir);
}
#endif

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
	unsigned char ea[ETHER_ADDR_LEN];
	char serial[18], uuid[37];
	char *proto, *port, *lport, *srcip, *dstip, *desc;
	char *nv, *nvp, *b;
	int upnp_mnp_enable, upnp_port;
	int unit, httpx_port, cnt;
#if defined(RTCONFIG_APP_PREINSTALLED) || defined(RTCONFIG_APP_NETINSTALLED)
	FILE *ifp = NULL;
	char tmpstr[80];
	int statDownloadMaster = 0;
#endif
	int min_lifetime, max_lifetime;
	int run_upnpd = 0;
#if defined(RTCONFIG_AURASYNC)
	char *aura_mode = "no";
#endif

#if defined(RTCONFIG_GEFORCENOW)
	char *gfn_mode = "no";
#endif

	if (getpid() != 1 && getuid() != 0) {
		notify_rc("start_upnp");
		return;
	}

	if (!is_routing_enabled())
		return;

	unit = wan_primary_ifunit();
#ifdef RTCONFIG_DUALWAN
	if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_NONE || !is_wan_connect(unit)) {
		int i;
		for (i = WAN_UNIT_FIRST; i < WAN_UNIT_MAX; i++) {
			if (get_dualwan_by_unit(i) == WANS_DUALWAN_IF_NONE || !is_wan_connect(i))
				continue;
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if (nvram_match(strcat_r(prefix, "proto", tmp), "static")) {
				snprintf(tmp, sizeof(tmp), i ? "link_wan%d" : "link_wan", i);
				if (!nvram_get_int(tmp))
					continue;
			}
			if (nvram_get_int(strcat_r(prefix, "upnp_enable", tmp))) {
				unit = i;
				break;
			}
		}
	}
#endif
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	upnp_mnp_enable = nvram_get_int("upnp_mnp");

	if (nvram_get_int(strcat_r(prefix, "upnp_enable", tmp)))
		run_upnpd = 1;
#if defined(RTCONFIG_AURASYNC)
	if (run_upnpd) {
		if (nvram_get_int("aurasync_enable"))
			aura_mode = "yes";
		else
			aura_mode = "no";
	} else {
		if (nvram_get_int("aurasync_enable")) {
			aura_mode = "standalone";
			run_upnpd = 1;
		}
	}
#endif

#if defined(RTCONFIG_GEFORCENOW)
	if (run_upnpd) {
		if (nvram_get_int("nvgfn_enable"))
			gfn_mode = "yes";
		else
			gfn_mode = "no";
	} else {
		if (nvram_get_int("nvgfn_enable")) {
			gfn_mode = "gfn_only";
			run_upnpd = 1;
			upnp_mnp_enable = 0;
		}
	}
#endif

	if ( run_upnpd ) {
		mkdir("/etc/upnp", 0777);
		if (f_exists("/etc/upnp/config.alt")) {
			xstart("miniupnpd", "-f", "/etc/upnp/config.alt");
#ifdef RTCONFIG_AUPNPC
			start_aupnpc();
#endif
		} else {
			if ((f = fopen("/etc/upnp/config", "w")) != NULL) {
				char *lanip = nvram_safe_get("lan_ipaddr");
				char *lanmask = nvram_safe_get("lan_netmask");

				upnp_port = nvram_get_int("upnp_port");
				if (upnp_port < 0 || upnp_port > 65535)
					upnp_port = 0;

				if (!ether_atoe(get_lan_hwaddr(), ea))
					f_read("/dev/urandom", ea, sizeof(ea));
				snprintf(serial, sizeof(serial), "%02x:%02x:%02x:%02x:%02x:%02x",
					 ea[0], ea[1], ea[2], ea[3], ea[4], ea[5]);
				snprintf(uuid, sizeof(uuid), "3ddcd1d3-2380-45f5-b069-%02x%02x%02x%02x%02x%02x",
					 ea[0], ea[1], ea[2], ea[3], ea[4], ea[5]);
#ifdef RTCONFIG_DUALWAN
				nvram_set_int("upnp_unit", unit);
				nvram_set("upnp_ifname", get_wan_ifname(unit));
#endif
				fprintf(f,
					"ext_ifname=%s\n"
					"listening_ip=%s\n"
					"port=%d\n"
					"enable_upnp=%s\n"
					"enable_natpmp=%s\n"
#if defined(RTCONFIG_AURASYNC)
					"enable_aurasync=%s\n"
#endif
#if defined(RTCONFIG_GEFORCENOW)
					"enable_nvgfn=%s\n"
#endif
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
					"uuid=%s\n"
#ifdef RTCONFIG_IGD2
					"lease_file6=/tmp/upnp.leases6\n"
#endif
					"lease_file=%s\n",
					get_wan_ifname(unit),
					nvram_safe_get("lan_ifname"),
					upnp_port,
					run_upnpd ? "yes" : "no",	// upnp enable
					upnp_mnp_enable ? "yes" : "no",	// natpmp enable
#if defined(RTCONFIG_AURASYNC)
					aura_mode, // yes, no, standalone
#endif
#if defined(RTCONFIG_GEFORCENOW)
					gfn_mode,
#endif
					nvram_get_int("upnp_secure") ? "yes" : "no",	// secure_mode (only forward to self)
					nvram_get_int("upnp_ssdp_interval"),
					get_lan_hostname(),
					"ASUS Wireless Router",
					"ASUS Wireless Router",
					get_productid(),
					nvram_get("serial_no") ? : serial, uuid,
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

				if (is_nat_enabled() && nvram_match("vts_enable_x", "1")) {
					nvp = nv = strdup(nvram_safe_get("vts_rulelist"));
					while (nv && (b = strsep(&nvp, "<")) != NULL) {
						char *portv, *portp, *c, *cptr;

						if ((cnt = vstrsep(b, ">", &desc, &port, &dstip, &lport, &proto, &srcip)) < 5)
							continue;
						else if (cnt < 6)
							srcip = "";

						// Handle port1,port2,port3 format
						portp = portv = strdup(port);
						while (portv && (c = strsep(&portp, ",")) != NULL) {
							cptr = c;
							while (*cptr) {
							if (*cptr == ':')
								*cptr = '-';
								cptr++;
							}
							if (strcmp(proto, "TCP") == 0 || strcmp(proto, "BOTH") == 0)
								fprintf(f, "deny %s 0.0.0.0/0 0-65535\n", c);
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
#endif
#if 0
					if (enable != 1)

					{
						httpx_port = nvram_get_int("misc_httpport_x") ? : 8080;
						fprintf(f, "deny %d 0.0.0.0/0 0-65535\n", httpx_port);
					}
#endif
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
#ifdef RTCONFIG_TUNNEL
				if((nvram_get_int("aae_enable") & 1) == 1) {
					fprintf(f, "deny 61689 0.0.0.0/0 0-65535\n");	// MASTIFF_DEF_PORT
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

				/* Provide real IP when in dual NAT situation */
#ifdef RTCONFIG_GETREALIP
				strcat_r(prefix, "realip_state", tmp);
				if (nvram_get_int(tmp) != 2)
					sleep(5);
				if (nvram_get_int(tmp) == 2)
					fprintf(f, "ext_ip=%s\n", nvram_safe_get(strcat_r(prefix, "realip_ip", tmp)));
#endif

				fappend(f, "/etc/upnp/config.custom");
				append_custom_config("upnp", f);

				fclose(f);
				use_custom_config("upnp", "/etc/upnp/config");
				run_postconf("upnp", "/etc/upnp/config");
#ifdef RTCONFIG_IGD2
				if (!nvram_get_int("upnp_pinhole_enable"))
					xstart("miniupnpd", "-f", "/etc/upnp/config", "-1");
				else
#endif
					xstart("miniupnpd", "-f", "/etc/upnp/config");

#ifdef RTCONFIG_AUPNPC
				start_aupnpc();
#endif
			}
		}
	}
}

void stop_upnp(void)
{
	if (getpid() != 1 && getuid() != 0) {
		notify_rc("stop_upnp");
		return;
	}

	killall_tk("miniupnpd");
#ifdef RTCONFIG_AUPNPC
	stop_aupnpc();
#endif
}

void reload_upnp(void)
{
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	int unit;

	if (!is_routing_enabled())
		return;

	unit = wan_primary_ifunit();
#ifdef RTCONFIG_DUALWAN
	if (nvram_match("wans_mode", "lb") && !is_phy_connect(unit))
		unit = 1 - unit;
#endif
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	if (nvram_get_int(strcat_r(prefix, "upnp_enable", tmp)))
		killall("miniupnpd", SIGUSR1);
}

int
start_ntpc(void)
{
#ifndef RTCONFIG_NTPD
	char *ntp_argv[] = {"ntp", NULL};
	int pid;
#endif
	int unit = wan_primary_ifunit();

	if(dualwan_unit__usbif(unit) && nvram_get_int("modem_pdp") == 2)
		return 0;

#ifdef RTCONFIG_NTPD
	start_ntpd();
#else
	if (!pids("ntp"))
		_eval(ntp_argv, NULL, 0, &pid);
#endif

	return 0;
}

void
stop_ntpc(void)
{
#ifdef RTCONFIG_NTPD
	stop_ntpd();
#else
	if (pids("ntpclient"))
		killall_tk("ntpclient");
#if defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2)
	if (pids("ntpdate"))
		killall_tk("ntpdate");
#endif
#endif
}

void refresh_ntpc(void)
{
	setup_timezone();

#ifdef RTCONFIG_NTPD
	/* TODO: refresh ntpd with signal */
	stop_ntpd();
	start_ntpd();
#else
	stop_ntpc();

	if (!pids("ntp"))
		start_ntpc();
	else
		kill_pidfile_s("/var/run/ntp.pid", SIGALRM);
#endif
}

#ifdef RTCONFIG_BCMARM
int write_lltd_conf(void)
{
	FILE *fp;

	if (!(fp = fopen("/tmp/lld2d.conf", "w"))) {
		perror("/tmp/lld2d.conf");
		return -1;
	}

#if defined(RTAX82U) || defined(RTAX88U) || defined(GTAX11000) || defined(RTAX82U_V2)
	if (!strncmp(nvram_safe_get("territory_code"), "GD", 2))
	{
		doSystem("cp /usr/sbin/icon_gd.ico /tmp/icon.ico");
		doSystem("cp /usr/sbin/icon_gd.large.ico /tmp/icon.large.ico");
	}
	else
	{
		doSystem("cp /usr/sbin/icon_default.ico /tmp/icon.ico");
		doSystem("cp /usr/sbin/icon_default.large.ico /tmp/icon.large.ico");
	}
#elif defined(GTAX6000)
	if (!strncmp(nvram_safe_get("CoBrand"), "3", 1))
	{
		doSystem("cp /usr/sbin/icon_eva.ico /tmp/icon.ico");
		doSystem("cp /usr/sbin/icon_eva.large.ico /tmp/icon.large.ico");
	}
	else
	{
		doSystem("cp /usr/sbin/icon_default.ico /tmp/icon.ico");
		doSystem("cp /usr/sbin/icon_default.large.ico /tmp/icon.large.ico");
	}
#else 
	fprintf(fp, "icon = /usr/sbin/icon.ico\n");
	fprintf(fp, "jumbo-icon = /usr/sbin/icon.large.ico\n");
#endif

	fclose(fp);

#ifdef RTAC68U
	if (is_dpsta_repeater()) {
		doSystem("cp /usr/sbin/lltd/icon_alt2.ico /tmp/icon.ico");
                doSystem("cp /usr/sbin/lltd/icon_alt2.large.ico /tmp/icon.large.ico");
	} else {
		doSystem("cp /usr/sbin/lltd/icon%s.ico /tmp/icon.ico", is_ac66u_v2_series() ? "_alt" : "");
		doSystem("cp /usr/sbin/lltd/icon%S.large.ico /tmp/icon.large.ico", is_ac66u_v2_series() ? "_alt" : "");
	}
#endif

	return 0;
}
#elif defined(RTCONFIG_MT798X)
int write_lltd_conf(void)
{
	char *model = NULL;
	FILE *fp;

	unlink("/etc/lld2d.conf");
	if (!(fp = fopen("/etc/lld2d.conf", "w"))) {
		perror("/etc/lld2d.conf");
		return -1;
	}

#ifdef TUFAX4200
	if (nvram_match("HwId", "B")) {	/* TUF-AX4200Q, CN sku */
		model = "TUF-AX4200Q";
	}
#endif

        fprintf(fp, "icon = /rom/etc/icon%s%s.ico\n", model? "." : "", model? model : "");
        fprintf(fp, "jumbo-icon = /rom/etc/icon.large%s%s.ico\n", model? "." : "", model? model : "");
	fclose(fp);
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
#if defined(RTCONFIG_BCMARM) || defined(RTCONFIG_MT798X)
		write_lltd_conf();
/* TODO: clarify do we need to use lan_hostname instead of productid here */
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
#if defined(RTCONFIG_ALEXA)
#define AVAHI_ALEXA_SERVICE_FN  "alexa.service"
#endif
#define TIMEMACHINE_BACKUP_NAME	"Backups.backupdb"

int generate_mdns_config(void)
{
	FILE *fp;
	char avahi_config[80];
	int ret = 0;
	char *wan1_ifname;

	sprintf(avahi_config, "%s/%s", AVAHI_CONFIG_PATH, AVAHI_CONFIG_FN);

	/* Generate avahi configuration file */
	if (!(fp = fopen(avahi_config, "w"))) {
		perror(avahi_config);
		return -1;
	}

	/* Set [server] configuration */
	fprintf(fp, "[Server]\n");

	fprintf(fp, "host-name=%s\n", get_lan_hostname());
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
	char *servername;

	sprintf(itune_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_ITUNE_SERVICE_FN);

	/* Generate afpd service configuration file */
	if (!(fp = fopen(itune_service_config, "w"))) {
		perror(itune_service_config);
		return -1;
	}

	servername = nvram_safe_get("daapd_friendly_name");
	if (*servername == '\0' || !is_valid_hostname(servername))
		servername = get_lan_hostname();

	fprintf(fp, "<service-group>\n");
	fprintf(fp, "<name replace-wildcards=\"yes\">%s</name>\n", servername);
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

#if defined(RTCONFIG_ALEXA)
int generate_alexa_config(void)
{
	FILE *fp;
	char alexa_service_config[80];
	int ret = 0;

	sprintf(alexa_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_ALEXA_SERVICE_FN);

	/* Generate afpd service configuration file */
	if (!(fp = fopen(alexa_service_config, "w"))) {
		perror(alexa_service_config);
		return -1;
	}

	fprintf(fp, "<service-group>\n");
	fprintf(fp, "<name replace-wildcards=\"yes\">%%h</name>\n");
	fprintf(fp, "<service>\n");
	fprintf(fp, "<type>_alexa._tcp</type>\n");
	fprintf(fp, "<port>80</port>\n");
	fprintf(fp, "<txt-record>skillSetupId=8b18386c-1353-4612-9626-714937decf3e</txt-record>\n");
	fprintf(fp, "<txt-record>version=1</txt-record>\n");
	fprintf(fp, "</service>\n");
	fprintf(fp, "</service-group>\n");

	fclose(fp);

	return ret;
}
#endif

int start_mdns(void)
{
	char afpd_service_config[80];
	char adisk_service_config[80];
	char itune_service_config[80];
#if defined(RTCONFIG_ALEXA)
	char alexa_service_config[80];
#endif
	char *avahi_daemon_argv[] = { "avahi-daemon",
		"-D",
		nvram_get_int("ava_verb") ? "--debug" : NULL,
		NULL
	};
	pid_t pid;

#ifdef RTAC68U
	if (!hw_usb_cap())
		return 0;
#endif

	sprintf(afpd_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_AFPD_SERVICE_FN);
	sprintf(adisk_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_ADISK_SERVICE_FN);
	sprintf(itune_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_ITUNE_SERVICE_FN);
#if defined(RTCONFIG_ALEXA)
	sprintf(alexa_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_ALEXA_SERVICE_FN);
#endif
	mkdir_if_none(AVAHI_CONFIG_PATH);
	mkdir_if_none(AVAHI_SERVICES_PATH);

	generate_mdns_config();
#if defined(RTCONFIG_ALEXA)
	generate_alexa_config();
#endif

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

	return _eval(avahi_daemon_argv, NULL, 0, &pid);
}

void stop_mdns(void)
{
	int i;
	for(i = 0; i < 2 ;i++){
		if (pids("avahi-daemon")){
			cprintf("%s: [%d] killall-avahi-daemon\n",__FUNCTION__,i+1);
			killall("avahi-daemon", SIGTERM);
		}else{
			cprintf("%s: no avahi-daemon\n",__FUNCTION__);
			break;
		}
		sleep(2);
	}

	if( i == 2 && pids("avahi-daemon") ){
		cprintf("%s: kill -9 avahi-daemon\n",__FUNCTION__);
		system("kill -9 `pidof avahi-daemon`");
	}

}

void restart_mdns(void)
{
	char afpd_service_config[80];
	char itune_service_config[80];
	sprintf(afpd_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_AFPD_SERVICE_FN);
	sprintf(itune_service_config, "%s/%s", AVAHI_SERVICES_PATH, AVAHI_ITUNE_SERVICE_FN);

	if (g_reboot || g_upgrade)
		return;

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

#ifdef RTCONFIG_IPERF3
void
stop_iperf3(void)
{
	if (pids("iperf3"))
		killall_tk("iperf3");

	return;
}

void
start_iperf3_server(void)
{
	char *iperf3_argv[] = {"iperf3", "-s", "-p", "5201", NULL};
	char iperf3_svr_port[8];
	pid_t pid;
	
	if(nvram_get_int("iperf3_svr_port") != 0) {
		strcpy(iperf3_svr_port, nvram_safe_get("iperf3_svr_port"));
		iperf3_argv[3] = iperf3_svr_port;
	}

	_eval(iperf3_argv, NULL, 0, &pid);

	return;
}

void
start_iperf3_client(void)
{
	char *iperf3_argv[] = {"iperf3","-c","192.168.50.218","-p","5201","-t","10","-P","2","-w","29200","-O","1","-l","131072","-i","2","-R", NULL};
	char host[18], port[8], time[8], parallel[8], window[16], omit[8], buf_len[8], interval[8];
	pid_t pid;

	strcpy(host, nvram_safe_get("iperf3_cli_host"));
	iperf3_argv[2] = host;
	strcpy(port, nvram_safe_get("iperf3_cli_port"));
	iperf3_argv[4] = port;
	strcpy(time, nvram_safe_get("iperf3_cli_time"));
	iperf3_argv[6] = time;
	strcpy(parallel, nvram_safe_get("iperf3_cli_parallel"));
	iperf3_argv[8] = parallel;
	strcpy(window, nvram_safe_get("iperf3_cli_window"));
	iperf3_argv[10] = window;
	strcpy(omit, nvram_safe_get("iperf3_cli_omit"));
	iperf3_argv[12] = omit;
	strcpy(buf_len, nvram_safe_get("iperf3_cli_buf_len"));
	iperf3_argv[14] = buf_len;
	strcpy(interval, nvram_safe_get("iperf3_cli_interval"));
	iperf3_argv[16] = interval;
	
	if(nvram_match("iperf3_cli_reverse", "0"))
		iperf3_argv[17] = NULL;

	_eval(iperf3_argv, NULL, 0, &pid);

	return;
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

#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
int
stop_plchost(void)
{
	nvram_set("plchost_active", "0");
	if (pids("plchost"))
		killall_tk("plchost");
	return 0;
}

int
start_plchost(void)
{
	char plc_ifname[16];
	char *plchost_argv[] = {"/usr/local/bin/plchost", "-i", plc_ifname
#ifdef RTCONFIG_QCA_PLC2
		, "-A", nvram_safe_get("lan_ifname")
#endif
		, "-N", BOOT_NVM_PATH, "-P", BOOT_PIB_PATH, NULL};
	pid_t pid;
	int ret;

	nvram_set("plc_wake", "1");
	get_plc_ifname(plc_ifname);
	_dprintf("[PLC]run plchost on:%s\n", plc_ifname);
#ifdef PLC_INTERFACE
	_ifconfig(PLC_INTERFACE, IFUP, NULL, NULL, NULL, 0);
#endif
	do_plc_reset(1);	//force reset when start plchost
	ret = _eval(plchost_argv, ">/dev/console" /*NULL*/, 0, &pid);
	nvram_set("plchost_active", "1");
	return ret;
}

void
reset_plc(int ending)
{
	FILE *fp;
	int rlen;
	int cnt = 0;

	if (nvram_invmatch("plc_ready", "1"))
		return;

#if defined(PLN12)
	if (!get_qca8337_PHY_power(1))
		doSystem("swconfig dev %s port 1 set power 1", MII_IFNAME);
#elif defined(PLAC56)
	int wake_gpio = nvram_get_int("plc_wake_gpio") & 0xff;

	if (get_gpio(wake_gpio))
		set_gpio(wake_gpio, 0);
#endif

	while (1) {
		if (chk_plc_alive())
			break;

		dbg("%s: wait Powerline wake up...\n", __func__);
		if (cnt++ > 3)
			break;
		sleep(1);
	}

	if(ending)
	{
		extern int stop_detect_plc(void);
		stop_detect_plc();
	}
	stop_plchost();
	do_plc_reset(0);
}

int 
restart_plc_main(int argc, char *argv[])
{
	reset_plc(1);
	start_plchost();
	start_detect_plc();
	return 0;
}


int
stop_detect_plc(void)
{
	if (pids("detect_plc"))
		killall_tk("detect_plc");
	return 0;
}

int
start_detect_plc(void)
{
	char *detect_plc_argv[] = {"detect_plc", NULL};
	pid_t pid;

	stop_detect_plc();
	return _eval(detect_plc_argv, NULL, 0, &pid);
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
#define UAM_JS_SCRIPT	"/usr/lighttpd/js/jquery.js"
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
	fprintf(fp, "   $HTTP[\"url\"] =~ \"^/jquery.js($|/)\"{\n");
	fprintf(fp, "       alias.url = ( \"/jquery.js\" => \"%s/jquery.js\" ) \n", UAM_WEB_DIR);
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

#if defined(RTAX56_XD4) || defined(PLAX56_XP4)
	if(nvram_match("HwId", "B") || nvram_match("HwId", "D")){
		/* Slave, no bluetooth */
		return;
	}
#endif
#if defined(RTAX56_XD4)
	if((nvram_match("HwId", "A") && nvram_get_int("BLE_BT") == 99) ||
		(nvram_match("HwId", "C") && nvram_get_int("BLE_BT") == 99)){
		/* Master without BT */
		return;
	}
#endif
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
#if defined(RTAX56_XD4) || defined(PLAX56_XP4)
	if(nvram_match("HwId", "B") || nvram_match("HwId", "D")){
		/* Slave, no bluetooth */
		return;
	}
#endif
#if defined(RTAX56_XD4)
	if((nvram_match("HwId", "A") && nvram_get_int("BLE_BT") == 99) ||
		(nvram_match("HwId", "C") && nvram_get_int("BLE_BT") == 99)){
		/* Master without BT */
		return;
	}
#endif
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
#if defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_HND_ROUTER_AX_6756)
	int ret = -1;
#else
	int ret = 0;
#endif

	if(bt_dev == NULL || strlen(bt_dev) < 4)
		return ret;

	if((fd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI)) >= 0)
	{
		memset(&di, 0, sizeof(di));
		di.dev_id = safe_atoi(bt_dev + 3);	// hci interface: hci0

		if (ioctl(fd, HCIGETDEVINFO, (void *) &di) == 0)
		{
#if defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_HND_ROUTER_AX_6756)
			ret = 0;
#else
			ret = 1;
#endif
		}
	}
	close(fd);

	return ret;
}

#if (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2)) && !defined(RTCONFIG_BCM_MFG)
static inline int hci_test_bit(int nr, void *addr)
{
	return *((uint32_t *) addr + (nr >> 5)) & (1 << (nr & 31));
}

#define HCI_UP		(1 << 0)

int check_bluetooth_device_up(const char *bt_dev)
{
	struct hci_dev_info di;
	int fd;
	int ret = 0;

	if(bt_dev == NULL || strlen(bt_dev) < 4)
		return ret;

	if((fd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI)) >= 0)
	{
		memset(&di, 0, sizeof(di));
		di.dev_id = safe_atoi(bt_dev + 3);	// hci interface: hci0

		if (ioctl(fd, HCIGETDEVINFO, (void *) &di) == 0)
		{
			if (hci_test_bit(HCI_UP, &di.flags))
				ret = 1;
		}
	}
	close(fd);

	return ret;
}
#endif

int isValidMac(unsigned char mac[]) {
	if (mac[0] == 0xff
	 ||(mac[0] == 0 && mac[1] == 0 && mac[2] == 0 && mac[3] == 0 && mac[4] == 0 && mac[5] == 0)
	   )
		return 0;

	/* valid mac */
	return 1;
}

#if defined(RTCONFIG_QCA)
int change_bt_mac(void)
{
	unsigned char bt_mac[6], mac2[6], mac5[6];
	unsigned char mac5_2[6];
	int plus = 0;
	int ret = 0;

	extern int get_mac_2g(unsigned char dst[]);
	extern int get_mac_5g(unsigned char dst[]);

	memset(mac2, 0, sizeof(mac2));
	if(get_mac_2g(mac2) < 0 || isValidMac(mac2) != 1) {
		cprintf("invalid mac2(%02x:%02x:%02x:%02x:%02x:%02x)\n", mac2[0], mac2[1], mac2[2], mac2[3], mac2[4], mac2[5]);
		return ret;
	}

	memset(mac5, 0, sizeof(mac5));
	if(get_mac_5g(mac5) < 0 || isValidMac(mac5) != 1) {
		cprintf("invalid mac5(%02x:%02x:%02x:%02x:%02x:%02x)\n", mac5[0], mac5[1], mac5[2], mac5[3], mac5[4], mac5[5]);
		return ret;
	}

	if(mac5[5] == mac2[5] +2)	// final version
	{
#if defined(MAPAC2200) || defined(RTAC95U)
		plus = 5;
#elif defined(MAPAC1300) || defined(VZWAC1300) || defined(SHAC1300)
		plus = 4;
#elif defined(MAPAC1750)
		plus = 3;
#endif
	}
	else if(mac5[5] == mac2[5] +4)	// need to check type
	{
#if defined(MAPAC2200) || defined(RTAC95U)
		extern int get_mac_5g_2(unsigned char dst[]);

		memset(mac5_2, 0, sizeof(mac5_2));
		if(get_mac_5g_2(mac5_2) < 0 || isValidMac(mac5_2) != 1) {
			cprintf("invalid mac5_2(%02x:%02x:%02x:%02x:%02x:%02x)\n", mac5_2[0], mac5_2[1], mac5_2[2], mac5_2[3], mac5_2[4], mac5_2[5]);
			return ret;
		}
		if(mac5_2[5] == mac5[5] +4)
			plus = 0xc;	// 1st version
		else if(mac5_2[5] < mac5[5])
			plus = 5;	// wrong high/low band version
#elif defined(MAPAC1300) || defined(VZWAC1300) || defined(SHAC1300)
		plus = 8;
#endif
	}
#if defined(MAPAC2200) || defined(RTAC95U)
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
		if (system("hciconfig hci0 up"))
			ret = -1;
		if (doSystem("btconfig wba %s", mac_str))
			ret = -1;
	}
	else
	{
		logmessage("BLE", "## mac2(%02x:%02x:%02x:%02x:%02x:%02x)\n", mac2[0], mac2[1], mac2[2], mac2[3], mac2[4], mac2[5]);
		logmessage("BLE", "## mac5(%02x:%02x:%02x:%02x:%02x:%02x)\n", mac5[0], mac5[1], mac5[2], mac5[3], mac5[4], mac5[5]);
		logmessage("BLE", "## mac5_2(%02x:%02x:%02x:%02x:%02x:%02x)\n", mac5_2[0], mac5_2[1], mac5_2[2], mac5_2[3], mac5_2[4], mac5_2[5]);
	}
	return ret;
}
#endif

static void _get_ble_name(char* name, size_t len)
{
	char *macp = NULL;
	char tmp[10];
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
	unsigned char buf[CFGSYNC_GROUPID_LEN+1];
#else
	unsigned char buf[16+1];
#endif
	int amas_bdl_val;

	memset(buf, '\0', sizeof(buf));
	macp = get_2g_hwaddr();
	ether_atoe(macp, buf);

	/* ASUS SSID */
	memset(name, '\0', len);
#if defined(RTCONFIG_REALTEK)
	snprintf(name, len, "ASUS_%02X_%s", buf[5], nvram_safe_get("productid"));
#elif defined(RTCONFIG_SSID_AMAPS)
	snprintf(name, len, "ASUS_%02X_AMAPS", buf[5]);
#else
	snprintf(name, len, get_default_ssid(0, 0));
#endif

#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
	/*  */
	strlcat(name, ".", len);

	/* UI flag */
	snprintf(tmp, sizeof(tmp), "%s", nvram_default_get("prelink_ui_flag")?nvram_default_get("prelink_ui_flag"):"0");
	tmp[1] = '\0';
	strlcat(name, tmp, len);

	/* WAN state */
	snprintf(tmp, sizeof(tmp), "%d", nvram_get_int("amas_bdl_wanstate")?nvram_get_int("amas_bdl_wanstate"):0);
	tmp[1] = '\0';
	strlcat(name, tmp, len);

	/* Bundle flag */
#if defined(RTAX95Q) || defined(XT8PRO) || defined(BT12) || defined(BT10) || defined(BQ16) || defined(BQ16_PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(ET12) || defined(XT12) || defined(XD6_V2) || defined(XC5)
	amas_bdl_val = strtol(cfe_nvram_safe_get_raw("amas_bdl"), NULL, 10);
	if (amas_bdl_val == 0)
		amas_bdl_val = 1; /* single pack case */
	snprintf(tmp, sizeof(tmp), "%X", amas_bdl_val);
#else
	memset(tmp, '\0', sizeof(tmp));
	memset(buf, '\0', sizeof(buf));
	FRead(buf, OFFSET_AMAS_BUNDLE_FLAG, CKN_STR1);
	buf[1] = '\0';
	if (buf[0]==0xff || buf==NULL) {
		/* single pack case */
		buf[0] = 1;
	}
	amas_bdl_val = buf[0];
	snprintf(tmp, sizeof(tmp), "%X", buf[0]);
#endif
	strlcat(name, tmp, len);

	/* Bundle key */
	memset(tmp, '\0', sizeof(tmp));
	if (amas_bdl_val == 1) {
		snprintf(tmp, sizeof(tmp), "XXXX");
	} else {
		memset(buf, '\0', sizeof(buf));
		amas_get_default_hash_bundle_key(buf, CFGSYNC_GROUPID_LEN);
		snprintf(tmp, sizeof(tmp), "%02X%02X", buf[CFGSYNC_GROUPID_LEN-2], buf[CFGSYNC_GROUPID_LEN-1]);
	}
	strlcat(name, tmp, len);
#endif
}

static void _write_bluetoothd_conf()
{
	char hci_name[32] = {0};
	FILE *fp = NULL;
	int dualmode = 0;
	int model = get_model();

	switch (model) {
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
		case MODEL_RTAX58U:
		case MODEL_RTAX82_XD6S:
		case MODEL_XD6_V2:
		case MODEL_ET12:
		case MODEL_XT12:
		case MODEL_BC109:
		case MODEL_EBG19:
		case MODEL_BC105:
		case MODEL_EBG15:
			dualmode = 1;
	}

	unlink("/etc/bluetooth");
	mkdir_if_none("/etc/bluetooth");
	symlink("/rom/etc/bluetooth/private.pem", "/etc/bluetooth/private.pem");
	symlink("/rom/etc/bluetooth/public.pem", "/etc/bluetooth/public.pem");

	fp = fopen("/etc/bluetooth/main.conf", "w");
	if(fp)
	{
		_get_ble_name(hci_name, sizeof(hci_name));

		fprintf(fp, "[General]\n"
			"ControllerMode = %s\n"
			"Name = %s\n"
			, (dualmode)?"dual":"le"
			, hci_name
			);
		fclose(fp);
	}
}

#if defined(ET12) || defined(XT12)
static int bt_add_ble_uuid() {
	FILE *fp = NULL;
	char cmd[128], buf[128];
	int success = 0;

	snprintf(cmd, sizeof(cmd), "hcitool -i hci0 cmd 0x08 0x0008 "
			"18 "	// Length: 3 + 3 + 18
			"02 01 04 "	// Flag: BR/EDR not supported, LE only
			"02 0a fe "	// Tx Power: -2 dBm
			"11 07 "BASE_UUID" "
			"00 00 00 00 00 00 00"	// reserved
			, UUID_AMAP);
	//_dprintf("cmd: [%s]\n", cmd);
	fp = popen(cmd, "r");
	if(fp) {
		memset(buf, 0, sizeof(buf));
		while(fgets(buf, sizeof(buf), fp) != NULL) {
			if (strstr(buf, "HCI Event: 0x0e") != NULL) { // Event Complete
				success = 1;
				break;
			}
		}
		pclose(fp);
	}

	return success;
}
#endif

#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
void ble_rename_ssid(void)
{
	char *hci_inf = "hci0";
	char hci_ssid[32] = {0};
	char *ble_rename_argv[] = {"hciconfig", hci_inf, "name", hci_ssid, NULL};
	char *ble_ifdownup_argv[] = {"hciconfig", hci_inf, "down", "up", NULL};
#if defined(RTAX95Q) || defined(XT8PRO) || defined(BT12) || defined(BT10) || defined(BQ16) || defined(BQ16_PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(ET12) || defined(XT12) || defined(XD6_V2) || defined(BC109) || defined(EBG19) || defined(BC105) || defined(EBG15) || defined(XC5)
	char *ble_leadv_argv[] = {"hciconfig", hci_inf, "leadv", "0", NULL};
#endif

#if defined(RTAX56_XD4) || defined(PLAX56_XP4)
	if(nvram_match("HwId", "B") || nvram_match("HwId", "D")){
		/* Slave, no bluetooth */
		return;
	}
#endif
#if defined(RTAX56_XD4)
	if((nvram_match("HwId", "A") && nvram_get_int("BLE_BT") == 99) ||
		(nvram_match("HwId", "C") && nvram_get_int("BLE_BT") == 99)){
		/* Master without BT */
		return;
	}
#endif
	if (nvram_invmatch("x_Setting", "0")
		|| !pids("bluetoothd")
		|| nvram_match("ble_dut_con", "1")
		|| nvram_invmatch("ble_rename_ssid", "1")
	)
		return;

	_get_ble_name(hci_ssid, sizeof(hci_ssid));

	_eval(ble_rename_argv, NULL, 0, NULL);
	_eval(ble_ifdownup_argv, NULL, 0, NULL);
#if defined(ET12) || defined(XT12)
	bt_add_ble_uuid();
#endif
#if defined(RTAX95Q) || defined(XT8PRO) || defined(BT12) || defined(BT10) || defined(BQ16) || defined(BQ16_PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(ET12) || defined(XT12) || defined(XD6_V2) || defined(BC109) || defined(EBG19) || defined(BC105) || defined(EBG15) || defined(XC5)
	_eval(ble_leadv_argv, NULL, 0, NULL);
#endif
}
#endif

/*
        [BLE_Max_default_TX_power]
        ??0 dbm
        hcitool -i hci0 cmd 0x3F 0x0013 c2 02 00 09 00 12 00 7a 68 00 00 00 00 00 00 00 00 00 00
        ??4 dbm
        hcitool -i hci0 cmd 0x3F 0x0013 c2 02 00 09 00 12 00 7a 68 00 00 04 00 00 00 00 00 00 00
        ??8 dbm
        hcitool -i hci0 cmd 0x3F 0x0013 c2 02 00 09 00 12 00 7a 68 00 00 08 00 00 00 00 00 00 00
*/
void start_bluetooth_service(void)
{
	pid_t pid;
	char *ble_argv[]= { "bluetoothd", "-n", "-p", "aqis", NULL, NULL };
	int dbg = nvram_get_int("ble_dbg");
	int retry=0;
#if !defined(BLUECAVE)
	const char *str_inf = "hci0";
	char cmd[64];
#endif

	if(dbg) ble_argv[4] = "-d";

#if defined(RTAX56_XD4) || defined(PLAX56_XP4)
	if(nvram_match("HwId", "B") || nvram_match("HwId", "D")){
		/* Slave, no bluetooth */
		return;
	}
#endif
#if defined(RTAX56_XD4)
	if((nvram_match("HwId", "A") && nvram_get_int("BLE_BT") == 99) ||
		(nvram_match("HwId", "C") && nvram_get_int("BLE_BT") == 99)){
		/* Master without BT */
		return;
	}
#endif
#ifndef RTCONFIG_BCM_MFG
	if(nvram_get_int("x_Setting") == 1){
		_dprintf("BLE: skip start_bluetooth_service\n");
		return;
	}
#endif
	if (getpid()!=1) {
		notify_rc("start_bluetooth_service");
		return;
	}
#if defined(RTCONFIG_QCA) && defined(RTCONFIG_BT_CONN_UART)
	if (!pids("hciattach")) {
		return;
	}
	else
		sleep(2);	// wait a minute for device ready
#endif

#if !defined(BLUECAVE) && !defined(RTAX95Q) && !defined(XT8PRO) && !defined(BT12) && !defined(BT10) && !defined(BQ16) && !defined(BQ16_PRO) && !defined(BM68) && !defined(XT8_V2) && !defined(RTAXE95Q) && !defined(ET8PRO) && !defined(ET8_V2) && !defined(RTAX56_XD4) && !defined(XD4PRO) && !defined(RTAX82_XD6) && !defined(RTAX82_XD6S) && !defined(ET12) && !defined(XT12) && !defined(XD6_V2) && !defined(BC109) && !defined(EBG19) && !defined(EBG15) && !defined(BC105) && !defined(XC5)
	while (!check_bluetooth_device(str_inf)) {
		sleep(1);
		_dprintf("Failed to get HCI Device! Retry after 1 sec!\n");
		if (++retry > 30) {
			_dprintf("Disable BLE daemon!\n");
			return;
		}
	}
#endif
	if (pids("bluetoothd")) killall_tk("bluetoothd");

#if defined(RTAX95Q) || defined(XT8PRO) || defined(BT12) || defined(BT10) || defined(BQ16) || defined(BQ16_PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(ET12) || defined(XT12) || defined(XD6_V2) || defined(BC109) || defined(EBG19) || defined(BC105) || defined(EBG15) || defined(XC5)
	nvram_set("ble_init", "1");
	system("hciconfig hci0 up");
	doSystem("btconfig wba %s", get_label_mac());
	system("hciconfig hci0 reset");
	system("hciconfig hci0 down");
	_write_bluetoothd_conf();
	_eval(ble_argv, dbg?"/dev/console":NULL, 0, &pid);
	sleep(2);
	system("btmgmt bredr off");
	system("hciconfig hci0 up");
	system("hciconfig hci0 leadv 0");
	if (dbg) system("btmgmt info");
	nvram_set("ble_init", "0");
#else
	retry = 0;
reset_again:
	if (retry > 3) {
		_dprintf("BT: reset fail[%d]\n", retry);
		return;
	}

	if (system("hciconfig hci0 reset")) {
		_dprintf("BT: reset fail, try again[%d]\n", retry++);
		sleep(1);
		goto reset_again;
	}
#if defined(RTCONFIG_QCA) && !defined(BT_CONN_UART)
	if (change_bt_mac()) {
		_dprintf("BT: WBA fail, try again[%d]\n", retry++);
		sleep(1);
		goto reset_again;
	}
#endif

#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
	nvram_set_int("ble_rename_ssid", 0);
#endif
	system("hciconfig hci0 down");
	_eval(ble_argv, dbg?"/dev/console":NULL, 0, &pid);
	sleep(2);
	system("hciconfig hci0 up");
#if defined(RTCONFIG_QSDK10CS) || defined(RTCONFIG_SOC_IPQ60XX)
	system("btmgmt advertising on");
#else
	system("hciconfig hci0 leadv 0");
#endif
	if (dbg) system("btmgmt info");
#endif
#if defined(RTCONFIG_QCA) && defined(BT_CONN_UART)
	{
		system("hcitool -i hci0 cmd 0x3F 0x0013 c2 02 00 09 00 12 00 7a 68 00 00 04 00 00 00 00 00 00 00");
	}
#endif

#if !defined(BLUECAVE)
	memset(cmd, '\0', sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "hciconfig %s | grep 'UP RUNNING' -q", str_inf);
	if (system(cmd) == 0)
	{
#if defined(RTCONFIG_LP5523)
		lp55xx_leds_proc(LP55XX_BTCOR_LEDS, LP55XX_ACT_NONE);
#elif defined(RTCONFIG_ZENWIFI_RGBLED)
		set_rgbled(RGBLED_DEFAULT_STANDBY);
#endif /* RTCONFIG_LP5523 */
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
		nvram_set_int("ble_rename_ssid", 1);
		sleep(3);
		ble_rename_ssid();
#endif
	}
#endif

#if defined(ET12) || defined(XT12)
	retry = 3;
	while(retry) {
		if(!bt_add_ble_uuid()) {
			_dprintf("[BT] add BLE UUID fail!!!\n");
			 retry--;
			 sleep(1);
		}
		else
			break;
	}
#endif
	logmessage("BLE", "daemon is started");
}

void stop_bluetooth_service(void)
{
#if defined(RTAX56_XD4) || defined(PLAX56_XP4)
	if(nvram_match("HwId", "B") || nvram_match("HwId", "D")){
		/* Slave, no bluetooth */
		return;
	}
#endif
#if defined(RTAX56_XD4)
	if((nvram_match("HwId", "A") && nvram_get_int("BLE_BT") == 99) ||
		(nvram_match("HwId", "C") && nvram_get_int("BLE_BT") == 99)){
		/* Master without BT */
		return;
	}
#endif
	if (pids("bluetoothd")) {
#if defined(RTCONFIG_LP5523)
		lp55xx_leds_proc(LP55XX_ALL_BREATH_LEDS, LP55XX_ACT_BREATH_UP_00);
#endif
		nvram_unset("bt_turn_off_service");
		nvram_unset("bt_turn_off");
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
		nvram_unset("ble_dut_con");
		nvram_unset("ble_rename_ssid");
#endif
		_dprintf("Turn off Bluetooth\n");
		killall_tk("bluetoothd");
		system("hciconfig hci0 reset down");
#if defined(RTCONFIG_QCA) && defined(RTCONFIG_BT_CONN_UART)
		sleep(1);
		killall_tk("hciattach");
#endif
		logmessage("bluetoothd", "daemon is stoped");
	}

}
#endif	/* RTCONFIG_BT_CONN */

#if defined(RTCONFIG_QCA_EZMESH)
void start_ezmesh_cap(void)
{
	#define EZ_SCRIPT "/tmp/ez.sh"
	FILE *fp;

	if(nvram_get_int("x_Setting"))
	{
		if (!(fp = fopen(EZ_SCRIPT, "w+")))
			return;

		fprintf(fp, "sleep 8\n");
		fprintf(fp, "rc rc_service start_ezmesh_test\n");
		fprintf(fp, "rc rc_service start_wsplcd_test\n");
		fprintf(fp, "exit\n");
		fclose(fp);

		chmod(EZ_SCRIPT, 0777);
		doSystem("%s &", EZ_SCRIPT);
	}
	else
		_dprintf("EZ process is not started yet\n");
}	
#endif

#ifdef RTCONFIG_WIFI_SON
#if defined(RTCONFIG_ETHBACKHAUL) && defined(RTCONFIG_QCA_ORG_UPDOWN_SEPARATE)
void start_ethbl_lldpd(void)
{
	char ifname[32],buf[120];
	char *next;
	//char *lldpd[] = {"lldpd", nvram_safe_get("lan_ifname"), NULL};
	char *lldpd[] = {"lldpd", NULL};
	pid_t pid;
	int len = 0;

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
		if(len)
			len += sprintf(buf + len,",%s",ifname);
		else
			len = sprintf(buf,"%s",ifname);
	}
	sprintf(buf + len,",%s","br0");
	eval("lldpcli","configure","system","interface","pattern",buf);
	eval("lldpcli","configure","lldp","tx-interval","10");
	eval("lldpcli","configure","lldp","tx-hold","2");
}
#endif

void start_hyfi_process(void)
{
#ifdef RTCONFIG_AMAS
#if defined(RTCONFIG_WIFI_SON)
        if(nvram_match("wifison_ready","1"))
	{
                stop_amas_lib();
#ifdef RTCONFIG_NEW_USER_LOW_RSSI
        	stop_roamast();
#endif
	}
#endif
#endif
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
		if (!cmd) return;

		if (!strcmp(cmd, "xx")) {
			char *argv[]={"/sbin/delay_exec","60","reboot",NULL};
			_eval(argv, NULL, 0, &pid);
		} else if (memcmp(cmd, "SCH", 3)==0) { /* SCH2G_5G1_5G2 */
			int ch0, ch1, ch2;
			int my_ch0, my_ch1, my_ch2;
			my_ch0 = get_channel(get_wififname(0));
			my_ch1 = get_channel(get_wififname(1));
#if defined(RTCONFIG_HAS_5G_2)
			my_ch2 = get_channel(get_wififname(2));
#else
			my_ch2 = 0;
#endif
			ch0=atoi(cmd+3);
			ch1=atoi(cmd+6);
			ch2=atoi(cmd+10);
			//_dprintf("[LYRA SCH] CAP's channel %d:%d:%d, mine %d:%d:%d\n", ch0, ch1, ch2, my_ch0, my_ch1, my_ch2);
			if (ch0 && (my_ch0 != ch0)) /* 2.4G front haul channel */
				doSystem("iwconfig %s channel %d", get_wififname(0), ch0);
			if (ch1 && (my_ch1 != ch1 && nvram_get_int("eth_backl")==1)) /* 5G backhaul haul channel */
				doSystem("iwconfig %s channel %d", get_wififname(1), ch1);
#if defined(RTCONFIG_HAS_5G_2)
			if (ch2 && (my_ch2 != ch2)) /* 5G front haul channel */
				doSystem("iwconfig %s channel %d", get_wififname(2), ch2);
#endif
		}
		nvram_set("spcmd", "0");
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
void radiusd_ascii_to_char(void)
{
	sqlite3 *db;
	char *errMsg=NULL;
	char **dbResult;
	int nRow, nColumn;
	int ret=0;
	int i=0;
	char char_user[64], char_passwd[64];
	char tmp_ascii_user[64], tmp_ascii_passwd[64];
	char tmpSql[128];

	memset(char_user, 0, sizeof(char_user));
	memset(char_passwd, 0, sizeof(char_passwd));
	memset(tmp_ascii_user, 0, sizeof(tmp_ascii_user));
	memset(tmp_ascii_passwd, 0, sizeof(tmp_ascii_passwd));
	memset(tmpSql, 0, sizeof(tmpSql));

	if (sqlite3_open_v2("/tmp/freeradius.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)) {
		return;
	}

	ret = sqlite3_exec(db, "attach '/jffs/.sys/Permission/PMS_data.db3' as pms;insert into main.radcheck select NULL, Name, 'Cleartext-Password', ':=', passwd from pms.user;", 0, 0, &errMsg);
	if(ret != SQLITE_OK )
		return;

	if(sqlite3_get_table(db, "select * from radcheck", &dbResult, &nRow, &nColumn, &errMsg) == SQLITE_OK) {
		for(i = 1; i <= nRow ; i++ ) {
			sprintf(tmp_ascii_user, "%s", dbResult[i*nColumn+1]);
			sprintf(tmp_ascii_passwd, "%s", dbResult[i*nColumn+4]);
			ascii_to_char_safe(char_user, tmp_ascii_user, sizeof(char_user));
			ascii_to_char_safe(char_passwd, tmp_ascii_passwd, sizeof(char_passwd));
			sprintf(tmpSql, "update radcheck set value='%s' where username='%s';", char_passwd, char_user);
			//_dprintf("tmpSql=%s\n", tmpSql);
			ret = sqlite3_exec(db, tmpSql, 0, 0, &errMsg);
			if( ret != SQLITE_OK )
				return;
		}
	}
	if(errMsg!=NULL)
		sqlite3_free(errMsg);

	sqlite3_free_table(dbResult);
	sqlite3_close(db);
}

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

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
		if(!access("/jffs/.sys/Permission/PMS_data.db3", F_OK)){
		//add user permission data to freeradius database.
			//system("sqlite3 /tmp/freeradius.db 'attach \"/jffs/.sys/Permission/PMS_data.db3\" as pms;insert into main.radcheck select NULL, Name, \"Cleartext-Password\", \":=\", passwd from pms.user;'");
			radiusd_ascii_to_char();
		}
#endif
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
void chilli_localUser_passcode(void)
{
	FILE *fp;
	unsigned char s[512];
	char *p;
	char salt[32];
	char *passwd;

	passwd = nvram_safe_get("captive_portal_passcode");
	if(strlen(passwd) == 0)
		return;

	strcpy(salt, "$1$");
	f_read("/dev/urandom", s, 6);
	base64_encode(s, salt + 3, 6);
	salt[3 + 8] = 0;
	p = salt;
	while (*p) {
		if (*p == '+') *p = '.';
		++p;
	}

	fp=fopen("/etc/shadow.chilli", "w");

    if (fp==NULL){
	   perror("open local user file failed\n");
	   return;
	}

	p = crypt(passwd, salt);
	fprintf(fp, "noauth:%s:0:0:99999:7:0:0:\n", p);

	if(fp!=NULL) fclose(fp);
}
void chilli_localUser(void)
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
	unsigned char s[512];
	char *p;
	char salt[32];
	char *username, *passwd;

	strcpy(salt, "$1$");
	f_read("/dev/urandom", s, 6);
	base64_encode(s, salt + 3, 6);
	salt[3 + 8] = 0;
	p = salt;
	while (*p) {
		if (*p == '+') *p = '.';
		++p;
	}

	if(nvram_match("cp_authtype", "0"))
		fp = fopen("/tmp/localusers_cp", "w");
	else
		fp = fopen("/etc/shadow.chilli-cp", "w");

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
	if(nvram_match("cp_authtype", "0"))
		fprintf(fp, "noauth:noauth\n");
	else{
		nv = nvp = strdup(nvram_safe_get("captive_portal_adv_local_clientlist"));
		if (nv) {
			#ifdef RTCONFIG_MULTILAN_CFG
				while ((b = strsep(&nvp, ">")) != NULL) {
					if (vstrsep(b, ":", &username, &passwd) < 1)
						continue;
						
					if (!username || !passwd || strlen(username) <= 0)
							continue;
							
					p = crypt(passwd, salt);
					fprintf(fp, "%s:%s:0:0:99999:7:0:0:\n", username, p);	

				}
			#else
				while ((b = strsep(&nvp, "<")) != NULL) {
					if ((vstrsep(b, ">", &profile_idx, &userlist) != 2 ))
						continue;
					while((tmp = strsep(&userlist, ",")) != NULL ){
						/*if(strlen(tmp) > 0){
							fprintf(fp, "%s\n", tmp);
						}*/
						if((vstrsep(tmp, ":", &username, &passwd)!=2)) continue;
						if(strlen(username)==0||strlen(passwd)==0) continue;

						p = crypt(passwd, salt);
						fprintf(fp, "%s:%s:0:0:99999:7:0:0:\n", username, p);
					}
				}
			#endif
			free(nv);
		}
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
	//char domain[64];
	char *tmp_str=NULL;
	char *tmp=NULL, *passwd=NULL;;
	int time=0;
	char ip_mask[32] = "192.168.182.0/24";
	int gw[4] = { 192, 168, 182, 1}, cidr = 24;
#ifdef RTCONFIG_NVRAM_ENCRYPT
	char *dec_passwd = NULL, *dec_passwd2 = NULL;
#endif
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
	passwd = nvram_safe_get("captive_portal_passcode");
	if(strlen(passwd) > 0){
		chilli_localUser_passcode();
		fprintf(fp, "localusers %s\n", "/etc/shadow.chilli");
	}
	else
	{
		fprintf(fp, "localusers %s\n", "/tmp/localusers");
		if (!(local_fp = fopen("/tmp/localusers", "w"))) {
			perror("/tmp/localusers");
			return;
		}
		fprintf(local_fp, "noauth:noauth");
		fclose(local_fp);
	}
		/*fprintf(fp, "localusers %s\n", "/tmp/localusers");
		if (!(local_fp = fopen("/tmp/localusers", "w"))) {
			perror("/tmp/localusers");
			return;
		}
		passwd=nvram_safe_get("captive_portal_passcode");
		if(strlen(passwd) >0 ) fprintf(local_fp, "noauth:%s", passwd);
		else fprintf(local_fp, "noauth:noauth");
		fclose(local_fp);*/
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
	/*memset(domain, 0, sizeof(domain));
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
	}*/
	fprintf(fp, "cmdsocketport %s\n", "42424");
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) fprintf(fp, "ipv6\n");
#endif
	fprintf(fp, "tundev %s\n", "tun22");
	fprintf(fp, "uamaliasip %d.%d.%d.%d\n", gw[0], gw[1], gw[2], gw[3]);
	fprintf(fp, "redirssl\n");
	fprintf(fp, "sslcertfile %s\n", HTTPD_CERT);
	fprintf(fp, "sslkeyfile %s\n", HTTPD_KEY);
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

	char *chilli_uamsecret =  nvram_safe_get("chilli_uamsecret");
#ifdef RTCONFIG_NVRAM_ENCRYPT
	int declen = strlen(chilli_uamsecret);
	dec_passwd = malloc(declen);
	if(dec_passwd){
		pw_dec(chilli_uamsecret, dec_passwd, declen, 1);
		chilli_uamsecret = dec_passwd;
	}
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
		int declen2 = strlen(chilli_macpasswd);
		dec_passwd2 = malloc(declen2);
		if(dec_passwd2){
			pw_dec(chilli_macpasswd, dec_passwd2, declen2, 1);
			chilli_macpasswd = dec_passwd2;
		}
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
#ifdef RTCONFIG_NVRAM_ENCRYPT
	if(dec_passwd) free(dec_passwd);
	if(dec_passwd2) free(dec_passwd2);
#endif
	fflush(fp);
	fclose(fp);

	return;
}
void chilli_config_CP(void)
{
	FILE *fp;
	int i, k=0;
	char lan_ip[64];
	//char domain[64];
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
#ifdef RTCONFIG_COOVACHILLI
		chilli_localUser();
		if(nvram_match("cp_authtype", "0"))
			fprintf(fp, "localusers %s\n", "/tmp/localusers_cp");
		else
			fprintf(fp, "localusers %s\n", "/etc/shadow.chilli-cp");
#endif
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
	/*memset(domain, 0, sizeof(domain));
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
	}*/
	fprintf(fp, "unixipc %s\n", "chilli-cp.ipc");
	fprintf(fp, "cmdsocketport %s\n", "42425");
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) fprintf(fp, "ipv6\n");
#endif
	fprintf(fp, "uamport %s\n", "3998");
	fprintf(fp, "tundev %s\n", "tun23");
	fprintf(fp, "uamaliasip %d.%d.%d.%d\n", gw[0], gw[1], gw[2], gw[3]);
	fprintf(fp, "redirssl\n");
	fprintf(fp, "sslcertfile %s\n", HTTPD_CERT);
	fprintf(fp, "sslkeyfile %s\n", HTTPD_KEY);
	if((time=nvram_get_int("cp_authtime")) > 0)
		fprintf(fp, "challengetimeout2 %d\n", time);
	tmp_str=nvram_safe_get("cp_awaytime");
	if(strcmp(tmp_str,""))
		fprintf(fp, "defidletimeout %lu\n", strtoul(tmp_str, &tmp, 10));
	tmp_str=nvram_safe_get("cp_sessiontime");
	if(strcmp(tmp_str,""))
		fprintf(fp, "defsessiontimeout %lu\n", strtoul(tmp_str, &tmp, 10));

	fprintf(fp, "dns1 %s\n", nvram_safe_get("lan_ipaddr"));

	char *enc_value =  nvram_safe_get("cp_uamsecret");
#ifdef RTCONFIG_NVRAM_ENCRYPT
	int declen = strlen(enc_value);
	char *dec_passwd = NULL;
	dec_passwd = malloc(declen);
	if(dec_passwd){
		pw_dec(enc_value, dec_passwd, declen, 1);
		enc_value = dec_passwd;
	}
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
#ifdef RTCONFIG_NVRAM_ENCRYPT
	if(dec_passwd) free(dec_passwd);
#endif
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
	char ifname[32], *next = NULL;

#ifdef	RTCONFIG_MULTILAN_CFG
	nvram_set("captive_portal_enable", "off");
	nvram_set("chilli_enable", "0");
#else
	if(!nvram_match("captive_portal_enable", "on")){
		nvram_set("chilli_enable", "0");
	}
#endif

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
		unlink("/etc/shadow.chilli");
	}

#ifdef	RTCONFIG_MULTILAN_CFG

#else
    iface = nvram_safe_get("lan1_ifnames");
    if (iface) {
       foreach(ifname, iface, next) {
           ifconfig(ifname, 0, NULL, NULL);
       }
    }

	iface = nvram_safe_get("chilli_interface");
	if(iface){
		ifconfig(FREEWIFIIF, 0, NULL, NULL);
		eval("brctl","delbr", FREEWIFIIF); //delete interface br1
		nvram_set("lan1_ifname", "");
		nvram_set("lan1_ifnames", "");
	}
#endif
	start_firewall(wan_primary_ifunit(), 0);

	return;
}
void stop_CP(void)
{
	char *iface = NULL;
	char ifname[32], *next = NULL;

#ifdef	RTCONFIG_MULTILAN_CFG
	nvram_set("captive_portal_adv_enable", "off");
	nvram_set("cp_enable", "0");
#else
	if(!nvram_match("captive_portal_adv_enable", "on")){
	   nvram_set("cp_enable", "0");
	}
#endif

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
		//unlink("/tmp/localusers_cp");
		unlink("/etc/shadow.chilli-cp");
	}
#ifdef	RTCONFIG_MULTILAN_CFG
#else
    iface = nvram_safe_get("lan2_ifnames");
    if (iface) {
       foreach(ifname, iface, next) {
           ifconfig(ifname, 0, NULL, NULL);
       }
	}

	iface = nvram_safe_get("cp_interface");
	if(iface){
		ifconfig(CPIF, 0, NULL, NULL);
		eval("brctl","delbr", CPIF); //delete interface br2
		nvram_set("lan2_ifname", "");
		nvram_set("lan2_ifnames", "");
	}
#endif
	start_firewall(wan_primary_ifunit(), 0);

	return;
}

#ifdef	RTCONFIG_MULTILAN_CFG
void Checkifnamesn(char *nvifnames, char *ifname)
{
	char tmp_ifnames[128], *next=NULL;
	char ifnames[16];
	memset(tmp_ifnames, 0, sizeof(tmp_ifnames));
	memset(ifnames, 0, sizeof(ifnames));
	foreach(ifnames, nvifnames, next){
		if(strcmp(ifname, ifnames))
			strcat(tmp_ifnames, ifnames);
			strcat(tmp_ifnames, " ");
	}
	strcpy(nvifnames, tmp_ifnames);
}
void bridge_ifByifname(char *ifs, char *brif, char *delif, int flag)
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

	char *duptr=NULL, *sptr=NULL, *dstr = NULL;
	if(ifs){
		int dct = 0;
		duptr = sptr = strdup(ifs);
		if(duptr){
			while ((dstr = strsep(&sptr, ",")) != NULL) {
					eval("brctl", "delif", delif, dstr);
					eval("brctl", "addif", brif, dstr);
					strcat(brifnames,dstr);
					strcat(brifnames," ");
					Checkifnamesn(lan_ifnames, dstr);
					dct++;
			}
			free(duptr);
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

}

int gen_chilli_ipup_script(char *type, int mtu)
{
	FILE *fp = NULL;
	char file_name[32];

	if (!type || strlen(type) == 0) {
		_dprintf("[%s-%d] no type.\n", __FUNCTION__, __LINE__);
		return 0;
	}

	snprintf(file_name, sizeof(file_name), "/tmp/ipup_%s.sh", type);
	fp = fopen(file_name, "w+");
	if (!fp) {
		_dprintf("[%s-%d] open %s fail.\n", __FUNCTION__, __LINE__, file_name);
		return 0;
	}

	fprintf(fp, "#!/bin/sh\n");
	fprintf(fp, "ifconfig $DEV mtu %d\n", mtu);

	if (fp)
		fclose(fp);

	chmod(file_name, 0777);

	return 1;
}

void chilli_config_CPN(char *sn, char *dns1, char *dns2, char *sub_ip)
{
	FILE *fp;
	int i, k=0;
	char lan_ip[64];
	//char domain[64];
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
#ifdef RTCONFIG_COOVACHILLI
		chilli_localUser();
		if(nvram_match("cp_authtype", "0"))
			fprintf(fp, "localusers %s\n", "/tmp/localusers_cp");
		else
			fprintf(fp, "localusers %s\n", "/etc/shadow.chilli-cp");
#endif
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
		fprintf(fp, "dhcpif %s\n", sn);
	else
		fprintf(fp, "dhcpif %s\n", "br0");

	memset(chilli_url,0, sizeof(chilli_url));
#ifdef RTCONFIG_MULTILAN_CFG
	sprintf(chilli_url, "%s://%s:8083/Uam", nvram_safe_get("cp_protocol"), sub_ip);
#else		
	memset(lan_ip, 0, sizeof(lan_ip));
	strcpy(lan_ip, nvram_safe_get("lan_ipaddr"));

	if(!nvram_match("chilli_url", lan_ip))
		nvram_set("chilli_url", lan_ip);
	//sprintf(chilli_url, "https://%s/Uam", lan_ip);
	sprintf(chilli_url, "%s://%s:8083/Uam", nvram_safe_get("cp_protocol"), lan_ip);
#endif	
	fprintf(fp, "uamserver %s\n", chilli_url);
	/*memset(domain, 0, sizeof(domain));
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
	}*/
	fprintf(fp, "unixipc %s\n", "chilli-cp.ipc");
	fprintf(fp, "cmdsocketport %s\n", "42425");
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) fprintf(fp, "ipv6\n");
#endif
	fprintf(fp, "uamport %s\n", "3998");
	fprintf(fp, "tundev %s\n", "tun23");
	//fprintf(fp, "uamaliasip %d.%d.%d.%d\n", gw[0], gw[1], gw[2], gw[3]);
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

	
	if((dns1 != NULL && *dns1) && (dns2 == NULL || !*dns2 || strlen(dns2)<7))
		fprintf(fp, "dns1 %s\n", dns1);
	else if((dns2 != NULL && *dns2) && (dns1 == NULL || !*dns1 || strlen(dns1)<7))
		fprintf(fp, "dns2 %s\n", dns2);
	else if((dns1 == NULL || !*dns1 || strlen(dns1)<7) && (dns2 == NULL || !*dns2 || strlen(dns2)<7))
		fprintf(fp, "dns1 %s\n", sub_ip);
	else if((dns1 != NULL && *dns1) && (dns2 != NULL && *dns2)){
		fprintf(fp, "dns1 %s\n", dns1);
		fprintf(fp, "dns2 %s\n", dns2);
	}

	char *enc_value =  nvram_safe_get("cp_uamsecret");
#ifdef RTCONFIG_NVRAM_ENCRYPT
	int declen = strlen(enc_value);
	char *dec_passwd = NULL;
	dec_passwd = malloc(declen);
	if(dec_passwd){
		pw_dec(enc_value, dec_passwd, declen, 1);
		enc_value = dec_passwd;
	}
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
#ifdef RTCONFIG_NVRAM_ENCRYPT
	if(dec_passwd) free(dec_passwd);
#endif

#ifdef RTCONFIG_MULTILAN_CFG
	if (gen_chilli_ipup_script("cp", nvram_get_int("cp_mtu")))
		fprintf(fp, "ipup /tmp/ipup_cp.sh\n");
#endif	/* RTCONFIG_MULTILAN_CFG */

	fflush(fp);
	fclose(fp);

	return;
}
void chilli_confign(char *sn, char *dns1, char *dns2, char *sub_ip)
{
	FILE *fp;
	FILE *local_fp;
	int i, k=0;
	char lan_ip[64];
	char chilli_url[100];
	//char domain[64];
	char *tmp_str=NULL;
	char *tmp=NULL, *passwd=NULL;;
	int time=0;
	char ip_mask[32] = "192.168.182.0/24";
	int gw[4] = { 192, 168, 182, 1}, cidr = 24;
#ifdef RTCONFIG_NVRAM_ENCRYPT
	char *dec_passwd = NULL, *dec_passwd2 = NULL;
#endif
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
	passwd = nvram_safe_get("captive_portal_passcode");
	if(strlen(passwd) > 0){
		chilli_localUser_passcode();
		fprintf(fp, "localusers %s\n", "/etc/shadow.chilli");
	}
	else
	{
		fprintf(fp, "localusers %s\n", "/tmp/localusers");
		if (!(local_fp = fopen("/tmp/localusers", "w"))) {
			perror("/tmp/localusers");
			return;
		}
		fprintf(local_fp, "noauth:noauth");
		fclose(local_fp);
	}
		/*fprintf(fp, "localusers %s\n", "/tmp/localusers");
		if (!(local_fp = fopen("/tmp/localusers", "w"))) {
			perror("/tmp/localusers");
			return;
		}
		passwd=nvram_safe_get("captive_portal_passcode");
		if(strlen(passwd) >0 ) fprintf(local_fp, "noauth:%s", passwd);
		else fprintf(local_fp, "noauth:noauth");
		fclose(local_fp);*/
//	}

	if (nvram_match("chilli_nowifibridge", "1"))
		fprintf(fp, "dhcpif %s\n", sn);
	else
		fprintf(fp, "dhcpif %s\n", "br0");
		
	memset(chilli_url,0, sizeof(chilli_url));
#ifdef RTCONFIG_MULTILAN_CFG
	sprintf(chilli_url, "%s://%s:8083/FreeUam", nvram_safe_get("chilli_protocol"), sub_ip);
#else
	memset(lan_ip, 0, sizeof(lan_ip));
	strcpy(lan_ip, nvram_safe_get("lan_ipaddr"));

	if(!nvram_match("chilli_url", lan_ip))
	   nvram_set("chilli_url", lan_ip);
	//sprintf(chilli_url, "https://%s/Uam", lan_ip);
	sprintf(chilli_url, "%s://%s:8083/FreeUam", nvram_safe_get("chilli_protocol"), lan_ip);
#endif
	fprintf(fp, "uamserver %s\n", chilli_url);
	/*memset(domain, 0, sizeof(domain));
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
	}*/
	fprintf(fp, "cmdsocketport %s\n", "42424");
#ifdef RTCONFIG_IPV6
	if (ipv6_enabled()) fprintf(fp, "ipv6\n");
#endif
	fprintf(fp, "tundev %s\n", "tun22");
	//fprintf(fp, "uamaliasip %d.%d.%d.%d\n", gw[0], gw[1], gw[2], gw[3]);
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
	
	if((dns1 != NULL && *dns1) && (dns2 == NULL || !*dns2 || strlen(dns2)<7))
		fprintf(fp, "dns1 %s\n", dns1);
	else if((dns2 != NULL && *dns2) && (dns1 == NULL || !*dns1 || strlen(dns1)<7))
		fprintf(fp, "dns2 %s\n", dns2);
	else if((dns1 == NULL || !*dns1 || strlen(dns1)<7) && (dns2 == NULL || !*dns2 || strlen(dns2)<7))
		fprintf(fp, "dns1 %s\n", sub_ip);
	else if((dns1 != NULL && *dns1) && (dns2 != NULL && *dns2)){
		fprintf(fp, "dns1 %s\n", dns1);
		fprintf(fp, "dns2 %s\n", dns2);
	}

	char *chilli_uamsecret =  nvram_safe_get("chilli_uamsecret");
#ifdef RTCONFIG_NVRAM_ENCRYPT
	int declen = strlen(chilli_uamsecret);
	dec_passwd = malloc(declen);
	if(dec_passwd){
		pw_dec(chilli_uamsecret, dec_passwd, declen, 1);
		chilli_uamsecret = dec_passwd;
	}
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
		int declen2 = strlen(chilli_macpasswd);
		dec_passwd2 = malloc(declen2);
		if(dec_passwd2){
			pw_dec(chilli_macpasswd, dec_passwd2, declen2, 1);
			chilli_macpasswd = dec_passwd2;
		}
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
#ifdef RTCONFIG_NVRAM_ENCRYPT
	if(dec_passwd) free(dec_passwd);
	if(dec_passwd2) free(dec_passwd2);
#endif

#ifdef RTCONFIG_MULTILAN_CFG
	if (gen_chilli_ipup_script("chilli", nvram_get_int("chilli_mtu")))
		fprintf(fp, "ipup /tmp/ipup_chilli.sh\n");
#endif	/* RTCONFIG_MULTILAN_CFG */

	fflush(fp);
	fclose(fp);

	return;
}
#endif

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
	stop_CP();		//ensure that its stopped
#ifdef	RTCONFIG_MULTILAN_CFG
	MTLAN_T *pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	CP_TYPE_RL *cp_type_rl = (CP_TYPE_RL *)INIT_MTLAN(sizeof(CP_TYPE_RL));
	CP_LOCAL_AUTH *cp_local_auth = (CP_LOCAL_AUTH *)INIT_MTLAN(sizeof(CP_LOCAL_AUTH));
	CP_PROFILE *cp_profile = (CP_PROFILE *)INIT_MTLAN(sizeof(CP_PROFILE));
	CP_RADIUS_PROFILE *cp_radius_profile = (CP_RADIUS_PROFILE *)INIT_MTLAN(sizeof(CP_RADIUS_PROFILE));
	CP_RADIUS_LIST *cp_radius_lst = (CP_RADIUS_LIST *)INIT_MTLAN(sizeof(CP_RADIUS_LIST));
	
	size_t mtl_sz = 0, cp_radiuspf_sz = 0;
	int sdn_idx = -1, cp_idx = -1, vid_idx = -1, cp_type = -1;
	int i= 0;
	char *sub_ifname = NULL, *sub_ip = NULL, *sub_mask = NULL, *sub_dns1 = NULL, *sub_dns2 = NULL;
	int sub_dlease = 0;
	int cprofile_enable = -1;
	int apgx_idx = -1;
	int sdn_enable = 0;
	int cp_auth_en = 0;
	char *cp_lauth_pd = NULL;
	int cp_lauth_en = -1;
	int UType = -1;
	int cp_radius_idx = 0;
	int cp_radius_client_en = 0;
	int r_ornot = 0;
	int ip_byte[4];
	char new_net[32];
	
	//get enabled cp_idx from sdn_rl
	if(pmtl)
	{
		if(!get_mtlan(pmtl, &mtl_sz)){
			FREE_MTLAN((void *)pmtl);
			return NULL;
		}
		for(i = 0; i < mtl_sz; ++i)
		{
			if((pmtl[i].enable == 1) && (pmtl[i].sdn_t.cp_idx > 0) && (cp_type_rl)){
				
				if(!get_cp_type_byidx(pmtl[i].sdn_t.cp_idx, cp_type_rl)){
					FREE_MTLAN((void *)cp_type_rl);
					return NULL;
				}
				cp_type = cp_type_rl->cp_type;
				if(cp_type == 1){
					sdn_idx = pmtl[i].sdn_t.sdn_idx;
					cp_idx = pmtl[i].sdn_t.cp_idx;
					vid_idx = pmtl[i].vid;
					sub_ifname = pmtl[i].nw_t.br_ifname;
					apgx_idx = pmtl[i].sdn_t.apg_idx;
					sdn_enable = pmtl[i].enable;
					sub_ip = pmtl[i].nw_t.addr;
					sub_mask = pmtl[i].nw_t.netmask;
					sub_dlease = pmtl[i].nw_t.dhcp_lease;
					sub_dns1 = pmtl[i].nw_t.dns[0];
					sub_dns2 = pmtl[i].nw_t.dns[1];
					if (sscanf(sub_ip, "%d.%d.%d.%d", &ip_byte[0], &ip_byte[1], &ip_byte[2], &ip_byte[3]) == 4) {
						snprintf(new_net, sizeof(new_net), "%d.%d.%d.0/24", ip_byte[0], ip_byte[1], ip_byte[2]);
						nvram_set("cp_net", new_net);
					}
					break;
				}
			}
			
		}
	}
	
	//cpX_profile
	int cp_au_type = 0, cp_connouttime = 0, cp_idle_timeout = 0, cp_auth_timeout = 0, bwMaxUp = 0, bwMaxDown = 0, enservice = 0;
	char *UPath = NULL, *hIdx = NULL, *nid = NULL, *cusid = NULL;
	char pcode_clist[2048];
	int radiusOrn = 0;
	if((cp_profile) && (sdn_enable == 1)){
		if(!get_cpX_profile_by_cpidx(cp_idx, cp_profile)){
			FREE_MTLAN((void *)cp_profile);
			return NULL;
		}
		if(cp_profile->cp_enable == 1){
			cprofile_enable = cp_profile->cp_enable;
			nvram_set("captive_portal_adv_enable", "on");
			cp_au_type = cp_profile->cp_auth_type;
			UPath = cp_profile->redirecturl;
			cp_connouttime = cp_profile->cp_conntimeout;
			cp_idle_timeout = cp_profile->cp_idletimeout;
			cp_auth_timeout = cp_profile->cp_authtimeout;
			nvram_set_int("captive_portal_adv_idle_timeout", cp_auth_timeout);
			hIdx = cp_profile->cp_idx_ui_customizes;	
			bwMaxUp = cp_profile->cp_bw_limit_ul;
			bwMaxDown= cp_profile->cp_bw_limit_dl;
			enservice = cp_profile->cp_term_service;
			if(UPath){
				UType = 1;
			}else{
				UType = 0;
			}
			nid = cp_profile->cp_nas_id;
			cusid = cp_profile->cp_idx_ui_customizes;
			//pcode_clist = (char *)malloc((strlen(cusid)+1));
			//strcat(pcode_clist, cusid);
			memset(pcode_clist, 0, sizeof(pcode_clist));
			snprintf(pcode_clist, sizeof(pcode_clist), "<%s>", cusid); 
			nvram_set("cp_enable", "1");
			nvram_set_int("cp_bandwidthMaxUp", bwMaxUp);
			nvram_set_int("cp_bandwidthMaxDown", bwMaxDown);	
		}else
			return NULL;		
	}else
		return NULL;
	
	//auth
	char *pcode = NULL;
	int r_idx = 0, r_port1 = 0, r_acct_port1 = 0, r_port2 = 0, r_acct_port2 = 0;
	char *r_ip1 = NULL, *r_key1 = NULL, *r_acct_ip1 = NULL, *r_acct_key1 = NULL, *r_ip2 = NULL, *r_key2 = NULL, r_acct_ip2 = NULL, *r_acct_key2 = NULL;
	
	size_t rlsz = 0;
	char ifnames[256];
	char bn[16];
	char *cr = NULL;
	if(cp_local_auth){
		if(cp_au_type == 0){
			cp_auth_en = 0;
			nvram_set("captive_portal_adv_local_clientlist", pcode_clist);
		}else if(cp_au_type == 1){
			cp_auth_en = 1;
			if(!get_cp_localauth_bytype(cp_local_auth, cp_idx, cp_au_type)){
					FREE_MTLAN((void *)cp_local_auth);
					return NULL;
			}
			pcode = cp_local_auth->passcode;
			if(pcode && *pcode){
				//snprintf(pcode_clist, sizeof(pcode_clist), "%s", cusid);
				strcat(pcode_clist, pcode);
				nvram_set("captive_portal_adv_local_clientlist", pcode_clist);
			}
		}else if(cp_au_type == 2){
			r_ornot = 1;
			//radius auth
			if((cp_radius_profile) && (sdn_enable == 1) && (cp_au_type == 2)){
				int h = -1;
				if(!get_cpX_radius_profile(cp_radius_profile, cp_idx)){
					FREE_MTLAN((void *)cp_radius_profile);
					return NULL;
				}
				cp_radius_idx = cp_radius_profile->radius_idx;
			}
			if(!match_radius_rl_by_cpidx(cp_radius_lst, cp_radius_idx)){
					FREE_MTLAN((void *)cp_radius_lst);
					return NULL;
			}
			r_idx = cp_radius_lst->idx;
			r_ip1 = cp_radius_lst->authipaddr1;
			r_port1 = cp_radius_lst->authport1;
			r_key1 = cp_radius_lst->authkey1;
			r_acct_ip1 = cp_radius_lst->acct_ipaddr1;
			r_acct_port1 = cp_radius_lst->acct_port1;
			r_acct_key1 = cp_radius_lst->acct_key1;
			
		}
		//ifnames = match_vif_from_rulelist(&rlsz, vid_idx);
		memset(ifnames, 0, sizeof(ifnames));
		if (get_ap_wifi_ifnames_by_vid(vid_idx, ifnames, sizeof(ifnames)-1))
		    _dprintf("[FUN:%s][LINE:%d][WARNING] right wifi ifnames\n",__FUNCTION__,__LINE__);
		else
		    _dprintf("[FUN:%s][LINE:%d][WARNING] wifi ifnames is NULL\n",__FUNCTION__,__LINE__);
		memset(bn, 0, sizeof(bn));
		snprintf(bn, sizeof(bn), "apg%d_ssid", apgx_idx);
		cr = nvram_safe_get(bn);	
	}

	// //authType: UI selected(username password)  or not

	char dst_cp[2048];
	char *uamallowed = NULL;
	memset(dst_cp, 0, sizeof(dst_cp));
	snprintf(dst_cp,sizeof(dst_cp),"<%s>%s>%d>%d>%s>%s>>%d>%d>%s>%d>%s>%s>%d>%d>%d", hIdx, cr, cp_idle_timeout, cp_connouttime, UPath, ifnames,cp_auth_en, r_ornot, r_ip1, r_port1, r_key1, nid, enservice, bwMaxDown,bwMaxUp);
	nvram_set("captive_portal_adv_profile", dst_cp);
	sleep(1);
#endif
	char htmlPath[128];
	char cssPath[128];
	char jsonPath[128];
	char *nv=NULL, *nvp=NULL, *b=NULL;
	int brCount=0;
	char *profileName=NULL, *htmlIdx=NULL,    *awayTime=NULL,   *sessionTime=NULL;
	char *UIPath=NULL,      *ifName=NULL,     *UamAllowed=NULL, *authType=NULL, *RadiusOrNot=NULL;
	char *RadiusIP=NULL,    *RadiusPort=NULL, *RadiusScrt=NULL, *RadiusNas=NULL;
	char  *enService=NULL, *bandwidthMaxUp=NULL, *bandwidthMaxDown=NULL;
	char word[64], *next = NULL;

	if(!nvram_match("captive_portal_adv_enable", "on"))
		return;
	if (nvram_get_int("sw_mode") != SW_MODE_ROUTER)
		return;

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
#ifdef	RTCONFIG_MULTILAN_CFG
#else
			  if(1 != (is_intf_up(CPIF))){
		  	     eval("brctl", "addbr", CPIF);
		  	     eval("ifconfig", CPIF, "up");
			  }
#endif
			  memset(htmlPath, 0, sizeof(htmlPath));
			  memset(cssPath, 0, sizeof(cssPath));
			  sprintf(htmlPath, "/jffs/customized_splash/%s.html", htmlIdx);
			  sprintf(cssPath, "/jffs/customized_splash/%s.css", htmlIdx);
			  sprintf(jsonPath, "/jffs/customized_splash/%s.json", htmlIdx);
#ifdef	RTCONFIG_MULTILAN_CFG
#else
			  bridge_ifByA(ifName, CPIF, 1);
              foreach(word, nvram_safe_get("lan2_ifnames"), next) {
                  if (1 != (is_intf_up(word)))
                      ifconfig(word, IFUP, NULL, NULL);
			  }
#endif
#ifdef	RTCONFIG_MULTILAN_CFG
			if(sessionTime){
			  	nvram_set_int("cp_sessiontime", atoi(sessionTime));
			}
#else
			  nvram_set("cp_sessiontime", sessionTime);
#endif
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

		memset(ip_cidr, 0, sizeof(ip_cidr));
		strlcpy(ip_cidr, nvram_get("cp_net"), sizeof(ip_cidr));
		r = test_and_get_free_char_network(7, ip_cidr, EXCLUDE_NET_CAPTIVE_PORTAL);
		if (r == 1) {
			dbg("Network conflicts. Choose %s for Captive Portal.\n", ip_cidr);
			logmessage("Captive Portal", "%s.\n", ip_cidr);
			nvram_set("cp_net", ip_cidr);
		}
#ifdef	RTCONFIG_MULTILAN_CFG
		chilli_config_CPN(sub_ifname,sub_dns1,sub_dns2,sub_ip);
#else
		chilli_config_CP();
#endif
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
#ifdef	RTCONFIG_MULTILAN_CFG
	if(pmtl) FREE_MTLAN((void *)pmtl);
	if(cp_profile) FREE_MTLAN((void *)cp_profile);
	if(cp_local_auth) FREE_MTLAN((void *)cp_local_auth);
	if(cp_type_rl) FREE_MTLAN((void *)cp_type_rl);
	if(cp_radius_profile) FREE_MTLAN((void *)cp_radius_profile);	
	if(cp_radius_lst) FREE_MTLAN((void *)cp_radius_lst);
#endif
	return;
}

void start_chilli(void)
{

	stop_chilli();		//ensure that its stopped
#ifdef	RTCONFIG_MULTILAN_CFG

	MTLAN_T *pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	CP_TYPE_RL *cp_type_rl = (CP_TYPE_RL *)INIT_MTLAN(sizeof(CP_TYPE_RL));
	CP_LOCAL_AUTH *cp_local_auth = (CP_LOCAL_AUTH *)INIT_MTLAN(sizeof(CP_LOCAL_AUTH));
	CP_PROFILE *cp_profile = (CP_PROFILE *)INIT_MTLAN(sizeof(CP_PROFILE));


	size_t mtl_sz = 0;
	int sdn_idx = -1, cp_idx = -1, cp_type = -1, vid_idx = -1;
	int i= 0;
	char *sub_ifname = NULL;
	int cp_connouttime = 0, cp_idle_timeout = 0, cp_auth_timeout = 0, sub_dlease = 0;
	char *sub_ip = NULL, *sub_mask = NULL, *sub_dns1 = NULL, *sub_dns2 = NULL;
	
	char *hIdx=NULL, *UPath=NULL;
	int bwMaxDown = 0, bwMaxUp = 0;
	int cprofile_enable = 0;
	int cp_au_type = 0;
	int apgx_idx = -1;
	int sdn_enable = -1;
	int enservice = -1;
	int cp_lauth_en = -1;
	int UType = -1;
	int ip_byte[4];
	char new_net[32];

	//get enabled cp_idx from sdn_rl
	if(pmtl)
	{
		if(!get_mtlan(pmtl, &mtl_sz)){
			FREE_MTLAN((void *)pmtl);
			return NULL;
		}
		for(i = 0; i < mtl_sz; ++i)
		{
			if((pmtl[i].enable == 1) && (pmtl[i].sdn_t.cp_idx > 0) && (cp_type_rl)){
				
				if(!get_cp_type_byidx(pmtl[i].sdn_t.cp_idx, cp_type_rl)){
					FREE_MTLAN((void *)cp_type_rl);
					return NULL;
				}
				cp_type = cp_type_rl->cp_type;
				if(cp_type == 2 || cp_type == 4){

					sdn_idx = pmtl[i].sdn_t.sdn_idx;
					cp_idx = pmtl[i].sdn_t.cp_idx;
					vid_idx = pmtl[i].vid;
					sub_ifname = pmtl[i].nw_t.br_ifname;
					apgx_idx = pmtl[i].sdn_t.apg_idx;
					sdn_enable = pmtl[i].enable;
					_dprintf("[FUN:%s][LINE:%d]----------cp_idx=%d----sub_ifname=%s-----\n",__FUNCTION__,__LINE__,cp_idx,sub_ifname);
					sub_ip = pmtl[i].nw_t.addr;
					sub_mask = pmtl[i].nw_t.netmask;
					sub_dlease = pmtl[i].nw_t.dhcp_lease;
					sub_dns1 = pmtl[i].nw_t.dns[0];
					sub_dns2 = pmtl[i].nw_t.dns[1];
					_dprintf("[FUN:%s][LINE:%d]-sub_ip=%s-sub_ifname=%s-sub_dlease=%d-dns1=%s-dns2=%s-\n",__FUNCTION__,__LINE__,sub_ip,sub_mask,sub_dlease,sub_dns1,sub_dns2);
					if (sscanf(sub_ip, "%d.%d.%d.%d", &ip_byte[0], &ip_byte[1], &ip_byte[2], &ip_byte[3]) == 4) {
						snprintf(new_net, sizeof(new_net), "%d.%d.%d.0/24", ip_byte[0], ip_byte[1], ip_byte[2]);
						nvram_set("chilli_net", new_net);
					}
					break;
				}
			}
			
		}
	} 

	if((cp_profile) && (sdn_enable == 1)){
		if(!get_cpX_profile_by_cpidx(cp_idx, cp_profile)){
			FREE_MTLAN((void *)cp_profile);
			return NULL;
		}
		if(cp_profile->cp_enable == 1){
			cprofile_enable = cp_profile->cp_enable;
			nvram_set("captive_portal_enable", "on");
			cp_au_type = cp_profile->cp_auth_type;
			UPath = cp_profile->redirecturl;
			cp_connouttime = cp_profile->cp_conntimeout;
			cp_idle_timeout = cp_profile->cp_idletimeout;
			cp_auth_timeout = cp_profile->cp_authtimeout;
			hIdx = cp_profile->cp_idx_ui_customizes;	
			bwMaxUp = cp_profile->cp_bw_limit_ul;
			bwMaxDown= cp_profile->cp_bw_limit_dl;
			enservice = cp_profile->cp_term_service;
			if(UPath){
				UType = 1;
			}else{
				UType = 0;
			}
			nvram_set("chilli_enable", "1");
			nvram_set_int("chilli_bandwidthMaxUp", bwMaxUp);
			nvram_set_int("chilli_bandwidthMaxDown", bwMaxDown);	
		}else
			return NULL;		
	}else 
		return NULL;

	if(cp_local_auth){
		char *pcode = NULL;
		if(cp_au_type == 0){
			nvram_set("captive_portal_passcode", "");
		}else if((cp_au_type == 1) || (cp_au_type == 2)){
			cp_lauth_en = 1;
			if(!get_cp_localauth_bytype(cp_local_auth, cp_idx, cp_au_type)){
					FREE_MTLAN((void *)cp_local_auth);
					return NULL;
			}
			pcode = cp_local_auth->passcode;
			if(pcode && *pcode){
				nvram_set("captive_portal_passcode", pcode);
			}else
				nvram_set("captive_portal_passcode", "");
		}
	}

/*
	size_t rlsz = 0;
	char *ifnames = NULL;
	ifnames = match_vif_from_rulelist(&rlsz, vid_idx);
*/
	char ifnames[256];
	memset(ifnames, 0, sizeof(ifnames));
	if (get_ap_wifi_ifnames_by_vid(vid_idx, ifnames, sizeof(ifnames)-1))
	    _dprintf("[FUN:%s][LINE:%d][WARNING] right wifi ifnames\n",__FUNCTION__,__LINE__);
	else
	    _dprintf("[FUN:%s][LINE:%d][WARNING] wifi ifnames is NULL\n",__FUNCTION__,__LINE__);

	char bn[16];
	memset(bn, 0, sizeof(bn));
	snprintf(bn, sizeof(bn), "apg%d_ssid", apgx_idx);
	char *cr = NULL;
	cr = nvram_safe_get(bn);

	char dst_cp[2048];
	memset(dst_cp, 0, sizeof(dst_cp));
	snprintf(dst_cp,sizeof(dst_cp),"<%s>%s>%d>%d>%s>%s>%d>%d>%d>%d", hIdx, cr, cp_connouttime, UType, UPath, ifnames, enservice, cp_lauth_en, bwMaxUp, bwMaxDown);
	nvram_set("captive_portal", dst_cp);
 #endif

	//char htmlIdx[64];
	char htmlPath[128];
	char cssPath[128];
#ifdef	RTCONFIG_MULTILAN_CFG
	char jsonPath[128];
#endif
	char tmpCmd[256];
	char *nv=NULL, *nvp=NULL, *b=NULL;
	int brCount=0;
	char *ifName=NULL, /* *typeIdx=NULL,*/ *htmlIdx=NULL, *sessionTime=NULL;
	char *UIType=NULL, *UIPath=NULL, *enService=NULL, *brdName=NULL, *enPass;
	char *bandwidthMaxUp=NULL, *bandwidthMaxDown=NULL;
	char BrandName[32];
	int k=0;
	char word[64], *next = NULL;
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
#ifdef	RTCONFIG_MULTILAN_CFG
#else
			  if(1 != (is_intf_up(FREEWIFIIF))){
		  	     eval("brctl", "addbr", FREEWIFIIF);
		  	     eval("ifconfig", FREEWIFIIF, "up");
			  }
#endif
			  memset(htmlPath, 0, sizeof(htmlPath));
			  memset(cssPath, 0, sizeof(cssPath));
#ifdef	RTCONFIG_MULTILAN_CFG
			  memset(jsonPath, 0, sizeof(jsonPath));
#endif
#if 0
			  if (BYPASS_CP == atoi(typeIdx))
			      sprintf(htmlPath, "%s",  BypassUI);
			  else
#endif
			  sprintf(htmlPath, "/jffs/customized_splash/%s.html", htmlIdx);
			  sprintf(cssPath, "/jffs/customized_splash/%s.css", htmlIdx);
#ifdef	RTCONFIG_MULTILAN_CFG
			  sprintf(jsonPath, "/jffs/customized_splash/%s.json", htmlIdx);
#endif

#ifdef	RTCONFIG_MULTILAN_CFG
			  //bridge_ifByifname(ifnames, FREEWIFIIF, sub_ifname,0);
#else
			  bridge_ifByA(ifName, FREEWIFIIF, 0);
		      foreach(word, nvram_safe_get("lan1_ifnames"), next) {
			  if (1 != (is_intf_up(word)))
			       ifconfig(word, IFUP, NULL, NULL);
		      }
#endif
			  memset(BrandName, 0, sizeof(BrandName));
			  strcpy(BrandName, brdName);
			  while(BrandName[k] != '\0'){
				if(BrandName[k] == ' ')
		   		   BrandName[k]='-';
				   k++;
			   }
			  nvram_set("brdName", BrandName);  //coovachilli added
#ifdef	RTCONFIG_MULTILAN_CFG
			  nvram_set_int("chilli_sessiontime", atoi(sessionTime));
#else
			  nvram_set_int("chilli_sessiontime", 60 * atoi(sessionTime));
#endif
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

		memset(ip_cidr, 0, sizeof(ip_cidr));
		strlcpy(ip_cidr, nvram_get("chilli_net"), sizeof(ip_cidr));
		r = test_and_get_free_char_network(7, ip_cidr, EXCLUDE_NET_FREE_WIFI);
		if (r == 1) {
			dbg("Network conflicts. Choose %s for Free Wi-Fi.\n", ip_cidr);
			logmessage("Free Wi-Fi", "%s.\n", ip_cidr);
			nvram_set("chilli_net", ip_cidr);
		}
#ifdef	RTCONFIG_MULTILAN_CFG
		chilli_confign(sub_ifname,sub_dns1,sub_dns2,sub_ip);
#else
		chilli_config();
#endif
	}
	else if(nvram_match("hotss_enable", "1"))
		hotspotsys_config();
	//_dprintf("66\n");

	kill_pidfile_tk("/var/run/chilli.pid");
	//_dprintf("77\n");

	if(htmlIdx!=NULL){
#ifdef	RTCONFIG_MULTILAN_CFG
		unlink("/www/SDN/Captive_Portal/FreeWiFi.html");
		symlink("/www/SDN/Captive_Portal/FreeWiFi.html", htmlPath);
		unlink("/www/SDN/Captive_Portal/FreeWiFi.css");
		symlink("/www/SDN/Captive_Portal/FreeWiFi.html", cssPath);
		
		unlink("/tmp/uamsrv/www/FreeUam.json");
		symlink(jsonPath, "/tmp/uamsrv/www/FreeUam.json");
#endif
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
#ifdef	RTCONFIG_MULTILAN_CFG
	if(pmtl) FREE_MTLAN((void *)pmtl);
	if(cp_profile) FREE_MTLAN((void *)cp_profile);
	if(cp_local_auth) FREE_MTLAN((void *)cp_local_auth);
	if(cp_type_rl) FREE_MTLAN((void *)cp_type_rl);
#endif
	return;

}
#endif // RTCONFIG_COOVACHILLI

#ifdef RTCONFIG_NOTIFICATION_CENTER
static int
get_nc_eIdx(NC_SETTING_T *p, int e)
{
	int i;
	for(i=0; i < MAX_NOTIFY_EVENT_NUM; i++) {
		if(p[i].eID == e) {
			return i;
		}
	}
	return -1;
}

void
sync_nc_conf(void)
{
	char *nv, *nvp, *b;
	char *eId = NULL, *eAct = NULL, *eType = NULL, *ePri = NULL;
	int i = 0;

	if (strcmp(nvram_safe_get("nc_setting_conf"), "")) {
		NC_SETTING_T *pEvt = NULL;
		pEvt = (NC_SETTING_T *) calloc(MAX_NOTIFY_EVENT_NUM, sizeof(NC_SETTING_T));
		if(pEvt == NULL) {
			return;
		}

		/* nvram to *pEvt */
		nvp = nv = strdup(nvram_safe_get("nc_setting_conf"));
		while (nv && (b = strsep(&nvp, "<")) != NULL) {
			/* init */
			eId = eAct = eType = ePri = NULL;

			if (!strlen(b))
				continue;
			if (vstrsep(b, ">", &eId, &eAct, &eType, &ePri) < 3)
				continue;
			if (eId && *eId)
				pEvt[i].eID = strtol(eId, NULL, 16);
			if (eAct && *eAct)
				pEvt[i].eAct = strtol(eAct, NULL, 10);
			if (eType && *eType)
				pEvt[i].eType = strtol(eType, NULL, 10);
			if (ePri && *ePri)
				pEvt[i].eType = strtol(eType, NULL, 10);
			i++;
		}
		free(nv);

		char synConf[2048];
		char tmp[20];
		int  bufsize;
		int  idx = -1;
		int  priority = 0;

		memset(synConf, 0, sizeof(synConf));
		for (i=0; mapInfo[i].value != 0; i++) {
			memset(tmp, 0, sizeof(tmp));
			idx = get_nc_eIdx(pEvt, mapInfo[i].value);
			if (idx != -1) {
				if (pEvt[idx].ePri == 0 && !(pEvt[idx].eID & RESERVATION_EVENT_PREFIX))
					priority = mapInfo[i].ePri;
				else
					priority = pEvt[idx].ePri;
				snprintf(tmp, sizeof(tmp), "<%x>%d>%d>%d",
					mapInfo[i].value,
					pEvt[idx].eAct,
					pEvt[idx].eType,
					priority);
			}
			else {
				snprintf(tmp, sizeof(tmp), "<%x>%d>%d>%d",
					mapInfo[i].value,
					mapInfo[i].action,
					mapInfo[i].eType,
					mapInfo[i].ePri);
			}
			bufsize = sizeof(synConf) - strlen(synConf);
			strncat(synConf, tmp, bufsize-1);
		}
		//dbg("[%s:(%d)]: %s\n", __FUNCTION__, __LINE__, synConf);
		nvram_set("nc_setting_conf", synConf);
		if (pEvt) {
			free(pEvt);
		}
	}
}

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
	if (!strcmp(nvram_safe_get("nc_setting_conf"), "") ||
	    !nvram_get("nc_ver") || (nvram_get_int("nc_ver") < NC_VERSION)) {

		_dprintf("[%s(%d)][WARNING] setting is empty or nc ver. is lower than %d. restore default settings.\n",
			 __FUNCTION__, __LINE__, NC_VERSION);
		snprintf(tmp, sizeof(tmp), "%d", NC_VERSION);
		nvram_set("nc_ver", tmp);

		memset(setConf, 0, sizeof(setConf));
		for (i=0; mapInfo[i].value != 0; i++) {
			memset(tmp, 0, sizeof(tmp));
			action = mapInfo[i].action;
			snprintf(tmp, sizeof(tmp), "<%x>%d>%d>%d", mapInfo[i].value, action, mapInfo[i].eType, mapInfo[i].ePri);
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

	if (mediabridge_mode())
		return 0;

#ifdef RTCONFIG_TCPLUGIN
	exec_tcplugin();
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

	if(getpid() != 1) { //not rc init process
		notify_rc("start_netool");
		return 0;
	}

	killall("netool", SIGTERM);

	return _eval(netool_argv, NULL, 0, &pid);
}

void
stop_netool(void)
{
	killall_tk("netool");
}
#endif

// add by Andrew
#ifdef RTCONFIG_NFCM
void start_nfcm()
{
	char *nfcm_argv[] = {"nfcm", NULL};
	pid_t pid;
        
	int enb = nvram_get_int("nfcm_enable");

	if (!enb)
		return;

	if(getpid() != 1) { //not rc init process
		notify_rc("start_nfcm");
		return;
	}

	killall("nfcm", SIGTERM);

	_eval(nfcm_argv, NULL, 0, &pid);

	return;
}

void stop_nfcm()
{
	if(getpid() != 1) { //not rc init process
		notify_rc("stop_nfcm");
		return;
	}

	killall("nfcm", SIGTERM);

	return;
}
#endif
// end of add

#ifdef RTCONFIG_FPROBE
void start_fprobe(void)
{
	if(nvram_get_int("fprobe_enable") == 0)
		return;

	char nf_srv[256]="";
	strncpy(nf_srv, nvram_get("nfcapd_server"), sizeof(nf_srv)-1);
	if(nf_srv[0] == '\0')
		return;

	int nf_port;
	nf_port = nvram_get_int("nfcapd_port");
	if(nf_port <= 0) nf_port = 9995;

	char brif[IFNAMSIZ]="";
	strncpy(brif, nvram_get("lan_ifname"), IFNAMSIZ-1);

	char nf_srvport[256];
	sprintf(nf_srvport, "%s:%d", nf_srv, nf_port);

	//please see the fprobe manual to understand the parameters
	//char *fprobe_argv[] = {"fprobe", "-i", brif, nf_srvport, "-fip -m1024 -B1024 -b200", NULL};
	char *fprobe_argv[] = {"fprobe", "-i", brif, "-fip", nf_srvport, NULL};
	pid_t pid;

	if(getpid()!=1) { //not rc init process
		notify_rc("start_fprobe");
		return;
	}

	killall("fprobe", SIGTERM);

	_eval(fprobe_argv, NULL, 0, &pid);

	return;
}

void stop_fprobe(void)
{
	if(getpid()!=1) { //not rc init process
		notify_rc("stop_fprobe");
		return;
	}

	killall("fprobe", SIGTERM);
}
#endif

#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
int
start_aura_rgb_nt(void)
{
	char *aura_rgb_nt_argv[] = {"aura_rgb_nt", NULL};
	pid_t pid;

	return _eval(aura_rgb_nt_argv, NULL, 0, &pid);
}

int
start_aura_rgb_sw(void)
{
	char *aura_rgb_sw_argv[] = {"aura_rgb_sw", NULL};
	pid_t pid;

	return _eval(aura_rgb_sw_argv, NULL, 0, &pid);
}
#endif

int
start_services(void)
{
#if 0
#ifdef HND_ROUTER
	start_jitterentropy();
#endif /* HND_ROUTER */
#else
	start_haveged();
#endif
#if defined(RTAX82U) || defined(DSL_AX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTAX6000) || defined(GT10) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(GTBE96) || defined(GTBE19000)
	start_ledg();
	start_ledbtn();
#endif
#ifdef GTAX6000
	start_antled();
#endif
#ifdef RTCONFIG_ASD
	start_asd();
#endif
#ifdef RTCONFIG_LANTIQ
	start_wave_monitor();
#endif
#if defined(RTCONFIG_RALINK_MT7621) || defined(RTCONFIG_RALINK_MT7629) || defined(RTCONFIG_RALINK_MT7622) || defined(RTCONFIG_MT798X)
	setup_smp(0);	/* for adjust smp_affinity of cpu */
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
	start_wlc_monitor();
#endif
#if defined(RTCONFIG_WLCEVENTD)
	start_wlceventd();
#endif
#if defined(RTCONFIG_HAPDEVENT)
	start_hapdevent();
#endif
#ifdef RTCONFIG_DNSQUERY_INTERCEPT
	start_dnsqd();
#endif
#ifdef RTCONFIG_ROUTERBOOST
	start_asus_rbd();
#endif
#ifdef RTCONFIG_AWSIOT
	start_awsiot();
#endif
#ifdef RTCONFIG_BCMWL6
#ifdef RTCONFIG_HSPOT
	start_hspotap();
#endif
	start_igmp_proxy();
#ifdef BCM_BSD
#if defined(RTCONFIG_BCMBSD_V2)
	smart_connect_realign_ifnames();
#endif
	start_bsd();
#endif
#ifdef BCM_APPEVENTD
	start_appeventd();
#endif
#ifdef BCM_CEVENTD
	start_ceventd();
#endif
#ifdef BCM_SSD
	start_ssd();
#endif
#ifdef BCM_EVENTD
	start_eventd();
#endif
	start_acsd();
#if defined(RTCONFIG_DHDAP) || defined(RTCONFIG_HND_ROUTER_AX)
	start_dhd_monitor();
#endif
#ifdef BCM_ASPMD
	start_aspmd();
#endif /* BCM_ASPMD */

#ifdef  __CONFIG_WBD__
	start_wbd();
#endif /* __CONFIG_WBD__ */

#endif	// RTCONFIG_BCMWL6

#ifdef RTCONFIG_MULTILAN_CFG
	start_dnsmasq(ALL_SDN);
#else
	start_dnsmasq();
#endif

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
#if defined(RTCONFIG_HMA)
	tpvpn_gen_hma_list();
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
#ifdef RTCONFIG_AHS
	start_ahs();
#endif /* RTCONFIG_AHS */
#ifdef RTCONFIG_TRAFFIC_LIMITER
	init_traffic_limiter();
#endif
#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
	start_aura_rgb_nt();
	start_aura_rgb_sw();
#endif
	start_watchdog();
	//start_check_watchdog();
    start_alt_watchdog(); // add by Andrew

#ifdef RTAC87U
	start_watchdog02();
#endif
#if defined(RTAC1200G) || defined(RTAC1200GP)
	start_wdg_monitor();
#endif
#if defined(CONFIG_BCMWL5) && !defined(HND_ROUTER) && defined(RTCONFIG_DUALWAN)
	if(!nvram_get_int("stop_adv_lb"))
		restart_dualwan();
#endif
#if defined(CONFIG_BCMWL5) && defined(RTCONFIG_FANCTRL)
	start_phy_tempsense();
#endif
#ifdef RTCONFIG_AMAS
	if (nvram_get_int("re_mode") == 1) {
#ifdef RTCONFIG_BHCOST_OPT
        start_amas_ssd();
		start_amas_status();
		start_amas_misc();
#endif
		start_amas_wlcconnect();
		start_amas_bhctrl();
#if !defined(RTCONFIG_NOWL)
		start_amas_lanctrl();
#endif
	}
	else
	{
#if defined(RTCONFIG_FRONTHAUL_DWB) || defined(RTCONFIG_VIF_ONBOARDING)
#if !defined(RTCONFIG_NOWL)
		if (is_router_mode() || access_point_mode())
			start_amas_lanctrl();
#endif
#endif
	}
#endif
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
	start_psta_monitor();
#endif
#if defined(RTCONFIG_AMAS) && (defined(RTCONFIG_BCMWL6) || defined(RTCONFIG_LANTIQ) || defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK) || defined(RTCONFIG_RALINK))
	start_obd();
#endif
#if defined(RTCONFIG_UPLOADER)
	start_uploader();
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_ETHOBD)
	start_eth_obd();
#endif
	start_snooper();
	start_lltd();
#ifdef RTCONFIG_TOAD
	start_toads();
#endif

#if defined(RTCONFIG_IPSEC)
	//if(nvram_get_int("ipsec_server_enable") || nvram_get_int("ipsec_client_enable"))
	rc_ipsec_nvram_convert_check();
	rc_ipsec_config_init();
#endif

#ifdef RTCONFIG_JFFS2USERICON
	start_lltdc();
#endif
	start_networkmap(1);

#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
	start_pptpd();
#endif

#ifdef RTCONFIG_USB
	//restart_nas_services(0, 1, 0);
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
	start_hour_monitor_service();
#endif
#ifdef RTCONFIG_CONNTRACK
	start_pctime_service();
#endif

#ifdef RTCONFIG_IPERF
	start_iperf();
	start_monitor();
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

#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
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

//#ifdef RTCONFIG_COOVACHILLI
#if defined(RTCONFIG_COOVACHILLI) && !defined(RTCONFIG_CAPTIVE_PORTAL)
	start_chilli();
	start_CP();
#ifdef RTCONFIG_MULTILAN_CFG
	handle_sdn_feature(ALL_SDN, SDN_FEATURE_ROUTING, RC_SERVICE_START);
#endif
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
	setup_passwd();
	start_uam_srv(); //move to init.c	/* start internal uam server */
#ifdef RTCONFIG_MULTILAN_CFG
	handle_sdn_feature(ALL_SDN, SDN_FEATURE_ROUTING, RC_SERVICE_START);
#endif
#endif

#if defined(RTCONFIG_BT_CONN)
	start_dbus_daemon();
#endif

	start_ecoguard();

//	start_upnp();

	sanity_logs();
#ifdef RTCONFIG_BCM_MFG
	brcm_mfg_services();
#endif
#if defined(RTCONFIG_MFGFW)
	if(nvram_match("mfgfw", "1"))
	brcm_mfg_services();
#endif
#ifdef RTCONFIG_CFGSYNC
	start_cfgsync();
#ifdef RTCONFIG_CONNDIAG
	start_conn_diag();
#endif
#endif

#if !(defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK) || defined(RTCONFIG_REALTEK)) \
 ||  (defined(RTCONFIG_SOC_IPQ8074))
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
#ifdef RTCONFIG_AMAS
	start_amas_lldpd();
#endif

#ifdef SW_DEVLED
	start_sw_devled();
#endif

#ifdef RTCONFIG_FRS_FEEDBACK
#ifdef RTCONFIG_DBLOG
	start_dblog(0);
#endif /* RTCONFIG_DBLOG */
#endif /* RTCONFIG_FRS_FEEDBACK */

#if defined(RTCONFIG_AMAS)
	start_amas_lib();
#endif
#if defined(RTCONFIG_RGBLED)
	start_aurargb();
#endif
#if defined(RTCONFIG_FANCTRL) && defined(RTCONFIG_QCA)
	if (strstr(nvram_safe_get("rc_support"), "fanctrl") != NULL)
		restart_fanctrl();
#endif
#ifdef RTCONFIG_FPROBE
	start_fprobe();
#endif
#ifdef RTCONFIG_NFCM
    start_nfcm();
#endif
#ifdef RTCONFIG_AMAS_ADTBW
	start_amas_adtbw();
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_VIF_ONBOARDING)
	set_onboarding_vif_status();
#endif
#ifdef RTCONFIG_TCPLUGIN
	exec_tcplugin();
	if(nvram_match("tencent_qmacc_enable", "1") && nvram_match("tencent_eula_check", "1"))
		start_qmacc();
#endif
#ifdef RTCONFIG_BCM_OAM
	start_oam();
#endif
#ifdef RTCONFIG_NBR_RPT
	start_nbr_monitor();
#endif

#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
	start_detect_plc();
#endif

#ifdef RTCONFIG_SCHED_DAEMON
        start_sched_daemon();
#endif

#ifdef RTCONFIG_WIREGUARD
	start_wgsall();
#endif
#ifdef RTCONFIG_SSH
	start_sshd();
#endif
#ifdef RTCONFIG_FSMD
	system("fsmd");
#endif
#ifdef RTCONFIG_GEARUPPLUGIN
	start_gu_service();
#endif

#ifdef RTCONFIG_MULTIWAN_IF
	start_mtwanduck();
#endif

#ifdef RTCONFIG_MTK_BSD
	 system("mkdir -p /etc/map");
	if (nvram_match("smart_connect_x", "1") && !pids("bs20"))
		start_mtk_bs20();
#endif

	run_custom_script("services-start", 0, NULL, NULL);

	return 0;
}

void
stop_services(void)
{
	run_custom_script("services-stop", 0, NULL, NULL);

#ifdef RTCONFIG_FSMD
	killall_tk("fsmd");
#endif
#ifdef RTCONFIG_WIREGUARD
	stop_wgsall();
#endif
#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
	stop_detect_plc();
#endif
#ifdef RTCONFIG_AMAS_ADTBW
	stop_amas_adtbw();
#endif
#ifdef RTCONFIG_NFCM
	stop_nfcm();
#endif
#ifdef RTCONFIG_FPROBE
	stop_fprobe();
#endif
#if defined(RTCONFIG_AMAS)
	stop_amas_lib();
#endif
	stop_ntpd();

#ifdef RTCONFIG_ADTBW
	stop_adtbw();
#endif
#if !(defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK) || defined(RTCONFIG_REALTEK)) \
 ||  (defined(RTCONFIG_SOC_IPQ8074))
	stop_erp_monitor();
#endif
#ifdef RTCONFIG_INTERNAL_GOBI
	stop_lteled();
#endif
#ifdef RTCONFIG_CONNTRACK
	stop_pctime_service();
#endif
#if defined(RTCONFIG_BWDPI)
	stop_hour_monitor_service();
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
	restart_nas_services(1, 0, 0);
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
	if (nvram_get_int("re_mode") == 1) {
#ifdef RTCONFIG_BHCOST_OPT
		stop_amas_status();
		stop_amas_misc();
        stop_amas_ssd();
#endif
		stop_amas_bhctrl();
#ifndef RTCONFIG_FRONTHAUL_DWB
		stop_amas_lanctrl();
#endif
		stop_amas_wlcconnect();
	}
#ifdef RTCONFIG_FRONTHAUL_DWB
	stop_amas_lanctrl();
#endif
	stop_amas_lldpd();
#ifdef RTCONFIG_LANTIQ
	stop_obd();
#endif
#ifdef RTCONFIG_ETHOBD
	stop_eth_obd();
#endif
#endif
	//stop_check_watchdog();
	stop_watchdog();
    stop_alt_watchdog(); // add by Andrew

#if defined(CONFIG_BCMWL5) && defined(RTCONFIG_FANCTRL)
	stop_phy_tempsense();
#endif
#ifdef RTCONFIG_BCMWL6
	stop_igmp_proxy();
#ifdef RTCONFIG_AMAS
	stop_obd();
#ifdef RTCONFIG_ETHOBD
	stop_eth_obd();
#endif
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
#ifdef RTCONFIG_MULTILAN_CFG
	stop_dnsmasq(ALL_SDN);
#else
	stop_dnsmasq();
#endif
#if defined(RTCONFIG_MDNS)
	stop_mdns();
#endif
#ifdef RTCONFIG_IPV6
	stop_rdisc6();
#endif
#ifdef RTCONFIG_BCMWL6
#ifdef BCM_ASPMD
	stop_aspmd();
#endif
#if defined(RTCONFIG_DHDAP) || defined(RTCONFIG_HND_ROUTER_AX)
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
#ifdef BCM_CEVENTD
	stop_ceventd();
#endif
#ifdef BCM_CEVENTD
	stop_ceventd();
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
#if defined(RTCONFIG_HAPDEVENT)
	stop_hapdevent();
#endif
#ifdef RTCONFIG_DNSQUERY_INTERCEPT
	stop_dnsqd();
#endif
#ifdef RTCONFIG_ROUTERBOOST
	stop_asus_rbd();
#endif
#ifdef RTCONFIG_AWSIOT
	stop_awsiot();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
	stop_wlc_monitor();
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
#ifdef RTCONFIG_AHS
	stop_ahs();
#endif /* RTCONFIG_AHS */
#ifdef RTCONFIG_CFGSYNC
#ifdef RTCONFIG_CONNDIAG
	stop_conn_diag();
#endif
	stop_cfgsync();
#endif
#ifdef RTCONFIG_NEW_USER_LOW_RSSI
	stop_roamast();
#endif
#if defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(GTBE19000) || defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE92U) || defined(RTBE95U)
	stop_rtkmonitor();
#endif
#ifdef GTAX6000
	stop_antled();
#endif
#if defined(RTAX82U) || defined(DSL_AX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTAX6000) || defined(GT10) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(GTBE96) || defined(GTBE19000)
	stop_ledg();
	stop_ledbtn();
#endif
#ifdef RTCONFIG_TCPLUGIN
	stop_qmacc();
#endif
#ifdef RTCONFIG_BCM_OAM
	stop_oam();
#endif
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
	stop_envrams();
#endif
#ifdef RTCONFIG_SCHED_DAEMON
	stop_sched_daemon();
#endif
#if 0
#ifdef HND_ROUTER
	stop_jitterentropy();
#endif /* HND_ROUTER */
#ifdef RTCONFIG_MULTIWAN_IF
	// stop_mtwanduck();
#endif

#ifdef RTCONFIG_MTK_BSD
	stop_mtk_bs20();
#endif	
#else
	stop_haveged();
#endif
}

#if 0	// #ifdef_HND_ROUTER
void start_jitterentropy()
{
	pid_t pid;
	char *cmd_argv[] = { "/usr/sbin/jitterentropy-rngd",
								"-p", "/var/run/jitterentropy-rngd.pid",
								NULL};
	_eval(cmd_argv, NULL, 0, &pid);
}

void stop_jitterentropy()
{
	pid_t pid;
	char *cmd_argv[] = { "killall", "jitterentropy-rngd", NULL};
	_eval(cmd_argv, NULL, 0, &pid);
}
#else
void start_haveged()
{
	pid_t pid;
	char *cmd_argv[] = { "/usr/sbin/haveged",
	                     "-r", "0",
	                     "-w", "1024",
#if 1	// All supported models use 32 KB so far
	                     "-d", "32",
	                     "-i", "32",
#endif
	                     NULL };

	_eval(cmd_argv, NULL, 0, &pid);
}

void stop_haveged()
{
	if (pids("haveged"))
		killall_tk("haveged");
}
#endif

#ifdef RTCONFIG_QCA
int stop_wifi_service(void)
{
   	int is_unload;

	deinit_all_vaps(0);

#if defined(RTCONFIG_WIRELESSREPEATER)
#if defined(RTCONFIG_CONCURRENTREPEATER)
	kill_wifi_wpa_supplicant(-1);
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
#ifdef RTCONFIG_ASD
	stop_asd();
#endif
#ifdef GTAX6000
	stop_antled();
#endif
#if defined(RTAX82U) || defined(DSL_AX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTAX6000) || defined(GT10) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(GTBE96) || defined(GTBE19000)
	stop_ledg();
	stop_ledbtn();
#endif
#ifdef RTCONFIG_USB
#ifdef RTCONFIG_DISK_MONITOR
	stop_diskmon();
#endif
#ifdef RTCONFIG_USB_PRINTER
	stop_lpd();
	stop_u2ec();
#endif
#ifdef RTCONFIG_CONNTRACK
	stop_pctime_service();
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
#ifdef RTCONFIG_BCM_MFG
#ifdef RTCONFIG_MULTILAN_CFG
	stop_dnsmasq(ALL_SDN);
#else
	stop_dnsmasq();
#endif
#endif
#ifdef RTCONFIG_MFGFW
	if(nvram_match("mfgfw", "1"))
	{
#ifdef RTCONFIG_MULTILAN_CFG
		stop_dnsmasq(ALL_SDN);
#else
		stop_dnsmasq();
#endif
	}
#endif
#if defined(RTCONFIG_MDNS)
	stop_mdns();
#endif
	stop_upnp();
	stop_lltd();
	stop_snooper();
#if defined (RTCONFIG_AMAS) && (defined(RTCONFIG_BCMWL6) || defined(RTCONFIG_LANTIQ) || defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK) || defined(RTCONFIG_RALINK))
#if defined(RTCONFIG_ETHOBD)
	stop_eth_obd();
#endif
	stop_obd();
#endif
#ifdef RTCONFIG_BCMWL6
#ifdef BCM_ASPMD
	stop_aspmd();
#endif
#if 0
#if defined(RTCONFIG_DHDAP) || defined(RTCONFIG_HND_ROUTER_AX)
	stop_dhd_monitor();
#if defined(RTCONFIG_HND_ROUTER_AX)
	killall_tk("debug_monitor");
#else
	killall_tk("dhd_monitor");
#endif
#endif
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
#ifdef BCM_CEVENTD
	stop_ceventd();
#endif
#ifdef BCM_BSD
	stop_bsd();
#endif
	stop_igmp_proxy();
#if defined(BCA_HNDROUTER) && defined(MCPD_PROXY)
	stop_mcpd_proxy();
#endif
#ifdef RTCONFIG_HSPOT
	stop_hspotap();
#endif
#endif
#if defined(RTCONFIG_WLCEVENTD)
	stop_wlceventd();
#endif
#if defined(RTCONFIG_HAPDEVENT)
	stop_hapdevent();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
	stop_wlc_monitor();
	stop_wlc_nt();
#endif
#ifdef RTCONFIG_WPS
	stop_wpsaide();
#endif
	stop_wps();

	stop_wanduck();
	//stop_logger();
#if defined(RTCONFIG_BWDPI)
	stop_bwdpi_check();
#endif
#ifdef RTCONFIG_NTPD
	stop_ntpd();
#else
	killall_tk("ntp");
	stop_ntpc();
#endif
	stop_udhcpc(-1);
	platform_start_ate_mode();
#ifdef RTCONFIG_QCA_PLC_UTILS
	ate_ctl_plc_led();
#endif
#ifdef SW_DEVLED
#ifdef RTCONFIG_HND_ROUTER_AX
	if(nvram_match("asus_mfg", "1"))
#endif
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
	stop_amas_lib();
	stop_amas_wlcconnect();
	stop_amas_bhctrl();
	stop_amas_lanctrl();
	stop_amas_lldpd();
#ifdef RTCONFIG_BHCOST_OPT
    stop_amas_ssd();
	stop_amas_misc();
	stop_amas_status();
#endif
#ifdef RTCONFIG_CONNDIAG
	stop_conn_diag_ss();
#endif
#endif
#ifdef RTCONFIG_WIRELESSREPEATER
	stop_wlcconnect();
#endif

#ifdef RTCONFIG_QCA
	stop_wifi_service();
#endif
#ifdef RTCONFIG_SYSSTATE
	stop_sysstate();
#endif
#ifdef RTCONFIG_AHS
	stop_ahs();
#endif /* RTCONFIG_AHS */
#ifdef RTCONFIG_PROTECTION_SERVER
	stop_ptcsrv();
#endif
#if !(defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK) || defined(RTCONFIG_REALTEK)) \
 ||  (defined(RTCONFIG_SOC_IPQ8074))
	stop_erp_monitor();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
	stop_notification_center();
#endif
#ifdef RTCONFIG_TUNNEL
	stop_mastiff();
#endif
#ifdef RTCONFIG_DNSQUERY_INTERCEPT
        stop_dnsqd();
#endif
#ifdef RTCONFIG_NFCM
	stop_nfcm();
#endif
#ifdef RTCONFIG_NEW_USER_LOW_RSSI
	stop_roamast();
#endif
#ifdef RTCONFIG_GETREALIP
	killall_tk("getrealip.sh");
	killall_tk("ministun");
#endif
#ifdef RTCONFIG_NETOOL
	stop_netool();
#endif
#ifdef HND_ROUTER
	//stop_jitterentropy();
#endif
#ifdef RTCONFIG_CONNDIAG
	stop_conn_diag();
#endif
#ifdef RTCONFIG_AWSIOT
	stop_awsiot();
#endif
#ifdef RTCONFIG_CONNDIAG
	stop_amas_portstatus();
#endif
#ifdef SW_DEVLED
	if (pids("sw_devled"))
		killall_tk("sw_devled");
#endif
#ifdef RTCONFIG_NBR_RPT
	if (pids("nbr_monitor"))
		killall_tk("nbr_monitor");
#endif
#ifdef RTCONFIG_SCHED_DAEMON
	stop_sched_daemon();
#endif
#ifdef RTCONFIG_FSMD
	if (pids("fsmd"))
		killall_tk("fsmd");
#endif
//#if defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(GTBE19000) || defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE92U) || defined(RTBE95U)
#if 0
	stop_rtkmonitor();
	eval("ebtables", "-t", "broute", "-F");
#endif
#if defined(RTCONFIG_USB_MODEM) \
	&& (defined(RTCONFIG_HND_ROUTER) \
	|| defined(RTCONFIG_QCA) \
	|| defined(RTCONFIG_MT798X))
#ifndef RTCONFIG_BCM4906_504
	if (pids("usbmuxd"))
		killall_tk("usbmuxd");
#endif
#endif
#ifdef RTCONFIG_GEARUPPLUGIN
	stop_gu_service(gu_enable_status());
#endif
}

// 2008.10 magic

int no_need_to_start_wanduck()
{
#if 0
	int sw_mode = sw_mode();
	if(sw_mode != SW_MODE_ROUTER && sw_mode != SW_MODE_REPEATER)
		return 1;
#endif

#ifdef RTCONFIG_BCM_MFG
	return 1;
#endif

#ifdef RTCONFIG_MFGFW
	if(nvram_match("mfgfw", "1"))
		return 1;
#endif

	if(ate_factory_mode())
		logmessage("wanduck", "WARNING - router is in manufacturing mode, and can behave unexpectedly (did you mess with your bootloader?)");
		//return 1;

	if(!strcmp(nvram_safe_get("wanduck_down"), "1"))
		return 1;

#if defined(RPAX56) || defined(RPAX58)
	if(!nvram_match("x_Setting", "0"))
		return 1;
#endif
#ifdef RTCONFIG_MODEM_BRIDGE
	if(sw_mode() == SW_MODE_AP && nvram_get_int("modem_bridge"))
		return 1;
#endif

	return 0;
}

int start_wanduck(void)
{
	char *argv[] = {"/sbin/wanduck", NULL};
	pid_t pid;

	if (no_need_to_start_wanduck())
		return 0;

	return _eval(argv, NULL, 0, &pid);
}

void stop_wanduck(void)
{
	killall("wanduck", SIGTERM);
}

#ifdef RTCONFIG_MULTIWAN_IF
int start_mtwanduck(void)
{
	char *argv[] = {"/sbin/mtwanduck", NULL};
	pid_t pid;

#ifdef RTCONFIG_BCM_MFG
	return 0;
#endif

#ifdef RTCONFIG_MFGFW
	if(nvram_match("mfgfw", "1"))
		return 0;
#endif

	if(ate_factory_mode())
		return 0;

	if(!strcmp(nvram_safe_get("mtwanduck_down"), "1"))
		return 0;

#ifdef RTCONFIG_MODEM_BRIDGE
	if(sw_mode() == SW_MODE_AP && nvram_get_int("modem_bridge"))
		return 0;
#endif
#ifdef RTCONFIG_DSL ///TODO
	nvram_set("freeze_duck", "0");
#endif

	return _eval(argv, NULL, 0, &pid);
}

void stop_mtwanduck(void)
{
	killall("mtwanduck", SIGTERM);
}
#endif

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

// add by Andrew
void
stop_alt_watchdog(void)
{
	killall_tk("alt_watchdog");
}
// end of add

void
stop_check_watchdog(void)
{
	//killall_tk("check_watchdog");
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

#if defined(CONFIG_BCMWL5) && !defined(HND_ROUTER) && defined(RTCONFIG_DUALWAN)
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

	if(nvram_match("stop_watchdog", "1"))  return 0;

	return _eval(watchdog_argv, NULL, 0, &whpid);
}

int
start_check_watchdog(void)
{
	return 0;
/*
	char *check_watchdog_argv[] = {"check_watchdog", NULL};
	pid_t pid;

	if(nvram_match("stop_watchdog", "1"))  return 0;

	return _eval(check_watchdog_argv, NULL, 0, &pid);
*/
}

// added by Andrew
int start_alt_watchdog(void)
{
	char *alt_watchdog_argv[] = {"alt_watchdog", NULL};
	pid_t whpid;

	if(nvram_match("stop_watchdog", "1"))  return 0;

	return _eval(alt_watchdog_argv, NULL, 0, &whpid);
}
// end of add

int
start_fwupg_flashing(void)
{
	char *the_argv[] = {"fwupg_flashing", NULL};
	pid_t pid;

	return _eval(the_argv, NULL, 0, &pid);
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

#if defined(RTAX95Q) || defined(XT8PRO) || defined(BT12) || defined(BT10) || defined(BQ16) || defined(BQ16_PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56U) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(CTAX56_XD4) || defined(ET12) || defined(XT12) || defined(XC5) || defined(EBA63)
	return 1;
#endif

#ifdef RTCONFIG_MFGFW
	if(nvram_match("mfgfw", "1"))
		return -1;
#endif

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

#ifdef RTCONFIG_DSL_REMOTE
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

#if defined(CONFIG_BCMWL5) && defined(RTCONFIG_FANCTRL)
int stop_phy_tempsense(void)
{
	if (pids("phy_tempsense")) {
		killall_tk("phy_tempsense");
	}
	return 0;
}

int start_phy_tempsense(void)
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

#if defined(RTCONFIG_AMAS)
	if (nvram_get_int("re_mode") == 1) {
		_dprintf("It is RE mode, don't start psta_monitor\n");
		return 0;
	}
#endif

	return _eval(psta_monitor_argv, NULL, 0, &pid);
}
#endif

#if defined(RTCONFIG_AMAS) && (defined(RTCONFIG_BCMWL6) || defined(RTCONFIG_LANTIQ) || defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK) || defined(RTCONFIG_RALINK))
void
stop_obd(void)
{
	if(getpid()!=1) {
		notify_rc("stop_obd");
		return;
	}

	if (pids("obd"))
		killall_tk("obd");
}

void
start_obd(void)
{
	char *obd_argv[] = {"obd", NULL};
	pid_t pid;

	if (no_need_obd() == -1)
		return;

	stop_obd();

	if(getpid()!=1) {
		notify_rc("start_obd");
		return;
	}

#ifdef CONFIG_BCMWL5
	if (!restore_defaults_g)
#endif
	_eval(obd_argv, NULL, 0, &pid);
}
#endif

#if defined(RTCONFIG_UPLOADER)
static void uploader_config(void)
{

	// /* uploader directory */
	mkdir_if_none("/tmp/diag_db_cloud");
	// chmod("/tmp/diag_db_cloud", 0777);
	mkdir_if_none("/tmp/diag_db_cloud/upload");
	chmod("/tmp/diag_db_cloud/upload", 0777);
	mkdir_if_none("/tmp/diag_db_cloud/download");
	chmod("/tmp/diag_db_cloud/download", 0777);


	FILE *fp;

	fp = fopen("/tmp/uploader_conf", "w");

	if(fp != NULL) {

		fprintf(fp, "%s", "upload_dir:/tmp/diag_db_cloud/upload\n");
		fprintf(fp, "%s", "download_dir:/tmp/diag_db_cloud/download");
		fclose(fp);
	}

}

void stop_uploader(void)
{
	if(getpid()!=1) {
		notify_rc("stop_uploader");
		return;
	}

	if (pids("uploader"))
		killall_tk("uploader");
}


void start_uploader(void)
{

	printf("start_uploader\n");

	uploader_config();

	char *uploader_argv[] = {"uploader", "/tmp/uploader_conf", NULL};

	pid_t pid;
	
	if(getpid()!=1) {
		notify_rc("start_uploader");
		return;
	} 

	if (!pids("uploader")) {
		_eval(uploader_argv, NULL, 0, &pid);
	}
}
#endif


#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_ETHOBD)
void start_obd_monitor(void)
{
        char *obd_monitor_argv[] = {"obd_monitor", NULL};
        pid_t pid;

        if(getpid()!=1) {
                notify_rc("start_obd_monitor");
                return;
        }

        killall("obd_monitor", SIGTERM);

        _eval(obd_monitor_argv, NULL, 0, &pid);
}

void stop_obd_monitor(void)
{
        if(getpid()!=1) {
                notify_rc("stop_obd_monitor");
                return;
        }

        killall("obd_monitor", SIGTERM);
}

void stop_eth_obd(void)
{
	if (pids("obd_eth"))
		killall_tk("obd_eth");
}

void start_eth_obd(void)
{
	char *obdeth_argv[] = {"obd_eth", NULL};
	pid_t pid;

	if (no_need_obdeth() == -1)
		return;

	stop_eth_obd();

#ifdef CONFIG_BCMWL5
	if (!restore_defaults_g)
#endif
	_eval(obdeth_argv, NULL, 0, &pid);
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

#ifdef RTCONFIG_BCM_MFG
	return 0;
#endif
#if defined(EBG15) || defined(EBG19)
	return 0;
#endif

#ifdef RTCONFIG_MFGFW
	if(nvram_match("mfgfw", "1"))
		return 0;
#endif
#ifdef RTAC68U
	if (!hw_usb_cap())
		return 0;
#endif

	if(nvram_match("stop_usbled", "1"))
		return 0;

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
	eval("crond", "-l", "9");
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
	int declen = strlen(zebra_passwd);
	char *dec_passwd = NULL;
	dec_passwd = malloc(declen);
	if(dec_passwd){
		pw_dec(zebra_passwd, dec_passwd, declen, 1);
		zebra_passwd = dec_passwd;
	}

	int declen2 = strlen(zebra_enpasswd);
	char *dec_passwd2 = NULL;
	dec_passwd2 = malloc(declen2);
	if(dec_passwd2){
		pw_dec(zebra_enpasswd, dec_passwd2, declen2, 1);
		zebra_enpasswd = dec_passwd2;
	}

	int declen3 = strlen(rip_passwd);
	char *dec_passwd3 = NULL;
	dec_passwd3 = malloc(declen3);
	if(dec_passwd3){
		pw_dec(rip_passwd, dec_passwd3, declen3, 1);
		rip_passwd = dec_passwd3;
	}
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
#ifdef RTCONFIG_NVRAM_ENCRYPT
	if(dec_passwd) free(dec_passwd);
	if(dec_passwd2) free(dec_passwd2);
	if(dec_passwd3) free(dec_passwd3);
#endif
	return 0;
}
#endif

#if defined(RTCONFIG_RGBLED)
#include <aura_rgb.h>
void start_aurargb(void)
{
	RGB_LED_STATUS_T rgb_cfg;

        if (
#ifdef CONFIG_BCMWL5
		ATE_BRCM_FACTORY_MODE() &&
#else
		ate_factory_mode() &&
#endif
		nvram_match("sb_flash_update", "0")
        )
		return;

	if (inhibit_led_on() || !nvram_get_int("aurargb_enable")) {
		memset(&rgb_cfg, 0x00, sizeof(rgb_cfg));
		aura_rgb_led(ROUTER_AURA_SET, &rgb_cfg, 0, 0);
#if defined(GTAC2900)
		nvram_set("pause_aura_rgb_nt", "1");
#endif
	}
#if defined(RTCONFIG_AURASYNC)
	else if (nvram_get_int("aurasync_enable") &&
		nvram_get_int("aurasync_set") && nv_to_rgb("aurasync_val", &rgb_cfg) == 0)
		aura_rgb_led(ROUTER_AURA_SET, &rgb_cfg, 0, 0);
#endif
	else if (nv_to_rgb("aurargb_val", &rgb_cfg) == 0){
		aura_rgb_led(ROUTER_AURA_SET, &rgb_cfg, 0, 0);
#if defined(GTAC2900)
		nvram_set("pause_aura_rgb_nt", "0");
#endif
	}

#if defined(RTCONFIG_TURBO_BTN)
	if (nvram_get_int("turbo_mode") == BOOST_AURA_RGB_SW ||
	    nvram_get_int("turbo_mode") == BOOST_AURA_SHUFFLE_SW) {
		boost_led_control(nvram_match("aurargb_enable", "1")? LED_ON : LED_OFF);
	}
#endif
}
#endif

void restore_config_before_firmware_downgrade()
{
#ifdef RTCONFIG_MSSID_PRELINK
	restore_prelink_config();
#endif
}

#if defined(RTAX82U) || defined(DSL_AX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTAX6000) || defined(GT10) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(GTBE96) || defined(GTBE19000)
int
start_ledg(void)
{
	char *ledg_argv[] = {"ledg", NULL};
	pid_t pid;
#ifdef RTCONFIG_BCM_MFG
	return 0;
#endif

	if (getpid() != 1) {
		notify_rc_and_wait_2min("start_ledg");
		return;
	}

	stop_ledg();
	return _eval(ledg_argv, NULL, 0, &pid);
}

int
stop_ledg(void)
{
	if (getpid() != 1) {
		notify_rc_and_wait_2min("stop_ledg");
		return;
	}

	if (pids("ledg")) {
		if (nvram_get_int("asus_mfg")) {
			nvram_set_int("ledg_scheme_tmp", 0);
			kill_pidfile_s("/var/run/ledg.pid", SIGTSTP);
		}

		killall_tk("ledg");
	}

	return 0;
}

int
start_ledbtn(void)
{
	char *ledbtn_argv[] = {"ledbtn", NULL};
	pid_t pid;
#ifdef RTCONFIG_BCM_MFG
	return 0;
#endif
	stop_ledbtn();
	return _eval(ledbtn_argv, NULL, 0, &pid);
}

int
stop_ledbtn(void)
{
	if (pids("ledbtn"))
		killall_tk("ledbtn");

	return 0;
}
#endif

#if defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(GTBE19000) || defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE92U) || defined(RTBE95U)
int
stop_rtkmonitor(void)
{
	if (pids("rtkmonitor"))
		killall_tk("rtkmonitor");

	return 0;
}

int
start_rtkmonitor(void)
{
	char *rtkmonitor_argv[] = {"rtkmonitor", NULL};
	pid_t pid;

	stop_rtkmonitor();
	if (hnd_boardid_cmp("GT-BE98_BCM") == 0)
		return 0;
	else
		return _eval(rtkmonitor_argv, NULL, 0, &pid);
}
#endif

#if defined(RTCONFIG_SCHED_DAEMON)
void
start_sched_daemon(void)
{
	char *sched_daemon_argv[] = {"sched_daemon", NULL};
	pid_t pid;

	if(getpid()!=1) {
		notify_rc("start_sched_daemon");
		return;
	}

	stop_sched_daemon();
	_eval(sched_daemon_argv, NULL, 0, &pid);
}

void
stop_sched_daemon(void)
{
	if(getpid()!=1) {
		notify_rc("stop_sched_daemon");
		return;
	}

	if (pids("sched_daemon"))
		killall_tk("sched_daemon");
	return;
}
#endif

#ifdef GTAX6000
int
start_antled(void)
{
	char *antled_argv[] = {"antled", NULL};
	pid_t pid;
#ifdef RTCONFIG_BCM_MFG
	return 0;
#endif
	stop_antled();
	return _eval(antled_argv, NULL, 0, &pid);
}

int
stop_antled(void)
{
	if (pids("antled"))
		killall_tk("antled");

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

/* init_x_Setting bounds to amas_lldpd */
static void _check_init_x(void (*func)(void))
{
	if(!(init_x_Setting == 0 && nvram_get_int("x_Setting") == 1))
		return;

	func();

	init_x_Setting = 1;	
}

static void _monitor(void (*func)(void))
{
	func();
}

int no_need_watchdog()
{
	if(nvram_match("stop_watchdog", "1"))  return -1;

	return pids("watchdog");
}

static void QOS_CONTROL()
{
#ifdef RTCONFIG_LANTIQ
	char ppa_cmd[255] = {0};
#endif

#if RTCONFIG_MULTIWAN_IF
	int i;
	for (i = MULTI_WAN_START_IDX; i < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; i++) {
		add_iQosRules(get_wan_ifname(mtwan_get_mapped_unit(i)));
	}
#else
	add_iQosRules(get_wan_ifname(wan_primary_ifunit()));
#endif	//RTCONFIG_MULTIWAN_IF
#if defined(RTCONFIG_BWDPI)
	start_dpi_engine_service();
#endif
	start_iQos();

#ifdef RTCONFIG_LANTIQ
	if (ppa_support(wan_primary_ifunit()) == 0) {
		disable_ppa_wan(get_wan_ifname(wan_primary_ifunit()));
		_dprintf("%s : remove ppa wan interface: %s\n", __FUNCTION__, ppa_cmd);
	}else{
		enable_ppa_wan(get_wan_ifname(wan_primary_ifunit()));
		_dprintf("%s : add ppa wan interface: %s\n", __FUNCTION__, ppa_cmd);
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
#ifdef RTCONFIG_CROND
	_check(pids("crond"), "crond", start_cron);
#endif
	_check(no_need_watchdog(), "watchdog", start_watchdog);

#ifdef RTCONFIG_AMAS
#if defined(RTCONFIG_BCMWL6) || defined(RTCONFIG_LANTIQ) || defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK) || defined(RTCONFIG_RALINK)
	_check(no_need_obd(), "obd", start_obd);
#endif
#if defined(RTCONFIG_ETHOBD)
	_check(no_need_obdeth(), "obd_eth", start_eth_obd);
#endif
	// To avoid deadlock, no need to use notify_rc("restart_amas_lldpd") here because the current function is called by process "init".
	_check_init_x(start_amas_lldpd);
#endif
#ifdef RTCONFIG_ASD
	_monitor(monitor_asd);
#endif
}

void factory_reset(void)
{
#ifdef RTCONFIG_CONCURRENTREPEATER
	nvram_set_int("led_status", LED_FACTORY_RESET);
#endif
	g_reboot = 1;
	f_write_string("/tmp/reboot", "1", 0, 0);
#ifdef RTCONFIG_REALTEK
/* [MUST] : Need to Clarify ... */
	set_led(LED_BLINK_SLOW, LED_BLINK_SLOW);
	nvram_commit();
#endif
#ifdef RTCONFIG_DSL_REMOTE
	system("adslate sysdefault &");
#endif
	stop_wan();
#ifdef RTCONFIG_MOCA
	stop_moca();
#endif
#ifdef RTCONFIG_USB_MODEM
	eval("/usr/sbin/modem_status.sh", "setband", "auto");
#endif

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
#ifdef RTCONFIG_CFGSYNC
#ifdef RTCONFIG_AMAS_CENTRAL_CONTROL
	eval("rm", "-rf", "/jffs/.sys/cfg_mnt");
#endif
#endif

#ifdef RTCONFIG_TRUSTZONE
	atee_clear_all();
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
	}
#ifdef RTCONFIG_QCA_PLC_UTILS
	reset_plc(1);
	eval("mtd-erase", "-d", "plc");
#endif

	nvram_set("sys_reboot_reason", "rc_func_fact_rst");
	kill(1, SIGTERM);
}

#ifdef RTCONFIG_DUAL_TRX
/* If RTCONFIG_DUAL_TRX2 is defined, bootcode just find first good
 * firmware and boot immediately.  Firmware may boot from 2nd partition.
 * In this case, we should write to 1st partition first.
 * @fwpart:
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 */
static int select_upgrade_fw_order(char *fwpart[2])
{
#if defined(RTCONFIG_DUAL_TRX2)
	if (!fwpart)
		return -1;

	if (get_active_fw_num() == 1) {
		/* 2nd FW is used to boot. Switch fwpart[]. */
		fwpart[0] = LINUX2_MTD_NAME;
		fwpart[1] = LINUX_MTD_NAME;
	}
#endif
	return 0;
}
#endif

#if defined(RTCONFIG_HTTPS)
/* Return string of specified X509V3 extension by NID.
 * NOTE: Caller must to release return value if it's not NULL!
 * @x509:
 * @nid:	e.g. NID_basic_constraints, NID_authority_key_identifier
 * @return:	pointer to string or NULL if @nid not found, malloc failed, wrong parameter, etc.
 */
static char *x509v3ext2str_by_nid(X509 *x509, int nid)
{
	char *ret = NULL;
	BUF_MEM *bptr = NULL;
	char *buf = NULL, *ptr;
	int loc;
	BIO *bio = NULL;
	X509_EXTENSION *ex;

	if (!x509)
		return NULL;

	loc = X509_get_ext_by_NID(x509, nid, -1);
	ex = X509_get_ext(x509, loc);
	if (!ex) {
		goto err_x509v3ext2str_by_nid;
	}
	bio = BIO_new(BIO_s_mem());
	if (!X509V3_EXT_print(bio, ex, 0, 0)) {
		dbg("%s: nid %d X509V3_EXT_print() fail!\n", __func__, nid);
		goto err_x509v3ext2str_by_nid;
	}
	BIO_flush(bio);
	BIO_get_mem_ptr(bio, &bptr);
	buf = malloc((bptr->length + 1) * sizeof(char));
	if (!buf) {
		goto err_x509v3ext2str_by_nid;
	}
	memcpy(buf, bptr->data, bptr->length);
	buf[bptr->length] = '\0';
	strtok_r(buf, "\r\n", &ptr);

	ret = buf;

err_x509v3ext2str_by_nid:
	if (bio)
		BIO_free(bio);

	return ret;
}

/* Select source of root/intermediate certificate or end-entity certificate,
 * and copy it to /etc/cacert.pem and /etc/cakey.pem or /etc/cert.pem respectively.
 * To keep latest selected certificates, save_cert() should be executed.
 * The function can't be porting to libshared unless it links against with
 * libssl/libcrypt for functions of Let's encrypt.
 * NOTE: We can't verify a intermediate certificate or a end-entity certificate
 *       without root certificate (and intermediate certificate if need) with
 *       openssl/mssl functions.
 * @return:
 * 	0:	HTTPD_CERT doesn't change
 *     <0:	error
 *     >0:	HTTPD_CERT changed
 */
int prepare_cert_in_etc(void)
{
	/* cert_ready = 1 if
	 * a. root/intermediate certificate ready. end-entity certificate will be signed if need.
	 * b. end-entity certificate ready if and only if uploaded certificate is end-entity certificate.
	 */
	int ret = 0, use_ca = 1, sign_srv_cert = 0, cert_ready = 0, first = 1, le_enable = 0;
	char *ca_subject_keyid = NULL, *c_keyid = NULL, *srv_issuer_keyid = NULL, *s_keyid = NULL;
	char o_md5[32 + 1] = "", n_md5[32 + 1] = "", cmd[sizeof("md5sum XXX" HTTPD_CERT)];
	FILE *fp;
	X509 *x509_ca = NULL, *x509_srv = NULL;

	if (!is_router_mode()) {
		/* always use self-generated cert. */
		return 0;
	}

	snprintf(cmd, sizeof(cmd), "md5sum %s", HTTPD_CERT);
	if (f_exists(HTTPD_CERT))
		exec_and_return_string(cmd, NULL, o_md5, sizeof(o_md5));

retry_prepare_cert_in_etc:
#if defined(RTCONFIG_LETSENCRYPT)
	le_enable = nvram_get_int("le_enable");
#else
	if ((f_exists(UPLOAD_CACERT) && f_exists(UPLOAD_CAKEY))
	 || (f_exists(UPLOAD_CERT) && f_exists(UPLOAD_KEY)))
		le_enable = 2;
	else
		le_enable = 0;
#endif

	if (le_enable ==1) {
#if defined(RTCONFIG_LETSENCRYPT)
		/* Let's encrypt */
		if (!is_le_cert(LE_HTTPD_CERT) || !cert_key_match(LE_HTTPD_CERT, LE_HTTPD_KEY)) {
			cp_le_cert(LE_FULLCHAIN, LE_HTTPD_CERT);
			cp_le_cert(LE_KEY, LE_HTTPD_KEY);
			use_ca = 0;
		}
		cert_ready = 1;
#endif
	} else if (le_enable == 2) {
		/* Uploaded cert. */
		if (f_exists(UPLOAD_CACERT) && f_exists(UPLOAD_CAKEY)) {
			/* uploaded certificate is root/intermediate certificate */
			if (!illegal_cert_and_key(UPLOAD_CACERT, UPLOAD_CAKEY)) {
				eval("cp", UPLOAD_CACERT, HTTPD_ROOTCA_CERT);
				eval("cp", UPLOAD_CAKEY, HTTPD_ROOTCA_KEY);
				if (f_exists(UPLOAD_GEN_CERT))
					eval("cp", UPLOAD_GEN_CERT, UL_HTTPD_CERT);
				if (f_exists(UPLOAD_GEN_KEY))
					eval("cp", UPLOAD_GEN_KEY, UL_HTTPD_KEY);
				cert_ready = 1;

				/* end-entity certificate will be signed by uploaded root/intermediate
				 * certificate and keep in /jffs/cert.tgz
				 */
				if (f_exists(UPLOAD_CERT))
					unlink(UPLOAD_CERT);
				if (f_exists(UPLOAD_KEY))
					unlink(UPLOAD_KEY);
			}
		} else if (f_exists(UPLOAD_CERT) && f_exists(UPLOAD_KEY)) {
			/* uploaded certificate is end-entity certificate */
			if (!illegal_cert_and_key(UPLOAD_CERT, UPLOAD_KEY)) {
				eval("cp", UPLOAD_CERT, UL_HTTPD_CERT);
				eval("cp", UPLOAD_KEY, UL_HTTPD_KEY);
				if (f_exists(UPLOAD_CACERT))
					unlink(UPLOAD_CACERT);
				if (f_exists(UPLOAD_CAKEY))
					unlink(UPLOAD_CAKEY);
				use_ca = 0;
				cert_ready = 1;
			}
		}
	} else {
		/* certificates that are generated by router itself. */
		if (f_exists(HTTPD_ROOTCA_GEN_CERT) && f_exists(HTTPD_ROOTCA_GEN_KEY)) {
			if (!illegal_cert_and_key(HTTPD_ROOTCA_GEN_CERT, HTTPD_ROOTCA_GEN_KEY)) {
				eval("cp", HTTPD_ROOTCA_GEN_CERT, HTTPD_ROOTCA_CERT);
				eval("cp", HTTPD_ROOTCA_GEN_KEY, HTTPD_ROOTCA_KEY);
				if (f_exists(HTTPD_GEN_CERT))
					eval("cp", HTTPD_GEN_CERT, HTTPD_CERT);
				if (f_exists(HTTPD_GEN_KEY))
					eval("cp", HTTPD_GEN_KEY, HTTPD_KEY);
				cert_ready = 1;
			}
		}
	}

	if (!cert_ready) {
		if (first && !nvram_match("le_enable", "0")) {
			first = 0;
			dbg("%s: No workable uploaded certificate, fallback to self-signed certificate.\n", __func__);
			remove_all_uploaded_cert_from_jffs();
			nvram_set("le_enable", "0");
			goto retry_prepare_cert_in_etc;
		} else {
			dbg("%s: Still no workable certificate, generate again.\n", __func__);
			sign_srv_cert = use_ca = 1;
		}
	}

	if (use_ca) {
		if (!sign_srv_cert && (!f_exists(HTTPD_CERT) || !f_exists(HTTPD_KEY)))
			sign_srv_cert  = 1;

		if (!sign_srv_cert) {
			/* If issuer of HTTPD_KEY is not HTTPD_ROOT_CA, sign new HTTPD_KEY. */
			if ((fp = fopen(HTTPD_ROOTCA_CERT, "r")) != NULL) {
				if (!PEM_read_X509(fp, &x509_ca, NULL, NULL)) {
					fseek(fp, 0, SEEK_SET);
					d2i_X509_fp(fp, &x509_ca);
				}
				fclose(fp);
				if (x509_ca) {
					c_keyid = ca_subject_keyid =
						x509v3ext2str_by_nid(x509_ca, NID_subject_key_identifier);
					if (ca_subject_keyid && !strncmp(ca_subject_keyid, "keyid:", 6))
						c_keyid = ca_subject_keyid + strlen("keyid:");
				}
			}
			if ((fp = fopen(HTTPD_CERT, "r")) != NULL) {
				if (!PEM_read_X509(fp, &x509_srv, NULL, NULL)) {
					fseek(fp, 0, SEEK_SET);
					d2i_X509_fp(fp, &x509_srv);
				}
				fclose(fp);
				if (x509_srv) {
					s_keyid = srv_issuer_keyid =
						x509v3ext2str_by_nid(x509_srv, NID_authority_key_identifier);
					if (srv_issuer_keyid && !strncmp(srv_issuer_keyid, "keyid:", 6))
						s_keyid = srv_issuer_keyid + strlen("keyid:");
				}
			}

			/* If issuer keyid of the end-entity certificate is not subject(=issuer) keyid
			 * of the root/intermediate certificate, it must be signed again.
			 */
			if (!c_keyid || !s_keyid || strncmp(c_keyid, s_keyid, strlen(c_keyid)))
				sign_srv_cert = 1;
			if (ca_subject_keyid)
				free(ca_subject_keyid);
			if (srv_issuer_keyid)
				free(srv_issuer_keyid);
			if (x509_ca)
				X509_free(x509_ca);
			if (x509_srv)
				X509_free(x509_srv);
		}

		if (sign_srv_cert) {
			/* If latest HTTPD_CERT is not signed by HTTPD_ROOTCA_CERT, sign new one. */
			GENCERT_SH();

			/* If uploaded certificate is used to sign end-entity certificate,
			 * copy it back to /jffs/.cert
			 */
			if (f_exists(UPLOAD_CACERT) && f_exists(UPLOAD_CAKEY)
#if defined(RTCONFIG_LETSENCRYPT)
			 && le_enable == 2
#endif
			) {
				eval("cp", HTTPD_CERT, UPLOAD_GEN_CERT);
				eval("cp", HTTPD_KEY, UPLOAD_GEN_KEY);
			}
		}
	}

	save_cert();
	if (f_exists(HTTPD_CERT))
		exec_and_return_string(cmd, NULL, n_md5, sizeof(n_md5));
	if (*o_md5 == '\0' || *n_md5 == '\0' || strcmp(o_md5, n_md5)) {
		dbg("%s: %s changed, MD5 [%s] -> [%s].\n", __func__, HTTPD_CERT, o_md5, n_md5);
		ret = 1;
	}

	return ret;
}
#endif	/* RTCONFIG_HTTPS */

void handle_notifications(void)
{
	char nv[256], nvtmp[32], *cmd[8], *script;
	char *nvp, *b, *nvptr, *actionstr;
	int action = 0;
	int count;
	int i;
	int wan_unit;
#ifdef RTCONFIG_USB_MODEM
	int modem_unit = 0;
	char tmp2[100], prefix2[32];
	char env_unit[32];
#endif
	char *fwpart[2] __attribute__((unused)) = { LINUX_MTD_NAME, LINUX2_MTD_NAME };

#if defined(RTCONFIG_LANTIQ)
	f_write_string("/proc/sys/vm/drop_caches", "1", 0, 0);
#endif
	// handle command one by one only
	// handle at most 7 parameters only
	// maximum rc_service strlen is 256
	strlcpy(nv, nvram_safe_get("rc_service"), sizeof(nv));
	nvptr = nv;
again:
	nvp = strsep(&nvptr, ";");
	//_dprintf("chk nv=%s, nvp=%s, rc_service=%s\n", nv, nvp, nvram_safe_get("rc_service"));

	memset(cmd, 0, sizeof(cmd));
	count = 0;
	while (count < ARRAY_SIZE(cmd) && (b = strtok_r(count ? NULL : nvp, " ", &nvp)) != NULL)
	{
		_dprintf("cmd[%d]=%s\n", count, b);
		cmd[count++] = b;
	}

	if (count == 0) {
		nvram_set("rc_service", "");
		return;
	}

	if(strncmp(cmd[0], "start_", 6)==0) {
		action |= RC_SERVICE_START;
		actionstr = "start";
		script = &cmd[0][6];
	}
	else if(strncmp(cmd[0], "stop_", 5)==0) {
		action |= RC_SERVICE_STOP;
		actionstr = "stop";
		script = &cmd[0][5];
	}
	else if(strncmp(cmd[0], "restart_", 8)==0) {
		action |= (RC_SERVICE_START | RC_SERVICE_STOP);
		actionstr = "restart";
		script = &cmd[0][8];
	}
	else {
		action = 0;
		actionstr = "";
		script = cmd[0];
	}

	TRACE_PT("running: %d %s\n", action, script);

	run_custom_script("service-event", 120, actionstr, script);

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
		if (wait_action_idle(10)) {
			set_action(ACT_REBOOT);
		}

		g_reboot = 1;
		f_write_string("/tmp/reboot", "1", 0, 0);

#ifdef RTCONFIG_HND_ROUTER_AX
		nvram_set_int("wlready", 0);
#endif
#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
		reset_plc(1);
		sleep(1);
#endif

		stop_wan();

#ifdef RTCONFIG_MOCA
	stop_moca();
#endif

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
		char prefix[PATH_MAX];

		snprintf(prefix, sizeof(prefix), "%s", nvram_safe_get("log_path"));

		eval("cp", "/tmp/syslog.log", "/tmp/syslog.log-1", prefix);
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
#if defined(RTCONFIG_MOCA)
		nvram_set_int("moca_dev_state", MOCA_STATE_UNKNOW);
#endif
		/* remove /jffs/.sys hidden folder */
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
		eval("touch", "/jffs/remove_hidden_flag");
#endif
#ifdef RTCONFIG_BCMARM
#ifdef RTCONFIG_ISP_CUSTOMIZE
		if (!is_customized())
#endif
		{
#ifdef HND_ROUTER
#if !defined(RTCONFIG_HND_ROUTER_AX_6756) && !defined(RTCONFIG_HND_ROUTER_BE_4916)
		mtd_erase_misc2();
#endif
#else
		umount("/jffs");
		system("mtd-erase2 brcmnand");
#endif
		}
#endif
		factory_reset();
	}
#if 0
#ifdef HND_ROUTER
	else if(strcmp(script, "jitterentropy") == 0)
	{
		if(action & RC_SERVICE_STOP)
		{
			stop_jitterentropy();
		}
		else if(action & RC_SERVICE_START)
		{
			start_jitterentropy();
		}
	}
#endif /* HND_ROUTER */
#endif
	else if(strcmp(script, "resetdefault") == 0)
	{
		factory_reset();
	}
	else if (strcmp(script, "all") == 0) {
#ifdef RTCONFIG_WIFI_SON
		if(sw_mode() != SW_MODE_REPEATER && nvram_match("wifison_ready", "1")) {
			action |= RC_SERVICE_STOP | RC_SERVICE_START;
			goto script_allnet;
		}
#endif
#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
		reset_plc(1);
#endif
#if defined(RTCONFIG_WISP)
		if(sw_mode() == SW_MODE_ROUTER)
		{	
			nvram_unset("wlc_band");
			nvram_commit();
		}	
#endif		
		nvram_set("sys_reboot_reason", "rc_all");
		sleep(2); // wait for all httpd event done
		stop_lan_port();
		start_lan_port(6);
		kill(1, SIGTERM);
	}
	else if(strcmp(script, "upgrade_ate") == 0) {
		FILE *fp;
		int ate_upgrade_reboot;
		int ate_upgrade_reset;
		int ate_upgrade_bootnew;
		char upgrade_file[64] = "/tmp/linux.trx";

		int stop_commit;
		stop_commit = nvram_get_int(ASUS_STOP_COMMIT);
		if(stop_commit == 0) {
			nvram_set(ASUS_STOP_COMMIT, "1");	/* NOT TO COMMIT when FW Writting */
			wait_action_idle(10);
		}

		g_upgrade = 1;
		f_write_string("/tmp/upgrade", "1", 0, 0);

#ifdef RTCONFIG_SMALL_FW_UPDATE
		snprintf(upgrade_file,sizeof(upgrade_file),"/tmp/mytmpfs/linux.trx");
#endif

		if(cmd[1] == NULL)	ate_upgrade_reboot = 1;
		else			ate_upgrade_reboot = atoi(cmd[1]);
		if(cmd[2] == NULL)	ate_upgrade_reset = 0;
		else			ate_upgrade_reset = atoi(cmd[2]);
		if(cmd[3] == NULL)	ate_upgrade_bootnew = 0;
		else			ate_upgrade_bootnew = atoi(cmd[3]);
#if RTCONFIG_PIPEFW
		nvram_set_int("ate_upgrade_reboot", ate_upgrade_reboot);
		nvram_set_int("ate_upgrade_reset", ate_upgrade_reset);
#endif
		_dprintf("REBOOT = %d, RESET = %d, BOOTNEW = %d\n", ate_upgrade_reboot, ate_upgrade_reset, ate_upgrade_bootnew);
#ifdef CONFIG_BCMWL5
		if (0 && !factory_debug() && !nvram_match(ATE_UPGRADE_MODE_STR(), "1"))
#else
		if (0 && !IS_ATE_FACTORY_MODE() && !nvram_match(ATE_UPGRADE_MODE_STR(), "1"))
#endif
		{
			_dprintf("Only support under ATE test mode, Skip...\n");
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)
			 nvram_set_int("ate_upgrade_state", _ATE_FW_NOT_ATE);	
#else
			if ((fp = fopen("/tmp/ate_upgrade_state", "w")) != NULL) {
				fprintf(fp, "Not ATE Mode\n");
				fclose(fp);
			}
			else	_dprintf("Fail to open /tmp/ate_upgrade_state\n");

			if (f_exists(upgrade_file))
				unlink(upgrade_file);
#endif
			goto skip;
		}

		if(action & RC_SERVICE_STOP) {
			if ((fp = fopen("/tmp/ate_upgrade_state", "w")) != NULL) {
				fprintf(fp, "stop_upgarde_ate\n");
				fclose(fp);
			}
			else	_dprintf("Fail to open /tmp/ate_upgrade_state\n");

#ifdef RTCONFIG_WIRELESSREPEATER
			stop_wlcconnect();
#endif

#ifdef RTCONFIG_BWDPI
			stop_hour_monitor_service();
#endif

			// what process need to stop to free memory or
			// to avoid affecting upgrade
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

#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
			reset_plc(1);
#endif
#ifdef RTCONFIG_CFGSYNC
#ifdef RTCONFIG_CONNDIAG
			stop_conn_diag();
#endif
			stop_cfgsync();
#endif
#if defined(RTCONFIG_AMAS) && (defined(RTCONFIG_BCMWL6) || defined(RTCONFIG_LANTIQ) || defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK))
			stop_obd();
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_ETHOBD)
			stop_eth_obd();
#endif
		}
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)
		if((action & RC_SERVICE_START) && nvram_match("upgrade_done", "2"))
			goto skip;

		_dprintf(">> wait upgrading...\n");
		int i;
		for(i=0; i<120; ++i) {
			_dprintf("*");
			sleep(1);
			if(nvram_match("upgrade_done", "1")) {
				nvram_set("upgrade_done", "2");
				break;
			}
		}
		_dprintf("<< elapse upgrading-time: %d secs\n", i);

		if(ate_upgrade_bootnew || nvram_get_int("bootnew")==1) {
			_dprintf("[ATE] BOOTNEW...(%d)(%d)\n", ate_upgrade_bootnew, nvram_get_int("bootnew"));
                        if (setBootImageState(BOOT_SET_NEW_IMAGE) != 0)
                                _dprintf("%s: setBootImageState(BOOT_SET_NEW_IMAGE) failed..\n", __func__);
		}

		if(ate_upgrade_reset) {
			_dprintf("[ATE] Reset Default...\n");
			ResetDefault();
		}

		if(ate_upgrade_reboot) {
			nvram_set("sys_reboot_reason", "ate_upg_rbt");
			_dprintf("[ATE] REBOOT...\n");
			kill(1, SIGTERM);
		}

		goto skip;
#endif
		if(action & RC_SERVICE_START) {
#ifndef RTCONFIG_PIPEFW
			int sw = 0, r;

			if ((fp = fopen("/tmp/ate_upgrade_state", "w")) != NULL) {
				fprintf(fp, "start_upgarde_ate\n");
				fclose(fp);
			}
			else	_dprintf("Fail to open /tmp/ate_upgrade_state\n");
#endif
#ifdef RTCONFIG_DSL_REMOTE
#ifdef RTCONFIG_RALINK
			_dprintf("to do start_tc_upgrade\n");
			start_tc_upgrade();
#else
			do_upgrade_adsldrv();
#endif
#endif

			limit_page_cache_ratio(90);
#ifndef RTCONFIG_PIPEFW
			/* flash it if exists */
			if (f_exists(upgrade_file)) {
				pre_firmware_upgrade(upgrade_file);
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
#ifdef RTCONFIG_MULTILAN_CFG
				stop_dnsmasq(ALL_SDN);
#else
				stop_dnsmasq();
#endif
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
				/* If boot from 2nd firmware, write to 1st firmware first. */
				select_upgrade_fw_order(fwpart);
				if (!nvram_match("nflash_swecc", "1"))
				{
#if defined(RTCONFIG_FITFDT)		// don't write trx header(64 bytes) to flash
					{
						char header_size[20];
						snprintf(header_size, sizeof(header_size), "%d", get_imageheader_size());
						_dprintf(" Write FW to the inactive partition (%s) skip(%s)\n", fwpart[1], header_size);
						eval("mtd-write", "-i", upgrade_file, "-d", fwpart[1], "-s", header_size);
					}
#else
					_dprintf(" Write FW to the inactive partition (%s).\n", fwpart[1]);
					if (nvram_contains_word("rc_support", "nandflash"))	/* RT-AC56S,U/RT-AC68U/RT-N16UHP */
						eval("mtd-write2", upgrade_file, "linux2");
					else
						eval("mtd-write", "-i", upgrade_file, "-d", fwpart[1]);
#endif	/* RTCONFIG_FITFDT */
				}
#endif
#if defined(RTCONFIG_TEMPROOTFS)
				if(nvram_match("temproot_task", "upgrade")) {
					_dprintf("%s: hold upgrade....\n", __func__);
				} else {
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
#if defined(RTCONFIG_FITFDT)
					{
						char header_size[20];
						snprintf(header_size, sizeof(header_size)-1, "%d", get_imageheader_size());
						_dprintf("mtd-write and skip header_size(%s)\n", header_size);
						eval("mtd-write", "-i", upgrade_file, "-d", "linux", "-s", header_size);
					}
#else
#ifdef RTCONFIG_MULTIFW
					if (nvram_invmatch("trx_skip", ""))
						eval("mtd-write", "-i", upgrade_file, "-d", fwpart[0], "-s", nvram_safe_get("trx_skip"), "-c", nvram_safe_get("trx_count"));
					else
#endif
					eval("mtd-write", "-i", upgrade_file, "-d", fwpart[0]);
#endif /* RTCONFIG_FITFDT */
#endif /* RTCONFIG_REALTEK */
#endif // RTAC1200G
				    }
#if defined(RTCONFIG_TEMPROOTFS)
				}
#endif

				if ((fp = fopen("/tmp/ate_upgrade_state", "w")) != NULL) {
					fprintf(fp, "Upgarde Complete\n");
					fclose(fp);
				}
				else	_dprintf("Fail to open /tmp/ate_upgrade_state\n");

				/* erase trx and free memory on purpose */
#if defined(RTCONFIG_TEMPROOTFS)
				if(!nvram_match("temproot_task", "upgrade"))
#endif
				unlink(upgrade_file);

				if(ate_upgrade_reset) {
					_dprintf("[ATE] Reset Default...\n");
#ifndef HND_ROUTER
					nvram_set("restore_defaults", "1");
					nvram_set(ASUS_STOP_COMMIT, "1");
					stop_commit = 1;
#endif
					ResetDefault();
				}

				if (sw) {
					_dprintf("switch to temp rootfilesystem\n");
					if (!(r = switch_root(TMP_ROOTFS_MNT_POINT))) {
						/* Do nothing. If switch_root() success, never reach here. */
					} else {
						if(*nvram_safe_get("temproot_task")) {
							_dprintf("%s: hold reboot\n", __func__);
						} else if(ate_upgrade_reboot)	{
							nvram_set("sys_reboot_reason", "ate_upg_sw");
							_dprintf("[ATE] REBOOT...\n");
							kill(1, SIGTERM);
						}
					}
				} else {
					if(ate_upgrade_reboot) {
						nvram_set("sys_reboot_reason", "ate_upg_norm");
						_dprintf("[ATE] REBOOT...\n");
						kill(1, SIGTERM);
					}
				}
				if(stop_commit == 0)
					nvram_unset(ASUS_STOP_COMMIT);		/* FINISH FW Writting */
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
#endif
		}
	}
	else if(strcmp(script, "upgrade") == 0) {
		int stop_commit;
		restore_config_before_firmware_downgrade();
		stop_commit = nvram_get_int(ASUS_STOP_COMMIT);
		if(stop_commit == 0) {
			nvram_set(ASUS_STOP_COMMIT, "1");	/* NOT TO COMMIT when FW Writting */
			wait_action_idle(10);
		}
		if(action&RC_SERVICE_STOP) {
#ifdef RTCONFIG_RGBLED
			RGB_LED_STATUS_T rgb_cfg;
			__nv_to_rgb("0,0,0,6,-2,0", &rgb_cfg);
			aura_rgb_led(ROUTER_AURA_SET, &rgb_cfg, 0, 0);
#endif
			g_upgrade = 1;
			f_write_string("/tmp/upgrade", "1", 0, 0);
#ifdef RTCONFIG_HND_ROUTER_AX
			nvram_set_int("wlready", 0);
#endif
#ifdef RTCONFIG_WIRELESSREPEATER
			stop_wlcconnect();
#endif

#ifdef RTCONFIG_BWDPI
			stop_hour_monitor_service();
#endif
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
#endif
#ifdef RTCONFIG_TR069
			stop_tr();
#endif
			/* stop_jffs2(1); */ // do it later. Will add it to webs_upgrade 
#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
			reset_plc(1);
#endif
			stop_misc();

#if 0
#ifdef HND_ROUTER
			stop_jitterentropy();
#endif /* HND_ROUTER */
#endif
			// TODO free necessary memory here
			// Free kernel page cache
			system("echo 1 > /proc/sys/vm/drop_caches");
			sleep(2);
		}
		if(action & RC_SERVICE_START) {
#ifndef RTCONFIG_PIPEFW
			int sw = 0, r;
			char upgrade_file[64] = "/tmp/linux.trx";
			char *webs_state_info = nvram_safe_get("webs_state_info");

#ifdef RTCONFIG_SMALL_FW_UPDATE
			snprintf(upgrade_file,sizeof(upgrade_file),"/tmp/mytmpfs/linux.trx");
#endif

		#ifdef RTCONFIG_DSL_REMOTE
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
				pre_firmware_upgrade(upgrade_file);
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
#ifdef RTCONFIG_MULTILAN_CFG
				stop_dnsmasq(ALL_SDN);
#else
				stop_dnsmasq();
#endif
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
#ifdef RTCONFIG_BT_CONN
				stop_dbus_daemon();
#endif
				if (!(r = build_temp_rootfs(TMP_ROOTFS_MNT_POINT)))
					sw = 1;

				int chk_time = nvram_get_int("chk_time");
				if(sw == 1 && chk_time > 0) {
					_dprintf("%s, stop and chk new root for %d seconds.\n", __func__, chk_time);
					sleep(chk_time);
				}
#ifdef RTCONFIG_DUAL_TRX
				/* If boot from 2nd firmware, write to 1st firmware first. */
				select_upgrade_fw_order(fwpart);
				if (!nvram_match("nflash_swecc", "1"))
				{
#if defined(RTCONFIG_FITFDT)		// don't write trx header(64 bytes) to flash
					{
						char header_size[20];
						snprintf(header_size, sizeof(header_size), "%d", get_imageheader_size());
						_dprintf(" Write FW to the inactive partition (%s) skip(%s)\n", fwpart[1], header_size);
						eval("mtd-write", "-i", upgrade_file, "-d", fwpart[1], "-s", header_size);
					}
#else
					_dprintf(" Write FW to the inactive partition (%s).\n", fwpart[1]);
					if (nvram_contains_word("rc_support", "nandflash"))	/* RT-AC56S,U/RT-AC68U/RT-N16UHP */
						eval("mtd-write2", upgrade_file, "linux2");
					else
						eval("mtd-write", "-i", upgrade_file, "-d", fwpart[1]);
#endif	/* RTCONFIG_FITFDT */
				}
#endif

#if defined(RTCONFIG_TEMPROOTFS)
				if(nvram_match("temproot_task", "upgrade")) {
					_dprintf("%s: hold upgrade\n", __func__);
				} else {
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
					f_write_string("/proc/sys/vm/drop_caches", "3", 0, 0);
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
#if defined(RTCONFIG_FITFDT)
					{
						char header_size[20];
						snprintf(header_size, sizeof(header_size)-1, "%d", get_imageheader_size());
						_dprintf("mtd-write and skip header_size(%s)\n", header_size);
						eval("mtd-write", "-i", upgrade_file, "-d", "linux", "-s", header_size);
					}
#else
#ifdef RTCONFIG_MULTIFW
					if (nvram_invmatch("trx_skip", ""))
						eval("mtd-write", "-i", upgrade_file, "-d", fwpart[0], "-s", nvram_safe_get("trx_skip"), "-c", nvram_safe_get("trx_count"));
					else
#endif
					eval("mtd-write", "-i", upgrade_file, "-d", fwpart[0]);
#endif /* RTCONFIG_FITFDT */
#endif /* RTCONFIG_REALTEK */
#endif // RTAC1200G
				  }
#if defined(RTCONFIG_TEMPROOTFS)
				}
#endif

				/* erase trx and free memory on purpose */
#if defined(RTCONFIG_TEMPROOTFS)
				if(!nvram_match("temproot_task", "upgrade"))
#endif
				unlink(upgrade_file);
				if (sw) {
					_dprintf("switch to temp rootfilesystem..\n");
					if (!(r = switch_root(TMP_ROOTFS_MNT_POINT))) {
						/* Do nothing. If switch_root() success, never reach here. */
					} else {
						if(*nvram_safe_get("temproot_task")) {
							_dprintf("%s: hold reboot\n", __func__);
						} else {
							nvram_set("sys_reboot_reason", "rc_upg_switch");
							_dprintf("%s: go reboot.\n", __func__);
							kill(1, SIGTERM);
						}
					}
				} else {
					nvram_set("sys_reboot_reason", "rc_upg_noswitch");
					_dprintf("%s: go reboot.\n", __func__);
					kill(1, SIGTERM);
				}
			}
			else
#endif
			{
				nvram_set("sys_reboot_reason", "rc_upg_fw");
				_dprintf("%s: go reboot.\n", __func__);
				// recover? or reboot directly
				kill(1, SIGTERM);
			}
		}
		if(stop_commit == 0)
			nvram_unset(ASUS_STOP_COMMIT);	/* FINISH FW Writting */
	}
	else if(strcmp(script, "mfgmode") == 0) {
		nvram_set("asus_mfg", "2");
#ifdef RTCONFIG_ASD
		no_asd();
#endif
		stop_services_mfg();
	}
	else if(strcmp(script, "wltest") == 0) {
		nvram_set("asus_mfg", "3");
#ifdef RTCONFIG_ASD
		no_asd();
#endif
		//stop_check_watchdog();
		stop_watchdog();
        stop_alt_watchdog(); // add by Andrew
		stop_infosvr();
		stop_services_mfg();
	}
	else if(strcmp(script, "ethtest") == 0) {
		nvram_set("asus_mfg", "3");
#ifdef RTCONFIG_ASD
		no_asd();
#endif
		//stop_check_watchdog();
		stop_watchdog();
        stop_alt_watchdog(); // add by Andrew
		stop_infosvr();
		stop_services_mfg();
		modprobe_r("nf_nat_sip");
		modprobe_r("nf_conntrack_sip");
		modprobe_r("nf_nat_h323");
		modprobe_r("nf_conntrack_h323");
		modprobe_r("nf_nat_rtsp");
		modprobe_r("nf_conntrack_rtsp");
		modprobe_r("nf_nat_ftp");
		modprobe_r("nf_conntrack_ftp");
		modprobe_r("ip6table_mangle");
		modprobe_r("ip6t_LOG");
		modprobe_r("usblp");
#ifdef RTCONFIG_USB_CDROM
		modprobe_r("isofs");
		modprobe_r("udf");
#endif
		modprobe_r("thfsplus");
		modprobe_r("tntfs");
		modprobe_r("ext2");
		modprobe_r("ext4");
		modprobe_r("jbd2");
		modprobe_r("crc16");
		modprobe_r("ext3");
		modprobe_r("jbd");
		modprobe_r("mbcache");
		modprobe_r("cdc_mbim");
		modprobe_r("qmi_wwan");
		modprobe_r("cdc_wdm");
		modprobe_r("cdc_ncm");
		modprobe_r("rndis_host");
		modprobe_r("cdc_ether");
		modprobe_r("asix");
		modprobe_r("cdc_acm");
		modprobe_r("usbnet");
		modprobe_r("mii");
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
#ifdef RTCONFIG_MULTILAN_CFG
			stop_dnsmasq(ALL_SDN);
#else
			stop_dnsmasq();
#endif
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
#if defined(RTCONFIG_DHDAP) || defined(RTCONFIG_HND_ROUTER_AX)
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
#ifdef BCM_CEVENTD
			stop_ceventd();
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
#if defined(RTCONFIG_HAPDEVENT)
			stop_hapdevent();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
			stop_wlc_monitor();
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
#ifdef RTCONFIG_MOCA
			stop_moca();
#endif
#ifdef RTCONFIG_DSL_TCLINUX
			stop_dsl();
#endif
			stop_vlan();
#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_PRINTER)
			stop_lpd();
			stop_u2ec();
#endif
#ifdef RTCONFIG_CFGSYNC
#ifdef RTCONFIG_CONNDIAG
			stop_conn_diag();
#endif
			if (!strstr(nvram_safe_get("rc_service"), "start_cfgsync"))
				stop_cfgsync();
#endif
#ifdef RTCONFIG_CAPTIVE_PORTAL
			stop_chilli();
			stop_CP();
			stop_uam_srv();
#endif
#if defined(RTCONFIG_WIFI_SON)
		        if(nvram_match("wifison_ready","1"))
                		stop_amas_lib();
#endif


#if defined(RTCONFIG_AMAS)
#if defined(RTCONFIG_WIFI_SON)
		        if(nvram_match("wifison_ready","1"))
			{
                		stop_amas_lib();
#ifdef RTCONFIG_NEW_USER_LOW_RSSI
 			       	stop_roamast();
#endif
			}
#endif
#endif

#ifdef RTCONFIG_FBWIFI
			stop_fbwifi();
			stop_fbwifi_check();
			stop_fbwifi_config();
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
#ifdef RTCONFIG_MOCA
			start_moca();
#endif
#if defined(CONFIG_BCMWL5) && defined(RTCONFIG_DHDAP)
			start_wl();
			lanaccess_wl();
#endif
#if defined(RTCONFIG_RALINK) && defined(RTCONFIG_WLMODULE_MT7615E_AP)
			start_wds_ra();
#endif

#ifdef RTCONFIG_MULTILAN_CFG
			start_dnsmasq(ALL_SDN);
#else
			start_dnsmasq();
#endif

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
			start_wlc_monitor();
#endif
#if defined(RTCONFIG_WLCEVENTD)
			start_wlceventd();
#endif
#if defined(RTCONFIG_HAPDEVENT)
			start_hapdevent();
#endif
#ifdef RTCONFIG_BCMWL6
			start_igmp_proxy();
#ifdef BCM_BSD
			start_bsd();
#endif
#ifdef BCM_APPEVENTD
			start_appeventd();
#endif
#ifdef BCM_CEVENTD
			start_ceventd();
#endif
#ifdef BCM_SSD
			start_ssd();
#endif
#ifdef BCM_EVENTD
			start_eventd();
#endif
			start_acsd();
#if defined(RTCONFIG_DHDAP) || defined(RTCONFIG_HND_ROUTER_AX)
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
#ifndef RTCONFIG_DHDAP
			start_wl();
			lanaccess_wl();
#endif
#ifdef RTCONFIG_CAPTIVE_PORTAL
			start_chilli();
			start_CP();
			setup_passwd();
			start_uam_srv();
#ifdef RTCONFIG_MULTILAN_CFG
			handle_sdn_feature(ALL_SDN, SDN_FEATURE_ROUTING, RC_SERVICE_START);
#endif
#endif
#ifdef RTCONFIG_BCMWL6
#ifdef RTCONFIG_HSPOT
			start_hspotap();
#endif
#endif
#ifdef RTCONFIG_CFGSYNC
			if (!strstr(nvram_safe_get("rc_service"), "start_cfgsync"))
				start_cfgsync();
#ifdef RTCONFIG_CONNDIAG
			start_conn_diag();
#endif
#endif
#if defined(RTCONFIG_AMAS)
			start_amas_lib();
#ifdef RTCONFIG_VIF_ONBOARDING
			set_onboarding_vif_status();
#endif
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
#ifdef RTCONFIG_MULTILAN_CFG
			stop_dnsmasq(ALL_SDN);
#else
			stop_dnsmasq();
#endif
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
#if defined(RTCONFIG_DHDAP) || defined(RTCONFIG_HND_ROUTER_AX)
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
#ifdef BCM_CEVENTD
			stop_ceventd();
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
#if defined(RTCONFIG_HAPDEVENT)
			stop_hapdevent();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
			stop_wlc_monitor();
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
#ifdef RTCONFIG_CONNDIAG
			stop_conn_diag();
#endif
			if (!strstr(nvram_safe_get("rc_service"), "start_cfgsync"))
				stop_cfgsync();
#endif
			stop_wan();
			stop_lan();
#ifdef RTCONFIG_MOCA
			stop_moca();
#endif
 			//stop_vlan();

#ifdef RTCONFIG_CAPTIVE_PORTAL
			stop_chilli();
			stop_CP();
			stop_uam_srv();
#endif

#ifdef RTCONFIG_FBWIFI
			stop_fbwifi();
			stop_fbwifi_check();
			stop_fbwifi_config();
#endif
			// free memory here
		}
		if(action & RC_SERVICE_START) {
			//start_vlan();
			start_lan();
			update_srv_cert_if_lan_ip_changed();
#ifdef RTCONFIG_MOCA
			start_moca();
#endif
#if defined(CONFIG_BCMWL5) && defined(RTCONFIG_DHDAP)
			start_wl();
			lanaccess_wl();
#endif
#if defined(RTCONFIG_RALINK) && defined(RTCONFIG_WLMODULE_MT7615E_AP)
			start_wds_ra();
#endif

#ifdef RTCONFIG_MULTILAN_CFG
			start_dnsmasq(ALL_SDN);
#else
			start_dnsmasq();
#endif

#if defined(RTCONFIG_MDNS)
			start_mdns();
#endif
#ifdef RTCONFIG_DSL
			config_stb_bridge();
#endif
#if defined(RTCONFIG_DSL_BCM)
			nvram_set("freeze_duck", "15");
			dsl_wan_config(2);
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
			start_wlc_monitor();
#endif
#if defined(RTCONFIG_WLCEVENTD)
			start_wlceventd();
#endif
#if defined(RTCONFIG_HAPDEVENT)
			start_hapdevent();
#endif
#ifdef RTCONFIG_BCMWL6
			start_igmp_proxy();
#ifdef BCM_BSD
			start_bsd();
#endif
#ifdef BCM_APPEVENTD
			start_appeventd();
#endif
#ifdef BCM_CEVENTD
			start_ceventd();
#endif
#ifdef BCM_SSD
			start_ssd();
#endif
#ifdef BCM_EVENTD
			start_eventd();
#endif
			start_acsd();
#if defined(RTCONFIG_DHDAP) || defined(RTCONFIG_HND_ROUTER_AX)
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
			setup_passwd();
			start_uam_srv();
#ifdef RTCONFIG_MULTILAN_CFG
			handle_sdn_feature(ALL_SDN, SDN_FEATURE_ROUTING, RC_SERVICE_START);
#endif
#endif
#ifndef RTCONFIG_DHDAP
			start_wl();
			lanaccess_wl();
#endif
#ifdef RTCONFIG_BCMWL6
#ifdef RTCONFIG_HSPOT
			start_hspotap();
#endif
#endif
#ifdef RTCONFIG_CFGSYNC
			if (!strstr(nvram_safe_get("rc_service"), "start_cfgsync"))
				start_cfgsync();
#ifdef RTCONFIG_CONNDIAG
			start_conn_diag();
#endif
#endif
#if defined(RTCONFIG_AMAS)
			start_amas_lib();
#ifdef RTCONFIG_VIF_ONBOARDING
			set_onboarding_vif_status();
#endif
#endif
		}
	}
	else if (strcmp(script, "net_and_phy") == 0) {
	script_net_and_phy:
		if(action & RC_SERVICE_STOP) {
			sleep(2); // wait for all httpd event done

#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
			stop_plchost();
#endif
#ifdef RTCONFIG_MEDIA_SERVER
			force_stop_dms();
			stop_mt_daapd(0);
#endif
#if (defined(RTCONFIG_SAMBASRV) || defined(RTCONFIG_TUXERA_SMBD)) && defined(RTCONFIG_FTP)
			stop_ftpd(0);
			stop_samba(0);
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
#ifndef RTCONFIG_MULTILAN_CFG
			stop_dnsmasq();
#else
			stop_dnsmasq(ALL_SDN);
#endif
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
#if defined(RTCONFIG_DHDAP) || defined(RTCONFIG_HND_ROUTER_AX)
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
#ifdef BCM_CEVENTD
			stop_ceventd();
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
#if defined(RTCONFIG_HAPDEVENT)
			stop_hapdevent();
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
			stop_wlc_monitor();
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
#ifdef RTCONFIG_FBWIFI
			stop_fbwifi();
			stop_fbwifi_check();
			stop_fbwifi_config();
#endif
#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
			//stop_pptpd();
#endif
#if defined(RTCONFIG_NFCM)
            stop_nfcm();
#endif
#ifdef RTCONFIG_CFGSYNC
#ifdef RTCONFIG_CONNDIAG
			stop_conn_diag();
#endif
			if (!strstr(nvram_safe_get("rc_service"), "start_cfgsync"))
				stop_cfgsync();
#endif
#if defined(RTCONFIG_AMAS)
			stop_amas_lib();
#endif
			stop_wan();
			stop_lan();
#if defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(RTBE58U) || defined(TUFBE3600) || defined(GTBE19000) || defined(RTBE92U) || defined(RTBE95U)
			reset_extwan();
#endif
#ifdef RTCONFIG_MOCA
			stop_moca();
#endif
			//stop_vlan();
			stop_lan_port();
#ifdef RTCONFIG_MOCA
			stop_moca();
#endif
			// free memory here
		}
#ifdef RTCONFIG_LANTIQ
		if((action & RC_SERVICE_STOP) && (action & RC_SERVICE_START)) {
			if(client_mode())
				restart_wireless();
		}
#endif
		if(action & RC_SERVICE_START) {
#ifdef RTCONFIG_MULTIWAN_PROFILE
			init_switch();
#endif
			//start_vlan();
#if defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(RTBE58U) || defined(TUFBE3600) || defined(GTBE19000) || defined(RTBE92U) || defined(RTBE95U)
			config_extwan();
#endif
#if defined(CONFIG_BCMWL5) && defined(RTCONFIG_LACP)
			config_lacp();
#endif
			start_lan();
			update_srv_cert_if_lan_ip_changed();
#ifdef RTCONFIG_MOCA
			start_moca();
#endif
#if defined(CONFIG_BCMWL5) && defined(RTCONFIG_DHDAP)
			start_wl();
			lanaccess_wl();
#endif
#ifndef RTCONFIG_MULTILAN_CFG
			start_dnsmasq();
#else
			start_dnsmasq(ALL_SDN);
#endif
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
			start_wlc_monitor();
#endif
#if defined(RTCONFIG_WLCEVENTD)
			start_wlceventd();
#endif
#if defined(RTCONFIG_HAPDEVENT)
			start_hapdevent();
#endif
#ifdef RTCONFIG_BCMWL6
			start_igmp_proxy();
#ifdef BCM_BSD
			start_bsd();
#endif
#ifdef BCM_APPEVENTD
			start_appeventd();
#endif
#ifdef BCM_CEVENTD
			start_ceventd();
#endif
#ifdef BCM_SSD
			start_ssd();
#endif
#ifdef BCM_EVENTD
			start_eventd();
#endif
			start_acsd();
#if defined(RTCONFIG_DHDAP) || defined(RTCONFIG_HND_ROUTER_AX)
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

#if (defined(RTCONFIG_SAMBASRV) || defined(RTCONFIG_TUXERA_SMBD)) && defined(RTCONFIG_FTP)
			setup_passwd();
			start_samba();
			start_ftpd();
#endif
#ifndef RTCONFIG_DHDAP
			start_wl();
			lanaccess_wl();
#endif
#ifdef RTCONFIG_CAPTIVE_PORTAL
			start_chilli();
			start_CP();
			setup_passwd();
			start_uam_srv();
#ifdef RTCONFIG_MULTILAN_CFG
			handle_sdn_feature(ALL_SDN, SDN_FEATURE_ROUTING, RC_SERVICE_START);
#endif
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
#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
			start_plchost();
#endif
#ifdef RTCONFIG_CFGSYNC
			if (!strstr(nvram_safe_get("rc_service"), "start_cfgsync"))
				start_cfgsync();
#ifdef RTCONFIG_CONNDIAG
			start_conn_diag();
#endif
#endif
#if defined(RTCONFIG_NFCM)
            start_nfcm();
#endif
#if defined(RTCONFIG_AMAS)
			start_amas_lib();
#ifdef RTCONFIG_VIF_ONBOARDING
			set_onboarding_vif_status();
#endif
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
#ifdef RTCONFIG_MULTILAN_CFG
		handle_sdn_feature(ALL_SDN, SDN_FEATURE_ALL, action);
#endif
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
#ifdef RTCONFIG_AMAS_ADTBW
			stop_amas_adtbw();
#endif
#ifdef RTCONFIG_ADTBW
			stop_adtbw();
#endif

#ifdef RTCONFIG_WIRELESSREPEATER
			stop_wlcconnect();
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

#ifdef RTCONFIG_WIRELESSREPEATER
			start_wlcconnect();
#endif
#ifdef RTCONFIG_AMAS_ADTBW
			start_amas_adtbw();
#endif
		}
		setup_leds();
		nvram_set("restart_wifi", "0");
	}
#if defined(RTCONFIG_POWER_SAVE)
	else if (!strcmp(script, "pwrsave")) {
		set_power_save_mode();
#if defined(RTCONFIG_FANCTRL)
		restart_fanctrl();
#endif
	}
#endif
#if defined(RTCONFIG_PAGECACHE_RATIO)
	else if (!strcmp(script, "pagecache_ratio")) {
		char val[4];

		snprintf(val, sizeof(val), "%d", get_pagecache_ratio());
		f_write_string("/proc/sys/vm/pagecache_ratio", val, 0, 0);
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
#ifdef RTCONFIG_TCODE
	else if (strcmp(script, "set_wltxpower") == 0) {
		if (!nvram_contains_word("rc_support", "pwrctrl"))
			dbG("\n\tDon't do this!\n\n");
		else
			set_wltxpower();
	}
#endif
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
	else if (strcmp(script, "wan_dns") == 0) {
		if(cmd[1]) {
			int wan_unit = atoi(cmd[1]);
			char tmp[64], tmp2[64], prefix[sizeof("wanXXXXXXXXXX_")];

			_dprintf("%s: wan_dns: %d.\n", __FUNCTION__, wan_unit);
			snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
			if (nvram_get_int(strcat_r(prefix, "dnsenable_x", tmp))) {
				nvram_set(strlcat_r(prefix, "dns", tmp, sizeof(tmp)), nvram_safe_get(strlcat_r(prefix, "dns_r", tmp2, sizeof(tmp2))));
			} else {
				get_userdns_r(prefix, tmp2, sizeof(tmp2));
				nvram_set(strlcat_r(prefix, "dns", tmp, sizeof(tmp)), tmp2);
			}
			update_resolvconf();
#ifdef RTCONFIG_MULTILAN_CFG
			update_sdn_resolvconf();
#endif
		}
	}
	else if (strcmp(script, "wan_if") == 0) {
		if(cmd[1]) {
			int wan_unit = atoi(cmd[1]);
			_dprintf("%s: wan_if: %d.\n", __FUNCTION__, wan_unit);
#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_MULTIWAN_IF
			int restart_ipv6 = ipv6x_enabled(wan_unit);
#else
			int restart_ipv6 = (wan_unit == wan_primary_ifunit_ipv6());
#endif
#endif
			if(action & RC_SERVICE_STOP)
			{
				stop_wan_if(atoi(cmd[1]));
#ifdef RTCONFIG_IPV6
				if (restart_ipv6)
#ifdef RTCONFIG_MULTIWAN_IF
					stop_mtwan_lan_ipv6(wan_unit);
#else
					stop_lan_ipv6();
#endif
#endif
			}
			if(action & RC_SERVICE_START)
			{
#ifdef RTCONFIG_MULTIWAN_IF
				mtwanduck_freeze(5);
#endif
#if defined(RTCONFIG_DETWAN)
				extern void detwan_set_net_block(int add);
				detwan_set_net_block(0);
#endif	/* RTCONFIG_DETWAN */
#ifdef RTCONFIG_IPV6
				if (restart_ipv6)
#ifdef RTCONFIG_MULTIWAN_IF
					start_mtwan_lan_ipv6(wan_unit);
#else
					start_lan_ipv6();
#endif
#endif
#ifdef DSL_AC68U	//Andy Chiu, 2015/09/15.
				//Check the vlan config of ethernet wan, reset the config by new vlan id.
				check_wan_if(atoi(cmd[1]));

#endif
#ifdef RTCONFIG_MULTISERVICE_WAN
#if defined(HND_ROUTER)
				config_mswan(atoi(cmd[1]));
#endif
#endif
				start_wan_if(atoi(cmd[1]));
#if defined(RTCONFIG_IPV6) && defined(RTCONFIG_DUALWAN)
#ifdef RTCONFIG_MULTIWAN_IF
				restart_mtwan_dnsmasq_ipv6(wan_unit);
#else
				restart_dnsmasq_ipv6();
#endif
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
#ifdef RTCONFIG_DSL_REMOTE
				remove_dsl_autodet();
#endif
				dsl_wan_config(2);
				start_wan_if(atoi(cmd[1]));
			}
		}
	}
	else if (strcmp(script, "dslwan_qis") == 0) {
		if((action&RC_SERVICE_STOP) && (action & RC_SERVICE_START)) {
			int unit = cmd[1] ? atoi(cmd[1]) : 0;

#ifdef RTCONFIG_DSL_REMOTE
			remove_dsl_autodet();
#endif
			stop_wan_if(unit);

#ifdef DSL_AX82U
			config_stb_bridge();
#else //for old qis
			nvram_set("dslx_converted", "0");
#endif
			dsl_wan_config(2);
			start_wan_if(unit);
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
#ifdef RTCONFIG_DSL_REMOTE
			remove_dsl_autodet();
#endif
			stop_wan_if(atoi(cmd[1]));
#ifdef DSL_AX82U
			config_stb_bridge();
#else //for old qis
			nvram_set("dslx_converted", "0");
#endif
			dsl_wan_config(2);
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
			config_xdsl();
		}
	}
	else if (strcmp(script, "dsl_autodet") == 0) {
		if(action & RC_SERVICE_STOP) stop_dsl_autodet();
		if(action & RC_SERVICE_START) start_dsl_autodet();
	}
	else if (strcmp(script, "dsl_diag") == 0) {
		if(action & RC_SERVICE_STOP) stop_dsl_diag();
		if(action & RC_SERVICE_START) start_dsl_diag();
	}
#endif
#ifdef RTCONFIG_FRS_FEEDBACK
#ifdef RTCONFIG_DBLOG
	else if (strcmp(script, "dblog") == 0) {
		if(action & RC_SERVICE_STOP) stop_dblog();
		if(action & RC_SERVICE_START) start_dblog(1);
	}
#endif /* RTCONFIG_DBLOG */
#endif /* RTCONFIG_FRS_FEEDBACK */
#ifdef	RTCONFIG_MOCA
	else if (strcmp(script, "moca") == 0) {
		if(action & RC_SERVICE_STOP) stop_moca();
		if(action & RC_SERVICE_START) start_moca();
	}
	else if (strcmp(script, "moca_mps") == 0) {
//		if(action & RC_SERVICE_STOP) stop_moca_mps();
		if(action & RC_SERVICE_START) start_moca_mps();
	}
	else if (strcmp(script, "moca_reset_privacy") == 0) {
		reset_moca_default_privacy();
	}
	else if (strcmp(script, "moca_node_state") == 0) {
		moca_node_state();
	}
	else if (strcmp(script, "moca_set_privacy") == 0) {
		moca_set_privacy();
	}
	else if (strcmp(script, "moca_local_node_state") == 0) {
		moca_local_node_state(NULL);
	}
	else if (strcmp(script, "moca_mib") == 0) {
		moca_mib(NULL);
	}
	else if (strcmp(script, "moca_log") == 0) {
		if(cmd[1])
		{
			create_moca_log(cmd[1]);
		}
		else
		{
			create_moca_log("/tmp/moca.log");
		}
	}
#endif
#ifdef RTCONFIG_AHS
	else if (strcmp(script, "ahs") == 0) {
		if(action & RC_SERVICE_STOP) stop_ahs();
		if(action & RC_SERVICE_START) start_ahs();
	}
#endif /* RTCONFIG_AHS */
	else if (strcmp(script, "wan_line") == 0) {
		_dprintf("%s: restart_wan_line: %s.\n", __FUNCTION__, cmd[1]);
		if(cmd[1]) {
			wan_unit = atoi(cmd[1]);
			char *current_ifname = get_wan_ifname(wan_unit);

			wan_up(current_ifname);
#ifdef RTCONFIG_MULTISERVICE_WAN
			if (nvram_match("switch_wantag", "none")) {
				update_iptv_ifname(wan_unit);
				start_igmpproxy(current_ifname);
			}
#endif
		}
	}
#ifdef CONFIG_BCMWL5
	else if (strcmp(script, "nas") == 0) {
		if(action & RC_SERVICE_STOP) stop_nas();
		if(action & RC_SERVICE_START) {
#ifdef RTCONFIG_DHDAP
			start_wl();
			lanaccess_wl();
#endif
			start_eapd();
			start_nas();
			start_wps();
#ifdef RTCONFIG_NOTIFICATION_CENTER
			start_wlc_nt();
			start_wlc_monitor();
#endif
#if defined(RTCONFIG_WLCEVENTD)
			start_wlceventd();
#endif
#if defined(RTCONFIG_HAPDEVENT)
			start_hapdevent();
#endif
#ifdef RTCONFIG_BCMWL6
			start_igmp_proxy();
#ifdef BCM_BSD
			start_bsd();
#endif
#ifdef BCM_APPEVENTD
			start_appeventd();
#endif
#ifdef BCM_CEVENTD
			start_ceventd();
#endif
#ifdef BCM_SSD
			start_ssd();
#endif
#ifdef BCM_EVENTD
			start_eventd();
#endif
			start_acsd();
#if defined(RTCONFIG_DHDAP) || defined(RTCONFIG_HND_ROUTER_AX)
			start_dhd_monitor();
#endif
#ifdef BCM_ASPMD
			start_aspmd();
#endif
#endif
#ifndef RTCONFIG_DHDAP
			start_wl();
			lanaccess_wl();
#endif
#ifdef RTCONFIG_BCMWL6
#ifdef RTCONFIG_HSPOT
			start_hspotap();
#endif
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_VIF_ONBOARDING)
			set_onboarding_vif_status();
#endif
		}
	}
#endif
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
#ifdef RTCONFIG_UAC_TUNNEL
	else if (strcmp(script, "aaeuac") == 0)
	{
		if(action&RC_SERVICE_STOP){
			stop_aaeuac();
		}
		if(action&RC_SERVICE_START) {
			start_aaeuac(count, cmd);
		}
	}
#endif
#endif
#ifdef RTCONFIG_USB
	else if (strcmp(script, "nasapps") == 0)
	{
		if(action&RC_SERVICE_STOP){
			restart_nas_services(1, 0, 0);
		}
		if(action&RC_SERVICE_START){
			int restart_upnp = 0;
			if (pidof("miniupnpd") != -1) {
				stop_upnp();
				restart_upnp = 1;
			}
			restart_nas_services(0, 1,0);
			if (restart_upnp) start_upnp();
		}
	}
#if (defined(RTCONFIG_SAMBASRV) || defined(RTCONFIG_TUXERA_SMBD)) && defined(RTCONFIG_FTP)
	else if (strcmp(script, "ftpsamba") == 0)
	{
		if(action & RC_SERVICE_STOP) {
			stop_ftpd(0);
			stop_samba(0);
		}
		if(action & RC_SERVICE_START) {
#ifdef RTCONFIG_MULTILAN_CFG
			start_dnsmasq(ALL_SDN);
#else
			start_dnsmasq();// this includes stop_dnsmasq
#endif
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
			stop_ftpd(0);
			stop_samba(0);
#endif
		}
		if(action & RC_SERVICE_START) {
#if defined(RTCONFIG_SAMBASRV) && defined(RTCONFIG_FTP)
#ifdef RTCONFIG_MULTILAN_CFG
			start_dnsmasq(ALL_SDN);
#else
			start_dnsmasq();	// this includes stop_dnsmasq
#endif
			setup_passwd();
			set_hostname();
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
		if(action & RC_SERVICE_STOP) stop_ftpd(0);
		if(action & RC_SERVICE_START) start_ftpd();

		/* for security concern, even if you stop ftp daemon, it is better to restart firewall to clean FTP port: 21. */
		start_firewall(wan_primary_ifunit(), 0);
	}
	else if (strcmp(script, "ftpd_force") == 0)
	{
		nvram_set("st_ftp_force_mode", nvram_safe_get("st_ftp_mode"));
		nvram_commit();

		if(action & RC_SERVICE_STOP) stop_ftpd(0);
		if(action & RC_SERVICE_START) start_ftpd();

		/* for security concern, even if you stop ftp daemon, it is better to restart firewall to clean FTP port: 21. */
		start_firewall(wan_primary_ifunit(), 0);
	}
#endif
#ifdef RTCONFIG_TFTP_SERVER
	else if (strcmp(script, "tftpd") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_tftpd(0);
		if(action & RC_SERVICE_START) start_tftpd();
	}
#endif
#if defined(RTCONFIG_SAMBASRV) || defined(RTCONFIG_TUXERA_SMBD)
	else if (strcmp(script, "samba") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_samba(0);
		if(action & RC_SERVICE_START) {
#ifdef RTCONFIG_MULTILAN_CFG
			start_dnsmasq(ALL_SDN);
#else
			start_dnsmasq();// this includes stop_dnsmasq
#endif
			start_samba();
		}
	}
	else if (strcmp(script, "samba_force") == 0)
	{
		nvram_set("st_samba_force_mode", nvram_safe_get("st_samba_mode"));
		nvram_commit();

		if(action & RC_SERVICE_STOP) stop_samba(0);
		if(action & RC_SERVICE_START) {
#ifdef RTCONFIG_MULTILAN_CFG
			start_dnsmasq(ALL_SDN);
#else
			start_dnsmasq();// this includes stop_dnsmasq
#endif
			start_samba();
		}
	}
#endif
#ifdef RTCONFIG_CAPTIVE_PORTAL
	else if (strcmp(script, "chilli") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_chilli();
		if(action&RC_SERVICE_START) start_chilli();
		setup_passwd();
	}
	else if (strcmp(script, "CP") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_CP();
		if(action&RC_SERVICE_START) start_CP();
		setup_passwd();
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
#ifdef RTCONFIG_BCMWL6
	else if (strcmp(script, "acsd") == 0)
	{
		if(action & RC_SERVICE_STOP){
			stop_acsd();
		}
		if(action & RC_SERVICE_START) {
			start_acsd();
		}
	}
#endif	/* RTCONFIG_BCMWL6 */
	else if (strcmp(script, "enable_webdav") == 0)
	{
		stop_upnp();
		stop_ddns();
#ifdef RTCONFIG_WEBDAV
		stop_webdav();
#endif
		start_firewall(wan_primary_ifunit(), 0);
		start_webdav();
		start_ddns(NULL);
		start_upnp();

	}
#ifdef RTCONFIG_FAST_ACL_SET
	else if (strcmp(script, "fastaclset") == 0 )
	{
		if(action&RC_SERVICE_START) {
			start_setacl_not_restart_wifi();
		}
	}
#endif
//#endif

#ifdef RTCONFIG_NFCM
	else if (strcmp(script, "nfcm") == 0)
	{
		if(action&RC_SERVICE_STOP){
			stop_nfcm();
		}
		if(action&RC_SERVICE_START) {
			start_nfcm();
		}
	}
#endif

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
#ifdef RTCONFIG_TCPLUGIN
	else if (strcmp(script, "qmacc") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_qmacc();
		if(action & RC_SERVICE_START) start_qmacc();
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
			stop_mt_daapd(0);
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
		if(action & RC_SERVICE_STOP) stop_mt_daapd(0);
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

#if !defined(RTCONFIG_HND_ROUTER)
				free_caches(FREE_MEM_PAGE, 1, 0);
#endif

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
		char phone[64], message[PATH_MAX];
		char *at_cmd[] = {"/usr/sbin/modem_status.sh", "send_sms", phone, message, NULL};

		snprintf(phone, sizeof(phone), "%s", nvram_safe_get("modem_sms_phone"));

		if(!strcmp(cmd[1], "alert"))
			snprintf(message, PATH_MAX, "%s", nvram_safe_get("modem_sms_message1"));
		else
			snprintf(message, PATH_MAX, "%s", nvram_safe_get("modem_sms_message2"));

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

#ifdef RTCONFIG_USB_MODEM
		char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];

		for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit){
			snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
			if(dualwan_unit__usbif(wan_unit) &&
					(wan_unit == wan_primary_ifunit()
#ifdef RTCONFIG_DUALWAN
							|| nvram_match("wans_mode", "lb")
#endif
							)
					){
				if(get_wan_sbstate(wan_unit) == WAN_STOPPED_REASON_DATALIMIT)
					nvram_set_int(strcat_r(prefix, "sbstate_t", tmp), WAN_STOPPED_REASON_NONE);
			}
		}
#endif

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
#if defined(RT4GAC86U) || defined(RT4GAX56)
	else if(!strcmp(script, "rebootmodem")){
		int i;
#if defined(RT4GAC86U)
		int gpio = 100;
#elif defined(RT4GAX56)
		int gpio = 12;
#endif

		nvram_set("freeze_duck", "6");
		set_gpio(gpio, 0);

		for(i = 0; i < 5; ++i){
			_dprintf("----- Turning off the power of modem (%d/5) seconds -----\n", i+1);
			sleep(1);
		}

		set_gpio(gpio, 1);
	}
	else if(!strcmp(script, "resetmodem")){
		int i;
#if defined(RT4GAC86U)
		int gpio = 77;
#elif defined(RT4GAX56)
		int gpio = 18;
#endif

		nvram_set("freeze_duck", "6");
		set_gpio(gpio, 1);

		for(i = 0; i < 5; ++i){
			_dprintf("----- Resetting modem (%d/5) seconds -----\n", i+1);
			sleep(1);
		}

		set_gpio(gpio, 0);
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
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
	else if (strcmp(script, "ble_rename_ssid") == 0)
	{
		if((action & RC_SERVICE_STOP) && (action & RC_SERVICE_START))
		{
			ble_rename_ssid();
		}
	}
#endif
#endif	/* RTCONFIG_BT_CONN */
#if defined(RTCONFIG_QCA_EZMESH)
	else if (strcmp(script, "ezmesh_test") == 0)
	{
		eval("hyctl", "attach", nvram_safe_get("lan_ifname"));
		gen_ez_config();
		doSystem("ezmesh -C /tmp/ez.conf -cfg80211 &");
		_dprintf("ezmesh service is running!\n");
	

	}	
	else if (strcmp(script, "wsplcd_test") == 0)
	{
		wsplcd_enable();
		if(nvram_get_int("role"))
		{	
			gen_1905_config(1);
			doSystem("wsplcd-map -c tmp/wsplcd.conf -cfg80211 &");
		}
		else
		{	
			gen_1905_config(0);
			gen_bss_policy();
			doSystem("wsplcd-map -c tmp/wsplcd.conf -M /tmp/bss.conf -cfg80211 &");
		}	
		_dprintf("1905 service is running!\n");

	}
#endif	
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
#ifdef RTCONFIG_UIDEBUG // Left for UI debug
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
#ifdef RTCONFIG_REVERTFW
	else if(strcmp(script, "revert_fw") == 0)
	{
		if(action & RC_SERVICE_START) {
#ifdef RTCONFIG_UIDEBUG // Left for UI debug
			char *webscript_dir;
			webscript_dir = nvram_safe_get("webscript_dir");
			if(strlen(webscript_dir))
				sprintf(nvtmp, "%s/revert_upgrade.sh", webscript_dir);
			else
#endif			
			snprintf(nvtmp, sizeof(nvtmp), "revert_upgrade.sh");
			cmd[0] = nvtmp;
			start_script(count, cmd);
		}
	}
#endif
#ifdef RTCONFIG_ROLLBACKFW
	else if(strcmp(script, "rollback_upgrade") == 0)
	{
		snprintf(nvtmp, sizeof(nvtmp), "rollback_upgrade.sh");
		cmd[0] = nvtmp;
		dbg("[start_rollback_upgrade] cmd: %s \n", cmd);
		start_script(count, cmd);
	}
#endif
#if defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033)
	else if (strcmp(script, "upgrade_aqr113c_fw") == 0)
	{
		if (action & RC_SERVICE_START)
			upgrade_aqr113c_fw();
	}
#endif
#ifdef RTCONFIG_REVERTFW
	else if(strcmp(script, "revert_fw") == 0)
	{
		if(action & RC_SERVICE_START) {
#ifdef RTCONFIG_UIDEBUG // Left for UI debug
			char *webscript_dir;
			webscript_dir = nvram_safe_get("webscript_dir");
			if(strlen(webscript_dir))
				sprintf(nvtmp, "%s/revert_upgrade.sh", webscript_dir);
			else
#endif			
			snprintf(nvtmp, sizeof(nvtmp), "revert_upgrade.sh");
			cmd[0] = nvtmp;
			start_script(count, cmd);
		}
	}
#endif
#ifdef RTCONFIG_LETSENCRYPT
	else if (strcmp(script, "ddns_le") == 0)
	{
		nvram_set("le_rc_notify", "1");
		system("rm -f /var/cache/inadyn/*.cache");
		if(action & RC_SERVICE_STOP) stop_ddns();
		if(action & RC_SERVICE_START) start_ddns(NULL);
	}
#endif
	else if (strcmp(script, "ddns") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_ddns();
		if(action & RC_SERVICE_START) {
			if (cmd[1])
				start_ddns(cmd[1]);
			else
				start_ddns(NULL);
			update_srv_cert_if_ddns_changed();
		}
	}
#if defined(RTCONFIG_HTTPS)
	else if (!strcmp(script, "prepare_cert")) {
		int r;

		r = prepare_cert_in_etc();
		if (r > 0) {
			/* Load new certification after the current session logout. */
			nvram_set("httpds_reload_cert", "2");
#if defined(RTCONFIG_IPV6)
			if (ipv6_enabled() && nvram_match("misc_http_x", "1")) {
				nvram_set("httpds6_reload_cert", "2");
			}
#endif
		}
	}
#endif
	else if(strcmp(script, "asusddns_unregister") == 0)
	{
		asusddns_unregister();
	}
	else if(strcmp(script, "clean_ddns") == 0)
	{
		clean_ddns();
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
	else if (strcmp(script, "rdisc6") == 0) {
		if (action & RC_SERVICE_STOP)
			stop_rdisc6();
		if (action & RC_SERVICE_START)
			start_rdisc6();
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
			else if(dualwan_unit__usbif(wan_primary_ifunit_ipv6())){
				//stop_wan_if(wan_primary_ifunit_ipv6());
				//start_wan_if(wan_primary_ifunit_ipv6());
				stop_wan6();
				start_wan6();
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
		if(action & RC_SERVICE_START)
		{
#ifdef RTCONFIG_MULTILAN_CFG
			reload_dnsmasq(ALL_SDN);
#else
			reload_dnsmasq();
#endif
		}
	}
#ifdef RTCONFIG_MULTILAN_CFG
	else if (strcmp(script, "apg") == 0) 
	{
		if (action & RC_SERVICE_STOP) {
			apg_stop();
			update_amas_lldpd_ifnames();
		}
		
		if (action & RC_SERVICE_START) {
			apg_start();
			update_amas_lldpd_ifnames();
		}
	}
	else if (strcmp(script, "sdn") == 0)
	{
		if(count > 1)
		{
			for(i=1;i<count;i++)
			{
				if(cmd[i])
				{
					handle_sdn_feature(atoi(cmd[i]), SDN_FEATURE_ALL, action);
				}
			}
		}
		else
		{
			handle_sdn_feature(ALL_SDN, SDN_FEATURE_ALL, action);
		}

#ifdef RTCONFIG_FBWIFI
		if (action & RC_SERVICE_STOP) {
			stop_fbwifi();
			stop_fbwifi_check();
			stop_fbwifi_config();
		}
		if (action & RC_SERVICE_START) {
			start_fbwifi();
		}
#endif
	}
	else if(strcmp(script, "sdn_del") == 0)
	{
		remove_sdn();
	}
	else if(strcmp(script, "sdn_del") == 0)
	{
		remove_sdn();
	}
	else if(strcmp(script, "urlfilter_sdn") == 0)
	{
		if(count > 1)
		{
			for(i=1;i<count;i++)
			{
				if(cmd[i])
				{
					handle_sdn_feature(atoi(cmd[i]), SDN_FEATURE_URL_FILTER, 0);
				}
			}
		}
		else
		{
			_dprintf("[%s] sdn index is required\n", script);
		}
	}
	else if(strcmp(script, "urlfilter_idx") == 0)
	{
		if(count > 1)
		{
			for(i=1;i<count;i++)
			{
				if(cmd[i])
				{
					update_URLFilter(atoi(cmd[i]));
				}
			}
		}
		else
		{
			_dprintf("[%s] url filter index is required\n", script);
		}
	}
	else if(strcmp(script, "nwsfilter_sdn") == 0)
	{
		if(count > 1)
		{
			for(i=1;i<count;i++)
			{
				if(cmd[i])
				{
					handle_sdn_feature(atoi(cmd[i]), SDN_FEATURE_NWS_FILTER, 0);
				}
			}
		}
		else
		{
			_dprintf("[%s] sdn index is required\n", script);
		}
	}
	else if(strcmp(script, "nwsfilter_idx") == 0)
	{
		if(count > 1)
		{
			for(i=1;i<count;i++)
			{
				if(cmd[i])
				{
					update_NwServiceFilter(atoi(cmd[i]));
				}
			}
		}
		else
		{
			_dprintf("[%s] network service filter index is required\n", script);
		}
	}
	else if(strcmp(script, "sdn_internal_access") == 0)
	{
		handle_sdn_feature(ALL_SDN, SDN_FEATURE_SDN_INTERNAL_ACCESS, 0);
	}
	else if(strcmp(script, "sdn_wan") == 0)
	{
		if(count > 1)
		{
			for(i=1;i<count;i++)
			{
				if(cmd[i])
					handle_sdn_feature(atoi(cmd[i]), SDN_FEATURE_WAN, 0);
			}
		}
		else
		{
			handle_sdn_feature(ALL_SDN, SDN_FEATURE_WAN, 0);
		}
	}
#endif
	else if (strcmp(script, "dnsmasq") == 0)
	{
#ifdef RTCONFIG_MULTILAN_CFG
		if(count > 1)
		{
			for(i=1;i<count;i++)
			{
				if(cmd[i])
				{
					if(action & RC_SERVICE_STOP) stop_dnsmasq(atoi(cmd[i]));
					if(action & RC_SERVICE_START) start_dnsmasq(atoi(cmd[i]));
				}
			}
		}
		else
		{
			if(action & RC_SERVICE_STOP) stop_dnsmasq(ALL_SDN);
			if(action & RC_SERVICE_START) start_dnsmasq(ALL_SDN);
		}
#else
		if(action & RC_SERVICE_STOP) stop_dnsmasq();
		if(action & RC_SERVICE_START) start_dnsmasq();
#endif
	}
#ifdef RTCONFIG_DNSPRIVACY
	else if (strcmp(script, "stubby") == 0)
	{
#ifdef RTCONFIG_MULTILAN_CFG
		if(count > 1)
		{
			for(i=1;i<count;i++)
			{
				if(cmd[i])
				{
					if(action & RC_SERVICE_STOP) stop_stubby(atoi(cmd[i]));
					if(action & RC_SERVICE_START) start_stubby(atoi(cmd[i]));
				}
			}
		}
		else
		{
			if(action & RC_SERVICE_STOP) stop_stubby(ALL_SDN);
			if(action & RC_SERVICE_START) start_stubby(ALL_SDN);
		}
#else
		if(action & RC_SERVICE_STOP) stop_stubby();
		if(action & RC_SERVICE_START) start_stubby();
#endif
	}
#endif
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
#ifdef RTCONFIG_AUPNPC
	else if (strcmp(script, "aupnpc") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_aupnpc();
		if(action & RC_SERVICE_START) start_aupnpc();
	}
#endif
#if defined(RTCONFIG_MLO)
	else if (strcmp(script, "init_mlo_config") == 0)
	{
		init_mlo_config();
	}
#endif
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
			QOS_CONTROL();
		}
		nvram_set("restart_qo", "0");
	}
#if defined(RTCONFIG_BWDPI)
	else if (strcmp(script, "wrs") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_dpi_engine_service(0);
		if(action & RC_SERVICE_START) start_dpi_engine_service();
	}
	else if (strcmp(script, "wrs_force") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_dpi_engine_service(3);
	}
	else if (strcmp(script, "sig_check") == 0)
	{
		if(action & RC_SERVICE_START){
			char *sig_update_argv[] = {"sig_update.sh", NULL};
			char dev_wan[16];
			_eval(sig_update_argv, NULL, 0, NULL);
			if(nvram_get_int("sig_state_flag")){
				char *sig_upgrade_argv[] = {"sig_upgrade.sh", NULL};
				_eval(sig_upgrade_argv, NULL, 0, NULL);
			}
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
	else if (strcmp(script, "traffic_analyzer") == 0)
	{
		// only stop service need to save database
		if(action & RC_SERVICE_STOP) hm_traffic_analyzer_save();
	}
	else if (strcmp(script, "wrs_wbl") == 0)
	{
		start_wrs_wbl_service();
	}
	else if (strcmp(script, "mobile_game") == 0)
	{
		MobileDevMode_restart();
	}
#endif
#ifdef RTCONFIG_GEARUPPLUGIN
	else if (strcmp(script, "gu_service") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_gu_service(gu_enable_status());
		if(action & RC_SERVICE_START) start_gu_service();

	}
#endif
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
	else if (strcmp(script, "oauth_google_gen_token_email") == 0)
	{
		oauth_google_gen_token_email();
#ifdef RTCONFIG_CFGSYNC
		// trigger cfg_server do sync
		const char config[] =
		{
			"{"\
			"\"oauth_google_refresh_token\":\"\","\
			"\"oauth_google_user_email\":\"\","\
			"\"fb_email_provider\":\"\""\
			"}\0"
		};
		char event_msg[133] = {0};
		memset(event_msg, 0, sizeof(event_msg));
		snprintf(event_msg, sizeof(event_msg)-1, RC_CONFIG_CHANGED_MSG, EID_RC_CONFIG_CHANGED, config);
		(void)send_cfgmnt_event(event_msg);
		//  WEVENT_GENERIC_MSG	 "{\""WEVENT_PREFIX"\":{\""EVENT_ID"\":\"%d\"}}"
#endif	// RTCONFIG_CFGSYNC
	}
	else if (strcmp(script, "oauth_google_drive_gen_token") == 0)
	{
		oauth_google_drive_gen_token();
#ifdef RTCONFIG_CFGSYNC
		// trigger cfg_server do sync
		const char config[] =
		{
			"{"\
			"\"oauth_google_drive_refresh_token\":\"\","\
			"\"oauth_google_user_email\":\"\","\
			"\"fb_email_provider\":\"\""\
			"}\0"
		};
		char event_msg[133] = {0};
		memset(event_msg, 0, sizeof(event_msg));
		snprintf(event_msg, sizeof(event_msg)-1, RC_CONFIG_CHANGED_MSG, EID_RC_CONFIG_CHANGED, config);
		(void)send_cfgmnt_event(event_msg);
		//  WEVENT_GENERIC_MSG	 "{\""WEVENT_PREFIX"\":{\""EVENT_ID"\":\"%d\"}}"
#endif	// RTCONFIG_CFGSYNC
	}
	else if (strcmp(script, "oauth_google_drive_check_token_status") == 0)
	{
		oauth_google_drive_check_token_status();
	}
	else if (strcmp(script, "oauth_google_check_token_status") == 0)
	{
		oauth_google_check_token_status();
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
		int wan_unit = (count > 1) ? atoi(cmd[1]) : wan_primary_ifunit();
		int lan_unit = (count > 2) ? atoi(cmd[2]) : 0;

		nvram_set("restart_fwl", "1");
		if(action & RC_SERVICE_START)
		{
#if defined(BRTAC828)
			setup_pt_conntrack();
#endif
#if defined(RTCONFIG_QCA) || \
		(defined(RTCONFIG_RALINK) && !defined(RTCONFIG_DSL) && !defined(RTN13U))
			reinit_hwnat(-1);
#endif
			start_default_filter(lan_unit);
#ifdef RTCONFIG_PARENTALCTRL
			start_pc_block();
#endif
			start_firewall(wan_unit, lan_unit);
#if defined(RTCONFIG_PARENTALCTRL) && defined(RTCONFIG_CONNTRACK)
			killall("pctime", SIGUSR1); // ask pctime to reload parental control rules.
			killall("pctime", SIGUSR2); // ask pctime to delete conntrack or flush fc if any.
#endif
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
					int evalRet = eval("iptables-restore", cmd[1]);
					if (evalRet) {
						rule_apply_checking("services", __LINE__, cmd[1], evalRet);
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
#ifdef RTCONFIG_NTPD
	else if (strcmp(script, "ntpd") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_ntpd();
		if(action & RC_SERVICE_START) start_ntpd();
	}
#endif
	else if (strcmp(script, "rebuild_cifs_config_and_password") ==0)
	{
		fprintf(stderr, "rc rebuilding CIFS config and password databases.\n");
//		regen_passwd_files(); /* Must be called before regen_cifs_config_file(). */
//		regen_cifs_config_file();
	}
	else if (strcmp(script, "time") == 0)
	{
		if(action & RC_SERVICE_STOP) {
#ifdef RTCONFIG_CROND
			stop_cron();
#endif
#ifdef RTCONFIG_NTPD
			stop_ntpd();
#endif
#ifdef RTCONFIG_BWDPI
			stop_hour_monitor_service();
#endif
			stop_telnetd();
#ifdef RTCONFIG_SSH
			stop_sshd();
#endif
			stop_logger();
			//stop_httpd();
		}
		if(action & RC_SERVICE_START) {
			setup_timezone();

			nvram_set("reload_svc_radio", "1");

			refresh_ntpc();
			start_logger();
			start_telnetd();
#ifdef RTCONFIG_SSH
			start_sshd();
#endif
			//start_httpd();
//			start_firewall(wan_primary_ifunit(), 0);
#ifdef RTCONFIG_BWDPI
			start_hour_monitor_service();
#endif
#ifdef RTCONFIG_CROND
			start_cron();
#endif
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
				if (!nvram_match("wps_ign_btn", "1"))
					kill_pidfile_s("/var/run/watchdog.pid", SIGUSR1);
				else
					kill_pidfile_s("/var/run/watchdog.pid", SIGTSTP);
			}
			nvram_unset("wps_ign_btn");
		}
	}
#ifdef RTCONFIG_AMAS
	else if (strcmp(script, "wps_method_ob")==0)
	{
		if(action & RC_SERVICE_STOP) {
			stop_wps_method_ob();
			if (!nvram_match("wps_ign_btn", "1"))
				kill_pidfile_s("/var/run/watchdog.pid", SIGUSR2);
		}
		if(action & RC_SERVICE_START) {
			if (!wps_band_radio_off(get_radio_band(nvram_get_int("wps_band_x"))) &&
			    !wps_band_ssid_broadcast_off(get_radio_band(nvram_get_int("wps_band_x")))) {
				nvram_set("wps_sta_pin", "00000000");
				start_wps_method_ob();
				if (!nvram_match("wps_ign_btn", "1"))
					kill_pidfile_s("/var/run/watchdog.pid", SIGUSR1);
				else
					kill_pidfile_s("/var/run/watchdog.pid", SIGTSTP);
			}
			nvram_unset("wps_ign_btn");
		}
	}
#if defined(RTCONFIG_QCA) || defined(RTCONFIG_BRCM_HOSTAPD)
	else if (strcmp(script, "wpsie")==0)
	{
		runtime_onoff_wps(nvram_get_int("wps_enable"));
	}
#endif
#endif
#if defined(RTCONFIG_AMAS) && defined(CONFIG_BCMWL5)
	else if (strcmp(script, "wps_enr")==0)
	{
		if (is_router_mode()
#ifdef RTCONFIG_DPSTA
			|| (dpsta_mode() && nvram_get_int("re_mode") == 0)
#if defined(RPAX56)
			|| (nvram_match("x_Setting", "0") && nvram_get_int("re_mode") == 0)
#endif
			|| (rp_mode() && nvram_get_int("re_mode") == 0)
#elif defined(RPAX58)
			|| (nvram_match("x_Setting", "0") && nvram_get_int("re_mode") == 0)
#endif
		) {
				int unit = nvram_get_int("wps_band_x");
				char tmp[100], prefix[] = "wlXXXXXXXXXX_";
#if defined(RPAX56) || defined(RPAX58)
			nvram_set_int("rpx_wps_enr", 1);
			if(!nvram_match("def_wps_method", "1"))
				nvram_unset("wps_config_method");
#endif
			if (
#if defined(RPAX56) || defined(RPAX58)
				nvram_match("x_Setting", "0")
#else
				is_router_mode()
#endif
			) {
				nvram_set_int("amesh_wps_enr", 1);
				snprintf(prefix, sizeof(prefix), "wl%d_", unit);
				nvram_set(strcat_r(prefix, "mfp", tmp), "1");
#ifdef RTCONFIG_HND_ROUTER_AX
				stop_acsd();
				stop_wps();
				stop_nas();
				stop_lan_wl();
				start_lan_wl();
				restart_wl();
				lanaccess_wl();
				start_nas();
				start_wps();
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_VIF_ONBOARDING)
				set_onboarding_vif_status();
#endif
#else
				char ifname[IFNAMSIZ] = { 0 };

#if defined(RTCONFIG_BCM4708) || defined(RTCONFIG_BCM_7114)  || defined(RTCONFIG_HND_ROUTER)
				nvram_set(strcat_r(prefix, "mode", tmp), "wet");
#else
				nvram_set(strcat_r(prefix, "mode", tmp), "psta");
				nvram_set(strcat_r(prefix, "dwds", tmp), "0");
#endif
				strlcpy(ifname, nvram_safe_get(strcat_r(prefix, "ifname", tmp)), sizeof(ifname));
				eval("wlconf", ifname, "down");
				eval("wlconf", ifname, "up");
				eval("wlconf", ifname, "start");
				eval("wl", "-i", ifname, "disassoc");
				start_wps();
				sleep(1);
#endif
			}

			stop_wps_method();

			snprintf(prefix, sizeof(prefix), "wl%d_", unit);
			if (!nvram_match("SG_mode", "1") && !nvram_match(strcat_r(prefix, "wps_mode", tmp), "enabled"))
				nvram_set_int("wps_proc_status", 99);
			else
			{
				count = 3;
retry_wps_enr:
				start_wps_enr();
				sleep(1);
				if (!nvram_get_int("wps_proc_status") && (count-- > 0))
					goto retry_wps_enr;
				kill_pidfile_s("/var/run/watchdog.pid", SIGTSTP);
				nvram_unset("wps_ign_btn");
			}
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
#if defined(RTCONFIG_MTK_BSD) 
		int restart_bs20 = 0;
#endif
#if defined(RTCONFIG_QCA_LBD) 
		int restart_lbd = 0;
#endif
		if(action & RC_SERVICE_STOP) {
#if defined(RTCONFIG_QCA_LBD)
			if (nvram_match("smart_connect_x", "1") && pids("lbd")) {
				stop_qca_lbd();
				restart_lbd = 1;
			}
#endif
#if defined(RTCONFIG_MTK_BSD)
			if (nvram_match("smart_connect_x", "1") && pids("bs20")) {
				stop_mtk_bs20();
				restart_bs20 = 1;
			}
#endif
			stop_wlcscan();
		}
		if(action & RC_SERVICE_START) {
			start_wlcscan();
#if defined(RTCONFIG_QCA_LBD)
			if (restart_lbd)
				start_qca_lbd();
#endif
#if defined(RTCONFIG_MTK_BSD)
			if (restart_bs20)
				start_mtk_bs20();
#endif
		}
	}
#endif
#ifdef RTCONFIG_WIRELESSREPEATER
#if defined(RTCONFIG_CHECK_PAP_WPAPSK) \
 && (defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)) \
 && !defined(RTCONFIG_CONCURRENTREPEATER)
	else if (strcmp(script, "check_pap_pw")==0)
	{
		check_pap_passwrod();
	}
#endif
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
#ifdef RTCONFIG_BHCOST_OPT
            start_amas_ssd();
			start_amas_status();
			start_amas_misc();
#endif
			start_amas_wlcconnect();
			start_amas_bhctrl();
#endif
			start_wlcconnect();

		}
	}
#ifdef RTCONFIG_BRCM_HOSTAPD
	else if (strcmp(script, "hapd")==0)
	{
		if(action & RC_SERVICE_STOP)
		{
			stop_hostapd();
		}

		if(action & RC_SERVICE_START) 
		{
			start_hapd_wpasupp(1);	// skip wpa_supp
		}
	}
#ifdef RTCONFIG_AMAS
	else if (strcmp(script, "wpasupp_qis")==0)	// rp qis dedicated preauth
	{
		int i = -1, t, r, band = -1;
		char tmp[128], prefix[16], prefix_b[16], *orig;
		char reason_tmp[16];
		int client_mode = 1;
		int reload_wait = nvram_get_int("reload_wait")?:5;
		int pre_auth = nvram_match("x_Setting", "0") && !nvram_match("stop_preauth", "1");

		if(cmd[1])
			band = atoi(cmd[1]);

		_dprintf("%s wpasupp_qis..pre_auth:%d, wl%d, cli:%d(%d/%d), reload_wait:%d\n", __func__, pre_auth, band, client_mode, dpsta_mode(), dpsr_mode(), reload_wait);

		if (pre_auth == 0) {
			_dprintf("%s wlc%d stop_preauth due not initial settings\n", __func__, band);
			nvram_set("wpas0_reason", "completed");
			nvram_set("wpas1_reason", "completed");
			nvram_set("wpas2_reason", "completed");

			if(client_mode)
        			killall("psta_monitor", SIGTERM);
		} else {
			nvram_set("wpas0_reason", "");
			nvram_set("wpas1_reason", "");
			nvram_set("wpas2_reason", "");
		}

		if(action & RC_SERVICE_STOP)
		{
			if(pre_auth)
				stop_wpasupp(band);
		}

		if(action & RC_SERVICE_START)
		{
			if(pre_auth && client_mode) {
        			//killall("psta_monitor", SIGTERM);

				set_wpasupp(band, 1);

				if(band >= 0) {
					snprintf(prefix, sizeof(prefix), "wlc%d_", band);
					if(*nvram_safe_get(strcat_r(prefix, "ssid", tmp)))
					{
						if(nvram_match("x_Setting", "1")) {
							_dprintf("backup current wlc%d settings\n", band);
	
							snprintf(prefix, sizeof(prefix), "wlr%d_", band);
							snprintf(prefix_b, sizeof(prefix_b), "wl%d_", band);

							orig = nvram_safe_get(strcat_r(prefix_b, "akm", tmp));
							nvram_set(strcat_r(prefix, "akm", tmp), orig);

							orig = nvram_safe_get(strcat_r(prefix_b, "wpa_psk", tmp));
							nvram_set(strcat_r(prefix, "wpa_psk", tmp), orig);

							orig = nvram_safe_get(strcat_r(prefix_b, "ssid", tmp));
							nvram_set(strcat_r(prefix, "ssid", tmp), orig);

							orig = nvram_safe_get(strcat_r(prefix_b, "crypto", tmp));
							nvram_set(strcat_r(prefix, "crypto", tmp), orig);

							orig = nvram_safe_get(strcat_r(prefix_b, "wep", tmp));
							nvram_set(strcat_r(prefix, "wep", tmp), orig);

							orig = nvram_safe_get(strcat_r(prefix_b, "wep_key", tmp));
							nvram_set(strcat_r(prefix, "wep_key", tmp), orig);
						}
						apply_config_to_driver_wlc(band, 0, 0);
						set_wpasupp(band, 2);
					} else
						_dprintf("skip wpasupp (%d) due invalid ssid\n", band);
				} else {
					for(i=0; i<num_of_wl_if(); ++i) {
						snprintf(prefix, sizeof(prefix), "wlc%d_", i);
						if(*nvram_safe_get(strcat_r(prefix, "ssid", tmp)))
						{
							apply_config_to_driver_wlc(i, 0, 0);
							set_wpasupp(band, 2);
						}
					}	
				}
			}
		}
	} 
	else if (strcmp(script, "wpasupp")==0)	// warp ver by wpa_cli
	{
		int i = -1, t, r, band = -1;
		char reason_tmp[16];
		char tmp[128], prefix[16], prefix_b[16], *orig;
		int client_mode = dpsta_mode()||dpsr_mode() ? 1:0;
		int reload_wait = nvram_get_int("reload_wait")?:5;

		if(cmd[1])
			band = atoi(cmd[1]);

		_dprintf("%s wpasupp.. wl%d, cli:%d, reload_wait:%d\n", __func__, band, client_mode, reload_wait);

		nvram_set("wpas0_reason", "");
		nvram_set("wpas1_reason", "");
		nvram_set("wpas2_reason", "");
		

		if(action & RC_SERVICE_STOP)
		{
			if(client_mode)
				stop_wpasupp(band);
			else
				stop_wpa_supplicant();
		}

		if(action & RC_SERVICE_START)
		{
			if(client_mode) {
				set_wpasupp(band, 1);

				if(band >= 0) {
					snprintf(prefix, sizeof(prefix), "wlc%d_", band);
					if(*nvram_safe_get(strcat_r(prefix, "ssid", tmp)))
					{
						apply_config_to_driver_wlc(band, 0, 0);
						set_wpasupp(band, 2);
					} else
						_dprintf("skip wpasupp (%d) due invalid ssid\n", band);
				} else {
					for(i=0; i<num_of_wl_if(); ++i) {
						snprintf(prefix, sizeof(prefix), "wlc%d_", i);
						if(*nvram_safe_get(strcat_r(prefix, "ssid", tmp)))
						{
							apply_config_to_driver_wlc(i, 0, 0);
							set_wpasupp(band, 2);
						}
					}	
				}
			} else {  // ap mode
				if(!nvram_match("preauth", "1")) {
					_dprintf("Don't preauth in ap mode.\n");

				} else {
					_dprintf("Go preauth in ap mode.\n");
				
					eval("killall", "debug_monitor");
					eval("killall", "-9", "hostapd");

					if(band >= 0) {
						snprintf(tmp, sizeof(tmp), "wl%d_mode", band);
						nvram_set(tmp, "wet");
					} else {
						for(i=0; i<num_of_wl_if(); ++i) {
							snprintf(tmp, sizeof(tmp), "wl%d_mode", i);
							nvram_set(tmp, "wet");
						}
					}

					// convert wlcx to wlx and create config file
					if(band >= 0) {
						snprintf(prefix, sizeof(prefix), "wlc%d_", band);
						if(*nvram_safe_get(strcat_r(prefix, "ssid", tmp)))
						{
							apply_config_to_driver_wlc(band, 1, 1);
						} else
							_dprintf("skip wpasupp (%d) due invalid ssid\n", band);
					} else {
						for(i=0; i<num_of_wl_if(); ++i) {
							snprintf(prefix, sizeof(prefix), "wlc%d_", i);
							if(*nvram_safe_get(strcat_r(prefix, "ssid", tmp)))
							{
								apply_config_to_driver_wlc(i, 1, 1);
							}
						}
					}

					unload_wl();
					load_wl();
					sleep(reload_wait);

					start_hapd_wpasupp(2);  // skip hostapd

					for(t=0; t<10; ++t) {
						if(band >= 0) {
							snprintf(reason_tmp, sizeof(reason_tmp), "wpas%d_reason", band);
							if(*nvram_safe_get(reason_tmp))
								break;
						} else {
							r = 0;
							for(i=0; i<num_of_wl_if(); ++i) {
								snprintf(reason_tmp, sizeof(reason_tmp), "wpas%d_reason", i);
								if(*nvram_safe_get(reason_tmp))
									++r;
							}
							if(r == num_of_wl_if())
								break;
						}
					}
					_dprintf("\n..%ds elapsed.\n", t);
					_dprintf("chk wpas0 reason=%s\n", nvram_safe_get("wpas0_reason"));
					_dprintf("chk wpas1 reason=%s\n", nvram_safe_get("wpas1_reason"));
					_dprintf("chk wpas2 reason=%s\n", nvram_safe_get("wpas2_reason"));

					// recover ap setting
					if(band >= 0) {
						snprintf(prefix, sizeof(prefix), "wl%d_", band);
						snprintf(prefix_b, sizeof(prefix_b), "wlb%d_", band);

						nvram_set(strcat_r(prefix, "mode", tmp), "ap");

						orig = nvram_safe_get(strcat_r(prefix_b, "akm", tmp));
						nvram_set(strcat_r(prefix, "akm", tmp), orig);

						orig = nvram_safe_get(strcat_r(prefix_b, "wpa_psk", tmp));
						nvram_set(strcat_r(prefix, "wpa_psk", tmp), orig);

						orig = nvram_safe_get(strcat_r(prefix_b, "ssid", tmp));
						nvram_set(strcat_r(prefix, "ssid", tmp), orig);

						orig = nvram_safe_get(strcat_r(prefix_b, "crypto", tmp));
						nvram_set(strcat_r(prefix, "crypto", tmp), orig);

						orig = nvram_safe_get(strcat_r(prefix_b, "wep", tmp));
						nvram_set(strcat_r(prefix, "wep", tmp), orig);

						orig = nvram_safe_get(strcat_r(prefix_b, "wep_key", tmp));
						nvram_set(strcat_r(prefix, "wep_key", tmp), orig);
					} else {
						for(i=0; i<num_of_wl_if(); ++i) {
							snprintf(prefix, sizeof(prefix), "wl%d_", i);
							snprintf(prefix_b, sizeof(prefix_b), "wlb%d_", i);

							nvram_set(strcat_r(prefix, "mode", tmp), "ap");

							orig = nvram_safe_get(strcat_r(prefix_b, "akm", tmp));
							nvram_set(strcat_r(prefix, "akm", tmp), orig);

							orig = nvram_safe_get(strcat_r(prefix_b, "wpa_psk", tmp));
							nvram_set(strcat_r(prefix, "wpa_psk", tmp), orig);

							orig = nvram_safe_get(strcat_r(prefix_b, "ssid", tmp));
							nvram_set(strcat_r(prefix, "ssid", tmp), orig);

							orig = nvram_safe_get(strcat_r(prefix_b, "crypto", tmp));
							nvram_set(strcat_r(prefix, "crypto", tmp), orig);

							orig = nvram_safe_get(strcat_r(prefix_b, "wep", tmp));
							nvram_set(strcat_r(prefix, "wep", tmp), orig);

							orig = nvram_safe_get(strcat_r(prefix_b, "wep_key", tmp));
							nvram_set(strcat_r(prefix, "wep_key", tmp), orig);
						}
					}

					unload_wl();
					load_wl();
					sleep(reload_wait);

					stop_wpa_supplicant();
					start_hapd_wpasupp(1);  // skip wpa_supp
					start_dhd_monitor();
				}
			}
		}
	}
#endif
#endif
	else if (strcmp(script, "wlcmode")==0)
	{
		if(cmd[1]&& (atoi(cmd[1]) != nvram_get_int("wlc_mode"))) {
			nvram_set_int("wlc_mode", atoi(cmd[1]));
			if(nvram_match("lan_proto", "dhcp") && atoi(cmd[1])==0) {
				nvram_set("lan_ipaddr", nvram_default_get("lan_ipaddr"));
			}
#if defined(RTCONFIG_QCA_LBD)
			if (nvram_match("wlc_mode", "0"))
				stop_qca_lbd();
#endif
#if defined(RTCONFIG_MTK_BSD)
			if (nvram_match("wlc_mode", "0"))
				stop_mtk_bs20();
#endif

#if (defined(RTCONFIG_SAMBASRV) || defined(RTCONFIG_TUXERA_SMBD)) && defined(RTCONFIG_FTP)
			stop_ftpd(0);
			stop_samba(0);
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
#ifdef RTCONFIG_CONNDIAG
			stop_conn_diag();
#endif
			stop_cfgsync();
#endif
#if defined(RTCONFIG_AMAS)
			stop_amas_lib();
#endif
#ifdef RTCONFIG_MULTILAN_CFG
			stop_dnsmasq(ALL_SDN);
#else
			stop_dnsmasq();
#endif
			stop_lan_wlc();
			stop_lan_port();
			stop_lan_wlport();
			start_lan_wlport();
			start_lan_port(8);
			start_lan_wlc();
#ifdef RTCONFIG_MULTILAN_CFG
			start_dnsmasq(ALL_SDN);
#else
			start_dnsmasq();
#endif
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
#if (defined(RTCONFIG_SAMBASRV) || defined(RTCONFIG_TUXERA_SMBD)) && defined(RTCONFIG_FTP)
			setup_passwd();
			start_samba();
			start_ftpd();
#endif
#if defined(RTCONFIG_QCA_LBD)
			if (nvram_match("wlc_mode", "1"))
				start_qca_lbd();
#endif
#if defined(RTCONFIG_MTK_BSD)
			if (nvram_match("wlc_mode", "1"))
				start_mtk_bs20();
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
#ifdef RTCONFIG_SPEEDTEST
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
		if (strlen(script) == 10) {
			if (action & RC_SERVICE_STOP) stop_ovpn_client(atoi(&script[9]));
			if (action & RC_SERVICE_START) start_ovpn_client(atoi(&script[9]));
		}
	}
	else if (strncmp(script, "vpnserver" ,9) == 0) {
		if (strlen(script) == 10) {
			if (action & RC_SERVICE_STOP) stop_ovpn_server(atoi(&script[9]));
			if (action & RC_SERVICE_START) start_ovpn_server(atoi(&script[9]));
		}
	}
	else if (strncmp(script, "vpnrouting" ,10) == 0) {
		int unit, lock;

		if (action & RC_SERVICE_START) {
			if (strlen(script) >= 11)
				unit = atoi(&script[10]);
			else
				unit = 0;

			lock = file_lock(VPNROUTING_LOCK);
			if (unit == 0) {
				amvpn_set_wan_routing_rules();
#ifdef RTCONFIG_WIREGUARD
				for (i = WG_CLIENT_MAX; i > 0; i--) {
					amvpn_set_routing_rules(i, VPNDIR_PROTO_WIREGUARD);
					amvpn_clear_exclusive_dns(i, VPNDIR_PROTO_WIREGUARD);
					wgc_set_exclusive_dns(i);
				}
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_BCM_502L07P2) || defined(RTCONFIG_HND_ROUTER_AX_675X)
				amvpn_refresh_wg_bypass_rules();
#endif
#endif
				for (i = OVPN_CLIENT_MAX; i > 0; i --) {
					amvpn_set_routing_rules(i, VPNDIR_PROTO_OPENVPN);
					amvpn_clear_exclusive_dns(i, VPNDIR_PROTO_OPENVPN);
					ovpn_set_exclusive_dns(i);
				}
			} else {
				// unit-specific only called for OpenVPN for now
				// NOTE: doing it for a single WGC would mess with other bypass rules
				amvpn_set_wan_routing_rules();
				amvpn_set_routing_rules(unit, VPNDIR_PROTO_OPENVPN);
				amvpn_clear_exclusive_dns(unit, VPNDIR_PROTO_OPENVPN);
				ovpn_set_exclusive_dns(unit);
				// Refresh prerouting rules to ensure correct order
				amvpn_update_exclusive_dns_rules();
			}
			file_unlock(lock);
		}
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
#if defined(RTCONFIG_MT798X)
		reinit_hwnat(-1);
#endif
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
		reset_ovpn_setting(OVPN_TYPE_SERVER, nvram_get_int("vpn_server_unit"), 1);
	}
        else if (strncmp(script, "clearvpnclient", 14) == 0)
	{
		reset_ovpn_setting(OVPN_TYPE_CLIENT, nvram_get_int("vpn_client_unit"), 1);
	}
#endif
#ifdef RTCONFIG_YANDEXDNS
	else if (strcmp(script, "yadns") == 0)
	{
		if (action & RC_SERVICE_STOP)
		{
#ifdef RTCONFIG_MULTILAN_CFG
			stop_dnsmasq(ALL_SDN);
#else
			stop_dnsmasq();
#endif
		}
		if (action & RC_SERVICE_START) {
			update_resolvconf();
#ifdef RTCONFIG_MULTILAN_CFG
			start_dnsmasq(ALL_SDN);
#else
			start_dnsmasq();
#endif
 		}
		start_firewall(wan_primary_ifunit(), 0);
	}
#endif
#ifdef RTCONFIG_DNSFILTER
	else if (strcmp(script, "dnsfilter") == 0)
	{
		if(action & RC_SERVICE_START) {
#ifdef RTCONFIG_MULTILAN_CFG
			start_dnsmasq(ALL_SDN);
#else
			start_dnsmasq();
#endif
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
		if(action & RC_SERVICE_STOP) stop_timemachine(0);
		if(action & RC_SERVICE_START) start_timemachine();
	}
	else if (strcmp(script, "afpd") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_afpd(0);
		if(action & RC_SERVICE_START) start_afpd();
	}
	else if (strcmp(script, "cnid_metad") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_cnid_metad(0);
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

#ifdef RTCONFIG_FRS_FEEDBACK
	else if (strcmp(script, "sendfeedback") == 0)
	{
		char *cmd[] = {"sendfeedback", NULL};
		int pid;

		if(!pids("sendfeedback"))
		{
			_eval(cmd, NULL, 0, &pid);
		}
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
		start_sendDSLdiag();
	}
#endif
#endif /* RTCONFIG_FRS_FEEDBACK */

#ifdef RTCONFIG_VPNC
#if defined(RTCONFIG_VPN_FUSION)
	else if (strcmp(script, "vpnc") == 0)	//Activate/inactivate a VPN profile
	{
		int vpnc_unit = nvram_get_int("vpnc_unit");

		//init
		vpnc_init();

		if(action & RC_SERVICE_STOP)
		{
			_dprintf("[%s]stop vpnc %d\n", __FUNCTION__, vpnc_unit);
			stop_vpnc_by_unit(vpnc_unit);
		}

		if(action & RC_SERVICE_START)
		{
			_dprintf("[%s]start vpnc %d\n", __FUNCTION__, vpnc_unit);
			start_vpnc_by_unit(vpnc_unit);
		}
#if defined(RTCONFIG_MT798X)
		reinit_hwnat(-1);
#endif
#if defined(RTCONFIG_BWDPI) && (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
		/* It's a workaround for QCA / MTK platform due to accelerator / module / vpn can't work together */
		start_dpi_engine_service();
#endif
	}
	else if (strcmp(script, "default_wan") == 0)	//change default WAN
	{
		_dprintf("[%s]change default wan\n", __FUNCTION__);

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
#ifdef RTCONFIG_TUNNEL
			stop_aae_sip_conn(1);
#endif
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
#if defined(RTCONFIG_HMA)
			else if(nvram_match("vpnc_proto", "hma")){
				char *nv = NULL, *nvp = NULL, *b = NULL;
				int unit = 0;
				char *desc, *proto, *server, *username, *passwd, *active, *vpnc_idx, *region, *conntype;
				char cmd[256] = {0};

				nv = nvp = strdup(nvram_safe_get("vpnc_clientlist"));
				while (nv && (b = strsep(&nvp, "<")) != NULL) {
					if (vstrsep(b, ">", &desc, &proto, &server, &username, &passwd, &active, &vpnc_idx, &region, &conntype) < 9)
						continue;

					if (server && (unit = atoi(server)) != openvpnc_unit)
						continue;

					if (is_tpvpn_configured(TPVPN_HMA, region, conntype, unit))
						start_ovpn_client(unit);
					else {
						snprintf(cmd, sizeof(cmd), "hmavpn setconf '%s' '%s' %d &"
							, region, conntype, unit);
						system(cmd);
					}
					break;
				}
				free(nv);

				stop_vpnc();
			}
#endif
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
#if defined(RTCONFIG_BWDPI) && (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
		/* It's a workaround for QCA / MTK platform due to accelerator / module / vpn can't work together */
		start_dpi_engine_service();
#endif
		}
	}
#endif
#ifdef RTCONFIG_TR069
	else if (strncmp(script, "tr", 2) == 0) {
		if (action & RC_SERVICE_STOP) stop_tr();
		if (action & RC_SERVICE_START) start_tr();
		start_firewall(wan_primary_ifunit(), 0);
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

#ifdef RTAC68U
		if (!hw_usb_cap())
			return;
#endif

		_dprintf("xhcimode: stop_usb_program...\n");
		stop_usb_program(1);

		_dprintf("xhcimode: remove xhci...\n");

		remove_kmods(POST_XHCI_KMODS);
		modprobe_r(USB30_MOD);
		// remove_kmods(PRE_XHCI_KMODS);

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

		// load_kmods(PRE_XHCI_KMODS);
		modprobe(USB30_MOD, param);
		load_kmods(POST_XHCI_KMODS);

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
				start_firewall(wan_primary_ifunit(), 0);
#ifdef RTCONFIG_MULTILAN_CFG
				start_dnsmasq(ALL_SDN);
#else
				start_dnsmasq();
#endif
#if defined(RTCONFIG_BWDPI) && (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
		/* It's a workaround for QCA / MTK platform due to accelerator / module / vpn can't work together */
				start_dpi_engine_service();
#endif
#if defined(RTCONFIG_SAMBASRV) || defined(RTCONFIG_TUXERA_SMBD)
		start_write_smb_conf();
#endif /* RTCONFIG_SAMBASRV */
        } else if(0 == strcmp(script, "ipsec_start")){
		rc_ipsec_set(IPSEC_START,PROF_SVR);
		start_firewall(wan_primary_ifunit(), 0);
#ifdef RTCONFIG_MULTILAN_CFG
		start_dnsmasq(ALL_SDN);
#else
		start_dnsmasq();
#endif
#if defined(RTCONFIG_BWDPI) && (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
		/* It's a workaround for QCA / MTK platform due to accelerator / module / vpn can't work together */
		start_dpi_engine_service();
#endif
#if defined(RTCONFIG_SAMBASRV) || defined(RTCONFIG_TUXERA_SMBD)
		start_write_smb_conf();
#endif /* RTCONFIG_SAMBASRV */
        } else if(0 == strcmp(script, "ipsec_stop")){
		rc_ipsec_set(IPSEC_STOP,PROF_SVR);
#if defined(RTCONFIG_SAMBASRV) || defined(RTCONFIG_TUXERA_SMBD)
		start_write_smb_conf();
#endif /* RTCONFIG_SAMBASRV */
        } else if(0 == strcmp(script, "ipsec_restart")){
		if(cmd[1])
		{
			if(strcmp(cmd[1], "clearCAfiles") == 0)
			{
				_dprintf("clear CA files\n");
				unlink(FILE_PATH_CA_ETC FILE_NAME_CERT_PEM);
				unlink(FILE_PATH_CA_ETC FILE_NAME_SVR_CERT_PEM);
			}
		}

		rc_ipsec_set(IPSEC_SET,PROF_SVR);
		start_firewall(wan_primary_ifunit(), 0);
#ifdef RTCONFIG_MULTILAN_CFG
		start_dnsmasq(ALL_SDN);
#else
		start_dnsmasq();
#endif
#if defined(RTCONFIG_BWDPI) && (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
		/* It's a workaround for QCA / MTK platform due to accelerator / module / vpn can't work together */
		start_dpi_engine_service();
#endif
#if defined(RTCONFIG_SAMBASRV) || defined(RTCONFIG_TUXERA_SMBD)
		start_write_smb_conf();
#endif /* RTCONFIG_SAMBASRV */
        } else if(0 == strcmp(script, "ipsec_set_cli")){
		rc_ipsec_set(IPSEC_SET,PROF_CLI);
        } else if(0 == strcmp(script, "ipsec_start_cli")){
		rc_ipsec_set(IPSEC_START,PROF_CLI);
#if defined(RTCONFIG_BWDPI) && (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
		/* It's a workaround for QCA / MTK platform due to accelerator / module / vpn can't work together */
		start_dpi_engine_service();
#endif
#if defined(RTCONFIG_SAMBASRV) || defined(RTCONFIG_TUXERA_SMBD)
		start_write_smb_conf();
#endif /* RTCONFIG_SAMBASRV */
        } else if(0 == strcmp(script, "ipsec_stop_cli")){
		rc_ipsec_set(IPSEC_STOP,PROF_CLI);
#if defined(RTCONFIG_SAMBASRV) || defined(RTCONFIG_TUXERA_SMBD)
		start_write_smb_conf();
#endif /* RTCONFIG_SAMBASRV */
        } else if(0 == strcmp(script, "ipsec_gen_cert")){
            rc_ipsec_gen_cert(0);
        } else if(0 == strcmp(script, "ipsec_force_gen_cert")){
            rc_ipsec_gen_cert(1);
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
		set_fbwifi_profile();
	}
	else if (strcmp(script, "overwrite_fbwifi_ssid") == 0)
	{
		overwrite_fbwifi_ssid();
	}
#endif
#ifdef RTCONFIG_LETSENCRYPT
	else if (strcmp(script, "letsencrypt") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_letsencrypt();
		if(action & RC_SERVICE_START) start_letsencrypt();
	}
#endif
#ifdef RTCONFIG_SOFTWIRE46
	else if (strcmp(script, "auto46det") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_auto46det();
		if(action & RC_SERVICE_START) start_auto46det();
	}
	else if (strcmp(script, "v6plusd") == 0)
	{
		int unit = (cmd[1]) ? atoi(cmd[1]) : wan_primary_ifunit();
		if(action & RC_SERVICE_STOP) stop_v6plusd(unit);
		if(action & RC_SERVICE_START) start_v6plusd(unit);
	}
	else if (strcmp(script, "ocnvcd") == 0)
	{
		int unit = (cmd[1]) ? atoi(cmd[1]) : wan_primary_ifunit();
		if(action & RC_SERVICE_STOP) stop_ocnvcd(unit);
		if(action & RC_SERVICE_START) start_ocnvcd(unit);
	}
	else if (strcmp(script, "s46reset") == 0)
	{
		int unit = (cmd[1]) ? atoi(cmd[1]) : wan_primary_ifunit();
		s46reset(unit);
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
	else if (strcmp(script, "httpd_uam")==0)
	{
		if(action&RC_SERVICE_STOP) stop_httpd_uam();
		if(action&RC_SERVICE_START) start_httpd_uam();
	}
	else if (strcmp(script, "fbwifi_config")==0)
	{
		if(action&RC_SERVICE_STOP) stop_fbwifi_config();
		if(action&RC_SERVICE_START) start_fbwifi_config();
	}
	else if (strcmp(script, "fbwifi_check")==0)
	{
		if(action&RC_SERVICE_STOP) stop_fbwifi_check();
		if(action&RC_SERVICE_START) start_fbwifi_check();
	}
	else if (strcmp(script, "set_fbwifi_profile") == 0)
	{
		set_fbwifi_profile();
	}
	else if (strcmp(script, "overwrite_fbwifi_ssid") == 0)
	{
		overwrite_fbwifi_ssid();
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
#ifdef RTCONFIG_REVERTFW
	else if (strcmp(script, "revertfw_release_note")==0)
	{
		snprintf(nvtmp, sizeof(nvtmp), "revert_note.sh");
		cmd[0] = nvtmp;
		start_script(count, cmd);
	}
#endif

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_BHCOST_OPT
	else if (strcmp(script,"amas_status") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_amas_status();
		if(action&RC_SERVICE_START) start_amas_status();
	}
	else if (strcmp(script,"amas_misc") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_amas_misc();
		if(action&RC_SERVICE_START) start_amas_misc();
	}
	else if (strcmp(script,"amas_ssd") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_amas_ssd();
		if(action&RC_SERVICE_START) start_amas_ssd();
	}
	else if (strcmp(script, "trigger_opt") == 0)
	{
		trigger_opt();
	}
#endif
	else if (strcmp(script,"amas_bhctrl") == 0)
	{
		if (action & RC_SERVICE_STOP) {
#ifdef RTCONFIG_AMAS_CHANNEL_PLAN
			if (nvram_get_int("channel_plan"))
				stop_amas_lanctrl();
#endif
#ifdef RTCONFIG_BHCOST_OPT
			stop_amas_status(); // reload amas_bhmode
#endif
			stop_amas_bhctrl();
		}

		if (action & RC_SERVICE_START) {
#ifdef RTCONFIG_BHCOST_OPT
			start_amas_status(); // reload amas_bhmode
#endif
			start_amas_bhctrl();
#ifdef RTCONFIG_AMAS_CHANNEL_PLAN
			if (nvram_get_int("channel_plan"))
				start_amas_lanctrl();
#endif
		}
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
	else if (strcmp(script,"obd") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_obd();
		if(action&RC_SERVICE_START) start_obd();
	}
#ifdef RTCONFIG_AWSIOT
	else if (strcmp(script,"awsiot") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_awsiot();
		if(action&RC_SERVICE_START) start_awsiot();
	}
#endif
#ifdef RTCONFIG_FPROBE
	else if (strcmp(script,"fprobe") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_fprobe();
		if(action&RC_SERVICE_START) start_fprobe();
	}
#endif
#ifdef RTCONFIG_ROUTERBOOST
        else if (strcmp(script,"routerboost") == 0)
        {
                if(action&RC_SERVICE_STOP){
					stop_asus_rbd();
					if(f_exists("/tmp/rb_sta_list.json"))
						system("rm /tmp/rb_sta_list.json");
				}
				if(action&RC_SERVICE_START) start_asus_rbd();
        }
#endif

#ifdef RTCONFIG_DNSQUERY_INTERCEPT
        else if (strcmp(script,"dnsqd") == 0)
        {
                if(action&RC_SERVICE_STOP) stop_dnsqd();
                if(action&RC_SERVICE_START) start_dnsqd();
        }
#endif
#ifdef RTCONFIG_NFCM
	else if (strcmp(script,"nfcm") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_nfcm();
		if(action&RC_SERVICE_START) start_nfcm();
	}
#endif
#ifdef RTCONFIG_ETHOBD
	else if (strcmp(script,"obd_monitor") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_obd_monitor();
		if(action&RC_SERVICE_START) start_obd_monitor();
	}
#endif
#ifdef RTCONFIG_NEW_USER_LOW_RSSI
	else if (strcmp(script,"roamast") == 0){
		if(action&RC_SERVICE_STOP) stop_roamast();
		if(action&RC_SERVICE_START) start_roamast();
	}
#endif
#ifdef RTCONFIG_CONNDIAG
	else if (strcmp(script,"conn_diag") == 0){
		if(action&RC_SERVICE_STOP) stop_conn_diag();
		if(action&RC_SERVICE_START) start_conn_diag();
	}
	else if (strcmp(script,"amas_portstatus") == 0){
		if(action&RC_SERVICE_STOP) stop_amas_portstatus();
		if(action&RC_SERVICE_START) start_amas_portstatus();
	}
#endif
	else if (strcmp(script, "apply_amaslib") == 0)
	{
		AMAS_EVENT_TRIGGER(NULL, NULL, 0);
	}
#ifdef RTCONFIG_STA_AP_BAND_BIND
	else if (strcmp(script, "update_sta_binding")==0)
	{
		extern int update_sta_binding_list(void);
		update_sta_binding_list();
	}
#endif
#ifdef RTCONFIG_NBR_RPT
	else if (strcmp(script, "update_nbr")==0)
	{
		update_nbr_list();
	}
#endif
#ifdef RTCONFIG_AMAS_CHANNEL_PLAN
	else if (strcmp(script,"channel_plan") == 0)
	{
		if (action & RC_SERVICE_STOP) {
			stop_amas_lanctrl();
			if (nvram_get_int("re_mode") == 1) {
				stop_amas_status(); // reload amas_bhmode
				stop_amas_bhctrl();
			}
		}

		if (action & RC_SERVICE_START) {
			if (nvram_get_int("re_mode") == 1) {
				start_amas_status(); // reload amas_bhmode
				start_amas_bhctrl();
			}
			start_amas_lanctrl();
		}
    }
#endif
#endif
#ifdef RTCONFIG_UPLOADER
	else if (strcmp(script,"uploader") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_uploader();
		if(action&RC_SERVICE_START) start_uploader();
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
	else if (strcmp(script, "reset_led") == 0)
	{
#if defined(BLUECAVE)
		reset_led();
#elif defined(RTCONFIG_LP5523)
		lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);
#endif
	}
#if defined(RTCONFIG_RGBLED)
	else if (strcmp(script, "aurargb") == 0)
	{
		if(action&RC_SERVICE_START) {
			start_aurargb();
		}
	}
#endif
#if defined(BCM_BSD) || defined(LANTIQ_BSD) 
	else if (strcmp(script,"bsd") == 0)
	{
		if(action&RC_SERVICE_STOP) stop_bsd();
		if(action&RC_SERVICE_START) start_bsd();
	}
#endif
#if defined(RTCONFIG_RGBLED)
	else if (strcmp(script, "aurargb") == 0)
	{
		if(action&RC_SERVICE_START) start_aurargb();
	}
#endif
	else if (strcmp(script, "clean_web_history") == 0)
	{
		remove("/jffs/.sys/WebHistory/WebHistory.db");
		remove("/jffs/.sys/WebHistory/WebHistory.db-journal");
	}
	else if (strcmp(script, "clean_traffic_analyzer") == 0)
	{
		remove("/jffs/.sys/TrafficAnalyzer/TrafficAnalyzer.db");
		remove("/jffs/.sys/TrafficAnalyzer/TrafficAnalyzer.db-journal");
	}
	else if (strcmp(script, "clean_backup_log") == 0)
	{
		// TODO : add path here
	}
#ifdef RTCONFIG_EXTPHY_BCM84880
	else if (strcmp(script, "br_addif") == 0)
        {
#if !(defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(GTBE19000) || defined(RTBE92U)) || defined(RTBE95U)
		if(!nvram_match("x_Setting", "0")) {
			eval("brctl", "addif", nvram_safe_get("lan_ifname"), "eth5");
		}
#endif
        }
#endif
#if defined(RTCONFIG_HND_ROUTER_AX)
	else if (strcmp(script, "cable_media") == 0)
        {
		// cmd[1]: ethernet interface, cmd[2]: media type
		eval("ethctl", cmd[1], "media-type", cmd[2]);
        }
#endif
#if defined(RTCONFIG_MTK_BSD)
	else if (strcmp(script, "mtk_bs20") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_mtk_bs20();
		if(action & RC_SERVICE_START) start_mtk_bs20();
	}
#endif
#if defined(RTCONFIG_QCA_LBD)
	else if (strcmp(script, "qca_lbd") == 0 || strcmp(script, "bsd") == 0)
	{
		if(action & RC_SERVICE_STOP) stop_qca_lbd();
		if(action & RC_SERVICE_START) start_qca_lbd();
	}
#endif
#ifdef RTCONFIG_IPERF3
	else if (strcmp(script, "iperf3_server") == 0)
	{
		if(action & RC_SERVICE_STOP){
			stop_iperf3();
			nvram_set("iperf3_svr_enable", "0");
		}
		if(action & RC_SERVICE_START) {
			start_iperf3_server();
			nvram_set("iperf3_svr_enable", "1");
		}
	}
	else if (strcmp(script, "iperf3_client") == 0)
	{
		if(action & RC_SERVICE_STOP){
			stop_iperf3();
			nvram_set("iperf3_cli_enable", "0");
		}
		if(action & RC_SERVICE_START) {
			start_iperf3_client();
			nvram_set("iperf3_cli_enable", "1");
		}
	}
#endif
#ifdef RTCONFIG_SW_CTRL_ALLLED
	else if (strcmp(script, "ctrl_led") == 0)
	{
		int led_val = nvram_get_int("led_val");
		int AllLED = nvram_get_int("AllLED");

		/* off */
		if (led_val == 0) {
			nvram_set("AllLED", "0");
			setAllLedOff();
		}
		/* on/brightness */
		else {
			if (AllLED == 0) {
				nvram_set("AllLED", "1");
				setAllLedNormal();
			}
			if (led_val > 1) {
				nvram_set_int("AllLED_brightness", led_val);
				setAllLedBrightness();
			}
		}
		nvram_commit();
	}
#endif
	else if (strcmp(script, "watchdog") == 0)
	{
		if (action & RC_SERVICE_STOP) stop_watchdog();
		if (action & RC_SERVICE_START) start_watchdog();
	}
// add by Andrew
    else if (strcmp(script, "alt_watchdog") == 0)
    {
        if (action & RC_SERVICE_STOP) stop_alt_watchdog();
        if (action & RC_SERVICE_START) start_alt_watchdog();
    }
// end of add
#if defined(RTAX82U) || defined(DSL_AX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTAX6000) || defined(GT10) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(GTBE96) || defined(GTBE19000)
	else if (strcmp(script, "ledg") == 0)
	{
		if (action & RC_SERVICE_STOP) stop_ledg();
		if (action & RC_SERVICE_START) start_ledg();
	}
#endif
	else if (strcmp(script, "jffs2") == 0)
	{
		int stop = (count > 1) ? atoi(cmd[1]) : 1;
		dbg("stop: %d\n", stop);
		if (action & RC_SERVICE_STOP) stop_jffs2(stop);
	}
#ifdef RTCONFIG_BCM_OAM
	else if (strncmp(script, "oam", 3) == 0) {
		if (action & RC_SERVICE_STOP) stop_oam();
		if (action & RC_SERVICE_START) start_oam();
	}
#endif
#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
	else if (strcmp(script, "plc") == 0) {
		if (action & RC_SERVICE_STOP) reset_plc(1);
		if (action & RC_SERVICE_START) {
			start_plchost();
			start_detect_plc();
		}
	}
	else if (strcmp(script, "detect_plc") == 0) {
		if (action & RC_SERVICE_STOP) stop_detect_plc();
		if (action & RC_SERVICE_START) start_detect_plc();
	}
#endif	/* RTCONFIG_QCA_PLC_UTILS || RTCONFIG_QCA_PLC2 */
#if defined(RTCONFIG_SCHED_DAEMON)
	else if (strcmp(script, "sched_daemon") == 0)
	{
		if (action & RC_SERVICE_STOP) stop_sched_daemon();
		if (action & RC_SERVICE_START) start_sched_daemon();
	}
#endif
#ifdef RTCONFIG_WIREGUARD
	else if (strcmp(script, "wgs") == 0) {
		if (cmd[1]) {
			int unit = atoi(cmd[1]);
			if (action & RC_SERVICE_STOP) stop_wgs(unit);
			if (action & RC_SERVICE_START) start_wgs(unit);
		}
		else {
			if (action & RC_SERVICE_STOP) stop_wgsall();
			if (action & RC_SERVICE_START) start_wgsall();
		}
	}
	else if (strcmp(script, "wgsc") == 0) {
		if (cmd[1]) {
			if (cmd[2]) {
				int s_unit = atoi(cmd[1]);
				int c_unit = atoi(cmd[2]);
				update_wgs_client(s_unit, c_unit);
			}
			else {
				int c_unit = atoi(cmd[1]);
				update_wgs_client(1, c_unit);
			}
		}
#if defined(RTCONFIG_BWDPI) && (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
		/* It's a workaround for QCA / MTK platform due to accelerator / module / vpn can't work together */
		start_dpi_engine_service();
#endif
	}
	else if (strcmp(script, "wgc") == 0) {
		if (cmd[1]) {
			int unit = atoi(cmd[1]);
			if (action & RC_SERVICE_STOP) stop_wgc(unit);
			if (action & RC_SERVICE_START) start_wgc(unit);
		}
		else {
			if (action & RC_SERVICE_STOP) stop_wgcall();
			if (action & RC_SERVICE_START) start_wgcall();
		}
#if defined(RTCONFIG_BWDPI) && (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
		/* It's a workaround for QCA / MTK platform due to accelerator / module / vpn can't work together */
		start_dpi_engine_service();
#endif
	}
#endif
#ifdef RTCONFIG_CFGSYNC
	else if (strcmp(script, "firmware_webs_update") == 0)
	{
		FWUPDATE_DBG("rc triger firmware_webs_update");
		int webs_cfg_check = 0, webs_retry = 0;
		nvram_set("webs_update_trigger", "WDG_CFG");
		nvram_unset("cfg_check");
		char event_msg[64] = {0};
		snprintf(event_msg, sizeof(event_msg), HTTPD_GENERIC_MSG, EID_HTTPD_FW_CHECK);
		if (strlen(event_msg))
			send_cfgmnt_event(event_msg);

		while(webs_retry < 30){
			webs_cfg_check = nvram_get_int("cfg_check");
			FWUPDATE_DBG("webs_cfg_check(retry %d): %d", webs_retry, webs_cfg_check);
			if(webs_cfg_check == 7){
				snprintf(event_msg, sizeof(event_msg), HTTPD_GENERIC_MSG, EID_HTTPD_FW_UPGRADE);
				if (strlen(event_msg))
					send_cfgmnt_event(event_msg);
				break;
			}
			else if(webs_cfg_check == 2 || webs_cfg_check == 3){
				FWUPDATE_DBG("can not link frs");
				break;
			}else{	//(webs_cfg_check == 0 || webs_cfg_check == 1 || webs_cfg_check == 5)
				webs_retry++;
				sleep(1);
			}
		}


	}
#endif
#ifdef RTCONFIG_QCA_PLC2
	else if (strcmp(script, "plc_master") == 0)
	{
		if (action & RC_SERVICE_STOP) stop_plc_master();
		if (action & RC_SERVICE_START) start_plc_master();
	}
#endif	/* RTCONFIG_QCA_PLC2 */
#ifdef RTCONFIG_GRE
	else if (strcmp(script, "l2gre") == 0)
	{
		int unit = (cmd[1]) ? atoi(cmd[1]) : nvram_get_int("l2gre_unit");
		if (action & RC_SERVICE_STOP) stop_l2gre(unit);
		if (action & RC_SERVICE_START) start_l2gre(unit);
	}
	else if (strcmp(script, "l3gre") == 0)
	{
		int unit = (cmd[1]) ? atoi(cmd[1]) : nvram_get_int("l3gre_unit");
		if (action & RC_SERVICE_STOP) stop_l3gre(unit);
		if (action & RC_SERVICE_START) start_l3gre(unit);
	}
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
	else if (strcmp(script, "wlc_nt") == 0)
	{
		if (action & RC_SERVICE_STOP){
			stop_wlc_monitor();
			stop_wlc_nt();
		}
		if (action & RC_SERVICE_START){
			start_wlc_nt();
			start_wlc_monitor();
		}
	}
#endif
#ifdef RTCONFIG_MULTIWAN_PROFILE
	else if (strcmp(script, "mtwan_profile") == 0)
	{
		mtwan_update_profile();
	}
#endif
#if defined(RTCONFIG_AMAS_CENTRAL_ADS)
	else if (strcmp(script, "ds_switch") == 0)
	{
		if (cmd[1])
			handle_ds_switch(atoi(cmd[1]));	/* 1: parent, 2: child */
		else
			handle_ds_switch(0);
	}
	else if (strcmp(script, "ds_measure") == 0)
	{
		handle_ds_measure();
	}
#endif
	else
	{
		fprintf(stderr,
			"WARNING: rc notified of unrecognized event `%s'.\n",
					script);
		if (nvram_get_int("rc_debug"))
			logmessage("rc", "received unrecognized event: %s", script);
	}

skip:

	run_custom_script("service-event-end", 0, actionstr, script);

	if(nvptr && strlen(nvptr)){
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
#if defined(BQ16) || defined(BQ16_PRO)
	wl_defer_conf(script);
#endif
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

	if((!repeater_mode()
#if defined(RTCONFIG_REALTEK) || defined(RTCONFIG_LANTIQ) || defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)
	 && !mediabridge_mode()
#endif
	 && !wisp_mode())
		|| nvram_get_int("x_Setting") == 0)
	{
		_dprintf("Not repeater mode, do not start_wlcconnect\n");
		return;
	}

	if(getpid()!=1) {
		notify_rc("start_wlcconnect");
		return;
	}

#if (defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)) \
 && !defined(RTCONFIG_CONCURRENTREPEATER)
	if (!pids("wlcconnect"))
		_eval(wlcconnect_argv, NULL, 0, &pid);
	else
		killall("wlcconnect", SIGCONT);
#else
	killall("wlcconnect", SIGTERM);
	_eval(wlcconnect_argv, NULL, 0, &pid);
#endif
}

void
stop_wlcconnect(void)
{
	if(getpid()!=1) {
		notify_rc("stop_wlcconnect");
		return;
	}

#if (defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)) \
 && !defined(RTCONFIG_CONCURRENTREPEATER)
	killall("wlcconnect", SIGTSTP);
#else
	killall("wlcconnect", SIGTERM);
#endif
}
#endif

#ifdef RTCONFIG_AMAS
void start_amas_wlcconnect(void)
{
#ifdef RTCONFIG_SW_HW_AUTH
	char *amas_wlcconnect_argv[] = {"amas_wlcconnect", NULL};
	pid_t pid;

	if (!(getAmasSupportMode() & AMAS_RE)) {
		_dprintf("not support RE, don't start_amas_wlcconnect\n");
		return;
	}

	if (nvram_get_int("re_mode") != 1)
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
	nvram_set_int("amas_wlcconnect_service_ready", 1);
#endif /* RTCONFIG_SW_HW_AUTH */
}

void stop_amas_wlcconnect(void)
{
	if(getpid()!=1) {
		notify_rc("stop_amas_wlcconnect");
		return;
	}
	nvram_unset("amas_wlcconnect_service_ready");

	killall("amas_wlcconnect", SIGTERM);
}

#ifdef RTCONFIG_BHCOST_OPT
void start_amas_ssd(void)
{
#ifdef RTCONFIG_SW_HW_AUTH
	char *amas_ssd_argv[] = {"amas_ssd", NULL};
	pid_t pid;

	if (!(getAmasSupportMode() & AMAS_RE)) {
		_dprintf("not support RE, don't start_amas_ssd\n");
		return;
	}

	if (nvram_get_int("re_mode") != 1)
	{
		_dprintf("Not AMAS RE mode, do not start_amas_ssd\n");
		return;
	}

	if(getpid()!=1) {
		notify_rc("start_amas_ssd");
		return;
	}

	killall("amas_ssd", SIGTERM);

	_eval(amas_ssd_argv, NULL, 0, &pid);
	nvram_set_int("amas_ssd_service_ready", 1);
#endif /* RTCONFIG_SW_HW_AUTH */
}

void stop_amas_ssd(void)
{
	if(getpid()!=1) {
		notify_rc("stop_amas_ssd");
		return;
	}
	nvram_unset("amas_ssd_service_ready");

	killall("amas_ssd", SIGTERM);
}

void start_amas_status(void)
{
#ifdef RTCONFIG_SW_HW_AUTH
	char *amas_status_argv[] = {"amas_status", NULL};
	pid_t pid;

	if (!(getAmasSupportMode() & AMAS_RE)) {
		_dprintf("not support RE, don't start_amas_status\n");
		return;
	}

	if (nvram_get_int("re_mode") != 1)
	{
		_dprintf("Not AMAS RE mode, do not start_amas_status\n");
		return;
	}

	if(getpid()!=1) {
		notify_rc("start_amas_status");
		return;
	}

	killall("amas_status", SIGTERM);

	_eval(amas_status_argv, NULL, 0, &pid);
	nvram_set_int("amas_status_service_ready", 1);
#endif /* RTCONFIG_SW_HW_AUTH */
}

void stop_amas_status(void)
{
	if(getpid()!=1) {
		notify_rc("stop_amas_status");
		return;
	}
	nvram_unset("amas_status_service_ready");

	killall("amas_status", SIGTERM);
}

void start_amas_misc(void)
{
#ifdef RTCONFIG_SW_HW_AUTH
	char *amas_misc_argv[] = {"amas_misc", NULL};
	pid_t pid;

	if (!(getAmasSupportMode() & AMAS_RE)) {
		_dprintf("not support RE, don't start_amas_misc\n");
		return;
	}

	if (nvram_get_int("re_mode") != 1)
	{
		_dprintf("Not AMAS RE mode, do not start_amas_misc.\n");
		return;
	}

	if(getpid()!=1) {
		notify_rc("start_amas_misc");
		return;
	}
	_dprintf("stop_amas_misc %s %d\n",__FUNCTION__,__LINE__);
	killall("amas_misc", SIGTERM);

	_eval(amas_misc_argv, NULL, 0, &pid);
	nvram_set_int("amas_misc_service_ready", 1);
#endif /* RTCONFIG_SW_HW_AUTH */
}

void stop_amas_misc(void)
{
	if(getpid()!=1) {
		notify_rc("stop_amas_misc");
		return;
	}
	nvram_unset("amas_misc_service_ready");

	killall("amas_misc", SIGTERM);
}


#endif
void start_amas_bhctrl(void)
{
#ifdef RTCONFIG_SW_HW_AUTH
	char *amas_bhctrl_argv[] = {"amas_bhctrl", NULL};
	pid_t pid;

	if (!(getAmasSupportMode() & AMAS_RE)) {
		_dprintf("not support RE, don't start_amas_bhctrl\n");
		return;
	}

	if (nvram_get_int("re_mode") != 1)
	{
		_dprintf("Not AMAS RE mode, do not start_amas_bhctrl\n");
		return;
	}

	if(getpid()!=1) {
		notify_rc("start_amas_bhctrl");
		return;
	}

	killall("amas_bhctrl", SIGTERM);

	_eval(amas_bhctrl_argv, NULL, 0, &pid);
	nvram_set_int("amas_bhctrl_service_ready", 1);
#endif /* RTCONFIG_SW_HW_AUTH */
}

void stop_amas_bhctrl(void)
{
	if(getpid()!=1) {
		notify_rc("stop_amas_bhctrl");
		return;
	}
	nvram_unset("amas_bhctrl_service_ready");

	killall("amas_bhctrl", SIGTERM);
}
void start_amas_lanctrl(void)
{
#ifdef RTCONFIG_SW_HW_AUTH
	char *amas_lanctrl_argv[] = {"amas_lanctrl", NULL};
	pid_t pid;

	if (!(getAmasSupportMode() & (AMAS_CAP | AMAS_RE))) {
		_dprintf("not support CAP or RE, don't start_amas_lanctrl\n");
		return;
	}

	if(getpid()!=1) {
		notify_rc("start_amas_lanctrl");
		return;
	}
	_dprintf("stop_amas_lanctrl %s %d\n",__FUNCTION__,__LINE__);
	killall("amas_lanctrl", SIGTERM);

	_eval(amas_lanctrl_argv, NULL, 0, &pid);
	nvram_set_int("amas_lanctrl_service_ready", 1);
#endif /* RTCONFIG_SW_HW_AUTH */
}

void stop_amas_lanctrl(void)
{
	if(getpid()!=1) {
		notify_rc("stop_amas_lanctrl");
		return;
	}
	nvram_unset("amas_lanctrl_service_ready");

	killall("amas_lanctrl", SIGTERM);
}

void gen_lldpd_if(char *bind_ifnames)
{
	char word[64], *next = NULL;
	int i = 0;
#ifdef HND_ROUTER
	char *lacp_ifs = nvram_get_int("lacp_enabled")?nvram_safe_get("lacp_ifnames"):NULL;
#endif
	char *wan_ifname =nvram_safe_get("wan0_ifname");
	char *eth_ifnames =nvram_safe_get("eth_ifnames");
#if defined(RTCONFIG_AMAS_WGN) && defined(WGN_HAVE_VLAN0) || defined(RTCONFIG_MULTILAN_CFG) && defined(APG_HAVE_VLAN0)
	char dpsta_ifname[32];
	int wgn_enabled = 0, apg_enabled = 0;
#endif
#ifdef RTCONFIG_FRONTHAUL_DWB
       char fh_ifname[16] = {};
#endif
	char *ptr_bind_ifnames = bind_ifnames;
#if defined(RTCONFIG_BCMARM) && defined(RTCONFIG_PROXYSTA)
	int j = 0;
#endif
	/* prepare binding interface list */

	if (nvram_get_int("x_Setting") == 0)
	{
		//skip no wan port model
		if( !strstr(nvram_safe_get("wans_cap"), "wan"))
			return;

		if(strcmp(wan_ifname, ""))
		{
			if (i == 0)
			{
				bind_ifnames += sprintf(bind_ifnames, "%s", wan_ifname);
				i = 1;
			}
			else
			{
				bind_ifnames += sprintf(bind_ifnames, ",%s", wan_ifname);
			}
		}

		foreach (word, eth_ifnames, next) {
			if (i == 0) {
				bind_ifnames += sprintf(bind_ifnames, "%s", word);
				i = 1;
			}
			else
			{
				if (strstr(ptr_bind_ifnames, word))
					continue;

				bind_ifnames += sprintf(bind_ifnames, ",%s", word);
			}
		}
	}
	else
	{
		if (nvram_get_int("re_mode") == 1)
		{
			/* for lan_ifnames */
			foreach (word, nvram_safe_get("lan_ifnames"), next) {

	#ifdef HND_ROUTER
				if(lacp_ifs && strstr(lacp_ifs, word))
					continue;
	#endif
#ifdef RTCONFIG_FRONTHAUL_DWB
				if (nvram_get_int("fh_ap_enabled") > 0) {
					snprintf(fh_ifname, sizeof(fh_ifname), "wl%d.%d", nvram_get_int("dwb_band"), nvram_get_int("fh_re_mssid_subunit"));
					if (!strcmp(fh_ifname, word))
						continue;
				}
#endif
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
#if defined(RTCONFIG_AMAS_WGN) && defined(WGN_HAVE_VLAN0) || defined(RTCONFIG_MULTILAN_CFG) && defined(APG_HAVE_VLAN0)
				/* add interface for guest network enabled */
#if defined(RTCONFIG_AMAS_WGN)
				wgn_enabled = nvram_get_int("wgn_enabled");
#endif
#if defined(RTCONFIG_MULTILAN_CFG)
				apg_enabled = (nvram_get_int(NV_APG_STARTED) == 1) ? 1 : 0;
#endif
				if (wgn_enabled == 1 || apg_enabled == 1) {
					bind_ifnames += sprintf(bind_ifnames, ",");
					memset(dpsta_ifname, 0, sizeof(dpsta_ifname));
					snprintf(dpsta_ifname, sizeof(dpsta_ifname), "%s.0", word);
					bind_ifnames += sprintf(bind_ifnames, "%s", dpsta_ifname);
				}
#endif		
			}
		}
		else
		{
#if defined(RTCONFIG_AUTO_WANPORT) && !defined(RTCONFIG_BCM_MFG)
			char autowan_detected_if[32];

			strlcpy(autowan_detected_if, nvram_safe_get("autowan_detected_ifname"), sizeof(autowan_detected_if));
#endif

			/* for lan_ifnames */
			foreach (word, nvram_safe_get("lan_ifnames"), next) {
#if defined(RTCONFIG_AUTO_WANPORT) && !defined(RTCONFIG_BCM_MFG)
				if(strstr(nvram_safe_get("autowan_ifnames"), word)){
					_dprintf("%s: skip binding interface %s for lldpd\n", __func__, word);
					continue;
				}
#endif

	#ifdef HND_ROUTER
				if(lacp_ifs && strstr(lacp_ifs, word))
					continue;
	#endif
#ifdef RTCONFIG_FRONTHAUL_DWB
				if (nvram_get_int("fh_ap_enabled") > 0) {
					snprintf(fh_ifname, sizeof(fh_ifname), "wl%d.%d", nvram_get_int("dwb_band"), nvram_get_int("fh_cap_mssid_subunit"));
					if (!strcmp(fh_ifname, word))
						continue;
				}
#endif
				if (i == 0)
					i = 1;
				else
					bind_ifnames += sprintf(bind_ifnames, ",");
				bind_ifnames += sprintf(bind_ifnames, "%s", word);
			}

#if defined(RTCONFIG_AUTO_WANPORT) && !defined(RTCONFIG_BCM_MFG)
			foreach (word, nvram_safe_get("autowan_ifnames"), next) {
				if(is_auto_wanport_enabled() == 2 && !strcmp(autowan_detected_if, word)){
					_dprintf("%s: skip binding interface %s for lldpd\n", __func__, word);
					continue;
				}

				if (i == 0)
					i = 1;
				else
					bind_ifnames += sprintf(bind_ifnames, ",");
				bind_ifnames += sprintf(bind_ifnames, "%s", word);
			}
#endif
		}


#if defined(RTCONFIG_BCMARM) && defined(RTCONFIG_PROXYSTA)
		for (j = 0; j < num_of_wl_if(); j++) {
			if (i == 0)
				i = 1;
			else
				bind_ifnames += sprintf(bind_ifnames, ",");
#if defined(HND_ROUTER)
			bind_ifnames += sprintf(bind_ifnames, "wds%d.*.*", j);
#else
			bind_ifnames += sprintf(bind_ifnames, "wds%d.*", j);
#endif
		}
#endif
	}
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

void update_amas_lldpd_ifnames(void)
{
#if defined(RTCONFIG_AMAS_WGN) || defined(RTCONFIG_MULTILAN_CFG)
	char ori_bind_ifnames[256];
#endif	// // RTCONFIG_AMAS_WGN || RTCONFIG_MULTILAN_CFG
	char bind_ifnames[256];
	
	if (!pids("lldpd"))
		return;
	
	memset(bind_ifnames, 0, sizeof(bind_ifnames));
	gen_lldpd_if(&bind_ifnames[0]);
#if defined(RTCONFIG_AMAS_WGN) || defined(RTCONFIG_MULTILAN_CFG)
	memset(ori_bind_ifnames, 0, sizeof(ori_bind_ifnames));
	if (f_read_string("/tmp/lldpd_bind_ifnames", ori_bind_ifnames, sizeof(ori_bind_ifnames)) > 0) {
		if (!strcmp(bind_ifnames, ori_bind_ifnames)) {
			_dprintf("rc: ==> binding interface match lldpd_bind_ifnames(%s)...\n", ori_bind_ifnames);
			return;
		}
	}
#endif  // RTCONFIG_AMAS_WGN || RTCONFIG_MULTILAN_CFG

	if (strlen(bind_ifnames) > 0) {
		if (amas_set_lldpd_bind_ifnames_retry(bind_ifnames,3)==AMAS_RESULT_SUCCESS) {
#if defined(RTCONFIG_AMAS_WGN) || defined(RTCONFIG_MULTILAN_CFG)
        	f_write_string("/tmp/lldpd_bind_ifnames", bind_ifnames, 0, 0);
#endif  // RTCONFIG_AMAS_WGN || RTCONFIG_MULTILAN_CFG
		}
		else {
			_dprintf("rc: ==> lldpd binding interface(%s) fail ...\n", bind_ifnames);
		}	
	}

	return;
}

void start_amas_lldpd(void)
{
	char bind_ifnames[256];
/* TODO: clarify do we need to use lan_hostname instead of productid here */
	char *productid = nvram_safe_get("productid");
	int i = 0, isWan = 0;
	char buf[64] = {}, tmp[16] = {};
	char ifnames[64] ,word[256], *next;
	memset(bind_ifnames, 0x00, sizeof(bind_ifnames));

#if defined(RTCONFIG_AMAS_WGN) || defined(RTCONFIG_MULTILAN_CFG)
	char lldpd_bind_ifnames[128] = {0};
	memset(lldpd_bind_ifnames, 0, sizeof(lldpd_bind_ifnames));
#endif

	init_x_Setting = nvram_get_int("x_Setting");

#ifdef RTCONFIG_BCM_MFG
	return;
#endif
#ifdef RTCONFIG_MFGFW
	if(nvram_match("mfgfw", "1"))
		return;
#endif
#if defined(RTCONFIG_WIFI_SON)
	if (nvram_match("wifison_ready", "1"))
		return;
#endif

#ifdef RTCONFIG_CONCURRENTREPEATER
	if ((repeater_mode() && nvram_get_int("x_Setting")) || mediabridge_mode())
#else
	if (repeater_mode() || mediabridge_mode())
#endif
		return;

#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
	if (psr_mode())
		return;
#endif

#ifdef RTCONFIG_SW_HW_AUTH
	if (!(getAmasSupportMode() & (AMAS_CAP | AMAS_RE))) {
		_dprintf("not support CAP/RE, don't start amas lldpd\n");
		return;
	}
#endif

#if defined(RTCONFIG_AMAS)
#if defined(RTCONFIG_DPSTA)
	if (dpsta_mode() && !nvram_get_int("re_mode") && nvram_get_int("x_Setting"))
		return;
#endif
        if (rp_mode() && !nvram_get_int("re_mode") && nvram_get_int("x_Setting"))
                return;
#endif

	if(nvram_match("stop_amas_lldpd", "1")) {
		_dprintf("stop_amas_lldpd = 1, don't start amas lldpd.\n");
		return;
	}

	if(getpid()!=1) {
		notify_rc("start_amas_lldpd");
		return;
	}

	gen_lldpd_if(&bind_ifnames[0]);

	if (strlen(bind_ifnames) == 0) {
		_dprintf("rc: ==> not binding interface for lldpd\n");
		return;
	}

	_dprintf("rc: ==> binding interface(%s) for lldpd\n", bind_ifnames);

#if defined(RTCONFIG_AMAS_WGN) || defined(RTCONFIG_MULTILAN_CFG)
	if (f_read_string("/tmp/lldpd_bind_ifnames", lldpd_bind_ifnames, sizeof(lldpd_bind_ifnames)) > 0) {
		if (strcmp(bind_ifnames, lldpd_bind_ifnames) == 0) {
			_dprintf("rc: ==> binding interface match lldpd_bind_ifnames(%s)...\n", lldpd_bind_ifnames);
			if (pids("lldpd")) {
				return;
			}
		}		
	}
#endif	// RTCONFIG_AMAS_WGN

	stop_amas_lldpd();

#ifdef RTCONFIG_AVOID_TZ_ENV
	#define CHROOTDIR_LLDPD "/var/run/lldpd"
	doSystem("mkdir -p %s/etc ; ln -f /tmp/TZ %s/etc/TZ", CHROOTDIR_LLDPD, CHROOTDIR_LLDPD);	/* add a hard link to /tmp/TZ at /var/run/lldpd/etc/TZ */
#endif
	FILE *fp = NULL;
	fp = fopen("/tmp/run_lldpd.sh", "w+");
	if (!fp) {
		_dprintf("Open run_lldpd.sh fail.\n");
		return;
	}
	fprintf(fp, "#!/bin/sh\n");

	if(nvram_get_int("lldpd_dbg") == 1) {
		char exec_lldpd[1024] = {0};
		sprintf(exec_lldpd, "lldpd -L /usr/sbin/lldpcli -s %s -dddd &", productid);
		dbG("exec lldpd debug mode(%s)\n", exec_lldpd);
		system(exec_lldpd);
	}
	else
		fprintf(fp, "lldpd -L /usr/sbin/lldpcli -s %s\n", productid);

	fprintf(fp, "sleep 2\n");
	fprintf(fp, "lldpcli configure system interface pattern %s\n", bind_ifnames);
	fprintf(fp, "lldpcli configure lldp tx-interval 10\n");
	fprintf(fp, "lldpcli configure lldp tx-hold 2\n");
#if 0
	if (strcmp(productid, "") != 0) {
		fprintf(fp, "lldpcli configure system hostname %s\n", productid);
	}
	else
		fprintf(fp, "lldpcli configure system hostname Ai-Mesh\n");
#endif
	fprintf(fp, "lldpcli resume\n");

	fprintf(fp, "nvram set amascli_dbg=1\n"); // for amas-utils-cli command.
	if (nvram_get_int("re_mode") == 1)
	{
		if(nvram_get_int("cfg_cost") == 0 && strstr(nvram_safe_get("sta_phy_ifnames"), nvram_safe_get("amas_ifname")))
			fprintf(fp, "amas-utils-cli set cost -v -1\n");
		else
			fprintf(fp, "amas-utils-cli set cost -v %d\n", nvram_get_int("cfg_cost"));

		if(strcmp(nvram_safe_get("cfg_group"), "") == 0) // OB processing
		{
			strlcpy(ifnames, nvram_safe_get("eth_ifnames"), sizeof(ifnames));
			isWan = 1;
			foreach(word, ifnames, next)
			{
				if (i != 0)
					strncat(buf, ">", 1);
				snprintf(tmp, sizeof(tmp), "%s:%d", word, isWan? 2:1);
				strncat(buf, tmp, sizeof(buf) - strlen(buf) - 1);     // eth0:0>eth1:0...
				i++;
			}
			fprintf(fp, "amas-utils-cli set ethRole -s \'%s\'\n", buf);
		}
	} else
	{
		fprintf(fp, "amas-utils-cli set cost -v %d\n", nvram_get_int("cfg_cost"));
		if(!nvram_get_int("x_Setting"))
		{
			strlcpy(ifnames, nvram_safe_get("eth_ifnames"), sizeof(ifnames));
			isWan = 1;
		}
		else
		{
			strlcpy(ifnames, nvram_safe_get("wired_ifnames"), sizeof(ifnames));
		}
		foreach(word, ifnames, next)
		{
			if (i != 0)
				strncat(buf, ">", 1);
			snprintf(tmp, sizeof(tmp), "%s:%d", word, isWan? 2:1);
			strncat(buf, tmp, sizeof(buf) - strlen(buf) - 1);     // eth0:0>eth1:0...
			i++;
		}
		fprintf(fp, "amas-utils-cli set ethRole -s \'%s\'\n", buf);
	}
	fprintf(fp, "nvram set amascli_dbg=0\n");

	if (fp)
		fclose(fp);
	
#if defined(RTCONFIG_AMAS_WGN) || defined(RTCONFIG_MULTILAN_CFG)
	f_write_string("/tmp/lldpd_bind_ifnames", bind_ifnames, 0, 0);
#endif	// RTCONFIG_AMAS_WGN
	eval("sh", "/tmp/run_lldpd.sh");
}

void stop_amas_lldpd(void)
{

#ifdef RTCONFIG_CONCURRENTREPEATER
	if ((repeater_mode() && nvram_get_int("x_Setting")) || mediabridge_mode())
#else
	if (repeater_mode() || mediabridge_mode())
#endif
		return;

	if(getpid()!=1) {
		notify_rc("stop_amas_lldpd");
		return;
	}

	killall_tk("lldpcli");
	killall_tk("lldpd");

#if defined(RTCONFIG_AMAS_WGN) || defined(RTCONFIG_MULTILAN_CFG)
	system("rm -rf /tmp/lldpd_bind_ifnames");
#endif	// RTCONFIG_AMAS_WGN

}
void stop_amas_services(void)
{
	if (nvram_get_int("re_mode") == 1) {
#ifdef RTCONFIG_BHCOST_OPT
		stop_amas_status();
		stop_amas_misc();
        stop_amas_ssd();
#endif
		stop_amas_bhctrl();
#ifndef RTCONFIG_FRONTHAUL_DWB
		stop_amas_lanctrl();
#endif
		stop_amas_wlcconnect();
	}
#ifdef RTCONFIG_FRONTHAUL_DWB
	stop_amas_lanctrl();
#endif
	stop_amas_lldpd();
}
void start_amas_services(void)
{
	if (nvram_get_int("re_mode") == 1) {
#ifdef RTCONFIG_BHCOST_OPT
        start_amas_ssd();
		start_amas_status();
		start_amas_misc();
#endif
		start_amas_wlcconnect();
		start_amas_bhctrl();
#if !defined(RTCONFIG_NOWL)
		start_amas_lanctrl();
#endif
	}
	else
	{
#if defined(RTCONFIG_FRONTHAUL_DWB) || defined(RTCONFIG_VIF_ONBOARDING)
#if !defined(RTCONFIG_NOWL)
		if (is_router_mode() || access_point_mode())
			start_amas_lanctrl();
#endif
#endif
	}
	start_amas_lldpd();
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

#ifdef RTCONFIG_SOFTWIRE46
void
start_auto46det(void)
{
	char *auto46det_argv[] = {"auto46det", "-r", NULL};
	pid_t pid;

	if(getpid() != 1) {
		notify_rc("start_auto46det");
		return;
	}

	// default, autodet will only be called by wanduck
	if(!nvram_get_int("x_Setting"))
		return;

	if(!pids("auto46det")) {
		_eval(auto46det_argv, NULL, 0, &pid);
	}
}

void
stop_auto46det(void)
{
	if(getpid() != 1) {
		notify_rc("stop_auto46det");
		return;
	}

	killall_tk("auto46det");
	nvram_set("wan46det_proceeding", "0");
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

	// default, autodet will only be called by wanduck
	if(!nvram_get_int("x_Setting"))
		return;

	if(!pids("autodet"))
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
	nvram_set("autodet_proceeding", "0");
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

	if (restart_upnp) start_upnp();

	return 0;
}

int start_nat_rules(void)
{
	char *fn = NAT_RULES, ln[PATH_MAX];
	struct stat s;
	int ret, retry, nat_state;

	// all rules applied directly according to currently status, wanduck help to triger those not cover by normal flow
#if defined(RTAC58U) || defined(RTAC59U) || defined(RTAX58U) || defined(RTAX56U)
	if (!strncmp(nvram_safe_get("territory_code"), "CX/01", 5)
	 || !strncmp(nvram_safe_get("territory_code"), "CX/05", 5))
		;
	else
#endif
 	if (nvram_match("x_Setting", "0"))
		return stop_nat_rules();

	nat_state = nvram_get_int("nat_state");
	if (nat_state == NAT_STATE_NORMAL) {
#if defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK)
		setup_ct_timeout(TRUE);
		setup_udp_timeout(TRUE);
#endif
		return nat_state;
}

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
	rule_apply_checking("services", __LINE__, NAT_RULES, ret);

#ifdef RTCONFIG_MULTIWAN_IF
	int i;
	int wan_unit;
	for (i = MULTI_WAN_START_IDX; i < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; i++) {
		wan_unit = mtwan_get_mapped_unit(i);
#ifdef RTCONFIG_MULTIWAN_PROFILE
		if (is_mtwan_primary(wan_unit))
			continue;
#endif
		snprintf(ln, sizeof(ln), "/tmp/nat_rules.%d", wan_unit);
		if (f_exists(ln))
			eval("iptables-restore", "--noflush", ln);
	}
#ifdef RTCONFIG_MULTISERVICE_WAN
	for (i = WAN_UNIT_FIRST_MULTISRV_START; i < WAN_UNIT_MULTISRV_MAX; i++) {
		snprintf(ln, sizeof(ln), "/tmp/nat_rules.%d", i);
		if (f_exists(ln))
			eval("iptables-restore", "--noflush", ln);
	}
	for (i = WAN_UNIT_MTWAN0_MS_START; i < WAN_UNIT_MTWAN_MS_MAX; i++) {
		snprintf(ln, sizeof(ln), "/tmp/nat_rules.%d", i);
		if (f_exists(ln))
			eval("iptables-restore", "--noflush", ln);
	}
#endif
#endif

#ifdef RTCONFIG_OPENVPN
	ovpn_run_fw_nat_scripts();
#endif
#ifdef RTCONFIG_WIREGUARD
	run_wgs_fw_nat_scripts();
	run_wgc_fw_nat_scripts();
#endif

	if (ret != 0)
		return nvram_get_int("nat_state");

	nvram_set_int("nat_state", NAT_STATE_NORMAL);

	setup_ct_timeout(TRUE);
	setup_udp_timeout(TRUE);

	run_custom_script("nat-start", 0, NULL, NULL);

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

#if defined(RTCONFIG_SOC_IPQ8074)
	if (is_router_mode()) {
		redirect_setting();
	}
#endif
	ret = eval("iptables-restore", REDIRECT_RULES);

	_dprintf("%s: apply the redirect_rules state %d ret %d\n", __FUNCTION__, nat_state, ret);
	rule_apply_checking("services", __LINE__, REDIRECT_RULES, ret);

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

#ifdef BCM_BSD
int start_bsd(void)
{
	int ret = 0;

	stop_bsd();

#ifdef RTCONFIG_FAST_ACL_SET
	if (nvram_match("watchdog_wait_a_moment", "1"))
		return ret;	
#endif

#ifndef RTCONFIG_AMASDB
	if (!nvram_get_int("smart_connect_x"))
		ret = -1;
	else {
#endif
		ret = eval("/usr/sbin/bsd");
#ifndef RTCONFIG_AMASDB
	}
#endif

	return ret;
}

void stop_bsd(void)
{
	killall_tk("bsd");
}
#endif /* BCM_BSD */

#if defined(LANTIQ_BSD)
int start_bsd(void)
{
	int ret = 0;
	pid_t bsd_pid = 0;
	char bsd_argv[64] = {"/usr/lib/fapi_wlan_beerock_cli"};
	stop_bsd();

	if (!nvram_get_int("smart_connect_x"))
		ret = -1;
	else{
		system("taskset -c 2 /usr/lib/fapi_wlan_beerock_cli BAND_STEERING -50 -70 3 10 10&");
	}

	return ret;
}

void stop_bsd(void)
{
	system("kill -9 `pidof fapi_wlan_beerock_cli`");
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

#if defined(RTCONFIG_DHDAP) || defined(RTCONFIG_HND_ROUTER_AX)
int start_dhd_monitor(void)
{
	int ret = 0;

#if defined(RTCONFIG_HND_ROUTER_AX)
	char *crash_log_backup_dir;
	crash_log_backup_dir = nvram_get("crash_log_backup_dir");
	if (!crash_log_backup_dir)
		cprintf("Start debug_monitor WARNING: backup directory not assigned\n");
#endif

#if defined(RTCONFIG_BCM7)
	return ret;
#endif

#if defined(RTCONFIG_HND_ROUTER_AX)
	killall_tk("debug_monitor");
	usleep(300000);
#else
	killall_tk("dhd_monitor");
#endif
	nvram_set("fast_restart", "1");
#if defined(RTCONFIG_HND_ROUTER_AX)
	ret = eval("/usr/sbin/debug_monitor", crash_log_backup_dir);
#else
	ret = eval("/usr/sbin/dhd_monitor");
#endif

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
	char tmp2[100], prefix2[32];
	int ret = 0;

	stop_lteled();

	if(nvram_get_int("lteled_down")){
#if defined(RT4GAC55U) || defined(RT4GAC56) || defined(RT4GAC68U)
		led_control(LED_LTE, LED_ON);
#elif defined(RT4GAC53U)
		led_control(LED_LTE_OFF, LED_OFF);
#endif
		led_control(LED_SIG1, LED_ON);
		led_control(LED_SIG2, LED_ON);
		led_control(LED_SIG3, LED_ON);
#if defined(RT4GAC53U) || defined(RT4GAC56)
		led_control(LED_SIG4, LED_ON);
#endif
#ifdef RT4GAC68U
		led_control(LED_3G, LED_ON);
#endif
		return 0;
	}

	for(modem_unit = MODEM_UNIT_FIRST; modem_unit < MODEM_UNIT_MAX; ++modem_unit){
		usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));

		if(is_builtin_modem(nvram_safe_get(strcat_r(prefix2, "act_type", tmp2))))
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

		if(is_builtin_modem(nvram_safe_get(strcat_r(prefix2, "act_type", tmp2)))){
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

#ifdef RTCONFIG_DSL_REMOTE
#ifdef RTCONFIG_RALINK
#else
	int isTcFwExist = 0;
	isTcFwExist = separate_tc_fw_from_trx(argv[1]);
#endif
#endif

#ifdef CONFIG_BCMWL5
	fw_check_pre();
#endif

	if(!check_imagefile(argv[1])) {
		_dprintf("FW OK\n");
		nvram_set("firmware_check", "1");
	}
	else {
		_dprintf("FW Fail\n");
		nvram_set("firmware_check", "0");
	}

#ifdef RTCONFIG_DSL_REMOTE
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
#endif

#ifdef RTCONFIG_DSL
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
	nvram_set("dsltmp_autodet_wan_type", "");
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

#if defined(RTCONFIG_HTTPS) || defined(RTCONFIG_ISP_CUSTOMIZE)
int check_rsa_signature(char *fname, char *fsign, char *fpkey)
{
	RSA *rsa_pkey = NULL;
	EVP_PKEY *pkey = NULL;
	EVP_MD_CTX *ctx = NULL;
	unsigned char buffer[16*1024];
	size_t len;
	unsigned char *sig = NULL;
	unsigned int siglen;
	struct stat stat_buf;

	FILE *publicKeyFP = NULL;
	FILE *dataFileFP = NULL;
	FILE *sigFileFP = NULL;

	/* check public key */
	if ((publicKeyFP = fopen( fpkey, "r")) == NULL) {
		_dprintf("Open publicKeyFP failure\n");
		return 0;
	}

	if (!PEM_read_RSA_PUBKEY(publicKeyFP, &rsa_pkey, NULL, NULL)) {
		_dprintf("Error loading RSA public Key File.\n");
		goto exit;
	}
	if (publicKeyFP) {
		fclose(publicKeyFP);
		publicKeyFP = NULL;
	}

	pkey = EVP_PKEY_new();
	if (!EVP_PKEY_assign_RSA(pkey, rsa_pkey)) {
		_dprintf("EVP_PKEY_assign_RSA: failed.\n");
		return 0;
	}

	/* check signature file */
	if ((sigFileFP = fopen( fsign, "r")) == NULL) {
		_dprintf("Open sigFileFP failure\n");
		return 0;
	}

	/* Read the signature */
	if (fstat(fileno(sigFileFP), &stat_buf) == -1) {
		_dprintf("Unable to read signature \n");
		goto exit;
	}

	siglen = stat_buf.st_size;
	sig = (unsigned char *)malloc(siglen);
	if (sig == NULL) {
		_dprintf("Unable to allocated %d bytes for signature\n", siglen);
		goto exit;
	}

	if ((fread(sig, 1, siglen, sigFileFP)) != siglen) {
		_dprintf("Unable to read %d bytes for signature\n", siglen);
		goto exit;
	}
	if (sigFileFP) {
		fclose(sigFileFP);
		sigFileFP = NULL;
	}

	if ((ctx = EVP_MD_CTX_create()) == NULL) {
		_dprintf("EVP_MD_CTX_create: failed.\n");
		goto exit;
	}

	if(nvram_get_int("live_update_rsa_ver") > 2){
		if (!EVP_VerifyInit(ctx, EVP_sha3_256())) {
			dbg("EVP_sha3_256 : failed.\n");
			goto exit;
		}
	}else{
		if (!EVP_VerifyInit(ctx, EVP_sha1())) {
			dbg("EVP_sha1: failed.\n");
			goto exit;
		}
	}

	/* check file */
	if ((dataFileFP = fopen(fname, "r")) == NULL) {
		_dprintf("Open dataFileFP failure\n");
		goto exit;
	}

	while ((len = fread(buffer, 1, sizeof(buffer), dataFileFP)) > 0) {
		if (!EVP_VerifyUpdate(ctx, buffer, len)) {
			_dprintf("EVP_SignUpdate: failed.\n");
			goto exit;
		}
	}

	if (ferror(dataFileFP)) {
		_dprintf("input file");
		goto exit;
	}
	if (dataFileFP) {
		fclose(dataFileFP);
		dataFileFP = NULL;
	}

	if (!EVP_VerifyFinal(ctx, sig, siglen, pkey)) {
		_dprintf("EVP_VerifyFinal: failed.\n");
		goto exit;
	}
	else {
		_dprintf("EVP_VerifyFinal: ok.\n");
	}

	if (ctx) EVP_MD_CTX_destroy(ctx);
	if (pkey) EVP_PKEY_free(pkey);
	if (sig) free(sig);
	return 1;

exit:
	/* safe to leave */
	if (ctx) EVP_MD_CTX_destroy(ctx);
	if (pkey) EVP_PKEY_free(pkey);
	if (dataFileFP) fclose(dataFileFP);
	if (sig) free(sig);
	if (sigFileFP) fclose(sigFileFP);
	if (publicKeyFP) fclose(publicKeyFP);
	return 0;
}
#endif

#ifdef RTCONFIG_HTTPS
int check_rsasign(char *fname)
{
	return check_rsa_signature(fname, "/tmp/rsasign.bin", "/usr/sbin/public.pem");
}
#endif

#ifdef RTCONFIG_ISP_CUSTOMIZE
int check_package_sign(char *fname, char *fsign)
{
	return check_rsa_signature(fname, fsign, "/usr/sbin/public.pem");
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

	/* pc_block - 20220615
		1. time-scheduling - BLOCK ALL DEVICES
		2. time-scheduling - BLOCK
		3. time-scheduling - TIME
	*/
	if (nvram_get_int("MULTIFILTER_BLOCK_ALL") == 1
		|| (nvram_get_int("MULTIFILTER_ALL") != 0 && count_pc_rules(pc_list, 2))
		|| (nvram_get_int("MULTIFILTER_ALL") != 0 && count_pc_rules(pc_list, 1)))
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

#ifdef RTCONFIG_NBR_RPT
void start_nbr_monitor(void){
	char *cmd[] = {"nbr_monitor", NULL};
	pid_t pid;
	if(pids("nbr_monitor"))
		killall_tk("nbr_monitor");
	_eval(cmd, NULL, 0, &pid);
	return;
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

	if (mediabridge_mode())
		return;

#ifdef RTCONFIG_FAST_ACL_SET
	if (nvram_match("watchdog_wait_a_moment", "1"))
		return;	
#endif

#if defined(RTCONFIG_WIFI_SON)
	if (nvram_match("wifison_ready", "1"))
		return;
#endif
#if defined(RTCONFIG_STA_AP_BAND_BIND) || defined(RTCONFIG_FORCE_ROAMING)
	_eval(cmd, NULL, 0, &pid);
	return ;
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

	if (!nvram_get_int("wlready") || nvram_get_int("guard_mode") != 1)	// guard_mode 0:default 1:eco mode
		return;
	else {
		foreach(ifname, nvram_safe_get("wl_ifnames"), next) {
#if defined(RTCONFIG_RALINK)
		eval("iwpriv", ifname,"set", "httxstream=1");
		eval("iwpriv", ifname,"set", "htrxstream=1");
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
/*** Disable ***/
	if (nvram_get_int("led_disable") == 1) {
		setAllLedOff();
		nvram_set("AllLED", "0");
#ifdef RTCONFIG_USB
		stop_usbled();
#endif
	} else {
/*** Enable ***/
		nvram_set("AllLED", "1");
#if defined(RTAX86U_PRO) || defined(RTAX88U_PRO)
		setAllLedNormal();
		led_control(LED_POWER, LED_ON);	// Must be run after
#else
		led_control(LED_POWER, LED_ON);
#ifdef RTCONFIG_USB
		start_usbled();
#endif
#ifdef RTCONFIG_LED_ALL
		led_control(LED_ALL, LED_ON);
#endif


/* WAN/LAN */
#if defined(RTAC3200) || defined(RTCONFIG_BCM_7114)
		eval("et", "-i", "eth0", "robowr", "0", "0x18", "0x01ff");
		eval("et", "-i", "eth0", "robowr", "0", "0x1a", "0x01ff");
#elif defined(HND_ROUTER)
#ifndef GTAC2900
#if defined(RTAX58U_V2) || defined(GTAX6000) || defined(RTAX3000N) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(RTAX5400) || defined(RTAX88U_PRO)
		wan_phy_led_pinmux(0);
#else
		led_control(LED_WAN_NORMAL, LED_ON);
#endif
#endif
#if defined(RTAX88U_PRO)
		lan_phy_led_pinmux(0);
#endif
		setLANLedOn();
#else
		eval("et", "robowr", "0", "0x18", "0x01ff");
		eval("et", "robowr", "0", "0x1a", "0x01ff");
#endif

#ifdef RTCONFIG_EXTPHY_BCM84880
#if defined(RTAX86U) || defined(GTAX11000) || defined(GTAX6000)
		int ext_phy_model = nvram_get_int("ext_phy_model");

		if(!strcmp(get_productid(), "RT-AX86S")) ;
		else if(ext_phy_model == EXT_PHY_GPY211)
				eval("ethctl", "phy", "ext", EXTPHY_GPY_ADDR_STR, "0x1e0001", "0x3f0");
		else if(ext_phy_model == EXT_PHY_RTL8226)
			eval("ethctl", "phy", "ext", EXTPHY_RTL_ADDR_STR, "0x1fd032", "0x0027");        // RTL LCR2 LED Control Reg
		else if(ext_phy_model == EXT_PHY_BCM54991){
			eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a832", "0x6");        // default. CTL LED3 MASK LOW
			eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a835", "0x40");       // default. CTL LED4 MASK LOW
		}
#endif
#endif

#ifdef RTCONFIG_LAN4WAN_LED
		LanWanLedCtrl();
#endif
#ifdef RTAC87U
		qcsapi_wifi_run_script("router_command.sh", "lan4_led_ctrl on");
#endif

		kill_pidfile_s("/var/run/wanduck.pid", SIGUSR2);


/* Wifi */
		if (nvram_match("wl0_radio", "1")) {
#ifdef RTAC68U
			eval("wl", "ledbh", "10", "7");
#elif defined(RTAC3200)
			eval("wl", "-i", "eth2", "ledbh", "10", "7");
#elif defined(RTCONFIG_BCM_7114) || defined(RTAC86U)
			eval("wl", "ledbh", "9", "7");
#elif defined(RTAX88U) || defined(GTAX11000)
			eval("wl", "-i", "eth6", "ledbh", "15", "7");
#elif defined(RTAX58U) || defined(RTAX56U)
			eval("wl", "-i", "eth5", "ledbh", "0", "25");
#elif defined(RTAX86U)
			if(!strcmp(get_productid(), "RT-AX86S"))
				eval("wl", "-i", "eth5", "ledbh", "7", "7");
			else
				eval("wl", "-i", "eth6", "ledbh", "7", "7");
#elif defined(RTAX68U)
			eval("wl", "-i", "eth5", "ledbh", "7", "7");
#elif defined(RTAC68U_V4)
			eval("wl", "-i", "eth5", "ledbh", "10", "7");
#elif defined(GTAC2900)
			eval("wl", "ledbh", "9", "1");
#elif defined(GTAXE11000)
			eval("wl", "-i", "eth6", "ledbh", "9", "7");
#elif defined(GTAX6000)
			eval("wl", "-i", "eth6", "ledbh", "13", "7");
#elif defined(GTAXE16000)
			eval("wl", "-i", "eth7", "ledbh", "13", "7");
#elif defined(GTAX11000_PRO)
			eval("wl", "-i", "eth6", "ledbh", "13", "7");
#endif
		}

		if (nvram_match("wl1_radio", "1")) {
#ifdef RTAC68U
			eval("wl", "-i", "eth2", "ledbh", "10", "7");
#elif defined(RTAC3200)
			eval("wl", "ledbh", "10", "7");
#elif defined(RTAC86U)
			eval("wl", "-i", "eth6", "ledbh", "9", "7");
#elif defined(GTAC2900)
			eval("wl", "-i", "eth6", "ledbh", "9", "1");
#elif defined(RTCONFIG_BCM_7114)
			eval("wl", "-i", "eth2", "ledbh", "9", "7");
#elif defined(RTAC87U)
			qcsapi_wifi_run_script("router_command.sh", "wifi_led_on");
			qcsapi_led_set(1, 1);
#elif defined(RTAX88U) || defined(RTAX86U) || defined(GTAX11000)
			if(!strcmp(get_productid(), "RT-AX86S"))
				eval("wl", "-i", "eth6", "ledbh", "15", "7");
			else
				eval("wl", "-i", "eth7", "ledbh", "15", "7");
#elif defined(RTAX68U)
			eval("wl", "-i", "eth6", "ledbh", "7", "7");
#elif defined(RTAC68U_V4)
			eval("wl", "-i", "eth6", "ledbh", "10", "7");
#elif defined(RTAX58U)
			eval("wl", "-i", "eth6", "ledbh", "15", "7");
#elif defined(RTAX56U)
			eval("wl", "-i", "eth6", "ledbh", "0", "25");
#elif defined(GTAXE11000)
			eval("wl", "-i", "eth7", "ledbh", "9", "7");
#elif defined(GTAX6000)
			eval("wl", "-i", "eth7", "ledbh", "13", "7");
#elif defined(GTAXE16000)
			eval("wl", "-i", "eth8", "ledbh", "13", "7");
#elif defined(GTAX11000_PRO)
			eval("wl", "-i", "eth7", "ledbh", "13", "7");
#endif
		}

#if defined(RTAC3200) || defined(RTCONFIG_HAS_5G_2)
		if (nvram_match("wl2_radio", "1")) {
#if defined(RTAC3200)
			eval("wl", "-i", "eth3", "ledbh", "10", "7");
#elif defined(RTAC5300)
			eval("wl", "-i", "eth3", "ledbh", "9", "7");
#elif defined(GTAX11000)
			eval("wl", "-i", "eth8", "ledbh", "15", "7");
#elif defined(GTAXE11000)
			eval("wl", "-i", "eth8", "ledbh", "9", "7");
#elif defined(GTAXE16000)
			eval("wl", "-i", "eth9", "ledbh", "13", "7");
#elif defined(GTAX11000_PRO)
			eval("wl", "-i", "eth8", "ledbh", "13", "7");
#endif
		}
#endif

#ifdef RTCONFIG_QUADBAND
		if (nvram_match("wl3_radio", "1")) {
#if defined(GTAXE16000)
			eval("wl", "-i", "eth10", "ledbh", "13", "7");
#endif
			stop_upnp();
			start_upnp();
		}
#endif

#ifdef RTCONFIG_LOGO_LED
		led_control(LED_LOGO, LED_ON);
#endif

#if defined(RTAX82U) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTAX6000)
		kill_pidfile_s("/var/run/ledg.pid", SIGTSTP);
#endif
#if defined(XT12) || defined(ET12)
		bcm_cled_ctrl_single_white(BCM_CLED_WHITE, BCM_CLED_STEADY_NOBLINK);
#endif

#ifdef GTAX6000
		AntennaGroupReset(LED_ON);
		setAntennaGroupOn();
		eval("sw", "0xff803140", "0x0000e002", "0x00c34a24", "0x00000a24", "0x00800080");
		eval("sw", "0xff80301c", "0x00040000");
		eval("sw", "0xff803150", "0x0000e002", "0x00c34a24", "0x00000a24", "0x00800080");
		eval("sw", "0xff80301c", "0x00080000");
		eval("sw", "0xff803160", "0x0000e002", "0x00c34a24", "0x00000a24", "0x00800080");
		eval("sw", "0xff80301c", "0x00100000");
		eval("sw", "0xff803170", "0x0000e002", "0x00c34a24", "0x00000a24", "0x00800080");
		eval("sw", "0xff80301c", "0x00200000");

		start_ledg();
		start_antled();
#endif
#if defined(GTAXE16000) || defined(GTAX11000_PRO)
		notify_rc("restart_ledg");
#endif
#endif // RT-AX86U_PRO
	}
#if defined(RTCONFIG_RGBLED)
	start_aurargb();
#endif
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
		nvram_set_int("ddns_last_wan_unit", wan_primary_ifunit());
		nvram_set("ddns_check_retry", "10"); //Reset DDNS Retry
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
	int nvram_changed = 0;

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
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("captive_portal_2g"));
		}

		//auth_mode_x
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_auth_mode_x", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "open")) {
			nvram_set(wl_item, "open");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "open");
		}

		//crypto
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_crypto", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "aes")) {
			nvram_set(wl_item, "aes");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "aes");
		}

		//wpa_psk
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_wpa_psk", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//macmode
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_macmode", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "disabled")) {
			nvram_set(wl_item, "disabled");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "disabled");
		}

		//lanaccess
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_lanaccess", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "off")) {
			nvram_set(wl_item, "off");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "off");
		}

		//expire
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_expire", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bss_enable
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "1")) {
			nvram_set(wl_item, "1");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "1");
		}

		//guest_num
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_guest_num", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "10")) {
			nvram_set(wl_item, "10");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "10");
		}

		//bw_enabled
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bw_dl
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_dl", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//bw_ul
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_ul", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//captive_portal_2g_if_temp
		if(strcmp(nvram_safe_get("captive_portal_2g_if_temp"),wl_unit)) {
			nvram_set("captive_portal_2g_if_temp", wl_unit);
			nvram_changed = 1;
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
			nvram_changed = 1;
			_dprintf("set captive_portal_2g_if_temp : %s\n", "off");

			//bss_enable
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
			nvram_set(wl_item, "0");
			nvram_changed = 1;
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
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("captive_portal_5g"));
		}

		//auth_mode_x
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_auth_mode_x", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "open")) {
			nvram_set(wl_item, "open");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "open");
		}

		//crypto
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_crypto", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "aes")) {
			nvram_set(wl_item, "aes");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "aes");
		}

		//wpa_psk
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_wpa_psk", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//macmode
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_macmode", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "disabled")) {
			nvram_set(wl_item, "disabled");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "disabled");
		}

		//lanaccess
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_lanaccess", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "off")) {
			nvram_set(wl_item, "off");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "off");
		}

		//expire
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_expire", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bss_enable
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "1")) {
			nvram_set(wl_item, "1");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "1");
		}

		//guest_num
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_guest_num", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "10")) {
			nvram_set(wl_item, "10");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "10");
		}

		//bw_enabled
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bw_dl
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_dl", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//bw_ul
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_ul", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//captive_portal_2g_if_temp
		if(strcmp(nvram_safe_get("captive_portal_5g_if_temp"),wl_unit)) {
			nvram_set("captive_portal_5g_if_temp", wl_unit);
			nvram_changed = 1;
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
			nvram_changed = 1;
			_dprintf("set captive_portal_5g_if_temp : %s\n", "off");

			//bss_enable
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
			nvram_set(wl_item, "0");
			nvram_changed = 1;
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
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, nvram_safe_get("captive_portal_5g_2"));
		}

		//auth_mode_x
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_auth_mode_x", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "open")) {
			nvram_set(wl_item, "open");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "open");
		}

		//crypto
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_crypto", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "aes")) {
			nvram_set(wl_item, "aes");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "aes");
		}

		//wpa_psk
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_wpa_psk", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//macmode
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_macmode", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "disabled")) {
			nvram_set(wl_item, "disabled");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "disabled");
		}

		//lanaccess
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_lanaccess", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "off")) {
			nvram_set(wl_item, "off");
			nvram_changed = 1;
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
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "1");
		}

		//guest_num
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_guest_num", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "10")) {
			nvram_set(wl_item, "10");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "10");
		}

		//bw_enabled
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_enabled", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "0")) {
			nvram_set(wl_item, "0");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "0");
		}

		//bw_dl
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_dl", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//bw_ul
		memset(wl_item, 0, sizeof(wl_item));
		(void)strcat_r(wl_unit, "_bw_ul", wl_item);
		if(strcmp(nvram_safe_get(wl_item), "")) {
			nvram_set(wl_item, "");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "");
		}

		//captive_portal_2g_if_temp
		if(strcmp(nvram_safe_get("captive_portal_5g_2_if_temp"),wl_unit)) {
			nvram_set("captive_portal_5g_2_if_temp", wl_unit);
			nvram_changed = 1;
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
			nvram_changed = 1;
			_dprintf("set captive_portal_5g_2_if_temp : %s\n", "off");

			//bss_enable
			memset(wl_item, 0, sizeof(wl_item));
			(void)strcat_r(wl_unit, "_bss_enabled", wl_item);
			nvram_set(wl_item, "0");
			nvram_changed = 1;
			_dprintf("set %s : %s\n", wl_item, "0");
		}
	}

	if (nvram_changed == 1)
		nvram_commit();
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
	nvram_commit();
}
void
overwrite_captive_portal_ssid(void) {
	char wl_item[30];
	char wl_if[5];
	int nvram_changed = 0;
	memset(wl_item, 0, sizeof(wl_item));
	memset(wl_if, 0, sizeof(wl_if));

	sprintf(wl_if, "wl%s.%s", nvram_safe_get("wl_unit"), nvram_safe_get("wl_subunit"));

	//ssid
	(void)strcat_r(wl_if, "_ssid", wl_item);
	switch(nvram_get_int("wl_unit")) {
		case 0 :
			nvram_set("captive_portal_2g", nvram_safe_get(wl_item));
			nvram_changed = 1;
			_dprintf("set captive_portal_2g : %s\n", nvram_safe_get(wl_item));
			break;
		case 1 :
			nvram_set("captive_portal_5g", nvram_safe_get(wl_item));
			nvram_changed = 1;
			_dprintf("set captive_portal_5g : %s\n", nvram_safe_get(wl_item));
			break;
		case 2 :
			nvram_set("captive_portal_5g_2", nvram_safe_get(wl_item));
			nvram_changed = 1;
			_dprintf("set captive_portal_5g_2 : %s\n", nvram_safe_get(wl_item));
			break;
	}

	if (nvram_changed == 1)
		nvram_commit();
}
void
overwrite_captive_portal_adv_ssid(void) {
	char wl_item[30];
	char wl_if[5];
	int nvram_changed = 0;
	memset(wl_item, 0, sizeof(wl_item));
	memset(wl_if, 0, sizeof(wl_if));

	sprintf(wl_if, "wl%s.%s", nvram_safe_get("wl_unit"), nvram_safe_get("wl_subunit"));

	//ssid
	(void)strcat_r(wl_if, "_ssid", wl_item);
	switch(nvram_get_int("wl_unit")) {
		case 0 :
			nvram_set("captive_portal_adv_2g_ssid", nvram_safe_get(wl_item));
			nvram_changed = 1;
			_dprintf("set captive_portal_adv_2g_ssid : %s\n", nvram_safe_get(wl_item));
			break;
		case 1 :
			nvram_set("captive_portal_adv_5g_ssid", nvram_safe_get(wl_item));
			 nvram_changed = 1;
			_dprintf("set captive_portal_adv_5g_ssid : %s\n", nvram_safe_get(wl_item));
			break;
		case 2 :
			nvram_set("captive_portal_adv_5g_2_ssid", nvram_safe_get(wl_item));
			 nvram_changed = 1;
			_dprintf("set captive_portal_adv_5g_2_ssid : %s\n", nvram_safe_get(wl_item));
			break;
	}

	if (nvram_changed == 1)
		nvram_commit();
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
	char *https_argv[] = { "httpds_uam", "-s", NULL};
	pid_t pid;

	chdir("/www/fbwifi");


	_eval(httpd_argv, NULL, 0, &pid);
	_eval(https_argv, NULL, 0, &pid);
	logmessage(LOGNAME, "start httpd_uam");

	chdir("/");
	nvram_set("fbwifi_running", "1");
	return;
}

void stop_httpd_uam()
{
	if (pids("httpd_uam"))
		killall_tk("httpd_uam");

	if (pids("httpds_uam"))
		killall_tk("httpds_uam");

	nvram_set("fbwifi_running", "0");
}

void stop_fbwifi_config()
{
	if (pids("fb_wifi_config"))
		killall_tk("fb_wifi_config");
}

void stop_fbwifi()
{
	stop_fbwifi_check();
	stop_httpd_uam();
	unlink("/jffs/fbwifi/gateway_configuration.txt");
	start_firewall(wan_primary_ifunit(), 0);
}

void start_fbwifi()
{
	int MAX_WAIT = 10;

	if(nvram_invmatch("fbwifi_id", "off") && nvram_invmatch("fbwifi_id", "") && nvram_invmatch("fbwifi_secret", ""))
	{
		mkdir_if_none("/jffs/fbwifi");
		start_fbwifi_config();//get fbwifi configuration
		while(!f_exists("/jffs/fbwifi/gateway_configuration.txt") && MAX_WAIT){
			sleep(1);
			MAX_WAIT--;
		}

		if(nvram_match("fbwifi_enable","on")){
			char *fbwifi_2g = nvram_safe_get("fbwifi_2g");
			char *fbwifi_5g = nvram_safe_get("fbwifi_5g");
			char *fbwifi_5g_2 = nvram_safe_get("fbwifi_5g_2");
			char *fbwifi_sdn_ifname = nvram_safe_get("fbwifi_sdn_ifname");

			if( f_exists("/jffs/fbwifi/gateway_configuration.txt") &&
				(strcmp(fbwifi_2g, "off") || strcmp(fbwifi_5g, "off") || strcmp(fbwifi_5g_2, "off") || strcmp(fbwifi_sdn_ifname, "")) ){
				start_httpd_uam();
				start_fbwifi_check();
				sleep(2);
				start_firewall(wan_primary_ifunit(), 0);
			}
		}
	}
	else{
		_dprintf("Fail to start fbwifi. Please check required nvram values.\n");
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

#ifdef RTCONFIG_HAS_5G_2
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
	_dprintf("[%s:(%d)]: set_fbwifi_profile: end\n", __FUNCTION__, __LINE__);
}
void
overwrite_fbwifi_ssid(void) {
	char wl_item[30];
	char wl_if[5];
	memset(wl_item, 0, sizeof(wl_item));
	memset(wl_if, 0, sizeof(wl_if));

	_dprintf("[%s:(%d)]: overwrite_fbwifi_ssid: start\n", __FUNCTION__, __LINE__);
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
#ifdef RTCONFIG_HAS_5G_2
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
#ifdef RTCONFIG_HAS_5G_2
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

	_dprintf("[%s:(%d)]: overwrite_fbwifi_ssid: end\n", __FUNCTION__, __LINE__);
}
#endif

#ifdef RTCONFIG_CFGSYNC
int start_cfgsync(void)
{

	char *cfg_server_argv[] = {"cfg_server", NULL};
	char *cfg_client_argv[] = {"cfg_client", NULL};
	pid_t pid;
	int ret = 0;

#ifdef RTCONFIG_MASTER_DET
	if (nvram_match("cfg_master", "1") && (is_router_mode() || access_point_mode()))
#else
	if (is_router_mode())
#endif	/* RTCONFIG_MASTER_DET */
	{

#ifdef RTCONFIG_AMAS
#if defined(RTCONFIG_WIFI_SON)
		if(!nvram_match("wifison_ready", "1")) {
			/* stop bluetoothd after first time amesh setting(for CAP only, RE will reboot) */
			if (nvram_match("x_Setting", "1") && pids("bluetoothd")) killall_tk("bluetoothd");
#endif
		if (!(getAmasSupportMode() & AMAS_CAP)) {
			_dprintf("not support CAP, don't start cfg_server\n");
			return 0;
		}
#if defined(RTCONFIG_WIFI_SON)
		}
#endif
#endif

		stop_cfgsync();
		nvram_set("cfg_pause", "0");
		ret = _eval(cfg_server_argv, NULL, 0, &pid);
	}
#ifdef RTCONFIG_MASTER_DET
	else if (nvram_match("x_Setting", "1") &&
		strlen(nvram_safe_get("cfg_group")) == 0 &&
		(is_router_mode() || (access_point_mode()
#if defined(RTCONFIG_WIFI_SON)
		&& !nvram_match("wifison_ready","1")
#endif
		)))
	{
#ifdef RTCONFIG_AMAS
#if defined(RTCONFIG_WIFI_SON)
		if(!nvram_match("wifison_ready", "1"))
#endif
		if (!(getAmasSupportMode() & AMAS_CAP)) {
			_dprintf("not support CAP, don't start cfg_server\n");
			return 0;
		}
#endif

		stop_cfgsync();
		nvram_set("cfg_master", "1");
		nvram_commit();
		ret = _eval(cfg_server_argv, NULL, 0, &pid);
	}
#endif

#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_WIFI_SON)
	else if ((nvram_get_int("re_mode") == 1
#if defined(RTCONFIG_WIFI_SON)
		 || (access_point_mode() && nvram_match("wifison_ready", "1"))
#endif
		 )&& nvram_get_int("lan_state_t") == LAN_STATE_CONNECTED)
	{
#if defined(RTCONFIG_AMAS)
#if defined(RTCONFIG_WIFI_SON)
		if(!nvram_match("wifison_ready", "1"))
#endif
		if (!(getAmasSupportMode() & AMAS_RE)) {
			_dprintf("not support RE, don't start cfg_client\n");
			return 0;
		}
#endif
		stop_cfgsync();
		nvram_set("cfg_pause", "0");
		ret = _eval(cfg_client_argv, NULL, 0, &pid);
	}
#endif
	return ret;
}

void stop_cfgsync(void)
{
	unlink("/var/run/cfg_server.pid");
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

#ifdef RTCONFIG_CONNDIAG
void stop_amas_portstatus(void){
	if(pids("amas_portstatus"))
		killall_tk("amas_portstatus");
}
void start_amas_portstatus(void){
	char *cmd[] = {"amas_portstatus", NULL};
	pid_t pid;

	stop_amas_portstatus();

	_eval(cmd, NULL, 0, &pid);
}
void stop_conn_diag_ss(void){
	if(pids("amas_ssd_cd"))
		killall_tk("amas_ssd_cd");
}
void start_conn_diag_ss(void){
	char *cmd[] = {"amas_ssd_cd", NULL};
	pid_t pid;

	stop_conn_diag_ss();

	_eval(cmd, NULL, 0, &pid);
}
void stop_conn_diag(void){
	stop_conn_diag_ss();
	if(pids("conn_diag"))
		killall_tk("conn_diag");
}

void start_conn_diag(void){
	char *cmd[] = {"conn_diag", NULL};
	pid_t pid;

	stop_conn_diag();

	_eval(cmd, NULL, 0, &pid);

	start_conn_diag_ss();
	//start_amas_portstatus();
}
#endif
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

#ifdef BCM_CEVENTD
int start_ceventd(void)
{
	int ret = 0;
	char *ceventd_argv[] = {"/bin/ceventd", NULL};
	pid_t pid;

	if (nvram_match("ceventd_enable", "1"))
		ret = _eval(ceventd_argv, NULL, 0, &pid);

	return ret;
}
void stop_ceventd(void)
{
	killall_tk("ceventd");
}
#endif

#ifdef RTCONFIG_BRCM_HOSTAPD
int
start_wps_pbcd()
{
	int ret = 0;
	char *wps_pbcd_argv[] = {"wps_pbcd", NULL};
	pid_t pid;

	if(pids("wps_pbcd"))
		stop_wps_pbcd();

	ret = _eval(wps_pbcd_argv, NULL, 0, &pid);
	sleep(2); // wait wps_pbcd to be completed

	return ret;
}

int
stop_wps_pbcd()
{
	killall_tk("wps_pbcd");

	return 0;
}
#endif  /* CONFIG_HOSTAPD */

#if defined(RTCONFIG_QCA_LBD)
void start_qca_lbd(void)
{
#ifdef RTCONFIG_WIRELESSREPEATER
	const int sw_mode = sw_mode();
#endif
	const int max_nr_wl_if = min(MAX_NR_WL_IF, WL_5G_2_BAND + 1);
	const char *dbglvlstr[] = { "none", "err", "info", "debug", "dump" };
	pid_t pid;
	int band, dbglvl, no_lbd = 0, nr_ssid = 0;
	int radio_on[WL_NR_BANDS] = { 0 };
	char lvlstr[sizeof("all=debugXXXXX")];
	char prefix[sizeof("wlXXXXXX_")], ssid[32 + 1] = { 0 };
	char *lbd_argv[]= { "lbd", "-C", LBD_PATH, 
#if defined(RTCONFIG_CFG80211)
		"-cfg80211",
#endif
		NULL, NULL };

	if (!nvram_get_int("smart_connect_x") || (!nvram_get_int("x_Setting")
		|| __repeater_mode(sw_mode) || __mediabridge_mode(sw_mode) || __aimesh_re_node(sw_mode)
#if defined(RTCONFIG_SOC_IPQ8074)
		|| nvram_match("wl1_precacen", "1")	/* band steering conflict with Agile DFS */
#endif
#if defined(RTCONFIG_WIFI_SON)
		|| nvram_get_int("wifison_ready")
#endif
	))
		no_lbd = 1;

	/* Both 2G/5G must be enabled. */
	if (!no_lbd) {
		for (band = WL_2G_BAND; band < max_nr_wl_if; ++band) {
			if (absent_band(band))
				continue;

			snprintf(prefix, sizeof(prefix), "wl%d_", band);
			if (nvram_pf_match(prefix, "radio", "1"))
				radio_on[band] = 1;
		}
		if (!radio_on[WL_2G_BAND] || (!radio_on[WL_5G_BAND] && !radio_on[WL_5G_2_BAND]))
			no_lbd = 1;
	}

	/* 2G SSID must equal to 5G SSID */
	if (!no_lbd) {
		for (band = WL_2G_BAND; band < max_nr_wl_if; ++band) {
			if (absent_band(band))
				continue;

			if (aimesh_re_node()) {
				snprintf(prefix, sizeof(prefix), "wl%d.1_", band);
			} else {
				snprintf(prefix, sizeof(prefix), "wl%d_", band);
#if defined(RTCONFIG_WIRELESSREPEATER)
				if (sw_mode == SW_MODE_REPEATER
#if !defined(RTCONFIG_CONCURRENTREPEATER)
				    && nvram_get_int("wlc_band") == band && nvram_invmatch("wlc_ssid", "")
#endif
				   )
					snprintf(prefix, sizeof(prefix), "wl%d.1_", band);
#endif
			}

			if (!strlen(nvram_pf_safe_get(prefix, "ssid")))
				continue;
			if (*ssid == '\0') {
				strlcpy(ssid, nvram_pf_safe_get(prefix, "ssid"), sizeof(ssid));
				nr_ssid++;
				continue;
			}
			if (strcmp(ssid, nvram_pf_safe_get(prefix, "ssid")))
				continue;
			nr_ssid++;
		}
		if (nr_ssid < 2)
			no_lbd = 1;
	}

	if (no_lbd) {
		if (pids("lbd")) {
			killall_tk("lbd");
			logmessage("QCA LBD", "daemon is stopped");
		}
		unlink(LBD_PATH);
		return;
	}
	if (pids("lbd"))
		return;

	if (!gen_lbd_config_file()) {
		setenv("DBG_OUT_FILE_PATH", nvram_get_int("qca_lbd_dbg")? "/dev/console" : "/dev/null", 1);
		dbglvl = nvram_get_int("qca_lbd_dbg");
		if (dbglvl < 0)
			dbglvl = 0;
		if (dbglvl >= ARRAY_SIZE(dbglvlstr))
			dbglvl = ARRAY_SIZE(dbglvlstr) - 1;
		snprintf(lvlstr, sizeof(lvlstr), "all=%s", dbglvlstr[dbglvl]);
		setenv("DBG_LEVELS", lvlstr, 1);
		_eval(lbd_argv, NULL, 0, &pid);
		logmessage("QCA LBD", "daemon is started");
		sleep(1);
		dis_steer();
	}
}

void stop_qca_lbd(void)
{
	killall_tk("lbd");
	unlink(LBD_PATH);
	logmessage("QCA LBD", "daemon is stopped");
}
#endif
#ifdef RTCONFIG_QCA_PLC2
void start_plc_master(void)
{
	sleep(1);
	nvram_set("cfg_plc_m_ex", nvram_get("cfg_plc_master"));
}

void stop_plc_master(void)
{
}
#endif

#if defined(RTCONFIG_MTK_BSD)
#if defined(RTCONFIG_WLMODULE_MT7915D_AP) ||defined(RTCONFIG_MT798X)
void stop_mtk_bs20(void)
{
	if(pids("bs20"))
	{
		killall_tk("bs20");
		unlink(BSD_LOG);
		unlink(BSD_PATH);
		unlink("/tmp/client_db.txt");
		logmessage("MTK BS20", "daemon is stopped");
	}
	if(pids("wapp"))
	{
		killall_tk("wapp");
		logmessage("MTK WAPP", "daemon is stopped");
	}
}
void start_mtk_bs20(void)
{
#ifdef RTCONFIG_WIRELESSREPEATER
	const int sw_mode = sw_mode();
#endif
	const int max_nr_wl_if = min(MAX_NR_WL_IF, WL_5G_2_BAND + 1);
	int band, no_bs20 = 0, nr_ssid = 0;
	int radio_on[WL_NR_BANDS] = { 0 };
	char prefix[sizeof("wlXXXXXX_")], ssid[32 + 1] = { 0 };
	char tmp[10],iface[30];

	stop_mtk_bs20();
	if (!nvram_get_int("smart_connect_x") || (!nvram_get_int("x_Setting")
		|| __repeater_mode(sw_mode) || __mediabridge_mode(sw_mode) || __aimesh_re_node(sw_mode)))
		no_bs20 = 1;

	/* Both 2G/5G must be enabled. */
	if (!no_bs20) {
		for (band = WL_2G_BAND; band < max_nr_wl_if; ++band) {
			if (absent_band(band))
				continue;

			snprintf(prefix, sizeof(prefix), "wl%d_", band);
			if (nvram_pf_match(prefix, "radio", "1"))
				radio_on[band] = 1;
		}
		if (!radio_on[WL_2G_BAND] || (!radio_on[WL_5G_BAND] && !radio_on[WL_5G_2_BAND]))
			no_bs20 = 1;
	}

	/* 2G SSID must equal to 5G SSID */
	if (!no_bs20) {
		for (band = WL_2G_BAND; band < max_nr_wl_if; ++band) {
			if (absent_band(band))
				continue;

			if (aimesh_re_node()) {
				snprintf(prefix, sizeof(prefix), "wl%d.1_", band);
			} else {
				snprintf(prefix, sizeof(prefix), "wl%d_", band);
#if defined(RTCONFIG_WIRELESSREPEATER)
				if (sw_mode == SW_MODE_REPEATER
#if !defined(RTCONFIG_CONCURRENTREPEATER)
				    && nvram_get_int("wlc_band") == band && nvram_invmatch("wlc_ssid", "")
#endif
				   )
					snprintf(prefix, sizeof(prefix), "wl%d.1_", band);
#endif
			}

			if (!strlen(nvram_pf_safe_get(prefix, "ssid")))
				continue;
			if (*ssid == '\0') {
				strlcpy(ssid, nvram_pf_safe_get(prefix, "ssid"), sizeof(ssid));
				nr_ssid++;
				continue;
			}
			if (strcmp(ssid, nvram_pf_safe_get(prefix, "ssid")))
				continue;
			nr_ssid++;
		}
		if (nr_ssid < 2)
			no_bs20 = 1;
	}

	if (no_bs20) 
		return;

	if (!gen_bsd_config_file()) 
	{
		memset(iface,0,sizeof(iface));
		for (band = WL_2G_BAND; band < max_nr_wl_if; ++band) 
		{
			sprintf(tmp, "-c%s ", get_wifname(band));
               		strcat(iface, tmp);
		}
		eval("wapp","-d1","-v2",iface);
		logmessage("MTK WAPP", "daemon is started");
		
		sleep(3);
		unlink("/tmp/client_db.txt");
		eval("touch", "/tmp/client_db.txt");

		if(nvram_get_int("bs20_dbg"))
			eval("bs20", "-f", BSD_LOG, "-B");
		else
			eval("bs20","-q","-B");
		logmessage("MTK BS20", "daemon is started");
		//sleep(1);
		//dis_steer();
	}	
}
#else
void stop_mtk_bs20(void){}	
void start_mtk_bs20(void){}
#endif //RTCONFIG_WLMODULE_MT7915D_AP
#endif
#if defined(RTCONFIG_HW_DOG)
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/watchdog.h>

extern void uptime_wait(int);
/* Usage:
 *   hwdog       - trigger CPU hardware watchdog
 *   hwdog NN    - trigger CPU hardware watchdog after delay NN seconds
 *   hwdog stop  - stop watchdog count down (prevent to reset)
 */
int hwdog_main(int argc, char **argv)
{
	int fd=-1, delay_start=0, ret;
	int flag;

	if ((argc >= 2) && (strcmp(argv[1], "stop")!=0))
		delay_start = atoi(argv[1]);

	if (delay_start)
		uptime_wait(delay_start);

	fd = open("/dev/watchdog", O_WRONLY);
	if (fd < 0) {
		printf("Cannot open /dev/watchdog!\n");
		return -1;
	}

	if ((argc >= 2) && (strcmp(argv[1], "stop")==0)) {
		flag = WDIOS_DISABLECARD;
		ret = ioctl(fd, WDIOC_SETOPTIONS, &flag);
		close(fd);
		if (ret < 0) {
			printf("[%s]:ioctl fail for disable!!\n", argv[0]);
			return -1;
		}
		printf("stop OK!\n");
		return 0;
	}

	// enable watchdog to 30 seconds count down
	flag = 30;
	ret = ioctl(fd, WDIOC_SETTIMEOUT, &flag);
	if (ret < 0) {
		printf("[%s]:ioctl fail for settimeout!!\n", argv[0]);
		close(fd);
		return -1;
	}
	flag = WDIOS_ENABLECARD;
	ret = ioctl(fd, WDIOC_SETOPTIONS, &flag);
	if (ret < 0) {
		printf("[%s]:ioctl fail for enable!!\n", argv[0]);
		close(fd);
		return -1;
	}

	flag = 0;
	ret = 0;
	while (!ret) {
		sleep(5); // refresh watchdog every 5 seconds
		ret = ioctl(fd, WDIOC_KEEPALIVE, &flag);
	}
	close(fd);
	if (ret)
		printf("[%s]:ioctl fail for keepalive!!\n", argv[0]);
	return 0;
}
#endif // end of HW_DOG

#if defined(RTCONFIG_PTHSAFE_POPEN)
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <string.h>
#include <sys/un.h>
#include <signal.h>
#if defined(__GLIBC__) || defined(__UCLIBC__) /* not musl */
#include <wait.h>
#else
#include <sys/wait.h>
#endif

#ifdef popen
#undef	popen
#endif
#ifdef pclose
#undef pclose
#endif

static void ps_clearchild(int no)
{
	int status;
	while (waitpid(0,&status,WNOHANG)>0); // clear all zombie child
}

#define	MAX_OUTBUF_SIZE	512*1024
#define STEP_BUF_SIZE	1024
#define SELECT_TIMEOUT	2
#define BACK_LOG_SIZE	5

static void server_handler(int sock)
{
	char cmd[200];
	int ret, len;
	fd_set rmask;
	struct timeval select_timeout;

	FD_ZERO(&rmask);
	FD_SET(sock, &rmask);
	select_timeout.tv_sec = SELECT_TIMEOUT;
	select_timeout.tv_usec = 0;

	ret = select(sock+1, &rmask, NULL, NULL, &select_timeout);
	if (ret > 0) {
		FILE *fp;
		char *buf;
		int read_len = 0, tmp_len, buf_len;
		len = read(sock, cmd, sizeof(cmd)-1);
		if (len > 0) {
			cmd[len] = '\0';
			// _dprintf("CMD[%s]\n", cmd);
			fp = popen(cmd, "r");
			if (fp) {
				buf = NULL;
				read_len = buf_len = 0;
				while (1) {
					buf_len += STEP_BUF_SIZE;
					buf = (char *)realloc(buf, buf_len);
					if (!buf) {
						read_len=0;
						break;
					}
					tmp_len = fread(buf+read_len, 1, STEP_BUF_SIZE, fp);
					read_len += tmp_len;
					if (tmp_len < STEP_BUF_SIZE)
						break;

					if (read_len >= MAX_OUTBUF_SIZE) {
						_dprintf("%s: read_len is over defined size:%d\n", MAX_OUTBUF_SIZE);
						break;
					}
				}
				pclose(fp);
			}
			if (read_len) {
				write(sock, buf, read_len);
				usleep(10000);
			} else {
				cmd[0]='\0';
				write(sock, cmd, 1);
			}
		}
	}
	close(sock);
}

static int accept_sock(int sock, int nodelay)
{
	struct sockaddr_in addrc;
	int ret;
	int cfd;
	const int on=1;

	ret = sizeof(addrc);
	cfd = accept(sock, (struct sockaddr *)&addrc, (socklen_t *)&ret);
	if (cfd == -1)
		return -1;
	if (nodelay)
		setsockopt(cfd, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
	return cfd;
}

static int un_tcpsock_bind(char *path, int backlog, int nodelay)
{
	const int on = 1;
	int sock;
	struct sockaddr_un uaddr;

	memset ((char *)&uaddr, 0, sizeof(uaddr));

	if( (sock=socket(AF_LOCAL, SOCK_STREAM, 0)) == -1 )
	{
		_dprintf("%s: socket ERR:%s\n", __func__, strerror(errno));
		return -1;
	}

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	uaddr.sun_family = AF_LOCAL;
	strcpy(uaddr.sun_path, path);

	if ( (bind(sock, (struct sockaddr *)&uaddr, sizeof(uaddr))) == -1 ) {
		close (sock);
		_dprintf("%s: bind ERR:%s\n", __func__, strerror(errno));
		return -1;
	}

	listen(sock, backlog);

	if (nodelay)
		setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &on, sizeof(on));

	return (sock);
}

void PS_pod_main(void)
{
	int pid;
	int sfd, cfd;

	unlink(PS_SOCK);
	sfd = un_tcpsock_bind(PS_SOCK, BACK_LOG_SIZE, 1);
	if (sfd == -1) {
		_dprintf("%s: tcpsock_bind!\n", __func__);
		return;
	}

	signal(SIGCHLD, ps_clearchild);
	while (1) {
		cfd = accept_sock(sfd, 1);
		if (cfd == -1) {
			_dprintf("%s: accept_sock!\n", __func__);
			return;
		}
		if ((pid = fork()) > 0) { //parent
			close(cfd);
			cfd = -1;
			continue;
		}
		else if (pid == 0) { // child
			close(sfd);
			sfd = -1;
			server_handler(cfd);
			break;
		}
		else
			_dprintf("%s: fork err!\n", __func__);
	}
	return;
}
#endif

#ifdef RTCONFIG_GETREALIP
#define STUN_RESULT	"/tmp/stun.res"
int realip_main(int argc, char *argv[])
{
	char *stun_server_list[] = {
		"stun.l.google.com",
		"stun.stunprotocol.org",
		"stun4.l.google.com:19302",
		// "stun3.l.google.com:19302",
		// "stun2.l.google.com:19302",
		// "stun1.l.google.com:19302",
		"stun.pjsip.org",
		// "stun.linphone.org",
		// "stun.freeswitch.org",
		// "stun.develz.org",
		// "stun.wifirst.net",
		// "stun.sipnet.net",
		// "stun.sipgate.net",
		// "stun.rolmail.net",
		// "stun.nonoh.net",
		// "stun.nfon.net",
		// "stun.infra.net",
		// "stun.dus.net",
		""};
	int i = 0;
	int wan_unit;
	char wan_prefix[16] = {0};
	char wan_ifname[IFNAMSIZ] = {0};
	char wan_gateway[INET_ADDRSTRLEN] = {0};
	char stun_server[64] = {0};
	char *stun_argv[] = {"ministun", "-t", "1000", "-c", "1"
		, "-i", wan_ifname, stun_server
		, NULL};
	char buf[64] = {0};
	char *server, *tmp_server, *port;
	char stun_server_ip[INET_ADDRSTRLEN] = {0};
	char table_str[8] = "main";
#if defined(RTCONFIG_MULTIWAN_IF) || defined(RTCONFIG_DUALWAN)
	char pref_str[8] = "30";
#endif
#ifdef RTCONFIG_MULTIWAN_IF
	int is_mtwan;
#endif
#ifdef RTCONFIG_DUALWAN
	int load_balance = (nvram_match("wans_mode", "lb") && get_nr_wan_unit() > 1) ? 1 : 0;
#endif
	int ret;
	int success = 0;

	if (argc < 2) {
		printf("Usage: realip <wan_unit>\n");
		return EXIT_FAILURE;
	}

	wan_unit = atoi(argv[1]);
	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_unit);
#ifdef RTCONFIG_MULTIWAN_IF
	is_mtwan = is_mtwan_unit(wan_unit);
	if (is_mtwan) {
		wan_unit = mtwan_get_real_wan(wan_unit, wan_prefix, sizeof(wan_prefix));
		if (wan_unit < 0) {
			printf("Get WAN(%d) dualwan_idx failed\n", atoi(argv[1]));
			return -10;
		}
		mtwan_get_route_table_id(wan_unit, table_str, sizeof(table_str));
	}
#endif
#ifdef RTCONFIG_DUALWAN
	if (load_balance && wan_unit < WAN_UNIT_MAX)
		snprintf(table_str, sizeof(table_str), "%d", IP_ROUTE_TABLE_ID_DUALWAN_BASE + wan_unit * 100);
#endif
	if (nvram_pf_get_int(wan_prefix, "state_t") != WAN_STATE_CONNECTED) {
		printf("WAN(%d) is not connected\n", wan_unit);
		return -1;
	}

	while (nvram_get_int("link_internet") != 2)
		sleep(1);

	strlcpy(wan_ifname, get_wan_ifname(wan_unit), sizeof(wan_ifname));
	strlcpy(wan_gateway, nvram_pf_safe_get(wan_prefix, "gateway"), sizeof(wan_gateway));

	for (i = 0; stun_server_list[i]; i++) {
		strlcpy(buf, stun_server_list[i], sizeof(buf));
		server = tmp_server = buf;
		port = NULL;
		if (strchr(buf, ':')) {
			server = strsep(&tmp_server, ":");
			port = tmp_server;
		}
		if (resolv_addr4(server, stun_server_ip, sizeof(stun_server_ip)) < 0) {
			printf("resolve stun server failed\n");
			return -2;
		}

		// add stun server route
#ifdef RTCONFIG_MULTIWAN_IF
		if (is_mtwan)
			eval("ip", "rule", "add", "iif", "lo", "to", stun_server_ip, "table", table_str, "pref", pref_str);
#endif
#ifdef RTCONFIG_DUALWAN
		if (load_balance && wan_unit < WAN_UNIT_MAX)
			eval("ip", "rule", "add", "iif", "lo", "to", stun_server_ip, "table", table_str, "pref", pref_str);
#endif
		eval("ip", "route", "add", stun_server_ip, "via", wan_gateway, "dev", wan_ifname, "table", table_str);

		if (port)
			snprintf(stun_server, sizeof(stun_server), "%s:%s", stun_server_ip, port);
		else
			strlcpy(stun_server, stun_server_ip, sizeof(stun_server));
		// _dprintf("final stun server: %s\n", stun_server);

		ret = _eval(stun_argv, ">"STUN_RESULT, 0, NULL);

		// del stun server route
#ifdef RTCONFIG_MULTIWAN_IF
		eval("ip", "rule", "del", "iif", "lo", "to", stun_server_ip, "table", table_str, "pref", pref_str);
#endif
		eval("ip", "route", "del", stun_server_ip, "via", wan_gateway, "dev", wan_ifname, "table", table_str);

		if (!ret) {
			FILE *fp = fopen(STUN_RESULT, "r");
			while (fgets(buf, sizeof(buf), fp)) {
				trimNL(buf);
				if (is_valid_ip4(buf)) {
					success = 1;
					break;
				}
			}

			if (success) {
				printf("%s\n", buf);
				nvram_pf_set(wan_prefix, "realip_ip", buf);
				nvram_pf_set(wan_prefix, "realip_state", "2");
				break;
			}
		}
	}

	if (!success)
		nvram_pf_set(wan_prefix, "realip_state", "1");

	unlink(STUN_RESULT);

	return (success) ? 0 : -3;
}
#endif

#ifdef RTCONFIG_FAST_ACL_SET
void start_setacl_not_restart_wifi(void)
{
	int bssidx=-1;
	int vifidx=-1;
	int stop_flag=0;

	if(nvram_safe_get("acl_tmp_bssidx") == NULL || nvram_safe_get("acl_tmp_vifidx")== NULL){
		_dprintf("parameters for acl setting are incorrect\n");
		return;
	}

	bssidx=nvram_get_int("acl_tmp_bssidx");
	vifidx=nvram_get_int("acl_tmp_vifidx");

	nvram_unset("acl_tmp_bssidx");
	nvram_unset("acl_tmp_vifidx");

	//if(vifidx > 0)
	//	snprintf(prefix, sizeof(prefix), "wl%d.%d_", bssidx, vifidx);
	//else
	//	snprintf(prefix, sizeof(prefix), "wl%d_", bssidx);

	/* set a nvram parameter to avoid watchdog restart related services before acl setting is done */
	nvram_set_int("watchdog_wait_a_moment",1);
	/* stop related funcions and let watchdog restart them */
	check_stop_flag(&stop_flag);
#ifdef BCM_BSD	
	if(stop_flag&STOP_BSD) stop_bsd();
#endif	
	if(stop_flag&STOP_ROAMAST) stop_roamast();

	/* sysdeps: set mac mode/list and remove */
	set_acl_and_remove_sta(bssidx, vifidx);


	/* unset related nvram */
	nvram_unset("watchdog_wait_a_moment");

}
#endif //RTCONFIG_FAST_ACL_SET


#if defined(RTCONFIG_HND_ROUTER_AX)
int set_cable_media(const char *eth_inf, const char *media_type){
	FILE *fp;
	int ret = -1;
	char cmd[256], buf[512], *ptr;
#define SET_MEDIA_OK "Link is Up at Speed: "

	if(!eth_inf || *eth_inf == '\0'){
		_dprintf("%s: Not assign the ethernet interface\n", __func__);
		return ret;
	}

	if(!media_type || *media_type == '\0'){
		_dprintf("%s: Not assign the media type\n", __func__);
		return ret;
	}

	// Start to set media-type of the cable
	snprintf(cmd, sizeof(cmd), "ethctl %s media-type %s 2>/dev/null", eth_inf, media_type);
	if((fp = popen(cmd, "r")) != NULL){
		memset(buf, 0, sizeof(buf));
		while(fgets(buf, sizeof(buf), fp) != NULL) {
			if ((ptr = strstr(buf, SET_MEDIA_OK)) != NULL) { // Event Complete
				ret = 0;
				break;
			}
		}
		pclose(fp);
	}

	if(ret == 0)
		_dprintf("%s: the media type will be linked up by %s later\n", __func__, media_type);
	else
		_dprintf("%s: Fail to set %s to be %s\n", __func__, eth_inf, media_type);

	return 0;
}
#endif

#if defined(RTCONFIG_AMAS_CENTRAL_ADS)
void start_ds_measure()
{
        int i, j, send_rate, percent = 1,rate_sum = 0, delay_time = 0, test_time = 10;
        float rssi_ret, rssi_sum = 0;
        char *node_mac, *wlif_name;
        char host[18], send[12], iperf_time[8], ds_report[36], tmp_cmd[48];
	char *iperf3_argv[] = {"iperf3","-c","192.168.50.218","-u","-b","1000000","-t","10"};
	struct ads_info ads_info_cur;
	FILE *fp;
        pid_t pid;
#if defined(RTCONFIG_CFGSYNC)
	char event_msg[64] = {0};
#endif

        ads_info_cur.tx_datarate = 0;
        for( i=0; i<4; i++)
                ads_info_cur.tx_rssi[i] = 0;
        wlif_name = nvram_safe_get("ds_ifname");
        node_mac = nvram_safe_get("ds_sta_mac");
        strcpy(host, nvram_safe_get("ds_ip"));
	percent = nvram_get_int("ds_data_rate");
	test_time = nvram_get_int("ds_measure_time");
	delay_time = nvram_get_int("ds_delay_time");
	sprintf(iperf_time, "%d", test_time + delay_time);

	printf("start_ds_measure: %s, %s, %s, %d, %d, %d, %s\n", wlif_name, node_mac, host, percent, test_time, delay_time, iperf_time);

	get_datarate_rssi(wlif_name, node_mac, &ads_info_cur);
	sprintf(send, "%d", ads_info_cur.tx_datarate*percent);
	iperf3_argv[2] = host;
	iperf3_argv[5] = send;
	iperf3_argv[7] = iperf_time;
        sprintf(tmp_cmd, "%s %s %s", iperf3_argv[2], iperf3_argv[5], iperf3_argv[7]);
	_eval(iperf3_argv, NULL, 0, &pid);

	printf("Sleep %d secs then measure datarate/rssi...\n", delay_time);
	sleep(delay_time);

        i = 0;
        while (i < test_time) {
                get_datarate_rssi(wlif_name, node_mac, &ads_info_cur);
                rssi_ret = log10((pow(10, (double)ads_info_cur.tx_rssi[0]/10) + pow(10, (double)ads_info_cur.tx_rssi[1]/10) + pow(10, (double)ads_info_cur.tx_rssi[2]/10) + pow(10, (double)ads_info_cur.tx_rssi[3]/10))/4)*10;
                rssi_sum = rssi_sum + rssi_ret;
                rate_sum = rate_sum + ads_info_cur.tx_datarate;
                sleep(1);
                i++;
        }
        sprintf(ds_report, "/tmp/cfg_mnt/%s.dsr%s", get_lan_hwaddr(),nvram_safe_get("ds_seq"));
        my_mkdir("/tmp/cfg_mnt");
		if (nvram_get_int("ds_sta_disconn") == 0) {
			fp = fopen(ds_report, "w");
			if(fp) {
				fprintf(fp, "{\"rssi\":%.2f,\"data_rate\":%d,\"seq\":%d}\n", rssi_sum/test_time, rate_sum/test_time, nvram_get_int("ds_seq"));
			fclose(fp);

#if defined(RTCONFIG_CFGSYNC)
				snprintf(event_msg, sizeof(event_msg), RC_ADS_DS_MSG, EID_RC_REPORT_DS_RESULT, nvram_get_int("ds_seq"));
				send_cfgmnt_event(event_msg);
#endif
			}
		}
}

void handle_ds_switch(int role)
{
	char *ds_ap_status_argv[] = {"monitor_ds_ap_status", NULL};
	char *ds_sta_status_argv[] = {"monitor_ds_sta_status", NULL};
	pid_t pid;

	nvram_unset("ds_sta_disconn");

	if (switch_diversity_state()) {
		if (role == 1) {	/* for parent role */
			killall("monitor_ds_ap_status", SIGTERM);
			_eval(ds_ap_status_argv, NULL, 0, &pid);
		}
		else if (role == 2) {	/* for child role */
			killall("monitor_ds_sta_status", SIGTERM);
			_eval(ds_sta_status_argv, NULL, 0, &pid);
		}
	}
}

void handle_ds_measure(void)
{
	char *ds_measure_argv[] = {"start_ds_measure", NULL};
	pid_t pid;

	_eval(ds_measure_argv, NULL, 0, &pid);
}

void monitor_ds_sta_status()
{
	int i = 0, total_test_time = 0, sta_connected = 1, band_unit = nvram_get_int("ds_band_unit");
	int p = 0, dps_val = 0, switch_last = 0;
	char wl_dps[16], ds_port[16], last_ds_port[16];

	total_test_time = nvram_get_int("ds_measure_time") + nvram_get_int("ds_delay_time");

	if (total_test_time) {
		/* check sta status */
		while (i < total_test_time) {
			if (get_psta_status(band_unit) != WLC_STATE_CONNECTED || nvram_get("sta_disconn")) {
				sta_connected = 0;
				break;
			}

			i++;
			sleep(1);
		}

		if (sta_connected) {	/* sta is connected */
			/* record the related */
			nvram_set("last_ds_band_unit", nvram_safe_get("ds_band_unit"));
			nvram_set("last_ds_ifname", nvram_safe_get("ds_ifname"));

			snprintf(wl_dps, sizeof(wl_dps), "wl%d_dps", band_unit);
			if ((dps_val = nvram_get_int(wl_dps))) {
				p = 0;
				while (dps_val) {
					snprintf(ds_port, sizeof(ds_port), "ds_p%d", p);
					if (nvram_get(ds_port)) {
						snprintf(last_ds_port, sizeof(last_ds_port), "last_ds_p%d", p);
						nvram_set(last_ds_port, nvram_safe_get(ds_port));
					}

					dps_val = dps_val >> 4;
					p++;
				}
			}
		}
		else	/* sta is disconnected */
		{
			/* switch ds based on last related */
			nvram_set("ds_band_unit", nvram_safe_get("last_ds_band_unit"));
			nvram_set("ds_ifname", nvram_safe_get("last_ds_ifname"));

			snprintf(wl_dps, sizeof(wl_dps), "wl%d_dps", band_unit);
			if ((dps_val = nvram_get_int(wl_dps))) {
				p = 0;
				while (dps_val) {
					snprintf(last_ds_port, sizeof(last_ds_port), "last_ds_p%d", p);
					if (nvram_get(last_ds_port)) {
						snprintf(ds_port, sizeof(ds_port), "ds_p%d", p);
						nvram_set(ds_port, nvram_safe_get(last_ds_port));
						switch_last = 1;
					}

					dps_val = dps_val >> 4;
					p++;
				}

				if (switch_last) {
					switch_diversity_state();
					notify_rc("stop_iperf3_server");
				}
			}
		}
	}
}

void monitor_ds_ap_status()
{
	int i = 0, total_test_time = 0, sta_connected = 1, band_unit = nvram_get_int("ds_band_unit");
	int p = 0, dps_val = 0, switch_last = 0;
	char wl_dps[16], ds_port[16], last_ds_port[16], ifname[16], sta_mac[18];
#if defined(RTCONFIG_CFGSYNC)
	char event_msg[64] = {0};
#endif

	total_test_time = nvram_get_int("ds_measure_time") + nvram_get_int("ds_delay_time");

	strlcpy(ifname, nvram_safe_get("ds_ifname"), sizeof(ifname));
	strlcpy(sta_mac, nvram_safe_get("ds_sta_mac"), sizeof(sta_mac));

	if (total_test_time) {
		/* check sta status */
		while (i < total_test_time) {
			if (check_sta_status_by_mac(ifname, sta_mac) == 0 || nvram_get("sta_disconn")) {
				sta_connected = 0;
				break;
			}

			i++;
			sleep(1);
		}

		if (sta_connected) {	/* sta is connected */
			/* record the related */
			nvram_set("last_ds_band_unit", nvram_safe_get("ds_band_unit"));
			nvram_set("last_ds_ifname", nvram_safe_get("ds_ifname"));

			snprintf(wl_dps, sizeof(wl_dps), "wl%d_dps", band_unit);
			if ((dps_val = nvram_get_int(wl_dps))) {
				p = 0;
				while (dps_val) {
					snprintf(ds_port, sizeof(ds_port), "ds_p%d", p);
					if (nvram_get(ds_port)) {
						snprintf(last_ds_port, sizeof(last_ds_port), "last_ds_p%d", p);
						nvram_set(last_ds_port, nvram_safe_get(ds_port));
					}

					dps_val = dps_val >> 4;
					p++;
				}
			}
		}
		else	/* sta is disconnected */
		{
			/* switch ds based on last related */
			nvram_set("ds_band_unit", nvram_safe_get("last_ds_band_unit"));
			nvram_set("ds_ifname", nvram_safe_get("last_ds_ifname"));
			nvram_set("ds_sta_disconn", "1");

			snprintf(wl_dps, sizeof(wl_dps), "wl%d_dps", band_unit);
			if ((dps_val = nvram_get_int(wl_dps))) {
				p = 0;
				while (dps_val) {
					snprintf(last_ds_port, sizeof(last_ds_port), "last_ds_p%d", p);
					if (nvram_get(last_ds_port)) {
						snprintf(ds_port, sizeof(ds_port), "ds_p%d", p);
						nvram_set(ds_port, nvram_safe_get(last_ds_port));
						switch_last = 1;
					}

					dps_val = dps_val >> 4;
					p++;
				}

				if (switch_last) {
					switch_diversity_state();
					notify_rc("stop_iperf3_client");
				}
			}

#if defined(RTCONFIG_CFGSYNC)
			snprintf(event_msg, sizeof(event_msg), RC_ADS_DS_MSG, EID_RC_REPORT_DS_SWITCH_STA_DISCONN, nvram_get_int("ds_seq"));
			send_cfgmnt_event(event_msg);
#endif
		}
	}
}
#endif

#ifdef RTCONFIG_TRUSTZONE
void start_tee(void)
{
	system("tee-supplicant &");

	killall_tk("ateed");
	eval("ateed");
}

void stop_tee(void)
{
	killall_tk("ateed");
	killall_tk("tee-supplicant");
}
#endif
