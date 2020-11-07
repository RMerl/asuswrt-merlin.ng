/*
* <:copyright-BRCM:2018:DUAL/GPL:standard
* 
*    Copyright (c) 2018 Broadcom 
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

#ifndef _RDPA_FLOW_IDX_POOL_H_
#define _RDPA_FLOW_IDX_POOL_H_

#if defined(__KERNEL__)

#include "idx_pool_util.h"

#else


typedef struct {
    uint32_t pool_size;             /* Max number of indexes = pool size */
    uint32_t idxs_in_use;           /* Current allocation count - for debugging */
    void *pTree;                    /* Root tree node */
}IdxPool_t;

#define BITS_PER_NODE       (sizeof(uint32_t)*8)

typedef void* (*alloc_func_t)(size_t size); /* void* function pointer with size_t arg */
typedef void  (*free_func_t)(void *p);      /* void function pointer with void* arg */

static int inline idx_pool_init(IdxPool_t *pPool, const uint32_t pool_size, alloc_func_t p_alloc_func)
{
    uint32_t num_bits = pool_size;
    int num_nodes = (num_bits + BITS_PER_NODE - 1) / BITS_PER_NODE;
    int i;
    uint32_t bit;
    uint32_t *pNode;
    pNode = (uint32_t *)(p_alloc_func((sizeof(uint32_t) * (num_nodes))));
    pPool->pTree = pNode;
    for (i = 0; i < num_nodes; i++)
    {
        if (num_bits >= BITS_PER_NODE)
        {
            bit = 0;
            pNode[i] = ~bit;
            num_bits -= BITS_PER_NODE;
        } else
        {
            bit = (1ULL << num_bits);
            pNode[i] = ~bit;
            num_bits -= num_bits;
        }
    }
    pPool->idxs_in_use = 0;
    pPool->pool_size = pool_size;
    return 0;
}

static int inline idx_pool_get_index(IdxPool_t *pPool)
{
    uint32_t num_bits = pPool->pool_size;
    int num_nodes = (num_bits + BITS_PER_NODE - 1) / BITS_PER_NODE;
    int i;
    uint32_t bit;
    uint32_t *pNode;
    pNode = pPool->pTree;
    for (i = 0; i < num_nodes; i++)
    {
        if (pNode[i])
        {
            for (bit = 0; bit < BITS_PER_NODE; bit++)
            {
                if (pNode[i] & (1ULL << bit))
                {
                    pNode[i] &= ~(1ULL << bit);
                    pPool->idxs_in_use++;
                    return (BITS_PER_NODE * i + bit);
                }
            }
        }
    }
    return -1;
}

static int inline idx_pool_return_index(IdxPool_t *pPool, uint32_t free_index)
{
    uint32_t num_bits = pPool->pool_size;
    int num_nodes = (num_bits + BITS_PER_NODE - 1) / BITS_PER_NODE;
    int i = free_index / BITS_PER_NODE;
    uint32_t bit = free_index % BITS_PER_NODE;
    uint32_t *pNode;
    pNode = pPool->pTree;
    if (free_index < num_bits && i < num_nodes)
    {
        if ((pNode[i] & (1ULL << bit)) == 0)
        {
            pNode[i] |= (1ULL << bit);
            pPool->idxs_in_use--;
            return 0;
        }
    }
    return -1;
}

static int inline idx_pool_index_in_use(IdxPool_t *pPool, uint32_t index)
{
    uint32_t num_bits = pPool->pool_size;
    int num_nodes = (num_bits + BITS_PER_NODE - 1) / BITS_PER_NODE;
    int i = index / BITS_PER_NODE;
    uint32_t bit = index % BITS_PER_NODE;
    uint32_t *pNode;
    pNode = pPool->pTree;
    if (index < num_bits && i < num_nodes)
    {
        if ((pNode[i] & (1ULL << bit)) == 0)
        {
            return 1;
        }
    }
    return 0;
}

static uint32_t inline idx_pool_num_in_use(IdxPool_t *pPool)
{
    return pPool->idxs_in_use;
}

static uint32_t idx_pool_get_pool_size(IdxPool_t *pIdxPool)
{
    return pIdxPool->pool_size;
}

static int inline idx_pool_first_in_use(IdxPool_t *pPool, uint32_t *index)
{
    uint32_t num_bits = pPool->pool_size;
    int num_nodes = (num_bits + BITS_PER_NODE - 1) / BITS_PER_NODE;
    int i;
    uint32_t bit;
    uint32_t *pNode;
    pNode = pPool->pTree;
    for (i = 0; i < num_nodes; i++)
    {
        for (bit = 0; bit < BITS_PER_NODE; bit++)
        {
            if ( (pNode[i] & (1ULL << bit)) == 0)
            {
                *index = (BITS_PER_NODE * i + bit);
                return 0;
            }
        }
    }
    return -1;
}
static int inline idx_pool_next_in_use(IdxPool_t *pPool, uint32_t index, uint32_t *next_idx)
{
    uint32_t num_bits = pPool->pool_size;
    int num_nodes = (num_bits + BITS_PER_NODE - 1) / BITS_PER_NODE;
    int i ;
    uint32_t bit;
    uint32_t *pNode;
    pNode = pPool->pTree;
    for (i = index/BITS_PER_NODE ; i < num_nodes; i++)
    {
        for (bit = 0; bit < BITS_PER_NODE; bit++)
        {
            if ( (pNode[i] & (1ULL << bit)) == 0)
            {
                *next_idx = (BITS_PER_NODE * i + bit);
                if (*next_idx > index)
                {
                    return 0;
                }
            }
        }
    }
    return -1;
}
static int inline idx_pool_exit(IdxPool_t *pPool, free_func_t p_free_func)
{
    pPool->idxs_in_use = 0;
    pPool->pool_size = 0;
    p_free_func(pPool->pTree);
    return 0;
}

