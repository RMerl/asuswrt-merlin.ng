/*
 * Copyright (C) 2017 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2017 George Joseph <gjoseph@digium.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */
#ifndef __PJ_COMPAT_LIMITS_H__
#define __PJ_COMPAT_LIMITS_H__

/**
 * @file limits.h
 * @brief Provides integer limits normally found in limits.h.
 */

#include <pj/config.h>

#if defined(PJ_HAS_LIMITS_H) && PJ_HAS_LIMITS_H != 0
#  include <limits.h>
#else

#  ifdef _MSC_VER
#  pragma message("limits.h is not found or not supported. LONG_MIN and "\
                 "LONG_MAX will be defined by the library in "\
                 "pj/compats/limits.h and overridable in config_site.h")
#  else
#  warning "limits.h is not found or not supported. LONG_MIN and LONG_MAX " \
           "will be defined by the library in pj/compats/limits.h and "\
           "overridable in config_site.h"
#  endif

/* Minimum and maximum values a `signed long int' can hold.  */
#  ifndef LONG_MAX
#    if __WORDSIZE == 64
#      define LONG_MAX     9223372036854775807L
#    else
#      define LONG_MAX     2147483647L
#    endif
#  endif

#  ifndef LONG_MIN
#    define LONG_MIN      (-LONG_MAX - 1L)
#  endif

/* Maximum value an `unsigned long int' can hold.  (Minimum is 0.)  */
#  ifndef ULONG_MAX
#    if __WORDSIZE == 64
#      define ULONG_MAX    18446744073709551615UL
#    else    
#      define ULONG_MAX    4294967295UL
#    endif
#  endif
#endif

#endif  /* __PJ_COMPAT_LIMITS_H__ */
