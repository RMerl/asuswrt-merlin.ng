/* Declarations for hsts.c
   Copyright (C) 1996-2012, 2015, 2018-2024 Free Software Foundation,
   Inc.

This file is part of GNU Wget.

GNU Wget is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

GNU Wget is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Wget.  If not, see <http://www.gnu.org/licenses/>.

Additional permission under GNU GPL version 3 section 7

If you modify this program, or any covered work, by linking or
combining it with the OpenSSL project's OpenSSL library (or a
modified version of that library), containing parts covered by the
terms of the OpenSSL or SSLeay licenses, the Free Software Foundation
grants you additional permission to convey the resulting work.
Corresponding Source for a non-source form of such a combination
shall include the source code for the parts of OpenSSL used as well
as that of the covered work.  */

#ifndef WGET_HSTS_H
#define WGET_HSTS_H

#ifdef HAVE_HSTS

#include "wget.h"
#include "url.h"

typedef struct hsts_store *hsts_store_t;

hsts_store_t hsts_store_open (const char *);

void hsts_store_save (hsts_store_t, const char *);
void hsts_store_close (hsts_store_t);
bool hsts_store_has_changed (hsts_store_t);

bool hsts_store_entry (hsts_store_t,
                       enum url_scheme, const char *, int,
                       int64_t, bool);
bool hsts_match (hsts_store_t, struct url *);

#endif /* HAVE_HSTS */
#endif /* WGET_HSTS_H */
