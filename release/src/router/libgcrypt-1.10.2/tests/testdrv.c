/* testdrv.c - Test driver to run all tests w/o using the Makefile.
 * Copyright (C) 2021 g10 Code GmbH
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifndef HAVE_W32_SYSTEM
# include <unistd.h>
# include <fcntl.h>
# include <sys/wait.h>
#endif
#include <gpg-error.h> /* For some macros.  */

#include "stopwatch.h"

#define PGM "testdrv"

/* Flags for testpgms.  */
#define LONG_RUNNING 1

/* This is our list of tests which are run in this order.  */
static struct {
  const char *name;
  const char *pgm;
  const char *args;
  unsigned int flags;  /* e.g. LONG_RUNNING */
} testpgms[] =
  {
   { "version"     },
   { "t-secmem"    },
   { "mpitests"    },
   { "t-sexp"      },
   { "t-convert"   },
   { "t-mpi-bit"   },
   { "t-mpi-point" },
   { "curves"      },
   { "t-lock"      },
   { "prime"       },
   { "basic"       },
   { "basic-disable-all-hwf", "basic", "--disable-hwf all" },
   { "keygen"      },
   { "pubkey"      },
   { "hmac"        },
   { "hashtest"    },
   { "t-kdf"       },
   { "keygrip"     },
   { "fips186-dsa" },
   { "aeswrap"     },
   { "pkcs1v2"     },
   { "random"      },
   { "dsa-rfc6979" },
   { "t-ed25519"   },
   { "t-cv25519"   },
   { "t-x448"      },
   { "t-ed448"     },
   { "benchmark"   },
   { "bench-slope" },
   { "hashtest-256g",  "hashtest", "--gigs 256 SHA1 SHA256 SHA512 SM3",
     LONG_RUNNING },
   { NULL }
  };

/* Extra files needed for the above tests.  */
static const char *extratestfiles[] =
  {
   "t-ed25519.inp",
   "t-ed448.inp",
   NULL
  };


/* A couple of useful macros.  */
#ifndef DIM
# define DIM(v)		     (sizeof(v)/sizeof((v)[0]))
#endif
#define DIMof(type,member)   DIM(((type *)0)->member)
#define xfree(a)      free ((a))
#define spacep(p)   (*(p) == ' ' || *(p) == '\t')

/* If we have a decent libgpg-error we can use some gcc attributes.  */
#ifdef GPGRT_ATTR_NORETURN
static void die (const char *format, ...)
  GPGRT_ATTR_UNUSED GPGRT_ATTR_NR_PRINTF(1,2);
static void fail (const char *format, ...)
  GPGRT_ATTR_UNUSED GPGRT_ATTR_PRINTF(1,2);
static void info (const char *format, ...) \
  GPGRT_ATTR_UNUSED GPGRT_ATTR_PRINTF(1,2);
static void printresult (const char *format, ...) \
  GPGRT_ATTR_UNUSED GPGRT_ATTR_PRINTF(1,2);
#endif /*GPGRT_ATTR_NORETURN*/


#ifndef TESTDRV_EXEEXT
# ifdef HAVE_W32_SYSTEM
#  define TESTDRV_EXEEXT ".exe"
# else
#  define TESTDRV_EXEEXT ""
# endif
#endif
#ifdef HAVE_W32_SYSTEM
# define MYPID_T HANDLE
# define MYINVALID_PID INVALID_HANDLE_VALUE
#else
# define MYPID_T pid_t
# define MYINVALID_PID ((pid_t)(-1))
#endif

/* Standard global variables.  */
static int verbose;
static int debug;
static int error_count;
static int die_on_error;
static int long_running;
static char **myenviron;
static int testcount, failcount, passcount, skipcount;

#ifdef HAVE_W32_SYSTEM
static char *
my_stpcpy (char *a, const char *b)
{
  while (*b)
    *a++ = *b++;
  *a = 0;

  return a;
}
#endif /*HAVE_W32_SYSTEM*/

