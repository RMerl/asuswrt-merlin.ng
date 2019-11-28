/* source: xiolockfile.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains socats explicit locking mechanisms */

#include "xiosysincludes.h"

#include "compat.h"
#include "mytypes.h"
#include "error.h"
#include "utils.h"
#include "sysutils.h"

#include "sycls.h"

#include "xio.h"
#include "xiolockfile.h"


/* returns 0 if it could create lock; 1 if the lock exists; -1 on error */
int xiogetlock(const char *lockfile) {
   char *s;
   struct stat strat;
   int fd;
   pid_t pid;
   char pidbuf[3*sizeof(pid_t)+1];
   size_t bytes;

   if (Lstat(lockfile, &strat) == 0) {
      return 1;
   }
   switch (errno) {
   case ENOENT: break;
   default:
      Error3("Lstat(\"%s\", %p): %s", lockfile, &strat, strerror(errno));
      return -1;
   }
   /* in this moment, the file did not exist */

   if ((s = Malloc(strlen(lockfile)+8)) == NULL) {
      errno = ENOMEM;
      return -1;
   }
   strcpy(s, lockfile);
   strcat(s, ".XXXXXX");

   if ((fd = Mkstemp(s)) < 0) {
      Error2("mkstemp(\"%s\"): %s", s, strerror(errno));
      return -1;
   }
  
   pid = Getpid();
   bytes = sprintf(pidbuf, F_pid, pid);
   if (writefull(fd, pidbuf, bytes) < 0) {
      Error4("write(%d, %p, "F_Zu"): %s", fd, pidbuf, bytes, strerror(errno));
      return -1;
   }
   Close(fd);

   /* Chmod(lockfile, 0600); */
   if (Link(s, lockfile) < 0) {
      int _errno = errno;
      Error3("link(\"%s\", \"%s\"): %s", s, lockfile, strerror(errno));
      Unlink(s);
      errno = _errno;
      return -1;
   }
   Unlink(s);

   return 0;
}

int xiounlock(const char *lockfile) {
   return Unlink(lockfile);
}


/* returns 0 when it could create lock, or -1 on error */
int xiowaitlock(const char *lockfile, struct timespec *intervall) {
   int rc;
   int level = E_NOTICE;	/* first print a notice */

   while ((rc = xiogetlock(lockfile)) == 1) {
      Msg1(level, "waiting for lock \"%s\"", lockfile);
      level = E_INFO;		/* afterwards only make info */
      Nanosleep(intervall, NULL);
   }
   return rc;
}


/* returns 0 when it could obtain lock or the lock is not valid
   (lockfile==NULL), 1 if it could not obtain the lock, or -1 on error */
int xiolock(xiolock_t *lock) {
   int result;

   if (lock->lockfile == NULL) {
      return 0;
   }
   if (lock->waitlock) {
      result = xiowaitlock(lock->lockfile, &lock->intervall);
   } else {
      result = xiogetlock(lock->lockfile);
   }
   if (result == 0) {
      Info1("obtained lock \"%s\"", lock->lockfile);
   }
   return result;
}


int xiofiledroplock(xiofile_t *xfd) {
   if (xfd->tag == XIO_TAG_DUAL) {
      xiofiledroplock((xiofile_t *)xfd->dual.stream[0]);
      xiofiledroplock((xiofile_t *)xfd->dual.stream[1]);
   } else {
      xfd->stream.havelock = false;
   }
   return 0;
}
