/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-run-session.c - run a child process in its own session
 *
 * Copyright © 2003-2006 Red Hat, Inc.
 * Copyright © 2006 Thiago Macieira <thiago@kde.org>
 * Copyright © 2011-2012 Nokia Corporation
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <config.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "dbus/dbus.h"

#define MAX_ADDR_LEN 512
#define PIPE_READ_END  0
#define PIPE_WRITE_END 1

/* PROCESSES
 *
 * If you are in a shell and run "dbus-run-session myapp", here is what
 * happens (compare and contrast with dbus-launch):
 *
 * shell
 *   \- dbus-run-session myapp
 *      \- dbus-daemon --nofork --print-address --session
 *      \- myapp
 *
 * All processes are long-running.
 *
 * When myapp exits, dbus-run-session kills dbus-daemon and terminates.
 *
 * If dbus-daemon exits, dbus-run-session warns and continues to run.
 *
 * PIPES
 *
 * dbus-daemon --print-address -> bus_address_pipe -> d-r-s
 */

static const char me[] = "dbus-run-session";

static void
usage (int ecode)
{
  fprintf (stderr,
      "%s [OPTIONS] [--] PROGRAM [ARGUMENTS]\n"
      "%s --version\n"
      "%s --help\n"
      "\n"
      "Options:\n"
      "--dbus-daemon=BINARY       run BINARY instead of dbus-daemon\n"
      "--config-file=FILENAME     pass to dbus-daemon instead of --session\n"
      "\n",
      me, me, me);
  exit (ecode);
}

static void
version (void)
{
  printf ("%s %s\n"
          "Copyright (C) 2003-2006 Red Hat, Inc.\n"
          "Copyright (C) 2006 Thiago Macieira\n"
          "Copyright © 2011-2012 Nokia Corporation\n"
          "\n"
          "This is free software; see the source for copying conditions.\n"
          "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
          me, VERSION);
  exit (0);
}

static void
oom (void)
{
  fprintf (stderr, "%s: out of memory\n", me);
  exit (1);
}

typedef enum
{
  READ_STATUS_OK,    /**< Read succeeded */
  READ_STATUS_ERROR, /**< Some kind of error */
  READ_STATUS_EOF    /**< EOF returned */
} ReadStatus;

static ReadStatus
read_line (int        fd,
           char      *buf,
           size_t     maxlen)
{
  size_t bytes = 0;
  ReadStatus retval;

  memset (buf, '\0', maxlen);
  maxlen -= 1; /* ensure nul term */

  retval = READ_STATUS_OK;

  while (1)
    {
      ssize_t chunk;
      size_t to_read;

    again:
      to_read = maxlen - bytes;

      if (to_read == 0)
        break;

      chunk = read (fd,
                    buf + bytes,
                    to_read);
      if (chunk < 0 && errno == EINTR)
        goto again;

      if (chunk < 0)
        {
          retval = READ_STATUS_ERROR;
          break;
        }
      else if (chunk == 0)
        {
          retval = READ_STATUS_EOF;
          break; /* EOF */
        }
      else /* chunk > 0 */
        bytes += chunk;
    }

  if (retval == READ_STATUS_EOF &&
      bytes > 0)
    retval = READ_STATUS_OK;

  /* whack newline */
  if (retval != READ_STATUS_ERROR &&
      bytes > 0 &&
      buf[bytes-1] == '\n')
    buf[bytes-1] = '\0';

  return retval;
}

static void
exec_dbus_daemon (const char *dbus_daemon,
                  int         bus_address_pipe[2],
                  const char *config_file)
{
  /* Child process, which execs dbus-daemon or dies trying */
#define MAX_FD_LEN 64
  char write_address_fd_as_string[MAX_FD_LEN];

  close (bus_address_pipe[PIPE_READ_END]);

  sprintf (write_address_fd_as_string, "%d", bus_address_pipe[PIPE_WRITE_END]);

  execlp (dbus_daemon,
          dbus_daemon,
          "--nofork",
          "--print-address", write_address_fd_as_string,
          config_file ? "--config-file" : "--session",
          config_file, /* has to be last in this varargs list */
          NULL);

  fprintf (stderr, "%s: failed to execute message bus daemon '%s': %s\n",
           me, dbus_daemon, strerror (errno));
}

static void
exec_app (int prog_arg, char **argv)
{
  execvp (argv[prog_arg], argv + prog_arg);

  fprintf (stderr, "%s: failed to exec '%s': %s\n", me, argv[prog_arg],
           strerror (errno));
  exit (1);
}

