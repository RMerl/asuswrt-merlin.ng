/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */


/*******************************************************************
 * bdmf_utils.c
 *
 * Broadlight Device Management Framework - utilities
 *
 * This file is Copyright (c) 2011, Broadlight Communications.
 * This file is licensed under GNU Public License, except that if
 * you have entered in to a signed, written license agreement with
 * Broadlight covering this file, that agreement applies to this
 * file instead of the GNU Public License.
 *
 * This file is free software: you can redistribute and/or modify it
 * under the terms of the GNU Public License, Version 2, as published
 * by the Free Software Foundation, unless a different license
 * applies as provided above.
 *
 * This program is distributed in the hope that it will be useful,
 * but AS-IS and WITHOUT ANY WARRANTY; without even the implied
 * warranties of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * TITLE or NONINFRINGEMENT. Redistribution, except as permitted by
 * the GNU Public License or another license agreement between you
 * and Broadlight, is prohibited.
 *
 * You should have received a copy of the GNU Public License,
 * Version 2 along with this file; if not, see
 * <http://www.gnu.org/licenses>.
 *
 * Author: Igor Ternovsky
 *******************************************************************/

/* #define DEBUG */

#include <bdmf_dev.h>

/** Convert hex-string to binary data
 *
 * The hex string must contain an even number of hexadecimal characters.
 *
 * \param[in]   src     0-terminated hex string
 * \param[out]  dst     buffer for binary output
 * \param[in]   dst_len dst buffer size
 *
 * \return   Returns number of data bytes if conversion succeded\n
 *  or error code < 0 if convertion failed.
 */
int bdmf_strhex(const char *src, uint8_t *dst, uint16_t dst_len)
{
    uint16_t src_len = (uint16_t)strlen( src );
    uint16_t i = src_len, j, shift = 0;

    if ( !dst || !dst_len || (src_len > 2*dst_len) || (src_len%2) )
        return BDMF_ERR_PARM;

    dst_len = src_len / 2;
    memset(dst, 0, dst_len);
    j = dst_len-1;
    do
    {
        int c = src[--i];

        if ( (c>='0') && (c<='9') )
            c = c - '0';
        else if ( (c>='a') && (c<='f') )
            c = 0xA + c - 'a';
        else if ( (c>='A') && (c<='F') )
            c = 0xA + c - 'A';
        else
            return BDMF_ERR_PARM;

        dst[j] |= (uint8_t)(c<<shift);

        j     -= shift>>2;
        shift ^= 4;

    } while( i );

    return dst_len;
}

/*
 * Exports
 */
EXPORT_SYMBOL(bdmf_strhex);
