/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

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
*/
#ifndef __LIB_BYTEORDER_H__
#define __LIB_BYTEORDER_H__

#include "lib_swab.h"

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

#define __cpu_to_le16(x)      swab16(x)
#define __le16_to_cpu(x)      swab16(x)
#define __cpu_to_le32(x)      swab32(x)
#define __le32_to_cpu(x)      swab32(x)
#define __cpu_to_le64(x)      swab64(x)
#define __le64_to_cpu(x)      swab64(x)

#define __cpu_to_be16(x)      (x)
#define __be16_to_cpu(x)      (x)
#define __cpu_to_be32(x)      (x)
#define __be32_to_cpu(x)      (x)
#define __cpu_to_be64(x)      (x)
#define __be64_to_cpu(x)      (x)

#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define __cpu_to_le16(x)      (x)
#define __le16_to_cpu(x)      (x)
#define __cpu_to_le32(x)      (x)
#define __le32_to_cpu(x)      (x)
#define __cpu_to_le64(x)      (x)
#define __le64_to_cpu(x)      (x)

#define __cpu_to_be16(x)      swab16(x)
#define __be16_to_cpu(x)      swab16(x)
#define __cpu_to_be32(x)      swab32(x)
#define __be32_to_cpu(x)      swab32(x)
#define __cpu_to_be64(x)      swab64(x)
#define __be64_to_cpu(x)      swab64(x)

#else
#error "compiler __BYTE_ORDER__ not defined!"
#endif

#define cpu_to_le16 __cpu_to_le16
#define le16_to_cpu __le16_to_cpu
#define cpu_to_le32 __cpu_to_le32
#define le32_to_cpu __le32_to_cpu
#define cpu_to_le64 __cpu_to_le64
#define le64_to_cpu __le64_to_cpu

#define cpu_to_be16 __cpu_to_be16
#define be16_to_cpu __be16_to_cpu
#define cpu_to_be32 __cpu_to_be32
#define be32_to_cpu __be32_to_cpu
#define cpu_to_be64 __cpu_to_be64
#define be64_to_cpu __be64_to_cpu

#define ___htons(x) __cpu_to_be16(x)
#define ___htonl(x) __cpu_to_be32(x)
#define ___ntohs(x) __be16_to_cpu(x)
#define ___ntohl(x) __be32_to_cpu(x)

#define htons(x) ___htons(x)
#define ntohs(x) ___ntohs(x)
#define htonl(x) ___htonl(x)
#define ntohl(x) ___ntohl(x)

#endif
