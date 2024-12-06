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
 * File Name  : bcm_bitmap_pool_util.c
 * This file provides implementation for bitmap pool relation functions.
 *******************************************************************************
 */

/* Includes */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include "bcm_bitmap_pool_utils.h"
#include "bcm_bitmap_utils.h"
#include <linux/kern_levels.h>
#include <linux/bcm_log.h>

/*
 *------------------------------------------------------------------------------
 * bitmap_pool_info_t;
 * This structure is used for managing the pool of bitmaps.
 *------------------------------------------------------------------------------
 */
typedef struct {
    uint32_t *bitmap_pool_p;        /* ptr to array of managed bitmaps */
    uint32_t *status_bitmap_p;      /* alloc/free status of each bitmap */
    uint16_t num_bitmaps;           /* max number of bitmaps in the pool */
    uint16_t num_bitmaps_words;     /* max number of bitmaps-words in the pool */
    uint16_t bitmap_size;           /* size of each bitmap  */
    uint16_t bitmap_size_words;     /* size of each bitmap in words */
    uint16_t last_bitmap_idx;       /* last bitmap index that was allocated */
    uint16_t avail_bitmap_cnt;      /* Currently available bitmap count */
} bitmap_pool_info_t;


#define CC_BCM_BITMAP_POOL_DEBUG

#if defined(CC_BCM_BITMAP_POOL_DEBUG)
uint32_t bitMapPoolLogLevel = 0;  /* 1: Notice, 2:Info, 3:Debug */
#define __bitMapPoolDebug(fmt, arg...)   if (bitMapPoolLogLevel >= BCM_LOG_LEVEL_DEBUG) bcm_print("%s():%u:" fmt, __FUNCTION__, __LINE__,##arg)
#define __bitMapPoolInfo(fmt, arg...)    if (bitMapPoolLogLevel >= BCM_LOG_LEVEL_INFO) bcm_print("%s():%u:" fmt, __FUNCTION__, __LINE__,##arg)
#define __bitMapPoolNotice(fmt, arg...)  if (bitMapPoolLogLevel >= BCM_LOG_LEVEL_NOTICE) bcm_print("%s():%u:" fmt, __FUNCTION__, __LINE__,##arg)
#else
#define __bitMapPoolDebug(fmt, arg...)   
#define __bitMapPoolInfo(fmt, arg...)    
#define __bitMapPoolNotice(fmt, arg...)  
#endif

#define __bitMapPoolError(fmt, arg...)   bcm_print("%s():%u:" fmt, __FUNCTION__, __LINE__,##arg)

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_pool_init
 * Description  : Initialize the pool of bitmaps
 * Input        : num_bitmaps = Total number of bitmaps in the pool
 *              : bitmap_size = Size of each bitmap in the pool
 *------------------------------------------------------------------------------
 */
