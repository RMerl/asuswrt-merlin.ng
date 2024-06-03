/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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

/*
 *******************************************************************************
 * File Name  : bcm_bitmap_util.c
 * This file provides implementation for bitmap relation functions.
 *******************************************************************************
 */

/* Includes */
#include <linux/kernel.h>
#include <linux/types.h>
#include "bcm_bitmap_utils.h"
#include <linux/kern_levels.h>
#include <linux/bcm_log.h>

#define CC_BCM_BITMAP_DEBUG

#if defined(CC_BCM_BITMAP_DEBUG)
uint32_t bitMapLogLevel = 0;  /* 1: Notice, 2:Info, 3:Debug */
#define __bitMapDebug(fmt, arg...)   if (bitMapLogLevel >= BCM_LOG_LEVEL_DEBUG) bcm_print("%s():%u:" fmt, __FUNCTION__, __LINE__,##arg)
#define __bitMapInfo(fmt, arg...)    if (bitMapLogLevel >= BCM_LOG_LEVEL_INFO) bcm_print("%s():%u:" fmt, __FUNCTION__, __LINE__,##arg)
#define __bitMapNotice(fmt, arg...)  if (bitMapLogLevel >= BCM_LOG_LEVEL_NOTICE) bcm_print("%s():%u:" fmt, __FUNCTION__, __LINE__,##arg)
#else
#define __bitMapDebug(fmt, arg...)   
#define __bitMapInfo(fmt, arg...)    
#define __bitMapNotice(fmt, arg...)  
#endif

#define __bitMapError(fmt, arg...)   bcm_print("%s():%u:" fmt, __FUNCTION__, __LINE__,##arg)

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_set_bit
 * Description  : Set the specified bit in the bitmap
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_set_bit(uint32_t *bitmap_p, uint32_t num_words, uint32_t bit_num)
{
    int idx;
    uint32_t bit;

    bit = bit_num % BCM_BITS_PER_WORD;
    idx = bit_num / BCM_BITS_PER_WORD;

    if (idx >= num_words)
    {
        __bitMapError("out-of-range bit_num = %u", bit_num);
        return -1;
    }

    if (bitmap_p[idx] & (BCM_GET_MASK(bit)))
    {
        __bitMapError("Already set bit_num = %u", bit_num);
        /* Duplicate free */
        return -1;
    }
    bitmap_p[idx] |= (BCM_GET_MASK(bit));
    __bitMapDebug(" bit_num = %u idx=%u bit=%u new bitmap=0x%08x\n", 
                  bit_num, idx, bit, bitmap_p[idx]);
    return idx;
}
EXPORT_SYMBOL(bcm_bitmap_set_bit);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_clear_bit
 * Description  : Set the specified bit in the bitmap
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_clear_bit(uint32_t *bitmap_p, uint32_t num_words, uint32_t bit_num)
{
    int idx;
    uint32_t bit;

    bit = bit_num % BCM_BITS_PER_WORD;
    idx = bit_num / BCM_BITS_PER_WORD;
    if (idx >= num_words)
    {
        __bitMapError("out-of-range bit_num = %u", bit_num);
        return -1;
    }

    if (!(bitmap_p[idx] & (BCM_GET_MASK(bit))))
    {
        __bitMapError("Already clear bit_num = %u", bit_num);
        /* Duplicate free */
        return -1;
    }
    bitmap_p[idx] &= ~(BCM_GET_MASK(bit));
    __bitMapDebug(" bit_num = %u idx=%u bit=%u new bitmap=0x%08x\n", 
                  bit_num, idx, bit, bitmap_p[idx]);
    return idx;
}
EXPORT_SYMBOL(bcm_bitmap_clear_bit);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_is_bit_set
 * Description  : Check if specified bit set(1)
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_is_bit_set(uint32_t *bitmap_p, uint32_t num_words, uint32_t bit_num)
{
    int idx;
    uint32_t bit;

    bit = bit_num % BCM_BITS_PER_WORD;
    idx = bit_num / BCM_BITS_PER_WORD;
    if (idx >= num_words)
    {
        __bitMapError("out-of-range bit_num = %u", bit_num);
        return -1;
    }

    __bitMapDebug(" bit_num = %u idx=%u bit=%u new bitmap=0x%08x\n", 
                  bit_num, idx, bit, bitmap_p[idx]);

    return (bitmap_p[idx] & (BCM_GET_MASK(bit))) ? 1 : 0;
}
EXPORT_SYMBOL(bcm_bitmap_is_bit_set);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_get_bit_index
 * Description  : User interface to get the next free bit index from the bitmap
 * Return       : -ve = Error/No free index, otherwise the free bit index value
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_get_bit_index(uint32_t *bitmap_p, uint32_t num_words)
{
    uint32_t value;
    uint32_t bit;
    uint32_t word_idx;
    int zero_bit_next_idx = -1;

    for (word_idx = 0; word_idx < num_words; word_idx++)
    {
        __bitMapDebug(" word_idx=%u bitmap=0x%08x\n", word_idx, bitmap_p[word_idx]);
        value = ~bitmap_p[word_idx];
        while (value)
        {
            bit = ffs(value);
            if (!bit)
            {
                break;
            }

            bit--; /* ffs return 1-based position */
            zero_bit_next_idx = (bit)+ (word_idx * BCM_BITS_PER_WORD);
            bitmap_p[word_idx] |= (BCM_GET_MASK(bit));
            __bitMapDebug(" word_idx=%u next_idx=%u bit=%u new bitmap=0x%08x \n", 
                          word_idx, zero_bit_next_idx, bit, bitmap_p[word_idx]);
            return zero_bit_next_idx;
        }
    }
    return zero_bit_next_idx;
}
EXPORT_SYMBOL(bcm_bitmap_get_bit_index);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_return_bit_index
 * Description  : User interface to return the previously allocated bit index to the bitmap
 * Return       : -ve = Error (Duplicate/Out-of-range), otherwise success
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_return_bit_index(uint32_t *bitmap_p, uint32_t num_words, uint32_t bit_idx)
{
    int err;
    err = bcm_bitmap_clear_bit(bitmap_p, num_words, bit_idx);
    if (err < 0) /* +ive values are index */
    {
        __bitMapError("Error : Invalid bit_idx <%u> to free\n", bit_idx);
        return -1;
    }
    __bitMapDebug("returned bit_idx = %u\n", bit_idx);
    return 0;
}
EXPORT_SYMBOL(bcm_bitmap_return_bit_index);

