/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file dir.c
 *
 * \brief Read directories, and create directories with restrictive
 * permissions.
 **/

#include "lib/fs/dir.h"
#include "lib/fs/path.h"
#include "lib/fs/userdb.h"

#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/log/win32err.h"
#include "lib/container/smartlist.h"
#include "lib/sandbox/sandbox.h"
#include "lib/malloc/malloc.h"
#include "lib/string/printf.h"
#include "lib/string/compat_string.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#else /* !(defined(_WIN32)) */
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#endif /* defined(_WIN32) */

#include <errno.h>
#include <string.h>

/** Check whether <b>dirname</b> exists and is private.  If yes return 0.
 * If <b>dirname</b> does not exist:
 *  - if <b>check</b>&CPD_CREATE, try to create it and return 0 on success.
 *  - if <b>check</b>&CPD_CHECK, and we think we can create it, return 0.
 *  - if <b>check</b>&CPD_CHECK is false, and the directory exists, return 0.
 *  - otherwise, return -1.
 * If CPD_GROUP_OK is set, then it's okay if the directory
 * is group-readable, but in all cases we create the directory mode 0700.
 * If CPD_GROUP_READ is set, existing directory behaves as CPD_GROUP_OK and
 * if the directory is created it will use mode 0750 with group read
 * permission. Group read privileges also assume execute permission
 * as norm for directories. If CPD_CHECK_MODE_ONLY is set, then we don't
 * alter the directory permissions if they are too permissive:
 * we just return -1.
 * When effective_user is not NULL, check permissions against the given user
 * and its primary group.
 */
MOCK_IMPL(int,
check_private_dir,(const char *dirname, cpd_check_t check,
                   const char *effective_user))
{
  int r;
  struct stat st;

  tor_assert(dirname);

#ifndef _WIN32
  int fd;
  const struct passwd *pw = NULL;
  uid_t running_uid;
  gid_t running_gid;

  /*
   * Goal is to harden the implementation by removing any
   * potential for race between stat() and chmod().
   * chmod() accepts filename as argument. If an attacker can move
   * the file between stat() and chmod(), a potential race exists.
   *
   * Several suggestions taken from:
   * https://developer.apple.com/library/mac/documentation/
   *     Security/Conceptual/SecureCodingGuide/Articles/RaceConditions.html
   */

  /* Open directory.
   * O_NOFOLLOW to ensure that it does not follow symbolic links */
  fd = open(sandbox_intern_string(dirname), O_NOFOLLOW);

  /* Was there an error? Maybe the directory does not exist? */
  if (fd == -1) {

    if (errno != ENOENT) {
      /* Other directory error */
      log_warn(LD_FS, "Directory %s cannot be read: %s", dirname,
               strerror(errno));
      return -1;
    }

    /* Received ENOENT: Directory does not exist */

    /* Should we create the directory? */
    if (check & CPD_CREATE) {
      log_info(LD_GENERAL, "Creating directory %s", dirname);
      if (check & CPD_GROUP_READ) {
        r = mkdir(dirname, 0750);
      } else {
        r = mkdir(dirname, 0700);
      }

      /* check for mkdir() error */
      if (r) {
        log_warn(LD_FS, "Error creating directory %s: %s", dirname,
            strerror(errno));
        return -1;
      }

      /* we just created the directory. try to open it again.
       * permissions on the directory will be checked again below.*/
      fd = open(sandbox_intern_string(dirname), O_NOFOLLOW);

      if (fd == -1) {
        log_warn(LD_FS, "Could not reopen recently created directory %s: %s",
                 dirname,
                 strerror(errno));
        return -1;
      } else {
        close(fd);
      }

    } else if (!(check & CPD_CHECK)) {
      log_warn(LD_FS, "Directory %s does not exist.", dirname);
      return -1;
    }

    /* XXXX In the case where check==CPD_CHECK, we should look at the
     * parent directory a little harder. */
    return 0;
  }

  tor_assert(fd >= 0);

  //f = tor_strdup(dirname);
  //clean_name_for_stat(f);
  log_debug(LD_FS, "stat()ing %s", dirname);
  //r = stat(sandbox_intern_string(f), &st);
  r = fstat(fd, &st);
  if (r == -1) {
      log_warn(LD_FS, "fstat() on directory %s failed.", dirname);
      close(fd);
      return -1;
  }
  //tor_free(f);

  /* check that dirname is a directory */
  if (!(st.st_mode & S_IFDIR)) {
    log_warn(LD_FS, "%s is not a directory", dirname);
    close(fd);
    return -1;
  }

  if (effective_user) {
    /* Look up the user and group information.
     * If we have a problem, bail out. */
    pw = tor_getpwnam(effective_user);
    if (pw == NULL) {
      log_warn(LD_CONFIG, "Error setting configured user: %s not found",
               effective_user);
      close(fd);
      return -1;
    }
    running_uid = pw->pw_uid;
    running_gid = pw->pw_gid;
  } else {
    running_uid = getuid();
    running_gid = getgid();
  }
  if (st.st_uid != running_uid) {
    char *process_ownername = NULL, *file_ownername = NULL;

    {
      const struct passwd *pw_running = tor_getpwuid(running_uid);
      process_ownername = pw_running ? tor_strdup(pw_running->pw_name) :
        tor_strdup("<unknown>");
    }

    {
      const struct passwd *pw_stat = tor_getpwuid(st.st_uid);
      file_ownername = pw_stat ? tor_strdup(pw_stat->pw_name) :
        tor_strdup("<unknown>");
    }

    log_warn(LD_FS, "%s is not owned by this user (%s, %d) but by "
        "%s (%d). Perhaps you are running Tor as the wrong user?",
             dirname, process_ownername, (int)running_uid,
             file_ownername, (int)st.st_uid);

    tor_free(process_ownername);
    tor_free(file_ownername);
    close(fd);
    return -1;
  }
  if ( (check & (CPD_GROUP_OK|CPD_GROUP_READ))
       && (st.st_gid != running_gid) && (st.st_gid != 0)) {
    struct group *gr;
    char *process_groupname = NULL;
    gr = getgrgid(running_gid);
    process_groupname = gr ? tor_strdup(gr->gr_name) : tor_strdup("<unknown>");
    gr = getgrgid(st.st_gid);

    log_warn(LD_FS, "%s is not owned by this group (%s, %d) but by group "
             "%s (%d).  Are you running Tor as the wrong user?",
             dirname, process_groupname, (int)running_gid,
             gr ?  gr->gr_name : "<unknown>", (int)st.st_gid);

    tor_free(process_groupname);
    close(fd);
    return -1;
  }
  unsigned unwanted_bits = 0;
  if (check & (CPD_GROUP_OK|CPD_GROUP_READ)) {
    unwanted_bits = 0027;
  } else {
    unwanted_bits = 0077;
  }
  unsigned check_bits_filter = ~0;
  if (check & CPD_RELAX_DIRMODE_CHECK) {
    check_bits_filter = 0022;
  }
  if ((st.st_mode & unwanted_bits & check_bits_filter) != 0) {
    unsigned new_mode;
    if (check & CPD_CHECK_MODE_ONLY) {
      log_warn(LD_FS, "Permissions on directory %s are too permissive.",
               dirname);
      close(fd);
      return -1;
    }
    log_warn(LD_FS, "Fixing permissions on directory %s", dirname);
    new_mode = st.st_mode;
    new_mode |= 0700; /* Owner should have rwx */
    if (check & CPD_GROUP_READ) {
      new_mode |= 0050; /* Group should have rx */
    }
    new_mode &= ~unwanted_bits; /* Clear the bits that we didn't want set...*/
    if (fchmod(fd, new_mode)) {
      log_warn(LD_FS, "Could not chmod directory %s: %s", dirname,
               strerror(errno));
      close(fd);
      return -1;
    } else {
      close(fd);
      return 0;
    }
  }
  close(fd);
#else /* defined(_WIN32) */
  /* Win32 case: we can't open() a directory. */
  (void)effective_user;

  char *f = tor_strdup(dirname);
  clean_fname_for_stat(f);
  log_debug(LD_FS, "stat()ing %s", f);
  r = stat(sandbox_intern_string(f), &st);
  tor_free(f);
  if (r) {
    if (errno != ENOENT) {
      log_warn(LD_FS, "Directory %s cannot be read: %s", dirname,
               strerror(errno));
      return -1;
    }
    if (check & CPD_CREATE) {
      log_info(LD_GENERAL, "Creating directory %s", dirname);
      r = mkdir(dirname);
      if (r) {
        log_warn(LD_FS, "Error creating directory %s: %s", dirname,
                 strerror(errno));
        return -1;
      }
    } else if (!(check & CPD_CHECK)) {
      log_warn(LD_FS, "Directory %s does not exist.", dirname);
      return -1;
    }
    return 0;
  }
  if (!(st.st_mode & S_IFDIR)) {
    log_warn(LD_FS, "%s is not a directory", dirname);
    return -1;
  }

#endif /* !defined(_WIN32) */
  return 0;
}

