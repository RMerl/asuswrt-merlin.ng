/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup types_i types
 * @{ @ingroup utils_i
 */

#ifndef TYPES_H_
#define TYPES_H_

/**
 * General purpose boolean type.
 */
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef HAVE__BOOL
#  define _Bool signed char
# endif /* HAVE__BOOL */
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif /* HAVE_STDBOOL_H */
#ifndef FALSE
# define FALSE false
#endif /* FALSE */
#ifndef TRUE
# define TRUE  true
#endif /* TRUE */

#ifdef HAVE_INT128
/**
 * 128 bit wide signed integer, if supported
 */
typedef __int128 int128_t;
/**
 * 128 bit wide unsigned integer, if supported
 */
typedef unsigned __int128 u_int128_t;

# define MAX_INT_TYPE int128_t
# define MAX_UINT_TYPE u_int128_t
#else
# define MAX_INT_TYPE int64_t
# define MAX_UINT_TYPE uint64_t
#endif

/**
 * deprecated pluto style return value:
 * error message, NULL for success
 */
typedef const char *err_t;

/**
 * Handle struct sockaddr as a simpler sockaddr_t type.
 */
typedef struct sockaddr sockaddr_t;

#endif /** TYPES_H_ @} */
