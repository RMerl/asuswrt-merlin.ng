/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <dirent.h>
#include <bcmnvram.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/file.h>
#include <limits.h>		//PATH_MAX
#include "shutils.h"
#include "shared.h"
#if defined(RTACRH18) || defined(RT4GAC86U)
#include <limits.h>
#include <time.h>
#endif
#if defined(RTCONFIG_KNV_BACKUP) || defined(RTCONFIG_NV_BACKUP2)
#include <time.h>
#include <knv.h>
#endif

unsigned int __dir_size__ = 0;

int f_exists(const char *path)	// note: anything but a directory
{
	struct stat st;
	return (stat(path, &st) == 0) && (!S_ISDIR(st.st_mode));
}

int l_exists(const char *path)  //  link only
{
	struct stat st;
	return (lstat(path, &st) == 0) && (S_ISLNK(st.st_mode));
}

int d_exists(const char *path)	//  directory only
{
	struct stat st;
	return (stat(path, &st) == 0) && (S_ISDIR(st.st_mode));
}

unsigned long f_size(const char *path)	// 4GB-1	-1 = error
{
	struct stat st;
	if (stat(path, &st) == 0) return st.st_size;
	return (unsigned long)-1;
}

// callback function for ftw() in d_size() to calculate size of a directory, in bytes.
int sum(const char *fpath, const struct stat *sb, int typeflag)
{
	__dir_size__ += sb->st_size;
	return 0;
}

int d_size(const char *dirPath)
{
	__dir_size__ = 0;

	if (ftw(dirPath, &sum, 1))
	{
		cprintf("ftw error!\n");
		return -1;
	}

	return __dir_size__;
}

int f_read_excl(const char *path, void *buffer, int max)
{
	int f;
	int n;

	if ((f = open(path, O_RDONLY)) < 0) return -1;
	flock(f, LOCK_EX);
	n = read(f, buffer, max);
	flock(f, LOCK_UN);
	close(f);
	return n;
}

int f_read(const char *path, void *buffer, int max)
{
	int f;
	int n;

	if ((f = open(path, O_RDONLY)) < 0) return -1;
	n = read(f, buffer, max);
	close(f);
	return n;
}

int f_write_excl(const char *path, const void *buffer, int len, unsigned flags, unsigned cmode)
{
	static const char nl = '\n';
	int f, fl;
	int r = -1;
	mode_t m;

	m = umask(0);
	if (cmode == 0) cmode = 0666;
	if ((fl = open(ACTION_LOCK_FILE, O_WRONLY|O_CREAT|O_EXCL|O_TRUNC, 0600)) >= 0) { // own the lock
		if (( f = open(path, (flags & FW_APPEND) ? (O_WRONLY|O_CREAT|O_APPEND) : (O_WRONLY|O_CREAT|O_TRUNC), cmode)) >= 0) {
			flock(f, LOCK_EX);
			if ((buffer == NULL) || ((r = write(f, buffer, len)) == len)) {
				if (flags & FW_NEWLINE) {
					if (write(f, &nl, 1) == 1) ++r;
				}
			}
			flock(f, LOCK_UN);
			close(f);
		}
		close(fl);
		unlink(ACTION_LOCK_FILE);
	}
	umask(m);
	return r;
}

/**
 * Write @buffer, length len, to file specified by @path.
 * @path:
 * @buffer:
 * @len:
 * @flags:	Combination of FW_APPEND, FW_NEWLINE, FW_SILENT, etc.
 * @cmode:
 * @return:
 * 	>0:	writted length
 * 	-1:	open file error or -EINVAL
 *  -errno:	errno of write file error
 */