/* Reporting functions.  */
static void
die (const char *format, ...)
{
  va_list arg_ptr ;

  /* Avoid warning.  */
  (void) debug;

  fflush (stdout);
#ifdef HAVE_FLOCKFILE
  flockfile (stderr);
#endif
  fprintf (stderr, "%s: ", PGM);
  va_start (arg_ptr, format) ;
  vfprintf (stderr, format, arg_ptr);
  va_end (arg_ptr);
  if (*format && format[strlen(format)-1] != '\n')
    putc ('\n', stderr);
#ifdef HAVE_FLOCKFILE
  funlockfile (stderr);
#endif
  exit (1);
}

static void *
xmalloc (size_t n)
{
  char *p = malloc (n);
  if (!p)
    die ("malloc failed");
  return p;
}

static void *
xcalloc (size_t n, size_t m)
{
  char *p = calloc (n, m);
  if (!p)
    die ("calloc failed");
  return p;
}

static char *
xstrdup (const char *s)
{
  size_t n = strlen (s);
  char *p = xmalloc (n+1);
  strcpy (p, s);
  return p;
}


static void
fail (const char *format, ...)
{
  va_list arg_ptr;

  fflush (stdout);
#ifdef HAVE_FLOCKFILE
  flockfile (stderr);
#endif
  fprintf (stderr, "%s: ", PGM);
  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  va_end (arg_ptr);
  if (*format && format[strlen(format)-1] != '\n')
    putc ('\n', stderr);
#ifdef HAVE_FLOCKFILE
  funlockfile (stderr);
#endif
  if (die_on_error)
    exit (1);
  error_count++;
}


static void
info (const char *format, ...)
{
  va_list arg_ptr;

  if (!verbose)
    return;
  fflush (stdout);
#ifdef HAVE_FLOCKFILE
  flockfile (stderr);
#endif
  fprintf (stderr, "%s: ", PGM);
  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  if (*format && format[strlen(format)-1] != '\n')
    putc ('\n', stderr);
  va_end (arg_ptr);
#ifdef HAVE_FLOCKFILE
  funlockfile (stderr);
#endif
}


static void
printresult (const char *format, ...)
{
  va_list arg_ptr;

  fflush (stdout);
#ifdef HAVE_FLOCKFILE
  flockfile (stdout);
#endif
  va_start (arg_ptr, format);
  vfprintf (stdout, format, arg_ptr);
  if (*format && format[strlen(format)-1] != '\n')
    putc ('\n', stdout);
  va_end (arg_ptr);
  fflush (stdout);
#ifdef HAVE_FLOCKFILE
  funlockfile (stdout);
#endif
}


/* Tokenize STRING using the set of delimiters in DELIM.  Leading
 * spaces and tabs are removed from all tokens.  The caller must free
 * the result. Returns a malloced and NULL delimited array with the
 * tokens.  */
static char **
strtokenize (const char *string, const char *delim)
{
  const char *s;
  size_t fields;
  size_t bytes, n;
  char *buffer;
  char *p, *px, *pend;
  char **result;

  /* Count the number of fields.  */
  for (fields = 1, s = strpbrk (string, delim); s; s = strpbrk (s + 1, delim))
    fields++;
  fields++; /* Add one for the terminating NULL.  */

  /* Allocate an array for all fields, a terminating NULL, and space
     for a copy of the string.  */
  bytes = fields * sizeof *result;
  if (bytes / sizeof *result != fields)
    die ("integer overflow at %d\n", __LINE__);
  n = strlen (string) + 1;
  bytes += n;
  if (bytes < n)
    die ("integer overflow at %d\n", __LINE__);
  result = xmalloc (bytes);
  buffer = (char*)(result + fields);

  /* Copy and parse the string.  */
  strcpy (buffer, string);
  for (n = 0, p = buffer; (pend = strpbrk (p, delim)); p = pend + 1)
    {
      *pend = 0;
      while (spacep (p))
        p++;
      for (px = pend - 1; px >= p && spacep (px); px--)
        *px = 0;
      result[n++] = p;
    }
  while (spacep (p))
    p++;
  for (px = p + strlen (p) - 1; px >= p && spacep (px); px--)
    *px = 0;
  result[n++] = p;
  result[n] = NULL;

  if (!((char*)(result + n + 1) == buffer))
    die ("bug at %d\n", __LINE__);

  return result;
}


