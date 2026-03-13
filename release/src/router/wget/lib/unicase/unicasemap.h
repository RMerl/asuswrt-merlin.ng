/* Case mapping for UTF-8/UTF-16/UTF-32 strings (locale dependent).
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

#include <stddef.h>

#include "unitypes.h"
#include "unicase.h"
#include "uninorm.h"

#ifdef __cplusplus
extern "C" {
#endif


extern uint8_t *
       u8_casemap (const uint8_t *s, size_t n,
                   casing_prefix_context_t prefix_context,
                   casing_suffix_context_t suffix_context,
                   const char *iso639_language,
                   ucs4_t (*single_character_map) (ucs4_t),
                   size_t offset_in_rule, /* offset in 'struct special_casing_rule' */
                   uninorm_t nf,
                   uint8_t *resultbuf, size_t *lengthp);

extern uint16_t *
       u16_casemap (const uint16_t *s, size_t n,
                    casing_prefix_context_t prefix_context,
                    casing_suffix_context_t suffix_context,
                    const char *iso639_language,
                    ucs4_t (*single_character_map) (ucs4_t),
                    size_t offset_in_rule, /* offset in 'struct special_casing_rule' */
                    uninorm_t nf,
                    uint16_t *resultbuf, size_t *lengthp);

extern uint32_t *
       u32_casemap (const uint32_t *s, size_t n,
                    casing_prefix_context_t prefix_context,
                    casing_suffix_context_t suffix_context,
                    const char *iso639_language,
                    ucs4_t (*single_character_map) (ucs4_t),
                    size_t offset_in_rule, /* offset in 'struct special_casing_rule' */
                    uninorm_t nf,
                    uint32_t *resultbuf, size_t *lengthp);


#ifdef __cplusplus
}
#endif
