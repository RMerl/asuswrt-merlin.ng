/* random.c - part of the Libgcrypt test suite.
   Copyright (C) 2005 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.  */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#ifndef HAVE_W32_SYSTEM
# include <signal.h>
# include <sys/wait.h>
#endif

#include "stopwatch.h"


#define PGM "random"
#define NEED_EXTRA_TEST_SUPPORT 1
#include "t-common.h"

static int with_progress;


/* Prepend FNAME with the srcdir environment variable's value and
 * return an allocated filename.  */
static char *
prepend_srcdir (const char *fname)
{
  static const char *srcdir;
  char *result;

  if (!srcdir && !(srcdir = getenv ("srcdir")))
    srcdir = ".";

  result = xmalloc (strlen (srcdir) + 1 + strlen (fname) + 1);
  strcpy (result, srcdir);
  strcat (result, "/");
  strcat (result, fname);
  return result;
}


static void
print_hex (const char *text, const void *buf, size_t n)
{
  const unsigned char *p = buf;

  info ("%s", text);
  for (; n; n--, p++)
    fprintf (stderr, "%02X", *p);
  putc ('\n', stderr);
}


static void
progress_cb (void *cb_data, const char *what, int printchar,
             int current, int total)
{
  (void)cb_data;

  info ("progress (%s %c %d %d)\n", what, printchar, current, total);
  fflush (stderr);
}


#ifndef HAVE_W32_SYSTEM
static int
writen (int fd, const void *buf, size_t nbytes)
{
  size_t nleft = nbytes;
  int nwritten;

  while (nleft > 0)
    {
      nwritten = write (fd, buf, nleft);
      if (nwritten < 0)
        {
          if (errno == EINTR)
            nwritten = 0;
          else
            return -1;
        }
      nleft -= nwritten;
      buf = (const char*)buf + nwritten;
    }

  return 0;
}
#endif /*!HAVE_W32_SYSTEM*/


#ifndef HAVE_W32_SYSTEM
static int
readn (int fd, void *buf, size_t buflen, size_t *ret_nread)
{
  size_t nleft = buflen;
  int nread;

  while ( nleft > 0 )
    {
      nread = read ( fd, buf, nleft );
      if (nread < 0)
        {
          if (nread == EINTR)
            nread = 0;
          else
            return -1;
        }
      else if (!nread)
        break; /* EOF */
      nleft -= nread;
      buf = (char*)buf + nread;
    }
  if (ret_nread)
    *ret_nread = buflen - nleft;
  return 0;
}
#endif /*!HAVE_W32_SYSTEM*/


/* Check that forking won't return the same random. */
static void
check_forking (void)
{
#ifdef HAVE_W32_SYSTEM
  if (verbose)
    info ("check_forking skipped: not applicable on Windows\n");
#else /*!HAVE_W32_SYSTEM*/
  pid_t pid;
  int rp[2];
  int i, status;
  size_t nread;
  char tmp1[16], tmp1c[16], tmp1p[16];

  if (verbose)
    info ("checking that a fork won't cause the same random output\n");

  /* We better make sure that the RNG has been initialzied. */
  gcry_randomize (tmp1, sizeof tmp1, GCRY_STRONG_RANDOM);
  if (verbose)
    print_hex ("initial random: ", tmp1, sizeof tmp1);

  if (pipe (rp) == -1)
    die ("pipe failed: %s\n", strerror (errno));

  pid = fork ();
  if (pid == (pid_t)(-1))
    die ("fork failed: %s\n", strerror (errno));
  if (!pid)
    {
      gcry_randomize (tmp1c, sizeof tmp1c, GCRY_STRONG_RANDOM);
      if (writen (rp[1], tmp1c, sizeof tmp1c))
        die ("write failed: %s\n", strerror (errno));
      if (verbose)
        {
          print_hex ("  child random: ", tmp1c, sizeof tmp1c);
          fflush (stdout);
        }
      _exit (0);
    }
  gcry_randomize (tmp1p, sizeof tmp1p, GCRY_STRONG_RANDOM);
  if (verbose)
    print_hex (" parent random: ", tmp1p, sizeof tmp1p);

  close (rp[1]);
  if (readn (rp[0], tmp1c, sizeof tmp1c, &nread))
    die ("read failed: %s\n", strerror (errno));
  if (nread != sizeof tmp1c)
    die ("read too short\n");

  while ( (i=waitpid (pid, &status, 0)) == -1 && errno == EINTR)
    ;
  if (i != (pid_t)(-1)
      && WIFEXITED (status) && !WEXITSTATUS (status))
    ;
  else
    die ("child failed\n");

  if (!memcmp (tmp1p, tmp1c, sizeof tmp1c))
    die ("parent and child got the same random number\n");
#endif  /*!HAVE_W32_SYSTEM*/
}



