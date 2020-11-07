/*
 * DHD Monitor Daemon
 *
 * Copyright (C) 2019, Broadcom. All Rights Reserved.
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
 * $Id: dhd_monitor.c 766006 2018-07-23 12:27:15Z $
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
#ifdef BCA_HNDROUTER
#include <sys/utsname.h> /* for uname */
#endif /* BCA_HNDROUTER */
#include <bcmnvram.h>

/* defines */
#ifndef RAMFS_MAGIC
#define RAMFS_MAGIC		0x858458f6
#endif // endif

#ifndef TMPFS_MAGIC
#define TMPFS_MAGIC		0x01021994
#endif // endif

#define	LOG_BASE_PATH			"/tmp/crash_logs"
#define	CRASH_LOG_PREFIX		"debug-"
#define	LOG_UNAME_FILENAME		"uname.txt"
#define	LOG_DMESG_FILENAME		"dmesg.txt"
#define	LOG_DONGLE_MEM_FILENAME		"dongle_mem_dump.bin"
#define IF_IDX_FILE_PATH		"/tmp/failed_if.txt"
#define LOG_D11REGDUMP_FILENAME		"d11reg_dump.txt"
#define LOG_D11REGXDUMP_FILENAME        "d11regx_dump.txt"
#define LOG_CONSOLE_DUMP                "console_dump.txt"
#define LOG_SOCRAM_DUMP                 "socram_dump.txt"
#define LOG_SCTPLDUMP_FILENAME          "sctpl_dump.txt"
#define LOG_SCTPLXDUMP_FILENAME         "sctplx_dump.txt"
#define LOG_VASIPDUMP_FILENAME          "vasip_dump.txt"
#define DEBUG_MONITOR_FILENAME          "/tmp/debug_monitor.txt"

#if defined(BCA_CPEROUTER)
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
#define rc_reboot()		kill(1, SIGTERM)

#define unload_driver(nic)	eval("rmmod", (nic ? "wl" : "dhd"))
#define load_driver(nic)	{					\
	struct utsname name;						\
	char buf[PATH_MAX];						\
	uname(&name);							\
	snprintf(buf, sizeof(buf),					\
	(nic ? "/lib/modules/%s/extra/wl.ko" : "/lib/modules/%s/extra/dhd.ko"),	\
		name.release);						\
	eval("insmod", buf);						\
}

#else /* ! BCA_HNDROUTER && !BCA_CPEROUTER */

#define rc_stop()		kill(1, SIGINT)
#define rc_start()		kill(1, SIGUSR2)
#define rc_restart()		kill(1, SIGHUP)
#define rc_reboot()		kill(1, SIGTERM)

#define unload_driver(nic)	eval("rmmod", (nic ? "wl" : "dhd"))
#define load_driver(nic)	eval("insmod", (nic ? "/tmp/wl.ko" : "/tmp/dhd.ko"))
#endif /* !BCA_CPEROUTER && !BCA_HNDROUTER */

/* Number of crash logs to retain */
#define MAX_CRASH_LOGS	3

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

static int dm_is_process_active(char * dirname);
static void dm_trim(char *str, size_t sz);
static int dm_read_pid_info_file(char *path, char *dep_cmd, size_t dep_cmd_sz,
	char *process_cmd, size_t process_cmd_sz);
static int dm_handle_process(char *pid, unsigned int depth);
static unsigned char dm_match_cmd(char *pid, char *process_cmd_needle);
static unsigned char dm_get_pid_of_cmd(char *dep_cmd, char *dep_pid, size_t dep_pid_sz);
static void dm_watchdog();

char *_mod_name = NULL;
int dbg_mon_disab_rstrt = FALSE;
#if defined(__CONFIG_DHDAP__)
char *_backup_dir = NULL;

static int do_command(const char *cmd)
{
	int status = system(cmd);
#ifdef WIFEXITED
	if (!WIFEXITED(status)) {
		fprintf(stderr, "%s: command (\"%s\") terminated\n",
			_mod_name, cmd);
		return -1;
	}

	status = WEXITSTATUS(status);
#endif // endif
	if (status) {
		fprintf(stderr, "%s: command (\"%s\") failed: %s\n", _mod_name,
			cmd, strerror(status));
		return -1;
	}
	return 0;
}