#ifdef HAVE_W32_SYSTEM
/* Helper functions for Windows. */
static char *
build_w32_commandline_copy (char *buffer, const char *string)
{
  char *p = buffer;
  const char *s;

  if (!*string) /* Empty string. */
    p = my_stpcpy (p, "\"\"");
  else if (strpbrk (string, " \t\n\v\f\""))
    {
      /* Need to do some kind of quoting.  */
      p = my_stpcpy (p, "\"");
      for (s=string; *s; s++)
        {
          *p++ = *s;
          if (*s == '\"')
            *p++ = *s;
        }
      *p++ = '\"';
      *p = 0;
    }
  else
    p = my_stpcpy (p, string);

  return p;
}

/* Build a command line for use with CreateProcess.  This function
 * either terminates the process or returns a malloced string.  */
static char *
build_w32_commandline (const char *pgmname, char **argv)
{
  int i, n;
  const char *s;
  char *buf, *p;

  s = pgmname;
  n = strlen (s) + 1 + 2;  /* (1 space, 2 quoting) */
  for (; *s; s++)
    if (*s == '\"')
      n++;  /* Account for to be doubled inner quotes.  */
  for (i=0; argv && (s=argv[i]); i++)
    {
      n += strlen (s) + 1 + 2;  /* (1 space, 2 quoting) */
      for (; *s; s++)
        if (*s == '\"')
          n++;  /* For doubling inner quotes.  */
    }
  n++;  /* String terminator.  */

  buf = p = xmalloc (n);
  p = build_w32_commandline_copy (p, pgmname);
  for (i=0; argv && argv[i]; i++)
    {
      *p++ = ' ';
      p = build_w32_commandline_copy (p, argv[i]);
    }

  return buf;
}

static HANDLE
w32_open_null (int for_write)
{
  HANDLE hfile;

  hfile = CreateFileW (L"nul",
                       for_write? GENERIC_WRITE : GENERIC_READ,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, 0, NULL);
  if (hfile == INVALID_HANDLE_VALUE)
    die ("can't open 'nul': ec=%lu\n", (unsigned long)GetLastError());
  return hfile;
}

#endif /*HAVE_W32_SYSTEM*/


/* Fork and exec the PGMNAME using ARGV as arguments (w/o pgmmname)
 * and return the pid at PID.  If ENVP is not NULL, add these strings
 * as environment variables.  Return -1 on severe errors. */
