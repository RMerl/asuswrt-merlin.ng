/* t-argparse.c - Check the argparse API
 * Copyright (C) 2018, 2020 g10 Code GmbH
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

#define PGM "t-argparse"
#include "t-common.h"


static struct {
    int verbose;
    int debug;
    char *outfile;
    char *crf;
    int myopt;
    int echo;
    int a_long_one;
    char *street;
} opt;



static const char *
my_strusage (int level)
{
  const char *p;

  switch (level)
    {
    case 9: p = "LGPL-2.1-or-later"; break;

    case 11: p = "t-argparse"; break;
    case 13: p = "6.42.17-beta99"; break;

    default: p = NULL;
    }
  return p;
}



int
main (int argc, char **argv)
{
  gpgrt_opt_t opts[] = {
    ARGPARSE_verbatim("Now for the options:\n"),
    ARGPARSE_x  ('v', "verbose", NONE, 0, "Laut sein"),
    ARGPARSE_s_n('e', "echo"   , ("Zeile ausgeben, damit wir sehen, "
                                  "was wir eingegeben haben")),
    ARGPARSE_s_n('d', "debug", "Debug\nfalls mal etwas\nschief geht"),
    ARGPARSE_s_s('o', "output", 0 ),
    ARGPARSE_o_s('c', "cross-ref", "cross-reference erzeugen\n" ),
    /* Note that on a non-utf8 terminal the ß might garble the output. */
    ARGPARSE_header("extra-options", "List of extra options"),
    ARGPARSE_s_s('s', "street","|Straße|set the name of the street to Straße"),
    ARGPARSE_o_i('m', "my-option", 0),
    ARGPARSE_o_i('M', "not-my-option", 0),
    ARGPARSE_s_n(500, "a-long-option", 0 ),
    ARGPARSE_conffile(501, "options", "|FILE|read options from FILE"),
    ARGPARSE_noconffile(502, "no-options", "Ignore conf files"),
    ARGPARSE_verbatim("This epilog consists\nof only 2 lines\n"),
    ARGPARSE_end()
  };
  gpgrt_argparse_t pargs = { &argc, &argv, (ARGPARSE_FLAG_ALL
                                            | ARGPARSE_FLAG_MIXED
                                            | ARGPARSE_FLAG_ONEDASH
                                            | ARGPARSE_FLAG_SYS
                                            | ARGPARSE_FLAG_USER
                                            /* | ARGPARSE_FLAG_VERBOSE */
                                            /* | ARGPARSE_FLAG_WITHATTR */
                                            ) };
  int i;
  const char *srcdir;
  int any_warn = 0;

  gpgrt_set_strusage (my_strusage);
  srcdir = getenv ("srcdir");
  if (!srcdir)
    srcdir = ".";
  gpgrt_set_confdir (GPGRT_CONFDIR_USER, srcdir);
  {
    char *p = gpgrt_fnameconcat (srcdir, "etc", NULL);
    gpgrt_set_confdir (GPGRT_CONFDIR_SYS, p);
    xfree (p);
  }

  while (gpgrt_argparser  (&pargs, opts, PGM".conf"))
    {
      /* printf ("got option %3d type %0x04x\n", pargs.r_opt, pargs.r_type); */
      /* if (pargs.r_type & (ARGPARSE_ATTR_IGNORE|ARGPARSE_ATTR_FORCE)) */
      /*   printf ("attributes:%s%s\n", */
      /*           (pargs.r_type & ARGPARSE_ATTR_IGNORE)? " ignore":"", */
      /*           (pargs.r_type & ARGPARSE_ATTR_FORCE)? " force":""); */
      /* if (pargs.r_type & ARGPARSE_OPT_IGNORE) */
      /*   { */
      /*     printf ("ignored\n"); */
      /*     continue; */
      /*   } */
      switch (pargs.r_opt)
        {
        case ARGPARSE_CONFFILE:
          printf ("current conffile='%s'\n",
                  pargs.r_type? pargs.r.ret_str: "[cmdline]");
          break;
        case ARGPARSE_IS_ARG :
          printf ("arg='%s'\n", pargs.r.ret_str);
          break;

        case 'v': opt.verbose++; break;
        case 'e': opt.echo++; break;
        case 'd': opt.debug++; debug=1;break;
        case 'o': opt.outfile = pargs.r.ret_str; break;
        case 'c': opt.crf = pargs.r_type? pargs.r.ret_str:"a.crf"; break;
        case 'm': opt.myopt = pargs.r_type? pargs.r.ret_int : 1; break;
        case 'M': opt.myopt = 0; break;
        case 's': opt.street = pargs.r.ret_str; break;
        case 500: opt.a_long_one++;  break;
        default : pargs.err = ARGPARSE_PRINT_WARNING; any_warn = 1; break;
	}
    }
  for (i=0; i < argc; i++ )
    printf ("%3d -> (%s)\n", i, argv[i] );
  if (opt.verbose)
    puts ("Options:");
  if (opt.verbose)
    printf ("  verbose=%d\n", opt.verbose );
  if (opt.debug)
    printf ("  debug=%d\n", opt.debug );
  if (opt.outfile)
    printf ("  outfile='%s'\n", opt.outfile );
  if (opt.crf)
    printf ("  crffile='%s'\n", opt.crf );
  if (opt.street)
    printf ("  street='%s'\n", opt.street );
  if (opt.myopt)
    printf ("  myopt=%d\n", opt.myopt );
  if (opt.a_long_one)
    printf ("  a-long-one=%d\n", opt.a_long_one );
  if (opt.echo)
    printf ("  echo=%d\n", opt.echo );

  gpgrt_argparse (NULL, &pargs, NULL);

  (void)show;
  (void)fail;
  (void)die;

  return !!any_warn;
}