/** Return a new list containing the filenames in the directory <b>dirname</b>.
 * Return NULL on error or if <b>dirname</b> is not a directory.
 */
MOCK_IMPL(smartlist_t *,
tor_listdir, (const char *dirname))
{
  smartlist_t *result;
#ifdef _WIN32
  char *pattern=NULL;
  TCHAR tpattern[MAX_PATH] = {0};
  char name[MAX_PATH*2+1] = {0};
  HANDLE handle;
  WIN32_FIND_DATA findData;
  tor_asprintf(&pattern, "%s\\*", dirname);
#ifdef UNICODE
  mbstowcs(tpattern,pattern,MAX_PATH);
#else
  strlcpy(tpattern, pattern, MAX_PATH);
#endif
  if (INVALID_HANDLE_VALUE == (handle = FindFirstFile(tpattern, &findData))) {
    tor_free(pattern);
    return NULL;
  }
  result = smartlist_new();
  while (1) {
#ifdef UNICODE
    wcstombs(name,findData.cFileName,MAX_PATH);
    name[sizeof(name)-1] = '\0';
#else
    strlcpy(name,findData.cFileName,sizeof(name));
#endif /* defined(UNICODE) */
    if (strcmp(name, ".") &&
        strcmp(name, "..")) {
      smartlist_add_strdup(result, name);
    }
    if (!FindNextFile(handle, &findData)) {
      DWORD err;
      if ((err = GetLastError()) != ERROR_NO_MORE_FILES) {
        char *errstr = format_win32_error(err);
        log_warn(LD_FS, "Error reading directory '%s': %s", dirname, errstr);
        tor_free(errstr);
      }
      break;
    }
  }
  FindClose(handle);
  tor_free(pattern);
#else /* !defined(_WIN32) */
  const char *prot_dname = sandbox_intern_string(dirname);
  DIR *d;
  struct dirent *de;
  if (!(d = opendir(prot_dname)))
    return NULL;

  result = smartlist_new();
  while ((de = readdir(d))) {
    if (!strcmp(de->d_name, ".") ||
        !strcmp(de->d_name, ".."))
      continue;
    smartlist_add_strdup(result, de->d_name);
  }
  closedir(d);
#endif /* defined(_WIN32) */
  return result;
}
