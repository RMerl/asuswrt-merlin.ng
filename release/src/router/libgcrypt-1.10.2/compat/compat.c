/* compat.c - Dummy file to avoid an empty library.
 * Copyright (C) 2010  Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "../src/g10lib.h"


const char *
_gcry_compat_identification (void)
{
  /* For complete list of copyright holders see the file AUTHORS in
     the source distribution.  */
  static const char blurb[] =
    "\n\n"
    "This is Libgcrypt " PACKAGE_VERSION " - The GNU Crypto Library\n"
    "Copyright (C) 2012-2022 g10 Code GmbH\n"
    "Copyright (C) 2013-2022 Jussi Kivilinna\n"
    "Copyright (C) 2000-2018 Free Software Foundation, Inc.\n"
    "\n"
    "(" BUILD_REVISION " " BUILD_TIMESTAMP ")\n"
    "\n\n";
  return blurb;
}
