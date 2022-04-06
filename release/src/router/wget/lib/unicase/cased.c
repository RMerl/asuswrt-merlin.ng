/* Test whether a Unicode character is cased.
   Copyright (C) 2002, 2006-2007, 2009-2022 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2009.

   This file is free software.
   It is dual-licensed under "the GNU LGPLv3+ or the GNU GPLv2+".
   You can redistribute it and/or modify it under either
     - the terms of the GNU Lesser General Public License as published
       by the Free Software Foundation, either version 3, or (at your
       option) any later version, or
     - the terms of the GNU General Public License as published by the
       Free Software Foundation; either version 2, or (at your option)
       any later version, or
     - the same dual license "the GNU LGPLv3+ or the GNU GPLv2+".

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License and the GNU General Public License
   for more details.

   You should have received a copy of the GNU Lesser General Public
   License and of the GNU General Public License along with this
   program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "caseprop.h"

/* Quoting the Unicode standard:
     Definition: A character is defined to be "cased" if it has the Lowercase
     or Uppercase property or has a General_Category value of
     Titlecase_Letter.  */

#if 0

#include "unictype.h"

bool
uc_is_cased (ucs4_t uc)
{
  return (uc_is_property_lowercase (uc)
          || uc_is_property_uppercase (uc)
          || uc_is_general_category (uc, UC_TITLECASE_LETTER));
}

#else

#include "unictype/bitmap.h"

/* Define u_casing_property_cased table.  */
#include "cased.h"

bool
uc_is_cased (ucs4_t uc)
{
  return bitmap_lookup (&u_casing_property_cased, uc);
}

#endif
