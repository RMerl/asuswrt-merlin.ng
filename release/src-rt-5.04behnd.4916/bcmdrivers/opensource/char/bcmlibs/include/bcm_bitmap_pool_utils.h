/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
   All Rights Reserved

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
