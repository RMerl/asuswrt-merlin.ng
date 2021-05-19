/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file freespace.c
 *
 * \brief Find the available disk space on the current volume.
 **/

#include "lib/fs/files.h"
#include "lib/cc/torint.h"

#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

#include <errno.h>
#include <string.h>

/** Return the amount of free disk space we have permission to use, in
 * bytes. Return -1 if the amount of free space can't be determined. */
int64_t
tor_get_avail_disk_space(const char *path)
{
#ifdef HAVE_STATVFS
  struct statvfs st;
  int r;
  memset(&st, 0, sizeof(st));

  r = statvfs(path, &st);
  if (r < 0)
    return -1;

  int64_t result = st.f_bavail;
  if (st.f_frsize) {
    result *= st.f_frsize;
  } else if (st.f_bsize) {
    result *= st.f_bsize;
  } else {
    return -1;
  }

  return result;
#elif defined(_WIN32)
  ULARGE_INTEGER freeBytesAvail;
  BOOL ok;

  ok = GetDiskFreeSpaceEx(path, &freeBytesAvail, NULL, NULL);
  if (!ok) {
    return -1;
  }
  return (int64_t)freeBytesAvail.QuadPart;
#else
  (void)path;
  errno = ENOSYS;
  return -1;
#endif /* defined(HAVE_STATVFS) || ... */
}
