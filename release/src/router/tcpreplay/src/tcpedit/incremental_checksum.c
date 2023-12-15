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

/*
 * This code is heavily based on (some might even say stolen from) Mike Shiffman's
 * checksumming code from Libnet 1.1.3
 */

#include "incremental_checksum.h"
#include "config.h"
#include "tcpedit.h"

static inline unsigned short
from32to16(unsigned int x)
{
    /* add up 16-bit and 16-bit for 16+c bit */
    x = (x & 0xffff) + (x >> 16);
    /* add up carry.. */
    x = (x & 0xffff) + (x >> 16);
    return x;
}

static unsigned int
do_csum(const unsigned char *buff, int len)
{
    int odd;
    unsigned int result = 0;

    if (len <= 0)
        goto out;
    odd = 1 & (unsigned long)buff;
    if (odd) {
#ifdef WORDS_BIGENDIAN
        result = *buff;
#else
        result += (*buff << 8);
#endif
        len--;
        buff++;
    }
    if (len >= 2) {
        if (2 & (unsigned long)buff) {
            result += *(unsigned short *)buff;
            len -= 2;
            buff += 2;
        }
        if (len >= 4) {
            const unsigned char *end = buff + ((unsigned)len & ~3);
            unsigned int carry = 0;
            do {
                unsigned int w = *(unsigned int *)buff;
                buff += 4;
                result += carry;
                result += w;
                carry = (w > result);
            } while (buff < end);
            result += carry;
            result = (result & 0xffff) + (result >> 16);
        }
        if (len & 2) {
            result += *(unsigned short *)buff;
            buff += 2;
        }
    }
    if (len & 1)
#ifdef WORDS_BIGENDIAN
        result += (*buff << 8);
#else
        result += *buff;
#endif
    result = from32to16(result);
    if (odd)
        result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
out:
    return result;
}

/*
 * computes the checksum of a memory block at buff, length len,
 * and adds in "sum" (32-bit)
 *
 * returns a 32-bit number suitable for feeding into itself
 * or csum_tcpudp_magic
 *
 * this function must be called with even lengths, except
 * for the last fragment, which may be odd
 *
 * it's best to have buff aligned on a 32-bit boundary
 */
__wsum
csum_partial(const void *buff, int len, __wsum wsum)
{
    unsigned int sum = (unsigned int)wsum;
    unsigned int result = do_csum(buff, len);

    /* add in old sum, and carry.. */
    result += sum;
    if (sum > result)
        result += 1;
    return (__wsum)result;
}