int f_write(const char *path, const void *buffer, int len, unsigned flags, unsigned cmode)
{
	static const char nl = '\n';
	const void *b;
	int f, ret = 0;
	size_t wlen;
	ssize_t r;
	mode_t m;

	m = umask(0);
	if (cmode == 0) cmode = 0666;
	if ((f = open(path, (flags & FW_APPEND) ? (O_WRONLY|O_CREAT|O_APPEND) : (O_WRONLY|O_CREAT|O_TRUNC), cmode)) >= 0) {
		if ((buffer == NULL)) {
			if (flags & FW_NEWLINE) {
				if (write(f, &nl, 1) == 1)
					ret = 1;
			}
		} else {
			for (b = buffer, wlen = len; wlen > 0;) {
				r = write(f, b, wlen);
				if (r < 0) {
					ret = -errno;
					if (!(flags & FW_SILENT)) {
						_dprintf("%s: Write [%s] to [%s] failed! errno %d (%s)\n",
							__func__, b, path, errno, strerror(errno));
					}
					break;
				} else {
					ret += r;
					b += r;
					wlen -= r;
				}
			}
		}
		close(f);
	} else {
		ret = -1;
	}
	umask(m);
	return ret;
}

int f_read_string(const char *path, char *buffer, int max)
{
	if (max <= 0) return -1;
	int n = f_read(path, buffer, max - 1);
	buffer[(n > 0) ? n : 0] = 0;
	return n;
}

int f_write_string(const char *path, const char *buffer, unsigned flags, unsigned cmode)
{
	return f_write(path, buffer, strlen(buffer), flags, cmode);
}

static int _f_read_alloc(const char *path, char **buffer, int max, int z)
{
	unsigned long n;

	*buffer = NULL;
	if (max >= 0) {
		if ((n = f_size(path)) != (unsigned long)-1) {
			if (n < max) max = n;
			if ((!z) && (max == 0)) return 0;
			if ((*buffer = malloc(max + z)) != NULL) {
				if ((max = f_read(path, *buffer, max)) >= 0) {
					if (z) *(*buffer + max) = 0;
					return max;
				}
				free(buffer);
			}
		}
	}
	return -1;
}

int f_read_alloc(const char *path, char **buffer, int max)
{
	return _f_read_alloc(path, buffer, max, 0);
}

int f_read_alloc_string(const char *path, char **buffer, int max)
{
	return _f_read_alloc(path, buffer, max, 1);
}

static int _f_wait_exists(const char *name, int max, int invert)
{
	while (max-- > 0) {
		if (f_exists(name) ^ invert) return 1;
		sleep(1);
	}
	return 0;
}

int f_wait_exists(const char *name, int max)
{
	return _f_wait_exists(name, max, 0);
}

int f_wait_notexists(const char *name, int max)
{
	return _f_wait_exists(name, max, 1);
}

int
check_if_dir_exist(const char *dirpath)
{
	return d_exists(dirpath);
}

int 
check_if_dir_empty(const char *dirpath)
{
	DIR *dir;
	struct dirent *dirent;
	int found=0;

	if((dir=opendir(dirpath))!=NULL) {
		while ((dirent=readdir(dir))!=NULL) {
			if(strcmp(dirent->d_name, ".") && strcmp(dirent->d_name, "..")) { 
				found=1;
				break;
			}
			
		}
		closedir(dir);
	}
	return found;
}

int
check_if_file_exist(const char *filepath)
{
/* Note: f_exists() checks not S_ISREG, but !S_ISDIR
	struct stat st;
	return (stat(path, &st) == 0) && (S_ISREG(st.st_mode));
*/
	return f_exists(filepath);
}

/* Test whether we can write to a directory.
 * @return:
 * 0		not writable
 * -1		invalid parameter
 * otherwise	writable
 */
int check_if_dir_writable(const char *dir)
{
	char tmp[PATH_MAX];
	FILE *fp;
	int ret = 0;

	if (!dir || *dir == '\0')
		return -1;

	snprintf(tmp, sizeof(tmp), "%s/.test_dir_writable", dir);
	if ((fp = fopen(tmp, "w")) != NULL) {
		fclose(fp);
		unlink(tmp);
		ret = 1;
	}

	return ret;
}


/* Serialize using fcntl() calls 
 */

/* when lock file has been re-opened by the same process,
 * it can't be closed, because it release original lock,
 * that have been set earlier. this results in file
 * descriptors leak.
 * one way to avoid it - check if the process has set the
 * lock already via /proc/locks, but it seems overkill
 * with al of related file ops and text searching. there's
 * no kernel API for that.
 * maybe need different lock kind? */
#define LET_FD_LEAK

