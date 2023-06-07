/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011-2014  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __ELL_STRING_H
#define __ELL_STRING_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

struct l_string;

struct l_string *l_string_new(size_t initial_length);
void l_string_free(struct l_string *string);
char *l_string_unwrap(struct l_string *string);

struct l_string *l_string_append(struct l_string *dest, const char *src);
struct l_string *l_string_append_c(struct l_string *dest, const char c);
struct l_string *l_string_append_fixed(struct l_string *dest, const char *src,
					size_t max);

void l_string_append_vprintf(struct l_string *dest,
					const char *format, va_list args)
					__attribute__((format(printf, 2, 0)));
void l_string_append_printf(struct l_string *dest, const char *format, ...)
					__attribute__((format(printf, 2, 3)));

struct l_string *l_string_truncate(struct l_string *string, size_t new_size);

unsigned int l_string_length(struct l_string *string);

char **l_parse_args(const char *args, int *out_n_args);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_STRING_H */