void *bcm_bitmap_pool_init(uint32_t num_bitmaps, uint32_t bitmap_size)
{
    bitmap_pool_info_t *pool_p = NULL;
    uint32_t alloc_size = 0;

    if ((bitmap_size % BCM_BITS_PER_WORD) != 0)
    {
        __bitMapPoolError("bitmap_size<%d> is not a multiple of <%d>", bitmap_size, BCM_BITS_PER_WORD);
        return NULL;
    }
    /* TODO: Remove this restriction */
    if ((num_bitmaps % BCM_BITS_PER_WORD) != 0)
    {
        __bitMapPoolError("num_bitmaps<%d> is not a multiple of <%d>", num_bitmaps, BCM_BITS_PER_WORD);
        return NULL;
    }

    pool_p = (bitmap_pool_info_t *) kmalloc(sizeof(bitmap_pool_info_t), GFP_KERNEL);
    if (pool_p == NULL)
    {
        __bitMapPoolError("Failure to bitmap pool of <%d> bytes", sizeof(bitmap_pool_info_t));
        return NULL;
    }

    pool_p->num_bitmaps = num_bitmaps;
    pool_p->num_bitmaps_words = (num_bitmaps/BCM_BITS_PER_WORD);
    pool_p->num_bitmaps_words += (num_bitmaps%BCM_BITS_PER_WORD)? 1 : 0;

    pool_p->bitmap_size = bitmap_size;
    pool_p->bitmap_size_words = (bitmap_size/BCM_BITS_PER_WORD);
    pool_p->bitmap_size_words += (bitmap_size%BCM_BITS_PER_WORD)? 1 : 0;

    alloc_size =  (pool_p->bitmap_size_words) * BCM_BYTES_PER_WORD; /* size of each bitmap */
    alloc_size =  alloc_size * num_bitmaps; /* size of bitmap pool */

    pool_p->bitmap_pool_p = (uint32_t *)kmalloc(alloc_size, GFP_KERNEL);

    if (pool_p->bitmap_pool_p == NULL)
    {
        __bitMapPoolError("Failure to allocate bitmap pool of <%d> bytes", alloc_size);
        goto pool_out;
    }
    memset((void *)pool_p->bitmap_pool_p, 0, alloc_size);

    alloc_size = (pool_p->num_bitmaps_words) * BCM_BYTES_PER_WORD; /* size of status bitmap */
    pool_p->status_bitmap_p = (uint32_t *)kmalloc(alloc_size, GFP_KERNEL);
    if (pool_p->status_bitmap_p == NULL)
    {
        __bitMapPoolError( "Failure to allocate status bitmap of size <%d>", alloc_size );
        goto status_bitmap_out;
    }

    /* Init time all the bitmaps are available for allocation */
    memset((void *)pool_p->status_bitmap_p, 0, alloc_size);
    pool_p->last_bitmap_idx = pool_p->num_bitmaps; /* Initial value */
    pool_p->avail_bitmap_cnt = pool_p->num_bitmaps;

    return pool_p;

status_bitmap_out:
    kfree(pool_p->bitmap_pool_p);
pool_out:
    kfree(pool_p);    
    return NULL;

}
EXPORT_SYMBOL(bcm_bitmap_pool_init);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_pool_alloc_bitmap
 * Description  : Allocate the first available/free bitmap
 * Input        : p = Pointer to pool
 * Return       : Returns index to the bitmap
 *              : Returns -1 if no bitmap is available.
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_pool_alloc_bitmap(void *p)
{
    bitmap_pool_info_t *pool_p = (bitmap_pool_info_t *)p;
    int alloc_idx = -1;
    if (pool_p->avail_bitmap_cnt)
    {
        alloc_idx = bcm_bitmap_get_bit_index(pool_p->status_bitmap_p, 
                                             pool_p->num_bitmaps_words);
        if ( alloc_idx >= 0 )
        {
            /* No need to memset the bitmap; Already done during free */
            pool_p->last_bitmap_idx = alloc_idx;
            pool_p->avail_bitmap_cnt--;
        }
        else
        {
            __bitMapPoolError( "Failure to allocate bitmap avail_bitmap_cnt=<%d>", 
                               pool_p->avail_bitmap_cnt );
        }
    }

    return alloc_idx;
}
EXPORT_SYMBOL(bcm_bitmap_pool_alloc_bitmap);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_pool_free_bitmap
 * Description  : Frees the previously allocated bitmap to the bitmap pool
 * Input        : p = Pointer to pool
 *              : bitmap_idx = index to the bitmap
 * Return       : Returns 0 - Success
 *              : Returns -1 - Failure.
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_pool_free_bitmap(void *p, uint16_t bitmap_idx)
{
    bitmap_pool_info_t *pool_p = (bitmap_pool_info_t *)p;

    if ( pool_p->num_bitmaps > bitmap_idx )
    {
        uint32_t *bitmap_p = &pool_p->bitmap_pool_p[bitmap_idx * pool_p->bitmap_size_words];

        int ret = bcm_bitmap_return_bit_index(pool_p->status_bitmap_p, pool_p->num_bitmaps_words, bitmap_idx);

        if ( ret < 0 )
        {
            __bitMapPoolError("Failure to free the bitmap index<%d> to pool ", bitmap_idx);
            return -1;
        }

        memset((void *)bitmap_p, 0, (pool_p->bitmap_size_words*BCM_BYTES_PER_WORD) );

        pool_p->avail_bitmap_cnt++;
    }
    else
    {
        __bitMapPoolError("Failure to free the bitmap index<%d> to pool ", bitmap_idx);
        return -1;
    }

    return bitmap_idx;
}
EXPORT_SYMBOL(bcm_bitmap_pool_free_bitmap);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_pool_alloc_bitmap
 * Description  : Allocate the next available/free bitmap starting from given idx
 * Input        : p = Pointer to pool
 *              : start_idx = bitmap index to start from
 * Return       : Returns index to the bitmap
 *              : Returns -1 if no bitmap is available.
 * Note         : This function does not wraps around the indexes 
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_pool_alloc_bitmap_next(void *p, uint16_t start_idx)
{
    int alloc_idx = -1;
    bitmap_pool_info_t *pool_p = (bitmap_pool_info_t *)p;

    if (pool_p->avail_bitmap_cnt)
    {
        alloc_idx = bcm_bitmap_get_next_bit_index(pool_p->status_bitmap_p, 
                                                  pool_p->num_bitmaps_words, start_idx);

        if ( alloc_idx >= 0 )
        {
            /* No need to memset the bitmap; Already done during free */
            pool_p->last_bitmap_idx = alloc_idx;
            pool_p->avail_bitmap_cnt--;
        }
        else
        {
            __bitMapPoolError( "Failure to allocate bitmap avail_bitmap_cnt=<%d>", 
                               pool_p->avail_bitmap_cnt );
        }
    }

    return alloc_idx;
}
EXPORT_SYMBOL(bcm_bitmap_pool_alloc_bitmap_next);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_pool_get_bitmap_ptr
 * Description  : returns the bitmap ptr for a given index
 * Input        : p = Pointer to pool
 *              : bitmap_idx = index to the bitmap
 * Return       : Returns index to the bitmap
 *              : Returns -1 if no bitmap is available.
 *------------------------------------------------------------------------------
 */