static void get_timestamp(char *buffer, int max_len)
{
	time_t rawtime;
	struct tm *info;

	time(&rawtime);
	info = localtime(&rawtime);
	strftime(buffer, max_len, "%F_%T", info);
	return;
}

static void handle_recovery(int nic_wl)
{
	char *val = (char *) nvram_get("fast_restart");

	if (val && atoi(val)) {
		/* Fast Restart - rc_restart */
		/* stop all services */
		printf("%s: stop services\n", _mod_name);
		rc_stop();

		/* rc_stop is a non-blocking call. So, add sufficient sleep
		 * to make sure all services are stopped before proceeding.
		 */
		sleep(3);

		/* unload dhd */
		printf("%s: unload dhd\n", _mod_name);
		unload_driver(nic_wl);
		sleep(1);

		/* reload dhd */
		printf("%s: reload dhd\n", _mod_name);
		load_driver(nic_wl);
		sleep(1);

		/* start all services */
		printf("%s: restart services\n", _mod_name);
		rc_start();
	} else {
		/* Full restart - reboot */
		val = (char *) nvram_get("watchdog");
		if (val && atoi(val)) {
			/* full reboot */
			printf("%s: rebooting system\n", _mod_name);
			rc_reboot();
		} else {
			printf("%s: watchdog disabled, ignoring dongle trap/wl kernel panic\n",
				_mod_name);
		}
	}
}

/* Filter out none "*.tgz" files */
static int dir_filter(const struct dirent *dptr)
{
	if (strstr(dptr->d_name, CRASH_LOG_PREFIX) && strstr(dptr->d_name, ".tgz"))
		return 1;
	return 0;
}

/* Remove any extra crash logs that might have accumulated over time */
/* Prune the oldest crash logs */
/* retains: Number of crash logs to retain */
static int delete_extra_crash_files(const char *log_dir, uint retains)
{
	struct dirent **fnamelist;
	int n, i;
	char filepath[256];

	if (!log_dir) {
		fprintf(stderr, "%s: Error: log_dir is NULL\n", _mod_name);
		return -1;
	}

	n = scandir(log_dir, &fnamelist, dir_filter, alphasort);
	if (n < 0) {
		fprintf(stderr, "%s: could not scan log_dir(%s) folder\n", _mod_name,
			log_dir, strerror(errno));
	} else {
		int deletes = n - retains;
		for (i = 0; i < n; i++) {
			if (deletes > 0) {
				deletes--;
				printf("%s: pruning old crash log - %s/%s\n", _mod_name,
					log_dir, fnamelist[i]->d_name);
				snprintf(filepath, sizeof(filepath), "%s/%s",
					log_dir, fnamelist[i]->d_name);
				unlink(filepath);
			}
			/* free each entry allocated by scandir() */
			free(fnamelist[i]);
		}
		free(fnamelist);
	}

	return 0;
}

static int get_sys_free_mem(void)
{
	FILE *fp;
	char buf[PATH_MAX];
	int free_mem = -1;

	if ((fp = fopen("/proc/meminfo", "r")) == NULL) {
		fprintf(stderr, "%s: read meminfo fail, %s\n", _mod_name, strerror(errno));
		return -1;
	}

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (sscanf(buf, "MemFree: %d kB", &free_mem))
			break;
	}
	fclose(fp);

	return free_mem;
}

static int get_dir_free_space(const char *dir)
{
	struct statfs dir_statfs;
	int free_space = 0;

	if (statfs(dir, &dir_statfs)) {
		fprintf(stderr, "%s: statfs backup dir %s fail\n", _mod_name, dir,
			strerror(errno));
		return -1;
	}

	if (dir_statfs.f_type == RAMFS_MAGIC ||
			dir_statfs.f_type == TMPFS_MAGIC) {
		if ((free_space = get_sys_free_mem()) < 0) {
			fprintf(stderr, "%s: get sys free mem fail\n", _mod_name);
			return -1;
		}
		free_space = free_space * 1024;
	} else {
		free_space = dir_statfs.f_bsize * dir_statfs.f_bfree;
	}

	return free_space;
}

