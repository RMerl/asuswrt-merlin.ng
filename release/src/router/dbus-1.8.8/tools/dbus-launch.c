/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-launch.c  dbus-launch utility
 *
 * Copyright (C) 2003, 2006 Red Hat, Inc.
 * Copyright (C) 2006 Thiago Macieira <thiago@kde.org>
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
#include "dbus-launch.h"
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/select.h>
#include <time.h>

#ifdef DBUS_BUILD_X11
#include <X11/Xlib.h>
extern Display *xdisplay;
#endif

/* PROCESSES
 *
 * If you are in a shell and run "dbus-launch myapp", here is what happens:
 *
 * shell [*]
 *   \- main()               --exec--> myapp[*]
 *      \- "intermediate parent"
 *         \- bus-runner     --exec--> dbus-daemon --fork
 *         \- babysitter[*]            \- final dbus-daemon[*]
 *
 * Processes marked [*] survive the initial flurry of activity.
 *
 * If you run "dbus-launch --sh-syntax" then the diagram is the same, except
 * that main() prints variables and exits 0 instead of exec'ing myapp.
 *
 * PIPES
 *
 * dbus-daemon --print-pid     -> bus_pid_to_launcher_pipe     -> main
 * dbus-daemon --print-address -> bus_address_to_launcher_pipe -> main
 * main                        -> bus_pid_to_babysitter_pipe   -> babysitter
 *
 * The intermediate parent looks pretty useless at first glance. Its purpose
 * is to avoid the bus-runner becoming a zombie: when the intermediate parent
 * terminates, the bus-runner and babysitter are reparented to init, which
 * reaps them if they have finished. We can't rely on main() to reap arbitrary
 * children because it might exec myapp, after which it can't be relied on to
 * reap its children. We *can* rely on main() to reap the intermediate parent,
 * because that happens before it execs myapp.
 *
 * It's unclear why dbus-daemon needs to fork, but we explicitly tell it to
 * for some reason, then wait for it. If we left it undefined, a forking
 * dbus-daemon would get the parent process reparented to init and reaped
 * when the intermediate parent terminated, and a non-forking dbus-daemon
 * would get reparented to init and carry on there.
 *
 * myapp is exec'd by the process that initially ran main() so that it's
 * the shell's child, so the shell knows how to do job control and stuff.
 * This is desirable for the "dbus-launch an application" use-case, less so
 * for the "dbus-launch a test suite in an isolated session" use-case.
 */

static char* machine_uuid = NULL;

const char*
get_machine_uuid (void)
{
  return machine_uuid;
}

static void
save_machine_uuid (const char *uuid_arg)
{
  if (strlen (uuid_arg) != 32)
    {
      fprintf (stderr, "machine ID '%s' looks like it's the wrong length, should be 32 hex digits",
               uuid_arg);
      exit (1);
    }

  machine_uuid = xstrdup (uuid_arg);
}

#ifdef DBUS_BUILD_X11
#define UUID_MAXLEN 40
/* Read the machine uuid from file if needed. Returns TRUE if machine_uuid is
 * set after this function */
static int
read_machine_uuid_if_needed (void)
{
  FILE *f;
  char uuid[UUID_MAXLEN];
  size_t len;
  int ret = FALSE;

  if (machine_uuid != NULL)
    return TRUE;

  f = fopen (DBUS_MACHINE_UUID_FILE, "r");
  if (f == NULL)
    return FALSE;

  if (fgets (uuid, UUID_MAXLEN, f) == NULL)
    goto out;

  len = strlen (uuid);
  if (len < 32)
    goto out;

  /* rstrip the read uuid */
  while (len > 31 && isspace((int) uuid[len - 1]))
    len--;

  if (len != 32)
    goto out;

  uuid[len] = '\0';
  machine_uuid = xstrdup (uuid);
  verbose ("UID: %s\n", machine_uuid);
  ret = TRUE;

out:
  fclose(f);
  return ret;
}
#endif /* DBUS_BUILD_X11 */

