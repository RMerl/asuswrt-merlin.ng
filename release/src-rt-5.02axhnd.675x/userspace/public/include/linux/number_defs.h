/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2018:DUAL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
 *
 ************************************************************************/

#ifndef __NUMBER_DEFS_H__
#define __NUMBER_DEFS_H__

#include <stdint.h>  /* for the various integer types */

/*!\file number_defs.h
 * \brief Common number definitions.
 *
 *  This file assumes we are running on a Linux system.  Can be changed for
 *  other systems where sizes of numbers are different.
 */

#ifndef NUMBER_TYPES_ALREADY_DEFINED
#define NUMBER_TYPES_ALREADY_DEFINED

/** Unsigned 64 bit integer.
 */
typedef uint64_t   UINT64;

/** Signed 64 bit integer.
 */
typedef int64_t    SINT64;

/** Unsigned 32 bit integer. */
typedef uint32_t   UINT32;

/** Signed 32 bit integer. */
typedef int32_t    SINT32;

/** Unsigned 16 bit integer. */
typedef uint16_t   UINT16;

/** Signed 16 bit integer. */
typedef int16_t    SINT16;

/** Unsigned 8 bit integer. */
typedef uint8_t    UINT8;

/** Signed 8 bit integer. */
typedef int8_t     SINT8;

#endif /* NUMBER_TYPES_ALREADY_DEFINED */


#endif /* __NUMBER_DEFS_H__ */