static int is_ramfs(const char *dir)
{
	struct statfs dir_statfs;
	int ret = 0;

	if (statfs(dir, &dir_statfs)) {
		fprintf(stderr, "%s: statfs backup dir %s fail\n", _mod_name, dir,
			strerror(errno));
		return ret;
	}

	if (dir_statfs.f_type == RAMFS_MAGIC ||
			dir_statfs.f_type == TMPFS_MAGIC) {
		ret = 1;
	}

	return ret;
}

static int backup_logs(char *timestamp, const char *backup_dir)
{
	char logfile[128];
	char cp_file[256];
	DIR *dir;
	struct stat log_stat;
	int free_space = 0;
	int ret = -1;

	if (!timestamp) {
		fprintf(stderr, "%s: Error: timestamp is NULL\n", _mod_name);
		goto fail;
	}

	if (!backup_dir) {
		fprintf(stderr, "%s: Error: backup_dir is NULL\n", _mod_name);
		goto fail;
	}

	if (!(dir = opendir(backup_dir))) {
		fprintf(stderr, "%s: open backup dir %s fail, %s\n", _mod_name, backup_dir,
			strerror(errno));
		goto fail;
	}
	closedir(dir);

	if (access(backup_dir, W_OK)) {
		fprintf(stderr, "%s: access backup dir %s fail\n", _mod_name, backup_dir,
			strerror(errno));
		goto fail;
	}

	snprintf(logfile, sizeof(logfile), "%s/%s%s.tgz", LOG_BASE_PATH,
		CRASH_LOG_PREFIX, timestamp);

	if (stat(logfile, &log_stat)) {
		fprintf(stderr, "%s: open log file %s fail, %s\n", _mod_name, logfile,
			strerror(errno));
		goto fail;
	}

	free_space = get_dir_free_space(backup_dir);

	if (log_stat.st_size > free_space || log_stat.st_size == 0) {
		fprintf(stderr, "%s: log size (%d) over backup dir (%s%s)"
			" available space (%d)\n", _mod_name, log_stat.st_size,
			backup_dir, is_ramfs(backup_dir)? " a ram based fs": "", free_space);
		goto fail;
	}

	snprintf(cp_file, sizeof(cp_file), "cp %s %s", logfile, backup_dir);
	ret = do_command(cp_file);
	if (ret < 0) {
		fprintf(stderr, "%s: failed to backup logfile %s\n", _mod_name,
			logfile);
		goto fail;
	}

	printf("%s: logfile (%s) are backed up to (%s%s)\n",
			_mod_name, logfile, backup_dir,
			is_ramfs(backup_dir)? " a ram based fs": "");
	sync();
	fflush(stdout);
	return 0;

fail:
	fflush(stderr);
	return ret;
}

