/* source: sycls.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* explicit system call and C library trace function, for those who miss strace
 */

#include "config.h"
#include "xioconfig.h"	/* what features are enabled */

#include "sysincludes.h"

#include "mytypes.h"
#include "compat.h"
#include "errno.h"

#include "error.h"
#include "filan.h"
#include "utils.h"
#include "sysutils.h"
#include "sycls.h"


#if WITH_SYCLS

mode_t Umask(mode_t mask) {
   mode_t result;
   int _errno;
   Debug1("umask("F_mode")", mask);
   result = umask(mask);
   _errno = errno;
   Debug1("umask() -> "F_mode, result);
   errno = _errno;
   return result;
}

#endif /* WITH_SYCLS */


int Open(const char *pathname, int flags, mode_t mode) {
   int result, _errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug3("open(\"%s\", 0%o, 0%03o)", pathname, flags, mode);
#endif /* WITH_SYCLS */
   result = open(pathname, flags, mode);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Info4("open(\"%s\", 0%o, 0%03o) -> %d", pathname, flags, mode, result);
#endif /* WITH_SYCLS */
   errno = _errno;
   return result;
}

#if WITH_SYCLS

int Creat(const char *pathname, mode_t mode) {
   int result, _errno;
   Debug2("creat(\"%s\", 0%03o)", pathname, mode);
   result = creat(pathname, mode);
   _errno = errno;
   Info3("creat(\"%s\", 0%03o) -> %d", pathname, mode, result);
   errno = _errno;
   return result;
}

off_t Lseek(int fildes, off_t offset, int whence) {
   int _errno;
   off_t result;
   Debug3("lseek(%d, "F_off", %d)", fildes, offset, whence);
   result = lseek(fildes, offset, whence);
   _errno = errno;
   Debug1("lseek() -> "F_off, result);
   errno = _errno;
   return result;
}

#if HAVE_LSEEK64
off64_t Lseek64(int fildes, off64_t offset, int whence) {
   int _errno;
   off64_t result;
   Debug3("lseek64(%d, "F_off64", %d)", fildes, offset, whence);
   result = lseek64(fildes, offset, whence);
   _errno = errno;
   Debug1("lseek64() -> "F_off64, result);
   errno = _errno;
   return result;
}
#endif /* HAVE_LSEEK64 */

pid_t Getpid(void) {
   pid_t result;
   int _errno;
   Debug("getpid()");
   result = getpid();
   _errno = errno;
   Debug1("getpid() -> "F_pid, result);
   errno = _errno;
   return result;
}

pid_t Getppid(void) {
   pid_t result;
   int _errno;
   Debug("getppid()");
   result = getppid();
   _errno = errno;
   Debug1("getppid() -> "F_pid, result);
   errno = _errno;
   return result;
}

pid_t Getpgrp(void) {
   pid_t result;
   int _errno;
   Debug("getpgrp()");
   result = getpgrp();
   _errno = errno;
   Debug1("getpgrp() -> "F_pid, result);
   errno = _errno;
   return result;
}

#if 0	/* does not compile for FreeBSD */
/* setpgrp() is not BSD compatible, needs setpgid(..., ...) instead */
int Setpgrp(void) {
   int result, _errno;
   Debug("setpgrp()");
   result = setpgrp();
   _errno = errno;
   Debug1("setpgrp() -> %d", result);
   errno = _errno;
   return result;
}
#endif

#if HAVE_GETPGID
int Getpgid(pid_t pid) {
   pid_t result;
   int _errno;
   Debug1("getpgid("F_pid")", pid);
   result = getpgid(pid);
   _errno = errno;
   Debug1("getpgid() -> "F_pid, result);
   errno = _errno;
   return result;
}
#endif

int Setpgid(pid_t pid, pid_t pgid) {
   int result, _errno;
   Debug2("setpgid("F_pid", "F_pid")", pid, pgid);
   result = setpgid(pid, pgid);
   _errno = errno;
   Debug1("setpgid() -> %d", result);
   errno = _errno;
   return result;
}

pid_t Tcgetpgrp(int fd) {
   int result, _errno;
   Debug1("tcgetpgrp(%d)", fd);
   result = tcgetpgrp(fd);
   _errno = errno;
   Debug1("tcgetpgrp() -> %d", result);
   errno = _errno;
   return result;
}

int Tcsetpgrp(int fd, pid_t pgrpid) {
   int result, _errno;
   Debug2("tcsetpgrp(%d, "F_pid")", fd, pgrpid);
   result = tcsetpgrp(fd, pgrpid);
   _errno = errno;
   Debug1("tcsetpgrp() -> %d", result);
   errno = _errno;
   return result;
}

#if HAVE_GETSID
pid_t Getsid(pid_t pid) {
   int result, _errno;
   Debug1("getsid("F_pid")", pid);
   result = getsid(pid);
   _errno = errno;
   Debug1("getsid() -> "F_pid, result);
   errno = _errno;
   return result;
}
#endif

pid_t Setsid(void) {
   int result, _errno;
   Debug("setsid()");
   result = setsid();
   _errno = errno;
   Debug1("setsid() -> "F_pid, result);
   errno = _errno;
   return result;
}

uid_t Getuid(void) {
   uid_t result;
   int _errno;
   Debug("getuid()");
   result = getuid();
   _errno = errno;
   Debug1("getuid() -> "F_uid, result);
   errno = _errno;
   return result;
}

uid_t Geteuid(void) {
   uid_t result;
   int _errno;
   Debug("geteuid()");
   result = geteuid();
   _errno = errno;
   Debug1("geteuid() -> "F_uid, result);
   errno = _errno;
   return result;
}

int Setuid(uid_t uid) {
   int result, _errno;
   Debug1("setuid("F_uid")", uid);
   result = setuid(uid);
   _errno = errno;
   Debug1("setuid() -> %d", result);
   errno = _errno;
   return result;
}

gid_t Getgid(void) {
   gid_t result;
   int _errno;
   Debug("getgid()");
   result = getgid();
   _errno = errno;
   Debug1("getgid() -> "F_gid, result);
   errno = _errno;
   return result;
}

gid_t Getegid(void) {
   gid_t result;
   int _errno;
   Debug("getegid()");
   result = getegid();
   _errno = errno;
   Debug1("getegid() -> "F_gid, result);
   errno = _errno;
   return result;
}

int Setgid(gid_t gid) {
   int result, _errno;
   Debug1("setgid("F_gid")", gid);
   result = setgid(gid);
   _errno = errno;
   Debug1("setgid() -> %d", result);
   errno = _errno;
   return result;
}

int Initgroups(const char *user, gid_t group) {
   int result, _errno;
   Debug2("initgroups(\"%s\", "F_gid")", user, group);
   result = initgroups(user, group);
   _errno = errno;
   Debug1("initgroups() -> %d", result);
   errno = _errno;
   return result;
}

int Getgroups(int size, gid_t list[]) {
   int result, _errno;
   Debug2("getgroups(%d, "F_gid",...)", size, list[0]);
   result = getgroups(size, list);
   _errno = errno;
   Debug1("getgroups() -> %d", result);
   errno = _errno;
   return result;
}