/* Check that forking won't return the same nonce. */
static void
check_nonce_forking (void)
{
#ifdef HAVE_W32_SYSTEM
  if (verbose)
    info ("check_nonce_forking skipped: not applicable on Windows\n");
#else /*!HAVE_W32_SYSTEM*/
  pid_t pid;
  int rp[2];
  int i, status;
  size_t nread;
  char nonce1[10], nonce1c[10], nonce1p[10];

  if (verbose)
    info ("checking that a fork won't cause the same nonce output\n");

  /* We won't get the same nonce back if we never initialized the
     nonce subsystem, thus we get one nonce here and forget about
     it. */
  gcry_create_nonce (nonce1, sizeof nonce1);
  if (verbose)
    print_hex ("initial nonce: ", nonce1, sizeof nonce1);

  if (pipe (rp) == -1)
    die ("pipe failed: %s\n", strerror (errno));

  pid = fork ();
  if (pid == (pid_t)(-1))
    die ("fork failed: %s\n", strerror (errno));
  if (!pid)
    {
      gcry_create_nonce (nonce1c, sizeof nonce1c);
      if (writen (rp[1], nonce1c, sizeof nonce1c))
        die ("write failed: %s\n", strerror (errno));
      if (verbose)
        {
          print_hex ("  child nonce: ", nonce1c, sizeof nonce1c);
          fflush (stdout);
        }
      _exit (0);
    }
  gcry_create_nonce (nonce1p, sizeof nonce1p);
  if (verbose)
    print_hex (" parent nonce: ", nonce1p, sizeof nonce1p);

  close (rp[1]);
  if (readn (rp[0], nonce1c, sizeof nonce1c, &nread))
    die ("read failed: %s\n", strerror (errno));
  if (nread != sizeof nonce1c)
    die ("read too short\n");

  while ( (i=waitpid (pid, &status, 0)) == -1 && errno == EINTR)
    ;
  if (i != (pid_t)(-1)
      && WIFEXITED (status) && !WEXITSTATUS (status))
    ;
  else
    die ("child failed\n");

  if (!memcmp (nonce1p, nonce1c, sizeof nonce1c))
    die ("parent and child got the same nonce\n");
#endif  /*!HAVE_W32_SYSTEM*/
}


/* Check that a closed random device os re-opened if needed. */
static void
check_close_random_device (void)
{
#ifdef HAVE_W32_SYSTEM
  if (verbose)
    info ("check_close_random_device skipped: not applicable on Windows\n");
#else /*!HAVE_W32_SYSTEM*/
  pid_t pid;
  int i, status;
  char buf[4];

  if (verbose)
    info ("checking that close_random_device works\n");

  gcry_randomize (buf, sizeof buf, GCRY_STRONG_RANDOM);
  if (verbose)
    print_hex ("parent random: ", buf, sizeof buf);

  pid = fork ();
  if (pid == (pid_t)(-1))
    die ("fork failed: %s\n", strerror (errno));
  if (!pid)
    {
      xgcry_control ((GCRYCTL_CLOSE_RANDOM_DEVICE, 0));

      /* The next call will re-open the device.  */
      gcry_randomize (buf, sizeof buf, GCRY_STRONG_RANDOM);
      if (verbose)
        {
          print_hex ("child random : ", buf, sizeof buf);
          fflush (stdout);
        }
      _exit (0);
    }

  while ( (i=waitpid (pid, &status, 0)) == -1 && errno == EINTR)
    ;
  if (i != (pid_t)(-1)
      && WIFEXITED (status) && !WEXITSTATUS (status))
    ;
  else
    die ("child failed\n");

#endif  /*!HAVE_W32_SYSTEM*/
}