int _file_lock(const char *dir, const char *tag)
{
	struct flock lock;
	char path[64];
	int lockfd;
	pid_t pid, err;
#ifdef LET_FD_LEAK
	pid_t lockpid;
#else
	struct stat st;
#endif

	snprintf(path, sizeof(path), "%s/%s.lock", dir, tag);

#ifndef LET_FD_LEAK
	pid = getpid();

	/* check if we already hold a lock */
	if (stat(path, &st) == 0 && !S_ISDIR(st.st_mode) && st.st_size > 0) {
		FILE *fp;
		char line[100], *ptr, *value;
		char id[sizeof("XX:XX:4294967295")];

		if ((fp = fopen("/proc/locks", "r")) == NULL)
			goto error;

		snprintf(id, sizeof(id), "%02x:%02x:%ld",
			 major(st.st_dev), minor(st.st_dev), st.st_ino);
		while ((value = fgets(line, sizeof(line), fp)) != NULL) {
			strtok_r(line, " ", &ptr);
			if ((value = strtok_r(NULL, " ", &ptr)) && strcmp(value, "POSIX") == 0 &&
			    (value = strtok_r(NULL, " ", &ptr)) && /* strcmp(value, "ADVISORY") == 0 && */
			    (value = strtok_r(NULL, " ", &ptr)) && strcmp(value, "WRITE") == 0 &&
			    (value = strtok_r(NULL, " ", &ptr)) && atoi(value) == pid &&
			    (value = strtok_r(NULL, " ", &ptr)) && strcmp(value, id) == 0)
				break;
		}
		fclose(fp);

		if (value != NULL) {
			syslog(LOG_DEBUG, "Error locking %s: %d %s", path, 0, "Already locked");
			return -1;
		}
	}
#endif

	if ((lockfd = open(path, O_CREAT | O_RDWR, 0666)) < 0)
		goto error;

#ifdef LET_FD_LEAK
	pid = getpid();

	/* check if we already hold a lock */
	if (read(lockfd, &lockpid, sizeof(pid_t)) == sizeof(pid_t) &&
	    lockpid == pid) {
		/* don't close the file here as that will release all locks */
		syslog(LOG_DEBUG, "Error locking %s: %d %s", path, 0, "Already locked");
		return -1;
	}
#endif

	memset(&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	lock.l_pid = pid;
	while (fcntl(lockfd, F_SETLKW, &lock) < 0) {
		if (errno != EINTR)
			goto close;
	}

	if (lseek(lockfd, 0, SEEK_SET) < 0 ||
	    write(lockfd, &pid, sizeof(pid_t)) < 0)
		goto close;

	return lockfd;

close:
	err = errno;
	close(lockfd);
	errno = err;
error:
	syslog(LOG_DEBUG, "Error locking %s: %d %s", path, errno, strerror(errno));
	return -1;
}

int file_lock(const char *tag)
{
	return _file_lock("/var/lock", tag);
}

int _file_unlock(int lockfd)
{
	if (lockfd < 0) {
		errno = EBADF;
		return -1;
	}

	ftruncate(lockfd, 0);
	return close(lockfd);
}

void file_unlock(int lockfd)
{
	if (_file_unlock(lockfd) < 0)
		syslog(LOG_DEBUG, "Error unlocking %d: %d %s", lockfd, errno, strerror(errno));
}

long file_copy_offset(const char *src_name, long src_offset, const char *dst_name, long dst_offset)
{
	int fsrc = -1, fdst = -1;
	int readSize, writeSize;
	unsigned char buff[(1 * 1024)];
	long retval = -2;
	int mode;

	if(!src_name || !dst_name)
		goto END;

	if(src_offset < 0)
		src_offset = 0;
	fsrc = open(src_name, O_RDONLY);
	if(fsrc < 0)
		goto END;

	mode = O_CREAT | O_WRONLY;
	if(dst_offset <= 0) {
		dst_offset = 0;
		mode |= O_TRUNC;
	}
	fdst = open(dst_name, mode);
	if(fdst < 0)
		goto END;

	lseek(fsrc, src_offset, SEEK_SET);
	lseek(fdst, dst_offset, SEEK_SET);

	retval = 0;
	do
	{
		readSize = read(fsrc, buff, sizeof(buff));
		writeSize = write(fdst, buff, readSize);

		if (writeSize != readSize) {
			retval = -1;
			break;
		}
		retval += readSize;

	} while (readSize == sizeof(buff));

END:
	if(fsrc >= 0)
		close(fsrc);
	if(fdst >= 0)
		close(fdst);

	return retval;
}

long file_copy(const char *src_file, const char *dst_file)
{
	return file_copy_offset(src_file, 0, dst_file, 0);
}

long file_append(const char *src_file, const char *dst_file)
{
	unsigned long fileSize;

	if ((fileSize = f_size(dst_file)) == (unsigned long) -1)
		fileSize = 0;	//new file

	return file_copy_offset(src_file, 0, dst_file, fileSize);
}

#if defined(RTCONFIG_KNV_BACKUP) || defined(RTCONFIG_NV_BACKUP2)
#define MAX_KNVLOG_SIZE		20 * 1024

void knv_logmessage(const char *logfile, const char *title, const char *format, ...) 
{
	FILE *file = fopen(logfile, "a+");
	if (!file) {
		printf("Failed to open knv_log %s", logfile);
		return;
	}

	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);

	if(file_size > MAX_KNVLOG_SIZE) {
		fclose(file);

		printf("truncate the knv log\n");
		file = fopen(logfile, "w");
		if (!file) {
			printf("Failed to open knv_log %s", logfile);
			return;
		}
	}


	time_t now = time(NULL);
	struct tm *local_time = localtime(&now);
	if (local_time) {
		fprintf(file, "[%04d-%02d-%02d %02d:%02d:%02d] ",
		local_time->tm_year + 1900,
		local_time->tm_mon + 1,
		local_time->tm_mday,
		local_time->tm_hour,
		local_time->tm_min,
		local_time->tm_sec);
	}

	if (title)
		fprintf(file, "%s: ", title);

	va_list args;
	va_start(args, format);
	vfprintf(file, format, args);
	va_end(args);

	fprintf(file, "\n");

	fclose(file);
}