static int
my_spawn (const char *pgmname, char **argv, char **envp, MYPID_T *pid)
{
#ifdef HAVE_W32_SYSTEM
  int rc;
  SECURITY_ATTRIBUTES sec_attr;
  PROCESS_INFORMATION pi = { NULL };
  STARTUPINFO si;
  char *cmdline;
  char *pgmnamefull = NULL;
  char **saveenviron = NULL;
  int i;

  /* Prepare security attributes.  */
  memset (&sec_attr, 0, sizeof sec_attr );
  sec_attr.nLength = sizeof sec_attr;
  sec_attr.bInheritHandle = FALSE;

  if (!(strlen (pgmname) > 4 && !strcmp (pgmname+strlen(pgmname)-4, ".exe")))
    {
      pgmnamefull = xmalloc (strlen (pgmname) + 4 + 1);
      strcpy (my_stpcpy (pgmnamefull, pgmname), ".exe");
      pgmname = pgmnamefull;
    }

  /* Build the command line.  */
  cmdline = build_w32_commandline (pgmname, argv);

  memset (&si, 0, sizeof si);
  si.cb = sizeof (si);
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdInput  = w32_open_null (0);
  if (verbose)
    si.hStdOutput = GetStdHandle (STD_OUTPUT_HANDLE);
  else
    si.hStdOutput = w32_open_null (1);
  si.hStdError  = GetStdHandle (STD_ERROR_HANDLE);

  if (envp)
    {
      for (i=0; envp[i]; i++)
        ;
      saveenviron = xcalloc (i+1, sizeof *saveenviron);
      for (i=0; envp[i]; i++)
        saveenviron[i] = xstrdup (envp[i]);
      for (i=0; envp[i]; i++)
        putenv (envp[i]);
    }

  if (debug)
    info ("CreateProcess, path='%s' cmdline='%s'\n", pgmname, cmdline);
  if (!CreateProcess (pgmname,       /* Program to start.  */
                      cmdline,       /* Command line arguments.  */
                      &sec_attr,     /* Process security attributes.  */
                      &sec_attr,     /* Thread security attributes.  */
                      TRUE,          /* Inherit handles.  */
                      (CREATE_DEFAULT_ERROR_MODE
                       | GetPriorityClass (GetCurrentProcess ())
                       | CREATE_SUSPENDED | DETACHED_PROCESS),
                      NULL,          /* Environment.  */
                      NULL,          /* Use current drive/directory.  */
                      &si,           /* Startup information. */
                      &pi            /* Returns process information.  */
                      ))
    {
      fail ("CreateProcess failed: ec=%lu\n", (unsigned long)GetLastError());
      rc = -1;
    }
  else
    rc = 0;

  if (saveenviron)
    {
      for (i=0; saveenviron[i]; i++)
        xfree (saveenviron[i]);
      xfree (saveenviron);
    }
  xfree (cmdline);
  CloseHandle (si.hStdInput);
  if (!verbose)
    CloseHandle (si.hStdOutput);
  xfree (pgmnamefull); pgmname = NULL;
  if (rc)
    return rc;

  if (debug)
    info ("CreateProcess ready: hProcess=%p hThread=%p"
          " dwProcessID=%d dwThreadId=%d\n",
          pi.hProcess, pi.hThread,
          (int) pi.dwProcessId, (int) pi.dwThreadId);

  /* Process has been created suspended; resume it now. */
  ResumeThread (pi.hThread);
  CloseHandle (pi.hThread);

  *pid = pi.hProcess;
  return 0;

#else /*!HAVE_W32_SYSTEM*/

  char **arg_list;
  int i, j;
  int fd;

  /* Create the command line argument array.  */
  i = 0;
  if (argv)
    while (argv[i])
      i++;
  arg_list = xcalloc (i+2, sizeof *arg_list);
  arg_list[0] = strrchr (pgmname, '/');
  if (arg_list[0])
    arg_list[0]++;
  else
    arg_list[0] = xstrdup (pgmname);
  if (argv)
    for (i=0,j=1; argv[i]; i++, j++)
      arg_list[j] = (char*)argv[i];


  *pid = fork ();
  if (*pid == MYINVALID_PID)
    {
      xfree (arg_list);
      fail ("error forking process: %s\n", strerror (errno));
      return -1;
    }

  if (!*pid)
    {
      /* This is the child. */
      if (envp)
        for (i=0; envp[i]; i++)
          putenv (xstrdup (envp[i]));

      /* Assign /dev/null to stdin. */
      fd = open ("/dev/null", O_RDONLY);
      if (fd == -1)
        {
          xfree (arg_list);
          die ("failed to open '%s': %s\n", "/dev/null", strerror (errno));
        }
      if (fd != 0 && dup2 (fd, 0) == -1)
        {
          xfree (arg_list);
          die ("dup2(%d,0) failed: %s\n", fd, strerror (errno));
        }
      /* Assign /dev/null to stdout unless in verbose mode. */
      if (!verbose)
        {
          fd = open ("/dev/null", O_RDONLY);
          if (fd == -1)
            {
              xfree (arg_list);
              die ("failed to open '%s': %s\n", "/dev/null", strerror (errno));
            }
          if (fd != 1 && dup2 (fd, 1) == -1)
            {
              xfree (arg_list);
              die ("dup2(%d,1) failed: %s\n", fd, strerror (errno));
            }
        }

      /* Exec the program.  */
      execv (pgmname, arg_list);
      info ("exec '%s' failed: %s\n", pgmname, strerror (errno));
      _exit (127);
      /*NOTREACHED*/
    }

  /* This is the parent. */
  xfree (arg_list);
  return 0;
#endif /*!HAVE_W32_SYSTEM*/
}


