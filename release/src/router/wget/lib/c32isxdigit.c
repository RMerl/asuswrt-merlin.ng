/* Test 32-bit wide character for being a hexadecimal digit.
   Copyright (C) 2020-2024 Free Software Foundation, Inc.

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

#include <config.h>

#define IN_C32ISXDIGIT
/* Specification.  */
#include <uchar.h>

#define FUNC c32isxdigit
#define WCHAR_FUNC iswxdigit
#define UCS_FUNC uc_is_xdigit
#include "c32is-impl.h"
