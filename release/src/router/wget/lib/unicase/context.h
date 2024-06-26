/* Case-mapping contexts of UTF-8/UTF-16/UTF-32 substring.
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


/* The context of a prefix string combines the information of the "Before C"
   conditions of the Unicode Standard,
   <https://www.unicode.org/versions/Unicode5.0.0/ch03.pdf>, section 3.13,
   table 3-14 "Context Specification for Casing".

   casing_prefix_context_t contains the following fields:

     // Helper for evaluating the FINAL_SIGMA condition:
     //  Last character that was not case-ignorable.
     ucs4_t last_char_except_ignorable;

     // Helper for evaluating the AFTER_SOFT_DOTTED and AFTER_I conditions:
     // Last character that was of combining class 230 ("Above") or 0.
     ucs4_t last_char_normal_or_above;

   Three bits would be sufficient to carry the context information, but
   that would require to invoke uc_is_cased and uc_is_property_soft_dotted
   ahead of time, more often than actually needed.  */


/* The context of a suffix string combines the information of the "After C"
   conditions of the Unicode Standard,
   <https://www.unicode.org/versions/Unicode5.0.0/ch03.pdf>, section 3.13,
   table 3-14 "Context Specification for Casing".

   casing_suffix_context_t contains the following fields:

     // For evaluating the FINAL_SIGMA condition:
     //  First character that was not case-ignorable.
     ucs4_t first_char_except_ignorable;

     // For evaluating the MORE_ABOVE condition:
     // Bit 0 is set if the suffix contains a character of combining class
     // 230 (Above) with no character of combining class 0 or 230 (Above)
     // before it.
     //
     // For evaluating the BEFORE_DOT condition:
     // Bit 1 is set if the suffix contains a COMBINING DOT ABOVE (U+0307)
     // with no character of combining class 0 or 230 (Above) before it.
     //
     uint32_t bits;

   Three bits would be sufficient to carry the context information, but
   that would require to invoke uc_is_cased ahead of time, more often than
   actually needed.  */
#define SCC_MORE_ABOVE_MASK  1
#define SCC_BEFORE_DOT_MASK  2