/* Wait for PID and return its exitcode at R_EXITCODE.  PGMNAME is
 * only used for diagnostics.  */
static int
my_wait (const char *pgmname, MYPID_T pid, int *r_exitcode)
{
  int rc = -1;

#ifdef HAVE_W32_SYSTEM
  HANDLE procs[1];
  DWORD exc;
  int code;

  if (pid == MYINVALID_PID)
    die ("invalid pid passed to my_wait\n");

  procs[0] = (HANDLE)pid;
  code = WaitForMultipleObjects (1, procs, TRUE, INFINITE);
  switch (code)
    {
    case WAIT_TIMEOUT: /* Should not happen.  */
      fail ("waiting for process %p (%s) to terminate failed: timeout\n",
            pid, pgmname);
      break;

    case WAIT_FAILED:
      fail ("waiting for process %p (%s) to terminate failed: ec=%lu\n",
            pid, pgmname, (unsigned long)GetLastError ());
      break;

    case WAIT_OBJECT_0:
      if (!GetExitCodeProcess (procs[0], &exc))
        {
          fail ("error getting exit code for process %p (%s): ec=%lu\n",
                pid, pgmname, (unsigned long)GetLastError ());
        }
      else
        {
          *r_exitcode = (int)exc;
          rc = 0;
        }
      break;

    default:
      fail ("WaitForMultipleObjects returned unexpected code %d\n", code);
      break;
    }
  CloseHandle ((HANDLE)pid);

#else /*!HAVE_W32_SYSTEM*/

  int i, status;

  if (pid == MYINVALID_PID)
    die ("invalid pid passed to my_wait\n");

  while ((i=waitpid (pid, &status, 0)) == MYINVALID_PID
         && errno == EINTR)
    ;

  if (i == MYINVALID_PID)
    {
      fail ("waiting for process %d (%s) to terminate failed: %s\n",
            (int)pid, pgmname, strerror (errno));
    }
  else if (!i)
    {
      die ("waitpid returns unexpected code 0\n");
    }
  else if (WIFEXITED (status) && WEXITSTATUS (status) == 127)
    {
      fail ("error running '%s': probably not installed\n", pgmname);
    }
  else if (WIFEXITED (status) && WEXITSTATUS (status))
    {
      *r_exitcode = WEXITSTATUS (status);
      rc = 0;
    }
  else if (!WIFEXITED (status))
    {
      info ("error running '%s': terminated\n", pgmname);
      rc = 1;
    }
  else
    {
      *r_exitcode = 0;
      rc = 0;
    }

#endif /*!HAVE_W32_SYSTEM*/

  return rc;
}


static void
run_one_test (int idx)
{
  MYPID_T pid;
  int exitcode, rc;
  const char *name = testpgms[idx].name;
  const char *pgm = testpgms[idx].pgm;
  char **args;

  if (!pgm)
    pgm = name;

  testcount++;
  if ((testpgms[idx].flags & LONG_RUNNING)
      && !long_running)
    {
      printresult ("SKIP: %s\n", name);
      skipcount++;
      return;
    }

  args = testpgms[idx].args? strtokenize (testpgms[idx].args, " ") : NULL;
  rc = my_spawn (pgm, args, myenviron, &pid);
  xfree (args);
  if (rc)
    {
      printresult ("FAIL: %s (error invoking test)\n", name);
      failcount++;
      return;
    }
  rc = my_wait (pgm, pid, &exitcode);
  if (rc < 0)
    {
      printresult ("FAIL: %s (error running test)\n", name);
      failcount++;
    }
  else if (rc)
    {
      printresult ("FAIL: %s (test crashed)\n", name);
      failcount++;
    }
  else if (exitcode == 77)
    {
      printresult ("SKIP: %s\n", name);
      skipcount++;
    }
  else if (exitcode == 1)
    {
      printresult ("FAIL: %s\n", name);
      failcount++;
    }
  else if (exitcode)
    {
      printresult ("FAIL: %s (exit code %d)\n", name, exitcode);
      failcount++;
    }
  else
    {
      printresult ("PASS: %s\n", name);
      passcount++;
    }
}



