/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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
 * @defgroup printf_hook_builtin printf_hook_builtin
 * @{ @ingroup utils
 */

#ifndef PRINTF_HOOK_BUILTIN_H_
#define PRINTF_HOOK_BUILTIN_H_

#include <stdarg.h>
#include <stdio.h>

int builtin_printf(const char *format, ...);
int builtin_fprintf(FILE *stream, const char *format, ...);
int builtin_sprintf(char *str, const char *format, ...);
int builtin_snprintf(char *str, size_t size, const char *format, ...);
int builtin_asprintf(char **str, const char *format, ...);

int builtin_vprintf(const char *format, va_list ap);
int builtin_vfprintf(FILE *stream, const char *format, va_list ap);
int builtin_vsprintf(char *str, const char *format, va_list ap);
int builtin_vsnprintf(char *str, size_t size, const char *format, va_list ap);
int builtin_vasprintf(char **str, const char *format, va_list ap);

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

#define printf builtin_printf
#define fprintf builtin_fprintf
#define sprintf builtin_sprintf
#define snprintf builtin_snprintf
#define asprintf builtin_asprintf

#define vprintf builtin_vprintf
#define vfprintf builtin_vfprintf
#define vsprintf builtin_vsprintf
#define vsnprintf builtin_vsnprintf
#define vasprintf builtin_vasprintf

#endif /** PRINTF_HOOK_BUILTIN_H_ @}*/
