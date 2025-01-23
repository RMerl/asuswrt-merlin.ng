/* t-malloc.c - Check some malloc functions
 * Copyright (C) 2020 g10 Code GmbH
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
#include <errno.h>

#define PGM "t-malloc"
#include "t-common.h"


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


static void
check_reallocarray (void)
{
  struct foo_s { const char *a; int b; } *array;
  size_t n;

  array = gpgrt_calloc (10, sizeof *array);
  if (!array)
    die ("%s: malloc failed\n", __func__);

  for (n=0; n < 10; n++)
    if (array[n].a || array[n].b)
      fail ("%s: array not cleared at index %zu\n", __func__, n);

  for (n=0; n < 10; n++)
    {
      array[n].a = "dummy string";
      array[n].b = 100+n;
    }

  array = gpgrt_reallocarray (array, 10, 20, sizeof *array);
  if (!array)
    die ("%s: realloc failed\n", __func__);

  for (n=0; n < 10; n++)
    {
      if (!array[n].a || strcmp (array[n].a, "dummy string"))
        fail ("%s: string in realloced array changed at index %zu\n",
              __func__, n);

      if (array[n].b != 100 + n)
        fail ("%s: number in realloced array changed at index %zu\n",
              __func__, n);
    }
  for (n=10; n < 20; n++)
    if (array[n].a || array[n].b)
      fail ("%s: realloced array not cleared at index %zu\n", __func__, n);

  /* We can't easily check whether the reallocated array does not
   * iniitialze in the case OLDN is equal or larger to N, so we skip
   * this.  Let's do a simple shrink test instead.  */

  array = gpgrt_reallocarray (array, 20, 7, sizeof *array);
  if (!array)
    die ("%s: realloc (shrinking) failed\n", __func__);

  for (n=0; n < 7; n++)
    {
      if (!array[n].a || strcmp (array[n].a, "dummy string"))
        fail ("%s: string in shrunk array changed at index %zu\n",
              __func__, n);

      if (array[n].b != 100 + n)
        fail ("%s: number in shrunk array changed at index %zu\n",
              __func__, n);
    }

  xfree (array);
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

  show ("testing malloc functions\n");

  check_reallocarray ();

  return !!errorcount;
}