static int capture_logs(int noclk, char *timestamp, int nic_wl)
{
	char basepath[128];
	char filepath[256];
	char command[256];
	struct stat file_stat;
	FILE *fp;
	char if_name[16];
	int ch, nch;
	int error = 0;
	int ret = -1;
	int skip_dumps = 0;

	/* create the output directory for the logs */
	snprintf(basepath, sizeof(basepath), "%s/%s%s", LOG_BASE_PATH,
		CRASH_LOG_PREFIX, timestamp);
	if (mkdir(LOG_BASE_PATH, 0777) < 0 && errno != EEXIST) {
		perror("could not create dhd log folder");
		printf("%s: could not create %s folder\n", _mod_name, LOG_BASE_PATH);
		return ret;
	}
	if (mkdir(basepath, 0777) < 0 && errno != EEXIST) {
		perror("could not create dhd log folder");
		printf("%s: could not create %s folder\n", _mod_name, basepath);
		return ret;
	}

	/* dump version */
	snprintf(command, sizeof(command), "uname -a > %s/%s", basepath, LOG_UNAME_FILENAME);
	do_command(command);

	/* copy the dongle mem dump if available */
	if (stat("/tmp/mem_dump", &file_stat) == 0) {
		snprintf(command, sizeof(command), "cp /tmp/mem_dump %s/%s", basepath, LOG_DONGLE_MEM_FILENAME);
		do_command(command);
		do_command("rm -f /tmp/mem_dump");
		/* only skip dumps when noclk is set */
		skip_dumps = noclk;
	} else {
		printf("%s: dongle memory dump is not available\n", _mod_name);
	}

	/* copy the macreg dump files if available */
	snprintf(filepath, sizeof(filepath), "%s/", basepath);
	snprintf(command, sizeof(command), "ls /tmp/dump_*.txt");
	if (!do_command(command)) {
		snprintf(command, sizeof(command), "cp /tmp/dump_*.txt %s", filepath);
		do_command(command);
		snprintf(command, sizeof(command), "rm -f /tmp/dump_*.txt");
		do_command(command);
	}

	/* dump dmesg to command */
	snprintf(command, sizeof(command), "dmesg -c > %s/%s", basepath, LOG_DMESG_FILENAME);
	do_command(command);

	if (nic_wl) {
		goto exit;
	}

	/* Get the interface name */
	fp = fopen(IF_IDX_FILE_PATH, "r");
	if (!fp) {
		printf("%s: cannot open file %s\n", _mod_name, IF_IDX_FILE_PATH);
		error = -1;
		goto exit;
	} else {
		// get failed interface name
		nch = 0;
		while ((ch = fgetc(fp)) != '\n' && ch != ' ' && ch != EOF) {
			if_name[nch++] = ch;
		}
		if_name[nch] = '\0';
		fclose(fp);
	}

	printf("%s: Trap/Assert on interface %s!! noclk %d\n",
		_mod_name, if_name, noclk);

	if (skip_dumps)
		goto exit;

	/* dump console log */
	snprintf(command, sizeof(command), "dhd -i %s consoledump > %s/%s",
		if_name, basepath, LOG_CONSOLE_DUMP);
	do_command(command);

	/* SOCRAM dump to socram_dump.txt */
	snprintf(command, sizeof(command), "dhd -i %s upload %s/%s",
		if_name, basepath, LOG_SOCRAM_DUMP);
	do_command(command);

	if (noclk) {
		goto exit;
	}

	/* dump d11reg to command */
	snprintf(command, sizeof(command), "echo \"%s dump\" > %s/%s",
		if_name, basepath, LOG_D11REGDUMP_FILENAME);
	do_command(command);

	/* Make sure dongle is dead */
	snprintf(command, sizeof(command), "dhd -i %s pcie_device_trap >> %s/%s",
		if_name, basepath, LOG_D11REGDUMP_FILENAME);
	do_command(command);

	/* dump PSMr regs */
	snprintf(command, sizeof(command), "dhd -i %s dump_mac > %s/%s",
		if_name, basepath, LOG_D11REGDUMP_FILENAME);
	do_command(command);

	/* dump PSMx regs */
	snprintf(command, sizeof(command), "dhd -i %s dump_mac -x > %s/%s",
		if_name, basepath, LOG_D11REGXDUMP_FILENAME);
	do_command(command);

	/* dump sample capture log */
	snprintf(command, sizeof(command), "dhd -i %s dump_sctpl > %s/%s",
		if_name, basepath, LOG_SCTPLDUMP_FILENAME);
	do_command(command);

	/* dump SHMx sample capture log */
	snprintf(command, sizeof(command), "dhd -i %s dump_sctpl -u shmx > %s/%s",
		if_name, basepath, LOG_SCTPLXDUMP_FILENAME);
	do_command(command);

	/* dump vasip dump log */
	snprintf(command, sizeof(command), "dhd -i %s dump_svmp > %s/%s",
		if_name, basepath, LOG_VASIPDUMP_FILENAME);
	do_command(command);

exit:
	/* assure file handles are idle before creating tar */
	sync();

	/* tar/zip the logs and memory dump */
	snprintf(command, sizeof(command), "tar cf %s.tar %s%s -C %s", basepath,
		CRASH_LOG_PREFIX, timestamp, LOG_BASE_PATH);
	if (do_command(command) < 0)
		return -1;

	/* delete the raw folder */
	snprintf(command, sizeof(command), "rm -rf %s", basepath);
	do_command(command);

	snprintf(command, sizeof(command), "gzip -c %s.tar > %s.tgz", basepath,
		basepath);
	if (do_command(command) < 0)
		return -1;

	snprintf(filepath, sizeof(filepath), "%s.tgz", basepath);
	if ((stat(filepath, &file_stat) == 0) && file_stat.st_size) {
		printf("%s: log (%s) is collected, file size %d\n", _mod_name,
				filepath, file_stat.st_size);
		ret = file_stat.st_size;
	} else {
		printf("%s: log (%s) is not available or an empty file\n", _mod_name, filepath);
	}

	return ret;
}