typedef enum {
    GET_BIT = 1,
    FIND_BIT = 2,
}bitmap_op_t;
/*
 *------------------------------------------------------------------------------
 * Function     : _bcm_bitmap_next_one
 * Description  : Traverse the bitmap to find/get the next one bit 
 *------------------------------------------------------------------------------
 */
static int _bcm_bitmap_next_one(bitmap_op_t op, uint32_t *bitmap_p, uint32_t num_words, uint32_t bit_idx)
{
    uint32_t value;
    uint32_t bit;
    uint32_t word_idx;
    int one_bit_next_idx = -1;

    for (word_idx = (bit_idx/BCM_BITS_PER_WORD); word_idx < num_words; word_idx++)
    {
        __bitMapDebug(" word_idx=%u bitmap=0x%08x\n", word_idx, bitmap_p[word_idx]);
        value = bitmap_p[word_idx];
        while (value)
        {
            bit = ffs(value);
            if (!bit)
            {
                break;
            }

            bit--; /* ffs return 1-based position */
            one_bit_next_idx = (bit)+word_idx * BCM_BITS_PER_WORD;
            if (one_bit_next_idx >= bit_idx)
            {
                if (op == GET_BIT)
                {
                    bitmap_p[word_idx] &= ~(BCM_GET_MASK(bit));
                }
                __bitMapDebug(" word_idx=%u bit_idx=%u next_idx=%u bit=%u new bitmap=0x%08x \n", 
                        word_idx, bit_idx, one_bit_next_idx, bit, bitmap_p[word_idx]);
                return one_bit_next_idx;
            }
            value &= ~(BCM_GET_MASK(bit));
        }
    }
    return one_bit_next_idx;
}

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_find_next_one
 * Description  : Traverse the bitmap to find the next one bit 
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_find_next_one(uint32_t *bitmap_p, uint32_t num_words, uint32_t bit_idx)
{
    return _bcm_bitmap_next_one(FIND_BIT, bitmap_p, num_words, bit_idx);
}
EXPORT_SYMBOL(bcm_bitmap_find_next_one);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_get_next_one
 * Description  : Traverse the bitmap to get the next one bit 
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_get_next_one(uint32_t *bitmap_p, uint32_t num_words, uint32_t bit_idx)
{
    return _bcm_bitmap_next_one(GET_BIT, bitmap_p, num_words, bit_idx);
}
EXPORT_SYMBOL(bcm_bitmap_get_next_one);

