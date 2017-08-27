/* A program to put stress on a POSIX system (stress).
 *
 * Copyright (C) 2001,2002,2003,2004,2005,2006,2007,2008,2009,2010
 * Amos Waterland <apw@rossby.metr.ou.edu>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

/* By default, print all messages of severity info and above.  */
static int global_debug = 2;

/* Name of this program */
static char *global_progname = PACKAGE;

/* Implemention of runtime-selectable severity message printing.  */
#define dbg(OUT, STR, ARGS...) if (global_debug >= 3) \
	fprintf (stdout, "%s: dbug: [%lli] ", \
		global_progname, (long long)getpid()), \
		fprintf(OUT, STR, ##ARGS), fflush(OUT)
#define out(OUT, STR, ARGS...) if (global_debug >= 2) \
	fprintf (stdout, "%s: info: [%lli] ", \
		global_progname, (long long)getpid()), \
		fprintf(OUT, STR, ##ARGS), fflush(OUT)
#define wrn(OUT, STR, ARGS...) if (global_debug >= 1) \
	fprintf (stderr, "%s: WARN: [%lli] (%d) ", \
		global_progname, (long long)getpid(), __LINE__), \
		fprintf(OUT, STR, ##ARGS), fflush(OUT)
#define err(OUT, STR, ARGS...) if (global_debug >= 0) \
	fprintf (stderr, "%s: FAIL: [%lli] (%d) ", \
		global_progname, (long long)getpid(), __LINE__), \
		fprintf(OUT, STR, ##ARGS), fflush(OUT)

/* Implementation of check for option argument correctness.  */
#define assert_arg(A) \
          if (++i == argc || ((arg = argv[i])[0] == '-' && \
              !isdigit ((int)arg[1]) )) \
            { \
              err (stderr, "missing argument to option '%s'\n", A); \
              exit (1); \
            }

/* Prototypes for utility functions.  */
int usage (int status);
int version (int status);
long long atoll_s (const char *nptr);
long long atoll_b (const char *nptr);

/* Prototypes for worker functions.  */
int hogcpu (void);
int hogio (void);
int hogvm (long long bytes, long long stride, long long hang, int keep);
int hoghdd (long long bytes);

int
main (int argc, char **argv)
{
  int i, pid, children = 0, retval = 0;
  long starttime, stoptime, runtime, forks;

  /* Variables that indicate which options have been selected.  */
  int do_dryrun = 0;
  long long do_backoff = 3000;
  long long do_timeout = 0;
  long long do_cpu = 0;
  long long do_io = 0;
  long long do_vm = 0;
  long long do_vm_bytes = 256 * 1024 * 1024;
  long long do_vm_stride = 4096;
  long long do_vm_hang = -1;
  int do_vm_keep = 0;
  long long do_hdd = 0;
  long long do_hdd_bytes = 1024 * 1024 * 1024;

  /* Record our start time.  */
  if ((starttime = time (NULL)) == -1)
    {
      err (stderr, "failed to acquire current time: %s\n", strerror (errno));
      exit (1);
    }

  /* SuSv3 does not define any error conditions for this function.  */
  global_progname = basename (argv[0]);

  /* For portability, parse command line options without getopt_long.  */
  for (i = 1; i < argc; i++)
    {
      char *arg = argv[i];

      if (strcmp (arg, "--help") == 0 || strcmp (arg, "-?") == 0)
        {
          usage (0);
        }
      else if (strcmp (arg, "--version") == 0)
        {
          version (0);
        }
      else if (strcmp (arg, "--verbose") == 0 || strcmp (arg, "-v") == 0)
        {
          global_debug = 3;
        }
      else if (strcmp (arg, "--quiet") == 0 || strcmp (arg, "-q") == 0)
        {
          global_debug = 0;
        }
      else if (strcmp (arg, "--dry-run") == 0 || strcmp (arg, "-n") == 0)
        {
          do_dryrun = 1;
        }
      else if (strcmp (arg, "--backoff") == 0)
        {
          assert_arg ("--backoff");
          if (sscanf (arg, "%lli", &do_backoff) != 1)
            {
              err (stderr, "invalid number: %s\n", arg);
              exit (1);
            }
          if (do_backoff < 0)
            {
              err (stderr, "invalid backoff factor: %lli\n", do_backoff);
              exit (1);
            }
          dbg (stdout, "setting backoff coeffient to %llius\n", do_backoff);
        }
      else if (strcmp (arg, "--timeout") == 0 || strcmp (arg, "-t") == 0)
        {
          assert_arg ("--timeout");
          do_timeout = atoll_s (arg);
          if (do_timeout <= 0)
            {
              err (stderr, "invalid timeout value: %llis\n", do_timeout);
              exit (1);
            }
        }
      else if (strcmp (arg, "--cpu") == 0 || strcmp (arg, "-c") == 0)
        {
          assert_arg ("--cpu");
          do_cpu = atoll_b (arg);
          if (do_cpu <= 0)
            {
              err (stderr, "invalid number of cpu hogs: %lli\n", do_cpu);
              exit (1);
            }
        }
      else if (strcmp (arg, "--io") == 0 || strcmp (arg, "-i") == 0)
        {
          assert_arg ("--io");
          do_io = atoll_b (arg);
          if (do_io <= 0)
            {
              err (stderr, "invalid number of io hogs: %lli\n", do_io);
              exit (1);
            }
        }
      else if (strcmp (arg, "--vm") == 0 || strcmp (arg, "-m") == 0)
        {
          assert_arg ("--vm");
          do_vm = atoll_b (arg);
          if (do_vm <= 0)
            {
              err (stderr, "invalid number of vm hogs: %lli\n", do_vm);
              exit (1);
            }
        }
      else if (strcmp (arg, "--vm-bytes") == 0)
        {
          assert_arg ("--vm-bytes");
          do_vm_bytes = atoll_b (arg);
          if (do_vm_bytes <= 0)
            {
              err (stderr, "invalid vm byte value: %lli\n", do_vm_bytes);
              exit (1);
            }
        }
      else if (strcmp (arg, "--vm-stride") == 0)
        {
          assert_arg ("--vm-stride");
          do_vm_stride = atoll_b (arg);
          if (do_vm_stride <= 0)
            {
              err (stderr, "invalid stride value: %lli\n", do_vm_stride);
              exit (1);
            }
        }
      else if (strcmp (arg, "--vm-hang") == 0)
        {
          assert_arg ("--vm-hang");
          do_vm_hang = atoll_b (arg);
          if (do_vm_hang < 0)
            {
              err (stderr, "invalid value: %lli\n", do_vm_hang);
              exit (1);
            }
        }
      else if (strcmp (arg, "--vm-keep") == 0)
        {
          do_vm_keep = 1;
        }
      else if (strcmp (arg, "--hdd") == 0 || strcmp (arg, "-d") == 0)
        {
          assert_arg ("--hdd");
          do_hdd = atoll_b (arg);
          if (do_hdd <= 0)
            {
              err (stderr, "invalid number of hdd hogs: %lli\n", do_hdd);
              exit (1);
            }
        }
      else if (strcmp (arg, "--hdd-bytes") == 0)
        {
          assert_arg ("--hdd-bytes");
          do_hdd_bytes = atoll_b (arg);
          if (do_hdd_bytes <= 0)
            {
              err (stderr, "invalid hdd byte value: %lli\n", do_hdd_bytes);
              exit (1);
            }
        }
      else
        {
          err (stderr, "unrecognized option: %s\n", arg);
          exit (1);
        }
    }

  /* Print startup message if we have work to do, bail otherwise.  */
  if (do_cpu + do_io + do_vm + do_hdd)
    {
      out (stdout, "dispatching hogs: %lli cpu, %lli io, %lli vm, %lli hdd\n",
           do_cpu, do_io, do_vm, do_hdd);
    }
  else
    usage (0);

  /* Round robin dispatch our worker processes.  */
  while ((forks = (do_cpu + do_io + do_vm + do_hdd)))
    {
      long long backoff, timeout = 0;

      /* Calculate the backoff value so we get good fork throughput.  */
      backoff = do_backoff * forks;
      dbg (stdout, "using backoff sleep of %llius\n", backoff);

      /* If we are supposed to respect a timeout, calculate it.  */
      if (do_timeout)
        {
          long long currenttime;

          /* Acquire current time.  */
          if ((currenttime = time (NULL)) == -1)
            {
              perror ("error acquiring current time");
              exit (1);
            }

          /* Calculate timeout based on current time.  */
          timeout = do_timeout - (currenttime - starttime);

          if (timeout > 0)
            {
              dbg (stdout, "setting timeout to %llis\n", timeout);
            }
          else
            {
              wrn (stderr, "used up time before all workers dispatched\n");
              break;
            }
        }

      if (do_cpu)
        {
          switch (pid = fork ())
            {
            case 0:            /* child */
              alarm (timeout);
              usleep (backoff);
              if (do_dryrun)
                exit (0);
              exit (hogcpu ());
            case -1:           /* error */
              err (stderr, "fork failed: %s\n", strerror (errno));
              break;
            default:           /* parent */
              dbg (stdout, "--> hogcpu worker %lli [%i] forked\n",
                   do_cpu, pid);
              ++children;
            }
          --do_cpu;
        }

      if (do_io)
        {
          switch (pid = fork ())
            {
            case 0:            /* child */
              alarm (timeout);
              usleep (backoff);
              if (do_dryrun)
                exit (0);
              exit (hogio ());
            case -1:           /* error */
              err (stderr, "fork failed: %s\n", strerror (errno));
              break;
            default:           /* parent */
              dbg (stdout, "--> hogio worker %lli [%i] forked\n", do_io, pid);
              ++children;
            }
          --do_io;
        }

      if (do_vm)
        {
          switch (pid = fork ())
            {
            case 0:            /* child */
              alarm (timeout);
              usleep (backoff);
              if (do_dryrun)
                exit (0);
              exit (hogvm
                    (do_vm_bytes, do_vm_stride, do_vm_hang, do_vm_keep));
            case -1:           /* error */
              err (stderr, "fork failed: %s\n", strerror (errno));
              break;
            default:           /* parent */
              dbg (stdout, "--> hogvm worker %lli [%i] forked\n", do_vm, pid);
              ++children;
            }
          --do_vm;
        }

      if (do_hdd)
        {
          switch (pid = fork ())
            {
            case 0:            /* child */
              alarm (timeout);
              usleep (backoff);
              if (do_dryrun)
                exit (0);
              exit (hoghdd (do_hdd_bytes));
            case -1:           /* error */
              err (stderr, "fork failed: %s\n", strerror (errno));
              break;
            default:           /* parent */
              dbg (stdout, "--> hoghdd worker %lli [%i] forked\n",
                   do_hdd, pid);
              ++children;
            }
          --do_hdd;
        }
    }

  /* Wait for our children to exit.  */
  while (children)
    {
      int status, ret;

      if ((pid = wait (&status)) > 0)
        {
          --children;

          if (WIFEXITED (status))
            {
              if ((ret = WEXITSTATUS (status)) == 0)
                {
                  dbg (stdout, "<-- worker %i returned normally\n", pid);
                }
              else
                {
                  err (stderr, "<-- worker %i returned error %i\n", pid, ret);
                  ++retval;
                  wrn (stderr, "now reaping child worker processes\n");
                  if (signal (SIGUSR1, SIG_IGN) == SIG_ERR)
                    err (stderr, "handler error: %s\n", strerror (errno));
                  if (kill (-1 * getpid (), SIGUSR1) == -1)
                    err (stderr, "kill error: %s\n", strerror (errno));
                }
            }
          else if (WIFSIGNALED (status))
            {
              if ((ret = WTERMSIG (status)) == SIGALRM)
                {
                  dbg (stdout, "<-- worker %i signalled normally\n", pid);
                }
              else if ((ret = WTERMSIG (status)) == SIGUSR1)
                {
                  dbg (stdout, "<-- worker %i reaped\n", pid);
                }
              else
                {
                  err (stderr, "<-- worker %i got signal %i\n", pid, ret);
                  ++retval;
                  wrn (stderr, "now reaping child worker processes\n");
                  if (signal (SIGUSR1, SIG_IGN) == SIG_ERR)
                    err (stderr, "handler error: %s\n", strerror (errno));
                  if (kill (-1 * getpid (), SIGUSR1) == -1)
                    err (stderr, "kill error: %s\n", strerror (errno));
                }
            }
          else
            {
              err (stderr, "<-- worker %i exited abnormally\n", pid);
              ++retval;
            }
        }
      else
        {
          err (stderr, "error waiting for worker: %s\n", strerror (errno));
          ++retval;
          break;
        }
    }

  /* Record our stop time.  */
  if ((stoptime = time (NULL)) == -1)
    {
      err (stderr, "failed to acquire current time\n");
      exit (1);
    }

  /* Calculate our runtime.  */
  runtime = stoptime - starttime;

  /* Print final status message.  */
  if (retval)
    {
      err (stderr, "failed run completed in %lis\n", runtime);
    }
  else
    {
      out (stdout, "successful run completed in %lis\n", runtime);
    }

  exit (retval);
}

int
hogcpu (void)
{
  while (1)
    sqrt (rand ());

  return 0;
}

int
hogio ()
{
  while (1)
    sync ();

  return 0;
}

int
hogvm (long long bytes, long long stride, long long hang, int keep)
{
  long long i;
  char *ptr = 0;
  char c;
  int do_malloc = 1;

  while (1)
    {
      if (do_malloc)
        {
          dbg (stdout, "allocating %lli bytes ...\n", bytes);
          if (!(ptr = (char *) malloc (bytes * sizeof (char))))
            {
              err (stderr, "hogvm malloc failed: %s\n", strerror (errno));
              return 1;
            }
          if (keep)
            do_malloc = 0;
        }

      dbg (stdout, "touching bytes in strides of %lli bytes ...\n", stride);
      for (i = 0; i < bytes; i += stride)
        ptr[i] = 'Z';           /* Ensure that COW happens.  */

      if (hang == 0)
        {
          dbg (stdout, "sleeping forever with allocated memory\n");
          while (1)
            sleep (1024);
        }
      else if (hang > 0)
        {
          dbg (stdout, "sleeping for %llis with allocated memory\n", hang);
          sleep (hang);
        }

      for (i = 0; i < bytes; i += stride)
        {
          c = ptr[i];
          if (c != 'Z')
            {
              err (stderr, "memory corruption at: %p\n", ptr + i);
              return 1;
            }
        }

      if (do_malloc)
        {
          free (ptr);
          dbg (stdout, "freed %lli bytes\n", bytes);
        }
    }

  return 0;
}

int
hoghdd (long long bytes)
{
  long long i, j;
  int fd;
  int chunk = (1024 * 1024) - 1;        /* Minimize slow writing.  */
  char buff[chunk];

  /* Initialize buffer with some random ASCII data.  */
  dbg (stdout, "seeding %d byte buffer with random data\n", chunk);
  for (i = 0; i < chunk - 1; i++)
    {
      j = rand ();
      j = (j < 0) ? -j : j;
      j %= 95;
      j += 32;
      buff[i] = j;
    }
  buff[i] = '\n';

  while (1)
    {
      char name[] = "./stress.XXXXXX";

      if ((fd = mkstemp (name)) == -1)
        {
          err (stderr, "mkstemp failed: %s\n", strerror (errno));
          return 1;
        }

      dbg (stdout, "opened %s for writing %lli bytes\n", name, bytes);

      dbg (stdout, "unlinking %s\n", name);
      if (unlink (name) == -1)
        {
          err (stderr, "unlink of %s failed: %s\n", name, strerror (errno));
          return 1;
        }

      dbg (stdout, "fast writing to %s\n", name);
      for (j = 0; bytes == 0 || j + chunk < bytes; j += chunk)
        {
          if (write (fd, buff, chunk) == -1)
            {
              err (stderr, "write failed: %s\n", strerror (errno));
              return 1;
            }
        }

      dbg (stdout, "slow writing to %s\n", name);
      for (; bytes == 0 || j < bytes - 1; j++)
        {
          if (write (fd, &buff[j % chunk], 1) == -1)
            {
              err (stderr, "write failed: %s\n", strerror (errno));
              return 1;
            }
        }
      if (write (fd, "\n", 1) == -1)
        {
          err (stderr, "write failed: %s\n", strerror (errno));
          return 1;
        }
      ++j;

      dbg (stdout, "closing %s after %lli bytes\n", name, j);
      close (fd);
    }

  return 0;
}

/* Convert a string representation of a number with an optional size suffix
 * to a long long.
 */
long long
atoll_b (const char *nptr)
{
  int pos;
  char suffix;
  long long factor = 0;
  long long value;

  if ((pos = strlen (nptr) - 1) < 0)
    {
      err (stderr, "invalid string\n");
      exit (1);
    }

  switch (suffix = nptr[pos])
    {
    case 'b':
    case 'B':
      factor = 0;
      break;
    case 'k':
    case 'K':
      factor = 10;
      break;
    case 'm':
    case 'M':
      factor = 20;
      break;
    case 'g':
    case 'G':
      factor = 30;
      break;
    default:
      if (suffix < '0' || suffix > '9')
        {
          err (stderr, "unrecognized suffix: %c\n", suffix);
          exit (1);
        }
    }

  if (sscanf (nptr, "%lli", &value) != 1)
    {
      err (stderr, "invalid number: %s\n", nptr);
      exit (1);
    }

  value = value << factor;

  return value;
}

/* Convert a string representation of a number with an optional time suffix
 * to a long long.
 */
long long
atoll_s (const char *nptr)
{
  int pos;
  char suffix;
  long long factor = 1;
  long long value;

  if ((pos = strlen (nptr) - 1) < 0)
    {
      err (stderr, "invalid string\n");
      exit (1);
    }

  switch (suffix = nptr[pos])
    {
    case 's':
    case 'S':
      factor = 1;
      break;
    case 'm':
    case 'M':
      factor = 60;
      break;
    case 'h':
    case 'H':
      factor = 60 * 60;
      break;
    case 'd':
    case 'D':
      factor = 60 * 60 * 24;
      break;
    case 'y':
    case 'Y':
      factor = 60 * 60 * 24 * 365;
      break;
    default:
      if (suffix < '0' || suffix > '9')
        {
          err (stderr, "unrecognized suffix: %c\n", suffix);
          exit (1);
        }
    }

  if (sscanf (nptr, "%lli", &value) != 1)
    {
      err (stderr, "invalid number: %s\n", nptr);
      exit (1);
    }

  value = value * factor;

  return value;
}

int
version (int status)
{
  char *mesg = "%s %s\n";

  fprintf (stdout, mesg, global_progname, VERSION);

  if (status <= 0)
    exit (-1 * status);

  return 0;
}

int
usage (int status)
{
  char *mesg =
    "`%s' imposes certain types of compute stress on your system\n\n"
    "Usage: %s [OPTION [ARG]] ...\n"
    " -?, --help         show this help statement\n"
    "     --version      show version statement\n"
    " -v, --verbose      be verbose\n"
    " -q, --quiet        be quiet\n"
    " -n, --dry-run      show what would have been done\n"
    " -t, --timeout N    timeout after N seconds\n"
    "     --backoff N    wait factor of N microseconds before work starts\n"
    " -c, --cpu N        spawn N workers spinning on sqrt()\n"
    " -i, --io N         spawn N workers spinning on sync()\n"
    " -m, --vm N         spawn N workers spinning on malloc()/free()\n"
    "     --vm-bytes B   malloc B bytes per vm worker (default is 256MB)\n"
    "     --vm-stride B  touch a byte every B bytes (default is 4096)\n"
    "     --vm-hang N    sleep N secs before free (default none, 0 is inf)\n"
    "     --vm-keep      redirty memory instead of freeing and reallocating\n"
    " -d, --hdd N        spawn N workers spinning on write()/unlink()\n"
    "     --hdd-bytes B  write B bytes per hdd worker (default is 1GB)\n\n"
    "Example: %s --cpu 8 --io 4 --vm 2 --vm-bytes 128M --timeout 10s\n\n"
    "Note: Numbers may be suffixed with s,m,h,d,y (time) or B,K,M,G (size).\n";

  fprintf (stdout, mesg, global_progname, global_progname, global_progname);

  if (status <= 0)
    exit (-1 * status);

  return 0;
}