uint32_t* bcm_bitmap_pool_get_bitmap_ptr(void *p, uint32_t bitmap_idx)
{
    bitmap_pool_info_t *pool_p = (bitmap_pool_info_t *)p;
    if ( pool_p->num_bitmaps > bitmap_idx )
    {
        return &pool_p->bitmap_pool_p[bitmap_idx * pool_p->bitmap_size_words];

    }
    __bitMapPoolError("Failure to get the bitmap for index<%d> to pool ", bitmap_idx);
    return NULL;
}
EXPORT_SYMBOL(bcm_bitmap_pool_get_bitmap_ptr);
/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_pool_copy_bitmap
 * Description  : copies the bitmap to provided storage
 * Input        : p = Pointer to pool
 *              : bitmap_idx = index to the bitmap
 *              : dst_p = pointer to destination memory
 *              : dst_size_words = size of dst memory in words
 * Return       : Returns 0 if success
 *              : Returns -1 if failure.
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_pool_copy_bitmap(void *p, uint32_t bitmap_idx, uint32_t *dst_p, uint32_t dst_size_words)
{
    bitmap_pool_info_t *pool_p = (bitmap_pool_info_t *)p;
    if ( pool_p->num_bitmaps > bitmap_idx )
    {
        /* Destination always need to be equal/bigger than source */
        if ( pool_p->bitmap_size_words <= dst_size_words )
        {
            uint32_t *src_p = &pool_p->bitmap_pool_p[bitmap_idx * pool_p->bitmap_size_words];
            int array_idx = 0;
            /* TODO - check if memcpy would be better */
            for (; array_idx < dst_size_words; array_idx++)
            {
                if ( array_idx < pool_p->bitmap_size_words )
                {
                    dst_p[array_idx] = src_p[array_idx];
                }
                else
                {
                    dst_p[array_idx] = 0;
                }
            }
            return 0;
        }

    }
    __bitMapPoolError("Failure to copy the bitmap for index<%d> src-size <%d> dst-size <%d> ", 
                      bitmap_idx, pool_p->bitmap_size_words, dst_size_words);
    return -1;
}
EXPORT_SYMBOL(bcm_bitmap_pool_copy_bitmap);
/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_pool_set_bit
 * Description  : Wrapper function around bitmap utils
 *              : sets the given bit in the bitmap at a given index
 * Input        : p = Pointer to pool
 *              : bitmap_idx = index to the bitmap
 *              : bit = bit index to set in the bitmap
 * Return       : Return 0 if success
 *              : Returns -1 if failure.
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_pool_set_bit(void *p, uint32_t bitmap_idx, uint32_t bit)
{
    bitmap_pool_info_t *pool_p = (bitmap_pool_info_t *)p;
    if ( pool_p->num_bitmaps > bitmap_idx )
    {
        uint32_t *bitmap_p = &pool_p->bitmap_pool_p[bitmap_idx * pool_p->bitmap_size_words];
        return bcm_bitmap_set_bit(bitmap_p, pool_p->bitmap_size_words, bit);

    }
    __bitMapPoolError("Invalid bitmap for index<%d> to pool ", bitmap_idx);
    return -1;
}
EXPORT_SYMBOL(bcm_bitmap_pool_set_bit);
/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_pool_clear_bit
 * Description  : Wrapper function around bitmap utils
 *              : clears the given bit in the bitmap at a given index
 * Input        : p = Pointer to pool
 *              : bitmap_idx = index to the bitmap
 *              : bit = bit index to clear in the bitmap
 * Return       : Return 0 if success
 *              : Returns -1 if failure.
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_pool_clear_bit(void *p, uint32_t bitmap_idx, uint32_t bit)
{
    bitmap_pool_info_t *pool_p = (bitmap_pool_info_t *)p;
    if ( pool_p->num_bitmaps > bitmap_idx )
    {
        uint32_t *bitmap_p = &pool_p->bitmap_pool_p[bitmap_idx * pool_p->bitmap_size_words];
        return bcm_bitmap_clear_bit(bitmap_p, pool_p->bitmap_size_words, bit);

    }
    __bitMapPoolError("Invalid bitmap for index<%d> to pool ", bitmap_idx);
    return -1;
}
EXPORT_SYMBOL(bcm_bitmap_pool_clear_bit);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_pool_return_bit_index
 * Description  : Wrapper function around bitmap utils
 *              : returns the given bit in the bitmap at a given index
 * Input        : p = Pointer to pool
 *              : bitmap_idx = index to the bitmap
 *              : bit = bit index to return to the bitmap
 * Return       : Return 0 if success
 *              : Returns -1 if failure.
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_pool_return_bit_index(void *p, uint32_t bitmap_idx, uint32_t bit)
{
    bitmap_pool_info_t *pool_p = (bitmap_pool_info_t *)p;
    if ( pool_p->num_bitmaps > bitmap_idx )
    {
        uint32_t *bitmap_p = &pool_p->bitmap_pool_p[bitmap_idx * pool_p->bitmap_size_words];
        return bcm_bitmap_return_bit_index(bitmap_p, pool_p->bitmap_size_words, bit);

    }
    __bitMapPoolError("Invalid bitmap for index<%d> to pool ", bitmap_idx);
    return -1;
}
EXPORT_SYMBOL(bcm_bitmap_pool_return_bit_index);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bitmap_pool_get_next_bit_index
 * Description  : Wrapper function around bitmap utils
 *              : returns the next index from given start index
 * Input        : p = Pointer to pool
 *              : bitmap_idx = index to the bitmap
 *              : bit = bit index to start from
 * Return       : Return 0 if success
 *              : Returns -1 if failure.
 *------------------------------------------------------------------------------
 */
