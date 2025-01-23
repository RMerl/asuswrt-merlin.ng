/* t-logging.c - Check the logging interface
 * Copyright (C) 2018 g10 Code GmbH
 *
 * This file is part of Libgpg-error.
 *
 * Libgpg-error is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgpg-error is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#define PGM "t-logging"
#include "t-common.h"

/* The memory based estream we use for logging.  */
static estream_t logmemfp;


static const char *
my_strusage (int level)
{
  const char *p;

  switch (level)
    {
    case 9: p = "LGPL-2.1-or-later"; break;
    case 11: p = PGM; break;
    default: p = NULL;
    }
  return p;
}


/* Read all data from the log stream into a new malloced buffer and return
 * that buffer.  The buffer is always 0 terminated.  Either returns a
 * string or dies.  The stream will be truncated to zero. */
static char *
log_to_string (void)
{
#define NCHUNK 1024
  estream_t stream = gpgrt_log_get_stream ();
  char *buffer;
  size_t bufsize, buflen;
  size_t nread;

  gpgrt_log_flush ();
  gpgrt_rewind (stream);

  buffer = NULL;
  buflen = bufsize = 0;
  do
    {
      bufsize += NCHUNK;
      buffer = realloc (buffer, bufsize+1);
      if (!buffer)
        die ("malloc failed at line %d\n", __LINE__);

      nread = gpgrt_fread (buffer + buflen, 1, NCHUNK, stream);
      if (nread < NCHUNK && gpgrt_ferror (stream))
        die ("fread failed at line %d: %s\n", __LINE__,
             gpg_strerror (gpg_err_code_from_syserror ()));
      buflen += nread;
    }
  while (nread == NCHUNK);
  buffer[nread] = 0;

  if (strlen (buffer) != buflen)
    fail ("stream_to_string detected an embedded nul");

  gpgrt_ftruncate (stream, 0);
  return buffer;
#undef NCHUNK
}


static void
check_log_info (void)
{
  char *logbuf;

  log_info ("first log\n");
  logbuf = log_to_string ();
  if (strcmp (logbuf, "t-logging: first log\n"))
    fail ("log_info test failed at line %d\n", __LINE__);
  free (logbuf);

  /* The second line should not have a LF.  */
  log_info ("second log line");
  log_info ("third log line");
  logbuf = log_to_string ();
  if (strcmp (logbuf, ("t-logging: second log line\n"
                       "t-logging: third log line")))
    fail ("log_info test failed at line %d\n", __LINE__);
  free (logbuf);

  /* Now a multi line log.  */
  log_info ("This is log line 1\nand 2\nand 3\n");
  logbuf = log_to_string ();
  if (strcmp (logbuf, ("t-logging: This is log line 1\n"
                       "and 2\n"
                       "and 3\n")))
    fail ("log_info test failed at line %d\n", __LINE__);
  free (logbuf);

  /* With arguments.  */
  log_info ("file '%s' line %d: %s\n", "/foo/bar.txt", 20, "not found");
  logbuf = log_to_string ();
  if (strcmp (logbuf, "t-logging: file '/foo/bar.txt' line 20: not found\n"))
    fail ("log_info test failed at line %d\n", __LINE__);
  free (logbuf);

  /* With arguments and a control char in the string arg.  */
  log_info ("file '%s' line %d: %s\n", "/foo/bar.txt\b", 20, "not found");
  logbuf = log_to_string ();
  if (strcmp (logbuf,
              "t-logging: file '/foo/bar.txt\\b' line 20: not found\n"))
    fail ("log_info test failed at line %d\n", __LINE__);
  free (logbuf);

  /* With arguments and the prefix in a string arg.  */
  log_info ("file '%s': %s\n", "/foo/bar.txt\nt-logging", "not \x01 found");
  logbuf = log_to_string ();
  if (strcmp (logbuf,
              "t-logging: file '/foo/bar.txt\\nt-logging': not \\x01 found\n"))
    fail ("log_info test failed at line %d\n", __LINE__);
  free (logbuf);

  /* With arguments and byte with bit 7 set in a string arg.  */
  log_info ("file '%s': %s\n", "/foo/bar.txt\n", "not \x81 found");
  logbuf = log_to_string ();
  if (strcmp (logbuf,
              "t-logging: file '/foo/bar.txt\\n': not \x81 found\n"))
    fail ("log_info test failed at line %d\n", __LINE__);
  /* show ("===>%s<===\n", logbuf); */

  free (logbuf);
}