/*
 *------------------------------------------------------------------------------
 * Function     : _bcm_bitmap_next_zero
 * Description  : Traverse the bitmap to find/get the next zero bit 
 *------------------------------------------------------------------------------
 */
static int _bcm_bitmap_next_zero(bitmap_op_t op, uint32_t *bitmap_p, uint32_t num_words, uint32_t bit_idx)
{
    uint32_t value;
    uint32_t bit;
    uint32_t word_idx;
    int zero_bit_next_idx = -1;

    for (word_idx = (bit_idx/BCM_BITS_PER_WORD); word_idx < num_words; word_idx++)
    {
        __bitMapDebug(" word_idx=%u bitmap=0x%08x\n", word_idx, bitmap_p[word_idx]);
        value = ~bitmap_p[word_idx];
        while (value)
        {
            bit = ffs(value);
            if (!bit)
            {
                break;
            }

            bit--; /* ffs return 1-based position */
            zero_bit_next_idx = (bit)+word_idx * BCM_BITS_PER_WORD;
            if (zero_bit_next_idx >= bit_idx)
            {
                if (op == GET_BIT)
                {
                    bitmap_p[word_idx] |= (BCM_GET_MASK(bit));
                }
                __bitMapDebug(" word_idx=%u bit_idx=%u next_idx=%u bit=%u new bitmap=0x%08x \n", 
                        word_idx, bit_idx, zero_bit_next_idx, bit, bitmap_p[word_idx]);
                return zero_bit_next_idx;
            }
            value &= ~(BCM_GET_MASK(bit));
        }
    }
    return zero_bit_next_idx;
}

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_find_next_zero
 * Description  : Traverse the bitmap to find the next zero bit 
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_find_next_zero(uint32_t *bitmap_p, uint32_t num_words, uint32_t bit_idx)
{
    return _bcm_bitmap_next_zero(FIND_BIT, bitmap_p, num_words, bit_idx);
}
EXPORT_SYMBOL(bcm_bitmap_find_next_zero);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_get_next_zero
 * Description  : Traverse the bitmap to find the next zero bit 
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_get_next_zero(uint32_t *bitmap_p, uint32_t num_words, uint32_t bit_idx)
{
    return _bcm_bitmap_next_zero(GET_BIT, bitmap_p, num_words, bit_idx);
}
EXPORT_SYMBOL(bcm_bitmap_get_next_zero);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_get_next_bit_index
 * Description  : Allocates the next bit index starting from given bit_idx 
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_get_next_bit_index(uint32_t *bitmap_p, uint32_t num_words, uint32_t bit_idx)
{
    return _bcm_bitmap_next_zero(GET_BIT, bitmap_p, num_words, bit_idx);
}
EXPORT_SYMBOL(bcm_bitmap_get_next_bit_index);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_dump
 * Description  : Debug function to dump the given bitmap 
 *------------------------------------------------------------------------------
 */
void bcm_bitmap_dump(uint32_t *bitmap_p, uint32_t num_words)
{
    int i;

    for (i = 0; i < num_words; i++)
    {
        if (i%8 == 0)
        {
            bcm_print("\n");
        }
        bcm_print("%08x ", bitmap_p[i]);
    }

    bcm_print("\n");
}
EXPORT_SYMBOL(bcm_bitmap_dump);


