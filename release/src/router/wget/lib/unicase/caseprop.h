/* Case related properties of Unicode characters.
   Copyright (C) 2009-2024 Free Software Foundation, Inc.
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

#include "unitypes.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Determine whether a character is "cased" according to the Unicode Standard,
   <https://www.unicode.org/versions/Unicode5.0.0/ch03.pdf>, section 3.13,
   definition D120.  */
extern bool
       uc_is_cased (ucs4_t uc)
       _UC_ATTRIBUTE_CONST;

/* Determine whether a character is "case-ignorable"
   according to the Unicode Standard,
   <https://www.unicode.org/versions/Unicode5.0.0/ch03.pdf>, section 3.13,
   definition D121.  */
extern bool
       uc_is_case_ignorable (ucs4_t uc)
       _UC_ATTRIBUTE_CONST;


#ifdef __cplusplus
}
#endif