int sort_file(const char *input_file, const char *temp_file) 
{
	char command[1024];
	snprintf(command, sizeof(command), "sort %s > %s", input_file, temp_file);

	int ret = system(command);
	if (ret != 0) {
		fprintf(stderr, "Error: Failed to sort file %s.\n", input_file);
		return -1;
	}
	return 0;
}

int compare_sorted_files(const char *sorted_file1, const char *sorted_file2) 
{
	FILE *fp1 = fopen(sorted_file1, "r");
	FILE *fp2 = fopen(sorted_file2, "r");
	int ret = 0;

	if (!fp1 || !fp2) {
		perror("Error opening sorted files");
		if (fp1) fclose(fp1);
		if (fp2) fclose(fp2);
		return -1;
	}

	char line1[1250], line2[1250];
	int read1 = fgets(line1, sizeof(line1), fp1) != NULL;
	int read2 = fgets(line2, sizeof(line2), fp2) != NULL;

	while (read1 || read2) {
		if (read1) line1[strcspn(line1, "\n")] = '\0'; // Remove newline
		if (read2) line2[strcspn(line2, "\n")] = '\0'; // Remove newline

		if (read1 && (!read2 || strcmp(line1, line2) < 0)) {
			// Line in file1 but not in file2
			printf("-- %s\n", line1);
			read1 = fgets(line1, sizeof(line1), fp1) != NULL;
			ret++;
		} else if (read2 && (!read1 || strcmp(line1, line2) > 0)) {
			// Line in file2 but not in file1
			printf("++ %s\n", line2);
			read2 = fgets(line2, sizeof(line2), fp2) != NULL;
			ret++;
		} else {
			// Line matches in both files
			read1 = fgets(line1, sizeof(line1), fp1) != NULL;
			read2 = fgets(line2, sizeof(line2), fp2) != NULL;
		}
	}

	fclose(fp1);
	fclose(fp2);

	return ret;
}