#if HAVE_SETGROUPS
int Setgroups(size_t size, const gid_t *list) {
   int result, _errno;
   switch (size) {
   case 0: Debug1("setgroups("F_Zu", [])", size); break;;
   case 1: Debug2("setgroups("F_Zu", ["F_gid"])", size, list[0]); break;;
   case 2: Debug3("setgroups("F_Zu", ["F_gid","F_gid"])", size, list[0], list[1]); break;;
   default: Debug3("setgroups("F_Zu", ["F_gid","F_gid",...])", size, list[0], list[1]); break;;
   }
   result = setgroups(size, list);
   _errno = errno;
   Debug1("setgroups() -> %d", result);
   errno = _errno;
   return result;
}
#endif

#if HAVE_GETGROUPLIST
int Getgrouplist(const char *user, gid_t group, gid_t *groups, int *ngroups) {
   int n = *ngroups, result;
   Debug4("getgrouplist(\"%s\", "F_gid", %p, [%d])", user, group, groups, n);
   result = getgrouplist(user, group, groups, ngroups);
   switch (Min(n,*ngroups)) {
   case 0:  Debug2("getgrouplist(,, [], [%d]) -> %d", *ngroups, result); break;
   case 1:  Debug3("getgrouplist(,, ["F_gid"], [%d]) -> %d", groups[0], *ngroups, result); break;
   case 2:  Debug4("getgrouplist(,, ["F_gid","F_gid"], [%d]) -> %d", groups[0], groups[1], *ngroups, result); break;
   default: Debug4("getgrouplist(,, ["F_gid","F_gid",...], [%d]) -> %d", groups[0], groups[1], *ngroups, result); break;
   }
   return result;
}
#endif

int Chdir(const char *path) {
   int result, _errno;
   Debug1("chdir(\"%s\")", path);
   result = chdir(path);
   _errno = errno;
   Debug1("chdir() -> %d", result);
   errno = _errno;
   return result;
}

int Chroot(const char *path) {
   int result, _errno;
   Debug1("chroot(\"%s\")", path);
   result = chroot(path);
   _errno = errno;
   Debug1("chroot() -> %d", result);
   errno = _errno;
   return result;
}

int Gettimeofday(struct timeval *tv, struct timezone *tz) {
   int result, _errno;
#if WITH_MSGLEVEL <= E_DEBUG
   if (tz) {
      Debug3("gettimeofday(%p, {%d,%d})",
	     tv, tz->tz_minuteswest, tz->tz_dsttime);
   } else {
      Debug1("gettimeofday(%p, NULL)", tv);
   }
#endif /* WITH_MSGLEVEL <= E_DEBUG */
   result = gettimeofday(tv, tz);
   _errno = errno;
#if WITH_MSGLEVEL <= E_DEBUG
   if (tz) {
      Debug5("gettimeofday({%ld,%ld}, {%d,%d}) -> %d",
             tv->tv_sec, tv->tv_usec, tz->tz_minuteswest, tz->tz_dsttime,
	     result);
   } else {
      Debug3("gettimeofday({%ld,%ld},) -> %d",
	     tv->tv_sec, tv->tv_usec, result);
   }
#endif /* WITH_MSGLEVEL <= E_DEBUG */
   errno = _errno;
   return result;
}

int Mknod(const char *pathname, mode_t mode, dev_t dev) {
   int result, _errno;
   Debug3("mknod(\"%s\", 0%o, "F_dev")", pathname, mode, dev);
   result = mknod(pathname, mode, dev);
   _errno = errno;
   Debug1("mknod() -> %d", result);
   errno = _errno;
   return result;
}

int Mkfifo(const char *pathname, mode_t mode) {
   int result, _errno;
   Debug2("mkfifo(\"%s\", 0%o)", pathname, mode);
   result = mkfifo(pathname, mode);
   _errno = errno;
   Debug1("mkfifo() -> %d", result);
   errno = _errno;
   return result;
}

static void prtstat(const char *func, struct stat *buf, int result) {
   char txt[256], *t = txt;

   t += sprintf(t, "%s(, {"F_dev","F_st_ino","F_mode","F_st_nlink","F_uid","F_gid,
		func, buf->st_dev, buf->st_ino,
		buf->st_mode, buf->st_nlink, buf->st_uid, buf->st_gid);
#if HAVE_ST_RDEV
   t += sprintf(t, ","F_dev, buf->st_rdev);
#endif
   t += sprintf(t, ","F_st_size, buf->st_size);
#if HAVE_ST_BLKSIZE
   t += sprintf(t, ","F_st_blksize, buf->st_blksize);
#endif
#if HAVE_ST_BLOCKS
   t += sprintf(t, ","F_st_blocks, buf->st_blocks);
#endif
   sprintf(t, ",...}) -> %d", result);
   Debug(txt);
}

#if defined(HAVE_STAT64) || defined(HAVE_FSTAT64) || defined(HAVE_LSTAT64)
static void prtstat64(const char *func, struct stat64 *buf, int result) {
   char txt[256], *t = txt;

   if (result < 0) {
      sprintf(t, "%s(, {}) -> %d", func, result);
   } else {
   t += sprintf(t, "%s(, {"F_dev","F_st64_ino","F_mode","F_st_nlink","F_uid","F_gid,
		func, buf->st_dev, buf->st_ino,
		buf->st_mode, buf->st_nlink, buf->st_uid, buf->st_gid);
#if HAVE_ST_RDEV
   t += sprintf(t, ","F_dev, buf->st_rdev);
#endif
   t += sprintf(t, ","F_st64_size, buf->st_size);
#if HAVE_ST_BLKSIZE
   t += sprintf(t, ","F_st_blksize, buf->st_blksize);
#endif
#if HAVE_ST_BLOCKS
   t += sprintf(t, ","F_st64_blocks, buf->st_blocks);
#endif
   sprintf(t, ",...}) -> %d", result);
   }
   Debug(txt);
}
#endif /* defined(HAVE_STAT64) || defined(HAVE_FSTAT64) || defined(HAVE_LSTAT64) */

int Stat(const char *file_name, struct stat *buf) {
   int result, _errno;
   Debug2("stat(%s, %p)", file_name, buf);
   result = stat(file_name, buf);
   _errno = errno;
   prtstat("stat", buf, result);
   errno = _errno;
   return result;
}

#if HAVE_STAT64
int Stat64(const char *file_name, struct stat64 *buf) {
   int result, _errno;
   Debug2("stat64(%s, %p)", file_name, buf);
   result = stat64(file_name, buf);
   _errno = errno;
   prtstat64("stat64", buf, result);
   errno = _errno;
   return result;
}
#endif /* HAVE_STAT64 */

int Fstat(int filedes, struct stat *buf) {
   int result, _errno;
   Debug2("fstat(%d, %p)", filedes, buf);
   result = fstat(filedes, buf);
   _errno = errno;
   prtstat("fstat", buf, result);
   errno = _errno;
   return result;
}

