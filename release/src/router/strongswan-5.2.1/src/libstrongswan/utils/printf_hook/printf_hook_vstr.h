/*
 * Copyright (C) 2009 Tobias Brunner
 * Copyright (C) 2006-2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup printf_hook_vstr printf_hook_vstr
 * @{ @ingroup utils
 */

#ifndef PRINTF_HOOK_VSTR_H_
#define PRINTF_HOOK_VSTR_H_

#include <stdarg.h>
#include <stdio.h>

int vstr_wrapper_printf(const char *format, ...);
int vstr_wrapper_fprintf(FILE *stream, const char *format, ...);
int vstr_wrapper_sprintf(char *str, const char *format, ...);
int vstr_wrapper_snprintf(char *str, size_t size, const char *format, ...);
int vstr_wrapper_asprintf(char **str, const char *format, ...);

int vstr_wrapper_vprintf(const char *format, va_list ap);
int vstr_wrapper_vfprintf(FILE *stream, const char *format, va_list ap);
int vstr_wrapper_vsprintf(char *str, const char *format, va_list ap);
int vstr_wrapper_vsnprintf(char *str, size_t size, const char *format, va_list ap);
int vstr_wrapper_vasprintf(char **str, const char *format, va_list ap);

#ifdef printf
#undef printf
#endif
#ifdef fprintf
#undef fprintf
#endif
#ifdef sprintf
#undef sprintf
#endif
#ifdef snprintf
#undef snprintf
#endif
#ifdef asprintf
#undef asprintf
#endif
#ifdef vprintf
#undef vprintf
#endif
#ifdef vfprintf
#undef vfprintf
#endif
#ifdef vsprintf
#undef vsprintf
#endif
#ifdef vsnprintf
#undef vsnprintf
#endif
#ifdef vasprintf
#undef vasprintf
#endif

#define printf vstr_wrapper_printf
#define fprintf vstr_wrapper_fprintf
#define sprintf vstr_wrapper_sprintf
#define snprintf vstr_wrapper_snprintf
#define asprintf vstr_wrapper_asprintf

#define vprintf vstr_wrapper_vprintf
#define vfprintf vstr_wrapper_vfprintf
#define vsprintf vstr_wrapper_vsprintf
#define vsnprintf vstr_wrapper_vsnprintf
#define vasprintf vstr_wrapper_vasprintf

#endif /** PRINTF_HOOK_VSTR_H_ @}*/
