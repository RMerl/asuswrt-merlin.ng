/* protos.h - Miscellaneous prototypes
 * Copyright (C) 2020 g10 Code GmbH
 *
 * This file is part of libgpg-error.
 *
 * libgpg-error is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * libgpg-error is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <https://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef _GPGRT_PROTOS_H
#define _GPGRT_PROTOS_H

/*-- w32-gettext.c --*/
wchar_t *_gpgrt_utf8_to_wchar (const char *string);
void     _gpgrt_free_wchar (wchar_t *wstring);
char    *_gpgrt_wchar_to_utf8 (const wchar_t *string, size_t length);

/*-- estream.c --*/
void     _gpgrt_w32_set_errno (int ec);
gpg_err_code_t _gpgrt_w32_get_last_err_code (void);


#endif /*_GPGRT_PROTOS_H*/
