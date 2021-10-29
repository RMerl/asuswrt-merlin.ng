/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2013:DUAL/GPL:standard

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
/* this header provides the target endian handling macro for the host image
   utilities. 
*/

#ifndef __BCMTARGETENDIAN_H__
#define __BCMTARGETENDIAN_H__

#include <endian.h>
#include <byteswap.h>

#define BCM_TARGET_BIG_ENDIAN             0
#define BCM_TARGET_LITTLE_ENDIAN          1

static int targetEndianess = BCM_TARGET_BIG_ENDIAN;

#define BCM_SET_TARGET_ENDIANESS(endian)  (targetEndianess = (endian));
#define BCM_GET_TARGET_ENDIANESS(endian)  targetEndianess
#define BCM_GET_TARGET_ENDIANESS_STR()    ((targetEndianess == BCM_TARGET_BIG_ENDIAN) ? "big endian" : "little endian")
#define BCM_IS_TARGET_BIG_ENDIAN()        (targetEndianess == BCM_TARGET_BIG_ENDIAN)
#define BCM_IS_TARGET_LITTLE_ENDIAN()     (targetEndianess == BCM_TARGET_LITTLE_ENDIAN)

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define BCM_HOST_TO_TARGET16(x)           \
    ((targetEndianess == BCM_TARGET_BIG_ENDIAN) ? bswap_16(x) : (x))
#define BCM_HOST_TO_TARGET32(x)           \
    ((targetEndianess == BCM_TARGET_BIG_ENDIAN) ? bswap_32(x) : (x))
#define BCM_HOST_TO_TARGET64(x)           \
    ((targetEndianess == BCM_TARGET_BIG_ENDIAN) ? bswap_64(x) : (x))

#define BCM_TARGET16_TO_HOST(x)           \
    ((targetEndianess == BCM_TARGET_BIG_ENDIAN) ? bswap_16(x) : (x))
#define BCM_TARGET32_TO_HOST(x)           \
    ((targetEndianess == BCM_TARGET_BIG_ENDIAN) ? bswap_32(x) : (x))
#define BCM_TARGET64_TO_HOST(x)           \
    ((targetEndianess == BCM_TARGET_BIG_ENDIAN) ? bswap_64(x) : (x))
#else
#define BCM_HOST_TO_TARGET16(x)           \
    ((targetEndianess == BCM_TARGET_BIG_ENDIAN) ? (x) : bswap_16(x))
#define BCM_HOST_TO_TARGET32(x)           \
    ((targetEndianess == BCM_TARGET_BIG_ENDIAN) ? (x) : bswap_32(x))
#define BCM_HOST_TO_TARGET64(x)           \
    ((targetEndianess == BCM_TARGET_BIG_ENDIAN) ? (x) : bswap_64(x))

#define BCM_TARGET16_TO_HOST(x)           \
    ((targetEndianess == BCM_TARGET_BIG_ENDIAN) ? (x) : bswap_16(x))
#define BCM_TARGET32_TO_HOST(x)           \
    ((targetEndianess == BCM_TARGET_BIG_ENDIAN) ? (x) : bswap_32(x))
#define BCM_TARGET64_TO_HOST(x)           \
    ((targetEndianess == BCM_TARGET_BIG_ENDIAN) ? (x) : bswap_64(x))
#endif

#endif
