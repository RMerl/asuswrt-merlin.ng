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
#ifndef __PJ_LIMITS_H__
#define __PJ_LIMITS_H__

/**
 * @file limits.h
 * @brief Common min and max values
 */

#include <pj/compat/limits.h>

/** Maximum value for signed 32-bit integer. */
#define PJ_MAXINT32     0x7fffffff

/** Minimum value for signed 32-bit integer. */
#define PJ_MININT32     0x80000000

/** Maximum value for unsigned 16-bit integer. */
#define PJ_MAXUINT16    0xffff

/** Maximum value for unsigned char. */
#define PJ_MAXUINT8     0xff

/** Maximum value for long. */
#define PJ_MAXLONG      LONG_MAX

/** Minimum value for long. */
#define PJ_MINLONG      LONG_MIN

/** Minimum value for unsigned long. */
#define PJ_MAXULONG     ULONG_MAX

/** Maximum value for generic unsigned integer. */
#if defined(PJ_HAS_INT64) && PJ_HAS_INT64!=0
#  define PJ_MAXUINT    0xffffffffffffffffULL
#else
#  define PJ_MAXUINT    0xffffffff
#endif

#endif  /* __PJ_LIMITS_H__ */
