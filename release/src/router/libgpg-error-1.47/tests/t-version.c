/* t-version.c - Check the version info function
 * Copyright (C) 2013 g10 Code GmbH
 *
 * This file is part of libgpg-error.
 *
 * libgpg-error is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * libgpg-error is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define PGM "t-version"
#include "t-common.h"

static const char *logpfx = PGM;


static void
t_gpgrt_cmp_version (void)
{
  struct { int result; int level; const char *a; const char *b; } t[] = {
    {  0, 1, "0", "0" },
    { -1, 1, "0", "1" },
    {  1, 1, "1", "0" },
    { -1, 1, "0.0", "0.1" },
    { -1, 1, "0.1", "1.2" },
    {  1, 1, "1.0", "0.9" },
    { -1, 1, "-1.0", "0.9" }, /* A is invalid */
    {  0, 1, "0rc0", "0rc0" },
    {  1, 1, "0rc1", "0rc0" },
    { -1, 1, "0rc1", "0rc2" },
    {  0, 1, "0.rc0", "0.rc0" },
    {  1, 1, "0.rc1", "0.rc0" },
    { -1, 1, "0.rc1", "0.rc2" },
    {  0, 1, "0.rc1", "0.rc1" },
    { -1, 1, "0qc1",  "0rc0" },
    { -1, 1, "0.qc1", "0.rc0" },
    {  0, 2, "0.0", "0.0" },
    { -1, 2, "0.1", "0.2" },
    { -1, 2, "3.1", "3.2" },
    { -1, 2, "3.1", "4.0" },
    {  0, 2, "1.1rc0", "1.1rc0" },
    {  1, 2, "1.1rc1", "1.1rc0" },
    { -1, 2, "1.1rc0", "1.1rc1" },
    {  0, 3, "7.0.0", "7.0.0" },
    { -1, 3, "7.0.1", "7.0.2" },
    { -1, 3, "7.3.1", "7.3.2" },
    { -1, 3, "7.3.1", "7.4.0" },
    {  0, 3, "7.1.1rc0", "7.1.1rc0" },
    {  1, 3, "7.1.1rc1", "7.1.1rc0" },
    { -1, 3, "7.1.1rc0", "7.1.1rc1" },
    {  1, 3, "6.0.0", "5.0.0" },
    {  0, 3, "6.0.0", "6.0.0" },
    {  1, 3, "6.0.1", "6.0.0" },
    {  1, 3, "6.1.0", "6.0.0" },
    {  1, 3, "6.2.1", "6.2.0" },
    { -1, 3, "6.2.1", "6.2.2" },
    { -1, 3, "6.0.0", "6.0.2" },
    { -1, 3, "6.0.0", "6.1.0" },
    { -1, 3, "6.2.0", "6.2.1" },
    {  1, 3, "6.0.0-beta1", "6.0.0-beta0" },
    {  0, 3, "6.0.0-beta2", "6.0.0-beta2" },
    {  1, 3, "6.0.0-beta20", "6.0.0-beta19" },
    { -1, 3, "6.0.0-beta1", "6.0.0-beta2" },
    {  1, 3, "6.0.0-beta2", "6.0.0-beta1" },
    { -1, 3, "6.0.0-beta20", "6.0.0-beta21" },
    {  0,13, "6.0.0-beta1", "6.0.0-beta0" },
    {  0,13, "6.0.0-beta2", "6.0.0-beta2" },
    {  0,13, "6.0.0-beta20", "6.0.0-beta19" },
    {  0,13, "6.0.0-beta1", "6.0.0-beta2" },
    {  0,13, "6.0.0-beta2", "6.0.0-beta1" },
    {  0,13, "6.0.0-beta20", "6.0.0-beta21" }
  };
  int i;
  int result, expected;

  for (i=0; i < DIM (t); i++)
    {
      expected = t[i].result;
      result = gpgrt_cmp_version (t[i].a, t[i].b, t[i].level);
      if (result != expected)
        fail ("test %d failed: cmp('%s','%s',%d) = %d expected %d",
              i, t[i].a, t[i].b, t[i].level, result, expected);
    }
  for (i=0; i < DIM (t); i++)
    {
      expected = 0 - t[i].result;
      result = gpgrt_cmp_version (t[i].a, t[i].b, -t[i].level);
      if (result != expected)
        fail ("test %d-rev failed: cmp('%s','%s',%d) = %d expected %d",
              i, t[i].a, t[i].b, -t[i].level, result, expected);
    }
}



int
main (int argc, char **argv)
{
  int last_argc = -1;

  if (argc)
    {
      argc--; argv++;
    }
  while (argc && last_argc != argc )
    {
      last_argc = argc;
      if (!strcmp (*argv, "--help"))
        {
          puts (
"usage: ./version [options]\n"
"\n"
"Options:\n"
"  --verbose      Show what is going on\n"
);
          exit (0);
        }
      if (!strcmp (*argv, "--verbose"))
        {
          verbose = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--debug"))
        {
          verbose = debug = 1;
          argc--; argv++;
        }
    }

  t_gpgrt_cmp_version ();

  if (!gpg_error_check_version (GPG_ERROR_VERSION))
    {
      fprintf (stderr, "%s: gpg_error_check_version returned an error\n",
               logpfx);
      errorcount++;
    }
  if (!gpg_error_check_version ("1.10"))
    {
      fprintf (stderr, "%s: gpg_error_check_version returned an "
               "error for an old version\n", logpfx);
      errorcount++;
    }
  if (gpg_error_check_version ("15.0"))
    {
      fprintf (stderr, "%s: gpg_error_check_version did not return an error"
               " for a newer version\n", logpfx);
      errorcount++;
      show ("\n"); /* Reference this function to silence gcc. */
    }
  if (verbose || errorcount)
    {
      printf ("Version from header: %s (0x%06x)\n",
               GPG_ERROR_VERSION, GPG_ERROR_VERSION_NUMBER);
      printf ("Version from binary: %s\n", gpg_error_check_version (NULL));
      printf ("Copyright blurb ...:%s\n", gpg_error_check_version ("\x01\x01"));
    }

  return errorcount ? 1 : 0;
}
