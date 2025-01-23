/* gen-w32-lock-obj.c - Build tool to get the size of the lock object.
   Copyright (C) 2014 g10 Code GmbH

   This file is part of libgpg-error.

   libgpg-error is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   libgpg-error is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef HAVE_W32_SYSTEM
# error This module may only be build for Windows.
#endif

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#include "w32-lock-obj.h"


int
main (void)
{
  _gpgrt_lock_t lk;
  unsigned char *p;
  int i;

  printf ("sizeof CRITICAL_SECTION = %u\n", (int)sizeof (CRITICAL_SECTION));
  printf ("sizeof    _gpgrt_lock_t = %u\n", (int)sizeof lk);

  memset (&lk, 0, sizeof lk);
  lk.vers = LOCK_ABI_VERSION;
  lk.started = -1;
  printf ("#define GPGRT_LOCK_INITIALIZER {");
  p = (unsigned char *)&lk;
  for (i=0; i < sizeof lk - 1; i++)
    printf ("%u,", p[i]);
  printf ("%u}\n", p[sizeof(lk)-1]);

  return 0;
}
