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
 * File Name  : bcm_bitmap_pool_utils.h
 * This file provides interface for bitmap pool utitilies.
 *******************************************************************************
 */

#ifndef _BCM_BITMAP_POOL_UTILS_H
#define _BCM_BITMAP_POOL_UTILS_H

/* Interface functions */

void *bcm_bitmap_pool_init(uint32_t num_bitmaps, uint32_t bitmap_size);
int bcm_bitmap_pool_alloc_bitmap(void *p);
int bcm_bitmap_pool_free_bitmap(void *p, uint16_t bitmap_idx);
int bcm_bitmap_pool_alloc_bitmap_next(void *p, uint16_t start_idx);
uint32_t* bcm_bitmap_pool_get_bitmap_ptr(void *p, uint32_t bitmap_idx);
int bcm_bitmap_pool_copy_bitmap(void *p, uint32_t bitmap_idx, uint32_t *dst_p, 
                                uint32_t dst_size_words);
void bcm_bitmap_pool_dump(void *p, int num, int bitmap_idx, int in_use_only);
void bcm_bitmap_pool_dump_one(void *p, int bitmap_idx);

/* Wrapper functions around bcm_bitmap_utils */
int bcm_bitmap_pool_set_bit(void *p, uint32_t bitmap_idx, uint32_t bit);
int bcm_bitmap_pool_clear_bit(void *p, uint32_t bitmap_idx, uint32_t bit);
int bcm_bitmap_pool_return_bit_index(void *p, uint32_t bitmap_idx, uint32_t bit);
int bcm_bitmap_pool_get_next_bit_index(void *p, uint32_t bitmap_idx, uint32_t bit);
#endif /* _BCM_BITMAP_POOL_UTILS_H */
