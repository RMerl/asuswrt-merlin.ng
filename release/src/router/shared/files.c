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

