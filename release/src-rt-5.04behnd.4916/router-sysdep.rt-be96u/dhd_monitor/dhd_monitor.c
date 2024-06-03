/*
 * DHD Monitor Daemon
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_monitor.c 833330 2023-11-23 07:24:04Z $
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <shutils.h>
#include <pthread.h>
#include <limits.h>
#include <wlioctl.h>
#include <wlutils.h>
#include <bcmutils.h>
#include <bcmevent.h>
#include <bcmnvram.h>
#ifdef BCA_HNDROUTER
#include <sys/utsname.h> /* for uname */
#endif /* BCA_HNDROUTER */
#include <bcmnvram.h>

/* defines */
#ifndef RAMFS_MAGIC
#define RAMFS_MAGIC		0x858458f6
#endif

#ifndef TMPFS_MAGIC
#define TMPFS_MAGIC		0x01021994
#endif

#define	CRASH_LOG_FOLDERNAME_SZ		40
#define	LOG_BASE_PATH			"/tmp/crash_logs"
#define	LOG_HC_FOLDER			"healthcheck"
#define	CRASH_LOG_PREFIX		"debug-"
#define	DSTALL_LOG_PREFIX		"hc_dstall_"
#define	HC_DBG_DATA_PREFIX		"hc_"
#define	LOG_UNAME_FILENAME		"uname.txt"
#define	LOG_DMESG_FILENAME		"dmesg.txt"
#define	LOG_DONGLE_MEM_FILENAME		"dongle_mem_dump.bin"
#define LOG_CONSOLE_DUMP                "console_dump.txt"
#define LOG_SOCRAM_DUMP                 "socram_dump.txt"
#define HC_BASE_PATH			"/tmp/hc/"
#define HC_TEMP_FILE			"/tmp/hc/tmp.txt"
#define HC_SOCRAM_FILE			"/tmp/hc/socram_dump.txt"
#define HC_LOG_BUFF			(300)
#define IFNAMSIZ			16
#define SOCRAM_DUMP_ON_PROLONG_STALL	0
#define ETHER_ADDR_STR_LEN		18
#define MAX_WL_IFS			4
#define CONFLUENCE_LINK_DSTALL		"Confluence: [%cx+data+stall+health+check]"
#define LOG_NVRAM_DUMP_FILENAME		"nvram_dump.txt"
#define	DSTALL_HEADER			"%cxStall detected at %s level for MAC:%s"

#ifdef BCA_SUPPORT_UNFWLCFG
extern char *nvram_kget(const char *name);
#define nvram_safe_kget(nnn) (nvram_kget(nnn) ? : "")
#else
#define nvram_safe_kget(nnn) (nvram_get(nnn) ? : "")
#endif

#define PCIE_IPC_AP_UNIT_MAX	4
#define WL_MLO_DEFAULT		"-1 -1 -1 -1"

//#if defined(BCA_CPEROUTER)
#if 0
#define rc_stop()		/* do nothing */
#define rc_start()		system("nvram restart")
#define rc_restart()		system("nvram restart")
#define rc_reboot()		system("reboot")

#define unload_driver(nic)	(nic) ?					\
	system("/etc/init.d/bcm-wlan-drivers.sh stop wl") :		\
	system("/etc/init.d/bcm-wlan-drivers.sh stop dhd")
#define load_driver(nic)	(nic) ?					\
	system("/etc/init.d/bcm-wlan-drivers.sh start wl") :		\
	system("/etc/init.d/bcm-wlan-drivers.sh start dhd")

#elif defined(BCA_HNDROUTER)

#define rc_stop()		kill(1, SIGINT)
#define rc_start()		kill(1, SIGUSR2)
#define rc_restart()		kill(1, SIGHUP)

#if defined(CMWIFI)
#define rc_reboot()		system("reboot")
#else
#define rc_reboot()		kill(1, SIGTERM)
#endif /* CMWIFI */

#if defined(RTBE86U)
int instance_base = 1;
#else
int instance_base = 0;
#endif
#if defined(BT10) || defined(RTBE95U)
int instance_base_wl = 1;
#else
int instance_base_wl = 0;
#endif
#define unload_driver(nic)	eval("rmmod", (nic ? "wl" : "dhd"))
#define load_driver(nic)	{					\
	struct utsname name;						\
	char buf[PATH_MAX];						\
	char buf2[64];							\
	char buf3[64];							\
	uname(&name);							\
	snprintf(buf, sizeof(buf),					\
	(nic ? "/lib/modules/%s/extra/wl.ko" : "/lib/modules/%s/extra/dhd.ko"),	\
		name.release);						\
	snprintf(buf2, sizeof(buf2), "instance_base=%d dhd_msg_level=0 iface_name=wl", instance_base); \
	snprintf(buf3, sizeof(buf3), "instance_base=%d intf_name=wl%d", instance_base_wl); \
	eval("insmod", buf, nic ? buf3 : buf2); \
}

#else /* ! BCA_HNDROUTER && !BCA_CPEROUTER */

#define rc_stop()		kill(1, SIGINT)
#define rc_start()		kill(1, SIGUSR2)
#define rc_restart()		kill(1, SIGHUP)
#define rc_reboot()		kill(1, SIGTERM)

#define unload_driver(nic)	eval("rmmod", (nic ? "wl" : "dhd"))
#define load_driver(nic)	eval("insmod", (nic ? "/tmp/wl.ko" : "/tmp/dhd.ko"))
#endif /* !BCA_CPEROUTER && !BCA_HNDROUTER */

/* Number of crash logs to retain by default */
#define DEFAULT_CRASH_LOGS_LIMIT	3
/* In case of MLO */
#define DEFAULT_CRASH_LOGS_MLO_LIMIT	6

/* Number of HC logs (non trap cases) to retain */
#define MAX_HC_LOGS		(100)

/* A no crash HC like datastalls may only save some debug logs, not memdumps */
#define SIZE_SMALL_HC_LOGS (6*1024)

#ifndef TRUE
#define TRUE			1
#endif /* TRUE */
#ifndef FALSE
#define FALSE			0
#endif /* FALSE */

#define DM_BUF_SZ		256
#define DM_DIR			"/tmp/dm"
#define DM_DEPENDENCY_DEPTH_MAX	6	// recurse up to DM_DEPENDENCY_DEPTH_MAX
#define DM_INTERVAL 5 /* call debug monitor at every 5th tick only */

#ifdef DM_INTERNAL_QUEUE_SIZE
#define DM_QUEUE_MAX		128
#else
#define DM_QUEUE_MAX		16
#endif /* DM_INTERNAL_QUEUE_SIZE */

#define DUMP_BASE_PATH		"/tmp"
#define DUMP_BASE_MISC_PATH	"/mnt/misc/crash_logs"
#define BACKUP_BASE_PATH	"/data"
#define IFNAME_STR		"wl%d"
#define DUMP_COMPLETION		0x100
#define CMD_TRY_CNT		5
/*
* Worst case
* It takes about 60 sec to get the dump completion
* Then the timeout for the completion is increased with additional 10 sec
*/
#define PENDING_CNT		70
#define WAIT_FOR_MLO_TIMEOUT	20

#define CMD_LEN			256
#define FILE_LEN		128
#define PATH_LEN		256
#define TIMESTAMP_LEN		64
#define DRIVER_NAME_LEN		10
#define IFNAME_LEN		4
#define DUMP_SIGN_LEN		4

/* Signal processing status */
#define NOTYETPROCESS		0
#define INPROGRESS		1
#define PROCESSED_DONE		2

/* Worst case : the size of one tgz file is around 6M */
#define ONE_TGZ_SIZE		(6 * 1024 * 1024)
/*
 * Worst case :
 * the total size of all the dumps created by a driver is about 50M.
 * 3 interfaces x 50M = 150Mbytes
 */
#define LIMIT_RAM_DISK_SIZE	(150 * 1024 * 1024)

/* if a ram disk is used - the max number of tgz files */
#define CRASH_LOGS_RAMFS_MAX_LIMIT	(LIMIT_RAM_DISK_SIZE / ONE_TGZ_SIZE)

#define NIC_MLO_SIG_FLAG	0x200