#if HAVE_FSTAT64
int Fstat64(int filedes, struct stat64 *buf) {
   int result, _errno;
   Debug2("fstat64(%d, %p)", filedes, buf);
   result = fstat64(filedes, buf);
   _errno = errno;
   prtstat64("fstat64", buf, result);
   errno = _errno;
   return result;
}
#endif /* HAVE_FSTAT64 */

int Lstat(const char *file_name, struct stat *buf) {
   int result, _errno;
   Debug2("lstat(%s, %p)", file_name, buf);
   result = lstat(file_name, buf);
   _errno = errno;
   prtstat("lstat", buf, result);
   errno = _errno;
   return result;
}

#if HAVE_LSTAT64
int Lstat64(const char *file_name, struct stat64 *buf) {
   int result, _errno;
   Debug2("lstat64(%s, %p)", file_name, buf);
   result = lstat64(file_name, buf);
   _errno = errno;
   prtstat64("lstat64", buf, result);
   errno = _errno;
   return result;
}
#endif /* HAVE_LSTAT64 */

int Dup(int oldfd) {
   int newfd, _errno;
   Debug1("dup(%d)", oldfd);
   newfd = dup(oldfd);
   _errno = errno;
   Info2("dup(%d) -> %d", oldfd, newfd);
   errno = _errno;
   return newfd;
}

int Dup2(int oldfd, int newfd) {
   int result, _errno;
   Debug2("dup2(%d, %d)", oldfd, newfd);
   result = dup2(oldfd, newfd);
   _errno = errno;
   Info3("dup2(%d, %d) -> %d", oldfd, newfd, result);
   errno = _errno;
   return result;
}

int Pipe(int filedes[2]) {
   int result, _errno;
   Debug1("pipe(%p)", filedes);
   result = pipe(filedes);
   _errno = errno;
   Info3("pipe({%d,%d}) -> %d", filedes[0], filedes[1], result);
   errno = _errno;
   return result;
}

#endif /* WITH_SYCLS */

ssize_t Read(int fd, void *buf, size_t count) {
   ssize_t result;
   int _errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug3("read(%d, %p, "F_Zu")", fd, buf, count);
#endif /* WITH_SYCLS */
   result = read(fd, buf, count);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug1("read -> "F_Zd, result);
#endif /* WITH_SYCLS */
   errno = _errno;
   return result;
}

ssize_t Write(int fd, const void *buf, size_t count) {
   ssize_t result;
   int _errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug3("write(%d, %p, "F_Zu")", fd, buf, count);
#endif /* WITH_SYCLS */
   result = write(fd, buf, count);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug1("write -> "F_Zd, result);
#endif /* WITH_SYCLS */
   errno = _errno;
   return result;
}

int Fcntl(int fd, int cmd) {
   int result, _errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug2("fcntl(%d, %d)", fd, cmd);
#endif /* WITH_SYCLS */
   result = fcntl(fd, cmd);
   if (!diag_in_handler) diag_flush();
   _errno = errno;
#if WITH_SYCLS
   Debug1("fcntl() -> %d", result);
#endif /* WITH_SYCLS */
   errno = _errno;
   return result;
}

int Fcntl_l(int fd, int cmd, long arg) {
   int result, _errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug3("fcntl(%d, %d, %ld)", fd, cmd, arg);
#endif /* WITH_SYCLS */
   result = fcntl(fd, cmd, arg);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug1("fcntl() -> %d", result);
#endif /* WITH_SYCLS */
   errno = _errno;
   return result;
}

int Fcntl_lock(int fd, int cmd, struct flock *l) {
   int result, _errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug7("fcntl(%d, %d, {type=%hd,whence=%hd,start="F_off",len="F_off",pid="F_pid"})",
	  fd, cmd, l->l_type, l->l_whence, l->l_start, l->l_len, l->l_pid);
#endif /* WITH_SYCLS */
   result = fcntl(fd, cmd, l);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug1("fcntl() -> %d", result);
#endif /* WITH_SYCLS */
   errno = _errno;
   return result;
}

#if WITH_SYCLS

int Ftruncate(int fd, off_t length) {
   int retval, _errno;
   Debug2("ftruncate(%d, "F_off")", fd, length);
   retval = ftruncate(fd, length);
   _errno = errno;
   Debug1("ftruncate() -> %d", retval);
   errno = _errno;
   return retval;
}

#if HAVE_FTRUNCATE64
int Ftruncate64(int fd, off64_t length) {
   int retval, _errno;
   Debug2("ftruncate64(%d, "F_off64")", fd, length);
   retval = ftruncate64(fd, length);
   _errno = errno;
   Debug1("ftruncate64() -> %d", retval);
   errno = _errno;
   return retval;
}
#endif /* HAVE_FTRUNCATE64 */

#endif /* WITH_SYCLS */

#if HAVE_FLOCK
int Flock(int fd, int operation) {
   int retval, _errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug2("flock(%d, %d)", fd, operation);
#endif /* WITH_SYCLS */
   retval = flock(fd, operation);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug1("flock() -> %d", retval);
#endif /* WITH_SYCLS */
   errno = _errno;
   return retval;
}
#endif /* HAVE_FLOCK */

int Ioctl(int d, int request, void *argp) {
   int retval, _errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   if (argp > (void *)0x10000) {	/* fuzzy...*/
      Debug4("ioctl(%d, 0x%x, %p{%lu})", d, request, argp, *(unsigned long *)argp);
   } else {
      Debug3("ioctl(%d, 0x%x, 0x%p)", d, request, argp);
   }
#endif /* WITH_SYCLS */
   retval = ioctl(d, request, argp);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug1("ioctl() -> %d", retval);
#endif /* WITH_SYCLS */
   errno = _errno;
   return retval;
}

int Ioctl_int(int d, int request, int arg) {
   int retval, _errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug3("ioctl(%d, 0x%x, %d)", d, request, arg);
#endif /* WITH_SYCLS */
   retval = ioctl(d, request, arg);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug1("ioctl() -> %d", retval);
#endif /* WITH_SYCLS */
   errno = _errno;
   return retval;
}

#if WITH_SYCLS

int Close(int fd) {
   int retval, _errno;
   Info1("close(%d)", fd);
   retval = close(fd);
   _errno = errno;
   Debug1("close()  -> %d", retval);
   errno = _errno;
   return retval;
}

int Fchown(int fd, uid_t owner, gid_t group) {
   int retval, _errno;
   Debug3("fchown(%d, "F_uid", "F_gid")", fd, owner, group);
   retval = fchown(fd, owner, group);
   _errno = errno;
   Debug1("fchown() -> %d", retval);
   errno = _errno;
   return retval;
}

int Fchmod(int fd, mode_t mode) {
   int retval, _errno;
   Debug2("fchmod(%d, 0%o)", fd, mode);
   retval = fchmod(fd, mode);
   _errno = errno;
   Debug1("fchmod()  -> %d", retval);
   errno = _errno;
   return retval;
}

