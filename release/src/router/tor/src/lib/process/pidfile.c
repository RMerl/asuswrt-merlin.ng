/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file pidfile.c
 * \brief Record this process's PID to disk.
 **/

#include "orconfig.h"
#include "lib/process/pidfile.h"

#include "lib/log/log.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>

/** Write the current process ID, followed by NL, into <b>filename</b>.
 * Return 0 on success, -1 on failure.
 */
int
write_pidfile(const char *filename)
{
  FILE *pidfile;

  if ((pidfile = fopen(filename, "w")) == NULL) {
    log_warn(LD_FS, "Unable to open \"%s\" for writing: %s", filename,
             strerror(errno));
    return -1;
  } else {
#ifdef _WIN32
    int pid = (int)_getpid();
#else
    int pid = (int)getpid();
#endif
    int rv = 0;
    if (fprintf(pidfile, "%d\n", pid) < 0)
      rv = -1;
    if (fclose(pidfile) < 0)
      rv = -1;
    return rv;
  }
}