static int
rng_type (void)
{
  int rngtype;
  if (gcry_control (GCRYCTL_GET_CURRENT_RNG_TYPE, &rngtype))
    die ("retrieving RNG type failed\n");
  return rngtype;
}


static void
check_rng_type_switching (void)
{
  int rngtype, initial;
  char tmp1[4];

  if (verbose)
    info ("checking whether RNG type switching works\n");

  rngtype = rng_type ();
  if (debug)
    info ("rng type: %d\n", rngtype);
  initial = rngtype;
  gcry_randomize (tmp1, sizeof tmp1, GCRY_STRONG_RANDOM);
  if (debug)
    print_hex ("  sample: ", tmp1, sizeof tmp1);
  if (rngtype != rng_type ())
    die ("RNG type unexpectedly changed\n");

  xgcry_control ((GCRYCTL_SET_PREFERRED_RNG_TYPE, GCRY_RNG_TYPE_SYSTEM));

  rngtype = rng_type ();
  if (debug)
    info ("rng type: %d\n", rngtype);
  if (rngtype != initial)
    die ("switching to System RNG unexpectedly succeeded\n");
  gcry_randomize (tmp1, sizeof tmp1, GCRY_STRONG_RANDOM);
  if (debug)
    print_hex ("  sample: ", tmp1, sizeof tmp1);
  if (rngtype != rng_type ())
    die ("RNG type unexpectedly changed\n");

  xgcry_control ((GCRYCTL_SET_PREFERRED_RNG_TYPE, GCRY_RNG_TYPE_FIPS));

  rngtype = rng_type ();
  if (debug)
    info ("rng type: %d\n", rngtype);
  if (rngtype != initial)
    die ("switching to FIPS RNG unexpectedly succeeded\n");
  gcry_randomize (tmp1, sizeof tmp1, GCRY_STRONG_RANDOM);
  if (debug)
    print_hex ("  sample: ", tmp1, sizeof tmp1);
  if (rngtype != rng_type ())
    die ("RNG type unexpectedly changed\n");

  xgcry_control ((GCRYCTL_SET_PREFERRED_RNG_TYPE, GCRY_RNG_TYPE_STANDARD));

  rngtype = rng_type ();
  if (debug)
    info ("rng type: %d\n", rngtype);
  if (rngtype != GCRY_RNG_TYPE_STANDARD)
    die ("switching to standard RNG failed\n");
  gcry_randomize (tmp1, sizeof tmp1, GCRY_STRONG_RANDOM);
  if (debug)
    print_hex ("  sample: ", tmp1, sizeof tmp1);
  if (rngtype != rng_type ())
    die ("RNG type unexpectedly changed\n");
}


static void
check_early_rng_type_switching (void)
{
  int rngtype, initial;

  if (verbose)
    info ("checking whether RNG type switching works in the early stage\n");

  rngtype = rng_type ();
  if (debug)
    info ("rng type: %d\n", rngtype);
  initial = rngtype;

  xgcry_control ((GCRYCTL_SET_PREFERRED_RNG_TYPE, GCRY_RNG_TYPE_SYSTEM));

  rngtype = rng_type ();
  if (debug)
    info ("rng type: %d\n", rngtype);
  if (initial >= GCRY_RNG_TYPE_SYSTEM && rngtype != GCRY_RNG_TYPE_SYSTEM)
    die ("switching to System RNG failed\n");

  xgcry_control ((GCRYCTL_SET_PREFERRED_RNG_TYPE, GCRY_RNG_TYPE_FIPS));

  rngtype = rng_type ();
  if (debug)
    info ("rng type: %d\n", rngtype);
  if (initial >= GCRY_RNG_TYPE_FIPS && rngtype != GCRY_RNG_TYPE_FIPS)
    die ("switching to FIPS RNG failed\n");

  xgcry_control ((GCRYCTL_SET_PREFERRED_RNG_TYPE, GCRY_RNG_TYPE_STANDARD));

  rngtype = rng_type ();
  if (debug)
    info ("rng type: %d\n", rngtype);
  if (rngtype != GCRY_RNG_TYPE_STANDARD)
    die ("switching to standard RNG failed\n");
}