void
verbose (const char *format,
         ...)
{
#ifdef DBUS_ENABLE_VERBOSE_MODE
  va_list args;
  static int verbose = TRUE;
  static int verbose_initted = FALSE;
  
  /* things are written a bit oddly here so that
   * in the non-verbose case we just have the one
   * conditional and return immediately.
   */
  if (!verbose)
    return;
  
  if (!verbose_initted)
    {
      verbose = getenv ("DBUS_VERBOSE") != NULL;
      verbose_initted = TRUE;
      if (!verbose)
        return;
    }

  fprintf (stderr, "%lu: ", (unsigned long) getpid ());
  
  va_start (args, format);
  vfprintf (stderr, format, args);
  va_end (args);
#endif /* DBUS_ENABLE_VERBOSE_MODE */
}

static void
usage (int ecode)
{
  fprintf (stderr, "dbus-launch [--version] [--help] [--sh-syntax]"
           " [--csh-syntax] [--auto-syntax] [--binary-syntax] [--close-stderr]"
           " [--exit-with-session] [--autolaunch=MACHINEID]"
           " [--config-file=FILENAME] [PROGRAM] [ARGS...]\n");
  exit (ecode);
}

static void
version (void)
{
  printf ("D-Bus Message Bus Launcher %s\n"
          "Copyright (C) 2003 Red Hat, Inc.\n"
          "This is free software; see the source for copying conditions.\n"
          "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
          VERSION);
  exit (0);
}

char *
xstrdup (const char *str)
{
  int len;
  char *copy;
  
  if (str == NULL)
    return NULL;
  
  len = strlen (str);

  copy = malloc (len + 1);
  if (copy == NULL)
    return NULL;

  memcpy (copy, str, len + 1);
  
  return copy;
}

