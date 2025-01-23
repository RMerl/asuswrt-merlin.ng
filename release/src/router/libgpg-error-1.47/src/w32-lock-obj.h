/* w32-lock-obj.h - Declaration of the Windows lock object
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

#ifndef W32_LOCK_OBJ_H
#define W32_LOCK_OBJ_H

#define LOCK_ABI_VERSION 1

/* The real definition of our lock object.  The public definition is
   named gpgrt_lock_t and hides this internal structure.  */
#pragma pack(push, 8)
typedef struct
{
  long vers;
  volatile long initdone;
  volatile long started;
  CRITICAL_SECTION csec;
} _gpgrt_lock_t;
#pragma pack(pop)


#endif /*W32_LOCK_OBJ_H*/