static void
check_drbg_reinit (void)
{
  static struct { const char *flags; } tv[] = {
    { NULL },
    { "" },
    { "sha1" },
    { "sha1 pr" },
    { "sha256" },
    { "sha256 pr" },
    { "sha512" },
    { "sha512 pr" },
    { "hmac sha1" },
    { "hmac sha1 pr" },
    { "hmac sha256" },
    { "hmac sha256 pr" },
    { "hmac sha512" },
    { "hmac sha512 pr" },
    { "aes sym128" },
    { "aes sym128 pr" },
    { "aes sym192" },
    { "aes sym192 pr" },
    { "aes sym256" },
    { "aes sym256 pr" }
  };
  int tidx;
  gpg_error_t err;
  char pers_string[] = "I'm a doctor, not an engineer.";
  gcry_buffer_t pers[1];

  if (verbose)
    info ("checking DRBG_REINIT\n");

  memset (pers, 0, sizeof pers);
  pers[0].data = pers_string;
  pers[0].len = strlen (pers_string);

  err = gcry_control (GCRYCTL_DRBG_REINIT, "", NULL, 0, &err);
  if (gpg_err_code (err) != GPG_ERR_INV_ARG)
    die ("gcry_control(DRBG_REINIT) guard value did not work\n");

  err = gcry_control (GCRYCTL_DRBG_REINIT, "", NULL, -1, NULL);
  if (gpg_err_code (err) != GPG_ERR_INV_ARG)
    die ("gcry_control(DRBG_REINIT) npers negative detection failed\n");

  if (rng_type () != GCRY_RNG_TYPE_FIPS)
    {
      err = gcry_control (GCRYCTL_DRBG_REINIT, "", NULL, 0, NULL);
      if (gpg_err_code (err) != GPG_ERR_NOT_SUPPORTED)
        die ("DRBG_REINIT worked despite that DRBG is not active\n");
      return;
    }

  err = gcry_control (GCRYCTL_DRBG_REINIT, "", NULL, 1, NULL);
  if (gpg_err_code (err) != GPG_ERR_INV_ARG)
    die ("_gcry_rngdrbg_reinit failed to detact: (!pers && npers)\n");
  err = gcry_control (GCRYCTL_DRBG_REINIT, "", pers, 2, NULL);
  if (gpg_err_code (err) != GPG_ERR_INV_ARG)
    die ("_gcry_rngdrbg_reinit failed to detect: (pers && npers != 1)\n");

  err = gcry_control (GCRYCTL_DRBG_REINIT, "aes sym128 bad pr ", pers, 1, NULL);
  if (gpg_err_code (err) != GPG_ERR_INV_FLAG)
    die ("_gcry_rngdrbg_reinit failed to detect a bad flag\n");

  for (tidx=0; tidx < DIM(tv); tidx++)
    {
      err = gcry_control (GCRYCTL_DRBG_REINIT, tv[tidx].flags, NULL, 0, NULL);
      if (err)
        die ("_gcry_rngdrbg_reinit failed for \"%s\" w/o pers: %s\n",

             tv[tidx].flags, gpg_strerror (err));
      err = gcry_control (GCRYCTL_DRBG_REINIT, tv[tidx].flags, pers, 1, NULL);
      if (err)
        die ("_gcry_rngdrbg_reinit failed for \"%s\" with pers: %s\n",
             tv[tidx].flags, gpg_strerror (err));
      /* fixme: We should extract some random after each test.  */
    }
}


