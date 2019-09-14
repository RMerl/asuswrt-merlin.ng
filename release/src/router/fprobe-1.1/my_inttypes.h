/*
	Copyright (C) Slava Astashonok <sla@0n.ru>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.

	$Id: my_inttypes.h,v 1.1.2.1 2004/08/13 20:41:19 sla Exp $
*/

#ifndef _MY_INTTYPES_H_
#define _MY_INTTYPES_H_

#include <config.h>

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#else

typedef unsigned char uint8_t;
typedef signed char int8_t;

typedef unsigned short uint16_t;
typedef short int16_t;

#if SIZEOF_INT == 4
typedef unsigned int uint32_t;
typedef int int32_t;
#else
#if SIZEOF_LONG == 4
typedef unsigned long uint32_t;
typedef long int32_t;
#else
#error "Can't find any 32 bit type"
#endif
#endif

#if SIZEOF_LONG == 8
typedef unsigned long uint64_t;
typedef long int64_t;
#else
#if SIZEOF_LONG_LONG == 8
typedef unsigned long long uint64_t;
typedef long long int64_t;
#else
#error "Can't find any 64 bit type"
#endif
#endif

#endif /* HAVE_INTTYPES_H */

#endif /* _MY_INTTYPES_H_ */
