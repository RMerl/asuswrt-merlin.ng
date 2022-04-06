/* Safe automatic memory allocation.
   Copyright (C) 2003, 2006-2007, 2009-2022 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003, 2018.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#define _GL_USE_STDLIB_ALLOC 1
#include <config.h>

/* Specification.  */
#include "malloca.h"

#include "idx.h"
#include "intprops.h"
#include "verify.h"

/* The speed critical point in this file is freea() applied to an alloca()
   result: it must be fast, to match the speed of alloca().  The speed of
   mmalloca() and freea() in the other case are not critical, because they
   are only invoked for big memory sizes.
   Here we use a bit in the address as an indicator, an idea by Ondřej Bílka.
   malloca() can return three types of pointers:
     - Pointers ≡ 0 mod 2*sa_alignment_max come from stack allocation.
     - Pointers ≡ sa_alignment_max mod 2*sa_alignment_max come from heap
       allocation.
     - NULL comes from a failed heap allocation.  */

/* Type for holding very small pointer differences.  */
typedef unsigned char small_t;
/* Verify that it is wide enough.  */
verify (2 * sa_alignment_max - 1 <= (small_t) -1);

void *
mmalloca (size_t n)
{
#if HAVE_ALLOCA
  /* Allocate one more word, used to determine the address to pass to freea(),
     and room for the alignment ≡ sa_alignment_max mod 2*sa_alignment_max.  */
  uintptr_t alignment2_mask = 2 * sa_alignment_max - 1;
  int plus = sizeof (small_t) + alignment2_mask;
  idx_t nplus;
  if (!INT_ADD_WRAPV (n, plus, &nplus) && !xalloc_oversized (nplus, 1))
    {
      char *mem = (char *) malloc (nplus);

      if (mem != NULL)
        {
          uintptr_t umem = (uintptr_t)mem, umemplus;
          /* The INT_ADD_WRAPV avoids signed integer overflow on
             theoretical platforms where UINTPTR_MAX <= INT_MAX.  */
          INT_ADD_WRAPV (umem, sizeof (small_t) + sa_alignment_max - 1,
                         &umemplus);
          idx_t offset = ((umemplus & ~alignment2_mask)
                          + sa_alignment_max - umem);
          void *vp = mem + offset;
          small_t *p = vp;
          /* Here p >= mem + sizeof (small_t),
             and p <= mem + sizeof (small_t) + 2 * sa_alignment_max - 1
             hence p + n <= mem + nplus.
             So, the memory range [p, p+n) lies in the allocated memory range
             [mem, mem + nplus).  */
          p[-1] = offset;
          /* p ≡ sa_alignment_max mod 2*sa_alignment_max.  */
          return p;
        }
    }
  /* Out of memory.  */
  return NULL;
#else
# if !MALLOC_0_IS_NONNULL
  if (n == 0)
    n = 1;
# endif
  return malloc (n);
#endif
}

#if HAVE_ALLOCA
void
freea (void *p)
{
  /* Check argument.  */
  if ((uintptr_t) p & (sa_alignment_max - 1))
    {
      /* p was not the result of a malloca() call.  Invalid argument.  */
      abort ();
    }
  /* Determine whether p was a non-NULL pointer returned by mmalloca().  */
  if ((uintptr_t) p & sa_alignment_max)
    {
      void *mem = (char *) p - ((small_t *) p)[-1];
      free (mem);
    }
}
#endif

/*
 * Hey Emacs!
 * Local Variables:
 * coding: utf-8
 * End:
 */