#if defined(USE_POSIX_SPAWN_FOR_TESTS) && defined (HAVE_SPAWN_H)
#include <spawn.h>
extern char **environ;

static void
run_all_rng_tests (const char *program)
{
  static const char *options[][2] = {
    { "--early-rng-check",     NULL },
    { "--early-rng-check",     "--prefer-standard-rng" },
    { "--early-rng-check",     "--prefer-fips-rng" },
    { "--early-rng-check",     "--prefer-system-rng" },
    { "--prefer-standard-rng", NULL },
    { "--prefer-fips-rng",     NULL },
    { "--prefer-system-rng",   NULL },
    { NULL, NULL }
  };
  int idx;
  char *argv[8];

  for (idx=0; options[idx][0]; idx++)
    {
      int i;
      pid_t pid;
      int status;

      if (verbose)
        info ("now running with options '%s%s%s'\n",
              options[idx][0],
              options[idx][1] ? " " : "",
              options[idx][1] ? options[idx][1] : "");

      i = 0;
      argv[i++] = xstrdup (program);
      argv[i++] = xstrdup ("--in-recursion");
      argv[i++] = xstrdup ("--verbose");
      argv[i++] = xstrdup ("--debug");
      argv[i++] = xstrdup ("--progress");
      argv[i++] = xstrdup (options[idx][0]);
      if (options[idx][1])
        argv[i++] = xstrdup (options[idx][1]);
      argv[i++] = NULL;

      if (posix_spawn (&pid, program, NULL, NULL, argv, environ))
        die ("spawning '%s' failed\n", program);

      if (waitpid (pid, &status, 0) < 0)
        die ("waitpid for '%s' failed\n", program);

      if (WIFEXITED (status) && WEXITSTATUS (status))
        die ("running '%s' failed with %d\n", program, WEXITSTATUS (status));
      else if (!WIFEXITED (status))
        die ("running '%s' failed\n", program);

      while (i)
        xfree (argv[--i]);
    }
}
#else
/* Because we want to check initialization behaviour, we need to
   fork/exec this program with several command line arguments.  We use
   system, so that these tests work also on Windows.  */
static void
run_all_rng_tests (const char *program)
{
  static const char *options[] = {
    "--early-rng-check",
    "--early-rng-check --prefer-standard-rng",
    "--early-rng-check --prefer-fips-rng",
    "--early-rng-check --prefer-system-rng",
    "--prefer-standard-rng",
    "--prefer-fips-rng",
    "--prefer-system-rng",
    NULL
  };
  int idx;
  size_t len, maxlen;
  char *cmdline;

  maxlen = 0;
  for (idx=0; options[idx]; idx++)
    {
      len = strlen (options[idx]);
      if (len > maxlen)
        maxlen = len;
    }
  maxlen += strlen (program);
  maxlen += strlen (" --in-recursion --verbose --debug --progress");
  maxlen++;
  cmdline = malloc (maxlen + 1);
  if (!cmdline)
    die ("out of core\n");

  for (idx=0; options[idx]; idx++)
    {
      if (verbose)
        info ("now running with options '%s'\n", options[idx]);
      strcpy (cmdline, program);
      strcat (cmdline, " --in-recursion");
      if (verbose)
        strcat (cmdline, " --verbose");
      if (debug)
        strcat (cmdline, " --debug");
      if (with_progress)
        strcat (cmdline, " --progress");
      strcat (cmdline, " ");
      strcat (cmdline, options[idx]);
      if (system (cmdline))
        die ("running '%s' failed\n", cmdline);
    }

  free (cmdline);
}
#endif


static void
run_benchmark (void)
{
  char rndbuf[32];
  int i, j;

  if (verbose)
    info ("benchmarking GCRY_STRONG_RANDOM (/dev/urandom)\n");

  start_timer ();
  gcry_randomize (rndbuf, sizeof rndbuf, GCRY_STRONG_RANDOM);
  stop_timer ();

  info ("getting first 256 bits: %s", elapsed_time (1));

  for (j=0; j < 5; j++)
    {
      start_timer ();
      for (i=0; i < 100; i++)
        gcry_randomize (rndbuf, sizeof rndbuf, GCRY_STRONG_RANDOM);
      stop_timer ();

      info ("100 calls of 256 bits each: %s", elapsed_time (100));
    }

}


