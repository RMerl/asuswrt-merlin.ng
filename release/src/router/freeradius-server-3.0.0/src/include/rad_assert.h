#ifndef RAD_ASSERT_H
#define RAD_ASSERT_H
/*
 * rad_assert.h	  Debug assertions, with logging.
 *
 * Version:	$Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2000,2001,2006  The FreeRADIUS server project
 */

RCSIDH(rad_assert_h, "$Id$")

#ifdef __cplusplus
extern "C" {
#endif

extern void rad_assert_fail (char const *file, unsigned int line, char const *expr);

#ifdef NDEBUG
	#define rad_assert(expr) ((void) (0))

#elif !defined(FR_SCAN_BUILD)
	#define rad_assert(expr) \
		((void) ((expr) ? (void) 0 : \
			(void) rad_assert_fail (__FILE__, __LINE__, #expr)))

#else
#include <assert.h>
#define rad_assert assert
#endif

#ifdef __cplusplus
}
#endif

#endif
