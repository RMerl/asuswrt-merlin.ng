/* Copy wide character array, return pointer after last written wide character.
   Copyright (C) 2020-2021 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include <wchar.h>

/* Copy N wide characters of SRC to DEST.
   Return pointer to wide characters after the last written wide character.  */
wchar_t *
wmempcpy (wchar_t *dest, const wchar_t *src, size_t n)
{
  return wmemcpy (dest, src, n) + n;
}