int
main (int argc, char **argv)
{
  int last_argc = -1;
  int early_rng = 0;
  int in_recursion = 0;
  int benchmark = 0;
  int with_seed_file = 0;
  const char *program = NULL;

  if (argc)
    {
      program = *argv;
      argc--; argv++;
    }
  else
    die ("argv[0] missing\n");

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
          fputs ("usage: random [options]\n", stdout);
          exit (0);
        }
      else if (!strcmp (*argv, "--verbose"))
        {
          verbose = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--debug"))
        {
          debug = verbose = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--progress"))
        {
          argc--; argv++;
          with_progress = 1;
        }
      else if (!strcmp (*argv, "--in-recursion"))
        {
          in_recursion = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--benchmark"))
        {
          benchmark = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--early-rng-check"))
        {
          early_rng = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--with-seed-file"))
        {
          with_seed_file = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--prefer-standard-rng"))
        {
          /* This is anyway the default, but we may want to use it for
             debugging. */
          xgcry_control ((GCRYCTL_SET_PREFERRED_RNG_TYPE,
                          GCRY_RNG_TYPE_STANDARD));
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--prefer-fips-rng"))
        {
          xgcry_control ((GCRYCTL_SET_PREFERRED_RNG_TYPE, GCRY_RNG_TYPE_FIPS));
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--prefer-system-rng"))
        {
          xgcry_control ((GCRYCTL_SET_PREFERRED_RNG_TYPE, GCRY_RNG_TYPE_SYSTEM));
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--disable-hwf"))
        {
          argc--;
          argv++;
          if (argc)
            {
              if (gcry_control (GCRYCTL_DISABLE_HWF, *argv, NULL))
                die ("unknown hardware feature `%s'\n", *argv);
              argc--;
              argv++;
            }
        }
    }

#ifndef HAVE_W32_SYSTEM
  signal (SIGPIPE, SIG_IGN);
#endif

  if (benchmark && !verbose)
    verbose = 1;

  if (early_rng)
    {
      /* Don't switch RNG in fips mode. */
      if (!gcry_fips_mode_active())
        check_early_rng_type_switching ();
    }

  xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));
  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");

  if (with_progress)
    gcry_set_progress_handler (progress_cb, NULL);

  if (with_seed_file)
    {
      char *fname = prepend_srcdir ("random.seed");

      if (access (fname, F_OK))
        info ("random seed file '%s' not found\n", fname);
      gcry_control (GCRYCTL_SET_RANDOM_SEED_FILE, fname);
      xfree (fname);
    }

  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));
  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u, 0));

  if (benchmark)
    {
      run_benchmark ();
    }
  else if (!in_recursion)
    {
      check_forking ();
      check_nonce_forking ();
      check_close_random_device ();
    }
  /* For now we do not run the drgb_reinit check from "make check" due
     to its high requirement for entropy.  */
  if (!benchmark && !getenv ("GCRYPT_IN_REGRESSION_TEST"))
    check_drbg_reinit ();

  /* Don't switch RNG in fips mode.  */
  if (!benchmark && !gcry_fips_mode_active())
    check_rng_type_switching ();

  if (!in_recursion && !benchmark)
    run_all_rng_tests (program);

  /* Print this info last so that it does not influence the
   * initialization and thus the benchmarking.  */
  if (!in_recursion && verbose)
    {
      char *buf;
      char *fields[5];

      buf = gcry_get_config (0, "rng-type");
      if (buf
          && split_fields_colon (buf, fields, DIM (fields)) >= 5
          && atoi (fields[4]) > 0)
        info ("The JENT RNG was active\n");
      gcry_free (buf);
    }

  if (debug)
    xgcry_control ((GCRYCTL_DUMP_RANDOM_STATS));

  return 0;
}