static void sig_handler(int signo)
{
	char timestamp[64], *tmp;
	int nic_wl = 0;
	int log_size;
	int retain_logs;
	int free_space = 0;

	printf("%s: Detected firmware trap/assert !!\n", _mod_name);

	/* figure out module type. dhd or nic */
	char *val = (char *) nvram_get("kernel_mods");

	if (val && !strstr(val, "dhd")) {
		nic_wl = 1;
	}
	printf("%s: kernel_mods: %s nic_wl %d\n",
		_mod_name, (val ? val : "unset"), nic_wl);

	/* get current timestamp */
	get_timestamp(timestamp, sizeof(timestamp));
	/* reformatting time stamp string */
	tmp = timestamp;
	while (*tmp) {
		if (*tmp == '-' || *tmp == ':') {
			*tmp = '_';
		}
		tmp++;
	}
	printf("%s: Logging timestamp: %s\n", _mod_name, timestamp);

	/* retain latest crash logs under /tmp/crash_logs */
	delete_extra_crash_files(LOG_BASE_PATH, (MAX_CRASH_LOGS - 1));

	/* capture all relevant logs */
	log_size = capture_logs((signo == SIGUSR2 ? 1 : 0), timestamp, nic_wl);
	if (log_size < 0) {
		printf("%s: capture failed\n", _mod_name);
		return;
	}

	for (retain_logs = (MAX_CRASH_LOGS - 1); retain_logs >= 0; retain_logs--) {
		printf("%s: going to retain %d log(s) under backup dir (%s)\n",
				_mod_name, retain_logs, _backup_dir);
		delete_extra_crash_files(_backup_dir, retain_logs);

		if (retain_logs) {
			free_space = get_dir_free_space(_backup_dir);
			if (free_space < 0) {
				/* failed to get disk space of backup dir, skip space check */
				break;
			}

			if (log_size > free_space) {
				printf("%s: Disk space (%d) not enough for new log (%d), "
						"retain one less log\n",
						_mod_name, free_space, log_size);
			} else {
				/* free disk space is large enough for the new log */
				break;
			}
		}
	}

	/* back up the logs to persistent store */
	backup_logs(timestamp, _backup_dir);

	/* flush file system operations */
	sync();

	/* reboot or rc_restart based on configuration */
	handle_recovery(nic_wl);
}

void usage(char *progname)
{
	printf("Usage: %s <backup directory>\n", progname);
}
#endif /* __CONFIG_DHDAP__ */

static int dm_is_process_active(char * pid)
{
	char proc_path[DM_BUF_SZ];
	DIR *dir;

	snprintf(proc_path, sizeof(proc_path), "/proc/%s", pid);

	if ((dir = opendir(proc_path)) == NULL) {
		fprintf(stderr, "%s error opening %s\n", __FUNCTION__, proc_path);
		return FALSE;
	}

	closedir(dir);
	return TRUE;
}

/* trim trailing space like characters */
static void dm_trim(char *str, size_t sz)
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

static int dm_read_pid_info_file(char *path, char *dep_cmd, size_t dep_cmd_sz,
	char *process_cmd, size_t process_cmd_sz)
{
	FILE *fp;

	if ((fp = fopen(path, "r")) == 0) {
		return -1;
	}

	fgets(dep_cmd, dep_cmd_sz, fp);
	dm_trim(dep_cmd, dep_cmd_sz);

	fgets(process_cmd, process_cmd_sz, fp);
	dm_trim(process_cmd, process_cmd_sz);

	fclose(fp);

	return 0;
}

