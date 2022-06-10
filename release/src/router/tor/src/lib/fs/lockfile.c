/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file lockfile.c
 *
 * \brief Implements lock files to prevent two Tor processes from using the
 * same data directory at the same time.
 **/

#include "orconfig.h"
#include "lib/fs/files.h"
#include "lib/fs/lockfile.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"

#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef _WIN32
#include <windows.h>
#include <sys/locking.h>
#endif

#include <errno.h>
#include <string.h>

/** Represents a lockfile on which we hold the lock. */
struct tor_lockfile_t {
  /** Name of the file */
  char *filename;
  /** File descriptor used to hold the file open */
  int fd;
};

/** Try to get a lock on the lockfile <b>filename</b>, creating it as
 * necessary.  If someone else has the lock and <b>blocking</b> is true,
 * wait until the lock is available.  Otherwise return immediately whether
 * we succeeded or not.
 *
 * Set *<b>locked_out</b> to true if somebody else had the lock, and to false
 * otherwise.
 *
 * Return a <b>tor_lockfile_t</b> on success, NULL on failure.
 *
 * (Implementation note: because we need to fall back to fcntl on some
 *  platforms, these locks are per-process, not per-thread.  If you want
 *  to do in-process locking, use tor_mutex_t like a normal person.
 *  On Windows, when <b>blocking</b> is true, the maximum time that
 *  is actually waited is 10 seconds, after which NULL is returned
 *  and <b>locked_out</b> is set to 1.)
 */
tor_lockfile_t *
tor_lockfile_lock(const char *filename, int blocking, int *locked_out)
{
  tor_lockfile_t *result;
  int fd;
  *locked_out = 0;

  log_info(LD_FS, "Locking \"%s\"", filename);
  fd = tor_open_cloexec(filename, O_RDWR|O_CREAT|O_TRUNC, 0600);
  if (fd < 0) {
    log_warn(LD_FS,"Couldn't open \"%s\" for locking: %s", filename,
             strerror(errno));
    return NULL;
  }

#ifdef _WIN32
  _lseek(fd, 0, SEEK_SET);
  if (_locking(fd, blocking ? _LK_LOCK : _LK_NBLCK, 1) < 0) {
    if (errno != EACCES && errno != EDEADLOCK)
      log_warn(LD_FS,"Couldn't lock \"%s\": %s", filename, strerror(errno));
    else
      *locked_out = 1;
    close(fd);
    return NULL;
  }
#elif defined(HAVE_FLOCK)
  if (flock(fd, LOCK_EX|(blocking ? 0 : LOCK_NB)) < 0) {
    if (errno != EWOULDBLOCK)
      log_warn(LD_FS,"Couldn't lock \"%s\": %s", filename, strerror(errno));
    else
      *locked_out = 1;
    close(fd);
    return NULL;
  }
#else
  {
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    if (fcntl(fd, blocking ? F_SETLKW : F_SETLK, &lock) < 0) {
      if (errno != EACCES && errno != EAGAIN)
        log_warn(LD_FS, "Couldn't lock \"%s\": %s", filename, strerror(errno));
      else
        *locked_out = 1;
      close(fd);
      return NULL;
    }
  }
#endif /* defined(_WIN32) || ... */

  result = tor_malloc(sizeof(tor_lockfile_t));
  result->filename = tor_strdup(filename);
  result->fd = fd;
  return result;
}

/** Release the lock held as <b>lockfile</b>. */
void
tor_lockfile_unlock(tor_lockfile_t *lockfile)
{
  tor_assert(lockfile);

  log_info(LD_FS, "Unlocking \"%s\"", lockfile->filename);
#ifdef _WIN32
  _lseek(lockfile->fd, 0, SEEK_SET);
  if (_locking(lockfile->fd, _LK_UNLCK, 1) < 0) {
    log_warn(LD_FS,"Error unlocking \"%s\": %s", lockfile->filename,
             strerror(errno));
  }
#elif defined(HAVE_FLOCK)
  if (flock(lockfile->fd, LOCK_UN) < 0) {
    log_warn(LD_FS, "Error unlocking \"%s\": %s", lockfile->filename,
             strerror(errno));
  }
#else
  /* Closing the lockfile is sufficient. */
#endif /* defined(_WIN32) || ... */

  close(lockfile->fd);
  lockfile->fd = -1;
  tor_free(lockfile->filename);
  tor_free(lockfile);
}