#define DM_PRINT(fmt, arg...)   \
	do { \
		printf("debug_monitor: "fmt, ##arg); \
		if (dhd_monitor_msglevel & DHD_MONITOR_SYSLOG) \
		logmessage("dhd_monitor", "%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

int dhd_monitor_msglevel = 0;

/* Debug Print */
#define DHD_MONITOR_ERROR	0x000001
#define DHD_MONITOR_SYSLOG	0x000004

#define dprintf(fmt, arg...) \
	do { \
		if (dhd_monitor_msglevel & DHD_MONITOR_ERROR) \
		fprintf(stderr, fmt, ##arg); \
		if (dhd_monitor_msglevel & DHD_MONITOR_SYSLOG) \
		logmessage("dhd_monitor", "%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

char *_mod_name = NULL;
int dbg_mon_disab_rstrt = FALSE;

typedef struct dm_sig_info {
	int signo; /* NIC, FD, HC, or MLO */
	int ifidx; /* for example, for wl1, 1 is ifidx */
	int dump_cmpl; /* dump generation was done */
	int is_recovery; /* need recovery or not */
	int status; /* signal processed or not */
	char timestamp[TIMESTAMP_LEN]; /* time to get the signal */
	time_t rawtime; /* To save the time to receive a signal */
	char drv_type[DRIVER_NAME_LEN]; /* save driver name. e.g DHD, NIC */
	int check_init; /* a flag for CHECK_DUMPS_WHEN_INIT */
	char dump_signature[DUMP_SIGN_LEN + 1]; /* dump_iovar_5c02.txt - 5c02 */
	char filename[FILE_LEN]; /* contain filename like debug-2023_02_07_07_17_06_5c02_wl1_DHD */
	int mlo_sig;
} dm_sig_info_t;

typedef struct _dm_sig_status {
	dm_sig_info_t sig_queue[DM_QUEUE_MAX];
	/* index for received signal */
	int received;
	/* The number of received signals */
	int tot_received;
	/* the number of processed signals */
	int tot_processed;
	int cur_idx; //Current processing index
	int overwrap;
	int recovery_cnt;
	/* For mlo,
	* need to wait for catching all the signals
	* from all the mlo interfaces
	*/
	int wait_for_mlo;
	int wait_for_mlo_timeout;
	/* To check which interface reported a signal for mlo */
	int if_check[PCIE_IPC_AP_UNIT_MAX];
	/* Current signal in progress */
	dm_sig_info_t cur_sig;
	/* time cnt for cur sig */
	int cur_time_cnt;
} dm_sig_status_t;

/* Health check signal related local info */
typedef struct hc_sig_info {
	uint hc_sig;
	uint hc_log_only; /* Only collect small debug logs, not memdump */
	uint is_dstall;		/* Is it a data(rx or tx) stall signal */
	uint dstall_dir;	/* Rx stall or Tx stall */
	uint n_dstall_reports;
} hc_sig_info_t;

static int dm_is_process_active(char * dirname);
static void dm_trim(char *str, size_t sz);
static int dm_read_pid_info_file(char *path, char *dep_cmd, size_t dep_cmd_sz,
	char *process_cmd, size_t process_cmd_sz);
static int dm_handle_process(char *pid, unsigned int depth);
static unsigned char dm_match_cmd(char *pid, char *process_cmd_needle);
static unsigned char dm_get_pid_of_cmd(char *dep_cmd, char *dep_pid, size_t dep_pid_sz);
static void dm_watchdog();

#if defined(__CONFIG_DHDAP__)

dm_sig_status_t dm_sig_status;
bool is_sig_inprogress = FALSE;

static int create_log_tmp_dirs(void);
static int create_dir(char *pdir);
static void handle_hc_dstall(char *timestamp, int nic_wl, int ifidx,
	hc_sig_info_t *hc_sig_info, char *basepath);

static bool is_all_mlo_if(void);
static int check_file_size(const char *srcfile, const char *tofile);
static int scan_dump_dir(const char *path);
static int compare_files(const char *from, const char *to,
	const char *fname, int ifidx, const char *dsign);
static bool copy_files(const char *from, const char *to,
	const char *fname, int ifidx, const char *dsign);
static void time_formatting(time_t *rawtime, char *fmt);
static void driver_type_str(dm_sig_info_t *siginfo);

static unsigned long get_dir_free_space(const char *dir);
static int get_sys_free_mem(void);

static bool is_driver_dump_path_to_misc(void);
static bool is_misc_partition_mounted(void);
static void print_storage_size(void);

static bool is_mem_dump_exist(int ifidx);
static void check_missing_mlo_signals(void);
static void sig_handler(int signo, siginfo_t *sinfo, void *ctx);
static void sig_handler_queueing(dm_sig_info_t *siginfo);
static void sig_handler_watchdog(void);

static void check_recovery(dm_sig_info_t *sinfo, hc_sig_info_t *hc_sig_info);
static bool is_fast_restart(void);
static bool is_watchdog_enabled(void);
static void check_max_crash_logs(void);

static int is_ramfs(const char *dir);
static bool is_ramfs_mounted(void);
static bool is_misc_size_enough(void);
static void check_auto_mount_ramdisk(void);

static bool is_wlup(int ifidx);
static void dm_term_init(void);
static int check_dump_dir(char *path);

#ifdef DM_NO_NVRAM_RESTART
static bool is_nvram_restart(void);
static void check_nvram_restart_done(void);
#endif /* DM_NO_NVRAM_RESTART */

#ifdef CRASH_HISTORY_FILE_LOGGING
/* The reason to create a history
 * is because there were cutomers'issues about false alarm
 */

/* The history needs to be stored permanently
* so it will be stored into /data directory
*/
#define CRASH_HISTORY_PATH              "/data"
#define CRASH_HISTORY_FILE_PREFIX               "crash_history_"
#define CRASH_TGZ_HISTORY_FILENAME              CRASH_HISTORY_FILE_PREFIX"tgz_gen_log.txt"
#define CRASH_SIGINFO_HISTORY_FILENAME          CRASH_HISTORY_FILE_PREFIX"siginfo_log.txt"
#define CRASH_TGZ_HISTORY_LOG_NUM_MAX           20
#define CRASH_SIGINFO_HISTORY_LOG_NUM_MAX       40

/* timestamp(tab)signal number(tab)driver type(tab)interface name(tab)tgz file name */
#define CRASH_TGZ_REC_FMT                       "%s\t%d\t%s\t%s\t%s"
/* timestamp(tab)signal number(tab)driver type(tab)interface idx(tab)ifname(tab)
 * dump completion(tab)tgz file name
 */
#define CRASH_SIGINFO_REC_FMT                   "%s\t%d\t%s\t%d\t%s\t%d\t%s"

/* Each record will be stored into the history file */
typedef struct dm_record_info {
	char time[TIMESTAMP_LEN];
	char tgzname[FILE_LEN];
	char driver[DRIVER_NAME_LEN];
	char ifname[IFNAME_LEN];
	int ifidx;
	int signo;
	int dump_cmpl;
} dm_record_info_t;

/* Structure for the history file */
typedef struct dm_history_info {
	char tgz_gen_history_file[PATH_LEN]; /* filepath for tarball creation */
	char siginfo_file[PATH_LEN]; /* filepath for receiving a signal */
	FILE *tgz_gen_history_fp; /* file descriptor to point out the file */
	FILE *siginfo_fp;
	int tgz_items; /* current recorded items for tarball creation */
	int sig_items; /* current recorded items for received signals */
} dm_history_info_t;

static void create_history_files(char *_backup_dir);
static int open_history_files(void);
static void close_history_files(void);

static void write_tgz_gen_history(dm_sig_info_t *_siginfo);
static void write_siginfo_history(dm_sig_info_t *siginfo);

static dm_history_info_t dm_history = {0, };
#endif /* CRASH_HISTORY_FILE_LOGGING */

#define SIGUSR3 SIGWINCH

/* Define SIGUSR4 for health check exclusively
 * Use SIGURG(default action ignore) for hc handling in the process.
 */
#define SIGUSR4 SIGURG

/* MLO_IPC: Initialize all array element to -1. */
int nvram_mlo[PCIE_IPC_AP_UNIT_MAX] = { [ 0 ... (PCIE_IPC_AP_UNIT_MAX-1) ] = -1 };

char *_backup_dir = NULL;

int max_crash_logs = DEFAULT_CRASH_LOGS_LIMIT;  /* by default 3 */

char ifname[] = "wlxxx";
char* covert_ifidx_to_ifname(int ifidx) {
#if defined(BT12) || defined(BT10) || defined(BQ16) || defined(BQ16_PRO)
	snprintf(ifname, sizeof(ifname), "wl%d", ifidx);

	return &ifname[0];
#else
        char nv_name[16];
        snprintf(nv_name, sizeof(nv_name), "wl%d_ifname", ifidx);

	return nvram_safe_get(nv_name);
#endif
}


/* If returned value indicates an error happened,
 * retry again
 */
static int
do_command(const char *cmd)
{
	int cnt = 0, ret = BCME_OK, status = -1;

	/* system() returns -1 if error
	* WIFEXITED() returns zero if error
	* WEXITSTATUS () returns non-zero if error
	*/

	while ((status == -1) || !WIFEXITED(status) ||
		(WIFEXITED(status) && WEXITSTATUS(status))) {

		status = system(cmd);
		cnt++;

		if (cnt > CMD_TRY_CNT) {
			DM_PRINT("do_command cmd = %s, status = %d\n", cmd, status);
			DM_PRINT("WIFEXITED %d, WEXITSTATUS %d\n",
				WIFEXITED(status), WEXITSTATUS(status));
			DM_PRINT("errno(%d)  = %s\n", errno, strerror(errno));
			ret = BCME_ERROR;
			break;
		}
	}

	return ret;
}

static void
get_timestamp(time_t *rawtime, char *buffer, int max_len)
{
	time_t tmptime;
	struct tm *info;

	if (rawtime) {
		info = localtime(rawtime);
	} else {
		time(&tmptime);
		info = localtime(&tmptime);
	}
	strftime(buffer, max_len, "%F_%T", info);
	return;
}

static void
time_formatting(time_t *rawtime, char *fmt)
{
	char *tmp;
	/* get current timestamp */
	get_timestamp(rawtime, fmt, TIMESTAMP_LEN - 1);
	/* reformatting time stamp string */
	tmp = fmt;
	while (*tmp) {
		if (*tmp == '-' || *tmp == ':') {
			*tmp = '_';
		}
		tmp++;
	 }
}

#ifdef DM_NO_NVRAM_RESTART
static bool
is_nvram_restart(void)
{
	 char *val = (char *) nvram_get("_wlrestart_");
	 if (val && (val[0] == '1') && (val[1] == '\0')) {
		    return TRUE;
	 }
	 return FALSE;
}

static void
check_nvram_restart_done()
{
	while (is_nvram_restart()) {
		sleep(1);
	}
}
#endif /* DM_NO_NVRAM_RESTART */

static bool
is_wlup(int ifidx)
{
	int val = 0, ret = BCME_OK;
	char ifname[IFNAME_LEN] = {0, };

	snprintf(ifname, sizeof(ifname), IFNAME_STR, ifidx);
	ret = wl_ioctl(ifname, WLC_GET_UP, &val, sizeof(val));
	if (!val || ret < 0) {
		DM_PRINT("%s isn't up, ret = %d, val = %d\n", ifname, ret, val);
		return FALSE;
	}

	return TRUE;
}

static void
handle_recovery(int nic_wl, int ifidx, int is_full_reboot)
{
	char cmd[CMD_LEN] = {0, };

	/*
	* If reboot is triggered,
	* all things in the ram disk will go away.
	* For ramdisk, fast_restart is required for recovery.
	*/
	if (!is_full_reboot &&
		(is_fast_restart() || is_ramfs_mounted())) {
		/*
		 * check support of wifi deep sleep power save.
		 * Wifi dsps is not supported for MLO capable radios.
		 */
#ifdef DM_NO_NVRAM_RESTART
		check_nvram_restart_done();
#endif /* DM_NO_NVRAM_RESTART */
		if (!(nvram_mlo[ifidx] >= 0) && (system("which wifi_dsps.sh") == 0)) {
			/* Fast Restart:
			 * wlconf <wlx> power_down/power_up
			*/
			DM_PRINT("wlconf power_down & up for recovery\n");

			snprintf(cmd, sizeof(cmd), "wlconf "IFNAME_STR" power_down &", covert_ifidx_to_ifname(ifidx));
			cmd[(int)sizeof(cmd) - 1] = '\0';
			do_command(cmd);
			sleep(3);

			snprintf(cmd, sizeof(cmd), "wlconf "IFNAME_STR" power_up &", covert_ifidx_to_ifname(ifidx));
			cmd[(int)sizeof(cmd) - 1] = '\0';
			do_command(cmd);
			sleep(8);
		}

		if (!is_wlup(ifidx)) {
			/* Fast Restart - rc_restart */
			/* stop all services */
			DM_PRINT("Stop services\n");
//			rc_stop();

			/*
			* rc_stop is a non-blocking call. So, add sufficient sleep
			* to make sure all services are stopped before proceeding.
			*/
			sleep(3);

			/* unload dhd */
			DM_PRINT("Unload dhd\n");
			unload_driver(nic_wl);
			sleep(1);

			/* reload dhd */
			DM_PRINT("Reload dhd\n");
			load_driver(nic_wl);
			sleep(1);

			/* start all services */
			DM_PRINT("Restart services\n");
//			rc_start();
			sleep(1);
			system("restart_wireless");
		}

		/*
		* if wlan interface doesn't recover in the case of fast_restart,
		* reboot will be triggered for recovery.
		*/
#ifdef DM_NO_NVRAM_RESTART
		check_nvram_restart_done();
#endif /* DM_NO_NVRAM_RESTART */

		if (!is_wlup(ifidx)) {
			DM_PRINT("Not up from fast_restart - reboot\n");
			rc_reboot();
		}
	} else {
		if (is_watchdog_enabled()) {
			/* full reboot */
			rc_reboot();
		} else {
			DM_PRINT("Watchdog disabled, ignoring dongle trap/wl kernel panic\n");
		} /* is_watchdog_enabled() */
	} /* if (!is_full_reboot ... */
}

/* Filter out none "*.tgz" files */
static int
dir_filter(const struct dirent *dptr)
{
	if (strstr(dptr->d_name, CRASH_LOG_PREFIX) && strstr(dptr->d_name, ".tgz")) {
		return 1;
	}
	return 0;
}

/*
* Remove any extra crash logs that might have accumulated over time
* Prune the oldest crash logs
* retains: Number of crash logs to retain
*/
static int
delete_extra_crash_files(const char *log_dir, uint retains, char *last_dump_sign)
{
	struct dirent **fnamelist;
	int n, i;
	char filepath[PATH_LEN];
	char *dump_sign;

	if (!log_dir) {
		DM_PRINT("log_dir is NULL\n");
		return -1;
	}

	n = scandir(log_dir, &fnamelist, dir_filter, alphasort);
	if (n < 0) {
		DM_PRINT("could not scan log_dir(%s) folder, error: %s\n",
			log_dir, strerror(errno));
	} else {
		int deletes = n - retains;

		dump_sign = nvram_get("crash_log_last_dump_sign");
		if (last_dump_sign) {
			/* dump signature length always is 4 */
			if (strlen(last_dump_sign) == 4) {
				dump_sign = last_dump_sign;
			}
		}

		for (i = 0; i < n; i++) {
			if (deletes > 0) {
				/* To keep a dump with the latest dump signature */
				if (dump_sign &&
					strstr(fnamelist[i]->d_name, dump_sign)) {
					DM_PRINT("the latest dump sign is %s, "
						"don't delete %s\n", dump_sign,
						fnamelist[i]->d_name);
					continue;
				}

				deletes--;
				DM_PRINT("pruning old crash log - %s/%s\n", log_dir,
					fnamelist[i]->d_name);
				snprintf(filepath, sizeof(filepath), "%s/%s", log_dir,
					fnamelist[i]->d_name);
				unlink(filepath);
			}
			/* free each entry allocated by scandir() */
			free(fnamelist[i]);
		}
		free(fnamelist);
	}

	return BCME_OK;
}

static int
get_sys_free_mem(void)
{
	FILE *fp;
	char buf[PATH_MAX];
	int free_mem = -1;

	if ((fp = fopen("/proc/meminfo", "r")) == NULL) {
		DM_PRINT("Read meminfo fail, %s\n", strerror(errno));
		return free_mem;
	}

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (sscanf(buf, "MemFree: %d KB", &free_mem)) {
			break;
		}
	}
	fclose(fp);

	return free_mem;
}

static unsigned long
get_dir_free_space(const char *dir)
{
	struct statfs dir_statfs;
	unsigned long free_space = 0;

	if (statfs(dir, &dir_statfs)) {
		DM_PRINT("statfs backup dir %s fail, error: %s\n", dir, strerror(errno));
		return BCME_ERROR;
	}

	if (dir_statfs.f_type == RAMFS_MAGIC || dir_statfs.f_type == TMPFS_MAGIC) {
		if ((free_space = get_sys_free_mem()) < 0) {
			DM_PRINT("Get sys free mem fail\n");
			return BCME_ERROR;
		}
		free_space = free_space * 1024;
	} else {
		free_space = dir_statfs.f_bsize * dir_statfs.f_bfree;
	}

	return free_space;
}

static int
is_ramfs(const char *dir)
{
	struct statfs dir_statfs;
	int ret = FALSE;

	if (statfs(dir, &dir_statfs)) {
		DM_PRINT("statfs backup dir %s fail, error: %s\n", dir, strerror(errno));
		return ret;
	}

	if (dir_statfs.f_type == RAMFS_MAGIC || dir_statfs.f_type == TMPFS_MAGIC) {
		ret = TRUE;
	}

	return ret;
}

static int
backup_logs(dm_sig_info_t *siginfo, const char *backup_dir,
	hc_sig_info_t *hc_sig_info)
{
	char logfile[FILE_LEN] = {0, }; //FILE_LEN 128
	char cp_file[256];
	char fullpath[PATH_LEN] = {0, }; //PATH_LEN 256
	char backup_path[PATH_LEN] = {0, };
	char src_path[PATH_LEN] = {0, };
	char cmd[CMD_LEN] = {0, };
	DIR *dir;
	struct stat log_stat;
	/* some customers use big stroage so interger cannot be enough sometimes */
	unsigned long free_space = 0;
	int ret = -1;

	if (!backup_dir) {
		DM_PRINT("backup_dir is NULL\n");
		goto fail;
	}

	snprintf(src_path, sizeof(src_path), "%s/", LOG_BASE_PATH);
	if (hc_sig_info &&
		hc_sig_info->hc_log_only) {
		snprintf(backup_path, sizeof(backup_path), "%s/%s/", _backup_dir,
			LOG_HC_FOLDER);
	} else {
		snprintf(backup_path, sizeof(backup_path), "%s/", _backup_dir);
	}

	snprintf(logfile, sizeof(logfile), "%s.tgz", siginfo->filename);
	snprintf(fullpath, sizeof(fullpath), "%s%s", src_path, logfile);

	if (!(dir = opendir(backup_path))) {
		DM_PRINT("Open backup dir %s fail, %s\n", backup_path, strerror(errno));
		goto fail;
	}
	closedir(dir);

	if (access(backup_path, W_OK)) {
		DM_PRINT("Access backup dir %s fail, error: %s\n", backup_path, strerror(errno));
		goto fail;
	}

	if (stat(fullpath, &log_stat)) {
		DM_PRINT("Open log file %s fail, %s\n", logfile, strerror(errno));
		goto fail;
	}

	free_space = get_dir_free_space(backup_dir);

	if (log_stat.st_size > free_space || log_stat.st_size == 0) {
		DM_PRINT("Log size (%d) over backup dir (%s%s) available space (%lu bytes)\n",
			log_stat.st_size, backup_dir,
			is_ramfs(backup_dir)? " a ram based fs": "", free_space);
		goto fail;
	}

	if (!copy_files(src_path, backup_path, logfile,
		siginfo->ifidx, siginfo->dump_signature)) {
		DM_PRINT("Failed to copy %s to %s", fullpath, backup_path);
		goto fail;
	}

	/*
	* delete all the things in /tmp/crash_logs
	* because tgz files are successfully moved.
	*/
	snprintf(cmd, sizeof(cmd), "rm -rf %s*", src_path);
	do_command(cmd);

	nvram_set("crash_log_last_dump_sign", siginfo->dump_signature);

	DM_PRINT("logfile (%s) are backed up to (%s%s)\n",
		fullpath, backup_path, is_ramfs(backup_path)? " a ram based fs": "");

	snprintf(cp_file, sizeof(cp_file), "rm -rf %s", LOG_BASE_PATH);
	ret = do_command(cp_file);
	if (ret < 0)
		dprintf("%s: failed to delete temp folder %s\n", _mod_name, LOG_BASE_PATH);

	if (is_ramfs(backup_path)) {
		DM_PRINT("Please copy tgz files before rebooting "
			"because tgz files are in a ram disk\n");
	}

	sync();
	fflush(stdout);
	return 0;

fail:
	snprintf(cp_file, sizeof(cp_file), "rm -rf %s", LOG_BASE_PATH);
	ret = do_command(cp_file);
	if (ret < 0)
		dprintf("%s: failed to delete temp folder %s\n", _mod_name, LOG_BASE_PATH);

	fflush(stderr);
	return ret;
}

int
merge_logs(char *subcommand, const char *dest, const char *src)
{
	char *file_buff = NULL;
	char delimiter[] = "--------------------------------------"
		"---------------------------------------\n";
	FILE *fp = NULL, *fp_temp = NULL;
	int bytes = 0;
	char cmd[100] = {0};
	int ret = BCME_OK;

	file_buff = (char*) malloc(HC_LOG_BUFF);
	if (file_buff == NULL) {
		DM_PRINT("Malloc failure\n");
		ret = BCME_ERROR;
		goto exit;
	}

	fp = fopen(dest, "a+");
	if (fp == NULL) {
		DM_PRINT("Cannot open file %s\n", dest);
		ret = BCME_ERROR;
		goto exit;
	}

	fp_temp = fopen(src, "r");
	if (fp_temp == NULL) {
		DM_PRINT("Cannot open file %s\n", src);
		ret = BCME_ERROR;
		goto exit;
	}

	/* update confluence link,  delimiter and type of log info to  main file */
	if (subcommand) {
		fprintf(fp, "%s", "\n");
		fprintf(fp, "%s", delimiter);
		fprintf(fp, "%s", subcommand);
		fprintf(fp, "%s", "\n");
		fprintf(fp, "%s", delimiter);
	} else {
		fprintf(fp, "%s", "\n");
	}

	while (fgets(file_buff, sizeof(file_buff), fp_temp)) {
		fprintf(fp, "%s", file_buff);
	}

	/* remove temp file */
	snprintf(cmd, sizeof(cmd), "rm -f %s", src);
	do_command(cmd);

exit:
	if (file_buff)
		free(file_buff);
	if (fp)
		fclose(fp);
	if (fp_temp)
		fclose(fp_temp);

	return ret;
}

void
capture_dstall_generic_logs(char *logfile, char *ifname)
{
	/* update buffer length based on the command/subcommand length */
	char command[100] = {0};
	char subcommand[50] = {0};

	snprintf(command, sizeof(command), "wl -i %s counters > %s", ifname, HC_TEMP_FILE);
	snprintf(subcommand, sizeof(subcommand), "wl -i %s counters", ifname);
	do_command(command);
	merge_logs(subcommand, logfile, HC_TEMP_FILE);

	/* Reset the counters */
	snprintf(command, sizeof(command), "wl -i %s reset_cnts", ifname);
	do_command(command);

	/* chanim_stats logs */
	snprintf(command, sizeof(command), "wl -i %s chanim_stats > %s", ifname, HC_TEMP_FILE);
	snprintf(subcommand, sizeof(subcommand), "wl -i %s chanim_stats", ifname);
	do_command(command);
	merge_logs(subcommand, logfile, HC_TEMP_FILE);
}

void
capture_dstall_scb_logs(char *logfile, char *ifname, char *eabuf)
{
	/* update buffer length based on the command length */
	char command[150] = {0};
	char subcommand[150] = {0};

	/* dummy command here needed as first dpstats dumps are not displaying updated
	 * value and the second command does it.
	 */
	snprintf(command, sizeof(command), "wl -i %s dpstats c:// a:%s m:%s p:%s > %s",
		ifname, eabuf, eabuf, eabuf, HC_TEMP_FILE);
	do_command(command);

	/* dpstats dumps */
	do_command(command);

	snprintf(subcommand, sizeof(subcommand), "wl -i %s dpstats c:// a:%s m:%s p:%s",
		ifname, eabuf, eabuf, eabuf);
	merge_logs(subcommand, logfile, HC_TEMP_FILE);
}

/* place holder to add further logs in future */
void
capture_dstall_wlc_logs(char *logfile, char *ifname, char *eabuf)
{
	return;
}

/* place holder to add further logs in future */
void
capture_dstall_bsscfg_logs(char *logfile, char *ifname, char *eabuf)
{
	return;
}

void
capture_dstall_scb_bsscfg_logs(char *logfile, char *ifname)
{
	/* update buffer length based on the command length */
	char command[100] = {0};
	char subcommand[100] = {0};

	/* assoclist logs */
	snprintf(command, sizeof(command), "wl -i %s assoclist > %s", ifname, HC_TEMP_FILE);
	snprintf(subcommand, sizeof(subcommand), "wl -i %s assoclist", ifname);
	do_command(command);
	merge_logs(subcommand, logfile, HC_TEMP_FILE);
}

static int
check_file_size(const char *srcfile, const char *tofile)
{
	int src_size = 0, to_size = 0, ret = 0;
	struct stat file_stat;

	ret = stat(srcfile, &file_stat);

	if (!ret && (file_stat.st_size > 0)) {
		if (!tofile) {
			DM_PRINT("log (%s) collected, file size %d bytes\n",
				srcfile, file_stat.st_size);
		}
		src_size = file_stat.st_size;
	} else {
		DM_PRINT("log (%s) is not available or an empty file - ret = %d, err = %s\n",
			srcfile, ret, strerror(errno));
		return FALSE;
	}

	if (tofile) {
		ret = stat(tofile, &file_stat);

		if (!ret && (file_stat.st_size > 0)) {
			to_size = file_stat.st_size;
			DM_PRINT("from %s %d bytes to %s %d bytes\n",
				srcfile, src_size, tofile, to_size);
		} else {
			DM_PRINT("log (%s) is not available or"
				"an empty file - ret = %d, err = %s\n",
				tofile, ret, strerror(errno));
			return FALSE;
		}

		/* compare size */
		if (to_size != src_size) {
			DM_PRINT("%s %d bytes, %s %d bytes - mismatch\n",
				srcfile, src_size, tofile, to_size);
			return FALSE;
		}
	}
	return TRUE;
}

static int
scan_dump_dir(const char *path)
{
	int fcnt = 0;
	struct dirent *entry;
	DIR *dir = NULL;
	struct stat dir_stat;
	char filepath[PATH_LEN] = {0, };

	dir = opendir(path);
	if (dir) {
		while ((entry = readdir(dir)) != NULL) {
			snprintf(filepath, sizeof(filepath), "%s%s", path, entry->d_name);

			if (!stat(filepath, &dir_stat) && S_ISDIR(dir_stat.st_mode)) {
				continue;
			}
			fcnt++;
		}
		closedir(dir);
	}
	return fcnt;
}

/*
* if copying all the files in a directory is done,
* the number of files is checked first and then
* each file's size is checked
*/
static int
compare_files(const char *from, const char *to,
	const char *fname, int ifidx, const char *dsign)
{
	int from_cnt = 0, to_cnt = 0, ret = 0;
	struct dirent *entry = NULL;
	DIR *dir = NULL;
	char srcfilepath[PATH_LEN] = {0, };
	char tofilepath[PATH_LEN] = {0, };
	struct stat dir_stat;

	if (!fname) {
		/* files in /tmp/wlX directory */
		from_cnt = scan_dump_dir(from);
		/* files in the basepath */
		to_cnt = scan_dump_dir(to);

		if (from_cnt != to_cnt) {
			DM_PRINT("The number of copyied file mismatch = %d, %d\n",
				from_cnt, to_cnt);
			return ret;
		}

		DM_PRINT("Collected %d files from %s to %s\n", from_cnt, from, to);

		dir = opendir(from);
		if (dir) {
			while ((entry = readdir(dir)) != NULL) {
				snprintf(srcfilepath, sizeof(srcfilepath), "%s%s",
					from, entry->d_name);
				snprintf(tofilepath, sizeof(tofilepath), "%s%s",
					to, entry->d_name);

				/* skip "." and ".." directories */
				if (!stat(srcfilepath, &dir_stat) &&
					S_ISDIR(dir_stat.st_mode)) {
					continue;
				}

				/* mem_dump_wlX_abcd is copied to dongle_mem_dump_wlX_abcd.bin */
				if (strstr(entry->d_name, "mem_dump")) {
					snprintf(tofilepath, sizeof(tofilepath),
						"%s%s_" IFNAME_STR "_%s",
						to, LOG_DONGLE_MEM_FILENAME, ifidx, dsign);
				}

				ret = check_file_size(srcfilepath, tofilepath);
				if (!ret) {
					break;
				}
			} /* while() loop */
			closedir(dir);
			dir = NULL;
		}
	} else {
		/* check one file */
		snprintf(srcfilepath, sizeof(srcfilepath), "%s%s", from, fname);
		snprintf(tofilepath, sizeof(tofilepath), "%s%s", to, fname);

		ret = check_file_size(srcfilepath, tofilepath);
	} /* if (!fname) */

	return ret;
}

/*
* Check if copy operation is succeeded or not and then retry again
* based on the number of the files and size in src path and dst path
*/
static bool
copy_files(const char *from, const char *to,
	const char *fname, int ifidx, const char *dsign)
{
	int c, fcnt = 0;
	char command[CMD_LEN] = {0, };
	char filepath[PATH_LEN] = {0, };
	DIR *dir = NULL;
	struct dirent *entry = NULL;
	struct stat dir_stat;

	for (c = 0; c < CMD_TRY_CNT; c++) {
		if (!fname) {
			dir = opendir(from);
			if (dir) {
				while ((entry = readdir(dir)) != NULL) {

					snprintf(filepath, sizeof(filepath), "%s%s",
						from, entry->d_name);

					/* to skip '.' and '..' */
					if (!stat(filepath, &dir_stat) &&
						S_ISDIR(dir_stat.st_mode)) {
						continue;
					}

					/* mem_dump is copied to dongle_mem_dump.bin */
					if (strstr(entry->d_name, "mem_dump")) {
						snprintf(command, sizeof(command),
							"cp %s%s %s%s_" IFNAME_STR "_%s",
							from, entry->d_name, to,
							LOG_DONGLE_MEM_FILENAME, ifidx, dsign);
					} else {
						snprintf(command, sizeof(command), "cp %s%s %s%s",
							from, entry->d_name, to, entry->d_name);
					}
					do_command(command);
				} /* while () loop */
				closedir(dir);
				dir = NULL;
				entry = NULL;
			} /* if (dir) */
		} else {
			snprintf(command, sizeof(command), "cp %s%s %s%s",
				from, fname, to, fname);
			do_command(command);
		} /* if (!fname) */

		if (compare_files(from, to, fname, ifidx, dsign)) {
			return TRUE;
		}
		sleep(1);
	} /* for() loop */
	return FALSE;
}

static void
create_otherdumpfiles(const char *path)
{
	char command[CMD_LEN] = {0, };
	char filepath[PATH_LEN] = {0, };
	int c;
	struct stat file_stat;

	/* uname version - uname.txt */
	snprintf(filepath, sizeof(filepath), "%s/%s", path, LOG_UNAME_FILENAME);
	for (c = 0; c < CMD_TRY_CNT; c++) {
		snprintf(command, sizeof(command), "uname -a > %s/%s",
			path, LOG_UNAME_FILENAME);
		do_command(command);
		sleep(1);

		if (check_file_size(filepath, NULL)) {
			break;
		}
	}

	/* nvram dumpi - nvram_dump.txt */
	snprintf(filepath, sizeof(filepath), "%s/%s", path, LOG_NVRAM_DUMP_FILENAME);
	for (c = 0; c < CMD_TRY_CNT; c++) {
		snprintf(command, sizeof(command), "nvram dump > %s/%s",
			path, LOG_NVRAM_DUMP_FILENAME);
		do_command(command);
		sleep(1);

		if (check_file_size(filepath, NULL)) {
			break;
		}
	}

	/* dmesg - dmesg.txt */
	snprintf(filepath, sizeof(filepath), "%s/%s", path, LOG_DMESG_FILENAME);
	for (c = 0; c < CMD_TRY_CNT; c++) {
		snprintf(command, sizeof(command), "dmesg > %s/%s",
			path, LOG_DMESG_FILENAME);
		do_command(command);
		sleep(1);

		if (check_file_size(filepath, NULL)) {
			break;
		}
	}
}

/* Filter only "hc_ts.bin" files */
static int
filter_hc_dbg_file(const struct dirent *dptr)
{
	if (dptr && dptr->d_name) {
		if (strstr(dptr->d_name, HC_DBG_DATA_PREFIX) && strstr(dptr->d_name, ".bin")) {
			return TRUE;
		}
	}
	return FALSE;
}

/* Copy the health check debug data file (hc_timestamp.bin) for the health check event */
void
hc_cpy_dbg_logfile(hc_sig_info_t *hc_sig_info, char *basepath, int ifidx)
{
	char command[CMD_LEN] = {0, };
	char filepath[256];
	struct dirent **fnamelist = NULL;
	int i, n;

	/* Create the folder wlx */
	snprintf(filepath, sizeof(filepath), "%s/wl%u", basepath, ifidx);
	if (mkdir(filepath, 0777) < 0 && errno != EEXIST) {
		perror("could not create hc logbase/wlx folder");
		return;
	}
	/* Copy the hc_ts.bin file for non data stall health checks */
	if (hc_sig_info->n_dstall_reports == 0) {
		/* Scan the dir for hc_tx.bin files */
		snprintf(filepath, sizeof(filepath), "/tmp/hc/if%u", ifidx);
		n = scandir(filepath, &fnamelist, filter_hc_dbg_file, alphasort);
		if (n > 0) {
			/* Take the first(oldest file) for process */
			snprintf(command, sizeof(command), "cp -f /tmp/hc/if%u/%s %s/wl%u/",
				ifidx, fnamelist[0]->d_name, basepath, ifidx);
			do_command(command);

			snprintf(command, sizeof(command), "rm -f /tmp/hc/if%u/%s",
				ifidx, fnamelist[0]->d_name);
			do_command(command);

			for (i = 0; i < n; i++) {
				/* free each entry allocated by scandir() */
				free(fnamelist[i]);
			}

			free(fnamelist);
		}
	}
	return;
}

static void
extract_dump_signature(char *path, char *sign)
{
	struct dirent *entry;
	DIR *dir = NULL;
	char temp[FILE_LEN] = {0, };
	char *tmp = NULL, *tmp1 = NULL, *tmp2 = NULL;

	dir = opendir(path);
	if (!dir) {
		DM_PRINT("Failed to extract dump sign\n");
		return;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (strstr(entry->d_name, ".txt") || strstr(entry->d_name, ".bin")) {
			strncpy(temp, entry->d_name, FILE_LEN - 1);
			/* filter .txt or .bin */
			tmp = strtok(temp, ".");
			/* tokenize "_" because it's like dump_macx_5c02.txt */
			tmp1 = strtok(tmp, "_");

			while (tmp1 != NULL) {
				tmp2 = tmp1;
				tmp1 = strtok(NULL, "_");
			}

			strncpy(sign, tmp2, DUMP_SIGN_LEN);
			break;
		}
	}
	closedir(dir);
}

static void
tarball_name_formatting(dm_sig_info_t *siginfo, hc_sig_info_t *hc_sig_info)
{
	char crash_log_prefix[CRASH_LOG_FOLDERNAME_SZ] = CRASH_LOG_PREFIX;
	int nic_wl = (siginfo->signo == SIGUSR1);
	/*
	* NIC : SIGUSR1
	* DHD : SIGUSR2
	* MLO : SIGUSR3
	* HC(health-check) : SIGUSR4 (HEALTH_CHECK_SIG == SIGURG)
	*/
	if (siginfo->signo != SIGUSR4) {

		if (siginfo->check_init) {
			DM_PRINT("Create a tgz with dumps on interface wl%d, dump signature %s\n",
				siginfo->ifidx, siginfo->dump_signature);

			snprintf(siginfo->filename, sizeof(siginfo->filename), "%s%s_%s_"IFNAME_STR,
				crash_log_prefix, siginfo->timestamp, siginfo->dump_signature,
				siginfo->ifidx);
		} else {
			DM_PRINT("Handle signal(%s, %d) on interface wl%d, dump signature %s, "
				"nic_wl %d\n", siginfo->drv_type, siginfo->signo,
				siginfo->ifidx, siginfo->dump_signature, nic_wl);

			/*
			* Format - debug-timestamp_sign_wlX_type
			* -  debug-2022_08_01_03_46_57_abcd_wl1_DHD
			*/
			snprintf(siginfo->filename, sizeof(siginfo->filename),
				"%s%s_%s_"IFNAME_STR"_%s", crash_log_prefix,
				siginfo->timestamp, siginfo->dump_signature,
				siginfo->ifidx, siginfo->drv_type);
		}
	} else {
		/*
		* HC signal can just report a helath-check report.
		* So there is no dump signature.
		*/
		DM_PRINT("Handle signal(%s, %d) on interface wl%d, nic_wl %d\n",
			siginfo->drv_type, siginfo->signo, siginfo->ifidx, nic_wl);

		/* Format : debug-2022_08_01_03_46_57_wl1_HC */
		snprintf(siginfo->filename, sizeof(siginfo->filename), "%s%s_"IFNAME_STR"_%s",
			crash_log_prefix, siginfo->timestamp, siginfo->ifidx,
			siginfo->drv_type);
	} /* if(siginfo->signo != SIGUSR4) */

	/*
	* In case of MLO
	* - debug-2022_08_01_03_46_57_abcd_wl1_MLO_MAP
	* - debug-2022_08_01_03_46_57_abcd_wl1_MLO_AAP1
	*/
	if (siginfo->mlo_sig) {
		if (nvram_mlo[siginfo->ifidx] >= 0) {
			/* in case of MAP */
			if (!nvram_mlo[siginfo->ifidx]) {
				snprintf(siginfo->filename, sizeof(siginfo->filename),
					"%s%s_%s_"IFNAME_STR"_%s_MAP", crash_log_prefix,
					siginfo->timestamp, siginfo->dump_signature,
					siginfo->ifidx,	siginfo->drv_type);
			} else {
			/* in case of AAP */
				snprintf(siginfo->filename, sizeof(siginfo->filename),
					"%s%s_%s_"IFNAME_STR"_%s_AAP%d", crash_log_prefix,
					siginfo->timestamp, siginfo->dump_signature,
					siginfo->ifidx, siginfo->drv_type,
					nvram_mlo[siginfo->ifidx]);
			}
		} /* if(nvram_mlo[.... */
	} /* if (siginfo->signo == SIGUSR3) */

	DM_PRINT("%s.tgz creation start\n", siginfo->filename);
}

static int
capture_logs(dm_sig_info_t *siginfo, hc_sig_info_t *hc_sig_info)
{
	/*
	* NIC : SIGUSR1
	* DHD : SIGUSR2
	* MLO : SIGUSR3
	* HC(health-check) : SIGUSR4 (HEALTH_CHECK_SIG == SIGURG)
	*/

	char basepath[FILE_LEN] = {0, };
	char filepath[PATH_LEN] = {0, };
	char command[CMD_LEN] = {0, };
	char srcpath[PATH_LEN] = {0, };
	char topath[PATH_LEN] = {0, };
	struct stat file_stat;
	int size = 0;
	int nic_wl = (siginfo->signo == SIGUSR1);
	int ifidx = siginfo->ifidx;
	bool incomplete = FALSE;

	tarball_name_formatting(siginfo, hc_sig_info);

	/*
	* basepath has a whole path
	* like /tmp/crash_logs/debug-2022_08_01_03_46_57_abcd_wl1_MLO_MAP
	*/
	snprintf(basepath, sizeof(basepath), "%s/%s", LOG_BASE_PATH, siginfo->filename);

	if (mkdir(LOG_BASE_PATH, 0777) < 0 && errno != EEXIST) {
		DM_PRINT("Could not create %s folder\n", LOG_BASE_PATH);
		return BCME_ERROR;
	}

	if (mkdir(basepath, 0777) < 0 && errno != EEXIST) {
		DM_PRINT("Could not create %s folder\n", basepath);
		return BCME_ERROR;
	}

	/* Copy the hc folder structure(+rename ifx =>wlx) and client's event debug data files */
	if (hc_sig_info &&
		hc_sig_info->hc_sig) {

		hc_cpy_dbg_logfile(hc_sig_info, basepath, ifidx);

		/* Take the logs needed for data stall debug, copy the logs into basepath */
		if (hc_sig_info->n_dstall_reports) {
			handle_hc_dstall(siginfo->timestamp, nic_wl, ifidx,
				hc_sig_info, basepath);
			goto exit;
		}
	}

	/* Print dump file list */
	snprintf(command, sizeof(command),
		"ls "DUMP_BASE_PATH "/" IFNAME_STR "/", ifidx);
	do_command(command);

	/*
	* all dumps are created by drivers
	* and dump files are in /tmp/wlX directory
	* and then copy the files into /tmp/crash_logs directory.
	* if a tgz file is successfully created,
	* it is copied in the backup dir
	*/

	/* basepath - /tmp/crash_logs/debug-timestamp_wlX */
	snprintf(topath, sizeof(topath), "%s/", basepath);
	/* drivers will store dumps in /tmp/wlX */
	snprintf(srcpath, sizeof(srcpath), DUMP_BASE_PATH "/" IFNAME_STR "/", ifidx);

	if (!siginfo->dump_cmpl) {
		incomplete = TRUE;
		DM_PRINT("Driver didn't signal dump completion - copying dumps created so far\n");
	}

	if (!copy_files(srcpath, topath, NULL, ifidx, siginfo->dump_signature)) {
		incomplete = TRUE;
		DM_PRINT("Storing some dumps in %s failed\n", topath);
	}

	create_otherdumpfiles(basepath);
exit:
	/* assure file handles are idle before creating tar */
	sync();

	/* tar/zip the logs and memory dump */
	snprintf(command, sizeof(command), "tar cf %s.tar %s -C %s", basepath,
		siginfo->filename, LOG_BASE_PATH);
	if (do_command(command) < 0) {
		DM_PRINT("%s error to create a tar\n", command);
		return BCME_ERROR;
	}

	snprintf(filepath, sizeof(filepath), "%s.tar", basepath);
	size = check_file_size(filepath, NULL);
	if (!size) {
		DM_PRINT("%s size is %d error\n", filepath, size);
		snprintf(command, sizeof(command), "rm -rf %s", filepath);
		do_command(command);
		return BCME_ERROR;
	}

	/*
	* If fast_restart is set, dumps are still in /tmp/wlX.
	* After recovery, the previous dumps can be included into another tgz file.
	* These will be still in there until reboot is triggered.
	* Not to mix up dumps, dumps are needed to be removed.
	* If dumps are in misc partition or on a ramdisk,
	* these will be there in case of deleting these.
	*/
	if (is_fast_restart() ||
		is_driver_dump_path_to_misc() ||
		is_ramfs_mounted()) {
		/* delete the dump folder /tmp/wlX */
		snprintf(command, sizeof(command), "rm -f " DUMP_BASE_PATH
			"/" IFNAME_STR "/*", ifidx);
		do_command(command);
	}

	/*
	 * delete the raw folder
	 * like /tmp/crash_logs/debug-2022_08_01_03_46_57_abcd_wl1_MLO_MAP
	 * a tarball was successfully created.
	 */
	snprintf(command, sizeof(command), "rm -rf %s", basepath);
	do_command(command);

	snprintf(command, sizeof(command), "gzip -c %s.tar > %s.tgz", basepath,
		basepath);
	if (do_command(command) < 0) {
		DM_PRINT("%s error to create a tgz\n", command);
		return BCME_ERROR;
	}

	/* create a tgz file from a tar */
	snprintf(filepath, sizeof(filepath), "%s.tgz", basepath);
	size = check_file_size(filepath, NULL);
	/* return value is the size of the tgz file */
	if (!size) {
		DM_PRINT("%s size is %d error\n", filepath, size);
		snprintf(command, sizeof(command), "rm -rf %s", filepath);
			do_command(command);
		return BCME_ERROR;
	}

	if (incomplete) {
		DM_PRINT("Tarball (%s.tgz) content may be incomplete\n", basepath);
	}

	/* delete the .tar file after compression */
	snprintf(command, sizeof(command), "rm -rf %s.tar", basepath);
	do_command(command);

#if SOCRAM_DUMP_ON_PROLONG_STALL
	if (access(HC_SOCRAM_FILE, F_OK) == 0) {
		/* delete socram_dump.txt file */
		snprintf(command, sizeof(command), "rm -rf %s", HC_SOCRAM_FILE);
		do_command(command);
	}
#endif
	return size;
}

static void
collect_logs_by_if(dm_sig_info_t *siginfo, hc_sig_info_t *hc_sig_info)
{
	char basepath[FILE_LEN] = {0, };
	int log_size, retain_logs;
	/* some customers use big stroage so interger cannot be enough sometimes */
	unsigned long free_space = 0;
	struct stat dir_stat;

	DM_PRINT("Logging timestamp: %s\n", siginfo->timestamp);

	/* capture_logs() checks dump files and creates a tgz file */
	log_size = capture_logs(siginfo, hc_sig_info);

	if (log_size < 0) {
		DM_PRINT("Capture failed for wl%d\n", siginfo->ifidx);
		return;
	} else if (hc_sig_info && (hc_sig_info->hc_log_only)) {
		if (log_size > SIZE_SMALL_HC_LOGS) {
			DM_PRINT("Log size:%u, is > %u Bytes, Not able to restore!!\n",
				log_size, SIZE_SMALL_HC_LOGS);
			return;
		} else {
			/* Limit the no of files in the hc folder */
			snprintf(basepath, sizeof(basepath), "%s/%s", _backup_dir,
				LOG_HC_FOLDER);
			delete_extra_crash_files(basepath, (MAX_HC_LOGS - 1), NULL);

			/* Small logs, just proceed to save */
			goto backup;
		}
	}

	/* Make space the the crash log dumps */
	for (retain_logs = (max_crash_logs - 1); retain_logs >= 0; retain_logs--) {
		DM_PRINT("going to retain %d log(s) under backup dir (%s)\n", retain_logs,
			_backup_dir);

		if (siginfo->mlo_sig) {
			delete_extra_crash_files(_backup_dir, retain_logs, siginfo->dump_signature);
		} else {
			delete_extra_crash_files(_backup_dir, retain_logs, NULL);
		}

		if (retain_logs) {
			free_space = get_dir_free_space(_backup_dir);
			if (free_space < 0) {
				/* failed to get disk space of backup dir, skip space check */
				break;
			}

			if (log_size > free_space) {
				DM_PRINT("Disk space (%lu bytes) not enough for "
					"new log (%d bytes), retain one less log\n",
					free_space, log_size);
			} else {
				/* free disk space is large enough for the new log */
				break;
			}
		} /* if (retain_logs) */
	} /* for() loop */

backup:
	/* back up the logs to persistent store */
	backup_logs(siginfo, _backup_dir, hc_sig_info);
}

static int
mlo_read_nvram(void)
{
	char *wl_mlo_config_str;
	char nvram_str[64];

	snprintf(nvram_str, sizeof(nvram_str), "wl_mlo_config");

	if ((!(wl_mlo_config_str = nvram_safe_kget(nvram_str)) ||
		!strcmp(wl_mlo_config_str, ""))) {
		wl_mlo_config_str = WL_MLO_DEFAULT;
	}

	if (sscanf(wl_mlo_config_str, "%d %d %d %d",
		&nvram_mlo[0], &nvram_mlo[1], &nvram_mlo[2], &nvram_mlo[3]) !=
		(PCIE_IPC_AP_UNIT_MAX))
	{
		DM_PRINT("%s: ERROR wl_mlo_config_str[%s]", wl_mlo_config_str);
		return -1;
	}

	return 0;
}

/* For mlo, check if all the siganls are catched or not */
static bool
is_all_mlo_if(void)
{
	int unit;
	int ret = TRUE;

	if (!dm_sig_status.wait_for_mlo) {
		return ret;
	}

	/*
	* Check all the corresponding MLO AP logs are collected.
	* Note : In case of MLO,
	* the number of participating radios can be found by checking nvram wl_mlo_config.
	* eg. wl_mlo_config value of "0 1 -1 -1" implies wl0 and wl1 are participating in MLO.
	* It is positional. First entry is always about wl0, second about wl1 and so on.
	* Value of '-1' implies that the interface is not in any MLO
	*/
	for (unit = 0; unit < PCIE_IPC_AP_UNIT_MAX; unit++) {
		DM_PRINT("nvram_mlo[%d] = %d, if_check[%d] = %d\n",
			unit, nvram_mlo[unit], unit, dm_sig_status.if_check[unit]);

		if ((nvram_mlo[unit] >= 0) && (dm_sig_status.if_check[unit] != 1)) {
			ret = FALSE;
			break;
		}
	}

	return ret;
}

/* The below data format should be in sync with the driver */
/* Health Check report structure for Tx packet failure check */
typedef struct {
	char  octet[6];
} ether_addr1_t;

wl_dstall_hc_info_extended_t hc_info;

/** Utility function to convert module type to string */
char *stallmodule2str(char stall_module)
{
	switch (stall_module)
	{
		case WLC_DSTALL_MODULE_GLOBAL:
			return "WLC";
		case WLC_DSTALL_MODULE_BSSCFG:
			return "BSSCFG";
		case WLC_DSTALL_MODULE_SCB:
			return "SCB";
		default:
			return "NONE";
	}
}

/** Utility function to covert stall type to string */
char *stalltype2str(char stall_type)
{
	switch (stall_type)
	{
		case 1:
			return "ONCE"; /* Reference: WLC_DSTALL_ONCE(wlioctl.h) */
		case 2:
			return "PROLONG"; /* Reference: WLC_DSTALL_PROLONG(wlioctl.h) */
		default:
			return "StallTypeErr";
	}
}

/* The function takes action on the data stall signal+data.
 * For a 1 sec stall - take the needed logs and store in persistant memory.
 * For continuous N sec stall - Reset the module.
 */

static void
handle_hc_dstall(char *timestamp, int nic_wl, int ifidx, hc_sig_info_t *hc_sig_info, char *basepath)
{
	uint idx = 0, skip = 0;
	char eabuf[ETHER_ADDR_STR_LEN] = {0};
	wl_dstall_hc_log_extended_t *hc_log;
	char ifname[IFNAMSIZ] = {0};
	char command[] = {0};
	char logfile[100] = {0};
	uint8 generic_logs_taken = 0;
	uint8 take_bss_logs = 0;
	char curr_ifname[IFNAMSIZ] = "randomifname";
	char is_socramdump_taken = 0;
	uint nitems;
	char stall_type;

	nitems = hc_sig_info->n_dstall_reports;
	stall_type = (hc_sig_info->dstall_dir == WL_HC_DD_RX_STALL) ? 'R': 'T';

	/* A file to merge all logs */
	snprintf(logfile, sizeof(logfile), "/tmp/hc/if%d/dstall/%cxStall_dumps_%s.txt",
		ifidx, stall_type, timestamp);

	/* update confluence link, delimiter and type of log info to main file */
	snprintf(command, sizeof(command), "echo "CONFLUENCE_LINK_DSTALL" > %s",
		stall_type, HC_TEMP_FILE);
	do_command(command);
	merge_logs(NULL, logfile, HC_TEMP_FILE);

	for (idx = 0; (idx < nitems) && (!skip); idx++) {
		hc_log = &(hc_info.dstall_log_ext[idx]);
		strncpy(ifname, hc_log->ifname, IFNAMSIZ);
		ifname[IFNAMSIZ-1] = '\0';

		if (strcmp(curr_ifname, ifname) != 0)
			take_bss_logs = 1;

		snprintf(eabuf, sizeof(eabuf), "%02x:%02x:%02x:%02x:%02x:%02x",
			(char)hc_log->ethaddr.octet[0]&0xff,
			(char)hc_log->ethaddr.octet[1]&0xff,
			(char)hc_log->ethaddr.octet[2]&0xff,
			(char)hc_log->ethaddr.octet[3]&0xff,
			(char)hc_log->ethaddr.octet[4]&0xff,
			(char)hc_log->ethaddr.octet[5]&0xff);

		/* update txstall log info to main file */
		snprintf(command, sizeof(command), "echo "DSTALL_HEADER" > %s",
			stall_type, stallmodule2str(hc_log->stall_module), eabuf,
			HC_TEMP_FILE);
		do_command(command);
		merge_logs(NULL, logfile, HC_TEMP_FILE);

		/* Capture generic logs */
		if (!generic_logs_taken) {
			capture_dstall_generic_logs(logfile, ifname);
			generic_logs_taken = 1;
		}

		DM_PRINT("%cX stall, mod:%s, stall_Type:%s, eth:%s\n", stall_type,
			stallmodule2str(hc_log->stall_module),
			stalltype2str(hc_log->stall_type), eabuf);

		if (hc_log->stall_type == WLC_DSTALL_ONCE) {
			switch (hc_log->stall_module) {
				case WLC_DSTALL_MODULE_GLOBAL:
					capture_dstall_wlc_logs(logfile, ifname, eabuf);
					break;
				case WLC_DSTALL_MODULE_BSSCFG:
					capture_dstall_bsscfg_logs(logfile, ifname, eabuf);
					break;
				case WLC_DSTALL_MODULE_SCB:
					capture_dstall_scb_logs(logfile, ifname, eabuf);
					if (take_bss_logs) {
						capture_dstall_scb_bsscfg_logs(logfile,
							ifname);
						strncpy(curr_ifname, ifname, IFNAMSIZ);
						curr_ifname[IFNAMSIZ-1] = '\0';
						take_bss_logs = 0;
					}
					break;
				default:
					break;
			}
		}
	}
done:
	/* Copy the data stall file in the final log folder(.tgz folder) */
	snprintf(command, sizeof(command), "cp %s %s/wl%u/", logfile, basepath, ifidx);
	do_command(command);

	/* Delete the data stall log file after copying to backup dir */
	snprintf(command, sizeof(command), "rm -f %s", logfile);
	do_command(command);

	return;
}

/* Read the report and full in the structure, assume little endian */
uint
read_hc_dstall_report(FILE *fp, uint *dstall_dir)
{
	char ch = 0;
	uint nitems_read = 0, nitems_expected = 0;

	/* Read the type of hc report */
	ch = fgetc(fp);
	if (ch == EOF) {
		return BCME_ERROR;
	}
	hc_info.type = ch;

	ch = fgetc(fp);
	if (ch == EOF) {
		return BCME_ERROR;
	}
	hc_info.type |= (ch << 8);

	/* Read the length of hc data stall log */
	ch = fgetc(fp);
	if (ch == EOF) {
		return BCME_ERROR;
	}

	hc_info.length = ch;

	ch = fgetc(fp);
	if (ch == EOF) {
		return BCME_ERROR;
	}

	hc_info.length |= (ch << 8);

	DM_PRINT("%s, type: %cxStall, len:%d\n", __FUNCTION__,
		(hc_info.type == WL_HC_DD_RX_STALL) ? 'R' : 'T', hc_info.length);
	if (hc_info.length % sizeof(wl_dstall_hc_log_extended_t)) {
		DM_PRINT("%s,check if HC data stall structs are upto date\n", __FUNCTION__);
		return BCME_OK;
	}
	*dstall_dir = hc_info.type;

	nitems_expected = hc_info.length/sizeof(wl_dstall_hc_log_extended_t);

	nitems_read = fread(&hc_info.dstall_log_ext[0], sizeof(wl_dstall_hc_log_extended_t),
		nitems_expected, fp);

	if (nitems_read != nitems_expected) {
		DM_PRINT("%s, Could only read %d items(size:%d) out of %d \n", __FUNCTION__,
			nitems_read, sizeof(wl_dstall_hc_log_extended_t), nitems_expected);
	}

	return nitems_read;
}

/* Filter only "hc_dstall_ts.txt" files */
static int
filter_hc_dstall_file(const struct dirent *dptr)
{
	if (dptr && dptr->d_name) {
		if (strstr(dptr->d_name, DSTALL_LOG_PREFIX) && strstr(dptr->d_name, ".txt")) {
			return TRUE;
		}
	}

	return FALSE;
}

uint
fetch_hc_dstall_report(int ifidx, hc_sig_info_t *hc_sig_info)
{
	char filepath[256], command[256];
	struct dirent **fnamelist = NULL;
	FILE *fp = NULL;
	int n = 0, i = 0;

	snprintf(filepath, sizeof(filepath), "/tmp/hc/if%u/dstall", ifidx);
	DM_PRINT("ls %s\n", filepath);

	snprintf(command, sizeof(command), "ls -l /tmp/hc/if%u/dstall", ifidx);
	do_command(command);

	/* Check if dstall datafile exists */
	n = scandir(filepath, &fnamelist, filter_hc_dstall_file, alphasort);
	if (n <= 0) {
		hc_sig_info->n_dstall_reports = 0;
		return FALSE;
	} else {
		/* Take the first(oldest file) for process */
		snprintf(filepath, sizeof(filepath), "/tmp/hc/if%u/dstall/%s",
			ifidx, fnamelist[0]->d_name);
		DM_PRINT("Data stall file in process: %s\n", filepath);

		for (i = 0; i < n; i++) {
			/* free each entry allocated by scandir() */
			free(fnamelist[i]);
		}

		free(fnamelist);
	}

	fp = fopen(filepath, "r");
	if (fp) {
		hc_sig_info->n_dstall_reports =
			read_hc_dstall_report(fp, &(hc_sig_info->dstall_dir));
		fclose(fp);
		snprintf(command, sizeof(command), "rm -f %s", filepath);
		do_command(command);
		return TRUE;
	} else {
		DM_PRINT("File doesn't exist %s\n", filepath);
		hc_sig_info->n_dstall_reports = 0;
		return FALSE;
	}
}

static void
check_recovery(dm_sig_info_t *sinfo, hc_sig_info_t *hc_sig_info)
{
	if (hc_sig_info && hc_sig_info->is_dstall) {
		sinfo->is_recovery = 0;
	}

	if (sinfo->mlo_sig) {
		dm_sig_status.wait_for_mlo = 1;
	}

	if (sinfo->is_recovery) {
		dm_sig_status.recovery_cnt++;
	}
}

static int
gen_tarball(void)
{
	dm_sig_info_t *siginfo = &(dm_sig_status.cur_sig);
	hc_sig_info_t hc_sig_info = {0};
	char filepath[PATH_LEN] = {0, };
	char timestamp[TIMESTAMP_LEN] = {0, };

	/*
	* signal type
	* NIC : SIGUSR1
	* DHD : SIGUSR2
	* MLO : SIGUSR3
	* health-check : SIGUSR4 (HEALTH_CHECK_SIG == SIGURG)
	*/

	time_formatting(&siginfo->rawtime, timestamp);
	strncpy(siginfo->timestamp, timestamp, TIMESTAMP_LEN - 1);

	if (siginfo->signo == SIGUSR4) {
		DM_PRINT("HEALTH CHECK signal from "IFNAME_STR"!\n", siginfo->ifidx);
		hc_sig_info.hc_sig = 1;
	} else {
	/* to prevent a signal that has something worng from not being handled */
		snprintf(filepath, sizeof(filepath),
			DUMP_BASE_PATH "/" IFNAME_STR "/", siginfo->ifidx);

		extract_dump_signature(filepath, siginfo->dump_signature);

		if (strlen(siginfo->timestamp) &&
			strlen(siginfo->drv_type) &&
			strlen(siginfo->dump_signature)) {

			DM_PRINT("Detected firmware trap/assert from "IFNAME_STR"!\n",
				siginfo->ifidx);

		} else {
			DM_PRINT("Skip a wrong signal - "
				"signo %d, ifidx %d, timestamp %s, drv_type %s, dump sig %s\n",
				siginfo->signo, siginfo->ifidx, siginfo->timestamp,
				siginfo->drv_type, siginfo->dump_signature);

			return FALSE;
		}
	}

	if (siginfo->mlo_sig) {
		dm_sig_status.if_check[siginfo->ifidx] = 1;
	}

	if (siginfo->signo == SIGUSR4) {
		hc_sig_info.is_dstall = fetch_hc_dstall_report(siginfo->ifidx,
			&hc_sig_info);
		/* Take only debug dumps for data stall */
		if (hc_sig_info.is_dstall) {
			hc_sig_info.hc_log_only = 1;
		}
	}

	collect_logs_by_if(siginfo, &hc_sig_info);
	check_recovery(siginfo, &hc_sig_info);

	sync();

	return TRUE;
}

static bool
is_fast_restart(void)
{
	char *val = (char *) nvram_get("fast_restart");

	if (val && atoi(val)) {
		return TRUE;
	}

	return FALSE;
}

static bool
is_watchdog_enabled(void)
{
	/* Full restart - reboot - default is watchdog=4000 */
	char *val = (char *) nvram_get("watchdog");

#if defined(CMWIFI)
	/*
	* default reboot system if watchdog nvram is not defined.
	* take action based on watchdog config value if it is available
	*/
	if ((!val) || atoi(val)) {
#else
	if (val && atoi(val)) {
#endif /* CMWIFI */
		DM_PRINT("Rebooting system watchdog=%d \n", val ? atoi(val) : 0);
		return TRUE;
	}
	return FALSE;
}

static bool
is_all_signals_done(void)
{
	int i;
	dm_sig_info_t temp;

	if (!dm_sig_status.overwrap) {
		if (dm_sig_status.tot_processed == dm_sig_status.tot_received) {
			DM_PRINT("All signals complete %d, %d\n",
				dm_sig_status.tot_received, dm_sig_status.tot_processed);
			return TRUE;
		} else {
			return FALSE;
		}
	} else {
		for (i = 0; i < DM_QUEUE_MAX; i++) {
			temp = dm_sig_status.sig_queue[i];
			if (temp.status != PROCESSED_DONE) {
				return FALSE;
			}
		}
	}

	DM_PRINT("All signals are done in queue %d, %d\n",
		dm_sig_status.tot_received, dm_sig_status.tot_processed);

	return TRUE;
}

static bool
is_mem_dump_exist(int ifidx)
{
	struct dirent *entry;
	DIR *dir = NULL;
	char path[PATH_LEN] = {0, };
	bool ret = FALSE;

	snprintf(path, sizeof(path), DUMP_BASE_PATH "/" IFNAME_STR, ifidx);

	dir = opendir(path);
	if (!dir) {
		DM_PRINT("Failed to opendir %s\n", path);
		return ret;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (strstr(entry->d_name, "mem_dump")) {
			ret = TRUE;
			break;
		}
	}
	closedir(dir);
	return ret;
}

static bool
is_dump_exist(int ifidx)
{
	char path[PATH_LEN] = {0, };
	int ret = FALSE;
	snprintf(path, sizeof(path), DUMP_BASE_PATH "/" IFNAME_STR, ifidx);

	if (check_dump_dir(path)) {
		ret = TRUE;
	}

	DM_PRINT("Dump %s exist in %s\n", ret ? "" : "not", path);
	return ret;
}

static void
check_missing_mlo_signals(void)
{
	int unit;
	siginfo_t sinfo;
	char ifname[IFNAME_LEN] = {0, };
	char buf[512] = {0, };
	char command[CMD_LEN] = {0, };

	if (dm_sig_status.wait_for_mlo) {
		for (unit = 0; unit < PCIE_IPC_AP_UNIT_MAX; unit++) {
			if ((nvram_mlo[unit] >= 0) &&
				(dm_sig_status.if_check[unit] != 1)) {
				sinfo.si_int = (unit | DUMP_COMPLETION);

				/* if dumps are in /tmp/wlX */
				if (is_dump_exist(unit)) {
					DM_PRINT("retrigger tgz creation for wl%d\n", unit);
					if (is_mem_dump_exist(unit)) {
						/* SIGUSR3 is mlo signal */
						sig_handler(SIGUSR3, &sinfo, NULL);
					} else {
						/* assume the interface is NIC MLO */
						sinfo.si_int |= NIC_MLO_SIG_FLAG;
						sig_handler(SIGUSR1, &sinfo, NULL);
					}
				} else {
					/* just trigger dumps */
					DM_PRINT("just trigger dumps for wl%d\n", unit);
					snprintf(ifname, sizeof(ifname), IFNAME_STR, unit);
					if (wl_iovar_get(ifname, "ver", (void *)buf, sizeof(buf))) {
						DM_PRINT("no response from %s\n", ifname);
						dm_sig_status.if_check[unit] = 1;
					} else {
						/* The version string has FWID- if it is FD */
						if (strstr(buf, "FWID-")) {
							/* force to trigger trap */
							DM_PRINT("try to force a trap for %s\n",
								ifname);
							if (wl_iovar_setint(ifname,
								"bus:disconnect", 99)) {
								DM_PRINT("failed trap for %s\n",
									ifname);
								dm_sig_status.if_check[unit] = 1;
							}
						} else {
							/* force psmwd for NIC */
							DM_PRINT("try to force psmwd for %s\n",
								ifname);
							if (wl_iovar_setint(ifname,
								"psmwd_after", 1)) {
								DM_PRINT("failed psmwd for %s\n",
									ifname);
								dm_sig_status.if_check[unit] = 1;
							}
						}
					} /* if (wl_iovar_get() */
				} /* if (is_dump_exist() */
			} /* if ((nvram_mlo[ */
		} /* for(unit */
	} /* if(dm_sig_status */
}

static void
sig_handler_watchdog(void)
{
	char *val = NULL;

	/* No signal received */
	if (!dm_sig_status.tot_received) {
		return;
	}

#ifdef DM_NO_NVRAM_RESTART
	if (is_sig_inprogress) {
		val = nvram_get("debug_monitor_proc_status");
		if (!val ||
			!(val && !strcmp(val, "1"))) {
			nvram_set("debug_monitor_proc_status", "1");
		}
	}
#endif /* DM_NO_NVRAM_RESTART */

	/* All the sigals have been processed, will do recovery */
	if (is_all_signals_done()) {
		/*
		* check mlo interfaces
		* don't trigger recovery
		* until all the signals from mlo interfaces are received.
		*/
		if (!is_all_mlo_if()) {
			if (dm_sig_status.wait_for_mlo_timeout > WAIT_FOR_MLO_TIMEOUT) {
				/* just trigger a tarball */
				check_missing_mlo_signals();
			}
			dm_sig_status.wait_for_mlo_timeout++;
			return;
		}

		/* Clear dmesg */
		do_command("dmesg -c > /dev/null");

#ifdef CRASH_HISTORY_FILE_LOGGING
		close_history_files();
#endif /* CRASH_HISTORY_FILE_LOGGING */

		sync();

		/*
		* in case traps happen mutilple interface across multiple interfaces,
		* just reboot instead turning off/on each interface.
		*/
		if (is_sig_inprogress) {
			is_sig_inprogress = FALSE;
#ifdef DM_NO_NVRAM_RESTART
			nvram_unset("debug_monitor_proc_status");
#endif /* DM_NO_NVRAM_RESTART */
			/* To save the last dump signature */
			nvram_commit();
		}

		if ((dm_sig_status.recovery_cnt > 1) ||
			dm_sig_status.wait_for_mlo) {
			handle_recovery(0, 0, TRUE);
		} else {
			dm_sig_info_t temp;

			for (int i = 0; i < DM_QUEUE_MAX; i++) {
				temp = dm_sig_status.sig_queue[i];

				if ((temp.status == PROCESSED_DONE) &&
					temp.is_recovery) {
					handle_recovery((temp.signo == SIGUSR1),
						temp.ifidx, FALSE);
				}
			}
		} /* if (dm_sig_status.recovery_cnt > 1) */

		memset(&dm_sig_status, 0, sizeof(dm_sig_status));
	} else {
		int cidx, ifidx, signo;

		/* If cur_idx is exceeded, reset to 0 */
		if (dm_sig_status.cur_idx >= DM_QUEUE_MAX) {
			dm_sig_status.cur_idx = 0;
		}

		cidx = dm_sig_status.cur_idx;
		ifidx = dm_sig_status.cur_sig.ifidx;
		signo = dm_sig_status.cur_sig.signo;

		if (dm_sig_status.cur_sig.status == NOTYETPROCESS) {
			/* Wait for dump completion */
			if (!dm_sig_status.cur_sig.dump_cmpl) {

			/*
			* is_mem_dump_exist() is a war fix to resolve a KP issue.
			* The KP issue came from  createing mem_dump.
			* Only with the dump completion timeout,
			* the KP sometimes can happen.
			* In mem_dump creation is still in progress due to host system load,
			* if this timeout is expired first,
			* and debug_monitor trie to recover from the trap,
			* the KP can happen from accessing pcie interface.
			* So first check if mem_dump exists
			* and then wait for dump completion siganls.
			*/
				if (signo == SIGUSR2 || signo == SIGUSR3) {
				/*
				* mem_dump will be created by dhd driver.
				* Also in csae of MLO, it is running FD MODE.
				* Checking mem_dump is only available
				* for MLO and DHD signal.
				*
				* DHD : SIGUSR2
				* MLO : SIGUSR3
				*/
					if (!is_mem_dump_exist(ifidx)) {
						return;
					}
				}

				if (dm_sig_status.cur_time_cnt < PENDING_CNT) {
					dm_sig_status.cur_time_cnt++;
					return;
				}
			} /* if (!dm_sig_status.sig_queue[cidx].dump_cmpl) */

			dm_sig_status.sig_queue[cidx].status = INPROGRESS;
			dm_sig_status.cur_sig.status = INPROGRESS;

			/*
			* if some items of the signal information have problems,
			* tgz file creation is unnecessary.
			*/

			if (gen_tarball()) {
#ifdef CRASH_HISTORY_FILE_LOGGING
				write_tgz_gen_history(NULL);
#endif /* CRASH_HISTORY_FILE_LOGGING */

				dm_sig_status.sig_queue[cidx].status = PROCESSED_DONE;
				dm_sig_status.cur_sig.status = PROCESSED_DONE;
				dm_sig_status.sig_queue[cidx].is_recovery =
					dm_sig_status.cur_sig.is_recovery;

				dm_sig_status.cur_time_cnt = 0;
				dm_sig_status.cur_idx++;
				dm_sig_status.cur_sig =
					dm_sig_status.sig_queue[dm_sig_status.cur_idx];

				dm_sig_status.tot_processed++;
			} else {
			/* if the signal is wrong, don't count it as received signal */
				dm_sig_status.sig_queue[cidx].status = NOTYETPROCESS;
				dm_sig_status.cur_sig.status = NOTYETPROCESS;
				dm_sig_status.cur_time_cnt = 0;
				dm_sig_status.tot_received--;
			}
		} /* if (!dm_sig_status.sig_queue[cidx].status) */
	} /* if (is_all_signals_done()) */
}

#define STDOUT			1
#define DECIMAL_BASE		10
#define INT_TO_CHAR_OFF		48
#define STR_BUF_LEN		32

static void
sig_int_to_str(int num, char *str, int len)
{
	int i, digit = 1, cnt = 0, tmp;

	if (num == 0) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}

	while (TRUE) {
		if ((num / digit) > 0) {
			cnt++;
		} else {
			break;
		}
		digit *= DECIMAL_BASE;
	}

	digit /= DECIMAL_BASE;

	for (i = 0; i < cnt; i++) {
		tmp = num / digit;
		str[i] = tmp + INT_TO_CHAR_OFF;
		num -= (tmp * digit);
		digit /= DECIMAL_BASE;
	}

	str[len - 1] = '\0';
	str[i] = '\0';
}

static void
sig_str_print(char *msg)
{
	if (msg) {
		write(STDOUT, "debug_monitor: ", strlen("debug_monitor: "));
		write(STDOUT, msg, strlen(msg));
	} else {
		write(STDOUT, "\n", strlen("\n"));
	}
}

static void
sig_int_print(char *msg, int num)
{
	char val[STR_BUF_LEN] = {0, };
	if (msg) {
		write(STDOUT, msg, strlen(msg));
	}
	sig_int_to_str(num, val, STR_BUF_LEN);
	write(STDOUT, val, strlen(val));
}

static void
sig_handler_queueing(dm_sig_info_t *siginfo)
{
	int i = 0;
	int signum = siginfo->signo;
	int sifidx = siginfo->ifidx;
	int ridx = (dm_sig_status.received >= DM_QUEUE_MAX)? 0:
		dm_sig_status.received;
	bool dump_cmpl = siginfo->dump_cmpl;

	/* to check if the signal queue has a signal without the dump completion */
	for (; i < ridx; i++) {
		dm_sig_info_t sinfo = dm_sig_status.sig_queue[i];

		/*
		* sometimes each psm can send a signal
		* so we need to ignore the signal
		*/
		if ((signum != SIGUSR4) &&
			(signum == sinfo.signo) &&
			(sifidx == sinfo.ifidx) &&
			(dump_cmpl == sinfo.dump_cmpl)) {

			sig_str_print("ignore - received same signal\n");
			return;
		}

		/* to check if the signal queue has a signal without the dump completion */
		if (dump_cmpl) {
			/*
			* signal number, interface index,
			* and then the signal status are checked
			*/
			if ((signum == sinfo.signo) &&
				(sifidx == sinfo.ifidx) &&
				(!sinfo.dump_cmpl)) {
				/*
				* the signal is being processed in progress
				* after the dump completion timeout
				* the current tarball generation can be corrupted.
				* so the signal process status needs to be checked
				*/
				if (sinfo.status == NOTYETPROCESS) {
					dm_sig_status.sig_queue[i] = *siginfo;

					if (dm_sig_status.cur_idx == i) {
						dm_sig_status.cur_sig = *siginfo;
					}
#ifdef CRASH_HISTORY_FILE_LOGGING
					write_siginfo_history(siginfo);
#endif /* CRASH_HISTORY_FILE_LOGGING */
					sig_str_print("received dump cmpl\n");
					return;
				} else {
					/*
					* if the signal status is "in progress" or "done",
					* the dump signal completion signal can be skipped.
					*/
					return;
				} /* if (sinfo.status == NOTYETPROCESS) { */
			} /* if ((signum == sinfo.signo) &&  */
		} /* if(dump_cmpl */
	} /* for (; i < DM_QUEUE_MAX; i++) */

	/* the received signal doesn't exist in the signal queue */

	sig_str_print("queued signal\n");

	dm_sig_status.sig_queue[ridx] = *siginfo;

	if (ridx == dm_sig_status.cur_idx) {
		dm_sig_status.cur_sig = *siginfo;
	}

#ifdef CRASH_HISTORY_FILE_LOGGING
	write_siginfo_history(siginfo);
#endif /* CRASH_HISTORY_FILE_LOGGING */

	/*
	* dm_sig_status.received is reached to the max
	* and then reset to 0.
	*/
	if (dm_sig_status.received >= DM_QUEUE_MAX) {
		dm_sig_status.received = 0;
		dm_sig_status.overwrap = 1;
	} else {
		dm_sig_status.received++;
	}

	/*
	* If dm_sig_status.overwrap is set, it means dm_sig_status.received
	* is reached to the max. To protect the curren signal from overwraping,
	* dm_sig_status.received is limited.
	*/
	if (dm_sig_status.overwrap &&
		(dm_sig_status.received >= dm_sig_status.cur_idx)) {
		dm_sig_status.received = dm_sig_status.cur_idx - 1;
	}

	dm_sig_status.tot_received++;
}

static void
sig_handler(int signo, siginfo_t *sinfo, void *ctx)
{
	int sint = sinfo->si_int;

	sig_str_print("Enter sig handler\n");

	dm_sig_info_t siginfo = {
		.signo = signo,
		.ifidx = (sint & ~(DUMP_COMPLETION | NIC_MLO_SIG_FLAG)),
		.dump_cmpl = ((sint & DUMP_COMPLETION) ? 1 : 0),
		.is_recovery = 1,
		.status = NOTYETPROCESS,
		.drv_type = {0, },
		.timestamp = {0, },
		.check_init = 0,
		.mlo_sig = 0
	};

	time(&siginfo.rawtime);

	if (!is_sig_inprogress) {
		is_sig_inprogress = TRUE;
	}

	if ((signo == SIGUSR3) ||
		((signo == SIGUSR1) &&
		(sint & NIC_MLO_SIG_FLAG))) {
		siginfo.mlo_sig = 1;
	}

	sig_str_print("Rcvd sig = ");
	sig_int_print("ifidx ", siginfo.ifidx);
	sig_int_print(", signo ", siginfo.signo);
	sig_str_print(NULL);

	driver_type_str(&siginfo);

	sig_handler_queueing(&siginfo);

	sig_str_print("Exit sig handler : ");
	sig_int_print("tot received sig = ", dm_sig_status.tot_received);
	sig_str_print(NULL);
}

static void
dm_term_init(void)
{
#ifdef DM_NO_NVRAM_RESTART
	char *val = (char *) nvram_get("debug_monitor_proc_status");

	if (val && atoi(val)) {
		nvram_unset("debug_monitor_proc_status");
		nvram_commit();
	}
#endif /* DM_NO_NVRAM_RESTART */
}

static void
driver_type_str(dm_sig_info_t *siginfo)
{
	switch (siginfo->signo) {
	/* NIC */
		case SIGUSR1:
		if (siginfo->mlo_sig) {
			strncpy(siginfo->drv_type, "MLO_NIC", DRIVER_NAME_LEN - 1);
		} else {
			strncpy(siginfo->drv_type, "NIC", DRIVER_NAME_LEN - 1);
		}
		break;
	/* DHD */
		case SIGUSR2:
			strncpy(siginfo->drv_type, "DHD", DRIVER_NAME_LEN - 1);
		break;
	/* MLO */
		case SIGUSR3:
			strncpy(siginfo->drv_type, "MLO_DHD", DRIVER_NAME_LEN - 1);
		break;
	/* Health-check (HC) */
		case SIGUSR4:
			strncpy(siginfo->drv_type, "HC", DRIVER_NAME_LEN - 1);
		break;
	/* default is N/A */
		default:
			strncpy(siginfo->drv_type, "NA", DRIVER_NAME_LEN - 1);
		break;
	}
}

static int
check_dump_dir(char *path)
{
	int fcnt = 0;
	struct dirent *entry = NULL;
	DIR *dir = NULL;

	dir = opendir(path);
	if (dir) {
		while ((entry = readdir(dir)) != NULL) {
			if (strstr(entry->d_name, "dump_") || strstr(entry->d_name, "mem_dump")) {
				fcnt++;
			}
		}
		closedir(dir);
	}
	return fcnt;
}

#ifdef CRASH_HISTORY_FILE_LOGGING
static void
check_history_records(void)
{
	int items = 0;
	dm_record_info_t cur = {0, };

	while (!feof(dm_history.tgz_gen_history_fp)) {
		memset(&cur, 0, sizeof(cur));

		/*
		* timestamp(tab)signal number(tab)driver type(tab)
		* interface name(tab)tgz file name
		*/
		fscanf(dm_history.tgz_gen_history_fp, CRASH_TGZ_REC_FMT, &cur.time, &cur.signo,
			&cur.driver, &cur.ifname, &cur.tgzname);

		if ((cur.tgzname[0] != '\0') &&
			(cur.time[0] != '\0') &&
			(cur.signo > 0)) {

			items++;
		}
	}
	dm_history.tgz_items = items;

	items = 0;

	while (!feof(dm_history.siginfo_fp)) {
		memset(&cur, 0, sizeof(cur));
		/*
		* timestamp(tab)signal number(tab)driver type(tab)interface idx(tab)ifname(tab)
		* dump completion(tab)tgz file name
		*/
		fscanf(dm_history.siginfo_fp, CRASH_SIGINFO_REC_FMT, &cur.time, &cur.signo,
			&cur.driver, &cur.ifidx, &cur.ifname, &cur.dump_cmpl,
			&cur.tgzname);

		if ((cur.tgzname[0] != '\0') &&
			(cur.time[0] != '\0') &&
			(cur.signo > 0)) {

			items++;
		}
	}
	dm_history.sig_items = items;
}

static void
create_history_files(char *_backup_dir)
{
	char command[CMD_LEN] = {0, };
	dm_record_info_t sig_rec = {0, };

	if (_backup_dir) {
		snprintf(dm_history.tgz_gen_history_file, sizeof(dm_history.tgz_gen_history_file),
			"%s/"CRASH_TGZ_HISTORY_FILENAME, _backup_dir);
		snprintf(dm_history.siginfo_file, sizeof(dm_history.siginfo_file),
			"%s/"CRASH_SIGINFO_HISTORY_FILENAME, _backup_dir);
	} else {
		/* if _backup_dir is NULL, /data directory is used */
		snprintf(dm_history.tgz_gen_history_file, sizeof(dm_history.tgz_gen_history_file),
			"%s/"CRASH_TGZ_HISTORY_FILENAME, CRASH_HISTORY_PATH);
		snprintf(dm_history.siginfo_file, sizeof(dm_history.siginfo_file),
			"%s/"CRASH_SIGINFO_HISTORY_FILENAME, CRASH_HISTORY_PATH);
	}

	if (!open_history_files()) {
		return;
	}

	check_history_records();
}

static int
open_history_files(void)
{
	dm_history.tgz_gen_history_fp = fopen(dm_history.tgz_gen_history_file, "a+");

	if (!dm_history.tgz_gen_history_fp) {
		DM_PRINT("Doesn't open history file %s\n", dm_history.tgz_gen_history_file);

		return FALSE;
	}

	dm_history.siginfo_fp = fopen(dm_history.siginfo_file, "a+");
	if (!dm_history.siginfo_fp) {
		DM_PRINT("Doesn't open siginfo file %s\n", dm_history.siginfo_file);

		fclose(dm_history.tgz_gen_history_fp);
		dm_history.tgz_gen_history_fp = NULL;

		return FALSE;
	}

	return TRUE;
}

static void
create_record(dm_record_info_t *record, dm_sig_info_t siginfo)
{
	strncpy(record->time, siginfo.timestamp, TIMESTAMP_LEN - 1);

	record->dump_cmpl = siginfo.dump_cmpl;

	record->ifidx = siginfo.ifidx;

	record->signo = siginfo.signo;

	snprintf(record->ifname, sizeof(record->ifname), IFNAME_STR, siginfo.ifidx);

	strncpy(record->driver, siginfo.drv_type, DRIVER_NAME_LEN - 1);

	if (siginfo.dump_cmpl) {
		snprintf(record->tgzname, sizeof(record->tgzname), "%s%s_"IFNAME_STR"_%s",
			CRASH_LOG_PREFIX, siginfo.timestamp, siginfo.ifidx, siginfo.drv_type);
	} else {
		snprintf(record->tgzname, sizeof(record->tgzname), "NA");
	}
}

static void
write_tgz_gen_history(dm_sig_info_t *_siginfo)
{
	char command[CMD_LEN] = {0, };
	int cidx = dm_sig_status.cur_idx;
	dm_sig_info_t siginfo = dm_sig_status.sig_queue[cidx];
	dm_record_info_t record = {0, };

	if (siginfo.signo == SIGUSR4) {
		return;
	}

	if (!dm_history.tgz_gen_history_fp) {
		DM_PRINT("Don't write fp is null\n");
		return;
	}

	if (_siginfo) {
		siginfo = *_siginfo;
	}

	create_record(&record, siginfo);

	dm_history.tgz_items++;

	/* timestamp(tab)signal number(tab)driver type(tab)interface name(tab)tgz file name */
	fprintf(dm_history.tgz_gen_history_fp, CRASH_TGZ_REC_FMT, record.time, record.signo,
		record.driver, record.ifname, record.tgzname);
	fprintf(dm_history.tgz_gen_history_fp, "\n");
}

static void
close_history_files(void)
{
	char timestamp[TIMESTAMP_LEN] = {0, };
	char command[CMD_LEN] = {0, };

	if (dm_history.tgz_gen_history_fp) {
		fclose(dm_history.tgz_gen_history_fp);
	}
	dm_history.tgz_gen_history_fp = NULL;

	if (dm_history.siginfo_fp) {
		fclose(dm_history.siginfo_fp);
	}
	dm_history.siginfo_fp = NULL;

	/* create backup file */
	if ((dm_history.tgz_items >= CRASH_TGZ_HISTORY_LOG_NUM_MAX) ||
		(dm_history.sig_items >= CRASH_SIGINFO_HISTORY_LOG_NUM_MAX)) {

		time_formatting(NULL, timestamp);

		snprintf(command, sizeof(command), "rm %s_bak_* %s_bak_*",
			dm_history.tgz_gen_history_file, dm_history.siginfo_file);
		do_command(command);

		snprintf(command, sizeof(command), "mv %s %s_bak_%s",
			dm_history.tgz_gen_history_file, dm_history.tgz_gen_history_file,
			timestamp);
		do_command(command);

		snprintf(command, sizeof(command), "mv %s %s_bak_%s",
			dm_history.siginfo_file, dm_history.siginfo_file,
			timestamp);
		do_command(command);

		dm_history.tgz_items = 0;
		dm_history.sig_items = 0;
	}
}

static void
write_siginfo_history(dm_sig_info_t *siginfo)
{
	dm_record_info_t record;

	if (siginfo->signo == SIGUSR4) {
		return;
	}

	if (!dm_history.siginfo_fp) {

		/*
		* retry agian when a signal is recorded,
		* or open a file pointer in case the pointer was already closed
		*/
		if (!open_history_files()) {
			DM_PRINT("Don't write sig info fp is null\n");
			return;
		}
	}

	create_record(&record, *siginfo);

	dm_history.sig_items++;

	/*
	* timestamp(tab)signal number(tab)driver type(tab)interface idx(tab)ifname(tab)
	* dump completion(tab)tgz file name
	*/
	fprintf(dm_history.siginfo_fp, CRASH_SIGINFO_REC_FMT, record.time, record.signo,
		record.driver, record.ifidx, record.ifname, record.dump_cmpl, record.tgzname);
	fprintf(dm_history.siginfo_fp, "\n");

}
#endif /* CRASH_HISTORY_FILE_LOGGING */

#ifdef CHECK_DUMPS_WHEN_INIT
static void
free_size_check(char *fname)
{
	int log_size, retain_logs;
	unsigned long free_space = 0;
	struct stat file_stat;

	if (stat(fname, &file_stat)) {
		dprintf("%s: open log file %s fail, %s\n", _mod_name, fname,
			strerror(errno));
	}

	for (retain_logs = (max_crash_logs - 1); retain_logs >= 0; retain_logs--) {
		DM_PRINT("%s: going to retain %d log(s) under backup dir (%s)\n",
			retain_logs, _backup_dir);
		delete_extra_crash_files(_backup_dir, retain_logs, NULL);

		if (retain_logs) {
			free_space = get_dir_free_space(_backup_dir);
			if (free_space < 0) {
			/* failed to get disk space of backup dir, skip space check */
				break;
			}

			if (log_size > free_space) {
				DM_PRINT("Disk space (%lu) not enough for new log (%d), "
					"retain one less log\n",
					free_space, log_size);
			} else {
			/* free disk space is large enough for the new log */
				break;
			}
		}
	}
}

static int
file_size_check(char *file)
{
	struct stat file_stat;
	int ret = 0;

	if ((stat(file, &file_stat) == 0) && file_stat.st_size) {
		DM_PRINT("%s, file size %d bytes\n", file, file_stat.st_size);
		ret = file_stat.st_size;
	}
	return ret;
}

static void
check_crash_logs(void)
{
	int i;
	char command[CMD_LEN] = {0, };
	char ifpath[PATH_LEN] = {0, };
	char fname[FILE_LEN] = {0, };
	char timestamp[TIMESTAMP_LEN] = {0, };
	struct dirent *entry = NULL;
	struct stat fstat;
	DIR* dir = NULL;

	dm_sig_info_t siginfo = {
		.signo = -1,
		.ifidx = -1,
		.dump_cmpl = 1,
		.check_init = 1,
		.timestamp = {0, }
	};

	if (!_backup_dir) {
		return;
	}

#ifdef CRASH_HISTORY_FILE_LOGGING
	create_history_files(_backup_dir);
#endif /* CRASH_HISTORY_FILE_LOGGING */

	dir = opendir(LOG_BASE_PATH);
	if (dir) {
		while ((entry = readdir(dir)) != NULL) {
			/* If a file with the prefix, "debug-",in /tmp/crash_logs exists */
			if (strstr(entry->d_name, CRASH_LOG_PREFIX)) {
				/* If the file has a tgz file */
				if (strstr(entry->d_name, ".tgz")) {
					snprintf(ifpath, sizeof(ifpath), LOG_BASE_PATH"/%s",
						entry->d_name);

					/* Check the file size */
					if (file_size_check(ifpath)) {
						DM_PRINT("%s in %s directory, move into %s\n",
							entry->d_name, LOG_BASE_PATH, _backup_dir);

						/* Check current storage free size */
						free_size_check(ifpath);

						/* Move the tgz file to the backup dir */
						snprintf(command, sizeof(command),
							"mv "LOG_BASE_PATH"/%s %s",
							entry->d_name, _backup_dir);
						do_command(command);
					}
				}	/* strstr(entry->d_name, ".tgz") */
				else if (strstr(entry->d_name, ".tar")) {
					snprintf(ifpath, sizeof(ifpath), LOG_BASE_PATH "/%s",
						entry->d_name);

					if (file_size_check(ifpath)) {
						DM_PRINT("%s in %s directory, create a tgz\n",
							entry->d_name, LOG_BASE_PATH);
						/*
						* 4 is the length of ".tar"
						* so only debug-time_wlX will be copied
						*/
						memcpy(fname, entry->d_name,
							strlen(entry->d_name) - 4);

						/* the tar fill is converted to a tgz file */
						snprintf(command, sizeof(command),
							"gzip -c " LOG_BASE_PATH "/%s > "
							LOG_BASE_PATH "/%s.tgz", entry->d_name,
							fname);
						do_command(command);

						snprintf(ifpath, sizeof(ifpath),
							LOG_BASE_PATH "/%s.tgz", fname);

						if (file_size_check(ifpath)) {
							DM_PRINT("%s move into %s\n",
								ifpath, _backup_dir);
							free_size_check(ifpath);
							snprintf(command, sizeof(command),
								"mv %s %s", ifpath,
								_backup_dir);
							do_command(command);
						}
					} /* if (file_size_check()) */
				} /* strstr(entry->d_name, ".tar") */
			}
		} /* while */

		/* delete files in /tmp/crash_logs */
		snprintf(command, sizeof(command), "rm -rf " LOG_BASE_PATH);
		do_command(command);
		closedir(dir);
		dir = NULL;
		sync();
	} /* if (dir) */

	/* check if dumps are in /tmp/wlX */
	for (i = 0; i < MAX_WL_IFS; i++) {
		snprintf(ifpath, sizeof(ifpath), DUMP_BASE_PATH "/" IFNAME_STR, i);

		if (check_dump_dir(ifpath) > 0) {

			extract_dump_signature(ifpath, siginfo.dump_signature);
			if (strlen(siginfo.dump_signature) > 0) {

				DM_PRINT("Detected dumps in %s during init\n", ifpath);

				siginfo.ifidx = i;

				time_formatting(NULL, timestamp);

				strncpy(siginfo.timestamp, timestamp, TIMESTAMP_LEN - 1);

				collect_logs_by_if(&siginfo, NULL);

#ifdef CRASH_HISTORY_FILE_LOGGING
				write_tgz_gen_history(&siginfo);
#endif /* CRASH_HISTORY_FILE_LOGGING */
				sync();
			}
		} /* if (check_dump_dir()) */
	} /* for () */

#ifdef CRASH_HISTORY_FILE_LOGGING
	close_history_files();
#endif /* CRASH_HISTORY_FILE_LOGGING */
}
#endif /* CHECK_DUMPS_WHEN_INIT */

static int
create_symlink(void)
{
	int ret = BCME_OK, unit;
	char to[PATH_LEN] = {0, };
	char from[PATH_LEN] = {0, };

	for (unit = 0; unit < MAX_WL_IFS; unit++) {
		snprintf(to, sizeof(to), DUMP_BASE_PATH "/" IFNAME_STR, unit);

		snprintf(from, sizeof(from), DUMP_BASE_MISC_PATH "/" IFNAME_STR, unit);

		if ((symlink(from, to) < 0) && errno != EEXIST) {
			DM_PRINT("Failed to create sym link - err : %s\n", strerror(errno));
			ret = BCME_ERROR;
		}
	}
	return ret;
}

static int
create_log_hc_dir(char *_backup_dir)
{
	char filepath[PATH_LEN] = {0, };
	int ret = BCME_OK;

	/* Create a healthcheck folder at the persistent folder */
	snprintf(filepath, sizeof(filepath), "%s/%s", _backup_dir, LOG_HC_FOLDER);

	if ((mkdir(filepath, 0777) < 0) && errno != EEXIST) {
		DM_PRINT("Unable to create health check log dir %s, err '%s'\n",
			filepath, strerror(errno));
		ret = BCME_ERROR;
	}
	return ret;
}

static int
create_log_tmp_dirs(void)
{
	char tmp_path[PATH_LEN] = {0, };
	char misc_path[PATH_LEN] = {0, };
	int ret = BCME_OK, unit;

	/* Create a healthcheck parent folder */
	snprintf(tmp_path, sizeof(tmp_path), DUMP_BASE_PATH "/hc");
	ret = create_dir(tmp_path);

	if (ret) {
		goto exit;
	}

	/* Create interface specific folders */
	for (unit = 0; unit < MAX_WL_IFS; unit++) {

		snprintf(tmp_path, sizeof(tmp_path), DUMP_BASE_PATH "/hc/if%d", unit);
		ret = create_dir(tmp_path);

		snprintf(tmp_path, sizeof(tmp_path),  DUMP_BASE_PATH "/hc/if%d/dstall", unit);
		ret = create_dir(tmp_path);

		if (ret) {
			goto exit;
		}

		snprintf(tmp_path, sizeof(tmp_path), DUMP_BASE_PATH "/" IFNAME_STR, unit);

		if (is_driver_dump_path_to_misc()) {
			snprintf(misc_path, sizeof(misc_path),
				DUMP_BASE_MISC_PATH "/" IFNAME_STR, unit);

			ret = create_dir(misc_path);

			if (ret) {
				ret = create_dir(tmp_path);
			}
		} else {
			ret = create_dir(tmp_path);
		} /* if (is_dirver_dump_path_to_misc()) */

		if (ret) {
			goto exit;
		}
	} /* for loop */

exit:
	return ret;
}

static int
create_dir(char *pdir)
{
	int ret = BCME_OK, unit;

	if ((mkdir(pdir, 0777) < 0) && errno != EEXIST) {
		DM_PRINT("Unable to create log directory %s, err '%s'\n",
			pdir, strerror(errno));
		ret = BCME_ERROR;
	}
	return ret;
}

static void
remove_dirs(void)
{
	char path[PATH_LEN] = {0, };
	char cmd[CMD_LEN] = {0, };
	int i;

	snprintf(path, sizeof(path), "%s", LOG_BASE_PATH);

	if (!access(path, F_OK)) {
		snprintf(cmd, sizeof(cmd), "rm -rf %s", LOG_BASE_PATH);
	}

	for (i = 0; i < MAX_WL_IFS; i++) {
		snprintf(path, sizeof(path), DUMP_BASE_PATH "/" IFNAME_STR, i);
		if (!access(path, F_OK)) {
			snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
			do_command(cmd);
		}
	} /* for loop */
}

static bool
is_driver_dump_path_to_misc(void)
{
	char *misc_var = (char *) nvram_get("crash_log_backup_mtd");
	char *dump_var = (char *) nvram_get("crash_dump_to_misc_backup_to_data");

	if (misc_var && !strcmp(misc_var, "misc") &&
		is_misc_partition_mounted() &&
		dump_var && (atoi(dump_var) > 0)) {

		return TRUE;
	}
	return FALSE;
}

static bool
is_misc_partition_mounted(void)
{
	char *var = (char *)nvram_get("crash_log_backup_mtd");
	char buf[DM_BUF_SZ] = {0, };
	FILE *fp = NULL;
	int ret = FALSE;

	if (var && !strcmp(var, "misc")) {
		fp = fopen("/proc/mounts", "r");

		if (!fp) {
			return ret;
		}

		while (!feof(fp)) {
			fscanf(fp, "%s", buf);

			if ((strlen(buf) > 0) && (buf[0] == '/')) {
				if (!strcmp("/mnt/misc", buf)) {
					ret = TRUE;
					break;
				}
			} /* if (strlen( ... */
		} /* while loop */

		fclose(fp);
	} /* if (var && .... */
	return ret;
}

static void
print_storage_size(void)
{
	unsigned long backupdir_size_bytes = 0;
	unsigned long backupdir_size_mbytes = 0;
	/* This is allocated bytes to the mis partition */
	unsigned long partition_size = 0;
	char buf[DM_BUF_SZ] = {0, };
	FILE *fp = NULL;

	if (is_misc_partition_mounted()) {
		/* misc partition vol num is 12 */
		fp = fopen("/sys/class/ubi/ubi0_12/data_bytes", "r");

		if (!fp) {
			return;
		}

		fscanf(fp, "%s", buf);
		fclose(fp);
		fp = NULL;

		partition_size = atol(buf);
		/* Kbytes */
		partition_size /= 1024;
		/* Mbytes */
		partition_size /= 1024;

		DM_PRINT("Allocated misc partition size = %s bytes, %lu Mb\n",
			buf, partition_size);
	}

	backupdir_size_bytes = get_dir_free_space(_backup_dir);
	/* Kbytes */
	backupdir_size_mbytes = backupdir_size_bytes/1024;
	/* Mbytes */
	backupdir_size_mbytes /= 1024;

	DM_PRINT("dir %s free size %lu bytes(%lu Mbytes)\n",
		_backup_dir, backupdir_size_bytes, backupdir_size_mbytes);
}

/*
* crash_dump_to_misc_backup_to data is intended
* for drivers only to create dumps in directories on the misc partition.
*/
static void
check_backup_path(void)
{
	if (is_driver_dump_path_to_misc()) {
		/* BACKUP_BASE_PATH is /data */
		_backup_dir = BACKUP_BASE_PATH;

		DM_PRINT("Backup dir %s, dump dir %s\n", _backup_dir, DUMP_BASE_MISC_PATH);
	}
}

static bool
is_mlo_configured(void)
{
	int i;

	for (i = 0; i < PCIE_IPC_AP_UNIT_MAX; i++) {
		if (nvram_mlo[i] >= 0) {
			return TRUE;
		}
	}
	return FALSE;
}

static void
check_max_crash_logs(void)
{
	char *maxnum_var = (char *) nvram_get("crash_log_backup_max_num");
	int max_val = 0;

	if (maxnum_var) {
		max_val = atoi(maxnum_var);

	/*
	* "/data" directory is a common directory that all applications can access.
	* It normally has about 20Mbytes, and one tgz file is about 3~4Mbytes.
	* If the directory is filled with tgz files, applications can't work.
	* To avoid it, the max num is needed to set 3 by default.
	*/
		if (max_val <= 0) {
			goto exit;
		}

		max_crash_logs = max_val;
	}

	/*
	* "/data" directory is a common directory that all applications can access.
	* It normally has about 20Mbytes, and one tgz file is about 3~4Mbytes.
	* If the directory is filled with tgz files, applications can't work.
	* To avoid it, the max num is needed to set 3 by default.
	*/

	if (!strcmp(_backup_dir, BACKUP_BASE_PATH)) {
		max_crash_logs = DEFAULT_CRASH_LOGS_LIMIT;
		goto exit;
	}

	/*
	* If the backup_dir isn't "/data" directory and
	* the mlo nvram configuration, wl_mlo_config, is set,
	* the number can be set to >=6.
	*/
	if (is_mlo_configured() &&
		(max_crash_logs < DEFAULT_CRASH_LOGS_MLO_LIMIT)) {
		max_crash_logs = DEFAULT_CRASH_LOGS_MLO_LIMIT;
	}

	/* if a ramdisk is mounted */
	if (is_ramfs_mounted()) {
		if ((get_dir_free_space(_backup_dir) >= LIMIT_RAM_DISK_SIZE) &&
			(max_crash_logs > CRASH_LOGS_RAMFS_MAX_LIMIT)) {

			max_crash_logs = CRASH_LOGS_RAMFS_MAX_LIMIT;
		}
	}

exit:
	DM_PRINT("Up to %d files will be stored in %s\n", max_crash_logs, _backup_dir);

	/* print storage size /data dir or misc partition */
	print_storage_size();
}

static bool
is_ramfs_mounted(void)
{
	char *var = (char *) nvram_get("crash_log_backup_mtd");

	if (var && !strcmp(var, "ram") && is_ramfs(_backup_dir)) {
		return TRUE;
	}
	return FALSE;
}

static bool
is_misc_size_enough(void)
{
	unsigned long size_bytes = 0;
	char buf[DM_BUF_SZ] = {0, };
	FILE *fp = NULL;

	if (is_misc_partition_mounted()) {
		/* misc partition vol num is 12 */
		fp = fopen("/sys/class/ubi/ubi0_12/data_bytes", "r");

		if (!fp) {
			return TRUE;
		}

		fscanf(fp, "%s", buf);
		fclose(fp);
		fp = NULL;

		size_bytes = atol(buf);

		if ((max_crash_logs * ONE_TGZ_SIZE) > size_bytes) {
			return FALSE;
		}
	}
	return TRUE;
}

static void
check_auto_mount_ramdisk(void)
{
	char cmd[CMD_LEN] = {0, };

	/* ram disk is already mounted */
	if (is_ramfs_mounted()) {
		return;
	}

	/*
	* misc partition is mounted but
	* no enough space to save files.
	*/

	if (!is_misc_size_enough()) {

		DM_PRINT("Try to mount ramdisk\n");

		/* set nvarm to "ram" */
		nvram_set("crash_log_backup_mtd", "ram");
		nvram_commit();

		/* unmount misc */
		snprintf(cmd, sizeof(cmd),
			"hnddm.sh \"\" \"\" %s %s 2> /dev/null",
			"misc", "/mnt/misc/crash_logs");
		do_command(cmd);

		/* remove all things in /mnt/misc */
		snprintf(cmd, sizeof(cmd), "rm -rf /mnt/misc");
		do_command(cmd);

		/* mount a ramdisk */
		snprintf(cmd, sizeof(cmd),
			"hnddm.sh %s %s 2> /dev/null",
			"ram", "/mnt/misc/crash_logs");
		do_command(cmd);
	}
}

void
usage(char *progname)
{
	DM_PRINT("Usage: %s <backup directory>\n", progname);
}
#endif /* __CONFIG_DHDAP__ */

static int
dm_is_process_active(char * pid)
{
	char proc_path[DM_BUF_SZ];
	DIR *dir;

	snprintf(proc_path, sizeof(proc_path), "/proc/%s", pid);

	if ((dir = opendir(proc_path)) == NULL) {
		dprintf("%s error opening %s\n", __FUNCTION__, proc_path);
		return FALSE;
	}

	closedir(dir);
	return TRUE;
}

/* trim trailing space like characters */
static void
dm_trim(char *str, size_t sz)
{
	size_t len = 0;
	int i;

	if (!str || !str[0]) {
		return;
	}

	len = strnlen(str, sz);

	for (i = len - 1; i > 0 && isspace(str[i]); i--) {
		str[i] = '\0';
	}
}

static int
dm_read_pid_info_file(char *path, char *dep_cmd, size_t dep_cmd_sz,
	char *process_cmd, size_t process_cmd_sz)
{
	FILE *fp;

	if ((fp = fopen(path, "r")) == 0) {
		return BCME_ERROR;
	}

	fgets(dep_cmd, dep_cmd_sz, fp);
	dm_trim(dep_cmd, dep_cmd_sz);

	fgets(process_cmd, process_cmd_sz, fp);
	dm_trim(process_cmd, process_cmd_sz);

	fclose(fp);

	return BCME_OK;
}

static int
dm_handle_process(char *pid, unsigned int depth)
{
	char process_dm_path[DM_BUF_SZ];
	char dep_cmd[DM_BUF_SZ] = {0};
	char process_cmd[DM_BUF_SZ] = {0};
	char dep_pid[DM_BUF_SZ] = {0};

	if (depth > DM_DEPENDENCY_DEPTH_MAX) {
		return BCME_ERROR;
	}

	if (nvram_match("wlready", "0"))
		return FALSE;

	if (dm_is_process_active(pid)) {
		/*
		 * TODO: Could check timestamp if agreed to touch DM_DIR
		 * at specific intervals
		 * if process is active; nothing to do
		 */
		return FALSE;
	}

	dprintf("%s process id %s does not exist will restart\n",
		__FUNCTION__, pid);

	snprintf(process_dm_path, sizeof(process_dm_path), "%s/%s", DM_DIR, pid);

	/* if process is not active */
	if (dm_read_pid_info_file(process_dm_path, dep_cmd, sizeof(dep_cmd),
		process_cmd, sizeof(process_cmd)) != 0) {
		return FALSE;
	}

	/* if dependency is mentioned ... */
	if (dep_cmd[0] && dep_cmd[0] != '\r' && dep_cmd[0] != '\n') {
		/* ... find the pid of the dep_cmd */
		if (dm_get_pid_of_cmd(dep_cmd, dep_pid, sizeof(dep_pid))) {
			/* ... and handle the dep_cmd process first */
			dm_handle_process(dep_pid, depth + 1);
		}
	}

	/* remove entry of process in DM_DIR */
	if (remove(process_dm_path) != 0) {
		dprintf("%s: Couldnt remove %s\n", __FUNCTION__, process_dm_path);
	}

	/* restart using process_cmd */
	dprintf("%s Restarting process %s\n", __FUNCTION__, process_cmd);
	system(process_cmd);

	return TRUE;
}

static unsigned char
dm_match_cmd(char *pid, char *process_cmd_needle)
{
	char process_dm_path[DM_BUF_SZ];
	char dep_cmd[DM_BUF_SZ] = {0};
	char process_cmd[DM_BUF_SZ] = {0};
	char dep_pid[DM_BUF_SZ] = {0};

	snprintf(process_dm_path, sizeof(process_dm_path), "%s/%s", DM_DIR, pid);

	dm_read_pid_info_file(process_dm_path, dep_cmd, sizeof(dep_cmd),
		process_cmd, sizeof(process_cmd));

	if (strncmp(process_cmd, process_cmd_needle, sizeof(process_cmd)) == 0) {
		return TRUE;
	}

	return FALSE;
}

static unsigned char
dm_get_pid_of_cmd(char *dep_cmd, char *dep_pid, size_t dep_pid_sz)
{
	DIR *dir;
	struct dirent *de;

	if ((dir = opendir(DM_DIR)) == NULL) {
		dprintf("%s error opening %s\n", __FUNCTION__, DM_DIR);
		return FALSE;
	}

	while ((de = readdir(dir)) != NULL) {
	/* walk through entries in DM_DIR and find respective pid */
		if (de->d_name[0] == '.')
			continue;
		if (dm_match_cmd(de->d_name, dep_cmd)) {
			strncpy(dep_pid, de->d_name, dep_pid_sz);
			closedir(dir);
			return TRUE;
		}
	}

	closedir(dir);
	return FALSE;
}

static void
dm_watchdog()
{
	DIR *dir;
	struct dirent *de;

	if ((dir = opendir(DM_DIR)) == NULL) {
		dprintf("%s error opening %s\n", __FUNCTION__, DM_DIR);
		return;
	}

	while ((de = readdir(dir)) != NULL) {
	/* walk through entries in DM_DIR and call dm_handle_process on each */
		if (de->d_name[0] == '.') {
			continue;
		}
		dm_handle_process(de->d_name, 0);
	}
	closedir(dir);
}

int
main(int argc, char **argv)
{
	_mod_name = argv[0];
	char *nv_str;

#if defined(__CONFIG_DHDAP__)
	struct sigaction act_dmsig = {0, };
	int ret;

	dm_term_init();

	if (argc < 2) {
		DM_PRINT("Error no backup dir.\n");
		usage(_mod_name);
		return BCME_OK;
	} else {
		_backup_dir = argv[1];
		check_backup_path();
	}

	/* Read and parse MLO nvram config */
	mlo_read_nvram();

	check_auto_mount_ramdisk();

	check_max_crash_logs();

	/* Register signal handlers to recevie siganls from drivers */
	memset(&dm_sig_status, 0, sizeof(dm_sig_status));
	/*
	 * sigfillset can temporarily block all the signals to debug_monitor
	 * while the signal handler is being executed.
	 */
	sigfillset(&act_dmsig.sa_mask);
	act_dmsig.sa_flags = SA_SIGINFO | SA_RESTART;
	act_dmsig.sa_sigaction = sig_handler;

	ret = sigaction(SIGUSR1, &act_dmsig, (struct sigaction *)NULL);
	if (ret != 0) {
		DM_PRINT("Error - %d setting up SIGUSR1 handler.\n", ret);
		return BCME_ERROR;
	}

	ret = sigaction(SIGUSR2, &act_dmsig, (struct sigaction *)NULL);
	if (ret != 0) {
		DM_PRINT("Error - %d setting up SIGUSR2 handler.\n", ret);
		return BCME_ERROR;
	}

	ret = sigaction(SIGUSR3, &act_dmsig, (struct sigaction *)NULL);
	if (ret != 0) {
		DM_PRINT("Error - %d setting up SIGUSR3 handler.\n", ret);
		return BCME_ERROR;
	}

	ret = sigaction(SIGUSR4, &act_dmsig, (struct sigaction *)NULL);
	if (ret != 0) {
		DM_PRINT("Error - %d setting up SIGUSR4 handler.\n", ret);
		return BCME_ERROR;
	}

#ifdef CHECK_DUMPS_WHEN_INIT
	/* When debug_monitor is launched, dumps are still in there */
	check_crash_logs();
#endif	/* CHECK_DUMPS_WHEN_INIT */

	 remove_dirs();
	/* Create separate log dirs for each interface */
	create_log_tmp_dirs();

	/* Create separate health check log dir folder */
	create_log_hc_dir(_backup_dir);

	/* if driver creates dumps to a dir on misc partition */
	if (is_driver_dump_path_to_misc()) {
	/*
	* Drivers still create dumps in /tmp/wlX.
	* but /tmp/wlX directories is linked to /mnt/misc/crash_logs/wlX.
	* Then the dumps exist in the directories on the misc partition
	*/
		create_symlink();
	}

#ifdef CRASH_HISTORY_FILE_LOGGING
	/* open crash history file logging */
	create_history_files(_backup_dir);
#endif /* CRASH_HISTORY_FILE_LOGGING */

#endif /* __CONFIG_DHDAP__ */
	nv_str = (char *)nvram_get("debug_monitor_disable_app_restart");

	if (nv_str && (nv_str[0] == '1') && (nv_str[1] == '\0')) {
		dbg_mon_disab_rstrt = TRUE;
	}
	if (daemon(1, 1) == -1) {
		DM_PRINT("Error daemonizing.\n");
	} else {
		unsigned int tick = 0;
		while (1) {
#if defined(__CONFIG_DHDAP__)
			sig_handler_watchdog();
#endif /* __CONFIG_DHDAP__ */

			if (!dbg_mon_disab_rstrt &&
				((tick++ % DM_INTERVAL) == 0) &&
#if defined(__CONFIG_DHDAP__)
				!is_sig_inprogress &&
#endif /* __CONFIG_DHDAP__ */
				1) {
				dm_watchdog();
			}
			sleep(1);
		}
	}
#if defined(__CONFIG_DHDAP__)
#ifdef CRASH_HISTORY_FILE_LOGGING
	close_history_files();
#endif /* CRASH_HISTORY_FILE_LOGGING */
#endif /* __CONFIG_DHDAP__ */

	return BCME_OK;
}
