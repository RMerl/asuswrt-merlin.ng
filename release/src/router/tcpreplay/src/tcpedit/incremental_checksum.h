/* $Id$ */

/*
 *   Copyright (c) 2001-2010 Aaron Turner <aturner at synfin dot net>
 *   Copyright (c) 2013-2015 Fred Klassen <tcpreplay at appneta dot com> - AppNeta
 *
 *   The Tcpreplay Suite of tools is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, either version 3 of the
 *   License, or with the authors permission any later version.
 *
 *   The Tcpreplay Suite is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with the Tcpreplay Suite.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "defines.h"
#include "config.h"

/*
 * shamelessly stolen from Linux kernel
 *
 * Implements RFC 1624 (Incremental Internet Checksum)
 * 3. Discussion states :
 *     HC' = ~(~HC + ~m + m')
 *  m : old value of a 16bit field
 *  m' : new value of a 16bit field
 */
typedef uint16_t __le16;
typedef uint16_t __be16;
typedef uint32_t __le32;
typedef uint32_t __be32;

typedef uint16_t __sum16;
typedef uint32_t __wsum;

/*
 * Fold a partial checksum
 */
static inline __sum16
csum_fold(__wsum csum)
{
    uint32_t sum = (uint32_t)csum;
    sum = (sum & 0xffff) + (sum >> 16);
    sum = (sum & 0xffff) + (sum >> 16);
    return (__sum16)~sum;
}

static inline __wsum
csum_add(__wsum csum, __wsum addend)
{
    uint32_t res = (uint32_t)csum;
    res += (uint32_t)addend;
    return (__wsum)(res + (res < (uint32_t)addend));
}

static inline __wsum
csum_sub(__wsum csum, __wsum addend)
{
    return csum_add(csum, ~addend);
}

static inline __sum16
csum16_add(__sum16 csum, __be16 addend)
{
    uint16_t res = (uint16_t)csum;

    res += (uint16_t)addend;
    return (__sum16)(res + (res < (uint16_t)addend));
}

static inline __sum16
csum16_sub(__sum16 csum, __be16 addend)
{
    return csum16_add(csum, ~addend);
}

static inline __wsum
csum_unfold(__sum16 n)
{
    return (__wsum)n;
}

__wsum csum_partial(const void *buff, int len, __wsum wsum);
static inline void
csum_replace16(__sum16 *sum, const __be32 *from, const __be32 *to)
{
    __be32 diff[] = {
            ~from[0],
            ~from[1],
            ~from[2],
            ~from[3],
            to[0],
            to[1],
            to[2],
            to[3],
    };

    *sum = csum_fold(csum_partial(diff, sizeof(diff), ~csum_unfold(*sum)));
}

static inline void
csum_replace4(__sum16 *sum, __be32 from, __be32 to)
{
    *sum = csum_fold(csum_add(csum_sub(~csum_unfold(*sum), from), to));
}

static inline void
csum_replace2(__sum16 *sum, __be16 from, __be16 to)
{
    *sum = ~csum16_add(csum16_sub(~(*sum), from), to);
}