int Unlink(const char *pathname) {
   int retval, _errno;
   Debug1("unlink(\"%s\")", pathname);
   retval = unlink(pathname);
   _errno = errno;
   Debug1("unlink()  -> %d", retval);
   errno = _errno;
   return retval;
}

int Symlink(const char *oldpath, const char *newpath) {
   int retval, _errno;
   Debug2("symlink(\"%s\", \"%s\")", oldpath, newpath);
   retval = symlink(oldpath, newpath);
   _errno = errno;
   Debug1("symlink()  -> %d", retval);
   errno = _errno;
   return retval;
}

int Readlink(const char *path, char *buf, size_t bufsiz) {
   int retval, _errno;
   Debug3("readlink(\"%s\", %p, "F_Zu")", path, buf, bufsiz);
   retval = readlink(path, buf, bufsiz);
   _errno = errno;
   Debug1("readlink() -> %d", retval);
   errno = _errno;
   return retval;
}

int Chown(const char *path, uid_t owner, gid_t group) {
   int retval, _errno;
   Debug3("chown(\"%s\", "F_uid", "F_gid")", path, owner, group);
   retval = chown(path, owner, group);
   _errno = errno;
   Debug1("chown()  -> %d", retval);
   errno = _errno;
   return retval;
}

int Chmod(const char *path, mode_t mode) {
   int retval, _errno;
   Debug2("chmod(\"%s\", 0%o)", path, mode);
   retval = chmod(path, mode);
   _errno = errno;
   Debug1("chmod()  -> %d", retval);
   errno = _errno;
   return retval;
}

#endif /* WITH_SYCLS */

#if HAVE_POLL
/* we only show the first struct pollfd; hope this is enough for most cases. */
int Poll(struct pollfd *ufds, unsigned int nfds, int timeout) {
   int _errno, result;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   if (nfds == 4) {
      Debug10("poll({%d,0x%02hx,}{%d,0x%02hx,}{%d,0x%02hx,}{%d,0x%02hx,}, %u, %d)",
	      ufds[0].fd, ufds[0].events, ufds[1].fd, ufds[1].events,
	      ufds[2].fd, ufds[2].events, ufds[3].fd, ufds[3].events,
	      nfds, timeout);
   } else {
      Debug4("poll({%d,0x%02hx,}, , %u, %d)", ufds[0].fd, ufds[0].events, nfds, timeout);
   }
#endif /* WITH_SYCLS */
   result = poll(ufds, nfds, timeout);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   if (nfds == 4) {
      Debug5("poll(, {,,0x%02hx}{,,0x%02hx}{,,0x%02hx}{,,0x%02hx}) -> %d",
	     ufds[0].revents, ufds[1].revents, ufds[2].revents, ufds[3].revents, result);
   } else {
      Debug2("poll(, {,,0x%02hx}) -> %d", ufds[0].revents, result);
   }
#endif /* WITH_SYCLS */
   errno = _errno;
   return result;
}
#endif /* HAVE_POLL */

/* we only show the first word of the fd_set's; hope this is enough for most
   cases. */
int Select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	   struct timeval *timeout) {
   int result, _errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
#if HAVE_FDS_BITS
   Debug7("select(%d, &0x%lx, &0x%lx, &0x%lx, %s%lu."F_tv_usec")",
	  n, readfds?readfds->fds_bits[0]:0, writefds?writefds->fds_bits[0]:0,
	  exceptfds?exceptfds->fds_bits[0]:0,
	  timeout?"&":"NULL/", timeout?timeout->tv_sec:0,
	  timeout?timeout->tv_usec:0);
#else
   Debug7("select(%d, &0x%lx, &0x%lx, &0x%lx, %s%lu.%06u)",
	  n, readfds?readfds->__fds_bits[0]:0, writefds?writefds->__fds_bits[0]:0,
	  exceptfds?exceptfds->__fds_bits[0]:0,
	  timeout?"&":"NULL/", timeout?timeout->tv_sec:0,
	  timeout?timeout->tv_usec:0);
#endif
#endif /* WITH_SYCLS */
   result = select(n, readfds, writefds, exceptfds, timeout);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
#if HAVE_FDS_BITS
   Debug7("select -> (, 0x%lx, 0x%lx, 0x%lx, %s%lu."F_tv_usec"), %d",
	  readfds?readfds->fds_bits[0]:0, writefds?writefds->fds_bits[0]:0,
	  exceptfds?exceptfds->fds_bits[0]:0,
	  timeout?"&":"NULL/", timeout?timeout->tv_sec:0,
	  timeout?timeout->tv_usec:0, result);
#else
   Debug7("select -> (, 0x%lx, 0x%lx, 0x%lx, %s%lu.%06u), %d",
	  readfds?readfds->__fds_bits[0]:0, writefds?writefds->__fds_bits[0]:0,
	  exceptfds?exceptfds->__fds_bits[0]:0,
	  timeout?"&":"NULL/", timeout?timeout->tv_sec:0,
	  timeout?timeout->tv_usec:0, result);
#endif
#endif /* WITH_SYCLS */
   errno = _errno;

   return result;
}

#if WITH_SYCLS

pid_t Fork(void) {
   pid_t pid;
   int _errno;
   Debug("fork()");
   pid = fork();
   _errno = errno;
   Debug1("fork() -> %d", pid);	/* attention: called twice! */
   errno = _errno;
   return pid;
}

#endif /* WITH_SYCLS */

pid_t Waitpid(pid_t pid, int *status, int options) {
   int _errno;
   pid_t retval;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug3("waitpid("F_pid", %p, %d)", pid, status, options);
#endif /* WITH_SYCLS */
   retval = waitpid(pid, status, options);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug2("waitpid(, {%d}, ) -> "F_pid, *status, retval);
#endif /* WITH_SYCLS */
   errno = _errno;
   return retval;
}

#if WITH_SYCLS

sighandler_t Signal(int signum, sighandler_t handler) {
   int _errno;
   sighandler_t retval;
   Debug2("signal(%d, %p)", signum, handler);
   retval = signal(signum, handler);
   _errno = errno;
   Debug1("signal() -> %p", retval);
   errno = _errno;
   return retval;
}

#if HAVE_SIGACTION
int Sigaction(int signum, const struct sigaction *act,
	      struct sigaction *oldact) {
   int retval;
   Debug3("sigaction(%d, %p, %p)", signum, act, oldact);
   retval = sigaction(signum, act, oldact);
   Debug1("sigaction() -> %d", retval);
   return retval;
}
#endif /* HAVE_SIGACTION */

int Sigprocmask(int how, const sigset_t *set, sigset_t *oset) {
   int retval;
   Debug3("sigprocmask(%d, %p, %p)", how, set, oset);
   retval = sigprocmask(how, set, oset);
   Debug1("sigprocmask() -> %d", retval);
   return retval;
}

unsigned int Alarm(unsigned int seconds) {
   unsigned int retval;
   Debug1("alarm(%u)", seconds);
   retval = alarm(seconds);
   Debug1("alarm() -> %u", retval);
   return retval;
}