static void
check_with_pid (void)
{
  char testbuf[100];
  char *logbuf;

  snprintf (testbuf, sizeof testbuf, "t-logging[%u]: ",
            (unsigned int)getpid ());

  log_info ("first log\n");
  logbuf = log_to_string ();
  if (strncmp (logbuf, testbuf, strlen (testbuf))
      || strcmp (logbuf+strlen (testbuf), "first log\n"))
    fail ("log_with_pid test failed at line %d\n", __LINE__);
  free (logbuf);

  log_info ("This is log line 1\nand 2\nand 3\n");
  logbuf = log_to_string ();
  if (strncmp (logbuf, testbuf, strlen (testbuf))
      || strcmp (logbuf+strlen (testbuf), ("This is log line 1\n"
                                           "and 2\n"
                                           "and 3\n")))
    fail ("log_with_pid test failed at line %d\n", __LINE__);
  free (logbuf);
}


static void
check_log_error (void)
{
  char *logbuf;

  if (log_get_errorcount (0))
    fail ("log_get_errorcount() != 0 at line %d\n", __LINE__);

  log_error ("Hola, something went wrong\n");
  if (log_get_errorcount (0) != 1)
    fail ("log_get_errorcount() != 1 at line %d\n", __LINE__);
  logbuf = log_to_string ();
  if (strcmp (logbuf, "t-logging: Hola, something went wrong\n"))
    fail ("log_info test failed at line %d\n", __LINE__);
  free (logbuf);
  if (log_get_errorcount (0) != 1)
    fail ("log_get_errorcount() != 1 at line %d\n", __LINE__);
  if (log_get_errorcount (1) != 1)  /* note: clear returns old value.  */
    fail ("log_get_errorcount() != 1 at line %d\n", __LINE__);
  if (log_get_errorcount (0))
    fail ("log_get_errorcount() != 0 after clear at line %d\n", __LINE__);
}


int
main (int argc, char **argv)
{
  gpgrt_opt_t opts[] = {
    ARGPARSE_x  ('v', "verbose", NONE, 0, "Print more diagnostics"),
    ARGPARSE_s_n('d', "debug", "Flyswatter"),
    ARGPARSE_end()
  };
  gpgrt_argparse_t pargs = { &argc, &argv, 0 };

  gpgrt_set_strusage (my_strusage);
  gpgrt_log_set_prefix (gpgrt_strusage (11), GPGRT_LOG_WITH_PREFIX);

  while (gpgrt_argparse  (NULL, &pargs, opts))
    {
      switch (pargs.r_opt)
        {
        case 'v': verbose++; break;
        case 'd': debug++; break;
        default : pargs.err = ARGPARSE_PRINT_ERROR; break;
	}
    }
  gpgrt_argparse (NULL, &pargs, NULL);

  show ("testing logging using a memory log stream\n");
  logmemfp = gpgrt_fopenmem (0, "w+b");
  if (!logmemfp)
    die ("fopenmem failed at line %d\n", __LINE__);
  gpgrt_log_set_sink (NULL, logmemfp, -1);

  check_log_info ();
  gpgrt_log_set_prefix (NULL, GPGRT_LOG_WITH_PREFIX|GPGRT_LOG_WITH_PID);
  check_with_pid ();
  gpgrt_log_set_prefix (NULL, GPGRT_LOG_WITH_PREFIX);
  check_log_error ();

  /* FIXME: Add more tests.  */

  show ("testing logging finished\n");
  return !!errorcount;
}