static char *
concat2 (const char *a,
    const char *b)
{
  size_t la, lb;
  char *ret;

  la = strlen (a);
  lb = strlen (b);

  ret = malloc (la + lb + 1);

  if (ret == NULL)
    return NULL;

  memcpy (ret, a, la);
  memcpy (ret + la, b, lb + 1);
  return ret;
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
  
  while (TRUE)
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

static ReadStatus
read_pid (int        fd,
          pid_t     *buf)
{
  size_t bytes = 0;
  ReadStatus retval;

  retval = READ_STATUS_OK;
  
  while (TRUE)
    {
      ssize_t chunk;    
      size_t to_read;
      
    again:
      to_read = sizeof (pid_t) - bytes;

      if (to_read == 0)
        break;
      
      chunk = read (fd,
                    ((char*)buf) + bytes,
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

  return retval;
}

static void
do_write (int fd, const void *buf, size_t count)
{
  size_t bytes_written;
  int ret;
  
  bytes_written = 0;
  
 again:
  
  ret = write (fd, ((const char*)buf) + bytes_written, count - bytes_written);

  if (ret < 0)
    {
      if (errno == EINTR)
        goto again;
      else
        {
          fprintf (stderr, "Failed to write data to pipe! %s\n",
                   strerror (errno));
          exit (1); /* give up, we suck */
        }
    }
  else
    bytes_written += ret;
  
  if (bytes_written < count)
    goto again;
}

static void
write_pid (int   fd,
           pid_t pid)
{
  do_write (fd, &pid, sizeof (pid));
}

static int
do_waitpid (pid_t pid)
{
  int ret;
  
 again:
  ret = waitpid (pid, NULL, 0);

  if (ret < 0 &&
      errno == EINTR)
    goto again;

  return ret;
}

static pid_t bus_pid_to_kill = -1;

static void
kill_bus(void)
{
  if (bus_pid_to_kill <= 0)
    return;

  verbose ("Killing message bus and exiting babysitter\n");
  kill (bus_pid_to_kill, SIGTERM);
  sleep (3);
  kill (bus_pid_to_kill, SIGKILL);
}

void
kill_bus_and_exit (int exitcode)
{
  /* in case these point to any NFS mounts, get rid of them immediately */
  close (0);
  close (1);
  close (2);

  kill_bus();

  exit (exitcode);
}

static void
print_variables (const char *bus_address, pid_t bus_pid, long bus_wid,
		 int c_shell_syntax, int bourne_shell_syntax,
		 int binary_syntax)
{
  if (binary_syntax)
    {
      do_write (1, bus_address, strlen (bus_address) + 1);
      do_write (1, &bus_pid, sizeof bus_pid);
      do_write (1, &bus_wid, sizeof bus_wid);
      return;
    }
  else if (c_shell_syntax)
    {
      printf ("setenv DBUS_SESSION_BUS_ADDRESS '%s';\n", bus_address);	
      printf ("set DBUS_SESSION_BUS_PID=%ld;\n", (long) bus_pid);
      if (bus_wid)
        printf ("set DBUS_SESSION_BUS_WINDOWID=%ld;\n", (long) bus_wid);
      fflush (stdout);
    }
  else if (bourne_shell_syntax)
    {
      printf ("DBUS_SESSION_BUS_ADDRESS='%s';\n", bus_address);
      printf ("export DBUS_SESSION_BUS_ADDRESS;\n");
      printf ("DBUS_SESSION_BUS_PID=%ld;\n", (long) bus_pid);
      if (bus_wid)
        printf ("DBUS_SESSION_BUS_WINDOWID=%ld;\n", (long) bus_wid);
      fflush (stdout);
    }
  else
    {
      printf ("DBUS_SESSION_BUS_ADDRESS=%s\n", bus_address);
      printf ("DBUS_SESSION_BUS_PID=%ld\n", (long) bus_pid);
      if (bus_wid)
	printf ("DBUS_SESSION_BUS_WINDOWID=%ld\n", (long) bus_wid);
      fflush (stdout);
    }
}

static int got_sighup = FALSE;

static void
signal_handler (int sig)
{
  switch (sig)
    {
    case SIGHUP:
    case SIGINT:
    case SIGTERM:
      got_sighup = TRUE;
      break;
    }
}

static void
kill_bus_when_session_ends (void)
{
  int tty_fd;
  int x_fd;
  fd_set read_set;
  fd_set err_set;
  struct sigaction act;
  sigset_t empty_mask;
  
  /* install SIGHUP handler */
  got_sighup = FALSE;
  sigemptyset (&empty_mask);
  act.sa_handler = signal_handler;
  act.sa_mask    = empty_mask;
  act.sa_flags   = 0;
  sigaction (SIGHUP,  &act, NULL);
  sigaction (SIGTERM,  &act, NULL);
  sigaction (SIGINT,  &act, NULL);
  
#ifdef DBUS_BUILD_X11
  x11_init();
  if (xdisplay != NULL)
    {
      x_fd = ConnectionNumber (xdisplay);
    }
  else
    x_fd = -1;
#else
  x_fd = -1;
#endif

  if (isatty (0))
    tty_fd = 0;
  else
    tty_fd = -1;

  if (x_fd >= 0)
    {
      verbose ("session lifetime is defined by X, not monitoring stdin\n");
      tty_fd = -1;
    }
  else if (tty_fd >= 0)
    {
      verbose ("stdin isatty(), monitoring it\n");
    }
  else
    {
      verbose ("stdin was not a TTY, not monitoring it\n");
    }

  if (tty_fd < 0 && x_fd < 0)
    {
      fprintf (stderr, "No terminal on standard input and no X display; cannot attach message bus to session lifetime\n");
      kill_bus_and_exit (1);
    }
  
  while (TRUE)
    {
#ifdef DBUS_BUILD_X11
      /* Dump events on the floor, and let
       * IO error handler run if we lose
       * the X connection. It's important to
       * run this before going into select() since
       * we might have queued outgoing messages or
       * events.
       */
      x11_handle_event ();
#endif
      
      FD_ZERO (&read_set);
      FD_ZERO (&err_set);

      if (tty_fd >= 0)
        {
          FD_SET (tty_fd, &read_set);
          FD_SET (tty_fd, &err_set);
        }

      if (x_fd >= 0)
        {
          FD_SET (x_fd, &read_set);
          FD_SET (x_fd, &err_set);
        }

      select (MAX (tty_fd, x_fd) + 1,
              &read_set, NULL, &err_set, NULL);

      if (got_sighup)
        {
          verbose ("Got SIGHUP, exiting\n");
          kill_bus_and_exit (0);
        }
      
#ifdef DBUS_BUILD_X11
      /* Events will be processed before we select again
       */
      if (x_fd >= 0)
        verbose ("X fd condition reading = %d error = %d\n",
                 FD_ISSET (x_fd, &read_set),
                 FD_ISSET (x_fd, &err_set));
#endif

      if (tty_fd >= 0)
        {
          if (FD_ISSET (tty_fd, &read_set))
            {
              int bytes_read;
              char discard[512];

              verbose ("TTY ready for reading\n");
              
              bytes_read = read (tty_fd, discard, sizeof (discard));

              verbose ("Read %d bytes from TTY errno = %d\n",
                       bytes_read, errno);
              
              if (bytes_read == 0)
                kill_bus_and_exit (0); /* EOF */
              else if (bytes_read < 0 && errno != EINTR)
                {
                  /* This shouldn't happen I don't think; to avoid
                   * spinning on the fd forever we exit.
                   */
                  fprintf (stderr, "dbus-launch: error reading from stdin: %s\n",
                           strerror (errno));
                  kill_bus_and_exit (0);
                }
            }
          else if (FD_ISSET (tty_fd, &err_set))
            {
              verbose ("TTY has error condition\n");
              
              kill_bus_and_exit (0);
            }
        }
    }
}

static void
babysit (int   exit_with_session,
         pid_t child_pid,
         int   read_bus_pid_fd)  /* read pid from here */
{
  int ret;
  int dev_null_fd;
  const char *s;

  verbose ("babysitting, exit_with_session = %d, child_pid = %ld, read_bus_pid_fd = %d\n",
           exit_with_session, (long) child_pid, read_bus_pid_fd);
  
  /* We chdir ("/") since we are persistent and daemon-like, and fork
   * again so dbus-launch can reap the parent.  However, we don't
   * setsid() or close fd 0 because the idea is to remain attached
   * to the tty and the X server in order to kill the message bus
   * when the session ends.
   */

  if (chdir ("/") < 0)
    {
      fprintf (stderr, "Could not change to root directory: %s\n",
               strerror (errno));
      exit (1);
    }

  /* Close stdout/stderr so we don't block an "eval" or otherwise
   * lock up. stdout is still chaining through to dbus-launch
   * and in turn to the parent shell.
   */
  dev_null_fd = open ("/dev/null", O_RDWR);
  if (dev_null_fd >= 0)
    {
      if (!exit_with_session)
        dup2 (dev_null_fd, 0);
      dup2 (dev_null_fd, 1);
      s = getenv ("DBUS_DEBUG_OUTPUT");
      if (s == NULL || *s == '\0')
        dup2 (dev_null_fd, 2);
      close (dev_null_fd);
    }
  else
    {
      fprintf (stderr, "Failed to open /dev/null: %s\n",
               strerror (errno));
      /* continue, why not */
    }
  
  ret = fork ();

  if (ret < 0)
    {
      fprintf (stderr, "fork() failed in babysitter: %s\n",
               strerror (errno));
      exit (1);
    }

  if (ret > 0)
    {
      /* Parent reaps pre-fork part of bus daemon, then exits and is
       * reaped so the babysitter isn't a zombie
       */

      verbose ("=== Babysitter's intermediate parent continues again\n");
      
      if (do_waitpid (child_pid) < 0)
        {
          /* shouldn't happen */
          fprintf (stderr, "Failed waitpid() waiting for bus daemon's parent\n");
          exit (1);
        }

      verbose ("Babysitter's intermediate parent exiting\n");
      
      exit (0);
    }

  /* Child continues */
  verbose ("=== Babysitter process created\n");

  verbose ("Reading PID from bus\n");
      
  switch (read_pid (read_bus_pid_fd, &bus_pid_to_kill))
    {
    case READ_STATUS_OK:
      break;
    case READ_STATUS_EOF:
      fprintf (stderr, "EOF in dbus-launch reading PID from bus daemon\n");
      exit (1);
      break;
    case READ_STATUS_ERROR:
      fprintf (stderr, "Error in dbus-launch reading PID from bus daemon: %s\n",
	       strerror (errno));
      exit (1);
      break;
    }

  verbose ("Got PID %ld from daemon\n",
           (long) bus_pid_to_kill);
  
  if (exit_with_session)
    {
      /* Bus is now started and launcher has needed info;
       * we connect to X display and tty and wait to
       * kill bus if requested.
       */
      
      kill_bus_when_session_ends ();
    }

  verbose ("Babysitter exiting\n");
  
  exit (0);
}

static void
do_close_stderr (void)
{
  int fd;

  fflush (stderr);

  /* dbus-launch is a Unix-only program, so we can rely on /dev/null being there.
   * We're including unistd.h and we're dealing with sh/csh launch sequences...
   */
  fd = open ("/dev/null", O_RDWR);
  if (fd == -1)
    {
      fprintf (stderr, "Internal error: cannot open /dev/null: %s", strerror (errno));
      exit (1);
    }

  close (2);
  if (dup2 (fd, 2) == -1)
    {
      /* error; we can't report an error anymore... */
      exit (1);
    }
  close (fd);
}

static void
pass_info (const char *runprog, const char *bus_address, pid_t bus_pid,
           long bus_wid, int c_shell_syntax, int bourne_shell_syntax,
           int binary_syntax,
           int argc, char **argv, int remaining_args)
{
  char *envvar = NULL;
  char **args = NULL;

  if (runprog)
    {
      int i;

      envvar = malloc (strlen ("DBUS_SESSION_BUS_ADDRESS=") +
          strlen (bus_address) + 1);
      args = malloc (sizeof (char *) * ((argc-remaining_args)+2));

      if (envvar == NULL || args == NULL)
        goto oom;

      args[0] = xstrdup (runprog);
      if (!args[0])
        goto oom;
      for (i = 1; i <= (argc-remaining_args); i++)
        {
          size_t len = strlen (argv[remaining_args+i-1])+1;
          args[i] = malloc (len);
          if (!args[i])
	    {
              while (i > 1)
                free (args[--i]);
              goto oom;
	    }
          strncpy (args[i], argv[remaining_args+i-1], len);
        }
      args[i] = NULL;

      strcpy (envvar, "DBUS_SESSION_BUS_ADDRESS=");
      strcat (envvar, bus_address);
      putenv (envvar);

      execvp (runprog, args);
      fprintf (stderr, "Couldn't exec %s: %s\n", runprog, strerror (errno));
      exit (1);
    }
   else
    {
      print_variables (bus_address, bus_pid, bus_wid, c_shell_syntax,
          bourne_shell_syntax, binary_syntax);
    }
  verbose ("dbus-launch exiting\n");

  fflush (stdout);
  fflush (stderr);
  close (1);
  close (2);
  exit (0);
oom:
  if (envvar)
    free (envvar);

  if (args)
    free (args);

  fprintf (stderr, "Out of memory!");
  exit (1);
}

#define READ_END  0
#define WRITE_END 1

int
main (int argc, char **argv)
{
  const char *prev_arg;
  const char *shname;
  const char *runprog = NULL;
  int remaining_args = 0;
  int exit_with_session;
  int binary_syntax = FALSE;
  int c_shell_syntax = FALSE;
  int bourne_shell_syntax = FALSE;
  int auto_shell_syntax = FALSE;
  int autolaunch = FALSE;
  int requires_arg = FALSE;
  int close_stderr = FALSE;
  int i;
  int ret;
  int bus_pid_to_launcher_pipe[2];
  int bus_pid_to_babysitter_pipe[2];
  int bus_address_to_launcher_pipe[2];
  char *config_file;
  
  exit_with_session = FALSE;
  config_file = NULL;
  
  prev_arg = NULL;
  i = 1;
  while (i < argc)
    {
      const char *arg = argv[i];
 
      if (strcmp (arg, "--help") == 0 ||
          strcmp (arg, "-h") == 0 ||
          strcmp (arg, "-?") == 0)
        usage (0);
      else if (strcmp (arg, "--auto-syntax") == 0)
        auto_shell_syntax = TRUE;
      else if (strcmp (arg, "-c") == 0 ||
               strcmp (arg, "--csh-syntax") == 0)
        c_shell_syntax = TRUE;
      else if (strcmp (arg, "-s") == 0 ||
               strcmp (arg, "--sh-syntax") == 0)
        bourne_shell_syntax = TRUE;
      else if (strcmp (arg, "--binary-syntax") == 0)
        binary_syntax = TRUE;
      else if (strcmp (arg, "--version") == 0)
        version ();
      else if (strcmp (arg, "--exit-with-session") == 0)
        exit_with_session = TRUE;
      else if (strcmp (arg, "--close-stderr") == 0)
        close_stderr = TRUE;
      else if (strstr (arg, "--autolaunch=") == arg)
        {
          const char *s;

          if (autolaunch)
            {
              fprintf (stderr, "--autolaunch given twice\n");
              exit (1);
            }
          
          autolaunch = TRUE;

          s = strchr (arg, '=');
          ++s;

          save_machine_uuid (s);
        }
      else if (prev_arg &&
               strcmp (prev_arg, "--autolaunch") == 0)
        {
          if (autolaunch)
            {
              fprintf (stderr, "--autolaunch given twice\n");
              exit (1);
            }
          
          autolaunch = TRUE;

          save_machine_uuid (arg);
	  requires_arg = FALSE;
        }
      else if (strcmp (arg, "--autolaunch") == 0)
	requires_arg = TRUE;
      else if (strstr (arg, "--config-file=") == arg)
        {
          const char *file;

          if (config_file != NULL)
            {
              fprintf (stderr, "--config-file given twice\n");
              exit (1);
            }
          
          file = strchr (arg, '=');
          ++file;

          config_file = xstrdup (file);
        }
      else if (prev_arg &&
               strcmp (prev_arg, "--config-file") == 0)
        {
          if (config_file != NULL)
            {
              fprintf (stderr, "--config-file given twice\n");
              exit (1);
            }

          config_file = xstrdup (arg);
	  requires_arg = FALSE;
        }
      else if (strcmp (arg, "--config-file") == 0)
	requires_arg = TRUE;
      else if (arg[0] == '-')
        {
          if (strcmp (arg, "--") != 0)
            {
              fprintf (stderr, "Option `%s' is unknown.\n", arg);
              exit (1);
            }
          else
            {
              runprog = argv[i+1];
              remaining_args = i+2;
              break;
            }
        }
      else
	{
	  runprog = arg;
	  remaining_args = i+1;
	  break;
	}
      
      prev_arg = arg;
      
      ++i;
    }
  if (requires_arg)
    {
      fprintf (stderr, "Option `%s' requires an argument.\n", prev_arg);
      exit (1);
    }

  if (auto_shell_syntax)
    {
      if ((shname = getenv ("SHELL")) != NULL)
       {
         if (!strncmp (shname + strlen (shname) - 3, "csh", 3))
           c_shell_syntax = TRUE;
         else
           bourne_shell_syntax = TRUE;
       }
      else
       bourne_shell_syntax = TRUE;
    }  

  if (exit_with_session)
    verbose ("--exit-with-session enabled\n");

  if (autolaunch)
    {      
#ifndef DBUS_BUILD_X11
      fprintf (stderr, "Autolaunch requested, but X11 support not compiled in.\n"
	       "Cannot continue.\n");
      exit (1);
#else /* DBUS_BUILD_X11 */
#ifndef DBUS_ENABLE_X11_AUTOLAUNCH
      fprintf (stderr, "X11 autolaunch support disabled at compile time.\n");
      exit (1);
#else /* DBUS_ENABLE_X11_AUTOLAUNCH */
      char *address;
      pid_t pid;
      long wid;
      
      if (get_machine_uuid () == NULL)
        {
          fprintf (stderr, "Machine UUID not provided as arg to --autolaunch\n");
          exit (1);
        }

      verbose ("Autolaunch enabled (using X11).\n");
      if (!exit_with_session)
	{
	  verbose ("--exit-with-session automatically enabled\n");
	  exit_with_session = TRUE;
	}

      if (!x11_init ())
	{
	  fprintf (stderr, "Autolaunch error: X11 initialization failed.\n");
	  exit (1);
	}

      if (!x11_get_address (&address, &pid, &wid))
	{
	  fprintf (stderr, "Autolaunch error: X11 communication error.\n");
	  exit (1);
	}

      if (address != NULL)
	{
	  verbose ("dbus-daemon is already running. Returning existing parameters.\n");
	  pass_info (runprog, address, pid, wid, c_shell_syntax,
			   bourne_shell_syntax, binary_syntax, argc, argv, remaining_args);
	  exit (0);
	}
#endif /* DBUS_ENABLE_X11_AUTOLAUNCH */
    }
  else if (read_machine_uuid_if_needed())
    {
      x11_init();
#endif /* DBUS_BUILD_X11 */
    }


  if (pipe (bus_pid_to_launcher_pipe) < 0 ||
      pipe (bus_address_to_launcher_pipe) < 0 ||
      pipe (bus_pid_to_babysitter_pipe) < 0)
    {
      fprintf (stderr,
               "Failed to create pipe: %s\n",
               strerror (errno));
      exit (1);
    }

  ret = fork ();
  if (ret < 0)
    {
      fprintf (stderr, "Failed to fork: %s\n",
               strerror (errno));
      exit (1);
    }

  if (ret == 0)
    {
      /* Child */
#define MAX_FD_LEN 64
      char write_pid_fd_as_string[MAX_FD_LEN];
      char write_address_fd_as_string[MAX_FD_LEN];

#ifdef DBUS_BUILD_X11
      xdisplay = NULL;
#endif

      if (close_stderr)
	do_close_stderr ();

      verbose ("=== Babysitter's intermediate parent created\n");

      /* Fork once more to create babysitter */
      
      ret = fork ();
      if (ret < 0)
        {
          fprintf (stderr, "Failed to fork: %s\n",
                   strerror (errno));
          exit (1);
        }
      
      if (ret > 0)
        {
          /* In babysitter */
          verbose ("=== Babysitter's intermediate parent continues\n");
          
          close (bus_pid_to_launcher_pipe[READ_END]);
	  close (bus_pid_to_launcher_pipe[WRITE_END]);
          close (bus_address_to_launcher_pipe[READ_END]);
          close (bus_address_to_launcher_pipe[WRITE_END]);
          close (bus_pid_to_babysitter_pipe[WRITE_END]);

          /* babysit() will fork *again*
           * and will also reap the pre-forked bus
           * daemon
           */
          babysit (exit_with_session, ret,
                   bus_pid_to_babysitter_pipe[READ_END]);
          exit (0);
        }

      verbose ("=== Bus exec process created\n");
      
      /* Now we are the bus process (well, almost;
       * dbus-daemon itself forks again)
       */
      close (bus_pid_to_launcher_pipe[READ_END]);
      close (bus_address_to_launcher_pipe[READ_END]);
      close (bus_pid_to_babysitter_pipe[READ_END]);
      close (bus_pid_to_babysitter_pipe[WRITE_END]);

      sprintf (write_pid_fd_as_string,
               "%d", bus_pid_to_launcher_pipe[WRITE_END]);

      sprintf (write_address_fd_as_string,
               "%d", bus_address_to_launcher_pipe[WRITE_END]);

      verbose ("Calling exec()\n");
 
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
      {
        const char *test_daemon;
        /* exec from testdir */
        if (getenv ("DBUS_USE_TEST_BINARY") != NULL &&
            (test_daemon = getenv ("DBUS_TEST_DAEMON")) != NULL)
          {
            if (config_file == NULL && getenv ("DBUS_TEST_DATA") != NULL)
              {
                config_file = concat2 (getenv ("DBUS_TEST_DATA"),
                    "/valid-config-files/session.conf");

                if (config_file == NULL)
                  {
                    fprintf (stderr, "Out of memory\n");
                    exit (1);
                  }
              }

            execl (test_daemon,
                   test_daemon,
                   "--fork",
                   "--print-pid", write_pid_fd_as_string,
                   "--print-address", write_address_fd_as_string,
                   config_file ? "--config-file" : "--session",
                   config_file, /* has to be last in this varargs list */
                   NULL);

            fprintf (stderr,
                     "Failed to execute test message bus daemon %s: %s.\n",
                     test_daemon, strerror (errno));
            exit (1);
          }
      }
 #endif /* DBUS_ENABLE_EMBEDDED_TESTS */

      execl (DBUS_DAEMONDIR"/dbus-daemon",
             DBUS_DAEMONDIR"/dbus-daemon",
             "--fork",
             "--print-pid", write_pid_fd_as_string,
             "--print-address", write_address_fd_as_string,
             config_file ? "--config-file" : "--session",
             config_file, /* has to be last in this varargs list */
             NULL);

      fprintf (stderr,
               "Failed to execute message bus daemon %s: %s.  Will try again without full path.\n",
               DBUS_DAEMONDIR"/dbus-daemon", strerror (errno));
      
      /*
       * If it failed, try running without full PATH.  Note this is needed
       * because the build process builds the run-with-tmp-session-bus.conf
       * file and the dbus-daemon will not be in the install location during
       * build time.
       */
      execlp ("dbus-daemon",
              "dbus-daemon",
              "--fork",
              "--print-pid", write_pid_fd_as_string,
              "--print-address", write_address_fd_as_string,
              config_file ? "--config-file" : "--session",
              config_file, /* has to be last in this varargs list */
              NULL);

      fprintf (stderr,
               "Failed to execute message bus daemon: %s\n",
               strerror (errno));
      exit (1);
    }
  else
    {
      /* Parent */
#define MAX_PID_LEN 64
      pid_t bus_pid;  
      char bus_address[MAX_ADDR_LEN];
      char buf[MAX_PID_LEN];
      char *end;
      long wid = 0;
      long val;

      verbose ("=== Parent dbus-launch continues\n");
      
      close (bus_pid_to_launcher_pipe[WRITE_END]);
      close (bus_address_to_launcher_pipe[WRITE_END]);
      close (bus_pid_to_babysitter_pipe[READ_END]);

      verbose ("Waiting for babysitter's intermediate parent\n");
      
      /* Immediately reap parent of babysitter
       * (which was created just for us to reap)
       */
      if (do_waitpid (ret) < 0)
        {
          fprintf (stderr, "Failed to waitpid() for babysitter intermediate process: %s\n",
                   strerror (errno));
          exit (1);
        }

      verbose ("Reading address from bus\n");
      
      /* Read the pipe data, print, and exit */
      switch (read_line (bus_address_to_launcher_pipe[READ_END],
                         bus_address, MAX_ADDR_LEN))
        {
        case READ_STATUS_OK:
          break;
        case READ_STATUS_EOF:
          fprintf (stderr, "EOF in dbus-launch reading address from bus daemon\n");
          exit (1);
          break;
        case READ_STATUS_ERROR:
          fprintf (stderr, "Error in dbus-launch reading address from bus daemon: %s\n",
                   strerror (errno));
          exit (1);
          break;
        }
        
      close (bus_address_to_launcher_pipe[READ_END]);

      verbose ("Reading PID from daemon\n");
      /* Now read data */
      switch (read_line (bus_pid_to_launcher_pipe[READ_END], buf, MAX_PID_LEN))
	{
	case READ_STATUS_OK:
	  break;
	case READ_STATUS_EOF:
	  fprintf (stderr, "EOF reading PID from bus daemon\n");
	  exit (1);
	  break;
	case READ_STATUS_ERROR:
	  fprintf (stderr, "Error reading PID from bus daemon: %s\n",
		   strerror (errno));
	  exit (1);
	  break;
	}

      end = NULL;
      val = strtol (buf, &end, 0);
      if (buf == end || end == NULL)
	{
	  fprintf (stderr, "Failed to parse bus PID \"%s\": %s\n",
		   buf, strerror (errno));
	  exit (1);
	}

      bus_pid = val;

      /* Have to initialize bus_pid_to_kill ASAP, so that the
         X error callback can kill it if an error happens. */
      bus_pid_to_kill = bus_pid;

      close (bus_pid_to_launcher_pipe[READ_END]);

#ifdef DBUS_ENABLE_X11_AUTOLAUNCH
      if (xdisplay != NULL)
        {
          int ret2;

          verbose("Saving x11 address\n");
          ret2 = x11_save_address (bus_address, bus_pid, &wid);
          /* Only get an existing dbus session when autolaunching */
          if (autolaunch)
            {
              if (ret2 == 0)
                {
                  char *address = NULL;
                  /* another window got added. Return its address */
                  if (x11_get_address (&address, &bus_pid, &wid)
                       && address != NULL)
                    {
                      verbose ("dbus-daemon is already running. Returning existing parameters.\n");
                      /* Kill the old bus */
                      kill_bus();
                      pass_info (runprog, address, bus_pid, wid,
                         c_shell_syntax, bourne_shell_syntax, binary_syntax,
                         argc, argv, remaining_args);
                    }
                  }
              if (ret2 < 0)
                {
                  fprintf (stderr, "Error saving bus information.\n");
                  bus_pid_to_kill = bus_pid;
                  kill_bus_and_exit (1);
                }
            }
        }
#endif

      /* Forward the pid to the babysitter */
      write_pid (bus_pid_to_babysitter_pipe[WRITE_END], bus_pid);
      close (bus_pid_to_babysitter_pipe[WRITE_END]);

       pass_info (runprog, bus_address, bus_pid, wid, c_shell_syntax,
              bourne_shell_syntax, binary_syntax, argc, argv, remaining_args);
    }

  return 0;
}