int Kill(pid_t pid, int sig) {
   int retval, _errno;
   Debug2("kill("F_pid", %d)", pid, sig);
   retval = kill(pid, sig);
   _errno = errno;
   Debug1("kill() -> %d", retval);
   errno = _errno;
   return retval;
}

int Link(const char *oldpath, const char *newpath) {
   int retval, _errno;
   Debug2("link(\"%s\", \"%s\")", oldpath, newpath);
   retval = link(oldpath, newpath);
   _errno = errno;
   Debug1("link() -> %d", retval);
   errno = _errno;
   return retval;
}

int Execvp(const char *file, char *const argv[]) {
   int result, _errno;
   if (argv[1] == NULL)
      Debug2("execvp(\"%s\", \"%s\")", file, argv[0]);
   else if (argv[2] == NULL)
      Debug3("execvp(\"%s\", \"%s\" \"%s\")", file, argv[0], argv[1]);
   else if (argv[3] == NULL)
      Debug4("execvp(\"%s\", \"%s\" \"%s\" \"%s\")", file, argv[0], argv[1], argv[2]);
   else if (argv[4] == NULL)
      Debug5("execvp(\"%s\", \"%s\" \"%s\" \"%s\" \"%s\")", file, argv[0], argv[1], argv[2], argv[3]);
   else if (argv[5] == NULL)
      Debug6("execvp(\"%s\", \"%s\" \"%s\" \"%s\" \"%s\" \"%s\")", file, argv[0], argv[1], argv[2], argv[3], argv[4]);
   else
      Debug6("execvp(\"%s\", \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" ...)", file, argv[0], argv[1], argv[2], argv[3], argv[4]);

   result = execvp(file, argv);
   _errno = errno;
   Debug1("execvp() -> %d", result);
   errno = _errno;
   return result;
}

#endif /* WITH_SYCLS */

int System(const char *string) {
   int result, _errno;
#if WITH_SYCLS
   Debug1("system(\"%s\")", string);
#endif /* WITH_SYCLS */
   diag_immediate_exit = 1;
   result = system(string);
   diag_immediate_exit = 0;
   _errno = errno;
#if WITH_SYCLS
   Debug1("system() -> %d", result);
#endif /* WITH_SYCLS */
   errno = _errno;
   return result;
}

#if WITH_SYCLS

int Socketpair(int d, int type, int protocol, int sv[2]) {
   int result, _errno;
   Debug4("socketpair(%d, %d, %d, %p)", d, type, protocol, sv);
   result = socketpair(d, type, protocol, sv);
   _errno = errno;
   Info6("socketpair(%d, %d, %d, {%d,%d}) -> %d", d, type, protocol, sv[0], sv[1], result);
   errno = _errno;
   return result;
}

#if _WITH_SOCKET
int Socket(int domain, int type, int protocol) {
   int result, _errno;
   Debug3("socket(%d, %d, %d)", domain, type, protocol);
   result = socket(domain, type, protocol);
   _errno = errno;
   Info4("socket(%d, %d, %d) -> %d", domain, type, protocol, result);
   errno = _errno;
   return result;
}
#endif /* _WITH_SOCKET */

#if _WITH_SOCKET
int Bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen) {
   int result, _errno;
   char infobuff[256];

   sockaddr_info(my_addr, addrlen, infobuff, sizeof(infobuff));
   Debug3("bind(%d, %s, "F_socklen")", sockfd, infobuff, addrlen);
   result = bind(sockfd, my_addr, addrlen);
   _errno = errno;
   Debug1("bind() -> %d", result);
   errno = _errno;
   return result;
}
#endif /* _WITH_SOCKET */

#endif /* WITH_SYCLS */

#if _WITH_SOCKET
int Connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen) {
   int result, _errno;
   char infobuff[256];

   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   /*sockaddr_info(serv_addr, infobuff, sizeof(infobuff));
   Debug3("connect(%d, %s, "F_Zd")", sockfd, infobuff, addrlen);*/
#if 0
   Debug18("connect(%d,{0x%02x%02x%02x%02x %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x}, "F_Zd")",
	   sockfd,
	   ((unsigned char *)serv_addr)[0],  ((unsigned char *)serv_addr)[1],
	   ((unsigned char *)serv_addr)[2],  ((unsigned char *)serv_addr)[3], 
	   ((unsigned char *)serv_addr)[4],  ((unsigned char *)serv_addr)[5], 
	   ((unsigned char *)serv_addr)[6],  ((unsigned char *)serv_addr)[7], 
	   ((unsigned char *)serv_addr)[8],  ((unsigned char *)serv_addr)[9], 
	   ((unsigned char *)serv_addr)[10], ((unsigned char *)serv_addr)[11], 
	   ((unsigned char *)serv_addr)[12], ((unsigned char *)serv_addr)[13], 
	   ((unsigned char *)serv_addr)[14], ((unsigned char *)serv_addr)[15], 
	   addrlen);
#else
   Debug4("connect(%d, {%d,%s}, "F_socklen")",
	  sockfd, serv_addr->sa_family,
	  sockaddr_info(serv_addr, addrlen, infobuff, sizeof(infobuff)),
	  addrlen);
#endif
#endif /* WITH_SYCLS */
   result = connect(sockfd, serv_addr, addrlen);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug1("connect() -> %d", result);
#endif /* WITH_SYCLS */
   errno = _errno;
   return result;
}
#endif /* _WITH_SOCKET */

#if WITH_SYCLS

#if _WITH_SOCKET
int Listen(int s, int backlog) {
   int result, _errno;
   Debug2("listen(%d, %d)", s, backlog);
   result = listen(s, backlog);
   _errno = errno;
   Debug1("listen() -> %d", result);
   errno = _errno;
   return result;
}
#endif /* _WITH_SOCKET */

#endif /* WITH_SYCLS */

#if _WITH_SOCKET
/* don't forget to handle EINTR when using Accept() ! */
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
   int result, _errno;
   fd_set accept_s;
   if (!diag_in_handler) diag_flush();
   FD_ZERO(&accept_s);
   FD_SET(s, &accept_s);
   if (diag_select(s+1, &accept_s, NULL, NULL, NULL) < 0) {
      return -1;
   }
#if WITH_SYCLS
   Debug3("accept(%d, %p, %p)", s, addr, addrlen);
#endif /* WITH_SYCLS */
   result = accept(s, addr, addrlen);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   if (result >= 0) {
      char infobuff[256];
      sockaddr_info(addr, *addrlen, infobuff, sizeof(infobuff));
      Info5("accept(%d, {%d, %s}, "F_socklen") -> %d", s,
	    addr->sa_family,
	    sockaddr_info(addr, *addrlen, infobuff, sizeof(infobuff)),
	    *addrlen, result);
   } else {
      Debug1("accept(,,) -> %d", result);
   }
#endif /* WITH_SYCLS */
   errno = _errno;
   return result;
}
#endif /* _WITH_SOCKET */

#if WITH_SYCLS

