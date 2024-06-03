/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2018:DUAL/GPL:standard

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