int diff_knv(char *file1, char *file2) 
{
	const char *sorted_file1 = "sorted_file1.tmp";
	const char *sorted_file2 = "sorted_file2.tmp";

	if (!file1 || !file2) {
		file1 = KNV_FILE;
		file2 = KNV_FILE_BP;
	}

	printf("diff files: %s/%s\n", file1, file2);

	// Sort both files
	if (sort_file(file1, sorted_file1) != 0 || sort_file(file2, sorted_file2) != 0) {
		return EXIT_FAILURE;
	}

	// Compare the sorted files
	if (compare_sorted_files(sorted_file1, sorted_file2) != 0) {
		return EXIT_FAILURE;
	}

	// Clean up temporary files
	remove(sorted_file1);
	remove(sorted_file2);

	return EXIT_SUCCESS;
}

#define MAX_LINE_LEN 1265
#define MAX_KEY_LEN 64
#define MAX_VAL_LEN 1200

// Error tables
const char *error_table_re_mode[][2] = 
{
	{"x_Setting", "0"},
	//{"wl0_ssid", "ASUS"},
	//{"wl1_ssid", "ASUS"},
	//{"wl2_ssid", "ASUS"},
	{"wlc0_ssid", ""},
	{"wlc1_ssid", ""},
	{"wlc2_ssid", ""},
	{"wlc3_ssid", ""},
	{"wl0_akm", ""},
	{"wl1_akm", ""},
	{"wl2_akm", ""},
	{"wl3_akm", ""},
};

const char *error_table_no_re_mode[][2] = 
{
	{"x_Setting", "0"},
	//{"wl0_ssid", "ASUS"},
	//{"wl1_ssid", "ASUS"},
	//{"wl2_ssid", "ASUS"},
	{"wl0_akm", ""},
	{"wl1_akm", ""},
	{"wl2_akm", ""},
	{"wl3_akm", ""},
};

#define RE_MODE_TABLE_SIZE (sizeof(error_table_re_mode) / sizeof(error_table_re_mode[0]))
#define CAP_MODE_TABLE_SIZE (sizeof(error_table_no_re_mode) / sizeof(error_table_no_re_mode[0]))

// ignore those run time nvram when knv restore
const char *ignored_keys[] = {
	"ahs_bhc_log_ext",
	"amas_lanctrl",
	"service_ready",
	"buildinfo",
	"httpd_handle_request",
	"httpd_handle_request_fromapp",
	"lan_state_t",
	"ntp_server_tried",
	"reload_svc_radio",
	"setting_update_time",
	"start_service_ready",
	"success_start_service",
	"sys_uptime_now",
	"sys_uptime_prev",
	"wait_httpd",
	"wl0_sel_bw",
	"wl0_sel_channel",
	"wl0_sel_nctrlsb",
	"wl1_sel_bw",
	"wl1_sel_channel",
	"wl1_sel_nctrlsb",
	"wl2_sel_bw",
	"wl2_sel_channel",
	"wl2_sel_nctrlsb",
	"wl3_sel_bw",
	"wl3_sel_channel",
	"wl3_sel_nctrlsb",
	"wlnband_list",
	"wlready",
	"wps_device_pin"
};

int is_ignored_key(const char *key) {
	size_t ignored_count = sizeof(ignored_keys) / sizeof(ignored_keys[0]);
	for (size_t i = 0; i < ignored_count; i++) {
		if (strcmp(key, ignored_keys[i]) == 0) {
			return 1;
		}
	}
	return 0;
}

void restore_nvram(FILE *config) 
{
	char line[MAX_LINE_LEN];
	char key[MAX_KEY_LEN], value[MAX_VAL_LEN];

	while (fgets(line, sizeof(line), config)) {
		// Remove the newline character if it exists
		line[strcspn(line, "\r\n")] = '\0';

		key[0] = '\0';
		value[0] = '\0';

		if (sscanf(line, "%64[^=]=%1199[^\"]", key, value) >= 1) {
			if (*key && !is_ignored_key(key))
				nvram_set(key, value);
		}
	}
}

int check_config(FILE *config, const char *error_table[][2], size_t table_size) 
{
	char line[MAX_LINE_LEN];
	int error_count = 0;

	while (fgets(line, sizeof(line), config)) {
		char key[MAX_KEY_LEN], value[MAX_VAL_LEN];

		if (sscanf(line, "%64[^=]=%1199[^\n]", key, value) == 2) {
			for (size_t i = 0; i < table_size; ++i) {
				if (strcmp(key, error_table[i][0]) == 0 && strcmp(value, error_table[i][1]) == 0) {
					printf("nv_san error: %s=%s\n", key, value);
					++error_count;
					knv_logmessage(KNV_LOG, "nv_sanity", "[err%d]: %s=%s\n", error_count, key, value);
				}
			}
		}
	}

	return error_count;
}