#if _WITH_SOCKET
int Getsockname(int s, struct sockaddr *name, socklen_t *namelen) {
   int result, _errno;
   char infobuff[256];

   Debug4("getsockname(%d, %p, %p{"F_socklen"})", s, name, namelen, *namelen);
   result = getsockname(s, name, namelen);
   _errno = errno;
   /*Debug2("getsockname(,, {"F_socklen"}) -> %d",
      *namelen, result);*/
   Debug3("getsockname(, {%s}, {"F_socklen"}) -> %d",
	  sockaddr_info(name, *namelen, infobuff, sizeof(infobuff)),
	  *namelen, result);
   errno = _errno;
   return result;
}
#endif /* _WITH_SOCKET */

#if _WITH_SOCKET
int Getpeername(int s, struct sockaddr *name, socklen_t *namelen) {
   int result, _errno;
   char infobuff[256];

   Debug4("getpeername(%d, %p, %p{"F_socklen"})", s, name, namelen, *namelen);
   result = getpeername(s, name, namelen);
   _errno = errno;
   sockaddr_info(name, *namelen, infobuff, sizeof(infobuff));
   Debug3("getpeername(, {%s}, {"F_socklen"}) -> %d",
	  infobuff, *namelen, result);
   errno = _errno;
   return result;
}
#endif /* _WITH_SOCKET */

#if _WITH_SOCKET
int Getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen) {
   int result, _errno;
   Debug5("getsockopt(%d, %d, %d, %p, {"F_socklen"})",
	  s, level, optname, optval, *optlen);
   result = getsockopt(s, level, optname, optval, optlen);
   _errno = errno;
   Debug3("getsockopt() -> (,,, 0x%08x, %d), %d",
	  *(int *)optval, *optlen, result);
   errno = _errno;
   return result;
}
#endif /* _WITH_SOCKET */

#if _WITH_SOCKET
int Setsockopt(int s, int level, int optname, const void *optval, int optlen) {
   int result, _errno;
   if (optlen <= sizeof(int)) {
      Debug5("setsockopt(%d, %d, %d, {0x%x}, %d)",
       s, level, optname, *(unsigned int *)optval, optlen);
   } else {
      Debug6("setsockopt(%d, %d, %d, {0x%08x,%08x}, %d)",
	     s, level, optname,
	     ((unsigned int *)optval)[0], ((unsigned int *)optval)[1],
	     optlen);
   }
   result = setsockopt(s, level, optname, optval, optlen);
   _errno = errno;
   Debug1("setsockopt() -> %d", result);
   errno = _errno;
   return result;
}
#endif /* _WITH_SOCKET */

#endif /* WITH_SYCLS */

#if _WITH_SOCKET
int Recv(int s, void *buf, size_t len, int flags) {
   int retval, _errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug4("recv(%d, %p, "F_Zu", %d)", s, buf, len, flags);
#endif /* WITH_SYCLS */
   retval = recv(s, buf, len, flags);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug1("recv() -> %d", retval);
#endif /* WITH_SYCLS */
   errno = _errno;
   return retval;
}
#endif /* _WITH_SOCKET */

#if _WITH_SOCKET
int Recvfrom(int s, void *buf, size_t len, int flags, struct sockaddr *from,
	     socklen_t *fromlen) {
   int retval, _errno;
   char infobuff[256];
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug6("recvfrom(%d, %p, "F_Zu", %d, %p, "F_socklen")",
	  s, buf, len, flags, from, *fromlen);
#endif /* WITH_SYCLS */
   retval = recvfrom(s, buf, len, flags, from, fromlen);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   if (from) {
      Debug4("recvfrom(,,,, {%d,%s}, "F_socklen") -> %d",
	     from->sa_family,
	     sockaddr_info(from, *fromlen, infobuff, sizeof(infobuff)),
	     *fromlen, retval);
   } else {
      Debug1("recvfrom(,,,, NULL, NULL) -> %d", retval);
   }
#endif /* WITH_SYCLS */
   errno = _errno;
   return retval;
}
#endif /* _WITH_SOCKET */

#if _WITH_SOCKET
int Recvmsg(int s, struct msghdr *msgh, int flags) {
   int retval, _errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   char infobuff[256];
#if defined(HAVE_STRUCT_MSGHDR_MSGCONTROL) && defined(HAVE_STRUCT_MSGHDR_MSGCONTROLLEN) && defined(HAVE_STRUCT_MSGHDR_MSGFLAGS)
   Debug10("recvmsg(%d, %p{%p,%u,%p,"F_Zu",%p,"F_Zu",%d}, %d)", s, msgh,
	  msgh->msg_name, msgh->msg_namelen,  msgh->msg_iov,  msgh->msg_iovlen,
	  msgh->msg_control,  msgh->msg_controllen,  msgh->msg_flags, flags);
#else
   Debug7("recvmsg(%d, %p{%p,%u,%p,%u}, %d)", s, msgh,
	  msgh->msg_name, msgh->msg_namelen,  msgh->msg_iov,  msgh->msg_iovlen,
	  flags);
#endif
#endif /* WITH_SYCLS */
   retval = recvmsg(s, msgh, flags);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
#if defined(HAVE_STRUCT_MSGHDR_MSGCONTROLLEN)
   Debug5("recvmsg(, {%s,%u,,"F_Zu",,"F_Zu",}, ) -> %d",
	  msgh->msg_name?sockaddr_info(msgh->msg_name, msgh->msg_namelen, infobuff, sizeof(infobuff)):"NULL",
	  msgh->msg_namelen, msgh->msg_iovlen, msgh->msg_controllen,
	  retval);
#else
   Debug4("recvmsg(, {%s,%u,,%u,,}, ) -> %d",
	  msgh->msg_name?sockaddr_info(msgh->msg_name, msgh->msg_namelen, infobuff, sizeof(infobuff)):"NULL",
	  msgh->msg_namelen, msgh->msg_iovlen,
	  retval);
#endif
#endif /* WITH_SYCLS */
   errno = _errno;
   return retval;
}
#endif /* _WITH_SOCKET */

#if _WITH_SOCKET
int Send(int s, const void *mesg, size_t len, int flags) {
   int retval, _errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug5("send(%d, %p[%08x...], "F_Zu", %d)",
	  s, mesg, ntohl(*(unsigned long *)mesg), len, flags);
#endif /* WITH_SYCLS */
   retval = send(s, mesg, len, flags);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug1("send() -> %d", retval);
#endif /* WITH_SYCLS */
   errno = _errno;
   return retval;
}
#endif /* _WITH_SOCKET */

#if _WITH_SOCKET
int Sendto(int s, const void *mesg, size_t len, int flags,
	   const struct sockaddr *to, socklen_t tolen) {
   int retval, _errno;
   char infobuff[256];

   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   sockaddr_info(to, tolen, infobuff, sizeof(infobuff));
   Debug7("sendto(%d, %p[%08x...], "F_Zu", %d, {%s}, %d)",
	  s, mesg, htonl(*(unsigned long *)mesg), len, flags, infobuff, tolen);
#endif /* WITH_SYCLS */
   retval = sendto(s, mesg, len, flags, to, tolen);
   _errno = errno;
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug1("sendto() -> %d", retval);
#endif /* WITH_SYCLS */
   errno = _errno;
   return retval;
}
#endif /* _WITH_SOCKET */

