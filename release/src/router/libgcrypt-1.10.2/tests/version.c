/* version.c  -  This version test should be run first.
   Copyright (C) 2007 Free Software Foundation, Inc.

   This file is part of Libgcrypt.

   Libgcrypt is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   Libgcrypt is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.  */

/* This test should be run first because due to a failing config.links
   script or bad configure parameters the just build libgcrypt may
   crash in case MPI function for specific CPU revisions have been
   enabled.  Running this test first will print out information so to
   make it easier to figure out the problem. */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include "../src/gcrypt-int.h"

#define PGM "version"
#include "t-common.h"

static void
test_get_config (void)
{
  char *string;
  const char *s;
  int i;

  string = gcry_get_config (0, NULL);
  if (!string)
    fail ("gcry_get_config does not return anything: %s\n",
          gpg_strerror (gpg_error_from_syserror ()));
  else if ( !strchr (string, '\n') )
    fail ("gcry_get_config(0, NULL) did not return multiple lines\n");

  xfree (string);
  string = gcry_get_config (0, "version");
  if (!string)
    fail ("gcry_get_config(\"version\") returned NULL: %s\n",
          gpg_strerror (gpg_error_from_syserror ()));
  else if ( strchr (string, '\n') )
    fail ("gcry_get_config(\"version\") returned more than one line\n");
  else if ( strncmp (string, "version:", 8) )
    fail ("gcry_get_config(\"version\") returned wrong line\n");

  /* Test an item which is not the first.  */
  xfree (string);
  string = gcry_get_config (0, "cpu-arch");
  if (!string)
    fail ("gcry_get_config(\"cpu-arch\") returned NULL: %s\n",
          gpg_strerror (gpg_error_from_syserror ()));
  else if ( strchr (string, '\n') )
    fail ("gcry_get_config(\"cpu-arch\") returned more than one line\n");
  else if ( strncmp (string, "cpu-arch:", 9) )
    fail ("gcry_get_config(\"cpu-arch\") returned wrong line\n");

  /* Test that an unknown item return sthe correct error.  */
  xfree (string);
  string = gcry_get_config (0, "no-such-item");
  if (string)
    fail ("gcry_get_config(\"no-such-item\") returned something\n");
  else if (errno)
    fail ("gcry_get_config(\"no-such-item\") returned wrong error: %s\n",
          gpg_strerror (gpg_error_from_syserror ()));

  /* Check the rng-type.  */
  xfree (string);
  string = gcry_get_config (0, "rng-type");
  if (!string)
    fail ("gcry_get_config(\"rng-type\") not returned\n");
  else
    {
      for (i=0, s = string; *s; s++)
        if (*s == ':')
          i++;
      if (i < 5)
        fail ("gcry_get_config(\"rng-type\") has not enough fields\n");
    }


  xfree (string);
}


int
main (int argc, char **argv)
{
  int last_argc = -1;

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
      else if (!strcmp (*argv, "--verbose"))
        {
          verbose++;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--debug"))
        {
          /* Dummy option */
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--disable-hwf"))
        {
          argc--;
          argv++;
          if (argc)
            {
              if (gcry_control (GCRYCTL_DISABLE_HWF, *argv, NULL))
                fprintf (stderr,
                        PGM
                        ": unknown hardware feature '%s' - option ignored\n",
                        *argv);
              argc--;
              argv++;
            }
        }
    }

  xgcry_control ((GCRYCTL_SET_VERBOSITY, (int)verbose));

  xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));
  if (strcmp (GCRYPT_VERSION, gcry_check_version (NULL)))
    {
      int oops = !gcry_check_version (GCRYPT_VERSION);
      fprintf (stderr, PGM ": %sversion mismatch; pgm=%s, library=%s\n",
               oops? "":"warning: ", GCRYPT_VERSION, gcry_check_version (NULL));
      if (oops)
        exit (1);
    }

  xgcry_control ((GCRYCTL_PRINT_CONFIG, stdout));

  test_get_config ();

  return 0;
}