static int dm_handle_process(char *pid, unsigned int depth)
{
	char process_dm_path[DM_BUF_SZ];
	char dep_cmd[DM_BUF_SZ] = {0};
	char process_cmd[DM_BUF_SZ] = {0};
	char dep_pid[DM_BUF_SZ] = {0};

	if (depth > DM_DEPENDENCY_DEPTH_MAX) {
		return -1;
	}

	if (nvram_match("wlready", "0"))
		return FALSE;

	if (dm_is_process_active(pid)) {
		// TODO: could check timestamp if agreed to touch DM_DIR
		// at specific intervals
		// if process is active; nothing to do
		return FALSE;
	}

	fprintf(stderr, "%s process id %s does not exist will restart\n",
		__FUNCTION__, pid);

	snprintf(process_dm_path, sizeof(process_dm_path), "%s/%s", DM_DIR, pid);

	// if process is not active
	if (dm_read_pid_info_file(process_dm_path, dep_cmd, sizeof(dep_cmd),
			process_cmd, sizeof(process_cmd)) != 0) {
		return FALSE;
	}

	// if dependency is mentioned ...
	if (dep_cmd[0] && dep_cmd[0] != '\r' && dep_cmd[0] != '\n') {
		// ... find the pid of the dep_cmd
		if (dm_get_pid_of_cmd(dep_cmd, dep_pid, sizeof(dep_pid))) {
			// ... and handle the dep_cmd process first
			dm_handle_process(dep_pid, depth + 1);
		}
	}

	// remove entry of process in DM_DIR
	remove(process_dm_path);

	// restart using process_cmd
	fprintf(stderr, "%s Restarting process %s\n", __FUNCTION__, process_cmd);
	system(process_cmd);

	return TRUE;
}

static unsigned char dm_match_cmd(char *pid, char *process_cmd_needle)
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

static unsigned char dm_get_pid_of_cmd(char *dep_cmd, char *dep_pid, size_t dep_pid_sz)
{
	int ret;
	DIR *dir;
	struct dirent *de;

	if ((dir = opendir(DM_DIR)) == NULL) {
		fprintf(stderr, "%s error opening %s\n", __FUNCTION__, DM_DIR);
		return FALSE;
	}

	while ((de = readdir(dir)) != NULL) {
	// walk through entries in DM_DIR and find respective pid
		if (de->d_name[0] == '.')
			continue;
		if (dm_match_cmd(de->d_name, dep_cmd)) {
			strncpy(dep_pid, de->d_name, dep_pid_sz);
			return TRUE;
		}
	}
	if (ret != 0) {
		perror("readdir_r() error");
	}

	closedir(dir);
	return FALSE;
}

static void dm_watchdog()
{
	int ret;
	DIR *dir;
	struct dirent *de;

	if ((dir = opendir(DM_DIR)) == NULL) {
		fprintf(stderr, "%s error opening %s\n", __FUNCTION__, DM_DIR);
		return;
	}

	while ((de = readdir(dir)) != NULL) {
	// walk through entries in DM_DIR and call dm_handle_process on each
		if (de->d_name[0] == '.')
			continue;
		dm_handle_process(de->d_name, 0);
	}
	if (ret != 0) {
		perror("readdir() error");
	}
	closedir(dir);
}

int main(int argc, char **argv)
{
	_mod_name = argv[0];
	char *nv_str;
#if defined(__CONFIG_DHDAP__)
	if (argc < 2) {
		printf("%s: error no backup dir.\n", _mod_name);
		usage(_mod_name);
	} else {
		_backup_dir = argv[1];
	}

	if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
		printf("%s: error setting up signal1 handler.\n", _mod_name);
		return -1;
	}
	if (signal(SIGUSR2, sig_handler) == SIG_ERR) {
		printf("%s: error setting up signal2 handler.\n", _mod_name);
		return -1;
	}
#endif /* __CONFIG_DHDAP__ */
	nv_str = (char *)nvram_get("debug_monitor_disable_app_restart");

	if (nv_str && (nv_str[0] == '1') && (nv_str[1] == '\0')) {
		dbg_mon_disab_rstrt = TRUE;
	}
	if (daemon(1, 1) == -1) {
		printf("%s: error daemonizing.\n", _mod_name);
	} else {
		unsigned int tick = 0;
		while (1) {
			if (!dbg_mon_disab_rstrt &&
				(tick++ % DM_INTERVAL) == 0) {
				dm_watchdog();
			}
			sleep(1);
		}
	}

	return 0;
}