int
main (int argc, char **argv)
{
  int prog_arg = 0;
  int bus_address_pipe[2] = { 0, 0 };
  const char *config_file = NULL;
  const char *dbus_daemon = NULL;
  char bus_address[MAX_ADDR_LEN] = { 0 };
  const char *prev_arg = NULL;
  int i = 1;
  int requires_arg = 0;
  pid_t bus_pid;
  pid_t app_pid;

  while (i < argc)
    {
      const char *arg = argv[i];

      if (requires_arg)
        {
          const char **arg_dest;

          assert (prev_arg != NULL);

          if (strcmp (prev_arg, "--config-file") == 0)
            {
              arg_dest = &config_file;
            }
          else if (strcmp (prev_arg, "--dbus-daemon") == 0)
            {
              arg_dest = &dbus_daemon;
            }
          else
            {
              /* shouldn't happen */
              fprintf (stderr, "%s: internal error: %s not fully implemented\n",
                       me, prev_arg);
              return 127;
            }

          if (*arg_dest != NULL)
            {
              fprintf (stderr, "%s: %s given twice\n", me, prev_arg);
              return 127;
            }

          *arg_dest = arg;
          requires_arg = 0;
          prev_arg = arg;
          ++i;
          continue;
        }

      if (strcmp (arg, "--help") == 0 ||
          strcmp (arg, "-h") == 0 ||
          strcmp (arg, "-?") == 0)
        {
          usage (0);
        }
      else if (strcmp (arg, "--version") == 0)
        {
          version ();
        }
      else if (strstr (arg, "--config-file=") == arg)
        {
          const char *file;

          if (config_file != NULL)
            {
              fprintf (stderr, "%s: --config-file given twice\n", me);
              return 127;
            }

          file = strchr (arg, '=');
          ++file;

          config_file = file;
        }
      else if (strstr (arg, "--dbus-daemon=") == arg)
        {
          const char *file;

          if (dbus_daemon != NULL)
            {
              fprintf (stderr, "%s: --dbus-daemon given twice\n", me);
              return 127;
            }

          file = strchr (arg, '=');
          ++file;

          dbus_daemon = file;
        }
      else if (strcmp (arg, "--config-file") == 0 ||
               strcmp (arg, "--dbus-daemon") == 0)
        {
          requires_arg = 1;
        }
      else if (arg[0] == '-')
        {
          if (strcmp (arg, "--") != 0)
            {
              fprintf (stderr, "%s: option '%s' is unknown\n", me, arg);
              return 127;
            }
          else
            {
              prog_arg = i + 1;
              break;
            }
        }
      else
        {
          prog_arg = i;
          break;
        }

      prev_arg = arg;
      ++i;
    }

  /* "dbus-run-session" and "dbus-run-session ... --" are not allowed:
   * there must be something to run */
  if (prog_arg < 1 || prog_arg >= argc)
    {
      fprintf (stderr, "%s: a non-option argument is required\n", me);
      return 127;
    }

  if (requires_arg)
    {
      fprintf (stderr, "%s: option '%s' requires an argument\n", me, prev_arg);
      return 127;
    }

  if (dbus_daemon == NULL)
    dbus_daemon = "dbus-daemon";

  if (pipe (bus_address_pipe) < 0)
    {
      fprintf (stderr, "%s: failed to create pipe: %s\n", me, strerror (errno));
      return 127;
    }

  bus_pid = fork ();

  if (bus_pid < 0)
    {
      fprintf (stderr, "%s: failed to fork: %s\n", me, strerror (errno));
      return 127;
    }

  if (bus_pid == 0)
    {
      /* child */
      exec_dbus_daemon (dbus_daemon, bus_address_pipe, config_file);
      /* not reached */
      return 127;
    }

  close (bus_address_pipe[PIPE_WRITE_END]);

  switch (read_line (bus_address_pipe[PIPE_READ_END], bus_address, MAX_ADDR_LEN))
    {
    case READ_STATUS_OK:
      break;

    case READ_STATUS_EOF:
      fprintf (stderr, "%s: EOF reading address from bus daemon\n", me);
      return 127;
      break;

    case READ_STATUS_ERROR:
      fprintf (stderr, "%s: error reading address from bus daemon: %s\n",
               me, strerror (errno));
      return 127;
      break;
    }

  close (bus_address_pipe[PIPE_READ_END]);

  if (!dbus_setenv ("DBUS_SESSION_BUS_ADDRESS", bus_address) ||
      !dbus_setenv ("DBUS_SESSION_BUS_PID", NULL) ||
      !dbus_setenv ("DBUS_SESSION_BUS_WINDOWID", NULL) ||
      !dbus_setenv ("DBUS_STARTER_ADDRESS", NULL) ||
      !dbus_setenv ("DBUS_STARTER_BUS_TYPE", NULL))
    oom ();

  app_pid = fork ();

  if (app_pid < 0)
    {
      fprintf (stderr, "%s: failed to fork: %s\n", me, strerror (errno));
      return 127;
    }

  if (app_pid == 0)
    {
      /* child */
      exec_app (prog_arg, argv);
      /* not reached */
      return 127;
    }

  while (1)
    {
      int child_status;
      pid_t child_pid = waitpid (-1, &child_status, 0);

      if (child_pid == (pid_t) -1)
        {
          int errsv = errno;

          if (errsv == EINTR)
            continue;

          /* shouldn't happen: the only other documented errors are ECHILD,
           * which shouldn't happen because we terminate when all our children
           * have died, and EINVAL, which would indicate programming error */
          fprintf (stderr, "%s: waitpid() failed: %s\n", me, strerror (errsv));
          return 127;
        }
      else if (child_pid == bus_pid)
        {
          /* no need to kill it, now */
          bus_pid = 0;

          if (WIFEXITED (child_status))
            fprintf (stderr, "%s: dbus-daemon exited with code %d\n",
                me, WEXITSTATUS (child_status));
          else if (WIFSIGNALED (child_status))
            fprintf (stderr, "%s: dbus-daemon terminated by signal %d\n",
                me, WTERMSIG (child_status));
          else
            fprintf (stderr, "%s: dbus-daemon died or something\n", me);
        }
      else if (child_pid == app_pid)
        {
          if (bus_pid != 0)
            kill (bus_pid, SIGTERM);

          if (WIFEXITED (child_status))
            return WEXITSTATUS (child_status);

          /* if it died from a signal, behave like sh(1) */
          if (WIFSIGNALED (child_status))
            return 128 + WTERMSIG (child_status);

          /* I give up (this should never be reached) */
          fprintf (stderr, "%s: child process died or something\n", me);
          return 127;
        }
      else
        {
          fprintf (stderr, "%s: ignoring unknown child process %ld\n", me,
              (long) child_pid);
        }
    }

  return 0;
}
