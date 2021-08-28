/* Type-safe arrays which grow dynamically.
   Copyright 2021 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Paul Eggert, 2021.  */

#ifndef _GL_DYNARRAY_H
#define _GL_DYNARRAY_H

#include <libc-config.h>

#define __libc_dynarray_at_failure gl_dynarray_at_failure
#define __libc_dynarray_emplace_enlarge gl_dynarray_emplace_enlarge
#define __libc_dynarray_finalize gl_dynarray_finalize
#define __libc_dynarray_resize_clear gl_dynarray_resize_clear
#define __libc_dynarray_resize gl_dynarray_resize
#include <malloc/dynarray.h>

#endif /* _GL_DYNARRAY_H */