int bcm_bitmap_pool_get_next_bit_index(void *p, uint32_t bitmap_idx, uint32_t bit)
{
    bitmap_pool_info_t *pool_p = (bitmap_pool_info_t *)p;
    if ( pool_p->num_bitmaps > bitmap_idx )
    {
        uint32_t *bitmap_p = &pool_p->bitmap_pool_p[bitmap_idx * pool_p->bitmap_size_words];
        return bcm_bitmap_get_next_bit_index(bitmap_p, pool_p->bitmap_size_words, bit);

    }
    __bitMapPoolError("Invalid bitmap for index<%d> to pool ", bitmap_idx);
    return -1;
}
EXPORT_SYMBOL(bcm_bitmap_pool_get_next_bit_index);

/*------------------------------------------------------------------------------
 * Function     : bcm_bitmap_pool_next_start_idx
 * Description  : Return the next index to start search for bitmap.
 *                If index is > max then the value wraps around.
 * Input        : p = Pointer to pool
 *------------------------------------------------------------------------------
 */
uint16_t bcm_bitmap_pool_next_start_idx(void *p)
{
    bitmap_pool_info_t *pool_p = (bitmap_pool_info_t *)p;
    uint16_t idx = pool_p->last_bitmap_idx;

    idx++;
    if ( idx >= pool_p->num_bitmaps )
        idx = 0;

    return idx;
}
EXPORT_SYMBOL(bcm_bitmap_pool_next_start_idx);

