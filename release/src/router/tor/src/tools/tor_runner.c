/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file tor_runner.c
 * @brief Experimental module to emulate tor_run_main() API with fork+exec
 *
 * The functions here are meant to allow the application developer to
 * use the tor_run_main() API without having to care whether Tor is
 * running in-process or out-of-process.  For in-process usage, the
 * developer can link Tor as a library and call tor_run_main(); for
 * out-of-process usage, the developer can link this library instead.
 *
 * This interface is EXPERIMENTAL; please let us know if you would like
 * to depend on it.  We don't know yet whether it will be reliable in
 * practice.
 */

/* NOTE: This module is supposed to work without the standard Tor utility
 * functions.  Don't add more dependencies!
 */

#include "feature/api/tor_api.h"
#include "feature/api/tor_api_internal.h"

#include "orconfig.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <stdlib.h>
#include <string.h>

#ifndef __GNUC__
#define __attribute__(x)
#endif

static void child(const tor_main_configuration_t *cfg)
  __attribute__((noreturn));

const char *
tor_api_get_provider_version(void)
{
  return "libtorrunner " VERSION;
}

int
tor_run_main(const tor_main_configuration_t *cfg)
{
  pid_t pid = fork();
  if (pid == 0) {
    child(cfg);
    exit(0); /* Unreachable */
  }

  pid_t stopped_pid;
  int status = 0;
  do {
    stopped_pid = waitpid(pid, &status, 0);
  } while (stopped_pid == -1);

  /* Note: these return values are not documented.  No return value is
   * documented! */

  if (stopped_pid != pid) {
    return -99999;
  }
  if (WIFSTOPPED(status)) {
    return WEXITSTATUS(status);
  }
  if (WIFSIGNALED(status)) {
    return -WTERMSIG(status);
  }

  return -999988;
}

/* circumlocution to avoid getting warned about calling calloc instead of
 * tor_calloc. */
#define real_calloc calloc
#define real_free free

static void
child(const tor_main_configuration_t *cfg)
{
  /* XXXX Close unused file descriptors. */

  char **args = real_calloc(cfg->argc + cfg->argc_owned+1, sizeof(char *));
  memcpy(args, cfg->argv, cfg->argc * sizeof(char *));
  if (cfg->argc_owned)
    memcpy(args + cfg->argc, cfg->argv_owned,
           cfg->argc_owned * sizeof(char *));

  args[cfg->argc + cfg->argc_owned] = NULL;

  int rv = execv(BINDIR "/tor", args);

  if (rv < 0) {
    real_free(args);
    exit(254);
  } else {
    abort(); /* Unreachable */
  }
}