#if WITH_SYCLS

#if _WITH_SOCKET
int Shutdown(int fd, int how) {
   int retval, _errno;
   Info2("shutdown(%d, %d)", fd, how);
   retval = shutdown(fd, how);
   _errno = errno;
   Debug1("shutdown()  -> %d", retval);
   errno = _errno;
   return retval;
}
#endif /* _WITH_SOCKET */

unsigned int Sleep(unsigned int seconds) {
   unsigned int retval;
   Debug1("sleep(%u)", seconds);
   retval = sleep(seconds);
   Debug1("sleep() -> %u", retval);
   return retval;
}

/* obsolete by POSIX.1-2001 */
void Usleep(unsigned long usec) {
   Debug1("usleep(%lu)", usec);
   usleep(usec);
   Debug("usleep() ->");
   return;
}

#if HAVE_NANOSLEEP
unsigned int Nanosleep(const struct timespec *req, struct timespec *rem) {
   int retval, _errno;
   Debug3("nanosleep({"F_time",%ld},%p)", req->tv_sec, req->tv_nsec, rem);
   retval = nanosleep(req, rem);
   _errno = errno;
   if (rem) {
      Debug3("nanosleep(,{"F_time",%ld}) -> %d",
	     rem->tv_sec, rem->tv_nsec, retval);
   } else {
      Debug1("nanosleep() -> %d", retval);
   }
   errno = _errno;
   return retval;
}
#endif /* HAVE_NANOSLEEP */

int Pause(void) {
   int retval, _errno;
   Debug("pause()");
   retval = pause();
   _errno = errno;
   Debug1("pause() -> %d", retval);
   errno = _errno;
   return retval;
}

#if WITH_IP4 || WITH_IP6
struct hostent *Gethostbyname(const char *name) {
   struct hostent *hent;
   Debug1("gethostbyname(\"%s\")", name);
   hent = gethostbyname(name);
   if (hent == NULL) {
      Debug("gethostbyname() -> NULL");
   } else {
      Debug4("gethostbyname() -> %d.%d.%d.%d",
	     ((unsigned char *)hent->h_addr_list[0])[0],
	     ((unsigned char *)hent->h_addr_list[0])[1],
	     ((unsigned char *)hent->h_addr_list[0])[2],
	     ((unsigned char *)hent->h_addr_list[0])[3]);
   }
   return hent;
}
#endif /* WITH_IP4 || WITH_IP6 */

#if (_WITH_IP4 || _WITH_IP6) && HAVE_GETADDRINFO
int Getaddrinfo(const char *node, const char *service,
		const struct addrinfo *hints, struct addrinfo **res) {
   int result;
   Debug15("getaddrinfo(%s%s%s, %s%s%s, {%d,%d,%d,%d,"F_socklen",%p,%p,%p}, %p)",
	   node?"\"":"", node?node:"NULL", node?"\"":"",
	   service?"\"":"", service?service:"NULL", service?"\"":"",
	   hints->ai_flags, hints->ai_family, hints->ai_socktype,
	   hints->ai_protocol, hints->ai_addrlen, hints->ai_addr,
	   hints->ai_canonname, hints->ai_next, res);
   result = getaddrinfo(node, service, hints, res);
   if (result == 0) {
      char sockbuff[256];
      sockaddr_info((*res)->ai_addr, hints->ai_addrlen, sockbuff, sizeof(sockbuff));
      Debug2("getaddrinfo(,,,{{%s, %s}) -> 0",
	     sockbuff,
	    (*res)->ai_canonname?(*res)->ai_canonname:"");
   } else {
      Debug2("getaddrinfo(,,,{%p}) -> %d", *res, result);
   }
   return result;
}
#endif /* (_WITH_IP4 || _WITH_IP6) && HAVE_GETADDRINFO */

#if (WITH_IP4 || WITH_IP6) && HAVE_PROTOTYPE_LIB_getipnodebyname
struct hostent *Getipnodebyname(const char *name, int af, int flags,
                                int *error_num) {
   struct hostent *result;
   Debug4("getipnodebyname(\"%s\", %d, %d, %p)", name, af, flags, error_num);
   result = getipnodebyname(name, af, flags, error_num);
   if (result == NULL) {
      Debug1("getipnodebyname(,,, {%d}) -> NULL", *error_num);
   } else {
      Debug4("getipnodebyname() -> {\"%s\", %p, %d, %d, ???}",
	     result->h_name, result->h_aliases, result->h_addrtype,
	     result->h_length);
   }
   return result;
}
#endif /* (WITH_IP4 || WITH_IP6) && HAVE_PROTOTYPE_LIB_getipnodebyname */

void *Malloc(size_t size) {
   void *result;
   Debug1("malloc("F_Zd")", size);
   result = malloc(size);
   Debug1("malloc() -> %p", result);
   if (result == NULL) {
      Error1("malloc("F_Zd"): out of memory", size);
      return NULL;
   }
   return result;
}

void *Calloc(size_t nmemb, size_t size) {
   void *result;
   Debug2("calloc("F_Zd", "F_Zd")", nmemb, size);
   result = calloc(nmemb, size);
   Debug1("calloc() -> %p", result);
   if (result == NULL) {
      Error2("calloc("F_Zd", "F_Zd"): out of memory", nmemb, size);
      return NULL;
   }
   return result;
}

void *Realloc(void *ptr, size_t size) {
   void *result;
   Debug2("realloc(%p, "F_Zd")", ptr, size);
   result = realloc(ptr, size);
   Debug1("realloc() -> %p", result);
   if (result == NULL) {
      Error2("realloc(%p, "F_Zd"): out of memory", ptr, size);
      return NULL;
   }
   return result;
}

#if _WITH_TERMIOS
int Tcgetattr(int fd, struct termios *termios_p) {
   int i, result, _errno;
   char chars[5*NCCS], *cp = chars;

   Debug2("tcgetattr(%d, %p)", fd, termios_p);
   result = tcgetattr(fd, termios_p);
   _errno = errno;

   for (i = 0; i < NCCS-1; ++i) {
      cp += sprintf(cp, "%02x,", termios_p->c_cc[i]);
   }
   sprintf(cp, "%02x", termios_p->c_cc[i]);
   Debug6("tcgetattr(, {%08x,%08x,%08x,%08x,%s}) -> %d",
	  termios_p->c_iflag, termios_p->c_oflag,
	  termios_p->c_cflag, termios_p->c_lflag, 
	  chars, result);
   errno = _errno;
   return result;
}
#endif /* _WITH_TERMIOS */

#if _WITH_TERMIOS
int Tcsetattr(int fd, int optional_actions, struct termios *termios_p) {
   int i, result, _errno;
   char chars[5*NCCS], *cp = chars;

   for (i = 0; i < NCCS-1; ++i) {
      cp += sprintf(cp, "%02x,", termios_p->c_cc[i]);
   }
   sprintf(cp, "%02x", termios_p->c_cc[i]);
   Debug7("tcsetattr(%d, %d, {%08x,%08x,%08x,%08x,%s})", fd, optional_actions,
	  termios_p->c_iflag, termios_p->c_oflag,
	  termios_p->c_cflag, termios_p->c_lflag, chars);
   result = tcsetattr(fd, optional_actions, termios_p);
   _errno = errno;
   Debug1("tcsetattr() -> %d", result);
   errno = _errno;
   return result;
}
#endif /* _WITH_TERMIOS */

