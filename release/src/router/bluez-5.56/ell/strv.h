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

#ifndef __ELL_STRV_H
#define __ELL_STRV_H

#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void l_strfreev(char **strlist);
char **l_strsplit(const char *str, const char sep);
char **l_strsplit_set(const char *str, const char *separators);
char *l_strjoinv(char **str_array, const char delim);

char **l_strv_new(void);
void l_strv_free(char **str_array);
unsigned int l_strv_length(char **str_array);
bool l_strv_contains(char **str_array, const char *item);
char **l_strv_append(char **str_array, const char *str);
char **l_strv_append_printf(char **str_array, const char *format, ...)
					__attribute__((format(printf, 2, 3)));
char **l_strv_append_vprintf(char **str_array, const char *format,
							va_list args)
					__attribute__((format(printf, 2, 0)));
char **l_strv_copy(char **str_array);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_STRV_H */
