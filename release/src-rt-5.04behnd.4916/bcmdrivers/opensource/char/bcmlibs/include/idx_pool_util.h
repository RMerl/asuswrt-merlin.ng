/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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
 * File Name  : idx_pool_util.h
 * This file provides interface for free index pool.
 *******************************************************************************
 */

#ifndef _IDX_POOL_UTIL_H
#define _IDX_POOL_UTIL_H

/* ***************  data structures **************/


typedef struct {
    uint32_t pool_size;             /* Max number of indexes = pool size */
    uint32_t idxs_in_use;           /* Current allocation count - for debugging */
    void *pTree;                    /* Root tree node */
}IdxPool_t;

/* ********** Interface functions ********* */

int idx_pool_init(IdxPool_t *pPool, const uint32_t pool_size, const char *const owner);

int idx_pool_get_index(IdxPool_t *pPool);

int idx_pool_return_index(IdxPool_t *pPool, uint32_t free_index);

int idx_pool_index_in_use(IdxPool_t *pPool, uint32_t index);

uint32_t idx_pool_num_in_use(IdxPool_t *pPool);

uint32_t idx_pool_get_pool_size(IdxPool_t *pIdxPool);

int idx_pool_first_in_use(IdxPool_t *pPool, uint32_t *index);

int idx_pool_next_in_use(IdxPool_t *pIdxPool, uint32_t index, uint32_t *next_index);

int idx_pool_exit(IdxPool_t *pPool);


#endif /* _FREE_IDX_POOL_UTIL_H */