char *Ttyname(int fd) {
   char *result;
   int _errno;
   Debug1("ttyname(%d)", fd);
   result = ttyname(fd);
   _errno = errno;
   if (result)
      Debug1("ttyname() -> %s", result);
   else
      Debug("ttyname() -> NULL");
   errno = _errno;
   return result;
}

int Isatty(int fd) {
   int result, _errno;
   Debug1("isatty(%d)", fd);
   result = isatty(fd);
   _errno = errno;
   Debug1("isatty() -> %d", result);
   errno = _errno;
   return result;
}

#if HAVE_OPENPTY
int Openpty(int *ptyfd, int *ttyfd, char *ptyname, struct termios *termp,
	    struct winsize *winp) {
   int result, _errno;
   Debug5("openpty(%p, %p, %p, %p, %p)", ptyfd, ttyfd, ptyname, termp, winp);
   result = openpty(ptyfd, ttyfd, ptyname, termp, winp);
   _errno = errno;
   Info4("openpty({%d}, {%d}, {\"%s\"},,) -> %d", *ptyfd, *ttyfd, ptyname,
	  result);
   errno = _errno;
   return result;
}
#endif /* HAVE_OPENPTY */

#if HAVE_GRANTPT
int Grantpt(int fd) {
   int result, _errno;
   Debug1("grantpt(%d)", fd);
   result = grantpt(fd);
   _errno = errno;
   Debug1("grantpt() -> %d", result);
   errno = _errno;
   return result;
}
#endif /* HAVE_GRANTPT */

#if HAVE_UNLOCKPT
int Unlockpt(int fd) {
   int result, _errno;
   Debug1("unlockpt(%d)", fd);
   result = unlockpt(fd);
   _errno = errno;
   Debug1("unlockpt() -> %d", result);
   errno = _errno;
   return result;
}
#endif /* HAVE_UNLOCKPT */

#if HAVE_PROTOTYPE_LIB_ptsname	/* AIX, not Linux */
char *Ptsname(int fd) {
   char *result;
   int _errno;
   Debug1("ptsname(%d)", fd);
   result = ptsname(fd);
   _errno = errno;
   if (result)
      Debug1("ptsname() -> %s", result);
   else
      Debug("ptsname() -> NULL");
   errno = _errno;
   return result;
}
#endif /* HAVE_PROTOTYPE_LIB_ptsname */

int Uname(struct utsname *buf) {
   int result, _errno;
   Debug1("uname(%p)", buf);
   result = uname(buf);
   _errno = errno;
#if UNAME_DOMAINNAME
   Debug6("uname({%s, %s, %s, %s, %s, %s})",
	  buf->sysname, buf->nodename, buf->release,
	  buf->version, buf->machine, buf->domainname);
#else
   Debug5("uname({%s, %s, %s, %s, %s})",
	  buf->sysname, buf->nodename, buf->release,
	  buf->version, buf->machine);
#endif
   errno = _errno;
   return result;
}

int Gethostname(char *name, size_t len) {
   int result, _errno;
   Debug2("gethostname(%p, "F_Zu")", name, len);
   result = gethostname(name, len);
   _errno = errno;
   Debug2("gethostname(\"%s\", ) -> %d", name, result);
   errno = _errno;
   return result;
}

/* due to Linux docu, it does not set errno */
int Atexit(void (*func)(void)) {
   int result;
   Debug1("atexit(%p)", func);
   result = atexit(func);
   Debug1("atexit() -> %d", result);
   return result;
}

#endif /* WITH_SYCLS */

void Exit(int status) {
   if (!diag_in_handler) diag_flush();
#if WITH_SYCLS
   Debug1("exit(%d)", status);
#endif /* WITH_SYCLS */
   exit(status);
}

#if WITH_SYCLS

void Abort(void) {
   Debug("abort()");
   abort();
}

int Mkstemp(char *template) {
   int result, _errno;
   Debug1("mkstemp(\"%s\")", template);
   result = mkstemp(template);
   _errno = errno;
   Info2("mkstemp({%s}) -> %d", template, result);
   errno = _errno;
   return result;
}

int Setenv(const char *name, const char *value, int overwrite) {
   int result, _errno;
   Debug3("setenv(\"%s\", \"%s\", %d)", name, value, overwrite);
   result = setenv(name, value, overwrite);
   _errno = errno;
   Debug1("setenv() -> %d", result);
   errno = _errno;
   return result;
}

#if HAVE_UNSETENV
/* on Linux it returns int but on FreeBSD void.
   we do not expect many errors, so we take void which works on all systems. */
void Unsetenv(const char *name) {
   int _errno;
   Debug1("unsetenv(\"%s\")", name);
   unsetenv(name);
   _errno = errno;
   Debug("unsetenv() ->");
   errno = _errno;
   return;
}
#endif

#if WITH_READLINE

char *Readline(const char *prompt) {
   char *result;

   if (prompt) {
      Debug1("readline(\"%s\")", prompt);
   } else {
      Debug("readline(NULL)");
   }
   result = readline(prompt);
   if (result) {
      Debug("readline() -> \"...\"");
   } else {
      Debug("readline() -> NULL");
   }
   return result;
}

void Using_history(void) {
   Debug("using_history()");
   using_history();
   Debug("using_history() ->");
}

int Read_history(const char *filename) {
   int result;

   if (filename) {
      Debug1("read_history(\"%s\")", filename);
   } else {
      Debug("read_history(NULL)");
   }
   result = read_history(filename);
   if (result) {
      Debug1("read_history() -> %d", result);
   } else {
      Debug("read_history() -> 0");
   }
   return result;
}

int Write_history(const char *filename) {
   int result;

   if (filename) {
      Debug1("write_history(\"%s\")", filename);
   } else {
      Debug("write_history(NULL)");
   }
   result = write_history(filename);
   if (result) {
      Debug1("write_history() -> %d", result);
   } else {
      Debug("write_history() -> 0");
   }
   return result;
}

int Append_history(int nelements, const char *filename) {
   int result;

   if (filename) {
      Debug2("append_history(%d, \"%s\")", nelements, filename);
   } else {
      Debug1("append_history(%d, NULL)", nelements);
   }
   result = append_history(nelements, filename);
   if (result) {
      Debug1("append_history() -> %d", result);
   } else {
      Debug("append_history() -> 0");
   }
   return result;
}

int Where_history(void) {
   int result;

   Debug("where_history()");
   result = where_history();
   Debug1("where_history() -> %d", result);
   return result;
}

void Add_history(const char *string) {
   Debug1("add_history(\"%s\")", string);
   add_history(string);
   Debug("add_history() ->");
}

#endif /* WITH_READLINE */

#endif /* WITH_SYCLS */
