/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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
