/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2013:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

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