static void
runtests (char **argv)
{
  int i;

  if (argv && *argv)
    {
      for ( ; *argv; argv++)
        {
          for (i=0; testpgms[i].name; i++)
            if (!strcmp (testpgms[i].name, *argv))
              {
                run_one_test (i);
                break;
              }
          if (!testpgms[i].name)
            {
              fail ("requested test '%s' not found\n", *argv);
              testcount++;
            }
        }
    }
  else /* Run all tests.  */
    {
      for (i=0; testpgms[i].name; i++)
        run_one_test (i);
    }
}


int
main (int argc, char **argv)
{
  int last_argc = -1;
  int listtests = 0;
  int i;
  const char *srcdir;

  if (argc)
    { argc--; argv++; }

  while (argc && last_argc != argc )
    {
      last_argc = argc;
      if (!strcmp (*argv, "--"))
        {
          argc--; argv++;
          break;
        }
      else if (!strcmp (*argv, "--help"))
        {
          fputs ("usage: " PGM " [options] [tests_to_run]\n"
                 "Options:\n"
                 "  --verbose       print timings etc.\n"
                 "  --debug         flyswatter\n"
                 "  --list          list all tests\n"
                 "  --files         list all files\n"
                 "  --long          include long running tests\n"
                 , stdout);
          exit (0);
        }
      else if (!strcmp (*argv, "--verbose"))
        {
          verbose++;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--debug"))
        {
          verbose += 2;
          debug++;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--list"))
        {
          listtests = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--files"))
        {
          listtests = 2;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--long"))
        {
          long_running = 1;
          argc--; argv++;
        }
      else if (!strncmp (*argv, "--", 2))
        die ("unknown option '%s'", *argv);
    }

  srcdir = getenv ("srcdir");

  myenviron = xcalloc (2, sizeof *myenviron);
  myenviron[0] = xstrdup ("GCRYPT_IN_REGRESSION_TEST=1");
#ifndef HAVE_W32_SYSTEM
  if (!access ("libgcrypt-standalone-tests", F_OK))
    myenviron[1] = xstrdup ("LD_LIBRARY_PATH=.");
#endif

  if (listtests == 1)
    {
      for (i=0; testpgms[i].name; i++)
        {
          printf ("%s", testpgms[i].name);
          if (testpgms[i].pgm || testpgms[i].args)
            printf (" (%s %s)",
                    testpgms[i].pgm? testpgms[i].pgm : testpgms[i].name,
                    testpgms[i].args? testpgms[i].args : "");
          if (testpgms[i].flags)
            {
              putchar (' ');
              putchar ('[');
              if (testpgms[i].flags)
                fputs ("long", stdout);
              putchar (']');
            }
          putchar ('\n');
        }
    }
  else if (listtests == 2)
    {
      for (i=0; testpgms[i].name; i++)
        printf ("%s%s%s\n",
                strcmp (TESTDRV_EXEEXT, ".exe")? "":".libs/",
                testpgms[i].pgm? testpgms[i].pgm : testpgms[i].name,
                TESTDRV_EXEEXT);
      for (i=0; extratestfiles[i]; i++)
        printf ("%s%s%s\n",
                srcdir? srcdir :"",
                srcdir? "/" :"",
                extratestfiles[i]);
    }
  else
    {
      start_timer ();
      runtests (argv);
      stop_timer ();

      printresult ("%d tests run, %d succeeded, %d failed, %d skipped.\n",
                   testcount-skipcount, passcount, failcount, skipcount);
      if (testcount != passcount + failcount + skipcount)
        printresult ("Warning: Execution of some tests failed\n");

      info ("All tests completed in %s.  Errors: %d\n",
            elapsed_time (1), error_count + failcount);
    }

  for (i=0; myenviron[i]; i++)
    xfree (myenviron[i]);
  xfree (myenviron);

  return !!error_count;
}