#endif /* !defined(__KERNEL__) */

#define RDD_FLOW_ID_INVALID ((uint32_t)~0)

/* */
typedef struct _rdpa_flow_idx_pool_t {
    IdxPool_t flow_idx_pool;
    uint32_t  *rdd_flow_id_tbl_p;
} rdpa_flow_idx_pool_t;

static int inline idx_pool_init_wrap(IdxPool_t *pPool, const uint32_t pool_size, const char *const owner)
{
#if defined(__KERNEL__)
    return idx_pool_init(pPool, pool_size, owner);
#else
    return idx_pool_init(pPool, pool_size, bdmf_alloc);
#endif
}
static int inline idx_pool_exit_wrap(IdxPool_t *pPool)
{
#if defined(__KERNEL__)
    return idx_pool_exit(pPool);
#else
    return idx_pool_exit(pPool, bdmf_free);
#endif
}
static int inline rdpa_flow_idx_pool_init(rdpa_flow_idx_pool_t *p_pool, uint32_t max_flow_id, const char *const owner)
{
    int err;
    uint32_t idx;
    err = idx_pool_init_wrap(&p_pool->flow_idx_pool, max_flow_id, owner);
    if (err)
        return err;
    p_pool->rdd_flow_id_tbl_p = (uint32_t*)bdmf_alloc(sizeof(uint32_t)*max_flow_id);
    if (!p_pool->rdd_flow_id_tbl_p)
    {
        /* Memory allocation failure; exit the idx pool as well */
        idx_pool_exit_wrap(&p_pool->flow_idx_pool);
        return -1;
    }

    for (idx = 0; idx < max_flow_id; idx++)
    {
        p_pool->rdd_flow_id_tbl_p[idx] = RDD_FLOW_ID_INVALID;
    }
    return 0;
}
static int inline rdpa_flow_idx_pool_exit(rdpa_flow_idx_pool_t *p_pool)
{
    idx_pool_exit_wrap(&p_pool->flow_idx_pool);
    bdmf_free(p_pool->rdd_flow_id_tbl_p);
    return 0;
}
static int inline rdpa_flow_idx_pool_get_index(rdpa_flow_idx_pool_t *p_pool, uint32_t *flow_idx)
{
    int val = idx_pool_get_index(&p_pool->flow_idx_pool);
    if (val < 0)
        return -1;

    *flow_idx = val;
    return 0;
}
static int inline rdpa_flow_idx_pool_return_index(rdpa_flow_idx_pool_t *p_pool, uint32_t idx)
{
    int err;
    err = idx_pool_return_index(&p_pool->flow_idx_pool, idx);
    if (!err)
    {
        p_pool->rdd_flow_id_tbl_p[idx] = RDD_FLOW_ID_INVALID;
    }
    return err;
}
static int inline rdpa_flow_idx_pool_set_id(rdpa_flow_idx_pool_t *p_pool, uint32_t idx, uint32_t flow_id)
{
    if (idx_pool_index_in_use(&p_pool->flow_idx_pool, idx))
    {
        p_pool->rdd_flow_id_tbl_p[idx] = flow_id;
        return 0;
    }
    return -1;
}
static int inline rdpa_flow_idx_pool_get_id(rdpa_flow_idx_pool_t *p_pool, uint32_t idx, uint32_t *flow_id)
{
    if (idx_pool_index_in_use(&p_pool->flow_idx_pool, idx) &&
        p_pool->rdd_flow_id_tbl_p[idx] != RDD_FLOW_ID_INVALID)
    {
        *flow_id = p_pool->rdd_flow_id_tbl_p[idx];
        return 0;
    }
    return -1;
}
static int inline rdpa_flow_idx_pool_reverse_get_index(rdpa_flow_idx_pool_t *p_pool, uint32_t *idx, uint32_t flow_id)
{
    for (*idx = 0; *idx < p_pool->flow_idx_pool.pool_size; (*idx)++)
    {
        if (p_pool->rdd_flow_id_tbl_p[*idx] == flow_id)
            return 0;
    }
    return -1;
}
static uint32_t inline rdpa_flow_idx_pool_num_idx_in_use(rdpa_flow_idx_pool_t *p_pool)
{
    return idx_pool_num_in_use(&p_pool->flow_idx_pool);
}
static uint32_t inline rdpa_flow_idx_pool_get_pool_size(rdpa_flow_idx_pool_t *p_pool)
{
    return idx_pool_get_pool_size(&p_pool->flow_idx_pool);
}
static int inline rdpa_flow_idx_pool_first_in_use(rdpa_flow_idx_pool_t *p_pool, uint32_t *idx)
{
    return idx_pool_first_in_use(&p_pool->flow_idx_pool, idx);
}
static int inline rdpa_flow_idx_pool_next_in_use(rdpa_flow_idx_pool_t *p_pool, uint32_t idx, uint32_t *next_idx)
{
    return idx_pool_next_in_use(&p_pool->flow_idx_pool, idx, next_idx);
}
#endif /* _RDPA_FLOW_IDX_POOL_H_ */
