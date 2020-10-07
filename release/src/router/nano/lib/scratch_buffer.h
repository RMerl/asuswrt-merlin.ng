/* Variable-sized buffer with on-stack default allocation.
   Copyright (C) 2017-2020 Free Software Foundation, Inc.

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

/* Written by Paul Eggert, 2017.  */

#ifndef _GL_SCRATCH_BUFFER_H
#define _GL_SCRATCH_BUFFER_H

#include <libc-config.h>

#define __libc_scratch_buffer_grow gl_scratch_buffer_grow
#define __libc_scratch_buffer_grow_preserve gl_scratch_buffer_grow_preserve
#define __libc_scratch_buffer_set_array_size gl_scratch_buffer_set_array_size
#include <malloc/scratch_buffer.h>

#endif /* _GL_SCRATCH_BUFFER_H */
