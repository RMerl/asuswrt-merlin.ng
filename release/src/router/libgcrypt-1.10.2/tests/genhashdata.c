/* genhashdata.c - Create data for hash tests
 * Copyright (C) 2013 g10 Code GmbH
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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/* Results:

$  for i in -64 -1 0 1 64; do ./genhashdata --gigs 256 --bytes $i|sha1sum;done
92fc51850c7b750e6e774b75f294f6979d4059f0  -
4bddeeb4c08683f02d4944d93dbcb02ebab50134  -
71b923afde1c8c040884c723a2e3335b333e64c6  -
2d99f9b5b86e9c9c937104f4242bd6b8bc0927ef  -
a60dabe8d749f798b7ec3a684cc3eab487451482  -

$ for i in -64 -1 0 1 64; do ./genhashdata --gigs 256 --bytes $i|sha224sum;done
b5672b54d2480a5688a2dc727a1ad4db7a81ef31ce8999e0bbaeffdc  -
814ea7159473e6ffc1c64b90026a542e13ac6980f7f3ca3c4582a9b8  -
9ec0e1829455db8650ec7a8b06912196f97a7358bc3a73c79911cd4e  -
e578d5d523320876565bbbc892511a485427caee6dd754d57e3e58c2  -
ff0464df248cd298b63765bc4f87f21e25c93c657fdf3656d3c878e5  -

$ for i in -64 -1 0 1 64; do ./genhashdata --gigs 256 --bytes $i|sha256sum;done
87a9828d3de78d55d252341db2a622908c4e0ceaee9961ecf9768700fc799ec8  -
823bf95f64ef04a4a77579c38760b1d401b56bf3a8e664bdf56ca15afb468a03  -
2d0723878cb2c3d5c59dfad910cdb857f4430a6ba2a7d687938d7a20e63dde47  -
5a2e21b1e79cd866acf53a2a18ca76bd4e02c4b01bf4627354171824c812d95f  -
34444808af8e9d995e67f9e155ed94bf55f195a51dc1d8a989e6bcf95511c8a2  -


$ for i in -64 -1 0 1 64; do ./genhashdata --gigs 256 --bytes $i|sha512sum;done
e01bf8140874bf240e8426cb2bcbc377cbed2e6037334116637149e1cd8cd462 \
96828b71f32b9f002771d4cb51172ce578b73b7939221e4df655ecd08601e655  -
4917ff94514b1757705c289fdc3e7d6ffcce5771b20ae237ebc03d2ec9eb435f \
b7ce9f0e27272be8cced77a5edae1a01a0ad62b0a44169d88bbee45474a17734  -
1e28e8b3c79f2f47da11f3c0b7da4e7981e7d932db6d17d528a31e191922edda \
8fc4bb2df10ea876232db5a1c606bc41886e8b2c570a3e721221f60c8c7dc4ab  -
027d3324dd1cf127770ceb53681f4c70937c9bca4e3acd5fd76cb266c7d4527d \
58140290a1822e8d60c4d3ae9725fb923183230d6dfd2d7d73c0d74a4757f34a  -
49920704ea9d6ee19f0742d6c868110fa3eda8ac09f026e9ef22cc731af53020 \
de40eedef66cb1afd94c61e285fa9327e01336e804903740a9145ab1f065c2d5  -

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define PGM "genhashdata"
#include "t-common.h"

int
main (int argc, char **argv)
{
  int last_argc = -1;
  int gigs = 0;
  int bytes = 0;
  char pattern[1024];
  int i, g;

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
          fputs ("usage: " PGM " [options]\n"
                 "Options:\n"
                 "  --gigs  N     Emit N GiB of test bytes\n"
                 "  --bytes DIFF  Stop DIFF bytes earlier or later\n",
                 stdout);
          exit (0);
        }
      else if (!strcmp (*argv, "--gigs"))
        {
          argc--; argv++;
          if (argc)
            {
              gigs = atoi (*argv);
              argc--; argv++;
            }
        }
      else if (!strcmp (*argv, "--bytes"))
        {
          argc--; argv++;
          if (argc)
            {
              bytes = atoi (*argv);
              argc--; argv++;
            }
        }
      else if (!strncmp (*argv, "--", 2))
        die ("unknown option '%s'", *argv);
    }

  if (gigs < 0 || gigs > 1024*1024)
    die ("value for --gigs must be in the range 0 to %d", 1024*1024);
  if (bytes < -1024 || bytes > 1024)
      die ("value for --bytes must be in the range -1024 to 1024");
  if (sizeof pattern != 1024)
    die ("internal error");

  if (argc > 1)
    die ("arguments are not expected");

  memset (pattern, 'a', sizeof pattern);

  for (g=0; g < gigs; g++)
    {
      if (g + 1 == gigs && bytes < 0)
        {
          for (i = 0; i < 1024*1023; i++)
            if (fwrite (pattern, sizeof pattern, 1, stdout) != 1)
              die ("writing to stdout failed: %s", strerror (errno));
          for (i = 0; i < 1023; i++)
            if (fwrite (pattern, sizeof pattern, 1, stdout) != 1)
              die ("writing to stdout failed: %s", strerror (errno));
          if (fwrite (pattern, sizeof pattern + bytes, 1, stdout) != 1)
            die ("writing to stdout failed: %s", strerror (errno));
        }
      else
        {
          for (i = 0; i < 1024*1024; i++)
            if (fwrite (pattern, sizeof pattern, 1, stdout) != 1)
              die ("writing to stdout failed: %s", strerror (errno));
        }
    }
  if (bytes > 0)
    if (fwrite (pattern, bytes, 1, stdout) != 1)
      die ("writing to stdout failed: %s", strerror (errno));
  if (fflush (stdout))
    die ("writing to stdout failed: %s", strerror (errno));

  return 0;
}