/*------------------------------------------------------------------------------
 * Function     : bcm_bitmap_pool_dump
 * Description  : Dump info about bitmap pool and all the bitmaps
 * Input        : p = Pointer to pool
 *              : bitmap_idx = start index
 *              : in_use_only = dump in use bitmaps
 *              : num = num of bitmaps to dump
 *------------------------------------------------------------------------------
 */
void bcm_bitmap_pool_dump(void *p, int num, int bitmap_idx, int in_use_only)
{
    bitmap_pool_info_t *pool_p = (bitmap_pool_info_t *)p;
    bcm_print("Num Bitmaps = %u (words:%u)\n", pool_p->num_bitmaps, pool_p->num_bitmaps_words);
    bcm_print("Bitmap Size = %u (words:%u)\n", pool_p->bitmap_size, pool_p->bitmap_size_words);
    bcm_print("Available   = %u \n", pool_p->avail_bitmap_cnt);
    bcm_print("Last Alloc  = %u\n", pool_p->last_bitmap_idx);
    bcm_print("\n");

    bcm_print("Dump Status Bitmap: \n");
    bcm_bitmap_dump(pool_p->status_bitmap_p, pool_p->num_bitmaps_words);

    if (num < 0)
    {
        /* Dump all */
        num = pool_p->num_bitmaps;
    }
    if (bitmap_idx < 0)
    {
        /* start from 0 */
        bitmap_idx = 0;
    }
    if (num)
    {
        bcm_print("Dump Individual Bitmaps (dumps=%d): \n",num);
        for (; num && bitmap_idx < pool_p->num_bitmaps; bitmap_idx++)
        {
            if (!in_use_only || bcm_bitmap_is_bit_set(pool_p->status_bitmap_p, pool_p->num_bitmaps_words, bitmap_idx))
            {
                uint32_t *bitmap_p = &pool_p->bitmap_pool_p[bitmap_idx * pool_p->bitmap_size_words];
                bcm_print("\nDump bitmap @index %04u:\n",bitmap_idx);
                bcm_bitmap_dump(bitmap_p, pool_p->bitmap_size_words);
                num--;
            }
        }
    }
}
EXPORT_SYMBOL(bcm_bitmap_pool_dump);

/*------------------------------------------------------------------------------
 * Function     : bcm_bitmap_pool_dump_one
 * Description  : Dump info about bitmap specified by index
 * Input        : p = Pointer to pool
 *              : bitmap_idx = index
 *------------------------------------------------------------------------------
 */
void bcm_bitmap_pool_dump_one(void *p, int bitmap_idx)
{
    bitmap_pool_info_t *pool_p = (bitmap_pool_info_t *)p;
    if ( pool_p->num_bitmaps > bitmap_idx )
    {
        uint32_t *bitmap_p = &pool_p->bitmap_pool_p[bitmap_idx * pool_p->bitmap_size_words];
        bcm_print("\nDump bitmap index %04u:\n",bitmap_idx);
        bcm_bitmap_dump(bitmap_p, pool_p->bitmap_size_words);
    }
}
EXPORT_SYMBOL(bcm_bitmap_pool_dump_one);