int check_knv_errors(const char *config)
{
	int is_re = nvram_get_int("re_mode");
	const char *(*error_table)[2] =  is_re ? error_table_re_mode : error_table_no_re_mode;
	size_t table_size = is_re==1 ? RE_MODE_TABLE_SIZE : CAP_MODE_TABLE_SIZE;
	FILE *config_A = fopen(config, "r");

	if (!config_A) {
		perror("Error opening config");
		return -1;
	}

	rewind(config_A);
	int error_count_A = check_config(config_A, error_table, table_size);

	fclose(config_A);

	printf("%s, errors in %s:%d\n", __func__, config, error_count_A);

	return error_count_A;
}

int chk_nv_sanity(int is_re, int action) 
{
	const char *(*error_table)[2] =  is_re ? error_table_re_mode : error_table_no_re_mode;
	size_t table_size = is_re==1 ? RE_MODE_TABLE_SIZE : CAP_MODE_TABLE_SIZE;

	FILE *config_A = fopen(KNV_FILE, "r");
	FILE *config_B = fopen(KNV_FILE_BP, "r");

	if (!config_A) {
		perror("Error opening KNV_FILE");
		return -1;
	}

	if (!config_B) {
		perror("KNV_BP file does not exist. Shall be still in defaults ?\n");
		fclose(config_A);
		return -1;
	}

	rewind(config_A);
	int error_count_A = check_config(config_A, error_table, table_size);
	fclose(config_A);

	if (error_count_A == 0 && action == 1) {
		printf("Warning: No errors in knv, but nvram contains unexpected errors !\n");
		// restore_nvram ? TBD.
	}

	if (action) {
		printf("%s: [re:%d], err in knv:%d\n", __func__, is_re, error_count_A);
		knv_logmessage(KNV_LOG, "nv_sanity", "[re:%d], err in knv:%d\n", is_re, error_count_A);
	} else {
		printf("%s[chk]: re(%d), err in knv:%d\n", __func__, is_re, error_count_A);
	}

	if (error_count_A > 0 && action == 1) {

		rewind(config_B);
		int error_count_B = check_config(config_B, error_table, table_size);

		printf("%s: [re:%d], err in knv_bp:%d\n", __func__, is_re, error_count_B);
		knv_logmessage(KNV_LOG, "nv_sanity", "[re:%d], err in knv_bp:%d\n", is_re, error_count_B);

		if (error_count_B == 0) {
			// Copy config_B(knv_bp) to config_A(knv)
			fclose(config_B);

			config_B = fopen(KNV_FILE_BP, "r");
			if (!config_B) {
				perror("Error reopening config_B");
				return -1;
			}

			FILE *config_A_write = fopen(KNV_FILE, "w");
			if (!config_A_write) {
				perror("Error opening knv for writing");
				fclose(config_B);
				return -1;
			}

			char line[MAX_LINE_LEN];
			while (fgets(line, sizeof(line), config_B)) {
				fputs(line, config_A_write);
			}

			fclose(config_A_write);
			fclose(config_B);

			// Restore NVRAM from knv_bp
			config_B = fopen(KNV_FILE_BP, "r");
			if (!config_B) {
				perror("Error reopening config_B for NVRAM restore");
				return -1;
			}
			printf("%s, restore nvram from knv_bp\n", __func__);
			knv_logmessage(KNV_LOG, "nv_sanity", "restore nvram from knv_bp\n");

			restore_nvram(config_B);
			fclose(config_B);

			nvram_commit();
		} else {
			printf("%s: warning: Errors found in both knv and knv_bp !\n", __func__);
			knv_logmessage(KNV_LOG, "nv_sanity", "warning: errors found in both knv and knv_bp !\n");

			fclose(config_B);
		}
	}

	knv_logmessage(KNV_LOG, "nv_sanity", "---------\n");

	return error_count_A;
}
#endif	// defined(RTCONFIG_KNV_BACKUP) || defined(RTCONFIG_NV_BACKUP2)

